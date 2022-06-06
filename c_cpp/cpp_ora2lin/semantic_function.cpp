#include <functional>
#include <stack>
#include "config_converter.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "semantic_flags.h"
#include "semantic_function.h"
#include "semantic_statements.h"
#include "statements_tree.h"
#include "semantic_blockplsql.h"
#include "semantic_object.h"
#include "semantic_expr.h"
#include "semantic_plsql.h"

using namespace Sm;
extern SyntaxerContext syntaxerContext;

void translateFieldComparsion(Sm::Codestream &str, Sm::FLoc lhsLoc, Sm::FLoc rhsLoc, Sm::EntityFields &lhsFields, Sm::EntityFields &rhsFields);

bool Sm::Function::getFieldRefInArglist(Ptr<Id> &field) {
  if (SemanticTree *refnode = field->semanticNode())
    if (Ptr<Id> parentName = refnode->parentInRefList(field)) { // если перед именем поля есть имя функции, совпадающее с именем данной функции
      if (parentName->definition() == this) {
        if (SemanticTree *n = getSemanticNode()) {
          if (arglist && n->childNamespace->findVariable(field))
            return true;
          if (body_ && (body_->getSemanticNode()->childNamespace->findVariable(field) || body_->getSemanticNode()->childNamespace->findCollectionField(field)))
            return true;
        }
      }
    }
  return false;
}



void Function::checkAllArgsIsDefault() {
  if (beginedFrom(63174,13) || (name && name->entity() && name->entity()->beginedFrom(63174,13)))
    cout << "";
  int pos = 0;
  if (arglist)
    for (Vector<FunctionArgument>::iterator it = arglist->begin(); it != arglist->end(); ++it, ++pos) {
      (*it)->positionInArglist = pos;
      if (*it && !(*it)->defaultValue())
        flags.clrAllArgsDefault();
    }
  setOwnerBlockPlSql(0);
}

Sm::IsSubtypeValues Function::isSubtype(ResolvedEntity *supertype, bool plContext) const { return rettype ? rettype->isSubtype(supertype, plContext) : EXPLICIT; }


Function::Function(const Function &oth) :
  GrammarBase(oth),
  Smart(),
  ResolvedEntity(oth),
  ResolvedEntityLoc(oth),
  ResolvedEntitySNodeLoc(oth),
  Declaration(oth.getLLoc()),
  name       (oth.name     ),
  arglist    (oth.arglist  ),
  rettype    (oth.rettype  ),
  body_      (oth.body_    ),
  reducedRettype  (oth.reducedRettype  ),
  reducedCathegory(oth.reducedCathegory),
  actionNumber_   (oth.actionNumber_   )
{
  flags.v = oth.flags.v;
  updateDefinition();
  checkAllArgsIsDefault();
}


Function::Function(const Ptr<Id2> &n, const Ptr<Arglist> &args, const Ptr<Datatype> &rT, const Ptr<BlockPlSql> &b)
  : name(n), arglist(args), rettype(rT), body_(b)
{
  updateDefinition(); checkAllArgsIsDefault();
}


Function::Function(CLoc l, Ptr<Id2> n, Ptr<Arglist> args, Ptr<Datatype> rT, Ptr<BlockPlSql> b)
  : GrammarBase(l), name(n), arglist(args), rettype(rT), body_(b)
{
  updateDefinition(); checkAllArgsIsDefault();
}

Function::Function(const string &n, Ptr<Arglist> args, Ptr<Datatype> rT, bool isElementaryLinFun)
  : arglist(args), rettype(rT)
{
  name = new Id2(new Id(string(n)));
  updateDefinition();
  checkAllArgsIsDefault();
  if (isElementaryLinFun)
    flags.setElementaryLinterFunction();
}

Function::Function(const string &n, std::initializer_list<FunctionArgumentContainer> args, Ptr<Datatype> rT, bool isElementaryLinFun)
  : rettype(rT)
{
  name = new Id2(new Id(string(n)));
  if (args.size()) {
    arglist = new Arglist();
    for (const auto &v : args)
      arglist->push_back(new FunctionArgument(new Id(string(v.name)), v.t, v.dflt));
  }
  updateDefinition();
  checkAllArgsIsDefault();
  if (isElementaryLinFun)
    flags.setElementaryLinterFunction();
}


bool Sm::Function::getFieldRef(Ptr<Id> &field) {
  if (field) {
    // if field.semanticLevel.referenceSize != 1 && field.semanticLevel.reference.parent_in_list(field)->definition() == this
    // искать в списке аргументов. Иначе - искать в возвращаемом типе
    if (getFieldRefInArglist(field))
      return true;
    if (rettype)
      return rettype->getFieldRef(field);
    else
      return false;
  }
  return false;
}


BlockPlSql *Function::getOwnerBlk() {
  if (body_ && body_->ownerPlBlock())
    return body_->ownerPlBlock();
  return ResolvedEntity::ownerPlBlock();
}

bool Function::setOwnerBlockPlSql(BlockPlSql *b) {
  if (body_)
    body_->setOwnerBlockPlSql(b);
  return false;
}

UserContext *Function::userContext() const { return Declaration::userContext(); }



void Function::updateDefinition() {
  if (body_)
    body_->isFunctionTopBody_ = true;
  if (name)
    name->definition(this);
}

void Function::callarglistTR(CallarglistTranslator *tr) { callarglistTranslator = tr; }

void Function::nameTR(NameTranslator tr) { nameTranslator        = tr; }

void Function::translateSelfRef(Codestream &) {}



