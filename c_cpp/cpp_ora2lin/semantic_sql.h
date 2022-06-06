#ifndef SEMANTIC_SQL_H
#define SEMANTIC_SQL_H

#include <map>
#include <sstream>
#include <algorithm>
#include "app_conf.h"
#include "semantic_table.h"
#include "semantic_datatype.h"
#include "semantic_expr.h"
#include "semantic_id.h"
#include "semantic_table.h"
#include "semantic_tree.h"
#include "semantic_base.h"

class SyntaxerContext;
class ModelContext;

namespace Sm {


ostream& operator<< (ostream& os, const SequenceBody& str);


inline Codestream& operator<<(Codestream& s, Sequence &obj) { return obj.translate(s); }


class Table;


namespace table {
  namespace field_property {
    class ObjectProperties;
    class ObjectField;
    class NestedTable;
    class XmlField;
    class VarrayField;
    class FieldProperty;
  }
}


namespace constraint  {


#define FLAG_CONSTRAINT_STATE_EMPTY               (0)
#define FLAG_CONSTRAINT_STATE_NOT_DEFFERABLE      (1 << 0)
#define FLAG_CONSTRAINT_STATE_DEFFERABLE          (1 << 1)
#define FLAG_CONSTRAINT_STATE_DISABLE             (1 << 2)
#define FLAG_CONSTRAINT_STATE_ENABLE              (1 << 3)
#define FLAG_CONSTRAINT_STATE_INITIALLY_DEFFERED  (1 << 4)
#define FLAG_CONSTRAINT_STATE_INITIALLY_IMMEDIATE (1 << 5)
#define FLAG_CONSTRAINT_STATE_NORELY              (1 << 6)
#define FLAG_CONSTRAINT_STATE_NOVALIDATE          (1 << 7)
#define FLAG_CONSTRAINT_STATE_RELY                (1 << 8)
#define FLAG_CONSTRAINT_STATE_VALIDATE            (1 << 9)
#define FLAG_CONSTRAINT_STATE_USING_INDEX         (1 << 10)
#define FLAG_CONSTRAINT_STATE_CASCADE             (1 << 13)


class ConstraintState : public GrammarBaseSmart   {
public:
protected:
public:
  Ptr<Index>   index;     // TODO: можно ли поместить это в контекст модели при инициализации?
  Ptr<Id2>     indexName;
  unsigned int flags = 0;
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void checkIndex() const { Sm::sAssert(index); /*TODO: нужно реализовать перенос индексов в модель*/ }

  bool isDisable() { return flags & FLAG_CONSTRAINT_STATE_DISABLE; }

  inline void concat(Ptr<ConstraintState> o) {
    if (!o)
      return;
    flags |= o->flags;
    if (o->index)
      index = o->index;
    if (o->indexName)
      indexName = o->indexName;
  }

  void linterDefinition(ostream &str);

  ConstraintState() { checkIndex(); }
  ConstraintState(CLoc l, unsigned int f) : GrammarBaseSmart(l), flags(f) { checkIndex(); }
  DEF_LBCONSTRUCTOR1(ConstraintState, indexName), flags(FLAG_CONSTRAINT_STATE_USING_INDEX) { checkIndex(); }
  DEF_LBCONSTRUCTOR1(ConstraintState, index), flags(FLAG_CONSTRAINT_STATE_USING_INDEX)     { checkIndex(); }

  void collectSNode(SemanticTree *n) const;
};

class Attribute : public virtual GrammarBaseSmart, public Translator {
  virtual bool eq_(Attribute* /*attr*/) const { return false; }
protected:
  Sm::ResolvedEntitySet indexedFields(Ptr<List<Id> > fields) const;
public:
  enum CathegoryAttribute {
    EMPTY            ,
    CHECK            ,
    FOREIGN_KEY      ,
    PRIMARY_KEY      ,
    UNIQUE           ,
    NOT_NULL         ,
    NULL_tok         ,
    REFERENCES_CLAUSE,
    ROW_MOVEMENT     ,
  };

  int cathegoryToModelActionFlags();

