#include <stdlib.h>
#include "siter.h"
#include "i2str.h"
#include "model_context.h"
#include "semantic_id.h"
#include "syntaxer_context.h"
#include "semantic_collection.h"
#include "semantic_function.h"
#include "system_sysuser.h"
#include "semantic_blockplsql.h"
#include "semantic_sql.h"
#include "resolvers.h"
#include "semantic_sql.h"
#include "dynamic_sql_op.h"
#include "semantic_plsql.h"
#include "smart_lexer.h"

using namespace std;
using namespace Sm;
using namespace PlsqlHelper;

extern SyntaxerContext syntaxerContext;
ResolvingContext *currentResolvingContext = 0;
extern bool __STreeDestructionStarted__;



Ptr<Sm::SqlExpr> Sm::SqlExpr::boolFalse() {
  return new RefExpr(new Id(std::string("FALSE"), 0, false));
}


bool ResolvedEntity::isVariable() const {
  switch (ddlCathegory()) {
    case ResolvedEntity::Variable_:
    case ResolvedEntity::FunctionArgument_:
      return true;
    default:
      return false;
  }
}

mpf_class Sm::NumericFloat::maxSequenceValue( "9223372036854775807");
mpf_class Sm::NumericFloat::minSequenceValue("-9223372036854775808");
mpz_class Sm::NumericInt  ::maxSequenceValue( "9223372036854775807");
mpz_class Sm::NumericInt  ::minSequenceValue("-9223372036854775808");

bool Sm::Id::isSemanticString(const char *keyword, int length) const {
  unsigned int decLen = --length;
  if ((unsigned int)(length) == text->size() ||
      (decLen < text->size() &&
       (keyword[decLen] == '\0' || toupper(keyword[decLen]) == toupper((char)((*text)[decLen])) ))) {
    const char *c = keyword;
    for (string::const_iterator it = text->begin(); it != text->end(); ++it, ++c)
      if (toupper(*it) != toupper(*c))
        return false;
    return true;
  }
  else
    return false;
  return true;
}

void Id::textToUpper()
{
  setNormalized();
  hashValue = BackportHashMap::internal::HashFunctions::std_string_hash(normalizedStr);
  text = getCachedTextPtr(hashValue, normalizedStr);
}

void Id::setUpperAndQuoted() {
  textToUpper();
  setQuoted();
}

void Id::toUpperIfUnquoted()
{
  if (notquoted())
    textToUpper();
}

string Sm::NumericInt::toString(bool isProc) const {
  char buf[512];
  string src;
  if (intval < maxSequenceValue) {
    if (intval > minSequenceValue)
      src = string(buf, gmp_snprintf(buf, 512, "%Zd", intval.get_mpz_t()));
    else
      src = string(buf, gmp_snprintf(buf, 512, "%Zd", minSequenceValue.get_mpz_t()));
  }
  else
    src =  string(buf, gmp_snprintf(buf, 512, "%Zd", maxSequenceValue.get_mpz_t()));
  if (isStringLiteral()) {
    string dst;
    if (isProc)
      quotingAndEscapingSpec(dst, src, '\"', '\\');
    else
      quotingAndEscapingSpec(dst, src, '\'', '\'');
    return dst;
  }
  return src;
}


string Sm::NumericFloat::toString(bool isProc) const {
  char buf[512];
  string src;
  if (floatVal < maxSequenceValue) {
    if (floatVal > minSequenceValue)
      src = string(buf, gmp_snprintf(buf, 512, "%Ff", floatVal.get_mpf_t()));
    else
      src = string(buf, gmp_snprintf(buf, 512, "%Ff", minSequenceValue.get_mpf_t()));
  }
  else
    src = string(buf, gmp_snprintf(buf, 512, "%Ff", maxSequenceValue.get_mpf_t()));
  if (isStringLiteral()) {
    string dst;
    if (isProc)
      quotingAndEscapingSpec(dst, src, '\"', '\\');
    else
      quotingAndEscapingSpec(dst, src, '\'', '\'');
    return dst;
  }
  return src;
}

string NumericSimpleInt::toString(bool isProc) const {
  if (isStringLiteral()) {
    string dst;
    if (isProc)
      quotingAndEscapingSpec(dst, itostr(simpleInt), '\"', '\\');
    else
      quotingAndEscapingSpec(dst, itostr(simpleInt), '\'', '\'');
    return dst;
  }
  return itostr(simpleInt);
}

bool Sm::Synonym::isSystem() const {
  if (isSystemFlag())
    return true;
  if (target) {
    if (target->uname())
      if (ResolvedEntity *tuser = target->uname()->definition())
        if (syntaxerContext.model->sysusers.count(tuser))
          return true;
    if (ResolvedEntity *def = target->definition())
      return def->isSystem();
  }
  return false;
}




namespace Sm { // toString


uint Id::getHashValue(char *_text, size_t length, bool isQuoted) {
  if (isQuoted)
    return BackportHashMap::internal::HashFunctions::cchar_hash(_text, length);
  else
    return BackportHashMap::internal::HashFunctions::cchar_upper_hash(_text, length);
}
uint Id::getHashValue(const std::string &_text, bool isQuoted) {
  if (isQuoted)
    return BackportHashMap::internal::HashFunctions::std_string_hash(_text);
  else
    return BackportHashMap::internal::HashFunctions::std_string_upper_hash(_text);
}

bool Id::operator==(const HString &o) const {
  if (hashValue == o.hash()) {
    if (normalizedStr.empty()) {
      if (quoted())
        return *text == o;
      else {
        setNormalized();
        return normalizedStr == o;
      }
    }
    else
      return normalizedStr == o;
  }
  return false;
}

/// Сравнение с оптимизацией производительности ветвления
bool Id::operator==(const Id &o) const {
  if (hashValue == o.hashValue) {
    if (normalizedStr.empty() || o.normalizedStr.empty()) {
      if (notquoted()) {
        string::size_type len  =   text->length();
        string::size_type olen = o.text->length();
        const char *c  =   text->data();
        const char *oc = o.text->data();
        const char *end_c = c + len;
        if (len == olen) {
          if (o.notquoted()) { // 0 0
            for (; c != end_c; ++c, ++oc)
              if (toupper(*c) != toupper(*oc))
                return false;
            return true;
          }
          else { // 0 q
            for (; c != end_c; ++c, ++oc)
              if (toupper(*c) != *oc)
                return false;
            return true;
          }
        }
      }
      else {
      if (o.quoted())  // q q
        return text == o.text;
      else { // q 0
        string::size_type  len =   text->length();
        string::size_type olen = o.text->length();
        const char *c =    text->data();
        const char *oc = o.text->data();
        const char *end_c = c + len;
        if (len == olen) {
          for (; c != end_c; ++c, ++oc)
            if (*c != toupper(*oc))
              return false;
          return true;
        }
      }
    }
    }
    else
      return normalizedStr == o.normalizedStr;
  }
  return false;
}

}

namespace Sm { // getFieldRef

bool setDDLReferenceToField(Ptr<ResolvedEntity> parent, Ptr<Id> fieldReference) {
  return parent && parent->getFieldRef(fieldReference);
}

void Sm::Subquery::pullUpOrderGroup(QueryBlock *child) {
  if (child && ownerSelectModel) {
    if (child->tailSpec && child->tailSpec->groupBy) {
      groupBy = child->tailSpec->groupBy;
      child->tailSpec->groupBy = 0;
    }
    if (child->orderBy) {
      orderBy = child->orderBy;
      child->orderBy = 0;
    }
  }
}

Sm::AnalyticFun::AnalyticFun(Ptr<Id> _name, Ptr<IdEntitySmart> refcall, Ptr<OrderBy> _orderByClause)
  : GrammarBase(refcall->entity()->getLLoc()), RefAbstract(refcall), name(_name), orderByClause(_orderByClause)
{
  static const AnalyticCathegoryMap cathegories =
  {
    AnalyticCathegoryMap::value_type("LAG"        , LAG         ),
    AnalyticCathegoryMap::value_type("FIRST_VALUE", FIRST_VALUE ),
    AnalyticCathegoryMap::value_type("SUM"        , SUM         ),
    AnalyticCathegoryMap::value_type("ROW_NUMBER" , ROW_NUMBER  ),
    AnalyticCathegoryMap::value_type("COUNT"      , COUNT       ),
    AnalyticCathegoryMap::value_type("MAX"        , MAX         ),
    AnalyticCathegoryMap::value_type("MIN"        , MIN         ),
    AnalyticCathegoryMap::value_type("LEAD"       , LEAD        ),
  };
  name->definition(this);

  AnalyticCathegoryMap::const_iterator it = cathegories.find(name->toNormalizedString());
  if (it != cathegories.end())
    analyticCathegory = it->second;
  else
    throw 999; // нужно учесть все варианты аналитических функций

  if (name)
    callarglist = refcall->entity()->callArglist;
}

bool Sm::AnalyticFun::getFieldRef(Ptr<Id> &field) {
  switch (analyticCathegory) {
  case ROW_NUMBER:
  case COUNT:
    return false;
  default:
    if (callarglist)
      if (FunCallArg *arg = callarglist->front())
        return arg->getFieldRef(field);
    return false;
  }
}

Ptr<Datatype> Sm::AnalyticFun::getDatatype() const {
  switch (analyticCathegory) {
  case ROW_NUMBER:
    return Datatype::mkNumber();
  case COUNT:
    return Datatype::mkNumber(12);
  default:
    if (callarglist && callarglist->front())
      return callarglist->front()->getDatatype();
    break;
  }
  return 0;
}


bool Sequence::getFieldRef(Ptr<Id> &field) {
  if (field)
    for (int i = 0; i < 2; ++i)
      if (pseudocolumns[i]->getName()->normalizedString() == field->normalizedString()) {
        field->definition(pseudocolumns[i].object());
        return true;
      }
  return false;
}

bool Table::getFieldRef(Ptr<Id> &field) {
  if (objectName) {
    if (Ptr<ResolvedEntity> objdef = objectName->definition())
      return objdef->getFieldRef(field);
  }
  else if (relationFields) {
    if (!fieldSNode || !fieldSNode->childNamespace) {
      RelationFieldsMap::iterator it = relationFieldsMap.find(field->toNormalizedString());
      if (it != relationFieldsMap.end()) {
        field->definition(it->second);
        return true;
      }
    }
    else
      return fieldSNode->childNamespace->findField(field);
  }
  return false;
}



bool View::semanticResolve() const {
  semanticResolveBase();
  select->semanticResolve();
  EntityFields fields;
  select->getFields(fields);
  EntityFields::iterator fit = fields.begin();
  for(AliasedFields::const_iterator it = aliasedFields.begin(); it != aliasedFields.end() && fit != fields.end(); ++it, ++fit) {
    QueryPseudoField *f = *it;
    f->definition((*fit)->definition());
    f->setSemanticNode((*fit)->semanticNode());
  }
  return true;
}



bool Type::RecordField::getFieldRef(Ptr<Sm::Id> &field) {
  return datatype && datatype->getFieldRef(field);
}


bool Type::Object::getFieldRef(Ptr<Id> &reference) {
  if (!reference)
    return false;
  // искать по мемберам, пока не отрезолвится
  Ptr<ResolvedEntity> ent = (Object*)this;
  if (!isDefinition())
    ent = getDefinitionFirst();
  if (SemanticTree *n = ent->getSemanticNode())
    if (LevelResolvedNamespace *childNamespace = n->childNamespace.object()) {
      Sm::SemanticTree *node = 0;
      if (childNamespace->findDeclaration(node, reference)) {
        if (!reference->semanticNode())
          reference->semanticNode(node);
        return true;
      }
    }
  return false;
  // for (List<MemberInterface>::iterator it = o->elements.begin(); it != o->elements.end(); ++it)
  //   if (Ptr<Id> memberName = (*it)->getMemberName())
  //     if (field->normalizedString() == memberName->normalizedString()) {
  //       field->definition((*it)->getThisDefinition());
  //       return true;
  //     }
  // return false;
}

bool Datatype::getFieldRef(Ptr<Id> &field) {
  if (ResolvedEntity *def = tidDef())
    return def->getFieldRef(field);
  semanticResolve();
  if (ResolvedEntity *def = tidDef())
    return def->getFieldRef(field);
  return false;
}

Ptr<ResolvedEntity> Datatype::tidDdl() const {
  Datatype *t = (Datatype*)this;
  return t->tryResoveConcreteDefinition();
}

bool Subtype::getFieldRef(Ptr<Id> &field) { return datatype && datatype->getFieldRef(field); }
ResolvedEntity* Subtype::getNextDefinition() const { return datatype ? datatype.object() : (ResolvedEntity*)0; }

bool UnionQuery::getFieldRef(Ptr<Id> &field) {
  if (lhs && lhs->getFieldRef(field)) {
    if (ResolvedEntity *def = field->definition()) {
      bool needToChanged = def->isElementaryLiteral() || def->isNullField();
      if (!needToChanged) {
        Ptr<Datatype> t = def->getDatatype();
        if (!t || t->isNull())
          needToChanged = true;
      }
      if (needToChanged) {
        field->clearDefinition();
        rhs->getFieldRef(field);
        if (!field->unresolvedDefinition())
          field->definition(def);
      }
    }
    return true;
  }
  return (rhs && rhs->getFieldRef(field));
}

ResolvedEntity* UnionQuery::getNextDefinition() const {
  ResolvedEntity *lhsE = 0, *rhsE = 0;
  if (lhs)
    lhsE = lhs->getNextDefinition();
  if (rhs)
    rhsE = rhs->getNextDefinition();
  return lhsE ? lhsE : rhsE;
}

void FactoringItem::connectAliasWithQuery() const
{
  if (!columnAliases)
    return;

  FactoringItem* obj = (FactoringItem*)this;
  std::vector<Ptr<Id> > v;
  for (BaseList<Id>::iterator it = obj->columnAliases->begin(); it != obj->columnAliases->end(); ++it)
    v.push_back(*it);
  std::vector<Ptr<Id> >::iterator vit = v.begin();
  obj->subquery->assignFieldReferenceByPosition(v, vit);
}

bool FactoringItem::getFieldRef(Ptr<Sm::Id> &field) {
  connectAliasWithQuery();
  return subquery && subquery->getFieldRef(field);
}
ResolvedEntity* FactoringItem::getNextDefinition() const { return subquery ? subquery->getNextDefinition() : (ResolvedEntity*)0; }


bool FromTableReference::getFieldRef(Ptr<Sm::Id> &field) {
  if (id) {
    if (!id->definition())
      semanticResolve();
    ResolvedEntity *def;
    if (((def = id->definition()) && def->getFieldRef(field)) || Sm::checkToRowidPseudocolumn(field))
      return true;
  }
  return false;
}

ResolvedEntity* FromTableReference::getNextDefinition() const {
  return id && id->definition() ? id->definition()->getNextDefinition() : (ResolvedEntity*)0;
}

bool FromTableSubquery::getFieldRef(Ptr<Sm::Id> &field) { return subquery && subquery->getFieldRef(field); }

bool FromTableDynamic::getFieldRef(Ptr<Sm::Id> &field) {
  if (dynamicTable) {
    if (dynamicTable->getFieldRef(field)) {
      initDynamicFields();
      if (ResolvedEntity *d = field->unresolvedDefinition())
        if (ResolvedEntity *dF = d->getDefinitionFirst()) {
          DynamicFieldsMap::iterator it = fieldsMap.find(dF);
          if (it != fieldsMap.end())
            field->definition(it->second.object());
        }
      return true;
    }
    else if (field->toNormalizedString() == "COLUMN_VALUE") {
      if (!columnValue) {
        columnValue = new QueryPseudoField(new Id("COLUMN_VALUE"), this);
        columnValue->definition(dynamicTable->getDatatype());
        columnValue->isColumnValue  = true;
      }
      field->definition(columnValue.object());
      return true;
    }
  }
  return false;
}

ResolvedEntity* FromTableDynamic::getNextDefinition() const {
  if (dynamicTable)
    return dynamicTable->getNextDefinition();
  return 0;
}
bool FromSingle::getFieldRef(Ptr<Id> &field) { return reference && reference->getFieldRef(field); }

bool FromSingle::getFieldRefAsTableName(Ptr<Id> &field) {
  if (field) {
    if (alias && alias->normalizedString() == field->normalizedString()) {
      field->definition(reference.object());
      return true;
    }
    else
      return reference->getFieldRefAsTableName(field);
  }
  return false;
}

bool FromJoin::getFieldRef(Ptr<Id> &field) {
  /* Т.к. искаться будет старшая часть имени - нужно проверять совпадение с псевдонимом */
  if (lhs) {
    lhs.object()->noCheckAlias = true;
    if (lhs->getFieldRef(field))
      return true;
  }
  if (rhs) {
    rhs.object()->noCheckAlias = true;
    return rhs->getFieldRef(field);
  }
  return false;
}

bool FromJoin::getFieldRefAsTableName(Ptr<Id> &field) {
  if ((lhs && lhs->getFieldRefAsTableName(field)) ||
      (rhs && !field->definition() && rhs->getFieldRefAsTableName(field)))
    return true;
  return false;
}

bool FromJoin::getFields(EntityFields &fields) const {
  if (lhs)
    lhs->getFields(fields);
  if (rhs)
    rhs->getFields(fields);
  return true;
}

bool SelectSingle::getFieldRef(Ptr<Id> &field) { return queryBlock && queryBlock->getFieldRef(field); }
bool SelectBrackets::getFieldRef(Ptr<Id> &field) { return subquery && subquery->getFieldRef(field); }

}

