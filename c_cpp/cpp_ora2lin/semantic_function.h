#ifndef SEMANTIC_FUNCTION_H
#define SEMANTIC_FUNCTION_H

#include <stack>
#include <functional>
#include "semantic_datatype.h"

namespace Sm {
  class FieldDefinition;
  class VariableField;


#define FLAG_VARIABLE_NOT_NULL               (1 << 0)
#define FLAG_VARIABLE_TEMPORARY              (1 << 1)
#define FLAG_VARIABLE_CONSTANT               (1 << 1)
#define FLAG_VARIABLE_GLOBAL                 (1 << 2)
#define FLAG_VARIABLE_DYNAMIC_LOOP_COUNTER   (1 << 3)
#define FLAG_VARIABLE_FIELD_FOR_MAKESTR      (1 << 4)
#define FLAG_VARIABLE_CURSOR_CALLS_RESOLVED  (1 << 5)
#define FLAG_VARIABLE_TEMP_USED              (1 << 6)

union VariableFlags {
  unsigned int v;
  struct Flags {
    unsigned int isNotNull            :1;
    unsigned int isTemporary          :1;
    unsigned int isConstant           :1;
    unsigned int isGlobal             :1;
    unsigned int isDynamicLoopCounter :1;
    unsigned int isFieldForMakestr    :1;
    unsigned int isCursorCallsResolved:1;
    unsigned int isUsed               :1;
  };

  VariableFlags() : v(0) {}

  void setNotNull            () { v |= FLAG_VARIABLE_NOT_NULL             ; }
  void setTemporary          () { v |= FLAG_VARIABLE_TEMPORARY            ; }
  void setConstant           () { v |= FLAG_VARIABLE_CONSTANT             ; }
  void setGlobal             () { v |= FLAG_VARIABLE_GLOBAL               ; }
  void setDynamicLoopCounter () { v |= FLAG_VARIABLE_DYNAMIC_LOOP_COUNTER ; }
  void setCursorCallsResolved() { v |= FLAG_VARIABLE_CURSOR_CALLS_RESOLVED; }
  void setUsed               () { v |= FLAG_VARIABLE_CURSOR_CALLS_RESOLVED; }

  void clrUsed               () { v &= ~FLAG_VARIABLE_CURSOR_CALLS_RESOLVED; }

  bool isNotNull            () const { return v & FLAG_VARIABLE_NOT_NULL             ; }
  bool isTemporary          () const { return v & FLAG_VARIABLE_TEMPORARY            ; }
  bool isConstant           () const { return v & FLAG_VARIABLE_CONSTANT             ; }
  bool isGlobal             () const { return v & FLAG_VARIABLE_GLOBAL               ; }
  bool isDynamicLoopCounter () const { return v & FLAG_VARIABLE_DYNAMIC_LOOP_COUNTER ; }
  bool isCursorCallsResolved() const { return v & FLAG_VARIABLE_CURSOR_CALLS_RESOLVED; }
  bool isUsed               () const { return v & FLAG_VARIABLE_CURSOR_CALLS_RESOLVED; }
};


class VariableCursorFields : public virtual CathegoriesOfDefinitions {
public:
  EntityFields fields_;
  const ResolvedEntity *fieldsSource = 0;
  VariableFlags flags;

protected:
  std::vector<Ptr<VariableField> > fieldsCursor;

  bool getFields(EntityFields &fields) const;

  bool buildCursorFields();
  bool getVariableFieldRef(Ptr<Id> &field);

  virtual Ptr<Id> getVarName() const = 0;
  virtual BlockPlSql *getOwnerBlk() = 0;

  virtual SemanticTree *getVariableSNode() const = 0;
  virtual Ptr<Datatype> getVariableDatatype() const = 0;
  virtual ResolvedEntity::ScopedEntities ddlCathegory() const = 0;
public:
  VariableCursorFields();

  virtual CLoc getLLoc() const = 0;
  bool checkConsistanseForOtherFieldList(Sm::Statement *stmt, bool outError = true);
  void initFieldsFromOpenFor(OpenFor *openFor, FoundedStatements::iterator it, Sm::FoundedStatements &stmts);
  bool translateRefCursorUsing();
  void setFieldsFrom(EntityFields &flds, const ResolvedEntity *_fieldsSource, string prefix = "");

  virtual Ptr<Datatype> getDatatype() const = 0;


  virtual ResolvedEntity *toSelfResolvedEntity() = 0;
  VariableCursorFields *toSelfVariableCursorFields() const { return const_cast<VariableCursorFields*>(this); }

  void getFieldsRecursive(EntityFields &fields) const;
};


class FunctionArgument            : public ResolvedEntitySNodeLoc, public VariableCursorFields {
public:
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>       name;
  Ptr<Datatype> datatype;
  Ptr<SqlExpr>  defaultValue_;
  function_argument::Direction direction;

