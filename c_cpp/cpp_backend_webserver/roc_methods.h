/// @file
/// @brief Функциональность методов ROC API

#pragma once
#ifndef ROC_METHODS_H
#define ROC_METHODS_H


#include <roc_db_context.h>


namespace itcs
{
namespace roc
{

class RocParser;
class UriFilterUsr;
class DBResult;
class UserInfoContext;
class RocNsmsWorker;


class PqConv;


/// Контейнер подготовленных операторов
struct PreparedRocStatements
{
     /// Запрос списка пользователей
     PreparedOperator getUsersList = PreparedOperator( "GET_USERS_LIST" );

     /// Добавить пользователя
     PreparedOperator addUser = PreparedOperator( "ADD_USER" );

     /// Получить информацию о пользователе из данных авторизации
     PreparedOperator getSelfUser = PreparedOperator( "GET_SELF_USER" );

     /// Запросить информацию о пользователе для компании
     PreparedOperator getUserInfo = PreparedOperator( "GET_USER_INFO" );

     /// Добавить устройство
     PreparedOperator addDevice = PreparedOperator( "ADD_DEVICE" );

     /// Запросить список устройств
     PreparedOperator getDevices = PreparedOperator( "GET_DEVICES" );

     /// Запросить информацию об устройстве
     PreparedOperator getDeviceInfo = PreparedOperator( "GET_DEVICE_INFO" );

     /// Запросить информацию об организации
     PreparedOperator getCompanyInfo = PreparedOperator( "GET_COMPANY_INFO" );

     /// Обновить информацию о пользователе
     PreparedOperator updateUser = PreparedOperator( "UPDATE_USER_INFO" );

     /// Удалить пользователя
     PreparedOperator deleteUser = PreparedOperator( "DELETE_USER" );

     /// Получить идентификатор запроса создания устройства
     PreparedOperator getAddDeviceRequestId = PreparedOperator( "GET_ADD_DEVICE_REQUEST_ID" );

     /// Установить идентификатор запроса создания устройства
     PreparedOperator setAddDeviceRequestId = PreparedOperator( "SET_ADD_DEVICE_REQUEST_ID" );

     /// Запросить идентификатор запроса выдачи DST
     PreparedOperator getDstRequestId = PreparedOperator( "GET_DST_REQUEST_ID" );

     /// Установить идентификатор запроса выдачи DST
     PreparedOperator setDstRequestId = PreparedOperator( "SET_DST_REQUEST_ID" );

     /// Установить флаг необходимости запроса DST
     PreparedOperator setDstRequested = PreparedOperator( "SET_DST_REQUESTED" );

     /// Получить VipNetId устройства
     PreparedOperator getVipNetId = PreparedOperator( "GET_VIPNET_ID" );

     /// Получить статус формирования DST-файла
     PreparedOperator getDstStatus = PreparedOperator( "GET_DST_STATUS" );

     PreparedOperator setUniqueNodename = PreparedOperator( "SET_UNIQUE_NODENAME" );
};


/// Контекст взаимодействия с БД и исполнения методов API
class RocMethodsContext : public RocDbContext
{
public:
     typedef pg_int64 Limit;
     typedef pg_int64 Offset;

     explicit RocMethodsContext( std::shared_ptr< HttpHandlerSettings >& config, RocNsmsWorker& nsmsWorker );

     ~RocMethodsContext();

     /// Записать в response JSON со списком пользователей, ответить на запросы
     void getUsersList( Company company, const std::string& filterText, const std::string& os_types,
                        const std::string& statuses, Limit limit, bool limitIsSet, Offset offset,
                        const std::string& fieldsFilter );

     /// Выполнить команду добавления пользователя
     void addUser( Company inCompany );

     /// Обновить информацию о пользователе
     void updateUser( Company inCompany, User inUid );

     /// Удалить пользователя
     void deleteUser( Company inCompany, User inUid );

     /// Получить информацию о пользователе из данных авторизации
     void getSelfInfo();

     /// Получить информацию о заданном пользователе для компании
     void getUserInfo( Company inCompany, User inUid );

     /// Добавить устройство для пользователя
     void addDevice( Company inCompany, User inUid );

