#ifndef PARSERDFA_INTERNAL
#define PARSERDFA_INTERNAL

#include "parserdfa.h"
#include "lexsubtokenizer.h"
#include "semantic_id.h"
#include "lexlib_internal.h"

extern LexStringBuffer lexer_id_text;


#define LEX_DFA_ADD_CHAR(f, c) LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c);


inline LexerDFA::action_t findInLexerDfa(LexerDFA::LexerDfa *l, yyscan_t yyscanner, int &idFlags) {
  using namespace std;
  using namespace LexerDFA;

  cl::location *loc;
  LEXLIB_SET_PTR_LOC(loc)

  int c  = *(unsigned char *)(((struct yyguts_t*)yyscanner)->yytext_ptr);	/* cast for 8-bit char's */
  // LEX_DFA_CHECK_SYMBOL_TO_SPEC(idFlags, c);

  lexer_id_text.clear();
  LEX_STRINGBUFFER_ADD_CHAR(lexer_id_text, c);

  const vector<LexerDfa::UcharTable> &mappedCharTable = l->mappedCharTable;
  const vector<vector<vector<LexerDfa::State> > > &transitionCube = l->transitionCube;

  unsigned int level      = 0;
  BranchNum    currBranch = 0;
  for (;; ++level) {
    unsigned int mapIdx;
    {
      register int c1 = LexerDFA::LexerDfa::acceptIdTable.alphaUpper[c];
      if ((mapIdx = mappedCharTable[level][c1]) == UCHAR_MAX) {
        for (c = lex_input(yyscanner); LexerDFA::LexerDfa::acceptIdTable.state[c]; c = lex_input(yyscanner))
          LEX_DFA_ADD_CHAR(idFlags, c);
        lex_unput(c, yyscanner);
        break;
      }
    }

    const LexerDfa::State *state;
    {
      const LexerDfa::TransitionCube::value_type::value_type &cubeItem = transitionCube[level][currBranch];
      if (cubeItem.size() <= mapIdx) {
        for (c = lex_input(yyscanner); LexerDFA::LexerDfa::acceptIdTable.state[c]; c = lex_input(yyscanner))
          LEX_DFA_ADD_CHAR(idFlags, c);
        lex_unput(c, yyscanner);
        break;
      }
      state = &(cubeItem[mapIdx]);
    }

    c = lex_input(yyscanner);

    // следующий символ - недопустимый для продолжения идентификатора
    if (!LexerDFA::LexerDfa::acceptIdTable.state[c]) {
      lex_unput(c, yyscanner);
      if (state->flags & FINALLY) { // проверка на найденность токена
        struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
        loc->step(yyg->yy_c_buf_p - yyg->yytext_ptr);
        idFlags |= FLAG_ID_IS_KEYWORD_TOKEN;
        return l->finalStates[level][currBranch][mapIdx];
      }
      break;
    }

    if ((currBranch = state->nextBranch) == UINT_MAX) {
      LEX_DFA_ADD_CHAR(idFlags, c);
      for (c = lex_input(yyscanner); LexerDFA::LexerDfa::acceptIdTable.state[c]; c = lex_input(yyscanner))
        LEX_DFA_ADD_CHAR(idFlags, c);
      lex_unput(c, yyscanner);
      break;
    }
    LEX_DFA_ADD_CHAR(idFlags, c);
  }
  struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
  loc->step(yyg->yy_c_buf_p - yyg->yytext_ptr);

  return 0;
}


inline void lex_get_rawId(yyscan_t yyscanner) {
  int f = 0;
  int c  = *(unsigned char *)(((struct yyguts_t*)yyscanner)->yytext_ptr);	/* cast for 8-bit char's */
//  LEX_DFA_CHECK_SYMBOL_TO_SPEC(f, c);

  lexer_id_text.clear();
  LEX_DFA_ADD_CHAR(f, c);
  for (c = lex_input(yyscanner); LexerDFA::LexerDfa::acceptIdTable.state[c]; c = lex_input(yyscanner))
    LEX_DFA_ADD_CHAR(f, c);
  lex_unput(c, yyscanner);

  {
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yylloc->step(yyg->yy_c_buf_p - yyg->yytext_ptr);
    yylval->id = new Sm::Identificator(yylloc, f, 1);
  }
}




namespace Sm {



  void LexSubtokenizer::updTableState()
  {
    if (!braceDeep)
      switch (state) {
        case IN_CREATE_TABLE:
          state = IN_CREATE_TABLE_PROPERTIES;
          break;
        default:
          break;
      }
  }

