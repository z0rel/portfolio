#include "sstream"
#include "semantic_plsql.h"
#include "semantic_tree.h"
#include "resolvers.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "dynamic_sql_op.h"

using namespace Sm;
extern SyntaxerContext syntaxerContext;

Sm::trigger::non_dml_events::T trigger::NonDmlEvents::nonDmlEventsCathegoies() const { return events.f; }

std::string Sm::trigger::NonDmlEvents::schemaOfNonDmlEvent() const {
  switch (schemaOrDatabase) {
    case SCHEMA:   return schema ? schema->toQString() : std::string("DATABASE"); break;
    case DATABASE: return "DATABASE"; break;
  }
  return "";
}

trigger::NonDmlEvents::NonDmlEvents(CLoc l, const NonDmlEvent &e, const Ptr<Id> &_schema, trigger::NonDmlEvents::SchemaOrDatabase _schemaOrDatabase)
  : GrammarBaseSmart(l), events(e), schema(_schema), schemaOrDatabase(_schemaOrDatabase) {}

string trigger::NonDmlEvents::toLogStringCahtegory() { return events.toString() + " " + schemaOfNonDmlEvent(); }

void trigger::NonDmlEvents::replaceChildsIf(ExprTr tr) { replace(tr, schema); }


Sm::trigger::DmlEvents::DmlEvents(CLoc l, Ptr<DmlEvent> dmlEv, Ptr<Id2> tableref, Ptr<Id> nested, Ptr<List<DmlReferencing> > references, bool forEachRow)
  : GrammarBaseSmart(l),
    dmlEvents       (dmlEv),
    tableRef        (tableref),
    fieldNestedTable(nested),
    isForEachRow    (forEachRow)
{
  Ptr<Id> records[3];
  if (references)
    for (List<trigger::DmlReferencing>::iterator it = references->begin(); it != references->end(); ++it)
      if (*it)
        records[(int)((*it)->cathegory)] = (*it)->name;

  for (int i = 0; i < 3; ++i) {
    Ptr<Id> name = records[i];
    TriggerAbstractRowReference::Pseudorecord cathegory = (TriggerAbstractRowReference::Pseudorecord)i;
    bool createdInResolving = false;
    if (!name) {
      name = new Id(TriggerAbstractRowReference::pseudorecordToString(cathegory));
      createdInResolving = true;
    }

    switch (cathegory) {
    case TriggerAbstractRowReference::NEW:
    case TriggerAbstractRowReference::OLD:
      if (fieldNestedTable)
        pseudorecords[i] = new TriggerNestedRowReference((DmlEvents*)this, name, cathegory, createdInResolving);
      else
        pseudorecords[i] = new TriggerRowReference((DmlEvents*)this, name, cathegory, createdInResolving);
      break;
    case TriggerAbstractRowReference::PARENT:
      pseudorecords[i] = new TriggerRowReference((DmlEvents*)this, name, cathegory, createdInResolving);
      break;
    default:
      break;
    }
  }
}


void Sm::trigger::DmlEvent::collectSNode(SemanticTree *node) const {
  if (updatingFields)
    for (List<Id>::const_iterator it = updatingFields->begin(); it != updatingFields->end(); ++it)
      node->addChild(it->object()->toSNodeRef(SCathegory::Field));
}

void Sm::trigger::DmlEvent::concat(Ptr<DmlEvent> other) {
  if (!other)
    return;
  cathegoryEvent.i |= other->cathegoryEvent.i;
  if (!updatingFields)
    updatingFields = other->updatingFields;
  else if (other->updatingFields)
    updatingFields->splice(updatingFields->end(), *(other->updatingFields));
};

void Sm::trigger::DmlEvent::CathegoryEvent::linterDefinition(Sm::Codestream &str) {
  bool isNotFirst = false;

  auto outOr = [&isNotFirst]() -> string {
    if (isNotFirst)
      return " OR ";
    else
      isNotFirst = true;
    return "";
  };

  if (isDelete())
    str << outOr() << "DELETE";
  if (isInsert())
    str << outOr() << "INSERT";
  if (isUpdate())
    str << outOr() << "UPDATE";
}

