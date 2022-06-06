#include <sstream>
#include <stack>
#include "semantic_plsql.h"
#include "syntaxer_context.h"
#include "resolvers.h"
#include "siter.h"
#include "i2str.h"
#include "semantic_id.h"
#include "codespacer.h"
#include "codegenerator.h"
#include "semantic_object.h"
#include "depworkflow.h"
#include "lwriter.h"
#include "model_context.h"
#include "semantic_collection.h"
#include "semantic_statements.h"
#include "config_converter.h"
#include "dynamic_sql_op.h"

using namespace std;
using namespace Sm;
using namespace PlsqlHelper;

extern SyntaxerContext syntaxerContext;


static const char *g_self = "self";

namespace Sm {
bool findDynamicNode(SemanticTree *n, int deepDynamic);
}

void Subtype::translateObjRef(Sm::Codestream &str, Sm::Type::RefInfo *ref) {
  if (datatype) {
    if (Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(datatype))
      if (ResolvedEntity *d = t->tidDef())
        d->translateObjRef(str, ref);
  }
}

void Subtype::translateAssign(Sm::Codestream & str, Sm::Type::RefInfo *ref, Sm::PlExpr *expr) {
  if (datatype) {
    if (Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(datatype))
      if (ResolvedEntity *d = t->tidDef())
        d->translateAssign(str, ref, expr);
  }
}

void ResolvedEntity::translateAssign(Sm::Codestream &, Sm::Type::RefInfo *, Sm::PlExpr *) {
  Sm::Codestream str;
  str << "Method translateAssign for " << ddlCathegoryToString() << " with name " << getName() << " is unimplemented" << s::endl;
  cout << str.str();
}

Sm::ResolvedEntitySet Sm::Index::alreadyIndexedFields;

template <typename Container>
void translateStatementsList(Codestream &str, Container &stmts, Sm::BlockPlSql *codeBlock) {
  str << inctab(2);
  for (Sm::Statements::iterator it = stmts.begin(); it != stmts.end(); ++it) {
    if (!(*it)->isIgnoredTranslate())
      Sm::StatementsTranslator::translateStatement(str, it->object(), codeBlock);
  }
  str << dectab(2);
}

Codestream& operator<<(Codestream& s, Sm::Constraint &obj) { return obj.translate(s); }

Codestream& Sm::operator <<(Codestream &os, const Sm::WhenExpr::Then&) {
  os << s::name << "THEN" << s::name;
  return os;
}

template <typename T>
void storeSqlContainerItem(Codestream &str, T item, bool predicate = true, bool needCreateOrReplace = true) {
  if (predicate && needCreateOrReplace)
    str << syntaxerContext.createStatement << item << s::semicolon << s::endl;
  else
    str << item;
  str.join();
}

template <typename T>
void stroreSqlContainer(DependEntitiesMap &filter, bool predicate, Codestream &str, T &container, bool drop = false, bool needCreateOrReplace = true) {
  if (predicate || drop)
    for (typename T::iterator it = container.begin(); it != container.end(); ++it ) {
      if (filter.size() && !filter.count(it->second))
        continue;
      storeSqlContainerItem(str, it->second, predicate, needCreateOrReplace);
    }
}

void setStreamMode(Codestream &str) {
  str.dbMode(Sm::CodestreamState::LINTER);
  str.inlMode();
}

void ModelContext::storeUserTablesKeys(LinterWriter& linterWriter) {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it )
    if (!sysusers.count(it->second.object()))
      it->second->storeSQLTableKeys(linterWriter);
}

void ModelContext::storeSQL(LinterWriter & linterWriter, DependEntitiesMap &filter) {
  Codestream str;
  setStreamMode(str);
  if (modelActions.createUsers()) {
    for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it )
      if (!sysusers.count(it->second.object())) {
        Ptr<Id> uname = it->second->getName();
        if (uname && !uname->empty()) {
          UserContext *uctx = it->second.object();
          str << s::ocmd(it->second.object(), CmdCat::DROP_USER)   << "DROP USER " << s::cref(uctx) << " CASCADE;" << s::endl << s::ccmd;
          uctx->linterDefinition(str);
          uctx->grantResourceDefinition(str);
        }
      }
    str << s::endl;
  }

  if (modelActions.createPublicSynonyms()) {
    stroreSqlContainer(filter, modelActions.createPublicSynonyms(), str, publicSynonyms);
    str << s::endl;
  }

//  printTablesThatWillChanged(str);
  linterWriter.storeSqlMetadata(str);

  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it )
    if (!sysusers.count(it->second.object()))
      it->second->storeSQL(linterWriter, modelActions, filter);


  ModelActions old = modelActions;
  ModelActions clr = modelActions;

  clr.clearAllTableKeys();

  if (modelActions.createTableKeys()) {
    if (old.createPrimaryKeys()) { modelActions.enableOnlyTablePrimaryKeys();     storeUserTablesKeys(linterWriter); }
    if (old.createUniqueKeys() ) { modelActions.enableOnlyTableUniqueKeys();      storeUserTablesKeys(linterWriter); }
    if (old.createChecks()     ) { modelActions.enableOnlyTableChecks();          storeUserTablesKeys(linterWriter); }
    if (old.createForeignKeys()) { modelActions.enableOnlyTableForeignKeys();     storeUserTablesKeys(linterWriter); }
    if (old.dropForeignKeys()  ) { modelActions.enableOnlyDropTableForeignKeys(); storeUserTablesKeys(linterWriter); }
    if (old.createOtherKeys()  ) { modelActions.enableOnlyTableOtherKeys();       storeUserTablesKeys(linterWriter); }
  }
  modelActions = old;
}

void UserContext::initUserOnStream(Sm::Codestream &str) {
  setStreamMode(str);
  str.activatePreactions();
  str << s::connect(this);
  str.activatePrevious();
}

void UserContext::storeSQLTableKeys(LinterWriter &linterWriter) {
  Codestream str;
  initUserOnStream(str);
  for (Tables::iterator it = tables.begin(); it != tables.end(); ++it )
    if (!it->second->isSystem()) {
      it->second->linterDefinitionKeys(str);
      str.join();
    }

  str.join();
  str.joinPreactions();
  str.joinUserpostactions();
  linterWriter.storeSqlMetadata(str);
}


void UserContext::storeSQL(LinterWriter &linterWriter, ModelActions &actions, DependEntitiesMap &filter) {
  Codestream str;
  str.state().currentUser_ = (UserContext*)this;
  initUserOnStream(str);

  stroreSqlContainer(filter, actions.createSequences(), str, sequences);
  if (syntaxerContext.model->modelActions.diffMode) {
    for (DiffTables::iterator it = diffTables.begin(); it != diffTables.end(); ++it)
      if (!it->second.first && it->second.second)
        storeSqlContainerItem(str, it->second.second);
      else if (it->second.first && !it->second.second)
        storeSqlContainerItem(str, it->second.first);
      else if (it->second.first->diff(it->second.second)) {
        it->second.first->diffOutput(str);
        str.join();
      }
  }
  else if (actions.createTables() || actions.dropTables())
    for (Tables::iterator it = tables.begin(); it != tables.end(); ++it ) {
      if (filter.size() && !filter.count(it->second))
        continue;

//      if (it->second->hasNumberFieldsThatWillChanged())
      storeSqlContainerItem(str, it->second, actions.createTables(), true);
    }

  stroreSqlContainer(filter, actions.createIndices(), str, indices , actions.dropIndices(), false);
  stroreSqlContainer(filter, actions.createSynonym(), str, synonyms);

  Codestream::SortedGranteesClauses sortedGranteesClauses;
  if (actions.createViews())
    for (Views::iterator it = views.begin(); it != views.end(); ++it ) {
      if (filter.size() && !filter.count(it->second))
        continue;
      str << syntaxerContext.createStatement << it->second << s::semicolon << s::endl;
      needToGranteeToSortedGrantee(sortedGranteesClauses, str.needToGrantee);
      str.needToGrantee.clear();
      str.join();
      str << s::endl;
    }

  str.activatePrefixes();
  outputSortedGranteeClause(sortedGranteesClauses, str, this, false);
  str.activateActions();

  stroreSqlContainer(filter, actions.createTriggers() , str, triggers, false, false);

  str.needToGrantee.clear();
  stroreSqlContainer(filter, actions.createProcedures(), str, functions, false, false);
  if (actions.createProcedures() && packages.size())
    for (Packages::iterator it = packages.begin(); it != packages.end(); ++it) {
      it->second->filter_ = &filter;
      str << s::def << it->second;
    }

  str.join();
  str.joinPreactions();

  linterWriter.storeSqlMetadata(str);
}

Sm::Codestream& Translator::translate(Codestream &s) {
  switch (s.dbMode()) {
    case Sm::CodestreamState::LINTER:
      switch (s.procMode()) {
        case Sm::CodestreamState::SQL:
          switch (s.namesMode()) {
            case Sm::CodestreamState::DEFINITION : sqlDefinition (s); break;
            case Sm::CodestreamState::DECLARATION: sqlDeclaration(s); break;
            case Sm::CodestreamState::REFERENCE  : sqlReference  (s); break;
          }
          break;
        case Sm::CodestreamState::PROC:
          switch (s.namesMode()) {
            case Sm::CodestreamState::DEFINITION : linterDefinition (s); break;
            case Sm::CodestreamState::DECLARATION: linterDeclaration(s); break;
            case Sm::CodestreamState::REFERENCE  : linterReference  (s); break;
          }
          break;
      }
      break;
    case Sm::CodestreamState::ORACLE:
      switch (s.namesMode()) {
        case Sm::CodestreamState::DEFINITION : oracleDefinition (s); break;
        case Sm::CodestreamState::DECLARATION: oracleDeclaration(s); break;
        case Sm::CodestreamState::REFERENCE  : oracleReference  (s); break;
      }
      break;
  }
  s << s::def;
  return s;
}


void ResolvedEntity::oracleDefinition(Sm::Codestream &str) {
  str << "!!! Error: oraleDefinition not implemented for " << Sm::toString(ddlCathegory());
}

void UserContext::linterDefinition(Sm::Codestream &str) {
  if (getName()) {
    str << s::ocreate(this) << "USER " << s::cref(this) << " IDENTIFIED BY ";
//    if (password)
//      str << password;
    str << "'MANAGER'" << s::semicolon;
    str << s::ccreate;
  }
}

void UserContext::grantResourceDefinition(Sm::Codestream &str) {
  str << s::ogrant << "GRANT RESOURCE TO " << s::cref(this) << s::semicolon << s::endl << s::cgrant;
}


template <class T>
int numDigits(T number)
{
  if (number == 0)
    return 0;
  int digits = 0;
  while (number) {
      number /= 10;
      digits++;
  }
  return digits;
}

namespace Sm {

Codestream& operator<< (Codestream &os, List<Id> *v) {
  if (v && !v->empty()) {
    os << **(v->begin());
    for (List<Id>::const_iterator i = ++(v->begin()); i != v->end(); ++i)
      os << s::CommaCodegen() << **i;
  }
  return os;
}

Codestream& operator<< (Codestream& os, const Id2& str) {
  if (str.id[1] && !str.id[1]->empty())
    os << *(str.id[1]) << '.';
  if (str.id[0])
    os << *(str.id[0]);
  return os;
}

void translateMultilineText(Codestream& os, string &s)
{
  unsigned int lastLineLength = Sm::s::MultilineTextChunck::getLastLineLength(s);
  if (lastLineLength != s.length()) {
    s::MultilineTextChunck *instance = new s::MultilineTextChunck(lastLineLength);
    instance->text_.swap(s);
    os.addSpacer(instance);
  }
  else
    os << s;
}

Codestream& operator<< (Codestream& os, const Id& str) {

  if (str.quoted() && str.empty()) {
    os << "\"\"";
    if (os.isSql())
      throw 999;
  }
  else {
    ResolvedEntity *d;
    if ((d = str.definition()) && !(d->toSelfDatatype() && d->isVarcharDatatype()))
      os << str.toQInvariantNormalizedString(os.isProc());
    else {
      string s;
      str.toQInvariantNormalizedString(s, os.isProc());
      translateMultilineText(os, s);
    }

  }
  return os;
}

Codestream& operator<< (Codestream& os, EntityFields &fields) {
  if (!fields.empty()) {
    EntityFields::iterator it = fields.begin();
    os << (*it);
    for (++it; it != fields.end(); ++it)
      os << s::CommaCodegen() << (*it);
  }
  return os;
}

std::string Id::toCodeId(ResolvedEntity* space, bool addToNamespace/* = false*/, bool notChangeReserved) const {
  if (space)
    return toCodeId(space->getSemanticNode()->levelNamespace, addToNamespace, notChangeReserved);
  else
    return toQString();
}

std::string Id::toCodeId(LevelResolvedNamespace *levelNamespace, bool addToNamespace/* = false*/, bool notChangeReserved) const {
  ResolvedEntity *definition = unresolvedDefinition();
  if (definition && definition->eid() == 53584)
    cout << "";
  std::string dst = definition->translatedName();
  if (dst.empty()) {
    if (definition->isVariable() || definition->toSelfIndex())
      dst = toNormalizedString();
    else
      dst = getText();
  }

  bool notChanged = true;
  for (std::string::iterator it = dst.begin(); it != dst.end(); ++it)
    if (*it == '#' || *it == '$') {
      *it = '_';
      notChanged = false;
    }
  if (notChanged) {
    std::string s = dst;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s == "RESULT") {
      dst = "res__";
      notChanged = false;
    }
    else if (Id::isFieldReserved(s) && !notChangeReserved) {
      dst += '_';
      notChanged = false;
    }
  }
  auto generateName = [&]() {
    dst = levelNamespace->getUniqueName(dst);
    definition->translatedName(dst);
    LevelResolvedNamespace::iterator it = levelNamespace->find(dst);
    if (it != levelNamespace->end())
      throw 999; // неправильно работает код для получения уникальных идентификаторов.
  };

  definition->translatedName(dst);
  if (!notChanged || addToNamespace) {
    LevelResolvedNamespace::iterator it = levelNamespace->find(dst);
    if (it != levelNamespace->end()) {
      if (it->second->compareWithFront(definition) > 0)
        return dst; // в начале соответствующего списка стоит тот же элемент
      else { // в пространстве имен либо находится другой эл-т с таким именем, либо отсутствует
        generateName();
        levelNamespace->addWithoutFind(const_cast<Id*>(this));
      }
    }
    else {
      if (levelNamespace->semanticLevel->cathegory == SCathegory::BlockPlSql) {
        LevelResolvedNamespace *funcNamespace = levelNamespace->parent;
        if (!funcNamespace) {
          stringstream s;
          s << "error: funcNamespace in toCodeId is NULL. id = " << this->getLLoc().locText() << endl;
          cout << s.str();
          return s.str();
        }
        LevelResolvedNamespace::iterator pIt = funcNamespace->find(dst);
        if (pIt != funcNamespace->end()) {
          // в пространстве аргументов функции найдено такое имя
          generateName();
        }
      }
      levelNamespace->addWithoutFind(const_cast<Id*>(this));
    }
  }
  return dst;
}

void Id::toQuotedNondateLiteral(std::string &dst, bool isProcMode) const
{
  if (empty())
    dst.append("''"); // sql mode ok else "" => err;
  else {
    if (!isProcMode &&
        (isdigit(text->front()) || isStringLiteral() || !definition() || squoted() ||
         (idDefinition_ && !idDefinition_->isField() && idDefinition_->isVarcharDatatype())
        )
       )
      PlsqlHelper::quotingAndEscaping(dst, *text, '\'');
    else
      PlsqlHelper::quotingAndEscapingSpec(dst, *text, '\"', '\\');
  }
}


bool Id::isSqlIdentificator() const {
  // проверка выполняется для сущностей, находящихся вне процедурного
  // контекста, и часть сущностей может быть указана некорректно
  static const unordered_set<CathegoriesOfDefinitions::ScopedEntities> sqlIdCats = {
    FieldOfRecord_,
    FieldOfTable_,
    QueriedPseudoField_,
    SqlSelectedField_,
    FieldOfVariable_ ,
    Variable_,
    FunctionArgument_,
    TriggerRowReference_,
    TriggerPredicateVariable_,
    FromSingle_,
    FactoringItem_,
    MemberFunction_,
    Function_,
    Table_,
    View_,
    Index_,
    IndexUnique_,
    Sequence_,
  };
  ResolvedEntity *d;
  return (d = definition()) && sqlIdCats.count(d->ddlCathegory());
}


std::string Id::getQuotedText() const {
  string dst;
  PlsqlHelper::quotingAndEscaping(dst, toNormalizedString(), '\"');
  return dst;
}

std::string Id::getText() const {
  if (hasSpecSymbols()) {
    std::string dst;
    PlsqlHelper::quotingAndEscaping(dst, toNormalizedString(), '\"');
    return dst;
  }
  else
    return text ? *text : string();
}

void Id2::toNormalizedString(string &dst) const {
  if (id[1]) { // size = 2
    id[1]->toNormalizedString(dst);
    dst += '.';
  }
  if (id[0]) // size = 1
    id[0]->toNormalizedString(dst);
}

void IdEntitySmart::toNormalizedString(string &dst) const {
  if (empty())
    return;
  IdEntitySmart::const_reverse_iterator it = this->rbegin();
  size_t len = 0;
  for ( ; it != this->rend(); ++it)
    len += (*it)->length();
  len += (size()-1);
  dst.reserve(dst.size() + len);

  it = this->rbegin();
  if (*it)
    (*it)->toNormalizedString(dst);
  for (++it ; it != this->rend(); ++it) {
    dst += ".";
    if (*it)
      (*it)->toNormalizedString(dst);
  }
}

void IdEntitySmart::toStringWithType(std::string &str) const {
  auto type2String = [](std::string &str, ResolvedEntity *otype) {
    if (otype->isVarray())
      str += "[VARRAY ";
    else if (otype->isAssocArray())
      str += "[ASSOC ";
    else if (otype->isNestedTable())
      str += "[NTABLE ";
    else if (otype->isObject())
      str += "[OBJ ";
    else
      str += "[";
    str += otype->getName()->toNormalizedString() + "]";
  };

  bool isNotFirst = false;
  for (auto rIt = rbegin(); rIt != rend(); ++rIt) {
    if (isNotFirst && !(*rIt)->definition()->isCollectionAccessor())
      str += ".";
    else
      isNotFirst = true;
    str += (*rIt)->toNormalizedString();
    if (((*rIt)->callArglist && (*rIt)->callArglist->size() > 0) || (*rIt)->isMethod())
      str += "(...)";

    if (Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype((*rIt)->getDatatype()))
      type2String(str, datatype->getNextDefinition());
    else
      str += "[UNK]";
  }
}

void IdEntitySmart::toQString(std::string &dst) const {
  Codestream str;
  str << *this;
  dst += str.str();
}

Sm::Codestream &IdEntitySmart::translate(Sm::Codestream &str) {
  str << *this; return str;
}


typedef std::vector<std::pair<int, Ptr<Sm::PlExpr> > > NamedFunArgExpr;

bool leastNamedFunArg(NamedFunArgExpr::value_type v1, NamedFunArgExpr::value_type v2) { return v1.first < v2.first; }

void translateCallArg(Codestream &str, Ptr<PlExpr> argExpr) {
  RefAbstract *exprId;
  ResolvedEntity *pCollection;
  if ((exprId = argExpr->toSelfRefAbstract()) != NULL && exprId->refSize() > 1 &&
       exprId->reference->majorObjectRef(&pCollection) && exprId->reference->isDynamicUsing()) {
    Ptr<Id> ent = exprId->refEntity();
    Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(ent->getDatatype());
    if (datatype && (datatype->isRowTypeOf() || datatype->isRecordType())) {
      string declVarName;
      ResolvedEntity *majorDef = exprId->reference->majorEntity()->definition();
      if (Ptr<Sm::Variable> var = majorDef->getTemporaryVar(str, datatype, &declVarName, true)) {
        str.levelPush();
        str.activateActions();
        str << s::tab();
        translateCursorAsSelectCollection(str, *(exprId->reference), pCollection, var);
        str << s::procendl(exprId->getLLoc());
        str.actionsToPrefixes();
        str.levelPop();
        str << declVarName;
        return;
      }
    }
  }

  str << argExpr;
}

void translateCallArglist(Codestream &str, ResolvedEntity *funDef, Ptr<Sm::CallArgList> callArglist) {
  bool isNotFirst = false;
  if (!funDef) {
    str << callArglist;
    return;
  }
  else if (Sm::Function *fun = funDef->toSelfFunction()) {
    if (!fun->externalVariables.empty()) {
      funDef->translateExternalVariablesToCallarglist(str, 0);
      isNotFirst = true;
    }
  }

  if (!callArglist)
    return;

  Ptr<Sm::FunArgList> defArglist = funDef->funArglist();
  Sm::CallArgList::iterator callIt = callArglist->begin();
  if (!defArglist) {
    for (; callIt != callArglist->end(); ++callIt) {
      switch ((*callIt)->argclass()) {
      case Sm::FunCallArg::ASTERISK:
        if (str.procMode() == CodestreamState::PROC)
          throw 999; // Звёздочек в процедурном языке быть не должно.
        str << s::comma(&isNotFirst) << '*';
        break;
      case Sm::FunCallArg::POSITIONAL:
        str << s::comma(&isNotFirst);
        translateCallArg(str, (*callIt)->expr());
        break;
      case Sm::FunCallArg::NAMED:
        throw 999; // В методах коллекций нет именованых аргументов
        break;
      }
    }
    return;
  }

  Sm::FunArgList::iterator defIt = defArglist->begin();

  NamedFunArgExpr namedExprs;
  for (; callIt != callArglist->end() && defIt != defArglist->end(); ++callIt) {
    switch ((*callIt)->argclass()) {
      case Sm::FunCallArg::ASTERISK:
        if (str.procMode() == CodestreamState::PROC)
          throw AsteriskInReferenceTranslation(); // Звёздочек в процедурном языке быть не должно.
        str << s::comma(&isNotFirst) << '*';
        ++defIt;
        break;
      case Sm::FunCallArg::POSITIONAL:
        str << s::comma(&isNotFirst);
        translateCallArg(str, (*callIt)->expr());
        ++defIt;
        break;
      case Sm::FunCallArg::NAMED: {
        if (!(*callIt)->argname())
          throw 999; // должно быть задано для именованного аргумента
        unsigned int pos;
        Ptr<FunctionArgument> arg = findByNormalizedName(funDef, callIt, pos);
        if (arg) {
          (*callIt)->setArgPositionInDef(pos);
          namedExprs.push_back(make_pair(pos, (*callIt)->expr()));
        }

//        for (Sm::FunArgList::iterator it = defIt; it != defArglist->end(); ++it)
//          if (*((*callIt)->argname()) == *((*it)->getName())) {
//            int pos = std::distance(defArglist->begin(), it);
//            (*callIt)->setArgPositionInDef(pos);
//            namedExprs.push_back(make_pair(pos, (*callIt)->expr()));
//          }
      } break;
    }
  }
  if (namedExprs.size()) {
    std::sort(namedExprs.begin(), namedExprs.end(), leastNamedFunArg);
    int previousPos = namedExprs.front().first;
    for (NamedFunArgExpr::iterator it = namedExprs.begin(); it != namedExprs.end(); ++it) {
      if ((it->first - previousPos) > 1)
        for (++previousPos; previousPos != it->first; ++previousPos) {
          str << s::comma(&isNotFirst) << (*defArglist)[previousPos]->defaultValue();
        }
      str << s::comma(&isNotFirst);
      translateCallArg(str, it->second);
      previousPos = it->first;
    }
  }
}


void translateUndefinedReference(Codestream &str, Ptr<Id> id)
{
  if (id->quoted()) {
    str << s::ref << id;
    if (id->callArglist)
      throw 999; // аргументы неизвестной функции. Функция должна быть разрешена
    str << s::def;
  }
  else {
    str << id;
    if (id->callArglist)
      str << s::obracket << id->callArglist << s::cbracket;
  }
}

void translateReferenceCallarglist(Ptr<ResolvedEntity> definition, Ptr<Id> id, Codestream &str)
{
  Sm::CallarglistTranslator *tr = definition->callarglistTR();
  if (definition->bracesOutput())
    str << s::obracket;
  str << s::def;
  if (id->beginedFrom(117869,33))
    cout << "";
  if (tr)
    (*tr)(id, str);
  else if (id->callArglist && definition->interTranslateCallArg(str, id, id->callArglist)) {
    // Весь вывод сделан в interTranslateCallArg
  }
  else
    translateCallArglist(str, id->definition(), id->callArglist);
  if (definition->bracesOutput())
    str << s::cbracket;
}

Sm::Codestream& translateIdReference(Codestream &str, Ptr<Id> id, bool &isNotFirst, ResolvedEntity::ScopedEntities &prevCat, bool &userAlreadyOutput) {
  if (!id)
    return str;

  Ptr<ResolvedEntity> definition = id->definition();
  if (definition && !definition->hasLinterEquivalent())
    return str;

  if (isNotFirst && (!definition || !definition->isCollectionAccessor()))
    str << ".";
  else
    isNotFirst = true;

  SemanticTree *node = id->semanticNode();
  Sm::QueryPseudoField *psf;

  if (!definition)
    translateUndefinedReference(str, id);
  else {
    ResolvedEntity::ScopedEntities currCat = definition->ddlCathegory();
    if (Sm::NameTranslator nameTr = definition->nameTR())
      nameTr(str, id->callArglist);
    else if (definition->isFunction()) {
      if (!userAlreadyOutput && definition->userContext() && !definition->isElementaryLinterFunction())
        str << s::cref(definition->userContext()) << '.';
      definition->translateName(str);
    }
    else if (currCat == ResolvedEntity::FundamentalDatatype_ ||
             ((psf = definition->toSelfQueryPseudoField()) && psf->isColumnValue))
      str << s::cref(definition);
    else if (id->quoted() ||
        currCat == ResolvedEntity::FromSingle_       ||
        currCat == ResolvedEntity::SqlSelectedField_ ||
        currCat == ResolvedEntity::QueriedTable_ ||
        currCat == ResolvedEntity::SqlEntity_ ||
        prevCat == ResolvedEntity::FromSingle_||
             str.isSelectedField() || id->isRownumPseudocol() ||
             id->isRowidPseudocol())
      str << id;
    else if (currCat == ResolvedEntity::FieldOfTable_ &&
             prevCat == ResolvedEntity::TriggerRowReference_)
      str << id;
    else if (currCat == ResolvedEntity::CollectionMethod_) {
      // Не выводим идентификатор. Будет выведен в interTranslateCallArg
    }
    /*  DATATYPE_REFERENCE - ссылка на тип
     *  SCathegory::Datatype - тип переменной/поля, но не вызова New и не часть выражения Is Of (TYPES)
     */
    else if (node && node->nametype == SemanticTree::DATATYPE_REFERENCE && node->cathegory == SCathegory::Datatype) {
      if (Ptr<Id> n = definition->getName()) {
        bool needToQuoting = false;
        ResolvedEntity::ScopedEntities defCat = definition->ddlCathegory();
        bool hasSpecChar = str.isProc() ? n->hasDollarSymbols() : n->hasSquareSymbols();
        bool defIsTblEnt = (defCat == ResolvedEntity::Table_ || defCat == ResolvedEntity::View_ || defCat == ResolvedEntity::Sequence_);
        if (definition->isField()) {
          if (n->isReservedField() || hasSpecChar)
            needToQuoting = true;
        }
        else if (defIsTblEnt || defCat == ResolvedEntity::Function_ || defCat == ResolvedEntity::MemberFunction_) {
          if (n->isReservedField() || hasSpecChar)
            needToQuoting = true;
        }

        if (needToQuoting) {
          if (defIsTblEnt) {
            Ptr<Id2> n2 = definition->getName2();
            if (n2->id[1] && !n2->id[1]->empty())
               str << *(n2->id[1]) << '.';
          }
          str << n->getQuotedText();
        }
        else
          str << s::cref(definition);
      }
      else if (definition->getDatatype()->isNull())
        str << s::cref(definition);
      else
        throw 999;
    }
    else
      str << s::cref(definition);

    Sm::Function *fun = definition->toSelfFunction();
    if (id->callArglist || (fun && (fun->flags.v & (FLAG_FUNCTION_ANYDATA_MEMBER | FLAG_FUNCTION_XML_FUNCTION))))
      translateReferenceCallarglist(definition, id, str);
    else if (definition && !definition->isLinterQueryLiteral())
      if (Function *fun = definition->toSelfFunction()) {
        fun->extractExternalVariables();
        if (!fun->externalVariables.empty())
          translateReferenceCallarglist(definition, id, str);
        else if (fun->bracesOutput())
          str << s::obracket << s::cbracket;
      }

    userAlreadyOutput = (currCat == ResolvedEntity::User_);
    prevCat = currCat;
  }
  return str;
}


/* Определить, является ли ent - внешней для динамической конструкции переменной или нет */
bool isExternalQueryParam(const IdEntitySmart& ent) {
  Ptr<Id> major = ent.majorEntity();

  std::set<ResolvedEntity::ScopedEntities> ents;

  if (ResolvedEntity *majDef = major->definition())
    switch (majDef->ddlCathegory()) {
      case ResolvedEntity::VariableUndeclaredIndex_:
      case ResolvedEntity::FunctionArgument_:
      case ResolvedEntity::Variable_:
      case ResolvedEntity::MemberVariable_:
      case ResolvedEntity::TriggerRowReference_:
      case ResolvedEntity::CollectionMethod_:
        if (ent.size())
          return true;
        else
          throw 999; // бываете ли такое ??
        break;
      case ResolvedEntity::SqlSelectedField_: {
        RefAbstract *expr;
        SqlExpr *sExpr;
        SelectedField *fld;
        if ((fld = majDef->toSelfSelectedField()) &&
            (sExpr = fld->expr()) && (expr = sExpr->toSelfRefAbstract()))
          return isExternalQueryParam(*(expr->reference));
        break;
      }

      default:
        break;
    }

  if (ResolvedEntity *entDef = ent.definition()) {
    switch (entDef->ddlCathegory()) {
      case ResolvedEntity::FunctionArgument_:
      case ResolvedEntity::Variable_:
      case ResolvedEntity::MemberVariable_:
      case ResolvedEntity::LinterCursorField_:
      case ResolvedEntity::CollectionMethod_:
  case ResolvedEntity::FieldOfRecord_:
        return true;
      default:
        break;
    }
  }
  return false;
}

bool needCastStaticQueryArg(SemanticTree *semNode) {
  SemanticTree *parentNode = (semNode != NULL) ? semNode->getParent() : NULL;
  if (!parentNode)
    return false;
  switch (parentNode->cathegory) {
    case SCathegory::Function: {
      if (ResolvedEntity *parentDef = parentNode->entityDef())
        return (parentDef->isSystem() || parentDef->isSystemTemplate());
      else {
        cout << "error: unresoved semantic node " << parentNode->sid << " " << parentNode->getLocText() << endl;
        return false;
      }
    }
    case SCathegory::AlgebraicCompound: //
    case SCathegory::UnaryOp:           //
    case SCathegory::QueryBlockField:   //
    case SCathegory::Case:              //
      return true;
    case SCathegory::Comparsion: {
      if (parentNode->childs.size() < 2)
        return false;
      SemanticTree *otherNode;
      if (parentNode->childs.begin() == semNode->getPositionInParent())
        otherNode = *(++parentNode->childs.begin());
      else
        otherNode = *(parentNode->childs.begin());
      // внешним параметром запроса является статическая декларация процедурного кода
      if (otherNode->reference() && isExternalQueryParam(*otherNode->reference()))
        return true;
  } break;
  default:
    break;
  }
  return false;
}


SemanticTree *getFirstLevelNamespaceSnode(SemanticTree *n);

struct FirstLevelNamespaceTraversionTable : public SCathegory {
  typedef Sm::SemanticTree* (*action)(Sm::SemanticTree*/*curr*/, Sm::SemanticTree*/*par*/);

  action arcs[__LAST_SCATHEGORY__][__LAST_SCATHEGORY__];

  static Sm::SemanticTree* getCurr(Sm::SemanticTree* curr, Sm::SemanticTree*/*par*/) { return curr; }
  static Sm::SemanticTree* getPar(Sm::SemanticTree* /*curr*/, Sm::SemanticTree* par) { return par; }
  static Sm::SemanticTree* getDad(Sm::SemanticTree* /*curr*/, Sm::SemanticTree* par) { return par->getParent(); }

