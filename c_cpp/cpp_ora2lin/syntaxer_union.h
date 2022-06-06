#ifndef SYNTAXER_UNION_H
#define SYNTAXER_UNION_H

#include <string.h>
#include "model_head.h"
#include "semantic_flags.h"

namespace cl {

union semantic_type {

  struct PhysicalTableProps {
    Sm::table::field_property::PhysicalProperties *physical;
    Sm::table::TableProperties                    *tableProp;
  };

  bool boolval;
  int intval;
  unsigned int uintval;


  struct PhysicalTableProps *physicalTableProps;

  Sm::DynamicFuncallTranslator *dynamicFuncallTranslator;
  Sm::ConstructBlockStmt  *constructBlockStmt;
  Sm::ConstructExprStmt   *constructExprStmt;
  Sm::QueryEntityDyn      *queryEntityDyn;
  Sm::QueryTailData       *queryTailData;
  Sm::DblinkUserAuthentication          *userAuthentication;
  Sm::QueryHint           *optimizeHint;
  Sm::List<Sm::QueryHint> *optimizeHints;

  Sm::ConstructExprStmtContext *constructExprStmtContext;

  Sm::LoopBounds *loopBounds;
  Sm::AbstractPy::Node *abstractPyNode;
  Sm::AbstractPy::List *abstractPyList;
  Sm::AbstractPy::Dict *abstractPyDict;

  Sm::ExtractedEntity extractedEntityKeyword;
  Sm::time_expr::TimestampCathegory timestampCathegory;
  Sm::Type::auth_id::AuthID                                                   authId;
  Sm::function_argument::Direction                                            argumentDirection;
  Sm::EnableState ::T                                                         enableState;
  Sm::lock_table  ::LockMode                                                  lockMode;
  Sm::lock_table  ::WaitMode                                                  waitMode;
  Sm::table       ::OnCommitRowsAction                                        onCommitRows;
  Sm::transaction ::TransactionType                                           transactionType;
  Sm::trigger     ::TriggerMode                                               trigger_Mode;
  Sm::Type        ::Inheritance::T                                            inheritanceCathegory;
  Sm::Type        ::java_external_spec::Module                                javaExternalModule;
  Sm::Type        ::member_function::Specificators                            specificators;
  Sm::alter_user  ::connect::GrantOrRevoke                                    grantOrRevoke;
  Sm::constraint  ::referenced_key::OnDelete                                  constraintOnDelete;
  Sm::insert      ::conditional_insert::AllOrFirst                            allOrFirst;
  Sm::              comparsion_list::ComparsionOp                             comparsionListOp;
  Sm::              ComparsionOp::t                                           plComparsionOp;
  Sm::              like_cathegory::t                                         likeCathegory;
  Sm::              logical_compound::AndOr                                   andOr;
  Sm::              QuantorOp::t                                              quantorOp;
  Sm::pragma      ::PragmaRestrictFlags                                       restrictFlags;
  Sm::algebraic_compound::t                                                   sqlCompound;
  Sm::cursor_properties::Properties                                           refclass;
  Sm::sql_union::UnionOperation                                               subqueryUnionOp;
  Sm::flashback_query::ScnOrTimestamp                                         scnOrTimestamp;
  Sm::JoinQueries::Operation                                                         joinOp;
  Sm::query_block::QueryPrefix                                                queryPrefix;
  Sm::table       ::enable_disable_constraint::KeepDropState                  keepDropState;
  Sm::table       ::enable_disable_constraint::ValidateState                  validateState;
  Sm::table       ::table_properties::CachingState                            cachingState;
  Sm::table       ::table_properties::RowDependenciesState                    rowDependenciesState;
  Sm::table       ::field_property::nested_table::LocatorOrValue              locatorOrValue;
  Sm::trigger     ::NonDmlEvent                                               trigger_NonDmlFlags;
  Sm::Label                                                       *label;

