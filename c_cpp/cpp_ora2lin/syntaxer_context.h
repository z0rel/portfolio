#ifndef SYNTAXER_CONTEXT_H
#define SYNTAXER_CONTEXT_H

#include <vector>
#include <deque>
#include <stack>
#include "smartptr.h"
#include "lexlib.h"
#include "synlib.h"
#include "sorted_statistic.h"

#define SYNTAXER_CONTEXT_ENTITY_STACK_SIZE 40

namespace Sm {
  class String;
  class SemanticTree;
  class ResolvedEntity;
  class IdEntityChain;
  class Id;
  class VirtualErrors;
  class FunctionContext;
}



class LexerTracer {
public:
  enum State {
    START            ,
    CREATE_USER      ,
    CREATE_PACKAGE   ,
    CREATE_SYNONYM   ,
    CREATE_SEQUENCE  ,
    CREATE_TRIGGER   ,
    CREATE_TYPE      ,
    CREATE_VIEW      ,
    CREATE_FUNCTION  ,
    CREATE_PROCEDURE ,
    CREATE_INDEX     ,
    CREATE_TABLE     ,
    CREATE_DBLINK    ,
    GRANT            ,
    ALTER_USER       ,
    ALTER_TABLE      ,
    CONNECT          ,
    ID1              ,
    DOT_ID           ,
    ID2              ,
    WRAPPED
  };

protected:
  State state;
  vector<State> stackState;

public:
  LexerTracer() : state(START) {}
  void step(State newState = START);
};


struct DownContexts {
  Sm::CathegoryIndexEnum::CathegoryIndex parsedIndex;
};

class ModelContext;

namespace syntaxer_context {


struct ParsedFilenameNode : public smart::Smart {
  Ptr<Sm::String> baseName;
  string          fullName;

  ParsedFilenameNode(const string& full, Ptr<Sm::String> base);

  string findFullname(string *basename);
};

struct ParsedFilenames {
  typedef std::map<string , Ptr<ParsedFilenameNode> > Name2file;
  typedef std::map<const string*, Ptr<ParsedFilenameNode> > Ptr2file;

  Name2file name2file;
  Ptr2file  ptr2file;

  void addFilename(const string &fullname, Ptr<Sm::String> baseName);
  string findFullname(const string *basename);
};


struct SkippedLocation {
  unsigned int     line = 0;
  cl::filelocation floc;

  SkippedLocation() {}
  SkippedLocation(const SkippedLocation &oth)
    : line(oth.line), floc(oth.floc) {}

  SkippedLocation(int _line, cl::location &_floc)
    : line(_line), floc(_floc) {}
};


}



class SyntaxerContext {
public:

  enum Stage {
    START,
    CONFIGURE,
    SYNTAX_ANALYZE,
    CONTEXT_ANALYZE,
    FORMAL_TRANSLATIONS,
    CODEGENERATION,
    GLOBAL_CLEANUP,
    INVALIDATING_CLEANUP
  };

  inline bool isGlobalCleanup() const { return stage == GLOBAL_CLEANUP; }
  inline bool isContextAnalyze() const { return stage == CONTEXT_ANALYZE; }

  unsigned int constructExprBraces = 0;


  bool descrErrorsEntitiesResolve = false;
  Stage stage = START;

  DownContexts downCtxs;

  typedef std::vector<syntaxer_context::SkippedLocation> SkippedLocations;
  SkippedLocations skippedLocations;

  bool exportSkippedSyntax = false;
  string exportSkippedSyntaxFile;


  syntaxer_context::ParsedFilenames parsedFilenames;

  bool generateFullStatistic = false;
  bool fullRecordsReport = false;



  /// Location токена, который нужно отладить на этапе контекстного анализа
  std::vector<int> debugTokenLocation;

  bool exportEntitiesLocation = false;
  std::string exportEntitiesLocationFile;

  std::string temporaryPath;
  std::string temporarySpacersFile = "spacers_log.txt";

  std::string errorseFile = "/opt/linter/linter/dict/errorse.lod";
  std::string errorsFile = "/opt/linter/linter/dict/errors.lod";

  bool needTableRowProperties = false;

  std::string sortedErrorLogBase = "";

  std::string linterUsername = "SYSTEM";
  std::string linterPassword = "MANAGER";
  std::string linterNodename = "    ";

  std::string converterRepositoryPath;
  std::string modelSrcRepositoryPath;