namespace Sm { // resolve

void Sm::AlterUser::resolve(ModelContext &model) {
  for (List<Id>::iterator it = users->begin(); it != users->end(); ++it)
    if (*it)
      if (UserContext *cntx = model.userMap.find((*it)->toNormalizedString())->second.object() )
        (*it)->definition(cntx);
}

void Sm::Index::resolve(ModelContext &model) {
  Sm::resolve(model, table);
  Ptr<ResolvedEntity> tableDdl = table->entity()->definition();
  if (alias)
    alias->definition(tableDdl);
  if (tableDdl && fieldList)
    for (List<SqlExpr>::iterator it = fieldList->begin(); it != fieldList->end(); ++it) {
      Ptr<Id> fname = (*it)->getName();
      if (fname)
        tableDdl->getFieldRef(fname);
    }
}

void Sm::Synonym::resolve(ModelContext &model) {
  Sm::resolve(model, target);
  if (name && name->entity())
    name->entity()->definition(target->entity()->definition());
}

bool Sm::Table::diff(Ptr<Table> tbl) {
  othTable = tbl;
  diffFields.clear();
  if (!relationFields || !tbl->relationFields)
    return false;
  RelationFields currLst = *relationFields;
  RelationFields othLst  = *tbl->relationFields;

  for (RelationFields::iterator currIt = currLst.begin(); currIt != currLst.end(); ) {
    bool founded = false;
    for (RelationFields::iterator othIt = othLst.begin(); othIt != othLst.end(); )
      if (*((*currIt)->name) == *((*othIt)->name)) {
        if (!(*currIt)->getDatatype()->isExactlyEqually((*othIt)->getDatatype()))
          diffFields.push_back(make_pair(*currIt, *othIt));
        othIt = othLst.erase(othIt);
        founded = true;
        break;
      }
      else
        ++othIt;
    if (founded)
      currIt = currLst.erase(currIt);
    else
      ++currIt;
  }
  for (RelationFields::iterator currIt = currLst.begin(); currIt != currLst.end(); ++currIt)
    diffFields.push_back(make_pair(*currIt, Ptr<table::FieldDefinition>()));
  for (RelationFields::iterator othIt = othLst.begin(); othIt != othLst.end(); ++othIt)
    diffFields.push_back(make_pair(Ptr<table::FieldDefinition>(), *othIt));
  return diffFields.size();
}


}
namespace Sm { // Datatype constructors

class DefaultType : public ResolvedEntity {
public:
  DefaultType* toSelfDefaultType() const { return const_cast<DefaultType*>(this); }
  ScopedEntities ddlCathegory() const { return EMPTY__; }
  SmartVoidType* getThisPtr() const  { return (SmartVoidType*)this; }
  Ptr<Datatype> getDatatype() const { return Datatype::mkDefault(); }
  bool isExactlyEquallyByDatatype(ResolvedEntity *) { throw 999; return false; }
  void sqlReference  (Sm::Codestream &str)  { str << "default"; }
};
class NullType : public ResolvedEntity {
public:
  NullType* toSelfNullType() const { return const_cast<NullType*>(this); }
  ScopedEntities ddlCathegory() const { return EMPTY__; }
  SmartVoidType* getThisPtr() const  { return (SmartVoidType*)this; }
  Ptr<Datatype> getDatatype() const { return Datatype::mkNull(); }
  bool isExactlyEquallyByDatatype(ResolvedEntity *oth) { return oth->getDatatype()->isNull(); }
  void sqlReference  (Sm::Codestream &str)  { str << "NULL"; }
  void linterReference  (Sm::Codestream &str)  { str << "NULL"; }
};

static Ptr<DefaultType> defaultType = new DefaultType();
static Ptr<NullType>    nullType    = new NullType();

Datatype* Datatype::mkDefault() {
  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find("DEFAULT");
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();
   Ptr<IdEntitySmart> tid = new IdEntitySmart("__DEFAULT_TID__");
   Ptr<Datatype> t = new Datatype(tid);
   tid->entity()->definition(defaultType.object());
   t->setIsDefault();
   Datatype::globalDatatypes["DEFAULT"] = t;
   return t;
}

Datatype* Datatype::mkNull() {
  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find("NULL");
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();
  Ptr<IdEntitySmart> tid = new IdEntitySmart("__NULL_TID__");
  Ptr<Datatype> t = new Datatype(tid);
  tid->entity()->definition(nullType.object());
  t->setIsNull();
  Datatype::globalDatatypes["NULL"] = t;
  return t;
}

Datatype* Datatype::mkAnydata() {
  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find("ANYDATA");
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();
  Ptr<Datatype> t = new Datatype(syntaxerContext.model->sysuser()->anydata()->getName());
  Datatype::globalDatatypes["ANYDATA"] = t;
  return t;
}

Datatype* Datatype::mkXmltype() {
  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find("XMLTYPE");
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();
  Ptr<Sm::Type::Object> xmltype = syntaxerContext.model->sysuser()->xmltype();
  if (!xmltype) {
    cout << "ERROR: Xmltype is not found." << endl;
    return 0;
  }
  Ptr<Datatype> t = new Datatype(xmltype->getName());
  Datatype::globalDatatypes["XMLTYPE"] = t;
  return t;
}

Datatype* Datatype::getSystemDatatype(const string &name) {
  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find(name);
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();
  ResolvedEntity *def = syntaxerContext.model->globalDatatypes.systemTypesMap.find(THCChar(name))->object();
  std::pair<GlobalDatatypes::iterator, bool> insIt = Datatype::globalDatatypes.insert(GlobalDatatypes::value_type(name, 0));
  Ptr<Id> tname = new Id(string(name), def, false);
  Ptr<Datatype> p = new Datatype(tname);
  insIt.first->second = p;
  return p.object();
}

Datatype* Datatype::getSystemDatatype(const string &name, int prec, int scale) {
  if (prec <= 0)
    return getSystemDatatype(name);

  stringstream str;
  str << name;
  if (prec > 0) {
    str << "(" << prec;
    if (scale > 0)
      str << "," << scale << ")";
  }
  string fullname = str.str();

  Datatype::GlobalDatatypes::iterator it = Datatype::globalDatatypes.find(fullname);
  if (it != Datatype::globalDatatypes.end())
    return it->second.object();

  Ptr<Id> tname = new Id(string(name), 0, false);
  Datatype *t = getSystemDatatype(name);
  Datatype *newT = new Datatype(tname);
  tname->definition(t);

  newT->precision = prec;
  if (scale > 0)
    newT->scale = scale;

  Datatype::globalDatatypes[fullname] = newT;
  return newT;
}

#define GET_SYS_DATATYPE(NAME) \
  static Datatype *t = 0; \
  if (t) \
    return t; \
  return (t = getSystemDatatype(NAME))


Datatype* Datatype::mkNumber(int prec, int scale) { return getSystemDatatype("NUMBER", prec, scale); }
Datatype* Datatype::mkBoolean()              { GET_SYS_DATATYPE("BOOLEAN"         ); }
Datatype* Datatype::mkString()               { GET_SYS_DATATYPE("STRING"          ); }
Datatype* Datatype::mkDate()                 { GET_SYS_DATATYPE("DATE"            ); }
Datatype* Datatype::mkInteger()              { GET_SYS_DATATYPE("INTEGER"         ); }
Datatype* Datatype::mkNumber()               { GET_SYS_DATATYPE("NUMBER"          ); }
Datatype* Datatype::mkDouble()               { GET_SYS_DATATYPE("DOUBLE"          ); }
Datatype* Datatype::mkRowid()                { GET_SYS_DATATYPE("ROWID"           ); }
Datatype* Datatype::mkTimestamp()            { GET_SYS_DATATYPE("TIMESTAMP SIMPLE"); }
Datatype* Datatype::mkTimestampTimezone()    { GET_SYS_DATATYPE("TIMESTAMP_TZ_UNCONSTRAINED"    ); }
Datatype* Datatype::mkTimestampLtzTimezone() { GET_SYS_DATATYPE("TIMESTAMP_LTZ_UNCONSTRAINED"   ); }
Datatype* Datatype::mkIntervalDayToSecond()  { GET_SYS_DATATYPE("DSINTERVAL_UNCONSTRAINED"    ); }
Datatype* Datatype::mkIntervalYearToMonth()  { GET_SYS_DATATYPE("YMINTERVAL_UNCONSTRAINED"    ); }
Datatype* Datatype::mkDecimal()              { GET_SYS_DATATYPE("DECIMAL"         ); }
Datatype* Datatype::mkFloat()                { GET_SYS_DATATYPE("FLOAT"           ); }
Datatype* Datatype::mkReal()                 { GET_SYS_DATATYPE("REAL"            ); }
Datatype* Datatype::mkSmallint()             { GET_SYS_DATATYPE("SMALLINT"        ); }
Datatype* Datatype::mkBinaryInteger()        { GET_SYS_DATATYPE("BINARY_INTEGER"  ); }
Datatype* Datatype::mkNatural()              { GET_SYS_DATATYPE("NATURAL"         ); }
Datatype* Datatype::mkNaturalN()             { GET_SYS_DATATYPE("NATURALN"        ); }
Datatype* Datatype::mkPositive()             { GET_SYS_DATATYPE("POSITIVE"        ); }
Datatype* Datatype::mkPositiveN()            { GET_SYS_DATATYPE("POSITIVEN"       ); }
Datatype* Datatype::mkSignType()             { GET_SYS_DATATYPE("SIGNTYPE"        ); }
Datatype* Datatype::mkPlsInteger()           { GET_SYS_DATATYPE("PLS_INTEGER"     ); }
Datatype* Datatype::mkBlob()                 { GET_SYS_DATATYPE("BLOB"            ); }
Datatype* Datatype::mkClob()                 { GET_SYS_DATATYPE("CLOB"            ); }
Datatype* Datatype::mkNClob()                { GET_SYS_DATATYPE("NCLOB"           ); }
Datatype* Datatype::mkBfile()                { GET_SYS_DATATYPE("BFILE"           ); }
Datatype* Datatype::mkChar()                 { GET_SYS_DATATYPE("CHAR"            ); }
Datatype* Datatype::mkRaw()                  { GET_SYS_DATATYPE("RAW"             ); }
Datatype* Datatype::mkLongRaw()              { GET_SYS_DATATYPE("LONG RAW"        ); }
Datatype* Datatype::mkURowId()               { GET_SYS_DATATYPE("UROWID"          ); }
Datatype* Datatype::mkLong()                 { GET_SYS_DATATYPE("LONG"            ); }
Datatype* Datatype::mkVarchar2()             { GET_SYS_DATATYPE("VARCHAR2"        ); }
Datatype* Datatype::mkNChar()                { GET_SYS_DATATYPE("NCHAR"           ); }
Datatype* Datatype::mkNVarchar2()            { GET_SYS_DATATYPE("NVARCHAR2"       ); }
Datatype* Datatype::mkNVarchar()             { GET_SYS_DATATYPE("NVARCHAR"        ); }
Datatype* Datatype::mkVarchar()              { GET_SYS_DATATYPE("VARCHAR"         ); }



Datatype* Datatype::mkVarchar2(int length)   {
  Datatype::Varchar2LengthDatatypes::iterator it = Datatype::varchar2LengthDatatypes.find(length);
  if (it != Datatype::varchar2LengthDatatypes.end())
    return it->second.object();

  Datatype *t = new Datatype(Datatype::mkVarchar2(), length);
  Datatype::varchar2LengthDatatypes[length] = t;
  return  t;
}

Datatype* Datatype::mkChar(int length)   {
  Datatype::Varchar2LengthDatatypes::iterator it = Datatype::charLengthDatatypes.find(length);
  if (it != Datatype::varchar2LengthDatatypes.end())
    return it->second.object();

  Datatype *t = new Datatype(Datatype::mkChar(), length);
  Datatype::charLengthDatatypes[length] = t;
  return  t;
}

}
namespace Sm { // isSubtype interface
bool Subtype::datatypeIsNotResolved() const { return !datatype || !datatype->tidDef(); }

template <typename T>
int isTypesEquality(Ptr<ResolvedEntity> thisDdl, Ptr<ResolvedEntity> othDdl) { return thisDdl->eqByVEntities(othDdl) ? 1 : 0; }


Sm::IsSubtypeValues Subtype::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (supertype) {
    while (supertype->isDatatype())
      if(!(supertype = supertype->getNextDefinition()))
         return EXPLICIT;
    if (eqByVEntities(supertype))
      return EXACTLY_EQUALLY;
    return datatype->isSubtype(supertype, plContext);
  }
  return EXPLICIT;
}

Sm::IsSubtypeValues Type::Object::isSubtype(ResolvedEntity *_supertype, bool plContext) const {
  if (!_supertype)
    return EXPLICIT;
  if (eqByVEntities(_supertype))
    return EXACTLY_EQUALLY;
  Ptr<ResolvedEntity> superType = _supertype->tryResoveConcreteDefinition();

  if (superType->ddlCathegory() != ResolvedEntity::Object_) {
    // TODO: возможно нужно сравнение по полям.
    return EXPLICIT;
  }

  Ptr<Id2> superName = superType->getName2();

  if (superType == static_cast<const ResolvedEntity*>(this) ||
      (superName->uname()->normalizedString()  == name->uname()->normalizedString() &&
       superName->entity()->normalizedString() == name->entity()->normalizedString()))
    return EXACTLY_EQUALLY;

  if (this->supertype)
    if (Ptr<ResolvedEntity> superT = this->supertype->definition())
      if (Ptr<ResolvedEntity> thisSupertype = superT->tryResoveConcreteDefinition())
        return thisSupertype->isSubtype(superType, plContext);
  return EXPLICIT;
}

Sm::IsSubtypeValues Datatype::isSubtypeAsRef(Datatype *supertypeDef, bool plContext) const {
  throw 999;
  // Тип A является подтипом типа B если
  // A == Ref A1, B == Ref B1 => если A1 - подтип B1
  ResolvedEntity *unrefA = getNextDefinition(); // UNREF(A)
  if (unrefA == 0) {
    tryResoveConcreteDefinition();
    unrefA = tidDef();
  }
  if (unrefA) {
    ResolvedEntity *unrefB = supertypeDef->getNextDefinition(); // UNREF(B)
    if (unrefB == 0) {
      supertypeDef->tryResoveConcreteDefinition();
      if (!(unrefB = supertypeDef->tryResoveConcreteDefinition()))
        return EXPLICIT;
    }
    return (unrefA->isSubtype(unrefB->getNextDefinition(), plContext)); // return UNREF(A) IS SUBTYPE UNREF(B)
  }
  else
    return EXPLICIT;
}

Datatype::ScopedEntities Datatype::ddlCathegoryWithDatatypeSpec() const {
  unsigned int thisFlags = flags.v;
  if (thisFlags & (FLAG_DATATYPE_IS_TYPE_OF | FLAG_DATATYPE_IS_ROWTYPE_OF |
                   FLAG_DATATYPE_IS_NULL    | FLAG_DATATYPE_IS_DEFAULT))
  {
    if (thisFlags & FLAG_DATATYPE_IS_TYPE_OF)
      return DatatypeType_;
    if (thisFlags & FLAG_DATATYPE_IS_ROWTYPE_OF)
      return DatatypeRowtype_;
    if (thisFlags & FLAG_DATATYPE_IS_NULL)
      return DatatypeNull_;
    if (thisFlags & FLAG_DATATYPE_IS_DEFAULT)
      return DatatypeDefault_;
  }
  return Datatype_;
}

namespace SubtypeRelationsAction {
  enum Action {
    l_neq,
    l_implicit,
    l_eq,
    l_eq_in_sql_oth_throw,

    l_compare_by_entity,
    l_compare_rowtypes,
    l_compare_by_entity_and_fields,
    l_compare_by_entity_and_datatypes,
    l_compare_by_fields,

    l_compare_fundamental_datatypes,

    l_sup_nextdef,
    l_sup_nextdef_datatype,
    l_sub_nextdef,
    l_sub_nextdef_datatype,
    l_sub_get_datatype,
    l_sub_get_subtype,


    l_sub_compare_by_entity_or_nextdef,
    l_sub_unwrap_subquery_in_sql,
    l_sub_unref_collection,
    l_sub_unref_datatype_type,

    l_sup_unref_datatype_type,
    l_sup_get_datatype,

    l_undefined
  };
}


class SubtypeRelations : public CathegoriesOfDefinitions {
public:
                                            /*    subtype                               supertype */
  SubtypeRelationsAction::Action table[ResolvedEntity::LAST_ENTITY_NUMBER][ResolvedEntity::LAST_ENTITY_NUMBER];

  SubtypeRelations() {
    using namespace SubtypeRelationsAction;
    for (int i = 0; i < ResolvedEntity::LAST_ENTITY_NUMBER; ++i)
      for (int j = 0; j < ResolvedEntity::LAST_ENTITY_NUMBER; ++j)
        table[i][j] = l_undefined;


    for (int i = 0; i < ResolvedEntity::LAST_ENTITY_NUMBER; ++i) {
      table[i][Datatype_]        = l_sup_nextdef_datatype;
      table[i][DatatypeType_]    = l_sup_unref_datatype_type;

      symmetric((ScopedEntities)i, DatatypeNull_   , l_implicit);
      symmetric((ScopedEntities)i, DatatypeDefault_, l_implicit);

      table[i][Subtype_ ]     = l_sup_nextdef;
      table[Subtype_ ][i]     = l_sub_nextdef;
      table[Datatype_][i]     = l_sub_nextdef_datatype;
      table[DatatypeType_][i] = l_sub_unref_datatype_type;
    }

    table[NestedTable_][Object_] = l_sub_unref_collection; // BULK COLLECT INTO



    table[FieldOfTable_      ][FieldOfTable_      ] = l_sub_compare_by_entity_or_nextdef;
    table[FieldOfRecord_     ][FieldOfRecord_     ] = l_sub_compare_by_entity_or_nextdef;
    table[QueriedPseudoField_][QueriedPseudoField_] = l_sub_compare_by_entity_or_nextdef;
    table[Variable_          ][Variable_          ] = l_sub_compare_by_entity_or_nextdef;
    table[FunctionArgument_  ][FunctionArgument_  ] = l_sub_compare_by_entity_or_nextdef;
    symmetric(FieldOfTable_, QueriedPseudoField_, l_sub_compare_by_entity_or_nextdef);

    table[FundamentalDatatype_][FundamentalDatatype_] = l_compare_fundamental_datatypes;
    table[Object_][Object_] = l_sub_get_subtype;

    table[FundamentalDatatype_][FieldOfTable_       ] = l_sup_nextdef;
    table[FundamentalDatatype_][QueriedPseudoField_ ] = l_sup_nextdef;
    table[FundamentalDatatype_][FunctionArgument_   ] = l_sup_nextdef;
    table[FundamentalDatatype_][Variable_           ] = l_sup_nextdef;
    table[FundamentalDatatype_][SqlSelectedField_   ] = l_sup_get_datatype;

    table[Variable_          ][FundamentalDatatype_] = l_sub_get_datatype;
    table[FunctionArgument_  ][FundamentalDatatype_] = l_sub_get_datatype;
    table[FieldOfTable_      ][FundamentalDatatype_] = l_sub_get_datatype;
    table[QueriedPseudoField_][FundamentalDatatype_] = l_sub_get_datatype;

    table[QueriedPseudoField_][FieldOfTable_       ] = l_sub_get_datatype;

    table[FieldOfTable_      ][NestedTable_] = l_sub_get_datatype;
    table[FieldOfTable_      ][Variable_   ] = l_sub_get_datatype;
    table[FieldOfTable_      ][FunctionArgument_] = l_sub_get_datatype;

    table[Variable_          ][NestedTable_] = l_sub_get_datatype;
    table[NestedTable_       ][Variable_   ] = l_sup_get_datatype;

    table[FunctionArgument_  ][NestedTable_     ] = l_sub_get_datatype;
    table[NestedTable_       ][FunctionArgument_] = l_sup_get_datatype;

    table[QueryBlock_][RefCursor_] = l_eq_in_sql_oth_throw;
    table[QueryBlock_][FundamentalDatatype_] = l_sub_unwrap_subquery_in_sql;
    table[QueryBlock_][FieldOfTable_       ] = l_sub_unwrap_subquery_in_sql;

    symmetric(Table_     , DatatypeRowtype_, l_compare_by_fields);
    symmetric(Cursor_    , DatatypeRowtype_, l_compare_by_fields);
    symmetric(QueryBlock_, DatatypeRowtype_, l_compare_by_fields);
    symmetric(QueryBlock_, DatatypeRowtype_, l_compare_by_fields);
    symmetric(Record_    , DatatypeRowtype_, l_compare_by_fields);

    table[DatatypeRowtype_][DatatypeRowtype_] = l_compare_rowtypes;

    table[DatatypeRowtype_][FieldOfTable_      ] = l_sup_get_datatype;
    table[DatatypeRowtype_][QueriedPseudoField_] = l_sup_get_datatype;
    table[DatatypeRowtype_][FunctionArgument_  ] = l_sup_get_datatype;
    table[DatatypeRowtype_][Variable_          ] = l_sup_get_datatype;

    table[Variable_][DatatypeRowtype_          ] = l_sub_get_datatype;

    table[Cursor_    ][Cursor_] = l_compare_by_entity_and_fields;
    table[View_      ][View_  ] = l_compare_by_entity_and_fields;
    table[Table_     ][Table_ ] = l_compare_by_entity_and_fields;
    table[Varray_    ][Varray_] = l_compare_by_entity_and_datatypes;

    table[Cursor_    ][Table_ ] = l_compare_by_fields;
    table[QueryBlock_][Table_ ] = l_compare_by_fields;

    table[Record_][Record_] = l_compare_by_entity;

    table[NestedTable_][NestedTable_] = l_compare_by_entity;

    table[View_  ][FundamentalDatatype_] = l_neq;
    table[Cursor_][FundamentalDatatype_] = l_neq;

    symmetric(DatatypeRowtype_, FundamentalDatatype_, l_neq);
    symmetric(Object_         , FundamentalDatatype_, l_neq);
    symmetric(RefCursor_      , FundamentalDatatype_, l_neq);
    symmetric(RefCursor_      , Object_             , l_neq);
    symmetric(Table_          , FundamentalDatatype_, l_neq);
    symmetric(Record_         , FundamentalDatatype_, l_neq);
    symmetric(NestedTable_    , FundamentalDatatype_, l_neq);
    symmetric(AnydataObject_  , FundamentalDatatype_, l_neq);
    symmetric(AnydataObject_  , Object_             , l_neq);

    table[AnydataObject_][AnydataObject_] = l_eq;
    table[RefCursor_][RefCursor_] = l_eq;
  }
private:
  void symmetric(ScopedEntities sub, ScopedEntities sup, SubtypeRelationsAction::Action jumpAddr) {
    table[sub][sup] = jumpAddr;
    table[sup][sub] = jumpAddr;
  }
};

Sm::IsSubtypeValues Datatype::isSubtype(ResolvedEntity *superDef, bool plContext) const {
  return isSubtype(superDef, plContext, -1);
}

template <ResolvedEntity* (ResolvedEntity::*op)() const, bool generateException>
inline bool nextDef(ResolvedEntity *&p) {
  if (ResolvedEntity* newP = (p->*op)())
    p = newP;
  else {
    if (p)
      p->tryResoveConcreteDefinition();
    if (generateException) {
      if (!p || !(p = (p->*op)())) {
        cerr << "ERROR: nullptr in bool nextDef" << endl;
        return false;
      }
    }
    else
      return false;
  }
  return true;
}

inline CathegoriesOfDefinitions::ScopedEntities typeCat(const ResolvedEntity *p) {
  return p->ddlCathegoryWithDatatypeSpec();
}

template <ResolvedEntity* (ResolvedEntity::*op)() const>
inline void nextDefCast(ResolvedEntity *&p) { nextDef<op, true>(p); }

inline bool nextDefSubtype(ResolvedEntity *&p, ResolvedEntity::ScopedEntities &cat) {
  if (nextDef<&ResolvedEntity::getNextDefinition, false>(p)) {
    cat = typeCat(p);
    return true;
  }
  return false;
}

void unrefCollectionOracle(ResolvedEntity *&p, Datatype *&t, ResolvedEntity::ScopedEntities &cat) {
  if (Sm::Type::CollectionType *c = p->toSelfCollectionType()) {
    t = c->mappedType().object();
    p = t;
  }
  cat = typeCat(p);
}

inline bool nextUnrefDatatype(ResolvedEntity *&p, Datatype *&t, ResolvedEntity::ScopedEntities &cat) {
  t = p->toSelfDatatype();
  sAssert(!t);
  if (!nextDef<&ResolvedEntity::getNextDefinition, false>(p))
    return false;
  unrefCollectionOracle(p, t, cat);
  return true;
}


inline bool nextDefSubtype(ResolvedEntity *&lhs, ResolvedEntity *&rhs) {
  return nextDef<&ResolvedEntity::getNextDefinition, false>(lhs) &&
         nextDef<&ResolvedEntity::getNextDefinition, false>(rhs);
}