  void definitionBase(Sm::Codestream &str);

  // VariableCursorFields interface
  SemanticTree *getVariableSNode() const { return getSemanticNode(); }
  Ptr<Datatype> getVariableDatatype() const { return getDatatype(); }
public:
  int positionInArglist = 0;
  BlockPlSql *getOwnerBlk() { return 0; }

  CLoc getLLoc() const { return GrammarBase::getLLoc(); }
  FunctionArgument(CLoc l, Ptr<Id> n, Ptr<Datatype> t, Ptr<SqlExpr> dflt = 0, function_argument::Direction dir = function_argument::IN);

  FunctionArgument(Ptr<Id> n, Ptr<Datatype> t, Ptr<SqlExpr> dflt = 0, function_argument::Direction dir = function_argument::IN);
  FunctionArgument(const string &n, Ptr<Datatype> t, Ptr<SqlExpr> dflt = 0, function_argument::Direction dir = function_argument::IN);

  Ptr<Id> getVarName() const { return name.object(); }
  void setDatatype(Ptr<Sm::Datatype> t) { datatype = t; }
  void setDatatypeForMember(Ptr<Sm::Id> name, Ptr<Sm::Datatype> t);

  bool out  () const { return isEntry(direction, function_argument::OUT); }
  bool in   () const { return isEntry(direction, function_argument::IN); }
  bool inout() const { return direction == function_argument::IN_OUT; }

  ScopedEntities ddlCathegory() const { return FunctionArgument_; }
  bool isDefinition() const { return true; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  bool isRefCursor() const { return datatype && datatype->isRefCursor(); }
  bool isField() const { return true; }
  bool isFunArgument() const { return true; }
  bool isLinterStructType() const;
  function_argument::Direction dir() const { return direction; }
  Ptr<SqlExpr> defaultValue() const { return defaultValue_; }
  
  ResolvedEntity *getFieldDDLContainer() const;

  Ptr<Datatype> getDatatype() const { return datatype; }
  Ptr<Id> getName() const { return name; }
  bool getFieldRef(Ptr<Sm::Id> &field);
  bool getFields(EntityFields &fields) const;
  ResolvedEntity* getNextDefinition() const { return datatype.object(); }
  
  bool hasDynamicDefaultValue() const;
  void addToInitializers();

  void oracleDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  void linterReference(Sm::Codestream &str) {
    if (!isTrNameEmpty())
      translateName(str);
    else
      str << name;
  }

  void collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }
  Sm::SemanticTree *toSTreeBase() const;
  virtual ~FunctionArgument();
  FunctionArgument* toSelfFunctionArgument() const { return const_cast<FunctionArgument*>(this); }
  VariableCursorFields *toSelfVariableCursorFields() const { return const_cast<FunctionArgument*>(this); }

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, datatype, defaultValue_); }

  ResolvedEntity *toSelfResolvedEntity() { return this; }

  virtual bool explicitCast();

  void checkFieldsConsistance(EntityFields &srcFields, const FLoc &srcLoc);
};

Codestream& operator<<(Codestream& s, FunctionArgument &obj);
typedef std::function<bool(FunctionArgument *)> const ArgActor;


class FunctionArgumentContainer {
public:
  std::string   name;
  Ptr<Datatype> t;
  Ptr<SqlExpr>  dflt;

  FunctionArgumentContainer();
  FunctionArgumentContainer(std::string n, Datatype *_t, SqlExpr *_dflt = 0);
  FunctionArgumentContainer(const FunctionArgumentContainer &o);
};


#define FLAG_FUNCTION_ALL_ARGS_DEFAULT             (1 << 0)
#define FLAG_FUNCTION_DATATYPE_CHANGED             (1 << 1)
#define FLAG_FUNCTION_RETTYPE_CONVERSION_ENTERED   (1 << 2)
#define FLAG_FUNCTION_ELEMENTARY_LINTER_FUNCTION   (1 << 3)
#define FLAG_FUNCTION_LINTER_QUERY_LITERAL         (1 << 4)
#define FLAG_FUNCTION_ANYDATA_MEMBER               (1 << 5)
#define FLAG_FUNCTION_XML_FUNCTION                 (1 << 6)
#define FLAG_FUNCTION_PIPELINED                    (1 << 7)
#define FLAG_FUNCTION_ALREADY_IN_CODESTREAM        (1 << 8)
#define FLAG_FUNCTION_EXTERNAL_VARIABLES_EXTRACTED (1 << 9)
#define FLAG_FUNCTION_DBMS_SQL_PARSE               (1 << 10)
#define FLAG_FUNCTION_DBMS_SQL_OPEN_CURSOR         (1 << 11)
#define FLAG_FUNCTION_DBMS_SQL_BIND_VARIABLE       (1 << 12)
#define FLAG_FUNCTION_DBMS_SQL_EXECUTE             (1 << 13)
#define FLAG_FUNCTION_DBMS_SQL_VARIABLE_VALUE      (1 << 14)
#define FLAG_FUNCTION_DBMS_SQL_CLOSECURSOR         (1 << 15)
#define FLAG_FUNCTION_SYS_TO_CHAR                  (1 << 16)

