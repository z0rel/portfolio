#ifndef SEMANTIC_UTILITY_H
#define SEMANTIC_UTILITY_H

#include <sstream>
#include <unordered_set>
#include <utility>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <vector>
#include <list>
#include "lex_location.h"
#include "smartptr.h"
#include "hash_table.h"
#include "codespacer.h"
#include "model_head.h"

typedef size_t GlobalEntitiesCnt;
class UserContext;
class ModelContext;

extern std::string  *globalCurrentFile;


namespace Sm {

inline void sAssert(bool v) {
  if (v)
    throw 999;
}

  class SmError : public std::exception {
  public:
    string message;
    SmError(string msg)
      : message(msg) {}
    operator string() const { return message; }
  };

  class UnimplementedOperation : public SmError {
  public:
    UnimplementedOperation(string msg = "") : SmError(msg) {}
  };

  class AsteriskInReferenceTranslation : public SmError {
  public:
    AsteriskInReferenceTranslation(string msg = "") : SmError(msg) {}
  };

  class CursorHasNotActualParameter : public SmError {
  public:
    CursorHasNotActualParameter(string msg = "") : SmError(msg) {}
  };




  class Id;
  class NumericSimpleInt;
  class SemanticTree;
  namespace GlobalDatatype {
    struct SysTypeInfo;
    class FundamentalDatatype;
    // При изменении данного перечисления - нужно обновить HasImplicitConversion:: datatypeCathegories и HasImplicitConversion:: greatherDatatypeCathegories
    enum DatatypeCathegory : unsigned int {
     EMPTY              =  0,
     _BFILE_            =  1,
     _BLOB_             =  2,
     _CLOB_             =  3,
     _NCLOB_            =  4,
     _DATE_             =  5,
     _TIMESTAMP_        =  6,
     _TIMESTAMP_LTZ_    =  7,
     _TIMESTAMP_SIMPLE_ =  8,
     _TIMESTAMP_TZ_     =  9,
     _INTERVAL_         = 10,
     _INTERVAL_DTS_     = 11,
     _INTERVAL_YTM_     = 12,
     _BOOLEAN_          = 13,
     _SMALLINT_         = 14,
     _INT_              = 15,
     _BIGINT_           = 16,
     _PLS_INTEGER_      = 17,
     _REAL_             = 18,
     _DOUBLE_           = 19,
     _DECIMAL_          = 20,
     _NUMBER_           = 21,
     _NUMERIC_          = 22,
     _LONG_RAW_         = 23,
     _RAW_              = 24,
     _ROWID_            = 25,
     _UROWID_           = 26,
     _CHAR_             = 27,
     _STRING_           = 28,
     _VARCHAR_          = 29,
     _LONG_             = 30,
     _NCHAR_            = 31,
     _NVARCHAR_         = 32,
     _VARRAY_           = 33,
     _NESTED_TABLE_     = 34,
     _OBJECT_           = 35,
     _ANYDATA_          = 36,
     _XMLTYPE_          = 37
  };

#define FUNDAMENTAL_TYPES_COUNT 38

  }


class CallarglistTranslator : public Smart {
public:
  typedef void (*TrFun)(smart::Ptr<Id> &call, Sm::Codestream &str);
  CallarglistTranslator() {}

  virtual void operator() (smart::Ptr<Id> &call, Sm::Codestream &str) = 0;
  virtual ~CallarglistTranslator() {}
};

class CallarglistTranslatorSimple : public CallarglistTranslator {
  TrFun tr;
public:
  typedef void (*TrFun)(smart::Ptr<Id> &call, Sm::Codestream &str);
  CallarglistTranslatorSimple(TrFun _tr);

  virtual void operator() (smart::Ptr<Id> &call, Sm::Codestream &str);
};



}

extern std::set<Sm::Id*> idManagerMap;

class IdManager {
public:
  ~IdManager();
  void add(Sm::Id */*n*/) { /*idManagerMap.insert(n)*/; }
  void del(Sm::Id */*n*/) { /*idManagerMap.erase(n); */ }
};

class NumericSimpleIntManager {
public:
  std::set<Sm::NumericSimpleInt*, Sm::LE_ResolvedEntities> managerMap;

  ~NumericSimpleIntManager();
  void add(Sm::NumericSimpleInt */*n*/) { /*managerMap.insert(n)*/; }
  void del(Sm::NumericSimpleInt */*n*/) { /*managerMap.erase(n); */ }
};

class SemanticTreeManager {
public:
  std::set<Sm::SemanticTree *> managerMap;
  size_t streeCnt;
  SemanticTreeManager() : streeCnt(0) {}

  ~SemanticTreeManager();
  void add(Sm::SemanticTree *n);
  void del(Sm::SemanticTree *n) { managerMap.erase(n);  }
};

extern IdManager idManager;
extern NumericSimpleIntManager numericSimpleIntManager;


namespace Sm {
  class Id;
  class Id2;
  class ResolvedEntity;
  class StatementInterface;
  class Datatype;
  class Datatype;
  class SqlExpr;
  class LValue;
  class SemanticTree;
  class FunctionCall;
  class FunctionArgument;
  class BlockPlSql;
  class NumericValue;
  class Function;
  class Exception;
  class Object;
  class OpenCursor;
  class IdEntitySmart;
  class PartitialResolvedFunction;
  class PlExpr;
  class Variable;
  class Constraint;

  struct FunCallArg;
  class Subquery;


  namespace trigger {
    class DmlEvents;
  }

  template <typename T> class List;
  template <typename T> class Vector;
  namespace Type {
//    class   Object;
//    class   Record;
    class MemberFunction;
    struct  RefInfo;
    class ArrayConstructor;
    class CollectionType;
    namespace collection_methods {
      class AccessToItem;
////      class CollectionMethod;
    }
  }
  namespace view {
    class ViewConstraints;
  }
  namespace table {
    namespace field_property {
      class FieldProperty;
    }
  }

  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> ResolvedEntitySet;
  typedef Sm::Vector<Sm::FunctionArgument> FunArgList;
  typedef cl::location Loc;
  typedef cl::filelocation FLoc;
//  typedef const cl::location &CLoc;
  typedef const cl::filelocation &CLoc;

  typedef std::map<std::string, std::pair<Exception*, FLoc> > DeclaredExceptions;

  typedef Sm::Vector<Sm::FunCallArg> CallArgList;
  class Codestream;

  typedef void (*NameTranslator)(Sm::Codestream &str, smart::Ptr<CallArgList>);

namespace CathegoryStatementInterface {
  enum  T {
    __EMPTY__,
    __DECLTYPE_CURSOR__,
    __DECLTYPE_FIELD__,
    NULL_STATEMENT,
    PRAGMA_STMT,
    LABEL,
    ASSIGNMENT,
    BLOCK_PLSQL,
    CASE,
    CLOSE,
    EXECUTE_IMMEDIATE,
    EXIT,
    FETCH,
    FORALL,
    FOR_OF_EXPR,
    FOR_OF_RANGE,
    FUNCTION_CALL,
    GOTO,
    IF,
    LOOP,
    OPEN_CURSOR,
    OPEN_FOR,
    PIPE_ROW,
    RAISE,
    RETURN,
    SQL_STATEMENT,
    WHILE
  };
}

  class SemanticInterfaceBase {
  protected:
    virtual Sm::SemanticTree *toSTreeBase() const = 0;
  public:
    Sm::SemanticTree *toSTree() const;
  };
  class SemanticInterface : public virtual smart::Smart {
  protected:
    virtual Sm::SemanticTree *toSTreeBase() const = 0;
  public:
    Sm::SemanticTree *toSTree() const;
  };
  class CollectSemanticInterface : public virtual smart::Smart {
  public:
    virtual void collectSNode(Sm::SemanticTree *node) const = 0;
  };

  class LinterCursor;
  typedef std::vector<std::pair<smart::Ptr<Sm::FunctionArgument>, std::vector<smart::Ptr<Sm::LinterCursor> > > > OutRefCursors;

using namespace BackportHashMap;
using namespace smart;

template <typename Key, typename Value>
class Map : public smart::SingleSmart, public std::map<Key, Value> {
public:
  Map() {}
};

template <typename Key>
class Set : public smart::SingleSmart, public std::set<Key> {
public:
  Set() {}
};

template <typename T>
class SortedVector : public std::vector<T*> {
public:
  typedef T* mapped_type;
  typedef T  mapped_dereferenced_type;
  typedef unsigned int key_type;
  typedef typename std::vector<mapped_type>::iterator iterator;
  typedef std::pair<iterator, bool> inserted_result_type;
  typedef std::vector<mapped_type> Container;

private:
  mapped_type operator[] (void *) { return 0; }
  mapped_type operator[] (int) { return 0; }
  inserted_result_type insert(mapped_type val) {
    inserted_result_type foundedNode = find_mid(val->eid());
    if (foundedNode.second) {
      std::vector<mapped_type> *v = this;
      if (foundedNode.first == this->end())
        return std::pair<iterator, bool>(v->insert(v->end(), val), true);
      else if (val->eid() < (*foundedNode.first)->eid())
        return inserted_result_type(v->insert(foundedNode.first, val), true);
      else // valKey > midKey (равенство исключено бинарным поиском)
        // вставляем после mid
        return inserted_result_type(v->insert(++foundedNode.first, val), true);
    }
    else {
      if (val != *foundedNode.first)
        throw 999; // Индексы элемента в пространстве имен - совпадают, а реальные указатели - не совпадают
      // => ошибка в механизме индексации элементов пространства имен
      return foundedNode;
    }
  }

public:
  SortedVector() {}