void Sm::trigger::DmlEvent::linterDefinition(Sm::Codestream &str)  {
  cathegoryEvent.linterDefinition(str);
  if (updatingFields)
    str << " OF " << updatingFields;
}

std::string Sm::trigger::DmlEvent::toLogStringCahtegory() {
  Codestream str;
  cathegoryEvent.linterDefinition(str);
  return str.str();
}

std::string Sm::trigger::DmlEvents::toLogStringCahtegory() { return dmlEvents->toLogStringCahtegory(); }

std::string Sm::Trigger::toLogStringCahtegory() {
  std::string res;
  if (events)
    res += events->toLogStringCahtegory();
  res += " ";
  res += Sm::trigger::triggerModeToString(mode);
//  res += " ";
//  res += Sm::EnableState::toSTring(enableState);
  if (action && action->cathegoryTriggerActionInterface() == trigger::TriggerActionInterface::COMPOUND_TRIGGER_BLOCK) {
    res += " ";
    res += " COMPOUND BLOCK (TODO:)";
  }
  if (whenCondition) {
    Sm::Codestream s;
    s << "WHEN " << s::obracket << whenCondition << s::cbracket;
    res += s.str();
  }

  // TODO: доделать вывод категории триггера
  return res;
}


string trigger::TriggerAbstractRowReference::pseudorecordToString(trigger::TriggerAbstractRowReference::Pseudorecord t) {
  switch (t) {
    case NEW:    return "NEW";
    case OLD:    return "OLD";
    case PARENT: return "PARENT";
  }
  return "";
}

trigger::TriggerAbstractRowReference::TriggerAbstractRowReference(trigger::DmlEvents *_reference, Ptr<Id> _name, trigger::TriggerAbstractRowReference::Pseudorecord _cathegory, bool createdInResolving)
  : reference(_reference), name(_name), cathegory(_cathegory), createdInResolving_(createdInResolving) { name->definition((TriggerAbstractRowReference*)this); }

Sm::IsSubtypeValues Sm::trigger::TriggerAbstractRowReference::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (Ptr<ResolvedEntity> def = entity())
    return def->isSubtype(supertype, plContext);
  return EXPLICIT;
}

SemanticTree *trigger::TriggerAbstractRowReference::toSTreeBase() const { return name->toSNodeDecl(SCathegory::TriggerRowReference, this); }



void Sm::RefExpr::linterReference(Sm::Codestream &str) {
  if (ResolvedEntity *d = getNextDefinition())
    d->linterReference(str);
  else {
    str << reference;
    trError(str, s << "ERROR: RefExpr reference expression is unresolved: " << *reference << ":" << reference->entity()->getLLoc());
  }
}

std::string Sm::pl_expr::ComparsionList::opToString() const {
  switch (op) {
    case comparsion_list::IN:
      return "IN";
    case comparsion_list::EQ:
      return "=";
    case comparsion_list::NE:
      return "<>";
  }
  return "";
}

std::string Sm::pl_expr::ComparsionList::notOpToString() const {
  switch (op) {
    case comparsion_list::IN:
      return "NOT IN";
    case comparsion_list::EQ:
      return "<>";
    case comparsion_list::NE:
      return "=";
  }
  return "";
}

void Sm::pl_expr::ComparsionList::linterDefinition(Codestream &str) {
  if (!lhs || !rhs || rhs->empty())
    return;
  if (lhs->size() < 2)
    throw 999;
  if (rhs->size() > 1 || !(rhs->front()->list) || rhs->front()->list->empty() ||
      rhs->front()->list->size() > 1)
    throw 999;

  Ptr<SqlExpr> expr = rhs->front()->list->front();
  Sm::Subquery *q = expr->toSelfSubquery();

  if (!q)
    throw 999;
  str << s::obracket << lhs << s::cbracket;
  str << s::name << (isNot() ? notOpToString() : opToString()) << s::name;
  str << s::obracket << *q << s::cbracket;
}

