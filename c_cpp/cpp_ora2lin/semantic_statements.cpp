#include <exception>
#include "semantic_statements.h"
#include "semantic_datatype.h"
#include "semantic_sql.h"
#include "semantic_function.h"
#include "semantic_blockplsql.h"
#include "statements_tree.h"
#include "resolvers.h"
#include "usercontext.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "dynamic_sql_op.h"

extern SyntaxerContext syntaxerContext;

namespace Sm {
  Codestream& translateIdReference(Codestream &str, Ptr<Id> id, bool &isNotFirst, ResolvedEntity::ScopedEntities &prevCat, bool &userAlreadyOutput);
  bool checkToRowidPseudocolumn(Ptr<Id> &field);
}


using namespace Sm;


StatementsTree::StatementsTree(StatementInterface *_stmt)
  : stmt_(nAssert(_stmt)) {}

void StatementsTree::addChild(Ptr<StatementsTree> stmt) {
  stmt->parent = this;
  childs.push_back(stmt);
}

void StatementsTree::getAllReturn(Stmts &stmts) {
  if (stmt_->toSelfReturn())
    stmts.push_back(this);
  for (Childs::value_type &v : childs)
    v->getAllReturn(stmts);
}

StatementsTree::Stmts StatementsTree::getStatementsChangedEntityBeforeStmt(ResolvedEntity *ent, StatementsTree *endStmt) {
  Stmts v;
  getStatementsChangedEntityBeforeStmt(v, ent, endStmt);
  return v;
}

/**
 * @brief StatementsTree::getStatementsChangedEntityBeforeStmt
 * @param stmts
 * @param ent
 * @param endStmt
 * @return true - если нужно продолжать просмотр списка стейтментов,
 *         false - если встретился завершающий оператор и просмотр нужно прекратить на текущем уровне
 */
void StatementsTree::traverseChildFindStmtsChangedEntity(StatementsTree::Stmts &stmts, ResolvedEntity *ent, StatementsTree *endStmt)
{
  for (Childs::value_type &c : childs)
    if (!c->getStatementsChangedEntityBeforeStmt(stmts, ent, endStmt))
      break;
}


bool StatementsTree::addStatementToChangerVector(Stmts &stmts, ResolvedEntity *ent, StatementsTree *endStmt) {
  rRef refEnt(ent);
  using namespace insert;

  ScopedEntities cat = stmt_->ddlCathegory();
  switch (cat) {
    case Assignment_: {
      Assignment *op = stmt_->toSelfAssignment();
      if (rRef(op->lValue->refDefinition()) == refEnt || op->assignedExpr->changesEntity(ent))
        return true;
    } break;

    case FunctionCall_:
      if (nAssert(stmt_->toSelfFunctionCall())->funCall->changesEntity(ent))
        return true;
      break;

    case OpenFor_:
      if (rRef(nAssert(stmt_->toSelfOpenFor())->openForIterVariable->definition()) == refEnt)
        return true;
      break;

    case BlockPlSql_:
    case If_:
    case Loop_:
    case ForOfRange_:
    case CaseStatement_:
      traverseChildFindStmtsChangedEntity(stmts, ent, endStmt);
      break;

    case WhenExpr_: {
      WhenExpr *op = stmt_->toSelfWhenExpr();
      traverseChildFindStmtsChangedEntity(stmts, ent, endStmt);
      if (op->condition && op->condition->changesEntity(ent))
        return true;
    } break;

    case SingleInsert_: {
      SingleInsert *op = stmt_->toSelfSingleInsert();
      if (Ptr<ReturnInto> ret = op->data->getReturning())
        for (IntoCollections::value_type &v : *ret->intoCollections)
          if (rRef(v->refDefinition()) == refEnt)
            return true;
      if (op->changesEntityInExprFuncall(ent))
        return true;
    } break;

    case SelectStatement_: {
      SelectStatement *stmt = stmt_->toSelfSelectStatement();
      if (Subquery::IntoList into = stmt->subquery->intoList())
        for (List<RefAbstract>::value_type &v : *into)
          if (rRef(v->refDefinition()) == refEnt)
            return true;
    } break;

    case ExecuteImmediate_: {
      ExecuteImmediate *stmt = stmt_->toSelfExecuteImmediate();
      if (stmt->intoVars)
        for (Ptr<RefExpr> &ref : *(stmt->intoVars))
          if (rRef(ref->refDefinition()) == refEnt)
            return true;

//      sAssert(stmt->retlist);
    } break;

    case Return_: {
      Return *stmt = stmt_->toSelfReturn();
      if (stmt->expr && stmt->expr->changesEntity(ent))
        return true;
    } break;

    case OpenCursor_: {
      OpenCursor *stmt = stmt_->toSelfOpenCursor();
      if (rRef(stmt->cursor->refDefinition()) == refEnt)
        return true;
    } break;

    case Fetch_: {
      Fetch *stmt = stmt_->toSelfFetch();
      if (rRef(stmt->cursorRef->refDefinition()) == refEnt)
        return true;
      for (Fetch::Fields::value_type &f : stmt->fields)
        if (rRef(f->refDefinition()) == refEnt)
          return true;
    } break;

    case Exit_: {
      Exit *stmt = stmt_->toSelfExit();
      if (stmt->when && stmt->when->changesEntity(ent))
        return true;
    } break;

    case ForOfExpression_: {
      ForOfExpression *stmt = stmt_->toSelfForOfExpression();
      traverseChildFindStmtsChangedEntity(stmts, ent, endStmt);
      if (rRef(stmt->indexVariable->definition()) == refEnt)
        return true;
    } break;

    case While_: {
      While *stmt = stmt_->toSelfWhile();
      traverseChildFindStmtsChangedEntity(stmts, ent, endStmt);
      if (stmt->condition && stmt->condition->changesEntity(ent))
        return true;
    } break;

    case Update_:
      sAssert(stmt_->toSelfUpdate()->returnInto);
      break;

    case DeleteFrom_:
      sAssert(stmt_->toSelfDeleteFrom()->returnInto);
      break;

    case NullStatement_:
    case Commit_:
    case DeclNamespace_:
    case ConstructExprStmt_:
    case CursorFieldDecltype_:
    case Close_:
    case Resignal_:
    case Rollback_:
    case Raise_:
    case CursorDecltype_:
    case Goto_:
    case Label_:
    case LockTable_:
    case Savepoint_:
    case PipeRow_:
      break;
    default:
      throw 999;
  }
  return false;
}

bool StatementsTree::getStatementsChangedEntityBeforeStmt(StatementsTree::Stmts &stmts, ResolvedEntity *ent, StatementsTree *endStmt) {
  static size_t cnt = 0;
  ++cnt;
  (void)cnt;
  if (nodeStmt()->beginedFrom(56817))
    cout << "";

  // реализовать так: все категории, что не обрабатываются - генерируют исключение
  if (this == endStmt)
    return false;

  if (addStatementToChangerVector(stmts, ent, endStmt))
    stmts.push_back(this);

  return true;
}


bool StatementInterface::changesEntityInExprFuncall(ResolvedEntity *ent) const {
  struct FoundedExprOk : public std::exception {
    virtual const char* what() const throw() { return "Function call, that changes expr - founded"; }
  };

  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (construct && expr->changesEntity(ent) )
      throw FoundedExprOk();

    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr* { return e; };
  ExprTR tr(trFun, cond);

  try {
    const_cast<StatementInterface*>(this)->replaceChildsIf(tr);
  }
  catch (FoundedExprOk) {
    return true;
  }
  return false;

}

