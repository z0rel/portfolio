#ifndef SEMANTIC_PLSQL_H
#define SEMANTIC_PLSQL_H

#include <map>
#include <stack>
#include <initializer_list>
#include "system_datatype.h"
#include "semantic_sql.h"
#include "codegenerator.h"
#include "system_functions.h"
#include "semantic_function.h"
#include "semantic_blockplsql.h"


namespace Sm {

/* declarations     */   /*{<<*/

namespace Type {
  class RefCursor;
  class Record;
  class NestedTable;
  class Varray;
  class Object;
  class ObjectType;
}

class Cursor;

class CursorParameter : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>       name;
  bool          in;
  Ptr<Datatype> datatype;
  Ptr<PlExpr>   defaultValue;
  int           indexInParameters = -1;
  Cursor       *ownerCursor = 0;

  bool getFieldRef(Ptr<Id> &field);
  Ptr<Datatype> getDatatype() const { return datatype; }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  ScopedEntities ddlCathegory() const { return CursorParameter_; }
  bool isDefinition() const { return true; }
  bool getFields(EntityFields &fieldStructure) const { return datatype && datatype->getFields(fieldStructure); }
  bool isField() const { return true; }
  ResolvedEntity* getNextDefinition() const { return datatype.object(); }
  void linterReference(Codestream &str);
  ResolvedEntity *getFieldDDLContainer() const;

  CursorParameter(CLoc l, Id* _name, bool _in, Datatype *t, PlExpr* _defaultValue = 0)
    : GrammarBase(l), name(_name), in(_in), datatype(t), defaultValue(_defaultValue) {}

  ResolvedEntity *toSelfResolvedEntity() { return this; }

  void collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }
  SemanticTree *toSTreeBase() const;
  virtual ~CursorParameter() {}
};

class OpenCursor;

class Cursor : public Declaration, public VariableCursorFields {
public:
  typedef List<CursorParameter> Parameters;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Ptr<Datatype> thisDatatype;
  Ptr<Id>                 name;


  SemanticTree *getVariableSNode()    const { return getSemanticNode(); }
  Ptr<Datatype> getVariableDatatype() const;
  Ptr<Id> getVarName() const { return name.object(); }
  BlockPlSql *getOwnerBlk() { return ResolvedEntity::ownerPlBlock(); }
public:
  typedef std::map<BlockPlSql* /*topBlock*/, Ptr<Variable> /*copyCursorVariable*/, LE_ResolvedEntities> InstanceMap;
  InstanceMap instanceMap;


  std::vector<Ptr<Sm::FunCallArg> > actualCursorParameters;
  std::vector<Ptr<CursorParameter> > parameters;
  Ptr<Datatype>               rowtype;
  Ptr<Subquery>               select;
  OpenCursor                 *currentOpenCommand;


  CLoc getLLoc() const { return GrammarBase::getLLoc(); }

  UserContext* userContext()     const { return ResolvedEntity::userContext(); }
  ResolvedEntity *ownerPackage() const { return ResolvedEntity::ownerPackage(); }
  ResolvedEntity* owner() const { return ResolvedEntity::owner(); }


  void setOpenCursorCommand(Sm::OpenCursor *cmd) { currentOpenCommand = cmd; }
  void clrOpenCursorCommand() { currentOpenCommand = 0; }
  Sm::Subquery *getSelectQuery() { return select; }

  Cursor *toSelfCursor() const { return const_cast<Cursor*>(this); }


  void linterReference(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;
  Ptr<Id> getName() const { return name; }
  ScopedEntities ddlCathegory() const { return Cursor_; }
  bool isDefinition() const { return select; }
  Ptr<Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  Cursor(CLoc l, Ptr<Id> _name, Ptr<Parameters> _parameters, Ptr<Datatype> _rowtype, Ptr<Subquery> _select);

  void collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }
  SemanticTree *toSTreeBase() const;

  ResolvedEntity *toSelfResolvedEntity() { return this; }

  void replaceChildsIf(Sm::ExprTr tr);
  void traverseModelStatements(Sm::StatementActor &) {}
};


