/// @file
/// @brief Класс настроек для http_handler и для всего приложения
#pragma once
#ifndef HTTP_HANDLER_SETTINGS_H
#define HTTP_HANDLER_SETTINGS_H

#include <string>
#include <memory>
#include <map>

// Объявление пространства имен из библиотеки libjsoncpp, чтобы не подключать ее заголовочный файл
namespace Json
{
class Value;
}

namespace itcs
{
namespace roc
{


class HttpHandlerSettings
{
public:
     HttpHandlerSettings( const std::string& configFilename );

     /// Число потоков для обработки запросов
     unsigned int threadPoolSize() const;

     /// URI сокета и порта, на которых необходимо открыть FastCGI сервер
     const std::string& socketPath() const;

     /// Строка подключения к базе PostgreSQL
     const std::string& postgresConnString() const;

     /// Максимальный размер, до которого может расти очередь ожидающих соединений у sockfd в вызове listen
     unsigned int queueLength() const;

     /// Сколько байт содержит payload запроса. Больше не допускается (для безопасности)
     unsigned int requestPayloadLen() const;

     /// Число попыток переподключиться при неудаче (сокет, БД)
     unsigned int maxFailedAttempt() const;

     /// Таймаут перед завершением (сек)
     unsigned int timeoutEndSec() const;

     /// Таймаут при ошибке (сек)
     unsigned int timeoutErrorSec() const;

     /// Максимальная длина email (символов)
     unsigned int maxEmailLength() const;

     /// Таймаут перед попыткой восстановления соединения к PostgreSQL (сек)
     unsigned int dbReconnectTimeout() const;

     /// Число попыток восстановить соединение к PostgreSQL
     unsigned int dbReconnectAttempts() const;

     /// Время ожидания данных на сокете через вызов poll (мсек)
     unsigned int pollTimeout() const;

     /// Максимальное время сна при отсутствии новых запросов к NSMS в миллисекундах
     unsigned int nsmsSleepMsec() const;

     /// Частота обновления информации об организации
     unsigned int updateCompanyInfoFrequency() const;

     /// Лимит тела запросов (в байтах)
     size_t requestBodyLimit() const;

     /// Лимит строки с JSON, полученной по http (необходим, т.к. большой некорректный json приводит к ошибке
     /// сегментации в старой версии libjsoncpp, которая использется в сборке
     size_t jsonBodyLimit() const;

     /// Порт NSMS
     unsigned int nsmsServerPort() const;

     /// Адрес хоста NSMS
     const std::string& nsmsServerHost() const;

     /// Порт сервиса нотификации
     unsigned int notifyPort() const;

     /// Адрес хоста сервиса нотификации
     const std::string& notifyHost() const;

     /// Шаблон имени узла
     const std::string& nodenameTemplate() const;

     /// Ограничение длины логина в имени узла
     unsigned int templateLoginChars() const;

     /// Ограничение длины имени платформы в имени узла
     unsigned int templateOsChars() const;

     /// Ограничение длины описанияя в имени узла
     unsigned int templateDescriptionChars() const;

     /// Ограничение общей длины имени узла
     unsigned int nodenameMaxlen() const;

private:
     /// Контекст для получения значений аргументов в списке инициализации
     std::unique_ptr< Json::Value > ctx_;

     /// Таймаут перед завершением (сек)
     const unsigned int timeoutEndSec_;

     /// Таймаут при ошибке (сек)
     const unsigned int timeoutErrorSec_;

     /// Число потоков для обработки запросов
     const unsigned int threadPoolSize_;

     /// URI сокета и порта, на которых необходимо открыть FastCGI сервер
     const std::string socketPath_;

     /// Строка подключения к базе PostgreSQL
     const std::string postgresConnString_;

     /// Максимальный размер, до которого может расти очередь ожидающих соединений у sockfd в вызове listen
     const unsigned int queueLength_;

     /// Сколько байт содержит payload запроса. Больше не допускается (для безопасности)
     const unsigned int requestPayloadLen_;

     /// Число попыток переподключиться при неудаче (сокет, БД)
     const unsigned int maxFailedAttempt_;

     /// Максимальная длина email (символов)
     const unsigned int maxEmailLength_;

     /// Таймаут перед попыткой восстановления соединения к PostgreSQL (сек)
     const unsigned int dbReconnectTimeout_;

     /// Число попыток восстановить соединение к PostgreSQL
     const unsigned int dbReconnectAttempts_;

     /// Время ожидания данных на сокете через вызов poll (мсек)
     unsigned int pollTimeout_;

     /// Лимит тела запросов (в байтах)
     const size_t requestBodyLimit_;

     /// Лимит строки с JSON, полученной по http (необходим, т.к. большой некорректный json приводит к ошибке
     /// сегментации в старой версии libjsoncpp, которая использется в сборке
     const size_t jsonBodyLimit_;

     /// Порт NSMS
     unsigned int nsmsServerPort_;

     /// Адрес хоста NSMS
     const std::string nsmsServerHost_;

     /// Порт сервиса Notify
     unsigned int notifyPort_;

     /// Адрес хоста сервиса Notify
     const std::string notifyHost_;

     /// Максимальное время сна при отсутствии новых запросов к NSMS (в миллисекундах)
     unsigned int nsmsSleepMsec_;

     /// Частота обновления информации об организации
     unsigned int updateCompanyInfoFrequency_;

     /// Шаблон названия узла
     std::string nodenameTemplate_;

     /// Ограничение длины логина в имени узла
     unsigned int templateLoginChars_;

     /// Ограничение длины имени платформы в имени узла
     unsigned int templateOsChars_;

     /// Ограничение длины описанияя в имени узла
     unsigned int templateDescriptionChars_;

     /// Ограничение общей длины имени узла
     unsigned int nodenameMaxlen_;
};


} // namespace roc
} // namespace


#endif // HTTP_HANDLER_SETTINGS_H
