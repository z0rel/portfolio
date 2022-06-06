#ifndef SEMANTIC_FLAGS_H
#define SEMANTIC_FLAGS_H

#include <string>
#include "smartptr.h"



namespace Sm {

class PlExpr;
typedef std::function<bool(PlExpr*)> const ExpressionActor;


#define  FLAG_REPLACE_NEED_TRANSLATION      (1 << 0)
#define  FLAG_REPLACE_TRAVERSE_NEXT         (1 << 1)
#define  FLAG_REPLACE_SKIP_SECOND_CONDITION (1 << 2)
#define  FLAG_REPLACE_SKIP_DEPTH_TRAVERSING  0
#define  FLAG_REPLACE_NEED_TRANSLATION_AND_TRAVERSE (FLAG_REPLACE_NEED_TRANSLATION | FLAG_REPLACE_TRAVERSE_NEXT)
#define  FLAG_REPLACE_TRAVERSE_ONLY  (FLAG_REPLACE_TRAVERSE_NEXT | FLAG_REPLACE_SKIP_SECOND_CONDITION)

class StatementInterface;

class ExprTR {
public:
  typedef std::function<PlExpr*(PlExpr*)> const Tr;
  typedef std::function<int(PlExpr*, ExprTR&, bool construct)> const Cond;

  ExprTR(Tr _tr, Cond _trCond)
    : tr(_tr), cond(_trCond) {}

  Tr   tr;
  Cond cond;

  virtual void preaction(StatementInterface *) {}
  virtual void postaction(Sm::StatementInterface *) {}
};

typedef ExprTR& ExprTr;

class Declaration;
typedef std::function<bool(Declaration*)> const DeclActor;

#define FLAG_DATATYPE_IS_REF            (1 << 0)
#define FLAG_DATATYPE_IS_NULL           (1 << 1)
#define FLAG_DATATYPE_IS_DEFAULT        (1 << 2)
#define FLAG_DATATYPE_AS_CHAR_LENGTH    (1 << 3)
#define FLAG_DATATYPE_AS_BYTE_LENGTH    (1 << 4)
#define FLAG_DATATYPE_IS_TYPE_OF        (1 << 5)
#define FLAG_DATATYPE_IS_ROWTYPE_OF     (1 << 6)
#define FLAG_DATATYPE_IS_EMPTY_VARCHAR  (1 << 7)
#define FLAG_DATATYPE_IS_SPEC_REFERENCE (1 << 8)

  union DatatypeFlags {
    struct Flags {
      unsigned int isRef         :1; // FLAG_DATATYPE_IS_REF         1 << 0
      unsigned int isNull        :1; // FLAG_DATATYPE_IS_NULL        1 << 1
      unsigned int isDefault     :1; // FLAG_DATATYPE_IS_DEFAULT     1 << 2
      unsigned int asCharLength  :1; // FLAG_DATATYPE_AS_CHAR_LENGTH 1 << 3
      unsigned int asByteLength  :1; // FLAG_DATATYPE_AS_BYTE_LENGTH 1 << 4
      unsigned int isTypeOf      :1; // FLAG_DATATYPE_IS_TYPE_OF     1 << 5
      unsigned int isRowtypeOf   :1; // FLAG_DATATYPE_IS_ROWTYPE_OF  1 << 6
      unsigned int isEmptyVarchar:1; // FLAG_DATATYPE_IS_EMPTY_VARCHAR 1 << 7

    } __attribute__((packed));
    Flags a;
    unsigned int v;