template <typename T>
ostream& linterDefinitionList(ostream &o, Ptr<List<T> > l, const char separator = ',') {
  if (l && l->size()) {
     typename List<T>::iterator it = l->begin();
     if (*it)
       (*it)->linterDefinition(o);
     for (++it; it != l->end(); ++it) {
       o << separator << ' ';
       if (*it)
         (*it)->linterDefinition(o);
     }
  }
  return o;
}

template <typename T>
ostream& oracleDefinitionListByIt(ostream &o, T it, T end, const char separator = ',') {
  if (it == end)
    return o;
  if (*it)
    (*it)->oracleDefinition(o);
  for (++it; it != end; ++it) {
    o << separator << ' ';
    if (*it)
      (*it)->oracleDefinition(o);
  }
  return o;
}

template <typename T>
ostream& oracleDefinitionList(ostream &o, Ptr<List<T> > l, const char separator = ',') {
  if (l && l->size())
    oracleDefinitionListByIt(o, l->begin(), l->end(), separator);
  return o;
}

// namespace Type   {
//   class MemberFunction;
// }


namespace pragma {
  class Pragma;
}

namespace Type   {
class Object;


class MemberVariable         : public MemberInterface, public Declaration {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>       name;
  Ptr<Datatype> datatype;
  Ptr<Id>       javaExtName;


  ResolvedEntity *owner() const { return ResolvedEntity::owner(); }
  UserContext* userContext() const { return ResolvedEntity::userContext(); }
  ResolvedEntity *ownerPackage() const { return ResolvedEntity::ownerPackage(); }

  CathegoryMember cathegoryMember() const { return VARIABLE; }
  ScopedEntities ddlCathegory() const { return MemberVariable_; }
  void resolve(Ptr<Id>) {}
  bool getFieldRef(Ptr<Id> &name);
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  Ptr<ResolvedEntity> getThisDefinition() const { return (MemberVariable*)this; }
  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const { return datatype; }
  Ptr<Id> getName() const { return name; }
  Ptr<Id> getMemberName() const { return getName(); }
  bool isField() const { return true; }
  ResolvedEntity *getFieldDDLContainer() const;
  ResolvedEntity* getNextDefinition() const { return datatype.object(); }
  bool isVariable() const { return true; }
  bool isMemberVariable() const { return true; }
  void linterDefinition(Codestream &str) { str << name << s::name << datatype; }
  void linterReference(Codestream &str) { if (!isTrNameEmpty()) translateName(str); else str << name; }
  MemberVariable *toSelfMemberVariable() const { return const_cast<MemberVariable*>(this); }

  void collectSNodeM(SemanticTree *n) const { n->addChild(toSTree()); }
  MemberVariable(CLoc l, Ptr<Id> _name, Ptr<Datatype> _datatype, Ptr<Id> _jext = 0);

  SemanticTree *toSTreeBase() const;
  virtual ~MemberVariable() {}
};

class CollectionType;

class RefCursor              : public Declaration {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Ptr<Datatype> thisDatatype;
public:
  Ptr<Id>       name;
  Ptr<Datatype> datatype; // optional

  bool getFields(std::vector<Ptr<Sm::Id> > &fields) const;
  ScopedEntities ddlCathegory() const { return RefCursor_; }
  bool isDefinition() const { return datatype; }
  ResolvedEntity* getNextDefinition() const { return 0; }
  Ptr<Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  Ptr<Id> getName() const { return name; }
  bool isRefCursor() const { return true; }
  void linterReference(Sm::Codestream &str)  { str << s::OMultiLineComment() << " REF CURSOR " << s::CMultiLineComment(); }

  RefCursor(CLoc l = cl::emptyFLocation(), Ptr<Id> _name = 0, Ptr<Datatype> t = 0);

  SemanticTree *toSTreeBase() const;
  RefCursor* toSelfRefCursor() const { return (RefCursor*)this; }
  bool isExactlyEquallyByDatatype(ResolvedEntity *oth);

  void traverseModelStatements(StatementActor &) {}
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, datatype); }
};
};
namespace pragma {
class AutonomousTransaction : public Pragma {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  AutonomousTransaction(CLoc l) : GrammarBase(l) {}
  SemanticTree *toSTreeBase() const { return 0; }

  void resolve(Ptr<Id>) {}
  Ptr<Id> getMemberName() const { return 0; }
  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  virtual ScopedEntities ddlCathegory() const { return EMPTY__; }
  CathegoryPragma cathegoryPragma() const { return AUTONOMOUS_TRANSACTION; }
};
class ExceptionInit         : public Pragma  {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>           exceptionName;
  Ptr<NumericValue> errorNumber;

