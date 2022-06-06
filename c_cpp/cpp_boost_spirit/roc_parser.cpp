#include <roc_parser.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/type_traits.hpp>

#pragma GCC diagnostic pop


#include <functional>
#include <iostream>
#include <iostream>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include <roc_methods.h>
#include <base_interfaces/uni_logger_module.h>
#include <roc_fcgi_utils.h>
#include <http_handler_settings.h>

namespace itcs
{
namespace roc
{


namespace qi     = boost::spirit::qi;
namespace spirit = boost::spirit;
namespace phx    = boost::phoenix;

const std::string rocApiVersion = "/roc/api/v1";


template < typename T >
struct MakeSharedF
{
     template < typename... A >
     struct result
     {
          typedef std::shared_ptr< T > type;
     };


     template < typename... A >
     typename result< A... >::type operator()( A&&... a ) const
     {
          return std::make_shared< T >( std::forward< A >( a )... );
     }
};


template < typename T >
using PhoenixMakeShared = phx::function< MakeSharedF< T > >;


struct AssignValue
{
     template < typename T, typename M, typename V >
     struct result
     {
          typedef void type;
     };


     template < typename T, typename M, typename V >
     void operator()( T* val, M m, V storedVal ) const
     {
          ( val->*m ) = storedVal;
     }

     template < class T, typename M, typename V >
     void operator()( T* val, std::unique_ptr< M > T::*m, V storedVal ) const
     {
          if ( !( val->*m ) )
          {
               ( val->*m ) = std::unique_ptr< M >( new M( storedVal ) );
          }
          else
          {
               *( val->*m ) = storedVal;
          }
     }

     template < typename T, typename M, typename V >
     void operator()( std::reference_wrapper< std::shared_ptr< T > > val, M m, V storedVal ) const
     {
          if ( !val.get() )
          {
               val.get() = std::make_shared< T >( storedVal );
          }
          else
          {
               ( val.get().get()->*m ) = storedVal;
          }
     }
};

using PhxAssignValue = phx::function< AssignValue >;



struct SetLimit
{
     template < typename T, typename V >
     struct result
     {
          typedef void type;
     };


     template < typename T, typename V >
     void operator()( std::reference_wrapper< T > val, V limitVal ) const
     {
          val.get().limit = limitVal;
          val.get().limitIsSet = true;
     }
};


struct SwapValue
{
     template < typename T, typename V >
     struct result
     {
          typedef void type;
     };


     template < typename T, typename V >
     void operator()( std::reference_wrapper< T > val, V& m ) const
     {
          val.get().swap( m );
     }
};

using PhxSwapValue = phx::function< SwapValue >;


struct LogUnknownParams
{
     template < typename K, typename V >
     struct result
     {
          typedef void type;
     };


     template < typename K, typename V >
     void operator()( const K& key, V& val ) const
     {
          if ( val )
          {
               ITCS_INFO( "Unknown http params: " << key << "=" << val.get() );
          }
          else
          {
               ITCS_INFO( "Unknown http params: " << key );
          }
     }
};


template < unsigned int MaxBit >
struct MaskToStrList
{
     template < typename T, typename F >
     static void convert( std::string& dest, T container, F fun )
     {
          typedef typename boost::function_traits< typename boost::remove_pointer< F >::type >::arg1_type arg1_type;

          if ( !container )
          {
               return;
          }

          bool isFirst = true;
          for ( unsigned int i = 0; i < MaxBit; ++i )
          {
               unsigned int bit = 1 << i;
               if ( container & bit )
               {
                    if ( !isFirst )
                    {
                         dest.append( "," );
                    }
                    else
                    {
                         isFirst = false;
                    }
                    std::string str = fun( static_cast< arg1_type >( bit ) );
                    assert( !str.empty() );
                    dest.append( str );
               }
          }
     }
};


class RocParser;


struct RocGrammar
{
     explicit RocGrammar( RocParser& svalue );

     typedef unsigned int                UInt;
     typedef std::string::const_iterator Iterator;
     typedef qi::rule< Iterator, qi::unused_type() > Rule;
     typedef qi::rule< Iterator, unsigned int() >    RuleUint;
     typedef qi::rule< Iterator, std::string() >     RuleString;

     typedef qi::rule< Iterator, char() >            RuleChar;
     typedef qi::symbols< char, CathegoryOs::T >     SymOs;
     typedef qi::symbols< char, CathegoryStatus::T > SymStatus;
     typedef qi::symbols< char, UInt >               SymFields;

     RuleChar   uuidHc             = RuleChar();
     RuleChar   b64Hc              = RuleChar();
     RuleChar   fieldValHc         = RuleChar();
     RuleChar   fieldValEscHc      = RuleChar();
     RuleString accessTokFilter    = RuleString();
     RuleString ampAccessTokFilter = RuleString();
     RuleString accessTok          = RuleString();
     RuleString unknownUriParam    = RuleString();
     RuleString unknownVal         = RuleString();
     RuleUint   uuid               = RuleUint();
     RuleString fieldVal           = RuleString();
     Rule       companyId          = Rule();
     Rule       deviceId           = Rule();
     Rule       userId             = Rule();
     Rule       devName            = Rule();
     Rule       devHwid            = Rule();
     Rule       getDevFilter       = Rule();
     Rule       getDevFilterItem   = Rule();