void Function::getFieldsFromStmtsBeforeOp(
    VariableCursorFields *flds,
    ResolvedEntity       *changedVar,
    StatementsTree       &statementsTree,
    StatementsTree       *op,
    const ResolvedEntity **sourceFields)
{
  StatementsTree::Stmts varChangeStmts = statementsTree.getStatementsChangedEntityBeforeStmt(changedVar, op);

  if (varChangeStmts.empty())
    return;

  auto buildFields = [&changedVar, &flds](StatementInterface *stmt, EntityFields &f) {
    if (OpenFor *op = stmt->toSelfOpenFor())
      op->select->getFields(f);
    else if (FunctionCall *fcall = stmt->toSelfFunctionCall()) {
      if (Ptr<IdRef> reference = fcall->reference())
        if (Ptr<Id> refEntity = reference->entity()) {
          CallArgList *args = nAssert(refEntity->callArglist);
          CallArgList::iterator it = std::find_if(args->begin(), args->end(),
            [&](Ptr<Sm::FunCallArg> &v) -> bool {
              RefExpr *expr;
              ResolvedEntity *def;
              return ( v                                  &&
                      (expr = v->expr()->toSelfRefExpr()) &&
                      (def  = expr->refDefinition()     ) &&
                      (rRef(def) == rRef(changedVar)    )
                     );
            });
          sAssert(it == args->end());
          if (ResolvedEntity *d = reference->definition()) {
            Function *fun = nAssert(d->toSelfFunction());
            fun->getFieldsFromOutRefcursor(flds, std::distance(args->begin(), it), &(flds->fieldsSource));
            f = flds->fields_;
          }
        }
    }
    else
      throw 999;
  };

  StatementsTree::Stmts::iterator it = varChangeStmts.begin();

  FLoc firstLoc = cl::emptyFLocation();

  if (!flds->fields_.empty())  {
    if (sourceFields && *sourceFields)
      firstLoc = (*sourceFields)->getLLoc();
  }
  else {
    EntityFields f;
    buildFields((*it)->nodeStmt(), f);

    if (StatementInterface *stmt = (*it)->nodeStmt()) {
      if (OpenFor  *openForSmt = stmt->toSelfOpenFor()) {
        flds->setFieldsFrom(f, openForSmt->select.object());
        if (sourceFields)
          *sourceFields = (*it)->nodeStmt()->toSelfStatement();
        firstLoc = (*it)->nodeStmt()->getLLoc();
        ++it;
      }
      else {
        cout << "error: statement is not openFor: " << Sm::toString(stmt->ddlCathegory()) << endl;
        return;
      }
    }
    else {
      cout << "error: StatementsTree node StatementInterface attribute is NULL" << endl;
      return;
    }
  }

  for (; it != varChangeStmts.end(); ++it) {
    StatementsTree* v = *it;
    EntityFields f;
    buildFields(v->nodeStmt(), f);
    if (!compareFields(flds->fields_, f, true, false)) {
      EntityFields fff;

      buildFields((*(varChangeStmts.begin()))->nodeStmt(), fff);
      Codestream str;

      translateFieldComparsion(str, firstLoc, v->nodeStmt()->getLLoc(), flds->fields_, f);
      cout << "ERROR: Bad new field struct in getFields of Function " << getName()->toQInvariantNormalizedString(true) << " " << getLLoc() << ": " << str.str() << endl;
    }
  }
}


void Function::getFieldsFromReturnRefcursor(VariableCursorFields *flds, const ResolvedEntity **sourceFields) {
  if (!body_)
    throw 999;

  if (beginedFrom(75620))
    cout << "";

  StatementsTree statementsTree = body_->buildStatementsTree();
  StatementsTree::Stmts retStmts = statementsTree.getAllReturn();
  if (retStmts.empty() || retStmts.size() > 1)
    throw 999;
  Ptr<PlExpr> retExpr = retStmts.front()->nodeStmt()->toSelfReturn()->expr->unwrapBrackets();
  Ptr<RefExpr> ref = retExpr->toSelfRefExpr();
  if (!ref)
    throw 999;
  ResolvedEntity *def = ref->refDefinition();
  if (!def)
    return;
  Variable *var = def->toSelfVariable();
  if (!var)
    throw 999;

  getFieldsFromStmtsBeforeOp(flds, var, statementsTree, retStmts.front(), sourceFields);
}

void Function::getFieldsFromOutRefcursor(VariableCursorFields *flds, unsigned int pos, const ResolvedEntity **sourceFields) {
  if (!body_ || pos >= arglistSize())
    return;

  FunctionArgument *cursorArg = (*arglist)[pos];
  StatementsTree statementsTree = body_->buildStatementsTree();

  getFieldsFromStmtsBeforeOp(flds, cursorArg, statementsTree, 0, sourceFields);
}

void Function::setRetType(Ptr<Datatype> t) { rettype = t; }


int Function::countArglistRefcursorFiels() {
  int cnt = 0;
  if (arglist)
    for (Vector<FunctionArgument>::iterator it = arglist->begin(); it != arglist->end(); ++it)
      switch ((*it)->dir()) {
        case function_argument::OUT:
          break;
        case function_argument::IN:
         case function_argument::IN_OUT:
           if ((*it)->getDatatype()->isRefCursor()) {
             Codestream str;
             str << s::ref << *this << " " << s::iloc(this->getLLoc()) << s::endl;
             cout << str.str();
             return 1;
           }
       }
  return cnt;
}


Ptr<BlockPlSql> Function::funBody() const { return body_; }


bool Sm::Function::semanticResolve() const {
  if (rettype)
    rettype->semanticResolve();
  if (body_)
    body_->semanticResolve();
  if (arglist)
    for (Vector<FunctionArgument>::const_iterator it = arglist->begin(); it != arglist->end(); ++it)
      if (*it)
        (*it)->semanticResolve();
  return semanticResolveBase();
}


void Function::addArgument(Ptr<FunctionArgument> arg) {
  if (arg) {
    if (!arglist)
      arglist = new Arglist();
    arglist->push_back(arg);
    if (!arg->defaultValue())
      flags.clrAllArgsDefault();
  }
}


void Function::traverseDeclarationsForce(DeclActor &fun) {
  if (body_)
    body_->traverseDeclarations(fun);
}

