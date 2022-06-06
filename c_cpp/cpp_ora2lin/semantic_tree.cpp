#include <map>
#include <set>
#include "siter.h"
#include "model_context.h"
#include "semantic_datatype.h"
#include "semantic_sql.h"

extern SyntaxerContext syntaxerContext;

namespace Sm {

// определения конструкторов для SemanticTree {

size_t SemanticTree::streeGlobalCounter__ = 0;
ResolvingProgress SemanticTree::resolvingProgress;

size_t SemanticTree::getStreeGlobalCounter__() {
  size_t v = streeGlobalCounter__++;
  if (v == 2746606)
    cout << "";
  return v;
}


SemanticTree &SemanticTree::operator=(const SemanticTree &tr) {
  parent           = tr.parent;
  positionInParent = tr.positionInParent;
  unnamedDdlEntity = tr.unnamedDdlEntity;
  cathegory        = tr.cathegory;
  nametype         = tr.nametype;
  flags            = tr.flags;
  referenceName_   = tr.referenceName_;
  levelNamespace   = tr.levelNamespace;
  childNamespace   = tr.childNamespace;
  childs           = tr.childs;
  return *this;
}


void SemanticTree::deleteInvalidChilds() {

  // Наговнокодили так наговнокодили....
  // Из за кода ResolvingContext::createCollectionAccessOperator - удаление будет вести к падениям при поиске зависимостей.
  //  Ptr<Type::collection_methods::AccessToItem> ResolvingContext::createCollectionAccessOperator(ResolvedEntity *currentPartDef, SemanticTree *refSNode)
  //  Ptr<Sm::Type::collection_methods::AccessToItem> accessOp = new Sm::Type::collection_methods::AccessToItem(currentPartDef);
//  syntaxerContext.model->resolvedAccessOperators.push_back(accessOp);
//  accessOp->setSemanticNode(refSNode);
// Изменить ситуацию можно, если перестраивать RefExpr в что-то другое, что потом будет копироваться в deepCopy
// а не связывать с ним definition на акцессор

  return;
  SyntaxerContext::Stage saved = syntaxerContext.stage;
  syntaxerContext.stage = SyntaxerContext::INVALIDATING_CLEANUP;

  for (Childs::iterator it = childs.begin(); it != childs.end(); ) {
    if ((*it)->isInvalid()) {


      delete *it;
      it = childs.erase(it);
    }
    else
      ++it;
  }


  syntaxerContext.stage = saved;
}

SemanticTree::~SemanticTree() {
  switch (syntaxerContext.stage) {
    case SyntaxerContext::GLOBAL_CLEANUP:
    case SyntaxerContext::INVALIDATING_CLEANUP:
      for (SemanticTree *v : childs) {
        pair<DeletedNodes::iterator, bool> it = deletedNodes.insert(v);
        if (it.second)
          delete v;
      }
      break;

    case SyntaxerContext::CONTEXT_ANALYZE:
      throw 999;

    default: {
      Childs copy = childs;
      for (SemanticTree *v : copy) {
        pair<DeletedNodes::iterator, bool> it = deletedNodes.insert(v);
        if (it.second)
          delete v;
      }
      if (parent) // не фаза финальной очистки
        for (Childs::iterator it = parent->childs.begin(); it != parent->childs.end(); ++it)
          if (*it == this) {
            parent->childs.erase(it);
            break;
          }
    } break;
  }
}

string SemanticTree::nametypeToString() {
  switch (nametype) {
    case EMPTY:                return "EMPTY";
    case NEW_LEVEL:            return "NEW_LEVEL";
    case REFERENCE:            return "REFERENCE";
    case EXTDB_REFERENCE:      return "EXTDB_REFERENCE";
    case DATATYPE_REFERENCE:   return "DATATYPE_REFERENCE";
    case DECLARATION:          return "DECLARATION";
    case DEFINITION:           return "DEFINITION";
    case LAST_NAMETYPE_IDX:    return "LAST_NAMETYPE_IDX";
  }
  return "";
}


uint32_t SemanticTree::getPackedCathegory(Sm::SCathegory::t cat, NameType t) {
  PackedCathegory nodeValue;
  nodeValue.cat = 0;
  nodeValue.fields.cathegory = (uint16_t)(cat);
  nodeValue.fields.nametype  = (uint16_t)(t);
  return nodeValue.cat;
}

uint32_t SemanticTree::getPackedCathegory() const {
  return getPackedCathegory(cathegory, nametype);
}

void SemanticTree::setRecursiveFlags(FlagsType v) {
  flags |= v;
  for (auto &c : childs)
    c->setRecursiveFlags(v);
}

string SemanticTree::getLocText() {
  stringstream str;
  if (referenceName_ && !referenceName_->empty()) {
    cl::filelocation l = referenceName_->back()->getLLoc();
    l.loc.end = referenceName_->front()->getLLoc().loc.end;
    str << l.locTextNoEndl();
  }
  else if (unnamedDdlEntity)
    str << unnamedDdlEntity->getLLoc().locTextNoEndl();
  return str.str();
}



SemanticTree::SemanticTree(SCathegory::t _cathegory, NameType ntype, const ResolvedEntity *def)
  : unnamedDdlEntity(const_cast<ResolvedEntity*>(def)), cathegory(_cathegory), nametype(ntype) {}


SemanticTree::SemanticTree(SCathegory::t _cathegory, const ResolvedEntity *def)
  : unnamedDdlEntity(const_cast<ResolvedEntity*>(def)), cathegory(_cathegory) {}


void SemanticTree::setIdentificators(NameType t, Ptr<Id> v) {
  if (v) {
    v->semanticNode(this);
    switch (t) {
      case DECLARATION:
      case DEFINITION:
        referenceName_ = new IdEntitySmart(v);
        break;
      default:
        if (!v->empty()) {
          if (referenceName_)
            referenceName_->push_back(v);
          else
            referenceName_ = new IdEntitySmart(v);
        }
        break;
    }
  }
}


void SemanticTree::setIdentificators(NameType t, Ptr<Id2> v) {
  if (v) {
    v->semanticNode(this);
    switch (t) {
      case EMPTY:
      case NEW_LEVEL:
        return;
      case DECLARATION:
      case DEFINITION:
      default:
        if (referenceName_)
          v->toIdEntity(*referenceName_);
        else
          referenceName_ = new IdRef(v);
        break;
    }
  }
}


SemanticTree::SemanticTree(Ptr<Id> v, NameType t, SCathegory::t _cathegory, ResolvedEntity *def)
  : /*SingleSmart(1), */unnamedDdlEntity(def), cathegory(_cathegory), nametype(t) { setIdentificators(t, v);  }


SemanticTree::SemanticTree(Ptr<Id2> v, NameType t, SCathegory::t _cathegory, ResolvedEntity *def)
  : unnamedDdlEntity(def), cathegory(_cathegory), nametype(t)/*, ddlName_(v)*/ { setIdentificators(t, v); }


SemanticTree::SemanticTree(Ptr<Datatype> v, NameType t, SCathegory::t _cathegory)
  : cathegory(_cathegory), nametype(t), referenceName_(v ? v->tid : Ptr<IdEntitySmart>())
{
  if (!referenceName_ || referenceName_->empty())
    throw 999;
  referenceName_->semanticNode(this);
}


SemanticTree::SemanticTree(Sm::IdEntitySmart *v, NameType t, SCathegory::t _cathegory)
  : cathegory(_cathegory), nametype(t), referenceName_(v)
{
  if (!referenceName_ || referenceName_->empty())
    throw 999;
  referenceName_->semanticNode(this);

  if (referenceName_->entity()) {
    if (referenceName_->entity()->callArglist)
      cathegory = SCathegory::Function;
    for (IdEntitySmart::value_type &it : *referenceName_)
      if (it->callArglist)
        for (CallArgList::value_type &cIt : *(it->callArglist))
          if (cIt)
            cIt->collectSNode((SemanticTree*)this);
  }
}


SemanticTree::SemanticTree(Ptr<Id> v, SCathegory::t _cathegory)
  : cathegory(_cathegory), nametype(DEFINITION), referenceName_(new IdEntitySmart(v))// , declarationName_(v)
{
  if (v)
    v->semanticNode(this);
}

// }

void SemanticTree::setPlContextTrue()
{
  flags |= FLAG_SEMANTIC_TREE_IS_PL_CONTEXT;

  switch (cathegory) {
    case SCathegory::SelectedField:
    case SCathegory::From:
    case SCathegory::FromSingle:
    case SCathegory::FromJoin:
    case SCathegory::OrderBy:
    case SCathegory::QueryPseudoField:
    case SCathegory::QueryBlockField:
    case SCathegory::FactoringItem:
    case SCathegory::WhereClause:
    case SCathegory::HierarhicalClause:
      setNotPlContext();
      break;
    default:
      break;
  }
  for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::setPlContextTrue));
}


