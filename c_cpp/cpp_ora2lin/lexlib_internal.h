#ifndef LEXLIB_INTERNAL
#define LEXLIB_INTERNAL

#include "project_optimization.h"

COMPILER_LEXER_OPTIMIZATION_PUSH()

#include "lexlib.h"
#include "parserdfa.h"

#ifndef yylloc
  #  define  yylloc yyg->yylloc_r
#endif

#ifndef yyleng
  #  define  yyleng yyg->yyleng_r
#endif

#ifndef yylval
  #  define  yylval yyg->yylval_r
#endif

#define LEXLIB_CONTEXT_INIT(yyscanner) struct yyguts_t *yyg = (struct yyguts_t *)(yyscanner);

#define LEXLIB_SET_PTR_LOC(loc) \
  { \
    LEXLIB_CONTEXT_INIT(yyscanner) \
    loc = yylloc; \
  }

#define LEXLIB_SET_YYLENG(leng) \
  { \
    LEXLIB_CONTEXT_INIT(yyscanner) \
    yyleng = (leng); \
  }


#define LEXLIB_SET_LINES_LENG(lines, leng, b) \
  { \
    cl::location *loc; \
    { \
      LEXLIB_CONTEXT_INIT(yyscanner) \
      loc = yylloc; \
      yyleng = (leng); \
      loc->lines(lines, b); \
    } \
  }


extern LexStringBuffer lexer_id_text;

inline void lex_eof_in_quoted(yyscan_t yyscanner, const char *message) {
  if (lex_top_state(yyscanner) == Cds) {
    LEXLIB_CONTEXT_INIT(yyscanner)
//    Sm::sAssert(yylval->intval != (int)yy::parser::token::RawID);
    yylval->intval = (int)(yy::parser::token::cds_EOF_IN_QUOTED_ID);
  }
  else
    printf(message);
}


inline int lex_input(yyscan_t yyscanner);

inline void lex_loc_char(int c, cl::location *loc) {
  if (c == '\n')
    loc->lines();
  else
    loc->columns();
}


inline void lex_add_char_id_text(int c, cl::location *loc) {
  lex_loc_char(c, loc);
  LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c);
}


template <yyinput_t yyinput>
inline void lex_get_quoted2(yyscan_t yyscanner, int endchar) {
  register int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endchar && c != EOF ) {
      lex_add_char_id_text(c, loc);
    }
    // c in [endc1, EOF]

    if (c == endchar)
    {
      c = yyinput(yyscanner);
      if (c == static_cast<int>('\'')) // задан признак окончания
      {
        loc->columns2();
        escapingChars += 2;
        break;
      }
      else {
        loc->columns2();
        LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, endchar);
        LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c);
      }
    } // c in [EOF]
    else {
      lex_eof_in_quoted(yyscanner, "EOF in special quoted ID\n");
      break;
    }
  }
  // 2 - открывающая кавычка и закрывающая кавычка + q + символ специальной кавычки
  LEXLIB_SET_YYLENG(lexer_id_text.size() + escapingChars + 4)
}



/**
 * @brief lex_get_quoted Извлечение закавыченного идентификатора из потока
 *
 * @param yyscanner
 * @param yyinput       функция для получения токенов из потока
 * @param endc1         первый символ конца закавычивания
 * @param yylval_param  семантическое значение
 * @param yylloc        размешение в исходнике
 *
 */
template <int endc, yyinput_t yyinput>
inline void lex_get_quoted(yyscan_t yyscanner) {
  register int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endc && c != EOF ) {
      lex_add_char_id_text(c, loc);
    }
    // c in [endc1, EOF]

    if (c == endc)
    {
        c = yyinput(yyscanner); // получить символ, следующий за кавычкой
        if (c == endc) // задан признак эскапинга
        {
          loc->columns2();
          ++escapingChars;
          LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, endc);
        }
        else // c in [^endc1] => конец закавыченности
        {
          loc->columns();
          lex_unput(c, yyscanner);  // положить символ, следующий за кавычкой обратно
          break;
        }
    } // c in [EOF]
    else {
      lex_eof_in_quoted(yyscanner, "EOF in quoted ID\n");
      break;
    }
  }
  // 2 - открывающая кавычка и закрывающая кавычка
  LEXLIB_SET_YYLENG(lexer_id_text.size() + escapingChars + 2)
}


