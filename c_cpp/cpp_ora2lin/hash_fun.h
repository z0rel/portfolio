#ifndef HASH_FUNCTIONS_H
#define HASH_FUNCTIONS_H

#include "project_optimization.h"

COMPILER_HASH_FUN_OPTIMIZATION_PUSH()

#include <string>
#include <string.h>
#include <iostream>

#define __BACKPORT_HASH_STRHIGS_BASE__ 5381

#if  defined(_MSC_VER) && ( _MSC_VER < 1400 )

#include "inter.h"

typedef L_ULONG  uint_fast32_t;
typedef L_UDLONG uint_fast64_t;
typedef L_DLONG  int_fast64_t ;
typedef L_BYTE   uint8_t      ;
typedef L_WORD   uint16_t     ;
typedef L_SWORD  int16_t      ;
typedef L_ULONG  uint32_t     ;

#else
#include <stdint.h>
#endif


typedef unsigned int  uint;

namespace BackportHashMap {

typedef uint_fast32_t uint32;
typedef uint_fast64_t uint64;
typedef int_fast64_t  int64 ;

typedef char       * char_ptr;
typedef const char * cchar_ptr;

namespace internal {

template < typename key_t, typename value_t >
struct HashActions {
  typedef uint ( *HashFunc           ) ( const key_t & key   );
  typedef bool ( *KeyEqualFunc       ) ( const key_t *  a,
                                         const key_t &  b    );
  typedef void ( *DestroyKeyNotify   ) ( key_t       *  key  );
  typedef void ( *DestroyValueNotify ) ( value_t     *  data );
};

namespace HashFunctions {
  inline uint str_const_char_ptr_hash ( cchar_ptr v              );
  inline uint sized_cchar_hash        ( cchar_ptr v, int size    );
  inline uint std_string_hash         ( const std::string & str  );

  template < typename key_t >
  inline bool key_equal( const key_t *  v1, const key_t & v2 ) { return *v1 == v2; }

  template <>
  inline bool key_equal<char_ptr>( const char_ptr *  v1, const char_ptr & v2 ) {
    return !strcmp (( const char * )*v1, ( const char * )v2);
  }

  template <>
  inline bool key_equal<cchar_ptr>( const cchar_ptr *  v1, const cchar_ptr & v2 ) {
    return !strcmp (*v1, v2);
  }


  /*
   * Converts a string to a hash value.
   *
   * This function implements the widely used "djb" hash apparently posted
   * by Daniel Bernstein to comp.lang.c some time ago.  The 32 bit
   * unsigned hash value starts at 5381 and for each byte 'c' in the
   * string, is updated: <literal>hash = hash * 33 + c</literal>.  This
   * function uses the signed value of each byte.
   */
  inline uint str_const_char_ptr_hash ( cchar_ptr v ) {
    register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p = (const signed char *)v;

      for (; *p != '\0'; ++p)
          h = (h << 5) + h + *p;

      return h;
  }

  inline uint cchar_hash ( cchar_ptr v, int size ) {
      register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p   = (const signed char *)v;
      register const signed char * end = p + size;

      for ( ; p != end; ++p)
          h = (h << 5) + h + *p;

      return h;
  }

  inline uint cchar_upper_hash ( cchar_ptr v, int size ) {
      register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p   = (const signed char *)v;
      register const signed char * end = p + size;

      for ( ; p != end; ++p)
          h = (h << 5) + h + (signed char)(toupper(*p));

      return h;
  }

  inline uint std_string_hash_upper ( const std::string & str ) {
      register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p = (const signed char *)str.data();
      register const signed char * end = p + str.size();

      for (; p != end; ++p)
        h = (h << 5) + h + (signed char)(toupper(*p));

      return h;
  }

  inline uint std_string_hash ( const std::string & str ) {
      register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p = (const signed char *)str.data();
      register const signed char * end = p + str.size();

      for (; p != end; ++p)
        h = (h << 5) + h + *p;

      return h;
  }

