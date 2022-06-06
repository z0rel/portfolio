#ifndef SEMANTIC_STATEMENTS
#define SEMANTIC_STATEMENTS

#include <stack>
#include "semantic_base.h"
#include "resolved_entity.h"

namespace Sm {



class SqlStatementInterface : public StatementInterface {
public:
  enum CathegorySqlStatementInterface {
    COMMIT           ,
    DELETE_FROM      ,
    INSERT           ,
    LOCK_TABLE       ,
    ROLLBACK         ,
    SAVEPOINT        ,
    SELECT_STATEMENT ,
    TRANSACTION      ,
    UPDATE           ,
    MERGE
  };
  SqlStatementInterface();

  virtual CathegorySqlStatementInterface cathegorySqlStatementInterface() const = 0;

  Sm::SqlStatementInterface *toSelfSqlStatementInterface() { return this; }
  Ptr<Sm::Datatype> getDatatype() const { return 0; }
  void translate(Codestream &str);
  SemanticTree *getSemanticNode() const;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~SqlStatementInterface() {}
  StatementInterface *toSelfStatementInterface() const { return const_cast<SqlStatementInterface*>(this); }
};


class Assignment            : public Statement, public virtual ResolvedEntitySNode {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }

public:
  Ptr<LValue> lValue;
  Ptr<PlExpr> assignedExpr;

  Assignment(CLoc l, LValue *_lValue, PlExpr *_assignedExpr);
  Assignment(CLoc l, Ptr<Id> _lvalue, Ptr<Id> _rvalue);

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;

  Assignment *toSelfAssignment() const { return (Assignment*)this; }
  void linterDefinition(Codestream &str);
  Ptr<Sm::LValue> lvalue();
  void replaceChildsIf(Sm::ExprTr tr);

  void unwrapStructuredLValue(Sm::VariableField *var, list<Ptr<StatementInterface> > &stmts, list<Ptr<StatementInterface> >::iterator &it);

  void translateCallSignature(Codestream &str, RefExpr *simpleFunctionCall);
  void translateCollectionCtor(Codestream &str, RefExpr *simpleFunctionCall);


  ScopedEntities ddlCathegory() const { return Assignment_; }
protected:
  SemanticTree *removeThisSnode();
};


class LinterCursorField : public ResolvedEntity {
protected:
  mutable Ptr<Datatype> thisDatatype_;
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Id>               name;
  ResolvedEntity  *resolvedField;
  ResolvedEntity  *ownerVariable;
  bool transitive;
public:
  LinterCursorField(Ptr<Id> _name, ResolvedEntity *_resolvedField, ResolvedEntity *_ownerVariable);

  ResolvedEntity *owner() const { return ResolvedEntity::owner(); }
  UserContext* userContext() const { return ResolvedEntity::userContext(); }
  ResolvedEntity *ownerPackage() const { return ResolvedEntity::ownerPackage(); }

  ScopedEntities ddlCathegory() const { return LinterCursorField_; }
  SemanticTree *toSTreeBase() const { return 0; }
  Ptr<Id> getName() const { return name; }
  Ptr<Datatype> getDatatype() const;
  void linterReference  (Sm::Codestream &str);
  void setTransitive(bool v) { transitive = v; }
};

/*
class VariableUndeclaredIndex : public ResolvedEntity, public TranslatedName {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  BlockPlSql *ownerBlock = 0;
  Ptr<LinterCursor> declVar;
  typedef std::map<std::string, Ptr<LinterCursorField> > CurFields;
  CurFields fields;
public:
  Ptr<Id>      indexVariable;
  Ptr<SqlExpr> sqlExpression;

  VariableUndeclaredIndex(Ptr<Id> indexVar, Ptr<SqlExpr> expr)
    : indexVariable(indexVar), sqlExpression(expr) { indexVariable->definition(this); }

  ResolvedEntity *owner() const { return ResolvedEntity::owner(); }
  UserContext* userContext() const { return ResolvedEntity::userContext(); }

  CLoc getLLoc() const { return indexVariable->getLLoc(); }

  ResolvedEntity *ownerVariable() const { return (ResolvedEntity*)this; }
  ResolvedEntity *ownerPackage() const { return ResolvedEntity::ownerPackage(); }

  std::string translatedName() const { return translateNameForLocalContext(); }
  void translatedName(const std::string &v) { TranslatedName::translatedName(v); }
  void translatedNameForce(const std::string &v) { translatedName_ = v; }


  bool setOwnerBlockPlSql(BlockPlSql *b) { ownerBlock = b; return true; }

  void linterDefinition(Sm::Codestream &str) {
    if (!declVar)
      createDeclVar();
    declVar->linterDefinition(str);
  }
  void linterReference(Sm::Codestream &str) {
    if (!declVar)
      createDeclVar();
    declVar->linterReference(str);
  }

  bool isDefinition()   const { return true; }
  Ptr<Sm::Id> getName() const { return indexVariable; }
  bool getFields(EntityFields &_fields) const { return sqlExpression->getFields(_fields); }
  bool getFieldRef(Ptr<Sm::Id> &field) {
    if (!field)
      return false;
    CurFields::iterator it = fields.find(field->toNormalizedString());
    if (it != fields.end()) {
      field->definition(it->second.object());
      return true;
    }
    if (sqlExpression->getFieldRef(field)) {
      if (ResolvedEntity *d = field->unresolvedDefinition())
        fields[field->toNormalizedString()] = new LinterCursorField(field, d, (ResolvedEntity*)this);
      return true;
    }
    return false;
  }

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) {
    if (eqByVEntities(supertype))
      return 2;
    return sqlExpression->isSubtype(supertype, plContext);
  }

  ResolvedEntity*   getNextDefinition()  const { return sqlExpression->getNextDefinition(); }
  ScopedEntities    ddlCathegory()       const { return VariableUndeclaredIndex_; }
  Ptr<Sm::Datatype> getDatatype()        const { return sqlExpression->getDatatype(); }

  SemanticTree *toVariableDeclarationNode() const {
    return setSemanticNode(indexVariable->toSNodeDecl(SCathegory::VariableUndeclaredIndex, this));
  }

  SemanticTree *toSTreeBase() const {
    SemanticTree *n = toVariableDeclarationNode();
    CTREE(sqlExpression)
        return n;
  }
  void createDeclVar()
  {
    if (!ownerBlock)
      throw 999; // translate as cursor see linterCursor;
    std::string uniqueName = indexVariable->toCodeId(ownerBlock->getSemanticNode()->levelNamespace);
    // Сгенерировать тип данных.
    declVar = new Sm::LinterCursor(std::forward<string>(uniqueName), sqlExpression);
  }
};
*/