  void add(mapped_type val) {
    if (val)
      insert(val);
  }

  inserted_result_type find_mid(unsigned int valKey) const {
    iterator left  = ((Container*)this)->begin();
    iterator right = ((Container*)this)->end();
    iterator mid   = left + std::distance(left, right) / 2;
    iterator end   = right;
    unsigned int midKey = 0;

    for (; left <= right && mid != end; mid = left + std::distance(left, right) / 2)
    {
      midKey = (*mid)->eid();
      if (valKey < midKey)
        right = mid - 1;
      else if (valKey > midKey)
        left = mid + 1;
      else // valKey == midKey
        return inserted_result_type(mid, false);
    }
    return inserted_result_type(mid, true);
  }
  mapped_dereferenced_type& operator[] (mapped_type val) {
    if (val)
      *(this->insert(val).first);
    throw 999; // нельзя вставлять нулевые указатели
  }

  bool count(const mapped_dereferenced_type* val) const {
    if (val) {
      inserted_result_type foundedNode = find_mid(val->eid());
      return (!foundedNode.second && val == *foundedNode.first);
    }
    return false;
  }
};

class ResolvedEntity;



struct EquallyEntities : public SingleSmart {
  EquallyEntities() : key(0) {}
  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> Container;
  ResolvedEntity* key;
  Container declarations;
  Container definitions;

  std::pair<Container::iterator, bool> insert(const Container::value_type &v);
  void check(Container &c);
  ResolvedEntity *getDefinitionFirst() const { return definitions.empty() ? *(declarations.begin()) : *(definitions.begin()); }

  size_t bSize(smart::SmartptrSet &s);
};

class FunctionResolvingContext;

class IsSubtypeValuesEnum {
public:
  enum IsSubtypeCathegory {
    EXPLICIT                = 0,
    IMPLICIT_CAST_LOW       = 1,
    IMPLICIT_CAST_1         = 2,
    IMPLICIT_CAST_2         = 3,
    IMPLICIT_CAST_3         = 4,
    IMPLICIT_CAST_HIGH      = 5,
    IMPLICIT_CAST_BY_FIELDS = 6,

    NEED_CHAR_COMPARSION_BY_LENGTH   = 7,
    NEED_NUMBER_COMPARSION_BY_LENGTH = 8,
    EXACTLY_EQUALLY         = 9,
  };
};

class IsSubtypeValues : public IsSubtypeValuesEnum {
public:
  IsSubtypeCathegory val;
  IsSubtypeValues(IsSubtypeCathegory v) : val(v) {}

  operator bool() { return val != EXPLICIT; }
  bool operator== (IsSubtypeCathegory v) { return v == val; }
};




class VEntities : public smart::Smart {
  friend class FunctionResolvingContext;
public:
  typedef  SortedVector<ResolvedEntity> Container;
  typedef pair<ResolvedEntity*, Ptr<EquallyEntities> > EquallyEntitiesWithDefinitionFirst;


public:
                   /* base entitiy        entities list   */
  typedef std::map<ResolvedEntity*, Ptr<EquallyEntities>, LE_ResolvedEntities> OverloadedFunctions;
            /* overloaded id    pair<base entitiy, entities list> */
  typedef std::map<int, Ptr<EquallyEntities> > GlobalOverloadedNodes;

  static size_t getVentityId();
//  size_t ventityId = getVentityId();

private:
  bool checkRefPtrEquality(Sm::SemanticTree *sNode, Ptr<Sm::Id> &currentPart) const;
  void check(Container &c);
  void check(Ptr<EquallyEntities> c);
  bool assignDeclForce(Sm::Id *reference, ResolvedEntity *def, Sm::SemanticTree *&referenceNode, Sm::SemanticTree *&definitionNode) const;

  void setOwerloadResolvingEntered() { __flags__.v |= FLAG_VENTITIES_OVERLOAD_RESOLVING_ENTERED; }
  bool owerloadResolvingEntered()    { return __flags__.v & FLAG_VENTITIES_OVERLOAD_RESOLVING_ENTERED; }
public:
  // Мэп перегруженных функций, индексированный по eid (большие числа)
  OverloadedFunctions overloadedFunctions;
private:
  // Мэп перегруженных функций, индексированный по индексу перегрузки (0, 1, 2, 3, ....)
  GlobalOverloadedNodes overloadedNodes;

  // Сортировка в этом векторе выполняется по позиции в пространстве имен
  Container functions;
  Ptr<EquallyEntities> variables;
  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> Others;

public:
  Others others;
  VEntities() { __flags__.v |= FLAG_VENTITIES_INJECTIVE; }


  Container &getFunctions() { return functions; }

  bool othersInjective() const { return __flags__.v & FLAG_VENTITIES_INJECTIVE; }
  void clrOthersInjective() { __flags__.v &= ~(FLAG_VENTITIES_INJECTIVE); }
  size_t bSize(smart::SmartptrSet &s);

  bool findByPtr(ResolvedEntity *def) const;

  void check();
  bool checkToCorrectResolving(Sm::SemanticTree *&referenceSNode, Ptr<Sm::Id> &reference) const;
  bool count(ResolvedEntity *def) const;
  void add  (ResolvedEntity *def);
  void addWithoutOverloadResolving(ResolvedEntity *def);
private:
  void resolveOverloaded(ResolvedEntity *def);
  void resolveOverloadedForce(ResolvedEntity *def);
  void addOverloadedNode(ResolvedEntity* def);

public:
  void resolveOverloaded();


  void setSpecialNamesTranslator(ResolvedEntity *d, Sm::NameTranslator nameTr, Sm::CallarglistTranslator *callTr = 0, bool forceElementaryLinter = false);
  // TODO: можно оптимизировать - положив изначально всё без алиасов - в отдельный мэп.
  ResolvedEntity *findFieldWithoutAlias();
  ResolvedEntity *findFieldDefinition();
  ResolvedEntity *findVariableDefinition();

  Container::size_type variablesNotEmpty() const { return variables; }
  EquallyEntities::Container::value_type variablesFront() const { return variables->definitions.empty() ? *(variables->declarations.begin()) : *(variables->definitions.begin()); }
  static inline bool eqByVEntities(const ResolvedEntity *def1, const ResolvedEntity *def2);

  inline ResolvedEntity *getDefinitionFirst(ResolvedEntity *base) const;

  void identificateOverloadedFunctions(Sm::Function *src);
  int compareWithFront(ResolvedEntity *def) const;
};


#define FLAG_DEP_ARC_CONTEXT_IS_WRITE                  (1 << 0)
#define FLAG_DEP_ARC_CONTEXT_IS_EXECUTE_WITHOUT_DIRECT (1 << 1)
#define FLAG_DEP_ARC_CONTEXT_TRAVERSING_MARK           (1 << 2)

class DepArcContext {
  uint32_t value = 0;

  size_t eid = getEid();
public:

  static size_t getEid();

  typedef std::set<ResolvedEntity*> References;
  References references;

  DepArcContext(const DepArcContext &oth);
  DepArcContext();
  DepArcContext(uint32_t val);

  DepArcContext& operator|=(const DepArcContext &oth);

  bool isExecuteWithoutDirect() const { return value & FLAG_DEP_ARC_CONTEXT_IS_EXECUTE_WITHOUT_DIRECT; }
  void setExecuteWithoutDirect() { value |= FLAG_DEP_ARC_CONTEXT_IS_EXECUTE_WITHOUT_DIRECT; }

  bool traversingMark() const { return value & FLAG_DEP_ARC_CONTEXT_TRAVERSING_MARK; }
  void clearTraveringMark() { value &= ~FLAG_DEP_ARC_CONTEXT_TRAVERSING_MARK; }
  void setTraversingMark() { value |= FLAG_DEP_ARC_CONTEXT_TRAVERSING_MARK; }