bool PlExpr::changesEntity(ResolvedEntity *ent) const {
  bool foundedOk = false;
  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (foundedOk)
      return FLAG_REPLACE_SKIP_DEPTH_TRAVERSING;
    if (!construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;
    if (Ptr<IdEntitySmart> ref = expr->getMajorIdEntity())
      for (Ptr<Id> &id : *ref) {
        ResolvedEntity *d;
        Function       *fun;
        if (id->callArglist && (d = id->definition()) && (fun = d->toSelfFunction())) {
          unsigned int pos = 0;
          for (Ptr<FunCallArg> &fc : *(id->callArglist)) {
            IdEntitySmart *argRef;
            ResolvedEntity *argCallDef;
            FunctionArgument *argDef;

            if ((argRef     = fc->expr()->getMajorIdEntity()) &&
                (argCallDef = argRef->definition()) &&
                rRef(argCallDef) == rRef(ent) &&
                fun->arglistSize() >= pos &&
                (argDef = *(fun->arglistBegin() + pos)) && argDef->out())
            {
              if (Ptr<BlockPlSql> b = fun->body()) {
                StatementsTree tr = b->buildStatementsTree();
                StatementsTree::Stmts ents = tr.getStatementsChangedEntityBeforeStmt(argDef, 0);
                if (ents.size()) {
                  foundedOk = true;
                  return FLAG_REPLACE_SKIP_DEPTH_TRAVERSING;
                }
              }
              else {
                foundedOk = true;
                return FLAG_REPLACE_SKIP_DEPTH_TRAVERSING;
              }
            }
            ++pos;
          }
        }
      }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr* { return e; };
  ExprTR tr(trFun, cond);

  if (RefAbstract *op = this->toSelfRefAbstract())
    cond(op, tr, true);
  if (!foundedOk)
    const_cast<PlExpr*>(this)->replaceChildsIf(tr);
  return foundedOk;
}

BaseList<StatementInterface> *Sm::StatementWithLabelPrefixedList::toStatementInterfaceList() const {
  Sm::BaseList<Sm::StatementInterface> *p = new Sm::BaseList<Sm::StatementInterface>();
  toStatementInterfaceList(p);
  return p;
}

void Sm::StatementWithLabelPrefixedList::toStatementInterfaceList(Sm::BaseList<Sm::StatementInterface> *p) const {
  if (labels)
    for (List<Label>::const_iterator it = labels->begin(); it != labels->end(); ++it)
      p->push_back(it->object());
  if (stmt)
    p->push_back(stmt);
}


BlockPlSql::BlockPlSql(Ptr<Declarations> _declarations, Ptr<Statements> _statements, Ptr<ExceptionHandlers> _exceptionHandlers, Ptr<Id> _endLabel)
  : endLabel(_endLabel) { initBlockPlSql(_declarations, _statements, _exceptionHandlers); }

BlockPlSql::BlockPlSql(CLoc l, Ptr<Declarations> _declarations, Ptr<Statements> _statements, Ptr<ExceptionHandlers> _exceptionHandlers, Ptr<Id> _endLabel)
  : GrammarBase(l), endLabel(_endLabel) { initBlockPlSql(_declarations, _statements, _exceptionHandlers); }

BlockPlSql::BlockPlSql() {}

BlockPlSql::BlockPlSql(CLoc l, Ptr<Declarations> _declarations, Ptr<PlExpr> _dynamicDeclTail)
  : GrammarBase(l), dynamicDeclTail(new Sm::FunctionDynField(l, _dynamicDeclTail)) {
  initBlockPlSql(_declarations, 0, 0);
}


RefAbstract *BlockPlSql::findLastReturnExpression() {
  Sm::Statements retList;
  auto func = [](Sm::StatementInterface *stmt) -> bool {
    return stmt->toSelfStatement() != NULL && stmt->toSelfStatement()->toSelfReturn() != NULL;
  };

  if (findStatements(func, retList, false)) {
    Sm::Return *ret = retList.back()->toSelfStatement()->toSelfReturn();
    if (ret->expr)
      return ret->expr->toSelfRefAbstract();
  }
  return 0;
}


StatementsTree BlockPlSql::buildStatementsTree() {
  StatementsTree node(this);
  buildStmtsTree(node, statements, exceptionHandlers);
  return node;
}

BlockPlSql::BlockPlSql(Ptr<BlockPlSql> o)
  : GrammarBase      (o->getLLoc()),
    branchId_        (o.object()->branchId_),
    endLabel         (std::forward<Ptr<Id> >         (o->endLabel          ))
{
  initBlockPlSql(&o->declarations, &o->statements.items, &o->exceptionHandlers.items);
}

If::If(CLoc l, Ptr<WhenExpr> _first, Ptr<BaseList<StatementInterface> > _elseIfStmts, Ptr<WhenExpr> _elseStmts)
  : GrammarBase(l)
{
  if (!_first)
    throw 999;

  if (_elseIfStmts)
    branches.swap(*_elseIfStmts);

  branches.push_front(_first.object());
  if (_elseStmts)
    branches.push_back(_elseStmts.object());
}

If::If(FLoc l, BaseList<StatementInterface> &_statements)
  : GrammarBase(l)
{
  branches.swap(_statements);
}


void If::traverseDeclarationsStmt(DeclActor &fun) {
  for (auto &v : branches)
    v->traverseDeclarationsStmt(fun);
}

void If::traverseStatements(StatementActor &fun) { traverseStmts(this, fun, branches); }

void If::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, branches); }

void If::replaceChildsIf(ExprTr tr)  { replace(tr, branches); }

void If::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, branches);
}

void If::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementIf, this);
  n->addChildForce(node);
  CollectSNode(node, branches);
}

OpenCursor::OpenCursor(CLoc l, Ptr<Id> _cursor, Ptr<CallArgList> _actualCursorParameters)
  : GrammarBase(l)
{
  if (_cursor)
    cursor = new RefExpr(l, _cursor);
  if (_actualCursorParameters)
    for (CallArgList::value_type &it : *_actualCursorParameters)
      actualCursorParameters.push_back(it);
}

void OpenCursor::replaceChildsIf(ExprTr tr) { replace(tr, cursor, actualCursorParameters); }

void OpenCursor::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementOpenCursor, this);
  n->addChildForce(node);
  if (cursor)
    node->addChild(cursor->toSTree());
  CollectSNode(node, actualCursorParameters);
}


OpenFor::OpenFor(CLoc l, Ptr<Id> curVar, SqlExpr *sel, List<ArgumentNameRef> *bindArgs)
  : GrammarBase(l),
    baseCursorVariable(new RefExpr(l, Sm::nAssert(curVar))),
    openForIterVariable(Sm::nAssert(curVar)),
    select(sel),
    bindArguments(bindArgs)
{}



ResolvedEntity *OpenFor::openedEntity() { return openForIterVariable->unresolvedDefinition(); }

bool Sm::OpenFor::getStatementsThatContainEntity(ResolvedEntity *entity, FoundedStatements &outList) {
  if (openForIterVariable->unresolvedDefinition() == entity)
    outList.push_back(this);
  else
    select->getStatementsWithEntity(entity, this, outList);
  return true;
}

void OpenFor::changeCursorVariable(Ptr<LinterCursor> cur) { openForIterVariable = cur->getName(); }

void OpenFor::replaceChildsIf(ExprTr tr) { replace(tr, baseCursorVariable, select, bindArguments); }

void OpenFor::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementOpenFor, this);
  n->addChildForce(node);
  node->addChild(baseCursorVariable->toSTree());
  select->collectSNode(node);
  CollectSNode(node, bindArguments);
}

Sm::While::While(CLoc l, Ptr<PlExpr> _condition, Ptr<Loop> _loop)
  : GrammarBase(l), condition(_condition), loop(_loop)
{
  if (!loop)
    throw 999;
}

void While::traverseDeclarationsStmt(DeclActor &fun) {
  if (loop)
    loop->traverseDeclarationsStmt(fun);
}

void While::traverseStatements(StatementActor &fun) { traverseStmts(this, fun, *loop); }

void While::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, *loop); }

void While::replaceChildsIf(ExprTr tr) { replace(tr, condition, loop); }

Statements *While::getChildStatements() { return loop->getChildStatements(); }

void While::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, loop->loopStatements);
}

void While::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementWhile, this);
  n->addChildForce(node);
  condition->collectSNode(node);
  loop->collectSNode(node);
}

FunctionCall::FunctionCall(CLoc l, SqlExpr *call)
  : GrammarBase(l), funCall(call)
{
  if (!funCall)
    throw 999;
}

void FunctionCall::replaceChildsIf(ExprTr tr) { replace(tr, funCall); }


//ResolvedEntity* FunctionCall::refDefinition() const {
//  if (RefExpr *x = funCall->toSelfRefExpr())
//    return x->refDefinition();
//  return 0;
//}

Ptr<IdRef> FunctionCall::reference() const {
  if (RefExpr *x = funCall->toSelfRefExpr())
    return x->reference;
  return 0;
}

void FunctionCall::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementFunctionCall, this);
  n->addChildForce(node);
  if (funCall) {
    if (RefExpr *r = funCall->toSelfRefExpr()) {
      SemanticTree *t = r->toSTree();
      t->unnamedDdlEntity = r;
      // t->unnamedDdlEntity = (FunctionCall*)this;
      node->addChild(t);
    }
    else
      funCall->collectSNode(n);
  }
}

Sm::Close::Close(CLoc &l, Id *curEnt)
  : GrammarBase(l), cursorEntity(curEnt) {}

void Close::changeCursorVariable(Ptr<LinterCursor> cur) { cursorEntity = cur->getName(); }

void Close::replaceChildsIf(ExprTr tr) { replace(tr, cursorEntity);  }

void Close::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementClose, this);
  n_src->addChildForce(node);

  node->addChild(cursorEntity ? cursorEntity->toSNodeRef(SCathegory::CursorEntity) : (SemanticTree*)0);
}

Sm::Goto::Goto(CLoc &l, Id *_gotoLabel)
  : GrammarBase(l), gotoLabel(_gotoLabel) {}

void Goto::replaceChildsIf(ExprTr tr) { replace(tr, gotoLabel); }