  virtual CathegoryAttribute cathegory() const = 0;
  bool eq(Ptr<Attribute> attr) const {  return (attr && attr->cathegory() == cathegory()) ? eq_(attr.object()) : false; }
  std::string key();
  virtual ~Attribute() {}
  virtual void replaceChildsIf(Sm::ExprTr) {}
  virtual void linterDefinitionPrivilegie(Codestream& ) {}

  virtual PrimaryKey* toSelfPrimaryKey() const { return 0; }
  virtual ForeignKey* toSelfForeignKey() const { return 0; }
  virtual CheckCondition* toSelfCheckCondition() const { return 0; }
  virtual Unique *toSelfUniqueCondition() const { return 0; }
};

inline Codestream& operator<<(Codestream& os, Attribute &attr) { return attr.translate(os); }

class CheckCondition  : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> condition;

  CheckCondition(CLoc l, Ptr<PlExpr> _cond)
    : GrammarBaseSmart(l), condition(_cond) {}
  void linterDefinition(Codestream& os);

  void replaceChildsIf(Sm::ExprTr tr)  { replace(tr, condition); }

  CathegoryAttribute cathegory() const { return CHECK; }
  CheckCondition *toSelfCheckCondition() const { return const_cast<CheckCondition*>(this); }
};
class ForeignReference : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id2>                 foreignTable;  // reference (Id2)
  Ptr<List<Id> >           foreignFields; // Resolve: столбцы принадлежат объекту. (Id)
  referenced_key::OnDelete onDeleteAction;

  ForeignReference(CLoc l, Ptr<Id2> _entityRef, Ptr<List<Id> > _columnList, referenced_key::OnDelete _onDelete)
    : GrammarBaseSmart(l),
      foreignTable(_entityRef),
      foreignFields(_columnList),
      onDeleteAction(_onDelete) {}

  Ptr<List<Id> > foreignColumnList() { return foreignFields; }

  void linterDefinition(Sm::Codestream &os);

  CathegoryAttribute cathegory() const { return REFERENCES_CLAUSE; }

  bool eq(ForeignReference* o) const;
  ResolvedEntity *entityDef() const;
};
class ForeignKey      : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
protected:
  bool eq_(Attribute *attr) const;
public:
  Ptr<List<Id> >        fieldListKey;  // (Id)
  Ptr<ForeignReference> referencedKey; // (primary by man)

  ForeignKey(CLoc l, Ptr<List<Id> > _fieldListKey, Ptr<ForeignReference> _referencedKey);
  void linterDefinition(Codestream& os);
  void linterDefinitionPrivilegie(Codestream& os);

  CathegoryAttribute cathegory() const { return FOREIGN_KEY; }
  ForeignKey* toSelfForeignKey() const { return const_cast<ForeignKey*>(this); }
};
class PrimaryKey      : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<List<Id> > fieldListKey;    // (Id)

  PrimaryKey(CLoc l = cl::emptyFLocation(), Ptr<List<Id> > _fieldListKey = 0);
  void linterDefinition(Codestream& result);

  CathegoryAttribute cathegory() const { return PRIMARY_KEY; }
  PrimaryKey* toSelfPrimaryKey() const {  return const_cast<PrimaryKey*>(this); }
};
class Unique          : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<List<Id> > fieldListKey;

  Unique(CLoc l = cl::emptyFLocation(), Ptr<List<Id> > _fieldListKey = 0)
    : GrammarBaseSmart(l), fieldListKey(_fieldListKey) {}
  void linterDefinition(Codestream &result);

  CathegoryAttribute cathegory() const { return UNIQUE; }
  Unique *toSelfUniqueCondition() const { return const_cast<Unique*>(this); }
};
class RowMovement     : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  RowMovement(CLoc l) : GrammarBaseSmart(l) {}
  CathegoryAttribute cathegory() const { return ROW_MOVEMENT; }
  virtual bool hasLinterEquivalent() const { return false; }
};
class NotNull         : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  NotNull(CLoc l = cl::emptyFLocation()) : GrammarBaseSmart(l) {}
  void linterDefinition(Codestream& s) { s << "NOT NULL"; }

  CathegoryAttribute cathegory() const { return NOT_NULL; }
};
class Null            : public Attribute {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Null(CLoc l) : GrammarBaseSmart(l) {}
  void linterDefinition(Codestream &s) { s << "NULL"; }
  CathegoryAttribute cathegory() const { return NULL_tok; }
};

}

