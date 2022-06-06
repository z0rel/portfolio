#include "model_context.h"
#include "codegenerator.h"
#include "semantic_table.h"
#include "semantic_function.h"
#include "semantic_collection.h"
#include "semantic_blockplsql.h"
#include "resolvers.h"

using namespace std;
using namespace Sm;

extern SyntaxerContext syntaxerContext;

bool g_callConstructor = false;
bool g_callSubconstruct = false;

namespace Sm {

namespace Type {

ObjectType::ObjectType(CLoc l, Ptr<Id2> _name, Ptr<Id> _oid)
  : GrammarBase(l), name(_name), oid(_oid), addTypeName_(false) { setNameDefinition(); }

Object::~Object() {}

BaseList<MemberInterface> *Object::getMembers()      { return &elements; }

void Object::collectInitializers(EntitiesSet &container) {
  for (Elements::value_type &e : elements)
    if (Sm::Type::MemberFunction *fn = e->toSelfMemberFunction())
      fn->collectInitializers(container);
  for (auto &c : constructors)
    c->collectInitializers(container);
}

bool Object::isRefCursor() const {
  if (isSystem()) {
    static const Id v("SYS_REFCURSOR");
    return name && name->entity() && *(name->entity()) == v;
  }
  return false;
}


bool ObjectType::isXmlType() const {
  static const Id v("XMLTYPE");
  return name && name->entity() && *(name->entity()) == v;
}

bool ObjectType::isAnydata() const {
  static const Id v("ANYDATA");
  return name && name->entity() && *(name->entity()) == v && isSystem();
}

bool ObjectType::isAnytype() const {
  static const Id v("ANYTYPE");
  return name && name->entity() && *(name->entity()) == v && isSystem();
}

bool ObjectType::isOracleNontranslatedDatatype() const {
  static const std::map<std::string, bool> systypes = {
    {"XMLTYPE"      , false},
    {"ANYDATA"      , false},
    {"SYS_REFCURSOR", true },
    {"ANYTYPE"      , true }
  };
  if (name && name->entity()) {
    map<std::string, bool>::const_iterator it = systypes.find(name->entity()->toNormalizedString());
    if (it != systypes.end())
      return it->second ? isSystem() : true;
  }
  return false;
}


void translateOneId(Sm::Codestream &str, Ptr<Id> idRef) {
  ResolvedEntity *def = idRef->definition();
  if (!def || idRef->toNormalizedString() == "SELF")
    str << idRef;
  else
    def->linterReference(str);
}

#define PUSH_REFLIST(inf) { \
  decodedRefs.push_back(inf);\
  if (prevInfo) prevInfo->next = inf;\
  prevInfo = inf;\
}

ResolvedEntity *RefDecoder::getMethodObject(ResolvedEntity *method) {
  if (method->ddlCathegory() == ResolvedEntity::CollectionMethod_)
    return static_cast<collection_methods::CollectionMethod*>(method)->getCollection();
  else
    return method->getOwner();
}

Ptr<RefInfo> RefDecoder::newFieldInfo(RefInfo *refInfo, Ptr<Id> field) {
  Ptr<RefInfo> newInfo;
  if (refInfo->idRef->isMethod() && !refInfo->idRef->definition()->isCollectionAccessor()) {
    // Добавим новый нод в начало
    newInfo = new RefInfo(field->toNormalizedString(), field->getDatatype(), field, refInfo->fieldType->getNextDefinition());
    newInfo->next = refInfo;
  }
  else {
    // Заменим первый нод извлечения поля
    newInfo = new RefInfo(*refInfo);
    newInfo->next = refInfo->next;
    newInfo->fieldType = field->getDatatype();
    newInfo->fieldName += (!newInfo->fieldName.empty() ? "$" : "") + field->toNormalizedString();
  }
  return newInfo;
}

void RefDecoder::copyFields(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  Sm::RefAbstract *sqlExprId;
  if ((sqlExprId = expr->toSelfRefAbstract()) == NULL)
    throw 999; // Поддерживается только RefAbstract тип

  ResolvedEntity *record = refInfo->fieldType->getNextDefinition();
  EntityFields fields;
  record->getFields(fields);

  for (EntityFields::iterator it = fields.begin(); it != fields.end(); ) {
    Ptr<RefInfo> newInfo = newFieldInfo(refInfo, *it);
    Ptr<IdEntitySmart> newExprRef = new IdEntitySmart();
    newExprRef->reserve(sqlExprId->refSize() + 1);
    newExprRef->push_back(*it);
    newExprRef->insert(newExprRef->end(), sqlExprId->reference->begin(), sqlExprId->reference->end());
    Ptr<Sm::RefExpr> newExpr = new Sm::RefExpr(sqlExprId->getLLoc(), newExprRef);
    refInfo->translateAssign(str, newInfo.object(), newExpr.object());
    str << s::semicolon << s::name << s::Comment() << "Copy " << *it;
    if (++it != fields.end())
      str << s::endl << s::tab();
  }
}

bool RefDecoder::copyCollection(Codestream          &str,
                                const IdEntitySmart &lEntity,
                                ResolvedEntity      *lObject,
                                const IdEntitySmart &rEntity,
                                ResolvedEntity      *rObject) {
  ResolvedEntity *lValueDef = lEntity.definition();
  ResolvedEntity *rValueDef = rEntity.definition();
  if ((lValueDef->isVariable() || lValueDef->isFunArgument()) &&
      (rValueDef->isVariable() || rValueDef->isFunArgument())) {
    if (rValueDef->isTempVariable())
      return false;
    // Копируем все записи из коллекции rValueDef в коллекцию lValueDef
    string methodName = lObject->toSelfObjectType()->getMethodName(ObjectType::BM_COPY_ALL, 0, NULL);
    str << "CALL " << methodName;
    str << s::obracket << lEntity << s::comma() << rEntity << s::comma();
    translateQueryFieldsEscape(str, rObject, true);
    str << s::cbracket;
    return true;
  }

  Ptr<Datatype> lType = lValueDef->getDatatype()->getFinalType();
  Ptr<Datatype> rType = rValueDef->getDatatype()->getFinalType();
  if ((lType->isRowTypeOf() || lType->isRecordType()) &&
      (rType->isRowTypeOf() || rType->isRecordType())) {
    // Копируем все поля для одной записи из коллекции rValueDef в lValueDef по индексам
    ObjectType::ExtractColFields lExtrFields, rExtrFields;
    if (!ObjectType::extractCollectionAccessorFields(lEntity, lExtrFields))
      throw 999;
    if (!ObjectType::extractCollectionAccessorFields(rEntity, rExtrFields))
      throw 999;
    if (lExtrFields.opFields.size() != rExtrFields.opFields.size())
      throw 999;
    if (lExtrFields.indexExpr->getDatatype()->isExactlyEqually(Datatype::mkVarchar()) && !rExtrFields.indexExpr->getDatatype()->isExactlyEqually(Datatype::mkVarchar()))
      throw 999; // Функции SYS.??_COPY_IDX не расчитаны на работу с коллекциями разных типов.
    if (rExtrFields.indexExpr->getDatatype()->isExactlyEqually(Datatype::mkVarchar()) && !lExtrFields.indexExpr->getDatatype()->isExactlyEqually(Datatype::mkVarchar()))
      throw 999; // Функции SYS.??_COPY_IDX не расчитаны на работу с коллекциями разных типов.

    string methodName = lObject->toSelfObjectType()->getMethodName(ObjectType::BM_COPY_IDX, 0, NULL);
    str << "CALL " << methodName << s::obracket;
    str << lExtrFields.newReference << s::comma() << lExtrFields.indexExpr << s::comma();
    str << rExtrFields.newReference << s::comma() << rExtrFields.indexExpr << s::comma();

    string fields1, fields2;
    Codestream fStr1, fStr2;
    translateQueryFields(fStr1, lExtrFields.opFields, false, "");
    translateQueryFields(fStr2, rExtrFields.opFields, false, "");
    PlsqlHelper::quotingAndEscapingSpecOnlyQuote(fields1, fStr1.str(), '\"', '\\');
    PlsqlHelper::quotingAndEscapingSpecOnlyQuote(fields2, fStr2.str(), '\"', '\\');
    if (fields1 != fields2) {
      cout << "ERROR: COPY COLLECTION: fields has different names, need change functions SYS.??_COPY_IDX" << endl;
      return true;
    }
    str << fields1;
    str << s::cbracket;
    return true;
  }

  return false;
}

bool RefDecoder::copyFromCursorFields(Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity */*lObject*/, const IdEntitySmart &rEntity, ResolvedEntity *rCursor) {
  if (rEntity.size() > 1)
    throw 999; // Не поддерживается больше одной ссылки в курсоре.
  if (rCursor->isFunction()) {
    SemanticTree *n;
    if ((n = lEntity.majorEntity()->semanticNode()) && n->unnamedDdlEntity)
      n->unnamedDdlEntity->translateAsUserFunctionCall(str, const_cast<IdEntitySmart &>(rEntity), false, true, &rCursor);
    else if (!lEntity.majorEntity()->definition()->translateAsUserFunctionCall(str, const_cast<IdEntitySmart &>(rEntity), false, true, &rCursor))
      throw 999;
  }

  translateCursorAsInsertCollection(str, lEntity, rCursor);
  return true;
}

bool RefDecoder::copyToCursorFields(Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity *lCursor, const IdEntitySmart &rEntity, ResolvedEntity *rObject) {
  if (lEntity.size() > 1)
    throw 999; // Не поддерживается больше одной ссылки в курсоре.
  translateCursorAsSelectCollection(str, rEntity, rObject, lCursor);
  return true;
}

bool RefDecoder::copyAssignment(Sm::Codestream &str, const IdEntitySmart &lEntity, const IdEntitySmart &rEntity) {
  if (lEntity.entity() && lEntity.entity()->beginedFrom(23670))
    cout << "";

  ResolvedEntity *lValueDef = lEntity.definition();
  ResolvedEntity *rValueDef = rEntity.definition();
  if (!lValueDef) {
    //trError(str, s << "ERROR: unresolved lValue entity: " << lEntity << ":" << lEntity.entity()->getLLoc());
    return false;
  }
  if (!rValueDef) {
    //trError(str, s << "ERROR: unresolved rValue entity: " << rEntity << ":" << rEntity.entity()->getLLoc());
    return false;
  }

  Ptr<Datatype> lType = lValueDef->getDatatype();
  if (lType)
    lType = lType->getFinalType();
  Ptr<Datatype> rType = rValueDef->getDatatype();
  if (rType)
    rType = rType->getFinalType();
  if (!lType) {
    //trError(str, s << "ERROR: undefined type of" << lValueDef);
    return false;
  }
  if (!rType) {
    //trError(str, s << "ERROR: undefined type of" << rValueDef);
    return false;
  }
  ResolvedEntity *lObject = NULL, *rObject = NULL;
  lEntity.majorObjectRef(&lObject);
  rEntity.majorObjectRef(&rObject);

  if (lObject && rObject) {
    if ((lObject->isAnydata() || lObject->isXmlType()) ||
        (rObject->isAnydata() || rObject->isXmlType()))
      return false;
    if (rValueDef->isConstructor())
      return false;
    return copyCollection(str, lEntity, lObject, rEntity, rObject);
  }
  else if (rObject && (lValueDef->isCursorVariable() || lType->isRowTypeOf() || lType->isRecordType())) {
    return copyToCursorFields(str, lEntity, lValueDef, rEntity, rObject);
  }
  else if (lObject && (rValueDef->isCursorVariable() || rType->isRowTypeOf() || rType->isRecordType())) {
    return copyFromCursorFields(str, lEntity, lObject, rEntity, rValueDef);
  }
  return false;
}

bool RefDecoder::checkObjectType(Codestream &str, RefInfo *refFirst){
  if (refFirst->objectType)
    return true;

  trError(str, s << "ERROR : refInfo->objectType is NULL: " << refFirst->toDebugString());
  return false;
}

Ptr<RefInfo> RefDecoder::newFieldInfoCursor(ResolvedEntity *var, const IdEntitySmart &entity, IdEntitySmart::const_iterator it) {
  Codestream newStr;
  if (next(it) != entity.end() || entity.size() < 3)
    throw 999; //Нужно разобраться, если эта ссылка не первая.

  var->linterReference(newStr);
  newStr << (var->isPackageVariable() ? "_" : ".");
  ResolvedEntity *field = (*--it)->definition();
  field->linterReference(newStr);
  if ((*--it).object() != (*decodedRefs.begin())->idRef.object())
   throw 999; //Больше двух ссылок в курсоре не поддерживается Линтером

  Ptr<Id> newId = new Id(newStr.str(), NULL);
  return new RefInfo("", field->getDatatype(), newId, NULL);
}

bool RefDecoder::decode(Codestream &str, const IdEntitySmart &entity, Sm::PlExpr *expr) {
  if (entity.entity() && entity.entity()->getLLoc().beginedFrom(2458))
    cout << "";

  if (!g_callConstructor && !entity.isDynamicUsing())
    return false;

  RefInfo *prevInfo = NULL, *newInfo;
  (void)prevInfo;
  Ptr<Id> lastProccessed;
  ResolvedEntity *curObject;
  Ptr<Datatype> datatype, fieldType;
  string fieldName;
  bool pairAdded = false;

  auto prepFieldName = [](string &fName) {
    if (!fName.empty() && (Id::isFieldReserved(fName) || fName.find("#") != string::npos)) {
      fName = "\\\"" + fName + "\\\"";
    }
  };

  if (expr)
    if (Sm::RefAbstract *sqlExprId = expr->toSelfRefAbstract())
      if (copyAssignment(str, entity, *(sqlExprId->reference)))
        return true;

  if (!entity.majorObjectRef(NULL))
    return false;

  for (auto it = entity.begin(); it != entity.end(); ++it) {
    ResolvedEntity *def = (*it)->definition();
    if (!def)
      continue;

    switch (def->ddlCathegory()) {
    case ResolvedEntity::ArrayConstructor_:
      lastProccessed = (*it);
      datatype = def->getDatatype();
      curObject = def->toSelfArrayConstructor()->getOwnerCollection();
      break;
    case ResolvedEntity::MemberFunction_:
    case ResolvedEntity::CollectionMethod_:
      datatype = def->getDatatype();
      if (lastProccessed && lastProccessed->definition()->isField()) {
        if (!def->isConstructor() && datatype && datatype->isObjectType()) {
          if (fieldName.empty() || !fieldType) {
            fieldName = lastProccessed->toNormalizedString();
            fieldType = lastProccessed->getDatatype();
          }
          Ptr<Id> newId = new Id("", datatype);
          newInfo = new RefInfo(fieldName, fieldType, newId, datatype->getNextDefinition());
          PUSH_REFLIST(newInfo);
          fieldName.clear();
          fieldType = NULL;
        }
      }
      lastProccessed = (*it);
      curObject = getMethodObject(def);
      pairAdded = true;
      break;
    case ResolvedEntity::MemberVariable_:
    case ResolvedEntity::FieldOfRecord_:
    case ResolvedEntity::FieldOfTable_:
    case ResolvedEntity::SqlSelectedField_:
    case ResolvedEntity::QueriedPseudoField_:
    case ResolvedEntity::FieldOfVariable_:
      lastProccessed = (*it);
      datatype = def->getDatatype();
      if (!datatype)
        continue;
      datatype = datatype->getFinalType();
      if (it ==  entity.begin() || pairAdded || (datatype && !datatype->isObjectType())) {
        if (!fieldType)
          fieldType = datatype;
        fieldName = string((*it)->toNormalizedString()) + (!fieldName.empty() ? "$" : "") + fieldName;
        pairAdded = false;
        continue;
      }
      curObject = datatype->getNextDefinition();
      pairAdded = true;
      break;
    case ResolvedEntity::FunctionArgument_:
    case ResolvedEntity::Variable_:
      lastProccessed = (*it);
      datatype = def->getDatatype();
      if (!datatype)
        continue;
      datatype = datatype->getFinalType();
      if (!datatype->isObjectType()) {
        if ((def->isCursorVariable() || datatype->isRecordType()) && decodedRefs.size() > 0) {
          Ptr<RefInfo> nInfo = newFieldInfoCursor(def, entity, it);
          PUSH_REFLIST(nInfo);
        }
        continue;
      }
      curObject = datatype->getNextDefinition();
      pairAdded = true;
      break;
    case ResolvedEntity::Object_:
      lastProccessed = (*it);
      datatype = def->getDatatype();
      curObject = def->getOwner();
      pairAdded = true;
      break;
    default:
      continue;
    }

    if (!fieldType)
      fieldType = datatype;

    prepFieldName(fieldName);
    newInfo = new RefInfo(fieldName, fieldType, *it, curObject);
    PUSH_REFLIST(newInfo);
    if (def->isField()) {
      fieldName = lastProccessed->toNormalizedString();
      fieldType = datatype;
    }
    else {
      fieldName.clear();
      fieldType = NULL;
    }
  }

  if (lastProccessed && (!pairAdded || decodedRefs.size() == 1)) {
    // Там где нет self - добавим
    ResolvedEntity *lastDef = lastProccessed->definition();
    switch (lastDef->ddlCathegory()) {
    case ResolvedEntity::CollectionMethod_: {
      if (!lastDef->isCollectionAccessor())
        break;
      collection_methods::CollectionMethod *colMethod = static_cast<collection_methods::CollectionMethod*>(lastDef);
      if (!colMethod->collectionDatatype() || colMethod->collectionDatatype()->ddlCathegory() != ResolvedEntity::MemberVariable_)
        break;
      CollectionType *collection = colMethod->getCollection();
      Ptr<Datatype> newDatatype = new Datatype(collection->getName());
      Ptr<Id> newId = new Id("self", newDatatype);
      fieldName = lastProccessed->toNormalizedString();
      prepFieldName(fieldName);
      newInfo = new RefInfo(fieldName, newDatatype, newId, colMethod->collectionDatatype()->getOwner());
      newInfo->member = lastProccessed;
      PUSH_REFLIST(newInfo);
    } break;
    case ResolvedEntity::MemberVariable_:
    case ResolvedEntity::MemberFunction_: {
      if (lastDef->isConstructor())
        break;
      Ptr<Datatype> newDatatype = lastDef->getDatatype();
      Ptr<Id> newId = new Id("self", newDatatype);
      fieldName = lastProccessed->toNormalizedString();
      prepFieldName(fieldName);
      newInfo = new RefInfo(fieldName, newDatatype, newId, lastDef->getOwner());
      newInfo->member = lastProccessed;
      PUSH_REFLIST(newInfo);
    } break;
    default:
      break;
    }
  }

  if (decodedRefs.size() > 0) {
    if (onlyDecode)
      return true;

    RefInfo *refFirst = (*decodedRefs.begin()).object();
    if (expr) {
      if (refFirst->fieldType && (
          (!refFirst->fieldType->isObjectType() && refFirst->fieldType->isRowTypeOf()) ||
          refFirst->fieldType->isRecordType()))
        copyFields(str, refFirst, expr);
      else
        refFirst->translateAssign(str, refFirst, expr);
    }
    else
      refFirst->translateObjRef(str);
    return true;
  }
  return false;
}

ObjectType::TranslatedMap ObjectType::s_translatedObjMap;
Ptr<Id>                   ObjectType::s_currentConstructedVar;

bool ObjectType::extractCollectionAccessorFields(const IdEntitySmart &reference, ExtractColFields &result) {
  IdEntitySmart &collEntity = result.newReference = reference;

  // Достанем ацессор, его аргумент и поля
  for (IdEntitySmart::iterator it = collEntity.begin(); it != collEntity.end(); ) {
    Ptr<Id> id = *it;
    ResolvedEntity *def = id->definition();
    it = collEntity.erase(it);
    if (def && def->isCollectionAccessor()) {
      Ptr<FunCallArg> arg = *(id->callArglist->begin());
      result.indexExpr = arg->expr();
      result.accessor = id;
      break;
    }
    result.baseName += id->toNormalizedString() + "$";
  }

  ResolvedEntity *def = reference.definition();
  result.opFields.clear();
  if (def && (def->toSelfVariable() || def->toSelfFunctionArgument()))
    def->getFieldsExp(result.opFields, false, result.baseName);
  if (result.opFields.empty()) {
    Ptr<Datatype> datatype = reference.entity()->getDatatype();
    datatype->getFieldsExp(result.opFields, false, result.baseName);
  }

  return result.accessor.valid();
}

void ObjectType::setObjectBody(Ptr<Object> b) { objectBody = b; }

Ptr<Datatype> RecordField::getDatatype() const { return datatype; }

bool Object::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  if (Object *othObj = oth->toSelfObject())
    return this->eqByVEntities(othObj);
  return false;
}