//void mergeInternalBlockDecl(BlockPlSql *topBlock);

class CaseStatement         : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> caseOperand;
  Statements  whenList; // WhenExpr
//  BaseList<StatementInterface> elseStmts;
  Ptr<Id>     endLabel;
  bool        outStatements_ = true;

  CaseStatement(CLoc l, Ptr<PlExpr> _caseOperand, Ptr<BaseList<StatementInterface> > whenLst, Ptr<WhenExpr> _elseStmts = 0, Ptr<Id> endLbl = 0);

  bool needSemicolon() { return false; }
  ScopedEntities ddlCathegory() const { return CaseStatement_; }
  Ptr<Datatype>  getDatatype() const  { return Ptr<Datatype>(); }

  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const;
  void collectSNode(SemanticTree *n) const;

  void linterDefinition(Sm::Codestream &str);

  void traverseDeclarationsStmt(DeclActor &fun);
  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun, whenList); }

  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  void setNotOutStatements() { outStatements_ = false; }
  CaseStatement *toSelfCaseStmt() const { return const_cast<CaseStatement*>(this); }
  bool whenListConditionComplex() const;
  bool caseOperandIsBooleanLiteral() const;
  bool inExceptionSection() const;

  Statements *getChildStatements() { return &whenList; }

  If* transformIfSpecial();

  ~CaseStatement();
};


class ExecuteImmediate      : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef List<IdEntitySmart> EntityList;
  Ptr<PlExpr>                 execExpr;
  Ptr<IntoCollections>        intoVars;
  Ptr<List<ArgumentNameRef> > usingArglist;
  Ptr<IntoCollections>        retlist;

  ExecuteImmediate(CLoc l, Ptr<PlExpr> _execExpr);
  ExecuteImmediate(CLoc l, Ptr<List<ArgumentNameRef> > _usingArglist = 0, Ptr<IntoCollections> _retlist = 0, Ptr<IntoCollections> _intoVars = 0);

  void collectSNode(SemanticTree *n) const;
  void linterDefinition (Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  ExecuteImmediate *toSelfExecuteImmediate() const { return const_cast<ExecuteImmediate*>(this); }

  ScopedEntities ddlCathegory() const { return ExecuteImmediate_; }
};


class Exit                  : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id>     label; // loop label reference
  Ptr<PlExpr> when;
  bool            outStatements_ = true;

  Ptr<PlExpr> exitWhen() const { return when; }

  Exit(CLoc l, Ptr<PlExpr> _when = 0, Ptr<Id> _label = 0);
  void linterDefinition(Sm::Codestream &str);

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, label, when); }
  void setNotOutStatements() { outStatements_ = false; }

  Exit* toSelfExit() const { return const_cast<Exit*>(this); }
  ScopedEntities ddlCathegory() const { return Exit_; }
};


class Fetch                 : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  /**
   * bulk_collect_into_clause используется для определения одной или нескольких
   * коллекций, в которые нужно сохранять строки, возвращаемые оператором FETCH
   */

  typedef Subquery::IntoList::value_type Fields;
  enum Into { EMPTY, COLUMNS, COLLECTION };
  Ptr<RefExpr> cursorRef; // cursor, cursor variable, host cursor
  Fields       fields;
  Ptr<PlExpr>  limit;
  Into         intoCathegory;

  Fetch(CLoc l, Ptr<Id> _cursorRef, Ptr<List<IdEntitySmart> > _fields, Into cat = COLUMNS, Ptr<PlExpr> _limit = 0);

  void linterDefinition(Sm::Codestream &str);
  bool isFetchStatement() const { return true; }

  void collectSNode(SemanticTree *n) const;

  void changeCursorVariable(Ptr<LinterCursor> cur);
  bool getFields(EntityFields &fields) const;

  void replaceChildsIf(Sm::ExprTr tr);
  Fetch* toSelfFetch() const { return const_cast<Fetch*>(this); }
  void trFetchInto(bool isInsertCollection, Sm::Codestream &str);

  ScopedEntities ddlCathegory() const { return Fetch_; }
};


class NullStatement         : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  NullStatement();
  NullStatement(CLoc l);
  void collectSNode(SemanticTree *) const;
  void linterDefinition (Sm::Codestream &str);

  NullStatement* toSelfNullStmt() const { return const_cast<NullStatement*>(this); }
  ScopedEntities ddlCathegory() const { return NullStatement_; }
};


class LoopBounds            : public GrammarBaseSmart {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<SqlExpr> lowerBound;
  Ptr<SqlExpr> upperBound;

