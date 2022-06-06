#ifndef SEMANTIC_TABLE_H
#define SEMANTIC_TABLE_H

#include "semantic_utility.h"
#include "semantic_base.h"

class DependenciesStruct;

namespace Sm {


template <>
class List<Constraint> : public list<Ptr<Constraint> >, public ResolvedEntityLoc
{
protected:
  virtual SmartVoidType* getThisPtr() const;
public:
  List();
  List(const List<Constraint> & o);
  List(Ptr<Constraint> x);
  List(Constraint *x);

  ScopedEntities ddlCathegory() const;
  Ptr<Sm::Datatype> getDatatype() const;
  Sm::Codestream &translate(Sm::Codestream &str);

  virtual ~List();
};


class Table                   : public ResolvedEntitySNodeLoc {
public:
  enum TableCathegory     { RELATION, OBJECT, XML };
  typedef List<table::FieldDefinition> RelationFields;

  typedef map<string, table::FieldDefinition*> RelationFieldsMap;
  typedef std::map<std::string, Ptr<constraint::ForeignKey> > ForeignKeyMap;
  typedef std::map<std::string, Ptr<constraint::Unique> > UniqueKeyMap;

  typedef List<Sm::ParsingStageTableField> ParsingStageFields;
  typedef std::map<std::string, List<Constraint> > ConstraintsMap;
  typedef table::field_property::PhysicalProperties PhysicalProperties;
  typedef table::TableProperties  TableProperties;
  typedef table::field_property::ObjectProperties ObjectProperties;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  void setNameDefinition();

  std::string               translatedName_;

  Ptr<Id2>                  name;
  Ptr<PhysicalProperties>   physicalProperties;
  Ptr<TableProperties>      tableProperties;
  Ptr<RelationFields>       relationFields;

  RelationFieldsMap         relationFieldsMap;

  Ptr<Id2>                  objectName;
  Ptr<ObjectProperties>     objectProperties;
  Ptr<table::OidIndex>      oidIndex;
  Ptr<Id>                   xmlSchemaUrl;
  Ptr<NumericValue>         maxrow_;
  Ptr<NumericValue>         tablesize_;

  table::OnCommitRowsAction onCommitAction = table::EMPTY;
  TableCathegory            tableCathegory = RELATION;
  bool                      isGlobalTemporary_       = false;
  mutable bool              indexedFieldsInitialized = false;

  mutable SemanticTree         *fieldSNode = 0;
  mutable Ptr<Datatype>         thisDatatype;
  mutable Sm::ResolvedEntitySet indexedFields;

  int                           actionNumber_ = 0;

  typedef std::vector<std::pair<Ptr<table::FieldDefinition>, Ptr<table::FieldDefinition> > > DiffFields;
  DiffFields diffFields;
public:
  Ptr<Table> othTable;

  Ptr<Sm::constraint::PrimaryKey>      primaryKey;
  List<PlExpr>                         checkConditions;

  ForeignKeyMap                        foreignKeys;
  UniqueKeyMap                         uniqueKeys;

  bool diff(Ptr<Table> tbl);

  void isGlobalTemporary(bool val) { isGlobalTemporary_ = val; }

  ScopedEntities ddlCathegory() const { return Table_; }
  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const;

  void maxrow(Ptr<NumericValue> m) { maxrow_ = m; }
  void tablesize(Ptr<Sm::NumericValue> v) { tablesize_ = v; }

  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;
  Ptr<Id>  getName () const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }
  bool isTable() const { return true; }
protected:
  void initRelationFields(Ptr<ParsingStageFields> &relFields);

public:

  Table();
  Table(int, Ptr<Id2> _name);

  Table(Ptr<Id2> _name, Ptr<ParsingStageFields> _relationFields = 0);

  Table(CLoc l, Ptr<Id2> n, Ptr<PhysicalProperties> physProps, Ptr<TableProperties> tblProps, table::OnCommitRowsAction commitAct,
        Ptr<ParsingStageFields> relFields);

