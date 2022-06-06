#include "semantic_utility.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "semantic_datatype.h"
#include "semantic_id.h"
#include "semantic_function.h"
#include "parserdfa.h"
#include "semantic_id_lists.h"


using namespace Sm;

extern SyntaxerContext syntaxerContext;


bool IdEntitySmart::updateQuotedUserIdToVarchar2Literal() {
  return size() == 1 && front()->updateQuotedUserIdToVarchar2Literal();
}

bool IdEntitySmart::updateUnresolvedQuotedToVarchar2Literal() {
  return size() == 1 && front()->updateQuotedUserIdToVarchar2Literal();
}

size_t Id::bSize(SmartptrSet &s) const {
  if (!s.insert(static_cast<const Smart*>(this)).second)
    return 0;
  return sizeof(Id) +
//      (callArglist ? callArglist->bSize(s) : 0) +
      getContainerSize(*text) + getContainerSize(normalizedStr);
}


size_t IdEntitySmart::bSize(SmartptrSet &s) const {
  if (!s.insert(static_cast<const Smart*>(this)).second)
    return 0;
  size_t sz = sizeof(IdEntitySmart) +
         getContainerSize(*this) * sizeof(value_type);
  for (const value_type &v : *this)
    sz += v->bSize(s);
  return sz;
}

Ptr<CallArgList> Id::getConsistantCallArglist() {
  if (!callArglist)
    callArglist = new CallArgList();
  return callArglist;
}

void Id::pushFrontCallarglist(const Ptr<Id> &id, bool setSelf/*= true*/) {
  if (!callArglist)
    callArglist = new CallArgList();

  callArglist->insert(callArglist->begin(), new Sm::FunCallArgExpr(new Sm::RefExpr(id)));
  if (setSelf)
    callArglist->front()->setSelf();
}

void Id::pushFrontCallarglist(const Ptr<IdEntitySmart> &reference, bool setSelf/*= true*/) {
  if (!callArglist)
    callArglist = new CallArgList();

  callArglist->insert(callArglist->begin(), new Sm::FunCallArgExpr(new Sm::RefExpr(reference)));
  if (setSelf)
    callArglist->front()->setSelf();
}

bool Id::quotedInvariant(bool proc) const {
  struct InvariantCathegories : public CathegoriesOfDefinitions {
    const std::unordered_set<CathegoriesOfDefinitions::ScopedEntities> allowedInvariants = {
      Sequence_, Table_, View_, User_, SqlSelectedField_, QueriedPseudoField_, SqlExprCursor_,
      Variable_, VariableUndeclaredIndex_, TriggerRowReference_, TriggerNestedRowReference_,
      Package_, Function_, LinterCursorField_, FieldOfRecord_, FieldOfTable_, FunctionArgument_,
      MemberFunction_, Record_, RefCursor_, Varray_, NestedTable_, Subtype_, Object_, FactoringItem_, FromSingle_, FromJoin_
    };

    InvariantCathegories() {}
  };
  static const InvariantCathegories cat;

  if (attributes.v & (proc ? FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC : FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL))
    return false;
  else if (attributes.v & (proc ? FLAG_ID_IS_INVARIANT_QUOTE_PROC : FLAG_ID_IS_INVARIANT_QUOTE_SQL))
    return true;

  if ((attributes.v & (FLAG_ID_IS_SQUOTED              |  // squoted()
                       FLAG_ID_IS_STRING_LITERAL       |  // isStringLiteral()
                       FLAG_ID_IS_DATELITERAL          |  // isDateLiteral()
                       FLAG_ID_IS_EMPTY                |  // isEmptyId
                       FLAG_ID_HAS_CP1251_SPEC_SYMBOLS |
                       FLAG_ID_IS_RESERVED_FIELD       |
                       FLAG_ID_NEED_TO_DQUOTING  // <- устанавливается только если FieldDefinition - зарезервированное слово
                       )) || isReservedField() || (!empty() && isdigit(text->front())))
  {
    attributes.v |= (FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC | FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL);
    return false;
  }

  bool notInvariantInProc = hasDollarSymbols();
  bool notInvariantInSql  = hasSquareSymbols();
  if (notInvariantInProc)
    attributes.v |= FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC;

  if (notInvariantInSql)
    attributes.v |= FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL;

  if (notInvariantInProc && notInvariantInSql)
    return false;

  ResolvedEntity *def = definition();
  if (def)
    def = def->getConcreteDefinition();

  //if (text->empty()) // должно обрабатываться ранее за счет флага FLAG_ID_IS_EMPTY
  //  throw 999;

  if (def && !cat.allowedInvariants.count(def->ddlCathegory())) {
    attributes.v |= (FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL | FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC);
    return false;
  }
  // инварианты могут иметь либо неотрезолвленные сущности
  // либо отрезолвленные и одновременно допустимые по своей категории
  if (quoted()) {
    string s = *text;
    transform(s.begin(), s.end(), s.begin(), ::toupper);
    if (s != *text) {
      attributes.v |= (FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL | FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC);
      return false;
    }
  }
  else
    cout << "";
  if (!notInvariantInProc)
    attributes.v |= FLAG_ID_IS_INVARIANT_QUOTE_PROC;
  if (!notInvariantInSql)
    attributes.v |= FLAG_ID_IS_INVARIANT_QUOTE_SQL;
  return proc ? !notInvariantInProc : !notInvariantInSql;
}


