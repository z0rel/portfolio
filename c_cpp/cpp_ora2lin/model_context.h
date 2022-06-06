#ifndef MODEL_CONTEXT_H
#define MODEL_CONTEXT_H

#include "unordered_set"
#include "usercontext.h"
#include "codestream.h"
#include "model_head.h"
#include "system_datatype.h"
#include "system_functions.h"
#include <anydata.h>
#include "syntaxer_context.h"
#include "semantic_flags.h"
#include "xmltype.h"
#include "blobtype.h"
#include "depworkflow.h"

class SyntaxerContext;
class SysUser;
using namespace smart;

namespace Sm {
  class RowIdExpr;
  class SpecialStoreKeys;


struct DelayedConstraints {

  size_t id = getDelayedConstrId();

  Ptr<Sm::table::FieldDefinition>  field;
  Sm::Table                       *table = 0;
  Ptr<Sm::List<Sm::Constraint> >   constraints;

  static size_t getDelayedConstrId();

  DelayedConstraints(
      Sm::table::FieldDefinition *f,
      Sm::Table                  *t,
      Sm::List<Sm::Constraint>   *cL);
};

}



inline float diffTime(clock_t beginTime) { return ((float)(clock() - beginTime))/CLOCKS_PER_SEC; }


/// Модель БД
class  ModelContext : public Sm::ResolvedEntitySNode {
  size_t actionCounter;
public:
  SyntaxerContext *parent;

private:
  typedef map<string, Ptr<Sm::Id> >        GlobalLiterals;
  typedef map<string, Ptr<Sm::Exception> > GlobalExceptions;

  typedef std::vector<Ptr<Sm::BooleanLiteral> > BooleanLiterals;

  GlobalLiterals          globalLiterals;
  BooleanLiterals         booleanLiterals;
  GlobalExceptions        globalExceptions;
  list<Sm::SemanticTree*> rootParent;
  unsigned int            systemNamespacePosition;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  std::vector<Ptr<Sm::Type::collection_methods::AccessToItem> > resolvedAccessOperators;
  std::vector<Ptr<Sm::StatementInterface> > delayDeletedStatements;
  std::vector<Ptr<Sm::PlExpr> >             delayDeletedExpressions;
  std::vector<Ptr<Sm::Id> >                 delayDeletedIds;
  std::vector<Ptr<Sm::ResolvedEntity> >     delayDeletedEntities;
  Ptr<Sm::LevelResolvedNamespace>           indicesNamespace;
  std::set<Sm::Id*> partitiallyResolvedNodes; // узлы, которые были частично разрешены. Нужно для фильтрации редукции неотрезолвленных ссылок на поля ненайденных сущностей
  Ptr<Sm::SpecialStoreKeys> specialStoreKeysAction; // для сортировки

  //             user      content
  typedef pair<string, Ptr<UserContext> > UserDataMapPair;
  typedef Sm::UserDataMap UserDataMap;
  typedef vector<pair<UserContext*, Ptr<Sm::AlterTable> > > AlterCommands;

  /// Хэш-таблица пользовательских контекстов, ключ - имя пользователя
  UserDataMap           userMap;
  /// Хэш-таблица общедоступных синонимов
  UserContext::Synonyms publicSynonyms;
  /// Активный пользовательский контекст
  UserContext          *udata;
  /// Управляющие команды (полученные при запуске программы)
  ModelActions          modelActions;

  /// Alter-комманды, которые нужно выполнить после того, как построена модель
  /// но до построения SemanticTree
  AlterCommands                      alterCommands;
  Sm::GlobalDatatype::GlobalDatatype globalDatatypes;
  /// Глобальное пространство уникальных сущностей
  Sm::VEntities::OverloadedFunctions globalUniqueEntitiesSpace;
  Sm::SemanticTree                  *rootGlobalTree;
  /// списки запросов, определенные в несвязных кусках модели, которые нужно кастить как если бы они
  /// открывали одну и ту же курсорную переменную
  Sm::GlobalDynSqlCursors            globalDynSqlCursors;

protected:
  Ptr<SysUser>     sysuser_;
  Ptr<UserContext> systemUser_;
public:
  bool                 codegenerationStarted = false;
  Sm::GlobalFunctions *globalFunctions;

