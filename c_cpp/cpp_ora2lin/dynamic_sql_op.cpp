#include "dynamic_sql_op.h"
#include "codegenerator.h"
#include "semantic_expr_select.h"
#include "semantic_expr.h"
#include "syntaxer_context.h"
#include "semantic_function.h"
#include "semantic_blockplsql.h"
#include "semantic_plsql.h"
#include "codegenerator.h"
#include "system_sysuser.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "statements_tree.h"

using namespace Sm;
extern SyntaxerContext syntaxerContext;


FunctionDynExpr::FunctionDynExpr(CLoc l, PlExpr* _expr, PlExpr* _typeSrc)
  : GrammarBase(l), expr(Sm::nAssert(_expr)), sourceForCastDatatype(_typeSrc) {}


FunctionDynExpr::FunctionDynExpr(CLoc l, PlExpr* _expr, Datatype* _typeSrc)
  : GrammarBase(l), expr(Sm::nAssert(_expr)), sourceForCastDatatype(new Sm::Cast(l, new Sm::NullExpr(l), _typeSrc)) {}


FunctionDynField::FunctionDynField(CLoc l, PlExpr *_expr, PlExpr *_typeSrc)
  : GrammarBase(l), expr(_expr), sourceForCastDatatype(_typeSrc) {}


FunctionDynField::FunctionDynField(CLoc l, PlExpr* _expr, Datatype* _typeSrc)
  : GrammarBase(l), expr(_expr), sourceForCastDatatype(new Sm::Cast(l, new Sm::NullExpr(l), _typeSrc)) {}


Ptr<Sm::Datatype> Sm::FunctionDynExpr::getDatatype() const {
  if (sourceForCastDatatype)
    return sourceForCastDatatype->getDatatype();
  return expr->getDatatype();
}


void pushMakestrTopArg(PlExpr *self, Sm::Codestream &str)
{
  if (str.state().dynamicSqlDeep_ < 2)
    str.state().queryExternalIdentficators.push_back(self);
  else
    nAssert(str.state().parentStream_)->state().queryExternalIdentficators.push_back(self);
}


void Sm::QueryEntityDyn::sqlDefinition(Sm::Codestream &str) {
  pushMakestrTopArg(dynRef, str);

  bool old = str.state().isDynamicSignOutput_;
  str.state().isDynamicSignOutput_ = true;

  str << s::querySign;

  str.state().isDynamicSignOutput_ = old;
}


void translateDynamicEntity(PlExpr *self, PlExpr *expr, Sm::Codestream &str) {
  pushMakestrTopArg(self, str);

  bool old = str.state().isDynamicSignOutput_;
  str.state().isDynamicSignOutput_ = true;

  translateAsMakestrArg(str, self->getSemanticNode(), /*def*/expr->getNextDefinition(), self, true);

  str.state().isDynamicSignOutput_ = old;
}


void Sm::FunctionDynExpr::sqlDefinition(Sm::Codestream &str) {
  translateDynamicEntity(this, expr, str);
}


void Sm::FunctionDynField::sqlDefinition(Sm::Codestream &str) {
  translateDynamicEntity(this, expr, str);
}



void FunctionDynExpr::linterDefinition(Codestream &str) {
  if (str.state().dynamicSqlDeep_)
    this->sqlDefinition(str);
  else
    expr->linterDefinition(str);
}

void FunctionDynField::linterDefinition(Codestream &str) {
  if (str.state().dynamicSqlDeep_)
    this->sqlDefinition(str);
  else
    expr->linterDefinition(str);
}

void DynTailExpr::linterDefinition(Sm::Codestream &str) {
  if (!inverse)
    str << expr << s::name << tail;
  else
    str << tail << s::name << expr;
}


DynTailExpr::DynTailExpr(CLoc l, PlExpr* _expr, PlExpr* _tail, bool _inverse)
  : GrammarBase(l),
    expr(nAssert(_expr)),
    tail(new FunctionDynField(l, nAssert(_tail))),
    inverse(_inverse)
{}


