#ifndef SEMANTIC_EXPR_H
#define SEMANTIC_EXPR_H

#include "semantic_expr_select.h"

#define SM_SIZEOF_TABLE( X ) ( sizeof( X ) / sizeof( X[ 0 ] ) )
#define SM_STRING( X ) X, SM_SIZEOF_TABLE( X )

namespace Sm {
  using namespace std;
  using namespace dstring;

class AlgebraicCompound   : public SqlExpr {
  static const int stateTab[4][4];
public:
  Ptr<SqlExpr>          lhs;
  algebraic_compound::t op;
  Ptr<SqlExpr>          rhs;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
protected:
  void translateConcat(Codestream &str);
  void translateSubs(Codestream &str);
  void translateAdd(Codestream &str);
public:
  enum OperandTypeCathegory {
    DATE     = 0,
    INTERVAL = 1,
    NUMBER   = 2,
    VARCHAR  = 3,
    OTHER    = -1
  };

//  AlgebraicCompound() {}

  AlgebraicCompound(CLoc l, Ptr<SqlExpr> _lhs, algebraic_compound::t _op, Ptr<SqlExpr> _rhs, SemanticTree *n = 0);
  AlgebraicCompound(SqlExpr *_lhs, algebraic_compound::t _op, SqlExpr *_rhs);

  AlgebraicCompound *toSelfAlgebraicCompound() const { return const_cast<AlgebraicCompound*>(this); }

  bool isConcatenation() const { return op == algebraic_compound::CONCAT; }
  Ptr<Datatype> getDatatype() const;
  Id *getSelectedFieldName() const { return 0; }

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;


  Ptr<SqlExpr> getLhs() { return lhs; }
  Ptr<SqlExpr> getRhs() { return rhs; }

  bool isDataDatatypeAlgebraicCompound();

  std::string opToString() const;
  std::string opToLogString() const;
  void linterDefinition(Codestream &str);
  void sqlDefinition(Sm::Codestream &str);
  void oracleDefinition(Sm::Codestream &str);
  std::string toOperationString() const;

  void collectSNode(SemanticTree *node) const;
  ScopedEntities ddlCathegory() const { return AlgebraicCompound_; }
  virtual bool isComplexExpr() const { return true; }
  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs); }
  void translateToConsistantOperand();

  void enumSubExpressions(List<PlExpr> &exprList);

  static OperandTypeCathegory typeNumericCat(Ptr<Datatype> &t);
};

class ExtractExpr : public SqlExpr {
public:
  Ptr<SqlExpr> fromEntity;
  ExtractedEntity extractedEntity_;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  ExtractExpr(CLoc l, ExtractedEntity ent = ExtractedEntity::HOUR, SqlExpr *_fromEntity = 0)
    : GrammarBase(l),
      fromEntity(_fromEntity),
      extractedEntity_(ent) {}

  ScopedEntities ddlCathegory() const { return ExtractFrom_; }

  static bool identifyKeyword(Id *kw, ExtractedEntity *p);

  void collectSNode(SemanticTree *n) const;

  ExtractExpr* toSelfExtractFromExpression() const { return (ExtractExpr*)this; }
  Ptr<Datatype> getDatatype() const;
  Id *getSelectedFieldName() const { return 0; }
  std::string toStringEntity() const;

  void sqlDefinition(Codestream &str);
  void replaceChildsIf(ExprTr tr) { replace(tr, fromEntity); }
};

class TrimFromExpr : public SqlExpr {
public:
  enum ExtractedEntity { LEADING, TRAILING, BOTH };
protected:
  Ptr<SqlExpr> charExpr;
  Ptr<SqlExpr> fromEntity;
  ExtractedEntity extractedEntity_ = BOTH;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

public:
  TrimFromExpr(CLoc l, CLoc chExprLoc, Id *_charExpr, SqlExpr *_fromEntity = 0, ExtractedEntity ent = BOTH);
  TrimFromExpr(CLoc l, SqlExpr *_charExpr, ExtractedEntity ent = BOTH);

  ScopedEntities ddlCathegory() const { return TrimFrom_; }

  void setFromEntity(SqlExpr *expr) { fromEntity = expr; }

  static bool identifyKeyword(Ptr<Id> kw, ExtractedEntity *p);
  void collectSNode(SemanticTree *n) const;

  Id *getSelectedFieldName() const { return 0; }
  Ptr<Datatype> getDatatype() const { return Datatype::mkVarchar2(); }

  std::string cathegoryToString() const;
  void sqlDefinition(Codestream &str);
  void linterDefinition(Codestream &str);

  void replaceChildsIf(ExprTr tr) { replace(tr, charExpr, fromEntity); }
};



