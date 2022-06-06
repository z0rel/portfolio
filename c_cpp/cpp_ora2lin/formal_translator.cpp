#include <iomanip>
#include "formal_translator.h"
#include "codespacer.h"
#include "semantic_function.h"
#include "semantic_statements.h"
#include "semantic_blockplsql.h"
#include "semantic_plsql.h"
#include "semantic_expr_select.h"
#include "semantic_object.h"
#include "semantic_collection.h"
#include "config_converter.h"
#include "dynamic_sql_op.h"
//#include "dynamic_sql.h"

#include "sql_syntaxer_bison.h"

#include "smart_lexer.h"

extern PtrYYLex yylex;

using namespace Sm;
extern SyntaxerContext syntaxerContext;

bool containsEntityAsReference(SemanticTree *n, ResolvedEntity *ref);
void enumUseVarStatements(ResolvedEntity *var, BlockPlSql *blk, FoundedStatements &usedStatements);

SemanticTree::ReferencesForCastedCalls SemanticTree::referencesForCastedCalls;

void setSpecialNameTranslator(Sm::IdEntitySmart name, Sm::NameTranslator nameTr,
                              Ptr<Sm::CallarglistTranslator> callTr = 0,
                              bool forceElementaryLinter = false) {
  name.resolveByModelContext();
  if (ResolvedEntity *d = name.definition()) {
    d->callarglistTR(callTr);
    d->nameTR(nameTr);
    if (forceElementaryLinter)
      if (Function *f = d->toSelfFunction())
        f->setElementaryLinFunc();
    if (VEntities *ent = d->vEntities())
      ent->setSpecialNamesTranslator(d, nameTr, callTr, forceElementaryLinter);
  }
}

void ModelContext::translateCursorFieldDecltype() {
  FormalTranslator tr(*this);
  tr.translateCursorFieldDecltype();
}

void ModelContext::formalTranslations() {
  FormalTranslator tr(*this);
  syntaxerContext.stage = SyntaxerContext::FORMAL_TRANSLATIONS;
  tr.formalTranslations();
  syntaxerContext.model->rootGlobalTree->deleteInvalidChilds();
}

void ModelContext::formalPrepareModel() {
  FormalTranslator tr(*this);
  tr.prepareDynamicExpressions();
}


FormalTranslator::FormalTranslator(ModelContext &cntx)
  : cntx_(cntx) {}

void FormalTranslator::formalTranslations() {
  cout << " == formal translations  ==" << endl;

  setSpecialNameTranslator(Sm::IdEntitySmart({"SYS"    , "DBMS_SESSION", "SET_CONTEXT"        }), Sm::trDbmsSessionSetContextName, 0);
  setSpecialNameTranslator(Sm::IdEntitySmart({"UPDATES", "DBMS_OPERLOG", "SET_OPERLOG_CONTEXT"}), Sm::trDbmsOperlogSetOperlogContext, new CallarglistTranslatorSimple(trDbmsOperlogSetOperlogContextCallarglist));

  auto timingExecute = [&](string action, std::function<void()> f) {
    clock_t begin = clock();
    cout << " === " << action << " ===" << flush;
    f();
    cout << " " << diffTime(begin) << endl;
  };

  cntx_.setPackagesAttributesOnBlocks();

  timingExecute("initializeDeclarationsNames"               , [&]() { initializeDeclarationsNames(); });
  timingExecute("transformFunctions"                        , [&]() { transformFunctions(); });
  timingExecute("translateAssignmentRefcursorFromFunctions" , [&]() { translateAssignmentRefcursorFromFunctions(); });
  timingExecute("translateReferences"                       , [&]() { translateReferences(); });
  timingExecute("translateStatements"                       , [&]() { translateStatements(); });

  // транслировать и обновить объявление курсорной переменной и всех ее полей от каждого открытия до следующего ее изменения

  cntx_.setPackagesAttributesOnBlocks();
  timingExecute("translateVariablesDynamicInitializers"     , [&]() { translateVariablesDynamicInitializers(); });
  timingExecute("translateDynamicTables"                    , [&]() { translateDynamicTables(); });
  if (syntaxerContext.printCursorVariables)
    timingExecute("printCursorVariables"                    , [&]() { cntx_.rootGlobalTree->printCursorVariables(); });

  cntx_.anydataInterface.initAnydataContainers();
  cntx_.xmlInterface.initContainers();
  cntx_.blobInterface.initContainers();

  cntx_.setPackagesAttributesOnBlocks();

  cout << " == formal translations finished ==" << endl;
}

void FormalTranslator::translateDynamicTables() {
  static std::function<void (ResolvedEntity *)> setDynamicFlag = [](ResolvedEntity *def) -> void {
    if (!def || def->isDynamicUsing())
      return;
    switch (def->ddlCathegory()) {
    case ResolvedEntity::MemberFunction_:
    case ResolvedEntity::Function_: {
      if (def->beginedFrom(1444767, 10))
        cout << "";
      Function *func = def->getDefinitionFirst()->toSelfFunction();
      BlockPlSql *block = func->body();
      if (!block)
        return;
      if (func->isPipelined()) {
        setDynamicFlag(func->pipeVar.object());
        func->setIsDynamicUsing();
        def->setIsDynamicUsing();
      }
      else if (Sm::RefAbstract *exprId = block->findLastReturnExpression()) {
        setDynamicFlag(exprId->refDefinition());
        func->setIsDynamicUsing();
        def->setIsDynamicUsing();
      }
    } break;

    case ResolvedEntity::FunctionArgument_: {
      //TODO: в текущей модели не встречается.
      //Сделать полный поиск ссылок на функцию и найти искомую используемую как аргумент переменную.
      cout << "ERROR: translateDynamicTables meet the dynamic argument " << def->getName()->toNormalizedString() << ":" << def->getLLoc().locText() << endl;
    } break;
    case ResolvedEntity::ArrayConstructor_: {
       cout << "ERROR: translateDynamicTables for ArrayConstructor_ unsupported " << def->getLLoc().locText() << endl;
       break;
    }

    case ResolvedEntity::MemberVariable_:
    case ResolvedEntity::Variable_: {
      def->setIsDynamicUsing();
      if (Ptr<Datatype> datatype = def->getDatatype()->getFinalType())
        if (Ptr<Datatype> elDatatype = datatype->getNextDefinition()->mappedType()) {
          elDatatype = elDatatype->getFinalType();
          if (elDatatype->isObjectType())
            setDynamicFlag(elDatatype->getNextDefinition());
        }
    } break;

    case ResolvedEntity::Object_:
    case ResolvedEntity::Varray_:
    case ResolvedEntity::NestedTable_: {
      EntityFields fields;
      def->setIsDynamicUsing();
      def->getFields(fields);
      for (EntityFields::const_reference f : fields) {
        if (f->definition() && !f->definition()->toSelfDatatype())
          setDynamicFlag(f->definition());
      }
      if (def->isObject())
        def->toSelfObject()->applyDynamicUsing();
    } break;

    default:
      throw 999;
    }
  };

  // Обходим все преобразования TABLE(...) в запросах
  SemanticTree::EnumList allTableDynamics;
  cntx_.rootGlobalTree->enumNodesByCat(SCathegory::FromTableDynamic, allTableDynamics);

  for (SemanticTree *dynNode : allTableDynamics) {
    if (dynNode->childs.empty())
      throw 999;
    ResolvedEntity *def = dynNode->childs.front()->refDefinition();
    setDynamicFlag(def);
  }

  // Обходим все пайплайн функции
  auto trDecl = [&](Declaration* decl) -> bool {
    if (decl && decl->isFunction() && decl->toSelfFunction()->isPipelined() && !decl->isDynamicUsing())
      setDynamicFlag(decl);
    return true;
  };
  traverseUserDeclarations(trDecl);

  // Обходим все присваивания и возвраты функций
  auto checkDynamicExpr = [](Sm::PlExpr *expr) -> bool {
    if (expr && expr->beginedFrom(78832))
      cout << "";
    if (expr)
      if (Sm::RefExpr *refExpr = expr->toSelfRefExpr())
        if (Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(refExpr->getDatatype())) {
          if (datatype->isObjectType() && !refExpr->refDefinition()->isConstructor() && refExpr->refDefinition()->isDynamicUsing())
            return true;
          if (Sm::ResolvedEntity *typeObj = datatype->getNextDefinition())
            if (typeObj->isObject() && refExpr->refDefinition()->isConstructor() && typeObj->isDynamicUsing())
              return true;
        }
    return false;
  };
  auto majorDefinition = [](Sm::IdEntitySmart *reference) -> Sm::ResolvedEntity* {
    for (auto it = reference->rbegin(); it != reference->rend(); ++it) {
      if (ResolvedEntity *def = (*it)->definition())
        if (def->ddlCathegory() != ResolvedEntity::User_ &&
            def->ddlCathegory() != ResolvedEntity::Package_)
          return def;
    }
    return NULL;
  };
  Sm::StmtTrCond cond = [&](StatementInterface *lastStmt, bool constructor, list<Ptr<StatementInterface> > &/*stmts*/, list<Ptr<StatementInterface> >::iterator &/*it*/) -> int {
    if (!constructor)
      return FLAG_REPLACE_TRAVERSE_ONLY;
    if (Sm::Assignment *assignment = lastStmt->toSelfAssignment()) {
      if (checkDynamicExpr(assignment->assignedExpr)) {
        Sm::ResolvedEntity *def = majorDefinition(assignment->lValue->reference);
        setDynamicFlag(def);
      }
      else if (checkDynamicExpr(assignment->lValue.object())) {
       Sm::RefExpr *rExpr = assignment->assignedExpr->toSelfRefExpr();
       Sm::ResolvedEntity *def = majorDefinition(rExpr->reference);
       if (!def->isConstructor())
         setDynamicFlag(def);
      }
    }
    else if (Sm::Return *ret = lastStmt->toSelfReturn()) {
      if (checkDynamicExpr(ret->expr))
        setDynamicFlag(ret->getOwnerBlock()->ownerFunction());
      else if (Sm::ResolvedEntity *func = ret->getOwnerBlock()->ownerFunction()) {
        if (ret->expr && func->isDynamicUsing())
          if (Sm::RefExpr *refExpr = ret->expr->toSelfRefExpr())
            setDynamicFlag(refExpr->refDefinition());
      }
    }

    return FLAG_REPLACE_TRAVERSE_ONLY;
  };
  StmtTr tr = [](StatementInterface *s, list<Ptr<StatementInterface> > *, list<Ptr<StatementInterface> >::iterator *) -> StatementInterface* { return s; };
  cntx_.replaceStatementsIf(tr, cond);
}

void FormalTranslator::prepareDynamicExpressions() {
  std::stack<StatementInterface*> blkStack;

  typedef std::stack<DeclNamespace*> DeclNamespaceStack;
  typedef std::map<string, DeclNamespaceStack> ActiveNamespace;

  typedef std::pair<StatementInterface *, std::set<string> > NspaceStackItem;

  ActiveNamespace activeNamespace;
  std::stack<NspaceStackItem> nspaceStack;

  StatementActor traverse = [&](StatementInterface *stmt, bool isConstructor, bool hasSublevels) -> bool {
    if (hasSublevels) {
      if (isConstructor)
        blkStack.push(stmt);
      else {
        if (!nspaceStack.empty() && nspaceStack.top().first == stmt) {
           for (const string &s : nspaceStack.top().second) {
             ActiveNamespace::iterator it = activeNamespace.find(s);
             if (it == activeNamespace.end())
               throw 999;
             it->second.pop();
             if (it->second.empty())
               activeNamespace.erase(it);
           }
           nspaceStack.pop();
        }
        blkStack.pop();
      }
    }
    if (!isConstructor)
      return true;

    if (Sm::DeclNamespace *n = stmt->toSelfDeclNamespace()) {
      if (blkStack.empty())
        throw 999;
      string nspaceVar = n->name->toNormalizedString();
      activeNamespace[nspaceVar].push(n);

      if (nspaceStack.empty() || nspaceStack.top().first != blkStack.top())
        nspaceStack.push(NspaceStackItem(blkStack.top(), set<string>({nspaceVar})));
      else if (!nspaceStack.top().second.insert(nspaceVar).second)
        throw 999; // дублирующееся имя динамического пространства имен
    }
    else if (Sm::ConstructExprStmt* op = stmt->toSelfConstructorExpr()) {
      if (op->srcNamespace) {
        ActiveNamespace::iterator it = activeNamespace.find(op->srcNamespace->toNormalizedString());
        if (it == activeNamespace.end())
          cout << "ERROR: not found _decl_namespace for " << op->srcNamespace->getText() << " " << op->getLLoc() << endl;
        else
          op->fromNode = it->second.top();
      }
    }
    return true;
  };

  cntx_.traverseModelStatements(traverse);
}