Sm::IsSubtypeValues Datatype::isSubtype(ResolvedEntity *argSuperDef, bool plContext, int inSqlCode) const {
  ResolvedEntity *sup = argSuperDef;
  if (!sup)
    return EXPLICIT;

  ResolvedEntity *sub = const_cast<Datatype*>(this);

  ResolvedEntity::ScopedEntities subCat = sub->ddlCathegoryWithDatatypeSpec();
  ResolvedEntity::ScopedEntities supCat = sup->ddlCathegoryWithDatatypeSpec();
  Datatype *subT = nullptr;
  Datatype *supT = nullptr;

  static const SubtypeRelations subtypeRelations;
  using namespace SubtypeRelationsAction;

  while (true)
    switch (subtypeRelations.table[subCat][supCat]) {
      case l_neq:
        return EXPLICIT;

      case l_implicit:
        return IMPLICIT_CAST_HIGH;

      case l_compare_by_entity:
        return rRef(sub) == rRef(sup) ? EXACTLY_EQUALLY : EXPLICIT;

      case l_eq_in_sql_oth_throw:
        if (inSqlCode <= 0)
          throw 999;
        // pass
      case l_eq:
        return EXACTLY_EQUALLY;

      case l_compare_fundamental_datatypes: {
        sAssert(!subT || !supT);
        Sm::GlobalDatatype::DatatypeCathegory subFCat = fundCat(subT, sub->toSelfFundamentalDatatype());
        Sm::GlobalDatatype::DatatypeCathegory supFCat = fundCat(supT, sup->toSelfFundamentalDatatype());
        IsSubtypeCathegory cat = Sm::GlobalDatatype::HasImplicitConversion::hasImplicitConversion(subFCat, supFCat, plContext);
        switch (cat) {
          case NEED_CHAR_COMPARSION_BY_LENGTH:
            if ((subT->precision == supT->precision) || (subT->hasMaxCharLen() && supT->hasMaxCharLen()))
              return EXACTLY_EQUALLY;
            return IMPLICIT_CAST_HIGH;
          case NEED_NUMBER_COMPARSION_BY_LENGTH:
            if (((subT->precision == supT->precision) || (subT->isMaxNumberType() && supT->isMaxNumberType())) &&
                ((subT->scale == subT->scale) || (subT->scaleIsEmpty() && supT->scaleIsEmpty()))
               )
              return EXACTLY_EQUALLY;
            return IMPLICIT_CAST_HIGH;
          default:
            return cat;
        }
        return cat;
      }

      // сравнение по subtype
      case l_sub_get_subtype:
        return sub->isSubtype(sup, plContext);

      // Сравнение по полям:

      case l_compare_rowtypes:
        if (rRef(sub) == rRef(sup)) // по сущности
          return EXACTLY_EQUALLY;
        if (!nextDefSubtype(sub, sup)) // разворачивание sub и sup одновременно
          return EXPLICIT;
        // pass
      case l_compare_by_entity_and_fields:
        if (rRef(sub) == rRef(sup)) // снова сравнение по сущности
          return EXACTLY_EQUALLY;
        // pass
      case l_compare_by_fields:
        if (inSqlCode < 0)
          throw 999;
        // сравнение по полям
        return sub->eqByFields(sup, plContext, inSqlCode) ? IMPLICIT_CAST_BY_FIELDS : EXPLICIT;

      // случаи разворачивания

      case l_compare_by_entity_and_datatypes: {
        if (rRef(sub) == rRef(sup))
          return EXACTLY_EQUALLY;
        sub = subT = sub->getUnforeignDatatype();
        sup = supT = sup->getUnforeignDatatype();
        subCat = typeCat(sub);
        supCat = typeCat(sup);
        continue;
      }

      case l_sub_unwrap_subquery_in_sql: {
        ResolvedEntity *oldSub = sub;
        if (inSqlCode <= 0) {
          cout << "error: compare query block with datatype of field or fundamental in PL/SQL code ";
          if (sub)
            cout << sub->getLLoc().locText();
          cout << endl;
          return EXPLICIT;
        }
        if (!(sub = SubqueryUnwrapper::unwrap(sub)))
          throw 999;
        if (oldSub == sub || oldSub == sub->getNextDefinition()) {
          cout << "error: isSubtype - unresolved subquery fields" << endl;
          return EXPLICIT;
        }
        subCat = typeCat(sub);
      }
        continue;

      case l_sup_nextdef_datatype:
        sAssert(!(supT = sup->toSelfDatatype()));
        if (!nextDefSubtype(sup, supCat))
          return EXPLICIT;
        continue;

      case l_sup_nextdef:
        if (!nextDefSubtype(sup, supCat))
          return EXPLICIT;
        continue;


      case l_sub_nextdef_datatype:
        sAssert(!(subT = sub->toSelfDatatype()));
        if (!nextDefSubtype(sub, subCat))
          return EXPLICIT;
        continue;

      case l_sub_compare_by_entity_or_nextdef:
        if (rRef(sub) == rRef(sup))
          return EXACTLY_EQUALLY;
        // pass
      case l_sub_nextdef:
        if (!nextDefSubtype(sub, subCat))
          return EXPLICIT;
        continue;

      case l_sub_get_datatype:
        sub = subT = sub->getUnforeignDatatype();
        subCat = typeCat(sub);
        continue;

      case l_sup_get_datatype:
        sup    = supT = sup->getUnforeignDatatype();
        supCat = typeCat(sup);
        continue;

      case l_sup_unref_datatype_type:
        if (!nextUnrefDatatype(sup, supT, supCat))
          return EXPLICIT;
        continue;

      case l_sub_unref_datatype_type:
        if (!nextUnrefDatatype(sub, subT, subCat))
          return EXPLICIT;
        continue;

      case l_sub_unref_collection:
        unrefCollectionOracle(sub, subT, subCat);
        continue;

      case l_undefined: {
        cout << "error: undefined isSubtype case: subCat=" << Sm::toString(subCat) << ", supCat=" << Sm::toString(supCat) << endl;
        return EXPLICIT;
      }
    }
  return EXPLICIT;
}


/**
 * @brief greatherByLen
 * @param lhs
 * @param rhs
 * @return
 *    CAST_NUM_GT_PREC_LT_SCALE  precision(lhs) > precision(rhs) and scale(lhs) < scale(rhs)
 *    CAST_NUM_BY_LENGTH_GT         lhs > rhs
 *    CAST_EQUALLY                  lhs = rhs
 *    CAST_NUM_BY_LENGTH_LT         lhs < rhs
 *    CAST_NUM_LT_PREC_GT_SCALE  precision(lhs) < precision(rhs) and scale(lhs) > scale(rhs)
 */
unsigned int greatherByLenNum(Datatype *lhs, Datatype *rhs) {
  if (!lhs->isNumberDatatype() || !rhs->isNumberDatatype())
    throw 999; // неправильно выведены типы для этого места (они должны совпадать по категориям)
  /* (0,2) < (); (0,2) < (0, 3) */
  if (lhs->isMaxNumberType()) {     // lhs = (),(>=30,>=10)
    if (rhs->isMaxNumberType())
      return CAST_EQUALLY;          // (),(>=30,>=10);                   (),(>=30,>=10)                   => lhs = rhs
    else
      return CAST_NUM_BY_LENGTH_GT; // (),(>=30,>=10);                   (<30,<10),(<30,>=10),(>=30,<10)  => lhs > rhs
  }
  else if (rhs->isMaxNumberType())  // (<30,<10),(<30,>=10),(>=30,<10);  (),(>=30,>=10)                   => lhs < rhs
    return CAST_NUM_BY_LENGTH_LT;
  else { // (<30,<10),(<30,>=10),(>=30,<10);  (<30,<10),(<30,>=10),(>=30,<10)
    unsigned int lhsPrec  = lhs->getMaxNumberPrecision();
    unsigned int lhsScale = lhs->getMaxNumberScale();

    unsigned int rhsPrec  = rhs->getMaxNumberPrecision();
    unsigned int rhsScale = rhs->getMaxNumberScale();

    static const unsigned int GT_GT = CAST_NUM_BY_LENGTH_GT | CAST_BY_PREC_LHS_GT_RHS | CAST_BY_SCALE_LHS_GT_RHS;
    static const unsigned int EQ_GT = CAST_NUM_BY_LENGTH_GT | CAST_BY_SCALE_LHS_GT_RHS;
    static const unsigned int GT_EQ = CAST_NUM_BY_LENGTH_GT | CAST_BY_PREC_LHS_GT_RHS ;
    static const unsigned int LT_LT = CAST_NUM_BY_LENGTH_LT | CAST_BY_PREC_LHS_LT_RHS | CAST_BY_SCALE_LHS_LT_RHS;
    static const unsigned int LT_EQ = CAST_NUM_BY_LENGTH_LT | CAST_BY_PREC_LHS_LT_RHS;
    static const unsigned int EQ_LT = CAST_NUM_BY_LENGTH_LT | CAST_BY_SCALE_LHS_LT_RHS;
    static const unsigned int EQ_EQ = CAST_EQUALLY;

    static const unsigned int GT_LT = CAST_NUM_GT_PREC_LT_SCALE;
    static const unsigned int LT_GT = CAST_NUM_LT_PREC_GT_SCALE;

    static const unsigned int precendence[3][3] =
                    //                           by scale
                    // by prec        lhs < rhs lhs = rhs lhs > rhs
                    /* lhs < rhs */ {{   LT_LT ,   LT_EQ ,    LT_GT},
                    /* lhs = rhs */  {   EQ_LT ,   EQ_EQ ,    EQ_GT},
                    /* lhs > rhs */  {   GT_LT ,   GT_EQ ,    GT_GT}};

    static const auto precendencePos = [](int a, int b) -> int { return (a < b) ? 0 : ((a == b) ? 1 : 2); };
    return precendence[precendencePos(lhsPrec, rhsPrec)][precendencePos(lhsScale, rhsScale)];
  }
}


unsigned int greatherByLenChar(Datatype *lhs, Datatype *rhs) {
  if ((!lhs->isCharVarchar() || !rhs->isCharVarchar()) &&
      (!lhs->isNCharVarchar() || !rhs->isNCharVarchar()))
    throw 999;

  // У Char и Varchar нет scale;
  if (lhs->hasMaxCharLen()) {
    if (rhs->hasMaxCharLen())       // ([,<scale>])   ([,<scale>])
      return CAST_EQUALLY; // lhs == rhs;
    else
      return CAST_CHAR_BY_LENGTH_GT; // lhs > rhs by lenght
  }
  else if (rhs->hasMaxCharLen()) /* lhs < maxLen, rhs == maxLen */
    return CAST_CHAR_BY_LENGTH_LT; // lhs < rhs by length
  if (lhs->precision > rhs->precision)
    return CAST_CHAR_BY_LENGTH_GT;
  else if (lhs->precision == rhs->precision)
    return CAST_EQUALLY;
  else
    return CAST_CHAR_BY_LENGTH_LT;
}

unsigned int Datatype::getHsBase(Sm::GlobalDatatype::DatatypeCathegory l, Sm::GlobalDatatype::DatatypeCathegory r)
{
  return ((unsigned int)(l)  < (unsigned int)(r) ? CAST_GET_RHS_BASE :
         ((unsigned int)(l) == (unsigned int)(r) ? CAST_EQUALLY_BY_BASE :
                                                   CAST_GET_LHS_BASE));
}

Sm::GlobalDatatype::DatatypeCathegory Datatype::fundCat(Datatype* t, Sm::GlobalDatatype::FundamentalDatatype *unref)
{
  if (t->translatedToSmallInt())
    return Sm::GlobalDatatype::_SMALLINT_;
  if (t->isInt())
    return Sm::GlobalDatatype::_INT_;
  if (t->isBigint())
    return Sm::GlobalDatatype::_BIGINT_;
  else
    return unref->converters->typeCathegory;
}

void Datatype::unrefDatatype(Datatype *&unrefT1, Datatype *&unrefT0, ResolvedEntity *&unrefFdt, std::vector<Datatype*> &tVector)
{
  tVector.push_back(unrefT0);
  while (true) {
    if (ResolvedEntity *def = unrefFdt->getNextDefinition()) {
      unrefFdt = def;

      if (table::FieldDefinition *f = def->toSelfFieldDefinition()) {
        def = f->getUnforeignDatatype();
      }

      if (Datatype *d = def->toSelfDatatype()) {
        if (unrefT1 && (unrefT1->isRowTypeOf() || unrefT1->scaleOrPrecIsSet()))
          throw 999;
        unrefT1 = unrefT0;
        unrefT0 = d;
        tVector.push_back(d);
      }
      else if (Subtype *t = def->toSelfSubtype()) {
        if (t->constraint())
          throw 999;
      }
      else
        return;
    }
    else {
      cout << "error: Datatype::unrefDatatype: unresolved datatype " << unrefFdt->getLLoc() << endl;
      unrefFdt = 0;
      return;
    }
  }
}

Datatype* Datatype::selectDatatype(Datatype *t1, Datatype *t0) {
  if (t1 && t1->scaleOrPrecIsSet()) {
    if (!t0->scaleOrPrecIsSet())
      return t1;
    else if (t1->scale == t0->scale && t1->precision == t0->precision)
      return t0;
    else
      throw 999;

  }
  return t0;
}


void unrefCollection(ResolvedEntity *&p) {
  if (Sm::Type::CollectionType *c = p->toSelfCollectionType())
    p = c->mappedType().object();
}



namespace get_cast_cathegory_action {
  enum Action {
    EQUALLY,
    EXPLICIT_ALL,
    IMPLICIT_LHS_BASE,
    IMPLICIT_RHS_BASE,
    LHS_RHS_UNREF_DATATYPE,
    LHS_UNREF_DATATYPE,
    RHS_UNREF_DATATYPE,
    LHS_RHS_GET_DATATYPE,
    LHS_GET_DATATYPE,
    RHS_GET_DATATYPE,
    LHS_UNREF_DATATYPE_TYPE,
    RHS_UNREF_DATATYPE_TYPE,
    COMPARE_FUNDAMENTAL_DATATYPES,
    COMPARE_ROWTYPES,
    COMPARE_OBJECTS,
    COMPARE_BY_ENTITIES_OR_FIELDS,
    COMPARE_BY_ENTITIES_OR_DATATYPES,
    COMPARE_BY_FIELDS,

    CHECK_BULK_COLLECT_UNREF_LHS_COLLECTION,
    CHECK_BULK_COLLECT_UNREF_LHS_RECODRD,
    UNDEFINED
  };
}

class CastCathegoryTable : public CathegoriesOfDefinitions {
public:
  get_cast_cathegory_action::Action table[ResolvedEntity::LAST_ENTITY_NUMBER][ResolvedEntity::LAST_ENTITY_NUMBER];

  CastCathegoryTable() {
    using namespace get_cast_cathegory_action;

    for (int i = 0; i < LAST_ENTITY_NUMBER; ++i)
      for (int j = 0; j < LAST_ENTITY_NUMBER; ++j)
        table[i][j] = UNDEFINED;


    for (int i = 0; i < LAST_ENTITY_NUMBER; ++i) {
      symmetric(AnydataObject_, (ScopedEntities)i, EXPLICIT_ALL);
      symmetric(ReturnInto_, (ScopedEntities)i, EXPLICIT_ALL);
      table[DatatypeType_][i] = LHS_UNREF_DATATYPE_TYPE;
      table[i][DatatypeType_] = RHS_UNREF_DATATYPE_TYPE;
      table[i][DatatypeNull_] = IMPLICIT_LHS_BASE;
      table[DatatypeNull_][i] = IMPLICIT_RHS_BASE;
    }
    table[Datatype_][FundamentalDatatype_] = LHS_UNREF_DATATYPE;
    table[Datatype_][Object_]              = LHS_UNREF_DATATYPE;

    table[RefCursor_][RefCursor_]          = EQUALLY;

    table[DatatypeNull_][DatatypeNull_] = EQUALLY;
    table[Datatype_][Datatype_] = LHS_RHS_UNREF_DATATYPE;

    table[Record_][FundamentalDatatype_] = CHECK_BULK_COLLECT_UNREF_LHS_RECODRD;
    table[FundamentalDatatype_][Record_] = EXPLICIT_ALL;

    symmetric(Datatype_, DatatypeRowtype_, COMPARE_BY_FIELDS);
    symmetric(QueryBlock_, Record_, COMPARE_BY_FIELDS);
    symmetric(QueryBlock_, FundamentalDatatype_, EXPLICIT_ALL);
    symmetric(FieldOfTable_, DatatypeRowtype_, EXPLICIT_ALL);
    table[QueryBlock_][QueryBlock_] = COMPARE_BY_FIELDS;

    table[FundamentalDatatype_][FundamentalDatatype_] = COMPARE_FUNDAMENTAL_DATATYPES;
    table[AnydataObject_][AnydataObject_] = EQUALLY;

    table[DatatypeRowtype_][DatatypeRowtype_] = COMPARE_ROWTYPES;
    table[NestedTable_][NestedTable_] = COMPARE_BY_ENTITIES_OR_FIELDS;
    table[Record_][Record_] = COMPARE_BY_ENTITIES_OR_FIELDS;
    symmetric(NestedTable_, Record_, EXPLICIT_ALL);
    symmetric(NestedTable_, QueryBlock_, EXPLICIT_ALL);

    table[Varray_][Varray_] = COMPARE_BY_ENTITIES_OR_DATATYPES;

    table[Object_][Object_] = COMPARE_OBJECTS;
    table[NestedTable_][FundamentalDatatype_] = CHECK_BULK_COLLECT_UNREF_LHS_COLLECTION;
    table[NestedTable_][Object_] = CHECK_BULK_COLLECT_UNREF_LHS_COLLECTION;
    table[FundamentalDatatype_][NestedTable_] = EXPLICIT_ALL;
    table[FundamentalDatatype_][Datatype_] = RHS_UNREF_DATATYPE;

    table[QueryBlock_][Variable_] = RHS_GET_DATATYPE;
    table[QueryBlock_][Datatype_] = RHS_UNREF_DATATYPE;

    symmetric(FundamentalDatatype_, Object_, EXPLICIT_ALL);

    initializer_list<ScopedEntities> getDatatypeEntities   = { FieldOfRecord_, FieldOfTable_, Variable_, FunctionArgument_, QueriedPseudoField_, SqlSelectedField_ };
    initializer_list<ScopedEntities> getDatatypeStaticPart = { Datatype_, FundamentalDatatype_ };

    for (ScopedEntities i : getDatatypeEntities) {
      for (ScopedEntities j : getDatatypeStaticPart) {
        table[i][j] = LHS_GET_DATATYPE;
        table[j][i] = RHS_GET_DATATYPE;
      }
      table[i][i] = LHS_RHS_GET_DATATYPE;
    }
  }
private:
  void symmetric(ScopedEntities sub, ScopedEntities sup, get_cast_cathegory_action::Action jumpAddr) {
    table[sub][sup] = jumpAddr;
    table[sup][sub] = jumpAddr;
  }

};



/**
 * @brief Datatype::getCastCathegory
 * @return
 *  @see CastCathegory and Sm::GlobalDatatype::HasImplicitConversion::greatherTable
 */
