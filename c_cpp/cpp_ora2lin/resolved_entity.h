#ifndef RESOLVED_ENTITY_H
#define RESOLVED_ENTITY_H

#include <stack>
#include "semantic_utility.h"


namespace Sm {

namespace Type {
  class MemberVariable;
}

namespace update {
class FieldFromExpr;
class FieldsFromSubquery;
}
class DynSubquery;
class FunctionDynExpr;
class ConstructBlockStmt;
class DynWhere;
class CursorDecltype;
//class CursorFieldDecltype;
class NullStatement;
class NullType;
class DefaultType;
class Brackets;
class AsteriskExpr;
class AsteriskRefExpr;
class FromTableReference;
class FromTableDynamic;
class SelectSingle;
class Subtype;
class VariableCursorFields;
class VariableField;
class FunCallArgExpr;
class RowNumExpr;
class FunctionResolvingContext;
class Cast;
class QueryPseudoField;
class ChangedQueryEntityRef;

class SelectBrackets;

  class AlgebraicCompound;
  class SelectSingle;
  class Brackets;
  class RowNumExpr;
  class Statement;
  class NumericValue;
  class ExtractExpr;
  class SqlExpr;
  class Subquery;
  class RefAbstract;
  class RefExpr;
  class RefHostExpr;
  class RefExpr;
  class Case;
  class CursorExpr;

namespace pl_expr {
  class LogicalCompound;
  class Comparsion;
  class Between;
}

namespace trigger {
  class TriggerAbstractRowReference;
  class TriggerRowReference;
};



class ResolvedEntity : public virtual smart::Smart,
                       public virtual TranslatedName,
                       public virtual CathegoriesOfDefinitions,
                       public IsSubtypeValuesEnum,
                       public TranslatorTailMakestr {
protected:
  static const cl::filelocation emptyFilelocation;
  mutable BasicEntityAttributes *basicEntityAttributes = 0;
  mutable Sm::SemanticTree      *semanticNode = 0;   // 8
  const GlobalEntitiesCnt eid_ = nextGlobalCnt();

  inline Sm::SemanticTree *clrSTreeOwner() const { __flags__.v |= FLAG_ENTITY_IS_NOT_STREE_OWNER; return 0; }
public:
  GlobalEntitiesCnt eid() const;

  static GlobalEntitiesCnt globalEntitiesCounter;
  static GlobalEntitiesCnt nextGlobalCnt();


  ResolvedEntity() {}
  ResolvedEntity(const ResolvedEntity &oth)
    : Smart(oth),
      basicEntityAttributes(oth.basicEntityAttributes),
      semanticNode(oth.semanticNode) {}

  void clrSRef();
  inline void clrSTree() { setSemanticNode(0); }
  inline BasicEntityAttributes *lazyBasicEntityAttirbutes() const {
    if (basicEntityAttributes)
      return basicEntityAttributes;
    else {
      basicEntityAttributes = new BasicEntityAttributes();
      return basicEntityAttributes;
    }
  }
  inline EntityAttributes *lazyAttributes() const { return lazyBasicEntityAttirbutes()->lazyAttributes(); }

  virtual Sm::SemanticTree *setSemanticNode(Sm::SemanticTree *t) const { semanticNode = t; return t; }
  virtual Sm::SemanticTree *getSemanticNode() const { return semanticNode; }
  void clearSemanticNodeIfEq(Sm::SemanticTree *node) { if (semanticNode && semanticNode == node) semanticNode = 0; }

  VEntities* vEntities() const { return basicEntityAttributes ? basicEntityAttributes->vEntities : (VEntities*)0; }
  void setVEntities(const VEntities* n) const { lazyBasicEntityAttirbutes()->vEntities = (VEntities*)n; }

  virtual void* vEntity() const { return getThisPtr(); }
  virtual SmartVoidType* getThisPtr() const = 0;

  Ptr<LevelResolvedNamespace> internalNamespace() const;
  LevelResolvedNamespace* levelNamespace() const { return basicEntityAttributes ? basicEntityAttributes->levelNamespace.object() : nullptr; }
  void levelNamespace(LevelResolvedNamespace *n) const { lazyBasicEntityAttirbutes()->levelNamespace = n; }

  static Sm::Datatype* getLastConcreteDatatype(ResolvedEntity *start);
  static Sm::Datatype* getLastUnwrappedDatatype(ResolvedEntity *start);
  virtual Ptr<Sm::Datatype> getDatatype()  const = 0;
  virtual Ptr<Sm::Datatype> getUnforeignDatatype()  const;


  Datatype* unwrappedDatatype() const;

protected:
  Ptr<Sm::Datatype> getDatatypeWithSetFieldStruct(Ptr<Sm::Datatype> &thisDatatype, Ptr<Sm::Id> entityName) const;
  IsSubtypeValues isSubtypeByDatatype(Ptr<ResolvedEntity> supertype, bool isPlContext) const;
public:

  virtual void setRetType(Ptr<Sm::Datatype>);
  virtual void setDatatype(Ptr<Sm::Datatype>);
  virtual void setDatatypeForMember(Ptr<Sm::Id>, Ptr<Sm::Datatype>);

  virtual Sm::IsSubtypeValues isSubtypeFundamental(Sm::GlobalDatatype::DatatypeCathegory /*typeCat*/, bool /*plContext*/) const { return EXPLICIT; }
  virtual IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  virtual ResolvedEntity* getNextDefinition() const { return (ResolvedEntity*)this; }
  virtual ResolvedEntity* getNextNondatatypeDefinition() const;
  inline  ResolvedEntity* getDefinitionFirst() const;
  ResolvedEntity* getConcreteDefinition() const;
  ResolvedEntity* getConcreteDatatype() const;
  ResolvedEntity* getResolvedConcreteDefinition() const;
  ResolvedEntity* getResolvedNextDefinition();

  inline bool eqByVEntities(const ResolvedEntity *oth) const { return VEntities::eqByVEntities(this, oth); }

  virtual void identificateOverloaded();
  virtual int  overloadedId() const { return -1; }
  virtual void overloadedId(int) { }

  virtual void setNotOutStatements() {}

  virtual void resolveMember(Ptr<Sm::Id> /*call*/, FunctionResolvingContext &/*cntx*/) const {}
  virtual void resolveName(ModelContext &/*model*/, const Sm::Id2 */*name*/) {}

  virtual bool isFieldForMakestr() const { return false; }

  virtual bool skipLevelNamespaceUpdating() const { return false; }

  Ptr<Sm::Datatype> tryResolveDatatype();

  virtual ResolvedEntity* tryResoveConcreteDefinition() const;
  bool semanticResolveBase() const;
  virtual bool semanticResolve() const;

protected:
  ResolvedEntity* __tryResolveConcreteDefinition() const;
  void updateNameDefinition();

public:

  virtual ForOfRange* toSelfForOfRange() const { return 0; }
  virtual DeleteFrom* toSelfDeleteFrom() const { return 0; }
  virtual FunCallArgExpr *toSelfFunCallArgExpr() const { return 0; }
  virtual QueryEntityDyn* toSelfQueryEntityDyn() const { return 0; }
  virtual DynSubquery* toSelfDynSubquery() const { return 0; }
  virtual FunctionDynExpr *toSelfDynExpr() const { return 0; }
  virtual DynWhere *toSelfDynWhere() const { return 0; }
  virtual ConstructBlockStmt *toSelfConstructBlockStmt() const { return 0; }
  virtual update::FieldFromExpr* toSelfFieldFromExpr() const { return 0; }
  virtual update::FieldsFromSubquery* toSelfFieldsFromSubquery() const { return 0; }
  virtual FunctionDynField *toSelfFunctionDynField() const { return 0; }
  virtual Statement *toSelfStatement() const { return 0; }
  virtual trigger::TriggerRowReference* toSelfTriggerRowReference() const { return 0; }
  virtual RowNumExpr* toSelfRowNumExpr() const { return 0; }
  virtual Sm::ExtractExpr* toSelfExtractFromExpression() const { return 0; }
  virtual Sm::RefHostExpr* toSelfRefHostExpr() const { return 0; }
  virtual RefExpr*toSimpleFunctionCall() const { return 0; }
  virtual pl_expr::LogicalCompound* toSelfLogicalCompound() const { return 0; }
  virtual pl_expr::Comparsion* toSelfComparion() const { return 0; }
  virtual Sm::Case* toSelfCase() const { return 0; }
  virtual pl_expr::Between* toSelfBetween() const { return 0; }
  virtual CursorExpr* toSelfCursorExpr() const { return 0; }

  virtual Type::RecordField *toSelfRecordField() const { return 0; }
  virtual Type::MemberVariable *toSelfMemberVariable() const { return 0; }
  virtual SelectBrackets *toSelfSelectBrackets() const { return 0; }
  virtual ChangedQueryEntityRef* toSelfChangedQueryEntityRef() const { return 0; }
  virtual QueryPseudoField *toSelfQueryPseudoField() const { return 0; }
  virtual Cast* toSelfCastStatement() const { return 0; }
  virtual BooleanLiteral *toSelfBooleanLiteral() const { return 0; }
  virtual StatementInterface *toSelfStatementInterface() const { return 0; }
  virtual Synonym *toSelfSynonym() const { return 0; }
  virtual VariableCursorFields *toSelfVariableCursorFields() const { return 0; }
  virtual Index* toSelfIndex() const { return 0; }
  virtual Table* toSelfTable() const { return 0; }
  virtual Sequence* toSelfSequence() const { return 0; }
  virtual Update* toSelfUpdate() const { return 0; }
  virtual table::FieldDefinition *toSelfFieldDefinition() const { return 0; }
  virtual Subtype* toSelfSubtype() const { return 0; }
  virtual NumericValue *toSelfNumericValue() const { return 0; }
  virtual SelectSingle *toSelfSelectSingle() const { return 0; }
  virtual FromTableReference* toSelfFromTableReference() const { return 0; }
  virtual FromTableDynamic* toSelfFromTableDynamic() const { return 0; }
  virtual FromSingle* toSelfFromSingle() const { return 0; }
  virtual SelectedField* toSelfSelectedField() const { return 0; }
  virtual AsteriskRefExpr* toSelfAsteriskRefExpr() const { return 0; }
  virtual AsteriskExpr* toSelfAsteriskExpr() const { return 0; }
  virtual NullStatement* toSelfNullStmt() const { return 0; }
  virtual NullType* toSelfNullType() const { return 0; }
  virtual DefaultType* toSelfDefaultType() const { return 0; }
  virtual UserContext* toSelfUserContext() const { return 0; }
  virtual Fetch* toSelfFetch() const { return 0; }
  virtual Close* toSelfClose() const { return 0; }
  virtual FunctionArgument* toSelfFunctionArgument() const { return 0; }
  virtual CaseStatement *toSelfCaseStmt() const { return 0; }
  virtual OpenFor* toSelfOpenFor() const { return 0; }
  virtual OpenCursor* toSelfOpenCursor() const { return 0; }
//  virtual CursorFieldDecltype* toSelfCursorFieldDecltype() const { return 0; }
  virtual CursorDecltype* toSelfCursorDecltype() const { return 0; }
  virtual FunctionCall *toSelfFunctionCall() const { return 0; }
  virtual LValue* toSelfLValue() const { return 0; }
  virtual Cursor    *toSelfCursor() const { return 0; }
  virtual Type::ArrayConstructor   *toSelfArrayConstructor() const { return 0; }
  virtual Subquery *toSelfSubquery() const { return 0; }
  virtual Variable   *toSelfVariable() const { return 0; }
  virtual VariableField *toSelfVariableField() const { return 0; }
  virtual Trigger    *toSelfTrigger() const { return 0; }
  virtual Package    *toSelfPackage() const { return 0; }
  virtual View       *toSelfView() const { return 0; }
  virtual Assignment *toSelfAssignment() const { return 0; }
  virtual AlgebraicCompound *toSelfAlgebraicCompound() const { return 0; }
  virtual Type::collection_methods::CollectionMethod* toSelfCollectionMethod() const { return 0; }
  virtual GlobalDatatype::FundamentalDatatype* toSelfFundamentalDatatype() const { return 0; }
  virtual Type::ObjectType*     toSelfObjectType() const { return 0; }
  virtual Type::CollectionType* toSelfCollectionType() const { return 0; }
  virtual Type::MemberFunction* toSelfMemberFunction() const { return 0; }
  virtual QueryBlock *toSelfQueryBlock() const { return 0; }
  virtual RefExpr*     toSelfRefExpr()     const { return 0; }
  virtual SqlExpr*              toSelfSqlExpr()    const { return 0; }
  virtual RefAbstract*            toSelfRefAbstract()    const { return 0; }
  virtual Function*             toSelfFunction()   const { return 0; }
  virtual Datatype*             toSelfDatatype()   const { return 0; }
  virtual pragma::Pragma*       toSelfPragma()     const { return 0; }
  virtual Type::Record*         toSelfRecord()     const { return 0; }
  virtual BlockPlSql*           toSelfBlockPlSql() const { return 0; }
  virtual If*                   toSelfIf()         const { return 0; }
  virtual Type::RefCursor*      toSelfRefCursor()  const { return 0; }
  virtual Type::Object*         toSelfObject()     const { return 0; }
  virtual Type::Object*         toSelfAnydata()     const { return 0; }
  virtual Return*               toSelfReturn()     const { return 0; }
  virtual Function *            castToFunction()   const { return 0; }
  virtual Brackets *            toSelfBrackets()   const { return 0; }
  virtual trigger::TriggerAbstractRowReference*   toSelfAbstractRowReference() const { return 0; }
  virtual void setFieldNumber(int);

  virtual ResolvedEntity *ownerVariable() const;
  virtual ResolvedEntity *ownerPackage() const;
  virtual Sm::ResolvedEntity*  ownerFunction() const;
  virtual Sm::BlockPlSql*      maximalCodeBlock(Sm::BlockPlSql *start = 0) const;
  virtual Sm::BlockPlSql*      ownerPlBlock() const;
  ResolvedEntity*         owner () const;
  Sm::SemanticTree*            sOwner() const;
  virtual Sm::Type::Object    *getOwner() const { return 0; }

  virtual void userOwner(ResolvedEntity *)  {}
  virtual void foreignDatatype(Ptr<Sm::Datatype>) {}

  virtual void clrOpenCursorCommand();
  virtual void setOpenCursorCommand(Sm::OpenCursor *);
  virtual void setTransitive(bool) {}
  virtual void setTranslateToRefCursorAssignment() {}

  virtual void maxrow   (Ptr<Sm::NumericValue>) {}
  virtual void tablesize(Ptr<Sm::NumericValue>) {}

  virtual void setSquoted();
  virtual void addFieldProperty(Sm::table::field_property::FieldProperty*) {}
  virtual Sm::Type::collection_methods::AccessToItem* addAccesingMethod(ResolvedEntity */*def*/) { return 0; }

  inline void setIsSystem()         { __flags__.v |= FLAG_RESOLVED_ENTITY_IS_SYSTEM; }
  inline void setIsSystemTemplate() { __flags__.v |= (FLAG_RESOLVED_ENTITY_IS_SYSTEM_TEMPLATE | FLAG_RESOLVED_ENTITY_IS_SYSTEM); }
  inline void setIsSysUnsupported() { __flags__.v |= FLAG_RESOLVED_ENTITY_IS_SYS_UNSUPPORTED; }
  inline void clrBracesOutput()     { __flags__.v |= FLAG_RESOLVED_ENTITY_BRACES_NOT_OUTPUT; }
  inline void setIsDynamicUsing()   { __flags__.v |= FLAG_RESOLVED_ENTITY_IS_DYNAMIC_USING; }

  inline bool isSystemTemplate() const { return __flags__.v & FLAG_RESOLVED_ENTITY_IS_SYSTEM_TEMPLATE; }
  inline bool isSystemFlag()     const { return __flags__.v & FLAG_RESOLVED_ENTITY_IS_SYSTEM; }
  inline bool isSysUnsupported() const { return __flags__.v & FLAG_RESOLVED_ENTITY_IS_SYS_UNSUPPORTED; }
  inline bool bracesOutput()     const { return !(__flags__.v & FLAG_RESOLVED_ENTITY_BRACES_NOT_OUTPUT); }
  bool isDynamicUsing()          const;

  virtual bool usedInQueryAndContainsFields() { return false; }
  bool containsEntity(ResolvedEntity *entity);
  bool eqByFields(ResolvedEntity *supertype, bool isPlContext, bool inSqlCode) const;
  virtual bool equallyByArglists(ResolvedEntity *) const { return false; }
  virtual bool allArgsIsDefault() const { return false; }

  bool isResolved()   { return getResolvedConcreteDefinition() != 0; }
  bool isUnresolved() { return getResolvedConcreteDefinition() == 0; }
  bool isCodeBlockEntity()   const;
  bool isScopeEntity()       const;
  bool isElementaryLiteral() const;
  bool isElementaryType()    const;

  bool isSystemPartDBMS() const;

  virtual ResolvedEntity *getFieldDDLContainer() const { throw 999; return 0; }


  virtual bool isSystem()                       const ;
  virtual bool isCollectionType()               const { return false; }
  virtual bool isDatatype()                     const { return false; }
  virtual bool isObject()                       const { return false; }
  virtual bool isField()                        const { return false; }
  virtual bool isSqlIdentificator()             const { return false; }
  virtual bool isEverything()                   const { return false; }
  virtual bool isRef()                          const { return false; }
  virtual bool isException()                    const { return false; }
  virtual bool isNumberDatatype()               const { return false; }
  virtual bool isNumberSubtype()                const { return false; }
  virtual bool isBool()                         const { return false; }
  virtual bool isInt()                          const { return false; }
  virtual bool isBigint()                       const { return false; }
  virtual bool isSmallint()                     const { return false; }
  virtual bool isDecimal()                      const { return false; }
  virtual bool isDouble()                       const { return false; }
  virtual bool isFloat()                        const { return false; }
  virtual bool isReal()                         const { return false; }
  virtual bool isFundamentalDatatype()          const { return false; }

  virtual bool isIntervalDatatype()             const { return false; }
  virtual bool isDateDatatype()                 const { return false; }
  virtual bool isClobDatatype()                 const { return false; }
  virtual bool isBlobDatatype()                 const { return false; }
  virtual bool isVarcharDatatype()              const { return false; }
  virtual bool isCharDatatype()                 const { return false; }
  virtual bool isLongDatatype()                 const { return false; }


  virtual bool isNcharDatatype()                const { return false; }
  virtual bool isNVarcharDatatype()             const { return false; }

  virtual bool isTypeOf()                       const { return false; }
  virtual bool isRowTypeOf()                    const { return false; }
  virtual bool isObjectType()                   const { return false; }
  virtual bool isXmlType()                      const { return false; }
  virtual bool isAnydata()                      const { return false; }
  virtual bool isRecordType()                   const { return false; }
  virtual bool isCompositeType()                const;
  virtual bool isProcedure()                    const { return false; }
  virtual bool isConstructor()                  const { return false; }
  virtual bool isCollectionAccessor()           const { return false; }
  virtual bool isSingleFieldReference()         const { return false; }
  virtual bool isUnionChild()                   const { return false; }
  virtual bool isRefCursor()                    const { return false; }
  virtual bool datatypeIsNotResolved()          const { return false; }
  virtual bool isDefinition()                   const { return false; }
  virtual bool isSqlTableContainerFuncall()     const { return false; }
  virtual bool isCursorVariable()               const { return false; }
  virtual bool isDual()                         const { return false; }
  virtual bool isElementaryLinterFunction()     const { return false; }
  virtual bool isEmptyId()                      const { return false; }
  virtual bool isFunArgument()                  const { return false; }
  virtual bool isFunCallStatement()             const { return false; }
  virtual bool isFunction()                     const { return false; }
  virtual bool isMethod()                       const { return false; }
  virtual bool isCollectionMethod()             const { return false; }
  virtual bool isAssocArray()                   const { return false; }
  virtual bool isNestedTable()                  const { return false; }
  virtual bool isVarray()                       const { return false; }

  virtual bool isTrigger()                      const { return false; }
  virtual bool isView()                         const { return false; }
  virtual bool isTable()                        const { return false; }

  virtual bool isLinterQueryLiteral()           const { return false; }
  virtual bool isMemberVariable()               const { return false; }
  virtual bool isNonblockPseudoField()          const { return false; }
  virtual bool isNumericLiteral()               const { return false; }
  virtual bool isNumericValue()                 const { return false; }
  virtual bool isPackageVariable()              const { return false; }
  virtual bool isRowidDatatype()                const { return false; }
  virtual bool isRownum()                       const { return false; }
  virtual bool isMemberFunctionStatic()         const { return false; }
  virtual bool isPragma()                       const { return false; }
  virtual bool isNullField()                    const { return false; }

  virtual bool isMemberFunctionConstructor()    const { return false; }
  virtual bool isTriggerRowReferenceParent()    const { return false; }
  virtual bool isTriggerRowReference()          const { return false; }
  virtual bool isVariable()                     const;
  virtual bool isTempVariable()                 const { return false; }
  virtual bool isUnsupportedSysFuncall() { return isSysUnsupported(); }
  virtual bool isExactlyEquallyByDatatype(ResolvedEntity*);
  virtual bool isLinterStructType() const { return false; }


  virtual bool translatedAsExecuteWithoutDirect() const { return false; }

  virtual Ptr<Sm::Arglist> getArglist() const;
  virtual Ptr<Sm::BlockPlSql> funBody() const;
  virtual Ptr<Sm::CallArgList> callArglist() const;

  virtual Ptr<Sm::Datatype> getDatatypeOfCollectionItem() const;
  virtual Ptr<Sm::Datatype> getElementDatatype() const;
  virtual Ptr<Sm::Datatype> keyType()    const;
  virtual Ptr<Sm::Datatype> mappedType() const;

  virtual Ptr<Sm::FunArgList> funArglist() const;
  virtual Ptr<Sm::Id2> getTarget() const;
  virtual Ptr<Sm::LValue> lvalue();
  virtual Sm::BlockPlSql *childCodeBlock() const;
  virtual Sm::Constraint *constraint() const { return 0; }
  virtual Sm::Exception* getExceptionDef() const { return 0; }
  virtual Sm::Function *getDefaultConstructor() const;
  virtual Sm::GlobalDatatype::SysTypeInfo *getFundamentalDatatypeConverters() const { return 0; }
  virtual Sm::SqlExpr* getSqlExprPtr() const { return 0; }
  virtual Sm::Subquery *getSelectQuery();
  virtual UserContext* userContext() const;
  virtual size_t arglistSize() const { return std::numeric_limits<size_t>::max(); }
  virtual void getStatementsWithEntity(ResolvedEntity* ent, Sm::StatementInterface *ownerStmt, FoundedStatements &outList);
  virtual bool hasDynamicQueryExpr() const { return false; }

  virtual bool beginedFrom(uint32_t, uint32_t) const { return false; }
  virtual CLoc getLLoc() const { return emptyFilelocation; }

  virtual void translateAssign(Sm::Codestream &, Sm::Type::RefInfo *, Sm::PlExpr *);
  virtual void translateVariableType(Sm::Codestream &, Sm::ResolvedEntity *, bool /*addTypeName*/ = false);
  virtual void translateObjRef(Sm::Codestream &, Sm::Type::RefInfo *);
  virtual void translateLocalObjects(Sm::Codestream &);
  virtual void translateAsCursor(Sm::Codestream &/*str*/) {}
  virtual void translateExternalVariablesToCallarglist(Sm::Codestream &/*str*/, Ptr<Sm::CallArgList> /*callarglist*/) {}
  virtual std::string getObjectTempTable(Sm::ResolvedEntity *);

  virtual void translatedNames(std::vector<std::string> **) {}

  virtual void translatedName(const std::string &v);
  virtual std::string translatedName() const;

  
  virtual void linterDefinitionKeys(Sm::Codestream &);
  virtual void extractExternalVariables() {}
  bool translateAsUserFunctionCall(Sm::Codestream &str, Sm::IdEntitySmart &reference, bool isNot, bool ignoreChecks = false, ResolvedEntity **tempVar = NULL);
  Ptr<Sm::Variable> addVariableIntoOwnerBlock(Sm::Datatype *varT, std::string *dstDeclVarName = 0, CLoc loc = cl::emptyFLocation(), std::string baseName = "tmp", bool addFirstNumber = true, bool notPushBackToDeclarations = false);
  Ptr<Sm::Variable> getTemporaryVar(Sm::Codestream &str, Datatype *varT, std::string *dstDeclVarName, bool dynamic = false);
  void checkToGrantOtherUser(Privs priv, Sm::Codestream &str);

  virtual Sm::NameTranslator        nameTR()        const { return 0; }
  virtual void                      nameTR       (Sm::NameTranslator       ) {}

  virtual Sm::CallarglistTranslator* callarglistTR() const { return 0; }
  virtual void                       callarglistTR(Sm::CallarglistTranslator*) {}

  virtual bool                      interTranslateCallArg(Sm::Codestream &, Ptr<Sm::Id>, Ptr<Sm::CallArgList>) { return false; }

  bool containsInSystemContext() const;

  virtual Sm::Id *getSelectedFieldName() const { throw 999; return 0; }
  virtual Ptr<Sm::Id>  getName () const;
  virtual Ptr<Sm::Id>  getAlias() const;
  virtual Ptr<Sm::Id2> getName2() const;

  virtual bool getFieldRefByFromList(Ptr<Sm::Id> &/*field*/, Sm::SemanticTree */*reference*/) { return false; }
  virtual bool getFieldRefByFromListFromRootQuery(Ptr<Sm::Id> &/*field*/, Sm::SemanticTree */*reference*/) { return false; }
  virtual bool getFieldRefFromRootQuery(Ptr<Sm::Id> &/*field*/) { return false; }
  virtual bool getFieldRefInArglist(Ptr<Sm::Id> &) { return false; }
  virtual bool getFieldRef         (Ptr<Sm::Id> &field);
  virtual bool getFields           (EntityFields &fields) const;
  virtual bool getFieldsExp        (EntityFields &fields, bool isProc, const std::string &baseName = std::string()) const;
  virtual bool findInFactoringList(Ptr<Sm::Id> &/*factoringTable*/) const { return false; }

  void oracleDefinition(Sm::Codestream &str);
  std::string ddlCathegoryToString() const;

  virtual ~ResolvedEntity();

  virtual ScopedEntities ddlCathegory() const = 0;

  virtual ScopedEntities ddlCathegoryWithDatatypeSpec() const { return ddlCathegory(); }

  void outputUnimplementedMethod(const std::string &methodName) const;

  ResolvedEntity* unwrapReference() const;

  bool isSqlCode() const;

  // Formal translations
  virtual void formalTranslateArgs(Ptr<Sm::Id> /*call*/) {;}
};



inline bool VEntities::eqByVEntities(const ResolvedEntity *def1, const ResolvedEntity *def2) {
  return (def1 && def2) &&
      (def1->vEntity() == def2->vEntity() ||
       def1->getDefinitionFirst()->vEntity() == def2->getDefinitionFirst()->vEntity());
}

inline ResolvedEntity* ResolvedEntity::getDefinitionFirst() const {
  if (VEntities *vEnt = vEntities())
    if (ResolvedEntity *d = vEnt->getDefinitionFirst((ResolvedEntity*)this))
      return d;
  return (ResolvedEntity*)this;
}


class ResolvedEntitySNode: public virtual ResolvedEntity {
  typedef Sm::SemanticTree* (ResolvedEntitySNode::*ToSTreeAction)() const;


private:
  static ToSTreeAction toSTreeAction;
public:
  Sm::SemanticTree *toSTreeGenerateStage() const;
  Sm::SemanticTree *toSTreeTransformStage() const;
  static std::stack<ToSTreeAction> toSTreeActionStack;

