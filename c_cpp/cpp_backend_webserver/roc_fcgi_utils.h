/// @file
/// @brief Вспомогательные функции для модулей fastcgi_external_server
#pragma once
#ifndef ROC_FCGI_UTILS_H
#define ROC_FCGI_UTILS_H

#include <string>
#include <exception>
#include <array>
#include <limits>

#include <fcgiapp.h>


namespace itcs
{
namespace roc
{

class HttpHandlerSettings;

/// Класс для декодирования URI Percent encoding
class UriDecoder
{
public:
     typedef unsigned char UChar;

     UriDecoder();

     /// Декодировать URI-encoding в текст
     void decode( std::string& dest, const std::string& src );

private:
     /// Таблица для декодирования hex-символов в числовые значения
     std::array< char, 1 << ( 8 * sizeof( UChar ) ) > decodeTable_;
};


/// Класс для кодирования и декодирования base64
class Base64Decoder
{
public:
     Base64Decoder();

     void encode( std::string& dest, const char* src, unsigned long srcLen );
     void decode( std::string& dest, const char* src, unsigned long srcLen );

private:
     typedef char Char;
     typedef unsigned char UChar;

     enum CodingValues
     {
          /// Размер таблиц для перекодирования символов
          DecodeTablesSize = 1 << ( 8 * sizeof( Char ) ),

          /// Количество символов в кодировке Base64
          Base64Size = 64,

          // Маски для перекодирования символов
          B2I_11 = 0xfc, // 0b11111100
          B2I_12 = 0x03, //       0b11
          B2I_22 = 0xf0, // 0b11110000
          B2I_23 = 0x0f, //     0b1111
          B2I_33 = 0xc0, // 0b11000000
          B2I_34 = 0x3f, //   0b111111
          I2B_21 = 0x30, //   0b110000
          I2B_22 = 0x0f, //     0b1111
          I2B_32 = 0x3c, //   0b111100
          I2B_33 = 0x03  //       0b11
     };

     /// Таблица преобразования значения символа base64 в его индекс
     unsigned int b64charToIndex_[ DecodeTablesSize ] = { std::numeric_limits< unsigned int >::max() };

     /// Таблица признаков символов, относящихся к кодировке base64
     bool isBase64Char_[ DecodeTablesSize ] = { false };
};



/// Получить json объект "error" с кодом ошибки, описание извлечь автоматически
void getErrorJson( std::string& dst, unsigned int code );

/// Получить json объект "error" с кодом ошибки и описанием
void getErrorJson( std::string& dst, unsigned int code, const std::string& message );

/// Получить json объект "error" с кодом ошибки и описанием, но не закрывать JSON-объект двумя фигурными скобками
/// (чтобы туда можно было добавить еще поля)
void getErrorJsonOpened( std::string& dst, unsigned int code, const std::string& message );

/// Получить тело json объекта "error"
void getErrorJsonBody( std::string& dst, unsigned int code, const std::string& message );

/// Получить тело запроса с учетом ограничения по размеру
void getRequestBody( std::string& dst, FCGX_Request& request_fcgx, const size_t bodyLimit );


/// @brief Получить http-заголовки для json-ответа
inline constexpr const char* getJsonResponseHeadersOk()
{
     return "Status: 200 OK \r\n"
            "Content-Type: application/json; charset=UTF-8\r\n"
            "\r\n";
}

/// @brief Получить http-заголовки для json-ответа
inline constexpr const char* getJsonResponseHeadersBadRequest()
{
     return "Status: 400 Bad request \r\n"
            "Content-Type: application/json; charset=UTF-8\r\n"
            "\r\n";
}


} // namespace roc
} // namespace itcs

#endif // ROC_FCGI_UTILS_H