void Goto::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementGoto, this);
  n->addChildForce(node);
  node->addChild(gotoLabel ? gotoLabel->toSNodeRef(SCathegory::Label) : (SemanticTree*)0);
}

PipeRow::PipeRow(CLoc l, Ptr<IdEntitySmart> _pipedRow) : GrammarBase(l), pipedRow(_pipedRow) { checkIdEntitySmart(pipedRow); }

void PipeRow::replaceChildsIf(ExprTr tr) { replace(tr, pipedRow); }

void PipeRow::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementPipeRow, this);
  n->addChildForce(node);
  node->addChild(pipedRow->toSTreeRef(SCathegory::Variable));
}

Label::Label(Ptr<Id> n) : name(n) {
  if (name)
    name->definition(this);
}

void Label::replaceChildsIf(ExprTr tr) { replace(tr, name); }

void Label::collectSNode(SemanticTree *n) const {
  SemanticTree *node = name->toSNodeDecl(SCathegory::Label, this);
  setSemanticNode(node);
  n->addChild(node);
}

Raise::Raise(CLoc l, Ptr<IdEntitySmart> _exception)
  : GrammarBase(l), exception(_exception) { checkIdEntitySmart(exception); }

void Raise::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementRaise, this);
  setSemanticNode(node);
  n->addChildForce(node);
  node->addChild(exception->toSTreeRef(SCathegory::Exception));
}

void Raise::replaceChildsIf(ExprTr tr) { replace(tr, exception); }

Resignal::Resignal(CLoc l) : GrammarBase(l) {}

void Resignal::collectSNode(SemanticTree *n) const { setSemanticNode(n); }

Return::Return(CLoc l, Ptr<PlExpr> _expr) : GrammarBase(l), expr(_expr) {}

Ptr<Datatype> Return::getDatatype() const {
  PlExpr *e = expr.object();
  return e ? e->getDatatype() : Ptr<Datatype>();
}

void Return::replaceChildsIf(ExprTr tr) { replace(tr, expr); }

void Return::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::Return);
  node->unnamedDdlEntity = const_cast<Return*>(this);
  CTREE2(node, expr);
  n->addChild(node);
}



void ForOfRange::traverseDeclarationsStmt(DeclActor &fun) {
  if (loop)
    loop->traverseDeclarationsStmt(fun);
}

void ForOfRange::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, *loop); }

void ForOfRange::replaceChildsIf(ExprTr tr) { replace(tr, indexVariable, bounds, loop); }

Statements *ForOfRange::getChildStatements() { return loop->getChildStatements(); }

void ForOfRange::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, loop->loopStatements);
}

void ForOfRange::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new Sm::SemanticTree(SCathegory::ForOfRange, SemanticTree::NEW_LEVEL);
  CTREE(idxVar);
  SemanticTree *boundsNode = new Sm::SemanticTree(SCathegory::ForOfRangeRange);
  n->addChildForce(boundsNode);
  CTREE2(boundsNode, bounds);
  CTREE(loop);
  node->addChild(n);
}


ForOfRange::ForOfRange(CLoc &l, LoopBounds *_bounds, Loop *_loop)
  : GrammarBase(l),
    bounds(_bounds),
    loop(nAssert(_loop)) {}

void ForOfRange::setIndexVariable(CLoc &l, Ptr<Id> indexVar, bool reverse) {
  setLLoc(l);
  indexVariable = indexVar;
  isReverse     = reverse;

  idxVar = new Variable(indexVariable, Datatype::mkInteger());
  idxVar->flags.setDynamicLoopCounter();
  idxVar->loc(indexVariable->getLLoc());
}

ForOfExpression::ForOfExpression(CLoc l, Ptr<SqlExpr> sqlExpr, Ptr<Loop> _loop)
  : GrammarBase(l),
    sqlExpression(sqlExpr),
    loop(nAssert(_loop)) {}


void ForOfExpression::setIndexVariable(CLoc &l, Ptr<Id> indexVar, bool reverse) {
  setLLoc(l);
  indexVariable = indexVar;
  isReverse = reverse;

  indexVariableDefinition = new VariableField(indexVar, sqlExpression);
  indexVariableDefinition->flags.setDynamicLoopCounter();
}

bool Sm::ForOfExpression::setOwnerBlockPlSql(BlockPlSql *b) {
  if (!b)
    throw 999;
  ownerBlock = b;
  if (indexVariableDefinition)
    indexVariableDefinition->setOwnerBlockPlSql(b);
  return true;
}

void ForOfExpression::traverseDeclarationsStmt(DeclActor &fun) {
  if (loop)
    loop->traverseDeclarationsStmt(fun);
}

Statements *ForOfExpression::getChildStatements() { return loop->getChildStatements(); }

void ForOfExpression::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, loop->loopStatements);
}

void ForOfExpression::replaceChildsIf(ExprTr tr) { replace(tr, indexVariable, sqlExpression, loop); }

void ForOfExpression::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementForOfExpression, this);
  n_src->addChildForce(node);

  if (!ownerBlock)
    throw 999;

  CTREE2(node, sqlExpression);

  SemanticTree *n = new SemanticTree(SCathegory::ForOfExpression, SemanticTree::NEW_LEVEL);
  n->unnamedDdlEntity = (ResolvedEntity*)this;
  n->addChild(indexVariableDefinition->toSTree());
  CTREE(loop);
  setSemanticNode(n);
  node->addChild(n);
}

void ForOfExpression::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, *loop); }

Sm::IsSubtypeValues ForAll::isSubtype(ResolvedEntity *, bool) const { return IsSubtypeValues::EXPLICIT; }

ForAll::ForAll(CLoc l, Id *_idx, LoopBounds *_bounds, SqlStatementInterface *sqlStmt)
  : GrammarBase(l), indexVariable(_idx), bounds(_bounds), sqlStatement(sqlStmt)
{
  idxVar = new Variable(indexVariable, Datatype::mkInteger());
  idxVar->flags.setDynamicLoopCounter();
  idxVar->loc(indexVariable->getLLoc());

  if (!sqlStatement)
    throw 999;
  singleStatementContainer.push_back(sqlStatement.object());
}

void ForAll::replaceChildsIf(ExprTr tr) { replace(tr, indexVariable, bounds, sqlStatement); }

void ForAll::replaceSubstatementsIf(StmtTr /*tr*/, StmtTrCond /*cond*/) {
  // replaceStatements(tr, cond, *sqlStatement);
}

void ForAll::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(SCathegory::ForAll, SemanticTree::NEW_LEVEL);
  if (indexVariable)
    n->addChild(indexVariable->toSNodeDecl(SCathegory::VariableUndeclaredIndex, this));
  CTREE(idxVar);
  CTREE(bounds)
  if (sqlStatement)
    sqlStatement->collectSNode(n);
  node->addChild(n);
}

void ForAll::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, *sqlStatement); }

LoopBounds::LoopBounds(CLoc l, SqlExpr *_lowerBound, SqlExpr *_upperBound)
  : GrammarBaseSmart(l), lowerBound(_lowerBound), upperBound(_upperBound) {}

void LoopBounds::replaceChildsIf(ExprTr tr) { replace(tr, lowerBound, upperBound); }

void LoopBounds::collectSNode(SemanticTree *n) const { (void)n; CTREE(lowerBound); CTREE(upperBound); }

NullStatement::NullStatement() {}

NullStatement::NullStatement(CLoc l) : GrammarBase(l) {}

void NullStatement::collectSNode(SemanticTree *) const {}

void NullStatement::linterDefinition(Codestream &str) { str << "NULL"; }

Fetch::Fetch(CLoc l, Ptr<Id> _cursorRef, Ptr<List<IdEntitySmart> > _fields, Fetch::Into cat, Ptr<PlExpr> _limit)
  : GrammarBase(l), cursorRef(new RefExpr(_cursorRef)), limit(_limit), intoCathegory(cat) {
  if (_fields) {
    for (Ptr<IdEntitySmart> &it : *_fields)
      fields.push_back(static_cast<RefAbstract*>(new RefExpr(it->back()->getLLoc(), it)));
  }
}

bool Sm::Fetch::getFields(EntityFields &f) const {
  for (auto it = fields.begin(); it != fields.end(); ++it)
    f.push_back((*it)->refEntity());
  return f.size() > 0;
}

void Fetch::changeCursorVariable(Ptr<LinterCursor> cur) { cursorRef->reference->back() = cur->getName(); }

void Fetch::replaceChildsIf(ExprTr tr) { replace(tr, cursorRef, fields, limit); }

void Fetch::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementFetch, this);
  n_src->addChildForce(node);
  if (cursorRef)
    cursorRef->collectSNode(node);
  for (Fields::const_reference &it : fields)
    node->addChild(it->toSTree());
  if (limit)
    limit->collectSNode(node);
}

Exit::Exit(CLoc l, Ptr<PlExpr> _when, Ptr<Id> _label)
  : GrammarBase(l), label(_label), when(_when) {}