  static void pushTransformStage();
  static void pushGenerateStage();
  static void popStage();

  ResolvedEntitySNode() {}

  virtual Sm::SemanticTree *toSTreeBase() const = 0;
  Sm::SemanticTree *toSTree() const {
    return (this->*ResolvedEntitySNode::toSTreeAction)();
  }
};



std::string toString(ResolvedEntity::ScopedEntities t);
inline Codestream& operator<<(Codestream &str, ResolvedEntity &ent) { ent.translate(str); return str; }
inline Codestream& operator<<(Codestream& s, ResolvedEntity *obj) { return obj ? obj->translate(s) : s; }




inline ResolvedEntity *VEntities::getDefinitionFirst(ResolvedEntity *base) const {
  if (base && base->isFunction()) {
    VEntities::GlobalOverloadedNodes::const_iterator it = overloadedNodes.find(base->overloadedId());
    if (it != overloadedNodes.end())
      return it->second->key;
    return 0;
  }
  else if (variables)
    return variables->key;
  else if (others.size())
    return *(others.begin());
  return 0;
}

class ResolvedEntityLoc : public virtual Sm::GrammarBase, public virtual ResolvedEntity {
public:
  ResolvedEntityLoc() {}
  ResolvedEntityLoc(Sm::CLoc l) : Sm::GrammarBase(l) {}
  bool beginedFrom(uint32_t line, uint32_t column) const { return GrammarBase::beginedFrom(line, column); }
  bool beginedFrom(uint32_t line) const { return GrammarBase::beginedFrom(line); }
  CLoc getLLoc() const { return GrammarBase::getLLoc(); }
};

class ResolvedEntitySNodeLoc : public virtual Sm::ResolvedEntityLoc, public virtual ResolvedEntitySNode {
public:
  ResolvedEntitySNodeLoc() {}
  ResolvedEntitySNodeLoc(Sm::CLoc l) : GrammarBase(l) {}
};



class Unwrapper {
protected:
  void updateCounters(ResolvedEntity **unref, ResolvedEntity **curr) const;

