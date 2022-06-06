/// @file
/// @brief Средства для работы с JSON
#pragma once
#ifndef JSON_UTILS_H
#define JSON_UTILS_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"

#include <jsoncpp/json/json.h>

#pragma GCC diagnostic pop


template < typename T, T ( Json::Value::*mf )() const, Json::ValueType enumVal >
struct TypedValue
{
     static T get( Json::Value& val, const std::string& key, const T& defaultVal )
     {
          Json::Value& itemVal = val[ key ];
          if ( itemVal.isNull() || itemVal.type() != enumVal )
          {
               return defaultVal;
          }

          return ( itemVal.*mf )();
     }

     static T get( Json::Value& val, const std::string& key )
     {
          Json::Value& itemVal = val[ key ];
          if ( itemVal.isNull() || itemVal.type() != enumVal )
          {
               return T();
          }

          return ( itemVal.*mf )();
     }

     static void get( std::pair< T, bool >& dest, Json::Value& val, const std::string& key )
     {
          Json::Value& itemVal = val[ key ];
          if ( !itemVal.isNull() && itemVal.type() == enumVal )
          {
               dest.first  = ( itemVal.*mf )();
               dest.second = true;
          }
     }
};


typedef TypedValue< unsigned int, &Json::Value::asUInt, Json::booleanValue > JsonValueBool;
typedef TypedValue< unsigned int, &Json::Value::asUInt, Json::uintValue >    JsonValueUInt;
typedef TypedValue< int, &Json::Value::asInt, Json::intValue >               JsonValueInt;
typedef TypedValue< std::string, &Json::Value::asString, Json::stringValue > JsonValueStr;


#endif // JSON_UTILS_H