  Table(CLoc l, Ptr<Id2> n, Ptr<PhysicalProperties> physProps, Ptr<TableProperties> tblProps, table::OnCommitRowsAction commitAct,
        Ptr<Id2> objName, Ptr<ObjectProperties> objProperties, Ptr<table::OidIndex> oidIdx, Ptr<Id> xmlUrl = 0);

//  DEF_LBCONSTRUCTOR6(Table, name, physicalProperties, tableProperties, onCommitAction, tableCathegory, isGlobalTemporary_) { setNameDefinition(); }

  size_t columnsSize() const { return relationFields ? relationFields->size() : 0; }
  void getDefaults(Ptr<Table> t);

  void diffOutput(Sm::Codestream &str);

  void linterDefinition(Sm::Codestream &str);
  void linterReference (Sm::Codestream &str);

  void pythonDefinition(Sm::Codestream &str);

  void linterDefinitionKeys(Sm::Codestream &str);

  string tableEntityCathegory() const;

  bool hasObjectFields() const;
  bool hasCleanNumberFields() const;
  bool hasNumberFieldsThatWillChanged() const;

  void printCleanNumberFields(Sm::Codestream &str);
  void printFieldForStatistics(Sm::Codestream &str, RelationFields::iterator it);

  void printBigNumberFields(Sm::Codestream &str);
  void calculateNumberPrecStatistic(std::map<string, int> &stat);

  bool usedInQueryAndContainsFields() { return true; }

  Sm::SemanticTree *toSTreeBase() const;
  virtual ~Table();

  void translatedName(const string &n);
  Table* toSelfTable() const { return const_cast<Table*>(this); }
};


class SpecialStoreKeys : public ResolvedEntity {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef std::vector<Sm::ResolvedEntity *> SortedEntities;
  ModelContext &ctx;
  DependenciesStruct *m = 0;
  SortedEntities &sortedEntities;
  Codestream &str;
  Codestream &strVars;

  SpecialStoreKeys(
      ModelContext &_ctx,
      DependenciesStruct *_m,
      SortedEntities &_sortedEntities,
      Codestream &_str,
      Codestream &_strVars);

  void linterDefinition(Codestream &str);

  Ptr<Sm::Datatype> getDatatype()  const { return 0; }

  ScopedEntities ddlCathegory() const { return SpecialKeysActor_; }
};

inline Codestream& operator<<(Codestream& s, Table &obj) { return obj.translate(s); }


class Package : public virtual Declaration {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef BaseList<BlockPlSql> Container;
  Ptr<Id2>  name;
protected:
public:
  Container heads;  // заголовок
  Container bodies; // объявления + код инициализатора сессии

  int actionNumber_ = 0;
  DependEntitiesMap *filter_;

  bool bodyEmpty() const;
  bool headEmtpy() const;

  Sm::BlockPlSql *childCodeBlock() const;

  Package(CLoc l, Ptr<Id2> name, Ptr<BlockPlSql> content, bool _isBody);

  void pullFromOther(Ptr<Package> oth);

  void collectInitializers(EntitiesSet &container);
  void traverseDeclarations(DeclActor &fun);
  void traverseModelStatements(StatementActor &fun);
  void replaceChildsIf(Sm::ExprTr tr);
  void replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond);

  void setPackageAttributesForBlocks();

  std::string getInitializerName() const;

  Ptr<Id>         getName     () const;
  ScopedEntities  ddlCathegory() const { return Package_; }
  bool            isDefinition() const { return bodies.size(); }
  Ptr<Datatype>   getDatatype () const;
  bool            hasLinterEquivalent() const { return false; }
  ResolvedEntity *ownerPackage() const { return (Package*)this; }

  bool getFieldRef(Ptr<Id> &field);
  void addSemanticStatements  (SemanticTree *node, const BaseList<BlockPlSql> &entities) const;
  void addSemanticExceptions  (SemanticTree *node, const BaseList<BlockPlSql> &entities) const;
  void addSemanticEndLabels   (SemanticTree *node, const BaseList<BlockPlSql> &entities) const;
  void addSemanticDeclarations(SemanticTree *node, const BaseList<BlockPlSql> &entities) const;

  SemanticTree *toSTreeBase() const;
  void linterDefinition (Codestream &str);
  void linterDeclaration(Codestream &str);

  void userInitializer(Sm::Codestream &str);
  bool initializerEmpty();
  void collectInitializedEntities(Sm::Definitions &defs);

  Sm::Package *toSelfPackage() const { return (Package*)this; }

  void traverseDeclarationsForce(DeclActor &tr);
};