union FunctionFlags {
  unsigned int v;
  struct Flags {
    unsigned int allArgsIsDefault            :1; // = true;
    unsigned int datatypeIsChanged           :1; // = false;
    unsigned int rettypeConversionEntered    :1; // = false;
    unsigned int isElementaryLinterFunction  :1; // = false;
    unsigned int isLinterQueryLiteral        :1; // = false;
    unsigned int isAnydataMember             :1; // = false;
    unsigned int isXmlFunction               :1; // = false;
    unsigned int isPipelined                 :1; // = false;
    unsigned int isAlreadyInCodestream       :1; // = false;
    unsigned int isExternalVariablesExtracted:1; // = false;

    unsigned int isDbmsSqlParse         :1;
    unsigned int isDbmsSqlOpenCursor    :1;
    unsigned int isDbmsSqlBindVariable  :1;
    unsigned int isDbmsSqlExecute       :1;
    unsigned int isDbmsSqlVariableValue :1;
    unsigned int isDbmsSqlCloseCursor   :1;
  } f;

  FunctionFlags() : v(FLAG_FUNCTION_ALL_ARGS_DEFAULT) {}

  bool isAllArgsDefault()             const { return v & FLAG_FUNCTION_ALL_ARGS_DEFAULT            ; }
  bool isDatatypeChanged()            const { return v & FLAG_FUNCTION_DATATYPE_CHANGED            ; }
  bool isRettypeConversionEntered()   const { return v & FLAG_FUNCTION_RETTYPE_CONVERSION_ENTERED  ; }
  bool isElementaryLinterFunction()   const { return v & FLAG_FUNCTION_ELEMENTARY_LINTER_FUNCTION  ; }
  bool isLinterQueryLiteral()         const { return v & FLAG_FUNCTION_LINTER_QUERY_LITERAL        ; }
  bool isAnydataMember()              const { return v & FLAG_FUNCTION_ANYDATA_MEMBER              ; }
  bool isXmlFunction()                const { return v & FLAG_FUNCTION_XML_FUNCTION                ; }
  bool isPipelined()                  const { return v & FLAG_FUNCTION_PIPELINED                   ; }
  bool isAlreadyInCodestream()        const { return v & FLAG_FUNCTION_ALREADY_IN_CODESTREAM       ; }
  bool isExternalVariablesExtracted() const { return v & FLAG_FUNCTION_EXTERNAL_VARIABLES_EXTRACTED; }

  bool isDbmsSqlParse()         const { return v & FLAG_FUNCTION_DBMS_SQL_PARSE         ; }
  bool isDbmsSqlOpenCursor()    const { return v & FLAG_FUNCTION_DBMS_SQL_OPEN_CURSOR   ; }
  bool isDbmsSqlBindVariable()  const { return v & FLAG_FUNCTION_DBMS_SQL_BIND_VARIABLE ; }
  bool isDbmsSqlExecute()       const { return v & FLAG_FUNCTION_DBMS_SQL_EXECUTE       ; }
  bool isDbmsSqlVariableValue() const { return v & FLAG_FUNCTION_DBMS_SQL_VARIABLE_VALUE; }
  bool isDbmsSqlCloseCursor()   const { return v & FLAG_FUNCTION_DBMS_SQL_CLOSECURSOR   ; }
  bool isDbmsSqlInterface()     const { return v & (FLAG_FUNCTION_DBMS_SQL_PARSE          |
                                                    FLAG_FUNCTION_DBMS_SQL_OPEN_CURSOR    |
                                                    FLAG_FUNCTION_DBMS_SQL_BIND_VARIABLE  |
                                                    FLAG_FUNCTION_DBMS_SQL_EXECUTE        |
                                                    FLAG_FUNCTION_DBMS_SQL_VARIABLE_VALUE |
                                                    FLAG_FUNCTION_DBMS_SQL_CLOSECURSOR);
                                      }

  bool isSysToChar() const            { return v & FLAG_FUNCTION_SYS_TO_CHAR; }

