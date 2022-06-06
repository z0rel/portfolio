#ifndef SEMANTIC_TREE_H
#define SEMANTIC_TREE_H

#include <limits>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <list>
#include <initializer_list>
#include <stdint.h>
#include <gmpxx.h>
#include <utility>
#include <memory>
#include <set>
#include <map>
#include <unordered_set>
#include "i2str.h"
#include "dstring.h"
#include "hash_table.h"
#include "model_head.h"
#include "semantic_base.h"
#include "semantic_utility.h"

class ModelContext;

void globalCleanup();

extern SemanticTreeManager streeManager;


namespace Sm {

class ResolvingContext;
class CursorDecltype;


struct AllowedMasks {
  vector<SCathegory::t> cathegories;
  vector<SCathegory::t> masks;
};

// > 0 LHS >= RHS, RHS casted to LHS
// < 0 LHS <= RHS, LHS casted to RHS


#define CAST_EXPLICIT_ALL                          0
#define CAST_EQUALLY_BY_BASE                       (0)
#define CAST_HS_BASE_OFFSET                        12
#define CAST_LENGTH_CATHEGORY_OFFSET               (CAST_HS_BASE_OFFSET            + 2)
#define CAST_RELATION_PRECISION_OFFSET             (CAST_LENGTH_CATHEGORY_OFFSET   + 2)
#define CAST_RELATION_SCALE_OFFSET                 (CAST_RELATION_PRECISION_OFFSET + 2)
#define CAST_RELATION_LENGTH_OFFSET                (CAST_RELATION_SCALE_OFFSET     + 2)
#define CAST_STATE_OFFSET                          (CAST_RELATION_LENGTH_OFFSET    + 2)


#define CAST_IMPLICIT                              (1 << 0 )
#define CAST_EQUALLY                               (1 << 1 )
#define CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR   (1 << 2 )
#define CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE     (1 << 3 )
#define CAST_DATATYPES_UNRESOLVED_ERROR            (1 << 4 )
#define CAST_INTERNAL_TYPES_NUM_NEED_CMP_BY_LEN    (1 << 5 )
#define CAST_INTERNAL_TYPES_CHR_NEED_CMP_BY_LEN    (1 << 6 )
#define CAST_FUNCTION_CALLS_TO_REDUCED_RETTYPE     (1 << 7 )
#define CAST_ASSIGNMENT                            (1 << 8 )
#define CAST_RETURN                                (1 << 9 )

#define CAST_CAST_LHS_TO_RHS                       (1 << 10 )
#define CAST_CAST_RHS_TO_LHS                       (1 << 11 )
//12-23 SEE CAST_HS_BASE_OFFSET
#define CAST_IN_UNION                              (1 << 24 )

#define CAST_GET_RHS_BASE                          (1 << (CAST_HS_BASE_OFFSET   ) )
#define CAST_GET_LHS_BASE                          (1 << (CAST_HS_BASE_OFFSET+1 ) )
#define CAST_HS_BASE_MASK                          (CAST_GET_RHS_BASE | CAST_GET_LHS_BASE)
#define CAST_VARCHAR_CATHEGORY                     (1 << (CAST_LENGTH_CATHEGORY_OFFSET))
#define CAST_DECIMAL_CATHEGORY                     (1 << (CAST_LENGTH_CATHEGORY_OFFSET + 1))
#define CAST_BY_PREC_LHS_GT_RHS                    (1 << (CAST_RELATION_PRECISION_OFFSET) )
#define CAST_BY_PREC_LHS_LT_RHS                    (1 << (CAST_RELATION_PRECISION_OFFSET + 1))
#define CAST_BY_SCALE_LHS_GT_RHS                   (1 << (CAST_RELATION_SCALE_OFFSET))
#define CAST_BY_SCALE_LHS_LT_RHS                   (1 << (CAST_RELATION_SCALE_OFFSET + 1))
#define CAST_BY_SCALE_MASK                         (CAST_BY_SCALE_LHS_GT_RHS | CAST_BY_SCALE_LHS_LT_RHS)
#define CAST_BY_LENGTH_LHS_GT_RHS                  (1 << (CAST_RELATION_LENGTH_OFFSET))
#define CAST_BY_LENGTH_LHS_LT_RHS                  (1 << (CAST_RELATION_LENGTH_OFFSET + 1))
#define CAST_STATE_PROC                            (1 << (CAST_STATE_OFFSET))
#define CAST_STATE_SQL                             (1 << (CAST_STATE_OFFSET + 1))


#define CAST_EXPLICIT_SELECT                       (CAST_IMPLICIT | CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR)
#define CAST_EXPLICIT_RETURN                       (CAST_IMPLICIT | CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE  )
#define CAST_EXPLICIT_UNION                        (CAST_IMPLICIT | CAST_IN_UNION)
#define CAST_EXPLICIT_SELECT_AND_RETURN            (CAST_EXPLICIT_SELECT | CAST_EXPLICIT_RETURN             )
#define CAST_EXPLICIT_SELECT_UNION                 (CAST_IMPLICIT | CAST_EXPLICIT_SELECT | CAST_EXPLICIT_UNION)
#define CAST_EXPLICIT_SELECT_RETURN_UNION          (CAST_EXPLICIT_SELECT | CAST_EXPLICIT_RETURN | CAST_EXPLICIT_UNION )


#define CAST_CHAR_BY_LENGTH_GT                     (CAST_IMPLICIT | CAST_VARCHAR_CATHEGORY | CAST_BY_PREC_LHS_GT_RHS | CAST_BY_LENGTH_LHS_GT_RHS) // таблица преобразований несимметрична. field - всегда должен быть rhs,
                                                                                                                                                  // len(lhs=into var) < len(rhs=field)
#define CAST_CHAR_BY_LENGTH_LT                     (CAST_IMPLICIT | CAST_VARCHAR_CATHEGORY | CAST_BY_PREC_LHS_LT_RHS | CAST_BY_LENGTH_LHS_LT_RHS  | CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR)
#define CAST_NUM_BY_LENGTH_GT                      (CAST_IMPLICIT | CAST_DECIMAL_CATHEGORY | CAST_BY_LENGTH_LHS_GT_RHS)
#define CAST_NUM_BY_LENGTH_LT                      (CAST_IMPLICIT | CAST_DECIMAL_CATHEGORY | CAST_BY_LENGTH_LHS_LT_RHS)
#define CAST_NUM_GT_PREC_LT_SCALE                  (CAST_IMPLICIT | CAST_DECIMAL_CATHEGORY | CAST_BY_PREC_LHS_GT_RHS | CAST_BY_SCALE_LHS_LT_RHS | CAST_BY_LENGTH_LHS_GT_RHS | CAST_BY_LENGTH_LHS_LT_RHS)
#define CAST_NUM_LT_PREC_GT_SCALE                  (CAST_IMPLICIT | CAST_DECIMAL_CATHEGORY | CAST_BY_PREC_LHS_LT_RHS | CAST_BY_SCALE_LHS_GT_RHS | CAST_BY_LENGTH_LHS_GT_RHS | CAST_BY_LENGTH_LHS_LT_RHS)


#define CAST_STRONG_EQUALLY                        (CAST_EQUALLY | CAST_EQUALLY_BY_BASE)

#define BIT01OR(a)               (((a) & 1) | (((a) & 2) >> 1))

#define CAST_DECODE_HS_BASE(a)   ((a) ^ (BIT01OR(a) | (BIT01OR(a) << 1)))
#define CAST_DECODE_ITEM(a, off) (((a) >> off) & 3)
#define CAST_HS_BASE(a)          CAST_DECODE_ITEM(a, CAST_HS_BASE_OFFSET)


class CastCathegory {
public:
  enum DebugBasePriority { EQ_HS_BASE_DEBUG = 0, LHS_BASE_DEBUG = 2,  RHS_BASE_DEBUG = 1, EQ_HS_BASE_DEBUG1 = 3 };
  enum BasePriority { EQ_HS_BASE = 0, LHS_BASE = 1,  RHS_BASE = 2 };
  enum LengthCathegory { EMPTY = 0, VARCHAR = 1, DECIMAL = 2, ERROR_CATHEGORY = 3 };
  enum Relation        { EQ = 0, GT = 1, LT = 2, ERROR_RELATION = 3 };
  enum CastingState { EMPTY_STATE = 0, PROC = 1, SQL = 2, ERROR_STATE = 3 };