  bool isWrite() const { return value & FLAG_DEP_ARC_CONTEXT_IS_WRITE; }
  void setWrite() { value |= FLAG_DEP_ARC_CONTEXT_IS_WRITE; }
};


class DependEntitiesMap : public std::map<ResolvedEntity*, DepArcContext, LE_ResolvedEntities>, public CathegoriesOfDefinitions {
public:
  typedef std::map<ResolvedEntity*, DepArcContext, LE_ResolvedEntities> Base;
  void insert(ResolvedEntity *def, ResolvedEntity *parentNode, const DepArcContext &ctx);
  void insert(const Base::value_type &p);
};


struct EntityAttributes : public Smart {
  typedef DependEntitiesMap Arcs;

//  Attributes        flags;
  /// Сущности, которые используют данную сущность
  DependEntitiesMap  inArcs;
  /// Сущности, которые данная сущность - использует
  DependEntitiesMap  outArcs;
  ResolvedEntity    *entity = 0;
  Sm::IdEntitySmart *fullName = 0;
  unsigned int level_ = 0;

  EntityAttributes() {}
  EntityAttributes(const EntityAttributes &o)
    : SingleSmart(),
      entity(o.entity), fullName(o.fullName) {}
  EntityAttributes(bool read, bool write, Sm::IdEntitySmart *_fullName = 0)
    : fullName(_fullName)
  {
    if (read)
      __flags__.v |= FLAG_ENTITY_ATTRIBUTES_READ;
    if (write)
      __flags__.v |= FLAG_ENTITY_ATTRIBUTES_WRITE;
  }

  void concat(const EntityAttributes &o) {
//    flags.v |= o.flags.v;
    __flags__.v |= o.__flags__.v;
  }
  void concat(bool isRead, bool isWrite, Sm::IdEntitySmart *_fullName)  {
    if (isRead)
      __flags__.v |= FLAG_ENTITY_ATTRIBUTES_READ;
    if (isWrite)
      __flags__.v |= FLAG_ENTITY_ATTRIBUTES_WRITE;
    if (_fullName && !fullName)
      fullName = _fullName;
  }
  void translateAsLinterFunctionArgument(Sm::Codestream &);

  inline bool write () const { return __flags__.v & FLAG_ENTITY_ATTRIBUTES_WRITE;   }
  inline bool read  () const { return __flags__.v & FLAG_ENTITY_ATTRIBUTES_READ;    }
  inline bool isOpen() const { return __flags__.v & FLAG_ENTITY_ATTRIBUTES_IS_OPEN; }
  inline void setWrite () { __flags__.v |= FLAG_ENTITY_ATTRIBUTES_WRITE;      }
  inline void setRead  () { __flags__.v |= FLAG_ENTITY_ATTRIBUTES_READ;       }
  inline void setIsOpen() { __flags__.v |= FLAG_ENTITY_ATTRIBUTES_IS_OPEN;    }
  inline void clrIsOpen() { __flags__.v &= ~(FLAG_ENTITY_ATTRIBUTES_IS_OPEN); }
};

typedef std::map<ResolvedEntity*, Ptr<EntityAttributes>, LE_ResolvedEntities> UniqueEntitiesMap;

class LevelResolvedNamespace : public smart::SingleSmart, public std::map<std::string, Ptr<VEntities> > {
public:
  typedef std::map<std::string, Ptr<VEntities> > BaseType ;
  typedef size_t LevelSizeType;
  typedef std::vector<LevelResolvedNamespace *> Childs;
  LevelSizeType  levelFullSize = 0;
  typedef std::map<key_type, mapped_type> ParentType;
  size_t globalId = __getGlobalId();
  static size_t __getGlobalId();
private:
  LevelSizeType  positionInParent_ = 0;

public:
  Childs childs;
  LevelResolvedNamespace *parent;
  Sm::SemanticTree       *semanticLevel;

private:
  static void inline addChild(LevelResolvedNamespace *parent, LevelResolvedNamespace *child) {
    child->positionInParent_ = parent->childs.size();
    parent->childs.push_back(child);
  }
public:
  LevelResolvedNamespace(LevelResolvedNamespace *_parent, Sm::SemanticTree *_semanticLevel);

  LevelSizeType positionInParent() const { return positionInParent_; }
  bool findDeclaration(Sm::SemanticTree *&foundedNode, Ptr<Sm::Id> &reference) const;
  bool findField(Ptr<Sm::Id> &field) const;
  bool findCollectionField(Ptr<Sm::Id> &field) const;
  bool findVariable(Ptr<Sm::Id> &field) const;
  const_iterator findFieldNode(Ptr<Sm::Id> &reference) const;

  ResolvedEntity *findFieldIdDef(Ptr<Sm::Id> &reference) const;
  std::string getUniqueName(std::string prefix);

  size_t bSize(smart::SmartptrSet &s);

  void check();
  iterator node(const std::string &name);

  void deleteIncludedDeclarationsFromSet(UniqueEntitiesMap &declSet) const;
  bool add(::Sm::Id *name);
  void addWithoutFind(::Sm::Id *name);
  void resolveOverloaded();
  ~LevelResolvedNamespace();
};

class Translator {
public:
  virtual ~Translator() {}

  virtual void sqlDeclaration(Sm::Codestream &s) { sqlDefinition   (s); }
  virtual void sqlDefinition (Sm::Codestream &s) { linterDefinition(s); }
  virtual void sqlReference  (Sm::Codestream &s) { linterReference (s); }

  virtual void linterDeclaration(Sm::Codestream &s) { linterDefinition(s); }
  virtual void linterDefinition (Sm::Codestream &)  { throw UnimplementedOperation("linterDefinition"); }
  virtual void linterReference  (Sm::Codestream &)  { throw UnimplementedOperation("linterReference"); }
  virtual void linterDatatypeReference(Sm::Codestream &s) { linterReference(s); }

  virtual void oracleDeclaration(Sm::Codestream &str) { oracleDefinition(str); }
  virtual void oracleDefinition (Sm::Codestream &str) { sqlDefinition(str); }
  virtual void oracleReference  (Sm::Codestream &) {}

  Sm::Codestream &translate(Sm::Codestream &s);
  /// Для данной сущности имеется точный эквивалент в Линтер
  virtual bool hasLinterEquivalent() const { return true; }

  virtual ResolvedEntity *getCursorFuncall() { return 0; }
  virtual void translateAsFunArgumentReference(Sm::Codestream &str)  { linterReference(str); }

  virtual bool needSemicolonAfterEntity() const { return true; }
};

class TranslatedName: public virtual Translator {
public:
  TranslatedName() {;}
  virtual ~TranslatedName() {;}

  void sqlName(const string &name) { sqlName_ = name; }
  void procName(const string &name) { procName_ = name; }
  const string &sqlName() const  { return sqlName_; }
  const string &procName() const { return procName_; }
  bool isTrNameEmpty() const { return procName_.empty() && sqlName_.empty(); }

  virtual std::string procTranslatedName() const { return procName_.empty() ? sqlName() : procName(); }
  virtual void procTranslatedName(const std::string &v)  { procName(v); }


  virtual std::string translatedName() const = 0;
  virtual void translateName(Codestream &str) {
    sAssert(isTrNameEmpty());
    if (str.isProc())
      str << procTranslatedName();
    else
      str << translatedName();
  }
  virtual void generateUniqueName() { throw 999; }

private:
  string sqlName_;
  string procName_;
};

typedef std::vector<Ptr<EntityAttributes> > ExternalVariables;

class STreeNode {
public:
  virtual Sm::SemanticTree *toSTreeBase() const = 0;
  virtual Sm::SemanticTree *getSemanticNode() const = 0;
  virtual Sm::SemanticTree *setSemanticNode(Sm::SemanticTree *) const = 0;
  Sm::SemanticTree *toSTree() const {
    if (getSemanticNode())
      return 0;
    else {
      Sm::SemanticTree *p = toSTreeBase();
      setSemanticNode(p);
      return p;
    }
  }
};

class BasicEntityAttributes
#ifdef SMARTPTR_DEBUG
  : public Smart
#endif
{
public:
  VEntities*                    vEntities    = 0;      // 8
  Ptr<LevelResolvedNamespace>   levelNamespace; // 8
  mutable Ptr<EntityAttributes> attributes;     // 8 -> 32


#ifdef SMARTPTR_DEBUG
  BasicEntityAttributes() { smartDoAction(this, true); }
#endif

  inline EntityAttributes *lazyAttributes() const {
    if (attributes)
      return attributes.object();
    else {
      attributes = new EntityAttributes();
      return attributes.object();
    }
  }
};


class GrammarBase {
  cl::filelocation lloc = cl::emptyFLocation();
public:
  inline void loc(const FLoc & l) { lloc = l; }
  inline CLoc getLLoc() const { return lloc; }

  GrammarBase() {}
  GrammarBase(CLoc l) : lloc(l) {}
  GrammarBase(const GrammarBase &o) : lloc(o.lloc) {}
  inline bool beginedFrom(uint32_t line, uint32_t column) const { return lloc.beginedFrom(line, column); }
  inline bool beginedFrom(uint32_t line) const { return lloc.beginedFrom(line); }

