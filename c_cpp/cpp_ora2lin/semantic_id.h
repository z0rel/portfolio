#ifndef SEMANTIC_ID_H
#define SEMANTIC_ID_H

#include <memory>
#include "siter.h"
#include "synlib.h"
#include "semantic_tree.h"
#include "semantic_base.h"

extern LexStringBuffer lexer_id_text;

namespace Sm {


struct FunCallArg  : public ResolvedEntityLoc {
  enum Argclass { ASTERISK, POSITIONAL, NAMED };

  virtual Ptr<Id> argname() const;
  virtual Ptr<PlExpr> expr() const  = 0; // { return _expr.object(); }
  virtual void setExpr(PlExpr *) = 0; // { return _expr.object(); }
  virtual Argclass    argclass() const  = 0; // { return _t; }

  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  ScopedEntities ddlCathegory() const { return FunctionCallArgument_; }

  Ptr<Sm::Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  Ptr<Id> getName() const;

  virtual bool isAsterisk() const;
  virtual void setArgPositionInDef(int) {}
  virtual int getArgPositionInDef(int pos) { return pos; }

  bool semanticResolve() const;

  bool getFieldRef(Ptr<Sm::Id> &field);
  ResolvedEntity* getNextDefinition() const;
  bool getFields(EntityFields &fields) const;

  void oracleDefinition(Sm::Codestream &str);
  virtual void sqlDefinition   (Sm::Codestream &s)  = 0;
  virtual void linterDefinition(Sm::Codestream &s)  = 0;
  void sqlReference    (Sm::Codestream &s) { sqlDefinition(s); }

  inline bool isSelf() const { return __flags__.v &  FLAG_FUNCTION_CALL_ARGUMENT_IS_SELF; }
  inline void setSelf()             { __flags__.v |= FLAG_FUNCTION_CALL_ARGUMENT_IS_SELF; }

  FunCallArg() {}

  virtual void collectSNode(SemanticTree *) const = 0;
  virtual ~FunCallArg();

  virtual void replaceChildsIf(ExprTr tr) = 0;
  virtual Ptr<FunCallArg> deepCopy() { throw 999; return 0; }
};


#define  FLAG_ID_IS_QUOTED                   (1 << 0)
#define  FLAG_ID_IS_SQUOTED                  (1 << 1)
#define  FLAG_ID_IS_HOST_ID                  (1 << 2)
#define  FLAG_ID_IS_EMPTY                    (1 << 3)
#define  FLAG_ID_IS_CALLED_ARGUMENT          (1 << 4)
#define  FLAG_ID_HAS_SQUARE_SYMBOL           (1 << 5)
#define  FLAG_ID_HAS_DOLLAR_SYMBOL           (1 << 6)
#define  FLAG_ID_HAS_CP1251_SPEC_SYMBOLS     (1 << 7)
#define  FLAG_ID_HAS_SPEC_SYMBOL             (FLAG_ID_HAS_SQUARE_SYMBOL | FLAG_ID_HAS_DOLLAR_SYMBOL)
#define  FLAG_ID_IS_STRING_LITERAL           (1 << 8)
#define  FLAG_ID_IS_CONTEXT_LEVEL_UP         (1 << 9)
#define  FLAG_ID_IS_DATELITERAL              (1 << 10)
#define  FLAG_ID_SELF_ALREADY_PUSHED         (1 << 11)
#define  FLAG_IS_ROWID_PSEUDOCOL             (1 << 12)
#define  FLAG_IS_ROWNUM_PSEUDOCOL            (1 << 13)
#define  FLAG_ID_NEED_TO_DQUOTING            (1 << 14)
#define  FLAG_ID_IS_RESERVED_FIELD           (1 << 15)
#define  FLAG_ID_IS_NOT_RESERVED_FIELD       (1 << 16)
#define  FLAG_ID_IS_INVARIANT_QUOTE_SQL      (1 << 17)
#define  FLAG_ID_IS_NOT_INVARIANT_QUOTE_SQL  (1 << 18)
#define  FLAG_ID_IS_INVARIANT_QUOTE_PROC     (1 << 19)
#define  FLAG_ID_IS_NOT_INVARIANT_QUOTE_PROC (1 << 20)
#define  FLAG_ID_IS_KEYWORD_TOKEN            (1 << 21)
#define  FLAG_ID_IS_DBLINK                   (1 << 22)
#define  FLAG_ID_SKIP_FUNCTION_RESOLVING     (1 << 23)
#define  FLAG_ID_RESOLVED_FUNCTION           (1 << 24)

struct IdFlags {
  uint IS_QUOTED                   :1;
  uint IS_SQUOTED                  :1;
  uint IS_HOST_ID                  :1;
  uint IS_EMPTY                    :1;
  uint IS_CALLED_ARGUMENT          :1;
  uint HAS_SQUARE_SYMBOL           :1;
  uint HAS_DOLLAR_SYMBOL           :1;
  uint HAS_CP1251_SPEC_SYMBOLS     :1;
  uint IS_STRING_LITERAL           :1;
  uint IS_CONTEXT_LEVEL_UP         :1;
  uint IS_DATELITERAL              :1;
  uint SELF_ALREADY_PUSHED         :1;
  uint ROWID_PSEUDOCOL             :1;
  uint ROWNUM_PSEUDOCOL            :1;
  uint NEED_TO_DQUOTING            :1;
  uint IS_RESERVED_FIELD           :1;
  uint IS_NOT_RESERVED_FIELD       :1;
  uint IS_INVARIANT_QUOTE_SQL      :1;
  uint IS_NOT_INVARIANT_QUOTE_SQL  :1;
  uint IS_INVARIANT_QUOTE_PROC     :1;
  uint IS_NOT_INVARIANT_QUOTE_PROC :1;
  uint IS_KEYWORD_TOKEN            :1;
  uint IS_DBLINK                   :1;
  uint IS_SKIP_FUNCTION_RESOLVING  :1;
} __attribute__((packed));

//inline std::ostream &operator<<(std::ostream &os, const IdFlags &a) {
//  os << "isQuoted         = " << a.isQuoted << endl
//     << "isSQuoted        = " << a.isSQuoted << endl
//     << "isHostId         = " << a.isHostId << endl
//     << "isEmpty          = " << a.isEmpty << endl
//     << "isCalledArgument = " << a.isCalledArgument << endl
//     << "hasSpecSymbols   = " << a.hasSpecSymbols << endl
//     << "isStringLiteral  = " << a.isStringLiteral << endl
//     << "contextLevelUp   = " << a.contextLevelUp << endl;
//  return os;
//}

union IdAttributes {
  typedef unsigned int Value;