class AlterTable              : public GrammarBaseSmart, public Translator {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Sm::SemanticTree *semanticNode;
public:
  typedef AlterTableCommand::ConstraintsList   ConstraintsList;
  typedef List<table::EnableDisableConstraint> EnablingSpec;

  Ptr<Id2>                 name; /* table name*/
  Ptr<AlterTableCommand>   command;
  Ptr<EnablingSpec>        enablingSpec;
  int actionNumber_ = 0;

  inline void actionNumber(size_t val) { actionNumber_ = val; }

  void linterDefinition(Sm::Codestream &str);


  AlterTable(CLoc l, Ptr<Id2> _name, Ptr<AlterTableCommand> cmd, Ptr<EnablingSpec> _enablingSpec);

  virtual ~AlterTable();
};

namespace alter_user {
class  UserProxyConnect      : public GrammarBaseSmart {
  mutable Sm::SemanticTree *semanticNode;
public:
  enum ConnectType { THROUGH_ENTERPRISE_USERS, THROUGH_USER };
  Ptr<Id>        uname;
  Ptr<List<Id> > roles;
  bool           isAuthReq;
  ConnectType    connectType;

  UserProxyConnect();
  UserProxyConnect(CLoc l, Ptr<Id> _uname, Ptr<List<Id> > _roles, bool _isAuthReq);
  SemanticTree *toSTree() const;
};
class UserRole              : public GrammarBaseSmart {
public:
  enum RoleType { ID, CONNECT, DBA };
  Ptr<Id>  name;
  RoleType roleType;

  void collectSNode(Sm::SemanticTree *n) const;
  UserRole(CLoc l, Ptr<Id> _name, RoleType t);
};
class  UserRoles             : public GrammarBaseSmart {
  mutable Sm::SemanticTree *semanticNode;
public:
  enum Quantors { EMPTY, ALL, ALL_EXCEPT, NONE, ROLES };
  Quantors             quantor;
  Ptr<List<UserRole> > roleList;

  UserRoles();
  UserRoles(CLoc l, Quantors _qantor, Ptr<List<UserRole> > _roleList = 0);
  Sm::SemanticTree *toSTree() const;
};
struct Connect               : public GrammarBaseSmart {
  connect::GrantOrRevoke grantOrRevoke;
  Ptr<UserProxyConnect>  proxy;

  Connect(CLoc l, connect::GrantOrRevoke _grntOrRevoke, Ptr<UserProxyConnect>  _proxy);
  SemanticTree *toSTree() const;
};
class UserSettings          : public GrammarBaseSmart {
public:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  enum SettingsCathegory { EMPTY, PASSWORD, DEFAULT_ROLE, CONNECT };

  Ptr<Id>           password;
  Ptr<UserRoles>    defaultRole;
  Ptr<Connect>      connect;
  SettingsCathegory settingsCathegory;

  UserSettings();
  UserSettings(CLoc l, Ptr<Id>        _pass    );
  UserSettings(CLoc l, Ptr<UserRoles> _dfltRole);
  UserSettings(CLoc l, Ptr<Connect>   _connect );

  SemanticTree *toSTree() const;
};
}

class AlterUser              : public GrammarBaseSmart {
protected:
  //  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable Sm::SemanticTree *semanticNode;
public:
  Ptr<List<Id> >                users;
  Ptr<alter_user::UserSettings> settings;
  int  actionNumber_ = 0;

