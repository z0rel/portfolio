#ifndef SEMANTIC_EXPR_SELECT_H
#define SEMANTIC_EXPR_SELECT_H

#include "semantic_id_lists.h"
#include "semantic_datatype.h"

namespace Sm {
  using namespace std;
  using namespace dstring;


// https://docs.oracle.com/cd/B10500_01/server.920/a96533/hintsref.htm
class QueryHint : public ResolvedEntityLoc {
public:
  enum Cathegory {
    INDEX,
    INDEX_DESC,
    ORDERED,
    NO_MERGE,
    FIRST_ROWS,
    ALL_ROWS,
    CHOOSE,
    INLINE,
    LEADING,
    USE_NL
  };
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  static const std::map<string, Cathegory> kwMap;
public:

  Cathegory      cathegory = INDEX;
  Ptr<List<Id> > argList;

  ScopedEntities ddlCathegory() const { return QueryHint_; }
  Ptr<Datatype>  getDatatype()  const { return 0; }
  void collectSNode(SemanticTree *) const {}

  QueryHint(CLoc l, List<Id> *argList);
  QueryHint(CLoc l, Cathegory cat);
  void parseCathegory(Ptr<Id> cat);
  static QueryHint *checkFrontKeyword(Ptr<Id> cat);
};


typedef List<QueryHint> QueryHintContainer;


class QueryPseudoField : public ResolvedEntitySNode {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  Ptr<Id> fieldName;
  ResolvedEntity *owner_ = 0;
  ResolvedEntity *fieldDefinition_ = 0;

public:
  Sm::SelectedField *selectedFieldOwner = 0;
  bool isColumnValue = false;
protected:
  bool nameTranslated_ = false;
  int fieldNumber_ = 0;

public:

  QueryPseudoField(Ptr<Id> _fieldName, ResolvedEntity *_owner);
  QueryPseudoField(Ptr<Id> _fieldName, ResolvedEntity *_owner, ResolvedEntity *fieldDefinition);

  inline void setFieldNumber(int n);

  bool isNullField() const { return fieldDefinition_ && fieldDefinition_->isNullField(); }
  bool isNonblockPseudoField() const;
  void checkDef() const;
  bool semanticResolve() const { checkDef(); return true; }
  Ptr<Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Id> &field);
  Ptr<Id> getName() const { return fieldName; }
  bool isField() const { return true; }
  bool isSqlIdentificator() const { return true; }

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  bool isExactlyEquallyByDatatype(ResolvedEntity *oth);
  void linterDefinition(Codestream &str);
  void linterReference (Codestream &str);
  void sqlReference    (Codestream &str);
  void translateAsFunArgumentReference(Sm::Codestream &str) { str << fieldName; }
  void translateNameIfSpecial();

  ResolvedEntity *getNextDefinition() const { return fieldDefinition_; }
  ScopedEntities ddlCathegory() const { return QueriedPseudoField_; }

  SemanticTree *toSTreeBase() const { return fieldName->toSNodeDecl(SCathegory::QueryPseudoField, this); }
  void definition(ResolvedEntity *def) { fieldDefinition_ = def; }
  ResolvedEntity* definition() const { checkDef(); return fieldDefinition_; }
  ResolvedEntity *getFieldDDLContainer() const { return owner_; }

  QueryPseudoField *toSelfQueryPseudoField() const { return const_cast<QueryPseudoField*>(this); }
};


class RefAbstract;

class OrderByItem : public GrammarBaseSmart, public Translator {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum OrderCathegory {
      EMPTY            = 0,
      RAW_ASC          = 1 << 1,
      RAW_DESC         = 1 << 2,
      NULLS_FIRST      = 1 << 3,
      NULLS_LAST       = 1 << 4,
      ASC              = RAW_ASC  | NULLS_LAST,
      DESC             = RAW_DESC | NULLS_FIRST,
      ASC_NULLS_LAST   = ASC,
      DESC_NULLS_FIRST = DESC,
      ASC_NULLS_FIRST  = RAW_ASC  | NULLS_FIRST,
      DESC_NULLS_LAST  = RAW_DESC | NULLS_LAST
  };
  Ptr<SqlExpr> expr;
  OrderCathegory orderCathegory;

  void oracleDefinition(Sm::Codestream &str);
  void sqlDefinition(Sm::Codestream &str);
  string orderCathegoryString() const;
  DEF_LBCONSTRUCTOR2(OrderByItem, expr, orderCathegory) {}
  void collectSNode(SemanticTree *n) const;
  virtual ~OrderByItem() {}
  void replaceChildsIf(ExprTr tr) { replace(tr, expr); }
};

