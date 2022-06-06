#include "dynamic_sql.h"
#include "semantic_function.h"
#include "semantic_statements.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "semantic_blockplsql.h"
#include "semantic_expr_select.h"
#include "semantic_expr.h"
#include "statements_tree.h"
#include "siter.h"
#include "smart_lexer.h"
#include "dynamic_sql_op.h"
#include "model_statistic.h"


using namespace Sm;
using namespace ds;
extern SyntaxerContext syntaxerContext;
extern PtrYYLex yylex;

void ds::DynamicSql::buildDbmsSqlInterface() {
  auto resolveItem = [](IdEntitySmart item, void (FunctionFlags::*fun)()) {
    auto setFunctionFlag = [](ResolvedEntity *def, void (FunctionFlags::*fun)()) {
      if (Function *f = def->toSelfFunction())
        (f->flags.*fun)();
      else
        throw 999;
    };

    item.resolveByModelContext();
    ResolvedEntity *def = item.definition();
    if (!def)
      throw 999;
    // return;
    VEntities *v = def->vEntities();
    if (!v)
    // return;
      throw 999;
    for (VEntities::OverloadedFunctions::value_type &i : v->overloadedFunctions) {
      setFunctionFlag(i.first, fun);
      setFunctionFlag(i.second->key, fun);
      for (ResolvedEntity *d : i.second->declarations)
        setFunctionFlag(d, fun);
      for (ResolvedEntity *d : i.second->definitions)
        setFunctionFlag(d, fun);
    }
  };
  resolveItem({"DBMS_SQL", "PARSE"         }, &FunctionFlags::setDbmsSqlParse        );
  resolveItem({"DBMS_SQL", "OPEN_CURSOR"   }, &FunctionFlags::setDbmsSqlOpenCursor   );
  resolveItem({"DBMS_SQL", "BIND_VARIABLE" }, &FunctionFlags::setDbmsSqlBindVariable );
  resolveItem({"DBMS_SQL", "EXECUTE"       }, &FunctionFlags::setDbmsSqlExecute      );
  resolveItem({"DBMS_SQL", "VARIABLE_VALUE"}, &FunctionFlags::setDbmsSqlVariableValue);
  resolveItem({"DBMS_SQL", "CLOSE_CURSOR"  }, &FunctionFlags::setDbmsSqlCloseCursor  );
}


void DynamicSql::extractDynamicSql() {
  auto fun = [&](StatementInterface* stmt, bool /*isConstructor*/, bool /*hasSublevels*/) -> bool {
    // если оператор - это оператор EXECUTE IMMIDIATE
    if (ExecuteImmediate *imm = stmt->toSelfExecuteImmediate()) {
      if (imm->retlist)
        cout << "Execute immediate has returning list in " << imm->getLLoc() << endl;
      execExpressions.push_back(ContextPair(imm->execExpr, ContextPair::EXECUTE_IMMIDIATE_ARGUMENT));
    }
    else if (Fetch *fetch = stmt->toSelfFetch()) {
      if (ResolvedEntity *d = fetch->cursorRef->refDefinition()) {
        if (d->toSelfCursor())
          return true;
        else if (d->toSelfVariable() || d->toSelfFunctionArgument())
          execExpressions.push_back(ContextPair(fetch->cursorRef, ContextPair::FETCH_ARGUMENT));
        else
          throw 999;
      }
    }
    else if (OpenFor *openFor = stmt->toSelfOpenFor()) {
      Datatype *t;
      if (openFor->select && (t = openFor->select->getDatatype()) && t->isVarcharDatatype())
        execExpressions.push_back(ContextPair(openFor->select, ContextPair::OPEN_FOR_STRING));
    }
    return true;
  };
  syntaxerContext.model->traverseModelStatements(fun);

  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;

    RefAbstract    *ref;
    ResolvedEntity *def;
    Function       *f  ;

    // Если выражение - это вызов функции dbms_sql.parse(<имя курсора>, <строка с динамическим SQL>, <флаг версии [V6, NATIVE, V7]>)
    if ((ref = expr->toSelfRefAbstract()) && (def = ref->refDefinition()) &&
        (f = def->toSelfFunction()) && f->flags.isDbmsSqlParse()) {

      CallArgList *callArglist = ref->reference->entity()->callArglist;
      sAssert(!callArglist || callArglist->size() < 2);
      execExpressions.push_back(ContextPair((*callArglist)[1]->expr(), ContextPair::DBMS_OPEN_CURSOR_ARGUMENT));
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };
  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr* { return e; };
  ExprTR     tr(trFun, cond);
  syntaxerContext.model->replaceChildsIf(tr);
}