  Sm::List<Sm::NumericValue>                                      *numericValueList;
  Sm::List<Sm::MergeFieldAssignment>                              *fieldAssignmentList;
  Sm::List<Sm::ArgumentNameRef>                                   *argumentNames;
  Sm::List<Sm::Constraint>                                        *constraintList;
  Sm::List<Sm::CursorParameter>                                   *cursorParameters;
  Sm::List<Sm::Declaration>                                       *declarations;
  Sm::Vector<Sm::FunctionArgument>                                *functionArgumentList;
  Sm::List<Sm::Id2>                                               *id2List;
  Sm::List<Sm::Id>                                                *idList;
  Sm::List<Sm::Label>                                             *labelList;
  Sm::List<Sm::IdEntitySmart>                                     *entityList;
  Sm::List<Sm::List<Sm::SqlExpr> >                                *sqlExprListList;
  Sm::List<Sm::SqlExpr>                                           *sqlExprList;
  Sm::List<Sm::PlExpr>                                            *plExprList;
  Sm::BaseList<Sm::StatementInterface>                            *statementsList;
  Sm::BaseList<Sm::Type::MemberInterface>                         *objectMemberList;
  Sm::List<Sm::Type::RecordField>                                 *recordFieldDecls;
  Sm::BaseList<Sm::StatementInterface>                            *whenExprList;
  Sm::List<Sm::alter_table::AlterFieldsBase>                      *manipulateFields;
  Sm::List<Sm::alter_user::UserRole>                              *alterUser_UserRoleList;
  Sm::List<Sm::insert::InsertValues>                              *insert_InsertValuesList;
  Sm::List<Sm::insert::conditional_insert::InsertWhenThen>        *insertWhenThenList;
  Sm::List<Sm::pl_expr::BracketedPlExprList>                      *bracketedPlExprListList;
  Sm::List<Sm::pl_expr::OfType>                                   *ofTypes;
  Sm::List<Sm::CaseIfThen>                              *ifThenList;
  Sm::Vector<Sm::FunCallArg>                            *functionCallArglist;
  Sm::List<Sm::RefAbstract>                               *sqlExprIdList;
  Sm::List<Sm::FactoringItem>                 *factoringList;
  Sm::List<Sm::From>                          *fromBody;
  Sm::List<Sm::GroupBy>                       *groupByList;
  Sm::List<Sm::GroupingSetsClause>                  *groupingSetsList;
  Sm::List<Sm::OrderByItem>                   *orderByItems;
  Sm::Vector<Sm::SelectedField>                 *selectedFields;
  Sm::List<Sm::table::EnableDisableConstraint>                    *table_EnableDisableConstraintList;
  Sm::List<Sm::ParsingStageTableField>                            *table_FieldDefinitionList;
  Sm::List<Sm::table::field_property::FieldProperty>              *tableFieldProperties;
  Sm::List<Sm::trigger::DmlReferencing>                           *triggerReferencingList;
  Sm::List<Sm::PlExpr>                                            *updateSetItemList;
  Sm::List<Sm::view::ViewConstraint>                              *view_ViewConstraints;

  Sm::List<Sm::FromSingle> *fromSingleList;