void Object::traverseModelStatements(StatementActor &fun) {
  for (Elements::value_type &v : elements) {
    if (Sm::Function *f = v->toSelfFunction())
      f->traverseModelStatements(fun);
  };
  for (BaseList<Type::MemberFunction>::value_type &v : constructors)
    v->traverseModelStatements(fun);
  if (objectBody && objectBody.object() != this)
    objectBody->traverseModelStatements(fun);
}

bool Record::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  if (Record *othObj = oth->toSelfRecord())
    return this->eqByVEntities(othObj);
  return false;
}

Sm::IsSubtypeValues RecordField::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  throw 999;
  if (eqByVEntities(supertype))
    return EXPLICIT;
  return datatype->isSubtype(supertype, plContext);
}

Ptr<Id> RecordField::getName() const { return name; }

ResolvedEntity *RecordField::getNextDefinition() const { return datatype.object(); }

RecordField::RecordField(CLoc l, Ptr<Id> _name, Ptr<Datatype> t, Ptr<PlExpr> dfltValue, bool _isNotNull)
  : GrammarBase(l), name(_name), datatype(t), defaultValue(dfltValue), isNotNull(_isNotNull) { name->definition((RecordField*)this); }

RecordField::RecordField() : isNotNull(false) {}

void RecordField::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }

