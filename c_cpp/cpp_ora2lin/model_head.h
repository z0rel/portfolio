#ifndef MODEL_HEAD_H
#define MODEL_HEAD_H

#include <list>
#include <map>
#include <set>
#include <tuple>
#include <vector>
#include "smartptr.h"

class SyntaxerContext;
class UserContext;

namespace Sm {
  class DynamicFuncallTranslator;
  class ConstructBlockStmt;
  class ConstructExprStmt;
  class QueryHint;
  class DynTailExpr;
  class Label;
  class PlExpr;
  class IdEntityChain;
  class IdEntitySmart;
  class AlterTable;
  class AlterUser;
  class ArgumentNameRef;
  class Assignment;
  class LValue;
  class BooleanLiteral;
  class BlockPlSql;
  class CaseStatement;
  class Close;
  class Commit;
  class Cursor;
  class CursorParameter;
  class Datatype;
  class Declaration;
  class DeleteFrom;
  class ExecuteImmediate;
  class Exit;
  class Fetch;
  class ForAll;
  class ForOfExpression;
  class ForOfRange;
  class Function;
  class QueryTailData;
  class FunctionArgument;
  class Goto;
  class Id;
  class Id2;
  class If;
  class Index;
  class Insert;
  class LockTable;
  class Loop;
  class NumericValue;
  class OpenCursor;
  class OpenFor;
  class Package;
  class PipeRow;
  class PlExpr;
  class Raise;
  class Return;
  class ReturnInto;
  class Rollback;
  class Savepoint;
  class Sequence;
  class SequenceBody;
  class ChangedQueryEntity;
  class SqlExpr;
  class SqlStatementInterface;
  class StatementInterface;
  class StatementWithLabelPrefixedList;
  class Synonym;
  class Table;
  class Transaction;
  class Trigger;
  class Update;
  class View;
  class WhenExpr;
  class WhereClause;
  class While;
  class ArgumentNameRef;
  class Constraint;
  class CursorParameter;
  class Declaration;
  class Id2;
  class Id;
  class SqlExpr;
  class PlExpr;
  class StatementInterface;
  class WhenExpr;
  class Merge;
  class AlterTableCommand;
  class DatabaseLink;
  class DatabaseLinkBody;
  union DatatypeFlags;
  struct DblinkUserAuthentication;
  template <typename T> class List;
  template <typename T> class BaseList;
  template <typename T> class Vector;
  namespace AbstractPy {
    class List;
    class Dict;
  }

  class LoopBounds;
  class FunctionDynField;
  class ConstructExprStmtContext;
  class GroupBy;
  class HierarhicalClause;
  class FactoringItem;
  class GroupBy;
  class GroupingSetsClause;
  class SelectedField;
  class FromJoin;
  class FactoringItem;
  class FlashbackQueryClause;
  class ForUpdateClause;
  class From;
  class FromSingle;
  class GroupBy;
  class GroupingSetsClause;
  class HierarhicalClause;
  class Join;
  class OrderBy;
  class OrderByItem;
  class QueriedTable;
  class QueryBlock;
  class SelectList;
  class SelectedField;
  class Tablesample;
  class RefAbstract;
  class ExtractExpr;
  class TrimFromExpr;
  class RefExpr;
  class Subquery;
  struct FunCallArg;
  class RefAbstract;
  class CaseIfThen;
  class TimeExprInterval;
  class TimeExprTimezone;
  class IntoCollections;

  class Constraint;

  namespace table {
    class TableProperties;
    class EnableDisableConstraint;
    class EncryptSpec;
    class FieldDefinition;
    class OidIndex;
    class SubstitutableProperty;
    class TableProperties;
    class EnableDisableConstraint;
    class FieldDefinition;
    namespace field_property {
      class PhysicalProperties;
      class FieldProperty;
      class NestedName;
      class ObjectProperties;
      class PhysicalProperties;
      class VarrayField;
      class FieldProperty;
    }
  }

  class ParsingStageTableField;

