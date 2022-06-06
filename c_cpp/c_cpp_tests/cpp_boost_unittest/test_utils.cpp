#include <test_utils.h>

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <functional>
#include <cstdlib>
#include <iostream>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

#include <thrift/server/TSimpleServer.h>

#pragma GCC diagnostic pop

#include <base_interfaces/uni_logger_module.h>
#include <roc_fcgi_utils.h>
#include <uni_log_impl.h>
#include <notify_service_simulator.h>
#include <http_handler_settings.h>
#include <roc_db_context.h>


namespace itcs
{
namespace roc
{


/// Инициализировать библиотеку SSL
void initSSL()
{
     SSL_library_init();
     OpenSSL_add_all_algorithms(); // Загрузить криптографию и все остальное
     SSL_load_error_strings(); // Получить и зарегистрировать сообщения об ошибках
}

/// Освободить ресурсы библиотеки SSL
void cleanupSSL()
{
    FIPS_mode_set(0);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
}

/// Инициализировать контекст текущего SSL-запроса
bool initCtx( SSL_CTX*& ctx )
{
     const SSL_METHOD* method = TLSv1_2_client_method(); // Создать новый экземпляр клиентского метода
     if ( nullptr == ( ctx = SSL_CTX_new( method ) ) ) // Создать новый контекст
     {
          ERR_print_errors_fp( stderr );
          return false;
     }
     return true;
}


void stripHeader( std::string &httpCode, std::string& header )
{
     std::string::size_type pos = 0;
     std::string::size_type spacePos = header.find( ' ' );
     if ( std::string::npos != spacePos )
     {
          std::string::size_type endlPos = header.find( "\r\n", spacePos );
          if ( std::string::npos != endlPos )
          {
               ++spacePos;
               httpCode = header.substr( spacePos, endlPos - spacePos );
               pos = endlPos;
          }
     }

     pos = header.find( "\r\n\r\n", pos );
     if ( std::string::npos == pos )
     {
          return;
     }

     if ( std::string::npos == ( pos = header.find( "\r\n", pos + 4 ) ) )
     {
          return;
     }
     header.erase( 0, pos + 2 );

     if ( std::string::npos == ( pos = header.find( "\r\n0\r\n\r\n" ) ) )
     {
          return;
     }
     header.erase( pos, std::string::npos );
}


void httpTail( std::string& dest, const std::string& hostname, const int portnum, const std::string& authData,
               const std::string& body )
{
     dest.append( "Host: " + hostname + ":" + std::to_string(portnum) + "\r\n" );
     dest.append( "Authorization: Basic ");

     itcs::roc::Base64Decoder().encode( dest, authData.c_str(), authData.size() );

     dest.append( "\r\n" );
     if ( !body.empty() )
     {
          dest.append( "Content-Length: " );
          dest.append( std::to_string(body.length()) );
          dest.append( "\r\n\r\n" );
          dest.append( body );
          dest.append("\r\n");
     }
     dest.append("\r\n");
}


void httpRequest( std::string& httpCode, std::string& response,
                  const std::string& hostname, const int portnum, const std::string& authData,
                  const std::string& requestBase, const std::string& requestBody)
{
     std::string request = requestBase;
     response.clear();
     httpTail( request, hostname, portnum, authData, requestBody );


     int sd;
     std::string errMsg;
     if ( openTcpConnection( sd, hostname.c_str(), portnum, errMsg ) )
     {
          write( sd, request.c_str(), request.size() );

          char    buf[ 16384 ];
          ssize_t bytes = read( sd, buf, static_cast< ssize_t >( sizeof( buf ) ) );
          if ( bytes > 0 )
          {
               response.append( buf, static_cast< std::string::size_type >( bytes ) );
          }
     }
     else
     {
          ITCS_ERROR( errMsg );
     }

     stripHeader( httpCode, response );
}


void httpsRequest( std::string& httpCode, std::string& response,
                   const std::string& hostname, const int portnum, const std::string& authData,
                   const std::string& requestBase, const std::string& requestBody )
{
     std::string request = requestBase;
     response.clear();
     httpTail( request, hostname, portnum, authData, requestBody );

     int server;
     SSL_CTX* ctx;
     std::string errMsg;
     if ( initCtx( ctx ) && openTcpConnection( server, hostname.c_str(), portnum, errMsg ) )
     {
          SSL* ssl = SSL_new( ctx ); // Создать новое состояние подключения по SSL
          SSL_set_fd( ssl, server ); // Прикрепить дескриптор сокета
          if ( SSL_connect( ssl ) == -1 ) // Подключиться
          {
               ERR_print_errors_fp( stderr );
          }
          else
          {
               // Зашифровать и отправить сообщение
               SSL_write( ssl, request.c_str(), static_cast< int >( request.length() ) );

               char buf[ 16384 ];
               int  bytes = SSL_read( ssl, buf, sizeof( buf ) );
               if ( bytes > 0 )
               {
                    // Получить ответ и расшифровать
                    response.append( buf, static_cast< std::string::size_type>( bytes ) );
               }
          }
          SSL_free( ssl ); // Освободить состояние подключения
          close( server );     // Закрыть сокет
          SSL_CTX_free( ctx ); // Освободить контекст
     }
     else if ( !errMsg.empty() )
     {
          ITCS_ERROR( errMsg );
     }

     stripHeader( httpCode, response );
}


TestCaseItemData::TestCaseItemData()
{

}


void TestCaseItemData::getBodyString( std::string& dest )
{
     dest = bodyText.empty() ? Json::FastWriter().write( body ) : bodyText;
}


TestContext::~TestContext()
{
     if ( handler )
     {
          handler->finish();
          testingThread.join();
          notifyServiceSimulator->stop();
          notifyServeThread.join();
     }
     if  ( needTestHttps )
     {
          itcs::roc::cleanupSSL();
     }
}


void TestCaseItemData::setSqlQuery( Json::Value& q )
{
     sqlQuery.clear();
     if ( q.isString() )
     {
          sqlQuery = q.asString();
     }
     else if ( q.isArray() )
     {
          for ( Json::Value& qPart : q )
          {
               if ( qPart.isString() )
               {
                    sqlQuery.append( qPart.asString() );
               }
          }
     }
}


void TestCaseItemData::initFromJson( Json::Value& it )
{
     if ( it.isMember( "sleep" ) && it[ "sleep" ].isIntegral() )
     {
          sleepTime = it[ "sleep" ].asInt();
     }
     if ( it.isMember( "uri" ) && it[ "uri" ].isString() )
     {
          uri = it[ "uri" ].asString();
     }
     if ( it.isMember( "auth" ) && it[ "auth" ].isString() )
     {
          auth = it[ "auth" ].asString();
     }
     if ( it.isMember( "body" ) )
     {
          body = it[ "body" ];
     }
     if ( it.isMember( "body_text" ) && it[ "body_text" ].isString() )
     {
          bodyText = it[ "body_text" ].asString();
          if ( it.isMember("repeats") &&  it["repeats"].isIntegral() )
          {
               std::string t = bodyText;
               Json::LargestUInt max = it["repeats"].asLargestUInt();
               for ( Json::LargestUInt i = 1; i < max; ++i )
               {
                    bodyText.append( t );
               }
          }
     }
     if ( it.isMember( "response" ) )
     {
          response = it[ "response" ];
     }
     if ( it.isMember( "method" ) && it[ "method" ].isString() )
     {
          httpMethod = it[ "method" ].asString();
     }
     if ( it.isMember( "query" ) )
     {
          setSqlQuery( it["query"] );
     }
     exitAfterError = it.isMember( "exit_after_error" );
     httpCode = JsonValueStr ::get( it, "http_code", "200 OK" );
}


bool TestContext::getTestcaseData( TestContext::TestCaseData& dest, const std::string& testcase )
{
     std::ifstream fs( testdataFile.c_str(), std::ifstream::in );
     if ( !fs.good() )
     {
          ITCS_TRACE( "ERROR: bad testdata file: " << testdataFile );
          return false;
     }

     Json::Value val;
     Json::Reader().parse( fs, val );
     if ( !val.isMember( "testcases" ) || !val[ "testcases" ].isObject() || !val[ "testcases" ].isMember( testcase )
          || !val[ "testcases" ][ testcase ].isMember( "testdata" )
          || !val[ "testcases" ][ testcase ][ "testdata" ].isArray() )
     {
          ITCS_ERROR( "ERROR: testdata file has not testcase " << testcase << " data" );
          return false;
     }

     Json::Value testCaseData = val[ "testcases" ][ testcase ][ "testdata" ];
     for ( Json::Value& it : testCaseData )
     {
          TestCaseItemData item;
          item.initFromJson( it );
          if ( !it.isMember( "query_list" ) || !it[ "query_list" ].isArray() )
          {
               dest.push_back( item );
          }
          else
          {
               Json::Value& qList = it[ "query_list" ];
               for ( Json::Value& query : qList )
               {
                    item.setSqlQuery( query );
                    dest.push_back( item );
               }
          }

     }
     return true;
}


} // namespace roc
} // namespace itcs