  Value v;
  IdFlags a;

  IdAttributes() : v(0) {}
  IdAttributes(const IdAttributes &o) : v(o.v) {}
  IdAttributes(unsigned int val) : v(val) {}
  IdAttributes(int isQuoted, bool isEmpty) : v((isQuoted ? FLAG_ID_IS_QUOTED : 0 ) | (isEmpty ? FLAG_ID_IS_EMPTY : 0)) {}

  inline void setQuoted() { v |= FLAG_ID_IS_QUOTED; }
  inline void setIsEmpty() { v |= FLAG_ID_IS_EMPTY; }
};

namespace CachedIdentificators {

struct ItemsDestructor {
  ItemsDestructor() {}
  ~ItemsDestructor();
};

COMPILER_LEXER_OPTIMIZATION_PUSH()



class FastIdHashMap {
public:
  typedef pair<string, unsigned int> Value;
private:
  int       reservedSize; /* init in setShift */
  int       mod;         /* init in setShift */
  uint      mask;        /* init in setShift */
  /// Количество добавленных узлов
  int       size_ = 0;

  uint     *hashes;
  Value    *values;

  bool      cleaned  = false;


  void setShift(int shift);

  uint lookupNode(uint key_hash, const char *ptr, size_t length) const;

  inline string* returnExisted(uint &dstFlags, uint node_index) {
    Value &v = values[node_index];
    dstFlags |= v.second;
    return &(v.first);
  }

  inline void checkStaticSize() {
    if ((reservedSize - size_) < size_ / 16)
      throw 999;
  }

  void setSpecIdFlags(const char *ptr, size_t length, uint &dstFlags, uint &valueFlags);
public:
  FastIdHashMap();
  ~FastIdHashMap();

  void cleanup();

  string *insert(uint key_hash, const char *ptr, size_t length, uint &dstFlags);
  string *insert(uint key_hash, string &str, uint &dstFlags);
};


COMPILER_LEXER_OPTIMIZATION_POP()

}




class Id                : public SingleSmart, public CathegoriesOfDefinitions
{ /* сегмент имени с признаком закавыченности (<0 => -TOKEN) */
  friend Codestream& operator<< (Codestream& os, const Id& str);