  Ptr<Id> getMemberName() const { return exceptionName; }
  Ptr<Id> getName() const { return exceptionName; }
  Ptr<Sm::Datatype> getDatatype() const { return Ptr<Datatype>(); }
  ScopedEntities ddlCathegory() const { return Exception_; }
  bool isDefinition() const { return false; }
  void resolve(Ptr<Id> ) {}
  ExceptionInit(CLoc l, Ptr<Id> _exceptionName, Ptr<NumericValue> errnum);

  SemanticTree *toSTreeBase() const;
  CathegoryPragma cathegoryPragma() const { return EXCEPTION_INIT; }
};
class Inline                : public Pragma {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> functionRef;
  bool    isInline;

  Ptr<Id> getMemberName() const { return functionRef; }
  void resolve(Ptr<Id>) {}
  Inline(CLoc l, Ptr<Id> funref, bool _isInline) : GrammarBase(l), functionRef(funref), isInline(_isInline) {}
  SemanticTree *toSTreeBase() const;
  CathegoryPragma cathegoryPragma() const { return INLINE; }

  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  virtual ScopedEntities ddlCathegory() const { return EMPTY__; }
};
class SeriallyReusable      : public Pragma {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  void resolve(Ptr<Id>) {}
  SeriallyReusable(CLoc l) : GrammarBase(l) {}
  SemanticTree *toSTreeBase() const { return 0; }
  CathegoryPragma cathegoryPragma() const { return SERIALLY_REUSABLE; }
  Ptr<Id> getMemberName() const { return 0; }
  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  virtual ScopedEntities ddlCathegory() const { return EMPTY__; }
};
class RestrictReferences    : public Pragma {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:

  Ptr<Id>       functionRef;
  PragmaRestrictFlags restrictFlags;
  bool          isDefault;

  Ptr<Id> getMemberName() const { return functionRef; }
  void resolve(Ptr<Id>) {}

  RestrictReferences(CLoc l, PragmaRestrictFlags fl, Ptr<Id> funref = 0);

  CathegoryPragma cathegoryPragma() const { return RESTRICT_REFERENCES; }
  SemanticTree *toSTreeBase() const;

  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  virtual ScopedEntities ddlCathegory() const { return EMPTY__; }
};
};
                          /*}>>*/
/* } */


/* create trigger         {<<*/
namespace trigger {




class DmlEvents;

class TriggerAbstractRowReference : public ResolvedEntitySNode {
public:
  enum Pseudorecord { NEW = 0, OLD = 1, PARENT = 2 };

  static string pseudorecordToString(Pseudorecord t);

protected:
  DmlEvents   *reference = 0;
  Ptr<Id>      name;
  Pseudorecord cathegory;
  bool createdInResolving_;

  virtual Ptr<ResolvedEntity> entity() const = 0;

public:
  TriggerAbstractRowReference(DmlEvents *_reference, Ptr<Id> _name, Pseudorecord _cathegory, bool createdInResolving);

  Pseudorecord rowCathegory() const { return cathegory; }

  Ptr<Id> getName() const { return name; }
  Ptr<Sm::Datatype> getDatatype() const { Ptr<ResolvedEntity> p = entity(); return p ? p->getDatatype() : Ptr<Sm::Datatype>(); }
  bool getFields(EntityFields &fields)  const { Ptr<ResolvedEntity> p = entity(); return p && p->getFields(fields); }
  bool getFieldRef(Ptr<Sm::Id> &field)  { Ptr<ResolvedEntity> p = entity(); return p && p->getFieldRef(field); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  ResolvedEntity* getNextDefinition() const { return entity().object(); }

  bool isTriggerRowReferenceParent() const { return rowCathegory() == PARENT; }
  bool isTriggerRowReference()       const { return true; }
  TriggerAbstractRowReference *toSelfAbstractRowReference() const { return const_cast<TriggerAbstractRowReference*>(this); }
  virtual bool linterMapReference(Sm::Codestream &, const IdEntitySmart &) = 0;

  SemanticTree *toSTreeBase() const;
};

// :new[.<foo>]  :old[.<foo>]
class TriggerRowReference : public TriggerAbstractRowReference {
protected:
  void* getThisPtr() const { return (void*)this; }
  Ptr<ResolvedEntity> entity() const;
public:
  TriggerRowReference(DmlEvents *_reference, Ptr<Id> _name, Pseudorecord _cathegory, bool createdInResolving);
  ScopedEntities ddlCathegory() const { return TriggerRowReference_; }
  TriggerRowReference* toSelfTriggerRowReference() const { return (TriggerRowReference*)this; }