size_t ds::getDynId() {
  static size_t x = 0;
  return ++x;
}


DepTreeShare::IndexedStatements::mapped_type DepTreeShare::index(StatementInterface *stmt) {
  std::pair<IndexedStatements::iterator, bool> res = indexedStatements.insert(IndexedStatements::value_type(stmt, indexedStatements.size()));
  return res.first->second;
}


void BuildDepTreeCtx::buildRefDependenciesTree(BuildDepTreeCtx &ctx, RefAbstract *srcRef, ResolvedEntity *entityDef) {
  static const unordered_set<ScopedEntities> skipCathegories = {
    Package_, User_, BooleanLiteral_, CollectionMethod_ // , FieldOfRecord_
  };
  static const unordered_set<ScopedEntities> changingAnalyzeCathegories = {
    FieldOfRecord_, Variable_, FunctionArgument_, TriggerPredicateVariable_, FieldOfVariable_, MemberVariable_,
    TriggerRowReference_
  };

  if (srcRef->toSelfRefHostExpr() || entityDef->toSelfVariableField())
    entityDef = srcRef->refMajorEntity()->definition();

  ScopedEntities cat = entityDef->ddlCathegory();

  if (skipCathegories.count(cat))
    return;

  if (!ctx.share.handledEntities.insert(entityDef).second)
    return;

  if (changingAnalyzeCathegories.count(cat)) {
    // TODO: обходить надо все дерево в глубину, и надо проверить что дерево вообще иерархически строится
    CreatedNodeRef addedCtx = ctx.addRefNode(srcRef);
    addedCtx.refNode->externalSrcCathegory = cat;

    if (Ptr<Datatype> t = SubqueryUnwrapper::unwrap(entityDef->getDatatype()))
      if (t->isNum())
        return;

    StatementsTree::Stmts stmts = ctx.topBlockStmtTree.getStatementsChangedEntityBeforeStmt(entityDef, 0);
    // Пройти по всем Statements и добавить узлы, которые изменяют данную переменную.
    for (StatementsTree::Stmts::value_type &s : stmts) {
      StatementInterface *stmt = nAssert(s->nodeStmt());
      addedCtx.refNode->changingOperators.insert(stmt->ddlCathegory());
      if (Assignment *op = stmt->toSelfAssignment())
        createDynopTree(addedCtx.newCtx, op->assignedExpr);
      else if (OpenFor *openFor = stmt->toSelfOpenFor()) {
        Datatype *t;
        if (openFor->select && (t = openFor->select->getDatatype()) && t->isVarcharDatatype()) {
          addedCtx.refNode->isDynamicFetch = true;
          createDynopTree(addedCtx.newCtx, openFor->select);
        }
      }
    }
  }
  else if (Function *fun = entityDef->toSelfFunction()) {
    if (fun->isSystem() || fun->isSystemTemplate())
      return;

    sAssert(!fun->body_);

    StatementsTree bodyTree = fun->body_->buildStatementsTree();
    // TODO: проверить, работает ли данный код - возвращает ли он Return-ы внутри if-ов
    StatementsTree::Stmts returns = bodyTree.getAllReturn();
    for (StatementsTree *returnStmt : returns) {
      Return *ret = nAssert(nAssert(returnStmt->nodeStmt())->toSelfReturn());
      sAssert(!ret->expr);

      StatementContext stmtContext = ret->expr->getOwnerStatementContext();

      BuildDepTreeCtx newCtx(stmtContext, bodyTree, ctx.share, ctx.node);
      createDynopTree(newCtx, ret->expr);
    }
  }
  else if (entityDef->toSelfUserContext() || entityDef->toSelfPackage() || entityDef->toSelfCollectionMethod() ||
           entityDef->toSelfBooleanLiteral() || entityDef->toSelfRecord() || entityDef->toSelfRecordField())
    return;
  else if (entityDef->toSelfAnydata())
    ctx.addRefNode(srcRef).refNode->isAnydataNode = true;
  else if (entityDef->toSelfCollectionMethod())
    ctx.addRefNode(srcRef).refNode->isCollectionMethod = true;
  else
    throw 999;
}


