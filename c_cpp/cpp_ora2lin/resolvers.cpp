#include <set>
#include <unordered_map>
#include "model_head.h"
#include "resolvers.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "semantic_function.h"
#include "semantic_collection.h"
#include "semantic_id.h"
#include "semantic_table.h"
#include "semantic_blockplsql.h"
#include "semantic_expr_select.h"
#include "semantic_statements.h"

extern Sm::ResolvingContext *currentResolvingContext;
extern SyntaxerContext syntaxerContext;

using namespace Sm;

template <typename T>
inline int findPosOfMaxPower(const vector<T> &powersVector) {
  int maxPos   = numeric_limits<int>::min();
  double maxPower = numeric_limits<double>::min();
  for (typename vector<T>::const_iterator it = powersVector.begin(); it != powersVector.end(); ++it) {
    if ( it->positives > maxPower ) {
      maxPower = it->positives;
      maxPos   = std::distance(powersVector.begin(), it);
    }
  }
  return maxPos;
}


bool debugIsLexem(const string &str, const string &lexem, Id *name, int line = -1, int column = -1) {
  if (str == lexem) {
    if (name) {
      if (line >= 0 && name->getLLoc().loc.begin.line != (unsigned int)line)
        return false;
      if (column >= 0 && name->getLLoc().loc.begin.column != (unsigned int)column)
        return false;
      return true;
    }
  }
  return false;
}


bool debugIsLexem(const string &str, const string &lexem, IdEntitySmart &name, int line = -1, int column = -1) {
  return debugIsLexem(str, lexem, name.entity(), line, column);
}


template <typename T>
static inline typename T::value_type::second_type findInContainerAndResolve(const T &container, Id *name) {
  typename  T::const_iterator it = container.find(name->toNormalizedString());
  if ( it != container.end() )
    if (typename T::value_type::second_type::dereferenced_type *p = it->second.object()) {
      name->definition(p);
      return p;
    }
  return 0;
}


template <typename T>
bool resolveEntity(const Id2 *name, T &container) { return findInContainerAndResolve(container, name->entityName()); }


namespace Sm { // resolve[Synonym,Object,Table,Index,Trigger]; Add::[synonym,package]; Alter::table; ResolvedEntity::getFieldRef
  bool resolveSynonym(ModelContext &model, UserContext *cntx, Id *name, bool maybePublic);
  bool resolveSynonym(ModelContext &model, UserContext *cntx, const Id2 *name);
}


template <typename T>
bool resolveEntityWithSynonym(ModelContext &model, UserContext *cntx, const Id2 * name, T & container) {
  if ( resolveEntity(name, container) || resolveSynonym(model, cntx, name) )
    return true;
  return false;
}


bool UserContext::resolveName(ModelContext &model, Sm::Id *name, bool maybePublic) {
  return
       findInContainerAndResolve(tables   , name) ||
       findInContainerAndResolve(views    , name) ||
       findInContainerAndResolve(sequences, name) ||
       findInContainerAndResolve(functions, name) ||
       findInContainerAndResolve(packages , name) ||
       findInContainerAndResolve(types    , name) ||
       resolveSynonym(model, this, name, maybePublic);
}

void UserContext::resolveName(ModelContext &model, const Sm::Id2 *n) {
  resolveName(model, n->entity(), n->size() == 1);
}


UserContext* ModelContext::parsingStageUserContext(const Sm::Id2 *name) {
  if (UserContext *cntx = getUContextWithoutAddEmpty(name))
    return cntx;
  return udata;
}


UserContext* ModelContext::getUContextWithoutAddEmpty(const Sm::Id2 *name) {
  if (!name)
    return 0;
  if (name->size() == 1) {
    UserContext* uCntx = udata;
    if (udata)
      ((Id2*)name)->userName() = new Id(*(uCntx->username));
    return uCntx;
  }
  else if (name->size() == 2) {
    Ptr<Id> userName = name->uname();
    return addUser(userName);
  }
  else
    throw 2;
  return 0;
}

UserContext *ModelContext::getUContextWithAdd(const Sm::Id2 *name) {
  UserContext *uCntx = getUContextWithoutAddEmpty(name);
  return uCntx ? uCntx : addUser(new Id(""));
}


namespace Sm { // resolve[Synonym,Object,Table,Index,Trigger]; Add::[synonym,package]; Alter::table; ResolvedEntity::getFieldRef

bool resolveSynonym(ModelContext &model, UserContext *cntx, Id *name, bool maybePublic) {
  if ( findInContainerAndResolve(cntx->synonyms, name)  ||
         (
           maybePublic && // публичные синонимы можно резолвить только если не задано имя схемы
           findInContainerAndResolve(model.publicSynonyms, name)
         )
     )
    return true;
  return false;
}

bool resolveSynonym(ModelContext &model, UserContext *cntx, const Id2 *name) {
  if (name)
    return resolveSynonym(model, cntx, name->entity(), name->size() == 1);
  return false;
}

void resolve(ModelContext &model, const Id2 *name) {
  if (UserContext *uCntx = model.getUContextOrNull(name))
    uCntx->resolveName(model, name);
}

void resolveObject(ModelContext &model, const Id2 * name) {
  if (UserContext *uCntx = model.getUContextOrNull(name))
    resolveEntityWithSynonym(model, uCntx, name, uCntx->types);
  return;
}

void resolveTable(ModelContext &model, const Id2 *name) {
  if(UserContext *uCntx = model.getUContextOrNull(name))
    if (resolveEntityWithSynonym(model, uCntx, name, uCntx->tables) ||
        resolveEntityWithSynonym(model, uCntx, name, uCntx->views))
      return;
  return;
}

void resolveIndex(ModelContext &model, const Id2 *name) {
  if(UserContext *uCntx = model.getUContextOrNull(name))
    resolveEntity(name, uCntx->indices);
  return;
}

void resolveTrigger(ModelContext &model, const Id2 *name) {
  if (UserContext *uCntx = model.getUContextOrNull(name))
    resolveEntityWithSynonym(model, uCntx, name, uCntx->triggers);
  return;
}



bool ResolvedEntity::getFieldRef(Ptr<Sm::Id> &field) {
  if (field)
    std::cout << "ERROR: Can't resolve reference field " << field->toNormalizedString()
              << ". Location: " << field->getLLoc()
              << ". Syntax cathegory = " << Sm::toString(ddlCathegory()) << std::endl;
  else
    std::cout << "ERROR: Can't resolve reference field \"\". Element datatype cathegory = " << Sm::toString(ddlCathegory()) << std::endl;
  return false;
};

}

bool isNullOrIsEntry( SCathegory::t entry, const AllowedMasks & allowed)
{
  if (allowed.cathegories.empty() && allowed.masks.empty())
    return true;
  for (vector<SCathegory::t>::const_iterator it = allowed.cathegories.begin(); it != allowed.cathegories.end(); ++it)
    if (entry == *it)
      return true;
  for (vector<SCathegory::t>::const_iterator mit = allowed.masks.begin(); mit != allowed.masks.end(); ++mit)
    if (isEntry(entry, *mit))
      return true;
  return false;
}


bool assignDeclToRef(ResolvedEntity *resolvedDef, Id *reference, SemanticTree *&foundedNode, SemanticTree *&sNode)
{
  if (resolvedDef)
    reference->definition(resolvedDef);
  else
    /*  Исходя из механизма инициализации объявлений и определений - для таких
     * имен обязательно должна быть задана непустая ссылка на структуру данных
     * со своим внутренним представлением.
     *
     * В противном случае - это баг в реализации функциональности инициализации
     * семантических ссылок для объявлений и определений.
     */
    throw 999;
  foundedNode = sNode;
  return true;
}

bool UserContext::getFieldRef(Ptr<Sm::Id> &field) {
  SemanticTree *n = getSemanticNode();
  if (!n)
    return resolveName(*syntaxerContext.model, field, false);
  SemanticTree *node = 0;
  if (n->childNamespace->findDeclaration(node, field)) {
    if (!field->semanticNode())
      field->semanticNode(node);
    return true;
  }
  return false;
}

bool Sm::Synonym::getFieldRef(Ptr<Id> &field) {
  if (target) {
    if (ResolvedEntity *def = target->entity()->definition())
      return def->getFieldRef(field);
    else if (Ptr<UserContext> ucntx = syntaxerContext.model->getUContext(target.object())) {
      if (target->uname())
        ucntx->resolveName(*syntaxerContext.model, target->entity(), false);
      if (ResolvedEntity *def = target->entity()->definition())
        return def->getFieldRef(field);
    }
  }
  return false;
}

bool previousConditionResult(bool condition, bool & previousResult) {
  bool ret = previousResult;
  previousResult = condition;
  return ret;
}




bool LevelResolvedNamespace::findDeclaration(SemanticTree *&foundedNode, Ptr<Id> &reference) const
{
  LevelResolvedNamespace::const_iterator node =
      static_cast<const ParentType*>(this)->find(reference->toNormalizedString());
  if (node == end())
    return false;

  if (VEntities *ventities = node->second) {
    return ventities->checkToCorrectResolving(foundedNode, reference);
  }
  return false;
}

bool LevelResolvedNamespace::findField(Ptr<Sm::Id> &field) const {
  if (field->callArglist)
    return false;
  return findCollectionField(field);
}