  void linterReference (Sm::Codestream &str) { str << pseudorecordToString(cathegory);  }
  void linterDefinition(Sm::Codestream &str);
  bool linterMapReference(Sm::Codestream &str, const IdEntitySmart &ref);
};

class TriggerNestedRowReference : public TriggerAbstractRowReference {
protected:
  void* getThisPtr() const { return (void*)this; }
  Ptr<ResolvedEntity> entity() const;
public:
  TriggerNestedRowReference(DmlEvents *_reference, Ptr<Id> _name, Pseudorecord _cathegory, bool createdInResolving);
  ScopedEntities ddlCathegory() const { return TriggerNestedRowReference_; }
  bool linterMapReference(Sm::Codestream &, const IdEntitySmart &) { return false; }
};


struct DmlEvent       : public GrammarBaseSmart, public Translator {
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  union CathegoryEvent {
    enum Event { DELETE = 1, INSERT = 1 << 1, UPDATE = 1 << 2 };
    Event f;
    unsigned int i;
    inline bool isDelete() const { return isEntry(i, (unsigned int)(DELETE)); }
    inline bool isInsert() const { return isEntry(i, (unsigned int)(INSERT)); }
    inline bool isUpdate() const { return isEntry(i, (unsigned int)(UPDATE)); }
    CathegoryEvent(Event _f) : f(_f) {}
    void linterDefinition(Sm::Codestream &str);
  };

  Ptr<List<Id> > updatingFields; // resolved by table ref
  CathegoryEvent cathegoryEvent;

  void linterDefinition(Sm::Codestream &str);

  std::string toLogStringCahtegory();

  void concat(Ptr<DmlEvent> other);

  DmlEvent(CLoc l, Ptr<List<Id> > _updatingFields) : GrammarBaseSmart(l), updatingFields(_updatingFields), cathegoryEvent(CathegoryEvent::UPDATE) {}
  DmlEvent(CLoc l, CathegoryEvent::Event ev) : GrammarBaseSmart(l), cathegoryEvent(ev) {}

  void collectSNode(SemanticTree *node) const;

  virtual ~DmlEvent() {}
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, updatingFields); }
};
struct DmlReferencing : public SingleSmart {
  Ptr<Id>      name;
  TriggerAbstractRowReference::Pseudorecord cathegory;
  DmlReferencing(Ptr<Sm::Id> _name, TriggerAbstractRowReference::Pseudorecord t) : name(_name), cathegory(t) {}
};

class DmlEvents           : public TriggerEvents {
protected:
  mutable Sm::SemanticTree *semanticNode = 0;
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  typedef TriggerAbstractRowReference::Pseudorecord Pseudorecord;

  Ptr<DmlEvent>    dmlEvents;
  Ptr<Id2>         tableRef;
  Ptr<Id>          fieldNestedTable;

  /* Если триггер создан на вложенную таблицу в представлении, то OLD и NEW ссылаются на текущую строку вложенной таблицы,
       и PARENT ссылается на текущую строку таблицы-предка.
     Если триггер создан на обычную таблицу или представление, то OLD и NEW ссылаются на текущую строку в таблице или представлении,
       и PARENT - не определен.

     OLD, NEW, и PARENT также называют псевдозаписями (pseudorecords), т.к. они имеют структуру записи, но являются допустимыми
     в более широком контексте, чем записи. Структура псевдозаписей - это table_name%ROWTYPE, где table_name - это имя таблицы,
     на которой создан триггер (для OLD и NEW) или имя таблицы-предка (для PARENT).
  */

  Ptr<TriggerAbstractRowReference> pseudorecords[3];
  bool isForEachRow;

  DmlEvents(CLoc l, Ptr<DmlEvent> _dmlEvents, Ptr<Id2> _tableRef, Ptr<Id> _nestedTableField, Ptr<List<DmlReferencing> > _references, bool _isForEachRow);