template <int endc, yyinput_t yyinput>
inline void lex_get_python_quoted(yyscan_t yyscanner) {
  register int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endc && c != static_cast<int>('\\') && c != EOF ) {
      lex_add_char_id_text(c, loc);
    }
    // c in [endc1, escc, EOF]

    if (c == static_cast<int>('\\')) {
      c = yyinput(yyscanner);
      if (c == EOF) {
        lex_eof_in_quoted(yyscanner, "EOF in quoted ID\n");
        break;
      }
      ++escapingChars;
      loc->columns();
      lex_add_char_id_text(c, loc);
    }
    else if (c == endc) { // конец закавыченности
      loc->columns();
      break;
    } // c in [EOF]
    else {
      lex_eof_in_quoted(yyscanner, "EOF in quoted ID\n");
      break;
    }
  }
  // 2 - открывающая кавычка и закрывающая кавычка
  LEXLIB_SET_YYLENG(lexer_id_text.size() + escapingChars + 2)
}


template <int endc, yyinput_t yyinput>
inline void lex_get_3quoted(yyscan_t yyscanner) {
  register int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endc && c != EOF )
      lex_add_char_id_text(c, loc);

    // c in [endc1, EOF]
    if (c == endc) { // 1 кавычка
      int c2 = yyinput(yyscanner); // получить символ, следующий за кавычкой
      loc->columns();
      if (c2 == endc) { // 2 кавычка
        int c3 = yyinput(yyscanner); // получить символ, следующий за кавычкой
        loc->columns();

        if (c3 == endc)  // 3 кавычка
          break;
        LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c3);
      }
      LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c2);
      LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c);
    }
    else {
      lex_eof_in_quoted(yyscanner, "EOF in quoted ID\n");
      break;
    }
  }
  // 2 - открывающая кавычка и закрывающая кавычка
  LEXLIB_SET_YYLENG(lexer_id_text.size() + escapingChars + 2)
}




/**
 * @brief lex_get_multiline_comment Извлечение многострочного комментария из потока
 *
 * @param comment
 * @param yyscanner
 * @param yyinput
 *
 * @return
 *   Смещение начала последней строки отностиельно начала лексемы
 */
template <yyinput_t yyinput, int prefix_len>
inline void lex_get_multiline_comment(yyscan_t yyscanner) {
  register int c;
  int size = 0;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  for ( ; ; ) {
    while ( (c = yyinput(yyscanner)) != '*' && c != EOF ) {
      lex_loc_char(c, loc);
      ++size;
    }
    // с in ['*', EOF]
    if ( c == '*' )
    {
      loc->columns();
      ++size;
      while ( (c = yyinput(yyscanner)) == '*' ) {
        loc->columns();
        ++size;
      }
      // c in [^'*']

      if ( c == '/' )
      {
        loc->columns();
        ++size;
        break;    /* found the end */
      }
      else {
        lex_loc_char(c, loc);
        ++size;
      }
    }
    else {
      lex_eof_in_quoted(yyscanner, "EOF in quoted ID\n");
      break;
    }
  }
  LEXLIB_SET_YYLENG(prefix_len + size)
}



/*
 * '\n' -> '/' -> '\n' -> return
 *             -> '\r' -> return
 *
 * \n && /  -> /
 * /  && \n -> return
 * /  && \r -> return
 */
template <yyinput_t yyinput>
inline void skipWrappedEntity(yyscan_t yyscanner) {
  int lines = 0;

  int c;
  while ( (c = yyinput(yyscanner)) != EOF ) {
    if (c == '\n') {
      ++lines;
      while ((c = yyinput(yyscanner)) == '\n') {
        ++lines;
      }
      if (c == '/') {
        if ((c = yyinput(yyscanner)) == '\n') {
          lex_unput('\n', yyscanner);
          lex_unput('/', yyscanner);
          break;
        }
        else if (c == '\r') {
          lex_unput('\r', yyscanner);
          lex_unput('/', yyscanner);
          break;
        }
      }
    }
  }

  {
    cl::location *loc;
    LEXLIB_CONTEXT_INIT(yyscanner)
    loc = yylloc;
    yyleng = yyg->yy_c_buf_p - yyg->yytext_ptr;
    loc->lines(lines, yyleng);
  }
}



/// Обновление размещений
template <int begin_offset, int end_offset>
inline void llocBuf(yyscan_t yyscanner)
{
  cl::location *loc;
  char *it;
  char *endIt;
  {
    LEXLIB_CONTEXT_INIT(yyscanner)
    loc   = yylloc;
    it    = yytext + begin_offset;
    endIt = it + yyleng - end_offset;
  }

  loc->step();
  for (; it != endIt; ++it)
    lex_loc_char(*it, loc);
}

inline void llocBufSimple(yyscan_t yyscanner)
{
  cl::location *loc;
  char *it;
  char *endIt;

  {
    LEXLIB_CONTEXT_INIT(yyscanner)
    loc   = yylloc;
    it    = yytext;
    endIt = it + yyleng;
  }
  loc->step();
  for (; it != endIt; ++it)
    lex_loc_char(*it, loc);
}

COMPILER_LEXER_OPTIMIZATION_POP()

#endif // LEXLIB_INTERNAL

