/// @file
/// @brief Интерфейс семантических значений парсера
#pragma once
#ifndef ROC_PARSER_DICTIONARY_H
#define ROC_PARSER_DICTIONARY_H

#include <string>

namespace itcs
{
namespace roc
{

/// @brief Абстрация категории выполненной команды
class CathegoryRocCommand
{
public:
     enum T
     {
          /// Получить информацию о пользователе
          GET_INFO_ME,
          /// Получить информацию об устройстве
          GET_DEVICE_INFO,
          /// Выполнить подтверждение почты пользователя
          CONFIRM_EMAIL,
          /// Запросить сброс пароля пользователя - отправить ссылку
          PASSWORD_RESET_REQUEST,
          /// Обработать сброс пароля пользователя
          RESTORE_PASSWORD,
          /// Получить токен для фактического сброса пароля
          POST_TOKENS,
          /// Задать пароля пользователя
          SET_USER_PASSWORD,
          /// Удаление пользователя
          DELETE_USER,
          /// Получить объект Company
          GET_COMPANY,
          /// Получить лог событий организации
          GET_LOG_RECORDS,
          /// Получить список пользователей организации
          GET_USERS_LIST,
          /// Получить информацию о пользователе
          GET_USER_INFO,
          /// Выслать пользователю информацию по получению ключевой информации
          POST_NOTIFY,
          /// Получить список устройств пользователя
          GET_DEVICES,
          /// Добавить устройство пользователя
          POST_DEVICE,
          /// Получить DST файл для устройства
          GET_DSTX,
          /// Получить статус подготовки DST файла для устройства
          GET_DSTX_STATUS,
          /// Добавить пользователя
          POST_USER,
          /// Обновить информацию о пользователе
          PUT_USER,
          /// Незаданная команда
          UNKNOWN
     };
};


/// @brief Абстрация поля фильтра операционной системы
class CathegoryOs
{
public:
     enum T
     {
          IOS_ANY     = ( 1 << 0 ),
          ANDROID_ANY = ( 1 << 1 ),
          WINDOWS_ANY = ( 1 << 2 ),
          MACOS_ANY   = ( 1 << 3 )
     };

     enum MaxBit
     {
          N = 4
     };
};


/// @brief Абстрация поля фильтра статуса пользователя
class CathegoryStatus
{
public:
     enum T
     {
          NEW         = ( 1 << 0 ),
          IN_PROGRESS = ( 1 << 1 ),
          DST_SENT    = ( 1 << 2 ),
          BLOCKED     = ( 1 << 3 )
     };

     enum MaxBit
     {
          N = 4
     };
};


/// @brief Абстрация события в логе событий
class CathegoryEvent
{
public:
     enum T
     {

          CREATE_USER   = ( 1 << 0 ),
          CREATE_DEVICE = ( 1 << 1 ),
          DST_REQUEST   = ( 1 << 2 ),
          EDIT_USER     = ( 1 << 3 ),
          REMOVE_USER   = ( 1 << 4 ),
          BLOCK_USER    = ( 1 << 5 ),
          UNBLOCK_USER  = ( 1 << 6 ),
          EDIT_DEVICE   = ( 1 << 7 ),
          REMOVE_DEVICE = ( 1 << 8 )
     };

     enum MaxBit
     {
          N = 9
     };
};


/// @brief Абстрация идентификаторов полей объекта User
class FilterUserFields
{
public:
     enum T
     {
          ID            = ( 1 << 0 ),
          LOGIN         = ( 1 << 1 ),
          EMAIL         = ( 1 << 2 ),
          PHONE         = ( 1 << 3 ),
          DESCRIPTION   = ( 1 << 4 ),
          STATUS        = ( 1 << 5 ),
          COMPANY_ROLES = ( 1 << 6 )
     };

     enum MaxBit
     {
          N = 7
     };
};


/// @brief Абстрация идентификаторов полей объекта Device
class FilterDeviceFields
{
public:
     enum T
     {
          ID   = ( 1 << 0 ),
          NAME = ( 1 << 1 ),
          HWID = ( 1 << 2 ),
          OS   = ( 1 << 3 )
     };

