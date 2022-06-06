#include <unordered_set>

#include "semantic_id.h"
#include "semantic_tree.h"
#include "semantic_function.h"
#include "semantic_collection.h"
#include "semantic_blockplsql.h"
#include "semantic_statements.h"

using namespace Sm;


bool SemanticTree::isWriteReference() const {
  SCathegory::t cat = cathegory;
  switch (cat) {
    case SCathegory::ReturnInto:
    case SCathegory::Into:
      return isList();
    case SCathegory::Assignment:
      // TODO: еще Function call argument имеющий свойство OUT
      return true;
    case SCathegory::BlockPlSql:
    case SCathegory::Function:
    case SCathegory::User:
    case SCathegory::AlgebraicCompound:
    case SCathegory::Comparsion:
      return false;
    default:
      if (parent)
        return parent->isWriteReference();
      else
        return false;
  }
}

void DependEntitiesMap::insert(const Base::value_type &p) {
  static_cast<Base*>(this)->insert(Base::value_type(p.first, p.second));
}

// entityWhichUses[outArcs] -> [inArcs]usedEntity
void DependEntitiesMap::insert(
    ResolvedEntity *usedEntity,
    ResolvedEntity *entityWhichUses,
    const Sm::DepArcContext &ctx)
{
  static const std::unordered_set<ResolvedEntity::ScopedEntities> admissibleCathegories = {
    ModelContext_, Table_, DatabaseLink_, Sequence_, CollectionMethod_, Synonym_, Package_,
    Index_, IndexUnique_, User_, MemberFunction_, MemberVariable_, Object_, Variable_, View_, Function_, Trigger_
  };

  usedEntity = usedEntity->getDefinitionFirst();
  if (!admissibleCathegories.count(usedEntity->ddlCathegory()))
    return;
  std::pair<DependEntitiesMap::iterator, bool> it
      = static_cast<Base*>(this)->insert(Base::value_type(usedEntity, DepArcContext()));

  if (!entityWhichUses)
    return;

  if (ctx.isWrite())
    entityWhichUses->lazyAttributes()->setWrite(); // определение - изменяемое (для аргументов функций)

  // установить обратную ссылку в используемой сущности
  // т.е. добавить entityWhichUses  в использемой сущности - в список ее использующих
  {
    DependEntitiesMap &childInArcs = it.first->first->lazyAttributes()->inArcs;
    // сущности, которые используют  it.first->first
    std::pair<DependEntitiesMap::iterator, bool> mIt =
        static_cast<Base&>(childInArcs).insert(DependEntitiesMap::value_type(entityWhichUses, ctx));

    if (!mIt.second)
      mIt.first->second |= ctx;
  }

  if (entityWhichUses->ddlCathegory() != ResolvedEntity::ModelContext_) {
    DependEntitiesMap::iterator parentIt = find(entityWhichUses->getDefinitionFirst());
    if (parentIt != end()) {
      DependEntitiesMap &outArcs = parentIt->first->lazyAttributes()->outArcs;
      // В список используемых сущностей outArcs - добавить используемую сущность usedEntity
      std::pair<DependEntitiesMap::iterator, bool> mIt =
          static_cast<Base&>(outArcs).insert(Base::value_type(usedEntity, DepArcContext(ctx)));
      if (!mIt.second)
        mIt.first->second |= ctx;
    }
    else
      throw 999;
  }
}


ResolvedEntity *referenceUnwrap(ResolvedEntity *def) {
  while (def) {
    if (RefAbstract *ref = def->toSelfRefAbstract())
      def = ref->refDefinition();
    else if (/*LValue *l = */ def->toSelfLValue())
      throw 999;
//      def = l->lEntity->definition();
    else
      return def;
  }
  return def;
}


void SemanticTree::addToEnititiesMap(ResolvedEntity *def, Sm::SemanticTree::UniqueEntitiesMap &dst, Sm::IdEntitySmart *ent) {
  if (!(def = referenceUnwrap(def)))
    return;
  pair<UniqueEntitiesMap::iterator,bool> it = dst.insert(UniqueEntitiesMap::value_type(def, 0));
  bool isWriteRef = isWriteReference();
  if (!it.second)
    it.first->second->concat(!isWriteRef, isWriteRef, ent);
  else
    it.first->second = new EntityAttributes(!isWriteRef, isWriteRef, ent);
}