void Sm::TrimFromExpr::linterDefinition(Codestream &str) {
  switch (extractedEntity_) {
    case LEADING:
       str << "ltrim";
      break;
    case TRAILING:
       str << "rtrim";
      break;
    case BOTH:
      if (charExpr)
        str << "ltrim" << s::obracket
                         << "rtrim" << s::obracket << fromEntity << s::comma() << charExpr << s::cbracket
                         << s::comma() << charExpr
                       << s::cbracket;
      else
        str << "trim" << s::obracket << fromEntity << s::cbracket;
      return;
  };
  str << s::obracket << fromEntity;
  if (charExpr)
    str << s::comma() << charExpr;
  str << s::cbracket;
}





trigger::TriggerCode::TriggerCode(CLoc l, Ptr<BlockPlSql> block)
  : GrammarBase(l), BlockPlSql(block)
{
  booleanStateVariables.push_back(new TriggerPredicateVariable(l, TriggerPredicateVariable::DELETING));
  booleanStateVariables.push_back(new TriggerPredicateVariable(l, TriggerPredicateVariable::UPDATING));
  booleanStateVariables.push_back(new TriggerPredicateVariable(l, TriggerPredicateVariable::INSERTING));
  for (auto &v : booleanStateVariables)
    v->setOwnerBlockPlSql(this);
}

SemanticTree *trigger::TriggerCode::toSTreeBase() const {
  SemanticTree *node = toSTreeBNew();
  CollectSNode(node, booleanStateVariables);
  collectSNodeBase(node);
  return node;
}

void trigger::TriggerCode::traverseDeclarations(DeclActor &tr) {
  BlockPlSql::traverseDeclarations(tr);
}

trigger::TriggerRowReference::TriggerRowReference(trigger::DmlEvents *_reference, Ptr<Id> _name, trigger::TriggerAbstractRowReference::Pseudorecord _cathegory, bool createdInResolving)
  : TriggerAbstractRowReference(_reference, _name, _cathegory, createdInResolving) {}

void trigger::TriggerRowReference::linterDefinition(Codestream &str) {
  if (cathegory == PARENT || createdInResolving_)
    return;
  string catStr = pseudorecordToString(cathegory);
  if (name && catStr == name->toNormalizedString())
    return;

  str << s::name << catStr << " AS " << name;
}

bool trigger::TriggerRowReference::linterMapReference(Sm::Codestream &str, const IdEntitySmart &ref) {
  // TODO: NEED KERNEL CHANGES!!!
  return false;

  // See PL/SQL Language Reference, Table 9–3 OLD and NEW Pseudorecord Field Values
  if (ref.size() < 2 || !reference->dmlEvents)
    return false;

  const DmlEvent::CathegoryEvent &cat = reference->dmlEvents->cathegoryEvent;
  if (cat.isUpdate())
    return false;
  else if (cat.isDelete() && cat.isInsert())
    return false;
  else if ((cat.isDelete() && rowCathegory() == NEW) ||
           (cat.isInsert() && rowCathegory() == OLD)) {
    str << "NULL";
    return true;
  }
  return false;
}


bool Variable::isCursorVariable() const {
  Ptr<Datatype> t = getDatatype();
  return (!syntaxerContext.translateReferences && t->isRowTypeOf()) || t->isRefCursor();
}

Sm::IsSubtypeValues Variable::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return datatype->isSubtype(supertype, plContext);
}


bool Variable::getFieldRef(Ptr<Id> &field) { return getVariableFieldRef(field); }

Variable::Variable(Id *_name, Ptr<Datatype> _datatype, bool _notNull, Ptr<PlExpr> _defaultValue, bool _isConstant, CLoc l)
  : GrammarBase (l),
    name         (_name        ),
    datatype     (_datatype    ),
    defaultValue_(_defaultValue)
{
  if (_notNull)
    flags.setNotNull();
  if (_isConstant)
    flags.setConstant();
  if (name)
    name->definition(this);
}

