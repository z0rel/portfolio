#include <roc_db_context.h>

#include <string.h>
#include <thread>

#include <base_interfaces/uni_logger.h>
#include <roc_parser.h>
#include <json_utils.h>
#include <http_handler_settings.h>
#include <roc_db_utils.h>
#include <roc_fcgi_utils.h>


namespace itcs
{
namespace roc
{


template < typename T, typename F >
void strJoinAppend( std::string& dest, const std::string& delimeter, T container, F fun )
{
     auto it = container.begin();
     if ( it == container.end() )
     {
          return;
     }

     fun( dest, *it );
     for ( ++it; it != container.end(); ++it )
     {
          dest.append( delimeter );
          fun( dest, *it );
     }
}


PreparedOperator::PreparedOperator(const std::string& str)
     : operatorId( str )
{

}


void RequestContext::clear()
{
     response   .clear();
     responseErrors.clear();
     requestBody.clear();
     uid   = 0;
     pgUid = 0;
     responseHeaders_ = getJsonResponseHeadersBadRequest();
     responseHeadersStr_.clear();
     commandStatus = false;
}


bool RequestContext::setStatus( bool newStatus )
{
     commandStatus    = newStatus;
     responseHeaders_ = newStatus ? getJsonResponseHeadersOk() : getJsonResponseHeadersBadRequest();
     return newStatus;
}


RocDbContext::RocDbContext( std::shared_ptr<HttpHandlerSettings>& config )
     : config_( config )
     , base64Decoder_( new Base64Decoder() )
     , nodenameGenerator_( config )
{
     assert( config_ );
}

RocDbContext::~RocDbContext()
{

}


bool RocDbContext::success() const
{
     return requestCtx_.commandStatus;
}


void RocDbContext::execQuery( std::string& dest, const std::string& query )
{
     if ( !consistance(" RocMethodsContext::execQuery ") )
     {
          return;
     }
     dest.clear();
     DBResult res( conn_.get(),  query, 0, nullptr, nullptr, nullptr, nullptr, 0, PGRES_TUPLES_OK,
                   "Execute query failed: " );
     if ( !res )
     {
          return;
     }

     for ( int i = 0; i < PQntuples( res ); i++ )
     {
          dest.append( PQgetvalue( res, i, 0 ) );
     }
}


std::string& RocDbContext::response()

{
     return requestCtx_.response;
}


const char* RocDbContext::responseHeaders()
{
     return requestCtx_.responseHeaders_;
}


void RocDbContext::transformResponseIfError()
{
     if ( requestCtx_.responseErrors.empty() )
     {
          return;
     }
     setResponseErrors( requestCtx_.responseErrors );
     requestCtx_.responseErrors.clear();
}


std::string& RocDbContext::requestBody()
{
     return requestCtx_.requestBody;
}


const std::shared_ptr< HttpHandlerSettings >& RocDbContext::config() const
{
     return config_;
}


void RocDbContext::clearRequestContext()
{
     requestCtx_.clear();
}


bool RocDbContext::authorization( const char *authStr )
{
     const char authHeader[] = "Basic ";
     if ( strncmp( authHeader, authStr, sizeof( authHeader ) - 1 ) )
     {
          return false;
     }

     authStr += sizeof( authHeader ) - 1;
     std::string authData;
     base64Decoder_->decode( authData, authStr, strlen( authStr ) );

     if ( !consistance( "setAuthorizationContext" ) )
     {
          return false;
     }

     std::string::size_type pos = authData.find( ':' );
     if ( pos == std::string::npos )
     {
          ITCS_TRACE( "UNKNOWN authorization format (it is not password):" << pos );
          return false;
     }

     std::string login    = authData.substr( 0, pos );
     std::string password = authData.substr( pos + 1 );

     const int formats[]  = { 0, 1 };
     const int lengths[]  = { 0, static_cast< int >( password.size() ) };
     const char* values[] = { pgValue( login ), pgValue( password ) };

     if ( !bigintPgQuery( requestCtx_.pgUid, checkAuthorization_, values, lengths, formats ) )
     {
          return false;
     }

     requestCtx_.uid = static_cast< RequestContext::Uid >( be64toh( static_cast< uint64_t >( requestCtx_.pgUid ) ) );
     return true;
}


Oid RocDbContext::int4Oid() const
{
     return int4Oid_;
}


Oid RocDbContext::int8Oid() const
{
     return int8Oid_;
}


Oid RocDbContext::boolOid() const
{
     return boolOid_;
}


const std::string&RocDbContext::boolTrue() const
{
     return boolTrue_;
}


int RocDbContext::boolSize() const
{
     return boolSize_;
}


void RocDbContext::setErrorResponse(RocErrorCodes::T code)
{
     requestCtx_.responseErrors.insert( code );
}


bool RocDbContext::initPgTypesMetadata()
{
     bool consistanceInit = true;
     const auto initOid = [ this, &consistanceInit ]( Oid &dest, const std::string& cmd, const std::string& err )
     {
          DBResult res( conn_.get(), PQexec( conn_.get(), cmd.c_str() ), PGRES_TUPLES_OK, err, cmd );
          if ( res )
          {
               dest = PQftype( res, 0 );
          }
          consistanceInit = consistanceInit && res;
     };

     const auto getInt8 = [ this, &consistanceInit ]( UserStatus& dest, const std::string& cmd,
                                                      const std::string& err )
     {
          DBResult res( conn_.get(), cmd, 0, nullptr, nullptr, nullptr, nullptr, 1, PGRES_TUPLES_OK, err );
          if ( res && PQnfields( res ) == 1 && PQntuples( res ) == 1 )
          {
               dbToHost( dest, this, res, 0, 0 );
          }
          consistanceInit = consistanceInit && res;
     };


     initOid( int4Oid_ , "SELECT 0::int4;", "get int4 Oid failed: " );
     initOid( int8Oid_ , "SELECT 0::int8;", "get int8 Oid failed: " );

     initOid( varcharOid_ , "SELECT ''::varchar;", "get varchar Oid failed: " );
     initOid( byteaOid_, "SELECT '0'::bytea;", "get bytea Oid failed: " );

     const std::string statusQuery = "SELECT status_id FROM rollout.obj_status WHERE status_name = ";
     getInt8( userNew_,        statusQuery + "'new';"        , "get status for user is failed" );
     getInt8( userInProgress_, statusQuery + "'in_progress';", "get status for user is failed" );
     getInt8( userDstSend_,    statusQuery + "'dst-send';"   , "get status for user is failed" );
     getInt8( userBlocked_,    statusQuery + "'blocked';"    , "get status for user is failed" );

     DBResult fmtBoolT( conn_.get(), "SELECT 't'::boolean, 'f'::boolean;", 0, nullptr, nullptr, nullptr, nullptr, 1,
                        PGRES_TUPLES_OK, "get bool metadata failed: " );

     if ( fmtBoolT )
     {
          boolOid_  = PQftype( fmtBoolT, 0 );
          boolSize_ = PQfsize( fmtBoolT, 0 );
          boolFmt_  = PQfformat( fmtBoolT, 0 );
          boolTrue_.assign( std::string( PQgetvalue( fmtBoolT, 0, 0 ),
                                         static_cast< std::string::size_type>( PQfsize( fmtBoolT, 0 ) ) ) );
          boolFalse_.assign( std::string( PQgetvalue( fmtBoolT, 0, 1 ),
                                         static_cast< std::string::size_type>( PQfsize( fmtBoolT, 1 ) ) ) );
     }
     consistanceInit = consistanceInit && fmtBoolT;

     prepareStmt( consistanceInit, checkAuthorization_, { 0, byteaOid_ },
                  "SELECT rollout.check_authenticate( in_login := $1, in_password_shadow := $2 );" );

     prepareStmt( consistanceInit, checkPrivilege_, { int8Oid_, 0 },
                  "SELECT rollout.check_privilege( in_uid := $1, in_privilege := $2 );" );

     prepareStmt( consistanceInit, getNsmsToken_, { int8Oid_ },
                  "SELECT rollout.get_nsms_access_token( in_company := $1 );" );

     prepareStmt( consistanceInit, setUserStatus_, { int8Oid_, int8Oid_, int8Oid_ },
                  "SELECT rollout.set_user_status( in_company_id := $1, in_user_id := $2, in_status_id := $3 );" );

     prepareStmt( consistanceInit, getNodeInfo_, { int8Oid_ },
                  "SELECT out_login, out_description, out_platform "
                   " FROM rollout.get_node_info( in_device := $1 );" );

     prepareStmt( consistanceInit, countDuplicatedFirstLoginChars_, { varcharOid_ },
                   "SELECT COUNT(*)::int FROM rollout.users "
                   " WHERE substring(login, 0, " + std::to_string( config_->templateLoginChars() ) + ") = $1;" );

     prepareStmt( consistanceInit, setNodeName_, { varcharOid_, int8Oid_ },
                  "UPDATE rollout.devices SET node_name = $1 WHERE device_id = $2;" );

     return consistanceInit;
}


bool RocDbContext::getNodeInfo( Device inDevice, std::string& login, std::string& description, std::string& os )
{
    Device device = hostToNet( inDevice );
    const char* vals[] = { pgValue( device ) };
    DBResult res( conn_.get(), getNodeInfo_, vals, { sizeof( device ) }, { 1 }, 1, PGRES_TUPLES_OK,
                  "Error in getNodeInfo" );

    if ( !res || 1 != PQntuples( res ) || 3 != PQnfields( res ) )
    {
         return false;
    }

    login.clear();
    description.clear();
    dbToHost( login      , this, res, 0, 0 );
    dbToHost( description, this, res, 0, 1 );
    dbToHost( os         , this, res, 0, 2 );

    if ( login.size() > config_->templateLoginChars() )
    {
         login.erase( login.begin() + config_->templateLoginChars(), login.end() );
    }
    if ( description.size() > config_->templateDescriptionChars() )
    {
         description.erase( description.begin() + config_->templateDescriptionChars(), description.end() );
    }
    if ( os.size() > config_->templateOsChars() )
    {
         os.erase( os.begin() + config_->templateOsChars(), os.end() );
    }

    const char* valsCnt[] = { pgValue( login ) };
    DBResult cntRes( conn_.get(), countDuplicatedFirstLoginChars_, valsCnt,
                     { static_cast< int >( login.size() ) }, { 1 }, 1,  PGRES_TUPLES_OK, "Error in getNodeInfo" );

    if ( !cntRes )
    {
         return false;
    }
    int duplicatedLoginChars;
    dbToHost( duplicatedLoginChars, this, cntRes, 0, 0 );
    if ( duplicatedLoginChars > 1 )
    {
         login.append( std::to_string( duplicatedLoginChars ) );
    }
    return true;
}


bool RocDbContext::updateNodeName( Device inDevice, const std::string& nodeName )
{
     Device device = hostToNet( inDevice );
     DBResult res( conn_.get(), setNodeName_, pgPack( nodeName, device ).begin(),
                   { static_cast< int >( nodeName.size() ), sizeof( device ) }, { 1 }, 1, PGRES_TUPLES_OK,
                   "RocDbContext::updateNodeName" );
     return res;
}


bool RocDbContext::bigintPgQuery( pg_int64& dest, PreparedOperator& stmt, const char* const* values,
                                     const int* lengths, const int* formats )
{

     DBResult res( conn_.get(), stmt, values, lengths, formats, 1, PGRES_TUPLES_OK, "Exec prepared failed: " );

     if ( !res || 1 != PQntuples( res ) || 1 != PQnfields( res ) || PQgetisnull( res, 0, 0 ) ||
          PQftype( res, 0 ) != int8Oid_ )
     {
          return false;
     }
     dest = *reinterpret_cast< pg_int64* >( PQgetvalue( res, 0, 0 ) );
     return true;
}


void RocDbContext::prepareStmt( bool& status, PreparedOperator& stmtInfo, const std::vector< Oid >& paramTypes,
                                     const std::string& query )
{
     stmtInfo.nargs = static_cast< PreparedOperator::Nargs >( paramTypes.size() );
     if ( DBResult( conn_.get(), PQprepare(
                                 /*conn=      */ conn_.get(),
                                 /*stmtName=  */ stmtInfo.operatorId.c_str(),
                                 /*query=     */ query.c_str(),
                                 /*nParams=   */ stmtInfo.nargs,
                                 /*paramTypes=*/ paramTypes.data() ),
                     PGRES_COMMAND_OK, "Prepare " + stmtInfo.operatorId + " failed: ", stmtInfo.operatorId ) )
     {
          // Проверить, что запрос написан корректно и число элементов в векторе типов совпадает с числом аргументов
          // в запросе
          assert( checkPreparedStatement( stmtInfo, paramTypes.size() ) );
     }
     else
     {
          stmtInfo.operatorId.clear();
          status = false;
     }
}


bool RocDbContext::execPrepared( std::string&      destJson,
                                 PreparedOperator& stmtInfo,
                                 const char* const* vals,
                                 const std::initializer_list<int>& lengths,
                                 const std::initializer_list<int>& formats )
{
     return execPrepared( destJson, stmtInfo, vals, lengths.begin(), formats.begin() );
}


bool RocDbContext::execPrepared( std::string& destJson, PreparedOperator& stmtInfo,
                                      const char* const* values, const int* lengths, const int* formats )
{
     destJson.clear();
     DBResult res( conn_.get(), stmtInfo, values, lengths, formats, 0, PGRES_TUPLES_OK, "Exec prepared failed: " );
     if ( !res )
     {
          return false;
     }

     for ( int i = 0; i < PQntuples( res ); i++ )
     {
          destJson.append( PQgetvalue( res, i, 0 ) );
     }

     return !transformDbmsError( destJson );
}


bool RocDbContext::initAfterConnect()
{
     if ( initPgTypesMetadata() && prepareDbMethods() )
     {
          return true;
     }
     else
     {
          ITCS_WARNING( "ERROR: RocMethodsContext::initAfterConnect - bad connection to PostgreSQL" );
          conn_.reset();
     }
     return false;
}


void RocDbContext::setResponseErrors( std::set< RocErrorCodes::T >& errors )
{
     requestCtx_.response.clear();
     if ( errors.size() == 1 )
     {
          getErrorJson( requestCtx_.response, *errors.begin() );
     }
     else
     {
          static const auto convertErrcode = []( std::string& dest, RocErrorCodes::T val )
          {
               getErrorJsonBody( dest, val, SvalConvert::rocErrorMessage(val) );
          };

          getErrorJsonOpened( requestCtx_.response, RocErrorCodes::MULTI_ERROR,
                              SvalConvert::rocErrorMessage( RocErrorCodes::MULTI_ERROR ) );
          requestCtx_.response.append(",\"errors_list\":[{");
          strJoinAppend( requestCtx_.response, "},{", errors, convertErrcode );
          requestCtx_.response.append("}]}}");
     }
}


bool RocDbContext::transformDbmsError( std::string& errorStr )
{
     const char errorPrefix[] = "{\"error\":";

     // декодировать ошибку из СУБД
     if ( 0 != errorStr.compare( 0, std::extent< decltype( errorPrefix ) > ::value - 1 , errorPrefix ) )
     {
          return false;
     }

     Json::Value val;
     try
     {
          if ( Json::Reader().parse( errorStr, val, false ) )
          {
               errorStr.clear();
               setErrorResponse( SvalConvert::dbErrorCodeId( val["error"].asString() ) );
               return true;
          }
     }
     catch ( std::exception& )
     {

     }

     ITCS_TRACE( "RocMethodsContext::addUser - bad dbms json: " << errorStr );
     setErrorResponse( RocErrorCodes::BAD_JSON_FORMAT );
     return true;
}


bool RocDbContext::consistance( const std::string& caller )
{
     for ( unsigned int i = 0; i < config_->dbReconnectAttempts(); ++i )
     {
          if ( !conn_ )
          {
               conn_ = PtrConnection( PQconnectdb( config_->postgresConnString().c_str() ), PQfinish );

               if ( PQstatus( conn_.get() ) == CONNECTION_OK && initAfterConnect() )
               {
                    return true;
               }
               else
               {
                    ITCS_WARNING( "ERROR: Connection to database '" << config_->postgresConnString()
                                  << "' failed: " << PQerrorMessage( conn_.get() ) );
                    conn_.reset();
                    continue;
               }
          }

          if ( PQstatus( conn_.get() ) == CONNECTION_OK )
          {
               return true;
          }

          // Попытка восстановить соединение, если оно перешло в состояние "bad"
          PQreset( conn_.get() );

          if ( PQstatus( conn_.get() ) == CONNECTION_OK && initAfterConnect() )
          {
               return true;
          }
          ITCS_WARNING( "ERROR: RocMethodsContext::consistance() - restore connection is fail");

          std::this_thread::sleep_for( std::chrono::seconds( config_->dbReconnectTimeout() )  );
     }
     ITCS_WARNING( "ERROR: RocMethodsContext::" << caller << " - connection inconsistance" );
     return false;
}


bool RocDbContext::checkPreparedStatement( PreparedOperator& op, size_t typesLen )
{
     DBResult res( conn_.get(), PQdescribePrepared( conn_.get(), op.operatorId.c_str() ),
                   PGRES_COMMAND_OK, "Describe prepared command for " + op.operatorId + " failed", op.operatorId );
     if ( res )
     {
          return PQnparams( res ) == static_cast< int >( typesLen );
     }
     return false;
}


bool RocDbContext::checkRequestPrivilege( User uid, const std::string& admPriv, const std::string& userPriv )
{
     const int   fmts[]    = { 1, 0 };
     const int   lengths[] = { sizeof( requestCtx_.pgUid ), 0 };
     const char* vals[]    = { pgValue( requestCtx_.pgUid ), pgValue( admPriv ) };

     pg_int64 res = 0;
     if ( bigintPgQuery( res, checkPrivilege_, vals, lengths, fmts ) && be64toh( res ) )
     {
          return true;
     }

     if ( uid && requestCtx_.uid == uid )
     {
          vals[1] = pgValue( userPriv );
          res = 0;
          if ( bigintPgQuery( res, checkPrivilege_, vals, lengths, fmts ) && be64toh( res ) )
          {
               return true;
          }
     }

     setErrorResponse( RocErrorCodes::AUTHORIZATION_ERROR );
     return false;
}


bool RocDbContext::parseJsonBody( Json::Value& res )
{
     std::string& request = requestCtx_.requestBody;
     try
     {
          if ( Json::Reader().parse( request, res, false ) )
          {
               return true;
          }
     }
     catch ( std::exception& )
     {

     }

     setErrorResponse( RocErrorCodes::BAD_JSON_FORMAT );
     return false;
}


bool RocDbContext::getNsmsToken( std::string& destNsmsToken, Company inCompany )
{
     Company company = hostToNet( inCompany );

     DBResult res( conn_.get(), getNsmsToken_, pgPack( company ).begin(),
                   { sizeof( company ) },
                   { 1 }, 1, PGRES_TUPLES_OK, "Exec authNsmsToken failed: " );

     if ( !res || 1 != PQnfields( res ) || 1 != PQntuples( res ) )
     {
          setErrorResponse( RocErrorCodes::BAD_DB_NSMS_TOKEN_AUTH );
          return false;
     }

     destNsmsToken.assign( PQgetvalue( res, 0, 0 ), PQgetlength( res, 0, 0 ) );
     return true;
}


bool RocDbContext::getRequestId( RequestId& requestId, bool& isNull, Device inDev, PreparedOperator& op )
{
     Device  device  = hostToNet( inDev );

     DBResult res( conn_.get(), op, pgPack( device ).begin(), { sizeof( device )  }, { 1 }, 1, PGRES_TUPLES_OK,
                   "Exec RocMethodsContext::getRequestId failed: " );
     if ( !res )
     {
          ITCS_ERROR( "ERROR: Error in execution of prepared query: " << op.operatorId );
          setErrorResponse( RocErrorCodes::EXEC_DB_OPERATOR_ERROR );
          return false;
     }

     isNull = !dbToHost( requestId, this, res, 0, 0 );
     return true;
}


bool RocDbContext::setRequestId( const RequestId* inRequestId, Device inDevice, PreparedOperator& op )
{
     Company     device      = hostToNet( inDevice );
     const char* pgRequestId = nullptr;
     RequestId   requestId = 0;

     if ( nullptr != inRequestId )
     {
          requestId = hostToNet( *inRequestId );
          pgRequestId = pgValue( requestId );
     }

     DBResult res( conn_.get(), op, pgPack( device, pgRequestId ).begin(),
                   { sizeof( device ),  sizeof( requestId )  },
                   { 1, 1 }, 0, PGRES_TUPLES_OK, "Exec RocMethodsContext::setRequestId failed: " );

     if ( !res )
     {
          setErrorResponse( RocErrorCodes::EXEC_DB_OPERATOR_ERROR );
          return false;
     }

     std::string dbResult;
     dbResult.assign( PQgetvalue( res, 0, 0 ), PQgetlength( res, 0, 0 ) );
     return !transformDbmsError( dbResult ); // декодировать ошибку из базы, если она есть
}


bool RocDbContext::setUserStatus( Company inCompanyId, User inUserId, UserStatus inUserStatus )
{
     Company    company = hostToNet( inCompanyId );
     User       user    = hostToNet( inUserId );
     UserStatus status  = hostToNet( inUserStatus );

     DBResult res( conn_.get(), setUserStatus_, pgPack( company, user, status ).begin(),
                   { sizeof( company ),  sizeof( user ), sizeof( status )  },
                   { 1, 1, 1 }, 1, PGRES_TUPLES_OK, "Exec setUserStatus failed: " );
     if ( !res )
     {
          setErrorResponse( RocErrorCodes::EXEC_DB_OPERATOR_ERROR );
          ITCS_ERROR( "Exec prepared operator error: stmts_.setUserStatus" );
     }
     return res;
}


Oid RocDbContext::varcharOid() const
{
     return varcharOid_;
}


bool openTcpConnection( int& sd, const char* hostname, int port, std::string& errMessage )
{
     struct hostent*    host;
     struct sockaddr_in addr;

     if ( ( host = gethostbyname( hostname ) ) == NULL )
     {
          perror( hostname );
          errMessage = std::string( "ERROR: error gethostbyname for " ) + hostname + " " + std::to_string( port );
          return false;
     }
     sd = socket( PF_INET, SOCK_STREAM, 0 );
     bzero( &addr, sizeof( addr ) );
     addr.sin_family      = AF_INET;
     addr.sin_port        = htons( static_cast< uint16_t >( port ) );
     addr.sin_addr.s_addr = *reinterpret_cast< decltype( addr.sin_addr.s_addr )* >( host->h_addr );
     if ( connect( sd, reinterpret_cast< struct sockaddr* >( &addr ), sizeof( addr ) ) != 0 )
     {
          close( sd );
          errMessage = std::string( "ERROR: bad connect to " ) + hostname + " " + std::to_string( port );
          return false;
     }
     return true;
}


} // namespace roc
} // namespace itcs