void BuildDepTreeCtx::buildReferenceNode(BuildDepTreeCtx &ctx, RefAbstract *ref) {
  CreatedNodeRef addedRefNodeCtx = ctx.addRefNode(ref);
  if (!addedRefNodeCtx.isNewCreated)
    return;

  // TODO: нужен еще контекст - левое или правое выражение, и к какому оператору это относится
  if (ref->reference->size() == 1) {  // обработка заковыченных строк
    Ptr<Id> refId = ref->reference->front();
    if (refId->quoted() || refId->squoted()) {
      addedRefNodeCtx.refNode->isQuotedLeaf = true;
      return;
    }
  }

  // обработка ссылок на переменные и вызовов функций
  // TODO: обрабатывать ситуацию || TO_CHAR(...) ||

  ResolvedEntity *def = nAssert(ref->reference->definition());
  Function *f;
  if ((f = def->toSelfFunction()) && f->flags.isSysToChar()) {
    addedRefNodeCtx.refNode->isToCharFunction = true;

    CallArgList *l = nAssert(ref->reference->entity()->callArglist);
    Ptr<Datatype> t;

    if ((t = l->at(0)->getDatatype()) && t->isVarcharDatatype())
      createDynopTree(addedRefNodeCtx.newCtx, l->at(0)->expr().object());
  }
  else {
    for (IdEntitySmart::value_type &v : *(ref->reference))
      if (v->callArglist) // если это вызов функций - в его аргументах также может лежать динамический sql
        for (Ptr<FunCallArg> &callArg : *(v->callArglist))
          createDynopTree(addedRefNodeCtx.newCtx, callArg->expr().object());
    buildRefDependenciesTree(addedRefNodeCtx.newCtx, ref, ref->reference->entity()->definition());
  }
}


void BuildDepTreeCtx::createDynopTree(BuildDepTreeCtx &ctx, PlExpr *expr) {
  static const unordered_set<ScopedEntities> skippedCathegories = {
    SqlExprUnaryMinus_, SqlExprNull_, SqlExprEmptyId_, NumericIntVal_, NumericFloatVal_, NumericSimpleInt_,
    LogicalCompound_, Comparsion_
  };
  ScopedEntities cat = expr->ddlCathegory();

  switch (cat) {
    case RefExpr_:
      buildReferenceNode(ctx, expr->toSelfRefExpr());
      break;
    case RefHostExpr_: {
      RefHostExpr *ref = expr->toSelfRefHostExpr();
      sAssert(!nAssert(ref->refMajorEntity()->definition())->toSelfTriggerRowReference());
      buildReferenceNode(ctx, ref);
      break;
    }
    case AlgebraicCompound_: {
      AlgebraicCompound *concat = expr->toSelfAlgebraicCompound();
      if (!concat->isConcatenation()) {
        Datatype *t;
        sAssert(!(t = SubqueryUnwrapper::unwrap(concat->getDatatype())) || !t->isNum());
        return;
      }
      createDynopTree(ctx, concat->getLhs());
      createDynopTree(ctx, concat->getRhs());
    } break;
    case SqlExprBrackets_:
      createDynopTree(ctx, nAssert(expr->toSelfBrackets())->brackets);
      break;
    default:
      if (skippedCathegories.count(cat))
        break;
      throw 999;
  }
}


BuildDepTreeCtx::BuildDepTreeCtx(
  StatementContext   &_stmtContext,
  StatementsTree     &_topBlockStmtTree,
  DepTreeShare       &_share,
  Ptr<DynOperatorRef> _node)
  : node            (_node            ),
    stmtContext     (_stmtContext     ),
    topBlockStmtTree(_topBlockStmtTree),
    share           (_share           ) {}


BuildDepTreeCtx::BuildDepTreeCtx(const BuildDepTreeCtx &oth) :
  node            (oth.node            ),
  stmtContext     (oth.stmtContext     ),
  topBlockStmtTree(oth.topBlockStmtTree),
  share           (oth.share           ) {}


CreatedNodeRef::CreatedNodeRef(const CreatedNodeRef &oth) :
  newCtx      (oth.newCtx),
  refNode     (oth.refNode),
  isNewCreated(oth.isNewCreated) {}


CreatedNodeRef::CreatedNodeRef(
    const BuildDepTreeCtx &_newCtx,
    DynOperatorRef        *_refDynop,
    bool                   _isNewCreated
  ) :
  newCtx      (_newCtx      ),
  refNode     (_refDynop    ),
  isNewCreated(_isNewCreated) { newCtx.node = refNode; }