void FormalTranslator::translateCursorFieldDecltype() {
//  Sm::StmtTrCond cond = [&](StatementInterface *lastStmt, bool constructor, list<Ptr<StatementInterface> > &/*stmts*/, list<Ptr<StatementInterface> >::iterator &/*it*/) -> int {
//    if (constructor) {
////      if (Sm::CursorFieldDecltype *v = lastStmt->toSelfCursorFieldDecltype()) {
////        for (Ptr<Id> var : *v->varNames)
////          setIsFieldForMakestr(var->definition());
////      }
////      else
////      if (Sm::ConstructExprStmt *v = lastStmt->toSelfConstructorExpr())
////        setIsFieldForMakestr(v->var->definition());
//    }
//    return FLAG_REPLACE_TRAVERSE_ONLY;
//  };

//  StmtTr tr = [](StatementInterface *s, list<Ptr<StatementInterface> > *, list<Ptr<StatementInterface> >::iterator *) -> StatementInterface* { return s; };
//  cntx_.replaceStatementsIf(tr, cond);
}

void translateFieldComparsion(Codestream &str, FLoc lhsLoc, FLoc rhsLoc, EntityFields &lhsFields, EntityFields &rhsFields) {
  str << "Old: " << s::iloc(lhsLoc) << s::name;
  debugTranslateEntityFields(str, lhsFields);
  str << s::endl
      << "New: open for - " << s::iloc(rhsLoc) << s::name;
  debugTranslateEntityFields(str, rhsFields);
  str << s::endl;
}

