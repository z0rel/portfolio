#ifndef DYNAMIC_SQL_OP
#define DYNAMIC_SQL_OP

#include "semantic_flags.h"
#include "semantic_function.h"
#include "semantic_statements.h"
#include "semantic_expr_select.h"

namespace Sm {

class DynTailExpr : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> expr;

  Ptr<PlExpr> tail;
  bool inverse = false;

  DynTailExpr(CLoc l, PlExpr* _expr, PlExpr* _tail, bool _inverse);

  Ptr<Datatype> getDatatype() const;
  bool getFields(EntityFields &fields) const { return expr->getFields(fields); }
  bool getFieldRef(Ptr<Id> &f)               { return expr->getFieldRef(f);    }

  // linterDefinition вызывается когда генерируется аргумент в вызове makestr
  void linterDefinition(Sm::Codestream &str);
//  void sqlDefinition(Sm::Codestream &str);
  void collectSNode(SemanticTree *n) const;
  bool isFieldForMakestr() const              { return true; }
};



class FunctionDynExpr : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> expr;
  Ptr<PlExpr> sourceForCastDatatype;

  FunctionDynExpr(CLoc l, PlExpr* _expr, PlExpr* _typeSrc = 0);
  FunctionDynExpr(CLoc l, PlExpr* _expr, Datatype* _typeSrc);

  Ptr<Datatype> getDatatype() const;
  bool getFields(EntityFields &fields) const { return expr->getFields(fields); }
  bool getFieldRef(Ptr<Id> &f)               { return expr->getFieldRef(f); }
  // linterDefinition вызывается когда генерируется аргумент в вызове makestr
  void linterDefinition(Sm::Codestream &str);
  void sqlDefinition(Sm::Codestream &str);
  void collectSNode(SemanticTree *n) const;

  FunctionDynExpr *toSelfDynExpr() const { return const_cast<FunctionDynExpr*>(this); }
};

class FunctionDynField : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> expr;
  Ptr<PlExpr> sourceForCastDatatype;

  FunctionDynField(CLoc l, PlExpr* _expr, PlExpr* _typeSrc = 0);
  FunctionDynField(CLoc l, PlExpr* _expr, Datatype* _typeSrc);

  Ptr<Datatype> getDatatype() const;
  bool getFields(EntityFields &fields) const;
  bool getFieldRef(Ptr<Id> &f);

  void collectSNode(SemanticTree *n) const;
  Sm::Id *getSelectedFieldName() const { return 0; }
  bool isFieldForMakestr() const { return true; }
  FunctionDynField *toSelfFunctionDynField() const { return (FunctionDynField*)this; }

  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);

  void linterDefinition(Sm::Codestream &str);
  void sqlDefinition(Sm::Codestream &str);
};

class DynWhereClause : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> expr;

  DynWhereClause(CLoc l, PlExpr* _expr);
  Ptr<Datatype> getDatatype() const { return expr->getDatatype(); }
  bool getFields(EntityFields &fields) const { return expr->getFields(fields); }
  bool getFieldRef(Ptr<Id> &f)               { return expr->getFieldRef(f); }

  void collectSNode(SemanticTree *n) const;

  void linterDefinition(Sm::Codestream &str);
};



class DynamicFuncallTranslator : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum Cathegory {
    CALL_SIGNATURE,
    CALL_NAME,
  };

  static Ptr<Function> funnameTranslator;
  static Ptr<Function> funcallSignatureTranslator;

  Cathegory cathegory;

  Ptr<PlExpr>      funnameSrc;
  Ptr<Datatype>    funType;
  Ptr<CallArgList> callArguments; // транслировать через callArglist

  DynamicFuncallTranslator(CLoc l, PlExpr* _funnameSrc, Datatype *_funType, CallArgList *caArgs);
  DynamicFuncallTranslator(CLoc l, PlExpr* _funnameSrc);

  Ptr<Datatype> getDatatype() const;
  bool getFields   (EntityFields &fields) const;
  bool getFieldRef (Ptr<Id>      &f);
  void collectSNode(SemanticTree *n) const;

  Id               *getSelectedFieldName  () const { return 0;    }
  bool              isFieldForMakestr     () const { return true; }
  FunctionDynField *toSelfFunctionDynField() const { return (FunctionDynField*)this; }

  void sqlDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);

  static Ptr<Function> createTranslatorFunction();
  static Ptr<Function> createSignatureTranslatorFunction();
  void initFunnameSrc(CLoc l, PlExpr* _funnameSrc, Ptr<Function> trFun);
};



class QueryEntityDyn : public ChangedQueryEntity, public QueriedTable {
public:
  Ptr<SqlExpr> dynRef;
  Ptr<Id2>     fieldsSrc;

