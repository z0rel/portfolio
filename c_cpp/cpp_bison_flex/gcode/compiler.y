%{

#include "src/compiler/compiler_global.h"
#include "gen_flp_parser.h"
#include "gen_flp_lexer.h"

# define YYERROR_VERBOSE 1

extern void yyerror(YYLTYPE *locp, flpx::ParsingContext *ctx, yyscan_t, const char *msg);

%} 

/* Location tracking.  */


/* Pure yyparse.  */
%parse-param { flpx::ParsingContext *ctx }
%parse-param { yyscan_t yyscanner }
%parse-param { std::string *currentFilename }
%lex-param   { yyscan_t yyscanner }

%verbose
%error-verbose
%glr-parser
%skeleton "glr.cc"

%defines
%locations
%define api.location.type {cl::location}

%debug


%code requires {

#include "src/compiler/compiler_global.h"
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;

#define YY_NO_UNISTD_H
#include <cstdint>

#ifndef __GNUC__
#  define isatty(x) false
#else
#  include <unistd.h>
#endif

}


%code provides {

#define YYLLOC_DEFAULT(Current, Rhs, N)	\
    do \
      if (N) \
      { \
        (Current).begin = YYRHSLOC (Rhs, 1).begin;	\
        (Current).end  = YYRHSLOC (Rhs, N).end; \
      }	\
      else\
      {	\
        (Current).begin = (Current).end = (YYRHSLOC (Rhs, 0)).end; \
      }	\
    while (/*CONSTCOND*/ 0)


extern yy::parser::token::yytokentype yylex \
               (YYSTYPE * yylval_param,YYLTYPE * yylloc_param ,yyscan_t yyscanner);

#define YY_DECL yy::parser::token::yytokentype yylex \
               (YYSTYPE * yylval_param, YYLTYPE * yylloc_param , yyscan_t yyscanner)

}

%token <comment> COMMENT
%token <numval>  NUM
%token <comment> CADR_COMMENT

%token PERCENT
%token BR_CLOSE
%token K_L
%token K_G
%token K_R
%token K_M
%token K_N
%token K_T
%token K_TR
%token K_H
%token K_X
%token K_Y
%token K_F
%token K_I
%token K_J
%token K_E
%token K_S

%token BAD_TOKEN
%token END_OF_FILE


%%

program_body:
      COMMENT cadres_list_opt { ctx->program = new flpx::Program(@$, $1, $2); }
    | cadres_list_opt         { ctx->program = new flpx::Program(@$, NULL, $1); }
    ;

cadres_list_opt:
      %empty       { $$ = NULL; }
    | cadres_list  { $$ = $1; }
    ;
%type <cadres_list> cadres_list_opt;

cadres_list:
      cadr              { $$ = flpx::mkList(@$, $1); }
    | cadres_list cadr  { $$ = $1; $$->push_back($2); $$->loc(@$);  }
    ;
%type <cadres_list> cadres_list;

cadr:
      K_N NUM xysfj_chars { $$ = new flpx::Cadr(@$, $2, $3); }
    | K_N xysfj_chars { $$ = new flpx::Cadr(@$, nullptr, $2); }
    | PERCENT             {
          $$ = new flpx::Cadr(@$, nullptr, mkList(@$, new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_PERCENT, nullptr, nullptr)));
      }
    | kn_num_opt cadr_l_entry command_m_opt {
          flpx::Cadr *p = new flpx::Cadr(@$, $1, $2);
          $$ = p;
          if ($3) {
              p->body->push_back($3);
          }
      }
    ;
%type <cadr> cadr;


kn_num_opt:
      %empty    { $$ = NULL; }
    | K_N NUM   { $$ = $2;   }
    ;
%type <numval> kn_num_opt;


kr_comment_opt:
      %empty        { $$ = NULL; }
    | CADR_COMMENT  { $$ = $1; }
    ;
%type <comment> kr_comment_opt;


kr_item:
      K_R NUM kr_comment_opt { $$ = new flpx::XYJ_item(@2, flpx::XYJ_item::CMD_R,  $2, $3); }
    ;
%type <xyj_item> kr_item;

kr_list:
      kr_item          { $$ = mkList(@$, $1); }
    | kr_list kr_item  { $$ = $1; $$->push_back($2); $$->loc(@$); }
    ;
%type <xyj_entries> kr_list;


cadr_l_entry:
      K_L NUM  {
          $$ = mkList(@$, new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_L,  $2, NULL));
      }
    | K_L NUM kr_list {
          $$ = $3;
          $$->push_front(new flpx::XYJ_item(@2, flpx::XYJ_item::CMD_L,  $2, NULL));
      }
    ;
%type <xyj_entries> cadr_l_entry;

command_m_opt:
      %empty      { $$ = NULL; }
    | command_m   { $$ = $1; }
    ;
%type <xyj_item> command_m_opt;

command_m:
        K_M NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_M, $2, NULL); }
    ;
%type <xyj_item> command_m;

xysfj_chars:
      xysfj_chars_item                { $$ = mkList(@$, $1); }
    | xysfj_chars xysfj_chars_item    { $$ = $1; $$->push_back($2); $$->loc(@$); }
    ;
%type <xyj_entries> xysfj_chars;

xysfj_chars_item:
      K_X  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_X,  $2, NULL); }
    | K_Y  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_Y,  $2, NULL); }
    | K_I  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_I,  $2, NULL); }
    | K_J  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_J,  $2, NULL); }
    | K_F  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_F,  $2, NULL); }
    | K_E  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_E,  $2, NULL); }
    | K_G  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_G,  $2, NULL); }
    | K_S  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_S,  $2, NULL); }
    | K_H  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_H,  $2, NULL); }
    | K_T NUM CADR_COMMENT { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_T,  $2, $3); }
    | K_T  NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_T,  $2, NULL); }
    | K_TR NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_TR, $2, NULL); }
    | K_TR NUM CADR_COMMENT { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_TR, $2, $3); }
    | K_M NUM { $$ = new flpx::XYJ_item(@$, flpx::XYJ_item::CMD_M, $2, NULL); }
    ;


%type <xyj_item> xysfj_chars_item;



%%

namespace yy {
    void init_yyloc_default() {
        yyloc_default.initialize();
    }
}
