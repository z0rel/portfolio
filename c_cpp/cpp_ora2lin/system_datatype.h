#ifndef SYSTEM_DATATYPE_H
#define SYSTEM_DATATYPE_H

#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include "hash_table.h"
#include "smartptr.h"
#include "crossplatform_func.h"
#include "resolved_entity.h"

namespace Sm {

using namespace BackportHashMap;
using namespace smart;

class Datatype;
class Id;

namespace GlobalDatatype {
  class DmpType {
    enum MASKS {
      maskUnknown        = 1 << 0 ,
      maskRaw            = 1 << 1 ,
      maskNumber         = 1 << 2 ,
      maskLong           = 1 << 3 ,
      maskFloat          = 1 << 4 ,
      maskChar           = 1 << 5 ,
      maskNChar          = 1 << 6 ,
      maskNVarchar       = 1 << 7 ,
      maskSYS            = 1 << 8 ,
      maskVarchar        = 1 << 9 ,
      maskDate           = 1 << 10,
      maskTimestamp      = 1 << 11,
      maskTimestampTZ    = 1 << 12,
      maskTimestampLTZ   = 1 << 13,
      maskYearToMonth    = 1 << 14,
      maskDayToSec       = 1 << 15,
      maskClob           = 1 << 16,
      maskBlob           = 1 << 17,
      maskBfile          = 1 << 18,
      maskNClob          = 1 << 19,
      maskObject         = 1 << 20,
      maskNestedTable    = 1 << 21,
      maskVarray         = 1 << 22,
      maskRowid          = 1 << 23,
      maskUrowid         = 1 << 24,
      maskXML            = 1 << 25,
      maskANYDATA        = 1 << 26,
      maskIsCharacter    = maskChar      | maskVarchar                                   ,
      maskIsMultibyte    = maskNChar     | maskNVarchar                                  ,
      maskIsRaw          = maskRaw       | maskRowid       | maskUrowid                  ,
      maskIsNumber       = maskNumber    | maskLong        | maskFloat                   ,
      maskIsObject       = maskObject    | maskNestedTable | maskVarray                  ,
      maskIsTimestamp    = maskTimestamp | maskTimestampTZ | maskTimestampLTZ            ,
      maskIsLob          = maskBlob      | maskNClob       | maskClob         | maskBfile,
      maskIsDeferredBlob = maskIsObject  | maskXML         | maskANYDATA      | maskIsLob
    };
    struct       MaskArray { unsigned int v[0x200]; MaskArray(); };
    static const MaskArray maskArray;

  public:
    enum DmpCathegory {
      ftUnknown          = 0x100, // stub
      ftRaw              = 0x017,

      ftNumber           = 0x002, // это и number и float
      ftLong             = 0x008,
      ftFloat            = 0x102, // stub

      ftChar             = 0x060, // и char и NCHAR
      ftNChar            = 0x160, // stub
      ftNVarchar         = 0x101, // stub

      ftSYS              = 0x1FF, // stub не идентификатор типа, а идентификатор пространства имен
      ftVarchar          = 0x001,

      ftDate             = 0x00C,

      ftTimestamp        = 0x0B4,
      ftTimestampTZ      = 0x0B5,
      ftTimestampLTZ     = 0x0E7,

      ftYearToMonth      = 0x0B6, // Для ускорения определения класса
      ftDayToSec         = 0x0B7,

      ftClob             = 0x070,
      ftBlob             = 0x071,
      ftBfile            = 0x072,
      ftNClob            = 0x170, // stub

      ftObject           = 0x079,
      ftNestedTable      = 0x07A,
      ftVarray           = 0x07B,

      ftRowid            = 0x045,
      ftUrowid           = 0x0D0,

      ftXML              = 0x03A,
      ftANYDATA          = 0x13A  // stub на самом деле - те же 0x3A
    };

    DmpCathegory type;

    inline operator unsigned int()                 const { return (unsigned int)type; }

    inline bool operator== (const DmpType &o) const { return type == o.type; }
    inline bool operator== (DmpCathegory   t) const { return type == t; }
    inline void operator=  (DmpCathegory   t)       { type        =  t; }

    inline bool isXML         () const { return type == ftXML;     }
    inline bool isAnydata     () const { return type == ftANYDATA; }
    inline bool isUnknown     () const { return type == ftUnknown; }
    inline bool isFloat       () const { return type == ftFloat;   }
    inline bool isLong        () const { return type == ftLong;    }
    inline bool isCharacter   () const { return(maskArray.v[type] & maskIsCharacter   ) != 0; }
    inline bool isMultibyte   () const { return(maskArray.v[type] & maskIsMultibyte   ) != 0; }
    inline bool isRaw         () const { return(maskArray.v[type] & maskIsRaw         ) != 0; }
    inline bool isNumber      () const { return(maskArray.v[type] & maskIsNumber      ) != 0; }
    inline bool isLob         () const { return(maskArray.v[type] & maskIsLob         ) != 0; }
    inline bool isObject      () const { return(maskArray.v[type] & maskIsObject      ) != 0; }
    inline bool isTimestamp   () const { return(maskArray.v[type] & maskIsTimestamp   ) != 0; }
    inline bool isDeferredBlob() const { return(maskArray.v[type] & maskIsDeferredBlob) != 0; }