void Variable::traverseDeclarations(DeclActor &fun) { fun(this); }


Cursor::Cursor(CLoc l, Ptr<Id> _name, Ptr<Cursor::Parameters> _parameters, Ptr<Datatype> _rowtype, Ptr<Subquery> _select)
  : GrammarBase(l), name(_name), rowtype(_rowtype), select(_select), currentOpenCommand(0) {
  if (name)
    name->definition(this);
  int idx = 0;
  if (_parameters)
    for (Parameters::value_type &it : *_parameters) {
      it->indexInParameters = idx;
      ++idx;
      it->ownerCursor = (Cursor*)this;
      parameters.push_back(it);
    }
//  Ptr<Id> curvarName = new Id(*name);
//  curvarName->definition(0);
  if (select) {
    select->isSqlStatementRoot = true;
    select->setFldDefPosOnRootQuery();
  }
}

void Cursor::replaceChildsIf(ExprTr tr) { replace(tr, rowtype, actualCursorParameters, select); }


Sm::IsSubtypeValues Type::RefCursor::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  if (supertype && supertype->ddlCathegory() == ResolvedEntity::RefCursor_)
    return IMPLICIT_CAST_BY_FIELDS;
  if (datatype)
    return datatype->isSubtype(supertype, plContext);
  return EXPLICIT;
}

Type::RefCursor::RefCursor(CLoc l, Ptr<Id> _name, Ptr<Datatype> t)
  : GrammarBase(l), name(_name), datatype(t) { if (name) name->definition((RefCursor*)this); }

SemanticTree *Type::RefCursor::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::RefCursor, this);
  ANODE(datatype);
  return n;
}

Sm::IsSubtypeValues CursorParameter::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return datatype->isSubtype(supertype, plContext);
}

ResolvedEntity *CursorParameter::getFieldDDLContainer() const {
  return const_cast<Cursor*>(ownerCursor);
}


SemanticTree *CursorParameter::toSTreeBase() const {
  if (name) {
    SemanticTree *n = name->toSNodeDecl(SCathegory::Field, this);
    ANODE(datatype);
    CTREE2(n, defaultValue);
    return n;
  }
  return 0;
}


Sm::IsSubtypeValues Type::MemberVariable::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return datatype->isSubtype(supertype, plContext);
}

ResolvedEntity *Type::MemberVariable::getFieldDDLContainer() const {
  if (SemanticTree *n = getSemanticNode())
    if (SemanticTree *p = n->getParent())
      if (ResolvedEntity *d = p->ddlEntity())
        if (d->toSelfObjectType())
          return d;
  return 0;
}

Type::MemberVariable::MemberVariable(CLoc l, Ptr<Id> _name, Ptr<Datatype> _datatype, Ptr<Id> _jext)
  : GrammarBase(l), name(_name), datatype(_datatype), javaExtName(_jext) { name->definition((MemberVariable*)this); }

SemanticTree *Type::MemberVariable::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::Field, this);
  ANODE(datatype);
  return n;
}


pragma::ExceptionInit::ExceptionInit(CLoc l, Ptr<Id> _exceptionName, Ptr<NumericValue> errnum)
  : GrammarBase(l), exceptionName(_exceptionName), errorNumber(errnum) {}



pragma::RestrictReferences::RestrictReferences(CLoc l, PragmaRestrictFlags fl, Ptr<Id> funref)
  : GrammarBase(l), functionRef(funref), restrictFlags(fl), isDefault(funref ? false : true) {}


trigger::TriggerNestedRowReference::TriggerNestedRowReference(trigger::DmlEvents *_reference, Ptr<Id> _name, trigger::TriggerAbstractRowReference::Pseudorecord _cathegory, bool createdInResolving)
  : TriggerAbstractRowReference(_reference, _name, _cathegory, createdInResolving) {}