  void clrAllArgsDefault()             { v &= ~FLAG_FUNCTION_ALL_ARGS_DEFAULT           ; }
  void setDatatypeChanged()            { v |= FLAG_FUNCTION_DATATYPE_CHANGED            ; }
  void setRettypeConversionEntered()   { v |= FLAG_FUNCTION_RETTYPE_CONVERSION_ENTERED  ; }
  void setElementaryLinterFunction()   { v |= FLAG_FUNCTION_ELEMENTARY_LINTER_FUNCTION  ; }
  void setLinterQueryLiteral()         { v |= FLAG_FUNCTION_LINTER_QUERY_LITERAL        ; }
  void setAnydataMember()              { v |= FLAG_FUNCTION_ANYDATA_MEMBER              ; }
  void setXmlFunction()                { v |= FLAG_FUNCTION_XML_FUNCTION                ; }
  void setPipelined()                  { v |= FLAG_FUNCTION_PIPELINED                   ; }
  void setAlreadyInCodestream()        { v |= FLAG_FUNCTION_ALREADY_IN_CODESTREAM       ; }
  void setExternalVariablesExtracted() { v |= FLAG_FUNCTION_EXTERNAL_VARIABLES_EXTRACTED; }

  void setDbmsSqlParse()               { v |= FLAG_FUNCTION_DBMS_SQL_PARSE         ; }
  void setDbmsSqlOpenCursor()          { v |= FLAG_FUNCTION_DBMS_SQL_OPEN_CURSOR   ; }
  void setDbmsSqlBindVariable()        { v |= FLAG_FUNCTION_DBMS_SQL_BIND_VARIABLE ; }
  void setDbmsSqlExecute()             { v |= FLAG_FUNCTION_DBMS_SQL_EXECUTE       ; }
  void setDbmsSqlVariableValue()       { v |= FLAG_FUNCTION_DBMS_SQL_VARIABLE_VALUE; }
  void setDbmsSqlCloseCursor()         { v |= FLAG_FUNCTION_DBMS_SQL_CLOSECURSOR   ; }

  void setSysToChar()                  { v |= FLAG_FUNCTION_SYS_TO_CHAR            ; }
  void clrElementaryLinterFunction()   { v &= ~FLAG_FUNCTION_ELEMENTARY_LINTER_FUNCTION  ; }
};

class FunctionContext {
public:
  typedef std::map<string, ConstructExprStmt*> UnionDynamicComponents;
  UnionDynamicComponents unionDynamicComponents;

  void swap(FunctionContext &oth);
};

class Function : public Declaration, public VariableCursorFields {
public:
  Ptr<Sm::CallarglistTranslator> callarglistTranslator;
  Sm::NameTranslator             nameTranslator = 0;
  Sm::ExternalVariables          externalVariables;

  size_t getGlobalFunId() const;
//  size_t __funId = getGlobalFunId();
protected:
  Ptr<Id2>        name;
  Ptr<Arglist>    arglist;
  Ptr<Datatype>   rettype; // 0 => procedure

public:
  Ptr<BlockPlSql> body_;

  FunctionContext metaContext;

  // Если здесь лежит более одного элемента - выводить предупреждение, что эту ситуацию нужно переписывать руками
  OutRefCursors outRefCursors;

  Ptr<Datatype>   reducedRettype;
  Sm::CastCathegory reducedCathegory;

  FunctionFlags flags;

  Ptr<Variable> pipeVar;
  Ptr<Variable> pipeIndex;
protected:

  int overloadedId_ = -1;
  int actionNumber_ = 0;
  int branchId_     = 0;

  SemanticTree *getVariableSNode()    const { return getSemanticNode(); }
  Ptr<Datatype> getVariableDatatype() const { return getDatatype(); }
  Ptr<Id> getVarName() const { return getName(); }
  BlockPlSql *getOwnerBlk();

  ResolvedEntity *owner() const { return Declaration::owner(); }
  bool setOwnerBlockPlSql(BlockPlSql *b);

  ResolvedEntity *toSelfResolvedEntity() { return this; }

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void updateDefinition();
  void callarglistTR(Sm::CallarglistTranslator *tr);
  void nameTR       (Sm::NameTranslator         tr);


  Ptr<LinterCursor> tranlateAllOpenRefCursorLayer(Sm::OutRefCursors::value_type::first_type refCursorVar, Ptr<BlockPlSql> b, std::vector<StatementInterface*> &layer);
  Ptr<LinterCursor> generateLinterCursorVar(Ptr<BlockPlSql> b, std::vector<StatementInterface*>::iterator &pos, std::string varName = "genCurVar");
  Ptr<LinterCursor> tranlateRefCursorLayer(
    Sm::OutRefCursors::value_type::first_type refCursorVar,
    Ptr<BlockPlSql> b,
    std::vector<StatementInterface*> &layer,
    std::vector<StatementInterface*>::iterator &pos);
  Ptr<LinterCursor> generateLinterCursor(Ptr<SqlExpr> query, Ptr<BlockPlSql> b, std::string varName);

  virtual void translateSelfRef(Sm::Codestream &);

public:
  UserContext* userContext() const;
  CLoc getLLoc() const { return GrammarBase::getLLoc(); }