    DatatypeFlags() : v(0) {}
    DatatypeFlags(const DatatypeFlags &o) : v(o.v) {}
    inline void concat(const DatatypeFlags o) { v |= o.v; }
  };
  namespace lock_table {
    enum LockMode { ROW_SHARE, ROW_EXCLUSIVE, SHARE_UPDATE, SHARE, SHARE_ROW_EXCLUSIVE, EXCLUSIVE };
    enum WaitMode { EMPTY, WAIT, NOWAIT };
  }
  enum class ExtractedEntity : int { DAY, YEAR, MONTH, SECOND, HOUR, MINUTE, TIMEZONE_HOUR, TIMEZONE_MINUTE, TIMEZONE_REGION, TIMEZONE_ABBR };
  namespace sql_union {
    enum UnionOperation { SIMPLE_UNION, UNION_ALL, INTERSECT, MINUS };
  };
  namespace JoinQueries {
    enum Operation {
        EMPTY  = 0,

        /* 0 1 */
        OUTHER = 1,
        INNER  = 2,
        CROSS  = 3,

        RIGHT  = OUTHER | 1 << 3,
        LEFT   = OUTHER | 1 << 4,
        FULL   = OUTHER | RIGHT | LEFT,

        NATURAL       = 1 << 5,
        NATURAL_INNER = INNER | NATURAL,
        NATURAL_CROSS = CROSS | NATURAL,
        NATURAL_FULL  = FULL  | NATURAL,
        NATURAL_RIGHT = RIGHT | NATURAL,
        NATURAL_LEFT  = LEFT  | NATURAL
     };
  }
  namespace query_block {
    enum QueryPrefix { EMPTY, DISTINCT, UNIQUE, ALL } ;
  }
  namespace flashback_query {
    enum ScnOrTimestamp { SCN, TIMESTAMP, EMPTY };
  }

  namespace cursor_properties {
    enum Properties { CURSOR_FOUND, CURSOR_ISOPEN, CURSOR_NOTFOUND, CURSOR_ROWCOUNT };
  }
  namespace algebraic_compound {
    enum t { MULTIPLE, DIVIDE, PLUS, MINUS, CONCAT, DEGREE, MOD };
  }
  namespace time_expr {
    enum TimestampCathegory { TIMESTAMP_EMPTY, TIMESTAMP_WITH_TIME_ZONE, TIMESTAMP_WITH_LOCAL_TIME_ZONE };
  }

