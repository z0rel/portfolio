#include <stdio.h>
#include "crossplatform_func.h"
#include "syntaxer_context.h"


extern SyntaxerContext syntaxerContext;

using namespace std;
using namespace dmperr;

ErrorInfo::ErrorInfo(
    const std::string & _parsedString,
    const std::string & _errorBegins ,
    errors _code
    )
  : parsedString( _parsedString, 0, _parsedString.length() - _errorBegins.length() ),
    errorBegins ( _errorBegins  ),
    code        ( _code         )
{}

void ErrorInfo::toString(std::string & str) const {
  toString(str, parsedString );
}

void ErrorInfo::toString(std::string & str, const std::string & parsedStr ) const {
  str += "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  str += "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  str += "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
  str += "Exception: ";
  str += dmperr::toString( code );
  str += "\n";
  str += "Analyzed String: \n        ";
  str += parsedStr;
  str += "\nOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO\n";
  str += "Error In String: \n        ";
  str += errorBegins.substr ( 0, errorBegins.size()  > 200 ? 200 : errorBegins.size() );
  str += "\n";
}


struct ExceptCounter {
  ExceptCounter() : count(0) {}
  void operator++() { ++count; }
  int count;
};

void dmperr::handleError(const string    & str      ,
                         size_t            streamlen,
                         const ErrorInfo & err      ,
                         const string    & pkgName  )
{
  static int count = 0;
  static HashMap<int, ExceptCounter> exceptCount;

  string errstr;

  // Длина разобранной подстроки от начала конструкции до исключения
  size_t parsedLen   = str.size() - streamlen;
  size_t beginOffset = parsedLen > 400 ? (parsedLen - 400) : 0;

  string parsedStr(str, beginOffset, string::npos);

  errstr.assign(0, '\0');
  err.toString( errstr, parsedStr );

  cout                                 << endl
                                       << endl
                                       << endl
       << "Package name = " << pkgName << endl
                                       << endl
       << errstr                       << endl;

  ++exceptCount[(int)err.code];

  if ( exceptCount.size() )
    printf("\n COUNTS:\n");


  for ( HashMap<int, ExceptCounter>::const_iterator errIt = exceptCount.cbegin(); errIt; ++errIt )
    printf( "%s == %i\n", toString( (errors)*errIt.key ), errIt.value->count );

  count++;
  printf("\nTOTAL ERRORS: %i\n", count);
}

#define exceptDescr( exceptName ) hash.insert( exceptName,  #exceptName )

