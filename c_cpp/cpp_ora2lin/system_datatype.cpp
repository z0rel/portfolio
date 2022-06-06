#include "model_head.h"
#include "smartptr.h"
#include "system_datatype.h"
#include "semantic_function.h"
#include "model_context.h"
#include "ora2text.h"
#include "ora2call.h"

extern SyntaxerContext syntaxerContext;

using namespace std;

const Sm::GlobalDatatype::DmpType::MaskArray Sm::GlobalDatatype::DmpType::maskArray;

#define ADD_MASK( typeName )  v[ft ## typeName ] = Sm::GlobalDatatype::DmpType::mask ## typeName

Sm::GlobalDatatype::DmpType::MaskArray::MaskArray() {
  memset(v, 0, 0x200 * sizeof(unsigned int) );
  ADD_MASK(Unknown     );
  ADD_MASK(Raw         );
  ADD_MASK(Number      );
  ADD_MASK(Long        );
  ADD_MASK(Float       );
  ADD_MASK(Char        );
  ADD_MASK(NChar       );
  ADD_MASK(NVarchar    );
  ADD_MASK(SYS         );
  ADD_MASK(Varchar     );
  ADD_MASK(Date        );
  ADD_MASK(Timestamp   );
  ADD_MASK(TimestampTZ );
  ADD_MASK(TimestampLTZ);
  ADD_MASK(YearToMonth );
  ADD_MASK(DayToSec    );
  ADD_MASK(Clob        );
  ADD_MASK(Blob        );
  ADD_MASK(Bfile       );
  ADD_MASK(NClob       );
  ADD_MASK(Object      );
  ADD_MASK(NestedTable );
  ADD_MASK(Varray      );
  ADD_MASK(Rowid       );
  ADD_MASK(Urowid      );
  ADD_MASK(XML         );
}

typedef Sm::GlobalDatatype::FundamentalDatatype   Fdt;
typedef Sm::GlobalDatatype::StringDatatype        Sdt;
typedef Sm::GlobalDatatype::NumberDatatype        Ndt;
typedef Sm::GlobalDatatype::SysTypeInfo           SysDtConv;
typedef Sm::GlobalDatatype::SysTypeInfo::FldType  DmpPrecision;
typedef Sm::GlobalDatatype::DmpType::DmpCathegory DmpCathegory;

void setPrecisionByte  (int &precision, DmpPrecision &t, FILE *f) { fread_throw<int, 4>::read(f); precision = t.field1; }
void setPrecisionObject(int &precision, DmpPrecision &t, FILE * ) { precision = t.field1; }
void setPrecisionRaw   (int &precision, DmpPrecision &t, FILE * ) { precision = t.field1; }
void setPrecisionInterv(int &precision, DmpPrecision &t, FILE * ) { precision = t.field1; }
void setPrecisionClob  (int &         , DmpPrecision & , FILE *f) { /*precision = t.field1;*/ fread_throw<int, 4>::read(f); }
void setPrecisionBlob  (int &         , DmpPrecision & , FILE * ) { /*precision = t.field1;*/ /*fread<int, 4>(f);*/ }
void setPrecisionRowid (int &precision, DmpPrecision &t, FILE * ) { precision = t.field1; }

void toNumberLength(string &result, int precision, int length, DmpCathegory = Sm::GlobalDatatype::DmpType::ftUnknown) {
  //       precision < 0   => 20
  // 0  <= precision <  10 => 10
  // 10 <= precision <= 20 => precision
  //       precision >  20 => 20
  int tmpPrec = 20;
  /*if ( precision < 10 ) {
    if ( precision >= 0 )
      tmpPrec = 10;
  }
  else*/ if ( precision < 20 /* заказчик считают, что это баг -> && precision >= 10 */ )
    tmpPrec = precision;

  //                             частота появления
  //      scale < 0   => 10    -> 0
  // 0 <= scale <= 10 => scale -> 3
  //      scale >  10 => 10    -> 1

  int tmpSacle = length;
  if ( length > 10 || length < 0 )
    tmpSacle = 10;

  result += "(";
  toString(result, tmpPrec);
  result += ",";
  toString(result, tmpSacle);
  result += ")";
}

void toFloatLength(string &result, int precision, int = 0, DmpCathegory = Sm::GlobalDatatype::DmpType::ftUnknown) {
  if( precision > 0 ) {
    result += "(";
    toString( result, precision > 53 ? 53 : precision );
    result += ")";
  }
}

