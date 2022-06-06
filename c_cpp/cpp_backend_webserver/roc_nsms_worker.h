/// @file
/// @brief Рабочий поток для синхронного взаимодействия с NSMS
#ifndef ROCNSMSWORKER_H
#define ROCNSMSWORKER_H

#include <roc_db_context.h>

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>


namespace itcs
{
namespace roc
{

/// Контейнер подготовленных операторов
struct PreparedRocNsmsStatements
{
     /// Получить идентификатор запроса создания устройства
     PreparedOperator getAddDeviceRequestId = PreparedOperator( "NSMS_GET_ADD_DEVICE_REQUEST_ID" );

     /// Установить идентификатор запроса выдачи DST
     PreparedOperator setDstRequestId = PreparedOperator( "NSMS_SET_DST_REQUEST_ID" );

     /// Запросить идентификатор запроса выдачи DST
     PreparedOperator getDstRequestId = PreparedOperator( "NSMS_GET_DST_REQUEST_ID" );

     /// Установить идентификатор запроса создания устройства
     PreparedOperator setAddDeviceRequestId = PreparedOperator( "NSMS_SET_ADD_DEVICE_REQUEST_ID" );

     /// Запросить следующую отложенную операцию взаимодействия с NSMS
     PreparedOperator nextNsmsOperation = PreparedOperator( "NSMS_NEXT_NSMS_OPERATION" );

     /// Запросить ViPNet Id устройства
     PreparedOperator setVipnetId = PreparedOperator( "NSMS_SET_VIPNET_ID" );

     /// Установить флаг необходимости запроса DST
     PreparedOperator setDstRequested = PreparedOperator( "NSMS_SET_DST_REQUESTED" );

     /// Получить идентификаторы организаций
     PreparedOperator getCompaniesId = PreparedOperator( "NSMS_GET_COMPANIES_ID" );

     /// Обновить информацию об организации
     PreparedOperator updateCompanyInfo = PreparedOperator( "UPDATE_COMPANY_INFO" );
};


/// Структура состояния одной операции взаимодействия с NSMS
struct NsmsDeviceOperation
{
     typedef pg_int64 RequestId;
     typedef pg_int64 Company;
     typedef pg_int64 Device;

     /// Идентификатор устройства
     Device deviceId     = 0;

     /// Имя узла
     std::string nodeName = std::string();

     /// Платформа
     CathegoryOs::T platformId   = CathegoryOs::WINDOWS_ANY;

     /// ViPNetID
     RequestId vipnetId = 0;

     /// ViPNetID задан
     bool issetVipnetId = false;

     /// Идентификатор компании
     Company companyId = 0;

     /// Флаг запрошенности DST
     RequestId dstRequested = 0;

     /// Идентификатор запроса DST
     RequestId dstRequestId = 0;

     /// Идентификатор запроса DST задан
     bool issetDstRequestId = false;

     /// Идентификатор добавления устройства
     RequestId addRequestId = 0;

     /// Идентификатор добавления устройства задан
     bool issetAddRequestId = false;
};


class RocNsmsWorker : public RocDbContext
{
public:
     explicit RocNsmsWorker( std::shared_ptr< HttpHandlerSettings >& config );

     ~RocNsmsWorker();

     /// Запустить поток
     void start();

     /// Завершить выполнение потока
     void stop();

     /// Разбудить рабочий поток, если он находится в состоянии ожидания
     void wakeupOperations();

private:
     /// Статус подключения к сервису Thrift
     enum ConnectionState
     {
          /// Неизвестно, есть подключение или нет
          UNKNOWN,
          /// Подключение к NSMS есть
          CONSISTANCE,
          /// Подключения к nsms нет
          INCONSISTANCE
     };


     /// Рабочий цикл потока
     void workLoop();

     /// Подготовить необходимые операторы СУБД для последующего исполнения
     bool prepareDbMethods();

     /// Выполнить одну итерацию добавления устройств
     /// @return true, если выполнялось добавление устройств, false если очередь пуста
     bool workIteration( size_t iteration );

     /// Выполнить запрос добавления устройства
     bool addDeviceNode( RequestId& vipnetId, bool& nullVipnetId, RequestId& requestId,
                         const std::string& nsmsToken, Device inDevice, CathegoryOs::T osCathegory,
                         const std::string& inNodeName );

     /// Проверить готовность VipNetId устройства
     void checkViPNetId( RequestId& vipnetId, RequestId addRequestId, const std::string& nsmsToken, Device inDevice );

     /// Выполнить запрос DST
     bool requestDst( const std::string& nsmsToken, RequestId vipnetId, Device inDevice,
                      RequestId& requestId, bool& nullRequestId );

     /// Проверить готовность DST
     void checkDstReady( const std::string& nsmsToken, RequestId dstRequestId, Device inDevice );

     /// Отправить по SMS оповещение с паролем
     RocErrorCodes::T sendSmsNotify( const std::string& secret );

     /// Установить в базе признак готовности DST
     bool setDstReady( RequestId& requestId, Device inDevice );

     /// Инициализировать последовательность операций с NSMS
     bool getNsmsOperations( std::vector< NsmsDeviceOperation >& dest );

     /// Обновить информацию об организации.
     void updateCompaniesInfo();

private:
     /// Рабочий поток
     std::thread             workThread_ = std::thread();

     /// Флаг активности рабочего потока
     std::atomic_bool        isRunned_;

     /// Мьютекс для механизма уведомления о новых данных
     std::mutex              conditionLock_;

     /// Переменная состояния для возможности уведомить поток о появлении новых данных, если он их ждет
     std::condition_variable conditionVar_;

     /// Подговтовленные SQL-операторы
     PreparedRocNsmsStatements stmts_ = PreparedRocNsmsStatements();

     /// Контекст RPC взаимодействия с NSMS
     std::unique_ptr< NsmsRpcContext > nsmsRpcContext_;

     /// Контекст RPC взаимодействия с сервисом нотификации
     std::unique_ptr< NotifyRpcContext > notifyRpcContext_;

     /// Время сна при бездействии
     int sleepMsec_;

     /// Частота обновления информации о компании (интервал обновления = updateCompanyInfoFrequency_ * sleepMsec_)
     int updateCompanyInfoFrequency_;

     /// Статус подключения к NSMS
     ConnectionState nsmsConnectionState_ = UNKNOWN;

     /// Статус подключения к сервису нотификации
     ConnectionState notifyConnectionState_ = UNKNOWN;
};


} // namespace roc
} // namespace itcs


#endif // ROCNSMSWORKER_H