Ptr<Datatype> DynTailExpr::getDatatype() const { return expr->getDatatype(); }


void Sm::FunctionDynExpr::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new Sm::SemanticTree(SCathegory::FunctionDynExpr, this);
  setSemanticNode(n);
  node->addChildForce(n);
  expr->collectSNode(n);
  if (sourceForCastDatatype)
    sourceForCastDatatype->collectSNode(n);
}

void Sm::FunctionDynField::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new Sm::SemanticTree(SCathegory::FunctionDynField, this);
  setSemanticNode(node);
  n->addChildForce(node);
  expr->collectSNode(node);
  if (sourceForCastDatatype)
    sourceForCastDatatype->collectSNode(node);
}

void DynTailExpr::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new Sm::SemanticTree(SCathegory::FunctionDynTail_, this);
  setSemanticNode(node);
  n->addChild(node);
  expr->collectSNode(node);
  tail->collectSNode(node);
}

void Sm::FunctionDynField::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it)  {
  sAssert(!sourceForCastDatatype);
  sourceForCastDatatype->assignFieldReferenceByPosition(container, it);
}



Ptr<Sm::Datatype> Sm::FunctionDynField::getDatatype() const {
  if (sourceForCastDatatype)
    return sourceForCastDatatype->getDatatype();
  return 0;
}


bool Sm::FunctionDynField::getFields(EntityFields &fields) const {
  if (sourceForCastDatatype)
    return sourceForCastDatatype->getFields(fields);
  return false;
}

bool Sm::FunctionDynField::getFieldRef(Ptr<Id> &f) {
  if (sourceForCastDatatype)
    return sourceForCastDatatype->getFieldRef(f);
  return false;
}



Sm::SemanticTree * Sm::QueryEntityDyn::toSTreeBase() const {
  SemanticTree *baseNode = new Sm::SemanticTree(SCathegory::QueryEntityDyn, SemanticTree::NEW_LEVEL, this);
  baseNode->unnamedDdlEntity = const_cast<Sm::QueryEntityDyn*>(this);

  SemanticTree *node     = baseNode;
  if (fieldsSrc) {
    node = fieldsSrc->toSNodeRef(SCathegory::TableViewMaterializedView);
    baseNode->addChildForce(node);
  }

  setSemanticNode(node);
  dynRef->collectSNode(node);
  return baseNode;
}

Sm::QueryEntityDyn::QueryEntityDyn(CLoc l, SqlExpr *_dynRef, Id2 *_entityRef)
  : Sm::GrammarBase(l), dynRef(_dynRef), fieldsSrc(_entityRef)
{
  Sm::sAssert(!dynRef);
}


CursorDecltype::CursorDecltype(CLoc l, Ptr<Id> v, Ptr<SqlExpr> s) : GrammarBase(l), varName(v), select(s) {}

void CursorDecltype::replaceChildsIf(ExprTr tr) { replace(tr, select); }

void CursorDecltype::collectSNode(SemanticTree *n) const {
  n->addChild(varName->toSNodeRef(Sm::SCathegory::Variable));
  CTREE(select);
}

bool CursorDecltype::getFields(EntityFields &fields) const {
  return select && select->getFields(fields);
}


Ptr<Datatype>     BoolTailObj::getDatatype() const { return Datatype::mkBoolean();  }
Ptr<Datatype>     StrTailObj ::getDatatype() const { return Datatype::mkVarchar2(); }
Ptr<Sm::Datatype> NumTailObj ::getDatatype() const { return Datatype::mkNumber();   }

bool ConstructExprStmt::getFieldRef(Ptr<Id> &field) {
  if (fromNode && fromNode->declarations) {

  }
  return getFieldRefByFromList(field, field->semanticNode());
}


DeclNamespace::DeclNamespace(CLoc l, Ptr<Id> n, Ptr<DeclNamespace::FromList> lst)
  : GrammarBase(l), name(n), fromList(lst)
{
  if (!name)
    throw 999;
}


