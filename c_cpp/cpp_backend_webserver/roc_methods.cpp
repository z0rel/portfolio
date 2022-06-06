#include <roc_methods.h>

#include <assert.h>
#include <arpa/inet.h>
#include <endian.h>
#include <time.h>
#include <thread>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Weffc++"

// std::regex для gcc-4.7.2 из песочницы работают некорректно и не могут сопоставить шаблоны
// ^[+]([0-9] ?){6,14}[0-9]$ и ^[-a-zA-Z0-9_]*$, хотя на хостовой машине, gcc-4.9 компилирует работающий код
// с такими шаблонами. Для решения данной проблемы - используются header-based boost regex

#include <boost/xpressive/xpressive.hpp>

#pragma GCC diagnostic pop

#include <json_utils.h>
#include <base_interfaces/uni_logger_module.h>
#include <http_handler_settings.h>
#include <roc_parser.h>
#include <roc_fcgi_utils.h>
#include <roc_db_utils.h>
#include <roc_nsms_worker.h>


namespace itcs
{
namespace roc
{


RocMethodsContext::RocMethodsContext( std::shared_ptr< HttpHandlerSettings >& config, RocNsmsWorker& nsmsWorker  )
     : RocDbContext( config )
     , nsmsRpcContext_( new NsmsRpcContext( config->nsmsServerHost(), config->nsmsServerPort() ) )
     , nsmsWorker_( nsmsWorker )
{
     try
     {
          consistance( "ERROR: RocMethodsContext::RocMethodsContext - connection inconsistance" );
          uriParser_ = PtrRocParser( new RocParser( *this ) );
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: RocMethodsContext::RocMethodsContext() - exception: " << err.what() << "\n" );
     }
}


RocMethodsContext::~RocMethodsContext()
{

}


RocParser* RocMethodsContext::getParser()
{
     return uriParser_.get();
}




bool RocMethodsContext::prepareDbMethods()
{
     bool status = true;
     Oid company    = int8Oid_;
     Oid user       = int8Oid_;
     Oid authUser   = int8Oid_;
     Oid device     = int8Oid_;
     Oid limit      = int8Oid_;
     Oid offset     = int8Oid_;
     Oid boolOid    = boolOid_;
     Oid requestId  = int8Oid_;
     Oid varchar    = varcharOid_;

     prepareStmt( status, stmts_.getUsersList, { company, varchar, varchar, varchar, limit, offset, varchar },
                  "SELECT rollout.get_users_list( in_company := $1, in_text := $2, in_status := $3, in_os := $4, "
                  "in_limit := $5, in_offset := $6, in_fields := $7 ); " );

     prepareStmt( status, stmts_.addUser, { company, varchar, varchar, byteaOid_, varchar, varchar, varchar, boolOid },
                  "SELECT rollout.add_user( in_company := $1, in_login := $2, in_email := $3, in_password := $4, "
                  "in_phone := $5, in_description := $6, in_nsms_access_token := $7, in_nsms_token_active := $8 );" );

     prepareStmt( status, stmts_.getSelfUser, { authUser },
                  "SELECT rollout.get_user_with_companies( in_uid := $1 );" );

     prepareStmt( status, stmts_.getUserInfo, { user, company },
                  "SELECT rollout.get_user_for_company( in_uid := $1, in_company := $2 );" );

     prepareStmt( status, stmts_.addDevice, { company, user, varchar, varchar, varchar },
                  "SELECT rollout.add_device( in_company := $1, in_user := $2, in_name := $3, in_os := $4, "
                                            " in_hwid := $5 );" );

     prepareStmt( status, stmts_.getDevices, { user, company, varchar, varchar, varchar, limit, offset, varchar },
                  "SELECT rollout.get_devices_list( in_user := $1, in_company := $2, in_name := $3, in_os := $4, "
                                            " in_hwid := $5, in_limit := $6, in_offset := $7, in_fields := $8 ); ");

     prepareStmt( status, stmts_.getDeviceInfo, { device, user, company },
                  "SELECT rollout.get_device_info( in_device := $1, in_user := $2, in_company := $3 ); ");

     prepareStmt( status, stmts_.getCompanyInfo, { company },
                  "SELECT rollout.get_company_info( in_company := $1 ); ");

     prepareStmt( status, stmts_.deleteUser, { company, user },
                  "SELECT rollout.delete_user( in_company := $1, in_uid := $2 ); ");

     prepareStmt( status, stmts_.updateUser,
                  { company, user, varchar, varchar, varchar, varchar, boolOid, varchar, boolOid },
                  "SELECT rollout.update_user( in_company := $1, in_uid := $2, in_login := $3, in_email := $4, "
                                             " in_phone := $5, in_description := $6, in_block_user := $7, "
                                             " in_nsms_access_token := $8, in_nsms_token_active := $9  ); ");

     prepareStmt( status, stmts_.getAddDeviceRequestId, { device },
                  "SELECT rollout.get_add_request_id( in_device := $1 );" );

     prepareStmt( status, stmts_.setAddDeviceRequestId, { device, requestId },
                  "SELECT rollout.set_add_request_id( in_device := $1, in_request_id := $2 );" );

     prepareStmt( status, stmts_.getDstRequestId, { device },
                  "SELECT rollout.get_dst_request_id( in_device := $1 );" );

     prepareStmt( status, stmts_.setDstRequestId, { company, requestId },
                  "SELECT rollout.set_dst_request_id( in_device_id := $1, in_request_id := $2 );" );

     prepareStmt( status, stmts_.setDstRequested, { company, requestId },
                  "SELECT rollout.set_dst_requested( in_device := $1, in_requested_status := $2 );" );

     prepareStmt( status, stmts_.getDstStatus, { device },
                  "SELECT * FROM rollout.get_dst_status( in_device_id := $1 );" );

     prepareStmt( status, stmts_.getVipNetId, { device },
                  "SELECT rollout.get_vipnet_id( in_device := $1 );" );

     prepareStmt( status, stmts_.setUniqueNodename, { device, varchar },
                  "SELECT rollout.set_device_nodename( in_device := $1, in_node_name_base := $2 );" );

     return status;
}


void RocMethodsContext::getUsersList( Company inCompany, const std::string& filterText, const std::string& osTypes,
                                      const std::string& statuses, Limit inLimit, bool limitIsSet, Offset inOffset,
                                      const std::string& fieldsFilter )
{
     TraceHelper trace( "RocMethodsContext::getUsersList" );

     if ( !consistance( "getUsersList" ) || !checkRequestPrivilege( 0, "read_company_users" ) )
     {
          return trace.setFail();
     }

     Company company = hostToNet( inCompany );
     Offset  offset  = hostToNet( inOffset );
     Limit   limit   = limitIsSet ? hostToNet( inLimit ) : 0;

     if ( !requestCtx_.setStatus(
               execPrepared( requestCtx_.response, stmts_.getUsersList,
                             pgPack( company, filterText, statuses, osTypes,
                                     ( limitIsSet ? pgValue( limit ) : nullptr ), offset, fieldsFilter ).begin(),
                             { sizeof( company ),
                               static_cast< int >( filterText.size() ),
                               static_cast< int >( statuses.size() ),
                               static_cast< int >( osTypes.size() ),
                               limitIsSet ? static_cast< int >( sizeof( limit ) ) : 0,
                               sizeof( offset ),
                               static_cast< int >( fieldsFilter.size() ) },
                             { 1, 1, 1, 1, 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void UserInfoContext::initNsmsTokenIsActive( RocDbContext::NsmsRpcContext* nsmsRpc,
                                             std::set< RocErrorCodes::T >& errors )
{
     if ( !nsmst.second )
     {
          return;
     }

     nsmstActive = false;
     std::string errMsg;
     if ( !nsmsRpc->consistance( errMsg ) )
     {
          return; // Токен можно проверить, когда NSMS станет доступным
     }
     try
     {
          nsms_api_1_0::GCIResult result;
          nsms_api_1_0::Credentials creds;
          creds.access_token = nsmst.first;

          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpc->client;
          rpc.GetCompanyInfo( result, creds );

          if ( !( nsmstActive = result.result_node == nsms_api_1_0::Result::OK ) )
          {
               errors.insert( transformNsmsError( result.result_node ) );
          }
     }
     catch ( apache::thrift::TException& exc )
     {
          errors.insert( RocErrorCodes::NOTIFY_THRIFT_EXCEPTION );
          ITCS_ERROR( "UserInfoContext::initNsmsTokenIsActive error:" << exc.what() );
     }
}


void UserInfoContext::initFromJson( Json::Value& src, HttpHandlerSettings& config,
                                    RocDbContext::NsmsRpcContext* nsmsRpc, std::set< RocErrorCodes::T >& errors )
{
     JsonValueStr::get( login, src, "login" );
     JsonValueStr::get( email, src, "email" );
     JsonValueStr::get( phone, src, "phone" );
     JsonValueStr::get( descr, src, "description" );
     JsonValueStr::get( paswd, src, "password" );
     JsonValueStr::get( nsmst, src, "nsms_access_token" );
     if ( ( isBlockedSet = src.isMember( "blockuser" ) ) )
     {
          block = JsonValueBool::get( src, "blockuser" );
     }

     namespace x = boost::xpressive;

     static const x::sregex reEmail = x::sregex_compiler().compile(
       "^[a-zA-Z0-9.!#$%&’*+/=?^_`{|}~-]+@[a-zA-Z0-9-]+(?:\\.[a-zA-Z0-9-]+)*$" );

     static const x::sregex rePassword    = x::sregex_compiler().compile( "^[A-Za-z0-9$@$!%*#?&';\"~_+]{8,}$" );
     static const x::sregex reLogin       = x::sregex_compiler().compile( "^[a-zA-Z0-9_-]{3,256}$" );
     static const x::sregex rePhone       = x::sregex_compiler().compile( "^[+](?:[0-9] ?){6,14}[0-9]$" );
     static const x::sregex rePhoneFilter = x::sregex_compiler().compile( "[() -]" );

     if ( login.second && !x::regex_match( login.first, reLogin ) )
     {
          errors.insert( RocErrorCodes::BAD_LOGIN_FORMAT );
     }

     if ( paswd.second )
     {
          if ( !x::regex_match( paswd.first, rePassword ) )
          {
               errors.insert( RocErrorCodes::BAD_PASSWORD_FORMAT );
          }
          else
          {
               // TODO: Генерация паролей в виде соль+тень зависит от интерфейса, реализуемого Феликсом Шаховым
               // (TFS 215357)
               ITCS_WARNING( "Password_shadow interface is not implemented yet" );
          }
     }
     if ( paswd.first.empty() )
     {
          paswd.first = "123"; // TODO: временное решение для первоначальной демонстрации KeySetup
          paswd.second = true;
     }

     if ( !phone.first.empty() && phone.first.front() != '+' )
     {
          phone.first.insert( phone.first.begin(), '+' );
     }
     phone.first = x::regex_replace( phone.first, rePhoneFilter, "" );
     if ( phone.second && !phone.first.empty() && !x::regex_match( phone.first, rePhone ) )
     {
          errors.insert( RocErrorCodes::BAD_PHONE_FORMAT );
     }

     if ( !email.second || email.first.size() > config.maxEmailLength() || !x::regex_match( email.first, reEmail ) )
     {
          errors.insert( RocErrorCodes::BAD_EMAIL_FORMAT );
     }

     initNsmsTokenIsActive( nsmsRpc, errors );
}


void RocMethodsContext::addUser( Company inCompany )
{
     TraceHelper trace( "RocMethodsContext::addUser" );
     if ( !consistance( "addUser" )  || !checkRequestPrivilege( 0, "add_new_user" ) )
     {
          return trace.setFail();
     }

     Json::Value res;

     if ( !RocMethodsContext::parseJsonBody( res ) )
     {
          return trace.setFail();
     }

     UserInfoContext ctx;
     ctx.login.second = true; // Логин обязателен при добавлении пользователя
     ctx.email.second = true; // email обязателен при добавлении пользователя

     ctx.initFromJson( res, *config_, nsmsRpcContext_.get(), requestCtx_.responseErrors );
     if ( !requestCtx_.responseErrors.empty() )
     {
          ITCS_WARNING( "RocMethodsContext::addUser - validate error occurs " );
          return trace.setFail();
     }

     Company company = hostToNet( inCompany );

     if ( !requestCtx_.setStatus(
               execPrepared( requestCtx_.response, stmts_.addUser,
                             pgPack( company, ctx.login, ctx.email, ctx.paswd, ctx.phone, ctx.descr,
                                     ctx.nsmst, ctx.nsmstActive ? boolTrue_.c_str() : nullptr ).begin(),
                             { sizeof( company ),
                               static_cast< int >( ctx.login.first.size() ),
                               static_cast< int >( ctx.email.first.size() ),
                               static_cast< int >( ctx.paswd.first.size() ),
                               static_cast< int >( ctx.phone.first.size() ),
                               static_cast< int >( ctx.descr.first.size() ),
                               static_cast< int >( ctx.nsmst.first.size() ),
                               boolSize_ },
                             { 1, 1, 1, 1, 1, 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::updateUser( Company inCompany, User inUid  )
{
     TraceHelper trace( "RocMethodsContext::updateUser" );
     if ( !consistance( "updateUser" )  || !checkRequestPrivilege( inUid, "edit_user_info", "edit_self_info" ) )
     {
          return trace.setFail();
     }

     Json::Value res;

     if ( !RocMethodsContext::parseJsonBody( res ) )
     {
          return trace.setFail();
     }

     UserInfoContext ctx;
     ctx.initFromJson( res, *config_, nsmsRpcContext_.get(), requestCtx_.responseErrors );
     if ( !requestCtx_.responseErrors.empty() )
     {
          ITCS_WARNING( "RocMethodsContext::updateUser - validate error occurs " );
          return trace.setFail();
     }

     Company company = hostToNet( inCompany );
     User    uid     = hostToNet( inUid );

     const char* needBlocked = ctx.isBlockedSet ?
                                    ( ctx.block ? boolTrue_.c_str() : boolFalse_.c_str() ) : nullptr;


     if ( !requestCtx_.setStatus(
               execPrepared( requestCtx_.response, stmts_.updateUser,
                             pgPack( company, uid, ctx.login, ctx.email, ctx.phone, ctx.descr,
                                     needBlocked, ctx.nsmst,
                                     ctx.nsmstActive ? boolTrue_.c_str() : boolFalse_.c_str() ).begin(),
                             { sizeof( company ),
                               sizeof( uid ),
                               static_cast< int >( ctx.login.first.size() ),
                               static_cast< int >( ctx.email.first.size() ),
                               static_cast< int >( ctx.phone.first.size() ),
                               static_cast< int >( ctx.descr.first.size() ),
                               ctx.isBlockedSet ? boolSize_ : 0,
                               static_cast< int >( ctx.nsmst.first.size() ),
                               boolSize_ },
                             { 1, 1, 1, 1, 1, 1, 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::deleteUser( Company inCompany, User inUid )
{
     TraceHelper trace( "RocMethodsContext::deleteUser" );
     if ( !consistance( "deleteUser" )  || !checkRequestPrivilege( inUid, "delete_user" ) )
     {
          return trace.setFail();
     }

     User    uid     = hostToNet( inUid );
     Company company = hostToNet( inCompany );

     if ( !requestCtx_.setStatus( execPrepared( requestCtx_.response, stmts_.deleteUser, pgPack( company, uid ).begin(),
                                                { sizeof( company ), sizeof( uid ) }, { 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::getSelfInfo()
{
     TraceHelper trace( "RocMethodsContext::getSelfInfo" );

     if ( !consistance( "getSelfInfo" ) || !checkRequestPrivilege( 0, "get_self_info" ) )
     {
          return trace.setFail();
     }

     if ( !requestCtx_.setStatus( execPrepared( requestCtx_.response, stmts_.getSelfUser,
                                                pgPack( requestCtx_.pgUid ).begin(), { sizeof( requestCtx_.pgUid ) },
                                                { 1 } ) ) )
     {
          trace.setFail();
     }
}


void RocMethodsContext::getUserInfo( Company inCompany, User inUid )
{
     TraceHelper trace( "RocMethodsContext::getUserInfo" );
     if ( !consistance( "getUserInfo" ) || !checkRequestPrivilege( inUid, "get_user_info", "get_self_info" ) )
     {
          return trace.setFail();
     }

     User    uid     = hostToNet( inUid );
     Company company = hostToNet( inCompany );

     if ( !requestCtx_.setStatus( execPrepared( requestCtx_.response, stmts_.getUserInfo,
                                                pgPack( uid, company ).begin(), { sizeof( uid ), sizeof( company ) },
                                                { 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


AddDeviceContext::AddDeviceContext( Json::Value& src )
     : os(   JsonValueStr::get( src, "os" ) ),
       osCathegory( CathegoryOs::WINDOWS_ANY ),
       name( JsonValueStr::get( src, "name" ) ),
       hwid( JsonValueStr::get( src, "hwid" ) )
{
     if ( os.empty() || !SvalConvert::osFromStr( osCathegory, os ) )
     {
          errors.push_back( RocErrorCodes::BAD_OS_IDENTIFIER );
     }
     if ( name.empty() )
     {
          errors.push_back( RocErrorCodes::BAD_DEVICE_NAME );
     }
     if ( hwid.empty() )
     {
          errors.push_back( RocErrorCodes::BAD_DEVICE_HWID );
     }
}


void RocMethodsContext::addDevice( Company inCompany, User inUid )
{
     TraceHelper trace( "RocMethodsContext::addDevice" );
     if ( !consistance( "addDevice" ) || !checkRequestPrivilege( inUid, "add_new_user_device", "add_new_self_device" ) )
     {
          return trace.setFail();
     }

     Json::Value res;

     if ( !RocMethodsContext::parseJsonBody( res ) )
     {
          return trace.setFail();
     }

     AddDeviceContext ctx( res );

     if ( !ctx.errors.empty() )
     {
          requestCtx_.responseErrors.insert( ctx.errors.begin(), ctx.errors.end() );
          return trace.setFail();
     }

     Company   company   = hostToNet( inCompany );
     User      uid       = hostToNet( inUid );

     if ( !requestCtx_.setStatus(
               execPrepared( requestCtx_.response, stmts_.addDevice,
                             pgPack( company, uid, ctx.name, ctx.os, ctx.hwid ).begin(),
                                     { sizeof( company ), sizeof( uid ),
                                       static_cast< int >( ctx.name.size() ),
                                       static_cast< int >( ctx.os.size() ),
                                       static_cast< int >( ctx.hwid.size() ) },
                                     { 1, 1, 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
     ITCS_WARNING( "ADD DEVICE: " << requestCtx_.response );

     Json::Value val;
     try
     {
          if ( !Json::Reader().parse( requestCtx_.response, val, false ) )
          {
               return;
          }
          if ( val.isMember("device") )
          {
               Device devId = atoi( val["device"]["id"].asString().c_str() );
               std::string ulogin;
               std::string udescr;
               std::string os;
               getNodeInfo( devId, ulogin, udescr, os );
               std::string baseNodename;
               nodenameGenerator_.generate( baseNodename, ulogin, udescr, os );

               Device device = hostToNet( devId );
               DBResult res( conn_.get(), stmts_.setUniqueNodename, pgPack( device, baseNodename ).begin(),
                             { sizeof( device ), static_cast< int >( baseNodename.size() ) },
                             { 1, 1 }, 1, PGRES_TUPLES_OK, "error in setUniqueName" );
               if ( !res )
               {
                    return trace.setFail();
               }
          }
     }
     catch ( std::exception& )
     {
          ITCS_ERROR( "ERROR JSON IN ADD DEVICE: " << requestCtx_.response );
     }
}


void RocMethodsContext::getDevices( Company inCompany, User inUid, const std::string& inName, const std::string& inOs,
                                    const std::string& inHwid, Limit inLimit, bool limitIsSet, Offset inOffset,
                                    const std::string& fieldsFilter)
{
     TraceHelper trace( "RocMethodsContext::getDevices" );
     if ( !consistance( "getDevices" ) || !checkRequestPrivilege( inUid, "get_user_devices", "get_self_devices" ) )
     {
          return trace.setFail();
     }

     User    uid     = hostToNet( inUid );
     Company company = hostToNet( inCompany );
     Limit   limit   = limitIsSet ? hostToNet( inLimit ) : 0;
     Offset  offset  = hostToNet( inOffset );
     std::string& response = requestCtx_.response;

     if ( !requestCtx_.setStatus(
               execPrepared( response, stmts_.getDevices,
                             pgPack( uid, company, inName, inOs, inHwid, ( limitIsSet ? pgValue( limit ) : nullptr ),
                                     offset, fieldsFilter ).begin(),
                             { sizeof( uid ), sizeof( company ),
                               static_cast< int >( inName.size() ),
                               static_cast< int >( inOs.size() ),
                               static_cast< int >( inHwid.size() ),
                               sizeof( limit ),
                               sizeof( offset ),
                               static_cast< int >( fieldsFilter.size() ) },
                             { 1, 1, 1, 1, 1, 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::getDeviceInfo( Company inCompany, User inUser, Device inDevice )
{
     TraceHelper trace( "RocMethodsContext::getDeviceInfo" );
     if ( !consistance( "getDeviceInfo" )
          || !checkRequestPrivilege( inUser, "get_user_device_info", "get_self_device_info" ) )
     {
          return trace.setFail();
     }

     Device  device  = hostToNet( inDevice );
     User    uid     = hostToNet( inUser );
     Company company = hostToNet( inCompany );
     std::string& response = requestCtx_.response;

     if ( !requestCtx_.setStatus( execPrepared( response, stmts_.getDeviceInfo, pgPack( device, uid, company ).begin(),
                                                { sizeof( device ), sizeof( uid ), sizeof( company ) },
                                                { 1, 1, 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::getCompany( Company inCompany )
{
     TraceHelper trace( "RocMethodsContext::getCompany" );
     if ( !consistance( "getCompany" ) || !checkRequestPrivilege( 0, "read_company_info" ) )
     {
          return trace.setFail();
     }

     Company company = hostToNet( inCompany );

     if ( !requestCtx_.setStatus( execPrepared( requestCtx_.response, stmts_.getCompanyInfo, pgPack( company ).begin(),
                                                { sizeof( company ) }, { 1 } ) ) )
     {
          return trace.setFail();
     }
}


void RocMethodsContext::responseDst( const std::string& dst )
{
     requestCtx_.commandStatus = true;
     // TODO: реализовать формирование имени файла как хеша
     requestCtx_.responseHeadersStr_ =
            "Status: 200 Ok \r\n"
            "Content-Type: application/octet-stream\r\n"
            "Content-Length: " + std::to_string( dst.size() ) + "\r\n"
            "Content-Disposition: attachment; filename=\"dstfile.dstx\"\r\n\r\n";
     requestCtx_.responseHeaders_ = requestCtx_.responseHeadersStr_.c_str();
     requestCtx_.response.assign(dst);
}


DstStatus::T RocMethodsContext::getDbDstStatus( Device inDevice )
{
     Device device = hostToNet( inDevice );
     DBResult res( conn_.get(), stmts_.getDstStatus, pgPack( device ).begin(), { sizeof( device ) }, { 1 }, 1,
                   PGRES_TUPLES_OK, " error in stmts_.getDstStatus ");
     if ( !res || PQnfields( res ) != 2 || PQntuples( res ) != 1 )
     {
          return DstStatus::DB_ERROR;
     }

     bool dstRequested = true;
     bool dstReady     = true;
     dstRequested = dbToHost( dstRequested, this, res, 0, 0 ) && dstRequested;
     dstReady     = dbToHost( dstReady,     this, res, 0, 1 ) && dstReady;

     if ( dstReady )
     {
          return DstStatus::DST_READY;
     }
     else if ( dstRequested )
     {
          return DstStatus::DST_NOT_READY;
     }
     return DstStatus::DST_NOT_REQUESTED;
}


void RocMethodsContext::getDstx( Company inCompany, User inUser, Device inDevice )
{
     TraceHelper trace( "RocMethodsContext::getDstx" );
     if ( !consistance( "getDstx" ) || !checkRequestPrivilege( inUser, "request_user_dst", "request_self_dst" ) )
     {
          return trace.setFail();
     }

     switch ( getDbDstStatus( inDevice) )
     {
          case DstStatus::DST_NOT_REQUESTED:
          {
               // Задать статус пользователя "В процессе"
               setUserStatus( inCompany, inUser, userInProgress_ );
               // Установить в базе флаг "запросить dst для устройства"
               RequestId dstRequested = 1;
               setRequestId( &dstRequested, inDevice, stmts_.setDstRequested );
               responseDstxLocation( "/dstx/status", inCompany, inUser, inDevice, "202 Accepted" );
               nsmsWorker_.wakeupOperations();
               return; // не запрашивать DST, запрос DST будет помещен в очередь запросов к NSMS и выполнен позже
          }
          case DstStatus::DST_NOT_READY:
               responseDstxLocation( "/dstx/status", inCompany, inUser, inDevice, "202 Accepted" );
               return;
          case DstStatus::DST_READY:
               break; // Если DST готов - запросить его обычным способом
          case DstStatus::DB_ERROR:
               setErrorResponse( RocErrorCodes::EXEC_DB_OPERATOR_ERROR );
               return trace.setFail();
     }

     try
     {
          std::string errMsg;
          if ( !nsmsRpcContext_->consistance( errMsg ) )
          {
               ITCS_ERROR( errMsg );
               setErrorResponse( RocErrorCodes::NSMS_UNREACHIBLE );
               return trace.setFail();
          }

          nsms_api_1_0::NSMSServiceClient& rpc = *nsmsRpcContext_->client;

          nsms_api_1_0::GRSResult   result;
          nsms_api_1_0::Credentials auth;
          nsms_api_1_0::VipnetID    dstRequestId = 0;

          bool requestIdIsNull = false;
          if ( !getRequestId( dstRequestId, requestIdIsNull, inDevice, stmts_.getDstRequestId ) || requestIdIsNull )
          {
               setErrorResponse( RocErrorCodes::DST_REQUEST_ID_IS_EMPTY );
               return trace.setFail();
          }

          if ( !getNsmsToken( auth.access_token, inCompany ) )
          {
               return trace.setFail();
          }

          rpc.GetRequestStatus( result, auth, dstRequestId );

          using nsms_api_1_0::Result;

          if ( !result.__isset.getdst_result || !result.getdst_result.__isset.dst_file )
          {
               ITCS_ERROR( "RocMethodsContext::getDstx ERROR: dst is not set" );
               setErrorResponse( RocErrorCodes::NSMS_EMPTY_READY_DST );
               return trace.setFail();
          }

          // Сбросить идентификатор RPC-запроса по получении dst
          // Задать статус пользователя "Ключи отправлены"
          if ( !setRequestId( nullptr, inDevice, stmts_.setDstRequestId ) ||
               !setUserStatus( inCompany, inUser, userDstSend_ ) )
          {
               return trace.setFail();
          }

          // Отдать dst пользователю
          responseDst( result.getdst_result.dst_file );
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RocMethodsContext::getDstx RPC error:" << exc.what() );
         setErrorResponse( RocErrorCodes::NSMS_THRIFT_EXCEPTION );
         return trace.setFail();
     }
}


void RocMethodsContext::responseDstStatus()
{
     requestCtx_.setStatus( true );
     requestCtx_.response = "{\"status\":\"pending\",\"eta\":\"";
     char mbstr[100];
     std::time_t t = std::time( nullptr );
     std::tm tm;
     std::strftime( mbstr, sizeof(mbstr), "%Y-%m-%dT%H:%M:%S.00Z", gmtime_r( &t, &tm ) );
     requestCtx_.response.append( mbstr );
     requestCtx_.response.append("\"}");
}


void RocMethodsContext::responseDstxLocation( const std::string& endpoint, Company inCompany, User inUser,
                                              Device inDevice, const std::string& resultCode )
{
     requestCtx_.commandStatus = true;
     std::string& dest = requestCtx_.responseHeadersStr_;
     dest = "Status: " + resultCode + " \r\n"
            "Location: /roc/api/v1/company/";
     dest.append( std::to_string( inCompany ) );
     dest.append( "/users/" );
     dest.append( std::to_string( inUser ) );
     dest.append( "/devices/" );
     dest.append( std::to_string( inDevice ) );
     dest.append( endpoint );
     dest.append( "\r\n\r\n" );
     requestCtx_.responseHeaders_ = dest.c_str();
}



void RocMethodsContext::getDstxStatus( Company inCompany, User inUser, Device inDevice )
{
     TraceHelper trace( "RocMethodsContext::getDstxStatus" );
     if ( !consistance( "getDstxStatus" )
          || !checkRequestPrivilege( inUser, "request_user_dst_status", "request_self_dst_status" ) )
     {
          return trace.setFail();
     }

     // Вернуть сразу ответ, если в БД нет информации о готовности DST
     switch ( getDbDstStatus( inDevice) )
     {
          case DstStatus::DST_NOT_REQUESTED:
               responseDstxLocation( "/dstx", inCompany, inUser, inDevice, "303 See Other" ); // не было запроса
               break;
          case DstStatus::DST_NOT_READY:
               responseDstStatus(); // сообщить, что dst не готов
               break;
          case DstStatus::DST_READY:
               responseDstxLocation( "/dstx", inCompany, inUser, inDevice, "303 See Other" ); // файл готов
               break;
          case DstStatus::DB_ERROR:
               setErrorResponse( RocErrorCodes::EXEC_DB_OPERATOR_ERROR );
               return trace.setFail();
     }
}



} // namespace roc
} // namespace itcs
