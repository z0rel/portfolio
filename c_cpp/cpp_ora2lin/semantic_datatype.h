#ifndef SEMANTIC_DATATYPE_H
#define SEMANTIC_DATATYPE_H

#include "semantic_base.h"
#include "semantic_id.h"

namespace Sm {

namespace GlobalDatatype {
  class FundamentalDatatype;
}

class Subtype : public Declaration {
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>         name;
  Ptr<Datatype>   datatype;
  Ptr<Constraint> constraint_;

public:
  bool            notNull;

  bool isExactlyEquallyByDatatype(ResolvedEntity* t);

  ScopedEntities  ddlCathegory() const { return Subtype_; }
  bool            isDefinition() const { return true; }
  Ptr<Datatype>   getDatatype () const;
  Constraint     *constraint  () const { return constraint_.object(); }

  bool datatypeIsNotResolved  () const;
  inline bool isNumberDatatype() const;
  inline bool isNumberSubtype () const;

  inline bool isInt    () const;
  inline bool isBigint () const;
  inline bool isDecimal() const;
  inline bool isDouble () const;
  inline bool isFloat  () const;
  inline bool isReal   () const;
  inline bool isRowidDatatype  () const;
  inline bool isDateDatatype   () const;
  inline bool isVarcharDatatype() const;
  inline bool isBool           () const;

  bool getFields(EntityFields &fields) const;
  bool getFieldRef(Ptr<Id> &field);
  ResolvedEntity* getNextDefinition() const;
  bool isSubtype() const { return true; }
  bool isRefCursor()  const;
  Function *getDefaultConstructor() const;

  Ptr<ResolvedEntity> tidDdl() const;

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *t, bool plContext) const;
  Ptr<Id> getName() const { return name; }

  void linterReference(Sm::Codestream &str);

  Subtype(CLoc l, Id *n, Datatype *t, Constraint *c, bool _notNull);
  SemanticTree *toSTreeBase() const;
  void translateAssign(Sm::Codestream &str, Sm::Type::RefInfo *ref, Sm::PlExpr *expr);
  void translateObjRef(Sm::Codestream &str, Sm::Type::RefInfo *ref);
  Subtype* toSelfSubtype() const { return const_cast<Subtype*>(this); }

  Type::ObjectType *toSelfObjectType() const;
  void traverseModelStatements(StatementActor &) {}
};

class Datatype;

class DatatypeLeakManager {
public:
  static std::set<Datatype*> managedTypes;
  Datatype* t_;
  DatatypeLeakManager(Datatype* t) : t_(t) { managedTypes.insert(t); }
  ~DatatypeLeakManager() { managedTypes.erase(t_); }
};

class Datatype : public ResolvedEntitySNodeLoc
{
public:
  typedef map<string, Ptr<Datatype> > GlobalDatatypes;
  typedef map<int, Ptr<Datatype>    > Varchar2LengthDatatypes;

  static GlobalDatatypes         globalDatatypes;
  static Varchar2LengthDatatypes varchar2LengthDatatypes;
  static Varchar2LengthDatatypes charLengthDatatypes;

  Ptr<IdRef> tid;  // REFERENCE to type or object
  uint16_t precision = 0;
  uint16_t scale     = std::numeric_limits<uint16_t>::max();

  DatatypeFlags flags;
  Datatype *finalType = NULL;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

  static Datatype* getSystemDatatype(const string &name);
  static Datatype* getSystemDatatype(const string &name, int prec, int scale);

  static void unrefDatatype(Datatype *&unrefT1, Datatype *&unrefT0, ResolvedEntity *&unrefFdt, std::vector<Datatype*> &tVector);
  static Sm::GlobalDatatype::DatatypeCathegory fundCat(Datatype* t, Sm::GlobalDatatype::FundamentalDatatype *unref);
  static unsigned int getHsBase(Sm::GlobalDatatype::DatatypeCathegory l, Sm::GlobalDatatype::DatatypeCathegory r);
  static Datatype* selectDatatype(Datatype *t1, Datatype *t0);