DeclNamespace::DeclNamespace(CLoc l, Ptr<Id> n, Declarations *lst)
  : GrammarBase(l), name(n), declarations(lst)
{
  if (!name)
    throw 999;
}


void DeclNamespace::collectSNode(SemanticTree *n) const {
  SemanticTree *fNode = new SemanticTree(SCathegory::From, SemanticTree::NEW_LEVEL);
  if (fromList) {
    fNode->setIsList();
    setSemanticNode(fNode);
    for (const FromList::value_type &v : *(fromList))
      if (v)
        v->collectSNode(fNode);
  }
  CollectSNode(fNode, declarations);
  setSemanticNode(fNode);
  n->addChild(fNode);
}


ConstructExprStmt::ConstructExprStmt(CLoc l, Ptr<Id> v, Ptr<Id> srcNspace, Ptr<PlExpr> e, Ptr<ConstructExprStmtContext> _ctx, bool _procMode)
  : GrammarBase(l), procMode(_procMode), var(v), srcNamespace(srcNspace), expr(e)
{
  if (!expr)
    throw 999;
  if (_ctx) {
    context = *_ctx;
    if (!context.nameOfUnionChunck.empty()) {
      sAssert(syntaxerContext.funCtx.empty());
      syntaxerContext.funCtx.top()->unionDynamicComponents[context.nameOfUnionChunck] = this;
    }
  }
  varRef = new RefExpr(l, var);
  if (!context.globalCursorId.empty())
    syntaxerContext.model->globalDynSqlCursors[context.globalCursorId].push_back(nAssert(expr->toSelfSubquery()));
}


void ConstructExprStmt::collectSNode(SemanticTree *tr) const {
  SemanticTree *parent = new SemanticTree(SCathegory::StatementConstructExpr);
  tr->addChildForce(parent);

  varRef->collectSNode(parent);
//  parent->addChild(var->toSNodeRef(SCathegory::EMPTY));


  // TODO: возможно придется еще добавлять выбираемые из неймспейса поля-алиасы
  SemanticTree *n = new SemanticTree(SCathegory::QueryBlock, SemanticTree::DECLARATION);
  setSemanticNode(n);
  n->unnamedDdlEntity = const_cast<ConstructExprStmt*>(this);

  if (fromNode)
    this->fromSNode = fromNode->getSemanticNode();

  SemanticTree *whereNode = new SemanticTree(SCathegory::WhereClause, SemanticTree::NEW_LEVEL);
  expr->collectSNode(whereNode);
  n->addChild(whereNode);
  parent->addChild(n);
}

void ConstructExprStmt::replaceChildsIf(ExprTr tr) { replace(tr, expr); }

bool ConstructExprStmt::getFieldRefByFromList(Ptr<Id> &field, SemanticTree *reference) {
  if ((fromSNode && resolveAsTable(reference, field, fromSNode)      ) ||
      (fromNode  && resolveAsFromItemField(field, fromNode->fromList)) ||
      resolveAsSquotedField (field))
    return true;
  if (fromNode && fromNode->declarations && fromSNode && fromSNode->levelNamespace)
    return fromSNode->childNamespace->findVariable(field);
  return false;
}


//void CursorFieldDecltype::collectSNode(SemanticTree *n) const {
//  for (Ptr<Id> var : *varNames)
//    n->addChild(var->toSNodeRef(Sm::SCathegory::Variable, this));
//}


//CursorFieldDecltype::CursorFieldDecltype(CLoc l, List<Id> *v)
//  : GrammarBase(l), varNames(v) { sAssert(!varNames); }

//void CursorFieldDecltype::replaceChildsIf(ExprTr) {}


bool OpenFor::getFields(EntityFields &fields) const {
  ResolvedEntity *var;
  return (select && select->getFields(fields)) ||
         (!decltypeSelectStmts.empty() && decltypeSelectStmts.front()->getFields(fields)) ||
         ((var = openForIterVariable->definition()) && var->getFields(fields));
}


