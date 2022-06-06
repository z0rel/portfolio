#include "smartptr.h"
#include "semantic_collection.h"
#include "semantic_function.h"
#include "syntaxer_context.h"

using namespace Sm;


Type::collection_methods::CollectionMethod::CollectionMethod(ResolvedEntity *_collectionVariable)
  : collectionDatatype_(_collectionVariable) { clrBracesOutput(); }

UserContext *Type::collection_methods::CollectionMethod::userContext() const {
  if (collectionDatatype_)
    return collectionDatatype_->userContext();
  if (collectionVariableRef)
    return collectionVariableRef->userContext();
  return 0;
}


Ptr<Datatype> Type::collection_methods::CollectionMethod::getDatatype() const {
  if (collectionDatatype_)
    return collectionDatatype_->getDatatype();
  else
    return 0;
}


CLoc Type::collection_methods::CollectionMethod::getLLoc() const {
  if (collectionDatatype_)
    return collectionDatatype_->getLLoc();
  return emptyFilelocation;
}


void Type::collection_methods::AccessToItem::linterReference(Codestream &str) {
  if (collectionVariableRef)
    collectionVariableRef->linterReference(str);
  else if (collectionDatatype_)
    collectionDatatype_->linterReference(str);
}


void Type::CollectionType::updateThisDatatypeSemanticNode(SemanticTree *node) const {
  thisDatatype->setSemanticNode(node);
  thisDatatype->tidEntity()->semanticNode(node);
}



Sm::IsSubtypeValues Type::CollectionType::isSubtype(ResolvedEntity *supertype, bool isPlContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  else if (mappedType_)
    return mappedType_->isSubtype(supertype, isPlContext);
  else
    return EXPLICIT;
}


Sm::Type::CollectionType::ParsedMetods::mapped_type
Type::CollectionType::findMethod(const string &methodName) {
  ParsedMetods::iterator it = parsedMetods.find(methodName);
  if (it != parsedMetods.end())
    return it->second;
  return 0;
}


Sm::Type::CollectionType::ParsedMetods::mapped_type
Type::CollectionType::addMethod(const string &methodName, ParsedMetods::mapped_type method) {
  return parsedMetods.insert(ParsedMetods::value_type(methodName, method)).first->second;
}


Type::Varray::Varray(CLoc l, Ptr<Id2> _name, Ptr<Id> oid, Ptr<NumericValue> _sizeLimit, Ptr<Datatype> _elementType, bool _isNotNull)
  : CollectionType(l, _name, oid, new Datatype(_name->entity()), _elementType),
    sizeLimit  (_sizeLimit  ),
    isNotNull  (_isNotNull  ) {}


Type::NestedTable::NestedTable(CLoc l, Ptr<Id2> _name, Ptr<Id> _oid, Ptr<Datatype> _valueType, Ptr<Datatype> _keyType, bool _isNotNull)
  : CollectionType(l, _name, _oid, new Datatype(_name), _valueType),
    keyType_   (_keyType  ),
    isNotNull (_isNotNull)  {}


Ptr<Datatype> Type::NestedTable::keyType() const {
  if (keyType_)
    return keyType_;
  else
    return Datatype::mkInteger();
}
