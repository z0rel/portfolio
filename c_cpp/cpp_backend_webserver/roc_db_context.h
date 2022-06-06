/// @file
/// @brief Общий контекст для работы с базой
#pragma once
#ifndef ROC_DB_CONTEXT_H
#define ROC_DB_CONTEXT_H


#include <string>
#include <memory>
#include <vector>
#include <set>
#include <libpq-fe.h>

#include <roc_parser_dictionary.h>
#include <roc_nodename_generator.h>


// Объявление пространства имен из библиотеки libjsoncpp, чтобы не подключать ее заголовочный файл
namespace Json
{
class Value;
}


namespace roc_notify_api_1_0
{
class NotifyServiceClient;
}


namespace nsms_api_1_0
{
class NSMSServiceClient;
}


namespace itcs
{
namespace roc
{


template < typename T >
class RpcContext;


class Base64Decoder;
class HttpHandlerSettings;


/// Информация о подготовленном SQL операторе
struct PreparedOperator
{
     typedef int Nargs;

     PreparedOperator( const std::string& str );

     /// Идентификатор подготовленного оператора
     std::string operatorId = std::string();

     /// Количество аргументов в SQL операторе
     Nargs nargs = 0;
};


/// Контекст текущего запроса - запрос, ответ, авторизация, статус и т.д.
struct RequestContext
{
     typedef pg_int64 Uid;

     /// Очистить контекст
     void clear();

     /// Установить статус и дефолтные заголовки ответа
     bool setStatus( bool setStatus );

     /// Строка ответа
     std::string response = std::string();

     /// Множество кодов ошибок выполнения
     std::set< RocErrorCodes::T > responseErrors = std::set< RocErrorCodes::T >();

     /// Тело запроса
     std::string requestBody = std::string();

     /// Идентификатор авторизовавшегося пользователя
     Uid uid = 0;

     /// Идентификатор авторизовавшегося пользователя в формате BigEndian
     Uid pgUid = 0;

     /// Заголовки ответа
     const char* responseHeaders_ = nullptr;

     /// Заголовки ответа для случая, когда нужно их динамическое формирование
     std::string responseHeadersStr_ = std::string();

     /// Состояние последней команды
     bool commandStatus = false;
};


/// Установить соединение с заданным портом на заданном хосте
bool openTcpConnection( int& sd, const char* hostname, int port , std::string& errMessage );


/// Общий контекст для работы с базой
class RocDbContext
{
public:
     typedef pg_int64 User;
     typedef pg_int64 Device;
     typedef pg_int64 Company;
     typedef pg_int64 UserStatus;
     typedef RpcContext< nsms_api_1_0::NSMSServiceClient > NsmsRpcContext;
     typedef RpcContext< roc_notify_api_1_0::NotifyServiceClient > NotifyRpcContext;

     explicit RocDbContext( std::shared_ptr< HttpHandlerSettings >& config );

     virtual ~RocDbContext();

     /// Вернуть статус последней команды
     bool success() const;

     /// Исполнить текстовый запрос и вернуть текстовый ответ
     void execQuery( std::string& dest, const std::string& query );

     /// Строка ответа
     std::string& response();

     /// Заголовки ответа
     const char* responseHeaders();

     /// Вернуть тело запроса
     std::string& requestBody();

     /// Конфиг приложения
     const std::shared_ptr< HttpHandlerSettings >& config() const;

     /// Очистить контекст запроса по завершении его исполнения
     void clearRequestContext();

     /// Установить для запроса контекст авторизации
     bool authorization( const char* authStr );

     /// Получить Oid для int
     Oid int4Oid() const;

     /// Получить Oid для bigint
     Oid int8Oid() const;

     /// Получить Oid для boolean
     Oid boolOid() const;

     /// получить varchar устройства
     Oid varcharOid() const;

     /// Получить true-значение для boolean
     const std::string& boolTrue() const;

     /// Получить размер boolean
     int boolSize() const;

     /// Установить ответ-ошибку, если заданы ошибки
     void transformResponseIfError();

protected:
     typedef pg_int64 RequestId;

     /// Интерфейс для подготовки при инициализации соединения с базой
     virtual bool prepareDbMethods() = 0;

     /// Установить статус ответа равным заданной ошибке
     void setErrorResponse( RocErrorCodes::T code );

     /// Инициализация метаданных для используемых типов Postgresql
     bool initPgTypesMetadata();

     /// Выполнить запрос, возвращающий bigint
     bool bigintPgQuery( pg_int64& dest, PreparedOperator& stmt, const char* const* values,
                       const int* lengths, const int* formats );

     /// Подготовка операторов для исполнения в БД
     void prepareStmt( bool& status, PreparedOperator& stmtInfo, const std::vector< Oid >& paramTypes,
                       const std::string& query );

     /// Выполнить подготовленный оператор как транзакцию
     bool execPrepared( std::string& destJson, PreparedOperator& stmtInfo, const char* const* values,
                        const int* lengths, const int* formats );