  union {
    struct {
      unsigned int implicit                      :1; // 0   CAST_IMPLICIT
      unsigned int equally                       :1; // 1   CAST_EQUALLY
      unsigned int inSelectRhsFieldToIntoLhsVar  :1; // 2   CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR
      unsigned int inReturnRhsFactToLhsRettype   :1; // 3   CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE
      unsigned int datatypesUnresolvedError      :1; // 4   CAST_DATATYPES_UNRESOLVED_ERROR
      unsigned int                               :1; // 5   CAST_INTERNAL_TYPES_NUM_NEED_CMP_BY_LEN
      unsigned int                               :1; // 6   CAST_INTERNAL_TYPES_CHR_NEED_CMP_BY_LEN

      unsigned int funcallsToReducedRettype      :1; // 7   CAST_FUNCTION_CALLS_TO_REDUCED_RETTYPE
      unsigned int castAssignment                :1; // 8   CAST_ASSIGNMENT
      unsigned int castReturn                    :1; // 9   CAST_RETURN
      unsigned int castLhsToRhs                  :1; // 10;
      unsigned int castRhsToLhs                  :1; // 11;
      DebugBasePriority hsBase                   :2; // 12,13
      LengthCathegory lengthCathegory            :2; // 14,15
      Relation        precisionRelation          :2; // 16,17
      Relation        scaleRelation              :2; // 18,19
      Relation        lengthRelation             :2; // 20,21
      CastingState    castingMode                :2; // 22,23
      unsigned int castUnion                     :1; // 24  CAST_IN_UNION
    } flags;
    unsigned int val;
  };

  CastCathegory() : val(0) {}
  CastCathegory(const CastCathegory &oth) : val(oth.val) {}
  CastCathegory(unsigned int cat) : val(cat) {}


  inline void setSqlCastState() { val |= CAST_STATE_SQL; }
  inline void setProcCastState() { val |= CAST_STATE_PROC; }
  inline void setIsCastFunctionCallsToReducedRettype() { val |= CAST_FUNCTION_CALLS_TO_REDUCED_RETTYPE; }
  inline bool isCastFunctionCallsToReducedRettype() { return val & CAST_FUNCTION_CALLS_TO_REDUCED_RETTYPE; }


  inline void setCastAssignment() { val |= CAST_ASSIGNMENT; }
  inline bool castAssignment() const { return val & CAST_ASSIGNMENT; }

