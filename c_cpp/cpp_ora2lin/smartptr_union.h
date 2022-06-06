#ifndef SMARTPTR_UNION
#define SMARTPTR_UNION

#include <functional>

namespace smart {

#define FLAG_BIT(b)       (1 << (b))
#define FLAG_SMART_BIT0   FLAG_BIT(0)
#define FLAG_SMART_BIT1   FLAG_BIT(1)
#define FLAG_SMART_BIT2   FLAG_BIT(2)
#define FLAG_SMART_BIT3   FLAG_BIT(3)
#define FLAG_SMART_BIT4   FLAG_BIT(4)
#define FLAG_SMART_BIT5   FLAG_BIT(5)
#define FLAG_SMART_BIT6   FLAG_BIT(6)
#define FLAG_SMART_BIT7   FLAG_BIT(7)
#define FLAG_SMART_BIT8   FLAG_BIT(8)
#define FLAG_SMART_BIT9   FLAG_BIT(9)
#define FLAG_SMART_BIT10  FLAG_BIT(10)
#define FLAG_SMART_BIT11  FLAG_BIT(11)
#define FLAG_SMART_BIT12  FLAG_BIT(12)
#define FLAG_SMART_BIT13  FLAG_BIT(13)
#define FLAG_SMART_BIT14  FLAG_BIT(14)
#define FLAG_SMART_BIT15  FLAG_BIT(15)

typedef unsigned short SmartCount;

namespace attributes {

#define FLAG_ENTITY_ATTRIBUTES_READ    FLAG_BIT(0)
#define FLAG_ENTITY_ATTRIBUTES_WRITE   FLAG_BIT(1)
#define FLAG_ENTITY_ATTRIBUTES_IS_OPEN FLAG_BIT(2)

struct EntityAttributesFlags {
  SmartCount read  :1;              // 0 EntitiyAttributes  1 << 0
  SmartCount write :1;             // 1 EntitiyAttributes  1 << 1
  SmartCount isOpen:1;            // 2 EntitiyAttributes  1 << 2
} __attribute__((packed));

#define FLAG_LVALUE_IS_HOST                        FLAG_BIT(0)

struct LValueFlags {
  SmartCount isHost:1;            // 0 LValue             1 << 0
} __attribute__((packed));


#define FLAG_LIST_ELEMENT_HAS_BRACKETS             FLAG_BIT(0)

struct ListFlags {
  SmartCount hasBrackets:1;       // List<Element>, исключая List<Constraint>
} __attribute__((packed));


#define FLAG_VENTITIES_INJECTIVE                   FLAG_BIT(0)
#define FLAG_VENTITIES_OVERLOAD_RESOLVING_ENTERED  FLAG_BIT(1)

struct VEntitiesFlags {
  SmartCount injective:1;        // VEntities, исключая List<Constraint>
  SmartCount ventitiesResolve:1; // VEntities, исключая List<Constraint>
} __attribute__((packed));

// Биты 0-3 нельзя трогать для наследников ResolvedEntity
#define FLAG_RESOLVED_ENTITY_IS_SYSTEM             FLAG_BIT(0)
#define FLAG_RESOLVED_ENTITY_BRACES_NOT_OUTPUT     FLAG_BIT(1)
#define FLAG_RESOLVED_ENTITY_IS_SYSTEM_TEMPLATE    FLAG_BIT(2)
#define FLAG_RESOLVED_ENTITY_IS_SYS_UNSUPPORTED    FLAG_BIT(3)
#define FLAG_RESOLVED_ENTITY_IS_DYNAMIC_USING      FLAG_BIT(4)
#define FLAG_RESOLVED_ENTITY_LAST                  4

#define __RESOLVED_ENTITY_FLAGS__ \
      SmartCount isSystem:1;          /* ResolvedEntity     1 << 0 */ \
      SmartCount bracesNotOutput:1;   /* ResolvedEntity     1 << 1 */ \
      SmartCount isSystemTemplate:1;  /* ResolvedEntity     1 << 2 */ \
      SmartCount isSysUnsupported:1;  /* ResolvedEntity     1 << 3 */ \
      SmartCount isDynamicUsing:1;    /* ResolvedEntity     1 << 4 */ \

#define FLAG_FUNCTION_CALL_ARGUMENT_IS_SELF        FLAG_BIT(FLAG_RESOLVED_ENTITY_LAST + 1)

struct FunctionCallArgumentFlags {
  __RESOLVED_ENTITY_FLAGS__
  SmartCount isSelf:1;                /* FunCallArg         1 << 5 */
} __attribute__((packed));


#define FLAG_PL_EXPR_IS_NOT                        FLAG_BIT(FLAG_RESOLVED_ENTITY_LAST + 1)
#define FLAG_PL_EXPR_IS_DISTINCT                   FLAG_BIT(FLAG_RESOLVED_ENTITY_LAST + 2)
#define FLAG_PL_EXPR_IS_WHERE_EXPR                 FLAG_BIT(FLAG_RESOLVED_ENTITY_LAST + 3)
#define FLAG_PL_EXPR_LAST                          FLAG_RESOLVED_ENTITY_LAST + 3

#define __PL_EXPR_FLAGS__ \
      SmartCount isNot:1;             /* PlExpr             1 << 5 */ \
      SmartCount isDistinct:1;        /* PlExpr             1 << 6 */ \
      SmartCount isWhereExpr:1;       /* PlExpr             1 << 7 */ \

struct PlExprFlags {
  __RESOLVED_ENTITY_FLAGS__
  __PL_EXPR_FLAGS__
} __attribute__((packed));

#define FLAG_NUMERIC_VALUE_IS_STRING_LITERAL       FLAG_BIT(FLAG_PL_EXPR_LAST + 1)

struct NumericValueFlags {
  __RESOLVED_ENTITY_FLAGS__
  __PL_EXPR_FLAGS__
  SmartCount isStringLiteral:1;       /* NumericValue       1 << 8 */
} __attribute__((packed));

#define FLAG_LOGICAL_COMPOUND_IS_OR                FLAG_BIT(FLAG_PL_EXPR_LAST + 1)


struct LogicalCompoundFlags {
  __RESOLVED_ENTITY_FLAGS__
  __PL_EXPR_FLAGS__
  SmartCount isOr:1;                  /*  LogicalCompound   1 << 8 */
} __attribute__((packed));



struct SemanticTreeFlags {
  SmartCount openCount       :4;
  SmartCount isList          :1;
  SmartCount isPlContext     :1;
  SmartCount childsResolved  :1;
  SmartCount isAlias         :1;
  SmartCount isBaseForAlias  :1;
  SmartCount isNotPlContext  :1;
  SmartCount isFromNode      :1;
  SmartCount isIntoNode      :1;
  SmartCount isInsertingValue:1;
} __attribute__((packed));

#define FLAG_ENTITY_IS_NOT_STREE_OWNER        FLAG_SMART_BIT15


union Attributes {
  EntityAttributesFlags     entityAttributes;
  PlExprFlags               plExprFlags;
  NumericValueFlags         numericValueFlags;
  LogicalCompoundFlags      logicalCompoundFlags;
  FunctionCallArgumentFlags functionCallArgumentFlags;
  ListFlags                 listFlags;
  VEntitiesFlags            vEntitiesFlags;
  LValueFlags               lValueFlags;
  SemanticTreeFlags         semanticTreeFlags;
  SmartCount v;

  inline void setWhereExpr() { v |= FLAG_PL_EXPR_IS_WHERE_EXPR; }
  inline bool isWhereExpr()  const { return v & FLAG_PL_EXPR_IS_WHERE_EXPR; }
};

}

template<typename Ret, typename T, template<class> class Ptr>
class mem_fun_Ptr_t : public std::unary_function<T, Ret> {
public:
  explicit mem_fun_Ptr_t(Ret (T::*pmf)()) : mf(pmf) {}
  Ret operator()(Ptr<T> &instance) const { return (instance.object()->*mf)(); }
private:
  Ret (T::*mf)();
};

template<typename Ret, typename T, template<class> class Ptr>
inline mem_fun_Ptr_t<Ret, T, Ptr> mem_fun_ptr(Ret (T::*pmf)()) { return mem_fun_Ptr_t<Ret, T, Ptr>(pmf); }



}

#endif // SMARTPTR_UNION

