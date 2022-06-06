#ifndef SEMANTIC_BLOCKPLSQL_H
#define SEMANTIC_BLOCKPLSQL_H

#include "semantic_base.h"

namespace Sm {

class StatementsContainer : public Statement {
public:
  Statements items;
  SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Sm::Datatype> getDatatype() const { return 0; }

  StatementsContainer() { strongRef = 1; }
  SemanticTree *getSemanticNode() const { return 0; }
  void collectSNode(SemanticTree */*n*/) const { throw 999; }
  void translate(Codestream &) { throw 999; }
  CLoc getLLoc() const { throw 999; }

  Statements *getChildStatements() { return &items; }

  typedef Statements::iterator iterator;
  typedef Statements::const_iterator const_iterator;
  typedef Statements::value_type value_type;

  Statements::iterator begin() { return items.begin(); }
  Statements::iterator end() { return items.end(); }

  Statements::const_iterator begin() const { return items.begin(); }
  Statements::const_iterator end() const { return items.end(); }


  bool empty() const { return items.empty(); }
  Statements::size_type size() const { return items.size(); }
  void push_back(const Statements::value_type &v) { items.push_back(v); }
  Statements::value_type &front() { return items.front(); }
  Statements::value_type &back() { return items.back(); }
  void swap(list<Ptr<StatementInterface> > &oth) { items.swap(oth); }

  StatementsContainer* toSelfStatementsContainer() const { return const_cast<StatementsContainer*>(this); }
  ScopedEntities ddlCathegory() const { return StatementsContainer_; }
};

template <>
inline void replace(ExprTr tr, StatementsContainer &container) {
  replaceOnListStatementInterface(tr, container);
}


template<>
inline void replaceStatements(StmtTr tr, StmtTrCond cond, StatementsContainer &lst) {
  for (StatementsContainer::iterator it = lst.begin(); it != lst.end(); ++it)
    replaceStatementInterface(tr, cond, lst.items, it);
}

template<>
inline void traverseStmts(Sm::StatementActor &fun, StatementsContainer &stmts) {
  for (typename StatementsContainer::value_type &sIt : stmts)
    if (sIt)
      sIt->traverseStatements(fun);
}

class BlockPlSql            : public Statement, public virtual ResolvedEntitySNode, public virtual Declaration {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  int branchId_ = 0;
  int setOwnerBlockIsEntered_ = 0;
  static size_t __getGlobalBlkId();
//  size_t __globalBlkId;

public:
  /// Блок объявлений
  Declarations      declarations;
  /// Блок операторов
  StatementsContainer statements;

  StatementsContainer initializators;
  StatementsContainer exceptionHandlers;

  Declarations      removedCursors;
protected:
  Statements stmtsContainer = {
    smart::Ptr<StatementInterface>(&initializators),
    smart::Ptr<StatementInterface>(&statements),
    smart::Ptr<StatementInterface>(&exceptionHandlers)
  };

  std::string         translatedName;
  std::stack<Ptr<Sm::Loop> > loopStack;
  typedef std::map<ResolvedEntity *, Ptr<Variable>, LE_ResolvedEntities> LockerMap;
  LockerMap cursorLockers;

public:
  BlockPlSql*        packageBody_ = 0;
  /// Блок исключений
  ExceptionHandlers  translatedExceptionDeclarations;

  DeclaredExceptions exceptionFromOperators;

  Ptr<PlExpr>        dynamicDeclTail;

  Ptr<Id>            endLabel;
  int                labelCounter_      = 0;
  bool               isNotPackageBlock_ = true;
  bool               isFunctionTopBody_ = false;

protected:
  BlockPlSql(Ptr<Declarations> _declarations, Ptr<Statements> _statements,  Ptr<ExceptionHandlers> _exceptionHandlers, Ptr<Id> _endLabel);

public:
  BlockPlSql(CLoc l,
             Ptr<Declarations>      _declarations      = 0,
             Ptr<Statements>        _statements        = 0,
             Ptr<ExceptionHandlers> _exceptionHandlers = 0,
             Ptr<Id>                _endLabel          = 0);

  BlockPlSql();


  BlockPlSql(CLoc l, Ptr<Declarations> _declarations, Ptr<PlExpr> _dynamicDeclTail);

  bool empty() { return statements.empty() && initializators.empty() && exceptionHandlers.empty(); }

  void pushLoop(Sm::Loop* loop);
  void popLoop(Sm::Loop* loop);
  Ptr<Sm::Loop> getActiveLoop() const;
  void collectStatementsSNode(SemanticTree *&node) const;
  bool isBlockPlSql() const { return true; }