class OutherJoinExpr          : public RefAbstract {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  OutherJoinExpr(CLoc l, Ptr<IdEntitySmart> _reference) : GrammarBase(l), RefAbstract(_reference) {}
  SemanticTree* toSTreeBase() const;
  void collectSNode(SemanticTree *n) const;
  Id *getSelectedFieldName() const { return 0; }
  ScopedEntities ddlCathegory() const { return SqlExprOutherJoin_; }
  void sqlDefinition(Codestream &str) { str << reference << "(+)"; }
};
class AsteriskExpr            : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  // getDatatype - должен вернуть строку-запись (если реализация getDatatype для таблиц, курсоров, типов и записей - корректна).
  Ptr<Datatype> getDatatype() const { throw 999; return 0; }
  AsteriskExpr(CLoc l);
  SemanticTree *toSTreeBase() const;
  void collectSNode(SemanticTree *n) const { setSemanticNode(n); }
  ScopedEntities ddlCathegory() const { return SqlExprAsterisk_; }
  Id *getSelectedFieldName() const { return 0; }
  bool isAsterisk() const { return true; }
  void sqlDefinition(Sm::Codestream &str);
  AsteriskExpr* toSelfAsteriskExpr() const { return const_cast<AsteriskExpr*>(this); }
};
class AsteriskRefExpr         : public RefAbstract {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  // getDatatype - должен вернуть строку-запись (если реализация getDatatype для таблиц, курсоров, типов и записей - корректна).
  Ptr<Datatype> getDatatype() const;
  AsteriskRefExpr(CLoc l, Ptr<IdEntitySmart> _reference);
  SemanticTree *toSTreeBase() const;
  void collectSNode(SemanticTree *n) const { ANODE2(n, this); }
  ScopedEntities ddlCathegory() const { return AsteriskRefExpr_; }
  Id *getSelectedFieldName() const { return 0; }
  bool isAsterisk() const { return true; }
  void sqlDefinition(Sm::Codestream &str);
  AsteriskRefExpr* toSelfAsteriskRefExpr() const { return const_cast<AsteriskRefExpr*>(this); }
  ~AsteriskRefExpr() { clrSTree(); }
};

// SQL % свойство
class CursorSqlProperties    : public virtual SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

protected:
  cursor_properties::Properties property;
public:
  static Ptr<Datatype> getDatatypeByProperty(cursor_properties::Properties property);

  Ptr<Datatype> getDatatype() const { return getDatatypeByProperty(property); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }

  Id *getSelectedFieldName() const { return 0; }
  void linterDefinition(Sm::Codestream &str);
  CursorSqlProperties(CLoc l, cursor_properties::Properties cursor_property);
  CursorSqlProperties(cursor_properties::Properties cursor_property);

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return SqlExprSqlCursorProperties_; }
};


class CursorProperties    : public RefAbstract, public CursorSqlProperties {
public:
  CursorProperties(CLoc l, Ptr<IdEntitySmart> _reference, cursor_properties::Properties cursor_property);

  Id *getSelectedFieldName() const { return 0; }
  Ptr<Datatype> getDatatype() const { return CursorSqlProperties::getDatatype(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return CursorSqlProperties::isSubtype(supertype, plContext); }
  void linterDefinition(Sm::Codestream &str);
  SemanticTree* toSTreeBase() const;
  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return SqlExprCursorProperties_; }
};


class HostCursorPropertiesExpr: public RefAbstract {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  cursor_properties::Properties property;
public:
  Ptr<Datatype> getDatatype() const { return CursorProperties::getDatatypeByProperty(property); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }
  HostCursorPropertiesExpr(CLoc l, Ptr<IdEntitySmart> _reference, cursor_properties::Properties cursor_property);
  Id *getSelectedFieldName() const { return 0; }
  SemanticTree* toSTreeBase() const;
  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return SqlExprHostCursorProperties_; }
};
class RefHostExpr       : public RefAbstract {
protected:
  Ptr<IdEntitySmart> secondRef;
  bool               indicator;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  RefHostExpr(CLoc l, Ptr<IdEntitySmart> _reference, Ptr<IdEntitySmart> _secondRef = 0, bool ind = false)
    : GrammarBase(l), RefAbstract(_reference), secondRef(_secondRef), indicator(ind) {}

  Sm::RefHostExpr* toSelfRefHostExpr() const { return const_cast<RefHostExpr*>(this); }

  SemanticTree *toSTreeBase() const;

  bool getFieldRef(Ptr<Sm::Id> &field);

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return RefHostExpr_; }
  void linterDefinition(Sm::Codestream &str);
  void replaceChildsIf(ExprTr tr);
};
class BulkRowcountExpr     : public RefAbstract {
protected:
  Ptr<NumericValue> argval;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Datatype> getDatatype() const { return Datatype::mkInteger(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }
  BulkRowcountExpr(CLoc l, Ptr<NumericValue> _argval);

  Id *getSelectedFieldName() const { return 0; }

  SemanticTree *toSTreeBase() const;
  inline void collectSNode(SemanticTree *n) const { ANODE2(n, this); }
  ScopedEntities ddlCathegory() const { return SqlBulkRowcount_; }
  void replaceChildsIf(ExprTr tr) { replace(tr, reference, argval); }
};

class AnalyticFun                 : public RefAbstract {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum AnalyticCathegory {
    LAG,
    FIRST_VALUE,
    SUM,
    ROW_NUMBER,
    COUNT,
    MAX,
    MIN,
    LEAD
  };
  typedef std::map<string, AnalyticCathegory> AnalyticCathegoryMap;