  static Sm::SemanticTree* fieldParent(Sm::SemanticTree* /*curr*/, Sm::SemanticTree* par) {
    return getFirstLevelNamespaceSnode(par);
  }

  static Sm::SemanticTree* getModelContext(Sm::SemanticTree* /*curr*/, Sm::SemanticTree*/*par*/) {
    return syntaxerContext.model->getSemanticNode();
  }

  static Sm::SemanticTree* selectedFieldParent(Sm::SemanticTree* /*curr*/, Sm::SemanticTree* par) {
    switch (par->getParent()->cathegory) {
      case From:
      case QueryBlock:
      case Cursor:
        return getFirstLevelNamespaceSnode(par);
      default:
        break;
    }
    switch (par->cathegory) {
      case Cursor:
      case QueryBlock:
        return getFirstLevelNamespaceSnode(par);
      default:
        break;
    }

    return par;
  }

  FirstLevelNamespaceTraversionTable();
};

FirstLevelNamespaceTraversionTable::FirstLevelNamespaceTraversionTable() {
  for (int i = 0; i < __LAST_SCATHEGORY__; ++i)
    for (int j = 0; j < __LAST_SCATHEGORY__; ++j)
      arcs[i][j] = FirstLevelNamespaceTraversionTable::getCurr;

  arcs[Variable     ][ForOfExpression] = FirstLevelNamespaceTraversionTable::getPar;
  arcs[Variable     ][BlockPlSql     ] = FirstLevelNamespaceTraversionTable::getPar;
  arcs[RecordType   ][BlockPlSql     ] = FirstLevelNamespaceTraversionTable::getPar;
  arcs[Variable     ][Variable       ] = FirstLevelNamespaceTraversionTable::fieldParent;
  arcs[Argument     ][Function       ] = FirstLevelNamespaceTraversionTable::getPar;
  arcs[From         ][BlockPlSqlStatementsList] = FirstLevelNamespaceTraversionTable::getDad;

  arcs[SelectedField][QueryBlock     ] = FirstLevelNamespaceTraversionTable::selectedFieldParent;
  arcs[QueryBlock   ][From           ] = FirstLevelNamespaceTraversionTable::selectedFieldParent;
  arcs[From         ][QueryBlock     ] = FirstLevelNamespaceTraversionTable::selectedFieldParent;
  arcs[QueryBlock   ][Cursor         ] = FirstLevelNamespaceTraversionTable::selectedFieldParent;

  for (int i = 0; i < __LAST_SCATHEGORY__; ++i) {
    arcs[Package][i] = getModelContext; // пакетные сущности могут принадлежать пакету из другого поддерева
    arcs[i][Package] = getModelContext; // пакетные сущности могут принадлежать пакету из другого поддерева
    arcs[User][i]    = getModelContext;
    arcs[i][User]    = getModelContext;
  }
}





struct DynamicContextCases : public CathegoriesOfDefinitions, public SCathegory, public SemanticNameType {
  typedef bool (*action)(ResolvedEntity*/*def*/, Sm::SemanticTree*/*par*/);

  action needSignCases[LAST_ENTITY_NUMBER][__LAST_SCATHEGORY__][LAST_NAMETYPE_IDX];
  action isDynCtxCases[LAST_ENTITY_NUMBER][__LAST_SCATHEGORY__][LAST_NAMETYPE_IDX];
  action needStaticCasting[LAST_ENTITY_NUMBER][__LAST_SCATHEGORY__][LAST_NAMETYPE_IDX];

  static bool falseStub(ResolvedEntity*/*def*/, Sm::SemanticTree*/*par*/) { return false; }
  static bool trueStub(ResolvedEntity*/*def*/, Sm::SemanticTree*/*par*/) { return true; }
  static bool parentOfQueryBlock(ResolvedEntity*/*def*/, Sm::SemanticTree *par);
  static bool parentRefAbstract(ResolvedEntity*/*def*/, Sm::SemanticTree *par);
  static bool parentFunRef(ResolvedEntity*/*def*/, Sm::SemanticTree *par);

  DynamicContextCases();

  template <typename T>
  static void setVariableCase(T &arr, SCathegory::t cat, int pos, action act) {
    arr[Variable_           ][cat][pos] = act;
    arr[FunctionArgument_   ][cat][pos] = act;
    arr[MemberVariable_     ][cat][pos] = act;
    arr[TriggerRowReference_][cat][pos] = act;
  }
};


bool DynamicContextCases::parentOfQueryBlock(ResolvedEntity*/*def*/, Sm::SemanticTree *par) {
  SemanticTree *p = nAssert(par->getParent());
  switch (p->cathegory) {
    case StatementForOfExpression:
    case StatementOpenCursor:
    case StatementSingleInsert:
    case StatementDeleteFrom:
    case SCathegory::Update:
    case StatementOpenFor:
    case StatementSelect:
    case SqlStatement:
      // для общего случая определения необходимости подстановки знаков
    case StatementMerge:
    case BlockPlSqlStatementsList:
      return true;
    default:
      return false;
  }
  return false;
}

bool DynamicContextCases::parentRefAbstract(ResolvedEntity*/*def*/, Sm::SemanticTree *par) {
  SemanticTree *p = nAssert(par->getParent());
  switch (p->cathegory) {
    case StatementOpenCursor:
      return true;
    default:
      return false;
  }
  return false;
}

bool DynamicContextCases::parentFunRef(ResolvedEntity*/*def*/, Sm::SemanticTree *par) {
  SemanticTree *p = nAssert(par->getParent());
  switch (p->cathegory) {
    case StatementForOfExpression:
      return true;
    default:
      return false;
  }
  return false;
}


DynamicContextCases::DynamicContextCases() {
  for (int i = 0; i < LAST_ENTITY_NUMBER; ++i)
    for (int j = 0; j < __LAST_SCATHEGORY__; ++j)
      for (int k = 0; k < SemanticTree::LAST_NAMETYPE_IDX; ++k) {
        needStaticCasting[i][j][k] = falseStub;
        isDynCtxCases    [i][j][k] = falseStub;
        needSignCases    [i][j][k] = falseStub;
      }

  setVariableCase(isDynCtxCases, SCathegory::StatementConstructBlockPlsql, SemanticTree::NEW_LEVEL, trueStub);

  for (int k = 0; k < SemanticTree::LAST_NAMETYPE_IDX; ++k) {
    setVariableCase(isDynCtxCases, StatementConstructExpr, k, trueStub); // переменные, формирующие динамическое выражение
    isDynCtxCases[Function_][DynamicFuncallTranslator][k] = trueStub; // вызов системной функции, являющейся динамическим транслятором функций
    for (int i = 0; i < LAST_ENTITY_NUMBER; ++i) { // граница узла сущности динамического pl/sql - всегда признак динамического случая
      isDynCtxCases[i][SCathegory::FunctionDynField][k] = trueStub;
      isDynCtxCases[i][SCathegory::FunctionDynExpr ][k] = trueStub;
      isDynCtxCases[i][SCathegory::FunctionDynTail_][k] = trueStub;
    }
  }

  for (int i = 0; i < LAST_ENTITY_NUMBER; ++i)
    for (int j = 0; j < __LAST_SCATHEGORY__; ++j)
      for (int k = 0; k < SemanticTree::LAST_NAMETYPE_IDX; ++k)
        needSignCases[i][j][k] = isDynCtxCases[i][j][k];

    // переменные внутри запросов

//  setVariableCase(needSignCases, StatementForOfExpression, SemanticNameType::EMPTY, trueStub);
//  setVariableCase(needSignCases, StatementOpenCursor     , SemanticNameType::EMPTY, trueStub);
//  setVariableCase(needSignCases, StatementSingleInsert, NEW_LEVEL, trueStub);
//  setVariableCase(needSignCases, StatementDeleteFrom  , NEW_LEVEL, trueStub);
//  setVariableCase(needSignCases, Update               , NEW_LEVEL, trueStub);



  setVariableCase(needSignCases, RefAbstract       , REFERENCE              , parentRefAbstract); // OpenCursor only
  setVariableCase(needSignCases, Expr_HostSecondRef, REFERENCE              , parentRefAbstract); // OpenCursor only
  setVariableCase(needSignCases, Function          , REFERENCE              , parentFunRef); // ForOfExpression only
  setVariableCase(needSignCases, SqlStatement      , NEW_LEVEL              , parentOfQueryBlock);
  setVariableCase(needSignCases, SqlStatement      , REFERENCE              , parentOfQueryBlock);
  setVariableCase(needSignCases, QueryBlock        , DECLARATION            , parentOfQueryBlock);
  setVariableCase(needSignCases, UnionQuery        , NEW_LEVEL              , parentOfQueryBlock);
  setVariableCase(needSignCases, SelectBrackets    , SemanticNameType::EMPTY, parentOfQueryBlock);
  setVariableCase(needSignCases, SelectSingle      , SemanticNameType::EMPTY, parentOfQueryBlock);

  setVariableCase(needSignCases, SCathegory::Cursor, DECLARATION, trueStub);


  needSignCases[Function_][SCathegory::QueryBlock][DECLARATION] = parentOfQueryBlock;
  needStaticCasting[Function_][SCathegory::DynamicFuncallTranslator][SemanticTree::EMPTY] = trueStub;
}



class DynamicContextDetector {
  void detectHierarchyDynamicState(SemanticTree *refSnode, SemanticTree *defLevel);

public:
  bool isDynCtx          = false;
  bool needStaticCasting = false;
  bool isSignCase        = false;

  void detectDynamicContext(ResolvedEntity *d, ResolvedEntity *ref);

  void clear() {
    isDynCtx          = false;
    needStaticCasting = false;
    isSignCase        = false;
  }
  SemanticTree* getIntoOperator(SemanticTree *defLevel, SemanticTree *it);
};

void DynamicContextDetector::detectHierarchyDynamicState(SemanticTree *refSnode, SemanticTree *defLevel) {
  SemanticTree *prev = 0;
  for (SemanticTree *it = refSnode; it && it != defLevel; it = it->getParent()) {
    switch (it->cathegory) {
      case SCathegory::Into:
        return;
      case SCathegory::StatementForOfExpression:
      case SCathegory::StatementOpenFor:
      case SCathegory::StatementSelect:
      case SCathegory::StatementMerge:
      case SCathegory::StatementSingleInsert:
      case SCathegory::StatementDeleteFrom:
      case SCathegory::StatementOpenCursor:
      case SCathegory::Update: {
        if (it->hasDynamicChilds() || nAssert(prev)->hasDynamicChilds()) {
          isDynCtx = true;
          isSignCase = true;
        }
        return;
      }
      case SCathegory::Function:
        if (it->nametype == SemanticTree::REFERENCE && it->hasDynamicChilds()) {
          isDynCtx = true;
          isSignCase = true;
          return;
        }
        break;
      default:
        break;
    }
    prev = it;
  }
}


SemanticTree *getFirstLevelNamespaceSnode(SemanticTree *n) {
  SemanticTree *defLevel = 0;
  for (; n; n = n->getParent())  {
    LevelResolvedNamespace *sp;
    if ((sp = n->levelNamespace) && sp->semanticLevel) {
      defLevel = sp->semanticLevel;
      break;
    }
  }
  if (!defLevel)
    return syntaxerContext.model->getSemanticNode();

  static FirstLevelNamespaceTraversionTable transformTable;

  SemanticTree *par;
  // переустановка общего предка
  if ((par = defLevel->getParent()))
    return transformTable.arcs[defLevel->cathegory][par->cathegory](defLevel, par);

  return defLevel;
}


SemanticTree* DynamicContextDetector::getIntoOperator(SemanticTree *defLevel, SemanticTree *it)
{
  for (; it && it != defLevel; it = it->getParent())
    switch (it->cathegory) {
      case SCathegory::StatementForOfExpression:
      case SCathegory::StatementOpenFor:
      case SCathegory::StatementSelect:
      case SCathegory::StatementMerge:
      case SCathegory::StatementSingleInsert:
      case SCathegory::StatementDeleteFrom:
      case SCathegory::StatementOpenCursor:
      case SCathegory::Update:
        return it->getParent();
      default:
        break;
    }
  return it;
}

void DynamicContextDetector::detectDynamicContext(ResolvedEntity *d, ResolvedEntity *ref) {
  static DynamicContextCases dynCases;

  SemanticTree *refSnode = nAssert(ref)->getSemanticNode();

  ResolvedEntity *def = d;
  {
    RefAbstract *t;
    if (ref && (t = ref->toSelfRefAbstract()))
      def = nAssert(t->reference->majorBaseEntity())->definition();

    if (SelectedField *f = def->toSelfSelectedField())
      if ((t = nAssert(f->expr())->toSelfRefAbstract()))
        def = nAssert(t->reference->majorBaseEntity())->definition();
  }
  SemanticTree *defLevel = nAssert(def)->getSemanticNode();
  defLevel = getFirstLevelNamespaceSnode(defLevel);

  ResolvedEntity::ScopedEntities cat = def->ddlCathegory();
  sAssert(!refSnode || !defLevel);

  detectHierarchyDynamicState(refSnode, defLevel);

  SemanticTree *it = refSnode;
  for (; it; it = it->getParent()) {
    SCathegory::t nodeCat = it->cathegory;
    if (nodeCat == SCathegory::Into && it->nametype == SemanticTree::EMPTY)
      it = getIntoOperator(defLevel, it);


    if (dynCases.needSignCases[cat][nodeCat][it->nametype](def, it))
      isSignCase = true;

    if (dynCases.needStaticCasting[cat][nodeCat][it->nametype](def, it))
      needStaticCasting = true;

    if (dynCases.isDynCtxCases[cat][nodeCat][it->nametype](def, it)) {
      isDynCtx   = true;
      isSignCase = true;
      if (needCastStaticQueryArg(refSnode))
        needStaticCasting = true;
      break;
    }

    if (it == defLevel) {
      if (needCastStaticQueryArg(refSnode))
        needStaticCasting = true;
      break;
    }
  }
  if (!it)
    throw 999;
  if (isDynCtx)
    needStaticCasting = false;

  return;
}



void translateAsMakestrArg(
    Codestream     &str  ,
    SemanticTree   *snode, /* семантический узел той конструкции, для которой выполняется подстановка */
    ResolvedEntity *d,     /*сущность, для которой выполняется подстановка, может быть определением   */
    ResolvedEntity *ref,
    bool isDirectContext)
{
  Datatype *t;
  if (!d || !(t = d->getDatatype())) {
    str << s::querySign;
    return;
  }

  bool refIsDynExpr = (ref && ref->toSelfDynExpr());

  Sm::sAssert(t->isCompositeType());
  if (!isDirectContext) { /* вне контекста динамического sql */
    if (needCastStaticQueryArg(snode))  /* допустимо статическое приведение (needCastStaticQueryArg) */
      str << s::querySign << s::obracket << s::lindef(Datatype::getLastConcreteDatatype(t)) << s::cbracket;
    else
      str << s::querySign; /* не допустимо статическое приведение */
  } // далее - все проверки выполняются для случая, когда установлен контекст динамического sql
  else if ((d->isFieldForMakestr() && !refIsDynExpr) ||
           (ref && ref->isFieldForMakestr() && !refIsDynExpr)) // выражение или определение (любого типа) - является полем для форматной строки makestr.
    str << s::querySign;
  else if (t->isVarcharDatatype()) // определение имеет строковый тип (VARCHAR, CHAR, RAW, LONG, ROWID, UROWID, LONG) - см. Sm::GlobalDatatype::StringDatatype и конструктор GlobalDatatype
    str << "'" << s::querySign << "'";
  else if (t->isDateDatatype()) // определение имеет тип, производный от даты (DATE, TIMESTAMP [WITH [LOCAL] TIMEZONE]) - см. определение FundamentalDatatype::isDateDatatype)
    str << "TO_DATE('" << s::querySign << "')";
  else // Все остальные случаи - неотрезолвлено либо сущность имеет тип, который нельзя кавычить.
    str << s::querySign;
}



bool translateIdEntitySpecial(Codestream& str, CodestreamState::NamesMode namesMode, ResolvedEntity *d, const IdEntitySmart& ent) {
  if (d) {
    DynamicContextDetector ctx;
    ResolvedEntity *ref = 0;
    if (Ptr<Id> x = ent.entity())
      if (x->beginedFrom(132267,29))
        cout << "";

    if (!str.state().isMakestrArglistTrStage_ &&
         str.state().queryForExecute_  && isExternalQueryParam(ent)) {
      if (Id *entId = ent.majorBaseEntity())
        if (SemanticTree *refSNode = entId->semanticNode())
          ref = refSNode->unnamedDdlEntity;

      ctx.detectDynamicContext(d, ref);
    }

    if (str.procMode() == CodestreamState::PROC && !ctx.isSignCase)
      if (d->ddlCathegory() == ResolvedEntity::FunctionArgument_ ||
          d->ddlCathegory() == ResolvedEntity::LinterCursorField_) {
        d->linterReference(str);
        return true;
      }

    ResolvedEntity *majDef = 0;
    Ptr<Id> major = ent.majorEntity();
    majDef = major->definition();
    if (ctx.isSignCase) {
      str.state().queryExternalIdentficators.push_back(new RefExpr(ent.entity()->getLLoc(), new IdEntitySmart(ent)));
      translateAsMakestrArg(str, ent.entity()->semanticNode(), d, ref, ctx.isDynCtx);
      return true;
    }
    if (majDef && ent.size() > 1 && majDef->ddlCathegory() == ResolvedEntity::Sequence_) {
      majDef->sqlReference(str);
      str << ".";
      d->sqlReference(str);
      return true;
    }
    if (!d->hasLinterEquivalent()) {
      if (ent.size() == 1 && d->ddlCathegory() == ResolvedEntity::Package_ && ent.front()->quoted()) {
        str << ent.front();
      }
      else if (ent.size() == 1 && d->ddlCathegory() == ResolvedEntity::Object_ && ent.front()->toNormalizedString() == "SELF")
        str << g_self;
      else {
        d->linterDatatypeReference(str);
        str.namesMode(namesMode);
      }
      return true;
    }
  }
  return false;
}

std::ostream& operator<< (std::ostream& os, const IdEntitySmart& str) {
  bool isNotFirst = false;
  for (IdEntitySmart::const_reverse_iterator rIt = str.rbegin(); rIt != str.rend(); ++rIt) {
    if (isNotFirst)
      os << ".";
    else
      isNotFirst = true;
    os << (*rIt)->toNormalizedString();
  }
  return os;
}

Codestream& translateNativeOracle(Codestream& str, const IdEntitySmart& entityRef, CodestreamState::NamesMode namesMode)
{
  static const auto trId = [](Codestream &str, const Ptr<Id> &id) {
    if (!id)
      return;
    str << id;
    if (id->callArglist)
      str << '(' << id->callArglist << ')';
    else if (ResolvedEntity *def = id->definition()) {
      if (def->toSelfFunction())
        str << "()";
    }
  };
  static const auto outputPointedComponent = [](Codestream &str, const Ptr<Id> &ent) {
    str << ".";
    trId(str, ent);
  };

  IdEntitySmart::const_reverse_iterator it = entityRef.rbegin();
  trId(str, *it);
  for (++it ; it != entityRef.rend(); ++it)
    outputPointedComponent(str, *it);
  str.namesMode(namesMode);
  return str;
}


void logTranslateReference(const IdEntitySmart& entityRef, Codestream& str, string reflog)
{
  if (!str.state().isLogger_) {
    Codestream logstr(str.state());
    logstr.state().isLogger_ = true;
    logstr.dbMode(CodestreamState::ORACLE);
    logstr << entityRef;
    logstr.dbMode(CodestreamState::LINTER);

    logstr << s::iloc(entityRef.getLLoc()) << " -> " << entityRef << s::endl;
    string s = logstr.str();
    OutputCodeFile::storeCode(reflog, s);
  }
}

void splitReferences(const IdEntitySmart& entityRef, Vector<IdEntitySmart> &refs) {
  Ptr<Id> lastCall;
  IdEntitySmart::const_iterator it, firstIt;
  it = firstIt = entityRef.begin();
  auto addRef = [&](IdEntitySmart::const_iterator begin, IdEntitySmart::const_iterator end) {
    if (lastCall) {
      if (lastCall->selfAlreadyPushed())
        return;
      lastCall->pushFrontCallarglist(new IdEntitySmart(begin, end), true);
      lastCall->setSelfAlreadyPushed();
    }
    else
      refs.push_back(new IdEntitySmart(begin, end));
  };

  for (; it != entityRef.end(); ++it) {
    ResolvedEntity *def = (*it)->definition();
    if (!def)
      continue;
    if (!def->isMethod() || def->isCollectionAccessor() || def->isConstructor())
      continue;

    addRef(firstIt, next(it));
    lastCall = (*it);
    firstIt = next(it);
  }

  if (firstIt != it) {
    addRef(firstIt, it);
  }
}

Codestream& translateSplittedRef(Codestream& str, const IdEntitySmart& entityRef, bool &isNotFirst, bool &userAlreadyOutput);

Codestream& operator<<(Codestream& str, const IdEntitySmart& entityRef) {
  if (entityRef.empty())
    return str;
  bool isNotFirst = false;
  CodestreamState::NamesMode namesMode = str.namesMode();
  if (str.dbMode() == CodestreamState::ORACLE)
    return translateNativeOracle(str, entityRef, namesMode);

  ResolvedEntity::ScopedEntities cat = ResolvedEntity::EMPTY__;
  bool userAlreadyOutput = false;

  if (ResolvedEntity *def = entityRef.definition()) {
    if (Sm::Function *f = def->toSelfFunction()) {
      if (f->flags.isAnydataMember()) {
        Ptr<Id> ent = entityRef.front();
        syntaxerContext.model->delayDeletedIds.push_back(ent.object());
        return translateIdReference(str, ent, isNotFirst, cat, userAlreadyOutput);
      }
      if (f->flags.isXmlFunction()) {
        userAlreadyOutput = true;
//        logTranslateReference(entityRef, str, OutputCodeFile::temporaryPath("reflog.xmltype.native.ora2lin"));
        return translateIdReference(str, entityRef.front(), isNotFirst, cat, userAlreadyOutput);
      }
    }

    if (def->isConstructor() && def->owner()->isXmlType()) {
      str << "XML_LOAD" << s::obracket << entityRef.front()->callArglist << s::cbracket;
      return str;
    }
  }


  Ptr<Id> majorEntity = entityRef.majorEntity();
  if (majorEntity && majorEntity->beginedFrom(117869,41))
    cout << "";

  if (translateIdEntitySpecial(str, namesMode, entityRef.definition(), entityRef))
    return str;

  if (Ptr<Sm::Id> idRef = entityRef.majorObjectRef(NULL)) {
    Sm::Type::RefDecoder decoder;
    if (decoder.decode(str, entityRef, NULL))
      return str;
  }

  Vector<IdEntitySmart> callRefs;
  splitReferences(entityRef, callRefs);
  if (callRefs.empty())
    throw 999;

  for (Ptr<IdEntitySmart> &ref : callRefs) {
    translateSplittedRef(str, *ref, isNotFirst, userAlreadyOutput);
  }

  str.namesMode(namesMode);
  return str;
}

Codestream& translateSplittedRef(Codestream& str, const IdEntitySmart& entityRef, bool &isNotFirst, bool &userAlreadyOutput) {
  ResolvedEntity::ScopedEntities cat = ResolvedEntity::EMPTY__;

  if (ResolvedEntity *def = entityRef.definition())
    if (def->toSelfMemberFunction() && !syntaxerContext.translateReferences) {
      UserContext *ctx = def->userContext();
      if (!ctx)
        throw 999;
      string uname = ctx->getName()->toNormalizedString();
      static const unordered_set<string> s = { "SYS", "XDB", "SYSTEM" };
      if (ctx->isSystem() || s.count(uname))
        throw 999;
      def->translateName(str);
      translateReferenceCallarglist(def, entityRef.entity(), str);
      return str;
    }

  bool isSelectedField = str.isSelectedField();
  IdEntitySmart::const_iterator it = entityRef.end();
  for (--it; it != entityRef.begin(); --it) {
    // если вначале идет имя означающее юзера - транслировать особым способом
    ResolvedEntity *entDef = (*it)->definition();
    if (!entDef) {
      cout << "unresolved entity " << (*it)->toNormalizedString() << " : " << (*it)->getLLoc() << endl;
      continue;
    }


    ResolvedEntity::ScopedEntities prevCat = ResolvedEntity::EMPTY__;
    ResolvedEntity *prevDef = (*prev(it))->definition();
    if (prevDef)
      prevCat = prevDef->ddlCathegory();
    ResolvedEntity::ScopedEntities entCat = entDef->ddlCathegory();

    bool prevIsField;
    switch (prevCat) {
      case ResolvedEntity::FieldOfRecord_:
      case ResolvedEntity::FieldOfTable_:
      case ResolvedEntity::QueriedPseudoField_:
      case ResolvedEntity::SqlSelectedField_:
      case ResolvedEntity::FieldOfVariable_:
        prevIsField = true;
        break;
      default:
        prevIsField = false;
    }

    if (entCat == ResolvedEntity::Synonym_) {
      cl::filelocation l1 = entityRef.back()->getLLoc();
      l1.loc = l1.loc + entityRef.front()->getLLoc().loc;
      cout << "error: synonyms in references must be unwrapped on resolving or refdecoder stage: "  << l1.locText() << endl;
    }

    if (entCat == ResolvedEntity::User_)  {
      if (prev(it) != entityRef.begin() && prevCat == ResolvedEntity::Package_)
        --it;
      else if (prevCat == ResolvedEntity::Table_ || prevCat == ResolvedEntity::View_ || prevCat == ResolvedEntity::Sequence_)
        str.isSelectedField(false);
      //else if (syntaxerContext.translateReferences && prevCat == ResolvedEntity::ArrayConstructor_)
      //  str << "NULL"; //?????
      continue;
    }
    else if (!syntaxerContext.translateReferences && entCat == ResolvedEntity::Variable_) {
      if (entDef->isPackageVariable() && prevIsField) {
        translateIdReference(str, *it--, isNotFirst, cat, userAlreadyOutput);
        str << "_" << *it;
        if (it != entityRef.begin())
          throw 999;
        return str;
      }
      else if (prevIsField) {
        translateIdReference(str, *it--, isNotFirst, cat, userAlreadyOutput);
        translateIdReference(str, *it, isNotFirst, cat, userAlreadyOutput);
        if (it != entityRef.begin()) {
          --it;
          str << "_" << *it;
          if (it != entityRef.begin())
            throw 999;
        }
        return str;
      }
      else
        translateIdReference(str, *it, isNotFirst, cat, userAlreadyOutput);
    }
    else if (entCat == ResolvedEntity::Function_ && prevCat == ResolvedEntity::Variable_)
      continue;
    else if (syntaxerContext.translateReferences && entCat == ResolvedEntity::MemberVariable_ && !isNotFirst) {
      isNotFirst = true;
      str << g_self;
      translateIdReference(str, *it, isNotFirst, cat, userAlreadyOutput);
    }
    else if (syntaxerContext.translateReferences && entCat == ResolvedEntity::Object_ && prevCat == ResolvedEntity::MemberVariable_) {
      isNotFirst = true;
      str << g_self;
    }
    else if ((entCat == ResolvedEntity::TriggerRowReference_ || entCat == ResolvedEntity::TriggerNestedRowReference_) &&
             entDef->toSelfAbstractRowReference()->linterMapReference(str, entityRef)) {
      str.isSelectedField(isSelectedField);
      return str;
    }
    else
      // Иначе - т.к. это не юзер - транслировать прямо данный кусок неединичного идентификатора
      translateIdReference(str, *it, isNotFirst, cat, userAlreadyOutput);
    str.isSelectedField(isSelectedField);
  };
  if (syntaxerContext.translateReferences && entityRef.size() == 1 &&
      (*it)->definition() && (*it)->definition()->ddlCathegory() == ResolvedEntity::MemberVariable_) {
    isNotFirst = true;
    str << g_self;
  }
  translateIdReference(str, *it, isNotFirst, cat, userAlreadyOutput);

  return str;
}

void translateExpandLValue(Sm::Codestream &str, Ptr<RefAbstract> expr, bool *isNotFirst) {
  if (expr->beginedFrom(117869,109))
    cout << "";
  Ptr<Datatype> datatype;
  if (!syntaxerContext.translateReferences) {
    datatype = expr->getDatatype();
    if (datatype)
      datatype= datatype->getFinalType();
  }
  if (datatype && (datatype->isCompositeType() && !datatype->isCollectionType())) {
    if (ResolvedEntity *def = expr->reference->definition()) {
      EntityFields typeFields;
      def->getFields(typeFields); // def - может быть переменной с клонированными полями, а а datatype - может быть таблицей с исходными
      for (Ptr<Id> fld : typeFields) {
        str << s::comma(isNotFirst) << expr << ((!syntaxerContext.translateReferences && def->isPackageVariable()) ? "_" : ".");
        str << fld->toCodeId(fld->definition());
        if (Ptr<Datatype> fldType = fld->getDatatype()->getFinalType())
          if (fldType->isRecordType() || fldType->isRowTypeOf())
            trError(str, s << "ERROR: row type in LValue field " << fld->getText());
      }
    }
  }
  else
    str << s::comma(isNotFirst) << expr;
}

template <typename RefExprList>
void translateLValueList(Codestream &str, RefExprList &inlist, ResolvedEntity */*statement*/) {
  bool isNotFirst = false;
  for (typename RefExprList::iterator it = inlist.begin(); it != inlist.end(); ++it) {
    Sm::RefAbstract *param = *it;
    ResolvedEntity *pObject;
    if (!param->reference->majorObjectRef(&pObject) || (param->refSize() > 1 && !param->reference->isDynamicUsing())) {
      translateExpandLValue(str, param, &isNotFirst);
      continue;
    }

    if (param->refSize() == 1 && param->refDefinition()->isVariable()) {
      if(pObject->isCollectionType()) {
        cout << "Bulk read into collection variable " << param->refEntity()->toNormalizedString() << std::endl;
        str << s::comma(&isNotFirst) << s::cref(param->refEntity()) << "(BULK !!!)";
      }
      else {
        str << s::comma(&isNotFirst) << param;
      }
      continue;
    }

    std::string declVarName;
    Ptr<Id> ent = param->refEntity();
    Ptr<Sm::Variable> var = param->getTemporaryVar(str, ent->getDatatype(), &declVarName, param->reference->isDynamicUsing());
    Ptr<Id> name = new Id(string(declVarName));
    name->definition(var.object());

    str.levelPush();
    str.activatePostactions();
    Ptr<Sm::LValue> lValue = new Sm::LValue(param->getLLoc(), new IdEntitySmart(*(param->reference)));
    Ptr<Assignment> assignment = new Assignment(param->getLLoc(), lValue, new RefExpr(name));
    str << s::tab() << assignment << s::procendl();
    str.activatePrevious();
    str.levelPop();

    str << s::comma(&isNotFirst) << declVarName;
  }
}


void translateQueryFields(Sm::Codestream &str, EntityFields &fields, bool expand, const string &parentStr/* = ""*/, unsigned int flags) {
  bool firstComma = false;
  for (EntityFields::iterator it = fields.begin(); it != fields.end(); ++it) {
    ResolvedEntity *def = (*it)->definition();
    if (expand && def)
      if (Ptr<Datatype> datatype = ResolvedEntity::getLastConcreteDatatype(def->getDatatype())) {
        if (datatype->isRecordType() || datatype->isRowTypeOf()) {
          EntityFields interFields;
          def->getDatatype()->getFinalType()->getFields(interFields);
          str << s::comma(&firstComma);

          string newParentStr = parentStr + (*it)->toNormalizedString() + "$";
          translateQueryFields(str, interFields, true, newParentStr, flags | (*it)->getAttributesValue() | FLAG_ID_HAS_DOLLAR_SYMBOL);
          continue;
        }
      }

    string name = parentStr;
    if ((*it)->empty() || (*it)->toNormalizedString()[0] == 0 || (*it)->toNormalizedString() == "COLUMN_VALUE")
      name += "VAL__";
    else
      name += (*it)->toNormalizedString();

    Ptr<Id> trName = new Id(string(name), 0, flags | (*it)->getAttributesValue());
    str << s::comma(&firstComma) << trName->toQInvariantNormalizedString(str.isProc());

//    if ((*it)->hasSpecSymbols() || (*it)->quoted()) {
//      string newName;
//      PlsqlHelper::quotingAndEscaping(newName, name);
//      str << s::comma(&firstComma) << newName;
//    }
//    else
//      str << s::comma(&firstComma) << name;
  }
}