  string* getCachedTextPtrOnLexer(uint hVal);
  string* getCachedTextPtrOnLexer();
  string* getCachedTextPtr(const char *str, size_t length);
  string* getCachedTextPtr(uint hVal, const char *str, size_t length);

  string* getCachedTextPtr(string &str);
  string* getCachedTextPtr(uint hVal, string str);

public:
  static CachedIdentificators::FastIdHashMap cachedIds;
  static const string emptyText;

  typedef SingleSmart Base; // 4
protected:
  ResolvedEntity       *idDefinition_ = 0; // 8
public:
  Ptr<CallArgList>      callArglist;   // 8
protected:
  FLoc                  lloc = cl::emptyFLocation(); // 32
  uint                  hashValue = BackportHashMap::emptyStringHash(); // 4
  mutable IdAttributes  attributes;     // 4
  const string         *text = &emptyText;           // 8
  SemanticTree         *semanticNode_ = 0; // 8
  mutable string        normalizedStr;  // 8

//  size_t __globalId = __getGlobalId();
//  ~Id();
  size_t __getGlobalId();

  void setNormalized() const {
    normalizedStr = *text;
    transform(normalizedStr.begin(), normalizedStr.end(), normalizedStr.begin(), ::toupper);
  }
  void toDQNormalized(std::string &dst) const {
    if (normalizedStr.empty())
      setNormalized();
    PlsqlHelper::quotingAndEscaping(dst, normalizedStr, '\"');
  }
  bool isSqlIdentificator() const;


public:
  static bool isFieldReserved(const std::string &s);

public:
  inline std::string::size_type length() const { return text->size(); }
  size_t bSize(SmartptrSet &s) const;

  /// Получить список аргументов вызова или создать его если он не существует
  Ptr<CallArgList> getConsistantCallArglist();
  void pushFrontCallarglist(const Ptr<Id> &id, bool setSelf = true);
  void pushFrontCallarglist(const Ptr<IdEntitySmart> &reference, bool setSelf = true);

  bool quotedInvariant(bool proc) const;
  static uint getHashValue(const string &str, bool isQuoted);
  static uint getHashValue(char *_text, size_t length, bool isQuoted);

  IdAttributes::Value getAttributesValue() const { return attributes.v; }

  void setRownumPseudocol() { attributes.v |=  FLAG_IS_ROWNUM_PSEUDOCOL; }
  bool isRownumPseudocol() const { return attributes.v & FLAG_IS_ROWNUM_PSEUDOCOL; }

  void setRowidPseudocol() { attributes.v |=  FLAG_IS_ROWID_PSEUDOCOL; }
  bool isRowidPseudocol() const { return attributes.v & FLAG_IS_ROWID_PSEUDOCOL; }

  void setNeedIdToDQuoting() { attributes.v |= FLAG_ID_NEED_TO_DQUOTING; }
  bool needIdToDQuoting() { return attributes.v & FLAG_ID_NEED_TO_DQUOTING; }

  bool needQuotingInSqlMode() const { return attributes.v & (FLAG_ID_NEED_TO_DQUOTING | FLAG_ID_HAS_SPEC_SYMBOL); }
  bool needQuotingInProcMode() const { return attributes.v & (FLAG_ID_NEED_TO_DQUOTING | FLAG_ID_HAS_DOLLAR_SYMBOL); }

  void clearSpecSymbols() { attributes.v &= ~(FLAG_ID_HAS_SPEC_SYMBOL); }


  void setSelfAlreadyPushed() { attributes.v |= FLAG_ID_SELF_ALREADY_PUSHED; }
  bool selfAlreadyPushed() const { return attributes.v & FLAG_ID_SELF_ALREADY_PUSHED; }

  inline void toNormalizedString(string &s) const;
  inline std::string toNormalizedString() const;
  std::string toQInvariantNormalizedString(bool proc) const;
  void toQInvariantNormalizedString(string &s, bool proc) const;

  std::string toCodeId(LevelResolvedNamespace *levelNamespace, bool addToNamespace = false, bool notChangeReserved = false) const;
  std::string toCodeId(ResolvedEntity*, bool addToNamespace = false, bool notChangeReserved = false) const;