  AnalyticCathegory analyticCathegory;
  Ptr<Id> name;
  Ptr<CallArgList> callarglist;
  Ptr<OrderBy> orderByClause;

  Id *getSelectedFieldName() const { return 0; }

  AnalyticFun(Ptr<Id> _name, Ptr<IdEntitySmart> refcall, Ptr<OrderBy> _orderByClause);

  void sqlDefinition(Codestream &str);
  Ptr<Id> getName() const { return name; }
  bool getFieldRef(Ptr<Id> &field);
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }
  Ptr<Datatype> getDatatype() const;

  void collectSNode(SemanticTree *n) const { ANODE2(n, this); }
  SemanticTree *toSTreeBase() const;
  ResolvedEntity::ScopedEntities ddlCathegory() const { return AnalyticFun_; }

  void replaceChildsIf(ExprTr tr) { replace(tr, reference, name, callarglist, orderByClause); }
};

inline RefAbstract* makeOwerExpr(Ptr<IdEntitySmart> refcall, Ptr<Id> keyword, Ptr<OrderBy> orderByClause) {
  if (!keyword || keyword->toNormalizedString() != "OVER")
    throw 999; // OVER должно быть определено по синтаксису
  if (Ptr<Id> ent = refcall->entity()) {
    return new AnalyticFun(ent, refcall, orderByClause);
  }
  else
    throw 999;
}

class RowIdExpr               : public SqlExpr {
  mutable Ptr<Id> id;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  static Ptr<RowIdExpr> self;
  RowIdExpr(CLoc l = cl::emptyFLocation()) : GrammarBase(l) {}

  bool skipSelectedFieldExprNamespaceUpdating() const { return false; }

  inline void collectSNode(SemanticTree *n) const { SNODE(n); }
  Ptr<Datatype> getDatatype() const { return Datatype::mkRowid(); }
  ScopedEntities ddlCathegory() const { return RowId_; }
  void sqlDefinition(Codestream &str) { str << "ROWID"; }
  Id *getSelectedFieldName() const;
};
class RowNumExpr              : public SqlExpr {
  mutable Ptr<Id> id;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  static Ptr<RowNumExpr> self;
  RowNumExpr(CLoc l = cl::emptyFLocation()) : GrammarBase(l) {}
  inline void collectSNode(SemanticTree *n) const { SNODE(n); }
  Ptr<Datatype> getDatatype() const { return Datatype::mkInteger(); }
  ScopedEntities ddlCathegory() const { return RowNum_; }
  void sqlDefinition(Codestream &str) { str << "ROWNUM"; }
  bool isRownum() const { return true; }
  Id *getSelectedFieldName() const;
  RowNumExpr* toSelfRowNumExpr() const { return const_cast<RowNumExpr*>(this); }
};
class EmptyIdExpr             : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  EmptyIdExpr() : SqlExpr() {}
  EmptyIdExpr(CLoc l) : GrammarBase(l) {}

  ScopedEntities ddlCathegory() const { return SqlExprEmptyId_; }

  inline void collectSNode(SemanticTree *n) const { SNODE(n); }

  bool isEmpty() const { return true; }
  bool isEmptyId() const { return true; }
  Ptr<Sm::Datatype> getDatatype() const { return Sm::Datatype::mkVarchar2(); }
  Id *getSelectedFieldName() const { return 0; }

  void oracleDefinition(Sm::Codestream &str) { str << "\"\""; }
  void linterDefinition(Sm::Codestream &str) { str << "\"\""; }
  void sqlDefinition   (Sm::Codestream &str) { str << "''"; /* sql ok. proc - ? */ }

};
class PriorExpr               : public SqlExpr {
protected:
  Ptr<SqlExpr> prior;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR1(PriorExpr, prior) {}
  void collectSNode(SemanticTree *n) const;
  Ptr<Sm::Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return prior->isSubtype(supertype, plContext); }
  bool getFieldRef(Ptr<Sm::Id> &field) { return prior->getFieldRef(field); }
  ScopedEntities ddlCathegory() const { return SqlExprPrior_; }
  Id *getSelectedFieldName() const { return 0; }

  void sqlDefinition(Codestream &str);

  void replaceChildsIf(ExprTr tr) { replace(tr, prior); }
};

