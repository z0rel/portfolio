#include "dependenciesstruct.h"
#include "resolvers.h"
#include "codegenerator.h"


extern SyntaxerContext syntaxerContext;

using namespace dep;

DependenciesStruct::DependenciesStruct() {}

void DependenciesStruct::init() {
  DependenciesBuilder b(*this);
  for (DependEntitiesMap::value_type &v : syntaxerContext.model->dependenciesGraph)
    b.collectDependenciesSubgraph(v.first);
}

void DependenciesStruct::init(std::vector<Sm::IdEntitySmart> &outherReferences) {
  DependenciesBuilder b(*this);
  for (vector<IdEntitySmart>::iterator it = outherReferences.begin(); it != outherReferences.end(); ++it) {
    it->resolveByModelContext();
    if (ResolvedEntity *def = it->definition())
      b.collectDependenciesSubgraph(def);
    else {
      string str;
      it->toNormalizedString(str);
      cout << "unknown reference " << str << endl;
    }
  }
}

void DependenciesStruct::init(std::set<Sm::ResolvedEntity*, Sm::LE_ResolvedEntities> &baseReferences) {
  DependenciesBuilder b(*this);
  for (ResolvedEntity *it : baseReferences)
    b.collectDependenciesSubgraph(it);
}

void DependenciesBuilder::collectDependenciesSubgraph(ResolvedEntity *def){
  static const unordered_set<CathegoriesOfDefinitions::ScopedEntities> skipCathegories = {
    FieldOfTable_, SqlSelectedField_, FromSingle_, QueriedTable_, QueryBlock_
  };

  if (!(def = def->unwrapReference()))
    return;

  while (Synonym *sm = def->toSelfSynonym()) {
    if (ResolvedEntity *newDef = sm->target->definition())
      def = newDef;
    else {
      cout << "error: unresolved synonym: " << sm->getLLoc().locText() << endl;
      return;
    }
  }

  if (def->isSystem()                 ||
      def->isSystemTemplate()         ||
      def->isException()              ||
      def->toSelfStatementInterface() ||
      skipCathegories.count(def->ddlCathegory()))
    return;

  DependEntitiesMap::iterator it = syntaxerContext.model->dependenciesGraph.find(def->getDefinitionFirst());
  if (it != syntaxerContext.model->dependenciesGraph.end())
    getDependenciesSubgraph(it->first); // dfs and insert all subgraph;
  else {
    Codestream str;
    str << "Entity " << s::linref(def)  << " with cathegory " << def->ddlCathegoryToString() << " is not found " << s::endl;
    cout << str.str();
  }
}


void DependenciesBuilder::getDependenciesSubgraph(ResolvedEntity* def) {
  static const unordered_set<CathegoriesOfDefinitions::ScopedEntities> skipCathegories = {
     Variable_, User_, Package_
  };

  while (Synonym *sm = def->toSelfSynonym())
    def = nAssert(sm->target->definition());

  if (!(def->isPackageVariable()) && skipCathegories.count(def->ddlCathegory()))
    return;

  src.entitiesForCodegen.insert(def);
  for (DependEntitiesMap::value_type &cIt : def->lazyAttributes()->outArcs) {
    ResolvedEntity *child = cIt.first;
    if (!src.entitiesForCodegen.count(child))
      getDependenciesSubgraph(child);
  }
}


void dep::addArc(PtrArcs &arcs, ResolvedEntity* from, ResolvedEntity *to) {
//  arcs.push_back(PtrArc(from, to));
  arcs.push_back(PtrArc(to, from));
}