  inline void setCastReturn() { val |= CAST_RETURN; }
  inline bool castReturn() const { return val & CAST_RETURN; }

  inline void setCastLhsToRhs() { val |= CAST_CAST_LHS_TO_RHS; }
  inline bool castLhsToRhs() const { return val & CAST_CAST_LHS_TO_RHS; }

  inline void setCastRhsToLhs() { val |= CAST_CAST_RHS_TO_LHS; }
  inline bool castRhsToLhs() const { return val & CAST_CAST_RHS_TO_LHS; }

  inline void setCastUnion() { val |= CAST_IN_UNION; }
  inline bool castUnion() const { return (val & CAST_IN_UNION) || explicitAll(); }

  inline CastingState castState() const { return (CastingState)(CAST_DECODE_ITEM(val, CAST_STATE_OFFSET)); }

  inline bool explicitAll() const { return !implicit(); }

  inline bool implicitFlag() const  { return val & CAST_IMPLICIT; }
  inline bool implicit() const  { return val & (CAST_IMPLICIT | CAST_EQUALLY); }
  inline bool implicitAlmost() const {
    return equally() || (implicit() && !(val & (CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE | CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR)) &&
                         (!varcharLength() || relationLength() == EQ));
  }

  /// соответствие по категории, длине и точности (но без учета приоритетов между CHAR и VARCHAR)
  inline bool equallyAlmost() const  { return val & CAST_EQUALLY; }
  /// строгое соответствие по категории, длине и точности
  inline bool equally() const  {
    return (equallyAlmost() && equallyBasePriority()) ||
           (
            ~(val & (CAST_HS_BASE_MASK | CAST_BY_SCALE_MASK)) && // equally по основанию и по
             (val & (CAST_VARCHAR_CATHEGORY | CAST_IMPLICIT))
           );
  }


  inline bool rhsBasePriority() const { return val & CAST_GET_RHS_BASE; }
  inline bool lhsBasePriority() const { return val & CAST_GET_LHS_BASE; }
  inline bool equallyBasePriority() const { return !(val & (CAST_GET_RHS_BASE | CAST_GET_LHS_BASE)); }

  inline BasePriority base() const { unsigned int a = CAST_HS_BASE(val); return (BasePriority)(CAST_DECODE_HS_BASE(a)); }

//  inline bool inSelectRhsField2IntoLhsVar() const { return val & CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR; }

  inline bool explicitInSelectLhsIntoRhsField() const { return explicitAll() || (val & CAST_IN_SELECT_RHS_FIELD_TO_INTO_LHS_VAR); }

  inline bool inReturnRhsVar2LhsRettype() const { return val & CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE; }

  inline bool explicitInReturn() const { return explicitAll() || (val & CAST_IN_RETURN_RHS_FACT_TO_LHS_RETTYPE); }
  inline bool implicitInReturn() const { return !inReturnRhsVar2LhsRettype() && implicit(); }

  inline bool varcharLength() const { return val & CAST_VARCHAR_CATHEGORY; }
  inline bool decimalLength() const { return val & CAST_DECIMAL_CATHEGORY; }
  inline LengthCathegory lengthCathegory() const { return (LengthCathegory)(CAST_DECODE_ITEM(val, CAST_LENGTH_CATHEGORY_OFFSET)); }

  inline bool precLhsGtRhs() const { return val & CAST_BY_PREC_LHS_GT_RHS; }
  inline bool precLhsLtRhs() const { return val & CAST_BY_PREC_LHS_LT_RHS; }
  inline Relation relationPrecision() const { return (Relation)(CAST_DECODE_ITEM(val, CAST_RELATION_PRECISION_OFFSET)); }

  inline bool scaleLhsGtRhs() const { return val & CAST_BY_SCALE_LHS_GT_RHS; }
  inline bool scaleLhsLtRhs() const { return val & CAST_BY_SCALE_LHS_LT_RHS; }
  inline Relation relationScale() const { return (Relation)(CAST_DECODE_ITEM(val, CAST_RELATION_SCALE_OFFSET)); }

  inline bool lengthLhsGtRhs() const { return val & CAST_BY_LENGTH_LHS_GT_RHS && !lengthLhsLtRhs(); }
  inline bool lengthLhsLtRhs() const { return val & CAST_BY_LENGTH_LHS_LT_RHS && !lengthLhsGtRhs(); }
  inline bool lengthErrorRelation() const { return lengthLhsGtRhs() && lengthLhsLtRhs(); }
  inline Relation relationLength() const { return (Relation)(CAST_DECODE_ITEM(val, CAST_RELATION_LENGTH_OFFSET)); }

  inline bool numGtPrecLtScale() const { return val & CAST_NUM_GT_PREC_LT_SCALE; }
  inline bool numLtPrecGtScale() const { return val & CAST_NUM_LT_PREC_GT_SCALE; }

