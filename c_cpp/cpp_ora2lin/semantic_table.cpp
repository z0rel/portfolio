#include "semantic_table.h"
#include "model_context.h"
#include "codegenerator.h"
#include "semantic_id.h"
#include "semantic_datatype.h"
#include "semantic_sql.h"

using namespace Sm;

extern SyntaxerContext syntaxerContext;

alter_table::AddConstraints::AddConstraints(CLoc l, ConstraintsList constr)
  : GrammarBaseSmart(l), constraints(constr)
{
  sAssert(!constraints);
}

Sm::constraint::PrimaryKey::PrimaryKey(CLoc l, Ptr<List<Id> > _fieldListKey)
  : GrammarBaseSmart(l), fieldListKey(_fieldListKey)
{
  sAssert(!fieldListKey);
}

Table::Table() {}

Table::Table(int, Ptr<Id2> _name)
  : name(_name)
{
  setNameDefinition();
}

Table::Table(Ptr<Id2> _name, Ptr<Sm::Table::ParsingStageFields> _relationFields)
  : name(_name)
{
  setNameDefinition();
  initRelationFields(_relationFields);
}

Table::Table(CLoc l,
             Ptr<Id2> n,
             Ptr<Table::PhysicalProperties> physProps,
             Ptr<Table::TableProperties> tblProps,
             table::OnCommitRowsAction commitAct,
             Ptr<Sm::Table::ParsingStageFields> relFields)
  : GrammarBase(l),
    name(n),
    physicalProperties(physProps),
    tableProperties(tblProps),
    onCommitAction(commitAct)

{
  setNameDefinition();
  initRelationFields(relFields);
}

Table::Table(CLoc l,
             Ptr<Id2> n,
             Ptr<Table::PhysicalProperties> physProps,
             Ptr<Table::TableProperties> tblProps,
             table::OnCommitRowsAction commitAct,
             Ptr<Id2> objName,
             Ptr<Table::ObjectProperties> objProperties,
             Ptr<table::OidIndex> oidIdx, Ptr<Id> xmlUrl)
  : GrammarBase(l),
    name(n),
    physicalProperties(physProps),
    tableProperties(tblProps),
    objectName(objName),
    objectProperties(objProperties),
    oidIndex(oidIdx),
    xmlSchemaUrl(xmlUrl),
    onCommitAction(commitAct),
    tableCathegory(objName ? OBJECT: XML)
{
  setNameDefinition();
}

Table::~Table() {}

namespace Sm{
namespace constraint {

void ConstraintState::collectSNode(SemanticTree *n) const { CTREE(index); }

bool ForeignReference::eq(ForeignReference *o) const {
  return o &&
      ((!foreignTable && !o->foreignTable) || (foreignTable && foreignTable->definition()->eqByVEntities(o->foreignTable->definition()))) &&
      ((!foreignFields && !o->foreignFields) || (foreignFields && foreignFields->eq(o->foreignFields))) &&
      onDeleteAction == o->onDeleteAction;
}

ResolvedEntity *ForeignReference::entityDef() const { return foreignTable->definition(); }

}
}


void ModelContext::alterTable(Ptr<Sm::AlterTable> alterCmd) {
  ModelContext *ctx = syntaxerContext.model;
  typedef ModelContext::AlterCommands::value_type value_type;
  sAssert(!alterCmd);
  sAssert(!alterCmd->command);
  ctx->alterCommands.push_back(value_type(ctx->udata, alterCmd));
}




struct PushUdata {
  ModelContext *model;
  UserContext  *ctx;
  PushUdata(ModelContext *m) : model(m), ctx(m->udata) {}
  ~PushUdata() { model->udata = ctx; }
};




