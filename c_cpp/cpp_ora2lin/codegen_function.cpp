#include <sstream>
#include "semantic_function.h"
#include "semantic_object.h"
#include "model_context.h"
#include "semantic_expr.h"
#include "semantic_blockplsql.h"
#include "semantic_statements.h"
#include "semantic_plsql.h"
#include "codegenerator.h"

using namespace Sm;

extern SyntaxerContext syntaxerContext;

void Sm::Function::extractExternalVariables() {
  if (flags.isExternalVariablesExtracted())
    return;
  flags.setExternalVariablesExtracted();
  externalVariables.clear();
  if (body_) {
    UniqueEntitiesMap uniqueMap;
    body_->extractBlockExternalVariables(externalVariables, uniqueMap);
  }
  if (arglist && externalVariables.size())
    for (Arglist::iterator it = arglist->begin(); it != arglist->end(); ++it) {
      for (ExternalVariables::iterator exIt = externalVariables.begin(); exIt != externalVariables.end(); )
        if (it->object() == (*exIt)->entity)
          exIt = externalVariables.erase(exIt);
        else
          ++exIt;
      }
}

void Sm::Function::translateExternalVariablesToCallarglist(Sm::Codestream &str, Ptr<CallArgList> /*callarglist*/) {
  bool isNotFirst = false;
  if (!flags.isExternalVariablesExtracted())
    extractExternalVariables();
  for (Sm::ExternalVariables::iterator it = externalVariables.begin(); it != externalVariables.end(); ++it)
    str << s::comma(&isNotFirst) << s::linref((*it)->entity);
}

void Sm::Function::oracleDeclaration(Sm::Codestream &str) {
  str << (rettype ? "FUNCTION  " : "PROCEDURE ") << name << s::obracket << s::def << arglist << s::cbracket;
  if (rettype)
    str << " RETURN " << s::decl << rettype;
}

std::string Sm::Type::MemberFunction::prefix() {
  if (isEntry(member_function::MAP, specificators))
    return "MAP";
  else if (isEntry(member_function::ORDER, specificators))
    return "ORDER";
  if (isEntry(member_function::MEMBER, specificators))
    return "MEMBER";
  if (isEntry(member_function::STATIC, specificators))
    return "STATIC";

  if (isEntry(inheritance, Inheritance::FINAL))
    return "FINAL";
  if (isEntry(inheritance, Inheritance::INSTANTIABLE))
    return "INSTANTIABLE";

  if (isEntry(member_function::CONSTRUCTOR, specificators))
    return "CONSTRUCTOR";
  return "";
}

void Sm::Type::MemberFunction::oracleDeclaration(Sm::Codestream &str) {
  str << prefix() << ' ' << (rettype ? "FUNCTION  " : "PROCEDURE ") << name << s::obracket;
  Ptr<Id> n;
  if (arglist && arglist->size() && (n = arglist->front()->getName()) && n->toNormalizedString() == "SELF")
    //oracleDefinitionListByIt(str, ++(arglist->begin()), arglist->end());
    str << s::def << skip(1) << arglist;
  else
    //oracleDefinitionList(str, arglist);
    str << s::def << arglist;

  str << s::cbracket;
  if (rettype)
    str << " RETURN " << s::decl << rettype;
}

void Sm::Type::MemberFunction::addReturnSelf() {
  Sm::Return *retStatement = NULL;

  // Ищем return
  for (Statements::iterator it = body_->statements.begin(); it != body_->statements.end(); ++it)
    if ((retStatement = (*it)->toSelfReturn()))
      break;

  if (!retStatement) {
    retStatement = new Sm::Return(getLLoc());
    body_->statements.push_back(retStatement);
  }
  if (!retStatement->expr) {
    retStatement->expr = new Sm::RefExpr(new Id("self"));
  }
}

