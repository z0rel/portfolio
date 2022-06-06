#include <array>
#include <algorithm>
#include <iostream>
#include "model_context.h"
#include "system_sysuser.h"
#include "resolvers.h"
#include "semantic_function.h"
#include "semantic_blockplsql.h"
#include "semantic_collection.h"
#include "depworkflow.h"
#include "smart_lexer.h"
#include "config_converter.h"
#include "error_codes.h"
#include "dynamic_sql_op.h"
#include "lexsubtokenizer.h"

// Uncomment for additional printing
#ifndef PRINT_COMPLEX_REFERENCES
//#define PRINT_COMPLEX_REFERENCES
#endif
#ifndef PRINT_SYSTEM_FUNCTIONS
//#define PRINT_SYSTEM_FUNCTIONS
#endif


extern SyntaxerContext syntaxerContext;
extern bool __STreeDestructionStarted__;
extern Sm::LexSubtokenizer lexerSubtokenizer;

void parseFile(SyntaxerContext * context, const char * filename);

using namespace Sm;

template <typename T>
void containerToSTree(SemanticTree &root, T &container) {
  for (typename T::const_iterator it = container.begin(); it != container.end(); ++it)
    root.addChild(it->second->toSTree());
}


Ptr<SysUser> ModelContext::sysuser() const { return sysuser_; }




void ModelContext::alterUser(Sm::AlterUser *c) {
  if (Ptr<Sm::AlterUser> cmd = c) {
    cmd->actionNumber(nextAct());
    for (Sm::List<Id>::const_iterator cIt = cmd->users->begin(); cIt != cmd->users->end(); ++cIt)
      getUser(*cIt)->alterCommands.push_back( cmd->settings );
    cmd->resolve(*this);
  }
}


void ModelContext::addSynonym(Sm::Synonym *cmd) {
  if (Ptr<Sm::Synonym> data = cmd)
    if (data->name) {
      data->actionNumber(nextAct());

      if (data->isPublic)
        data->name->entity()->definition(publicSynonyms.insert(make_pair(data->name->entity()->toNormalizedString(), data)).first->second.object());
      else
        data->name->entity()->definition(getUContextWithAdd(data->name.object())->synonyms.insert(make_pair(data->name->entity()->toNormalizedString(), data)).first->second.object());
      data->resolve(*this);
    }
}


void ModelContext::addPackage(Sm::Package *cmd) {
  if (Ptr<Sm::Package> pack = cmd)
    if (pack->name) {
      UserContext *uCntx = getUContextWithAdd(pack->name.object());
      UserContext::Packages::iterator it = uCntx->packages.find(pack->name->entity()->toNormalizedString());
      if (it != uCntx->packages.end()) {
        if (syntaxerContext.joinEntities || (it->second->bodyEmpty() && !pack->bodyEmpty()))
          it->second->pullFromOther(pack);
      }
      else {
        pack->actionNumber_ = nextAct();
        it = uCntx->packages.insert(make_pair(pack->name->entity()->toNormalizedString(), pack)).first;
        it->second->name->entity()->definition(it->second.object());
      }
    }
}


Ptr<UserContext> ModelContext::getSystemUser() const { return systemUser_; }


ModelContext::ModelContext(SyntaxerContext *par)
  : parent         (par),
    udata          (0),
    globalDatatypes(),
    sysuser_       (new SysUser(*this)),
    globalFunctions(0)
{
  indicesNamespace = new LevelResolvedNamespace(0, 0);

  levelNamespace(new LevelResolvedNamespace(0, 0));
  globalFunctions = new Sm::GlobalFunctions();

  systemUser_ = addUser(new Id(string("SYSTEM")));

  modelActions.diffMode = false;
  systemNamespacePosition = numeric_limits<unsigned int>::max() - 1;
  Ptr<BooleanLiteral> tr = new Sm::BooleanLiteral(true);
  Ptr<BooleanLiteral> fl = new Sm::BooleanLiteral(false);
  booleanLiterals.push_back(tr);
  booleanLiterals.push_back(fl);
  globalLiterals["TRUE" ] = new Sm::Id("TRUE" , tr.object());
  globalLiterals["FALSE"] = new Sm::Id("FALSE", fl.object());
  // TODO: для таких определений исключений - нужно задавать таблицу трансляции

  globalExceptions["NO_DATA_FOUND"   ] = new Exception(new Id("NO_DATA_FOUND"   ), {"2"});
  globalExceptions["TOO_MANY_ROWS"   ] = new Exception(new Id("TOO_MANY_ROWS"   ), {"3006"});
  globalExceptions["CASE_NOT_FOUND"  ] = new Exception(new Id("CASE_NOT_FOUND"  ), {"10080"});
  globalExceptions["DUP_VAL_ON_INDEX"] = new Exception(new Id("DUP_VAL_ON_INDEX"), {"903"});
  globalExceptions["INVALID_CURSOR"  ] = new Exception(new Id("INVALID_CURSOR"  ), {"CURNOTOPEN", "BADCURSOR"});
  globalExceptions["INVALID_NUMBER"  ] = new Exception(new Id("INVALID_NUMBER"  ), {"1040", "1041", "1042"});
  globalExceptions["VALUE_ERROR"     ] = new Exception(new Id("VALUE_ERROR"     ), {"1029", "1030", "1031", "1032", "1033", "1034", "1036", "1038", "1039", "1040", "1041", "1042", "1055"});
  globalExceptions["ZERO_DIVIDE"     ] = new Exception(new Id("ZERO_DIVIDE"     ), {"DIVZERO"});

  rootGlobalTree = new Sm::SemanticTree();
  syntaxerContext.globalRootNode = rootGlobalTree;
  rootGlobalTree->cathegory = Sm::SCathegory::RootSemanticNode;
  levelNamespace()->semanticLevel = rootGlobalTree;
  rootParent.push_back(nullptr);
  this->userMap["SYS"] = sysuser_.object();
}