  void setLLoc(const FLoc &l) { lloc = l; }
};

class GrammarBaseSmart : public virtual smart::Smart, public GrammarBase {
protected:
public:
  GrammarBaseSmart() {}
  GrammarBaseSmart(CLoc l) : GrammarBase(l) {}
  GrammarBaseSmart(const GrammarBaseSmart & o) : Smart(o), GrammarBase(o) {}
};

class ResolvingContext;


struct SCathegory
{

  enum t
  {
    EMPTY                            ,//=  0,

    DeclNamespace,

    BulkRowcount                     ,
    AlgebraicCompound                ,
    FieldProperty                    ,//=  1,
    NestedTableProperty              ,//=  2,
    SynonymTarget                    ,//=  3,
    TableProperties                  ,//=  4,
    BooleanLiteral                   ,//=  5,
    ModelContext                     ,//=  6,
    RootSemanticNode                 ,//=  7,


    InsertingValues                  ,
    PlExpr                           ,//=  1 << 11,
    SqlExpr                          ,//=  1 << 12 | PlExpr,
    Subquery                         ,//=  1 << 13 | SqlExpr,
    RefAbstract                        ,//=  1 << 14 | SqlExpr,
    SubqueryId                       ,//=  1 << 15,
    SubqueryPart                     ,//=  1 << 16,
    From                             ,//=  1 << 17 | SubqueryPart,
    Constraint                       ,//=  1 << 18,
    Entity                           ,//=  1 << 19,
    PlEntity                         ,//=  1 << 20 | Entity,
    ChangedQueryEntity                        ,//=  1 << 21 | Entity,
    Declaration                      ,//=  1 << 22 | PlEntity,
    CursorEntity                     ,//=  1 << 23 | PlEntity,
    Trigger                          ,//=  1 << 24 | ChangedQueryEntity,
    Field                            ,//=  1 << 25 | ChangedQueryEntity,
    Table                            ,//=  1 << 26 | ChangedQueryEntity,
    Statement                        ,//=  1 << 27,
    StatementPart                    ,//=  1 << 28,
    SqlCommand                       ,//=  1 << 29,
    Datatype                         ,//=  1 << 30 | Declaration,
    OfTypes                          ,//=  1 << 30 | Declaration,
    Subtype                          ,//=  1 << 30 | Declaration,

    RecordField                      ,//=   1 | Field,

    CompoundTimingBlock              ,//=   1 | Trigger,
    DmlEvents                        ,//=   2 | Trigger,
    TimingPoint                      ,//=   3 | Trigger,
    TriggerActionCall                ,//=   4 | Trigger,
    TriggerReferencig                ,//=   5 | Trigger,

    ConstraintState                  ,//=   1 | Constraint,
    ForeignField                     ,//=   2 | Constraint,
    PrimaryKey                       ,//=   3 | Constraint,
    Unique                           ,//=   4 | Constraint,

    FromJoin                         ,//=   1 | From,

    AsteriskExpr                         ,//=   1 | SubqueryId,
    SelectedField                    ,//=   2 | SubqueryId,    // SelectListItem
    QueryBlockField                  ,//=   3 | SubqueryId,    // SelectListItem
    QueryPseudoField                 ,//=   4 | SubqueryId,    // SelectListItem
    FromTableDynamic                 ,//=   5 | SubqueryId | SqlExpr  | Table,
    FunctionDynField                 ,//=   5 | SubqueryId | SqlExpr  | Table,
    FunctionDynExpr                  ,//=   5 | SubqueryId | SqlExpr  | Table,
    FunctionDynTail_                 ,//=   5 | SubqueryId | SqlExpr  | Table,
    QueryEntityDyn                   ,
    DynamicFuncallTranslator         ,
    TableFromSubquery                ,//=   6 | SubqueryId | Subquery | Table,
    UnionQuery                            ,//=   7 | Subquery,
    TableQueryReference              ,//=   8 | SubqueryId,
    FromSingle                       ,//=   9 | SubqueryId,
    FromTableReference               ,//=  10 | SubqueryId | Table,

    Dblink                           ,//=   1 | SubqueryPart,
    Flashback                        ,//=   2 | SubqueryPart,
    ForUpdateClause                        ,//=   3 | SubqueryPart,
    GroupBy                          ,//=   4 | SubqueryPart,
    GroupingSetsClause                     ,//=   5 | SubqueryPart,
    GroupingSetsCube                 ,//=   6 | SubqueryPart,
    GroupingSetsRollup               ,//=   7 | SubqueryPart,
    GroupingSetsSimple               ,//=   8 | SubqueryPart,
    HierarhicalClause                 ,//=   9 | SubqueryPart,
    OrderBy                          ,//=  10 | SubqueryPart,
    PartitionList                    ,//=  11 | SubqueryPart,
    StatementMerge                   ,
    QueryBlock                       ,//=  12 | SubqueryPart,
    SelectSingle                     ,//=  13 | SubqueryPart,
    SelectBrackets                   ,//=  14 | SubqueryPart,
    FactoringItem                    ,//=  15 | SubqueryPart,
    WhereClause                      ,//=  16 | SubqueryPart,

    Between                          ,//=   1 | PlExpr,
    Comparsion                       ,//=   2 | PlExpr,
    Compound                         ,//=   3 | PlExpr,
    IsOfTypes                        ,//=   4 | PlExpr,
    Like                             ,//=   5 | PlExpr,
    MemberOf                         ,//=   6 | PlExpr,
    RegexpLike                       ,//=   7 | PlExpr,
    Submultiset                      ,//=   8 | PlExpr,
    Exists                           ,

    Case                             ,//=   1 | SqlExpr,
    CaseVariants                     ,//=   2 | SqlExpr,
    Cast                             ,//=   3 | SqlExpr,
    CastMultiset                     ,//=   4 | SqlExpr,
    CollectionAccess                 ,//=   5 | SqlExpr,
    Constructor                      ,//=   6 | SqlExpr,
    DynamicCursor                    ,//=   7 | SqlExpr,
    IfThen                           ,//=   8 | SqlExpr,

    Cursor                           ,//=   1 | CursorEntity,
    CursorVariable                   ,//=   2 | CursorEntity,

    Argument                         ,//=   1 | Entity,
    Collection                       ,//=   2 | Entity,
    Record                           ,//=   3 | Entity,
    Variable                         ,//=   5 | Entity,
    VariableHost                     ,//=   6 | Entity,
    VariableUndeclaredIndex          ,//=   7 | Entity,
    XmlElement                       ,//=   8 | Entity,
    XmlSchema                        ,//=   9 | Entity,
    ObjectMember                     ,//=  10 | Entity,
    TriggerRowReference              ,//=  11 | Entity,

    NestedTableInstance              ,//=   2 | PlEntity,

    Index                            ,//=   1 | ChangedQueryEntity,
    Package                          ,//=   2 | ChangedQueryEntity,
    Role                             ,//=   3 | ChangedQueryEntity,
    Schema                           ,//=   4 | ChangedQueryEntity,
    Sequence                         ,//=   5 | ChangedQueryEntity,
    Synonym                          ,//=   6 | ChangedQueryEntity,
    ChangedQueryTableCollectionExpr              ,//=   8 | ChangedQueryEntity,   // TABLE ( <expr> )
    TableOrView                      ,//=   9 | ChangedQueryEntity,
    TableViewMaterializedView        ,//=  10 | ChangedQueryEntity,   /* TABLE, VIEW, MATERIALIZED VIEW           */
    Tablespace                       ,//=  11 | ChangedQueryEntity,
    User                             ,//=  12 | ChangedQueryEntity,
    View                             ,//=  13 | ChangedQueryEntity,
    DatabaseLink                     ,//=  14 | ChangedQueryEntity,

    Assignment                       ,//=   1 | Statement,
    BlockPlSql                       ,//=   2 | Statement,
    CaseStatement                    ,//=   3 | Statement,
    Close                            ,//=   4 | Statement,
    CursorVariableHost               ,//=   5 | Statement,
    ExecuteImmediate                 ,//=   7 | Statement,
    Exit                             ,//=   8 | Statement,
    Fetch                            ,//=   9 | Statement,
    ForAll                           ,//=  10 | Statement,
    ForOfExpression                  ,//=  11 | Statement,
    ForOfRange                       ,//=  12 | Statement,
    Goto                             ,//=  13 | Statement,
    If                               ,//=  14 | Statement,
    InsertSingleValue                ,//=  15 | Statement,
    InsertMulitpleValues             ,//=  16 | Statement,
    InsertMultipleConditional        ,//=  17 | Statement,
    LockTable                        ,//=  18 | Statement,
    Loop                             ,//=  19 | Statement,
    OpenCursor                       ,//=  20 | Statement,
    Raise                            ,//=  22 | Statement,
    Return                           ,//=  23 | Statement,
    Rollback                         ,//=  24 | Statement,
    Savepoint                        ,//=  25 | Statement,
    SelectStatement                  ,//=  26 | Statement,
    Transaction                      ,//=  27 | Statement,
    Update                           ,//=  28 | Statement,
    While                            ,//=  29 | Statement,
    Merge                            ,//=  30 | Statement,
    Label                            ,//=  31 | Statement,
    PragmaRestrictReferences         ,//=  32 | Statement,