     Rule       getLogFilter       = Rule();
     Rule       getLogFilterItem   = Rule();

     Rule       filterOs           = Rule();
     Rule       filterUsrStatus    = Rule();
     Rule       filterEventType    = Rule();
     Rule       usrText            = Rule();

     Rule       logFromTime        = Rule();
     Rule       logToTime          = Rule();
     Rule       logUserId          = Rule();
     Rule       logLogin           = Rule();

     Rule       filterLimit        = Rule();
     Rule       filterOffset       = Rule();
     Rule       filterUsrFields    = Rule();
     Rule       filterDevFields    = Rule();
     Rule       filterLogFields    = Rule();
     Rule       getUsersFilter     = Rule();
     Rule       getUsersFilterItem = Rule();
     Rule       getCommonItem      = Rule();
     Rule       uriParams          = Rule();
     RuleUint   off                = RuleUint();
     RuleUint   lim                = RuleUint();
     Rule       userSpec           = Rule();
     Rule       deviceSpec         = Rule();
     Rule       get                = Rule();
     Rule       post               = Rule();
     Rule       del                = Rule();
     Rule       put                = Rule();
     Rule       start              = Rule();
     Rule       company            = Rule();
     Rule       dstx               = Rule();
     Rule       ver                = Rule();

     SymStatus  symStatus          = SymStatus();
     SymOs      symOs              = SymOs();
     SymFields  symUserObjFields   = SymFields();
     SymFields  symDeviceObjFields = SymFields();
     SymFields  symEvent           = SymFields();
     SymFields  symLogFields       = SymFields();