SemanticTree *RecordField::toSTreeBase() const {
  if (name) {
    SemanticTree *n = name->toSNodeDecl(SCathegory::RecordField, this);
    ANODE(datatype);
    CTREE(defaultValue);
    return n;
  }
  return 0;
}

RecordField::~RecordField() {}

bool Record::getFieldRef(Ptr<Id> &field) {
  if (!field)
    return false;
  Ptr<ResolvedEntity> ent = (Record*)this;
  if (!isDefinition())
    ent = getDefinitionFirst();
  Ptr<Record> r = ent->toSelfRecord();
  if (r->fields)
    return r->getSemanticNode()->childNamespace->findCollectionField(field);
  return false;
}


Ptr<Id> Record::getName() const { return name; }

bool Record::isDefinition() const { return fields; }

Record::Record(Ptr<Id> _name, Ptr<List<RecordField> > _fields, CLoc l)
  : GrammarBase(l), name(_name), fields(_fields)
{
  if (!fields || fields->empty())
    throw 999;
  if (name)
    name->definition(this);
}

SemanticTree *Record::toSTreeBase() const {
  SemanticTree *node = name->toSNodeDef(SCathegory::RecordType, this);
  CollectSNode(node, fields);
  return node;
}

bool Type::CollectionType::getFieldRef(Ptr<Sm::Id> &field) {
  if (field) {
    if (field->unresolvedDefinition())
      return true;
    else if (Ptr<collection_methods::CollectionMethod> m = collection_methods::CollectionMethod::parse(field, this)) {
      field->definition(m.object());
      return true;
    }
    else if (mappedType_ && mappedType_->getFieldRef(field))
      return true;

  }
  return false;
}

namespace collection_methods {

template <typename T> inline Ptr<CollectionMethod> createCollectionMethod(CollectionType *variable, std::string &&methodName) {
  Ptr<CollectionMethod> res;
  return (res = variable->findMethod(methodName)).valid() ? res : variable->addMethod(methodName, new T(variable));
}
typedef std::map<std::string, std::function<Ptr<CollectionMethod>(CollectionType *)> > MethodCreatorMap;

Ptr<CollectionMethod> CollectionMethod::parse(Ptr<Id> field, CollectionType *variable) {
  static const MethodCreatorMap collectionMethods = {
    {"COUNT" , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Count       >(v, "COUNT" ); } }, // 0
    {"FIRST" , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<First       >(v, "FIRST" ); } }, // 0
    {"LAST"  , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Last        >(v, "LAST"  ); } }, // 0
    {"LIMIT" , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Limit       >(v, "LIMIT" ); } }, // 0
    {"TRIM"  , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Trim        >(v, "TRIM"  ); } }, // 0,1
    {"DELETE", [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Delete      >(v, "DELETE"); } }, // 0,1,2
    {"EXTEND", [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Extend      >(v, "EXTEND"); } }, // 0,1,2
    {"EXISTS", [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Exists      >(v, "EXISTS"); } }, // 1
    {"NEXT"  , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<Next        >(v, "NEXT"  ); } }, // 1
    {"PRIOR" , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<PriorExpr       >(v, "PRIOR" ); } }, // 1
    {""      , [](CollectionType *v) -> Ptr<CollectionMethod> { return createCollectionMethod<AccessToItem>(v, ""      ); } }, // 1
  };
  if (field) {
    MethodCreatorMap::const_iterator it = collectionMethods.find(field->toNormalizedString());
    if (it != collectionMethods.end())
      return (it->second)(variable);
  }
  return 0;
}

}

Sm::Type::CollectionType::CollectionType(CLoc l, Ptr<Id2> _name, Ptr<Id> _oid, Datatype *thisT, Ptr<Datatype> mT)
 : ObjectType(l, _name, _oid),
   thisDatatype(thisT),
   constructor(new ArrayConstructor(this, name->entity(), thisT)),
   mappedType_(mT) {}

Type::CollectionType::~CollectionType() {}

Sm::Function *CollectionType::getDefaultConstructor() const { return constructor.object(); }

bool Object::getFields(EntityFields &fields) const {
  if (supertype)
    supertype->getFields(fields);

  for (BaseList<MemberInterface>::const_iterator it = elements.begin(); it != elements.end(); ++it)
    if ((*it)->cathegoryMember() == Type::MemberInterface::VARIABLE)
      fields.push_back((*it)->getMemberName());
  return true;
}

