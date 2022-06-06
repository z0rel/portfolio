#ifndef LEXLIB_INTERNAL
#define LEXLIB_INTERNAL

#include <string>

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


inline int lex_input(yyscan_t yyscanner);

inline void lex_loc_char(int c, cl::location *loc) {
  if (c == '\n')
    loc->lines();
  else
    loc->columns();
}


inline void lex_add_char_id_text(std::string& lexer_id_text, int c, cl::location *loc) {
  lex_loc_char(c, loc);
  lexer_id_text.push_back(c);
}


template <typename T, T yyinput>
inline void lex_get_quoted2(std::string& lexer_id_text, yyscan_t yyscanner, int endchar) {
  int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endchar && c != EOF ) {
//      LEX_DFA_CHECK_SYMBOL_TO_SPEC(f, c)
      lex_add_char_id_text(lexer_id_text, c, loc);
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
        lexer_id_text.push_back(endchar);
//        LEX_DFA_CHECK_SYMBOL_TO_SPEC(f, c)
        lexer_id_text.push_back(c);
      }
    } // c in [EOF]
    else {
      printf("EOF in special quoted ID\n");
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
template <int endc>
inline void lex_get_quoted(std::string& lexer_id_text, yyscan_t yyscanner) {
  int c;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  lexer_id_text.clear();

  int escapingChars = 0;

  for ( ; ; )
  {
    while ( (c = yyinput(yyscanner)) != endc && c != EOF ) {
      lex_add_char_id_text(lexer_id_text, c, loc);
    }
    // c in [endc1, EOF]

    if (c == endc)
    {
        c = yyinput(yyscanner); // получить символ, следующий за кавычкой
        if (c == endc) // задан признак эскапинга
        {
          loc->columns2();
          ++escapingChars;
          lexer_id_text.push_back(endc);
        }
        else // c in [^endc1] => конец закавыченности
        {
          loc->columns();
          yyunput(c, ((struct yyguts_t*)yyscanner)->yytext_ptr, yyscanner);  // положить символ, следующий за кавычкой обратно
          break;
        }
    } // c in [EOF]
    else {
      printf("EOF in quoted ID\n");
      break;
    }
  }
  // 2 - открывающая кавычка и закрывающая кавычка
  LEXLIB_SET_YYLENG(lexer_id_text.size() + escapingChars + 2)
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

  loc->step0();
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
  loc->step0();
  for (; it != endIt; ++it)
    lex_loc_char(*it, loc);
}


#endif // LEXLIB_INTERNAL