     RocParser& svalue;
};


RocParser::~RocParser()
{
}


RocParser::RocParser( RocMethodsContext& ctx ) : rocMethods_( ctx ), g_( new RocGrammar( *this ) )
{
}


RocGrammar::RocGrammar( RocParser& ctx ) : svalue( ctx )
{
     try
     {
          using boost::spirit::qi::eoi;
          using boost::spirit::_val;
          using boost::spirit::qi::lit;
          using boost::spirit::qi::char_;
          using boost::spirit::qi::repeat;
          using boost::spirit::qi::eps;

          start =
               // GET Получить информацию о пользователе (объект User):
               // /me
                 ( get >> ver >> lit( "/me" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::getSelfInfo, &svalue )
                    ]
               // GET Подтвердить почту пользователя - выполнение
               // /confirm_email?code={accessTok}
               | ( get >> lit( "/confirm_email?code=" ) >> accessTok >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::confirmEmail, &svalue )
                    ]
               // POST Запросить сброс пароля пользователя - отправить ссылку
               // /password_reset_request
               | ( post >> ver >> lit( "/password_reset_request" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::sendPasswordResetUri, &svalue )
                    ]
               // GET Обработать сброс пароля пользователя
               // /restore_password?access_token={accessTok}
               | ( get >> ver >> lit( "/restore_password?" ) >> accessTokFilter >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::restorePassword, &svalue )
                    ]
               // POST Получить токен для фактического сброса пароля (res = 201
               // Created|400 bad request)
               // /tokens
               | ( post >> ver >> lit( "/tokens" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::postPasswordResetTokens, &svalue )
                    ]
               // POST Задать пароля пользователя (res = 200 Ok | 400 bad request)
               // /{user-id}/password
               | ( post >> ver >> '/' >> userId >> '/' >> lit( "password" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::setUserPassword, &svalue )
                    ]
               // PUT Обновить информацию о пользователе
               // /company/{company-id}/users/{user-id}
               | ( put >> ver >> company >> userSpec >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::updateUserInfo, &svalue )
                    ]
               // DELETE Удаление элементов
               // /company/{company-id}/users/{user-id}
               | ( del >> ver >> company >> userSpec >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::deleteUser, &svalue )
                    ]
               // POST Удаление элементов
               // /company/{company-id}/users/{user-id}?method=delete&
               | ( post >> ver >> company >> userSpec >> '?' >> ( ( lit( "method=delete" ) | unknownUriParam ) % '&' )
                   >> eoi )
                    [
                         phx::bind( &RocParser::deleteUser, &svalue )
                    ]
               // GET Получить объект Company (obj Company)
               // /company/{company-id}
               | ( get >> ver >> company >> '/' >> companyId >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::getCompany, &svalue )
                    ]
               // GET Получить лог событий организации ([obj LogRecords]):
               // /{company-id}/events
               | ( get >> ver >> company >> '/' >> companyId >> lit( "/events" ) >> getLogFilter >> eoi )
                    [
                         phx::bind( &RocParser::getLogRecords, &svalue )
                    ]
               // GET  Получить список пользователей организации (список объектов User):
               // /company/{company-id}/users
               | ( get >> ver >> company >> '/' >> companyId >> lit( "/users" ) >> getUsersFilter  >> eoi )
                    [
                         phx::bind( &RocParser::getUserList, &svalue )
                    ]
               // POST Добавить пользователя (JSON объект User):
               // /company/{company-id}/users
               | ( post >> ver >> company >> '/' >> companyId >> lit( "/users" ) >> uriParams >> eoi )
                   [
                        phx::bind( &RocParser::addUser, &svalue )
                   ]
               // GET Получить информацию о пользователе (объект User):
               // /company/{company-id}/users/{user-id}
               | ( get >> ver >> company >> userSpec >> uriParams >> eoi )
                   [
                        phx::bind( &RocParser::getUserInfo, &svalue )
                   ]
               // POST Выслать пользователю информацию по получению ключевой информации
               // для устройства
               // /company/{company-id}/users/{user-id}/notify
               | ( post >> ver >> company >> userSpec >> lit( "/notify" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::sendNotify, &svalue )
                    ]
               // GET  Получить список устройств пользователя ([obj Device])
               // /company/{company-id}/users/{user-id}/devices
               | ( get >> ver >> company >> userSpec >> lit( "/devices" ) >> getDevFilter >> eoi )
                    [
                         phx::bind( &RocParser::getDevices, &svalue )
                    ]
               // POST Добавить устройство пользователя (obj Device):
               // /company/{company-id}/users/{user-id}/devices
               | ( post >> ver >> company >> userSpec >> lit( "/devices" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::addDevice, &svalue )
                    ]
               // GET Получить информацию об устройстве (Device):
               // /company/{company-id}/users/{user-id}/devices/{device-id}
               | ( get >> ver >> company >> deviceSpec >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::getDeviceInfo, &svalue )
                    ]
               // GET Получить DST файл для устройства:
               // /company/{company-id}/users/{user-id}/devices/{device-id}/dstx
               | ( get >> ver >> company >> deviceSpec >> dstx >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::getDstx, &svalue )
                    ]
               // GET Получить статус подготовки DST файла для устройства
               // /company/{company-id}/users/{user-id}/devices/{device-id}/dstx/status
               | ( get >> ver >> company >> deviceSpec >> dstx >> lit( "/status" ) >> uriParams >> eoi )
                    [
                         phx::bind( &RocParser::getDstxStatus, &svalue )
                    ];

          userSpec   = ( '/' >> companyId >> lit( "/users/" ) >> userId );
          deviceSpec = ( '/' >> companyId >> lit( "/users/" ) >> userId >> lit( "/devices/" ) >> deviceId );

          companyId = uuid[ phx::ref( svalue.sval_.companyId ) = qi::_1 ];
          deviceId  = uuid[ phx::ref( svalue.sval_.deviceId  ) = qi::_1 ];
          userId    = uuid[ phx::ref( svalue.sval_.userId    ) = qi::_1 ];

          get  = SvalConvert::httpMethodToken( HttpMethodTok::GET );
          post = SvalConvert::httpMethodToken( HttpMethodTok::POST );
          del  = SvalConvert::httpMethodToken( HttpMethodTok::DELETE );
          put  = SvalConvert::httpMethodToken( HttpMethodTok::PUT );

          company = lit( "/company" );
          dstx    = lit( "/dstx" );
          ver     = lit( rocApiVersion );
          uuidHc  = char_( "0-9" );
          b64Hc   = char_( "-_0-9a-zA-Z" );

          uuid = qi::uint_;

          accessTok          = repeat( 2, 1000 )[ b64Hc ];
          accessTokFilter    = lit( "access_token=" ) >> accessTok;
          ampAccessTokFilter = '&' >> accessTokFilter;

          getUsersFilter = ( '?' >> ( getUsersFilterItem % '&' ) ) | eps;

          uriParams = ( '?' >> ( unknownUriParam % '&' ) ) | eps;

          getCommonItem =
                 filterLimit
               | filterOffset
               | unknownUriParam
               ;

          getUsersFilterItem =
                 usrText
               | filterOs
               | filterUsrStatus
               | filterUsrFields
               | getCommonItem
               ;

          getDevFilter = ( '?' >> ( getDevFilterItem % '&' ) ) | eps;

          getDevFilterItem =
                 devName
               | devHwid
               | filterOs
               | filterDevFields
               | getCommonItem
               ;

          getLogFilter = ( '?' >> ( getLogFilter % '&' ) ) | eps;

          getLogFilterItem =
                 logFromTime
               | logToTime
               | logUserId
               | filterEventType
               | logLogin
               | filterOs
               | filterLogFields
               | getCommonItem
               ;

          devName = ( lit( "filter_name=" ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterText_ ), qi::_1 ) ];
          devHwid = ( lit( "filter_hwid=" ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterHwid_ ), qi::_1 ) ];
          usrText = ( lit( "filter_text=" ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterText_ ), qi::_1 ) ];

          logFromTime = ( lit( "from_time=" ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterLogFrom_ ), qi::_1 ) ];
          logToTime   = ( lit( "to_time="   ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterLogTo_   ), qi::_1 ) ];
          logLogin    = ( lit( "filter_login=" ) >> fieldVal ) [ PhxSwapValue()( ref( svalue.filterHwid_ ), qi::_1 ) ];
          logUserId   = ( lit( "user_id=" ) >> userId );

          filterOs = ( lit( "filter_os=" )[ phx::ref( svalue.sval_.filterOs ) = 0 ]
                          >> ( symOs[ phx::ref( svalue.sval_.filterOs ) |= qi::_1 ] % ',' ) );

          filterUsrStatus = ( lit( "filter_status=" )[ phx::ref( svalue.sval_.filterStatus ) = 0 ]
                              >> ( symStatus[ phx::ref( svalue.sval_.filterStatus ) |= qi::_1 ] % ',' ) );

          filterEventType = ( lit( "filter_event_type=" )[ phx::ref( svalue.sval_.eventTypes ) = 0 ]
                              >> ( symStatus[ phx::ref( svalue.sval_.eventTypes ) |= qi::_1 ] % ',' ) );

          using Ctx = RocParserIntegralSValue;

          filterUsrFields = ( lit( "fields=" ) [ phx::ref( svalue.sval_.userFilelds ) = Ctx::DEFAULT_USER_FIELDS ]
                              >> ( symUserObjFields[ phx::ref( svalue.sval_.userFilelds ) |= qi::_1 ] % ',' ) );

          filterDevFields = ( lit( "fields=" )[ phx::ref( svalue.sval_.deviceFields ) = Ctx::DEFAULT_DEVICE_FIELDS ]
                              >> ( symDeviceObjFields[ phx::ref( svalue.sval_.deviceFields ) |= qi::_1 ] % ',' ) );

          filterLogFields = ( lit( "fields=" )[ phx::ref( svalue.sval_.logFields ) = Ctx::DEFAULT_LOG_FIELDS ]
                              >> ( symLogFields[ phx::ref( svalue.sval_.logFields ) |= qi::_1 ] % ',' ) );

          filterLimit = lim[ phx::function< SetLimit >()( std::ref( svalue.sval_ ), qi::_1 ) ];

          filterOffset = off[ phx::ref( svalue.sval_.offset ) = qi::_1 ];

          symOs.add
               ( SvalConvert::os( CathegoryOs::IOS_ANY     ), CathegoryOs::IOS_ANY     )
               ( SvalConvert::os( CathegoryOs::ANDROID_ANY ), CathegoryOs::ANDROID_ANY )
               ( SvalConvert::os( CathegoryOs::WINDOWS_ANY ), CathegoryOs::WINDOWS_ANY )
               ( SvalConvert::os( CathegoryOs::MACOS_ANY   ), CathegoryOs::MACOS_ANY   );

          symStatus.add
               ( SvalConvert::status( CathegoryStatus::NEW         ), CathegoryStatus::NEW         )
               ( SvalConvert::status( CathegoryStatus::IN_PROGRESS ), CathegoryStatus::IN_PROGRESS )
               ( SvalConvert::status( CathegoryStatus::DST_SENT    ), CathegoryStatus::DST_SENT    )
               ( SvalConvert::status( CathegoryStatus::BLOCKED     ), CathegoryStatus::BLOCKED     );

          symUserObjFields.add
               ( SvalConvert::userFields( FilterUserFields::ID            ), FilterUserFields::ID            )
               ( SvalConvert::userFields( FilterUserFields::LOGIN         ), FilterUserFields::LOGIN         )
               ( SvalConvert::userFields( FilterUserFields::EMAIL         ), FilterUserFields::EMAIL         )
               ( SvalConvert::userFields( FilterUserFields::PHONE         ), FilterUserFields::PHONE         )
               ( SvalConvert::userFields( FilterUserFields::DESCRIPTION   ), FilterUserFields::DESCRIPTION   )
               ( SvalConvert::userFields( FilterUserFields::STATUS        ), FilterUserFields::STATUS        )
               ( SvalConvert::userFields( FilterUserFields::COMPANY_ROLES ), FilterUserFields::COMPANY_ROLES );

          symDeviceObjFields.add
               ( SvalConvert::deviceFields( FilterDeviceFields::ID   ), FilterDeviceFields::ID   )
               ( SvalConvert::deviceFields( FilterDeviceFields::HWID ), FilterDeviceFields::HWID )
               ( SvalConvert::deviceFields( FilterDeviceFields::NAME ), FilterDeviceFields::NAME )
               ( SvalConvert::deviceFields( FilterDeviceFields::OS   ), FilterDeviceFields::OS   );

          symLogFields.add
               ( SvalConvert::logFields( FilterLogFields::TIME       ), FilterLogFields::TIME       )
               ( SvalConvert::logFields( FilterLogFields::ACTOR      ), FilterLogFields::ACTOR      )
               ( SvalConvert::logFields( FilterLogFields::EVENT_TYPE ), FilterLogFields::EVENT_TYPE )
               ( SvalConvert::logFields( FilterLogFields::USER_ID    ), FilterLogFields::USER_ID    )
               ( SvalConvert::logFields( FilterLogFields::USER_EMAIL ), FilterLogFields::USER_EMAIL )
               ( SvalConvert::logFields( FilterLogFields::USER_LOGIN ), FilterLogFields::USER_LOGIN )
               ( SvalConvert::logFields( FilterLogFields::DEVICE_OS  ), FilterLogFields::DEVICE_OS  );

          symEvent.add
               ( SvalConvert::event( CathegoryEvent::CREATE_USER   ), CathegoryEvent::CREATE_USER   )
               ( SvalConvert::event( CathegoryEvent::CREATE_DEVICE ), CathegoryEvent::CREATE_DEVICE )
               ( SvalConvert::event( CathegoryEvent::DST_REQUEST   ), CathegoryEvent::DST_REQUEST   )
               ( SvalConvert::event( CathegoryEvent::EDIT_USER     ), CathegoryEvent::EDIT_USER     )
               ( SvalConvert::event( CathegoryEvent::REMOVE_USER   ), CathegoryEvent::REMOVE_USER   )
               ( SvalConvert::event( CathegoryEvent::BLOCK_USER    ), CathegoryEvent::BLOCK_USER    )
               ( SvalConvert::event( CathegoryEvent::UNBLOCK_USER  ), CathegoryEvent::UNBLOCK_USER  )
               ( SvalConvert::event( CathegoryEvent::EDIT_DEVICE   ), CathegoryEvent::EDIT_DEVICE   )
               ( SvalConvert::event( CathegoryEvent::REMOVE_DEVICE ), CathegoryEvent::REMOVE_DEVICE );

          fieldValEscHc = lit( "\\&" )[ _val = '&' ] | lit( "\\\\" )[ _val = '\\' ];

          fieldValHc = ~qi::char_( "&\\" ) | fieldValEscHc;

          fieldVal = repeat( 1, 1000 )[ fieldValHc ];

          lim = ( lit( "limit=" ) >> qi::uint_ )[ _val = qi::_1 ];
          off = ( lit( "offset=" ) >> qi::uint_ )[ _val = qi::_1 ];

          unknownVal      = repeat( 1, 1000 )[ ~qi::char_( "=&" ) ];

          using LogUnknown = phx::function< LogUnknownParams >;

          unknownUriParam =
                   ( unknownVal >> ( "=" >> unknownVal | eps ) ) [ LogUnknown()( qi::_1, qi::_2 ) ]
               ;

     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: RocGrammar::RocGrammar() - exception: " << err.what() << "\n" );
     }
}