class FactoringItem;


class QueryBlock;
class SelectSingle;

class FactoringItem        : public ResolvedEntitySNodeLoc {
protected:
  mutable Ptr<Datatype>       cachedDatatype;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

public:
  Ptr<Id>        queryName;
  /**
   * Список столбцов-псевдонимов для внутренних полей запроса.
   *
   * Определение псевдонима для столбцового выражения. Oracle использует этот
   * псевдоним как заголовк столбца результирующего набора. Ключевое слово AS - опциональное.
   * Псевдоним эффективно переименовывает элемент списка выборки на всё время выполнения запроса.
   * Псевдоним может использоваться в order_by_clause, но не других пунктах запроса.
   */
  Ptr<List<Id> > columnAliases;
  Ptr<SqlExpr>   subquery;

  DEF_LUCONSTRUCTOR3(FactoringItem, queryName, columnAliases, subquery) {
    queryName->definition((FactoringItem*)this);
    sAssert(!subquery);
  }

  SemanticTree *toSTreeBase() const;

  void linterReference (Sm::Codestream &str);


  bool getFieldRef(Ptr<Sm::Id> &field);
  Ptr<Sm::Id> getName() const { return queryName; }
  ResolvedEntity* getNextDefinition() const;
  bool getFieldRefAsTableName(Ptr<Sm::Id> &field);
  bool getFields(EntityFields &fields) const;

  void sqlDefinition(Codestream &str);
  ScopedEntities ddlCathegory() const { return FactoringItem_; }
  bool usedInQueryAndContainsFields() { return true; }
  Ptr<Datatype> getDatatype() const;
  void connectAliasWithQuery() const;
  virtual ~FactoringItem() {}

  bool isSqlIdentificator() const { return true; }

  void replaceChildsIf(ExprTr tr) { replace(tr, queryName, columnAliases, subquery); }
};
class SelectedField        : public ResolvedEntityLoc {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<QueryPseudoField> pseudoField;
  Ptr<SqlExpr> expr_;
  Ptr<Id>      alias_;
  mutable Ptr<Id> fieldName;

  int fieldNumber_ = 0;
  bool fieldNameReallyEmpty = false;
  Ptr<IdRef>  asteriksRef;

  void setFieldNumber(int i) { fieldNumber_ = i; }

  Ptr<CallArgList> callArglist() const { return expr_ ? expr_->callArglist() : Ptr<CallArgList>(0); }
  bool isNumericLiteral() const { return expr_ && expr_->isNumericLiteral(); }
  bool isEmptyId() const { return expr_ && expr_->isEmptyId(); }
  SelectedField* toSelfSelectedField() const { return const_cast<SelectedField*>(this); }

  Ptr<Id> getName  () const;
  Ptr<Id> getAlias () const { return alias_; }
  Ptr<SqlExpr> expr() const { return expr_; }

  SelectedField(CLoc l, Ptr<SqlExpr> expr, Ptr<Id> _alias = 0);

  void collectSNode(SemanticTree *n) const;

  void translateAsFunArgumentReference(Sm::Codestream &str);
  void linterReference(Sm::Codestream &str);

  bool skipLevelNamespaceUpdating() const;
  bool isNullField() const { return expr_ && expr_->isNull(); }
  bool isField() const { return true; }
  bool isSqlIdentificator() const { return true; }
  Id* toAsterisk() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return expr_ ? expr_->isSubtype(supertype, plContext) : EXPLICIT; }
  Ptr<Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Id> &field)  { return expr_ && expr_->getFieldRef(field); }
  ScopedEntities ddlCathegory() const { return SqlSelectedField_; }
  void sqlDefinition(Codestream &str);
  virtual bool isExactlyEquallyByDatatype(ResolvedEntity *t);
  virtual ~SelectedField() { clrSTree(); }
  ResolvedEntity *getFieldDDLContainer() const;

  void replaceChildsIf(ExprTr tr) { replace(tr, expr_, alias_); }
};
class SelectList           : public GrammarBaseSmart, public virtual Translator, public virtual ResolvedEntity {
public:
  typedef Vector<SelectedField> SelectedFields;
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  mutable Sm::SemanticTree *semanticNode = 0;
  Ptr<SelectedFields> fields;
  bool                isAsterisk_ = false;
  List<Id>            asterisksList;

  SelectList(CLoc l, SelectedFields *_fields = 0);
  
  bool isSingleField();