     /// Запросить список устройств
     void getDevices( Company inCompany, User inUid, const std::string& inName,
                      const std::string& inOs, const std::string& inHwid, Limit limit, bool limitIsSet,
                      Offset offset, const std::string& fieldsFilter);

     /// Запросить информацию об устройстве
     void getDeviceInfo( Company inCompany, User inUser, Device inDevice );

     /// Получить ключи для устройства
     void getDstx( Company inCompany, User inUser, Device inDevice );

     /// Получить статус формирования ключей для устройства
     void getDstxStatus( Company inCompany, User inUser, Device inDevice );

     /// Запросить информацию об организации
     void getCompany( Company inCompany );

     /// Вернуть структуру парсера
     RocParser* getParser();

private:
     typedef std::unique_ptr< RocParser > PtrRocParser;
     typedef pg_int64 RequestId;

     /// Подготовить необходимые операторы СУБД для последующего исполнения
     bool prepareDbMethods();

     /// Сформировать ответ-перенаправление на запрос DST или его состояния
     void responseDstxLocation( const std::string& endpoint, Company inCompany, User inUser, Device inDevice,
                                const std::string& resultCode );

     /// Сформировать ответ с dst-файлом
     void responseDst( const std::string& dst );

     /// Сформировать ответ с статусом dst-файла
     void responseDstStatus();

     /// Получить статус формирования DST файла
     DstStatus::T getDbDstStatus( Device inDevice );

     /// Копирование запрещено
     RocMethodsContext( const RocMethodsContext& ) = delete;

     /// Перемещение запрещено
     RocMethodsContext( RocMethodsContext&& ) = delete;

     /// Присваивание запрещено
     RocMethodsContext& operator=( const RocMethodsContext& ) = delete;

     /// Перемещение запрещено
     RocMethodsContext& operator=( RocMethodsContext&& ) = delete;

private:
     /// Подговтовленные SQL-операторы
     PreparedRocStatements stmts_ = PreparedRocStatements();

     /// Обертка над парсером URI ссылок
     PtrRocParser uriParser_ = PtrRocParser();

     /// Контекст RPC взаимодействия с NSMS
     std::unique_ptr< NsmsRpcContext > nsmsRpcContext_;

     /// Ссылка поток взаимодействия с NSMS, для возможности его пробуждения из других потоков
     RocNsmsWorker& nsmsWorker_;
};


/// Контекст добавления пользователя
class UserInfoContext
{
public:
     typedef std::pair< std::string, bool > OptStr;

     void initFromJson( Json::Value& src , HttpHandlerSettings& config, RocMethodsContext::NsmsRpcContext* nsmsRpc,
                        std::set< RocErrorCodes::T >& errors );

     /// login
     OptStr login = OptStr( std::string(), false );
     /// email
     OptStr email = OptStr( std::string(), false );
     /// phone
     OptStr phone = OptStr( std::string(), false );
     /// description
     OptStr descr = OptStr( std::string(), false );
     /// password
     OptStr paswd = OptStr( std::string(), false );
     /// NSMS access token
     OptStr nsmst = OptStr( std::string(), false );

     /// Необходимо ли заблокировать пользователя
     bool block = false;

     /// Задана ли необходимость блокировки или нет
     bool isBlockedSet = false;

     /// Является ли токен NSMS активным
     bool nsmstActive = false;

private:
     /// Проверить валидность токена доступа к NSMS
     void initNsmsTokenIsActive( RocMethodsContext::NsmsRpcContext* nsmsRpc, std::set< RocErrorCodes::T >& errors );
};


/// Контекст добавления устройства
struct AddDeviceContext
{
     AddDeviceContext( Json::Value& src );

     /// Платформа
     std::string os;
     /// Числовой идентификатор платформы
     CathegoryOs::T osCathegory;
     /// Читабельное имя устройства
     std::string name;
     /// Идентификатор устройства
     std::string hwid;

     /// Контейнер ошибок валидации
     std::vector< RocErrorCodes::T > errors = std::vector< RocErrorCodes::T >();
};


} // namespace roc
} // namespace itcs


#endif // ROC_METHODS_H