trigger::TriggerPredicateVariable::TriggerPredicateVariable(CLoc l, trigger::TriggerPredicateVariable::TypePredicate t)
  : GrammarBase(l),
    Variable(new Id(l, toSTring(t)), Sm::Datatype::mkBoolean(), false, 0, false, l) { setIsSystem(); }


Ptr<ResolvedEntity> trigger::TriggerRowReference::entity() const {
  return reference && reference->tableRef ? reference->tableRef->definition() : (ResolvedEntity*)0;
}


Ptr<ResolvedEntity> trigger::TriggerNestedRowReference::entity() const {
  return reference && reference->fieldNestedTable ? reference->fieldNestedTable->definition() : (ResolvedEntity*)0;
}

trigger::Funcall::Funcall(CLoc l, Ptr<SqlExpr> c, Ptr<SqlExpr> into)
  : GrammarBase(l), call(c), intoHostVar(into) {}

Sm::trigger::non_dml_events::T Sm::trigger::TriggerEvents::nonDmlEventsCathegoies() const { return trigger::non_dml_events::EMPTY; }


bool Sm::checkToRowidPseudocolumn(Ptr<Id> &field) {
  static const HString rowidPseudocolumn = "ROWID";
  static const HString rownumPseudocolumn = "ROWNUM";
  if (field) {
    if (*field == rowidPseudocolumn) {
      field->definition(Sm::RowIdExpr::self.object());
      field->setRowidPseudocol();
      return true;
    }
    else if (*field == rownumPseudocolumn) {
      field->definition(Sm::RowNumExpr::self.object());
      field->setRownumPseudocol();
      return true;
    }
  }
  return false;
}

bool View::getFieldRef(Ptr<Id> &field) {
  if (!field)
    return false;
  if (properties && properties->cathegoryViewProperties() == Sm::view::ViewProperties::OBJECT) {
    Sm::view::ObjectReference * objRef = (Sm::view::ObjectReference *)(properties.object());
    if (Ptr<ResolvedEntity> objdef = objRef->name->definition())
      if (objdef->getFieldRef(field))
        return true;
    if (objRef->superview)
      if (Ptr<ResolvedEntity> objdef = objRef->superview->definition())
        if (objdef->getFieldRef(field))
          return true;
  }
  if (aliasedFieldsNode && aliasedFieldsNode->childNamespace) {
    aliasedFieldsNode->childNamespace->findField(field);
    if (field->unresolvedDefinition()) {
      field->unresolvedDefinition()->semanticResolve();
      return true;
    }
  }

  if (!field->unresolvedDefinition() && select)
    return select->getFieldRef(field);
  return false;
}

void View::parseConstraintList()
{
  if (properties)
    if (Ptr<List<view::ViewConstraint> > l = properties->getViewConstraints())
      if (!(*l->begin())->constraints)
        for (List<view::ViewConstraint>::iterator it = l->begin(); it != l->end(); ++it)
          aliasedFields.push_back(new Sm::QueryPseudoField((*it)->constraintGroupAlias, this));
}

bool CursorParameter::getFieldRef(Ptr<Id> &field) {
  return datatype && datatype->getFieldRef(field);
}

bool Type::MemberVariable::getFieldRef(Ptr<Id> &name) {
  if (this->datatype)
    return datatype->getFieldRef(name);
  return false;
}


void Sm::table::field_property::XmlField::resolve(Ptr<ResolvedEntity> parent) {
  if (name && !name->definition())
    parent->getFieldRef(name);
  if (name->definition())
    name->definition()->addFieldProperty((XmlField*)this);
}

void Sm::table::field_property::VarrayField::resolve(Ptr<ResolvedEntity> parent) {
  if (fieldRef && !fieldRef->definition())
    parent->getFieldRef(fieldRef);
  if (fieldRef->definition())
    fieldRef->definition()->addFieldProperty((VarrayField*)this);
}

void Sm::table::TableProperties::aggregateFieldProperties(Ptr<ResolvedEntity> parent) {
  if(fieldsProperties)
    for (List<field_property::FieldProperty>::iterator it = fieldsProperties->begin(); it != fieldsProperties->end(); ++it)
      if (*it)
        (*it)->resolve(parent);
}


