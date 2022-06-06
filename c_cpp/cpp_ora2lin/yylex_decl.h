#ifndef YYLEX_DECL
#define YYLEX_DECL

#include "sql_syntaxer_bison.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

extern yy::parser::token::yytokentype ora2linLex(
    cl::semantic_type *yylval_param,
    cl::location      *yylloc_param,
    yyscan_t           yyscanner);

#define YY_DECL yy::parser::token::yytokentype ora2linLex( \
  cl::semantic_type * yylval_param, \
  cl::location      * yylloc_param, \
  yyscan_t yyscanner)


#endif // YYLEX_DECL