void SetupTests::initDb( const std::string& testdataFile, const std::string& initModelFile, const std::string& psql )
{
     std::vector< std::string > commands =
     {
          psql + " -U postgres -f " + initModelFile,
          psql + " -U roc_service -d rollout -f " + testdataFile
     };

     for ( std::string& cmd : commands )
     {
          int code = system( cmd.c_str() );
          if ( code )
          {
               ITCS_TRACE( "initialize db with test data failed. code= " << code << " command= " << cmd );
          }
     }
}


void SetupTests::setBoolValue( bool& dest, const std::string& val )
{
     dest = !val.empty() && val != "0";
}


void SetupTests::setStrValue( std::string& dest, const std::string& val )
{
     dest = val;
}


void SetupTests::setIntValue( int& dest, const std::string& val )
{
     dest = atoi( val.c_str() );
}


void testHttpHandlerLoop()
{
     try
     {
          SetupTests::context.handler->mainLoop();
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "testHttpHandlerLoop() - exception: " << err.what() << "\n" );
     }
}

void listenNotify()
{
     try
     {
          SetupTests::context.notifyServiceSimulator->serve();
     }
     catch ( apache::thrift::TException& exc )
     {
          ITCS_ERROR( "Notify simulator service error:" << exc.what() );
     }
}