//bool checkAndEraseConstraint(alter_table::ModifyFields::Fields::iterator &fit, alter_table::ModifyFields::Fields *fields) {
//  if (!(*fit)->constraints() || (*fit)->constraints()->empty()) {
//    fit = fields->erase(fit);
//    return true;
//  }
//  return false;
//}
template <typename Container1, typename Container2, typename Iterator>
bool checkAndEraseCmd(Container1 &c1, Container2 &c2, Iterator &it) {
  if (c1->empty()) {
    it = c2.erase(it);
    return true;
  }
  return false;
}

//bool Sm::alter_table::ModifyFields::resolveFields(Sm::Table */*baseTable*/, ManipulateFieldsList &/*manipFields*/, ManipulateFieldsList::iterator &/*mit*/) {
//  throw 999;
//  if (fields) {
//    for (Fields::iterator fit = fields->begin(); fit != fields->end(); ) {
//      (*fit)->name->definition(0);
//      baseTable->getFieldRef((*fit)->name);
//      if (ResolvedEntity *fdef = (*fit)->name->definition())
//        fdef->parseConstraintsList((*fit)->constraints());
//        if (checkAndEraseConstraint(fit, fields))
//          continue;
//      ++fit;
//    }
//    if (checkAndEraseCmd(fields, manipFields, mit))
//      return true;
//  }
//  return false;
//}

std::string Sm::constraint::Attribute::key() {
  Codestream str;
  linterDefinition(str);
  return str.str();
}




void Sm::table::field_property::NestedTable::resolve(Ptr<ResolvedEntity> parent) {
  if (name && name->name && !name->name->definition())
    parent->getFieldRef(name->name);

  if (name->name->definition())
      name->name->definition()->addFieldProperty((NestedTable*)this);

  if (object && object->fieldRef && !object->fieldRef->definition())
    parent->getFieldRef(object->fieldRef);

  if (fieldsProperties)
    for (List<FieldProperty>::iterator it = fieldsProperties->begin(); it != fieldsProperties->end(); ++it)
      (*it)->resolve(parent);
}

void Sm::table::field_property::ObjectProperties::resolve(Ptr<ResolvedEntity> parent) {
  if (!fieldRef->definition() && parent)
    parent->getFieldRef(fieldRef);

  if (fieldRef->definition())
     fieldRef->definition()->addFieldProperty((ObjectProperties*)this);

  sAssert(constraints);
}

void Sm::table::field_property::ObjectField::resolve(Ptr<ResolvedEntity> parent) {
  if (!name->definition() && parent)
    parent->getFieldRef(name);
  if (name->definition())
    name->definition()->addFieldProperty((ObjectField*)this);
}

void Sm::view::ViewConstraint::resolve(Ptr<ResolvedEntity> ) {
  sAssert(constraints);
}

void Sm::view::ObjectReference::resolve(Ptr<ResolvedEntity> parent) {
  if (superview)
    resolveTable(*syntaxerContext.model, superview);
  resolveObject(*syntaxerContext.model, name);
  if (objectIdentifiers)
    if (Ptr<ResolvedEntity> objdef = name->definition())
    for (List<Id>::iterator it = objectIdentifiers->begin(); it != objectIdentifiers->end(); ++it)
      objdef->getFieldRef(*it);
  if (constraints)
    for (List<ViewConstraint>::iterator it = constraints->begin(); it != constraints->end(); ++it)
      (*it)->resolve(parent);
}

