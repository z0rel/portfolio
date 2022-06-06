#include <roc_db_utils.h>

#include <assert.h>


namespace itcs
{
namespace roc
{


ExecStatusType DBResult::status() const
{
     return status_;
}


bool DBResult::success() const
{
     return status_ == waitStatus_;
}


DBResult::operator bool() const
{
     return success();
}


DBResult::DBResult( PGconn* conn, PGresult* res, ExecStatusType waitStatus, const std::string& errMessage,
                    const std::string& operatorName )
     : res_( res, PQclear )
     , status_( PQresultStatus( res ) )
     , waitStatus_( waitStatus )
{
     try
     {
          assert( res );
          if ( !success() )
          {
               describeDbError( errMessage, operatorName, res, conn );
          }
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: DBResult::DBResult() - exception: " << err.what() << "\n" );
          res_.reset();
     }
}


DBResult::DBResult( PGconn* conn,
                    PreparedOperator& op,
                    const char* const* vals,
                    const int* lengths,
                    const int* formats,
                    int resultFormat,
                    ExecStatusType waitStatus, const std::string& errMessage )
     : DBResult( conn, PQexecPrepared( conn, op.operatorId.c_str(), op.nargs, vals, lengths, formats, resultFormat ),
                 waitStatus, errMessage, op.operatorId )
{

}


DBResult::DBResult( PGconn* conn, PreparedOperator& op,
                    const char* const* vals,
                    const std::initializer_list<int>& lengths,
                    const std::initializer_list<int>& formats,
                    int resultFormat, ExecStatusType waitStatus, const std::string& errMessage )
     : DBResult( conn, op, vals, lengths.begin(), formats.begin(), resultFormat, waitStatus, errMessage )
{

}


DBResult::DBResult( PGconn* conn, const std::string& query, const int nargs, const Oid *paramTypes, const int* formats,
                    const int* lengths, const char** vals, int resultFormat, ExecStatusType waitStatus,
                    const std::string& errMessage )
     : DBResult( conn, PQexecParams( conn, query.c_str(), nargs, paramTypes, vals, lengths, formats, resultFormat ),
                 waitStatus, errMessage, query )
{

}


PGresult* DBResult::get() const
{
     return res_.get();
}


DBResult::operator PGresult*() const
{
     return res_.get();
}


template < typename T, typename F >
void strJoin( std::string& dest, const std::string& delimeter, T container, F fun )
{
     auto it = container.begin();
     if ( it == container.end() )
          return;
     dest.append( fun( *it ) );
     for ( ++it; it != container.end(); ++it )
     {
          std::string str = fun( *it );
          if ( str.empty() )
               continue;
          dest.append( delimeter );
          dest.append( str );
     }
}


template < typename T >
void strJoin( std::string& dest, const std::string& delimeter, T container )
{
     auto it = container.begin();
     if ( it == container.end() )
          return;
     dest.append( *it );
     for ( ++it; it != container.end(); ++it )
     {
          dest.append( delimeter );
          dest.append( *it );
     }
}


/// Вывести подробное описание ошибки
void describeDbError( const std::string& prefix, const std::string& operatorName, PGresult* result, PGconn* conn )
{
     using std::string;
     using std::pair;
     static const std::vector< pair< string, int > > fields =
     {
          { "SEVERITY",           PG_DIAG_SEVERITY           },
          { "SQLSTATE",           PG_DIAG_SQLSTATE           },
          { "MESSAGE_PRIMARY",    PG_DIAG_MESSAGE_PRIMARY    },
          { "MESSAGE_DETAIL",     PG_DIAG_MESSAGE_DETAIL     },
          { "MESSAGE_HINT",       PG_DIAG_MESSAGE_HINT       },
          { "STATEMENT_POSITION", PG_DIAG_STATEMENT_POSITION },
          { "INTERNAL_POSITION",  PG_DIAG_INTERNAL_POSITION  },
          { "CONTEXT",            PG_DIAG_CONTEXT            },
          { "SCHEMA_NAME",        PG_DIAG_SCHEMA_NAME        },
          { "TABLE_NAME",         PG_DIAG_TABLE_NAME         },
          { "COLUMN_NAME",        PG_DIAG_COLUMN_NAME        },
          { "DATATYPE_NAME",      PG_DIAG_DATATYPE_NAME      },
          { "CONSTRAINT_NAME",    PG_DIAG_CONSTRAINT_NAME    },
          { "SOURCE_FILE",        PG_DIAG_SOURCE_FILE        },
          { "SOURCE_LINE",        PG_DIAG_SOURCE_LINE        },
          { "SOURCE_FUNCTION",    PG_DIAG_SOURCE_FUNCTION    }
     };


     if ( conn == nullptr || result == nullptr )
     {
          ITCS_WARNING( prefix << " nullptr error: "
                        << ( conn   == nullptr ? "conn = nullptr " : "")
                        << ( result == nullptr ? "result = nullptr" : "") );
          return;
     }

     auto desrcFun = [result]( pair< string, int > f ) -> string
     {
          const char* res = PQresultErrorField( result, f.second );
          return ( res == nullptr ) ? string() : f.first + ": " + res;
     };

     string dest;
     strJoin( dest, "\n  >>> ", fields, desrcFun );
     ITCS_WARNING( prefix << " operator " << operatorName << " " << PQerrorMessage( conn ) << " " << dest );
}


const char* pgValue( const std::string& str )
{
     return str.empty() ? nullptr : str.c_str();
}


const char* pgValue( unsigned int& val )
{
     return reinterpret_cast< const char* >( &val );
}

const char* pgValue( pg_int64& val )
{
     return reinterpret_cast< const char* >( &val );
}


TraceHelper::TraceHelper( const char* method )
     : methodName_(method)
{
     ITCS_TRACE( methodName_ << " - start" );
}


TraceHelper::~TraceHelper()
{
     ITCS_TRACE( methodName_ << ( isSuccess_ ? " - success" : " - exec error " ) );
}


void TraceHelper::setFail()
{
     isSuccess_ = false;
}


const char* operator<<( const PgFormatter&, const std::string& val )
{
     return pgValue( val );
}


const char*operator<<( const PgFormatter&, const unsigned int& val )
{
     return reinterpret_cast< const char* >( &val );
}


const char* operator<<( const PgFormatter&, const pg_int64& val )
{
     return reinterpret_cast< const char* >( &val );
}


const char* operator<<( const PgFormatter&, const char* val )
{
     return val;
}


const char* operator<<(const PgFormatter&, const std::pair<std::string, bool>& optStr)
{
     return optStr.second ? optStr.first.c_str() : nullptr;
}


RocErrorCodes::T transformNsmsError( nsms_api_1_0::Result::type resultCode )
{
     using nsms_api_1_0::Result;
     switch ( resultCode )
     {
          case Result::NO_RESULT:               return RocErrorCodes::NSMS_BAD_REQUEST_ARGUMENTS;
          case Result::REQUEST_LIMIT_EXCEEDED:  return RocErrorCodes::NSMS_REQUEST_LIMIT_EXCEEDED;
          case Result::ADDRESS_LIMIT_REACHED:   return RocErrorCodes::NSMS_ADDRESS_LIMIT_REACHED;
          case Result::LICENSE_LIMIT_REACHED:   return RocErrorCodes::NSMS_LICENSE_LIMIT_REACHED;
          case Result::NODE_ALREADY_EXIST:      return RocErrorCodes::NSMS_NODE_ALREADY_EXISTS;
          case Result::INTERNAL_SERVER_ERROR:   return RocErrorCodes::NSMS_INTERNAL_SERVER_ERROR;
          default: break;
     }
     assert( 0 ); // такого встречаться не должно
     return RocErrorCodes::SUCCESS;
}


nsms_api_1_0::Platform::type transformNsmsPlatformId( CathegoryOs::T os )
{
     switch ( os )
     {
          case CathegoryOs::WINDOWS_ANY: return nsms_api_1_0::Platform::WINDOWS_ANY;
          case CathegoryOs::IOS_ANY:     return nsms_api_1_0::Platform::IOS_ANY;
          case CathegoryOs::MACOS_ANY:   return nsms_api_1_0::Platform::MACOS_ANY;
          case CathegoryOs::ANDROID_ANY: return nsms_api_1_0::Platform::ANDROID_ANY;
     }
     assert( 0 );
     return nsms_api_1_0::Platform::WINDOWS_ANY;
}


RocErrorCodes::T transformNotifyErrors(roc_notify_api_1_0::Result::type resultCode)
{
     using roc_notify_api_1_0::Result;
     switch ( resultCode )
     {
          case Result::OK:                     assert( 0 ); return RocErrorCodes::SUCCESS;
          case Result::INTERNAL_SERVER_ERROR:  return RocErrorCodes::NOTIFY_INTERNAL_SERVER_ERROR;
          case Result::NO_RESULT:              return RocErrorCodes::NOTIFY_NO_RESULT;
          case Result::REQUEST_LIMIT_EXCEEDED: return RocErrorCodes::NOTIFY_REQUEST_LIMIT_EXCEEDED;
     }
     assert( 0 );
     return RocErrorCodes::SUCCESS;
}


} // namespace roc
} // namespace itcs
