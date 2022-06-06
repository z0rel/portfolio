#include <roc_fcgi_utils.h>

#include <fcgi_config.h>
#include <string>
#include <type_traits>
#include <assert.h>

#include <base_interfaces/uni_logger_module.h>
#include <http_handler_settings.h>
#include <roc_parser.h>


namespace itcs
{
namespace roc
{


UriDecoder::UriDecoder() : decodeTable_()
{
     const char uriPercentEncChars[] = "0123456789ABCDEF";

     decodeTable_.fill( -1 );

     // - 1, т.е. не итерировать элемент '\0'
     for ( unsigned int i = 0; i < std::extent< decltype( uriPercentEncChars ) >::value - 1; ++i )
     {
          decodeTable_[ static_cast< UChar >( uriPercentEncChars[ i ] ) ]            = static_cast< char >( i );
          decodeTable_[ static_cast< UChar >( tolower( uriPercentEncChars[ i ] ) ) ] = static_cast< char >( i );
     }
}


void UriDecoder::decode( std::string& dest, const std::string& src )
{
     using std::string;

     const string::size_type srcLen = src.length();

     if ( srcLen < 3 )
     {
          dest = src;
          return;
     }

     const UChar* srcIter = reinterpret_cast< const UChar* >( src.c_str() );

     // Искать символы '%' во всей строке за исключением последних 2х символов.
     // Последние 2 символа исключаются, т.к. после любого '%' в URI Percent Encoding всегда должно идти
     // 2 hex-символа (RFC1630)
     const UChar* const pEndDec = srcIter + srcLen - 2;

     dest.reserve( dest.length() + srcLen );

     // Используется оператор "меньше", т.к. если конце строки стоит
     // %YY, то после его обработки  - указатель-итератор перескочит на символ,
     // следующий за концом строки
     for ( ; srcIter < pEndDec; ++srcIter )
     {
          char dec1, dec2;
          if ( *srcIter == '%' && -1 != ( dec1 = decodeTable_[ *( srcIter + 1 ) ] )
               && -1 != ( dec2 = decodeTable_[ *( srcIter + 2 ) ] ) )
          {
               dest.push_back( static_cast< char >( ( dec1 << 4 ) + dec2 ) );
               ++srcIter;
               ++srcIter;
          }
          else
          {
               dest.push_back( *reinterpret_cast<const char*>( srcIter ) );
          }
     }
     if ( srcIter == pEndDec )
     {
          // Последние 2 символа
          dest.push_back( *reinterpret_cast<const char*>( srcIter++ ) );
          dest.push_back( *reinterpret_cast<const char*>( srcIter++ ) );
     }
}


typedef std::array< unsigned int, 3 > DecodedTuple;
typedef std::array< unsigned int, 4 > EncodedTuple;

Base64Decoder::Base64Decoder()
{
     // Массив символов кодировки Base64
     const Char base64_[ Base64Size + 1 ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

     for ( unsigned int i = 0; i < Base64Size; ++i )
     {
          UChar index = static_cast< UChar >( base64_[ i ] );
          b64charToIndex_[ index ] = i;
          isBase64Char_  [ index ] = true;
     }
}


void Base64Decoder::encode( std::string& dest, const char* inSrc, unsigned long srcLen )
{
     // Массив символов кодировки Base64
     const Char base64_[ Base64Size + 1 ] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

     DecodedTuple buf;
     const UChar* bytesToEncode = reinterpret_cast< const UChar* >( inSrc );

     unsigned int i = 0;
     while ( srcLen-- )
     {
          buf[ i++ ] = *( bytesToEncode++ );
          if ( i == buf.size() )
          {
               // Выполнить разбиение трехбайтного кортежа на четыре 6-и битные части, являющиеся индексами в base64
               dest.push_back( base64_[   ( buf[ 0 ] & B2I_11 ) >> 2 ] );
               dest.push_back( base64_[ ( ( buf[ 0 ] & B2I_12 ) << 4 ) | ( ( buf[ 1 ] & B2I_22 ) >> 4 ) ] );
               dest.push_back( base64_[ ( ( buf[ 1 ] & B2I_23 ) << 2 ) | ( ( buf[ 2 ] & B2I_33 ) >> 6 ) ] );
               dest.push_back( base64_[     buf[ 2 ] & B2I_34 ] );
               i = 0;
          }
     }

     if ( i )
     {
          std::fill( buf.begin() + i, buf.end(), 0 );

          EncodedTuple encodedTuple;
          // Выполнить разбиение трехбайтного кортежа на четыре 6-и битные части, являющиеся индексами в base64
          encodedTuple[ 0 ] =   ( buf[ 0 ] & B2I_11 ) >> 2;
          encodedTuple[ 1 ] = ( ( buf[ 0 ] & B2I_12 ) << 4 ) | ( ( buf[ 1 ] & B2I_22 ) >> 4 );
          encodedTuple[ 2 ] = ( ( buf[ 1 ] & B2I_23 ) << 2 ) | ( ( buf[ 2 ] & B2I_33 ) >> 6 );
          encodedTuple[ 3 ] =     buf[ 2 ] & B2I_34;

          for ( unsigned int j = 0; j < i + 1; j++ )
          {
               dest.push_back( base64_[ encodedTuple[ j ] ] );
          }

          while ( i++ < buf.size() )
          {
               dest.push_back( '=' );
          }
     }
}


void Base64Decoder::decode( std::string& dest, const char* src, unsigned long srcLen )
{
     EncodedTuple buf;
     const UChar* bytesToDecode = reinterpret_cast< const UChar* >( src );
     const UChar* endChar =  bytesToDecode + srcLen;

     unsigned int i = 0;
     while ( bytesToDecode != endChar && *bytesToDecode != '=' && isBase64Char_[ *bytesToDecode ] )
     {
          buf[ i++ ] = b64charToIndex_[ *( bytesToDecode++ ) ];
          if ( i == buf.size() )
          {
               // Сжать четыре 6-и битных индекса кодировки base64 в 3 байта, являющиеся декодированными данными
               dest.push_back( static_cast< Char >(   ( buf[ 0 ] << 2 )            | ( ( buf[ 1 ] & I2B_21 ) >> 4 ) ) );
               dest.push_back( static_cast< Char >( ( ( buf[ 1 ] & I2B_22 ) << 4 ) | ( ( buf[ 2 ] & I2B_32 ) >> 2 ) ) );
               dest.push_back( static_cast< Char >( ( ( buf[ 2 ] & I2B_33 ) << 6 ) |     buf[ 3 ] ) );
               i = 0;
          }
     }

     if ( i )
     {
          std::fill( buf.begin() + i, buf.end(), 0 );

          DecodedTuple decodedTuple;
          decodedTuple[ 0 ] =   ( buf[ 0 ] << 2 )            | ( ( buf[ 1 ] & I2B_21 ) >> 4 );
          decodedTuple[ 1 ] = ( ( buf[ 1 ] & I2B_22 ) << 4 ) | ( ( buf[ 2 ] & I2B_32 ) >> 2 );
          decodedTuple[ 2 ] = ( ( buf[ 2 ] & I2B_33 ) << 6 ) |     buf[ 3 ];

          for ( unsigned int j = 0; j < i - 1; j++ )
          {
               dest.push_back( static_cast< Char >( decodedTuple[ j ] ) );
          }
     }
}


void getRequestBody( std::string& dest, FCGX_Request& req, const size_t bodyLimit )
{
     std::array< char, 1024 > buf;
     int        rdSize = 0;

     dest.clear();

     while ( ( rdSize = FCGX_GetStr( const_cast< char* >( buf.data() ),
                                     static_cast< int >( buf.size() ), req.in ) ) > 0 )
     {
          if ( bodyLimit > dest.size() + static_cast< unsigned int >( rdSize ) )
          {
               dest.append( buf.data(), static_cast< unsigned int >( rdSize ) );
          }
          else
          {
               assert( bodyLimit > dest.size()
                       && static_cast< unsigned int >( rdSize ) >= ( bodyLimit - dest.size() ) );
               dest.append( buf.data(), static_cast< unsigned int >( rdSize ) - ( bodyLimit - dest.size() ) );
               ITCS_WARNING( "WARNING: request body size greather than limit. Body was stripped" );
               break;
          }
          if ( buf.size() > static_cast< unsigned int >( rdSize ) )
          {
               break;
          }
     }
}


void getErrorJsonBody( std::string& dst, unsigned int code, const std::string& message )
{

     assert( message.find( '"' ) == std::string::npos );
     // TODO: реализовать escaping или assert на кавычки внутри message
     dst.append( "\"message\":\"" );
     dst.append( message );
     dst.append( "\",\"code\":" );
     dst.append( std::to_string( code ) );
}


void getErrorJsonOpened( std::string& dst, unsigned int code, const std::string& message )
{
     dst.append( "{\"error\":{" );
     getErrorJsonBody( dst, code, message );
}


void getErrorJson( std::string& dst, unsigned int code, const std::string& message )
{
     getErrorJsonOpened( dst, code, message );
     dst.append( "}}" );
}


void getErrorJson( std::string& dst, unsigned int code )
{
     getErrorJson( dst, code, SvalConvert::rocErrorMessage( static_cast< RocErrorCodes::T >( code ) ) );
}


} // namespace roc
} // namespace fcgi