namespace table {

class EnableDisableConstraint : public GrammarBaseSmart, public Translator {
public:
    //  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  Ptr<Constraint>         constraint;
  bool                    cascade = false;
  enable_disable_constraint::ValidateState           validateState = enable_disable_constraint::V_EMPTY;
  enable_disable_constraint::EnableState             enableState   = enable_disable_constraint::E_EMPTY;
  enable_disable_constraint::KeepDropState           keepDropState = enable_disable_constraint::K_EMPTY;

  EnableDisableConstraint() {}
  EnableDisableConstraint(
      CLoc        l,
      Constraint *c,
      bool        _cascade,
      enable_disable_constraint::ValidateState validating,
      enable_disable_constraint::EnableState   enabling,
      enable_disable_constraint::KeepDropState keepDrop)
    : GrammarBaseSmart(l), constraint(c), cascade(_cascade),
      validateState(validating),
      enableState(enabling), keepDropState(keepDrop) {}

  virtual ~EnableDisableConstraint() {}
};

/**
 * Характеристики заменяемости столбца.
 *
 * substitutable_column_clause.
 *
 * Предолжение о таком столбце показывает, являются ли взаимозаменяемыми друг
 * для друга столбцы объекта или атрибуты объекта в одной той же иерархии.
 * Можно определить, что столбец имеет определенный тип, или может ли он
 * содержать экземпляры своих подтипов (или оба этих ограничения).
 */
class SubstitutableProperty   : public GrammarBaseSmart {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum CathegorySubstituable {
    /// default
    SUBSTITUTABLE_AT_ALL_LEVELS,
    /**
     * Столбец объекта не может содержать экземпляры, соответствующие любому из его подтипов.
     * Кроме того, замена отключена для любых встраиваемых атрибутов объектов и
     * элементов встроенных вложенных таблиц и varrays.
     */
    NOT_SUBSTITUTABLE_AT_ALL_LEVELS
  };

  Ptr<Datatype>         datatype;
  ///  Предложение  IS OF [TYPE] (ONLY тип) ограничивает тип столбца объекта до подтипа его объявленного типа.
  bool                  isOfType;
  /// Ограничиение типа элемента столбца коллекции или аттрибута до подтипа его объявленного типа.
  bool                  elementConstraint;
  CathegorySubstituable cathegorySubstituable;

  SubstitutableProperty() : isOfType(false), elementConstraint(false), cathegorySubstituable(SUBSTITUTABLE_AT_ALL_LEVELS) {}
  DEF_LBCONSTRUCTOR3(SubstitutableProperty, datatype, isOfType, elementConstraint), cathegorySubstituable(SUBSTITUTABLE_AT_ALL_LEVELS)   {}
  DEF_LBCONSTRUCTOR1(SubstitutableProperty, cathegorySubstituable)   {}

  void collectSNode(SemanticTree *n) const { ANODE(datatype) }
};



namespace field_property {

class FieldProperty : public virtual GrammarBaseSmart, public Translator {
public:
  virtual SmartVoidType* getThisPtr() const = 0;
  enum CathegoryFieldProperty { OBJECT_FIELD, NESTED_TABLE, XML_FIELD, VARRAY_FIELD, PHYSICAL_PROPERTY, OBJECT_PROPERTY };
  virtual void resolve(Ptr<ResolvedEntity> parent) = 0;
  virtual CathegoryFieldProperty cathegoryFieldProperty() const = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~FieldProperty() {}
};

class PhysicalProperties : public FieldProperty {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
   Ptr<NumericValue> intrans;
   Ptr<NumericValue> pctfree;
   Ptr<NumericValue> pctused;
   Ptr<Id>           tablespace;

   inline void concat(Ptr<PhysicalProperties> o) {
     if (o->intrans)
       intrans = o->intrans;
     if (o->pctfree)
       pctfree = o->pctfree;
     if (o->pctused)
       pctused = o->pctused;
     if (o->tablespace)
       tablespace = o->tablespace;
   }

   void resolve(Ptr<ResolvedEntity>) {}
   void linterDefinition (Sm::Codestream &str);