DynOperatorRef::DynOperatorRef(RefAbstract *_ref, BuildDepTreeCtx &ctx)
  : ref(_ref)
{
  for (SemanticTree *n = nAssert(ref->getSemanticNode()); n; n = n->getParent())
    if (n->unnamedDdlEntity && (ownerStmt = n->unnamedDdlEntity->toSelfStatementInterface()))
      break;
  sAssert(!ownerStmt);
  ownerStmtId = ctx.share.index(ownerStmt);
}


void BuildDepTreeCtx::addOrSetNode(Ptr<DynOperatorRef> refNode)
{
  if (node)
    node->childs.push_back(refNode.object());
  else
    node = refNode.object();
}


CreatedNodeRef BuildDepTreeCtx::addRefNode(RefAbstract *ref) {
  pair<DepTreeShare::NodesMappedByExprs::iterator, bool> it
      = share.nodesMappedByExprs.insert(DepTreeShare::NodesMappedByExprs::value_type(ref, 0));

  if (!it.second) {
    addOrSetNode(it.first->second->toSelfDynOperatorRef());
    return CreatedNodeRef(*this, nAssert(it.first->second->toSelfDynOperatorRef()), false);
  }

  Ptr<DynOperatorRef> refNode = new DynOperatorRef(ref, *this);
  it.first->second = refNode.object();
  addOrSetNode(refNode);

  return CreatedNodeRef(*this, refNode.object(), true);
}


namespace ds {
  bool allChildsIsStringLiterals(DynOperatorRef *node) {
    ResolvedEntity *def = nAssert(node->ref->refDefinition());
    if (!def->toSelfDatatype() || !def->isVarcharDatatype())
      return false;

    for (DynOperatorRef *child : node->childs)
      if (!allChildsIsStringLiterals(child))
        return false;
    return true;
  }

  bool childHasBadTokens(const ClassificationCtx &ctx, DynOperatorRef *node) {
    ResolvedEntity *def = nAssert(node->ref->refDefinition());
    if (def->toSelfDatatype() || def->isVarcharDatatype()) {
      ctx.share.stringLexer->setLexString(node->ref->refEntity()->getText());

      yy::parser::token::yytokentype state;
      while ((state = ctx.share.stringLexer->lex()))
        switch (state) {
          case yy::parser::token::cds_BAD_TO_AUTOMATIC_CAST:
            return true;
          case yy::parser::token::cds_EOF_IN_QUOTED_ID:
            cout << "EOF IN quoted dynamic sql ID: location is " << node->ref->getLLoc().toString() << " text: " << node->ref->refEntity()->getText() << endl;
            break;
          default:
            break;
        }
    }

    for (DynOperatorRef *child : node->childs)
      if (!ctx.share.traversedNodes[child->nodeId]) {
        ctx.share.traversedNodes[child->nodeId] = true;
        if (childHasBadTokens(ctx + 1, child))
          return true;
      }

    return false;
  }
}


ClassificationCathegory::Cathegory DynOperatorRef::classificate(const ClassificationCtx &ctx) {
  if (ctx.level == 0) {
    ctx.share.cleanTraversedNodes();
    if (childHasBadTokens(ctx + 1, this))
      return BAD_BY_TOKENS;

    if (allChildsIsStringLiterals(this))
      return SINGLE_STRING;
  }
  return BAD_UNCLASSIFIED;
  throw 999;
}



void ClassificationShare::cleanTraversedNodes() {
  traversedNodes.clear();
  traversedNodes.resize(maxDynOpId, false);
}

ClassificationShare::ClassificationShare() {
  stringLexer = new SmartLexer::StringLexer();
  maxDynOpId = getDynId();
  cleanTraversedNodes();
}


void DynamicSql::classificate() {
  ClassificationShare lexer;
  ClassificationCtx   ctx(lexer);

  for (ContextPair &argCtx : execExpressions)
    classificated[argCtx.dynTree->classificate(ctx)].push_back(argCtx);
}


void DynamicSql::buildDynamicSqlDependencies() {
  auto sortByCathegory = [](const ContextPair &l, const ContextPair &r) -> bool { return l.cathegory < r.cathegory; };
  DepTreeShare share;

  std::stable_sort(execExpressions.begin(), execExpressions.end(), sortByCathegory);
  // TODO: проверить уникальность выражений-аргументов

  for (ContextPair &argCtx : execExpressions) {
    // получить оператор, внутри которого находится выражение expr
    StatementContext stmtContext = argCtx.srcArgExpr->getOwnerStatementContext();
    sAssert(!stmtContext.statement);

    BlockPlSql *topBlock = 0;
    if (BlockPlSql *b = stmtContext.statement->getOwnerBlock())
      topBlock = b->getTopBlock();

    StatementsTree topBlockStmtTree = nAssert(topBlock)->buildStatementsTree();

    BuildDepTreeCtx ctx(stmtContext, topBlockStmtTree, share);

    BuildDepTreeCtx::createDynopTree(ctx, argCtx.srcArgExpr);
    argCtx.dynTree = ctx.node;
  }
}