     enum MaxBit
     {
          N = 4
     };
};


/// @brief Абстрация идентификаторов полей объекта Company
class FilterCompanyFields
{
public:
     enum T
     {
          ID   = ( 1 << 0 ),
          NAME = ( 1 << 1 ),
     };

     enum MaxBit
     {
          N = 2
     };
};


/// @brief Абстрация идентификаторов полей объекта LogRecord
class FilterLogFields
{
public:
     enum T
     {
          TIME       = ( 1 << 0 ),
          ACTOR      = ( 1 << 1 ),
          EVENT_TYPE = ( 1 << 2 ),
          USER_ID    = ( 1 << 3 ),
          USER_EMAIL = ( 1 << 4 ),
          USER_LOGIN = ( 1 << 5 ),
          DEVICE_OS  = ( 1 << 6 )
     };

     enum MaxBit
     {
          N = 7
     };
};


/// @brief Абстрация HTTP-метода для единообразного выбора действия по результатам парсинга
class HttpMethodTok
{
public:
     typedef char TokStr;
     enum T
     {
          POST,
          GET,
          DELETE,
          PUT,
          UNKNOWN
     };
};


/// @brief Статус формирования DST-файла
class DstStatus
{
public:
     enum T
     {
          /// Dst файл не запрошен
          DST_NOT_REQUESTED,
          /// Dst файл не готов
          DST_NOT_READY,
          /// Dst файл готов
          DST_READY,
          /// Ошибка запроса статуса в базе
          DB_ERROR
     };
};


/// @brief Идентификаторы кодов ошибок
class RocErrorCodes
{
public:
     enum T
     {
          /// Нет ошибки
          SUCCESS                         = 0,
          /// Ошибка валидации: некорректный формат телефона
          BAD_PHONE_FORMAT                = 101,
          /// Ошибка валидации: некорректный формат логина
          BAD_LOGIN_FORMAT                = 102,
          /// Ошибка валидации: некорректный email
          BAD_EMAIL_FORMAT                = 103,
          /// Ошибка валидации: некорректный формат JSON
          BAD_JSON_FORMAT                 = 104,
          /// Ошибка валидации: Некорректное значение платформы
          BAD_OS_IDENTIFIER               = 105,
          /// Ошибка валидации: Некорректное или пустое имя устройства
          BAD_DEVICE_NAME                 = 106,
          /// Ошибка валидации: Некорректный или пустой идентификатор устройства
          BAD_DEVICE_HWID                 = 107,
          /// Ошибка валидации: Не задан идентификатор пользователя
          EMPTY_USER_ID                   = 108,
          /// Ошибка валидации: Некорректный формат пароля
          BAD_PASSWORD_FORMAT             = 109,
          /// Ошибка целостности данных в БД: попытка добавить существующий логин
          LOGIN_ALREADY_EXISTS            = 130,
          /// Ошибка целостности данных в БД: попытка добавить существующий email
          EMAIL_ALREADY_EXISTS            = 131,
          /// Ошибка целостности данных в БД: попытка добавить существующий телефон
          PHONE_ALREADY_EXISTS            = 132,
          /// Ошибка целостности данных в БД: попытка добавить пользователя с несуществующей компанией
          COMPANY_NOT_EXISTS              = 133,
          /// Ошибка целостности данных в БД: попытка добавить пользователя с пустой компанией
          EMPTY_COMPANY_ID                = 134,
          /// Ошибка целостности данных в БД: Пользователь уже существует
          USER_ALREADY_EXISTS             = 135,
          /// Ошибка целостности данных в БД: Пользователь с таким идентификатором не существует
          USER_NOT_EXISTS                 = 136,
          /// Адрес VipNet Id для устройства еще не создан. Попробуйте повторить запрос позже (через 15-20 секунд)
          DEVICE_VIPNET_ID_IS_NOT_READY   = 137,
          /// Ошибка целостности данных в БД: Идентификатор запроса ключей не задан в базе данных для устройства.
          /// Возможно, запрос на создание ключей не был выполнен.
          DST_REQUEST_ID_IS_EMPTY         = 138,
          /// Ошибка целостности данных в БД: Идентификатор запроса на добавление устройства не задан в базе данных.
          /// Возможно, запрос на добавление устройства в NSMS не был выполнен.
          DEVICE_REQUEST_ID_IS_EMPTY      = 139,
          /// Ошибка целостности данных: Идентификатор запроса ключей уже существует в базе для другого устройства.
          /// Либо ключи были получены другим способом и данный request_id в базе не был обновлен, либо база NSMS
          /// была изменена и данные в части идентификаторов запросов теперь несогласованны.
          NSMS_REQUEST_ID_ALREADY_EXISTS  = 140,
          /// Некорректный формат структуры ответа после запроса в бд данных авторизации для получения dst
          BAD_DB_NSMS_VIPNET_ID           = 141,
          /// Некорректный формат структуры ответа после запроса в бд данных авторизации для получения статуса dst
          BAD_DB_NSMS_DST_REQUEST_ID      = 142,
          /// Слишком большая длина одного из аргументов при выполнении операции модификации данных в базе
          BAD_ARGUMENT_LENGTH             = 143,
          /// Некорректный формат структуры ответа после запроса в бд токена доступа к NSMS
          BAD_DB_NSMS_TOKEN_AUTH          = 144,