bool LevelResolvedNamespace::findCollectionField(Ptr<Sm::Id> &field) const {
  LevelResolvedNamespace::const_iterator node =
      static_cast<const ParentType*>(this)->find(field->toNormalizedString());
  if (node == end())
    return false;
  if (VEntities *vEnt = node->second.object())
    if (ResolvedEntity *def = vEnt->findFieldDefinition()) {
      field->definition(def);
      return true;
    }
  return false;
}


bool LevelResolvedNamespace::findVariable(Ptr<Sm::Id> &field) const {
  if (field->callArglist)
    return false;
  LevelResolvedNamespace::const_iterator node =
      static_cast<const ParentType*>(this)->find(field->toNormalizedString());
  if (node == end())
    return false;
  if (VEntities *vEnt = node->second.object())
    if (ResolvedEntity *def = vEnt->findVariableDefinition()) {
      field->definition(def);
      return true;
    }
  return false;
}

LevelResolvedNamespace::const_iterator LevelResolvedNamespace::findFieldNode(Ptr<Sm::Id> &reference) const {
  if (reference->callArglist)
    return this->end();
  LevelResolvedNamespace::const_iterator node =
      static_cast<const ParentType*>(this)->find(reference->toNormalizedString());
  return node;
}

ResolvedEntity* LevelResolvedNamespace::findFieldIdDef(Ptr<Sm::Id> &fieldReference) const {
  LevelResolvedNamespace::const_iterator node = findFieldNode(fieldReference);
  if (node == end())
    return 0;
  if (VEntities *vEnt = node->second.object())
    return vEnt->findFieldDefinition();
  return 0;
}

LevelResolvedNamespace::iterator LevelResolvedNamespace::node(const std::string &name) {
  iterator it = ((ParentType*)this)->find(name);
  if (it == this->end())
    return insert(value_type(name, mapped_type(0))).first;
  return it;
}


// TODO: для ссылки создавать контекст резолвинга, и делать подъем вверх

bool Sm::ResolvingContext::resolveAsSelectedField(ResolvedEntity *q) {
  if(q->getFieldRef(*currentPartIt))
    return true;
  else if ((child->cathegory != Sm::SCathegory::FactoringItem || !child->isList()) && q->getFieldRefByFromList(*currentPartIt, reference))
    return true;
  else if (q->isUnionChild()) {
    if (q->getFieldRefFromRootQuery(*currentPartIt))
      return true;
    if (q->getFieldRefByFromListFromRootQuery(*currentPartIt, reference))
      return true;
  }
  return (*currentPartIt)->unresolvedDefinition() ? true : findLevelup();
}

bool Sm::ResolvingContext::resolveInQueryBlock(ResolvedEntity *q) {
  hasUnresolvedQuotedLiteral = true;
  if (childIsNotFrom) {
    // если поднимаемся из where - поля заданные в from должны быть приоритетнее полей,
    // заданных в SelectFieldList
    childIsNotFrom = false;
    if(q->getFieldRefByFromList(*currentPartIt, reference))
      return true;
    else if (q->isUnionChild() && q->getFieldRefByFromListFromRootQuery(*currentPartIt, reference))
      return true;
  }
  else if (!childIsFrom && !childIsFactoring) {
    childIsNotFrom = false;
    if(q->getFieldRefByFromList(*currentPartIt, reference))
      return true;
    else if (q->isUnionChild() && q->getFieldRefByFromListFromRootQuery(*currentPartIt, reference))
      return true;
    else if (!childIsSelectedFieldPart)
      return resolveAsSelectedField(q);
  }
  else if (childIsFrom) {
    if (q->findInFactoringList(*currentPartIt))
      return true;
    else if (reference->cathegory != SCathegory::FromTableReference && q->getFieldRefByFromList(*currentPartIt, reference))
      if (ResolvedEntity *def = (*currentPartIt)->unresolvedDefinition())
        switch (def->ddlCathegory()) {
          case ResolvedEntity::SqlSelectedField_:
          case ResolvedEntity::QueriedPseudoField_:
            (*currentPartIt)->clearDefinition();
            break;
          default:
            return true;
        }
  }
  else
    return resolveAsSelectedField(q);
  return (*currentPartIt)->unresolvedDefinition() ? true : findLevelup();
}

/**
 * @brief commonFindDeclarationUp
 *        Обобщенный поиск вверх. Вызывается когда необходимо найти мажорную компоненту ссылки.
 * @param foundedIt             Последнее найденное определение для текущей компоненты ссылки.
 * @param reference             Ссылка целиком.
 * @param currentParent         Текущий предок для ссылки.
 * @param begin                 Начало семантических узлов текущего уровня для просмотра
 * @param end                   Узел, следующий за концом семантических деревьев текущего уровня
 * @param allowed.cathegories    Разрешенное множество категорий (если пусто - то все)
 * @param allowed.masks Разрешенное множество масочныйх категорий (если пусто - то все)
 * @return Истина, если резолвинг выполнен успешно.
 */