void RocParser::clearSemanticValues()
{
     filterLogFrom_.clear();
     filterLogTo_  .clear();
     filterText_.clear();
     filterHwid_.clear();
     sval_ = RocParserIntegralSValue();
     result_ = CathegoryRocCommand::UNKNOWN;
}


CathegoryRocCommand::T parseURI( std::string& str, RocMethodsContext& ctx )
{
     return ctx.getParser()->parseURI( str );
}


CathegoryRocCommand::T RocParser::parseURI( std::string& str )
{
     clearSemanticValues();

     RocGrammar::Iterator iter = str.begin();
     RocGrammar::Iterator end  = str.end();

     if ( qi::parse( iter, end, g_->start ) && iter == end )
     {
          return result_;
     }
     else
     {
          getErrorJson(rocMethods_.response(), RocErrorCodes::BAD_REQUEST);
     }

     return CathegoryRocCommand::UNKNOWN;
}


void RocParser::getUserList()
{
     std::string os;
     std::string status;
     std::string fields;

     MaskToStrList< CathegoryOs     ::MaxBit::N >::convert( os,     sval_.filterOs,     SvalConvert::os         );
     MaskToStrList< CathegoryStatus ::MaxBit::N >::convert( status, sval_.filterStatus, SvalConvert::status     );
     MaskToStrList< FilterUserFields::MaxBit::N >::convert( fields, sval_.userFilelds,  SvalConvert::userFields );

     rocMethods_.getUsersList( sval_.companyId, filterText_, os, status, sval_.limit, sval_.limitIsSet, sval_.offset,
                               fields );
     result_ = CathegoryRocCommand::GET_USERS_LIST;
}