SemanticTree *UserContext::toSTreeBase() const {
  SemanticTree *root = username->toSNodeDef(SCathegory::User, this);
  root->unnamedDdlEntity = (UserContext*)this;
  containerToSTree(*root, synonyms );
  containerToSTree(*root, sequences);
  containerToSTree(*root, tables   );
  containerToSTree(*root, views    );
  containerToSTree(*root, functions);
  containerToSTree(*root, types    );
  containerToSTree(*root, packages );
  containerToSTree(*root, triggers );
  containerToSTree(*root, indices  );
  containerToSTree(*root, dblinks  );
  for (DiffTables::const_iterator it = diffTables.begin(); it != diffTables.end(); ++it)
    root->addChild(it->second.second->toSTree());


  Ptr<Id> initializerSemanticNodeName = new Id(getName()->toQString() + string(".USER_INITIALIZER"), (UserContext*)this);
  initializerSemanticNode = initializerSemanticNodeName->toSNodeDef(SCathegory::Function, this);
  root->addChild(initializerSemanticNode);

  return root;
}


template <typename T> void containerResolve(T &container, ModelContext &model) {
  for (typename T::iterator it = container.begin(); it != container.end(); ++it)
    it->second->resolve(model);
}


void UserContext::resolve(ModelContext & model) {
  containerResolve(synonyms , model);
  containerResolve(sequences, model);
  containerResolve(views    , model);
  containerResolve(functions, model);
  containerResolve(types    , model);
  containerResolve(triggers , model);
  containerResolve(indices  , model);
}


bool ModelContext::findInRootNamespace(Ptr<Sm::Id> &reference) {
  if (Sm::SemanticTree *n = getSemanticNode()) {
    Sm::SemanticTree *node = 0;
    if (n->childNamespace->findDeclaration(node, reference)) {
      if (!reference->semanticNode())
        reference->semanticNode(node);
      return true;
    }
  }
  return false;
}


bool ModelContext::getFieldRef(Ptr<Sm::Id> &reference) {
  if (!reference)
    return false;
  GlobalFunctions::OraTemplatesFunMap::iterator it = globalFunctions->oraTemplatesFunMap.find(reference->toNormalizedString());
  if (it != globalFunctions->oraTemplatesFunMap.end()) {
    bool plContext = false;
    if (SemanticTree *n = reference->semanticNode())
      plContext = n->isPlContext();
    else
      throw 999;
    static CallArgList defaultCallArglist;
    CallArgList *callArgList = reference->callArglist.object();
    if (!callArgList)
      callArgList = &defaultCallArglist;
    if (Ptr<Sm::Function> fun = it->second->constructor(reference, *callArgList, plContext).object()) {
      if (!fun->getSemanticNode()) {
        SemanticTree *modelNode = getSemanticNode();
        if (SemanticTree *funNode = fun->toSTree()) {
          fun->setSemanticNode(funNode);
          modelNode->childs.push_front(funNode);
          funNode  ->setPositionInParent(modelNode->childs.begin());
        }
      }
      reference->definition(fun.object());
    }
    return true;
  }
  else
    return findInRootNamespace(reference);
}


Ptr<Datatype> ModelContext::getDatatype() const { return 0; }