  LoopBounds(CLoc l, SqlExpr* _lowerBound, SqlExpr* _upperBound);
  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);
};


class ForAll                : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Sm::Variable>          idxVar;
public:
  Ptr<Id>                    indexVariable; // Declaration - индекс коллекции
  Ptr<LoopBounds>            bounds;
  Ptr<SqlStatementInterface> sqlStatement;
  Statements                 singleStatementContainer; // contains single stmt - sqlStatement
  bool                       outStatements_ = true;


  ScopedEntities ddlCathegory() const { return ForAll_; }
  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const;

  ForAll(CLoc l, Id *_idx, LoopBounds *_bounds, SqlStatementInterface *sqlStmt);

  void linterDefinition (Sm::Codestream &str);
  void collectSNode(SemanticTree *node) const;
  bool needSemicolon() { return false; }

  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun , *sqlStatement); }

  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  void setNotOutStatements() { outStatements_ = false; }

  Statements *getChildStatements() { return &singleStatementContainer; }
};


class ForOfExpression       : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Variable> indexVariableDefinition;
  Ptr<Id>       indexVariable;
  Ptr<SqlExpr>  sqlExpression;
  Ptr<Loop>     loop;
  bool          isReverse;
  bool          outStatements_ = true;

  ForOfExpression(CLoc l, Ptr<SqlExpr> sqlExpr, Ptr<Loop> _loop);

  void setIndexVariable(CLoc &l, Ptr<Id> indexVar, bool reverse);

  bool setOwnerBlockPlSql(Sm::BlockPlSql *b);

  void linterDefinition(Codestream &str);
  void linterReference (Sm::Codestream &) {}

  void collectSNode(SemanticTree *n_src) const;

  virtual ~ForOfExpression() {}
  bool needSemicolon() { return false; }

  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun, *loop); }
  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);
  void setNotOutStatements() { outStatements_ = false; }

  void traverseDeclarationsStmt(DeclActor &fun);
  Statements *getChildStatements();

  ForOfExpression *toSelfForOfExpression() const { return const_cast<ForOfExpression*>(this); }
  ScopedEntities ddlCathegory() const { return ForOfExpression_; }
};


class ForOfRange            : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Sm::Variable> idxVar;
public:
  Ptr<Id>         indexVariable;
  Ptr<LoopBounds> bounds;
  Ptr<Loop>       loop;
  bool            isReverse;
  bool            outStatements_ = true;

  ForOfRange(CLoc &l, LoopBounds *_bounds, Loop *_loop);

  void setIndexVariable(CLoc &l, Ptr<Id> indexVar, bool reverse);

  void collectSNode(SemanticTree *node) const;
  bool needSemicolon() { return false; }
  void linterDefinition(Sm::Codestream &str);

  void traverseDeclarationsStmt(DeclActor &fun);
  void traverseStatements(StatementActor &fun) { traverseStmts(this, fun, *loop); }

  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  void setNotOutStatements() { outStatements_ = false; }
  Statements *getChildStatements();

  ForOfRange* toSelfForOfRange() const { return const_cast<ForOfRange*>(this); }
  ScopedEntities ddlCathegory() const { return ForOfRange_; }
};


class If                    : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Statements    branches; // WhenExpr

  bool          outStatements_ = true;

  If(CLoc l, Ptr<WhenExpr> _first, Ptr<BaseList<StatementInterface> > _elseIfStmts, Ptr<WhenExpr> _elseStmts);
  If(FLoc l, BaseList<StatementInterface>  &_statements);

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  void translateBranch(std::string condCmd, Ptr<Sm::WhenExpr> branch, Codestream &str);
  bool needSemicolon() { return false; }

  void traverseDeclarationsStmt(DeclActor &fun);
  void traverseStatements(StatementActor &fun);

  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  void setNotOutStatements() { outStatements_ = false; }

  Statements *getChildStatements() { return &branches; }

  Sm::If* toSelfIf() const { return (If*)this; }
  ScopedEntities ddlCathegory() const { return If_; }
};


class OpenCursor            : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<RefExpr> cursor;
  std::vector<Ptr<Sm::FunCallArg> > actualCursorParameters;

  OpenCursor(CLoc l, Ptr<Id> _cursor, Ptr<CallArgList> _actualCursorParameters = 0);

  void linterDefinition(Sm::Codestream &str);
  void collectSNode(SemanticTree *n) const;

  void replaceChildsIf(Sm::ExprTr tr);

  OpenCursor* toSelfOpenCursor() const { return const_cast<OpenCursor*>(this); }
  ScopedEntities ddlCathegory() const { return OpenCursor_; }
};


class OpenFor               : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<RefExpr>         baseCursorVariable;
  Ptr<Id>              openForIterVariable; // может изменяться для RefCursor-ов
  /** Категория Select:
   * 1) Явный Select.
   * 2) Строковый литерал, строковая переменная или строковое выражение типа
   * CHAR, VARCHAR2, или CLOB, которое представляет из себя SQL оператор
   * SELECT.  Обычно, dynamic_statement представляет SQL оператор SELECT,
   * который возвращает множество строк.
   */
  Ptr<SqlExpr>                select;
  Ptr<List<ArgumentNameRef> > bindArguments;

  /// Макроподстановка для кастинга полей курсора, инициализируемого из динамически генерируемых запросов,
  /// разбросанных по разным кускам кода
  Ptr<Sm::Id>           globalCursorDecl;

  list<CursorDecltype*> decltypeSelectStmts;

  OpenFor(CLoc l, Ptr<Sm::Id> curVar, SqlExpr *sel, List<ArgumentNameRef> *bindArgs);

  void collectSNode(SemanticTree *n) const;

  bool isOpenStatement() const { return true; }
  Ptr<SqlExpr> selectExpr() { return select; }
  void changeCursorVariable(Ptr<LinterCursor> cur);
  void linterDefinition (Sm::Codestream &str);
  bool getStatementsThatContainEntity(ResolvedEntity *entity, FoundedStatements &outList);
  ResolvedEntity *openedEntity();
  bool getFields(EntityFields &fields) const;

  bool translatedAsExecuteWithoutDirect() const;

  void replaceChildsIf(Sm::ExprTr tr);

  OpenFor* toSelfOpenFor() const { return const_cast<OpenFor*>(this); }
  ScopedEntities ddlCathegory() const { return OpenFor_; }
};