void Exit::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementExit, this);
  n_src->addChildForce(node);
  node->addChild(label ? label->toSNodeRef(SCathegory::Label) : (SemanticTree*)0);
  if (when)
    when->collectSNode(node);
}

ExecuteImmediate::ExecuteImmediate(CLoc l, Ptr<PlExpr> _execExpr)
  : GrammarBase(l), execExpr(_execExpr) {}

ExecuteImmediate::ExecuteImmediate(CLoc l, Ptr<List<ArgumentNameRef> > _usingArglist, Ptr<IntoCollections> _retlist, Ptr<IntoCollections> _intoVars)
  : GrammarBase(l), intoVars(_intoVars), usingArglist(_usingArglist), retlist(_retlist) {}

void ExecuteImmediate::replaceChildsIf(ExprTr tr) { replace(tr, execExpr, intoVars, usingArglist, retlist); }

void ExecuteImmediate::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementImmediate, this);
  n_src->addChildForce(node);
  setSemanticNode(node);

  execExpr->collectSNode(node);
  SemanticTree *intoNode = new SemanticTree(SCathegory::Into);
  intoNode->setIsList();
  if (intoVars)
    for (IntoCollections::const_reference ref : *intoVars)
      intoNode->addChild(ref->reference->toSTreeRef(SCathegory::Variable));
  node->addChild(intoNode);
  CollectSNode(node, usingArglist);
  if (retlist)
    for (IntoCollections::const_reference ref : *retlist)
      node->addChild(ref->reference->toSTreeRef(SCathegory::Variable));
}

If* CaseStatement::transformIfSpecial() {
  bool translAsIf  = false;
  bool needEqualOp  = true;
  if (beginedFrom(145251,5))
    cout << "";

  if (!caseOperand || caseOperandIsBooleanLiteral()) {
    translAsIf = true;
    needEqualOp = false;
  }
  else if (whenListConditionComplex() || inExceptionSection())
    translAsIf = true;

  if (!translAsIf || whenList.empty())
    return 0;

  WhenExpr::Cathegory cat =  WhenExpr::IF_FIRST_STATEMENT;
  ResolvedEntitySNode::pushTransformStage();
//  Sm::cleanSemanticNodeOfPlExpr(caseOperand);

  for (BaseList<StatementInterface>::iterator it = whenList.begin(); it != whenList.end(); ++it) {
    WhenExpr *op = (*it)->toSelfWhenExpr();
    if (!op)
      throw 999; // некорректно строится список операторов для кейса
    switch (op->cathegory) {
      case WhenExpr::WHEN_THEN:
        op->cathegory = cat;
        if (needEqualOp) {
          Ptr<Sm::pl_expr::Comparsion> cmp = new Sm::pl_expr::Comparsion(op->condition->getLLoc(), caseOperand->deepCopy(), op->condition);

          cmp->op = ComparsionOp::EQ;
          op->condition = cmp.object();
          op->condition->collectSNode(op->getSemanticNode());
          //            op->getSemanticNode()->unnamedDdlEntity = op->condition.object();
        }
        break;
      case WhenExpr::WHEN_OTHERS:
        op->cathegory = WhenExpr::ELSE_STATEMENT;
        break;
      default:
        throw 999;
        break;
    }
    cat = WhenExpr::ELSEIF_STATEMENT;
  }
  If *ifStmt = new Sm::If(getLLoc(), whenList);
  if (SemanticTree *n = getSemanticNode()) {
    n->cathegory = SCathegory::StatementIf;
    ifStmt->setSemanticNode(n);
    n->unnamedDdlEntity = ifStmt;
    this->setSemanticNode(0);
    if (n->sid == 1792570)
      cout << "";
    n->deleteInvalidChilds();
  }
  else
    throw 999;

  ResolvedEntitySNode::popStage();

  return ifStmt;
}

CaseStatement::CaseStatement(CLoc l, Ptr<PlExpr> _caseOperand, Ptr<BaseList<StatementInterface> > whenLst, Ptr<WhenExpr> _elseStmts, Ptr<Id> endLbl)
  : GrammarBase(l), caseOperand(_caseOperand), endLabel(endLbl)
{
  swap(whenList, whenLst);
  if (_elseStmts)
    whenList.push_back(_elseStmts.object());
}

Sm::IsSubtypeValues CaseStatement::isSubtype(ResolvedEntity *, bool) const {
  return IsSubtypeValues::EXPLICIT; /* case statement не должен быть типом данных */
}

void CaseStatement::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, whenList);
}

void CaseStatement::replaceChildsIf(ExprTr tr) { replace(tr, caseOperand, whenList, endLabel); }

void CaseStatement::traverseDeclarationsStmt(DeclActor &fun) {
  for (auto &v : whenList)
    v->traverseDeclarationsStmt(fun);
}

void CaseStatement::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, whenList); }

void CaseStatement::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementCase, this);
  n_src->addChildForce(node);
  setSemanticNode(node);

  if (caseOperand)
    caseOperand->collectSNode(node);
  CollectSNode(node, whenList);
  if (endLabel)
    node->addChild(endLabel->toSNodeDef(SCathegory::Label, this));
}














LinterCursorField::LinterCursorField(Ptr<Id> _name, ResolvedEntity *_resolvedField, ResolvedEntity *_ownerVariable)
  : name(_name), resolvedField(_resolvedField), ownerVariable(_ownerVariable), transitive(false)
{
  name->definition((ResolvedEntity*)this);
}

void Sm::LinterCursorField::linterReference(Sm::Codestream &str) {
  if (resolvedField && ownerVariable && resolvedField->isRownum()) {
    str << "currow" << s::obracket << s::ref << *ownerVariable << s::cbracket;
  }
  else {
    if (!transitive && ownerVariable) {
      ownerVariable->linterReference(str);
      str << ".";
    }
    if (!isTrNameEmpty())
      TranslatedName::translateName(str);
    else
      resolvedField->linterReference(str);
  }
}

Ptr<Datatype> LinterCursorField::getDatatype() const {
  Ptr<Datatype> datatype = resolvedField->getDatatype();
  //TODO: проверку нужно обобщить и вынести в SelectedField
  if (datatype && datatype->isNull())
    return Datatype::mkInteger();
  return datatype;
}


Assignment::Assignment(CLoc l, LValue *_lValue, PlExpr *_assignedExpr)
  : GrammarBase(l), lValue(_lValue), assignedExpr(_assignedExpr)
{
  if (!lValue || !assignedExpr)
    throw 999;
  if (l.beginedFrom(881602,2))
    cout << "";
}

Assignment::Assignment(CLoc l, Ptr<Id> _lvalue, Ptr<Id> _rvalue)
  : GrammarBase(l),
    lValue(new LValue(getLLoc(), _lvalue)),
    assignedExpr(new RefExpr(getLLoc(), _rvalue))
{
}

void Assignment::replaceChildsIf(ExprTr tr) { replace(tr, lValue, assignedExpr); }

SemanticTree* Assignment::removeThisSnode()
{
  SemanticTree *parent = 0;
  SemanticTree *currentNode = 0;
  if ((currentNode = getSemanticNode())) {
     parent = currentNode->getParent();
  }

  if (!currentNode || !parent)
    throw 999;
  // remove subtree of this semantic node
//  parent->deleteChild(currentNode);
  return parent;
}

