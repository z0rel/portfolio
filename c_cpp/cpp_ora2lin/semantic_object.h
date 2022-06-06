#ifndef SEMANTIC_OBJECT_H
#define SEMANTIC_OBJECT_H

#include "semantic_base.h"
#include "semantic_id.h"

namespace Sm {

namespace Type {

class RecordField           : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>       name;
  Ptr<Datatype> datatype;
  Ptr<PlExpr>   defaultValue;
  bool          isNotNull;

  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Id> &field);
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  bool isField() const { return true; }
  Ptr<Id> getName() const;
  ResolvedEntity *getFieldDDLContainer() const;

  ResolvedEntity* getNextDefinition() const;
  void oracleDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  void linterReference(Sm::Codestream &str);

  RecordField(CLoc l, Ptr<Id> _name, Ptr<Datatype> t, Ptr<PlExpr> dfltValue = 0, bool _isNotNull = false);
  RecordField();

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
  virtual ~RecordField();

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, datatype, defaultValue); }


  ScopedEntities ddlCathegory() const { return FieldOfRecord_; }
  RecordField *toSelfRecordField() const { return const_cast<RecordField*>(this); }
};

class Record                 : public Declaration {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Ptr<Datatype> thisDatatype;
  Ptr<Id>                 name;
public:
  int xmlType_ = 0;
  Ptr<List<RecordField> > fields;

  string translatedName() const { return name->toNormalizedString(); }

  Record(Ptr<Id> _name, Ptr<List<RecordField> > _fields, CLoc l = cl::emptyFLocation());

  ScopedEntities ddlCathegory() const { return Record_; }
  bool getFieldRef(Ptr<Id> &field);
  void linterDefinition(Sm::Codestream &)  {}
  void linterDatatypeReference(Sm::Codestream &s);
  void linterReference(Sm::Codestream &str) { str << name; }
  bool isExactlyEquallyByDatatype(ResolvedEntity *oth);
  Sm::Type::Record* toSelfRecord() const { return (Record*)this; }
  Ptr<Id> getName() const;
  bool isDefinition() const;
  Ptr<Datatype> getDatatype() const;
  bool getFields(std::vector<Ptr<Sm::Id> > &fields) const;
  bool hasLinterEquivalent() const { return false; }
  bool isRecordType() const { return true; }

  void traverseModelStatements(StatementActor &) {}
  SemanticTree *toSTreeBase() const;
  Ptr<List<RecordField> > getRecordFields() const { return fields.object(); }

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, *fields); }
};

class JavaExternalSpec       : public GrammarBaseSmart {
public:
  Ptr<Id>                     javaExtName;
  Type::java_external_spec::Module  module;
  DEF_LBCONSTRUCTOR2(JavaExternalSpec, javaExtName, module) {}
};

class Object;

struct RefInfo : public smart::Smart {
  std::string fieldName;
  Ptr<Datatype> fieldType;
  Ptr<Id> idRef;
  Ptr<Id> member;
  ResolvedEntity *objectType;
  RefInfo *next;

  RefInfo(const string &fieldName_,
          Ptr<Datatype> fieldType_,
          Ptr<Id> idRef_,
          ResolvedEntity *object_) :
        Smart(),
        fieldName(fieldName_),
        fieldType(fieldType_),
        idRef(idRef_),
        objectType(object_),
        next(NULL) {;}
  RefInfo(const RefInfo &other) :
        Smart(other),
        fieldName(other.fieldName),
        fieldType(other.fieldType),
        idRef(other.idRef),
        objectType(other.objectType),
        next(other.next) {;}


  std::string toDebugString() const;

  void translateObjRef(Sm::Codestream &);
  void translateAssign(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr);
  bool updateObjectTypeReference(Sm::Codestream &str);
};

class RefDecoder {
public:
    RefDecoder() {;}
    ~RefDecoder() {;}

    bool decode(Codestream &str, const IdEntitySmart &entity, Sm::PlExpr *expr = NULL);
private:
    bool copyCollection(Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity *lObject, const IdEntitySmart &rEntity, ResolvedEntity *rObject);
    bool copyFromCursorFields(Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity *lObject, const IdEntitySmart &rEntity, ResolvedEntity *rCursor);
    bool copyToCursorFields(Codestream &str, const IdEntitySmart &lEntity, ResolvedEntity *lCursor, const IdEntitySmart &rEntity, ResolvedEntity *rObject);
    bool copyAssignment(Sm::Codestream &str, const IdEntitySmart &lEntity, const IdEntitySmart &rEntity);
    bool checkObjectType(Codestream &str, RefInfo *refFirst);
    