#include <lwriter.h>

void collectDependenciesGraphOnResolvedEntity(
    ResolvedEntity      *usedEntity,
    DependEntitiesMap   &dst,
    ResolvedEntity      *entityThatUses,
    const DepArcContext &ctx)
{
  usedEntity = usedEntity->getDefinitionFirst();

  switch (usedEntity->ddlCathegory()) {
    case ResolvedEntity::Variable_:
      if (usedEntity->isPackageVariable())
        dst.insert(usedEntity, entityThatUses, ctx);
      if (Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(usedEntity->getDatatype()))
        if (Ptr<Datatype> elType = datatype->getNextDefinition()->mappedType())
          if (elType->getNextDefinition()->toSelfObject())
            dst.insert(elType->getNextDefinition()->getDefaultConstructor(), entityThatUses, ctx);
      break;

    case ResolvedEntity::Function_:
      if (!usedEntity->isSystemPartDBMS())
        dst.insert(usedEntity, entityThatUses, ctx);
      break;

    case ResolvedEntity::Assignment_:
      if (Ptr<Sm::LValue> lv = usedEntity->lvalue())
        for (IdEntitySmart::iterator it = lv->lEntity()->begin(); it != lv->lEntity()->end(); ++it)
          if (ResolvedEntity *d = (*it)->definition()) {
            dst.insert(d, entityThatUses, DepArcContext(FLAG_DEP_ARC_CONTEXT_IS_WRITE));
            if (d->isCollectionAccessor())
              dst.insert(d->toSelfCollectionMethod()->collectionDatatype(), entityThatUses, DepArcContext(FLAG_DEP_ARC_CONTEXT_IS_WRITE));
          }
      break;

    case ResolvedEntity::CollectionMethod_:
      if (usedEntity->isCollectionAccessor())
        dst.insert(usedEntity->toSelfCollectionMethod()->collectionDatatype(), entityThatUses, ctx);
      dst.insert(usedEntity, entityThatUses, ctx);
      break;

    case ResolvedEntity::MemberVariable_:
      if (Ptr<Datatype> datatype = usedEntity->getDatatype()->getFinalType())
        if (datatype->isObjectType() && !datatype->isCollectionType())
          dst.insert(datatype->getDefaultConstructor(), entityThatUses, ctx);
      dst.insert(usedEntity, entityThatUses, ctx);
      break;

    default:
      dst.insert(usedEntity, entityThatUses, ctx);
      break;
  }
}

bool isNodeInDependenciesHierarhy(ResolvedEntity *usedEntity) {
  Sm::ResolvedEntity::ScopedEntities cat = usedEntity->ddlCathegory();
  switch (cat) {
    case ResolvedEntity::MemberVariable_:
    case ResolvedEntity::MemberFunction_:
    case ResolvedEntity::Trigger_:
    case ResolvedEntity::Package_:
    case ResolvedEntity::View_:
      return true;
    case ResolvedEntity::Variable_:
      return usedEntity->isPackageVariable();
    case ResolvedEntity::Function_:
      return !usedEntity->isSystemPartDBMS();
    default:
      return false;
  }
  return false;
}