    AlterUserSettings                ,//=   1 | StatementPart,
    CollectionIndex                  ,//=   2 | StatementPart,    // Индексы для адресации элемента коллекции
    Else                             ,//=   3 | StatementPart,
    InsertConditional                ,//=   4 | StatementPart,
    InsertFromSubquery               ,//=   5 | StatementPart,
    InsertFromValues                 ,//=   6 | StatementPart,
    InsertInto                       ,//=   7 | StatementPart,
    InsertedValues                   ,//=   8 | StatementPart,
    LoopBounds                       ,//=   9 | StatementPart,
    ReturnInto                       ,//=  10 | StatementPart,
    UpdatingField                    ,//=  11 | StatementPart,
    WhenThen                         ,//=  12 | StatementPart,
    SqlStatement                     ,//=  13 | StatementPart,

    AddFields                        ,//=   1 | SqlCommand,
    AlterFields                      ,//=   2 | SqlCommand,
    AlterTable                       ,//=   3 | SqlCommand,
    AlterTableAddConstraint          ,//=   4 | SqlCommand,
    AlterTableDropConstraint         ,//=   5 | SqlCommand,
    AlterTableDropFields             ,//=   6 | SqlCommand,
    AlterTableDropKey                ,//=   7 | SqlCommand,
    AlterTableManipulateFields       ,//=   8 | SqlCommand,
    AlterTableModifyConstraint       ,//=   9 | SqlCommand,
    AlterTableModifyFields           ,//=  10 | SqlCommand,
    AlterTableModifyKey              ,//=  11 | SqlCommand,
    AlterTableRenameTable            ,//=  13 | SqlCommand,
    AlterUser                        ,//=  14 | SqlCommand,

    Expr_Asterisk                    ,//=   1 | RefAbstract,
    Expr_Currval                     ,//=   2 | RefAbstract,
    Expr_CursorProperty              ,//=   3 | RefAbstract,
    Expr_ExistFunc                   ,//=   4 | RefAbstract,
    Expr_HostCursorProperty          ,//=   5 | RefAbstract,
    Expr_HostSecondRef               ,//=   6 | RefAbstract,
    Expr_NextVal                     ,//=   7 | RefAbstract,
    Expr_OutherJoin                  ,//=   8 | RefAbstract,
    Expr_SqlBulkRowcount             ,//=   9 | RefAbstract,
    Expr_SqlCursorProperty           ,//=  10 | RefAbstract,
    AnalyticFun                      ,//=  11 | RefAbstract,
    Into                             ,//=  12 | RefAbstract,

    ObjectType                       ,//=   1 | Datatype,
    Anydata                          ,//=   1 | Datatype,
    RecordType                       ,//=   2 | Datatype,
    Varray                           ,//=   3 | Datatype,
    Exception                        ,//=   4 | Declaration,
    Function                         ,//=   5 | Declaration,
    FunctionPragmaRestriction        ,//=   5 | Declaration,
    ArrayConstructor                 ,//=   6 | Declaration,
    RefCursor                        ,//=   7 | Datatype,
    NestedTableType                  ,//=   8 | Datatype,

    ExceptionSection                 ,//

    UnaryOp                          ,//
    BlockPlSqlStatementsList,
    WhenExprClause,
    StatementLoop,
    StatementIf,
    StatementOpenCursor,
    StatementOpenFor,
    StatementWhile,
    StatementFunctionCall,
    StatementClose,
    StatementGoto,
    StatementPipeRow,
    StatementRaise,
    StatementForOfExpression,
    StatementFetch,
    StatementExit,
    StatementImmediate,
    StatementCase,
    StatementDeleteFrom,
    StatementSelect,
    StatementRollback,
    StatementLockTable,
    StatementTransaction,
    StatementSingleInsert,

    WhereClauseDML,
    OrderByPartitionList,
    LValueHost,
    LValue,
    AliasedFieldsList ,
    ViewQueryNode,
    ForOfRangeRange,
    StatementConstructExpr,
    StatementConstructBlockPlsql,

    __LAST_SCATHEGORY__
  };

  class Cathegory {
  public:
    SCathegory::t v;
    union CathegoryFields {
      SCathegory::t v;
      struct EntityFields {
        int value        :10;
        int listList     :1;
        int plExpr       :1;
        int sqlExpr      :1;
        int subquery     :1;
        int sqlExprId    :1;
        int subqueryId   :1;
        int subqueryPart :1;
        int from         :1;
        int constraint   :1;
        int entity       :1;
        int plEntity     :1;
        int sqlEntity    :1;
        int declaration  :1;
        int cursorEntity :1;
        int trigger      :1;
        int field        :1;
        int table        :1;
        int statement    :1;
        int statementPart:1;
        int sqlCommand   :1;
        int datatype     :1;
        int list         :1;
      } f;
    } f;

    Cathegory(SCathegory::t val) { set(val); }
    Cathegory &operator=(SCathegory::t val) { set(val); return *this; }
    bool operator!=(SCathegory::t val) const { return v != val; }
    operator SCathegory::t() { return v; }