  void linterDefinition(Sm::Codestream &str);

  DmlEvents *toSelfDmlEvents() const { return (DmlEvents*)this; }
  CathegoryTriggerEvents cathegoryTriggerEvents() const { return DML_EVENT; }
  SemanticTree *toSTree() const;

  std::string toLogStringCahtegory();
  void  replaceChildsIf(Sm::ExprTr tr) { replace(tr, dmlEvents, tableRef, fieldNestedTable); }
};
struct NonDmlEvents        : public TriggerEvents {
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  enum SchemaOrDatabase { SCHEMA, DATABASE };

  NonDmlEvent events;

  Ptr<Id> schema;
  SchemaOrDatabase schemaOrDatabase;
  bool isNonDmlEvent() const { return true; }
  non_dml_events::T nonDmlEventsCathegoies() const;

  std::string schemaOfNonDmlEvent() const;

  CathegoryTriggerEvents cathegoryTriggerEvents() const { return NON_DML_EVENT; }

  NonDmlEvents(CLoc l, const NonDmlEvent &e, const Ptr<Id> & _schema, SchemaOrDatabase _schemaOrDatabase);

  SemanticTree *toSTree() const;

  std::string toLogStringCahtegory();
  void replaceChildsIf(Sm::ExprTr tr);
};


class Funcall     : public TriggerActionInterface, public virtual GrammarBase {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> call;
  Ptr<SqlExpr> intoHostVar;

  Funcall(CLoc l, Ptr<SqlExpr> c, Ptr<SqlExpr> into);
  void collectSNode(SemanticTree *n) const;
  CathegoryTriggerActionInterface cathegoryTriggerActionInterface() const { return FUNCALL; }

  Sm::BlockPlSql *childCodeBlock() const { return call->childCodeBlock(); }
  void traverseDeclarations(DeclActor &) {}
  void replaceChildsIf(Sm::ExprTr tr)  { replace(tr, call, intoHostVar); }
  void replaceStatementsIf(StmtTr /*tr*/, StmtTrCond /*cond*/) {}

  void linterDefinitionWithWhenCondition(Codestream &/*str*/, Ptr<PlExpr> &/*condition*/) { throw 999; }
};

class TriggerPredicateVariable : public Sm::Variable {
public:
  enum TypePredicate { DELETING, INSERTING, UPDATING };
protected:
  TypePredicate predicateCathegory;
  static inline string toSTring(TypePredicate t)  {
    switch (t) {
      case DELETING:  return "DELETING";
      case INSERTING: return "INSERTING";
      case UPDATING:  return "UPDATING";
    }
    return "";
  }
public:
  TriggerPredicateVariable(CLoc l, TypePredicate t);

  virtual ScopedEntities ddlCathegory() const { return TriggerPredicateVariable_; }
};

class TriggerCode : public BlockPlSql, public TriggerActionInterface {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef std::vector<Ptr<TriggerPredicateVariable> > BooleanStateVariables;
  BooleanStateVariables booleanStateVariables;

  Sm::BlockPlSql *childCodeBlock() const { return (BlockPlSql*)this; }

  TriggerCode(CLoc l, Ptr<BlockPlSql> block);
  CathegoryTriggerActionInterface cathegoryTriggerActionInterface() const { return TRIGGER_CODE; }

  void collectSNode(SemanticTree *n) const { BlockPlSql::collectSNode(n); }
  SemanticTree *toSTreeBase() const;

  void linterDefinition(Codestream &str) { linDef(str); }
  void replaceChildsIf(Sm::ExprTr tr) { BlockPlSql::replaceChildsIf(tr); }
  void replaceStatementsIf(StmtTr tr, StmtTrCond cond) { replaceSubstatementsIf(tr, cond); }
  void traverseDeclarations(DeclActor &tr);
  void linterDefinitionWithWhenCondition(Codestream &str, Ptr<PlExpr> &condition) { linDef(str, condition.object()); }

  virtual BlockPlSql *toSelfBlockPlSql() const { return BlockPlSql::toSelfBlockPlSql(); }
};



}


namespace trigger {
}

bool checkToRowidPseudocolumn(Ptr<Id> &field);


}

#endif // SEMANTIC_PLSQL_H