  bool setOwnerBlockPlSql(BlockPlSql *b);
  void setOwnerBlockOnChildrenDeclarations();
  void incBranchId() { ++branchId_; }
  int  branchId() const { return branchId_; }
  Sm::BlockPlSql* toSelfBlockPlSql() const { return (BlockPlSql*)this; }
  size_t exceptionListSize() const { return exceptionHandlers.size(); }
  Sm::Declarations *blockDeclarations()   { return &declarations; }
  StatementsContainer *blockInitializators() { return &initializators; }
  void extractBlockExternalVariables(ExternalVariables &sortedExternalReferences, UniqueEntitiesMap &externalReferences);
  void addReturn(Ptr<Id> name);
  void addToInitializators(ResolvedEntity *var, Sm::PlExpr *expr);
  BlockPlSql *getTopBlock();
  bool hasExceptionDeclaration(Ptr<PlExpr> exception);
  void addExceptionDeclaration(Ptr<WhenExpr> exception);
  Ptr<Declaration> hasDeclaration(Ptr<Id> varId);
  Ptr<Variable> getCreateCursorLocker(Codestream &str, ResolvedEntity *ent);

  Statements *getChildStatements() { return &stmtsContainer; }

  BlockPlSql* maximalCodeBlock(Sm::BlockPlSql *start = 0) const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; /* блок кода не должен быть типом данных */ }

  ScopedEntities ddlCathegory() const { return BlockPlSql_; }
  Ptr<Datatype> getDatatype() const;

  SemanticTree *toSTreeBNew() const;

  void collectSNodeBase(SemanticTree *&n) const;

  void collectSNode(SemanticTree *n) const;

  SemanticTree *toSTreeBase() const;

  void addDeclarations(Ptr<Declarations> decl);
  void add(Ptr<Declaration> decl);

  void translateDeclarations(Codestream &str, DependEntitiesMap *filter, std::set<ResolvedEntity*, LE_ResolvedEntities> &outputted, bool isDecl_);
  void linterDefinition(Codestream &str);

  void linterGlobalVariable(Codestream &str, Declarations::iterator it);
  void translateDependedBlockVariablesToArglist(Codestream &str, ExternalVariables &arglist, UniqueEntitiesMap &externalReferences);

  void collectInitializers(EntitiesSet &container);

  void traverseDeclarations(DeclActor &fun);
  void traverseDeclarationsStmt(DeclActor &fun);

  void traverseChildStatements(StatementActor &fun) {
    traverseStmts(fun, statements, exceptionHandlers);
  }
  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun, statements, initializators, exceptionHandlers); }
  void traverseModelStatements(StatementActor &fun);

  void buildStatementsTree(StatementsTree &parent);
  StatementsTree buildStatementsTree();

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, declarations, statements, initializators, exceptionHandlers); }
  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceBlockSubstatementsIf(StmtTr tr, StmtTrCond cond);


  template <class Predicate, class ResultList = Statements>
  bool findStatements(Predicate pred, ResultList &result, bool findFirst);

  Sm::RefAbstract *findLastReturnExpression();

  void traverseDecls(DeclActor &fun);
  bool traverseDecls(DeclActor &fun, std::stack<BlockPlSql*> &stackBlks);

  // whenCondition - нужна для триггеров
  void linDef(Codestream &str, PlExpr *whenCondition = 0);
  void lindefCodeBlock(Codestream &str, bool declareIsNotOutput = true, bool isInternalBlock = false, PlExpr *whenCondition = 0);

  void mergeInternalBlockDecl(BlockPlSql *topBlock);

protected:
  void initBlockPlSql(Declarations* _declarations, list<Ptr<StatementInterface> > *_statements, list<Ptr<StatementInterface> > *_exceptionHandlers);
  BlockPlSql(Ptr<BlockPlSql> o);
};

template <class Predicate, class ResultList>
bool BlockPlSql::findStatements(Predicate pred, ResultList &result, bool findFirst) {
  for (Statements::iterator it = statements.begin(); it != statements.end(); ++it) {
    if ((*it)->isBlockPlSql()) {
      BlockPlSql *block = (*it)->toSelfStatement()->toSelfBlockPlSql();
      if (block->findStatements(pred, result, findFirst) && findFirst)
        return true;
    }
    else if (pred((*it).object())) {
      result.push_back(*it);
      if (findFirst)
        return true;
    }
  }
  return result.size() > 0;
}







}

#endif // SEMANTIC_BLOCKPLSQL_H

