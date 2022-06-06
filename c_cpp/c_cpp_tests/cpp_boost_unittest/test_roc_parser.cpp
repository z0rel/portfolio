#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE roc_test


#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#include <boost/test/unit_test.hpp>

#pragma GCC diagnostic pop

#include <json_utils.h>
#include <base_interfaces/uni_logger_module.h>
#include <roc_parser.h>
#include <roc_methods.h>
#include <http_handler_settings.h>
#include <uni_log_impl.h>
#include <http_handler.h>
#include <test_utils.h>
#include <roc_nsms_worker.h>


itcs::roc::TestContext SetupTests::context;


namespace itcs
{
namespace roc
{


bool checkResponseJson( const std::string &testcase, int testnum, TestCaseItemData& item,
                        const std::string& httpCodeResponse, std::string& realResponseStr )
{
     Json::Value realResponse;
     bool result = true;
     bool parseResult = Json::Reader().parse( realResponseStr, realResponse, false );
     if ( realResponse.isObject() && realResponse.isMember("eta") )
     {
          realResponse["eta"] = "";
     }
     if ( item.response.isObject() && item.response.isMember("eta") )
     {
          item.response["eta"] = "";
     }

     if ( item.httpCode != httpCodeResponse )
     {
          ITCS_WARNING( "ERROR: testcase " << testcase << " testnum: " << testnum );
          ITCS_WARNING( "       auth: "    << item.auth );
          ITCS_WARNING( "       uri: "     << item.uri );
          ITCS_WARNING( "       wait http code=" << item.httpCode );
          ITCS_WARNING( "       real http code=" << httpCodeResponse  );
          result = false;
     }
     if ( !parseResult )
     {
          if ( item.response.isString() && item.response.asString().empty() )
          {
               return true;
          }
          else
          {
               ITCS_WARNING( "ERROR: testcase " << testcase << " testnum: " << testnum );
               ITCS_WARNING( "       auth: " << item.auth );
               ITCS_WARNING( "       uri: " << item.uri );
               ITCS_WARNING( "       response JSON is invalid: " );
               ITCS_WARNING( realResponseStr );
               result = false;
          }
     }
     if ( item.response != realResponse )
     {
          ITCS_WARNING( "ERROR: testcase " << testcase << " testnum: " << testnum );
          ITCS_WARNING( "       auth: " << item.auth );
          ITCS_WARNING( "       uri: " << item.uri );
          ITCS_WARNING( "       wait str=" << Json::FastWriter().write( item.response ) );
          ITCS_WARNING( "       real str=" << realResponseStr  );
          result = false;
     }
     if ( !result && item.exitAfterError )
     {
          // В отладочных целях, выйти после падения на этом тесте
          exit( 0 );
     }
     return result;
}


bool testRocParser( const std::string& httpMethod, CathegoryRocCommand::T cmd,
                    TestContext::TestCaseData& testData,  const std::string& jsonTestCase, bool notCheckCathegory )
{
     using std::string;

     bool isOk = true;
     int caseItemNum = 0;
     for ( TestCaseItemData &testdataIt : testData )
     {
          ++caseItemNum;
          if ( testdataIt.sleepTime )
          {
               sleep( testdataIt.sleepTime );
               continue;
          }

          std::shared_ptr< HttpHandlerSettings > config = std::make_shared< HttpHandlerSettings >( "" );
          RocNsmsWorker nsmsWorker( config );
          nsmsWorker.start();
          RocMethodsContext ctx( config, nsmsWorker );

          testdataIt.getBodyString( ctx.requestBody() );

          string s = string( 1, SvalConvert::httpMethodToken( SvalConvert::httpMethodTokenId( httpMethod ) ) )
                     + testdataIt.uri;
          ITCS_TRACE( " " << httpMethod << " " << s );
          CathegoryRocCommand::T parsedCmd = ctx.getParser()->parseURI( s );

          ITCS_TRACE( "parsed command id=" << parsedCmd );
          if ( notCheckCathegory || parsedCmd == cmd )
          {
               if ( !ctx.success() )
               {
                    ITCS_WARNING( "ERROR: last command fail (success = false)" );
               }
               if ( !checkResponseJson( jsonTestCase, caseItemNum, testdataIt, "", ctx.response() ) )
               {
                    isOk = false;
               }
          }
     }
     return isOk;
}


bool testRocHttp( const std::string& httpMethod, TestContext::TestCaseData& testData, const std::string& jsonTestCase )
{
     auto testFun = ( SetupTests::context.needTestHttps ? httpsRequest : httpRequest );

     bool isOk = true;
     int i = 0;
     for ( TestCaseItemData& testIt : testData )
     {
          i++;

          if ( testIt.sleepTime )
          {
               sleep( testIt.sleepTime );
               continue;
          }

          std::string request = ( testIt.httpMethod.empty() ? httpMethod : testIt.httpMethod ) + " " + testIt.uri +
                                " HTTP/1.1\r\n";
          std::string httpCodeResponse;
          std::string response;
          std::string bodyString;
          testIt.getBodyString( bodyString );

          testFun( httpCodeResponse, response, SetupTests::context.host, SetupTests::context.port, testIt.auth,
                   request, bodyString );
          if ( !checkResponseJson( jsonTestCase, i, testIt, httpCodeResponse, response ) )
          {
               isOk = false;
          }
     }
     return isOk;
}


bool execTestRoc( const std::string& httpMethod, const std::string& testcaseId, TestContext::TestCaseData& testData,
                  CathegoryRocCommand::T cmd, bool notCheckCathegory )
{
     if ( SetupTests::context.needTestParser )
     {
          return testRocParser( httpMethod, cmd, testData, testcaseId, notCheckCathegory );
     }
     else if ( SetupTests::context.needTestHttps || SetupTests::context.needTestHttp )
     {
          return testRocHttp( httpMethod, testData, testcaseId );
     }
     return false;
}


bool testRoc( const std::string& httpMethod, const std::string& jsonTestCase, CathegoryRocCommand::T cmd )
{
     std::string testcaseId = httpMethod + " " + jsonTestCase;
     ITCS_TRACE( "Run test case " << testcaseId );
     TestContext::TestCaseData testData;
     SetupTests::context.getTestcaseData( testData, testcaseId );
     return execTestRoc( httpMethod, testcaseId, testData, cmd, false );
}


bool testBigPostBody( const std::string& caseName )
{
     TestContext::TestCaseData testData;
     SetupTests::context.getTestcaseData( testData, caseName );
     ITCS_TRACE( "Run test case " << caseName );
     return execTestRoc( "", caseName, testData, CathegoryRocCommand::UNKNOWN, true );
}


} // namespace roc
} // namespace itcs