void ModelContext::addSystemDatatypesNodes(SemanticTree *globalTree) const {
  globalTree->addChild(Datatype::mkBoolean()             ->toSTree());
  globalTree->addChild(Datatype::mkString()              ->toSTree());
  globalTree->addChild(Datatype::mkInteger()             ->toSTree());
  globalTree->addChild(Datatype::mkNumber()              ->toSTree());
  globalTree->addChild(Datatype::mkDouble()              ->toSTree());
  globalTree->addChild(Datatype::mkDate()                ->toSTree());
  globalTree->addChild(Datatype::mkRowid()               ->toSTree());
  globalTree->addChild(Datatype::mkTimestamp()           ->toSTree());
  globalTree->addChild(Datatype::mkTimestampTimezone()   ->toSTree());
  globalTree->addChild(Datatype::mkTimestampLtzTimezone()->toSTree());
  globalTree->addChild(Datatype::mkIntervalDayToSecond() ->toSTree());
  globalTree->addChild(Datatype::mkIntervalYearToMonth() ->toSTree());
  globalTree->addChild(Datatype::mkDecimal()             ->toSTree());
  globalTree->addChild(Datatype::mkFloat()               ->toSTree());
  globalTree->addChild(Datatype::mkReal()                ->toSTree());
  globalTree->addChild(Datatype::mkSmallint()            ->toSTree());
  globalTree->addChild(Datatype::mkBinaryInteger()       ->toSTree());
  globalTree->addChild(Datatype::mkNatural()             ->toSTree());
  globalTree->addChild(Datatype::mkNaturalN()            ->toSTree());
  globalTree->addChild(Datatype::mkPositive()            ->toSTree());
  globalTree->addChild(Datatype::mkPositiveN()           ->toSTree());
  globalTree->addChild(Datatype::mkSignType()            ->toSTree());
  globalTree->addChild(Datatype::mkPlsInteger()          ->toSTree());
  globalTree->addChild(Datatype::mkBlob()                ->toSTree());
  globalTree->addChild(Datatype::mkClob()                ->toSTree());
  globalTree->addChild(Datatype::mkNClob()               ->toSTree());
  globalTree->addChild(Datatype::mkBfile()               ->toSTree());
  globalTree->addChild(Datatype::mkChar()                ->toSTree());
  globalTree->addChild(Datatype::mkRaw()                 ->toSTree());
  globalTree->addChild(Datatype::mkLongRaw()             ->toSTree());
  globalTree->addChild(Datatype::mkURowId()              ->toSTree());
  globalTree->addChild(Datatype::mkLong()                ->toSTree());
  globalTree->addChild(Datatype::mkVarchar2()            ->toSTree());
  globalTree->addChild(Datatype::mkNChar()               ->toSTree());
  globalTree->addChild(Datatype::mkNVarchar2()           ->toSTree());
  globalTree->addChild(Datatype::mkNVarchar()            ->toSTree());
  globalTree->addChild(Datatype::mkVarchar()             ->toSTree());
  globalTree->addChild(Datatype::mkNull()                ->toSTree());
  globalTree->addChild(Datatype::mkDefault()             ->toSTree());
}


ModelContext::~ModelContext() {
  __STreeDestructionStarted__ = true;
  if (globalFunctions)
    delete globalFunctions;

  sysuser_ = 0;
  userMap.clear();
  publicSynonyms.clear();

  SemanticTree *t = rootGlobalTree;
  rootGlobalTree = 0;
  delete t;
}


Sm::SemanticTree *ModelContext::toSTreeBase() const {
  Ptr<Id> modelName = new Id("", (ModelContext*)this);
  SemanticTree *globalTree = modelName->toSNodeDef(SCathegory::ModelContext, this);
  globalTree->unnamedDdlEntity = (ModelContext*)this;

  containerToSTree(*globalTree, publicSynonyms);

  addSystemDatatypesNodes(globalTree);

  for (BooleanLiterals::const_iterator lIt = booleanLiterals.begin(); lIt != booleanLiterals.end(); ++lIt)
    globalTree->addChild((*lIt)->toSTree());
  for (GlobalExceptions::const_iterator dIt = globalExceptions.begin(); dIt != globalExceptions.end(); ++dIt)
    globalTree->addChild(dIt->second->toSTree());
  // TODO: Добавить все системные функции, которые имеются в глобальном пространстве имен
  for (Sm::GlobalFunctions::SemanticNodes::const_iterator it = globalFunctions->semanticNodes.begin(); it != globalFunctions->semanticNodes.end(); ++it)
    globalTree->addChild(*it);
  globalFunctions->semanticNodes.clear();

  containerToSTree(*globalTree, userMap);

  SemanticTree* linterAppendix = new SemanticTree(SCathegory::EMPTY, SemanticTree::NEW_LEVEL);
  globalTree->addChildForce(linterAppendix);
  globalFunctions->addLinterFunctions();
  linterAppendix->addChild(globalFunctions->linterLenblob->toSTree());
  linterAppendix->addChild(globalFunctions->linterGettext->toSTree());
  linterAppendix->addChild(globalFunctions->linterToDate->toSTree());

  return globalTree;
}

Sm::GlobalFunctions* modelFuns() {
  return syntaxerContext.model->globalFunctions;
}

void ModelContext::replaceStatementsIf(StmtTr tr, StmtTrCond cond) {
  for (UserDataMap::value_type &v : userMap)
    v.second->replaceStatementsIf(tr, cond);
}

void ModelContext::traverseModelStatements(StatementActor &fun) {
  for (UserDataMap::value_type &v : userMap)
    v.second->traverseModelStatements(fun);
}

void ModelContext::replaceChildsIf(Sm::ExprTr tr)  {
  for (UserDataMap::value_type &v : userMap)
    v.second->replaceChildsIf(tr);
}