  QueryEntityDyn(CLoc l, SqlExpr *_dynRef, Id2 *_entityRef);

  SmartVoidType* getThisPtr() const { return const_cast<QueryEntityDyn*>(this); }

  bool getFieldRefAsTableName(Ptr<Id> &) { return false; }

  Ptr<Datatype> getDatatype() const { return fieldsSrc ? fieldsSrc->getDatatype() : Ptr<Datatype>(); }
  bool getFieldRef(Ptr<Id> &field);

  bool getFields(EntityFields &field) const { return fieldsSrc && fieldsSrc->getFields(field); }
  bool isFieldForMakestr() const { return true; }

  void sqlDefinition(Sm::Codestream &str);
  void sqlReference (Sm::Codestream &/*str*/) { throw 999; }
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, dynRef); }

  ScopedEntities ddlCathegory() const { return SqlEntity_; }
  QueryEntityDyn* toSelfQueryEntityDyn() const { return const_cast<QueryEntityDyn*>(this); }

  Sm::SemanticTree *toSTreeBase() const;
};

class DynWhere : public Sm::PlExpr {
public:
  enum CompoundType {
    DYN_EMPTY,
    DYN_AND,
    DYN_OR
  };

  Ptr<PlExpr>  where;
  Ptr<PlExpr>  tail;
  CompoundType compoundType = DYN_EMPTY;

  DynWhere(CLoc &l, Ptr<PlExpr> _where, Ptr<PlExpr> _tail, CompoundType compound = DYN_EMPTY);

  SmartVoidType* getThisPtr() const { return const_cast<DynWhere*>(this); }

  void sqlDefinition(Sm::Codestream &str);

  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void collectSNode(SemanticTree *n) const;

  DynWhere *toSelfDynWhere() const { return const_cast<DynWhere*>(this); }
};

class DynSubquery : public Sm::Subquery {
public:
  typedef vector<Ptr<Sm::Subquery> > QueryExpressions;

  FunctionContext *ctx = 0;
  Ptr<Id> referenceSubquery;

  bool isGlobalSubquery = false;

  Ptr<Subquery> expr_;

  Ptr<RefExpr> reference_;

  IntoList     intoList_;

  Ptr<Subquery> expr() const;
  Ptr<Subquery> singleExpr() const;
  void lazyExprInit();
  void checkConsistance() const;
  
  Sm::GlobalDynSqlCursors::mapped_type *consistantGlobalQueries() const;

  DynSubquery(CLoc l, Ptr<Id> referenceSubquery, Ptr<RefExpr> codeRef, IntoList _intoList);

  void translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory castedCathegory);
  void sqlDefinitionForNthField(Codestream &str, int fieldPos);
  IntoList intoList() const;
  Ptr<Datatype> getDatatype() const;
  bool getFields(EntityFields &fields) const;

  bool getFieldRef(Ptr<Id> &f);

  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);
  Ptr<List<FactoringItem> > pullUpFactoringList() const;
  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference);
  void setIsUnionChild();
  CathegorySubquery cathegorySubquery() const;

  void collectSNode(SemanticTree *n) const;
  Sm::SemanticTree *toSTreeBase() const { return 0; }

  void linterDefinition(Sm::Codestream &str);

  void translateByFetchOps(QueryExpressions *q) const;

  bool isFieldForMakestr() const { return true; }

  DynSubquery* toSelfDynSubquery() const { return const_cast<DynSubquery*>(this); }
};


class BoolTailObj : public SqlExpr {
public:
  SmartVoidType* getThisPtr() const { return const_cast<BoolTailObj*>(this); }
  BoolTailObj(CLoc l) : GrammarBase(l) {}

  Ptr<Sm::Datatype> getDatatype() const;
  ScopedEntities ddlCathegory() const { return TailObj_; }

  void linterDefinition(Sm::Codestream &) {}
  void linterReference(Sm::Codestream &)  {}
  void collectSNode(SemanticTree *) const {}
};

class StrTailObj : public BoolTailObj {
public:
  StrTailObj(CLoc l) : GrammarBase(l), BoolTailObj(l) {}
  Ptr<Sm::Datatype> getDatatype() const;
};

class NumTailObj : public BoolTailObj {
public:
  NumTailObj(CLoc l) : GrammarBase(l), BoolTailObj(l) {}
  Ptr<Sm::Datatype> getDatatype() const;
};

class ConstructExprStmtContext : public Smart {
public:
  bool   concat = false; // - транслировать в вид var := var || tr('expr')
  string nameOfUnionChunck;
  string globalCursorId;

  ConstructExprStmtContext() {}
};


class ConstructExprStmt        : public Statement, public FromResolver {
public:
  typedef List<Sm::FromSingle> FromList;

