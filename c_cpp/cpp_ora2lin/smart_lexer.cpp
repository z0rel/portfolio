#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include "resolvers.h"
#include "smart_lexer.h"
#include "model_context.h"
#include "semantic_function.h"
#include "semantic_table.h"

#include "sql_syntaxer_bison.h"
#define  YY_HEADER_EXPORT_START_CONDITIONS
#include "sql_lexer_lex.h"
#include "parserdfa.h"
#include "lexsubtokenizer.h"
#include "semantic_base.h"

PtrYYLex yylex = ora2linLex;
extern Sm::LexSubtokenizer lexerSubtokenizer;

int openFile(const char* name, const char* opt, FILE **file);
extern LexInputData pLexInputData;

YY_BUFFER_STATE get_current_buffer(yyscan_t yyscanner);

using namespace std;

typedef int yy_state_type;


struct MatchPathSeparator {
  bool operator()(char ch) const {
     return ch == OutputCodeFile::pathSeparator();
    // TODO: for windows: return ch == '\\' || ch == '/';
  }
};

std::string removeExtension(const std::string &filename) {
  std::string::const_reverse_iterator pivot = std::find(filename.rbegin(), filename.rend(), '.');
  return pivot == filename.rend()
      ? filename
      : std::string( filename.begin(), pivot.base() - 1 );
}

std::string basename(const std::string &pathname) {
  return std::string(
        std::find_if( pathname.rbegin(), pathname.rend(),
                      MatchPathSeparator() ).base(),
        pathname.end() );
}



struct yyguts_t {

    /* User-defined. Not touched by flex. */
    YY_EXTRA_TYPE yyextra_r;

    /* The rest are the same as the globals declared in the non-reentrant scanner. */
    FILE *yyin_r, *yyout_r;
    size_t yy_buffer_stack_top; /**< index of top of stack. */
    size_t yy_buffer_stack_max; /**< capacity of stack. */
    YY_BUFFER_STATE * yy_buffer_stack; /**< Stack as an array. */
    char yy_hold_char;
    yy_size_t yy_n_chars;
    yy_size_t yyleng_r;
    char *yy_c_buf_p;
    int yy_init;
    int yy_start;
    int yy_did_buffer_switch_on_eof;
    int yy_start_stack_ptr;
    int yy_start_stack_depth;
    int *yy_start_stack;
    yy_state_type yy_last_accepting_state;
    char* yy_last_accepting_cpos;

    int yylineno_r;
    int yy_flex_debug_r;

    yy_state_type *yy_state_buf;
    yy_state_type *yy_state_ptr;
    char *yy_full_match;
    int yy_lp;

    /* These are only needed for trailing context rules,
     * but there's no conditional variable for that yet. */
    int yy_looking_for_trail_begin;
    int yy_full_lp;
    int *yy_full_state;

    char *yytext_r;
    int yy_more_flag;
    int yy_more_len;

    YYSTYPE * yylval_r;

    YYLTYPE * yylloc_r;

    }; /* end struct yyguts_t */


void yyset_leng  (yyscan_t yyscanner, yy_size_t length) {
    struct yyguts_t * yyg = (struct yyguts_t*)yyscanner;
    yyg->yyleng_r = length;
}

extern SyntaxerContext syntaxerContext;
extern lex_push_state_t lex_push_state;
extern lex_top_state_t  lex_top_state;

#include "lexlib_internal.h"

struct GoodFileLine {
  GoodFileLine() : pos(0), line(1) {}
  long   pos;
  size_t line;
};

static size_t getFileLineCount(FILE *f, long realPos, const GoodFileLine &oldGood) {
  long oldPos = ftell(f);
  fseek(f, oldGood.pos, SEEK_SET);
  int line = oldGood.line;
  int c;
  while (ftell(f) != realPos && (c = fgetc(f)) != EOF)
    if (c == '\n')
      ++line;
  fseek(f, oldPos, SEEK_SET);
  return line;
}

void checkLineno(FILE *f, yyguts_t * scanner, yy_buffer_state *lexerBuf, cl::location &loc, cl::location &oldL, GoodFileLine &oldGood) {
  if (!lexerBuf)
    return;
  const long oldPos  = ftell(f);
  long tokenStartPos = /* символов распарсено в буфере */ scanner->yytext_r - lexerBuf->yy_ch_buf;
  long tokenEndPos   = /* символов распарсено в буфере */ tokenStartPos + yyget_leng((yyscan_t)scanner);
  long bufStartPos   = oldPos - scanner->yy_n_chars;
  char *endTokPos    = lexerBuf->yy_ch_buf + tokenEndPos;
  long realPos       = bufStartPos + tokenEndPos - 1;
  size_t realLineno = getFileLineCount(f, realPos, oldGood);
  if (realLineno != loc.end.line && !(realLineno == loc.end.line - 1 && loc.end.column == 1)) {
    fseek(f, realPos, SEEK_SET);
    const char* c = (const char*)(new char[21]);
    for (int i = 0; i < 20; ++i)
      ((char*)c)[i] = fgetc(f);
    ((char*)c)[20] = 0;

    cout << " !!!!!!!! Error: line number in lexer is not matched with real line !!!!!" << endl
         << "          lexer lineno: " << loc.end.line
         << "          real  lineno: " << realLineno << endl
         << "          c = "           << c << endl
         << "          oldLoc = " << oldL
         << endTokPos << endl;
    fseek(f, oldPos, SEEK_SET);
    exit(-1);
  }
  else {
     oldGood.line = realLineno;
     oldGood.pos  = realPos;
  }
}