void SemanticTree::setNotPlContext() {
  flags |= FLAG_SEMANTIC_TREE_IS_NOT_PL_CONTEXT;
  for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::setNotPlContext));
}


void SemanticTree::setPlContext() {
  switch (cathegory) {
    case SCathegory::Function:
      if (nametype == DEFINITION || nametype == DECLARATION) {
        setPlContextTrue();
        return;
      } break;
    case SCathegory::BlockPlSql:
      setPlContextTrue();
      return;
    case SCathegory::Package:
      switch (nametype) {
        case SemanticTree::DECLARATION:
        case SemanticTree::DEFINITION:
          setPlContextTrue();
          return;
        default:
          break;
      }
      //pass-through
    default:
      for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::setPlContext));
      break;
  }
}


ResolvedEntity* SemanticTree::ddlEntity() const {
  if (ResolvedEntity *p = refDefinition())
    return p;
//  else if (ddlName_ && ddlName_->definition())
//    return ddlName_->definition();
//  else if (declarationName_)
//    return declarationName_->definition();
  else if (unnamedDdlEntity)
    return unnamedDdlEntity;

  return 0;
}


ResolvedEntity* SemanticTree::uddlEntity() const {
  if (unnamedDdlEntity)
    return unnamedDdlEntity;
  else
    return (ddlEntity());
}