class While                 : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> condition; // if 0 then loop;
  Ptr<Loop>   loop;
  bool        outStatements_ = true;

  While(CLoc l, Ptr<PlExpr> _condition, Ptr<Loop> _loop);
  void collectSNode(SemanticTree *n) const;
  bool needSemicolon() { return false; }

  void linterDefinition(Sm::Codestream &str);

  void traverseDeclarationsStmt(DeclActor &fun);
  void traverseStatements(StatementActor &fun);

  void buildStatementsTree(StatementsTree &parent);

  void replaceSubstatementsIf(StmtTr tr, StmtTrCond cond);
  void replaceChildsIf(Sm::ExprTr tr);

  void setNotOutStatements() { outStatements_ = false; }
  Statements *getChildStatements();

  While* toSelfWhile() const { return const_cast<While*>(this); }
  ScopedEntities ddlCathegory() const { return While_; }
};


class FunctionCall          : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  bool translateToRefCursorAssignment = false;
public:
  Ptr<SqlExpr> funCall;

  FunctionCall(CLoc l, SqlExpr *call);

  void collectSNode(SemanticTree *n) const;
  ResolvedEntity *getCursorFuncall() { return (FunctionCall*)this; }
  void setTranslateToRefCursorAssignment() { translateToRefCursorAssignment = true; }
  bool isFunCallStatement() const { return true; }

  void changeCursorVariable(Ptr<LinterCursor> /*cur*/) {}
  void linterDefinition(Sm::Codestream &str);

  void replaceChildsIf(Sm::ExprTr tr);

  FunctionCall *toSelfFunctionCall() const { return const_cast<FunctionCall*>(this); }
  ScopedEntities ddlCathegory() const { return FunctionCall_; }

//  ResolvedEntity* refDefinition() const; // may be null
  Ptr<IdRef>      reference() const;
};


class Close                 : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> cursorEntity;
  void linterDefinition (Sm::Codestream &str);
  Close(CLoc &l, Id *curEnt);
  void collectSNode(SemanticTree *n) const;
  bool isCloseStatement() const { return true; }
  void changeCursorVariable(Ptr<LinterCursor> cur);
  void  replaceChildsIf(Sm::ExprTr tr);

  ScopedEntities ddlCathegory() const { return Close_; }
  Close* toSelfClose() const { return const_cast<Close*>(this); }
};


class Goto                  : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> gotoLabel;
  Goto(CLoc &l, Id *_gotoLabel);
  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  Goto* toSelfGoto() const { return const_cast<Goto*>(this); }
  ScopedEntities ddlCathegory() const { return Goto_; }
};


class PipeRow               : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<IdEntitySmart> pipedRow;
  PipeRow(CLoc l, Ptr<IdEntitySmart> _pipedRow);
  void collectSNode(SemanticTree *) const;
  void oracleDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);

  void replaceChildsIf(Sm::ExprTr tr);

  ScopedEntities ddlCathegory() const { return PipeRow_; }
};


class Label                 : public Statement {
  Ptr<Id> name;
public:
  SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Label(Ptr<Id> n);
  Ptr<Id> getName() const { return name; }
  Ptr<Datatype> getDatatype() const { return 0; }

  void collectSNode(SemanticTree *n) const;
  ScopedEntities ddlCathegory() const { return Label_; }
  void linterDefinition(Sm::Codestream &str);
  bool needSemicolon() { return false; }

  void replaceChildsIf(Sm::ExprTr tr);
  Label* toSelfLabel() const { return const_cast<Label*>(this); }
};


class Raise                 : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<IdEntitySmart> exception;

  Raise(CLoc l, Ptr<IdEntitySmart> _exception);
  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  Raise* toSelfRaise() const { return const_cast<Raise*>(this); }
  ScopedEntities ddlCathegory() const { return Raise_; }
};

class Resignal                 : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Resignal(CLoc l);
  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  Resignal* toSelfResignal() const { return const_cast<Resignal*>(this); }

  ScopedEntities ddlCathegory() const { return Resignal_; }
};

class Return                : public Statement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<PlExpr> expr;

  Return(CLoc l, Ptr<PlExpr> _expr = 0);

  Ptr<Datatype> getDatatype() const;

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Codestream &str);
  bool getMinimalCommonDatatypeOfRetstatement(Ptr<Datatype> &dst, Ptr<Datatype> &funRettype);
  bool getRetStatementsToCast(Ptr<Datatype> &t, RetStmtsToCast &retStmtsToCast);

  void replaceChildsIf(Sm::ExprTr tr);
  bool castReturnStatementsToDatatype(Ptr<Datatype> &t);

  Return *toSelfReturn() const { return const_cast<Return*>(this); }
  ScopedEntities ddlCathegory() const { return Return_; }
};