  bool isAsterisk() const { return isAsterisk_; }
  void isAsterisk(bool val) { isAsterisk_ = val; }
  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);
  bool getFields(EntityFields &fields) const;
  Sm::IsSubtypeValues isSubtypeSingle(Ptr<ResolvedEntity> supertype, bool plContext) const;
  Ptr<SelectedFields> selectedFields() const { return fields; }
  SemanticTree *toSTree() const;
  void sqlDefinition(Codestream &str);
  virtual ~SelectList() {}
  Ptr<Sm::Datatype> getDatatype()  const { return 0; }
  ScopedEntities ddlCathegory() const { return SelectList_; }

  void replaceChildsIf(ExprTr tr) { replace(tr, fields, asterisksList); }
};
class Tablesample          : public GrammarBaseSmart {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool              block;
  Ptr<NumericValue> blockVal;
  Ptr<NumericValue> seedVal;

  DEF_LBCONSTRUCTOR3(Tablesample, block, blockVal, seedVal) {}
  DEF_LBCONSTRUCTOR2(Tablesample, block, blockVal) {}

  Tablesample() : block(0) {}
  inline SemanticTree *toSTree() const { return 0; }

  void replaceChildsIf(ExprTr tr) { replace(tr, blockVal, seedVal); }
};


class FlashbackQueryClause : public GrammarBaseSmart {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  class AsOf {
  protected:
    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  public:
     flashback_query::ScnOrTimestamp t = flashback_query::EMPTY;
     Ptr<SqlExpr> expr;

     DEF_CONSTRUCTOR2(AsOf, t, expr) {}
     AsOf() {}
  } asOf;
  struct VersionBetween {
     flashback_query::ScnOrTimestamp t = flashback_query::EMPTY;
     Ptr<SqlExpr> range[2];

     VersionBetween(flashback_query::ScnOrTimestamp _t, Ptr<SqlExpr> r1, Ptr<SqlExpr> r2) : t(_t) { range[0] = r1; range[1] = r2; }
     VersionBetween() {}
  } versionBetweenAsOf;

  FlashbackQueryClause(CLoc l, flashback_query::ScnOrTimestamp aot, Ptr<SqlExpr> e, flashback_query::ScnOrTimestamp vbt, Ptr<SqlExpr> r1, Ptr<SqlExpr> r2);
  FlashbackQueryClause(CLoc l, flashback_query::ScnOrTimestamp aot, Ptr<SqlExpr> e);
  FlashbackQueryClause(CLoc l);
  FlashbackQueryClause() {}

  void collectSNode(SemanticTree *n) const;

  void replaceChildsIf(ExprTr tr) { replace(tr, asOf.expr, versionBetweenAsOf.range[0], versionBetweenAsOf.range[1]); }
};

class QueriedTable : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum QueryCathegory {
    EMPTY,
    /// для иерархических VIEW
    ONLY,
    PIVOT,
    UNPIVOT
  };
  QueryCathegory        queryCathegory = EMPTY;


  virtual Ptr<Id> getAlias() const { return Ptr<Id>(); }
  virtual bool getFieldRef(Ptr<Sm::Id> &field) = 0;
  virtual bool getFieldRefAsTableName(Ptr<Id> &field) = 0;
  ScopedEntities ddlCathegory() const { return QueriedTable_; }
  virtual bool isDual() const { return false; }

  bool usedInQueryAndContainsFields() { return true; }

  QueriedTable() {}
  QueriedTable(CLoc l) : GrammarBase(l) {}
  virtual ~QueriedTable() {}

  virtual void replaceChildsIf(ExprTr tr) = 0;
};
class FromTableReference       : public QueriedTable {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>  tableReference;
public:
  Ptr<Id2>         id;
  Ptr<Id>          dblink; // id only for ID_REMOTE_DB
  /** sample_clause - команда: - делать select по случайной выборке данных из таблицы, а не по всей таблице.  */
  Ptr<Tablesample> sample;

  void sqlDefinition(Codestream &str);
  Ptr<Sm::Datatype> getDatatype() const;
  Ptr<Id> getName()  const { return (id) ? Ptr<Id>(id->entity()) : Ptr<Id>(); }
  Ptr<Id2> getName2() const { return id; }
  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const;
  virtual bool getFieldRefAsTableName(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const { return id && id->definition() && id->definition()->getFields(fields); }
  UserContext* userContext() const;

  FromTableReference(Ptr<Id2> _id, Ptr<Id> _dblink, Ptr<Tablesample> _sample, CLoc l = cl::emptyFLocation());

  bool haveFactoringItem();
  bool isDual() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  SemanticTree *toSTreeBase() const;
  void linterReference(Codestream &str);

  FromTableReference* toSelfFromTableReference() const { return const_cast<FromTableReference*>(this); }

  void replaceChildsIf(ExprTr tr) { replace(tr, tableReference, id, sample); }
};
class FromTableSubquery        : public QueriedTable {
  Ptr<Subquery> subquery;
  /// THE ( <подзапрос> )
  bool hasThe;
  /// <подзапрос> WITH (READ ONLY)|(CHECK OPTION [CONSTANT constraint])
  bool isWith;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  FromTableSubquery(CLoc l = cl::emptyFLocation(), Ptr<Subquery> q = 0, bool _hasThe = false, bool _isWith = false);