void Function::traverseDeclarations(DeclActor &fun) {
  if (beginedFrom(1520598))
    cout << "";
  if (fun(this))
    traverseDeclarationsForce(fun);
}

void Function::traverseModelStatements(StatementActor &fun) {
  if (body_)
    body_->traverseModelStatements(fun);
}

void Function::traverseArguments(ArgActor &fun) {
  if (arglist)
    for (Arglist::value_type &arg : *arglist) {
      fun(arg.object());
    }
}

void Function::replaceChildsIf(Sm::ExprTr tr) {
  replace(tr, arglist, body_);
}

CallarglistTranslator *Function::callarglistTR() const { return callarglistTranslator; }

NameTranslator Function::nameTR() const { return nameTranslator; }

Ptr<Arglist> Function::funArglist() const { return arglist; }

Ptr<Id> Function::getName() const { return name->entity(); }

Ptr<Id2> Function::getName2() const { return name; }

void Function::actionNumber(size_t val) { actionNumber_ = val; }

bool Function::isProcedure() const { return !rettype; }

Ptr<Arglist> Function::getArglist() const { return arglist; }

Vector<FunctionArgument>::iterator Function::arglistBegin() { return arglist->begin(); }

Vector<FunctionArgument>::iterator Function::arglistEnd() { return arglist->end  (); }

bool Function::isDefinition() const { return body_; }

Ptr<Datatype> Function::getDatatype() const { return rettype; }

Datatype *Function::getRettype() const { return rettype.object(); }


void Function::replaceStatementsIf(StmtTr tr, StmtTrCond cond) {
  if (body_)
    body_->replaceBlockSubstatementsIf(tr, cond);
}



void BlockPlSql::pushLoop(Loop *loop) { loopStack.push(loop); }


void BlockPlSql::popLoop(Loop *loop) {
  if (loopStack.empty() || loop != loopStack.top())
    throw 999;
  loopStack.pop();
}


Ptr<Loop> BlockPlSql::getActiveLoop() const { return (!loopStack.empty()) ? loopStack.top() : Ptr<Sm::Loop>(NULL); }


void BlockPlSql::collectStatementsSNode(SemanticTree *&node) const { CollectSNode(node, statements); }


bool BlockPlSql::setOwnerBlockPlSql(BlockPlSql *b) {
  if (setOwnerBlockIsEntered_)
    return true; // (1)

  // setOwnerBlockCnt_ == 0
  ++setOwnerBlockIsEntered_;
  // setOwnerBlockCnt_ == 1
  // BlockPlSql::setOwnerBlockPlSql =(1)= true => callilng functor for statements subtree
  setOwnerBlockOnChildrenDeclarations();
  traverseChildStatements(SetOwnerBlk(this));
  ownerBlock = b;
  --setOwnerBlockIsEntered_;

  // BlockPlSql::setOwnerBlockPlSql =(1)= false => functor for statements subtree has not called
  return false;
}

void BlockPlSql::setOwnerBlockOnChildrenDeclarations() {
  DeclActor setOwnerBlk = [this](Declaration *self) -> bool {
    self->setOwnerBlockPlSql(this);
    return true;
  };
  traverseDeclarations(setOwnerBlk);
}


void BlockPlSql::initBlockPlSql(Declarations* _declarations, list<Ptr<StatementInterface> > *_statements, list<Ptr<StatementInterface> > *_exceptionHandlers)
{
  if (_declarations)
    declarations.swap(*_declarations);
  if (_statements)
    statements.swap(*_statements);
  if (_exceptionHandlers)
    exceptionHandlers.swap(*_exceptionHandlers);

  ++strongRef;
  setOwnerBlockPlSql(ownerBlock);
  --strongRef;
}




Ptr<Datatype> BlockPlSql::getDatatype() const { return Ptr<Datatype>(); }


SemanticTree *BlockPlSql::toSTreeBNew() const {
  SemanticTree *node = new SemanticTree(SCathegory::BlockPlSql, SemanticTree::NEW_LEVEL);
  node->unnamedDdlEntity = (BlockPlSql*)this;
  return node;
}


void BlockPlSql::collectSNodeBase(SemanticTree *&n) const {
  CollectSNode(n, declarations);

  SemanticTree *exceptionNode = new SemanticTree(SCathegory::ExceptionSection, &exceptionHandlers);
  n->addChildForce(exceptionNode);
  CollectSNode(exceptionNode, exceptionHandlers);

  SemanticTree *stmts = new SemanticTree(SCathegory::BlockPlSqlStatementsList, &statements);
  n->addChildForce(stmts);
  CollectSNode(stmts, statements);
//  if (endLabel)
//    n->addChild(endLabel->toSNodeDecl(SCathegory::Label, this));
}


void BlockPlSql::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }


void FunctionContext::swap(FunctionContext &oth) {
  unionDynamicComponents.swap(oth.unionDynamicComponents);
}

void SyntaxerContext::pushFunCtx() {
  funCtx.push(new Sm::FunctionContext());
}

void SyntaxerContext::popFunCtx(Function *fun) {
  if (fun)
    fun->metaContext.swap(*funCtx.top());
  delete funCtx.top();
  funCtx.pop();
}


SemanticTree *BlockPlSql::toSTreeBase() const { SemanticTree *n = toSTreeBNew(); collectSNodeBase(n);  return n; }


void BlockPlSql::addDeclarations(Ptr<Declarations> decl) {
  if (decl) {
    declarations.swap(*decl);
    if (decl->size())
      declarations.insert(declarations.end(), decl->begin(), decl->end());
  }
}