   CathegoryFieldProperty cathegoryFieldProperty() const { return PHYSICAL_PROPERTY; }
   void collectSNode(SemanticTree *n) const;
};

/**
 * Свойства объектной таблицы - те же, что и у реляционной.
 * Однако вместо определения столбца определяются аттрибуты объекта.
 * Для аттрибута определяется имя столбца как элемента объекта
 */
class ObjectProperties   : public FieldProperty {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>                fieldRef;
  Ptr<SqlExpr>           defaultExpr;
  Ptr<List<Constraint> > constraints;

  ObjectProperties(Ptr<Constraint> constr)
    : constraints(new List<Constraint>(constr)) {}
  ObjectProperties(CLoc l, Ptr<List<Constraint> > _constraints)
    : GrammarBaseSmart(l), constraints(_constraints) {}
  ObjectProperties(CLoc l, Ptr<Id> _fieldRef, Ptr<List<Constraint> > _constraints)
    : GrammarBaseSmart(l), fieldRef(_fieldRef), constraints(_constraints) {}
  ObjectProperties(CLoc l, Ptr<Id> _fieldRef, Ptr<SqlExpr> _defaultExpr, Ptr<List<Constraint> > _constraints)
    : GrammarBaseSmart(l), fieldRef(_fieldRef), defaultExpr(_defaultExpr), constraints(_constraints) {}

  void resolve(Ptr<ResolvedEntity> parent);

  void collectSNode(SemanticTree *node) const;
  CathegoryFieldProperty cathegoryFieldProperty() const { return OBJECT_PROPERTY; }
  bool hasLinterEquivalent() const { return false; }
};
class ObjectField : public FieldProperty {
  Ptr<Id>                    name;
  Ptr<SubstitutableProperty> substitutableProperty;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(Ptr<ResolvedEntity> parent);

  ObjectField(CLoc l = cl::emptyFLocation(), Ptr<Id> _name = 0, Ptr<SubstitutableProperty> _substitutableProperty = 0)
    : GrammarBaseSmart(l), name(_name), substitutableProperty(_substitutableProperty) {}

  void collectSNode(SemanticTree *node) const;

  CathegoryFieldProperty cathegoryFieldProperty() const { return OBJECT_FIELD; }
  bool hasLinterEquivalent() const { return false; }
};
class NestedName : public GrammarBaseSmart {
public:
  /// Имя столбца или аттрибут верхнего уровня объектного типа таблицы, чей тип - вложенная таблица.
  Ptr<Id> name;
  /// Если вложенная таблица представляет собой многоуровневую коллекцию, то внутренняя вложенная таблица или VARRAY может не иметь имени.
  /// В этом случае задается COLUMN_VALUE вместо имени столбца (аттрибута объекта).
  bool isColumnValue;

  NestedName() : isColumnValue(false) {}
  DEF_LBCONSTRUCTOR1(NestedName, isColumnValue) {}
  DEF_LBCONSTRUCTOR1(NestedName, name) {}

  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
};
class NestedTable : public FieldProperty {
public:
protected:

  Ptr<NestedName>              name;
  Ptr<SubstitutableProperty>   substitutableProperty;
  Ptr<Id>                      storageTable; // Вероятно это определение, причем в отдельном пространстве имен для
                                           // storage-таблиц; TODO: создать пространство STORAGE-таблиц и класть туда такие таблицы.
  Ptr<List<FieldProperty> >    fieldsProperties;
  nested_table::LocatorOrValue returnAs;

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(Ptr<ResolvedEntity> parent);

  Ptr<field_property::ObjectProperties  > object;
  Ptr<field_property::PhysicalProperties> physical;

  CathegoryFieldProperty cathegoryFieldProperty() const { return NESTED_TABLE; }

  NestedTable(CLoc l,
              Ptr<NestedName>              _name,
              Ptr<SubstitutableProperty>   _substProperty,
              Ptr<Id>                      _storageTable,
              Ptr<List<FieldProperty> >    _fldsProp,
              nested_table::LocatorOrValue _returnAs)
   : GrammarBaseSmart     (l),
     name                 (_name),
     substitutableProperty(_substProperty),
     storageTable         (_storageTable),
     fieldsProperties     (_fldsProp),
     returnAs             (_returnAs) {}

  void collectSNode(SemanticTree *node) const;