static void exceptCodeInit( HashMap<int, const char *> & hash, void * ) {
  exceptDescr(ADD_CHECK                              );
  exceptDescr(ADD_CONSTRAINT                         );
  exceptDescr(ADD_FOREIGN_KEY                        );
  exceptDescr(ADD_PRIMARY_KEY                        );
  exceptDescr(ADD_UNIQUE                             );
  exceptDescr(ALTER_TABLE                            );
  exceptDescr(ARGUMENT_NOT_HAVE_CURSOR_TYPE          );
  exceptDescr(BAD_CAST                               );
  exceptDescr(BLOB_BEFORE_DATA_ROW                   );
  exceptDescr(BRACKET_ITEM_EMPTY                     );
  exceptDescr(CASE_NOT_END                           );
  exceptDescr(CASE_WHEN_NOT_THEN                     );
  exceptDescr(CAST_NOT_AS                            );
  exceptDescr(CAST_NOT_BRACKET                       );
  exceptDescr(CLOSE_NO_SEMICOLON                     );
  exceptDescr(COMMENT_ON_COLUMN                      );
  exceptDescr(COMMENT_ON_TABLE                       );
  exceptDescr(CREATE_INDEX                           );
  exceptDescr(CREATE_SEQUENCE                        );
  exceptDescr(CREATE_SYNONYM                         );
  exceptDescr(CREATE_TABLE                           );
  exceptDescr(CREATE_TABLE_INFINITE_LOOP             );
  exceptDescr(CREATE_TYPE_HAS_NOT_NULL_MARKER        );
  exceptDescr(CREATE_TYPE_UNEXPECTED_SYNTAX          );
  exceptDescr(CREATE_UNIQUE_INDEX                    );
  exceptDescr(CREATE_VIEW                            );
  exceptDescr(CURSOR_BAD_SYNTAX                      );
  exceptDescr(CURSOR_NOT_FOUND                       );
  exceptDescr(CURSOR_NOT_HAVE_REQUEST                );
  exceptDescr(CURSOR_NO_SEMICOLON                    );
  exceptDescr(CURSOR_VARIABLE_BAD_SYNTAX             );
  exceptDescr(DECLARE_NO_SEMICOLON                   );
  exceptDescr(DELETE_UNCOMPLETED_CONSTRUCTION        );
  exceptDescr(DELETE_UNEXPECTED_TAG                  );
  exceptDescr(DMP_STRUCT_ERROR                       );
  exceptDescr(EXCEPTION_VALUE_EMPTY                  );
  exceptDescr(EXECUTE_UNEXPECTED_TAG                 );
  exceptDescr(EXPR_IN_NO_CLOSE_BRACKET               );
  exceptDescr(EXTRACT_NO_CLOSE_BRACKET               );
  exceptDescr(EXTRACT_NO_FROM                        );
  exceptDescr(EXTRACT_NO_OPEN_BRACKET                );
  exceptDescr(EXTRACT_UNKNOWN_PART_NAME              );
  exceptDescr(FETCH_NOT_HAVE_INTO                    );
  exceptDescr(FETCH_NO_SEMICOLON                     );
  exceptDescr(FOR_BAD_OPERATION_SYNTAX               );
  exceptDescr(FOR_UNCOMPLETED_CONSTRUCTION           );
  exceptDescr(FREAD_NONINTEGRAL                      );
  exceptDescr(FREAD_OVERFLOW                         );
  exceptDescr(FREAD_RDERROR                          );
  exceptDescr(FSEEK_ERROR                            );
  exceptDescr(FUNCTION_NO_CLOSE_BRACKET              );
  exceptDescr(FUNCTION_UNCOMPLETED_CONSTRUCRION      );
  exceptDescr(HEADER_NO_SEMICOLON                    );
  exceptDescr(IF_BAD_OPERATION_SYNTAX                );
  exceptDescr(IF_UNCOMPLETED_CONSTRUCTION            );
  exceptDescr(IF_UNEXPECTED_TAG                      );
  exceptDescr(INCORRECT_NUMBER_OF_BLOB_RECORDS       );
  exceptDescr(INSERT_INTO                            );
  exceptDescr(INSERT_NO_CLOSE_BRACKET                );
  exceptDescr(INSERT_NO_OPEN_BRACKET                 );
  exceptDescr(INSERT_UNCOMPLETED_CONSTRUCTION        );
  exceptDescr(INSERT_UNEXPECTED_TAG                  );
  exceptDescr(LABEL_HAS_NOT_CLOSE_BRACKETS           );
  exceptDescr(LOCK_TABLE_HAS_NOT_IN                  );
  exceptDescr(LOCK_TABLE_HAS_NOT_MODE                );
  exceptDescr(LOCK_TABLE_UNKNOWN_WAIT                );
  exceptDescr(LOOP_BAD_OPERATION_SYNTAX              );
  exceptDescr(LOOP_UNCOMPLETED_CONSTRUCTION          );
  exceptDescr(MERGE_HAS_NOT_INTO                     );
  exceptDescr(MERGE_HAS_NOT_ON                       );
  exceptDescr(MERGE_HAS_NOT_USING                    );
  exceptDescr(MERGE_INTO_HAS_NOT_FIELD_LIST          );
  exceptDescr(MERGE_INTO_HAS_NOT_VALUES              );
  exceptDescr(MODIFY_DEFAULT                         );
  exceptDescr(MODIFY_UNCOMPLETED_CONSTRUCRION        );
  exceptDescr(NAME_ALREDY_EXIST_WITH_OTHER_TYPE      );
  exceptDescr(NOT_TYPE_CONVERTER_FOR_ROW_VALUE       );
  exceptDescr(OK                                     );
  exceptDescr(OPENED_CURSOR_CANNOT_HAVE_FOR          );
  exceptDescr(OPEN_CURSOR_VARIABLE_NOT_HAVE_FOR      );
  exceptDescr(OPERAND_NAME_EMPTY                     );
  exceptDescr(OPERATION_BAD_OPERATION_SYNTAX         );
  exceptDescr(OPERATION_UNCOMPLETED_CONSTRUCTION     );
  exceptDescr(OPERATION_UNEXPECTED_TAG               );
  exceptDescr(OPERATOR_NO_SEMICOLON                  );
  exceptDescr(OPERNAD_NO_CLOSE_BRACKET               );
  exceptDescr(OVER_HAS_NOT_CLOSE_BRACKET             );
  exceptDescr(OVER_HAS_NOT_OPEN_BRACKET              );
  exceptDescr(PACKAGE_NO_SEMICOLON                   );
  exceptDescr(PACKAGE_UNCOMPLETED_CONSTRUCTION       );
  exceptDescr(PACKAGE_UNCOMPLETE_TYPE                );
  exceptDescr(PARAMS_LIST_BAD_OPERATION_SYNTAX       );
  exceptDescr(PARAMS_LIST_NO_CLOSE_BRACKET           );
  exceptDescr(PARAMS_LIST_UNCOMPLETED_CONSTRUCTION   );
  exceptDescr(PRAGMA_NO_CLOSE_BRACKET                );
  exceptDescr(PRAGMA_NO_COMMA                        );
  exceptDescr(PRAGMA_NO_OPEN_BRACKET                 );
  exceptDescr(PRAGMA_NO_SEMICOLON                    );
  exceptDescr(RECORD_NO_SEMICOLON                    );

  exceptDescr(REFTYPE_NO_CLOSE_BRACKET               );
  exceptDescr(REFTYPE_PRECISION_EMPTY                );
  exceptDescr(REFTYPE_PRECISION_UNEXPECTED           );
  exceptDescr(REFTYPE_UNEXPECTED_TAG                 );
  exceptDescr(REFTYPE_UNEXPECTED_TYPE                );
  exceptDescr(REFTYPE_WIDTH_EMPTY                    );
  exceptDescr(REFTYPE_WIDTH_UNEXPECTED               );

  exceptDescr(RETURNING_CLAUSE_NOT_INTO              );
  exceptDescr(SCOPE_ADD_FUNCTION_ERROR               );
  exceptDescr(SELECT_UNCOMPLETED_CONSTRUCTION        );
  exceptDescr(SELECT_UNEXPECTED_CHARACTER            );
  exceptDescr(SELECT_UNEXPECTED_TAG                  );
  exceptDescr(SELECT_WITH_HAS_NOT_AS                 );
  exceptDescr(SVT_BAD_OPERATION_SYNTAX               );
  exceptDescr(SV_END_OF_DATASET_MUST_BE_FIRST_IN_ROW );
  exceptDescr(SV_UNALIGNED_BLOBLEN                   );
  exceptDescr(TRIM_HAS_NOT_FROM                      );
  exceptDescr(TRIM_NO_CLOSE_BRACKET                  );
  exceptDescr(TRIM_NO_OPEN_BRACKET                   );
  exceptDescr(UNKNOWN_DMP_CODE_FOR_TYPE              );
  exceptDescr(UNKNOWN_DMP_MAGIC_CODE                 );
  exceptDescr(UNKNOWN_TEXT_AFTER_TYPE_SQL_DECLARATION);
  exceptDescr(UNKNOWN_TYPE_SQL_DECLARATION           );
  exceptDescr(UPDATE_NOT_SEMICOLON                   );
  exceptDescr(UPDATE_NOT_SET                         );
}


