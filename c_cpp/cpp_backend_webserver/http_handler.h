/// @file
/// @brief FastCgi сервер
#pragma once
#ifndef HTTP_HANDLER_H
#define HTTP_HANDLER_H

#include <string>
#include <memory>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <map>
#include <stdint.h>
#include <fcgiapp.h>

#include <roc_parser_dictionary.h>

namespace itcs
{
namespace roc
{


class HttpHandlerSettings;
class RocMethodsContext;
class UriDecoder;
class Base64Decoder;
class RocNsmsWorker;


class HttpHandler
{
public:
     enum ResponseDataType
     {
          JSON_TYPE,
     };

     typedef std::pair< ResponseDataType, std::string > HttpResponse;

     explicit HttpHandler( const std::string& configFilename );
     ~HttpHandler();

     /// Главный управляющий цикл. Создает пул потоков и ожидает завершения
     void mainLoop();

     /// Завершить работу
     void finish();

     /// Получить конфигурацию исполнения
     HttpHandlerSettings& config();

private:
     typedef std::map< std::string, std::string > HttpParams;

     /// Состояние приема данных
     enum AcceptState
     {
          /// Данные приняты
          ACCEPTED,
          /// Ложное срабатывание
          NO_DATA,
          /// Поток остановлен
          STOPPED
     };

     /// @brief Обнулить факт исполнения
     void stopRunning();

     /// @brief Выполняется в потоке обработки запросов.
     void work();

     /// @brief Выполнить FastCGI запрос
     void execRequest( RocMethodsContext& ctx, FCGX_Request& req );

     /// @brief описать ошибку, возникшую при обработке запроса
     void sendJsonResponse( FCGX_Request& request, const char* responseHeaders, const std::string& responseBody );

     /// Отправить в качестве FastCGI-ответа - json-сообщение об ошибке
     void sendErrorJsonResponse( FCGX_Request& req, RocErrorCodes::T code );

     /// Ждать запрос
     AcceptState pollRequest();

     /// Принять запрос в цикле исполнения
     AcceptState acceptFcgxRequest( FCGX_Request& request );

     /// Проверить авторизованность запроса, установить привилегии пользователя
     bool authorization( RocMethodsContext& ctx, FCGX_Request& req );

private:
     typedef std::vector< std::thread > WorkThreads;

     /// Конфигурация исполнения
     std::shared_ptr< HttpHandlerSettings > config_;

     /// Пул потоков для обработки запросов
     WorkThreads workThreads_;

     /// Состояние выполнения пула потоков - когда false, либо не он еще не выполняется, либо завершается
     volatile bool isRunning_ = false;

     /// Блокировка для монопольного вызова FCGX_Accept_r.
     std::mutex acceptLock_;

     /// Дескриптор сокета для FastCGI
     int socketFd_ = -1;

     /// Декодировщик строк URI запросов для декодирования символов %HH
     std::unique_ptr< UriDecoder > uriDecoder_;

     /// Фоновый поток для взаимодействия с NSMS
     std::unique_ptr< RocNsmsWorker > nsmsWorker_;
};


} // namespace roc
} // namespace itcs

#endif // HTTP_HANDLER_H