string ContextPair::cathegoryToString() const {
  switch (cathegory) {
    case EXECUTE_IMMIDIATE_ARGUMENT:
      return "'executeImmidiate'";
    case OPEN_FOR_STRING:
      return "'openForString'";
    case DBMS_OPEN_CURSOR_ARGUMENT:
      return "'openCursor'";
    case FETCH_ARGUMENT:
      return "'fetch'";
  };
  return "";
}


namespace ds {

ResolvedEntity* getOwner(ResolvedEntity *expr)  {
  if (!expr)
    return 0;
  for (SemanticTree *n = expr->getSemanticNode(); n; n = n->getParent())
    if (ResolvedEntity *d = n->unnamedDdlEntity)
      if (d->toSelfPackage() || d->toSelfTrigger() || d->toSelfFunction()) {
        return d;
      }
  return 0;
}


void printNodes(std::set<ResolvedEntity *> &owners, DynOperatorRef &op, std::set<DynOperatorRef*> &printedNodes) {
  owners.insert(getOwner(op.ref));
  if (printedNodes.insert(&op).second)
    for (Ptr<DynOperatorRef> &childOp : op.childs)
      printNodes(owners, *childOp, printedNodes);
}


string fullnameByStack(ResolvedEntity *defNode) {
  std::vector<string> namesStack;
  static const auto setCathegory = [](string &cat, const string &val) { if (cat.empty()) cat = val; };
  string cathegory;

  ResolvedEntity *def = 0;
  for (SemanticTree *n = defNode->getSemanticNode(); n; n = n->getParent()) {
    if ((n->nametype == SemanticTree::DEFINITION || n->nametype == SemanticTree::DECLARATION) &&
        (def = n->unnamedDdlEntity)) {
      if (def->toSelfView()) {
        setCathegory(cathegory, "VIEW");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
      else if (def->toSelfTrigger()) {
        setCathegory(cathegory, "TRIGGER");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
      else if (def->toSelfFunction()) {
        setCathegory(cathegory, "PROCEDURE");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
      else if (def->toSelfVariable()) {
        setCathegory(cathegory, "VARIABLE");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
      else if (def->toSelfPackage()) {
        setCathegory(cathegory, "PACKAGE");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
      else if (def->toSelfUserContext()) {
        setCathegory(cathegory, "USER");
        namesStack.push_back(nAssert(def->getName())->toString());
      }
    }
  }
  std::reverse(namesStack.begin(), namesStack.end());
  using namespace PlsqlHelper;
  return "(" + quotingAndEscapingPython(cathegory) + "," + quotingAndEscapingPython(Sm::pyJoin(".",namesStack)) + ")";
}

}


void DynamicSql::printPy(const std::string &filename, std::vector<ContextPair> &container) {
  stringstream str;


  str << "G = [" << endl;
  for (ContextPair &argCtx : container) {
    std::set<DynOperatorRef*> printedNodes;
    if (argCtx.cathegory == ContextPair::FETCH_ARGUMENT &&
        argCtx.dynTree                                  &&
        argCtx.dynTree->toSelfDynOperatorRef()          &&
        !(argCtx.dynTree->toSelfDynOperatorRef()->isDynamicFetch))
      continue;
    if (!argCtx.srcArgExpr)
      continue;
//    argCtx.srcArgExpr->linterDefinition(cstr);

    str << "  {'entity':" << ds::fullnameByStack(argCtx.srcArgExpr);

    if (argCtx.dynTree) {
      str << ", 'deps':[" << endl;
      std::set<ResolvedEntity*> s;
      ds::printNodes(s, *(argCtx.dynTree), printedNodes);
      std::vector<string> names;
      for (ResolvedEntity *d : s)
        names.push_back(ds::fullnameByStack(d));
      str << pyJoin(",", names);

      str << "]," << endl;
    }
    str << "}," << endl;
  }
  str << "]" << endl;

  OutputCodeFileChunck::storeCode(OutputCodeFile::temporaryPath(filename), str.str());
  OutputCodeFileChunck::flush();
  syntaxerContext.stage = SyntaxerContext::GLOBAL_CLEANUP;
}


void DynamicSql::printPy() {
  printPy("dynsql_graph.py", execExpressions);
//  for (int i = 0; i < ClassificationCathegory::LAST_CATHEGORY; ++i)
//    printPy("dynsql_graph" + Sm::to_string(i) + ClassificationCathegory::toString((ClassificationCathegory::Cathegory)i) + ".py", classificated[i]);
}


string DynOperatorRef::toString(size_t parentId, int indentingLevel) {
  Sm::CodestreamState state(CodestreamState::PROC);
  Codestream cstr(state);

  sAssert(!ref);
//  ref->linterDefinition(cstr);
  Sm::CathegoriesOfDefinitions::ScopedEntities cat = ref->ddlCathegory();

  stringstream str;
  str << string(indentingLevel, ' ') << "{ "
      << "'id':"     << nodeId      << ", "
      << "'parent':" << parentId    << ", "
      << "'stmtId':" << ownerStmtId << ", "
      << "'val':"    << "\"\"\" " << cstr.str() << " \"\"\", "
      << "'refC':"   << '"' << Sm::toString(cat) << "\", ";
  str << "'defC':"   << '"' << Sm::toString(nAssert(ref->refDefinition())->ddlCathegory()) << "\", ";
  str << "'stmtC':"  << '"';
  if (ownerStmt) {
    FunctionCall   *fc;
    ResolvedEntity *rentity;
    Function       *f;

    Ptr<IdRef> reference;
    if ((fc        = ownerStmt->toSelfFunctionCall()) &&
        (reference = fc->reference()                ) &&
        (rentity   = reference->definition()        ) &&
        (f         = rentity->toSelfFunction()      ) &&
         f->flags.isDbmsSqlParse()
       )
      str << "DBMS_SQL_PARSE";
    else
      str << Sm::toString(ownerStmt->ddlCathegory());
  }
  else
    str << string("EMPTY");

  str << "\", ";
  str << "'loc':" << PlsqlHelper::quotingAndEscapingPython(ref->getLLoc().toString()) << ", "
      << "'flags':({";
  if (isQuotedLeaf)
    str << "'QuotedLeaf',";
  if (isAnydataNode)
    str << "'AnydataNode',";
  if (isToCharFunction)
    str << "'ToChar',";
  if (isCollectionMethod)
    str << "'CollectionMethod',";
  str << "}, {";

  for (CathegoriesOfDefinitions::ScopedEntities val : changingOperators)
    str << PlsqlHelper::quotingAndEscapingPython(Sm::toString(val)) << ",";
  str << "})}";
  return str.str();
}



void ModelContext::resolveDynamicOperators() {
  if (syntaxerContext.descrErrorsEntitiesResolve) {
    ModelStatistic stat(this, 0);
    stat.resolveDescrErrors();
    syntaxerContext.stage = SyntaxerContext::GLOBAL_CLEANUP;

    DynamicSql dynSql;
    dynSql.buildDbmsSqlInterface();
    dynSql.extractDynamicSql();
    dynSql.buildDynamicSqlDependencies();
    dynSql.classificate();
    dynSql.printPy();
    exit(0);
  }

  return;
//  printSqlStatementStatistics(dynSql.dbmsOpenCursorStrings, OutputCodeFile::temporaryPath("using_dbms_sql_cursor_log.sql"), "DBMS_SQL.CURSOR");
//  printSqlStatementStatistics(dynSql.execImmStrings , OutputCodeFile::temporaryPath("using_execute_immideate.sql"), "DBMS_SQL.CURSOR");
}








string ClassificationCathegory::toString(ClassificationCathegory::Cathegory cat) {
  switch (cat) {
    case BAD_BY_TOKENS                            : return "bad_by_tokens";
    case BAD_UNCLASSIFIED                         : return "bad_unclassified";
    case SINGLE_STRING                            : return "str1";
    case SINGLE_CONCATENATION_WITHOUT_VARIABLES   : return "c1offvar";
    case SINGLE_CONCATENATION_WITH_VARIABLES_NOSQL: return "c1varNosql";
    case SINGLE_CONCATENATION_WITH_VARIABLES_SQL  : return "c1varSql";
    case ASSIGN_STATEMENTS                        : return "assign";
    default: return "";
  }
  return "";
}