  void setIsDefault() { flags.v |= FLAG_DATATYPE_IS_DEFAULT; }
  void setIsNull()    { flags.v |= FLAG_DATATYPE_IS_NULL; }
  bool tryTranslateToIntegralType(Sm::Codestream &str);
  uint16_t truncNumberPrecision() const;
  uint16_t truncNumberScale() const;
  uint16_t truncVarcharPrecision() const;
  uint16_t truncNVarcharPrecision() const;

  inline void checkTid() const {
    if (!tid || tid->empty() || !*(tid->begin()))
      throw 999;
  }
  inline void checkTidObj() const {
    if (tid->empty() || !*(tid->begin()))
      throw 999;
  }

public:

//  Datatype() { throw 999; }
  Datatype(Ptr<IdEntitySmart> o) : GrammarBase(o->entity()->getLLoc()), tid(o)  { checkTid(); }
  Datatype(Ptr<Id>  o)           : GrammarBase(o->getLLoc()), tid(new IdRef(o)) { checkTidObj(); }
  Datatype(Id      *o)           : GrammarBase(o->getLLoc()), tid(new IdRef(o)) { checkTidObj(); }
  Datatype(Ptr<Id2> o)           : GrammarBase(o->getLLoc()), tid(new IdRef(o)) { checkTidObj(); }

  Datatype(Id *o, Ptr<NumericValue> l, Ptr<NumericValue> prec = 0)
    : GrammarBase(o->getLLoc()),
      tid(new IdRef(o)),
      precision(l ? l->getUIntValue() : 0),
      scale(prec ? prec->getUIntValue() : std::numeric_limits<uint16_t>::max()) { checkTidObj(); }
  Datatype(Datatype* o, uint16_t l, uint16_t prec = std::numeric_limits<uint16_t>::max())
    : tid(o->tid), precision(l), scale(prec) { checkTidObj(); }

  Sm::Datatype* toSelfDatatype() const { return (Sm::Datatype*)this; }
  SemanticTree *toSTreeBase() const;

  bool isCleanNumberType() const;
  void linterReference(Sm::Codestream &str);
  void sqlReference(Sm::Codestream &str);
  void concatFlags(unsigned int oth) { flags.v |= oth; }
  void clearScale() { scale = std::numeric_limits<uint16_t>::max(); }

private:
  template <bool (ResolvedEntity::*mf)(void) const>
  inline bool getTidAttribute() const { ResolvedEntity *d = tid->definition(); return d && (d->*mf)(); }

public:

  bool isFundamentalDatatype() const { return getTidAttribute<&ResolvedEntity::isFundamentalDatatype>(); }