  void actionNumber(size_t val) { actionNumber_ = val; }
  void resolve(ModelContext &);
  AlterUser(CLoc l, Ptr<List<Id> > _users, Ptr<alter_user::UserSettings> _settings);
  SemanticTree *toSTree() const;
};

class Synonym                : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id2> name;
  Ptr<Id2> target;
  bool isPublic = false;
  int actionNumber_ = 0;

  Ptr<Id>  getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }
  bool isSystem() const;
  ScopedEntities ddlCathegory() const { return Synonym_; }
  void resolve(ModelContext &model);
  Ptr<Id2> getTarget() const;
  bool isDefinition() const { return target; }
  Ptr<Datatype> getDatatype() const;
  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;

  ResolvedEntity* getNextDefinition() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  void linterDefinition(Sm::Codestream &str);

  Synonym(Ptr<Id2> _name, Ptr<Id2> _target, bool _isPublic = false, CLoc l = cl::emptyFLocation());

  SemanticTree *toSTreeBase() const;

  void setPublic() { isPublic = true; }
  virtual ~Synonym();

  void linterReference  (Sm::Codestream &)  {}

  Synonym *toSelfSynonym() const { return const_cast<Synonym*>(this); }
};

class QueryPseudoField;

class View                   : public ResolvedEntitySNodeLoc, public virtual TraversingInterface {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  bool  semanticResolved;
  mutable SemanticTree *aliasedFieldsNode;
  mutable SemanticTree *queryNode;
  mutable Ptr<Datatype> thisDatatype;
public:
  typedef  List<QueryPseudoField> AliasedFields;

  Ptr<Id2>                    name        ; // resolved in model
  Ptr<view::ViewProperties>   properties  ;
  Ptr<Subquery>     select      ;
  Ptr<view::ViewQRestriction> qRestriction;
  AliasedFields               aliasedFields;
  int actionNumber_ = 0;
  bool isMaterialized = false;

  Ptr<Id>  getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }
  ScopedEntities ddlCathegory() const { return View_; }
  void resolve(ModelContext &model);
  bool getFieldRef(Ptr<Sm::Id> &name);
  bool getFields(EntityFields &fields) const;
  bool isDefinition() const { return true; }
  bool isView() const { return true; }
  bool semanticResolve() const;
  Sm::View *toSelfView() const { return (View*)this; }

  bool usedInQueryAndContainsFields() { return true; }
  Ptr<Datatype> getDatatype() const;

  void replaceChildsIf(Sm::ExprTr tr);
  void traverseDeclarations(DeclActor &) {}
  void traverseModelStatements(StatementActor &) {}
  void translateHead(Sm::Codestream &str);

  View(CLoc l, Ptr<Id2> _name, Ptr<view::ViewProperties> _properties, Ptr<Subquery> q, Ptr<view::ViewQRestriction> restr);
  void linterReference (Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  void pythonDefinition(Sm::Codestream &str);
  void parseConstraintList();
  Sm::SemanticTree *toSTreeBase() const;
  void updateFieldsDefinitions();
  virtual ~View();

};

class SequencePseudocolumn   : public ResolvedEntity {
public:
  enum Cathegory { CURRVAL = 0, NEXTVAL = 1 };
  static string cathegoryToSTring(Cathegory t);

protected:
  Cathegory       cathegory;
  mutable Ptr<Id> name;
public:
  void * getThisPtr() const { return (void*)this; }

  SequencePseudocolumn(Cathegory c);
  Ptr<Id> getName() const;
  void linterReference(Codestream &str);
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool isPlContext) const;
  ScopedEntities ddlCathegory() const { return SequencePseudocolumn_; }
  Ptr<Sm::Datatype> getDatatype() const;
  SemanticTree *toSTreeBase() const { return 0; }
};