void BlockPlSql::collectInitializers(EntitiesSet &container) {
  std::stack<BlockPlSql*> ownerBlocks;
  ownerBlocks.push(this);
  DeclActor *filterPtr;

  DeclActor filter = [&](Declaration *decl) -> bool {
    if (!decl)
      return true;

    if (BlockPlSql *b = decl->toSelfBlockPlSql()) {
      ownerBlocks.push(b);
      b->traverseDecls(*filterPtr);
      ownerBlocks.pop();
      return false;
    }
    else if (Variable *var = decl->toSelfVariable()) {
      BlockPlSql *b = ownerBlocks.top();
      if (var->flags.isDynamicLoopCounter())
        return true;
      else if (!(b->isNotPackageBlock_) || var->isPackageVariable()
               || var->hasObjectType())
        container.insert(var);
    }
    else if (Sm::Type::Object *obj = decl->toSelfObject())
      obj->collectInitializers(container);

    return true;
  };
  filterPtr = &filter;
  traverseDeclarationsStmt(filter);
}


void BlockPlSql::add(Ptr<Declaration> decl) { declarations.push_back(decl); }


void BlockPlSql::linterDefinition(Codestream &str) { linDef(str); }


void BlockPlSql::traverseDecls(DeclActor &fun)
{
  for (Declarations::value_type &v : declarations)
    v->traverseDeclarations(fun);
  for (auto &v : statements)
    v->traverseDeclarationsStmt(fun);
}

bool BlockPlSql::traverseDecls(DeclActor &fun, std::stack<BlockPlSql*> &stackBlks) {
  stackBlks.push(this);
  traverseDecls(fun);
  stackBlks.pop();
  return false;
}


void BlockPlSql::traverseDeclarationsStmt(DeclActor &fun) {
  if (fun(this))
    traverseDecls(fun);
}


void Sm::BlockPlSql::traverseDeclarations(DeclActor &fun) {
  if (fun(this))
    traverseDecls(fun);
}


void Sm::BlockPlSql::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceBlockSubstatementsIf(tr, cond);
}

void Sm::BlockPlSql::replaceBlockSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  for (Declarations::value_type &v : declarations)
    v->replaceStatementsIf(tr, cond);
  replaceStatements(tr, cond, statements, initializators, exceptionHandlers);
}

BlockPlSql *Function::childCodeBlock() const { return body_; }

BlockPlSql *Function::body() const { return const_cast<BlockPlSql*>(body_.object()); }


void Function::collectInitializers(EntitiesSet &container) {
  if (body_)
    body_->collectInitializers(container);
}

void Function::translatedName(const string &v) {
  ResolvedEntity::translatedName(v);
}

ResolvedEntity *Function::ownerPackage() const { return Declaration::ownerPackage(); }

bool Function::isExactlyEquallyByDatatype(ResolvedEntity *oth) { return rettype ? rettype->isExactlyEquallyByDatatype(oth) : false; }


static const int functionDatatypeConversionDebugOutput = 0;
void Function::debugOutput(Ptr<Datatype> minimalCommonDatatype) {
  if (functionDatatypeConversionDebugOutput) {
    Ptr<Datatype> concrLhs = ResolvedEntity::getLastConcreteDatatype(rettype);
    Ptr<Datatype> concrRhs = ResolvedEntity::getLastConcreteDatatype(minimalCommonDatatype);
    Sm::Codestream str ;
    str << s::proc << concrLhs << " -> ";
    if (concrRhs->isNull())
      str << "null";
    else
      str << concrRhs;
    str << "  : " << s::ref << *this << s::iloc(getLLoc());
    cout << str.str() << endl;
  }
}


void Sm::Function::inferenceReducedRettype() {
  flags.setRettypeConversionEntered();
  if (!rettype || !body_)
    return;

  Ptr<Datatype> minimalCommonDatatype;
  if (beginedFrom(855914,28))
    cout << "";
  StatementActor reudce = [&minimalCommonDatatype, this](StatementInterface *self, bool isConstructor, bool /*hasSublevels*/) -> bool {
    if (isConstructor)
      if (Return *ret = self->toSelfReturn())
        ret->getMinimalCommonDatatypeOfRetstatement(minimalCommonDatatype, rettype);
    return true;
  };

  body_->traverseStatements(reudce);
  
  if (!minimalCommonDatatype)
    return;

  Ptr<Datatype> t = rettype;
  if (t && minimalCommonDatatype && t->isRefCursor() && minimalCommonDatatype->isRefCursor()) // TODO: нужно реализовать поддержку этого случая для рефкурсоров
    return;
  CastCathegory val = t->getCastCathegory(minimalCommonDatatype, true);
  if (val.lengthLhsGtRhs()) {
    reducedRettype   = ResolvedEntity::getLastConcreteDatatype(minimalCommonDatatype);
    reducedCathegory = val;
    debugOutput(minimalCommonDatatype);
  }
}


void Sm::Function::castReturnExpressionsToRettype() {
  if (!rettype)
    return;
  inferenceReducedRettype();
  Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(rettype);
  using namespace std::placeholders;
  if (body_) {
    if (isPipelined()) {
      string name;
      Ptr<Datatype> colDatatype = getLastConcreteDatatype(rettype);
      pipeVar = addVariableIntoOwnerBlock(colDatatype, &name, getLLoc(), "PIPERES_");
      pipeIndex = addVariableIntoOwnerBlock(Datatype::mkInteger(), &name, getLLoc(), "PIPEIDX_");
      pipeIndex->setDefaultValue(new NumericSimpleInt(1));
    }

    body_->traverseStatements(std::bind(mem_fn(&StatementInterface::castReturnStatementsToDatatype), _1, ref(t)));
  }
}

void Sm::Function::castFuncArgDefaultValue() {
  traverseArguments(mem_fn(&FunctionArgument::explicitCast));
}


Type::MemberFunction::MemberFunction(
    CLoc l,
    Type::Inheritance::T _inheritance,
    Type::member_function::Specificators _specificators,
    Ptr<Id2>        n,
    Ptr<Arglist>    args,
    Ptr<Datatype>   rT,
    Ptr<BlockPlSql> b)
  : GrammarBase(l), Function(l, n ? n : Ptr<Id2>(new Id2(new Id("", 0, false, true))), args, rT, b),
    inheritance  (_inheritance),
    specificators(_specificators) {}