void translateQueryFieldsEscape(Sm::Codestream &str, EntityFields &fields, bool expand) {
  string fstr;
  Codestream fStream;
  fStream.procMode(CodestreamState::SQL);
  translateQueryFields(fStream, fields, expand);
  PlsqlHelper::quotingAndEscapingSpecOnlyQuote(fstr, fStream.str(), '\"', '\\');
  str << fstr;
}

void translateQueryFieldsEscape(Sm::Codestream &str, Sm::ResolvedEntity *fromObject, bool expand) {
  EntityFields fields;
  if (expand)
    fromObject->getFieldsExp(fields, false);
  else
    fromObject->getFields(fields);
  translateQueryFieldsEscape(str, fields, false);
}

void translateCursorDataToString(Sm::Codestream &str, ResolvedEntity *cursor, EntityFields &cursorFields) {
  // Все поля уже должны быть раскрыты методом getFieldExp
  bool needComma;
  if (cursorFields.empty())
    cursor->getFieldsExp(cursorFields, true);
  str << "makestr" << s::obracket << s::oquote;
  needComma = false;
  for (EntityFields::iterator it = cursorFields.begin(); it != cursorFields.end(); ++it) {
    if (needComma)
      str << ',';
    needComma = true;
    bool isFieldForMakestr = false;
    if (ResolvedEntity *d = (*it)->definition())
      isFieldForMakestr = d->isFieldForMakestr();
    Ptr<Datatype> t = Datatype::getLastConcreteDatatype((*it)->getDatatype());
    if (t && (t->isVarcharDatatype() || t->isDateDatatype()) && !isFieldForMakestr)
      str << "'" << s::querySign << "'";
    else
      str << s::querySign;
  }

  str << s::cquote << s::comma();
  needComma = false;
  for (EntityFields::iterator it = cursorFields.begin(); it != cursorFields.end(); ++it) {
    str << s::comma(&needComma) << s::linref(cursor) << "." << (*it)->toCodeId((*it)->definition());
  }
  str << s::cbracket;
}

void translateCursorAsInsertCollection(Sm::Codestream &str, const IdEntitySmart &collEntity, ResolvedEntity *cursor, Ptr<PlExpr> indexExpr,
                                       int fieldIndex, string /*baseField*/, const EntityFields &cFields) {
  EntityFields cursorFields, collectFields;
  ResolvedEntity *pCollectionType;
  if (!collEntity.majorObjectRef(&pCollectionType))
    throw 999;

  if (cFields.size() == 0)
    pCollectionType->getFieldsExp(collectFields, false);
  else
    collectFields = cFields;
  if (cursor->eid() == 1122175)
    cout << "";
  cursor->getFieldsExp(cursorFields, true);
  if (fieldIndex >= 0 && int(cursorFields.size()) > fieldIndex) {
    // Для вставки в определенную коллекцию урезаем до одного значимого поля
    Ptr<Id> significant;
    significant = cursorFields[fieldIndex];
    cursorFields.clear();
    cursorFields.push_back(significant);
  }

  Ptr<Variable> objVar;

  if (collectFields.size() == 1 &&  cursorFields.size() > 0) {
    if (Ptr<Datatype> elDatatype = Datatype::getLastConcreteDatatype(pCollectionType->getElementDatatype()))
      if (Type::Object *obj = elDatatype->getNextDefinition()->toSelfObject()) {
        if (fieldIndex >= 0)
          throw 999; // Это надо реализовать

        EntityFields objFields;
        obj->getFields(objFields);
        string objVarName;
        if (objFields.size() != cursorFields.size() && cursorFields.size() == 1) {
          Ptr<Sm::Id> f = cursorFields.front();
          ResolvedEntity *fDef;
          Datatype *fType;
          ResolvedEntity *tidType;
          if ((fDef = f->definition()) && (fType = fDef->getDatatype()) && (tidType = fType->tidDdl()) && tidType->toSelfObject()) {
            cursorFields.clear();
            tidType->getFields(cursorFields);
          }
        }

        if (objFields.size() != cursorFields.size()) {
          // Неправильно изьяты поля объекта
          trError(str, s << "ERROR: collection object "
                         << obj->getLLoc() << " fields and cursor "
                         << cursor->getLLoc() << " fields has different size. Check cursors.");
        }

        objVar = cursor->getTemporaryVar(str, Datatype::mkNumber(19), &objVarName);
        str << "CALL" << s::name << s::linref(obj->getDefaultConstructor()) << s::obracket;
        str << obj->getMethodName(Type::ObjectType::BM_CTOR, 0, NULL);
        str << s::obracket << "\"" << obj->getObjectTempTable(obj) << "\"" << s::cbracket;

        for (Ptr<Id> &field : cursorFields) {
          str << s::comma() << s::linref(cursor) << "." << field->toCodeId(field->definition());
        }
        str << s::cbracket << s::name << "INTO" << s::name << objVarName;
        str << s::procendl() << s::tab();
      }
  }

  if (!objVar && collectFields.size() != cursorFields.size()) {
    //Неправильно изьяты поля курсоров
    trError(str, s << "ERROR: collection fields and cursor fields has different size. Check cursors.");
  }

  string insertMethod = pCollectionType->toSelfObjectType()->getMethodName(Type::ObjectType::BM_INSERT_SQL, 0, NULL);
  str << "CALL " << insertMethod << s::obracket;
  Sm::Type::RefDecoder().decode(str, collEntity);

  str << s::comma() << indexExpr << s::comma();
  translateQueryFieldsEscape(str, collectFields, false);
  str << s::comma();
  if (objVar)
    str << s::linref(objVar);
  else
    translateCursorDataToString(str, cursor, cursorFields);
  str << s::cbracket;
}

void translateCursorAsInsertCollection(Sm::Codestream &str, const IdEntitySmart &collEntity, ResolvedEntity *cursor,
                                       Sm::Variable *indexVar, int fieldIndex/* = -1*/) {
  Ptr<PlExpr> indexExpr(new Sm::RefExpr(indexVar->getName()));
  translateCursorAsInsertCollection(str, collEntity, cursor,  indexExpr, fieldIndex, "", EntityFields());
}


void translateCursorAsInsertCollection(Sm::Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity *cursor) {
  Type::ObjectType::ExtractColFields extrFields;
  if (!Type::ObjectType::extractCollectionAccessorFields(lEntity, extrFields) || !extrFields.indexExpr)
    throw 999;
  translateCursorAsInsertCollection(str, extrFields.newReference, cursor, extrFields.indexExpr, -1, extrFields.baseName, extrFields.opFields);
}

Codestream& operator<< (Codestream& os, SequenceBody *psb) {
  if (!psb)
    return os;
  SequenceBody &sb = *psb;
  if (sb.v.flags.startWith)
    os << " START WITH "    << sb.startWith;
  if (sb.v.flags.incrementBy)
    os << " INCREMENT BY " << sb.incrementBy;
  if (sb.v.flags.minvalue)
    os << " MINVALUE "     << sb.minvalue;
  else if (sb.v.flags.nominvalue)
    os << " NOMINVALUE";
  if (sb.v.flags.maxvalue)
    os << " MAXVALUE "     << sb.maxvalue;
  else if (sb.v.flags.nomaxvalue)
    os << " MAXVALUE";
  if (sb.v.flags.cycle)
    os << " CYCLE";
  else if (sb.v.flags.nocycle)
    os << " NO CYCLE";
  return os;
}

void Sequence::linterDefinition(Codestream& os) {
  os << s::ocreate(this) << "SEQUENCE " << name << s::name << data.object() << s::semicolon << s::ccreate;
}

void Sequence::linterReference(Codestream& os) { os << name; }

void Synonym::linterDefinition(Codestream &str) {
  if (isPublic)
    str << "PUBLIC" << s::name;
  str << "SYNONYM" << s::name << *name << s::name << "FOR" << s::name << *target
      << s::name << s::loc(getLLoc());
}

void Index::printIndexField(Codestream &str, Ptr<SqlExpr> field, bool &isNotFirst) {
  str << s::comma(&isNotFirst) << field;
}

string Index::translatedName() const {
  setTranslatedNameIfNot();
  return indexTranslatedName_->toNormalizedString();
}


void Index::setTranslatedNameIfNot() const {
  if (indexTranslatedName_)
    return;

  Ptr<LevelResolvedNamespace> sp = syntaxerContext.model->indicesNamespace;

  string trName = name->entity()->toNormalizedString();
  LevelResolvedNamespace::iterator it = sp->find(trName);

  if (it != syntaxerContext.model->indicesNamespace->end()) {
    if (it->second->compareWithFront(const_cast<Index*>(this)) <= 0) { // в начале соответствующего списка не стоит тот же элемент
      // в пространстве имен либо находится другой эл-т с таким именем, либо отсутствует
      trName = sp->getUniqueName(trName);
      LevelResolvedNamespace::iterator it = sp->find(trName);
      Sm::sAssert(it != sp->end()); // на случай, если неправильно работает код для получения уникальных идентификаторов.
      indexTranslatedName_ = new Id(string(trName), const_cast<Index*>(this));
      sp->addWithoutFind(indexTranslatedName_.object());
    }
  }
  else {
    indexTranslatedName_ = new Id(string(trName), const_cast<Index*>(this));
    sp->addWithoutFind(indexTranslatedName_.object());
  }
}

void Index::linterDefinition(Codestream &str) {
  if (ResolvedEntity *tbl = table->definition())
    if (syntaxerContext.model->modelActions.flags & MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE) {
      UserContext *tblUCtx = tbl->userContext();
      if (tblUCtx != userContext())
        str << s::connect(tblUCtx) << s::grant(tbl, Privs::INDEX, userContext());
      return;
    }

  Sm::sAssert(!name); // нужно реализовать кодогенерацию для неименованых индексов

  Ptr<Id> ent = name->entity();

  if (syntaxerContext.model->modelActions.dropIndices())
    str << s::ocmd(this, CmdCat::DROP_INDEX) << "DROP INDEX " << ent << " ON " << *table << s::semicolon << s::endl << s::ccmd;
  if (syntaxerContext.model->modelActions.createIndices() == 0)
    return;

  str << s::ocreate(this) << indexCathegory() << "INDEX ";

  setTranslatedNameIfNot();

  str << indexTranslatedName_->toQInvariantNormalizedString(str.isProc());
  str << s::loc(getLLoc())
      << s::name << s::otablevel(2)
      << s::subconstruct
      << "ON" << s::name << table->toInvariantQString(str.isProc()) << s::ctablevel();

  if (fieldList && fieldList->size()) {
    str << s::obracket << s::sql;
    bool isNotFirst = false;
    for (FieldList::iterator it = fieldList->begin(); it != fieldList->end(); ++it)
      printIndexField(str, *it, isNotFirst);
    str << s::cbracket;
  }

  str << s::semicolon
      << s::ccreate
      << s::endl;
}

namespace constraint {

void ConstraintState::linterDefinition(ostream &str) {
  if (flags & FLAG_CONSTRAINT_STATE_ENABLE)
    str << "ENABLE";
  if (flags & FLAG_CONSTRAINT_STATE_DISABLE)
    str << "DISABLE";
}

void ForeignReference::linterDefinition(Codestream &os) {
  if (foreignTable->empty())
    return;
  os << s::subconstruct;
  os << "REFERENCES" << s::name << *foreignTable;
  if (foreignFields->size())
    os << s::name << s::obracket << foreignFields << s::cbracket;
  switch (onDeleteAction) {
    case referenced_key::CASCADE:  os << " ON DELETE CASCADE" ; break;
    case referenced_key::SET_NULL: os << " ON DELETE SET NULL"; break;
    case referenced_key::EMPTY:    break;
  }
}

void PrimaryKey    ::linterDefinition(Codestream &os) { os << "PRIMARY KEY " << s::obracket << fieldListKey << s::cbracket; }
void Unique        ::linterDefinition(Codestream &os) {
  os << "UNIQUE" << s::name << s::otablevel(2) << s::subconstruct << s::obracket << fieldListKey << s::cbracket << s::ctablevel();
}
void CheckCondition::linterDefinition(Codestream &os) { os << "CHECK "       << s::obracket << condition    << s::cbracket; }


void ForeignKey    ::linterDefinition(Codestream &str) {
  str << "FOREIGN KEY "
      << s::otablevel(2)
      << s::obracket << fieldListKey << s::cbracket;
  if (!syntaxerContext.model->modelActions.dropForeignKeys())
    str << s::name << referencedKey;
  str << s::ctablevel();
}

void ForeignKey::linterDefinitionPrivilegie(Codestream& str) {
  (void)str;
  if (!Codestream::mainStream)
    return;
  Ptr<Id> field = fieldListKey->front();

  ResolvedEntity *fieldDef = field->definition();
  if (!fieldDef)
    return;
  auto getUcontext = [](ResolvedEntity *def) -> UserContext* {
    SemanticTree *n = def->getSemanticNode();
    while (n && (n->cathegory != SCathegory::User || n->nametype != SemanticTree::DEFINITION))
      n = n->getParent();
    UserContext *ucntx = 0;
    if (!n || !n->unnamedDdlEntity || !(ucntx = n->unnamedDdlEntity->toSelfUserContext()))
      throw 999;
    return ucntx;
  };

  UserContext *targetUserContext = getUcontext(fieldDef);
  string targetUser = targetUserContext->getName()->toQInvariantNormalizedString(false);
  if (targetUser.empty())
    throw 999;

  Table *sourceTable = 0;
  if (ResolvedEntity *d = referencedKey->entityDef())
    sourceTable = d->toSelfTable();

  if (!sourceTable)
    throw 999;
  UserContext *srcUser = getUcontext(sourceTable);
  if (targetUserContext != srcUser) {
    (*Codestream::mainStream) << s::connect(srcUser);
    (*Codestream::mainStream) << s::grant(sourceTable, Privs::REFERENCES, targetUser);
    (*Codestream::mainStream) << s::connect(targetUserContext);
  }
}

}

void FunctionArgument::definitionBase(Sm::Codestream &str) {
  if (!name)
    return;
  switch (direction) {
    case function_argument::IN:     str << "IN ";    break;
    case function_argument::OUT:    str << "OUT ";   break;
    case function_argument::IN_OUT: str << "INOUT "; break;
  }
  string strname = name->toCodeId(levelNamespace());
  str << strname << s::name;
}

void FunctionArgument::oracleDefinition(Sm::Codestream &str) {
  definitionBase(str);
  str  << s::def << datatype;
  if (defaultValue_)
    str << s::name << "DEFAULT " << s::def << defaultValue_;
  else
    str << s::stub;
}


bool constraintOutputIsAllowed(Ptr<Constraint> constr) {
  return constr && syntaxerContext.model->modelActions.flags & constr->attribute()->cathegoryToModelActionFlags();
}



void AlterTable::linterDefinition(Sm::Codestream &str) {
  auto getNameDbg = [&]() -> string {
    Codestream s(str.state());
    s << name;
    if (ResolvedEntity *def = name->definition())
      s << s::name << s::loc(def->getLLoc());
    return s.strWithoutEndl();
  };

  if (command->toSelfDropConstraint())
    return;
  AlterTableCommand::ConstraintsList l;
  if (command) {
    if (alter_table::AddConstraints* cmd = command->toSelfAddConstraints()) {
        if ((l = cmd->constraints)) {
          for (AlterTableCommand::ConstraintsList::dereferenced_type::value_type &it : *l) {
            if (constraintOutputIsAllowed(it)) {
              bool isDropCmd = syntaxerContext.model->modelActions.dropKeys();

              it->attribute()->linterDefinitionPrivilegie(str);

              Codestream s(str.state());
              s << (isDropCmd ? " DROP " : " ADD ") << it << s::loc(getLLoc());

              str << s::InlMessage(getNameDbg())
                  << s::InlMessage(s.strWithoutEndl())
                  << s::ocmd(name->definition(), CmdCat::ALTER_TABLE)
                  << "ALTER TABLE " << name << s::name
                  << (isDropCmd ? "DROP " : "ADD ") << it << s::semicolon << s::endl
                  << s::ccmd;
            }
          }
      }
    }
    else {
      if (alter_table::ManipulateFields *cmd = command->toSelfManipulateFields())
        if (List<alter_table::AlterFieldsBase> *l = cmd->manipulateFields)
          if (l->empty() || (l->size() == 1 && !l->front()))
            return;
      if (command->toSelfDropKey() && syntaxerContext.model->modelActions.dropKeys() == 0)
        return;
      throw 999;

      Codestream s(str.state());
      s << " " << command << s::loc(getLLoc());
      str << s::InlMessage(getNameDbg())
          << s::InlMessage(s.strWithoutEndl())
          << s::ocmd(name->definition(), CmdCat::ALTER_TABLE)
          << "ALTER TABLE " << name << s::name << command << s::semicolon << s::endl
          << s::ccmd;
    }
  }
}

uint16_t Datatype::truncNumberPrecision() const {
//  uint16_t prec = truncNumberPrecision();

//  uint16_t maxlen = 30 - prec;
//  return length > 30 ? maxlen : length;

  if (precision > 20 && (scaleIsNotSet() || scale == 0))
    return 20;

  return precision > 30 ? 30 : precision;
}

uint16_t Datatype::truncNumberScale() const {
  uint16_t truncPrec = truncNumberPrecision();
  uint16_t truncScale = (scaleIsNotSet() ? 0 : (scale > 10 ? 10 : scale));

  // При общей длине большей 20 - длина целая часть не может быть более 20.
  if (truncPrec > 20 && truncScale < (truncPrec-20))
    return truncPrec-20;

  return truncScale;
}

uint16_t Datatype::truncVarcharPrecision() const {
  return (precision > 4000) ? 4000 : precision;
}

uint16_t Datatype::truncNVarcharPrecision() const {
  return (precision > 2000) ? 2000 : precision;
}

void Datatype::lenprecDefinition(Sm::Codestream &str) {
  if (precision && !isDateDatatype()) {
    uint16_t newPrecision = precision;
    str << s::DisableIndenting() << s::obracket;
    if (isNumberDatatype())
      throw 999; // NUMBER должен обрабатываться отдельно
    else if (isCharVarchar())
      newPrecision = truncVarcharPrecision();
    else if (isNCharVarchar())
      newPrecision = truncNVarcharPrecision();
    str << newPrecision;
    if (scaleIsSet())
      str << s::comma() << scale;
    str << s::cbracket << s::EnableIndenting();

    if (newPrecision != precision) {
      str.activateSuffixes();
      str << s::OMultiLineComment() << s::name;
      str << tid << s::obracket << precision;
      if (scaleIsSet())
           str << s::comma() << scale;
      str << s::cbracket << s::CMultiLineComment() << s::name;
      str.activatePrevious();
    }
  }
  else if (isCharVarchar())
    str << s::DisableIndenting() << s::obracket << 4000 << s::cbracket << s::EnableIndenting();
  else if (isNCharVarchar())
    str << s::DisableIndenting() << s::obracket << 2000 << s::cbracket << s::EnableIndenting();
  else
    str << s::stub << s::stub << s::stub;
}

void Datatype::oracleDefinition(Sm::Codestream &str) {
  if (isNull()) {
    str << "null";
    return;
  }
  else if (isDefault()) {
    str << "default";
    return;
  }
  if (isRef())
    str << " REF ";
  linterDefinition(str);
//  str << tid;
//  lenprecDefinition(str);
//  if (isRowTypeOf())
//    str << "%ROWTYPE";
//  if (isTypeOf())
//    str << "%TYPE";
}

bool Datatype::tryTranslateToIntegralType(Sm::Codestream &str) {
  bool f = false;
  if ((f = translatedToSmallInt()))
    str << "SMALLINT";
  else if ((f = translatedToInt()))
    str << "INT";
  else if ((f = translatedToBigint()))
    str << "BIGINT";
  return f;
}

void Datatype::commentForTruncLength(Sm::Codestream &str) {
  bool precTrunc  = truncNumberPrecision() != precision;
  bool scaleTrunc = truncNumberScale() != scale;
  if (precTrunc || scaleTrunc) {
    str.activateSuffixes();
    str << s::name << s::OMultiLineComment();
    if (precTrunc)
       str << " TRUNC LEN " << precision << " -> " << truncNumberPrecision();
    if (scaleTrunc)
       str << " TRUNC PREC " << scale << " -> " << truncNumberScale();
    str << s::CMultiLineComment();
    str.activateActions();
  }
}

bool Datatype::numericOrDecimal3010(Sm::Codestream &str)
{
  bool f = false;
  if ((f = str.procMode() == CodestreamState::PROC))
    str << "NUMERIC";
  else if ((f = precision == 0))  // str.procMode() == CodestreamState::SQL)
    str << "DECIMAL"; // << s::obracket << 30 << s::comma << 10 << s::cbracket;
  return f;
}

void Datatype::linterDefinition(Sm::Codestream &str) {
  if (isRef())
    throw 999; // Ref - не поддерживаются, их поддержку надо реализовать, когда они встретятся;

  if (beginedFrom(800612))
    cout << "";

  ResolvedEntity *def = tidDef();
  if (isNumberDatatype()) {
    if (scaleIsEmpty()) {
      if (likeSmallInt())
        str << "SMALLINT";
      else if (likeInt())
        str << "INT";
      else if (likeBigint())
        str << "BIGINT";
      else // precision == 0
        numericOrDecimal3010(str);
    }
    else if (!numericOrDecimal3010(str)) {
      str << "DECIMAL" << s::obracket << truncNumberPrecision() << s::comma() << truncNumberScale() << s::cbracket;
      commentForTruncLength(str);
    }
  }
  else if (isTypeOf() && def && def->isField()) {
    if (def->containsInSystemContext() || str.isSql())
        str << tidDef()->getConcreteDatatype();
    else {
      ResolvedEntity *fieldOwner = 0;
      if (tidSize() > 1) {
        fieldOwner = (*tid)[1]->definition();
        if (fieldOwner && fieldOwner->toSelfPackage())
          fieldOwner = tidDef();
      }
      else
        fieldOwner = def->getFieldDDLContainer();
      if (!fieldOwner && (!def->toSelfVariable() || def->isPackageVariable()))
        throw 999;
      if (fieldOwner && (fieldOwner->isPackageVariable() ||
                         (syntaxerContext.requestEntities && !syntaxerContext.model->entityAlreadyCreatedInDb(fieldOwner))
                        )) {
        def->getConcreteDatatype()->translate(str);
        str.activateSuffixes();
        str << s::name << s::OMultiLineComment();
        str << "TYPEOF(" << tid << ")";
        str << s::CMultiLineComment() << s::name;
        str.activatePrevious();
      }
      else {
        str << "TYPEOF(" << tid << ")";

        str.activateSuffixes();
        str << s::name << s::OMultiLineComment();

        ResolvedEntity *def = tidDef()->getConcreteDatatype();
        def->translate(str);
        str << s::CMultiLineComment() << s::name;
        str.activatePrevious();
      }
    }
  }
  else if (isRowTypeOf())
    if (isLinterStructType() && !str.state().isDynamicCollection_)
      translateAsLinterStruct(tidDef(), str);
    else
      translateAsLinterCursor(tidDef(), str);
  else {
    if (ResolvedEntity *d = tidDef())
      if (d->toSelfSubtype()) {
        str << Ptr<Datatype>(SyntaxUnwrapper::unwrap(d));
        return;
      }
    str << tid;
    if (!isInt() && !isSmallint() && !isBigint() && !isDouble() && (!tidDef() || !tidDef()->toSelfSubtype()))
      lenprecDefinition(str);
  }
}

void Datatype::linterReference(Sm::Codestream &str) {
  if (ResolvedEntity *d = tidDef())
    d->linterReference(str);
}

void Datatype::sqlReference(Sm::Codestream &str) {
  if (ResolvedEntity *d = tidDef())
    d->sqlReference(str);
}

void Datatype::translateVariableType(Sm::Codestream &str, Sm::ResolvedEntity *var, bool addTypeName) {
  if (ResolvedEntity *d = tidDef())
     d->translateVariableType(str, var, addTypeName);
}

Sm::Function *Datatype::getDefaultConstructor() const {
  if (ResolvedEntity *d = tidDef())
    return d->getDefaultConstructor();
  return NULL;
}

Ptr<Datatype> Datatype::getFinalType() {
  if (finalType != NULL)
    return finalType;
  Ptr<Datatype> type = this, oldType = NULL;
  if (isEverything())
     return type;
  ResolvedEntity *def = 0;
  while (type != oldType && !type->isObjectType() &&
         (type->isTypeOf() ||
         (((def = type->tidDef())) && !def->toSelfFundamentalDatatype()))) {
    oldType = type;
    if ((def = type->getNextDefinition()))
      type = def->getDatatype();
    else
      return type;
  }
  finalType = type.object();
  return type;
}

void Subtype::linterReference(Sm::Codestream &str) {
  if (datatype)
    datatype->linterDefinition(str);
}

Sm::Function *Subtype::getDefaultConstructor() const {
  if (datatype)
    return datatype->getDefaultConstructor();
  return NULL;
}

void Variable::oracleDefinition(Sm::Codestream &str) {
  str << name << s::name << s::def << datatype;
  if (defaultValue_)
    str << " = " << s::def << defaultValue_;
}

void Table::diffOutput(Sm::Codestream &str) {
  str << name <<  s::name << s::loc(getLLoc()) << s::loc(othTable->getLLoc()) <<  s::endl;
  str.incIndentingLevel(2);
  for (DiffFields::iterator it = diffFields.begin(); it != diffFields.end(); ++it)
    if (it->first && !it->second)
      str << s::tab(2) << "[-] " << it->first << s::endl;
    else if (!it->first && it->second)
      str << s::tab(2) << "[+] " << it->second->getName() << s::name << it->second->getDatatype() << s::endl;
    else if (it->first && it->second)
      str << s::tab(1) << "[+-] " << it->first->getName() << " [-] " << it->first->getDatatype() << " [+] " << it->second->getDatatype() << s::endl;
  str.decIndentingLevel(2);
}


void Table::pythonDefinition(Sm::Codestream &str) {
  str << '(' << PlsqlHelper::quotingAndEscapingPython1(name->toNormalizedString()) << ',' << '[';
  if (relationFields)
    for (Ptr<table::FieldDefinition> &f : *relationFields)
      str << PlsqlHelper::quotingAndEscapingPython1(f->name->toNormalizedString()) << ',';
  str << ']' << ')';
}

void Table::linterDefinition(Codestream &str) {
  PushProcModeContext pushProcMode(str);
  (void)pushProcMode;
  str.procMode(CodestreamState::SQL);

  if (syntaxerContext.model->modelActions.dropTables()) {
    str.activatePrefixes();
    str << s::ocmd(this, CmdCat::DROP_TABLE) << "DROP TABLE " << s::linref(this) << " CASCADE;" << s::endl << s::ccmd;
    str.activatePrevious();
  }
  if (!syntaxerContext.model->modelActions.createTables())
    return;
  str << s::ocreate(this) << tableEntityCathegory() << "TABLE ";
  str << s::cref(this);
  str << s::loc(name->getLLoc()) << s::name;
  if (relationFields) {
    str << s::obracketArglist;
    if (relationFields->size()) {
      RelationFields::iterator it = relationFields->begin();
      str << *it;
      for (++it; it != relationFields->end(); ++it) {
        str << s::comma();
        str.joinSuffixes();
        str << *it;
      }
      str.joinSuffixes();
    }
    str << s::cbracketArglist;

    switch (onCommitAction) {
      case table::EMPTY: break;
      case table::ON_COMMIT_DELETE_ROWS  : str << " ON COMMIT DELETE ROWS"; break;
      case table::ON_COMMIT_PRESERVE_ROWS: str << " ON COMMIT PRESERVE ROWS"; break;
    }

    if (syntaxerContext.needTableRowProperties) {
      if (tableProperties)
        str << tableProperties;
      if (physicalProperties)
        str << physicalProperties;
      if (maxrow_ && maxrow_->getUIntValue())
        str << " MAXROW " << maxrow_ << " MAXROWID " << maxrow_;
    }
  }
  else // Объектная таблица. Нужно реализовать
    throw 0;
  str << s::ccreate;
}

struct LinterCheckTranslator {
  Ptr<PlExpr> check;

  LinterCheckTranslator(Ptr<PlExpr> _check) : check(nAssert(_check)) {}

  void linterDefinition(Codestream &str) {
    str << "CHECK" << s::name << s::otablevel(2) << s::subconstruct << check << s::ctablevel();
  }
};

template <typename T>
void tableConstraintsDefinition(Table *owner, Sm::Codestream &str, T& constr, bool isDelete = false) {
  if (isDelete)
    throw 999;

//  Sm::Codestream s1(str.state());
//  Sm::Codestream s2(str.state());
//  s1 << owner->name->toInvariantQString(str.isProc()) << " " << s::loc(getLLoc());
//  s2 << " ADD " << constr << s::loc(constr->getLLoc());
//  throw 999;
//  str << s::InlMessage(s1.strWithoutEndl())
//      << s::InlMessage(s2.strWithoutEndl());
  str << "ALTER TABLE " << s::cref(owner) << s::name << "ADD ";
  constr.linterDefinition(str);

  str << s::semicolon << s::endl;

}

void Table::linterDefinitionKeys(Codestream &str) {
  unsigned int v = syntaxerContext.model->modelActions.flags;
  if (v & MODEL_ACTIONS_DROP_KEYS && !(v & MODEL_ACTIONS_DROP_TABLES_FOREIGN_KEYS)) {
    if (primaryKey)
      tableConstraintsDefinition(this, str, *primaryKey, true);
    for (Ptr<PlExpr> v : checkConditions) {
      LinterCheckTranslator tr(v);
      tableConstraintsDefinition(this, str, tr, true);
    }
    for (UniqueKeyMap::value_type &v : uniqueKeys)
      tableConstraintsDefinition(this, str, *v.second, true);
  }

  if (v & MODEL_ACTIONS_DROP_TABLES_FOREIGN_KEYS)
    for (ForeignKeyMap::value_type &v : foreignKeys)
      tableConstraintsDefinition(this, str, *v.second, true);

  if (v & MODEL_ACTIONS_CREATE_TABLE_PRIMARY_KEYS)
    if (primaryKey)
      tableConstraintsDefinition(this, str, *primaryKey);

  if (v & MODEL_ACTIONS_CREATE_TABLE_UNIQUE_KEYS)
    for (UniqueKeyMap::value_type &v : uniqueKeys)
      tableConstraintsDefinition(this, str, *v.second);

  if (v & MODEL_ACTIONS_CREATE_TABLE_CHECKS)
    for (Ptr<PlExpr> v : checkConditions) {
      LinterCheckTranslator tr(v);
      tableConstraintsDefinition(this, str, tr);
    }

  if (v & MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS_GRANT_STAGE) {
    for (ForeignKeyMap::value_type &v : foreignKeys)
      v.second->linterDefinitionPrivilegie(str);
  }
  else if (v & MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS)
    for (ForeignKeyMap::value_type &v : foreignKeys)
      tableConstraintsDefinition(this, str, *v.second);


  sAssert(v & MODEL_ACTIONS_CREATE_TABLE_PRIMARY_FOREIGN_REFERENCES);
}


namespace table {

void FieldDefinition::oracleDefinition(Sm::Codestream &str) {
  str << name << s::name << s::def << datatype;
  if (defaultExpr)
    str << " = " << s::def << defaultExpr;
  if (!isNull)
    str << " IS NOT NULL";
}

void FieldDefinition::linterDefinition(Sm::Codestream &str) {
  linterReference(str);
  str << s::name << s::def << getDatatype();
  if (defaultExpr) {
    if (datatype->isSubtype(Datatype::mkVarchar2(), false))
      if (Ptr<Datatype> t = defaultExpr->getDatatype())
        if (t->isSubtype(Datatype::mkNumber(), false))
          defaultExpr->setStringType();
    if (!defaultExpr->isUnsupportedSysFuncall())
      str << " DEFAULT " << s::def << defaultExpr;
  }
  if (!isNull)
    str << " NOT NULL";
}

void FieldDefinition::translateNameIfSpecial() {
  if (!nameTranslated_) {
    if (name->isReservedField())
      name->setNeedIdToDQuoting();
    nameTranslated_ = true;
  }
}

void FieldDefinition::linterReference(Sm::Codestream &str) {
  if (str.isProc() && !procTranslatedName().empty())
    str << procTranslatedName();
  else
    str << name;
}

void field_property::PhysicalProperties::linterDefinition(Sm::Codestream &str) {
  if (pctfree)
    str << s::name << "PCTFREE " << pctfree;
  else if (pctused)
    str << s::name << "PCTUSED " << pctused;
}

void TableProperties::linterDefinition(Sm::Codestream &str) {
  if (fieldsProperties) {
    for (FieldsProperties::iterator it = fieldsProperties->begin(); it != fieldsProperties->end(); ) {
      if (!*it)
        it = fieldsProperties->erase(it);
      else {
        if ((*it)->hasLinterEquivalent())
          str << *it;
        ++it;
      }
    }
    if (fieldsProperties->empty())
      fieldsProperties = 0;
  }

  // TODO: enableDisableConstraints
  if (asSubquery) {
    str << s::name << "AS " << asSubquery;
    if (cachingState == table_properties::CACHE)
      str << " /*+ CACHE */";
    else if (cachingState == table_properties::NOCACHE)
      str << " /*+ NOCACHE */";
  }

//  if (enableDisableConstraints && owner_)
//    for (EnableDisableConstraints::iterator it = enableDisableConstraints->begin(); it != enableDisableConstraints->end(); ++it)
//      if ((*it) && (*it)->constraint && ((*it)->enableState == enable_disable_constraint::E_EMPTY || (*it)->enableState == enable_disable_constraint::ENABLE))
//        if ((*it)->constraint->hasLinterEquivalent())
//          owner_->linterAlterTableConstraints(str, (*it)->constraint);
}

}