  const Id& normalizedString() const { return *this; }
  uint hash() const { return hashValue; }

  bool contextLevelUp() const { return attributes.v & FLAG_ID_IS_CONTEXT_LEVEL_UP; }
  void incContextLevelUp() { attributes.v |= FLAG_ID_IS_CONTEXT_LEVEL_UP; }

  Sm::Codestream &translate(Sm::Codestream &str) { str << *this; return str; }

  bool isFunction() const;
  bool isObject()   const;
  bool isMethod()   const;
  bool updateQuotedUserIdToVarchar2Literal();
  bool updateUnresolvedQuotedToVarchar2Literal();
  bool hasNoncursorCallSemantic() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  SemanticTree *toSNodeRef(SCathegory::t t, const ResolvedEntity *d = 0) const;
  SemanticTree *toSNodeDecl(SCathegory::t t, const ResolvedEntity *d) const;
  SemanticTree *toSNodeDef(SCathegory::t t, const ResolvedEntity *d) const;

  static inline Id *setCallArgList(Id *id, CLoc l, PlExpr *arg1);
  static inline uint getQHashValue(char *_text, size_t length);
  static inline uint getQHashValue(const std::string &_text);
  static inline uint getNQHashValue(const char *_text, size_t length);
  static inline uint getNQHashValue(const std::string &s);

  Id() : text(&emptyText) {}

  Id(CLoc &l, Ptr<CallArgList> callArglist)
    : Base(),
      callArglist(callArglist),
      lloc(l),
      text(&emptyText) {}

  Id(Id &&oth)
    : Base  (),
      idDefinition_(oth.idDefinition_),
      callArglist  (oth.callArglist  ),
      lloc         (oth.lloc         ),
      hashValue    (oth.hashValue    ),
      attributes   (oth.attributes   ),
      text         (oth.text         ),
      semanticNode_(oth.semanticNode_)
  {
    normalizedStr.swap(normalizedStr);
    oth.callArglist = 0;
  }


  Id(const Id &oth)
    : Base  (),
      idDefinition_(oth.idDefinition_),
      callArglist  (oth.callArglist  ),
      lloc         (oth.lloc         ),
      hashValue    (oth.hashValue    ),
      attributes   (oth.attributes   ),
      text         (oth.text         ),
      semanticNode_(oth.semanticNode_) {}

  Id(const Id &oth, const FLoc &l)
    : Id(oth) { lloc = l; }

  Id(const FLoc &l, const string &_text, initializer_list<PlExpr *> _callArgList, ResolvedEntity *def = 0);
  Id(const FLoc &l, const string &_text, initializer_list<Ptr<PlExpr> > _callArgList, ResolvedEntity *def = 0);

  Id(string &&_text, ResolvedEntity *_definition = 0, bool isQuoted = false, bool isEmpty = false)
    : Base(),
      idDefinition_(_definition),
      hashValue    (getHashValue(_text, isQuoted)),
      attributes   (isQuoted, isEmpty),
      text(isQuoted ? getCachedTextPtr(hashValue, _text) : getCachedTextPtr(_text)) {}

  Id(string &&_text, ResolvedEntity *_definition, IdAttributes::Value attrVal)
    : Base(),
      idDefinition_(_definition),
      hashValue    (getHashValue(_text, attrVal & (FLAG_ID_IS_SQUOTED | FLAG_ID_IS_QUOTED))),
      attributes   (attrVal),
      text((attrVal & (FLAG_ID_IS_SQUOTED | FLAG_ID_IS_QUOTED)) ? getCachedTextPtr(hashValue, _text) : getCachedTextPtr(_text)) {}

  Id(const FLoc &l, string &&_text, ResolvedEntity *_definition = 0)
    : Base(),
      idDefinition_(_definition),
      lloc         (l),
      hashValue    (getHashValue(_text, false)),
      text(getCachedTextPtr(_text)) {}

  inline Id(uint hashV, unsigned int attributesValue, char *_text, size_t length, Loc *l) // !!! notQuoted
    : Base(),
      lloc      (cl::fLoc(*l)),
      hashValue (hashV),
      attributes(attributesValue),
      text      (getCachedTextPtr(_text, length)) {}

