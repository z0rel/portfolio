#ifndef LEXSUBTOKENIZER_H
#define LEXSUBTOKENIZER_H

#include "project_optimization.h"
COMPILER_LEXER_OPTIMIZATION_PUSH()

#include <set>
#include "parserdfa.h"
#include "syntaxer_external.h"
#include "lexlib.h"
#include "sql_syntaxer_bison.h"

namespace Sm {

class  LexSubtokenizer : public yy::parser::token {
public:
  typedef std::map<std::string, int> TokensTable;
  enum State {
    STATE_EMPTY,
    IN_CREATE_USER,
    IN_ALTER_USER,
    IN_CREATE_TABLE,
    IN_CREATE_TABLE_PROPERTIES,
    IN_CREATE_INDEX,

    AFTER_UNPIVOT,


    IN_ALTER_TABLE,
    IN_CREATE_SEQUENCE,
    IN_CREATE_TYPE
  };

private:
  static TokensTable getOraTokensTable();
  static TokensTable getConfigTokensTable();

public:
  LexerDFA::LexerDfa oraKeywordsTokenizer    = getOraTokensTable();
  LexerDFA::LexerDfa configKeywordsTokenizer = getConfigTokensTable();

  LexerDFA::LexerDfa *tokenizer              = &oraKeywordsTokenizer;

private:
  State state;
  int idFlags;

public:
  inline void popStateOnSemicolon();
  inline void pushCreateTrigger();
  inline void pushCreateView();
  inline void pushCreatePackage();
  inline void pushCreatePackageBody();
  inline void pushCreateTypeBody();
  inline void pushCreateProcedure();
  inline void pushCreateFunction();
  inline void pushCreateType();
  inline void pushCreateUser();
  inline void pushCreateSequence();
  inline void pushCreateTable();
  inline void pushCreateIndex();
  inline void pushAlterTable();
  inline void pushAlterUser();

  size_t callCounter = 0;
  int    procDeep = 0;
  int    braceDeep = 0;

  int    blockCount = 0;
  int    declarationDeep = 0;

  inline void setConfigStage() { tokenizer = &configKeywordsTokenizer; }
  inline void setOracleStage() { tokenizer = &oraKeywordsTokenizer; }

  inline LexSubtokenizer() {}

  inline bool isOraParserStage() const { return tokenizer == &oraKeywordsTokenizer; }
  inline int  findInDfa(yyscan_t yyscanner);

  inline int  getSubtoken(yyscan_t yyscanner);
  void updTableState();
};


}

COMPILER_LEXER_OPTIMIZATION_POP()

#endif // LEXSUBTOKENIZER_H