  mutable map<Sm::ResolvedEntity*, int> sysusers;
  std::string scenario_actor_user;
  bool diffMode;

  Sm::DependEntitiesMap dependenciesGraph;

  Sm::Anydata anydataInterface;
  Sm::XmltypeIntf xmlInterface;
  Sm::BlobtypeIntf blobInterface;

  typedef Sm::CathegoriesOfDefinitions::ScopedEntities ScopedEntities;


  Sm::LinterCreatedEntities linterCreatedEntities;
  std::set<Sm::ResolvedEntity *, Sm::LE_ResolvedEntities> linterCreatedResolvedEntities;

  typedef std::map<std::string, std::pair<Ptr<Sm::Id>, Ptr<Sm::Id> > > LexerDefines;
  LexerDefines lexerDefines;

  vector<Ptr<Sm::Table> > granteeGlobalTemporaryObjectTables;


  std::string defLhs;
  std::vector<Ptr<ResolvedEntity> > actionsContainer;

  std::vector<Sm::DelayedConstraints> delayedConstraints;

  Ptr<UserContext> getSystemUser() const;

  ModelContext(SyntaxerContext *par);

  unsigned int getNextSystemNamespacePos() { return --systemNamespacePosition; }
  void connect( Ptr<Sm::Id> name ); // activate specified user
  UserContext *addUser(Ptr<Sm::Id> name, Sm::Id *password = 0);
  UserContext *getUser(Ptr<Sm::Id> name); // create if not found
  UserContext *findUser(Ptr<Sm::Id> name);

  void configure(int argc, char **argv);
  void syntaxAnalyze();
  void contextAnalyze();
  void formalTranslations();
  void formalPrepareModel();
  void calculateStatistics(DependenciesStruct::SortedEntities *sortedEntities);
  void translateCursorFieldDecltype();
  void codegeneration();
  void dumpDB();


  void resolveCreatedObjects();
  void resolveDynamicOperators();

  void printRecordsUsing();

  Ptr<SysUser> sysuser() const;
  UserContext *getUContextWithAdd(const Sm::Id2 *name);
  UserContext *getUContextOrNull(const Sm::Id2 *name);
  Ptr<UserContext> getUContext(const Sm::Id2 *name);

  void addTable(Ptr<Sm::Table> tbl);
  void addObjectTypeBody(Ptr<Sm::Type::Object> body);
  void generateReportSystemEntitiesUsing(Sm::Codestream &str);
  void findTablesWithObjectFields();

  void storeSQL(LinterWriter &linterWriter, Sm::DependEntitiesMap &filter);
  size_t nextAct() { return actionCounter++; }
  SyntaxerContext *parserContext() { return parent; }

  bool getFieldRef(Ptr<Sm::Id> &reference);
  bool findInRootNamespace(Ptr<Sm::Id> &reference);

  bool isDefinition() const { return true; }
  ScopedEntities        ddlCathegory() const { return ModelContext_; }
  Ptr<Sm::Datatype>     getDatatype () const;
  Sm::SemanticTree *toSTreeBase() const;