void ModelContext::traverseDeclarations(Sm::DeclActor tr)  {
  for (UserDataMap::value_type &v : userMap)
    v.second->traverseDeclarations(tr);
}


template <typename T, typename... Arglists>
void traverseUserContextByMfCall(UserContext *cntx, T fun, Arglists&... args) {
  for (UserContext::Views::value_type &v : cntx->views)
    ((v.second.object())->*fun)(args...);
  for (UserContext::Functions::value_type &v : cntx->functions)
    ((v.second.object())->*fun)(args...);
  for (UserContext::Packages::value_type &v : cntx->packages)
    ((v.second.object())->*fun)(args...);
  for (UserContext::Types::value_type &v : cntx->types)
    ((v.second.object())->*fun)(args...);
  for (UserContext::Triggers::value_type &v : cntx->triggers)
    ((v.second.object())->*fun)(args...);
}


void UserContext::traverseDeclarations(Sm::DeclActor tr) {
  traverseUserContextByMfCall(this, &TraversingInterface::traverseDeclarations, tr);
}


void UserContext::replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond) {
  traverseUserContextByMfCall(this, &TraversingInterface::replaceStatementsIf, tr, cond);
}

void UserContext::traverseModelStatements(StatementActor &fun) {
  traverseUserContextByMfCall(this, &TraversingInterface::traverseModelStatements, fun);
}

void UserContext::replaceChildsIf(Sm::ExprTr tr) {
  traverseUserContextByMfCall(this, &TraversingInterface::replaceChildsIf, tr);
}


void UserContext::collectInitializers(UserEntitiesMap &container) {
  traverseUserContextByMfCall(this, &TraversingInterface::collectInitializers, container[this]);
}


void ModelContext::configure(int argc, char **argv) {
  syntaxerContext.stage = SyntaxerContext::CONFIGURE;
  lexerSubtokenizer.setConfigStage();
  configConverter(syntaxerContext);

  if (argc > 1)
    syntaxerContext.confFileName = argv[1];

  parseConfFile(syntaxerContext.confFileName.c_str(), syntaxerContext.model->modelActions, syntaxerContext.model->modelActions.refsToCodegen);

  dmperr::errorseInit(dmperr::errorCodeConverter, syntaxerContext.errorseFile);
  dmperr::errorCodeInit(dmperr::errorCodeConverter, syntaxerContext.errorsFile);

  if (syntaxerContext.model->modelActions.directCreateInDB && !syntaxerContext.forceNotFilterEntities)
    syntaxerContext.filterEntities = true; // защита от дикой нелепой случайности при работе с основной базой.

  if (syntaxerContext.requestEntities)
    syntaxerContext.model->queryCreatedObjects();
}


void ModelContext::syntaxAnalyze() {
  cout << " == parsing == " << endl;
  syntaxerContext.stage = SyntaxerContext::SYNTAX_ANALYZE;

  clock_t begin = clock();

  lexerSubtokenizer.setOracleStage();

  for (std::vector<std::string>::iterator it = syntaxerContext.model->modelActions.files.begin(); it != syntaxerContext.model->modelActions.files.end(); ++it) {
    cout << "   " << it->c_str() << endl;
    parseFile(&syntaxerContext, it->c_str());
    if (it == syntaxerContext.model->modelActions.files.begin())
      syntaxerContext.model->globalFunctions->addSpecialFun();
  }

  syntaxerContext.joinEntities = false;
  for (std::vector<std::string>::iterator it = syntaxerContext.model->modelActions.joinFiles.begin(); it != syntaxerContext.model->modelActions.joinFiles.end(); ++it) {
    cout << "   " << it->c_str() << endl;
    parseFile(&syntaxerContext, it->c_str());
  }

  cout << " == parsing finished == " << diffTime(begin) << " sec" << endl;
}


void ModelContext::modelBsizes()
{
  SmartptrSet s;
//  SmartptrSet id;
//  SmartptrSet idSmart;
//  SmartptrSet id2;
  cout << "SemanticTree: " << (rootGlobalTree->bSize(s) / (1024*1024)) << " Mb" << endl;
  size_t idSz = 0;
  size_t idSmartSz = 0;
  size_t id2Sz = 0;
//  for (SmartptrSet::value_type v : s) {
//    if (const Id *i = dynamic_cast<const Id*>(v))
//      idSz += i->bSize(id);
//    else if (const IdEntitySmart *i = dynamic_cast<const IdEntitySmart*>(v))
//      idSmartSz += i->bSize(idSmart);
//    else if (const Id2 *i = dynamic_cast<const Id2*>(v))
//      id2Sz += i->bSize(id2);
//  }
  cout << "Id size: " << (idSz / (1024*1024)) << " Mb" << endl;
  cout << "IdEntitySmart size: " << (idSmartSz / (1024*1024)) << " Mb" << endl;
  cout << "Id2 size: " << (id2Sz / (1024*1024)) << " Mb" << endl;

  SmartptrSet s2;
  cout << "LevelResolvedNamespace: " << (levelNamespace()->bSize(s2) / (1024*1024)) << " Mb" << endl;
  cout << "LevelResolvedNamespace oth: " << (rootGlobalTree->bLevelNamespaceSize(s2) / (1024*1024)) << " Mb" << endl;
}