class CursorExpr              : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Subquery> cursor;

  CursorExpr(CLoc l, Subquery *_cursor)
    : GrammarBase(l), cursor(_cursor) {}

  bool skipSelectedFieldExprNamespaceUpdating() const { return false; }
  ScopedEntities ddlCathegory() const { return SqlExprCursor_; }

  void collectSNode(SemanticTree *n) const;
  Ptr<Sm::Datatype> getDatatype() const { return cursor ? cursor->getDatatype() : Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return cursor->isSubtype(supertype, plContext); }
  Ptr<Id> getName() const { return cursor->getName(); }
  bool getFieldRef(Ptr<Sm::Id> &field) { return cursor->getFieldRef(field); }
  ResolvedEntity* getNextDefinition() const { return cursor->getNextDefinition(); }
  Id *getSelectedFieldName() const { return cursor->getSelectedFieldName(); }

  bool isCursor() const { return true; }
  void replaceChildsIf(ExprTr tr);

  CursorExpr* toSelfCursorExpr() const { return const_cast<CursorExpr*>(this); }
};
class Brackets            : public virtual SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> brackets;

  Brackets(CLoc l, SqlExpr *_br) : GrammarBase(l), brackets(_br) { if (!brackets) throw 999; }

  RefExpr* toSimpleFunctionCall() const;
  RefExpr* unwrapRefExpr() const { return brackets->unwrapRefExpr(); }

  ScopedEntities ddlCathegory() const { return SqlExprBrackets_; }

  bool isRownum() const { return brackets->isRownum(); }
  bool isIntValue() const { return brackets->isIntValue(); }
  unsigned long int getUIntValue() { return brackets->getUIntValue(); }
  long int getSIntValue() { return brackets->getSIntValue(); }
  bool isNull() const { return brackets->isNull(); }
  bool isComplexExpr() const { return brackets->isComplexExpr(); }

  void collectSNode(SemanticTree *n) const;
  Ptr<SqlExpr> inbrackets() const { return brackets.object(); }
  Ptr<Id> getName() const { return brackets->getName(); }
  Ptr<Datatype> getDatatype() const { return brackets->getDatatype(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return brackets->isSubtype(supertype, plContext); }
  bool getFieldRef(Ptr<Sm::Id> &field) { return brackets->getFieldRef(field); }
  ResolvedEntity* getNextDefinition() const { return brackets->getNextDefinition(); }
  Id *getSelectedFieldName() const { return brackets->getSelectedFieldName(); }

  void linterDefinition(Sm::Codestream &s) { s << s::obracket << brackets << s::cbracket; }
  void replaceChildsIf(ExprTr tr) { replace(tr, brackets); }
  bool isQuotedSqlExprId() const { return brackets->isQuotedSqlExprId(); }
  void setDateLiteral() { brackets->setDateLiteral(); }
  void setSQuotedLiteral() { brackets->setSQuotedLiteral(); }
  bool isNumericValue() const { return brackets->isNumericValue(); }
  Brackets * toSelfBrackets() const { return const_cast<Brackets*>(this); }
  bool isEmptyId() const { return brackets->isEmptyId(); }
  void setStringType() { brackets->setStringType(); }
  bool skipLevelNamespaceUpdating() const { return brackets->skipLevelNamespaceUpdating(); }
  bool skipSelectedFieldExprNamespaceUpdating() const { return brackets->skipSelectedFieldExprNamespaceUpdating(); }
  Ptr<PlExpr> unwrapBrackets(int *isNot = 0) const;

};

class UnaryPlus           : public SqlExpr {
protected:
  Ptr<SqlExpr> op;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool isNumericLiteral() const { return op->isNumericLiteral(); }
  bool skipSelectedFieldExprNamespaceUpdating() const { return op->skipSelectedFieldExprNamespaceUpdating(); }
  DEF_LUCONSTRUCTOR1(UnaryPlus, op) { if (!op) throw 999; }
  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return op->getDatatype(); }
  ScopedEntities ddlCathegory() const { return SqlExprUnaryPlus_; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return op->isSubtype(supertype, plContext); }
  void linterDefinition(Sm::Codestream &s) { s << '+' << op; }
  Id *getSelectedFieldName() const { return op->getSelectedFieldName(); }

  void replaceChildsIf(ExprTr tr) { replace(tr, op); }

  bool isIntValue() const { return op->isIntValue(); }
  unsigned long int getUIntValue() { return op->getUIntValue(); }
  long int getSIntValue() { return op->getSIntValue(); }
  bool isNumericValue() const { return op->isNumericValue(); }
  void setStringType() { op->setStringType(); }
  ResolvedEntity* getNextDefinition() const { return op; }
};
class UnaryMinus          : public SqlExpr {
protected:
  Ptr<PlExpr> op;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool isNumericLiteral() const { return op->isNumericLiteral(); }
  bool skipSelectedFieldExprNamespaceUpdating() const { return op->skipSelectedFieldExprNamespaceUpdating(); }

  UnaryMinus(CLoc l, PlExpr *_op) : GrammarBase(l), op(nAssert(_op)) {}

  void collectSNode(SemanticTree *n) const;
  bool getFieldRef(Ptr<Id> &field) { return op->getFieldRef(field); }
  Id *getSelectedFieldName() const { return op->getSelectedFieldName(); }
  Ptr<Datatype> getDatatype() const { return op->getDatatype(); }
  ScopedEntities ddlCathegory() const { return SqlExprUnaryMinus_; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return op->isSubtype(supertype, plContext); }
  void linterDefinition(Sm::Codestream &s);
  void replaceChildsIf(ExprTr tr) { replace(tr, op); }