    ResolvedEntity *getMethodObject(ResolvedEntity *method);
    void copyFields(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr);
    Ptr<RefInfo> newFieldInfo(RefInfo *refInfo, Ptr<Id> field);
    Ptr<RefInfo> newFieldInfoCursor(ResolvedEntity *var, const IdEntitySmart &entity, IdEntitySmart::const_iterator it);

    typedef std::list<Ptr<RefInfo>> RefInfoList;
    RefInfoList decodedRefs;
    bool onlyDecode = false;
};

class ObjectType : public Declaration {
  typedef std::map<std::string, Ptr<ObjectType>> TranslatedMap;
  static TranslatedMap s_translatedObjMap;
  static Ptr<Id>         s_currentConstructedVar;

  void setNameDefinition() {
    if (name)
      if (Id *entity = name->entity())
        entity->definition((ObjectType*)this);
  }
protected:
  Ptr<Object>     objectBody;
  Ptr<Id2>        name;
  Ptr<Id>         oid;
  int             actionNumber_ = 0;
  bool            addTypeName_  = false;

public:
  enum BuiltinMethod {
    BM_CTOR = 0,
    BM_GET,
    BM_SET,
    BM_DELETE,
    BM_TRIM,
    BM_EXTEND,
    BM_EXISTS,
    BM_FIRST,
    BM_LAST,
    BM_COUNT,
    BM_LIMIT,
    BM_PRIOR,
    BM_NEXT,
    // Методы копирования и вставки
    BM_COPY_SQL, // копирование с запросом в другую таблицу
    BM_COPY_ALL, // копирование из другой коллекции или объекта
    BM_COPY_IDX, // копирование определенного нумерованного индекса из коллекции в другую коллекцию
    BM_INSERT_SQL, // вставка данных в виде строки в коллекцию
  };

  virtual std::string getMethodName(BuiltinMethod /*mtype*/, int /*argsize*/, Sm::Datatype * /*dtype*/ = NULL) { return ""; }
  bool isXmlType() const;
  bool isAnydata() const;
  bool isAnytype() const;
  bool isOracleNontranslatedDatatype() const;

  struct ExtractColFields {
  public:
    IdEntitySmart newReference;
    Ptr<Id> accessor;
    Ptr<PlExpr> indexExpr;
    EntityFields opFields;
    string baseName;
  };
  static bool extractCollectionAccessorFields(const IdEntitySmart &reference, ExtractColFields &result);

protected:
  std::string getTypeString(Sm::Datatype *type);
  std::string generateTabName(ResolvedEntity *inEntity);
  void extractNestedFields(Sm::Codestream &str, ResolvedEntity *obj, const std::string &prevField = std::string());
  void baseAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr);
  virtual void translateSpecificFields(Sm::Codestream &) {;}
  virtual void translateIndexes(Sm::Codestream &, const std::string &) {;}

public:
  virtual void callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor) = 0;
  virtual void setAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr) = 0;
  virtual void getAccessor(Sm::Codestream &str, RefInfo *refInfo) = 0;

public:
  virtual void resolve(ModelContext &model) = 0;

  ScopedEntities ddlCathegory() const = 0;
  Ptr<Id> getName() const { return name ? Ptr<Id>(name->entity()) : Ptr<Id>(); }
  Ptr<Id2> getName2() const { return name; }
  void actionNumber(size_t val) { actionNumber_ = val; }
  void setObjectBody(Ptr<Object> b);

  virtual bool isDefinition() const = 0;
  virtual Ptr<Datatype> getDatatype() const = 0;
  bool isObjectType() const { return true; }
  void translateAssign(Sm::Codestream &str, RefInfo *info, Sm::PlExpr *expr);
  void translateVariableType (Sm::Codestream &str, Sm::ResolvedEntity* var, bool addTypeName);
  void translateObjRef(Sm::Codestream &str, RefInfo *info);
  void linterDefinition (Sm::Codestream &str)  { str << "BIGINT"; }
  void linterDatatypeReference(Sm::Codestream &str) { linterDefinition(str); }
  void linterReference(Sm::Codestream &str) { str << name; }
  bool hasLinterEquivalent() const { return false; }
  std::string getObjectTempTable(Sm::ResolvedEntity *def);
  ObjectType* toSelfObjectType() const { return (ObjectType*)this; }

  ObjectType(CLoc l = cl::emptyFLocation(), Ptr<Id2> _name = 0, Ptr<Id> _oid = 0);
  ObjectType(Ptr<Id2> _name, Ptr<Id> _oid) : name(_name), oid(_oid), addTypeName_(false) { setNameDefinition(); }
  void translateRefInfoNextOrIdRef(Sm::Codestream &str, RefInfo *refInfo);
  string generateTabNameWithoutUser(ResolvedEntity *inEntity, UserContext **userOwner);

};