  void extractExternalVariables();
  int countArglistRefcursorFiels();

  void getQueriesForRefCursor(int posInArglist, QueriesContainer &queries);

  Sm::Function *castToFunction() const { return (Function*)this; }
  bool equallyByArglists(ResolvedEntity *oth) const;
  Sm::BlockPlSql *childCodeBlock() const;

  Sm::BlockPlSql* body() const;
  void getFieldsFromReturnRefcursor(VariableCursorFields *flds, const ResolvedEntity **sourceFields);
  void getFieldsFromOutRefcursor(VariableCursorFields *flds, unsigned int pos, const ResolvedEntity **sourceFields);

  virtual bool isLinterQueryLiteral() const { return flags.isLinterQueryLiteral(); }
  virtual bool isElementaryLinterFunction() const { return flags.isElementaryLinterFunction(); }

  void translateOutCursorInArglist();
  void setElementaryLinFunc() { flags.setElementaryLinterFunction(); }
  void clrElementaryLinFunc() { flags.clrElementaryLinterFunction(); }

  void translateExternalVariablesToCallarglist(Sm::Codestream &str, Ptr<CallArgList> callarglist);
  void setRetType(Ptr<Sm::Datatype> t);

  bool isPipelined() const { return flags.isPipelined(); }
  void setPipelined(unsigned int flg) { flags.v |= flg; }

  void collectInitializers(EntitiesSet &container);

  std::string translatedName() const;
  void translatedName(const std::string &v);
  void translateName(Codestream &str);

  void translateLocalObjects(Sm::Codestream &str);

  void identificateOverloaded();

  ResolvedEntity *ownerPackage() const;

  bool isExactlyEquallyByDatatype(ResolvedEntity *oth);

  Ptr<Sm::BlockPlSql> funBody() const;
  void resolve(ModelContext &) {}
  void resolve(Ptr<Id> owner, Ptr<ResolvedEntity> pOwner);
  bool semanticResolve() const;
  bool isFunction() const { return true; }
  void addArgument(Ptr<FunctionArgument> arg);

  void traverseDeclarationsForce(DeclActor &fun);
  void traverseDeclarations(DeclActor &fun);
  void traverseModelStatements(StatementActor &fun);
  void traverseArguments(ArgActor &fun);
  void replaceStatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  Sm::ResolvedEntity *ownerFunction() const { return const_cast<Function*>(this); }

  Sm::CallarglistTranslator *callarglistTR() const;
  Sm::NameTranslator         nameTR() const;
  Ptr<Arglist> funArglist() const;

  void linterReference(Sm::Codestream &s);
  void linterDeclaration(Sm::Codestream &str);
  void oracleDeclaration(Sm::Codestream &str);
  void sqlDeclaration(Codestream &str);
  void sqlDefinition(Codestream &str);
  void sqlHeader(Codestream &str);

  Ptr<Id> getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val);
  bool allArgsIsDefault() const { return flags.isAllArgsDefault(); }

  int  overloadedId() const { return overloadedId_; }
  void overloadedId(int v) { overloadedId_ = v; }

  bool isProcedure() const;

  Ptr<Arglist> getArglist() const;
  Vector<FunctionArgument>::iterator arglistBegin();
  Vector<FunctionArgument>::iterator arglistEnd  ();
  size_t arglistSize() const { return arglist ? arglist->size() : 0; }
  bool   arglistEmpty() const { return arglist ? arglist->empty() : true; }

  ScopedEntities ddlCathegory() const { return Function_; }
  bool isDefinition() const;
  Ptr<Datatype> getDatatype() const;
  Datatype* getRettype() const;
  bool getFieldRef(Ptr<Id> &field);
  bool getFieldRefInArglist(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;
  Sm::Function* toSelfFunction() const { return (Function*)this; }

  void checkAllArgsIsDefault();

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  void incBranchId() { ++branchId_; }
  int  branchId() const { return branchId_; }

  Function(const Function &oth);
  Function(const Ptr<Id2> &n = 0, const Ptr<Arglist> &args = 0, const Ptr<Datatype> &rT = 0,  const Ptr<BlockPlSql> &b = 0);
  Function(CLoc l, Ptr<Id2> n = 0, Ptr<Arglist> args = 0, Ptr<Datatype> rT = 0,  Ptr<BlockPlSql> b = 0);
  Function(const string &name, Ptr<Arglist> args, Ptr<Datatype> rT = 0, bool isElementaryLinFun = false);
  Function(const string &name, std::initializer_list<FunctionArgumentContainer> args, Ptr<Datatype> rT = 0, bool isElementaryLinFun = false);

  Sm::SemanticTree *toSTreeBase() const;
  void translateArglist(Codestream &str);
  void translateRettype(Codestream &str);
  void inferenceReducedRettype();
  void castReturnExpressionsToRettype();
  void castFuncArgDefaultValue();

  void debugOutput(Ptr<Datatype> minimalCommonDatatype);

  //Formal translations
  bool castArgIfNeed(Ptr<Sm::FunCallArg> arg, Ptr<Datatype> needType);
  void formalTranslateArgs(Ptr<Sm::Id> call);

protected:
  void getFieldsFromStmtsBeforeOp(VariableCursorFields  *flds,
                                  ResolvedEntity        *var,
                                  StatementsTree        &statementsTree,
                                  StatementsTree        *op,
                                  const ResolvedEntity **sourceFields);
};