Type::MemberFunction::MemberFunction(CLoc l, Type::Inheritance::T _inheritance, Ptr<Arglist> args, Ptr<Datatype> rT, Ptr<BlockPlSql> b, Ptr<Datatype> _selfConstructorDatatype)
  : GrammarBase(l), Function(l, new Id2(new Id("", 0, false, true)), args, rT, b),
    inheritance            (_inheritance),
    specificators          (member_function::CONSTRUCTOR),
    selfConstructorDatatype(_selfConstructorDatatype) {}

void Type::MemberFunction::collectSNodeM(SemanticTree *n) const { collectSNode(n); }

void Type::MemberFunction::resolve(Ptr<Id>) {}

Ptr<Id> Type::MemberFunction::getMemberName() const { return getName(); }


Type::ArrayConstructor::ArrayConstructor(Type::ObjectType *_owner, Ptr<Id> ownerName, Ptr<Datatype> ownerDatatype)
  : GrammarBase(ownerName->getLLoc()),
    Function(new Id2(ownerName->toNormalizedString()), 0, ownerDatatype, 0),
    owner(_owner) {}


void WhenExpr::collectSNode(SemanticTree *n_src) const {
  SemanticTree *n = new SemanticTree(SCathegory::WhenExprClause, this);
  n_src->addChildForce(n);
  setSemanticNode(n);
  CTREE(condition)
  CollectSNode(n, branchStatements);
}


WhenExpr::~WhenExpr() {}


void WhenExpr::replaceChildsIf(ExprTr tr) { replace(tr, condition, branchStatements); }


void WhenExpr::traverseDeclarationsStmt(DeclActor &fun) {
  for (auto &s : branchStatements)
    s->traverseDeclarationsStmt(fun);
}

void WhenExpr::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, branchStatements);
}

void WhenExpr::traverseStatements(StatementActor &fun) {
  traverseStmts(this, fun, branchStatements);
}

void WhenExpr::buildStatementsTree(StatementsTree &parent) {
  buildStmtsTree(this, parent, branchStatements);
}


WhenExpr::WhenExpr(CLoc l, Ptr<WhenExpr::Statements> stmts, Ptr<PlExpr> cond, Cathegory _cathegory)
  : GrammarBase(l), condition(cond), cathegory(_cathegory)
{
  Sm::swap(branchStatements, stmts);
}


WhenExpr::WhenExpr(Ptr<StatementInterface> stmt, Ptr<PlExpr> cond, Cathegory _cathegory)
  : condition(cond), branchStatements(stmt), cathegory(_cathegory) {}

WhenExpr::WhenExpr(Ptr<WhenExpr::Statements> stmts, Ptr<PlExpr> cond, Cathegory _cathegory)
  : condition(cond), cathegory(_cathegory) { Sm::swap(branchStatements, stmts); }


Ptr<Id> LinterCursor::getName() const { return name; }


Ptr<Datatype> LinterCursor::getDatatype() const { return thisDatatype_ ?  thisDatatype_ : (thisDatatype_ = new Datatype(name)); }


bool LinterCursor::getFields(EntityFields &fields) const { return query->getFields(fields); }


void LinterCursor::linterReference(Codestream &str) {
  str << name;
}


Sm::LinterCursor::LinterCursor(std::string &&_name, Ptr<SqlExpr> _query)
  : name(new Id(std::forward<string>(_name), (LinterCursor*)this)),
    query(_query)
{
  EntityFields fields;
  query->getFields(fields);
  std::set<std::string> names;
  for (EntityFields::iterator it = fields.begin(); it != fields.end(); ++it) {
    if (ResolvedEntity *d = (*it)->definition()) {
      std::string dst = (*it)->toString();
      for (std::string::iterator it = dst.begin(); it != dst.end(); ++it)
        if (*it == '#' || *it == '$')
          *it = '_';
      std::pair<std::set<std::string>::iterator, bool> res = names.insert(dst);
      while (!res.second) {
        dst.push_back('_');
        res = names.insert(dst);
      }
      d->procTranslatedName(dst);
    }
  }
}


Ptr<Datatype> Loop::getDatatype() const { return Ptr<Datatype>(); }


Loop::Loop(CLoc l, Ptr<Statements> stmts, Ptr<Id> endLblId)
  : GrammarBase(l), endLabelId(endLblId), endloopLabel(false) { Sm::swap(loopStatements, stmts); }


void Loop::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementLoop, this);
  n->addChildForce(node);
  CollectSNode(node, loopStatements);
  if (endLabelId)
    node->addChild(endLabelId->toSNodeDef(SCathegory::Label, this));
}


void Loop::traverseDeclarationsStmt(DeclActor &fun) {
  if (beginedFrom(1474155))
    cout << "";
  for (auto &v : loopStatements)
    v->traverseDeclarationsStmt(fun);
}

void Loop::buildStatementsTree(StatementsTree &parent) { buildStmtsTree(this, parent, loopStatements); }


void Loop::replaceSubstatementsIf(StmtTr tr, StmtTrCond cond) {
  replaceStatements(tr, cond, loopStatements);
}

LValue::LValue(CLoc l, Ptr<IdEntitySmart> _lEntity)
  : GrammarBase(l), RefExpr(l, _lEntity) {}

LValue::LValue(CLoc l, Ptr<IdEntitySmart> _lEntity, int)
  : GrammarBase(l), RefExpr(l, _lEntity)
{
  __flags__.v |= FLAG_LVALUE_IS_HOST;
}

LValue::LValue(CLoc l, Id* name)
  : GrammarBase(l), RefExpr(l, name, NULL) {}

LValueIntoList::LValueIntoList(CLoc l, Ptr<IdEntitySmart> _lEntity)
  : GrammarBase(l), LValue(l, _lEntity) {}

void LValue::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }


