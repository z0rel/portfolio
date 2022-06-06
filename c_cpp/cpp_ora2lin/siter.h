#ifndef __PLHELPER_H
#define __PLHELPER_H

#include "project_optimization.h"
COMPILER_SITER_OPTIMIZATION_PUSH()

#include <sstream>
#include "dstring.h"
// #include "crossplatform_func.h"
#include "inter.h"
#include "crossplatform_func.h"
#include "smartptr.h"
#include "hash_table.h"

#ifndef SIZEOF_TABLE
#define SIZEOF_TABLE( X ) ( sizeof( X ) / sizeof( X[ 0 ] ) )
//#define STRING( X )         X, ( sizeof(X) - 1 )

// Более важная штука в силу совместимости с табличной инициализацией SIterator и NString
#define STRING_S( X )         ( sizeof(X) - 1 ) , X

#endif
#define CHAR_TO_INT( ch )         (int)( *((unsigned char *)(&(ch))) )
#define CHARPTR_TO_INT( pointer ) (int)(*((unsigned char *)(pointer)))

namespace PlsqlHelper {

using namespace dstring;

inline void replaceCr( string & str, const char * to ) {
  size_t pos;
  while( ( pos = str.find( '\n' ) ) != string::npos )
    str.replace( pos, /*pos + */ 1, to );
}

inline void escaping( string & result, const string & str, char quote, char predicate );
inline void escapingOnlyQuote( string & result, const string & str, char quote, char predicate );

/**
 * @brief Sql::quotingAndEscaping
 *  Экранировать символами @c predicate все кавычки @c quote в строке @c str и поместить саму строку в
 *  кавычки @c quote .
 * @param  result    Выходная строка
 * @param  str       Входная строка
 * @param  quote     Экранируемый символ
 * @param  predicate Экранирующий символ
 * @tparam T         Тип строкового класса. Нужен для использования ф-ции в getActionList
 */
template < typename T >
inline void quotingAndEscapingSpec( string &result, const T & str, char quote, char predicate ) {
  result += quote;
  PlsqlHelper::escaping( result, str, quote, predicate );
  result += quote;
}
template < typename T >
inline string quotingAndEscapingSpec(const T & str, char quote, char predicate ) {
  string result;
  quotingAndEscapingSpec(result, str, quote, predicate);
  return result;
}
template < typename T >
inline void quotingAndEscapingSpecOnlyQuote( string & result, const T & str, char quote, char predicate ) {
  result += quote;
  PlsqlHelper::escapingOnlyQuote( result, str, quote, predicate );
  result += quote;
}

template < typename T >
inline string quotingAndEscapingDQuote(const T &str) {
  string result;
  quotingAndEscapingSpec( result, str, '\"', '\"' );
  return result;
}

template < typename T >
inline string quotingAndEscapingPython(const T &str) {
  string result;
  quotingAndEscapingSpec(result, str, '"', '\\');
  return result;
}

template < typename T >
inline string quotingAndEscapingPython1(const T &str) {
  string result;
  quotingAndEscapingSpec(result, str, '\'', '\\');
  return result;
}

template < typename T >
inline void quotingAndEscaping( string & result, const T & str) {
  quotingAndEscapingSpec( result, str, '\"', '\"' );
}
template < typename T >
inline void quotingAndEscaping( string & result, const T & str, char quote ) {
  quotingAndEscapingSpec( result, str, quote, quote );
}

template < typename T >
inline void quotingAndEscapingOnlyQuote( string & result, const T & str) {
  quotingAndEscapingSpecOnlyQuote( result, str, '\"', '\"' );
}
template < typename T >
inline void quotingAndEscapingOnlyQuote( string & result, const T & str, char quote ) {
  quotingAndEscapingSpecOnlyQuote( result, str, quote, quote );
}

inline void escapeSingleChar(string &result, const char *&startData, const char *&data, char quote, char predicate) {
  result.append(startData, /* количество символов */ data - startData);
  result.push_back(predicate);
  result.push_back(quote    );
  startData = ++data;
}

inline void escaping(string &result, const string &str, char quote, char predicate) {
  // -> '\'
  // -> quote
  const char *data      = str.data();
  const char *startData = data;
  const char *endData = data+str.size();
  while (data != endData) {
    if (*data == quote) {
      escapeSingleChar(result, startData, data, quote, predicate);
      continue;
    }
    else if (*data == '\\') {
      escapeSingleChar(result, startData, data, '\\', '\\');
      continue;
    }
    ++data;
  }
  if (startData != endData)
    result.append(startData, /* количество символов */ data - startData);
}

inline void escapingOnlyQuote(string &result, const string &str, char quote, char predicate) {
  // -> '\'
  // -> quote
  const char *data      = str.data();
  const char *startData = data;
  const char *endData = data+str.size();
  while (data != endData) {
    if (*data == quote) {
      escapeSingleChar(result, startData, data, quote, predicate);
      continue;
    }
    ++data;
  }
  if (startData != endData)
    result.append(startData, /* количество символов */ data - startData);
}


}