bool SemanticTree::empty() const {
  return
      refEmpty() && childs.empty()
//      &&
//      (!ddlName_         || ddlName_->empty()) &&
//      (!declarationName_ || declarationName_->empty()) &&
//      !alias_
      ;
}


SemanticTree *SemanticTree::alias(SemanticTree *node) {
  if (node) {
    node->setIsAlias();
    if (parent)
      parent->addChild(node);
    else
      throw 999;
//    alias_ = node;

//    alias_->alias_ = (SemanticTree*)this;
    setIsBaseForAlias();
  }
  return (SemanticTree*)this;
}


void SemanticTree::setParentForChilds(SemanticTree::Childs::iterator posInParent) {
  positionInParent = posInParent;
  for (SemanticTree::Childs::iterator it = childs.begin(); it != childs.end(); ++it) {
    (*it)->parent = this;
    (*it)->setParentForChilds(it);
  }
//  if (SemanticTree *a = alias())  // псевдоним будет находиться на том же уровне, что и всё остальное.
//    if (a->isAlias()) {
//      a->parent = parent;
//      a->setParentForChilds(posInParent);
//    }
}


void SemanticTree::check() {
  if (cathegory == SCathegory::AlgebraicCompound)
    unnamedDdlEntity->getDatatype();

  for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::check));
//  if (SemanticTree *a = alias())
//    if (a->isAlias())
//      a->check();
}



size_t SemanticTree::bLevelNamespaceSize(smart::SmartptrSet &s) {
  size_t sz = 0;
  if (levelNamespace)
    sz += levelNamespace->bSize(s);
  if (childNamespace)
    sz += childNamespace->bSize(s);
  for (Childs::value_type &v : childs)
    sz += v->bLevelNamespaceSize(s);
  return sz;
}

void SemanticTree::removeChildFromParent(SemanticTree *child) {
  if (!child)
    throw 999;
  SemanticTree *parent = child->parent;
  if (!parent)
    throw 999;
  child->parent = 0;
  child->positionInParent = child->childs.end();
  parent->childs.erase(child->positionInParent);
}

void SemanticTree::deleteChild(SemanticTree *child) {
  removeChildFromParent(child);
  delete child;
}

bool SemanticTree::isFringeOfStatementsLevel() const {
  switch (nametype) {
    case DECLARATION:
    case DEFINITION:
      break;
    default:
      return false;
  }
  switch (cathegory) {
    case SCathegory::Function:
    case SCathegory::Trigger:
    case SCathegory::Package:
      return true;
    default:
      throw 999;
  }
  return false;
}

