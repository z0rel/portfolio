#include "semantic_object.h"
#include "semantic_datatype.h"

namespace Sm {

namespace Type {

namespace collection_methods {

class CollectionMethod : public ResolvedEntity {
protected:
  ResolvedEntity* collectionDatatype_ = 0;
public:
  ResolvedEntity* collectionVariableRef = 0;

  CollectionMethod(ResolvedEntity *_collectionVariable);

  ScopedEntities ddlCathegory() const { return ResolvedEntity::CollectionMethod_; }

  UserContext* userContext() const;
  CollectionMethod* toSelfCollectionMethod() const { return (CollectionMethod*)this; }

  Ptr<Id> getName() const { return collectionDatatype_->getName(); }

  static Ptr<CollectionMethod> parse(Ptr<Sm::Id> field, CollectionType *variable);
  ResolvedEntity* collectionDatatype() const { return collectionDatatype_; }
  CollectionType* getCollection() const;
  Ptr<Datatype> getElementDatatype() const;
  Ptr<Datatype> getKeyDatatype() const;
  Ptr<Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }
  virtual std::string methodName() const = 0;
  bool isNonblockPseudoField() const { return true; }
  void linterReference(Sm::Codestream &str) { str << collectionDatatype_; }
  virtual bool isMethod() const { return true; }
  virtual bool isCollectionMethod() const { return true; }
  void translateLoopHead(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::FunCallArg> N, Ptr<Variable> &tmpVar);

  virtual void formalTranslateArgs(Ptr<Sm::Id> /*call*/) {;}