void Sm::collectConstraintsOnField(ModelContext *self, Table *table, table::FieldDefinition *field, Ptr<List<Constraint> > constraints)
{
  static const auto resolveTableFields = [](Table* table, List<Id> *fields) {
    sAssert(!fields);
    for (Ptr<Id> &f : *fields) {
      table->getFieldRef(f);
      sAssert(!f->unresolvedDefinition());
    }
  };

  sAssert(!table);
  if (!constraints)
    return;
  for (Ptr<Constraint> &c : *constraints) {
      sAssert(!c);
      if (c->state && c->state->isDisable())
        continue;

      sAssert(c->state && c->state->index);

      Ptr<constraint::Attribute> attr = c->attribute();
      auto cat = attr->cathegory();
      switch (cat) {
        case constraint::Attribute::NOT_NULL:
          sAssert(!field);
          field->isNull = false;
          break;
        case constraint::Attribute::NULL_tok:
          sAssert(!field);
          field->isNull = true;
          break;
        case constraint::Attribute::PRIMARY_KEY: {
          table->primaryKey = attr->toSelfPrimaryKey();
          for (Ptr<Id> &field : *(table->primaryKey->fieldListKey)) {
            table->getFieldRef(field);
            sAssert(!field->unresolvedDefinition());
          }
          break;
        }
        case constraint::Attribute::FOREIGN_KEY: {
          constraint::ForeignKey *key = attr->toSelfForeignKey();
          resolveTableFields(table, key->fieldListKey);

          constraint::ForeignReference *ref = nAssert(key->referencedKey.object());
          Table *foreignTable = nAssert(self)->getTableRef(ref->foreignTable);
          resolveTableFields(foreignTable, key->fieldListKey);

          pair<Table::ForeignKeyMap::iterator, bool> it =
              table->foreignKeys.insert(Table::ForeignKeyMap::value_type(join(" ", key->fieldListKey), key));

          if (!it.second) {
            constraint::ForeignKey *fk = it.first->second;
            constraint::ForeignReference *existedRef = fk->referencedKey;
            sAssert(existedRef->foreignTable->definition() != foreignTable ||
                join(" ", existedRef->foreignFields) != join(" ", ref->foreignFields));
          }
          break;
        }
        case constraint::Attribute::UNIQUE: {
          constraint::Unique *uniq = attr->toSelfUniqueCondition();
          resolveTableFields(table, uniq->fieldListKey);
          table->uniqueKeys.insert(Table::UniqueKeyMap::value_type(join(" ", uniq->fieldListKey), uniq));
          // проверять факт неуникальности уникального кортежа - не нужно, т.к. уникальный кортеж ни в какой другой не отображается
          break;
        }
        case constraint::Attribute::CHECK: {
          table->checkConditions.push_back(attr->toSelfCheckCondition()->condition);
          break;
        }
        default:
          throw 999;
      }
  }
}



Table *ModelContext::getTableRef(Ptr<Id2> &table)
{
  UserContext *ctx = this->getUContextWithAdd(table); // [создать] определить id[1] = user
  UserContext::Tables::iterator it = ctx->tables.find(table->entity()->toNormalizedString());
  sAssert(it == ctx->tables.end());
  Table* tableDef = it->second.object();
  table->entity()->definition(tableDef);
  return tableDef;
}