          /// Ошибка выполнения подготовленного оператора
          EXEC_DB_OPERATOR_ERROR          = 159,
          /// Исключение в общем цикле обработки запросов
          EXCEPTION                       = 160,
          /// В запросе не были переданы данные аутентификации, либо если они не верны
          AUTHORIZATION_ERROR             = 190,

          /// NSMS/Administrator недоступны
          NSMS_UNREACHIBLE                = 200,
          /// Исключение Thrift при взаимодействии с NSMS
          NSMS_THRIFT_EXCEPTION           = 201,
          /// Что-то пошло не так. Нужно проверить лог сервера NSMS/Administrator.
          NSMS_INTERNAL_SERVER_ERROR      = 202,
          /// Ошибка авторизации NSMS, либо какие-то аргументы заданы некорректно
          NSMS_BAD_REQUEST_ARGUMENTS      = 203,
          /// NSMS перегружен запросами
          NSMS_REQUEST_LIMIT_EXCEEDED     = 204,
          /// Истек срок действия лицензии на NSMS
          NSMS_LICENSE_LIMIT_REACHED      = 205,
          /// Доступных ViPNet адресов больше нет. Нужно попытаться удалить бесполезные узлы.
          NSMS_ADDRESS_LIMIT_REACHED      = 206,
          /// NSMS сообщил о готовности DST при запросе ключей, но ключи не вернул
          NSMS_EMPTY_READY_DST            = 207,
          /// Узел с таким именем уже зарегистрирован. Нужно попробовать задать другое имя.
          NSMS_NODE_ALREADY_EXISTS        = 208,
          /// Запрос статуса DST файла не вернул DST-файла
          NSMS_EMPTY_DST_STATUS_RESPONSE  = 210,


          /// Сервер нотификации недоступен
          NOTIFY_UNREACHIBLE              = 220,
          /// Исключение Thrift при отправке уведомлений
          NOTIFY_THRIFT_EXCEPTION         = 221,
          /// Что-то пошло не так. Нужно проверить лог сервера нотификации.
          NOTIFY_INTERNAL_SERVER_ERROR    = 222,
          /// Пустой результат или ошибка аутентификации. Все остальные поля являются пустыми.
          NOTIFY_NO_RESULT                = 223,
          /// Слишком много запросов. Попытайтесь уменьшить их интенсивность или будете забанены.
          NOTIFY_REQUEST_LIMIT_EXCEEDED   = 224,
          /// Множественные ошибки
          MULTI_ERROR                     = 399,
          /// Некорректный URI-запрос
          BAD_REQUEST                     = 400
     };
};


} // namespace roc
} // namespace itcs


#endif // ROC_PARSER_DICTIONARY_H