  inline Id(Loc *l, unsigned int attributesValue) // !!! onlyQuoted
    : Base(),
      lloc      (cl::fLoc(*l)),
      hashValue (getQHashValue(lexer_id_text.ptr(), lexer_id_text.size())),
      attributes(attributesValue),
      text      (getCachedTextPtrOnLexer(hashValue)) {}

  inline Id(Loc *l, unsigned int attributesValue, int) // !!! onlyNotQuoted
    : Base(),
      lloc      (cl::fLoc(*l)),
      hashValue (getNQHashValue(lexer_id_text.ptr(), lexer_id_text.size())),
      attributes(attributesValue),
      text      (getCachedTextPtrOnLexer()) {}

  inline Id(const string &str)
    : Base(),
      hashValue(getQHashValue(str)),
      attributes(FLAG_ID_IS_QUOTED),
      text     (getCachedTextPtr(str.c_str(), str.size())) {}

  inline Id(char *_text, size_t length, CLoc l)
    : Base(),
      lloc     (l),
      hashValue(getNQHashValue(_text, length)),
      text     (getCachedTextPtr(_text, length)) {}

  inline Id(const char *_text, size_t length, CLoc l)
    : Base(),
      lloc(l),
      hashValue         (getNQHashValue(_text, length)),
      text              (getCachedTextPtr(_text, length)) {}

  void setCalArglistAndUpdateLoc(CLoc &l, CallArgList *cList);

  bool operator==(const Id &o) const;
  bool operator!=(const Id &o) const { return !this->operator==(o); }
  bool operator==(const HString &o) const;
  bool operator!=(const HString &o) const { return !this->operator==(o); }


  void setIsEmpty()                       { attributes.v |= FLAG_ID_IS_EMPTY; }
  inline bool empty   () const            { return text->empty(); }
  inline bool emptyStr() const            { return empty();  }
  inline bool squoted () const            { return attributes.v & FLAG_ID_IS_SQUOTED; }
  inline bool quoted  () const            { return attributes.v & FLAG_ID_IS_QUOTED; }
  inline bool hostId  () const            { return attributes.v & FLAG_ID_IS_HOST_ID; }
  inline bool isStringLiteral() const     { return attributes.v & FLAG_ID_IS_STRING_LITERAL; }
  inline bool hasSpecSymbols() const      { return attributes.v & FLAG_ID_HAS_SPEC_SYMBOL; }
  inline bool hasDollarSymbols() const    { return attributes.v & FLAG_ID_HAS_DOLLAR_SYMBOL; }
  inline bool hasSquareSymbols() const    { return attributes.v & FLAG_ID_HAS_SQUARE_SYMBOL; }
  inline bool notquoted() const           { return (attributes.v & FLAG_ID_IS_QUOTED) == 0; }
  inline bool calledArgument() const      { return attributes.v & FLAG_ID_IS_CALLED_ARGUMENT; }
  inline bool isEmptyId() const           { return attributes.v & FLAG_ID_IS_EMPTY; }
  inline bool isDateLiteral() const       { return attributes.v & FLAG_ID_IS_DATELITERAL; }
  inline bool isDblink() const            { return attributes.v & FLAG_ID_IS_DBLINK; }
  inline bool skipFunctionResolving() const { return attributes.v & FLAG_ID_SKIP_FUNCTION_RESOLVING; }
  inline bool isResolvedFunction() const { return attributes.v & FLAG_ID_RESOLVED_FUNCTION; }

  inline void setCalledArgument()  { attributes.v |= FLAG_ID_IS_CALLED_ARGUMENT; }
  inline void setSQuoted()       { attributes.v |= FLAG_ID_IS_SQUOTED; }
  inline void setStringLiteral() { toUpperIfUnquoted(); attributes.v |= FLAG_ID_IS_STRING_LITERAL; }
  inline void setQuoted()        { toUpperIfUnquoted(); attributes.v |= FLAG_ID_IS_QUOTED; }
  inline void setSpecSymbols()   { toUpperIfUnquoted(); attributes.v |= FLAG_ID_HAS_SPEC_SYMBOL; }
  inline void setSquareSymbol()  { toUpperIfUnquoted(); attributes.v |= FLAG_ID_HAS_SQUARE_SYMBOL; }
  inline void setDollarSymbol()  { toUpperIfUnquoted(); attributes.v |= FLAG_ID_HAS_DOLLAR_SYMBOL; }
  inline void setHostId()        { attributes.v |= FLAG_ID_IS_HOST_ID; }
  inline void setDateLiteral(bool flag);
  inline void setIsDblink()      { attributes.v |= FLAG_ID_IS_DBLINK; }
  inline bool setSkipFunctionResolving() const { return attributes.v |= FLAG_ID_SKIP_FUNCTION_RESOLVING; }
  inline bool setResolvedFunction() const { return attributes.v |= FLAG_ID_RESOLVED_FUNCTION; }

