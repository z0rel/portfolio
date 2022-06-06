#include <uni_log_impl.h>

#include <iostream>
#include <ctime>
#include <time.h>

#include <base_interfaces/uni_logger_module.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wsign-conversion"

LOG_INTERFACE( rollout_center );

LOG_MODULE( rollout_center )

namespace itcs
{
namespace roc
{


namespace roc_logger
{


/// @brief Callback функция, вызываемая в начале создания отладочного сообщения
/// @param[in] iface Указатель на структуру настройки системы логирования
/// @param[in] level Уровень логирования создаваемого сообщения
/// @param[in] func  Имя функции, внутри которой происходит вызов функции логирования (подставляется препроцессором).
/// @param[in] file  Имя файла, где происходит вызов функции (подставляется препроцессором в макросе).
/// @param[in] line  Номер строки в файле, где происходит вызов функции (подставляется препроцессором в макросе).
/// @return Handle отладочного сообщения
int startMsg( const void* /* iface */, const ItcsLoglevel level,
                       const char* /* func  */, const char* /* file */, const int /* line */ )
{
     const char* fmt = nullptr;
     switch ( level )
     {
          case LOGLEVEL_FATAL:
               fmt = "[F %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;

          case LOGLEVEL_ERROR:
               fmt = "[E %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;

          case LOGLEVEL_WARNING:
               fmt = "[W %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;

          case LOGLEVEL_INFO:
               fmt = "[I %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;

          case LOGLEVEL_DEBUG:
               fmt = "[D %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;

          case LOGLEVEL_TRACE:
               fmt = "[T %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;
          default:
               fmt = "[  %H:%M:%S %Y-%m-%d(%w,%a)] ";
               break;
     }

     std::time_t t = std::time( nullptr );
     char mbstr[100];
     std::tm tm;
     if ( std::strftime( mbstr, sizeof(mbstr), fmt, localtime_r( &t, &tm ) ) )
     {
          std::cout << mbstr;
     }
     return 0;
}


/// @brief Callback функция для вывода в лог текстовой строки
/// @param[in] handle отладочного сообщения
/// @param[in] message Выводимая текстовая строка
void writeStr( int /* handle */, const char* message )
{
     std::cout << message;
}


/// @brief Callback функция для завершения создания отладочного сообщения
/// @param[in] handle отладочного сообщения
void endMsg( int /* handle */ )
{
     std::cout << std::endl;
}


/// Вывести данные с некорректным размером
void writeBadSize( const void* data, size_t size )
{
     const uint8_t* start = reinterpret_cast< const uint8_t* >( data );
     const uint8_t* end   = start + size;
     std::cout << "bad size=" << size << " data: ";
     // Манипуляторы не используются, чтобы не менять состояние ostream
     const char* const hexChars = "0123456789ABCDEF";
     for ( ; start != end; ++start )
     {
          std::cout << hexChars[ *start >> 4 ] << hexChars[ *start & 0x0F ] << ' ';
     }
}


/// @brief Callback функция для вывода в лог числа со знаком
/// @param[in] handle отладочного сообщения
/// @param[in] data Адрес переменной с данными для вывода
/// @param[in] size Размер числа в байтах
void writeSigned( int /*handle*/, const void* data, size_t size )
{
     switch ( size )
     {
          case 1:
               std::cout << *reinterpret_cast< const int8_t* >( data );
               break;

          case 2:
               std::cout << *reinterpret_cast< const int16_t* >( data );
               break;

          case 4:
               std::cout << *reinterpret_cast< const int32_t* >( data );
               break;

          case 8:
               std::cout << *reinterpret_cast< const int64_t* >( data );
               break;

          default:
               writeBadSize( data, size );
               break;
     }
}


/// @brief Callback функция для вывода в лог числа без знака
/// @param[in] handle отладочного сообщения
/// @param[in] data Адрес переменной с данными для вывода
/// @param[in] size Размер числа в байтах
void writeUnsigned( int /* handle */, const void* data, size_t size )
{
     switch ( size )
     {
          case 1:
               std::cout << *reinterpret_cast< const uint8_t* >( data );
               break;

          case 2:
               std::cout << *reinterpret_cast< const uint16_t* >( data );
               break;

          case 4:
               std::cout << *reinterpret_cast< const uint32_t* >( data );
               break;

          case 8:
               std::cout << *reinterpret_cast< const uint64_t* >( data );
               break;

          default:
               writeBadSize( data, size );
               break;
     }
}


/// @brief Callback функция для вывода в лог double
/// @param[in] handle отладочного сообщения
/// @param[in] value число для вывода в лог
void writeDouble( int /*handle*/, const double value )
{
     std::cout << value;
}


}


void initLogger( int loglevel )
{
     LOG_PROPERTY( rocLogger );

     rocLogger.logLevel    = static_cast< ItcsLoglevel >( loglevel );
     rocLogger.logStart    = &roc_logger::startMsg;
     rocLogger.logStop     = &roc_logger::endMsg;
     rocLogger.logStr      = &roc_logger::writeStr;
     rocLogger.logDouble   = &roc_logger::writeDouble;
     rocLogger.logSigned   = &roc_logger::writeSigned;
     rocLogger.logUnsigned = &roc_logger::writeUnsigned;

     LOGGER( SetProps, rollout_center )( &rocLogger, true );
}



} // namespace roc
} // namespace itcs

#pragma GCC diagnostic pop