void Id::toQString(std::string &dst, bool isProcMode) const {
  if (beginedFrom(777678))
    cout << "";
  if (quoted()) {
    if (!isDateLiteral()) // литералы даты должны добавляться без закавычивания
      return toQuotedNondateLiteral(dst, isProcMode);
  }
  else if (!isProcMode) {
    if (attributes.v & (FLAG_ID_NEED_TO_DQUOTING | FLAG_ID_HAS_SQUARE_SYMBOL))
      return toDQNormalized(dst);
    else if (isSqlIdentificator() && isReservedField()) // ключевое слово-идентификатор в кодогенерации SQL
      return PlsqlHelper::quotingAndEscaping(dst, normalizedStr, '\"');
  }
  else if (needQuotingInProcMode()) { // доллар или ключевое слово - как поле таблицы в кодогенерации процедур
    dst = toNormalizedString() + "_";
    return;
  }

  dst.append(*text);
}




bool Id::updateQuotedUserIdToVarchar2Literal() {
  return idDefinition_ &&
         idDefinition_->ddlCathegory() == ResolvedEntity::User_ &&
         updateUnresolvedQuotedToVarchar2Literal();
}

bool Id::updateUnresolvedQuotedToVarchar2Literal() {
  if (quoted()) {
    setStringLiteral();
    definition(Datatype::mkVarchar2(text->length()));
    return true;
  }
  return false;
}

bool Id::hasNoncursorCallSemantic() const {
  return idDefinition_  && (idDefinition_->toSelfFunction() || (callArglist && idDefinition_->ddlCathegory() != ResolvedEntity::Cursor_));
}

Sm::IsSubtypeValues Id::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (ResolvedEntity *def = definition())
    return def->isSubtype(supertype, plContext);
  return Sm::IsSubtypeValues::EXPLICIT;
}

SemanticTree *Id::toSNodeRef(SCathegory::t t, const ResolvedEntity *d) const {
  return new SemanticTree((Id*)this, SemanticTree::REFERENCE, t, (ResolvedEntity*)d);
}

SemanticTree *Id::toSNodeDecl(SCathegory::t t, const ResolvedEntity *d) const {
  if (d)
    ((Id*)this)->definition(d);
  SemanticTree * n = new SemanticTree((Id*)this, SemanticTree::DECLARATION, t, (ResolvedEntity*)d);
  n->unnamedDdlEntity = (ResolvedEntity*)d;
  return n;
}

SemanticTree *Id::toSNodeDef(SCathegory::t t, const ResolvedEntity *d) const {
  if (d)
    ((Id*)this)->definition(d);
  SemanticTree *n = new SemanticTree((Id*)this, SemanticTree::DEFINITION, t, idDefinition_);
  n->unnamedDdlEntity = (ResolvedEntity*)d;
  return n;
}

Id::Id(const FLoc &l, const string &_text, initializer_list<PlExpr* > _callArgList, ResolvedEntity *def)
  : Base(),
    idDefinition_(def),
    lloc(l),
    hashValue(getHashValue(_text, false)),
    text(getCachedTextPtr(_text.c_str(), _text.size()))
{
  if (_callArgList.size()) {
    callArglist = new CallArgList();
    for (PlExpr *v : _callArgList)
      callArglist->push_back(new FunCallArgExpr(l, Ptr<PlExpr>(v)));
  }
}