void RocParser::addUser()
{
     ITCS_TRACE( "RocMethodsContext::getUserInfo - start" );
     rocMethods_.addUser( sval_.companyId );
     ITCS_TRACE( "RocMethodsContext::getUserInfo - end" );

     result_ = CathegoryRocCommand::POST_USER;
}


void RocParser::getSelfInfo()
{
     rocMethods_.getSelfInfo();
     result_ = CathegoryRocCommand::GET_INFO_ME;
}


void RocParser::getUserInfo()
{
     rocMethods_.getUserInfo( sval_.companyId, sval_.userId );
     result_ = CathegoryRocCommand::GET_USER_INFO;
}


void RocParser::addDevice()
{
     rocMethods_.addDevice( sval_.companyId, sval_.userId );
     result_ = CathegoryRocCommand::POST_DEVICE;
}


void RocParser::getDevices()
{
     std::string os;
     std::string fields;
     MaskToStrList< CathegoryOs::MaxBit::N >::convert( os, sval_.filterOs, SvalConvert::os );

     MaskToStrList< FilterDeviceFields::MaxBit::N >::convert( fields, sval_.deviceFields, SvalConvert::deviceFields );
     rocMethods_.getDevices( sval_.companyId, sval_.userId, filterText_, os, filterHwid_, sval_.limit,
                             sval_.limitIsSet, sval_.offset, fields );
     result_ = CathegoryRocCommand::GET_DEVICES;
}


