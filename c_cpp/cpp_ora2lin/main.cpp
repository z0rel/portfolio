#include <stdio.h>
#include <unordered_set>
#include <string.h>
#include <iostream>
#include <vector>
#include <set>
#include "datetime.h"

#include "semantic_base.h"
#include "semantic_id.h"
#include "semantic_datatype.h"
#include "model_context.h"
#include "lexsubtokenizer.h"
#include "semantic_table.h"
#include "semantic_sql.h"
#include "config_converter.h"
#include "dynamic_sql_op.h"
#include "semantic_blockplsql.h"
#include "lindumper.h"
#include "sentence_transformer.h"


std::set<Sm::Datatype*> Sm::DatatypeLeakManager::managedTypes;

const cl::filelocation Sm::ResolvedEntity::emptyFilelocation = cl::emptyFLocation();
Sm::SemanticTree::DeletedNodes Sm::SemanticTree::deletedNodes;

GlobalEntitiesCnt Sm::ResolvedEntity::globalEntitiesCounter = 0;

Ptr<Sm::RowIdExpr> Sm::RowIdExpr::self = new Sm::RowIdExpr();
Ptr<Sm::RowNumExpr> Sm::RowNumExpr::self = new Sm::RowNumExpr();

Sm::ResolvedEntitySNode::ToSTreeAction  Sm::ResolvedEntitySNode::toSTreeAction =
    &Sm::ResolvedEntitySNode::toSTreeGenerateStage;

std::stack<Sm::ResolvedEntitySNode::ToSTreeAction>
  Sm::ResolvedEntitySNode::toSTreeActionStack;

const string Sm::Id::emptyText;
Sm::CachedIdentificators::FastIdHashMap Sm::Id::cachedIds;
std::set<int> LinterWriter::errcodesToSkip;
unordered_set<string> OutputCodeFile::usedFiles;
OutputCodeFileChunck::Chuncks OutputCodeFileChunck::chuncks;

Sm::Codestream *Sm::Codestream::mainStream = 0;
std::set<Sm::Id*> globalIds;

Sm::LexSubtokenizer lexerSubtokenizer;
Sm::StatisticNode::ErrorStatistic Sm::StatisticNode::errorStatVector;

Ptr<Sm::Function> Sm::DynamicFuncallTranslator::funnameTranslator;
Ptr<Sm::Function> Sm::DynamicFuncallTranslator::funcallSignatureTranslator;

namespace Sm {
  namespace s {
    Ptr<StaticSpacers> staticSpacers = new StaticSpacers();
  }
}


class StartInit {
public:
  int i = 0;
  StartInit() {}
  ~StartInit() {
    cout << "";
  }
};


size_t Sm::Id::__getGlobalId() {
  static size_t cnt = 0;
  globalIds.insert(this);
  ++cnt;
  if (cnt == 301616)
    cout << "";
  return cnt;
}

//Sm::Id::~Id() {
//  globalIds.erase(this);
//}

size_t Sm::RefExpr::__getId() {
  static size_t cnt = 0;
  ++cnt;
  if (cnt == 23063)
    cout << "";
  return cnt;
}

StartInit startInit;

bool __STreeDestructionStarted__ = false;
LexStringBuffer lexer_id_text;
SyntaxerContext *globalContext;

#if SMARTPTR_DEBUG
MaximalSizes maximalSizes;
unordered_map<const type_info*, int> Register_v;
#endif


