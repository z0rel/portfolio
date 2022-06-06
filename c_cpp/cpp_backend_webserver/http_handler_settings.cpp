#include <http_handler_settings.h>

#include <fstream>

#include <base_interfaces/uni_logger_module.h>
#include <json_utils.h>


namespace itcs
{
namespace roc
{


using std::string;
using std::ifstream;
using std::unique_ptr;


std::unique_ptr< Json::Value > readConfigFile( const string& configFilename )
{
     unique_ptr< Json::Value > ctx( new Json::Value() );
     ifstream fs( configFilename.c_str(), ifstream::binary | std::ios_base::in );
     Json::Reader reader;

     if ( !fs )
     {
          return ctx;
     }
     if ( !reader.parse( fs, *ctx, true ) )
     {
          ITCS_WARNING( "Read config error: " << reader.getFormattedErrorMessages() << "\n" );
     }
     return ctx;
}


HttpHandlerSettings::HttpHandlerSettings( const string& configFilename )
     : ctx_                       ( readConfigFile( configFilename ) )
     , timeoutEndSec_             ( JsonValueInt::get( *ctx_, "TIMEOUT_END_SEC", 3 ) )
     , timeoutErrorSec_           ( JsonValueInt::get( *ctx_, "TIMEOUT_ERROR_SEC", 1 ) )
     , threadPoolSize_            ( JsonValueInt::get( *ctx_, "NUM_THREAD", 1 ) )
     , socketPath_                ( JsonValueStr::get( *ctx_, "SOCKET_PATH", "127.0.0.1:9091" ) )
     , postgresConnString_        ( JsonValueStr::get( *ctx_, "POSTGRES_CONN_STRING",
          "dbname=rollout user=roc_service connect_timeout=5" ) )
     , queueLength_               ( JsonValueInt::get( *ctx_, "QUEUE_LENGTH", 1 )  )
     , requestPayloadLen_         ( JsonValueInt::get( *ctx_, "REQUEST_PAYLOAD_LEN", 1024 ) )
     , maxFailedAttempt_          ( JsonValueInt::get( *ctx_, "MAX_FAILED_ATTEMPT", 5 ) )
       // Значение максимальной длины email (320) - установлено согласно требованиям на вики
     , maxEmailLength_            ( JsonValueInt::get( *ctx_, "MAX_EMAIL_LENGTH", 320 ) )
     , dbReconnectTimeout_        ( JsonValueInt::get( *ctx_, "DB_RECONNECT_TIMEOUT", 5 ) )
     , dbReconnectAttempts_       ( JsonValueInt::get( *ctx_, "DB_RECONNECT_ATTEMPTS", 5 ) )
     , pollTimeout_               ( JsonValueInt::get( *ctx_, "POLL_TIMEOUT", 500 ) )
     , requestBodyLimit_          ( JsonValueInt::get( *ctx_, "REQUEST_BODY_LIMIT", 1024 * 128 ) ) // 128 КБ
     , jsonBodyLimit_             ( JsonValueInt::get( *ctx_, "JSON_STRING_LIMIT", 1000 ) )
     , nsmsServerPort_            ( JsonValueInt::get( *ctx_, "NSMS_SERVER_PORT", 9985 ) )
     , nsmsServerHost_            ( JsonValueStr::get( *ctx_, "NSMS_SERVER_HOST", "localhost" ) )
     , notifyPort_                ( JsonValueInt::get( *ctx_, "NOTIFY_SERVER_PORT", 9986 ) )
     , notifyHost_                ( JsonValueStr::get( *ctx_, "NOTIFY_SERVER_HOST", "localhost" ) )
     , nsmsSleepMsec_             ( JsonValueInt::get( *ctx_, "NSMS_SLEEP_MSEC", 1000 ) )
     , updateCompanyInfoFrequency_( JsonValueInt::get( *ctx_, "UPDATE_COMPANY_INFO_FREQUENCY", 30 ) )
     , nodenameTemplate_          ( JsonValueStr::get( *ctx_, "NODENAME_TEMPLATE",
                                                       "timestamp [login][ desc][ platform]" ) )
     , templateLoginChars_        ( JsonValueInt::get( *ctx_, "TEMPLATE_LOGIN_CHARS", 28 ) )
     , templateOsChars_           ( JsonValueInt::get( *ctx_, "TEMPLATE_OS_CHARS", 3 ) )
     , templateDescriptionChars_  ( JsonValueInt::get( *ctx_, "TEMPLATE_DESCRIPTION_CHARS", 12 ) )
     , nodenameMaxlen_            ( JsonValueInt::get( *ctx_, "NODENAME_MAXLEN", 49 ) )
{
     ctx_.reset(); // после инициализации конфига, json больше не нужен
}


const std::string& HttpHandlerSettings::socketPath() const
{
     return socketPath_;
}


const std::string& HttpHandlerSettings::postgresConnString() const
{
     return postgresConnString_;
}


unsigned int HttpHandlerSettings::threadPoolSize() const
{
     return threadPoolSize_;
}


unsigned int HttpHandlerSettings::queueLength() const
{
     return queueLength_;
}


unsigned int HttpHandlerSettings::requestPayloadLen() const
{
     return requestPayloadLen_;
}


unsigned int HttpHandlerSettings::maxFailedAttempt() const
{
     return maxFailedAttempt_;
}


unsigned int HttpHandlerSettings::timeoutEndSec() const
{
     return timeoutEndSec_;
}


unsigned int HttpHandlerSettings::timeoutErrorSec() const
{
     return timeoutErrorSec_;
}


unsigned int HttpHandlerSettings::maxEmailLength() const
{
     return maxEmailLength_;
}


unsigned int HttpHandlerSettings::dbReconnectTimeout() const
{
     return dbReconnectTimeout_;
}


unsigned int HttpHandlerSettings::pollTimeout() const
{
     return pollTimeout_;
}


unsigned int HttpHandlerSettings::dbReconnectAttempts() const
{
     return dbReconnectAttempts_;
}


size_t HttpHandlerSettings::requestBodyLimit() const
{
     return requestBodyLimit_;
}


size_t HttpHandlerSettings::jsonBodyLimit() const
{
     return jsonBodyLimit_;
}


unsigned int HttpHandlerSettings::nsmsServerPort() const
{
     return nsmsServerPort_;
}


const std::string& HttpHandlerSettings::nsmsServerHost() const
{
     return nsmsServerHost_;
}


unsigned int HttpHandlerSettings::notifyPort() const
{
     return notifyPort_;
}


const std::string& HttpHandlerSettings::notifyHost() const
{
     return notifyHost_;
}


unsigned int HttpHandlerSettings::nsmsSleepMsec() const
{
     return nsmsSleepMsec_;
}


unsigned int HttpHandlerSettings::updateCompanyInfoFrequency() const
{
     return updateCompanyInfoFrequency_;
}


const std::string& HttpHandlerSettings::nodenameTemplate() const
{
     return nodenameTemplate_;
}


unsigned int HttpHandlerSettings::templateLoginChars() const
{
     return templateLoginChars_;
}


unsigned int HttpHandlerSettings::templateOsChars() const
{
     return templateOsChars_;
}


unsigned int HttpHandlerSettings::templateDescriptionChars() const
{
     return templateDescriptionChars_;
}


unsigned int HttpHandlerSettings::nodenameMaxlen() const
{
     return nodenameMaxlen_;
}


} // namespace roc
} // namespace itcs