void checkBytePosition(FILE *f, cl::location &loc, YYSTYPE un, yy::parser::token::yytokentype token) {
  if (loc.begin.bytePosition > loc.end.bytePosition) {
    cout << "error: begin > end" << endl;
    exit(-1);
  }

  if (token != yy::parser::token::yytokentype::RawID)
    return;
  string text = un.id->toString();

  const uint32_t len = loc.end.bytePosition - loc.begin.bytePosition;

  bool needExit = false;

  if (text.find('\'') != string::npos)
    return;
  if (text.length() == len-2 || (text == "'" && len == 4)) // кавычки
    return;

  if (text.length() != len) {
    cout << "error: bad length : " << loc << " token:" << text << " len: " << text.length() << " newLen:" << len << endl;
    needExit = true;
  }

  char *buf = new char[len+10];
  const long oldPos  = ftell(f);

  fseek(f, loc.begin.bytePosition, SEEK_SET);
  fread(buf, 1, len, f);
  char *p = buf;
  buf[len] = 0;
  for (string::iterator it = text.begin(); it != text.end(); ++it, ++p) {
    char itC = *it;
    char pC  = *p;
    if (itC != pC) {
      cout << "error chars nonequally at " << loc << " " << "tokC:" << itC << " readedC:"<< pC << " token:" << text << " readed:" << buf << endl;
      exit(-1);
    }
  }
  if (needExit)
    exit(-1);




  fseek(f, oldPos, SEEK_SET);
  delete[] buf;
}




YY_BUFFER_STATE init_smart_lexer(int new_state, FILE *f, yyscan_t *&scanner, cl::location &loc)
{
  yylex_init_extra(&syntaxerContext, (yyscan_t*)(&scanner));
  yyset_in (f, scanner);
  yyrestart(yyget_in(scanner), scanner);
  yypop_buffer_state(scanner);

  lex_push_state(new_state, scanner);
  lex_push_state(new_state, scanner);
  loc.initialize();

  YY_BUFFER_STATE curBuf = yy_create_buffer(f, 2097152, scanner);
  yy_switch_to_buffer(curBuf, scanner);

  return curBuf;
}

void destroy_smart_lexer(FILE *f, YY_BUFFER_STATE curBuf, yyscan_t *scanner)
{
  yy_delete_buffer(curBuf, scanner);
  if (f)
    fclose(f);
  yylex_destroy(scanner);
}


void test_lexer_speed(const char *filename) {
  yyscan_t *scanner;
  FILE *f = 0;
  if (openFile(filename, "rb", &f))
    return;

  YYSTYPE un;
  cl::location loc = cl::emptyLocation();
  YY_BUFFER_STATE curBuf = init_smart_lexer(Sql, f, scanner, loc);
  yy::parser::token::yytokentype state;

  while ((state = yylex(&un, &loc, scanner))) {;}

  destroy_smart_lexer(f, curBuf, scanner);
}



void check_lexer_locations(const char *filename) {
    yyscan_t *scanner;
    FILE *f = 0;
    if (openFile(filename, "rb", &f))
      return;

    YYSTYPE un;
    cl::location loc = cl::emptyLocation();
    cl::location oldLoc = cl::emptyLocation();
    YY_BUFFER_STATE curBuf = init_smart_lexer(Sql, f, scanner, loc);
    yy::parser::token::yytokentype state;
    GoodFileLine goodFileLine;

    int cnt = 0;

    while ((state = yylex(&un, &loc, scanner))) {
      ++cnt;
      checkLineno(f, (yyguts_t*)scanner, curBuf, loc, oldLoc, goodFileLine);
      checkBytePosition(f, loc, un, state);
      oldLoc = loc;
    }
    loc = oldLoc;

    destroy_smart_lexer(f, curBuf, scanner);
}

enum StateRowcountLine { OWNER, TABLE, VALUE };

void SmartLexer::addGlobalUserException(smart::Ptr<Sm::Id> excName, smart::Ptr<Sm::NumericValue> code) {
  Sm::Exception::globalUserExceptions[Sm::nAssert(excName)->toNormalizedString()] = nAssert(code)->getSIntValue();
}

Sm::Table *getTableDefinition(smart::Ptr<Sm::Id> owner, smart::Ptr<Sm::Id> table) {
  syntaxerContext.model->getFieldRef(owner);
  if (Sm::ResolvedEntity *def = owner->definition()) {
    def->getFieldRef(table);
    if (Sm::ResolvedEntity *tbl = table->definition())
      return tbl->toSelfTable();
  }
  return 0;
}

void SmartLexer::setTableRowcount(smart::Ptr<Sm::Id> owner, smart::Ptr<Sm::Id> table, smart::Ptr<Sm::NumericValue> value) {
  if (Sm::Table *def = getTableDefinition(owner, table))
    def->maxrow(value);
}

void SmartLexer::setTableSize  (smart::Ptr<Sm::Id> owner, smart::Ptr<Sm::Id> table, smart::Ptr<Sm::NumericValue> value) {
  if (Sm::Table *def = getTableDefinition(owner, table))
    def->tablesize(value);
}


void SyntaxerContext::initializeAdditionalConfigData() {
  LexerDFA::LexerDfa *oldTokenizer = lexerSubtokenizer.tokenizer;
  lexerSubtokenizer.setConfigStage();

  parseConfFile(tablesRowcountFileName.c_str(), model->modelActions, model->modelActions.refsToCodegen);
  parseConfFile(tablesSizeFileName    .c_str(), model->modelActions, model->modelActions.refsToCodegen);
  if (!manualEntitiesFileName.empty())
    parseConfFile(manualEntitiesFileName.c_str(), model->modelActions, model->modelActions.refsToCodegen);

  lexerSubtokenizer.tokenizer = oldTokenizer;
}


namespace StateConfFileParsing {
  enum T {
    EMPTY, ROWCOUNT, BYTES
  };
}

void errorAndExit(std::string msg)
{
  cout << msg << endl;
  exit(1);
}


struct ParseException {
  string message;
  ParseException(string msg = "") : message(msg) {}
};