bool CollectionType::getFields(EntityFields &fields) const {
  EntityFields nestedFields;
  if (!mappedType_)
    return false;

  Ptr<Datatype> mappedType = Datatype::getLastConcreteDatatype(mappedType_);
  if (!mappedType->isObjectType())
    mappedType->getFields(nestedFields);

  if (!nestedFields.size())
    fields.push_back(new Id("COLUMN_VALUE", mappedType_.object()));
  else
    fields.insert(fields.end(), nestedFields.begin(), nestedFields.end());
  return true;
}

Object::Object(CLoc l, Ptr<Id2> name, Ptr<BaseList<MemberInterface> > elem,
         Ptr<Id>         oid         ,
         Ptr<Id2>        superType   ,
         auth_id::AuthID auth        ,
         Ptr<JavaSpec>   extJavaSpec )
    : GrammarBase(l), ObjectType(l, name, oid), head(0), declObj(NULL), supertype(superType), authID(auth), javaExtSpec(extJavaSpec)
{
  if (elem)
    for (Ptr<MemberInterface> & it : *elem)
      if (it) {
        it->owner_ = const_cast<Object*>(this);
        if (pragma::Pragma *p = it->toSelfPragma())
          pragmas.push_back(p);
        else if (it->isConstructorM())
          constructors.push_back(it->toSelfMemberFunction());
        else
          elements.push_back(it);
      }
}

void Type::Object::collectChildsToSNode(SemanticTree *node) const {
  for (List<MemberFunction>::const_iterator it = constructors.begin(); it != constructors.end(); ++it)
    if (*it)
      node->addChild(it->object()->toSTree());

  for (List<MemberInterface>::const_iterator it = elements.begin(); it != elements.end(); ++it)
    if (*it)
      (*it)->collectSNodeM(node);
  CollectSNode(node, pragmas);
}

void Type::Object::addChilds(Object *obj, SemanticTree *node) const {
  if (SemanticTree *n = obj->toSTree()) {
    n->nametype = SemanticTree::EMPTY;
    node->addChild(n);
  }
}

SemanticTree *Type::Object::toSTreeBase() const {
  if (isBody())
    return 0;

  SemanticTree *node = name->toSNodeDef(isAnydata() ? SCathegory::Anydata : SCathegory::ObjectType, this);
  collectChildsToSNode(node);

  if (head && head != this)
    head->collectChildsToSNode(node);
  else if (Object *obj = objectBody.object()) {
    obj->setSemanticNode(node);
    obj->collectChildsToSNode(node);
  }
  return node;
}

void Type::Object::applyDynamicUsing() {
  // Устанавливаем у всех элементов объекта флаг использования в NestedTable коллекциях, участвующих в запросах.
  for (Elements::reference member : elements) {
    member->setIsDynamicUsing();
  }

  for (List<MemberFunction>::reference ctor : constructors) {
    ctor->setIsDynamicUsing();
  }
}

// }


SemanticTree *Varray::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::Varray, this);
  ANODE(mappedType_);
  CTREE(constructor);
  updateThisDatatypeSemanticNode(n);
  return n;
}
SemanticTree *NestedTable::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::NestedTableType, this);
  ANODE(mappedType_)
      ANODE(keyType_)
      CTREE(constructor)
      updateThisDatatypeSemanticNode(n);
  return n;
}

/* SemanticTree *Varray::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::NestedTableType, this);
  ANODE(mappedType_)
  ANODE(keyType_)
  CTREE(constructor)
  updateThisDatatypeSemanticNode(n);
  return n;
} */

void Sm::Type::Object::resolve(ModelContext &model) {
  if (objectBody)
    objectBody->resolve(model);
  if (objectBody && objectBody->supertype)
    resolveObject(model, objectBody->supertype.object());
  if (supertype)
    resolveObject(model, supertype);
  for (List<MemberInterface>::iterator it = elements.begin(); it != elements.end(); ++it) {
    (*it)->setOwnerObject(this);
    (*it)->resolve(name->entity());
  }
  for (List<Type::MemberFunction>::iterator it = constructors.begin(); it != constructors.end(); ++it) {
    (*it)->setOwnerObject(this);
    (*it)->resolve(name->entity());
  }
  for (List<pragma::Pragma>::iterator pit = pragmas.begin(); pit != pragmas.end(); ++pit) {
    (*pit)->setOwnerObject(this);
    (*pit)->resolve(name->entity());
  }
}

bool Record::getFields(EntityFields &fields) const {
  Ptr<ResolvedEntity> ent = static_cast<ResolvedEntity*>((Record*)this);
  if (!isDefinition())
    ent = getDefinitionFirst();
  Ptr<Record> r = ent->toSelfRecord();
  if (r->fields) {
    for (List<RecordField>::iterator it = r->fields->begin(); it != r->fields->end(); ++it)
      fields.push_back((*it)->name);
    return true;
  }
  return false;
}

std::string ObjectType::getTypeString(Sm::Datatype *type) {
  if (!type) {
    cout << "ERROR: type is NULL in getTypeString: object is " << getName()->toNormalizedString();
    return "";
  }

  type = type->getFinalType();
  if (!type || type->isBigint() || type->isInt() || type->isObjectType() || type->isAnydata())
    return "INT";
  else if (type->isDecimal() || type->isNumberDatatype())
    return "NUMBER";
  else if (type->isVarcharDatatype())
    return "VARCHAR";
  else if (type->isDateDatatype())
    return "DATE";
  else if (type->isExactlyEqually(Datatype::mkBoolean()))
    return "BOOL";
  else if (type->isDouble() || type->isExactlyEqually(Datatype::mkFloat()))
    return "DOUBLE";
  else if (type->isExactlyEqually(Datatype::mkBlob())  ||
           type->isExactlyEqually(Datatype::mkClob())  ||
           type->isExactlyEqually(Datatype::mkNClob()) ||
           type->isExactlyEqually(Datatype::mkLongRaw()) )
    return "BLOB";
  else {
    Sm::Codestream str;
    str << "ObjectType::getTypeString has unimplemented for " << s::ref << *type << s::endl;
    cout << str.str();
    return "";
  }
}

std::string ObjectType::generateTabNameWithoutUser(ResolvedEntity *inEntity, UserContext **userOwner) {
  std::string tabName;
  std::list<Ptr<Id> > names;

  if (toSelfObject())
    inEntity = this;

  UserContext *ucntx = 0;
  for (ResolvedEntity *ent = inEntity; ent && !(ucntx = ent->toSelfUserContext()); ent = ent->owner())
    if (Ptr<Id> n = ent->getName())
      names.push_front(n);

  if (ucntx && userOwner)
    *userOwner = ucntx;

  bool isNotFirst = false;
  for (std::list<Ptr<Id> >::iterator it = names.begin(); it != names.end(); ++it) {
    if (isNotFirst)
      tabName.push_back('_');
    isNotFirst = true;
    tabName += (*it)->toNormalizedString();
  }

  if (addTypeName_ && !toSelfObject())
    tabName += "_" + name->entity()->toNormalizedString();
  return tabName;
}

std::string ObjectType::generateTabName(ResolvedEntity *inEntity) {
  UserContext *ucntx = 0;
  std::string tab    = generateTabNameWithoutUser(inEntity, &ucntx);
  if (ucntx)
    return ucntx->getName()->toNormalizedString() + "." + tab;
  else
    return tab;
}

std::string ObjectType::getObjectTempTable(Sm::ResolvedEntity *def) {
  switch (def->ddlCathegory()) {
  case ResolvedEntity::RefExpr_:
    return getObjectTempTable(def->toSelfRefExpr()->refDefinition());
  case ResolvedEntity::Variable_:
  case ResolvedEntity::MemberVariable_:
//  case ResolvedEntity::FunctionArgument_:
    return generateTabName(def);
  case ResolvedEntity::Function_:
  case ResolvedEntity::MemberFunction_: {
    Function *func = def->toSelfFunction();
    if (func->isPipelined()) {
      return getObjectTempTable(func->pipeVar.object());
    }
    else if (Ptr<Sm::BlockPlSql> block = func->funBody()) {
      if (Sm::RefAbstract *exprId = block->findLastReturnExpression())
        return getObjectTempTable(exprId->refDefinition());
    }
  } break;
  case ResolvedEntity::Object_:
    return def->toSelfObject()->generateTabName(def);
  default:
    break;
  }

  cout << "ERROR: Can't extract a table name from " << def->ddlCathegoryToString() << " " << def->getName()->toNormalizedString() << endl;
  return "???";
}