PtrArcs dep::makeArcs(DependenciesStruct &src) {
  PtrArcs arcs;

  // outArcs - отражают что владелец outArcs использует сущности, лежащие в outArcs.
  // например, функция-владелец outArcs - вызывает функции, лежащие в outArcs
  // корни же генерируемого дерева - должны самыми малоиспользующими и самыми используемыми.
  // Следовательно, граф будем строить с обратным направлением дуг

  for (Sm::ResolvedEntity *ent : src.entitiesForCodegen) {
    for (DependEntitiesMap::value_type &it : ent->lazyAttributes()->outArcs)
      if (src.entitiesForCodegen.count(it.first))
        addArc(arcs, /*from*/ it.first , /*to*/ ent); /* from=it.first - используемая, to=ent - использующая */

    for (DependEntitiesMap::value_type &it : ent->lazyAttributes()->inArcs)
      if (src.entitiesForCodegen.count(it.first))
        addArc(arcs, /*from*/ ent, /*to*/ it.first); /* from=ent - используемая, to=it.first - использующая */
  }
  transformSortAndUnique(arcs);
  return arcs;
}


PtrNodes dep::getAllNodes(DependenciesStruct &src) {
  PtrNodes allNodes;
  for (ResolvedEntity *i : src.entitiesForCodegen)
    allNodes.push_back(i);
  transformSortAndUnique(allNodes);
  return allNodes;
}


PtrNodes dep::orphan(DependenciesStruct &src) {
  return diff(getAllNodes(src), getConnectedNodes(makeArcs(src)));
}

PtrNodes dep::getRootNodes(DependenciesStruct &src) {
  return diff(getAllNodes(src), toNodes(makeArcs(src)));
}


GraphLayingContext::GraphLayingContext(DependenciesStruct &src) {
  NodeIds      nodeIds;

  PtrArcs  _arcs          = makeArcs(src);
  PtrNodes connectedNodes = getConnectedNodes(_arcs);
  PtrNodes orphan         = diff(getAllNodes(src), connectedNodes);

  PtrNodes v_intersection;

  std::set_intersection(connectedNodes.begin(), connectedNodes.end(),
                        orphan        .begin(), orphan        .end(),
                        std::back_inserter(v_intersection), Sm::LE_ResolvedEntities());

  if (!v_intersection.empty())
    throw 999;


  for (ResolvedEntity *node : connectedNodes)
    addNode(node, nodeIds);
  for (ResolvedEntity *node : orphan)
    addNode(node, nodeIds);

  for (PtrArcs::value_type &v : _arcs)
    this->arcs.push_back(Arc(nodeIds.at(v.from), nodeIds.at(v.to)));

  laying();
  representAsPyDict();
}


void GraphLayingContext::representAsPyDict() {
  if (syntaxerContext.dependenciesStructReprFileName.empty())
    return;

  stringstream str;
  int cnt = 0;
  auto outEndl = [&]() -> string {
    if (cnt > 5) {
      cnt = 0;
      return "\n";
    }
    ++cnt;
    return " ";
  };

  auto outIndent = [&]() -> string { return cnt ? " " : "  "; };

  str << "depGraph = [" << endl;
  for (Arcs::value_type &v : arcs)
    str << outIndent() << "(" <<  v.first << "," << v.second << ")," << outEndl();
  if (cnt)
    str << endl;
  str << "]" << endl << endl;

  cnt = 0;
  str << "sortedResult = [" << endl;
  for (NodeId v : sortedNodes)
    str << outIndent() << v << "," << outEndl();
  if (cnt)
    str << endl;
  str << "]" << endl << endl;


  str << "names = [" << endl;
  int i = 0;
  for (ResolvedEntity *v : nodes) {
    str << "  (" << i++ << ", '" << v->ddlCathegoryToString() << "'";
    if (Ptr<Id> n = v->getName()) {
      str << ", \"\"\"";
      if (UserContext *uctx = v->userContext())
        str << uctx->getName()->toNormalizedString() << ".";
      if (ResolvedEntity *pkg = v->ownerPackage())
        str << pkg->getName()->toString() << "_";
      str << n->toString() << "\"\"\"";
    }
    str << ", '" << v->getLLoc() << "'";
    str << ")," << endl;
  }
  str << "]" << endl << endl;
  sortedNodes.reserve(nodes.size());

  OutputCodeFileChunck::storeCode(syntaxerContext.dependenciesStructReprFileName, str.str(), /*startEndl = */ false);
  OutputCodeFileChunck::flush();
}