     /// Выполнить подготовленный оператор как транзакцию
     bool execPrepared( std::string& destJson, PreparedOperator& stmtInfo,
                        const char* const* vals,
                        const std::initializer_list<int>& lengths,
                        const std::initializer_list<int>& formats );

     /// Выполнить инициализацию контекста после установки соединения
     bool initAfterConnect();

     /// Форматировать в JSON коды ошибок
     void setResponseErrors( std::set<RocErrorCodes::T>& errors );

     /// Форматировать в JSON коды ошибок СУБД
     bool transformDbmsError( std::string& errorStr );

     /// Проверить состояние подключения, и если оно стало некорректным - попытаться его восстановить
     bool consistance( const std::string& errMessage );

     /// Проверить корректность подготовленного оператора
     bool checkPreparedStatement( PreparedOperator& op , size_t typesLen );

     /// Проверить наличие или отсутствие заданной привилегии для авторизовавшегося пользователя
     bool checkRequestPrivilege( User uid , const std::string& admPriv, const std::string& userPriv = "" );

     /// Парсить тело fastcgi-запроса, если он в json
     bool parseJsonBody( Json::Value& res );

     /// Получить значение идентификатора из базы
     bool getRequestId( RequestId& requestId, bool& isNull, Device inDevice, PreparedOperator& op );

     /// Записать значение значение идентификатора в базу
     bool setRequestId(const RequestId* requestId, Device inDevice, PreparedOperator& op);

     /// Получить токен доступа к NSMS
     bool getNsmsToken( std::string& destNsmsToken, Company inCompany );

     /// Обновить статус пользователя
     bool setUserStatus( Company companyId, User userId, UserStatus userStatus );

     /// Получить информацию для формирования узла
     bool getNodeInfo( Device inDevice, std::string& login, std::string& description, std::string& os );

     /// Обновить имя узла в базе
     bool updateNodeName( Device inDevice, const std::string& nodeName );

protected:
     typedef std::shared_ptr< PGconn > PtrConnection;

     /// Структура конфига приложения
     std::shared_ptr< HttpHandlerSettings > config_;

     /// Структура подключения к БД
     PtrConnection conn_ = PtrConnection();

     /// Данные текущего запроса - запрос, ответ, авторизация, статус и т.д.
     RequestContext requestCtx_ = RequestContext();

     /// Декодировщик строк Base64 для декодирования данных авторизации
     std::unique_ptr< Base64Decoder > base64Decoder_;

     /// Значение OID для BYTEA
     Oid byteaOid_ = 0;

     /// Значение OID для INT4
     Oid int4Oid_ = 0;

     /// Значение OID для INT4
     Oid int8Oid_ = 0;

     /// Значение OID для VARCHAR
     Oid varcharOid_ = 0;

     /// Значение OID для BOOLEAN
     Oid boolOid_ = 0;

     /// Размер Bool в PostgreSQL
     int boolSize_ = 0;

     /// Формат Bool в PostgreSQL
     int boolFmt_ = 0;

     /// Значение 'True' в PostgreSQL
     std::string boolTrue_ = std::string();

     /// Значение 'False' в PostgreSQL
     std::string boolFalse_ = std::string();

     /// Статус пользователя "Новый"
     UserStatus userNew_ = 0;

     /// Статус пользователя "В процессе"
     UserStatus userInProgress_ = 0;

     /// Статус пользователя "Ключи отосланы"
     UserStatus userDstSend_ = 0;

     /// Статус пользователя "Заблокирован"
     UserStatus userBlocked_ = 0;

     /// Проверить авторизованность
     PreparedOperator checkAuthorization_ = PreparedOperator( "CHECK_AUTHORIZATION" );

     /// Проверить наличие привилегии
     PreparedOperator checkPrivilege_ = PreparedOperator( "CHECK_PRIVILEGE" );

     /// Получить токен доступа к NSMS
     PreparedOperator getNsmsToken_ = PreparedOperator( "GET_NSMS_TOKEN" );

     /// Установить статус пользователя
     PreparedOperator setUserStatus_ = PreparedOperator( "SET_USER_STATUS" );

     /// Получить информацию о пользователе для формирования узла
     PreparedOperator getNodeInfo_ = PreparedOperator( "GET_NODE_INFO" );

     /// Подсчитать число дубликаторв логинов для первых 28 (или сколько задано в конфиге) символов логина
     PreparedOperator countDuplicatedFirstLoginChars_ = PreparedOperator( "COUNT_DUPLICATED_FIRST_LOGIN" );

     PreparedOperator setNodeName_ = PreparedOperator( "SET_NODE_NAME" );

     /// Генератор имен узлов
     NodenameGenerator nodenameGenerator_;
};


} // namespace roc
} // namespace itcs


#endif // ROC_DB_CONTEXT_H