void Sm::CursorProperties::linterDefinition(Sm::Codestream &str) {
  switch (property) {
    case cursor_properties::CURSOR_FOUND:
      str << "not outofcursor" << s::obracket << reference << s::cbracket;
      return;
    case cursor_properties::CURSOR_ISOPEN:
      str << "currow" << s::obracket << reference << s::cbracket << " <> 0";
      return;
    case cursor_properties::CURSOR_NOTFOUND:
      str << "outofcursor" << s::obracket << reference << s::cbracket;
      return;
    case cursor_properties::CURSOR_ROWCOUNT:
      str << "rowcount" << s::obracket << reference << s::cbracket;
      break;
  }
}

void Sm::CursorSqlProperties::linterDefinition(Sm::Codestream &str) {
  switch (property) {
    case cursor_properties::CURSOR_FOUND:
      str << "rowcount() > 0";
      break;
    case cursor_properties::CURSOR_ISOPEN:
      str << "currow() <> 0";
      break;
    case cursor_properties::CURSOR_NOTFOUND:
      str << "rowcount() = 0";
      break;
    case cursor_properties::CURSOR_ROWCOUNT:
      str << "rowcount()";
      break;
  }
}


void Sm::Close::linterDefinition(Sm::Codestream &str) {
  str << "CLOSE " << cursorEntity;
}

void Sm::PriorExpr::sqlDefinition(Codestream &str) { str << "PRIOR " << prior; }

void Sm::RefHostExpr::linterDefinition(Sm::Codestream &str) {
  if (ResolvedEntity *ent = refMajorEntity()->definition()) {
    ResolvedEntity::ScopedEntities cat = ent->ddlCathegory();
    if (cat == ResolvedEntity::TriggerRowReference_)
      str << reference;
    else
      throw 999;
  }
  else {
    cout << "error: unresolved RefHostExpr: " << getLLoc().locText() << endl;
    str << getLLoc().textFromFile();
  }
}

void SqlExpr::linterDefinition(Sm::Codestream &str) {
  static int x = 0;
  if (x > 1000) {
    cout << "error: infinite recursion in SqlExpr::linterDefinition:" << getLLoc().locText() << endl;
    return;
  }
  x++;
  sqlDefinition(str);
  --x;
}


bool Id::isFunction() const {
  return idDefinition_ && !(idDefinition_->isElementaryLinterFunction()) && idDefinition_->isFunction();
}

bool Id::isObject() const {
  if (idDefinition_)
    if (Ptr<Datatype> datatype = idDefinition_->getDatatype())
      return datatype->isObjectType();
  return false;
}

bool Id::isMethod() const {
  if (idDefinition_ && idDefinition_->isMethod())
    return true;
  return false;
}

void FunCallArg::oracleDefinition(Sm::Codestream &str) {
  if (argname())
    str << argname()->toQString() << " => ";
  if (argclass() == ASTERISK)
    str << "*";
  if (expr())
   str << s::def << expr();
}

void RefExpr::translateRef(Sm::Codestream &str)
{
  SemanticTree *tr;
  if ((tr = this->getSemanticNode())) // костыль для обхода обнуления в кастинге
    if (!tr->unnamedDdlEntity)
      tr->unnamedDdlEntity = this;

  str << (isNot() ? "not " : "") << s::def;
  str << reference;
  str.state().isCallOperator_ = false;
}

bool RefExpr::translateSplitCursorRef(Sm::Codestream &str, IdEntitySmart &ref) {
  //return false;
  if (ref.size() < 2 || !ref.definition() || ref.definition()->isFunArgument() || ref.definition()->isVariable())
    return false; // Игнорируем доступ к аргументу или переменной функции через ссылку Func.Arg, Func.LocalVar

  // Разделить функцию, так как Линтер не поддерживает ссылки вида Func().Field
  for (IdEntitySmart::iterator it = ++ref.begin(); it != ref.end(); ++it) {
    ResolvedEntity *def = (*it)->definition();
    if (!def ||
        !def->isFunction() ||
        def->isElementaryLinterFunction() ||
        def->isMethod() ||
        def->isCollectionAccessor())
      continue;

    if (str.isSql())
      throw 999; // Такое не должно встретиться в sql

    IdEntitySmart newEnt1(ref.begin(), it);
    IdEntitySmart newEnt2(it, ref.end());
    translateAsUserFunctionCall(str, newEnt2, isNot(), true);
    str << "." << newEnt1;
    return true;
  }
  return false;
}

void RefExpr::linterDefinitionBase(Sm::Codestream &str) {
  if (beginedFrom(23821,20))
    cout << "";
  if (str.procMode() == CodestreamState::SQL) {
    if (ResolvedEntity *d = refDefinition())
      if (d->isFunction())
        d->checkToGrantOtherUser(Privs::EXECUTE, str);
  }
  else if (str.isExpr()) {
    if (ResolvedEntity *d = syntaxerContext.model->inExternalVariablesStack(this))
      str << (isNot() ? "not " : "") << s::def << d->getName();
    if (translateAsUserFunctionCall(str, *reference, isNot()))
      return;
  }

  str.enterExpr();

  if (str.state().isCallOperator_)
    translateRef(str);
  else if (!translateAsUserFunctionCall(str, *reference, isNot())) {
    if (!translateSplitCursorRef(str, *reference))
      translateRef(str);
  }

  str.outExpr();
}


void RefExpr::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(132267,29))
    cout << "";
  linterDefinitionBase(str);
}

void RefExpr::sqlDefinition(Sm::Codestream &str) {
  if (ResolvedEntity *def = refDefinition()) {
    if (Sm::Variable *var = def->toSelfVariable())
      if (var->baseField)
        def = var->baseField;
    if (Sm::Cursor *c = def->toSelfCursor()) {
      Ptr<CallArgList> l = refEntity()->callArglist;
      if (l) {
        if (SemanticTree *n = getSemanticNode())  {
          if (findDynamicNode(n, str.state().dynamicSqlDeep_)) {
            n->setHasDynamicChilds();
            str.state().queryMakeStr_ = true;
          }
        }

        if (l->size() && l->front()->beginedFrom(1583244))
          cout << "";
        c->actualCursorParameters.insert(c->actualCursorParameters.end(), l->begin(), l->end());
      }
      str << c->select;
      c->actualCursorParameters.clear();
      return;
    }
  }
  linterDefinitionBase(str);
}

void AlgebraicCompound::oracleDefinition(Sm::Codestream &str) {
  str << s::def << lhs << s::name << toOperationString() << s::name << s::def << rhs;
}

std::string AlgebraicCompound::toOperationString() const {
  switch (op) {
    case algebraic_compound::MULTIPLE: return "*"  ;
    case algebraic_compound::DIVIDE:   return "\\" ;
    case algebraic_compound::PLUS:     return "+"  ;
    case algebraic_compound::MINUS:    return "-"  ;
    case algebraic_compound::CONCAT:   return "||" ;
    case algebraic_compound::DEGREE:   return "**" ;
    case algebraic_compound::MOD:      return "mod";
  }
  return "";

}

void AsteriskExpr::sqlDefinition(Sm::Codestream &str) { str << "*"; }

void AsteriskRefExpr::sqlDefinition(Sm::Codestream &str) { str << *reference << ".*"; }

std::string AlgebraicCompound::opToString() const {
  switch (op) {
    case algebraic_compound::MULTIPLE: return "*";
    case algebraic_compound::DIVIDE:   return "/";
    case algebraic_compound::PLUS:     return "+";
    case algebraic_compound::MINUS:    return "-";
    case algebraic_compound::CONCAT:   return "||";
    case algebraic_compound::DEGREE:   throw 999; // Возведение в степень не поддерживается. Нужно транслировать до вызова этой ф-ции.
    case algebraic_compound::MOD:      throw 999; // Остаток от деления не поддерживается. Нужно транслировать до вызова этой ф-ции.
  }
  return "";
}

std::string AlgebraicCompound::opToLogString() const {
  switch (op) {
    case algebraic_compound::MULTIPLE: return "*";
    case algebraic_compound::DIVIDE:   return "/";
    case algebraic_compound::PLUS:     return "+";
    case algebraic_compound::MINUS:    return "-";
    case algebraic_compound::CONCAT:   return "||";
    case algebraic_compound::DEGREE:   return "^"; // Возведение в степень не поддерживается. Нужно транслировать до вызова этой ф-ции.
    case algebraic_compound::MOD:      return "MOD"; // Остаток от деления не поддерживается. Нужно транслировать до вызова этой ф-ции.
  }
  return "";
}

void AlgebraicCompound::translateConcat(Codestream &str)
{
  string oper(str.isProc() ? "+" : "||");
  Ptr<Datatype> lhsT = Datatype::getLastConcreteDatatype(lhs->getDatatype());
  Ptr<Datatype> rhsT = Datatype::getLastConcreteDatatype(rhs->getDatatype());

  if (lhsT && rhsT) {
    if (lhsT->isCharVarchar() && !rhsT->isCharVarchar())
      str << lhs << s::name << oper << s::name << s::subconstruct << "TO_CHAR" << s::obracket << rhs->unwrapBrackets() << s::cbracket;
    else if (rhsT->isCharVarchar() && !lhsT->isCharVarchar())
      str << "TO_CHAR" << s::obracket << lhs->unwrapBrackets() << s::cbracket << s::name << oper << s::name << s::subconstruct << rhs;
    else
      str << lhs << s::name << oper << s::name << s::subconstruct << rhs;
  }
  else
    str << lhs << s::name << oper << s::name << s::subconstruct << rhs;
}

void AlgebraicCompound::translateSubs(Codestream &str) {
  Ptr<Datatype> lhsT = getLastConcreteDatatype(lhs->getDatatype());
  Ptr<Datatype> rhsT = getLastConcreteDatatype(rhs->getDatatype());

  if (lhsT && (lhsT->isDateDatatype() || lhsT->isIntervalDatatype()) &&
      rhsT && (rhsT->isDateDatatype() || rhsT->isIntervalDatatype())) {
    str << lhs << s::name << (str.isProc() ? "$" : "-") << s::name << rhs;
  }
  else
    str << lhs << s::name << opToString() << s::name << rhs;
}

void AlgebraicCompound::translateAdd(Codestream &str) {
  Ptr<Datatype> lhsT = getLastConcreteDatatype(lhs->getDatatype());
  Ptr<Datatype> rhsT = getLastConcreteDatatype(rhs->getDatatype());

  if (lhsT && lhsT->isNum() &&
      rhsT && rhsT->isDateDatatype()) {
    str << rhs << s::name << opToString() << s::name << s::subconstruct << lhs;
  }
  else
    str << lhs << s::name << opToString() << s::name << s::subconstruct << rhs;
}

void AlgebraicCompound::linterDefinition(Codestream &str) {
  switch (op) {
    case algebraic_compound::CONCAT:
      translateConcat(str);
      break;
    case algebraic_compound::DEGREE:
      str << lhs << s::name << "^" << s::name << rhs;
      break;
    case algebraic_compound::MOD:
      str << "MOD" << s::obracket << lhs << s::comma() << s::name << rhs << s::cbracket;
      break;
    case algebraic_compound::PLUS:
      translateAdd(str);
      break;
    case algebraic_compound::MINUS:
      translateSubs(str);
      break;
    default: // MULTIPLE DIVIDE PLUS
      str << lhs << s::name << opToString() << s::name << s::subconstruct << rhs;
      break;
  }
}

void AlgebraicCompound::sqlDefinition(Codestream &str) {
  switch (op) {
    case algebraic_compound::CONCAT:
      translateConcat(str);
      break;
    case algebraic_compound::DEGREE:
      str << "POWER" << s::obracket << lhs << s::comma() << s::name << rhs << s::cbracket;
      break;
    case algebraic_compound::MOD:
      str << "MOD" << s::obracket << lhs << s::comma() << s::name << rhs << s::cbracket;
      break;
    case algebraic_compound::PLUS:
      translateAdd(str);
      break;
    default:
      str << lhs << s::name << opToString() << s::name << s::subconstruct << rhs;
      break;
  }
}

void AnalyticFun::sqlDefinition(Codestream &str) {

  str << s::ocommalist();
  str << name << s::obracket << s::def << callarglist << s::cbracket << s::name;
  if (orderByClause) {
    str << s::subconstruct;
    orderByClause->sqlOverDefinition(str);
  }
  else
    str << "OVER ()";
  str << s::ccommalist();
}

void UnionQuery::sqlDefinitionOperation(Codestream &str)
{
  str << s::name << s::subconstruct;
  switch (op) {
    case sql_union::SIMPLE_UNION: str << "UNION";     break;
    case sql_union::UNION_ALL   : str << "UNION ALL"; break;
    case sql_union::INTERSECT   : str << "INTERSECT"; break;
    case sql_union::MINUS       : str << "EXCEPT";    break;
  }
  str << s::name << s::subconstruct;
}

void UnionQuery::sqlDefinition(Codestream &str) {
  str << lhs;
  sqlDefinitionOperation(str);
  str << rhs;
}

void UnionQuery::sqlDefinitionForNthField(Codestream &str, int fieldPos) {
  lhs->sqlDefinitionForNthField(str, fieldPos);
  sqlDefinitionOperation(str);
  rhs->sqlDefinitionForNthField(str, fieldPos);
}



void Constraint::linterDefinition(Codestream &os) { os << attribute_; }

namespace alter_table {

void ManipulateFields::linterDefinition(Codestream &str) {
  if (manipulateFields)
    for (ManipulateFieldsList::iterator it = manipulateFields->begin(); it != manipulateFields->end(); ++it)
      str << s::subconstruct << *it << s::name;
}

void DropFields::linterDefinition(Codestream &str) {
  str << fields;
  if (dropCascade)
    str << " CASCADE";
}

void AddFields::linterDefinition(Codestream &/*str*/) {
  throw 999;
//  str << "ADD COLUMN" << s::obracket << fields << s::cbracket;
}

void RenameTable::linterDefinition(Codestream &str) {
  str << "RENAME TO" << s::name << newName;
}

void RenameField::linterDefinition(Codestream &str) {
  str << "ALTER COLUMN " << oldName << s::name << "RENAME TO" << s::name << newName;
}

void AddConstraints::linterDefinition(Codestream &str) {
  if (constraints && constraints->size())
    for (List<Constraint>::iterator it = constraints->begin(); it != constraints->end(); ++it)
      str << s::subconstruct << "ADD " << *it << s::name;
}


void KeyFields::linterDefinition(Codestream &str) {
  if (primaryKey)
    str << "PRIMARY KEY";
  else if (uniqueFields)
    str << "UNIQUE" << s::obracket << uniqueFields << s::cbracket;
}

void DropKey::linterDefinition(Codestream &str) {
  if (key)
    str << "DROP" << s::name << key;
  if (isCascade)
    str << " CASCADE";
}

}

void pl_expr::Comparsion::sqlDefinition(Codestream &s) {
  Ptr<PlExpr> locRhs;
  size_t sz;

  if ((sz = rhs->size()) == 1) {
    locRhs = *rhs->begin();
    Ptr<Datatype> lType = SubqueryUnwrapper::unwrap(lhs->getDatatype().object());
    Ptr<Datatype> rType = SubqueryUnwrapper::unwrap(locRhs->getDatatype().object());
    if (lType && rType) {
      Ptr<PlExpr> blobNullComp;
      // При сравнении строки с числовым значением, преобразовать строку в число
      // TODO: этот код нужно переписать.
      if (lType->isCharVarchar() && locRhs->isNumericValue())
        lhs->setStringType();
      else if (rType->isCharVarchar() && lhs->isNumericValue())
        locRhs->setStringType();
      else if ((lType->isClobDatatype() || lType->isBlobDatatype()) && locRhs->isNull()) {
        blobNullComp = lhs;
      }
      else if (lhs->isNull() && (rType->isClobDatatype() || rType->isBlobDatatype())) {
        blobNullComp = locRhs;
      }

      if (blobNullComp) {
        s << "BLOB_SIZE" << s::obracket << blobNullComp << s::cbracket;
        s << s::name << (isNot() ? ComparsionOp::toInvertedString(op) : ComparsionOp::toString(op)) << s::name;
        s << 0;
        return;
      }
    }
  }

  if (lhs->toSelfRowNumExpr()) {
    Sm::CommonDatatypeCast::CastContext cntx;
    SemanticTree *n = getSemanticNode();
    if (!n)
      throw 999;
    n->getExpressionContext(cntx);
    string operatorName;
    if (cntx.updateStmt)
      operatorName = "UPDATE";
    else if (cntx.deleteStmt)
      operatorName = "DELETE";
    else
      throw 999;
    if (!operatorName.empty()) {
      string out = "error: LIMIT is not supported in " + operatorName + ": " + getLLoc().locText();
      s << s::OMultiLineComment() << " " << out << s::CMultiLineComment();
      cout << out << endl;
    }
  }

  s << lhs;
  s << s::name << (isNot() ? ComparsionOp::toInvertedString(op) : ComparsionOp::toString(op)) << s::name;
  if (quantor != QuantorOp::EMPTY)
    s << QuantorOp::toString(quantor) << s::name;

  if (sz > 1)
    s << s::obracket << rhs << s::cbracket;
  else if (op == ComparsionOp::IN && locRhs->isSubquery() &&
           locRhs->toSelfSubquery()->cathegorySubquery() != Subquery::BRACKETS)
    s << s::obracket << locRhs << s::cbracket;
  else
    s << locRhs;
}