class SqlStatement : public SqlStatementInterface {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  CLoc getLLoc() const { return GrammarBase::getLLoc(); }
  virtual ~SqlStatement() {}
public:
  bool beginedFrom(uint32_t l, uint32_t c) const { return GrammarBase::beginedFrom(l, c); }
  bool beginedFrom(uint32_t l) const { return GrammarBase::beginedFrom(l); }
};


class Commit                  : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> commitComment;
  Commit(CLoc l, Id* _commitComment);
  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return COMMIT; }
  void collectSNode(SemanticTree *) const;
  void linterDefinition(Sm::Codestream &str)  { str << "COMMIT"; }
  void replaceChildsIf(Sm::ExprTr tr);

  Commit* toSelfCommit() { return this; }
  ScopedEntities ddlCathegory() const { return Commit_; }
};

class SelectStatement         : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Subquery> subquery;

  SelectStatement(CLoc l, Subquery* _subquery);
  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return SELECT_STATEMENT; }

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  bool translatedAsExecuteWithoutDirect() const;

  SelectStatement *toSelfSelectStatement() const { return const_cast<SelectStatement*>(this); }
  ScopedEntities ddlCathegory() const { return SelectStatement_; }
};


class Savepoint               : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> savepoint;

  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }

  Savepoint(CLoc l, Id *_savepoint);
  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return SAVEPOINT; }

  void collectSNode(SemanticTree *n) const;
  bool isSavepoint() const { return true; }
  void linterDefinition(Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  Savepoint* toSelfSavepoint() const { return const_cast<Savepoint*>(this); }
  ScopedEntities ddlCathegory() const { return Savepoint_; }
};


class Rollback                : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Id> savepoint;

  Rollback(CLoc l = cl::emptyFLocation(), Ptr<Id> _savepoint = 0);
  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return ROLLBACK; }

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  Rollback* toSelfRollback() const { return const_cast<Rollback*>(this); }
  ScopedEntities ddlCathegory() const { return Rollback_; }
};


class LockTable               : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<List<Id2> > tables;
  lock_table::LockMode  lockMode;
  lock_table::WaitMode  waitMode;

  Ptr<Id2> currentTbl;

  LockTable(CLoc l, List<Id2> *_tables,  lock_table::LockMode _lockMode, lock_table::WaitMode _waitMode);

  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return LOCK_TABLE; }
  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Sm::Codestream &str);
  void sqlDefinition(Sm::Codestream &str);
  void sqlDefinitionSingleTbl(Sm::Codestream &str, Ptr<Id2> &tbl);
  void replaceChildsIf(Sm::ExprTr tr);

  bool translatedAsExecuteWithoutDirect() const;

  LockTable* toSelfLockTable() const { return const_cast<LockTable*>(this); }
  ScopedEntities ddlCathegory() const { return LockTable_; }
};


class Transaction             : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  transaction::TransactionType transactionType;
  Ptr<Id>                      name;

  Ptr<Datatype> getDatatype() const { return Ptr<Datatype>(); }
  Sm::IsSubtypeValues isSubtype(ResolvedEntity*, bool) const { return EXPLICIT; }

  Transaction(CLoc l, transaction::TransactionType _transactionType, Id *_name);

  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return TRANSACTION; }

  void collectSNode(SemanticTree *n) const;
  void linterDefinition(Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  ScopedEntities ddlCathegory() const { return Transaction_; }
};

class IntoCollections         : public List<RefExpr> {
public:
  typedef List<RefExpr> Base;
  typedef RefExpr Elem;

  IntoCollections() : Base(), bulkCollect(false) {}
  IntoCollections(const IntoCollections & other) : Base(other), bulkCollect(other.bulkCollect) {}
  IntoCollections(Ptr<Elem> elem) : Base(elem), bulkCollect(false) {}
  IntoCollections(Elem *elem)     : Base(elem), bulkCollect(false) {}
  IntoCollections(Ptr<IdEntitySmart> elem)     : Base(new Elem(elem)), bulkCollect(false) {}

  bool isBulkCollect() const { return bulkCollect; }
  void setBulkCollect(bool flag) { bulkCollect = flag; }
private:
  bool bulkCollect;
};

class ReturnInto              : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  enum QueryMode {FOR_INSERT, FOR_UPDATE, FOR_DELETE};

  Ptr<List<SqlExpr> > exprList;
  Ptr<IntoCollections> intoCollections;

  ReturnInto(CLoc l, Ptr<List<SqlExpr> > _exprList, Ptr<IntoCollections> _intoCollections);

  void collectSNode(SemanticTree *n) const;

  Ptr<Sm::Datatype> getDatatype() const;
  bool              getFields(EntityFields &fields) const;
  ScopedEntities    ddlCathegory() const { return ReturnInto_; }

  SemanticTree *toSTreeBase() const;
  void replaceChildsIf(Sm::ExprTr tr) { replace(tr, exprList, intoCollections); }
  void setupQueryParams(QueryMode mode, Ptr<ChangedQueryEntity> entity, Ptr<Id> alias, Ptr<Sm::WhereClause> where);
  void sqlDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);

private:
  void translateIntoList(Sm::Codestream &str);
  void translateBulkCollections(Sm::Codestream &str);

  QueryMode               qMode;
  Ptr<ChangedQueryEntity> qEntity;
  Ptr<Id>                 qAlias;
  Ptr<WhereClause>        qWhere;
  mutable Ptr<Datatype>   thisDatatype;
};