const char* dmperr::toString(errors err) {
  static const HashMap<int, const char *> codes_hash( exceptCodeInit );
  const char** code = codes_hash.find((int)err);
  return code ? *code : "";
}


#include <fstream>
#include <iostream>

static inline void getStringData( fstream &f, int & key, char * buf,
                                  fstream::pos_type & start, fstream::pos_type & end) {
  static char delim;
  f >> delim >> key >> delim;
  start = f.tellg();
  f.getline(buf, 0xFFFF);
  end   = f.tellg();
}


#if LINUX_DEFINED

#include <iconv.h>
//#define iconvInit() iconv_t converter = iconv_open ( "UTF-8", "CP866" )
#define iconvInit() iconv_t converter = iconv_open ( "CP1251", "CP866" )
#define iconvDestruct() iconv_close (converter)
#define convertString(p2, p3, p4, p5) iconv(converter, p2, p3, p4, p5)


#elif defined(_WIN64) || defined(_MSC_VER) || _WIN64 || defined(_WIN32) || defined(__WIN32__) || defined(WIN32)

static const char baseSymbolsTbl[256] = {
     0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,
    16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,
    32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,
    48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,
    64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,
    80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,   95,
    96,   97,   98,   99,  100,  101,  102,  103,  104,  105,  106,  107,  108,  109,  110,  111,
   112,  113,  114,  115,  116,  117,  118,  119,  120,  121,  122,  123,  124,  125,  126,  127,
   -60,  -77,  -38,  -65,  -64,  -39,  -61,  -76,  -62,  -63,  -59,  -33,  -36,  -37,  -35,  -34,
   -80,  -79,  -78, -125,   -2,   -7,   -5,  126,    0,    0,   -1,   74,   -8,    0,   -6,   47,
   -51,  -70,  -43,  -15,  -42,  -55,  -72,  -73,  -69,  -44,  -45,  -56,  -66,  -67,  -68,  -58,
   -57,  -52,  -75,  -16,  -74,  -71,  -47,  -46,  -53,  -49,  -48,  -54,  -40,  -41,  -50,    0,
   -18,  -96,  -95,  -26,  -92,  -91,  -28,  -93,  -27,  -88,  -87,  -86,  -85,  -84,  -83,  -82,
   -81,  -17,  -32,  -31,  -30,  -29,  -90,  -94,  -20,  -21,  -89,  -24,  -19,  -23,  -25,  -22,
   -98, -128, -127, -106, -124, -123, -108, -125, -107, -120, -119, -118, -117, -116, -115, -114,
  -113,  -97, -112, -111, -110, -109, -122, -126, -100, -101, -121, -104,  -99, -103, -105, -102,
};