  Ptr<Id> getName()  const { return subquery->getName(); }
  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const { return subquery ? subquery->getNextDefinition() : (ResolvedEntity*)0; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return subquery->isSubtype(supertype, plContext); }
  void sqlDefinition(Codestream &str) { str << s::obracket << subquery << s::cbracket; }

  bool getFieldRefAsTableName(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const { return subquery && subquery->getFields(fields); }

  SemanticTree *toSTreeBase() const;
  Ptr<Sm::Datatype> getDatatype() const { return subquery ? subquery->getDatatype() : Ptr<Datatype>(); }

  void replaceChildsIf(ExprTr tr);
};
/// TABLE ( <выражение> )
class FromTableDynamic         : public QueriedTable {
  Ptr<SqlExpr> dynamicTable;
  mutable Ptr<QueryPseudoField> columnValue;

  typedef std::vector<Ptr<QueryPseudoField> > DynamicFields;
  typedef std::map<ResolvedEntity*, Ptr<QueryPseudoField>, LE_ResolvedEntities> DynamicFieldsMap;
  mutable DynamicFields fields_;
  mutable DynamicFieldsMap fieldsMap;

  /// TABLE ( <выражение> (+) )
  bool hasLJoin;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  void getDynamicFields(SqlExpr *dynamicTable, EntityFields &f) const;
public:
  FromTableDynamic(CLoc l = cl::emptyFLocation(), Ptr<SqlExpr> dt = 0, bool hj = false) : QueriedTable(l), dynamicTable(dt), hasLJoin(hj) {}

  SemanticTree *toSTreeBase() const;

  void sqlDefinition(Codestream &str);
  Ptr<Sm::Datatype> getDatatype() const { return dynamicTable ? dynamicTable->getDatatype() : Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return dynamicTable->isSubtype(supertype, plContext); }
  bool getFieldRefAsTableName(Ptr<Id> &field);
  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const;
  bool getFields(EntityFields &fields) const;
  FromTableDynamic* toSelfFromTableDynamic() const { return (FromTableDynamic *)this; }

  Ptr<Id> getName()  const { return dynamicTable->getName(); }

  void replaceChildsIf(ExprTr tr) { replace(tr, dynamicTable); }
protected:
  void initDynamicFields() const;
};


class Join                 : public virtual GrammarBaseSmart, public Translator {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  JoinQueries::Operation op = JoinQueries::EMPTY;

  std::string toJoinOpSting() const;
  virtual void sqlDefinition(Codestream &str) = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~Join() {}
};
class JoinOnDefault        : public Join {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void collectSNode(SemanticTree *) const {}
  void sqlDefinition(Codestream &) {}
};
class JoinCondition        : public Join {
  Ptr<PlExpr> condition;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  JoinCondition() {}
  JoinCondition(CLoc l, PlExpr* cond)
    : GrammarBaseSmart(l), condition(cond) {}
  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Codestream &str) { str << s::name << "ON" << s::name << condition; }
};
class JoinOnFieldList      : public Join {
  Ptr<List<Id> > onFieldList;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  JoinOnFieldList(CLoc l, List<Id> *onList)
    : GrammarBaseSmart(l), onFieldList(onList) {}

  void collectSNode(SemanticTree *n) const {
    for (List<Id>::const_iterator it = onFieldList->begin(); it != onFieldList->end(); ++it)
      n->addChild(it->object()->toSNodeRef(SCathegory::Field));
  }
  void sqlDefinition(Codestream &str) { str << s::name << "USING" << s::name << s::obracket << onFieldList << s::cbracket; }
};

class From    : public ResolvedEntityLoc {
public:
  enum CathegoryFrom { FROM_SINGLE, FROM_JOIN };
protected:
  bool hasBrackets_ = false;
  /// Не проверять псевдонимы в getFieldRef
public:
  bool noCheckAlias = false;

  From(CLoc l = cl::emptyFLocation()) : GrammarBase(l) {}
  virtual CathegoryFrom  cathegoryFrom() const = 0;