  mutable Ptr<BoolTailObj> tailObj;
  mutable Ptr<BoolTailObj> beginObj;

  bool procMode = false;

  Ptr<RefExpr>  varRef;

  Ptr<Id>       var;
  Ptr<Id>       srcNamespace;
  Ptr<PlExpr>   expr;

  DeclNamespace        *fromNode  = 0;
  mutable SemanticTree *fromSNode = 0;

  ConstructExprStmtContext context;

  ConstructExprStmt(CLoc l, Ptr<Id> v, Ptr<Id> srcNspace, Ptr<PlExpr> e, Ptr<ConstructExprStmtContext> _ctx, bool _procMode);

  SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void linterDefinition(Sm::Codestream &str);
  bool needSemicolon() { return true; }

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(ExprTr tr);

  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference);
  bool getFieldRef(Ptr<Id> &field);


  ConstructExprStmt *toSelfConstructorExpr() { return this; }

  bool findInFactoringList(Ptr<Id> &/*factoringTable*/) const { return false; }

  ScopedEntities ddlCathegory() const { return ConstructExprStmt_; }
};


class ConstructBlockStmt        : public Statement, public FromResolver {
public:
  typedef List<Sm::FromSingle> FromList;

  Ptr<BlockPlSql> dynamicBlock;
  Ptr<RefAbstract> into;

  ConstructBlockStmt(CLoc l, BlockPlSql* _dynamicBlock, RefAbstract *_into);

  SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  void linterDefinition(Sm::Codestream &str);
  bool needSemicolon() { return true; }

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(ExprTr tr);

  ConstructBlockStmt *toSelfConstructBlockStmt() const { return const_cast<ConstructBlockStmt*>(this); }

  bool findInFactoringList(Ptr<Id> &/*factoringTable*/) const { return false; }

  ScopedEntities ddlCathegory() const { return ConstructExprStmt_; }

  Sm::ResolvedEntity *ownerFunction() const { return const_cast<ConstructBlockStmt*>(this); }

  Ptr<Datatype> getDatatype() const { return into ? into->getDatatype() : Ptr<Datatype>(); }

  void traverseDeclarationsStmt(DeclActor &fun);
};


//class DynamicPlsqlStatement : public Statement {
//public:
//  typedef List<Sm::FromSingle> FromList;

//  Ptr<PlExpr> stmt;

//};


//class CursorFieldDecltype   : public Statement {
//public:
//  Ptr<List<Id> >      varNames;

//  CursorFieldDecltype(CLoc l, List<Id> *v);

//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
//  void collectSNode(SemanticTree *n) const;

//  void linterDefinition(Sm::Codestream &) {}
//  CursorFieldDecltype* toSelfCursorFieldDecltype() const { return const_cast<CursorFieldDecltype*>(this); }
//  void replaceChildsIf(ExprTr );
//  bool needSemicolon() { return false; }

//  ScopedEntities ddlCathegory() const { return CursorFieldDecltype_; }
//};


class DeclNamespace        : public Statement {
public:
  typedef List<Sm::From> FromList;

  Ptr<Id>       name;
  Ptr<FromList> fromList;

  Ptr<Declarations>    declarations;

  mutable SemanticTree *fromNode = 0;

  DeclNamespace(CLoc l, Ptr<Id> n, Ptr<FromList> lst);
  DeclNamespace(CLoc l, Ptr<Id> n, Declarations *lst);

  SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void linterDefinition (Sm::Codestream &) {}
  bool needSemicolon() { return false; }

  DeclNamespace *toSelfDeclNamespace() { return this; }
  ScopedEntities ddlCathegory() const { return DeclNamespace_; }

  void collectSNode(SemanticTree *n) const;
};


class CursorDecltype        : public Statement {
public:
  Ptr<Id>      varName;
  Ptr<SqlExpr> select;

  CursorDecltype(CLoc l, Ptr<Id> v, Ptr<SqlExpr> s);

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void collectSNode(SemanticTree *n) const;
  void linterDefinition (Sm::Codestream &) {}
  CursorDecltype* toSelfCursorDecltype() const { return const_cast<CursorDecltype*>(this); }
  bool getFields(EntityFields &fields) const;
  void replaceChildsIf(ExprTr tr);
  bool needSemicolon() { return false; }

  ScopedEntities ddlCathegory() const { return CursorDecltype_; }
};


class DynLength : public SqlExpr {
public:
  SmartVoidType* getThisPtr() const { return const_cast<DynLength*>(this); }

  Ptr<PlExpr> lengthExpr;

  DynLength(CLoc l, Ptr<PlExpr> expr);

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &);

  Ptr<Datatype> getDatatype() const;
};


}



#endif // DYNAMIC_SQL

