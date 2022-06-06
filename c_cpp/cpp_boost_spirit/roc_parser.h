/// @file
/// @brief Парсер uri-запросов
///
/// @copyright Copyright (c) InfoTeCS. All rights reserved
#pragma once
#ifndef ROC_PARSER_H
#define ROC_PARSER_H

#include <string>
#include <vector>
#include <limits>
#include <memory>

#include <roc_parser_dictionary.h>

namespace itcs
{
namespace roc
{

class RocMethodsContext;
struct RocGrammar;


/// Семантические значения парсера, представимые целыми числами
struct RocParserIntegralSValue
{
     typedef unsigned int UInt;

     enum DefaultValues
     {
          /// Данные по умолчанию для фильтра полей объекта пользователя
          DEFAULT_USER_FIELDS   = FilterUserFields::ID,

          /// Данные по умолчанию для фильтра полей объекта устройства
          DEFAULT_DEVICE_FIELDS = FilterDeviceFields::ID,

          /// Данные по умолчанию для фильтра полей объекта лога
          DEFAULT_LOG_FIELDS = FilterLogFields::TIME | FilterLogFields::EVENT_TYPE
     };

     /// Фильтр платформ для запроса списка пользователей
     UInt filterOs = 0;

     /// Фильтр статусов пользователей для запроса списка пользователей
     UInt filterStatus = 0;

     /// Фильтр идентификатора компании
     UInt companyId = 0;

     /// Фильтр идентификатора устройства
     UInt deviceId = 0;

     /// Фильтр идентификатора пользователя
     UInt userId = 0;

     /// Смещение в выборке
     UInt offset = 0;

     /// Фильтр категорий обытий
     UInt eventTypes = 0;

     /// Ограничение числа элементов в смещенной выборке
     UInt limit = 0;

     /// Если ограничение не задано - выдавать все элементы
     bool limitIsSet = false;

     /// Фильтр полей списка объектов User
     UInt userFilelds = DEFAULT_USER_FIELDS;

     /// Фильтр полей списка объектов Devices
     UInt deviceFields = DEFAULT_DEVICE_FIELDS;

     /// Фильтр полей списка объектов Log
     UInt logFields = DEFAULT_LOG_FIELDS;
};


/// Контекст парсера запросов
class RocParser
{
     /// В грамматике необходимо напрямую заполнять семантические значения.
     /// Грамматика вынесена в отдельный класс, чтобы отделить правила грамматики от семантических значений.
     friend struct RocGrammar;

public:
     typedef unsigned int UInt;

     explicit RocParser( RocMethodsContext& req );
     ~RocParser();

     /// @brief Выполнить синтаксический анализ текста запроса. По результатам вызвать обработчик.
     /// @param str - текст строки-запроса
     /// @return если парсинг завершился успешно, возвращает идентификатор команды, иначе - CathegoryRocCommand::UNKNOWN
     CathegoryRocCommand::T parseURI( std::string& str );

private:

     /// Очистить семантические данные
     void clearSemanticValues();

     /// Получить список пользователей из базы
     void getUserList();

     /// Добавить пользователя
     void addUser();

     /// Получить информацию о пользователе из данных авторизации
     void getSelfInfo();

     /// Получить информацию о заданном пользователе для компании
     void getUserInfo();

     /// Получить информацию об организации
     void getCompany();

     /// Добавить устройство
     void addDevice();

     /// Получить список устройств пользователя
     void getDevices();

     /// Получить информацию об устройстве
     void getDeviceInfo();

     /// Подтвердить почту пользователя - выполнение
     void confirmEmail();

     /// Отправить ссылку на сброс пароля
     void sendPasswordResetUri();

     /// Обработать команду восстановления пароля (переход по ссылке)
     void restorePassword();

     /// Получить токен для фактического сброса пароля
     void postPasswordResetTokens();

     /// Задать пароль пользователя
     void setUserPassword();

     /// Обновить информацию о пользователе
     void updateUserInfo();

     /// Удаление пользователя
     void deleteUser();

     /// Получить лог событий организации
     void getLogRecords();

     /// Выслать пользователю информацию по получению ключевой информации
     void sendNotify();

     /// Получить DST-файл для устройства
     void getDstx();

     /// Получить статус подготовки DST файла для устройства
     void getDstxStatus();

private:
     RocMethodsContext& rocMethods_;

     /// Фильтр лога событий (время, ISO 8601) - возвращать события с указанного времени включительно
     std::string filterLogFrom_ = std::string();

     /// Фильтр лога событий (время, ISO 8601) - возвращать до указанного времени
     std::string filterLogTo_ = std::string();

     /// Фильтр текстовых данных для запроса пользователей
     std::string filterText_ = std::string();

     /// Фильтр символов для запроса устройств
     std::string filterHwid_ = std::string();

     /// Семантические значения парсера, представимые целыми числами
     RocParserIntegralSValue sval_ = RocParserIntegralSValue();

     /// Статус выполнения последней команды
     CathegoryRocCommand::T result_ = CathegoryRocCommand::UNKNOWN;

     /// Грамматика Spirit::Qi для ROC API
     std::unique_ptr< RocGrammar > g_;
};


/// Класс для преобразования семантических значений в строковое и числовое представление
class SvalConvert
{
public:
     /// Получить строковое представление названия поля JSON-объекта "User"
     static std::string userFields( FilterUserFields::T cat );

     /// Получить строковое представление названия поля JSON-объекта "Device"
     static std::string deviceFields( FilterDeviceFields::T cat );

     /// Получить строковое представление названия поля JSON-объекта "Event"
     static std::string logFields( FilterLogFields::T cat );

     /// Получить строковое представление названия поля JSON-объекта "Company"
     static std::string companyFields( FilterCompanyFields::T cat );

     /// Получить строковое представление события
     static std::string event( CathegoryEvent::T cat );

     /// Получить строковое представление кода фильтра статусов
     static std::string status( CathegoryStatus::T cat );

     /// Получить строковое представление кода фильтра операционных систем
     static std::string os( CathegoryOs::T cat );

     /// Получить числовое представление фильтра операционных систем
     static bool osFromStr( CathegoryOs::T& dest, const std::string& cat );

     /// Получить полное название HTTP-запроса
     static std::string httpMethod( HttpMethodTok::T cat );

     /// Получить лексему для парсера, представляющую тип HTTP-запроса
     static HttpMethodTok::TokStr httpMethodToken( HttpMethodTok::T cat );

     /// Получить код типа HTTP-запроса
     static HttpMethodTok::T httpMethodTokenId( const std::string& tok );

     /// Получить код ошибки по идентификатору, полученному в JSON-ответе из вызова хранимой процедуры БД
     static RocErrorCodes::T dbErrorCodeId( const std::string& dbStr );

     /// Получить описание ошибки для поля "message"
     static std::string rocErrorMessage( RocErrorCodes::T id );
};


} // namespace roc
} // namespace itcs

#endif // ROC_PARSER_H