  private:
    void set(SCathegory::t val) {
      f.v = val;
      v = (SCathegory::t)(val & 0x3FF);
    }
  };
};

template <typename T1, typename T2>
inline bool isEntry(T1 setEnumItem, T2 entriedPart) {
  bool result = (setEnumItem & entriedPart) == entriedPart ? true : false;
  return result;
}

std::string debugSCathegoryConvert(SCathegory::t t);

template <typename T>
struct __Dereference {
  typedef T dereference_type ;
};

template <typename T>
struct __Dereference<smart::Ptr<T> > {
  typedef T* dereference_type ;
};

/* Макросы для создания конструкторов {*/
#define _CM(v) Sm::__Dereference<typeof(v)>::dereference_type _ ## v

// защита от повторного создания и/или разламывания ссылок при динамической генерации шаблонных функций

#define MK_CONSTRUCT1(a) a(_ ## a)
#define MK_CONSTRUCT2(a,b)               MK_CONSTRUCT1(a), MK_CONSTRUCT1(b)
#define MK_CONSTRUCT3(a,b,c)             MK_CONSTRUCT2(a,b), MK_CONSTRUCT1(c)
#define MK_CONSTRUCT4(a,b,c,d)           MK_CONSTRUCT3(a,b,c), MK_CONSTRUCT1(d)
#define MK_CONSTRUCT5(a,b,c,d,e)         MK_CONSTRUCT4(a,b,c,d), MK_CONSTRUCT1(e)
#define MK_CONSTRUCT6(a,b,c,d,e,f)       MK_CONSTRUCT5(a,b,c,d,e), MK_CONSTRUCT1(f)
#define MK_CONSTRUCT7(a,b,c,d,e,f,g)     MK_CONSTRUCT6(a,b,c,d,e,f), MK_CONSTRUCT1(g)
#define MK_CONSTRUCT8(a,b,c,d,e,f,g,h)   MK_CONSTRUCT7(a,b,c,d,e,f,g), MK_CONSTRUCT1(h)
#define MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i) MK_CONSTRUCT8(a,b,c,d,e,f,g,h), MK_CONSTRUCT1(i)

#define DEF_UND_CONSTRUCTOR0( Tp,Base,Cathegory) Tp ()              : Base(   Cathegory)
#define DEF_UND_LCONSTRUCTOR0(Tp,Base,Cathegory) Tp (CLoc l) : Base(l, Cathegory)
#define DEF_UNDG_LCONSTRUCTOR0(Tp,Base,Cathegory) Tp (CLoc l) : Base(Cathegory), Grammar(l)

#define HARG1(a) _CM(a)
#define DEF_H_CONSTRUCTOR1(    Tp,               a) Tp (               HARG1(a))
#define DEF_CONSTRUCTOR1(      Tp,               a) Tp (               HARG1(a)) :                     MK_CONSTRUCT1(a)
#define DEF_H_LCONSTRUCTOR1(   Tp,               a) Tp (CLoc l, HARG1(a)) : Grammar(l)
#define DEF_LCONSTRUCTOR1(     Tp,               a) Tp (CLoc l, HARG1(a)) : GrammarBaseSmart(l),         MK_CONSTRUCT1(a)
#define DEF_LUCONSTRUCTOR1(     Tp,               a) Tp (CLoc l, HARG1(a)) : GrammarBase(l),         MK_CONSTRUCT1(a)
#define DEF_LBCONSTRUCTOR1(     Tp,               a) Tp (CLoc l, HARG1(a)) : GrammarBaseSmart(l),         MK_CONSTRUCT1(a)
#define DEF_UND_LCONSTRUCTOR1( Tp,Base,Cathegory,a) Tp (CLoc l, HARG1(a)) : Base(l, Cathegory), MK_CONSTRUCT1(a)
#define DEF_VUND_LCONSTRUCTOR1( Tp,Base,a) Tp (CLoc l, HARG1(a)) : Base(l), MK_CONSTRUCT1(a)
#define DEF_UND_CONSTRUCTOR1(  Tp,Base,Cathegory,a) Tp (               HARG1(a)) : Base(   Cathegory), MK_CONSTRUCT1(a)
#define DEF_UNDG_LCONSTRUCTOR1( Tp,Base,Cathegory,a) Tp (CLoc l, HARG1(a)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT1(a)

#define HARG2(    a,b)         HARG1(a),_CM(b)
#define DEF_H_CONSTRUCTOR2(    Tp,                a,b) Tp (               HARG2(a,b))
#define DEF_CONSTRUCTOR2(      Tp,                a,b) Tp (               HARG2(a,b)) :                     MK_CONSTRUCT2(a,b)
#define DEF_H_LCONSTRUCTOR2(   Tp,                a,b) Tp (CLoc l, HARG2(a,b)) : Grammar(l)
#define DEF_LCONSTRUCTOR2(     Tp,                a,b) Tp (CLoc l, HARG2(a,b)) : GrammarBaseSmart(l),         MK_CONSTRUCT2(a,b)
#define DEF_LUCONSTRUCTOR2(     Tp,                a,b) Tp (CLoc l, HARG2(a,b)) : GrammarBase(l),         MK_CONSTRUCT2(a,b)
#define DEF_LBCONSTRUCTOR2(    Tp,                a,b) Tp (CLoc l, HARG2(a,b)) : GrammarBaseSmart(l),    MK_CONSTRUCT2(a,b)
#define DEF_UND_LCONSTRUCTOR2( Tp,Base,Cathegory, a,b) Tp (CLoc l, HARG2(a,b)) : Base(l, Cathegory), MK_CONSTRUCT2(a,b)
#define DEF_VUND_LCONSTRUCTOR2(Tp,Base,a,b) Tp (CLoc l, HARG2(a,b)) : Base(l), MK_CONSTRUCT2(a,b)
#define DEF_UND_CONSTRUCTOR2(  Tp,Base,Cathegory, a,b) Tp (               HARG2(a,b)) : Base(   Cathegory), MK_CONSTRUCT2(a,b)
#define DEF_UNDG_LCONSTRUCTOR2(Tp,Base,Cathegory, a,b) Tp (CLoc l, HARG2(a,b)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT2(a,b)

#define HARG3(    a,b,c)         HARG2(a,b),_CM(c)
#define DEF_H_CONSTRUCTOR3(    Tp,                a,b,c) Tp (               HARG3(a,b,c))
#define DEF_CONSTRUCTOR3(      Tp,                a,b,c) Tp (               HARG3(a,b,c)) :                     MK_CONSTRUCT3(a,b,c)
#define DEF_H_LCONSTRUCTOR3(   Tp,                a,b,c) Tp (CLoc l, HARG3(a,b,c)) : Grammar(l)
#define DEF_LCONSTRUCTOR3(     Tp,                a,b,c) Tp (CLoc l, HARG3(a,b,c)) : GrammarBaseSmart(l),         MK_CONSTRUCT3(a,b,c)
#define DEF_LUCONSTRUCTOR3(     Tp,                a,b,c) Tp (CLoc l, HARG3(a,b,c)) : GrammarBase(l),         MK_CONSTRUCT3(a,b,c)
#define DEF_LBCONSTRUCTOR3(    Tp,                a,b,c) Tp (CLoc l, HARG3(a,b,c)) : GrammarBaseSmart(l),     MK_CONSTRUCT3(a,b,c)
#define DEF_UND_LCONSTRUCTOR3( Tp,Base,Cathegory, a,b,c) Tp (CLoc l, HARG3(a,b,c)) : Base(l, Cathegory), MK_CONSTRUCT3(a,b,c)
#define DEF_VUND_LCONSTRUCTOR3( Tp,Base,a,b,c) Tp (CLoc l, HARG3(a,b,c)) : Base(l), MK_CONSTRUCT3(a,b,c)
#define DEF_UND_CONSTRUCTOR3(  Tp,Base,Cathegory, a,b,c) Tp (               HARG3(a,b,c)) : Base(   Cathegory), MK_CONSTRUCT3(a,b,c)
#define DEF_UNDG_LCONSTRUCTOR3( Tp,Base,Cathegory, a,b,c) Tp (CLoc l, HARG3(a,b,c)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT3(a,b,c)

#define HARG4(    a,b,c,d)         HARG3(a,b,c),_CM(d)
#define DEF_H_CONSTRUCTOR4(    Tp,                a,b,c,d) Tp (               HARG4(a,b,c,d))
#define DEF_CONSTRUCTOR4(      Tp,                a,b,c,d) Tp (               HARG4(a,b,c,d)) :                     MK_CONSTRUCT4(a,b,c,d)
#define DEF_H_LCONSTRUCTOR4(   Tp,                a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : Grammar(l)
#define DEF_LCONSTRUCTOR4(     Tp,                a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : Grammar(l),         MK_CONSTRUCT4(a,b,c,d)
#define DEF_LUCONSTRUCTOR4(     Tp,                a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : GrammarBase(l),         MK_CONSTRUCT4(a,b,c,d)
#define DEF_LBCONSTRUCTOR4(    Tp,                a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : GrammarBaseSmart(l),     MK_CONSTRUCT4(a,b,c,d)
#define DEF_UND_LCONSTRUCTOR4( Tp,Base,Cathegory, a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : Base(l, Cathegory), MK_CONSTRUCT4(a,b,c,d)
#define DEF_VUND_LCONSTRUCTOR4( Tp,Base,a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : Base(l), MK_CONSTRUCT4(a,b,c,d)
#define DEF_UND_CONSTRUCTOR4(  Tp,Base,Cathegory, a,b,c,d) Tp (               HARG4(a,b,c,d)) : Base(   Cathegory), MK_CONSTRUCT4(a,b,c,d)
#define DEF_UNDG_LCONSTRUCTOR4( Tp,Base,Cathegory, a,b,c,d) Tp (CLoc l, HARG4(a,b,c,d)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT4(a,b,c,d)

#define HARG5(    a,b,c,d,e)         HARG4(a,b,c,d),_CM(e)
#define DEF_H_CONSTRUCTOR5(    Tp,                a,b,c,d,e) Tp (               HARG5(a,b,c,d,e))
#define DEF_CONSTRUCTOR5(      Tp,                a,b,c,d,e) Tp (               HARG5(a,b,c,d,e)) :                     MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_H_LCONSTRUCTOR5(   Tp,                a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : Grammar(l)
#define DEF_LCONSTRUCTOR5(     Tp,                a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : GrammarBaseSmart(l),         MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_LUCONSTRUCTOR5(     Tp,                a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : GrammarBase(l),         MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_LBCONSTRUCTOR5(    Tp,                a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : GrammarBaseSmart(l),     MK_CONSTRUCT5(a,b,c,d,e)

#define DEF_LBCONSTRUCTOR5_h(    Tp,                a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e));
#define DEF_LBCONSTRUCTOR5_b(    Tp,                a,b,c,d,e) Tp::Tp (CLoc l, HARG5(a,b,c,d,e)) : GrammarBaseSmart(l),     MK_CONSTRUCT5(a,b,c,d,e)

#define DEF_UND_LCONSTRUCTOR5( Tp,Base,Cathegory, a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : Base(l, Cathegory), MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_VUND_LCONSTRUCTOR5( Tp,Base,a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : Base(l), MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_UND_CONSTRUCTOR5(  Tp,Base,Cathegory, a,b,c,d,e) Tp (               HARG5(a,b,c,d,e)) : Base(   Cathegory), MK_CONSTRUCT5(a,b,c,d,e)
#define DEF_UNDG_LCONSTRUCTOR5( Tp,Base,Cathegory, a,b,c,d,e) Tp (CLoc l, HARG5(a,b,c,d,e)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT5(a,b,c,d,e)

#define HARG6(    a,b,c,d,e,f)         HARG5(a,b,c,d,e),_CM(f)
#define DEF_H_CONSTRUCTOR6(    Tp,                a,b,c,d,e,f) Tp (               HARG6(a,b,c,d,e,f))
#define DEF_CONSTRUCTOR6(      Tp,                a,b,c,d,e,f) Tp (               HARG6(a,b,c,d,e,f)) :                     MK_CONSTRUCT6(a,b,c,d,e,f)
#define DEF_H_LCONSTRUCTOR6(   Tp,                a,b,c,d,e,f) Tp (CLoc l, HARG6(a,b,c,d,e,f)) : Grammar(l)
#define DEF_LCONSTRUCTOR6(     Tp,                a,b,c,d,e,f) Tp (CLoc l, HARG6(a,b,c,d,e,f)) : Grammar(l),         MK_CONSTRUCT6(a,b,c,d,e,f)
#define DEF_LBCONSTRUCTOR6(    Tp,                a,b,c,d,e,f) Tp (CLoc l, HARG6(a,b,c,d,e,f)) : GrammarBaseSmart(l),     MK_CONSTRUCT6(a,b,c,d,e,f)
#define DEF_UND_LCONSTRUCTOR6( Tp,Base,Cathegory, a,b,c,d,e,f) Tp (CLoc l, HARG6(a,b,c,d,e,f)) : Base(l, Cathegory), MK_CONSTRUCT6(a,b,c,d,e,f)
#define DEF_UND_CONSTRUCTOR6(  Tp,Base,Cathegory, a,b,c,d,e,f) Tp (               HARG6(a,b,c,d,e,f)) : Base(   Cathegory), MK_CONSTRUCT6(a,b,c,d,e,f)
#define DEF_UNDG_LCONSTRUCTOR6( Tp,Base,Cathegory, a,b,c,d,e,f) Tp (CLoc l, HARG6(a,b,c,d,e,f)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT6(a,b,c,d,e,f)

#define HARG7(    a,b,c,d,e,f,g)         HARG6(a,b,c,d,e,f),_CM(g)
#define DEF_H_CONSTRUCTOR7(    Tp,                a,b,c,d,e,f,g) Tp (               HARG7(a,b,c,d,e,f,g))
#define DEF_CONSTRUCTOR7(      Tp,                a,b,c,d,e,f,g) Tp (               HARG7(a,b,c,d,e,f,g)) :                     MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_H_LCONSTRUCTOR7(   Tp,                a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : Grammar(l)
#define DEF_LCONSTRUCTOR7(     Tp,                a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : Grammar(l),         MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_LBCONSTRUCTOR7(    Tp,                a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : GrammarBaseSmart(l),     MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_UND_LCONSTRUCTOR7( Tp,Base,Cathegory, a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : Base(l, Cathegory), MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_VUND_LCONSTRUCTOR7( Tp,Base, a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : Base(l), MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_UND_CONSTRUCTOR7(  Tp,Base,Cathegory, a,b,c,d,e,f,g) Tp (               HARG7(a,b,c,d,e,f,g)) : Base(   Cathegory), MK_CONSTRUCT7(a,b,c,d,e,f,g)
#define DEF_UNDG_LCONSTRUCTOR7( Tp,Base,Cathegory, a,b,c,d,e,f,g) Tp (CLoc l, HARG7(a,b,c,d,e,f,g)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT7(a,b,c,d,e,f,g)

#define HARG8(    a,b,c,d,e,f,g,h)         HARG7(a,b,c,d,e,f,g),_CM(h)
#define DEF_H_CONSTRUCTOR8(    Tp,                a,b,c,d,e,f,g,h) Tp (               HARG8(a,b,c,d,e,f,g,h))
#define DEF_CONSTRUCTOR8(      Tp,                a,b,c,d,e,f,g,h) Tp (               HARG8(a,b,c,d,e,f,g,h)) :                     MK_CONSTRUCT8(a,b,c,d,e,f,g,h)
#define DEF_LCONSTRUCTOR8(     Tp,                a,b,c,d,e,f,g,h) Tp (CLoc l, HARG8(a,b,c,d,e,f,g,h)) : Grammar(l),         MK_CONSTRUCT8(a,b,c,d,e,f,g,h)
#define DEF_LBCONSTRUCTOR8(    Tp,                a,b,c,d,e,f,g,h) Tp (CLoc l, HARG8(a,b,c,d,e,f,g,h)) : GrammarBaseSmart(l),     MK_CONSTRUCT8(a,b,c,d,e,f,g,h)
#define DEF_H_LCONSTRUCTOR8(   Tp,                a,b,c,d,e,f,g,h) Tp (CLoc l, HARG8(a,b,c,d,e,f,g,h)) : Grammar(l)
#define DEF_UND_LCONSTRUCTOR8( Tp,Base,Cathegory, a,b,c,d,e,f,g,h) Tp (CLoc l, HARG8(a,b,c,d,e,f,g,h)) : Base(l, Cathegory), MK_CONSTRUCT8(a,b,c,d,e,f,g,h)
#define DEF_UND_CONSTRUCTOR8(  Tp,Base,Cathegory, a,b,c,d,e,f,g,h) Tp (               HARG8(a,b,c,d,e,f,g,h)) : Base(   Cathegory), MK_CONSTRUCT8(a,b,c,d,e,f,g,h)
#define DEF_UNDG_LCONSTRUCTOR8( Tp,Base,Cathegory, a,b,c,d,e,f,g,h) Tp (CLoc l, HARG8(a,b,c,d,e,f,g,h)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT8(a,b,c,d,e,f,g,h)

#define HARG9(    a,b,c,d,e,f,g,h,i)         HARG8(a,b,c,d,e,f,g,h),_CM(i)
#define DEF_H_CONSTRUCTOR9(   Tp,a,b,c,d,e,f,g,h,i) Tp                (               HARG9(a,b,c,d,e,f,g,h,i))
#define DEF_CONSTRUCTOR9(     Tp,a,b,c,d,e,f,g,h,i) Tp                (               HARG9(a,b,c,d,e,f,g,h,i)) :                     MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)
#define DEF_H_LCONSTRUCTOR9(  Tp,a,b,c,d,e,f,g,h,i) Tp                (CLoc l, HARG9(a,b,c,d,e,f,g,h,i)) : Grammar(l)
#define DEF_LCONSTRUCTOR9(    Tp,a,b,c,d,e,f,g,h,i) Tp                (CLoc l, HARG9(a,b,c,d,e,f,g,h,i)) : Grammar(l),         MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)
#define DEF_LBCONSTRUCTOR9(   Tp,a,b,c,d,e,f,g,h,i) Tp                (CLoc l, HARG9(a,b,c,d,e,f,g,h,i)) : GrammarBaseSmart(l),     MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)
#define DEF_UND_LCONSTRUCTOR9(Tp,Base,Cathegory,a,b,c,d,e,f,g,h,i) Tp (CLoc l, HARG9(a,b,c,d,e,f,g,h,i)) : Base(l, Cathegory), MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)
#define DEF_UND_CONSTRUCTOR9( Tp,Base,Cathegory,a,b,c,d,e,f,g,h,i) Tp (               HARG9(a,b,c,d,e,f,g,h,i)) : Base(   Cathegory), MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)
#define DEF_UNDG_LCONSTRUCTOR9(Tp,Base,Cathegory,a,b,c,d,e,f,g,h,i) Tp (CLoc l, HARG9(a,b,c,d,e,f,g,h,i)) : Base(Cathegory), Grammar(l), MK_CONSTRUCT9(a,b,c,d,e,f,g,h,i)

/*}*/

/* Макросы для создания семантических деревьев { */






/* } */

template <typename Element, typename Container>
class BaseContainer : public Smart, public Container
{
public:
  BaseContainer() {}
  BaseContainer(const BaseContainer & x) : SingleSmart(), Container(const_cast<BaseContainer&>(x)) {}
  BaseContainer(Ptr<Element> x) : Container(1, x) {}
  BaseContainer(Element     *x) { Ptr<Element> p = x; this->push_back(p); }
  BaseContainer(std::initializer_list<Ptr<Element> > lst)  : Container(lst) {}
  BaseContainer(Ptr<Element> x, BaseContainer<Element, Container> *tail) : Container(1, x)
  {
    if (tail) this->insert(this->end(), tail->begin(), tail->end());
  }
  BaseContainer(BaseContainer<Element, Container> *startList, Ptr<Element> tail )
    : Container(startList ? *startList : Container()) { this->push_back(tail); }
};

template <typename Element>
class BaseList : public BaseContainer<Element, list<Ptr<Element> > >
{
  typedef BaseContainer<Element, list<Ptr<Element> > > BaseClass;
public:
  BaseList() {}
  BaseList(const BaseList & x) : BaseClass(x) {}
  BaseList(Ptr<Element> x)     : BaseClass(x) {}
  BaseList(Element* x)         : BaseClass(x) {}
  BaseList(Ptr<Element> x, BaseList<Element> *tail) : BaseClass(x, tail) {}
  BaseList(BaseList<Element> *startList, Ptr<Element> tail) : BaseClass(startList, tail) {}
  BaseList(std::initializer_list<Ptr<Element> > lst)  : BaseClass(lst) {}
};


template <typename T, typename Manip>
void translateListByManip(const T &obj, Sm::Codestream &str) {
  if (obj.empty())
    return;
  T* deconst = (T*)&obj;
  typename T::iterator it = deconst->begin();
  if (unsigned int end = str.skipListFromStart()) {
    if (end >= obj.size()) {
      str.skipListFromStart(0);
      return;
    }
    for (unsigned int i = 0; i < end; ++i)
      ++it;
    str.skipListFromStart(0);
  }

  if (*it)
    str << s::def << (*it);
  for (++it; it != deconst->end(); ++it)
    str << s::comma() << Manip() << (*it);
}

template <typename T>
void translateList(Sm::Codestream &str, const T &obj) {
  switch (str.namesMode()) {
  case Sm::CodestreamState::DEFINITION:
    translateListByManip<T, Def>(obj, str);
    break;
  case Sm::CodestreamState::DECLARATION:
    translateListByManip<T, Decl>(obj, str);
    break;
  case Sm::CodestreamState::REFERENCE:
    translateListByManip<T, Ref1>(obj, str);
    break;
  }
}

template <typename Element>
class List : public BaseList<Element>
{
public:
  List() {}
  List(const List & o) : BaseList<Element>(o) {}
  List(Ptr<Element> x) : BaseList<Element>(x) {}
  List(Element *x)     : BaseList<Element>(x) {}
  List(Ptr<Element> x, Ptr<List<Element> > tail) : BaseList<Element>(x, tail.object()) {}
  List(Ptr<List<Element> > startList, Ptr<Element> tail) : BaseList<Element>(startList.object(), tail) {}