void Sm::Trigger::resolve(ModelContext &model) {
  if (followsAfterTrigger)
    for ( List<Id2>::iterator it = followsAfterTrigger->begin(); it != followsAfterTrigger->end(); ++it)
      resolveTrigger(model, it->object());
  if (!events)
    return;
  if (events->cathegoryTriggerEvents() == trigger::TriggerEvents::NON_DML_EVENT) {
    Sm::trigger::NonDmlEvents *p = (Sm::trigger::NonDmlEvents*)(events.object());
    if (p->schema && p->schemaOrDatabase == Sm::trigger::NonDmlEvents::SCHEMA)
      p->schema->definition(model.getUser(p->schema));
    return;
  }
  Sm::trigger::DmlEvents *p = (Sm::trigger::DmlEvents*)(events.object());
  if (p->tableRef) {
    resolveTable(model, p->tableRef.object());
    if (ResolvedEntity *d = p->tableRef->definition()) {
      if (p->fieldNestedTable)
        d->getFieldRef(p->fieldNestedTable);
      if (p->dmlEvents && p->dmlEvents->updatingFields) {
        for (List<Id>::iterator it = p->dmlEvents->updatingFields->begin(); it != p->dmlEvents->updatingFields->end(); ++it)
          d->getFieldRef(*it);
      }
    }
  }
}

void Sm::View::resolve(ModelContext &) {
  if (properties)
    properties->resolve((View*)this);
}


void Sm::table::TableProperties::resolve(Ptr<ResolvedEntity> parent) {
  if (fieldsProperties)
    for (List<field_property::FieldProperty>::iterator it = fieldsProperties->begin(); it != fieldsProperties->end(); ++it) {
      if (*it)
        (*it)->resolve(parent);
    }
}


bool Sm::constraint::ForeignKey::eq_(Attribute *attr) const {
  ForeignKey *o = (ForeignKey*)attr;
  return ((fieldListKey && fieldListKey  ->eq(o->fieldListKey))  || (!fieldListKey  && !o->fieldListKey )) &&
         ((referencedKey && referencedKey->eq(o->referencedKey)) || (!referencedKey && !o->referencedKey));
}


//void Sm::constraint::ForeignKey::resolve(Ptr<ResolvedEntity> parent) { // parent = table owner
//  throw 999;
//  static const auto inequallyTranslation = [](Ptr<Datatype> t1, Ptr<Datatype> t2) -> bool {
//    return t1->translatedToBigint() && t2->translatedToInt();
//  };

//  static const auto translatedNumber = [](Ptr<Datatype> t1, Ptr<Datatype> t2) -> bool {
//    return (t1->isNumberDatatype() && (t2->translatedToBigint() || t2->translatedToInt()));
//  };

//  if (fieldListKey && parent)
//    for (List<Id>::iterator it = fieldListKey->begin(); it != fieldListKey->end(); ++it)
//      parent->getFieldRef(*it);

//  if (referencedKey)
//    referencedKey->resolve(parent);
//  if (fieldListKey && parent && referencedKey && referencedKey->foreignColumnList()) {
//    Ptr<List<Id> > foreignList = referencedKey->foreignColumnList();
//    List<Id>::iterator currentIt        = fieldListKey->begin();
//    List<Id>::iterator foreignIt = foreignList->begin();

//    for (; currentIt != fieldListKey->end() && foreignIt != foreignList->end(); ++currentIt, ++foreignIt) {
//      if (!(*currentIt)->definition() || !(*foreignIt)->definition())
//        continue;
//      Ptr<Datatype> current = (*currentIt)->definition()->getDatatype();
//      Ptr<Datatype> foreign = (*foreignIt)->definition()->getDatatype();
//      if      (inequallyTranslation(current, foreign) || translatedNumber(foreign, current))
//        (*foreignIt)->definition()->foreignDatatype(current);
//      else if (inequallyTranslation(foreign, current) || translatedNumber(current, foreign))
//        (*currentIt)->definition()->foreignDatatype(foreign);
//    }
//  }
//}


Ptr<Datatype> Type::RefCursor  ::getDatatype() const { return getDatatypeWithSetFieldStruct(thisDatatype, name); }
Ptr<Datatype> Cursor           ::getDatatype() const { return getDatatypeWithSetFieldStruct(thisDatatype, name); }

Ptr<Datatype> Cursor::getVariableDatatype() const {
  if (rowtype)
    return rowtype;
  if (select)
    return select->getDatatype();
  else
    return 0;
}



