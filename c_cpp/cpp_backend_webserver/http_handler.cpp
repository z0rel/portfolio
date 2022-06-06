#include <http_handler.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <unistd.h>
#include <assert.h>
#include <fcgi_config.h>
#include <poll.h>

#include <base_interfaces/uni_logger_module.h>
#include <json_utils.h>
#include <roc_fcgi_utils.h>
#include <roc_parser.h>
#include <http_handler_settings.h>
#include <roc_methods.h>
#include <roc_nsms_worker.h>


namespace itcs
{
namespace roc
{


HttpHandler::HttpHandler( const std::string& configFilename )
     : config_( std::make_shared< HttpHandlerSettings >( configFilename ) )
     , workThreads_()
     , acceptLock_()
     , uriDecoder_( new UriDecoder() )
     , nsmsWorker_( new RocNsmsWorker( config_ ) )
{
     TRACE( "HttpHandler HttpHandler" );
     FCGX_Init();
}


void HttpHandler::finish()
{
     stopRunning();

     if ( socketFd_ >= 0 )
     {
          close( socketFd_ );
          socketFd_ = -1;
     }

     for ( auto& thread : workThreads_ )
     {
          thread.join();
     }
     workThreads_.clear();
     nsmsWorker_->stop();
}


HttpHandler::~HttpHandler()
{
     TRACE( "HttpHandler ~HttpHandler" );
     finish();
     TRACE( "HttpHandler ~HttpHandler end" );
}


void HttpHandler::mainLoop()
{
     TRACE( "HttpHandler mainLoop" );
     assert( !isRunning_ ); // не должно быть более одного экземпляра HttpHandler, запустившего mainLoop()

     TRACE( "HttpHandler run open socket \"" << config_->socketPath() << "\"" );
     socketFd_ = FCGX_OpenSocket( config_->socketPath().c_str(), static_cast< int >( config_->queueLength() ) );
     if ( socketFd_ < 0 )
     {
          TRACE( "CRIT ERROR: could not create socket" );
          return stopRunning();
     }

     isRunning_ = true;
     TRACE( "HttpHandler run starting threads" );
     for ( unsigned int i = 0; i < config_->threadPoolSize(); ++i )
     {
          workThreads_.push_back( std::thread( &HttpHandler::work, this ) );
     }
     nsmsWorker_->start();


     while ( isRunning_ )
     {
          // Если один из потоков заврешил свою работу, выйдем после таймаута
          std::this_thread::sleep_for( std::chrono::seconds( config_->timeoutEndSec() ) );
     }
     TRACE( "HttpHandler stop" );
}


void HttpHandler::stopRunning()
{
     isRunning_ = false;
}


HttpHandler::AcceptState HttpHandler::pollRequest()
{
     struct pollfd ufds;
     ufds.fd      = socketFd_;
     ufds.events  = POLLIN | POLLPRI | POLLRDHUP; // доступны данные или соединение сброшено
     ufds.revents = 0; // возвращеные события

     int retval;
     int pollTimeout = static_cast< int >( config_->pollTimeout() );
     while ( !( retval = poll( &ufds, 1, /* timeout = */ pollTimeout ) ) ) // ждать, пока на сокете не появятся данные
     {
          if ( !isRunning_ )
          {
               return STOPPED;
          }
     }

     if ( !isRunning_ )
     {
          return STOPPED;
     }

     if ( retval < 0 )
     {
          int errnum = errno;
          ITCS_ERROR( "ERROR: poll socket error: " << errnum << " " << strerror( errnum ) );
          return STOPPED;
     }
     else if ( ufds.revents & ( POLLERR | POLLHUP | POLLRDHUP | POLLNVAL ) )
     {
          decltype( ufds.revents ) f = ufds.revents;
          ITCS_ERROR( "ERROR: socket event error: "
                      << ( f & POLLERR   ? "ERR,"   : "" )  << ( f & POLLHUP  ? "HUP,"  : "" )
                      << ( f & POLLRDHUP ? "RDHUP," : "" )  << ( f & POLLNVAL ? "NVAL," : "" ) );
          return STOPPED;
     }

     int sockerr = 0;
     socklen_t len = sizeof( sockerr );
     if ( getsockopt( socketFd_, SOL_SOCKET, SO_ERROR, &sockerr, &len ) == -1 )
     {
          int errnum = errno;
          ITCS_ERROR( "ERROR: getsockopt error: "
                      << errnum << " " << strerror( errnum ) );
          return STOPPED;
     }
     else if ( sockerr )
     {
          ITCS_ERROR( "ERROR: getsockopt SOL_SOCKET error: "
                      << sockerr << " " << strerror( sockerr ) );
          return STOPPED;
     }
     else if ( !( ufds.revents & ( POLLIN | POLLPRI ) ) )
     {
          return NO_DATA;
     }
     return ACCEPTED;
}


HttpHandler::AcceptState HttpHandler::acceptFcgxRequest( FCGX_Request& request )
{
     TRACE( "HttpHandler work FCGX_Accept_r" );

     // В каждый момент времени - только один поток принимает запросы с сокета
     std::lock_guard< std::mutex > lk( acceptLock_ );
     // До входа в данную секцию, в другом потоке мог произойти сброс isRunning
     if ( !isRunning_ )
     {
          return STOPPED;
     }

     // Ждать данные через poll, чтобы была возможность корректно обрабатывать завершение
     // FCGX_Accept_r блокирует исполнение до поступления данных, а реализация с O_NONBLOCK - громоздкая.
     AcceptState pollState = pollRequest();
     if ( pollState != ACCEPTED )
     {
          return pollState;
     }

     // В каждый момент времени - только один поток принимает запросы с сокета
     int acceptRes = FCGX_Accept_r( &request );
     if ( socketFd_ <= 0 )
     {
          TRACE( "ERROR: FCGX_Accept_r error : sockFd <= 0" );
          stopRunning();
          return STOPPED;
     }
     else if ( acceptRes < 0 )
     {
          TRACE( "WARNING: FCGX_Accept_r: Socket does not contain new data to read" );
          return NO_DATA;
     }

     TRACE( "HttpHandler FCGX_Accept_r acceptRes= " << acceptRes );
     return ACCEPTED;
}


void HttpHandler::work()
{
     try
     {
          TRACE( "HttpHandler work" );
          RocMethodsContext ctx( this->config_, *(this->nsmsWorker_) );
          FCGX_Request request;

          {
               unsigned int attemptCount = config_->maxFailedAttempt();
               while ( FCGX_InitRequest( &request, socketFd_, 0 ) != 0 && socketFd_ > 0 )
               {
                    // Ошибка при инициализации структуры запроса
                    TRACE( "HttpHandler work Can not init request. Main socket is: "
                                << socketFd_ << "\n" << "Let's try again " << attemptCount << " times." );
                    std::this_thread::sleep_for( std::chrono::seconds( config_->timeoutErrorSec() ) );
                    if ( !--attemptCount )
                    {
                         std::lock_guard< std::mutex > lk( acceptLock_ );
                         stopRunning();
                         return;
                    }
               }
          }


          AcceptState state;
          while ( STOPPED != ( state = acceptFcgxRequest( request ) ) )
          {
               if ( state == ACCEPTED )
               {
                    execRequest( ctx, request ); // Обработать HTTP-запрос и сформировать ответ
               }
               else
               {
                    // Либо poll ложно сработал, либо FCGX_Accept_r не увидел данных, а они есть (на сокете)
                    std::this_thread::sleep_for( std::chrono::milliseconds( config_->pollTimeout() ) );
               }
          }

          FCGX_Free( &request, 1 );
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: HttpHandler::work - exception: " << err.what() << "\n" );
     }
}



void HttpHandler::sendJsonResponse( FCGX_Request& reqv, const char* responseHeaders, const std::string& responseBody )
{
     FCGX_PutS( responseHeaders, reqv.out );
     FCGX_PutStr( responseBody.c_str(), static_cast< int >( responseBody.size() ), reqv.out );
     FCGX_Finish_r( &reqv ); // Записать ответ в сокет
}


void HttpHandler::sendErrorJsonResponse( FCGX_Request& req, RocErrorCodes::T code )
{
     std::string errStr;
     getErrorJson( errStr, code );
     sendJsonResponse( req, getJsonResponseHeadersBadRequest(), errStr );
}


bool HttpHandler::authorization( RocMethodsContext& ctx, FCGX_Request& req )
{
     return ctx.authorization( FCGX_GetParam( "AUTHORIZATION", req.envp ) );
}


void HttpHandler::execRequest( RocMethodsContext& ctx, FCGX_Request& req )
{
     using std::string;
     try
     {
          const char *httpMethodStr;
          HttpMethodTok::T httpMethod = SvalConvert::httpMethodTokenId(
               ( httpMethodStr = FCGX_GetParam( "REQUEST_METHOD", req.envp ) ) ? httpMethodStr : ""  );

          if ( httpMethod != HttpMethodTok::UNKNOWN )
          {
               if ( !authorization( ctx, req ) )
               {
                    sendErrorJsonResponse( req, RocErrorCodes::AUTHORIZATION_ERROR );
               }
               else
               {
                    switch ( httpMethod )
                    {
                         case HttpMethodTok::POST: // pass
                         case HttpMethodTok::PUT:
                              getRequestBody( ctx.requestBody(), req, config_->requestBodyLimit() );
                              break;

                         default:
                              break;
                    }

                    string uri( 1, SvalConvert::httpMethodToken( httpMethod ) );
                    const char *bufferPtr;
                    uri.append( ( bufferPtr = FCGX_GetParam( "SCRIPT_NAME", req.envp ) ) ? bufferPtr : ""  );

                    bufferPtr = FCGX_GetParam( "QUERY_STRING", req.envp );
                    if ( bufferPtr && strlen( bufferPtr ) )
                    {
                         uri.push_back('?');
                         // Строки от nginx могут быть как в URI-encoding (percent encoding), так и декодированые
                         uriDecoder_->decode( uri, bufferPtr );
                    }

                    TRACE( "HttpHandler - parse: " << uri );

                    if ( ctx.getParser()->parseURI( uri ) != CathegoryRocCommand::UNKNOWN || !ctx.response().empty() )
                    {
                         ctx.transformResponseIfError();
                         sendJsonResponse( req, ctx.responseHeaders(), ctx.response() );
                         TRACE( "HttpHandler - success: " << uri );
                    }
                    else
                    {
                         sendErrorJsonResponse( req, RocErrorCodes::BAD_REQUEST );
                         ITCS_WARNING( "HttpHandler - bad request: " << uri );
                    }
               }
          }
          else
          {
               sendErrorJsonResponse( req, RocErrorCodes::BAD_REQUEST );
               ITCS_WARNING( "HttpHandler - unknown HTTP method: " << ( httpMethodStr ? httpMethodStr : "" ) );
          }
     }
     catch ( std::exception& err )
     {
          string errStr;
          getErrorJson( errStr, RocErrorCodes::EXCEPTION,
                        SvalConvert::rocErrorMessage( RocErrorCodes::EXCEPTION ) + ": " + err.what() );
          sendJsonResponse( req, getJsonResponseHeadersBadRequest(), errStr );
          ITCS_ERROR( "ERROR: HttpHandler::execRequest - exception: " << err.what() );
     }
     ctx.clearRequestContext();
}


/// Получить конфигурацию исполнения
HttpHandlerSettings& HttpHandler::config()
{
     return *config_;
}


} // namespace roc
} // namespace itcs