  Sm::DatabaseLinkBody                                            *linkBody;
  Sm::DatabaseLink                                                *databaseLink;
  Sm::MergeUpdate                                                 *mergeMatchedUpdate;
  Sm::MergeInsert                                                 *mergeNotMatchedInsert;
  Sm::MergeFieldAssignment                                        *fieldAssignment;
  Sm::Merge                                                       *merge;
  Sm::ExtractExpr                             *extractFromExpression;
  Sm::TrimFromExpr                                *trimFromExpression;
  Sm::FromJoin                                *joinClauseTail;
  Sm::IdEntityChain                                               *idEntity;
  Sm::IdEntitySmart                                               *idEntitySmart;
  Sm::AlterTable                                                  *alterTable;
  Sm::AlterUser                                                   *alterUser;
  Sm::ArgumentNameRef                                             *argumentName;
  Sm::Assignment                                                  *assignment;
  Sm::LValue                                                      *assignmentLValue;
  Sm::BlockPlSql                                                  *blockPlSql;
  Sm::CaseStatement                                               *caseStatement;
  Sm::Close                                                       *close_stmt;
  Sm::Commit                                                      *commit;
  Sm::Constraint                                                  *constraint;
  Sm::Cursor                                                      *cursorDecl;
  Sm::CursorParameter                                             *cursorParameter;
  Sm::Datatype                                                    *datatype;
  Sm::Declaration                                                 *declaration;
  Sm::DeleteFrom                                                  *deleteFrom;
  Sm::ExecuteImmediate                                            *executeImmediate;
  Sm::Exit                                                        *exit_stmt;
  Sm::Fetch                                                       *fetch;
  Sm::ForAll                                                      *forAll;
  Sm::ForOfExpression                                             *forOfExpression;
  Sm::ForOfRange                                                  *forOfRange;
  Sm::Function                                                    *function;
  Sm::FunctionArgument                                            *functionArgument;
  Sm::Goto                                                        *goto_stmt;
  Sm::Id                                                          *id;
  Sm::Id2                                                         *id2;
  Sm::If                                                          *if_stmt;
  Sm::Index                                                       *index;
  Sm::Insert                                                      *insert;
  Sm::LockTable                                                   *lockTable;
  Sm::Loop                                                        *loop;
  Sm::NumericValue                                                *numericValue;
  Sm::OpenCursor                                                  *openCursor;
  Sm::OpenFor                                                     *openFor;
  Sm::Package                                                     *package;
  Sm::PipeRow                                                     *pipeRow;
  Sm::PlExpr                                                      *plCond;
  Sm::Return                                                      *return_stmt;
  Sm::ReturnInto                                                  *returnInto;
  Sm::Rollback                                                    *rollback;
  Sm::Savepoint                                                   *savepoint;
  Sm::Sequence                                                    *sequence;
  Sm::SequenceBody                                                *sequenceBody;
  Sm::ChangedQueryEntity                                                   *sqlEntity;
  Sm::SqlExpr                                                     *sqlExpr;
  Sm::RefExpr                                                     *refExpr;
  Sm::SqlStatementInterface                                       *sqlStatement;
  Sm::StatementInterface                                          *statement;
  Sm::StatementWithLabelPrefixedList                              *statementWithLabel;
  Sm::Synonym                                                     *synonym;
  Sm::Table                                                       *table;
  Sm::Transaction                                                 *transaction;
  Sm::Trigger                                                     *trigger;
  Sm::Update                                                      *update;
  Sm::View                                                        *view;
  Sm::WhenExpr                                                    *whenExpr;
  Sm::WhereClause                                                 *whereClause;
  Sm::While                                                       *while_stmt;
  Sm::Type       ::MemberVariable                                 *type_MemberVariable;
  Sm::Type       ::ObjectType                                     *type_Object;
  Sm::Type       ::Object                                         *type_ObjectConcrete;
  Sm::Type       ::Varray                                         *type_Varray;
  Sm::Type       ::JavaExternalSpec                               *javaExternalSpec;
  Sm::Type       ::MemberInterface                                *objectMember;
  Sm::Type       ::RecordField                                    *recordFieldDecl;
  Sm::alter_table::AddFields                                      *alterTable_AddFields;
  Sm::AlterTableCommand                                           *alterConstraintBase;
  Sm::alter_table::AlterFieldsBase                                *alterTable_AlterFieldsBase;
  Sm::AlterTableCommand                                           *alterTableCommand;
  Sm::alter_table::DropFields                                     *alterTable_DropFields;
  Sm::alter_table::KeyFields                                      *alterTable_KeyFields;
  Sm::alter_table::ModifyFields                                   *alterTable_ModifyFields;
  Sm::alter_table::RenameField                                    *alterTable_RenameField;
  Sm::alter_user ::UserProxyConnect                               *alterUser_UserProxyConnect;
  Sm::alter_user ::UserRole                                       *alterUser_UserRole;
  Sm::alter_user ::UserRoles                                      *alterUser_UserRoles;
  Sm::alter_user ::UserSettings                                   *alterUser_UserSettings;
  Sm::constraint ::Attribute                                      *constraint_Attribute;
  Sm::constraint ::CheckCondition                                 *constraint_CheckCondition;
  Sm::constraint ::ConstraintState                                *constraint_State;
  Sm::constraint ::ForeignKey                                     *constraint_ForeignKey;
  Sm::constraint ::PrimaryKey                                     *constraint_PrimaryKey;
  Sm::constraint ::ForeignReference                                  *constraint_ReferencedKey;
  Sm::constraint ::Unique                                         *constraint_Unique;
  Sm::insert     ::InsertValues                                   *insert_InsertValues;
  Sm::insert     ::Into                                           *insert_Into;
  Sm::insert     ::SingleInsert                                   *insert_SingleInsert;
  Sm::insert     ::InsertFrom                                     *insertFrom;
  Sm::insert     ::InsertingValues                                *insertingValues;
  Sm::insert     ::conditional_insert::InsertConditional          *insertConditional;
  Sm::insert     ::conditional_insert::InsertWhenThen             *insertWhenThen;
  Sm::pl_expr    ::Submultiset                                    *submultiset;
  Sm::pl_expr    ::BracketedPlExprList                            *bracketedPlExprList;
  Sm::pl_expr    ::Like                                           *likeCond;
  Sm::pl_expr    ::OfType                                         *ofType;
  Sm::pragma     ::Pragma                                         *pragma_Pragma;
  Sm::Subquery                                       *subquery;
  Sm::FunctionDynField                               *functionDynField;
  Sm::CaseIfThen                                     *ifThen;
  Sm::FunCallArg                           *functionCallArgument;
  Sm::RefAbstract                                      *sqlExprId;
  Sm::TimeExprInterval                               *interval;
  Sm::TimeExprTimezone                               *timezone;
  Sm::FactoringItem                        *factoringItem;
  Sm::FlashbackQueryClause                 *flashbackQueryClause;
  Sm::ForUpdateClause                            *forUpdate;
  Sm::From                                 *fromBodyItem;
  Sm::FromSingle                           *fromSingle;
  Sm::GroupBy                              *groupBy;
  Sm::GroupingSetsClause                         *groupingSets;
  Sm::HierarhicalClause                     *hierarhicalQueryClause;
  Sm::Join                                 *joinCondition;
  Sm::OrderBy                              *orderBy;
  Sm::OrderByItem                          *orderByItem;
  Sm::QueriedTable                         *queriedTable;
  Sm::QueryBlock                           *queryBlock;
  Sm::SelectList                           *selectList;
  Sm::SelectedField                        *selectedField;
  Sm::Tablesample                          *tablesample;
  Sm::table      ::EnableDisableConstraint                        *table_EnableDisableConstraint;
  Sm::table      ::EncryptSpec                                    *table_EncryptSpec;
  Sm::             ParsingStageTableField                         *table_FieldDefinition;
  Sm::table      ::OidIndex                                       *table_OidIndex;
  Sm::table      ::SubstitutableProperty                          *table_SubstitutableProperty;
  Sm::table      ::TableProperties                                *table_TableProperties;
  Sm::table      ::field_property::FieldProperty                  *tableFieldProperty;
  Sm::table      ::field_property::NestedName                     *nestedName;
  Sm::table      ::field_property::ObjectProperties               *table_ObjectProperties;
  Sm::table      ::field_property::PhysicalProperties             *table_PhysicalProperties;
  Sm::table      ::field_property::VarrayField                    *tableFieldProperty_VarrayField;
  Sm::trigger    ::DmlEvents                                      *trigger_DmlEvents;
  Sm::trigger    ::DmlEvent                                       *dmlTriggerEvent;
  Sm::trigger    ::DmlReferencing                                 *triggerReferencing;
  Sm::trigger    ::Funcall                                        *trigger_Funcall;
  Sm::trigger    ::TriggerActionInterface                         *triggerAction;
  Sm::trigger    ::TriggerEvents                                  *trigger_Events;
  Sm::update     ::SetClause                                      *updateSetClause;
  Sm::view       ::ViewConstraint                                 *view_ViewConstraint;
  Sm::view       ::ViewProperties                                 *view_ViewProperties;
  Sm::view       ::ViewQRestriction                               *view_ViewQRestriction;
  Sm::view       ::XmlSchemaId                                    *xmlReference;
  Sm::IntoCollections                                             *intoCollectionList;


};

}

#endif // SYNTAXER_UNION_H
