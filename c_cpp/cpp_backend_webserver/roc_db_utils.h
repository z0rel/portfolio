/// @file
/// @brief Вспомогательные функции и классы для работы c PostgreSQL и сервисами Thrift
#pragma once
#ifndef ROC_DB_UTILS_H
#define ROC_DB_UTILS_H

#include <memory>
#include <utility>
#include <assert.h>
#include <initializer_list>

#include <libpq-fe.h>

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"


#include <boost/shared_ptr.hpp>

#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include <NSMSService.h>
#include <NotifyService.h>

#pragma GCC diagnostic pop

#include <roc_parser_dictionary.h>
#include <roc_db_context.h>
#include <base_interfaces/uni_logger_module.h>

namespace itcs
{
namespace roc
{


/// Обертка над результатом операции с PostgreSQL, для упрощенной проверки статуса и автоосвобождения
class DBResult
{
public:
     /// Сформировать результат операции по результату выполненной команды
     explicit DBResult( PGconn* conn, PGresult* res, ExecStatusType waitStatus, const std::string& errMessage,
                        const std::string& operatorName );

     /// Выполнить команду и сформировать результат операции
     explicit DBResult( PGconn* conn, PreparedOperator& op, const char* const* vals, const int* lengths,
                        const int* formats, int resultFormat, ExecStatusType waitStatus,
                        const std::string& errMessage );

     /// Выполнить команду и сформировать результат операции
     explicit DBResult( PGconn* conn, PreparedOperator& op,
                        const char* const* vals,
                        const std::initializer_list<int>& lengths,
                        const std::initializer_list<int>& formats,
                        int resultFormat,
                        ExecStatusType waitStatus, const std::string& errMessage );

     /// Выполнить команду и сформировать результат операции
     explicit DBResult( PGconn* conn, const std::string& query, const int nargs, const Oid *paramTypes,
                        const int* formats, const int* lengths, const char** vals, int resultFormat,
                        ExecStatusType waitStatus, const std::string& errMessage );
     PGresult* get() const;
     operator PGresult*() const;

     ExecStatusType status() const;
     bool           success() const;
     operator bool() const;

private:
     typedef std::shared_ptr< PGresult > PtrResult;

     /// Копирование запрещено
     DBResult( const DBResult& ) = delete;

     /// Присваивание запрещено
     DBResult& operator=( const DBResult& ) = delete;

     /// Перемещение запрещено
     DBResult( DBResult&& ) = delete;

     /// Перемещение запрещено
     DBResult& operator=( DBResult&& ) = delete;

private:
     PtrResult      res_        = PtrResult();
     ExecStatusType status_     = PGRES_EMPTY_QUERY;
     ExecStatusType waitStatus_ = PGRES_EMPTY_QUERY;
};


/// Вспомогательный класс для логгирования исполнения функций
class TraceHelper
{
public:
     TraceHelper( const char* method );
     ~TraceHelper();

     /// установить флаг неуспешности завершения
     void setFail();

private:
     TraceHelper& operator=( const TraceHelper& ) = delete;
     TraceHelper& operator=( TraceHelper&& ) = delete;
     TraceHelper( const TraceHelper& ) = delete;
     TraceHelper( TraceHelper&& ) = delete;