  virtual Ptr<Datatype> getDatatype() const = 0;
  virtual Ptr<Id> getAlias() const = 0;
  virtual bool getFieldRef(Ptr<Id> &field) = 0;
  virtual bool getFieldRefAsTableName(Ptr<Id> &field) = 0;
  virtual bool getFields(vector<Ptr<Id> > &fields) const = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;

  virtual bool isDual() const { return false; }
  void setHasBrackets() { hasBrackets_ = true; }
  bool usedInQueryAndContainsFields() { return true; }
  virtual ~From() {}

  virtual void replaceChildsIf(ExprTr tr) = 0;
};

class FromSingle           : public From {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Ptr<Id> referenceName;
  mutable Ptr<Datatype>     cachedDatatype;
public:
  Ptr<QueriedTable>         reference;
  Ptr<FlashbackQueryClause> flashback;
  Ptr<Id>                   alias;
  Ptr<List<SqlExpr> >       partitionExprList;

  Ptr<Sm::Datatype> getDatatype() const;
  Ptr<Id>  getAlias() const { return alias ? alias : reference->getAlias(); }
  Ptr<Id>  getName () const { Ptr<Id> name = reference->getName(); return name ? name : getAlias(); }
  Ptr<Id2> getName2() const { return reference ? reference->getName2() : Ptr<Id2>(0); }

  FromSingle(CLoc l, Ptr<QueriedTable> _reference, Ptr<FlashbackQueryClause> _flashback, Ptr<Id> _alias);

  virtual bool getFields(EntityFields &fields) const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  virtual bool getFieldRefAsTableName(Ptr<Id> &field);
  virtual bool getFieldRef(Ptr<Id> &field);
  void sqlDefinition(Codestream &str);
  void linterReference(Codestream &str);
  bool isDual() const { return (reference) ? reference->isDual() : false; }

  bool isSqlIdentificator() const { return true; }

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return FromSingle_; }
  CathegoryFrom  cathegoryFrom() const { return FROM_SINGLE; }
  void initReferenceName() const;

  void replaceChildsIf(ExprTr tr) { replace(tr, reference, flashback, alias, partitionExprList); }
  FromSingle* toSelfFromSingle() const { return const_cast<FromSingle*>(this); }
};
class FromJoin             : public From {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<From> lhs;
  Ptr<Join> op;
  Ptr<From> rhs;

  Ptr<Datatype> getDatatype() const;
  // join в чистом синтаксисе не имеет псевдонимов и имен
  Ptr<Id> getAlias() const { return Ptr<Id>(); }

  virtual bool getFieldRef(Ptr<Id> &field);
  virtual bool getFieldRefAsTableName(Ptr<Id> &field);

  bool getFields(EntityFields &fields) const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool) const { return eqByVEntities(supertype) ? EXACTLY_EQUALLY : EXPLICIT; }
  void sqlDefinition(Codestream &str);

  FromJoin(CLoc l, Ptr<From> _lhs, Ptr<Join> _op, Ptr<From> _rhs);
  void collectSNode(SemanticTree *n) const;
  void toSTreeBaseInQueryBlock(SemanticTree *&node) const;
  ScopedEntities ddlCathegory() const { return FromJoin_; }
  CathegoryFrom  cathegoryFrom() const { return FROM_JOIN; }

  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs); }
};

class GroupingSetsClause : public virtual GrammarBaseSmart {
public:
  typedef List<List<SqlExpr> > CubeT;
  void collectCube(const Ptr<CubeT> &cube, SemanticTree *n) const;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~GroupingSetsClause() {}
  virtual void replaceChildsIf(ExprTr tr) = 0;
};

namespace grouping_sets {
class Cube : public GroupingSetsClause {
  Ptr<CubeT> cube;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LCONSTRUCTOR1(Cube, cube) { if (!cube) throw 999; }
  void collectSNode(SemanticTree *n) const { collectCube(cube, n); }
  void replaceChildsIf(ExprTr tr) { replace(tr, *cube); }
};
class Rollup : public GroupingSetsClause {
  Ptr<CubeT> rollup;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LCONSTRUCTOR1(Rollup, rollup) { if (!rollup) throw 999; }
  void collectSNode(SemanticTree *n) const { collectCube(rollup, n); }
  void replaceChildsIf(ExprTr tr) { replace(tr, *rollup); }
};
class Single : public GroupingSetsClause {
  Ptr<List<SqlExpr> >       simple;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void collectSNode(SemanticTree *n) const { CollectSNode(n, simple); }
  DEF_LCONSTRUCTOR1(Single, simple) { if (!simple) throw 999; }
  void replaceChildsIf(ExprTr tr) { replace(tr, *simple); }
};

}