  inline uint std_string_upper_hash ( const std::string & str ) {
      register uint32 h = __BACKPORT_HASH_STRHIGS_BASE__;
      register const signed char * p = (const signed char *)str.data();
      register const signed char * end = p + str.size();

      for (; p != end; ++p)
        h = (h << 5) + h + (signed char)(toupper(*p));

      return h;
  }


  constexpr inline uint hash_fun_emptystr() { return __BACKPORT_HASH_STRHIGS_BASE__; }

  template < typename key_t >
  inline uint hash_fun                 ( const key_t          &v ) { return ((uint) *(uint64*) (&v));   }
  template <>
  inline uint hash_fun<std::string>    ( const std::string    &v ) { return std_string_hash(v);         }
  template <>
  inline uint hash_fun<char_ptr>       ( const char_ptr       &v ) { return str_const_char_ptr_hash(v); }
  template <>
  inline uint hash_fun<cchar_ptr>      ( const cchar_ptr      &v ) { return str_const_char_ptr_hash(v); }
  template <>
  inline uint hash_fun<int>            ( const int            &v ) { return *(uint*)&v;                 }
  template <>
  inline uint hash_fun<unsigned int>   ( const unsigned int   &v ) { return v;                          }
  template <>
  inline uint hash_fun<char>           ( const char           &v ) { return (uint)*(unsigned char*)&v;  }
  template <>
  inline uint hash_fun<signed char>    ( const signed char    &v ) { return (uint)*(unsigned char*)&v;  }
  template <>
  inline uint hash_fun<unsigned char>  ( const unsigned char  &v ) { return (uint)v;                    }
  template <>
  inline uint hash_fun<signed short>   ( const signed short   &v ) { return (uint)*(unsigned short*)&v; }
  template <>
  inline uint hash_fun<unsigned short> ( const unsigned short &v ) { return (uint)v;                    }
  template <>
  inline uint hash_fun<int64 >         ( const int64          &v ) { return *(uint*)&v;                 }
  template <>
  inline uint hash_fun<double>         ( const double         &v ) { return *(uint*)&v;                 }
}

}

struct sized_cchar_base {
  sized_cchar_base( const sized_cchar_base & o )
    : hashValue(o.hashValue),
      str(o.str),
      size(o.size) {}

  sized_cchar_base( uint _hashValue, const char * _str, const int _size  )
    : hashValue(_hashValue),
      str (_str ),
      size(_size) {}

  // Хэш будет рассчитан позже
  sized_cchar_base( const char * _str, const int _size  )
    : str (_str ),
      size(_size) {}

  inline sized_cchar_base & operator= ( const sized_cchar_base & o ) { str = o.str; return *this; }

  inline bool operator==( const sized_cchar_base & other ) const { return size == other.size && !memcmp(str, other.str, size); }

  inline bool operator==( const std::string & other ) const {
    return size == (int)other.size() && !memcmp(str, other.data(), size );
  }
  inline bool operator==( const char * other ) const {
    return !strncmp(str, other, size + 1); // последний ведущий ноль для const char * будет учтен.
  }

  inline operator std::string() const { return std::string( str, size ); }
  inline std::string toString() const { return std::string( str, size ); }

  uint         hashValue;
  mutable const char * str ;
  mutable int          size;

  virtual ~sized_cchar_base() {}
};

/**
 * @brief Статическая строка для которой известны хеш и длина
 * используестя в качестве статической констанстной строки
 * (для нее в рантайме известен хэш)
 *
 */
struct static_hashed_cchar : sized_cchar_base {
  static_hashed_cchar() : sized_cchar_base( 0, 0, 0 ) {}

  static_hashed_cchar( const char * str, size_t size )
    : sized_cchar_base(
        internal::HashFunctions::cchar_hash(str, size),
        str,
        size) {}

  static_hashed_cchar( const std::string & _str )
    : sized_cchar_base( _str.data(), _str.size() )
  {
    hashValue = internal::HashFunctions::cchar_hash( str, size );
  }