Id::Id(const FLoc &l, const string &_text, initializer_list<Ptr<PlExpr> > _callArgList, ResolvedEntity *def)
  : Base(),
    idDefinition_(def),
    lloc(l),
    hashValue(getHashValue(_text, false)),
    text(getCachedTextPtr(_text.c_str(), _text.size()))
{
  if (_callArgList.size()) {
    callArglist = new CallArgList();
    for (const Ptr<PlExpr> &v : _callArgList)
      callArglist->push_back(new FunCallArgExpr(l, v));
  }
}

void Id::setCalArglistAndUpdateLoc(CLoc &l, CallArgList *cList) {
  lloc.loc.end = l.loc.end;
  callArglist = cList;
}


bool Id2::getFields(EntityFields &fields) {
  if (ResolvedEntity *def = definition())
    return def->getFields(fields);
  return false;
}

ResolvedEntity *Id2::definition() const {
  Id* i = id[0].object();
  if (Id* usr = uname()) {
    ResolvedEntity *usrDef = usr->unresolvedDefinition();
    if (!usrDef) {
      usrDef = syntaxerContext.model->getUser(usr);
      usr->definition(usrDef);
    }
    if (!i->unresolvedDefinition() && usrDef) {
      Ptr<Id> uid = i;
      usrDef->getFieldRef(uid);
      return i->definition();
    }
  }
  return i ? i->definition() : nullptr;
}


void Id2::toIdEntityDeepCopy(IdEntitySmart &entity) const {
  if (id[0]) {
    entity.push_back(new Id(*(id[0])));
    if (id[1])
      entity.push_back(new Id(*(id[1])));
  }
}

Ptr<IdEntitySmart> Id2::toIdEntityDeepCopy() const {
  Ptr<IdEntitySmart> entity = new IdEntitySmart;
  toIdEntityDeepCopy(*entity);
  return entity;
}

void Id2::toIdEntity(IdEntitySmart &entity) const {
  if (id[0]) {
    entity.push_back(id[0]);
    if (id[1])
      entity.push_back(id[1]);
  }
}

Ptr<IdEntitySmart> Id2::toIdEntity() const {
  Ptr<IdEntitySmart> entity = new IdEntitySmart;
  toIdEntity(*entity);
  return entity;
}

IsSubtypeValues Id2::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (ResolvedEntity *def = definition())
    return def->isSubtype(supertype, plContext);
  return Sm::IsSubtypeValues::EXPLICIT;
}

void Id2::semanticNode(const SemanticTree *node) {
  if (id[0])
    id[0]->semanticNode(node);
  if (id[1])
    id[1]->semanticNode(node);
}

SemanticTree *Id2::toSNodeDef(SCathegory::t t, const ResolvedEntity *d) const {
  if (d)
    ((Id*)this)->definition(d);
  SemanticTree * n = new SemanticTree((Id2*)this, SemanticTree::DEFINITION, t, (ResolvedEntity*)d);
  n->unnamedDdlEntity = (ResolvedEntity*)d;
  return n;
}

SemanticTree *Id2::toSNodeDecl(SCathegory::t t, const ResolvedEntity *d) const {
  if (d)
    ((Id*)this)->definition(d);
  SemanticTree * n = new SemanticTree((Id2*)this, SemanticTree::DECLARATION, t, (ResolvedEntity*)d);
  n->unnamedDdlEntity = (ResolvedEntity*)d;
  return n;
}

SemanticTree *Id2::toSNodeRef(SCathegory::t t, const ResolvedEntity *d) const {
  return new SemanticTree((Id2*)this, SemanticTree::REFERENCE, t, (ResolvedEntity*)d);
}

SemanticTree *Id2::toSNodeDatatypeRef(SCathegory::t t) const {
  return new SemanticTree((Id2*)this, SemanticTree::DATATYPE_REFERENCE, t, 0);
}



size_t Id2::bSize(SmartptrSet &s) const {
  if (!s.insert(this).second)
    return 0;
  return sizeof(Id2) + (id[0] ? id[0]->bSize(s) : 0) + (id[1] ? id[1]->bSize(s) : 0);
  }

  Ptr<Datatype> Id2::getDatatype() {
  if (ResolvedEntity *def = definition())
  return def->getDatatype();
  return 0;
}

bool Id2::getFieldRef(Ptr<Id> &field) {
  if (ResolvedEntity *def = definition())
    return def->getFieldRef(field);
  return false;
}



string Id2::toInvariantQString(bool proc) const {
  string dst;
  if (id[1]) { // size = 2
    dst += id[1]->toQInvariantNormalizedString(proc);
    dst += '.';
  }
  if (id[0]) // size = 1
    dst += id[0]->toQInvariantNormalizedString(proc);
  return dst;
}