DynLength::DynLength(CLoc l, Ptr<PlExpr> expr)
  : GrammarBase(l), lengthExpr(expr) {}

void DynLength::collectSNode(SemanticTree *n) const {
  lengthExpr->collectSNode(n);
}

void DynLength::linterDefinition(Sm::Codestream &str) {
  Codestream tmp;
  setTranslateSqlToStringFlags(tmp, str, SQLSTR_NEED_MAKESTR | SQLSTR_NEED_DIRECT);
  lengthExpr->sqlDefinition(tmp);
  string s = tmp.str();
  str << s.length();
}

void ConstructExprStmt::linterDefinition(Codestream &str) {
  str << s::cref(var) << " := ";
  unsigned int flags = SQLSTR_SUPRESS_DIRECT | SQLSTR_NEED_DIRECT | SQLSTR_NEED_MAKESTR;
  Sm::Codestream sqlExprStr;
  setTranslateSqlToStringFlags(sqlExprStr, str, flags); 
  if (procMode)
    sqlExprStr.procMode(CodestreamState::PROC);
  if (context.concat) {
    // sqlExprStr << s::cref(var) << " ";
    sqlExprStr.state().queryExternalIdentficators.push_back(varRef);
    sqlExprStr << s::querySign << s::name;
  }
  sqlExprStr << expr;
  str << s::otabcommalist();
  wrapSqlCodestreamIntoString((Sm::SqlExpr*)0, str, sqlExprStr, flags);
  str << s::ctabcommalist();
}


ConstructBlockStmt::ConstructBlockStmt(CLoc l, BlockPlSql *_dynamicBlock, RefAbstract *_into)
  : GrammarBase(l), dynamicBlock(Sm::nAssert(_dynamicBlock)), into(_into)
{
  dynamicBlock->isFunctionTopBody_ = true;
}

void ConstructBlockStmt::linterDefinition(Codestream &str) {
  unsigned int flags = SQLSTR_IS_EXECUTE_BLOCK | SQLSTR_NEED_DIRECT | SQLSTR_NEED_MAKESTR;
  Sm::Codestream sqlExprStr;
  setTranslateSqlToStringFlags(sqlExprStr, str, flags);
  sqlExprStr.procMode(CodestreamState::PROC);
  sqlExprStr.activatePredeclarations();
  sqlExprStr << "EXECUTE BLOCK ";
  sqlExprStr.activatePrevious();
  if (beginedFrom(132265))
    cout << "";
  sqlExprStr << dynamicBlock << s::semicolon;
  sqlExprStr.joinPredeclarations();
  sqlExprStr.join();

  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
  wrapSqlCodestreamIntoString((Sm::SqlExpr*)0, str, sqlExprStr, flags);
  str << s::ctabcommalist();
  if (into)
    str << s::name << s::subconstruct << "INTO" << s::name << into;
}


Ptr<Datatype> DynLength::getDatatype() const { return Datatype::mkInteger(); }

void DynSubquery::lazyExprInit() {
  if (expr_ || isGlobalSubquery)
    return;
  string strReferenceSubquery = nAssert(referenceSubquery)->toNormalizedString();
  FunctionContext::UnionDynamicComponents::iterator it = ctx->unionDynamicComponents.find(strReferenceSubquery);
  if (it != ctx->unionDynamicComponents.end()) {
    ConstructExprStmt* cstmt = it->second;
    expr_ = nAssert(cstmt->expr->toSelfSubquery());

    forUpdate          = expr_->forUpdate;
    orderBy            = expr_->orderBy;
    groupBy            = expr_->groupBy;
    isSqlStatementRoot = expr_->isSqlStatementRoot;
  }
  else {
    Sm::GlobalDynSqlCursors::iterator v =
        syntaxerContext.model->globalDynSqlCursors.find(strReferenceSubquery);

    if (v != syntaxerContext.model->globalDynSqlCursors.end())
      isGlobalSubquery = true;
  }
}