  bool isSqlTableContainerFuncall() const { ResolvedEntity *d = tid->definition(); return d && d->isSqlTableContainerFuncall(); }
//  bool isRowidDatatype  () const { return getTidAttribute<ResolvedEntity::isRowidDatatype>() ResolvedEntity *d = tid.definition(); return  d && d->isRowidDatatype  (); }
  bool isRowidDatatype  () const { return getTidAttribute<&ResolvedEntity::isRowidDatatype  >(); }
  bool isLikeNum        () const;
  bool isNum            () const;
  bool isLinterNumber   () const;
  bool isCharVarchar    () const;
  bool isNCharVarchar   () const;
  bool isBool           () const { return getTidAttribute<&ResolvedEntity::isBool           >(); }
  bool isNumberDatatype () const { return getTidAttribute<&ResolvedEntity::isNumberDatatype >(); }
  bool isNumberSubtype  () const { return getTidAttribute<&ResolvedEntity::isNumberDatatype >(); } // entry of Number and != BOOLEAN
  bool isSmallint()        const { return getTidAttribute<&ResolvedEntity::isSmallint       >() || translatedToSmallInt(); }
  bool isInt()             const { return getTidAttribute<&ResolvedEntity::isInt            >() || translatedToInt(); }    // entry of int     or translatedToInt && !translatedToBigint
  bool isBigint()          const { return getTidAttribute<&ResolvedEntity::isBigint         >() || translatedToBigint(); } // entry of bigint  or translatedToBigint()
  bool isDecimal()         const { return getTidAttribute<&ResolvedEntity::isDecimal        >(); } // entry of decimal
  bool isDouble()          const { return getTidAttribute<&ResolvedEntity::isDouble         >(); } // entry of double
  bool isFloat()           const { return getTidAttribute<&ResolvedEntity::isFloat          >(); }
  bool isReal()            const { return getTidAttribute<&ResolvedEntity::isReal           >(); } // entry of real
  bool isDateDatatype   () const { return getTidAttribute<&ResolvedEntity::isDateDatatype   >(); }
  bool isIntervalDatatype() const { return getTidAttribute<&ResolvedEntity::isIntervalDatatype>(); }
  bool isClobDatatype   () const { return getTidAttribute<&ResolvedEntity::isClobDatatype   >(); }
  bool isBlobDatatype   () const { return getTidAttribute<&ResolvedEntity::isBlobDatatype   >(); }
  bool isVarcharDatatype() const { return getTidAttribute<&ResolvedEntity::isVarcharDatatype>(); }
  bool isCharDatatype   () const { return getTidAttribute<&ResolvedEntity::isCharDatatype   >(); }
  bool isLongDatatype()    const { return getTidAttribute<&ResolvedEntity::isLongDatatype   >(); }
  bool isNcharDatatype() const { return getTidAttribute<&ResolvedEntity::isNcharDatatype>(); }
  bool isNVarcharDatatype   () const { return getTidAttribute<&ResolvedEntity::isNVarcharDatatype>(); }

  bool isRefCursor()       const { return getTidAttribute<&ResolvedEntity::isRefCursor      >(); }
  bool isObjectType()      const { return getTidAttribute<&ResolvedEntity::isObjectType     >(); }
  bool isCollectionType()  const { return getTidAttribute<&ResolvedEntity::isCollectionType >(); }
  bool isXmlType()         const { return getTidAttribute<&ResolvedEntity::isXmlType        >(); }
  bool isAnydata()         const { return getTidAttribute<&ResolvedEntity::isAnydata        >(); }
  bool isRecordType()      const { return getTidAttribute<&ResolvedEntity::isRecordType     >(); }

  bool isLinterStructType() const;

  bool isExactlyEqually(Datatype *t);
  bool isExactlyEquallyByDatatype(ResolvedEntity *t);

  Type::ObjectType *toSelfObjectType() const { ResolvedEntity *d = tid->definition(); return d ? d->toSelfObjectType() : (Type::ObjectType*)0; }

  bool isChangedNumberDatatype() const;
  static Sm::Datatype* getConcatVarcharDatatype(Datatype *lhs, Datatype *rhs, bool isPlContext, const FLoc &loc);

  Ptr<Id> getName() const { return *(tid->begin()); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext, int inSqlCode) const;
  CastCathegory getCastCathegory(Datatype *oth, bool plContext, bool inSqlCode = false);
  static Datatype* getMaximal(Datatype *lhs, Datatype *rhs, bool pl);
  /**
   * @brief getMaximal На основании lhs и rhs вывести наимениьший общий тип и записать его в dst
   *        dst не обновляется, если его указатель равен выведенному типу.
   * @param pl производить выведение типов с учетом таблицы неявного приведения pl/sql (true) или sql (false)
   * @return true, если минимальный общий тип выведен, false - если из двух типов нельзя неявно вывести общий.
   */
  static bool getMaximal(Ptr<Datatype> &dst, Datatype *lhs, Datatype *rhs, bool pl);
  static CastCathegory castCathegory(Datatype *lhs, Datatype *rhs, bool plContext);