void pl_expr::Comparsion::linterDefinition(Sm::Codestream &str) {
  if (op == ComparsionOp::IN) {
    bool firstOut = false;
    str << s::obracket;
    for (List<PlExpr>::iterator it = rhs->begin(); it != rhs->end(); ++it) {
      if (firstOut)
        str << s::name << "OR" << s::name << s::subconstruct;
      else
        firstOut = true;
      str << lhs << (isNot() ? " <> " : " = ") << *it;
    }
    str << s::cbracket;
  }
  else
    sqlDefinition(str);
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

void View::translateHead(Sm::Codestream &str) {
  str << s::name << s::loc(getLLoc()) << s::name;
  if (aliasedFields.size())
  {
    for (auto it = aliasedFields.begin(); it != aliasedFields.end(); ++it)
      (*it)->translateNameIfSpecial();
    str << s::OBracketView() << aliasedFields << s::CBracketView() << s::endl;
  }
  str << "AS" << s::name;

  if (syntaxerContext.model->modelActions.markedAsAutogenerated())
  {
    static std::string strGenTime = currentDateTime();
    str << s::endl << s::OMultiLineComment() << " Autogenerated at " << strGenTime << s::name << s::CMultiLineComment() << s::endl;
  }
}


void View::linterDefinition(Sm::Codestream &str) {
  syntaxerContext.viewsForTranslate++;
  str.procMode(CodestreamState::SQL);

  str << s::ocreate(this);
  if (isMaterialized)
    str << " MATERIALIZED ";

  str << "VIEW " << s::cref(this);
  translateHead(str);
  str << select << s::semicolon << s::endl
      << s::ccreate;

  str << s::grant(this, Privs::SELECT, syntaxerContext.model->modelActions.scenarioActorUsers);
}


void View::pythonDefinition(Sm::Codestream &str) {
  EntityFields f;
  getFields(f);
  str << '(' << PlsqlHelper::quotingAndEscapingPython1(name->toNormalizedString()) << ',' << '[';
  for (Ptr<Id> &i : f)
    str << PlsqlHelper::quotingAndEscapingPython1(i->toNormalizedString()) << ',';
  str << ']' << ')';
}



void SelectSingle::sqlDefinition(Codestream &str) {
  if (beginedFrom(94411))
    cout << "";

  auto checkLimitContext = [](SemanticTree *node) -> bool {
    for ( ; node != NULL; node = node->getParent()) {
      switch (node->cathegory) {
      case SCathegory::Function:
        return (node->entityDef()->isSystem() || node->entityDef()->isSystemTemplate());
      case SCathegory::Comparsion:
      case SCathegory::AlgebraicCompound:
      case SCathegory::UnionQuery:
        return true;
      case SCathegory::Exists:
      case SCathegory::View:
      case SCathegory::SelectSingle:
      case SCathegory::SelectStatement:
      case SCathegory::Update:
      case SCathegory::StatementDeleteFrom:
      case SCathegory::InsertInto:
      case SCathegory::SelectedField:
        return false;
      default:
        break;
      }
    }
    return false;
  };

  bool isSubqueryLimit = queryBlock->getLimitExpr() && !isRootQuery() && checkLimitContext(semanticNode->getParent());
  if (isSubqueryLimit)
    str << "SELECT * FROM" << s::subconstruct << s::tab(1) << s::obracket;

  str << queryBlock;

  if (isSubqueryLimit)
    str << s::cbracket;
}

void QueryBlock::setLimitExpr(Ptr<pl_expr::Comparsion> &cmp, int isNot) {
  if (limit)
    throw 999;

  if (cmp->rhs->front()->toSelfRowNumExpr())
    throw 999;

  if (isNot ^ cmp->isNotAsInt())
    cmp->toggleNot();
  if (isNot ^ cmp->isNotAsInt())
    throw 999;
  if (cmp->isNot())
    cmp->op = cmp->getNormalizedComparsionByNot();


  if (Ptr<SqlExpr> l = cmp->getRownumValue())
    limit = l;
  else
    throw 999;
}


void QueryBlock::sqlDefinitionHead(Codestream &str)
{
  if (factoringList && factoringList->size())
    str << "WITH" << s::name << s::ocommalist() << factoringList << s::name << s::ccommalist()
        << s::name << s::subconstruct;
//        << s::endl;
//        << s::StubLength(110) << s::name << s::subconstruct;

  str << "SELECT" << s::name;

  switch (queryPrefix) {
    case query_block::EMPTY:    break;
    case query_block::DISTINCT: str << "DISTINCT" << s::name; break;
    case query_block::UNIQUE:   str << "UNIQUE"   << s::name; break;
    case query_block::ALL:      str << "ALL"      << s::name; break;
    default: break;
  }

  str << s::ocommalist();

  if (SelectList::SelectedFields *f = selectList->selectedFields())
    if (f->size()) {
      SelectList::SelectedFields::iterator it = f->begin();
      if (*it && (*it)->expr()->distinct()) {
        if (queryPrefix != query_block::EMPTY)
          throw 999;
        str << "DISTINCT" << s::name;
        for (++it; it != f->end(); ++it)
          if ((*it) && (*it)->expr() && (*it)->expr()->distinct())
            throw 999; // DISTINCT в середине списка. Поддержку этого нужно реализовать
      }
    }
  str << s::ccommalist();

  if (!str.state().queryForExecute_)
    str << s::loc(getLLoc()); // << s::name;
}

void QueryBlock::sqlDefinitionFromWhereEtc(Codestream &str)
{
  if (from) {
    // TODO: учесть вариант с полем DUMMY
    if (from->size() == 1 && from->front()->isDual() &&
        tailSpec && tailSpec->hierarhicalSpec && tailSpec->hierarhicalSpec->isPseudoSequence())
      str << s::subconstruct << s::tab(2) << "FROM (SELECT 1)";
    else if (from->size() != 1 || !from->front()->isDual() || where || limit || orderBy || (tailSpec && tailSpec->groupBy))
      str << s::subconstruct << s::tab(2) << "FROM" << s::name << s::ocommalist() << from << s::ccommalist();
  }

  if (where) {
    if (DynWhere *dwh = where->toSelfDynWhere()) {
      str << s::name;
      dwh->sqlDefinition(str);
    }
    else
      str << s::subconstruct << s::tab(1) << "WHERE" << s::name << s::ocommalist() << where << s::ccommalist();
  }
  if (tailSpec && tailSpec->hierarhicalSpec)
    str << s::subconstruct << s::tab(1) << tailSpec->hierarhicalSpec;
  if (tailSpec && tailSpec->groupBy)
    str << s::subconstruct << s::tab(1) << s::otabcommalist() << "GROUP BY" << s::name
        << s::subconstruct << tailSpec->groupBy << s::ctabcommalist();
  if (tailSpec && tailSpec->having)
    str << s::subconstruct << s::tab(1) << s::otabcommalist() << "HAVING"   << s::name << tailSpec->having << s::ctabcommalist();

  if (orderBy)
    str << s::subconstruct << s::tab(1) << s::otabcommalist() << orderBy << s::ctabcommalist();
  if (limit)
    str << s::subconstruct << s::tab(1) << s::tab(1) << "LIMIT " << limit;
}

void QueryBlock::sqlDefinition(Codestream &str) {
  sqlDefinitionHead(str);

  if (selectList) {
    str << s::ocommalist();
    if (str.state().expandSelect_) {
      str.state().expandSelect_ = false;
      str << "$$$FIELDS" << s::comma();
      if (selectList->isAsterisk()) {
        EntityFields queryFields;
        getFields(queryFields);
        translateQueryFields(str, queryFields, false);
      }
      else
        str << selectList;
    }
    else
      str << selectList;
    str << s::ccommalist();
  }
  sqlDefinitionFromWhereEtc(str);
}

void QueryBlock::sqlDefinitionForNthField(Codestream &str, int fieldPos) {
  sqlDefinitionHead(str);
  if (selectList) {
    EntityFields queryFields;
    getFields(queryFields);
    EntityFields trFields;
    if ((int)(queryFields.size()) > fieldPos) {
      trFields.push_back(queryFields[fieldPos]);
      str << s::ocommalist();
      translateQueryFields(str, trFields, false);
      str << s::ccommalist();
    }
  }
  sqlDefinitionFromWhereEtc(str);
}

bool QueryBlock::isExactlyEquallyByDatatype(ResolvedEntity *t) {
  if (selectList && selectList->selectedFields()) {
    Ptr<SelectList::SelectedFields> fields = selectList->selectedFields();
    if (!fields->empty()) {
      if (selectList->isAsterisk() || fields->front()->expr_->isAsterisk()) {
        if (fields->size() > 1)
          throw 999;
        return t->eqByFields(this, false, true) != 0;
      }
      return t->eqByFields(this, false, true) != 0;
    }
  }
  return false;
}

bool HierarhicalClause::isPseudoSequence() const {
  const char *levelKwd = "LEVEL";
  if (!startWithCondition && connectCondition)
    if (pl_expr::Comparsion *comp = connectCondition->unwrapBrackets()->toSelfComparion()) {
      string lstr;
      comp->lhs->toNormalizedString(lstr);
      if (lstr == levelKwd)
        return true;
    }
  return false;
}

void HierarhicalClause::translateSeqCondition(Sm::Codestream &str) {
  if (pl_expr::Comparsion *comp = connectCondition->unwrapBrackets()->toSelfComparion()) {
    if (comp->op == ComparsionOp::LE) {
      // В линтере некорректно работает условие для псевдопоследовательностей
      // и выдает на одну запись больше.
      comp->op = ComparsionOp::LT;
      str << connectCondition;
      comp->op = ComparsionOp::LE;
      return;
    }
  }

  str << connectCondition;
}

void Sm::HierarhicalClause::linterDefinition (Sm::Codestream &str) {
  if (startWithCondition)
    str << "START WITH " << s::ocommalist() << startWithCondition << s::ccommalist() << s::name << s::subconstruct;
  str << "CONNECT BY ";
  if (isPseudoSequence())
    translateSeqCondition(str);
  else
    str << connectCondition;
}

void SelectList::sqlDefinition(Codestream &str) {
  if (isAsterisk_)
    str << "*";
  else if (fields) {
    bool isNotFirst;
    for (Ptr<SelectedField> &f : *fields) {
      str << s::comma(&isNotFirst) << s::ocolumn() << f << s::ccolumn();
    }
  }
}

UserContext* FromTableReference::userContext() const {
  if (ResolvedEntity *d = id->definition())
    return d->userContext();
  return NULL;
}

bool FromTableReference::haveFactoringItem() {
  if (ResolvedEntity *d = id->definition()) {
    if (d->ddlCathegory() == FactoringItem_)
      return true;
    if (FromTableReference *tRef = d->toSelfFromTableReference())
      if (tRef->id->definition()->ddlCathegory() == FactoringItem_)
        return true;
  }
  return false;
}

void FromTableReference::linterReference(Codestream &str) {
  if (id && id->size() == 1) {
    if (ResolvedEntity *d = id->entity()->unresolvedDefinition())
      if (d->toSelfVariable() || d->toSelfFunctionArgument())
        throw 999; // if (translateId2AsDynamicField(str, id, this)) return;  убрал, т.к. динамический sql транслируется теперь иначе

    if (ResolvedEntity *d = id->entity()->definition())
      if (!haveFactoringItem())
        if (UserContext *ctx = d->userContext()) {
          ctx->linterReference(str);
          str << '.';
        }
  }
  str << id;
}

void Sm::FromTableReference::sqlDefinition(Codestream &str) {
  if (beginedFrom(16908))
    cout << "";
  if (ResolvedEntity *def = id->definition())
    if (!haveFactoringItem())
      def->checkToGrantOtherUser(Privs::SELECT, str);
  linterReference(str);
}

void SelectedField::sqlDefinition(Codestream &str) {
  bool oldState = str.isSelectedField();
  str.isSelectedField(true);

  if (asteriksRef)
    str << asteriksRef << ".";
  str << expr_;
  if (alias_)
    str << s::name << "AS" << s::name << s::def << alias_;

  str.isSelectedField(oldState);
}

bool SelectedField::isExactlyEquallyByDatatype(ResolvedEntity *t) {
  Ptr<Datatype> dType;
  if (expr_ && (dType = expr_->getDatatype()).valid())
     return dType->isExactlyEqually(t->getDatatype());
  return false;
}

void FromSingle::linterReference(Codestream &str) {
  if (alias)
    str << alias;
  else
    str << reference;
}

void FromSingle::sqlDefinition(Codestream &str) {
  if (beginedFrom(443715))
    cout << "";
  str << reference;
  if (alias)
    str << s::name << "AS" << s::name << alias;
}

void FactoringItem::sqlDefinition(Codestream &str) {
  str << queryName;
  if (columnAliases)
    str << s::name << s::obracket << columnAliases << s::cbracket;
  str << s::name << "AS" << s::name
      << s::subconstruct;
  Subquery *q;
  if ((q = subquery->toSelfSubquery()) && q->cathegorySubquery() == Subquery::BRACKETS)
    str << subquery;
  else
    str << s::obracket << subquery << s::cbracket;
}

void OrderBy::oracleDefinition(Sm::Codestream &str) {
  if (siblings)
    str << "ORDER SIBLINGS BY ";
  else
    str << "ORDER BY ";
  str << s::def << orderList;
}

void OrderBy::sqlOverDefinition(Sm::Codestream &str) {
  str << "OVER (";
  if (partitionBy) {
    str << "PARTITION BY ";
    bool mostOne = partitionBy->size() > 1;
    if (mostOne)
      str << '(';
    str << s::def << partitionBy;
    if (mostOne)
      str << ')';
    if (orderList)
      str << ' ';
  }
  if (orderList)
    oracleDefinition(str);
  str << ')';
}

std::string OrderByItem::orderCathegoryString() const {
  switch (orderCathegory) {
    case EMPTY:            return "";
    case RAW_ASC:          return "ASC";
    case RAW_DESC:         return "DESC";
    case NULLS_FIRST:      return "NULLS FIRST";
    case NULLS_LAST:       return "NULLS LAST";
    case ASC:              return "ASC";
    case DESC:             return "DESC";
    case ASC_NULLS_FIRST:  return "ASC NULLS FIRST";
    case DESC_NULLS_LAST:  return "DESC NULLS LAST";
  }
  return "";
}
void OrderByItem::oracleDefinition(Sm::Codestream &str) {
  str << s::def << expr << s::name << orderCathegoryString();
}

void OrderByItem::sqlDefinition(Codestream &str) {
  str << s::def << expr << s::name << orderCathegoryString();
}

void OrderBy::sqlDefinition(Codestream &str) {
  str << (siblings ? "ORDER SIBLINGS BY" : "ORDER BY") << s::name << orderList;
}


void pl_expr::Exists::sqlDefinition(Codestream &str) {
  if (isNot())
    str << "NOT" << s::name;
  str << "EXISTS" << s::name << s::obracket << query << s::cbracket;
}

std::string Sm::Join::toJoinOpSting() const {
  switch (op) {
    case JoinQueries::EMPTY:         return "INNER JOIN";
    case JoinQueries::OUTHER:        return "";
    case JoinQueries::INNER:         return "INNER JOIN";
    case JoinQueries::CROSS:         return "CROSS JOIN";
    case JoinQueries::RIGHT:         return "RIGHT JOIN";
    case JoinQueries::LEFT:          return "LEFT JOIN";
    case JoinQueries::FULL:          return "FULL JOIN";
    case JoinQueries::NATURAL:       return "";
    case JoinQueries::NATURAL_INNER: return "NATURAL INNER JOIN";
    case JoinQueries::NATURAL_CROSS: return "CROSS JOIN";
    case JoinQueries::NATURAL_FULL:  return "NATURAL FULL JOIN";
    case JoinQueries::NATURAL_RIGHT: return "NATURAL RIGHT JOIN";
    case JoinQueries::NATURAL_LEFT:  return "NATURAL LEFT JOIN";
  }
  return "";
}

void FromJoin::sqlDefinition(Codestream &str) {
  str << lhs << s::name << op->toJoinOpSting() << s::name
      << s::otablevel(2) << s::subconstruct << rhs << op << s::ctablevel();
}

void CaseIfThen::sqlDefinition(Codestream &str) {
  str << s::subconstruct
      << "WHEN" << s::name << s::ocommalist() << condition << s::ccommalist() << s::subconstruct << s::name
      << "THEN" << s::name << action << s::name;
}

Sm::Codestream &operator<<(Codestream& str, Ptr<List<CaseIfThen>> cases) {
  if (!cases || cases->empty())
    return str;
  for (List<CaseIfThen>::iterator it = cases->begin(); it != cases->end(); ++it)
    str << Def() << (*it);
  return str;
}

void Case::sqlDefinition(Codestream &str) {
  str << s::ocommalist() << s::otablevel(2) << "CASE" << s::name;
  if (source)
    str << s::ocommalist() << source << s::ccommalist() << s::name;

  str << cases;
  if (elseClause)
    str << s::subconstruct << "ELSE" << s::name << s::ocommalist() << elseClause << s::ccommalist();
  str << s::ctablevel() << s::name << s::subconstruct << "END" << s::ccommalist();
}

void Case::outCaseSourceClause(Codestream &str, List<CaseIfThen>::iterator it) {
  auto outEif = [this, &it, &str]() -> void {
      str << "EIF" << s::name << '[';
      if (source)
        str << source << " = ";
      str << (*it)->condition << ']' << s::name
          << (*it)->action;
  };

  if (it == cases->begin()) {
    outEif();
    outCaseSourceClause(str, ++it);
  }
  else if (it == cases->end()) {
    if (elseClause)
      str << s::subconstruct << s::name << "ELSE" << s::name << s::obracket << elseClause << s::cbracket;
  }
  else {
    str << s::subconstruct << s::name << "ELSE" << s::name
        << s::OBracketCall();
    outEif();
    outCaseSourceClause(str, ++it);
    str << s::CBracketCall();
  }
}

void Case::linterDefinition(Codestream &str) {
//  str << s::otabcommalist();
  outCaseSourceClause(str, cases->begin());
//  str << s::ctabcommalist();
}

void FromTableDynamic::sqlDefinition(Codestream &str) {
  if (beginedFrom(506846))
    cout << "";
  if (hasLJoin)
    throw 999; // поддержку этого нужно формализовать
  if (dynamicTable) {
      Ptr<CallArgList> arglist;
      if (ResolvedEntity *c = dynamicTable->getCursorFuncall()) {
        if ((arglist = c->callArglist()))
          for (CallArgList::iterator it = arglist->begin(); it != arglist->end(); ++it)
            if ((*it)->expr()->isCursor())
              return; // TODO: данную ситуацию нужно доработать в контекстном анализаторе. См. пометки в vim
        Ptr<Datatype> datatype = c->getDatatype()->getFinalType();
        if (datatype->isObjectType()) {
          Type::CollectionType *coll = datatype->getNextDefinition()->toSelfCollectionType();
          Ptr<Datatype> nestedType = coll->getElementDatatype()->getFinalType();
          Type::Object *nestedObj = nestedType->getNextDefinition()->toSelfObject();
          string objTable = nestedObj ? nestedObj->getObjectTempTable(nestedObj) : string();
          if (!str.state().queryForExecute_) {
            string colTable = coll->getObjectTempTable(c);
            if (nestedObj) {
              str << s::obracket << "SELECT * FROM " << colTable;
              str << " C JOIN " << objTable
                  << " O ON C.OBJREFID__ = SYS.CI_GET_OBJREFID__" << s::obracket;
              c->linterDefinition(str);
              str << s::cbracket;
              str << " AND GETLONG(C.VAL__, 0) = O.ID__";
              str << s::cbracket;
            }
            else {
              str << s::obracket << "SELECT * FROM " << colTable;
              str << " WHERE OBJREFID__ = SYS.CI_GET_OBJREFID__" << s::obracket;
              c->linterDefinition(str);
              str << s::cbracket;
              str << s::cbracket;
            }
          }
          else {
            if(!str.state().parentStream_)
               throw 999;

            if (nestedObj)
              str << s::obracket << "SELECT O.* FROM " << s::querySign << " C JOIN " << objTable
                  << " O ON C.OBJREFID__ = " << s::querySign << " AND GETLONG(C.VAL__, 0) = O.ID__" << s::cbracket;
            else
              str << s::obracket << "SELECT * FROM " << s::querySign << " WHERE OBJREFID__ = " << s::querySign << s::cbracket;

            Ptr<RefExpr> expr = new RefExpr(c->getName());
            if (expr->isFuncall()) {
              Codestream &parentStr = *str.state().parentStream_;
              string tmpVarName;
              Ptr<Variable> tmpVar = c->getTemporaryVar(parentStr, c->getDatatype(), &tmpVarName, true);

              parentStr.activatePrefixes();
              parentStr << s::tab() << s::otabcommalist()
                        << "CALL " << expr << s::name << s::subconstruct << "INTO" << s::name << tmpVarName
                        << s::ctabcommalist() << s::procendl(getLLoc());
              parentStr.activateActions();

              expr = new RefExpr(tmpVar->getName());
            }

            Ptr<Id> f1 = new Id("SYS.CI_GET_TABNAME__");
            f1->loc(getLLoc());
            f1->callArglist = new CallArgList({new FunCallArgExpr(getLLoc(), expr)});
            str.state().queryExternalIdentficators.push_back(new RefExpr(f1->getLLoc(), new IdEntitySmart(f1)));

            Ptr<Id> f2 = new Id("SYS.CI_GET_OBJREFID__");
            f2->loc(getLLoc());
            f2->callArglist = new CallArgList({new FunCallArgExpr(getLLoc(), expr)});
            str.state().queryExternalIdentficators.push_back(new RefExpr(f2->getLLoc(), new IdEntitySmart(f2)));
          }
        }
        else {
          str << s::obracket << "SELECT * FROM" << s::name;
          c->linterDefinition(str);
          str << s::cbracket;
        }
      }
      else
        throw 999; // TODO: эту ситуацию нужно реализовать
  }
}

void pl_expr::Between::sqlDefinition(Codestream &str) {
  str << value << s::name << (isNot() ? "NOT BETWEEN" : "BETWEEN")
      << s::name << s::otablevel(2) << s::subconstruct  << low << s::name << "AND" << s::name << s::subconstruct << high << s::ctablevel();
}

void pl_expr::Between::linterDefinition(Codestream &str) {
  str << s::obracket;
  if (isNot())
    str << low << " > " << value << " OR " << value << " > " << high;
  else
    str << low << " <= " << value << " AND " << value << " <= " << high;
  str << s::cbracket;
}

void pl_expr::Like::sqlDefinition(Codestream &str) {
  if (likeCathegory != like_cathegory::SIMPLE_LIKE)
    throw 999; // необходимо реализовать поддержку
  if (str.isProc()) {
    string tmpVarName;
    Ptr<Variable> var = char1->getTemporaryVar(str, Datatype::mkBoolean(), &tmpVarName);

    str.levelPush();
    str.activatePrefixes();
    str << s::tab() << s::executeStatementPointer(this, str.state()) << "EXECUTE \"";
    str.procMode(CodestreamState::SQL);
    str << "SELECT " << s::querySign <<  s::name << (isNot() ? "NOT LIKE" : "LIKE") << s::name << char2;
    if (esc_char)
      str << s::name << "ESCAPE" << s::name << esc_char;
    str.procMode(CodestreamState::PROC);
    str << ";\"" << s::comma() << char1 << " INTO " << tmpVarName << s::procendl();
    str.activatePrevious();
    str.levelPop();

    str << tmpVarName;
  }
  else {
    str << char1 << s::name << (isNot() ? "NOT LIKE" : "LIKE") << s::name << char2;
    if (esc_char)
      str << s::name << "ESCAPE" << s::name << esc_char;
  }
}

void Cast::sqlDefinition(Sm::Codestream &str) {
  str << s::ocommalist() << "CAST" << s::obracket << castedExpr << s::name << s::subconstruct << "AS" << s::name << toType << s::cbracket
      << s::ccommalist();
}

std::string ExtractExpr::toStringEntity() const {
  switch (extractedEntity_) {
    case ExtractedEntity::DAY:             return "DAY";
    case ExtractedEntity::YEAR:            return "YEAR";
    case ExtractedEntity::MONTH:           return "MONTH";
    case ExtractedEntity::SECOND:          return "SECOND";
    case ExtractedEntity::HOUR:            return "HOUR";
    case ExtractedEntity::MINUTE:          return "MINUTE";
    case ExtractedEntity::TIMEZONE_HOUR:   throw 999;
    case ExtractedEntity::TIMEZONE_MINUTE: throw 999;
    case ExtractedEntity::TIMEZONE_REGION: throw 999;
    case ExtractedEntity::TIMEZONE_ABBR:   throw 999;
  }
  return "";
}

void ExtractExpr::sqlDefinition(Codestream &str) {
  str << toStringEntity() << s::name << "FROM" << s::name << fromEntity;
}

std::string TrimFromExpr::cathegoryToString() const {
  switch (extractedEntity_) {
    case LEADING:  return "LEADING";
    case TRAILING: return "TRAILING";
    case BOTH:     return "BOTH";
  }
  return "";
}

void TrimFromExpr::sqlDefinition(Codestream &str) {
  str << "TRIM" << s::name << s::obracket << cathegoryToString() << s::name << charExpr << s::name << "FROM" << s::name << fromEntity << s::cbracket;
}

void Sm::Package::linterDeclaration(Codestream &str) {
  std::set<ResolvedEntity*, LE_ResolvedEntities> outputted;
  for (Container::iterator it = heads.begin(); it != heads.end(); ++it)
    (*it)->translateDeclarations(str, filter_, outputted, true);
}

void Sm::Package::linterDefinition(Codestream &str) {
  std::set<ResolvedEntity*, LE_ResolvedEntities> outputted;
  for (Container::iterator it = bodies.begin(); it != bodies.end(); ++it)
    (*it)->translateDeclarations(str, filter_, outputted, false);
  for (Container::iterator it = heads.begin(); it != heads.end(); ++it)
    (*it)->translateDeclarations(str, filter_, outputted, false);
}

void Sm::BlockPlSql::addReturn(Ptr<Id> name) {
  statements.push_back(new Sm::Return(cl::emptyFLocation(), new RefExpr(name)));
}


void outputGranteeClausesForUser(Sm::Codestream::SortedGranteesClauses::iterator uIt, UserContext *curUcntx, Codestream &str, bool toScenarioActor)
{
  for (Sm::Codestream::SortedGranteesClauses::mapped_type::iterator entIt = uIt->second.begin(); entIt != uIt->second.end(); ++entIt)
    for (Sm::Codestream::SortedGranteesClauses::mapped_type::mapped_type::iterator privIt = entIt->second.begin(); privIt != entIt->second.end(); ++privIt) {
      if (toScenarioActor)
        str << s::grant(entIt->first, *privIt, syntaxerContext.model->modelActions.scenarioActorUsers);
      else
        str << s::grant(entIt->first, *privIt, curUcntx->getName()->toQString());
    }
}

void outputSortedGranteeClause(Sm::Codestream::SortedGranteesClauses &sortedGranteesClauses, Codestream &str, UserContext *curUcntx, bool needGranteeToCurrentActor) {
  for (Sm::Codestream::SortedGranteesClauses::iterator uIt = sortedGranteesClauses.begin(); uIt != sortedGranteesClauses.end(); ++uIt)
    if (uIt->first != curUcntx) {
      str << s::connect(uIt->first);
      outputGranteeClausesForUser(uIt, curUcntx, str, needGranteeToCurrentActor);
    }
  str << s::connect(curUcntx);
}

void needToGranteeToSortedGrantee(Codestream::SortedGranteesClauses &sortedGranteesClauses, Sm::Codestream::NeedToGrantee &needToGrantee) {
  for (Sm::Codestream::NeedToGrantee::iterator it = needToGrantee.begin(); it != needToGrantee.end(); ++it)
    for (Sm::Codestream::NeedToGrantee::mapped_type::iterator uit = it->second.begin(); uit != it->second.end(); ++uit)
      for (Sm::Codestream::NeedToGrantee::mapped_type::mapped_type::iterator sit = uit->second.begin(); sit != uit->second.end(); ++sit)
        sortedGranteesClauses[/*user*/ uit->first][/*entity*/ *sit].insert(it->first);
}

void outputGranteeClause(Sm::Codestream::NeedToGrantee &needToGrantee, Codestream &str, UserContext *curUcntx, bool needGranteeToCurrentActor) {
  Sm::Codestream::SortedGranteesClauses sortedGranteesClauses;
  needToGranteeToSortedGrantee(sortedGranteesClauses, needToGrantee);
  outputSortedGranteeClause(sortedGranteesClauses, str, curUcntx, needGranteeToCurrentActor);
}

void translateGranteeClausesForPlSqlBlock(Codestream &str) {
  UserContext *curUcntx = str.state().currentUser_;
  str.activatePreactions();
  // TODO: вывести команды подключения к пользователям, упорядоченные по пользователям
  // Для этого заменить вектор сущностей на мэп сущностей по пользователям
  if (curUcntx) {
    if (str.needToGrantee.size())
      str << s::endl;
    Sm::outputGranteeClause(str.needToGrantee, str, curUcntx, true);
  }
  str.activatePrevious();
  str.needToGrantee.clear();
}


void setTranslateSqlToStringFlags(Codestream &dst, Codestream &src, unsigned int flags) {
  dst.state().indentingLongConstruct_ = src.state().indentingLongConstruct_;
  dst.state().currentUser_ = src.state().currentUser_;
  dst.dbMode(Sm::CodestreamState::LINTER);
  dst.procMode(Sm::CodestreamState::SQL);
  dst.incIndentingLevel(src.indentingLevel());
  dst.state().queryForExecute_ = true;
  dst.state().dynamicSqlDeep_  = src.state().dynamicSqlDeep_ + 1;
  dst.state().queryMakeStr_ = flags & SQLSTR_NEED_DIRECT;
  dst.state().expandSelect_ = flags & SQLSTR_EXPAND_SELECT;
  dst.state().executeBlock_ = flags & SQLSTR_IS_EXECUTE_BLOCK;
  dst.state().parentStream_ = &src;
}

void translateExternalId(Codestream &dst, CodestreamState::QueryExternalIdentficators::value_type &it)
{
  Ptr<PlExpr> p = it;
  if (RefExpr *expr = p->toSelfRefExpr())
    Sm::sAssert(syntaxerContext.model->inExternalVariablesStack(expr->refDefinition()));
  dst << *it;
}

void wrapSqlCodestreamIntoString(TranslatorTailMakestr *translator, Codestream &dst, Codestream &sqlExprStr, unsigned int flags) {
  for (auto &it : sqlExprStr.needToGrantee)
    for (auto &uit : it.second)
      for (auto &eit : uit.second)
        dst.needToGrantee[/*privilegie*/ it.first][/*user*/ uit.first].insert(/*entity*/ eit);

  string query;
  sqlExprStr.deleteLocations();
  PlsqlHelper::quotingAndEscapingSpecOnlyQuote(query, sqlExprStr.str(), '\"', '\\');

  CodestreamState &state = sqlExprStr.state();
  bool needDirect = !(flags & SQLSTR_SUPRESS_DIRECT) &&
                     ((flags & SQLSTR_NEED_DIRECT) || state.queryMakeStr_);
  if (needDirect)
    dst << "DIRECT" << s::name;
  if (!(flags & SQLSTR_HAS_ADDITIONAL_IDENTIFICATORS) && state.queryExternalIdentficators.empty()) {
    translateMultilineText(dst, query);
  }
  else {
    if (needDirect || (flags & SQLSTR_NEED_MAKESTR))
      dst << "makestr(";
    translateMultilineText(dst, query);

    if (translator)
      translator->translateTail(dst);

    bool old = dst.state().isMakestrArglistTrStage_;
    dst.state().isMakestrArglistTrStage_ = true;

    CodestreamState::QueryExternalIdentficators &externalIds = state.queryExternalIdentficators;
    if (!externalIds.empty()) {
      dst << s::CommaMakestr() << s::ocommalist();
      CodestreamState::QueryExternalIdentficators::iterator it = externalIds.begin();
      translateExternalId(dst, *it);

      for (++it; it != externalIds.end(); ++it) {
        dst << s::CommaCodegen();
        translateExternalId(dst, *it);
      }
      dst << s::ccommalist();
    }
    if (needDirect || (flags & SQLSTR_NEED_MAKESTR))
      dst << ")";

    dst.state().isMakestrArglistTrStage_ = old;
  }
}


bool findDynamicNodeStaticCtx(SemanticTree *n) {
  switch (n->cathegory) {
    case SCathegory::FromTableDynamic:
    case SCathegory::FunctionDynField:
    case SCathegory::FunctionDynTail_:
    case SCathegory::QueryEntityDyn:
    case SCathegory::DynamicFuncallTranslator:
      return true;
    default:
      break;
  }

  for (Sm::SemanticTree::Childs::value_type &c : n->childs)
    if (findDynamicNodeStaticCtx(c))
      return true;
  return false;
}


bool findDynamicNodeDynamicCtx(SemanticTree *n) {
  static const auto checkVarContext = [](SemanticTree *node, ResolvedEntity *def) -> bool {
    if (!def->isVariable() && !def->isFunArgument())
      return false;
    if (ResolvedEntity *parentDef = node->getParent()->uddlEntity())
      if (RefAbstract *parentExpr = parentDef->toSelfRefAbstract())
        if (Function *func = parentExpr->refDefinition()->toSelfFunction())
          // Check TO_CHAR(x, ?) - Variable mask
          if (func->isSystem() &&
            func->getName()->toNormalizedString() == "TO_CHAR" &&
            std::distance(node->getParent()->childs.begin(), node->getPositionInParent()) == 1)
            return true;
    return false;
  };
  switch (n->cathegory) {
    case SCathegory::FromTableDynamic: // преобразования TABLE(...)
    case SCathegory::QueryEntityDyn:
    case SCathegory::DynamicFuncallTranslator:
    case SCathegory::FunctionDynTail_:
      return true;
    case SCathegory::FromTableReference:
      if (ResolvedEntity *def = n->refDefinition())
        if (def->ddlCathegory() == ResolvedEntity::Variable_ || def->ddlCathegory() == ResolvedEntity::FunctionArgument_)
          return true;
      break;

    default:
      break;
  }
  if (ResolvedEntity *nodeDef = n->uddlEntity())
    if (RefAbstract *expr = nodeDef->toSelfRefAbstract())
      if (ResolvedEntity *def = expr->reference->majorEntity()->definition())
        if (checkVarContext(n, def))
          return true;
  for (Sm::SemanticTree::Childs::value_type &c : n->childs)
    if (findDynamicNodeDynamicCtx(c))
      return true;

  return false;
}


bool findDynamicNode(SemanticTree *n, int deepDynamic) {
  if (deepDynamic > 1)
    return findDynamicNodeDynamicCtx(n);
  return findDynamicNodeStaticCtx(n);
}


template <typename T>
bool setNeedDirectFlagInternal(T &obj, int deepDynamic) {
  if (obj.hasDynamicQueryExpr())
    return true;

  return findDynamicNode(nAssert(obj.getSemanticNode()), deepDynamic);
}


/**
 * Установить признак необходимости трансляции через DIRECT makestr(
 * для случаев, когда поддерево имеет сущности нуждающиеся
 * в динамической трансляции
 */
template <typename T>
void setNeedDirectFlag(T &obj, unsigned int &flags, int deepDynamic) {
  if (setNeedDirectFlagInternal(obj, deepDynamic)) {
    flags |= SQLSTR_NEED_DIRECT;
    if (SemanticTree *n = obj.getSemanticNode())
      n->setHasDynamicChilds();
  }
}


template <typename T>
void translateSqlToString(T &obj, Codestream &str, unsigned int flags) {
  Sm::Codestream sqlExprStr;
  setNeedDirectFlag(obj, flags, str.state().dynamicSqlDeep_ + 1);
  setTranslateSqlToStringFlags(sqlExprStr, str, flags);
  obj.translate(sqlExprStr);
  sqlExprStr << ';'; // не s::semicolon - для запрета индентинга последней кавычки
  sqlExprStr << s::StubLength(110);

  wrapSqlCodestreamIntoString(&obj, str, sqlExprStr, flags);
}

void translateAsOpenFor(Sm::Codestream &str, ResolvedEntity *def, Ptr<SqlExpr> query) {
  ResolvedEntity::ScopedEntities cat = def->ddlCathegory();
  str << s::otabcommalist();
  str << "OPEN" << s::name;
  switch (cat) {
    case ResolvedEntity::LinterCursor_:
    case ResolvedEntity::Variable_:
    case ResolvedEntity::FunctionArgument_:
      def->linterReference(str);
      break;
    default:
      throw 999; // нужно это реализовать
      break;
  }

  str << " FOR ";

  if (query->isSubquery())
    translateSqlToString(*query, str, 0);
  else if (query->isQuotedSqlExprId())
    str << query;
  else
    str << "DIRECT " << query;
  str << s::ctabcommalist();
}
template <typename T>
void translateAsCopyCollection(Codestream &str, const IdEntitySmart &collEntity, T *query) {
  // TODO: сделать индексацию для записи в список коллекций
  ResolvedEntity *pCollectionType;
  if (!collEntity.majorObjectRef(&pCollectionType)) {
    trError(str, s << "ERROR: majorObjectRef is empty for " << collEntity);
    return;
  }

  string insertMethod = pCollectionType->toSelfObjectType()->getMethodName(Type::ObjectType::BM_COPY_SQL, 0, NULL);
  str << "CALL " << insertMethod << s::obracket;

  str << collEntity << s::comma()
      << "makestr("; // в индентинге makestr() - нужно пропускать скобки

  str.incIndentingLevel(6);
  translateSqlToString(*query, str, SQLSTR_EXPAND_SELECT | SQLSTR_SUPRESS_DIRECT);
  str.decIndentingLevel(6);

  str << ")" << s::comma();
  translateQueryFieldsEscape(str, pCollectionType, true);
  str << s::cbracket;
}

template <typename CollectionRefList>
void translateFetchIntoCollection(Sm::Codestream &str, const CollectionRefList &colReferences, ResolvedEntity *cursor, Ptr<PlExpr> limit, CLoc &loc) {
  std::string indexVarName;
  Ptr<Sm::Variable> indexVar = cursor->getTemporaryVar(str, Sm::Datatype::mkInteger(), &indexVarName);
  str << indexVarName << s::name << ":=" << s::name << 1;
  str << s::procendl(loc) << s::tab();

  // Очистка коллекций
  for (auto it = colReferences.begin(); it != colReferences.end(); ++it) {
    ResolvedEntity *pCollectionType;
    if (!(*it)->reference->majorObjectRef(&pCollectionType)) {
      trError(str, s << "ERROR: unresolved major object for " << *((*it)->reference));
      return;
    }
    if ((*it)->reference->isDynamicUsing()) {
      string deleteMethod = pCollectionType->toSelfObjectType()->getMethodName(Type::ObjectType::BM_DELETE, 0, NULL);
      str << "CALL " << deleteMethod << s::obracket << (*it)->reference << s::cbracket;
    }
    else
      str << "ARRAY_DELETE" << s::obracket << (*it)->reference << s::cbracket;
    str << s::procendl() << s::tab();
  }

  str << "WHILE not outofcursor" << s::obracket << s::linref(cursor) << s::cbracket;
  if (limit)
    str << " AND " << indexVarName << " <= " << limit;
  str << s::name << "LOOP" << s::name;
  if (str.state().indentingLongConstruct_)
    str << s::endl;
  str << inctab(2) << s::tab();

  // Вставка в коллекции
  int idx = 0;
  for (auto it = colReferences.begin(); it != colReferences.end(); ++it, ++idx) {
    if ((*it)->reference->isDynamicUsing()) {
      translateCursorAsInsertCollection(str, *((*it)->reference), cursor, indexVar, (colReferences.size() > 1) ? idx : -1);
      str << s::procendl() << s::tab();
    }
    else {
      EntityFields collectFields, cursorFields;
      (*it)->reference->getFields(collectFields);
      cursor->getFields(cursorFields);
      if (colReferences.size() > 1 && int(cursorFields.size()) > idx) {
        // Для вставки в определенную коллекцию урезаем до одного значимого поля
        Ptr<Id> significant = cursorFields[idx];
        cursorFields.clear();
        cursorFields.push_back(significant);
      }

      if (collectFields.size() != cursorFields.size()) {
        //Неправильно изьяты поля курсоров
        trError(str, s << "ERROR: translateFetchIntoCollection collection fields and cursor fields has different size. Check cursors.");
      }
      else {
        EntityFields::iterator colIt = collectFields.begin();
        EntityFields::iterator curIt = cursorFields.begin();
        for ( ; colIt != collectFields.end(); ++colIt, ++curIt) {
          bool isDef = (*colIt)->toNormalizedString() == "COLUMN_VALUE";
          str << (*it)->reference << "[" << indexVarName << "]";
          if (!isDef)
            str << "." << s::linref((*colIt)->definition());
          str << s::name << ":=" << s::name << s::linref(cursor) << "." << s::linref((*curIt)->definition());
          str << s::procendl() << s::tab();
        }
      }
    }
  }

  str << "FETCH " << s::linref(cursor);
  if (str.state().indentingLongConstruct_)
    str << s::procendl();
  str << s::tab() << indexVarName << s::name << ":=" << s::name << indexVarName << " + 1" << s::name;
  if (str.state().indentingLongConstruct_)
    str << s::procendl();
  str << dectab(2);
  str << s::tab() << "ENDLOOP";
}


class TranslatorAsSelectCollection : public TranslatorTailMakestr {
public:

  const IdEntitySmart &reference;
  Sm::ResolvedEntity *collection;
  Type::ObjectType::ExtractColFields extrFields;

  TranslatorAsSelectCollection(const IdEntitySmart &_reference, Sm::ResolvedEntity *_collection)
    : reference(_reference), collection(_collection) {}

  void translate(Sm::Codestream &str) {
    str << s::ocommalist();
    str << "SELECT" << s::name;

    if (!Type::ObjectType::extractCollectionAccessorFields(reference, extrFields))
      throw 999;

    str << s::ocommalist();
    translateQueryFields(str, extrFields.opFields, false);
    str << s::ccommalist();
    str << s::name << s::subconstruct
        << " FROM " << s::querySign << " WHERE OBJREFID__ = " << s::querySign << " AND ";
    switch (collection->ddlCathegory()) {
      case ResolvedEntity::Object_:
        throw 999;
      case ResolvedEntity::NestedTable_: {
        Type::NestedTable *ntable = static_cast<Type::NestedTable*>(collection->toSelfCollectionType());
        if (ntable->keyType()->isExactlyEqually(Datatype::mkVarchar()))
          str << "KEY__ = '" << s::querySign << "'";
        else
          str << "KEY__ = CAST(" << s::querySign << " AS INT)";
      } break;
      default:
        str << "ID__ = CAST(" << s::querySign << " AS INT)";
        break;
    }
    str << s::ccommalist();
  }

  void translateTail(Sm::Codestream &str) {
    str << s::CommaMakestr()
        << s::ocommalist() << "SYS.CI_GET_TABNAME__" << s::obracket << extrFields.newReference << s::cbracket
        << s::comma() << "SYS.CI_GET_OBJREFID__" << s::obracket << extrFields.newReference << s::cbracket
        << s::comma() << extrFields.indexExpr
        << s::ccommalist();
  }

  Sm::SemanticTree *getSemanticNode() const {
    static SemanticTree *n = new SemanticTree(); // Empty node
    return n;
  }
  bool hasDynamicQueryExpr() const { return false; }
};

void translateCursorAsSelectCollection(Sm::Codestream &str, const IdEntitySmart &reference, Sm::ResolvedEntity *collection, Sm::ResolvedEntity *cursor) {
  if (reference.entity()->beginedFrom(23665, 62))
    cout << "";
  str << s::otabcommalist();
  str << "OPEN";
  str << s::name << s::ref << cursor << s::name;
  str << "FOR" << s::name;

  TranslatorAsSelectCollection translator(reference, collection);
  // DIRECT нужен, т.к. в строку будут включены конструкции вида FROM ? WHERE OBJREFID__ = ? AND
  translateSqlToString(translator, str, SQLSTR_NEED_DIRECT | SQLSTR_HAS_ADDITIONAL_IDENTIFICATORS);
  str << s::ctabcommalist();
}

bool isUsedAsCollectionFetch(Sm::BlockPlSql *block, ResolvedEntity *cursor, FoundedStatements &foundStmts) {
  /* Условия использования:
   *  оператор Open (Open for)
   *  оператор Fetch bulk collect into (без limit)
   *  оператор Close
   *  никакие больше операции с курсором не поддерживаются
   */
  std::function<bool(StatementInterface *)> functor = [cursor](StatementInterface *statement) -> bool {
    if (statement->isFetchStatement()) {
      Sm::Fetch *fetch = static_cast<Sm::Fetch *>(statement);
      if (fetch->cursorRef->refDefinition() == cursor &&
          fetch->intoCathegory == Sm::Fetch::COLLECTION &&
          !fetch->limit) {
          if (!syntaxerContext.translateReferences ||
            (fetch->fields.size() == 1 && fetch->fields.front()->reference->isDynamicUsing()))
            return true;
          return false;
        }
    }
    else if (Sm::OpenFor *open = statement->toSelfOpenFor()) {
      if (open->openForIterVariable->unresolvedDefinition() == cursor)
        return true;
    }
    else if (Sm::OpenCursor *open = statement->toSelfOpenCursor()) {
      if (open->cursor->getNextDefinition() == cursor)
        return true;
    }
    else if (statement->isCloseStatement()) {
      Sm::Close *close = static_cast<Sm::Close *>(statement);
      if (close->cursorEntity->unresolvedDefinition() == cursor)
        return true;
    }
    FoundedStatements containStatements;
    statement->getStatementsThatContainEntity(cursor, containStatements);
    return containStatements.size() > 0;
  };

  block->getTopBlock()->findStatements(functor, foundStmts, false);

  if (foundStmts.size() == 3) {
    return (foundStmts[0]->toSelfOpenCursor() != NULL /* ||
           foundStmts[0]->isOpenStatement()*/) &&
           foundStmts[1]->isFetchStatement() &&
           foundStmts[2]->isCloseStatement();
  }
  return false;
}

template <typename T>
bool translateAsCollectionFetch(Sm::Codestream &str, Sm::BlockPlSql *block, ResolvedEntity *cursor, T *query) {
  // Способ 1. Второй алгоритм см. в Fetch
  FoundedStatements foundStmts;
  if (!isUsedAsCollectionFetch(block, cursor, foundStmts))
    return false;

  Sm::Fetch *fetchStmt = static_cast<Sm::Fetch *>(foundStmts[1]);
  for (auto it = fetchStmt->fields.begin(); it != fetchStmt->fields.end(); ++it)
    translateAsCopyCollection(str, *((*it)->reference), query);

  for (auto it = foundStmts.begin(); it != foundStmts.end(); ++it)
    (*it)->translateMode(StatementInterface::TM_IGNORE); // выключить трансляцию найденых операторов

  return true;
}

void translateInUsingList(Sm::Codestream &str, Ptr<List<ArgumentNameRef>> bindArguments) {
  bool isNotFirst = false;
  str << s::name << s::subconstruct << "USING" << s::name << s::ocommalist();
  for (Ptr<ArgumentNameRef> &bindArg : *bindArguments) {
    if (bindArg->direction == ArgumentNameRef::IN || bindArg->direction == ArgumentNameRef::IN_OUT)
      str << s::comma(&isNotFirst) << bindArg->id;
  }
  str << s::ccommalist();
}

bool Sm::OpenFor::translatedAsExecuteWithoutDirect() const { return true; }

void Sm::OpenFor::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(117358))
    cout << "";
  str << s::executeStatementPointer(this, str.state());
  translateAsOpenFor(str, openForIterVariable->definition(), select);
  if (bindArguments && bindArguments->size() > 0)
    translateInUsingList(str, bindArguments);
}