void ModelContext::parseAlterTableCommands() {
  PushUdata saveUdata(this);
  (void)saveUdata;
  using namespace alter_table;

  for (ModelContext::AlterCommands::value_type &ctxCmd : alterCommands) {
    udata = ctxCmd.first;
    Ptr<AlterTable> alterCommand = ctxCmd.second;
    Ptr<Id2> table = alterCommand->name;

    Table* tableDef = getTableRef(table);

    sAssert(alterCommand->enablingSpec);

    AlterTableCommand *cmd = alterCommand->command;
    AlterTableCommand::CathegoryAlterTableCommand cat = cmd->cathegoryAlterTableCommand();
    switch (cat) {
      case AlterTableCommand::DROP_KEY:
        break;
      case AlterTableCommand::ADD_CONSTRAINTS:
        collectConstraintsOnField(this, tableDef, 0, nAssert(cmd->toSelfAddConstraints())->constraints);
        break;
      case AlterTableCommand::RENAME_TABLE:
      case AlterTableCommand::RENAME_FIELD:
      case AlterTableCommand::MANIPULATE_FIELDS: {
        ManipulateFields *manip = cmd->toSelfManipulateFields();
        for (Ptr<AlterFieldsBase> &alterField : *(manip->manipulateFields)) {
          if (ModifyFields *mFlds = alterField->toSelfModifyFields()) {
            for (Ptr<ParsingStageTableField> &f : *(nAssert(mFlds->fields.object()))) {
              sAssert(!f->field || !f->constraints);
              Ptr<Id> fieldName = nAssert(f->field->getName());
              tableDef->getFieldRef(fieldName);
              ResolvedEntity *fieldEntity = nAssert(fieldName->unresolvedDefinition());
              collectConstraintsOnField(this, tableDef, nAssert(fieldEntity->toSelfFieldDefinition()), f->constraints);
            }
          }
          else
            throw 999;
        }
        break;
      }
      case AlterTableCommand::DROP_CONSTRAINT:
        break;
      case AlterTableCommand::ADD_REF_CONSTRAINT:
      case AlterTableCommand::MODIFY_CONSTRAINT:
      case AlterTableCommand::MODIFY_KEY:
        throw 999;
    }
  }
  alterCommands.clear();
}



AlterTable::AlterTable(CLoc l, Ptr<Id2> _name, Ptr<AlterTableCommand> cmd, Ptr<AlterTable::EnablingSpec> _enablingSpec)
  : GrammarBaseSmart(l), semanticNode(0), name(_name), command(cmd), enablingSpec(_enablingSpec) {}
AlterTable::~AlterTable() {}


void Table::initRelationFields(Ptr<ParsingStageFields> &relFields) {
  sAssert(!relFields);
  relationFields = new RelationFields();
  for (Ptr<ParsingStageTableField> &parsedFieldRepr : *relFields) {
    Ptr<table::FieldDefinition> field = parsedFieldRepr->field;
    if (field) {
      field->owner(this);
      relationFields->push_back(field);
      relationFieldsMap[field->getName()->toNormalizedString()] = field;
    }

    if (parsedFieldRepr->constraints && !parsedFieldRepr->constraints->empty()) {
      UserContext *cntx = nAssert(syntaxerContext.model->parsingStageUserContext(name));
      Table *tbl = this;

      UserContext::Tables::iterator it = cntx->tables.find(name->entity()->toNormalizedString());
      if (it != cntx->tables.end())
        tbl = it->second.object();
      syntaxerContext.model->delayedConstraints.push_back(DelayedConstraints(field, tbl, parsedFieldRepr->constraints));
    }
  }
}

void Sm::Trigger::linterDefinition(Sm::Codestream &str) {
  if (beginedFrom(166599))
    cout << "";
  if (events->isNonDmlEvent()) {
    trigger::non_dml_events::T cat = events->nonDmlEventsCathegoies();
    bool afterLogon = isEntry(cat, trigger::non_dml_events::AFTER_LOGON);
    bool beforeLogoff = isEntry(cat, trigger::non_dml_events::BEFORE_LOGOFF);
    if (afterLogon || beforeLogoff)
      str << s::connect(syntaxerContext.model->getSystemUser().object());
    if (afterLogon)
      linterDef(str, std::string("AFTER LOGON ON ") + events->schemaOfNonDmlEvent());
    if (beforeLogoff)
      linterDef(str, std::string("BEFORE LOGOFF ON ") + events->schemaOfNonDmlEvent());
    str << s::connect(userContext());
  }
  else
    linterDef(str, "");
}