  inline bool datatypeUnresolvedError() const { return val & CAST_DATATYPES_UNRESOLVED_ERROR; }


};







//#define CAST_CHAR_BY_LENGTH_GT              7
//#define CAST_EXPLICIT_LENGTH_NUMBER         8
//#define CAST_NUMBER_GT_PREC_LT_SCALE        9




//enum class CastedCathegory {
//  EMPTY              = 0,
//  NEED_EXPLICIT_CAST = 3,
//  NEED_EXPLICIT_CAST_IN_SELECT_INTO = 4,
//  NEED_EXPLICIT_CAST_IN_PL_SQL_QUERY_INTO = 10,
//  NEED_EXPLICIT_CAST_BY_LENGTH      = 7
//};


template <int a, int... args>
struct ConstMax {
  enum Max {
    v = a > ConstMax<args...>::v ? a : ConstMax<args...>::v
  };
};

template <int a>
struct ConstMax<a> { enum Max { v = a  }; };



class CommonDatatypeCast {
public:
  enum NumberCathegory {
    SMALLINT     = 0,
    INT          = 1,
    BIGINT       = 2,
    REAL_        = 3,
    DOUBLE       = 4,
    NUMBER       = 5,
    EMPTY        = 6
  };

  enum TypeCathegory {
    CHAR_VARCHAR            = 0,
    NCHAR                   = 1,
    NVARCHAR                = 2,
    LIKE_NUMBER             = 3,
    DATE                    = 4,
    CLOB                    = 5,
    NUMERIC_VALUE           = 6,
    NUMERIC_VALUE_0         = 7,
    QUOTED_SQL_EXPR_ID      = 8,
    EMPTY_ID                = 9,
    BOOL_                   = 10,
    NULLCAT                 = 11,

    UNINITIALIZED_CATHEGORY = 12
  };
private:
  static void getTypeCat(PlExpr *expr, Datatype *t, TypeCathegory &tCat, NumberCathegory &nCat);

public:
  struct CastContext {
    struct ContextFlags {
      bool sqlQuery             = false;
      bool sqlQueryIntoVariable = false;
      bool selectedField        = false;
      bool view                 = false;
      bool assignment           = false;
      bool functionCall         = false;
      bool functionDefinition   = false;
      bool compoundExpression   = false;
      bool concatExpression     = false;
      bool returnStatement      = false;
    };
    ContextFlags                f;
    SqlExpr                    *insertingValue = 0;
    ResolvedEntity             *fromNode = 0;
    Sm::View                   *view = 0;       // +
    std::vector<Sm::Function*>  functionDefinitions;
    std::vector<Sm::RefExpr*>   functionCalls;
    Sm::RefAbstract            *intoVariableExpression = 0;
    Sm::AlgebraicCompound      *compound = 0;   // +
    Sm::AlgebraicCompound      *concat   = 0;   // +
    Sm::QueryBlock             *queryBlock = 0; // +
    Sm::Subquery               *subquery = 0;   // +
    Sm::Assignment             *assignment = 0; // +
    Sm::Return                 *retStmt    = 0;
    Sm::Update                 *updateStmt = 0;
    Sm::DeleteFrom             *deleteStmt = 0;
    Sm::OpenFor                *openForStmt = 0;
    Sm::ResolvedEntity         *dynamicEntity = 0;

    // позиция в списке выборки и само выбираемое поле.
  };

  static std::string castedCathegoryToString(TypeCathegory c);

  static Sm::PlExpr *cast(Sm::PlExpr *castedExpr, Datatype *castedType, Datatype *newType, CastCathegory castCathegory);
  static void castAndReplace(bool isPl, Ptr<PlExpr> &castedExpr, Datatype *castedType, Datatype *newType, CastCathegory castCathegory);
  static void castAndReplace(bool isPl, Ptr<SqlExpr> &castedExpr, Datatype *castedType, Datatype *newType, CastCathegory castCathegory);

  static SqlExpr *convertExperssion(Sm::PlExpr *castedExpr, Datatype *newType, string nameOfConvFun, Datatype *castedType, const FLoc flc);
  static void castQuotedIdToNumber(SqlExpr **newExpr, Sm::PlExpr *castedExpr, Datatype *newType, Datatype* castedType, CastCathegory castCathegory, NumberCathegory newNumCat, FLoc l);
  static void castToNull(SqlExpr** newExpr, Sm::PlExpr *castedExpr);
};


struct SysDefCounters {
  typedef map<ResolvedEntity*, size_t, LE_ResolvedEntities> EntityMap;
  typedef map<std::string, std::vector<ResolvedEntity*> > AnalyticMap;
  typedef std::map<std::string, size_t> Map;
  typedef std::map<std::string, int> TriggerCathegories;

  Map         varrayCounters;
  Map         nestedTableCounters;
  Map         userenvArgumentsCounters;
  EntityMap   entitiesCounters;
  AnalyticMap analyticCounters;

  TriggerCathegories triggerCathegories;
  bool   triggerUsesParent    = false;
  size_t triggersThatUsesParentField = 0;

  size_t objectTableFieldsCounter = 0;
  size_t parentTriggerPseudorecordsCounters = 0;
  Map    objectTableFields;
  size_t nestedTableFieldsCounter = 0;
  size_t varrayFieldsCounter = 0;
  SysDefCounters()  {}


  size_t viewsCnt = 0;
  size_t functionsCnt = 0;
  size_t freeFunctionsCnt = 0;
  size_t packagesFunctionCnt = 0;
  size_t memberFunctionsCnt = 0;
  size_t triggersCnt = 0;

  typedef std::set<string> TypeLocations;

  TypeLocations xmltypeLocations;
  TypeLocations anydataLocations;