class GroupBySqlExpr       : public GroupBy {
  Ptr<SqlExpr> sqlExpr;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DEF_LCONSTRUCTOR1(GroupBySqlExpr, sqlExpr) {}

  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Codestream &str);
  CathegoryGroupBy cathegoryGroupBy() const { return SQL_EXPR; }
  void replaceChildsIf(ExprTr tr) { replace(tr, sqlExpr); }
};
class GroupByRollupCubes   : public GroupBy {
  List<GroupingSetsClause> cubes;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  GroupByRollupCubes(CLoc l, Ptr<List<GroupingSetsClause> > _cubes) : GrammarBaseSmart(l) { cubes.swap(*_cubes); }
  GroupByRollupCubes(CLoc l, Ptr<GroupingSetsClause>         cube)  : GrammarBaseSmart(l) { cubes.push_back(cube); }

  void collectSNode(SemanticTree *n) const { CollectSNode(n, cubes); }
  void sqlDefinition(Codestream &str) { trError(str, s << "Grouping by cube doesn't support " << getLLoc()); }
  CathegoryGroupBy cathegoryGroupBy() const { return ROLLUP_CUBE; }

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, cubes); }
};
class HierarhicalClause     : public GrammarBaseSmart, public Translator  {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool isNocycle;
  Ptr<PlExpr> connectCondition;
  Ptr<PlExpr> startWithCondition;

  HierarhicalClause() : isNocycle(false) {}
  DEF_LBCONSTRUCTOR3(HierarhicalClause, isNocycle, connectCondition, startWithCondition) {}

  inline bool empty() const { return !(connectCondition || startWithCondition); }
  bool isPseudoSequence() const;
  void translateSeqCondition(Sm::Codestream &str);

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &);
  virtual ~HierarhicalClause() {}

  void replaceChildsIf(ExprTr tr) { replace(tr, connectCondition, startWithCondition); }
};


class QueryBlock           : public virtual ResolvedEntitySNodeLoc, public FromResolver {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable SemanticTree *fromNode_         = 0;
  mutable SemanticTree *factoringNode_    = 0;
  mutable SemanticTree *selectedListNode_ = 0;
  mutable Ptr<Datatype> thisDatatype;
  Ptr<Sm::QueryPseudoField> levelPseudoField;
  Ptr<SqlExpr> limit;
public:
  typedef Ptr<List<RefAbstract> > IntoList;

  Ptr<FactoringList>        factoringList;
  query_block::QueryPrefix  queryPrefix = query_block::EMPTY;

  Ptr<SelectList>           selectList;
  Subquery::IntoList        intoList;
  Ptr<FromList>             from;
  Ptr<PlExpr>               where;

  Ptr<QueryTailData>        tailSpec;

  // Ptr<HierarhicalClause>    hierarhicalSpec;
  // Ptr<List<GroupBy> >       groupBy;
  // Ptr<PlExpr>               having;

  bool                      hasModelClause       = false;
  bool                      toStreeFactoringList = true;
  bool                      isUnionChild_        = false;
  bool                      bulkCollect          = false;

  
  bool isSingleField();
  
  Ptr<ForUpdateClause>  forUpdate;
  Ptr<OrderBy>    orderBy;

  SelectSingle *ownerSelectModel = 0;

  Ptr<QueryHintContainer>  hints;


  void isBulkCollect(bool val) { bulkCollect = val; }
  bool isBulkCollect() const { return bulkCollect; }

  bool findInFactoringList(Ptr<Id> &factoringTable) const;

  ScopedEntities ddlCathegory() const { return QueryBlock_; }

  Ptr<Sm::Datatype> getDatatype() const;

  void getWhereResolvedNamespace(Sm::SemanticTree::Childs &dstNamespace) const;