     /// Название функции
     const char* methodName_;
     /// Флаг успешности завершения функции
     bool isSuccess_ = true;
};



/// Вывести подробное описание ошибки
void describeDbError( const std::string& prefix, const std::string& operatorName, PGresult* result, PGconn* conn );


/// Преобразовать string в формат аргумента SQL оператора.
const char* pgValue( const std::string& str );


/// Преобразовать int в формат аргумента SQL оператора. Значение нужно заранее перевести в сетевой порядок байт.
const char* pgValue( unsigned int& val );


/// Преобразовать bigint в формат аргумента SQL оператора. Значение нужно заранее перевести в сетевой порядок байт.
const char* pgValue( pg_int64& val );


/// Вспомогательный класс для шаблонных преобразований типов C++ в формат аргумента SQL-оператора
class PgFormatter
{
public:

};


/// Преобразовать string в формат аргумента SQL оператора.
const char* operator<< ( const PgFormatter&, const std::string& val );


/// Преобразовать int в формат аргумента SQL оператора.
const char* operator<< ( const PgFormatter&, const unsigned int& val );


/// Преобразовать bigint в формат аргумента SQL оператора.
const char* operator<< ( const PgFormatter&, const pg_int64& val );


/// Упаковать указатель на данные без преобразования
const char* operator<< ( const PgFormatter&, const char* val );


/// Преобразовать необязательную строку в формат аргумента SQL оператора.
const char* operator<< ( const PgFormatter&, const std::pair< std::string, bool >& optStr );


/// Шаблон для короткой упаковки списка значений при вызове функции исполнения SQL-команды
template<typename... Ts>
inline std::array< const char*, sizeof...(Ts) > pgPack( const Ts&... args){
    std::array< const char*, sizeof...(Ts) > result = {{ (PgFormatter() << args)... }};
    return result;
}


/// Шаблон для единообразного преобразования хостового порядка байт в сетевой
template < typename T >
inline T hostToNet( T value );


/// Шаблон для единообразного преобразования сетевого байт в хостовый
template < typename T >
inline T netToHost( T value );


/// Шаблон для единообразного преобразования полученного из базы значения в тип целевой переменной.
template < typename T >
inline bool dbToHost( T& dest, const RocDbContext* ctx, DBResult& val, int field, int row );


/// Специализация преобразования знаковых 64х разрядных целых из хостового порядка байт в сетевой
template <>
inline int64_t hostToNet< int64_t >( int64_t value )
{
     return htobe64( value );
}


/// Специализация преобразования беззнаковых 64х разрядных целых из хостового порядка байт в сетевой
template <>
inline uint64_t hostToNet< uint64_t >( uint64_t value )
{
     return htobe64( value );
}


/// Специализация преобразования знаковых 32х разрядных целых из хостового порядка байт в сетевой
template <>
inline int32_t hostToNet< int32_t >( int32_t value )
{
     return htobe32( value );
}


/// Специализация преобразования беззнаковых 32х разрядных целых из хостового порядка байт в сетевой
template <>
inline uint32_t hostToNet< uint32_t >( uint32_t value )
{
     return htobe32( value );
}


/// Специализация преобразования знаковых 64х разрядных целых из сетевого порядка байт в хостовый
template <>
inline int64_t netToHost< int64_t >( int64_t value )
{
     return be64toh( value );
}


/// Специализация преобразования беззнаковых 64х разрядных целых из сетевого порядка байт в хостовый
template <>
inline uint64_t netToHost< uint64_t >( uint64_t value )
{
     return be64toh( value );
}


/// Специализация преобразования знаковых 32х разрядных целых из сетевого порядка байт в хостовый
template <>
inline int32_t netToHost< int32_t >( int32_t value )
{
     return be32toh( value );
}


/// Специализация преобразования беззнаковых 32х разрядных целых из сетевого порядка байт в хостовый
template <>
inline uint32_t netToHost< uint32_t >( uint32_t value )
{
     return be32toh( value );
}


/// Вспомогательная функция для проверки значения из базы на равенство NULL-значению
inline bool dbValIsNull( DBResult& val, Oid valOid, int row, int field )
{
     if ( PQgetisnull( val, row, field ) )
     {
          return true;
     }
     assert( PQftype( val, field ) == valOid );
     return false;
}


/// Вспомогательная функция для преобразования 32х разрядных целых из базы в хостовый формат
template < typename T >
inline bool dbToHost32( T& dest, DBResult& val, Oid int4Oid, int row, int field )
{
     if ( dbValIsNull( val, int4Oid, row, field ) )
     {
          return false;
     }
     dest = be32toh( *reinterpret_cast< int32_t* >( PQgetvalue( val, row, field ) ) );
     return true;
}


/// Вспомогательная функция для преобразования 64х разрядных целых из базы в хостовый формат
template < typename T >
inline bool dbToHost64( T& dest, DBResult& val, Oid int8Oid, int row, int field )
{
     if ( dbValIsNull( val, int8Oid, row, field ) )
     {
          return false;
     }

     dest = be64toh( *reinterpret_cast< int64_t* >( PQgetvalue( val, row, field ) ) );
     return true;
}


/// Специализация для преобразования знаковых 32х разрядных целых из базы в хостовый формат
template <>
inline bool dbToHost< int32_t >( int32_t& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     return dbToHost32( dest, val, ctx->int4Oid(), row, field );
}


/// Специализация для преобразования беззнаковых 32х разрядных целых из базы в хостовый формат
template <>
inline bool dbToHost< uint32_t >( uint32_t& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     return dbToHost32( dest, val, ctx->int4Oid(), row, field );
}


/// Специализация для преобразования знаковых 64х разрядных целых из базы в хостовый формат
template <>
inline bool dbToHost< int64_t >( int64_t& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     return dbToHost64( dest, val, ctx->int8Oid(), row, field );
}


/// Специализация для преобразования беззнаковых 64х разрядных целых из базы в хостовый формат
template <>
inline bool dbToHost< uint64_t >( uint64_t& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     return dbToHost64( dest, val, ctx->int8Oid(), row, field );
}


/// Специализация для преобразования булевых значений из базы в хостовый формат
template <>
inline bool dbToHost< bool >( bool& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     if ( dbValIsNull( val, ctx->boolOid(), row, field ) )
     {
          return false;
     }

     dest = !memcmp( PQgetvalue( val, row, field ), ctx->boolTrue().c_str(), ctx->boolSize() );
     return true;
}

/// Специализация для преобразования строк из базы в хостовый формат
template <>
inline bool dbToHost< std::string >( std::string& dest, const RocDbContext* ctx, DBResult& val, int row, int field )
{
     if ( dbValIsNull( val, ctx->varcharOid(), row, field ) )
     {
          return false;
     }
     dest.assign( PQgetvalue( val, row, field ), PQgetlength( val, row, field ) );
     return true;
}

/// Преобразовать код ошибки NSMS API в код ошибки ROC API
RocErrorCodes::T transformNsmsError( nsms_api_1_0::Result::type resultCode );


/// Преобразовать идентификатор платформы из ROC API в NSMS API
nsms_api_1_0::Platform::type transformNsmsPlatformId( CathegoryOs::T os );


/// Преобразовать код ошибки NOTIFY API в код ошибки ROC API
RocErrorCodes::T transformNotifyErrors( roc_notify_api_1_0::Result::type resultCode );


/// Контекст RPC-взаимодействия с Nsms и сервисом нотификации
template < typename T >
class RpcContext
{
public:
     typedef apache::thrift::protocol::TBinaryProtocol Protocol;
     typedef apache::thrift::transport::TTransport     Transport;
     typedef T                                         RpcClient;