CastCathegory Datatype::getCastCathegory(Datatype *oth, bool plContext, bool inSqlCode) {
  if (!oth) {
    cerr << "ERROR: oth is empty in Datatype::getCastCathegory : " << this->getLLoc().toString() << endl;
    return CastCathegory();
  }

  using namespace GlobalDatatype;
  ResolvedEntity *lhs = this;
  ResolvedEntity *rhs = oth;
  ScopedEntities lhsCat = lhs->ddlCathegoryWithDatatypeSpec();
  ScopedEntities rhsCat = rhs->ddlCathegoryWithDatatypeSpec();
  Datatype *lhsT = nullptr;
  Datatype *rhsT = nullptr;

  static const CastCathegoryTable table;
  using namespace get_cast_cathegory_action;

  while (true) {
    switch (table.table[lhsCat][rhsCat]) {
      case LHS_RHS_UNREF_DATATYPE: {
        vector<Datatype*> rhsVector;
        Datatype *rhs1 = 0, *rhs0 = rhs->toSelfDatatype();
        unrefDatatype(rhs1, rhs0, rhs, rhsVector);
        rhsT = selectDatatype(rhs1, rhs0);
        if (!rhs) {
          cout << "error: Datatype::getCastCathegory: unresolved rhs datatype " << endl;
          return CAST_EXPLICIT_ALL;
        }
        rhsCat = typeCat(rhs);
      }
      // pass;
      case LHS_UNREF_DATATYPE: {
        vector<Datatype*> lhsVector;
        Datatype *lhs1 = 0, *lhs0 = lhs->toSelfDatatype();
        unrefDatatype(lhs1, lhs0, lhs, lhsVector);
        lhsT = selectDatatype(lhs1, lhs0);
        if (!lhs) {
          cout << "error: Datatype::getCastCathegory: unresolved lhs datatype " << endl;
          return CAST_EXPLICIT_ALL;
        }
        lhsCat = typeCat(lhs);
        continue;
      }
      case RHS_UNREF_DATATYPE: {
        vector<Datatype*> rhsVector;
        Datatype *rhs1 = 0, *rhs0 = rhs->toSelfDatatype();
        unrefDatatype(rhs1, rhs0, rhs, rhsVector);
        rhsT = selectDatatype(rhs1, rhs0);
        rhsCat = typeCat(rhs);
        continue;
      }
      case LHS_RHS_GET_DATATYPE:
        lhsT = lhs->getUnforeignDatatype();
        rhsT = rhs->getUnforeignDatatype();
        lhs = lhsT;
        rhs = rhsT;
        lhsCat = typeCat(lhs);
        rhsCat = typeCat(rhs);
        continue;
      case COMPARE_ROWTYPES: {
        lhsT = lhs->toSelfDatatype();
        rhsT = rhs->toSelfDatatype();
        nextDefCast<&ResolvedEntity::getNextDefinition>(lhs);
        nextDefCast<&ResolvedEntity::getNextDefinition>(rhs);
        if (rRef(lhs) == rRef(rhs))
          return CAST_EQUALLY;
        IsSubtypeValues v = lhsT->isSubtype(rhsT, plContext, inSqlCode);
        if (v.val == 2)
          return CAST_EQUALLY;
        else if (v.val == 6)
          return CAST_IMPLICIT;
        else if (v.val == 1) {
          cout << "error: Datatype::getCastCathegory: val == 1 unimplemented for COMPARE_ROWTYPES" << endl;
          return CAST_EXPLICIT_ALL;
        }
        else if (v.val < 0)
          throw 999;

        else {
          lhs = lhs->getNextNondatatypeDefinition();
          rhs = rhs->getNextNondatatypeDefinition();
          if (lhs->isRefCursor() || rhs->isRefCursor())
            return CAST_EQUALLY;
        }
        return CAST_EXPLICIT_ALL;
      }
      case COMPARE_OBJECTS: {
        if (rRef(lhs) == rRef(rhs))
          return CAST_EQUALLY;
        if (lhs->eqByFields(rhs, plContext, inSqlCode))
          return CAST_EQUALLY;
        int val = lhs->toSelfObject()->isSubtype(rhs, plContext);
        if (val == 2)
          return CAST_EQUALLY;
        else if (val != 0)
          throw 999;
        return CAST_EXPLICIT_ALL;
      }
      case CHECK_BULK_COLLECT_UNREF_LHS_RECODRD: {
        if (plContext && inSqlCode) {
          Sm::Type::Record *r = lhs->toSelfRecord();
          if (r->fields->size() > 1)
            throw 999;
          lhs = r->fields->front();
          lhsCat = typeCat(lhs);
          continue;
        }
        else if (plContext)
          return CAST_EXPLICIT_ALL;
        else
          throw 999;
      }
      case CHECK_BULK_COLLECT_UNREF_LHS_COLLECTION: {
        if (plContext && inSqlCode) {
          unrefCollection(lhs);
          lhsT = lhs->getUnforeignDatatype();
          lhs = lhsT;
          lhsCat = typeCat(lhs);
          continue;
        }
        else if (plContext)
          return CAST_EXPLICIT_ALL;
        else
          throw 999;
      }
      case COMPARE_BY_ENTITIES_OR_DATATYPES:
        if (rRef(lhs) == rRef(rhs))
          return CAST_EQUALLY;
        lhs = lhs->getDatatype();
        rhs = rhs->getDatatype();
        lhsCat = typeCat(lhs);
        rhsCat = typeCat(rhs);
        continue;
      case COMPARE_BY_ENTITIES_OR_FIELDS:
        if (rRef(lhs) == rRef(rhs))
          return CAST_EQUALLY;
        // pass
      case COMPARE_BY_FIELDS:
        return lhs->eqByFields(rhs, plContext, inSqlCode) ? CAST_EQUALLY : CAST_EXPLICIT_ALL;
      case EQUALLY:
        return CAST_EQUALLY;
      case EXPLICIT_ALL:
        return CAST_EXPLICIT_ALL;
      case IMPLICIT_LHS_BASE:
        return CAST_IMPLICIT | CAST_GET_LHS_BASE;
      case IMPLICIT_RHS_BASE:
        return CAST_IMPLICIT | CAST_GET_RHS_BASE;
      case LHS_UNREF_DATATYPE_TYPE:
        lhsT = lhs->toSelfDatatype();
        nextDefCast<&ResolvedEntity::getNextDefinition>(lhs);
        if (!lhs) {
          cout << "error: Datatype::getCastCathegory: unresolved lhs datatype " << endl;
          return CAST_EXPLICIT_ALL;
        }
        unrefCollection(lhs);
        lhsCat = typeCat(lhs);
        continue;
      case RHS_UNREF_DATATYPE_TYPE:
        rhsT = rhs->toSelfDatatype();
        nextDefCast<&ResolvedEntity::getNextDefinition>(rhs);
        if (!rhs) {
          cout << "error: unresolved rhs in getCastCathegory" << endl;
          return CAST_EXPLICIT_ALL;
        }
        unrefCollection(rhs);
        rhsT = rhs->getUnforeignDatatype();
        rhs = rhsT;
        rhsCat = typeCat(rhs);
        continue;
      case LHS_GET_DATATYPE:
        lhsT = lhs->getUnforeignDatatype();
        lhs = lhsT;
        lhsCat = typeCat(lhs);
        continue;
      case RHS_GET_DATATYPE:
        rhsT = rhs->getUnforeignDatatype();
        rhs  = rhsT;
        rhsCat = typeCat(rhs);
        continue;
      case COMPARE_FUNDAMENTAL_DATATYPES: {
        DatatypeCathegory lhsFCat = fundCat(lhsT, lhs->toSelfFundamentalDatatype());
        DatatypeCathegory rhsFCat = fundCat(rhsT, rhs->toSelfFundamentalDatatype());
        int castCathegory = HasImplicitConversion::greather(lhsFCat, rhsFCat);
        switch (castCathegory) {
          case CAST_INTERNAL_TYPES_CHR_NEED_CMP_BY_LEN:
            return greatherByLenChar(lhsT, rhsT) | getHsBase(lhsFCat, rhsFCat);
          case CAST_INTERNAL_TYPES_NUM_NEED_CMP_BY_LEN:
            return greatherByLenNum(lhsT, rhsT);
          default:
            return castCathegory | getHsBase(lhsFCat, rhsFCat);
        }
        throw 999;
        break;
      }
      case UNDEFINED: {
        throw 999;
        break;
      }
    }
  }
  return CAST_EXPLICIT_ALL;
}



CastCathegory Datatype::castCathegory(Datatype *lhs, Datatype *rhs, bool plContext) {
  return (!lhs || !rhs) ? CastCathegory(CAST_DATATYPES_UNRESOLVED_ERROR) : lhs->getCastCathegory(rhs, plContext);
}



Datatype* Datatype::getMaximal(Datatype *lhs, Datatype *rhs, bool pl) {
  if (!lhs)
    return rhs;
  if (!rhs)
    return lhs;
  lhs = SubqueryUnwrapper::unwrap(lhs);
  rhs = SubqueryUnwrapper::unwrap(rhs);
  CastCathegory cat = lhs->getCastCathegory(rhs, pl);
  CastCathegory::LengthCathegory lengthCathegory = cat.lengthCathegory();
  CastCathegory::BasePriority    base = cat.base();
  switch (lengthCathegory) {
    case CastCathegory::EMPTY: // если это не NUMBER и не символьный тип
      if (cat.implicit()) // но есть неявное преобразование либо типы эквивалентны
        return cat.rhsBasePriority() ? rhs : lhs; // вернуть соответственно приоритетному фундаментальному типу
      break;
    case CastCathegory::VARCHAR: {
      Datatype *hs = 0;
      uint16_t prec = 0;
      switch (base) {
        case CastCathegory::EQ_HS_BASE:
          return cat.lengthLhsGtRhs() ? lhs : rhs;
        case CastCathegory::LHS_BASE:
          if (cat.lengthLhsGtRhs())
            return lhs;
          hs   = lhs;
          prec = rhs->precision;
          break;
        case CastCathegory::RHS_BASE:
          if (cat.lengthLhsLtRhs())
            return rhs;
          hs   = rhs;
          prec = rhs->precision;
          break;
      }
      if (hs->isVarcharDatatype())
        return Datatype::mkVarchar2(prec);
      else if (hs->isCharDatatype())
        return Datatype::mkChar(prec);
      else
        throw 999;
      break;
    }
    case CastCathegory::DECIMAL: {
      CastCathegory::Relation lenRel = cat.relationLength();
      switch (lenRel) {
        case CastCathegory::EQ:
          return lhs;
        case CastCathegory::ERROR_RELATION:
          if (cat.numGtPrecLtScale())
            return Datatype::mkNumber(lhs->precision, rhs->scale);
          else if (cat.numLtPrecGtScale())
            return Datatype::mkNumber(rhs->precision, lhs->scale);
          else
            throw 999; // недопустимые флаги.
        case CastCathegory::GT:
          return lhs;
        case CastCathegory::LT:
          return rhs;
      }
    }
    case CastCathegory::ERROR_CATHEGORY:
      throw 999;
  }
  // lengthCathegory == empty, проверены случаи для NUMBER, символьных типов, для остальных имеющих неявное преобразование.
  // => неявного преобразования нет.
  return 0;
}


bool Datatype::getMaximal(Ptr<Sm::Datatype> &dst, Datatype *lhs, Datatype *rhs, bool pl) {
  if (Datatype *res = Datatype::getMaximal(lhs, rhs, pl)) {
    if (dst.object() != res)
      dst = res;
    return true;
  }
  return false;
}



}

namespace Sm { // getDatatype

Ptr<Datatype> getNestedTableKey(const IdEntitySmart &funname) {
  if (ResolvedEntity *def = funname.definition())
    if (Ptr<Datatype> t = def->keyType())
      return t;
  return Datatype::mkInteger();
}


bool Sm::Datatype::semanticResolve() const {
  if (SemanticTree *node = tid->entity()->semanticNode())
    node->semanticResolve();
  return semanticResolveBase();
}

bool Sm::RefAbstract::semanticResolve() const {
  Id *ent = refEntity();
  if (SemanticTree *node = ent->semanticNode())
    node->semanticResolve();
  if (ResolvedEntity *def = ent->definition())
    def->semanticResolve();
  return semanticResolveBase();
}

Ptr<Datatype> Sm::RefAbstract::getDatatype() const {
  if (ResolvedEntity *def = refDefinition()) {
    if (refSize() == 1 && refEntity()->squoted())
      return Datatype::mkVarchar2(refEntity()->length());
    else if (Ptr<Datatype> t = def->getDatatype()) {
      if (refEntity()->callArglist && def->isField())
        return isSqlCode() ? StructureSubqueryUnwrapper::unwrap(t) : SubqueryUnwrapper::unwrap(t);
      return t;
    }
  }
  // Для неотрезовленных закавыченных элементов, возвращать string
  else if (refSize() == 1 && (refEntity()->quoted() || refEntity()->squoted()))
    return Datatype::mkVarchar2(refEntity()->length());
  return 0;
}

Sm::GlobalDatatype::SysTypeInfo::SysTypeInfo(
    DmpType::DmpCathegory   _fieldTypeID,
    char                    _overloadConversionId,
    DatatypeCathegory       _typeCathegory,
    DatatypeCathegory       _parentTypeCathegory,
    SetType                 _setType,
    GetLengthPrecDefinition _getBrLenPrec,
    GetValueText            _getValueText,
    Ora2CallConverter       _ora2CallConverter,
    std::string             _oracleName,
    std::string             _linterProcName,
    std::string             _linterSqlName)
  : fieldTypeID         (_fieldTypeID         ),
    overloadConversionId(_overloadConversionId),
    typeCathegory       (_typeCathegory       ),
    parentTypeCathegory (_parentTypeCathegory ),
    setType             (_setType             ),
    getBrLenPrec        (_getBrLenPrec        ),
    getValueText        (_getValueText        ),
    ora2CallConverter   (_ora2CallConverter   ),
    oracleName          (_oracleName          ),
    linterProcName      (_linterProcName      ),
    linterSqlName       (_linterSqlName       ) {}


Ptr<Datatype> PriorExpr::getDatatype() const { return prior ? prior->getDatatype() : Ptr<Datatype>(); }

Ptr<Datatype> Case::getDatatype() const {
  if (!cases || !cases->size())
    return 0;
  Ptr<Datatype> t;
  SemanticTree *node = getSemanticNode();
  if (!node)
    throw 999;
  bool inSqlCode   = node->isSqlCode();
  bool isPlContext = node->isPlContext();
  static const auto updateMaximalDatatype = [](Ptr<Datatype> &dst, Datatype *t, bool inSqlCode, bool isPlContext) {
    if (!t || t->isEverything())
      return;

    if (inSqlCode)
      t = SubqueryUnwrapper::unwrap(t);
    Datatype::getMaximal(dst, dst, t, isPlContext);
  };

  for (Sm::List<CaseIfThen>::const_iterator it = cases->begin(); it != cases->end(); ++it)
    if ((*it)->action)
      updateMaximalDatatype(t, (*it)->action->getDatatype(), inSqlCode, isPlContext);

  if (elseClause) {
    if (t && elseClause->isFuncall(true))
      if (Id *id = elseClause->toSelfRefAbstract()->refEntity())
        if (id->toNormalizedString() == "TO_NUMBER" &&
            id->callArglist &&
            id->callArglist->size() > 0 &&
            (*id->callArglist->begin())->expr()->isNull())
          // Исключаем некорректное преобразование для TO_NUMBER(NULL)
          return t;
    updateMaximalDatatype(t, elseClause->getDatatype(), inSqlCode, isPlContext);
  }

  return t;
}

bool Case::getFieldRef(Ptr<Sm::Id> &field) {
  if (!cases || !cases->size() || !field)
    return false;
  for (Sm::List<CaseIfThen>::iterator it = cases->begin(); it != cases->end(); ++it)
    if ((*it)->action && (*it)->action->getFieldRef(field))
      return true;
  return false;
}

ResolvedEntity* Case::getNextDefinition() const {
  if (!cases || !cases->size())
    return 0;
  for (Sm::List<CaseIfThen>::const_iterator it = cases->begin(); it != cases->end(); ++it)
    if ((*it)->action)
      if (ResolvedEntity *p = (*it)->action->getNextDefinition())
        return p;
  return 0;
}


Ptr<Datatype> updateThisDatatype(Ptr<Datatype> &thisDatatype, Ptr<Sm::Id> entityName, const ResolvedEntity *def) {
  if (!thisDatatype) {
    if (!entityName->definition())
      entityName->definition(def);
      entityName->semanticNode(def->getSemanticNode());
      thisDatatype = new Datatype(entityName);
      thisDatatype->setSemanticNode(def->getSemanticNode());
  }
  return thisDatatype;
}

// Выполняется раскрутка полей и возврат динамической структуры полей в виде записей
Ptr<Datatype> Table            ::getDatatype() const { return getDatatypeWithSetFieldStruct(thisDatatype, name->entity()); }
Ptr<Datatype> View             ::getDatatype() const { return getDatatypeWithSetFieldStruct(thisDatatype, name->entity()); }
Ptr<Datatype> Type::Object     ::getDatatype() const { return updateThisDatatype(thisDatatype, name->entity(), this); }
Ptr<Datatype> Type::Record     ::getDatatype() const { return updateThisDatatype(thisDatatype, name, this); }
Ptr<Datatype> Subtype          ::getDatatype() const { return datatype; }

Ptr<Datatype> NumericInt      ::getDatatype() const { return Datatype::mkInteger(); }
Ptr<Datatype> NumericFloat    ::getDatatype() const { return Datatype::mkNumber (); }
Ptr<Datatype> NumericSimpleInt::getDatatype() const { return Datatype::mkInteger(); }


void FromTableDynamic::getDynamicFields(SqlExpr *dynamicTable, EntityFields &f) const {
  ResolvedEntity *def = dynamicTable;
  while (def)
    switch (def->ddlCathegory()) {
      case ResolvedEntity::NestedTable_:
      case ResolvedEntity::Varray_:
        def = def->toSelfCollectionType()->mappedType()->tidDef();
        break;
      case ResolvedEntity::SqlExprCast_:
        def = def->toSelfCastStatement()->getDatatype()->tidDef();
        break;
      case ResolvedEntity::RefExpr_:
        def = def->toSelfRefExpr()->getNextDefinition();
        break;
      case ResolvedEntity::Function_:
      case ResolvedEntity::Variable_:
        def = def->getDatatype();
        break;
      case ResolvedEntity::Datatype_:
        def = def->toSelfDatatype()->tidDef();
        break;
      case ResolvedEntity::Object_:
        def->getFields(f);
        return;
      case ResolvedEntity::FundamentalDatatype_: {
        // можно еще создавать COLUMN_VALUE
        if (!columnValue) {
          columnValue = new QueryPseudoField(new Id("COLUMN_VALUE"), const_cast<FromTableDynamic*>(this));
          columnValue->definition(def->getDatatype());
          columnValue->isColumnValue  = true;
        }
        f.push_back(columnValue->getName());
        return;
      }
      default:
        throw 999;
        break;
    }
}


void FromTableDynamic::initDynamicFields() const
{
  if (beginedFrom(1389187))
    cout << "";
  if (fields_.empty() && dynamicTable) {
    EntityFields f;
    getDynamicFields(dynamicTable, f);
    for (EntityFields::value_type &v : f) {
      Ptr<QueryPseudoField> qF = new QueryPseudoField(new Sm::Id(v->toString(), 0, v->quoted(), v->empty()), (FromTableDynamic*)this, v->definition());
      fields_.push_back(qF);
      fieldsMap[v->definition()->getDefinitionFirst()] = qF;
    }
  }
}

bool FromTableDynamic::getFields(EntityFields &fields) const {
  initDynamicFields();
  for (const DynamicFields::value_type &v : fields_)
    fields.push_back(v->getName());
  return !fields.empty();
}

bool FromSingle::getFields(EntityFields &f) const {
  return reference && reference->getFields(f);
}

Ptr<Datatype> FromSingle::getDatatype() const {
  if (!reference)
    return 0;
  if (!cachedDatatype) {
    Ptr<Id> name = new Id(*getName());
    name->definition((FromSingle*)this);
    cachedDatatype = new Datatype(name);
  }
  return cachedDatatype;
}

Ptr<Datatype> FromJoin::getDatatype() const {
  Ptr<Id> emptyName = new Id();
  emptyName->setIsEmpty();
  emptyName->definition((FromJoin*)this);
  return new Datatype(emptyName);
}

bool QueryBlock::getFieldRefFromRootQuery(Ptr<Id> &field) {
  Subquery* root = ownerSelectModel;
  if (root) {
    for (Subquery* it = root->ownerSelectModel; it; it = root->ownerSelectModel)
      root = it;
    return root->getFieldRef(field);
  }
  return false;
}

bool QueryBlock::getFieldRefByFromListFromRootQuery(Ptr<Id> &field, Sm::SemanticTree *reference) {
  Subquery* root = ownerSelectModel;
  if (root) {
    for (Subquery* it = root->ownerSelectModel; it; it = root->ownerSelectModel)
      root = it;
    return root->getFieldRefByFromList(field, reference);
  }
  return false;
}


Sm::SemanticTree* FromResolver::createFromListNode(Ptr<FromList> fromList) const {
  SemanticTree *fromNode = new SemanticTree(SCathegory::From, SemanticTree::NEW_LEVEL);
  fromNode->setIsList();
  CollectSNode(fromNode, fromList);
  return fromNode;
}

bool FromResolver::resolveAsTable(Sm::SemanticTree *reference, Ptr<Id> &field, SemanticTree *fromNode_)
{
  if (!field)
    throw 999; // return false;

  // как таблицы
  if ((field->semanticNode() &&
       field->semanticNode()->cathegory == SCathegory::FromTableReference) ||
      (reference &&
       (reference->refSize() > 1 ||
        reference->cathegory == Sm::SCathegory::Expr_Asterisk))) {
    if (fromNode_)
      if (Ptr<LevelResolvedNamespace> &fromNamespace = fromNode_->childNamespace) {
        LevelResolvedNamespace::const_iterator node = fromNamespace->findFieldNode(field);
        if (node != fromNamespace->end()) {
          VEntities *vEnt = 0;
          ResolvedEntity *def = 0;
          if ((vEnt = node->second.object()) && (def = vEnt->findFieldDefinition()) && !field->quoted()) {
            field->definition(def);
            return true;
          }
          else
            syntaxerContext.model->partitiallyResolvedNodes.insert(field.object());
        }
//        if (ResolvedEntity *def = fromNamespace->findFieldIdDef(field))
//          if (!field->quoted()) {
//            field->definition(def);
//            return true;
//          }
      }
    if (findInFactoringList(field))
      return true;
  }

  return false;
}

bool FromResolver::resolveAsFromItemField(Ptr<Id> &field, FromList *from)
{
  if (from)
    for (FromList::value_type &v : *from)
      if (v->getFieldRef(field))
        return true;
  return false;
}

bool FromResolver::resolveAsFromItemField(Ptr<Id> &field, FromSingleList *from)
{
  if (from)
    for (FromSingleList::value_type &v : *from)
      if (v->getFieldRef(field))
        return true;
  return false;
}


bool FromResolver::resolveAsFactoringItemField(Ptr<Id> &field, FactoringList *factoringList)
{
  if (factoringList)
    for (Ptr<FactoringItem> &it : *factoringList)
      if (it->subquery)
        if (it->subquery->getFieldRef(field))
          return true;
  return false;
}

bool FromResolver::checkToHierarhicalPseudoColumn(
    Ptr<Id> &field,
    HierarhicalClause *hierarhicalSpec,
    Ptr<Sm::QueryPseudoField> *levelPseudoField,
    ResolvedEntity *owner)
{
  static const HString levelStr = "LEVEL";
  if (hierarhicalSpec && field->hash() == levelStr.hash() && field->toNormalizedString() == levelStr) {
    if (levelPseudoField && !*levelPseudoField) {
      *levelPseudoField = new QueryPseudoField(new Id(string(levelStr)), owner);
      (*levelPseudoField)->definition(Datatype::mkInteger());
    }
    field->definition(levelPseudoField->object());
    return true;
  }
  return false;
}

bool FromResolver::resolveAsSquotedField(Ptr<Id> &field)
{
  if (field->quoted()) {
    field->definition(Datatype::mkVarchar2());
    return true;
  }
  return false;
}

bool QueryBlock::getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference) {
  // как поля таблиц
  return resolveAsTable(reference, field, fromNode_) ||
         resolveAsFromItemField        (field, from) ||
         resolveAsFactoringItemField   (field, factoringList) ||
         (tailSpec && checkToHierarhicalPseudoColumn(field, tailSpec->hierarhicalSpec, &levelPseudoField, this)) ||
         resolveAsSquotedField         (field);
}