  bool getFieldRef(Ptr<Id> &field);
  bool getFieldRefFromRootQuery(Ptr<Id> &field);
  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference);
  bool getFieldRefByFromListFromRootQuery(Ptr<Id> &field, Sm::SemanticTree *reference);
  bool getFields(EntityFields &fields) const;
  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);
  void extractLimit();

  void translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory castedCathegory);
  void setLimitExpr(Ptr<pl_expr::Comparsion> &cmp, int isNot);
  Ptr<SqlExpr> getLimitExpr() const { return limit; }
  bool hasDynamicQueryExpr() const;

  SemanticTree *fromNode()      const { return fromNode_; }
  SemanticTree *factoringNode() const { return factoringNode_; }

  QueryBlock() {}
  QueryBlock(
    CLoc l,
    Ptr<List<FactoringItem> > _factoringList,
    query_block::QueryPrefix  _queryPrefix,
    Ptr<SelectList>           _selectList,
    Ptr<List<RefAbstract> >   _intoList,
    Ptr<List<From> >          _from,
    Ptr<PlExpr>               _where,
    Ptr<QueryTailData>        _tail,
    bool                      _hasModelClause
  ) : GrammarBase    (l),
      factoringList  (_factoringList  ),
      queryPrefix    (_queryPrefix    ),
      selectList     (_selectList     ),
      intoList       (_intoList       ),
      from           (_from           ),
      where          (_where          ),
      tailSpec       (_tail           ),
      hasModelClause (_hasModelClause ) {}

  SemanticTree *toSTreeBase() const;


  void sqlDefinition(Codestream &str);
  void sqlDefinitionForNthField(Codestream &str, int fieldPos);
  void linterReference(Sm::Codestream &str); // вывести курсорный тип
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  virtual bool isExactlyEquallyByDatatype(ResolvedEntity*);
  Sm::QueryBlock *toSelfQueryBlock() const { return (QueryBlock*)this; }
  void sqlDefinitionHead(Codestream &str);
  void sqlDefinitionFromWhereEtc(Codestream &str);

  void replaceChildsIf(ExprTr tr);

};


class SelectSingle        : public Subquery {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<QueryBlock> queryBlock;

  SelectSingle(CLoc l, Ptr<QueryBlock> q);

  IntoList          intoList() const { return queryBlock->intoList; }
  Id *getSelectedFieldName() const { return 0; }

  Ptr<Sm::Datatype> getDatatype() const { return queryBlock ? queryBlock->getDatatype() : Ptr<Datatype>(); }
  ResolvedEntity*   getNextDefinition() const { return queryBlock ? queryBlock->getNextDefinition() : (ResolvedEntity*)0; }

  bool getFieldRef(Ptr<Id> &field);
  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference) { return queryBlock->getFieldRefByFromList(field, reference); }
  bool getFields(EntityFields &fields) const { return queryBlock && queryBlock->getFields(fields); }

  void translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory cat);
  void formalTranslations();

  Sm::IsSubtypeValues  isSubtype(ResolvedEntity *supertype, bool plContext) const { return queryBlock->isSubtype(supertype, plContext); }

  ScopedEntities            ddlCathegory()      const { return SqlExprSelectSingle_; }
  virtual CathegorySubquery cathegorySubquery() const { return SINGLE_SELECT; }

  virtual bool isBulkCollect() const { return queryBlock && queryBlock->isBulkCollect(); }

  Ptr<List<FactoringItem> > pullUpFactoringList() const;
  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);

  void setIsUnionChild() { queryBlock->isUnionChild_ = true; }
  bool isUnionChild() const { return queryBlock->isUnionChild(); }
  bool hasDynamicQueryExpr() const { return queryBlock->hasDynamicQueryExpr(); }

  SemanticTree *toSTreeBase() const;
  void collectSNode(SemanticTree *node) const;
  void sqlDefinition(Codestream &str);
  void sqlDefinitionForNthField(Codestream &str, int fieldPos) { queryBlock->sqlDefinitionForNthField(str, fieldPos); }
  void replaceChildsIf(ExprTr tr) { replace(tr, queryBlock, forUpdate, orderBy, groupBy); }
  SelectSingle *toSelfSelectSingle() const { return const_cast<SelectSingle*>(this); }
  void extractLimitInQueryBlocks();
};

class SelectBrackets      : public Subquery {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Subquery> subquery;
  SelectBrackets(CLoc l, Ptr<Subquery> q);

  IntoList          intoList()          const { return subquery->intoList(); }
  Id *getSelectedFieldName() const { return 0; }
  Ptr<Sm::Datatype> getDatatype()       const { return subquery ? subquery->getDatatype() : Ptr<Datatype>(); }
  ResolvedEntity*   getNextDefinition() const { return subquery ? subquery->getNextDefinition() : (ResolvedEntity*)0; }