class LoopBounds;


class Variable : public Declaration, public VariableCursorFields {
  static size_t __getGlobalVarId();
//public:
//  size_t __globalVarId = __getGlobalVarId();
//private:

  mutable SemanticTree *datatypeNode = 0;

  Ptr<Variable> refCursorString;
  Ptr<Variable> refCursorRowId;

  Ptr<Id>       name;
  Ptr<Datatype> datatype;
  Ptr<PlExpr>   defaultValue_;

  int anydataCode_ = std::numeric_limits<int>::min();
public:

  ResolvedEntity *baseField = 0;
private:
  BlockPlSql *ownerBlock_ = 0;


protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  // VariableCursorFields interface
  SemanticTree *getVariableSNode() const { return getSemanticNode(); }
  Ptr<Datatype> getVariableDatatype() const { return getDatatype(); }
  void translateName(Codestream &str);

public:
  ResolvedEntity *owner() const { return Declaration::owner(); }
  CLoc getLLoc() const { return GrammarBase::getLLoc(); }

  void anydataCode(int code) { anydataCode_ = code; }
  bool isAnydataCode() const { return anydataCode_ != std::numeric_limits<int>::min(); }
  bool isTemporary() const { return flags.isTemporary(); }
  bool isUsed() const { return flags.isUsed(); }

  void use() { flags.setUsed(); }
  void unuse() { flags.clrUsed(); }

  bool hasLinterEquivalent() const { return true; }
  void setDatatypeForMember(Ptr<Sm::Id> name, Ptr<Sm::Datatype> t);
  bool setOwnerBlockPlSql(BlockPlSql *b) { ownerBlock_ = b; return true; }

  Sm::BlockPlSql* ownerPlBlock() const;

  bool isSqlIdentificator() const { return true; }

  void setDatatype(Ptr<Sm::Datatype> t);

  bool isVariable() const { return !isAnydataCode() && baseField == NULL; }
  bool isCursorVariable() const;
  bool isTempVariable() const { return isTemporary(); }
  bool isLinterStructType() const;

  bool hasDynamicDefaultValue() const;
  bool hasObjectType() const;
  Type::ObjectType *getObjectDatatype() const;

  void userInitializer(Sm::Codestream &str, bool &somethingOutputted);

  Ptr<PlExpr> defaultValue() const { return defaultValue_.object(); }
  void setDefaultValue(Ptr<PlExpr> expr) { defaultValue_  = expr; }

  UserContext* userContext() const { return Declaration::userContext(); }
  ResolvedEntity *ownerPackage() const { return Declaration::ownerPackage(); }

  bool isPackageVariable() const;

  ScopedEntities ddlCathegory() const { return baseField ? FieldOfVariable_ : Variable_; }
  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const { return datatype;  }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  bool isField() const { return true; }
  bool getFieldRef(Ptr<Sm::Id> &field);

  Ptr<Id> getVarName() const { return name.object(); }
  BlockPlSql *getOwnerBlk() { return getOwnerBlock(); }

  ResolvedEntity *getFieldDDLContainer() const { return isPackageVariable() ? const_cast<Variable*>(this) : (Variable*)0; }

  void generateUniqueName();
  void outAsForLoopVariable(Sm::Codestream &str, Ptr<LoopBounds> bounds, bool isReverse, BlockPlSql *ownerBlock);
  void linterDefinition(Sm::Codestream &str);
  void linterReference (Sm::Codestream &str);
  void oracleDefinition(Sm::Codestream &str);

  void collectInitializedEntities(Definitions &defs);

  Variable(Id* _name, Ptr<Datatype> _datatype, bool _notNull = false, Ptr<PlExpr> _defaultValue = 0, bool _isConstant = false, CLoc l = cl::emptyFLocation());