  std::string codegenSortCmd;
  std::string confFileName;
  std::string tablesRowcountFileName;
  std::string tablesSizeFileName;
  std::string userExceptionsFileName;
  std::string outFileName;
  std::string initializersFilename;
  std::string initializersDependenciesFilename;

  std::string dependenciesStructReprFileName;

  std::string manualEntitiesFileName;

  std::string createStatement = "CREATE OR REPLACE ";

  std::string dumpDbFile;
  bool        dumpDB = false;
  bool        dumpSplitFiles = false;
  bool        dumpDateOnly = false;
  std::vector<Sm::IdEntitySmart>  dumpEntityList;

  std::string stopLinterCommand  = "/opt/linter/bin/stop_linter.sh";
  std::string startLinterCommand = "/opt/linter/bin/start_linter.sh";

  bool skipCodegeneration = false;

  std::string dependencyAnalyzerOutFileName;
  bool dependencyAnalyzerOutDepGraph = false;

  bool checkLexerLocations = false; // если = true - вместо парсинга проверять корректность определения размещений лексическим анализатором
  bool diffgenEnable  = false;
  string generatePythonRepr;

  bool codegenEntitiesBytelocation = false;

  bool joinEntities   = true;
  bool filterEntities = false;
  bool forceNotFilterEntities = false;
  bool requestEntities = false;
  bool noReplaceStatements = false;
  bool translateReferences = false;
  bool unwrapStructuredFields = false;

  bool printExistedLinterEntities = false;

  bool printExistedProcedures = false;
  bool printExistedTriggers   = false;
  bool printExistedViews      = false;
  bool printExistedTables     = false;
  bool printExistedVariables  = false;
  bool printCursorVariables     = false;

  bool createInitializers             = false;
  bool createInitializersDependencies = false;

  int  currDotStateNum          = -1;
  int  currDotTransitionFromNum = -1;
  int  currDotTransitionToNum   = -1;

  int  dateToCharDefaultLength = 8;

  bool has_parser_error = false;
  bool listNotInit      = true;

  size_t proceduresForTranslate = 0;
  size_t viewsForTranslate = 0;


  string transformSentence;


  smart::Ptr<ModelContext> containerOfModel;
  ModelContext *model = 0;
  Sm::SemanticTree *globalRootNode = 0;

  bool emulateErrorsInCallInterface = false;
  Sm::VirtualErrors virtualErrors;

  map<int, smart::Ptr<SyntaxerState> > states_descr;

  typedef map<Sm::IdEntityChain*, smart::Ptr<Sm::IdEntityChain> > IdEntityPool;

  // SYNTAXER_CONTEXT_ENTITY_STACK_SIZE

  Sm::IdEntityChain  idEntityPoolUnused[SYNTAXER_CONTEXT_ENTITY_STACK_SIZE];
  Sm::IdEntityChain* pidEntityPoolUnused = idEntityPoolUnused;
  int sizeDeque = SYNTAXER_CONTEXT_ENTITY_STACK_SIZE;


  string *currentFileName = 0;

  /// В ходе парсинга возникли ошибки парсинга. Резолвить что-либо нельзя.
  bool hasSyntaxerContextErrors = false;

  typedef std::map<string, string> PythonStringVariablesMap;
  PythonStringVariablesMap pythonStringVariablesMap;

  LexerTracer tracer;

  std::stack<Sm::ConstructExprStmtContext*> constrExprStmtCtx;
  std::stack<Sm::FunctionContext*> funCtx;

  void pushFunCtx();
  void popFunCtx(Sm::Function* fun);

  Sm::IdEntityChain *getIdChain(Sm::Id *id) {
    // Так сделано в силу того, что парсер однопоточный - сформированная цепочка либо передается куда либо еще, либо освобождается.
    // При корректной грамматике это должно работать.
    // Если будут баги - сделать проверку переполнения
    Sm::IdEntityChain* k = pidEntityPoolUnused++;
    k->clear();
    k->push_back(id);
    return k;
  }

  inline void releaseConverted(Sm::IdEntityChain*) { --pidEntityPoolUnused; }
  inline void release(Sm::IdEntityChain* p) { p->free(); --pidEntityPoolUnused; }

  void initializeAdditionalConfigData();

  SyntaxerContext();
  inline void clrlist() { listNotInit = true; }
  ~SyntaxerContext();


  std::set<std::string> s_reservedFields;


  void addParsedFilename(const string &fullname, Ptr<Sm::String> fname);

};

extern SyntaxerContext syntaxerContext;






#endif