  std::vector<bool> inFunctionsStack;
  std::vector<Sm::Trigger*> inTriggerStack;
  bool inPackageFlag = false;
  void printFunCounters();
};

class SemanticTreeDdlEntityAttribute {
  ResolvedEntity* value = 0;
  size_t sid = getSid();
  static size_t getSid();
public:
  ResolvedEntity* operator= (ResolvedEntity* i);
  operator ResolvedEntity*() const { return value; }
  ResolvedEntity* operator->()     { return value; }
  SemanticTreeDdlEntityAttribute(ResolvedEntity* oth);
  SemanticTreeDdlEntityAttribute();
  SemanticTreeDdlEntityAttribute(const SemanticTreeDdlEntityAttribute &oth);
};




#define FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK    (1 << 0 | 1 << 1 | 1 << 2 | 1 << 3)
#define FLAG_SEMANTIC_TREE_OPEN_COUNT_CLRMASK (~FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK)
#define FLAG_SEMANTIC_TREE_IS_LIST            FLAG_SMART_BIT4
#define FLAG_SEMANTIC_TREE_IS_PL_CONTEXT      FLAG_SMART_BIT5
#define FLAG_SEMANTIC_TREE_IS_CHILDS_RESOLVED FLAG_SMART_BIT6
#define FLAG_SEMANTIC_TREE_IS_ALIAS           FLAG_SMART_BIT7
#define FLAG_SEMANTIC_TREE_IS_BASE_FOR_ALIAS  FLAG_SMART_BIT8
#define FLAG_SEMANTIC_TREE_IS_NOT_PL_CONTEXT  FLAG_SMART_BIT9
#define FLAG_SEMANTIC_TREE_IS_FROM_NODE       FLAG_SMART_BIT10
#define FLAG_SEMANTIC_TREE_IS_INTO_NODE       FLAG_SMART_BIT11
#define FLAG_SEMANTIC_TREE_IS_INSERTING_VALUE FLAG_SMART_BIT12
#define FLAG_SEMANTIC_TREE_IS_INVALID         FLAG_SMART_BIT13
#define FLAG_SEMANTIC_TREE_IS_DELETED_NODE    FLAG_SMART_BIT14
#define FLAG_SEMANTIC_TREE_HAS_DYNAMIC_CHILDS FLAG_SMART_BIT15

struct SemanticNameType {
  enum NameType : uint16_t {
    EMPTY              = 0,
    NEW_LEVEL          = 1,
    REFERENCE          = 2,
    EXTDB_REFERENCE    = 3,
    DATATYPE_REFERENCE = 4,
    DECLARATION        = 5,
    DEFINITION         = 6,
    LAST_NAMETYPE_IDX

  };
};

class ResolvingProgress {
public:
  size_t totalTraversed = 0;
};


#define UNRESOLVED_STATE_BY_NAME         (1 << 0)
#define UNRESOLVED_STATE_BY_SIGNATURE    (1 << 1)
#define UNRESOLVED_STATE_IS_DBLINK       (1 << 2)
#define UNRESOLVED_STATE_IS_PARTITIALLY  (1 << 3)
#define UNRESOLVED_STATE_NO_CONCRETE_DEF (1 << 4)

struct UnresolvedState {
  unsigned int state = 0;
  UnresolvedState() {}
  UnresolvedState(unsigned int _state) : state(_state) {}

  bool byName()        const { return state & UNRESOLVED_STATE_BY_NAME; }
  bool bySignature()   const { return state & UNRESOLVED_STATE_BY_SIGNATURE; }
  bool isDblink()      const { return state & UNRESOLVED_STATE_IS_DBLINK; }
  bool isPartitially() const { return state & UNRESOLVED_STATE_IS_PARTITIALLY; }
  bool noConcreteDef() const { return state & UNRESOLVED_STATE_NO_CONCRETE_DEF; }

  bool unresolvedNoDblink()      const { return state && !isDblink(); }

  bool unresolved()      const { return state != 0; }
  bool resolved()        const { return state == 0; }

  void setByName()        { state |= UNRESOLVED_STATE_BY_NAME; }
  void setBySignature()   { state |= UNRESOLVED_STATE_BY_SIGNATURE; }
  void setIsDblink()      { state |= UNRESOLVED_STATE_IS_DBLINK; }
  void setPartitially()   { state |= UNRESOLVED_STATE_IS_PARTITIALLY; }
  void setNoConcreteDef() { state |= UNRESOLVED_STATE_NO_CONCRETE_DEF; }

  UnresolvedState& operator|= (const UnresolvedState &right) { state |= right.state; return *this; }
};

inline UnresolvedState operator | (const UnresolvedState &left, const UnresolvedState &right) { return UnresolvedState(left.state | right.state); }

struct UnresolvedDataValue {
  IdEntitySmart    *reference = 0;
  cl::filelocation loc;
  SemanticTree    *node = 0;
  UnresolvedState  state;

  string toString() const;
};

class SemanticTree : public SemanticNameType // : public SingleSmart
{
  typedef std::map<ResolvedEntity*, std::set<ResolvedEntity*, LE_ResolvedEntities>, LE_ResolvedEntities> ReferencesForCastedCalls;
  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> UniqueDefinitions;
  static ReferencesForCastedCalls referencesForCastedCalls;
  static UniqueDefinitions globalUniqueDefinitions;

  static size_t streeGlobalCounter__;
  static ResolvingProgress resolvingProgress;
  size_t getStreeGlobalCounter__();
public:
  size_t sid = getStreeGlobalCounter__();

  static size_t constStreeGlobalCounter__() { return streeGlobalCounter__; }

private:

  typedef std::set<SemanticTree*> DeletedNodes;

