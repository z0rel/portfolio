%x Cds
%x Sql
%x Python
%x Password
%x SId1
%x SIdDot
%x SId2
%x SIdWrapped
%x EndOfHint



%option bison-bridge
%option bison-locations
%option extra-type="SyntaxerContext *"
%option noyywrap
%option reentrant
%option stack

%option debug

%{


// Cds        /* Classification of dynamic Sql */
// Sql
// Python
// Password
// SId1       /* state Id1         {<Id>}[.<Id>[<wrapped>]] */
// SIdDot     /* state IdDot       <Id>{.}[<Id>[<wrapped>]] */
// SId2       /* state Id2         <Id>.{<Id>}[<wrapped>]   */
// SIdWrapped /* state id wrapped  <Id>[.<Id>]{[<wrapped>]} */

  // <Sql>(?i:SELECT){s}"/*+"                          { llocBufSimple(yyscanner); RetTok(SELECT_HINT) }
  // <Sql>(?i:DELETE){s}"/*+"                          { llocBufSimple(yyscanner); RetTok(DELETE_HINT) }
  // <Sql>(?i:UPDATE){s}"/*+"                          { llocBufSimple(yyscanner); RetTok(UPDATE_HINT) }
  // <Sql>(?i:INSERT){s}"/*+"                          { llocBufSimple(yyscanner); RetTok(INSERT_HINT) }



#include "project_optimization.h"
COMPILER_LEXER_OPTIMIZATION_PUSH()

#include "yylex_decl.h"

#define ENABLE_DEBUG 0
#define FORCE_DEBUG_IN_ENABLE_RESTRICT_DEBUG 0

#define YY_INPUT(buf,result,max_size) \
    pLexInputData(yyin, buf, &result, max_size);


#define IS_FLEX_INCLUDE

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <string>
#include <iostream>
#include "semantic_id.h"
#include "semantic_base.h"
#include "model_context.h"
#include "sql_syntaxer_bison.h"
#include "lexlib_internal_header.h"
#include "lexsubtokenizer.h"
#include "parserdfa.h"

extern LexStringBuffer lexer_id_text;

extern Sm::LexSubtokenizer lexerSubtokenizer;

/// Вввод данных из файла во внутренний буффер flex
void lex_input_data(FILE *f, char *buf, yy_size_t *result, const yy_size_t max_size) {
  errno = 0;
  if (buf)  {
    while (!(*result = fread(buf, 1, max_size, f)) && ferror(f)) {
      if (errno == EINTR) {
        errno=0;
        clearerr(f);
      }
      else {
        fprintf( stderr, "input in lexer scanner failed\n" );
        exit(2);
     }
    }
  }
}


LexInputData pLexInputData = lex_input_data;


using namespace std;

extern int yydebug;

extern const yytype_uint16 *bison_stos   ; 
extern const yytype_uint16 *bison_translate; 

void lex_unput(int c, yyscan_t yyscanner);

/* Функциональность,  в которую не нужно помещать трассировку {{{*/

#define BASE_TOKEN_RETURN(tracerState, tokenId) \
  BEGIN(SId1); \
  llocBufSimple(yyscanner); \
  return yy::parser::token::tokenId;

#define sCompound(val)        yylval_param->sqlCompound = Sm::algebraic_compound::val;
#define sComparsion(val)      yylval->plComparsionOp    = Sm::ComparsionOp::val;

#define ReturnRawId \
  return yy::parser::token::RawID
//  cout << "==\"\"\"" << yylval_param->id->toString() << "\"\"\"==" << endl;

/*}}}*/

/* RawId {{{*/


#define LEX_GET_QUOTED2(endchar) \
  yylloc->step(); \
  yylloc->columns3(); \
  lex_get_quoted2<yyinput>(yyscanner, endchar); \

#define SQUOTED_RETURN2(endchar) \
  LEX_GET_QUOTED2(endchar) \
  yylval_param->id = new Sm::Identificator(yylloc, FLAG_ID_IS_QUOTED | FLAG_ID_IS_SQUOTED); \
  ReturnRawId;

#define TQUOTED_RETURN(quote) \
  yylloc->step(); \
  yylloc->columns(); \
  lex_get_3quoted<static_cast<int>(quote), yyinput>(yyscanner); \
  yylval_param->id = new Sm::Identificator(yylloc, FLAG_ID_IS_QUOTED); \
  ReturnRawId;

#define LEX_GET_QUOTED(quote, fun) \
  yylloc->step(); \
  yylloc->columns(); \
  fun<static_cast<int>(quote), yyinput>(yyscanner);

#define SQUOTED_RETURN(quote, fun) \
  LEX_GET_QUOTED(quote, fun) \
  yylval_param->id = new Sm::Identificator(yylloc, FLAG_ID_IS_QUOTED | FLAG_ID_IS_SQUOTED); \
  ReturnRawId;

#define DQUOTED_RETURN(quote, fun) \
  LEX_GET_QUOTED(quote, fun) \
  yylval_param->id = new Sm::Identificator(yylloc, FLAG_ID_IS_QUOTED); \
  ReturnRawId;




/*}}}*/

/* Keywords {{{*/

#define KEYWORD_RETURN( tokenId ) \
  yylloc_param->step(yyleng); \
  return yy::parser::token::tokenId;

// __VA_ARGS__
#define L_KEYWORD_RETURN( tokenId, ... ) \
  llocBufSimple(yyscanner); \
  return yy::parser::token::tokenId;
/*}}}*/

/* Numberic Tokens {{{*/
template <int leng_offset>
inline void getIntegerToken(yyscan_t yyscanner);

inline void getFloatToken(yyscan_t yyscanner);
/*}}}*/
 
#define RetTok(tok)   return yy::parser::token::tok;
#define RetForce(tok) return (yy::parser::token::yytokentype)(tok);


#define RetOnlyRawId() \
  lex_get_rawId(yyscanner); \
  ReturnRawId;

/*
** Nonquoted identifiers can contain only alphanumeric characters from your
** database character set and the underscore (_), dollar sign ($), and pound sign
** (# = 0x23). Database links can also contain periods (.) and "at" signs (@). Oracle
** strongly discourages you from using $ and # in nonquoted identifiers.
*/

%}