  void setUpperAndQuoted();

  operator THCChar() { return THCChar(toString()); }

  void definition(Ptr<ResolvedEntity> def) { if (def) idDefinition_ = def.object(); }
  void definition(const ResolvedEntity *def) { if (def) idDefinition_ = (ResolvedEntity*)def; }
  void clearDefinition() { idDefinition_ = 0; }
  ResolvedEntity *definition() const;
  ResolvedEntity *unresolvedDefinition() const { return idDefinition_; }

  Id* def(const ResolvedEntity *d);

  void semanticNode(const SemanticTree* node) { semanticNode_ = (SemanticTree*)node; }
  SemanticTree* semanticNode() { return semanticNode_; }

  // Совпадение без учета закавыченности
  bool isSemanticString(const char *keyword, int length) const;

  bool callArglistEmpty() const { return !callArglist || callArglist->empty(); }
  bool callArglistGE(unsigned int value) { return callArglist && callArglist->size() >= value; }
  bool callArglistLE(unsigned int value) { return callArglist ? callArglist->size() <= value : true; }
  bool callArglistEQ(unsigned int value) { return (value > 0 && callArglist && callArglist->size() == value) || callArglistEmpty(); }
  size_t callArglistSize() const { return callArglist ? callArglist->size() : 0; }

  // Закавыченные входные строки не рассматриваются
  inline bool isKeyword(const char *keyword, int length) const;
  bool isReservedField() const;
  void toQString(std::string & dst, bool isProcMode = false) const;

  Ptr<Datatype> getDatatype() const;
  inline bool getFields(EntityFields &fields) const;

  std::string toQString(bool isProcMode = false) const { std::string s; toQString(s, isProcMode); return s; }
  std::string toString() const { return *text; }
  std::string getText() const;
  std::string getQuotedText() const;

  // inline void loc(const FLoc & l) { lloc = l.loc; }
  inline void loc(const FLoc & l) { lloc = l; }
  inline const FLoc &getLLoc() const { return lloc; }

  bool beginedFrom(uint32_t line, uint32_t column) const { return lloc.beginedFrom(line, column); }
  bool beginedFrom(uint32_t line) const { return lloc.beginedFrom(line); }
  bool beginedFrom(cl::position pos) const { return lloc.beginedFrom(pos); }
  bool beginedFrom(const std::vector<int> &pos) const {
    if (pos.size() == 1)
      return lloc.beginedFrom(*(pos.begin()));
    else if (pos.size() == 2)
      return lloc.beginedFrom(*(pos.begin()), pos.back());
    else if (pos.empty())
      return false;
    else
      throw 999;
    return false;
  }
  void toUpperIfUnquoted();

private:
  void textToUpper();
  void toQuotedNondateLiteral(std::string &dst, bool isProcMode) const;
};



Codestream& operator<< (Codestream& os, const Id& str);

inline IdEntitySmart::IdEntitySmart(std::string &&str, bool quoted)
  : BaseType(1, new Id(std::forward<string>(str), 0, quoted)) {}

inline bool Sm::IdEntitySmart::getFields(EntityFields &fields) const {
  if (Id *ent = entity())
    return ent->getFields(fields);
  return false;
}

Codestream& operator<< (Codestream& os, const Id2& str);

class Id2               : public SingleSmart, public GrammarBase {
  friend  Codestream& operator<<(Codestream& os, const Id2& str);
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> id[2];

  Id2& operator=(Ptr<Id> o)  { id[0] = o; id[1] = Ptr<Id>(); return * this; }
  Id2() {}
  Id2(Ptr<Id> o) { id[0] = o; }
  Id2(const Id2 & o) : Smart(), GrammarBase(o)  { id[0] = o.id[0]; id[1] = o.id[1]; }
  Id2(CLoc l, Ptr<Id> o)               : GrammarBase(l) { id[0] = o; }
  Id2(CLoc l, Ptr<Id> o1, Ptr<Id> o2 ) : GrammarBase(l) { id[0] = o1; id[1] = o2; }
  Id2(Ptr<Id> _entity, Ptr<Id> _uname) { id[0] = _entity; id[1] = _uname; }