void ModelContext::contextAnalyze() {
  clock_t begin;

  cout << " == context analyse ==" << endl;
  syntaxerContext.stage = SyntaxerContext::CONTEXT_ANALYZE;

  for (Sm::DelayedConstraints &dC : delayedConstraints)
    Sm::collectConstraintsOnField(this, dC.table, dC.field, dC.constraints);

  delayedConstraints.clear();


  // Сборка свойств полей в определениях полей для кодогенерации.
  parseAlterTableCommands();

  setPackagesAttributesOnBlocks();
  formalPrepareModel();

  sysusers[sysuser().object()] = 0;
  UserDataMap::iterator it;
  if ( (it = userMap.find("XDB")) != userMap.end() )
    sysusers[it->second.object()] = 0;

  containerResolve(userMap, *this);

  cout << " === generate semantic tree ===" << flush;
  // Построить глобальное семантическое дерево.
  begin = clock();
  rootGlobalTree->childs.push_back(toSTree());

  cout << " " << diffTime(begin) << endl;

  // Установить обратные ссылки для внутренних определений базовых сущностей модели
  // В дереве установить обратные ссылки (ссылки на родителей).
  rootGlobalTree->setParentForChilds(rootParent.begin());
  rootGlobalTree->setPlContext();
  // В дереве установить для элементов модели ссылки на узлы дерева

  cout << " === build declaration namespace ===" << flush;
  // Собрать объявления и определения одних и тех же объектов для удобства
  // сравнения по указателю на сущность во внутренней модели.
  getSemanticNode()->childNamespace = levelNamespace();
  rootGlobalTree->unnamedDdlEntity = this;
  begin = clock();
  Ptr<LevelResolvedNamespace> rootNamespace = levelNamespace();
  collectEqualsDeclaration(
      rootGlobalTree->childs.begin(),
      rootGlobalTree->childs.end(),
      rootNamespace);
  cout << " " << diffTime(begin) << endl;

  // поиск определений для всех имен-ссылок исключая функции и (возможно)
  // сложные подзапросы.

  translateCursorFieldDecltype();

  cout << " === check declaration namespace ===" << flush;
  begin = clock();
  levelNamespace()->check();

  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    for (UserContext::Views::iterator tIt = it->second->views.begin(); tIt != it->second->views.end(); ++tIt)
      tIt->second->semanticResolve();

  levelNamespace()->resolveOverloaded(); // вызов у самого старшего LevelResolvedNamespace, до резолвинга ссылок

  cout << " " << diffTime(begin) << endl;

  cout << " === resolve references ===" << flush;
  begin = clock();

  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    for (UserContext::Views::iterator tIt = it->second->views.begin(); tIt != it->second->views.end(); ++tIt)
      tIt->second->semanticResolve();

  rootGlobalTree->resolveReference();

  resolveDynamicOperators();

  cout << " " << diffTime(begin) << endl;

  cout << " === collect graph of dependencies  ===" << flush;
  begin = clock();
  rootGlobalTree->clearOpenState();
//  rootGlobalTree->collectDependenciesGraph(dependenciesGraph);
  cout << " " << diffTime(begin) << endl;
  cout << " == context analyze finished ==" << endl;

  if (!syntaxerContext.model->modelActions.supressUnresolvedPrinting)
    rootGlobalTree->printSortedUnresolvedDeclarations();

  rootGlobalTree->deleteInvalidChilds();


#ifdef PRINT_COMPLEX_REFERENCES
  cout << "=== print complex references === " << endl;
  rootGlobalTree->printComplexReferences();
#endif

#ifdef PRINT_SYSTEM_FUNCTIONS
  cout << "=== print system functions === " << endl;
  rootGlobalTree->printSystemFunctions();
  cout << "=== end === " << endl;
  exit(0);
#endif

//  exit(0);
}

namespace Sm {
  extern int __longLineLimit__;

  size_t DelayedConstraints::getDelayedConstrId() {
    static size_t x = 0;
    ++x;
    if (x == 32120)
      cout << "";
    return x;
  }

  DelayedConstraints::DelayedConstraints(table::FieldDefinition *f, Table *t, Sm::List<Constraint> *cL)
    : field(f), table(t), constraints(cL) {}

}


SpecialStoreKeys::SpecialStoreKeys(ModelContext &_ctx, DependenciesStruct *_m, DependenciesStruct::SortedEntities &_sortedEntities, Codestream &_str, Codestream &_strVars)
  : ctx(_ctx),
    m(_m),
    sortedEntities(_sortedEntities),
    str(_str),
    strVars(_strVars) {}