SemanticTree *LValue::toSTreeBase() const {
  SemanticTree *node = reference->toSTreeRef(isHost() ? SCathegory::LValueHost : SCathegory::LValue);
  node->unnamedDdlEntity = const_cast<LValue*>(this);
  return node;
}


LValue::~LValue() {
}


Ptr<Datatype> LValue::getDatatype() const {
  if (ResolvedEntity *d = reference->definition())
    return d->getDatatype();
  return 0;
}


void VariableCursorFields::getFieldsRecursive(EntityFields &fields) const {
  if (fields_.empty())
    const_cast<VariableCursorFields*>(this)->buildCursorFields();
  if (!fields_.empty()) {
    for (const EntityFields::value_type &f : fields_) {
      if (!f->definition())
        throw 999;

      VariableField *fldDef = f->definition()->toSelfVariableField();
      if (!fldDef)
        throw 999;
      if (fldDef->isStructuredField)
        fldDef->getFieldsRecursive(fields);
      else
        fields.push_back(f);
    }
  }
}

void VariableCursorFields::setFieldsFrom(EntityFields &flds, const ResolvedEntity *_fieldsSource, string prefix) {
  //    Ptr<Datatype> t = getLastConcreteDatatype(id->getDatatype());
  //    if (t && (t->isRowTypeOf() || t->isRecordType()))
  // STRCODE
  if (getVarName()->definition()->eid() == 997640)
    cout << "";
  if (getVarName()->toNormalizedString() == "FILTER")
    cout << "";
  if (getLLoc().beginedFrom(1429074))
    cout << "";
  SemanticTree *n = getVariableSNode();
  Ptr<LevelResolvedNamespace> space = n->childNamespace;
  if (!space) {
    space = new LevelResolvedNamespace(n->levelNamespace, n);
    sAssert(!n || !n->levelNamespace);
    n->levelNamespace->childs.push_back(space);
  }

  fieldsSource = _fieldsSource;

  for (const Ptr<Id> &f : flds) {
    ResolvedEntity *fDef = f->definition();
    ResolvedEntity::ScopedEntities cat;
    // Избыточных случаев для других категорий - не выявлено
    while (fDef && (cat = fDef->ddlCathegory()) == ResolvedEntity::FieldOfVariable_)
      fDef = fDef->toSelfVariableField()->sourceQuery;

    if (!fDef)
      throw 999;

    Ptr<Id> newVarName;
    {
      Sm::SelectedField *sFld;
      if ((sFld = fDef->toSelfSelectedField()) && (sFld->fieldNameReallyEmpty))
        if (sFld->expr()->isRownum())
          newVarName = new Id(f->getLLoc(), "ROWNUM", fDef);
        else
          newVarName = new Id(f->getLLoc(), space->getUniqueName("FIELD"), fDef);
      else
        newVarName = new Id(*f);
    }

    Ptr<VariableField> varField = new VariableField(newVarName, fDef);
    varField->baseField     = fDef;
    varField->ownerVariable = this;

    // для последующей трансляции остальных ссылок в формальных преобразованиях
    // необходимо поддерживать возможность поиска во внутреннем пространстве имен по
    // исходному - неоттранслированному имени
    {
      // контроль уникальности для неиспользуемых полей с одинаковыми именами
      string baseName = varField->getName()->toNormalizedString();
      if (!space->count(baseName)) { // Пропускать неуникальные неиспользуемые поля.

        // Добавление во внутренне пространство имен неоттранслированного имени
        varField->translatedName(baseName);
        space->add(varField->getName());

        // транслированное имя будет позже переустановлено
        varField->translatedName(string());
      }
    }

    SemanticTree *varNode = varField->toSTree();
    n->addChild(varNode);
    varNode->levelNamespace = space;
    newVarName = varField->getName();
    newVarName->clearSpecSymbols();

    Ptr<Datatype> t = SubqueryUnwrapper::unwrap(fDef->getDatatype());
    bool isStruct = t && (t->isRowTypeOf() || t->isRecordType());

    std::string s = newVarName->toCodeId(space, /*addToNamespace =*/ true, isStruct);

    varField->procTranslatedName(prefix + s);
    varField->translatedName(prefix + s);

    if (isStruct) {
      varField->isStructuredField = true;
      varField->buildCursorFields();
    }

    fieldsCursor.push_back(varField);
    fields_.push_back(newVarName);
  }
}

bool VariableCursorFields::buildCursorFields() {
  Ptr<Datatype> t = getVariableDatatype();
  if (!t)
    return false;

  if (getLLoc().beginedFrom(1429074))
    cout << "";

  if (fieldsCursor.size())
    return true;

  if (ddlCathegory() == ResolvedEntity::TriggerPredicateVariable_)
    throw 999;
  ResolvedEntity *fieldsSource = t;
  for (ResolvedEntity *it = t; it; ) {
    ResolvedEntity::ScopedEntities cat = it->ddlCathegoryWithDatatypeSpec();
    switch (cat) {
      case Table_:
      case View_:
      case Cursor_:
      case DatatypeRowtype_:
      case QueryBlock_:
      case ReturnInto_:
        fieldsSource = it;
        break;
      case Record_:
        if (it->toSelfRecord()->xmlType_)
          return false;
        fieldsSource = it;
        break;
      case FunctionArgument_:
      case Variable_:
      case FieldOfTable_:
      case FieldOfRecord_:
      case SqlSelectedField_:
      case QueriedPseudoField_:
        it = it->getDatatype();
        continue;
      case Datatype_:
      case DatatypeType_:
      case Subtype_:
        it = it->getNextDefinition();
        continue;
      case NestedTable_:
      case Varray_:
      case FundamentalDatatype_:
      case AnydataObject_:
      case Object_:
        return false;
      case RefCursor_: {
        Sm::Type::RefCursor *rc = it->toSelfRefCursor();
        if (rc->datatype) {
          it = rc->datatype;
          continue;
        }
        return false;
      }
      default:
        throw 999;
    }
    break;
  }

  {
    EntityFields flds;
//    fieldsSource->getFieldsExp(flds, /*isProc = */ true); // TODO: getFieldsExp
    fieldsSource->getFields(flds); // getFieldsExp

    if (syntaxerContext.unwrapStructuredFields)
      if (VariableField *self = toSelfResolvedEntity()->toSelfVariableField())
        if (self->isStructuredField) {
          setFieldsFrom(flds, fieldsSource, self->translatedName() + "_");
          return true;
        }
    setFieldsFrom(flds, fieldsSource);
  }
  return true;
}