void SemanticTree::cleanup() { deletedNodes.clear(); }


size_t SemanticTree::bSize(smart::SmartptrSet &s) {
  if (!s.insert(this).second)
    return 0;
  size_t sz = sizeof(SemanticTree);
  if (referenceName_)
    sz += referenceName_->bSize(s);
  for (Childs::value_type &v : childs)
    sz += v->bSize(s);

  return sz;
}

void SemanticTree::enumNodes(EnumFunctor func, EnumList &list) {
  for (Childs::value_type &node : childs) {
    if (func(node))
      list.push_back(node);
    node->enumNodes(func, list);
  }
}

void SemanticTree::enumNodesByCat(SCathegory::t cat, EnumList &list) {
  for (Childs::value_type &node : childs) {
    if (node->cathegory == cat)
      list.push_back(node);
    node->enumNodesByCat(cat, list);
  }
}


size_t LevelResolvedNamespace::bSize(smart::SmartptrSet &s) {
  if (!s.insert(this).second)
    return 0;
  size_t sz = sizeof(LevelResolvedNamespace);
  sz += sizeof(value_type) * getContainerSize(static_cast<const BaseType&>(*this));
  for (value_type &v : *this)
    sz += getContainerSize(v.first) + v.second->bSize(s);

  for (Childs::value_type &v : childs)
    sz += v->bSize(s);
  return sz;
}

size_t VEntities::bSize(smart::SmartptrSet &s) {
  if (!s.insert(this).second)
    return 0;
  size_t sz = sizeof(VEntities);
  sz += getContainerSize(overloadedFunctions) * sizeof(OverloadedFunctions::value_type);
  for (OverloadedFunctions::value_type &fun : overloadedFunctions)
    sz += fun.second->bSize(s);

  sz += getContainerSize(overloadedNodes) * sizeof(GlobalOverloadedNodes::value_type);
  for (GlobalOverloadedNodes::value_type &fun : overloadedNodes)
    sz += fun.second->bSize(s);

  sz += getContainerSize(functions) * sizeof(Container::value_type);
  if (variables)
    sz += variables->bSize(s);
  sz += getContainerSize(others) * sizeof(std::set<ResolvedEntity*, Sm::LE_ResolvedEntities>::value_type);

  return sz;
}

size_t EquallyEntities::bSize(smart::SmartptrSet &s) {
  if (!s.insert(this).second)
    return 0;
  size_t sz = sizeof(EquallyEntities);
  sz += getContainerSize(declarations) * sizeof(Container::value_type);
  sz += getContainerSize(definitions) * sizeof(Container::value_type);
  return sz;
}

void SemanticTree::addChildForce(Sm::SemanticTree *node) {
  childs.push_back(node);
  node->parent = this;
  node->positionInParent = --(childs.end());
}


void SemanticTree::addChild(SemanticTree *node) {
  if (node)
    addChildForce(node);
}


Sm::SemanticTree *SemanticInterfaceBase::toSTree() const { return toSTreeBase(); }


Sm::SemanticTree *SemanticInterface::toSTree() const { return toSTreeBase(); }


void SemanticTree::deleteFromParent() {
  if (parent) {
    parent->childs.erase(positionInParent);
    positionInParent = parent->childs.end();
    parent = 0;
  }
}


bool Sm::SemanticTree::containsEntity(ResolvedEntity *entity) {
  if (unnamedDdlEntity && unnamedDdlEntity->containsEntity(entity))
    return true;
  else if (ResolvedEntity *def = ddlEntity())
    if (def->containsEntity(entity))
      return true;
  for (Childs::value_type c : childs)
    if (c->containsEntity(entity))
      return true;
  return false;
}





void CollectUsingByStatements::preaction(StatementInterface *stmt) {
  if (blockPlSqlOwners)
    if (Statement *s = stmt->toSelfStatement() )
      if (BlockPlSql *b = s->toSelfBlockPlSql())
        blockPlSqlOwners->push_back(b);
  if (stmtsStack)
    stmtsStack->push_front(stmt);
}


void CollectUsingByStatements::postaction(Sm::StatementInterface *) {
  if (blockPlSqlOwners && blockPlSqlOwners->size())
  if(stmtsStack)
    stmtsStack->pop_front();
}

size_t SemanticTreeDdlEntityAttribute::getSid()
{
  static size_t x;
  ++x;
  return x;
}