Ptr<Subquery> DynSubquery::expr() const {
  if (!expr_) {
    const_cast<DynSubquery*>(this)->lazyExprInit();
    return Sm::nAssert(expr_);
  }
  else
    checkConsistance();

  return expr_;
}


Ptr<Subquery> DynSubquery::singleExpr() const {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  if (isGlobalSubquery) {
    Sm::GlobalDynSqlCursors::mapped_type *q = consistantGlobalQueries();
    return nAssert(q)->front();
  }
  else
    return expr();
}


void DynSubquery::checkConsistance() const {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  if (isGlobalSubquery)
    consistantGlobalQueries();
}


void DynSubquery::translateByFetchOps(QueryExpressions *q) const {
  /*
    Если выражение используется в OpenFor запросе -
    получить все операторы fetch into
    для курсорной переменной openFor запроса
    и соответственно их into-списку выполнить приведение.
    иначе - сгенерировать исключение.
  */
  CommonDatatypeCast::CastContext ctx;
  getSemanticNode()->getExpressionContext(ctx);
  if (!ctx.openForStmt) // трансляция производится не в openFor запросе
    return;
  SemanticTree *n = ctx.openForStmt->baseCursorVariable->getSemanticNode();
  BlockPlSql *b = n->ownerBlock()->getTopBlock();

  StatementsTree t = b->buildStatementsTree();
  StatementsTree::Stmts stmts = t.getStatementsChangedEntityBeforeStmt(ctx.openForStmt->baseCursorVariable->refDefinition(), 0);
  for (StatementsTree::Stmts::value_type &v : stmts) {
    StatementInterface *s = nAssert(v->nodeStmt());
    if (Fetch *f = s->toSelfFetch())
      for (auto subqueryOp : *q)
        subqueryOp->translateSelectedFieldsDatatypesToIntoListTypes(&(f->fields));
  }
}




void DynSubquery::translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory castedCathegory) {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  if (isGlobalSubquery) {
    QueryExpressions *q = nAssert(consistantGlobalQueries());
    for (QueryExpressions::value_type v : *q)
      v->translateFieldToDatatype(i, oldT, newT, castedCathegory);
  }
  else
    return expr()->translateFieldToDatatype(i, oldT, newT, castedCathegory);
}

void DynSubquery::sqlDefinitionForNthField(Codestream &str, int fieldPos) {
  singleExpr()->sqlDefinitionForNthField(str, fieldPos);
}

Sm::Subquery::IntoList DynSubquery::intoList() const {
  if (intoList_)
    return intoList_;

  const_cast<DynSubquery*>(this)->lazyExprInit();
  Sm::sAssert(isGlobalSubquery);
  return expr()->intoList();
}

Ptr<Datatype> DynSubquery::getDatatype() const {
  return singleExpr()->getDatatype();
}


bool DynSubquery::getFields(EntityFields &fields) const {
  if (syntaxerContext.stage == SyntaxerContext::SYNTAX_ANALYZE)
    return false;
  return singleExpr()->getFields(fields);
}

bool DynSubquery::getFieldRef(Ptr<Id> &f) {
  return singleExpr()->getFieldRef(f);
}

void DynSubquery::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
  return singleExpr()->assignFieldReferenceByPosition(container, it);
}

Ptr<List<FactoringItem> > DynSubquery::pullUpFactoringList() const {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  sAssert(isGlobalSubquery);
  return expr()->pullUpFactoringList();
}

bool DynSubquery::getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference) {
  return singleExpr()->getFieldRefByFromList(field, reference);
}

void DynSubquery::setIsUnionChild() {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  if (isGlobalSubquery) {
    QueryExpressions *q = nAssert(consistantGlobalQueries());
    for (QueryExpressions::value_type v : *q)
      v->setIsUnionChild();
  }
  else
    return expr()->setIsUnionChild();
}