bool updateFieldsDefinition(std::vector<Ptr<Id> >::iterator &it, Ptr<ResolvedEntity> def, std::vector<Ptr<Id> > &container) {
  if (it == container.end())
    return true;
  if (!(*it)->definition() || (*it)->definition()->isElementaryLiteral() || (*it)->definition()->isNullField())
    (*it)->definition(def);
  ++it;
  if (it == container.end())
    return true;
  return false;
}

bool updateFieldsDefinition(std::vector<Ptr<Id> >::iterator &it, std::vector<Ptr<Id> > &fields, std::vector<Ptr<Id> > &container)
{
  for (vector<Ptr<Id> >::iterator fit = fields.begin(); fit != fields.end(); ++fit)
    if (updateFieldsDefinition(it, (*fit)->definition(), container))
      return true;
  return false;
}

void QueryBlock::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
  if (!selectList)
    return;
  if (selectList->isAsterisk()) {
    std::vector<Ptr<Id> > fields;
    getFields(fields);
    if (updateFieldsDefinition(it, fields, container))
      return;
  }
  else {
    selectList->assignFieldReferenceByPosition(container, it);
  }
}

bool Subtype::getFields(EntityFields &fields) const { return datatype && datatype->getFields(fields); }

bool QueryBlock::getFields(EntityFields &fields) const {
  if (!selectList)
    return false;
  else if (selectList->isAsterisk()) {
    if (from)
      for (const Ptr<From> &it : *from)
        it->getFields(fields);
    if (factoringList)
      for (const Ptr<FactoringItem> &it : *factoringList)
        it->getFields(fields);
  }
  else
    return selectList->getFields(fields);
  return true;
}

bool QueryBlock::findInFactoringList(Ptr<Id> &currentPart) const {
  if (factoringNode_)
    if (Ptr<LevelResolvedNamespace> &factoringNamespace = factoringNode_->childNamespace)
      if (factoringNamespace->findField(currentPart))
        return true;
  return false;
}

void Sm::SelectList::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
  for (SelectList::SelectedFields::iterator sit = fields->begin(); sit != fields->end(); ++sit) {
    if (Ptr<Id> asterisk = (*sit)->toAsterisk()) {
      if (asterisk->definition()) {
        std::vector<Ptr<Id> > fields;
        asterisk->definition()->getFields(fields);
        if (updateFieldsDefinition(it, fields, container))
          return;
      }
    }
    else if ((*sit)->getName()) {
      if (updateFieldsDefinition(it, (*sit)->getName()->unresolvedDefinition(), container))
        return;
    }
    else
      ++it;
  }
}

bool Sm::SelectList::getFields(EntityFields &fields) const {
  for (SelectedFields::const_iterator it = this->fields->begin(); it != this->fields->end(); ++it) {
    if (Ptr<Id> asterisk = (*it)->toAsterisk()) {
      if (asterisk->definition())
        asterisk->definition()->getFields(fields);
    }
    else if (Ptr<Id> itName = (*it)->getName())   // не AsteriskExpr, но имеет имя
      fields.push_back(itName);
    else {
      itName = (*it)->getName();
      throw 999; // имя должно быть в любом случае, иначе не соблюдается целостность полей запроса
    }
  }
  return true;
}




bool QueryBlock::getFieldRef(Ptr<Id> &field) {
  if (!field)
    return false;

  if (Sm::checkToRowidPseudocolumn(field))
    return true;

  if (!selectList)
    return false;
  else if (selectList->isAsterisk())
    return getFieldRefByFromList(field, 0);
  else {
    // обходить selectList и обрабатывать selectField
    // TODO: оптимизировать
    if (selectedListNode_ && selectedListNode_->childNamespace)
      if (ResolvedEntity *def = selectedListNode_->childNamespace->findFieldIdDef(field)) {
        if (def->getAlias())
          if (VEntities *vEnt = def->vEntities())
            if (ResolvedEntity *def = vEnt->findFieldWithoutAlias()) {
              field->definition(def);
              return true;
            }
        if (field.object() == def->getName().object())
          return false;
        field->definition(def);
        return true;
      }
    for (List<Id>::const_iterator it = selectList->asterisksList.begin(); it != selectList->asterisksList.end(); ++it)
      if ((*it)->definition() && (*it)->definition()->getFieldRef(field))
        return true;
    if (tailSpec && checkToHierarhicalPseudoColumn(field, tailSpec->hierarhicalSpec, &levelPseudoField, this))
      return true;
  }
  return false;
}

bool FromTableReference::getFieldRefAsTableName(Ptr<Id> &field) {
  if (field && id && id->entity() && field->normalizedString() == id->entity()->normalizedString()) {
    field->definition((FromTableReference*)this);
    return true;
  }
  return false;
}


Ptr<Datatype> QueryBlock::getDatatype() const {
  if (!thisDatatype) {
    Ptr<Id> emptyName = new Id();
    emptyName->setIsEmpty();
    emptyName->definition((QueryBlock*)this);
    emptyName->semanticNode(getSemanticNode());
    thisDatatype = new Datatype(emptyName);
    thisDatatype->setSemanticNode(getSemanticNode());
  }
  return thisDatatype;
}

Ptr<Sm::Datatype> FactoringItem::getDatatype() const {
  if (!cachedDatatype)
    cachedDatatype = new Datatype(queryName);
  return cachedDatatype;
}

Ptr<Sm::Datatype> FromTableReference::getDatatype() const {
  if (id && id->entity() && id->entity()->definition())
    return id->entity()->definition()->getDatatype();
  return Ptr<Sm::Datatype>();
}

Ptr<Datatype> UnionQuery::getDatatype() const {
  if (lhs)
    return lhs->getDatatype();
  else if (rhs)
    return rhs->getDatatype();
  return 0;
}

}

namespace Sm { // getFields

bool Datatype::getFields(EntityFields &fields) const { return tid->getFields(fields); }

bool Table::getFields(EntityFields &fields) const {
  if (relationFields) { // генерация полей по реляционной таблице
    for (List<table::FieldDefinition>::const_iterator it = relationFields->begin(); it != relationFields->end(); ++it)
      fields.push_back((*it)->name);
    return true;
  }
  else if (objectName)
    return objectName->getFields(fields);
  else
    return false;
}


bool View::getFields(EntityFields &fields) const {
  if (aliasedFields.size()) {
    for (AliasedFields::const_iterator it = aliasedFields.begin(); it != aliasedFields.end(); ++it)
      fields.push_back((*it)->getName());
    return true;
  }
  else if (properties) {
    if (Sm::Id2 *n = properties->getName2())
      return n && n->getFields(fields);
    else
      return select && select->getFields(fields);
  }
  else
    return select && select->getFields(fields);
}


}
namespace Sm { // Type::collection_methods::CollectionMethod
namespace Type {
}
}

Ptr<ResolvedEntity> Sm::Subtype::tidDdl() const { return datatype ? datatype->tidDdl() : Ptr<ResolvedEntity>(); }


IdEntitySmart::IdEntitySmart(const IdEntitySmart &o, bool deepCopy)
{
  if (deepCopy) {
    for (IdEntitySmart::const_iterator it = o.begin(); it != o.end(); ++it) {
      if ((*it).valid())
        push_back(new Id(*(*it).object()));
      else
        push_back(NULL);
    }
  }
  else
    assign(o.begin(), o.end());
  __flags__.v = o.__flags__.v;
}

void IdEntitySmart::resolveByModelContext() {
  ResolvedEntity *def = 0;
  for (IdEntitySmart::reverse_iterator rit = rbegin(); rit != rend(); ++rit) {
    (*rit)->setSkipFunctionResolving();
    if (rit == rbegin())
      syntaxerContext.model->getFieldRef(*rit);
    else if (def)
      def->getFieldRef(*rit);
    else
      break;
    def = (*rit)->definition();
  }
}


IdEntitySmart::IdEntitySmart(std::initializer_list<std::string> strList) {
  reserve(strList.size());
  std::initializer_list<std::string>::const_iterator it = strList.end();
  if (it == strList.begin())
    return;
  do {
    --it;
    push_back(new Id(string(*it)));
  } while (it != strList.begin());
}

bool Sm::IdEntitySmart::isEqual(const IdEntitySmart &oth) const {
  IdEntitySmart::const_iterator it = this->begin();
  IdEntitySmart::const_iterator oIt = oth.begin();
  while (it != this->end() && oIt != oth.end()) {
    if ((*it)->normalizedString() != (*oIt)->normalizedString())
      return false;
    ++it;
    ++oIt;
  }
  return true;
}

bool Sm::IdEntitySmart::isResolved() const {
  Ptr<Id> e = entity();
  return e && e->definition();
}

ResolvedEntity* Sm::IdEntitySmart::definition() const {
  if (static_cast<const BaseType*>(this)->empty())
    return 0;
  return (*(this->begin()))->definition();
}

bool Sm::IdEntitySmart::isDynamicUsing() const {
  if (!syntaxerContext.translateReferences || (__flags__.v & FLAG_RESOLVED_ENTITY_IS_DYNAMIC_USING))
    return true;
  for (IdEntitySmart::const_reference id : *this) {
    if (id->definition() && id->definition()->isDynamicUsing())
      return true;
  }
  return false;
}

SemanticTree *Sm::IdEntitySmart::toSTreeRefBase(SCathegory::t t) const {
  return new SemanticTree(const_cast<IdEntitySmart*>(this), SemanticTree::REFERENCE, t);
}


CLoc Sm::IdEntitySmart::getLLoc() const {
  return majorEntity()->getLLoc();
}

Ptr<Id> Sm::IdEntitySmart::majorBaseEntity() const {
  if (empty())
    return Ptr<Id>();
  if (size() == 1)
    return *(this->begin());
  IdEntitySmart::const_iterator it = --(this->end());
  Id *itVal;
  ResolvedEntity *ent;
  while (!(itVal = *it) || itVal->empty() ||
         ((ent = itVal->unresolvedDefinition()) &&
          (ent->toSelfUserContext() || ent->toSelfPackage() ||
           (it != this->begin() && ent->toSelfFunction()) )
         )) {
    --it;
    if ((!itVal || itVal->empty()) && it == this->begin())
      return Ptr<Id>();
  }
  return *it;

}

Ptr<Id> Sm::IdEntitySmart::majorEntity() const {
  if (empty())
    return Ptr<Id>();
  if (size() == 1)
    return *(this->begin());
  IdEntitySmart::const_iterator it = --(this->end());
  while (!(*it) || (*it)->empty()) {
    --it;
    if ((!(*it) || (*it)->empty()) && it == this->begin())
      return Ptr<Id>();
  }
  return *it;
}

IdEntitySmart::iterator Sm::IdEntitySmart::majorEntityIter() {
  if (empty())
    return (this->end());
  IdEntitySmart::iterator it = --(this->end());
  while (!(*it) || (*it)->empty()) {
    --it;
    if ((!(*it) || (*it)->empty()) && it == this->begin())
      return this->end();
  }
  return it;
}

Ptr<Sm::Id> Sm::IdEntitySmart::majorObjectRef(ResolvedEntity **ppObjType) const {
  for (auto it = begin(); it != end(); ++it) {
    ResolvedEntity *ent = (*it)->definition();
    if (!ent)
      continue;
    if (Ptr<Datatype> datatype = ent->getDatatype()) {
      datatype = datatype->getFinalType();
      if (datatype->isObjectType()) {
        if (ppObjType)
          *ppObjType = datatype->getNextDefinition();
        return (*it);
      }
    }

    auto nextit = it;
    if (++nextit == end() && ent->isMemberVariable()) {
      if (ppObjType)
        *ppObjType = ent->getOwner();
      return (*it);
    }

    if (ent->isCollectionAccessor()) {
      if (ppObjType)
        *ppObjType = static_cast<Sm::Type::collection_methods::CollectionMethod*>(ent)->getCollection();
      return (*it);
    }
    else if (ent->isConstructor() || ent->isMethod()) {
      if (ppObjType)
        *ppObjType = ent->getOwner();
      return (*it);
    }
  }
  return NULL;
}

std::string Sm::debugSCathegoryConvert(SCathegory::t t) {
  switch (t) {
    case SCathegory::InsertingValues                : return "InsertingValues";
    case SCathegory::StatementConstructBlockPlsql   : return "StatementConstructBlockPlsql";
    case SCathegory::StatementMerge                 : return "StatementMerge";
    case SCathegory::FunctionDynExpr                : return "FunctionDynExpr";
    case SCathegory::StatementConstructExpr         : return "ConstructExpr";
    case SCathegory::FunctionDynTail_               : return "DynTailExpr";
    case SCathegory::BlockPlSqlStatementsList       : return "BlockPlSqlStatementsList";
    case SCathegory::WhenExprClause                 : return "WhenExprClause";
    case SCathegory::StatementLoop                  : return "StatementLoop";
    case SCathegory::StatementIf                    : return "StatementIf";
    case SCathegory::StatementOpenCursor            : return "StatementOpenCursor";
    case SCathegory::StatementOpenFor               : return "StatementOpenFor";
    case SCathegory::StatementWhile                 : return "StatementWhile";
    case SCathegory::StatementFunctionCall          : return "StatementFunctionCall";
    case SCathegory::StatementClose                 : return "StatementClose";
    case SCathegory::FunctionDynField               : return "FunctionDynField";
    case SCathegory::DynamicFuncallTranslator       : return "DynamicFuncallTranslator";
    case SCathegory::QueryEntityDyn                 : return "QueryEntityDyn";
    case SCathegory::StatementGoto                  : return "StatementGoto";
    case SCathegory::StatementPipeRow               : return "StatementPipeRow";
    case SCathegory::StatementRaise                 : return "StatementRaise";
    case SCathegory::StatementForOfExpression       : return "StatementForOfExpression";
    case SCathegory::StatementFetch                 : return "StatementFetch";
    case SCathegory::StatementExit                  : return "StatementExit";
    case SCathegory::StatementImmediate             : return "StatementImmediate";
    case SCathegory::StatementCase                  : return "StatementCase";
    case SCathegory::StatementDeleteFrom            : return "StatementDeleteFrom";
    case SCathegory::StatementSelect                : return "StatementSelect";
    case SCathegory::StatementRollback              : return "StatementRollback";
    case SCathegory::StatementLockTable             : return "StatementLockTable";
    case SCathegory::StatementTransaction           : return "StatementTransaction";
    case SCathegory::StatementSingleInsert          : return "StatementSingleInsert";
    case SCathegory::WhereClauseDML                 : return "WhereClauseDML";
    case SCathegory::OrderByPartitionList           : return "OrderByPartitionList";
    case SCathegory::LValueHost                     : return "LValueHost";
    case SCathegory::LValue                         : return "LValue";
    case SCathegory::ForOfRangeRange                : return "ForOfRangeRange";
    case SCathegory::ViewQueryNode                  : return "ViewQueryNode";
    case SCathegory::AliasedFieldsList              : return "AliasedFieldsList";
    case SCathegory::__LAST_SCATHEGORY__            : return "__LAST_SCATHEGORY__";
    case SCathegory::DeclNamespace                  : return "DeclNamespace";
    case SCathegory::OfTypes                        : return "OfTypes";
    case SCathegory::BulkRowcount                   : return "BulkRowcount";
    case SCathegory::FunctionPragmaRestriction      : return "FunctionPragmaRestriction";
    case SCathegory::Anydata                        : return "Anydata";
    case SCathegory::AlgebraicCompound              : return "AlgebraicCompound";
    case SCathegory::PartitionList                  : return "PartitionList";
    case SCathegory::Subtype                        : return "Subtype";
    case SCathegory::Into                           : return "Into";
    case SCathegory::PragmaRestrictReferences       : return "PragmaRestrictReferences";
    case SCathegory::ArrayConstructor               : return "ArrayConstructor";
    case SCathegory::Merge                          : return "Merge";
    case SCathegory::FactoringItem                  : return "FactoringItem";
    case SCathegory::WhereClause                    : return "WhereClause";
    case SCathegory::RootSemanticNode               : return "RootSemanticNode";
    case SCathegory::ModelContext                   : return "ModelContext";
    case SCathegory::BooleanLiteral                 : return "BooleanLiteral";
    case SCathegory::AddFields                      : return "AddFields";
    case SCathegory::InsertInto                     : return "InsertInto";
    case SCathegory::SqlStatement                   : return "SqlStatement";
    case SCathegory::AlterFields                    : return "AlterFields";
    case SCathegory::AlterTable                     : return "AlterTable";
    case SCathegory::AlterTableAddConstraint        : return "AlterTableAddConstraint";
    case SCathegory::AlterTableDropConstraint       : return "AlterTableDropConstraint";
    case SCathegory::AlterTableDropFields           : return "AlterTableDropFields";
    case SCathegory::AlterTableDropKey              : return "AlterTableDropKey";
    case SCathegory::AlterTableManipulateFields     : return "AlterTableManipulateFields";
    case SCathegory::AlterTableModifyConstraint     : return "AlterTableModifyConstraint";
    case SCathegory::AlterTableModifyFields         : return "AlterTableModifyFields";
    case SCathegory::AlterTableModifyKey            : return "AlterTableModifyKey";
    case SCathegory::AlterTableRenameTable          : return "AlterTableRenameTable";
    case SCathegory::AlterUser                      : return "AlterUser";
    case SCathegory::AlterUserSettings              : return "AlterUserSettings";
    case SCathegory::Label                          : return "Label";
    case SCathegory::Argument                       : return "Argument";
    case SCathegory::Assignment                     : return "Assignment";
    case SCathegory::AsteriskExpr                   : return "AsteriskExpr";
    case SCathegory::Between                        : return "Between";
    case SCathegory::BlockPlSql                     : return "BlockPlSql";
    case SCathegory::Case                           : return "Case";
    case SCathegory::CaseStatement                  : return "CaseStatement";
    case SCathegory::CaseVariants                   : return "CaseVariants";
    case SCathegory::Cast                           : return "Cast";
    case SCathegory::CastMultiset                   : return "CastMultiset";
    case SCathegory::Close                          : return "Close";
    case SCathegory::Collection                     : return "Collection";
    case SCathegory::CollectionAccess               : return "CollectionAccess";
    case SCathegory::CollectionIndex                : return "CollectionIndex";
    case SCathegory::Comparsion                     : return "Comparsion";
    case SCathegory::Compound                       : return "Compound";
    case SCathegory::CompoundTimingBlock            : return "CompoundTimingBlock";
    case SCathegory::Constraint                     : return "Constraint";
    case SCathegory::ConstraintState                : return "ConstraintState";
    case SCathegory::Constructor                    : return "Constructor";
    case SCathegory::Cursor                         : return "Cursor";
    case SCathegory::CursorEntity                   : return "CursorEntity";
    case SCathegory::CursorVariable                 : return "CursorVariable";
    case SCathegory::CursorVariableHost             : return "CursorVariableHost";
    case SCathegory::Datatype                       : return "Datatype";
    case SCathegory::Dblink                         : return "Dblink";
    case SCathegory::Declaration                    : return "Declaration";
    case SCathegory::DmlEvents                      : return "DmlEvents";
    case SCathegory::DynamicCursor                  : return "DynamicCursor";
    case SCathegory::EMPTY                          : return "EMPTY";
    case SCathegory::Else                           : return "Else";
    case SCathegory::Entity                         : return "Entity";
    case SCathegory::Exception                      : return "Exception";
    case SCathegory::ExecuteImmediate               : return "ExecuteImmediate";
    case SCathegory::Exit                           : return "Exit";
    case SCathegory::Expr_Asterisk                  : return "Expr_Asterisk";
    case SCathegory::Expr_Currval                   : return "Expr_Currval";
    case SCathegory::Expr_CursorProperty            : return "Expr_CursorProperty";
    case SCathegory::Expr_ExistFunc                 : return "Expr_ExistFunc";
    case SCathegory::Expr_HostCursorProperty        : return "Expr_HostCursorProperty";
    case SCathegory::Expr_HostSecondRef             : return "Expr_HostSecondRef";
    case SCathegory::Expr_NextVal                   : return "Expr_NextVal";
    case SCathegory::Expr_OutherJoin                : return "Expr_OutherJoin";
    case SCathegory::Expr_SqlBulkRowcount           : return "Expr_SqlBulkRowcount";
    case SCathegory::Expr_SqlCursorProperty         : return "Expr_SqlCursorProperty";
    case SCathegory::AnalyticFun                    : return "Lag";
    case SCathegory::Fetch                          : return "Fetch";
    case SCathegory::Field                          : return "Field";
    case SCathegory::FieldProperty                  : return "FieldProperty";
    case SCathegory::Flashback                      : return "Flashback";
    case SCathegory::ForAll                         : return "ForAll";
    case SCathegory::ForOfExpression                : return "ForOfExpression";
    case SCathegory::ForOfRange                     : return "ForOfRange";
    case SCathegory::ForUpdateClause                      : return "ForUpdateClause";
    case SCathegory::ForeignField                   : return "ForeignField";
    case SCathegory::From                           : return "From";
    case SCathegory::FromJoin                       : return "FromJoin";
    case SCathegory::Function                       : return "Function";
    case SCathegory::Goto                           : return "Goto";
    case SCathegory::GroupBy                        : return "GroupBy";
    case SCathegory::GroupingSetsClause                   : return "GroupingSetsClause";
    case SCathegory::GroupingSetsCube               : return "GroupingSetsCube";
    case SCathegory::GroupingSetsRollup             : return "GroupingSetsRollup";
    case SCathegory::GroupingSetsSimple             : return "GroupingSetsSimple";
    case SCathegory::HierarhicalClause               : return "HierarhicalClause";
    case SCathegory::If                             : return "If";
    case SCathegory::IfThen                         : return "IfThen";
    case SCathegory::Index                          : return "Index";
    case SCathegory::InsertConditional              : return "InsertConditional";
    case SCathegory::InsertFromSubquery             : return "InsertFromSubquery";
    case SCathegory::InsertFromValues               : return "InsertFromValues";
    case SCathegory::InsertSingleValue              : return "InsertSingleValue";
    case SCathegory::InsertMulitpleValues           : return "InsertMulitpleValues";
    case SCathegory::InsertMultipleConditional      : return "InsertMultipleConditional";
    case SCathegory::InsertedValues                 : return "InsertedValues";
    case SCathegory::IsOfTypes                      : return "IsOfTypes";
    case SCathegory::Like                           : return "Like";
    case SCathegory::LockTable                      : return "LockTable";
    case SCathegory::Loop                           : return "Loop";
    case SCathegory::LoopBounds                     : return "LoopBounds";
    case SCathegory::MemberOf                       : return "MemberOf";
    case SCathegory::NestedTableType                : return "NestedTableType";
    case SCathegory::NestedTableInstance            : return "NestedTableInstance";
    case SCathegory::NestedTableProperty            : return "NestedTableProperty";
    case SCathegory::ObjectMember                   : return "ObjectMember";
    case SCathegory::TriggerRowReference            : return "TriggerRowReference";
    case SCathegory::ObjectType                     : return "ObjectType";
    case SCathegory::OpenCursor                     : return "OpenCursor";
    case SCathegory::OrderBy                        : return "OrderBy";
    case SCathegory::Package                        : return "Package";
    case SCathegory::PlEntity                       : return "PlEntity";
    case SCathegory::PlExpr                         : return "PlExpr";
    case SCathegory::PrimaryKey                     : return "PrimaryKey";
    case SCathegory::QueryBlock                     : return "QueryBlock";
    case SCathegory::Raise                          : return "Raise";
    case SCathegory::Record                         : return "Record";
    case SCathegory::RecordField                    : return "RecordField";
    case SCathegory::RecordType                     : return "RecordType";
    case SCathegory::RefCursor                      : return "RefCursor";
    case SCathegory::RegexpLike                     : return "RegexpLike";
    case SCathegory::Return                         : return "Return";
    case SCathegory::ReturnInto                     : return "ReturnInto";
    case SCathegory::Role                           : return "Role";
    case SCathegory::Rollback                       : return "Rollback";
    case SCathegory::Savepoint                      : return "Savepoint";
    case SCathegory::Schema                         : return "Schema";
    case SCathegory::SelectStatement                : return "SelectStatement";
    case SCathegory::QueryBlockField                : return "QueryBlockField";
    case SCathegory::SelectSingle                   : return "SelectSingle";
    case SCathegory::SelectBrackets                 : return "SelectBrackets";
    case SCathegory::SelectedField                  : return "SelectedField";
    case SCathegory::Sequence                       : return "Sequence";
    case SCathegory::SqlCommand                     : return "SqlCommand";
    case SCathegory::ChangedQueryEntity                      : return "ChangedQueryEntity";
    case SCathegory::SqlExpr                        : return "SqlExpr";
    case SCathegory::RefAbstract                      : return "RefAbstract";
    case SCathegory::Statement                      : return "Statement";
    case SCathegory::StatementPart                  : return "StatementPart";
    case SCathegory::Submultiset                    : return "Submultiset";
    case SCathegory::Subquery                       : return "Subquery";
    case SCathegory::SubqueryId                     : return "SubqueryId";
    case SCathegory::SubqueryPart                   : return "SubqueryPart";
    case SCathegory::Synonym                        : return "Synonym";
    case SCathegory::SynonymTarget                  : return "SynonymTarget";
    case SCathegory::Table                          : return "Table";
    case SCathegory::ChangedQueryTableCollectionExpr            : return "ChangedQueryTableCollectionExpr";
    case SCathegory::FromTableDynamic               : return "FromTableDynamic";
    case SCathegory::FromTableReference          : return "QueriedTableReference";
    case SCathegory::QueryPseudoField               : return "QueryPseudoField";
    case SCathegory::FromSingle                     : return "FromSingle";
    case SCathegory::TableQueryReference            : return "TableQueryReference";
    case SCathegory::TableFromSubquery              : return "TableFromSubquery";
    case SCathegory::TableOrView                    : return "TableOrView";
    case SCathegory::TableProperties                : return "TableProperties";
    case SCathegory::TableViewMaterializedView      : return "TableViewMaterializedView";
    case SCathegory::Tablespace                     : return "Tablespace";
    case SCathegory::TimingPoint                    : return "TimingPoint";
    case SCathegory::Transaction                    : return "Transaction";
    case SCathegory::Trigger                        : return "Trigger";
    case SCathegory::TriggerActionCall              : return "TriggerActionCall";
    case SCathegory::TriggerReferencig              : return "TriggerReferencig";
    case SCathegory::UnionQuery                          : return "UnionQuery";
    case SCathegory::Unique                         : return "Unique";
    case SCathegory::Update                         : return "Update";
    case SCathegory::UpdatingField                  : return "UpdatingField";
    case SCathegory::User                           : return "User";
    case SCathegory::Variable                       : return "Variable";
    case SCathegory::VariableHost                   : return "VariableHost";
    case SCathegory::VariableUndeclaredIndex        : return "VariableUndeclaredIndex";
    case SCathegory::Varray                         : return "Varray";
    case SCathegory::View                           : return "View";
    case SCathegory::DatabaseLink                   : return "DatabaseLink";
    case SCathegory::WhenThen                       : return "WhenThen";
    case SCathegory::While                          : return "While";
    case SCathegory::XmlElement                     : return "XmlElement";
    case SCathegory::XmlSchema                      : return "XmlSchema";
    case SCathegory::ExceptionSection               : return "ExceptionSection";
    case SCathegory::UnaryOp                        : return "UnaryOp";
    case SCathegory::Exists                         : return "Exists";
  };
  return "!!!UNKNOWN!!!";
}