namespace SmartLexer {


string findPyValueByVarname(const string &id) {
  SyntaxerContext::PythonStringVariablesMap::iterator it =
      syntaxerContext.pythonStringVariablesMap.find(id);
  if (it == syntaxerContext.pythonStringVariablesMap.end()) {
    cout << "ERROR: unknown or unhandled python string variable " << id << endl;
    exit(-1);
  }
  return it->second;
}


std::string pyTok2Str(int token) {
  LexerDFA::LexerDfa::Token2KeywordMap::iterator it = lexerSubtokenizer.tokenizer->token2keyword.find(token);
  if (it == lexerSubtokenizer.tokenizer->token2keyword.end()) {
    cout << "bad token " << token << endl;
    exit(-1);
  }
  return it->second;
}

string pyId2Str(smart::Ptr<Sm::Id> id) {
  if (id->quoted() || id->squoted())
    return id->toString();
  else
    return findPyValueByVarname(id->toString());
}

Sm::Id* joinPathList(smart::Ptr<Sm::List<Sm::Id> > pathList) {

  if (pathList->empty())
    throw 999;
  typedef Sm::List<Sm::Id> PathList;
  PathList::iterator it = pathList->begin();
  string path = pyId2Str(*it);
  for (++it; it != pathList->end(); ++it) {
    path.push_back(OutputCodeFile::pathSeparator());
    path.append(pyId2Str(*it));
  }
  return new Sm::Id(path);
}

void clearSources() {
  syntaxerContext.model->modelActions.files.clear();
}

string substitutePath(const string &str) {
  if (str.empty() || str[0] == OutputCodeFile::pathSeparator() || str[0] == '.')
    return str;
  if (syntaxerContext.modelSrcRepositoryPath.empty())
    errorAndExit("Error: Model sources repository path is empty");

  string res = syntaxerContext.modelSrcRepositoryPath;
  res.push_back(OutputCodeFile::pathSeparator());
  res.append(str);
  return res;
}

void appendPathList(PtrIdList pathList) {
  for (Ptr<Sm::Id> &id : *pathList)
    syntaxerContext.model->modelActions.files.push_back(substitutePath(pyId2Str(id)));
}


void setEntityForDependFind(PtrIdEntityList pathList) {
  syntaxerContext.model->modelActions.entitiesForDependFind.clear();
  if (pathList)
    for (auto &id : *pathList)
      syntaxerContext.model->modelActions.entitiesForDependFind.push_back(*id);
}

void setManualEntities(PtrIdEntityList idList) {
  syntaxerContext.model->modelActions.manualEntities.clear();
  if (idList) {
    int i = 0;
    std::vector<Sm::ManualEntity> &ents = syntaxerContext.model->modelActions.manualEntities;
    ents.reserve(idList->size()/2);

    for (auto &id : *idList) {
       if (i % 2 == 0) {
         ents.push_back(Sm::ManualEntity());
         ents.back().reference = *id;
       }
       else {
         ents.back().sqls = *id;
       }
       ++i;
    }
  }
}

void setDumpEntityList(PtrIdEntityList idList) {
  syntaxerContext.dumpEntityList.clear();
  if (idList)
     for (auto &id : *idList)
       syntaxerContext.dumpEntityList.push_back(*id);
}

void setPathList(PtrIdList pathList) {
  clearSources();
  appendPathList(pathList);
}

void insertFrontPathList(PtrIdList pathList) {
  for (Ptr<Sm::Id> &id : *pathList)
    syntaxerContext.model->modelActions.files.insert(
           syntaxerContext.model->modelActions.files.begin(),
           substitutePath(id->toString())
         );
}

void clearExistedEntityQueries() {
  syntaxerContext.model->modelActions.existedEntitiesQueries.clear();
}

void appendExistedEntityQueries(PtrIdList idList) {
  for (Ptr<Sm::Id> &id : *idList)
    syntaxerContext.model->modelActions.existedEntitiesQueries.push_back(id->toString());
}

void setExistedEntityQueries(PtrIdList idList) {
  clearExistedEntityQueries();
  appendExistedEntityQueries(idList);
}

void insertFrontExistedEntityQueries(PtrIdList idList) {
  for (Ptr<Sm::Id> &id : *idList)
    syntaxerContext.model->modelActions.existedEntitiesQueries.insert(
          syntaxerContext.model->modelActions.existedEntitiesQueries.begin(),
          id->toString());
}

Sm::IdEntitySmart* idListToIdEntity(PtrIdList idList) {
  Sm::IdEntitySmart* ent = new Sm::IdEntitySmart;
  for (PtrIdList::dereferenced_type::reverse_iterator it = idList->rbegin(); it != idList->rend(); ++it)
    ent->push_back(*it);
  return ent;
}


void clearReferences() {
  syntaxerContext.model->modelActions.refsToCodegen.clear();
}

void appendReferences(PtrIdEntityList idList) {
  for (auto &id : *idList)
    syntaxerContext.model->modelActions.refsToCodegen.push_back(*id);
}

void setReferences(PtrIdEntityList idList) {
  clearReferences();
  appendReferences(idList);
}

void setOutActors(PtrIdList idList) {
  syntaxerContext.model->modelActions.scenarioActorUsers.clear();
  for (auto &id : *idList)
    syntaxerContext.model->modelActions.scenarioActorUsers.insert(id->toString());

}

void setOutActors(Ptr<Sm::Id> id) {
  syntaxerContext.model->modelActions.scenarioActorUsers.clear();
  syntaxerContext.model->modelActions.scenarioActorUsers.insert(id->toString());
}


void insertFrontReferences(PtrIdEntityList idList) {
  for (auto &id : *idList)
    syntaxerContext.model->modelActions.refsToCodegen.insert(
          syntaxerContext.model->modelActions.refsToCodegen.begin(),
          *id);
}

void clearSkippedErrors() {
   LinterWriter::errcodesToSkip.clear();
}

void addSkippedError(int code) {
  LinterWriter::errcodesToSkip.insert(code);
}

void delSkippedError(int code) {
  LinterWriter::errcodesToSkip.erase(code);
}

StrReader::StrReader(const string &s)
  : textCurr(s.begin()), textEnd(s.end()) {}

void StrReader::operator()(FILE *, char *buf, yy_size_t *result, const yy_size_t max_size) {
  std::string::const_iterator textFinish;
  if (textEnd == textCurr) {
    *result = 0;
    return;
  }
  size_t chars = textEnd - textCurr;
  if (chars > max_size) {
    textFinish = textCurr + max_size;
    *result = max_size;
  }
  else {
    textFinish = textEnd;
    *result = chars;
  }

  std::copy(textCurr, textFinish, buf);
  textCurr = textFinish;
}

FileLexer::FileLexer(const std::string &_filename, SyntaxerContext *_context)
  : lexerState(Sql),
    shared(_filename, _context)
{
  initLexer();
}

FileLexer::FileLexer(const std::string &_filename, SyntaxerContext *_context, int state)
  : lexerState(state),
    shared(_filename, _context)
{
  initLexer();
}


void FileLexer::initLexer() {
  if (openFile(shared.filename.c_str(), "rb", &f)) {
    invalid_ = true;
    return;
  }


  yylex_init_extra(shared.context, &scanner);
  yyset_in (f, scanner);
  yyrestart(yyget_in(scanner), scanner);

  lex_push_state(lexerState, scanner);
  lex_push_state(lexerState, scanner);

  yy_delete_buffer(get_current_buffer(scanner), scanner);
  YY_BUFFER_STATE curBuf = yy_create_buffer(f, 2097152, scanner);
  yy_switch_to_buffer(curBuf, scanner);

  shared.setGlobalCurrentFile();
}

FileLexer::~FileLexer() {
  if (f)
    fclose(f);
  if (valid())
    yylex_destroy(scanner);
}

SharedParsingCtx::SharedParsingCtx(const string &_filename, SyntaxerContext *_context)
  : filename(_filename),
    baseName(new Sm::String(basename(removeExtension(filename)))),
    context(_context)
{
  context->addParsedFilename(filename, baseName);
}

void SharedParsingCtx::setGlobalCurrentFile()  {
  Sm::sAssert(prevIsSet);
  previousGlobalCurrentFile = globalCurrentFile;
  prevIsSet = true;

  globalCurrentFile = baseName.object();
  context->currentFileName = baseName;
}

SharedParsingCtx::~SharedParsingCtx() {
  if (prevIsSet)
    globalCurrentFile = previousGlobalCurrentFile;
}




}