ResolvedEntity *SemanticTreeDdlEntityAttribute::operator=(ResolvedEntity *i) {
  if (sid == 1328347)
    cout << "";
  return value = i;
}


SemanticTreeDdlEntityAttribute::SemanticTreeDdlEntityAttribute(const SemanticTreeDdlEntityAttribute &oth)
  : value(oth.value)
{
  if (sid == 1328347)
    cout << "";
}

SemanticTreeDdlEntityAttribute::SemanticTreeDdlEntityAttribute()
  : value(0)
{
  if (sid == 1328347)
    cout << "";
}

SemanticTreeDdlEntityAttribute::SemanticTreeDdlEntityAttribute(ResolvedEntity* oth)
  : value(oth)
{
  if (sid == 1328347)
    cout << "";
}

string CommonDatatypeCast::castedCathegoryToString(TypeCathegory c) {
  switch (c) {
    case CHAR_VARCHAR:            return "CHAR_VARCHAR";
    case NCHAR:                   return "NCHAR";
    case NVARCHAR:                return "NVARCHAR";
    case LIKE_NUMBER:             return "LIKE_NUMBER";
    case DATE:                    return "DATE";
    case CLOB:                    return "CLOB";
    case NUMERIC_VALUE:           return "NUMERIC_VALUE";
    case NUMERIC_VALUE_0:         return "NUMERIC_VALUE_0";
    case QUOTED_SQL_EXPR_ID:      return "QUOTED_SQL_EXPR_ID";
    case EMPTY_ID:                return "EMPTY_ID";
    case BOOL_:                   return "BOOL_";
    case NULLCAT:                 return "NULLCAT";
    case UNINITIALIZED_CATHEGORY: return "UNINITIALIZED_CATHEGORY";
  }
  return "";
}



}

namespace Sm {


  void SemanticTree::clearDefinitions() {
    if (referenceName_)
      for (Ptr<Id> &n : *referenceName_)
        n->definition(0);
    //  if (ddlName_) {
    //    if (ddlName_->id[0])
    //      ddlName_->id[0]->definition(0);
//    if (ddlName_->id[1])
//      ddlName_->id[1]->definition(0);
//  }
//  if (declarationName_)
//    declarationName_->definition(0);
  unnamedDdlEntity = 0;
}


Ptr<Id> SemanticTree::entityName() const {
  Ptr<Id> n;
  if (//(n = declarationName()) || (n = ddlNameEntity()) ||
      (n = refEntity()))
    return n;
  return n;
}

Ptr<Id> SemanticTree::refEntity() const { return referenceName_ ? referenceName_->entity() : Ptr<Id>(); }

ResolvedEntity *SemanticTree::refUnresolvedDefinition() const {
  if (Id *ent = refEntity())
    return ent->unresolvedDefinition();
  return 0;
}

bool SemanticTree::isDeclarationOrDefinition() const {
  switch (nametype) {
    case DECLARATION:
    case DEFINITION:
      return true;
    default:
      return false;
  }
  return false;
}


void update2def(Ptr<Sm::Id> ent, Sm::ResolvedEntity *from, Sm::ResolvedEntity *to) {
  if (!ent)
    return;
  if (Sm::ResolvedEntity *def = ent->definition())
    if (def == from || def == from->getNextDefinition())
      ent->definition(to);
}


void SemanticTree::changeReferences2(ResolvedEntity *from, ResolvedEntity *to) {
  if (unnamedDdlEntity)
    if (unnamedDdlEntity == from || unnamedDdlEntity == from->getNextDefinition() )
      unnamedDdlEntity = to;
  update2def(referenceName_->entity(), from, to);
//  if (ddlName_)
//    update2def(ddlName_->entity(), from, to);
//  if (declarationName_)
//    update2def(declarationName_, from, to);
  for (SemanticTree *c : childs)
    c->changeReferences2(from, to);
}


template <>
void CollectSNode(Sm::SemanticTree *node, const Sm::List<StatementInterface> &obj) {
  for (List<StatementInterface>::const_iterator it = obj.begin(); it != obj.end(); ++it)
    it->object()->collectSNode(node);
}


void Declaration::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }


void ArgumentNameRef::collectSNode(SemanticTree *n) const { id->collectSNode(n); }