Ptr<Datatype> Id2::getDatatype() const {
  if (ResolvedEntity *def = definition())
    return def->getDatatype();
  return 0;
}

ResolvedEntity* Id2::getNextDefinition() const {
  if (ResolvedEntity *def = definition())
    return def->getNextDefinition();
  return 0;
}

ResolvedEntity *Id::definition() const {
  if (!idDefinition_)
    if (SemanticTree *node = semanticNode_) {
      if (node->unentered())
        node->resolveReference();
      else if (node->childsNotResolved() && !callArglist) {
        node->resolveCurrent();
      }
    }
  return idDefinition_;
}

Id *Id::def(const ResolvedEntity *d) {
  definition(d);
  return (Id*)this;
}

bool Id::isReservedField() const {
  if (attributes.v & FLAG_ID_IS_NOT_RESERVED_FIELD)
    return false;
  else if (attributes.v & FLAG_ID_IS_RESERVED_FIELD)
    return true;
  else if (isFieldReserved(toNormalizedString())) {
    attributes.v |= FLAG_ID_IS_RESERVED_FIELD;
    return true;
  }
  attributes.v |= FLAG_ID_IS_NOT_RESERVED_FIELD;
  return false;
}

size_t Sm::Function::getGlobalFunId() const {
  static size_t id = 0;
  ++id;
  if (id == 28495)
    cout << "";
  return id;
}

string Sm::Id::toQInvariantNormalizedString(bool proc) const {
  string str;
  toQInvariantNormalizedString(str, proc);
  return str;
}

void Sm::Id::toQInvariantNormalizedString(string &s, bool proc) const {
  if (quotedInvariant(proc))
    s = *text;
  else
    toQString(s, proc);

}

FunCallArgExpr::FunCallArgExpr(PlExpr *e)
  : _expr(e)
{
  if (!_expr)
    throw 999;
  loc(_expr->getLLoc());
}

Ptr<FunCallArg> FunCallArgExpr::deepCopy() {
  return new FunCallArgExpr(getLLoc(), _expr->deepCopy());
}

COMPILER_LEXER_OPTIMIZATION_PUSH()




string* Id::getCachedTextPtrOnLexer(uint hVal) {
  if (beginedFrom(41803,4))
    cout << "";
  return cachedIds.insert(hVal, lexer_id_text.ptr(), lexer_id_text.size(), attributes.v);
}

string *Id::getCachedTextPtrOnLexer() {
  if (beginedFrom(41803))
    cout << "";
  char *ptr = lexer_id_text.ptr();
  unsigned int size = lexer_id_text.size();
  return cachedIds.insert(BackportHashMap::internal::HashFunctions::cchar_hash(ptr, size), ptr, size, attributes.v);
}



string *Id::getCachedTextPtr(const char *str, size_t length) {
  if (beginedFrom(41803))
    cout << "";
  return cachedIds.insert(BackportHashMap::internal::HashFunctions::cchar_hash(str, length),
                          str, length, attributes.v);
}

string* Id::getCachedTextPtr(uint hVal, const char *str, size_t length) {
  if (beginedFrom(41803))
    cout << "";
  return cachedIds.insert(hVal, str, length, attributes.v);
}


string* Id::getCachedTextPtr(string &str) {
  if (beginedFrom(41803))
    cout << "";
  return cachedIds.insert(BackportHashMap::internal::HashFunctions::cchar_hash(str.data(), str.size()),
                          str, attributes.v);
}

string* Id::getCachedTextPtr(uint hVal, string str) {
  if (beginedFrom(41803))
    cout << "";
  return cachedIds.insert(hVal, str, attributes.v);
}

bool Id::isFieldReserved(const string &s) { return syntaxerContext.s_reservedFields.count(s); }