  bool getFieldRef(Ptr<Id> &field);
  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference) { return subquery->getFieldRefByFromList(field, reference); }
  bool getFields(EntityFields &fields) const { return subquery && subquery->getFields(fields); }

  void translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory cat);

  virtual CathegorySubquery cathegorySubquery() const { return BRACKETS; }
  Sm::IsSubtypeValues  isSubtype(ResolvedEntity *supertype, bool plContext) const { return subquery->isSubtype(supertype, plContext); }
  ScopedEntities ddlCathegory() const { return SqlExprSelectBrackets_; }

  virtual bool isBulkCollect() const { return subquery && subquery->isBulkCollect(); }
  bool hasDynamicQueryExpr() const { return subquery->hasDynamicQueryExpr(); }

  SelectBrackets *toSelfSelectBrackets() const { return const_cast<SelectBrackets*>(this); }

  Ptr<List<FactoringItem> > pullUpFactoringList() const { return subquery ? subquery->pullUpFactoringList() : Ptr<List<FactoringItem> >(); }

  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);
  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
  void sqlDefinition(Codestream &str);
  void sqlDefinitionForNthField(Codestream &str, int fieldPos);

  void setIsUnionChild() { subquery->setIsUnionChild(); }
  bool isUnionChild() const { return subquery->isUnionChild(); }
  void replaceChildsIf(ExprTr tr);
  void extractLimitInQueryBlocks() { subquery->extractLimitInQueryBlocks(); }
  void initQueryFieldPos() { subquery->initQueryFieldPos(); }
private:
  SemanticTree *toSTreeBaseHeadNode() const;
  void collectBodyNodes(SemanticTree *n) const;
};


class UnionQuery               : public Subquery {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  bool isUnionChild_ = false;
public:
  Ptr<Subquery>  lhs;
  sql_union::UnionOperation op;
  Ptr<Subquery>  rhs;

  UnionQuery(CLoc l, Ptr<Subquery> _lhs, sql_union::UnionOperation uop, Ptr<Subquery> _rhs);

  CathegorySubquery cathegorySubquery() const { return UNION; }

  IntoList intoList() const { IntoList l; return ((l = lhs->intoList())) ? l : rhs->intoList(); }
  Id *getSelectedFieldName() const { return 0; }

  Ptr<Sm::Datatype> getDatatype() const;
  ResolvedEntity*   getNextDefinition() const;

  bool getFieldRef(Ptr<Id> &field);
  bool getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference);
  bool getFields(EntityFields &fields) const;

  ScopedEntities ddlCathegory() const { return SqlExprUnion_; }

  virtual bool isBulkCollect() const { return lhs && lhs->isBulkCollect(); }
  bool hasDynamicQueryExpr() const { return lhs->hasDynamicQueryExpr() || rhs->hasDynamicQueryExpr(); }

  void translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory castedCathegory);
  void convertUnionFieldTypes(EntityFields &intoFlds, bool isFirstQuery);
  void formalTranslations();

  bool isUnionChild() const { return isUnionChild_; }
  void setIsUnionChild();
  Sm::IsSubtypeValues  isSubtype(ResolvedEntity *supertype, bool plContext) const;

  Ptr<List<FactoringItem> > pullUpFactoringList() const { return lhs ? lhs->pullUpFactoringList() : Ptr<List<FactoringItem> >(); }
  void assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it);

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
private:
  void collectBodyNodes(SemanticTree *n) const;
  SemanticTree *toSTreeBaseHeadNode() const;
  bool isRootQuery() const { return !isUnionChild_ || isSqlStatementRoot; }
public:
  void sqlDefinition(Codestream &str);
  void sqlDefinitionForNthField(Codestream &str, int fieldPos);

  void sqlDefinitionOperation(Codestream &str);
  void replaceChildsIf(ExprTr tr) { replace(tr, lhs, rhs, forUpdate, orderBy, groupBy); }

  void extractLimitInQueryBlocks();
  void initQueryFieldPos();
};


inline Codestream& operator<<(Codestream& s, OrderByItem &obj) { return obj.translate(s); }

class QueryTailData : public Smart {
public:
  QueryTailData(Sm::PlExpr* h, Sm::List<Sm::GroupBy> *gb = 0, Sm::HierarhicalClause *hq = 0)
    : having(h), groupBy(gb), hierarhicalSpec(hq) {}

  Ptr<Sm::PlExpr>             having;
  Ptr<Sm::List<Sm::GroupBy> > groupBy;
  Ptr<Sm::HierarhicalClause>  hierarhicalSpec;

  void replaceChildsIf(ExprTr tr) { replace(tr, hierarhicalSpec, groupBy, having); }
  void collectSNode(SemanticTree *node) const;
};

inline void QueryPseudoField::setFieldNumber(int n) {
  if (selectedFieldOwner)
    selectedFieldOwner->setFieldNumber(n);
  else
    fieldNumber_ = n;
}

}

#endif // SEMANTIC_EXPR_SELECT_H
// vim:foldmethod=marker:foldmarker={,}