    inline DmpType( const DmpType & other ) : type(other.type) {}
    inline DmpType( unsigned int    id    ) : type((DmpCathegory)id) {}
    inline DmpType()                        : type(ftUnknown) {}
  };

  struct SysTypeInfo : public Smart {
//    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
    /// Для короткого доступа к типу поля при парсинге dmp
    struct FldType {
      int type;
      int field1;
    };

    typedef void (*SetType                )(int &precision, FldType &src, FILE *);
    typedef void (*GetLengthPrecDefinition)(std::string &result, int        length, int    precision, DmpType::DmpCathegory);
    typedef void (*GetValueText           )(std::string &result, const void *value, size_t size     );
    typedef bool (*Ora2CallConverter      )(char *& pbuf, size_t & bufsize, const void * value, size_t size );

    /// Dmp-идентификатор Oracle для хэширования
    DmpType::DmpCathegory   fieldTypeID;
    char                    overloadConversionId;
    DatatypeCathegory       typeCathegory;
    DatatypeCathegory       parentTypeCathegory;
    /// Обработчик для установки типа значения - нужен для настройки типа в INSERT INTO.
    /// Например из файла вычитывается длина либо длина и точность, либо еще и дополнительные ссылки
    SetType                 setType;
    /// Обработчик для преобразования длины и точности в строку -> используется в виртуальных ф-циях field
    GetLengthPrecDefinition getBrLenPrec;
    /// Обработчик для преобразования значения данного типа в строку
    GetValueText            getValueText;
    Ora2CallConverter       ora2CallConverter;

    std::string                   oracleName;
    std::string                   linterProcName;
    std::string                   linterSqlName;

    SysTypeInfo(DmpType::DmpCathegory   _fieldTypeID,
                char                    _overloadConversionId,
                DatatypeCathegory       _typeCathegory,
                DatatypeCathegory       _parentTypeCathegory,
                SetType                 _setType,
                GetLengthPrecDefinition _getBrLenPrec,
                GetValueText            _getValueText,
                Ora2CallConverter       _ora2CallConverter,
                std::string             _oracleName,
                std::string             _linterProcName,
                std::string             _linterSqlName);

    virtual ~SysTypeInfo() {}
  };

  class FundamentalDatatype : public virtual Smart, public ResolvedEntity {
  public:
    mutable Ptr<Sm::Datatype> datatype;
    /// Имя Oracle для хэширования
    Ptr<SysTypeInfo> converters;
    Ptr<Id> name_;

    FundamentalDatatype* toSelfFundamentalDatatype() const { return (FundamentalDatatype*)this; }
    SysTypeInfo* getFundamentalDatatypeConverters() const { return converters.object(); }
    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
    FundamentalDatatype(Ptr<SysTypeInfo> _converters);
    virtual std::string getLinterTypename(int = 0, int = 0, bool = false) const { return converters->linterSqlName; }
    Ptr<Id> getName() const;

    bool getFieldRef(Ptr<Id> &) { return false; }
    bool getFields(EntityFields &) const { return false; }
    ScopedEntities    ddlCathegory() const { return FundamentalDatatype_; }
    bool isDefinition() const { return true; }
    Ptr<Sm::Datatype> getDatatype () const;
    inline SemanticTree *toSTreeBase() { return 0; }

    bool isFundamentalDatatype() const { return true; }
    bool isNumberSubtype () const;
    bool isSmallint()     const;
    bool isInt()     const;
    bool isXmlType() const { return converters->typeCathegory == Sm::GlobalDatatype::_XMLTYPE_; }
    bool isAnydata() const { return converters->typeCathegory == Sm::GlobalDatatype::_ANYDATA_; }
    bool isDecimal() const { return converters->typeCathegory == Sm::GlobalDatatype::_DECIMAL_; }
    bool isDouble()  const { return converters->typeCathegory == Sm::GlobalDatatype::_DOUBLE_; }
    bool isFloat()   const { return converters->typeCathegory == Sm::GlobalDatatype::_REAL_;  }
    bool isReal()    const { return converters->typeCathegory == Sm::GlobalDatatype::_REAL_; }
    bool isNumberDatatype() const;
    bool isBool          () const { return converters->typeCathegory == Sm::GlobalDatatype::_BOOLEAN_; }
    bool isRowidDatatype () const;
    bool isClobDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_CLOB_; }
    bool isBlobDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_BLOB_; }