  CLoc getLLoc() const;
protected:
  bool getCollectionFieldStruct(EntityFields &fieldStructure) const { return collectionDatatype_ && collectionDatatype_->getFields(fieldStructure); }
  bool getCollectionFieldRef(Ptr<Id> &field);
  bool castArgIfNeed(Ptr<Sm::FunCallArg> arg, Ptr<Datatype> needType);
  void translateArgsType(Ptr<CallArgList> argList, unsigned int keyMask);
};

class AccessToItem : public CollectionMethod { // operator()
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  AccessToItem(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return getElementDatatype(); }
  bool getFields(EntityFields &fieldStructure) const { return getCollectionFieldStruct(fieldStructure); }
  bool getFieldRef(Ptr<Id> &field) { return getCollectionFieldRef(field); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return collectionDatatype_->isSubtype(supertype, plContext); }
  virtual std::string methodName() const { return "()"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
  bool isCollectionAccessor() const { return true; }
  bool isCollectionMethod() const { return false; }
  void linterReference(Sm::Codestream &str);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class Count  : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Count(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return Datatype::mkInteger(); }
  virtual std::string methodName() const { return "COUNT"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
};
class Delete : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  vector<Ptr<SqlExpr> > indices;

  Delete(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return 0; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }
  virtual std::string methodName() const { return "DELETE"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
  void translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class Exists : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> index;

  Exists(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  virtual std::string methodName() const { return "EXISTS"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class Extend : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  vector<Ptr<SqlExpr> > indices;

  Extend(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return 0; }
  virtual std::string methodName() const { return "EXTEND"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
  void translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class First  : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  First(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return getKeyDatatype(); }
  bool getFields(EntityFields &fieldStructure) const { return getCollectionFieldStruct(fieldStructure); }
  virtual std::string methodName() const { return "FIRST"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
};
class Last   : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Last(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return getKeyDatatype(); }
  bool getFields(EntityFields &fieldStructure) const { return getCollectionFieldStruct(fieldStructure); }
  virtual std::string methodName() const { return "LAST"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
};
class Limit  : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Limit(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return Datatype::mkInteger(); }
  virtual std::string methodName() const { return "LIMIT"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
};
class Next   : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> index;

  Next(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return getKeyDatatype(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }
  virtual std::string methodName() const { return "NEXT"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class PriorExpr  : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> index;

  PriorExpr(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return getKeyDatatype(); }
  virtual std::string methodName() const { return "PRIOR"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  virtual void formalTranslateArgs(Ptr<Sm::Id> call);
};
class Trim   : public CollectionMethod {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> index;

  Trim(Ptr<ResolvedEntity> _collectionVariable) : CollectionMethod(_collectionVariable) {}
  Ptr<Datatype> getDatatype() const { return 0; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }
  virtual std::string methodName() const { return "TRIM"; }
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
  void translateAsLoop(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);
};
};

class CollectionType : public ObjectType {
protected:
  mutable Ptr<Datatype> thisDatatype;
  Ptr<ArrayConstructor> constructor;
  typedef  map<std::string, Ptr<collection_methods::CollectionMethod> > ParsedMetods;
  vector<Ptr<collection_methods::AccessToItem> > accesingInExpressions;
  ParsedMetods parsedMetods;
  Ptr<Datatype> mappedType_;

  void updateThisDatatypeSemanticNode(SemanticTree *node) const;

  CollectionType* toSelfCollectionType() const { return (CollectionType*)this; }
  void procCtorArgs(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor);
  virtual bool needAddExtend() const { return false; }

public:
  virtual void setAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr);
  virtual void getAccessor(Sm::Codestream &str, RefInfo *refInfo);

public:

  CollectionType(CLoc l, Ptr<Id2> _name, Ptr<Id> _oid, Datatype *thisT, Ptr<Datatype> mT);
  ~CollectionType();
  Ptr<Datatype> mappedType() const { return mappedType_; }
  bool isSqlTableContainerFuncall() const { return true; }
  bool isCollectionType() const { return true; }
  Sm::Function *getDefaultConstructor() const;
  collection_methods::AccessToItem *addAccesingMethod(ResolvedEntity* def);

  string translatedName() const { return getName()->toNormalizedString(); }

  Ptr<Sm::Datatype> getElementDatatype() const { return mappedType_; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool isPlContext) const;
  bool getFieldRef(Ptr<Sm::Id> &field);
  Ptr<Datatype> getDatatype() const { return thisDatatype; }
  bool isDefinition() const { return true; }
  void resolve(ModelContext &) {}
  bool getFields(EntityFields &fields) const;
  ParsedMetods::mapped_type findMethod(const string &methodName);

  ParsedMetods::mapped_type addMethod(const string &methodName, ParsedMetods::mapped_type method);
  bool isExactlyEquallyByDatatype(ResolvedEntity*) { return false; }
  void traverseModelStatements(Sm::StatementActor &) {}
};

class Varray                 : public CollectionType {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  virtual void translateSpecificFields(Sm::Codestream &str);
  virtual void translateIndexes(Sm::Codestream &str, const std::string &tableName);
  virtual void callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor);
  virtual bool needAddExtend() const { return true; }

public:
  Ptr<NumericValue> sizeLimit;
  bool              isNotNull;

  Varray(CLoc l,
         Ptr<Id2> _name,
         Ptr<Id>  oid,
         Ptr<NumericValue> _sizeLimit,
         Ptr<Datatype>     _elementType,
         bool              _isNotNull);

  virtual bool isVarray() const { return true; }

  Ptr<Sm::Datatype> keyType() const { return Datatype::mkInteger(); }

  ScopedEntities ddlCathegory() const { return Varray_; }

  SemanticTree *toSTreeBase() const;

  void linterDefinition (Sm::Codestream &str);
  std::string getMethodName(BuiltinMethod mtype, int argsize, Sm::Datatype * dtype = NULL);
};
class NestedTable            : public CollectionType {    // nested or assoc_array, см ман
protected:
  Ptr<Datatype> keyType_;   // Когда задано - это associative array

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  virtual void translateSpecificFields(Sm::Codestream &str);
  virtual void translateIndexes(Sm::Codestream &str, const std::string &tableName);
  virtual void callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor);
  virtual bool needAddExtend() const { return isNestedTable(); }

public:
  bool          isNotNull;

  NestedTable(
      CLoc          l,
      Ptr<Id2>      _name,
      Ptr<Id>       _oid,
      Ptr<Datatype> _valueType,
      Ptr<Datatype> _keyType,
      bool          _isNotNull);

  Ptr<Sm::Datatype> keyType() const;
  virtual bool isAssocArray() const { return keyType_; }
  virtual bool isNestedTable() const { return !keyType_; }

  ScopedEntities ddlCathegory() const { return NestedTable_; }
  ResolvedEntity* getNextDefinition() const { return mappedType_.object(); }

  SemanticTree *toSTreeBase() const;

  void oracleDefinition(Sm::Codestream &str);
  void linterDefinition (Sm::Codestream &str);
  std::string getMethodName(BuiltinMethod mtype, int argsize, Sm::Datatype * dtype = NULL);
};



}

}