SemanticTree *table::OidIndex::toSTree() const { return name ? name->toSNodeRef(SCathegory::Index) : (SemanticTree*)0; }
void NumericValue::collectSNode(SemanticTree *n) const { SNODE(n); }
void NullExpr::collectSNode(SemanticTree *n) const { SNODE(n);  clrSTreeOwner(); }

void OrderByItem::collectSNode(SemanticTree *n) const { (void)n; CTREE(expr); }
void JoinCondition::collectSNode(SemanticTree *n) const { (void)n; CTREE(condition); }
void GroupBySqlExpr::collectSNode(SemanticTree *n) const { (void)n; CTREE(sqlExpr); }
void TimeExprTimezone::collectSNode(SemanticTree *n) const { (void)n; CTREE(timezone); }
void FromJoin::collectSNode(SemanticTree *n) const { (void)n; CTREE(lhs); CTREE(op); CTREE(rhs); }
void FlashbackQueryClause::collectSNode(SemanticTree *n) const { (void)n; CTREE(asOf.expr); CTREE(versionBetweenAsOf.range[0]); CTREE(versionBetweenAsOf.range[1]); }
void HierarhicalClause::collectSNode(SemanticTree *n) const { CTREE(connectCondition); CTREE(startWithCondition); }


void table::field_property::PhysicalProperties::collectSNode(SemanticTree *n) const { if (tablespace) n->addChild(tablespace->toSNodeRef(SCathegory::Tablespace)); }
void table::field_property::XmlField::collectSNode(SemanticTree *n) const { if (name) n->addChild(name->toSNodeRef(SCathegory::Field)); }

void table::field_property::VarrayField::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(fieldRef, SemanticTree::REFERENCE, SCathegory::Field);
  CTREE(substitutableProperty);
  node->addChild(n);
}

void table::field_property::NestedTable::collectSNode(SemanticTree *node) const {
  if (name && name->name) {
    SemanticTree *n = new SemanticTree(name->name, SemanticTree::REFERENCE, SCathegory::Field);
    CTREE(substitutableProperty);
    CollectSNode(n, fieldsProperties);
    node->addChild(n);
  }
}

void table::field_property::ObjectField::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(name, SemanticTree::REFERENCE, SCathegory::Field);
  CTREE(substitutableProperty);
  node->addChild(n);
}

void table::field_property::ObjectProperties::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(fieldRef, SemanticTree::REFERENCE, SCathegory::Field);
  CTREE(defaultExpr)
  sAssert(constraints);
  node->addChild(n);
}

void table::TableProperties::collectSNode(SemanticTree *n) const {
  CollectSNode(n, fieldsProperties);
  CTREE(asSubquery);
}


SemanticTree *view::XmlSchemaId::toSTreeBase() const {
  SemanticTree *n= xmlSchemaRef->toSNodeDef(SCathegory::XmlSchema, this);
  n->addChild(xmlElementRef->toSNodeDef(SCathegory::XmlElement, this));
  return n;
}


SemanticTree *view::XmlReference::toSTree() const {
  if (reference) {
    SemanticTree *n = reference->toSTree();
    CollectSNode(n, objectIdentifiers);
    return n;
  }
  return 0;
}

void CaseIfThen::collectSNode(SemanticTree *n) const {
  (void)n;
  CTREE(condition);
  CTREE(action);
}



}


struct PrintSTreeCathegoriesGraph {
  typedef map<uint32_t, std::map<uint32_t, uint32_t> > Cathegories;

  Cathegories cathegories;
  std::set<uint32_t> leafNodes;

  typedef pair<Sm::SCathegory::t, Sm::SemanticTree::NameType> PairCat;

  PairCat unwrapCat(uint32_t cat) {
    Sm::SemanticTree::PackedCathegory nodeValue;
    nodeValue.cat = cat;
    return PairCat((PairCat::first_type)(nodeValue.fields.cathegory), (PairCat::second_type)(nodeValue.fields.nametype));
  }


  void collectCathegories(Sm::SemanticTree::Childs::value_type node, uint32_t parentValue) {
    if (!node)
      return;
    uint32_t cat = node->getPackedCathegory();
    Cathegories::mapped_type &m = cathegories[parentValue];

    pair<Cathegories::mapped_type::iterator, bool> res = m.insert(Cathegories::mapped_type::value_type(cat, 1));
    if (!res.second)
      ++(res.first->second);
    for (Sm::SemanticTree::Childs::value_type &c : node->childs)
      collectCathegories(c, cat);
  }