class SequenceBody           : public GrammarBaseSmart {
  friend ostream& operator<< (ostream& os, const SequenceBody& str);
public:
   Ptr<NumericValue> incrementBy;
   Ptr<NumericValue> startWith  ;
   Ptr<NumericValue> maxvalue   ;
   Ptr<NumericValue> minvalue   ;
   Ptr<NumericValue> cache      ;
   union Flags {     /* флаги того, что инициализировано */
     struct t {
       int cycle      :1;
       int order      :1;
       int nocache    :1;
       int nocycle    :1;
       int nomaxvalue :1;
       int nominvalue :1;
       int noorder    :1;
       int incrementBy:1;
       int startWith  :1;
       int maxvalue   :1;
       int minvalue   :1;
       int cache      :1;
     } flags;
     uint_least16_t i;
   } v;
   SequenceBody(CLoc l) : GrammarBaseSmart(l) { v.i = 0; }

   void concat(Ptr<SequenceBody> o);
   void toString(string &str) const;
   inline SemanticTree *toSTree() const { return 0; }
};


class Sequence               : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id2>          name;
  Ptr<SequenceBody> data;
  int actionNumber_ = 0;

  Ptr<SequencePseudocolumn> pseudocolumns[2];

  Ptr<Id> getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }

  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  void linterDefinition(Codestream &os);
  void linterReference (Sm::Codestream &str);

  bool getFieldRef(Ptr<Sm::Id> &field);

  ScopedEntities ddlCathegory() const { return Sequence_; }
  void resolve(ModelContext &) {}

  Sequence(CLoc l, Ptr<Id2> _name, Ptr<SequenceBody> _data);

  SemanticTree *toSTreeBase() const;
  Sequence* toSelfSequence() const { return const_cast<Sequence*>(this); }
};

namespace trigger {
class TriggerEvents    : public virtual GrammarBaseSmart, public Translator {
public:
  enum  CathegoryTriggerEvents { DML_EVENT, NON_DML_EVENT };
public:
  virtual CathegoryTriggerEvents cathegoryTriggerEvents() const = 0;
  virtual Sm::trigger::DmlEvents *toSelfDmlEvents() const { return 0; }
  virtual bool isNonDmlEvent() const { return false; }
  virtual trigger::non_dml_events::T nonDmlEventsCathegoies() const;
  virtual std::string schemaOfNonDmlEvent() const { return 0; }
  virtual Sm::SemanticTree *toSTree() const = 0;
  virtual ~TriggerEvents() {}

  virtual string toLogStringCahtegory() = 0;
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
};

class TriggerActionInterface : public virtual Smart, public virtual Translator {
public:
  enum  CathegoryTriggerActionInterface { FUNCALL, TRIGGER_CODE, COMPOUND_TRIGGER_BLOCK };

  TriggerActionInterface() {}
  virtual CathegoryTriggerActionInterface cathegoryTriggerActionInterface() const = 0;
  virtual void collectSNode(SemanticTree *n) const = 0;

  virtual Sm::BlockPlSql *childCodeBlock() const = 0;
  virtual ~TriggerActionInterface() {}
  virtual void traverseDeclarations(DeclActor &tr) = 0;
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
  virtual void replaceStatementsIf(StmtTr tr, StmtTrCond cond) = 0;
  virtual void linterDefinitionWithWhenCondition(Codestream &str, Ptr<PlExpr> &condition) = 0;

  virtual BlockPlSql *toSelfBlockPlSql() const { return 0; }
};

}

class Trigger                : public ResolvedEntitySNodeLoc, public virtual TraversingInterface {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:

  Ptr<Id2>                    name;
  Ptr<trigger::TriggerEvents> events;
  trigger::TriggerMode        mode; Ptr<List<Id2> >             followsAfterTrigger;
  EnableState::T              enableState    ;
  Ptr<PlExpr>                 whenCondition  ;
  Ptr<trigger::TriggerActionInterface> action;
  int actionNumber_ = 0;