  template < typename CCHAR >
  static_hashed_cchar( const CCHAR & s )
    : sized_cchar_base(
        internal::HashFunctions::cchar_hash( (const char*)s, sizeof(CCHAR) - 1 ),
        (const char*)s,
        sizeof(CCHAR) - 1) {}
};


class dynamic_hashed_cchar : public sized_cchar_base {
protected:
  const bool temporary;

  dynamic_hashed_cchar( const char * str, size_t size, int, int )
    : sized_cchar_base( str, size ), temporary(true)
  {
    hashValue = internal::HashFunctions::cchar_hash( str, size );
  }

  dynamic_hashed_cchar( const char * str, size_t size, int )
    : sized_cchar_base( str, size ), temporary(false)
  {
    hashValue = internal::HashFunctions::cchar_hash( str, size );
  }
public:
  dynamic_hashed_cchar() :
    sized_cchar_base( 0, 0, 0 ), temporary(false) {}
  /// Создание динамической строки из класса string
  dynamic_hashed_cchar( const std::string & s )
    : sized_cchar_base(
        (const char *)memcpy( new char[s.size()] , s.data(), s.size() ),
        s.size()
      ), temporary(false)
  {
    hashValue = internal::HashFunctions::cchar_hash( str, size );
  }

  dynamic_hashed_cchar( const char * str, size_t size )
    : sized_cchar_base(
        (const char *)memcpy( new char[size] , str, size ),
        size
      ), temporary(false)
  {
    hashValue = internal::HashFunctions::cchar_hash( str, size );
  }

  /// Конструктор копирования с семантикой владения
  dynamic_hashed_cchar( const dynamic_hashed_cchar & o )
    : sized_cchar_base( o ), temporary(o.temporary) { o.str  = 0; o.size = 0; }

  ~dynamic_hashed_cchar() {
    if (temporary)
      return;
    if( str ) {
      delete[] (char*)str;
      str = 0;
    }
  }
};

// Временный объект для поиска в хэше с выделенной памятью
class temporary_hashed_cchar : public dynamic_hashed_cchar {
public:
  temporary_hashed_cchar() :
    dynamic_hashed_cchar( 0, 0, 1, 1 ) {}
  /// Создание динамической строки из класса string
  temporary_hashed_cchar(const std::string &s)
    : dynamic_hashed_cchar(s.c_str(), s.size(), 1, 1) {}

  temporary_hashed_cchar( const char * str, size_t size )
    : dynamic_hashed_cchar( str, size, 1, 1 ) {}

  /// Конструктор копирования с семантикой владения
  temporary_hashed_cchar( const dynamic_hashed_cchar & o )
    : dynamic_hashed_cchar( o.str, o.size, 1, 1 ) {}

  ~temporary_hashed_cchar() {}
};

typedef static_hashed_cchar    SHCChar;
typedef dynamic_hashed_cchar   DHCChar;
typedef temporary_hashed_cchar THCChar;


inline bool operator==(const std::string& lhs, const sized_cchar_base& rhs) {
  return rhs == lhs;
}
inline void operator+=(      std::string& lhs, const sized_cchar_base& rhs) {
  lhs.append(rhs.str, rhs.size);
}

inline std::ostream& operator<<(std::ostream& os, const sized_cchar_base& obj) {
  return os.write(obj.str, obj.size - 1);
}

namespace internal {
namespace HashFunctions {
  template <>
  inline uint hash_fun< static_hashed_cchar > ( const static_hashed_cchar  & v ) { return v.hashValue; }
  template <>
  inline uint hash_fun< dynamic_hashed_cchar> ( const dynamic_hashed_cchar & v ) { return v.hashValue; }
}
}

}

COMPILER_HASH_FUN_OPTIMIZATION_POP()

#endif // HASH_FUNCTIONS_H
// vim:foldmethod=syntax