  void collectCathegoriesUpper(Sm::SemanticTree::Childs::value_type node, int level = 0) {
    if (!node)
      return;
    uint32_t childValue = node->getPackedCathegory();

    uint32 parentValue = 0;
    Sm::SemanticTree *parent = node->getParent();
    if (parent)
      parentValue = parent->getPackedCathegory();


    Cathegories::mapped_type &m = cathegories[parentValue];
    if (!level)
      leafNodes.insert(childValue);

    pair<Cathegories::mapped_type::iterator, bool> res = m.insert(Cathegories::mapped_type::value_type(childValue, 1));
    if (!res.second)
      ++(res.first->second);

    if (parent && level < 2)
      collectCathegoriesUpper(parent, level + 1);
  }


  string getNumNodename(uint32_t n) {
    stringstream str;
    str << "n" << n;
    return str.str();
  }

  string getNodename(uint32_t n, string space = "_") {
    stringstream str;
    PairCat node = unwrapCat(n);
    str << Sm::debugSCathegoryConvert(node.first);
    switch (node.second) {
      case Sm::SemanticTree::EMPTY:
        break;
      case Sm::SemanticTree::NEW_LEVEL:          str << space << "level";     break;
      case Sm::SemanticTree::REFERENCE:          str << space << "ref";       break;
      case Sm::SemanticTree::EXTDB_REFERENCE:    str << space << "extdb" << space << "ref"; break;
      case Sm::SemanticTree::DATATYPE_REFERENCE: str << space << "datatype";  break;
      case Sm::SemanticTree::DECLARATION:        str << space << "decl";      break;
      case Sm::SemanticTree::DEFINITION:         str << space << "def";       break;
      case Sm::SemanticTree::LAST_NAMETYPE_IDX:  str << space << "ERROR: last idx";       break;
    }
    return str.str();
  }

  string nodename(uint32_t n) { return getNodename(n, "_"); }
//  string nodename(uint32_t n) { return getNumNodename(n); }

  void printCollectedValues() {
    std::set<uint32_t> nodes;
    for (Cathegories::value_type &c : cathegories) {
      nodes.insert(c.first);
      for (Cathegories::mapped_type::value_type &c1 : c.second)
        nodes.insert(c1.first);
    }

    stringstream str;
    str << "digraph STreeCathegories {" << endl;
    str << "  rankdir=LR;" << endl;

    for (uint32_t n : nodes) {
      str << "  " << nodename(n) << " [label=\"" << getNodename(n, " ") << "\"";
      if (leafNodes.count(n))
        str << ",color=lightblue,style=filled";
      str << "];" << endl;
    }


    for (Cathegories::value_type &from : cathegories)
      for (Cathegories::mapped_type::value_type &to : from.second)
        str << "  " << nodename(from.first) << " -> " << nodename(to.first) << " [label=\"" << to.second << "\"];" << endl;

    str << "}";
    OutputCodeFile::storeCode(syntaxerContext.sortedErrorLogBase + "_scathegoryGraph.dot", str.str());
  }
};


PrintSTreeCathegoriesGraph printSTreeCathegoriesGraphPrinter; // static

void Sm::SemanticTree::collectSCathegoryUpperGraph() {
  printSTreeCathegoriesGraphPrinter.collectCathegoriesUpper(this);
}

void Sm::SemanticTree::printSCathegoryGraph() {
  printSTreeCathegoriesGraphPrinter.printCollectedValues();
}



template <>
Sm::SemanticTree* Sm::ToSTreeRef<Sm::Id>(const Ptr<Id> &obj, SCathegory::t t) {
  if (obj)
    return new SemanticTree(obj, SemanticTree::REFERENCE, t);
  else
    return new SemanticTree(t, SemanticTree::REFERENCE);
}

template <>
Sm::SemanticTree *Sm::ToSTreeRef<Sm::Id2>(const Ptr<Id2> &obj, SCathegory::t t) {
  if (obj)
    return new SemanticTree(obj, SemanticTree::REFERENCE, t);
  else
    return new SemanticTree(t, SemanticTree::REFERENCE);
}





// vim:foldmethod=marker:foldmarker={,}