  std::string modeToString() const;
  Ptr<Id>  getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }

  Sm::BlockPlSql *childCodeBlock() const;

  ScopedEntities ddlCathegory() const { return Trigger_; }
  bool isDefinition() const;
  bool isTrigger() const { return true; }
  void resolve(ModelContext &model);

  void collectInitializers(EntitiesSet &container);
  void traverseDeclarations(DeclActor &tr);
  void replaceChildsIf(Sm::ExprTr tr);
  void replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond);

  void linterDefinition (Sm::Codestream &str);
  void linterReference  (Sm::Codestream &str);

  void linterDef(Sm::Codestream &str, std::string eventsStr);

  Ptr<Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }

  Trigger(CLoc l,
          Ptr<Id2>                             _name,
          Ptr<trigger::TriggerEvents>          _events,
          trigger::TriggerMode                 _mode,
          Ptr<List<Id2> >                      _followsAfterTrigger,
          EnableState::T                       _enableState,
          Ptr<PlExpr>                          _whenCondition,
          Ptr<trigger::TriggerActionInterface> _action);

  SemanticTree *toSTreeBase() const;
  virtual ~Trigger();

  Sm::Trigger *toSelfTrigger() const { return (Trigger*)this; }
  std::string toLogStringCahtegory();
  void traverseModelStatements(StatementActor &) {}
};


class Index                  : public ResolvedEntitySNodeLoc, public CathegoryIndexEnum {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  static ResolvedEntitySet alreadyIndexedFields;
public:
  typedef List<SqlExpr> FieldList;
  Ptr<Id2> name ;
  Ptr<Id2> table;

protected:
  Ptr<Id > alias;
  /// one column or column expression list
  Ptr<FieldList>  fieldList;
  CathegoryIndex  cathegoryIndex_;
  int actionNumber_ = 0;

  mutable Ptr<Id> indexTranslatedName_;
public:

  Ptr<Id> indexTranslatedName() const;

  void sqlReference  (Sm::Codestream &s);

  string translatedName() const;

  ResolvedEntity *tableDef() const;
  Ptr<Id>  getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }
//  ScopedEntities ddlCathegory() const { return cathegoryIndex_ == UNIQUE ? IndexUnique_ : Index_; }
  ScopedEntities ddlCathegory() const { return Index_; }
  void resolve(ModelContext &model);
  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const;
  void linterDefinition(Codestream &os);
  string indexCathegory();

  void cathegoryIndex(CathegoryIndex val) { cathegoryIndex_ = val; }
  CathegoryIndex cathegoryIndex() { return cathegoryIndex_; }
  void setName(Ptr<Id2> name);
  bool getFieldRef(Ptr<Id> &field);

  Index(CLoc l, Ptr<Id2> tbl, Ptr<Id> _alias, Ptr<FieldList> fldList, CathegoryIndex cat);

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
  Index* toSelfIndex() const { return const_cast<Index*>(this); }

  bool needSemicolonAfterEntity() const { return false; }

  void setTranslatedNameIfNot() const;

protected:
  void printIndexField(Codestream &str, Ptr<SqlExpr> field, bool &isNotFirst);
};

class DatabaseLink           : public ResolvedEntitySNodeLoc {
public:
private:
  Ptr<Id2> name;
  Ptr<DatabaseLinkBody> body;
  Ptr<Id> connectionString;
  bool isPublic_;
  bool isShared_;
  int actionNumber_ = 0;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  DatabaseLink();
  DatabaseLink(Ptr<Id2> n, Ptr<DatabaseLinkBody> b, Ptr<Id> connString);
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool) const;
  Ptr<Id>  getName() const;
  Ptr<Id2> getName2() const;
  void actionNumber(size_t val) { actionNumber_ = val; }
  void resolve(ModelContext &);
  void setPublic() { isPublic_ = true; }
  void setShared() { isShared_ = true; }
  void setPublicShared() { isPublic_ = true; isShared_ = true; }
  ScopedEntities ddlCathegory() const { return DatabaseLink_; }
  bool isDefinition() const { return true; }
  Ptr<Datatype> getDatatype() const;
  SemanticTree *toSTreeBase() const;
  virtual ~DatabaseLink();
};