bool VariableCursorFields::getVariableFieldRef(Ptr<Id> &field) {
  if (getLLoc().beginedFrom(1147179,3))
    cout << "";

  if (buildCursorFields())
    if (getVariableSNode()->childNamespace->findField(field))
      return true;

  if (Ptr<Sm::Datatype> t = getVariableDatatype())
    return t->getFieldRef(field);
  else
    return false;
}


VariableCursorFields::VariableCursorFields() {

}


FunctionArgument::FunctionArgument(CLoc l, Ptr<Id> n, Ptr<Datatype> t, Ptr<SqlExpr> dflt, function_argument::Direction dir)
  : GrammarBase(l), name(n ? n->def(this) : (Id*)0), datatype(t), defaultValue_(dflt), direction(dir)
{}


FunctionArgument::FunctionArgument(Ptr<Id> n, Ptr<Datatype> t, Ptr<SqlExpr> dflt, function_argument::Direction dir)
  : name(n ? n->def(this) : (Id*)0), datatype(t), defaultValue_(dflt), direction(dir) {}

FunctionArgument::FunctionArgument(const string &n, Ptr<Datatype> t, Ptr<SqlExpr> dflt, function_argument::Direction dir)
  : name(new Id(string(n), this)), datatype(t), defaultValue_(dflt), direction(dir) {}


Sm::IsSubtypeValues FunctionArgument::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  return eqByVEntities(supertype) ? EXACTLY_EQUALLY : datatype->isSubtype(supertype, plContext);
}


ResolvedEntity *FunctionArgument::getFieldDDLContainer() const {
  if (SemanticTree *n = getSemanticNode())
    if (SemanticTree *p = n->getParent())
      if (ResolvedEntity *d = p->ddlEntity())
        if (d->toSelfFunction())
          return d;
  return 0;
}


bool FunctionArgument::getFieldRef(Ptr<Id> &field) { return getVariableFieldRef(field); }