namespace Conf {

#define CFG(tok) cfg_ ## tok

  class ParseConfFile : public yy::parser::token {
    friend void SmartLexer::setParsedFilename(yyscan_t scanner, Sm::Id* filename);

  public:
    typedef std::vector<Sm::IdEntitySmart> OutherReferences;

    FILE *f = 0;

  private:
    YY_BUFFER_STATE  curBuf;
    static yyscan_t *flexScanner;
    yyscan_t        *scanner = 0;
    cl::location     loc;
    YYSTYPE          un;
    yytokentype      state;
    string           filename;

    std::stack<pair<yytokentype, YYSTYPE> > stateStack;

  OutherReferences &outherReferences;
  ModelActions &ctx;

  string sloc() const { return cl::fLoc(loc, &filename).toString(); }

  void setStringParam(std::string &param, std::string msg);
  void extractFilename(std::string j, std::vector<std::string> &container);
  void handleParsedToken();
  void parseImport();

  void skipState(bool predicate);
  yytokentype getNextState();
  yytokentype nextStateNothrow();
  void pushState() { stateStack.push(make_pair(state, un)); }
  void pushStartConfig() { stateStack.push(make_pair(cfg_START_CONFIG_CONSTRUCT, un)); }

  Ptr<Sm::NumericValue> getNumValue();
  bool getBoolValue();
  void setBoolValue(bool &flag) { flag = getBoolValue(); }
  void setModelFlag(unsigned int val);

  string getFilename();
  void setFilename(string &param, const string &base = string());
  void doBisonParsing();

public:

  ParseConfFile(ModelActions &_ctx, OutherReferences &orefs, const string &fname);
  void parseConfFile();

  void parseConfigFileAbsPath(const string &fname);
  void exitAtBadToken(Ptr<Sm::Id> id);


  static yy::parser::token::yytokentype lex(
    cl::semantic_type *yylval_param,
    cl::location      *yylloc_param,
    yyscan_t           yyscanner)
  {
    ParseConfFile *ptr = (ParseConfFile*)yyscanner;
    ptr->nextStateNothrow();
    *yylloc_param = ptr->loc;
    *yylval_param = ptr->un;
    return ptr->state;
  }