void CachedIdentificators::FastIdHashMap::setShift(int shift) {
  /* Каждый размер таблицы ассоциируется с простым модулем (первые простые числа
     * меньше, чем размер таблицы) used to find the initial bucket. Probing
     * then works modulo 2^n. Простой модуль необходим для получения хорошего
     * распределения при плохих хэш-функциях.
     */
  static const int prime_mod [] = {
    1         , /* 1 <<  0 */
    2         , /* 1 <<  1 */
    3         , /* 1 <<  2 */
    7         , /* 1 <<  3 */
    13        , /* 1 <<  4 */
    31        , /* 1 <<  5 */
    61        , /* 1 <<  6 */
    127       , /* 1 <<  7 */
    251       , /* 1 <<  8 */
    509       , /* 1 <<  9 */
    1021      , /* 1 << 10 */
    2039      , /* 1 << 11 */
    4093      , /* 1 << 12 */
    8191      , /* 1 << 13 */
    16381     , /* 1 << 14 */
    32749     , /* 1 << 15 */
    65521     , /* 1 << 16 */
    131071    , /* 1 << 17 */
    262139    , /* 1 << 18 */
    524287    , /* 1 << 19 */
    1048573   , /* 1 << 20 */
    2097143   , /* 1 << 21 */
    4194301   , /* 1 << 22 */
    8388593   , /* 1 << 23 */
    16777213  , /* 1 << 24 */
    33554393  , /* 1 << 25 */
    67108859  , /* 1 << 26 */
    134217689 , /* 1 << 27 */
    268435399 , /* 1 << 28 */
    536870909 , /* 1 << 29 */
    1073741789, /* 1 << 30 */
    2147483647  /* 1 << 31 */
  };

  reservedSize = 1 << shift      ;
  mod         = prime_mod[shift];
  mask        = ~((uint)-1 << shift); /* shift единиц от младшего разряда */
}

CachedIdentificators::FastIdHashMap::FastIdHashMap() {
  setShift(18); // 18 - означает, что даже сжатых идентификаторов очень много

  values = new Value[reservedSize];
  hashes = new uint[ reservedSize ];
  memset(hashes, 0, reservedSize * sizeof(uint));
}

void CachedIdentificators::FastIdHashMap::cleanup()
{
  if (!cleaned) {
    delete[] values;
    delete[] hashes;
  }
  cleaned = true;
}

CachedIdentificators::FastIdHashMap::~FastIdHashMap() {
  cleanup();
}

uint CachedIdentificators::FastIdHashMap::lookupNode(uint key_hash, const char *ptr, size_t length) const {
  uint step = 0;

  if (!key_hash) // хэш недействителен
    throw 999;

  uint node_index = key_hash % mod;
  uint node_hash  = hashes[node_index];

  /* хэш действителен, пока выбираемые в таблице ячейки не пустые */
  while (node_hash) {
    if (node_hash == key_hash) {
      string &node = values[node_index].first;
      if (node.size() == length && !memcmp(node.c_str(), ptr, length))
        return node_index;
    }

    node_index += ++step;
    node_index &= mask;
    node_hash   = hashes[node_index];
  }

  return node_index;
}

void CachedIdentificators::FastIdHashMap::setSpecIdFlags(const char *ptr, size_t length, uint &dstFlags, uint &valueFlags)
{
  if (dstFlags & FLAG_ID_IS_KEYWORD_TOKEN)
    return;

  unsigned int f = 0;
  const unsigned char *it = (unsigned char*)ptr;
  for (const unsigned char *end = it + length; it != end; ++it) {
    unsigned int i = *it;
    f |= LexerDFA::LexerDfa::acceptIdTable.flags[i];
  }

  if (f) {
    dstFlags   |= f;
    valueFlags |= f;
  }
}

string *CachedIdentificators::FastIdHashMap::insert(uint key_hash, const char *ptr, size_t length, uint &dstFlags) {
  uint node_index = lookupNode(key_hash, ptr, length);

  uint &hashItem = hashes[node_index];
  if (hashItem)  // значение уже существует
    return returnExisted(dstFlags, node_index);
  else {
    checkStaticSize();

    hashItem = key_hash;
    Value &v = values[node_index];

    setSpecIdFlags(ptr, length, dstFlags, v.second);

    string *s;
    {
      s = &v.first;
      string tmp(ptr, length);
      s->swap(tmp);
      tmp.clear();
    }

    ++size_;
    return s;
  }
}

string *CachedIdentificators::FastIdHashMap::insert(uint key_hash, string &str, uint &dstFlags) {
  const char *ptr = str.data();
  size_t length = str.size();
  uint node_index = lookupNode(key_hash, ptr, length);

  uint &hashItem = hashes[node_index];
  if (hashItem)  // значение уже существует
    return returnExisted(dstFlags, node_index);
  else {
    checkStaticSize();

    hashItem = key_hash;
    Value &v = values[node_index];

    setSpecIdFlags(ptr, length, dstFlags, v.second);

    string *s = &v.first;
    s->swap(str);

    ++size_;
    return s;
  }

}






COMPILER_LEXER_OPTIMIZATION_POP()
