#ifndef DYNAMIC_SQL_H
#define DYNAMIC_SQL_H

#include <tuple>
#include "semantic_base.h"
#include "lexlib.h"

#include "yylex_decl.h"

namespace SmartLexer {
  class StringLexer;
}

namespace ds {

using namespace Sm;
using namespace std;

std::string fullnameByStack(Sm::ResolvedEntity *defNode);

class ClassificationCathegory {
public:
  enum Cathegory {
    BAD_BY_TOKENS                             = 0,
    BAD_UNCLASSIFIED                          = 1,
    SINGLE_STRING                             = 2,
    SINGLE_CONCATENATION_WITHOUT_VARIABLES    = 3,
    SINGLE_CONCATENATION_WITH_VARIABLES_NOSQL = 4,
    SINGLE_CONCATENATION_WITH_VARIABLES_SQL   = 5,
    ASSIGN_STATEMENTS                         = 6,

    LAST_CATHEGORY
  };
  static string toString(ClassificationCathegory::Cathegory cat);
};


class ClassificationShare {
public:
  size_t            maxDynOpId = 0;
  std::vector<bool> traversedNodes;

  Ptr<SmartLexer::StringLexer> stringLexer;

  void cleanTraversedNodes();

  ClassificationShare();
};


class ClassificationCtx {
public:
  ClassificationShare &share;
  unsigned int  level   = 0;

  ClassificationCtx operator +(unsigned int incLevel) const { return ClassificationCtx(share, level + incLevel); }
  ClassificationCtx(ClassificationShare &_lexer) : share(_lexer) {}
  ClassificationCtx(const ClassificationCtx &oth)
     : share(oth.share), level(oth.level) {}
  ClassificationCtx(ClassificationShare &_lexer, unsigned int _level)
     : share(_lexer), level(_level) {}
};


class DynOpCathegory {
public:
  enum DynCathegory {
    ROOT,
    INIT_ASSIGN,
    APPEND_FRONT,
    APPEND_TAIL,
    VAR_REFERENCE
  };
};


size_t getDynId();


typedef DynOpCathegory::DynCathegory DynCathegory;
class BuildDepTreeCtx;


class DynOperatorRef : public Smart, public DynOpCathegory, public ClassificationCathegory  {
public:
  vector<Ptr<DynOperatorRef> > childs;

  size_t nodeId = getDynId();

  RefAbstract        *ref         = 0;
  StatementInterface *ownerStmt   = 0;
  size_t              ownerStmtId = 0;

  bool isQuotedLeaf       = false;
  bool isAnydataNode      = false;
  bool isToCharFunction   = false;
  bool isCollectionMethod = false;
  bool isDynamicFetch     = false;

  std::set<CathegoriesOfDefinitions::ScopedEntities> changingOperators;

  CathegoriesOfDefinitions::ScopedEntities externalSrcCathegory = CathegoriesOfDefinitions::EMPTY__;

  DynOperatorRef(RefAbstract *_ref, BuildDepTreeCtx &ctx);
  DynOperatorRef *toSelfDynOperatorRef() const { return const_cast<DynOperatorRef*>(this); }

  string toString(size_t parentId, int indentingLevel);
  DynCathegory cathegory() const { return VAR_REFERENCE; }

  ClassificationCathegory::Cathegory classificate(const ClassificationCtx &ctx);
};

class CreatedNodeRef;

class DepTreeShare : public CathegoriesOfDefinitions {
public:
  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> HandledEntities;
  typedef std::map<PlExpr*, DynOperatorRef*> NodesMappedByExprs;
  typedef std::map<ResolvedEntity*, unsigned int, LE_ResolvedEntities> IndexedStatements;

  HandledEntities    handledEntities;
  NodesMappedByExprs nodesMappedByExprs;
  IndexedStatements  indexedStatements;

  IndexedStatements::mapped_type index(StatementInterface *stmt);
};


class BuildDepTreeCtx : public CathegoriesOfDefinitions {
public:
  Ptr<DynOperatorRef> node;
  StatementContext   &stmtContext;
  StatementsTree     &topBlockStmtTree;
  DepTreeShare       &share;

  static void createDynopTree         (BuildDepTreeCtx &ctx, PlExpr *expr);
  static void buildRefDependenciesTree(BuildDepTreeCtx &ctx, RefAbstract *srcRef, ResolvedEntity *entityDef);
  static void buildReferenceNode      (BuildDepTreeCtx &ctx, RefAbstract *ref);

  CreatedNodeRef addRefNode(RefAbstract *ref);

  BuildDepTreeCtx(StatementContext   &_stmtContext,
                  StatementsTree     &_topBlockStmtTree,
                  DepTreeShare       &_share,
                  Ptr<DynOperatorRef> _node = 0
                 );

  BuildDepTreeCtx(const BuildDepTreeCtx &oth);
  void addOrSetNode(Ptr<DynOperatorRef> refNode);
};


class CreatedNodeRef {
public:
  CreatedNodeRef(const BuildDepTreeCtx &_newCtx,  DynOperatorRef *_refDynop, bool _isNewCreated);
  CreatedNodeRef(const CreatedNodeRef &oth);

  BuildDepTreeCtx  newCtx;
  DynOperatorRef  *refNode;
  bool             isNewCreated;
};


class ContextPair  {
public:
  enum Cathegory {
    EXECUTE_IMMIDIATE_ARGUMENT = 0,
    OPEN_FOR_STRING            = 1,
    DBMS_OPEN_CURSOR_ARGUMENT  = 2,
    FETCH_ARGUMENT             = 3,
  };
  size_t nodeId = getDynId();

  Cathegory cathegory;

  /// Аргумент, который берется из оператора EXECUTE IMMIDIATE и оператора OPEN CURSOR
  PlExpr *srcArgExpr;

  Ptr<DynOperatorRef> dynTree;

  ContextPair(PlExpr *_srcArgExpr, Cathegory _cathegory) :
    cathegory(_cathegory),
    srcArgExpr(nAssert(_srcArgExpr)) {}

  string cathegoryToString() const;
};


class DynamicSql : public ClassificationCathegory {
public:
  /// Выражения которые передаются коду исполнения запросов oracle
  std::vector<ContextPair> execExpressions;

  std::vector<ContextPair> classificated[LAST_CATHEGORY];

  void buildDbmsSqlInterface();
  void extractDynamicSql();
  void buildDynamicSqlDependencies();
  void classificate();

  void printPy(const std::string &filename, std::vector<ContextPair> &container);
  void printPy();
};


}


#endif // DYNAMIC_SQL_H