  bool isIntValue() const { return op->isIntValue(); }
  unsigned long int getUIntValue() { return op->getUIntValue(); }
  long int getSIntValue() { return -(op->getSIntValue()); }
  bool isNumericValue() const { return op->isNumericValue(); }
  void setStringType() { op->setStringType(); }
  ResolvedEntity* getNextDefinition() const { return op; }


  virtual Ptr<PlExpr> deepCopy();
};


class NewCall             : public SqlExpr {
  Ptr<Id2> objectTypeRef;
  Ptr<CallArgList> argList;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR2(NewCall, objectTypeRef, argList) { objectTypeRef->entity()->callArglist = argList.object(); }

  ScopedEntities ddlCathegory() const { return SqlExprNewCall_; }

  void collectSNode(SemanticTree *n) const;
  bool getFields(EntityFields &fields) const;

  Ptr<Datatype> getDatatype() const;
  IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  bool getFieldRef(Ptr<Id> &field);
  ResolvedEntity* getNextDefinition() const;
  Id *getSelectedFieldName() const { return 0; }

  void replaceChildsIf(ExprTr tr) { replace(tr, objectTypeRef, argList); }
};
class CaseIfThen : public GrammarBaseSmart, public Translator {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> condition;
  Ptr<PlExpr> action;

  CaseIfThen(CLoc l, PlExpr *cond, PlExpr *act);
  CaseIfThen(CLoc l, Ptr<PlExpr> cond, Ptr<PlExpr> act);

  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Codestream &str);
  virtual ~CaseIfThen() {}
  void replaceChildsIf(ExprTr tr) { replace(tr, condition, action); }
};
class Case                : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr>        source;
  Ptr<List<CaseIfThen> > cases;
  Ptr<PlExpr>        elseClause; // <<- hasElse + sql_expr

public:
  Case(CLoc l, List<CaseIfThen> *_cases, PlExpr *_elseClause, PlExpr *src = 0);
  Case(FLoc l, Ptr<PlExpr> caseIf, Ptr<PlExpr> caseThen, Ptr<PlExpr> caseElse, SemanticTree *n = 0);


  Ptr<Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const;
  Id *getSelectedFieldName() const { return 0; }

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return SqlExprCase_; }
  void sqlDefinition(Codestream &str);
  void linterDefinition(Codestream &str);
  void replaceChildsIf(ExprTr tr) { replace(tr, source, cases, elseClause); }

  Sm::Case* toSelfCase() const { return const_cast<Case*>(this); }
  void translateToConsistantOperand();

private:
  void outCaseSourceClause(Codestream &str, List<CaseIfThen>::iterator it);
};
class Cast                : public SqlExpr {
  Ptr<PlExpr>  castedExpr; // real in sql grammar - is SqlExpr
  Ptr<Datatype> toType;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Cast(CLoc l, PlExpr *_castedExpr, Datatype* _toType)
    : GrammarBase(l), castedExpr(_castedExpr), toType(_toType) {}

  ScopedEntities ddlCathegory() const { return SqlExprCast_; }
  bool isSqlTableContainerFuncall() const { return toType->isSqlTableContainerFuncall(); }

  void collectSNode(SemanticTree *n) const;

  Cast* toSelfCastStatement() const { return const_cast<Cast*>(this); }
  Ptr<Datatype> getDatatype() const { return toType; }
  bool getFieldRef(Ptr<Sm::Id> &field) { return toType && toType->getFieldRef(field); }
  ResolvedEntity* getNextDefinition() const { return toType ? toType->getNextDefinition() : (ResolvedEntity*)0; }
  ResolvedEntity *getCursorFuncall() { return castedExpr->getCursorFuncall(); }
  Id *getSelectedFieldName() const { return castedExpr->getSelectedFieldName(); }

  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(ExprTr tr) { replace(tr, castedExpr, toType); }
};
class CastMultiset        : public SqlExpr {
  Ptr<Subquery> castedQuery;
  Ptr<Datatype> toType;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  CastMultiset(CLoc l, Subquery *q, Datatype *dstT)
    : GrammarBase(l), castedQuery(q), toType(dstT) {}

  ScopedEntities ddlCathegory() const { return SqlExprCastMultiset_; }

  void collectSNode(SemanticTree *n) const;

  Ptr<Datatype> getDatatype() const { return toType; }
  bool getFieldRef(Ptr<Sm::Id> &field) { return toType && toType->getFieldRef(field); }
  ResolvedEntity* getNextDefinition() const { return toType ? toType->getNextDefinition() : (ResolvedEntity*)0; }
  Id *getSelectedFieldName() const { return castedQuery->getSelectedFieldName(); }

  void replaceChildsIf(ExprTr tr);
};
class CollectionAccess    : public SqlExpr {
  Ptr<IdEntitySmart>  collectionRef;
  Ptr<List<SqlExpr> > indices;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  CollectionAccess(CLoc l, Ptr<IdEntitySmart> _collectionRef, List<SqlExpr> *_indices);

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  void collectSNode(SemanticTree *n) const;
  Id *getSelectedFieldName() const { return 0; }