void SpecialStoreKeys::linterDefinition(Codestream &str) {
  str.procMode(CodestreamState::SQL);
  int savedFlags = ctx.modelActions.flags;

  UserContext *savedUctx = m->curUser;
  auto storeKeys = [&](unsigned int cond) {
    if (savedFlags & cond) {
      ctx.modelActions.flags = cond;
      for (ResolvedEntity *d : sortedEntities)
        if (Table *tbl = d->toSelfTable()) {
          if (UserContext *uctx = tbl->userContext())
            m->checkoutUserCtx(uctx, this->str, strVars);
          str.state().procMode(CodestreamState::SQL);
          tbl->linterDefinitionKeys(str);
        }
    }
  };

  int oldLongValue = Sm::__longLineLimit__;
  __longLineLimit__ = 160;

  storeKeys(MODEL_ACTIONS_DROP_KEYS);
  storeKeys(MODEL_ACTIONS_DROP_TABLES_FOREIGN_KEYS);
  storeKeys(MODEL_ACTIONS_CREATE_TABLE_PRIMARY_KEYS);
  storeKeys(MODEL_ACTIONS_CREATE_TABLE_UNIQUE_KEYS);
  storeKeys(MODEL_ACTIONS_CREATE_TABLE_CHECKS);
  if (savedFlags & MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS) {
    storeKeys(MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS | MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS_GRANT_STAGE);
    storeKeys(MODEL_ACTIONS_CREATE_TABLE_FOREIGN_KEYS);
  }
  storeKeys(MODEL_ACTIONS_CREATE_TABLE_PRIMARY_FOREIGN_REFERENCES);
  if (ctx.modelActions.createIndices()) {
    ctx.modelActions.flags |= MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE;
    storeKeys(MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE);
    ctx.modelActions.flags &= ~(MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE);
    storeKeys(MODEL_ACTIONS_CREATE_INDICES);
    ctx.modelActions.flags &= ~(MODEL_ACTIONS_CREATE_INDICES);
  }

  __longLineLimit__ = oldLongValue;

  if (savedUctx)
    m->checkoutUserCtx(savedUctx, this->str, strVars);

  ctx.modelActions.flags = savedFlags & ~(MODEL_ACTIONS_KEYS_MANIPULATIONS);
}


void storeIndices(ModelContext       &ctx,
                  DependenciesStruct &m,
                  Codestream         &str,
                  Codestream         &str_vars,
                  DependenciesStruct::SortedEntities &sortedEntities)
{
  int oldLongValue = Sm::__longLineLimit__;
  Sm::__longLineLimit__ = 160;

  ctx.modelActions.flags |= MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE;
  str.state().procMode(CodestreamState::SQL);
  m.storeSql(sortedEntities, str, str_vars);
  ctx.modelActions.flags &= ~(MODEL_ACTIONS_CREATE_INDICES_GRANT_STAGE);

  str.state().procMode(CodestreamState::SQL);
  m.storeSql(sortedEntities, str, str_vars);
  ctx.modelActions.flags &= ~(MODEL_ACTIONS_CREATE_INDICES);
  Sm::__longLineLimit__ = oldLongValue;
}

void ModelContext::codegeneration() {
  rootGlobalTree->collectDependenciesGraph(dependenciesGraph);
  syntaxerContext.stage = SyntaxerContext::CODEGENERATION;

  codegenerationStarted = true;
  if (syntaxerContext.createInitializers) {
    Codestream str_initializer;
    userInitializer(str_initializer);
  }

  bool codegenByRefs = !(modelActions.isCreateAllModel() || modelActions.createIndices());
  DependenciesStruct::SortedEntities sortedEntities;

  if (!syntaxerContext.skipCodegeneration) {
    DependenciesStruct m;

    Codestream str;
    Codestream str_vars;

    if (modelActions.createIndices() || modelActions.keysManipulation()) {
      specialStoreKeysAction = new SpecialStoreKeys(*this, &m, sortedEntities, str, str_vars);
      m.entitiesForCodegen.insert(specialStoreKeysAction.object());
    }

    if (codegenByRefs && !syntaxerContext.descrErrorsEntitiesResolve)
      m.init(modelActions.refsToCodegen);
    else
      m.init();

    m.layingGraph(sortedEntities);


//    if (modelActions.createIndices())
//      storeIndices(*this, m, str, str_vars, sortedEntities);

    if (modelActions.isCreateAllModel())
      m.storeSql(sortedEntities, str, str_vars);
    else
      m.storeSql(sortedEntities, str, str_vars);

    str_vars.store(syntaxerContext.outFileName + "_variables.sql", /* skipIfEmpty = */ true);
    str.store(syntaxerContext.outFileName, /* skipIfEmpty = */ true);

    if (modelActions.directCreateInDB)
      Sm::StatisticNode::printAggregatedSortedStatistic(Sm::StatisticNode::errorStatVector);
  }

  if (!codegenByRefs && !syntaxerContext.skipCodegeneration)
    calculateStatistics(&sortedEntities);
  else {
    DependenciesStruct statM;
    statM.init();
    DependenciesStruct::SortedEntities statSortedEntities;
    statM.layingGraph(statSortedEntities);
    calculateStatistics(&statSortedEntities);
  }
}