NumArcs dep::graphToArcs(GraphLayingContext &src) {
  NumArcs arcs;
  for (GraphLayingContext::Arcs::value_type &fromTo : src.arcs)
    arcs.push_back(NumArc(fromTo.first, fromTo.second));
  transformSortAndUnique(arcs);
  return arcs;
}


SpecialSortingContext::SpecialSortingContext(GraphLayingContext &_src)
  : src(_src) {}

LayingTypes::NumNodes SpecialSortingContext::filterSpecial(const SpecialCathegories &filter, const function<bool(NodeId, NodeId)> &sortFunctor) {
  NumNodes result;
  Nodes::size_type nodesSize = src.nodes.size();
  for (Nodes::size_type node = 0; node < nodesSize; ++node)
    if (filter.count(src.nodes[node]->ddlCathegory()))
      result.push_back(node);

  std::stable_sort(result.begin(), result.end(), sortFunctor);
  return result;
}


bool dep::ltByUserAndName(ResolvedEntity *lDef, ResolvedEntity *rDef) {
  {
    Index *lIdx, *rIdx;
    if ((lIdx = lDef->toSelfIndex()) && (rIdx = rDef->toSelfIndex())) {
      ResolvedEntity *lTbl, *rTbl;
      if ((lTbl = lIdx->table->definition()) && (rTbl = rIdx->table->definition())) {
        if (lTbl->getDefinitionFirst() != rTbl->getDefinitionFirst())
          return ltByUserAndName(lTbl, rTbl);
      }
      if (lIdx->cathegoryIndex() == rIdx->cathegoryIndex())
        return lIdx->translatedName() < rIdx->translatedName();
      return lIdx->cathegoryIndex() < rIdx->cathegoryIndex();
    }
  }

  UserContext *lUserCtx = lDef->userContext();
  UserContext *rUserCtx = rDef->userContext();

  if (lUserCtx == rUserCtx) {
    string lEnt = nAssert(lDef->getName())->toNormalizedString();
    string rEnt = nAssert(rDef->getName())->toNormalizedString();
    return lEnt < rEnt;
  }

  if (!lUserCtx || !rUserCtx)
    return lUserCtx < rUserCtx;

  ResolvedEntity *lPkg = lDef->ownerPackage();
  ResolvedEntity *rPkg = rDef->ownerPackage();

  string lUser = (lPkg ? lPkg->getName()->toNormalizedString() : string()) + nAssert(lUserCtx->getName())->toNormalizedString();
  string rUser = (rPkg ? rPkg->getName()->toNormalizedString() : string()) + nAssert(rUserCtx->getName())->toNormalizedString();

  return lUser < rUser;
}


// < by cathegory or (= by cathegory and < by name)
bool SpecialSortingContext::operator() (NodeId l, NodeId r) const {
  ResolvedEntity *lDef = src.nodes.at(l);
  ResolvedEntity *rDef = src.nodes.at(r);
  SpecialCathegories::const_iterator lFilter = frontCathegories.find(lDef->ddlCathegory());
  SpecialCathegories::const_iterator rFilter = frontCathegories.find(rDef->ddlCathegory());

  sAssert(lFilter == frontCathegories.end() || rFilter == frontCathegories.end());
  Level lLevel = lFilter->second;
  Level rLevel = rFilter->second;

  if (lLevel == rLevel)
    return ltByUserAndName(lDef, rDef);

  return lLevel < rLevel;
}

void checkDuplicateNodes(vector<LayingTypes::NodeId> &sortedNodes, LayingTypes::Nodes &nodes) {
  std::set<LayingTypes::NodeId> checkDup;
  for (vector<LayingTypes::NodeId>::iterator it = sortedNodes.begin(); it != sortedNodes.end(); ++it) {
    if (checkDup.find(*it) != checkDup.end() && nodes[*it]->ddlCathegory() != ResolvedEntity::ModelContext_) {
      cout << "Error: Duplicate entity " << nodes[*it]->getName()->toNormalizedString() << endl;
      continue;
    }
    checkDup.insert(*it);
  }
}