Id *Sm::SelectedField::toAsterisk() const {
  if (AsteriskRefExpr *ref = expr_->toSelfAsteriskRefExpr())
    return ref->reference->entity();
  return 0;
}


void Sm::table::FieldDefinition::addFieldProperty(Sm::table::field_property::FieldProperty *p) { resolvedProperties[p->getThisPtr()] = p; }

Ptr<Sm::Id> Sm::NumericValue::getName() const { return new Sm::Id(toString(false), (NumericValue*)this, true); }

Sm::IsSubtypeValues Sm::QueryBlock::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (!supertype)
    return EXPLICIT;

  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;

  if (supertype->ddlCathegory() == ResolvedEntity::RefCursor_)
    return IMPLICIT_CAST_BY_FIELDS;

  if (selectList) {
    if (selectList->isAsterisk() && from && from->size() && from->front()) {
      Ptr<From> t = from->front();
      if (Sm::IsSubtypeValues res = t->isSubtype(supertype, plContext))
        return res;
    }
    return selectList->isSubtypeSingle(supertype, plContext);
  }

  return EXPLICIT;
}

Sm::IsSubtypeValues Sm::FromSingle::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  return reference->isSubtype(supertype, plContext);
}

Sm::IsSubtypeValues Sm::FromTableReference::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  return id ? id->isSubtype(supertype, plContext) : EXPLICIT;
}

void Sm::Table::setNameDefinition() {
  if (name->entity())
    name->entity()->definition((Table*)this);
  if (tableProperties)
    tableProperties->owner((Table*)this);
}

void Sm::RefExpr::resolve(ResolvedEntity *owner) {
  throw 999;
  if (owner) {
    IdEntitySmart::iterator it = reference->end();
    --it;
    owner->getFieldRef(*it);
    if (it != reference->begin() && (*it)->definition()) {
      do {
        Ptr<Id> prev = *it;
        --it;
        prev->definition()->getFieldRef(*it);
      } while (it != reference->begin() && (*it)->definition());
    }
  }
}

void Sm::pl_expr::Comparsion::resolve(ResolvedEntity *owner) {
  if (lhs)
    lhs->resolve(owner);
  if (rhs)
    for (List<PlExpr>::iterator it = rhs->begin(); it != rhs->end(); ++it)
      if (*it)
        (*it)->resolve(owner);
}



bool Sm::UnionQuery::getFields(EntityFields &fields) const {
     EntityFields interFields;
     if (lhs) {
       lhs->getFields(interFields);
       if (rhs) {
         EntityFields::iterator it = interFields.begin();
         UnionQuery *p = (UnionQuery*)this;
         p->assignFieldReferenceByPosition(interFields, it);
       }
       fields.insert(fields.end(), interFields.begin(), interFields.end());
       return true;
     }
     else if (rhs)
       return rhs->getFields(fields);
     return true;
  }

bool Sm::UnionQuery::getFieldRefByFromList(Ptr<Id> &field, Sm::SemanticTree *reference) {
    if (lhs && lhs->getFieldRefByFromList(field, reference)) {
      if (field->definition() && field->definition()->isElementaryLiteral())
        field->clearDefinition();
      else
        return true;
    }
    if (rhs && rhs->getFieldRefByFromList(field, reference))
      return true;
    return false;
  }

void Sm::UnionQuery::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
    std::vector<Ptr<Id> >::iterator first = it;
    if (lhs)
      lhs->assignFieldReferenceByPosition(container, it);
    if (rhs)
      rhs->assignFieldReferenceByPosition(container, first);
  }

Sm::IsSubtypeValues Sm::UnionQuery::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (lhs)
    if (Sm::IsSubtypeValues res = lhs->isSubtype(supertype, plContext))
      return res;
  if (rhs)
    return rhs->isSubtype(supertype, plContext);
  return EXPLICIT;
}

void Sm::UnionQuery::setIsUnionChild() {
  lhs->setIsUnionChild();
  lhs->setIsUnionChild();
  isUnionChild_ = true;
}

Sm::UnionQuery::UnionQuery(CLoc l, Ptr<Subquery> _lhs, sql_union::UnionOperation uop, Ptr<Subquery> _rhs)
  : GrammarBase(l), lhs(_lhs), op(uop), rhs(_rhs)
{
  if (!lhs || !rhs)
    throw 999;
  lhs->setIsUnionChild();
  rhs->setIsUnionChild();
  lhs->ownerSelectModel = this;
  rhs->ownerSelectModel = this;

  setFldDefPosOnRootQuery();
}


Sm::SelectBrackets::SelectBrackets(CLoc l, Ptr<Subquery> q)
  : GrammarBase(l), subquery(q)
{
  if (!subquery)
    throw 999;
  subquery->ownerSelectModel = (SelectBrackets*)this;

  setFldDefPosOnRootQuery();
}

SemanticTree *Sm::SelectBrackets::toSTreeBase() const {
  SemanticTree *n = toSTreeBaseHeadNode();
  SNODE(n);
  collectBodyNodes(n);
  return n;
}

void Sm::SelectBrackets::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::SelectBrackets, SemanticTree::EMPTY, this);
  n->addChildForce(node);
  if (isRootQuery() && subquery)
    node->addChild(toSTree());
  else {
    SNODE(node);
    CTREE2(node, subquery);
    collectBodyNodes(node);
  }
}

Sm::SemanticTree *Sm::SelectBrackets::toSTreeBaseHeadNode() const {
  SemanticTree *n = subquery->toSTree();
  n->unnamedDdlEntity = (SelectBrackets*)this;
  return n;
}

void Sm::SelectBrackets::collectBodyNodes(SemanticTree *n) const {
  CTREE(forUpdate);
  CTREE(orderBy);
  CollectSNode(n, groupBy);
}

void Sm::SelectSingle::collectSNode(SemanticTree *node) const {
  node->addChild(toSTree());
}


SemanticTree *Sm::SelectSingle::toSTreeBase() const {
  SemanticTree *n = queryBlock->toSTree();
  SNODE(n);
  CTREE(forUpdate);
  CTREE(orderBy);
  CollectSNode(n, groupBy);
  return n;
}

void Sm::SelectBrackets::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
  if (subquery)
    subquery->assignFieldReferenceByPosition(container, it);
}


Sm::SelectSingle::SelectSingle(CLoc l, Ptr<QueryBlock> q)
  : GrammarBase(l), queryBlock(q)
{
  if (!queryBlock)
    throw 999;
  queryBlock->ownerSelectModel = (SelectSingle*)this;

  setFldDefPosOnRootQuery();
}

Ptr<List<FactoringItem> > Sm::SelectSingle::pullUpFactoringList() const {
  if (queryBlock) {
    queryBlock.object()->toStreeFactoringList = false;
    return queryBlock->factoringList;
  }
  return 0;
}

void Sm::SelectSingle::assignFieldReferenceByPosition(std::vector<Ptr<Id> > &container, std::vector<Ptr<Id> >::iterator &it) {
  if (queryBlock)
    queryBlock->assignFieldReferenceByPosition(container, it);
}


void Sm::RefExpr::setStringType() {
  if (refSize() == 1) {
    Id* ent = refEntity();
    string str = ent->toQString();
    if (str.size() && isdigit(str.front())) {
      ent->setQuoted();
      ent->setStringLiteral();
    }
    if (ent->definition() == Datatype::mkVarchar2())
      ent->setStringLiteral();
    if (!ent->definition() && ent->quoted())
      ent->setStringLiteral();
  }
}

Sm::ResolvedEntitySet Sm::constraint::Attribute::indexedFields(Ptr<List<Id> > fields) const {
  Sm::ResolvedEntitySet lst;
  if (fields) {
    for (List<Id>::const_iterator it = fields->begin(); it != fields->end(); ++it)
      if (ResolvedEntity *def = (*it)->definition())
        lst.insert(def);
  }
  return lst;
}


bool Sm::Statement::getStatementsThatContainEntity(ResolvedEntity *entity, FoundedStatements &outList) {
  getStatementsWithEntity(entity, (Sm::Statement*)this, outList);
  return true;
}

bool Sm::Table::hasObjectFields() const {
  if (!relationFields)
    return true;

  for (RelationFields::const_iterator it = relationFields->begin(); it != relationFields->end(); ++it) {
    if (Ptr<Datatype> t = (*it)->getDatatype())
      if (ResolvedEntity *concreteDef = t->getConcreteDefinition())
      {
        if (concreteDef->ddlCathegory() == ResolvedEntity::Object_)
          return true;
        else if (concreteDef->ddlCathegory() != ResolvedEntity::FundamentalDatatype_)
          throw 999;
      }
  }
  return false;
}


bool Sm::Datatype::isExactlyEqually(Datatype *t) {
  if (!t)
    return false;
  ResolvedEntity *d1 = tidDef();
  ResolvedEntity *d2 = t->tidDef();
  if (!d1 || !d2)
    return false;
  if (d1 == d2)
    return true;
  if (isTypeOf()) {
    if (Ptr<Datatype> nextT = d1->getDatatype())
      return nextT->isExactlyEqually(t);
    else
      return false;
  }
  else if (isRowTypeOf()) {
    if (t->isRowTypeOf())
      return d1->eqByVEntities(d2);
    else
      return false;
  }

  return d1->isExactlyEquallyByDatatype(d2);
}


bool Sm::Datatype::isExactlyEquallyByDatatype(ResolvedEntity *t) {
  if (Sm::Datatype *othT = t->toSelfDatatype())
    return isExactlyEqually(othT);
  else if (Ptr<Datatype> &&othDatatype = t->getDatatype())
    return isExactlyEqually(othDatatype);
  else
    return false;
}



bool Subtype::isExactlyEquallyByDatatype(ResolvedEntity* t) {
  return datatype && datatype->isExactlyEqually(t->getDatatype());
}




bool Sm::Table::hasCleanNumberFields() const {
  if (objectName || !relationFields)
    throw 999;

  for (RelationFields::const_iterator it = relationFields->begin(); it != relationFields->end(); ++it)
    if ((*it)->getDatatype()->isCleanNumberType())
      return true;

  return false;
}

bool Sm::Table::hasNumberFieldsThatWillChanged() const {
  for (RelationFields::const_iterator it = relationFields->begin(); it != relationFields->end(); ++it)
    if ((*it)->getDatatype()->isChangedNumberDatatype()) // foreignDatatype - учитывается в getDatatype
      return true;
  return false;
}


void Sm::Table::printFieldForStatistics(Sm::Codestream &str, RelationFields::iterator it) {
  str << "'";
  this->linterReference(str);
  str << "', '";
  (*it)->linterReference(str);
  str << "'" << s::endl;
}

void Sm::Table::printCleanNumberFields(Sm::Codestream &str) {
  if (objectName || !relationFields)
    throw 999;

  for (RelationFields::iterator it = relationFields->begin(); it != relationFields->end(); ++it)
    if ((*it)->getConcreteDatatype()->isCleanNumberType())
      printFieldForStatistics(str, it);

}

void Sm::Table::printBigNumberFields(Sm::Codestream &str) {
  for (RelationFields::iterator it = relationFields->begin(); it != relationFields->end(); ++it)
    if ((*it)->getConcreteDatatype()->isNumberDatatype() && (*it)->getConcreteDatatype()->isBigNumber())
      printFieldForStatistics(str, it);
}

bool Sm::Datatype::isCleanNumberType() const {
  if (!isNumberDatatype() || precision > 0 || scaleIsSet())
    return false;
  return true;
}

bool Subtype::isRefCursor() const { return getDatatype()->isRefCursor(); }

ResolvedEntity* Sm::Datatype::getNextDefinition() const {
  if (isEverything())
    return (ResolvedEntity*)this;
  return tidDef();
}

bool Sm::Datatype::isChangedNumberDatatype() const {
  return !translatedToBigint() && !translatedToInt() && translatedToSmallInt();
}

std::string Sm::Datatype::toStringNumber() const {
  if (!isNumberDatatype())
    throw 999;
  if (precision == 0)
    return "NUMBER";
  else if (scaleIsNotSet()) {
    stringstream str;
    str <<  "NUMBER(" << precision << ")";
    return str.str();
  }
  else {
    stringstream str;
    str <<  "NUMBER(" << precision << "," << scale << ")";
    return str.str();
  }
  return "";
}

void Sm::Table::calculateNumberPrecStatistic(std::map<string, int> &stat) {
  for (RelationFields::iterator it = relationFields->begin(); it != relationFields->end(); ++it) {
    if (Ptr<Datatype> t = (*it)->getConcreteDatatype()) {
      if (!t->isNumberDatatype())
        continue;
      string s = t->toStringNumber();
      std::map<string, int>::iterator it = stat.find(s);
      if (it != stat.end())
        ++(it->second);
      else
        stat[s] = 1;
    }
  }
}

void Sm::Table::getDefaults(Ptr<Sm::Table> tbl) {
  if (relationFields && tbl->relationFields) {
    RelationFields::iterator it = relationFields->begin();
    RelationFields::iterator oIt = tbl->relationFields->begin();
    while (it != relationFields->end() && oIt != tbl->relationFields->end()) {
      if (!(*it)->defaultExpr && (*oIt)->defaultExpr && *((*it)->getName()) == *((*it)->getName()))
       (*it)->defaultExpr  = (*oIt)->defaultExpr;
      ++it; ++oIt;
    }
  }
}


void Sm::Variable::setDatatype(Ptr<Sm::Datatype> t) {
  datatype = t;
  if (datatypeNode) {
    datatypeNode->unnamedDdlEntity = t.object();
    datatypeNode->refEntity()->definition(t.object());
  }
}

Sm::Package::Package(CLoc l, Ptr<Id2> name, Ptr<BlockPlSql> content, bool _isBody)
  : GrammarBase(l),
    name(name),
    filter_(0)
{
  if (name && name->entity())
    name->entity()->definition(this);
  if (content) {
    content->isNotPackageBlock_ = false;
    if(_isBody)
      bodies.push_back(content);
    else
      heads.push_back(content);
  }
}

void Sm::Package::pullFromOther(Ptr<Package> oth) {
  if (oth) {
    for (Ptr<BlockPlSql> &b : oth->heads) {
      b->isNotPackageBlock_ = false;
      heads.push_back(b);
    }
    for (Ptr<BlockPlSql> &b : oth->bodies) {
      b->isNotPackageBlock_ = false;
      bodies.push_back(b);
    }
  }
}

bool Sm::Type::collection_methods::CollectionMethod::getCollectionFieldRef(Ptr<Id> &field) {
  if (field->empty()) {
    if (collectionDatatype_)
      if (Ptr<Datatype> t = collectionDatatype_->getDatatype())
        if (ResolvedEntity *d1 = t->getNextDefinition())
          if (ResolvedEntity *d2 = d1->getNextDefinition()) {
            field->definition(d2);
            return true;
          }
    return false;
  }
  return collectionDatatype_ && collectionDatatype_->getFieldRef(field);
}


Ptr<Sm::Arglist>     ResolvedEntity::getArglist()  const { return 0; }
Ptr<Sm::BlockPlSql>  ResolvedEntity::funBody()     const { return 0; }
Ptr<Sm::CallArgList> ResolvedEntity::callArglist() const { return 0; }
Ptr<Sm::Datatype>    ResolvedEntity::keyType()     const { return 0; }
Ptr<Sm::Datatype>    ResolvedEntity::mappedType()  const { return 0; }
Ptr<Sm::Datatype>    ResolvedEntity::getElementDatatype() const { return getDatatype(); }
Ptr<Sm::FunArgList>  ResolvedEntity::funArglist()  const { return 0; }
Ptr<Sm::Id2>         ResolvedEntity::getTarget()   const { return 0; }
Ptr<Sm::LValue>      ResolvedEntity::lvalue()   { return 0; }