     inline RpcContext( const std::string& host, unsigned int port );
     inline ~RpcContext();

     /// проверить корректность подключения к NSMS
     inline bool consistance( std::string& destErrMsg );

     /// Уровень устройства
     boost::shared_ptr< Transport > socket;
     /// Транспортный уровень
     boost::shared_ptr< Transport > transport;
     /// Уровень протокола
     boost::shared_ptr< Protocol > protocol;
     /// Интерфейс клиента RPC
     std::unique_ptr< RpcClient > client;

private:
     /// Адрес хоста
     std::string host_;

     /// Порт хоста
     unsigned int port_;
};


template < typename T >
inline RpcContext< T >::RpcContext( const std::string& host, unsigned int port  )
     : socket(    new apache::thrift::transport::TSocket( host, port ) )
     , transport( new apache::thrift::transport::TBufferedTransport( socket ) )
     , protocol(  new apache::thrift::protocol::TBinaryProtocol( transport ) )
     , client(    new RpcClient( protocol ) )
     , host_( host )
     , port_( port )
{

}


template < typename T >
inline RpcContext< T >::~RpcContext()
{
     try
     {
          RpcContext< T >::Transport &tr = *transport;
          if ( tr.isOpen() )
          {
               tr.close();
          }
     }
     catch ( apache::thrift::TException& exc )
     {
         ITCS_ERROR( "RpcContext::~RpcContext error:" << exc.what() );
     }
}


template < typename T >
inline bool RpcContext< T >::consistance( std::string& destErrMsg )
{
     try
     {
          RpcContext::Transport &tr = *transport;
          if ( !tr.isOpen() )
          {
               int sd;
               // Проверка через открытие соединения, чтобы в лог не попадало множество сообщений
               // о недоступности сервиса, которые выдает Thrift
               if ( !openTcpConnection( sd, host_.c_str(), port_, destErrMsg ) )
               {
                    return false;
               }
               else
               {
                    close( sd );
               }
               tr.open();
          }
          return true;
     }
     catch ( apache::thrift::TException& exc )
     {
          ITCS_ERROR( "RpcContext::consistance error:" << exc.what() );
     }
     return false;
}




} // namespace roc
} // namespace itcs


#endif // ROC_DB_UTILS_H