void RocParser::getDeviceInfo()
{
     rocMethods_.getDeviceInfo( sval_.companyId, sval_.userId, sval_.deviceId );
     result_ = CathegoryRocCommand::GET_DEVICE_INFO;
}


void RocParser::getCompany()
{
     rocMethods_.getCompany( sval_.companyId );
     result_ = CathegoryRocCommand::GET_COMPANY;
}


void RocParser::updateUserInfo()
{
     rocMethods_.updateUser( sval_.companyId, sval_.userId );
     result_ = CathegoryRocCommand::PUT_USER;
}


void RocParser::deleteUser()
{
     rocMethods_.deleteUser( sval_.companyId, sval_.userId );
     result_ = CathegoryRocCommand::DELETE_USER;
}


void RocParser::getDstx()
{
     rocMethods_.getDstx( sval_.companyId, sval_.userId, sval_.deviceId );
     result_ = CathegoryRocCommand::GET_DSTX;
}


void RocParser::getDstxStatus()
{
     rocMethods_.getDstxStatus( sval_.companyId, sval_.userId, sval_.deviceId );
     result_ = CathegoryRocCommand::GET_DSTX_STATUS;
}


void RocParser::getLogRecords()
{
     result_ = CathegoryRocCommand::GET_LOG_RECORDS;
}


void RocParser::setUserPassword()
{
     result_ = CathegoryRocCommand::SET_USER_PASSWORD;
}


void RocParser::sendPasswordResetUri()
{
     result_ = CathegoryRocCommand::PASSWORD_RESET_REQUEST;
}


void RocParser::restorePassword()
{
     result_ = CathegoryRocCommand::RESTORE_PASSWORD;
}


void RocParser::postPasswordResetTokens()
{
     result_ = CathegoryRocCommand::POST_TOKENS;
}


void RocParser::confirmEmail()
{
     result_ = CathegoryRocCommand::CONFIRM_EMAIL;
}


void RocParser::sendNotify()
{
     result_ = CathegoryRocCommand::POST_NOTIFY;
}


bool SvalConvert::osFromStr( CathegoryOs::T& dest, const std::string& src )
{
     typedef std::map< std::string, CathegoryOs::T > OsValues;
     static const OsValues osValues =
     {
          { "ios-any",      CathegoryOs::IOS_ANY      },
          { "android-any",  CathegoryOs::ANDROID_ANY  },
          { "windows-any",  CathegoryOs::WINDOWS_ANY  },
          { "macos-any",    CathegoryOs::MACOS_ANY    },
     };

     OsValues::const_iterator it = osValues.find( src );
     if ( it == osValues.end() )
     {
          return false;
     }
     dest = it->second;
     return true;
}