void Sm::Trigger::linterDef(Sm::Codestream &str, std::string eventsStr) {
  if (beginedFrom(362420,9))
    cout << "";
  str.levelPush();
  str << s::ocreate(this) << "TRIGGER " << s::cref(this);
  str << s::name;
  if (eventsStr.size())
    str << s::subconstruct
        << eventsStr;
  else
    str << modeToString() << s::name << s::subconstruct
        << events;
  str << s::name << s::subconstruct
      << "EXECUTE ";
  str.procMode(CodestreamState::PROC);
  str.levelPush();
  str.activateActions();
  if (whenCondition)
    action->linterDefinitionWithWhenCondition(str, whenCondition);
  else
    str << action;
  str.joinPrefixes();
  str.joinSuffixes();
  str.joinPostactions();
  str.join();
  str.activatePrevious();
  str.levelPop();

  str.procMode(CodestreamState::SQL);
  str.joinPreactions();
  str << s::semicolon << s::endl;
  str << s::ccreate;
  str.levelPop();
  if (followsAfterTrigger)
    throw 999; // нужно поддерживать.
}


void SequenceBody::concat(Ptr<SequenceBody> o) {
  v.i |= o->v.i;
  if (o->incrementBy)
    incrementBy = o->incrementBy;
  if (o->startWith)
    startWith = o->startWith;
  if (o->maxvalue)
    maxvalue = o->maxvalue;
  if (o->minvalue)
    minvalue = o->minvalue;
  if (o->cache)
    cache = o->cache;
}


void Table::linterReference(Codestream &str) {
  if (translatedName_.empty())
    str << name;
  else
    str << translatedName_;
}

string Table::tableEntityCathegory() const {
  if (isGlobalTemporary_)
    return "GLOBAL TEMPORARY ";
  return "";
}

void Table::translatedName(const string &n) {
  translatedName_ = n;
}


string Package::getInitializerName() const {
  std::string text = getName()->toQString();
  bool isQ = false;
  char b = 0;
  if (text.size() && (text.back() == '"' || text.back() == '\'')) {
    b = text.back();
    text.pop_back();
    isQ = true;
  }
  text.append("_PKG_INIT");
  if (isQ)
    text.push_back(b);
  return text;
}


Codestream &Sm::List<Constraint>::translate(Codestream &str) { Sm::translateList<List<Constraint> >(str, *this); return str; }

Sm::List<Constraint>::List(Ptr<Constraint> x) : list<Ptr<Constraint> >(1, x) {}

Sm::List<Constraint>::List() {}

CathegoriesOfDefinitions::ScopedEntities Sm::List<Constraint>::ddlCathegory() const { return ListConstraint_; }

Ptr<Datatype> Sm::List<Constraint>::getDatatype() const { return Ptr<Datatype>(); }

Sm::List<Constraint>::List(Constraint *x) : list<Ptr<Constraint> >(1, Ptr<Constraint>(x)) {}

Sm::List<Constraint>::List(const List<Constraint> &o)
  : GrammarBase(o),
    Smart(),
    ResolvedEntity(o),
    list<Ptr<Constraint> >(o),
    ResolvedEntityLoc(o)

{}

Smart::SmartVoidType *Sm::List<Constraint>::getThisPtr() const { return (SmartVoidType*)this; }

Sm::List<Constraint>::~List() {}

Sm::constraint::ForeignKey::ForeignKey(CLoc l, Ptr<List<Id> > _fieldListKey, Ptr<ForeignReference> _referencedKey)
  : GrammarBaseSmart(l), fieldListKey(_fieldListKey), referencedKey(_referencedKey)
{
  if (!fieldListKey || !referencedKey || fieldListKey->empty() || !*(fieldListKey->begin()))
    throw 999;
}

Ptr<Datatype> table::FieldDefinition::getUnforeignDatatype() const { return datatype; }

ResolvedEntity *table::FieldDefinition::owner() const { return owner_; }

Ptr<Id> Index::indexTranslatedName() const {
  setTranslatedNameIfNot();
  return indexTranslatedName();
}

void Index::sqlReference(Codestream &s) { s << name; }

string Sm::Index::indexCathegory() {
  switch (cathegoryIndex_) {
    case UNIQUE:
      return "UNIQUE ";
    case BITMAP:
      return "-- WARNING: bitmap index has not supported: \n";
    default:
      return "";
  }
}