F_CP1251_SPEC   [\x80\x81\x83\x8A\x8C-\x90\x9A\x9C-\x9F\xA1-\xA3\xA5\xA8\xAA\xAF\xB2\xB3\xB4\xB8\xBA\xBC-\xFF]
F_ID_CHAR_SQDOL [\?#$a-zA-Z_]
L_ID_CHAR_SQDOL [\?#$a-zA-Z_0-9]
L_F_CP1251_SPEC [\?#$a-zA-Z_0-9\x80\x81\x83\x8A\x8C-\x90\x9A\x9C-\x9F\xA1-\xA3\xA5\xA8\xAA\xAF\xB2\xB3\xB4\xB8\xBA\xBC-\xFF]

FIN           [^[:alnum:]_#$]
NumFIN        [^0-9]
IntNumFIN     [.][.]|[^0-9.A-Za-z]
NoIntFIN      [^0-9.]
RangeFIN      [.][.]
FFIN          [^[:alnum:]_.]
ssp           [\ \t]+
slp           [\ \t\n\r]+
sp            [\ \t\n\r]*
s             [\ \t\n\r]+

NULL_CHARACTER_GROUP [\x0]

create (?i:CREATE)({s}(?i:OR){s}(?i:REPLACE))?{s}


%%

%{
#if ENABLE_DEBUG
   // yylloc_param->filename = yyget_extra(yyscanner)->currentFileName;
   if (lexerSubtokenizer.isOraParserStage() && yylloc_param->begin.line > 0) {
     yy_flex_debug = 1;
     yydebug = 1;
   }
   else {
     yy_flex_debug = FORCE_DEBUG_IN_ENABLE_RESTRICT_DEBUG;
     yydebug = FORCE_DEBUG_IN_ENABLE_RESTRICT_DEBUG;
   }
#endif
%}

  /* Комментарии (многострочный и однострочный) // TODO: Запилить сохранение и вывод комментариев {{{ */
<Python>"# == START SKIP =="[^\r\n]* { yylloc_param->step(yyleng); RetTok(cfg_ENABLE_SKIP_TOKENS) }
<Python>"# == STOP SKIP =="[^\r\n]* { yylloc_param->step(yyleng); RetTok(cfg_DISABLE_SKIP_TOKENS) }
<Python>"#"[^\r\n]*   { yylloc_param->step(yyleng); } /* однострочный комментарий */

<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped>--[^\r\n]*   { yylloc_param->step(yyleng); } /* однострочный комментарий */
<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped>"//"[^\r\n]* { yylloc_param->step(yyleng); }
<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped>"/*"         { yylloc_param->step(2); lex_get_multiline_comment<yyinput, 2>(yyscanner); } /* TODO: Запилить сохранение многострочных комментариев */
  /* }}} */
  /* Пробелы и переводы строк {{{ */
<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped,Python,Password>[ \t]+ { yylloc_param->step(yyleng); }    /* Обработка размещений для цепочек пробелов и табуляций */
<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped,Python>[\r]            { yylloc_param->bytes(); }
<Sql,Cds,SId1,SIdDot,SId2,SIdWrapped,Python>[\n]            { yylloc_param->step(); yylloc_param->lines();  } /* Обработка конца строки и перевода каретки */
  /*}}}*/

  /* Идентификаторы и закавыченные строки для режима считывания двойного имени {{{ */
<SId1>'                 { BEGIN SIdDot; SQUOTED_RETURN('\'', lex_get_quoted); }
<SId1>["]               { BEGIN SIdDot; DQUOTED_RETURN('\"', lex_get_quoted); }
<SId1>q'.               { BEGIN SIdDot; SQUOTED_RETURN2(yytext[2]); }
<SId1>{F_ID_CHAR_SQDOL} { BEGIN SIdDot; RetOnlyRawId() } /* Незакавыченный идентификатор: ID */
<SId1>.                 { BEGIN Sql; lex_unput(yytext[0], yyscanner); }

<SId2>'                 { BEGIN SIdWrapped; SQUOTED_RETURN('\'', lex_get_quoted)  }
<SId2>["]               { BEGIN SIdWrapped; DQUOTED_RETURN('\"', lex_get_quoted);  }
<SId2>q'.               { BEGIN SIdWrapped; SQUOTED_RETURN2(yytext[2]);  }
<SId2>{F_ID_CHAR_SQDOL} { BEGIN SIdWrapped; RetOnlyRawId() } /* Незакавыченный идентификатор: ID */

<SIdDot>"."       { BEGIN SId2; yylloc_param->step1(); RetForce('.') }

<SIdDot,SIdWrapped>. { BEGIN Sql; lex_unput(yytext[0], yyscanner); }
<SIdDot,SIdWrapped>(?i:WRAPPED)/{FIN} {
                               BEGIN Sql;
                               yylloc->step();
//                               yylloc->columns7();
                               skipWrappedEntity<yyinput>(yyscanner);
                               RetTok(WRAPPED_ENTITY)
                             }

  /*}}}*/
  /* Операторы: арифметика, логика {{{ */

<Python>"+="        { yylloc_param->step2(); RetTok(cfg_ADD_SELF); }
<Python>"|="        { yylloc_param->step2(); RetTok(cfg_UNION_SELF); }
<Python>"-="        { yylloc_param->step2(); RetTok(cfg_MINUS_SELF); }

<Sql,Cds>"**"           { yylloc_param->step2();      sCompound(DEGREE)    RetTok(op_EXP)    }
<Sql,Cds>"*"{ssp}"*"    { yylloc_param->step(yyleng); sCompound(DEGREE)    RetTok(op_EXP)    }
<Sql,Cds>"*"{slp}"*"    { llocBuf<1,1>(yyscanner); sCompound(DEGREE)    RetTok(op_EXP)    }

<Sql,Cds>"||"           { yylloc_param->step2();      sCompound(CONCAT)    RetTok(op_CONCAT) }
<Sql,Cds>"|"{ssp}"|"    { yylloc_param->step(yyleng); sCompound(CONCAT)    RetTok(op_CONCAT) }
<Sql,Cds>"|"{slp}"|"    { llocBuf<1,1>(yyscanner); sCompound(CONCAT)    RetTok(op_CONCAT) }


<Sql,Cds>"-"            { yylloc_param->step1(); sCompound(MINUS)          RetForce('-') }
<Sql,Cds>"+"            { yylloc_param->step1(); sCompound(PLUS)           RetForce('+') }
<Sql,Cds>"/"            { yylloc_param->step1(); sCompound(DIVIDE)         RetForce('/') }
<Sql,Cds>"*"            { yylloc_param->step1(); sCompound(MULTIPLE)       RetForce('*') }
<Sql,Cds>"%"            { yylloc_param->step1();                           RetForce('%') }
<Sql,Cds>"^"            { yylloc_param->step1();                           RetForce('^') }
<Sql,Cds>","            { yylloc_param->step1();                           RetForce(',') }
<Sql,Cds>"."            { yylloc_param->step1();                           RetForce('.') }
<Sql,Cds>":"            { yylloc_param->step1();                           RetForce(':') }
<Sql>"("                { ++lexerSubtokenizer.braceDeep; yylloc_param->step1(); RetForce('(') }
<Sql>")"                { --lexerSubtokenizer.braceDeep; yylloc_param->step1(); RetForce(')') }
<Sql,Cds>";"            { lexerSubtokenizer.popStateOnSemicolon(); yylloc_param->step1(); RetForce(';') }
<Sql,Cds>"<"            { yylloc_param->step1(); sComparsion(LT)           RetForce('<') }
<Sql,Cds>">"            { yylloc_param->step1(); sComparsion(GT)           RetForce('>') }
<Sql,Cds>"="            { yylloc_param->step1(); sComparsion(EQ)           RetForce('=') }

<Sql,Cds>">="           { yylloc_param->step2();      sComparsion(GE)      RetTok(GE)    }
<Sql,Cds>">"{ssp}"="    { yylloc_param->step(yyleng); sComparsion(GE)      RetTok(GE)    }
<Sql,Cds>">"{slp}"="    { llocBuf<1,1>(yyscanner); sComparsion(GE)  RetTok(GE)    }

<Sql,Cds>"<="           { yylloc_param->step2();      sComparsion(LE)      RetTok(LE)    }
<Sql,Cds>"<"{ssp}"="    { yylloc_param->step(yyleng); sComparsion(LE)      RetTok(LE)    }
<Sql,Cds>"<"{slp}"="    { llocBuf<1,1>(yyscanner); sComparsion(LE)  RetTok(LE)    }

<Sql,Cds>"<>"           { yylloc_param->step2();      sComparsion(NEQ)     RetTok(NE)    }
<Sql,Cds>"<"{ssp}">"    { yylloc_param->step(yyleng); sComparsion(NEQ)     RetTok(NE)    }
<Sql,Cds>"<"{slp}">"    { llocBuf<1,1>(yyscanner);    sComparsion(NEQ)     RetTok(NE)    }

<Sql,Cds>[!~^]=         { yylloc_param->step2();      sComparsion(NEQ)     RetTok(NE)    }
<Sql,Cds>[!~^]{ssp}=    { yylloc_param->step(yyleng); sComparsion(NEQ)     RetTok(NE)    }
<Sql,Cds>[!~^]{slp}=    { llocBuf<1,1>(yyscanner);    sComparsion(NEQ)     RetTok(NE)    }

<Sql,Cds>":="           { yylloc_param->step2();                           RetTok(op_ASSIGN)        }
<Sql,Cds>":"{ssp}"="    { yylloc_param->step(yyleng);                      RetTok(op_ASSIGN)        }
<Sql,Cds>":"{slp}"="    { llocBuf<1,1>(yyscanner);                         RetTok(op_ASSIGN)        }

<Sql,Cds>"=>"           { yylloc_param->step2();                           RetTok(op_FORMAL_ASSIGN) }
<Sql,Cds>"="{ssp}">"    { yylloc_param->step(yyleng);                      RetTok(op_FORMAL_ASSIGN) }
<Sql,Cds>"="{slp}">"    { llocBuf<1,1>(yyscanner);                         RetTok(op_FORMAL_ASSIGN) }

<Sql,Cds>".."           { yylloc_param->step2();                           RetTok(op_RANGE)         }
<Sql,Cds>"<<"           { yylloc_param->step2();                           RetTok(op_LABEL_OPEN)    }
<Sql,Cds>">>"           { yylloc_param->step2();                           RetTok(op_LABEL_CLOSE)   }

  /*  }}} */

  /* Ключевые слова : TODO: Ассоциации с комментариями и позициями в исходнке  */

<Sql>{create}(?i:USER)/{FIN}                                   { lexerSubtokenizer.pushCreateUser();        BASE_TOKEN_RETURN(CREATE_USER, CREATE_USER) }
<Sql>{create}((?i:FORMAT71){s})?(?i:PACKAGE)/{FIN}             { lexerSubtokenizer.pushCreatePackage();     BASE_TOKEN_RETURN(CREATE_PACKAGE, CREATE_PACKAGE) }
<Sql>{create}((?i:FORMAT71){s})?(?i:PACKAGE){s}(?i:BODY)/{FIN} { lexerSubtokenizer.pushCreatePackageBody(); BASE_TOKEN_RETURN(CREATE_PACKAGE  , CREATE_PACKAGE_BODY) }
<Sql>{create}(?i:SEQUENCE)/{FIN}                               { lexerSubtokenizer.pushCreateSequence();    BASE_TOKEN_RETURN(CREATE_SEQUENCE, CREATE_SEQUENCE) }
<Sql>{create}(?i:TRIGGER)/{FIN}                                { lexerSubtokenizer.pushCreateTrigger();     BASE_TOKEN_RETURN(CREATE_TRIGGER, CREATE_TRIGGER) }
<Sql>{create}(?i:TYPE)/{FIN}                                   { lexerSubtokenizer.pushCreateType();        BASE_TOKEN_RETURN(CREATE_TYPE, CREATE_TYPE) }
<Sql>{create}(?i:TYPE){s}(?i:BODY)/{FIN}                       { lexerSubtokenizer.pushCreateTypeBody();    BASE_TOKEN_RETURN(CREATE_TYPE, CREATE_TYPE_BODY) }
<Sql>{create}(((?i:NO){s})?(?i:FORCE){s})?(?i:VIEW)/{FIN}      { lexerSubtokenizer.pushCreateView();        BASE_TOKEN_RETURN(CREATE_VIEW, CREATE_VIEW) }
<Sql>{create}(((?i:NO){s})?(?i:FORCE){s})?(?i:MATERIALIZED){s}(?i:VIEW)/{FIN}  { lexerSubtokenizer.pushCreateView(); BASE_TOKEN_RETURN(CREATE_VIEW, CREATE_MATERIALIZED_VIEW) }
<Sql>{create}(?i:FUNCTION)/{FIN}                               { lexerSubtokenizer.pushCreateFunction();    BASE_TOKEN_RETURN(CREATE_FUNCTION, CREATE_FUNCTION) }
<Sql>{create}(?i:PROCEDURE)/{FIN}                              { lexerSubtokenizer.pushCreateProcedure();   BASE_TOKEN_RETURN(CREATE_PROCEDURE, CREATE_PROCEDURE) }
<Sql>{create}(?i:TABLE)/{FIN}                                  { lexerSubtokenizer.pushCreateTable();       BASE_TOKEN_RETURN(CREATE_TABLE, CREATE_TABLE) }
<Sql>{create}(?i:GLOBAL){s}(?i:TEMPORARY){s}(?i:TABLE)/{FIN}   { lexerSubtokenizer.pushCreateTable();       BASE_TOKEN_RETURN(CREATE_TABLE, CREATE_GLOBAL_TEMPORARY_TABLE) }
<Sql>(?i:ALTER){s}(?i:USER)/{FIN}                              { lexerSubtokenizer.pushAlterUser();         BASE_TOKEN_RETURN(ALTER_USER, ALTER_USER) }
<Sql>(?i:ALTER){s}(?i:TABLE)/{FIN}                             { lexerSubtokenizer.pushAlterTable();        BASE_TOKEN_RETURN(ALTER_TABLE, ALTER_TABLE) }
<Sql>{create}(?i:INDEX)/{FIN}                                  { lexerSubtokenizer.pushCreateIndex();       BASE_TOKEN_RETURN(CREATE_INDEX    , CREATE_INDEX                      )  }
<Sql>{create}(?i:UNIQUE){s}(?i:INDEX)/{FIN}                    { lexerSubtokenizer.pushCreateIndex();       BASE_TOKEN_RETURN(CREATE_INDEX    , CREATE_UNIQUE_INDEX               )  }
<Sql>{create}(?i:BITMAP){s}(?i:INDEX)/{FIN}                    { lexerSubtokenizer.pushCreateIndex();       BASE_TOKEN_RETURN(CREATE_INDEX    , CREATE_BITMAP_INDEX               )  }

<Sql>{create}(?i:SYNONYM)/{FIN}                                             { BASE_TOKEN_RETURN(CREATE_SYNONYM  , CREATE_SYNONYM                    )  }
<Sql>{create}(?i:PUBLIC){s}(?i:SYNONYM)/{FIN}                               { BASE_TOKEN_RETURN(CREATE_SYNONYM  , CREATE_PUBLIC_SYNONYM             )  }
<Sql>{create}(?i:DATABASE){s}(?i:LINK)/{FIN}                                { BASE_TOKEN_RETURN(CREATE_DBLINK   , CREATE_DATABASE_LINK              )  }
<Sql>{create}(?i:SHARED){s}(?i:DATABASE){s}(?i:LINK)/{FIN}                  { BASE_TOKEN_RETURN(CREATE_DBLINK   , CREATE_SHARED_DATABASE_LINK       )  }
<Sql>{create}(?i:PUBLIC){s}(?i:DATABASE){s}(?i:LINK)/{FIN}                  { BASE_TOKEN_RETURN(CREATE_DBLINK   , CREATE_PUBLIC_DATABASE_LINK       )  }
<Sql>{create}(?i:SHARED){s}(?i:PUBLIC){s}(?i:DATABASE){s}(?i:LINK)/{FIN}    { BASE_TOKEN_RETURN(CREATE_DBLINK   , CREATE_SHARED_PUBLIC_DATABASE_LINK)  }



<Sql>%{sp}(?i:ROWTYPE)/{FIN}                                                { L_KEYWORD_RETURN(pROWTYPE                            , "%")  }
<Sql>%{sp}(?i:TYPE)/{FIN}                                                   { L_KEYWORD_RETURN(pTYPE                               , "%")  }
<Sql>(?i:ALTER){s}(?i:TRIGGER)/{FIN}                                        { L_KEYWORD_RETURN(ALTER_TRIGGER                       )  }
<Sql>(?i:MODEL){s}(?i:IGNORE){s}(?i:NAV)/{FIN}                              { L_KEYWORD_RETURN(MODEL_IGNORE_NAV                     , MODEL, IGNORE, NAV )  }
<Sql>(?i:MODEL){s}(?i:KEEP){s}(?i:NAV)/{FIN}                                { L_KEYWORD_RETURN(MODEL_KEEP_NAV                       , MODEL, KEEP, NAV )  }
<Sql>(?i:GROUPING){s}(?i:SETS)/{FIN}                                        { L_KEYWORD_RETURN(GROUPING_SETS                       , GROUPING, SETS )  }
<Sql>(?i:SAMPLE){s}(?i:BLOCK)/{FIN}                                         { L_KEYWORD_RETURN(SAMPLE_BLOCK                        , SAMPLE, BLOCK )  }
<Sql>(?i:ACCESS){s}(?i:PARAMETERS)/{FIN}                                    { L_KEYWORD_RETURN(ACCESS_PARAMETERS                   , ACCESS, PARAMETERS )  }
<Sql>(?i:ACCOUNT){s}(?i:LOCK)/{FIN}                                         { L_KEYWORD_RETURN(ACCOUNT_LOCK                        , ACCOUNT, LOCK )  }
<Sql>(?i:ACCOUNT){s}(?i:UNLOCK)/{FIN}                                       { L_KEYWORD_RETURN(ACCOUNT_UNLOCK                      , ACCOUNT, UNLOCK )  }
<Sql>(?i:ALLOCATE){s}(?i:EXTENT)/{FIN}                                      { L_KEYWORD_RETURN(ALLOCATE_EXTENT                     , ALLOCATE, EXTENT )  }
<Sql>(?i:AS){s}(?i:CLOB)/{FIN}                                              { L_KEYWORD_RETURN(AS_CLOB                             , AS, CLOB )  }
<Sql>(?i:AS){s}(?i:VALUE)/{FIN}                                             { L_KEYWORD_RETURN(AS_VALUE                            , AS, VALUE )  }
<Sql>(?i:AUTHENTICATED){s}(?i:BY)/{FIN}                                     { L_KEYWORD_RETURN(AUTHENTICATED_BY                    , AUTHENTICATED, BY )  }
<Sql>(?i:C){s}(?i:LIBRARY)/{FIN}                                            { L_KEYWORD_RETURN(C_LIBRARY                           , C, LIBRARY )  }
<Sql>(?i:C){s}(?i:NAME)/{FIN}                                               { L_KEYWORD_RETURN(C_NAME                              , C, NAME )  }
<Sql>(?i:COMMIT){s}(i:WORK)/{FIN}                                           { L_KEYWORD_RETURN(COMMIT                              , COMMIT, WORK )  }
<Sql>(?i:CONSTRUCTOR){s}(?i:FUNCTION)/{FIN}                                 { L_KEYWORD_RETURN(CONSTRUCTOR_FUNCTION                , CONSTRUCTOR, FUNCTION )  }
<Sql>(?i:DEALLOCATE){s}(?i:UNUSED)/{FIN}                                    { L_KEYWORD_RETURN(DEALLOCATE_UNUSED                   , DEALLOCATE, UNUSED )  }
<Sql>(?i:DEFAULT){s}(?i:ROLE)                                               { L_KEYWORD_RETURN(DEFAULT_ROLE                        , DEFAULT, ROLE )  }
<Sql>(?i:DROP){s}(?i:PACKAGE){s}(?i:BODY)/{FIN}                             { L_KEYWORD_RETURN(DROP_PACKAGE_BODY                   , DROP, PACKAGE, BODY )  }
<Sql>(?i:DROP){s}(?i:TYPE)({s}(?i:BODY))?/{FIN}                             { L_KEYWORD_RETURN(DROP_TYPE                           , DROP, TYPE, BODY )  }
<Sql>(?i:DROP){s}(?i:TYPE)/{FIN}                                            { L_KEYWORD_RETURN(DROP_TYPE                           , DROP, TYPE )  }
<Sql>(?i:EXCEPTIONS){s}(?i:INTO)/{FIN}                                      { L_KEYWORD_RETURN(EXCEPTIONS_INTO                     , EXCEPTIONS, INTO )  }
<Sql>(?i:EXTERNAL){s}(?i:NAME)/{FIN}                                        { L_KEYWORD_RETURN(EXTERNAL_NAME                       , EXTERNAL, NAME )  }
<Sql>(?i:FLASHBACK){s}(?i:ARCHIVE)/{FIN}                                    { L_KEYWORD_RETURN(NO_FLASHBACK_ARCHIVE                , FLASHBACK, ARCHIVE )  }
<Sql>(?i:FOREIGN){s}(?i:KEY)/{FIN}                                          { L_KEYWORD_RETURN(FOREIGN_KEY                         , FOREIGN, KEY )  }
<Sql>(?i:FULL){s}(?i:JOIN)/{FIN}                                            { L_KEYWORD_RETURN(FULL_JOIN                           , FULL, JOIN )  }
<Sql>(?i:FULL){s}(?i:OUTER){s}(?i:JOIN)/{FIN}                               { L_KEYWORD_RETURN(FULL_OUTER_JOIN                     , FULL, OUTER, JOIN )  }
<Sql>(?i:GENERATED){s}(?i:ALWAYS)/{FIN}                                     { L_KEYWORD_RETURN(GENERATED_ALWAYS                    , GENERATED, ALWAYS )  }
<Sql>(?i:GLOBAL){s}(?i:PARTITION){s}(?i:BY)/{FIN}                           { L_KEYWORD_RETURN(GLOBAL_PARTITION_BY                 , GLOBAL, PARTITION, BY )  }
<Sql>(?i:GO){sp}(?i:TO)/{FIN}                                               { L_KEYWORD_RETURN(GOTO                                , GO, TO)  }
<Sql>(?i:IDENTIFIED){s}(?i:BY)/{FIN}                                        { yy_push_state(Password, yyscanner); L_KEYWORD_RETURN(IDENTIFIED_BY, IDENTIFIED, BY)  }
<Sql>(?i:IDENTIFIED){s}(?i:EXTERNALLY)/{FIN}                                { L_KEYWORD_RETURN(IDENTIFIED_EXTERNALLY                , IDENTIFIED, EXTERNALLY)  }
<Sql>(?i:IDENTIFIED){s}(?i:EXTERNALLY){s}(?i:AS)/{FIN}                      { L_KEYWORD_RETURN(IDENTIFIED_EXTERNALLY_AS             , IDENTIFIED, EXTERNALLY, AS)  }
<Sql>(?i:IDENTIFIED){s}(?i:GLOBALLY)/{FIN}                                  { L_KEYWORD_RETURN(IDENTIFIED_GLOBALLY                  , IDENTIFIED, GLOBALLY)  }
<Sql>(?i:IDENTIFIED){s}(?i:GLOBALLY){s}(?i:AS)/{FIN}                        { L_KEYWORD_RETURN(IDENTIFIED_GLOBALLY_AS               , IDENTIFIED, GLOBALLY, AS)  }
<Sql>(?i:INDEXTYPE){s}(?i:IS)/{FIN}                                         { L_KEYWORD_RETURN(INDEXTYPE_IS                         , INDEXTYPE, IS)  }
<Sql>(?i:INNER){s}(?i:JOIN)/{FIN}                                           { L_KEYWORD_RETURN(INNER_JOIN                           , INNER, JOIN)  }
<Sql>(?i:IS){s}(?i:RECORD)/{FIN}                                            { L_KEYWORD_RETURN(IS_RECORD                            , IS, RECORD)  }
<Sql>(?i:JAVA){s}(?i:NAME)/{FIN}                                            { L_KEYWORD_RETURN(JAVA_NAME                            , JAVA, NAME)  }
<Sql>(?i:JAVA){s}(?i:RESOURCE)/{FIN}                                        { L_KEYWORD_RETURN(JAVA_RESOURCE                        , JAVA, RESOURCE)  }
<Sql>(?i:JAVA){s}(?i:SOURCE)/{FIN}                                          { L_KEYWORD_RETURN(JAVA_SOURCE                          , JAVA, SOURCE)  }
<Sql>(?i:KEEP){s}(?i:INDEX)/{FIN}                                           { L_KEYWORD_RETURN(KEEP_INDEX                           , KEEP, INDEX)  }
<Sql>(?i:LEFT){s}(?i:JOIN)/{FIN}                                            { L_KEYWORD_RETURN(LEFT_JOIN                            , LEFT, JOIN)  }
<Sql>(?i:LEFT){s}(?i:OUTER){s}(?i:JOIN)/{FIN}                               { L_KEYWORD_RETURN(LEFT_OUTER_JOIN                      , LEFT, OUTER, JOIN)  }
<Sql>(?i:LIMIT){s}(?i:UNLIMITED)/{FIN}                                      { L_KEYWORD_RETURN(LIMIT_UNLIMITED                      , LIMIT, UNLIMITED)  }
<Sql>(?i:LOG){s}(?i:ERRORS)/{FIN}                                           { L_KEYWORD_RETURN(LOG_ERRORS                           , LOG, ERRORS)  }
<Sql>(?i:NO){s}(?i:FLASHBACK){s}(?i:ARCHIVE)/{FIN}                          { L_KEYWORD_RETURN(FLASHBACK_ARCHIVE                    , NO, FLASHBACK, ARCHIVE)  }
<Sql>(?i:NO){s}(?i:LOG)/{FIN}                                               { L_KEYWORD_RETURN(NO_LOG                               , NO, LOG)  }
<Sql>(?i:NO){s}(?i:ROLES)/{FIN}                                             { L_KEYWORD_RETURN(NO_ROLES                             , NO, ROLES)  }
<Sql>(?i:NO){s}(?i:SALT)/{FIN}                                              { L_KEYWORD_RETURN(NO_SALT                              , NO, SALT)  }
<Sql>(?i:NULLS){s}(?i:FIRST)/{FIN}                                          { L_KEYWORD_RETURN(NULLS_FIRST                          , NULLS, FIRST)  }
<Sql>(?i:NULLS){s}(?i:LAST)/{FIN}                                           { L_KEYWORD_RETURN(NULLS_LAST                           , NULLS, LAST)  }
<Sql>(?i:OBJECT){s}(?i:IDENTIFIER)/{FIN}                                    { L_KEYWORD_RETURN(OBJECT_IDENTIFIER                    , OBJECT, IDENTIFIER)  }
<Sql>(?i:OBJECT){s}(?i:IDENTIFIER){s}(?i:IS){s}(?i:SYSTEM){s}(?i:GENERATED) { L_KEYWORD_RETURN(OBJECT_IDENTIFIER_IS_SYSTEM_GENERATED, OBJECT, IDENTIFIER, IS, SYSTEM, GENERATED)  }
<Sql>(?i:OF){s}(?i:TYPE)/{FIN}                                              { L_KEYWORD_RETURN(OF_TYPE                              , OF, TYPE)  }
<Sql>(?i:ON){s}(?i:COMMIT)/{FIN}                                            { L_KEYWORD_RETURN(ON_COMMIT                            , ON, COMMIT)  }
<Sql>(?i:ON){s}(?i:COMMIT)/{FIN}                                            { L_KEYWORD_RETURN(ON_COMMIT                            , ON, COMMIT)  }
<Sql>(?i:ON){s}(?i:COMMIT){s}(?i:DELETE){s}(?i:ROWS)/{FIN}                  { L_KEYWORD_RETURN(ON_COMMIT_DELETE_ROWS                , ON, COMMIT, DELETE, ROWS)  }
<Sql>(?i:ON){s}(?i:COMMIT){s}(?i:PRESERVE){s}(?i:ROWS)/{FIN}                { L_KEYWORD_RETURN(ON_COMMIT_PRESERVE_ROWS              , ON, COMMIT, PRESERVE, ROWS)  }
<Sql>(?i:ON){s}(?i:DATABASE)/{FIN}                                          { L_KEYWORD_RETURN(ON_DATABASE                          , ON, DATABASE)  }
<Sql>(?i:ORGANIZATION){s}(?i:EXTERNAL)/{FIN}                                { L_KEYWORD_RETURN(ORGANIZATION_EXTERNAL                , ORGANIZATION, EXTERNAL)  }
<Sql>(?i:ORGANIZATION){s}(?i:HEAP)/{FIN}                                    { L_KEYWORD_RETURN(ORGANIZATION_HEAP                    , ORGANIZATION, HEAP)  }
<Sql>(?i:ORGANIZATION){s}(?i:INDEX)/{FIN}                                   { L_KEYWORD_RETURN(ORGANIZATION_INDEX                   , ORGANIZATION, INDEX)  }
<Sql>(?i:PARTITION){s}(?i:BY){s}(?i:SYSTEM)                                 { L_KEYWORD_RETURN(PARTITION_BY_SYSTEM                  , PARTITION, BY, SYSTEM)  }
<Sql>(?i:PASSWORD){s}(?i:EXPIRE)/{FIN}                                      { L_KEYWORD_RETURN(PASSWORD_EXPIRE                      , PASSWORD, EXPIRE)  }
<Sql>(?i:PRIMARY){s}(?i:KEY)/{FIN}                                          { L_KEYWORD_RETURN(PRIMARY_KEY                          , PRIMARY, KEY)  }
<Sql>(?i:READ){s}(?i:WRITE)/{FIN}                                           { L_KEYWORD_RETURN(READ_WRITE                           , READ, WRITE)  }
<Sql>(?i:REJECT){s}(?i:LIMIT)/{FIN}                                         { L_KEYWORD_RETURN(REJECT_LIMIT                         , REJECT, LIMIT)  }
<Sql>(?i:RETURN){s}(?i:UPDATED){s}(?i:ROWS)/{FIN}                           { L_KEYWORD_RETURN(RETURN_UPDATED_ROWS                  , RETURN, UPDATED, ROWS)  }
<Sql>(?i:RIGHT){s}(?i:JOIN)/{FIN}                                           { L_KEYWORD_RETURN(RIGHT_JOIN                           , RIGHT, JOIN)  }
<Sql>(?i:RIGHT){s}(?i:OUTER){s}(?i:JOIN)/{FIN}                              { L_KEYWORD_RETURN(RIGHT_OUTER_JOIN                     , RIGHT, OUTER, JOIN)  }
<Sql>(?i:ROLLBACK){s}(?i:WORK)/{FIN}                                        { L_KEYWORD_RETURN(ROLLBACK                             , ROLLBACK, WORK)  }
<Sql>(?i:SHOW){s}(?i:ERRORS)/{FIN}                                          { L_KEYWORD_RETURN(SHOW_ERRORS                          , SHOW, ERRORS)  }
<Sql>(?i:SHRINK){s}(?i:SPACE)/{FIN}                                         { L_KEYWORD_RETURN(SHRINK_SPACE                         , SHRINK, SPACE)  }
<Sql>(?i:SKIP){s}(?i:LOCKED)/{FIN}                                          { L_KEYWORD_RETURN(SKIP_LOCKED                          , SKIP, LOCKED)  }
<Sql>(?i:SUBPARTITION){s}(?i:TEMPLATE)/{FIN}                                { L_KEYWORD_RETURN(SUBPARTITION_TEMPLATE                , SUBPARTITION, TEMPLATE)  }
<Sql>(?i:SUBSTITUTABLE){s}(?i:AT){s}(?i:ALL){s}(?i:LEVELS)                  { L_KEYWORD_RETURN(SUBSTITUTABLE_AT_ALL_LEVELS          , SUBSTITUTABLE, AT, ALL, LEVELS)  }
<Sql>(?i:SUPPLEMENTAL){s}(?i:LOG)/{FIN}                                     { L_KEYWORD_RETURN(SUPPLEMENTAL_LOG                     , SUPPLEMENTAL, LOG)  }
<Sql>(?i:TEMPORARY){s}(?i:TABLESPACE)/{FIN}                                 { L_KEYWORD_RETURN(TEMPORARY_TABLESPACE                 , TEMPORARY, TABLESPACE)  }
<Sql>(?i:THROUGH){s}(?i:ENTERPRISE){s}(?i:USERS)/{FIN}                      { L_KEYWORD_RETURN(THROUGH_ENTERPRISE_USERS             , THROUGH, ENTERPRISE, USERS)  }
<Sql>(?i:UNLIMITED){s}(?i:ON)/{FIN}                                         { L_KEYWORD_RETURN(UNLIMITED_ON                         , UNLIMITED, ON)  }
<Sql>(?i:USE){s}(?i:ROLLBACK){s}(?i:SEGMENT)/{FIN}                          { L_KEYWORD_RETURN(USE_ROLLBACK_SEGMENT                 , USE, ROLLBACK, SEGMENT)  }
<Sql>(?i:USING){s}(?i:CLOB)/{FIN}                                           { L_KEYWORD_RETURN(USING_CLOB                           , USING, CLOB)  }
<Sql>(?i:VERSION){s}(?i:BETWEEN)/{FIN}                                      { L_KEYWORD_RETURN(VERSION_BETWEEN                      , VERSION, BETWEEN)  }
<Sql>(?i:WHEN){s}(?i:MATCHED){s}(?i:THEN){s}(?i:UPDATE){s}(?i:SET)/{FIN}    { L_KEYWORD_RETURN(WHEN_MATCHED_THEN_UPDATE_SET         , WHEN, MATCHED, THEN, UPDATE, SET)  }
<Sql>(?i:WHEN){s}(?i:NOT){s}(?i:MATCHED){s}(?i:THEN){s}(?i:INSERT)/{FIN}    { L_KEYWORD_RETURN(WHEN_NOT_MATCHED_THEN_INSERT         , WHEN, NOT, MATCHED, THEN, INSERT)  }
<Sql>(?i:WITH){s}(?i:CONTEXT)/{FIN}                                         { L_KEYWORD_RETURN(WITH_CONTEXT                         , WITH, CONTEXT)  }
<Sql>(?i:WITH){s}(?i:LOCAL){s}(?i:TIME){s}(?i:ZONE){s}                      { L_KEYWORD_RETURN(WITH_LOCAL_TIME_ZONE                 , WITH, LOCAL, TIME, ZONE) }
<Sql>(?i:WITH){s}(?i:ROLE)                                                  { L_KEYWORD_RETURN(WITH_ROLE                            , WITH, ROLE) }
<Sql>(?i:WITH){s}(?i:TIME){s}(?i:ZONE)                                      { L_KEYWORD_RETURN(WITH_TIME_ZONE                       , WITH, TIME, ZONE) }

  /* Режим считывания пароля */
<Password>[^[:space:]]+ {
    yy_pop_state(yyscanner);
    yylloc_param->step(yyleng);
    yylval_param->id = new Sm::Identificator(Sm::Identificator::getNQHashValue(yytext, yyleng), 0, yytext, yyleng, yylloc_param);
    ReturnRawId;
  }


  /* Пустые кавычки - EMPTY_ID */
<Sql,Cds,Python>''/[^'] { KEYWORD_RETURN(EMPTY_ID); }
<Sql,Cds>["]["]/[^"]    { KEYWORD_RETURN(EMPTY_ID); }

  /* Идентификаторы и закавыченные строки для python */
<Python>'             { SQUOTED_RETURN('\'', lex_get_python_quoted) }
<Python>["]           { DQUOTED_RETURN('\"', lex_get_python_quoted); }
<Python>["]["]["]     { TQUOTED_RETURN('\"'); }


  /* Идентификаторы и закавыченные строки для обычного режима */
<Sql>'                        { SQUOTED_RETURN('\'', lex_get_quoted)  }
<Sql>["]                      { DQUOTED_RETURN('\"', lex_get_quoted); }
<Sql>q'.                      { SQUOTED_RETURN2(yytext[2]);           }


<Sql>{F_ID_CHAR_SQDOL}{L_ID_CHAR_SQDOL}*{F_CP1251_SPEC}{L_F_CP1251_SPEC}* { // незакавыченный идентификатор с русскими буквами
    if (yyget_extra(yyscanner)->stage == SyntaxerContext::SYNTAX_ANALYZE)
      cout << "lexer warning: identifier has bad character: " << yytext << " " << *yylloc_param << endl;
    yylloc_param->step(yyleng);
    yylval_param->id = new Sm::Identificator(Sm::Identificator::getNQHashValue(yytext, yyleng), 0, yytext, yyleng, yylloc_param);
    ReturnRawId;
  }


<Sql>{F_CP1251_SPEC}{L_F_CP1251_SPEC}* { // незакавыченный идентификатор с русскими буквами
    if (yyget_extra(yyscanner)->stage == SyntaxerContext::SYNTAX_ANALYZE)
      cout << "lexer warning: identifier has bad character: " << yytext << " " << *yylloc_param << endl;
    yylloc_param->step(yyleng);
    yylval_param->id = new Sm::Identificator(Sm::Identificator::getNQHashValue(yytext, yyleng), 0, yytext, yyleng, yylloc_param);
    ReturnRawId;
  }



<Sql,Python>{F_ID_CHAR_SQDOL} { /* Незакавыченный идентификатор: ID */
    return (yy::parser::token::yytokentype)(lexerSubtokenizer.getSubtoken(yyscanner));
//    yy::parser::token::yytokentype zzzi;
//    if ((zzzi = (yy::parser::token::yytokentype)(lexerSubtokenizer.getSubtoken(yyscanner))) == yy::parser::token::RawID) {
//      ReturnRawId; // UNCOMMENT FOR cout all raw id
//    }
//    else
//      return zzzi;
  }
<Sql>{F_CP1251_SPEC}          { RetOnlyRawId()                        } /* идентификатор с русскими буквами */


  /* Числовые идентификаторы: INTVAL FLOATVAL {{{ */
<Sql,Python>[0-9]+/{NoIntFIN}                          { yylloc_param->step(yyleng); getIntegerToken<0>(yyscanner); RetTok(NUMERIC_ID) }
<Sql,Python>[0-9]+/{RangeFIN}                          { yylloc_param->step(yyleng); getIntegerToken<0>(yyscanner); RetTok(NUMERIC_ID) }
<Sql,Python>[0-9]+[KM]/{IntNumFIN}                     { yylloc_param->step(yyleng); getIntegerToken<1>(yyscanner); RetTok(NUMERIC_ID) }
<Sql,Python>[0-9]+"."[0-9]*/{IntNumFIN}                { yylloc_param->step(yyleng); getFloatToken(yyscanner);      RetTok(NUMERIC_ID) }
<Sql,Python>"."[0-9]+/{IntNumFIN}                      { yylloc_param->step(yyleng); getFloatToken(yyscanner);      RetTok(NUMERIC_ID) }
<Sql,Python>[0-9]+[eE][+-]?[0-9]+/{IntNumFIN}          { yylloc_param->step(yyleng); getFloatToken(yyscanner);      RetTok(NUMERIC_ID) }
<Sql,Python>[0-9]+"."[0-9]*[eE][+-]?[0-9]+/{IntNumFIN} { yylloc_param->step(yyleng); getFloatToken(yyscanner);      RetTok(NUMERIC_ID) }
<Sql,Python>"."[0-9]*[eE][+-]?[0-9]+/{IntNumFIN}       { yylloc_param->step(yyleng); getFloatToken(yyscanner);      RetTok(NUMERIC_ID) }
  /* }}} */


  /*Токены для классификатора динамического SQL */
<Cds>(?i:EXPLAIN){s}(?i:PLAN)/{FIN}               { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:ALTER){s}(?i:SESSION)/{FIN}              { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:ALTER){s}(?i:TRIGGER)/{FIN}              { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:DROP){s}(?i:INDEX)/{FIN}                 { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:TRUNCATE){s}(?i:TABLE)/{FIN}             { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:CREATE){s}(?i:UNIQUE){s}(?i:INDEX)/{FIN} { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:CREATE){s}(?i:INDEX)/{FIN}               { llocBufSimple(yyscanner); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>(?i:BEGIN)/{FIN}                             { yylloc_param->step(yyleng); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }
<Cds>[@]                                          { yylloc_param->step(yyleng); RetTok(cds_BAD_TO_AUTOMATIC_CAST) }




<Cds>'                                          { yylval->intval = (int)(yy::parser::token::RawID); LEX_GET_QUOTED('\'', lex_get_quoted) RetForce(yylval->intval) }
<Cds>["]                                        { yylval->intval = (int)(yy::parser::token::RawID); LEX_GET_QUOTED('\"', lex_get_quoted) RetForce(yylval->intval) }
<Cds>q'.                                        { yylval->intval = (int)(yy::parser::token::RawID); LEX_GET_QUOTED2(yytext[2]); RetForce(yylval->intval) }
<Cds>{F_ID_CHAR_SQDOL}{L_ID_CHAR_SQDOL}*        { ReturnRawId;                } /* Незакавыченный идентификатор: ID */
<Cds>{F_CP1251_SPEC}{L_F_CP1251_SPEC}*          { ReturnRawId;                } /* идентификатор с русскими буквами */
<Cds>[0-9]+/{NoIntFIN}                          { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>[0-9]+/{RangeFIN}                          { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>[0-9]+[KM]/{IntNumFIN}                     { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>[0-9]+"."[0-9]*/{IntNumFIN}                { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>"."[0-9]+/{IntNumFIN}                      { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>[0-9]+[eE][+-]?[0-9]+/{IntNumFIN}          { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>[0-9]+"."[0-9]*[eE][+-]?[0-9]+/{IntNumFIN} { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }
<Cds>"."[0-9]*[eE][+-]?[0-9]+/{IntNumFIN}       { yylloc_param->step(yyleng); RetTok(NUMERIC_ID) }

<Sql>{NULL_CHARACTER_GROUP}                     { yylloc_param->step1(); RetTok(NULL_CHARACTER) }

<Sql>"*/"/"/*"                                  { BEGIN EndOfHint; lex_unput(yytext[1], yyscanner); RetForce(yytext[0]) }
<EndOfHint>"/"                                  { BEGIN Sql; RetForce(yytext[0]) }
<EndOfHint>.                                    { BEGIN Sql; lex_unput(yytext[0], yyscanner); }


<<EOF>> { RetForce(0) }  /* Обработка EOF */

<INITIAL,Sql,Python,Cds>[^\n]   { yylloc_param->step1(); RetForce(yytext[0]) } /* Все одиночные символы не совпадающие с концом строки и не извлеченные правилами выше */


%%

lex_push_state_t lex_push_state = yy_push_state;
lex_pop_state_t  lex_pop_state  = yy_pop_state;
lex_top_state_t  lex_top_state  = yy_top_state;

#include "parserdfa_internal.h"

void lex_unput(int c, yyscan_t yyscanner) {
  yyunput(c, ((struct yyguts_t*)yyscanner)->yytext_ptr, yyscanner);
}


YY_BUFFER_STATE get_current_buffer(yyscan_t yyscanner) {

  struct yyguts_t* yyg = (struct yyguts_t*)(yyscanner);
  return YY_CURRENT_BUFFER;
}

inline int lex_input(yyscan_t yyscanner) { return yyinput(yyscanner); }


inline void getFloatToken(yyscan_t yyscanner) {
  LEXLIB_CONTEXT_INIT(yyscanner)
  yylval->numericValue = new Sm::NumericFloat(*(yylloc), yytext);
}


template <int leng_offset>
inline void getIntegerToken(yyscan_t yyscanner) {
  LEXLIB_CONTEXT_INIT(yyscanner)
  if ((leng_offset + yyleng) < (unsigned int)numeric_limits<int>::digits10)
    yylval->numericValue = new Sm::NumericSimpleInt(*(yylloc), yytext);
  else
    yylval->numericValue = new Sm::NumericInt(*(yylloc), yytext);
}



// vim:foldmethod=marker
COMPILER_LEXER_OPTIMIZATION_POP()