void ObjectType::extractNestedFields(Sm::Codestream &str,
                                     ResolvedEntity *obj,
                                     const std::string &prevField/* = std::string()*/) {
  EntityFields fields;
  obj->getFields(fields);
  for (auto it = fields.begin(); it != fields.end(); ++it) {
    ResolvedEntity *ent = (*it)->definition();
    Ptr<Sm::Datatype> datatype = ent->getDatatype();
    if ((*it)->toNormalizedString() == "COLUMN_VALUE" && prevField.empty()) {
      str << "VAL__" << s::name;
      datatype->getFinalType()->sqlDefinition(str);
    }
    else if (datatype) {
      if (datatype->ddlCathegory() == ResolvedEntity::Record_ || datatype->isRowTypeOf()) {
        extractNestedFields(str, datatype, prevField + (*it)->toNormalizedString() + "$");
      }
      else {
        string fld = prevField + ent->getName()->toNormalizedString();
        if (ent->getName()->hasSpecSymbols() || prevField.size() || Id::isFieldReserved(fld)) {
          string tmpField;
          PlsqlHelper::quotingAndEscaping(tmpField, fld, '\"');
          fld = tmpField;
        }
        str << fld << s::name;
        datatype->getFinalType()->sqlDefinition(str);
      }
    }
    if (it + 1 != fields.end())
      str << s::comma();
  }
}

void ObjectType::translateVariableType(Sm::Codestream &str, Sm::ResolvedEntity* var, bool addTypeName) {
  if (!syntaxerContext.model->modelActions.createGlobalVars())
    return;
  if (isOracleNontranslatedDatatype())
    return;
  if (!var->isDynamicUsing())
    return;
  addTypeName_ = addTypeName;

  UserContext *ucntx = 0;

  ResolvedEntity *ent = var;
  if (toSelfObject())
    ent = this;

  std::string tblName = generateTabNameWithoutUser(ent, &ucntx);
  std::string uName;
  Ptr<Id> usrRef;
  if (ucntx) {
    uName = ucntx->getName()->toNormalizedString();
    usrRef = new Id(*(ucntx->getName()));
  }

  string genName = uName.empty() ? tblName : uName + "." + tblName;

  if (s_translatedObjMap.find(genName) != s_translatedObjMap.end())
    return;

  Ptr<Table> tbl = new Sm::Table(-1, new Id2(new Id(string(tblName)), usrRef));
  tbl->translatedName(genName);

  syntaxerContext.model->granteeGlobalTemporaryObjectTables.push_back(tbl);
  CodestreamState::ProcedureMode oldMode = str.procMode();
  str.procMode(CodestreamState::SQL);

  if (var->getLLoc().beginedFrom(108263))
    cout << "";
  s_translatedObjMap[genName] = Ptr<ObjectType>(this);
  Codestream headStr(str.state());
  headStr << "GLOBAL TEMPORARY TABLE" << s::name << genName;

  str << s::ocmd(this, CmdCat::TABLE_FOR_OBJECT, headStr.str())
      << syntaxerContext.createStatement << "GLOBAL TEMPORARY TABLE" << s::name << genName << s::name
      << s::obracket;
  {
    str.incIndentingLevel(2);
    translateSpecificFields(str);
    extractNestedFields(str, this);
    str.decIndentingLevel(2);
  }
  str << s::cbracket << " ON COMMIT PRESERVE ROWS" << s::semicolon
      << s::OMultiLineComment() << s::linref(this) << s::loc(var->getLLoc()) << s::CMultiLineComment()
      << s::endl
      << s::ccmd;

  translateIndexes(str, genName);
  str << s::endl;
  str << s::grant(tbl.object(), {Privs::SELECT, Privs::INSERT, Privs::DELETE, Privs::UPDATE}, syntaxerContext.model->modelActions.scenarioActorUsers);

  //Translate included object types
  EntityFields fields;
  getFields(fields);
  for (auto it = fields.begin(); it != fields.end(); ++it ) {
    Ptr<Datatype> datatype = (*it)->getDatatype();
    if (datatype && datatype->isObjectType()) {
      str << s::endl;
      datatype->translateVariableType(str, ent, true);
    }
  }

  str.procMode(oldMode);

  addTypeName_ = false;
}

void ObjectType::translateAssign(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  if (!refInfo->idRef || !expr)
    throw 999;

  ResolvedEntity *objRef = refInfo->idRef->definition();
  ResolvedEntity *exprDef = expr->getNextDefinition();

  s_currentConstructedVar = refInfo->member ? refInfo->member : refInfo->idRef;
  if (objRef && objRef->isCollectionAccessor()) {
    //Доступ к полю
    setAccessor(str, refInfo, expr);
  }
  else if (!objRef->isMemberVariable() &&
           refInfo->idRef->toNormalizedString() != "SELF" &&
           exprDef && exprDef->isConstructor()) {
    //Constructor
    g_callConstructor = true;
    str << "CALL " << Ptr<Sm::PlExpr>(expr);
    g_callConstructor = false;
  }
  else {
    //Доступ к полю объекта
    setAccessor(str, refInfo, expr);
  }
  s_currentConstructedVar = NULL;
}

std::string RefInfo::toDebugString() const {
  stringstream str;
  str << idRef->definition()->getName()->toNormalizedString() << ":" << idRef->definition()->getLLoc();
  return str.str();
}

void ObjectType::translateObjRef(Sm::Codestream &str, RefInfo *refInfo) {
  ResolvedEntity *def = refInfo->idRef->definition();
  if (!def) {
    translateRefInfoNextOrIdRef(str, refInfo);
    return;
  }

  if (def->isConstructor()) {
    if (!s_currentConstructedVar && isCollectionType()) {
      cout << "Call constructor of " << refInfo->idRef->toNormalizedString() <<  " in not variable context" << std::endl;
      str << refInfo->idRef->toNormalizedString() << "???";
      return;
    }
    callCtor(str, s_currentConstructedVar, refInfo->idRef);
  }
  else if (!def->isCollectionAccessor() && def->isMethod()) {
    static int recurseCount = 0;
    if (++recurseCount > 200) {
      cout << "Recurse error in ObjectType::translateObjRef" << endl;
      return;
    }
    Ptr<Sm::Id> mfCall = refInfo->idRef;
    Ptr<Sm::CallArgList> argList = mfCall->callArglist;
    if (!argList) {
      argList = new Sm::CallArgList();
      mfCall->callArglist = argList;
    }

    Codestream funcStr;
    funcStr.procMode(CodestreamState::ProcedureMode::PROC);
    bool needPushSelf = true;
    if (refInfo->next) {
        refInfo->next->translateObjRef(funcStr);

      if (refInfo->next->fieldType && refInfo->next->fieldType->isAnydata())
        cout << "";
    }
    else
      trError(str, s << "ERROR: refInfo->next is NULL: "  << refInfo->toDebugString());

    if (refInfo->idRef) {
      if (refInfo->idRef->selfAlreadyPushed())
        needPushSelf = false;
      else
        refInfo->idRef->setSelfAlreadyPushed();
    }

    if (needPushSelf) {
      Ptr<Sm::RefExpr> expr = new Sm::RefExpr(mfCall->getLLoc(), new Id(funcStr.str()));
      Ptr<Sm::FunCallArg> arg = new Sm::FunCallArgExpr(mfCall->getLLoc(), expr.object());
      arg->setSelf();
      expr->reference->setIsDynamicUsing();
      argList->insert(argList->begin(), arg);
    }

    ResolvedEntity::ScopedEntities prevCat = mfCall->definition()->ddlCathegory();
    bool userAlreadyOutput = false;
    bool isNotFirst = false;
    translateIdReference(str, mfCall, isNotFirst, prevCat, userAlreadyOutput);

    --recurseCount;
  }
  else if (def->isCollectionAccessor() || !refInfo->fieldName.empty()) {
    getAccessor(str, refInfo);
  }
  else
    translateOneId(str, refInfo->idRef);
}


bool RefInfo::updateObjectTypeReference(Sm::Codestream &str)
{
  if (!objectType) {
    if (ResolvedEntity *d = fieldType->tidDef())
      objectType = d; // ? d->toSelfObject() ? - но кроме объектов есть еще и Records
    else {
      trError(str, s << "ERROR: RefInfo::translateObjRef objectType is NULL, fieldType->tid is unresolved: " << this->toDebugString());
      return false;
    }
  }
  return true;
}

void Sm::Type::RefInfo::translateObjRef(Sm::Codestream &str) {
  if (updateObjectTypeReference(str))
    objectType->translateObjRef(str, this);
}

void Sm::Type::RefInfo::translateAssign(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  if (updateObjectTypeReference(str))
    objectType->translateAssign(str, refInfo, expr);
}


void ObjectType::translateRefInfoNextOrIdRef(Sm::Codestream &str, RefInfo *refInfo)
{
  if (refInfo->next)
    refInfo->next->translateObjRef(str);
  else
    translateOneId(str, refInfo->idRef);
}

void ObjectType::baseAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  if (expr)
    str << "CALL " << getMethodName(Type::ObjectType::BM_SET, 0, refInfo->fieldType);
  else
    str << getMethodName(Type::ObjectType::BM_GET, 0, refInfo->fieldType);

}

void Object::setAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  baseAccessor(str, refInfo, expr);
  str << s::obracket;
  translateRefInfoNextOrIdRef(str, refInfo);
  str << s::comma() << "\"" << refInfo->fieldName << "\"" <<  s::comma();
  expr->translate(str);
  str << s::cbracket;
}