void Sm::OpenCursor::linterDefinition(Sm::Codestream &str) {
  ResolvedEntity *cursorDef = cursor->getNextDefinition();
  if (!cursorDef)
    return;

  cursorDef->setOpenCursorCommand((OpenCursor*)this);
  if (!translateAsCollectionFetch(str, ownerBlock, cursorDef, cursorDef->getSelectQuery())) {
    str << s::otabcommalist();
    str << s::executeStatementPointer(this, str.state());
    str << "OPEN" << s::name << cursor << " FOR ";
    if (Subquery *q = cursorDef->getSelectQuery())
      translateSqlToString(*q, str, 0 /*SQLSTR_NEED_DIRECT*/);
    str << s::ctabcommalist();
  }
  cursorDef->clrOpenCursorCommand();
}

void Sm::CursorParameter::linterReference(Codestream &str) {
  if (indexInParameters < 0 || !ownerCursor)
    throw 999;

  if(!ownerCursor->currentOpenCommand      ||
      ownerCursor->currentOpenCommand->actualCursorParameters.size() <= (unsigned int)indexInParameters) {
    if (int(ownerCursor->actualCursorParameters.size()) > indexInParameters)
      str << ownerCursor->actualCursorParameters[indexInParameters];
    else {
      cout << "error: cursor has not actual parameter " << ownerCursor->getLLoc().toString() << endl;
      str << "__ERROR__: cursor has not actual parameter ";
    }
  }
  else
    str << ownerCursor->currentOpenCommand->actualCursorParameters[indexInParameters];
}

void Sm::Label::linterDefinition(Sm::Codestream &str) { str << name << ((name->isReservedField()) ? "_" : "") << ':'; }
void Sm::Goto::linterDefinition(Sm::Codestream &str) { str << "GOTO " << gotoLabel << ((gotoLabel->isReservedField()) ? "_" : ""); }

void Sm::PipeRow::oracleDefinition(Sm::Codestream &str) { str << "PIPE ROW" << s::obracket << pipedRow << s::cbracket; }
void Sm::PipeRow::linterDefinition(Sm::Codestream &str) {
  str << s::Comment() << "PIPE ROW( )" << s::endl << s::tab();

  BlockPlSql *block = getOwnerBlock()->getTopBlock();
  Function *func = block->ownerFunction()->toSelfFunction();
  if (!func || !func->isPipelined() || !func->pipeVar || !func->pipeIndex) {
    trError(str, s << "ERROR: PIPEROW used without pipelined function:" << getLLoc());
  }
  else {
    Ptr<Id> accessor = new Id("", new Type::collection_methods::AccessToItem(func->pipeVar.object()));
    Ptr<PlExpr> indexExpr = new RefExpr(getLLoc(), func->pipeIndex->getName());
    accessor->callArglist = new CallArgList({new FunCallArgExpr(getLLoc(), indexExpr)});
    IdEntitySmart ref;
    ref.push_back(accessor);
    ref.push_back(new Id(func->pipeVar->getName()->toNormalizedString(), func->pipeVar.object()));
    Ptr<PlExpr> expr = new RefExpr(getLLoc(), pipedRow);

    Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(func->pipeVar->getDatatype());
    Type::CollectionType *colType = datatype->getNextDefinition()->toSelfCollectionType();
    str << "CALL " << colType->getMethodName(Type::ObjectType::BM_EXTEND, 0, NULL);
    str << s::obracket << s::linref(func->pipeVar) << s::cbracket << s::procendl(getLLoc()) << s::tab();
    Type::RefDecoder().decode(str, ref, expr);
    str << s::procendl(getLLoc()) << s::tab();
    str << s::linref(func->pipeIndex) << s::name << ":=" << s::name << s::linref(func->pipeIndex) << " + " << 1;
  }
}


void Sm::LinterCursor::linterDefinition(Sm::Codestream &str) {
  str << "VAR" << s::name << name << s::name;
  translateAsCursor(str);
}

void translateExpandFields(EntityFields &fields, Sm::Codestream &str, bool &isNotFirst, string parentName, bool cursorRoot) {
  for (EntityFields::iterator it = fields.begin(); it != fields.end(); ++it) {
    ResolvedEntity *d = (*it)->definition();

    if (syntaxerContext.unwrapStructuredFields)
      if (VariableField *f = d->toSelfVariableField())
        if (!syntaxerContext.translateReferences && f->isStructuredField) {
          translateExpandFields(f->fields_, str, isNotFirst, "", false);
          continue;
        }

    Ptr<Datatype> datatype = d->getDatatype();
    Ptr<Datatype> lasttype = Datatype::getLastConcreteDatatype(datatype);

    if (syntaxerContext.unwrapStructuredFields
        && lasttype && (lasttype->isRowTypeOf() || lasttype->isRecordType()))  {
      string newParentName;
      EntityFields nestedFields;
      d->getFields(nestedFields);
      if (d && d->procTranslatedName().size())
        newParentName = parentName + d->procTranslatedName() + "_";
      else
        newParentName = parentName + (*it)->toNormalizedString() + "_";
      translateExpandFields(nestedFields, str, isNotFirst, newParentName, false);
    }
    else {
      str << s::comma(&isNotFirst);
      str << parentName;
      Ptr<Id> &n = *it;

      if (d && d->procTranslatedName().size())
        str << d->procTranslatedName();
      else if (d && cursorRoot && n->isReservedField()) {
        d->procTranslatedName(n->toNormalizedString() + "_");
        str << d->procTranslatedName();
      }
      else
        str << (*it);

      str << s::name;

      if (!datatype)
        trError(str, s << "Unknown datatype of field " << (*it)->getText());
      else if (!lasttype) {
        cout << "error: unresolved datatype " << datatype->getLLoc() << endl;
        str << datatype;
      }
      else if (lasttype->isNull())
        str << "INT";
      else
        str << datatype;
    }
  }
}

void translateFieldsAsCursor(EntityFields &fields, Sm::Codestream &str, bool /*expand = true*/) {
  bool isNotFirst = false;
  str << "CURSOR"
      << s::OBracketView();
  translateExpandFields(fields, str, isNotFirst, string(), true);
  str << s::CBracketView();
}

void translateAsLinterCursor(ResolvedEntity *query, Sm::Codestream &str, bool expand/*= true*/) {
  if (query) {
    EntityFields fields;
    query->getFields(fields);
    translateFieldsAsCursor(fields, str, expand);
  }
}

void translateFieldsAsStruct(EntityFields &fields, Sm::Codestream &str, bool /*expand = false*/) {
  bool isNotFirst = false;

  str <<  "STRUCT" << s::OBracketView();
  translateExpandFields(fields, str, isNotFirst, string(), true);
  str << s::CBracketView();
}

void translateAsLinterStruct(ResolvedEntity *query, Sm::Codestream &str, bool expand/* = false*/) {
  if (query) {
    EntityFields fields;
    query->getFields(fields);
    translateFieldsAsStruct(fields, str, expand);
  }
}

void Sm::Type::Record::linterDatatypeReference(Sm::Codestream &str) {
  if (xmlType_ != 0)
    str << "XMLREF";
  else if (syntaxerContext.translateReferences)
    translateAsLinterStruct((Record*)this, str);
  else
    translateAsLinterCursor((Record*)this, str);
}

void Sm::QueryBlock::linterReference(Sm::Codestream &str) { // вывести курсорный тип
   translateAsLinterCursor((QueryBlock*)this, str);
}

void Sm::LinterCursor::translateAsCursor(Sm::Codestream &str) {
  translateAsLinterCursor(query.object(), str);
}

void BlockPlSql::linterGlobalVariable(Codestream &str, Declarations::iterator it)
{
  if (Ptr<Id> n = (*it)->getName())
    if (n->beginedFrom(16467))
      cout << "";

  sAssert(!(*it)->toSelfVariable());
  str << s::ocreate(*it) << "VARIABLE " << s::cref(*it) << s::name;

  if (Ptr<Datatype> t = (*it)->getDatatype()) {
    ResolvedEntity *oldD = t.object();
    ResolvedEntity *d = t->getNextDefinition();
    while (d && d->ddlCathegory() != ResolvedEntity::FundamentalDatatype_ && oldD != d) {
      oldD = d;
      d = d->getNextDefinition();
    }
    if (oldD->getLLoc().beginedFrom(16467))
      cout << "";
    if (oldD->ddlCathegory() == ResolvedEntity::Object_)
      str << "BIGINT" << s::semicolon << s::Comment() << s::loc(oldD->getLLoc()); //iloc
    else {
      oldD->sqlDefinition(str);
      str << s::semicolon;
    }
  }
  else
    str << s::semicolon;
  str << s::name << s::loc((*it)->getLLoc())
      << s::ccreate;
}

void Sm::BlockPlSql::translateDeclarations(Codestream &str, DependEntitiesMap *filter, std::set<ResolvedEntity*, LE_ResolvedEntities> &outputted, bool isDecl_) {
  for (Declarations::iterator it = declarations.begin(); it != declarations.end(); ++it) {
    if (isNotPackageBlock_)
      (*it)->setOwnerBlockPlSql((BlockPlSql*)this);
    if (!(*it)->hasLinterEquivalent())
      continue;
    if ((*it)->isFunction() && !(*it)->isDefinition())
      continue;
    if ((filter && filter->size() && !filter->count(*it)) || outputted.count(it->object()))
      continue;
    outputted.insert(it->object());
    ResolvedEntity *defFirst = it->object()->getDefinitionFirst();
    if (it->object() != defFirst) {
      if ((filter && filter->size() && !filter->count(defFirst)) || outputted.count(defFirst))
        continue;
      outputted.insert(defFirst);
    }

    if (isDecl_)
      str << s::decl;
    else
      str << s::def;
    str.activateActions();

    if ((*it)->ddlCathegory() == ResolvedEntity::Variable_)
      linterGlobalVariable(str, it);
    else
      str << *it;
    str << s::endl;
    str.joinPostactions();
  }
}

void translateDeclarationList(Codestream &str, Sm::Declarations &decl) {
  str.incIndentingLevel(2);
  for (Sm::Declarations::iterator it = decl.begin(); it != decl.end(); ++it)
    if ((*it)->hasLinterEquivalent()) {
      if ((*it)->isFunction()) {
        str.levelPush();
        str.procMode(CodestreamState::SQL);
        // извлечь внешние переменные, и записать их дополнительным списком
        (*it)->extractExternalVariables();
        bool outputToCodestream = true;
        if (Sm::Function *f = (*it)->toSelfFunction())
          if (f->flags.isAlreadyInCodestream())
            outputToCodestream = false;

        if (outputToCodestream)
          str << *it << s::semicolon << s::endl;
        str.procMode(CodestreamState::PROC);
        str.join();
        str.actionsToPreactions();
        str.levelPop();
        str.activateDeclarations();
      }
      else if ((*it)->isRefCursor() || (*it)->toSelfSubtype() || (*it)->toSelfCursor())
        continue;
      else if ((*it)->isException())
        continue;
      else {
        str << s::tab(str.indentingLevel());
        str << *it;
        procEndlWithoutEndl(str, s::procendl());
        str << s::loc((*it)->getLLoc()) << s::endl;
      }
    }
  str.decIndentingLevel(2);
}


void StatementsTranslator::translateStatement(Codestream &str, StatementInterface *it, BlockPlSql* codeBlock) {
  static unordered_set<ScopedEntities> cathegoriesToSkip = {
    CursorFieldDecltype_, DeclNamespace_, CursorDecltype_,
  };
  if (cathegoriesToSkip.count(it->ddlCathegory()))
    return;

  bool needSemicolon = true;
  str.levelPush();

  if (it->toSelfLabel() == nullptr && !it->toSelfIf())
    str << s::tab(str.indentingLevel());
  if (codeBlock)
    it->traverseStatements(StatementInterface::SetOwnerBlk(codeBlock));
  str << s::def;

  if (BlockPlSql *blk = it->toSelfBlockPlSql()) {
    if (blk->exceptionListSize() == 0) {
      /*str.activateDeclarations();
      translateDeclarationList(str, *(it->blockDeclarations()));
      str.activatePrevious();
      */
      translateStatementsList(str, blk->statements, codeBlock);
    }
    else if (syntaxerContext.model->modelActions.createBranchFunctions())
      throw 999;
    else {
      str << s::statementPointer(it, str.state());
      it->translate(str);
      needSemicolon = false;
    }
  }
  else if (it) {
    str << s::statementPointer(it, str.state());
    it->translate(str); // TODO: в стейтментах писать в префиксы а не в preactions
  }
  if (needSemicolon && it->needSemicolon())
    str << s::procendl(it->getLLoc());
  else
    str << s::endl;

  str.joinPrefixes();
  str.joinSuffixes();
  str.joinPostactions();
  str.levelPop();

  VarHelper::getInstance().unuseAll();
}


bool compareByNamespacePosition(ResolvedEntity *v1, ResolvedEntity *v2) {
  if (v1 && v2)
    return v1->getDefinitionFirst()->eid() < v2->getDefinitionFirst()->eid();
  else
    return false;
}

bool compareEntitiesByPosAndFuncat(const Ptr<EntityAttributes> &pv1, const Ptr<EntityAttributes> &pv2) {
  const EntityAttributes *v1 = pv1.object();
  const EntityAttributes *v2 = pv2.object();
  if (v1->entity && v2->entity) {
    bool b1 = v1->entity->isFunArgument();
    bool b2 = v2->entity->isFunArgument();
    if (b1 && b2)
      return compareByNamespacePosition(v1->entity, v2->entity);
    else if (b2) // правый - аргумент функции -> сделать его левым
      return false; // все аргументы функций будут находиться в начале списка
    else if (b1) // левый - аргумент функции -> оставить его на месте
      return true; // все аргументы функций будут находиться в начале списка
    else
      return compareByNamespacePosition(v1->entity, v2->entity);
  }
  else if (v1->entity)
    return true; // все нули будут находиться в конце списка
  else if (v2->entity)
    return false;
  return false;
}

}

void EntityAttributes::translateAsLinterFunctionArgument(Sm::Codestream &str) {
  if (!entity)
    throw 999;
  if (read() && write())
    str << "INOUT ";
  else if (write())
    str << "OUT ";
  else
    str << "IN ";

  str << s::ref;
  entity->translateAsFunArgumentReference(str);

  str << s::def << s::name;
  if (Sm::Variable *var = entity->toSelfVariable())
    if (!var->fields_.empty()) {
      if (var->isLinterStructType())
      	translateFieldsAsStruct(var->fields_, str);
     	else
     		translateFieldsAsCursor(var->fields_, str);
      return;
    }
  str << entity->getDatatype();
}