namespace table {
class EncryptSpec           : public GrammarBaseSmart {
public:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>  encryptAlgorithm;
  Ptr<Id>  password;
  bool     salt;
  EncryptSpec();
  EncryptSpec(CLoc l, Ptr<Id> encrypt, Ptr<Id> pass, bool _salt);

  SemanticTree *toSTree() const { return 0; }
};

class FieldDefinition        : public ResolvedEntitySNodeLoc  {
  void setNameDefinition();
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  bool    nameTranslated_ = false;
  Table  *owner_ = 0; // нужно для системных таблиц
public:
  typedef map<void*, table::field_property::FieldProperty*> ResolvedProperties;
  typedef List<Constraint> Constraints;

  Ptr<Id>            name;             // resolved in semantic tree bulding functionality
  Ptr<Datatype>      datatype;
  Ptr<SqlExpr>       defaultExpr;      // <- if Virtual Columns - then it is definition
//protected:
//  Ptr<Constraints>   fieldConstraints;
public:
  Ptr<EncryptSpec>   encrypt;
  ResolvedProperties resolvedProperties;
  bool               isVirtualField;
  bool               isSort;
  bool               isNull = true;

  Ptr<Datatype>      foreignDatatype_;

  void oracleDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  void linterReference (Sm::Codestream &s);

  void setFieldNumber(int) {}

  bool isExactlyEquallyByDatatype(ResolvedEntity* ent);
  bool isDefinition() const { return true; }

  void foreignDatatype(Ptr<Sm::Datatype> t);
  void addFieldProperty(Sm::table::field_property::FieldProperty *p);

  bool getFieldRef(Ptr<Id> &field);

  bool isVarcharDatatype() const;
  bool isCharDatatype() const;
  ScopedEntities ddlCathegory() const { return FieldOfTable_; }
  Ptr<Sm::Datatype> getDatatype() const;
  Ptr<Sm::Datatype> getUnforeignDatatype()  const;
  Ptr<Sm::Datatype> getConcreteDatatype() const;
  Ptr<Id> getName() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool isPlContext) const;
  bool isField() const { return true; }
  bool isSqlIdentificator() const { return true; }


  ResolvedEntity* getNextDefinition() const;

  bool getFields(EntityFields &fields) const;

  FieldDefinition(Ptr<Id>                _name,                        // 1
                  Ptr<Datatype>          _datatype,                    // 2
                  Ptr<SqlExpr>           _defaultExpr = 0,             // 3
                  CLoc                    l = cl::emptyFLocation(),    // 5
                  Ptr<EncryptSpec>       _encrypt = 0,                 // 6
                  bool                   _isSort = false,              // 7
                  bool                   _isVirtualField = false,      // 8
                  ResolvedProperties     _resolvedProperties = ResolvedProperties());

  void   owner(Table *_owner);
  ResolvedEntity *owner() const;
  Table *ownerTable() const { return owner_; }

  ResolvedEntity *getFieldDDLContainer() const { return owner(); }

  void collectSNode(SemanticTree *n) const;

  SemanticTree *toSTreeBase() const;

  void translateNameIfSpecial();

  FieldDefinition *toSelfFieldDefinition() const { return const_cast<FieldDefinition*>(this); }

  virtual ~FieldDefinition();
};

}

class ParsingStageTableField : public Smart {
public:
  Ptr<table::FieldDefinition>   field;
  Ptr<List<Constraint> > constraints;

  ParsingStageTableField(table::FieldDefinition *_field, List<Constraint> *_constraints)
    : field(_field), constraints(_constraints) {}
};


void collectConstraintsOnField(ModelContext *self, Table *table, table::FieldDefinition *field, Ptr<List<Constraint> > constraints);


}

#endif // SEMANTIC_TABLE_H