DynSubquery::CathegorySubquery DynSubquery::cathegorySubquery() const {
  const_cast<DynSubquery*>(this)->lazyExprInit();
  if (isGlobalSubquery) {
    QueryExpressions *q = nAssert(consistantGlobalQueries());

    CathegorySubquery c = q->front()->cathegorySubquery();
    for (QueryExpressions::value_type &f : *q)
      sAssert(c != f->cathegorySubquery());
    return c;
  }
  else
    return expr()->cathegorySubquery();
}

void DynSubquery::collectSNode(SemanticTree *n) const {
  setSemanticNode(n);
  if (reference_)
    reference_->collectSNode(n);
}

void DynSubquery::linterDefinition(Codestream &str) {
  if (reference_)
    str << reference_;
}


Sm::GlobalDynSqlCursors::mapped_type* DynSubquery::consistantGlobalQueries() const {
  string strReferenceSubquery = nAssert(referenceSubquery)->toNormalizedString();

  Sm::GlobalDynSqlCursors::iterator v =
      syntaxerContext.model->globalDynSqlCursors.find(strReferenceSubquery);

  Sm::sAssert(v == syntaxerContext.model->globalDynSqlCursors.end());

  QueryExpressions *q = &(v->second);
  sAssert(q->empty());

  // проверка консистентности типов всех глобальных запросов и их приведение
  // как если бы они входили в общий union
  EntityFields flds;
  QueryExpressions::iterator it = q->begin();
  (*it)->getFields(flds);
  for (++it; it != q->end(); ++it)
    (*it)->convertUnionFieldTypes(flds, true);
  translateByFetchOps(q);
  return q;
}


DynSubquery::DynSubquery(CLoc l, Ptr<Id> referenceSubquery_, Ptr<RefExpr> codeRef, IntoList _intoList)
  : GrammarBase(l),
    ctx(Sm::nAssert(syntaxerContext.funCtx.top())),
    referenceSubquery(Sm::nAssert(referenceSubquery_)),
    reference_(codeRef),
    intoList_(_intoList)
{
  lazyExprInit();
}


void DynamicFuncallTranslator::initFunnameSrc(CLoc l, PlExpr* _funnameSrc, Ptr<Function> trFun)
{
  Ptr<IdEntitySmart> ref = trFun->getName2()->toIdEntityDeepCopy();
  ref->entity()->loc(l);
  ref->majorEntity()->loc(l);
  Ptr<CallArgList>   callArglist = new CallArgList();
  callArglist->push_back(new Sm::FunCallArgExpr(nAssert(_funnameSrc)));
  ref->entity()->callArglist = callArglist;
  ref->entity()->definition(trFun.object());
  funnameSrc = new Sm::RefExpr(l, ref);
}


DynamicFuncallTranslator::DynamicFuncallTranslator(CLoc l, PlExpr* _funnameSrc, Datatype *_funType, CallArgList *caArgs)
  : GrammarBase(l),
    cathegory    (CALL_NAME),
    funType      (nAssert(_funType)),
    callArguments(caArgs)
{
  initFunnameSrc(l, _funnameSrc, createTranslatorFunction());
}


DynamicFuncallTranslator::DynamicFuncallTranslator(CLoc l, PlExpr* _funnameSrc)
  : GrammarBase(l),
    cathegory  (CALL_SIGNATURE)
{
  initFunnameSrc(l, _funnameSrc, createSignatureTranslatorFunction());
}


Ptr<Datatype> DynamicFuncallTranslator::getDatatype() const          { return funType; }
bool DynamicFuncallTranslator::getFields(EntityFields &fields) const { return funType->getFields(fields); }
bool DynamicFuncallTranslator::getFieldRef(Ptr<Id> &f)               { return funType->getFieldRef(f); }

void DynamicFuncallTranslator::collectSNode(SemanticTree *parent) const {
  SemanticTree *n = new SemanticTree(Sm::SCathegory::DynamicFuncallTranslator);
  n->unnamedDdlEntity = const_cast<DynamicFuncallTranslator*>(this);
  setSemanticNode(n);
  CTREE(funnameSrc);
  if (funType)
    n->addChild(funType->toSTree());
  if (callArguments)
    CollectSNode(n, callArguments);
  parent->addChild(n);
}