using itcs::roc::testRoc;
using itcs::roc::testBigPostBody;
using itcs::roc::CathegoryRocCommand;

typedef itcs::roc::CathegoryRocCommand RocCmd;

BOOST_AUTO_TEST_SUITE( test_parser_suite )

BOOST_GLOBAL_FIXTURE( SetupTests )

BOOST_AUTO_TEST_CASE( roc_test_get_users_list )
{
     // Получить список пользователей организации
     BOOST_CHECK( testRoc("GET", "/roc/api/v1/company/{company-id}/users", RocCmd::GET_USERS_LIST) );
}

BOOST_AUTO_TEST_CASE( roc_test_bad_uri )
{
     // Проверить обработку некорректного uri-адреса
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/incorrect_query", RocCmd::UNKNOWN ) );
}

BOOST_AUTO_TEST_CASE( roc_test_add_user )
{
     // Добавить пользователя
     BOOST_CHECK( testRoc( "POST", "/roc/api/v1/company/{company-id}/users", RocCmd::POST_USER ) );
}

BOOST_AUTO_TEST_CASE( roc_test_bigdata_body )
{
     BOOST_CHECK( testBigPostBody( "bigdata case" ) );
}

BOOST_AUTO_TEST_CASE( roc_test_get_self_info )
{
     // Получить информацию о пользователе
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/me", RocCmd::GET_INFO_ME ) );
}

BOOST_AUTO_TEST_CASE( roc_test_get_user_info )
{
     // Получить информацию о пользователе
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/company/{company-id}/users/{user-id}", RocCmd::GET_USER_INFO ) );
}

BOOST_AUTO_TEST_CASE( roc_test_add_device )
{
     // Добавить устройство пользователя
     BOOST_CHECK( testRoc( "POST", "/roc/api/v1/company/{company-id}/users/{user-id}/devices",
                           RocCmd::POST_DEVICE ) );
}

BOOST_AUTO_TEST_CASE( roc_test_get_devices_list )
{
     // Получить список устройств пользователя
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/company/{company-id}/users/{user-id}/devices",
                           RocCmd::GET_DEVICES ) );
}

BOOST_AUTO_TEST_CASE( roc_test_get_device_info )
{
     // Получить информацию об устройстве
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/company/{company-id}/users/{user-id}/devices/{device-id}",
                           RocCmd::GET_DEVICE_INFO ) );
}

BOOST_AUTO_TEST_CASE( roc_test_update_user )
{
     // Обновить информацию о пользователе
     BOOST_CHECK( testRoc( "PUT", "/roc/api/v1/company/{company-id}/users", RocCmd::PUT_USER ) );
}

BOOST_AUTO_TEST_CASE( roc_test_delete_user )
{
     // Обновить информацию о пользователе
     BOOST_CHECK( testRoc( "DELETE", "/roc/api/v1/company/{company-id}/users/{user-id}", RocCmd::DELETE_USER ) );
     BOOST_CHECK( testRoc( "POST", "/roc/api/v1/company/{company-id}/users/{user-id}?method=delete",
                           RocCmd::DELETE_USER ) );
}


BOOST_AUTO_TEST_CASE( roc_test_get_company )
{
     // Получить информацию о компании
     BOOST_CHECK( testRoc( "GET", "/roc/api/v1/company/{company-id}", RocCmd::GET_COMPANY ) );
}


BOOST_AUTO_TEST_CASE( roc_test_get_dst )
{
     // Получить DST файл для устройства
     BOOST_CHECK( testRoc( "GET", "/company/{company-id}/users/{user-id}/devices/{device-id}/dstx",
                           RocCmd::GET_DSTX ) );
}


BOOST_AUTO_TEST_SUITE_END()