  List(std::initializer_list<Ptr<Element> > lst)  : BaseList<Element>(lst) {}

  bool isDefinition() const { return false; }
  bool hasBrackets() const { return this->__flags__.v & FLAG_LIST_ELEMENT_HAS_BRACKETS; }
  void setHasBrackets() { this->__flags__.v |= FLAG_LIST_ELEMENT_HAS_BRACKETS; }

  Sm::Codestream &translate(Sm::Codestream &str) { translateList<List<Element> >(str, *this); return str; }
};

template <typename Element>
class List;

class StatementInterface;

typedef std::function<int (StatementInterface*, bool /*constructor*/,
                           list<Ptr<StatementInterface> > &/*stmts*/,
                           list<Ptr<StatementInterface> >::iterator &/*it*/
                          )> const StmtTrCond;

typedef std::function<StatementInterface*(StatementInterface*,
                                          list<Ptr<StatementInterface> > *stmts,
                                          list<Ptr<StatementInterface> >::iterator *it)> const StmtTr;
typedef std::function<bool(StatementInterface*, bool /*isConstructor*/, bool /*hasSublevels*/)> const StatementActor;



template <typename Element>
class Vector : public BaseContainer<Element, vector<Ptr<Element> > >
{
  typedef BaseContainer<Element, vector<Ptr<Element> > > BaseClass;
public:
  Vector() {}
  Vector(const Vector &x) : BaseClass(x) {}
  Vector(Ptr<Element> x)  : BaseClass(x) {}
  Vector(std::initializer_list<Ptr<Element> > lst)  : BaseClass(lst) {}
  Vector(Ptr<Element> x, Vector<Element> *tail) : BaseClass(x, tail) {}
  Vector(Vector<Element> *startList, Ptr<Element> tail) : BaseClass(startList, tail) {}