void Sm::Type::MemberFunction::addNestedCtors() {
  if (!owner_ || !body_)
    throw 999;

  bool isDefault = name->toNormalizedString() == "DEFAULT_CTOR__";
  Object *ownerObj = getOwner();
  if (ownerObj->getDeclaration())
    ownerObj = ownerObj->getDeclaration();

  BaseList<MemberInterface> *members = ownerObj->getMembers();
  for (BaseList<MemberInterface>::iterator it = members->begin(); it != members->end(); ++it) {
    ResolvedEntity *memberVar;
    if ((*it)->cathegoryMember() != MemberInterface::VARIABLE)
      continue;

    memberVar = (*it)->getThisDefinition();
    Ptr<Datatype> datatype = memberVar->getDatatype()->getFinalType();
    if (datatype->isObjectType()) {
      Ptr<Function> ctor = datatype->getDefaultConstructor();
      Ptr<PlExpr> expr = new RefExpr(getLLoc(), ctor->getName());
      body_->addToInitializators(memberVar, expr);
    }
    else if (isDefault) {
      string varName = memberVar->getName()->toNormalizedString();
      auto isSpec = [](const char c) -> bool { return  c == '#' || c == '$'; };
      std::replace_if(varName.begin(), varName.end(), isSpec, '_');
      Ptr<PlExpr> expr = new RefExpr(getLLoc(), new Id(varName + "_"));
      Ptr<LValue> lVal = new LValue(expr->getLLoc(), new IdEntitySmart(memberVar->getName()));
      Ptr<Assignment> assign = new Assignment(getLLoc(), lVal, expr);

      Ptr<PlExpr> condition = new pl_expr::IsNull(cl::emptyFLocation(), expr->toSelfSqlExpr());
      Ptr<WhenExpr> firstIfStmts = new WhenExpr(new Sm::Statements(), condition, Sm::WhenExpr::IF_FIRST_STATEMENT);
      condition->setNot();
      firstIfStmts->branchStatements.push_back(assign.object());
      Ptr<If> ifStatement = new If(cl::emptyFLocation(), firstIfStmts, NULL, NULL);

      body_->blockInitializators()->push_back(ifStatement.object());
    }
  }
}

void Sm::Type::MemberFunction::makeFormalTranslations() {
}

void Sm::Type::MemberFunction::sqlDefinition(Codestream &str) {
  if (isEntry(member_function::CONSTRUCTOR, specificators)) {
    if (!body_) {
      body_ = new BlockPlSql(getLLoc(), NULL, new Statements());
      body_->setOwnerBlockPlSql(ownerPlBlock());
    }
    addReturnSelf();
    addNestedCtors();
  }

  Function::sqlDefinition(str);
}

void Sm::Type::MemberFunction::linterDefinition(Codestream &str) {
  Function::linterDefinition(str);
}


void Sm::Type::MemberFunction::translateSelfRef(Sm::Codestream &str) {
  if (!(isEntry(specificators, member_function::CONSTRUCTOR) || isEntry(specificators, member_function::MEMBER)))
      return;

  if (str.namesMode() == Sm::CodestreamState::DEFINITION) {
    str << "IN self" << s::name;
    owner()->linterDefinition(str);
    if (arglist && arglist->size())
      str << s::semicolon << s::name;
  }
  else {
    str << "self";
    if (arglist && arglist->size())
      str << s::comma();
  }
}

bool Sm::Type::MemberFunction::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if ((!callArgList || callArgList->size() == 0 || !(*callArgList->begin())->isSelf())
      && (!owner_ || !owner_->isAnydata())) {
    // Если не указан self в вызове метода
    str << s::ref;
    translateSelfRef(str);
    str << s::def;
  }
  str << callArgList;
  return true;
}

UserContext * Sm::Type::MemberFunction::userContext() const {
  if (owner_)
    return owner_->userContext();
  return NULL;
}

Ptr<LinterCursor> Sm::Function::generateLinterCursor(Ptr<SqlExpr> query, Ptr<BlockPlSql> b, std::string varName) {
  Ptr<LinterCursor> linterCursor;
  std::string uniqueName = b->getSemanticNode()->levelNamespace->getUniqueName(varName);
  // Сгенерировать тип данных.
  pair<LevelResolvedNamespace::iterator, bool> it =
      b->getSemanticNode()->levelNamespace->insert(LevelResolvedNamespace::value_type(uniqueName, LevelResolvedNamespace::value_type::second_type(0)));

  linterCursor = new Sm::LinterCursor(std::forward<string>(uniqueName), query);
  b->add(linterCursor.object());
  Ptr<VEntities> ent = new VEntities();     ent->add(linterCursor.object());
  it.first->second = LevelResolvedNamespace::value_type::second_type(ent);
  return linterCursor;
}


