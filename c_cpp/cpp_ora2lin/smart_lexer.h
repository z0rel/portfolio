#include <stdio.h>
#include "app_conf.h"

#include "lexlib.h"


#include "yylex_decl.h"

#define  YY_HEADER_EXPORT_START_CONDITIONS
#include "sql_lexer_lex.h"

void check_lexer_locations(const char * filename);
void yyset_leng  (yyscan_t yyscanner, yy_size_t length);

namespace TableSizeType {
  enum T { ROWCOUNT, BYTES };
}

/// Действия, задаваемые из командной строки

namespace Sm {
  class IdEntitySmart;
  class String;
}

void parse_tables_sizes(const char * filename, TableSizeType::T sizeType);
void parse_user_exceptions_codes(const char * filename);
void parseConfFile(const char *filename, ModelActions &ctx, std::vector<Sm::IdEntitySmart> &outherReferences, bool createBuffer = false);

YY_BUFFER_STATE init_smart_lexer(int new_state, FILE *f, yyscan_t *&scanner, cl::location &loc);
void destroy_smart_lexer(FILE *f, YY_BUFFER_STATE curBuf, yyscan_t *scanner);


namespace SmartLexer {

class StrReader {
  std::string::const_iterator textCurr;
  std::string::const_iterator textEnd;
public:
  StrReader(const std::string &s);
  void operator() (FILE */*f*/, char *buf, yy_size_t *result, const yy_size_t max_size);
};


class StringLexer : public smart::Smart {
  LexInputData     oldLexBufFun;

  int lexerState = 0;
  std::string lexString;
  SmartLexer::StrReader strReader;

public:
  yyscan_t        *scanner = 0;
  YY_BUFFER_STATE  curBuf;
  cl::location     loc;
  YYSTYPE          un;

  StringLexer();
  StringLexer(int state);
  ~StringLexer();

  void setLexString(const std::string &str);

  yy::parser::token::yytokentype lex();

private:
  void initLexer();
};

struct SharedParsingCtx {
  std::string            filename;
  smart::Ptr<Sm::String> baseName;
  SyntaxerContext       *context;
  std::string           *previousGlobalCurrentFile = 0;
  bool                   prevIsSet = false;

  SharedParsingCtx(const std::string &_filename, SyntaxerContext *_context);

  void setGlobalCurrentFile();

  ~SharedParsingCtx();
};

class FileLexer {
protected:
  int lexerState = 0;

  FILE* f = 0;

  bool invalid_ = false;

public:
  yyscan_t         scanner = 0;
  YY_BUFFER_STATE  curBuf;
  cl::location     loc;
  YYSTYPE          un;

  SharedParsingCtx shared;

  bool invalid() const { return invalid_; }
  bool valid() const   { return !invalid_; }

  FileLexer(const std::string &_filename, SyntaxerContext *_context);
  FileLexer(const std::string &_filename, SyntaxerContext *_context, int state);

  ~FileLexer();


private:
  void initLexer();
};






char pathSeparator();
Sm::Id* joinPathList(smart::Ptr<Sm::List<Sm::Id> > pathList);
std::string pyId2Str(smart::Ptr<Sm::Id> id);
std::string pyTok2Str(int token);
void clearSources();

typedef smart::Ptr<Sm::List<Sm::Id> > PtrIdList;
typedef smart::Ptr<Sm::List<Sm::IdEntitySmart> > PtrIdEntityList;

void setEntityForDependFind(PtrIdEntityList pathList);

void setPathList(PtrIdList pathList);
void appendPathList(PtrIdList pathList);
void insertFrontPathList(PtrIdList pathList);

void setExistedEntityQueries(PtrIdList idList);
void appendExistedEntityQueries(PtrIdList idList);
void insertFrontExistedEntityQueries(PtrIdList idList);
void clearExistedEntityQueries();

void setManualEntities(PtrIdEntityList idList);

void setDumpEntityList(PtrIdEntityList idList);

void setReferences(PtrIdEntityList idList);
void setOutActors(PtrIdList idList);
void setOutActors(Ptr<Sm::Id> id);
void appendReferences(PtrIdEntityList idList);
void insertFrontReferences(PtrIdEntityList idList);
void clearReferences();

void clearSkippedErrors();
void addSkippedError(int code);
void delSkippedError(int code);

void setParsedFilename(yyscan_t scanner, Sm::Id *filename);

void addGlobalUserException(smart::Ptr<Sm::Id> excName, smart::Ptr<Sm::NumericValue> code);
void setTableRowcount(smart::Ptr<Sm::Id> owner, smart::Ptr<Sm::Id> table, smart::Ptr<Sm::NumericValue> value);
void setTableSize  (smart::Ptr<Sm::Id> owner, smart::Ptr<Sm::Id> table, smart::Ptr<Sm::NumericValue> value);

Sm::IdEntitySmart * idListToIdEntity(PtrIdList);

std::string textFromFile(const cl::filelocation &floc);
std::string fullFilename(const cl::filelocation &floc);
std::string fullFilename(const std::string *file);
//ctx.existedEntitiesQueries
}