bool Cursor::getFieldRef(Ptr<Id> &field) {
  return (rowtype && rowtype->getFieldRef(field)) ||
         (select  && select->getFieldRef(field));
}

bool Cursor::getFields(EntityFields &fields) const {
  if (fields_.empty()) {
    ResolvedEntity *fieldSrc = 0;
    EntityFields internalFlds_;
    if (rowtype) {
      rowtype->getFields(internalFlds_);
      fieldSrc = rowtype;
    }
    if (fields_.empty() && select) {
      select->getFields(internalFlds_);
      fieldSrc = select;
    }

    if (!internalFlds_.empty())
      const_cast<Cursor*>(this)->setFieldsFrom(internalFlds_, fieldSrc);
  }
  if (!fields_.empty()) {
    fields = VariableCursorFields::fields_;
    return true;
  }
  return false;
}

bool Type::RefCursor::getFields(std::vector<Ptr<Sm::Id> > &fields) const {
  if (datatype)
    return datatype->getFields(fields);
  return false;
  // TODO: для мягких RefCursor - нужно реализовать генерацию его структуры полей соответственно его последней инициализации.
}


Sm::IsSubtypeValues Sm::Cursor::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  if (rowtype)
    return rowtype->isSubtype(supertype, plContext);
  if (select)
    return select->isSubtype(supertype, plContext);
  return EXPLICIT;
}


SemanticTree *Sm::Cursor::toSTreeBase() const {
  SemanticTree *n = (rowtype && select) ? name->toSNodeDef(SCathegory::Cursor, this) : name->toSNodeDecl(SCathegory::Cursor, this);
  n->unnamedDdlEntity = (Cursor*)this;
  CollectSNode(n, parameters);
//  ANODE(cursorVariable)
  ANODE(rowtype);
  CTREE(select);
  return n;
}


SemanticTree *trigger::DmlEvents::toSTree() const {
  if (semanticNode)
    return 0;
  semanticNode = new SemanticTree(SCathegory::DmlEvents);
  CTREE2(semanticNode, dmlEvents);
  if (tableRef) {
    SemanticTree *tableRefNode = new SemanticTree(tableRef, SemanticTree::REFERENCE, SCathegory::TableOrView);
    if (fieldNestedTable)
      tableRefNode->addChild(new SemanticTree(fieldNestedTable, SemanticTree::REFERENCE, SCathegory::Field));
    semanticNode->addChild(tableRefNode);
  }
  return semanticNode;
}


SemanticTree *Trigger::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::Trigger, this);
  n->unnamedDdlEntity = (Trigger*)this;
  if (events) {
    ANODE(events)
    if (trigger::DmlEvents *p = events->toSelfDmlEvents()) {
      for (int i = 0; i < 3; ++i)
        n->addChild(p->pseudorecords[i]->toSTree());
    }
  }
  CTREE(whenCondition)

  if (action)
    action->collectSNode(n);
  return n;
}


bool Type::RefCursor::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  if (RefCursor *othObj = oth->toSelfRefCursor())
    return this->eqByVEntities(othObj);
  return false;
}


SemanticTree *pragma::ExceptionInit::toSTreeBase() const { return exceptionName->toSNodeRef(SCathegory::Exception, this); }


SemanticTree *pragma::Inline::toSTreeBase() const { return functionRef ? functionRef->toSNodeRef(SCathegory::Function) : (SemanticTree*)0; }
SemanticTree *pragma::RestrictReferences::toSTreeBase() const {
  return functionRef ? functionRef->toSNodeRef(SCathegory::FunctionPragmaRestriction, this) : (SemanticTree*)0;
}
SemanticTree *trigger::NonDmlEvents::toSTree() const { return schema ? schema->toSNodeRef(SCathegory::Schema) : 0; }
void trigger::Funcall::collectSNode(SemanticTree *n) const { (void)n; CTREE(call); CTREE(intoHostVar); }