Ptr<LinterCursor> Sm::Function::generateLinterCursorVar(
    Ptr<BlockPlSql>                             b,
    std::vector<StatementInterface*>::iterator &pos,
    std::string                                 varName)
{
  Ptr<LinterCursor> linterCursor = generateLinterCursor((*pos)->selectExpr(), b, varName);
  b->addReturn(linterCursor->getName());
  return linterCursor;
}


Ptr<LinterCursor> Sm::Function::tranlateAllOpenRefCursorLayer(
    Sm::OutRefCursors::value_type::first_type refCursorVar,
    Ptr<BlockPlSql> b,
    std::vector<StatementInterface*> &layer)
{
  Ptr<LinterCursor> linterCursor;

  for (std::vector<StatementInterface*>::iterator pos = layer.begin(); pos != layer.end(); ++pos) {
    if (!(*pos)->isOpenStatement())
      throw 999; // так быть не должно
    if (!linterCursor)
      // Из списка определений получить уникальный идентификатор.
      linterCursor = generateLinterCursorVar(b, pos);
    if (SemanticTree *node = (*pos)->getSemanticNode())
      node->changeReferences2(refCursorVar.object(), linterCursor.object());
    (*pos)->changeCursorVariable(linterCursor);
  }
  return linterCursor;
}


Ptr<LinterCursor> Sm::Function::tranlateRefCursorLayer(
    Sm::OutRefCursors::value_type::first_type   refCursorVar,
    Ptr<BlockPlSql>                             b,
    std::vector<StatementInterface*>           &layer,
    std::vector<StatementInterface*>::iterator &pos)
{
  Ptr<LinterCursor> linterCursor;

  if (pos == layer.end())
    return 0;

  for (; pos != layer.end(); ++pos)
    if ((*pos)->isOpenStatement()) {
      linterCursor = generateLinterCursorVar(b, pos);
      break;
    }
    else if ((*pos)->isCloseStatement()) {
      ++pos;
      return 0;
    }
  if (pos == layer.end())
    return 0;

  for (; pos != layer.end(); ++pos) {
    // В операторах, использующих данный RefCursor, до следующего close включительно - подменить ссылки на RefCursor на абстракцию-контейнер
    // Нужно (1) подменить все определения внутри ссылок
    //       (2) подменить все элементы внутри операторов, обрабатывающих этот курсор
    if (SemanticTree *node = (*pos)->getSemanticNode())
      node->changeReferences2(refCursorVar.object(), linterCursor.object());
    (*pos)->changeCursorVariable(linterCursor);
    if ((*pos)->isCloseStatement()) { ++pos; return linterCursor; }
  }

  return linterCursor;
}