  Ptr<Sm::Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const;

  void replaceChildsIf(ExprTr tr) { replace(tr, collectionRef, indices); }
};
class DefaultExpr             : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DefaultExpr() : SqlExpr() {}
  DefaultExpr(CLoc l) : GrammarBase(l) {}

  inline void collectSNode(SemanticTree *n) const { SNODE(n); }

  Id *getSelectedFieldName() const { return 0; }
  Ptr<Datatype> getDatatype() const { return Datatype::mkDefault(); }
  ScopedEntities ddlCathegory() const { return SqlExprDefault_; }
};

class TimeExprInterval : public GrammarBaseSmart {
protected:
  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum IntervalCathegory { DAY_TO_SECOND, YEAR_TO_MONTH };
  Ptr<NumericValue> day_or_year;
  Ptr<NumericValue> second;
  IntervalCathegory intervalCathegory;

  DEF_LBCONSTRUCTOR2(TimeExprInterval, day_or_year, second), intervalCathegory(DAY_TO_SECOND) {}
  DEF_LBCONSTRUCTOR1(TimeExprInterval, day_or_year), intervalCathegory(YEAR_TO_MONTH) {}

  void replaceChildsIf(ExprTr tr) { replace(tr, day_or_year, second); }
};
class TimeExprTimezone : public GrammarBaseSmart {
protected:
  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum TimezoneCathegory { AT_TIMEZONE_EXPR, AT_LOCAL, AT_TIMEZONE_DBTIMEZONE, AT_TIMEZONE_SESSIONTIMEZONE };
  Ptr<SqlExpr>      timezone;
  TimezoneCathegory timezoneCathegory;

  DEF_LBCONSTRUCTOR1(TimeExprTimezone, timezone), timezoneCathegory(AT_TIMEZONE_EXPR) {}
  DEF_LBCONSTRUCTOR1(TimeExprTimezone, timezoneCathegory) {}

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(ExprTr tr) { replace(tr, timezone); }
};

class TimeExpr            : public SqlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr>  base;
  Ptr<TimeExprInterval> interval;
  Ptr<TimeExprTimezone> timezone;

  Ptr<Sm::Datatype> getDatatype() const;
  TimeExpr(CLoc l = cl::emptyFLocation()) : GrammarBase(l) {}
  DEF_LUCONSTRUCTOR2(TimeExpr, base, timezone) {}
  DEF_LUCONSTRUCTOR2(TimeExpr, base, interval) {}

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return TimeExpr_; }
  void sqlDefinition(Codestream &str) { str << "CAST" << s::name << base << s::name << "AS DATE"; }
  void replaceChildsIf(ExprTr tr) { replace(tr, base, interval, timezone); }
  Id *getSelectedFieldName() const { return 0; }
};





namespace pl_expr  {

class OfType : public GrammarBaseSmart {
protected:
  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool     only;
  Ptr<Id2> tid;

  DEF_LBCONSTRUCTOR2(OfType, only, tid) {}
  void collectSNode(SemanticTree *n) const { n->addChild(tid->toSNodeDatatypeRef(SCathegory::OfTypes)); }
  void replaceChildsIf(ExprTr tr) { replace(tr, tid); }
};

class OfTypes       : public PlExpr  {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr>       entity;
  Ptr<List<OfType> > types;

  DEF_LUCONSTRUCTOR2(OfTypes, entity, types) {}

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, entity, types); }
};
class Submultiset   : public PlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum SubmultisetCathegory { IS_SUBMULTISET, IS_A_SET };
  Ptr<IdEntitySmart> exprEntity;
  Ptr<IdEntitySmart> submultisetEntity;
  SubmultisetCathegory submultisetCathegory = IS_A_SET;

  Submultiset() {}
  Submultiset(CLoc l) : GrammarBase(l) {}
  Submultiset(CLoc l, Ptr<IdEntitySmart> _submultisetEntity)
    : GrammarBase(l), submultisetEntity(_submultisetEntity), submultisetCathegory(IS_SUBMULTISET) {}

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr);
};
class Path          : public PlExpr {
  Ptr<Id>           column;
  Ptr<NumericValue> levels;
  Ptr<Id>           pathString;
  Ptr<NumericValue> correlationInteger;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR3(Path, column, pathString, correlationInteger        ) {}
  DEF_LUCONSTRUCTOR4(Path, column, levels, pathString, correlationInteger) {}

  void collectSNode(SemanticTree *n) const { SNODE(n); if (column) n->addChild(column->toSNodeRef(SCathegory::Field)); }
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, column, levels, pathString, correlationInteger); }
};
class RegexpLike    : public PlExpr {
  Ptr<SqlExpr> sourceChar;
  Ptr<SqlExpr> pattern;
  Ptr<Id>      mathParam;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR3(RegexpLike, sourceChar, pattern, mathParam) {}

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, sourceChar, pattern, mathParam); }
};