  bool hasLinterEquivalent() const { return false; }
};
class XmlField    : public FieldProperty {
  Ptr<Id> name; // определение, но на самом деле ссылка на тег в схеме XML
  Ptr<Id> xmlSchema;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(Ptr<ResolvedEntity> parent);
  void aggregateFieldProperties(Ptr<ResolvedEntity> parent);

  XmlField(CLoc l, Ptr<Id> _name, Ptr<Id> _xmlSchema)
    : GrammarBaseSmart(l), name(_name), xmlSchema(_xmlSchema) {}

  void collectSNode(SemanticTree *n) const;
  CathegoryFieldProperty cathegoryFieldProperty() const { return XML_FIELD; }
  bool hasLinterEquivalent() const { return false; }
};
class VarrayField : public FieldProperty {
  Ptr<Id>                    fieldRef;
  Ptr<SubstitutableProperty> substitutableProperty;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(Ptr<ResolvedEntity> parent);

  VarrayField(CLoc l, Ptr<Id> _fieldRef, Ptr<SubstitutableProperty> _substitutableProperty)
    : GrammarBaseSmart(l), fieldRef(_fieldRef), substitutableProperty(_substitutableProperty) {}

  void collectSNode(SemanticTree *node) const;
  CathegoryFieldProperty cathegoryFieldProperty() const { return VARRAY_FIELD; }
  bool hasLinterEquivalent() const { return false; }
};

};

class TableProperties         : public GrammarBaseSmart, public Translator {
public:

  typedef List<field_property::FieldProperty> FieldsProperties;
  typedef List<EnableDisableConstraint>       EnableDisableConstraints;
protected:
  Ptr<FieldsProperties>         fieldsProperties;
  Ptr<EnableDisableConstraints> enableDisableConstraints;
  Ptr<Subquery>       asSubquery;

  table_properties::CachingState         cachingState;
  table_properties::RowDependenciesState rowDependeniesState;
  Sm::Table                             *owner_;

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:

  void aggregateFieldProperties(Ptr<ResolvedEntity> parent);
  void resolve(Ptr<ResolvedEntity> parent);
  TableProperties() : cachingState(table_properties::C_EMPTY), rowDependeniesState(table_properties::R_EMPTY) {}
  void linterDefinition(Sm::Codestream &str);

  DEF_LBCONSTRUCTOR5(TableProperties, fieldsProperties, enableDisableConstraints, asSubquery, cachingState, rowDependeniesState), owner_(0) {}
  void owner(Sm::Table *own) { owner_ = own; }

  void collectSNode(SemanticTree *n) const;
  virtual ~TableProperties() {}
};
class OidIndex               : public GrammarBaseSmart {
public:
  Ptr<Id> name;
  Ptr<field_property::PhysicalProperties> physicalProperties;

  OidIndex() {}
  DEF_LBCONSTRUCTOR2(OidIndex, name, physicalProperties) {}
  DEF_LBCONSTRUCTOR1(OidIndex, physicalProperties)       {}
  SemanticTree *toSTree() const;
};
}

namespace Type {
  class Member;
  class MemberVariable;
};
class Record;
class Table;
class Cursor;

class AlterTable;


struct DblinkUserAuthentication {
  Id *user;
  Id *password;
};
class DatabaseLinkBody : public Smart {
//  bool toCurrentUser;
  Ptr<Id> connectToUser;
  Ptr<Id> connectToPassword;

  Ptr<Id> authenticatedByUser;
  Ptr<Id> authenticatedByPassword;
protected:
  //    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DatabaseLinkBody(bool /*_toCurrentUser*/ = false,
                   DblinkUserAuthentication *connectBy = 0,
                   DblinkUserAuthentication *authBy = 0)
//    : toCurrentUser(_toCurrentUser)
  {
    if (connectBy) {
      connectToUser       = connectBy->user;
      connectToPassword   = connectBy->password;
      delete connectBy;
    }
    if (authBy) {
      authenticatedByUser     = authBy->user;
      authenticatedByPassword = authBy->password;
      delete authBy;
    }
  }

  //    virtual ~LinkBody() {}
};