void ModelContext::dumpDB() {
  if (modelActions.isCreateAllModel())
    dumpDependenciesSql(syntaxerContext.dumpDbFile.c_str(), syntaxerContext.dumpSplitFiles);
  else
    dumpDependenciesSql(modelActions.refsToCodegen, syntaxerContext.dumpDbFile.c_str(), syntaxerContext.dumpSplitFiles);
}


UserContext::UserContext(Ptr<Sm::Id> _username, Sm::Id *_password)
    : username(_username), password(_password) { username->definition(this); }


UserContext::UserContext() {}


UserContext::~UserContext() {}


Ptr<Sm::Datatype> UserContext::getDatatype() const { return 0; }


Ptr<Sm::Id> UserContext::getName() const { return username; }


void UserContext::linterReference(Sm::Codestream &str) {
  if (username && username->toString().empty())
    str << "SYSTEM";
  else
    str << username;
}


Table *UserContext::findTable(string & str) {
  Tables::iterator it = tables.find(str);
  if (it != tables.end())
    return it->second.object();
  return 0;
}


UserContext *ModelContext::addUser(Ptr<Sm::Id> name, Id *password) {
  // поиск глобального контекста по имени пользователя unameInCmd (06 мая 2013 16:52:23)
  std::string &&uname = name->toNormalizedString();
  UserDataMap::iterator it = userMap.find(uname);
  if (it != userMap.end())
    if (UserContext *cntx = it->second.object()) {
      if (password)
        cntx->password = password; // сменить пароль
      name->definition(cntx);
      return cntx;
    }
  // else  Обработка ситуации, когда коннект выполняется к неизвестному юзеру. (23 апр. 2013 10:21:01)
  return userMap.insert(UserDataMapPair(name->toNormalizedString(), new UserContext(name, password))).first->second.object();
}


UserContext *ModelContext::getUser( Ptr<Sm::Id> name ) {
  UserDataMap::iterator it = userMap.find(name->toNormalizedString());
  if (it != userMap.end())
    return it->second.object();
  else {
//    cout << "warning: user " << name->toQString() << " not found. getUser create it." << endl;
    return userMap.insert( UserDataMapPair(name->toNormalizedString(), Ptr<UserContext>(new UserContext(name)))).first->second.object();
  }
}


UserContext *ModelContext::findUser(Ptr<Id> name) { return userMap.find(name->toNormalizedString())->second.object(); }



void ModelContext::connect(Ptr<Sm::Id> name) {
  UserDataMap::iterator it = userMap.find(name->toNormalizedString());
  if (it != userMap.end())
    udata = it->second;
  else
    udata = userMap.insert(UserDataMapPair(name->toNormalizedString(), new UserContext(name))).first->second.object();
}


Ptr<UserContext> ModelContext::getUContext(const Sm::Id2 *name) {
  if (!name)
    return 0;
  Ptr<Id> uname;
  if ((uname = name->uname()) || (uname = name->entity())) {
    UserDataMap::iterator uCntxIt = userMap.find(uname->toNormalizedString());
    if (uCntxIt != userMap.end()) {
      uname->definition(uCntxIt->second.object());
      return uCntxIt->second;
    }
  }
  return 0;
}


UserContext *ModelContext::getUContextOrNull(const Id2 *name) {
  UserContext *uCntx = 0;
  if (name->size() == 1) {
    uCntx = udata;
    if (udata)
      ((Id2*)name)->userName() = new Id(*(uCntx->username));
  }
  else if ( name->size() == 2 ) {
    UserDataMap::iterator uCntxIt = userMap.find(name->uname()->toNormalizedString());
    if (uCntxIt == userMap.end())
      return 0;
    else
      uCntx = uCntxIt->second;
    name->uname()->definition(uCntx->username->definition());
  } else
    throw 2;
  return uCntx;
}


void ModelContext::addTable(Ptr<Sm::Table> table) {
  if (!table)
    return;
  table->actionNumber(nextAct());
  UserContext *uCntx = getUContextWithAdd(table->getName2().object());
  table->getName()->definition(table.object());
  if (!table->getName2()->userName())
    table->getName2()->userName() = uCntx->username;

  if (modelActions.diffMode) {
    UserContext::Tables::iterator it = uCntx->tables.find(table->getName()->toNormalizedString());
    if (it == uCntx->tables.end())
      uCntx->diffTables.insert(UserContext::DiffTables::value_type(table->getName()->toNormalizedString(), make_pair(Ptr<Sm::Table>(), table.object())));
    else {
      it->second->othTable = table;
      uCntx->diffTables.insert(UserContext::DiffTables::value_type(table->getName()->toNormalizedString(), make_pair(it->second, table.object())));
    }
  }
  else {
    UserContext::Tables::iterator it = uCntx->tables.find(table->getName()->toNormalizedString());
    if (it == uCntx->tables.end())
      uCntx->tables.insert(make_pair(table->getName()->toNormalizedString(), table));
    else
      it->second->getDefaults(table);
  }
}