Ptr<Function> DynamicFuncallTranslator::createTranslatorFunction() {
  if (DynamicFuncallTranslator::funnameTranslator)
    return DynamicFuncallTranslator::funnameTranslator;

  Ptr<Arglist> l = new Arglist({new FunctionArgument("funsrc", Datatype::mkVarchar2())});
  Ptr<Id2> fname = new Id2(new Id(string("DYN_FUN_TR")), new Id(SysUser::getSelfName()));
  Ptr<Function> f = new Function(fname, l, Datatype::mkVarchar2(200));
  f->setIsSystem();
  DynamicFuncallTranslator::funnameTranslator = f;
  return f;
}

Ptr<Function> DynamicFuncallTranslator::createSignatureTranslatorFunction() {
  if (DynamicFuncallTranslator::funcallSignatureTranslator)
    return DynamicFuncallTranslator::funcallSignatureTranslator;

  Ptr<Arglist> l = new Arglist({new FunctionArgument("funsrc", Datatype::mkVarchar2())});
  Ptr<Id2> fname = new Id2(new Id(string("DYN_TR_CALL_SIGNATURE")), new Id(SysUser::getSelfName()));
  Ptr<Function> f = new Function(fname, l, Datatype::mkVarchar2(200));
  f->setIsSystem();
  DynamicFuncallTranslator::funcallSignatureTranslator = f;
  return f;

}

void DynamicFuncallTranslator::linterDefinition(Sm::Codestream &str) {
  if (str.state().executeBlock_)
    sqlDefinition(str);
  else
    str << funnameSrc;
}

void DynamicFuncallTranslator::sqlDefinition(Sm::Codestream &str) {
  str.state().queryExternalIdentficators.push_back(this);
  translateAsMakestrArg(str, this->getSemanticNode(), funnameSrc->getNextDefinition(), this, true);
  if (callArguments) {
    str << s::obracket;
    translateCallArglist(str, this, callArguments);
    str << s::cbracket;
  }
}


bool QueryEntityDyn::getFieldRef(Ptr<Id> &field) {
  if (Sm::checkToRowidPseudocolumn(field))
    return true;
  return fieldsSrc && fieldsSrc->getFieldRef(field);
}


void ConstructBlockStmt::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementConstructBlockPlsql, SemanticTree::NEW_LEVEL, this);
  n->addChildForce(node);
  dynamicBlock->collectSNode(node);
  if (into)
    into->collectSNode(n);
}

void ConstructBlockStmt::replaceChildsIf(ExprTr tr) {
  replace(tr, dynamicBlock);
}

void ConstructBlockStmt::traverseDeclarationsStmt(DeclActor &fun) {
  dynamicBlock->traverseDeclarationsStmt(fun);
}


DynWhere::DynWhere(CLoc &l, Ptr<PlExpr> _where, Ptr<PlExpr> _tail, CompoundType compound)
  : GrammarBase(l),
    where(new FunctionDynField(l, nAssert(_where))),
    tail(_tail),
    compoundType(compound) {}

void DynWhere::sqlDefinition(Codestream &str) {
  if (beginedFrom(117870))
    cout << "";
  str << where;
  if (tail) {
    switch (compoundType) {
      case DYN_AND:
        str << s::name << "AND";
      case DYN_OR:
        str << s::name << "OR";
      default: break;
    }
    str << s::name << tail;
  }
}

void DynWhere::collectSNode(SemanticTree *n) const {
  where->collectSNode(n);
  if (tail)
    tail->collectSNode(n);
}

DynWhereClause::DynWhereClause(CLoc l, PlExpr *_expr)
  : GrammarBase(l), expr(_expr) {}

void DynWhereClause::collectSNode(SemanticTree *n) const {
  expr->collectSNode(n);
}

void DynWhereClause::linterDefinition(Codestream &str) {
  str << "WHERE" << s::name << expr;
}

