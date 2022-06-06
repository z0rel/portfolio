#ifndef LEXER_H
#define LEXER_H

#include "project_optimization.h"
COMPILER_LEXER_OPTIMIZATION_PUSH()

#include <stdio.h>
#include <string>
#include <vector>
#include <ostream>
#include "lex_location.h"
#include "inter.h"
#include "syntaxer_external.h"

// macro yylex, declarations: yytype_uint16, yyscan_t {{{1


#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
#define YYTYPE_UINT16 L_WORD

typedef L_WORD yytype_uint16;
#endif

/**
 * Объявление типа сканера.
 * Запрещает использование объявления, в хедере flex
 **/
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#ifndef YYLTYPE
# define YYLTYPE cl::location
#endif

// 1}}}

typedef int  (*yyinput_t       )(yyscan_t yyscanner);
typedef void (*lex_pop_state_t )(yyscan_t yyscanner);
typedef int  (*lex_top_state_t )(yyscan_t yyscanner);
typedef void (*lex_push_state_t)(int new_state, yyscan_t yyscanner);

namespace Sm {
  class NumericValue;
}

void lex_unput(int c, yyscan_t yyscanner);
void llocBuf  (yyscan_t yyscanner);
void getNumber(Sm::NumericValue *, char *text, int length);



COMPILER_LEXER_OPTIMIZATION_POP()

#endif // LEXER_H
// vim:foldmethod=marker