const int Sm::GlobalDatatype::HasImplicitConversion::datatypeCathegories[FUNDAMENTAL_TYPES_COUNT] = {
  /* [EMPTY             ] */ -1,
  /* [_BFILE_           ] */ (int)(__BFILE),
  /* [_BLOB_            ] */ (int)(__BLOB),
  /* [_CLOB_            ] */ (int)(__CLOB),
  /* [_NCLOB_           ] */ (int)(__CLOB),
  /* [_DATE_            ] */ (int)(__DATE),
  /* [_TIMESTAMP_       ] */ (int)(__DATE),
  /* [_TIMESTAMP_LTZ_   ] */ (int)(__DATE),
  /* [_TIMESTAMP_SIMPLE_] */ (int)(__DATE),
  /* [_TIMESTAMP_TZ_    ] */ (int)(__DATE),
  /* [_INTERVAL_        ] */ (int)(__INTERVAL),
  /* [_INTERVAL_DTS_    ] */ (int)(__INTERVAL),
  /* [_INTERVAL_YTM_    ] */ (int)(__INTERVAL),
  /* [_BOOLEAN_         ] */ (int)(__BOOL),
  /* [_SMALLINT_        ] */ (int)(__SMALLINT),
  /* [_INT_             ] */ (int)(__INT),
  /* [_BIGINT_          ] */ (int)(__BIGINT),
  /* [_PLS_INTEGER_     ] */ (int)(__INT),
  /* [_REAL_            ] */ (int)(__REAL),
  /* [_DOUBLE_          ] */ (int)(__DOUBLE),
  /* [_DECIMAL_         ] */ (int)(__NUMBER),
  /* [_NUMBER_          ] */ (int)(__NUMBER),
  /* [_NUMERIC_         ] */ (int)(__NUMBER),
  /* [_LONG_RAW_        ] */ (int)(__RAW),
  /* [_RAW_             ] */ (int)(__RAW),
  /* [_ROWID_           ] */ (int)(__ROWID),
  /* [_UROWID_          ] */ (int)(__ROWID),
  /* [_CHAR_            ] */ (int)(__CHAR),
  /* [_STRING_          ] */ (int)(__VARCHAR2),
  /* [_VARCHAR_         ] */ (int)(__VARCHAR2),
  /* [_LONG_            ] */ (int)(__LONG),
  /* [_NCHAR_           ] */ (int)(__NCHAR),
  /* [_NVARCHAR_        ] */ (int)(__NVARCHAR2),
  /* [_VARRAY_          ] */ -1,
  /* [_NESTED_TABLE_    ] */ -1,
  /* [_OBJECT_          ] */ -1,
  /* [_ANYDATA_         ] */ -1,
  /* [_XMLTYPE_         ] */ -1
};

const int Sm::GlobalDatatype::HasImplicitConversion::greatherDatatypeCathegories[FUNDAMENTAL_TYPES_COUNT] = {
  /* [EMPTY             ] */ -1,
  /* [_BFILE_           ] */ (int)(__BFILE),
  /* [_BLOB_            ] */ (int)(__BLOB),
  /* [_CLOB_            ] */ (int)(__CLOB),
  /* [_NCLOB_           ] */ (int)(__CLOB),
  /* [_DATE_            ] */ (int)(__DATE),
  /* [_TIMESTAMP_       ] */ (int)(__DATE),
  /* [_TIMESTAMP_LTZ_   ] */ (int)(__DATE),
  /* [_TIMESTAMP_SIMPLE_] */ (int)(__DATE),
  /* [_TIMESTAMP_TZ_    ] */ (int)(__DATE),
  /* [_INTERVAL_        ] */ (int)(__INTERVAL),
  /* [_INTERVAL_DTS_    ] */ (int)(__INTERVAL),
  /* [_INTERVAL_YTM_    ] */ (int)(__INTERVAL),
  /* [_BOOLEAN_         ] */ (int)(__BOOL),
  /* [_SMALLINT_        ] */ (int)(__SMALLINT),
  /* [_INT_             ] */ (int)(__INT),
  /* [_BIGINT_          ] */ (int)(__BIGINT),
  /* [_PLS_INTEGER_     ] */ (int)(__INT),
  /* [_REAL_            ] */ (int)(__REAL),
  /* [_DOUBLE_          ] */ (int)(__DOUBLE),
  /* [_DECIMAL_         ] */ (int)(__NUMBER),
  /* [_NUMBER_          ] */ (int)(__NUMBER),
  /* [_NUMERIC_         ] */ (int)(__NUMBER),
  /* [_LONG_RAW_        ] */ (int)(__RAW),
  /* [_RAW_             ] */ (int)(__RAW),
  /* [_ROWID_           ] */ (int)(__ROWID),
  /* [_UROWID_          ] */ (int)(__ROWID),
  /* [_CHAR_            ] */ (int)(__CHAR),
  /* [_STRING_          ] */ (int)(__VARCHAR2),
  /* [_VARCHAR_         ] */ (int)(__VARCHAR2),
  /* [_LONG_            ] */ (int)(__LONG),
  /* [_NCHAR_           ] */ (int)(__NCHAR),
  /* [_NVARCHAR_        ] */ (int)(__NVARCHAR2),
  /* [_VARRAY_          ] */ -1,
  /* [_NESTED_TABLE_    ] */ -1,
  /* [_OBJECT_          ] */ -1,
  /* [_ANYDATA_         ] */ -1,
  /* [_XMLTYPE_         ] */ -1
};