namespace view        {
class ViewQRestriction       : public GrammarBaseSmart {
public:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  enum QRestriction { EMPTY, WITH_READ_ONLY, WITH_CHECK_OPTION, WITH_CHECK_OPTION_CEXPR };
  Ptr<Id>      constExprId;
  QRestriction qRestriction;

  ViewQRestriction() : qRestriction(EMPTY) {}
  DEF_LBCONSTRUCTOR1(ViewQRestriction, qRestriction) {}
  DEF_LBCONSTRUCTOR1(ViewQRestriction, constExprId), qRestriction(WITH_CHECK_OPTION_CEXPR) {}

  inline SemanticTree *toSTree() const { return 0; }
};
class ViewConstraint         : public GrammarBaseSmart {
public:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>                constraintGroupAlias;
  Ptr<List<Constraint> > constraints;

  void resolve(Ptr<ResolvedEntity> parent);

  DEF_LBCONSTRUCTOR2(ViewConstraint, constraintGroupAlias, constraints) {}
  DEF_LBCONSTRUCTOR1(ViewConstraint, constraints) {}
  ViewConstraint(Ptr<Constraint> c) : constraints(new List<Constraint>(c)) {}

  void replaceChildsIf(Sm::ExprTr tr) { if (constraints) replace(tr, *constraints); }

  inline SemanticTree *toSTree() const { return 0; }
};


class ViewProperties    : public virtual GrammarBaseSmart {
public:
  enum  CathegoryViewProperties { OBJECT, XML, CONSTRAINTS };

public:
  virtual Ptr<List<ViewConstraint> > getViewConstraints() const { return 0; }
  virtual CathegoryViewProperties cathegoryViewProperties() const = 0;
  virtual void resolve(Ptr<ResolvedEntity> parent) = 0;
  virtual Ptr<List<view::ViewConstraint> > viewProperties() const { return 0; }
  virtual Sm::Id2 *getName2() const { return 0; }
  virtual ~ViewProperties() {}
  virtual Sm::SemanticTree *toSTree() const = 0;
  virtual void replaceChildsIf(Sm::ExprTr) {}
};

class ObjectReference        : public ViewProperties {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id2>                   name; // это ссылка на объект
  Ptr<Id2      >             superview;
  Ptr<List<Id> >             objectIdentifiers;
  Ptr<List<ViewConstraint> > constraints;
  bool                       oidDefault = false;

  void resolve(Ptr<ResolvedEntity> parent);
  Id2 *getName2() const { return name.object(); }

  CathegoryViewProperties cathegoryViewProperties() const { return OBJECT; }
  ObjectReference(CLoc l, Ptr<Id2> _name, Ptr<List<ViewConstraint> > _constraints, bool _oidDefault)
    : GrammarBaseSmart(l), name(_name), constraints(_constraints), oidDefault(_oidDefault) {}
  ObjectReference(CLoc l, Ptr<Id2> _name, Ptr<Id2> _superview, Ptr<List<ViewConstraint> > _constraints)
    : GrammarBaseSmart(l), name(_name), superview(_superview), constraints(_constraints) {}
  ObjectReference(CLoc l, Ptr<Id2> _name, Ptr<List<Id> > _objectIdentifiers, Ptr<List<ViewConstraint> > _constraints)
    : GrammarBaseSmart(l), name(_name), objectIdentifiers(_objectIdentifiers), constraints(_constraints) {}

  inline SemanticTree *toSTree() const { return 0; }
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, name, superview, objectIdentifiers, constraints); }
};

class XmlSchemaId : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> xmlSchemaRef;
  Ptr<Id> xmlElementRef;

  ScopedEntities ddlCathegory() const { return XmlReference_; }
  Ptr<Sm::Datatype> getDatatype() const { return Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }

  XmlSchemaId(CLoc l, Id* _xmlSchemaRef, Id* _xmlElementRef = 0)
    : GrammarBase(l), xmlSchemaRef(_xmlSchemaRef), xmlElementRef(_xmlElementRef) {}

  SemanticTree *toSTreeBase() const;
  virtual ~XmlSchemaId() {}
};
class XmlReference           : public ViewProperties {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<XmlSchemaId>      reference;
  Ptr<List<SqlExpr> > objectIdentifiers;
  bool                oidDefault;

  void resolve(Ptr<ResolvedEntity>) {};

  CathegoryViewProperties cathegoryViewProperties() const { return XML; }