  Sm::Codestream &translate(Sm::Codestream &str) { translateList<Vector<Element> >(str, *this); return str; }

  size_t bSize(SmartptrSet &s) const;
};





template <typename T>
size_t getContainerSize(const T &c) { return c.capacity(); }

template <typename T>
size_t getContainerSize(const std::set<T>  &c) {
  return c.size();
}

template <typename T, typename V>
size_t getContainerSize(const std::set<T, V> &c) {
  return c.size();
}

template <typename T, typename V>
size_t getContainerSize(const std::map<T, V> &c) {
  return c.size();
}

template <typename T, typename V, typename C>
size_t getContainerSize(const std::map<T, V, C> &c) {
  return c.size();
}


template <typename Element>
size_t Vector<Element>::bSize(SmartptrSet &s) const {
  if (!s.insert(this).second)
    return 0;
  size_t sz = getContainerSize(*this) * sizeof(Vector<Element>);
  for (const typename Vector<Element>::value_type &v: *this)
    sz += v->bSize(s);
  return sz;
}

template <typename T>
inline Codestream &operator<<(Codestream& str, List<T> &obj) { return obj.translate(str); }

class Constraint;
class IdEntitySmart;

typedef std::set<ResolvedEntity*, LE_ResolvedEntities> Definitions;
typedef std::vector<Ptr<Sm::Id> > EntityFields;
typedef std::vector<Sm::StatementInterface*> FoundedStatements;

void debugTranslateEntityFields(Codestream &str, EntityFields &flds);



Codestream& operator<< (Codestream& os, const IdEntitySmart& str);
std::ostream& operator<< (std::ostream& os, const IdEntitySmart& str);

class IdEntitySmart : public SingleSmart, public vector<Ptr<Id> >
{
  friend Codestream& operator<< (Codestream& os, const IdEntitySmart& str);
protected:
  SemanticTree *toSTreeRefBase(SCathegory::t t) const;
public:
  typedef vector<Ptr<Id> > BaseType;
  IdEntitySmart();
  IdEntitySmart(const IdEntitySmart &o);
  IdEntitySmart(const IdEntitySmart &o, bool deepCopy);
  IdEntitySmart(IdEntitySmart::iterator begIt, IdEntitySmart::iterator endIt);
  IdEntitySmart(IdEntitySmart::const_iterator begIt, IdEntitySmart::const_iterator endIt);

//  inline IdEntitySmart(Ptr<IdEntitySmart> o);
  IdEntitySmart(Ptr<Id> &&x);
  IdEntitySmart(Id *x);
  IdEntitySmart(Id2 *x);

  IdEntitySmart(Ptr<Id> tail, Ptr<Id> front);

  IdEntitySmart(IdEntitySmart &tail, Ptr<Id> front);

  IdEntitySmart(std::initializer_list<std::string> strList);

  inline IdEntitySmart(string &&str, bool quoted = false);

  void pop_front();

  Ptr<Id> entity() const;

  void resolveByModelContext();

  inline Ptr<Id> parentInList(Ptr<Id> &field) const;

  Ptr<Id> majorBaseEntity() const;
  Ptr<Id> majorEntity() const;
  Ptr<Id> majorObjectRef(ResolvedEntity **ppObjType) const;
  IdEntitySmart::iterator majorEntityIter();
  inline bool isEqual(const IdEntitySmart &oth) const;
  bool isResolved() const;
  ResolvedEntity *definition() const;
  bool isDynamicUsing() const;
  void setIsDynamicUsing() { __flags__.v |= FLAG_RESOLVED_ENTITY_IS_DYNAMIC_USING; }

  Sm::Codestream &translate(Sm::Codestream &str);
  inline void semanticNode(const SemanticTree *node);

  void toNormalizedString(string &dst) const;
  std::string toNormalizedString() const { std::string res; toNormalizedString(res); return res; }

  void toStringWithType(string &dst) const;

  inline bool getFields(EntityFields &fields) const;

  void toQString         (string &dst) const;
  void toSyntaxQString   (string &dst, SCathegory::t t = SCathegory::EMPTY) const;
  void toResolvedQString (string &dst, SCathegory::t t = SCathegory::EMPTY) const;
  void toP2PResolvedQString(string &dst) const;
  string toP2PResolvedQString() const { string res; toP2PResolvedQString(res); return res; }


  bool updateQuotedUserIdToVarchar2Literal();
  bool updateUnresolvedQuotedToVarchar2Literal();

  inline SemanticTree *toSTreeRef(SCathegory::t t) const;

  Ptr<Datatype> getDatatype() const;
  CLoc getLLoc() const;

  size_t bSize(SmartptrSet &s) const;

};


inline void checkIdEntitySmart(IdEntitySmart *v) {
  IdEntitySmart *p = v;
  if (!p || p->empty() || !(p->begin()->object()))
    throw 999;
}


typedef IdEntitySmart IdRef;


template <> class List<Id>;
template <> class List<Id2>;


class TranslatorTailMakestr {
public:
  virtual void translateTail(Sm::Codestream &) {}
};

class ManualEntity {
public:
  Sm::IdEntitySmart reference;
  Sm::IdEntitySmart sqls;
  bool              stored = false;
};

}

#endif
// vim:foldmethod=marker:foldmarker={,}