  static DeletedNodes deletedNodes;
public:

  union PackedCathegory {
    uint32_t cat;
    struct Fields {
      uint16_t cathegory;
      uint16_t nametype;
    } __attribute__((packed));
    Fields fields;
  } __attribute__((packed));


  typedef list<SemanticTree*> Childs;
  static SysDefCounters globalSysDefCounters;

  typedef Sm::UniqueEntitiesMap UniqueEntitiesMap;

private:
  SemanticTree *parent = 0; // 8
  SemanticTree::Childs::iterator positionInParent; // 16

public:
  ResolvedEntity *unnamedDdlEntity = 0; // 24
//  SemanticTreeDdlEntityAttribute unnamedDdlEntity;

  SCathegory::t cathegory = SCathegory::EMPTY; // 28

  NameType nametype = EMPTY; // 30 ??
  typedef uint32_t FlagsType;
  FlagsType flags = 0;      // 32 ??

protected:
  Ptr<IdEntitySmart> referenceName_;   // 40
public:
  Ptr<LevelResolvedNamespace> levelNamespace; // 48
  Ptr<LevelResolvedNamespace> childNamespace; // 56

public:
  /// Элементы, семантически принадлежащие узлу.
  Childs childs; // 64


  SemanticTree& operator=(const SemanticTree& tr);

  SemanticTree(SCathegory::t _cathegory = SCathegory::EMPTY, NameType ntype = EMPTY, const ResolvedEntity *def = 0);
  SemanticTree(SCathegory::t _cathegory, const ResolvedEntity *def);
  SemanticTree(Ptr<Id2> _ddlName, NameType t, SCathegory::t _cathegory, ResolvedEntity *def = 0);
  SemanticTree(Ptr<Id>         v, NameType t, SCathegory::t _cathegory, ResolvedEntity *def = 0);
  SemanticTree(IdEntitySmart  *v, NameType t, SCathegory::t _cathegory);
  SemanticTree(Ptr<Datatype>   v, NameType t, SCathegory::t _cathegory);
  SemanticTree(Ptr<Id>         v,             SCathegory::t _cathegory);

  ~SemanticTree();



  string nametypeToString();

  uint32_t getPackedCathegory() const;
  static uint32_t getPackedCathegory(Sm::SCathegory::t cat, NameType t);


  void setRecursiveFlags(FlagsType v);

  inline bool isFromNode()        const { return   flags & FLAG_SEMANTIC_TREE_IS_FROM_NODE; }
  inline bool isIntoNode()        const { return   flags & FLAG_SEMANTIC_TREE_IS_INTO_NODE; }
  inline bool isInsertingValue()  const { return   flags & FLAG_SEMANTIC_TREE_IS_INSERTING_VALUE; }
  inline bool isInvalid()         const { return   flags & FLAG_SEMANTIC_TREE_IS_INVALID; }
  inline bool childsResolved()    const { return   flags & FLAG_SEMANTIC_TREE_IS_CHILDS_RESOLVED; }
  inline bool isAlias()           const { return   flags & FLAG_SEMANTIC_TREE_IS_ALIAS; }
  inline bool isBasForAlias()     const { return   flags & FLAG_SEMANTIC_TREE_IS_BASE_FOR_ALIAS; }
  inline bool isPlContext()       const { return   flags & FLAG_SEMANTIC_TREE_IS_PL_CONTEXT; }
  inline bool isNotPlContext()    const { return   flags & FLAG_SEMANTIC_TREE_IS_NOT_PL_CONTEXT; }
  inline bool isList()            const { return   flags & FLAG_SEMANTIC_TREE_IS_LIST; }
  inline bool childsNotResolved() const { return !(flags & FLAG_SEMANTIC_TREE_IS_CHILDS_RESOLVED); }
  inline bool unentered()         const { return !(flags & FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK); }
  inline bool isDeleted()         const { return   flags & FLAG_SEMANTIC_TREE_IS_DELETED_NODE; }


  inline bool setHasDynamicChilds()  { return flags |= FLAG_SEMANTIC_TREE_HAS_DYNAMIC_CHILDS; }
  inline bool hasDynamicChilds()  const { return flags & FLAG_SEMANTIC_TREE_HAS_DYNAMIC_CHILDS; }

  inline bool isLoopRecursion() const {
    return ((flags & FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK) == FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK);
  }

  inline bool isSqlCode()         const { return !isPlContext() || isNotPlContext(); }
  inline bool isSqlCodeInPlCode() const { return isNotPlContext() && isPlContext(); }


  inline void setDeleted()          { flags |= FLAG_SEMANTIC_TREE_IS_DELETED_NODE; }
  inline void setInvalid()          { flags |= FLAG_SEMANTIC_TREE_IS_INVALID; }
  inline void setInsertingValue()   { flags |= FLAG_SEMANTIC_TREE_IS_INSERTING_VALUE; }
  inline void setIsFromNode()       { flags |= FLAG_SEMANTIC_TREE_IS_FROM_NODE; }
  inline void setIsIntoNode()       { flags |= FLAG_SEMANTIC_TREE_IS_INTO_NODE; }
  inline void setChildsResolved()   { flags |= FLAG_SEMANTIC_TREE_IS_CHILDS_RESOLVED; }
  inline void setIsAlias()          { flags |= FLAG_SEMANTIC_TREE_IS_ALIAS; }
  inline void setIsBaseForAlias()   { flags |= FLAG_SEMANTIC_TREE_IS_BASE_FOR_ALIAS; }
  inline void setIsList()           { flags |= FLAG_SEMANTIC_TREE_IS_LIST; }