class Like          : public PlExpr {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr>  char1;
  Ptr<SqlExpr>  char2;
  Ptr<SqlExpr>  esc_char;
  like_cathegory::t likeCathegory;
  DEF_LUCONSTRUCTOR3(Like, char1, char2, likeCathegory) {}

  void collectSNode(SemanticTree *n) const;
  Ptr<Sm::Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void sqlDefinition(Codestream &str);
  void linterDefinition(Sm::Codestream &str) { sqlDefinition(str); }
  void replaceChildsIf(ExprTr tr) { replace(tr, char1, char2, esc_char); }

};
class Between       : public PlExpr {
  Ptr<SqlExpr> value;
  Ptr<SqlExpr> low;
  Ptr<SqlExpr> high;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR3(Between, value, low, high) {}
  void linterDefinition(Sm::Codestream &str);
  void collectSNode(SemanticTree *n) const;
  Ptr<Sm::Datatype> getDatatype() const { return Datatype::mkBoolean(); }

  void sqlDefinition(Codestream &str);

  void replaceChildsIf(ExprTr tr) { replace(tr, value, low, high); }

  Between *toSelfBetween() const { return const_cast<Between*>(this); }
  void translateToConsistantOperand();
};
class MemberOf      : public PlExpr  {
  Ptr<SqlExpr>       memberedItem;
  Ptr<IdEntitySmart> nestedTableRef;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:

  MemberOf(CLoc l, Ptr<SqlExpr> _memberItem, Ptr<IdEntitySmart> _nestedTableRef);

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, memberedItem, nestedTableRef); }
};

class Comparsion    : public PlExpr  {
  void checkInvariant() const;

public:
  Ptr<PlExpr>                   lhs;
  ComparsionOp::t               op;
  QuantorOp   ::t               quantor = QuantorOp::EMPTY;
  typedef List<PlExpr> RHS;
  Ptr<RHS> rhs;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Comparsion(FLoc l, PlExpr *_lhs, PlExpr *_rhs);
  Comparsion(CLoc l, PlExpr *_lhs, ComparsionOp::t _op, QuantorOp::t _q, RHS *_rhs);
  Comparsion(FLoc l, Ptr<PlExpr> _lhs, ComparsionOp::t _op, Ptr<PlExpr> _rhs, SemanticTree *n = 0);

  void collectSNode(SemanticTree *n) const;

  Ptr<Sm::Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void sqlDefinition(Codestream &s);
  void resolve(ResolvedEntity *owner);
  void linterDefinition (Sm::Codestream &str);
  Ptr<Sm::SqlExpr> getRownumValue();
  ComparsionOp::t getNormalizedComparsionByNot();
  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs); }
  bool isComplexExpr() const { return true; }
  Comparsion* toSelfComparion() const { return const_cast<Comparsion*>(this); }

  bool isLimitExpr();
  void translateToConsistantOperand();
  ScopedEntities ddlCathegory() const { return Comparsion_; }
};

class BracketsLogicalExpr : public PlExpr {
  Ptr<PlExpr> brackets;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(ResolvedEntity* owner) { if (brackets) brackets->resolve(owner); }
  DEF_LUCONSTRUCTOR1(BracketsLogicalExpr, brackets) { if (!brackets) throw 999; }

  RefExpr *toSimpleFunctionCall() const;
  RefExpr *unwrapRefExpr() const { return brackets->unwrapRefExpr(); }

  void collectSNode(SemanticTree *n) const;
  Ptr<Sm::Datatype> getDatatype() const { return brackets->getDatatype(); }
  void linterDefinition(Sm::Codestream &s) {
    if (isNot())
      s << "NOT" << s::name;
    s << s::obracket << brackets << s::cbracket;
  }
  bool isRownum() const { return brackets->isRownum(); }

  void replaceChildsIf(ExprTr tr) { replace(tr, brackets); }
  bool isIntValue() const { return brackets && brackets->isIntValue(); }
  bool isNull() const { return brackets->isNull(); }
  unsigned long int getUIntValue() { return brackets->getUIntValue(); }
  long int getSIntValue() { return brackets->getSIntValue(); }

  bool isQuotedSqlExprId() const { return brackets->isQuotedSqlExprId(); }
  void setDateLiteral() { brackets->setDateLiteral(); }
  void setSQuotedLiteral() { brackets->setSQuotedLiteral(); }
  bool isNumericValue() const { return brackets->isNumericValue(); }
  bool isEmptyId() const { return brackets->isEmptyId(); }
  void setStringType() { brackets->setStringType(); }
  Ptr<PlExpr> unwrapBrackets(int *isNot) const;
};

class BracketedPlExprList : public GrammarBaseSmart {
protected:
  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void checkInvariant() const { if (!list) throw 999; }
public:
  Ptr<List<SqlExpr> > list;
  bool isBracketed;

//  BracketedPlExprList() : isBracketed(false) {}
  DEF_LBCONSTRUCTOR2(BracketedPlExprList, list, isBracketed) { checkInvariant(); }
  BracketedPlExprList(const FLoc &l, const Ptr<List<SqlExpr> > &_list, bool _isBracketed)
    : GrammarBaseSmart(l), list(_list), isBracketed(_isBracketed) { checkInvariant(); }