// Для BIG ENDIAN будет другая реализация - нужно будет байты поменять местами
  inline void convertChar( char c, char **ptr ) {
    if ( ! (*(*ptr)++ = baseSymbolsTbl[*((unsigned char*)&c)]) && c ) {
      if ( c < -100 ) {
        if ( c == -103 ) // 0x99 >=
          *((uint16_t *)((*ptr)++ - 1 )) = 0x3D3E;
        else // 0x98 <=
          *(int16_t *)((*ptr)++ - 1 ) = 0x3D3C;
      }
      else if ( c == -99 ) // 0x9D ^2
        *((int16_t *)((*ptr)++ - 1 )) = 0x325E;
      else // 0xBF @c
        *((int16_t *)((*ptr)++ - 1 )) = 0x6340;
    }
  }

  // Буфер должен быть > 1
  inline void convertString( char ** inBuf, size_t * inSize, char ** outBuf, size_t * outSize  ) {
    const char * endPtr      = *inBuf + *inSize;
    const char * endOutBuf   = *outBuf + *outSize - 1;
    const char * startOutBuf = *outBuf;

    char * ptr;
    for ( ptr = *inBuf; ptr != endPtr && *outBuf < endOutBuf; ++ptr )
      convertChar( *ptr, outBuf );

    *inSize -= (ptr - *inBuf);
    *inBuf = ptr;
    *outSize -= (*outBuf - startOutBuf);
  }

#define iconv( p1, p2, p3, p4, p5 ) convertString( p2, p3, p4, p5 )
#define iconvInit() { ; }
#define iconvDestruct() ;

#endif

/*
 *
 *
 *
 **/


namespace errors {
void stub1() {}
void stub2() {}
}

namespace dmperr {


void errorseInit(HashMap<int, ErrorCodes> &hash, const string &fname) {
  char buf[0xFFFF];
  int  key;

  fstream::pos_type start = 0;
  fstream::pos_type end = 0;

  fstream f(fname.c_str(), fstream::in);

  for ( getStringData(f, key, buf, start, end); !f.eof() && f.good();
        getStringData(f, key, buf, start, end) )
    hash[key].engCode.assign( buf, (unsigned int)(end - start - 2) );

  f.close();
}

void errorCodeInit(HashMap<int, ErrorCodes> &hash, const string &fname) {
  char buf[0xFFFF];
  int  key;

  fstream::pos_type start = 0;
  fstream::pos_type end = 0;

  iconvInit();

//  f.open(syntaxerContext.errorsFile.c_str(), fstream::in);
  fstream f(fname.c_str(), fstream::in);
  for ( getStringData(f, key, buf, start, end); !f.eof() && f.good();
        getStringData(f, key, buf, start, end) ) {
    string str( buf, (unsigned int)(end - start - 2) );
    size_t inBufSize  = str.size();
    char * inBufPtr   = (char *)str.c_str();
    size_t outBufSize = sizeof(buf);
    char * outBufPtr  = (char*)buf;
    convertString(&inBufPtr, &inBufSize, &outBufPtr, &outBufSize);
    hash[key].ruCode.assign(buf, sizeof(buf) - outBufSize);
  }
  f.close();

  iconvDestruct();
}

}

HashMap<int, ErrorCodes> dmperr::errorCodeConverter;