void Object::getAccessor(Sm::Codestream &str, RefInfo *refInfo) {
  baseAccessor(str, refInfo, NULL);
  str << s::obracket;
  translateRefInfoNextOrIdRef(str, refInfo);
  str << s::comma() << "\"" << refInfo->fieldName << "\"" <<  s::cbracket;
}

void CollectionType::setAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) {
  if (!refInfo->idRef->definition()->isCollectionAccessor()) {
    str << "CALL " << getMethodName(BM_COPY_ALL, 0, NULL) << s::obracket;
    translateRefInfoNextOrIdRef(str, refInfo);
    str << s::comma();
    expr->translate(str);
    str << s::comma();
    translateQueryFieldsEscape(str, this, true);
    str << s::cbracket;
  }
  else {
    baseAccessor(str, refInfo, expr);
    str << s::obracket;
    translateRefInfoNextOrIdRef(str, refInfo);
    str << s::comma() << *(refInfo->idRef->callArglist->begin()) << s::comma();
    str << "\"" << refInfo->fieldName << "\"" <<  s::comma();
    expr->translate(str);
    str << s::cbracket;
  }
}

void CollectionType::getAccessor(Sm::Codestream &str, RefInfo *refInfo) {
  baseAccessor(str, refInfo, NULL);
  str << s::obracket;
  translateRefInfoNextOrIdRef(str, refInfo);
  if (refInfo->idRef->callArglist) {
    str << s::comma() << *(refInfo->idRef->callArglist->begin()) << s::comma();
    str << "\"" << refInfo->fieldName << "\"";
  }
  else
    trError(str, s << "ERROR: refInfo->idRef->callArglist is NULL: " << refInfo->toDebugString());
  str << s::cbracket;
}



void Object::callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor) {
  char quote = str.isProc() ? '"' : '\'';
  std::string tabName = generateTabName(idRef ? idRef->definition() : NULL);
  str << s::cref(idConstructor->definition());
  str << s::obracket;
  str << getMethodName(Type::ObjectType::BM_CTOR, 0) << s::obracket << quote << tabName << quote << s::cbracket;
  if (idConstructor->callArglist) {
    str << s::comma() << idConstructor->callArglist;
  }
  str << s::cbracket;
  if (g_callConstructor && !g_callSubconstruct && idRef)
    str << " INTO " << s::linref(idRef->definition());
}


std::string Object::getMethodName(BuiltinMethod mtype, int /*argsize*/, Sm::Datatype * dtype) {
  std::string mname;

  switch (mtype) {
  case BM_CTOR:     mname = "SYS.OBJ_CONSTRUCT"; break;
  case BM_GET:      mname = "SYS.OBJ_GET_" + getTypeString(dtype); break;
  case BM_SET:      mname = "SYS.OBJ_SET_" + getTypeString(dtype); break;
  case BM_COPY_ALL: mname = "SYS.OBJ_COPY_ALL"; break;
  default:
    throw 999;
  }
  return mname;
}

void Object::translateSpecificFields(Sm::Codestream &str) {
  str << "ID__ INT PRIMARY KEY AUTOINC" << s::comma()
      << "DELETED__ BOOLEAN NOT NULL" << s::comma()
      << "OBJREFID__ BIGINT" << s::comma();
}

void Object::translateIndexes(Sm::Codestream &str, const std::string &tableName) {
  str << s::ocmd(this, CmdCat::INDEX_FOR_OBJECT)
      << syntaxerContext.createStatement << "INDEX SPECIDX1 ON " << tableName << "(ID__, OBJREFID__)" << s::semicolon
      << s::ccmd;
}

// TODO: вероятно нужно создавать различные конструкторы в зависимости от списка аргументов name->callArglist
//Function *Type::Object::getDefaultConstructor() const
//{
//  if (defaultConstructor.object() == 0) {
//    Id *uname   = getName2()->uname () ? new Id(*(getName2()->uname ())) : (Id*)0;
//    Id *entName = getName2()->entity() ? new Id(*(getName2()->entity())) : (Id*)0;
//    defaultConstructor = new Type::MemberFunction(getLLoc(),
//                                                  Type::Inheritance::EMPTY,
//                                                  Type::MemberFunction::CONSTRUCTOR,
//                                                  new Id2(entName, uname), 0, new Datatype(this->getName()));
//    defaultConstructor->owner_ = this;
//    SemanticTree *constructorSTree = defaultConstructor->toSTree();
//    SemanticTree *modelNode =  getSemanticNode();
//    modelNode->addChild(constructorSTree);
//    collectEqualsDeclarationOnNode(constructorSTree, modelNode->childNamespace);
//    constructors.push_back(defaultConstructor);
//  }
//  return defaultConstructor.object();
//}

Sm::Function *Object::getDefaultConstructor() const {
  for (auto it = constructors.begin(); it != constructors.end(); ++it) {
    if ((*it)->allArgsIsDefault())
      return (Function*)((*it).object());
  }

  //Если конструктор не задан, создадим его
  Ptr<Datatype> selfDatatype = new Datatype(name);
  Ptr<FunArgList> argList = new FunArgList;
  // Занесем в аргументы все поля со значением по умолчанию.

  const Object *selfDecl = isBody() ? declObj : this;

  for (BaseList<MemberInterface>::const_iterator it = selfDecl->elements.begin(); it != selfDecl->elements.end(); ++it) {
    if ((*it)->cathegoryMember() != MemberInterface::VARIABLE)
      continue;
    ResolvedEntity *memberVar = (*it)->getThisDefinition();
    Ptr<Datatype> datatype = memberVar->getDatatype()->getFinalType();
    if (datatype->isObjectType())
      continue;
    Ptr<Id> argId = new Id(memberVar->getName()->toNormalizedString() + "_", NULL);
    argList->push_back(new FunctionArgument(argId, datatype, new NullExpr()));
  }

  Ptr<MemberFunction> defaultConstructor = new MemberFunction(getLLoc(), Inheritance::EMPTY, Type::member_function::CONSTRUCTOR,
                                                              new Id2("DEFAULT_CTOR__"), argList, selfDatatype,
                                                              new BlockPlSql());
  defaultConstructor->owner_ = (Object*)this;
  SemanticTree *constructorSTree = defaultConstructor->toSTree();
  SemanticTree *modelNode =  getSemanticNode();
  modelNode->addChild(constructorSTree);
  collectEqualsDeclarationOnNode(constructorSTree, modelNode->childNamespace);

  constructors.push_front(defaultConstructor);
  return Ptr<Function>(defaultConstructor.object());
}

void RecordField::oracleDefinition(Sm::Codestream &str) {
  str << s::name << name << s::name << s::def << datatype;
  if (defaultValue)
    str << " = " << s::def << defaultValue;
}

void RecordField::linterDefinition(Sm::Codestream &str) {
  str << name << s::name << s::def << datatype;
  if (defaultValue) {
    Ptr<Datatype> dTypeExpr =  defaultValue->getDatatype()->getConcreteDatatype()->getDatatype();
    if (!dTypeExpr || !dTypeExpr->isNull())
      throw 999;
  }
}

void RecordField::linterReference(Sm::Codestream &str) {
  if (!procTranslatedName().empty())
    str << procTranslatedName();
  else
    str << name;
}

void CollectionType::procCtorArgs(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor) {
  Ptr<CallArgList> argList = idConstructor->callArglist;
  if (!argList || argList->size() == 0)
    return;

  if (getElementDatatype()->isRecordType())
    throw 999; //TODO: конструктор для структур не поддерживается

  str << s::procendl(idConstructor->getLLoc());
  size_t i = 1;
  if (needAddExtend()) {
    str << s::tab() << "CALL " << getMethodName(BM_EXTEND, 1, NULL);
    str << s::obracket;
    translateOneId(str, idRef);
    str << s::comma() << argList->size() << s::cbracket << s::procendl(idConstructor->getLLoc());
  }
  g_callSubconstruct = true;
  for (CallArgList::iterator it = argList->begin(); it != argList->end(); ++i, ++it) {
    str << s::tab() << "CALL " << getMethodName(Type::ObjectType::BM_SET, 0, getElementDatatype()) ;
    str << s::obracket;
    translateOneId(str, idRef);
    str << s::comma() << i << s::comma() << "\"\"" <<  s::comma();
    (*it)->translate(str);
    str << s::cbracket;
    if (i != argList->size())
      str << s::procendl(idConstructor->getLLoc());
  }
  g_callSubconstruct = false;
}

void NestedTable::oracleDefinition(Sm::Codestream &str) {
  str << "TYPE " << name << " TABLE OF " << s::def << mappedType_;

  if (isNotNull)
    str << " NOT NULL ";
  if (keyType_)
    str << " INDEX BY " << s::def << keyType_;
}