LayingProcViewCtx::LayingProcViewCtx(SpecialSortingContext &_src)
  : src(_src.src)
{
  flagsVisited.resize(src.nodes.size(), false);
  childsVisited.resize(src.nodes.size(), false);
  for (const SpecialCathegories::value_type &v : _src.frontCathegories)
    allSpecialCathegories.insert(v.first);
  for (const SpecialCathegories::value_type &v : _src.backCathegories)
    allSpecialCathegories.insert(v.first);
}

void LayingProcViewCtx::addLayedNode(NodeId node) {
  layedNodes.push_back(node);
  if (node == 15558 || node == 44429)
    cout << "";
}

void GraphLayingContext::addNode(ResolvedEntity *node, NodeIds &nodeIds) {
  NodeId id = this->nodes.size();
  this->nodes.push_back(node);
  nodeIds[node] = id;
  if (id == 15558 || id == 44429)
    cout << "";
}


void LayingProcViewCtx::layingOnTraverse(NodeId node, unsigned int deepLevel) {
  if (flagsVisited[node])
    return;

  GraphLayingContext::Tree::iterator it = tree->find(node);
  if (it != tree->end())
    for (NodeId nIt : it->second) {
      if (childsVisited[nIt])
        continue;
      childsVisited[nIt] = true;
      layingOnTraverse(nIt, deepLevel + 1);
    }

  if (flagsVisited[node])
    return;

  Sm::ResolvedEntity *def = src.nodes[node];
  if (!allSpecialCathegories.count(def->ddlCathegory())) {
    addLayedNode(node);
    def->lazyAttributes()->level_ = deepLevel;
  }

  flagsVisited[node] = true;
}

void arcs2childsList(GraphLayingContext::Tree &dst, NumArcs &src) {
  for (NumArc &arc : src)
    dst[arc.from].insert(arc.to);
}

LayingTypes::NumNodes SpecialSortingContext::layingProcView() {
  // TODO: реализовать функциональность, проверяющую нарущение циклов и пытающуюся выполнить досортировку
  LayingProcViewCtx layingCtx(*this);

  NumArcs arcs = graphToArcs(src);
  GraphLayingContext::Tree dst;
  arcs2childsList(dst, arcs);

  layingCtx.tree = &dst;

  NumNodes rootNodes = diff(getConnectedNodes(arcs), toNodes(arcs));
  for (NodeId root: rootNodes)
    layingCtx.layingOnTraverse(root, 0);

  FlagsVisited::size_type maxNode = layingCtx.flagsVisited.size();

  for (NodeId i = 0; i < maxNode; ++i)
    if (!layingCtx.flagsVisited[i])
      layingCtx.layingOnTraverse(i, 0);

  return layingCtx.layedNodes;
}



void GraphLayingContext::laying() {
  SpecialSortingContext sortingCtx(*this);

  auto ltByName = [&](NodeId l, NodeId r) -> bool { return ltByUserAndName(nodes.at(l), nodes.at(r)); };
  auto addNodesChunck = [&](const NumNodes &src) { sortedNodes.insert(sortedNodes.end(), src.begin(), src.end()); };

  sortedNodes = sortingCtx.filterSpecial(sortingCtx.frontCathegories, sortingCtx);
  addNodesChunck(sortingCtx.layingProcView());
  addNodesChunck(sortingCtx.filterSpecial(sortingCtx.backCathegories , ltByName));
  checkDuplicateNodes(sortedNodes, nodes);

  representAsPyDict();
}





