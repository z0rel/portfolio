#ifndef ORAFMT2TEXT_H
#define ORAFMT2TEXT_H

#include <stdio.h>
#include <string>
#include <constants.h>

struct DateTime {
  DateTime()
      : year(0), month(0), day(0), hour(0), minute(0), second(0), fracSecond(0), leadZeros(0) {}
  int year, month, day;
  int hour, minute, second, fracSecond;
  int leadZeros;

  void normalizeFracSeconds();
  void toString(std::string & result);
};

void        varcharToText  ( std::string & result, const void * value, size_t size );

void        dateToText     ( std::string & result, const void * value, size_t size );
void        intervalToText ( std::string & result, const void * value, size_t size );
void        timestampToText( std::string & result, const void * value, size_t size );
void        blobToText     ( std::string & result, const void *      , size_t      );
void        clobToText     ( std::string & result, const void *      , size_t      );
void        xmlToText      ( std::string & result, const void * value, size_t size );
void        clobToText     ( std::string & result, const void *      , size_t      );
void        stubToText     ( std::string & result, const void *      , size_t      );

inline void rawToText      ( std::string & result, const void * value, size_t size );
inline void numberToText   ( std::string & result, const void * value, size_t size );

int  extractXmlOffset( const void * value, const size_t & size );

void dateToObj       ( DateTime   & dateTime, const void * value, size_t size );
void intervalToObj   ( DateTime   & dateTime, const void * value, size_t size );
void timestampToObj  ( DateTime   & dateTime, const void * value, size_t size );

struct FastCharConverter {
  char convTable[0x100][2];
  FastCharConverter() {
    char buf[3];
    for ( int i = 0; i < 0x100; ++i ) {
      sprintf( buf, "%02x", i );
      convTable[i][0] = buf[0];
      convTable[i][1] = buf[1];
    }
  }
};


inline void rawToText( std::string & result, const void * value, size_t size ) {
  static const FastCharConverter fastCharConverter;
  result.reserve( result.size() + (size << 1) /* т.е. * 2 */ + sizeof("HEX('''')") );
  result += "HEX('";
  unsigned char * ptr = (unsigned char*)value;
  for ( unsigned int i = 0; i < size; ++i, ++ptr )
    result.append( fastCharConverter.convTable[*ptr] , 2);
  result += "')";
}


inline void numberToText( std::string & result, const void * value, size_t size ) {
  register int sign    = ~( *(signed char *)value >> 7 );
  register int pow     = ( unsigned char )( ( *(unsigned char *)value ^ sign ) << 1 );
  register int numSub  = ( 101 & sign ) + 1;

  register const unsigned char *numPtr;

  // calc effective length
  register unsigned int numLen = size - 1;
  if( *( (char *)value + numLen ) == 102 )
    numLen--;
  if( !numLen || *(unsigned char *)value == SIGN_MASK ) {
    // that is 0
    result.append(1, '0');
    return;
  }

  register int num     = 0;
  register int leftNum = 0;

  // add sign
  if( sign )
    result.append(1, '-');

  pow -= SIGN_MASK;
  numPtr = (unsigned char *)value + 1;
  if( pow <= 0 ) {
    // set leading 0s after point
    result.append("0.", 2);
    if( pow < 0 )
      result.append( -pow, '0' );
  }
  else {
    // skip leading 0
    num = ( *numPtr - numSub ) ^ sign;
    leftNum = num / 10;
    num %= 10;
    if( !leftNum ) {
      pow--;
      leftNum = num;
      numPtr++; numLen--;
    }
  }

  // main loop
  do {
    if( pow & 1 ) {
      // skip tailing 0 after point
      if( pow < 0 && !num && !numLen )
        break;
    }
    else {
      num = ( *numPtr - numSub ) ^ sign;
      leftNum = num / 10;
      num %= 10;
      numPtr++; numLen--;
    }
    // put next digit
    result += '0' + leftNum;
    leftNum = num;
    pow--;
    // put point when non-0 tail after
    if( !pow && numLen )
      result.append(1, '.');
  } while( ( pow & 1 ) || numLen );

  // put tailing 0s before point
  if( pow > 0 )
    result.append( pow, '0' );
}

#endif // ORAFMT2TEXT_H