void NestedTable::callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef,  Ptr<Sm::Id> idConstructor) {
  std::string tabName = generateTabName(idRef->definition());
  str << getMethodName(Type::ObjectType::BM_CTOR, 0);
  str << s::obracket << "\"" << tabName << "\"";
  if (idRef->definition()->isMemberVariable())
    str << s::comma() << s::name << "self";
  str << s::cbracket;
  if (g_callConstructor && !g_callSubconstruct && idRef)
    str << " INTO " << s::linref(idRef->definition());
  procCtorArgs(str, idRef, idConstructor);
}


collection_methods::AccessToItem* CollectionType::addAccesingMethod(ResolvedEntity* def) {
  Ptr<collection_methods::AccessToItem> d = new collection_methods::AccessToItem(def);
  accesingInExpressions.push_back(d);
  return d;
}

void NestedTable::translateSpecificFields(Sm::Codestream &str) {
  if (isAssocArray())
    str << "ID__ INT PRIMARY KEY AUTOINC" << s::comma()
        << "KEY__ " << keyType_->getFinalType() << " NOT NULL" << s::comma();
  else
    str << "ID__ INT NOT NULL" << s::comma()
        << "DELETED__ BOOLEAN NOT NULL" << s::comma();
  str << "OBJREFID__ BIGINT" << s::comma();
}

void NestedTable::translateIndexes(Sm::Codestream &str, const std::string &tableName) {
  str << s::ocmd(this, CmdCat::INDEX_FOR_OBJECT) << syntaxerContext.createStatement << "UNIQUE INDEX SPECIDX1 ON " << tableName
      << s::obracket  << (isAssocArray() ? "KEY__" : "ID__") << s::comma() << "OBJREFID__" << s::cbracket
      << s::semicolon << s::ccmd;
}

std::string NestedTable::getMethodName(BuiltinMethod mtype, int argsize, Sm::Datatype * dtype) {
  std::string mname;
  std::string prefix = isAssocArray() ? "SYS.AA_" : (mtype == BM_GET || mtype == BM_SET) ? "SYS.CI_" : "SYS.NT_";
  std::string keytype;
  if (isAssocArray())
    keytype =  keyType()->getFinalType()->isVarcharDatatype() ? "_VCKEY" : "_IKEY";

  switch (mtype) {
  case BM_CTOR:   mname = "CONSTRUCT"; break;
  case BM_GET:    mname = "GET" + keytype + "_" + getTypeString(dtype); break;
  case BM_SET:    mname = "SET" + keytype + "_" + getTypeString(dtype); break;
  case BM_DELETE: mname = (argsize >= 2) ? "DELETEMN" + keytype : ((argsize == 1) ? "DELETEN" +keytype : "DELETE"); break;
  case BM_TRIM:   mname = (argsize >= 1) ? "TRIMN" : "TRIM"; break;
  case BM_EXTEND: mname = ((argsize >= 2) ? "EXTENDNI" : ((argsize == 1) ? "EXTENDN" : "EXTEND")) + keytype; break;
  case BM_EXISTS: mname = "EXISTS" + keytype; break;
  case BM_FIRST:  mname = "FIRST" + keytype; break;
  case BM_LAST:   mname = "LAST" + keytype; break;
  case BM_COUNT:  mname = "COUNT"; break;
  case BM_LIMIT:  mname = "LIMIT"; break;
  case BM_PRIOR:  mname = "PRIOR" + keytype; break;
  case BM_NEXT:   mname = "NEXT" + keytype; break;
  case BM_COPY_SQL:   mname = "COPY_SQL" + keytype; break;
  case BM_COPY_ALL:   mname = "COPY_ALL"; break;
  case BM_COPY_IDX:   mname = (isAssocArray()) ? "COPY" + keytype : "COPY_IDX"; break;
  case BM_INSERT_SQL: mname = "INSERT_SQL" + keytype; break;
  default:
    throw 999;
  }
  mname = prefix + mname;
  return mname;
}

std::string Varray::getMethodName(BuiltinMethod mtype, int argsize, Datatype * dtype) {
  std::string mname;

  switch (mtype) {
  case BM_CTOR:   mname = "VA_CONSTRUCT"; break;
  case BM_GET:    mname = "CI_GET_" + getTypeString(dtype); break;
  case BM_SET:    mname = "CI_SET_" + getTypeString(dtype); break;
  case BM_DELETE: mname = "VA_DELETE"; break;
  case BM_TRIM:   mname = (argsize == 1) ? "VA_TRIMN" : "VA_TRIM"; break;
  case BM_EXTEND: mname = (argsize == 2) ? "VA_EXTENDNI" : ((argsize == 1) ? "VA_EXTENDN" : "VA_EXTEND"); break;
  case BM_EXISTS: mname = "VA_EXISTS"; break;
  case BM_FIRST:  mname = "FIRST"; break;
  case BM_LAST:   mname = "LAST"; break;
  case BM_COUNT:  mname = "VA_COUNT"; break;
  case BM_LIMIT:  mname = "VA_LIMIT"; break;
  case BM_PRIOR:  mname = "VA_PRIOR"; break;
  case BM_NEXT:   mname = "VA_NEXT"; break;
  case BM_COPY_SQL:   mname = "VA_COPY_SQL"; break;
  case BM_COPY_ALL:   mname = "VA_COPY_ALL"; break;
  case BM_COPY_IDX:   mname = "VA_COPY_IDX"; break;
  case BM_INSERT_SQL: mname = "VA_INSERT_SQL"; break;
  default:
    throw 999;
  }

  mname = "SYS." + mname;
  return mname;
}

void Varray::callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor) {
  std::string tabName = generateTabName(idRef->definition());
  str << getMethodName(Type::ObjectType::BM_CTOR, 0);
  str << s::obracket << "\"" << tabName << "\"" << s::comma() << sizeLimit;
  if (idRef->definition()->isMemberVariable())
    str << s::comma() << s::name << "self";
  str << s::cbracket;
  if (g_callConstructor && !g_callSubconstruct && idRef)
    str << " INTO " << s::linref(idRef->definition());
  procCtorArgs(str, idRef, idConstructor);
}

void Varray::translateSpecificFields(Sm::Codestream &str) {
  str << "ID__ INT NOT NULL"          << s::comma()
      << "DELETED__ BOOLEAN NOT NULL" << s::comma()
      << "OBJREFID__ BIGINT"          << s::comma();
}

void Varray::translateIndexes(Sm::Codestream &str, const std::string &tableName) {
  str << s::ocmd(this, CmdCat::INDEX_FOR_OBJECT)
      << syntaxerContext.createStatement << "UNIQUE INDEX SPECIDX1 ON " << tableName << "(ID__, OBJREFID__)" << s::semicolon
      << s::ccmd;
}

void Varray::linterDefinition (Sm::Codestream &str) {
  if (!isDynamicUsing() && !str.state().isDynamicCollection_) {
    str << "ARRAY [" << keyType() << "]" << s::name << getElementDatatype();
  }
  else
    ObjectType::linterDefinition(str);
}

void NestedTable::linterDefinition (Sm::Codestream &str) {
  if (!isDynamicUsing() && !str.state().isDynamicCollection_) {
    str << "ARRAY [" << keyType() << "]" << s::name << getElementDatatype();
  }
  else
    ObjectType::linterDefinition(str);
}

void Object::linterDefinition(Sm::Codestream &str) {
  if (isXmlType())
    str << "XMLTYPE";
  else if (isAnydata() || isAnytype())
    str << "BIGINT";
  else if (!isDynamicUsing())
    translateAsLinterStruct(getDeclaration() ? getDeclaration() : this, str, false);
  else
    ObjectType::linterDefinition(str);
}

namespace collection_methods {

Ptr<Datatype> CollectionMethod::getElementDatatype() const {
  if (collectionDatatype_) {
      Ptr<Datatype> datatype = ResolvedEntity::getLastConcreteDatatype(collectionDatatype_->getDatatype());
      if (datatype && datatype->isObjectType())
        return datatype->tidDef()->mappedType();
      else
        return collectionDatatype_->getDatatype();
  }
  return 0;
}
Ptr<Datatype> CollectionMethod::getKeyDatatype() const {
  if (collectionDatatype_)
    return getCollection()->keyType();
  return Datatype::mkInteger();
}

Sm::Type::CollectionType* CollectionMethod::getCollection() const {
  if (collectionDatatype_->ddlCathegory() == ResolvedEntity::Variable_ ||
      collectionDatatype_->ddlCathegory() == ResolvedEntity::FunctionArgument_ ||
      collectionDatatype_->ddlCathegory() == ResolvedEntity::MemberVariable_ ||
      collectionDatatype_->ddlCathegory() == ResolvedEntity::FieldOfRecord_)
  {
    Ptr<Datatype> dt = collectionDatatype_->getDatatype()->getFinalType();
    if (ResolvedEntity *next = dt->getNextDefinition())
      return next->toSelfCollectionType();
    else {
      cout << "error: unresolved collection datatype " << dt->getLLoc() << endl;
      return 0;
    }
  }
  if (collectionDatatype_->isCollectionAccessor()) {
    Ptr<Datatype> dt = collectionDatatype_->getDatatype()->getFinalType();
    return dt->getNextDefinition()->toSelfCollectionType();
  }
  if (collectionDatatype_->ddlCathegory() == ResolvedEntity::Datatype_)
    return collectionDatatype_->getNextDefinition()->toSelfCollectionType();
  return collectionDatatype_->toSelfCollectionType();
}

inline bool linterArrayUsing(const CollectionMethod *cm, Ptr<Sm::CallArgList> callArgList) {
  if (syntaxerContext.translateReferences) {
    if (!cm->collectionDatatype()->isDynamicUsing()) {
      RefAbstract *expr = (callArgList && !callArgList->empty()) ? callArgList->front()->expr()->toSelfRefAbstract() : NULL;
      if (!expr || !expr->reference->isDynamicUsing())
        return true;
    }
  }
  return false;
}

bool AccessToItem::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> /*id*/, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, NULL))
    str << "[" << callArgList << "]";
  else
    throw 999; //Не должны сюда прийти, должны правильно обработать через setAccessor, getAccessor
  return true;
}