  namespace table {
    namespace table_properties {
      enum CachingState         { C_EMPTY, CACHE, NOCACHE };
      enum RowDependenciesState { R_EMPTY, ROW_DEPENDENCIES, NO_ROW_DEPENDENCIES };
    }
    namespace field_property {
      namespace nested_table {
        enum LocatorOrValue { EMPTY, LOCATOR, VALUE };
      }
    }
    namespace enable_disable_constraint {
      enum EnableState     { E_EMPTY, ENABLE  , DISABLE    };
      enum KeepDropState   { K_EMPTY, KEEP    , DROP       };
      enum ValidateState   { V_EMPTY, VALIDATE, NOVALIDATE };
    }
    enum OnCommitRowsAction { EMPTY, ON_COMMIT_DELETE_ROWS, ON_COMMIT_PRESERVE_ROWS };
  }
  namespace Type {
    namespace Inheritance {
      enum T {
        EMPTY              = 0,
        NOT                = 1 << 10,
        INSTANTIABLE       = 1, NOT_INSTANTIABLE = INSTANTIABLE | NOT,
        FINAL              = 2, NOT_FINAL        = FINAL        | NOT,
        OVERRIDING         = 2, NOT_OVERRIDING   = OVERRIDING   | NOT,
        FINAL_INSTANTIABLE = FINAL | INSTANTIABLE
      };
    };
    namespace member_function {
      enum Specificators   {
        STATIC       = 1 << 0,
        MEMBER       = 1 << 1,
        MAP          = 1 << 2,
        ORDER        = 1 << 3,
        MAP_MEMBER   = MAP   | MEMBER,
        ORDER_MEMBER = ORDER | MEMBER,
        CONSTRUCTOR  = 1 << 4
      };
    }
    namespace java_external_spec {
      enum Module { SQLData, CustomDatum, OraData };
    }
    namespace auth_id {
      enum AuthID { EMPTY, CURRENT_USER, DEFINER };
    }
  }
  namespace constraint {
    namespace referenced_key {
      enum OnDelete { CASCADE, SET_NULL, EMPTY };
    }

  }
  // namespace pl_expr {
    namespace QuantorOp {
      enum t { EMPTY, ANY, SOME, ALL };
      inline std::string toString(t val) {
        switch (val) {
          case EMPTY: return "";
          case ANY  : return "ANY";
          case SOME : return "SOME";
          case ALL  : return "ALL";
        }
        return "";
      }
    };
    namespace ComparsionOp {
      enum t { EQ, NEQ, IN, LT, GT, LE, GE };
      inline std::string toString(t val) {
        switch (val) {
          case EQ:  return "=";
          case NEQ: return "<>";
          case IN:  return "IN";
          case LT:  return "<";
          case GT:  return ">";
          case LE:  return "<=";
          case GE:  return ">=";
         }
        return "";
      }
      inline std::string toInvertedString(t val) {
        switch (val) {
          case EQ:  return "<>";
          case NEQ: return "=";
          case IN:  return "NOT IN";
          case LT:  return ">=";
          case GT:  return "<=";
          case LE:  return ">";
          case GE:  return "<";
        }
        return "";
      }
    };
    namespace like_cathegory {
      enum t { SIMPLE_LIKE, LIKEC, LIKE2, LIKE4 };
    }
    namespace logical_compound {
      enum AndOr : smart::SmartCount { AND = 0, OR = FLAG_LOGICAL_COMPOUND_IS_OR };
    }
    namespace comparsion_list {
      enum ComparsionOp { IN, EQ, NE };
    }
  // }
  namespace pragma {
    union PragmaRestrictFlags {
      enum Flags {
        EMPTY = 0,
        RNDS  = 1 << 0,
        WNDS  = 1 << 1,
        RNPS  = 1 << 2,
        WNPS  = 1 << 3,
        TRUST = 1 << 4
      };
      Flags f;
      unsigned int i;
    };
  }
  namespace insert {
    namespace conditional_insert {
      enum AllOrFirst { ALL, FIRST, EMPTY };
    }
  }
  namespace trigger {
    namespace non_dml_events {
      enum T {
        EMPTY                   = 0,
        DATABASE_EVENT          = 1 << 30,
        DDL_EVENT               = 1 << 31,

        ALTER                   = 1 <<  1 | DDL_EVENT,
        ANALYZE                 = 1 <<  2 | DDL_EVENT,
        ASSOCIATE_STATISTICS    = 1 <<  3 | DDL_EVENT,
        AUDIT                   = 1 <<  4 | DDL_EVENT,
        COMMENT                 = 1 <<  5 | DDL_EVENT,
        CREATE                  = 1 <<  6 | DDL_EVENT,
        DDL                     = 1 <<  7 | DDL_EVENT,
        DISASSOCIATE_STATISTICS = 1 <<  8 | DDL_EVENT,
        DROP                    = 1 <<  9 | DDL_EVENT,
        GRANT                   = 1 << 10 | DDL_EVENT,
        NOAUDIT                 = 1 << 11 | DDL_EVENT,
        RENAME                  = 1 << 12 | DDL_EVENT,
        REVOKE                  = 1 << 13 | DDL_EVENT,
        TRUNCATE                = 1 << 14 | DDL_EVENT,