void toByteLength(string &result, int precision, int = 0, DmpCathegory t = Sm::GlobalDatatype::DmpType::ftUnknown) {
  int tmpPrec = precision;
  if      ( t == Sm::GlobalDatatype::DmpType::ftRowid  && !tmpPrec )
    tmpPrec = 10;
  else if ( t == Sm::GlobalDatatype::DmpType::ftUrowid && !tmpPrec )
    tmpPrec = 4000;

  result += "(";
  toString( result, tmpPrec );
  result += ")";
}

namespace Sm {

namespace GlobalDatatype {

GlobalDatatype::GlobalDatatype() {

  /*                          fieldTypeID        overlId typeCathegory     parentCathegory setType                getBrLenPrec   getValueText   ora2CallConverter   oracleName      linterProcName linterSqlName */
  add(new Ndt(new SysDtConv(DmpType::ftNumber      , 'N', _NUMBER_          , EMPTY        , 0                 , toNumberLength, numberToText   , numToNumLinT   , "NUMBER"          , "NUMERIC" , "NUMBER"   )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'n', _NUMERIC_         , _NUMBER_     , 0                 , toNumberLength, numberToText   , numToNumLinT   , "NUMERIC"         , "NUMERIC" , "NUMERIC"  )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'D', _DECIMAL_         , _NUMERIC_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "DEC"             , "REAL"    , "DEC"      )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'D', _DECIMAL_         , _NUMERIC_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "DECIMAL"         , "REAL"    , "DECIMAL"  )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'd', _DOUBLE_          , _DECIMAL_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "DOUBLE"          , "DOUBLE"  , "DOUBLE"   )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'd', _DOUBLE_          , _DECIMAL_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "BINARY_DOUBLE"   , "DOUBLE"  , "DOUBLE"   )));
  add(new Fdt(new SysDtConv(DmpType::ftNumber      , 'd', _DOUBLE_          , _DECIMAL_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "DOUBLE PRECISION", "DOUBLE"  , "DOUBLE"   )));
  add(new Fdt(new SysDtConv(DmpType::ftFloat       , 'd', _DOUBLE_          , _DECIMAL_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "FLOAT"           , "DOUBLE"  , "FLOAT"    )));
  add(new Fdt(new SysDtConv(DmpType::ftFloat       , 'd', _DOUBLE_          , _DECIMAL_    , 0                 , toFloatLength , numberToText   , numToRealLinT  , "BINARY_FLOAT"    , "DOUBLE"  , "FLOAT"    )));
  add(new Fdt(new SysDtConv(DmpType::ftFloat       , 'f', _REAL_            , _DOUBLE_     , 0                 , toFloatLength , numberToText   , numToRealLinT  , "REAL"            , "REAL"    , "REAL"     )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "INT"             , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "INTEGER"         , "INT"     , "INTEGER"  )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "NATURAL"         , "NATURAL" , "NATURAL"  )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _SMALLINT_        , _INT_        , 0                 , 0             , numberToText   , numToIntLinT   , "SMALLINT"        , "SMALLINT", "SMALLINT" )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "BINARY_INTEGER"  , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "NATURAL"         , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "NATURALN"        , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "POSITIVE"        , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "POSITIVEN"       , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'i', _INT_             , _REAL_       , 0                 , 0             , numberToText   , numToIntLinT   , "SIGNTYPE"        , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'I', _PLS_INTEGER_     , _INT_        , 0                 , 0             , numberToText   , numToIntLinT   , "PLS_INTEGER"     , "INT"     , "INT"      )));
  add(new Fdt(new SysDtConv(DmpType::ftLong        , 'b', _BOOLEAN_         , _PLS_INTEGER_, 0                 , 0             , numberToText   , numToIntLinT   , "BOOLEAN"         , "BOOL"    , "BOOLEAN"  )));
  add(new Fdt(new SysDtConv(DmpType::ftBlob        , 'B', _BLOB_            , EMPTY        , setPrecisionBlob  , 0             , blobToText     , blobToLinT     , "BLOB"            , "BLOB"    , "BLOB"     )));
  add(new Fdt(new SysDtConv(DmpType::ftBlob        , 'C', _CLOB_            , _BLOB_       , setPrecisionClob  , 0             , clobToText     , clobToLinT     , "CLOB"            , "CLOB"    , "BLOB"     )));
  add(new Fdt(new SysDtConv(DmpType::ftNClob       , 'C', _NCLOB_           , _CLOB_       , setPrecisionClob  , 0             , clobToText     , clobToLinT     , "NCLOB"           , "CLOB"    , "BLOB"     )));
  add(new Fdt(new SysDtConv(DmpType::ftBfile       , 'F', _BFILE_           , _BLOB_       , 0                 , 0             , blobToText     , blobToLinT     , "BFILE"           , "BLOB"    , "BLOB"     )));
  add(new Fdt(new SysDtConv(DmpType::ftDate        , 'T', _DATE_            , EMPTY        , 0                 , 0             , dateToText     , dateToLinT     , "DATE"            , "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftTimestamp   , 't', _TIMESTAMP_SIMPLE_, _TIMESTAMP_  , 0                 , 0             , timestampToText, timestampToLinT, "TIMESTAMP SIMPLE", "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftTimestamp   , 't', _TIMESTAMP_SIMPLE_, _TIMESTAMP_  , 0                 , 0             , timestampToText, timestampToLinT, "TIMESTAMP_UNCONSTRAINED", "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftTimestampTZ , 'z', _TIMESTAMP_TZ_    , _TIMESTAMP_  , 0                 , 0             , timestampToText, timestampToLinT, "TIMESTAMP_TZ_UNCONSTRAINED"    , "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftTimestampLTZ, 'l', _TIMESTAMP_LTZ_   , _TIMESTAMP_  , 0                 , 0             , timestampToText, timestampToLinT, "TIMESTAMP_LTZ_UNCONSTRAINED"   , "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftDayToSec    , 'S', _INTERVAL_DTS_    , _INTERVAL_   , setPrecisionInterv, 0             , intervalToText , intervalToLinT , "DSINTERVAL_UNCONSTRAINED"    , "DATE"    , "DATE"     )));
  add(new Fdt(new SysDtConv(DmpType::ftYearToMonth , 'M', _INTERVAL_YTM_    , _INTERVAL_   , setPrecisionInterv, 0             , intervalToText , intervalToLinT , "YMINTERVAL_UNCONSTRAINED"    , "DATE"    , "DATE"     )));
  add(new Sdt(new SysDtConv(DmpType::ftChar        , 'c', _CHAR_            , _STRING_     , setPrecisionByte  , toByteLength  , varcharToText  , rawToLinT      , "CHARACTER"       , "CHAR"    , "CHARACTER"), 32  ));
  add(new Sdt(new SysDtConv(DmpType::ftChar        , 'c', _CHAR_            , _STRING_     , setPrecisionByte  , toByteLength  , varcharToText  , rawToLinT      , "CHAR"            , "CHAR"    , "CHAR"     ), 32  ));
  add(new Sdt(new SysDtConv(DmpType::ftRaw         , 'w', _RAW_             , _LONG_RAW_   , setPrecisionRaw   , toByteLength  , rawToText      , rawToLinT      , "RAW"             , "BYTE"    , "BYTE"     ), 32  ));
  add(new Sdt(new SysDtConv(DmpType::ftRaw         , 'W', _LONG_RAW_        , EMPTY        , setPrecisionRaw   , toByteLength  , rawToText      , rawToLinT      , "LONG RAW"        , "BYTE"    , "BYTE"     ), 32  ));
  add(new Sdt(new SysDtConv(DmpType::ftRowid       , 'u', _ROWID_           , _UROWID_     , setPrecisionRowid , toByteLength  , rawToText      , rawToLinT      , "ROWID"           , "VARCHAR" , "VARCHAR"  ), 10  ));
  add(new Sdt(new SysDtConv(DmpType::ftUrowid      , 'U', _UROWID_          , EMPTY        , setPrecisionRowid , toByteLength  , rawToText      , rawToLinT      , "UROWID"          , "VARCHAR" , "VARCHAR"  ), 4000));
  add(new Sdt(new SysDtConv(DmpType::ftLong        , 'L', _LONG_            , _STRING_     , setPrecisionByte  , toByteLength  , varcharToText  , varcharToLinT  , "LONG"            , "VARCHAR" , "VARCHAR"  )));
  add(new Sdt(new SysDtConv(DmpType::ftVarchar     , 'v', _VARCHAR_         , EMPTY        , setPrecisionByte  , toByteLength  , varcharToText  , varcharToLinT  , "VARCHAR2"        , "VARCHAR" , "VARCHAR"  )));
  add(new Fdt(new SysDtConv(DmpType::ftNChar       , 'r', _NCHAR_           , _NVARCHAR_   , setPrecisionByte  , toByteLength  , varcharToText  , rawToLinT      , "NCHAR"           , "NCHAR"   , "NCHAR"    )));
  add(new Fdt(new SysDtConv(DmpType::ftNVarchar    , 'R', _NVARCHAR_        , EMPTY        , setPrecisionByte  , toByteLength  , rawToText      , varcharToLinT  , "NVARCHAR2"       , "NVARCHAR", "NVARCHAR" )));
  add(new Fdt(new SysDtConv(DmpType::ftNVarchar    , 'R', _NVARCHAR_        , EMPTY        , setPrecisionByte  , toByteLength  , rawToText      , varcharToLinT  , "NVARCHAR"        , "NVARCHAR", "NVARCHAR" )));
  add(new Sdt(new SysDtConv(DmpType::ftVarchar     , 'v', _VARCHAR_         , EMPTY        , setPrecisionByte  , toByteLength  , varcharToText  , varcharToLinT  , "STRING"          , "VARCHAR" , "VARCHAR"  )));
  add(new Sdt(new SysDtConv(DmpType::ftVarchar     , 'v', _VARCHAR_         , EMPTY        , setPrecisionByte  , toByteLength  , varcharToText  , varcharToLinT  , "VARCHAR"         , "VARCHAR" , "VARCHAR"  )));
  // TODO: у этих типов должна быть кастомная поодержка конвертации. Т.е. их и для процедурного языка и для
  //       реляционной модели нужно преобразовывать в допустимые абстракции
  //       (в реляционное представление например, или в структурное если это возможно)
  add(new Sdt(new SysDtConv(DmpType::ftXML         , 'X', _XMLTYPE_         , EMPTY      , setPrecisionObject, 0             , stubToText     , stubToLinT     , "AS XMLTYPE"      , "BLOB"    , "BLOB"    )));
  add(new Sdt(new SysDtConv(DmpType::ftObject      , 'O', _OBJECT_          , EMPTY      , setPrecisionObject, 0             , blobToText     , blobToLinT     , "AS OBJECT"       , "BLOB"    , "BLOB"    )));
  add(new Sdt(new SysDtConv(DmpType::ftNestedTable , 'E', _NESTED_TABLE_    , EMPTY      , setPrecisionObject, 0             , blobToText     , blobToLinT     , "NESTED TABLE "   , "BLOB"    , "BLOB"    )));
  add(new Sdt(new SysDtConv(DmpType::ftVarray      , 'Y', _VARRAY_          , EMPTY      , setPrecisionObject, 0             , blobToText     , blobToLinT     , "AS VARRAY"       , "BLOB"    , "BLOB"    )));
}

void GlobalDatatype::add(Ptr<Sm::GlobalDatatype::FundamentalDatatype> v) {
  systemTypesMap.insert(DHCChar(v->converters->oracleName), v);
  typesParentRelation.insert((int)(v->converters->typeCathegory), (int)(v->converters->parentTypeCathegory));
}

Sm::IsSubtypeValues GlobalDatatype::isSubtype(DatatypeCathegory subtype, DatatypeCathegory supertype, bool plContext) {
  if (subtype == supertype)
    return Sm::IsSubtypeValues::EXACTLY_EQUALLY;
  return HasImplicitConversion::hasImplicitConversion(subtype, supertype, plContext);
}

//int GlobalDatatype::distance(DatatypeCathegory type1, DatatypeCathegory type2, bool plContext) {
//  int v = GlobalDatatype::isSubtype(type1, type2, plContext);
//  if (v == 2)
//    return 0;
//  else if (v > 0) {
//    int cnt = 1;
//    for (int * parent = typesParentRelation.find((int)type1); *parent != (int)type2;
//               parent = typesParentRelation.find((int)type1))
//    {
//      cnt++;
//      if (((DatatypeCathegory)*parent) == EMPTY       ||
//          ((DatatypeCathegory)*parent) == _TIMESTAMP_ ||
//          ((DatatypeCathegory)*parent) == _INTERVAL_)
//        return -1;
//    }
//    return cnt;
//  }
//  else if (v < 0)
//    return GlobalDatatype::distance(type2, type1, plContext);
//  else
//    return -1;
//}

Ptr<Id> FundamentalDatatype::getName() const { return name_; }

bool FundamentalDatatype::isNumberSubtype() const {
  switch (converters->typeCathegory) {
    case _NUMBER_:
    case _NUMERIC_:
    case _DECIMAL_:
    case _DOUBLE_:
    case _REAL_:
    case _INT_:
    case _BIGINT_:
    case _PLS_INTEGER_:
    case _SMALLINT_:
    case _BOOLEAN_:
      return true;
    default:
      return false;
  }
}

bool FundamentalDatatype::isInt() const {
  switch (converters->typeCathegory) {
    case _INT_:
    case _PLS_INTEGER_:
      return true;
    default:
      return false;
  }
}

bool FundamentalDatatype::isNumberDatatype() const {
  return Sm::GlobalDatatype::HasImplicitConversion::greatherDatatypeCathegories[converters->typeCathegory] ==
      Sm::GlobalDatatype::HasImplicitConversion::__NUMBER;
}

bool FundamentalDatatype::isSmallint() const { return converters->typeCathegory == _SMALLINT_; }

bool FundamentalDatatype::isRowidDatatype() const {
  switch (converters->typeCathegory) {
    case _ROWID_:
    case _UROWID_:
      return true;
    default:
      return false;
  }
}

bool FundamentalDatatype::isDateDatatype() const {
  switch (converters->typeCathegory) {
    case Sm::GlobalDatatype::_DATE_:
    case Sm::GlobalDatatype::_TIMESTAMP_:
    case Sm::GlobalDatatype::_TIMESTAMP_SIMPLE_:
    case Sm::GlobalDatatype::_TIMESTAMP_TZ_:
    case Sm::GlobalDatatype::_TIMESTAMP_LTZ_:
//    case Sm::GlobalDatatype::_INTERVAL_:
//    case Sm::GlobalDatatype::_INTERVAL_DTS_:
//    case Sm::GlobalDatatype::_INTERVAL_YTM_:
      return true;
    default:
      return false;
  }
  return false;
}

bool FundamentalDatatype::isIntervalDatatype() const {
  switch (converters->typeCathegory) {
    case Sm::GlobalDatatype::_INTERVAL_:
    case Sm::GlobalDatatype::_INTERVAL_DTS_:
    case Sm::GlobalDatatype::_INTERVAL_YTM_:
      return true;
    default:
      return false;
  }
  return false;

}


FundamentalDatatype::~FundamentalDatatype() {}

FundamentalDatatype::FundamentalDatatype(Ptr<SysTypeInfo> _converters)
  : converters(_converters)
{
  name_ = new Id(string(converters->oracleName), this);
}

Ptr<Sm::Datatype> FundamentalDatatype::getDatatype() const {
  if (datatype)
    return datatype;
  datatype = new Sm::Datatype(new Sm::Id(string(this->converters->oracleName),
                             syntaxerContext.model->globalDatatypes.systemTypesMap.find(THCChar(this->converters->oracleName))->object(),
                             false));
  return datatype;
}

bool Sm::GlobalDatatype::FundamentalDatatype::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  if (oth->ddlCathegory() != ResolvedEntity::FundamentalDatatype_) {
    Ptr<Datatype> t = ResolvedEntity::getLastConcreteDatatype(oth);
    if (ResolvedEntity *unrefOth = t->getNextDefinition())
      if (oth != unrefOth)
        return isExactlyEquallyByDatatype(unrefOth);
    return false;
  }
  if (SysTypeInfo * c = oth->getFundamentalDatatypeConverters())
    return c->typeCathegory == converters->typeCathegory;
  return false;
}

Sm::IsSubtypeValues FundamentalDatatype::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  throw 999;
  if (ResolvedEntity *superDdl = supertype->getResolvedConcreteDefinition() ) {
    if (superDdl->ddlCathegory() == ResolvedEntity::FundamentalDatatype_)
      return superDdl->isSubtypeFundamental(converters->typeCathegory, plContext);
  }
  return EXPLICIT;
}

int FundamentalDatatype::greather(FundamentalDatatype *supertype) {
  return HasImplicitConversion::greather(this->converters->typeCathegory, supertype->converters->typeCathegory);
}


} // end of namespace GlobalDatatype

} // end of namespace Sm