  inline int Sm::LexSubtokenizer::findInDfa(yyscan_t yyscanner) {
  idFlags = 0;
  int v = findInLexerDfa(tokenizer, yyscanner, idFlags);
  switch (v) {
    case PROFILE:
      switch (state) {
        case IN_CREATE_USER:
        case IN_ALTER_USER:
          return PROFILE;
        default:
          return 0;
      }
      return 0;

    case OPAQUE:
      switch (state) {
        case IN_CREATE_TABLE_PROPERTIES:
          return OPAQUE;
        default:
          return 0;
      }

    case DEFERRED:
      switch (state) {
        case IN_CREATE_TABLE_PROPERTIES:
          return DEFERRED;
        default:
          return 0;
      }

    case SEGMENT:
      switch (state) {
        case IN_CREATE_TABLE:
        case IN_CREATE_TABLE_PROPERTIES:
        case IN_CREATE_INDEX:
          updTableState();
          return SEGMENT;
        default:
          return 0;
      }

    case CREATION:
      switch (state) {
        case IN_CREATE_TABLE:
        case IN_CREATE_TABLE_PROPERTIES:
        case IN_CREATE_INDEX:
          return CREATION;
        default:
          return 0;
      }

    case LOGGING:
      switch (state) {
        case IN_CREATE_TABLE:
        case IN_CREATE_TABLE_PROPERTIES:
        case IN_CREATE_INDEX:
          return LOGGING;
        default:
          return 0;
      }
      return 0;

    case NOLOGGING:
      switch (state) {
        case IN_CREATE_TABLE:
        case IN_CREATE_TABLE_PROPERTIES:
        case IN_CREATE_INDEX:
          return NOLOGGING;
        default:
          return 0;
      }
      return 0;


    case UNPIVOT:
      state = AFTER_UNPIVOT;
      break;
    case BEGINk:
      ++blockCount;
      ++declarationDeep;
      break;
    case END:
      if (declarationDeep)
        --declarationDeep;
      break;

    case INCLUDE:
    case EXCLUDE:
    case NULLS:
      if (state != AFTER_UNPIVOT)
        return 0;
      break;
    case OBJECT:
      if ((state == IN_CREATE_TYPE && !braceDeep) || state == IN_CREATE_TABLE_PROPERTIES)
        return OBJECT;
      return 0;
    case LOB:
    case TABLESPACE:
    case PCTUSED:
      updTableState();
      break;

    case CACHE:
      switch (state) {
        case IN_CREATE_TABLE_PROPERTIES:
        case IN_ALTER_TABLE:
        case IN_CREATE_SEQUENCE:
          return (int)CACHE;
        default:
          return 0;
      }
      return 0;
    default:
      return v;
  }
  return v;
}



inline int Sm::LexSubtokenizer::getSubtoken(yyscan_t yyscanner) {
  int v = findInDfa(yyscanner);
  switch (v) {
    case 0: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      yylval->id = new Sm::Identificator(yylloc, idFlags, 1);
      return (int)yy::parser::token::RawID;
    }
    case yy::parser::token::AND: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      yylval->andOr = Sm::logical_compound::AND;
      return yy::parser::token::AND;
    }
    case yy::parser::token::FUNCTION: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      BEGIN SId1;
      return yy::parser::token::FUNCTION;
    }
    case yy::parser::token::PROCEDURE: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      BEGIN SId1;
      return yy::parser::token::PROCEDURE;
    }
    case yy::parser::token::IN: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      sComparsion(IN)
      return yy::parser::token::IN;
    }
    case yy::parser::token::OR: {
      struct yyguts_t* yyg = (struct yyguts_t*)yyscanner;
      yylval->andOr = Sm::logical_compound::OR;
      return yy::parser::token::OR;
    }
    default:
      return (yy::parser::token::yytokentype)v;
  }
}

inline void Sm::LexSubtokenizer::pushCreateTrigger() {}

inline void Sm::LexSubtokenizer::pushCreateView() {}

inline void Sm::LexSubtokenizer::pushCreatePackage() { ++declarationDeep; }

inline void Sm::LexSubtokenizer::pushCreatePackageBody() { ++declarationDeep; }

inline void Sm::LexSubtokenizer::pushCreateTypeBody() { ++declarationDeep; }

inline void Sm::LexSubtokenizer::pushCreateProcedure() { ++declarationDeep; }

inline void Sm::LexSubtokenizer::pushCreateFunction() { ++declarationDeep; }

inline void Sm::LexSubtokenizer::pushCreateType() { ++declarationDeep; state = IN_CREATE_TYPE; }

inline void Sm::LexSubtokenizer::pushCreateUser() { state = IN_CREATE_USER; }

inline void Sm::LexSubtokenizer::pushCreateSequence() { state = IN_CREATE_SEQUENCE; }

inline void Sm::LexSubtokenizer::pushCreateTable() {
  state = IN_CREATE_TABLE;
}

inline void Sm::LexSubtokenizer::pushCreateIndex() { state = IN_CREATE_INDEX; }

inline void Sm::LexSubtokenizer::pushAlterTable() { state = IN_ALTER_TABLE; }

inline void Sm::LexSubtokenizer::pushAlterUser() { state = IN_ALTER_USER; }

inline void Sm::LexSubtokenizer::popStateOnSemicolon() { state = STATE_EMPTY; }

}


#endif // PARSERDFA_INTERNAL