void Sm::Function::translateOutCursorInArglist() {
  if (beginedFrom(63174,13))
    cout << "";
  if (!arglist || outRefCursors.size())
    return;
  ResolvedEntity *thisDef = this;
  if (!body_) {
    thisDef = getDefinitionFirst();
    arglist = thisDef->funArglist();
  }
  Ptr<BlockPlSql> b = thisDef->funBody();
  if (!b)
    return;
  int pos = 0;
  for (FunArgList::iterator it = arglist->begin(); it != arglist->end(); ++pos) {
    if ((*it)->isRefCursor() &&
        ((*it)->dir() == function_argument::IN_OUT ||
         (*it)->dir() == function_argument::OUT)) {
      outRefCursors.push_back(OutRefCursors::value_type(*it, OutRefCursors::value_type::second_type()));
      it = arglist->erase(it);
    }
    else
      ++it;
  }

  if (outRefCursors.empty())
    return;
  for (OutRefCursors::iterator cit = outRefCursors.begin(); cit != outRefCursors.end(); ++cit) {
    std::vector<StatementInterface*> usedList;
    // Получить список использующих операторов для данного курсора.
    using namespace std::placeholders;
    b->traverseStatements(std::bind(mem_fn(&StatementInterface::getStatementsThatContainEntity), _1, cit->first.object(), ref(usedList)));
    bool allIsOpen = true;
    for (std::vector<StatementInterface*>::iterator it = usedList.begin(); it != usedList.end(); ++it)
      if (!(*it)->isOpenStatement()) {
        allIsOpen = false;
        break;
      }
    if (allIsOpen)
      rettype = tranlateAllOpenRefCursorLayer(cit->first, b, usedList)->getDatatype();
    else
      for (std::vector<StatementInterface*>::iterator pos = usedList.begin(); pos != usedList.end(); ) {
        Ptr<LinterCursor> lincursor = tranlateRefCursorLayer(cit->first, b, usedList, pos);
        rettype = lincursor->getDatatype();
      }
  }
  if (VEntities *vEnt = vEntities()) {
    int overId = overloadedId();
    VEntities::Container &funcs = vEnt->getFunctions();
    for (ResolvedEntity* it : funcs)
      if (it->overloadedId() == overId && it != this) {
        it->setRetType(rettype);
        if (Sm::Function *othThisFunDef = it->toSelfFunction())
          othThisFunDef->outRefCursors.insert(
                othThisFunDef->outRefCursors.begin(),
                outRefCursors.begin(),
                outRefCursors.end());
      }

    if (SemanticTree *node = syntaxerContext.model->getSemanticNode())
      node->translateUsageFunWithOutCursor(vEnt);
  }

  // TODO: теперь все ссылки на эту функцию нужно заменить операцией присваивания
  // Начиная с корня: обходом семантического дерева найти все ссылки (не объявления) на эту функцию
  // Функция изначально была с типом void; Значит она может встречаться лишь в ограниченном подмножестве
  // хранимых процедур. При нахождении ссылки на эту функцию сгенерировать исключение, и в отладчике под конкретную структуру написать конкретный код
}


void Sm::Function::linterDeclaration(Sm::Codestream &str) {
  str << s::ocreate(this) << "PROCEDURE " << s::cref(this);

  translateArglist(str);
  translateRettype(str);

  if (rettype)
    str /*<< s::subconstruct(1)*/ << " RESULT " << rettype.object();
  str << " for debug" << ' ' << s::loc(getLLoc()) << s::ccreate;
}


void Sm::Function::translateArglist(Codestream &str) {
  str << s::obracketArglist; /*<< s::subconstruct(2)*/
  translateSelfRef(str);
  if (externalVariables.size()) {
    bool isNotFirst = false;
    for (Sm::ExternalVariables::iterator it = externalVariables.begin(); it != externalVariables.end(); ++it) {
      if (isNotFirst) {
        str << s::semicolon << s::name;
        str.joinSuffixes();
      }
      else
        isNotFirst = true;
      if ((*it)->entity)
        (*it)->translateAsLinterFunctionArgument(str);
    }
  }

  if (arglist && arglist->size()) {
    if (externalVariables.size()) {
      str << s::semicolon << s::name;
      str.joinSuffixes();
    }
    str << arglist->front();
    FunArgList::iterator it = ++(arglist->begin());
    if (it == arglist->end())
      str.joinSuffixes();
    else
      for (; it != arglist->end(); ++it) {
        str << s::semicolon << s::name;
        str.joinSuffixes();
        str << s::name << *it;
      }
  }
  str << s::cbracketArglist;
}