bool Sm::ResolvingContext::commonFindDeclarationUp()
{
  if (!currentLevel)
    throw 999;
  if ((*currentPartIt)->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";
  if (currentLevel)
    // обработка особых случаев.
    // TODO: избавиться от begin.
    switch (currentLevel->cathegory) {
      case SCathegory::RootSemanticNode: {
        if (currentLevel->unnamedDdlEntity->getFieldRef(*currentPartIt)) {
          if (ResolvedEntity *def = (*currentPartIt)->unresolvedDefinition())
            foundedNode = def->getSemanticNode();
          return true;
        }
        return false;
      }
      case SCathegory::ObjectType: {
        static const HString selfReference = "SELF";
        if (!(*currentPartIt)->quoted() && **currentPartIt == selfReference) {
          (*currentPartIt)->definition(currentLevel->unnamedDdlEntity);
          return true;
        }
        break;
      }
      case SCathegory::InsertingValues: {
        childIsInsertingValues = true;
        return findLevelup();
      }
      case SCathegory::QueryBlock: {
        if (startLevel->getParent()->cathegory == SCathegory::Into)
          return findLevelup();
        childIsQueryBlock = true;
        if (ResolvedEntity * def = currentLevel->ddlEntity()) {
          hasUnresolvedQuotedLiteral = true;
          if (resolveInQueryBlock(def)) {
            if (ResolvedEntity *d = (*currentPartIt)->definition())
              if (d->ddlCathegory() == ResolvedEntity::CollectionMethod_) {
                (*currentPartIt)->clearDefinition();
                return findLevelup();
              }
            return true;
          }
        }
        break;
      }
      case SCathegory::SqlStatement: {
        SemanticTree *n = 0;
        if (childIsInsertingValues)
          return findLevelup();

        if (currentLevel->isBasForAlias() &&
            currentLevel->levelNamespace &&
            currentLevel->levelNamespace->findDeclaration(n, *currentPartIt))
          return true;
        else if (currentLevel->childNamespace && currentLevel->childNamespace->findDeclaration(n, *currentPartIt))
          return true;

        ResolvedEntity *def = currentLevel->unnamedDdlEntity;
        if (!childIsFrom && def->getFieldRef(*currentPartIt))
          return true;
        break;
      }
      case SCathegory::UnionQuery: {
        if (child->cathegory == SCathegory::OrderBy && currentLevel->unnamedDdlEntity->getFieldRef(*currentPartIt))
          return true;
        break;
      }
      case SCathegory::QueryBlockField:
      case SCathegory::TableQueryReference:
        return findLevelup(); // не делать ссылку поля на само себя
      case SCathegory::WhereClause:
        childIsNotFrom = true;
        return findLevelup();
      case SCathegory::SelectedField:
        childIsSelectedFieldPart = true;
        // pass
      case SCathegory::GroupBy:
        childIsNotFrom = true;
        if (currentLevel->isList())
          return findLevelup(); // не делать ссылку поля на само себя
        break;
      case SCathegory::Index:
        return currentLevel->unnamedDdlEntity &&
               currentLevel->unnamedDdlEntity->getFieldRef(*currentPartIt) ? true : findLevelup();
      case SCathegory::FromTableReference: {
        if (reference->cathegory == SCathegory::Dblink) {
          currentLevel->resolveCurrent();
          if (ResolvedEntity *def = currentLevel->ddlEntity())
            if (SemanticTree *defnode = def->getSemanticNode())
              if (LevelResolvedNamespace *refuserSpace = defnode->levelNamespace)
                if ((foundedLevel = refuserSpace->semanticLevel)) {
                  push(refuserSpace->semanticLevel, foundedLevel);
                  return commonFindDeclarationUp(); // итератор-указатель на отца в списке детей деда.
                }
        }
        break;
      }
      case SCathegory::From:
        childIsFrom = true;
        return findLevelup();
      case SCathegory::FactoringItem:
        childIsFactoring = true;
        break;
      default:
        break;
    }
  return findInLevelNamespaceOrLevelUp();
}

bool ResolvingContext::findInLevelNamespaceOrLevelUp() {
  if (currentLevel)
    switch (child->cathegory) {
      case SCathegory::TableQueryReference:
      case SCathegory::FromSingle:
        return findLevelup(); // не делать ссылку поля на само себя
      default:
        break;
    }

  // поиск на текущем уровне
  if (LevelResolvedNamespace *levelNamespace = currentLevel->childNamespace.object()) {

    if  (levelNamespace->findDeclaration(foundedNode, *currentPartIt)) {
      if (ResolvedEntity *def = (*currentPartIt)->unresolvedDefinition())
        switch (def->ddlCathegory()) {
          case ResolvedEntity::Variable_:
          case ResolvedEntity::FunctionArgument_:
            switch (reference->cathegory) {
              case SCathegory::FromTableReference:        // ищется ссылка на таблицу из секции FROM
              case SCathegory::TableViewMaterializedView: // ищется ссылка на таблицу, обрабатываемую оператором Update или Delete
//              case SCathegory::Function:                  // семантика функции - указаны скобки
                return clearAndFindLevelup();
              default:
                break;
            }
            if (ResolvedEntity *refNode = reference->unnamedDdlEntity) {
              if (refNode->toSelfChangedQueryEntityRef()) { // ищется ссылка на таблицу из секции from, update, delete
                return clearAndFindLevelup();
              }
            }
            if ((*currentPartIt)->callArglist)
              if (Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(def->getDatatype()))
                if (!datatype->isObjectType())
                  return clearAndFindLevelup();           // ищется как системная функция, а не аргумент
            break;
          default:
            break;
        }
      return true;
    }

    if (currentLevel->cathegory == SCathegory::Function  &&
        currentLevel->nametype == SemanticTree::DEFINITION &&
        !(*currentPartIt)->callArglist &&
        currentPartIt->object() != reference->entityName().object() &&
        (*currentPartIt)->toNormalizedString() == currentLevel->reference()->entity()->toNormalizedString()) {
      foundedNode = currentLevel;
      (*currentPartIt)->definition(currentLevel->ddlEntity());
      return true;
    }
    return findLevelup();
  }
  else if (currentLevel->cathegory == SCathegory::RootSemanticNode)
    return false;
  else
    throw 999;

}



bool Sm::ResolvingContext::findLevelup() {
  if (LevelResolvedNamespace *levelNamespace = currentLevel->childNamespace.object())
    if (LevelResolvedNamespace *parentLevelNamespace = levelNamespace->parent)
      if ((foundedLevel = parentLevelNamespace->semanticLevel)) {
        push(levelNamespace->semanticLevel, foundedLevel);
        return commonFindDeclarationUp();
      }

  return false; // ничего не найдено. имя не существует.
}

bool ResolvingContext::clearAndFindLevelup() {
  if ((*currentPartIt)->definition()) {
    if (!oldcnt) {
      ++oldcnt;
      resetPartitialResolvedDefs();
    }
  }
  (*currentPartIt)->clearDefinition();
  foundedNode = 0;
  return findLevelup();
}

bool ResolvingContext::commonFindDeclDownInNode(Sm::SemanticTree *node)
{
  // искать рекурсивно вглубь пока не встретится новый уровень
  switch (node->nametype) {
    case SemanticTree::EMPTY:
      if (node->commonFindDeclarationOnLevelDown(this))
        return true;
      break;
    case SemanticTree::DECLARATION:
    case SemanticTree::DEFINITION:
      if (Ptr<IdEntitySmart> ent = node->reference())
        if (Id *decl = ent->entity()) {
          Id *reference = currentPartIt->object();
          if (*decl == *reference) {
            assignDeclToRef(decl->definition(), reference, foundedNode, node);
            return true;
          }
        }
      break;
    default:
      break;
  }
  return false;
}


/*
 * Искать в пространстве имен, которое ровно на один уровень ниже.
 * в противном случае - вернуть истину
 *
 * Что такое уровень в семантическом дереве:
 * это поддерево со следующими свойствами:
 *  - его листьями являются объявления и определения
 *  - его корнем является одно объявление или определение.
 */
// TODO: убрать foundedIt, реализовать учет псевдонимов.
bool Sm::SemanticTree::commonFindDeclarationOnLevelDown(ResolvingContext *cntx)
{
  for (SemanticTree *c : childs) {
    if (cntx->commonFindDeclDownInNode(c))
      return true;
//    else if (Sm::SemanticTree *alias_ = c->alias())
//      if (cntx->commonFindDeclDownInNode(alias_))
//          return true;
  }
  return false;
}


void addToLevelNamespace(LevelResolvedNamespace *levelResolvedNamespace, std::string str, ResolvedEntity *definition)
{
  LevelResolvedNamespace::iterator it = levelResolvedNamespace->node(str);
  if (definition) {
    definition->levelNamespace(levelResolvedNamespace); // установить указатель на levelNamespace
    if (!it->second)
      it->second= new VEntities();
    it->second->addWithoutOverloadResolving(definition);
    definition->setVEntities(it->second);
  }
}

void Sm::updateResolvedNamespace(LevelResolvedNamespace *levelResolvedNamespace, SemanticTree *node)
{
  ResolvedEntity *definition = node->entityDef();
  Ptr<Id> entity;
  if ((entity = node->entityName())) {
    if (ResolvedEntity *d = entity->unresolvedDefinition()) {
      sAssert(definition != d);
      if (d->skipLevelNamespaceUpdating())
        return;
    }
    addToLevelNamespace(levelResolvedNamespace, entity->toNormalizedString(), definition);
  }
  else if (definition)
    addToLevelNamespace(levelResolvedNamespace, "", definition);
}


void Sm::collectEqualsDeclarationOnNode(SemanticTree *node, LevelResolvedNamespace *levelResolvedNamespace) {
  node->levelNamespace    = levelResolvedNamespace;

  switch (node->nametype) {
    case SemanticTree::EMPTY:
      collectEqualsDeclaration(node->childs.begin(), node->childs.end(), levelResolvedNamespace);
      break;
    case SemanticTree::DECLARATION:
    case SemanticTree::DEFINITION:
      // обработка текущего узла-определения
      switch (node->cathegory) {
        case SCathegory::ModelContext:
          if (!node->isList())
            updateResolvedNamespace(levelResolvedNamespace, node);
          collectEqualsDeclaration(node->childs.begin(), node->childs.end(), levelResolvedNamespace);
          break;
        case SCathegory::Index:
          collectEqualsDeclaration(node->childs.begin(), node->childs.end(), new LevelResolvedNamespace(levelResolvedNamespace, node));
          break;
        default:
          if (!node->isList())
            updateResolvedNamespace(levelResolvedNamespace, node);
          collectEqualsDeclaration(node->childs.begin(), node->childs.end(), new LevelResolvedNamespace(levelResolvedNamespace, node));
          break;
      }
      break;
    default:
      // обход следующего уровня
      collectEqualsDeclaration(node->childs.begin(), node->childs.end(), new LevelResolvedNamespace(levelResolvedNamespace, node));
      break;
  }
}

void Sm::collectEqualsDeclaration(SemanticTree::Childs::iterator begin,
                                  SemanticTree::Childs::iterator end,
                                  LevelResolvedNamespace        *levelNamespace) {
  for (SemanticTree::Childs::iterator it = begin; it != end; ++it) {
    ++levelNamespace->levelFullSize;
    collectEqualsDeclarationOnNode(*it, levelNamespace);
  }
}

void resolveAndFindRetry(Ptr<Id> &field, ResolvedEntity *ddlFoundedNode) {
  ResolvedEntity *p = ddlFoundedNode->getConcreteDefinition();
  if (p->semanticResolve())  // в данной позиции может оказаться недорезолвленный QueryBlock
    p->getFieldRef(field);

  if (!field->unresolvedDefinition()) {
    p->getFieldRef(field);
    p = ddlFoundedNode->getConcreteDefinition();
    if (p->semanticResolve())
      p->getFieldRef(field);
    // throw 999; // Иначе - ситуация непонятна - конкретное определение для ссылки - найдено, но поле найти не получается.
  }
}


ResolvingContext::ResolvingContext() {}

void ResolvingContext::push(SemanticTree *child_, SemanticTree *currLevel) {
  child = child_;
  currentLevel = currLevel;
}

void ResolvingContext::setReference(IdEntitySmart *reference_, SemanticTree *level) {
  foundedNode  = 0;
  foundedLevel = level;
  reference    = level;
  referenceId  = reference_;
  startLevel   = level;
  currentLevel = 0;
  child        = 0;
  hasUnresolvedQuotedLiteral = false;
  IdEntitySmart::iterator majorEntityIt = reference_->majorEntityIter();
  currentPartIt = majorEntityIt;
  childIsQueryBlock = false;
  childIsNotFrom    = false;
  childIsFrom       = false;
}

bool ResolvingContext::resolveMemberFunctionCall() {
  if (ResolvedEntity *d = (*currentPartIt)->definition()) {
    while (d->toSelfSynonym())
      if ((d = d->getNextDefinition()))
        (*currentPartIt)->definition(d);
      else
        return false;

    switch (d->ddlCathegory()) {
      case ResolvedEntity::Object_:
      case ResolvedEntity::MemberFunction_:
      case ResolvedEntity::MemberVariable_:
        if (Sm::Type::Object *obj = d->getOwner())
          return FunctionResolvingContext::resolveMfCall((*currentPartIt), obj);
        break;
      default:
        break;
    }
  }
  return false;
}

namespace Sm {
Ptr<Sm::FunctionArgument> findByNormalizedName(ResolvedEntity *fun, CallArgList::iterator it, unsigned int &pos) {
  Ptr<Id> argname = (*it)->argname();
  const HString &normalizedArgname = argname->toNormalizedString();

//  if (argname->beginedFrom(132556,21))
//    cout << "";

  Ptr<Sm::Arglist> defArglist = fun->getArglist();
  for (FunArgList::iterator defIt = defArglist->begin(); defIt != defArglist->end(); ++defIt)
    if ((*defIt)->getName()->normalizedString() == normalizedArgname) {
      pos = std::distance(defArglist->begin(), defIt);
      return *defIt;
    }

  if (VEntities *ve = fun->vEntities()) {
    VEntities::OverloadedFunctions::iterator it = ve->overloadedFunctions.find(fun);
    if (it != ve->overloadedFunctions.end()) {
      Ptr<Sm::EquallyEntities> eqEnts = it->second;
      if (eqEnts->declarations.size())
        if (Function *decl = (*(eqEnts->declarations.begin()))->toSelfFunction()) {
          Ptr<Sm::Arglist> declArglist = decl->getArglist();
          if (declArglist) {
            FunArgList::iterator declIt = declArglist->begin();
            for (; declIt != declArglist->end(); ++declIt)
              if ((*declIt)->getName()->normalizedString() == normalizedArgname)
                break;
            if (declIt != declArglist->end()) {
              pos = std::distance(declArglist->begin(), declIt);
              return *(defArglist->begin() + pos);
            }
          }
        }
    }
  }

  return 0;
}

}

void FunctionResolvingContext::debugAddToPartitialResolvedVector(int p, ResolvedEntity *f) {
  (void)p;
  (void)f;
//  partitialResolvedFunctions.push_back(cntx);
}

void FunctionResolvingContext::updateMaximalResolvedFundecl(Context &ctx) {
  int ctxPower = ctx.summaryPower;
  if (currentPower < ctxPower || (ctx.hasEverything && currentPower == ctxPower)) {
    maximalResolvedFundecl = ctx.fun ? ctx.fun->getDefinitionFirst() : nullptr;
    currentPower = ctxPower;
    ambiguousDeclarations = false;
  }
  else if (currentPower == ctxPower)
    ambiguousDeclarations = true;
}

bool FunctionResolvingContext::tryResolveFunref(ResolvedEntity *fundecl, Id *call) {
  Context context(fundecl, call);
  if (context.tryResolveFunref()) {
    updateMaximalResolvedFundecl(context);
    return true;
  }
  return false;
}

void FunctionResolvingContext::Context::updateCompatibility(IsSubtypeValues conversionArgTypePower)
{
  settedCallArguments[settedArgPosition] = true;
  summaryPower += (int)(conversionArgTypePower.val);
}

void FunctionResolvingContext::Context::calculateArumentSupertype() {
  declIsDate     = argDeclT->isSubtype(Datatype::mkDate    (), isPlContext);
  declIsNumber   = argDeclT->isSubtype(Datatype::mkNumber  (), isPlContext);
  declIsVarchar2 = argDeclT->isSubtype(Datatype::mkVarchar2(), isPlContext);
  if (declIsDate == 2)
    declIsNumber = declIsVarchar2 = 0;
  else if (declIsNumber == 2)
    declIsDate = declIsVarchar2 = 0;
  else if (declIsVarchar2 == 2)
    declIsDate = declIsNumber = 0;
}

bool FunctionResolvingContext::Context::compareDatatypesInCall(FunCallArg   *callArg)
{
  if (call->beginedFrom(syntaxerContext.debugTokenLocation) )
    cout << "";

  if (Ptr<Datatype> t = isPlContext ? callArg->getDatatype().object() : SubqueryUnwrapper::unwrap(callArg->getDatatype())) {
    if (t->isEverything())
      hasEverything = true;

    IsSubtypeValues pwr = t->isSubtype(argDeclT, isPlContext, inSqlCode);
    if (pwr != IsSubtypeValuesEnum::EXPLICIT) {
      updateCompatibility(pwr);
      return true;
    }
  }
  return false;
}




static bool needDatatypeInCall(Sm::SemanticTree *node) {
  static const uint32_t funref = Sm::SemanticTree::getPackedCathegory(SCathegory::Function , SemanticTree::REFERENCE);
  static const uint32_t exprId = Sm::SemanticTree::getPackedCathegory(SCathegory::RefAbstract, SemanticTree::REFERENCE);

  if (!node)
    return true; // что-то недорезолвлено, по дефолту считаем что возвращаемый тип нужен

  uint32_t cat;
  do {
    cat = Sm::SemanticTree::getPackedCathegory(node->cathegory, node->nametype);
  } while ((cat == funref || cat == exprId) && (node = node->getParent()));

  if (node && node->cathegory == SCathegory::StatementFunctionCall)
    return false;
  return true;
}


bool Sm::FunctionResolvingContext::Context::isSelfMfArgument(const Id &arg) {
  static const HString selfKw = "SELF";
  return arg == selfKw;
}

/**
 * @brief tryResolveFunref
 * Сравнение текущего объявления функции с вызовом функции
 * и проверка на совместимость.
 * @param fun            объявление функции
 * @param funcallArgList список аргументов вызываемой функции
 * @param positives      Ссылка на счетчик прямых соответствий тип-подтип в аргументах
 * @param negatives      Ссылка на счетчик обратных соответствий тип-подтип в аргументах
 * @return Истина, если нет несовместимых аргументов. Ложь, если есть несовместимые аргументы.
 */
bool Sm::FunctionResolvingContext::Context::tryResolveFunref()
{
  if (call->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";

  if (!call->callArglist || call->callArglist->empty()) {
    if (fun->allArgsIsDefault()) {
      summaryPower = 10;
      return true; // В найденном определении для всех функций заданы параметры по умолчанию
    }
    else if (Sm::Type::MemberFunction *mf = fun->toSelfMemberFunction()) {
      if (!fun->getArglist() || fun->getArglist()->empty())
        return false;
      if (mf->isMemberFunctionMember() && isSelfMfArgument(*(fun->getArglist()->begin()->object()->getName()))) {
        summaryPower = 10;
        return true;
      }
      else
        return false;
    }
    // Иначе - можно попытаться найти то же      для всех остальных функций с этим именем в данном пространстве имен
    // это нужно сделать вызовами данной функции для --||--
    else
      return false;
  }
  else if (call->callArglist->size() > fun->arglistSize())
    return false; // Если длина списка аргументов в вызове больше, чем длина списка аргументов в объявлении то это не то объявление


  if (SemanticTree *n = call->semanticNode()) {
    isPlContext = n->isPlContext();
    inSqlCode   = n->isSqlCode();

    Ptr<Datatype> rettype = fun->getDatatype();
    if (needDatatypeInCall(n)) {
      if (!rettype)
        return false; // не пытаться подставить хранимые процедуры в выражениях
    }
    else if (!rettype)
      summaryPower = 1; // небольшое преимущество для хранимых процедур в операторах вызова, где не нужен возвращаемый тип.
  }

  Ptr<Sm::Arglist> declArglist = fun->getArglist();
  FunArgList::iterator funArglistIt = declArglist->begin();

  settedCallArguments.resize(fun->arglistSize(), 0);


  if (Sm::Type::MemberFunction *mf = fun->toSelfMemberFunction()) {
    if ((mf->isMemberFunctionStatic() || mf->isMemberFunctionMember()) &&
        isSelfMfArgument(*(fun->getArglist()->begin()->object()->getName()))) {
      // пропуск self
      bool skipSelf = true;
      if (call->callArglist->size() == declArglist->size()) {
        Ptr<Datatype> firstArgT = (*(call->callArglist->begin()))->getDatatype();
        if (firstArgT->isSubtype((*funArglistIt)->getDatatype(), isPlContext) == Sm::IsSubtypeValues::EXACTLY_EQUALLY)
          skipSelf = false;
      }


      if (skipSelf) {
        updateCompatibility(IsSubtypeValuesEnum::EXACTLY_EQUALLY);
        ++settedArgPosition, ++funArglistIt;
      }
    }
  }


  for (CallArgList::iterator it = call->callArglist->begin(); it != call->callArglist->end(); ++it, ++settedArgPosition, ++funArglistIt) {
    if ((*it)->argname()) { // именованный аргумент
      unsigned int pos = std::numeric_limits<unsigned int>::max();
      Ptr<Sm::FunctionArgument> defFunArg = findByNormalizedName(fun, it, pos);
      if (!defFunArg)
        return false; // Именованный аргумент не найден. Резолвинг не удался

      argDeclT = defFunArg->tryResolveDatatype().object(); // Механизм доразрешения неразрешенных типов для argDeclT

      calculateArumentSupertype();
      unsigned int oldPos = settedArgPosition;
      settedArgPosition = pos;
      bool res = compareDatatypesInCall(it->object());
      settedArgPosition = oldPos;
      if (!res)
        return false;
    }
    else {
      if ((*it)->isAsterisk()) { // аргумет-звёздочка
        updateCompatibility(IsSubtypeValuesEnum::IMPLICIT_CAST_BY_FIELDS);
        hasEverything = true;
      }
      else { // позиционный аргумент
        argDeclT = (*funArglistIt)->tryResolveDatatype().object(); // Механизм доразрешения неразрешенных типов для argDeclT
        if (!compareDatatypesInCall(it->object()))
          return false;
      }
    }
  }

  // Проверка: все соответствующие нулевые позиции в объявлении должны иметь значение по умолчанию
  funArglistIt = declArglist->begin();
  if (settedArgPosition != settedCallArguments.size())
    for (vector<bool>::iterator compatibilityIt = settedCallArguments.begin(); compatibilityIt != settedCallArguments.end(); ++compatibilityIt, ++funArglistIt)
      if (!*compatibilityIt && !(*funArglistIt)->defaultValue())
        return false; // Аргумент отсутсвующий в списке не имеет значения по умолчанию. Это не то объявление.

  return true;
}


bool FunctionResolvingContext::resolveMfCall(Id *call, Sm::Type::Object *obj) {
  FunctionResolvingContext self;
  obj->resolveMember(call, self);
  return self.setMaxMatchedResolvedFunction(call);
}

void FunctionResolvingContext::resolveNewCall(Id *call, Type::Object *obj) {
  FunctionResolvingContext self;

  obj->resolveMember(call, self);
  self.setMaxMatchedResolvedFunction(call);
}


/// Поиск объявления функции, заданной в прагме, идущего на том же уровне до этой прагмы
bool Sm::FunctionResolvingContext::setPragmaFunctionReferenceDefinition(Id* call)
{
  if (SemanticTree *currentSemNode = call->semanticNode()) {
    SemanticTree *parent = currentSemNode->getParent();
    SemanticTree::Childs::iterator nodeIt = currentSemNode->getPositionInParent();
    do {
      --nodeIt;
      if ((*nodeIt)->cathegory == SCathegory::Function)
        switch ((*nodeIt)->nametype) {
          case SemanticTree::DECLARATION:
          case SemanticTree::DEFINITION:
            if (*((*nodeIt)->reference()->entity()) == *call) {
              call->definition((*nodeIt)->unnamedDdlEntity);
              call->setResolvedFunction();
              return true;
            }
            continue;
          default:
            continue;
        }
    } while (nodeIt != parent->childs.begin());
  }
  return false;
}



bool FunctionResolvingContext::resolveSameLvlFunDecl(Id *call) {
  FunctionResolvingContext self;
  if (call->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";

  ResolvedEntity* def = call->definition();
  if (!def)
    return false;

  if (!def->vEntities())
    def = def->tryResoveConcreteDefinition();
  if (!def->getNextDefinition())
    return false;
  if (def->ddlCathegory() == ResolvedEntity::CollectionMethod_)
    return true;
  if (!def->vEntities()) {
    def = def->tryResoveConcreteDefinition();
    throw 999;
  }

  // Поиск по всем одинаковым именам в уровне пространства имен модели

  // TODO:
  // Для оптимизации - в пространстве имен можно создать отдельно
  // -- список переменных Variable и MemberVariable
  // -- список функций Function и MemberFunction.
  //    Их нужно так добавить, чтобы один узел отображал все перегрузки
  // -- список всего остального

  if (VEntities *vEnt = def->vEntities()) {
    if (vEnt->variablesNotEmpty()) { // это обращение к элементу коллекции
      call->definition(vEnt->variablesFront());
      return true;
    }
    else {
      {
        SemanticTree *n;
        if ((n = call->semanticNode()) && n->cathegory == Sm::SCathegory::FunctionPragmaRestriction) {
          sAssert(!setPragmaFunctionReferenceDefinition(call));
          return true;
        }
      }

      self.resolveFuncallByMaximalPower(*vEnt, call);
      return self.setMaxMatchedResolvedFunction(call);
    }
  }
  return false;
}

bool VEntities::checkToCorrectResolving(SemanticTree *&refSNode, Ptr<Sm::Id> &ref) const {
  // TODO: еще можно отслеживать по семантическому дереву - был ли заход в процедуру - то поиск можно еще сильнее оптимизировать.

  if (!functions.empty()) {
    // FunctionResolvingContext::collectPartitialResolvedFunctionsVector(VEntities &container, Id *call) {
    ref->definition(functions.front());

    if (ref->skipFunctionResolving()) {
      refSNode = functions.front()->getSemanticNode();
      return true;
    }

    if (ref->beginedFrom(syntaxerContext.debugTokenLocation))
      cout << "";

    FunctionResolvingContext::resolveSameLvlFunDecl(ref.object());

    if (Sm::ResolvedEntity *resolvedFun = ref->definition())
      refSNode = resolvedFun->getSemanticNode();
    return true;
  }
  if (variables) {
    ref->definition(variables->key);
    refSNode = variables->key->getSemanticNode();
    return true;
  }
  if (!others.empty()) {
    if (SemanticTree *sNode = ((*others.begin()))->getSemanticNode()) {
      // если не алиас - просто присвоить.
      if (ResolvedEntity *def = *others.begin()) {
        if (Synonym* syn = def->toSelfSynonym()) {
          if (ResolvedEntity *synonymTarget = syn->target->definition()) {
            ref->definition(synonymTarget);
            refSNode = nAssert(synonymTarget->getSemanticNode());
            return true;
          }
          else
            cout << "error: reference " << ref->getLLoc().locText() << " s unresolved synonym " << syn->getLLoc().locText() << endl;
        }

        ref->definition(*others.begin());
        refSNode = sNode;
      }
      else
        throw 999;
      return true;
    }
    else {
      cout << "Error: Semantic nodes must be generated for all of the definitions and declarations (" << __FILE__ << ":" << __LINE__ << ")"  << endl;
      cout << "  Semantic nodes not generated for " << (*others.begin())->ddlCathegoryToString();
      if (Ptr<Id> n = (*others.begin())->getName())
        cout << " (" << n->toQString() << ", " << n->getLLoc() << ")" << endl;
      cout << "  Reference is " << ref->toQString() << " " << ref->getLLoc() << endl;
      return false;
    }
  }
  return false;

}



bool ResolvingContext::resolveTypecallEntity() {
  if (!isExpression || (*currentPartIt)->callArglist ||
      (*currentPartIt)->toNormalizedString() == "SELF")
    return false;

  if (ResolvedEntity *def = (*currentPartIt)->definition())  {
    if(def->isCollectionType()) {
      (*currentPartIt)->definition(def->getDefaultConstructor());
      (*currentPartIt)->setResolvedFunction();
      return true;
    }
    else if (def->ddlCathegory() == ResolvedEntity::Object_) {
      (*currentPartIt)->definition(def->getDefaultConstructor());
      (*currentPartIt)->setResolvedFunction();
      return true;
    }
  }
  return false;
}

bool ResolvingContext::resolveFuncallEntity()
{
  // Если это обращение к элементу коллекции (с функциональным синтаксисом)
  // дополнить или поменять foundenNode->cathegory на ResolvedEntity cathegory
  if (ResolvedEntity *def = (*currentPartIt)->definition())  {
    if (def->isField()) {
      Ptr<Datatype> dataType = def->getDatatype();
      dataType->semanticResolve();
      if (ResolvedEntity *t = dataType->getResolvedNextDefinition()) {
        for (ResolvedEntity::ScopedEntities tCat = t->ddlCathegory(); true; tCat = t->ddlCathegory())
        {
          if (t->isCollectionType()) {
            (*currentPartIt)->definition(t->addAccesingMethod(def));
            return true;
          }
          else if (t->isField()) {
            if (ResolvedEntity *newT = t->getNextDefinition())
              t = newT;
            else
              return false;
          }
          else if (tCat == ResolvedEntity::Subtype_ || tCat == ResolvedEntity::Datatype_)
            t = t->getResolvedNextDefinition();
          else
            break;
        }
        return false; // В объявлениях выше находится переменная с тем же именем, но данная ссылка объявлена еще выше
                      // например, такое может быть если делать select sum(...) внутри процедуры, у которой есть аргумент sum
      }
      else
        return true;
    }
    else if (def->isCollectionType()) {
      (*currentPartIt)->definition(def->getDefaultConstructor());
      (*currentPartIt)->setResolvedFunction();
      return true;
    }
    else if (foundedNode)
      switch (foundedNode->cathegory) {
        case SCathegory::Package:
          return false;
        case SCathegory::BlockPlSql: {
          SemanticTree *parent = def->getSemanticNode()->getParent();
          (*currentPartIt)->definition(parent->unnamedDdlEntity);
          if (parent->cathegory != SCathegory::Function)
            throw 999;
          break;
        }
        case SCathegory::ObjectType:
        case SCathegory::Field:
        case SCathegory::Function:
          if (resolveMemberFunctionCall()) // особая обработка для функций членов - для учета наследования
            return true;
          return currentPartIt->object()->definition();

//              FunctionResolvingContext::resolveSameLvlFunDecl(currentPartIt->object()) ? true : false;
        case SCathegory::Datatype:
          if (ResolvedEntity *def2 = def->getNextNondatatypeDefinition()) {
            switch (def2->ddlCathegory()) {
              case ResolvedEntity::NestedTable_:
              case ResolvedEntity::Varray_:
                if ((*currentPartIt)->callArglist)
                  (*currentPartIt)->definition(def2->addAccesingMethod(def));
                else
                  (*currentPartIt)->definition(def);
                return true;
              default:;
            }
          }
          //pass-through
        default:
          // особая обработка для функций членов - для учета наследования
          if (resolveMemberFunctionCall())
            return true;
          cout << "resolveFuncallEntity for " <<  Sm::debugSCathegoryConvert(foundedNode->cathegory)
               << " and id " << (*currentPartIt)->toNormalizedString() << ": " << (*currentPartIt)->getLLoc() << " is unimplemented" << endl;
          break;
      }
  }
  return currentPartIt->object()->definition();
//  return FunctionResolvingContext::resolveSameLvlFunDecl(currentPartIt->object()) ? true : false;
}



bool ResolvingContext::isSystemTemplateLastRef()
{
  return (*currentPartIt)->definition()->isSystemTemplate() &&
         currentPartIt == referenceId->begin();
}

bool ResolvingContext::resolveAsArgumentOfCurrentFunctionNamespace()
{
  if (!(*currentPartIt)->callArglist && referenceId->size() > 1 &&
      (*currentPartIt)->definition()->toSelfFunction() &&
      (*currentPartIt)->definition()->getFieldRefInArglist(*prev(currentPartIt))) { // это аргумент функции, к которому производится доступ через ее имя
    --currentPartIt;
    return true;
  }
  return false;
}


Ptr<Type::collection_methods::AccessToItem> ResolvingContext::createCollectionAccessOperator(ResolvedEntity *currentPartDef, SemanticTree *refSNode)
{
  Ptr<Sm::Type::collection_methods::AccessToItem> accessOp = new Sm::Type::collection_methods::AccessToItem(currentPartDef);
  syntaxerContext.model->resolvedAccessOperators.push_back(accessOp);
  accessOp->setSemanticNode(refSNode);
  return accessOp;
}


ResolvingContext::ResolvingSpecialCasesState ResolvingContext::resolveCollectionAccess() {
  // currentPartIt в данной позиции - старшее имя в referenceId
  ResolvedEntity *previousPartDef = (*currentPartIt)->definition();

  if (!(*currentPartIt)->callArglist || !previousPartDef->isField())
    return RESOLVING_NEXT;

  checkIdEntitySmart(referenceId);

  SemanticTree *refSNode = 0;
  if (Ptr<Id> n = referenceId->majorEntity())
    if (!(refSNode = n->semanticNode()))
      throw 999;

  IdEntitySmart *ref = refSNode->extractMajorDefinitionReference().object();
  if (!ref) {
    cout << "error: resolveCollectionAccess - ref is NULL" << endl;
    return RESOLVING_FINISH_SUCCESS; // в случае ошибки - сделать ничего нельзя
  }

  Id *id = (*currentPartIt);

  if (ref != referenceId || !id)
    throw 999;

  for (Sm::IdEntitySmart::iterator refIt = ref->end() - 1; refIt->object() != id; --refIt)
    if (refIt == ref->begin())
      throw 999; // узел сurrentPartIt не найден в ссылке, в резолвере поломано обновление currentPartIt

  if (referenceId->getLLoc().beginedFrom(1389461,101))
    cout << "";

  static const std::set<string> collectionMethods = {"COUNT", "FIRST", "LAST", "LIMIT", "TRIM", "DELETE", "EXTEND", "EXISTS", "NEXT", "PRIOR" };

  auto checkToCollectionMethod = [](Ptr<Id> f, ResolvedEntity *prevDef) -> bool {
    return collectionMethods.count(f->toNormalizedString()) && prevDef && prevDef->getFieldRef(f);
  };


  using namespace Sm::Type::collection_methods;
  Ptr<AccessToItem> accessOp = createCollectionAccessOperator(previousPartDef, refSNode);

  Ptr<Id> accessId = new Id();
  syntaxerContext.model->delayDeletedIds.push_back(accessId);

  accessId->definition(accessOp.object());
  accessId->callArglist = (*currentPartIt)->callArglist;
  accessId->loc(referenceId->getLLoc());
  accessId->semanticNode(refSNode);
  (*currentPartIt)->callArglist = NULL;

  currentPartIt = ref->insert(currentPartIt, accessId);
  if (currentPartIt == ref->begin()) // inserting accessor before of variable reference;
    return RESOLVING_FINISH_SUCCESS;

  do {
    previousPartDef = (*currentPartIt)->unresolvedDefinition();
    --currentPartIt;
    if ((*currentPartIt)->getText().empty()) {
      if ((*currentPartIt)->callArglist) {
        Ptr<AccessToItem> accessOp = createCollectionAccessOperator(previousPartDef, refSNode);
        (*currentPartIt)->definition(accessOp.object());
        (*currentPartIt)->loc(referenceId->getLLoc());
      }
    }
    else {
      if (checkToCollectionMethod(*currentPartIt, previousPartDef))          
        return RESOLVING_FINISH_SUCCESS;

      Ptr<Datatype> prevPartT = SyntaxUnwrapper::unwrap(previousPartDef->getDatatype());
      previousPartDef->getFieldRef(*currentPartIt);
      ResolvedEntity *currentPartDef = (*currentPartIt)->definition();

      if (currentPartDef && prevPartT) {
        if (currentPartDef->isField() && (*currentPartIt)->callArglist && SyntaxUnwrapper::unwrap(currentPartDef->getDatatype())->isCompositeType()) {
          Ptr<AccessToItem> accessOp = createCollectionAccessOperator(currentPartDef, refSNode);
          Ptr<Id> accessId = new Id();
          accessId->definition(accessOp.object());
          accessId->callArglist = (*currentPartIt)->callArglist;
          accessId->loc(referenceId->getLLoc());
          accessId->semanticNode(refSNode);
          (*currentPartIt)->callArglist = NULL;
          currentPartIt = ref->insert(currentPartIt, accessId);
        }
        else if (prevPartT->isCompositeType() && !currentPartDef->isField()) {
          (*currentPartIt)->definition(0);
          ++currentPartIt;
          return RESOLVING_NEXT_WITHOUT_SPECIAL_CASES;
        }
      }
      else if (checkToCollectionMethod(*currentPartIt, previousPartDef))
        return RESOLVING_FINISH_SUCCESS;
    }
  } while (currentPartIt != ref->begin());

  if ((*currentPartIt)->definition() && currentPartIt == ref->begin())
    return RESOLVING_FINISH_SUCCESS;
  ++currentPartIt;
  return RESOLVING_NEXT_WITHOUT_SPECIAL_CASES;
}

ResolvingContext::ResolvingSpecialCasesState ResolvingContext::resolveSpecialCases() {
  // currentPartIt в данной позиции - старшее имя в referenceId
  ResolvedEntity *currentPartDef = (*currentPartIt)->definition();
  if (!currentPartDef)
    return RESOLVING_FINISH_FAIL; // ситуация, когда какое то имя всё-таки было найдено, но в глубине оно не было разрешено.
                  // из за отсутствия некоторых элементов модели. такое возможно, например, когда запрашивается поле-позиционный псевдоним
                  // представления, для которого конкретное поле не имеет определения.


  if (foundedNode && currentPartDef->getSemanticNode())
    currentPartDef->setSemanticNode(foundedNode);

  switch (ResolvingContext::ResolvingSpecialCasesState state = resolveCollectionAccess()) {
    case RESOLVING_NEXT:
      break;
    default:
      return state;
  }

  SemanticTree *currentSemNode = (*currentPartIt)->semanticNode();

  switch (currentSemNode->cathegory) {
    case SCathegory::FromTableReference:
      if (!currentPartDef->usedInQueryAndContainsFields())
        return RESOLVING_FINISH_FAIL; // не должно встречаться одиночных ссылок (т.е. без задания полей) на таблицы,
                                      // представления, алиасы From, пользователей и элементы факторизации
      break;
    case SCathegory::RefAbstract:
      if (referenceId->updateQuotedUserIdToVarchar2Literal())
        return RESOLVING_FINISH_SUCCESS;
      break;
    default:
      break;
  }


  if (resolveAsArgumentOfCurrentFunctionNamespace() || !(*currentPartIt)->hasNoncursorCallSemantic())
    resolveTypecallEntity();
  else { // разрешение особых случаев семантики некурсорного вызова функции
    if (isSystemTemplateLastRef())
      return RESOLVING_FINISH_SUCCESS;
    else if (!resolveFuncallEntity())
      return RESOLVING_FINISH_FAIL;
  }

  return RESOLVING_NEXT;
}

bool ResolvingContext::commonFindDeclarationFromLevel() {
  if ((*currentPartIt)->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";

  if (referenceId->size() == 1) {
    Ptr<Id> &q = *(referenceId->begin());
    if (q->squoted()) {
      q->setStringLiteral();
      q->definition(Datatype::mkVarchar2(q->length()));
      return true;
    }
  }

  // поиск вверх - искать вверх налево - начиная с предка ссылки.
  if (ResolvedEntity *d = (*currentPartIt)->unresolvedDefinition())  {
    if ((foundedNode = d->getSemanticNode()))
      foundedNode->unnamedDdlEntity = d;
  }
  else {
    foundedLevel = startLevel->levelNamespace->semanticLevel;
    push(reference, foundedLevel);
    if (!commonFindDeclarationUp())
      return referenceId->updateUnresolvedQuotedToVarchar2Literal();
  }
  switch (resolveSpecialCases()) {
    case RESOLVING_FINISH_FAIL:
      return false;
    case RESOLVING_FINISH_SUCCESS:
      return true;
    default:
      break;
  }

  if (bool previousEqCondition = currentPartIt != referenceId->begin()) {
    ResolvedEntity *foundedNodeDef = (*currentPartIt)->definition();
    for (--currentPartIt; previousConditionResult(currentPartIt != referenceId->begin(), previousEqCondition); --currentPartIt) {
      if (foundedNodeDef) {
        foundedNodeDef->getFieldRef(*currentPartIt);
        if (!(*currentPartIt)->unresolvedDefinition())
          resolveAndFindRetry(*currentPartIt, foundedNodeDef);
      }

      /*
       * TODO: для RefCursor - ов найти предыдущую инициализацию Select-ом для данного экземпляра
       * RefCursor и выцепить из него структуру полей, по которой и выполнить резолвинг текущего имени
       */

      if (!(*currentPartIt)->unresolvedDefinition()) {
        static const auto exitUnresolved = [](IdEntitySmart::iterator it, ResolvedEntity *def) -> bool {
          if (!def || !def->toSelfUserContext())
            syntaxerContext.model->partitiallyResolvedNodes.insert(*next(it));
          return false;
        };

        if (foundedNode) {
          if (!foundedNode->commonFindDeclarationOnLevelDown(this)
              || !(*currentPartIt)->unresolvedDefinition()) // предыдущая часть имени не найдена. следующую часть имени найти не получится.
            return exitUnresolved(currentPartIt, foundedNodeDef);
        }
        else
          return exitUnresolved(currentPartIt, foundedNodeDef);
      }

      if (foundedNodeDef->ddlCathegory()                 == ResolvedEntity::NestedTable_  &&
          (*currentPartIt)->definition()->ddlCathegory() == ResolvedEntity::CollectionMethod_)
        (*currentPartIt)->unresolvedDefinition()->toSelfCollectionMethod()->collectionVariableRef = foundedNodeDef;

      foundedNode = (*currentPartIt)->unresolvedDefinition()->getSemanticNode();
      foundedNodeDef = (*currentPartIt)->unresolvedDefinition();

      switch (resolveCollectionAccess()) {
        case RESOLVING_NEXT:
        case RESOLVING_NEXT_WITHOUT_SPECIAL_CASES:
          break;
        case RESOLVING_FINISH_FAIL:
          return false;
        case RESOLVING_FINISH_SUCCESS:
          return true;
      }

      resolveTypecallEntity();

      if ((*currentPartIt)->callArglist)
        if (!resolveFuncallEntity())
          return false; // функцию разрешить не удалось. Значит не удастся и разрешить поле в ее возвращаемом значении.
    }
  }
  return true;
}


bool SemanticTree::findDeclaration() {
  return refEmpty() ? false : resolveOneCallReference();
}

/// Просканировать на системные типы.
bool resolveSystemDatatype(SemanticTree &node, ModelContext &model) {
  if (node.refSize() > 1 /*|| node.ddlNameSize() > 1*/ )
    return false;
  Ptr<Id> entity;
//  if ( !(entity = node.ddlNameEntity()) )
  entity = node.refEntity();
  if (!entity)
    return false;

  if ( Ptr<GlobalDatatype::FundamentalDatatype> *t = model.globalDatatypes.systemTypesMap.find(THCChar(entity->toNormalizedString())) ) {
    entity->definition((*t).object());
    return true;
  }
  return false;
}

namespace Sm {

void SemanticTree::resolveCurrent() {
  if (childsResolved())
    return;
  else
    setChildsResolved();

  resolvingProgress.totalTraversed++;
  if (resolvingProgress.totalTraversed > streeGlobalCounter__)
    throw 999;

  switch (nametype) {
    case REFERENCE:
      findDeclaration();
      break;
    case SemanticTree::DATATYPE_REFERENCE:
      if (!resolveSystemDatatype(*this, *syntaxerContext.model)) {
        // если системных типов не найдено - заполнить допустимые элементы, и затем вызвать обобщенный резолвинг
        // allowed.masks.push_back(SCathegory::Datatype);
        // это и Object и Subtype и другие
        // отрезолвленная ссылка может быть и таблицей (например для конструкций TABLE OF <имя таблицы>)
        findDeclaration();
        break;
      }
      //pass-through
    default:
      if (!refEmpty() && !refUnresolvedDefinition())
        findDeclaration();
      break;
  }
}

void SemanticTree::resolveReference() {
  if (isLoopRecursion()) // Защита от бесконечной рекурсии.
    return;
  else
    openNode();


  for (SemanticTree::Childs::iterator it = this->childs.begin(); it != this->childs.end(); ++it)
    (*it)->resolveReference();

  resolveCurrent();
}


void ResolvingContext::resetPartitialResolvedDefs()
{
  partitialResolvedDefs.clear();
  for (IdEntitySmart::iterator it = referenceId->begin(); it != referenceId->end(); ++it)
    partitialResolvedDefs.push_back((*it)->unresolvedDefinition());
}

bool ResolvingContext::resolveStartFromLevel()
{
  bool savedIsExpression = isExpression;
  if ((*currentPartIt)->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";

  if (!isExpression && startLevel) {
    if (startLevel->cathegory == SCathegory::RefAbstract)
      isExpression = true;
  }

  bool resolvedOk = false;
  bool retval = true;
  while (!(resolvedOk = commonFindDeclarationFromLevel())) {
    if (foundedLevel && foundedLevel->cathegory != SCathegory::RootSemanticNode)
      startLevel = foundedLevel;
    else {
      isExpression = savedIsExpression;
      retval = false;
      break;
    }
    unsigned int cnt = 0;
    for (IdEntitySmart::iterator it = referenceId->begin(); it != referenceId->end(); ++it)
      if ((*it)->unresolvedDefinition())
        ++cnt;
    if (cnt > oldcnt) {
      oldcnt = cnt;
      resetPartitialResolvedDefs();
    }
    for (IdEntitySmart::iterator it = referenceId->begin(); it != referenceId->end(); ++it)
      (*it)->clearDefinition();
    IdEntitySmart::iterator m = referenceId->majorEntityIter();
    currentPartIt = m;
    m->object()->incContextLevelUp();
  }
//  if (!resolvedOk)
//    syntaxerContext.model->partitiallyResolvedNodes.insert(referenceId);
  if (referenceId && !resolvedOk && oldcnt ==  referenceId->size()) {
    std::vector<ResolvedEntity*>::iterator pIt = partitialResolvedDefs.begin();
    for (IdEntitySmart::iterator it = referenceId->begin(); it != referenceId->end(); ++it, ++pIt)
      (*it)->definition(*pIt);
  }
  isExpression = savedIsExpression;
  return retval;
}

bool SemanticTree::isPartitialResolved(IdEntitySmart::reverse_iterator &rit)
{
  bool isPartitialResolved = false;
  for (rit = referenceName_->rbegin();  rit != referenceName_->rend(); ++rit) {
    if (!(*rit))
      break;
    else if (!(*rit)->definition())
      isPartitialResolved = true;
    else // (*rit)->definition() is set
      break;
  }

  return isPartitialResolved;
}

bool SemanticTree::resolveOneCallReference() {
  if (!parent)
    throw 999;

  Id *ent = refEntity();
  if (!ent->callArglist && ent->emptyStr())
    return ent->definition();

  Sm::ResolvingContext globalResolvingContext;
  currentResolvingContext = &globalResolvingContext;

  globalResolvingContext.setReference(referenceName_, *positionInParent);
  if (globalResolvingContext.resolveStartFromLevel()) {
    currentResolvingContext = 0;
    return ent->definition();
  }

  currentResolvingContext = 0;
  return ent->definition();
}

void Type::Object::resolveMemberInSupertype(Ptr<Id> name, FunctionResolvingContext &cntx) const {
  Id *supertypeName;
  ResolvedEntity *supertypeDef;

  if (supertype &&
      (supertypeName = supertype->entity()) &&
      (supertypeDef  = supertypeName->definition()) &&
      (supertypeDef->ddlCathegory() == Object_))
    supertypeDef->resolveMember(name, cntx);
}

void FunctionResolvingContext::resolveMemberFunction(Ptr<Id> name, ResolvedEntity *mf)
{
  if (tryResolveFunref(mf, name))
    return;
  else if (mf->isMemberFunctionConstructor() && mf->arglistSize() == 0) {
    Context ctx(mf, name);
    ctx.summaryPower  = 1;
    ctx.hasEverything = false;
    updateMaximalResolvedFundecl(ctx);
  }
}

void FunctionResolvingContext::resolveFuncallByMaximalPower(VEntities &container, Id *call) {
  if (!container.owerloadResolvingEntered())
    container.resolveOverloaded();

  // TODO: можно проходить не по всем функциям, а только по перегруженным
  for (VEntities::OverloadedFunctions::iterator it = container.overloadedFunctions.begin(); it != container.overloadedFunctions.end(); ++it)
    if (Sm::Function *fundecl = it->first->castToFunction()) {
      tryResolveFunref(fundecl, call);
      call->clearDefinition();  // обнулить ссылку и искать дальше
    }

  if (maximalResolvedFundecl == nullptr && container.overloadedFunctions.size() != container.functions.size())
    for (VEntities::Container::iterator it = container.functions.begin(); it != container.functions.end(); ++it)
      if (Sm::Function *fundecl = (*it)->castToFunction()) {
        tryResolveFunref(fundecl, call);
        call->clearDefinition();  // обнулить ссылку и искать дальше
      }
  if (ambiguousDeclarations) {
    call->clearDefinition();
    ambiguousDeclarations = false;
    currentPower = std::numeric_limits<int>::min();
    maximalResolvedFundecl = nullptr;
  }
}


bool FunctionResolvingContext::setMaxMatchedResolvedFunction(Id *call) {
  if (maximalResolvedFundecl == nullptr)
    return call->unresolvedDefinition() != nullptr;
  call->definition(maximalResolvedFundecl);
  call->setResolvedFunction();
  return true;
}


void Type::Object::resolveMember(Ptr<Id> name, FunctionResolvingContext &cntx) const
{
  if (!name)
    return;
  if (objectBody) {
    objectBody->levelNamespace(getSemanticNode()->levelNamespace);
    objectBody->resolveMember(name, cntx);
    return;
  }
  if (name->beginedFrom(syntaxerContext.debugTokenLocation))
    cout << "";
  if (*name == *this->getName()) {
    if (constructors.size())
      for (List<MemberFunction>::const_iterator it = constructors.begin(); it != constructors.end(); ++it)
        cntx.resolveMemberFunction(name, it->object());
    else {
      name->definition(getDefaultConstructor());
      name->setResolvedFunction();
    }
  }
  else {
    for (List<MemberInterface>::const_iterator it = elements.begin(); it != elements.end(); ++it) {
      if (Ptr<Id> memberName = (*it)->getMemberName()) {
        if (*memberName == *name) {
          switch ((*it)->cathegoryMember()) {
          case MemberInterface::FUNCTION:
            cntx.resolveMemberFunction(name, static_cast<ResolvedEntity*>(it->object()));
            break;
          case MemberInterface::VARIABLE: {
            name->definition(static_cast<ResolvedEntity*>(it->object()));
            return;
          } break;
          default:
            break;
          }
        }
      }
    }
    resolveMemberInSupertype(name, cntx);
  }
}


template <typename T>
bool LinterSemanticResolver::resolveCreatedLinterEntity(T &container, const std::string &name, ResolvedEntity *&foundPtr) {
  typename T::iterator it = container.find(name);
  if (it != container.end())  {
    foundPtr = it->second.object();
    syntaxerContext.model->linterCreatedResolvedEntities.insert(foundPtr->getDefinitionFirst());
    return true;
  }
  return false;
}

bool LinterSemanticResolver::resolveNestedFuncByName(
    ResolvedEntity *parent,
    ResolvedEntity *&def,
    const string   &nestedName,
    const string   &funcName) {

  Ptr<Sm::Id> fName = new Id(funcName);
  fName->setSkipFunctionResolving();
  parent->getFieldRef(fName);
  if (ResolvedEntity *mainDef = fName->unresolvedDefinition()) {
    if ((mainDef = mainDef->getDefinitionFirst())) {
      Ptr<Sm::Id> nName = new Id(nestedName);
      nName->setSkipFunctionResolving();
      SemanticTree *node = mainDef->getSemanticNode();
      SemanticTree *foundNode = NULL;
      LevelResolvedNamespace *nspace = node->childNamespace;

      for (LevelResolvedNamespace::Childs::iterator it = nspace->childs.begin(); it != nspace->childs.end(); ++it) {
        if ((*it)->findDeclaration(foundNode, nName) && foundNode) {
          def = foundNode->unnamedDdlEntity;
          break;
        }
      }
    }
  }

  return def != NULL;
}

bool LinterSemanticResolver::resolveNestedFunc(ResolvedEntity *parent, const std::string &name, ResolvedEntity *&def) {
  string funcName, nestName;
  for (std::string::size_type splitterPos = std::string::npos;
       (splitterPos = name.rfind('_', splitterPos)) != std::string::npos;
       --splitterPos) {
    funcName = name.substr(0, splitterPos);
    nestName = name.substr(splitterPos + 1);
    if (!nestName.size())
      continue;
    if (!funcName.size())
      break;
    if (resolveNestedFuncByName(parent, def, nestName, funcName))
      break;
  }
  return def != NULL;
}

template<typename Container>
bool LinterSemanticResolver::resolveNestedEntityByName(
    Container             &c,
    ResolvedEntity       *&def,
    string                &funNameS,
    string                &pkgName)
{
  typename Container::iterator pIt = c.find(pkgName);
  if (pIt != c.end()) { // найти пакет.
    std::string funNameStr = funNameS;
    std::string trName = pkgName + "_" + funNameS;
    if (trName == "C_MRP_GET_RESUNITQUANTITY_FOR_PWS1")
      cout << "";

    Ptr<Sm::Id> funName = new Id(string(funNameS));
    bool isBackDigit;
    do {
      funName->setSkipFunctionResolving();
      pIt->second->getFieldRef(funName);
      if (funName->definition()) { // найти в пакете переменную или функцию
        // Разрешение по точному соответствию имени. В общем случае это не совем верно - т.к. перегрузка изначально зависит от списка аргументов.
        ResolvedEntity *d = funName->unresolvedDefinition()->getDefinitionFirst();
        if (funNameStr.size() != funNameS.size()) {
          if (VEntities *vEnt = d->vEntities())
            for (VEntities::OverloadedFunctions::iterator fit = vEnt->overloadedFunctions.begin(); fit != vEnt->overloadedFunctions.end(); ++fit) {
              std::string n = fit->first->translatedName();
              transform(n.begin(), n.end(), n.begin(), ::toupper);
              if (n == trName) {
                def = fit->first;
                break;
              }
            }
        }
        else {
          def = d;
          break;
        }
      }
      else {
        if (resolveNestedFunc(pIt->second, funNameS, def))
          break;
      }

      if ((isBackDigit = isdigit(funNameStr.back()))) {
        funNameStr.pop_back();
        funName = new Id(string(funNameStr));
      }
    } while (!(def) && isBackDigit && funNameStr.size());
  }
  return def != NULL;
}


void LinterSemanticResolver::resolveNestedEntity(UserContext *user, const std::string &name, ResolvedEntity *&def) {
  def = NULL;
  std::string::size_type splitterPos;
  string pkgName;
  string funNameS = name;
  while ((splitterPos = funNameS.find('_')) != std::string::npos) {
    if (pkgName.size())
      pkgName.push_back('_');
    pkgName.append(funNameS.substr(0, splitterPos));
    funNameS.erase(0, splitterPos+1);
    if (resolveNestedEntityByName(user->packages, def, funNameS, pkgName))
      break;
    if (resolveNestedEntityByName(user->types, def, funNameS, pkgName))
      break;
    if (resolveNestedFuncByName(user, def, funNameS, pkgName))
      break;
  }
}

void LinterSemanticResolver::resolve(IdEntitySmart &reference) {
  UserContext *user;
  if (reference.size() != 2)
    return;

  for (IdEntitySmart::reverse_iterator rit = reference.rbegin(); rit != reference.rend(); ++rit) {
    (*rit)->setSkipFunctionResolving();
    if (rit == reference.rbegin()) {
      syntaxerContext.model->getFieldRef(*rit);
      ResolvedEntity *def = (*rit)->definition();
      if (!def || !(user = def->toSelfUserContext()))
        break;
    }
    else if (user) {
      user->getFieldRef(*rit);
      if (!(*rit)->definition()) {
        ResolvedEntity *def;
        resolveNestedEntity(user, (*rit)->toNormalizedString(), def);
        (*rit)->definition(def);
      }
    }
    else
      break;
  }
}

void LinterSemanticResolver::resolveCreatedObjects(LinterCreatedEntities &linterCreatedEntities) {
  ModelContext::UserDataMap &userMap = syntaxerContext.model->userMap;
  for (LinterCreatedEntities::iterator it = linterCreatedEntities.begin(); it != linterCreatedEntities.end(); ++it) {
    ModelContext::UserDataMap::iterator usrIt = userMap.find(it->first);
    if (usrIt != userMap.end())
      for (LinterCreatedEntities::mapped_type::iterator userEntsIt = it->second.begin(); userEntsIt != it->second.end(); ++userEntsIt)
        for (LinterCreatedEntities::mapped_type::mapped_type::iterator entIt = userEntsIt->second.begin(); entIt != userEntsIt->second.end(); ++entIt)
          switch (userEntsIt->first) {
            case ResolvedEntity::Table_   :
              resolveCreatedLinterEntity(usrIt->second->tables   , entIt->first, entIt->second);
              if (syntaxerContext.printExistedLinterEntities)
                cout << "TABLE: " << usrIt->first << "." << entIt->first << endl;
              break;
            case ResolvedEntity::View_    : resolveCreatedLinterEntity(usrIt->second->views    , entIt->first, entIt->second); break;
            case ResolvedEntity::Sequence_: resolveCreatedLinterEntity(usrIt->second->sequences, entIt->first, entIt->second); break;
            case ResolvedEntity::Synonym_ : resolveCreatedLinterEntity(usrIt->second->synonyms , entIt->first, entIt->second); break;
            case ResolvedEntity::Function_:
            case ResolvedEntity::Variable_: {
              UserContext::Functions::iterator it = usrIt->second->functions.find(entIt->first);
              // TODO: нужно обобщить для всех встречающихся ключевых слов и регистронезависимости
              if (it == usrIt->second->functions.end() && entIt->first == "get")
                it = usrIt->second->functions.find("GET");
              if (it != usrIt->second->functions.end())   // поиск по непакетным функциям
                syntaxerContext.model->linterCreatedResolvedEntities.insert(it->second->getDefinitionFirst());
              else {
                resolveNestedEntity(usrIt->second, entIt->first, entIt->second);
                if (entIt->second)
                  syntaxerContext.model->linterCreatedResolvedEntities.insert(entIt->second);

                switch (userEntsIt->first) {
                  case ResolvedEntity::Function_:
                    if (syntaxerContext.printExistedLinterEntities && syntaxerContext.printExistedProcedures)
                      cout << "PROCEDURE: " << usrIt->first << "." << entIt->first << " : <= " << ( entIt->second ? entIt->second->translatedName() : string()) << endl;
                    break;
                  case ResolvedEntity::Variable_:
                    if (syntaxerContext.printExistedLinterEntities && syntaxerContext.printExistedVariables)
                      cout << "VARIABLE: " << usrIt->first << "." << entIt->first << " : <= " << (entIt->second ? entIt->second->translatedName() : string()) << endl;
                    break;
                  default: ;
                }
              }
              break;
            }
            case ResolvedEntity::Trigger_ : resolveCreatedLinterEntity(usrIt->second->triggers, entIt->first, entIt->second); break;
            case ResolvedEntity::User_    : resolveCreatedLinterEntity(userMap                , entIt->first, entIt->second); break;
            default:
              break;
          }
  }
}


}



// vim:foldmethod=syntax
