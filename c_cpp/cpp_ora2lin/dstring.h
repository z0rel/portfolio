#ifndef __DSTRING_H
#define __DSTRING_H

#if defined(_MSC_VER) && (_MSC_VER < 1400)
#pragma warning (disable : 4786)
#endif

#include <string.h>
#include <vector>
#include <algorithm>
#include "smartptr.h"


#ifdef _DEBUG
#define __STRING_DEBUG
#endif

#define NSTRING_ADD( pointer, length, value ) {   (pointer) += (value); (length) -= (value); }
#define NSTRING_INC( pointer, length        ) { --(length ); ++(pointer ); }

using namespace smart;

namespace dstring {

using namespace std  ;
  using namespace smart;

/** Предоставляет интерфейс к const char *, состоящий из следующих операций:
 *  0) Инициализация: 0, (const char *, <длина>), (const char * str), копирование
 *  1) Сдвиг левой границы влево и вправо на 1 и n позиций через ++, --, +=, -=, +, -
 *  2) Разадресация *
 *  3) Неявное приведение к const char *
 *  4) Акцесоры для длины
 *  5) Valid - ненулевой указатель
 *  6) Empty - строка закончилась
 *  7) Разница начальных позиций через -
 */
class NString {
protected:
  unsigned int size   ;
  const char * pointer;

  /// Для тегов из одного символа
public:
  inline NString( void                         ) : size( 0           ), pointer( 0           ) {}
  inline NString( const char    * str, int len ) : size( len         ), pointer( str         ) {}
  inline NString( const char    * str          ) : size( strlen(str) ), pointer( str         ) {}
  inline NString( const NString & str          ) : size( str.size    ), pointer( str.pointer ) {}

  /// Интерфейс присваивания. Результатом будет еще один объект, указывающий на исходную строку
  inline NString & operator=( const NString & str )
  {
      pointer = str.getPointer();
      size    = str.length()    ;
      return *this;
  }

  /// Сдвиг левой границы строки вправо на одну позицию с модификацией размера
  inline NString & operator++( void    ) { ++pointer; --size; return *this; }
  inline NString   operator++( int     ) { return operator++();             }
  /// Сдвиг левой границы строки влево на одну позицию с модификацией размера
  inline NString & operator--( void    ) { --pointer; ++size; return *this; }
  inline NString   operator--( int     ) { return operator--();             }
  /// Сдвиг левой границы строки вправо на @code num @endcode позиций с модификацией размера
  inline NString & operator+=( int num ) { pointer += num; size -= num; return *this; }
  /// Сдвиг левой границы строки влево на @code num @endcode позиций с модификацией размера
  inline NString & operator-=( int num ) { pointer -= num; size -= num; return *this; }

  inline char operator*        ( ) const { return *pointer; }
  inline operator const char * ( ) const { return  pointer; }

  inline const char * data() const { return pointer; }

  /// Проверка на ненулевой указатель
  inline bool valid( void ) const { return pointer != 0; }
  /// Контейнер пуст (true)
  inline bool empty( void ) const { return size    == 0; } // OLD: было size <= 0

  /// Внешний интерфейс ограничивает доступ к указателю на строку
  const char * getPointer( void ) const { return pointer; }

  inline void reset( const char   * ptr, int len ) { pointer = ptr        ; size = len         ; }
  inline void reset( const string & str          ) { pointer = str.c_str(); size = str.length(); }

  /// Акцесоры для длины
  inline bool contains() const { return size != 0; } // OLD: было size > 0

  inline int  length   ( void    ) const { return size; }
  inline void setLength( int len )       { size = len ; }
};


inline NString operator+( const NString & str, int num ) {
  NString result( str );
  return result += num;
}

inline NString operator-( const NString & str, int num ) {
  NString result( str );
  return result -= num;
}

/**
 * Разница между двумя строками.
 *
 * @return Если обе строки работают с одним и тем же объектом, то
 *         возвращаемое значение есть разница смещений относительно
 *         начала исходного объекта
 */
inline int operator-( const NString & s1, const NString & s2 ) {
  return s1.getPointer() - s2.getPointer();
}

/// Обертка для String с возможностью подсчета ссылок
class String : public string, public Smart {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  inline String() : string(), Smart() {}
  inline String( const char    * str              ) : string( str       )          {}
  inline String( const char    * str  , size_t sz ) : string( str  , sz )          {}
  inline String( size_t          count, char   ch ) : string( count, ch )          {}
  inline String( const string  & str              ) : string( str       )          {}
  inline String( const String  & str              ) : string( str       ), Smart() {}

  inline String( const NString & str              ) : string( str.getPointer(), str.length() ) { }

  const String & operator=( const String & str ) { string::operator=( str ); return *this; }

  inline void     toUpper() { transform(begin(), end(), begin(), ::toupper); }
  inline String & toupper() { transform(begin(), end(), begin(), ::toupper); return *this; }
};

// class Dstring : public string {
//   int strongRef;
// public:
//   inline Dstring() : string(), strongRef(0) {}
//   inline Dstring( const char    * str              ) : string( str       ), strongRef(0) {}
//   inline Dstring( const char    * str  , size_t sz ) : string( str  , sz ), strongRef(0) {}
//   inline Dstring( size_t          count, char   ch ) : string( count, ch ), strongRef(0) {}
//   inline Dstring( const string  & str              ) : string( str       ), strongRef(0) {}
//   inline Dstring( const String  & str              ) : string( str       ), strongRef(0) {}
//   inline Dstring( const NString & str              ) : string( str.getPointer(), str.length() ), strongRef(0) { }
//
//   const Dstring & operator=( const String & str ) { string::operator=( str ); return *this; }
//
//   inline void     toUpper() { transform(begin(), end(), begin(), ::toupper); }
//   inline Dstring & toupper() { transform(begin(), end(), begin(), ::toupper); return *this; }
//
//   inline void take() { ++strongRef; }
//   inline void release() { --strongRef; }
//   inline int owners() { return strongRef; }
// };

}

#endif