std::string SvalConvert::os( CathegoryOs::T cat )
{
     switch ( cat )
     {
          case CathegoryOs::IOS_ANY:     return "ios-any";
          case CathegoryOs::ANDROID_ANY: return "android-any";
          case CathegoryOs::WINDOWS_ANY: return "windows-any";
          case CathegoryOs::MACOS_ANY:   return "macos-any";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::status( CathegoryStatus::T cat )
{
     switch ( cat )
     {
          case CathegoryStatus::NEW:         return "new";
          case CathegoryStatus::IN_PROGRESS: return "in-progress";
          case CathegoryStatus::DST_SENT:    return "dst-sent";
          case CathegoryStatus::BLOCKED:     return "blocked";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::deviceFields( FilterDeviceFields::T cat )
{
     switch ( cat )
     {
          case FilterDeviceFields::ID:   return "id";
          case FilterDeviceFields::HWID: return "hwid";
          case FilterDeviceFields::NAME: return "name";
          case FilterDeviceFields::OS:   return "os";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::userFields( FilterUserFields::T cat )
{
     switch ( cat )
     {
          case FilterUserFields::ID:            return "id";
          case FilterUserFields::LOGIN:         return "login";
          case FilterUserFields::EMAIL:         return "email";
          case FilterUserFields::PHONE:         return "phone";
          case FilterUserFields::DESCRIPTION:   return "description";
          case FilterUserFields::STATUS:        return "status";
          case FilterUserFields::COMPANY_ROLES: return "company_roles";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


HttpMethodTok::TokStr SvalConvert::httpMethodToken( HttpMethodTok::T cat )
{
     switch ( cat )
     {
          case HttpMethodTok::POST:    return 'p';
          case HttpMethodTok::GET:     return 'g';
          case HttpMethodTok::DELETE:  return 'd';
          case HttpMethodTok::PUT:     return 'u';
          case HttpMethodTok::UNKNOWN: break;
     }
     return 'e';
}


HttpMethodTok::T SvalConvert::httpMethodTokenId( const std::string& envValue )
{
     typedef std::map< std::string, HttpMethodTok::T > EnvValues;
     static const EnvValues envValues =
     {
          { "GET",    HttpMethodTok::GET    },
          { "POST",   HttpMethodTok::POST   },
          { "PUT",    HttpMethodTok::PUT    },
          { "DELETE", HttpMethodTok::DELETE },
     };
     EnvValues::const_iterator it = envValues.find( envValue );
     if ( it != envValues.end() )
     {
          return it->second;
     }
     return HttpMethodTok::UNKNOWN;
}


RocErrorCodes::T SvalConvert::dbErrorCodeId( const std::string& dbStr )
{
     typedef std::map< std::string, RocErrorCodes::T > Values;
     static const Values values =
     {
          { "bad_phone_format"       , RocErrorCodes::BAD_PHONE_FORMAT               },
          { "bad_login_format"       , RocErrorCodes::BAD_LOGIN_FORMAT               },
          { "login_already_exists"   , RocErrorCodes::LOGIN_ALREADY_EXISTS           },
          { "email_already_exists"   , RocErrorCodes::EMAIL_ALREADY_EXISTS           },
          { "phone_already_exists"   , RocErrorCodes::PHONE_ALREADY_EXISTS           },
          { "company_does_not_exists", RocErrorCodes::COMPANY_NOT_EXISTS             },
          { "empty_company_id"       , RocErrorCodes::EMPTY_COMPANY_ID               },
          { "empty_email"            , RocErrorCodes::BAD_EMAIL_FORMAT               },
          { "user_already_exists"    , RocErrorCodes::USER_ALREADY_EXISTS            },
          { "bad_os_identifier"      , RocErrorCodes::BAD_OS_IDENTIFIER              },
          { "user_not_exists"        , RocErrorCodes::USER_NOT_EXISTS                },
          { "empty_user_id"          , RocErrorCodes::EMPTY_USER_ID                  },
          { "nonunique_request_id"   , RocErrorCodes::NSMS_REQUEST_ID_ALREADY_EXISTS },
          { "bad_argument_length"    , RocErrorCodes::BAD_ARGUMENT_LENGTH            }
     };
     Values::const_iterator it = values.find( dbStr );
     if ( it != values.end() )
     {
          return it->second;
     }
     ITCS_WARNING( "WARNING: dbms error has not mapped: " << dbStr );
     assert( 0 );
     return RocErrorCodes::SUCCESS;
}


std::string SvalConvert::rocErrorMessage( RocErrorCodes::T id )
{
     switch ( id )
     {
          case RocErrorCodes::SUCCESS                         : assert( 0 ); return ""; // такого встречаться не должно
          case RocErrorCodes::BAD_PHONE_FORMAT                : return "bad phone format";
          case RocErrorCodes::BAD_LOGIN_FORMAT                : return "bad login format";
          case RocErrorCodes::BAD_EMAIL_FORMAT                : return "bad email format";
          case RocErrorCodes::USER_ALREADY_EXISTS             : return "user already exists";
          case RocErrorCodes::BAD_JSON_FORMAT                 : return "bad json format";
          case RocErrorCodes::LOGIN_ALREADY_EXISTS            : return "login already exists";
          case RocErrorCodes::EMAIL_ALREADY_EXISTS            : return "email already exists";
          case RocErrorCodes::PHONE_ALREADY_EXISTS            : return "phone already exists";
          case RocErrorCodes::COMPANY_NOT_EXISTS              : return "company not exists";
          case RocErrorCodes::EMPTY_COMPANY_ID                : return "empty company id";
          case RocErrorCodes::BAD_REQUEST                     : return "Bad URI in request";
          case RocErrorCodes::MULTI_ERROR                     : return "Multiple errors have occurred";
          case RocErrorCodes::EXCEPTION                       : return "exception";
          case RocErrorCodes::AUTHORIZATION_ERROR             : return "Error validating access token or login";
          case RocErrorCodes::BAD_OS_IDENTIFIER               : return "bad os identifier";
          case RocErrorCodes::BAD_DEVICE_NAME                 : return "bad device name";
          case RocErrorCodes::BAD_DEVICE_HWID                 : return "bad device hwid";
          case RocErrorCodes::EMPTY_USER_ID                   : return "empty user id";
          case RocErrorCodes::USER_NOT_EXISTS                 : return "user not exists";
          case RocErrorCodes::BAD_PASSWORD_FORMAT             : return "bad password format";
          case RocErrorCodes::DEVICE_VIPNET_ID_IS_NOT_READY   : return "device vipnet id is not ready";
          case RocErrorCodes::DST_REQUEST_ID_IS_EMPTY         : return "dst request id is empty";
          case RocErrorCodes::BAD_DB_NSMS_VIPNET_ID           : return "bad db nsms vipnet id";
          case RocErrorCodes::BAD_DB_NSMS_DST_REQUEST_ID      : return "bad db nsms dst request id";
          case RocErrorCodes::NSMS_UNREACHIBLE                : return "nsms unreachible";
          case RocErrorCodes::NSMS_ADDRESS_LIMIT_REACHED      : return "nsms addres limit reached";
          case RocErrorCodes::NSMS_EMPTY_DST_STATUS_RESPONSE  : return "nsms empty dst status response";
          case RocErrorCodes::NSMS_EMPTY_READY_DST            : return "nsms empty ready dst";
          case RocErrorCodes::NSMS_INTERNAL_SERVER_ERROR      : return "nsms internal server error";
          case RocErrorCodes::NSMS_BAD_REQUEST_ARGUMENTS      : return "nsms bad request arguments";
          case RocErrorCodes::NSMS_LICENSE_LIMIT_REACHED      : return "nsms license limit reached";
          case RocErrorCodes::NSMS_NODE_ALREADY_EXISTS        : return "nsms node already exists";
          case RocErrorCodes::NSMS_REQUEST_LIMIT_EXCEEDED     : return "nsms request limit exceeded";
          case RocErrorCodes::NSMS_THRIFT_EXCEPTION           : return "nsms thrift exception";
          case RocErrorCodes::NOTIFY_INTERNAL_SERVER_ERROR    : return "notify internal server error";
          case RocErrorCodes::NOTIFY_NO_RESULT                : return "notify no result";
          case RocErrorCodes::NOTIFY_REQUEST_LIMIT_EXCEEDED   : return "notify request limit exceeded";
          case RocErrorCodes::NOTIFY_THRIFT_EXCEPTION         : return "notify thrift exception";
          case RocErrorCodes::NOTIFY_UNREACHIBLE              : return "notify unreachible";
          case RocErrorCodes::NSMS_REQUEST_ID_ALREADY_EXISTS  : return "nsms request id already exists";
          case RocErrorCodes::BAD_ARGUMENT_LENGTH             : return "bad argument length";
          case RocErrorCodes::BAD_DB_NSMS_TOKEN_AUTH          : return "bad db nsms token auth";
          case RocErrorCodes::EXEC_DB_OPERATOR_ERROR          : return "exec db operator error";
          case RocErrorCodes::DEVICE_REQUEST_ID_IS_EMPTY      : return "device request id is empty";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::companyFields( FilterCompanyFields::T id )
{
     switch ( id )
     {
          case FilterCompanyFields::ID:   return "id";
          case FilterCompanyFields::NAME: return "name";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::logFields( FilterLogFields::T id )
{
     switch ( id )
     {
          case FilterLogFields::TIME      : return "time";
          case FilterLogFields::ACTOR     : return "actor";
          case FilterLogFields::EVENT_TYPE: return "event_type";
          case FilterLogFields::USER_ID   : return "user_id";
          case FilterLogFields::USER_EMAIL: return "user_email";
          case FilterLogFields::USER_LOGIN: return "user_login";
          case FilterLogFields::DEVICE_OS : return "device_os";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


std::string SvalConvert::event( CathegoryEvent::T id )
{
     switch ( id )
     {
          case CathegoryEvent::CREATE_USER   : return "create-user";
          case CathegoryEvent::CREATE_DEVICE : return "create-device";
          case CathegoryEvent::DST_REQUEST   : return "dst-request";
          case CathegoryEvent::EDIT_USER     : return "edit-user";
          case CathegoryEvent::REMOVE_USER   : return "remove-user";
          case CathegoryEvent::BLOCK_USER    : return "block-user";
          case CathegoryEvent::UNBLOCK_USER  : return "unblock-user";
          case CathegoryEvent::EDIT_DEVICE   : return "edit-device";
          case CathegoryEvent::REMOVE_DEVICE : return "remove-device";
     }
     assert( 0 ); // такого встречаться не должно
     return "";
}


} // namespace roc
} // namespace itcs