SetupTests::SetupTests()
{
     std::string initModelFile = "/mnt/main/opt/itcs/initial_create.sql";
     std::string testdataFile  = "/mnt/main/opt/itcs/testdata.sql";
     std::string psql          = "/opt/itcs/bin/psql";
     bool needInitDb = false;

     using namespace std::placeholders;

     typedef std::pair< const char*, std::function< void( const std::string& ) > > ConfigItem;
     std::vector< ConfigItem > envConfig =
     {
          // если установлена, переинициализировать БД
          { "ROC_TEST_NEED_INIT_DB"      , std::bind( setBoolValue, std::ref( needInitDb ), _1 ) },
          { "ROC_TEST_INIT_MODEL_FILE"   , std::bind( setStrValue,  std::ref( initModelFile ), _1 ) },
          { "ROC_TEST_INIT_TESTDATA_FILE", std::bind( setStrValue,  std::ref( testdataFile ), _1 ) },
          { "ROC_TEST_PSQL_PATH"         , std::bind( setStrValue,  std::ref( psql ), _1 ) },
          { "ROC_TEST_HTTPS"             , std::bind( setBoolValue, std::ref( context.needTestHttps ), _1 ) },
          { "ROC_TEST_HTTP"              , std::bind( setBoolValue, std::ref( context.needTestHttp ), _1 ) },
          { "ROC_TEST_PARSER"            , std::bind( setBoolValue, std::ref( context.needTestParser ), _1 ) },
          { "ROC_TEST_CONFIG_FILE"       , std::bind( setStrValue,  std::ref( context.configFilename ), _1 ) },
          { "ROC_TEST_HOST"              , std::bind( setStrValue,  std::ref( context.host ), _1 ) },
          { "ROC_TEST_PORT"              , std::bind( setIntValue,  std::ref( context.port ), _1 ) },
          { "ROC_TEST_DATAFILE"          , std::bind( setStrValue,  std::ref( context.testdataFile ), _1 ) },
          { "ROC_TEST_SIMULATE_ONLY"     , std::bind( setBoolValue, std::ref( context.simulateInterfacesOnly ), _1 ) }
     };

     // itcs::roc::initLogger( ITCS_LOGLEVEL_TRACE );
     itcs::roc::initLogger( ITCS_LOGLEVEL_INFO );

     for ( ConfigItem& item : envConfig )
     {
          char* v = getenv( item.first );
          if ( v && strlen( v ) )
          {
               item.second( v );
          }
     }

     // Базу нужно переинициализировать до запуска всего, что ее использует
     if ( needInitDb )
     {
          initDb( testdataFile, initModelFile, psql );
     }


     using itcs::roc::HttpHandler;
     itcs::roc::TestContext& context = SetupTests::context;
     context.handler                = std::make_shared< HttpHandler >( SetupTests::context.configFilename );
     context.notityThriftPort       = context.handler->config().notifyPort();
     context.notifyServiceSimulator = itcs::roc::NotifyServiceSimulator::getListener( context.notityThriftPort );

     context.notifyServeThread = std::thread( &listenNotify );
     std::this_thread::sleep_for( std::chrono::seconds( 1 ) );

     if ( context.simulateInterfacesOnly )
     {
          context.nsmsServeThread.join();
          exit( 0 );
     }

     if  ( context.needTestHttps )
     {
          itcs::roc::initSSL();
     }
     if ( context.needTestHttps || context.needTestHttp )
     {
          context.testingThread = std::thread( &testHttpHandlerLoop );
          std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
     }
}


SetupTests::~SetupTests()
{
}