  Datatype *unwrapSingleFieldStructure(ResolvedEntity *structType) const;

  virtual Datatype* unwrapStructuredType(ResolvedEntity *structType) const = 0;
  virtual Datatype* unwrapSubquery(ResolvedEntity *structType) const       = 0;

  Ptr<Datatype> unwrapStruct(ResolvedEntity *structType) const;

public:
  Datatype* unwrapDatatype(const ResolvedEntity *curr) const;
  Datatype* unwrapDatatype(const Ptr<Datatype> &curr) const;

  virtual ~Unwrapper() {}
};


// разворачивать запросы из одного поля, разворачивать структуры из одного поля
class StructureSubqueryUnwrapper : public Unwrapper {
  virtual Datatype* unwrapStructuredType(ResolvedEntity *structType) const;
  virtual Datatype* unwrapSubquery      (ResolvedEntity *structType) const;
public:
  static Datatype* unwrap(ResolvedEntity *structType);
  static Datatype* unwrap(const Ptr<Datatype> &t);

  StructureSubqueryUnwrapper() {}
};


// не разворачивать запросы из одного поля, не разворачивать структуры из одного поля
class SyntaxUnwrapper : public Unwrapper {
  virtual Datatype* unwrapStructuredType(ResolvedEntity *structType) const;
  virtual Datatype* unwrapSubquery      (ResolvedEntity *structType) const;
public:
  static Datatype* unwrap(ResolvedEntity *structType);
  static Datatype* unwrap(const Ptr<Datatype> &t);

  SyntaxUnwrapper() {}
};


// разворачивать запросы из одного поля, не разворачивать структуры из одного поля
class SubqueryUnwrapper : public Unwrapper {
  virtual Datatype* unwrapStructuredType(ResolvedEntity *structType) const;
  virtual Datatype* unwrapSubquery      (ResolvedEntity *structType) const;
public:
  static Datatype* unwrap(const ResolvedEntity *structType);
  static Datatype* unwrap(const Ptr<Datatype> &t);

  SubqueryUnwrapper() {}
};



class rRef {
public:
  const ResolvedEntity *ref;

  rRef(const ResolvedEntity *r)
    : ref(r) {}
};


} // Sm


bool operator==(const Sm::rRef &a, const Sm::rRef &b);

bool compareFields(Sm::EntityFields &a, Sm::EntityFields &b, bool isPlContext, bool inSqlCode);

#endif // RESOLVED_ENTITY_H