namespace Sm {

size_t Variable::__getGlobalVarId() {
  static size_t id = 0;
  ++id;
  if (id == 37964)
    cout << "";
  return id;
}

BlockPlSql *Variable::ownerPlBlock() const {
  if (ownerBlock_)
    return ownerBlock_;
  else
    return ResolvedEntity::ownerPlBlock();
}

size_t BlockPlSql::__getGlobalBlkId() {
  static size_t blkId = 0;
  ++blkId;
  if (blkId == 2)
    cout << "";
  return blkId;
}


bool Variable::hasDynamicDefaultValue() const {
  if (defaultValue_) {
    if (!defaultValue_->isNull()) {
      if (defaultValue_->isFuncall(true) || defaultValue_->isComplexExpr() || hasObjectType() ||
          ((flags.isGlobal() || isPackageVariable()) && defaultValue_->isQuotedSqlExprId()))
        return true;
      if (RefAbstract *defExpr = defaultValue_->toSelfRefAbstract())
        if (ResolvedEntity *exprDef = defExpr->refDefinition())
          if (exprDef->isPackageVariable() || exprDef->isFunArgument() || exprDef->isField())
            return true;
    }
  }
  return false;
}


bool Variable::hasObjectType() const {
  Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(getDatatype());
  return t && t->isObjectType();
}

bool Variable::explicitCast() {
  if (!defaultValue_)
    return true;

  if (beginedFrom(123611))
    cout << "";

  Ptr<Datatype> selfType = getDatatype();
  Ptr<Datatype> castedType = defaultValue_->getDatatype();
  if (selfType && castedType && !defaultValue_->isNull()) {
    CastCathegory v = selfType->getCastCathegory(castedType, true, false);
    if (v.explicitAll()) {
      v.setProcCastState();
      v.setCastAssignment();
      Ptr<PlExpr> newExpr = CommonDatatypeCast::cast(defaultValue_.object(), castedType, selfType, v);
      defaultValue_ = static_cast<SqlExpr*>(newExpr.object());
    }
  }
  return true;
}

void Variable::setOpenCursorCommand(OpenCursor *cmd) {
  if (ResolvedEntity *d = datatype->tidDdl())
    if (Cursor *c = d->toSelfCursor()) {
      c->setOpenCursorCommand(cmd);
      return;
    }
  throw 999;
}

void Variable::clrOpenCursorCommand() {
  if (ResolvedEntity *d = datatype->tidDdl())
    if (Cursor *c = d->toSelfCursor()) {
      c->clrOpenCursorCommand();
      return;
    }
  throw 999;
}

Subquery *Variable::getSelectQuery() {
  if (ResolvedEntity *d = datatype->tidDdl())
    if (Cursor *c = d->toSelfCursor())
      return c->getSelectQuery();
  throw 999;
  return 0;
}

Type::ObjectType *Variable::getObjectDatatype() const {
  if (Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(getDatatype()))
    return t->toSelfObjectType();
  return 0;
}




bool VariableCursorFields::getFields(EntityFields &fields) const {
  Ptr<Datatype> t = getVariableDatatype();
  if (!t)
    return false;

  if (fields_.empty())
    const_cast<VariableCursorFields*>(this)->buildCursorFields();

  if (!fields_.empty()) {
    fields = VariableCursorFields::fields_;
    return true;
  }
  return t->getFields(fields);
}

bool Variable::getFields(EntityFields &fields) const {
  return VariableCursorFields::getFields(fields);
}


bool FunctionArgument::getFields(EntityFields &fields) const {
  return VariableCursorFields::getFields(fields);
}

bool Function::getFields(EntityFields &fields) const {
  return VariableCursorFields::getFields(fields);
}

Sm::FunctionArgumentContainer::FunctionArgumentContainer() {}


Sm::FunctionArgumentContainer::FunctionArgumentContainer(string n, Datatype *_t, SqlExpr *_dflt)
  : name(n), t(_t), dflt(_dflt) {}


Sm::FunctionArgumentContainer::FunctionArgumentContainer(const FunctionArgumentContainer &o)
  : name(o.name), t(o.t), dflt(o.dflt) {}


bool FunctionArgument::hasDynamicDefaultValue() const {
  static unordered_set<string> unsupSysFuncs = {"SYSDATE", "SYSTIMESTAMP", "DBNAME", "USERNAME", "UID", "TRUNC"};

  if (beginedFrom(128233))
    cout << "";
  if (defaultValue_) {
    if (!defaultValue_->isNull()) {
      if (defaultValue_->isFuncall(false) || defaultValue_->isComplexExpr())
        return true;
      if (RefAbstract *defExpr = defaultValue_->toSelfRefAbstract())
        if (ResolvedEntity *exprDef = defExpr->refDefinition()) {
          if (exprDef->isPackageVariable() || exprDef->isFunArgument() || exprDef->isField())
            return true;
          if (exprDef->isFunction() &&
              unsupSysFuncs.find(exprDef->getName()->toNormalizedString()) != unsupSysFuncs.end())
            return true;
        }
    }
  }
  return false;
}

void FunctionArgument::addToInitializers() {
  if (!defaultValue_)
    return;
  
  BlockPlSql *block = ownerPlBlock();
  if (!block)
    return;

  Ptr<LValue> lVal = new LValue(getLLoc(), new IdEntitySmart(getName()));
  Ptr<Assignment> assign = new Assignment(getLLoc(), lVal, defaultValue_);

  Ptr<PlExpr> condition = new pl_expr::IsNull(cl::emptyFLocation(), new RefExpr(getLLoc(), getName()));
  Ptr<WhenExpr> firstIfStmts = new WhenExpr(new Sm::Statements(), condition, Sm::WhenExpr::IF_FIRST_STATEMENT);
  firstIfStmts->branchStatements.push_back(assign.object());
  Ptr<If> ifStatement = new If(cl::emptyFLocation(), firstIfStmts, NULL, NULL);

  block->blockInitializators()->push_back(ifStatement.object());
}

bool FunctionArgument::explicitCast() {
  if (!defaultValue_)
    return true;

  Ptr<Datatype> selfType = getDatatype();
  Ptr<Datatype> castedType = defaultValue_->getDatatype();
  if (selfType && castedType && !defaultValue_->isNull()) {
    CastCathegory v = selfType->getCastCathegory(castedType, true, false);
    if (v.explicitAll()) {
      v.setProcCastState();
      v.setCastAssignment();
      Ptr<PlExpr> newExpr = CommonDatatypeCast::cast(defaultValue_.object(), castedType, selfType, v);
      defaultValue_ = static_cast<SqlExpr*>(newExpr.object());
    }
  }

  if (hasDynamicDefaultValue())
    addToInitializers();
  return true;
}

void FunctionArgument::checkFieldsConsistance(EntityFields &srcFields, const FLoc &srcLoc) {
  bool cmpRes = compareFields(fields_, srcFields, true, false);
  if (!cmpRes) {
    Codestream str;
    translateFieldComparsion(str, getLLoc(), srcLoc, fields_, srcFields);
    cout << "ERROR: Bad new field structure of function argument: " << str.str() << endl;
  }
}

bool FunctionArgument::isLinterStructType() const {
  Ptr<Datatype> baseType = ResolvedEntity::getLastUnwrappedDatatype(getDatatype());
  return baseType && baseType->isLinterStructType();
}

void FunctionArgument::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(745413))
    cout << "";
  str.state().dynamicCollection(isDynamicUsing());
  definitionBase(str);
  if (buildCursorFields() || fields_.size()) {
    if (isLinterStructType())
      translateFieldsAsStruct(fields_, str);
    else
      translateFieldsAsCursor(fields_, str);
  }
  else
    str << s::def << datatype;
  if (defaultValue_)  {
    str << s::name << "DEFAULT ";
    if (!hasDynamicDefaultValue())
      str << defaultValue_;
    else
      str << "NULL";
  }
  else
    str << s::stub;
  str.state().dynamicCollection(false);
}


FunctionArgument::~FunctionArgument() {
  clrSRef();
}

Codestream &operator<<(Codestream &s, FunctionArgument &obj) { return obj.translate(s); }

VariableCursorFields *VariableField::getTopOwnerVariable() {
  VariableCursorFields *item = ownerVariable;
  while (VariableField *f = item->toSelfResolvedEntity()->toSelfVariableField()) {
    if (f->ownerVariable)
      item = f->ownerVariable;
    else
      return f;
  }
  return item;
}

VariableField::VariableField(Id *_name, ResolvedEntity *srcQuery)
  : GrammarBase(_name->getLLoc()),
    Variable(_name, 0, false, 0, false, _name->getLLoc()),
    sourceQuery(srcQuery)
{
  if (!sourceQuery || !_name)
    throw 999;
}

VarHelper::VarHelper() {
  tmpVars.reserve(30);
}

VarHelper &VarHelper::getInstance() {
  static VarHelper helper;
  return helper;
}

void VarHelper::add(Variable *var) {
  if (!var)
    return;
  var->use();
  tmpVars.push_back(var);
}

void VarHelper::unuseAll() {
  for (Vector<Variable>::reference var : tmpVars) {
    var->unuse();
  }

  tmpVars.clear();
}

}