  Id2(string &&name, Ptr<Id> uname)
    : id{new Id(std::forward<string>(name)), uname} {}

  Id2(string &&str, bool isQuoted = false) : id{new Id(std::forward<string>(str), 0, isQuoted), 0} {}

  size_t size() const;
  bool empty() const { return size() == 0; }

  Sm::Codestream &translate(Sm::Codestream &str) { str << *this; return str; }
  void toNormalizedString(string &str) const;
  string toNormalizedString() const { string s; toNormalizedString(s); return s; }
  string toInvariantQString(bool proc) const;

  inline std::string toString(int i) const { if (i < 2) return id[i]->toString(); else throw 999; return ""; }

  operator THCChar() const { return id[0] ? THCChar(toString(0)) : THCChar(); }
  THCChar toUnameDHCChar() const { return id[1] ? THCChar(toString(1)) : THCChar();}

  Ptr<Id> &userName()        { return id[1]; }
  Id *    uname()      const { return id[1].object(); }
  Ptr<Id> entityName() const { return id[0]; }
  inline Id *entity()     const { return id[0].object(); }

  bool getFields(std::vector<Ptr<Id> > &fields) const { return entity() ? entity()->getFields(fields) : false; }
  Ptr<Datatype> getDatatype() const;
  ResolvedEntity* getNextDefinition() const;
  bool getFields(EntityFields &fields);

  void definition(const ResolvedEntity* def) const { if (Id* i = id[0].object()) i->definition(def); }
  ResolvedEntity* definition() const;

  void toIdEntity(IdEntitySmart &entity) const;
  Ptr<IdEntitySmart> toIdEntity() const;

  void toIdEntityDeepCopy(IdEntitySmart &entity) const;
  Ptr<IdEntitySmart> toIdEntityDeepCopy() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  void semanticNode(const SemanticTree *node);

  SemanticTree *toSNodeDef(SCathegory::t t, const ResolvedEntity *d) const;
  SemanticTree *toSNodeDecl(SCathegory::t t, const ResolvedEntity *d) const;
  SemanticTree *toSNodeRef(SCathegory::t t, const ResolvedEntity *d = 0) const;
  SemanticTree *toSNodeDatatypeRef(SCathegory::t t) const;

  size_t bSize(SmartptrSet &s) const;

  Ptr<Datatype> getDatatype();
  bool getFieldRef(Ptr<Id> &field);
};


inline bool Id::getFields(EntityFields &fields) const  {
  if (ResolvedEntity *d = definition())
    return d->getFields(fields);
  return false;
}

//inline ResolvedEntity *SemanticTree::declarationNameDef() const { return declarationName_ ? declarationName_->definition() : (ResolvedEntity*)0; }

inline ResolvedEntity *SemanticTree::entityDef() const {
  Ptr<Id> n;
  ResolvedEntity *def = 0;
  if (//((n = declarationName()) && (def = n->unresolvedDefinition())) ||
      //((n = ddlNameEntity()  ) && (def = n->unresolvedDefinition())) ||
      ((n = refEntity()) && (def = n->unresolvedDefinition())))
    return def;
  return def;
}


//inline size_t SemanticTree::ddlNameSize() const { return ddlName_ ? ddlName_->size() : 0; }
//inline Id *SemanticTree::ddlNameEntity() const { return ddlName_ ? ddlName_->entity() : (Id*)(0); }
inline void IdEntitySmart::semanticNode(const SemanticTree *node) {
  for (iterator it = begin(); it != end(); ++it)
    if (*it)
      (*it)->semanticNode(node);
}

inline Ptr<Id> IdEntitySmart::parentInList(Ptr<Id> &field) const {
  if (size() <= 1 || !field || back().object() == field.object() || *back() == *field)
    return 0;
  for (const_reverse_iterator it = ++rbegin(); it != rend(); ++it) {
    if (it->object() == field.object() || **it == *field)
      return *(--it);
  }
  return 0;
}

inline Ptr<Id> SemanticTree::parentInRefList(Ptr<Id> &field) const {
  return referenceName_->parentInList(field);
}




struct FunCallArgAsterisk  : public FunCallArg {
  Ptr<PlExpr> _expr;
  Argclass argclass() const { return ASTERISK; }
  FunCallArgAsterisk(CLoc l);
  FunCallArgAsterisk(CLoc l, Ptr<PlExpr> __expr);