namespace Sm {

void Sm::Cursor::linterReference(Sm::Codestream &/*str*/) {
  throw UnimplementedOperation("Cursor::linterReference");
//  str << name;
//  str << s::linref(cursorVariable);
}

void Sm::Cursor::linterDefinition(Sm::Codestream &str) {
//  throw 999;
//  str << s::lindef(cursorVariable);
  return;
  str << "VAR" << s::name;
  linterReference(str);
  EntityFields fields;
  if (rowtype)
    rowtype->getFields(fields);
  if (fields.empty() && select)
    select->getFields(fields);
  if (fields.size())
    translateFieldsAsCursor(fields, str);
}


void Sm::SelectedField::translateAsFunArgumentReference(Sm::Codestream &str)  {
  if (alias_)
    str << alias_;
  else
    expr_->translateName(str);
}

void Sm::SelectedField::linterReference(Sm::Codestream &str)  {
  translateAsFunArgumentReference(str);
}

void Sm::QueryPseudoField::translateNameIfSpecial() {
  if (nameTranslated_)
    return;
  std::string s = fieldName->toNormalizedString();
  if (Sm::Id::isFieldReserved(s))
    fieldName->setSpecSymbols();
  nameTranslated_ = true;
}

void Sm::BlockPlSql::extractBlockExternalVariables(ExternalVariables &sortedExternalReferences, UniqueEntitiesMap &externalReferences) {
  // 0. Извлечь область имен с аргументами и определениями.
  // 1. Составить список зависимостей - аргументов функций, индексирующих переменных и локальных (непакетных) переменных
  //    не определенных в данной области имен.
  //    Особо обрабатывать индексирующие переменные - см. как они строятся и резолвятся.
  //    Причем учитывать, считывается ли переменная или присваивается, и сохранять для нее соответствующие модификаторы
  // -> { Unique References }  := getAllReferences
  SemanticTree *node = getSemanticNode();
  if (!node)
    return;
  node->getAllCodeBlockReferences(externalReferences);
  node->getParent()->getAllCodeBlockReferences(externalReferences); // Извлечь также DEFAULT параметры аргументов
  // ExternalDefinitions = { Unique References } \ ({ Internal Definitions } U { GLOBAL VARIABLES })
  // 2. Удалить из этого списка всё, что относится к базовым сущностям модели
  // и не является полями или является полями таблиц/представлений.
  if (node->childNamespace)
    node->childNamespace->deleteIncludedDeclarationsFromSet(externalReferences);


  for (UniqueEntitiesMap::iterator it = externalReferences.begin(); it != externalReferences.end(); ) {
    ResolvedEntity* ent = it->first;
    sAssert(!ent);
    Variable *var;
    if (ent->isPackageVariable() || ent->isNonblockPseudoField() || ent->toSelfMemberVariable())
      it = externalReferences.erase(it);
    else if ((var = ent->toSelfVariable()) && var->baseField && var->baseField->toSelfCursor())
      // Курсорные переменные создаются локально в каждом блоке plsql. См. FormalTranslator::translateCursorToCursorVariables
      it = externalReferences.erase(it);
    else if (!ent->isVariable() &&
             !ent->isFunArgument() &&
             !ent->isCursorVariable() &&
             !ent->isTriggerRowReference() &&
             !ent->toSelfFunction())
      it = externalReferences.erase(it);
    else
      ++it;
  }


  auto checkPushAttr = [&sortedExternalReferences](ExternalVariables::reference attr) {
    for (ExternalVariables::reference existAttr : sortedExternalReferences) {
      if (existAttr->entity == attr->entity) {
        existAttr->concat(*attr);
        return;
      }
    }
    sortedExternalReferences.push_back(attr);
  };

  BlockPlSql *ownerBlk = getOwnerBlock();

  for (UniqueEntitiesMap::iterator it = externalReferences.begin(); it != externalReferences.end(); ++it) {
    ResolvedEntity* ent = it->first;
    Function *func;
    if ((func = ent->toSelfFunction())) {
      if (ownerBlk && ownerBlk != this && func->body() && func->body()->getOwnerBlock() == ownerBlk) {
        // Если это вложенная функция, имеющая общую с текущей функцию-предок
        func->extractExternalVariables();
        for (ExternalVariables::reference attr : func->externalVariables)
          checkPushAttr(attr);
      }
    }
    else {
      it->second->entity = ent;
      checkPushAttr(it->second);
    }
  }

  // -> { Unique Definitions } -> sort by:
  //    function arguments early with saved order
  //    declarations with save order;
  std::sort(sortedExternalReferences.begin(), sortedExternalReferences.end(), compareEntitiesByPosAndFuncat);
}

void BlockPlSql::translateDependedBlockVariablesToArglist(Codestream &str, ExternalVariables &arglist, UniqueEntitiesMap &externalReferences)
{
  extractBlockExternalVariables(arglist, externalReferences);
  for (ExternalVariables::iterator it = arglist.begin(); it != arglist.end(); ++it)
    if (ResolvedEntity *ent = (*it)->entity)
      ent->setTransitive(true);
  if (arglist.size()) {
    ExternalVariables::iterator it = arglist.begin();
    (*it)->translateAsLinterFunctionArgument(str);
    ++it;
    if (it == arglist.end())
      str.joinSuffixes();
    else {
      str.incIndentingLevel(4);
      for (; it != arglist.end(); ++it) {
        str << s::semicolon << s::name;
        str.joinSuffixes();
//        str << s::endl << s::tab();
        (*it)->translateAsLinterFunctionArgument(str);
      }
      str.decIndentingLevel(4);
    }
  }
}

BlockPlSql *Sm::BlockPlSql::getTopBlock()
{
  BlockPlSql *topBlock;
  for (topBlock = this; topBlock->ownerBlock && !topBlock->isFunctionTopBody_; topBlock = topBlock->ownerBlock) {;}
  return topBlock;
}

bool Sm::BlockPlSql::hasExceptionDeclaration(Ptr<PlExpr> exceptCond) {
  if (!exceptCond)
    return false;

  string target;
  exceptCond->toNormalizedString(target);
  ExceptionHandlers &trExcs = translatedExceptionDeclarations;
  for (ExceptionHandlers::iterator it = trExcs.begin(); it != trExcs.end(); ++it) {
    if (!(*it)->toSelfWhenExpr()->condition)
      continue;

    string current;
    (*it)->toSelfWhenExpr()->condition->toNormalizedString(current);
    if (target == current)
      return true;
  }

  return false;
}

void Sm::BlockPlSql::addExceptionDeclaration(Ptr<WhenExpr> exception) {
  translatedExceptionDeclarations.push_back(exception.object());
}

Ptr<Declaration> Sm::BlockPlSql::hasDeclaration(Ptr<Id> varId) {
  for (Declaration *decl : declarations) {
    if (Ptr<Id> declName = decl->getName())
      if (declName->toNormalizedString() == varId->toNormalizedString())
        return decl;
  }
  return ownerBlock && !isFunctionTopBody_ ? ownerBlock->hasDeclaration(varId) : Ptr<Declaration>();
}

Ptr<Variable> BlockPlSql::getCreateCursorLocker(Codestream &str, ResolvedEntity *ent) {
  LockerMap::iterator it = cursorLockers.find(ent);
  if (it != cursorLockers.end())
    return it->second;

  string varName;
  Ptr<Variable> cursorLocker = ent->addVariableIntoOwnerBlock(Datatype::mkBoolean(), &varName, FLoc(), "TmpFF");
  str.activateDeclarations();
  str << s::tab(2) << cursorLocker << " DEFAULT FALSE" << s::procendl();
  str.activatePrevious();
  cursorLockers.insert(LockerMap::value_type(ent, cursorLocker));
  return cursorLocker;
}

void BlockPlSql::lindefCodeBlock(Codestream &str, bool declareIsNotOutput, bool isInternalBlock, PlExpr *whenCondition) {
  str << "CODE" << s::endl;


  if (whenCondition) {
    str.incIndentingLevel(2);
    str << s::tab(str.indentingLevel()) << "IF " << *whenCondition << " THEN" << s::endl;
    str.incIndentingLevel(2);
  }
  translateStatementsList(str, initializators, (BlockPlSql*)this);
  translateStatementsList(str, statements    , (BlockPlSql*)this);

  if (dynamicDeclTail)
    str << s::tab(str.indentingLevel() + 2) << dynamicDeclTail << s::endl;


  if (whenCondition) {
    str.decIndentingLevel(2);
    str << s::tab();
    str << "ENDIF" << s::endl;
    str.decIndentingLevel(2);
  }

  if (exceptionHandlers.size()) {
    str << s::tab(str.indentingLevel()) << "EXCEPTIONS" << s::endl;
    str.incIndentingLevel(2);
    for (ExceptionHandlers::iterator it = exceptionHandlers.begin(); it != exceptionHandlers.end(); ++it)
      str << s::tab(str.indentingLevel()) << *it;
    str.decIndentingLevel(2);
  }
  if (str.indentingLevel())
    str << s::tab(str.indentingLevel());

  if (!isInternalBlock) {
    str.activateDeclarations();
    str.incIndentingLevel(2);
    ExceptionHandlers &trExcs = translatedExceptionDeclarations;
    for (ExceptionHandlers::iterator it = trExcs.begin(); it != trExcs.end(); ++it)
       (*it)->toSelfWhenExpr()->translateLinterExceptionDeclarations(str, exceptionFromOperators);
    for (DeclaredExceptions::value_type &it : exceptionFromOperators)
      if (it.second.first != 0)
        it.second.first->translateExceptionDeclarations(str, it.second.second);
    str.decIndentingLevel(2);
    str.activatePrevious();
  }

  str << "END";

  if (!isInternalBlock) {
    CodestreamStack *currentLevel = str.getCurrentLevel();
    if (!currentLevel->declarations.empty() && declareIsNotOutput) {
      str.activatePredeclarations();
      str << "DECLARE" << s::endl;
      str.activatePrevious();
    }
    str.joinDeclarations();
    str.joinPredeclarations();
    translateGranteeClausesForPlSqlBlock(str);
  }
}

void Sm::BlockPlSql::mergeInternalBlockDecl(BlockPlSql *topBlock) {
  auto mergeDecl = [&](StatementInterface *stmt, bool isConstructor, bool /*hasSublevels*/) -> bool {
    if (!isConstructor || !stmt->isBlockPlSql())
      return true;

    BlockPlSql *block = stmt->toSelfStatement()->toSelfBlockPlSql();
    if (block == topBlock)
      return true;

    for (Declarations::iterator it = block->declarations.begin(); it != block->declarations.end(); ++it) {
      if (!it->object())
        continue;
      LevelResolvedNamespace *levelNamespace = topBlock->getSemanticNode()->childNamespace;
      Ptr<Id> name = (*it)->getName();
      string dst = name->toNormalizedString();
      if (levelNamespace->find(dst) != levelNamespace->end()) {
        dst = levelNamespace->getUniqueName(dst);
        name->definition()->translatedName(dst);
      }
      levelNamespace->addWithoutFind(name.object());
      topBlock->declarations.push_back(*it);
    }
    return true;
  };

  traverseStatements(mergeDecl);
}

void Sm::BlockPlSql::linDef(Codestream &str, PlExpr *whenCondition) {
  if (str.state().isErrorLog)
    return;

  BlockPlSql *topBlock = getTopBlock();
  bool isInternalBlock = topBlock != this;
  bool declareIsNotOutput = declarations.empty();
  branchId_ = 0;
  str.activateDeclarations();
  if (!isInternalBlock) {
    mergeInternalBlockDecl(this);
    if (declarations.size()) {
      str << "DECLARE" << s::endl;
      declareIsNotOutput = false;
    }
    else if (exceptionHandlers.size())
      for (ExceptionHandlers::iterator it = exceptionHandlers.begin(); it != exceptionHandlers.end(); ++it)
        if ((*it)->toSelfWhenExpr()->condition) {
          str << "DECLARE" << s::endl;
          declareIsNotOutput = false;
          break;
        }

    if (declarations.size())
      translateDeclarationList(str, declarations);
  }

  for (Ptr<StatementInterface> exceptExprStmt : exceptionHandlers) {
    Ptr<WhenExpr> exceptExpr = exceptExprStmt->toSelfWhenExpr();
    switch (exceptExpr->cathegory) {
      case WhenExpr::WHEN_THEN:
        exceptExpr->cathegory = WhenExpr::EXCEPTION_HANDLER;
        break;
      case WhenExpr::WHEN_ALL:
        break;
      case WhenExpr::EXCEPTION_HANDLER:
        cout << "error: WhenExpr::EXCEPTION_HANDLER " << exceptExpr->getLLoc() << endl;
        break;
      default:
        throw 999;
    }

    if (exceptExpr->condition && exceptExpr->condition->isPlLogicalCompound()) {
      // Проверить все выражения в логическом компаунде
      List<PlExpr> exprList;
      exceptExpr->condition->enumSubExpressions(exprList);
      for (Ptr<PlExpr> subExpr : exprList)
        if (!topBlock->hasExceptionDeclaration(subExpr)) {
          Ptr<WhenExpr> newExpr = new WhenExpr(subExpr->getLLoc(), new Statements(exceptExpr->branchStatements), subExpr, Sm::WhenExpr::EXCEPTION_HANDLER);
          topBlock->addExceptionDeclaration(newExpr);
        }
    }
    else if (!topBlock->hasExceptionDeclaration(exceptExpr->condition))
      topBlock->addExceptionDeclaration(exceptExpr);
  }

  str.activatePrevious();
  lindefCodeBlock(str, declareIsNotOutput, isInternalBlock, whenCondition);
}


void WhenExpr::translateBody(Codestream &str) {
  if (outStatements_) {
    str << s::endl;
    str.incIndentingLevel(2);
    if (!ownerBlock)
      throw 999;

    BlockPlSql *blk; // TODO: uncomment after bugfixing
    for (Statements::iterator it = branchStatements.begin(); it != branchStatements.end(); ++it)
      if ((blk = it->object()->toSelfBlockPlSql()) && blk->exceptionHandlers.empty())
        translateStatementsList(str, blk->statements, ownerBlock);
      else
        StatementsTranslator::translateStatement(str, it->object(), ownerBlock);

    str.decIndentingLevel(2);
  }
}

void Exception::translateExceptionDeclarations(Sm::Codestream &str, const FLoc &l) {
  if (translatedNames_.empty()) {
    str << s::tab();
    linterDefinition(str);
    str << s::procendl(l);
    return;
  }
  std::vector<std::string>::iterator it = translatedNames_.begin();
  str << s::tab() << "EXCEPTION " << getName() << " FOR " << *it << s::procendl(l);
  int pos = 2;
  for (++it; it != translatedNames_.end(); ++it, ++pos)
    str << s::tab() << "EXCEPTION " << getName() << pos << " FOR " << *it << s::procendl(l);
}

void translateLinterExceptionDeclarations(Codestream &str, PlExpr *condition, DeclaredExceptions &declExc) {
  if (!condition)
    return;

  if (Exception* exc = condition->getExceptionDef()) {
    declExc[exc->getName()->toNormalizedString()].first = 0;
    if (condition->beginedFrom(1870,22))
      cout << "";
    exc->translateExceptionDeclarations(str, condition->getLLoc());
  }
}

void Sm::pl_expr::LogicalCompound::translateLinterExceptionDeclarations(Codestream &str, DeclaredExceptions &declExc) {
  Sm::translateLinterExceptionDeclarations(str, lhs.object(), declExc);
  Sm::translateLinterExceptionDeclarations(str, rhs.object(), declExc);
}

void Sm::WhenExpr::translateLinterExceptionDeclarations(Codestream &str, DeclaredExceptions &declExc) {
  if (condition) {
    if (condition->isPlLogicalCompound())
      condition->translateLinterExceptionDeclarations(str, declExc);
    else
      Sm::translateLinterExceptionDeclarations(str, condition.object(), declExc);
  }
}

void PlExpr::translateExceptionLiterDefinition(Codestream &str, WhenExpr *whenExpr) {
  Exception* exc = getExceptionDef();
  if (!exc) {
    throw 999;
    return;
  }
  std::vector<std::string> *trNames = 0;
  exc->translatedNames(&trNames);
  str << "WHEN" << s::name << exc->getName() << WhenExpr::Then();
  whenExpr->translateBody(str);
  if (trNames && trNames->size()) {
    int end = trNames->size() + 1;
    for (int pos = 2; pos < end; ++pos) {
      str << s::endl << s::tab() << "WHEN" << s::name << exc->getName() << pos << WhenExpr::Then();
      whenExpr->translateBody(str);
    }
  }
}

void Sm::pl_expr::LogicalCompound::translateExceptionLiterDefinition(Codestream &str, WhenExpr *whenExpr) {
  if (lhs)
    lhs->translateExceptionLiterDefinition(str, whenExpr);
  if (rhs)
    rhs->translateExceptionLiterDefinition(str, whenExpr);
}

void WhenExpr::checkCondition()
{
  if (!condition)
    throw 999;
}

void Sm::WhenExpr::linterDefinition(Codestream &str) {
  switch (cathegory) {
    case EXCEPTION_HANDLER:
      checkCondition();
      condition->translateExceptionLiterDefinition(str, this);
      return;
    case IF_FIRST_STATEMENT:
      checkCondition();
      str << "IF" << s::name << s::ocommalist() << condition << s::ccommalist() << Then();
      break;
    case ELSEIF_STATEMENT:
      checkCondition();
      if (condition->beginedFrom(765512))
        cout << "";
      str << "ELSEIF" << s::name << s::ocommalist() << condition << s::ccommalist() << Then();
      break;
    case ELSE_STATEMENT:
      str << "ELSE" << s::name;
      break;
    case WHEN_ALL:
      str << "WHEN ALL" << Then();
      break;
    case WHEN_OTHERS:
      str << "WHEN OTHERS" << Then();
      break;
    case WHEN_THEN:
      checkCondition();
      str << "WHEN" << s::name << s::ocommalist() << condition << s::ccommalist() << Then();
      break;
  }
  if (str.state().isModelStatistic || str.state().isErrorLog)
    return;
  translateBody(str);
}

bool Variable::isPackageVariable() const {
  if ((ownerBlock_ && !ownerBlock_->isNotPackageBlock_) || flags.isGlobal())
    return true;
  if (SemanticTree *p = this->getSemanticNode()->getParent())  // случай, когда это пакетная переменная.
    if (p->unnamedDdlEntity && p->unnamedDdlEntity->ddlCathegory() == ResolvedEntity::Package_)
      return true;

  return false;
}


void Sm::Variable::translateName(Codestream &str) {
  TranslatedName::translateName(str);
}


void Sm::Variable::linterReference(Sm::Codestream &str) {
  if (isAnydataCode())
    str << anydataCode_;
  else if (isPackageVariable()) {
    if (str.state().currentUser_ && str.state().currentUser_ != userContext()) {
      if (UserContext *cntx = userContext())
        str << s::linref(cntx) << ".";
      else
        cout << "error: variable has not parent user context " << getLLoc().locText() << endl;
    }
    translateName(str);
  }
  else if (!isTrNameEmpty())
    translateName(str);
  else {
    BlockPlSql *_ownerBlock = ownerPlBlock();
    if (!_ownerBlock)
      throw 999;
    Ptr<LevelResolvedNamespace> nameSpace = _ownerBlock->internalNamespace();
    if (!nameSpace && (flags.isGlobal() || isPackageVariable()))
      nameSpace = levelNamespace();
    str << name->toCodeId(nameSpace); // меняет translatedName при необходимости
  }
}


void Sm::BlockPlSql::addToInitializators(ResolvedEntity *var, Sm::PlExpr *expr) {
  LValue *lVal = new LValue(expr->getLLoc(), var->getName());
  Assignment *assign = new Assignment(expr->getLLoc(), lVal, expr);
  assign->setOwnerBlockPlSql(this);

  blockInitializators()->push_back(assign);
}

void Sm::Variable::definitionRecordExpandedAsFields(Sm::Codestream &str) {
  bool packageVar = isPackageVariable();
  EntityFields fields;
  datatype->getFields(fields);

  for (EntityFields::iterator it = fields.begin(); it != fields.end(); ++it) {
    Ptr<Datatype> datatype = (*it)->getDatatype()->getFinalType();

    if (packageVar) {
      Sm::Codestream header(str.state());
      header << "VARIABLE " << s::linref(this) << "_" << (*it)->toNormalizedString();
      str << s::ocmd(this, CmdCat::VARIABLE, header.str())
          << syntaxerContext.createStatement << "VARIABLE ";
    }
    else
      str << "VAR ";

    str << s::linref(this) << "_" << (*it)->toNormalizedString();

    str << s::name << datatype;
    if (next(it) != fields.end()) {
      str << s::procendl(getLLoc());
      if (!packageVar)
        str << s::tab(str.indentingLevel());
    }
    if (packageVar)
      str << s::ccmd;
  }
}


void Variable::definitionPackageVariable(ResolvedEntity *baseType, Sm::Codestream &str) {
  if (isAnydataCode())
    return;
  CodestreamState::ProcedureMode oldProcMode = str.procMode();

  if (name->toNormalizedString() == "MAINFIELDVALUE")
    cout << "";

  str.procMode(CodestreamState::SQL);

  if (!syntaxerContext.translateReferences && baseType && (baseType->isRecordType() || baseType->isRowTypeOf()))
    definitionRecordExpandedAsFields(str);
  else
  {
    Sm::Type::ObjectType *baseObj = 0;
    if ((baseObj = getObjectDatatype()))
      baseObj->translateVariableType(str, this, false);

    str << s::ocreate(this) << "VARIABLE " << s::cref(this) << s::name;

    if (baseObj)
      str << s::lindef(baseObj);
    else if (baseType) {
      str << s::sqldef(baseType);
      if (defaultValue_ && !defaultValue_->isNull()) {
        if (!hasDynamicDefaultValue())  {
          if (flags.isConstant()) {
            // Достанем значение константы
            if (Ptr<PlExpr> it = defaultValue_->unwrapConstantValue())
                if (!it->isNull())
                str << " = " << it;
          }
          else
            str << " = " << defaultValue_;
        }
      }
    }
    str << s::semicolon << s::name << s::loc(getLLoc());
    if (baseType)
      str << s::loc(baseType->getLLoc());
    str << s::ccreate;
  }


  str.procMode(oldProcMode);
}

bool Variable::isLinterStructType() const {
  if (baseField != NULL || toSelfVariableField())
    return false;
  Ptr<Datatype> baseType = ResolvedEntity::getLastUnwrappedDatatype(getDatatype());
  return baseType && baseType->isLinterStructType();
}

void Variable::definitionCursor(Sm::Codestream &str) {
  str << "VAR" << s::name;
  linterReference(str);
  str << s::name;
  Ptr<Datatype> baseType = ResolvedEntity::getLastUnwrappedDatatype(getDatatype());
  if (isLinterStructType())
    translateFieldsAsStruct(fields_, str);
  else
    translateFieldsAsCursor(fields_, str);
}


BlockPlSql * Variable::getOwnerBlock()
{
  BlockPlSql *blk = 0;
  if (SemanticTree *n = getSemanticNode())
    blk = n->ownerBlock();
  if (!blk)
    blk = ownerBlock_;

  return blk;
}

void Variable::definitionRefCursor(Sm::Codestream &str)
{
  if (beginedFrom(1520827))
    cout << "";
  BlockPlSql *blk = getOwnerBlock();
  if (blk) {
    str << "VAR" << s::name;
    linterReference(str);
    if (fields_.empty()) {
      str << s::name << "CURSOR" << s::otablevel(2) << s::OBracketView()
          << s::OMultiLineComment() << " FIXME: the cursor fields must be set manually "
          << s::CMultiLineComment() << s::CBracketView() << s::ctablevel();
      cerr << "CURSOR: the cursor fields must be set manually " << getLLoc() << endl;
//      throw 999;
    }

    else
      translateFieldsAsCursor(fields_, str);
  }
  else
    throw 999;
}

void Sm::Variable::linterDefinition(Sm::Codestream &str) {
  ResolvedEntity *baseType = NULL;
  Ptr<Datatype> _datatype;
  if ((_datatype = getDatatype()))
    baseType = ResolvedEntity::getLastUnwrappedDatatype(_datatype);

  str.state().dynamicCollection(isDynamicUsing());
  if ((flags.isGlobal() || isPackageVariable()) && !flags.isDynamicLoopCounter()) {
    definitionPackageVariable(baseType, str);
    str.state().dynamicCollection(false);
    return;
  }
  else if (fields_.size())
    definitionCursor(str);
  else if (!_datatype) {
      trError(str, s << "ERROR variable datatype is NULL on declaration: " << name->toNormalizedString() << ":"
                     << (ownerBlock_ ? ownerBlock_->getLLoc() : cl::filelocation()));
  }
  else if (_datatype->isRefCursor())
    definitionRefCursor(str);
  else if (baseType && baseType->isObjectType())
    str << "VAR" << s::name << s::linref(this) << s::name << _datatype;
  else if (buildCursorFields()) {
    definitionCursor(str);
  }
  else {
    str << "VAR" << s::name;
    linterReference(str);
    str << s::name << _datatype;
  }
  str.state().dynamicCollection(false);
  if (defaultValue_ && !isCursorVariable() && !hasDynamicDefaultValue()) // сложные значения по умолчанию переносятся в инициализаторы на стадии формальных преобразований
    str << s::name << "DEFAULT" << s::name << defaultValue_;
}

void Sm::VariableField::linterDefinition(Sm::Codestream &str) {
  if (syntaxerContext.unwrapStructuredFields && isStructuredField)
    throw 999;
  Variable::linterDefinition(str);
}

void Sm::VariableField::linterReference(Sm::Codestream &str) {
  Variable::linterReference(str);
  if (syntaxerContext.unwrapStructuredFields && isStructuredField)
    throw 999;
}


void Sm::insert::InsertingRecordValues::sqlDefinition(Sm::Codestream &str) {
  EntityFields f;
  str << s::OBracketCall();

  if (!str.state().queryForExecute_)
    throw 999;
  if (record)
    if (ResolvedEntity *def = record->refDefinition()) {
      def->getFields(f);
      bool isNotFirst = false;
      for (EntityFields::value_type &v : f) {
        IdEntitySmart ent;
        ent.reserve(record->refSize() + 1);
        ent.push_back(v);
        ent.insert(ent.end(), record->reference->begin(), record->reference->end());
        str << s::comma(&isNotFirst) << ent;
      }
    }
  str << s::CBracketCall();
}

bool Sm::SelectStatement::translatedAsExecuteWithoutDirect() const {
  Subquery::IntoList l = subquery->intoList();
  if (subquery->isBulkCollect()) {
    if (l->front()->reference->isDynamicUsing())
      return false; // copy collection
  }
  // as open for and execute - true (without direct)
  return true;
}

void Sm::SelectStatement::linterDefinition(Codestream &str) {
  if (beginedFrom(57640))
    cout << "";
  Subquery::IntoList l = subquery->intoList();
  if (subquery->isBulkCollect()) {
    if (l->front()->reference->isDynamicUsing()) {
      if (l->size() > 1)
        trError(str, s << "ERROR: Multiselect into different collections " << getLLoc()); //Необходимо доработать для поддержки мультиселекта в несколько коллекций одновременно.
      translateAsCopyCollection(str, *(l->front()->reference), subquery.object());
    }
    else {
      std::string cursorVarName;
      Ptr<Sm::Variable> cursorVar = ownerBlock->getTemporaryVar(str, subquery->getDatatype(), &cursorVarName);
      str << s::executeStatementPointer(this, str.state());
      translateAsOpenFor(str, cursorVar, subquery.object());
      str << s::procendl(getLLoc()) << s::tab();
      translateFetchIntoCollection(str, *l, cursorVar, NULL, getLLoc());
      str << s::procendl(getLLoc()) << s::tab();
      str << "CLOSE" << s::name << cursorVarName;
    }
  }
  else if (l && l->size() == 1 && l->front()->isCursorVariable()/* && !syntaxerContext.translateReferences*/) {
    str << s::executeStatementPointer(this, str.state());
    translateAsOpenFor(str, l->front()->getNextDefinition(), subquery.object());
  }
  else {
    str << s::otabcommalist();
    str << s::executeStatementPointer(this, str.state()) << "EXECUTE" << s::name;
    if (Sm::DynSubquery *dQ = subquery->toSelfDynSubquery())
      str << "DIRECT" << s::name << dQ->reference_;
    else {
      str.incIndentingLevel(6);
      translateSqlToString(*subquery, str, 0);
      str.decIndentingLevel(6);
    }


    str << s::ctabcommalist();
    if (l) {
      str << s::name << s::subconstruct << "INTO" << s::name;
      translateLValueList(str, *l, this);
    }
  }
}

void Sm::Savepoint::linterDefinition(Codestream &str) {
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE" << s::name << s::oquote << "SET SAVEPOINT " << savepoint << s::cquote;
}


void DeleteFrom::sqlDefinition(Sm::Codestream &str) {
  str << s::ocommalist();
  str << s::otablevel(2);
  str << "DELETE FROM " << name << s::name;
  if (alias)
    str << "AS " << alias << s::name;
  if (whereClause) {
    str << s::subconstruct << "WHERE " << s::ocommalist() << whereClause << s::ccommalist();
  }
  str << s::ctablevel();
  str << s::ccommalist();
}

void DeleteFrom::linterDefinition(Sm::Codestream &str) {
  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE" << s::name;
  translateSqlToString(*this, str, 0 /*SQLSTR_NEED_DIRECT*/);
  str << s::ctabcommalist();
  if (returnInto) {
    returnInto->setupQueryParams(ReturnInto::FOR_DELETE, name, alias, whereClause);
    returnInto->linterDefinition(str);
  }
}

void insert::SingleInsert::sqlDefinition(Sm::Codestream &str) {
  str << "INSERT" << s::name << into << s::name << data;
}

void insert::SingleInsert::linterDefinition(Sm::Codestream &str) {
  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE" << s::name;
  translateSqlToString(*this, str, 0 /*SQLSTR_NEED_DIRECT*/);
  str << s::ctabcommalist();
  if (Ptr<ReturnInto> returnInto = data->getReturning()) {
    returnInto->setupQueryParams(ReturnInto::FOR_INSERT, into->entity, NULL, NULL);
    returnInto->linterDefinition(str);
  }
}

void ReturnInto::setupQueryParams(QueryMode mode, Ptr<ChangedQueryEntity> entity, Ptr<Id> alias, Ptr<Sm::WhereClause> where) {
  qMode = mode;
  qEntity = entity;
  qAlias  = alias;
  qWhere  = where;
}

void ReturnInto::translateIntoList(Sm::Codestream &str) {
  str << s::name << "INTO" << s::name;
  str << s::otabcommalist();
  bool isNotFirst = false;
  for (auto &refExpr : *intoCollections) {
    str << s::comma(&isNotFirst) << refExpr;
  }
  str << s::ctabcommalist();
}

void ReturnInto::translateBulkCollections(Sm::Codestream &str) {
  BlockPlSql *ownerBlock = ownerPlBlock();
  if (!ownerBlock)
    throw 999;

  std::string cursorVarName;
  Ptr<Sm::Variable> cursorVar = ownerBlock->getTemporaryVar(str, getDatatype(), &cursorVarName);

  str << s::otabcommalist();
  str << "OPEN" << s::name << cursorVarName << " FOR ";
  translateSqlToString(*this, str, SQLSTR_SUPRESS_DIRECT);
  str << s::ctabcommalist();

  str << s::procendl(getLLoc()) << s::tab();
  translateFetchIntoCollection(str, *intoCollections, cursorVar, NULL, getLLoc());
  str << s::procendl(getLLoc()) << s::tab();
  str << "CLOSE" << s::name << cursorVarName;
}

void ReturnInto::sqlDefinition(Sm::Codestream &str) {
  if (!qEntity)
    throw 999; // Need call the setupQueryParams

  str << "SELECT" << s::name << *exprList << s::name << "FROM"
      << s::name << qEntity;
  if (qAlias)
    str << s::name << "AS " << qAlias;

  // FIXME: Для UPDATE с RETURNING BULK INTO сделано допущение, что поля из where не меняются в полях UPDATE списка.
  // Т.е. можно использовать тоже условие where для выборки необходимых полей
  if ((qMode == FOR_DELETE) || (qMode == FOR_UPDATE && intoCollections->isBulkCollect())) {
    if (qWhere)
      str << s::name << s::subconstruct << " WHERE " << s::ocommalist() << qWhere << s::ccommalist();
  }
  else
    str << s::name << "WHERE ROWID = LAST_ROWID";
}

void ReturnInto::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(675924))
    cout << "";
  if (!qEntity)
    throw 999; // Need call the setupQueryParams

  auto queryType = [](QueryMode mode) -> const char* {
    switch (mode) {
    case FOR_UPDATE : return "UPDATE";
    case FOR_DELETE : return "DELETE";
    case FOR_INSERT : return "INSERT";
    }
    return "";
  };

  str.levelPush();
  if (qMode == FOR_DELETE) {
    str.activatePrefixes();
    str << s::tab() << s::Comment() << "Returning stub of next " << queryType(qMode) << " query" << s::endl;
  }
  else {
    str.activatePostactions();
    str << s::tab() << s::Comment() << "Returning stub of prev " << queryType(qMode) << " query" << s::endl;
    str << s::tab() << "IF ROWCOUNT() > 0 THEN" << s::endl;
    str.incIndentingLevel(2);
  }

  if (intoCollections->isBulkCollect()) {
    str << s::tab() << s::Comment() << "(WITH COLLECTIONS)" << s::endl << s::tab();
    translateBulkCollections(str);
  }
  else {
    str << s::tab() << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
    translateSqlToString(*this, str, SQLSTR_SUPRESS_DIRECT);
    translateIntoList(str);
  }
  str << s::procendl(getLLoc());

  if (qMode != FOR_DELETE) {
    str.decIndentingLevel(2);
    str << s::tab() << "ENDIF" << s::endl;
  }

  str.activatePrevious();
  str.levelPop();
}


bool Sm::LValue::isVariable() const {
  if (ResolvedEntity *def = reference->definition())
    if (def->isVariable())
      return true;
  return false;
}

void Assignment::translateCallSignature(Codestream &str, RefExpr*simpleFunctionCall)
{
  str << "CALL " << simpleFunctionCall->reference << " INTO " << s::lindef(lValue);
  if (simpleFunctionCall->isNot())
    lValue->translateInversedForm(str);
}

void LValue::translateInversedForm(Sm::Codestream &str) {
  str << s::procendl(getLLoc()) << s::tab() << reference << " := not " << reference;
}

void Sm::Assignment::translateCollectionCtor(Codestream &str, RefExpr *simpleFunctionCall) {
  // Удалить все из коллекции
  str << "ARRAY_DELETE" << s::obracket << lValue->lEntity() << s::cbracket;
  Ptr<CallArgList> callArgs = simpleFunctionCall->callArglist();
  if (callArgs) {
    int i = 1;
    for (Ptr<FunCallArg> &arg : *callArgs) {
      str << s::procendl();
      str << s::tab() << lValue->lEntity() << "[" << (i++) << "]"
          << s::name << ":=" << s::name << arg;
    }
  }
}

void Sm::Assignment::linterDefinition(Codestream &str) {
  if (beginedFrom(78832))
    cout << "";

  Sm::Type::RefDecoder decoder;
  if (decoder.decode(str, *(lValue->lEntity()), assignedExpr))
    return;

  if (assignedExpr) {
    Datatype *rhsT = 0;
    Datatype *lhsT = 0;
    RefExpr*simpleFunctionCall = assignedExpr->toSimpleFunctionCall();

    if (ResolvedEntity *def = lValue->refDefinition())
      if (Ptr<Datatype> t = def->getDatatype())
        lhsT = ResolvedEntity::getLastConcreteDatatype(t);
    if (Ptr<Datatype> t = assignedExpr->getDatatype())
      rhsT = ResolvedEntity::getLastConcreteDatatype(t);

    if (rhsT && lhsT && simpleFunctionCall && (rhsT->isElementaryType() || rhsT->isCompositeType() || rhsT->isAnydata())) {
      Sm::Function *f = simpleFunctionCall->refEntity()->definition()->toSelfFunction();
      if (!f->isElementaryLinterFunction()) {
        if (!lhsT->getCastCathegory(rhsT, true).implicit())
          throw 999;

        ResolvedEntity *def = simpleFunctionCall->refEntity()->definition();
        if (!lValue->lEntity()->isDynamicUsing()) {
          if (def->isConstructor()) {
            if (def->owner()->isCollectionType()) {
              translateCollectionCtor(str, simpleFunctionCall);
              return;
            }
            else {
              // Добавим ссылку в начало списка аргументов
              simpleFunctionCall->refEntity()->pushFrontCallarglist(lValue->lEntity(), true);
            }
          }
        }
        translateCallSignature(str, simpleFunctionCall);
        return;
      }
    }
    else if (RefExpr *refExpr = assignedExpr->toSelfRefExpr())
      if (refExpr->reference->size() > 1 && refExpr->reference->majorObjectRef(NULL).valid() && lValue->lEntity()->isDynamicUsing()) {
        translateCallSignature(str, refExpr);
        return;
      }
  }

  str << lValue << s::name << ":=" << s::name << assignedExpr;
}



void Sm::LValue::linterDefinition(Codestream &str) {
  str << reference;
}

void Sm::LValue::linterReference(Sm::Codestream &str) {
  throw UnimplementedOperation("LValue::linterReference");
  str << reference;
}

void Sm::LValueIntoList::linterDefinition(Codestream &str) {
  if (fields.empty())
    throw 999;
  ResolvedEntity *fld = fields.front()->definition();
  if (!fld)
    throw 999;
  VariableField *f = fld->toSelfVariableField();
  if (!f)
    throw 999;
  VariableCursorFields *owner = f->getTopOwnerVariable();
  if (!owner)
    throw 999;

  Ptr<Id> ownerName = owner->toSelfResolvedEntity()->getName();
  bool isNotFirst = false;

  str << s::ocommalist();
  for(auto &v : fields)
    str << s::comma(&isNotFirst) << ownerName << '.' << s::linref(v->definition());
  str << s::ccommalist();
}

template <typename T>
void loopVariableDefinition(Codestream &str, T *var, Ptr<BlockPlSql> ownerBlock) {
  str.activateDeclarations();
  int indentingLevel = str.indentingLevel();
  str.indentingLevel(2);
  if (ownerBlock)
    var->getName()->toCodeId(ownerBlock->getSemanticNode()->childNamespace, true);
  // добавить переменную в пространство имен, и если она там уже есть - поменять ее имя.

  str << s::tab() << s::lindef(var) << s::procendl(var->getLLoc());
  str.indentingLevel(indentingLevel);
  str.activatePrevious();
}

void Sm::Variable::generateUniqueName() {
  if (!isTrNameEmpty())
    return;
  ResolvedEntity::translatedName(name->toCodeId(ownerBlock_->getSemanticNode()->levelNamespace));
}



void Sm::ForOfExpression::linterDefinition(Codestream &str) {
  if (beginedFrom(40660))
    cout << "";
  indexVariableDefinition->setOwnerBlockPlSql(ownerBlock);
  loopVariableDefinition(str, indexVariableDefinition.object(), ownerBlock->getTopBlock());
  if (isReverse)
    throw 999; // это нужно реализовать
  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state());
  str << "OPEN" << s::name << s::ref;
  str << indexVariableDefinition;
  str << s::def << s::name << "FOR" << s::name;
  translateSqlToString(*sqlExpression, str, 0 /*SQLSTR_NEED_DIRECT*/);
  str << s::ctabcommalist();
  str << s::procendl(getLLoc());
  if (str.state().isModelStatistic || str.state().isErrorLog)
    return;
  str << s::tab() << "WHILE not outofcursor" << s::obracket;
  str << s::ref;
  str << indexVariableDefinition;
  str << s::def << s::cbracket << s::name << "LOOP" << s::name;
  if (loop && outStatements_) {
    str <<  s::endl;
    if (loop->getOwnerBlock() != ownerBlock)
      throw 999;
    loop->outStatements(str);
  }
  str << inctab(2);
  str << s::tab() << "FETCH " << s::ref << indexVariableDefinition << s::def;
  if (str.state().indentingLongConstruct_)
    str << s::procendl();
  str << dectab(2);
  str << s::tab() << "ENDLOOP";
  loop->outEndLoopLabel(str);
  if (str.state().indentingLongConstruct_)
    str << s::endl;
  str << s::tab() << "CLOSE" << s::name;
  str << s::ref;
  str << indexVariableDefinition;
  str << s::def << s::semicolon << " " << s::loc(getLLoc());
}

static void outEndlTab(Sm::Codestream &str, bool &isFirst)
{
  if (isFirst)
    isFirst = false;
  else
    str << s::endl << s::tab();
}

class IntoListTranslator {
public:
  IntoListTranslator() {;}
  IntoListTranslator(List<RefAbstract> &intoList, Ptr<RefExpr> cursorRef) { init(intoList, cursorRef); }

  void init(List<RefAbstract> &intoList, Ptr<RefExpr> cursorRef);
  void translateAssign(Sm::Codestream &str);
  void translateFetchInto(Sm::Codestream &str, Ptr<RefExpr> cursorRef, bool trInto);

private:
  Ptr<SqlExpr> createExprWithFld(Ptr<RefAbstract> oldRef, Ptr<Id> field);

  typedef List<SqlExpr> RefList;
  RefList target;
  RefList source;
  bool    needCast;
};

Ptr<SqlExpr> IntoListTranslator::createExprWithFld(Ptr<RefAbstract> oldExpr, Ptr<Id> field) {
  Ptr<IdEntitySmart> ref = new IdEntitySmart(*oldExpr->reference, field);
  Ptr<SqlExpr> newExpr = new RefExpr(oldExpr->getLLoc(), ref);
  syntaxerContext.model->delayDeletedExpressions.push_back(oldExpr.object());
  syntaxerContext.model->delayDeletedExpressions.push_back(newExpr.object());

  SemanticTree *node = new SemanticTree(ref.object(), SemanticTree::REFERENCE, SCathegory::RefAbstract);
  node->unnamedDdlEntity = newExpr.object();
  node->setRecursiveFlags(FLAG_SEMANTIC_TREE_IS_PL_CONTEXT);
  newExpr->setSemanticNode(node);
  oldExpr->getSemanticNode()->addChildForce(node);
  return newExpr;
}

void IntoListTranslator::init(List<RefAbstract> &intoList, Ptr<RefExpr> cursorRef) {
  EntityFields curFields;
  cursorRef->getFields(curFields);
  List<RefAbstract>::iterator intoIt = intoList.begin();
  EntityFields::iterator curIt = curFields.begin();

  needCast = false;
  target.clear();
  source.clear();

  if (intoList.size() == 1) {
    if ((*intoIt)->beginedFrom(176270))
      cout << "";
    Datatype *lhType = Datatype::getLastConcreteDatatype((*intoIt)->getDatatype());
    if (lhType && lhType->isCompositeType()) {
      Datatype *rhType = Datatype::getLastConcreteDatatype(cursorRef->getDatatype());
      CastCathegory cat = lhType->getCastCathegory(rhType, true, false);
      if (cat.equally() || cat.implicit()) {
        // Структуры идентичные по полям
        target.push_back((*intoIt).object());
        source.push_back(cursorRef.object());
      }
      else {
        EntityFields targetFields;
        (*intoIt)->getFields(targetFields);

        if (targetFields.size() != curFields.size()) {
          Codestream s;
          s << "ERROR in IntoListTranslator: " << s::iloc(cursorRef->getLLoc()) << " cursor " << cursorRef
            << " and target " << (*intoIt) << " has different fields" << s::endl;
          cout << s.str();
          return;
        }

        EntityFields::iterator tarIt = targetFields.begin();
        for (; tarIt != targetFields.end() && curIt != curFields.end(); ++tarIt, ++curIt) {
          Datatype *lhType = Datatype::getLastConcreteDatatype((*tarIt)->getDatatype());
          Datatype *rhType = Datatype::getLastConcreteDatatype((*curIt)->getDatatype());

          Ptr<SqlExpr> curExpr = createExprWithFld(cursorRef.object(), *curIt);
          CastCathegory cat = lhType->getCastCathegory(rhType, true, false);
          if (cat.explicitAll()) {
            needCast = true;
            cat.setProcCastState();
            cat.setCastAssignment();
            CommonDatatypeCast::castAndReplace(true, curExpr, rhType, lhType, cat);
          }

          Ptr<SqlExpr> tarExpr = createExprWithFld(*intoIt, *tarIt);
          target.push_back(tarExpr.object());
          source.push_back(curExpr.object());
        }
      }
      return;
    }
  }

  for (; intoIt != intoList.end() && curIt != curFields.end(); ++intoIt, ++curIt) {
    Datatype *lhType = Datatype::getLastConcreteDatatype((*intoIt)->getDatatype());
    Datatype *rhType = Datatype::getLastConcreteDatatype((*curIt)->getDatatype());

    Ptr<SqlExpr> curExpr = createExprWithFld(cursorRef.object(), *curIt);
    CastCathegory cat = lhType->getCastCathegory(rhType, true, false);
    if (cat.explicitAll()) {
      needCast = true;
      cat.setProcCastState();
      cat.setCastAssignment();
      CommonDatatypeCast::castAndReplace(true, curExpr, rhType, lhType, cat);
    }

    target.push_back((*intoIt).object());
    source.push_back(curExpr.object());
  }
}

void IntoListTranslator::translateAssign(Sm::Codestream &str) {
  bool isFirst = true;
  RefList::iterator tIt = target.begin();
  RefList::iterator sIt = source.begin();
  for (; tIt != target.end() && sIt != source.end(); ++tIt, ++sIt) {
    outEndlTab(str, isFirst);
    str << (*tIt) << s::name << ":=" << s::name << (*sIt) << s::semicolon;
  }
}

void IntoListTranslator::translateFetchInto(Sm::Codestream &str, Ptr<RefExpr> cursorRef, bool trInto) {
  bool isNotFirst = false;
  str << "FETCH " << s::ref << cursorRef << s::def;
  if (!target.size() || !trInto)
    return;

  if (needCast) {
    str << s::procendl(cursorRef->getLLoc()) << s::tab();
    translateAssign(str);
  }
  else {
    str << " INTO" << s::name << s::ocolumn();
    for (RefList::iterator tIt = target.begin(); tIt != target.end(); ++tIt) {
      str << s::comma(&isNotFirst) << (*tIt);
    }
    str << s::ccolumn();
  }
}

void Fetch::trFetchInto(bool isInsertCollection, Sm::Codestream &str)
{
  str << "FETCH " << s::ref << cursorRef << s::def;
  if (!isInsertCollection && !fields.empty()) {
    str << " INTO" << s::name << s::ocolumn();
    translateLValueList(str, fields, this);
    str << s::ccolumn();
  }
}

void Fetch::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(648464))
    cout << "";
  if (str.state().isModelStatistic) {
    trFetchInto(false, str);
    return;
  }
  if (intoCathegory == COLLECTION) {
    // Способ 2.
    // Может также транслироваться в операторе Open, OpenFor в другую форму.
    translateFetchIntoCollection(str, fields, cursorRef->refDefinition(), limit, getLLoc());
  }
  else {
    if (limit)
      throw 999;

    IntoListTranslator tr(fields, cursorRef);

    bool isInsertCollection = (fields.size() == 1 && fields.front()->reference->majorObjectRef(NULL) && fields.front()->reference->isDynamicUsing());
    BlockPlSql *block = getOwnerBlock()->getTopBlock();
    ResolvedEntity *cursor = cursorRef->refDefinition();
    Ptr<Variable> locker = block->getCreateCursorLocker(str, cursor);

    str << "IF NOT " << s::linref(locker) << " THEN" << s::endl;
    str.incIndentingLevel(2);
    str << s::tab() << s::linref(locker) << " := TRUE" << s::procendl();
    if (!isInsertCollection && fields.size()) {
      str << s::tab();
      tr.translateAssign(str);
      str << s::endl;
    }
    str.decIndentingLevel(2);

    str << s::tab() << "ELSE" << s::endl;
    str.incIndentingLevel(2);
    str << s::tab();
    tr.translateFetchInto(str, cursorRef, !isInsertCollection);
    str << s::procendl(getLLoc());
    str.decIndentingLevel(2);
    str << s::tab() << "ENDIF";

    if (isInsertCollection) {
      str << s::procendl();
      str << s::tab() << "IF not outofcursor" << s::obracket << s::ref << cursorRef << s::def << s::cbracket << " THEN" << s::endl;
      str.incIndentingLevel(2);
      str << s::tab();
      translateCursorAsInsertCollection(str, *(fields.front()->reference), cursorRef->refDefinition());
      str.decIndentingLevel(2);
      str << s::tab() << "ENDIF";
    }
  }
}