  string getLocText();

  inline void openNode() {
    FlagsType cnt = flags & FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK;
    flags &= FLAG_SEMANTIC_TREE_OPEN_COUNT_CLRMASK;
    flags |= (++cnt) & FLAG_SEMANTIC_TREE_OPEN_COUNT_MASK;
  }
  inline void clearOpenNode() {
    flags &= FLAG_SEMANTIC_TREE_OPEN_COUNT_CLRMASK;
  }
  void clearOpenState() {
    clearOpenNode();
    for (auto &c : childs)
      c->clearOpenState();
  }

  void clearDefinitions();
  ResolvedEntity *owner() const;
  Ptr<Id> entityName() const;
  inline ResolvedEntity * entityDef() const;

  Ptr<Id> refEntity() const;
  ResolvedEntity* refDefinition() const { return referenceName_ ? referenceName_->definition() : nullptr; }
  ResolvedEntity* refUnresolvedDefinition() const;
  IdEntitySmart::size_type refSize() const { return referenceName_->size(); }
  IdEntitySmart::iterator refBegin() { return referenceName_->begin(); }
  IdEntitySmart::iterator refEnd() { return referenceName_->end(); }
  bool refEmpty() const { return !referenceName_ || referenceName_->empty(); }

  const IdEntitySmart* reference() const { return referenceName_.object(); }
  IdEntitySmart* reference() { return referenceName_.object(); }

//  inline size_t ddlNameSize() const;
  inline Ptr<Id> parentInRefList(Ptr<Id> &field) const;
  void collectSystemDefinitions(SysDefCounters &counters);
  void collectSystemDefinitionsOnSysuser(SysDefCounters &counters);
  void countSystemDefinitions(SysDefCounters &counters);
  void countSystemDefinitionsOnUserContext(SysDefCounters &counters);
  int countArglistRefcursorFiels();

  bool isDeclarationOrDefinition() const;
  void deleteFromParent();

  BlockPlSql *ownerBlock();
  void changeReferences2(ResolvedEntity *from, ResolvedEntity *to);
  /// транслировать вызовы заданных функций с OUT REF CURSOR в списке аргументов.
  void translateUsageFunWithOutCursor(VEntities* funs);

  void collectDependenciesGraph(DependEntitiesMap &dst, ResolvedEntity *parentNode = 0, DepArcContext ctx = DepArcContext());
  bool collectEntitiesThatChangedVariable(ResolvedEntity *var, DependEntitiesMap &dst, ResolvedEntity *ent = 0);
  void getAllCodeBlockReferences(UniqueEntitiesMap &dst);

  void calculateLengthForVarcharResultOfFunctions();

  bool isWriteReference() const;

  void getFunctionCallsThatContainsEntityAsArgument(ResolvedEntity *, Sm::FunctionCalls &, Ptr<Id> fun = 0);
  ResolvedEntity* ddlEntity() const;
  ResolvedEntity* uddlEntity() const;
  SemanticTree *getParent() const { return parent; }
  void setParent(SemanticTree *_parent) { parent = _parent; }
  SemanticTree::Childs::iterator getPositionInParent() const { return positionInParent; }
  bool containsEntity(ResolvedEntity *entity);

  void setParentForChilds(SemanticTree::Childs::iterator posInParent);
  void setPositionInParent(SemanticTree::Childs::iterator it) { positionInParent = it; }

  void setPlContextTrue();
  void setPlContext();
  void setNotPlContext();

  bool empty() const;

  // Механизм резолвинга для различных семантических категорий (кроме функций)
  void resolveReference();
  // Механизм резолвинга для различных вызовов функций (с учетом перегрузки)
  bool resolveOneCallReference();
  bool semanticResolve() const;

  bool findDeclaration();

  SemanticTree *alias(SemanticTree *node);
  void addChild(SemanticTree *node);
  void addChildForce(SemanticTree *node);

  bool isPartitialResolved(IdEntitySmart::reverse_iterator &rit);

  void printSortedUnresolvedDeclarations();
  void printComplexReferences(string tab=string());
  void printCursorVariables();
  void printSystemFunctions();

  typedef map<string, vector<UnresolvedDataValue> > SortedOutputMap;
  typedef std::vector<vector<UnresolvedDataValue> > UnresolvedAggregateVector;
  UnresolvedState collectUnresolvedMap(SortedOutputMap &names, SortedOutputMap &reduction);

  void toNormalizedString(string &str) const;

  void printSCathegoryGraph();
  void collectSCathegoryUpperGraph();


  void resolveCurrent();
  void updateCounters(SysDefCounters &counters, ResolvedEntity *def);
  string setTranslatedName(bool q, std::string &ownerName, ResolvedEntity *userOwner);

  bool commonFindDeclarationOnLevelDown(ResolvingContext *cntx);

  void setIdentificators(NameType t, Ptr<Id> v);
  void setIdentificators(NameType t, Ptr<Id2> v);

  void check();

  void getExpressionContext(CommonDatatypeCast::CastContext &context);
  void castFunctionCallsToReducedDatatype(Sm::Function *fun);
  void collectReferencesForCastedCalls();

  void calculateCountOfEntities();