void Assignment::unwrapStructuredLValue(VariableField *var, list<Ptr<StatementInterface> > &stmts, list<Ptr<StatementInterface> >::iterator &it) {
  Ptr<IdEntitySmart> srcEntity;
  if (assignedExpr->toSimpleFunctionCall()) {
    EntityFields flds;
    var->getFieldsRecursive(flds);
    if (flds.empty())
      throw 999;

    Ptr<LValueIntoList> newIntoList = new LValueIntoList(lValue->getLLoc(), lValue->lEntity());
    newIntoList->fields.swap(flds);
    SemanticTree *n = getSemanticNode();
    if (!n)
      throw 999;
    n->addChild(newIntoList->toSTree());
    lValue = newIntoList.object();
    return;
  }
  else if (RefExpr *ref = assignedExpr->toSelfRefExpr()) {
    srcEntity = ref->reference;
    EntityFields lfields, rfields;
    {
      VariableCursorFields *rvar = 0;
      VariableCursorFields *lvar = 0;
      ResolvedEntity *rdef, *ldef;
      if (!(rdef = ref->refDefinition()         ) || !(rvar = rdef->toSelfVariableCursorFields()) ||
          !(ldef = lValue->refDefinition()) || !(lvar = ldef->toSelfVariableCursorFields()))
        return;

      rvar->getFieldsRecursive(rfields);
      lvar->getFieldsRecursive(lfields);
      if (rfields.empty() || rfields.size() != lfields.size())
        throw 999;
    }

    Ptr<Assignment> tmp = this;
    (void)tmp;
    Ptr<IdEntitySmart> dstEntity = tmp->lValue->lEntity();
    dstEntity->erase(dstEntity->begin());


    EntityFields::iterator lIt = lfields.begin();
    EntityFields::iterator rIt = rfields.begin();

    if (!ownerBlock || !ownerBlock->getSemanticNode())
      throw 999;
    Ptr<LevelResolvedNamespace> ownerNamespace = ownerBlock->getSemanticNode()->childNamespace;
    if (!ownerNamespace)
      throw 999;

    SemanticTree *parent = removeThisSnode();

    auto createStructureAssignment = [&]() -> Ptr<Assignment> {
      Ptr<IdEntitySmart> lEntity = new IdEntitySmart(*dstEntity, *lIt);
      Ptr<IdEntitySmart> srcReference = new IdEntitySmart(*srcEntity, *rIt);
      FLoc l = getLLoc();
      Ptr<Assignment> assignment = new Assignment(l, new LValue(l, lEntity), new Sm::RefExpr(l, srcReference));
      parent->addChild(assignment->toSTree());
      return assignment;
    };

    SemanticTree::Childs::iterator firstAssignmentIt;
    SemanticTree::Childs::iterator lastAssignmentIt;
    {
      Ptr<Assignment> assignment = createStructureAssignment();
      *it = assignment.object();
      ++it;
      firstAssignmentIt = assignment->getSemanticNode()->getPositionInParent();
      lastAssignmentIt = next(firstAssignmentIt);
    }

    ++lIt; ++rIt;
    for (; lIt != lfields.end() && rIt != rfields.end(); ++lIt, ++rIt) {
      // создать новые Assignmentы и положить их в stmts
      Ptr<Assignment> assignment = createStructureAssignment();
      lastAssignmentIt = next(assignment->getSemanticNode()->getPositionInParent());
      stmts.insert(it, Ptr<StatementInterface>(assignment.object()));
    }

    Sm::collectEqualsDeclaration(firstAssignmentIt, lastAssignmentIt, ownerNamespace);
    return;
  }
  throw 999;
}

void Assignment::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }

SemanticTree *Assignment::toSTreeBase() const {
  SemanticTree *n = new SemanticTree(SCathegory::Assignment);
  n->unnamedDdlEntity = (Assignment*)this;
  CTREE(lValue);
  CTREE(assignedExpr);
  return n;
}

Ptr<LValue> Assignment::lvalue() { return lValue; }

SqlStatementInterface::SqlStatementInterface() {}

void SqlStatementInterface::translate(Codestream &str) { Translator::translate(str); }

SemanticTree *SqlStatementInterface::getSemanticNode() const { return ResolvedEntity::getSemanticNode(); }

void BlockPlSql::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, statements, exceptionHandlers); }

Ptr<IdEntitySmart> SemanticTree::extractMajorDefinitionReference() {
  if (!unnamedDdlEntity) {
    cout << "error: unnamed reference is not set: " << nAssert(referenceName_)->entity()->getLLoc()
         << " " << referenceName_->entity()->toNormalizedString() << endl;
    return 0;
  }
  if (RefExpr *refExpr = unnamedDdlEntity->toSelfRefExpr())
    return refExpr->reference;
  else if (LValue *lv = unnamedDdlEntity->toSelfLValue())
    return lv->lEntity();
  else if (Sm::FunctionCall *fc = unnamedDdlEntity->toSelfFunctionCall())
    return fc->reference();
  else
    throw 999; // Необходимо контроллировать все варианты.
}

void StatementInterface::buildStatementsTree(StatementsTree &lvl) {
  lvl.addChild(new StatementsTree(this));
}

bool Sm::Return::getRetStatementsToCast(Ptr<Datatype> &t, RetStmtsToCast &retStmtsToCast) {
  if (expr)
    if (Ptr<Datatype> b = expr->getDatatype()) {
      CastCathegory cat = t->getCastCathegory(b, true);
      if (cat.explicitInReturn())
        retStmtsToCast.push_back(Sm::BlockPlSql::RetStmtsToCast::value_type(cat, (Return*)this));
    }
  return true;
}


bool Sm::Return::castReturnStatementsToDatatype(Ptr<Datatype> &rettype) {
  static const auto delayDeleted = [](Ptr<PlExpr> &expr) -> Ptr<PlExpr>& {
    if (expr->strongRef > 2)
      return expr;
    syntaxerContext.model->delayDeletedExpressions.push_back(expr);
    return expr;
  };

    cout << "";
  if (expr)
    if (Ptr<Datatype> castedType = expr->getDatatype()) {
      if (rettype && rettype->isRefCursor() && castedType->isRefCursor()) // TODO: нужно реализовать поддержку этого случая для рефкурсоров
        return true;
      Ptr<PlExpr> oldExpr = expr;
      CastCathegory v = rettype->getCastCathegory(castedType, true);
      // преобразовывать, если:
      // Преобразования между типами нет, или известно что оно нужно явное (3,-3,0)
      // если rettype короче castedType

      if (v.explicitInReturn()) {
        v.setProcCastState();
        v.setCastReturn();
        delayDeleted(expr);
        CommonDatatypeCast::castAndReplace(true, expr, castedType, rettype, v);
      }

      if (expr == oldExpr && (rettype->isInt() || rettype->isSmallint())) {
        if (RefAbstract *sqlExpr = expr->toSelfRefAbstract())
          if (sqlExpr->reference->majorObjectRef(NULL)) {
            v.setProcCastState();
            v.setCastReturn();
            delayDeleted(expr);
            CommonDatatypeCast::castAndReplace(true, expr, Datatype::mkNumber(), rettype, v);
          }
      }
    }
  return true;
}


bool Sm::Return::getMinimalCommonDatatypeOfRetstatement(Ptr<Datatype> &dst, Ptr<Datatype> &funRettype) {
  if (beginedFrom(855927))
    cout << "";
  if (expr) {
    if (funRettype->isCharVarchar() && expr->isEmptyId())
      return true;
    Ptr<Datatype> b = expr->getReducedRettype();
    if (!b)
      b = expr->getDatatype();

    if (b && !Datatype::getMaximal(dst, b.object(), dst.object(), true))
      if (!Datatype::castCathegory(funRettype, dst.object(), true).implicit()) // возможно нужно implicit in return - более сильное ограничение
        if ((Datatype::castCathegory(funRettype, b, true).implicitInReturn())) {
          dst = b;
          // Иначе -  В Return не удается получить общий тип в силу того,
          // что неявного преобразования для него нет (для Линтер),
          // ни между lhs и rhs, ни между кем-то из {lhs,rhs} и возвращаемым типом функции
        }
  }
  return true;
}


DeleteFrom::DeleteFrom(CLoc l, int _fromKeyword, ChangedQueryEntity *_name, Id *_alias, WhereClause *_whereClause, ReturnInto *_returnInto)
  : GrammarBase(l), fromKeyword(_fromKeyword), name(_name), alias(_alias),
    whereClause(_whereClause), returnInto(_returnInto) {}

void DeleteFrom::replaceChildsIf(ExprTr tr) { replace(tr, name, alias, whereClause, returnInto); }

void DeleteFrom::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementDeleteFrom, this);
  setSemanticNode(node);
  node->nametype = SemanticTree::NEW_LEVEL;
  n_src->addChildForce(node);
  SemanticTree *n = name->toSTree();
  node->addChildForce(n);
  if (alias)
    n->alias(alias->toSNodeDecl(SCathegory::ChangedQueryEntity, name.object()));
  n->cathegory = SCathegory::SqlStatement;
  n->unnamedDdlEntity = name.object();
  CTREE(whereClause);
  CTREE(returnInto);
}

insert::MultipleConditionalInsert::MultipleConditionalInsert(CLoc l, Condition *_values, Subquery *_subquery)
  : GrammarBase(l), condition(_values), subquery(_subquery) {}

void insert::MultipleConditionalInsert::collectSNode(SemanticTree *n) const {
  setSemanticNode(n);
  (void)n; CTREE(condition); CTREE(subquery);
}

void insert::MultipleConditionalInsert::replaceChildsIf(ExprTr tr) { replace(tr, condition, subquery); }

Commit::Commit(CLoc l, Id *_commitComment)
  : GrammarBase(l), commitComment(_commitComment) {}

void Commit::collectSNode(SemanticTree *) const {}

void Commit::replaceChildsIf(ExprTr tr) { replace(tr, commitComment); }

SelectStatement::SelectStatement(CLoc l, Subquery *_subquery)
  : GrammarBase(l), subquery(_subquery)
{
  if (!subquery)
    throw 999;
  subquery->isSqlStatementRoot = true;

  subquery->setFldDefPosOnRootQuery();
}

void SelectStatement::replaceChildsIf(ExprTr tr) { replace(tr, subquery); }

void SelectStatement::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementSelect, this);
  n_src->addChildForce(node);
  subquery->collectSNode(node);
}

Savepoint::Savepoint(CLoc l, Id *_savepoint)
  : GrammarBase(l), savepoint(_savepoint) { if (savepoint) savepoint->definition(this); }