        AFTER_DB_ROLE_CHANGE    = 1 << 15 | DATABASE_EVENT,
        AFTER_LOGON             = 1 << 16 | DATABASE_EVENT,
        AFTER_SERVERERROR       = 1 << 17 | DATABASE_EVENT,
        AFTER_STARTUP           = 1 << 18 | DATABASE_EVENT,
        AFTER_SUSPEND           = 1 << 19 | DATABASE_EVENT,
        BEFORE_LOGOFF           = 1 << 20 | DATABASE_EVENT,
        BEFORE_SHUTDOWN         = 1 << 21 | DATABASE_EVENT,
        DATABASE                = 1 << 22 | DATABASE_EVENT,
        SCHEMA                  = 1 << 23 | DATABASE_EVENT
      };
    };
    union NonDmlEvent {
      typedef non_dml_events::T Flags;
      non_dml_events::T f;
      unsigned int i;
      inline std::string toString() {
        switch (f) {
          case non_dml_events::EMPTY                  : return "EMPTY";
          case non_dml_events::DATABASE_EVENT         : return "DATABASE_EVENT";
          case non_dml_events::DDL_EVENT              : return "DDL_EVENT";
          case non_dml_events::ALTER                  : return "ALTER";
          case non_dml_events::ANALYZE                : return "ANALYZE";
          case non_dml_events::ASSOCIATE_STATISTICS   : return "ASSOCIATE_STATISTICS";
          case non_dml_events::AUDIT                  : return "AUDIT";
          case non_dml_events::COMMENT                : return "COMMENT";
          case non_dml_events::CREATE                 : return "CREATE";
          case non_dml_events::DDL                    : return "DDL";
          case non_dml_events::DISASSOCIATE_STATISTICS: return "DISASSOCIATE_STATISTICS";
          case non_dml_events::DROP                   : return "DROP";
          case non_dml_events::GRANT                  : return "GRANT";
          case non_dml_events::NOAUDIT                : return "NOAUDIT";
          case non_dml_events::RENAME                 : return "RENAME";
          case non_dml_events::REVOKE                 : return "REVOKE";
          case non_dml_events::TRUNCATE               : return "TRUNCATE";
          case non_dml_events::AFTER_DB_ROLE_CHANGE   : return "AFTER_DB_ROLE_CHANGE";
          case non_dml_events::AFTER_LOGON            : return "AFTER_LOGON";
          case non_dml_events::AFTER_SERVERERROR      : return "AFTER_SERVERERROR";
          case non_dml_events::AFTER_STARTUP          : return "AFTER_STARTUP";
          case non_dml_events::AFTER_SUSPEND          : return "AFTER_SUSPEND";
          case non_dml_events::BEFORE_LOGOFF          : return "BEFORE_LOGOFF";
          case non_dml_events::BEFORE_SHUTDOWN        : return "BEFORE_SHUTDOWN";
          case non_dml_events::DATABASE               : return "DATABASE";
          case non_dml_events::SCHEMA                 : return "SCHEMA";
        }
        return "";
      }
    };
    enum TriggerMode { M_EMPTY, BEFORE, AFTER,  INSTEAD_OF, FOR };
    inline std::string triggerModeToString(TriggerMode v) {
      switch (v) {
        case M_EMPTY: return "";
        case BEFORE: return "BEFORE";
        case AFTER: return "AFTER";
        case INSTEAD_OF: return "INSTEAD OF";
        case FOR: return "FOR";
      }
      return "";
    }
  }
  namespace EnableState {
    enum T {E_EMPTY, ENABLE, DISABLE};
    inline std::string toSTring(T v) {
      switch (v) {
        case E_EMPTY: return "";
        case ENABLE : return "ENABLE";
        case DISABLE: return "DISABLE";
      }
      return "";
    }

  };
  namespace alter_user {
    namespace connect {
      enum GrantOrRevoke { GRANT, REVOKE };
    }
  }
  namespace transaction {
    enum TransactionType {
      READ_ONLY,
      READ_WRITE,
      ISOLATION_LEVEL_SERIALIZABLE,
      ISOLATION_LEVEL_READ_COMMITED,
      USE_ROLLBACK_SEGMENT
    };
  }
  namespace function_argument {
    enum Direction { IN = 1 << 0, OUT = 1 << 1, IN_OUT = IN | OUT };
  }

template <typename T>
inline T nAssert(T v) {
  if (!v)
    throw 999;
  return v;
}

}


#endif // SEMANTIC_FLAGS_H