void ModelContext::addObjectTypeBody(smart::Ptr<Type::Object> body) {
  if (!body)
    return;
  UserContext *uCntx = getUContextWithAdd(body->getName2());
  UserContext::Types::iterator it = uCntx->types.find(body->getName()->toNormalizedString());
  if (it != uCntx->types.end()) {
    it->second->setObjectBody(body);
    body->setDeclaration(it->second->toSelfObject());
  }
  else
    it = uCntx->types.insert(UserContext::Types::value_type(body->getName()->toNormalizedString(), body.object())).first;
  body->getName()->definition(it->second.object());
}


void ModelContext::addLexerDefines(Ptr<Sm::Id> def) {
  if (def)  {
    Ptr<Id> qDef = new Id(*def);
    qDef->setQuoted();
    qDef->setSQuoted();
    lexerDefines[defLhs] = make_pair(def, qDef);
  }
}


Sm::Id* ModelContext::lexerDef(std::string &&s) const {
  LexerDefines::const_iterator it = lexerDefines.find(s);
  if (it != lexerDefines.end())
    return new Sm::Id(*(it->second.first));
  return new Sm::Id(std::forward<string>(s));
}


Sm::Id* ModelContext::lexerQDef(std::string &&s) const {
  LexerDefines::const_iterator it = lexerDefines.find(s);
  if (it != lexerDefines.end())
    return new Sm::Id(*(it->second.second));
  return new Sm::Id(std::forward<string>(s));
}


void ModelContext::delLexerDefines() { lexerDefines.erase(defLhs); }


void ModelContext::addWrapped(ResolvedEntity::ScopedEntities cat, Ptr<Sm::Id2> ent) {
  if (!ent)
    return;
  if (Ptr<UserContext> uctx = getUContext(ent))
    uctx->wrpEntities[cat][ent->entity()->toNormalizedString()] = ent->entity();
}


void ModelContext::queryCreatedObjects() {
  LinterWriter lwriter(syntaxerContext.linterUsername,  syntaxerContext.linterPassword, syntaxerContext.linterNodename);
  lwriter.queryAllObjects();
}


void ModelContext::resolveCreatedObjects() {
  if (syntaxerContext.requestEntities) {
    LinterSemanticResolver lsr;
    lsr.resolveCreatedObjects(linterCreatedEntities);
  }
}


void ModelContext::collectInitializers(UserEntitiesMap &container)  {
  for (UserDataMap::value_type &v : userMap)
    v.second->collectInitializers(container);
}


void ModelContext::addExistedInLinterDBEntity(std::string owner, std::string object_name, std::string object_type) {
  if (object_name.empty())
    return;
  std::map<std::string, ResolvedEntity::ScopedEntities> objectEntity = {
    {"TABLE"    , ResolvedEntity::Table_   },
    {"VIEW"     , ResolvedEntity::View_    },
    {"SEQUENCE" , ResolvedEntity::Sequence_},
    {"SYNONYM"  , ResolvedEntity::Synonym_ },
    {"PROCEDURE", ResolvedEntity::Function_},
    {"TRIGGER"  , ResolvedEntity::Trigger_ },
    {"USER"     , ResolvedEntity::User_    },
    {"VARIABLE" , ResolvedEntity::Variable_}
  };

  std::map<std::string, ResolvedEntity::ScopedEntities>::iterator it = objectEntity.find(object_type);
  if (it == objectEntity.end())
    throw 999;

  linterCreatedEntities[owner][it->second].insert(make_pair(object_name, (ResolvedEntity*)0));
}


/**
 * @brief openFile
 *   Открыть файл для парсинга
 * @param[in]  name
 *   Имя файла
 * @param[in]  opt
 *   Опции открытия, по умолчанию @c "rb"
 * @param[out] file
 *   Выходной дескриптор открытого файла
 * @return Успех или неудача
 * @retval  0 Файл успешно открыт
 * @retval -1 Не удалось открыть файл, либо имя файла пусто.
 */
int openFile(const char* name, const char* opt, FILE **file)
{
  if( !name ) {
    printf( "No input file defined\n"
            "Use -h option to get help\n" );
    return -1;
  }
  if( ( *file = fopen( name, opt ? opt : "rb" ) ) == 0 ) {
    printf( "Unable to open file '%s'\n", name );
    return -1;
  }
  return 0;
}


bool ModelContext::entityAlreadyCreatedInDb(Sm::ResolvedEntity *ent) {
  return (linterCreatedResolvedEntities.find(ent->getDefinitionFirst())
          != linterCreatedResolvedEntities.end());
}

ModelActions::ModelActions() {}


SyntaxerContext::SyntaxerContext()
  : containerOfModel(new ModelContext(this))
{
  model = containerOfModel.object();
}

SyntaxerContext::~SyntaxerContext() {
  containerOfModel = NULL;
}


// vim:foldmethod=syntax