  int getLengthForPlSqlVarcharImplicitConversion();

protected:
  Sm::IsSubtypeValues isSubtypeAsRef(Datatype *supertypeDef, bool plContext) const;
public:
  static Datatype* mkNumberPrec(int prec) { return mkNumber(prec); }
  static Datatype* mkVarchar2Prec(int prec) { return mkVarchar2(prec); }
  static Datatype* mkVarchar2NoPrec() { return mkVarchar2(); }

  bool semanticResolve() const;

  static Datatype* mkBoolean();
  static Datatype* mkString();
  static Datatype* mkInteger();
  static Datatype* mkNumber();
  static Datatype* mkNumber(int prec, int scale = -1);
  static Datatype* mkDouble();
  static Datatype* mkDate();
  static Datatype* mkRowid();
  static Datatype* mkTimestamp();
  static Datatype* mkTimestampTimezone();
  static Datatype* mkTimestampLtzTimezone();
  static Datatype* mkIntervalDayToSecond();
  static Datatype* mkIntervalYearToMonth();
  static Datatype* mkDecimal();
  static Datatype* mkFloat();
  static Datatype* mkReal();
  static Datatype* mkSmallint();
  static Datatype* mkBinaryInteger();
  static Datatype* mkNatural();
  static Datatype* mkNaturalN();
  static Datatype* mkPositive();
  static Datatype* mkPositiveN();
  static Datatype* mkSignType();
  static Datatype* mkPlsInteger();
  static Datatype* mkBlob();
  static Datatype* mkClob();
  static Datatype* mkNClob();
  static Datatype* mkBfile();
  static Datatype* mkChar();
  static Datatype* mkChar(int precision);
  static Datatype* mkRaw();
  static Datatype* mkLongRaw();
  static Datatype* mkURowId();
  static Datatype* mkLong();
  static Datatype* mkVarchar2();
  static Datatype* mkVarchar2(int precision);
  static Datatype* mkNChar();
  static Datatype* mkNVarchar2();
  static Datatype* mkNVarchar();
  static Datatype* mkVarchar();
  static Datatype* mkNull();
  static Datatype* mkAnydata();
  static Datatype* mkXmltype();
  static Datatype* mkDefault();

  Ptr<ResolvedEntity> tidDdl() const;
  inline size_t          tidSize() const { return tid->size(); }
  inline ResolvedEntity *tidDef()  const { return tid->definition(); }
  inline Id*             tidEntity() const { return *(tid->begin()); }
  ScopedEntities  ddlCathegory() const { return Datatype_; }
  ScopedEntities ddlCathegoryWithDatatypeSpec() const;
  Ptr<Datatype>   getDatatype()  const { return (Datatype*)this; }
  bool            getFieldRef(Ptr<Id> &field);
  ResolvedEntity* getNextDefinition() const;

  Ptr<Datatype> getFinalType();
  bool getFields(EntityFields &fields) const;
  ResolvedEntity* tryResolveConcreteDefinition();
  inline bool isNull()              const { return flags.v & FLAG_DATATYPE_IS_NULL; }
  inline bool isDefault()           const { return flags.v & FLAG_DATATYPE_IS_DEFAULT; }
  inline bool isRef()               const { return flags.v & FLAG_DATATYPE_IS_REF; }
  inline bool isTypeOf()            const { return flags.v & FLAG_DATATYPE_IS_TYPE_OF; }
  inline bool isRowTypeOf()         const { return flags.v & FLAG_DATATYPE_IS_ROWTYPE_OF; }

  inline bool isEmptyVarchar()      const { return flags.v & FLAG_DATATYPE_IS_EMPTY_VARCHAR; }
  inline void setEmptyVarchar()           { flags.v |= FLAG_DATATYPE_IS_EMPTY_VARCHAR; }