  void initConfLexer();
  void destroyConfLexer();
  void initConfBuffer();
  void destroyConfBuffer();


};

yyscan_t *ParseConfFile::flexScanner = 0;



string ParseConfFile::getFilename() {
  if (getNextState() != '=')
    errorAndExit("Error: = expected: " + sloc());

  state = cfg_GET_FILENAME;
  doBisonParsing();

  if (getNextState() != cfg_GET_FILENAME)
    throw 999; // ошибка в реализации взаимодействия обертки лексера с бизоном

  Ptr<Sm::Id> id = un.id;
  return id->toString();
}

void ParseConfFile::setFilename(string &param, const string &base) {
  if (!base.empty()) {
    param = base;
    param.push_back(OutputCodeFile::pathSeparator());
  }
  else
    param.clear();
  param.append(getFilename());
}

void ParseConfFile::setModelFlag(unsigned int val) {
  if (getBoolValue())
    syntaxerContext.model->modelActions.flags |= val;
  else
    syntaxerContext.model->modelActions.flags &= ~val;
}

Ptr<Sm::NumericValue> ParseConfFile::getNumValue() {
  if (getNextState() != '=')
    errorAndExit("= expected " + sloc());

  if (getNextState() != yy::parser::token::NUMERIC_ID)
    errorAndExit("Bad token: NumericValue expected: " + sloc());

  return un.numericValue;
}

bool ParseConfFile::getBoolValue() {
   if (getNextState() != '=')
     errorAndExit("= expected " + sloc());

   getNextState();
   if (state == yy::parser::token::NUMERIC_ID) {
     Ptr<Sm::NumericValue> v = un.numericValue;
     return (v->getSIntValue()) ? true : false;
   }
   else if (state != yy::parser::token::RawID)
     errorAndExit("Bad token: Boolean Value expected: " + sloc());

   Ptr<Sm::Id> id = un.id;
   string text = id->toString();
   if (text == "True")
     return true;
   else if (text != "False")
     errorAndExit("Bad token: Boolean Value expected: " + sloc());
   return false;
}

yy::parser::token::yytokentype ParseConfFile::nextStateNothrow() {
  if (!stateStack.empty()) {
    state = stateStack.top().first;
    un    = stateStack.top().second;
    stateStack.pop();
  }
  else
    state = ora2linLex(&un, &loc, scanner);
  return state;
}

yy::parser::token::yytokentype ParseConfFile::getNextState() {
  if (!stateStack.empty()) {
    state = stateStack.top().first;
    un    = stateStack.top().second;
    stateStack.pop();
  }
  else if (!(state = ora2linLex(&un, &loc, scanner)))
    throw ParseException();
  return state;
}

void ParseConfFile::setStringParam(string &param, string msg) {
  yytokentype currentState = state;
  if (getNextState() != '=')
    errorAndExit(string("= expected after ") + msg + " " + sloc());

  if (getNextState() != yy::parser::token::RawID)
    errorAndExit(string("Raw identificator expected after ") + msg + " = ");

  Ptr<Sm::Id> n = un.id;
  param = n->toNormalizedString();
  syntaxerContext.pythonStringVariablesMap[lexerSubtokenizer.tokenizer->token2keyword[currentState]] = param;
}

void ParseConfFile::doBisonParsing() {
  PtrYYLex oldLex = yylex;
  yylex = ParseConfFile::lex;
  pushState();
  pushStartConfig();

  yy::parser parserItem(&syntaxerContext, (yyscan_t)(this), &filename);
  parserItem.parse();
  pushState();

  yylex = oldLex;
}

void ParseConfFile::parseConfigFileAbsPath(const string &fname) {
  ::parseConfFile(fname.c_str(), ctx, outherReferences, /*createBuffer =*/ true);
}

void ParseConfFile::parseImport() {
  if (getNextState() != yy::parser::token::RawID)
    errorAndExit(string("Raw identificator expected after import: ") + sloc());

  Ptr<Sm::Id> n = un.id;
  string pkgName = n->toString();


  if (getNextState() == '.') {
    if (getNextState() != RawID)
      errorAndExit(std::string("Raw identificator expected after . : ") + sloc());

    Ptr<Sm::Id> fname = un.id;

    string filename = fname->toString();
    pkgName.append(OutputCodeFile::pathSeparator() + filename);
  }
  if (syntaxerContext.converterRepositoryPath.empty())
    errorAndExit(string("ERROR in config: converter_repository_path is empty"));

  parseConfigFileAbsPath(syntaxerContext.converterRepositoryPath + OutputCodeFile::pathSeparator() + pkgName + ".py");
}

void ParseConfFile::extractFilename(string j, std::vector<string> &container) {
  if (getNextState() != yy::parser::token::RawID)
    errorAndExit(std::string("Raw identificator expected after ") + j);
  Ptr<Sm::Id> n = un.id;
  container.push_back(n->toNormalizedString());
}

void ParseConfFile::exitAtBadToken(Ptr<Sm::Id> id)
{
  cout << "Bad token in config file: " << id->toNormalizedString() << ": " << filename << ":" << id->getLLoc();
  exit(-1);
}

void ParseConfFile::initConfLexer() {
  yylex_init_extra(&syntaxerContext, (yyscan_t*)(&(flexScanner)));
  yyset_in (f, flexScanner);
  yyrestart(yyget_in(flexScanner), flexScanner);

  curBuf = yy_create_buffer(f, 2097152, flexScanner);
  yy_switch_to_buffer(curBuf, flexScanner);
  lex_push_state(Python, flexScanner);
  lex_push_state(Python, flexScanner);

  scanner = flexScanner;
}

void ParseConfFile::destroyConfLexer() {
  if (f)
    fclose(f);
  yylex_destroy(scanner);
}

void ParseConfFile::initConfBuffer() {
  scanner = flexScanner;
  curBuf = yy_create_buffer(f, 2097152, scanner);
  yypush_buffer_state(curBuf, scanner);
  scanner = flexScanner;
}

void ParseConfFile::destroyConfBuffer() {
  yypop_buffer_state(scanner);
}


void ParseConfFile::skipState(bool predicate) {
  if (predicate)
    getNextState();
}


void ParseConfFile::handleParsedToken() {
  switch (state) {
    case CFG(SYSDEPS): {
      ctx.oracleSysdepsParsed = true;
      string sysdeps;
      setFilename(sysdeps, syntaxerContext.converterRepositoryPath);
      ctx.oracleSystemSource = sysdeps;
      return;
    }
    case CFG(BASE):
      return extractFilename("BASE", ctx.files);
    case CFG(EXISTED_ENTITY_QUERIES):
      return doBisonParsing(); // ctx.existedEntitiesQueries
    case CFG(TRANSFORM_SENTENCE):
      return setStringParam(syntaxerContext.transformSentence, "INITIALIZERS_FILENAME");
    case CFG(OUT_ACTOR_USER):
    case CFG(REFERENCES):
    case CFG(VIRTUAL_ERRORS):
    case CFG(ENTITY_FOR_DEPEND_FIND):
    case CFG(USERS_EXCEPTIONS_LIST):
    case CFG(TABLES_SIZES_LIST):
    case CFG(TABLES_ROWCOUNT_LIST):
    case CFG(DEBUG_TOKEN_LOCATION):
    case CFG(LINTER_RESERVED_KEYWORDS):
    case CFG(DUMP_ENTITY_LIST):
    case CFG(SOURCES):
    case CFG(MANUAL_ENTITIES):
      doBisonParsing();
      skipState(state == ']');
      return;
    case CFG(SKIP_ERRORS):
      doBisonParsing();
      skipState(state == '}');
      return;
    case CFG(IMPORT):
      return parseImport();
    case CFG(EXPORT_SKIPPED_SYNTAX):
      return setBoolValue(syntaxerContext.exportSkippedSyntax);
    case CFG(EXPORT_SKIPPED_SYNTAX_FILE):
      return setFilename(syntaxerContext.exportSkippedSyntaxFile);
    case CFG(TABLES_ROWCOUNT):
      return setFilename(syntaxerContext.tablesRowcountFileName, syntaxerContext.converterRepositoryPath);
    case CFG(DESCR_ERRORS_ENTITIES_RESOLVE):
      return setBoolValue(syntaxerContext.descrErrorsEntitiesResolve);
    case CFG(DEPENDENCIESSTRUCT_REPR):
      return setFilename(syntaxerContext.dependenciesStructReprFileName);
    case CFG(INITIALIZERS_FILE):
      return setStringParam(syntaxerContext.initializersFilename, "INITIALIZERS_FILENAME");
    case CFG(TEMPORARY_PATH):
      return setStringParam(syntaxerContext.temporaryPath, "TEMPORARY_PATH");
    case CFG(INITIALIZERS_DEPENDENCIES_FILE):
      return setStringParam(syntaxerContext.initializersDependenciesFilename, "INITIALIZERS_DEPENDENCIES_FILE");
    case CFG(TABLES_SIZE):
      return setFilename(syntaxerContext.tablesSizeFileName, syntaxerContext.converterRepositoryPath);
    case CFG(USER_EXCEPTIONS): {
      setFilename(syntaxerContext.userExceptionsFileName, syntaxerContext.converterRepositoryPath);
      return parseConfigFileAbsPath(syntaxerContext.userExceptionsFileName);
    }
    case CFG(OUT_FILE):
      return setStringParam(syntaxerContext.outFileName, "OUT_FILE");
    case CFG(LINTER_USERNAME):
      return setStringParam(syntaxerContext.linterUsername, "LINTER_USERNAME");
    case CFG(LINTER_PASSWORD):
      return setStringParam(syntaxerContext.linterPassword, "LINTER_PASSWORD");
    case CFG(LINTER_NODENAME):
      return setStringParam(syntaxerContext.linterNodename, "LINTER_NODENAME");
    case CFG(DEPENDENCY_ANALYZER_OUTFILE):
      syntaxerContext.dependencyAnalyzerOutFileName = getFilename(); return;
    case CFG(SKIP_CODEGENERATION):
      return setBoolValue(syntaxerContext.skipCodegeneration);
    case CFG(DEPENDENCY_ANALYZER_OUT_DEP_GRAPH):
      return setBoolValue(syntaxerContext.dependencyAnalyzerOutDepGraph);
    case CFG(STOP_LINTER_COMMAND):
      return setStringParam(syntaxerContext.stopLinterCommand, "STOP_LINTER_COMMAND");
    case CFG(TEMPORARY_SPACER_FILE):
      return setStringParam(syntaxerContext.temporarySpacersFile, "TEMPORARY_SPACERS_FILE");
    case CFG(SORTED_ERROR_LOG_BASE):
      return setStringParam(syntaxerContext.sortedErrorLogBase, "SORTED_ERROR_LOG_BASE");
    case CFG(START_LINTER_COMMAND):
      return setStringParam(syntaxerContext.startLinterCommand, "START_LINTER_COMMAND");
    case CFG(ERRORSE_FILE):
      return setFilename(syntaxerContext.errorseFile, syntaxerContext.converterRepositoryPath);
    case CFG(ERRORS_FILE):
      return setFilename(syntaxerContext.errorsFile, syntaxerContext.converterRepositoryPath);
    case CFG(GENERATE_PYTHON_REPR):
      return setFilename(syntaxerContext.generatePythonRepr);
    case CFG(GENERATE_ALL_ENTITIES):
      return setModelFlag(0xFFFFFFFF);
    case CFG(DELETE_FROM_TABLES):
      return setModelFlag(MODEL_ACTIONS_DELETE_FROM);
    case CFG(DROP_TABLES):
      return setModelFlag(MODEL_ACTIONS_DROP_TABLES);
    case CFG(DROP_FOREIGN_KEYS):
      return setModelFlag(MODEL_ACTIONS_DROP_TABLES_FOREIGN_KEYS);
    case CFG(DROP_KEYS):
      return setModelFlag(MODEL_ACTIONS_DROP_KEYS);
    case CFG(DROP_INDICES):
      return setModelFlag(MODEL_ACTIONS_DROP_INDICES);
    case CFG(DROP_TRIGGERS):
      return setModelFlag(MODEL_ACTIONS_DROP_TRIGGERS);
    case CFG(CREATE_SEQUENCES):
      return setModelFlag(MODEL_ACTIONS_CREATE_SEQUENCES);
    case CFG(CREATE_TABLES):
      return setModelFlag(MODEL_ACTIONS_CREATE_TABLES);
    case CFG(CREATE_KEYS):
      return syntaxerContext.model->modelActions.createAllKeys(getBoolValue());
    case CFG(CREATE_PRIMARY_KEYS):
      return syntaxerContext.model->modelActions.createPrimaryKeys(getBoolValue());
    case CFG(CREATE_UNIQUE_KEYS):
      return syntaxerContext.model->modelActions.createUniqueKeys(getBoolValue());
    case CFG(CREATE_FOREIGN_KEYS):
      return syntaxerContext.model->modelActions.createForeignKeys(getBoolValue());
    case CFG(CREATE_PRIMARY_TO_FOREIGN_REFERENCES):
      return syntaxerContext.model->modelActions.createReferences(getBoolValue());
    case CFG(CREATE_CHECKS):
      return setModelFlag(MODEL_ACTIONS_CREATE_TABLE_CHECKS);
    case CFG(CREATE_OTHERS_KEYS):
      return syntaxerContext.model->modelActions.createOtherKeys(getBoolValue());
    case CFG(CODEGEN_ENTITIES_BYTELOCATION):
      return setBoolValue(syntaxerContext.codegenEntitiesBytelocation);
    case CFG(DIFFGEN_ENABLE):
      return setBoolValue(syntaxerContext.diffgenEnable);
    case CFG(CREATE_INITIALIZERS):
      return setBoolValue(syntaxerContext.createInitializers);
    case CFG(CREATE_INITIALIZERS_DEPENDENCIES):
      return setBoolValue(syntaxerContext.createInitializersDependencies);
    case CFG(CHECK_LEXER_LOCATIONS):
      return setBoolValue(syntaxerContext.checkLexerLocations);
    case CFG(CREATE_IN_DB):
      return setBoolValue(syntaxerContext.model->modelActions.directCreateInDB);
    case CFG(CREATE_SYNONYM):
      return setModelFlag(MODEL_ACTIONS_CREATE_SYNONYM);
    case CFG(CREATE_PUBLIC_SYNONYM):
      return setModelFlag(MODEL_ACTIONS_CREATE_PUBLIC_SYNONYM);
    case CFG(CREATE_VIEWS):
      return setModelFlag(MODEL_ACTIONS_CREATE_VIEWS);
    case CFG(CREATE_INDICES):
      return setModelFlag(MODEL_ACTIONS_CREATE_INDICES);
    case CFG(CREATE_TRIGGERS):
      return setModelFlag(MODEL_ACTIONS_CREATE_TRIGGERS);
    case CFG(CREATE_PROC):
      return setModelFlag(MODEL_ACTIONS_CREATE_PROC);
    case CFG(CREATE_USERS):
      return setModelFlag(MODEL_ACTIONS_CREATE_USERS);
    case CFG(CREATE_CODEBLOCK_BRANCH):
      return setModelFlag(MODEL_ACTIONS_CREATE_CODEBLOCK_BRANCH);
    case CFG(CREATE_ALL_MODEL):
      return setModelFlag(MODEL_ACTIONS_CREATE_ALL_MODEL);
    case CFG(CREATE_GLOBAL_VARS):
      return setModelFlag(MODEL_ACTIONS_CREATE_GLOBAL_VARS);
    case CFG(DUMP_DB):
      return setBoolValue(syntaxerContext.dumpDB);
    case CFG(DUMP_SPLIT_FILES):
      return setBoolValue(syntaxerContext.dumpSplitFiles);
    case CFG(DUMP_OUT_FILE):
      return setFilename(syntaxerContext.dumpDbFile);
    case CFG(DUMP_DATE_ONLY):
      return setBoolValue(syntaxerContext.dumpDateOnly);
    case CFG(NEED_TABLE_PROPERTIES):
      return setBoolValue(syntaxerContext.needTableRowProperties);
    case CFG(FILTER_ENTITIES_BY_LINTER_EXISTS): {
      bool val = getBoolValue();
      syntaxerContext.requestEntities = val;
      syntaxerContext.filterEntities = val;
      return;
    }
    case CFG(FULL_RECORDS_REPORT):
      return setBoolValue(syntaxerContext.fullRecordsReport);
    case CFG(GENERATE_FULL_STATISTIC):
      return setBoolValue(syntaxerContext.generateFullStatistic);
    case CFG(NOT_FILTER_ENTITIES_IN_DB):
      return setBoolValue(syntaxerContext.forceNotFilterEntities);
    case CFG(PRINT_EXISTED_ENTITIES):
      return setBoolValue(syntaxerContext.printExistedLinterEntities);
    case CFG(REQUEST_ENTITIES_BY_LINTER_EXISTS):
      return setBoolValue(syntaxerContext.requestEntities);
    case CFG(PRINT_EXISTED_LINTER_PROCEDURES):
      return setBoolValue(syntaxerContext.printExistedProcedures);
    case CFG(PRINT_EXISTED_LINTER_TRIGGERS):
      return setBoolValue(syntaxerContext.printExistedTriggers);
    case CFG(PRINT_EXISTED_LINTER_VIEWS):
      return setBoolValue(syntaxerContext.printExistedViews);
    case CFG(PRINT_EXISTED_LINTER_TABLES):
      return setBoolValue(syntaxerContext.printExistedTables);
    case CFG(PRINT_EXISTED_LINTER_VARIABLES):
      return setBoolValue(syntaxerContext.printExistedVariables);
    case CFG(SUPRESS_UNRESOLVED_PRINTING):
      return setBoolValue(syntaxerContext.model->modelActions.supressUnresolvedPrinting);
    case CFG(EMULATE_ERRORS_IN_CALL_INTERFACE):
      return setBoolValue(syntaxerContext.emulateErrorsInCallInterface);
    case CFG(NO_REPLACE_STATEMENTS):
      syntaxerContext.noReplaceStatements = getBoolValue();
      if (syntaxerContext.noReplaceStatements)
        syntaxerContext.createStatement = "CREATE ";
      else
        syntaxerContext.createStatement = "CREATE OR REPLACE ";
      return;
    case CFG(DATE_TO_CHAR_DEFAULT_LENGTH):
      syntaxerContext.model->modelActions.dateToCharDefaultLength = getNumValue()->getSIntValue();
      return;
    case CFG(TRANSLATE_REFERENCES):
      return setBoolValue(syntaxerContext.translateReferences);
    case CFG(UNWRAP_STRUCTURED_FIELDS):
      return setBoolValue(syntaxerContext.unwrapStructuredFields);
    case CFG(PRINT_CURSOR_VARIABLES):
      return setBoolValue(syntaxerContext.printCursorVariables);
    case CFG(MARK_AS_AUTOGENERATED):
      return syntaxerContext.model->modelActions.setMarkAsAutogenerated(getBoolValue());
    case CFG(CODEGEN_SORT_CMD):
      return setStringParam(syntaxerContext.codegenSortCmd, "SORT_DIRECTORY");
    case CFG(CONVERTER_REPOSITORY_PATH):
      return setStringParam(syntaxerContext.converterRepositoryPath, "CONVERTER_REPOSITORY_PATH");
    case CFG(MODEL_SRC_REPOSITORY_PATH):
      return setStringParam(syntaxerContext.modelSrcRepositoryPath , "MODEL_SRC_REPOSITORY_PATH");
    case CFG(DEPEND_ENTITIES_LINTER):
      return setBoolValue(syntaxerContext.model->modelActions.dependEntitiesLinter);
    case CFG(EXPORT_ENTITIES_LOCATION):
      return setBoolValue(syntaxerContext.exportEntitiesLocation);
    case CFG(MANUAL_ENTITIES_FILENAME):
      syntaxerContext.manualEntitiesFileName = getFilename(); return;
    case CFG(EXPORT_ENTITIES_LOCATION_FILE):
      return setStringParam(syntaxerContext.exportEntitiesLocationFile, "EXPORT_ENTITIES_LOCATION_FILE");
    case yy::parser::token::NUMERIC_ID:
      delete un.numericValue;
      return;
    case yy::parser::token::RawID: {
      Ptr<Sm::Id> id = un.id;
      pushState();
      state = yylex(&un, &loc, scanner);
      if (id->quoted()) {
        getNextState();
        return;
      }
      exitAtBadToken(id);
    } return;
    default:
      throw 999;
      return;
  }
}


ParseConfFile::ParseConfFile(ModelActions &_ctx, ParseConfFile::OutherReferences &orefs, const string &fname)
  : filename(fname),
    outherReferences(orefs),
    ctx(_ctx)
{
  loc.initialize();
}

void ParseConfFile::parseConfFile() {
  try {
    while (nextStateNothrow()) {
      if (state == cfg_ENABLE_SKIP_TOKENS) {
        do {
          nextStateNothrow();
          switch (state) {
            case NUMERIC_ID: { delete un.numericValue; break; }
            case RawID: { delete un.id; break; }
            default: break;
          }
        } while (state && state != cfg_DISABLE_SKIP_TOKENS);
        if (!state || !nextStateNothrow())
          break;
      }
      handleParsedToken();
    }
  }
  catch (ParseException e) {
    cout << "Exception while parsing config file: " << e.message << endl;
    exit(-1);
  }

}


}