  namespace Type {
    class Object;
    class MemberVariable;
    class ObjectType;
    class Varray;
    class JavaExternalSpec;
    class MemberInterface;
    class RecordField;
    class CollectionType;
    class Record;
    class RefCursor;
    namespace collection_methods {
      class CollectionMethod;
    }
  }
  namespace alter_table {
    class AddFields;
    class AlterFieldsBase;
    class DropFields;
    class KeyFields;
    class ModifyFields;
    class RenameField;
  }
  class MergeFieldAssignment;
  class MergeUpdate;
  class MergeInsert;
  namespace alter_user {
    class UserRole;
    class UserProxyConnect;
    class UserRole;
    class UserRoles;
    class UserSettings;
  }
  namespace constraint {
    class Attribute;
    class CheckCondition;
    class ConstraintState;
    class ForeignKey;
    class PrimaryKey;
    class ForeignReference;
    class Unique;
  }
  namespace insert {
    class InsertValues;
    class InsertValues;
    class Into;
    class SingleInsert;
    class InsertFrom;
    class InsertingValues;
  }
  namespace pl_expr {
    class Submultiset;
    class Like;
    class OfType;
    class BracketedPlExprList;
  }
  namespace trigger {
    class DmlEvents;
    class Funcall;
    class TriggerActionInterface;
    class TriggerEvents;
  }
  namespace update {
    class SetClause;
  }
  namespace view {
    class ViewConstraint;
    class ViewProperties;
    class ViewQRestriction;
    class XmlSchemaId;
  }
  namespace pragma {
    class Pragma;
  }
  namespace insert {
    namespace conditional_insert {
      class InsertConditional;
      class InsertWhenThen;
    }
  }
  namespace trigger {
    struct DmlEvent;
    struct DmlReferencing;
  }

  typedef std::set<std::pair<Sm::Id*, int> > FunctionCalls;

  class CathegoriesOfDefinitions {
  public:
    enum ScopedEntities  {
      UNRESOLVED__,
      ROOT__,
      EMPTY__,
      PlExpr__,
      LValue_,
      Label_,
      TailObj_,
      ArrayConstructor_,
      QueryHint_,

      Commit_,
      DeleteFrom_,
      LockTable_,
      Rollback_,
      SelectStatement_,
      Update_,
      CursorFieldDecltype_,
      SingleInsert_,
      MultipleValuesInsert_,
      MultipleConditionalInsert_,

      Assignment_,
      Close_,
      ConstructExprStmt_,
      CursorDecltype_,
      DeclNamespace_,
      ExecuteImmediate_,
      Exit_,
      Fetch_,
      FunctionCall_,
      Goto_,
      If_,
      NullStatement_,
      OpenCursor_,
      OpenFor_,
      PipeRow_,
      Raise_,
      Resignal_,
      Return_,
      WhenExpr_,
      While_,

      Comparsion_,
      LogicalCompound_,
      SqlSelectedField_,
      RefExpr_,
      SqlExprNull_,
      SqlExprEmptyId_,
      BooleanLiteral_,
      SqlExprPrior_,
      AnalyticFun_,
      QueriedPseudoField_,
      SqlExprCursor_,
      SqlExprBrackets_,
      SqlExprUnaryPlus_,
      SqlExprUnaryMinus_,
      AlgebraicCompound_,
      NumericSimpleInt_,
      NumericIntVal_,
      SqlExprDefault_,
      TimeExpr_,
      RowId_,
      RowNum_,
      SqlExprCase_,
      SqlExprNewCall_,
      SqlExprOutherJoin_,
      SqlExprAsterisk_,
      AsteriskRefExpr_,
      SqlExprCursorProperties_,
      SqlExprSqlCursorProperties_,
      SqlExprHostCursorProperties_,
      RefHostExpr_,
      SqlBulkRowcount_,
      SqlExprCast_,
      SqlExprCastMultiset_,
      SqlExprSelectSingle_,
      SqlExprSelectBrackets_,
      SqlExprUnion_,
      NumericFloatVal_,
      ExtractFrom_,
      TrimFrom_,
      SequencePseudocolumn_,
      VariableUndeclaredIndex_ ,
      TriggerRowReference_,
      TriggerNestedRowReference_,