void Sm::Function::translateRettype(Codestream &str) {
  if (rettype) {
    if (!arglist || arglist->empty())
      str << s::ocommalist();
    else
      str << s::otabcommalist();

    str << s::name /*<< s::subconstruct(1)*/ << "RESULT" << s::name;
    Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(rettype);
    if (t && t->tidDdl()->ddlCathegory() == ResolvedEntity::LinterCursor_) {
      t->tidDdl()->translateAsCursor(str);
      str << s::name << s::loc(getLLoc());
    }
    else if (t->isRefCursor() && fields_.size() > 0 &&
        (!t->tidDdl()->toSelfRefCursor() || !t->tidDdl()->toSelfRefCursor()->datatype)) {
      translateFieldsAsCursor(fields_, str);
    }
    else {
      str.state().dynamicCollection(isDynamicUsing());
      str << t;
      str.state().dynamicCollection(false);
    }

    if (!arglist || arglist->empty())
      str << s::ccommalist();
    else
      str << s::ctabcommalist();
  }
  str << " for debug" << ' ' << s::loc(getLLoc()); // iloc
  str.joinSuffixes();
}

namespace
{

const std::string currentDateTime() {
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y.%m.%d %X", &tstruct);

    return buf;
}

}

void Sm::Function::sqlHeader(Codestream &str) {
  if (externalVariables.empty() && semanticNode && semanticNode->getParent())
    if (semanticNode->getParent()->cathegory == SCathegory::BlockPlSql)
      extractExternalVariables();

  str.procMode(CodestreamState::PROC);
  str.clrBlockEntry();

  translateArglist(str);
  translateRettype(str);

  if (syntaxerContext.model->modelActions.markedAsAutogenerated())
  {
    static std::string strGenTime = currentDateTime();
    str << s::endl;
    str << s::Comment() << "Autogenerated at " << strGenTime;
  }

  str << s::endl;
  str.procMode(CodestreamState::SQL);
}

void Sm::Function::sqlDeclaration(Codestream &str) {
  sqlHeader(str);
  str << "CODE" << s::endl << "END;";
}

void Sm::Function::linterReference(Sm::Codestream &str) {
  if (!isElementaryLinterFunction()) {
    str << s::cref(userContext());
    str << '.';
  }
  translateName(str);
}

void Sm::Function::sqlDefinition(Codestream &str) {
  if (flags.isAlreadyInCodestream())
    return;
  else
    flags.setAlreadyInCodestream();
  int oldIndentingLevel = str.state().indentingLevel_;
  str.state().indentingLevel_ = 0;

  syntaxerContext.proceduresForTranslate++;

  branchId_ = 0;
  str.levelPush();
  str.activatePredeclarations();

  str << s::endl;
  str << s::ocreate(this) << "PROCEDURE " << s::cref(this);
  sqlHeader(str);

  str.activatePrevious();

  str.procMode(CodestreamState::PROC);
  if (!body_) {
    str << "CODE" << s::endl;
    str << "END";
    str.joinDeclarations();
    str.joinPredeclarations();
  }
  else
    str << s::lindef(body_.object());

  str << s::semicolon << s::endl;
  str.procMode(CodestreamState::SQL);
  str << s::ccreate;
  str << s::grant(this, Privs::EXECUTE, syntaxerContext.model->modelActions.scenarioActorUsers);
  str.joinPreactions();
  str.levelPop();
  str.state().indentingLevel_ = oldIndentingLevel;
}

void Sm::Function::translateName(Codestream &str) {
  str << translatedName();
}

std::string Sm::Function::translatedName() const {
  if (this->eid_ == 4912595)
    cout << "";
  std::string str = !isTrNameEmpty() ? ResolvedEntity::translatedName() : name->toString(0);
  if (overloadedId_ > 0 && !isSystemTemplate() && !isElementaryLinterFunction()) {
    char buf[32];
    sprintf(buf, "%i", overloadedId_);
    str.append(buf);
  }
  return str;
}

void Sm::Function::translateLocalObjects(Sm::Codestream &str) {
  if (!body_ || !body_->blockDeclarations())
    return;

  Sm::Declarations *decl = body_->blockDeclarations();
  for (Sm::Declarations::iterator it = decl->begin(); it != decl->end(); ++it) {
    if ((*it)->isVariable()) {
      Ptr<Datatype> datatype = (*it)->getDatatype();
      if (datatype && datatype->isObjectType())
        datatype->translateVariableType(str, (*it)->toSelfVariable());
    }
  }
}