namespace SmartLexer {

void setParsedFilename(yyscan_t scanner, Sm::Id* filename) {
  Conf::ParseConfFile *ptr = (Conf::ParseConfFile*)scanner;
  ptr->state = Conf::ParseConfFile::cfg_GET_FILENAME;
  ptr->un.id = filename;
}

}


void parseConfFile(const char *filename, ModelActions &ctx, std::vector<Sm::IdEntitySmart> &outherReferences, bool createBuffer) {
  Conf::ParseConfFile obj(ctx, outherReferences, filename);
  if (openFile(filename, "rb", &obj.f))
    return;

  if (!createBuffer) {
    obj.initConfLexer();
    obj.parseConfFile();
    obj.destroyConfLexer();
  }
  else {
    obj.initConfBuffer();
    obj.parseConfFile();
    obj.destroyConfBuffer();
    fclose(obj.f);
  }
}




void parseFile(SyntaxerContext *context, const char *filename) {
  //  test_lexer_speed(filename);
  //  return;
  if (syntaxerContext.checkLexerLocations) {
    SmartLexer::SharedParsingCtx ctx(filename, context);
    ctx.setGlobalCurrentFile();

    check_lexer_locations(filename);
  }
  else {
    SmartLexer::FileLexer fileLexer(filename, context, Sql);
    yy::parser parserItem(context, fileLexer.scanner, context->currentFileName);
  //  parserItem.set_debug_level(0);
    parserItem.parse();
  }

}