void SemanticTree::collectDependenciesGraph(DependEntitiesMap &dst, ResolvedEntity *entityThatUses, DepArcContext ctx) {
  if (!unentered())
    return;
  openNode();

  if (isList() && (cathegory == SCathegory::ReturnInto || cathegory == SCathegory::Into))
    ctx.setWrite();

  if (unnamedDdlEntity && unnamedDdlEntity->translatedAsExecuteWithoutDirect())
    ctx.setExecuteWithoutDirect();

  auto traverseCallArglistGraph = [&](Id *id) {
    if (id->callArglist)
      for (CallArgList::value_type &c : *(id->callArglist))
        if (SemanticTree *n = c->getSemanticNode())
          n->collectDependenciesGraph(dst, entityThatUses, ctx);
  };

  if (referenceName_) {
    ResolvedEntity *usedEntity;
    if (nametype == DECLARATION || nametype == DEFINITION) { // declaration or definition
      for (Ptr<Id> &id : *referenceName_)
        if (id && (usedEntity = id->definition())) {
          collectDependenciesGraphOnResolvedEntity(usedEntity, dst, entityThatUses, ctx);

          // для объявлений и определений - список зависимостей формировать с установкой возможно новой сущности, которая
          // использует данные объявления и определения
          usedEntity = usedEntity->getDefinitionFirst();
          if (isNodeInDependenciesHierarhy(usedEntity)) /*isReference = false*/
            entityThatUses = usedEntity;

          traverseCallArglistGraph(id);
        }
    }
    else { // reference

      if (unnamedDdlEntity && !unnamedDdlEntity->getConcreteDefinition()->isSystemPartDBMS()) {
        ctx.references.insert(unnamedDdlEntity);
      }
      else if (cathegory == Sm::SCathegory::FromTableReference && parent->unnamedDdlEntity) {
        ctx.references.insert(parent->unnamedDdlEntity);
      }


      for (Ptr<Id> &id : *referenceName_)
        if (id && (usedEntity = id->definition())) {
          collectDependenciesGraphOnResolvedEntity(usedEntity, dst, entityThatUses, ctx);
          traverseCallArglistGraph(id);
        }
    }
  }

  for (Childs::value_type c : childs)
    c->collectDependenciesGraph(dst, entityThatUses, ctx);
}

bool SemanticTree::collectEntitiesThatChangedVariable(ResolvedEntity *var, DependEntitiesMap &dst, ResolvedEntity *ent) {
  ResolvedEntity::ScopedEntities cat = ResolvedEntity::EMPTY__;
  if (ResolvedEntity *def = ddlEntity()) {
    def = def->getDefinitionFirst();
    switch (nametype) {
      case DECLARATION:
      case DEFINITION:
        if (cathegory == SCathegory::Label)
          return false;
        cat = def->ddlCathegory();
        // Функции нельзя слепо добавлять.
        // Нужно строить иерархию вызовов обнаруженной функции, и если один из корней данной иерархии -
        // триггер или инициализатор пакета - добавлять цепочку от триггера до функции
        // А еще можно для каждого триггера и блока инициализации построить предварительно сеть вызовов (с обратными ссылками)
        // А каждой функции - задать список узлов этой сети (где она вызывается).
        switch (cat) {
          case ResolvedEntity::Package_:
            return def->getSemanticNode()->collectEntitiesThatChangedVariable(var, dst, ent);
          case ResolvedEntity::Function_:
          case ResolvedEntity::Trigger_:
          case ResolvedEntity::MemberFunction_:
            ent = def;
            if (Sm::BlockPlSql *blk = ent->childCodeBlock())
              return blk->getSemanticNode()->collectEntitiesThatChangedVariable(var, dst, ent);
            return false;
          default:
            break;
        }
        break;
      default:
        break;
    }
    if (Ptr<Sm::LValue> lv = def->lvalue())
      if (ResolvedEntity *d = lv->refDefinition())
        if (d->eqByVEntities(var)) {
          dst.insert(ent, ent, Sm::DepArcContext(FLAG_DEP_ARC_CONTEXT_IS_WRITE));
          return true;
        }
  }

  for (Childs::iterator it = childs.begin(); it != childs.end(); ++it)
    if ((*it)->collectEntitiesThatChangedVariable(var, dst, ent))
      if (cat == ResolvedEntity::Function_ || cat == ResolvedEntity::MemberFunction_ || cat == ResolvedEntity::Trigger_)
        return true;

  return false;
}

void Sm::SemanticTree::getAllCodeBlockReferences(UniqueEntitiesMap &dst) {
//  if (__streeId__ == 686350 || __streeId__ == 686366)
//    cout << "";
  switch (nametype) {
    case REFERENCE:
    case EXTDB_REFERENCE: {
      if (unnamedDdlEntity && unnamedDdlEntity->isCodeBlockEntity())
        addToEnititiesMap(unnamedDdlEntity, dst, referenceName_);

      for (IdEntitySmart::value_type &v : *referenceName_)
        if (ResolvedEntity *d = v->unresolvedDefinition())
          if (d != unnamedDdlEntity && d->isCodeBlockEntity())
            addToEnititiesMap(d, dst, referenceName_.object());
    }

    default:
      break;
  }
  for (Childs::value_type c : childs)
    c->getAllCodeBlockReferences(dst);
}

