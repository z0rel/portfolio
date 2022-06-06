/// @file
/// @brief Средства тестирования компонентов RoC Service
///
/// @copyright Copyright (c) InfoTeCS. All rights reserved
#pragma once
#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <string>
#include <fstream>
#include <memory>

#include <json_utils.h>
#include <http_handler.h>

namespace apache
{
namespace thrift
{
namespace server
{
class TSimpleServer;

} // namespace server
} // namespace thrift
} // namespace apache


namespace itcs
{
namespace roc
{

/// Инициализировать библиотеку SSL
void initSSL();

/// Освободить ресурсы библиотеки SSL
void cleanupSSL();

/// Выполнить запрос по HTTPS
void httpsRequest( std::string& httpCode, std::string& response,
                   const std::string& hostname, const int portnum, const std::string& authData,
                   const std::string& request, const std::string& requestBody );


/// Выполнить запрос по HTTP
void httpRequest( std::string& httpCode, std::string& response,
                  const std::string& hostname, const int portnum, const std::string& authData,
                  const std::string& request, const std::string& requestBody );


/// Контекст отдельного набора данных для теста
struct TestCaseItemData
{
     TestCaseItemData();

     /// Получить строковое представление тела запроса
     void getBodyString( std::string& dest );

     /// Установить поля из Json
     void initFromJson( Json::Value& it );

     /// Установить значение SQL Query из Json
     void setSqlQuery( Json::Value& q );

     /// Данные авторизации
     std::string auth = std::string();

     /// URI-строка запроса
     std::string uri  = std::string();

     /// Код HTTP-ответа
     std::string httpCode  = std::string();

     /// Тело запроса
     Json::Value body = Json::Value();

     /// Тело запроса в тестовом виде
     std::string bodyText = std::string();

     /// Ожидаемый ответ
     Json::Value response = Json::Value();

     /// Отладочный флаг: выйти сразу после ошибки для этих тестовых данных
     bool exitAfterError = false;

     /// HTTP-метод для выполнения запроса
     std::string httpMethod = std::string();

     /// Запрос для теста базы данных
     std::string sqlQuery = std::string();

     /// Время (секунды), на которое нужно приостановиться перед исполнением теста
     unsigned int sleepTime = 0;
};


/// Контекст всех тестов
struct TestContext
{
     typedef std::vector< TestCaseItemData > TestCaseData;

     ~TestContext();

     /// Получить данные для теста
     bool getTestcaseData( TestContext::TestCaseData& dest, const std::string& testcase );

     /// Флаг - нужно только запустить симуляцию сервисов NSMS и Notify
     bool simulateInterfacesOnly  = false;

     /// Флаг - нужно ли тестировать HTTPS взаимодействие
     bool needTestHttps  = true;

     /// Флаг - нужно ли тестировать HTTP взаимодействие ( может быть полезно для отладки через tcpdump )
     bool needTestHttp  = false;

     /// Флаг - нужно ли тестировать парсер без сети
     bool needTestParser = false;

     /// Файл конфигурации основного приложения
     std::string configFilename;

     /// Файл с тестовыми данными
     std::string testdataFile;

     /// Хост для тестирования HTTP/HTTPS
     std::string host = "localhost";

     /// Порт для тестирования HTTP/HTTPS
     int port = 9983;

     /// Порт для симуляции NSMS/Administrator
     int nsmsThriftPort   = 9985;

     /// Порт для симуляции сервиса нотификации
     int notityThriftPort = 9986;

     /// Поток для симуляции NSMS/Administrator
     std::thread nsmsServeThread;

     /// Поток для симуляции сервиса нотификации
     std::thread notifyServeThread;

     /// Основной поток обработки тестовых HTTP-сообщений
     std::thread testingThread;

     /// Контекст обработчика запросов
     std::shared_ptr< HttpHandler > handler;

     /// Симулятор взаимодействия с NSMS/Administrator
     std::shared_ptr< apache::thrift::server::TSimpleServer > nsmsServiceSimulator;

     /// Симулятор взаимодействия с Notify
     std::shared_ptr< apache::thrift::server::TSimpleServer > notifyServiceSimulator;
};


} // namespace roc
} // namespace itcs


/// Контекст инициализации тестов
class SetupTests
{
public:

     SetupTests();
     ~SetupTests();

     /// Глобальные данные конфигурации тестов
     static itcs::roc::TestContext context;

private:
     /// Выполнить инициализацию базы данных
     void initDb( const std::string& testdataFile, const std::string& initModelFile, const std::string& psql );

     static void setBoolValue( bool& dest, const std::string& val );
     static void setStrValue( std::string& dest, const std::string& val );
     static void setIntValue( int& dest, const std::string& val );
};


#endif // TCP_UTILS_H