#ifndef YY_END_OF_BUFFER_CHAR

#define YY_END_OF_BUFFER_CHAR 0
#define YY_CURRENT_BUFFER_LVALUE yyg->yy_buffer_stack[yyg->yy_buffer_stack_top]
#define EOB_ACT_CONTINUE_SCAN 0
#define EOB_ACT_END_OF_FILE 1
#define EOB_ACT_LAST_MATCH 2

#endif


SmartLexer::StringLexer::StringLexer()
  : lexerState(Cds), strReader(lexString)
{
  initLexer();
}

SmartLexer::StringLexer::StringLexer(int state)
  : lexerState(state), strReader(lexString)
{
  initLexer();
}

void SmartLexer::StringLexer::initLexer() {
  curBuf       = init_smart_lexer(lexerState, /*file = */0, scanner, loc);
  oldLexBufFun = pLexInputData;
}

SmartLexer::StringLexer::~StringLexer() {
  destroy_smart_lexer(0, curBuf, scanner);
  pLexInputData = oldLexBufFun;
}

yy::parser::token::yytokentype SmartLexer::StringLexer::lex() {
  return yylex(&un, &loc, scanner);
}


void SmartLexer::StringLexer::setLexString(const string &str) {
  lexString = str;
  strReader = SmartLexer::StrReader(lexString);
  pLexInputData = strReader;
}


std::string SmartLexer::fullFilename(const cl::filelocation &floc) {
  return syntaxerContext.parsedFilenames.findFullname(floc.file);
}

std::string SmartLexer::fullFilename(const std::string *file) {
  return syntaxerContext.parsedFilenames.findFullname(file);
}

std::string SmartLexer::textFromFile(const cl::filelocation &floc) {
  string fullname = syntaxerContext.parsedFilenames.findFullname(floc.file);
  if (fullname.empty())
    return "";

  ifstream fs(fullname.c_str(), ios_base::in);
  if (!fs)
    return "";
  fs.seekg(floc.loc.begin.bytePosition, ios_base::beg);

  int readLen = 10;
  if (floc.loc.end.bytePosition > floc.loc.begin.bytePosition)
    readLen = floc.loc.end.bytePosition - floc.loc.begin.bytePosition;

  char *buf = new char [readLen];
  fs.read(buf, readLen);
  string result(buf, readLen);

  delete[] buf;

  return result;
}




// vim:foldmethod=syntax