bool Count::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  if (id->beginedFrom(985595))
    cout << "";
  if (linterArrayUsing(this, callArgList))
    str << "ARRAY_COUNT" << s::obracket << callArgList << s::cbracket;
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_COUNT, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool Exists::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (index)
    throw 999;
  if (linterArrayUsing(this, callArgList)) {
    str << "ARRAY_EXISTS" << s::obracket << callArgList << s::cbracket;
  }
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_EXISTS, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

void CollectionMethod::translateLoopHead(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::FunCallArg> N, Ptr<Variable> &tmpVar) {
  string loopVar;
  tmpVar = id->semanticNode()->unnamedDdlEntity->getTemporaryVar(str, Datatype::mkInteger(), &loopVar);
  if (!tmpVar)
    throw 999;
  str << "FOR " << loopVar << " := 1 WHILE " << loopVar << " <= " << N
      << " BY " << loopVar << " := " << loopVar << " + 1 LOOP" << s::endl;
}

void Extend::translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  CallArgList::iterator it = callArgList->begin();
  Ptr<FunCallArg> colArg = *it++;
  Ptr<FunCallArg> N = *it++;
  Ptr<Variable> tmpVar;

  str << s::Comment() << " Collection EXTEND(" << callArgList << ")" << s::endl << s::tab();
  translateLoopHead(str, id, N, tmpVar);
  str.incIndentingLevel(2);
  str << s::tab() << colArg << "[ARRAY_COUNT(" << colArg << ") + 1] := ";
  if (callArgList->size() > 2)
    str << (*it) << s::procendl();
  else
    str << "NULL" << s::procendl();
  str.decIndentingLevel(2);
  str << s::tab() << "ENDLOOP";
}

bool Extend::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  if (indices.size())
    throw 999;
  if (linterArrayUsing(this, callArgList)) {
    switch (callArgList->size()) {
    case 1:
      str << callArgList << "[ARRAY_COUNT(" << callArgList << ") + 1] := NULL";
      break;
    case 2:
    case 3:
      translateAsLoop(str, id, callArgList);
      break;
    default:
      throw 999;
    }
  }
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_EXTEND, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

void Delete::translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  CallArgList::iterator it = callArgList->begin();
  Ptr<FunCallArg> colArg = *it;
  Ptr<FunCallArg> from = *(++it);
  Ptr<FunCallArg> into = *(++it);
  Ptr<Variable> tmpVar;

  str << s::Comment() << " Collection DELETE(" << callArgList << ")" << s::endl << s::tab();

  string loopVar;
  tmpVar = id->semanticNode()->unnamedDdlEntity->getTemporaryVar(str, Datatype::mkInteger(), &loopVar);
  if (!tmpVar)
    throw 999;
  str << "FOR " << loopVar << " := " << from << s::name << "WHILE " << loopVar << " <= " << into
      << s::name << "BY " << loopVar << " := " << loopVar << " + 1 LOOP" << s::endl;

  str.incIndentingLevel(2);
  str << s::tab() << "ARRAY_DELETE" << s::obracket << colArg << s::comma() << loopVar << s::cbracket << s::procendl(getLLoc());
  str.decIndentingLevel(2);
  str << s::tab() << "ENDLOOP";
}

bool Delete::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  if (id->beginedFrom(985595))
    cout << "";
  if (linterArrayUsing(this, callArgList)) {
    string name;
    switch (callArgList->size()) {
    case 1: case 2: name = "ARRAY_DELETE"; break;
    case 3 :
      translateAsLoop(str, id, callArgList);
      return true;
    default:
      throw 999;
    }
    str << name << s::obracket << callArgList << s::cbracket;
  }
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_DELETE, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool Last::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList))
    str << "ARRAY_LAST" << s::obracket << callArgList << s::cbracket;
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_LAST, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool First::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList))
    str << "ARRAY_FIRST" << s::obracket << callArgList << s::cbracket;
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_FIRST, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool Next::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList))
    str << "ARRAY_NEXT" << s::obracket << callArgList << s::cbracket;
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_NEXT, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool PriorExpr::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList))
    str << "ARRAY_PREV" << s::obracket << callArgList << s::cbracket;
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_PRIOR, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

bool Limit::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id>, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList)) {
    throw 999; //TODO:
    str << static_cast<Varray*>(getCollection())->sizeLimit;
  }
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_LIMIT, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

void Trim::translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  CallArgList::iterator it = callArgList->begin();
  Ptr<FunCallArg> colArg = *it++;
  Ptr<FunCallArg> N = *it++;
  Ptr<Variable> tmpVar;

  str << s::Comment() << " Collection TRIM(" << callArgList << ")" << s::endl << s::tab();
  translateLoopHead(str, id, N, tmpVar);
  str.incIndentingLevel(2);
  str << s::tab() << "IF ARRAY_LAST(" << colArg << ") <> NULL THEN" << s::endl;
  str << s::tab(4) << "ARRAY_DELETE" << s::obracket << colArg << s::comma()
      << "ARRAY_LAST" << s::obracket << colArg << s::cbracket << s::cbracket << s::procendl();
  str << s::tab() << "ENDIF" << s::endl;
  str.decIndentingLevel(2);
  str << s::tab() << "ENDLOOP";
  //tmpVar->unuse();
}

bool Trim::interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList) {
  if (linterArrayUsing(this, callArgList)) {
    string name;
    switch (callArgList->size()) {
    case 1:
      str << "ARRAY_DELETE" << s::obracket << callArgList << s::comma()
          << "ARRAY_LAST" << s::obracket << callArgList << s::cbracket << s::cbracket;
      break;
    case 2:
      translateAsLoop(str, id, callArgList);
      break;
    default:
      throw 999;
    }
  }
  else {
    str << getCollection()->getMethodName(Type::ObjectType::BM_TRIM, callArgList->size() - 1);
    str << s::obracket << callArgList << s::cbracket;
  }
  return true;
}

// Formal translations

#define MASK_ARG_ALL    (0xffffffff)
#define MASK_ARG(num)   (1 << (num))
#define MASK_ARG0       MASK_ARG(0)
#define MASK_ARG1       MASK_ARG(1)
#define MASK_ARG2       MASK_ARG(2)
#define MASK_ARG3       MASK_ARG(3)

bool CollectionMethod::castArgIfNeed(Ptr<Sm::FunCallArg> arg, Ptr<Datatype> needType) {
  bool nonPl = arg->expr()->getSemanticNode()->isNotPlContext();
  CastCathegory cat = needType->getCastCathegory(arg->getDatatype(), true, nonPl);
  if (cat.explicitInReturn()) {
    cat.setProcCastState();
    cat.setCastReturn();
    Ptr<PlExpr> newExpr = CommonDatatypeCast::cast(arg->expr(), arg->getDatatype().object(), needType.object(), cat);
    if (newExpr && newExpr != arg->expr()) {
      arg->setExpr(newExpr);
      return true;
    }
  }
  return false;
}

void CollectionMethod::translateArgsType(Ptr<CallArgList> argList, unsigned int keyMask) {
  if (!argList || !argList->size())
    return;

  CollectionType *owner = getCollection();
  if (!owner) {
    cout << "error: unresolved collection ";
    if (collectionDatatype_)
      cout << collectionDatatype_->getLLoc();
    cout << endl;
    return;
  }

  Ptr<Datatype> keyType = owner->keyType();

  for (size_t i = 0; i < argList->size(); ++i) {
    if (keyMask & MASK_ARG(i))
      castArgIfNeed((*argList)[i], keyType);
  }
}

void AccessToItem::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG_ALL);
}

void Exists::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG_ALL);
}

void Delete::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG_ALL);
}

void Extend::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG0);
}

void Next::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG_ALL);
}

void PriorExpr::formalTranslateArgs(Ptr<Sm::Id> call) {
  translateArgsType(call->callArglist, MASK_ARG_ALL);
}

} // namespace collection_methods

} // namespace Type

} //namespace Sm