class WhereClause             : public GrammarBaseSmart, public Translator {
  Ptr<PlExpr> condition;
  Ptr<Id>     whereCurrentOfCursorName;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  WhereClause(CLoc l, PlExpr *_condition);
  WhereClause(CLoc l, Id *_whereCurrentOfCursorName);

  void collectSNode(SemanticTree *n) const;

  virtual void sqlDefinition (Sm::Codestream &str);
  virtual ~WhereClause();
  void replaceChildsIf(Sm::ExprTr tr);
};

class ChangedQueryEntity      : public ResolvedEntitySNodeLoc {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  bool hasOnly = false;

  ScopedEntities ddlCathegory() const { return SqlEntity_; }

  ChangedQueryEntity(CLoc l);
  ChangedQueryEntity();

  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
};

class ChangedQueryRestricted : public ChangedQueryEntity {
  Ptr<Subquery>     subquery;
  Ptr<view::ViewQRestriction> restriction;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  ChangedQueryRestricted(CLoc l, Subquery *_subquery, view::ViewQRestriction *_restriction);

  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;
  ResolvedEntity* getNextDefinition() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  Ptr<Sm::Datatype> getDatatype() const;

  SemanticTree *toSTreeBase() const;
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);
};
class ChangedQueryEntityRef : public ChangedQueryEntity {
  Ptr<Id2> entityRef; // table, view, materialized view
  Ptr<Id> dblink;

protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  ChangedQueryEntityRef(CLoc l, Id2 *_entityRef, Id *_dblink = 0);


  Ptr<Id> getName() const;
  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &field) const;
  ResolvedEntity* getNextDefinition() const;
  Ptr<Sm::Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  SemanticTree *toSTreeBase() const;
  void sqlDefinition(Sm::Codestream &str);
  void sqlReference (Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  ChangedQueryEntityRef* toSelfChangedQueryEntityRef() const { return const_cast<ChangedQueryEntityRef*>(this); }
};
/// TABLE ( <expr> ) [ (+) ]
class ChangedQueryTableCollectionExpr : public ChangedQueryEntity {
  Ptr<SqlExpr> expr;
  bool         hasJoin;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  ChangedQueryTableCollectionExpr(CLoc l, SqlExpr *_expr, bool _hasJoin);

  bool getFieldRef(Ptr<Id> &field);
  bool getFields(EntityFields &fields) const;
  ResolvedEntity* getNextDefinition() const;

  void sqlDefinition(Sm::Codestream &) { throw 999; }

  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  Ptr<Sm::Datatype> getDatatype() const;

  SemanticTree *toSTreeBase() const;
  void replaceChildsIf(Sm::ExprTr tr);
};

class DeleteFrom              : public SqlStatement {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  int         fromKeyword;
  Ptr<ChangedQueryEntity>   name;
  Ptr<Id>          alias;
  Ptr<WhereClause> whereClause;
  Ptr<ReturnInto>  returnInto;

  Ptr<List<Sm::QueryHint> > hints;

  DeleteFrom(CLoc                l,
             int                 _fromKeyword,
             ChangedQueryEntity *_name,
             Id                 *_alias,
             WhereClause        *_whereClause,
             ReturnInto         *_returnInto);

  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return DELETE_FROM; }

  void collectSNode(SemanticTree *node) const;
  void sqlDefinition (Sm::Codestream &str);
  void linterDefinition (Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  DeleteFrom* toSelfDeleteFrom() const { return const_cast<DeleteFrom*>(this); }
  ScopedEntities ddlCathegory() const  { return DeleteFrom_; }
};


class InsertInterface : public SqlStatementInterface {
public:
  enum CathegoryInsertInterface { SINGLE_TABLE, MULTIPLE_TABLE_VALUES , MULTIPLE_TABLE_CONDITIONAL };

  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return INSERT; }
  virtual CathegoryInsertInterface cathegoryInsertInterface() const = 0;
};


class Insert : public InsertInterface  {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  CLoc getLLoc() const { return GrammarBase::getLLoc(); }
  bool beginedFrom(uint32_t l, uint32_t c) const { return GrammarBase::beginedFrom(l, c); }
  bool beginedFrom(uint32_t l) const { return GrammarBase::beginedFrom(l); }
  virtual ~Insert() {}
};

namespace insert {

class InsertingValues : public virtual GrammarBaseSmart, public Translator {
public:
  enum  CathegoryInsertingValues { RECORD, EXPRESSION_LIST };

  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual CathegoryInsertingValues cathegoryInsertingValues() const = 0;
  virtual ~InsertingValues() {}
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
};

class InsertingRecordValues         : public InsertingValues {
  Ptr<RefExpr> record;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  InsertingRecordValues(CLoc l, Ptr<IdEntitySmart> _record);
  void collectSNode(SemanticTree *n) const;
  CathegoryInsertingValues cathegoryInsertingValues() const { return RECORD; }
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);
};
class InsertingExpressionListValues : public InsertingValues {
public:
  Ptr<List<SqlExpr> > exprList;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  InsertingExpressionListValues(CLoc l, Ptr<List<SqlExpr> > _exprList);

  void collectSNode(SemanticTree *n) const;
  CathegoryInsertingValues cathegoryInsertingValues() const { return EXPRESSION_LIST; }
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);
};

class InsertFrom    : public virtual GrammarBaseSmart, public Translator {
public:
  enum  CathegoryInsertFrom { SUBQUERY, VALUES };
  virtual CathegoryInsertFrom cathegoryInsertFrom() const = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~InsertFrom() {}
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;

  virtual Ptr<ReturnInto> getReturning() { return 0; }
};

class InsertFromSubquery         : public InsertFrom {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Subquery>  subquery;