void DependenciesStruct::layingGraph(DependenciesStruct::SortedEntities &entities) {
  GraphLayingContext ctx(*this);
  this->entitiesForCodegen.clear();

  entities.reserve(ctx.sortedNodes.size());
  for (vector<GraphLayingContext::NodeId>::iterator it = ctx.sortedNodes.begin(); it != ctx.sortedNodes.end(); ++it) {
    ResolvedEntity *d = ctx.nodes[*it];
    if (d->eid() == 1483093)
      cout << "";
    if (d->userContext()) {
      entities.push_back(d);
    }
    else if (d->ddlCathegory() == ResolvedEntity::ModelContext_)
      continue;
    else if (d->ddlCathegory() == ResolvedEntity::SpecialKeysActor_)
      entities.push_back(d);
    else if (!d->isSystem() && !d->isSystemTemplate()) {
      cout << "Entity ";
      Codestream str;
      d->linterReference(str);
      cout << str.str() << " " << " has not user context, cathegory: " << toString(d->ddlCathegory()) << endl;
    }
  }
}





void dumpDependenciesSql(const char *dirOrFile, bool splitFiles) {
  DependenciesStruct m;
  m.init();
  m.buildTreeAndDumpSqlForDb(dirOrFile, splitFiles);
}


void DependenciesStruct::buildTreeAndDumpSqlForDb(const char *dirOrFile, bool splitFiles) {
  dumpSqlFromDb(dirOrFile, splitFiles);
}


void DependenciesStruct::checkoutUserCtx(UserContext *user, Sm::Codestream &str_vars, Sm::Codestream &str)
{
  if (user && user != curUser) {
    curUser = user;
    str      << s::connect(user);
    str_vars << s::connect(user);
  }
}

void DependenciesStruct::storeSql(SortedEntities &sortedEntities, Sm::Codestream &str, Sm::Codestream &str_vars) {
  if (sortedEntities.empty())
    return;

  curUser = 0;
  Codestream::mainStream = &str;

  Codestream::SortedGranteesClauses sortedGranteesClauses;
  StoreBaseEntity storeActions(str, str_vars, sortedGranteesClauses);

  for (SortedEntities::iterator it = sortedEntities.begin(); it != sortedEntities.end(); ++it) {
    ResolvedEntity *d = *it;
    if (!storeActions.filterEntity(d->ddlCathegory(), d))
      continue;
    UserContext *user = d->userContext();
    checkoutUserCtx(user, str_vars, str);
    str.needToGrantee.clear();
    storeActions.store(d->ddlCathegory(), d);
    storeActions.joinGrantee(d->ddlCathegory(), user);
  }

  storeActions.storeUndependedManual();
  str.join();
  str.joinPreactions();
}




StoreBaseEntity::StoreBaseEntity(Codestream &_str, Codestream &_str_vars, Codestream::SortedGranteesClauses &_sortedGranteesClauses)
  : str                  (_str),
    str_vars             (_str_vars),
    sortedGranteesClauses(_sortedGranteesClauses),
    modelActions         (syntaxerContext.model->modelActions)
{
  actions[ResolvedEntity::Sequence_        ].set(&StoreBaseEntity::storeSequence, modelActions.createSequences(), false);
  actions[ResolvedEntity::Table_           ].set(&StoreBaseEntity::storeTable   , modelActions.createTables   (), modelActions.dropTables());
  actions[ResolvedEntity::Index_           ].set(&StoreBaseEntity::storeIndex   , modelActions.createIndices  (), modelActions.dropIndices());
  actions[ResolvedEntity::IndexUnique_     ].set(&StoreBaseEntity::storeIndex   , modelActions.createIndices  (), modelActions.dropIndices());
  actions[ResolvedEntity::View_            ].set(&StoreBaseEntity::storeView    , modelActions.createViews    (), false);
  actions[ResolvedEntity::Synonym_         ].set(&StoreBaseEntity::storeSynonym , modelActions.createSynonym  (), false);
  actions[ResolvedEntity::Trigger_         ].set(&StoreBaseEntity::storeTrigger , modelActions.createTriggers (), modelActions.dropTriggers());
  actions[ResolvedEntity::MemberFunction_  ].set(&StoreBaseEntity::storeFunction, modelActions.createProcedures(), false);
  actions[ResolvedEntity::Function_        ].set(&StoreBaseEntity::storeFunction, modelActions.createProcedures(), false);
  actions[ResolvedEntity::Variable_        ].set(&StoreBaseEntity::storeVariable, modelActions.createGlobalVars(), false);
  actions[ResolvedEntity::SpecialKeysActor_].set(&StoreBaseEntity::storeTabKeys , true, false);

  setupManualEntities();
}