void Savepoint::replaceChildsIf(ExprTr tr) { replace(tr, savepoint); }

void Savepoint::collectSNode(SemanticTree *n) const {
  SemanticTree *node = savepoint->toSNodeDef(SCathegory::Savepoint, this);
  setSemanticNode(node);
  n->addChild(node);
}

Rollback::Rollback(CLoc l, Ptr<Id> _savepoint) : GrammarBase(l), savepoint(_savepoint) {}

void Rollback::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementRollback, this);
  n_src->addChildForce(node);
  if (savepoint)
    node->addChild(savepoint->toSNodeRef(SCathegory::Savepoint));
}

void Rollback::replaceChildsIf(ExprTr tr) { replace(tr, savepoint); }

LockTable::LockTable(CLoc l, List<Id2> *_tables, lock_table::LockMode _lockMode, lock_table::WaitMode _waitMode)
  : GrammarBase(l), tables(_tables), lockMode(_lockMode), waitMode(_waitMode) {}

void LockTable::replaceChildsIf(ExprTr tr) { replace(tr, tables); }

void LockTable::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementLockTable, this);
  setSemanticNode(node);
  n_src->addChildForce(node);
  if (tables)
    for (List<Id2>::const_iterator it = tables->begin(); it != tables->end(); ++it)
      node->addChild(it->object()->toSNodeRef(SCathegory::Table));
}

Transaction::Transaction(CLoc l, transaction::TransactionType _transactionType, Id *_name)
  : GrammarBase(l), transactionType(_transactionType), name(_name) {}

void Transaction::collectSNode(SemanticTree *n_src) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementTransaction, this);
  n_src->addChildForce(node);

  if (name)
    node->addChild(name->toSNodeDef(SCathegory::Transaction, this));
}

void Transaction::replaceChildsIf(ExprTr tr) { replace(tr, name); }

void Sm::Transaction::linterDefinition(Codestream &str) {
  str << "begin transaction;" << s::Comment() << " ";
  if (name)
    str << name->toNormalizedString() << " ";
  switch (transactionType) {
    case transaction::READ_ONLY:                     str << "READ ONLY";
    case transaction::READ_WRITE:                    str << "READ WRITE";
    case transaction::ISOLATION_LEVEL_SERIALIZABLE:  str << "ISOLATION LEVEL SERIALIZABLE";
    case transaction::ISOLATION_LEVEL_READ_COMMITED: str << "ISOLATION LEVEL READ COMMITED";
    case transaction::USE_ROLLBACK_SEGMENT:          str << "USE ROLLBACK SEGMENT";
  }
}

insert::SingleInsert::SingleInsert(CLoc l, insert::Into *_into, insert::InsertFrom *_data)
  : GrammarBase(l), into(_into), data(_data)
{
  if (!into || !data)
    throw 999;
}

void insert::SingleInsert::replaceChildsIf(ExprTr tr) { replace(tr, into, data); }

void insert::SingleInsert::collectSNode(SemanticTree *n_src) const {
  SemanticTree *n = new SemanticTree(SCathegory::StatementSingleInsert, this);
  setSemanticNode(n);
  n->nametype = SemanticTree::NEW_LEVEL;
  n_src->addChildForce(n);
  Sm::SemanticTree *node = into->collectSNode(n);
  data->collectSNode(node);
}

insert::InsertValues::InsertValues(CLoc l, insert::Into *_into, insert::InsertingValues *_values)
  : GrammarBaseSmart(l), into(_into), values(_values) {}

void insert::InsertValues::replaceChildsIf(ExprTr tr) { replace(tr, into, values); }

void insert::InsertValues::collectSNode(SemanticTree *n) const { (void)n; CTREE(into); CTREE(values); }

insert::conditional_insert::InsertWhenThen::InsertWhenThen(CLoc l, PlExpr *_when, ThenValues *_thenValues)
  : GrammarBaseSmart(l), when(_when), thenValues(_thenValues) {}

void insert::conditional_insert::InsertWhenThen::replaceChildsIf(ExprTr tr) { replace(tr, when, thenValues); }

void insert::conditional_insert::InsertWhenThen::collectSNode(SemanticTree *n) const { CTREE(when); CollectSNode(n, thenValues); }

insert::conditional_insert::InsertConditional::InsertConditional(CLoc l, insert::conditional_insert::AllOrFirst _spec, List<insert::conditional_insert::InsertWhenThen> *_whenThenClauses, List<insert::InsertValues> *_elseValuesInsert)
  : GrammarBaseSmart(l), spec(_spec), whenThenClauses(_whenThenClauses), elseValuesInsert(_elseValuesInsert) {}

void insert::conditional_insert::InsertConditional::collectSNode(SemanticTree *n) const { CollectSNode(n, whenThenClauses); CollectSNode(n, elseValuesInsert); }

void insert::conditional_insert::InsertConditional::replaceChildsIf(ExprTr tr) { replace(tr, whenThenClauses, elseValuesInsert); }

MergeUpdate::MergeUpdate(CLoc l, List<MergeFieldAssignment> *_fieldsAssignments, PlExpr *_where, PlExpr *_deleteWhere)
  : GrammarBaseSmart(l), fieldsAssignments(_fieldsAssignments), where(_where), deleteWhere(_deleteWhere) {}

void MergeUpdate::replaceChildsIf(ExprTr tr) { replace(tr, fieldsAssignments, where, deleteWhere); }

void MergeUpdate::collectSNode(SemanticTree *n) const { CollectSNode(n, fieldsAssignments); CTREE(where); CTREE(deleteWhere); }

insert::MultipleValuesInsert::MultipleValuesInsert(CLoc l, List<insert::InsertValues> *_values, Subquery *_subquery)
  : GrammarBase(l), values(_values), subquery(_subquery) {}

void insert::MultipleValuesInsert::replaceChildsIf(ExprTr tr)  { replace(tr, values, subquery); }

void insert::MultipleValuesInsert::collectSNode(SemanticTree *n) const {
  setSemanticNode(n);
  CollectSNode(n, values); CTREE(subquery);
}

MergeInsert::MergeInsert(CLoc l, List<Id> *_fields, List<SqlExpr> *_values, PlExpr *_where)
  : GrammarBaseSmart(l), fields(_fields), values(_values), where(_where) {}

void MergeInsert::replaceChildsIf(ExprTr tr) { replace(tr, fields, values, where); }

void MergeInsert::collectSNode(SemanticTree *n) const {
  if (fields)
    for (List<Id>::const_iterator it = fields->begin(); it != fields->end(); ++it)
      n->addChild(it->object()->toSNodeRef(SCathegory::Field));
  CollectSNode(n, values);
  CTREE(where);
}

Update::Update(CLoc l, ChangedQueryEntity *_entity, Id *_alias, update::SetClause *_setClause, WhereClause *_where, ReturnInto *_returnInto)
  : GrammarBase(l), entity(_entity), alias(_alias), setClause(_setClause),
    where(_where), returnInto(_returnInto) {}

void Update::replaceChildsIf(ExprTr tr) { replace(tr, entity, alias, setClause, where, returnInto); }

void Update::collectSNode(SemanticTree *n) const {
  n->addChild(toSTree());
}

SemanticTree *Update::toSTreeBase() const {
  if (entity) {
    SemanticTree *node = new SemanticTree(SCathegory::Update, SemanticTree::NEW_LEVEL);
    node->unnamedDdlEntity = const_cast<Update*>(this);
    SemanticTree *n = entity->toSTree();
    node->addChildForce(n);
    if (alias)
      n->alias(alias->toSNodeDecl(SCathegory::ChangedQueryEntity, entity.object()));
    n->cathegory = SCathegory::SqlStatement;
    n->unnamedDdlEntity = entity.object();
    CTREE(setClause);
    CTREE(where);
    CTREE(returnInto);
    return node;
  }
  return 0;
}

insert::Into::Into(CLoc l, Ptr<ChangedQueryEntity> _entity, Ptr<Id> _alias, Ptr<List<SqlExpr> > _fields)
  : GrammarBaseSmart(l), entity(_entity), alias(_alias), fields(_fields) {}

insert::Into::~Into() {}

void insert::Into::replaceChildsIf(ExprTr tr) { replace(tr, entity, alias, fields); }


SemanticTree *Sm::insert::Into::collectSNode(SemanticTree *parent) const {
  if (entity) {
    SemanticTree *node = entity->toSTree();
    parent->addChildForce(node);
    if (alias)
      node->alias(alias->toSNodeDecl(SCathegory::ChangedQueryEntity, entity.object()));
    node->cathegory = SCathegory::SqlStatement;
    node->nametype = SemanticTree::NEW_LEVEL;
    node->unnamedDdlEntity = entity.object();
    SemanticTree *intoNode = new SemanticTree(SCathegory::Into);
    intoNode->setIsList();
    if (fields)
      for (const Ptr<SqlExpr> &it : *fields)
        it->collectSNode(intoNode);
    node->addChild(intoNode);
    return node;
  }
  return 0;
}

