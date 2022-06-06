%x Glang

%option bison-bridge
%option bison-locations
%option noyywrap
%option reentrant
%option stack
%option debug

%{
/* Expert system based on the LP-structures.
 *
 * Copyright (C) 2012, Artem Shmarin, S. Makhortov.
 * Written by Artem Shmarin (tim-shr@mail.ru)
 *
 * This file is part of LPExpert 2.0.
 *
 * LPExpert 2.0 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * LPExpert 2.0 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LPExpert 2.0.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <math.h>
#include <string.h>

#include "src/compiler/token_table.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"

#define yyterminate() return static_cast<yy::parser::token::yytokentype>(YY_NULL)

template <int endc>
inline void lex_get_quoted(std::string& lexer_id_text, yyscan_t yyscanner);

%}


  /* NUM_TOK     [-+]?[0-9]+(\.?[0-9]+([eE][-+]?[0-9]+)?)? */
NUM_TOK     [-+]?[0-9]+(\.[0-9]+)?


%%


%{

yy_flex_debug = 0;

%}

<INITIAL>\^[^%\n]+ {
        yylloc_param->step(yyleng);
        flpx::String *str = new flpx::String();
        *str = yytext;
        yylval->comment = str;
        return yy::parser::token::COMMENT;
    }
<INITIAL>% {
        yylloc_param->step(yyleng);
        // BEGIN Glang;
        return yy::parser::token::PERCENT;
    }

<INITIAL>[(][^\n\r)]*[)] {
        yylloc_param->step(yyleng);
        flpx::String *str = new flpx::String();
        *str = yytext + 1;
        str->pop_back();
        yylval->comment = str;
        return yy::parser::token::CADR_COMMENT;
    }

<INITIAL>[MNLGRTHXYFIJES] {
        yylloc_param->step(yyleng);
        unsigned char *c = (unsigned char *)yytext;
        unsigned char idx_c = c[0];
        unsigned int idx = idx_c;
        return flpx::token_table::tokens[idx];
    }
<INITIAL>TR {
        yylloc_param->step(yyleng);
        return yy::parser::token::K_TR;
    }

<INITIAL>{NUM_TOK} {
        yylloc_param->step(yyleng);
        yylval->numval = new flpx::NumericValue(*(yylloc), yytext, yyleng);
        return yy::parser::token::NUM;
    }

<INITIAL>[\r]*[\n][\r]*               {
    yylloc_param->step(yyleng);
    yylloc_param->lines_no_bytes();
}
<INITIAL>[ \t]+           {
    yylloc_param->step(yyleng);
}

<INITIAL>[^\r\n\t MNLGRTHXYFIJES] {
        yylloc_param->step(yyleng);
        flpx::String *str = new flpx::String();
        *str = yytext;
        yylval->comment = str;
        return yy::parser::token::BAD_TOKEN;
    }
  /* <<EOF>> {
            return yy::parser::token::END_OF_FILE;
    } */

  /*<INITIAL>[^ \r\t\n]         {
    yylloc_param->step(yyleng);
    // return static_cast<yy::parser::token::yytokentype>(yytext[0]);
    } */

%%

#include "src/compiler/lexlib_internal.h"


#pragma GCC diagnostic pop

