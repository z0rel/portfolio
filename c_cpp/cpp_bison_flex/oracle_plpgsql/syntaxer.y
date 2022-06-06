%{


#include <iostream>

#include <string>
#include <vector>
#include <iomanip>
#include <string>
#include <utility>
#include <stdarg.h>
#include <list>

#define YYDEBUG         1

#include "config_converter.h"
#include "project_optimization.h"
COMPILER_BISON_OPTIMIZATION_PUSH()

#include "syntaxer_context.h"
#include "model_context.h"

#include "semantic_collection.h"
#include "resolvers.h"

#include "semantic_function.h"
#include "semantic_table.h"
#include "semantic_plsql.h"
#include "dynamic_sql_op.h"
#include "semantic_statements.h"
#include "semantic_blockplsql.h"
#include "sql_syntaxer_bison.h"
#include "smart_lexer.h"
#include "sorted_statistic.h"

#include "syntaxer_internal.h"


  /* Функции и объявления {{{1 */

using namespace std;

extern lex_push_state_t lex_push_state ;
extern lex_pop_state_t  lex_pop_state  ;
extern lex_top_state_t  lex_top_state  ;


void yy::parser::error(const cl::location &lloc, const std::string &msg) {
  cout << "=========================== " << endl
       << endl
       << "error: " << lloc << endl << " message " << msg << endl
       << endl
       << "=========================== " << endl;

  context->hasSyntaxerContextErrors = true;
}


  /* end of Функции и объявления 1}}} */

%}
  /* Опции генерации Bison     {{{1 */
/* Отслеживание местоположения.  */
%locations
/* Чистый yylex.  */
/*%define api.pure*/
/* %parse-param {CompilerContext *cntx}  */
/* %parse-param {SyntaxerContext *cntx}*/
%parse-param {SyntaxerContext * context}
%parse-param {yyscan_t yyscanner}
%parse-param {std::string *currentFilename}
%lex-param   {yyscan_t yyscanner}
/* %glr-parser  */
%verbose
%error-verbose
%skeleton "glr.cc"
%glr-parser

%define location_type "cl::location"

%debug

%code requires {
#include <string>
#include "smartptr.h"
#include "lex_location.h"
#include "syntaxer_union.h"
#include "syntaxer_external.h"

#define CL_TO_UINT64_VAL(val) (*((uint64_t*)(&(val))))

#define YYLLOC_DEFAULT(Current, Rhs, N)				        \
    do									\
      if (N)								\
        {								\
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;			\
          (Current).end    = YYRHSLOC (Rhs, N).end;   	                \
        }								\
      else								\
        {								\
          (Current).begin    = (Current).end = (YYRHSLOC (Rhs, 0)).end; \
        }								\
    while (/*CONSTCOND*/ 0)

}

%code provides {

typedef yy::parser::token::yytokentype (*PtrYYLex)(
        cl::semantic_type *yylval_param,
        cl::location      *yylloc_param ,
        yyscan_t yyscanner);

extern PtrYYLex yylex;


}

/* end of Опции генерации Bison 1}}} */

%union {}

 // %destructor { Ptr<smart::Smart> v = $$; (void)v;  } <fieldAssignmentList> <argumentNames> <constraintList> <cursorParameters> <declarations> <functionArgumentList> <id2List> <idList> <entityList> <intoCollectionList> <sqlExprListList> <sqlExprList> <plExprList> <statementsList> <objectMemberList> <recordFieldDecls> <whenExprList> <manipulateFields> <alterUser_UserRoleList> <insert_InsertValuesList> <insertWhenThenList> <bracketedPlExprListList> <ofTypes> <ifThenList> <functionCallArglist> <sqlExprIdList> <fromBody> <groupByList> <groupingSetsList> <orderByItems> <selectedFields> <table_EnableDisableConstraintList> <table_FieldDefinitionList> <tableFieldProperties> <triggerReferencingList> <updateSetItemList> <view_ViewConstraints> <linkBody> <databaseLink> <mergeMatchedUpdate> <mergeNotMatchedInsert> <fieldAssignment> <merge> <extractFromExpression> <trimFromExpression> <joinClauseTail> <alterTable> <alterUser> <argumentName> <assignment> <assignmentLValue> <blockPlSql> <caseStatement> <close_stmt> <commit> <constraint> <cursorDecl> <cursorParameter> <datatype> <declaration> <deleteFrom> <executeImmediate> <exit_stmt> <fetch> <forAll> <forOfExpression> <forOfRange> <function> <functionArgument> <goto_stmt> <id> <id2> <if_stmt> <index> <insert> <lockTable> <loop> <numericValue> <openCursor> <openFor> <package> <pipeRow> <plCond> <return_stmt> <returnInto> <rollback> <savepoint> <sequence> <sequenceBody> <sqlEntity> <sqlExpr> <refExpr> <sqlStatement> <statement> <synonym> <table> <transaction> <trigger> <update> <view> <whenExpr> <whereClause> <while_stmt> <type_MemberVariable> <type_Object> <type_ObjectConcrete> <type_Varray> <javaExternalSpec> <objectMember> <recordFieldDecl> <alterTable_AddFields> <alterConstraintBase> <alterTable_AlterFieldsBase> <alterTableCommand> <alterTable_DropFields> <alterTable_KeyFields> <alterTable_ModifyFields> <alterTable_RenameField> <alterUser_UserProxyConnect> <alterUser_UserRole> <alterUser_UserRoles> <alterUser_UserSettings> <constraint_Attribute> <constraint_CheckCondition> <constraint_State> <constraint_ForeignKey> <constraint_PrimaryKey> <constraint_ReferencedKey> <constraint_Unique> <insert_InsertValues> <insert_Into> <insert_SingleInsert> <insertFrom> <insertingValues> <insertConditional> <insertWhenThen> <submultiset> <bracketedPlExprList> <likeCond> <ofType> <pragma_Pragma> <subquery> <ifThen> <functionCallArgument> <sqlExprId> <interval> <timezone> <flashbackQueryClause> <forUpdate> <fromBodyItem> <fromSingle> <groupBy> <groupingSets> <hierarhicalQueryClause> <joinCondition> <orderBy> <orderByItem> <queriedTable> <queryBlock> <selectList> <selectedField> <tablesample> <table_EnableDisableConstraint> <table_EncryptSpec> <table_FieldDefinition> <table_OidIndex> <table_SubstitutableProperty> <table_TableProperties> <tableFieldProperty> <nestedName> <table_ObjectProperties> <table_PhysicalProperties> <tableFieldProperty_VarrayField> <trigger_DmlEvents> <dmlTriggerEvent> <triggerReferencing> <trigger_Funcall> <triggerAction> <trigger_Events> <updateSetClause> <view_ViewConstraint> <view_ViewProperties> <view_ViewQRestriction> <xmlReference> <idEntitySmart>
 // %destructor { $$->free(); } <idEntity>

  /* Очистка семантического значения и локаций {{{3 */

%initial-action  
{
  @$.initialize();
  memset(&$$, 0, sizeof(YYSTYPE));
};              /* 3}}} */



  /* Токены для классификатора динимического SQL  */
%token cds_BAD_TO_AUTOMATIC_CAST
%token cds_EOF_IN_QUOTED_ID

  /* Токены для конфигурационного файла  {{{ */
%token cfg_EXPORT_SKIPPED_SYNTAX
%token cfg_EXPORT_SKIPPED_SYNTAX_FILE
%token cfg_NEED_TABLE_PROPERTIES
%token cfg_LINTER_RESERVED_KEYWORDS
%token cfg_START_CONFIG_CONSTRUCT
%token cfg_TEMPORARY_PATH
%token cfg_EXPORT_ENTITIES_LOCATION
%token cfg_EXPORT_ENTITIES_LOCATION_FILE

%token cfg_BASE               cfg_CREATE_ALL_MODEL   cfg_CREATE_CHECKS       cfg_CREATE_FOREIGN_KEYS
%token cfg_CREATE_KEYS        cfg_CREATE_OTHERS_KEYS cfg_CREATE_PRIMARY_KEYS cfg_CREATE_PROC
%token cfg_CREATE_SEQUENCES   cfg_CREATE_SYNONYM     cfg_CREATE_TABLES       cfg_CREATE_TRIGGERS
%token cfg_CREATE_UNIQUE_KEYS cfg_CREATE_USERS       cfg_CREATE_VIEWS        cfg_DELETE_FROM_TABLES
%token cfg_DROP_FOREIGN_KEYS  cfg_DROP_INDICES       cfg_DROP_KEYS           cfg_DROP_TABLES
%token cfg_DROP_TRIGGERS      
%token cfg_ERRORSE_FILE       cfg_ERRORS_FILE        cfg_INITIALIZERS_FILE   cfg_JOIN
%token cfg_LINTER_NODENAME    cfg_LINTER_PASSWORD    cfg_LINTER_USERNAME     cfg_OUT_ACTOR_USER
%token cfg_OUT_FILE           cfg_REFERENCES         cfg_SKIP_ERRORS         cfg_SYSDEPS cfg_DESCR_ERRORS_ENTITIES_RESOLVE
%token cfg_TABLES_ROWCOUNT    cfg_TABLES_SIZE        cfg_USER_EXCEPTIONS     cfg_MARK_AS_AUTOGENERATED
%token cfg_ENTITY_FOR_DEPEND_FIND
%token cfg_DEPEND_ENTITIES_LINTER

%token cfg_DUMP_DB            cfg_DUMP_OUT_FILE      cfg_DUMP_SPLIT_FILES 	 cfg_DUMP_DATE_ONLY cfg_DUMP_ENTITY_LIST

%token cfg_CREATE_CODEBLOCK_BRANCH     cfg_CREATE_INITIALIZERS_DEPENDENCIES  cfg_CREATE_PRIMARY_TO_FOREIGN_REFERENCES
%token cfg_CREATE_PUBLIC_SYNONYM       cfg_EXISTED_ENTITY_QUERIES            cfg_FILTER_ENTITIES_BY_LINTER_EXISTS
%token cfg_GENERATE_ALL_ENTITIES       cfg_INITIALIZERS_DEPENDENCIES_FILE    cfg_NOT_FILTER_ENTITIES_IN_DB
%token cfg_NO_REPLACE_STATEMENTS       cfg_PRINT_EXISTED_ENTITIES            cfg_PRINT_EXISTED_LINTER_PROCEDURES
%token cfg_PRINT_EXISTED_LINTER_TABLES cfg_PRINT_EXISTED_LINTER_TRIGGERS     cfg_PRINT_EXISTED_LINTER_VARIABLES
%token cfg_PRINT_EXISTED_LINTER_VIEWS  cfg_REQUEST_ENTITIES_BY_LINTER_EXISTS cfg_START_LINTER_COMMAND
%token cfg_STOP_LINTER_COMMAND         cfg_SUPRESS_UNRESOLVED_PRINTING       cfg_DATE_TO_CHAR_DEFAULT_LENGTH
%token cfg_TRANSLATE_REFERENCES		     cfg_PRINT_CURSOR_VARIABLES            cfg_UNWRAP_STRUCTURED_FIELDS

%token cfg_CREATE_GLOBAL_VARS
%token cfg_CREATE_IN_DB
%token cfg_CREATE_INDICES
%token cfg_CREATE_INITIALIZERS

%token cfg_ADD_SELF
%token cfg_UNION_SELF
%token cfg_MINUS_SELF

%token cfg_CODEGEN_SORT_CMD
%token cfg_CONVERTER_REPOSITORY_PATH
%token cfg_MODEL_SRC_REPOSITORY_PATH
%token cfg_IMPORT
%token cfg_SOURCES
%token cfg_ENABLE_SKIP_TOKENS
%token cfg_DISABLE_SKIP_TOKENS
%token cfg_DIFFGEN_ENABLE

%token cfg_GET_FILENAME
%token cfg_VIRTUAL_ERRORS
%token cfg_EMULATE_ERRORS_IN_CALL_INTERFACE
%token cfg_SORTED_ERROR_LOG_BASE

%token cfg_DEPENDENCY_ANALYZER_OUTFILE  cfg_DEPENDENCY_ANALYZER_OUT_DEP_GRAPH
%token cfg_SKIP_CODEGENERATION

%token cfg_USERS_EXCEPTIONS_LIST
%token cfg_TABLES_SIZES_LIST
%token cfg_TABLES_ROWCOUNT_LIST

%token cfg_DEBUG_TOKEN_LOCATION

%token cfg_DEPENDENCIESSTRUCT_REPR
%token cfg_GENERATE_PYTHON_REPR

%token cfg_MANUAL_ENTITIES
%token cfg_MANUAL_ENTITIES_FILENAME
%token cfg_TRANSFORM_SENTENCE
%token TOK_TRANSFORM_SENTENCE


%token cfg_FULL_RECORDS_REPORT
%token cfg_GENERATE_FULL_STATISTIC
%token cfg_TEMPORARY_SPACER_FILE

%token cfg_CHECK_LEXER_LOCATIONS
%token cfg_CODEGEN_ENTITIES_BYTELOCATION

  /* }}} */



/* Токены - идентификаторы и числа       {{{ */

%token <numericValue> NUMERIC_ID
%token <int>          INTVAL
%token <id>           RawID
%token                EMPTY_ID

%token __STR_TAIL__
%token __BOOL_TAIL__
%token __NUM_TAIL__
%token _DYN_TAIL
%token _DYN_EXPR
%token _DYN_FIELD
%token _DYN_TR_FUN
%token _DYN_TR_CALL_SIGNATURE
%token _DYN_TABLE
%token _DYN_PLSQL_BLOCK
%token __DYN_LEN__
%token _DECLTYPE_EXPR
%token _DECL_NAMESPACE
%token _DYN_SUBQUERY
%token _CONSTRUCT_EXPR
%token _CONSTRUCT_PL_EXPR
%token _DECLTYPE_CURSOR
%token _DYN_WHERE

%token _DYN_BLOCK_TAIL
%token _DYN_PLSQL_STATEMENT


%token CONCAT
%token GLOBAL_CURSOR
%token SAVE_SUBQUERY

  /* end of Токены - идентификаторы и числа }}} */
  /* Ключевые слова-литералы               {{{ */
%token WRAPPED_ENTITY
%token CREATE_USER                       
%token CREATE_PACKAGE                    
%token CREATE_PACKAGE_BODY                    
%token CREATE_SYNONYM                    
%token CREATE_PUBLIC_SYNONYM             
%token CREATE_SEQUENCE                   
%token CREATE_TRIGGER                    
%token ALTER_TRIGGER
%token CREATE_TYPE
%token CREATE_TYPE_BODY                       
%token CREATE_VIEW                       
%token CREATE_MATERIALIZED_VIEW
%token CREATE_FUNCTION                   
%token CREATE_PROCEDURE                  
%token CREATE_INDEX                      
%token CREATE_UNIQUE_INDEX               
%token CREATE_BITMAP_INDEX               
%token CREATE_TABLE                      
%token CREATE_GLOBAL_TEMPORARY_TABLE     
%token CREATE_DATABASE_LINK              
%token CREATE_SHARED_DATABASE_LINK       
%token CREATE_PUBLIC_DATABASE_LINK       
%token CREATE_SHARED_PUBLIC_DATABASE_LINK
%token ALTER_USER                        
%token ALTER_TABLE                       

%token SUBSTITUTABLE_AT_ALL_LEVELS
%token RESULT_CACHE
%token AUTHENTICATED_BY
%token TYPE
%token DATABASE_LINK
%token MERGE
%token WHEN_MATCHED_THEN_UPDATE_SET
%token WHEN_NOT_MATCHED_THEN_INSERT
%token ON_DATABASE
%token DATE      
%token DECIMAL   
%token FILEtok   
%token FLOAT     
%token INITIALtok   
%token INTEGERtok   
%token LONGtok      
%token MAXEXTENTS
%token NUMBER    
%token RAW       
%token ROWID     
%token SMALLINT  
%token UID       
%token VARCHAR   
%token VARCHAR2  
%token GENERATED_ALWAYS
%token ACCOUNT_LOCK
%token ACCOUNT_UNLOCK
%token GLOBAL_TEMPORARY_TABLE
%token GLOBAL_PARTITION_BY
%token ADD
%token ADMIN
%token AFTER
%token ALLOCATE_EXTENT
%token DEALLOCATE_UNUSED
%token SHRINK_SPACE
%token AGENT_IN
%token UPGRADE
%token NO_FLASHBACK_ARCHIVE
%token FLASHBACK_ARCHIVE
%token MINIMIZE
%token NOMINIMIZE
%token AGGREGATE
%token ALTER
%token ALWAYS
%token ANALYZE
%token ARRAY
%token AS
%token ASC
%token ASSOCIATE
%token AT
%token AUDIT
%token AUTHENTICATION
%token AUTHID
%token AUTOMATIC
%token AUTONOMOUS_TRANSACTION
%token BEFORE
%token BEGINk
%token BULK
%token BULK_ROWCOUNT
%token BY
%token BYTE_tok
%token C_LIBRARY
%token CACHE
%token CALL
%token CASCADE
%token CASE
%token CAST
%token CHARtok
%token CHECK
%token CHUNK
%token AS_CLOB
%token USING_CLOB
%token CLOSE
%token CLUSTER
%token COLLECT
%token COLUMN
%token COLUMNS
%token COMMENT
%token ON_COMMIT
%token COMMIT
%token COMMITED
%token COMPRESS
%token COMPUTE
%token CONNECT
%token CONSTANT
%token CONSTRAINT
%token CONSTRUCTOR_FUNCTION
%token WITH_CONTEXT
%token CREATE
%token CROSS
%token CUBE
%token CURRENT
%token CURRENT_USER
%token CURSOR
%token CYCLE
%token CustomDatum
%token DAY
%token DBA
%token DBTIMEZONE
%token DB_ROLE_CHANGE
%token DDL
%token DECLARE
%token DECREMENT
%token DEFAULT
%token DEFFERABLE
%token DEFERRED
%token SEGMENT
%token CREATION
%token DEFINER
%token DELETE
%token DELETE_HINT
%token UPDATE_HINT
%token INSERT_HINT
%token DESC
%token DETERMINISTIC
%token DIMENSION
%token DIRECTORY
%token DIRECT_LOAD
%token DISABLE
%token DISASSOCIATE
%token DISTINCT
%token DROP
%token DROP_PACKAGE_BODY
%token EACH
%token ELEMENT
%token ELSE
%token ELSIF
%token EMPTY
%token ENABLE
%token ENCRYPT
%token END
%token EQUALS_PATH
%token SHOW_ERRORS
%token LOG_ERRORS
%token ESCAPE
%token EXCEPT
%token EXCEPTION
%token EXCEPTIONS_INTO
%token EXCLUDE
%token EXCLUSIVE
%token EXECUTE
%token EXISTS
%token EXIT
%token EXTEND
%token EXTERNAL
%token EXTERNALLY
%token FETCH
%token FINAL
%token NULLS_FIRST
%token FOR
%token FORALL
%token FORCE
%token FOUND
%token FREEPOOLS
%token FROM
%token FUNCTION
%token OBJECT_IDENTIFIER_IS_SYSTEM_GENERATED
%token GLOBALLY
%token GOTO
%token GRANT
%token GROUP
%token GROUPING_SETS
%token HASH
%token HAVING
%token HEAP
%token HIERARCHY
%token IDENTIFIED_BY
%token IDENTIFIED_EXTERNALLY
%token IDENTIFIED_EXTERNALLY_AS
%token IDENTIFIED_GLOBALLY
%token IDENTIFIED_GLOBALLY_AS
%token OBJECT_IDENTIFIER
%token IF
%token IGNORE
%token IMMEDIATE
%token INCLUDE
%token INCLUDING
%token INCREMENT
%token INDEX
%token INDEXTYPE_IS
%token INDICATOR
%token INFINITE
%token INITIALLY
%token INNER_JOIN
%token INSERT
%token INSTANTIABLE
%token INSTEAD
%token INTO
%token INITRANS
%token MAXTRANS
%token ISOPEN
%token IS_RECORD
%token ITERATE
%token JAVA
%token JAVA_SOURCE
%token JAVA_RESOURCE
%token JOIN
%token KEEP_INDEX
%token MODEL_IGNORE_NAV
%token MODEL_KEEP_NAV
%token FOREIGN_KEY
%token PRIMARY_KEY
%token LANGUAGE
%token NULLS_LAST
%token LEFT_JOIN
%token LEFT_OUTER_JOIN
%token FULL_JOIN
%token FULL_OUTER_JOIN
%token RIGHT_JOIN
%token RIGHT_OUTER_JOIN
%token LESS
%token LEVEL
%token ISOLATION
%token LIBRARY
%token LIKE2
%token LIKE4
%token LIKEC
%token REJECT_LIMIT
%token LIST
%token LOB
%token LOCAL
%token LOCK
%token SKIP_LOCKED
%token LOGGING
%token LOGOFF
%token LOGON
%token LOOP
%token MAP
%token MAPPING
%token MAXVALUE
%token MEASURES
%token MEMBER
%token MINVALUE
%token MODE
%token MODEL
%token MODIFY
%token MONTH
%token MOVEMENT
%token MULTISET
%token EXTERNAL_NAME
%token JAVA_NAME
%token C_NAME
%token NAN_token
%token NATURAL
%token NESTED
%token NEW
%token NO_SALT
%token NO_LOG
%token NO_ROLES
%token NOAUDIT
%token NOCACHE
%token NOCOMPRESS
%token NOCOPY
%token NOCYCLE
%token NOLOGGING
%token NOMAPPING
%token NOMAXVALUE
%token NOMINVALUE
%token NONE
%token NOORDER
%token NOPARALLEL
%token MONITORING
%token NOMONITORING
%token NORELY
%token NOROWDEPENDENCIES
%token NOSORT_tok
%token NOTFOUND
%token NOVALIDATE
%token NOWAIT
%token NULLS
%token NULLk
%token OBJECT
%token OF
%token OID
%token OIDINDEX
%token ON
%token ONLINE
%token ONLY
%token OPEN
%token OPERATIONS
%token OPTION
%token ORDER
%token ORGANIZATION_EXTERNAL
%token ORGANIZATION_HEAP    
%token ORGANIZATION_INDEX   
%token OTHERS
%token OUT
%token OVERFLOWk
%token OVERRIDING
%token OraData
%token PACKAGE
%token PARALLEL
%token PARALLEL_ENABLE
%token ACCESS_PARAMETERS
%token PARTITION
%token PARTITIONS
%token PASSWORD_EXPIRE
%token PCTFREE
%token PCTTHRESHOLD
%token PCTUSED
%token PCTVERSION
%token VERSION_BETWEEN
%token PIPE
%token PIPELINED
%token PIVOT
%token PRAGMA
%token PRESERVE
%token PRIOR
%token PRIVILEGES
%token PROCEDURE
%token PROFILE
%token QUOTA
%token RAISE
%token RANGE
%token READ
%token READS
%token RECORD
%token REF
%token REFERENCE
%token REFERENCES
%token REFERENCING
%token REGEXP_LIKE
%token REJECTk
%token REKEY
%token RELATIONAL
%token RELY
%token RENAME
%token CreateOrReplace
%token REQUIRED
%token RESTRICT_REFERENCES
%token RETENTION
%token RETURN
%token RETURNING
%token REVERSE
%token REVOKE
%token RNDS
%token RNPS
%token DEFAULT_ROLE
%token WITH_ROLE
%token ROLLBACK
%token ROLLUP
%token ROW
%token ROWDEPENDENCIES
%token ROWID_tok
%token ROWNUM
%token RETURN_UPDATED_ROWS
%token ON_COMMIT_DELETE_ROWS
%token ON_COMMIT_PRESERVE_ROWS
%token pROWTYPE
%token pTYPE
%token RULES
%token SALT
%token SAMPLE
%token SAMPLE_BLOCK
%token SAVEPOINT
%token SCHEMA
%token SCN
%token SCOPE
%token SECOND
%token SEED
%token USE_ROLLBACK_SEGMENT
%token SELECT
%token SELECT_HINT
%token SELF
%token SEQUENCE
%token SEQUENTAL
%token SERIALIZABLE
%token SERIALLY_REUSABLE
%token SERVERERROR
%token SESSIONTIMEZONE
%token SET
%token SHARE
%token SHOW
%token SHUTDOWN
%token SIBLINGS
%token SINGLE
%token SORT
%token SQL
%token SQLData
%token START
%token STARTUP
%token STATIC
%token STATISTICS
%token STORAGE
%token OPAQUE
%token STORE
%token SUBMULTISET
%token SUBPARTITION
%token SUBPARTITIONS
%token SUBTYPE
%token SUPPLEMENTAL_LOG
%token SUSPEND
%token PARTITION_BY_SYSTEM 
%token TABLE
%token TABLESPACE
%token SUBPARTITION_TEMPLATE
%token TEMPORARY_TABLESPACE
%token THAN
%token THE
%token THEN
%token THROUGH
%token WITH_TIME_ZONE      
%token WITH_LOCAL_TIME_ZONE
%token TIMESTAMP
%token TIMEZONE
%token TO
%token TRANSACTION
%token TRIGGERS
%token TRUNCATE
%token TRUST
%token DROP_TYPE
%token OF_TYPE
%token UNDER
%token UNDER_PATH
%token UNIQUE
%token LIMIT_UNLIMITED
%token UNLIMITED_ON
%token UNLOCK
%token UNPIVOT
%token UNTIL
%token UPDATE
%token UPDATED
%token UPSERT
%token USE
%token THROUGH_ENTERPRISE_USERS
%token USING
%token VALIDATE
%token AS_VALUE
%token VALUES
%token VARRAY
%token VARYING
%token DEFINE
%token UNDEFINE
%token WAIT
%token WHEN
%token WHERE
%token WHILE
%token WITH
%token WNDS
%token WNPS
%token READ_WRITE
%token XML
%token XMLSCHEMA
%token XMLSchema_URL
%token XMLTYPE
%token YEAR
%token ZONE
/* }}} */
  /* Объявление операторов-строк           {{{ */
%token op_FORMAL_ASSIGN "=>"
%token op_ASSIGN ":="

%token <sqlCompound> op_CONCAT "||"
%token <sqlCompound> '+'
%token <sqlCompound> '-'
%token <sqlCompound> MOD
%token <sqlCompound> op_EXP    "**"

%token op_RANGE  ".."
%token op_LABEL_OPEN  "<<"
%token op_LABEL_CLOSE ">>"
  /* end of Объявление операторов-строк }}} */
  /* Операторы работы с множествами        {{{ */

%left MINUS
%left INTERSECT
%left UNION

  /* end of Операторы работы с множествами }}} */
  /* Арифметические и логические операторы {{{ */

%right ":="
%right "=>"

%left ".."

%left ALL
%left SOME
%left ANY

%left <andOr> OR
%left <andOr> AND
%left NOT
%left UNOT
%left <plComparsionOp> IN
%left PRIOR
%left BETWEEN
%left LIKE
%left IS


%left <plComparsionOp> '>' '<' '=' LE GE NE
%left '+' '-' "||" MOD
%left <sqlCompound> '*' '/'
%left UPLUS UMINUS
%left "**"


%token NULL_CHARACTER
// %destructor { smart::Ptr<Sm::Identificator> p = $$; } <id>

/* end of Арифметические и логические операторы }}} **/

%%

/* base grammar    {{{1 */

schemaElementList:
    schemaElement
  | schemaElementList schemaElement
  ;

schemaElement:
    create                       
  | grant                                   { logSkipped(@$) }
  | alter                        
  | drop                         
  | connect                      
  | ':'                                     { logSkipped(@$) }
  | ';'
  | '/'
  | END selectedEntity endSqlGrammarOpt     { context->release($2); }
  | EXECUTE selectedEntity endSqlGrammarOpt { logSkipped(@$) context->model->actionsContainer.push_back(new Sm::ExecuteImmediate(@$, new Sm::RefExpr(@2, $2->toIdEntitySmart()))); }
  | skipedSchemaElement                     { logSkipped(@$) }
  | blockPlSql                              { logSkipped(@$) context->model->actionsContainer.push_back($1); }
  | commitStatement endSqlGrammarOpt        { logSkipped(@$) context->model->actionsContainer.push_back($1); }
  | updateStatement endSqlGrammarOpt        { logSkipped(@$) context->model->actionsContainer.push_back($1); }
  | cfg_START_CONFIG_CONSTRUCT configGrammar { YYACCEPT; }
  | TOK_TRANSFORM_SENTENCE transformSentenceGrammar TOK_TRANSFORM_SENTENCE { YYACCEPT; }
  ;


skipedSchemaElement:
    SHOW_ERRORS endSqlGrammarOpt
  | SET RawID ON { del ($2); }
  | SET RawID RawID { del($2, $3); }
  | comment
  | NULL_CHARACTER
  | NUMERIC_ID { del($1); }
  | RawID      { del($1); }
  | REFERENCES
  | ENABLE
  | VALIDATE
  | '.'
  | '('
  | ')'
  | '='
  | TABLE
  | SEQUENCE
  | RawID FUNCTION
  | RawID PROCEDURE
  ;



configGrammar:
    cfg_VIRTUAL_ERRORS '=' '[' configVErrList commaOpt ']' {  }
  | cfg_MANUAL_ENTITIES '=' '[' ']'         					 { SmartLexer::setManualEntities(0); }
  | cfg_MANUAL_ENTITIES '=' configBrRawIdCommaList2         	 { SmartLexer::setManualEntities($3); }
  | cfg_ENTITY_FOR_DEPEND_FIND '='  '[' ']'                      { SmartLexer::setEntityForDependFind(0); }
  | cfg_ENTITY_FOR_DEPEND_FIND '='  configBrRawIdCommaList2      { SmartLexer::setEntityForDependFind($3); }
  | cfg_DUMP_ENTITY_LIST '='  '[' ']' 					     	 { SmartLexer::setDumpEntityList(0); }
  | cfg_DUMP_ENTITY_LIST '='  configBrRawIdCommaList2 	     	 { SmartLexer::setDumpEntityList($3); }
  | cfg_SOURCES '='          '[' ']'                             { SmartLexer::clearSources(); }
  | cfg_SOURCES '='          configBrConfigFiles                 { SmartLexer::setPathList($3); }
  | cfg_SOURCES cfg_ADD_SELF configBrConfigFiles                 { SmartLexer::appendPathList($3); }
  | cfg_SOURCES '='          configBrConfigFiles '+' cfg_SOURCES { SmartLexer::insertFrontPathList($3); }
  | cfg_EXISTED_ENTITY_QUERIES '='  '[' ']'                        { SmartLexer::clearExistedEntityQueries(); }
  | cfg_EXISTED_ENTITY_QUERIES '='          configBrRawIdCommaList { SmartLexer::setExistedEntityQueries($3); }
  | cfg_EXISTED_ENTITY_QUERIES cfg_ADD_SELF configBrRawIdCommaList { SmartLexer::appendExistedEntityQueries($3); }
  | cfg_EXISTED_ENTITY_QUERIES '='          configBrRawIdCommaList '+' cfg_EXISTED_ENTITY_QUERIES { SmartLexer::insertFrontExistedEntityQueries($3); }
  | cfg_REFERENCES '=' '[' ']'                                             { SmartLexer::clearReferences(); }
  | cfg_REFERENCES '='          configBrRawIdCommaList2                    { SmartLexer::setReferences($3); }
  | cfg_REFERENCES cfg_ADD_SELF configBrRawIdCommaList2                    { SmartLexer::appendReferences($3); }
  | cfg_REFERENCES '='          configBrRawIdCommaList2 '+' cfg_REFERENCES { SmartLexer::insertFrontReferences($3); }
  | cfg_SKIP_ERRORS '=' '{' '}' { SmartLexer::clearSkippedErrors(); }
  | cfg_SKIP_ERRORS cfg_UNION_SELF configSkipErrorsBrAddNumbers
  | cfg_SKIP_ERRORS cfg_MINUS_SELF configSkipErrorsBrDelNumbers
  | cfg_SKIP_ERRORS '=' '{' { SmartLexer::clearSkippedErrors(); } configSkipErrorsAddNumbers commaOpt '}'
  | cfg_GET_FILENAME configFile { SmartLexer::setParsedFilename(yyscanner, $2); }
  | cfg_OUT_ACTOR_USER '=' configBrRawIdCommaList { SmartLexer::setOutActors($3); }
  | cfg_OUT_ACTOR_USER '=' RawID { SmartLexer::setOutActors($3); }
  | cfg_USERS_EXCEPTIONS_LIST '=' '[' configUserExceptionsList commaOpt ']'
  | cfg_TABLES_SIZES_LIST     '=' '[' configTablesSizesList    commaOpt ']'
  | cfg_TABLES_ROWCOUNT_LIST  '=' '[' configTablesRowcountList commaOpt ']'
  | cfg_DEBUG_TOKEN_LOCATION  '=' '[' NUMERIC_ID ',' NUMERIC_ID ']' {
      context->debugTokenLocation.push_back(Ptr<Sm::NumericValue>($4)->getSIntValue());
      context->debugTokenLocation.push_back(Ptr<Sm::NumericValue>($6)->getSIntValue());
    }
  | cfg_LINTER_RESERVED_KEYWORDS '=' '[' configLinterReservedKeywordsList commaOpt ']'
  
  ;

configLinterReservedKeywordsList:
                                         configLinterReservedKeyword
  | configLinterReservedKeywordsList ',' configLinterReservedKeyword
  ;

configLinterReservedKeyword: RawID { context->s_reservedFields.insert(Ptr<Sm::Identificator>($1)->toNormalizedString()); }

configUserExceptionsList:
                                  configUserException
  | configUserExceptionsList  ',' configUserException
  ;

configUserException: '(' RawID ',' NUMERIC_ID ')' { SmartLexer::addGlobalUserException($2, $4); }
  ;

configTablesSizesList:
                              configTablesSizes
  | configTablesSizesList ',' configTablesSizes
  ;

configTablesSizes: '(' RawID ',' RawID ',' NUMERIC_ID ')'  { SmartLexer::setTableSize($2, $4, $6); }

configTablesRowcountList:
                                 configTablesRowcount
  | configTablesRowcountList ',' configTablesRowcount
  ;

configTablesRowcount: '(' RawID ',' RawID ',' NUMERIC_ID ')' { SmartLexer::setTableRowcount($2, $4, $6); }
  ;

configVErrList:
                       configVErrItem
  | configVErrList ',' configVErrItem
  ;

configVErrItem: '{' configVErrItemDict commaOpt '}' { Sm::AbstractPy::addConfigVirtualError($2); }
  ;

configVErrItemDict:
                           RawID ':' configVErrorListItem { $$ = new Sm::AbstractPy::Dict(@$); (*$$)[smart::Ptr<Sm::Identificator>($1)->toString()] = $3; }
  | configVErrItemDict ',' RawID ':' configVErrorListItem { (*$$)[smart::Ptr<Sm::Identificator>($3)->toString()] = $5; }
  ;
%type <abstractPyDict> configVErrItemDict;

configVErrorListItem:
    NUMERIC_ID                      { $$ = new Sm::AbstractPy::NodeNumId(@$, $1); }
  | '-' NUMERIC_ID                  { $$ = new Sm::AbstractPy::NodeNumId(@$, $2, -1); }
  | RawID                           { $$ = new Sm::AbstractPy::NodeRawId(@$, $1); }
  | configVErrorItemDict            { $$ = $1; }
  | configVErrorItemList            { $$ = $1; }
  ;
%type <abstractPyNode> configVErrorListItem;


configVErrorItemDict: '{' configVErrItemDict commaOpt '}' { $$ = $2; }
  ;
%type <abstractPyDict> configVErrorItemDict;

configVErrorItemList: '[' configVErrorItemListInternal commaOpt ']' { $$ = $2; }
  ;
%type <abstractPyList> configVErrorItemList;

configVErrorItemListInternal:
                                     configVErrorListItem { $$ = new Sm::AbstractPy::List(@$); $$->push_back($1); }
  | configVErrorItemListInternal ',' configVErrorListItem { $$ = $1; $$->push_back($3); }
  ;
%type <abstractPyList> configVErrorItemListInternal;


configSkipErrorsBrAddNumbers:
    '{' configSkipErrorsAddNumbers commaOpt '}'
  ;

configSkipErrorsAddNumbers:
                                   NUMERIC_ID { SmartLexer::addSkippedError(smart::Ptr<Sm::NumericValue>($1)->getSIntValue()); }
  | configSkipErrorsAddNumbers ',' NUMERIC_ID { SmartLexer::addSkippedError(smart::Ptr<Sm::NumericValue>($3)->getSIntValue()); }
;

configSkipErrorsBrDelNumbers:
    '{' configSkipErrorsDelNumbers commaOpt '}'
  ;

configSkipErrorsDelNumbers:
                                   NUMERIC_ID { SmartLexer::delSkippedError(smart::Ptr<Sm::NumericValue>($1)->getSIntValue()); }
  | configSkipErrorsDelNumbers ',' NUMERIC_ID { SmartLexer::delSkippedError(smart::Ptr<Sm::NumericValue>($3)->getSIntValue()); }
;


commaOpt:
    /* EMPTY */
  | ','
  ;


configBrConfigFiles:  '[' configFiles commaOpt ']' { $$ = $2; }

%type <idList> configBrConfigFiles;

configFiles:
                    configFile { $$ = mkList($1); }
  | configFiles ',' configFile { $$ = $1; $$->push_back($3); }
  ;
%type <idList> configFiles;


configFile:
    configPathItem   { $$ = $1; }
  | cfg_JOIN '(' configPathCommaList ')' { $$ = SmartLexer::joinPathList($3); }
  | cfg_SYSDEPS { $$ = new Sm::Identificator(syntaxerContext.model->modelActions.oracleSystemSource); }
  ;
%type <id> configFile;

configBrRawIdCommaList:
    '[' configRawIdCommaList commaOpt ']' { $$ = $2; }
  ;



configPathItem:
    RawID              { $$ = $1; }
  | cfg_TEMPORARY_PATH { $$ = new Sm::Identificator(@1, SmartLexer::pyTok2Str(yy::parser::token::cfg_TEMPORARY_PATH), 0); }
  ;

%type <id> configPathItem;

configPathCommaList:
                            configPathItem { $$ = mkList($1);}
  | configPathCommaList ',' configPathItem { $$ = $1; $$->push_back($3); }
  ;
%type <idList> configPathCommaList;



configRawIdCommaList:
                             RawID { $$ = mkList($1);}
  | configRawIdCommaList ',' RawID { $$ = $1; $$->push_back($3); }
  ;
%type <idList> configRawIdCommaList configBrRawIdCommaList;


configBrRawIdCommaList2:
   '[' configRawIdCommaList2 commaOpt ']' { $$ = $2; }

configRawIdCommaList2:
                              configBrRawIdCommaList { $$ = mkList(SmartLexer::idListToIdEntity($1)); }
  | configRawIdCommaList2 ',' configBrRawIdCommaList { $$ = $1; $$->push_back(SmartLexer::idListToIdEntity($3)); }
  ;

%type <entityList> configRawIdCommaList2  configBrRawIdCommaList2;




transformSentenceListItem:
     assignmentStmt ';' { $$ = $1; }
   | executeImmediateStmt ';' { $$ = $1; }
   ;
%type <statement> transformSentenceListItem;


transformSentenceList:
                           transformSentenceListItem { $$ = new Sm::BaseList<Sm::StatementInterface>($1); }
  | transformSentenceList  transformSentenceListItem { $$ = $1; $$->push_back($2); }
  ;
%type <statementsList> transformSentenceList;



transformSentenceGrammarInternal:
  | assignmentStmt intoClauseOpt { sentence_transformer::transformStatement( $1, $2, false); }
  | condition      intoClauseOpt { sentence_transformer::transformExpression($1, $2, false); }
  | transformSentenceList { sentence_transformer::transformStmtList($1); }
  ;

transformSentenceGrammar:
     transformSentenceGrammarInternal
  |  transformSentenceGrammarInternal ';'
  ;

create:
    CREATE_USER createUser               endSqlGrammarOpt      /* context action - in grammar */ /* TODO: Убрать указатели */
  | package endSqlGrammarOpt                                 { Sm::Package *p = $1; context->model->addPackage(p); }
    /* TODO: need to resolve synonyms (may be on the new stage) */
  | CREATE_SYNONYM createSynonym endSqlGrammarOpt            { context->model->addSynonym($2); }
  | CREATE_PUBLIC_SYNONYM createSynonym endSqlGrammarOpt     { if ($2) $2->setPublic(); context->model->addSynonym($2); }
  | CREATE_SEQUENCE sequenceBody endSqlGrammarOpt            { Sm::Add::hash (*context->model, Ptr<Sm::Sequence>    ($2), &UserContext::sequences); }
  | CREATE_TRIGGER createTrigger endSqlGrammarOpt            { Sm::Add::hash (*context->model, Ptr<Sm::Trigger>     ($2), &UserContext::triggers ); }
  | createTypeGrammar
  | createViewGrammar
  | createFunctionSpec  endSqlGrammarOpt                     { Sm::Add::hash (*context->model, Ptr<Sm::Function>    ($1), &UserContext::functions); }
  | functionSpec        endSqlGrammarOpt                     { Sm::Add::hash (*context->model, Ptr<Sm::Function>    ($1), &UserContext::functions); }
  | createProcedureSpec               endSqlGrammarOpt       { Sm::Add::hash (*context->model, Ptr<Sm::Function>    ($1), &UserContext::functions); }
  | procedureSpec                     endSqlGrammarOpt       { Sm::Add::hash (*context->model, Ptr<Sm::Function>    ($1), &UserContext::functions); }
  | createIndexStatement endSqlGrammarOpt                    { Sm::Add::hash (*context->model, Ptr<Sm::Index>       ($1), &UserContext::indices  ); }
  | CREATE_TABLE                  tableBody endSqlGrammarOpt { if ($2) $2->isGlobalTemporary(false); context->model->addTable($2); }
  | CREATE_GLOBAL_TEMPORARY_TABLE tableBody endSqlGrammarOpt { if ($2) $2->isGlobalTemporary(true);  context->model->addTable($2); }
  | CREATE_DATABASE_LINK               databaseLink endSqlGrammarOpt { Sm::Add::hash(*context->model, Ptr<Sm::DatabaseLink>($2), &UserContext::dblinks); }
  | CREATE_SHARED_DATABASE_LINK        databaseLink endSqlGrammarOpt { if ($2) $2->setShared();       Sm::Add::hash(*context->model, Ptr<Sm::DatabaseLink>($2), &UserContext::dblinks); }
  | CREATE_PUBLIC_DATABASE_LINK        databaseLink endSqlGrammarOpt { if ($2) $2->setPublic();       Sm::Add::hash(*context->model, Ptr<Sm::DatabaseLink>($2), &UserContext::dblinks); }
  | CREATE_SHARED_PUBLIC_DATABASE_LINK databaseLink endSqlGrammarOpt { if ($2) $2->setPublicShared(); Sm::Add::hash(*context->model, Ptr<Sm::DatabaseLink>($2), &UserContext::dblinks); }
  ;

createViewGrammar:
    CREATE_VIEW createView endSqlGrammarOpt { Sm::Add::hash (*context->model, Ptr<Sm::View>        ($2), &UserContext::views    ); }
  | CREATE_MATERIALIZED_VIEW createView endSqlGrammarOpt {
      Sm::View *v = $2;
      v->isMaterialized = true;
      Sm::Add::hash (*context->model, Ptr<Sm::View>(v), &UserContext::views);
    }
  ;


comment:
    COMMENT ON droppedEntity selectedEntity AsIs RawID { context->release($4); del($6); logSkipped(@$) };

createTypeGrammarHead:
    CREATE_TYPE objectTypeDeclaration { Sm::Add::hash (*context->model, Ptr<Sm::Type::ObjectType>($2), &UserContext::types    ); }
  ;

createTypeGrammarBody:
    CREATE_TYPE_BODY objectTypeDefinition  { context->model->addObjectTypeBody($2); }
  ;

createTypeGrammar:
    createTypeGrammarHead endSqlGrammarOpt
  | createTypeGrammarHead createTypeGrammarBody endSqlGrammarOpt
  | createTypeGrammarBody endSqlGrammarOpt
  ;

/*  
 * Alter User and alter table command
 * Alter User:
 *    for each users in list
 * 1) find user context -> add alter user command
 *    if not found      -> do nothing
 * Alter Table:
 * 2) find user context -> find table -> add table context
 *                      -> if not found - do nothing
 *    if not found - do nothing
 */

alter:
    alterUser                   { context->model->alterUser($1); }
  | alterTable endSqlGrammarOpt { context->model->alterTable($1); }
  | alterTriggerEnableDisable  endSqlGrammarOpt { logSkipped(@$) }
  ;


alterTriggerEnableDisable:
    ALTER_TRIGGER schemaDotColumn EnableState;
  ;





drop:         /*{{{*/
    DROP_TYPE schemaDotColumn ForceOrValidateOpt  endSqlGrammarOpt                     { logSkipped(@$) del($2); }
  | DROP_PACKAGE_BODY schemaDotColumn ForceOrValidateOpt  endSqlGrammarOpt             { logSkipped(@$) del($2); }
  | DROP droppedEntity selectedEntity cascadeConstraintsInvalidateOpt endSqlGrammarOpt { logSkipped(@$) context->release($3); }
  ;

droppedEntity:
    SEQUENCE
  | COLUMN
  | CONSTRAINT
  | INDEX
  | TABLE
  | RawID { del($1); }
  | FUNCTION
  | PROCEDURE
  | DATABASE_LINK
  ;

/*}}}*/


ForceOrValidateOpt:
    FORCE
  | VALIDATE
  | /* EMPTY */
  ;

endSqlGrammarOpt:
    '/'
  | ';'
  ;

/* end of base grammar 1}}} */
/* CONNECT grammar {{{1 */
connect:
    CONNECT RawID endSqlGrammarOpt { context->model->connect($2); }
  ;

  /* }}}1 */
/* CREATE TRIGGER  {{{1 */
createTrigger:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Trigger_, $1); $$ = 0; }
  | rawIdDotRawId /*2DDL*/
    triggerMode             /*3*/
    triggerEvents           /*4*/
    triggerFollowsOpt       /*5*/
    whenConditionOpt        /*6*/
    EnableStateOpt          /*7*/
    triggerActionCode { $$ = new Sm::Trigger(@$, $1, $3, $2, $4, $6, $5, $7); }
  ;
%type <trigger> createTrigger;

triggerMode:              /*{{{*/
    BEFORE     { $$ = Sm::trigger::BEFORE;     }
  | AFTER      { $$ = Sm::trigger::AFTER;      }
  | INSTEAD OF { $$ = Sm::trigger::INSTEAD_OF; }
  | FOR        { $$ = Sm::trigger::FOR;        }
  ;
%type <trigger_Mode> triggerMode;
                          /*}}}*/
whenConditionOpt:         /*{{{*/
    WHEN '(' condition ')' { $$ = $3;  $$->loc(@$); }
  | /* EMPTY */            { $$ = 0; }
  ;
%type <plCond> whenConditionOpt;
                          /*}}}*/
triggerEvents:            /*{{{*/
    dmlEventClause                   { $$ = $1; }
  | nonDmlEvents ON RawID '.' SCHEMA { $$ = new Sm::trigger::NonDmlEvents(@$, $1, $3, Sm::trigger::NonDmlEvents::SCHEMA); }
  | nonDmlEvents ON_DATABASE         { $$ = new Sm::trigger::NonDmlEvents(@$, $1, 0 , Sm::trigger::NonDmlEvents::DATABASE); }
  ;
%type <trigger_Events> triggerEvents;
                         /*}}}*/
nonDmlEvents:             /*{{{*/
                    nonDmlEvent { $$ = $1; }
  | nonDmlEvents OR nonDmlEvent { $$ = $1; $$.i |= $3.i; }
  ;

nonDmlEvent:
    ASSOCIATE STATISTICS    { $$.f = Sm::trigger::non_dml_events::ASSOCIATE_STATISTICS;    }
  | CREATE                  { $$.f = Sm::trigger::non_dml_events::CREATE;                  }
  | DISASSOCIATE STATISTICS { $$.f = Sm::trigger::non_dml_events::DISASSOCIATE_STATISTICS; }
  | LOGOFF                  { $$.f = Sm::trigger::non_dml_events::BEFORE_LOGOFF;           }
  | RENAME                  { $$.f = Sm::trigger::non_dml_events::RENAME;                  }
  | SHUTDOWN                { $$.f = Sm::trigger::non_dml_events::BEFORE_SHUTDOWN;         }
  | TRUNCATE                { $$.f = Sm::trigger::non_dml_events::TRUNCATE;                }
  | ALTER                   { $$.f = Sm::trigger::non_dml_events::ALTER;                   }
  | AUDIT                   { $$.f = Sm::trigger::non_dml_events::AUDIT;                   }
  | DB_ROLE_CHANGE          { $$.f = Sm::trigger::non_dml_events::AFTER_DB_ROLE_CHANGE;    }
  | DROP                    { $$.f = Sm::trigger::non_dml_events::DROP;                    }
  | LOGON                   { $$.f = Sm::trigger::non_dml_events::AFTER_LOGON;             }
  | REVOKE                  { $$.f = Sm::trigger::non_dml_events::REVOKE;                  }
  | STARTUP                 { $$.f = Sm::trigger::non_dml_events::AFTER_STARTUP;           }
  | ANALYZE                 { $$.f = Sm::trigger::non_dml_events::ANALYZE;                 }
  | COMMENT                 { $$.f = Sm::trigger::non_dml_events::COMMENT;                 }
  | DDL                     { $$.f = Sm::trigger::non_dml_events::DDL;                     }
  | GRANT                   { $$.f = Sm::trigger::non_dml_events::GRANT;                   }
  | NOAUDIT                 { $$.f = Sm::trigger::non_dml_events::NOAUDIT;                 }
  | SERVERERROR             { $$.f = Sm::trigger::non_dml_events::AFTER_SERVERERROR;       }
  | SUSPEND                 { $$.f = Sm::trigger::non_dml_events::AFTER_SUSPEND;           }
  ;
%type <trigger_NonDmlFlags> nonDmlEvent nonDmlEvents;
                          /*}}}*/
dmlEventClause:           /*{{{*/
    dmlTriggerEvents ON nestedTableFieldOpt schemaDotColumn referencingClauseOpt FOR_EACH_ROW_opt
    /* triggerEditionClauseOpt */
    { $$ = new Sm::trigger::DmlEvents(@$, p($1), p($4), p($3), p($5), $6); }
  ;
%type <trigger_DmlEvents> dmlEventClause;
                          /*}}}*/
dmlTriggerEvents:         /*{{{*/
                        dmlTriggerEvent { $$ = $1; }
  | dmlTriggerEvents OR dmlTriggerEvent { $$ = $1; $$->concat($3); $$->loc(@$); }
  ;
dmlTriggerEvent:
    DELETE                 { $$ = new Sm::trigger::DmlEvent(@$, Sm::trigger::DmlEvent::CathegoryEvent::DELETE); }
  | INSERT                 { $$ = new Sm::trigger::DmlEvent(@$, Sm::trigger::DmlEvent::CathegoryEvent::INSERT); }
  | UPDATE                 { $$ = new Sm::trigger::DmlEvent(@$, Sm::trigger::DmlEvent::CathegoryEvent::UPDATE); }
  | UPDATE OF columnIdList { $$ = new Sm::trigger::DmlEvent(@$, $3); }
  | INSERT OF columnIdList { $$ = new Sm::trigger::DmlEvent(@$, Sm::trigger::DmlEvent::CathegoryEvent::INSERT); del($3); } /* TODO: INSERT OF field не поддерживается в БД*/
  ;
%type <dmlTriggerEvent> dmlTriggerEvent dmlTriggerEvents;

triggerFollowsOpt:
    /* EMPTY */ { $$ = 0; }
  | RawID /*FOLLOWS|PRECEDES*/ schemaDotEntityList 
    { 
      if ( $1->isKeyword(STRING("FOLLOWS")) )
        $$ = $2;   
      else
        $$ = 0;
      logSkipped(@$)
    }
  ;
%type <id2List> triggerFollowsOpt;

                          /*}}}*/

/*
Определение nested_table_column представления, на котором определен триггер
Такой триггер срабатывает только тогда, когда DML работает с элементами
вложенной таблицы.
*/
nestedTableFieldOpt:         /*{{{*/
    NESTED TABLE exprId OF { $$ = $3;  }
  | /* EMPTY */            { $$ = 0; }
  ;
%type <id> nestedTableFieldOpt;
                          /*}}}*/
referencingClauseOpt:     /*{{{*/
    REFERENCING triggerReferencingList            { $$ = $2; }
  | /* EMPTY */                                   { $$ = 0; }
  ;
triggerReferencingList:
                           triggerReferencingItem { $$ = mkList($1); }
  | triggerReferencingList triggerReferencingItem { $$ = $1; $$->push_back($2); }
  ;
%type <triggerReferencingList> triggerReferencingList referencingClauseOpt;
                          /*}}}*/
triggerReferencingItem:   /*{{{*/
    RawID  AS_opt exprId /* $1 = OLD | NEW | PARENT */ 
    { 
      if      ( $1->isKeyword( STRING("PARENT") ) )
        $$ = new Sm::trigger::DmlReferencing(p($3), Sm::trigger::TriggerAbstractRowReference::PARENT);
      else if ( $1->isKeyword( STRING("OLD"   ) ) )
        $$ = new Sm::trigger::DmlReferencing(p($3), Sm::trigger::TriggerAbstractRowReference::OLD);
      else
       $$ = 0;
      del($1);
    } 
  | NEW AS_opt exprId  { $$ = new Sm::trigger::DmlReferencing($3, Sm::trigger::TriggerAbstractRowReference::NEW); }
  ;
%type <triggerReferencing> triggerReferencingItem;
                          /*}}}*/
AS_opt:                   /*{{{*/
    AS
  | /* EMPTY */
  ;
                          /*}}}*/
FOR_EACH_ROW_opt:         /*{{{*/
    FOR EACH ROW { $$ = true;  }
  | /* EMPTY */  { $$ = false; }
  ;
%type <boolval> FOR_EACH_ROW_opt;

EnableStateOpt:
    EnableState { $$ = $1; }
  | /* EMPTY */ { $$ = Sm::EnableState::E_EMPTY; }
  ;
%type <enableState> EnableStateOpt;
                          /*}}}*/
triggerActionCode:        /*{{{*/
    triggerCallProcedure { $$ = $1; }
  | blockPlSql           { $$ = new Sm::trigger::TriggerCode(@1, $1); }
  ;
%type <triggerAction> triggerActionCode;
                          /*}}}*/
triggerCallProcedure:     /*{{{*/
    CALL sqlExpr intoExprStmtOpt { $$ = new Sm::trigger::Funcall(@$, $2, $3); } /* call object metod or function */
  ;
%type <trigger_Funcall> triggerCallProcedure;

intoExprStmtOpt:
    INTO sqlExpr { $$ = $2;  $$->loc(@$); }
  | /* EMPTY */  { $$ = 0; }
  ;
%type <sqlExpr> intoExprStmtOpt;
                          /*}}}*/
/* }}} */
/* CREATE SYNONYM  {{{  */
createSynonym:
     rawIdDotRawId WRAPPED_ENTITY      { context->model->addWrapped(Sm::ResolvedEntity::Synonym_, $1); $$ = 0; }
  |  rawIdDotRawId FOR schemaDotColumn { $$ = new Sm::Synonym($1, $3, false, @$); }
  ;
%type <synonym> createSynonym;

                /* }}} */
/* ALTER TABLE     {{{  */

alterTable:
    ALTER_TABLE schemaDotColumn /* DDL */ 
                alterTableCommand alterTableEnablingSpecOpt { $$ = new Sm::AlterTable(@$, $2, $3, $4); }
  ;
%type <alterTable> alterTable;
alterTableCommand:
    manipulateFields                               { $$ = new Sm::alter_table::ManipulateFields(@$, $1); logSkipped(@$) }
  | renameField                                    { $$ = $1; logSkipped(@$) }
  | modifyNestedTableRetvalue                      { $$ = 0;  logSkipped(@$) }
  | alterConstraintBase                            { $$ = $1; }
  | RENAME TO exprId alterIotClausesCompressionOpt { $$ = new Sm::alter_table::RenameTable(@$, $3); logSkipped(@$) } /*TODO: alterXMLSchemaClauseOpt <- need to skip*/
  | alterTablePropertiesList alterIotClausesOpt    { $$ = 0; logSkipped(@$) } /* <- scip; TODO: alterXMLSchemaClauseOpt <- need to skip*/
  | READ ONLY         /* scip */                   { $$ = 0; logSkipped(@$) }
  | READ_WRITE        /* scip */                   { $$ = 0; logSkipped(@$) }
  | shrinkClause      /* scip */                   { $$ = 0; logSkipped(@$) }
  | REKEY encryptSpec /* scip */                   { $$ = 0; del($2); }
  ;
%type <alterTableCommand> alterTableCommand;
  /* http://docs.oracle.com/cd/B28359_01/server.111/b28286/statements_3001.htm#i2087440 */
  /*| TODO: alterTablePartitioning       <- need to skip */
  /*| TODO: alter_external_table_clauses <- need to skip */
  /*| TODO: move_table_clause            <- need to skip */

shrinkClause:                      /*{{{ <- scip */
    SHRINK_SPACE               { logSkipped(@$) }
  | SHRINK_SPACE RawID         { del($2); logSkipped(@$) } /* $2 = COMPACT */
  | SHRINK_SPACE CASCADE       { logSkipped(@$) }
  | SHRINK_SPACE RawID CASCADE { del($2); logSkipped(@$) } /* $2 = COMPACT */
  ;
                                   /*}}}*/
renameField:                       /*{{{*/
    RENAME COLUMN exprId TO exprId { $$ = new Sm::alter_table::RenameField(@$, $3, $5); }
  ;
%type <alterTable_RenameField> renameField;
                                   /*}}}*/
modifyNestedTableRetvalue:         /*{{{ <- scip */
                              modifyCollectionRetrieval
  | modifyNestedTableRetvalue modifyCollectionRetrieval
  ;
modifyCollectionRetrieval:
    MODIFY NESTED TABLE exprId RETURN AS_VALUE   { del($4); }
  | MODIFY NESTED TABLE exprId RETURN AS RawID   { del($4); /* LOCATOR */ }
  ;
                                   /*}}}*/
manipulateFields:                  /*{{{*/
                     alterFieldsBase { $$ = mkList($1); }
  | manipulateFields alterFieldsBase { $$ = $1; $$->push_back($2); }
  ;
%type <manipulateFields> manipulateFields;

alterFieldsBase:                            /*{{{*/
    addFields    { $$ = $1; logSkipped(@$) }
  | modifyFields { $$ = $1; logSkipped(@$) }
  | dropFields   { $$ = $1; logSkipped(@$) }
  ;
%type <alterTable_AlterFieldsBase> alterFieldsBase;

addFields:                                      /*{{{*/
    ADD '(' addedColumns ')' fieldPropertiesOpt { $$ = new Sm::alter_table::AddFields(@$, $3, $5); }
  ;
%type <alterTable_AddFields> addFields;
addedColumns:
                     columnDefinitionAdd { $$ = mkList($1); }
  | addedColumns ',' columnDefinitionAdd { $$ = $1; $$->push_back($3); }
  ;
%type <table_FieldDefinitionList> addedColumns;
                                                      /*}}}*/
modifyFields:                                  /*{{{*/
    MODIFY '(' modifyFieldsList ')'        { $$ = new Sm::alter_table::ModifyFields(@$, $3); logSkipped(@$) }
  | MODIFY     modifyColSubstituable       { $$ = 0; logSkipped(@$) }
  | MODIFY     singleAlterColumnDefinition { $$ = new Sm::alter_table::ModifyFields(@$, mkList($2)); logSkipped(@$) }
  | MODIFY  alterFieldId i_constraintNamed { del($2); $$ = 0; /* TODO: */ del($3); logSkipped(@$) }
  ;
%type <alterTable_ModifyFields> modifyFields;
modifyFieldsList:                                          /*{{{*/
                         modifyFieldDefinition { $$ = mkList($1); }
  | modifyFieldsList ',' modifyFieldDefinition { $$ = $1; $$->push_back($3); }
  ;
%type <table_FieldDefinitionList> modifyFieldsList ;

modifyFieldDefinition:
    singleAlterColumnDefinition lobStorageClauseOpt { $$ = $1; } /* TODO: alterXMLschemasClauseOpt <- need to skip */
  ;
%type <table_FieldDefinition> modifyFieldDefinition;
                                                              /*}}}*/
modifyColSubstituable:                                        /*{{{ <- scip */
/* Use this clause to set or change the substitutability of an existing object type column. */
    COLUMN exprId NOT_opt SUBSTITUTABLE_AT_ALL_LEVELS FORCE_opt { del($2); }
  ;
                                                              /*}}}*/
                                                      /*}}}*/
FORCE_opt:                                            /*{{{ <- scip */
    FORCE
  | /* EMPTY */
  ;
                                                      /*}}}*/
dropFields:                                     /*{{{*/
    DROP COLUMN exprId               cascadeConstraintsInvalidateOpt checkpointOpt { $$ = new Sm::alter_table::DropFields(@$, $4, mkList($3)); }
  | DROP COLUMN '(' columnIdList ')' cascadeConstraintsInvalidateOpt checkpointOpt { $$ = new Sm::alter_table::DropFields(@$, $6, $4); }
  /* TODO Other drop syntax  - в оракле было больше*/
  ;
%type <alterTable_DropFields> dropFields;
cascadeConstraintsInvalidateOpt:                              /*{{{ <- scip */
    CASCADE RawID /* CONSTRAINTS */ { del($2); $$ = true; }
  | RawID         /* INVALIDATE  */ { del($1); $$ = false; }
  | RawID CASCADE RawID             { del($1, $3); $$ = true;  }
  | CASCADE RawID RawID             { del($2, $3); $$ = true;  }
  | /* EMPTY */                     { $$ = false; }
  ;
%type <boolval> cascadeConstraintsInvalidateOpt;
                                                              /*}}}*/
checkpointOpt:                                                /*{{{ <- scip */
    RawID NUMERIC_ID /* $1 = CHECKPOINT */ { del($1, $2); }
  | /* EMPTY */
  ;
                                                              /*}}}*/
                                                      /*}}}*/
                                                   /*}}}*/
                                   /*}}}*/
alterTableEnablingSpecOpt:         /*{{{*/
    alterTableEnablingSpec { $$ = $1; logSkipped(@$) }
  | /* EMPTY */            { $$ = 0; }
  ;
alterTableEnablingSpec:
                           alterTableEnablingSpecItem { $$ = mkList($1); }
  | alterTableEnablingSpec alterTableEnablingSpecItem { $$ = $1; $$->push_back($2); }
  ;
%type <table_EnableDisableConstraintList> alterTableEnablingSpecOpt alterTableEnablingSpec;
alterTableEnablingSpecItem:
    enableDisableConstraint { $$ = $1; $$->loc(@$); }
  | ENABLE  TABLE LOCK      { $$ = 0; logSkipped(@$) }
  | ENABLE  ALL   TRIGGERS  { $$ = 0; logSkipped(@$) }
  | DISABLE TABLE LOCK      { $$ = 0; logSkipped(@$) }
  | DISABLE ALL   TRIGGERS  { $$ = 0; logSkipped(@$) }
  ;
%type <table_EnableDisableConstraint> alterTableEnablingSpecItem;
                                   /*}}}*/
alterTablePropertiesList:          /*{{{ <- scip */
                             alterTableProperty
  | alterTablePropertiesList alterTableProperty
  ;
alterTableProperty:                        /*{{{ <- scip */
    physicalAttributeClause      /* <- scip */ { logSkipped(@$) }
  | LOGGING                      /* <- scip */ { logSkipped(@$) }
  | NOLOGGING                    /* <- scip */ { logSkipped(@$) }
  | tableCompression             /* <- scip */ { logSkipped(@$) }
  | CACHE                        /* <- scip */ { logSkipped(@$) }
  | NOCACHE                      /* <- scip */ { logSkipped(@$) }
  | parallelClause               /* <- scip */ { logSkipped(@$) }
  | supplementalTableLogging     /* <- scip */ { logSkipped(@$) }
  | allocateExtentClause         /* <- scip */ { logSkipped(@$) }
  | deallocateUnusedClause       /* <- scip */ { logSkipped(@$) }
  | upgrade_table_clause         /* <- scip */ { logSkipped(@$) }
  | recordsPerBlockClause        /* <- scip */ { logSkipped(@$) }
  | ENABLE ROW MOVEMENT /* <- scip; conflict with alterTableEnablingSpecOpt->enableDisable */  { logSkipped(@$) }
  | DISABLE ROW MOVEMENT /* <- scip; conflict with alterTableEnablingSpecOpt->enableDisable */ { logSkipped(@$) }
  | flashbackArchiveClause       /* <- scip */ { logSkipped(@$) }
  ;
                                           /*}}}*/
supplementalTableLogging:                  /*{{{ <- scip */
    ADD  addSupplementalTableLogs
  | DROP dropSupplementalTableLogs
  ;
addSupplementalTableLogs:
                                 addSupplementalTableLog
  | addSupplementalTableLogs ',' addSupplementalTableLog
  ;
addSupplementalTableLog:
    SUPPLEMENTAL_LOG supplemental_log_grp_clause { logSkipped(@$) }
  | SUPPLEMENTAL_LOG supplemental_id_key_clause  { logSkipped(@$) }
  ;
dropSupplementalTableLogs:
                                 dropSupplementalTableLog
  | dropSupplementalTableLog ',' dropSupplementalTableLog
  ;
dropSupplementalTableLog:
    SUPPLEMENTAL_LOG supplemental_log_grp_clause { logSkipped(@$) }
  | SUPPLEMENTAL_LOG GROUP RawID { del($3); logSkipped(@$) }
  ;
supplemental_log_grp_clause:
    GROUP RawID '(' idListNoLog ')' ALWAYS_opt { del($2); logSkipped(@$) }
  ;
idListNoLog:
                    exprId NO_LOG_opt { del($1); logSkipped(@$) }
  | idListNoLog ',' exprId NO_LOG_opt { del($3); logSkipped(@$) }
  ;
supplemental_id_key_clause:
    RawID '(' keyConstraints ')' COLUMNS /* $1 = DATA */ { del($1); logSkipped(@$) }
  ;
keyConstraints:
                       keyConstraint
  | keyConstraints ',' keyConstraint
  ;
keyConstraint:
    ALL
  | PRIMARY_KEY
  | UNIQUE
  | FOREIGN_KEY
  ;
                                           /*}}}*/
allocateExtentClause:                      /*{{{ <- scip */
    ALLOCATE_EXTENT allocateExtentBrListOpt { logSkipped(@$) }
  ;
allocateExtentBrListOpt:
    '(' allocateExtentList ')' { logSkipped(@$) }
  | /* EMPTY */
  ;
allocateExtentList:
                       allocateExtent
  | allocateExtentList allocateExtent
  ;
allocateExtent:
    RawID sizeClause /* $1 = SIZE */      { del($1); logSkipped(@$) }
  | RawID RawID      /* $1 = DATAFILE */  { del($1, $2); logSkipped(@$) }
  | RawID NUMERIC_ID /* $1 = INSTANCE */  { del($1, $2); logSkipped(@$) }
  ;
                                           /*}}}*/
deallocateUnusedClause:                    /*{{{ <- scip */
    DEALLOCATE_UNUSED
  | DEALLOCATE_UNUSED RawID sizeClause /* $2 = KEEP */ { del($2); logSkipped(@$) }
  ;
                                           /*}}}*/

upgrade_table_clause:                      /*{{{ <- skip */
/*
 * Upgrade_table_clause относится к объекным таблицам и к реляционным таблицам с
 * обектными столбцами.  Она позволяет отправить СУБД Oracle команду на
 * преобразование метаданных целевой таблицы в соответствии с последней версией
 * всех ссылочных типов. Если таблица уже валидна, то метаданные таблицы остаются
 * неизменными.
 */
    UPGRADE NOT_opt INCLUDING RawID /* $4 = DATA */ fieldProperties { del($4,$5); logSkipped(@$) }
  | UPGRADE                                         fieldProperties { del($2); logSkipped(@$) }
  ;
fieldProperties:
                    fieldProperty { $$ = mkList($1); }
  | fieldProperties fieldProperty { $$ = $1; $$->push_back($2); }
  ;
fieldPropertiesOpt:
    /* EMPTY */      { $$ = 0; }
  | fieldProperties  { $$ = $1; }
  ;
%type<tableFieldProperties> fieldProperties fieldPropertiesOpt;
                                           /*}}}*/
recordsPerBlockClause:                     /*{{{ <- skip */
    MINIMIZE   RawID { del($2); logSkipped(@$) }
  | NOMINIMIZE RawID { del($2); logSkipped(@$) }
  ;
                                           /*}}}*/
flashbackArchiveClause:                    /*{{{ <- skip */
    NO_FLASHBACK_ARCHIVE    { logSkipped(@$) }
  | FLASHBACK_ARCHIVE RawID { del($2); logSkipped(@$) }
  | FLASHBACK_ARCHIVE       { logSkipped(@$) }
  ;
                                           /*}}}*/
                               /*}}}*/
alterIotClausesOpt:                /*{{{ <- scip */
    indexOrgTableClause
  | addOwerflowClause
  | alterMappingTableClauses
  ;
indexOrgTableClause:
    indexOrgPrefixOpt includingOpt segOwerflowOpt
  ;
indexOrgPrefixOpt:
    indexOrgPrefix
  | /* EMPTY */
  ;
indexOrgPrefix:
                   indexOrg
  | indexOrgPrefix indexOrg
  ;
indexOrg:
    MAPPING TABLE { logSkipped(@$) }
  | NOMAPPING { logSkipped(@$) }
  | PCTTHRESHOLD NUMERIC_ID { del($2); logSkipped(@$) }
  ;                                /*}}}*/
alterIotClausesCompressionOpt:     /*{{{ <- scip */
    indexOrgTableCompressionClause { logSkipped(@$) }
  | addOwerflowClause { logSkipped(@$) }
  | alterMappingTableClauses { logSkipped(@$) }
  | RawID /* <- COALESCE */ { del($1); logSkipped(@$) }
  ;
indexOrgTableCompressionClause:
    indexOrgPrefixCompressionOpt includingOpt segOwerflowOpt
  ;
indexOrgPrefixCompressionOpt:
    indexOrgCompressionPrefix
  | /* EMPTY */
  ;
indexOrgCompressionPrefix:
                              indexOrgCompression
  | indexOrgCompressionPrefix indexOrgCompression
  ;
indexOrgCompression:
    MAPPING TABLE           { logSkipped(@$) }
  | NOMAPPING               { logSkipped(@$) }
  | PCTTHRESHOLD NUMERIC_ID { del($2); logSkipped(@$) }
  | COMPRESS                { logSkipped(@$) }
  | COMPRESS NUMERIC_ID     { del($2); logSkipped(@$) }
  | NOCOMPRESS { logSkipped(@$) }
  ;

includingOpt:
    INCLUDING exprId { del($2); logSkipped(@$) }
  | /* EMPTY */
  ;
                                   /*}}}*/
addOwerflowClause:                 /*{{{ <- scip */
   ADD OVERFLOWk segmentAttrsClauseOpt brPartitionSegAttrsOpt { logSkipped(@$) }
 | OVERFLOWk addOwerflowList { logSkipped(@$) }
 ;

addOwerflowList:                           /*{{{*/
                    addOwerflowItem
  | addOwerflowList addOwerflowItem
  ;
addOwerflowItem:
    segmentAttrClause { logSkipped(@$) }
  | allocateExtentClause { logSkipped(@$) }
  | shrinkClause { logSkipped(@$) }
  | deallocateUnusedClause { logSkipped(@$) }
  ;
                                           /*}}}*/
brPartitionSegAttrsOpt:                    /*{{{*/
    '(' partitionSegAttrs ')' { logSkipped(@$) }
  | /* EMPTY */
  ;
partitionSegAttrs:
                          partitionSegAttr
  | partitionSegAttrs ',' partitionSegAttr
  ;
partitionSegAttr:
    PARTITION segmentAttrsClause { logSkipped(@$) }
  ;
                                           /*}}}*/
                                   /*}}}*/
alterMappingTableClauses:          /*{{{ <- scip */
    MAPPING TABLE allocateExtentClause { logSkipped(@$) }
  | MAPPING TABLE deallocateUnusedClause { logSkipped(@$) }
  ;
                                   /*}}}*/
alterConstraintBase:               /*{{{*/
    ADD o_refConstraint                          { $$ = new Sm::alter_table::AddRefConstraint(@$); } 
  | ADD '(' o_refConstraint ')'                  { $$ = new Sm::alter_table::AddRefConstraint(@$); } 
  | ADD '(' o_constraints ')'                    { $$ = new Sm::alter_table::AddConstraints(@$, $3); } 
  | ADD o_constraint                             { $$ = new Sm::alter_table::AddConstraints(@$, mkList($2)); }
  | MODIFY constraint                            { $$ = new Sm::alter_table::ModifyConstraint(@$, $2); }
  | MODIFY keyFields constraint                  { $$ = new Sm::alter_table::ModifyKey(@$, $2, $3); }
  | RENAME CONSTRAINT exprId TO exprId           { del($3, $5); $$ = 0; logSkipped(@$)  }
  | DROP CONSTRAINT exprId CASCADE_opt           { $$ = new Sm::alter_table::DropConstraint(@$, $3, $4); logSkipped(@$) }
  | DROP keyFields  CASCADE_opt keepDropIndexOpt { $$ = new Sm::alter_table::DropKey(@$, $2, $3, $4); logSkipped(@$) }
  ;
%type <alterConstraintBase> alterConstraintBase;

keyFields:                  /*{{{*/
    PRIMARY_KEY                 { $$ = new Sm::alter_table::KeyFields(@$); }
  | UNIQUE '(' columnIdList ')' { $$ = new Sm::alter_table::KeyFields(@$, $3); }
  ;
%type <alterTable_KeyFields> keyFields;
                                   /*}}}*/
CASCADE_opt:                       /*{{{*/
    CASCADE     { $$ = 1; }
  | /* EMPTY */ { $$ = 0; }
  ;
%type <intval> CASCADE_opt;
                                   /*}}}*/
                                   /*}}}*/
/* }}} */
/* ALTER USER      {{{1 */

alterUser:
    ALTER_USER columnIdList userFullSettings endSqlGrammarOpt { $$ = new Sm::AlterUser(@$, $2, $3); }
  ;
%type <alterUser> alterUser;

userFullSettings:        /*{{{*/                 
    userIdentifiedClause                         { $$ = new Sm::alter_user::UserSettings(@$, $1); }
  | DEFAULT_ROLE userRoleClause                  { $$ = new Sm::alter_user::UserSettings(@$, $2); logSkipped(@$) }
  | userSettingsClause /* <-skip */              { $$ = 0; logSkipped(@$) }
  | grantOrRevoke CONNECT userProxyConnectClause { $$ = new Sm::alter_user::UserSettings(@$, new Sm::alter_user::Connect(@$, $1, $3)); logSkipped(@$) }
  ;
%type <alterUser_UserSettings> userFullSettings;
                         /*}}}*/
grantOrRevoke:           /*{{{*/
    GRANT  { $$ = Sm::alter_user::connect::GRANT;  }
  | REVOKE { $$ = Sm::alter_user::connect::REVOKE; }
  ;
%type <grantOrRevoke> grantOrRevoke;
                         /*}}}*/
userRoleClause:          /*{{{*/
    /* EMPTY */                  { $$ = new Sm::alter_user::UserRoles(@$, Sm::alter_user::UserRoles::EMPTY); }
  | ALL                          { $$ = new Sm::alter_user::UserRoles(@$, Sm::alter_user::UserRoles::ALL); }
  | ALL EXCEPT userRoleCommaList { $$ = new Sm::alter_user::UserRoles(@$, Sm::alter_user::UserRoles::ALL_EXCEPT, $3); }
  | NONE                         { $$ = new Sm::alter_user::UserRoles(@$, Sm::alter_user::UserRoles::NONE); }
  | userRoleCommaList            { $$ = new Sm::alter_user::UserRoles(@$, Sm::alter_user::UserRoles::ROLES); del($1); }
  ;
%type <alterUser_UserRoles> userRoleClause;
                         /*}}}*/
userRoleCommaList:       /*{{{*/
                          userRoleItem { $$ = mkList($1); }
  | userRoleCommaList ',' userRoleItem { $$ = $1; $$->push_back($3); }
  ;
%type <alterUser_UserRoleList> userRoleCommaList;
                         /*}}}*/
userRoleItem:            /*{{{*/
    RawID   { $$ = new Sm::alter_user::UserRole(@$, $1, Sm::alter_user::UserRole::ID); }
  | CONNECT { $$ = new Sm::alter_user::UserRole(@$, 0 , Sm::alter_user::UserRole::CONNECT); }
  | DBA     { $$ = new Sm::alter_user::UserRole(@$, 0 , Sm::alter_user::UserRole::DBA); }
  ;
%type <alterUser_UserRole> userRoleItem;
                         /*}}}*/
userProxyConnectClause:  /*{{{*/
    THROUGH_ENTERPRISE_USERS                                                       { $$ = 0; }
  | THROUGH RawID /* db user proxy */ userProxyConnectRoles authenticationRequired { $$ = new Sm::alter_user::UserProxyConnect(@$, $2, $3, $4); }
  ;
%type <alterUser_UserProxyConnect> userProxyConnectClause;
                         /*}}}*/
authenticationRequired:  /*{{{*/
    AUTHENTICATION REQUIRED { $$ = true; }
  | /* EMPTY */             { $$ = false; }
  ;
%type <boolval> authenticationRequired;
                         /*}}}*/
userProxyConnectRoles:   /*{{{*/
    /* EMPTY */                    { $$ = 0; }
  | WITH_ROLE            rawIdList { $$ = $2; } /* role names list */
  | WITH_ROLE ALL EXCEPT rawIdList { $$ = $4; } /* role names list */
  | WITH NO_ROLES                  { $$ = 0; }
  ;
%type <idList> userProxyConnectRoles;
                         /*}}}*/
/* End of ALTER USER }}}1 */
/* CREATE TYPE     {{{1 */
oidOpt:
    /* EMPTY */ { $$ = 0;  }
  | OID RawID   { $$ = $2; }
  ;
%type <id> oidOpt;

objectTypeDefinition:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Object_, $1); $$ = 0; }
  | rawIdDotRawId AsIs objectMemberList semicolonOpt END { $$ = new Sm::Type::Object(@$, $1, $3); }
  ;
%type <type_ObjectConcrete> objectTypeDefinition;
objectTypeDeclaration:
    typeName WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Object_, $1); $$ = 0; }
  | typeName oidOpt invokerRightsClauseOpt baseObject javaExternalName '(' objectMemberList semicolonOpt ')' inheritanceClauseOpt
    { $$ = new Sm::Type::Object(@$, $1, $7, $2, $4, $3, $5); }
  | typeName oidOpt AsIs TABLE OF datatype NOT_NULL_opt indexByTypeOpt                    { $$ = new Sm::Type::NestedTable(@$, $1, $2, $6, $8, $7); }
  | typeName oidOpt AsIs varrayOrVaryingArray '(' NUMERIC_ID ')' OF datatype NOT_NULL_opt { $$ = new Sm::Type::Varray(@$, $1, $2, $6, $9, $10); }
  ;
%type <type_Object> objectTypeDeclaration;

typeDeclaration:
    TYPE userTypeId AS RECORD '(' recordFieldsDecl ')'               { $$ = new Sm::Type::Record($2, $6, @$); }
  | TYPE userTypeId IS_RECORD '(' recordFieldsDecl ')'               { $$ = new Sm::Type::Record($2, $5, @$); }
  | TYPE userTypeId IS_RECORD     recordFieldsDecl                   { $$ = new Sm::Type::Record($2, $4, @$); }
  | TYPE userTypeId AsIs REF CURSOR datatypeReferenceRefCursorOpt    { $$ = new Sm::Type::RefCursor(@$, $2, $6); }
  | TYPE objectTypeDeclaration                                       { $$ = $2; }       
  ;
%type <declaration> typeDeclaration;

baseObject:             /*{{{*/
    AsIs OBJECT                     { $$ = 0; } 
  | UNDER schemaDotColumn /* DML */ { $$ = $2; resolveObject(*context->model, $$); $$->loc(@$); }
  ;
%type <id2> baseObject;
                        /*}}}*/

memberSplitter:
    ','
  | ';'
  ;

objectMemberList:            /*{{{*/
                                    objectMember { $$ = mkBaseList($1);    }
  | objectMemberList memberSplitter objectMember { $$ = $1; $$->push_back($3); }
  ;
%type <objectMemberList> objectMemberList;
objectMember:
    memberFunction    { $$ = $1; }
  | memberVariable    { $$ = $1; }
  | pragmaDeclaration { $$ = $1; }
  ;
%type <objectMember> objectMember;

memberVariable:
    exprId datatype                      { $$ = new Sm::Type::MemberVariable(@$, $1, $2); }
  | exprId datatype EXTERNAL_NAME exprId { $$ = new Sm::Type::MemberVariable(@$, $1, $2, $4); }
  ;
%type <type_MemberVariable> memberVariable;

/*}}}*/
inheritanceClauseOpt: 
    /* EMPTY */            { $$ = Sm::Type::Inheritance::EMPTY;              } 
  |     OVERRIDING         { $$ = Sm::Type::Inheritance::OVERRIDING;         } 
  |     FINAL              { $$ = Sm::Type::Inheritance::FINAL;              } 
  |     INSTANTIABLE       { $$ = Sm::Type::Inheritance::INSTANTIABLE;       } 
  |     FINAL INSTANTIABLE { $$ = Sm::Type::Inheritance::FINAL_INSTANTIABLE; } 
  | NOT OVERRIDING         { $$ = Sm::Type::Inheritance::NOT_OVERRIDING;     } 
  | NOT FINAL              { $$ = Sm::Type::Inheritance::NOT_FINAL;          } 
  | NOT INSTANTIABLE       { $$ = Sm::Type::Inheritance::NOT_INSTANTIABLE;   } 
  ;
%type <inheritanceCathegory> inheritanceClauseOpt;

memberSpecificator:
    MEMBER       { $$ = Sm::Type::member_function::MEMBER; }
  | STATIC       { $$ = Sm::Type::member_function::STATIC; }
  | MAP   MEMBER { $$ = Sm::Type::member_function::MAP_MEMBER; }
  | ORDER MEMBER { $$ = Sm::Type::member_function::ORDER_MEMBER; }
  ;
%type <specificators> memberSpecificator;

memberFunction:            /*{{{*/
    memberFunctionContent  { $$ = $1; }
  | memberProcedureContent { $$ = $1; }
  | inheritanceClauseOpt CONSTRUCTOR_FUNCTION datatype constructorArglist constructorRetspec  { $$ = new Sm::Type::MemberFunction(@$, $1, $4, $3, $5, $3); }
  ;
%type <objectMember> memberFunction;
                           /*}}}*/
selfInOutDatatypeOpt:      /*{{{*/
    /* EMPTY */              { $$ = 0;  }
  | SELF IN OUT datatype ',' { $$ = $4; logSkipped(@$) }
  | SELF IN datatype     ',' { $$ = $3; logSkipped(@$) }
  ;
%type <datatype> selfInOutDatatypeOpt;
                           /*}}}*/
constructorArglist:        /*{{{*/
    '(' selfInOutDatatypeOpt functionArgListOpt ')' { $$ = $3; }
  | /* EMPTY */                                     { $$ = 0; }
  ;
%type <functionArgumentList> constructorArglist;
                           /*}}}*/
constructorRetspec:        /*{{{*/
    RETURN SELF AS RawID                 { del($4); $$ = 0; logSkipped(@$) } /* $4 = RESULT */
  | RETURN SELF AS RawID AsIs callSpec   { del($4); $$ = 0; logSkipped(@$) } /* $4 = RESULT */
  | RETURN SELF AS RawID AsIs blockPlSql { del($4); $$ = $6; } /* $4 = RESULT */
  ;                     
%type <blockPlSql> constructorRetspec;
                           /*}}}*/
invokerRightsClauseOpt:    /*{{{  */
    invokerRightsClause   { $$ = $1; logSkipped(@$) }
  | /* EMPTY */           { $$ = Sm::Type::auth_id::EMPTY; }
  ;
invokerRightsClause:  
    AUTHID CURRENT_USER { $$ = Sm::Type::auth_id::CURRENT_USER; }
  | AUTHID DEFINER      { $$ = Sm::Type::auth_id::DEFINER; }
  ;                        
%type <authId> invokerRightsClause invokerRightsClauseOpt; /*}}}*/
javaExternalName:          /*{{{*/
    EXTERNAL_NAME exprId LANGUAGE JAVA USING sqljModule { $$ = new Sm::Type::JavaExternalSpec(@$, $2, $6); logSkipped(@$) }
  | /* EMPTY */ { $$ = 0; }
  ;
%type <javaExternalSpec> javaExternalName;

sqljModule:
    SQLData     { $$ = Sm::Type::java_external_spec::SQLData;     }
  | CustomDatum { $$ = Sm::Type::java_external_spec::CustomDatum; }
  | OraData     { $$ = Sm::Type::java_external_spec::OraData;     }
  ;                        /*}}}*/
%type <javaExternalModule> sqljModule;

callSpec:               /*{{{*/
    LANGUAGE JAVA_NAME    exprId { del($3); logSkipped(@$) }
  | C_LIBRARY             exprId callSpecAgentIn withContext parametersList { del($2); logSkipped(@$) }
  | C_NAME exprId LIBRARY exprId callSpecAgentIn withContext parametersList { del($2, $4); logSkipped(@$) }
  ;
withContext:
    WITH_CONTEXT { logSkipped(@$) }
  | /* EMPTY */
  ;
parametersList:
    RawID '(' columnIdList ')' /* $1 = PARAMETERS */ { del($1, $3); logSkipped(@$) }
  | /* EMPTY */
  ;
callSpecAgentIn:
  | RawID /*<- AGENT*/ IN '(' columnIdList ')' { del($1, $4); logSkipped(@$) }
  ;                     /*}}}*/
indexByTypeOpt:         /*{{{*/
    INDEX BY datatype { $$ = $3;  $$->loc(@$); logSkipped(@$) }
  | /* EMPTY */       { $$ = 0; }
  ;
%type <datatype> indexByTypeOpt;
                        /*}}}*/
recordFieldsDecl:       /*{{{*/
                         recordFieldDecl { $$ = mkList($1); }
  | recordFieldsDecl ',' recordFieldDecl { $$ = $1; $$->push_back($3); }
  ;
%type <recordFieldDecls> recordFieldsDecl;

recordFieldId:
    columnId { $$ = $1; } 
  ;
%type <id> recordFieldId;

recordFieldDecl:
    recordFieldId datatype NULL_NOT_NULL_opt                           { $$ = new Sm::Type::RecordField(@$, $1, $2, 0 , $3); }
  | recordFieldId datatype NULL_NOT_NULL_opt AssignOrDefault condition { $$ = new Sm::Type::RecordField(@$, $1, $2, $5, $3); }
  ;
%type <recordFieldDecl> recordFieldDecl;
                        /*}}}*/


NULL_NOT_NULL_opt:              /*{{{*/
    NOT NULLk   { $$ = true; }
  |     NULLk   { $$ = false; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> NULL_NOT_NULL_opt;


varrayOrVaryingArray:   /*{{{*/
    VARRAY
  | VARYING ARRAY
  ;                     /*}}}*/
/* }}} */
/* PRAGMA          {{{*/

minusOpt: 
    /* EMPTY */ { $$ = false; }
  | '-'         { $$ = true;  }
  ;
%type <boolval> minusOpt;

pragmaDeclaration: 
    PRAGMA AUTONOMOUS_TRANSACTION ';'                       { $$ = new Sm::pragma::AutonomousTransaction(@$); }
  | PRAGMA RawID '(' exprId ',' minusOpt NUMERIC_ID ')' ';' 
    { 
      Sm::NumericValue *nv = $7;
      if ($6)
        nv->neg();
      if ($2->isKeyword(STRING("EXCEPTION_INIT")))
        $$ = new Sm::pragma::ExceptionInit(@$, $4, nv); 
      else
         YYERROR;
      del($2);
    }
  | PRAGMA RawID '(' exprId ',' RawID /* 'YES', 'NO' */ ')' ';'
    {
       if (!$2->isKeyword(STRING("INLINE")))
         YYERROR;
       if ($6->isSemanticString(STRING("YES")))
         $$ = new Sm::pragma::Inline(@$, $4, true);
       else if ($6->isSemanticString(STRING("NO")))
         $$ = new Sm::pragma::Inline(@$, $4, false);
       else
         YYERROR;
    }
  | PRAGMA RESTRICT_REFERENCES '(' exprIdExists  ',' pragmaSubprogramAsserts ')' ';' { $$ = new Sm::pragma::RestrictReferences(@$, $6, $4); }
  | PRAGMA RESTRICT_REFERENCES '(' DEFAULT ',' pragmaSubprogramAsserts ')' ';' { $$ = new Sm::pragma::RestrictReferences(@$, $6); }
  | PRAGMA SERIALLY_REUSABLE ';'                            { $$ = new Sm::pragma::SeriallyReusable(@$); }
  ;              
%type <pragma_Pragma> pragmaDeclaration;
                 
pragmaSubprogramAsserts:
                                pragmaSubprogramAssert { $$ = $1; }
  | pragmaSubprogramAsserts ',' pragmaSubprogramAssert { $$ = $1; $$.i |= $3.i; }                  
  ;

pragmaSubprogramAssert:
    RNDS  { $$.f = Sm::pragma::PragmaRestrictFlags::RNDS;  }
  | WNDS  { $$.f = Sm::pragma::PragmaRestrictFlags::WNDS;  }
  | RNPS  { $$.f = Sm::pragma::PragmaRestrictFlags::RNPS;  }
  | WNPS  { $$.f = Sm::pragma::PragmaRestrictFlags::WNPS;  }
  | TRUST { $$.f = Sm::pragma::PragmaRestrictFlags::TRUST; }
  ;
%type <restrictFlags> pragmaSubprogramAsserts pragmaSubprogramAssert;
                 /*}}}*/
/* PACKAGE         {{{1 */
/* http://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_6006.htm */

package:                   /*{{{*/
    createPackageKeyword rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Package_, $2); $$ = 0; }
  | createPackageKeyword rawIdDotRawId invokerRightsClauseOpt AsIs blockPlSqlPackage packageEnd { $$ = new Sm::Package(@$, $2, $5, $1); } 
  ;
%type <package> package;
 
createPackageKeyword:
    CREATE_PACKAGE      { $$ = false; }
  | CREATE_PACKAGE_BODY { $$ = true; }
  ;
%type <boolval> createPackageKeyword;

packageEnd:
  | END
  | END exprId { del($2); }
  ;
                           /*}}}*/
blockPlSqlPackage:         /*{{{*/
    declarationsOpt bodyOpt  { $$ = $2; $$->addDeclarations($1); $$->setLLoc(@$); }
  ;
%type <blockPlSql> blockPlSqlPackage;

bodyOpt: 
    /* EMPTY */ { $$ = new Sm::BlockPlSql(); }
  | body        { $$ = $1; $$->setLLoc(@$); }
  ;
%type <blockPlSql> bodyOpt;

                           /*}}}*/
blockPlSql:                /*{{{*/
                            body { $$ = $1; } 
  | DECLARE declarationsOpt body { $$ = $3; $$->addDeclarations($2); }
  | DECLARE declarationsOpt _DYN_BLOCK_TAIL '(' plExpr ')' END { $$ = new Sm::BlockPlSql(@$, $2, $5);  }
  ;
%type <blockPlSql> blockPlSql;


dynamicPlSqlIntoOpt:
    /* EMPTY */    { $$ = 0;  }
  | INTO sqlExprId { $$ = $2; }
  ;

%type <sqlExprId> dynamicPlSqlIntoOpt;

dynamicBlockPlSql:
    _DYN_PLSQL_BLOCK dynamicPlSqlIntoOpt blockPlSql { $$ = new Sm::ConstructBlockStmt(@$, $3, $2); }
  ;
%type <constructBlockStmt> dynamicBlockPlSql;

                            /*}}}*/
blockPlSql_or_CallSpec:    /*{{{*/
    callSpec             { $$ = 0; logSkipped(@$) }
  | declarationsOpt body { $$ = $2; $$->addDeclarations($1); }
  ;
%type <blockPlSql> blockPlSql_or_CallSpec;
                           /*}}}*/  

semicolonOpt:
  | ';' 
  ;
body:                      /*{{{*/
    BEGINk
      statementsOpt
      exceptionHandlersOpt
    END labelIdOpt { $$ = new Sm::BlockPlSql(@$, 0, $2, $3, $5); }
  ;
%type <blockPlSql> body;

bodyEndNoLabel:                /*{{{*/
    BEGINk
      statementsOpt
      exceptionHandlersOpt
    END { $$ = new Sm::BlockPlSql(@$, 0, $2, $3, 0); }
  ;
%type <blockPlSql> bodyEndNoLabel;


exceptionHandlersOpt:              /*{{{*/
    EXCEPTION exceptionHandlers { $$ = $2; }
  | /* EMPTY */                 { $$ = 0; }
  ;
%type <whenExprList> exceptionHandlersOpt;
exceptionHandlers:      
                      exceptionHandler { $$ = mkBaseList<Sm::StatementInterface>($1); }
  | exceptionHandlers exceptionHandler { $$ = $1; $$->push_back($2); }
  ;
%type <whenExprList> exceptionHandlers;
exceptionHandler:            
    WHEN plCond THEN statementsOpt { $$ = new Sm::WhenExpr(@$, $4, $2, Sm::WhenExpr::WHEN_THEN);  }
  | WHEN OTHERS THEN statementsOpt { $$ = new Sm::WhenExpr(@$, $4, NULL, Sm::WhenExpr::WHEN_ALL); }
  ;
%type <whenExpr> exceptionHandler;
                                   /*}}}*/
whenPlExprOpt:                     /*{{{*/
    WHEN plCond { $$ = $2;  $$->loc(@$); } 
  | /* EMPTY */ { $$ = 0; }
  ;
%type <plCond> whenPlExprOpt;
                                   /*}}}*/
                           /*}}}*/   
declarationsOpt:           /*{{{*/
    declarations { $$ = $1; }
  | /* EMPTY */  { $$ = 0; }
  ;
declarations:
                 declaration { $$ = mkList($1); }
  | declarations declaration { $$ = $1; if ($2) $$->push_back($2); }
  ;
%type <declarations> declarations declarationsOpt;
declaration:
    pragmaDeclaration { $$ = $1; }
    /* в зависимости от контекста это и переменная и курсор */
  | exprId          datatype NOT_NULL_opt                        ';' { $$ = new Sm::Variable($1, $2, $3, 0 , false, @$); }
  | exprId          datatype NOT_NULL_opt AssignOrDefault plCond ';' { $$ = new Sm::Variable($1, $2, $3, $5, false, @$); }
  | exprId CONSTANT datatype NOT_NULL_opt AssignOrDefault plCond ';' { $$ = new Sm::Variable($1, $3, $4, $6, true , @$); }
  | exprId EXCEPTION  ';' { $$ = new Sm::Exception(@$, $1); } 
  | typeDeclaration   ';' { $$ = $1; }
  | SUBTYPE exprId IS datatype constraintBrOpt NOT_NULL_opt ';' { $$  = new Sm::Subtype(@$, $2, $4, $5, $6); } 
  | functionSpec      ';' { $$ = $1; }
  | procedureSpec     ';' { $$ = $1; }
  | cursorDeclaration ';' { $$ = $1; }
  | RETURN datatype   ';' { $$ = 0; del($2); logSkipped(@$) }
  ;
%type <declaration> declaration;
constraintBrOpt:               /*{{{*/
    '(' constraint ')'         { $$ = $2;  $$->loc(@$); logSkipped(@$) }
  | RANGE sqlExpr ".." sqlExpr { $$ = 0; del($2,$4); logSkipped(@$) }
  | /* EMPTY */                { $$ = 0; }
  ;
%type <constraint> constraintBrOpt;
                               /*}}}*/
                           /*}}}*/   


createProcedureSpec:
    CREATE_PROCEDURE pushFunCtx procedureContent { $$ = $3; context->popFunCtx($$); }
  ;
%type <function> createProcedureSpec;

procedureSpec:
    PROCEDURE pushFunCtx procedureContent { $$ = $3; context->popFunCtx($$); }
  ;
%type <function> procedureSpec;
                          /*{{{*/


procedureContent:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Function_, $1); $$ = 0; }
  | rawIdDotRawId 
      functionArgListBrOpt 
      invokerDeterministicParallelListOpt
      procedureBodyOpt     { $$ = new Sm::Function($1, $2, 0, $4); $$->loc(@$); }
  ;
%type <function> procedureContent;

memberProcedureContent:
    inheritanceClauseOpt memberSpecificator PROCEDURE rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Function_, $4); $$ = 0; }
  | inheritanceClauseOpt    /*1*/
    memberSpecificator      /*2*/
    PROCEDURE rawIdDotRawId pushFunCtx /*5*/
      functionArgListBrOpt  /*6*/
      invokerDeterministicParallelListOpt /*7*/
      procedureBodyOpt      /*8*/
    {
      Sm::Type::MemberFunction *ptr;
      $$ = ptr = new Sm::Type::MemberFunction(@$, $1, $2, $4, $6, 0, $8);
      context->popFunCtx(ptr);
    }
  ;

%type <objectMember> memberProcedureContent;
    
procedureBodyOpt:              /*{{{*/
    /* EMPTY */                 { $$ = 0;  }
  | AsIs blockPlSql_or_CallSpec { $$ = $2; }
  | AsIs EXTERNAL               { $$ = 0; logSkipped(@$) }
  ;
%type <blockPlSql> procedureBodyOpt;    
                               /*}}}*/
                           /*}}}*/

createFunctionSpec:
    CREATE_FUNCTION pushFunCtx functionContent { $$ = $3; context->popFunCtx($$); } /* DDL(sql) TODO:(plsql) */  /*2*/
  ;
%type <function> createFunctionSpec;

functionSpec:
    FUNCTION pushFunCtx functionContent { $$ = $3; context->popFunCtx($$); } /* DDL(sql) TODO:(plsql) */  /*2*/
  ;
                           /*{{{*/

pushFunCtx:
    /*EMPTY*/ { context->pushFunCtx(); }
  ;

functionContent:   
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Function_, $1); $$ = 0; }
  | rawIdDotRawId /* DDL(sql) TODO:(plsql) */  /*1*/
      functionArgListBrOpt                /*2*/
      RETURN datatype                     /*4*/
      invokerDeterministicParallelListOpt /*5*/ /*<-scip*/
      functionAggregatePipelinedOpt       /*6*/
      functionBodyOpt                     /*7*/ { $$ = new Sm::Function($1, $2, $4, $7); $$->loc(@$); $$->setPipelined($6); }
  ;
%type <function> functionSpec functionContent;

memberFunctionContent:
    inheritanceClauseOpt memberSpecificator FUNCTION rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::MemberFunction_, $4); $$ = 0; }
  | inheritanceClauseOpt                  /*1*/
    memberSpecificator                    /*2*/
    FUNCTION rawIdDotRawId pushFunCtx     /*4*/
      functionArgListBrOpt                /*5*/
      RETURN datatype                     /*7*/
      invokerDeterministicParallelListOpt /*8*/
      functionAggregatePipelinedOpt       /*9*/
      functionBodyOpt                     /*10*/
    {
      Sm::Type::MemberFunction *ptr;
      $$ = ptr = new Sm::Type::MemberFunction(@$, $1, $2, $4, $6, $8, $11);
      ptr->setPipelined($10);
      context->popFunCtx(ptr);
    }
  ;

%type <objectMember> memberFunctionContent;

functionArgList:                        /*{{{*/
                        functionArgument { $$ = new Sm::FunArgList($1); }
  | functionArgList ',' functionArgument { $$ = $1; $$->push_back($3); }
  ;
functionArgListBrOpt:            
    /* EMPTY */                  { $$ = 0; }
  | '(' functionArgListOpt ')'   { $$ = $2; }
  ;
functionArgListOpt:
    /* EMPTY */                  { $$ = 0; }
  | functionArgList              { $$ = $1; }
  ;
%type <functionArgumentList> functionArgListBrOpt functionArgList functionArgListOpt;

functionArgument:
    exprId inOutSpecOpt datatype defaultExprOpt { $$ = new Sm::FunctionArgument(@$, $1, $3, $4, $2); }
  ;
%type <functionArgument> functionArgument;
inOutSpecOpt:                                 /*{{{*/
    IN            { $$ = Sm::function_argument::IN;     }
  | OUT           { $$ = Sm::function_argument::OUT;    }
  | IN OUT        { $$ = Sm::function_argument::IN_OUT; }
  | OUT    NOCOPY { $$ = Sm::function_argument::OUT;    }
  | IN OUT NOCOPY { $$ = Sm::function_argument::IN_OUT; }
  | /* EMPTY */   { $$ = Sm::function_argument::IN;  }
  ;
%type <argumentDirection> inOutSpecOpt;
                                              /*}}}*/
                                          /*}}}*/
invokerDeterministicParallelListOpt:    /*{{{ - scip*/
    invokerDeterministicParallelList   { logSkipped(@$) }                               /*<-scip*/
  | /* EMPTY */                                                       /*<-scip*/ 
  ;
invokerDeterministicParallelList:                                     /*<-scip*/ 
                                     invokerDeterministicParallelItem /*<-scip*/ 
  | invokerDeterministicParallelList invokerDeterministicParallelItem /*<-scip*/ 
  ;
invokerDeterministicParallelItem:                                     /*<-scip*/ 
    invokerRightsClause                                               /*<-scip*/ { logSkipped(@$) }
  | DETERMINISTIC                                                     /*<-scip*/ { logSkipped(@$) }
  | PARALLEL_ENABLE                                                   /*<-scip*/ { logSkipped(@$) }
  | PARALLEL_ENABLE parallelSpec streamingClauseOpt                   /*<-scip*/ { logSkipped(@$) }
  ;
parallelSpec:                                                         /*<-scip*/ 
    exprId                                                                       { logSkipped(@$) }
  | exprId '(' PARTITION exprId BY parallelPartitionFormOpt ')'       /*<-scip*/ { del($1, $4); logSkipped(@$) }
  ;
streamingClauseOpt:                                                   /*<-scip*/ 
    CLUSTER RawID BY '(' columnIdList ')'                             /*<-scip*/ { del($2,$5); logSkipped(@$) }
  | ORDER   RawID BY '(' columnIdList ')'                             /*<-scip*/ { del($2,$5); logSkipped(@$) }
  | /* EMPTY */                                                       /*<-scip*/ 
  ;
parallelPartitionFormOpt:                                             /*<-scip*/ 
    ANY                                                               /*<-scip*/ { logSkipped(@$) }
  | RANGE '(' columnIdList ')'                                        /*<-scip*/ { del($3); logSkipped(@$) }
  | HASH  '(' columnIdList ')'                                        /*<-scip*/ { del($3); logSkipped(@$) }
  | /* EMPTY */
  ;
                                     /*}}}*/
functionBodyOpt:                        /*{{{*/
    USING datatype              { $$ = 0; del($2); }
  | AsIs blockPlSql_or_CallSpec { $$ = $2; }
  | /* EMPTY */                 { $$ = 0; }
  ;
%type <blockPlSql> functionBodyOpt;   

functionAggregatePipelinedOpt:                
    /* EMPTY */                           { $$ = 0; }
  | AGGREGATE                             { $$ = 0; logSkipped(@$) }
  | PIPELINED                             { $$ = FLAG_FUNCTION_PIPELINED; }
  ;
%type <intval> functionAggregatePipelinedOpt;
                                        /*}}}*/
                           /*}}}*/
NOT_NULL_opt:              /*{{{*/
    NOT NULLk   { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> NOT_NULL_opt;
                           /*}}}*/
AssignOrDefault:           /*{{{ semantic = "default" */
    ":="
  | DEFAULT
  ;                        /*}}}*/
label:                     /*{{{*/
    "<<" labelId ">>" { $$ = new Sm::Label($2); }
  ;
%type <label> label ;
labelId:
    RawID   { $$ = $1; $$->loc(@$); }
  | varSpec { $$ = $1; $$->loc(@$); }
  | EXISTS  { $$ = new Sm::Identificator(STRING("EXISTS"), @1); }
  | MODIFY  { $$ = new Sm::Identificator(STRING("MODIFY"), @1); }

  ;
labelIdOpt:
    labelId     { $$ = $1;  $$->loc(@$); } 
  | /* EMPTY */ { $$ = 0; }
  ;
%type <id> labelId labelIdOpt;

labelList:
                  label { $$ = new Sm::BaseList<Sm::StatementInterface>($1); }
  | labelList ',' label { $$ = $1; $$->push_back($3); }
  ;
%type <statementsList> labelList;
                           /*}}}*/
cursorDeclaration:         /*{{{*/
    CURSOR exprId cursorParametersBrOpt returnDatatypeReferenceOpt isSelectStmtOpt { $$ = new Sm::Cursor(@$, $2, $3, $4, $5); }
  ;
%type <cursorDecl> cursorDeclaration;

cursorParameters:              /*{{{*/
                         cursorParameter { $$ = mkList($1); }
  | cursorParameters ',' cursorParameter { $$ = $1; $$->push_back($3); }
  ;
%type <cursorParameters> cursorParameters cursorParametersBrOpt;
cursorParametersBrOpt:
    '(' cursorParameters ')' { $$ = $2; }
  | /* EMPTY */              { $$ = 0; }
  ;
   
cursorParameter:           
    exprId IN_opt datatype                        { $$ = new Sm::CursorParameter(@$, $1, $2, $3); }
  | exprId IN_opt datatype AssignOrDefault plCond { $$ = new Sm::CursorParameter(@$, $1, $2, $3, $5); }
  ;
%type <cursorParameter> cursorParameter;
                                       
IN_opt:                                
    IN          { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> IN_opt; 
                               /*}}}*/
returnDatatypeReferenceOpt:    /*{{{*/
    RETURN datatypeReference { $$ = $2; $$->loc(@$); }
  | /* EMPTY */              { $$ = 0; }
  ;
%type <datatype> returnDatatypeReferenceOpt;
                               /*}}}*/
isSelectStmtOpt:               /*{{{*/
    /* EMPTY */        { $$ = 0; }
  | IS selectStatement { $$ = $2; }        
  ;
%type <subquery> isSelectStmtOpt;               
                               /*}}}*/
                           /*}}}*/

statement:                 /*{{{*/
              statementSpec ';' { $$ = new Sm::BaseList<Sm::StatementInterface>($1); }
  | labelList statementSpec ';' { $$ = $1; $$->push_back($2); }
  | bodyEndNoLabel              { $$ = new Sm::BaseList<Sm::StatementInterface>($1); }
  ;

%type <statementsList> statement;

statementsOpt:                     /*{{{*/
    statementsList       { $$ = $1; }
  | /* EMPTY */          { $$ = 0; }
  ;
statementsList:
                   statement { $$ = $1; }
  | statementsList statement { $$ = $1; $$->splice($$->end(), *($2)); delete $2; }
  ;
%type <statementsList> statementsOpt statementsList;
                                   /*}}}*/

selectedEntityRefExpr:
    selectedEntity       { $$ = new Sm::RefExpr(@$, $1->toIdEntitySmart()); }
  ;
%type <refExpr> selectedEntityRefExpr;

statementSpec:                     /*{{{*/
                                           /*{{{*/
    NULLk                 { $$ = new Sm::NullStatement(@$); }
  | pragmaDeclaration     { $$ = $1; }
  | assignmentStmt        { $$ = $1; }
  | blockPlSql            { $$ = $1; }
  | dynamicBlockPlSql     { $$ = $1; }
  | caseStmt              { $$ = $1; }
  | closeStmt             { $$ = $1; }
  | executeImmediateStmt  { $$ = $1; }
  | exitStmt              { $$ = $1; }
  | fetchStmt             { $$ = $1; }
  | forallStmt            { $$ = $1; }
  | forOfExprStmt         { $$ = $1; }
  | selectedEntityRefExpr { $$ = new Sm::FunctionCall(@$, $1); }
  | dynTrCallSignature    { $$ = new Sm::FunctionCall(@$, $1); }
  | dblinkSelectedEntity  { $$ = new Sm::FunctionCall(@$, $1); }
  | gotoStmt              { $$ = $1; }
  | ifStmt                { $$ = $1; }
  | loopStmt              { $$ = $1; }
  | openCursorStmt        { $$ = $1; }
  | openForStmt           { $$ = $1; }
  | decltypeCursorStmt    { $$ = $1; }
  | declNamespaceStmt     { $$ = $1; }
  | constructExprStmt     { $$ = $1; }
  | pipeRowStmt           { $$ = $1; }
  | raiseStmt             { $$ = $1; }
  | returnStmt            { $$ = $1; }
  | sqlStatement          { $$ = $1; }
  | whileStmt             { $$ = $1; }
  /*| dynPlsqlStmt          { $$ = $1; } */
  ;
%type <statement> statementSpec;
                                           /*}}}*/
 /*
    dynPlsqlStmt:
       _DYN_PLSQL_STATEMENT '(' plExpr ')' { $$ = ???; }
     ;
    %type <statement> dynPlsqlStmt;
 */

assignmentStmt:                            /*{{{*/
    assignmentStatementLvalue ":=" plCond { $$ = new Sm::Assignment(@$, $1, $3); }
  ;
%type <assignment> assignmentStmt;

assignmentStatementLvalue:      
    selectedEntity     { $$ = new Sm::LValue(@$, $1->toIdEntitySmart()); }
  | ':' selectedEntity { $$ = new Sm::LValue(@$, $2->toIdEntitySmart(), 1); }
  ;
%type <assignmentLValue> assignmentStatementLvalue;

                                           /*}}}*/
caseStmt:                                  /*{{{*/
    CASE        caseWhenExpressions caseElseOpt END CASE                          { $$ = new Sm::CaseStatement(@$, 0,  $2, $3, 0 ); }
  | CASE plCond caseWhenExpressions caseElseOpt END CASE                          { $$ = new Sm::CaseStatement(@$, $2, $3, $4, 0 ); }
  | CASE        caseWhenExpressions caseElseOpt END CASE RawID /* <- labelName */ { $$ = new Sm::CaseStatement(@$, 0,  $2, $3, $6); }
  | CASE plCond caseWhenExpressions caseElseOpt END CASE RawID /* <- labelName */ { $$ = new Sm::CaseStatement(@$, $2, $3, $4, $7); }
  ;
%type <caseStatement> caseStmt;

caseWhenExpressions:         
                        caseWhenExpression { $$ = mkBaseList<Sm::StatementInterface>($1); }
  | caseWhenExpressions caseWhenExpression { $$ = $1; $$->push_back($2); }
  ;
%type <whenExprList> caseWhenExpressions;
caseWhenExpression:          
    WHEN plCond THEN statementsOpt { $$ = new Sm::WhenExpr(@$, $4, $2, Sm::WhenExpr::WHEN_THEN);  }
  ;
%type <whenExpr> caseWhenExpression;
caseElseOpt:
    ELSE statementsOpt  { $$ = new Sm::WhenExpr(@$, $2, NULL, Sm::WhenExpr::WHEN_OTHERS); }
  | /*EMPTY*/           { $$ = 0; }
  ;
%type <whenExpr> caseElseOpt;
                                           /*}}}*/
closeStmt:                                 /*{{{*/
    CLOSE cursorEntityName { $$ = new Sm::Close(@$, $2); }
  ;
%type <close_stmt> closeStmt;
                                           /*}}}*/
executeImmediateStmt:                      /*{{{*/
    EXECUTE IMMEDIATE plCond                      %dprec 2 { $$ = new Sm::ExecuteImmediate(@$); $$->execExpr = $3; }
  | EXECUTE IMMEDIATE plCond executeImmediateBody %dprec 1 { $$ = $4; $$->execExpr = $3; $$->loc(@$); }
  ;
%type <executeImmediate> executeImmediateStmt;


executeImmediateBody: 
    dynamicInto                                                          { $$ = new Sm::ExecuteImmediate(@$,  0, 0, $1); /*4*/ }
  | dynamicInto using_execImmediateUsingArguments                        { $$ = new Sm::ExecuteImmediate(@$, $2, 0, $1); /*4*/ }
  |             using_execImmediateUsingArguments                        { $$ = new Sm::ExecuteImmediate(@$, $1,  0); /*3*/ }
  |             using_execImmediateUsingArguments dynamicReturningClause { $$ = new Sm::ExecuteImmediate(@$, $1, $2); /*3*/ }
  | dynamicReturningClause                                               { $$ = new Sm::ExecuteImmediate(@$,  0, $1); /*2*/ }
  ;
%type <executeImmediate> executeImmediateBody;

bulkCollectList:
                        bulkCollectItem { $$ = new Sm::IntoCollections($1->toIdEntitySmart()); }
  | bulkCollectList ',' bulkCollectItem { $$ = $1; $$->push_back(new Sm::RefExpr($3->toIdEntitySmart())); }
  ;
%type <intoCollectionList> bulkCollectList;

bulkCollectItem: 
        selectedEntity { $$ = $1; }
  | ':' selectedEntity { $$ = $2; $$->majorEntity()->setHostId(); }
  ;
%type <idEntity> bulkCollectItem;

using_execImmediateUsingArguments:
    USING execImmediateUsingArguments { $$ = $2; }
  ;
%type <argumentNames> using_execImmediateUsingArguments;

execImmediateUsingArguments:
                                    execImmediateArgument { $$ = mkList($1); }
  | execImmediateUsingArguments ',' execImmediateArgument { $$ = $1; $$->push_back($3); }
  ;
%type <argumentNames> execImmediateUsingArguments ;
                                               /*}}}*/
execImmediateArgument:                         /*{{{*/
  /*  IN     selectedEntity { $$ = new Sm::ArgumentNameRef(@$, $2->toIdEntitySmart(), Sm::ArgumentNameRef::IN    ); } */
    OUT    selectedEntity { $$ = new Sm::ArgumentNameRef(@$, $2->toIdEntitySmart(), Sm::ArgumentNameRef::OUT   ); }
  | IN OUT selectedEntity { $$ = new Sm::ArgumentNameRef(@$, $3->toIdEntitySmart(), Sm::ArgumentNameRef::IN_OUT); }
  |        sqlExpr     { $$ = new Sm::ArgumentNameRef(@$, $1, Sm::ArgumentNameRef::IN    ); }
  | IN     sqlExpr     { $$ = new Sm::ArgumentNameRef(@$, $2, Sm::ArgumentNameRef::IN    ); }
  ;
%type <argumentName> execImmediateArgument;
                                               /*}}}*/

returningReturn:
    RETURNING
  | RETURN   
  ;

dynamicReturningClause:
    returningReturn dynamicInto { $$ = $2; }
  ;

dynamicInto: 
                 INTO bulkCollectList { $$ = $2; }
  | BULK COLLECT INTO bulkCollectList { $$ = $4; $$->setBulkCollect(true); }
  ;

%type <intoCollectionList> dynamicInto dynamicReturningClause;
                                           /*}}}*/
exitStmt:                                  /*{{{*/
    EXIT       whenPlExprOpt { $$ = new Sm::Exit(@$, $2);      } 
  | EXIT RawID whenPlExprOpt { $$ = new Sm::Exit(@$, $3, $2);  } 
  ;
%type <exit_stmt> exitStmt;
                                           /*}}}*/
fetchStmt:                                 /*{{{*/
    FETCH cursorEntityName              INTO selectedEntityList { $$ = new Sm::Fetch(@$, $2, $4); }
  | FETCH cursorEntityName BULK COLLECT INTO selectedEntityList                               {          $$ = new Sm::Fetch(@$, $2, $6, Sm::Fetch::COLLECTION); }
  | FETCH cursorEntityName BULK COLLECT INTO selectedEntityList RawID plCond /* $7 = LIMIT */ { del($7); $$ = new Sm::Fetch(@$, $2, $6, Sm::Fetch::COLLECTION, $8); }
  ;
%type <fetch> fetchStmt;
cursorEntityName:                              /*{{{*/
        exprId { $$ = $1; $$->loc(@$); }
  | ':' exprId { $$ = $2; $$->setHostId(); $$->loc(@$); }
  ;
%type <id> cursorEntityName;
/* {{{ cursor | cursor_variable | :host_cursor_variable
cursor

Имя открытого явного курсора.
Чтобы открыть явный курсор, используйте "OPEN Statement".  При попытке выборки
из явного курсора до его открытия или после закрытия, PL/SQL генерирует
предопределенное исключение INVALID_CURSOR.

cursor_variable

Имя открытой курсорной переменной.
Чтобы открыть курсорную переменную, используйте "OPEN FOR Statement".
Курсорная переменная может быть формальным параметром подпрограммы (см. "Cursor
Variables as Subprogram Parameters"). При попытке выборки из курсорной
переменной до её открытия или после закрытия, PL/SQL генерирует
предопределенное исключение INVALID_CURSOR.

:host_cursor_variable

Имя курсорной переменной, объявленной в хост-окружении PL/SQL внешняя среда,
передаваемой в PL/SQL как связанная переменная, а затем открытой.
Чтобы открыть хост-курсорную переменную, используйте "OPEN FOR Statement".
Не помещайте пробела между двоеточием (:) и host_cursor_variable.

Тип данных хост-курсорной переменной является совместимым с возвращаемым типом
любой курсорной переменной PL/SQL. 
 }}} */
                                               /*}}}*/
                                           /*}}}*/
forallStmt:                                /*{{{*/
    FORALL exprId /* indexName */
        IN sqlExpr ".." sqlExpr sqlStatement { $$ = new Sm::ForAll(@$, $2, new Sm::LoopBounds(@4, $4, $6), $7); }
  ;
%type <forAll> forallStmt;
/* index_name
Необъявленный идентификатор на который можно ссылаться только в предложении
FORALL и только как субиндекс коллекции.

Неявное объявление index_name переопределяет любое другое объявление вне цикла.
Вы не можете обратиться к другой переменной с тем же именем внутри предложения.
Внутри предложения FORALL, index_name не может появляться в выражениях и ему не
может быть присвоено значение.
*/


                                           /*}}}*/

forOfStmtHeader:
    FOR exprId IN { $$ = $2; }
  ;
%type <id> forOfStmtHeader;


forOfTail:
    sqlExpr              loopStmt { $$ = new Sm::ForOfExpression(@$, $1, $2); }
  | sqlExpr ".." sqlExpr loopStmt { $$ = new Sm::ForOfRange(@$, new Sm::LoopBounds(@1+@3, $1, $3), $4); }

%type <statement> forOfTail;

forOfExprStmt:                             /* {{{ */
    forOfStmtHeader         forOfTail %dprec 1 { $$ = $2; $$->setIndexVariable(@$, $1, false); }
  | forOfStmtHeader REVERSE forOfTail %dprec 2 { $$ = $3; $$->setIndexVariable(@$, $1, true); }
  ;
%type <statement> forOfExprStmt;
                                          
                                           /* }}} */
gotoStmt:                                  /*{{{*/
    GOTO labelId { $$ = new Sm::Goto(@$, $2); }
  ;
%type <goto_stmt> gotoStmt;                                                            
                                           /*}}}*/
ifStmt:                                    /*{{{*/
    IF plCond THEN statementsOpt elseIfStmtsOpt elseStmtOpt END IF { $$ = new Sm::If(@$, new Sm::WhenExpr($4, $2, Sm::WhenExpr::IF_FIRST_STATEMENT), $5, $6); }
  ;
%type <if_stmt> ifStmt;
elseIfStmtsOpt:                
    elseIfStmts            { $$ = $1; }
  | /* EMPTY */            { $$ = 0; }
  ;
elseIfStmts:
                elseIfStmt { $$ = mkBaseList<Sm::StatementInterface>($1); }
  | elseIfStmts elseIfStmt { $$ = $1; $$->push_back($2); }
  ;
%type <whenExprList> elseIfStmts elseIfStmtsOpt;
elseIfStmt:                    
    ELSIF plCond THEN statementsOpt { $$ = new Sm::WhenExpr(@$, $4, $2, Sm::WhenExpr::ELSEIF_STATEMENT);  }
  ;
%type <whenExpr> elseIfStmt;
elseStmtOpt:                   
    ELSE statementsOpt  { $$ = new Sm::WhenExpr(@$, $2, NULL, Sm::WhenExpr::ELSE_STATEMENT); }
  | /* EMPTY */         { $$ = 0; }
  ;
%type <whenExpr> elseStmtOpt;
                                           /*}}}*/
loopStmt:                                  /*{{{*/
    LOOP statementsOpt END LOOP labelIdOpt { $$ = new Sm::Loop(@$, $2, $5);  }
  ;
%type <loop> loopStmt;
                                           /*}}}*/
openCursorStmt:                            /*{{{*/
    OPEN exprId                                    { $$ = new Sm::OpenCursor(@$, $2); }
  | OPEN exprId '(' functionCallArglist ')'        { $$ = new Sm::OpenCursor(@$, $2, $4); }
  ;
%type <openCursor> openCursorStmt;
                                           /*}}}*/

openForHead:
    OPEN cursorVariableName FOR cursorVariableSelect { $$ = new Sm::OpenFor(@$, $2, $4, 0); }
  ;

openForStmt:                               /*{{{*/
    openForHead                                   { $$ = $1; }
  | openForHead USING execImmediateUsingArguments { $$ = $1; $1->bindArguments = $3; }
  ;
%type <openFor> openForHead openForStmt;

decltypeCursorStmt:
     _DECLTYPE_CURSOR  cursorVariableName FOR sqlExpr  { $$ = new Sm::CursorDecltype(@$, $2, $4); }
   ;
%type <statement> decltypeCursorStmt;

declNamespaceStmt:
     _DECL_NAMESPACE  exprId FROM fromBody { $$ = new Sm::DeclNamespace(@$, $2, $4); }
   | _DECL_NAMESPACE  exprId DECLARE declarations END { $$ = new Sm::DeclNamespace(@$, $2, $4); }
   | _DECL_NAMESPACE  exprId { $$ = new Sm::DeclNamespace(@$, $2, 0); }
   ;
%type <statement> declNamespaceStmt;


AND_OR_dyn_compound:
    AND { $$ = 0; }
  | OR  { $$ = 1; }
  ;
%type <intval> AND_OR_dyn_compound;

constructExpr_expression:
    condition                     { $$ = $1; }
  | AND_OR_dyn_compound condition { $$ = new Sm::pl_expr::LogicalCompound(@$, new Sm::BoolTailObj(@1), $2, $1); }
  | condition AND_OR_dyn_compound { $$ = new Sm::pl_expr::LogicalCompound(@$, $1, new Sm::BoolTailObj(@2), $2); }
  | WHERE condition               { $$ = new Sm::DynWhereClause(@$, $2); }
  ;

%type <plCond> constructExpr_expression;

constructExprStmt:
     _CONSTRUCT_EXPR     '{' constructExprId constructListOpt '}'                     constructExpr_expression  { $$ = new Sm::ConstructExprStmt(/*loc*/@$, /*var*/$3, /*nspace*/0, /*expr*/$6, /*ctx*/$4, /*procMode*/false);  }
   | _CONSTRUCT_EXPR     '{' constructExprId ',' constructExprId constructListOpt '}' constructExpr_expression  { $$ = new Sm::ConstructExprStmt(/*loc*/@$, /*var*/$3, /*nspace*/$5, /*expr*/$8, /*ctx*/$6, /*procMode*/false); }
   | _CONSTRUCT_PL_EXPR  '{' constructExprId constructListOpt '}'                     constructExpr_expression  { $$ = new Sm::ConstructExprStmt(/*loc*/@$, /*var*/$3, /*nspace*/0, /*expr*/$6, /*ctx*/$4, /*procMode*/true);   }
   | _CONSTRUCT_PL_EXPR  '{' constructExprId ',' constructExprId constructListOpt '}' constructExpr_expression  { $$ = new Sm::ConstructExprStmt(/*loc*/@$, /*var*/$3, /*nspace*/$5, /*expr*/$8, /*ctx*/$6, /*procMode*/true);  }
   ;
%type <constructExprStmt> constructExprStmt;


constructListOpt:
     /*EMPTY*/                 { $$ = 0; }
   | ',' constructListConcrete { $$ = $2; }
   ;

constructListConcrete:
   { context->constrExprStmtCtx.push(new Sm::ConstructExprStmtContext()); }
     constructList { $$ = context->constrExprStmtCtx.top(); context->constrExprStmtCtx.pop(); }
   ;

constructList:
                       constructListItem
   | constructList ',' constructListItem
   ;
%type <constructExprStmtContext> constructListConcrete constructListOpt;

constructListItem:
     CONCAT                      { context->constrExprStmtCtx.top()->concat = true; }
   | SAVE_SUBQUERY '(' RawID ')' { context->constrExprStmtCtx.top()->nameOfUnionChunck = $3->toNormalizedString(); del($3); }
   | GLOBAL_CURSOR '(' RawID ')' { context->constrExprStmtCtx.top()->globalCursorId    = $3->toNormalizedString(); del($3); }
   ;

cursorVariableSelect:
    sqlExpr            { $$ = $1; } /* TODO: строковое выражение */
  ;
%type <sqlExpr> cursorVariableSelect;

cursorVariableName:                            /*{{{*/
/*
  Курсорная переменная или параметр (без возвращаемого типа) ранее объявленная
  внутри текущего пространства имен. 
*/
        exprId { $$ = $1; $$->loc(@$); }
/*
  Курсорная переменная, ранее объявленная в хост-окружении PL/SQL и переданная
  в PL/SQL как связанная переменная. Тип данных хост-курсорной переменной
  является совместимым с возвращаемым типом любой курсорной переменной PL/SQL.
  Хост переменные должны быть префиксированы двоеточием.
*/
  | ':' exprId { $$ = $2; $$->setHostId(); $$->loc(@$); }
  ;

%type <id> cursorVariableName;
                                               /*}}}*/
                                           /*}}}*/
pipeRowStmt:                               /*{{{*/
    PIPE ROW '(' selectedEntity /* row */ ')' { $$ = new Sm::PipeRow(@$, $4->toIdEntitySmart()); }
  ;
/* 
Row (элемент таблицы), который функция возвращает ее вызывающему,
представленному выражением тип которого - это тип табличного элемента.

Если выражение - переменная записи (record variable), 
то она должна быть явно объявлена с типом данных табличного элемента.
Она не может быть объявлена с типом данных, который только структурно идентичен
типу элемента.  Например, если у типа элемента есть имя, то переменная записи
не может быть объявлена явно с %TYPE или %ROWTYPE или неявно с %ROWTYPE в
курсорном операторе FOR LOOP.
*/
%type <pipeRow> pipeRowStmt;
                                           /*}}}*/
raiseStmt:                                 /*{{{*/
      RAISE                { $$ = new Sm::Resignal(@$); }
    | RAISE selectedEntity { $$ = new Sm::Raise(@$, $2->toIdEntitySmart()); }
  ;
%type <statement> raiseStmt;
                                           /*}}}*/
returnStmt:                                /*{{{*/
      RETURN        { $$ = new Sm::Return(@$); }
    | RETURN plCond { $$ = new Sm::Return(@$, $2); }
  ;
%type <return_stmt> returnStmt;
                                           /*}}}*/
whileStmt:                                 /*{{{*/
    WHILE plCond loopStmt { $$ = new Sm::While(@$, $2, $3); }
  ;
%type <while_stmt> whileStmt;
                                           /*}}}*/
                                   /*}}}*/



sqlStatement:                      /*{{{*/
    commitStatement          { $$ = $1; } 
  | deleteStatement          { $$ = $1; } 
  | insertStatement          { $$ = $1; } 
  | updateStatement          { $$ = $1; } 
  | mergeStatement           { $$ = $1; }
  | lockTableStatement       { $$ = $1; } 
  | rollbackStatement        { $$ = $1; } 
  | savepointStatement       { $$ = $1; } 
  | selectStatement          { $$ = new Sm::SelectStatement(@$, $1); } 
  | setTransactionStatement  { $$ = $1; } 
  ;
%type <sqlStatement> sqlStatement;
commitStatement:                           /*{{{*/
    COMMIT commitCommentOpt { $$ = new Sm::Commit(@$, $2); }
  ;
%type <commit> commitStatement;
commitCommentOpt:                   
    COMMENT exprId { $$ = $2; $$->loc(@$); logSkipped(@$) }
  | /* EMPTY */    { $$ = 0; }
  ;
%type <id> commitCommentOpt;
                                           /*}}}*/
deleteStatement:                           /*{{{*/
    DELETE deleteStatementTail { $$ = $2; }
  | DELETE_HINT optimizeHints deleteStatementTail { $$ = $3; $$->hints = $2; }
  ;
%type <deleteFrom> deleteStatement;

deleteStatementTail:
    FROM_opt sqlStatementDmlEntity aliasOpt sqlStatementWhereClauseOpt returningClauseOpt { $$ = new Sm::DeleteFrom(@$, $1, $2, $3, $4, $5); }
  ;
%type <deleteFrom> deleteStatementTail;


sqlStatementDmlEntity:                         /*{{{*/
     dmlTableExpressionClause              { $$ = $1; $$->hasOnly = false; $$->loc(@$); }
   | ONLY '(' dmlTableExpressionClause ')' { $$ = $3; $$->hasOnly = true;  $$->loc(@$); logSkipped(@$) }
   ;
%type <sqlEntity> sqlStatementDmlEntity;


dynTable:
    _DYN_TABLE '(' sqlExpr ',' schemaDotColumn ')'             { $$ = new Sm::QueryEntityDyn(@$, $3, $5); }
  | _DYN_TABLE '(' sqlExpr                     ')'             { $$ = new Sm::QueryEntityDyn(@$, $3, 0); }
  ;
%type <queryEntityDyn> dynTable;

dmlTableExpressionClause:
    schemaDotColumn partitionExtensionClauseOpt                { $$ = new Sm::ChangedQueryEntityRef(@$, $1); }
  | schemaDotColumn '@' exprId                                 { $$ = new Sm::ChangedQueryEntityRef(@$, $1, $3); }
  | '(' subquery subqueryRestrictionClauseOpt ')'              { $$ = new Sm::ChangedQueryRestricted(@$, $2, $3); }
  | TABLE '(' sqlExpr /* <- collection expr */ ')' plusJoinOpt { $$ = new Sm::ChangedQueryTableCollectionExpr(@$, $3, $5); }
  | dynTable                                                   { $$ = $1; }
  ;
/* {{{ tableCollectionExpression позволяет сообщить Oracle, что значение 
collection_expression должно рассматриваться как таблица для запросов и DML
операций. collection_expression может быть подзапросом, столбцом, функцией или 
конструктором коллекции. Независимо от его формы оно должно возвратить
значение коллекции — т.е. значение, тип которого - вложенная таблица или
varray.

Этот процесс извлечения элементов коллекции называется раскруткой коллеции
(collection unnesting).  Дополнительный плюс (+) является актуальным, если вы
соединяете выражение TABLE collection с родительской таблицей. 
+ Создает внешнее соединение из двух, так что запрос возвращает строки из
внешней таблицы, даже если выражение коллекции пустое.
}}} */
%type <sqlEntity> dmlTableExpressionClause;

plusJoinOpt:
    /* EMPTY */ { $$ = false; }
  | '(' '+' ')' { $$ = true;  logSkipped(@$) }
  ;
%type <boolval> plusJoinOpt;
                                               /*}}}*/
FROM_opt:                                      /*{{{*/
    FROM        { $$ = 1; }
  | /* EMPTY */ { $$ = 0; }
  ;
%type <intval> FROM_opt;
                                               /*}}}*/
                                           /*}}}*/
insertStatement:                           /*{{{*/
    INSERT singleTableInsert { $$ = $2; $$->loc(@$); }
  | INSERT multiTableInsert  { $$ = $2; $$->loc(@$); }
  ;
%type <insert> insertStatement;
singleTableInsert:                             /*{{{*/
    insertIntoClause insertFrom errorLoggingClauseOpt { $$ = new Sm::insert::SingleInsert(@$, $1, $2); }
  ;
%type <insert_SingleInsert> singleTableInsert;

insertIntoClause:                                  /*{{{*/
    INTO dmlTableExpressionClause aliasOpt insertIntoSqlExprListBrOpt { $$ = new Sm::insert::Into(@$, $2, $3, $4);  }
  ;
%type <insert_Into> insertIntoClause;

insertIntoFieldExpr:
    sqlExpr { $$ = $1; }
  | USING   { $$ = new Sm::RefExpr(@$, new Sm::Identificator(STRING("USING"), @1)); }
  ;
%type <sqlExpr> insertIntoFieldExpr;

insertIntoSqlExprList:
                              insertIntoFieldExpr                      { $$ = mkList($1); }
  | insertIntoSqlExprList ',' insertIntoFieldExpr                      { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprList> insertIntoSqlExprList;


insertIntoSqlExprListBrOpt:
    /* EMPTY */                   { $$ = 0; }
  | '(' insertIntoSqlExprList ')' { $$ = $2; }
  ;
%type <sqlExprList> insertIntoSqlExprListBrOpt;


insertFrom:
    insertingValues returningClauseOpt { $$ = new Sm::insert::InsertFromValues(@$, $1, $2); }
  | subquery                           { $$ = new Sm::insert::InsertFromSubquery(@$, $1); }
  ;
%type <insertFrom> insertFrom;

insertingValues:
    VALUES '(' exprOrDefaultList ')'                { $$ = new Sm::insert::InsertingExpressionListValues(@$, $3); }
  | VALUES selectedEntity /* <- record (plsql extension) */ { $$ = new Sm::insert::InsertingRecordValues(@$, $2->toIdEntitySmart()); }
  ;
%type <insertingValues> insertingValues;
exprOrDefaultList:
                          exprOrDefault { $$ = mkList($1); }
  | exprOrDefaultList ',' exprOrDefault { $$ = $1; $$->push_back($3); }
  ; 
%type <sqlExprList> exprOrDefaultList;
exprOrDefault:
    sqlExpr { $$ = $1; }
  | DEFAULT { $$ = new Sm::DefaultExpr(@$); }
  ;
%type <sqlExpr> exprOrDefault;
                                                   /*}}}*/
                                               /*}}}*/
multiTableInsert:                              /*{{{*/
     ALL valuesInsertList    subquery { $$ = new Sm::insert::MultipleValuesInsert(@$, $2, $3);  }
   | insertConditional subquery { $$ = new Sm::insert::MultipleConditionalInsert(@$, $1, $2);  }
   ;
%type <insert> multiTableInsert;

valuesInsertList:                                  /*{{{*/
                      valuesInsert { $$ = mkList($1); }
   | valuesInsertList valuesInsert { $$ = $1; $$->push_back($2); }
   ;
%type <insert_InsertValuesList> valuesInsertList;

valuesInsert:
    insertIntoClause insertingValuesOpt errorLoggingClauseOpt { $$ = new Sm::insert::InsertValues(@$, $1, $2);   }
  ;
%type <insert_InsertValues> valuesInsert;

insertingValuesOpt:
    /* EMPTY */     { $$ = 0; }
  | insertingValues { $$ = $1; }
  ;
%type <insertingValues> insertingValuesOpt;
                                                   /*}}}*/
insertConditional:                                 /*{{{*/
    AllOrFirstOpt 
    whenThenValuesInsertList
    elseValuesInsertOpt  { $$ = new Sm::insert::conditional_insert::InsertConditional(@$, $1, $2, $3);  }
  ;
%type <insertConditional> insertConditional;

AllOrFirstOpt:                                         /*{{{*/
    /* EMPTY */ { $$ = Sm::insert::conditional_insert::EMPTY; }
  | ALL         { $$ = Sm::insert::conditional_insert::ALL; }
  | RawID { 
      if ($1->isKeyword(STRING("FIRST"))) 
        $$ = Sm::insert::conditional_insert::FIRST;
      else {
        cout << "Token FIRST expected, but " << $1->toQString() << " found " << endl;
        YYERROR;
      }  
    } /* <- FIRST */
  ;
%type <allOrFirst> AllOrFirstOpt;
                                                       /*}}}*/
whenThenValuesInsertList:                              /*{{{*/
                              whenThenValuesInsert { $$ = mkList($1); }
  | whenThenValuesInsertList  whenThenValuesInsert { $$ = $1; $$->push_back($2); }
  ;
%type <insertWhenThenList> whenThenValuesInsertList;
whenThenValuesInsert:
    WHEN condition THEN valuesInsertList { $$ = new Sm::insert::conditional_insert::InsertWhenThen(@$, $2, $4); }
  ;
%type <insertWhenThen> whenThenValuesInsert;
                                                       /*}}}*/
elseValuesInsertOpt:                                   /*{{{*/
    /* EMPTY */           { $$ = 0; }
  | ELSE valuesInsertList { $$ = $2;  }
  ;
%type <insert_InsertValuesList> elseValuesInsertOpt;
                                                       /*}}}*/
                                                   /*}}}*/      
errorLoggingClauseOpt:                             /*{{{*/ /* skip */
    /* EMPTY */
  | errorLoggingClause
  ;

errorLoggingClause:
    LOG_ERRORS intoLogOpt simpleExprOpt  rejectLimitOpt { logSkipped(@$) }
  ;
/* {{{
simple_expression 

Определяет значение, которое будет использоваться в качестве тега предложения,
так что можно будет определить ошибки от этого предложения в таблице журнала
ошибок. 

Выражение может быть текстовым литералом, числовым литералом или общим
SQL-выражением, таким как связанная переменная.

Можно также использовать функциональное выражение, если преобразовать его в
текстовый литерал — например, TO_CHAR (SYSDATE).
   }}} */

intoLogOpt:
    /* EMPTY */
  | INTO schemaDotColumn { del($2); logSkipped(@$) }
  ;

/* simple_expression: определяет значение, которое будет использоваться в
 * качестве тега оператора, так, чтобы можно было идентифицировать
 * ошибки от этого оператора в таблице логирования ошибок.  Выражение может
 * быть либо текстовым литералом, числовым литералом или общим SQL-выражением,
 * таким как привязка переменной (bind variable). Можно также использовать
 * функциональное выражение, если преобразовать его в текстовый литерал —
 * например, TO_CHAR (SYSDATE).
 */
simpleExpression:
     sqlExprNonSubquery { logSkipped(@$) }
   ;

simpleExprOpt:
    /* EMPTY */
  | simpleExpression { logSkipped(@$) }
  ;
rejectLimitOpt:                    
    /* EMPTY */
  | REJECT_LIMIT INTVAL { logSkipped(@$) }
  | REJECT_LIMIT RawID /* <- UNLIMITED */ { del($2); logSkipped(@$) }
  ;                                                /*}}}*/          
                                               /*}}}*/
                                           /*}}}*/
updateStatement:                           /*{{{*/
    UPDATE updateStmtTail                     { $$ = $2; }
  | UPDATE_HINT optimizeHints updateStmtTail  { $$ = $3; $$->hints = $2; }
  ;
%type <update> updateStatement;


updateStmtTail:
    sqlStatementDmlEntity aliasOpt
       SET updateSetClause
    sqlStatementWhereClauseOpt
    returningClauseOpt
    errorLoggingClauseOpt { $$ = new Sm::Update(@$, $1, $2, $4, $5, $6);  }
  ;
%type <update> updateStmtTail;



updateSetClause:                                   /*{{{*/
    ROW exprId     /* <- record - pl/sql extension */ { $$ = new Sm::update::SetRowRecord(@$, $2);  }
  | ROW '=' exprId /* <- record - pl/sql extension */ { $$ = new Sm::update::SetRowRecord(@$, $3);  }
  | RawID          /* VALUE                        */ '(' selectedEntity ')' '=' sqlExpr {
      if ( $1->isKeyword(STRING("VALUE")) )
        $$ = new Sm::update::SetUpdatingList(@$,
          mkList( static_cast<Sm::update::SetUpdatingList::UpdatingItemType*>(new Sm::update::FieldFromExpr(@$, $3->toIdEntitySmart(), $6)) ));
      else 
        YYERROR;
    }
  | updatingItems   { $$ = new Sm::update::SetUpdatingList(@$, $1); }
  ;
%type <updateSetClause> updateSetClause;
                                                   /*}}}*/
updatingItems:                                     /*{{{*/
                      updatingItem { $$ = mkList($1); }
  | updatingItems ',' updatingItem { $$ = $1; $$->push_back($3); }
  ;
%type <updateSetItemList> updatingItems;
updatingItem:
    selectedEntity             '=' sqlExpr          { $$ = new Sm::update::FieldFromExpr(@$, $1->toIdEntitySmart(), $3); }
  | selectedEntity             '=' DEFAULT          { $$ = new Sm::update::FieldFromExpr(@$, $1->toIdEntitySmart(), new Sm::DefaultExpr(@3)); }
  | '(' selectedEntityList ')' '=' '(' subquery ')' { $$ = new Sm::update::FieldsFromSubquery(@$, $2, $6); }
  | dynFieldGrammar            '=' sqlExpr          { $$ = new Sm::update::FieldFromExpr(@$, $1, $3); }
  | dynFieldSingle                                  { $$ = $1; }
  ;
%type <plCond> updatingItem;
                                                   /*}}}*/
sqlStatementWhereClauseOpt:                        /*{{{*/
    WHERE condition                                 { $$ = new Sm::WhereClause(@$, $2); }
/* WHERE CURRENT OF cursor_name
Этот оператор обновляет строку, которая была только что выбрана. {{{

Относится к последней строке, обрабатываемой связанным с указанным курсором
оператором FETCH. 
Курсор должен быть FOR UPDATE, должен быть открытым и
установлен на строку. Если курсор не открыт, пункт CURRENT OF приводит к ошибке.
Если курсор открыт, но не было выбрано никаких строк, или последняя выборка не
возвратила строк, PL/SQL генерирует предопределенное исключение NO_DATA_FOUND.

Можно использовать предложение UPDATE WHERE CURRENT OF  после выборки из
открытого курсора (включая выборку выполненную для курсорного FOR loop), если
связанный запрос - FOR UPDATE.

Неявный курсор SQL и курсорные аттрибуты NOTFOUND%, FOUND%, ROWCOUNT% и ISOPEN%
позволяют получать доступ к полезной информации о выполнении предложения UPDATE%. 

}}} */
  | WHERE CURRENT OF exprId /* for update cursor */ { $$ = new Sm::WhereClause(@$, $4); logSkipped(@$) } /* plsql extension */
  | /*EMPTY*/                                       { $$ = 0; }
  ;
%type <whereClause> sqlStatementWhereClauseOpt;
                                                   /*}}}*/
returningClauseOpt:                                /*{{{*/
    returningClause                                { $$ = $1; } 
  | /* EMPTY */                                    { $$ = 0; }
  ;
returningClause:
    returningReturn sqlExprList dynamicInto        { $$ = new Sm::ReturnInto(@$, $2, $3); }
  ;
%type <returnInto> returningClause returningClauseOpt;
                                                   /*}}}*/
bulkCollectOpt:                                    /*{{{ - skip */
    BULK COLLECT									{ $$ = true; }
  | /* EMPTY */										{ $$ = false; }
  ;                                                /*}}}*/
%type <boolval> bulkCollectOpt;
                                           /*}}}*/
mergeStatement:                            /*{{{*/
    MERGE                       /* 1*/
          INTO                  /* 2*/
      fromBodyItem              /* 3*/
    USING                       /* 4*/
      fromBodyItem              /* 5*/
      ON                        /* 6*/
        '('                     /* 7*/
            condition           /* 8*/
        ')'                     /* 9*/
    mergeUpdateClauseOpt        /*10*/
    mergeInsertClauseOpt        /*11*/
    errorLoggingClauseOpt       /*12*/
    {
      $$ = new Sm::Merge(@$, $3, $5, $8, $10, $11);
    }
  ;
%type <merge> mergeStatement;

mergeUpdateClauseOpt:                          /*{{{*/
    /* EMPTY */                                                                           { $$ = 0; }
  | WHEN_MATCHED_THEN_UPDATE_SET columnAssignmentList whereClauseOpt deleteWhereClauseOpt { $$ = new Sm::MergeUpdate(@$, $2, $3, $4); }
  ;
%type <mergeMatchedUpdate> mergeUpdateClauseOpt;

deleteWhereClauseOpt:
    /* EMPTY */        { $$ = 0;  }
  | DELETE whereClause { $$ = $2; }
  ;
%type <plCond> deleteWhereClauseOpt;

columnAssignmentList:
                             columnAssignmentItem { $$ = mkList($1); }
  | columnAssignmentList ',' columnAssignmentItem { $$ = $1; $$->push_back($3); }
  ;
%type <fieldAssignmentList> columnAssignmentList;

columnAssignmentItem: 
    selectedEntity '=' sqlExpr { $$ = new Sm::MergeFieldAssignment(@$, $1->toIdEntitySmart(), $3); }
  ;
%type <fieldAssignment> columnAssignmentItem;
                                               /*}}}*/
mergeInsertClauseOpt:
    /* EMPTY */ { $$ = 0; }
  | WHEN_NOT_MATCHED_THEN_INSERT brColumnListOpt VALUES '(' sqlExprList ')' whereClauseOpt { $$ = new Sm::MergeInsert(@$, $2, $5, $7); }
  ;
%type <mergeNotMatchedInsert> mergeInsertClauseOpt;
                                           /*}}}*/

lockTableStatement:                        /*{{{*/
    LOCK TABLE schemaDotEntityList IN lockMode MODE waitMode { $$ = new Sm::LockTable(@$, $3, $5, $7);  }
  ;
%type <lockTable> lockTableStatement;

waitMode:                  
    WAIT                { $$ = Sm::lock_table::WAIT;   }
  | NOWAIT              { $$ = Sm::lock_table::NOWAIT; }
  | /* EMPTY */         { $$ = Sm::lock_table::EMPTY;  }
  ;
%type <waitMode> waitMode;
lockMode:      
    ROW SHARE           { $$ = Sm::lock_table::ROW_SHARE;           }
  | ROW EXCLUSIVE       { $$ = Sm::lock_table::ROW_EXCLUSIVE;       }
  | SHARE UPDATE        { $$ = Sm::lock_table::SHARE_UPDATE;        }
  | SHARE               { $$ = Sm::lock_table::SHARE;               }
  | SHARE ROW EXCLUSIVE { $$ = Sm::lock_table::SHARE_ROW_EXCLUSIVE; }
  | EXCLUSIVE           { $$ = Sm::lock_table::EXCLUSIVE;           }
  ;
%type <lockMode> lockMode;
                                           /*}}}*/
rollbackStatement:                         /*{{{*/
       ROLLBACK                     { $$ = new Sm::Rollback(@$);     }
  |    ROLLBACK TO           exprId { $$ = new Sm::Rollback(@$, $3); } 
  |    ROLLBACK TO SAVEPOINT exprId { $$ = new Sm::Rollback(@$, $4); } 
  ;
%type <rollback>  rollbackStatement;
                                           /*}}}*/
savepointStatement:                        /*{{{*/
       SAVEPOINT exprId { $$ = new Sm::Savepoint(@$, $2); }
  ;
%type <savepoint> savepointStatement;
                                           /*}}}*/
setTransactionStatement:                   /*{{{*/
    SET TRANSACTION transactionType transactionNameOpt { $$ = new Sm::Transaction(@$, $3, $4);  }
  ;
%type <transaction> setTransactionStatement;
transactionType:
    READ                 ONLY         { $$ = Sm::transaction::READ_ONLY;                     }
  | READ_WRITE                        { $$ = Sm::transaction::READ_WRITE;                    }
  | ISOLATION LEVEL      SERIALIZABLE { $$ = Sm::transaction::ISOLATION_LEVEL_SERIALIZABLE;  }
  | ISOLATION LEVEL READ COMMITED     { $$ = Sm::transaction::ISOLATION_LEVEL_READ_COMMITED; }
  | USE_ROLLBACK_SEGMENT exprId       { $$ = Sm::transaction::USE_ROLLBACK_SEGMENT; del($2); }
  ;
%type <transactionType> transactionType;
transactionNameOpt:
    RawID exprId /* $1 == "NAME" */     { if ($1->isKeyword(STRING("NAME"))) $$ = $2; else YYERROR; } 
  | /*EMPTY*/                           { $$ = 0; }
  ;
%type <id> transactionNameOpt;
                                           /*}}}*/
                                   /*}}}*/
                           /*}}}*/
/* end of PACKAGE }}}1 */
/* CREATE VIEW     {{{1 */
createView:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::View_, $1); $$ = 0; }
  | rawIdDotRawId  /* DDL */ 
    viewPropertiesOpt
    AS selectStatement
    subqueryRestrictionClauseOpt { $$ = new Sm::View(@$, $1, $2, $4, $5); }
  ;
%type <view> createView;

viewPropertiesOpt:                   /*{{{*/
    /* EMPTY */                                                                            { $$ = 0; }
  | OF schemaDotColumn WITH OBJECT_IDENTIFIER      DEFAULT         '(' viewConstraints ')' { $$ = new Sm::view::ObjectReference(@$, $2, $7, true); logSkipped(@$) }
  | OF schemaDotColumn WITH OBJECT_IDENTIFIER '(' columnIdList ')' '(' viewConstraints ')' { $$ = new Sm::view::ObjectReference(@$, $2, $6, $9); logSkipped(@$) }
  | OF schemaDotColumn UNDER schemaDotColumn /* superview */       '(' viewConstraints ')' { $$ = new Sm::view::ObjectReference(@$, $2, $4, $6); logSkipped(@$) }
  | OF XMLTYPE xmlScemaSpecOpt WITH OBJECT_IDENTIFIER DEFAULT                              { $$ = new Sm::view::XmlReference(@$, $3, true); logSkipped(@$) }
  | OF XMLTYPE xmlScemaSpecOpt WITH OBJECT_IDENTIFIER '(' sqlExprList ')'                  { $$ = new Sm::view::XmlReference(@$, $3, $7); logSkipped(@$) }
  | '(' viewConstraints ')'                                                                { $$ = new Sm::view::ViewConstraints(@$, $2); logSkipped(@$) }
  ;
%type <view_ViewProperties> viewPropertiesOpt;
                                 /*}}}*/
viewConstraints:                 /*{{{*/
                        viewConstraint { $$ = mkList($1); }
  | viewConstraints ',' viewConstraint { $$ = $1; $$->push_back($3); }
  ;
%type <view_ViewConstraints> viewConstraints;
                                 /*}}}*/
viewConstraint:              /*{{{*/
    viewConstraintColumnId /*<-alias*/ i_constraintsOpt { $$ = new Sm::view::ViewConstraint(@$, $1, $2); } 
  |                                    o_constraint     { $$ = new Sm::view::ViewConstraint(@$, mkList($1)); }
  ;

viewConstraintColumnId:
    columnId  { $$ = $1; }
  | REFERENCE { $$ = new Sm::Identificator(STRING("REFERENCE"), @1); }
  | AT        { $$ = new Sm::Identificator(STRING("AT"), @1); }
  ;
%type <id> viewConstraintColumnId;




%type <view_ViewConstraint> viewConstraint;
                                 /*}}}*/
subqueryRestrictionClauseOpt:    /*{{{*/
    /* EMPTY */                       { $$ = 0; }
  | WITH READ ONLY                    { $$ = new Sm::view::ViewQRestriction(@$, Sm::view::ViewQRestriction::WITH_READ_ONLY); logSkipped(@$) }
  | WITH CHECK OPTION                 { $$ = new Sm::view::ViewQRestriction(@$, Sm::view::ViewQRestriction::WITH_CHECK_OPTION); logSkipped(@$) }
  | WITH CHECK OPTION CONSTANT exprId { $$ = new Sm::view::ViewQRestriction(@$, $5); logSkipped(@$) }
  ;
%type <view_ViewQRestriction> subqueryRestrictionClauseOpt;
                                 /*}}}*/
xmlScemaSpecOpt:                 /*{{{*/
    XMLSCHEMA RawID ELEMENT exprId  { $$ = new Sm::view::XmlSchemaId(@$, $2, $4); }
  |                 ELEMENT exprId  { $$ = new Sm::view::XmlSchemaId(@$, $2); }
  | /* EMPTY */                     { $$ = 0; }
  ;                              /*}}}*/
%type <xmlReference> xmlScemaSpecOpt;
  /* 1}}} END OF CREATE VIEW */
/* CREATE USER     {{{1 */

createUser:
    RawID createUserBody  { context->model->addUser($1, $2); }
  ;

createUserBody:
    userIdentifiedClause userSettingsOpt { $$ = $1; }
  ;
%type <id> createUserBody;

userIdentifiedClause:
    IDENTIFIED_BY            RawID    { $$ = $2; }
  | IDENTIFIED_EXTERNALLY             { $$ = 0; logSkipped(@$) }
  | IDENTIFIED_EXTERNALLY AS exprId   { $$ = $3; } 
  | IDENTIFIED_GLOBALLY               { $$ = 0; logSkipped(@$)  }
  | IDENTIFIED_GLOBALLY_AS   EMPTY_ID { $$ = 0; logSkipped(@$)  }
  | IDENTIFIED_GLOBALLY_AS   exprId   { $$ = $2; } 
  ;
%type <id> userIdentifiedClause;

userSettingsOpt:
    /* EMPTY */
  | userSettings { logSkipped(@$) }/* skip */
  ;

userSettings:
    userSettingsClause
  | userSettingsClause userSettings
  ;

userSettingsClause:
    DEFAULT   tablespaceClause
  | TEMPORARY_TABLESPACE tablespaceClause
  | QUOTA     UNLIMITED_ON RawID  { del($3); }
  | QUOTA     NUMERIC_ID   RawID
              /*<- sizeClause::= <число> [KMGTPE] */ ON exprId { del($2, $3, $5); }
  | PROFILE   RawID               { del($2); }
  | PROFILE   DEFAULT
  | PASSWORD_EXPIRE
  | ACCOUNT_LOCK
  | ACCOUNT_UNLOCK
  ;

/* end of create user }}} */
/* CREATE TABLE    {{{1 */
/* Полезный контент                  {{{*/

physTablePropsOpt:
     { $$ = 0; }
   | tablePropertiesOpt physicalProperties tablePropertiesOpt 
     { 
       $$ = new cl::semantic_type::PhysicalTableProps;
       $$->tableProp = ($1 ? $1 : $3);
       if ($1 && $3)
         delete $3;
       $$->physical  = $2;
     }

%type <physicalTableProps> physTablePropsOpt;

    /*  physicalPropertyOpt         4
      tablePropertiesOpt            5 { $$ = new Sm::Table(@$, $1, $4, $5, $3, $2); } */

tableBody:                                 /*{{{*/
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Table_, $1); $$ = 0; }
  | rawIdDotRawId  /* DDL */             /* реляционная таблица  */
      brRelationPropertiesOpt      /* 2*/
      onCommitRowsOpt              /* 3*/
      physTablePropsOpt            { $$ = new Sm::Table(@$, $1, $4 ? $4->physical : 0, $4 ? $4->tableProp : 0, $3, $2); if ($4) delete $4; }
  | rawIdDotRawId  /* DDL */              /* объектная таблица    */
      OF schemaDotColumn /* DDL */ /* 3*/   /* TODO: резолвить и найти имя объекта */
      objectTableSubstitutionOpt   /* 4 skip */
      brObjectPropertiesOpt        /* 5*/
      onCommitRowsOpt              /* 6*/
      oidClauseOpt                 /* 7 skip */
      oidIndexClauseOpt            /* 8*/
      physicalPropertyOpt          /* 9*/
      tablePropertiesOpt           /*10*/ { $$ = new Sm::Table(@$, $1, $9, $10, $6, $3, $5, $8); }
  | rawIdDotRawId /* DDL */               /* таблица из xml схемы */
      OF XMLTYPE                   /* 3*/ 
      brObjectPropertiesOpt        /* 4*/
      xmltypeStorageOpt            /* 5 skip*/
      xmlschemaSpecOpt             /* 6*/
      /*xmltypeVirtualColumnsOpt*/ /*  */
      onCommitRowsOpt              /* 7*/
      oidClauseOpt                 /* 8 skip */
      oidIndexClauseOpt            /* 9*/
      physicalPropertyOpt          /*10*/
      tablePropertiesOpt           /*11*/ { $$ = new Sm::Table(@$, $1, $10, $11, $7, 0, $4, $9, $6); }
  ;
%type <table> tableBody;
                                           /*}}}*/
tablePropertiesOpt:                        /*{{{*/
    segmentCreationOpt
    fieldPropertiesOpt                 /*1*/
    tablePartioningClausesOpt /*scip*/ /*2*/
    cacheNocacheOpt                    /*3*/
    parallelClauseOpt         /*scip*/ /*4*/
    monitoringClauseOpt
    rowdependenciesOpt                 /*6*/
    enableDisableConstraintListOpt     /*7*/
    /* flashbackArchiveClauseOpt */    /* */
    asSubqueryOpt                      /*8*/ { $$ = new Sm::table::TableProperties(@$, $2, $8, $9, $4, $7); }
  ;
%type <table_TableProperties> tablePropertiesOpt;

segmentCreationOpt:

  | SEGMENT CREATION DEFERRED   { logSkipped(@$) }
  | SEGMENT CREATION IMMEDIATE  { logSkipped(@$) }
  | SEGMENT CREATION RawID      { logSkipped(@$) }
  ;
                                           /*}}}*/

brRelationPropertiesOpt:                   /*{{{*/
    '(' relationProperties ')' { $$ = $2; }
  | /* EMPTY */                { $$ = 0; }
  ;
relationProperties:
                           relationProperty { $$ = mkList($1); }
  | relationProperties ',' relationProperty { $$ = $1; $$->push_back($3); }
  ;
%type <table_FieldDefinitionList> relationProperties brRelationPropertiesOpt;


defFieldId:
    RawID             { $$ = $1; }
  | varSpecAlterField { $$ = $1; }
  | CONSTRAINT        { $$ = new Sm::Identificator(STRING("CONSTRAINT"), @1); }
  | LOGGING           { $$ = new Sm::Identificator(STRING("LOGGING"   ), @1); }
  | FUNCTION          { $$ = new Sm::Identificator(STRING("FUNCTION"  ), @1); }
  | HIERARCHY         { $$ = new Sm::Identificator(STRING("HIERARCHY" ), @1); }
  | PIPELINED         { $$ = new Sm::Identificator(STRING("PIPELINED" ), @1); }
  | HASH              { $$ = new Sm::Identificator(STRING("HASH"      ), @1); }
  | PARALLEL          { $$ = new Sm::Identificator(STRING("PARALLEL"  ), @1); }
  | AGGREGATE         { $$ = new Sm::Identificator(STRING("AGGREGATE" ), @1); }
  | NESTED            { $$ = new Sm::Identificator(STRING("NESTED"    ), @1); }
  | RELY              { $$ = new Sm::Identificator(STRING("RELY"      ), @1); }
  ;
%type <id> defFieldId;


columnDefinition:
    defFieldId datatype GENERATED_ALWAYS_opt AS '(' sqlExpr ')' virtualOpt /* <- VIRTUAL */ i_constraintsOpt
    { /* 1        2             3             4  5    6      7     8                          9 */
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $6, @$, 0, false, $8), $9);
    }
  | defFieldId datatype SORT_opt defaultExprOpt encryptSpecOpt inlineAndIrefConstraintOpt
    { /* 1       2        3          4             5                6 */
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $4, @$, $5, $3), $6);
    }
  ;
%type <table_FieldDefinition> columnDefinition;

columnDefinitionAdd:
    alterFieldId datatype GENERATED_ALWAYS_opt AS '(' sqlExpr ')' virtualOpt /* <- VIRTUAL */ i_constraintsOpt
    {   /* 1        2             3             4  5    6      7     8                          9 */
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $6, @$, 0, false, $8), $9);
    }
  | alterFieldId datatype SORT_opt defaultExprOpt encryptSpecOpt inlineAndIrefConstraintOpt
    { /* 1       2        3          4             5                6 */
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $4, @$, $5, $3), $6);
    }
  ;
%type <table_FieldDefinition> columnDefinitionAdd;

alterFieldId:
    RawID             { $$ = $1; }
  | varSpecAlterField { $$ = $1; }
  ;
%type <id> alterFieldId;

singleAlterColumnDefinition:
    alterFieldId datatypeOpt GENERATED_ALWAYS_opt AS '(' sqlExpr ')' virtualOpt /* <- VIRTUAL */ i_constraintsOpt
    {
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $6, @$, 0, false, $8), $9);
    }
  | alterFieldId datatypeOpt SORT_opt defaultExprOpt encryptSpecOpt inlineNoNameAndIrefConstraintOpt
    {
      $$ = new Sm::ParsingStageTableField(new Sm::table::FieldDefinition($1, $2, $4, @$, $5, $3), $6);
    }
  ;

%type <table_FieldDefinition> singleAlterColumnDefinition;

virtualOpt:
    RawID       
    {
      if ($1->isKeyword(STRING("VIRTUAL")))
        $$ = true;
      else
        YYERROR;
      logSkipped(@$)
    }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> virtualOpt;

GENERATED_ALWAYS_opt:
    GENERATED_ALWAYS { logSkipped(@$) }
  | /* EMPTY */
  ;

datatypeOpt:
    datatype    { $$ = $1; }
  | /* EMPTY */ { $$ = 0; }
  ;
%type <datatype> datatypeOpt;

relationProperty:
    columnDefinition %dprec 1 { $$ = $1; }
  | outConstraint    %dprec 2
    {
      $$ = new Sm::ParsingStageTableField(0, mkList($1));
    }
  | supplementalLoggingProps { $$ = 0; logSkipped(@$) }
  ;
%type <table_FieldDefinition> relationProperty;
                                           /*}}}*/
fieldProperty:                            /*{{{*/
    COLUMN exprId substitutableFieldPropertyOpt              { $$ = new Sm::table::field_property::ObjectField(@$, $2, $3); logSkipped(@$) }
  | NESTED TABLE nestedName substitutableFieldPropertyOpt STORE AS exprId 
    nestedPropertiesBrOpt ReturnAsLocatorOrValueOpt          { $$ = new Sm::table::field_property::NestedTable(@$, $3, $4, $7, $8, $9); logSkipped(@$) }
  | XMLTYPE COLUMN_opt exprId xmltypeColumnPropertiesTailOpt { $$ = new Sm::table::field_property::XmlField(@$, $3, $4); logSkipped(@$)  }
  | varrayOrLob lobPartitionListBrOpt                        { $$ = $1; if ($$) $$->loc(@$); logSkipped(@$) }
  ;
%type <tableFieldProperty> fieldProperty;

substitutableFieldPropertyOpt:
    substitutableFieldProperty { $$ = $1;  logSkipped(@$)}
  | /* EMPTY */               { $$ = 0; }
  ;

varrayOrLob:
    varrayColProperties { $$ = $1;  logSkipped(@$) }
  | lobStorageClause    { $$ = 0;   logSkipped(@$) } /* scip */
  ;
%type <tableFieldProperty_VarrayField> varrayOrLob;
                                           /*}}}*/
nestedPropertiesBrOpt:                     /*{{{*/
    '(' nestedTableProperties ')' { $$ = $2; logSkipped(@$) }
  | /* EMPTY */                   { $$ = 0; }
  ;
nestedTableProperties:
                          nestedTableProperty { $$ = mkList($1); }
  | nestedTableProperties nestedTableProperty { $$ = $1; $$->push_back($2); }
  ;
%type <tableFieldProperties> nestedTableProperties nestedPropertiesBrOpt;
                                           /*}}}*/
nestedTableProperty:                       /*{{{*/
    '(' objectProperties ')' { $$ = $2; $$->loc(@$); logSkipped(@$) }
  | physicalProperty         { $$ = $1; logSkipped(@$) }
  | fieldProperty            { $$ = $1; logSkipped(@$) }
  ;
%type <tableFieldProperty> nestedTableProperty;
                                           /*}}}*/
brObjectPropertiesOpt:                     /*{{{*/
    '(' objectProperties ')' { $$ = $2; $$->loc(@$); logSkipped(@$) }
  | /* EMPTY */              { $$ = 0; }
  ;
objectProperties:
    exprId             inlineAndIrefConstraintOpt { $$ = new Sm::table::field_property::ObjectProperties(@$, $1, $2); logSkipped(@$) }
  | exprId defaultExpr inlineAndIrefConstraintOpt { $$ = new Sm::table::field_property::ObjectProperties(@$, $1, $2, $3); logSkipped(@$) }
  | outConstraint                                 { $$ = new Sm::table::field_property::ObjectProperties(@$, mkList($1)); logSkipped(@$) }
  | supplementalLoggingProps                      { $$ = 0; logSkipped(@$) }
  ;                                                                       
%type <table_ObjectProperties> objectProperties brObjectPropertiesOpt;    
                                           /*}}}*/                        
varrayColProperties:                       /*{{{*/                        
    VARRAY exprId varrayFieldProperties { $$ = new Sm::table::field_property::VarrayField(@$, $2, $3); logSkipped(@$) }
  ;                                                                       
%type <tableFieldProperty_VarrayField> varrayColProperties;               
varrayFieldProperties:                             /*{{{*/                
                               varrayStorageClause { $$ = 0; logSkipped(@$) }
  | substitutableFieldProperty varrayStorageClause { $$ = $1; $$->loc(@$); logSkipped(@$) }
  | substitutableFieldProperty                     { $$ = $1; logSkipped(@$) }
  ;
%type <table_SubstitutableProperty> varrayFieldProperties;
                                                   /*}}}*/
                                           /*}}}*/
xmltypeColumnPropertiesTailOpt:            /*{{{*/
    xmltypeStorage                                       { $$ = 0; logSkipped(@$) }
  | xmlSchemaSpec                                        { $$ = $1;  $$->loc(@$); logSkipped(@$) }
  | XMLSCHEMA XMLSchema_URL xmlSchemaSpec                { $$ = $3;  $$->loc(@$); logSkipped(@$) }
  | xmltypeStorage xmlSchemaSpec                         { $$ = 0; del($2); logSkipped(@$) }
  | xmltypeStorage XMLSCHEMA XMLSchema_URL xmlSchemaSpec { $$ = 0; del($4); logSkipped(@$) }
  | /* EMPTY */                                          { $$ = 0; }
  ;
%type <id> xmltypeColumnPropertiesTailOpt;
                                           /*}}}*/
supplementalLoggingProps:   /* scip */     /*{{{*/
    GROUP RawID '(' groupColList       ')' ALWAYS_opt { del($2); logSkipped(@$) }
  |       RawID '(' supplementalIdKeys ')' COLUMNS /* $1 = DATA */ { del($1); logSkipped(@$) }
  ;
groupColList:
                     groupColItem
  | groupColList ',' groupColItem { del($3); }
  ;
                                           /*}}}*/
enableDisableConstraintListOpt:            /*{{{*/
    enableDisableConstraintList                 { $$ = $1; }
  |                                             { $$ = 0; }
  ;
enableDisableConstraintList:
                                enableDisableConstraint { $$ = mkList($1); }
  | enableDisableConstraintList enableDisableConstraint { $$ = $1; $$->push_back($2); }
  ;
%type <table_EnableDisableConstraintList> enableDisableConstraintList enableDisableConstraintListOpt;
enableDisableConstraint:
    ENABLE  validateNovalidateOpt constraint /* exceptionsClauseOpt */ cascadeOpt keepDropIndexOpt 
    { 
      $$ = new Sm::table::EnableDisableConstraint(@$, $3, $4, $2, Sm::table::enable_disable_constraint::ENABLE,  $5);
      logSkipped(@$)
    }
  | DISABLE validateNovalidateOpt constraint /* exceptionsClauseOpt */ cascadeOpt keepDropIndexOpt 
    { 
      $$ = new Sm::table::EnableDisableConstraint(@$, $3, $4, $2, Sm::table::enable_disable_constraint::DISABLE, $5);
      logSkipped(@$)
    }
  ;
%type <table_EnableDisableConstraint> enableDisableConstraint;
                                           /*}}}*/
oidClauseOpt:               /* scip */     /*{{{*/
    oidClause { logSkipped(@$) }
  |
  ;                                        /*}}}*/
physicalPropertyOpt:                       /*{{{*/
    physicalProperties { $$ = $1; } 
  | /* EMPTY */        { $$ = 0; }
  ;
%type <table_PhysicalProperties> physicalPropertyOpt;

physicalProperties:                       
                       physicalProperty { $$ = $1; } 
  | physicalProperties physicalProperty
    {
      $$ = $1;
      if ($$ && $2)
        $$->concat($2);
      else if (!$$)
        $$ = $2;
    }
  ;
%type <table_PhysicalProperties> physicalProperties;


                                           /*}}}*/
                                           /*{{{*/
xmltypeStorageOpt:          /* scip */
    /* EMPTY */
  | XMLTYPE xmltypeStorage { logSkipped(@$) }
  ;                                        /*}}}*/
encryptSpec:                               /*{{{*/
    ENCRYPT usingExprIdOpt identifiedByOpt SALT_opt { $$ = new Sm::table::EncryptSpec(@$, $2, $3, $4); $$->loc(@$); }
  ;
encryptSpecOpt:
    /* EMPTY */                                    { $$ = 0; }
  | encryptSpec                                    { $$ = $1; }
  ;
%type <table_EncryptSpec> encryptSpec encryptSpecOpt;
identifiedByOpt:                               /*{{{*/
    /* EMPTY */   { $$ = 0; }
  | IDENTIFIED_BY RawID { $$ = $2; $$->loc(@$); }
  ;
%type <id> identifiedByOpt;                    /*}}}*/
SALT_opt:                                      /*{{{*/
    SALT        { $$ = true; }
  | NO_SALT     { $$ = false; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> SALT_opt;                      /*}}}*/
                                           /*}}}*/
groupColItem:                              /*{{{*/
    exprId NO_LOG_opt { $$ = $1; $$->loc(@$); logSkipped(@$)  }
  ;
%type <id> groupColItem;
                                           /*}}}*/
oidIndexClauseOpt:                         /*{{{*/
    oidIndexClause  { $$ = $1; logSkipped(@$) }
  |                 { $$ = 0; }
  ;
oidIndexClause:
    OIDINDEX exprId '(' physicalAttributesListOpt ')' { $$ = new Sm::table::OidIndex(@$, $2, $4); } 
  | OIDINDEX        '(' physicalAttributesListOpt ')' { $$ = new Sm::table::OidIndex(@$, $3); } 
  ;
%type <table_OidIndex> oidIndexClause oidIndexClauseOpt;
                                           /*}}}*/
physicalAttributesListOpt:                 /*{{{*/
    physicalAttributesList { $$ = $1; } 
  | /* EMPTY */            { $$ = 0; }
  ;
physicalAttributesList:
                            physicalAttributesListItem { $$ = $1; } 
  | physicalAttributesList  physicalAttributesListItem { $$ = $1; $$->concat($2); $$->loc(@$); } 
  ;
%type <table_PhysicalProperties> physicalAttributesList physicalAttributesListOpt;
physicalAttributesListItem:
    physicalAttributeClause { $$ = $1;  $$->loc(@$); } 
  | tablespaceClause        { $$ = 0; logSkipped(@$) }
  ;
%type <table_PhysicalProperties> physicalAttributesListItem;
                                           /*}}}*/
substitutableFieldProperty:                /*{{{*/
    ELEMENT_opt IS OF      '(' ONLY datatype ')' { $$ = new Sm::table::SubstitutableProperty(@$, $6, true, $1); logSkipped(@$) }
  | ELEMENT_opt IS OF_TYPE '(' ONLY datatype ')' { $$ = new Sm::table::SubstitutableProperty(@$, $6, true, $1); logSkipped(@$) }
  |     SUBSTITUTABLE_AT_ALL_LEVELS              { $$ = new Sm::table::SubstitutableProperty(@$, Sm::table::SubstitutableProperty::SUBSTITUTABLE_AT_ALL_LEVELS); logSkipped(@$) }
  | NOT SUBSTITUTABLE_AT_ALL_LEVELS              { $$ = new Sm::table::SubstitutableProperty(@$, Sm::table::SubstitutableProperty::NOT_SUBSTITUTABLE_AT_ALL_LEVELS); logSkipped(@$) }
  ;
%type <table_SubstitutableProperty> substitutableFieldProperty substitutableFieldPropertyOpt;
                                           /*}}}*/
xmlschemaSpecOpt:                          /*{{{*/
    XMLSCHEMA XMLSchema_URL { $$ = new Sm::Identificator(STRING("XMLSchema_URL"), @1); logSkipped(@$) }
  | /* EMPTY */             { $$ = 0; }
  ;  
%type <id> xmlschemaSpecOpt;               /*}}}*/ 
nestedName:                                /*{{{*/
    exprId       { $$ = new Sm::table::field_property::NestedName(@$, $1); }
  ;
%type <nestedName> nestedName;             /*}}}*/
xmlSchemaSpec:                             /*{{{*/
    ELEMENT exprId                   { $$ = $2; $$->loc(@$); logSkipped(@$) }
  | ELEMENT XMLSchema_URL '#' exprId { $$ = $4; $$->loc(@$); logSkipped(@$) }
  ;
%type <id> xmlSchemaSpec;                  /*}}}*/
asSubqueryOpt:                             /*{{{*/
    AS subquery { $$ = $2;  $$->loc(@$); }
  | /* EMPTY */ { $$ = 0; }
  ;
%type <subquery> asSubqueryOpt;            /*}}}*/
defaultExprOpt:                            /*{{{*/
    defaultExpr     { $$ = $1;  $$->loc(@$); }
  | /* EMPTY */     { $$ = 0; }
  ;
defaultExpr:
    DEFAULT sqlExpr { $$ = $2; $$->loc(@$); }
  | ":=" sqlExpr { $$ = $2; $$->loc(@$); }
  ;
%type <sqlExpr> defaultExpr defaultExprOpt;
                                           /*}}}*/
onCommitRowsOpt:                           /*{{{*/
    ON_COMMIT_DELETE_ROWS   { $$ = Sm::table::ON_COMMIT_DELETE_ROWS;   logSkipped(@$) }
  | ON_COMMIT_PRESERVE_ROWS { $$ = Sm::table::ON_COMMIT_PRESERVE_ROWS; logSkipped(@$) }
  |                         { $$ = Sm::table::EMPTY;                   }
  ;
%type <onCommitRows> onCommitRowsOpt;
                                           /*}}}*/
cacheNocacheOpt:                           /*{{{*/
    CACHE       { $$ = Sm::table::table_properties::CACHE; logSkipped(@$) }
  | NOCACHE     { $$ = Sm::table::table_properties::NOCACHE; logSkipped(@$) }
  | /* EMPTY */ { $$ = Sm::table::table_properties::C_EMPTY; }
  ;
%type <cachingState> cacheNocacheOpt;
                                           /*}}}*/
rowdependenciesOpt:                        /*{{{*/
    ROWDEPENDENCIES   { $$ = Sm::table::table_properties::ROW_DEPENDENCIES; logSkipped(@$) }
  | NOROWDEPENDENCIES { $$ = Sm::table::table_properties::NO_ROW_DEPENDENCIES; logSkipped(@$) }
  |                   { $$ = Sm::table::table_properties::R_EMPTY; }
  ;
%type <rowDependenciesState> rowdependenciesOpt;
                                           /*}}}*/
SORT_opt:                                  /*{{{*/
   SORT         { $$ = true; logSkipped(@$) }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> SORT_opt;           /*}}}*/
usingExprIdOpt:                            /*{{{*/
    USING exprId              { $$ = $2;  $$->loc(@$); } 
  | /* EMPTY */               { $$ = 0; }
  ;
%type <id> usingExprIdOpt;                 /*}}}*/
physicalAttributeClause:                   /*{{{*/
    INITRANS NUMERIC_ID { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->intrans = $2; }
  | PCTFREE  NUMERIC_ID { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->pctfree = $2; }
  | PCTUSED  NUMERIC_ID { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->pctused = $2; }
  | MAXTRANS NUMERIC_ID { $$ = 0; del($2);  logSkipped(@$) }
  | storageClause       { $$ = 0;  logSkipped(@$) }
  ;
%type <table_PhysicalProperties> physicalAttributeClause;
                                           /*}}}*/
ELEMENT_opt:                               /*{{{*/
    ELEMENT     { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> ELEMENT_opt;                /*}}}*/

default_or_force: DEFAULT | FORCE;

physicalProperty:                          /*{{{*/
    COMPRESS        NUMERIC_ID /*?*/  { $$ = 0; del($2); logSkipped(@$) }
  | FOR ALL         OPERATIONS /*?*/  { $$ = 0; logSkipped(@$) }
  | FOR DIRECT_LOAD OPERATIONS /*?*/  { $$ = 0; logSkipped(@$) }
  | INCLUDING selectedEntity /*skip*/ { $$ = 0; context->release($2); logSkipped(@$) }
  | MAPPING TABLE              /*?*/  { $$ = 0; logSkipped(@$) }
  | NOCOMPRESS                 /*?*/  { $$ = 0; logSkipped(@$) }
  | NOMAPPING                  /*?*/  { $$ = 0; logSkipped(@$) }
  | PCTTHRESHOLD NUMERIC_ID    /*?*/  { $$ = 0; del($2); logSkipped(@$) }
  | OVERFLOWk                  /*?*/  { $$ = 0; logSkipped(@$) }
  | PCTFREE      NUMERIC_ID           { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->intrans = $2; }
  | PCTUSED      NUMERIC_ID           { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->pctfree = $2; }
  | INITRANS     NUMERIC_ID           { $$ = new Sm::table::field_property::PhysicalProperties(); $$->loc(@$); $$->pctused = $2; }
  | MAXTRANS     NUMERIC_ID           { $$ = 0; del($2); logSkipped(@$) }
  | REJECT_LIMIT NUMERIC_ID           { $$ = 0; del($2); logSkipped(@$) }
  | REJECTk LIMIT_UNLIMITED           { $$ = 0; logSkipped(@$) }
  | RESULT_CACHE '(' MODE default_or_force ')' { $$ = 0; logSkipped(@$) }
  | storageClause                     { $$ = 0; logSkipped(@$) }
  | opaqueClause                      { $$ = 0; logSkipped(@$) }
  | xmltypeClause                     { $$ = 0; logSkipped(@$) }
  | TABLESPACE physicalTablespaceId   { $$ = 0; logSkipped(@$) }
  | LOGGING                           { $$ = 0; logSkipped(@$) }
  | NOLOGGING                         { $$ = 0; logSkipped(@$) }
  | ORGANIZATION_EXTERNAL '(' typeAccessDriverOpt externalDataProperties ')' { $$ = 0; logSkipped(@$) }
  | ORGANIZATION_HEAP                 { $$ = 0; logSkipped(@$) }
  | ORGANIZATION_INDEX                { $$ = 0; logSkipped(@$) }
  | CLUSTER RawID '(' schemaList ')'  { $$ = 0; del($2); logSkipped(@$) }
  ;
%type <table_PhysicalProperties> physicalProperty;

physicalTablespaceId:
    RawID
  ;

physicalPropertyOpaque:
    physicalProperty       { logSkipped(@$) }
  | DISABLE STORAGE IN ROW { logSkipped(@$) }
  | ENABLE  STORAGE IN ROW { logSkipped(@$) }
  | CHUNK NUMERIC_ID       { logSkipped(@$) }
  | PCTVERSION NUMERIC_ID  { logSkipped(@$) }
  | CACHE                  { logSkipped(@$) }
  | RawID NUMERIC_ID       { logSkipped(@$) }
  | RawID DEFAULT          { logSkipped(@$) }
  | NOCACHE                { logSkipped(@$) }
  ;
physicalPropertiesOpaque:
                             physicalPropertyOpaque
  | physicalPropertiesOpaque physicalPropertyOpaque
  ;

xmltypeClause:
    XMLTYPE COLUMN RawID STORE AS RawID RawID '(' physicalPropertiesOpaque ')' { logSkipped(@$) }
  ;

opaqueClause:
    OPAQUE TYPE RawID STORE AS RawID LOB '(' physicalPropertiesOpaque ')' { logSkipped(@$) }
  ;

typeAccessDriverOpt:                           /*{{{*/
  | RawID exprId  /* $1 = "TYPE" */ { del($1, $2); logSkipped(@$) }
  ;                                            /*}}}*/
externalDataProperties:                        /*{{{*/
    DEFAULT DIRECTORY RawID 
    accessParametersSpecOpt
    RawID '(' locationList ')' /* $5 = LOCATION */  { del($3,$5); logSkipped(@$) }
  ;
locationList:                     /* scip {{{*/
                     locationItem
  | locationList ',' locationItem
  ;                               /*}}}*/
accessParametersSpecOpt:
  | ACCESS_PARAMETERS USING_CLOB subquery { del($3); logSkipped(@$) }
  ;                                            /*}}}*/
                                           /*}}}*/
ReturnAsLocatorOrValueOpt:                 /*{{{*/
    RETURN AS RawID { $$ = Sm::table::field_property::nested_table::LOCATOR; }
  | RETURN AS_VALUE   { $$ = Sm::table::field_property::nested_table::VALUE;   }
  | /* EMPTY */       { $$ = Sm::table::field_property::nested_table::EMPTY;   }
  ;
%type <locatorOrValue> ReturnAsLocatorOrValueOpt;
                                           /*}}}*/
COLUMN_opt:                                /*{{{*/
    COLUMN      { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> COLUMN_opt;                /*}}}*/
ALWAYS_opt:                                /*{{{*/
    ALWAYS      { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> ALWAYS_opt;                /*}}}*/
NO_LOG_opt:                                /*{{{*/
    NO_LOG      { $$ = true; logSkipped(@$) }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> NO_LOG_opt;                /*}}}*/
EnableState:                               /*{{{*/
    ENABLE  { $$ = Sm::EnableState::DISABLE; }
  | DISABLE { $$ = Sm::EnableState::ENABLE ; }
  ;
%type <enableState> EnableState;
                                           /*}}}*/
tableCompression:                          /*{{{*/
    COMPRESS
  | COMPRESS FOR ALL         OPERATIONS
  | COMPRESS FOR DIRECT_LOAD OPERATIONS
  | NOCOMPRESS
  ;                                        /*}}}*/
supplementalIdKeys:          /* skip */    /*{{{*/
                           supplementalIdKey 
  | supplementalIdKeys ',' supplementalIdKey 
  ;
supplementalIdKey:
    ALL         
  | PRIMARY_KEY 
  | UNIQUE      
  | FOREIGN_KEY 
  ;
                                           /*}}}*/
validateNovalidateOpt:                     /*{{{*/
    VALIDATE     { $$ = Sm::table::enable_disable_constraint::VALIDATE;   }
  | NOVALIDATE   { $$ = Sm::table::enable_disable_constraint::NOVALIDATE; }
  | /* EMPTY */  { $$ = Sm::table::enable_disable_constraint::V_EMPTY;    }
  ;
%type <validateState> validateNovalidateOpt;
                                           /*}}}*/
cascadeOpt:                                /*{{{*/
    CASCADE     { $$ = true; }
  | /* EMPTY */ { $$ = false; }
  ;
%type <boolval> cascadeOpt;                 /*}}}*/
keepDropIndexOpt:                          /*{{{*/
    KEEP_INDEX  { $$ = Sm::table::enable_disable_constraint::KEEP; }
  | DROP INDEX  { $$ = Sm::table::enable_disable_constraint::DROP; }
  | /* EMPTY */ { $$ = Sm::table::enable_disable_constraint::K_EMPTY; }
  ;
%type <keepDropState> keepDropIndexOpt;    /*}}}*/
objectTableSubstitutionOpt:  /* skip */    /*{{{*/
    NOT_opt SUBSTITUTABLE_AT_ALL_LEVELS { logSkipped(@$) }
  | /* EMPTY */
  ;                                        /*}}}*/
oidClause:                   /* skip */    /*{{{*/
    OBJECT_IDENTIFIER_IS_SYSTEM_GENERATED { logSkipped(@$) }
  | OBJECT_IDENTIFIER IS PRIMARY_KEY      { logSkipped(@$) }
  ;                                        /*}}}*/
                                   /*}}}*/
/* Пропускаемый контент              {{{*/
tablePartioningClausesOpt:       
    /* EMPTY */
  | rangePartitions       { logSkipped(@$) }
  | hashPartitions        { logSkipped(@$) }
  | listPartitions        { logSkipped(@$) }
  | referencePartitioning { logSkipped(@$) }
  | systemPartitioning    { logSkipped(@$) }
  ;
storageClause:
    STORAGE '(' storageClauseList ')' { logSkipped(@$) }
  ;
storageClauseList:
                      storageClauseItem
  | storageClauseList storageClauseItem
  ;
xmltypeStorage:                    /*{{{ <- scip */
    STORE AS OBJECT RELATIONAL {  logSkipped(@$) }
  | STORE AS_CLOB xmlTypeStorageParametersOpt {  logSkipped(@$) }
  ;
xmlTypeStorageParametersOpt:
    RawID lobParametersBrOpt { del($1); logSkipped(@$) }
  | '(' lobParameters ')' { logSkipped(@$) }
  | /* EMPTY */
  ;
lobParametersBrOpt:
    '(' lobParameters ')' { logSkipped(@$) }
  | /* EMPTY */
  ;                                /*}}}*/
lobPartitionListBrOpt:             /*{{{ <- scip */
  | '(' lobPartitionList ')' { logSkipped(@$) }
  ;
lobPartitionList:
                         lobPartitionStorage
  | lobPartitionList ',' lobPartitionStorage
  ;
lobPartitionStorage:
    PARTITION RawID varrayAndLobStorageList lobSubparititonClauseOpt { del($2); logSkipped(@$) }
  ;
lobStorageClause:
    LOB '(' columnIdList ')' STORE AS lobStorageList { del($3); logSkipped(@$) }
  ;
lobStorageClauseOpt:
    /* EMPTY */
  | lobStorageClause { logSkipped(@$) }
  ;
                                   /*}}}*/
rangePartitions:                   /*{{{ <- scip */
    PARTITION BY RANGE '(' columnIdList ')' intervalExprOpt compositeOrSimpleRangeTail { del($5); logSkipped(@$) }
  ;
compositeOrSimpleRangeTail:          /*{{{*/
  '(' rangePartitionsList ')' { logSkipped(@$) }
  | compositeSubpartitions '('  rangePartitionDescs ')' { logSkipped(@$) }
  ;

rangePartitionsList:                   /*{{{*/
                            rangePartitionsListItem
  | rangePartitionsList ',' rangePartitionsListItem
  ;
rangePartitionsListItem:
    PARTITION       rangeValuesClause tablePartitionDescription { logSkipped(@$) }
  | PARTITION RawID rangeValuesClause tablePartitionDescription { del($2); logSkipped(@$) }
  ;
tablePartitionDescription:               /*{{{*/
    segmentAttrsClauseOpt compressionOpt segOwerflowOpt varrayAndLobStorageListOpt
  ;
segOwerflowOpt:                           /*{{{*/
    OVERFLOWk segmentAttrsClauseOpt { logSkipped(@$) }
  | /* EMPTY */
  ;                                       /*}}}*/
segmentAttrsClauseOpt:                    /*{{{*/
    segmentAttrsClause { logSkipped(@$) }
  |
  ;                                       /*}}}*/
varrayAndLobStorageListOpt:                 /*{{{*/
    varrayAndLobStorageList { logSkipped(@$) }
  | /* EMPTY */
  ;
varrayAndLobStorageList:                      /*{{{*/
                            varrayAndLobStorage
  | varrayAndLobStorageList varrayAndLobStorage
  ;
varrayAndLobStorage:
    lobStorageClause    { logSkipped(@$) }
  | varrayColProperties { logSkipped(@$) }
  ;
lobStorageList:                                 /*{{{*/
                   lobStorageItem
  | lobStorageList lobStorageItem
  ;
lobStorageItem:
    RawID /* SECUREFILE, BASICFILE, LOB_segname */ { del($1); }
  | '(' lobParameters ')' { logSkipped(@$) }
  ;
                                                  /*{{{*/
                                                    /*{{{*/
                                                      /*{{{*/

                                     
   

lobParameters:                                          /*{{{*/
                  lobParameter
  | lobParameters lobParameter
  ;

lobParameter:
    storageClause                    { logSkipped(@$) }
  | tablespaceClause                 { logSkipped(@$) }
  | LOGGING                          { logSkipped(@$) }
  | NOLOGGING                        { logSkipped(@$) }
  | READS                            { logSkipped(@$) }
  | DISABLE STORAGE IN ROW           { logSkipped(@$) }
  | CACHE                            { logSkipped(@$) }
  | ENABLE  STORAGE IN ROW           { logSkipped(@$) }
  | NUMERIC_ID                       { del($1); logSkipped(@$) }
  | CHUNK                            { logSkipped(@$) }
  | NOCACHE                          { logSkipped(@$) }
  | FREEPOOLS                        { logSkipped(@$) }
  | RETENTION                        { logSkipped(@$) }
  | PCTVERSION                       { logSkipped(@$) }
  | INDEX '(' physicalProperties ')' { del($3); logSkipped(@$) }
  ;


                                                        /*}}}*/
                                                      /*}}}*/
                                                    /*}}}*/
                                                  /*}}}*/
                                                /*}}}*/
varrayStorageClause:                            /*{{{*/
    STORE AS       LOB varrayStorageClauseTail { logSkipped(@$) }
  | STORE AS RawID LOB   varrayStorageClauseTail { del($3); logSkipped(@$) }
  ;
varrayStorageClauseTail:                      
          '(' lobParameters ')' { logSkipped(@$) }
  | RawID '(' lobParameters ')' { del($1); logSkipped(@$) }
  | RawID                       { del($1); logSkipped(@$) }
  ;                                           
                                                /*}}}*/
                                              /*}}}*/
                                            /*}}}*/
                                         /*}}}*/
                                       /*}}}*/
rangePartitionDescs:                   /*{{{*/
                            rangePartitionDesc
  | rangePartitionDescs ',' rangePartitionDesc
  ;
rangePartitionDesc:
    PARTITION       rangeValuesClause tablePartitionDescription subpartitionTemplateBodyOpt { logSkipped(@$) }
  | PARTITION RawID rangeValuesClause tablePartitionDescription subpartitionTemplateBodyOpt { del($2); logSkipped(@$) }
  ;                                    /*}}}*/
                                     /*}}}*/
                                   /*}}}*/
listPartitions:                    /*{{{ <- scip */
    PARTITION BY LIST '(' exprId ')' compositeOrSimpleListPartition { del($5); logSkipped(@$) }
  ;
compositeOrSimpleListPartition:
    '(' listPartitionSpecs ')' { logSkipped(@$) }
  | compositeSubpartitions '(' listPartitionDescs ')' { logSkipped(@$) }
  ;
listPartitionDescs:
                           listPartitionDesc
  | listPartitionDescs ',' listPartitionDesc
  ;
listPartitionDesc:
    PARTITION       listValuesClause tablePartitionDescription subpartitionTemplateBodyOpt
  | PARTITION RawID listValuesClause tablePartitionDescription subpartitionTemplateBodyOpt { del($2); }
  ;
listPartitionSpecs:
                           listPartitionSpec
  | listPartitionSpecs ',' listPartitionSpec
  ;
listPartitionSpec:
    PARTITION RawID listValuesClauseOpt tablePartitionDescription { del($2); }
  ;                                /*}}}*/
hashPartitions:                    /*{{{ <- scip */
    PARTITION BY HASH '(' columnIdList ')' individualOrQuantityPartition { del($5); logSkipped(@$) }
  ;
individualOrQuantityPartition:
    '(' individualHashPartitions ')'
  | hashParitionsByQuantity
  ;
individualHashPartitions:            /*{{{*/
                                 individualHashPartition
  | individualHashPartitions ',' individualHashPartition
  ;

individualHashPartition:
    PARTITION       partitioningStorageClauseOpt
  | PARTITION RawID partitioningStorageClauseOpt { del($2); }
  ;

                                     /*}}}*/
hashParitionsByQuantity:             /*{{{*/
    PARTITIONS RawID storeInOpt compressionOpt overflovStoreInOpt { del($2); }
  ;

overflovStoreInOpt:
    OVERFLOWk STORE IN '(' rawIdList ')' { del($5); }
  |
  ;                                  /*}}}*/
                                   /*}}}*/
systemPartitioning:                /*{{{ <- scip */
    PARTITION_BY_SYSTEM systemPatitioningBodyOpt { logSkipped(@$) }
  ;
systemPatitioningBodyOpt:
    referencePartitionDescs
  | PARTITIONS NUMERIC_ID  { del($2); }
  | /* EMPTY */
  ;                                /*}}}*/
referencePartitioning:             /*{{{ <- scip */
    PARTITION BY REFERENCE '(' constraint ')' brReferencePartitionDescsOpt { del($5); logSkipped(@$) }
  ;
brReferencePartitionDescsOpt:
    '(' referencePartitionDescs ')'
  | /* EMPTY */
  ;
referencePartitionDescs:
                                referencePartitionDesc
  | referencePartitionDescs ',' referencePartitionDesc
  ;
referencePartitionDesc:
    PARTITION       referencePartitionDescTail
  | PARTITION RawID referencePartitionDescTail { del($2); }
  ;

referencePartitionDescTail:
    rangeValuesClause tablePartitionDescription
  ;


                                   /*}}}*/
compositeSubpartitions:            /*{{{ <- scip */
    subpartitionByRange { logSkipped(@$) }
  | subpartitionByList { logSkipped(@$) }
  | subpartitionByHash { logSkipped(@$) }
  ;
subpartitionByRange:
    SUBPARTITION BY RANGE '(' columnIdList ')' subpartitionTemplateOpt { del($5); logSkipped(@$) }
  ;
subpartitionByList: SUBPARTITION BY LIST '(' exprId ')' subpartitionTemplateOpt { del($5); logSkipped(@$) }
  ;
subpartitionByHash:
    SUBPARTITION BY HASH '(' columnIdList ')' intSubpartOrTmpltOpt { del($5); logSkipped(@$) }
  ;
intSubpartOrTmpltOpt:
    SUBPARTITION NUMERIC_ID storeInOpt { del($2); logSkipped(@$) }
  | subpartitionTemplateOpt { logSkipped(@$) }
  ;
subpartitionTemplateOpt:
    subpartitionTemplate { logSkipped(@$) }
  | /* EMPTY */
  ;
subpartitionTemplate:
    SUBPARTITION_TEMPLATE subpartitionTemplateBody { logSkipped(@$) }
  ;
                                   /*}}}*/
subpartitionTemplateBody:          /*{{{ <- scip */
    hashSubpartsByQuantity
  | '(' rangeSubpartitionDescs ')' { logSkipped(@$) }
  | '(' listSubpartitionDescs  ')' { logSkipped(@$) }
  | '(' individualHashSubparts ')' { logSkipped(@$) }
  ;
hashSubpartsByQuantity:
    SUBPARTITIONS NUMERIC_ID storeInOpt { del($2); logSkipped(@$) }
  ;

rangeSubpartitionDescs:
                               rangeSubpartitionDesc
  | rangeSubpartitionDescs ',' rangeSubpartitionDesc
  ;
rangeSubpartitionDesc:
    SUBPARTITION       rangeValuesClause partitioningStorageClauseOpt { logSkipped(@$) }
  | SUBPARTITION RawID rangeValuesClause partitioningStorageClauseOpt { del($2); logSkipped(@$) }
  ;

listSubpartitionDescs:
                              listSubpartitionDesc
  | listSubpartitionDescs ',' listSubpartitionDesc
  ;
listSubpartitionDesc:
    SUBPARTITION       listValuesClause partitioningStorageClauseOpt { logSkipped(@$) }
  | SUBPARTITION RawID listValuesClause partitioningStorageClauseOpt { del($2); logSkipped(@$) }
  ;

individualHashSubparts:
                               individualHashSubpart
  | individualHashSubparts ',' individualHashSubpart
  ;
individualHashSubpart:
    SUBPARTITION       partitioningStorageClauseOpt { logSkipped(@$) }
  | SUBPARTITION RawID partitioningStorageClauseOpt { del($2); logSkipped(@$) }
  ;                                /*}}}*/
subpartitionTemplateBodyOpt:       /*{{{ <- scip */
    /* EMPTY */
  | subpartitionTemplateBody { logSkipped(@$) }
  ;                                /*}}}*/
partitioningStorageClauseOpt:      /*{{{ <- scip */
  | partitioningStorageClause { logSkipped(@$) }
  ;                                /*}}}*/
rangeValuesClause:                 /*{{{ <- scip */
    VALUES LESS THAN '(' rangeLiteralList ')' { logSkipped(@$) }
  ;
rangeLiteralList:                 /* scip {{{*/
                         sqlExpr { del($1); }
  | rangeLiteralList ',' sqlExpr { del($3); }
  ;                               /*}}}*/
segmentAttrsClause:                /*{{{ <- scip */
                       segmentAttrClause
  | segmentAttrsClause segmentAttrClause
  ;
segmentAttrClause:
    LOGGING             { logSkipped(@$) }
  | NOLOGGING           { logSkipped(@$) }
  | PCTFREE  NUMERIC_ID { del($2); logSkipped(@$) }
  | INITRANS NUMERIC_ID { del($2); logSkipped(@$) }
  | PCTUSED  NUMERIC_ID { del($2); logSkipped(@$) }
  | storageClause       { logSkipped(@$) }
  | tablespaceClause    { logSkipped(@$) }
  ;
                                   /*}}}*/
tablespaceClause:
    TABLESPACE exprId { del($2); logSkipped(@$) }
  ;              
compressionOpt:                    /*{{{ <- scip */
    COMPRESS FOR ALL OPERATIONS         { logSkipped(@$) }
  | COMPRESS FOR DIRECT_LOAD OPERATIONS { logSkipped(@$) }
  | NOCOMPRESS                          { logSkipped(@$) }
  | COMPRESS NUMERIC_ID                 { del($2); logSkipped(@$) }
  | /* EMPTY */
  ;                                /*}}}*/
intervalExprOpt:                   /*{{{ <- scip */
    RawID sqlExpr storeInOpt /* $1 = INTERVAL */ { del($1, $2); logSkipped(@$) }
  | /* EMPTY */
  ;/*}}}*/
storeInOpt:                        /*{{{ <- scip */
    STORE IN '(' rawIdList ')' { del($4); logSkipped(@$) }
  | /* EMPTY */
  ;                                /*}}}*/
listValuesClause:                  /*{{{ <- scip */
    VALUES '(' rangeLiteralList ')' { logSkipped(@$) }
  | VALUES '(' DEFAULT ')' { logSkipped(@$) }
  ;                                /*}}}*/
listValuesClauseOpt:               /*{{{ <- scip */
    /* EMPTY */
  | listValuesClause { logSkipped(@$) }
  ;                                /*}}}*/
lobSubparititonClauseOpt:          /*{{{ <- scip */
  | '(' SUBPARTITION RawID varrayAndPartitioningList ')' { del($3); logSkipped(@$) }
  ;
varrayAndPartitioningList:
                              varrayAndPartitioning
  | varrayAndPartitioningList varrayAndPartitioning
  ;
varrayAndPartitioning:
    lobPartitioninigStorage { logSkipped(@$) }
  | varrayColProperties { logSkipped(@$) }
  ;                                /*}}}*/
lobPartitioninigStorage:           /*{{{ <- scip */
    LOB '(' exprId ')' STORE AS lobPartitioninigStorageTail { del($3); logSkipped(@$) }
  ;
lobPartitioninigStorageTail:
    RawID tablespaceBrOpt { del($1); logSkipped(@$) }
  | '(' tablespaceClause  ')' { logSkipped(@$) }
  ;
tablespaceBrOpt:
    '(' tablespaceClause ')' { logSkipped(@$) }
  | /* EMPTY */
  ;                                /*}}}*/
locationItem:                      /*{{{ <- scip */
    RawID            { del($1); logSkipped(@$) }
  | RawID ':' RawID  { del($1, $3); logSkipped(@$)}
  ;                                /*}}}*/
storeInTblspaceOpt:                /*{{{ <- scip */
  | STORE IN '(' columnIdList ')' { del($4); logSkipped(@$) }
  ;                                /*}}}*/
subpartitionSpec:                  /*{{{ <- scip */
    SUBPARTITION       listValuesClauseOpt partitioningStorageClause { logSkipped(@$) }
  | SUBPARTITION RawID listValuesClauseOpt partitioningStorageClause { del($2); logSkipped(@$) }
  ;                                /*}}}*/
partitioningStorageClause:         /*{{{ <- scip */
                              partitioningStorageItem
  | partitioningStorageClause partitioningStorageItem
  ;
partitioningStorageItem:
    tablespaceClause                       { logSkipped(@$) }
  | OVERFLOWk                              { logSkipped(@$) }
  | lobPartitioninigStorage                { logSkipped(@$) }
  | VARRAY RawID STORE AS       LOB RawID  { del($2, $6); logSkipped(@$)}
  | VARRAY RawID STORE AS RawID LOB RawID  { del($2, $5, $7); logSkipped(@$) }
  ;
                                   /*}}}*/
parallelClauseOpt:                 /*{{{ <- scip */
    parallelClause { logSkipped(@$) }
  | /* EMPTY */
  ;

defaultOrNumericId: 
    DEFAULT 
  | NUMERIC_ID { del($1); }
  ;

monitoringClauseOpt:
    MONITORING    { logSkipped(@$) }
  | NOMONITORING  { logSkipped(@$) }
  | /* EMPTY */
  ;

parallelClause:
    NOPARALLEL { logSkipped(@$) }
  | PARALLEL   { logSkipped(@$) }
  | PARALLEL NUMERIC_ID { del($2); logSkipped(@$) }
  | PARALLEL '(' RawID /* <DEGREE */ defaultOrNumericId RawID /*<INSTANCES*/ defaultOrNumericId ')' { del($3, $5); logSkipped(@$) }
  ;                                /*}}}*/
                                   /*}}}*/
/*}}}*/
/* CREATE SEQUENCE+{{{1 */
 
sequenceBody:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::Sequence_, $1); $$ = 0; }
  | rawIdDotRawId /* DDL */ sequenceList { $$ = new Sm::Sequence(@$, $1, $2);  }
  ;
%type <sequence> sequenceBody;

sequenceItem:
    INCREMENT BY signedNumericId { $$ = new Sm::SequenceBody(@$); $$->v.flags.incrementBy = 1; $$->incrementBy = $3; }
  | START WITH   signedNumericId { $$ = new Sm::SequenceBody(@$); $$->v.flags.startWith   = 1; $$->startWith   = $3; }
  |   MAXVALUE   signedNumericId { $$ = new Sm::SequenceBody(@$); $$->v.flags.maxvalue    = 1; $$->maxvalue    = $2; }
  |   MINVALUE   signedNumericId { $$ = new Sm::SequenceBody(@$); $$->v.flags.minvalue    = 1; $$->minvalue    = $2; }
  |   CACHE      signedNumericId { $$ = new Sm::SequenceBody(@$); $$->v.flags.cache       = 1; $$->cache       = $2; }
  | NOMAXVALUE                   { $$ = new Sm::SequenceBody(@$); $$->v.flags.nomaxvalue  = 1; }
  | NOMINVALUE                   { $$ = new Sm::SequenceBody(@$); $$->v.flags.nominvalue  = 1; }
  |   CYCLE                      { $$ = new Sm::SequenceBody(@$); $$->v.flags.cycle       = 1; }
  | NOCYCLE                      { $$ = new Sm::SequenceBody(@$); $$->v.flags.nocycle     = 1; }
  | NOCACHE                      { $$ = new Sm::SequenceBody(@$); $$->v.flags.nocache     = 1; }
  |   ORDER                      { $$ = new Sm::SequenceBody(@$); $$->v.flags.order       = 1; }
  | NOORDER                      { $$ = new Sm::SequenceBody(@$); $$->v.flags.noorder     = 1; }
  ;
%type <sequenceBody> sequenceItem;

sequenceList:
                 sequenceItem { $$ = $1;  }
  | sequenceList sequenceItem { $$ = $1; $$->concat($2); $$->loc(@$); }
  ;
%type <sequenceBody> sequenceList;

  /* end of SEQUENCE 1}}} */
/* CREATE DATABASE LINK  {{{ */

databaseLink:
    rawIdDotRawId WRAPPED_ENTITY { context->model->addWrapped(Sm::ResolvedEntity::DatabaseLink_, $1); $$ = 0; }
  | rawIdDotRawId connectToCombOpt usingConnectStringOpt { $$ = new Sm::DatabaseLink($1, $2, $3); }
  ;
%type <databaseLink> databaseLink;

connectToCombOpt:
    /* { connectToCurrentUser connectToCustomUser authenticatedDblink } ->  {{{

       {1}, {2}, {3}, 
       {12, 21}, {13, 31}, {23, 32},
       {123, 132, 231, 312, 213, 321} 

       A1, A2, A3 -> $[Ai==1] $[Ai==2] $[Ai==3]
       
       1                1
         1              2
           1            3
       2                         1
         2                       2
           2                     3
       3                                   1
         3                                 2
           3                               3

    }}}*/
    /* EMPTY */                                                                      { $$ = 0; }
  | connectToCurrentUser /*1*/                                                       { $$ = new Sm::DatabaseLinkBody(true); }
  | connectToCustomUser  /*2*/                                                       { $$ = new Sm::DatabaseLinkBody(false, ($1),    0); }
  | authenticatedDblink  /*3*/                                                       { $$ = new Sm::DatabaseLinkBody(false,    0, ($1)); }
  | connectToCurrentUser /*1*/ connectToCustomUser  /*2*/                            { $$ = new Sm::DatabaseLinkBody(true , ($2),    0); }
  | connectToCustomUser  /*2*/ connectToCurrentUser /*1*/                            { $$ = new Sm::DatabaseLinkBody(true , ($1),    0); }
  | connectToCurrentUser /*1*/ authenticatedDblink  /*3*/                            { $$ = new Sm::DatabaseLinkBody(true ,    0, ($2)); }
  | authenticatedDblink  /*3*/ connectToCurrentUser /*1*/                            { $$ = new Sm::DatabaseLinkBody(true ,    0, ($1)); }
  | connectToCustomUser  /*2*/ authenticatedDblink  /*3*/                            { $$ = new Sm::DatabaseLinkBody(false, ($1), ($2)); }
  | authenticatedDblink  /*3*/ connectToCustomUser  /*2*/                            { $$ = new Sm::DatabaseLinkBody(false, ($2), ($1)); }
  | connectToCurrentUser /*1*/ connectToCustomUser  /*2*/ authenticatedDblink  /*3*/ { $$ = new Sm::DatabaseLinkBody(true , ($2), ($3)); }
  | connectToCurrentUser /*1*/ authenticatedDblink  /*3*/ connectToCustomUser  /*2*/ { $$ = new Sm::DatabaseLinkBody(true , ($3), ($2)); }
  | connectToCustomUser  /*2*/ authenticatedDblink  /*3*/ connectToCurrentUser /*1*/ { $$ = new Sm::DatabaseLinkBody(true , ($1), ($2)); }
  | authenticatedDblink  /*3*/ connectToCurrentUser /*1*/ connectToCustomUser  /*2*/ { $$ = new Sm::DatabaseLinkBody(true , ($3), ($1)); }
  | connectToCustomUser  /*2*/ connectToCurrentUser /*1*/ authenticatedDblink  /*3*/ { $$ = new Sm::DatabaseLinkBody(true , ($1), ($3)); }
  | authenticatedDblink  /*3*/ connectToCustomUser  /*2*/ connectToCurrentUser /*1*/ { $$ = new Sm::DatabaseLinkBody(true , ($2), ($1)); }
  ;
%type <linkBody> connectToCombOpt;

connectToCurrentUser /*1*/: CONNECT TO CURRENT_USER;
connectToCustomUser  /*2*/: CONNECT TO       exprId IDENTIFIED_BY RawID { $$ = new Sm::DblinkUserAuthentication; $$->user = $3; $$->password = $5; };
authenticatedDblink  /*3*/: AUTHENTICATED_BY exprId IDENTIFIED_BY RawID { $$ = new Sm::DblinkUserAuthentication; $$->user = $2; $$->password = $4; };

%type <userAuthentication> connectToCustomUser authenticatedDblink;

usingConnectStringOpt: 
    /* EMPTY */ { $$ = 0; }
  | USING RawID { $$ = $2; }
  ;
%type <id> usingConnectStringOpt;

/* end of CREATE DATABASE LINK }}} */
/* STORAGE CLAUSE  {{{ */

storageClauseItem: /*{{{*/
      RawID NULLk   /*| OPTIMAL         NULLk*/   { del($1); logSkipped(@$) }
    | RawID DEFAULT /*| BUFFER_POOL     DEFAULT*/ { del($1); logSkipped(@$) }
    | RawID RawID                                 { del($1, $2); logSkipped(@$) }
    | MAXEXTENTS RawID                            { del($2); logSkipped(@$) }
  /* | MINEXTENTS      NUMERIC_ID */
    | MAXEXTENTS      NUMERIC_ID                  { del($2); logSkipped(@$) }
  /*| BUFFER_POOL     KEEP*/
  /*| BUFFER_POOL     RECYCLE*/
    | RawID RawID NUMERIC_ID /* $1 = FREELIST $2 = GROUPS */ { del($1, $2, $3); logSkipped(@$) }
    | RawID NUMERIC_ID /* $1 = INITIALk NEXT MINEXTENTS MAXEXTENTS PCTINCREASE FREELISTS OPTIMAL */ { del($1, $2); logSkipped(@$) }
    | RawID sizeClause /* $1 = OPTIMAL */  { del($1); logSkipped(@$) }
    | INITIALtok      NUMERIC_ID           { del($2); logSkipped(@$) }
  /*| NEXT            NUMERIC_ID*/
  /*| PCTINCREASE     NUMERIC_ID */
  /*| FREELISTS       NUMERIC_ID*/
  /*| FREELIST GROUPS NUMERIC_ID*/
  /*| OPTIMAL         sizeClause*/
  ;
                   /*}}}*/
sizeClause:        /*{{{*/
    NUMERIC_ID 'K' { del($1); }
  | NUMERIC_ID 'k' { del($1); }
  | NUMERIC_ID 'M' { del($1); }
  | NUMERIC_ID 'm' { del($1); }
  | NUMERIC_ID 'G' { del($1); }
  | NUMERIC_ID 'g' { del($1); }
  | NUMERIC_ID 'T' { del($1); }
  | NUMERIC_ID 't' { del($1); }
  | NUMERIC_ID 'P' { del($1); }
  | NUMERIC_ID 'p' { del($1); }
  | NUMERIC_ID 'E' { del($1); }
  | NUMERIC_ID 'e' { del($1); }
  ;
                   /*}}}*/

  /* end of STORAGE CLAUSE }}} */
/* CREATE INDEX    {{{ */
 /* http://docs.oracle.com/cd/B19306_01/server.102/b14200/statements_5010.htm#i2125762 */

createIndexTok:
    CREATE_INDEX        { context->downCtxs.parsedIndex = Sm::CathegoryIndexEnum::SIMPLE; }

createUniqueIndexTok:
    CREATE_UNIQUE_INDEX { context->downCtxs.parsedIndex = Sm::CathegoryIndexEnum::UNIQUE; }

createBitmapIndexTok:
    CREATE_BITMAP_INDEX { context->downCtxs.parsedIndex = Sm::CathegoryIndexEnum::BITMAP; }

createIndexStatement:
    createIndexTok       createIndexBody { $$ = $2; $$->cathegoryIndex(Sm::Index::SIMPLE); }
  | createUniqueIndexTok createIndexBody { $$ = $2; $$->cathegoryIndex(Sm::Index::UNIQUE); }
  | createBitmapIndexTok createIndexBody { $$ = $2; $$->cathegoryIndex(Sm::Index::BITMAP); }
  ;
createIndexBody:                     /*{{{*/
    rawIdDotRawId WRAPPED_ENTITY {
      context->model->addWrapped(Sm::CathegoryIndexEnum::convertDdl(context->downCtxs.parsedIndex), $1);
      $$ = 0;
    }
  | rawIdDotRawId ON clusterOrTableIndex { $$ = $3; $$->setName($1); $$->loc(@$); }
  ;
clusterOrTableIndex:
    CLUSTER schemaDotColumn indexAttributesOpt {
      $$ = 0; del($2); logSkipped(@$)
    }
  | schemaDotColumn aliasOpt '(' indexExprList ')' tableOrBitmapPropertiesOpt { $$ = new Sm::Index(@$, $1, $2, $4, context->downCtxs.parsedIndex); };

%type <index> createIndexBody createIndexStatement clusterOrTableIndex;
tableOrBitmapPropertiesOpt:          /*{{{*/
    /* EMPTY */
  | indexProperties         { logSkipped(@$) }
  | REVERSE indexProperties { logSkipped(@$) }
  | FROM /* bitmap join index clause; в документации написано что это для joined tables ?? => scip */
    sqlNameAliasList WHERE condition localPartitionedIndexOpt indexAttributesOpt { del($4); logSkipped(@$) }
  ;                                  /*}}}*/
aliasOpt:                            /*{{{*/
    exprId       { $$ = $1; }
  | /* EMPTY */  { $$ = 0; }
  ;
%type <id> aliasOpt;
                                     /*}}}*/
indexExprList:                       /*{{{*/
                      indexExprItem { $$ = mkList($1); }
  | indexExprList ',' indexExprItem { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprList> indexExprList;

indexExprItem:
    sqlExpr AscDescOpt { $$ = $1; $$->loc(@$); }
  ;
%type <sqlExpr> indexExprItem;
/*}}}*/
sqlNameAliasList:                    /*{{{*/
                          sqlNameAliasItem /* skip */
  | sqlNameAliasList  ',' sqlNameAliasItem /* skip */
  ;
sqlNameAliasItem:       /* skip */
    selectedEntity /* skip */         { context->release($1); }
  | selectedEntity exprId  /* skip */ { context->release($1); del($2); }
  ;
                                     /*}}}*/
                                     /*}}}*/
AscDescOpt:                          /*{{{*/
    ASC | DESC | /* EMPTY */ 
  ;
                                     /*}}}*/


indexProperties:
    INDEXTYPE_IS datatype parallelClauseOpt                      /* scip */                  { del($2); logSkipped(@$) }
  | INDEXTYPE_IS datatype RawID '(' exprId ')'                   /* $3 = PARAMETERS scip */  { del($2, $3, $5); logSkipped(@$) }
  | INDEXTYPE_IS datatype parallelClause RawID '(' exprId ')'    /* $4 = PARAMETERS scip */  { del($2, $4, $6); logSkipped(@$) }
  | INDEXTYPE_IS datatype RawID '(' exprId ')' parallelClause    /* $7 = PARAMETERS scip */  { del($2, $3, $5); logSkipped(@$) }
  | indexPropertiesList                                          /* scip */ { logSkipped(@$) }
  ;
indexPropertiesList:      /*{{{*/
                        indexPropertiesItem { del($1); }
  | indexPropertiesList indexPropertiesItem { del($2); }
  ;
indexPropertiesItem:
    GLOBAL_PARTITION_BY globalPartitionedIndexTail { $$ = 0; logSkipped(@$) }
  | LOCAL localPartitionedIndex                    { $$ = 0; logSkipped(@$) }
  | indexAttribute                                 { $$ = $1;  if ($$) $$->loc(@$); logSkipped(@$) }
  ;
%type <table_PhysicalProperties> indexPropertiesItem;
globalPartitionedIndexTail:   /*{{{*/
    RANGE '(' columnIdList ')' '(' rangePartitioningList ')'       { del($3); logSkipped(@$) }
  | HASH  '(' columnIdList ')' '(' individualHashPartitionList ')' { del($3); logSkipped(@$) }
  | HASH  '(' columnIdList ')' hashPartitionsByQuantity            { del($3); logSkipped(@$) }
  ;
rangePartitioningList:            /*{{{*/
                              rangePartitioningItem
  | rangePartitioningList ',' rangePartitioningItem
  ;
rangePartitioningItem:
    PARTITION       rangeValuesClause tablePartioningDescription  { logSkipped(@$) }
  | PARTITION RawID rangeValuesClause tablePartioningDescription  { del($2); logSkipped(@$) }
  ;
tablePartioningDescription:                /*{{{*/
                                tablePartioningDescriptionItem
  | tablePartioningDescription  tablePartioningDescriptionItem
  ;
tablePartioningDescriptionItem:
    segmentAttrClause   /* <-+ */ { logSkipped(@$) }
  | COMPRESS            /*   | */ { logSkipped(@$) }
  | COMPRESS NUMERIC_ID /*   | */ { del($2); logSkipped(@$) }
  | NOCOMPRESS          /*   | */ { logSkipped(@$) }
  | OVERFLOWk /* TODO: segmentAttrsClause*/ { logSkipped(@$) }
 /*TODO:  | lobPartitionStorageItem*/
  | SUBPARTITIONS NUMERIC_ID      { del($2); logSkipped(@$) }
  | SUBPARTITIONS NUMERIC_ID STORE IN '(' columnIdList ')' { del($2,$6); logSkipped(@$) }
  | '(' subpartitionSpecList ')' { logSkipped(@$) }
  ;
subpartitionSpecList:             /* scip {{{*/
                             subpartitionSpec
  | subpartitionSpecList ',' subpartitionSpec
  ;                               /*}}}*/


                                           /*}}}*/
                                  /*}}}*/
individualHashPartitionList:      /*{{{*/
                                    individualHashPartitionItem
  | individualHashPartitionList ',' individualHashPartitionItem
  ;
individualHashPartitionItem:
    PARTITION       partitioningStorageClause
  | PARTITION RawID partitioningStorageClause { del($2); }
  ;
                                  /*}}}*/
hashPartitionsByQuantity:         /*{{{*/
    PARTITIONS RawID storeInTblspaceOpt overflowStoreInOpt  { del($2); logSkipped(@$) }
  ;
overflowStoreInOpt:
  | OVERFLOWk STORE IN '(' columnIdList ')' { del($5); logSkipped(@$) }
  ;                                  /*}}}*/
                              /*}}}*/

                          /*}}}*/
localPartitionedIndexOpt: /*{{{*/
  | localPartitionedIndex
  ;
localPartitionedIndex:
    LOCAL localPartitionedIndexBody { logSkipped(@$) }
  ;
localPartitionedIndexBody:
    indexPartitionedStoreInOpt '(' indexPartitionClauseList ')' {  logSkipped(@$) }
  | indexPartitionedStoreIn {  logSkipped(@$) }
  | /* EMPTY */
  ;
indexPartitionClauseList:
                                 indexPartitionClause
  | indexPartitionClauseList ',' indexPartitionClause
  ;
indexPartitionClause:
    PARTITION RawID indexPartitionClauseBodyOpt { del($2); logSkipped(@$) }
  | PARTITION       indexPartitionClauseBodyOpt { logSkipped(@$) }
  ;
indexPartitionClauseBodyOpt:
    segmentAttributeOrCompressionList indexSubpartitionClauseOpt {  logSkipped(@$) }
  | /* EMPTY */
  ;
indexSubpartitionClauseOpt:
  | indexSubpartitionClause
  ;
segmentAttributeOrCompressionList:
                                      segmentAttributeOrCompression
  | segmentAttributeOrCompressionList segmentAttributeOrCompression
  ;
segmentAttributeOrCompression:
    segmentAttrClause { logSkipped(@$) }
  | COMPRESS { logSkipped(@$) }
  | COMPRESS NUMERIC_ID { del($2); logSkipped(@$) }
  | NOCOMPRESS { logSkipped(@$) }
  ;
indexPartitionedStoreInOpt:
    indexPartitionedStoreIn {  logSkipped(@$) }
  |
  ;
indexPartitionedStoreIn:
      STORE IN '(' columnIdList ')' { del($4); logSkipped(@$) }
  ;
indexAttributesOpt:                  /*{{{*/
  | indexAttributes {  logSkipped(@$) }
  ;
indexAttributes:
                    indexAttribute { del($1); }
  | indexAttributes indexAttribute { del($2); }
  ;
indexAttribute:
    physicalAttributeClause { $$ = $1; } 
  | LOGGING                 { $$ = 0; logSkipped(@$) }
  | NOLOGGING               { $$ = 0; logSkipped(@$) }
  | ONLINE                  { $$ = 0; logSkipped(@$) }
  | COMPUTE STATISTICS      { $$ = 0; logSkipped(@$) }
  | tablespaceClause        { $$ = 0; logSkipped(@$) }
  | COMPRESS                { $$ = 0; logSkipped(@$) }
  | COMPRESS NUMERIC_ID     { $$ = 0; del($2); logSkipped(@$) }
  | NOCOMPRESS              { $$ = 0; logSkipped(@$) }
  | SORT                    { $$ = 0; logSkipped(@$) }
  | NOSORT_tok              { $$ = 0; logSkipped(@$) }
  | parallelClause          { $$ = 0; logSkipped(@$) }
  ;
%type <table_PhysicalProperties> indexAttribute;
                                     /*}}}*/
indexSubpartitionClause:           /*{{{*/
    STORE IN '(' columnIdList ')' { del($4); logSkipped(@$) }
  | '(' indexSubpartitionList ')' { logSkipped(@$) }
  ;
indexSubpartitionList:
                              indexSubpartitionListItem
  | indexSubpartitionList ',' indexSubpartitionListItem
  ;
indexSubpartitionListItem:
    SUBPARTITION       { logSkipped(@$) }
  | SUBPARTITION RawID { del($2); logSkipped(@$) }
  | SUBPARTITION RawID tablespaceClause { del($2); logSkipped(@$) }
  ;                                  /*}}}*/
                          /*}}}*/
  /* end of INDEX }}} */
/* Expressions    +{{{ */

 /* url links {{{*/
 /* SQL:        http://docs.oracle.com/cd/B19306_01/server.102/b14200/expressions001.htm#i1002626 */
 /* PL SQL:     http://docs.oracle.com/cd/B19306_01/appdev.102/b14261/expression_definition.htm#i34030*/
 /* CONDITIONS: http://docs.oracle.com/cd/B19306_01/server.102/b14200/conditions.htm#g1077361 */
 /*}}}*/

/* sqlExpr       {{{2 */


sqlExprList: 
                    sqlExpr                      { $$ = mkList($1); }
  | sqlExprList ',' sqlExpr                      { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprList> sqlExprList;

sqlExpr:                    /*{{{*/     /* нужно рекурсивное дерево разбора. списки здесь не нужны */
    sqlExprBase         { $$ = $1; }             
  | sqlExprCompound     { $$ = $1; }
  ;
sqlExprCompound:
    sqlExpr "**" sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr '*'  sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr '/'  sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr '+'  sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr '-'  sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr "||" sqlExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | sqlExpr MOD sqlExpr { $$ = Sm::refExprFromCallArgs(@$, @1, @2, @3, $1, $3); }
  ;

%type <sqlExpr> sqlExpr sqlExprCompound;
                            /*}}}*/
sqlExprBase:                /*{{{*/ 
    sqlExprKernel           { $$ = $1; }
  | sqlExprBaseTimeOrHquery { $$ = $1; }
  ;

sqlExprBaseTimeOrHquery:
    sqlExprKernel intervalExpressionTail      { $$ = new Sm::TimeExpr(@$, $1, $2); }
  | sqlExprKernel timezoneExprTail            { $$ = new Sm::TimeExpr(@$, $1, $2); }
   /* Parent row in Hierarhical clauses (related to condition for current row) */
  | PRIOR sqlExpr                             { $$ = new Sm::PriorExpr(@$, $2); }
  ;
%type <sqlExpr> sqlExprBase sqlExprBaseTimeOrHquery;
                            /*}}}*/

timezoneExprTail:
    AT TIMEZONE sqlExprBase     { $$ = new Sm::TimeExprTimezone(@$, $3); logSkipped(@$) }
  | AT LOCAL                    { $$ = new Sm::TimeExprTimezone(@$, Sm::TimeExprTimezone::AT_LOCAL); logSkipped(@$) }
  | AT TIMEZONE DBTIMEZONE      { $$ = new Sm::TimeExprTimezone(@$, Sm::TimeExprTimezone::AT_TIMEZONE_DBTIMEZONE); logSkipped(@$) }
  | AT TIMEZONE SESSIONTIMEZONE { $$ = new Sm::TimeExprTimezone(@$, Sm::TimeExprTimezone::AT_TIMEZONE_SESSIONTIMEZONE); logSkipped(@$) }
  ;
%type <timezone> timezoneExprTail;


sqlExprKernel: 
    selectStatement             %dprec 2 { $$ = $1; } 
  | sqlExprNonSubquery          %dprec 1 { $$ = $1; }
  | DISTINCT sqlExprNonSubquery %dprec 1 { $$ = $2; $$->setDistinct(); }
  ;

sqlExprNonSubquery: 
    sqlExprItem                                        { $$ = $1; } 
  | '(' sqlExprCompound ')'                            { $$ = new Sm::Brackets(@$, $2); }
  | '(' sqlExprBaseTimeOrHquery ')'                    { $$ = new Sm::Brackets(@$, $2); }
  | '(' sqlExprNonSubquery ')'                         { $$ = new Sm::Brackets(@$, $2); }
  | castExpr                                           { $$ = $1; }
  | '+' sqlExprKernel %prec UMINUS                     { $$ = new Sm::UnaryPlus(@$, $2); }
  | '-' sqlExprKernel %prec UPLUS                      { $$ = new Sm::UnaryMinus(@$, $2); }
  ;

dynFieldSingle:
    _DYN_FIELD  '(' plExpr ')'                                      { $$ = new Sm::FunctionDynField(@$, $3);                 }
%type <sqlExpr> dynFieldSingle;

dynTrCallSignature:
    _DYN_TR_CALL_SIGNATURE '(' plExpr                           ')' { $$ = new Sm::DynamicFuncallTranslator(@$, $3);  }
  | _DYN_TR_FUN '(' plExpr ',' datatype ',' functionCallArglist ')' { $$ = new Sm::DynamicFuncallTranslator(@$, $3, $5, $7); }
  | _DYN_TR_FUN '(' plExpr ',' datatype                         ')' { $$ = new Sm::DynamicFuncallTranslator(@$, $3, $5, 0);  }
  ;
%type <dynamicFuncallTranslator> dynTrCallSignature;

dynFieldGrammar:
    dynFieldSingle                                                  { $$ = $1; }
  | _DYN_FIELD  '(' plExpr ',' datatype ')'                         { $$ = new Sm::FunctionDynField(@$, $3, $5);             }
  | _DYN_FIELD  '(' plExpr ',' dynSubqueryGrammar ')'               { $$ = new Sm::FunctionDynField(@$, $3, $5);             }
  | dynTrCallSignature                                              { $$ = $1; }
  | _DYN_EXPR   '(' plExpr ')'                                      { $$ = new Sm::FunctionDynExpr(@$, $3); }
  // | _DYN_EXPR   '(' plExpr ',' plExpr ')'                           { $$ = new Sm::FunctionDynExpr(@$, $3, $5); }
  | _DYN_EXPR   '(' plExpr ',' datatype ')'                         { $$ = new Sm::FunctionDynExpr(@$, $3, $5); }
  ;
%type <sqlExpr> dynFieldGrammar;

sqlExprItem:                /*{{{*/
    EMPTY_ID                                                         { $$ = new Sm::EmptyIdExpr(@$); }
  | ROWID_tok                                                        { $$ = new Sm::RowIdExpr(@$); }
  | ROWNUM                                                           { $$ = new Sm::RowNumExpr(@$); }
  | NUMERIC_ID                                                       { $$ = $1; }
  | sqlExprId                                                        { $$ = $1; }
  | __STR_TAIL__                                                     { $$ = new Sm::StrTailObj(@$); }
  | __BOOL_TAIL__                                                    { $$ = new Sm::BoolTailObj(@$); }
  | __NUM_TAIL__                                                     { $$ = new Sm::NumTailObj(@$); }
  | dynFieldGrammar                                                  { $$ = $1; }
  | __DYN_LEN__  '(' plExpr ')'                                      { $$ = new Sm::DynLength(@$, $3); }
  | SQL '%' sqlExprIdPercentSuffix                                   { $$ = new Sm::CursorSqlProperties(@$, $3); }
  | CASE sqlExpr searchedCaseExpression elseClauseOpt END CASE_opt { $$ = new Sm::Case(@$, $3, $4, $2); }
  | CASE searchedCaseExpression elseClauseOpt END CASE_opt           { $$ = new Sm::Case(@$, $2, $3); }
  | CURSOR                                                           { $$ = new Sm::RefExpr(@1, new Sm::Identificator(STRING("CURSOR"), @1)); }
  | CURSOR '(' subquery ')'                                          { $$ = new Sm::CursorExpr(@$, $3); }
        /* schema.typename */
  | NEW schemaDotColumnNoNew  '(' functionCallArglistOpt ')'         { $$ = new Sm::NewCall(@$, $2, $4); }
  | NULLk                                                            { $$ = new Sm::NullExpr(@$); }
  /* TODO: - нужно именно для SQL (+ plsql, т.к. sqlExprItem включено в conditionKernel) - см. CALL ( expr ) . List<Id> [ (arglist_opt )] */
  ;
%type <sqlExpr> sqlExprItem sqlExprKernel sqlExprNonSubquery;
                            /*}}}*/

CASE_opt:
    /* EMPTY */
  | CASE
  ;

/* Outer join (+) Reference {{{3
 *
 * Oracle рекомендует использовать FROM синтаксис для OUTER JOIN, а не оператор соединения.
 *
 * Запросы внешнего соединения, которые используют join оператор (+) подчиняются
 * следующим правилам и ограничениям, которые не распространяются на FROM синтаксис для OUTER JOIN:
 *
 * 0   Нельзя указывать оператор (+) в блоке запроса, который также содержит от
 *     синтаксис join в предложении from.  1   Оператор (+) может появляться
 *     только в предложении WHERE или в контексте левой корреляции (при указании
 *     предложения TABLE) в FROM, и может быть применен только к столбцу таблицы
 *     или представления.
 * 2   Если А и В соединены несколько условиями соединения, то вы должны
 *     использовать оператор (+) во всех этих условях.
 *
 * 3   Если у вас такого нет, то Oracle Database вернет только те строки, которые
 *     являются результатом простого соединения, но без предупреждения или ошибки
 *     сообщающей Вам, что у Вас нет результатов внешнего соединения.
 *
 * 4   Оператор (+) не выполняет внешнее соединение если указать одну таблицу из
 *     внешнего запроса, и другую таблицу - из внутреннего запроса.
 *
 * 5   Нельзя использовать (+) оператор внешнего соединения таблицы с самой собой,
 *     хотя самосоединения являются допустимыми.
 *
 *     -- Следующая конструкция невалидна:
 *        SELECT employee_id, manager_id
 *        FROM employees
 *        WHERE employees.manager_id(+) = employees.employee_id;
 *
 *     -- Однако, следующее самосоединение - валидно:
 *
 *       SELECT e1.employee_id, e1.manager_id, e2.employee_id
 *       FROM employees e1, employees e2
 *       WHERE e1.manager_id(+) = e2.employee_id
 *       ORDER BY e1.employee_id, e1.manager_id, e2.employee_id;
 *
 * 6.  Оператор (+) может применяться только к столбцу, а не к произвольному
 *     выражению.
 *
 * 7.  Тем не менее, произвольное выражение может содержать один или более
 *     столбцов отмеченных оператором (+).
 *
 * 8.  Условие содержащие (+) оператор нельзя с помощью логического оператора OR
 *     использовать вместе другим условием.
 *
 * 9.  Условие WHERE не может использоваться в условии сравнения IN для сравнения
 *     столбца, помеченного оператором (+) с выражением
 *
 * 10. Если оператор Where содержит условие, которое сравнивает столбец таблицы B
 *     с константой, то оператор (+) должен быть применен к столбцу так, чтобы
 *     Oracle возвратил строки из таблицы A, для которых были сформированы
 *     NULL-значения для этого столбца. В противном случае Oracle возвращает
 *     только результаты простого соединения
 *
 *     В запросе, который выполняет внешнее соединение из более чем двух пар
 *     таблиц, одна таблица может быть пустой null-сгенерированной таблицей только
 *     для одной другой таблицы.  По этой причине, нельзя применить оператор (+) к
 *     столбцам из B в условии соединения для A и B и условии соединения для B и
 *     C.
 *
 }}}*/


queryPartitionClauseOpt:
    /* EMPTY */                                       { $$ = 0;  }
  | orderByClause                                     { $$ = $1; }
  | PARTITION BY expressionWhereList orderByClauseOpt
    {
      $$ = $4;
      if (!$$)
        $$ = new Sm::OrderBy(@$, 0, 0);
      $$->partitionBy = $3;
    }
  ;
%type <orderBy> queryPartitionClauseOpt;

dblinkSelectedEntity:
    selectedEntity '@' selectedEntity {  $$ = new Sm::RefExprDbLink(@$, $1->toIdEntitySmart(), $3->toIdEntitySmart()); }
  ;
%type <refExpr> dblinkSelectedEntity;


sqlExprId:                  /*{{{*/
    selectedEntity                                                                                   %dprec 1 { $$ = new Sm::RefExpr(@$, $1->toIdEntitySmart()); }
  | selectedEntity RawID /* KEEP */ '(' RawID /* DENSE_RANK */ RawID /* FIRST */ ORDER BY sqlExpr ')'%dprec 1 { $$ = new Sm::RefExpr(@$, $1->toIdEntitySmart()); del($2,$4,$5,$8); /* TODO: реализовать поддержку в модели */ }
  | selectedEntity RawID /* OVER */ '(' queryPartitionClauseOpt ')'                                  %dprec 1 { $$ = Sm::makeOwerExpr($1->toIdEntitySmart(), $2, $4); }
  | dblinkSelectedEntity                                                                                      { $$ = $1; }
  | outherJoinClause                                  { $$ = $1; }
  | selectedEntity '.' '*'                            { $$ = new Sm::AsteriskRefExpr(@$, $1->toIdEntitySmart()); }
  |     selectedEntity '%' sqlExprIdPercentSuffix     { $$ = new Sm::CursorProperties(@$, $1->toIdEntitySmart(), $3); }
  | ':' selectedEntity '%' sqlExprIdPercentSuffix     { $$ = new Sm::HostCursorPropertiesExpr(@$, $2->toIdEntitySmart(), $4); }
  | ':' selectedEntity                                { $$ = new Sm::RefHostExpr(@$, $2->toIdEntitySmart()); }
  | ':' selectedEntity           ':' selectedEntity   { $$ = new Sm::RefHostExpr(@$, $2->toIdEntitySmart(), $4->toIdEntitySmart()); }
  | ':' selectedEntity INDICATOR ':' selectedEntity   { $$ = new Sm::RefHostExpr(@$, $2->toIdEntitySmart(), $5->toIdEntitySmart(), true); }
  | SQL '%' BULK_ROWCOUNT '(' NUMERIC_ID ')'          { $$ = new Sm::BulkRowcountExpr(@$, $5); logSkipped(@$) }
  ;
%type <sqlExprId> sqlExprId;

outherJoinClause:
    selectedEntity '(' '+' ')'                        { $$ = new Sm::OutherJoinExpr(@$, $1->toIdEntitySmart()); }
  ;

%type <sqlExprId> outherJoinClause;

sqlExprIdList:
                       sqlExprId                 { $$ = mkList($1);            }
  | sqlExprIdList  ',' sqlExprId                 { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprIdList> sqlExprIdList;

sqlExprIdPercentSuffix:
    FOUND    { $$ = Sm::cursor_properties::CURSOR_FOUND; }
  | ISOPEN   { $$ = Sm::cursor_properties::CURSOR_ISOPEN; }
  | NOTFOUND { $$ = Sm::cursor_properties::CURSOR_NOTFOUND; }
  | RawID    { if ($1->isKeyword(STRING("ROWCOUNT"))) $$ = Sm::cursor_properties::CURSOR_ROWCOUNT; del($1); }
  ;
%type <refclass> sqlExprIdPercentSuffix;

                            /*}}}*/
/* http://docs.oracle.com/cd/E11882_01/server.112/e26088/index.htm */

whenThenPlSqlExpr:
    WHEN condition THEN condition { $$ = new Sm::CaseIfThen(@$, $2, $4); }
  ;
%type <ifThen> whenThenPlSqlExpr;


searchedCaseExpression:
                            whenThenPlSqlExpr { $$ = mkList($1); }
  | searchedCaseExpression  whenThenPlSqlExpr { $$ = $1; $$->push_back($2); }
  ;
%type <ifThenList> searchedCaseExpression;

elseClauseOpt:
                   { $$ = 0; }
  | ELSE condition { $$ = $2;  $$->loc(@$); }
  ;
%type <plCond> elseClauseOpt;
                            /*}}}*/
functionCallArgument:           /*{{{*/
    '*'                      { $$ = new Sm::FunCallArgAsterisk(@$, new Sm::AsteriskExpr(@$)); }
  | listedPlCond             { $$ = new Sm::FunCallArgExpr(@$, $1); }
  | exprId "=>" listedPlCond { $$ = new Sm::FunCallArgNamed(@$, $1, $3); }
  | extractExpression        { $$ = new Sm::FunCallArgExpr(@$, $1); }
  ;


extractExpression:
    extractKeyword FROM sqlExpr  {  $$ = new Sm::ExtractExpr(@$, $1, $3); logSkipped(@$) }
  | trimKeyword    FROM sqlExpr
    {
      Sm::TrimFromExpr *p = $1;
      p->setFromEntity($3);
      p->loc(@$);
      $$ = p;
    }
  | RawID FROM sqlExpr
    {
      Sm::ExtractedEntity kw;
      if (Sm::ExtractExpr::identifyKeyword($1, &kw)) {
        $$ = new Sm::ExtractExpr(@$, kw, $3);
        delete $1;
      }
      else
        $$ = new Sm::TrimFromExpr(@$, @1, $1, $3);
    }
  ;
%type <sqlExpr> extractExpression;


trimKeyword:
    RawID RawID
    {
      Sm::TrimFromExpr::ExtractedEntity kw;
      if (Sm::TrimFromExpr::identifyKeyword($1, &kw))
        $$ = new Sm::TrimFromExpr(@$, @2, $2, 0, kw);
      else
        YYERROR;
    }
  | RawID RawID '(' sqlExpr ')'
    {
      Sm::TrimFromExpr::ExtractedEntity kw;
      if (!Sm::TrimFromExpr::identifyKeyword($1, &kw))
        YYERROR;
      $$ = new Sm::TrimFromExpr(@$, new Sm::RefExpr(@2 + @5, Sm::Identificator::setCallArgList($2, @4, $4)), kw);
    }
  | RawID '(' sqlExpr ')'
    {
      $$ = new Sm::TrimFromExpr(@$, new Sm::RefExpr(@$, Sm::Identificator::setCallArgList($1, @3, $3)));
    }
  ;

extractKeyword:
    YEAR   { $$ = Sm::ExtractedEntity::YEAR  ; }
  | DAY    { $$ = Sm::ExtractedEntity::DAY   ; }
  | MONTH  { $$ = Sm::ExtractedEntity::MONTH ; }
  | SECOND { $$ = Sm::ExtractedEntity::SECOND; }
  ;

%type <trimFromExpression>    trimKeyword;
%type <extractedEntityKeyword> extractKeyword;


%type <functionCallArgument> functionCallArgument;
functionCallArglistOpt:
    /* EMPTY */          { $$ = 0; }
  | functionCallArglist  { $$ = $1; }
  ;
functionCallArglist:
                            functionCallArgument %dprec 1 { $$ = mkVector($1); }
  | functionCallArglist ',' functionCallArgument %dprec 2 { $$ = $1; $$->push_back($3); }
  ;
%type <functionCallArglist> functionCallArglist functionCallArglistOpt;
                            /*}}}*/
intervalExpressionTail:     /*{{{*/
    DAY  leadingFieldPrecisionBrOpt TO SECOND leadingFieldPrecisionBrOpt { $$ = new Sm::TimeExprInterval(@$, $2, $5); logSkipped(@$) }
  | YEAR leadingFieldPrecisionBrOpt TO MONTH                             { $$ = new Sm::TimeExprInterval(@$, $2); logSkipped(@$) }
  ;
%type <interval> intervalExpressionTail;

leadingFieldPrecisionBrOpt:
                       { $$ = 0; }
  | '(' NUMERIC_ID ')' { $$ = $2; $$->loc(@$); }
  ;
%type <numericValue> leadingFieldPrecisionBrOpt;
                            /*}}}*/
                            /*}}}*/

/* end of sqlExpr }}}2 */
/* CONDITIONS    {{{2 */

selectedEntityId:
    columnId   { $$ = $1; }
  | XMLTYPE    { $$ = new Sm::Identificator(STRING("XMLTYPE"  ), @1); }
  | PIPE       { $$ = new Sm::Identificator(STRING("PIPE"     ), @1); }
  | ROWID      { $$ = new Sm::Identificator(STRING("ROWID"    ), @1); }
  | SAVEPOINT  { $$ = new Sm::Identificator(STRING("SAVEPOINT"), @1); }
  | AT         { $$ = new Sm::Identificator(STRING("AT"        ), @1); }
  | TABLESPACE { $$ = new Sm::Identificator(STRING("TABLESPACE"), @1); }
  | LANGUAGE   { $$ = new Sm::Identificator(STRING("LANGUAGE"  ), @1); }
  ;

selectedEntityIdTail:
    selectedEntityId { $$ = $1; }
  | COMMIT { $$ = new Sm::Identificator(STRING("COMMIT"), @1); }
  | PRIOR  { $$ = new Sm::Identificator(STRING("PRIOR" ), @1); }
  | DELETE { $$ = new Sm::Identificator(STRING("DELETE"), @1); }
  | EXISTS { $$ = new Sm::Identificator(STRING("EXISTS"), @1); }
  | EXTEND { $$ = new Sm::Identificator(STRING("EXTEND"), @1); }
  | ROWNUM { $$ = new Sm::Identificator(STRING("ROWNUM"), @1); }
  | MODIFY { $$ = new Sm::Identificator(STRING("MODIFY"), @1); }
  | USING  { $$ = new Sm::Identificator(STRING("USING" ), @1); }
  ;
%type <id> selectedEntityId selectedEntityIdTail;

selectedEntity:
    selectedEntityId { $$ = context->getIdChain($1); }
  | selectedEntity '(' functionCallArglistOpt ')'
    {
       $$ = $1;
       Sm::Identificator *backId = $$->back();
       if (backId->callArglist)
         $$->push_back(new Sm::Identificator(@3, $3));
       else
         backId->setCalArglistAndUpdateLoc(@4, $3);
    }
  | selectedEntity '.' selectedEntityIdTail
    {
      $$ = $1;
      $$->push_back($3);
    }
  ;

%type <idEntity> selectedEntity;

commaNumOpt:
    /* EMPTY */    { $$ = 0; }
  | ',' NUMERIC_ID { $$ = $2;  }
  ;
%type <numericValue> commaNumOpt;

matchParamOpt:
    /* EMPTY */             { $$ = 0;  }
  | ',' RawID /* [icnmx] */ { $$ = $2; }
  ;
%type <id> matchParamOpt;

/* TODO: Model conditions can be used only in the MODEL clause of a SELECT statement. */
/*  | ANY                                                                     {} */
/*  | selectedEntity IS ANY */ /* dimension_column */  /*                     {} */
/*  | sqlExpr IS PRESENT                                                      {} */

multisetTail:
           SUBMULTISET selectedEntity   { $$ = new Sm::pl_expr::Submultiset(@$, $2->toIdEntitySmart()); logSkipped(@$) }
  |    NOT SUBMULTISET selectedEntity   { $$ = new Sm::pl_expr::Submultiset(@$, $3->toIdEntitySmart()); $$->setNot();  logSkipped(@$) }
  | IS     RawID SET /* TODO: $2 = A */ { if (!$2->isKeyword(STRING("A"))) YYERROR; $$ = new Sm::pl_expr::Submultiset(@$); logSkipped(@$) }
  | IS NOT RawID SET /* TODO: $4 = A */ { if (!$3->isKeyword(STRING("A"))) YYERROR; $$ = new Sm::pl_expr::Submultiset(@$); $$->setNot(); logSkipped(@$) }
  ;
%type <submultiset> multisetTail;

plExprCommaList:
                        plExpr  { $$ = mkList($1); }
  | plExprCommaList ',' plExpr  { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprList> plExprCommaList;

plExprListList:
                       plExprList { $$ = mkList($1); }
  | plExprListList ',' plExprList { $$ = $1; Sm::pl_expr::concatBracketedList($$, $3); }
  ;
%type <bracketedPlExprListList> plExprListList;

plExprList:
        plExprCommaList                 { $$ = new Sm::pl_expr::BracketedPlExprList(@$, $1, false); }
  | '(' plExprCommaList ',' plExpr ')'  { $$ = new Sm::pl_expr::BracketedPlExprList(@$, $2, true); $$->list->push_back($4);  }
  ;
%type <bracketedPlExprList> plExprList;



plExprWhere:
    plExprWhereBase         { $$ = $1; }
  | plExprWhere "**" plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere '*'  plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere '/'  plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere '+'  plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere '-'  plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere "||" plExprWhere { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExprWhere MOD  plExprWhere { $$ = Sm::refExprFromCallArgs(@$, @1, @2, @3, $1, $3); }
  ;
%type <sqlExpr> plExprWhere;

plExprWhereBase:
    plExprWhereKernel                        { $$ = $1; }
  | plExprWhereKernel intervalExpressionTail { $$ = new Sm::TimeExpr(@$, $1, $2); }
  | plExprWhereKernel timezoneExprTail       { $$ = new Sm::TimeExpr(@$, $1, $2); }
  | PRIOR plExprWhere                        { $$ = new Sm::PriorExpr(@$, $2); }
  | DISTINCT plExprWhereKernel               { $$ = $2; $$->setDistinct(); }
  ;
%type <sqlExpr> plExprWhereBase;

plExprWhereKernel:
    sqlExprItem                                        { $$ = $1; }
  | whereSubquery %dprec 2                             { $$ = $1; }
  | '(' plExprWhere  ')' %dprec 1                           { $$ = new Sm::Brackets(@$, $2); }
  | castExpr                                                { $$ = $1; }
  | '+' plExprWhereKernel %prec UMINUS                      { $$ = new Sm::UnaryPlus(@$, $2); }
  | '-' plExprWhereKernel %prec UPLUS                       { $$ = new Sm::UnaryMinus(@$, $2); }
  ;
%type <sqlExpr> plExprWhereKernel;


castSingle:
    CAST '(' plExpr AS datatype ')'                    { $$ = new Sm::Cast(@$, $3, $5); }
  ;
castMultiset:
    CAST '(' MULTISET '(' subquery ')' AS datatype ')' { $$ = new Sm::CastMultiset(@$, $5, $8); logSkipped(@$) }
  ;

castExpr:
    castSingle   { $$ = $1; }
  | castMultiset { $$ = $1; }
  ;
%type <sqlExpr> castMultiset castSingle castExpr;


exprWhereList:
                      plExprWhere                      { $$ = mkList((Sm::PlExpr*)($1)); }
  | exprWhereList ',' plExprWhere                      { $$ = $1; $$->push_back($3); }
  ;
%type <plExprList> exprWhereList;


expressionWhereList:
        exprWhereList                     { $$ = $1; }
  | '(' exprWhereList ',' plExprWhere ')' { $$ = $2; $$->push_back($4); $$->setHasBrackets(); }
  ;
%type <plExprList> expressionWhereList ;


plExpr:
    plExprBase         { $$ = $1; }
  | plExpr "**" plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr '*'  plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr '/'  plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr '+'  plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr '-'  plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr "||" plExpr { $$ = new Sm::AlgebraicCompound(@$, $1, $2, $3); }
  | plExpr MOD  plExpr { $$ = Sm::refExprFromCallArgs(@$, @1, @2, @3, $1, $3); }
%type <sqlExpr> plExpr;


plExprKernel:
    sqlExprItem                                        { $$ = $1; }
  | subquery               %dprec 2                    { $$ = $1; }
  | '(' plExpr  ')' %dprec 1                           { $$ = new Sm::Brackets(@$, $2); }
  | castExpr                                           { $$ = $1; }
  | '+' plExprKernel %prec UMINUS                      { $$ = new Sm::UnaryPlus(@$, $2); }
  | '-' plExprKernel %prec UPLUS                       { $$ = new Sm::UnaryMinus(@$, $2); }
  ;
%type <sqlExpr> plExprKernel;

plExprBase:
    plExprKernel                        { $$ = $1; }
  | plExprKernel intervalExpressionTail { $$ = new Sm::TimeExpr(@$, $1, $2); }
  | plExprKernel timezoneExprTail       { $$ = new Sm::TimeExpr(@$, $1, $2); }
  | PRIOR plExpr                        { $$ = new Sm::PriorExpr(@$, $2); }
  | DISTINCT plExprKernel               { $$ = $2; $$->setDistinct(); }
  ;
%type <sqlExpr> plExprBase;

likeCondition:
    plExpr     likeOp plExpr { $$ = new Sm::pl_expr::Like(@$, $1, $3, $2); }
  | plExpr NOT likeOp plExpr { $$ = new Sm::pl_expr::Like(@$, $1, $4, $3); $$->setNot(); }
  ;
%type <likeCond> likeCondition;

whereConditionKernel:
    /* Multiset Conditions */
    selectedEntity multisetTail
    {
      Sm::pl_expr::Submultiset *submultiset = $2;
      submultiset->exprEntity = $1->toIdEntitySmart();
      $$ = submultiset;
      $$->loc(@$);
    }
  | selectedEntity IS     EMPTY             { $$ = new Sm::pl_expr::IsEmpty(@$, $1->toIdEntitySmart()); }
  | selectedEntity IS NOT EMPTY             { $$ = new Sm::pl_expr::IsEmpty(@$, $1->toIdEntitySmart()); $$->setNot(); }
  | plExpr     MEMBER OF_opt selectedEntity { $$ = new Sm::pl_expr::MemberOf(@$, $1, $4->toIdEntitySmart()); }
  | plExpr NOT MEMBER OF_opt selectedEntity { $$ = new Sm::pl_expr::MemberOf(@$, $1, $5->toIdEntitySmart()); $$->setNot(); }
    /* XML Conditions */
  | EQUALS_PATH '(' exprId                ',' RawID    commaNumOpt ')' { $$ = new Sm::pl_expr::Path(@$, $3, $5, $6); logSkipped(@$) }
  | UNDER_PATH  '(' exprId                ',' RawID ',' NUMERIC_ID ')' { $$ = new Sm::pl_expr::Path(@$, $3, $5, $7); logSkipped(@$) }
  | UNDER_PATH  '(' exprId ',' NUMERIC_ID ',' RawID    commaNumOpt ')' { $$ = new Sm::pl_expr::Path(@$, $3, $5, $7, $8); logSkipped(@$) }
    /* Pattern-matching Conditions */    /* V ret string */
  | REGEXP_LIKE '(' plExpr ',' plExpr matchParamOpt ')' { $$ = new Sm::pl_expr::RegexpLike(@$, $3, $5, $6); logSkipped(@$) }
  | likeCondition                                       { $$ = $1; }
  | likeCondition ESCAPE plExpr                         { Sm::pl_expr::Like * likeCond = $1; likeCond->esc_char = $3; $$ = likeCond; $$->loc(@$); }
    /* BETWEEN Conditions */
  | plExpr     BETWEEN plExpr AND plExpr                { $$ = new Sm::pl_expr::Between(@$, $1, $3, $5); }
  | plExpr NOT BETWEEN plExpr AND plExpr                { $$ = new Sm::pl_expr::Between(@$, $1, $4, $6); $$->setNot(); }
    /* */
  | EXISTS      '(' subquery ')'                        { $$ = new Sm::pl_expr::Exists(@$, $3); }
  | EXISTS      '(' dynFieldSingle ')'                  { $$ = new Sm::pl_expr::Exists(@$, $3); }
  | plExpr IS     INFINITE                              { $$ = new Sm::pl_expr::IsInfinite(@$, $1); }
  | plExpr IS NOT INFINITE                              { $$ = new Sm::pl_expr::IsInfinite(@$, $1); $$->setNot(); }
  | plExpr IS     NAN_token                             { $$ = new Sm::pl_expr::IsNan( @$, $1); }
  | plExpr IS NOT NAN_token                             { $$ = new Sm::pl_expr::IsNan( @$, $1); $$->setNot(); }
  | plExpr IS     NULLk                                 { $$ = new Sm::pl_expr::IsNull(@$, $1); }
  | plExpr IS NOT NULLk                                 { $$ = new Sm::pl_expr::IsNull(@$, $1); $$->setNot(); }
  | plExpr IS     OF      '(' ofTypes ')'               { $$ = new Sm::pl_expr::OfTypes(@$, $1, $5); }
  | plExpr IS NOT OF      '(' ofTypes ')'               { $$ = new Sm::pl_expr::OfTypes(@$, $1, $6); $$->setNot(); }
  | plExpr IS     OF_TYPE '(' ofTypes ')'               { $$ = new Sm::pl_expr::OfTypes(@$, $1, $5); }
  | plExpr IS NOT OF_TYPE '(' ofTypes ')'               { $$ = new Sm::pl_expr::OfTypes(@$, $1, $6); $$->setNot(); }
  | '(' plExprCommaList ',' plExpr ')' equalityOp quantorOpOpt '(' plExprListList ')' { $$ = new Sm::pl_expr::ComparsionList(@$, new Sm::List<Sm::SqlExpr>($2, $4), $6, $7, $9); }
  | '(' plExprCommaList ',' plExpr ')'     IN     quantorOpOpt '(' plExprListList ')' { $$ = new Sm::pl_expr::ComparsionList(@$, new Sm::List<Sm::SqlExpr>($2, $4), Sm::comparsion_list::IN, $7, $9); }
  | '(' plExprCommaList ',' plExpr ')' NOT IN     quantorOpOpt '(' plExprListList ')'
     {
       $$ = new Sm::pl_expr::ComparsionList(@$, new Sm::List<Sm::SqlExpr>($2, $4), Sm::comparsion_list::IN, $8, $10);
       $$->setNot();
     }
  ;                                              /* XXX: для IN - не нужно ? */

whereCondition:
    brWhereCondition     %dprec 1 { $$ = $1; }
  | plExpr               %dprec 2 { $$ = $1; }
  ;

brBrWhereCond:
    '(' brWhereCondition ')' { $$ = new Sm::pl_expr::BracketsLogicalExpr(@$, $2); }
  ;

brBrWhereCondSpec:
    '(' brWhereCondition ')' { $$ = new Sm::pl_expr::BracketsLogicalExpr(@$, $2); }
  | sqlExprItem { $$ = $1; }
  ;

brWhereCondition:
    whereConditionKernel                                    %dprec 9 { $$ = $1; }
  | plExpr     IN quantorOpOpt expressionWhereList                   { $$ = new Sm::pl_expr::Comparsion(@$, $1, $2, $3, $4); }
  | plExpr NOT IN quantorOpOpt expressionWhereList                   { $$ = new Sm::pl_expr::Comparsion(@$, $1, $3, $4, $5); $$->setNot(); }
  | brBrWhereCond comparsion quantorOpOpt brBrWhereCondSpec %dprec 8 { $$ = new Sm::pl_expr::Comparsion(@$, $1, $2, $3, mkList((Sm::PlExpr*)$4)); }
  | plExpr comparsion quantorOpOpt expressionWhereList      %dprec 7 { $$ = new Sm::pl_expr::Comparsion(@$, $1, $2, $3, $4); }
  | NOT whereCondition  %prec UNOT                                   { $$ = $2; $$->setNot(); }
  | whereCondition AND whereCondition                       %dprec 5 { $$ = new Sm::pl_expr::LogicalCompound(@$, $1, $3, 0); }
  | whereCondition OR  whereCondition                       %dprec 5 { $$ = new Sm::pl_expr::LogicalCompound(@$, $1, $3, 1); }
  | whereCondition dynTail                                  %dprec 4 { $$ = new Sm::DynTailExpr(@$, $1, $2, /*inverse*/ false); }
  | dynTail whereCondition                                  %dprec 4 { $$ = new Sm::DynTailExpr(@$, $2, $1, /*inverse*/ true); }
  | brBrWhereCond                                           %dprec 6 { $$ = $1; }
  ;


%type <plCond> brWhereCondition brBrWhereCond brBrWhereCondSpec;

dynTail:
    _DYN_TAIL '(' whereCondition ')' { $$ = $3; }
  ;

condition:
    whereCondition         { $$ = $1; }
  // | whereCondition dynTail { $$ = new Sm::DynTailExpr(@$, $1, $2); }
  ;

plCond:
    whereCondition { $$ = $1; }
  ;

listedExpressionWhereList:
        plExprWhere                       { $$ = mkList((Sm::PlExpr*)($1)); }
  | '(' exprWhereList ',' plExprWhere ')' { $$ = $2; $$->push_back($4); $$->setHasBrackets(); }
  ;
%type <plExprList> listedExpressionWhereList;

listedWhereConditionItem:
    whereConditionKernel                                              { $$ = $1; }
  | plExpr                                                            { $$ = $1; }
  | plExpr     IN     quantorOpOpt listedExpressionWhereList          { $$ = new Sm::pl_expr::Comparsion(@$, $1, $2, $3, $4); }
  | plExpr NOT IN     quantorOpOpt listedExpressionWhereList          { $$ = new Sm::pl_expr::Comparsion(@$, $1, $3, $4, $5); $$->setNot(); }
  | plExpr comparsion quantorOpOpt listedExpressionWhereList %dprec 1 { $$ = new Sm::pl_expr::Comparsion(@$, $1, $2, $3, $4); }
  ;                                              /* XXX: для IN - не нужно ? */

listedPlCond:
    listedWhereConditionItem %dprec 3      { $$ = $1; }
  | '(' condition ')'        %dprec 2      { $$ = new Sm::pl_expr::BracketsLogicalExpr(@$, $2); }
  | NOT listedPlCond         %prec UNOT    { $$ = $2; $$->setNot(); }
  | listedPlCond AND listedPlCond %dprec 4 { $$ = new Sm::pl_expr::LogicalCompound(@$, $1, $3, 0); }
  | listedPlCond OR  listedPlCond %dprec 4 { $$ = new Sm::pl_expr::LogicalCompound(@$, $1, $3, 1); }
  ;


%type <plCond> dynTail condition plCond whereCondition  whereConditionKernel listedWhereConditionItem listedPlCond;

ofTypes:                                  /*{{{*/
                ofType { $$ = mkList($1); }
  | ofTypes ',' ofType { $$ = $1; $$->push_back($3); }
  ;
%type <ofTypes> ofTypes;

schemaDotTypeId:
              typeId   { $$ = new Sm::Id2(@$, $1); }
  | RawID '.' typeId   { $$ = new Sm::Id2(@$, $3, $1); }
  ;
%type <id2> schemaDotTypeId;

ofType:
    ONLY_opt schemaDotTypeId { $$ = new Sm::pl_expr::OfType(@$, $1, $2); }
  ;
%type <ofType> ofType;                    /*}}}*/


comparsion:                               /*{{{*/
    '='    { $$ = $1; }
  | NE     { $$ = $1; }
  | '<'    { $$ = $1; }
  | '>'    { $$ = $1; }
  | LE     { $$ = $1; }
  | GE     { $$ = $1; }
  ;
%type <plComparsionOp> comparsion;
                                          /*}}}*/
equalityOp:                               /*{{{*/
    '='    { $$ = Sm::comparsion_list::EQ; }
  | NE     { $$ = Sm::comparsion_list::NE; }
  ;
%type <comparsionListOp> equalityOp;            /*}}}*/
quantorOpOpt:                             /*{{{*/
    ANY         { $$ = Sm::QuantorOp::ANY;   }
  | SOME        { $$ = Sm::QuantorOp::SOME;  }
  | ALL         { $$ = Sm::QuantorOp::ALL;   }
  | /* EMPTY */ { $$ = Sm::QuantorOp::EMPTY; } /* если пусто - то это не списковое выражение */
  ;
%type <quantorOp> quantorOpOpt;
                                          /*}}}*/
NOT_opt:                                  /*{{{*/
         { $$ = false; }
  | NOT  { $$ = true; }
  ;
%type <boolval> NOT_opt;                   /*}}}*/
OF_opt:                                   /*{{{*/
         { $$ = 0; }
  | OF   { $$ = 1; }
  ;
%type <intval> OF_opt;                    /*}}}*/
ONLY_opt:                                 /*{{{*/
         { $$ = true; }
  | ONLY { $$ = false; }
  ;
%type <boolval> ONLY_opt;                  /*}}}*/
likeOp:                                   /*{{{*/
    LIKE   { $$ = Sm::like_cathegory::SIMPLE_LIKE;  }
  | LIKEC  { $$ = Sm::like_cathegory::LIKEC; }
  | LIKE2  { $$ = Sm::like_cathegory::LIKE2; }
  | LIKE4  { $$ = Sm::like_cathegory::LIKE4; }
  ;
%type <likeCathegory> likeOp;                    /*}}}*/

  /* }}}2 */
/* plsql utility {{{2 */

pattern:
    exprId       { del($1); logSkipped(@$) }
  | ESCAPE RawID { del($2); logSkipped(@$) }
  ;


/* end of plsql utility }}}2 */
  /* end of SQL Expressions }}} */
/* SELECT         +{{{ */

selectStatement:
    subquery { $$ = $1; }
  ;

%type <subquery> selectStatement ;

subqueryFactoringList:      /*{{{*/
                              subqueryFactoringItem { $$ = mkList($1); }
  | subqueryFactoringList ',' subqueryFactoringItem { $$ = $1; $$->push_back($3); }
  ;
%type <factoringList> subqueryFactoringList;

factoringSubqueryExpr:
    subquery        { $$ = $1; }
  | dynFieldGrammar { $$ = $1; }
  ;

%type <sqlExpr> factoringSubqueryExpr;

subqueryFactoringItem:
    exprId brColumnListOpt AS '(' factoringSubqueryExpr ')' /* TODO: searchClauseOpt */ cycleClauseOpt { $$ = new Sm::FactoringItem(@$, $1, $2, $5); }
  ;
%type <factoringItem> subqueryFactoringItem;

brColumnListOpt:
    /* EMPTY */          { $$ = 0; }
  | '(' columnIdList ')' { $$ = $2;  }
  ;
%type <idList> brColumnListOpt;

cycleClauseOpt:
    /* EMPTY */
  | CYCLE columnIdList SET exprId TO exprId DEFAULT exprId { del($2,$4,$6,$8); logSkipped(@$) }
  ;
                            /*}}}*/
     
forUpdateClauseOpt:         /*{{{*/
    /* EMPTY */                                { $$ = 0; }
  | FOR UPDATE forUpdateOfOpt forUpdateTailOpt { $$ = $4; $$->entityList = $3; $$->loc(@$); logSkipped(@$) }
  | LOCK IN SHARE MODE                         { $$ = 0; logSkipped(@$) }
  | LOCK IN SHARE MODE WAIT                    { $$ = 0; logSkipped(@$) }
  | LOCK IN SHARE MODE NOWAIT                  { $$ = 0; logSkipped(@$) }
  ;
%type <forUpdate> forUpdateClauseOpt;

forUpdateOfOpt:                   
                          { $$ = 0; }
  | OF selectedEntityList { $$ = $2; }  
  ;

selectedEntityList: 
                           selectedEntity  { $$ = mkList($1->toIdEntitySmart()); }
  | selectedEntityList ',' selectedEntity  { $$ = $1; $$->push_back($3->toIdEntitySmart()); }
  ;
%type <entityList> selectedEntityList forUpdateOfOpt;

forUpdateTailOpt:
                    { $$ = new Sm::ForUpdateClause(); }
  | SKIP_LOCKED     { $$ = new Sm::ForUpdateClause(@$, Sm::ForUpdateClause::SKIP_LOCKED); }
  | NOWAIT          { $$ = new Sm::ForUpdateClause(@$, Sm::ForUpdateClause::NOWAIT); }
  | WAIT NUMERIC_ID { $$ = new Sm::ForUpdateClause(@$, $2); }
  ;
%type <forUpdate> forUpdateTailOpt;

                            /*}}}*/
orderByClause:              /*{{{*/
    ORDER          BY orderByList { $$ = new Sm::OrderBy(@$, $3, false); }
  | ORDER SIBLINGS BY orderByList { $$ = new Sm::OrderBy(@$, $4, true); }
  ;
%type <orderBy> orderByClause;
orderByClauseOpt:           
    /* EMPTY */   { $$ = 0; }
  | orderByClause { $$ = $1; }
  ;
%type <orderBy> orderByClauseOpt;

orderByList: 
                     orderByItem                 { $$ = mkList($1); }
  | orderByList  ',' orderByItem                 { $$ = $1; $$->push_back($3); }
  ;
%type <orderByItems> orderByList;

orderByItem:
    sqlExpr /*ASC*/             { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC             ); }
  | sqlExpr /*ASC*/ NULLS_FIRST { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC_NULLS_FIRST ); }
  | sqlExpr /*ASC*/ NULLS_LAST  { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC_NULLS_LAST  ); }
  | sqlExpr ASC                 { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC             ); }
  | sqlExpr DESC                { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::DESC            ); }
  | sqlExpr ASC     NULLS_FIRST { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC_NULLS_FIRST ); }
  | sqlExpr DESC    NULLS_LAST  { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::DESC_NULLS_LAST ); }
  | sqlExpr ASC     NULLS_LAST  { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::ASC_NULLS_LAST  ); }
  | sqlExpr DESC    NULLS_FIRST { $$ = new Sm::OrderByItem(@$, $1, Sm::OrderByItem::DESC_NULLS_FIRST); }
  ;
%type <orderByItem> orderByItem;

                            /*}}}*/

whereSubquery:
    '(' queryBlock ')' { $$ = new Sm::SelectBrackets(@$, new Sm::SelectSingle(@$, $2)); }
  | '('subqueryKernel subqueryUnionOp subqueryBody ')'{ Ptr<Sm::Subquery> uni = new Sm::UnionQuery(@$, $2, $3, $4); $$ = new Sm::SelectBrackets(@$, uni);  uni->pullUpOrderGroup($4); }
  ;


%type <subquery> whereSubquery;


subquery:                   /*{{{*/
    subqueryBody { $$ = $1; } /* TODO: обновлять forUpdate и order by для union (выцепляя его из последнего элемента), если forUpdate есть */
  ;
%type <subquery> subquery;

subqueryBody:
    subqueryKernel                              %dprec 1 { $$ = $1; }
  | subqueryKernel subqueryUnionOp subqueryBody %dprec 2 { $$ = new Sm::UnionQuery(@$, $1, $2, $3);  $$->pullUpOrderGroup($3); }
  ;

dynSubqueryGrammar:
    _DYN_SUBQUERY   '(' RawID selectedEntityRefExpr_opt ')' intoClauseOpt { $$ = new Sm::DynSubquery(@$, $3, $4, $6); }
  ;

selectedEntityRefExpr_opt:
    /* EMPTY */                  { $$ = 0;  }
  | ',' selectedEntityRefExpr    { $$ = $2; }
  ;

%type <refExpr> selectedEntityRefExpr_opt;

subqueryKernel:
    queryBlock         { $$ = new Sm::SelectSingle(@$, $1); $$->pullUpOrderGroup($1);  }
  | '(' subqueryBody ')' orderByClauseOpt /* TODO: orderByClauseOpt   forUpdateClauseOpt */ { $$ = new Sm::SelectBrackets(@$, $2); $$->orderBy = $4; }
  | dynSubqueryGrammar { $$ = $1; }

  ;

%type <subquery> subqueryBody subqueryKernel  dynSubqueryGrammar;

AsIs:
    AS | IS
  ;

subqueryUnionOp:
    UNION     { $$ = Sm::sql_union::SIMPLE_UNION; }
  | UNION ALL { $$ = Sm::sql_union::UNION_ALL;    }
  | INTERSECT { $$ = Sm::sql_union::INTERSECT;    }
  | MINUS     { $$ = Sm::sql_union::MINUS;        }
  ;
%type <subqueryUnionOp> subqueryUnionOp;
                            /*}}}*/ 


columnNQIdList:
                    columnId                     { $$ = mkList($1); }
  | columnNQIdList  columnId                     { $$ = $1; $$->push_back($2); }
  ;
%type <idList> columnNQIdList;


queryHintsItems:
          queryHintsItemsSuffix  %dprec 2 { $$ = $1; }
  | RawID queryHintsItemsSuffix  %dprec 1 { $$ = $2; Sm::QueryHint::checkFrontKeyword($1); }
  | RawID {
      if (Sm::QueryHint *p = Sm::QueryHint::checkFrontKeyword($1))
        $$ = mkList(p);
      else
        $$ = 0;
    }
  | RULES                        { $$ = 0; }
  ;

optimizeHints:
    queryHintsItems '*' '/'  { $$ = $1; }

%type <optimizeHints> optimizeHints queryHintsItems;

queryHintsBrArgs:
    '(' columnId ')'                  { $$ = mkList($2); }
  | '(' columnId columnNQIdList ')'   { $$ = $3; $$->push_front($2); }
  | '(' columnId ',' columnIdList ')' { $$ = $4; $$->push_front($2); }
  ;
%type <idList> queryHintsBrArgs;

queryHint:
    INDEX queryHintsBrArgs { $$ = new Sm::QueryHint(@$, $2); }
  | RawID queryHintsBrArgs { $$ = new Sm::QueryHint(@$, $2); $$->parseCathegory($1); }
  | INDEX columnId         { $$ = new Sm::QueryHint(@$, mkList($2)); }
  ;

%type <optimizeHint> queryHint;

queryHintsItemsSuffix:
                           queryHint { $$ = mkList($1); }
  | queryHintsItemsSuffix  queryHint { $$ = $1; $$->push_back($2); }
  ;
%type <optimizeHints> queryHintsItemsSuffix;



queryBlock:                     /*{{{*/
    withSubqueryFactoringOpt queryBlockHints { $$ = $2; $$->factoringList = $1; }
  ;
%type <queryBlock> queryBlock;

queryBlockHints:
    SELECT      queryBlockTail                 { $$ = $2; }
  | SELECT_HINT optimizeHints   queryBlockTail { $$ = $3; $$->hints = $2; }
  ;
%type <queryBlock> queryBlockHints;


queryBlockTail:
      selectPrefixOpt             /* 1*/
      selectList                  /* 2*/
      bulkCollectOpt              /* 3*/
      intoClauseOpt               /* 4*/
      FROM                        /* 5*/
        fromBody                  /* 6*/
      whereClauseOpt              /* 7*/
      queryTailData               /* 8*/
      modelClauseOpt              /* 9*/
      orderByClauseOpt            /*10*/
      forUpdateClauseOpt          /*11*/
    {
      $$ = new Sm::QueryBlock(@$, 0, $1, $2, $4, $6, $7, $8, $9);
      // if ($8) {
      //   $$ = new Sm::QueryBlock(@$, 0, $1, $2, $4, $6, $7, $8->hierarhicalQuery, $8->groupBy, $8->having, $9);
      //   delete $8;
      // }
      // else
      //   $$ = new Sm::QueryBlock(@$, 0, $1, $2, $4, $6, $7, 0, 0, 0, $9);
      $$->orderBy   = $10;
      $$->forUpdate = $11;
      $$->isBulkCollect($3);
    }

%type <queryBlock> queryBlockTail;


withSubqueryFactoringOpt:
    /* EMPTY */                { $$ = 0;  }
  | WITH subqueryFactoringList { $$ = $2; }
  ;
%type <factoringList> withSubqueryFactoringOpt;

queryTailData:
    /* EMPTY */                { $$ = 0; }
  | havingClause               { $$ = new Sm::QueryTailData($1); }
  | groupByClause              { $$ = new Sm::QueryTailData(0, $1); }
  | hierarhicalQueryClause     { $$ = new Sm::QueryTailData(0, 0, $1); }
  /* { havingClause groupByClause } */
  | groupByClause havingClause { $$ = new Sm::QueryTailData($2, $1); }
  | havingClause groupByClause { $$ = new Sm::QueryTailData($1, $2); }
  /* { groupByClause hierarhicalQueryClause  } */
  | groupByClause hierarhicalQueryClause { $$ = new Sm::QueryTailData(0, $1, $2); }
  | hierarhicalQueryClause groupByClause { $$ = new Sm::QueryTailData(0, $2, $1); }
  /* { havingClause hierarhicalQueryClause } */
  | havingClause hierarhicalQueryClause { $$ = new Sm::QueryTailData($1, 0, $2); }
  | hierarhicalQueryClause havingClause { $$ = new Sm::QueryTailData($2, 0, $1); }
  /*    1               2                 3
  /* { havingClause groupByClause hierarhicalQueryClause } -> 123, 132, 231, 312, 213, 321 */
  | havingClause groupByClause hierarhicalQueryClause { $$ = new Sm::QueryTailData($1, $2, $3); }
  | havingClause hierarhicalQueryClause groupByClause { $$ = new Sm::QueryTailData($1, $3, $2); }
  | groupByClause hierarhicalQueryClause havingClause { $$ = new Sm::QueryTailData($3, $1, $2); }
  | hierarhicalQueryClause havingClause groupByClause { $$ = new Sm::QueryTailData($2, $3, $1); }
  | groupByClause havingClause hierarhicalQueryClause { $$ = new Sm::QueryTailData($2, $1, $3); }
  | hierarhicalQueryClause groupByClause havingClause { $$ = new Sm::QueryTailData($3, $2, $1); }
  ;

%type <queryTailData> queryTailData;
selectPrefix:                    /*{{{*/
    UNIQUE    { $$ = Sm::query_block::UNIQUE;   }
  | ALL       { $$ = Sm::query_block::ALL;      }
  ;
selectPrefixOpt:
                 { $$ = Sm::query_block::EMPTY;    }
  | selectPrefix { $$ = $1; }
  ;
%type <queryPrefix> selectPrefixOpt selectPrefix;
                                    /*}}}*/
intoClauseOpt:                      /*{{{*/
                       { $$ = 0; }
  | INTO sqlExprIdList { $$ = $2; }
  ;
%type <sqlExprIdList> intoClauseOpt;
                                    /*}}}*/
fromBody:                           /*{{{*/
                 fromBodyItem %dprec 1 { $$ = mkList($1); }
  | fromBody ',' fromBodyItem %dprec 2 { $$ = $1; $$->push_back($3); }
  ;
%type <fromBody> fromBody;

fromBodyItemBrSingle:
    '(' tableReference ')' { $$ = $2; $$->setHasBrackets(); }

fromBodyItemBr:
    fromBodyItemBrSingle                         { $$ = $1; }
  | fromBodyItemBrSingle  viewConstraintColumnId { $$ = $1; $$->alias = $2; }

%type <fromSingle> fromBodyItemBrSingle fromBodyItemBr;

fromBodyItem:                           /*{{{*/
    tableReference         { $$ = $1; }
  | fromBodyItemBr         { $$ = $1; }
  | tableReferenceSubquery { $$ = $1; }
  | '(' joinClause ')'     { $$ = $2; $$->setHasBrackets(); }
  |     joinClause         { $$ = $1; }
  ;
%type <fromBodyItem> fromBodyItem;
                                                    
joinClause:                                 /*{{{*/
    tableReference joinClauseTail 
    { 
      Sm::FromJoin *p = $2;
      p->loc(@$);
      $2->lhs = $1;
      $$ = $2;
    }
  | tableReferenceSubquery joinClauseTail 
    { 
      Sm::FromJoin *p = $2;
      p->loc(@$);
      $2->lhs = $1;
      $$ = $2;
    }
  | joinClause joinClauseTail  
    { 
      Sm::FromJoin *p = $2;
      p->loc(@$);
      $2->lhs = $1;
      $$ = $2;
    }
  ;
%type <fromBodyItem> joinClause;          

joinTableReference: 
    tableReference         { $$ = $1; }
  | tableReferenceSubquery { $$ = $1; }
  ;
%type <fromSingle> joinTableReference;

joinClauseTail: 
    joinOp partitionByExprListOpt joinTableReference partitionByExprListOpt joinConditionOpt  
    {
      Sm::Join * joinCond = $5;
      joinCond->op = $1;
      Sm::FromSingle * fromSingle = $3;
      if ($2)
        fromSingle->partitionExprList = $2;
      else if ($4)
        fromSingle->partitionExprList = $4;
      $$ = new Sm::FromJoin(@$, 0, joinCond, fromSingle);
    }
  ;

%type <joinClauseTail> joinClauseTail;


outherJoinOp:                                   /*{{{*/
    FULL_JOIN        { $$ = Sm::JoinQueries::FULL;  }
  | FULL_OUTER_JOIN  { $$ = Sm::JoinQueries::FULL;  }
  | RIGHT_JOIN       { $$ = Sm::JoinQueries::RIGHT; }
  | RIGHT_OUTER_JOIN { $$ = Sm::JoinQueries::RIGHT; }
  | LEFT_JOIN        { $$ = Sm::JoinQueries::LEFT;  }
  | LEFT_OUTER_JOIN  { $$ = Sm::JoinQueries::LEFT;  }
  ;                       
outherNaturalJoinOp:
    outherJoinOp          { $$ = $1; }
  | NATURAL outherJoinOp  { $$ = (Sm::JoinQueries::Operation)((unsigned int)$2 | (unsigned int)Sm::JoinQueries::NATURAL); }
  ;
joinOp:
         /* INNER */ JOIN { $$ = Sm::JoinQueries::INNER; }
  |         INNER_JOIN    { $$ = Sm::JoinQueries::INNER; }
  |         CROSS    JOIN { $$ = Sm::JoinQueries::CROSS; }
  | NATURAL          JOIN { $$ = Sm::JoinQueries::NATURAL_INNER; }
  | NATURAL INNER_JOIN    { $$ = Sm::JoinQueries::NATURAL_INNER; }
  | outherNaturalJoinOp   { $$ = $1; } 
  ;
%type <joinOp> joinOp outherNaturalJoinOp outherJoinOp;
                                                /*}}}*/
tableReference:                                 /*{{{*/
    tableReferenceBody flashbackQueryClauseOpt asAliasOpt { $$ = new Sm::FromSingle(@$, $1, $2, $3); }
  ;
%type <fromSingle> tableReference;

tableReferenceBody:                                 /*{{{*/
    ONLY '(' queryTableExpression ')' { $$ = $3; $$->queryCathegory = Sm::QueriedTable::ONLY; $$->loc(@$); }
  | queryTableExpression              { $$ = $1; }
  | queryTableExpression PIVOT XML_opt '(' pivotAggregateList  FOR columnOrBrColumnList IN '(' pivotRange ')' ')' 
    { $$ = $1; $$->queryCathegory = Sm::QueriedTable::PIVOT; $$->loc(@$); logSkipped(@$) }
  | queryTableExpression UNPIVOT includeOrExclude '(' columnOrBrColumnList FOR columnOrBrColumnList IN '(' columnAsConstList ')' ')' 
    { $$ = $1; $$->queryCathegory = Sm::QueriedTable::UNPIVOT; $$->loc(@$); logSkipped(@$) }
  ;
%type <queriedTable> tableReferenceBody;
queryTableExpression:                                    /*{{{*/
    /* <имя2> [ SAMPLE [BLOCK] \( <число> ) [SEED \( NUMERIC_ID \)] ] */
    schemaDotColumn sampleClauseOpt                          { $$ = new Sm::FromTableReference($1, 0, $2, @$); }
    /* <имя2> [SUB]PARTITION (FOR (<число> )+)|(<имя>) [ SAMPLE [BLOCK] \( <число> ) [SEED \( NUMERIC_ID \)] ] */
  | schemaDotColumn partitionExtensionClause sampleClauseOpt { $$ = new Sm::FromTableReference($1, 0, $3, @$); }
    /* <имя2>@<dblink> [ SAMPLE [BLOCK] \( <число> ) [SEED \( NUMERIC_ID \)] ] */
  | schemaDotColumn '@' exprId sampleClauseOpt               { $$ = new Sm::FromTableReference($1, $3, $4, @$); }
  | THE '(' subquery ')'                                     { $$ = new Sm::FromTableSubquery(@$, $3, true , false); }
  | TABLE '(' sqlExpr ')'                                    { $$ = new Sm::FromTableDynamic( @$, $3, false); }
  | TABLE '(' sqlExpr ')' '(' '+' ')'                        { $$ = new Sm::FromTableDynamic( @$, $3, true ); }
  | dynTable                                                 { $$ = $1; }
  ;
%type <queriedTable> queryTableExpression; 


tableReferenceSubquery:                                 /*{{{*/
    tableReferenceSubqueryBody flashbackQueryClauseOpt asAliasOpt { $$ = new Sm::FromSingle(@$, $1, $2, $3); }
  ;
%type <fromSingle> tableReferenceSubquery;

tableReferenceSubqueryBody:                                 /*{{{*/
    ONLY '(' subquery ')' { $$ = new Sm::FromTableSubquery(@$, $3, false, false); $$->queryCathegory = Sm::QueriedTable::ONLY; $$->loc(@$); }
  | ONLY '(' subquery subqueryRestrictionClause ')' { $$ = new Sm::FromTableSubquery(@$, $3, false, false); $$->queryCathegory = Sm::QueriedTable::ONLY; $$->loc(@$); }
  | subqueryTableExpression              { $$ = $1; }
  | subqueryTableExpression PIVOT XML_opt '(' pivotAggregateList  FOR columnOrBrColumnList IN '(' pivotRange ')' ')' 
    { $$ = $1; $$->queryCathegory = Sm::QueriedTable::PIVOT; $$->loc(@$);  logSkipped(@$) }
  | subqueryTableExpression UNPIVOT includeOrExclude '(' columnOrBrColumnList FOR columnOrBrColumnList IN '(' columnAsConstList ')' ')' 
    { $$ = $1; $$->queryCathegory = Sm::QueriedTable::UNPIVOT; $$->loc(@$); logSkipped(@$) }
  ;
%type <queriedTable> tableReferenceSubqueryBody;
subqueryTableExpression:                                    /*{{{*/
    '(' subquery ')'                                         { $$ = new Sm::FromTableSubquery(@$, $2, false, false); }
    /* <подзапрос> WITH (READ ONLY)|(CHECK OPTION [CONSTANT constraint]) */
  | '(' subquery subqueryRestrictionClause ')'               { $$ = new Sm::FromTableSubquery(@$, $2, true , true ); }
  ;
%type <queriedTable> subqueryTableExpression; 

sampleClauseOpt:                                         /*{{{*/
                                                                { $$ = 0; }
  | SAMPLE           '(' NUMERIC_ID ')'                         { $$ = new Sm::Tablesample(@$, false, $3); logSkipped(@$) }
  | SAMPLE_BLOCK     '(' NUMERIC_ID ')'                         { $$ = new Sm::Tablesample(@$, true , $3); logSkipped(@$) }
  | SAMPLE           '(' NUMERIC_ID ')' SEED '(' NUMERIC_ID ')' { $$ = new Sm::Tablesample(@$, false, $3, $7); logSkipped(@$) }
  | SAMPLE_BLOCK     '(' NUMERIC_ID ')' SEED '(' NUMERIC_ID ')' { $$ = new Sm::Tablesample(@$, true , $3, $7); logSkipped(@$) }
  ;
%type <tablesample> sampleClauseOpt;
                                                         /*}}}*/
partitionExtensionClause:                                /*{{{ skip (partition_for is unsupported) */
    PARTITION        '(' RawID                    ')' { del($3); logSkipped(@$) }
  | PARTITION    FOR '(' partitionKeyValueList    ')' { logSkipped(@$) }
  | SUBPARTITION     '(' RawID                    ')' { del($3); logSkipped(@$) }
  | SUBPARTITION FOR '(' subpartitionKeyValueList ')' { logSkipped(@$) }
  ;
partitionExtensionClauseOpt:
    /* EMPTY */
  | partitionExtensionClause
  ;
partitionKeyValueList:
    NUMERIC_ID                       { del($1); logSkipped(@$) }
  | NUMERIC_ID partitionKeyValueList { del($1); logSkipped(@$) }
  ;

subpartitionKeyValueList:
    NUMERIC_ID                           { del($1); logSkipped(@$) }
  | NUMERIC_ID subpartitionKeyValueList  { del($1); logSkipped(@$) }
  ;
                                                         /*}}}*/
subqueryRestrictionClause:                               /*{{{ skip*/
    WITH READ ONLY      { logSkipped(@$) }
  | WITH CHECK OPTION   { logSkipped(@$) }
  | WITH CHECK OPTION CONSTANT constraint { del($5); logSkipped(@$) }
  ;
                                                         /*}}}*/
                                                         /*}}}*/
XML_opt:                                                 /*{{{*/
        { $$ = 0; }
  | XML { $$ = 1; logSkipped(@$) }
  ;
%type <intval> XML_opt;
                                                         /*}}}*/
pivotAggregateList:                                      /*{{{ - skip*/
                            pivotAggregateFunCall   
  | pivotAggregateList  ',' pivotAggregateFunCall   
  ;
pivotAggregateFunCall:   
    exprId '(' sqlExpr ')'         { del($1,$3); } 
  | exprId '(' sqlExpr ')' asAlias { del($1,$3,$5); } 
  ;                                                      /*}}}*/
columnOrBrColumnList:                                    /*{{{*/
    exprId               
  | '(' columnIdList ')' { del($2); }
  ;
                                                         /*}}}*/
pivotRange:                                              /*{{{*/
    pivotInExprList 
  | anyList         
  ;           
pivotInExprList: 
                         pivotInExprListItem 
  | pivotInExprList  ',' pivotInExprListItem 
  ;
pivotInExprListItem:                
    expressionCompound asAlias { del($2); }
  ;
expressionCompound:
        sqlExpr                     
  | '(' sqlExprList ',' sqlExpr ')' { del($2,$4); }
  ;
anyList:
    ANY             
  | anyList ',' ANY 
  ;
                                                         /*}}}*/
includeOrExclude:                                        /*{{{*/
    INCLUDE       
  | EXCLUDE       
  | INCLUDE NULLS 
  | EXCLUDE NULLS 
  ; 
                                                         /*}}}*/
columnAsConstList:                                       /*{{{*/
                           columnAsConstListItem 
  | columnAsConstList  ',' columnAsConstListItem 
  ;

columnAsConstListItem:                               
    columnOrBrColumnList                         
  | columnOrBrColumnList AS exprId               { del($3); logSkipped(@$) }
  | columnOrBrColumnList AS '(' constantList ')' { del($4); logSkipped(@$) }
  ; 
                                                         /*}}}*/
                                                    /*}}}*/
flashbackQueryClauseOpt:                            /*{{{*/
    /* EMPTY */                                                                                 { $$ = 0; }
  |           AS OF scnOrTimestamp sqlExpr /*asAlias*/                                          { $$ = new Sm::FlashbackQueryClause(@$, $3, $4); logSkipped(@$) }
  | VERSION_BETWEEN scnOrTimestamp sqlExpr AND sqlExpr AS OF scnOrTimestamp sqlExpr /*asAlias*/ { $$ = new Sm::FlashbackQueryClause(@$, $8, $9, $2, $3, $5); logSkipped(@$) }
  ;
%type <flashbackQueryClause> flashbackQueryClauseOpt;
scnOrTimestamp:                                     /*{{{*/
    SCN        { $$ = Sm::flashback_query::SCN; }
  | TIMESTAMP  { $$ = Sm::flashback_query::TIMESTAMP; }
  ;
%type <scnOrTimestamp> scnOrTimestamp;
                                                    /*}}}*/
                                                    /*}}}*/
partitionByExprListOpt:                             /*{{{*/
    /* EMPTY */                 { $$ = 0; }
  | PARTITION BY expressionList { $$ = $3; logSkipped(@$) }
  ;
expressionList:
        sqlExprList                 { $$ = $1; }
  | '(' sqlExprList ',' sqlExpr ')' { $$ = $2; $$->push_back($4); }
  ;
%type <sqlExprList> expressionList partitionByExprListOpt;
                                                    /*}}}*/
joinConditionOpt:                                   /*{{{*/
    /* EMPTY */                   { $$ = new Sm::JoinOnDefault(); }
  | ON     condition              { $$ = new Sm::JoinCondition(  @$, $2); }
  | USING '(' columnIdList ')'    { $$ = new Sm::JoinOnFieldList(@$, $3); }
  ;
%type <joinCondition> joinConditionOpt;
                                                    /*}}}*/
                                                /*}}}*/
                                            /*}}}*/
                                        /*}}}*/
asAlias:                                /*{{{*/
       viewConstraintColumnId { $$ = $1; $$->loc(@$); }
  | AS viewConstraintColumnId { $$ = $2; $$->loc(@$); }
  | AS_VALUE                  { $$ = new Sm::Identificator(STRING("VALUE"), @1); }
  ;
asAliasOpt:
    /* EMPTY */  { $$ = 0; }
  | asAlias      { $$ = $1; }
  ;
%type <id> asAlias asAliasOpt;
                                        /*}}}*/
                                    /*}}}*/
whereClauseOpt:                     /*{{{*/
                { $$ = 0;  }
  | whereClause { $$ = $1; }
  ;

dynWhereHead:
   _DYN_WHERE '(' condition ')' { $$ = $3; }
  ;
%type <plCond> dynWhereHead;

whereClause:  
    WHERE condition  { $$ = $2; $$->loc(@$); }
  | dynWhereHead { $$ = new Sm::DynWhere(@$, $1, 0); }
  | dynWhereHead AND whereCondition { $$ = new Sm::DynWhere(@$, $1, $3, Sm::DynWhere::DYN_AND); }
  | dynWhereHead OR  whereCondition { $$ = new Sm::DynWhere(@$, $1, $3, Sm::DynWhere::DYN_OR); }
  ;
%type <plCond> whereClauseOpt whereClause;
                                    /*}}}*/
hierarhicalQueryClause:          /*{{{*/
    CONNECT BY NOCYCLE_opt condition startWithConditionOpt { $$ = new Sm::HierarhicalClause(@$, $3, $4, $5); }
  | START WITH condition CONNECT BY NOCYCLE_opt condition  { $$ = new Sm::HierarhicalClause(@$, $6, $7, $3); }
  ;
%type <hierarhicalQueryClause> hierarhicalQueryClause;

NOCYCLE_opt:                            /*{{{*/
            { $$ = 0; }
  | NOCYCLE { $$ = 1; }
  ;
%type <intval> NOCYCLE_opt;
                                        /*}}}*/
startWithConditionOpt:                  /*{{{*/
                         { $$ = 0; }
  | START WITH condition { $$ = $3; $$->loc(@$); }
  ;
%type <plCond> startWithConditionOpt;
                                        /*}}}*/
                                    /*}}}*/
groupByClause:                      /*{{{*/
    GROUP BY groupByClauseList { $$ = $3; }
  ;
%type <groupByList> groupByClause;

groupByClauseList: 
                           groupByClauseItem     { $$ = mkList($1); }
  | groupByClauseList  ',' groupByClauseItem     { $$ = $1; $$->push_back($3); }
  ;
%type <groupByList> groupByClauseList;

groupByClauseItem:
    sqlExpr                                { $$ = new Sm::GroupBySqlExpr(@$, $1); }
  | rollupCubeClause                       { $$ = new Sm::GroupByRollupCubes(@$, mkList($1)); }
  | GROUPING_SETS '(' groupingSetsList ')' { $$ = new Sm::GroupByRollupCubes(@$, $3); logSkipped(@$) }
  ;
%type <groupBy> groupByClauseItem;

rollupCubeClause:
    ROLLUP '(' groupingExprList ')' { $$ = new Sm::grouping_sets::Rollup(@$, $3); logSkipped(@$) }
  | CUBE   '(' groupingExprList ')' { $$ = new Sm::grouping_sets::Cube(  @$, $3); logSkipped(@$) }
  ;

groupingSetsList: 
                          groupingSetsClauseItem { $$ = mkList($1); }
  | groupingSetsList  ',' groupingSetsClauseItem { $$ = $1; $$->push_back($3); }
  ;
%type <groupingSetsList> groupingSetsList ;

groupingSetsClauseItem:
    rollupCubeClause      { $$ = $1; }
  | groupingExprListItem  { $$ = new Sm::grouping_sets::Single(@$, $1); }
  ;
%type <groupingSets> rollupCubeClause groupingSetsClauseItem;

groupingExprList: 
                         groupingExprListItem    { $$ = mkList($1); }
  | groupingExprList ',' groupingExprListItem    { $$ = $1; $$->push_back($3); }
  ;
%type <sqlExprListList> groupingExprList;

groupingExprListItem:
    sqlExpr                         { $$ = mkList($1); }
  | '(' sqlExprList ',' sqlExpr ')' { $$ = $2; $$->push_back($4); };
%type <sqlExprList> groupingExprListItem;

                                    /*}}}*/

havingClause:                       /*{{{*/
    HAVING condition  { $$ = $2; $$->loc(@$); } 
  ;
%type <plCond> havingClause;

                                    /*}}}*/
selectList:                         /*{{{*/
    '*' 
    {
      $$ = new Sm::SelectList(@$, mkVector( new Sm::SelectedField(@$, new Sm::AsteriskExpr(@$))));
      $$->isAsterisk(true);

    }
  | DISTINCT  '*'             
    { 
      Sm::SqlExpr *p = new Sm::AsteriskExpr(@$);
      p->setDistinct();
      $$ = new Sm::SelectList(@$, mkVector(new Sm::SelectedField(@$, p)));
      $$->isAsterisk(true);
    }
  | selectedFields  { $$ = new Sm::SelectList(@$, $1); }
  ;
%type <selectList> selectList;

selectedFields: 
                        selectedField           { $$ = mkVector($1); }
  | selectedFields  ',' selectedField           { $$ = $1; $$->push_back($3); }
  ;
%type <selectedFields> selectedFields ;

selectedField:
    sqlExpr         { $$ = new Sm::SelectedField(@$, $1); }
  | sqlExpr asAlias { $$ = new Sm::SelectedField(@$, $1, $2); }
  ;
%type <selectedField> selectedField;
                                    /*}}}*/
modelClauseOpt:                     /*{{{ - skip*/                                { $$ = false; }
    /* EMPTY */
  | cellReferenceOptions returnRowClauseOpt referenceModelListOpt mainModel { $$ = true; logSkipped(@$) }
  ;
%type <boolval> modelClauseOpt;

cellReferenceOptions:                   /*{{{*/
    IgnoreOrKeepNavOpt UniqueDimOrRefOpt
  ;
IgnoreOrKeepNavOpt:
    MODEL_IGNORE_NAV
  | MODEL_KEEP_NAV
  | MODEL
  ;
UniqueDimOrRefOpt:
  | UNIQUE DIMENSION
  | UNIQUE SINGLE REFERENCE
  ;                                     /*}}}*/
returnRowClauseOpt:                     /*{{{*/
    /* EMPTY */
  | returnRowClause
  ;
returnRowClause:
    RETURN_UPDATED_ROWS
  | RETURN UPDATED ALL
  ;                                     /*}}}*/
referenceModelListOpt:                  /*{{{*/
    /* EMPTY */
  | referenceModelList 
  ;
referenceModelList:
                       referenceModel
  | referenceModelList referenceModel
  ;
referenceModel:
    REFERENCE RawID ON '(' subquery ')' modelColumnClauses cellReferenceOptions { del($2,$5); }
  ;                                     /*}}}*/
mainModel:                              /*{{{*/
    mainModelNameOpt modelColumnClauses cellReferenceOptions modelRulesClause
  ;
mainModelNameOpt:
  |  RawID RawID { del($2); }
  ;/*}}}*/
modelColumnClauses:                     /*{{{*/
    modelColumnPartitionByOpt
    DIMENSION BY '(' exprAliasList ')' MEASURES '(' exprAliasList ')'
  ;
modelColumnPartitionByOpt:                  /*{{{*/
  | PARTITION BY '(' exprAliasList ')'
  ;
exprAliasList:                                  /*{{{*/
                      exprAliasListItem
  | exprAliasList ',' exprAliasListItem
  ;
exprAliasListItem:
    sqlExpr
  | sqlExpr exprId    { del($1,$2); }
  | sqlExpr AS exprId { del($1,$3); }
  ;
                                                /*}}}*/
                                            /*}}}*/
                                        /*}}}*/
modelRulesClause:                       /*{{{*/
    modelRulesPrefixOpt '(' modelRulesClauseList ')'
  ;
modelRulesPrefixOpt:                        /*{{{*/
  | RULES UpdateOrUpsertOpt AutomaticSequentalOpt modelIterateClause
  ;
AutomaticSequentalOpt:
  | AUTOMATIC ORDER
  | SEQUENTAL ORDER
  ;
modelIterateClause:
  | ITERATE '(' INTVAL ')'
  | ITERATE '(' INTVAL ')' UNTIL '(' condition ')' { del($7); }
  ;                                         /*}}}*/
modelRulesClauseList:                       /*{{{*/
                             modelRulesClauseListItem
  | modelRulesClauseList ',' modelRulesClauseListItem
  ;
modelRulesClauseListItem:                       /*{{{*/
    UpdateOrUpsertOpt cellAssignment orderByClauseOpt '=' sqlExpr { del($3,$5); }
  ;
cellAssignment:
    exprId '[' cellAssignmentList ']'
  ;
cellAssignmentList :
                           cellAssignmentItem
  | cellAssignmentList ',' cellAssignmentItem
  ;
cellAssignmentItem:
    condition
  | singleColumnForLoop
  | multiColumnForLoop
  ;
singleColumnForLoop:                                /*{{{*/
    FOR exprId IN '(' subqueryOrLiteralList ')' { del($2); }
  | FOR exprId likePatternOpt FROM exprId TO exprId IncrementOrDecrement exprId { del($2,$5,$7,$9); }
  ;
likePatternOpt:
  | LIKE pattern
  ;
IncrementOrDecrement:
    INCREMENT
  | DECREMENT
  ;
subqueryOrLiteralList:
    subquery
  | literalList
  ;                                                 /*}}}*/
multiColumnForLoop:                                 /*{{{*/
    FOR '(' columnIdList ')' IN '(' multiColumnSubqueryOrLiteralList ')' { del($3); }
  ;
multiColumnSubqueryOrLiteralList:
    subquery
  | brLiteralList
  ; 
brLiteralList: 
    /* EMPTY */               
  | brLiteralList1 
  ;
brLiteralList1: 
                       '(' literalList ')'       
  | brLiteralList1 ',' '(' literalList ')'       
  ;
literalList: 
                     RawID { del($1); }  
  | literalList  ',' RawID { del($3); }   
  ;
                                                    /*}}}*/
                                                /*}}}*/
                                            /*}}}*/
                                        /*}}}*/
UpdateOrUpsertOpt:                      /*{{{*/
  | UPDATE
  | UPSERT
  | UPSERT ALL
  ;                                     /*}}}*/
                                    /*}}}*/
                                /*}}}*/
/*}}}*/
/* CONSTRAINT      {{{ */

/* http://docs.oracle.com/cd/B19306_01/server.102/b14200/clauses002.htm#CJAEDFIB */

constraint:                   /*{{{*/                                     /* conflict with EnableState */
    constraintNameOpt io_constraintAttribute constraintStateOpt { $$ = new Sm::Constraint(@$, $1, $2, $3); }
  | SCOPE IS schemaDotColumn                                    { $$ = 0; del($3); logSkipped(@$) } /* inline_ref_constraint1 */
  | WITH ROWID_tok                                              { $$ = 0; logSkipped(@$) } /* inline_ref_constraint1 */
  | SCOPE FOR '(' exprId ')' IS schemaDotColumn                 { $$ = 0; del($4,$7); logSkipped(@$) } /* out_of_line_ref_constraint1 */
  | REF       '(' exprId ')' WITH ROWID_tok                     { $$ = 0; del($3); logSkipped(@$) } /* out_of_line_ref_constraint2 */
  ;
%type <constraint> constraint;
                              /*}}}*/

i_constraint:                 /*{{{*/
    constraintNameOpt i_constraintAttribute constraintStateOpt { $$ = new Sm::Constraint(@$, $1, $2, $3); }
  ;
%type <constraint> i_constraint;

i_constraintNamed:                 
    CONSTRAINT exprId i_constraintAttribute constraintStateOpt { $$ = new Sm::Constraint(@$, $2, $3, $4); }
  ;
%type <constraint> i_constraintNamed;

i_constraints:
                  i_constraint { $$ = mkList($1);        $$->loc(@$); }
  | i_constraints i_constraint { $$ = $1; $$->push_back($2); $$->loc(@$); } 
  ;

i_constraintsOpt:             
    i_constraints { $$ = $1; }
  | /* EMPTY */   { $$ = 0; }
  ;
%type <constraintList> i_constraints i_constraintsOpt;

inlineAndIrefConstraintOpt:
    i_constraints            { $$ = $1; }
  | SCOPE IS schemaDotColumn { $$ = 0; del($3); } /* inline_ref_constraint1 */
  | WITH ROWID_tok           { $$ = 0; } /* inline_ref_constraint1 */
  | /* EMPTY */              { $$ = 0; }
  ;
%type <constraintList> inlineAndIrefConstraintOpt;

i_constraintNoName:                 
    i_constraintAttribute constraintStateOpt { $$ = new Sm::Constraint(@$, 0, $1, $2); }
  ;
%type <constraint> i_constraintNoName;

i_constraintsNoName:
                        i_constraintNoName { $$ = mkList($1);        $$->loc(@$); }
  | i_constraintsNoName i_constraintNoName { $$ = $1; $$->push_back($2); $$->loc(@$); } 
  ;
%type <constraintList> i_constraintsNoName;

inlineNoNameAndIrefConstraintOpt:
    i_constraintsNoName            { $$ = $1; }
  | SCOPE IS schemaDotColumn { $$ = 0; del($3); logSkipped(@$) } /* inline_ref_constraint1 */
  | WITH ROWID_tok           { $$ = 0; logSkipped(@$) } /* inline_ref_constraint1 */
  | /* EMPTY */              { $$ = 0; }
  ;
%type <constraintList> inlineNoNameAndIrefConstraintOpt;

outConstraint:
    o_constraint     { $$ = $1; }
  | o_refConstraint  { $$ = 0; logSkipped(@$) }
  ;
%type <constraint>     outConstraint;
                              /*}}}*/
o_constraints:                /*{{{*/
                      o_constraint { $$ = mkList($1);        $$->loc(@$); }
  | o_constraints ',' o_constraint { $$ = $1; $$->push_back($3); $$->loc(@$); } 
  ;
%type <constraintList> o_constraints;
                              /*}}}*/
o_constraint:                 /*{{{*/
    constraintNameOpt o_constraintAttribute constraintStateOpt { $$ = new Sm::Constraint(@$, $1, $2, $3);     } 
  ;
%type <constraint> o_constraint;
                              /*}}}*/
constraintNameOpt:            /*{{{*/
    CONSTRAINT selectedEntityId  { $$ = $2;  $$->loc(@$); logSkipped(@$) }
  | /* EMPTY */                 { $$ = 0; }
  ;
%type <id> constraintNameOpt; /*}}}*/
constraintStateOpt:           /*{{{*/
    constraintState { $$ = $1; logSkipped(@$) }
  | /* EMPTY */     { $$ = 0;  }
  ;
constraintState:
                    constraintStateItem { $$ = $1; if ($$) $$->loc(@$); }
  | constraintState constraintStateItem {
      $$ = $1;
      if ($$) {
        $$->concat($2);
        $$->loc(@$);
      }
      else
        $$ = $2;
    }
  ;
constraintStateItem:
    NOT DEFFERABLE                  { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_NOT_DEFFERABLE     ); }
  |     DEFFERABLE                  { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_DEFFERABLE         ); }
  | DISABLE                         { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_DISABLE            ); }
  | ENABLE                          { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_ENABLE             ); }
  | INITIALLY DEFERRED              { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_INITIALLY_DEFFERED ); }
  | INITIALLY IMMEDIATE             { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_INITIALLY_IMMEDIATE); }
  | NORELY                          { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_NORELY             ); }
  | NOVALIDATE                      { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_NOVALIDATE         ); }
  | RELY                            { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_RELY               ); }
  | VALIDATE                        { $$ = new Sm::constraint::ConstraintState(@$, FLAG_CONSTRAINT_STATE_VALIDATE           ); }
  | EXCEPTIONS_INTO schemaDotColumn { $$ = 0; del($2); }
  | constraintStateIndex            { $$ = $1; } 
  ;
%type <constraint_State> constraintState constraintStateItem constraintStateOpt;
                          /*}}}*/
i_constraintAttribute:        /*{{{*/
    NOT NULLk             { $$ = new Sm::constraint::NotNull    (@$); }      
  | NULLk                 { $$ = new Sm::constraint::Null       (@$); }      
  | referencedKey         { $$ = $1; }
  | UNIQUE                { $$ = new Sm::constraint::Unique     (@$); }
  | PRIMARY_KEY           { $$ = new Sm::constraint::PrimaryKey (@$); }     
  | constraint_Check      { $$ = $1; } 
  ;


o_constraintAttribute:
    constraint_PrimaryKey { $$ = $1; }
  | constraint_Unique     { $$ = $1; } 
  | constraint_Check      { $$ = $1; }
  | constraint_ForeignKey { $$ = $1; }
  ;
io_constraintAttribute:
    NULLk                 { $$ = new Sm::constraint::Null       (@$); }     
  | NOT NULLk             { $$ = new Sm::constraint::NotNull    (@$); }     
  | PRIMARY_KEY           { $$ = new Sm::constraint::PrimaryKey (@$); }     
  | UNIQUE                { $$ = new Sm::constraint::Unique     (@$); }     
  | referencedKey         { $$ = $1; }
  | ROW MOVEMENT          { $$ = new Sm::constraint::RowMovement(@$); logSkipped(@$) }
  | constraint_PrimaryKey { $$ = $1; }
  | constraint_Unique     { $$ = $1; }
  | constraint_Check      { $$ = $1; }
  | constraint_ForeignKey { $$ = $1; }
  ;
%type <constraint_Attribute> o_constraintAttribute io_constraintAttribute i_constraintAttribute;
   /*}}}*/
o_refConstraint:              /*{{{*/
    SCOPE FOR '(' exprId ')' IS schemaDotColumn     { del($4); del($7); logSkipped(@$) }
  | REF       '(' exprId ')' WITH ROWID_tok         { del($3); logSkipped(@$) }
  ;
                              /*}}}*/
referencedKey:                /*{{{*/
    REFERENCES schemaDotColumn brColumnListOpt onDeleteClauseOpt { $$ = new Sm::constraint::ForeignReference(@$, $2, $3, $4); }
  ;
%type <constraint_ReferencedKey> referencedKey;
onDeleteClauseOpt:                                               /*{{{*/
    ON DELETE CASCADE   { $$ = Sm::constraint::referenced_key::CASCADE;  }
  | ON DELETE SET NULLk { $$ = Sm::constraint::referenced_key::SET_NULL; }
  | /* EMPTY */         { $$ = Sm::constraint::referenced_key::EMPTY;    }
  ;
%type <constraintOnDelete> onDeleteClauseOpt;                        /*}}}*/

constraint_PrimaryKey:
    PRIMARY_KEY '(' columnIdList ')' { $$ = new Sm::constraint::PrimaryKey    (@$, $3); }
  ;
%type <constraint_PrimaryKey> constraint_PrimaryKey;

constraint_Unique:
    UNIQUE      '(' columnIdList ')' { $$ = new Sm::constraint::Unique        (@$, $3); }
  ;
%type <constraint_Unique> constraint_Unique;

constraint_Check:
    CHECK       '(' condition    ')' { $$ = new Sm::constraint::CheckCondition(@$, $3); }
  ;
%type <constraint_CheckCondition> constraint_Check;

constraint_ForeignKey:
    FOREIGN_KEY '(' columnIdList ')' referencedKey { $$ = new Sm::constraint::ForeignKey(@$, $3, $5); }
  ;
%type <constraint_ForeignKey> constraint_ForeignKey;
                              /*}}}*/
// TODO: USING INDEX возможно это можно поддерживать.  {{{
/*
 * Предложение_using_index
 * Предложение_using_index задает индекс для обеспечения уникального или
 * первичного ключа (или же дает указание создать такой индекс).
 *
 * Предложение_using_index можно указывать только при включении ограничения
 * уникального или первичного ключа. В предложении_using_index можно
 * перечислять фрагменты предложения в любом порядке, но нельзя указывать один
 * и тот же фрагмент более одного раза.
 * 1. Если указать параметр схема.индекс, то Oracle будет пытаться обеспечить
 *    ограничение целостности с помощью указанного индекса. Если Oracle не может
 *    найти индекс или не может использовать его для обеспечения ограничения
 *    целостности, будет возвращена ошибка.
 * 2. Если указать оператор_create_index, то Oracle попытается создать индекс и
 *    использовать его для обеспечения ограничения целостности. Если Oracle не
 *    может создать индекс или не может использовать его для обеспечения
 *    ограничения целостности, будет возвращена ошибка.
 * 3. Если явно не указано ни создание нового, ни использование существующего
 *    индекса, Oracle автоматически создаст индекс. В этом случае:
 * 4. индекс получает то же имя, что и ограничение;
 * 5. для индекса можно устанавливать значения параметров INITRANS, MAXTRANS,
 *    TABLESPACE, PCTFREE и STORAGE. Для индекса нельзя указывать параметры
 *    PCTUSED или предложение_logging;
 * 6. если таблица секционирована, то для уникального или первичного ключа
 *    можно создать локально или глобально секционированный индекс.
 */
constraintStateIndex:
    USING INDEX schemaDotColumn              { $$ = new Sm::constraint::ConstraintState(@$, $3); logSkipped(@$) }
  | USING INDEX '(' createIndexStatement ')' { $$ = new Sm::constraint::ConstraintState(@$, $4); logSkipped(@$) }
  | USING INDEX indexProperties              { $$ = 0; logSkipped(@$) } /* scip */
  ;

%type <constraint_State> constraintStateIndex;
/* }}} */

  /* end of CONSTRAINT }}} */
/* Грамматики идентификаторов и типов {{{1 */

rawIdList:
                  RawID                          { $$ = mkList($1); }
  | rawIdList ',' RawID                          { $$ = $1; $$->push_back($3); }
  ;
%type <idList> rawIdList;

typeSpec:
    varCommon  { $$ = $1; $$->loc(@$); }    
  | NEW        { $$ = new Sm::Identificator(STRING("NEW"), @1); }
  | XMLTYPE    { $$ = new Sm::Identificator(STRING("XMLTYPE"), @1); }
  | NATURAL    { $$ = new Sm::Identificator(STRING("NATURAL"), @1); }
  ;

varSpecAlterField_:
    CLOSE        { $$ = new Sm::Identificator(STRING("CLOSE"    ), @1); }
  | DATE         { $$ = new Sm::Identificator(STRING("DATE"     ), @1); }
  | DAY          { $$ = new Sm::Identificator(STRING("DAY"      ), @1); }
  | EMPTY        { $$ = new Sm::Identificator(STRING("EMPTY"    ), @1); }
  | ENABLE       { $$ = new Sm::Identificator(STRING("ENABLE"   ), @1); }
  | EXECUTE      { $$ = new Sm::Identificator(STRING("EXECUTE"  ), @1); }
  | FORCE        { $$ = new Sm::Identificator(STRING("FORCE"    ), @1); }
  | IGNORE       { $$ = new Sm::Identificator(STRING("IGNORE"   ), @1); }
  | LOB          { $$ = new Sm::Identificator(STRING("LOB"      ), @1); }
  | MONTH        { $$ = new Sm::Identificator(STRING("MONTH"    ), @1); }
  | NEW          { $$ = new Sm::Identificator(STRING("NEW"      ), @1); }
  | OID          { $$ = new Sm::Identificator(STRING("OID"      ), @1); }
  | OPEN         { $$ = new Sm::Identificator(STRING("OPEN"     ), @1); }
  | REQUIRED     { $$ = new Sm::Identificator(STRING("REQUIRED" ), @1); }
  | REVERSE      { $$ = new Sm::Identificator(STRING("REVERSE"  ), @1); }
  | SCHEMA       { $$ = new Sm::Identificator(STRING("SCHEMA"   ), @1); }
  | SECOND       { $$ = new Sm::Identificator(STRING("SECOND"   ), @1); }
  | SEED         { $$ = new Sm::Identificator(STRING("SEED"     ), @1); }
  | SELF         { $$ = new Sm::Identificator(STRING("SELF"     ), @1); }
  | TIMESTAMP    { $$ = new Sm::Identificator(STRING("TIMESTAMP"), @1); }
  | TYPE         { $$ = new Sm::Identificator(STRING("TYPE"     ), @1); }
  | XML          { $$ = new Sm::Identificator(STRING("XML"      ), @1); }
  | YEAR         { $$ = new Sm::Identificator(STRING("YEAR"     ), @1); }
  | SEQUENCE     { $$ = new Sm::Identificator(STRING("SEQUENCE" ), @1); }
  | DISABLE      { $$ = new Sm::Identificator(STRING("DISABLE" ), @1); }
  ;
%type <id> varSpecAlterField_;

varSpecAlterFieldConstructExprNamespaceId:
    varCommonConstructExprNamespaceId  { $$ = $1; }
  | varSpecAlterField_                 { $$ = $1; }
  ;
%type <id> varSpecAlterFieldConstructExprNamespaceId;


varSpecAlterField:
    varCommon    { $$ = $1; $$->loc(@$); }    
  | varSpecAlterField_  { $$ = $1; }
  ;
%type <id> varSpecAlterField;

varSpec_:
    CONSTRAINT        { $$ = new Sm::Identificator(STRING("CONSTRAINT"), @1); }
  | EXCEPTION         { $$ = new Sm::Identificator(STRING("EXCEPTION" ), @1); }
  | FOUND             { $$ = new Sm::Identificator(STRING("FOUND"     ), @1); }
  | CASCADE           { $$ = new Sm::Identificator(STRING("CASCADE"   ), @1); }
  | REFERENCES        { $$ = new Sm::Identificator(STRING("REFERENCES"), @1); }
  | ELEMENT           { $$ = new Sm::Identificator(STRING("ELEMENT"   ), @1); }
  | CROSS             { $$ = new Sm::Identificator(STRING("CROSS"     ), @1); }
  | AUTHID            { $$ = new Sm::Identificator(STRING("AUTHID"), @1); }
  | HIERARCHY         { $$ = new Sm::Identificator(STRING("HIERARCHY"), @1); }
  | CALL              { $$ = new Sm::Identificator(STRING("CALL"), @1); }
  | VALIDATE          { $$ = new Sm::Identificator(STRING("VALIDATE"), @1); }
  | LOGOFF            { $$ = new Sm::Identificator(STRING("LOGOFF"), @1); }
  | STORE             { $$ = new Sm::Identificator(STRING("STORE") , @1); }
  | TRUNCATE          { $$ = new Sm::Identificator(STRING("TRUNCATE") , @1); }
  | MODEL             { $$ = new Sm::Identificator(STRING("MODEL") , @1); }
  | SCOPE             { $$ = new Sm::Identificator(STRING("SCOPE") , @1); }
  | SUBTYPE           { $$ = new Sm::Identificator(STRING("SUBTYPE") , @1); }
  | ADD               { $$ = new Sm::Identificator(STRING("ADD") , @1); }
  | EXCEPT            { $$ = new Sm::Identificator(STRING("EXCEPT") , @1); }
  | READ              { $$ = new Sm::Identificator(STRING("READ") , @1); }
  | COMMENT           { $$ = new Sm::Identificator(STRING("COMMENT") , @1); }

  ;
%type <id> varSpec_;


varSpecConstructExprNamespaceId:
    varSpecAlterFieldConstructExprNamespaceId { $$ = $1; }
  | varSpec_                                  { $$ = $1; }
  ;


%type <id> varSpecConstructExprNamespaceId;

varSpec:
    varSpecAlterField { $$ = $1; }
  | varSpec_          { $$ = $1; }
  ;
%type <id> varSpec;


varCommonConstructExprNamespaceId:
    MINVALUE      { $$ = new Sm::Identificator(STRING("MINVALUE"), @1); }
  | MONITORING    { $$ = new Sm::Identificator(STRING("MONITORING"), @1); }
  | NOMONITORING  { $$ = new Sm::Identificator(STRING("NOMONITORING"), @1); }
  | MAXVALUE      { $$ = new Sm::Identificator(STRING("MAXVALUE"), @1); }
  | FINAL         { $$ = new Sm::Identificator(STRING("FINAL"   ), @1); }
  | LEVEL         { $$ = new Sm::Identificator(STRING("LEVEL"   ), @1); }
  | UID           { $$ = new Sm::Identificator(STRING("UID"     ), @1); }
  | MOD           { $$ = new Sm::Identificator(STRING("MOD"     ), @1); }
  ;
%type <id> varCommonConstructExprNamespaceId;


varCommon:
    varCommonConstructExprNamespaceId { $$ = $1; }
  | CONCAT        { $$ = new Sm::Identificator(STRING("CONCAT"  ), @1); }
  | GLOBAL_CURSOR { $$ = new Sm::Identificator(STRING("GLOBAL_CURSOR"), @1); }
  | SAVE_SUBQUERY { $$ = new Sm::Identificator(STRING("SAVE_SUBQUERY"), @1); }
  ;
%type <id> varCommon;




typeId_clear:
    RawID       { $$ = $1; }
  | CHARtok     { $$ = new Sm::Identificator(STRING("CHAR"    ), @1); }
  | DECIMAL     { $$ = new Sm::Identificator(STRING("DECIMAL" ), @1); }
  | FILEtok     { $$ = new Sm::Identificator(STRING("FILE"    ), @1); }
  | FLOAT       { $$ = new Sm::Identificator(STRING("FLOAT"   ), @1); }
  | INTEGERtok  { $$ = new Sm::Identificator(STRING("INTEGER" ), @1); }
  | LONGtok     { $$ = new Sm::Identificator(STRING("LONG"    ), @1); }
  | NUMBER      { $$ = new Sm::Identificator(STRING("NUMBER"  ), @1); }
  | RAW         { $$ = new Sm::Identificator(STRING("RAW"     ), @1); }
  | ROWID       { $$ = new Sm::Identificator(STRING("ROWID"   ), @1); }
  | SMALLINT    { $$ = new Sm::Identificator(STRING("SMALLINT"), @1); }
  | VARCHAR     { $$ = new Sm::Identificator(STRING("VARCHAR" ), @1); }
  | VARCHAR2    { $$ = new Sm::Identificator(STRING("VARCHAR2"), @1); }
  | XMLTYPE     { $$ = new Sm::Identificator(STRING("XMLTYPE" ), @1); }
  ;

typeId:
    typeId_clear { $$ = $1; }
  | DATE         { $$ = new Sm::Identificator(STRING("DATE")   , @1); }
  | NATURAL      { $$ = new Sm::Identificator(STRING("NATURAL"), @1); }
  ;
%type <id> typeId_clear typeId ;

typeSpecWithRefs:
    typeId_clear  { $$ = $1; }
  | varSpec       { $$ = $1; }
  ;
%type <id> typeSpecWithRefs;

TypeOrRowtype:
    pTYPE    { $$ = FLAG_DATATYPE_IS_TYPE_OF; }
  | pROWTYPE { $$ = FLAG_DATATYPE_IS_ROWTYPE_OF; }
  ;
%type <uintval> TypeOrRowtype;

datatypeUnameId:
    RawID      { $$ = $1; }
  | varSpec_   { $$ = $1; }

%type <id> datatypeUnameId;

datatypeUsername:
              datatypeUnameId '.' typeSpecWithRefs { $$ = new Sm::IdEntitySmart(); $$->reserve(2); $$->push_back($3); $$->push_back($1); }
  | RawID '.' RawID '.' typeSpecWithRefs { $$ = new Sm::IdEntitySmart(); $$->reserve(3); $$->push_back($5); $$->push_back($3); $$->push_back($1); }
  ;
%type <idEntitySmart> datatypeUsername;



datatypeReference:
               RawID TypeOrRowtype { $$ = new Sm::Datatype($1); $$->concatFlags($2); $$->loc(@$); }
  | datatypeUsername TypeOrRowtype { $$ = new Sm::Datatype($1); $$->concatFlags($2); $$->loc(@$); }
  ;
%type <datatype> datatypeReference;

datatypeReferenceRefCursorOpt:   /*{{{*/
    datatypeReferenceRefCursor        { $$ = $1; } 
  | RETURN datatypeReferenceRefCursor { $$ = $2; } 
  | /* EMPTY */              { $$ = 0;  }
  ;
%type <datatype> datatypeReferenceRefCursorOpt;
                        /*}}}*/
datatypeReferenceRefCursor:
               typeId              { $$ = new Sm::Datatype($1); $$->loc(@$); }
  |            RawID TypeOrRowtype { $$ = new Sm::Datatype($1); $$->concatFlags($2); $$->loc(@$); }
  | datatypeUsername TypeOrRowtype { $$ = new Sm::Datatype($1); $$->concatFlags($2); $$->loc(@$); }
  ;
%type <datatype> datatypeReferenceRefCursor;

brNumericIdOpt:
   /* EMPTY */        { $$ = 0; }
 | '(' NUMERIC_ID ')' { $$ = $2; }
 ;
 %type <numericValue> brNumericIdOpt;  

datatypeBase:
    typeId { $$ = new Sm::Datatype($1); $$->loc(@$); }
  | RawID /*INTERVAL*/ DAY TO SECOND
    { 
      $$ = Sm::Datatype::mkIntervalDayToSecond(); 
      del($1); 
    }

  | RawID /*INTERVAL*/ DAY '(' NUMERIC_ID ')' TO SECOND '(' NUMERIC_ID ')'
    { 
      $$ = Sm::Datatype::mkIntervalDayToSecond(); 
      $$->precision = $4->getUIntValue();
      $$->scale     = $9->getUIntValue();
      del($1,$4,$9); 
    }
  | RawID /*INTERVAL*/ YEAR TO MONTH  
    {
      $$ = Sm::Datatype::mkIntervalYearToMonth(); 
      del($1); 
    }
  | NUMBER    '(' '*' ','  NUMERIC_ID ')'       { $$ = new Sm::Datatype(new Sm::Identificator(STRING("NUMBER"), @1), new Sm::NumericSimpleInt(38), $5); $$->loc(@$); }
  | typeId    '(' NUMERIC_ID          ')'       { $$ = new Sm::Datatype($1, $3); $$->loc(@$); }
  | typeId    '(' NUMERIC_ID BYTE_tok ')'       { $$ = new Sm::Datatype($1, $3); $$->flags.v |= FLAG_DATATYPE_AS_BYTE_LENGTH; $$->loc(@$); }
  | typeId    '(' NUMERIC_ID CHARtok  ')'       { $$ = new Sm::Datatype($1, $3); $$->flags.v |= FLAG_DATATYPE_AS_CHAR_LENGTH; $$->loc(@$); }
  | typeId    '(' NUMERIC_ID ',' CHARtok  ')'   { $$ = new Sm::Datatype($1, $3); $$->flags.v |= FLAG_DATATYPE_AS_CHAR_LENGTH; $$->loc(@$); }
  | typeId    '(' NUMERIC_ID ',' NUMERIC_ID ')' { $$ = new Sm::Datatype($1, $3, $5); $$->loc(@$); }
  | typeId    '(' functionCallArglistOpt ')' TypeOrRowtype {
      $1->callArglist = $3;
      $$ = new Sm::Datatype($1); $$->loc(@$);
      $$->flags.v |= $5 | FLAG_DATATYPE_IS_SPEC_REFERENCE;
    }
  | TIMESTAMP brNumericIdOpt timestampTimezoneClause
    {
      switch ($3) {
        case Sm::time_expr::TIMESTAMP_EMPTY:
         $$ = Sm::Datatype::mkTimestamp();
         break;
        case Sm::time_expr::TIMESTAMP_WITH_TIME_ZONE:
         $$ = Sm::Datatype::mkTimestampTimezone();
         break;
        case Sm::time_expr::TIMESTAMP_WITH_LOCAL_TIME_ZONE:
         $$ = Sm::Datatype::mkTimestampLtzTimezone();
         break;
      };
      if ($2) {
        $$->precision = $2->getUIntValue();
        delete $2;
      }
      $$->loc(@$);
    }
  | datatypeReference                           { $$ = $1; } 
  ;
%type <datatype> datatypeBase;

timestampTimezoneClause:
    /* EMPTY */          { $$ = Sm::time_expr::TIMESTAMP_EMPTY; }
  | WITH_TIME_ZONE       { $$ = Sm::time_expr::TIMESTAMP_WITH_TIME_ZONE; }
  | WITH_LOCAL_TIME_ZONE { $$ = Sm::time_expr::TIMESTAMP_WITH_LOCAL_TIME_ZONE; }
  ;
%type <timestampCathegory> timestampTimezoneClause;

datatype:
    datatypeBase     { $$ = $1;                   $$->loc(@$); }
  | datatypeBase CHARtok SET RawID { $$ = $1;     $$->loc(@$); del($4); }
  | REF datatypeBase { $$ = $2; $$->setIsRef(); $$->loc(@$); }
  | datatypeUsername { $$ = new Sm::Datatype($1); $$->loc(@$); }
  ;
%type <datatype> datatype;

schemaDotColumnNoNew:
                    exprIdNoNew     { $$ = new Sm::Id2(@$, $1); }       
  | exprIdNoNew '.' exprId          { $$ = new Sm::Id2(@$, $3, $1); }   
  ;
%type <id2> schemaDotColumnNoNew;

typeName:
              userTypeId  { $$ = new Sm::Id2(@$, $1); }     
  | RawID '.' userTypeId  { $$ = new Sm::Id2(@$, $3, $1); } 
  ;

schemaDotColumn:
              exprId         { $$ = new Sm::Id2(@$, $1); }
  | RawID '.' schemaDotColId { $$ = new Sm::Id2(@$, $3, $1); }
  ;
%type <id2> schemaDotColumn typeName;

schemaDotColId:
    exprId  { $$ = $1; }
  | XMLTYPE { $$ = new Sm::Identificator(STRING("XMLTYPE"  ), @1); }
  ;
%type <id> schemaDotColId;


rawIdDotRawId:
              RawID      { $$ = new Sm::Id2(@$, $1); }
  | RawID '.' RawID      { $$ = new Sm::Id2(@$, $3, $1); }
  ;
%type <id2> rawIdDotRawId;

schemaDotEntityList:        
                            schemaDotColumn      { $$ = mkList($1); }
  | schemaDotEntityList ',' schemaDotColumn      { $$ = $1; $$->push_back($3); }
  ;
%type <id2List> schemaDotEntityList;



exprId_:
    COLUMN    { $$ = new Sm::Identificator(STRING("COLUMN"), @1); }
  | LIST      { $$ = new Sm::Identificator(STRING("LIST"), @1); }
  | NESTED    { $$ = new Sm::Identificator(STRING("NESTED"), @1); }
  | FILEtok   { $$ = new Sm::Identificator(STRING("FILE"), @1); }
  | SHOW      { $$ = new Sm::Identificator(STRING("SHOW"), @1); }
  ;
%type <id> exprId_;


constructExprId:
    RawID                            { $$ = $1; }
  | varSpecConstructExprNamespaceId  { $$ = $1; }
  | exprId_                          { $$ = $1; }
  ;
%type <id> constructExprId;

exprId:
    RawID    { $$ = $1; }
  | varSpec  { $$ = $1; }
  | exprId_  { $$ = $1; }
  ;
%type <id> exprId  typeSpec;


exprIdExists:
    exprId { $$ = $1; }
  | EXISTS { $$ = new Sm::Identificator(STRING("EXISTS"), @1);; }
  ;
%type <id> exprIdExists ;


constantList: 
                      exprId                     { $$ = mkList($1); }
  | constantList  ',' exprId                     { $$ = $1; $$->push_back($3); }
  ;
%type <idList> constantList ;

columnId:
    exprId       { $$ = $1; }
  | SORT         { $$ = new Sm::Identificator(STRING("SORT"), @1); }
  | REF          { $$ = new Sm::Identificator(STRING("REF"), @1); }
  ;
%type <id> columnId;

columnIdList: 
                      columnId                     { $$ = mkList($1); }
  | columnIdList  ',' columnId                     { $$ = $1; $$->push_back($3); }
  ;
%type <idList> columnIdList;


userTypeId:
    RawID     { $$ = $1; }
  | typeSpec  { $$ = $1; }
  ;
%type <id> userTypeId;

exprIdNoNew:
    RawID     { $$ = $1; }
  | varCommon { $$ = $1; }
  ;
%type <id> exprIdNoNew;

signedNumericId:
        NUMERIC_ID              { $$ = $1;   } 
  | '-' NUMERIC_ID %prec UMINUS { $$ = $2; $$->neg(); } 
  ;
%type <numericValue> signedNumericId;

  /* 1}}} */
/* GRANT           {{{1 <- scip */

grant:
    GRANT grantSystemPrivileges endSqlGrammarOpt {  logSkipped(@$) }
  | GRANT grantObjectPrivileges endSqlGrammarOpt {  logSkipped(@$) }
  ;

grantSystemPrivileges: /*{{{*/
    grantPrivilegesList TO granteeList withAdminOptionOpt {  logSkipped(@$) }
  ;
withAdminOptionOpt:
    WITH ADMIN OPTION
  | WITH GRANT OPTION
  | /* EMPTY */
  ;

grantPrivilegesList:
                            grantPrivilegeClause
  | grantPrivilegesList ',' grantPrivilegeClause
  ;
grantPrivilegeClause:          /*{{{*/
    grantPrivilege
  | ALL PRIVILEGES
  | ALL
  | grantPrivilege  '(' schemaList ')'
  | ALL PRIVILEGES  '(' schemaList ')'
  | ALL             '(' schemaList ')'
  ;
schemaList:
    RawID                 { del($1); }
  | RawID ',' schemaList  { del($1); }
  ;
grantPrivilege:
                   grantPrivilegeId
  | grantPrivilege grantPrivilegeId
  ;
grantPrivilegeId:                      /*{{{*/
    RawID  { del($1); }
  /*| ACCESS */
  | ALTER
  | ANALYZE
  | ANY
  | AUDIT
  | CLUSTER
  | COMMENT
  | COMMIT
  | CONNECT
  | CREATE
  | DBA
  | DELETE
  | DIMENSION
  | DIRECTORY
  | DROP
  | EXECUTE
  | EXTERNAL
  | FORCE
  | GRANT
  | INDEX
  | INSERT
  | LIBRARY
  | LOCK
  | OBJECT
  | PROCEDURE
  | PROFILE
  | READ
  | REFERENCES
  | ROLLBACK
  | SELECT
  | SEQUENCE
  | SET
  | SQL
  | TABLE
  | TABLESPACE
  | TRANSACTION
  | UNDER
  | UPDATE
  | ON_COMMIT
  | MERGE
  ;
                                       /*}}}*/
                               /*}}}*/

                       /*}}}*/
grantObjectPrivileges: /*{{{*/
    grantPrivilegesList onObjectClause TO granteeClause grantObjectPrivilegesWithOpt {  logSkipped(@$) }
  ;

grantObjectPrivilegesWithOpt:
    /* EMPTY */
  | WITH grantObjectPrivilegesWithItem {  logSkipped(@$) }
  | WITH grantObjectPrivilegesWithItem WITH grantObjectPrivilegesWithItem {  logSkipped(@$) }
  ;

grantObjectPrivilegesWithItem:
    HIERARCHY OPTION {  logSkipped(@$) }
  | GRANT OPTION {  logSkipped(@$) }
  ;

onObjectClause:
    ON schemaDotColumn                                     { del($2); logSkipped(@$) }
  | ON DIRECTORY RawID   /* directory name */  /* scip */  { del($3); logSkipped(@$) }
  | ON JAVA_SOURCE   schemaDotColumn /* scip */            { del($3); logSkipped(@$) }
  | ON JAVA_RESOURCE schemaDotColumn /* scip */            { del($3); logSkipped(@$) }
  ;


                       /*}}}*/
granteeList:           /*{{{*/
    granteeClause
  | granteeClause ',' granteeList
  ;

granteeClause:
    RawID IDENTIFIED_BY RawID  { del($1, $3); logSkipped(@$) }
  | RawID                      { del($1); logSkipped(@$) }
  ;
                       /*}}}*/

/* End of GRANT 1}}} */


// http://docs.oracle.com/cd/B28359_01/server.111/b28286/conditions001.htm#i1034172

%%


/* Oracle Database Reserved Words {{{

Эти токены не могут встретиться в выражениях как идентификаторы.
Остальные - могут.
TODO: исходя из этого - допилить RawId и exprId

ACCESS
ADD *
ALL *
ALTER *
AND *
ANY *
AS *
ASC *
AUDIT
BETWEEN *
BY *
CHAR *
CHECK *
CLUSTER
COLUMN
COMMENT
COMPRESS
CONNECT *
CREATE *
CURRENT *
DATE *
DECIMAL *
DEFAULT *
DELETE *
DESC *
DISTINCT *
DROP *
ELSE *
EXCLUSIVE
EXISTS
FILE
FLOAT *
FOR *
FROM *
GRANT *
GROUP *
HAVING *
IMMEDIATE *
IN *
INCREMENT
INDEX
INITIAL
INSERT *
INTEGER *
INTERSECT *
INTO *
IS *
LEVEL *
LIKE *
LOCK
LONG
MAXEXTENTS
MINUS
MLSLABEL
MODE
MODIFY
NOAUDIT
NOCOMPRESS
NOT *
NOWAIT
NULL *
NUMBER
OF *
ON *
ONLINE
OPTION *
OR *
ORDER *
PCTFREE
PRIOR *
PRIVILEGES *
PUBLIC *
RAW
RENAME
RESOURCE
REVOKE *
ROW
ROWID
ROWNUM
ROWS *
SELECT *
SESSION *
SET *
SHARE
SIZE *
SMALLINT *
START
SUCCESSFUL
SYNONYM
SYSDATE
TABLE *
THEN *
TO *
TRIGGER
UID
UNION *
UNIQUE *
UPDATE *
USER *
VALIDATE
VALUES *
VARCHAR *
VARCHAR2
VIEW *
WHENEVER *
WHERE
WITH *




}}} */
/**
 * TODO list:
 * 1) database lisks is unsupported
 *    <database lisk> ::= @ <dblink>
 *    <dblink>        ::=   RawID
 *                        | RawID '.' RawID
 *                        ;
 * 2) DROP GRAMMARS - unsupported
 * 3) ALTER GRAMMARS - is part supported;
 */


COMPILER_BISON_OPTIMIZATION_POP()
// vim:foldmethod=marker