void enumUseVarStatements(ResolvedEntity *var, BlockPlSql *blk, FoundedStatements &usedStatements) {
  set<StatementInterface*> uniqueStmts;
  CollectUsingByStatements::StmtsStack stmtStack;
  set<SemanticTree*> handledNodes;

  ExprTR::Cond cond = [&](PlExpr *expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;
    if (SemanticTree *n = expr->getSemanticNode()) {
      if (handledNodes.insert(n).second)
        if (containsEntityAsReference(n, var)) {
          if (stmtStack.size())
            if (StatementInterface *s = *(stmtStack.begin())) {
              if (s->toSelfCaseStmt())
                return FLAG_REPLACE_TRAVERSE_NEXT;
              if (uniqueStmts.insert(s).second)
                usedStatements.push_back(s);
            }
          return FLAG_REPLACE_TRAVERSE_NEXT;
        }
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };
  CollectUsingByStatements tr(cond, &stmtStack);
  blk->replaceChildsIf(tr);
}

bool FormalTranslator::translateRefCursorQuery(Sm::OpenFor *openFor) {
  Subquery *q = openFor->selectExpr()->toSelfSubquery();
  if (!q)
    return false;
  ResolvedEntity *cursorDef = openFor->openForIterVariable->definition();
  BlockPlSql *blk = openFor->getOwnerBlock();
  if (!blk)
    return false;

  FoundedStatements usedStatements;
  enumUseVarStatements(cursorDef, blk, usedStatements);

  //Ищем  близлежащий Fetch оператор, чтобы поменять типы переменных на типы в IntoList
  for (FoundedStatements::iterator it = usedStatements.begin(); it != usedStatements.end(); ++it) {
    if (Sm::Fetch *fetchStmt = (*it)->toSelfFetch())
      if (!fetchStmt->fields.empty()) {
        ++(fetchStmt->fields.strongRef);
        q->translateSelectedFieldsDatatypesToIntoListTypes(&fetchStmt->fields);
        --(fetchStmt->fields.strongRef);
        return true;
      }
  }

  return false;
}

void FormalTranslator::translateAssignmentRefcursorFromFunctions() {
  CursorDecltypes cursorDecltypes;
  std::set<ResolvedEntity*, LE_ResolvedEntities> fetchCursorRefs;

  Sm::StmtTrCond cond = [&](StatementInterface *lastStmt, bool constructor, list<Ptr<StatementInterface> > &/*stmts*/, list<Ptr<StatementInterface> >::iterator &/*it*/) -> int {
    if (!constructor)
      return FLAG_REPLACE_TRAVERSE_ONLY;
    if (Sm::Assignment *assignment = lastStmt->toSelfAssignment())
      translateRefcursorAssignments(assignment);
    else if (Sm::CursorDecltype *declt = lastStmt->toSelfCursorDecltype())
      cursorDecltypes.insert(declt);
//    else if (Sm::CursorFieldDecltype *v = lastStmt->toSelfCursorFieldDecltype()) {
//      for (Ptr<Id> var : *v->varNames)
//        setIsFieldForMakestr(var->definition());
//    }
//    else if (Sm::ConstructExprStmt *v = lastStmt->toSelfConstructorExpr())
//      setIsFieldForMakestr(v->var->definition());
    else if (Sm::OpenFor *openFor = lastStmt->toSelfOpenFor()) {
      updateOpenForDecltypeStatements(openFor, cursorDecltypes);
      ResolvedEntity *cursorDef = openFor->openForIterVariable->definition();
      if (!cursorDef)
        throw 999;
      if (cursorDef->getDatatype()->isRefCursor())
        translateRefCursorQuery(openFor);
      EntityFields flds;
      openFor->getFields(flds);
      if (flds.empty()) {
        cout << "ERROR: " << std::left << setw(30) << openFor->getLLoc().toString() << " Empty OpenFor operator "  << endl;
        return FLAG_REPLACE_TRAVERSE_ONLY;
      }
      // TODO: нужно задавать поля для этих курсорных переменных
      VariableCursorFields *varCursor = cursorDef->toSelfVariableCursorFields();
      if (!varCursor)
        throw 999;
      if (varCursor->fields_.size()) {
        bool cmpRes = compareFields(flds, varCursor->fields_, true, false);
        if (!cmpRes) {
          Codestream str;
          if (!varCursor->fieldsSource)
            throw 999;

          translateFieldComparsion(str, varCursor->fieldsSource->getLLoc(), openFor->getLLoc(), varCursor->fields_, flds);
          cout << "ERROR: Bad new field struct OpenFor " << endl << str.str() << endl;

          return FLAG_REPLACE_TRAVERSE_ONLY;
        }
      }
      else
        varCursor->setFieldsFrom(flds, openFor);
    }
    else if (Sm::Fetch *fetchStmt = lastStmt->toSelfFetch()) {
      // Вынес сюда из-за порядка обхода и формирования курсорных переменных
      ResolvedEntity *def = fetchStmt->cursorRef->getNextDefinition();
      if (def)
      if (Cursor *cursor = def->toSelfCursor()) {
        Subquery *q = cursor->getSelectQuery();
        if (q && !fetchStmt->fields.empty() && fetchCursorRefs.find(cursor) == fetchCursorRefs.end()) {
          ++(fetchStmt->fields.strongRef);
          q->translateSelectedFieldsDatatypesToIntoListTypes(&fetchStmt->fields);
          --(fetchStmt->fields.strongRef);
          fetchCursorRefs.insert(cursor);
        }
      }
    }

    return FLAG_REPLACE_TRAVERSE_ONLY;
  };
  StmtTr tr = [](StatementInterface *s, list<Ptr<StatementInterface> > *, list<Ptr<StatementInterface> >::iterator *) -> StatementInterface* { return s; };
  cntx_.replaceStatementsIf(tr, cond);

  setFieldsOnUnopenedVariables(cursorDecltypes);

  auto trDecl = [&](Declaration* decl) -> bool { if (decl && decl->isFunction()) translateRefcursorReturn(decl->toSelfFunction()); return true; };
  traverseUserDeclarations(trDecl);
}

void FormalTranslator::translateStatements() {
  StmtTr tr = [](StatementInterface *s, list<Ptr<StatementInterface> > *, list<Ptr<StatementInterface> >::iterator *) -> StatementInterface* { return s; };

  Sm::StmtTrCond cond = [&](StatementInterface *lastStmt, bool constructor, list<Ptr<StatementInterface> > &stmts, list<Ptr<StatementInterface> >::iterator &it) -> int {
    if (!constructor)
      return FLAG_REPLACE_TRAVERSE_ONLY;
    if (Sm::CaseStatement *caseStmt = lastStmt->toSelfCaseStmt()) {
      ++(caseStmt->strongRef);
      Ptr<CaseStatement> tmp2 = caseStmt;
      --(caseStmt->strongRef);
      (void)tmp2;

      if (If *ifStmt = caseStmt->transformIfSpecial()) {
        Ptr<StatementInterface> tmp1 = ifStmt;
        syntaxerContext.model->delayDeletedStatements.push_back(caseStmt);
        *it = tmp1;
        int state = cond(it->object(), constructor, stmts, it);
        if (state & FLAG_REPLACE_TRAVERSE_NEXT)
          ifStmt->replaceSubstatementsIf(tr, cond);
        return FLAG_REPLACE_SKIP_SECOND_CONDITION;
      }
      return FLAG_REPLACE_TRAVERSE_ONLY;
    }
    else if (Sm::Assignment *assignment = lastStmt->toSelfAssignment()) {
      castAssignmentRvalue(assignment);
      translateRefcursorAssignments(assignment);
      if (syntaxerContext.unwrapStructuredFields)
        if (ResolvedEntity *lref = assignment->lvalue()->refDefinition())
          if (VariableField *var = lref->toSelfVariableField())
            if (var->isStructuredField)
              // после этого метода нужно выходить, т.к. итераторы и assignment в нем могут быть изменены
              assignment->unwrapStructuredLValue(var, stmts, it);

      return FLAG_REPLACE_TRAVERSE_ONLY;
    }
    return FLAG_REPLACE_TRAVERSE_ONLY;
  };

  cntx_.replaceStatementsIf(tr, cond);
}


void FormalTranslator::castAssignmentRvalue(Sm::Assignment *assignment) {
  if (ResolvedEntity *ent = assignment->lValue->refDefinition()) {
    Ptr<Datatype> l = ent->getDatatype();
    Ptr<Datatype> r = assignment->assignedExpr->getDatatype();
    if (r && l) {
      CastCathegory cat = r->getCastCathegory(l, true);
      if (cat.explicitAll()) {
        Ptr<PlExpr> expr = assignment->assignedExpr;
        if (!expr->isNull()) {
          cat.setProcCastState();
          cat.setCastAssignment();
          assignment->assignedExpr = CommonDatatypeCast::cast(expr.object(), r.object(), l.object(), cat);
        }
      }
    }
  }
}



bool FormalTranslator::updateOpenForDecltypeStatements(Sm::OpenFor *openFor, CursorDecltypes &cursorDecltypes) {
  bool ret = false;
  for (CursorDecltypes::iterator it = cursorDecltypes.begin(); it != cursorDecltypes.end();) {
    Sm::CursorDecltype* op = *it;
    if (op->beginedFrom(103397))
      cout << "";
    if (ResolvedEntity *varRef = op->varName->definition())
      if (ResolvedEntity *openVarRef = openFor->baseCursorVariable->getNextDefinition())
        if (varRef->eqByVEntities(openVarRef)) {
          if (openFor->decltypeSelectStmts.size())
            throw 999;
          EntityFields flds;
          op->select->getFields(flds);

          if (VariableCursorFields *var = varRef->toSelfVariableCursorFields())
            var->setFieldsFrom(flds, op);
          else
            throw 999;

          openFor->decltypeSelectStmts.push_back(op);
          it = cursorDecltypes.erase(it);
          ret = true;
          continue;
        }
    ++it;
  }
  return ret;
}


bool FormalTranslator::translateReference(PlExpr* expr) {
  Ptr<IdEntitySmart> ent;
  if (RefExpr *e = expr->toSelfRefExpr())
    ent = e->reference;
  else if (RefHostExpr *e = expr->toSelfRefHostExpr())
    ent = e->reference;
  else
    return false;

  IdEntitySmart::iterator mIt = ent->majorEntityIter();

  ResolvedEntity *mD = (*mIt)->definition();
  if (!mD)
    return false;

  if (ent->size() == 1)
    return false;

  if (mD->ddlCathegory() == ResolvedEntity::User_) {
    --mIt;
    if (!(mD = (*mIt)->definition()))
      return false;
  }
  if (mD->ddlCathegory() == ResolvedEntity::Package_) {
    --mIt;
    if (!(mD = (*mIt)->definition()))
      return false;
  }
  if (mIt == ent->begin())
    return false;
  Ptr<Datatype> t = mD->getDatatype();
  if (t && t->isCollectionType())
    return false;

  ResolvedEntity::ScopedEntities cat = mD->ddlCathegory();
  if (cat == ResolvedEntity::TriggerRowReference_ || cat == ResolvedEntity::TriggerNestedRowReference_) {
    IdEntitySmart::iterator it = ent->majorEntityIter();
    Ptr<Id> n = *(--it);
    if (n->toNormalizedString() == "NAME")
      cout << "";
    if (!n->quoted() && (n->hasDollarSymbols() || n->isReservedField()))
      n->setUpperAndQuoted();
  }
  else if (cat == ResolvedEntity::Variable_
           || cat == ResolvedEntity::VariableUndeclaredIndex_
           || cat == ResolvedEntity::FunctionArgument_) {
    IdEntitySmart::iterator varIt = mIt;
    --varIt;
    Ptr<Id> n = (*varIt);
    ResolvedEntity *fldDef = n->definition();
    if (!fldDef)
      return false;
    ResolvedEntity::ScopedEntities cat = fldDef->ddlCathegory();
    if (cat == ResolvedEntity::SqlSelectedField_ ||
        cat == ResolvedEntity::QueriedPseudoField_ ||
        cat == ResolvedEntity::FieldOfTable_) {

      n->clearDefinition();
      SemanticTree *snode = mD->getSemanticNode();
      if (!snode || !snode->childNamespace)
        throw 999;
      snode->childNamespace->findVariable(n);

      ResolvedEntity *def = n->definition();
      if (!def || def == fldDef)
        throw 999;
    }
    else if (cat == ResolvedEntity::LinterCursorField_) {
      SemanticTree *node = mD->getSemanticNode();
      if (!node || !node->childNamespace)
        throw 999;
      fldDef->procTranslatedName(n->toCodeId(node->childNamespace));
    }
  }
  return true;
}

void FormalTranslator::removeCursorFromTopBlockNamespace(string curName, BlockPlSql *topBlock)
{
  Ptr<LevelResolvedNamespace> l = topBlock->getSemanticNode()->childNamespace;
  sAssert(!l);
  LevelResolvedNamespace::iterator it = l->find(curName);
  if (it != l->end()) {
    if (it->second) {
      for (auto &v : it->second->others)
        if (v)
          v->setVEntities(0);
    }
    l->erase(it);
  }
}

BlockPlSql *FormalTranslator::getTopBlock(Sm::PlExpr* expr)
{
  BlockPlSql *topBlock = 0;
  for (SemanticTree *n = expr->getSemanticNode(); n; n = n->getParent())
    if (n->unnamedDdlEntity && (topBlock = n->unnamedDdlEntity->toSelfBlockPlSql()))
      break;
  return topBlock;
}

// Преобразование курсоров в курсорные переменные
void FormalTranslator::translateCursorToCursorVariables(Sm::PlExpr* expr) {
  RefAbstract *ref;
  if (!(ref = expr->toSelfRefAbstract()))
    return;

  IdEntitySmart::iterator it;
  Sm::Cursor *cursor;

  for (it = ref->reference->begin(); it != ref->reference->end(); ++it)
    if (ResolvedEntity *d = (*it)->definition())
      if ((cursor = d->toSelfCursor()))
        break;

  if (it == ref->reference->end())
    return;

  if (cursor->beginedFrom(715007))
    cout << "";

  BlockPlSql *topBlock = getTopBlock(expr);
  sAssert(!topBlock || !(topBlock = topBlock->getTopBlock()));

  Ptr<CallArgList> savedCallArglist = (*it)->callArglist;
  std::pair<Cursor::InstanceMap::iterator, bool> res = cursor->instanceMap.insert(Cursor::InstanceMap::value_type(topBlock, 0));

  if (!res.second)
    *it = new Id(*(res.first->second->getName()), (*it)->getLLoc());
  else {
    string declVarName;

    string curName = cursor->getName()->toNormalizedString();

    BlockPlSql *topBlock;
    sAssert(!(topBlock = ref->ownerPlBlock()) ||
            !(topBlock = topBlock->getTopBlock()));

    Declarations::iterator cursorDeclIt = topBlock->declarations.end();
    ResolvedEntity *curdef = cursor->getDefinitionFirst();
    for (Declarations::iterator it = topBlock->declarations.begin(); it != topBlock->declarations.end(); ++it)
      if ((*it)->getDefinitionFirst() == curdef) {
        cursorDeclIt = it;
        break;
      }

    removeCursorFromTopBlockNamespace(curName, topBlock); // удаляем курсор из области видимости, если нужно

    Ptr<Variable> newVar = ref->addVariableIntoOwnerBlock(cursor->getDatatype(), &declVarName, (*it)->getLLoc(), curName, false, true);
    newVar->baseField = cursor;
    newVar->setLLoc(cursor->getLLoc());
    topBlock->declarations.insert(cursorDeclIt, 1, newVar.object());
    if (cursorDeclIt != topBlock->declarations.end()) {
      topBlock->removedCursors.push_back(*cursorDeclIt);
      topBlock->declarations.erase(cursorDeclIt);
    }

    *it = new Id(*(newVar->getName()));
    res.first->second = newVar;
  }
  if (savedCallArglist)
    (*it)->callArglist = savedCallArglist;

  sAssert(++it != ref->reference->end());
}

void FormalTranslator::translateStructuredFieldReference(Sm::PlExpr* expr) {
  VariableField *exprVar = 0;
  RefAbstract *ref;
  {
    ResolvedEntity *d;
    if (!(ref = expr->toSelfRefAbstract()) || !(d = ref->refDefinition()) ||
        !(exprVar = d->toSelfVariableField()) || !exprVar->isStructuredField) {
      if (exprVar && ref) // ??? удаление всех непоследних ссылок, являющихся структурированными полями.
        for (IdEntitySmart::iterator it = ref->reference->begin(); it != ref->reference->end(); ++it)
          if (ResolvedEntity *d = (*it)->definition())
            if (VariableField *fld = d->toSelfVariableField())
              if (fld->isStructuredField)
                it = ref->reference->erase(it);
      return;
    }
  }
  Ptr<IdEntitySmart> srcEntity = ref->reference;
  srcEntity->erase(srcEntity->begin());
  SemanticTree *n = exprVar->getSemanticNode();
  if (!n || !exprVar->isStructuredField)
    throw 999;

  StatementContext stmtContext = expr->getOwnerStatementContext();

  std::string declVarName;
  Ptr<Variable> newVar = ref->addVariableIntoOwnerBlock(exprVar->getDatatype(), &declVarName, exprVar->getLLoc(), "TmpSubfields");

  EntityFields lfields, rfields;
  newVar->getFields(lfields);
  exprVar->getFields(rfields);
  if (lfields.empty() || rfields.empty())
    throw 999;

  SemanticTree *ownerStmtNode = stmtContext.parentStmt->getSemanticNode();
  if (!ownerStmtNode)
    throw 999;

  EntityFields::iterator lIt = lfields.begin();
  EntityFields::iterator rIt = rfields.begin();

  Statements* ownerStmtList = stmtContext.parentStmt->getChildStatements();
  if (!ownerStmtList)
    throw 999;

  for (; lIt != lfields.end() && rIt != rfields.end(); ++lIt, ++rIt) {
      Ptr<IdEntitySmart> lEntity      = new IdEntitySmart(newVar->getName(), *lIt);
      Ptr<IdEntitySmart> srcReference = new IdEntitySmart(*srcEntity, *rIt);
      FLoc l = expr->getLLoc();
      Ptr<Assignment> assignment = new Assignment(l, new LValue(l, lEntity), new Sm::RefExpr(l, srcReference));

//     Ptr<Assignment> assignment = new Assignment(expr->getLLoc(), *lIt, *rIt);
     ownerStmtNode->addChild(assignment->toSTree());
     ownerStmtList->insert(stmtContext.positionInParent, Ptr<StatementInterface>(assignment.object()));
  }

  ref->reference = new IdEntitySmart(new Id(*(newVar->getName())));
}

bool FormalTranslator::translateQuery(PlExpr* expr) {
  if (Subquery *q = expr->toSelfSubquery()) {
    q->formalTranslations();
    return true;
  }
  return false;
}

PlExpr* FormalTranslator::translateFunctionCallToItsRealDatatype(PlExpr* expr) {
  RefExpr *s = expr->toSelfSqlExpr()->toSelfRefExpr();
  Sm::Function *unrefFun = s->refDefinition()->toSelfFunction();
  if (unrefFun->reducedRettype) {
    Sm::CastCathegory cat = unrefFun->reducedCathegory;
    if (SemanticTree* n = expr->getSemanticNode()) {
      if (n->isSqlCode())
        cat.setSqlCastState();
      else
        cat.setProcCastState();
    }
    else
      throw 999;
    cat.setIsCastFunctionCallsToReducedRettype();
    return CommonDatatypeCast::cast(s, unrefFun->getRettype(), unrefFun->reducedRettype.object(), cat);
  }
  else
    return expr;
}


void Sm::QueryBlock::extractLimit() {
  if (!where)
    return;

  int isNot = 0;

  bool deleted = true;
  Ptr<Sm::pl_expr::Comparsion> oldCmp;


  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (expr->beginedFrom(120152))
      cout << "";
    isNot = isNot  ^ expr->isNotAsInt();
    if (!construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;

    if (Subquery *q = expr->toSelfSubquery()) {
      q->extractLimitInQueryBlocks();
      return FLAG_REPLACE_SKIP_DEPTH_TRAVERSING | FLAG_REPLACE_SKIP_SECOND_CONDITION;
    }
    else {
      Ptr<Sm::pl_expr::Comparsion> cmp;
      bool ok = false;
      PlExpr *b = expr->unwrapBrackets(&isNot);
      if (b->isLimitExpr()) {
        cmp = b->toSelfComparion();
        ok = true;
      }
      else if (pl_expr::LogicalCompound *c = b->toSelfLogicalCompound()) {
        if (c->lhs->unwrapBrackets(&isNot)->isLimitExpr())  {
          cmp = c->lhs->unwrapBrackets(&isNot)->toSelfComparion();
          ok  = true;
        }
        if (c->rhs->unwrapBrackets(&isNot)->isLimitExpr())  {
          cmp = c->rhs->unwrapBrackets(&isNot)->toSelfComparion();
          ok  = true;
        }
      }
      if (ok) {
        if (!cmp)
          throw 999;

        if (expr->beginedFrom(890,11))
          cout << "";
        if (limit) {
          if (cmp && cmp == oldCmp)
            return FLAG_REPLACE_TRAVERSE_NEXT | FLAG_REPLACE_SKIP_SECOND_CONDITION;
          else
            throw 999;
        }
        setLimitExpr(cmp, isNot);
        oldCmp = cmp;
        deleted = false;
        return FLAG_REPLACE_TRAVERSE_NEXT | FLAG_REPLACE_NEED_TRANSLATION | FLAG_REPLACE_SKIP_SECOND_CONDITION;
      }
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  bool whereDelete = false;
  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr*  {
    PlExpr *newExpr = 0;
    PlExpr *br = e->unwrapBrackets();
    if (pl_expr::LogicalCompound *c = br->toSelfLogicalCompound()) {
      if (c->lhs->isLimitExpr())
        newExpr = c->rhs;
      else if (c->rhs->isLimitExpr())
        newExpr = c->lhs;
    }
    if (newExpr) {
      deleted = true;
      return newExpr;
    }
    else if (br->isLimitExpr()) {
      whereDelete = true;
      return e;
    }
    throw 999;
  };
  ExprTR tr(trFun, cond);
  replace(tr, where);

  if (whereDelete) {
    if (!where->isLimitExpr())
      throw 999;
    where = 0;
  }
  else if (!deleted)
    throw 999;

}

FunctionArgument *FormalTranslator::extractFunDef(Ptr<Id> &call, unsigned int pos, Function **funDef)
{
  ResolvedEntity *def = call->definition();
  if (!def)
    return 0;

  Function *fun = def->toSelfFunction();
  if (!fun || !(fun->getDefinitionFirst()) || pos >= fun->arglistSize())
    throw 999;

  FunctionArgument *funArg = *(fun->arglistBegin() + pos);
  if (!funArg)
    throw 999;

  if (funDef)
    *funDef = fun;
  return funArg;
}


void FormalTranslator::translateCallWithRefcursorArg(Sm::PlExpr *expr, Ptr<Id> &call, unsigned int pos, VariableCursorFields *srcVar) {
  (void)expr; // for debug
  Function *fun;
  if (FunctionArgument *fundeclArg = extractFunDef(call, pos, &fun)) {
    if (fundeclArg->out())
      fun->getFieldsFromOutRefcursor(srcVar, pos, &(srcVar->fieldsSource));
    if (!srcVar->fields_.empty()) {
      if (fundeclArg->fields_.empty())
        fundeclArg->fields_ = srcVar->fields_;
      else
        fundeclArg->checkFieldsConsistance(srcVar->fields_, srcVar->getLLoc());
    }
    else
      cout << "ERROR: empty srcVar->fields " << srcVar->getLLoc()
           << " for function argument "      << fundeclArg->getLLoc() << endl;
  }
}

// Преобразование вызовов функций с аргументами-рефкурсорами и проверка согласованности полей в других вызовах
void FormalTranslator::translateFunctionCallsWithRefcursorArgs(Sm::PlExpr *expr) {
  if (Ptr<IdEntitySmart> ref = expr->getMajorIdEntity())
    for (IdEntitySmart::value_type &call : *ref)
      if (call->callArglist) {
        if (call->beginedFrom(506847))
          cout << "";
        unsigned int pos = 0;
        for (CallArgList::value_type &arg : *(call->callArglist)) {
          if (arg) {
            IdEntitySmart *argRef;
            ResolvedEntity *varDef;
            VariableCursorFields *varFlds;
            Datatype *t;

            if ((argRef = arg->expr()->getMajorIdEntity().object()) &&
                (varDef = argRef->definition()) &&
                (varFlds = varDef->toSelfVariableCursorFields()) &&
                (t = varFlds->getDatatype()) &&
                t->isRefCursor())
              translateCallWithRefcursorArg(expr, call, pos, varFlds);
            else if (CursorExpr *c = arg->expr()->toSelfCursorExpr()) {
              EntityFields f;
              c->cursor->getFields(f);

              if (f.empty())
                throw 999;

              FunctionArgument *fundeclArg = extractFunDef(call, pos, 0);
              if (fundeclArg->out() && !fundeclArg->in())
                throw 999;

              if (fundeclArg->fields_.empty())
                fundeclArg->setFieldsFrom(f, c->cursor.object());
              else
                fundeclArg->checkFieldsConsistance(f, c->getLLoc());
            }
          }
          ++pos;
        }
      }
}

bool Sm::Function::castArgIfNeed(Ptr<Sm::FunCallArg> arg, Ptr<Datatype> needType) {
  if (needType->isCompositeType() || needType->isRefCursor())
    return false;

  bool isSql = arg->expr()->getSemanticNode()->isSqlCode();
  Ptr<Datatype> argType = SubqueryUnwrapper::unwrap(arg->getDatatype());
  if (!argType)
    return false;
  CastCathegory cat = needType->getCastCathegory(argType, !isSql, isSql);
  if (cat.explicitAll()) {
    if (isSql)
      cat.setSqlCastState();
    else
      cat.setProcCastState();
    cat.castRhsToLhs();
    Ptr<PlExpr> newExpr = CommonDatatypeCast::cast(arg->expr(), argType.object(), needType.object(), cat);
    if (newExpr && newExpr != arg->expr()) {
      arg->setExpr(newExpr);
      return true;
    }
  }
  return false;
}

void Sm::Function::formalTranslateArgs(Ptr<Sm::Id> call) {
  if (!arglist || !call->callArglist || !call->callArglist->size())
    return;

  if (call->beginedFrom(28237))
    cout << "";

  Ptr<Sm::CallArgList> callArgs = call->callArglist;
  Sm::Arglist::iterator aIt = arglist->begin();
  Sm::CallArgList::iterator cIt = callArgs->begin();
  for ( ; aIt != arglist->end() && cIt != callArgs->end(); ++aIt, ++cIt) {
    switch ((*cIt)->argclass()) {
      case Sm::FunCallArg::ASTERISK:
        throw 999; // Необходимо добавить поддержку.
        break;
      case Sm::FunCallArg::POSITIONAL:
        castArgIfNeed(*cIt, (*aIt)->getDatatype());
        break;
      case Sm::FunCallArg::NAMED: {
        if (!(*cIt)->argname())
          throw 999; // должно быть задано для именованного аргумента
        for (Sm::FunArgList::iterator it = arglist->begin(); it != arglist->end(); ++it)
          if (*((*cIt)->argname()) == *((*it)->getName())) {
            castArgIfNeed(*cIt, (*it)->getDatatype());
          }
      } break;
      default:
        break;
    }
  }
}

// Приведение типов фактических аргрументов к типам формальных в вызовах функций и в коллекциях.
bool FormalTranslator::translateFunctionArgs(PlExpr *expr) {
  if (Ptr<IdEntitySmart> ref = expr->getMajorIdEntity())
     for (IdEntitySmart::value_type &call : *ref) {
       ResolvedEntity *def = call->definition();
       if (!def)
         continue;
       if (def->isCollectionMethod() || def->isCollectionAccessor())
         def->formalTranslateArgs(call);
       else if (def->isFunction() && !def->isSystem() && !def->isSystemTemplate())
         def->formalTranslateArgs(call);
     }

  return false;
}


int FormalTranslator::translateReferencesCondition(PlExpr* expr, FormalTranslator::Translator &translator) {
  if (syntaxerContext.unwrapStructuredFields)
    translateStructuredFieldReference(expr);
  translateCursorToCursorVariables(expr);
  translateFunctionCallsWithRefcursorArgs(expr);
  translateFunctionArgs(expr);
  if (expr->getReducedRettype()) { // Приведение типа результата вызовов функций, для определений которых установлен сжатый тип
    translator = FormalTranslator::translateFunctionCallToItsRealDatatype;
    return FLAG_REPLACE_NEED_TRANSLATION;
  }
  if (AlgebraicCompound *c = expr->toSelfAlgebraicCompound()) { // приведение типов операндов арифметического выражения
    c->translateToConsistantOperand();
    return FLAG_REPLACE_TRAVERSE_NEXT;
  }
  if (pl_expr::Comparsion *c = expr->toSelfComparion()) {
    c->translateToConsistantOperand();
    return FLAG_REPLACE_TRAVERSE_NEXT;
  }
  if (Sm::Case *c = expr->toSelfCase()) {
    c->translateToConsistantOperand();
    return FLAG_REPLACE_TRAVERSE_NEXT;
  }
  if (pl_expr::Between *c = expr->toSelfBetween()) {
    c->translateToConsistantOperand();
    return FLAG_REPLACE_TRAVERSE_NEXT;
  }
  if (translateReference(expr)) // Трансляция закавыченности псевдополей триггеров
    return FLAG_REPLACE_TRAVERSE_NEXT;
  if (translateQuery(expr)) // Трансляция запросов
    return FLAG_REPLACE_TRAVERSE_NEXT;
  return FLAG_REPLACE_TRAVERSE_NEXT;
}


void FormalTranslator::translateReferences() {
  Translator translator = 0;
  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (expr->beginedFrom(86572) && expr->toSelfSelectSingle())
      cout << "";
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;
    return  translateReferencesCondition(expr, translator);
  };

  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr*  {
    if (translator) {
      PlExpr *newExpr = translator(e);
      translator = 0;
      return newExpr;
    }
    else
      throw 999;
    return e;
  };
  ExprTR tr(trFun, cond);
  cntx_.replaceChildsIf(tr);
}

void FormalTranslator::setFieldsOnUnopenedVariables(CursorDecltypes &cursorDecltypes) {
  for (auto &v : cursorDecltypes)
    if (ResolvedEntity *d = v->varName->definition())
      if (Variable *var = d->toSelfVariable())
        if (var->fields_.empty())
          v->select->getFields(var->fields_);
}




void FormalTranslator::setTranslatedName(ResolvedEntity *def, bool isPackageVairable, std::vector<Id*> &nameComponents, UserContext *currentUser) {
  if (def->beginedFrom(1463583,13))
    cout << "";
  bool q = false;

  string fname;
  nameComponents.push_back(def->getName());
  for (Id *n : nameComponents)
    if (n->quoted() || n->hasSpecSymbols()) {
      q = true;
      break;
    }
  for (Id *n : nameComponents) {
    fname.append(q ? n->toNormalizedString() : n->toString());
    fname.push_back('_');
  }
  if (fname.empty())
    throw 999;
  fname.pop_back();
  nameComponents.pop_back();

  if (def->isConstructor())
    q = false; // Некоторые пользовательские классы объявлены в коде закавыченными,
               // но для обратной совместимости уберем эти кавычки.

  if (!q) {
    std::string copy = fname;
    transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
    if (Sm::Id::isFieldReserved(copy)) {
      if (isPackageVairable) {// пакетную переменную, совпадающую с ключевым словом - нужно переименовывать
        if (nameComponents.size() > 1)
          throw 999;
        if (Ptr<LevelResolvedNamespace> space = def->levelNamespace())
          fname = nameComponents[0]->toCodeId(space);
        else
          throw 999;
      }
      else {
        fname.clear();
        PlsqlHelper::quotingAndEscaping(fname, copy);
      }
    }
  }
//  else {
//    string copy = fname;
//    fname.clear();
//    PlsqlHelper::quotingAndEscaping(fname, copy);
//  }

  // Линтер 6.0 содержит ограничение длины идентификаторов в 66 символов.
  // Ограничение накладывает длина имени в системном словаре $$$PROC
  // Возможно в будущих версиях это ограничение будет снято.
  const size_t maxIdSize = 66;
  if (fname.length() > maxIdSize)
    fname.resize(maxIdSize);

  def->translatedName(fname);
  def->userOwner(currentUser);

  if (def->isVariable()) {
    fname.erase(std::remove(fname.begin(), fname.end(), '\"'), fname.end());
    std::transform(fname.begin(), fname.end(), fname.begin(), [](char ch) -> char { return (ch == '$') ?  '_' : ch; });
    def->procTranslatedName(fname);
  }
}


void FormalTranslator::traverseUserDeclarations(DeclActor tr, UserContext **currentUser)
{
  for (UserDataMap::value_type &v : cntx_.userMap)
    if (!cntx_.sysusers.count(v.second)) {
      if (currentUser)
        *currentUser = v.second;
      v.second->traverseDeclarations(tr);
    }
}

void FormalTranslator::traverseAllDeclarations(DeclActor tr, UserContext **currentUser)
{
  for (UserDataMap::value_type &v : cntx_.userMap) {
    if (currentUser)
      *currentUser = v.second;
    v.second->traverseDeclarations(tr);
  }
}

void FormalTranslator::initializeDeclarationsNames() {
  std::vector<Id*> nameComponents; // склеивать их знаком _ - от первого к последнему
  std::stack<BlockPlSql*> ownerBlocks;
  UserContext *currentUser = 0;

  DeclActor *handleDeclPtr;
  auto traverseSubdecls = [&](Declaration* decl) -> bool {
    nameComponents.push_back(decl->getName());
    decl->traverseDeclarationsForce(*handleDeclPtr);
    nameComponents.pop_back();
    return false;
  };

  bool funEntered = false;
  Sm::DeclActor handleDecl = [&](Declaration* decl) -> bool {
    if (!decl)
      return true;
    else if (BlockPlSql *blk = decl->toSelfBlockPlSql())
      return blk->traverseDecls(handleDecl, ownerBlocks);
    else if (decl->toSelfPackage())
      return traverseSubdecls(decl);
    else if (Sm::Type::Object *obj = decl->toSelfObject()) {
      if (obj->isBody()) {
        decl->traverseDeclarationsForce(handleDecl);
        return false;
      }
      else
        return traverseSubdecls(decl);
    }
    else if (Cursor *cur = decl->toSelfCursor()) {
      if (cur->select)
        cur->select->extractLimitInQueryBlocks();
      return true;
    }
    else if (Function *fun = decl->toSelfFunction()) {
      if (fun->isSystem() && !fun->isTrNameEmpty())
        return false;

      setTranslatedName(fun, false, nameComponents, currentUser);

      if (fun->isSystem())
        return false;

      if (!funEntered) {
        funEntered = true;
        traverseSubdecls(decl);
        funEntered = false;
      }
      else
        traverseSubdecls(decl);

      if (fun->overloadedId() < 0)
        fun->identificateOverloaded();
      return false;
    }
    else if (Type::MemberVariable *var = decl->toSelfMemberVariable()) {
      std::vector<Id*> localComponents;
      setTranslatedName(var, true, localComponents, currentUser);
    }
    else if (Variable *var = decl->toSelfVariable()) {
      if (funEntered) {
        var->userOwner(0);
        if (ownerBlocks.empty())
          throw 999;
        var->setOwnerBlockPlSql(ownerBlocks.top());
        var->generateUniqueName();
      }
      else
        setTranslatedName(var, true, nameComponents, currentUser);
    }
    return true;
  };
  handleDeclPtr = &handleDecl;

  traverseAllDeclarations(handleDecl, &currentUser);
}



void FormalTranslator::transformFunctions() {
  Sm::DeclActor tr = [&](Declaration* decl) -> bool {
    if (decl)
      if (Sm::Function *fun = decl->toSelfFunction()) {
        //        fun->translateOutCursorInArglist(); // это нужно переделать
        fun->castReturnExpressionsToRettype();
        fun->castFuncArgDefaultValue();
      }
    return true;
  };

  traverseUserDeclarations(tr);
}


void FormalTranslator::translateVariablesDynamicInitializers() {
  std::stack<BlockPlSql*> ownerBlocks;
  DeclActor *handleDeclPtr;

  Sm::DeclActor handleDecl = [&](Declaration* decl) -> bool {
    if (!decl)
      return true;
    if (BlockPlSql *b = decl->toSelfBlockPlSql())
      return b->traverseDecls(*handleDeclPtr, ownerBlocks);
    else if (Type::MemberFunction *mf = decl->toSelfMemberFunction())
      mf->makeFormalTranslations();
    else if (FunctionArgument *v = decl->toSelfFunctionArgument())
      v->translateRefCursorUsing();
    else if (Variable *v = decl->toSelfVariable()) {
      if (v->translateRefCursorUsing())
        return true;
      BlockPlSql *ownerBlock = v->ownerPlBlock();
      if (ownerBlocks.empty() || (ownerBlock != ownerBlocks.top() && ownerBlock != ownerBlocks.top()->packageBody_) ) {
        cout << "ERROR: bad owner block for variable " << v->getName()->getText() << ": " << v->getLLoc() << endl;
        return true;
      }
      if (v->flags.isDynamicLoopCounter())
        return true;
      Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(v->getDatatype());
      v->explicitCast();
      Ptr<PlExpr> defaultValue = v->defaultValue();
      // Если переменная с динамической инициализацией, добавим ее в блок инициализаторов
      if (v->hasDynamicDefaultValue())
         ownerBlock->addToInitializators(v, defaultValue);
      else if (v->hasObjectType()) {
        // Добавление конструктора по умолчанию
        // TODO: проверить наличие вызова в секции BlockPlSql
        if (Ptr<Function> ctor = t->getDefaultConstructor())
          ownerBlock->addToInitializators(v, new RefExpr(v->getLLoc(), ctor->getName()));
        else {
          Codestream str;
          trError(str, s << "ERROR: datatype hasn't the default constructor: tid = " << *(t->tid) << ":" << t->getLLoc());
          cout << str.str();
        }
      }
    }
    return true;
  };
  handleDeclPtr = &handleDecl;
  cntx_.traverseDeclarations(handleDecl);
}

bool FormalTranslator::translateRefcursorAssignments(Assignment *assignment) {
  if (refcursorAssignment.count(assignment))
    return false;

  ResolvedEntity *def = assignment->lValue->refDefinition();
  if (!def)
    return false;

  if (Ptr<Datatype> t = def->getDatatype())
    if (!t->isRefCursor())
      return false;

  VariableCursorFields *changedCursor = def->toSelfVariableCursorFields();
  if (!changedCursor || changedCursor->fields_.size())
    throw 999;

  ResolvedEntity *funDef = nullptr;
  if (RefExpr *ref = assignment->assignedExpr->toSelfRefExpr())
    funDef = ref->refDefinition();
  else
    throw 999;

  if (!funDef)
    return false;

  Function *fun = 0;
  if (!(fun = funDef->toSelfFunction())) {
    cout << "error: translateRefcursorAssignments - function is not function, cat=" << Sm::toString(funDef->ddlCathegory()) << "  " << assignment->getLLoc() << endl;
    return false;
  }

  Ptr<Datatype> funtype = funDef->getDatatype();

  if (funtype->isRefCursor()) {
//    EntityFields flds;
    fun->getFieldsFromReturnRefcursor(changedCursor, &(changedCursor->fieldsSource));
//    if (flds.empty())
//      throw 999;
//    changedCursor->setFieldsFrom(flds, changedCursor->fieldsSource);
  }
  else {
    EntityFields flds;
    funtype->getFields(flds);
    changedCursor->fieldsSource = funtype.object();
    if (flds.empty())
      throw 999;
    changedCursor->setFieldsFrom(flds, changedCursor->fieldsSource);
  }


  refcursorAssignment.insert(assignment);
  return true;
}

bool FormalTranslator::translateRefcursorReturn(Sm::Function *fun) {
  if (fun->isSystem() || fun->isSystemTemplate() || !fun->isDefinition())
    return false;

  Ptr<Datatype> funtype = fun->getDatatype();
  if (funtype && funtype->isRefCursor() && fun->fields_.size() == 0) {
    fun->getFieldsFromReturnRefcursor(fun, &(fun->fieldsSource));
    return true;
  }
  return false;
}

bool containsEntityAsReference(SemanticTree *n, ResolvedEntity *ref) {
  if (!n->reference())
    return false;
  IdEntitySmart::iterator endIt = n->refEnd();
  if (n->reference())
    for (IdEntitySmart::iterator it = n->refBegin(); it != endIt; ++it) {
      if (ResolvedEntity *ent = (*it)->unresolvedDefinition())
        if (ent->eqByVEntities(ref))
          return true;
      for (SemanticTree::Childs::value_type c : n->childs)
        if (containsEntityAsReference(c, ref))
          return true;
    }
  return false;
}

bool VariableCursorFields::checkConsistanseForOtherFieldList(Statement *stmt, bool outError) {
  bool consistance = true;
  EntityFields f;
  stmt->getFields(f);
  if (f.empty())
    return true;
  if (fields_.empty())
    setFieldsFrom(f, stmt);
  else if (fields_.size() != f.size()) {
    consistance = false;
    if (outError)
      cout << "ERROR unequally fields length for new value of cursor variable: " << stmt->getLLoc() << endl;
  }
  else {
    EntityFields::iterator it = fields_.begin();
    EntityFields::iterator fldIt = f.begin();
    for ( ; it != fields_.end() && fldIt != f.end(); ++it, ++fldIt) {
      ResolvedEntity *entIt  = (*it)->definition();
      ResolvedEntity *entFld = (*fldIt)->definition();
      Datatype *entT = 0, *fldT = 0;
      if (!entIt || !entFld ||
          !(entT = entIt ->unwrappedDatatype()) || !(fldT = entFld->unwrappedDatatype())) {
        consistance = false;
        if (outError)
          cout << "ERROR unresolved entities for new value of cursor variable: " << stmt->getLLoc() << endl;
      }
      else {
        CastCathegory c = entT->getCastCathegory(fldT, true);
        // implicitAlmost - сравнивает <FOO> и NULL как одинаковые
        // decimalLength  - сравнивает NUMBER(X, Y) и NUMBER(Z, F) (Y != 0, F != 0) как одинаковые
        if (!c.implicitAlmost() && !c.decimalLength()) {
          consistance = false;
          if (outError)
            cout << "ERROR inconsistance datatypes for new value of cursor variable: " << stmt->getLLoc() << endl;
        }
      }
    }
  }
  return consistance;
}


void VariableCursorFields::initFieldsFromOpenFor(OpenFor *openFor, FoundedStatements::iterator it, FoundedStatements &stmts) {
  if (fields_.empty()) {
    EntityFields f;
    openFor->getFields(f);
    setFieldsFrom(f, openFor);
  }
  else {
    if (this->toSelfResolvedEntity()->eid() == 1070010 &&
        openFor->eid() == 1069549)
      cout << "";
    bool consistance = checkConsistanseForOtherFieldList(openFor, false);
    ResolvedEntity *self = getVarName()->definition();
    if (!consistance) {
      string baseName = getVarName()->toNormalizedString();
      string strVar;
      if (BlockPlSql *blk = getOwnerBlk()) {
        Ptr<Sm::Variable> var = blk->addVariableIntoOwnerBlock(openFor->select->getDatatype(), &strVar, getLLoc(), baseName);
        syntaxerContext.model->delayDeletedExpressions.push_back(openFor->baseCursorVariable.object());
        openFor->baseCursorVariable = new RefExpr(var->getLLoc(), new Id(*(var->getName())));
        openFor->openForIterVariable = var->getName();
        for ( ; it != stmts.end(); ++it) {
          if (Statement *stmt = (*it)->toSelfStatement()) {
            if (stmt->toSelfOpenFor())
              break;
            else if (Fetch *f = stmt->toSelfFetch()) {
              if (f->cursorRef->refDefinition() && f->cursorRef->refDefinition()->eqByVEntities(self))
                f->cursorRef->reference->back() = var->getName();
            }
            else if (Close *c = stmt->toSelfClose()) {
              if (c->cursorEntity->definition() && c->cursorEntity->definition()->eqByVEntities(self))
                c->cursorEntity = var->getName();
            }
          }
          // TODO: переменную нужно переименовывать и в выражениях
        }
      }
      else
        cout << "Error: owner block is null. Cursor changing unimplemented for Function Argument " << getVarName()->toNormalizedString() << getLLoc() << endl;
    }
  }
  if (openFor->decltypeSelectStmts.size())
    for (auto &v : openFor->decltypeSelectStmts)
      checkConsistanseForOtherFieldList(v);
}



bool VariableCursorFields::translateRefCursorUsing() {
  if (getLLoc().beginedFrom(1474166))
    cout << "";
  if (!getVariableDatatype()->isRefCursor() || flags.isCursorCallsResolved())
    return false;
  flags.setCursorCallsResolved();
  BlockPlSql *blk = getOwnerBlk();
  if (!blk)
    return true;

  FoundedStatements usedStatements;
  ResolvedEntity *var = getVarName()->definition();
  enumUseVarStatements(var, blk, usedStatements);

  for (FoundedStatements::iterator it = usedStatements.begin(); it != usedStatements.end(); ++it) {
    if ((*it)->isOpenStatement() && (*it)->openedEntity() == var) {
      if (OpenFor *openFor = (*it)->toSelfStatement()->toSelfOpenFor()) {
        if (openFor->beginedFrom(28781))
          cout << "";
        initFieldsFromOpenFor(openFor, it, usedStatements);
      }
    }
    else if ((*it)->isFetchStatement() && fields_.empty())
      (*it)->getFields(fields_);
    else {
      QueriesContainer qContainer;
      (*it)->getCursorOpenFromCalledFunction(var, qContainer);
      if (qContainer.size()) {
        if (!blk)
          throw 999;

        for (auto &v : qContainer) {
          if (Sm::OpenFor *f = v->toSelfOpenFor())
            initFieldsFromOpenFor(f, it, usedStatements);
          else
            checkConsistanseForOtherFieldList(v);
        }
      }
    }
  }
  return true;
}


void StatementInterface::getCursorOpenFromCalledFunction(ResolvedEntity *var, QueriesContainer &container) {
  FunctionCalls calls;
  getFunctionCallsThatContainsEntityAsArgument(var, calls);
  for (const FunctionCalls::value_type &v : calls)
    if (ResolvedEntity *d = v.first->definition())
      if (Sm::Function *f = d->toSelfFunction())
        f->getQueriesForRefCursor(v.second, container);
}

void Sm::StatementInterface::getFunctionCallsThatContainsEntityAsArgument(ResolvedEntity *entity, FunctionCalls &outList) {
  if (SemanticTree *node = this->getSemanticNode()) {
    CommonDatatypeCast::CastContext exprContext;
    node->getExpressionContext(exprContext);
    if (exprContext.functionDefinitions.size())
      node = exprContext.functionDefinitions.back()->getSemanticNode();
    node->getFunctionCallsThatContainsEntityAsArgument(entity, outList);
  }
}

void Sm::SemanticTree::getFunctionCallsThatContainsEntityAsArgument(ResolvedEntity *entity, FunctionCalls &outList, Ptr<Id> fun) {
  if (Id *name = refEntity()) {
    int pos = 0;
    if (name->callArglist)
      for (CallArgList::value_type &it : *(name->callArglist)) {
        if (ResolvedEntity *def = it->getNextDefinition())
          if (def->eqByVEntities(entity))
            outList.insert(std::make_pair(name, it->getArgPositionInDef(pos)) );
        ++pos;
      }
  }

  for (Childs::value_type &it : childs)
    it->getFunctionCallsThatContainsEntityAsArgument(entity, outList, fun);
}


void Function::getQueriesForRefCursor(int posInArglist, QueriesContainer &queries) {
  Sm::Function *fun = this;
  if (!body_)
    fun = getDefinitionFirst()->toSelfFunction();
  if (!fun || !fun->body_)
    return;

  ResolvedEntity *refCursorVar = 0;
  if (fun->outRefCursors.size())
    throw 999;
  if (arglist && !refCursorVar)
    for (Arglist::value_type &a : *(fun->arglist))
      if (a->positionInArglist == posInArglist)
        refCursorVar = a.object();
  if (!refCursorVar)
    return;

  using namespace std::placeholders;
  Sm::CollectUsingByStatements::StmtsStack      statementsStack;
  Sm::ExprTR::Cond cond = [&](PlExpr *expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;
    if (statementsStack.empty())
      return FLAG_REPLACE_TRAVERSE_NEXT;
    StatementInterface *lastStmt = *(statementsStack.begin());
    if (Sm::Statement *stmt = lastStmt->toSelfStatement()) {
      if (Sm::OpenCursor *openCursor = stmt->toSelfOpenCursor()) {
        if (ResolvedEntity *cursorDef = openCursor->cursor->getNextDefinition())
          if (cursorDef->eqByVEntities(refCursorVar))
            if (cursorDef->getSelectQuery())
              queries.push_back(openCursor);
      }
      else if (Sm::OpenFor *openFor = stmt->toSelfOpenFor()) {
        if (ResolvedEntity *cursorDef = openFor->baseCursorVariable->getNextDefinition())
          if (cursorDef->eqByVEntities(refCursorVar))
            queries.push_back(openFor);
      }
      else if (Sm::RefExpr *ref = expr->toSelfRefExpr()) {
        if (ResolvedEntity *def = ref->getNextDefinition())
          if (Sm::Function *f = def->toSelfFunction())
            if (Sm::SemanticTree *n = ref->getSemanticNode()) {
              FunctionCalls outList;
              n->getFunctionCallsThatContainsEntityAsArgument(refCursorVar, outList);
              if (outList.size()) {
                if (outList.size() != 1)
                  throw 999;
                f->getQueriesForRefCursor(outList.begin()->second, queries);
              }
            }
      }
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  CollectUsingByStatements tr(cond, &statementsStack);
  fun->body_->replaceChildsIf(tr);
}




void ModelContext::setPackagesAttributesOnBlocks() {
  Sm::DeclActor handleDecl = [&](Declaration* decl) -> bool {
    if (decl)
      if (Package *b = decl->toSelfPackage())
        b->setPackageAttributesForBlocks();
    return true;
  };
  traverseDeclarations(handleDecl);
}


void SemanticTree::translateUsageFunWithOutCursor(VEntities* funs) {
  switch (nametype) {
    case REFERENCE:
    case EXTDB_REFERENCE:
      if (unnamedDdlEntity && funs->count(unnamedDdlEntity))
        throw 999;
      else if (ResolvedEntity *def = ddlEntity())
        if (funs->count(def)) {
          if (unnamedDdlEntity && unnamedDdlEntity->isFunCallStatement())
            unnamedDdlEntity->setTranslateToRefCursorAssignment();
          else
            throw 999;
        }
      break;
    default:
      break;
  }
  for (SemanticTree *c : childs)
    c->translateUsageFunWithOutCursor(funs);
}



void SemanticTree::collectReferencesForCastedCalls() {
  if (nametype == REFERENCE)
    if (ResolvedEntity *d = unnamedDdlEntity)
      if (RefExpr* s = d->toSelfRefExpr())
        if (ResolvedEntity *unref = s->refDefinition())
          if (Sm::Function *unrefFun = unref->toSelfFunction())
            referencesForCastedCalls[unrefFun->getDefinitionFirst()].insert(d);

  for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::collectReferencesForCastedCalls));
}

void SemanticTree::castFunctionCallsToReducedDatatype(Sm::Function *fun) {
  if (!fun->reducedRettype)
    return;

  if (referencesForCastedCalls.empty())
    collectReferencesForCastedCalls();

  ReferencesForCastedCalls::iterator it = referencesForCastedCalls.find(fun->getDefinitionFirst());
  if (it != referencesForCastedCalls.end())
    for (ReferencesForCastedCalls::mapped_type::iterator mIt = it->second.begin(); mIt != it->second.end(); ++mIt)
      if (RefExpr* s = (*mIt)->toSelfRefExpr())
        CommonDatatypeCast::cast(s, fun->getRettype(), fun->reducedRettype.object(), fun->reducedCathegory);
}


SqlExpr* CommonDatatypeCast::convertExperssion(Sm::PlExpr *castedExpr, Datatype *newType, string nameOfConvFun, Datatype *castedType, const FLoc flc)
{
  typedef map<string, Ptr<Function> > NamedFuns;
  typedef map<Datatype*, map<Datatype*,  NamedFuns>, LE_ResolvedEntities> Funs;
  static Funs funs;
  Ptr<Function> f;
  Funs::iterator it = funs.find(newType);
  if (it != funs.end()) {
    Funs::mapped_type::iterator it2 = it->second.find(castedType);
    if (it2 != it->second.end()) {
      NamedFuns::iterator fIt = it2->second.find(nameOfConvFun);
      if (fIt != it2->second.end())
        f = fIt->second;
    }
  }

  if (!f) {
    typedef FunctionArgumentContainer Arg;
    f = new Sm::Function(nameOfConvFun, { Arg("value", castedType) }, newType);
    f->setIsSystem();
    f->setElementaryLinFunc();
    funs[newType][castedType][nameOfConvFun] = f;
  }

  if (nameOfConvFun.size()) {
    Id* n = new Id(flc, nameOfConvFun, {castedExpr});
    n->definition(f.object());
    return new Sm::RefExpr(flc, n);
  }
  else {
    Sm::Codestream s;
    s << "ERROR: unimplemented explicit casting from "  << castedType << " to " << newType;
    cout << s.str() << endl;
    throw 999;
  }
  return 0;
}

void Sm::CommonDatatypeCast::getTypeCat(PlExpr *expr, Datatype *t, TypeCathegory &tCat, NumberCathegory &nCat) {
  nCat = EMPTY;
  if (expr) {
    if      (expr->isIntValue() && expr->getUIntValue() == 0) { tCat = NUMERIC_VALUE_0; return; }
    if      (expr->isNumericValue()   ) { tCat = NUMERIC_VALUE; return; }
    else if (expr->isEmptyId()        ) { tCat = EMPTY_ID; return; }
    else if (expr->isQuotedSqlExprId()) { tCat = QUOTED_SQL_EXPR_ID; return; }
  }
  if (!t) {
    cout << "error:  getTypeCat:  t == nullptr" << endl;
    return;
  }
  if (t->isVarcharDatatype())
    tCat = CHAR_VARCHAR;
  else if (t->isNcharDatatype())
    tCat = NCHAR;
  else if (t->isNVarcharDatatype())
    tCat = NVARCHAR;
  else if (t->isSmallint()) {
    tCat = LIKE_NUMBER;
    nCat = SMALLINT;
  }
  else if (t->isInt()) {
    tCat = LIKE_NUMBER;
    nCat = INT;
  }
  else if (t->isBigint()) {
    tCat = LIKE_NUMBER;
    nCat = BIGINT;
  }
  else if (t->isNumberDatatype() || t->isIntervalDatatype()) {
    tCat = LIKE_NUMBER;
    nCat = NUMBER;
  }
  else if (t->isReal()) {
    tCat = LIKE_NUMBER;
    nCat = REAL_;
  }
  else if (t->isDouble()) {
    tCat = LIKE_NUMBER;
    nCat = DOUBLE;
  }
  else if (t->isDateDatatype())
    tCat = DATE;
  else if (t->isClobDatatype())
    tCat = CLOB;
  else if (t->isBool())
    tCat = BOOL_;
  else if (t->isNull())
    tCat = NULLCAT;
  else if (t->isBlobDatatype())
    tCat = CLOB;
}


extern LexInputData pLexInputData;


void CommonDatatypeCast::castQuotedIdToNumber(
    SqlExpr       **newExpr,
    Sm::PlExpr     *castedExpr,
    Datatype       *newType,
    Datatype       *castedType,
    CastCathegory   castCathegory,
    NumberCathegory newNumCat,
    FLoc            flc)
{
  RefExpr *expr = castedExpr->unwrapRefExpr();
  if (!expr)
    throw 999;
  if (expr->beginedFrom(829482,22))
    cout << "";

  std::string text = (*(expr->reference->begin()))->toString();
  if (text.empty()) {
    Sm::NumericInt *val = new Sm::NumericInt(castedExpr->getLLoc(), 0);
    val->setSemanticNode(castedExpr->getSemanticNode());
    *newExpr = val;
    return;
  }
  text.push_back(' ');
  text.push_back(' ');
  text.push_back(' ');

  YYSTYPE un;
  yyscan_t *scanner;
  cl::location loc;

  YY_BUFFER_STATE curBuf = init_smart_lexer(Sql, /*file = */0, scanner, loc);
  LexInputData oldLexBufFun = pLexInputData; // подмена функции лексера, читающей данные из файла на функцию, читающую из строки text

  SmartLexer::StrReader strReadFun(text);
  pLexInputData = strReadFun;
  bool neg = false;

  while (1) {
    yy::parser::token::yytokentype state = yylex(&un, &loc, scanner);
    if (!state)
      throw 999;
    switch ((int)state) {
    case (int)yy::parser::token::RawID: {
      delete un.id;
      // todo: cast to null; NULL;
      castToNull(newExpr, castedExpr);
      break;
    }
    case (int)('-'):
      neg = !neg;
      continue;
    case (int)yy::parser::token::NUMERIC_ID: {
      Sm::NumericValue *val = un.numericValue;
      val->setSemanticNode(castedExpr->getSemanticNode());
      *newExpr = val;
      if (neg)
        val->neg();
      switch (val->cathegory()) {
      case NumericValue::INTVAL:
        throw 999;
      case NumericValue::SIMPLE_INT:
        break;
      case NumericValue::FLOATVAL:
        switch (castCathegory.castState()) {
        case CastCathegory::SQL:
          switch (newNumCat) {
          case INT:
          case SMALLINT:
            *newExpr = new Sm::Cast(flc, val, newType);
            break;
          default:
            break;
          }
          break;
          case CastCathegory::PROC:
            switch (newNumCat) {
            case INT:
              *newExpr = convertExperssion(val, newType, "tointeger", castedType, flc);
              break;
            case SMALLINT:
              *newExpr = convertExperssion(val, newType, "tosmallint", castedType, flc);
              break;
            default:
              break;
            }
            break;
            default:
              throw 999;
              break;
        }
        break;
        default:
          throw 999;
      }
      // TODO: реализовать проверку на максимальное значение
      // by value:
      //   len >  32767  - to smallint
      //   len >
      break;
    }
    default:
      throw 999;
      break;
    };

    break;
  }

  destroy_smart_lexer(0, curBuf, scanner);
  pLexInputData = oldLexBufFun;
}

void CommonDatatypeCast::castToNull(SqlExpr **newExpr, Sm::PlExpr *castedExpr)
{
  *newExpr  = new Sm::NullExpr(castedExpr->getLLoc());
  if (SemanticTree *n = castedExpr->getSemanticNode()) {
    (*newExpr)->setSemanticNode(n);
    if (n->unnamedDdlEntity == castedExpr->toSelfSqlExpr()) {
      n->unnamedDdlEntity = *newExpr;
      n->cathegory = SCathegory::EMPTY;
    }
  }
}


class CastingLocal {
public:
  Sm::PlExpr   *castedExpr;
  Sm::SqlExpr **newExpr;

  Sm::CommonDatatypeCast::TypeCathegory   castedCat;
  Sm::CommonDatatypeCast::TypeCathegory   newCat;
  Sm::CommonDatatypeCast::NumberCathegory castedNumCat;
  Sm::CommonDatatypeCast::NumberCathegory newNumCat;
  const FLoc   &flc;
  Datatype     *castedType;
  Datatype     *newType;
  CastCathegory castCathegory;

  Sm::CommonDatatypeCast::CastContext &castContext;

  CastingLocal(
    Sm::PlExpr                             *_castedExpr,
    Sm::SqlExpr                           **_newExpr,
    Sm::CommonDatatypeCast::TypeCathegory   _castedCat,
    Sm::CommonDatatypeCast::TypeCathegory   _newCat,
    Sm::CommonDatatypeCast::NumberCathegory _castedNumCat,
    Sm::CommonDatatypeCast::NumberCathegory _newNumCat,
    const FLoc                             &_flc,
    Datatype                               *_castedType,
    Datatype                               *_newType,
    CastCathegory                           _castCathegory,
    Sm::CommonDatatypeCast::CastContext    &_castContext
  ) : castedExpr   (_castedExpr   ),
      newExpr      (_newExpr      ),
      castedCat    (_castedCat    ),
      newCat       (_newCat       ),
      castedNumCat (_castedNumCat ),
      newNumCat    (_newNumCat    ),
      flc          (_flc          ),
      castedType   (_castedType   ),
      newType      (_newType      ),
      castCathegory(_castCathegory),
      castContext  (_castContext  ) {}
};

PlExpr* Sm::CommonDatatypeCast::cast(Sm::PlExpr *castedExpr, Datatype *castedType1, Datatype *newType1, CastCathegory castCathegory) {
  // для castedExpr нужно получить контекст и если это поле запроса
  //    если это поле запроса into -
  //      сначала привести тип поля запроса к типу переменной в into.
  //    иначе
  //      привести тип поля запроса к новому типу явно.
  // into - получить для него тип into переменной,
  // после чего сделать явное привдение или соответствующего поля into курсора
  Sm::SemanticTree *exprNode = castedExpr->getSemanticNode();
  if (castedExpr)
    if (ResolvedEntity *n = castedExpr->getNextDefinition()) {
      FunctionDynField *dynField;
      if (n->isFieldForMakestr() && !((dynField = n->toSelfFunctionDynField()) && dynField->sourceForCastDatatype))
        return castedExpr;
    }

  CastContext castContext;
  exprNode->getExpressionContext(castContext);
  Ptr<Datatype> newType = ResolvedEntity::getLastConcreteDatatype(newType1);
  Ptr<Datatype> castedType = ResolvedEntity::getLastConcreteDatatype(castedType1);
  if (newType.object() == castedType.object())
    return castedExpr;

  if (!castedType)
    return castedExpr;

  if (castedExpr->beginedFrom(67323))
    cout << "";

  // для всех plExpr - нужно сохранять семантический узел (хотя бы приблизительный) - чтобы можно было получить контекст
  // Datatype *assignedVariableDatatype;
  // contextType = exprNode->getContextCathegoryContext(&intoVariableDatatype)
  // определить тип cast

  SqlExpr* newExpr = 0;

  TypeCathegory castedCat = UNINITIALIZED_CATHEGORY;
  TypeCathegory newCat    = UNINITIALIZED_CATHEGORY;
  NumberCathegory castedNumCat, newNumCat;

  getTypeCat(castedExpr, castedType, castedCat, castedNumCat);
  getTypeCat(0         , newType   , newCat   , newNumCat   );

  const FLoc flc = castedExpr->getLLoc();
  CastCathegory::CastingState castState = castCathegory.castState();

  static const function<void(CastingLocal&)> noConv = [](CastingLocal &c) {
    cout << "error: type conversion not implemented: " << CommonDatatypeCast::castedCathegoryToString(c.castedCat)
         <<  " -> " << CommonDatatypeCast::castedCathegoryToString(c.newCat) << endl;
    return;
  };
  static const function<void(CastingLocal&)> nop    = [](CastingLocal&) {};
  static const auto numval2Ch = [](CastingLocal &c) { c.castedExpr->setStringType(); };
  static const auto qid2Num = [](CastingLocal &c) { castQuotedIdToNumber(c.newExpr, c.castedExpr, c.newType, c.castedType, c.castCathegory, c.newNumCat, c.flc); };

  switch (castState) {
    case CastCathegory::EMPTY_STATE:
      throw 999; // контекст приведения нужно задать заранее.
      break;
    case CastCathegory::SQL: {
      if (!exprNode->isSqlCode())
        throw 999; // контекст должен быть контекстом запроса.

      static const auto chToDate   = [](CastingLocal &c) {
        if (c.castedExpr->isQuotedSqlExprId())
          throw 999;
        SemanticTree *n = c.castedExpr->getSemanticNode();
        *(c.newExpr) =
            new Sm::RefExpr(c.flc, new Id(c.flc, "to_date", {c.castedExpr, SqlExpr::literal(modelFuns()->defaultDateFormat) }, modelFuns()->linterToDate), n);
      };
      static const auto toNumber   = [](CastingLocal &c) { *(c.newExpr) = new Sm::Cast(c.flc, convertExperssion(c.castedExpr, c.newType, "TO_NUMBER" , c.castedType, c.flc), c.newType); };
      static const auto toNewtype  = [](CastingLocal &c) { *(c.newExpr) = new Sm::Cast(c.flc, c.castedExpr, c.newType); };
      static const auto charToChar = [](CastingLocal &c) {
        if (c.castContext.dynamicEntity)
          return;
        if (c.castCathegory.lengthLhsGtRhs()) {
          // expr - это вызов функции в sql коде и его нужно привести к реальному укороченному типу
          if (c.castCathegory.isCastFunctionCallsToReducedRettype())
            toNewtype(c);
          else
            throw 999; // TODO: неизвестно что кастится как lhs, а что - как rhs, нужен признак "кастить по длине", либо понимание, когда это делать.
        }
        else if (c.castCathegory.lengthLhsLtRhs() && c.castCathegory.castRhsToLhs())  // rhs = castedExpr, lhs - newType
          toNewtype(c);
        else if (c.newType->isLongDatatype() || c.castedType->isLongDatatype())
          return;
        else if (c.castCathegory.castRhsToLhs() || c.castCathegory.implicitFlag())
          return;
        else
          throw 999;
      };
      static const auto toChar     = [](CastingLocal &c) { *c.newExpr = new Sm::Cast(c.flc, convertExperssion(c.castedExpr, c.newType, "TO_CHAR" , c.castedType, c.flc), c.newType); };
      static const auto numToNum   = [](CastingLocal &c) {
        if (c.newNumCat == c.castedNumCat) {
          if (c.newNumCat == Sm::CommonDatatypeCast::NUMBER &&
              c.castedType->scalePrecIsNotSet() && !c.newType->scalePrecIsNotSet() &&
              c.castCathegory.isCastFunctionCallsToReducedRettype())
            return;
          else if (c.castCathegory.castRhsToLhs())
            return;
          else
            throw 999;
        }
        else
          toNewtype(c);
      };
      static const auto toNull     = [](CastingLocal &c) {  castToNull(c.newExpr, c.castedExpr); };
      (void)toNull;
      static const auto clobToChar = [](CastingLocal &c) {
        SemanticTree *n = c.castedExpr->getSemanticNode();
        Ptr<RefExpr> lenblob = new Sm::RefExpr(c.flc, new Id(c.flc, "lenblob", {c.castedExpr, new Int(1)}, syntaxerContext.model->globalFunctions->linterLenblob), n);
        Ptr<pl_expr::Comparsion> cmp2000 = new pl_expr::Comparsion(c.flc, lenblob.object(), ComparsionOp::GE, new Int(2000), n);
        Ptr<pl_expr::Comparsion> cmp4000 = new pl_expr::Comparsion(c.flc, lenblob.object(), ComparsionOp::GE, new Int(4000), n);
        *(c.newExpr) = new Case(c.flc,
                           cmp2000.object(),
                           new AlgebraicCompound(c.flc,
                             new RefExpr(c.flc, new Id(c.flc, "gettext", {c.castedExpr, new Int(1), new Int(2000)}, syntaxerContext.model->globalFunctions->linterGettext), n),
                             algebraic_compound::CONCAT,
                             new RefExpr(c.flc, new Id(c.flc, "gettext",
                                                                     {c.castedExpr, new Int(2001),
                                                                      new Case
                                                                      (
                                                                        c.flc,
                                                                        cmp4000.object(),
                                                                        new Int(2000),
                                                                        new AlgebraicCompound(c.flc, lenblob.object(), algebraic_compound::MINUS, new Int(2000))
                                                                      )
                                                                     }, syntaxerContext.model->globalFunctions->linterGettext), n), n),
                           new RefExpr(c.flc, new Id(c.flc, "gettext", {c.castedExpr, new Int(1), lenblob.object()}, syntaxerContext.model->globalFunctions->linterGettext), n));

      };
      static const auto numvalToNum = [](CastingLocal &c) {
        NumericValue *v = c.castedExpr->toSelfNumericValue();
        if (!v)
          if (Ptr<PlExpr> unwrExpr = c.castedExpr->unwrapBrackets())
            if ((v = unwrExpr->toSelfNumericValue()) == NULL)
              if ((v = unwrExpr->getNextDefinition()->toSelfNumericValue()) == NULL)
                throw 999;
        switch (v->cathegory()) {
          case NumericValue::UNKNOWN:
            throw 999;
            break;
          case NumericValue::INTVAL:
          case NumericValue::SIMPLE_INT:
            switch (c.newNumCat) {
              case NUMBER:
              case SMALLINT:
              case BIGINT:
              case DOUBLE:
                toNewtype(c);
                break;
              default:
                throw 999;
                break;
            }
            break;
          case NumericValue::FLOATVAL:
            throw 999;
            break;
        }
      };

      // TODO: после select blob into clob - нужно вставить read_clob. * CLOB в таблицах Oracle - это BLOB в таблицах Линтер


      static const function<void(CastingLocal&)> convertFun[UNINITIALIZED_CATHEGORY+1][UNINITIALIZED_CATHEGORY+1] = {
      // casted v  new ->        CHAR_VARCHAR   NCHAR   NVARCHAR LIKE_NUMBER    DATE        CLOB   NUMERIC_VALUE NUMERIC_VALUE_0  QUOTED_SQL_EXPR_ID EMPTY_ID   BOOL_  NULLCAT  ERROR
      /*CHAR_VARCHAR       */ {     charToChar, noConv,  noConv,  toNumber,    chToDate,   noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*NCHAR              */ {     noConv,     noConv,  noConv,  noConv,      noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*NVARCHAR           */ {     noConv,     noConv,  noConv,  noConv,      noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*LIKE_NUMBER        */ {     toChar,     noConv,  noConv,  numToNum,    noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*DATE               */ {     toChar,     noConv,  noConv,  noConv,      nop   ,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*CLOB               */ {     clobToChar, noConv,  noConv,  noConv,      noConv,     nop   ,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*NUMERIC_VALUE      */ {     numval2Ch,  noConv,  noConv,  numvalToNum, noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*NUMERIC_VALUE_0    */ {     numval2Ch,  noConv,  noConv,  numvalToNum, noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*QUOTED_SQL_EXPR_ID */ {     nop,        nop,     nop,     qid2Num,     nop,        noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*EMPTY_ID           */ {     nop,        nop   ,  nop,     toNull,      noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*BOOL_              */ {     noConv,     noConv,  noConv,  noConv,      noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   nop,     nop,    noConv },
      /*NULLCAT            */ {     toNewtype, toNewtype, toNewtype, toNewtype,toNewtype,  toNewtype,toNewtype,       toNewtype,      toNewtype,      toNewtype,nop,     nop,    noConv },
      /*ERROR              */ {     noConv,     noConv,  noConv,  noConv,      noConv,     noConv,   noConv,          noConv,         noConv,         noConv,   noConv,  noConv, noConv },
      };

      CastingLocal cntx(castedExpr, &newExpr, castedCat, newCat, castedNumCat, newNumCat, flc, castedType, newType, castCathegory, castContext);
      convertFun[castedCat][newCat](cntx);
      break;
    }
    case CastCathegory::PROC: {
      // 3 - A > B, A op B -> A op cast B to A; тип более общий, нужно явное преобразование для Линтер
      if (exprNode->isSqlCode() && !exprNode->isNotPlContext())
        throw 999; // контекст должен быть контекстом процедурного языка, т.е. sql
      static const auto convertToNumber = [](CastingLocal &c) -> bool {
        if (c.newCat == c.castedCat)
          if (c.castCathegory.castState() == CastCathegory::PROC && !c.castCathegory.castReturn())
            return true;

        static const string trNames[7] = {
          /*SMALLINT*/  "tosmallint",
          /*INT     */  "tointeger",
          /*BIGINT  */  "tobigint",
          /*REAL_   */  "toreal",
          /*DOUBLE  */  "toreal",
          /*NUMBER  */  "tonumeric",
          /*EMPTY   */  ""
        };
        const string &str = trNames[c.newNumCat];
        if (str.empty())
          throw 999;
        *(c.newExpr) = convertExperssion(c.castedExpr, c.newType, str, c.castedType, c.flc);
        return true;
      };

      static const auto qid2Date     = [](CastingLocal &c) { c.castedExpr->setDateLiteral(); };
      static const auto toNull       = [](CastingLocal &c) { castToNull(c.newExpr, c.castedExpr); };
      static const auto toChar       = [](CastingLocal &c) { *(c.newExpr) = convertExperssion(c.castedExpr, c.newType, "tochar" , c.castedType, c.flc); };
      static const auto toClob       = [](CastingLocal &c) { *(c.newExpr) = convertExperssion(c.castedExpr, c.newType, "toclob" , c.castedType, c.flc); };
      static const auto toDate       = [](CastingLocal &c) { *(c.newExpr) = convertExperssion(c.castedExpr, c.newType, "to_date", c.castedType, c.flc); };
      static const auto toNum        = [](CastingLocal &c) {
        if (!convertToNumber(c))
          throw 999;
      };
      static const auto ch2Ch        = [](CastingLocal &c) {
        if (c.castedType->isRowidDatatype() && !c.newType->isRowidDatatype())
          return toChar(c);
        if (c.castCathegory.castAssignment())
          return;
        if (!c.castCathegory.isCastFunctionCallsToReducedRettype()) {
          cout << "error: case for not isCastFunctionCallsToReducedRettype is not implementing yet in ch2Ch" << endl;
          // необходимо реализовать проверку необходимости приведения для этого случая
        }
        Ptr<Id> f = new Id("substr", syntaxerContext.model->globalFunctions->getSubstr(true, c.newType->precision));
        f->loc(c.flc);
        f->callArglist = new CallArgList({new FunCallArgExpr(c.flc, c.castedExpr),
                                          new FunCallArgExpr(c.flc, new NumericSimpleInt(1)),
                                          new FunCallArgExpr(c.flc, new NumericSimpleInt(c.newType->precision))});
        *(c.newExpr) = new Sm::RefExpr(c.flc, f);
      };

      static const function<void(CastingLocal&)> convertFun[UNINITIALIZED_CATHEGORY+1][UNINITIALIZED_CATHEGORY+1] = {
      // casted v  new ->        CHAR_VARCHAR  NCHAR NVARCHAR LIKE_NUMBER DATE      CLOB  NUMERIC_VALUE NUMERIC_VALUE_0  QUOTED_SQL_EXPR_ID EMPTY_ID    BOOL_  NULLCAT  ERROR
      /*CHAR_VARCHAR       */ {    ch2Ch ,    noConv, noConv, toNum ,    toDate,   toClob,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*NCHAR              */ {    toChar,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*NVARCHAR           */ {    toChar,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*LIKE_NUMBER        */ {    toChar,    noConv, noConv, toNum ,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*DATE               */ {    toChar,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*CLOB               */ {    toChar,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*NUMERIC_VALUE      */ {    numval2Ch, noConv, noConv, nop   ,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*NUMERIC_VALUE_0    */ {    numval2Ch, noConv, noConv, nop   ,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*QUOTED_SQL_EXPR_ID */ {    nop,       noConv, noConv, qid2Num,   qid2Date, toClob,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*EMPTY_ID           */ {    toNull,    noConv, noConv, toNull  ,  toNull  , toNull,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*BOOL_              */ {    noConv,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv,  nop,   nop,    noConv },
      /*NULLCAT            */ {    nop,       nop,    nop,    nop,       nop,      nop,     nop,           nop,             nop,               nop,     nop,   nop,    noConv },
      /*ERROR              */ {    noConv,    noConv, noConv, noConv,    noConv,   noConv,  noConv,        noConv,          noConv,            noConv, noConv,noConv,  noConv },
      };

      CastingLocal cntx(castedExpr, &newExpr, castedCat, newCat, castedNumCat, newNumCat, flc, castedType, newType, castCathegory, castContext);
      convertFun[castedCat][newCat](cntx);
      break;
    }
    default:
      throw 999;
      // cout << "ERROR: unsupported datatype casting for value "  << castCathegory << endl;
      break;
  }

  if (!newExpr)
    return castedExpr;
  if (newExpr != castedExpr) {
    SemanticTree *parent = exprNode->getParent();
    sAssert(!parent);
//    exprNode->setInvalid();

    SemanticTree::FlagsType f = parent->flags;
    if (SqlExpr *q = castedExpr->toSelfSqlExpr()) {
      if (exprNode->unnamedDdlEntity == q) {
        parent = exprNode->getParent();
        exprNode->unnamedDdlEntity = 0;
//        f = exprNode->__flags__.v;
      }
      if (!parent)
        throw 999;
    }
    if (Sm::Brackets *brcs = castedExpr->toSelfBrackets()) {
      if (exprNode->unnamedDdlEntity == brcs->brackets.object()) {
        parent = exprNode->getParent();
        exprNode->unnamedDdlEntity = 0;
      }
      if (!parent)
        throw 999;
    }
    ResolvedEntitySNode::pushTransformStage();
    newExpr->collectSNode(parent);
    parent->setRecursiveFlags(f);
    ResolvedEntitySNode::popStage();
  }

//  newExpr->setSemanticNode(castedExpr->getSemanticNode());
  return newExpr;
}


void CommonDatatypeCast::castAndReplace(bool isPl, Ptr<PlExpr> &castedExpr, Datatype *castedType, Datatype *newType, CastCathegory castCathegory) {
  Ptr<PlExpr> use = castedExpr;
  (void)use;
  if (isPl)
    castCathegory.setProcCastState();
  else
    castCathegory.setSqlCastState();
  castedExpr = CommonDatatypeCast::cast(castedExpr.object(), castedType, newType, castCathegory);
}


void CommonDatatypeCast::castAndReplace(bool isPl, Ptr<SqlExpr> &castedExpr, Datatype *castedType, Datatype *newType, CastCathegory castCathegory) {
  Ptr<SqlExpr> use = castedExpr;
  (void)use;
  if (isPl)
    castCathegory.setProcCastState();
  else
    castCathegory.setSqlCastState();
  Ptr<PlExpr> t = CommonDatatypeCast::cast(castedExpr.object(), castedType, newType, castCathegory);
  if (SqlExpr* s = t->toSelfSqlExpr())
    castedExpr = s;
  else
    throw 999;
}


void SemanticTree::getExpressionContext(CommonDatatypeCast::CastContext &context) {
  if (ResolvedEntity *d = ddlEntity()) {
    if (isFromNode())
      context.fromNode = ddlEntity();
    if (isInsertingValue())
      context.insertingValue = d->toSelfSqlExpr();
    if (isIntoNode())
      context.intoVariableExpression = d->toSelfRefAbstract();
  }
  if (nametype == REFERENCE) {
    if (ResolvedEntity *d = unnamedDdlEntity)
      if (RefExpr* s = d->toSelfRefExpr())
        if (ResolvedEntity *unref = s->refDefinition())
          if (unref->toSelfFunction()) {
            context.f.functionCall = true;
            context.functionCalls.push_back(s);
          }
  }
  if (nametype != REFERENCE && nametype != DATATYPE_REFERENCE)
    if (ResolvedEntity *d = unnamedDdlEntity)
      if (Sm::Function *f = d->toSelfFunction()) {
        context.f.functionDefinition = true;
        context.functionDefinitions.push_back(f);
      }
    switch (cathegory) {
      case SCathegory::FunctionDynExpr:
      case SCathegory::FunctionDynField:
      case SCathegory::FunctionDynTail_:
      case SCathegory::QueryEntityDyn:
        context.dynamicEntity = unnamedDdlEntity;
        break;
      case SCathegory::StatementOpenFor:
        context.openForStmt = unnamedDdlEntity->toSelfOpenFor();
        break;
      case SCathegory::Update:
        context.updateStmt = unnamedDdlEntity->toSelfUpdate();
        break;
      case SCathegory::StatementDeleteFrom:
        context.deleteStmt = unnamedDdlEntity->toSelfDeleteFrom();
        break;
      case SCathegory::QueryBlock:       // pass
      case SCathegory::UnionQuery:            // pass
      case SCathegory::SelectSingle:     // pass
      case SCathegory::SelectBrackets: {
        context.f.sqlQuery = true;
        Subquery::IntoList into;
        if (unnamedDdlEntity) {
          if ((context.subquery = unnamedDdlEntity->toSelfSubquery()))
            into = context.subquery->intoList();
          else if ((context.queryBlock = unnamedDdlEntity->toSelfQueryBlock()))
            into = context.queryBlock->intoList;
        }
        if (into) {

          context.f.sqlQueryIntoVariable = true;
        }
        break;
      }
      case SCathegory::Return: {
        context.f.returnStatement = true;
        context.retStmt = unnamedDdlEntity->toSelfReturn();
        break;
      }
      case SCathegory::View:
        context.f.view = true;
        context.view = unnamedDdlEntity->toSelfView();
        break;
      case SCathegory::Assignment:
        context.f.assignment = true;
        context.assignment = unnamedDdlEntity->toSelfAssignment();
        break;
      case SCathegory::AlgebraicCompound:
        context.f.compoundExpression = true;
        context.compound = unnamedDdlEntity->toSelfAlgebraicCompound();
        if (context.compound->isConcatenation()) {
          context.f.concatExpression = true;
          context.concat = context.compound;
        }
        break;
      default:
        break;
    }
  if (this->parent)
    this->parent->getExpressionContext(context);
}



LimitExprNode::LimitExprNode(int _isNot, int queryDepth, int _limitExprDeep, Ptr<pl_expr::Comparsion> &_cmp)
  :isNot(_isNot), currentQueryDepth(queryDepth), limitExprDeep(_limitExprDeep), cmp(_cmp) {}

LimitExprNode::LimitExprNode() {}