Ptr<Sm::Id>          ResolvedEntity::getName () const { return 0; }
Ptr<Sm::Id>          ResolvedEntity::getAlias() const { return 0; }
Ptr<Sm::Id2>         ResolvedEntity::getName2() const { return 0; }
void ResolvedEntity::setDatatypeForMember(Ptr<Sm::Id>, Ptr<Sm::Datatype>) { throw 999; }
void ResolvedEntity::setDatatype(Ptr<Sm::Datatype> ) { throw 999; }
void ResolvedEntity::setRetType(Ptr<Sm::Datatype>) {}

Ptr<Sm::Datatype> Sm::Id::getDatatype() const { return definition() ? definition()->getDatatype() : Ptr<Sm::Datatype>(); }

Sm::Subtype::Subtype(CLoc l, Sm::Id *n, Sm::Datatype *t, Sm::Constraint *c, bool _notNull)
    : GrammarBase(l), name(n), datatype(t), constraint_(c), notNull(_notNull)  {  if (name) name->definition((Subtype*)this);  }


Sm::SemanticTree *Sm::Table::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::Table, this);
  switch (tableCathegory) {
    case RELATION:
      fieldSNode = new SemanticTree(SCathegory::Field);
      CollectSNode(fieldSNode, relationFields);
      n->addChild(fieldSNode);
      break;
    case OBJECT:
      if (objectName)
        n->addChild(objectName->toSNodeRef(SCathegory::ObjectType));
      // pass
    case XML:
      CTREE(objectProperties)
      ANODE(oidIndex)
  }
  CTREE(tableProperties)
  for (const Ptr<PlExpr> &x : checkConditions)
    CTREE(x)
  return n;
}

Sm::SemanticTree *Sm::Variable::toSTreeBase() const {
  if (name) {
    SemanticTree *n = name->toSNodeDecl(SCathegory::Variable, this);
    if (datatype) {
      datatypeNode = datatype->toSTree();
      n->addChild(datatypeNode);
    }
    CTREE(defaultValue_);
    n->unnamedDdlEntity = (Variable*)this;
    return n;
  }
  return 0;
}

Sm::SemanticTree *Sm::RefExpr::toSTreeBase() const {
  SemanticTree *node = reference->toSTreeRef(SCathegory::RefAbstract);
  node->unnamedDdlEntity = const_cast<RefExpr*>(this);
  SNODE(node);
  return node;
}

Sm::SemanticTree *Sm::Subtype::toSTreeBase() const {
  SemanticTree *n =  name->toSNodeDecl(SCathegory::Subtype, this);
  ANODE(datatype);
  return n;
}

Sm::SemanticTree *Sm::UnionQuery::toSTreeBaseHeadNode() const {
  SemanticTree *n = new SemanticTree(SCathegory::UnionQuery, SemanticTree::NEW_LEVEL);
  n->unnamedDdlEntity = (UnionQuery*)this;
  return n;
}

void Sm::UnionQuery::collectBodyNodes(Sm::SemanticTree *n) const {
  if (Ptr<List<FactoringItem> > f = pullUpFactoringList())
    for (List<FactoringItem>::const_iterator it = f->begin(); it != f->end(); ++it)
      n->addChild((*it)->toSTree());

  CTREE(lhs);
  CTREE(rhs);
  CTREE(forUpdate);
  CTREE(orderBy);
  CollectSNode(n, groupBy);
}

Sm::SemanticTree *Sm::UnionQuery::toSTreeBase() const {
  SemanticTree *n = toSTreeBaseHeadNode();
  SNODE(n);
  collectBodyNodes(n);
  return n;
}

void Sm::UnionQuery::collectSNode(SemanticTree *node) const {
  if (isRootQuery())
    node->addChild(toSTree());
  else {
    SNODE(node);
    collectBodyNodes(node);
  }
}

SemanticTree *Sm::View::toSTreeBase() const {
  SemanticTree *node = name->toSNodeDef(SCathegory::View, this);

  SemanticTree *qNode = new SemanticTree(SCathegory::ViewQueryNode);
  CTREE2(qNode, select)
  queryNode = qNode;
  node->addChild(queryNode);
  if (aliasedFields.size()) {
    SemanticTree *aNode = new SemanticTree(SCathegory::AliasedFieldsList, SemanticTree::NEW_LEVEL);
    aliasedFieldsNode = aNode;
    for (AliasedFields::const_iterator it = aliasedFields.begin(); it != aliasedFields.end(); ++it)
      aNode->addChild((*it)->toSTree());
    node->addChild(aNode);
  }
  return node;
}

SemanticTree *Sm::FunctionArgument::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDecl(SCathegory::Argument, static_cast<const ResolvedEntity*>(this));
  ANODE(datatype)
  CTREE(defaultValue_);
  return n;
}

void SelectedField::collectSNode(SemanticTree *node) const {
  if (!expr_)
    return;
  SemanticTree *n = fieldName->toSNodeDecl(SCathegory::QueryBlockField, this);
  node->addChildForce(n);
  setSemanticNode(n);
  CTREE(expr_);
  if (pseudoField)
    n->alias(pseudoField->toSTree());
}

SemanticTree *Sm::Function::toSTreeBase() const {
  if (name)
    name->definition(this);
  SemanticTree *n = body_ ? name->toSNodeDef(SCathegory::Function, this) : name->toSNodeDecl(SCathegory::Function, this);
  CollectSNode(n, arglist);
  n->unnamedDdlEntity = (Function*)this;
  ANODE(rettype);
  CTREE(body_);
  return n;
}

Sm::SemanticTree *FactoringItem::toSTreeBase() const {
  // Необходимо устанавливать соответствие между columnAliases и subquery (возможно через getFieldRef);
  if (columnAliases && subquery) {
    Ptr<List<Id> >    aliases = columnAliases;
    for (Ptr<Id> &fieldAlias: *aliases)
      subquery.object()->getFieldRef(fieldAlias);
  }
  SemanticTree *n = queryName->toSNodeDef(SCathegory::FactoringItem, this);
  CTREE(subquery);
  return n;
}


Sm::SemanticTree *QueryBlock::toSTreeBase() const {
  SemanticTree *n = new SemanticTree(SCathegory::QueryBlock, SemanticTree::DECLARATION);
  n->unnamedDdlEntity = (QueryBlock*)this;
  if (from && from->size()) {
    this->fromNode_ = createFromListNode(from);
    n->addChild(this->fromNode_);
  }
  if (toStreeFactoringList && factoringList && factoringList->size()) {
    SemanticTree *factoringNode = new SemanticTree(SCathegory::FactoringItem, SemanticTree::NEW_LEVEL);
    factoringNode->setIsList();
    this->factoringNode_ = factoringNode;
    for (List<FactoringItem>::const_iterator it = factoringList->begin(); it != factoringList->end(); ++it)
      factoringNode->addChild((*it)->toSTree());
    n->addChild(factoringNode);
  }

  if (selectList)
    if (SemanticTree *selNode = selectList->toSTree()) {
      n->addChild(selNode);
      selectedListNode_ = selNode;
    }

  if (where) {
    SemanticTree *whereNode = new SemanticTree(SCathegory::WhereClause, SemanticTree::NEW_LEVEL);
    CTREE2(whereNode, where);
    n->addChild(whereNode);
  }
  CTREE(orderBy)

  CTREE(tailSpec);
//  if (hierarhicalSpec) {
//    SemanticTree *hierarhNode = new SemanticTree(SCathegory::HierarhicalClause, SemanticTree::NEW_LEVEL);
//    CTREE2(hierarhNode, hierarhicalSpec);
//    n->addChild(hierarhNode);
//  }
//  CollectSNode(n, groupBy);
//  CTREE(having);

  if (intoList && intoList->size()) {
    SemanticTree *intoNode = new SemanticTree(SCathegory::Into);
    if (intoList)
      for (const IntoList::dereferenced_type::value_type &v : *intoList )
        if (v)
          if (SemanticTree *n = v->toSTree()) {
            n->setIsIntoNode();
            intoNode->addChild(n);
          }
    intoNode->setIsList();
    n->addChild(intoNode);
  }
  return n;
}


SemanticTree *Sm::Merge::toSTreeBase() const {
  SemanticTree *node = new SemanticTree(SCathegory::StatementMerge);
  SemanticTree *n = new SemanticTree(SCathegory::QueryBlock, SemanticTree::DECLARATION);
//  SemanticTree *n = new SemanticTree(SCathegory::SqlStatement, SemanticTree::NEW_LEVEL);
  n->unnamedDdlEntity = const_cast<Merge*>(this);

  this->fromNode = createFromListNode(fromList);
  n->addChild(this->fromNode);

  SemanticTree *whereNode = new SemanticTree(SCathegory::WhereClause, SemanticTree::NEW_LEVEL);

  CTREE2(whereNode, onCondition)
  CTREE2(whereNode, matchedUpdate)
  CTREE2(whereNode, notMatchedInsert)

  n->addChild(whereNode);
  node->addChild(n);
  node->unnamedDdlEntity = const_cast<Merge*>(this);
  return node;
}




void Sm::FromSingle::initReferenceName() const
{
  Ptr<Id> n = reference->getName();
  if (n && !n->toNormalizedString().empty()) {
    referenceName = new Id(*n);
  }
  else
    referenceName = new Id(string(0, 1));
  referenceName->definition(this);
}

Sm::SemanticTree *Sm::FromTableReference::toSTreeBase() const {
  SemanticTree *node = tableReference->toSNodeDecl(SCathegory::TableQueryReference, this);
  if (dblink) {
    SemanticTree *n = new SemanticTree(id, SemanticTree::EXTDB_REFERENCE, SCathegory::FromTableReference);
    n->addChild(new SemanticTree(dblink, SemanticTree::EMPTY, SCathegory::Dblink));
    node->addChild(n);
  }
  else
    node->addChild(id->toSNodeRef(SCathegory::FromTableReference));
  return node;
}


namespace Sm {

SemanticTree *Sm::Datatype::toSTreeBase() const {
  SemanticTree *node;
  if (!(flags.v & FLAG_DATATYPE_IS_SPEC_REFERENCE))
    node = new SemanticTree((Datatype*)this, SemanticTree::DATATYPE_REFERENCE, SCathegory::Datatype);
  else
    node = tid->toSTreeRef(SCathegory::EMPTY);


  node->unnamedDdlEntity = (Datatype*)this;
  return node;
}


BooleanLiteral::BooleanLiteral(bool _isTrue) : isTrue(_isTrue) {
  setIsSystem();
  if (isTrue)
    name = new Id("TRUE", (ResolvedEntity*)this);
  else
    name = new Id("FALSE", (ResolvedEntity*)this);
}

Ptr<Sm::Datatype> BooleanLiteral::getDatatype() const { return Sm::Datatype::mkBoolean(); }

void BooleanLiteral::linterDefinition(Codestream &str) {
  if (isTrue)
    str << "TRUE";
  else
    str << "FALSE";
}

SemanticTree *BooleanLiteral::toSTreeBase() const { return new SemanticTree(name, SemanticTree::DECLARATION, SCathegory::BooleanLiteral); }





SemanticTree *Exception::toSTreeBase() const {
  return exception->toSNodeDecl(SCathegory::Exception, this);
}

Ptr<Id> Table::getName () const { return name->entity(); }
Ptr<Id2> Table::getName2() const { return name; }

Ptr<Id> Package::getName() const { return name ? Ptr<Id>(name->entity()) : Ptr<Id>(); }

void Package::addSemanticDeclarations(SemanticTree *node, const BaseList<BlockPlSql> &entities) const {
  for (BaseList<BlockPlSql>::const_iterator it = entities.begin(); it != entities.end(); ++it) {
    // (*it)->collectNameSpace();
    for (List<Declaration>::const_iterator dit = (*it)->declarations.begin(); dit != (*it)->declarations.end(); ++dit)
      node->addChild((*dit)->toSTree());
  }
}

void Package::addSemanticEndLabels(SemanticTree *node, const BaseList<BlockPlSql> &entities) const {
  for (BaseList<BlockPlSql>::const_iterator it = entities.begin(); it != entities.end(); ++it)
    if (Id *endlabel = (*it)->endLabel.object())
      node->addChild(endlabel->toSNodeDecl(SCathegory::Label, this));
}

void Package::addSemanticExceptions(SemanticTree *n, const BaseList<BlockPlSql> &entities) const {
  for (BaseList<BlockPlSql>::const_iterator it = entities.begin(); it != entities.end(); ++it)
    CollectSNode(n, (*it)->exceptionHandlers);
}

void Package::addSemanticStatements(SemanticTree *node, const BaseList<BlockPlSql> &entities) const {
  for (BaseList<BlockPlSql>::const_iterator it = entities.begin(); it != entities.end(); ++it)
    (*it)->collectStatementsSNode(node);
}

bool Package::getFieldRef(Ptr<Id> &reference) {
  if (Sm::SemanticTree *n = getSemanticNode())
    if (LevelResolvedNamespace *childNamespace = n->childNamespace.object()) {
      SemanticTree *node = 0;
      if (childNamespace->findDeclaration(node, reference)) {
        if (!reference->semanticNode())
          reference->semanticNode(node);
        return true;
      }
    }
  return false;
}

SemanticTree *Package::toSTreeBase() const {
  SemanticTree *node = bodies.size() ? name->toSNodeDecl(SCathegory::Package, this) : name->toSNodeDef(SCathegory::Package, this);
  node->unnamedDdlEntity = (Package*)this;

  addSemanticDeclarations(node, heads );
  addSemanticDeclarations(node, bodies);
  addSemanticStatements  (node, heads );
  addSemanticStatements  (node, bodies);
  addSemanticExceptions  (node, heads );
  addSemanticExceptions  (node, bodies);
  addSemanticEndLabels   (node, heads );
  addSemanticEndLabels   (node, bodies);

  return node;
}

Sm::BlockPlSql *Package::childCodeBlock() const { return bodies.front().object(); }

bool Package::bodyEmpty() const { return bodies.empty(); }

bool Package::headEmtpy() const { return heads.empty(); }

Ptr<Datatype> Package::getDatatype() const { return Ptr<Datatype>(); }

string Constraint::key() { return attribute() ? attribute()->key() : string(); }
Ptr<constraint::Attribute> Constraint::attribute() const { return attribute_; }

Constraint::Constraint(CLoc l, Ptr<Id> _name, Ptr<constraint::Attribute> attr, Ptr<constraint::ConstraintState> s)
  : GrammarBaseSmart(l), name(_name), attribute_(attr), state(s)
{
  if (!attribute())
    throw 999;
}

bool Constraint::isPrimaryKey () const { return attribute_ && attribute_->cathegory() == constraint::Attribute::PRIMARY_KEY; }
bool Constraint::isUniqueKeys () const { return attribute_ && attribute_->cathegory() == constraint::Attribute::UNIQUE; }
bool Constraint::isCheck      () const { return attribute_ && attribute_->cathegory() == constraint::Attribute::CHECK; }
bool Constraint::isForeignKeys() const { return attribute_ && attribute_->cathegory() == constraint::Attribute::FOREIGN_KEY; }
bool Constraint::isOther      () const { return !isPrimaryKey() && !isUniqueKeys() && !isCheck() && !isForeignKeys() && !isOther(); }
bool Constraint::eq(Ptr<Constraint> c) const { return c && attribute() && c->attribute() && attribute()->eq(c->attribute()); }
Constraint::~Constraint() {}
void StatementInterface::changeCursorVariable(Ptr<LinterCursor>) { throw 999; }
void Exception::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }
StatementInterface::~StatementInterface() {}


Ptr<Datatype> Exception::getDatatype() const { return Ptr<Datatype>(); }
StatementInterface::StatementInterface() : translateMode_(TM_NORMAL) {}
bool StatementInterface::setOwnerBlockPlSql(BlockPlSql *b) {
  if (beginedFrom(58850,19))
      cout << "";
  ownerBlock = b;
  return true;
}

Ptr<Datatype> Statement::getDatatype() const { return 0; }


void FunCallArgExpr::linterDefinition(Codestream &s) { s << _expr; }

void FunCallArgExpr::sqlDefinition(Codestream &s) { s << _expr; }

FunCallArgNamed::FunCallArgNamed(CLoc l, Ptr<Id> n, Ptr<PlExpr> e)
  : GrammarBase(l), _name(n), _expr(e) {}

Ptr<Id> FunCallArgExpr::getName() const { return _expr->getName(); }

void FunCallArgExpr::collectSNode(SemanticTree *n) const {
  CTREE(_expr); setSemanticNode(n);
}

Ptr<Datatype> FunCallArg::getDatatype() const {
  if (PlExpr* e = expr().object())
    return e->getDatatype();
  return 0;
}

Sm::IsSubtypeValues FunCallArg::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return expr()->isSubtype(supertype, plContext);
}

Ptr<Id> FunCallArg::getName() const {
  if (PlExpr* e = expr().object())
    return e->getName();
  return 0;
}

bool FunCallArg::isAsterisk() const { return argclass() == ASTERISK || (expr() && expr()->isAsterisk()); }

bool FunCallArg::getFieldRef(Ptr<Id> &field) { return expr() && expr()->getFieldRef(field); }

ResolvedEntity *FunCallArg::getNextDefinition() const { return expr() ? expr()->getNextDefinition() : (ResolvedEntity*)0; }

bool FunCallArg::getFields(EntityFields &fields) const { return expr() && expr()->getFields(fields); }


FunCallArg::~FunCallArg() {
  setSemanticNode(0);
//  clearSemanticReferences();
}

void FunCallArgNamed::collectSNode(SemanticTree *n) const { CTREE(_expr); setSemanticNode(n); }

inline Codestream& operator<<(Codestream& s, FunCallArg &obj) { return obj.translate(s); }

Ptr<PlExpr> FunCallArgExpr::expr() const { return _expr.object(); }

Ptr<Id> FunCallArgNamed::argname() const { return _name.object(); }

Ptr<PlExpr> FunCallArgNamed::expr() const { return _expr.object(); }

FunCallArgAsterisk::FunCallArgAsterisk(CLoc l) : GrammarBase(l) {}

FunCallArgAsterisk::FunCallArgAsterisk(CLoc l, Ptr<PlExpr> __expr) : GrammarBase(l), _expr(__expr) {}

Ptr<Id> FunCallArg::argname() const { return 0; }


AlterUser::AlterUser(CLoc l, Ptr<List<Id> > _users, Ptr<alter_user::UserSettings> _settings)
  : GrammarBaseSmart(l), semanticNode(0),  users(_users), settings(_settings) {}

SemanticTree *AlterUser::toSTree() const {
  if (semanticNode)
    return 0;
  semanticNode =  new SemanticTree(Sm::SCathegory::AlterUser);
  if (users)
    for (List<Id>::const_iterator it = users->begin(); it != users->end(); ++it)
      semanticNode->addChild(it->object()->toSNodeRef(SCathegory::User));
  ANODE2(semanticNode, settings);
  return semanticNode;
}

Ptr<Id> Synonym::getName() const { return name->entity(); }

Ptr<Id2> Synonym::getName2() const { return name; }

Ptr<Id2> Synonym::getTarget() const { return target; }

Ptr<Datatype> Synonym::getDatatype() const {
  if (target->entity() && target->entity()->definition())
    return  target->entity()->definition()->getDatatype();
  return Ptr<Datatype>();
}

bool Synonym::getFields(EntityFields &fields) const {
  bool res = target && target->definition() && target->definition()->getFields(fields);
  return res;
}

Sm::IsSubtypeValues Synonym::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (ResolvedEntity *def = target->definition())
    return def->isSubtype(supertype, plContext);
  return EXPLICIT;
}

Synonym::Synonym(Ptr<Id2> _name, Ptr<Id2> _target, bool _isPublic, CLoc l)
  : GrammarBase(l), name(_name), target(_target), isPublic(_isPublic)
{
  if (Ptr<Id> n = name->entity())
    n->definition(this);
}

SemanticTree *Synonym::toSTreeBase() const { return name->toSNodeDef(SCathegory::Synonym, this); }

ResolvedEntity *Synonym::getNextDefinition() const { return target->definition(); }

Synonym::~Synonym() {}

Ptr<Id> View::getName() const { return name->entity(); }

Ptr<Id2> View::getName2() const { return name; }

View::View(CLoc l, Ptr<Id2> _name, Ptr<view::ViewProperties> _properties, Ptr<Subquery> q, Ptr<view::ViewQRestriction> restr)
  : GrammarBase(l), semanticResolved(false), aliasedFieldsNode(0), queryNode(0), name(_name), properties(_properties), select(q), qRestriction(restr) {
  name->entity()->definition((View*)this);
  parseConstraintList();
}

void View::linterReference(Sm::Codestream &str) { str << name; }

View::~View() {}

Ptr<Id> Sequence::getName() const { return name ? Ptr<Id>(name->entity()) : Ptr<Id>(); }

Ptr<Id2> Sequence::getName2() const { return name; }

Ptr<Datatype> Sequence::getDatatype() const { return Datatype::mkNumber(); }

Sm::IsSubtypeValues Sequence::isSubtype(ResolvedEntity *supertype, bool plContext) const { return isSubtypeByDatatype(supertype, plContext); }