  void collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }
  SemanticTree *toSTreeBase() const;

  ResolvedEntity* getNextDefinition() const { return datatype.object(); }
  bool getFields(EntityFields &fields) const;

  Ptr<Id> getName() const { return name; }
  Variable *toSelfVariable() const { return const_cast<Variable*>(this); }
  VariableCursorFields *toSelfVariableCursorFields() const { return const_cast<Variable*>(this); }

  void definitionRecordExpandedAsFields(Sm::Codestream &str);
  void definitionPackageVariable(ResolvedEntity *baseType, Sm::Codestream &str);
  void definitionCursor(Sm::Codestream &str);
  void definitionRefCursor(Sm::Codestream &str);


  ResolvedEntity *toSelfResolvedEntity() { return this; }

  void traverseDeclarations(DeclActor &fun);
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, defaultValue_); }
  void traverseModelStatements(StatementActor &) {}
  BlockPlSql * getOwnerBlock();

  virtual bool explicitCast();


  void setOpenCursorCommand(Sm::OpenCursor *cmd);
  void clrOpenCursorCommand();
  Sm::Subquery *getSelectQuery();

};


class VariableField : public Variable {
public:
  Sm::VariableCursorFields *ownerVariable = 0;

  ResolvedEntity *sourceQuery = 0;
  bool isStructuredField = false;

  Sm::VariableCursorFields *getTopOwnerVariable();

  VariableField(Id* _name, ResolvedEntity* srcQuery);

  Ptr<Datatype> getDatatype() const { return sourceQuery->getDatatype();  }
  bool isPackageVariable() const { return false; }

  void linterDefinition(Sm::Codestream &str);
  void linterReference (Sm::Codestream &str);
  VariableField *toSelfVariableField() const { return const_cast<VariableField*>(this); }
};

class VarHelper  {
public:
  VarHelper();
  ~VarHelper() {;}
  static VarHelper &getInstance();

  void add(Variable *var);
  void unuseAll();

private:
  Vector<Variable> tmpVars;
};


namespace Type {

class MemberFunction         : public MemberInterface, public Function {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  std::string prefix();
  virtual void translateSelfRef(Sm::Codestream &str);
  void addReturnSelf();
  void addNestedCtors();
public:
  Inheritance    ::T              inheritance;
  member_function::Specificators  specificators;
  /// Тип данных, к которому принадлежит конструктор
  Ptr<Datatype>  selfConstructorDatatype; // не резолвится, т.к не нужен

  MemberFunction(CLoc l,
                 Inheritance::T _inheritance,
                 member_function::Specificators  _specificators,
                 Ptr<Id2>        n    = 0,
                 Ptr<Arglist>    args = 0,
                 Ptr<Datatype>   rT   = 0,
                 Ptr<BlockPlSql> b    = 0);

  MemberFunction(CLoc l,
                 Inheritance::T  _inheritance,
                 Ptr<Arglist>    args = 0,
                 Ptr<Datatype>   rT   = 0,
                 Ptr<BlockPlSql> b    = 0,
                 Ptr<Datatype>   _selfConstructorDatatype = 0);

  bool isNonblockPseudoField() const { return true; }
  void collectSNodeM(SemanticTree *n) const;
  bool interTranslateCallArg(Sm::Codestream &str, Ptr<Sm::Id> id, Ptr<Sm::CallArgList> callArgList);

  ScopedEntities  ddlCathegory()         const { return MemberFunction_; }
  MemberFunction* toSelfMemberFunction() const { return (MemberFunction*)this; }

  CathegoryMember cathegoryMember() const { return MemberInterface::FUNCTION; }
  void resolve(Ptr<Id>);
  void oracleDeclaration(Sm::Codestream &str);
  void sqlDefinition(Codestream &str);
  void linterDefinition(Codestream &str);

  Ptr<ResolvedEntity> getThisDefinition() const { return (Function*)this; }
  Ptr<Id> getMemberName() const;

  bool isConstructor()  const { return isEntry(specificators, member_function::CONSTRUCTOR); }
  bool isConstructorM() const { return isConstructor(); }
  bool isMemberFunctionMember() const { return isEntry(specificators, member_function::MEMBER); }
  bool isMemberFunctionStatic() const { return isEntry(specificators, member_function::STATIC); }
  bool isMemberFunctionConstructor() const { return isEntry(specificators, member_function::CONSTRUCTOR); }
  bool isMethod() const { return true; }

  UserContext *userContext() const;
  void makeFormalTranslations();
};

class ArrayConstructor : public Function {
  ObjectType *owner;
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  ArrayConstructor(ObjectType *_owner, Ptr<Id> ownerName, Ptr<Datatype> ownerDatatype);

  ScopedEntities ddlCathegory() const { return ArrayConstructor_; }
  SemanticTree *toSTreeBase() const { return new SemanticTree(getName(), SemanticTree::DECLARATION, SCathegory::ArrayConstructor); }
  bool isConstructor() const { return true; }
  ObjectType *getOwnerCollection() { return owner; }

  Sm::Type::ArrayConstructor *toSelfArrayConstructor() const { return const_cast<ArrayConstructor*>(this); }
};

}

class VariableUndeclaredIndex;