update::FieldsFromSubquery::FieldsFromSubquery(CLoc l, EntityList *_fields, Ptr<Subquery> q)
  : GrammarBase(l), fields(_fields), subquery(q) {}

void update::FieldsFromSubquery::collectSNode(SemanticTree *n) const {
  if (fields)
    for (const EntityList::value_type &it : *fields)
      n->addChild(it->toSTreeRef(SCathegory::Field));
  CTREE(subquery);
}

void Sm::update::FieldsFromSubquery::sqlDefinition(Codestream &str) {
  bool isNotFirst = false;
  int dist = 0;
  if (fields)
    for (EntityList::value_type &it : *fields) {
      str << s::comma(&isNotFirst) << *it << " = " << s::obracket;
      if (subquery)
        subquery->sqlDefinitionForNthField(str, dist);
      str << s::cbracket;
      ++dist;
    }
}

void update::FieldsFromSubquery::replaceChildsIf(ExprTr tr) { replace(tr, fields, subquery); }

update::SetClause::~SetClause() {}

update::SetRowRecord::SetRowRecord(CLoc l, Ptr<Id> _rowRecord) : GrammarBaseSmart(l)
{
  rowRecord = new Sm::RefExpr(l, nAssert(_rowRecord));
}

update::SetRowRecord::~SetRowRecord() {}

void update::SetRowRecord::collectSNode(SemanticTree *n) const {
  if (rowRecord)
    rowRecord->collectSNode(n);
//    n->addChild(rowRecord->toSNodeRef(SCathegory::Record));
}

void update::SetRowRecord::replaceChildsIf(ExprTr tr) {
  replace(tr, updatedEntity, rowRecord);
}

void Sm::update::SetRowRecord::sqlDefinition(Sm::Codestream &str) {
  EntityFields flds;
  if (updatedEntity)
    updatedEntity->getFields(flds);
  EntityFields rowFlds;
//  if (ResolvedEntity* d = rowRecord->definition())
  if (ResolvedEntity* d = rowRecord->refDefinition())
    d->getFields(rowFlds);
  bool isNotFirst = false;
  int pos = 0;
  for (EntityFields::value_type &f : flds) {
    str << s::comma(&isNotFirst) << *f << " = ";
//    IdEntitySmart ent;

//    if (pos < int(rowFlds.size()))
//      ent.push_back(rowFlds[pos]);
//    ent.push_back(rowRecord);
//    str << ent;
    if (pos < int(rowFlds.size())) {
      IdEntitySmart ent(*(rowRecord->reference), rowFlds[pos]);
      str << ent;
    }
    else
      str << *(rowRecord->reference);

    ++pos;
  }
}

update::SetUpdatingList::SetUpdatingList(CLoc l, Ptr<List<UpdatingItemType> > _updatingList) : GrammarBaseSmart(l), updatingList(_updatingList) {}

update::SetUpdatingList::~SetUpdatingList() {}

void update::SetUpdatingList::collectSNode(SemanticTree *n) const { CollectSNode(n, updatingList); }

void update::SetUpdatingList::sqlDefinition(Codestream &str) { str << updatingList; }

void update::SetUpdatingList::replaceChildsIf(ExprTr tr) { replace(tr, updatedEntity, updatingList); }

insert::InsertFromValues::InsertFromValues(CLoc l, Ptr<insert::InsertingValues> val, Ptr<ReturnInto> ret)
  : GrammarBaseSmart(l), value(val), returning(ret) {}

void insert::InsertFromValues::replaceChildsIf(ExprTr tr) { replace(tr, value, returning); }

void insert::InsertFromValues::collectSNode(SemanticTree *node) const {
  if (returning)
    returning->collectSNode(node);
  if (value) {
    SemanticTree *n = new SemanticTree(SCathegory::InsertingValues, SemanticTree::NEW_LEVEL, 0);
    node->addChildForce(n);
    value->collectSNode(n);
  }
}

update::FieldFromExpr::FieldFromExpr(CLoc l, Ptr<IdEntitySmart> _field, Ptr<SqlExpr> _expr)
  : GrammarBase(l),
    field(new Sm::RefExpr(l, _field)),
    expr(_expr) { /* checkIdEntitySmart(field); */ }


update::FieldFromExpr::FieldFromExpr(CLoc l, Ptr<SqlExpr> _field, Ptr<SqlExpr> _expr)
  : GrammarBase(l), field(_field), expr(_expr) {}


void update::FieldFromExpr::replaceChildsIf(ExprTr tr) { replace(tr, field, expr); }


void update::FieldFromExpr::collectSNode(SemanticTree *n) const {
  /*n->addChild(field->toSTreeRef(SCathegory::Field));*/ CTREE(field); CTREE(expr);
}

MergeFieldAssignment::MergeFieldAssignment(CLoc l, Ptr<IdEntitySmart> _field, Ptr<SqlExpr> _assignment)
  : GrammarBaseSmart(l), field(_field), assignment(_assignment) { checkIdEntitySmart(field); }

void MergeFieldAssignment::replaceChildsIf(ExprTr tr) { replace(tr, field, assignment); }

void MergeFieldAssignment::collectSNode(SemanticTree *n) const { n->addChild(field->toSTreeRef(SCathegory::Field)); CTREE(assignment); }

void insert::InsertingRecordValues::replaceChildsIf(ExprTr tr) { replace(tr, record); }

insert::InsertingRecordValues::InsertingRecordValues(CLoc l, Ptr<IdEntitySmart> _record)
  : GrammarBaseSmart(l)
{
  if (_record)
    record = new Sm::RefExpr(_record->back()->getLLoc(), _record);
}

void insert::InsertingRecordValues::collectSNode(SemanticTree *n) const {
  if (SemanticTree *node = record->toSTree()) {
    node->setInsertingValue();
    n->addChild(node);
  }
}

insert::InsertFromSubquery::InsertFromSubquery(CLoc l, Ptr<Subquery> subq)
  : GrammarBaseSmart(l), subquery(subq) {}

void insert::InsertFromSubquery::replaceChildsIf(ExprTr tr) { replace(tr, subquery); }

void insert::InsertFromSubquery::collectSNode(SemanticTree *n) const { (void)n; CTREE(subquery); }

insert::InsertingExpressionListValues::InsertingExpressionListValues(CLoc l, Ptr<List<SqlExpr> > _exprList)
  : GrammarBaseSmart(l), exprList(_exprList) {}

void insert::InsertingExpressionListValues::sqlDefinition(Codestream &str) {
  if (exprList) {
    str << s::OBracketInsert();
    bool isNotFirst = false;
    for (Ptr<SqlExpr> &v : *exprList)
      str << s::comma(&isNotFirst) << s::ocolumn() << v << s::ccolumn();
    str << s::CBracketInsert();
  }
}

void insert::InsertingExpressionListValues::replaceChildsIf(ExprTr tr) { replace(tr, exprList); }

void insert::InsertingExpressionListValues::collectSNode(SemanticTree *n) const {
  if (n) {
    bool isEmpty = n->childs.empty();
    SemanticTree::Childs::iterator it = isEmpty ? n->childs.end() : --(n->childs.end());
    CollectSNode(n, exprList);
    if (n->childs.empty())
      return;
    if (isEmpty)
      it = n->childs.begin();
    else
      ++it;

    for (; it != n->childs.end(); ++it)
      (*it)->setInsertingValue();
  }
}


ResolvedEntity* Merge::getNextDefinition() const {
  return mergedEntity.object();
}


bool Merge::getFieldRef(Ptr<Sm::Id> &field) {
  return mergedEntity->getFieldRef(field);
}


bool Merge::getFields(EntityFields &fields) const {
  return mergedEntity->getFields(fields);
}


bool Merge::getFieldRefByFromList(Ptr<Id> &field, SemanticTree *reference) {
  if ((fromNode && resolveAsTable(reference, field, fromNode)) ||
      (resolveAsFromItemField(field, fromList)) ||
      resolveAsSquotedField (field))
    return true;
  return false;
}


Merge::Merge(CLoc l, From *ent, From *_using, PlExpr *onCond, MergeUpdate *upd, MergeInsert *ins)
  : GrammarBase(l),
    mergedEntity(Sm::nAssert(ent)),
    usingClause(_using),
    onCondition(onCond),
    matchedUpdate(upd),
    notMatchedInsert(ins)
{
  fromList  = new FromList();
  fromList->push_back(mergedEntity);
  fromList->push_back(usingClause);
}

void Merge::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }

Sm::IsSubtypeValues Sm::Merge::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return mergedEntity->isSubtype(supertype, plContext);
}

void Sm::Merge::linterReference(Sm::Codestream &str) {
  FromSingle *s = Sm::nAssert(mergedEntity->toSelfFromSingle());
  if (s->alias)
    str << s->alias;
  else
    str << s->reference;
}