  void collectSNode(SemanticTree *n) const { CollectSNode(n, list); }
  void replaceChildsIf(ExprTr tr) { replace(tr, *list); }

  bool isIntValue() const { return list && list->size() == 1 && list->front()->isIntValue(); }
};


class ComparsionList: public PlExpr  {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:

  Ptr<List<SqlExpr> >           lhs;
  comparsion_list::ComparsionOp op;
  QuantorOp::t                  quantor;

  Ptr<List<BracketedPlExprList> > rhs; // списки, заключенные в скобки

  DEF_LUCONSTRUCTOR4(ComparsionList, lhs, op, quantor, rhs) {}
  std::string opToString() const;
  std::string notOpToString() const;

  void collectSNode(SemanticTree *n) const { SNODE(n); CollectSNode(n, lhs); CollectSNode(n, rhs); }
  Ptr<Sm::Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void linterDefinition(Codestream &str);

  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs); }
};

inline void concatBracketedList(List<BracketedPlExprList> * res, Ptr<BracketedPlExprList> tail ) {
  if ( tail->isBracketed ) {
    res->push_back(tail);
    return;
  }
  Ptr<BracketedPlExprList> endItem = *(--(res->end()));

  if ( endItem->isBracketed ) // && tail->isBracketed == false;
    for (List<SqlExpr>::const_iterator it = tail->list->begin(); it != tail->list->end(); ++it)
      res->push_back(new BracketedPlExprList((*it)->getLLoc(), new List<SqlExpr>(it->object()), true));
  else // ($$->end()-1)->isBracketed == false && $3->isBracketed == false;
    for (List<SqlExpr>::const_iterator it = tail->list->begin(); it != tail->list->end(); ++it)
      endItem->list->push_back(*it);
}

class IsEmpty        : public PlExpr {
  Ptr<IdEntitySmart> entityRef;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  IsEmpty(CLoc l, Ptr<IdEntitySmart> _entityRef);

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, entityRef); }
};
class Exists         : public PlExpr {
  Ptr<PlExpr> query;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR1(Exists, query) {}

  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }

  void sqlDefinition(Codestream &str);
  void replaceChildsIf(ExprTr tr);
};
class IsInfinite     : public PlExpr {
  Ptr<SqlExpr> expr;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR1(IsInfinite, expr) {}
  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, expr); }
};
class IsNan          : public PlExpr {
  Ptr<SqlExpr> expr;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR1(IsNan, expr) {}
  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, expr); }
};
class IsNull         : public PlExpr {
  Ptr<SqlExpr> expr;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(ResolvedEntity* owner) { if (expr) expr->resolve(owner); }
  DEF_LUCONSTRUCTOR1(IsNull, expr) {}
  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void sqlDefinition (Sm::Codestream &s) { s << expr << s::name << (isNot() ? "IS NOT NULL" : "IS NULL"); }
  void linterDefinition (Sm::Codestream &s) { s << expr << s::name << (isNot() ? "<> NULL" : "= NULL"); }
  void replaceChildsIf(ExprTr tr) { replace(tr, expr); }
};


class LogicalCompound: public PlExpr {
  void checkInvariant() { if (!lhs || !rhs) throw 999; }
public:
  Ptr<PlExpr>             lhs;
  Ptr<PlExpr>             rhs;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  LogicalCompound(CLoc l, PlExpr *_lhs, PlExpr *_rhs, int isOr);
  LogicalCompound(CLoc l = cl::emptyFLocation()) : GrammarBase(l) { checkInvariant(); }

  inline bool isOr() const { return __flags__.v & (SmartCount)logical_compound::OR; }

  pl_expr::LogicalCompound* toSelfLogicalCompound() const { return const_cast<LogicalCompound*>(this); }
  void collectSNode(SemanticTree *n) const;
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  bool isPlLogicalCompound() const { return true; }
  void translateExceptionLiterDefinition(Codestream &str, WhenExpr *whenExpr);
  void translateLinterExceptionDeclarations(Codestream &str, Sm::DeclaredExceptions &declExc);
  void linterDefinition (Sm::Codestream &s);
  void resolve(ResolvedEntity *owner) { if (lhs) lhs->resolve(owner); if (rhs) rhs->resolve(owner); }
  virtual bool isComplexExpr() const { return true; }
  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs); }
  void enumSubExpressions(List<PlExpr> &exprList);

  ScopedEntities ddlCathegory() const { return LogicalCompound_; }
};
class CurrentOf      : public PlExpr  {
public:
  Ptr<Id> cond;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LUCONSTRUCTOR1(CurrentOf, cond) {}
  void collectSNode(SemanticTree *n) const { SNODE(n); if (cond) n->addChild(cond->toSNodeRef(SCathegory::CursorEntity)); }
  Ptr<Datatype> getDatatype() const { return Datatype::mkBoolean(); }
  void replaceChildsIf(ExprTr tr) { replace(tr, cond); }
};

}

}

#endif
// vim:foldmethod=marker:foldmarker={,}