  InsertFromSubquery(CLoc l, Ptr<Subquery> subq);
  CathegoryInsertFrom cathegoryInsertFrom() const { return SUBQUERY; }
  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Sm::Codestream &s);
  void replaceChildsIf(Sm::ExprTr tr);
};
class InsertFromValues           : public InsertFrom {
public:
  Ptr<InsertingValues> value;
  Ptr<ReturnInto>      returning;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  InsertFromValues(CLoc l, Ptr<InsertingValues> val, Ptr<ReturnInto> ret);
  CathegoryInsertFrom cathegoryInsertFrom() const { return VALUES; }

  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  Ptr<ReturnInto> getReturning() { return returning; }
};
class Into                       : public GrammarBaseSmart, public Translator {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef List<SqlExpr> EntityList;
  Ptr<ChangedQueryEntity> entity;
  Ptr<Id>         alias;
  Ptr<EntityList> fields;

  // Into(CLoc l, Ptr<ChangedQueryEntity> _entity, Ptr<Id> _alias, Ptr<EntityList> _fields);
  Into(CLoc l, Ptr<ChangedQueryEntity> _entity, Ptr<Id> _alias, Ptr<List<SqlExpr> > _fields);

  Sm::SemanticTree *collectSNode(Sm::SemanticTree *parent) const;
  void sqlDefinition(Sm::Codestream &str);
  virtual ~Into();
  void replaceChildsIf(Sm::ExprTr tr);
};

class SingleInsert              : public Insert {
public:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  Ptr<Into>       into;
  Ptr<InsertFrom> data;

  SingleInsert(CLoc l, Into *_into, InsertFrom *_data);
  CathegoryInsertInterface cathegoryInsertInterface() const { return SINGLE_TABLE; }

  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Sm::Codestream &str);
  void linterDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);

  SingleInsert *toSelfSingleInsert() { return this; }
  ScopedEntities ddlCathegory() const { return SingleInsert_; }
};

class InsertValues               : public GrammarBaseSmart {
protected:
//  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<Into>            into;
  Ptr<InsertingValues> values;

  InsertValues(CLoc l, Into *_into, InsertingValues *_values);
  void collectSNode(SemanticTree *n) const;
  void  replaceChildsIf(Sm::ExprTr tr);
};

class MultipleValuesInsert       : public Insert {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<List<InsertValues> > values;
  Ptr<Subquery>  subquery;

  MultipleValuesInsert(CLoc l, List<InsertValues> *_values, Subquery *_subquery);

  CathegoryInsertInterface cathegoryInsertInterface() const { return MULTIPLE_TABLE_VALUES; }

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);
  ScopedEntities ddlCathegory() const { return MultipleValuesInsert_; }
};

namespace conditional_insert {
  class InsertWhenThen             : public GrammarBaseSmart {
  public:
     typedef List<InsertValues> ThenValues;
     Ptr<PlExpr>     when;
     Ptr<ThenValues> thenValues;

     InsertWhenThen(CLoc l, PlExpr *_when, ThenValues *_thenValues);

     void collectSNode(SemanticTree *n) const;
     void replaceChildsIf(Sm::ExprTr tr);
  };
  class InsertConditional          : public GrammarBaseSmart {
  public:
    conditional_insert::AllOrFirst spec;
    Ptr<List<InsertWhenThen> >     whenThenClauses;
    Ptr<List<InsertValues> >       elseValuesInsert;

    InsertConditional(CLoc l, conditional_insert::AllOrFirst _spec, List<InsertWhenThen> *_whenThenClauses, List<InsertValues> *_elseValuesInsert);
    void collectSNode(SemanticTree *n) const;
    void replaceChildsIf(Sm::ExprTr tr);
  };
}

class MultipleConditionalInsert  : public Insert {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  typedef conditional_insert::InsertConditional Condition;
  Ptr<Condition>  condition;
  Ptr<Subquery> subquery;

  MultipleConditionalInsert(CLoc l,  Condition *_values, Subquery *_subquery);

  CathegoryInsertInterface cathegoryInsertInterface() const { return MULTIPLE_TABLE_CONDITIONAL; }

  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);

  ScopedEntities ddlCathegory() const { return MultipleConditionalInsert_; }
};
}


namespace update {

class SetItem: public virtual GrammarBaseSmart, public Translator {
public:
  enum  CathegorySetItem { FIELD_FROM_EXPR, FIELDS_FROM_SUBQUERY };
  virtual ~SetItem() {}
  virtual CathegorySetItem cathegorySetItem() const = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
};

class FieldFromExpr      : public PlExpr {
public:
  Ptr<SqlExpr> field;
  Ptr<SqlExpr> expr; // empty => DEFAULT
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  FieldFromExpr(CLoc l, Ptr<IdEntitySmart> _field, Ptr<SqlExpr> _expr = 0);
  FieldFromExpr(CLoc l, Ptr<SqlExpr> _field, Ptr<SqlExpr> _expr);

  FieldFromExpr* toSelfFieldFromExpr() const { return const_cast<FieldFromExpr*>(this); }


  virtual Ptr<Sm::Datatype> getDatatype()  const { return 0; }

  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Sm::Codestream &s);
  virtual void replaceChildsIf(Sm::ExprTr tr);
};
class FieldsFromSubquery : public PlExpr {
public:
  typedef List<IdEntitySmart> EntityList;
private:
  Ptr<EntityList> fields;
  Ptr<Subquery>   subquery;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  FieldsFromSubquery(CLoc l, EntityList *_fields, Ptr<Subquery> q);

  virtual Ptr<Sm::Datatype> getDatatype()  const { return 0; }