  void replaceChildsIf(Sm::ExprTr tr);
  void traverseDeclarations(Sm::DeclActor tr);
  void replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond);
  void traverseModelStatements(Sm::StatementActor &fun);

  void storeUserTablesKeys(LinterWriter &linterWriter);

  void printTablesThatWillChanged            (Sm::Codestream &str);
  void printTablesWithCleanNumberFields      (Sm::Codestream &str);
  void printTablesFieldsWithCleanNumberFields(Sm::Codestream &str);
  void printTablesFieldsWithBigNumberFields  (Sm::Codestream &str);
  void printNumberPrecStatistic();
  void addWrapped(ResolvedEntity::ScopedEntities cat, Ptr<Sm::Id2> ent);

  void printBlobInserts();

  void addLexerDefines(Ptr<Sm::Id> def);
  Sm::Id* lexerDef (string &&s) const;
  Sm::Id* lexerQDef(string &&s) const;
  void delLexerDefines();


  void setDefLhs(std::string s) { defLhs = s; transform(defLhs.begin(), defLhs.end(), defLhs.begin(), ::toupper); }


  void outWrappedEntities();
  void queryCreatedObjects();
  void addExistedInLinterDBEntity(std::string owner, std::string table, std::string object_type);
  void userInitializer(Sm::Codestream &str);
  void addSystemDatatypesNodes(Sm::SemanticTree *globalTree) const;

  std::vector<Sm::UniqueEntitiesMap*> externalVariablesStack;
  ResolvedEntity *inExternalVariablesStack(ResolvedEntity* d) const;


  void alterTable(Ptr<Sm::AlterTable> cmd);
  void parseAlterTableCommands();
  void alterUser (Sm::AlterUser * cmd);
  void addSynonym(Sm::Synonym   * cmd);
  void addPackage(Sm::Package   * cmd);

  bool entityAlreadyCreatedInDb(Sm::ResolvedEntity *ent);


  void collectInitializers(Sm::UserEntitiesMap &container);

  virtual ~ModelContext();

  struct RecordCounters {
    unsigned int collectionFieldCount                  = 0;
    unsigned int variablesThatContainsStructuredFields = 0;
    unsigned int recordsThatContainsStructuredFields   = 0;
    unsigned int fieldRefInRecords        = 0;
    unsigned int fieldRefInCollections    = 0;
    unsigned int fieldRefInObjects        = 0;
    unsigned int fieldRefInTables         = 0;
    unsigned int collectionByRecords      = 0;
    unsigned int collectionVarByRecords   = 0;
    unsigned int simpleRecordVariables    = 0;


    struct CursorCounters {
      unsigned int from = 0;
      unsigned int into = 0;
      unsigned int insertingValue = 0;
      void print(Sm::Codestream &str, string prefix);
      void count(unsigned int f);
    };
    CursorCounters recordTypes;
    CursorCounters recordRefs;
    CursorCounters collectionTypes;
    CursorCounters collectionRefs;
  };


  struct ZeroUnsignedInt {
    unsigned int v = 0;
  };

  typedef std::map<Sm::BlockPlSql*, std::map<Sm::StatementInterface*, ZeroUnsignedInt>, Sm::LE_ResolvedEntities> RecStmtsUsing;
  typedef map<Sm::ResolvedEntity*,  RecStmtsUsing, Sm::LE_ResolvedEntities> RecVarUsing;
  typedef std::map<Sm::Type::Record*, RecVarUsing, Sm::LE_ResolvedEntities> Rec;
  typedef map<ResolvedEntity*, Rec, Sm::LE_ResolvedEntities> Collections;
  typedef map<Sm::Type::Record*, std::map<Sm::Type::CollectionType*, RecVarUsing, Sm::LE_ResolvedEntities>, Sm::LE_ResolvedEntities> CollectionsByRecords;

  typedef std::set<ResolvedEntity*, Sm::LE_ResolvedEntities> ResolvedEntitySet;
  void printRecordsUsingAnalysisResult(Sm::Codestream     &str,
                                       RecordCounters     &counters,
                                       Rec                &records,
                                       Sm::ResolvedEntity *collectionVariable);
  void printRecVarUsing(Sm::Codestream    &str,
                        Sm::Type::Record  *recordDef,
                        RecVarUsing       &records,
                        RecordCounters    &counters);
  Sm::Codestream& printRecEntityName(Sm::Codestream& str, const std::string &prefix, const Ptr<Sm::Id> &name);
  void printTablesAndViewsCounters();
  void setPackagesAttributesOnBlocks();
  void modelBsizes();

  Sm::Table *getTableRef(Ptr<Sm::Id2> &table);
  UserContext* getUContextWithoutAddEmpty(const Sm::Id2 *name);
  UserContext* parsingStageUserContext(const Sm::Id2 *name);

  void extractLocFiles();
};

int openFile(const char *name, const char *opt, FILE **file);

Sm::GlobalFunctions* modelFuns();

namespace sentence_transformer {
  void transformStatement(Ptr<Sm::StatementInterface> stmt, Sm::List<Sm::RefAbstract> *intoList, bool semi);
  void transformExpression(Ptr<Sm::PlExpr> stmt, Sm::List<Sm::RefAbstract> *intoList, bool semi);
  void transformStmtList(Ptr<Sm::BaseList<Sm::StatementInterface> > stmtInterfaceList);
}




#endif // MODEL_CONTEXT_H
// vim:foldmethod=syntax