  XmlReference(CLoc l, Ptr<XmlSchemaId> _reference, bool _oidDefault)
    : GrammarBaseSmart(l), reference(_reference), oidDefault(_oidDefault) {}
  XmlReference(CLoc l, Ptr<XmlSchemaId> _reference, Ptr<List<SqlExpr> > _objectIdentifiers)
    : GrammarBaseSmart(l), reference(_reference), objectIdentifiers(_objectIdentifiers), oidDefault(false) {}

  SemanticTree *toSTree() const;
};
class ViewConstraints        : public ViewProperties {
public:
  Ptr<List<ViewConstraint> > viewConstraints;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  void resolve(Ptr<ResolvedEntity> parent) {
    for (List<ViewConstraint>::iterator it = viewConstraints->begin(); it != viewConstraints->end(); ++it)
      (*it)->resolve(parent);
  };

  Ptr<List<ViewConstraint> > getViewConstraints() const { return viewConstraints; }

  CathegoryViewProperties cathegoryViewProperties() const { return CONSTRAINTS; }
  ViewConstraints(CLoc l, Ptr<List<ViewConstraint> > _viewConstraints)
    : GrammarBaseSmart(l), viewConstraints(_viewConstraints) {}

  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, viewConstraints); }

  inline SemanticTree *toSTree() const { return 0; }
};
};


namespace alter_table {
class KeyFields         : public GrammarBaseSmart, public Translator {
protected:
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<List<Id> > uniqueFields;
  bool primaryKey;

  KeyFields() : primaryKey(false) {}

  void linterDefinition(Codestream &str);
  DEF_LBCONSTRUCTOR1(KeyFields, uniqueFields) {}
  KeyFields(CLoc l) : GrammarBaseSmart(l), primaryKey(true) {}

  virtual ~KeyFields() {}
};

class AddRefConstraint  : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  AddRefConstraint(CLoc l = cl::emptyFLocation()) : GrammarBaseSmart(l) {}
  AddRefConstraint* toSelfAddRefConstraint() const { return const_cast<AddRefConstraint*>(this); }

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return ADD_REF_CONSTRAINT; }
};

class AddConstraints    : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  ConstraintsList constraints;

  void linterDefinition(Codestream &str);
  AddConstraints(CLoc l, ConstraintsList constr);

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return ADD_CONSTRAINTS; }
  alter_table::AddConstraints* toSelfAddConstraints() const { return const_cast<AddConstraints*>(this); }
};
class ModifyConstraint  : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Constraint> constraint;

  ModifyConstraint(CLoc l, Ptr<Constraint> constr)
    : GrammarBaseSmart(l), constraint(constr) {}
  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return MODIFY_CONSTRAINT; }
};
class ModifyKey         : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<KeyFields>  key;
  Ptr<Constraint> constraint;

  ModifyKey(CLoc l, Ptr<KeyFields> _key, Ptr<Constraint> _constraint)
    : GrammarBaseSmart(l), key(_key), constraint(_constraint) {}

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return MODIFY_KEY; }
};
class DropKey           : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<KeyFields>  key;
  bool            isCascade;
  table::enable_disable_constraint::KeepDropState keepOrDrop;

  void linterDefinition(Codestream &str);
  bool isDropPrimaryKey() const { return key && key->primaryKey; }
  bool isDropUniqueKey() const { return key && key->uniqueFields; }

  DropKey(CLoc l = cl::emptyFLocation(),
          Ptr<KeyFields>  _key = 0,
          bool            _isCascade = false,
          table::enable_disable_constraint::KeepDropState _keepOrDrop  = table::enable_disable_constraint::K_EMPTY)
    : GrammarBaseSmart(l), key(_key), isCascade(_isCascade), keepOrDrop(_keepOrDrop) {}

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return DROP_KEY; }
  DropKey* toSelfDropKey() const { return const_cast<DropKey*>(this); }
};
class DropConstraint    : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>  constraintName;
  int      isCascade;

  DropConstraint(CLoc l = cl::emptyFLocation(), Ptr<Id> _constraintName = 0, int _isCascade = 0)
    : GrammarBaseSmart(l), constraintName(_constraintName), isCascade(_isCascade) {}

  bool isDropConstraint() const { return true; }

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return DROP_CONSTRAINT; }
  DropConstraint* toSelfDropConstraint() const { return const_cast<DropConstraint*>(this); }
};