void Sm::Exit::linterDefinition(Sm::Codestream &str) {
  if (!outStatements_) {
    str << "IF " << when << " THEN BREAK; ENDIF";
    return;
  }

  if (when) {
    str << "IF " << when << " THEN" << s::endl;
    str << inctab(2);
    str << s::tab();
  }
  /*
  str << "GOTO END_LOOP_LABEL" << ownerBlock->labelCounter_;
  if (Ptr<Sm::Loop> curLoop = ownerBlock->getActiveLoop())
    curLoop->endloopLabel = true;
  */
  str << "BREAK";
  if (when) {
    str << s::procendl(getLLoc());
    str << dectab(2);
    str << s::tab() << "ENDIF";
  }
}

void Sm::Variable::outAsForLoopVariable(Sm::Codestream &str, Ptr<LoopBounds> bounds, bool isReverse, BlockPlSql *ownerBlock) {
  flags.setDynamicLoopCounter();
  setOwnerBlockPlSql(ownerBlock);
  ownerBlock = ownerBlock->getTopBlock();
  Ptr<Declaration> decl = ownerBlock->hasDeclaration(name);
  if (!decl || !decl->getDatatype() || !decl->getDatatype()->isNum()) {
    //generateUniqueName();
    loopVariableDefinition(str, this, ownerBlock);
  }
  Ptr<SqlExpr> lBound = isReverse ? bounds->upperBound : bounds->lowerBound;
  Ptr<SqlExpr> uBound = isReverse ? bounds->lowerBound : bounds->upperBound;
  string cycleCond = isReverse ? " >= " : " <= ";

  str << "FOR ";
  linterReference(str);
  str << " := ";
  str << lBound;
  str << " WHILE ";
  linterReference(str);
  str << cycleCond << uBound;
  str << " BY ";
  linterReference(str);
  str << " := ";
  linterReference(str);
  str << (isReverse ? " - 1" : " + 1");
}


void Sm::ForOfRange::linterDefinition(Sm::Codestream &str) {
  if (Ptr<Datatype> t = bounds->lowerBound->getDatatype()) {
    if (t->isCharVarchar())
      trError(str, s << "WARN: ForOfRange bound is varchar type " << getLLoc());
  }
  else {
    cout << "error: unresolved lower bound For Of " << bounds->lowerBound->getLLoc() << endl;
  }
  idxVar->outAsForLoopVariable(str, bounds, isReverse, ownerBlock);
  str << " LOOP" << s::endl;
  if (loop && outStatements_) {
    if (loop->getOwnerBlock() != ownerBlock)
      throw 999;
    loop->outStatements(str);
  }
//  str << s::procendl(getLLoc());

  if (!SkipNameIters::previousSignificantSpacerIsEndl(str))
    str << s::endl;
  str << s::tab() << "ENDLOOP";
  loop->outEndLoopLabel(str);
}

void ForAll::linterDefinition(Sm::Codestream &str) {
  if (bounds->lowerBound->getDatatype()->isCharVarchar())
    trError(str, s << "WARN: ForAll bound is varchar type " << getLLoc());
  idxVar->outAsForLoopVariable(str, bounds, false, ownerBlock);
  str << " LOOP" << s::endl;
  str << inctab(2);

  if (sqlStatement->getOwnerBlock() != ownerBlock)
    throw 999;
  if (outStatements_)
    StatementsTranslator::translateStatement(str, sqlStatement.object(), ownerBlock);
  str << dectab(2);
  if (!SkipNameIters::previousSignificantSpacerIsEndl(str))
    str << s::endl;
  str << s::tab() << "ENDLOOP";
}

void Sm::While::linterDefinition(Sm::Codestream &str) {
  str << "WHILE " << condition << " LOOP" << s::name;
  if (loop && outStatements_) {
    str << s::endl;
    if (loop->getOwnerBlock() != ownerBlock)
      throw 999;
    loop->outStatements(str);
  }
  str << s::tab() << "ENDLOOP";
  loop->outEndLoopLabel(str);
}

void Loop::outStatements(Codestream &str, bool conditionNotOutput)
{
  if (loopStatements.empty())
    return;
  str << inctab(2);
  ownerBlock->pushLoop(this);
  Statements::iterator it = loopStatements.begin();
  if (!conditionNotOutput)
    ++it;
  for (; it != loopStatements.end(); ++it) {
    if ((*it)->toSelfExit() != nullptr)
      endloopLabel = true;
    StatementsTranslator::translateStatement(str, it->object(), ownerBlock);
  }
  ownerBlock->popLoop(this);
  str << dectab(2);
}

void Loop::outEndLoopLabel(Codestream &str) {
  if (endloopLabel) {
    BlockPlSql *topBlock = ownerBlock->getTopBlock();
    str << s::endl << s::tab() << "END_LOOP_LABEL" << topBlock->labelCounter_ << ":";
    ++(topBlock->labelCounter_);
  }
}

void Loop::linterDefinition(Sm::Codestream &str) {
  str << "WHILE ";
  bool conditionNotOutput = true;
  if (loopStatements.size())
    if (Ptr<PlExpr> p = loopStatements.front()->exitWhen())  {
      str << "NOT " << s::obracket << p << s::cbracket << " LOOP" << s::endl;
      conditionNotOutput = false;
    }
  if (conditionNotOutput)
      str << "TRUE LOOP" << s::endl;
  outStatements(str, conditionNotOutput);
  str << s::tab() << "ENDLOOP";
  outEndLoopLabel(str);
}

void If::translateBranch(std::string condCmd, Ptr<WhenExpr> branch, Codestream &str)
{
  str << s::def;
//  branch->ownerBlock = ownerBlock.object();
  str << condCmd << s::name << branch->condition << WhenExpr::Then();
  if (outStatements_)
    branch->translateBody(str);
  str << s::tab(str.indentingLevel());
}

void Sm::StatementInterface::translateAsListItem(Codestream &str) {
   str << s::tab();
   this->translate(str);
   if (needSemicolon())
     str << s::procendl(getLLoc());
   else
     str << s::endl;
}

void If::linterDefinition(Codestream &str)  {
  for (Statements::iterator it = branches.begin(); it != branches.end(); ++it) {
    str << s::tab(str.indentingLevel());
    (*it)->translate(str); // TODO: в стейтментах писать в префиксы а не в preactions
    if (str.state().isModelStatistic || str.state().isErrorLog)
      return;
//    str << s::endl;
//    translateStatement(str, it->object(), ownerBlock);
  }
  str << s::tab(str.indentingLevel()) << "ENDIF";
}

bool CaseStatement::whenListConditionComplex() const
{
  for (BaseList<StatementInterface>::const_iterator it = whenList.begin(); it != whenList.end(); ++it) {
    RefAbstract      *ref;
    ResolvedEntity *def;
    WhenExpr       *whenExpr;
    Variable       *var;
    if ((whenExpr = (*it)->toSelfWhenExpr()) == NULL || !whenExpr->condition)
      continue;
    if (whenExpr->condition->isComplexExpr())
      return true;
    if (whenExpr->condition->isNull())
      return true;
    if ((ref = whenExpr->condition->toSelfRefAbstract()) && (def = ref->refDefinition()) &&
        (var = def->toSelfVariable()) && (!var->flags.isConstant() || !var->isSystem()))
        return true;
  }
  return false;
}

bool CaseStatement::caseOperandIsBooleanLiteral() const
{
  RefAbstract *caseExpr;
  ResolvedEntity *def;

  if (caseOperand && (caseExpr = caseOperand->toSelfRefAbstract()) && (def = caseExpr->refDefinition())) {
    if (Sm::BooleanLiteral *literal = def->toSelfBooleanLiteral()) {
      if (literal->isTrue)
        return true;
      else
        throw 999; // надо разобраться что с таким делать
    }
  }

  return false;
}

bool CaseStatement::inExceptionSection() const {
  for (SemanticTree *node = getSemanticNode(); node != NULL; node = node->getParent())
    if (node->cathegory == SCathegory::Function)
      return false;
    else if (node->cathegory == SCathegory::ExceptionSection)
      // Кейс нельзя использовать в контексте обработки исключений в Линтере
      return true;
  return false;
}

void Sm::CaseStatement::linterDefinition(Sm::Codestream &str) {
  str << "CASE " << caseOperand << s::endl;
  for (BaseList<StatementInterface>::iterator it = whenList.begin(); it != whenList.end(); ++it)
    StatementsTranslator::translateStatement(str, *it, ownerBlock);
  str << s::tab() << "ENDCASE";
}

void ExecuteImmediate::linterDefinition(Sm::Codestream &str) {
  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
  str.incIndentingLevel(6);
  if (execExpr->isSubquery()) {
    translateSqlToString(*execExpr->toSelfSubquery(), str, 0 /*SQLSTR_NEED_DIRECT*/);
  }
  else if (execExpr->isQuotedSqlExprId())
    str << execExpr;
  else
    str << "DIRECT " << execExpr;
  str << s::ctabcommalist();
  str.decIndentingLevel(6);

  if (intoVars && !intoVars->empty()) {
    str << s::otablevel(2) << s::name << s::subconstruct << "INTO" << s::name;
    translateLValueList(str, *intoVars, this);
    str << s::ctablevel();
  }

  //TODO : Реализовать поддержку ReturnInto
}


void Sm::Merge::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(5293,5))
    cout << "";
  str << s::otabcommalist();
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
  str.incIndentingLevel(6);
  translateSqlToString(*this, str, 0);
  str.decIndentingLevel(6);
  str << s::ctabcommalist();
}

void Sm::Merge::sqlDefinition(Sm::Codestream &str) {
  str << s::ocommalist();
  str << s::otablevel(2);
  str << "MERGE INTO" << s::name << mergedEntity << s::name;
  str << s::subconstruct;

  str << s::ocommalist()
      << "USING" << s::name << usingClause << s::name;

  str << s::subconstruct << "ON" << s::name << onCondition << s::name
      << s::ccommalist();

  if (matchedUpdate) {
    str << s::subconstruct;
    str << s::ocommalist();
    str << "WHEN MATCHED THEN UPDATE SET" << s::name
        << s::subconstruct;
    bool isNotFirst = false;
    for (List<MergeFieldAssignment>::value_type &v : *(matchedUpdate->fieldsAssignments))
      str << s::comma(&isNotFirst) << v->field << " = " << v->assignment;
    if (matchedUpdate->where)
      trError(str, s << "ERROR: where clause in MERGE WHEN MATHCHED UPDATE is not supported: " << matchedUpdate->where->getLLoc());
    if (matchedUpdate->deleteWhere)
      trError(str, s << "ERROR: delete where clause in MERGE WHEN MATHCHED UPDATE is not supported: " << matchedUpdate->deleteWhere->getLLoc());
    str << s::ccommalist();
  }
  if (notMatchedInsert) {
    str << s::subconstruct;
    str << s::ocommalist();
    str << "WHEN NOT MATCHED THEN INSERT" << s::name;
    if (notMatchedInsert->fields)
      str << notMatchedInsert->fields << s::name;
    str << s::subconstruct << "VALUES" << s::name << notMatchedInsert->values;
    if (notMatchedInsert->where)
      trError(str, s << "ERROR: where clause in MERGE WHEN NOT MATHCHED INSERT is not supported: " << notMatchedInsert->where->getLLoc());
    str << s::ccommalist();
  }
  str << s::ctablevel();
  str << s::ccommalist();
}


void Update::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(1472746))
    cout << "";
  str << s::otabcommalist();
  str.incIndentingLevel(6);
  str << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
  translateSqlToString(*this, str, 0 /*SQLSTR_NEED_DIRECT*/);
  str.decIndentingLevel(6);
  str << s::ctabcommalist();
  if (returnInto) {
    returnInto->setupQueryParams(ReturnInto::FOR_UPDATE, entity, alias, where);
    returnInto->linterDefinition(str);
  }
}

void Update::sqlDefinition(Sm::Codestream &str) {
  str << s::ocommalist();
  if (beginedFrom(94931))
    cout << "";
  str << "UPDATE " << entity;
  if (alias)
    str << s::name << alias;
  setClause->updatedEntity = entity;
  setClause->updatedAlias = alias;
  str << s::otablevel(2);
  str << s::name << s::subconstruct << "SET" << s::name << s::ocommalist() << setClause << s::ccommalist();
  if (where)
    str << s::name << s::subconstruct << "WHERE" << s::name << s::ocommalist() << where << s::ccommalist();
  str << s::ctablevel();

  if (entity)
    if (ResolvedEntity *def = entity->getNextDefinition())
      if (def->ddlCathegory() == ResolvedEntity::Table_ || def->ddlCathegory() == ResolvedEntity::View_)
        def->checkToGrantOtherUser(Privs::UPDATE, str);
  str << s::ccommalist();
}

void update::FieldFromExpr::sqlDefinition(Sm::Codestream &str) { str << field << " = " << expr; }

void insert::Into::sqlDefinition(Sm::Codestream &str) {
  str << "INTO" << s::name << entity ;

  if (fields && !fields->empty()) {
    str << s::name
        << s::OBracketInsert();

    bool isNotFirst = false;
    for (Ptr<SqlExpr> &it : *fields) {
      if (RefAbstract *ref = it->toSelfRefAbstract()) {
        if (RefExpr *ex = ref->toSelfRefExpr())
          str << s::comma(&isNotFirst) << ex->reference->entity();
        else
          throw 999;
      }
      else
        str << s::comma(&isNotFirst) << it;
    }
    str << s::CBracketInsert();
  }
}

void insert::InsertFromValues::sqlDefinition(Sm::Codestream &str) {
  str << "VALUES" << s::name << value;
}

void insert::InsertFromSubquery::sqlDefinition(Sm::Codestream &str) {
  Subquery *q = subquery;
  if (SelectBrackets *br = q->toSelfSelectBrackets())
    q = br->subquery;
  str << s::OBracketInsert() << *q << s::CBracketInsert();
}

std::string Sm::Trigger::modeToString() const {
  switch (mode) {
    case trigger::M_EMPTY:    throw 999;
    case trigger::BEFORE:     return "BEFORE";
    case trigger::AFTER:      return "AFTER";
    case trigger::INSTEAD_OF: return "INSTEAD OF";
    case trigger::FOR:        throw 999;
  }
  throw 999;
  return "";
}


void Sm::Trigger::linterReference(Sm::Codestream &str) { str << name; }


void Sm::trigger::DmlEvents::linterDefinition(Sm::Codestream &str) {
  str << dmlEvents << " ON " << tableRef;
  if (isForEachRow)
    str << " FOR EACH ROW";
  for (int i = 0; i < 3; ++i)
    if (pseudorecords[i])
      str << pseudorecords[i];

  if (fieldNestedTable)
    throw 999;
}

}

namespace Sm {
extern int __longLineLimit__;
}

void ModelContext::userInitializer(Sm::Codestream &str) {
  Sm::UserEntitiesMap initializersContainer;
  collectInitializers(initializersContainer);

  int oldLongValue = Sm::__longLineLimit__;
  __longLineLimit__ = 160;

  Codestream::mainStream = &str;

  for (auto &usrCntx : initializersContainer)
    if (!usrCntx.first->isSystem() && !usrCntx.second.empty())
      usrCntx.first->userInitializer(usrCntx.second, str);

  str.store(syntaxerContext.initializersFilename);

  __longLineLimit__ = oldLongValue;
}


void UserContext::userInitializer(EntitiesSet &container, Codestream &str) {
  str.procMode(CodestreamState::SQL);

  str << s::connect(this);

  vector<Sm::Package*> packagesWithInitializerCode;
  vector<Sm::Variable*> variablesWithInitializators;

  for (EntitiesSet::value_type n : container) {
    if (!n)
      continue;
    if (Sm::Variable *var = n->toSelfVariable()) {
      if (var->beginedFrom(178897))
          cout << "";
      if (var->flags.isGlobal() || var->isPackageVariable()) {
        var->linterDefinition(str);
        if (var->hasDynamicDefaultValue() || var->hasObjectType())
          variablesWithInitializators.push_back(var);
        str << s::endl;
      }
      else if (Sm::Type::ObjectType *t = var->getObjectDatatype())
        t->translateVariableType(str, var, false);
    }
    else if (Sm::Package *p = n->toSelfPackage()) {

      packagesWithInitializerCode.push_back(p);
    }
    else
      throw 999;
  }

  vector<string> packagesNames;
  for (Sm::Package *v : packagesWithInitializerCode) {
    if (v->initializerEmpty())
      continue;
    string n = v->getInitializerName();
    packagesNames.push_back(n);

    Codestream headStr(str.state());
    headStr << "PROCEDURE " << s::linref(this) << '.' << n;

    string uniqueHeader = headStr.str();
    if (uniqueHeader == "PROCEDURE M2_ALL.ABS_RESERV_PKG_INIT")
      cout << "";

    str << s::ocmd(this, CmdCat::PROC, uniqueHeader)
        << syntaxerContext.createStatement
        << "PROCEDURE " << s::linref(this) << '.' << n << "()" << " for debug" << s::endl;
    if (n == "F_RESERVE_PKG_INIT")
      cout << "";
    str.procMode(CodestreamState::PROC);

    str.levelPush();
    v->userInitializer(str);
    str.levelPop();

    str.procMode(CodestreamState::SQL);

    str << s::procendl();
    str << s::ccmd;
    str << s::grant(username->toNormalizedString() + '.' + n, Privs::EXECUTE, syntaxerContext.model->modelActions.scenarioActorUsers);
  }

  str.levelPush();
  Codestream headStr(str.state());
  headStr << "PROCEDURE " << s::linref(this) << ".INIT_ENTITIES_PROC()";

  str << s::ocmd(this, CmdCat::PROC, headStr.str())
      << syntaxerContext.createStatement
      << "PROCEDURE " << s::linref(this) << ".INIT_ENTITIES_PROC() for debug" << s::endl;

  str.procMode(CodestreamState::PROC);

  str.levelPush();
  str << "CODE" << s::endl;
  str << inctab(2);

  for (auto &n : packagesNames)
    str << s::tab() << "CALL " << s::linref(this) << '.' << n << "()" << s::procendl();

  if (!str.getCurrentLevel()->declarations.empty()) {
    str.joinDeclarations();
    str.activateDeclarations();
    str << "DECLARE" << s::endl;
    str.activatePrevious();
    str.joinDeclarations();
  }
  str << dectab(2)
      << "END" << s::procendl() << s::endl;
  str.levelPop();
  str.procMode(CodestreamState::SQL);

  if (!str.getCurrentLevel()->actions.empty()) {
    str.activatePreactions();
    str << s::endl << s::endl;
    str.activatePrevious();
    str.joinPreactions();
  }
  str.levelPop();
  str << s::ccmd;
  str << s::grant(username->toNormalizedString() + ".INIT_ENTITIES_PROC", Privs::EXECUTE, syntaxerContext.model->modelActions.scenarioActorUsers);
}


bool Sm::Package::initializerEmpty() {
  if (bodies.empty())
    return true;
  if (Ptr<BlockPlSql> b = bodies.front()) {
    if (b->initializators.empty() && b->exceptionHandlers.empty()) {
      if (b->statements.empty())
        return true;
      else if (b->statements.size() == 1)
        if (Statement* stmt = b->statements.front()->toSelfStatement())
          if (stmt->toSelfNullStmt())
            return true;
    }
  }
  else
    return true;
  return false;
}

void Sm::Package::userInitializer(Sm::Codestream &str) {
  SemanticTree *n = 0;
  if (bodies.size())
    if (Ptr<BlockPlSql> b = bodies.front()) {
      if (!b->getSemanticNode()) {
        n = new SemanticTree(SCathegory::BlockPlSql);
        n->levelNamespace = new LevelResolvedNamespace(0, n);
        b->setSemanticNode(n);
      }
      b->lindefCodeBlock(str);
      if (n) {
        b->setSemanticNode(0);
        n->levelNamespace->semanticLevel = 0;
        delete n;
      }
      return;
    }
  throw 999;
}


void Sm::Variable::userInitializer(Sm::Codestream &str, bool &somethingOutputted) {
  if (defaultValue_) {
    somethingOutputted = true;
    str << s::tab();
    Ptr<Assignment> assignment = new Assignment(getLLoc(), new LValue(getLLoc(), name), defaultValue_);
    assignment->linterDefinition(str);
    str << s::procendl(getLLoc());
  }
}


void UserContext::storeKeysActions(Codestream &str) {
  str << s::connect(this);
  for (Tables::value_type &v : tables)
    v.second->linterDefinitionKeys(str);
}

namespace Sm {


void Sm::FunctionCall::linterDefinition (Sm::Codestream &str) {
  if (beginedFrom(1519707))
    cout << "";
  if (ResolvedEntity *d = funCall->getNextDefinition()) {
    RefExpr *r;
    if (d->isElementaryLinterFunction() || (d->isCollectionMethod() && (r = funCall->toSelfRefExpr()) && !r->reference->isDynamicUsing())) {
      str << funCall;
      return;
    }
  }
  str.state().isCallOperator_ = true;
  str << "CALL" << s::name << funCall;
  str.state().isCallOperator_ = false;
}


void Sm::Resignal::linterDefinition(Sm::Codestream &str) { str << "RESIGNAL";  }

void Sm::Raise::linterDefinition(Sm::Codestream &str) {
  if (BlockPlSql *b = ownerPlBlock()) {
    if (!b)
      throw 999;
    b = b->getTopBlock();
    if (ResolvedEntity *d = exception->definition()) {
      if (Exception *exc = d->getExceptionDef()) {
        b->exceptionFromOperators[exc->getName()->toNormalizedString()] = make_pair(exc, getLLoc());
      }
      else
        throw 999;
    }
    else
      throw 999;
  }

  str << "SIGNAL " << exception;
}

void RefExpr::setDatatype(Ptr<Sm::Datatype> t) {
  if (refSize() > 1)
    if (Ptr<Id> mEnt = reference->majorEntity())
      if (ResolvedEntity *var = mEnt->unresolvedDefinition()) {
        ResolvedEntity::ScopedEntities cat = var->ddlCathegory();
        if (cat == ResolvedEntity::Variable_ ||
            cat == ResolvedEntity::FunctionArgument_) {
          var->setDatatypeForMember(refEntity(), t);
          return;
        }
        else if (cat == ResolvedEntity::CollectionMethod_ ||
                 cat == ResolvedEntity::VariableUndeclaredIndex_)
          return; // TODO: обработку этого случая нужно реализовать
      }
  if (ResolvedEntity *d = refDefinition())
    d->setDatatype(t);
}

void Sm::Variable::setDatatypeForMember(Ptr<Sm::Id> name, Ptr<Sm::Datatype> t) {
  if (fields_.empty())
    datatype->getFields(fields_);
  for (EntityFields::iterator it = fields_.begin(); it != fields_.end(); ++it) {
    if ((*it)->toNormalizedString() == name->toNormalizedString())
      (*it)->definition(t.object());
  }
}

void Sm::FunctionArgument::setDatatypeForMember(Ptr<Sm::Id> name, Ptr<Sm::Datatype> t) {
  if (fields_.empty())
    datatype->getFields(fields_);
  for (EntityFields::iterator it = fields_.begin(); it != fields_.end(); ++it) {
    if ((*it)->toNormalizedString() == name->toNormalizedString())
      (*it)->definition(t.object());
  }
}



bool Sm::LockTable::translatedAsExecuteWithoutDirect() const { return true; }

void Sm::LockTable::linterDefinition(Sm::Codestream &str) {
  if (!tables)
    return;
  bool notFirst = false;
  for (Ptr<Id2> &tbl : *tables) {
    currentTbl = tbl;
    if (notFirst) {
      str << s::procendl();
    }
    str << s::otabcommalist();
    str << s::executeStatementPointer(this, str.state()) << "EXECUTE ";
    translateSqlToString(*this, str, 0);
    str << s::ctabcommalist();
    notFirst = true;
    currentTbl = 0;
  }
}

void Sm::LockTable::sqlDefinitionSingleTbl(Sm::Codestream &str, Ptr<Id2> &tbl) {
  str << "LOCK TABLE " << tbl << " IN ";
  switch (lockMode) {
    case lock_table::SHARE:
      str << "SHARE";
      break;
    case lock_table::EXCLUSIVE:
      str << "EXCLUSIVE";
      break;
    default:
      throw 999;
  }
  str << " MODE ";
  switch (waitMode) {
    case lock_table::EMPTY:
      break;
    case lock_table::WAIT:
      str << "WAIT";
      break;
    case lock_table::NOWAIT:
      str << "NOWAIT";
      break;
  }
}

void Sm::LockTable::sqlDefinition(Sm::Codestream &str) {
  if (currentTbl)
    sqlDefinitionSingleTbl(str, currentTbl);
  else {
    if (!tables)
      return;
    bool isNotFirst = false;
    for (Ptr<Id2> &tbl : *tables) {
      if (isNotFirst)
        str << s::procendl(getLLoc());
      sqlDefinitionSingleTbl(str, tbl);
    }
  }
}

void Sm::Rollback::linterDefinition(Sm::Codestream &str) {
  if (savepoint)
    str << s::executeStatementPointer(this, str.state()) << "EXECUTE " << s::oquote << "ROLLBACK TO SAVEPOINT " << savepoint << s::cquote;
  else
    str << "ROLLBACK";
}

Exception::GlobalUserExceptions Exception::globalUserExceptions;

void Sm::Exception::linterDefinition(Sm::Codestream &str) {
  str << "EXCEPTION" << s::name << exception << s::name << "FOR CUSTOM" << s::name;
  GlobalUserExceptions::iterator it = globalUserExceptions.find(exception->toNormalizedString());
  if (it != globalUserExceptions.end())
    str << it->second;
  else
    str << "ERROR: USER EXCEPTION CODE IS UNDEFINED";
}

void Exception::linterReference(Codestream &str) { str << exception; }

}


Ptr<Sm::Variable> ResolvedEntity::addVariableIntoOwnerBlock(Datatype *varT,
                                                            std::string *dstDeclVarName,
                                                            CLoc loc,
                                                            std::string baseName,
                                                            bool addFirstNumber,
                                                            bool notPushBackToDeclarations) {
  if (loc.beginedFrom(51356,55))
    cout << "";
  if (BlockPlSql *b = ownerPlBlock()) {
    b = b->getTopBlock();
    Ptr<LevelResolvedNamespace> l = b->getSemanticNode()->childNamespace;
    string declVarName;
    if (!addFirstNumber) {
      string copy = baseName;
      transform(copy.begin(), copy.end(), copy.begin(), ::toupper);
      if (l->count(copy))
        declVarName = l->getUniqueName(baseName);
      else
        declVarName = baseName;
    }
    else
      declVarName = l->getUniqueName(baseName);
    Ptr<Id> name = new Id(string(declVarName));
    Ptr<Sm::Variable> var = new Sm::Variable(name, varT);
    var->loc(loc);
    var->setOwnerBlockPlSql(b);
    if (SemanticTree *n = b->getSemanticNode()) {
      n->addChild(var->toSTree());
      var->getSemanticNode()->levelNamespace = n->childNamespace;
    }
    if (!notPushBackToDeclarations)
      b->declarations.push_back(var.object());
    Ptr<VEntities> ent = new VEntities();     ent->add(var.object());
    string upVarName = declVarName;
    transform(upVarName.begin(), upVarName.end(), upVarName.begin(), ::toupper);
    l->insert(LevelResolvedNamespace::value_type(upVarName, LevelResolvedNamespace::value_type::second_type(ent)));
    if (dstDeclVarName)
      dstDeclVarName->swap(declVarName);
    var->translatedName(*dstDeclVarName);
    var->flags.setTemporary();
    var->use();
    return var;
  }
  return 0;
}

/* Получить временную переменную типа varT через механизм кеширования переменных
 * Переменная должна быть освобождена через метод unuse() для повторного использования
 */
Ptr<Sm::Variable> ResolvedEntity::getTemporaryVar(Sm::Codestream &str, Datatype *varT, std::string *dstDeclVarName, bool dynamic) {
  BlockPlSql *b = ownerPlBlock();
  if (!b)
    return NULL;

  b = b->getTopBlock();
  for (Declarations::reverse_iterator it = b->declarations.rbegin(); it != b->declarations.rend(); ++it) {
    Variable *var = (*it)->toSelfVariable();
    if (!var || !var->isTemporary() || var->isUsed())
      continue;
    if (dynamic && !var->isDynamicUsing())
      continue;
    CastCathegory c = varT->getCastCathegory(var->getDatatype(), str.isProc(), str.isSql());
    if (!c.equally())
      continue;
    VarHelper::getInstance().add(var);
    *dstDeclVarName = var->translatedName();
    return var;
  }

  Ptr<Variable> var = addVariableIntoOwnerBlock(varT, dstDeclVarName);
  if (dynamic)
    var->setIsDynamicUsing();
  str.activateDeclarations();
  str << s::tab(2) << var << s::procendl();
  str.activatePrevious();
  VarHelper::getInstance().add(var);
  return var;
}

bool ResolvedEntity::translateAsUserFunctionCall(Sm::Codestream &str, IdEntitySmart &reference, bool _isNot, bool ignoreChecks, ResolvedEntity **tmpVar) {
  if (str.state().queryForExecute_)
    return false;
  {
    Ptr<Id> entity = reference.entity();
    if (!entity || !entity->isFunction())
      return false;
    ResolvedEntity *funcDef = entity->definition();
    if (!ignoreChecks) {
      Datatype *t = 0;
      if (funcDef->isCollectionMethod() && !reference.isDynamicUsing())
        return false;
      else if (funcDef->isConstructor() || funcDef->isElementaryLinterFunction())
        return false;
      else if ((t = funcDef->getDatatype()) == NULL ||
             (t->isCompositeType() && reference.isDynamicUsing()))
        return false;
    }
  }

  {
    std::string declVarName;
    Ptr<Id> ent = reference.entity();
    syntaxerContext.model->delayDeletedIds.push_back(ent.object());
  if (Ptr<Sm::Variable> var = getTemporaryVar(str, ent->getDatatype(), &declVarName, reference.isDynamicUsing())) {
      str.levelPush();
      str.activateActions();
      str << s::tab() << "CALL " << reference << " INTO " << declVarName << s::procendl();
      str.actionsToPrefixes();
      str.levelPop();
      if (_isNot)
        str << "not ";
      str << declVarName;
      if (tmpVar)
        *tmpVar = var;
      return true;
    }
  }
  return false;
}


ResolvedEntity *ModelContext::inExternalVariablesStack(ResolvedEntity* d) const {
  if (externalVariablesStack.size())
    if (UniqueEntitiesMap *ent = *(externalVariablesStack.end() - 1)) {
      //UniqueEntitiesMap::key_type = ent;
      UniqueEntitiesMap::const_iterator it = ent->find(d);
      if (it != ent->end()) {
        if (ResolvedEntity *d = it->second->entity)
          return d;
        else if (Sm::IdEntitySmart *ent = it->second->fullName)
          return ent->definition();
      }
    }
  return 0;
}


void Return::linterDefinition(Codestream &str) {
  if (beginedFrom(685037,7))
    cout << "";
  Sm::RefAbstract *exprId;
  if (expr && (exprId = expr->toSelfRefAbstract()) != NULL) {
    Ptr<Datatype> t = exprId->getDatatype();
    ResolvedEntity *collection;

    bool noskip = true;
    if (ResolvedEntity *fun = ownerBlock->getTopBlock()->ownerFunction()) {
      if (nAssert(fun->getDatatype())->isRefCursor() && t->isRefCursor())
        noskip = false;
    }
    else
      throw 999;

    if (noskip && exprId->reference->majorObjectRef(&collection) && exprId->reference->isDynamicUsing() &&
        (t->isRowTypeOf() || t->isRefCursor() || t->isRecordType())) {
      std::string declVarName;
      Ptr<Id> ent = exprId->reference->entity();
      if (Ptr<Sm::Variable> var = exprId->getTemporaryVar(str, t, &declVarName)) {
        str.levelPush();
        str.activateActions();
        str << s::tab();
        translateCursorAsSelectCollection(str, *(exprId->reference), collection, var.object());
        str << s::procendl(getLLoc());
        str.actionsToPrefixes();
        str.levelPop();
        str << "RETURN" << s::name << declVarName;
        return;
      }
    }
  }

  str << "RETURN";
  if (expr)
    str << s::name << expr;
  else {
    if (BlockPlSql *block = getOwnerBlock()) {
      block = block->getTopBlock();
      ResolvedEntity *func = block->ownerFunction();
      Function *f;
      if (func && (f = func->toSelfFunction()) && f->isPipelined())
        str << s::name << s::linref(f->pipeVar);
    }
  }
}