void StoreBaseEntity::setupManualEntities() {
  LinterSemanticResolver lsr;
  int i = 0;
  for (Sm::ManualEntity &manEntity : syntaxerContext.model->modelActions.manualEntities) {
    IdEntitySmart &reference = manEntity.reference;
    lsr.resolve(reference);
    ResolvedEntity *def = reference.entity()->unresolvedDefinition();
    if (def) {
      excludeEntities.insert(EntityMap::value_type(def, i));
    }
    else {
      cout << "Unresolved entity for exclude from codegeneration " << reference << endl;
    }
    ++i;
  }
}

void StoreBaseEntity::storeManual(Sm::ManualEntity &manEntity) {
  // Добавить запросы непосредственно в кодогенерацию
  manEntity.stored = true;
  Sm::ResolvedEntity *def = manEntity.reference.entity()->unresolvedDefinition();
  Sm::Codestream &s = (def && def->isPackageVariable()) ? str_vars : str;

  CmdCat cat = CmdCat::EMPTY;
  if (def)
    switch (def->ddlCathegory()) {
    case Variable_: cat = CmdCat::VARIABLE; break;
    case Function_: cat = CmdCat::PROC; break;
    case View_    : cat = CmdCat::VIEW; break;
    case Trigger_ : cat = CmdCat::TRIGGER; break;
    default: break;
    }

  for (IdEntitySmart::reverse_iterator it = manEntity.sqls.rbegin(); it != manEntity.sqls.rend(); ++it) {
    string sql = (*it)->toString();

    string subs = sql.substr(0, 5);
    std::transform(subs.begin(), subs.end(), subs.begin(), ::toupper);
    if (subs == "GRANT") {
      cat = CmdCat::EMPTY;
    }

    s << s::ocmd(def, cat);
    translateMultilineText(s, sql);
    s << s::endl << s::ccmd;
  }
}

void StoreBaseEntity::storeUndependedManual() {
  UserContext *curUser = NULL;

  for (Sm::ManualEntity &manEntity : syntaxerContext.model->modelActions.manualEntities) {
    if (!manEntity.stored) {
      ResolvedEntity *userDef = manEntity.reference.majorEntity()->definition();
      if (userDef) {
        UserContext *user = userDef->toSelfUserContext();
        if (user && user != curUser) {
          curUser = user;
          str      << s::connect(user);
          str_vars << s::connect(user);
        }
      }
      storeManual(manEntity);
    }
  }
}

void StoreBaseEntity::store(CathegoriesOfDefinitions::ScopedEntities cat, ResolvedEntity *def) {
  EntityMap::iterator it = excludeEntities.find(def);
  if (it != excludeEntities.end()) {
    storeManual(syntaxerContext.model->modelActions.manualEntities[it->second]);
    return;
  }
  ActionContext &ctx = actions[cat];
  if (ctx.action)
    (this->*(ctx.action))(def, ctx);
}

bool StoreBaseEntity::filterEntity(ScopedEntities cat, ResolvedEntity *def) {
  if (def && (def->isSystem() || def->isSystemTemplate()))
    return false;
  if (syntaxerContext.filterEntities && syntaxerContext.model->entityAlreadyCreatedInDb(def))
    return false; // не перезаписывать процедуры, которые уже созданы в базе

  ActionContext &ctx = actions[cat];
  if (def->toSelfTable())
    return ctx.create || (syntaxerContext.model->modelActions.flags & MODEL_ACTIONS_KEYS_MANIPULATIONS);
  if (ctx.action)
    return ctx.create;
  return true;
}