      ModelContext_,
      Table_,
      DatabaseLink_,
      View_,
      Sequence_,
      CollectionMethod_,
      Synonym_,
      Package_,
      Function_,
      IndexUnique_,
      Index_,
      Trigger_,
      User_,

      /* Не SQL-сущности */
      ArgVarDeclaration_,
      BlockPlSql_,
      CaseStatement_,
      LinterCursor_,
      LinterCursorField_,
      Cursor_,
      CursorParameter_,
      Exception_,
      ExceptionPragma_,
      FieldOfRecord_,
      FieldOfTable_,
      ForAll_,
      ForOfExpression_,
      ForOfRange_,
      FunctionArgument_,
      FunctionCallArgument_,
      ListConstraint_,
      Loop_,
      MemberFunction_,
      MemberVariable_,
      QueriedTable_,
      RenameTable_,
      Savepoint_,
      SqlEntity_,
      SqlExpr_,
      Transaction_,
      TriggerDmlReferencing_,
      Variable_,
      FieldOfVariable_,
      XmlReference_,

      // объявления типов данных базовых сущностей
      Record_,
      RefCursor_,
      Varray_,
      NestedTable_,
      Merge_,

      // В данных типах может применяться операция неявного приведения
      QueryBlock_,
      Subtype_,
      FundamentalDatatype_,
      Object_,
      AnydataObject_,

      Datatype_,
      DatatypeNull_,
      DatatypeDefault_,
      DatatypeIsRef_,
      DatatypeType_,
      DatatypeRowtype_,


      FactoringItem_,
      FromSingle_,
      FromJoin_,
      TriggerPredicateVariable_,

      StatementsContainer_,

      ReturnInto_,
      SelectList_,

      SpecialKeysActor_,

      LAST_ENTITY_NUMBER
    };

    static std::string ddlCathegoryToString(ScopedEntities cat);
  };


  class ResolvedEntity;

  struct LE_ResolvedEntities {
    static bool lt(const ResolvedEntity *l, const ResolvedEntity *r);
    bool operator() (const ResolvedEntity *l, const ResolvedEntity *r) const;
  };

  typedef std::map<std::string, Sm::ResolvedEntity*> EntitiesIndexedByName;
  typedef std::map<std::string, std::map<CathegoriesOfDefinitions::ScopedEntities, EntitiesIndexedByName > > LinterCreatedEntities;
  typedef std::map<std::string, smart::Ptr<UserContext> > UserDataMap;

  typedef std::set<ResolvedEntity*, LE_ResolvedEntities> EntitiesSet;
  typedef std::map<UserContext*, EntitiesSet, LE_ResolvedEntities> UserEntitiesMap;

  class Codestream;

  typedef Sm::Vector<Sm::FunctionArgument> Arglist;
  class QueryEntityDyn;

  namespace AbstractPy {
    class Node;
  }


  struct CathegoryIndexEnum {
    enum CathegoryIndex { UNIQUE, SIMPLE, BITMAP };

    inline CathegoriesOfDefinitions::ScopedEntities static convertDdl(CathegoryIndex ci) {
      switch (ci) {
        case SIMPLE: return CathegoriesOfDefinitions::Index_;
        case BITMAP: return CathegoriesOfDefinitions::Index_;
        case UNIQUE: return CathegoriesOfDefinitions::IndexUnique_;
      }
      return CathegoriesOfDefinitions::Index_;

    }
  };

  typedef std::map<std::string, std::vector<smart::Ptr<Sm::Subquery> > > GlobalDynSqlCursors;
}

namespace std {

  template<>
  struct hash<Sm::CathegoriesOfDefinitions::ScopedEntities> {
    inline size_t operator()  (Sm::CathegoriesOfDefinitions::ScopedEntities v) const { return (size_t)v; }
  };

}

#endif // MODEL_HEAD_H