#define TBL_SIZE (int)(Sm::GlobalDatatype::HasImplicitConversion::__LAST_MAPPED_TYPE)

#define E  CAST_EQUALLY
#define I  CAST_IMPLICIT
#define S  CAST_EXPLICIT_SELECT
#define U  CAST_EXPLICIT_UNION
#define SU CAST_EXPLICIT_SELECT_UNION
#define RS CAST_EXPLICIT_SELECT_AND_RETURN
#define RSU CAST_EXPLICIT_SELECT_RETURN_UNION
#define L  CAST_INTERNAL_TYPES_CHR_NEED_CMP_BY_LEN
#define N  CAST_INTERNAL_TYPES_NUM_NEED_CMP_BY_LEN

// отношения старшинства для ненулевых отношений - сравнивать по адресам строчек и столбцов - чем меньше - тем старше

const unsigned int Sm::GlobalDatatype::HasImplicitConversion::greatherTable[TBL_SIZE][TBL_SIZE] =
                    // |  N  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     /*B ->*/       // |  V  |V  |   |   |   |I  |   |   |   |   |   |   |   |   |   |   |   |   |S  |   |  // A > B
                    // |  A  |A  |   |   |   |N  |   |   |   |   |   |   |   |   |   |   |   |   |M  |   |
                    // |  R  |R  |   |   |   |T  |N  |D  |   |   |   |   |   |   |   |   |B  |   |A  |   |
                    // |  C  |C  |N  |   |   |E  |U  |O  |   |   |   |R  |   |B  |N  |   |I  |   |L  |   |
                    // |  H  |H  |C  |C  |D  |R  |M  |U  |R  |L  |   |O  |B  |F  |C  |C  |G  |   |L  |B  |
                    // |  A  |A  |H  |H  |A  |V  |B  |B  |E  |O  |R  |W  |L  |I  |L  |L  |I  |I  |I  |O  |
                    // |  R  |R  |A  |A  |T  |A  |E  |L  |A  |N  |A  |I  |O  |L  |O  |O  |N  |N  |N  |O  |
{                   // |  2  |2  |R  |R  |E  |L  |R  |E  |L  |G  |W  |D  |B  |E  |B  |B  |T  |T  |T  |L  |
  /* A    */        // |  0  |1  |2  |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |
  /* NVARCHAR2 =  0 */ {  L,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* VARCHAR2  =  1 */ {  0,  L,  0,  L,  0,  0,  0,  0,  0,  I,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* NCHAR     =  2 */ {  L,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* CHAR      =  3 */ {  0,  L,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* DATE      =  4 */ {  0,  0,  0,  0,  E,  I,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* INTERVAL  =  5 */ {  0,  0,  0,  0,  I,  E,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* NUMBER    =  6 */ {  0,  0,  0,  0,  0,  0,  N, SU, SU,  0,  0,  0,  0,  0,  0,  0, RS, RS, RS,  0  },
  /* DOUBLE    =  7 */ {  0,  0,  0,  0,  0,  0,RSU,  E,  S,  0,  0,  0,  0,  0,  0,  0,RSU,RSU,RSU,  0  }, // S - need to checks
  /* REAL      =  8 */ {  0,  0,  0,  0,  0,  0,RSU, SU,  E,  0,  0,  0,  0,  0,  0,  0,RSU,RSU,RSU,  0  }, // S - need to checks
  /* LONG      =  9 */ {  0,  I,  0,  0,  0,  0,  0,  0,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* RAW       = 10 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* ROWID     = 11 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  L,  0,  0,  0,  0,  0,  0,  0,  0  },
  /* BLOB      = 12 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  E,  I,  0,  0,  0,  0,  0,  0  },
  /* BFILE     = 13 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  I,  E,  E,  0,  0,  0,  0,  0  },
  /* NCLOB     = 14 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  E,  0,  0,  0,  0,  0  },
  /* CLOB      = 15 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  S,  0,  0,  0,  0  },
  /* BIGINT    = 16 */ {  0,  0,  0,  0,  0,  0, RS, SU, SU,  0,  0,  0,  0,  0,  0,  0,  E,  U,  U,  0  },
  /* INT       = 17 */ {  0,  0,  0,  0,  0,  0, RS,RSU,RSU,  0,  0,  0,  0,  0,  0,  0,RSU,  E,  U,  0  },
  /* SMALLINT  = 18 */ {  0,  0,  0,  0,  0,  0, RS,RSU,RSU,  0,  0,  0,  0,  0,  0,  0,RSU,RSU,  E,  0  },
  /* BOOL      = 19 */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  E  }
};

//  A всегда должно соответствовать LHS, B всегда должно соответствовать RHS
//   0 - отношения между типами неопределены, необходимо явное преобразование
// I   - есть неявное преобразование из A в B
// E   - A = B типы совпадают
// L   - нужно сравнение по длинам для символьных типов
// N   - нужно сравнение по длинам для числовых типов
// S   - явное преобразование нужно для SELECT B INTO A (B преобразуется к A)
//       (но если длина B больше длины A - то тоже будет сгенерировано исключение)
//       ! здесь ! A = LHS = INTO VAR, B = RHS = FIELD
//       (INTO VAR)->getCastCathegory(FIELD): - вернет S,
//       а если LEN(FIELD = B) > len(INTO VAR = A) - будут установлены
//          CAST_BY_LENGTH_LHS_LT_RHS
//          CAST_CHAR_BY_LENGTH_LT
//       => CAST_IN_SELECT_LHS_FIELD_TO_INTO_RHS_VAR
//
//           LHS_GT_RHS, CAST_CHAR_BY_LENGTH_GT - CAST_IN_SELECT_LHS_FIELD_TO_INTO_RHS_VAR
//
// проверка: (INTO VAR = BIGINT, FIELD = INT) = I
//
// R   - явное преобразование нужно для Fun RESULT A; CODE return B; END;
//        RETTYPE = LHS = A, RETEXPRTYPE = RHS = B
//        RETTYPE->getCastCathegory(RETEXPRTYPE)
// проверка: (result NUMERIC, return    INT) = I
//           (result NUMERIC, return BIGINT) = R


#undef E
#undef I
#undef S
#undef RS
#undef L
#undef N

/* Неявные преобразования в линтер
 * varchar, char
 *   len1 < len2
 *     - в вызове, в присваивании, в return
 *   len1 > len2 (обрезается
 *     - в вызове, в присваивании, в return
 *
 * явное преобразование нужно (только по длине) в select into - когда выбираемая длина больше длины переменной в секции into (len(lhs) > len(rhs))
 * char и varchar при допустимости длин и в select into преобразуются друг к другу неявно
 *
 *
 * явное преобразование bool к любому числовому типу необходимо:
 *    - в select into, причем: cast(cast(boolval as INTEGER) as NUMBER)
 *    - в вызове     , причем: SQL: call(cast(boolval as INTEGER)); code: EIF[boolval] 1 ELSE 0
 *    - в присваивании, в return, etc: code: EIF[boolval] 1 ELSE 0
 *
 * явное преобразование любого числового типа к bool необходимо везде
 *    - в коде: eif[val = 0] FALSE ELSE TRUE
 *    - в SQL: - для интегральных   типов CAST(VAL as BOOLEAN)
 *             - для неинтегральных типов CASE(VAL = 0 WHEN TRUE THEN FALSE WHEN FALSE THEN TRUE)
 *
 * Где lhs и RHS в SELECT:
 *
 * VAR i INT;
 * EXECUTE "SELECT CAST(1 AS BIGINT)" into i; -- исключение
 * Но:
 * VAR i BIGINT;
 * EXECUTE "SELECT CAST(1 AS INT)" into i; -- Ok: i == 1;
 *
 *
 *
 */

#define _0 Sm::IsSubtypeValuesEnum::EXPLICIT
#define I0 Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_LOW
#define I1 Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_1
#define I2 Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_2
#define I3 Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_3
#define IH Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_HIGH
#define IF Sm::IsSubtypeValuesEnum::IMPLICIT_CAST_BY_FIELDS


#define LC Sm::IsSubtypeValuesEnum::NEED_CHAR_COMPARSION_BY_LENGTH
#define LN Sm::IsSubtypeValuesEnum::NEED_NUMBER_COMPARSION_BY_LENGTH

#define EQ Sm::IsSubtypeValuesEnum::EXACTLY_EQUALLY




/// Таблица допустимых неявных преобразований для Oracle SQL
//smart::EventCounterMap eventsCounterMap;
const Sm::IsSubtypeValuesEnum::IsSubtypeCathegory Sm::GlobalDatatype::HasImplicitConversion::implicitConversionTable[TBL_SIZE][TBL_SIZE] =
                  // | N  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
                  // | V  |V  |   |   |   |I  |   |   |   |   |   |   |   |   |   |   |   |   |S  |   |
                  // | A  |A  |   |   |   |N  |   |   |   |   |   |   |   |   |   |   |   |   |M  |   |
                  // | R  |R  |   |   |   |T  |N  |D  |   |   |   |   |   |   |   |   |B  |   |A  |   |
                  // | C  |C  |N  |   |   |E  |U  |O  |F  |   |   |R  |   |B  |N  |   |I  |   |L  |   |
                  // | H  |H  |C  |C  |D  |R  |M  |U  |L  |L  |   |O  |B  |F  |C  |C  |G  |   |L  |B  |
                  // | A  |A  |H  |H  |A  |V  |B  |B  |O  |O  |R  |W  |L  |I  |L  |L  |I  |I  |I  |O  |
                  // | R  |R  |A  |A  |T  |A  |E  |L  |A  |N  |A  |I  |O  |L  |O  |O  |N  |N  |N  |O  |
{                 // | 2  |2  |R  |R  |E  |L  |R  |E  |T  |G  |W  |D  |B  |E  |B  |B  |T  |T  |T  |L  |
                  // | 0  |1  |2  |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |
  /*NVARCHAR2 =  0*/ { LC, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, I2, I2, _0, _0, _0, _0, _0, _0 },
  /*VARCHAR2  =  1*/ { _0, LC, _0, I3, I2, _0, I2, _0, _0, _0, _0, _0, I2, I2, _0, _0, I2, I2, I2, _0 },
  /*NCHAR     =  2*/ { _0, _0, LC, _0, _0, _0, _0, _0, _0, _0, _0, _0, I2, I2, _0, _0, _0, _0, _0, _0 },
  /*CHAR      =  3*/ { _0, I3, _0, LC, _0, _0, _0, _0, _0, _0, _0, I2, _0, _0, _0, _0, _0, _0, _0, _0 },
  /*DATE      =  4*/ { _0, I2, _0, _0, EQ, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, I1, _0, _0, _0, _0 },
  /*INTERVAL  =  5*/ { _0, _0, _0, _0, I1, EQ, I3, I2, I2, _0, I1, I1, I1, I1, I1, I1, _0, _0, _0, _0 },
  /*NUMBER    =  6*/ { _0, I2, _0, _0, I1, I3, LN, I2, _0, I1, I1, I1, I1, I1, I1, I1, I3, I3, I3, I3 }, // NUMBER->DOUBLE => поменял сам, т.к. oracle поддерживает это в запросах
  /*DOUBLE    =  7*/ { _0, _0, _0, _0, I1, I2, _0, EQ, _0, I1, I1, I1, I1, I1, I1, I1, _0, _0, _0, _0 },
  /*FLOAT     =  8*/ { _0, _0, _0, _0, I1, I2, _0, _0, EQ, I1, I1, I1, I1, I1, I1, I1, _0, _0, _0, _0 },
  /*LONG      =  9*/ { _0, _0, _0, _0, I1, _0, I1, I1, I1, EQ, _0, I2, I2, I2, _0, _0, _0, _0, _0, _0 },
  /*RAW       = 10*/ { _0, _0, _0, _0, I1, I1, I1, I1, I1, _0, EQ, I2, _0, _0, I2, I2, _0, _0, _0, _0 },
  /*ROWID     = 11*/ { _0, _0, _0, I2, I1, I1, I1, I1, I1, I2, I2, EQ, I2, I2, I2, I2, _0, _0, _0, _0 },
  /*BLOB      = 12*/ { I2, I2, I2, I2, I1, I1, I1, I1, I1, I2, _0, I2, EQ, _0, I2, I2, _0, _0, _0, _0 },
  /*BFILE     = 13*/ { I2, I2, I2, I2, I1, I1, I1, I1, I1, I2, _0, I2, _0, EQ, I2, I2, _0, _0, _0, _0 },
  /*NCLOB     = 14*/ { _0, _0, _0, _0, I1, I1, I1, I1, I1, _0, I2, I2, I2, I2, EQ, _0, _0, _0, _0, _0 },
  /*CLOB      = 15*/ { _0, _0, _0, _0, I1, I1, I1, I1, I1, _0, I2, I2, I2, I2, _0, EQ, _0, _0, _0, _0 },
  /*BIGINT    = 16*/ { _0, I2, _0, _0, I1, _0, IH, _0, _0, _0, _0, _0, _0, _0, _0, _0, EQ, I2, I1, I0 },
  /*INT       = 17*/ { _0, I1, _0, _0, I1, _0, I2, I2, I2, _0, _0, _0, _0, _0, _0, _0, IH, EQ, I3, I1 },
  /*SMALLINT  = 18*/ { _0, I2, _0, _0, I1, _0, I3, _0, _0, _0, _0, _0, _0, _0, _0, _0, IH, IH, EQ, IH },
  /*BOOL      = 19*/ { _0, _0, _0, _0, _0, _0, IH, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, IH, IH, EQ },
};

/// Таблица допустимых неявных преобразований для Oracle PlSql
const Sm::IsSubtypeValuesEnum::IsSubtypeCathegory Sm::GlobalDatatype::HasImplicitConversion::implicitConversionPlTable[TBL_SIZE][TBL_SIZE] =
                  // |    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
                  // |    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
                  // | N  |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
                  // | V  |V  |   |   |   |I  |   |   |   |   |   |   |   |   |   |   |   |   |S  |   |
                  // | A  |A  |   |   |   |N  |   |   |   |   |   |   |   |   |   |   |   |   |M  |   |
                  // | R  |R  |   |   |   |T  |N  |D  |   |   |   |   |   |   |   |   |B  |   |A  |   |
                  // | C  |C  |N  |   |   |E  |U  |O  |F  |   |   |R  |   |B  |N  |   |I  |   |L  |   |
                  // | H  |H  |C  |C  |D  |R  |M  |U  |L  |L  |   |O  |B  |F  |C  |C  |G  |   |L  |B  |
                  // | A  |A  |H  |H  |A  |V  |B  |B  |O  |O  |R  |W  |L  |I  |L  |L  |I  |I  |I  |O  |
                  // | R  |R  |A  |A  |T  |A  |E  |L  |A  |N  |A  |I  |O  |L  |O  |O  |N  |N  |N  |O  |
{                 // | 2  |2  |R  |R  |E  |L  |R  |E  |T  |G  |W  |D  |B  |E  |B  |B  |T  |T  |T  |L  |
                  // | 0  |1  |2  |3  |4  |5  |6  |7  |8  |9  |10 |11 |12 |13 |14 |15 |16 |17 |18 |19 |
  /*NVARCHAR2 =  0*/ { LC, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, _0, I3, I3, _0, _0, _0, _0, _0, _0 },
  /*VARCHAR2  =  1*/ { _0, LC, _0, I3, I1, _0, I1, I1, I1, I2, I2, I2, _0, _0, _0, I2, I1, I1, I1, I1 },
  /*NCHAR     =  2*/ { _0, _0, LC, _0, _0, _0, _0, _0, _0, _0, _0, _0, I3, I3, _0, _0, _0, _0, _0, _0 },
  /*CHAR      =  3*/ { _0, I3, _0, LC, I1, _0, I1, I1, I1, I2, I2, I2, _0, _0, _0, I2, I1, I1, I1, I1 },
  /*DATE      =  4*/ { _0, I1, _0, I1, EQ, I2, _0, _0, _0, I2, _0, _0, _0, _0, I1, _0, _0, _0, _0, _0 },
  /*INTERVAL  =  5*/ { _0, _0, _0, _0, I2, EQ, IH, I3, I3, _0, I1, I1, I1, I1, I1, I1, _0, _0, _0, _0 },
  /*NUMBER    =  6*/ { _0, I1, _0, I1, _0, I3, LN, I3, I3, I2, _0, _0, _0, _0, I1, _0, I3, I3, I3, I2 },
  /*DOUBLE    =  7*/ { _0, I1, _0, I1, _0, I3, IH, EQ, IH, I1, _0, _0, _0, _0, I1, _0, I3, I3, I3, I3 },
  /*FLOAT     =  8*/ { _0, I1, _0, I1, _0, I3, IH, IH, EQ, I1, _0, _0, _0, _0, I1, _0, I3, I3, I3, I3 },
  /*LONG      =  9*/ { _0, I2, _0, I2, _0, _0, _0, _0, _0, EQ, I3, _0, _0, _0, _0, _0, _0, _0, _0, _0 },
  /*RAW       = 10*/ { _0, I2, _0, I2, _0, I1, _0, _0, _0, I3, EQ, _0, I3, I3, I2, _0, _0, _0, _0, _0 },
  /*ROWID     = 11*/ { _0, I2, _0, I2, _0, I1, _0, _0, _0, _0, _0, EQ, _0, _0, I2, _0, _0, _0, _0, _0 },
  /*BLOB      = 12*/ { I3, _0, I3, _0, _0, I1, _0, _0, _0, _0, I3, _0, EQ, _0, I2, _0, _0, _0, _0, _0 },
  /*BFILE     = 13*/ { I3, _0, I3, _0, _0, I1, _0, _0, _0, _0, I3, _0, _0, EQ, I2, _0, _0, _0, _0, _0 },
  /*NCLOB     = 14*/ { _0, _0, _0, _0, I1, I1, I1, I1, I1, _0, I2, I2, I2, I2, EQ, _0, _0, _0, _0, _0 },
  /*CLOB      = 15*/ { _0, I2, _0, I2, _0, I1, _0, _0, _0, _0, _0, _0, _0, _0, _0, EQ, _0, _0, _0, _0 },
  /*BIGINT    = 16*/ { _0, I1, _0, I1, _0, _0, IH, I3, I3, I1, _0, _0, _0, _0, _0, _0, EQ, I2, I1, I0 },
  /*INT       = 17*/ { _0, I1, _0, I1, _0, _0, I3, I2, I2, I1, _0, _0, _0, _0, _0, _0, IH, EQ, I2, I1 },
  /*SMALLINT  = 18*/ { _0, I1, _0, I1, _0, _0, I3, I2, I2, I1, _0, _0, _0, _0, _0, _0, IH, IH, EQ, I2 },
  /*BOOL      = 19*/ { _0, I1, _0, I1, _0, _0, I3, I2, I2, I1, _0, _0, _0, _0, _0, _0, _0, I2, I3, EQ },
};


#undef _0
#undef I2
#undef I3
#undef IH
#undef IF
#undef LC
#undef LN
#undef EQ

#undef TBL_SIZE

std::string    *globalCurrentFile = 0;
Sm::Datatype::GlobalDatatypes Sm::Datatype::globalDatatypes;
Sm::Datatype::Varchar2LengthDatatypes Sm::Datatype::varchar2LengthDatatypes;
Sm::Datatype::Varchar2LengthDatatypes Sm::Datatype::charLengthDatatypes;
SyntaxerContext syntaxerContext;


itostr_helper hlp_init;
unsigned itostr_helper::out[10000];


void globalCleanup() {
  syntaxerContext.stage = SyntaxerContext::GLOBAL_CLEANUP;
  syntaxerContext.model = 0;
  syntaxerContext.containerOfModel = 0;

  Sm::StatisticNode::cleanup();
  Sm::SemanticTree::cleanup();
  Sm::s::staticSpacers = 0;
  Sm::Id::cachedIds.cleanup();
  Sm::Datatype::globalDatatypes.clear();
  Sm::Datatype::varchar2LengthDatatypes.clear();
  Sm::Datatype::charLengthDatatypes.clear();
}

#include "codestream_indenter.h"

//extern int yydebug;
int main(int argc, char **argv) {
//  yydebug = 1;
  //  Sm::indenterTest();
  //  return 0;

  Sm::ResolvedEntitySNode::toSTreeActionStack.push(&Sm::ResolvedEntitySNode::toSTreeGenerateStage);

  syntaxerContext.model->configure(argc, argv);
  if (syntaxerContext.dumpDB) {
    Dump::LinterDumper ldumper(syntaxerContext.linterUsername, syntaxerContext.linterPassword, syntaxerContext.linterNodename);
    if (syntaxerContext.dumpDateOnly)
      ldumper.setCfgFlag(Dump::CFDateOnly);
    for (Sm::IdEntitySmart &entity : syntaxerContext.dumpEntityList) {
      if (entity.size() != 2)
        continue;
      ldumper.addToFilter(entity[1]->toNormalizedString(), entity[0]->toNormalizedString());
    }

    return ldumper.start(syntaxerContext.dumpDbFile) ? 0 : -1;
  }
  if (!syntaxerContext.transformSentence.empty()) {
    SentenceTransformer sentenceTransformer(&syntaxerContext);
    sentenceTransformer.transform();
    return 0;
  }

  syntaxerContext.model->syntaxAnalyze();
  if (syntaxerContext.checkLexerLocations)
    return 0;
  if (syntaxerContext.hasSyntaxerContextErrors) {
    cout << "ERROR: Errors on syntax analyze stage" << endl;
    return -1;
  }
  syntaxerContext.model->contextAnalyze();

  syntaxerContext.model->formalTranslations();
  syntaxerContext.initializeAdditionalConfigData();
  syntaxerContext.model->resolveCreatedObjects();
  syntaxerContext.model->codegeneration();

  // Пропускать этап сравнения, если:
  // - Если выключен флаг "diffgen_enable" в конфиге
  // - Если выполняется кодогенерация по call-интерфейсу (в этом случае сравниваемый файл не создается)
  // - Если в конфиге не задана команда скрипта сортировки
  // - Если в конфиге задано требование пропускать кодогенерацию (в этом случае сортировать нечего)
  if (  !syntaxerContext.diffgenEnable
      || syntaxerContext.model->modelActions.directCreateInDB
      || syntaxerContext.codegenSortCmd.empty()
      || syntaxerContext.skipCodegeneration)
    return 0;
  globalCleanup();

  stringstream sortCmd;
  sortCmd << syntaxerContext.codegenSortCmd << " " << syntaxerContext.confFileName;
  string cmd = sortCmd.str();
  cout << "Execute: " << cmd << endl;
  int retval = system(cmd.c_str());
  cout << "return code = " << retval << ", error code = " << errno << endl;
  cout << endl << "Okay!" << endl;
  return 0;
}