  void checkForCurrentType(Ptr<Id> &it, Datatype *checkedType, bool (Datatype::*isTypeMf)() const, SysDefCounters::TypeLocations &container);
  void collectTypeUsing(Datatype *t, bool (Datatype::*isTypeMf)() const, SysDefCounters::TypeLocations &container);
  void collectXmltypeUsing();
  void collectAnydataUsing();

  typedef std::function<bool (SemanticTree *)> EnumFunctor;
  typedef vector<SemanticTree*> EnumList;
  void enumNodes(EnumFunctor func, EnumList  &list);
  void enumNodesByCat(SCathegory::t cat, EnumList &list);

  Ptr<IdEntitySmart> extractMajorDefinitionReference();

  size_t bSize(smart::SmartptrSet &s);
  size_t bLevelNamespaceSize(smart::SmartptrSet &s);

  static void deleteChild(SemanticTree *child);
  static void removeChildFromParent(SemanticTree *child);


  bool isFringeOfStatementsLevel() const;

  static void cleanup();
  void deleteInvalidChilds();

protected:
  void addToEnititiesMap(ResolvedEntity *def, Sm::SemanticTree::UniqueEntitiesMap &dst, IdEntitySmart *ent);
};


class CollectUsingByStatements : public ExprTR {
public:
  typedef std::list<StatementInterface*> StmtsStack;
  typedef std::list<BlockPlSql*>         BlockPlSqlOwners;
  StmtsStack       *stmtsStack = 0;
  BlockPlSqlOwners *blockPlSqlOwners = 0;


  inline static StatementInterface* lastStatement(StmtsStack *stmtsStack) {
    return stmtsStack && stmtsStack->size() ? *(stmtsStack->begin()) : (StatementInterface*)0;
  }
  inline static BlockPlSql* lastOwner(BlockPlSqlOwners *blockPlSqlOwners) {
    return blockPlSqlOwners && blockPlSqlOwners->size() ? *(blockPlSqlOwners->begin()) : (BlockPlSql*)0;
  }

  static PlExpr* trFun(PlExpr* expr) { return expr; }

  CollectUsingByStatements(Cond _trCond, StmtsStack *s = 0)
    : ExprTR(trFun, _trCond), stmtsStack(s) {}

  void preaction(StatementInterface *stmt);
  void postaction(Sm::StatementInterface *);
};


#define ANODE(node)     if (node) n->addChild(node->toSTree());
#define ANODE2(n, node) if (node) n->addChild(node->toSTree());
#define ATREE(n, node)  if (node) n->addChild(node);
#define CTREE(node)     if (node) node->collectSNode(n);
#define CTREE2(n, node) if (node) node->collectSNode(n);
#define SNODE(node)     if (node) setSemanticNode(node);


class SysDefSemanticTree : public SingleSmart {
public:
  typedef map<ResolvedEntity*, Ptr<SysDefSemanticTree>, LE_ResolvedEntities> Map;
  typedef std::vector<Ptr<SysDefSemanticTree> > Childs;

  static bool compareChilds(Childs::value_type i, Childs::value_type j);
  Sm::SysDefCounters::EntityMap::value_type node;
  SysDefSemanticTree *parent;
  Childs childs;

  SysDefSemanticTree(Sm::SysDefCounters::EntityMap::value_type o = Sm::SysDefCounters::EntityMap::value_type()) : node(o), parent(0) {}
  bool formatString(Sm::Codestream &str, int tab, int level = 0);

  void sortChilds();
};


template <typename T> inline SemanticTree *ToSTreeRef(     const T        &obj, SCathegory::t t) { return obj.toSTreeRef(t); }
template <typename T> inline SemanticTree *ToSTreeRef(     const Ptr<T>   &obj, SCathegory::t t) { return obj ? obj->toSTreeRef(t) : new SemanticTree(t); }
template <>           inline SemanticTree *ToSTreeRef<Id>( const Ptr<Id>  &obj, SCathegory::t t);
template <>           inline SemanticTree *ToSTreeRef<Id2>(const Ptr<Id2> &obj, SCathegory::t t);


template <SemanticTree::NameType nameCathegory, typename Element>
struct SelectSTreeConverter;


template <typename T> struct SmartPtrDereference { typedef T value_type; };
template <typename I> struct SmartPtrDereference<Ptr<I> > { typedef I value_type; };


namespace SemanticDefinition {

namespace Cathegory { enum T { DECLARATION, DEFINITION }; }

}


template <> SemanticTree *ToSTreeRef<Id>(const Ptr<Id> &obj, SCathegory::t t);


template <> SemanticTree *ToSTreeRef<Id2>(const Ptr<Id2> &obj, SCathegory::t t);


Sm::SemanticTree *Sm::IdEntitySmart::toSTreeRef(Sm::SCathegory::t t) const {
  return new SemanticTree(const_cast<IdEntitySmart*>(this), SemanticTree::REFERENCE, t);
}


template <typename T> inline void CollectSNode(SemanticTree *node, const T &obj) {
  for (typename T::const_iterator it = obj.begin(); it != obj.end(); ++it)
    (*it)->collectSNode(node);
}


template <>
void CollectSNode(SemanticTree *node, const List<StatementInterface> &obj);


template <typename T> inline void CollectSNode(SemanticTree *node, const Ptr<T> &obj) {
  if (obj)
    for (typename T::const_iterator it = obj->begin(); it != obj->end(); ++it)
      if (*it)
        (*it)->collectSNode(node);
}


}


#endif // SEMANTIC_TREE_H