extern int createForInl;

/// Функция для модификации конца строки при генерации для INL
inline void semiCr( std::string & result, bool forInl ) { result += (forInl ? "; //\n" : ";\n"); }
/// Функция для модификации конца строки при генерации для INL
inline std::string semiCr(bool forInl)                  { return forInl ? "; //\n" : ";\n"; }

namespace Sm {

class String : public Smart, public std::string {
protected:
//  void* getThisPtr() const { return (void*)this; }
public:
  inline String()                           : std::string()                {}
  inline String(const char *str)            : std::string(str)             {}
  inline String(const char *str, size_t sz) : std::string(str, sz)         {}
  inline String(size_t count, char c)       : std::string(count, c)        {}
  inline String(const std::string &str)     : std::string(str)             {}
  inline String(const String &str)          : smart::Smart(), std::string(str) {}

  inline const String &operator=(const String &str) { std::string::operator=(str); return *this; }
};

class HString : public std::string {
protected:
  uint hashValue_;
public:
  inline HString()                           : std::string()        , hashValue_(BackportHashMap::internal::HashFunctions::hash_fun(std::string())) {}
  inline HString(const char *str)            : std::string(str)     , hashValue_(BackportHashMap::internal::HashFunctions::hash_fun(str)) {}
  inline HString(const char *str, size_t sz) : std::string(str, sz) , hashValue_(BackportHashMap::internal::HashFunctions::cchar_hash(str, sz)) {}
  inline HString(size_t count, char c)       : std::string(count, c), hashValue_(BackportHashMap::internal::HashFunctions::std_string_hash(*this)) {}
  inline HString(const std::string &str)     : std::string(str)     , hashValue_(BackportHashMap::internal::HashFunctions::std_string_hash(str)) {}
  inline HString(const String &str)          : std::string(str)     , hashValue_(BackportHashMap::internal::HashFunctions::std_string_hash(str)) {}
  inline HString(const HString &str)         : std::string(str)     , hashValue_(str.hashValue_) {}

  inline const HString &operator=(const std::string &str) {
    std::string::operator=(str);
    hashValue_ = BackportHashMap::internal::HashFunctions::std_string_hash(str);
    return *this;
  }
  inline const HString &operator=(const String &str) {
    std::string::operator=(str);
    hashValue_ = BackportHashMap::internal::HashFunctions::std_string_hash(str);
    return *this;
  }
  inline const HString &operator=(const HString &str) {
    std::string::operator=(str);
    hashValue_ = str.hashValue_;
    return *this;
  }

  inline bool operator==(const HString &str) const {
    if (hashValue_ == str.hashValue_)
      return static_cast<const std::string&>(*this) == static_cast<const std::string&>(str);
    else
      return false;
  }
  inline bool operator!=(const HString &str) const {
    if (hashValue_ != str.hashValue_)
      return true;
    else
      return static_cast<const std::string&>(*this) != static_cast<const std::string&>(str);
  }

  inline uint hash() const { return hashValue_; }
};

class Id;
template <typename Element> class List;
template <> class List<Id>;

std::string join(std::string sep, Sm::List<Sm::Id> *keyFields);
std::string join(std::string sep, const std::vector<std::string> &keyFields);


template <typename T>
std::string pyJoin(const std::string &delimeter, T container) {
  auto it = container.begin();
  if (it == container.end())
    return "";
  std::stringstream str;
  str << *it;
  for (++it; it != container.end(); ++it)
    str << delimeter << *it;
  return str.str();
}

}

COMPILER_SITER_OPTIMIZATION_POP()

#endif