void StoreBaseEntity::joinGrantee(CathegoriesOfDefinitions::ScopedEntities cat, UserContext *user) {
  switch (cat) {
    case ResolvedEntity::View_:
      str.activatePrefixes();
      outputSortedGranteeClause(sortedGranteesClauses, str, user, false);
      str.activateActions();
      break;
    default:
      str.join();
      str.joinPreactions();
      break;
  }
}

void StoreBaseEntity::storeInternal(Codestream &str, ResolvedEntity *item, StoreBaseEntity::ActionContext &ctx) {
//  Table *tbl;
//  if ((tbl = item->toSelfTable()) && (syntaxerContext.model->modelActions.flags & MODEL_ACTIONS_KEYS_MANIPULATIONS))
//    tbl->linterDefinitionKeys(str);
  if (ctx.create) {
    item->sqlDefinition(str);
    if (item->needSemicolonAfterEntity())
      str << s::semicolon << s::endl;
  }
  else if (ctx.drop)
    item->sqlDefinition(str);
  str.join();
}

void StoreBaseEntity::storeView(ResolvedEntity *vIt, StoreBaseEntity::ActionContext &ctx) {
  storeInternal(str, vIt, ctx);
  needToGranteeToSortedGrantee(sortedGranteesClauses, str.needToGrantee);
  str.needToGrantee.clear();
  str.join();
  str << s::endl;
}

void StoreBaseEntity::storeTrigger(ResolvedEntity *vIt, StoreBaseEntity::ActionContext &ctx) {
  storeInternal(str, vIt, ctx);
  str << s::endl;
}

void StoreBaseEntity::storeFunction(ResolvedEntity *vIt, StoreBaseEntity::ActionContext &ctx) {
  Sm::Function *fun = vIt->toSelfFunction();
  if (fun->flags.isAnydataMember())
    return;
  if (!fun->isDefinition()) {
    UserContext    *userCtx       = fun->userContext();
    ResolvedEntity *pkg           = fun->ownerPackage();
    Sm::Function   *ownerFunction = 0;
    SemanticTree   *n             = fun->getSemanticNode();

    while (n && (n = n->getParent()))
      if (n->unnamedDdlEntity && (ownerFunction = n->unnamedDdlEntity->toSelfFunction()))
        break;

    cout << "Dependent function " ;
    if (userCtx)
      cout << userCtx->getName()->toNormalizedString() << ".";
    if (pkg)
      cout << pkg->getName()->toNormalizedString() << ".";
    if (ownerFunction)
      cout << ownerFunction->getName()->toNormalizedString() << "(function).";


    string fname = fun->getName()->toNormalizedString();
    if (fname.empty())
      cout << " " << fun->getLLoc().locText();
    else
      cout << fname;

    cout << " has declaration but not defined " << fun->getLLoc().toString() << endl;
    return;
  }
  if (modelActions.createProcedures() && modelActions.createGlobalVars())
    vIt->translateLocalObjects(str_vars);

  storeInternal(str, vIt, ctx);
  str << s::endl;
}

void StoreBaseEntity::storeTabKeys(Sm::ResolvedEntity *vIt, ActionContext &) {
  vIt->linterDefinition(str);
}

void StoreBaseEntity::storeVariable(ResolvedEntity *vIt, StoreBaseEntity::ActionContext &ctx) {
  if (modelActions.createGlobalVars() && vIt->isPackageVariable())
    storeInternal(str_vars, vIt, ctx);
}

