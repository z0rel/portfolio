#ifndef ORADMT2CALL_H
#define ORADMT2CALL_H

#include <stdlib.h>

typedef bool (*Ora2CallConverter)( char *& pbuf, size_t & bufsize, const void * value, size_t size );

bool dateToLinT      ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> decimals
bool intervalToLinT  ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> decimals
bool timestampToLinT ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> decimals

bool numToIntLinT    ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> int
bool numToRealLinT   ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> real
bool numToNumLinT    ( char *& pbuf, size_t & bufsize, const void * value, size_t size ); // -> decimals
bool xmlToLinT       ( char *& pbuf, size_t & bufsize, const void * value, size_t size );
// Все, что конвертится в DT_CHAR DT_BYTE DT_VARCHAR DT_VARBYTE DT_NCHAR DT_NVARCHAR
bool rawToLinT       ( char *& pbuf, size_t & bufsize, const void * value, size_t size );
// stubs
bool blobToLinT      ( char *& pbuf, size_t & bufsize, const void *      , size_t      );
bool clobToLinT      ( char *& pbuf, size_t & bufsize, const void *      , size_t      );
bool stubToLinT      ( char *& pbuf, size_t & bufsize, const void *      , size_t      );

bool nullToLinT      ( char *& pbuf, size_t & bufsize, const void *      , size_t      );
bool defaultToLinT   ( char *& pbuf, size_t & bufsize, const void *      , size_t      );

bool varcharToLinT   ( char *& pbuf, size_t & bufsize, const void * value, size_t size );

#endif // ORADMT2CALL_H
