#include "roc_nsms_worker.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <NSMSService.h>
#include <NotifyService.h>

#pragma GCC diagnostic pop

#include <array>

#include <base_interfaces/uni_logger_module.h>
#include <http_handler_settings.h>
#include <roc_db_utils.h>
#include <roc_parser.h>


namespace itcs
{
namespace roc
{


RocNsmsWorker::RocNsmsWorker( std::shared_ptr< HttpHandlerSettings >& config )
     : RocDbContext( config )
     , isRunned_()
     , conditionLock_()
     , conditionVar_()
     , nsmsRpcContext_( new NsmsRpcContext( config->nsmsServerHost(), config->nsmsServerPort() ) )
     , notifyRpcContext_( new NotifyRpcContext( config->notifyHost(), config->notifyPort() ) )
     , sleepMsec_( config->nsmsSleepMsec() )
     , updateCompanyInfoFrequency_( config->updateCompanyInfoFrequency() )
{
     try
     {
          consistance( "ERROR: RocNsmsWorker::RocNsmsWorker - connection inconsistance" );
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: RocNsmsWorker::RocNsmsWorker() - exception: " << err.what() << "\n" );
     }
}


RocNsmsWorker::~RocNsmsWorker()
{

}


bool RocNsmsWorker::prepareDbMethods()
{
     Oid device     = int8Oid_;
     Oid requestId  = int8Oid_;
     Oid company    = int8Oid_;
     Oid vipnetId   = int8Oid_;
     bool status = true;

     prepareStmt( status, stmts_.getAddDeviceRequestId, { device },
                  "SELECT rollout.get_add_request_id( in_device := $1 );" );

     prepareStmt( status, stmts_.setAddDeviceRequestId, { device, requestId },
                  "SELECT rollout.set_add_request_id( in_device := $1, in_request_id := $2 );" );

     prepareStmt( status, stmts_.getDstRequestId, { device },
                  "SELECT rollout.get_dst_request_id( in_device := $1 );" );

     prepareStmt( status, stmts_.setDstRequestId, { company, requestId },
                  "SELECT rollout.set_dst_request_id( in_device_id := $1, in_request_id := $2 );" );

     prepareStmt( status, stmts_.nextNsmsOperation,
                  { device, varcharOid_, varcharOid_, vipnetId, company, boolOid_, requestId, requestId },
                  "SELECT device_id, node_name, platform_name, vipnet_id, company_id, dst_requested, dst_request_id, "
                         "add_request_id "
                    "FROM rollout.next_nsms_request;" );

     prepareStmt( status, stmts_.setVipnetId, { device, vipnetId },
                  "SELECT rollout.set_vipnet_id( in_device := $1, in_vipnet_id := $2 );" );

     prepareStmt( status, stmts_.setDstRequested, { company, requestId },
                  "SELECT rollout.set_dst_requested( in_device := $1, in_requested_status := $2 );" );

     prepareStmt( status, stmts_.getCompaniesId, { company, requestId },
                  "SELECT company_id FROM rollout.companies;" );

     prepareStmt( status, stmts_.updateCompanyInfo, { company, varcharOid_ },
                  "SELECT rollout.update_company_info( in_company := $1, in_company_name := $2 );" );

     return status;
}


void RocNsmsWorker::wakeupOperations()
{
     std::unique_lock< std::mutex > lock( conditionLock_ );
     conditionVar_.notify_one();
}


void RocNsmsWorker::start()
{
     isRunned_ = true;
     workThread_ = std::thread( &RocNsmsWorker::workLoop, this );
}


void RocNsmsWorker::stop()
{
     if ( isRunned_ )
     {
          isRunned_ = false;
          workThread_.join();
     }
}

void RocNsmsWorker::workLoop()
{
     std::chrono::milliseconds waitTime( sleepMsec_ );
     for ( size_t iteration = 0 ; isRunned_; ++iteration )
     {
          bool result = workIteration( iteration );
          for ( auto& error : requestCtx_.responseErrors )
          {
               ITCS_ERROR( "ROC NSMS worker loop error: " << static_cast< int >( error ) << " " <<
                           SvalConvert::rocErrorMessage( error ) );
          }
          requestCtx_.clear();
          // condition variable loop
          if ( !result ) // если очередь пуста - перейти в режим ожидания
          {
               std::unique_lock< std::mutex > lock( conditionLock_ );
               conditionVar_.wait_for( lock, waitTime );
          }
     }
}


bool RocNsmsWorker::getNsmsOperations( std::vector< NsmsDeviceOperation >& dest )
{
     DBResult res( conn_.get(), stmts_.nextNsmsOperation, nullptr, nullptr, nullptr, 1, PGRES_TUPLES_OK,
                   "Exec prepared failed: " );

     int ntuples = PQntuples( res );
     if ( !res || !ntuples || PQnfields( res ) != 8  )
     {
          return false;
     }


     for ( int i = 0; i < ntuples; ++ i )
     {
          NsmsDeviceOperation op;
          if ( !dbToHost( op.deviceId, this, res, i, 0 ) )
          {
               ITCS_ERROR( "RocNsmsWorker::getNsmsOperations: error: Device Id is NULL" );
               continue;
          }
          op.nodeName.assign( PQgetvalue( res, i, 1), PQgetlength( res, i, 1 ) );
          std::string platformName( PQgetvalue( res, i, 2), PQgetlength( res, i, 2 ) );
          SvalConvert::osFromStr( op.platformId, platformName );
          op.issetVipnetId = dbToHost( op.vipnetId, this, res, i, 3 );

          if ( !dbToHost( op.companyId, this, res, 0, 4 ) )
          {
               ITCS_ERROR( "RocNsmsWorker::getNsmsOperations: error: dstRequested flag is NULL" );
               continue;
          }

          if ( !dbToHost( op.dstRequested, this, res, 0, 5 ) )
          {
               ITCS_ERROR( "RocNsmsWorker::getNsmsOperations: error: dstRequested flag is NULL" );
               continue;
          }

          op.issetDstRequestId = dbToHost( op.dstRequestId, this, res, i, 6 );
          op.issetAddRequestId = dbToHost( op.addRequestId, this, res, i, 7 );
          dest.push_back( op );
     }
     return true;
}


bool RocNsmsWorker::workIteration( size_t iteration )
{
     if ( !consistance( "RocNsmsWorker::workIteration: db inconsistance" ) )
     {
          return false;
     }

     std::string errMsg;
     if ( !nsmsRpcContext_->consistance( errMsg ) )
     {
          if ( nsmsConnectionState_ != INCONSISTANCE )
          {
               ITCS_ERROR( errMsg );
               ITCS_ERROR( "ERROR: RocNsmsWorker::workIteration: bad NSMS RPC connection" );
               nsmsConnectionState_ = INCONSISTANCE;
          }
          return false;
     }
     else if ( nsmsConnectionState_ != CONSISTANCE )
     {
          ITCS_INFO( "RocNsmsWorker::workIteration: NSMS RPC connection - ok" );
          nsmsConnectionState_ = CONSISTANCE;
     }

     std::vector< NsmsDeviceOperation > operations;
     if ( !getNsmsOperations( operations ) )
     {
          return false;
     }

     bool checkOnly = true;

     if ( !( iteration % updateCompanyInfoFrequency_ ) )
     {
          updateCompaniesInfo();
     }

     for ( NsmsDeviceOperation& op : operations )
     {

          std::string nsmsToken;
          if ( !getNsmsToken( nsmsToken, op.companyId ) )
          {
               continue;
          }

          if ( !op.issetVipnetId ) // vipnet id для устройства не задан
          {
               if ( !op.issetAddRequestId ) // VipnetId не запрошен
               {
                    if ( addDeviceNode( op.vipnetId, op.issetVipnetId, op.addRequestId, nsmsToken, op.deviceId,
                                        op.platformId, op.nodeName ) && !op.issetVipnetId )
                    {
                         checkOnly = false;
                         checkViPNetId( op.vipnetId, op.addRequestId, nsmsToken, op.deviceId );
                    }
               }
               else // VipnetId запрошен но пока не задан
               {
                    checkViPNetId( op.vipnetId, op.addRequestId, nsmsToken, op.deviceId );
               }
          }
          else if ( op.dstRequested ) // пользователем запрошено создание DST
          {
               if ( !op.issetDstRequestId ) // Dst не запрошен в NSMS
               {
                    if ( requestDst( nsmsToken, op.vipnetId, op.deviceId, op.dstRequestId, op.issetDstRequestId ) &&
                         !op.issetDstRequestId ) // Запрос DST-файла не обнулен, DST-файл не готов
                    {
                         checkDstReady( nsmsToken, op.dstRequestId, op.deviceId );
                         checkOnly = false;
                    }
               }
               else // Dst запрошен в NSMS, нужно теперь его дождаться
               {
                    checkDstReady( nsmsToken, op.dstRequestId, op.deviceId );
               }
          }
     }
     return !checkOnly;
}


bool RocNsmsWorker::addDeviceNode( RequestId& vipnetId, bool& issetVipnetId, RequestId& requestId,
                                   const std::string& nsmsToken, Device inDevice, CathegoryOs::T osCathegory,
                                   const std::string& inNodeName )
{
     ITCS_INFO( "RocNsmsWorker::addDeviceNode: " << inDevice << " " << inNodeName );
     nsms_api_1_0::ANResult result;
     try
     {
          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;
          nsms_api_1_0::Credentials auth;

          auth.access_token = nsmsToken;

          ITCS_INFO(" NSMS ADD DEVICE: " << inNodeName );
          rpc.AddNode( result, auth, inNodeName, transformNsmsPlatformId( osCathegory ) );
          if ( result.result_code != nsms_api_1_0::Result::OK )
          {
               ITCS_WARNING( "RocNsmsWorker::addDeviceNode: warning: "
                             << SvalConvert::rocErrorMessage( transformNsmsError( result.result_code ) ) );

          }
          else
          {
               if ( result.__isset.vipnet_id )
               {
                    vipnetId     = result.vipnet_id;
                    issetVipnetId = true;
                    setRequestId( &vipnetId, inDevice, stmts_.setVipnetId );
               }
               else
               {
                    issetVipnetId = false;
                    requestId = result.request_id;
                    setRequestId( &requestId, inDevice, stmts_.setAddDeviceRequestId );
               }
          }
          return true;
     }
     catch ( apache::thrift::TException& exc )
     {
          ITCS_ERROR( "RocNsmsWorker::addDeviceNode RPC error:" << exc.what() );
     }
     return false;
}


void RocNsmsWorker::updateCompaniesInfo()
{
     DBResult res( conn_.get(), stmts_.getCompaniesId, nullptr, nullptr, nullptr, 1, PGRES_TUPLES_OK,
                   "Exec prepared failed: " );

     int ntuples = PQntuples( res );
     if ( !res || !ntuples || PQnfields( res ) != 1 )
     {
          return;
     }

     try
     {
          for ( int i = 0; i < ntuples; ++ i )
          {
               Company inCompany;
               dbToHost( inCompany, this, res, i, 0 );

               nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;
               nsms_api_1_0::GCIResult   result;
               nsms_api_1_0::Credentials auth;

               if ( !getNsmsToken( auth.access_token, inCompany ) )
               {
                    return;
               }

               rpc.GetCompanyInfo( result, auth );

               if ( result.result_node != nsms_api_1_0::Result::OK )
               {
                    setErrorResponse( transformNsmsError( result.result_node ) );
               }
               else if ( result.__isset.company_name && ! result.company_name.empty() )
               {
                    Company company = hostToNet( inCompany );
                    DBResult res( conn_.get(), stmts_.updateCompanyInfo,
                                  pgPack( company, result.company_name.c_str() ).begin(),
                    { sizeof( company ), static_cast< int >( result.company_name.size() ) },
                    { 1, 1 }, 1, PGRES_TUPLES_OK, "updateCompanyInfo error" );
                    if ( !res )
                    {
                         ITCS_ERROR( "Update company info error" );
                    }

               }
          }
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RocMethodsContext::updateCompanyInfo RPC error:" << exc.what() );
         setErrorResponse( RocErrorCodes::NSMS_THRIFT_EXCEPTION );
     }
}


void RocNsmsWorker::checkViPNetId( RequestId& vipnetId, RequestId addRequestId, const std::string& nsmsToken,
                                   Device inDevice )
{
     try
     {
          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;
          nsms_api_1_0::GRSResult result;
          nsms_api_1_0::Credentials auth;
          auth.access_token = nsmsToken;

          rpc.GetRequestStatus( result, auth, addRequestId );

          if ( !result.__isset.addnode_result )
          {
               ITCS_ERROR( "RocNsmsWorker::waitViPNetId - addnode_result is not set " );
               // Добавление узла не удалось, сервис скорее всего вывел "request deviation"
               // Нужно попробовать добавить узел снова
               setRequestId( nullptr, inDevice, stmts_.setAddDeviceRequestId );
               return;
          }

          if ( !result.addnode_result.__isset.vipnet_id &&
               result.addnode_result.result_code != nsms_api_1_0::Result::RESULT_NOT_READY )
          {
               if ( result.addnode_result.result_code == nsms_api_1_0::Result::REQUEST_LIMIT_EXCEEDED )
               {
                    // Добавление узла не удалось, сервис скорее всего вывел "request deviation"
                    // Нужно попробовать добавить узел снова
                    setRequestId( nullptr, inDevice, stmts_.setAddDeviceRequestId );
               }
               else
               {
                    ITCS_ERROR( "RocNsmsWorker::waitViPNetId - unknown result "
                                << SvalConvert::rocErrorMessage(
                                     transformNsmsError( result.addnode_result.result_code ) ) );
               }
               return;
          }
          else if ( result.addnode_result.__isset.vipnet_id )
          {
               vipnetId = result.addnode_result.vipnet_id;
               setRequestId( &vipnetId, inDevice, stmts_.setVipnetId );
               return;
          }
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RocNsmsWorker::waitViPNetId RPC error:" << exc.what() );
     }
}


RocErrorCodes::T RocNsmsWorker::sendSmsNotify( const std::string& secret )
{
     std::string errMsg;
     if ( !notifyRpcContext_->consistance( errMsg ) )
     {
          if ( notifyConnectionState_ != INCONSISTANCE )
          {
              ITCS_ERROR( "ERROR: Notify connection inconsistance: " << errMsg );
              notifyConnectionState_ = INCONSISTANCE;
          }
          return RocErrorCodes::NOTIFY_UNREACHIBLE;
     }
     else if ( notifyConnectionState_ != CONSISTANCE )
     {
          ITCS_INFO( "Notify connection - Ok" );
          notifyConnectionState_ = CONSISTANCE;
     }

     roc_notify_api_1_0::NotifyServiceClient& rpc = *notifyRpcContext_->client;
     roc_notify_api_1_0::SNResult    result;
     roc_notify_api_1_0::Credentials creds;
     roc_notify_api_1_0::Message     msg;

     // TODO: нужно более аккуратно формировать ответы
     msg.message_body = "Your password is: " + secret;

     creds.access_token = "123"; // TODO: уточнить, что за пароль нужен сервису нотификации

     try
     {
          rpc.SendNotify( result, creds, roc_notify_api_1_0::Transport::CELL_PHONE_SMS, msg );
          if ( result.result_code != roc_notify_api_1_0::Result::OK )
          {
               return transformNotifyErrors( result.result_code );
          }
     }
     catch ( apache::thrift::TException& exc )
     {
          ITCS_ERROR( "RocMethodsContext::getDstx RPC error:" << exc.what() );
          return RocErrorCodes::NOTIFY_THRIFT_EXCEPTION;
     }
     return RocErrorCodes::SUCCESS;
}


bool RocNsmsWorker::setDstReady( RequestId& requestId, Device inDevice )
{
     RequestId dstxRequested = 0;

     // Установить RequestId и обнулить флаг запрошенности DST, чтобы пользователь мог получить DST
     return setRequestId( &requestId, inDevice, stmts_.setDstRequestId ) &&
            setRequestId( &dstxRequested, inDevice, stmts_.setDstRequested );
}


bool RocNsmsWorker::requestDst( const std::string& nsmsToken, RequestId vipnetId, Device inDevice,
                                RequestId& requestId, bool& nullRequestId )
{
     ITCS_INFO( " NSMS GET DST (RocNsmsWorker::requestDst): " << inDevice );
     try
     {
          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;

          nsms_api_1_0::GDSTResult  result;
          nsms_api_1_0::Credentials auth;
          nsms_api_1_0::Secret      secret;
          secret.password = "123"; // TODO: Реализовать полноценную генерацию случайного пароля
          auth.access_token = nsmsToken;

          rpc.GetDST( result, auth, vipnetId, secret );

          using nsms_api_1_0::Result;
          if ( result.result_code != nsms_api_1_0::Result::OK )
          {
               ITCS_ERROR( "RocNsmsWorker::requestDst: error: unknown result status "
                             << SvalConvert::rocErrorMessage( transformNsmsError( result.result_code ) ) );
               return false;
          }

          if ( !result.__isset.dst_file ) // DST файл не готов
          {
               requestId     = result.request_id;
               nullRequestId = true;
               return setRequestId( &result.request_id, inDevice, stmts_.setDstRequestId );
          }
          else // Dst файл готов
          {
               nullRequestId = false;
               return setDstReady( result.request_id, inDevice );
          }

          // отправить SMS с паролем к dst
          // и сохранить идентификатор RPC-запроса после создания запроса dst
          RocErrorCodes::T sendSmsResult = sendSmsNotify( secret.password );
          if ( RocErrorCodes::SUCCESS == sendSmsResult )
          {
               return true;
          }
          setErrorResponse( sendSmsResult );
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RocMethodsContext::getDstx RPC error:" << exc.what() );
     }
     return false;
}


void RocNsmsWorker::checkDstReady( const std::string& nsmsToken, RequestId dstRequestId, Device inDevice )
{
     try
     {
          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;

          nsms_api_1_0::GRSResult   result;
          nsms_api_1_0::Credentials auth;
          auth.access_token = nsmsToken;

          rpc.GetRequestStatus( result, auth, dstRequestId );

          if ( !result.__isset.getdst_result )
          {
               setErrorResponse( RocErrorCodes::NSMS_EMPTY_DST_STATUS_RESPONSE );
               return;
          }

          switch ( result.getdst_result.result_code )
          {
               case nsms_api_1_0::Result::OK: // return dst
                    if ( !result.getdst_result.__isset.dst_file )
                    {
                         ITCS_ERROR( "RocNsmsWorker::waitDstReady: status = Ok, but dst is not set" );
                    }
                    else
                    {
                         setDstReady( dstRequestId, inDevice );
                    }
                    break;

               case nsms_api_1_0::Result::RESULT_NOT_READY: // pass
                    break;

               default:
                    setErrorResponse( transformNsmsError( result.getdst_result.result_code ) );
                    break;
          }
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RocNsmsWorker::waitDstReady RPC error:" << exc.what() );
     }
}


} // namespace roc
} // namespace itcs