class Object : public ObjectType {
public:
  typedef JavaExternalSpec JavaSpec;
  typedef BaseList<MemberInterface> Elements;
private:
  mutable int supertypeUfsFieldsCnt = -1;
  mutable Ptr<Datatype> thisDatatype;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

public:
  Object *head = 0;

protected:

  Elements                               elements;
  mutable BaseList<Type::MemberFunction> constructors;
  BaseList<pragma::Pragma>               pragmas;
  Object *declObj = 0;

public:
  Ptr<Id2>                      supertype;
  auth_id::AuthID               authID;
  Ptr<JavaExternalSpec>         javaExtSpec;
  mutable Ptr<MemberFunction>   defaultConstructor;

  Object(CLoc l, Ptr<Id2> name, Ptr<BaseList<MemberInterface> > elem,
         Ptr<Id>         oid         = 0,
         Ptr<Id2>        superType   = 0,
         auth_id::AuthID auth        = auth_id::EMPTY,
         Ptr<JavaSpec>   extJavaSpec = 0);
  ~Object();

  void addChilds(Object *obj, SemanticTree *node) const;

  void resolve(ModelContext &model);
  void resolveMember(Ptr<Id> name, FunctionResolvingContext &cntx) const;
  void resolveMemberInSupertype(Ptr<Id> name, FunctionResolvingContext &cntx) const;

  SemanticTree *toSTreeBase() const;
  void collectChildsToSNode(SemanticTree *node) const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  bool isExactlyEquallyByDatatype(ResolvedEntity*);
  bool isBody() const { return declObj != NULL; }
  bool isDefinition() const { return objectBody.valid(); }
  virtual bool hasLinterEquivalent() const { return false; }
  ScopedEntities ddlCathegory() const { return  (isAnydata()) ?  AnydataObject_ : Object_; }

  inline bool isObject()     const { return (isAnydata() || isAnytype() || isXmlType() || isRefCursor()) ? false : true; }
  inline bool isObjectType() const { return (isAnydata() || isAnytype() || isXmlType() || isRefCursor()) ? false : true; }

  BaseList<MemberInterface> *getMembers();

  Ptr<Datatype> getDatatype() const;
  Type::Object *getOwner() const { return const_cast<Object*>(this); }

  bool isRefCursor() const;

  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;

  void setDeclaration(Object *b) { declObj = b; }
  Object *getDeclaration() const { return declObj; }

  std::string getMethodName(BuiltinMethod mtype, int argsize, Sm::Datatype *dtype = NULL);
  virtual void callCtor(Sm::Codestream &str, Ptr<Sm::Id> idRef, Ptr<Sm::Id> idConstructor);
  virtual void setAccessor(Sm::Codestream &str, RefInfo *refInfo, Sm::PlExpr *expr);
  virtual void getAccessor(Sm::Codestream &str, RefInfo *refInfo);

  void linterDefinition (Sm::Codestream &str);
  //virtual void linterDatatypeReference(Sm::Codestream &s) { if (isXmlType()) s << "XMLTYPE"; else s << "BIGINT"; }

  void collectInitializers(EntitiesSet &container);

  Function *getDefaultConstructor() const;

  void traverseDeclarations(DeclActor &fun);
  void replaceChildsIf(Sm::ExprTr tr);
  void replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond);

  Object* toSelfObject()  const { return isAnydata() ? nullptr : (Object*)this; }


  void traverseDeclarationsForce(DeclActor &fun);

  void traverseModelStatements(StatementActor &fun);

  void applyDynamicUsing();

protected:
  Object* toSelfAnydata() const { return isAnydata() ? (Object*)this : nullptr; }

  virtual void translateSpecificFields(Sm::Codestream &str);
  virtual void translateIndexes(Sm::Codestream &str, const std::string &tableName);
};








}

}

#endif // SEMANTIC_OBJECT_H