void Merge::replaceChildsIf(ExprTr tr) {
  replace(tr, mergedEntity, usingClause, onCondition, matchedUpdate, notMatchedInsert);
}


Ptr<Datatype> Merge::getDatatype() const { return mergedEntity->getDatatype(); }

ChangedQueryEntity::ChangedQueryEntity(CLoc l) : GrammarBase(l) {}

ChangedQueryEntity::ChangedQueryEntity() {}

ChangedQueryRestricted::ChangedQueryRestricted(CLoc l, Subquery *_subquery, view::ViewQRestriction *_restriction)
  : GrammarBase(l), subquery(_subquery), restriction(_restriction) {}


bool Sm::ChangedQueryRestricted::getFieldRef(Ptr<Id> &field) {
  if (Sm::checkToRowidPseudocolumn(field))
    return true;
  return subquery && subquery->getFieldRef(field);
}

bool ChangedQueryRestricted::getFields(EntityFields &fields) const { return subquery && subquery->getFields(fields); }

ResolvedEntity *ChangedQueryRestricted::getNextDefinition() const { return subquery ? subquery->getNextDefinition() : (ResolvedEntity*)0; }

Sm::IsSubtypeValues ChangedQueryRestricted::isSubtype(ResolvedEntity *supertype, bool plContext) const { return subquery->isSubtype(supertype, plContext); }

Ptr<Datatype> ChangedQueryRestricted::getDatatype() const { return subquery ? subquery->getDatatype() : Ptr<Datatype>(); }

void ChangedQueryRestricted::sqlDefinition(Codestream &str) { str << subquery; }

void ChangedQueryRestricted::replaceChildsIf(ExprTr tr)  { replace(tr, subquery); }

SemanticTree *ChangedQueryRestricted::toSTreeBase() const {
  SemanticTree *node = new SemanticTree(SCathegory::ChangedQueryEntity, SemanticTree::NEW_LEVEL);
  node->unnamedDdlEntity = (ChangedQueryRestricted*)this;
  CTREE2(node, subquery)
      return node;
}

ChangedQueryEntityRef::ChangedQueryEntityRef(CLoc l, Id2 *_entityRef, Id *_dblink)
  : GrammarBase(l), entityRef(_entityRef), dblink(_dblink)
{
  if (!_entityRef)
    throw 999;
}

Ptr<Id> ChangedQueryEntityRef::getName() const { return entityRef->entity(); }

bool Sm::ChangedQueryEntityRef::getFieldRef(Ptr<Id> &field) {
  if (Sm::checkToRowidPseudocolumn(field))
    return true;
  ResolvedEntity *d;
  return entityRef && (d = entityRef->definition()) && d->getFieldRef(field);
}

Sm::IsSubtypeValues ChangedQueryEntityRef::isSubtype(ResolvedEntity *supertype, bool plContext) const { return entityRef->isSubtype(supertype, plContext); }


void ChangedQueryEntityRef::replaceChildsIf(ExprTr tr) { replace(tr, entityRef); }

bool ChangedQueryEntityRef::getFields(EntityFields &field) const { return entityRef && entityRef->getFields(field); }

ResolvedEntity *ChangedQueryEntityRef::getNextDefinition() const { return entityRef ? entityRef->getNextDefinition() : nullptr; }

Ptr<Datatype> ChangedQueryEntityRef::getDatatype() const { return entityRef ? entityRef->getDatatype() : Ptr<Datatype>(); }

SemanticTree *ChangedQueryEntityRef::toSTreeBase() const {
  return entityRef->toSNodeRef(SCathegory::TableViewMaterializedView);
}


void ChangedQueryEntityRef::sqlDefinition(Codestream &str) {
  if (entityRef && entityRef->size() == 1)
    if (ResolvedEntity *d = entityRef->entity()->unresolvedDefinition())
      if (d->toSelfVariable() || d->toSelfFunctionArgument()) // <- translateId2AsDynamicField(str, entityRef, this)) return;
        throw 999; // убрал, т.к. динамический SQL транслируется теперь иначе

  if (entityRef && entityRef->size() == 1)
    if (UserContext *user = entityRef->definition()->userContext())
      str << user->getName() << ".";
  str << entityRef;

  if (dblink)
    str << s::name << s::OMultiLineComment() << "UNSUPPORTED DBLINK: " << entityRef << '@' << dblink << s::CMultiLineComment();
}

void ChangedQueryEntityRef::sqlReference(Codestream &str) {
  str << entityRef;
  if (dblink)
    str << s::name << s::OMultiLineComment() << "UNSUPPORTED DBLINK: " << entityRef << '@' << dblink << s::CMultiLineComment();
}

ChangedQueryTableCollectionExpr::ChangedQueryTableCollectionExpr(CLoc l, SqlExpr *_expr, bool _hasJoin)
  : GrammarBase(l), expr(_expr), hasJoin(_hasJoin) {}

bool Sm::ChangedQueryTableCollectionExpr::getFieldRef(Ptr<Id> &field) {
  if (Sm::checkToRowidPseudocolumn(field))
    return true;
  return expr && expr->getFieldRef(field);
}

bool ChangedQueryTableCollectionExpr::getFields(EntityFields &fields) const { return expr && expr->getFields(fields); }

ResolvedEntity *ChangedQueryTableCollectionExpr::getNextDefinition() const { return expr ? expr->getNextDefinition() : (ResolvedEntity*)0; }

Sm::IsSubtypeValues ChangedQueryTableCollectionExpr::isSubtype(ResolvedEntity *supertype, bool plContext) const { return expr->isSubtype(supertype, plContext); }

Ptr<Datatype> ChangedQueryTableCollectionExpr::getDatatype() const { return (expr) ? expr->getDatatype() : Ptr<Datatype>(); }

void ChangedQueryTableCollectionExpr::replaceChildsIf(ExprTr tr) { replace(tr, expr); }

SemanticTree *ChangedQueryTableCollectionExpr::toSTreeBase() const {
  SemanticTree *node = new SemanticTree(SCathegory::ChangedQueryTableCollectionExpr, SemanticTree::NEW_LEVEL);
  node->unnamedDdlEntity = (ChangedQueryTableCollectionExpr*)this;
  CTREE2(node, expr)
      return node;
}

WhereClause::WhereClause(CLoc l, PlExpr *_condition)
  : GrammarBaseSmart(l), condition(_condition) {}

WhereClause::WhereClause(CLoc l, Id *_whereCurrentOfCursorName)
  : GrammarBaseSmart(l), whereCurrentOfCursorName(_whereCurrentOfCursorName) {}

WhereClause::~WhereClause() {}

void WhereClause::replaceChildsIf(ExprTr tr) { replace(tr, condition, whereCurrentOfCursorName); }

void WhereClause::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::WhereClauseDML);
  n->addChildForce(node);
  if (condition)
    condition->collectSNode(node);
  else if (whereCurrentOfCursorName)
    node->addChild(whereCurrentOfCursorName->toSNodeRef(SCathegory::Cursor));
}

void Sm::WhereClause::sqlDefinition(Sm::Codestream &str) {
  if (condition && whereCurrentOfCursorName)
    throw 999;
  if (condition)
    str << condition;
  else if (whereCurrentOfCursorName)
    str << "CURRENT OF" << s::name << whereCurrentOfCursorName;
}

ReturnInto::ReturnInto(CLoc l, Ptr<List<SqlExpr> > _exprList, Ptr<IntoCollections> _intoCollections)
  : GrammarBase(l), exprList(_exprList), intoCollections(_intoCollections), qMode(FOR_DELETE) {}

void ReturnInto::collectSNode(SemanticTree *n) const { n->addChild(toSTreeBase()); }

SemanticTree *ReturnInto::toSTreeBase() const {
  SemanticTree *node = new SemanticTree(SCathegory::ReturnInto);
  CollectSNode(node, exprList);
  for (auto &it : *intoCollections)
    node->addChild(it->toSTree());
  semanticNode = node;
  return node;
}

Ptr<Sm::Datatype> ReturnInto::getDatatype() const {
  if (!thisDatatype) {
    Ptr<Id> emptyName = new Id();
    emptyName->setIsEmpty();
    emptyName->definition(this);
    emptyName->semanticNode(getSemanticNode());
    thisDatatype = new Datatype(emptyName);
    thisDatatype->setSemanticNode(getSemanticNode());
  }
  return thisDatatype;
}

bool ReturnInto::getFields(EntityFields &fields) const {
  for (auto &expr : *exprList) {
    fields.push_back(expr->toSelfRefExpr()->refEntity());
  }
  return true;
}

void BlockPlSql::traverseModelStatements(StatementActor &fun) {
  for (Declarations::value_type &v : declarations)
    v->traverseModelStatements(fun);

  traverseStatements(fun);
}

template <>
void Sm::replace(ExprTr tr, Ptr<IntoCollections> &lst) {
  replaceOnList(tr, lst);
}