    bool isDateDatatype() const;
    bool isIntervalDatatype() const;
    bool isVarcharDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_VARCHAR_; }
    bool isNcharDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_NCHAR_; }
    bool isNVarcharDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_NVARCHAR_; }
    bool isCharDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_CHAR_; }
    bool isLongDatatype() const { return converters->typeCathegory == Sm::GlobalDatatype::_LONG_; }

    void linterReference(Sm::Codestream &str) {
      if (str.procMode() == Sm::CodestreamState::SQL)
        str << converters->linterSqlName;
      else if (str.procMode() == Sm::CodestreamState::PROC)
        str << converters->linterProcName;
    }

    Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
    int greather(FundamentalDatatype *supertype);
    inline Sm::IsSubtypeValues isSubtypeFundamental(DatatypeCathegory typeCat, bool plContext) const;

    bool isExactlyEquallyByDatatype(ResolvedEntity *oth);

    virtual ~FundamentalDatatype();
  };

  struct StringDatatype : public FundamentalDatatype {
    int defaultLength;
    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
    StringDatatype(Ptr<SysTypeInfo> _converters, int _defaultLength = -1)
      : FundamentalDatatype(_converters), defaultLength(_defaultLength) {}
    std::string getLinterTypename(int = 0, int = 0, bool = false) const { return converters->linterSqlName; }
    bool isVarcharDatatype() const { return true; }
    bool isCharDatatype() const { return true; }
  };
  struct NumberDatatype : public FundamentalDatatype {
    virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
    NumberDatatype(Ptr<SysTypeInfo> _converters) : FundamentalDatatype(_converters) {}
    std::string getLinterTypename(int scale, int precision, bool isProc) const {
      if (scale > 0)
        return converters->linterSqlName;
      else if (precision < 10)
        return "INT";
      else if (precision < 19)
        return "BIGINT";
      if (isProc)
        return "NUMERIC";
      return converters->linterSqlName;
    }
    bool isNumberDatatype() const { return true; }
    bool isNumberSubtype () const { return true; }
  };


  class HasImplicitConversion {
  public:

    typedef int DatatypeCathegories[FUNDAMENTAL_TYPES_COUNT];
    static const int datatypeCathegories[FUNDAMENTAL_TYPES_COUNT];
    static const int greatherDatatypeCathegories[FUNDAMENTAL_TYPES_COUNT];

    enum SysTypes {
      __NVARCHAR2 =  0,
      __VARCHAR2  =  1,
      __NCHAR     =  2,
      __CHAR      =  3,
      __DATE      =  4,
      __INTERVAL  =  5, // datetime interval
      __NUMBER    =  6,
      __DOUBLE    =  7,
      __REAL      =  8,
      __LONG      =  9,
      __RAW       = 10,
      __ROWID     = 11,
      __BLOB      = 12,
      __BFILE     = 13,
      __NCLOB     = 14,
      __CLOB      = 15,
      __BIGINT    = 16,
      __INT       = 17,
      __SMALLINT  = 18,
      __BOOL      = 19,

      __LAST_MAPPED_TYPE
    };

    static const unsigned int greatherTable[(int)__LAST_MAPPED_TYPE][(int)__LAST_MAPPED_TYPE];
    static const Sm::IsSubtypeValuesEnum::IsSubtypeCathegory implicitConversionTable[(int)__LAST_MAPPED_TYPE][(int)__LAST_MAPPED_TYPE];
    static const Sm::IsSubtypeValuesEnum::IsSubtypeCathegory implicitConversionPlTable[(int)__LAST_MAPPED_TYPE][(int)__LAST_MAPPED_TYPE];


    HasImplicitConversion();
    inline static Sm::IsSubtypeValuesEnum::IsSubtypeCathegory hasImplicitConversion(DatatypeCathegory from, DatatypeCathegory to, bool isPlContext) {
      int fromIt, toIt;
      if ((fromIt = datatypeCathegories[from]) < 0 || (toIt = datatypeCathegories[to]) < 0)
        return Sm::IsSubtypeValuesEnum::EXPLICIT;
      if (isPlContext)
        return implicitConversionPlTable[fromIt][toIt];
      else
        return implicitConversionTable[fromIt][toIt];
    }
    inline static int greather(DatatypeCathegory from, DatatypeCathegory to) {
      int fromIt, toIt;
      if ((fromIt = greatherDatatypeCathegories[from]) < 0 || (toIt = greatherDatatypeCathegories[to]) < 0)
        return 0;
      return greatherTable[fromIt][toIt];
    }
  };


  class GlobalDatatype {
  public:
    typedef HashMap<DHCChar, Ptr<FundamentalDatatype> > SystemTypesMap;
    SystemTypesMap systemTypesMap;
    HashMap<int, int>                           typesParentRelation;
    void add(Ptr<FundamentalDatatype> v);
    static Sm::IsSubtypeValues isSubtype(DatatypeCathegory subtype, DatatypeCathegory supertype, bool plContext);
    bool getFieldRef(Ptr<Id> &) { return false; }

    GlobalDatatype();
    virtual ~GlobalDatatype() {
      for (SystemTypesMap::iterator i = systemTypesMap.begin(); i; ++i) {
        *(i.value) = 0;
      }
    }
  };


  inline Sm::IsSubtypeValues FundamentalDatatype::isSubtypeFundamental(DatatypeCathegory typeCat, bool plContext) const {
    return GlobalDatatype::isSubtype(typeCat, converters->typeCathegory, plContext);
  }
}

}


#endif