  inline bool isEverything()        const { return /*tid.empty() &&*/ (flags.v & (FLAG_DATATYPE_IS_DEFAULT | FLAG_DATATYPE_IS_NULL)); }
  inline bool isDatatype()          const { return true; }
  inline void setIsRef()                  { flags.v |= FLAG_DATATYPE_IS_REF; }
  inline void setIsRowtypeOf()            { flags.v |= FLAG_DATATYPE_IS_ROWTYPE_OF; }

  inline bool hasMaxCharLen() const { return !isEmptyVarchar() && (!precision || precision >= 4000); }
  inline bool isMaxNumberType() const { return (!precision && scaleIsNotSet()) || (precision >= 30 && scale >= 10); }
  inline unsigned int getMaxNumberPrecision() const { return precision > 30 ? 30 : precision; }
  inline unsigned int getMaxNumberScale()     const { return scaleIsNotSet() ? 0 : (scale > 10 ? 10 : scale); }

private:
  bool likeBigint() const   { return precision > 10; }
  bool likeInt() const      { return precision > 5  && precision <= 10; }
  bool likeSmallInt() const { return precision > 0  && precision <= 5;  }

public:
  bool translatedToBigint() const   { return translatedToIntegral() && likeBigint(); }
  bool translatedToInt() const      { return translatedToIntegral() && likeInt(); }
  bool translatedToSmallInt() const { return translatedToIntegral() && likeSmallInt();  }
  bool translatedToIntegral() const { return isNumberDatatype() && scaleIsEmpty(); }

  std::string toStringNumber() const;

  void lenprecDefinition(Codestream &str);
  virtual void oracleDefinition(Codestream &str);
  virtual void linterDefinition(Codestream &str);
  virtual void translateVariableType (Sm::Codestream &str, Sm::ResolvedEntity *var, bool addTypeName = false);
  virtual Sm::Function *getDefaultConstructor() const;

  bool scaleIsSet()     const { return  scale != numeric_limits<typeof(scale)>::max(); }
  bool scaleIsNotSet()  const { return  scale == numeric_limits<typeof(scale)>::max(); }
  bool scaleIsEmpty()   const { return  scaleIsNotSet() || scale == 0; }
  bool scalePrecIsNotSet() const { return  !precision && scaleIsNotSet(); }

  bool scaleOrPrecIsSet() const { return !scaleIsNotSet() || precision; }

  uint16_t getLength() const { return precision; }
  void     setLength(uint16_t v) { precision = v; }

  bool isBigNumber() { return precision > 30 || (scaleIsSet() && scale > 10); }
//  bool isBigNumber() { return scaleIsNotSet() && precision == 12; }
  void commentForTruncLength(Sm::Codestream &str);


  void replaceChildsIf(Sm::ExprTr tr);

private:
  bool numericOrDecimal3010(Sm::Codestream &str);
};




inline Codestream& operator<<(Codestream& s, Datatype &obj) { return obj.translate(s); }

inline bool Subtype::isNumberDatatype()  const { return datatype && datatype->isNumberDatatype(); }
inline bool Subtype::isNumberSubtype()   const { return datatype && datatype->isNumberDatatype(); }
inline bool Subtype::isInt()             const { return datatype && datatype->isInt    (); }
inline bool Subtype::isBigint()          const { return datatype && datatype->isBigint (); }
inline bool Subtype::isDecimal()         const { return datatype && datatype->isDecimal(); }
inline bool Subtype::isDouble()          const { return datatype && datatype->isDouble (); }
inline bool Subtype::isFloat()           const { return datatype && datatype->isFloat  (); }
inline bool Subtype::isReal()            const { return datatype && datatype->isReal   (); }
inline bool Subtype::isRowidDatatype()   const { return datatype && datatype->isRowidDatatype  (); }
inline bool Subtype::isDateDatatype()    const { return datatype && datatype->isDateDatatype   (); }
inline bool Subtype::isVarcharDatatype() const { return datatype && datatype->isVarcharDatatype(); }
inline bool Subtype::isBool()            const { return datatype && datatype->isBool(); }


}


#endif // SEMANTIC_DATATYPE_H