class WhenExpr                : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef BaseList<StatementInterface> Statements;
  Ptr<PlExpr>     condition; // 0 => OTHERS
  Statements      branchStatements;
  enum Cathegory {
    EXCEPTION_HANDLER,
    IF_FIRST_STATEMENT,
    ELSEIF_STATEMENT,
    ELSE_STATEMENT,
    WHEN_THEN,
    WHEN_ALL,
    WHEN_OTHERS
  };
  struct Then {};

  Cathegory cathegory = WHEN_THEN;

  bool outStatements_ = true;

  WhenExpr(CLoc l, Ptr<Statements> stmts, Ptr<PlExpr> cond, Cathegory _cathegory);
  WhenExpr(Ptr<StatementInterface> stmt, Ptr<PlExpr> cond, Cathegory _cathegory);
  WhenExpr(Ptr<Statements> stmts, Ptr<PlExpr> cond, Cathegory _cathegory);

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Codestream &str);
  void translateBody(Codestream &str);
  void translateLinterExceptionDeclarations(Codestream &str, DeclaredExceptions &declExc);
  virtual ~WhenExpr();
  void replaceChildsIf(Sm::ExprTr tr);

  void traverseDeclarationsStmt(DeclActor &fun);
  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void traverseStatements(StatementActor &fun);
  void buildStatementsTree(StatementsTree &parent);

  void translate(Codestream &str) { Translator::translate(str); }

  void checkCondition();
  Statements *getChildStatements() { return &branchStatements; }
  bool needSemicolon() { return false; }

  WhenExpr *toSelfWhenExpr() const { return const_cast<WhenExpr*>(this); }
  ScopedEntities ddlCathegory() const { return WhenExpr_; }
};


class LinterCursor : public Declaration {
protected:
  mutable Ptr<Datatype> thisDatatype_;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>      name;
  Ptr<SqlExpr> query;
public:
  LinterCursor(string &&_name, Ptr<SqlExpr> _query);
  ScopedEntities ddlCathegory() const { return LinterCursor_; }
  SemanticTree *toSTreeBase() const { return 0; }
  Ptr<Id> getName() const;
  Ptr<Datatype> getDatatype() const;
  bool getFields(EntityFields &fields) const;
  void translateAsCursor(Sm::Codestream &str);
  void linterDefinition (Sm::Codestream &str);
  void linterReference  (Sm::Codestream &str);

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, query); }
  void traverseModelStatements(Sm::StatementActor &) {}
};

class Loop                    : public Statement  {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Statements      loopStatements;
  Ptr<Id>         endLabelId;
  bool endloopLabel;
  ResolvedEntity* getNextDefinition() const { return 0; }
  ScopedEntities ddlCathegory() const { return Loop_; }
  Ptr<Datatype> getDatatype() const;
  Loop(CLoc l, Ptr<Statements> stmts, Ptr<Id> endLblId);
  void outEndLoopLabel(Codestream &str);
  void collectSNode(SemanticTree *n) const;
  bool needSemicolon() { return false; }
  void linterDefinition(Codestream &str);
  void outStatements(Codestream &str, bool conditionNotOutput = true);

  void traverseDeclarationsStmt(DeclActor &fun);
  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun, loopStatements); }
  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, loopStatements, endLabelId); }

  Statements *getChildStatements() { return &loopStatements; }

  Loop* toSelfLoop() const { return const_cast<Loop*>(this); }
};

class LValue : public RefExpr {
public:
  inline bool isHost() const { return __flags__.v & FLAG_LVALUE_IS_HOST; }
  LValue(CLoc l, Ptr<IdEntitySmart> _lEntity);      // LOCAL
  LValue(CLoc l, Ptr<IdEntitySmart> _lEntity, int); // HOST
  LValue(CLoc l, Id *name);

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;

  bool isVariable() const;
  void linterDefinition(Codestream &str);
  void linterReference(Sm::Codestream &str);

  virtual ~LValue();
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, reference); }

  SmartVoidType* getThisPtr() const { return reinterpret_cast<SmartVoidType*>(const_cast<LValue*>(this)); }
  Ptr<Sm::Datatype> getDatatype()  const;
  ScopedEntities ddlCathegory() const { return LValue_; }
  LValue* toSelfLValue() const { return const_cast<LValue*>(this); }
  Ptr<IdEntitySmart> lEntity() const { return reference; }


  virtual void translateInversedForm(Sm::Codestream &str);
};

struct LValueIntoList : public LValue {
  EntityFields fields;
  void linterDefinition(Codestream &str);
  LValueIntoList(CLoc l, Ptr<IdEntitySmart> _lEntity);
  void translateInversedForm(Sm::Codestream &/*str*/) { throw 999; }
};

Sm::Codestream& operator <<(Sm::Codestream &os, const Sm::WhenExpr::Then&);

}


#endif // SEMANTIC_FUNCTION_H