void DependenciesStruct::dumpSqlFromDb(string dirOrFile, bool splitFiles) {
  SortedEntities sortedEntities;
  this->layingGraph(sortedEntities);
  if (sortedEntities.empty())
    return;

  ofstream ofile;
  if (splitFiles)
    mkdir(dirOrFile.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
  else
    ofile.open(dirOrFile);

  string usrn, name, lastUsrn;
  DumpResult dumpRes;
  LinterDumper dumper(syntaxerContext.linterUsername,  syntaxerContext.linterPassword, syntaxerContext.linterNodename);

  cout << "== Start dump entities of DB ==" << endl;

  const int printedCount = 100;
  const int lineFeed = 5;
  int allCount = 0;
  int curCount = 0;
  clock_t begin = clock();
  auto printProgress = [&allCount, &curCount, &begin]() {
    cout << "[" << setw(3) << (curCount * 100 / allCount) << "%] "
              << setw(5) << curCount << "/" << setw(5) << allCount << " "
              << setprecision(2) << diffTime(begin) << " sec\t";
    cout << flush;
  };

  // Precalculate entities
  for (SortedEntities::iterator vIt = sortedEntities.begin(); vIt != sortedEntities.end(); ++vIt) {
    switch ((*vIt)->ddlCathegory()) {
    case ResolvedEntity::View_: {
      if (!syntaxerContext.model->modelActions.createViews())
        continue;
      allCount++;
    } break;
    case ResolvedEntity::Trigger_: {
      if (!syntaxerContext.model->modelActions.createTriggers())
        continue;
      allCount++;
    } break;
    case ResolvedEntity::MemberFunction_:
    case ResolvedEntity::Function_: {
      if (!syntaxerContext.model->modelActions.createProcedures() || (*vIt)->toSelfFunction()->flags.isAnydataMember())
        continue;
      allCount++;
    } break;
    default:
      continue;
    }
  }

  for (SortedEntities::iterator vIt = sortedEntities.begin(); vIt != sortedEntities.end(); ++vIt) {
    ResolvedEntity *def = (*vIt);
    string createHead;

    switch (def->ddlCathegory()) {
    case ResolvedEntity::View_: {
      if (!syntaxerContext.model->modelActions.createViews())
        continue;
      usrn = def->userContext()->getName()->toQString();
      name = def->getName()->toNormalizedString();
      if (!dumper.dumpViewSql(usrn, name, dumpRes))
        continue;
      Codestream stream;
      stream.procMode(CodestreamState::SQL);
      stream << "VIEW " << s::cref(def);
      def->toSelfView()->translateHead(stream);
      createHead = syntaxerContext.createStatement + stream.str();
    } break;
    case ResolvedEntity::Trigger_: {
      if (!syntaxerContext.model->modelActions.createTriggers())
        continue;
      usrn = def->userContext()->getName()->toQString();
      name = def->getName()->getText();
      if (!dumper.dumpTrigSql(usrn, name, dumpRes))
        continue;
    } break;
    case ResolvedEntity::MemberFunction_:
    case ResolvedEntity::Function_: {
      if (!syntaxerContext.model->modelActions.createProcedures() || def->toSelfFunction()->flags.isAnydataMember())
        continue;
      usrn = def->userContext()->getName()->toQString();
      name = def->translatedName();
      if (!dumper.dumpProcSql(usrn, name, dumpRes))
        continue;
    } break;
    default:
      continue;
    }

    if (splitFiles) {
      string fileName = usrn + "." + name;
      transform(fileName.begin(), fileName.end(), fileName.begin(), ::toupper);
      ofile.open(dirOrFile + "/" + fileName);
    }

    if (splitFiles || lastUsrn != usrn) {
      ofile << "USERNAME " << usrn << "/" << syntaxerContext.linterPassword;
      ofile << endl << endl;
      lastUsrn = usrn;
    }

    ofile << "-- Creation date "  << dumpRes.createDate << endl;
    ofile << createHead << dumpRes.createSql << endl;
    for (string &grant : dumpRes.grantsSql) {
      ofile << grant << endl;
    }
    ofile << endl;

    if (splitFiles)
      ofile.close();

    if ((++curCount % printedCount) == 0) {
      printProgress();
      if ((curCount % (lineFeed * printedCount)) == 0)
        cout << endl;
    }
  }

  printProgress();
  cout << endl << "== End ==" << endl;
  dumper.close();
}