class ModifyFields;

class AlterFieldsBase;
typedef List<AlterFieldsBase> ManipulateFieldsList;

class AlterFieldsBase : public virtual GrammarBaseSmart, public Translator {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  enum CathegoryAlterFieldsBase { DROP_FIELDS, ADD_FIELDS, MODIFY_FIELDS };

  virtual CathegoryAlterFieldsBase cathegoryAlterFieldsBase() const = 0;
  virtual ~AlterFieldsBase() {}
  virtual ModifyFields *toSelfModifyFields() const { return 0; }
};

class ManipulateFields : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<ManipulateFieldsList> manipulateFields;

  ManipulateFields(CLoc l, Ptr<List<AlterFieldsBase> > _manipulateFields)
    : GrammarBaseSmart(l), manipulateFields(_manipulateFields) {}

  List<alter_table::AlterFieldsBase> *manipFields() const { return manipulateFields.object(); }
  void linterDefinition(Codestream &str);

  ManipulateFields* toSelfManipulateFields() const { return const_cast<ManipulateFields*>(this); }
  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return MANIPULATE_FIELDS; }
};



class DropFields   : public AlterFieldsBase {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  bool           dropCascade;
  Ptr<List<Id> > fields;

  DropFields(CLoc l, bool _dropCascade, Ptr<List<Id> > _fields)
    : GrammarBaseSmart(l), dropCascade(_dropCascade), fields(_fields) {}

  CathegoryAlterFieldsBase cathegoryAlterFieldsBase() const { return DROP_FIELDS; }
  void linterDefinition(Codestream &str);
};


class ModifyFields : public AlterFieldsBase {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  typedef List<Sm::ParsingStageTableField> Fields;
  Ptr<Fields> fields;

  ModifyFields(CLoc l, Ptr<Fields> _fields) : GrammarBaseSmart(l), fields(_fields) {}

  CathegoryAlterFieldsBase cathegoryAlterFieldsBase() const { return MODIFY_FIELDS; }
  ModifyFields *toSelfModifyFields() const { return const_cast<ModifyFields*>(this); }
};


class AddFields    : public AlterFieldsBase {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<List<ParsingStageTableField> >               fields;
  Ptr<List<table::field_property::FieldProperty> > properties;

  AddFields(CLoc l, Ptr<List<ParsingStageTableField> > _fields, Ptr<List<table::field_property::FieldProperty> > _properties)
    : GrammarBaseSmart(l), fields(_fields), properties(_properties) {}
  void linterDefinition(Codestream &str);

  CathegoryAlterFieldsBase cathegoryAlterFieldsBase() const { return ADD_FIELDS; }
};

class RenameTable       : public AlterTableCommand, public ResolvedEntity {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> newName;

  RenameTable(CLoc l, Ptr<Id> _newName) : GrammarBaseSmart(l), newName(_newName) {}
  ScopedEntities ddlCathegory() const { return RenameTable_; }
  Ptr<Datatype> getDatatype() const {
    if (newName)
      if (ResolvedEntity *def = newName->definition())
        return def->getDatatype();
    return 0;
  }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const { return newName ? newName->isSubtype(supertype, plContext) : EXPLICIT; }

  ResolvedEntity* getNextDefinition() const {
    return newName && newName->definition() ? newName->definition()->getNextDefinition() : (ResolvedEntity*)0;
  }

  bool getFields(EntityFields &fields) const { return newName && newName->definition() && newName->definition()->getFields(fields); }
  void linterDefinition(Codestream &str);

  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return RENAME_TABLE; }
};
class RenameField       : public AlterTableCommand {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> oldName;
  Ptr<Id> newName;

  RenameField(CLoc l, Ptr<Id> _oldName, Ptr<Id> _newName)
    : GrammarBaseSmart(l), oldName(_oldName), newName(_newName) {}

  void linterDefinition(Codestream &str);
  CathegoryAlterTableCommand cathegoryAlterTableCommand() const { return RENAME_FIELD; }
};




};

};


#endif
// vim:foldmethod=marker:foldmarker={,}