  FieldsFromSubquery* toSelfFieldsFromSubquery() const { return const_cast<FieldsFromSubquery*>(this); }
  void collectSNode(SemanticTree *n) const;
  void sqlDefinition(Codestream &str);
  virtual void replaceChildsIf(Sm::ExprTr tr);
};

class SetClause: public virtual GrammarBaseSmart, public Translator {
public:
  Ptr<ChangedQueryEntity>         updatedEntity;
  Ptr<Id>                         updatedAlias;

  enum  CathegorySetClause { ROW_RECORD , UPDATING_LIST };
  virtual CathegorySetClause cathegorySetClause() const = 0;
  virtual void collectSNode(SemanticTree *node) const = 0;
  virtual ~SetClause();
  virtual void replaceChildsIf(Sm::ExprTr tr) = 0;
};
class SetRowRecord    : public SetClause {
  Ptr<RefExpr> rowRecord;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  SetRowRecord(CLoc l, Ptr<Id> _rowRecord);
  ~SetRowRecord();
  void collectSNode(SemanticTree *n) const;
  CathegorySetClause cathegorySetClause() const  { return ROW_RECORD; }
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);
};
class SetUpdatingList : public SetClause {
public:
  typedef PlExpr UpdatingItemType;
  Ptr<List<UpdatingItemType> > updatingList;
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  SetUpdatingList(CLoc l, Ptr<List<UpdatingItemType> > _updatingList);
  ~SetUpdatingList();

  void collectSNode(SemanticTree *n) const;
  CathegorySetClause cathegorySetClause() const  { return UPDATING_LIST; }
  void sqlDefinition(Sm::Codestream &str);
  void replaceChildsIf(Sm::ExprTr tr);
};
}
class Update                  : public SqlStatement, public virtual ResolvedEntitySNode {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
public:
  Ptr<ChangedQueryEntity> entity;
  Ptr<Id>                 alias;
  Ptr<update::SetClause>  setClause;
  Ptr<WhereClause>        where;
  Ptr<ReturnInto>         returnInto;

  Ptr<List<Sm::QueryHint> > hints;

  Update(CLoc l, ChangedQueryEntity *_entity, Id *_alias,  update::SetClause *_setClause,
         WhereClause *_where, ReturnInto *_returnInto);

  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return UPDATE; }
  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
  void linterDefinition(Sm::Codestream &);
  void sqlDefinition(Sm::Codestream &s);
  void replaceChildsIf(Sm::ExprTr tr);

  Update* toSelfUpdate() const { return const_cast<Update*>(this); }
  ScopedEntities ddlCathegory() const { return Update_; }
};

class MergeFieldAssignment : public GrammarBaseSmart {
public:
  Ptr<IdEntitySmart> field;
  Ptr<SqlExpr>       assignment;
  MergeFieldAssignment(CLoc l, Ptr<IdEntitySmart> _field, Ptr<SqlExpr> _assignment);
  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);
};

class MergeUpdate : public GrammarBaseSmart {
public:
  Ptr<List<MergeFieldAssignment> > fieldsAssignments;
  Ptr<PlExpr> where;
  Ptr<PlExpr> deleteWhere;

  MergeUpdate(CLoc l, List<MergeFieldAssignment> *_fieldsAssignments, PlExpr *_where, PlExpr *_deleteWhere);
  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);
};

class MergeInsert : public GrammarBaseSmart {
public:
  Ptr<List<Id> >      fields;
  Ptr<List<SqlExpr> > values;
  Ptr<PlExpr>         where;

  MergeInsert(CLoc l, List<Id> *_fields, List<SqlExpr> *_values, PlExpr *_where);
  void collectSNode(SemanticTree *n) const;
  void replaceChildsIf(Sm::ExprTr tr);
};

class Merge: public SqlStatement, public virtual ResolvedEntitySNode, public virtual FromResolver {
protected:
  virtual SmartVoidType* getThisPtr() const { return (SmartVoidType*)this; }
  mutable SemanticTree *fromNode = 0;
  Ptr<FromList>   fromList;

public:
  Ptr<From>       mergedEntity;
  Ptr<From>       usingClause;

  Ptr<PlExpr>     onCondition;

  Ptr<MergeUpdate>    matchedUpdate; // when matched
  Ptr<MergeInsert> notMatchedInsert; // when not matched

  Merge(CLoc l, From *ent, From *_using, PlExpr *onCond, MergeUpdate *upd, MergeInsert *ins);
  CathegorySqlStatementInterface cathegorySqlStatementInterface() const { return MERGE; }

  void collectSNode(SemanticTree *n) const;
  SemanticTree *toSTreeBase() const;
  Ptr<Sm::Datatype> getDatatype() const;
  Sm::IsSubtypeValues isSubtype(ResolvedEntity *supertype, bool plContext) const;

  bool getFields(EntityFields &fields) const;
  bool getFieldRef(Ptr<Sm::Id> &field);

  bool getFieldRefByFromList(Ptr<Id> &field, SemanticTree *reference);
  bool findInFactoringList(Ptr<Id> &/*factoringTable*/) const  { return false; }

  ResolvedEntity* getNextDefinition() const;


  void linterDefinition(Sm::Codestream &s);
  void sqlDefinition(Sm::Codestream &s);
  void linterReference(Sm::Codestream &str);

  ScopedEntities ddlCathegory() const { return Merge_; }
  void replaceChildsIf(Sm::ExprTr tr);
  bool translatedAsExecuteWithoutDirect() const { return true; }
};

}


#endif // SEMANTIC_STATEMENTS