  void linterDefinition(Sm::Codestream &) { throw 999; } // звёздочек в процедурном языке быть не должно
  void sqlDefinition   (Sm::Codestream &s) { s << '*'; }
  bool isAsterisk() const { return true; }
  void collectSNode(SemanticTree *) const {}
  Ptr<Id> argname() const { return 0; }
  Ptr<PlExpr> expr() const  { return _expr; }
  void setExpr(PlExpr *expr) { _expr = expr; }

  void replaceChildsIf(ExprTr tr);
};

struct FunCallArgExpr  : public FunCallArg {
  Ptr<PlExpr> _expr;
  FunCallArgExpr(PlExpr *e);
  FunCallArgExpr(const FLoc &l, PlExpr *e)
    : GrammarBase(l), _expr(e) { sAssert(!e); }

  Ptr<Id> getName() const;
  Argclass argclass() const { return POSITIONAL; }
  Ptr<PlExpr> expr() const;
  void setExpr(PlExpr *expr) { _expr = expr; }
  void linterDefinition(Sm::Codestream &s);
  void sqlDefinition   (Sm::Codestream &s);
  void collectSNode(SemanticTree *n) const;

  void replaceChildsIf(ExprTr tr);

  FunCallArgExpr *toSelfFunCallArgExpr() const { return const_cast<FunCallArgExpr*>(this); }
  virtual Ptr<FunCallArg> deepCopy();
};

struct FunCallArgNamed  : public FunCallArg {
  Ptr<Id> _name;
  Ptr<PlExpr> _expr;
  int posInDefinition = 0;

  FunCallArgNamed(CLoc l, Ptr<Id> n, Ptr<PlExpr> e);

  virtual void setArgPositionInDef(int v) { posInDefinition = v; }
  virtual int getArgPositionInDef(int) { return posInDefinition; }

  Ptr<Id> argname() const;
  Argclass argclass() const { return NAMED; }
  Ptr<PlExpr> expr() const;
  void setExpr(PlExpr *expr) { _expr = expr; }
  void sqlDefinition   (Sm::Codestream &) { throw 999; } // нужно реализовать перестановку на более раннем этапе
  void linterDefinition(Sm::Codestream &str) { trError(str, s << "FunCallArgNamed::linterDefinition not yet implemented"); } // нужно реализовать перестановку на более раннем этапе
  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(ExprTr tr);
};


void Id::toNormalizedString(string &s) const {
  if (normalizedStr.empty()) {
    if (quoted())
      s.append(*text);
    else {
      setNormalized();
      s.append(normalizedStr);
    }
  }
  else
    s.append(normalizedStr);
}

string Id::toNormalizedString() const {
  if (normalizedStr.empty()) {
    if (quoted())
      return *text;
    else {
      setNormalized();
      return normalizedStr;
    }
  }
  else
    return normalizedStr;
}





inline Sm::Id *Sm::Id::setCallArgList(Id *id, CLoc l, PlExpr *arg1) {
  id->callArglist = new Sm::Vector<Sm::FunCallArg>(new Sm::FunCallArgExpr(l, arg1));
  return id;
}

uint Id::getQHashValue(char *_text, size_t length) {
  return BackportHashMap::internal::HashFunctions::cchar_hash(_text, length);
}


uint Id::getQHashValue(const string &_text) {
  return BackportHashMap::internal::HashFunctions::std_string_hash(_text);
}

inline uint Id::getNQHashValue(const char *_text, size_t length) {
  return BackportHashMap::internal::HashFunctions::cchar_upper_hash(_text, length);
}

inline uint Id::getNQHashValue(const std::string &s) {
  return BackportHashMap::internal::HashFunctions::std_string_upper_hash(s);
}

void Id::setDateLiteral(bool flag) {
  if (flag) attributes.v |= FLAG_ID_IS_DATELITERAL;
  else attributes.v &= ~FLAG_ID_IS_DATELITERAL;
}

bool Id::isKeyword(const char *keyword, int length) const {
  if (quoted())
    return false;
  else
    return isSemanticString(keyword, length);
}




}

#endif // SEMANTIC_ID_H