Sequence::Sequence(CLoc l, Ptr<Id2> _name, Ptr<SequenceBody> _data)
  : GrammarBase(l), name(_name), data(_data)
{
  pseudocolumns[(int)(SequencePseudocolumn::CURRVAL)] = new SequencePseudocolumn(SequencePseudocolumn::CURRVAL);
  pseudocolumns[(int)(SequencePseudocolumn::NEXTVAL)] = new SequencePseudocolumn(SequencePseudocolumn::NEXTVAL);
}

SemanticTree *Sequence::toSTreeBase() const { return name->toSNodeDef(SCathegory::Sequence, this); }

string SequencePseudocolumn::cathegoryToSTring(SequencePseudocolumn::Cathegory t) {
  switch (t) {
    case CURRVAL: return "CURRVAL";
    case NEXTVAL: return "NEXTVAL";
  }
  return "";
}

SequencePseudocolumn::SequencePseudocolumn(SequencePseudocolumn::Cathegory c) : cathegory(c) {}
Ptr<Sm::Datatype> SequencePseudocolumn::getDatatype() const { return Datatype::mkNumber(); }
Sm::IsSubtypeValues SequencePseudocolumn::isSubtype(ResolvedEntity *supertype, bool isPlContext) const { return Datatype::mkNumber()->isSubtype(supertype, isPlContext); }
Ptr<Id> SequencePseudocolumn::getName() const { return name ? name : (name = new Id(cathegoryToSTring(cathegory), (SequencePseudocolumn*)this)); }

void SequencePseudocolumn::linterReference(Codestream &str) { str << cathegoryToSTring(cathegory); }

Trigger::~Trigger() {}

Trigger::Trigger(CLoc l, Ptr<Id2> _name, Ptr<trigger::TriggerEvents> _events, trigger::TriggerMode _mode, Ptr<List<Id2> > _followsAfterTrigger, EnableState::T _enableState, Ptr<PlExpr> _whenCondition, Ptr<trigger::TriggerActionInterface> _action)
  : GrammarBase(l), name(_name), events(_events), mode(_mode), followsAfterTrigger(_followsAfterTrigger), enableState(_enableState),
    whenCondition(_whenCondition), action(_action)
{
  if (Sm::Id *n = name->entity())
    n->definition(this);
}

Ptr<Datatype> Trigger::getDatatype() const { return Ptr<Datatype>(); }

Ptr<Id> Trigger::getName() const { return name->entity(); }

Ptr<Id2> Trigger::getName2() const { return name; }

Sm::BlockPlSql *Trigger::childCodeBlock() const { return action ? action->childCodeBlock() : 0; }

bool Trigger::isDefinition() const { return action; }

ResolvedEntity *Index::tableDef() const { return table->definition(); }

Ptr<Id> Index::getName() const { return name ? Ptr<Id>(name->entity()) : Ptr<Id>(); }

Ptr<Id2> Index::getName2() const { return name; }

Ptr<Datatype> Index::getDatatype() const { return Ptr<Datatype>(); }

void Index::setName(Ptr<Id2> name) {
  this->name = name;
  updateNameDefinition();
}

bool Index::getFieldRef(Ptr<Id> &field) {
  if (ResolvedEntity *tableDef = table->definition())
    return tableDef->getFieldRef(field);
  return false;
}

Index::Index(CLoc l, Ptr<Id2> tbl, Ptr<Id> _alias, Ptr<Index::FieldList> fldList, Sm::CathegoryIndexEnum::CathegoryIndex cat)
  : GrammarBase(l),
    table(nAssert(tbl)),
    alias(_alias),
    fieldList(fldList) ,
    cathegoryIndex_(cat) { updateNameDefinition(); }

void Index::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }

SemanticTree *Index::toSTreeBase() const {
  if (Sm::Id *n = name->entity())
    n->definition(this);
  SemanticTree *node = name->toSNodeDef(SCathegory::Index, this);
  if (alias)
    node->addChild(alias->toSNodeRef(SCathegory::Table));
  CollectSNode(node, fieldList);
  node->unnamedDdlEntity = (Index*)this;
  return node;
}

DatabaseLink::DatabaseLink(Ptr<Id2> n, Ptr<DatabaseLinkBody> b, Ptr<Id> connString)
  : name(n), body(b), connectionString(connString), isPublic_(false), isShared_(false) {
  if (Sm::Id *n = name->entity())
    n->definition(this);
}

DatabaseLink::DatabaseLink() : isPublic_(false), isShared_(false) {}

Sm::IsSubtypeValues DatabaseLink::isSubtype(ResolvedEntity *supertype, bool) const { return eqByVEntities(supertype) ? EXACTLY_EQUALLY : EXPLICIT; }

Ptr<Id> DatabaseLink::getName() const { return name->entity(); }

Ptr<Id2> DatabaseLink::getName2() const { return name; }

void DatabaseLink::resolve(ModelContext &) {}

Ptr<Datatype> DatabaseLink::getDatatype() const { return 0; }

SemanticTree *DatabaseLink::toSTreeBase() const { return name->toSNodeDef(SCathegory::DatabaseLink, this); }

DatabaseLink::~DatabaseLink() {}


void table::FieldDefinition::owner(Table *_owner) { owner_ = _owner; }



void table::FieldDefinition::collectSNode(SemanticTree *n) const { n->addChild(toSTree()); }

SemanticTree *table::FieldDefinition::toSTreeBase() const {
  SemanticTree *n = name->toSNodeDef(SCathegory::Field, this);
  ANODE(datatype);
  CTREE(defaultExpr);
  return n;
}

table::FieldDefinition::~FieldDefinition() {}

void table::FieldDefinition::setNameDefinition() { name->definition((FieldDefinition*)this); }

bool table::FieldDefinition::isExactlyEquallyByDatatype(ResolvedEntity *ent) { return getDatatype()->isExactlyEqually(ent->getDatatype()); }

void table::FieldDefinition::foreignDatatype(Ptr<Datatype> t) { foreignDatatype_ = t; }

bool table::FieldDefinition::getFieldRef(Ptr<Id> &field) {
  if (datatype)
    return datatype->getFieldRef(field);
  return false;
}

bool table::FieldDefinition::isVarcharDatatype() const { return getDatatype()->isVarcharDatatype(); }
bool table::FieldDefinition::isCharDatatype() const { return getDatatype()->isCharDatatype(); }

Ptr<Datatype> table::FieldDefinition::getDatatype() const { return foreignDatatype_ ? foreignDatatype_ : datatype; }

Ptr<Datatype> table::FieldDefinition::getConcreteDatatype() const { return getDatatype(); }

Ptr<Id> table::FieldDefinition::getName() const { return name; }

Sm::IsSubtypeValues table::FieldDefinition::isSubtype(ResolvedEntity *supertype, bool isPlContext) const {
  if (supertype && datatype)
    return datatype->isSubtype(supertype, isPlContext);
  return EXPLICIT;
}

ResolvedEntity *table::FieldDefinition::getNextDefinition() const { return datatype.object(); }

bool table::FieldDefinition::getFields(EntityFields &fields) const { return datatype && datatype->getFields(fields); }

table::FieldDefinition::FieldDefinition(
    Ptr<Id> _name,
    Ptr<Datatype> _datatype,
    Ptr<SqlExpr> _defaultExpr,
    CLoc l,
    Ptr<EncryptSpec> _encrypt,
    bool _isSort,
    bool _isVirtualField,
    FieldDefinition::ResolvedProperties _resolvedProperties
  ) :
    GrammarBase(l),
    name(_name),
    datatype(_datatype),
    defaultExpr(_defaultExpr),
    encrypt(_encrypt),
    resolvedProperties(_resolvedProperties),
    isVirtualField(_isVirtualField),
    isSort(_isSort)
{
  sAssert(!name);
  setNameDefinition();
}

table::EncryptSpec::EncryptSpec() : salt(0) {}

table::EncryptSpec::EncryptSpec(CLoc l, Ptr<Id> encrypt, Ptr<Id> pass, bool _salt)
  : GrammarBaseSmart(l), encryptAlgorithm(encrypt), password(pass), salt(_salt) {}

alter_user::UserSettings::UserSettings() : settingsCathegory(EMPTY) {}

alter_user::UserSettings::UserSettings(CLoc l, Ptr<Id> _pass) : GrammarBaseSmart(l), password   (_pass    ), settingsCathegory(PASSWORD    ) {}

alter_user::UserSettings::UserSettings(CLoc l, Ptr<UserRoles> _dfltRole) : GrammarBaseSmart(l), defaultRole(_dfltRole), settingsCathegory(DEFAULT_ROLE) {}

alter_user::UserSettings::UserSettings(CLoc l, Ptr<Connect> _connect) : GrammarBaseSmart(l), connect    (_connect ), settingsCathegory(CONNECT     ) {}

SemanticTree *alter_user::UserSettings::toSTree() const {
  switch (settingsCathegory) {
    case DEFAULT_ROLE:
      return defaultRole ? defaultRole->toSTree() : (SemanticTree*)(0);
    case CONNECT:
      return connect ? connect->toSTree() : (SemanticTree*)(0);
    default:
      return 0;
  }
  return 0;
}

alter_user::Connect::Connect(CLoc l, connect::GrantOrRevoke _grntOrRevoke, Ptr<UserProxyConnect> _proxy)
  : GrammarBaseSmart(l), grantOrRevoke(_grntOrRevoke), proxy(_proxy) {}

SemanticTree *alter_user::Connect::toSTree() const { return proxy ? proxy->toSTree() : (Sm::SemanticTree*)(0); }

namespace alter_user {

UserProxyConnect::UserProxyConnect() : semanticNode(0), isAuthReq(false), connectType(THROUGH_USER) {}

UserProxyConnect::UserProxyConnect(CLoc l, Ptr<Id> _uname, Ptr<List<Id> > _roles, bool _isAuthReq)
  : GrammarBaseSmart(l), semanticNode(0), uname(_uname), roles(_roles), isAuthReq(_isAuthReq), connectType(THROUGH_USER) {}

SemanticTree *UserProxyConnect::toSTree() const {
  if (semanticNode)
    return 0;
  semanticNode = uname ? uname->toSNodeRef(SCathegory::User) : new SemanticTree();
  if (roles)
    for (List<Id>::const_iterator it = roles->begin(); it != roles->end(); ++it)
      semanticNode->addChild(it->object()->toSNodeRef(SCathegory::Role));
  return semanticNode;
}

void UserRole::collectSNode(SemanticTree *n) const { if (name) n->addChild(name->toSNodeRef(SCathegory::User)); }

UserRole::UserRole(CLoc l, Ptr<Id> _name, UserRole::RoleType t)
  : GrammarBaseSmart(l), name(_name), roleType(t) {}

UserRoles::UserRoles() : quantor(EMPTY) {}

UserRoles::UserRoles(CLoc l, UserRoles::Quantors _qantor, Ptr<List<UserRole> > _roleList)
  : GrammarBaseSmart(l) , semanticNode(0), quantor(_qantor), roleList(_roleList) {}

SemanticTree *UserRoles::toSTree() const {
  if (semanticNode)
    return 0;
  semanticNode = new SemanticTree();
  CollectSNode(semanticNode, roleList);
  return semanticNode;
}

}

namespace pragma {

Pragma::Pragma() {}
Pragma::Pragma(CLoc l) : GrammarBase(l) {}



void Pragma::collectSNode(SemanticTree *n) const { Declaration::collectSNode(n); }

Ptr<Id> Pragma::getName() const { return 0; }

void Pragma::resolve(Ptr<Id>) {}

void Pragma::collectSNodeM(SemanticTree *n) const { collectSNode(n); }

void Pragma::linterDefinition(Codestream &)  {}

}


Ptr<Datatype> NullExpr::getDatatype() const { return Datatype::mkNull();  }

}


SelectedField::SelectedField(CLoc l, Ptr<SqlExpr> expr, Ptr<Id> _alias)
  : GrammarBase(l), expr_(expr), alias_(_alias)
{
  if (!expr_)
    throw 999;

  if (alias_) {
    pseudoField = new QueryPseudoField(alias_, this, this);
    pseudoField->selectedFieldOwner = (SelectedField*)this;
    alias_->definition(this);
  }
  if (expr && !expr->isElementaryLiteral())
    if (Ptr<Id> name = expr_->getName())
      if (name->callArglistEmpty() && !name->isStringLiteral() && !name->isDateLiteral())
        fieldName = new Id(*name);

  if (!fieldName) {
    static int defFieldCnt = 0;
    fieldName = new Sm::Id("FIELD" + Sm::to_string(defFieldCnt++) + "__");
    fieldNameReallyEmpty = true;
  }
  fieldName->loc(getLLoc());
  fieldName->definition(this);
}

void Sm::Type::Object::traverseDeclarationsForce(DeclActor &fun)
{
  for (BaseList<MemberInterface>::value_type & v : elements)
    if (Sm::Type::MemberFunction *m = v->toSelfMemberFunction())
      m->traverseDeclarations(fun);
    else if (Sm::Type::MemberVariable *m = v->toSelfMemberVariable())
      m->traverseDeclarations(fun);
  for (BaseList<MemberFunction>::value_type & c : constructors)
    c->traverseDeclarations(fun);
  if (objectBody && objectBody.object() != this)
    objectBody->traverseDeclarations(fun);
}

void Sm::Type::Object::traverseDeclarations(DeclActor &fun) {
  if (fun(this))
    traverseDeclarationsForce(fun);
}

void Sm::Type::Object::replaceChildsIf(Sm::ExprTr tr) {
  for (BaseList<MemberInterface>::value_type & v : elements)
    if (Sm::Type::MemberFunction *m = v->toSelfMemberFunction())
      m->replaceChildsIf(tr);
  if (objectBody && objectBody.object() != this)
    objectBody->replaceChildsIf(tr);
}


void Sm::Type::Object::replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond) {
  for (BaseList<MemberInterface>::value_type & v : elements)
    if (Sm::Type::MemberFunction *m = v->toSelfMemberFunction())
      m->replaceStatementsIf(tr, cond);
  if (objectBody && objectBody.object() != this)
    objectBody->replaceStatementsIf(tr, cond);
}


void Sm::Datatype::replaceChildsIf(Sm::ExprTr tr)  { replace(tr, tid); }
void Sm::ArgumentNameRef::replaceChildsIf(Sm::ExprTr tr)  { replace(tr, id); }
void Sm::Constraint::replaceChildsIf(Sm::ExprTr tr) { replace(tr, name, attribute_); }
void Sm::View::replaceChildsIf(Sm::ExprTr tr)  { replace(tr, name, select, properties); }

void Package::setPackageAttributesForBlocks() {
  auto setPackageAttribute = [&](BlockPlSql *blk, BlockPlSql **bodyBlk) {
    if (blk) {
      blk->isNotPackageBlock_ = false;
      for (auto &d : blk->declarations)
        if (d)
          if (Sm::Variable *var = d->toSelfVariable()) {
            if (!*bodyBlk) {
              Ptr<BlockPlSql> body = new BlockPlSql();
              body->isNotPackageBlock_ = false;
              bodies.push_back(body);
              *bodyBlk = body.object();
            }
            var->setOwnerBlockPlSql(*bodyBlk);
            if (var->beginedFrom(178897))
                cout << "";
            var->flags.setGlobal();
          }
      blk->packageBody_ = *bodyBlk;
    }
  };
  BlockPlSql *bodyBlk = 0;
  if (!bodies.empty())
    bodyBlk = *(bodies.begin());

  for (Container::value_type &v : heads)
    setPackageAttribute(v, &bodyBlk);
  for (Container::value_type &v : bodies) {
    bodyBlk = v;
    setPackageAttribute(v, &bodyBlk);
  }
}


void Package::collectInitializers(EntitiesSet &container) {
  auto collectInBlocks = [&](Container &blks) {
    for (Container::value_type &v : blks) {
      v->isNotPackageBlock_ = false;
      v->collectInitializers(container);
      if (!v->empty())
        container.insert(this);
    }
  };
  collectInBlocks(heads );
  collectInBlocks(bodies);
}

void Package::traverseDeclarationsForce(DeclActor &tr)
{
  for (Container::value_type &v : heads)
    v->traverseDeclarations(tr);
  for (Container::value_type &v : bodies)
    v->traverseDeclarations(tr);
}

void Sm::Package::traverseDeclarations(DeclActor &tr) {
  if (tr(this))
    traverseDeclarationsForce(tr);
}

void Sm::Package::traverseModelStatements(StatementActor &fun) {
  for (Container::value_type &v : heads)
    v->traverseModelStatements(fun);
  for (Container::value_type &v : bodies)
    v->traverseModelStatements(fun);
}


void Sm::Package::replaceChildsIf(Sm::ExprTr tr) {
  for (Container::value_type &v : heads)
    v->replaceChildsIf(tr);
  for (Container::value_type &v : bodies)
    v->replaceChildsIf(tr);
}

void Sm::Package::replaceStatementsIf(Sm::StmtTr tr, Sm::StmtTrCond cond) {
  for (Container::iterator it = heads.begin(); it != heads.end(); ++it)
    (*it)->replaceBlockSubstatementsIf(tr, cond);
  for (Container::iterator it = bodies.begin(); it != bodies.end(); ++it)
    (*it)->replaceBlockSubstatementsIf(tr, cond);
}


void Sm::Trigger::traverseDeclarations(DeclActor &tr) {
  if (action)
    action->traverseDeclarations(tr);
}

void Sm::Trigger::replaceChildsIf(Sm::ExprTr tr) {
  replace(tr, events, whenCondition);
  if (action)
    action->replaceChildsIf(tr);
}

void Trigger::replaceStatementsIf(StmtTr tr, StmtTrCond cond) {
  if (action)
    action->replaceStatementsIf(tr, cond);
}

void Sm::FunCallArgAsterisk::replaceChildsIf(ExprTr tr) {
  replace(tr, _expr);
}

void Sm::Trigger::collectInitializers(EntitiesSet &container) {
  if (action)
    if (BlockPlSql *blk = action->toSelfBlockPlSql())
      blk->collectInitializers(container);
}


void Sm::FunCallArgExpr::replaceChildsIf(ExprTr tr) {
  replace(tr, _expr);
}

void Sm::FunCallArgNamed::replaceChildsIf(ExprTr tr) {
  replace(tr, _name, _expr);
}

namespace Sm {

template <>
void replace(ExprTr tr, Ptr<Id> &id) {
  if (id && id->callArglist)
    replace(tr, id->callArglist);
}

template <>
void replace(ExprTr tr, Ptr<Id2> &id) {
  if (id) {
    replace(tr, id->id[0]);
    replace(tr, id->id[1]);
  }
}

template <>
void replace(ExprTr tr, Ptr<IdEntitySmart> &ent) {
  for (Ptr<Id> &i : *ent)
    replace(tr, i);
}

template <>
void replace(ExprTr tr, Ptr<Datatype> &t) {
  if (t)
    replace(tr, t->tid);
}

ResolvedEntity *Type::RecordField::getFieldDDLContainer() const {
  if (SemanticTree *n = getSemanticNode())
    if (SemanticTree *p = n->getParent())
      if (ResolvedEntity *d = p->ddlEntity())
        if (d->toSelfRecord())
          return d;
  return 0;
}

}

string SysUser::getSelfName() { return "SYS"; }

SysUser::SysUser(ModelContext &model)
  : UserContext(new Sm::Id(SysUser::getSelfName()))
{
  model.parent->model = &model;
  sysrefcursor = new Sm::SysRefcursor(getName());
  types["SYS_REFCURSOR"] = sysrefcursor.object();
  model.addSynonym(new Sm::Synonym(new Id2(new Id(sysrefcursor->getName()->toNormalizedString())), sysrefcursor->getName2(), true));

  Ptr<Table> dual = new Sm::SysDual();
  tables["DUAL"] = dual;
  model.addSynonym(new Sm::Synonym(new Id2(new Id(dual->getName()->toNormalizedString())), dual->getName2(), true));
  Ptr<Function> f = DynamicFuncallTranslator::createTranslatorFunction();
  functions[f->getName()->toNormalizedString()] = f;
  f = DynamicFuncallTranslator::createSignatureTranslatorFunction();
  functions[f->getName()->toNormalizedString()] = f;
}

SystemTable::SystemTable(Ptr<Id2> _name)
{
  name = _name;
  setNameDefinition();
  relationFields = new RelationFields();
  setIsSystem();
}

void SystemTable::addField(const string &name, Ptr<Datatype> t)
{
  Ptr<table::FieldDefinition> field = new table::FieldDefinition(new Id(string(name)), t);
  field->owner((Table*)this);
  field->setIsSystem();
  this->relationFields->push_back(field);
}

Sm::SysRefcursor::SysRefcursor(Ptr<Id> owner)
  : Object(cl::emptyFLocation(), new Id2("SYS_REFCURSOR", owner), 0) {}

Ptr<Sm::Type::Object> SysUser::anydata() const {
  if (!anydata_) {
    UserContext::Types::const_iterator it = types.find("ANYDATA");
    if (it != types.end())
      anydata_ = it->second->toSelfAnydata();
  }
  return anydata_;
}

Ptr<Sm::Type::Object> SysUser::xmltype() const {
  if (!xmltype_) {
    UserContext::Types::const_iterator it = types.find("XMLTYPE");
    if (it != types.end())
      xmltype_ = it->second->toSelfObject();
  }
  return xmltype_;
}


Sm::SysDual::SysDual() : SystemTable(new Id2(new Id("DUAL"))), dummy("DUMMY") {
  addField("DUMMY", Sm::Datatype::mkVarchar2(1));
}

// vim:foldmethod=syntax
