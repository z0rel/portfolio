#ifndef CODESTREAM_H
#define CODESTREAM_H
#include <ostream>
#include <sstream>
#include <vector>
#include <list>
#include <limits>
#include <map>
#include <set>
#include "config_converter.h"
#include "smartptr.h"
#include "lex_location.h"
#include "model_head.h"

class UserContext;
class LinterWriter;

namespace Sm {
  using namespace std;
  using namespace smart;
  class Id;
  class ResolvedEntity;
  class IdEntitySmart;
  class StatementInterface;
  namespace pragma {
    class Pragma;
  }
  class RefAbstract;
  class AlgebraicCompound;
  class StatisticNode;

  namespace s {
    class CMultiLineComment;
    class OMultiLineComment;
    class TextChunk;
    class MultilineTextChunck;
    class Subconstruct;
    class ocmd;
    class statementPointer;
    class executeStatementPointer;
    class grant;
    class tab;
    class Name;
  }

class Spacer;
class Codestream;

typedef std::list<Ptr<Spacer> > SpacerStream;


enum class Privs {
  EXECUTE,
  SELECT,
  UPDATE,
  REFERENCES,
  INDEX,

  INSERT,
  DELETE
};


enum class CmdCat {
  EMPTY,
  DROP_USER,
  DROP_INDEX,
  DROP_TABLE,
  ALTER_TABLE,
  VARIABLE,
  TABLE_FOR_OBJECT,
  INDEX_FOR_OBJECT,
  PROC,
  VIEW,
  TRIGGER,
  CREATE_SPACER,
  GRANT,

  __LAST_CATHEGORY__
};

class CodestreamState {
public:
  enum OutputMode        { INL, LINDESK, CALL };
  enum ProcedureMode     { SQL, PROC };
  enum CodegeneratorMode { ORACLE, LINTER };
  enum NamesMode         { DEFINITION, DECLARATION, REFERENCE };
  enum Level             { USER, PACKAGE, BLOCK, STATEMENT, VIEW_QUERY };

private:
  ProcedureMode     procMode_                    = PROC;
public:
  OutputMode        outputMode_                  = LINDESK;
  CodegeneratorMode dbMode_                      = LINTER;
  NamesMode         namesMode_                   = DEFINITION;
  bool              blockEntry                   = false;
  int               initializerBranchId          = 0;
  bool              queryForExecute_             = false;
  bool              queryMakeStr_                = false;

  int               dynamicSqlDeep_              = 0;

  bool              expandSelect_                = false;
  bool              executeBlock_                = false;
  bool              indentingLongConstruct_      = true;
  bool              isCallOperator_              = false;
  mutable bool      isNotInl_                    = true;
  int               indentingLevel_              = 0;
  int               subconstructLevelAfterComma_ = -1;
  bool              isSelectedField_             = false;
  unsigned int      uniqueLevel_                 = 0;
  bool              isLogger_                    = false;
  bool              isDynamicCollection_         = false;
  Codestream       *parentStream_                = NULL;
  bool              isModelStatistic             = false;
  bool              isErrorLog                   = false;
  bool              isDynamicSignOutput_         = false;
  bool              isMakestrArglistTrStage_     = false;

  UserContext      *currentUser_                 = 0;

  typedef std::vector<Sm::PlExpr*> QueryExternalIdentficators;
  QueryExternalIdentficators queryExternalIdentficators;
  std::vector<bool>               inExpression_;

  unsigned int skipListFromStart_ = 0;

// Level level;

  bool inl() { return outputMode_ == INL; }

  ProcedureMode procMode() const { return procMode_; }
  void procMode(ProcedureMode mode) {
    procMode_ = mode;
    if (mode == SQL)
      blockEntry = false;
    else
      blockEntry = true;
  }
  bool dynamicCollection(bool val) { bool oldVal = isDynamicCollection_; isDynamicCollection_ = val; return oldVal; }

  CodestreamState() {}
  CodestreamState(ProcedureMode m) : procMode_(m) {}

};

class SpacerCathegories {
public:
  enum Cathegory {
    Empty_,
    OColumn_, CColumn_,
    OCommalist_, CCommalist_,
    OTabCommalist_, CTabCommalist_,
    OTabLevel_, CTabLevel_,
    OBracket_, CBracket_,
    OBracketCall_, CBracketCall_,
    OBracketInsert_, CBracketInsert_,
    OBracketArglist_, CBracketArglist_,
    OBracketView_, CBracketView_,
    GrantCmd_,
    OGrantCmd_, CGrantCmd_,
    OCallCmd_, CCallCmd_,
    OCreate_, CCreate_,
    StatementPointer_,
    ExecuteStatementPointer_,
    Name_,
    Connect_,
    CommaMakestr_,
    Comma_,
    Endl_,
    StubLength_,
    Comment_,
    OMultiLineComment_, CMultiLineComment_,
    GroupOpen_, GroupClose_,
    Tab_,
    Stub_,
    OInl_, CInl_,
    InlMessage_,
    Subconstruct_,
    Loc_,
    ILoc_,
    TextChunk_,
    MultilineTextChunck_,
    Semicolon_,
    Oquote_, Cquote_,
    DisableIndenting_, EnableIndenting_,

    LastSpacerCathegory_
  };
};

struct PairSpacerCathegories : public SpacerCathegories {
  Cathegory pairCathegories[LastSpacerCathegory_];
  bool emptyCathegories[LastSpacerCathegory_];

  PairSpacerCathegories() {
    for (int i = 0; i < LastSpacerCathegory_; ++i)
      pairCathegories[i] = (Cathegory)i;

    for (int i = 0; i < LastSpacerCathegory_; ++i)
      emptyCathegories[i] = false;

    setPairCathegory(GroupOpen_        , GroupClose_       );
    setPairCathegory(EnableIndenting_  , DisableIndenting_ );
    setPairCathegory(OBracket_         , CBracket_         );
    setPairCathegory(OBracketArglist_  , CBracketArglist_  );
    setPairCathegory(OBracketInsert_   , CBracketInsert_   );
    setPairCathegory(OBracketCall_     , CBracketCall_     );
    setPairCathegory(OBracketView_     , CBracketView_     );
    setPairCathegory(OCommalist_       , CCommalist_       );
    setPairCathegory(OColumn_          , CColumn_          );
    setPairCathegory(OTabCommalist_    , CTabCommalist_    );
    setPairCathegory(OTabLevel_        , CTabLevel_        );
    setPairCathegory(OGrantCmd_        , CGrantCmd_        );
    setPairCathegory(OCallCmd_         , CCallCmd_         );
    setPairCathegory(OCreate_          , CCreate_          );
    setPairCathegory(OMultiLineComment_, CMultiLineComment_);
    setPairCathegory(GroupOpen_        , GroupClose_       );
    setPairCathegory(OInl_             , CInl_             );
    setPairCathegory(Oquote_           , Cquote_           );

    emptyCathegories[OCommalist_   ] = true;
    emptyCathegories[OColumn_      ] = true;
    emptyCathegories[OTabCommalist_] = true;
    emptyCathegories[OTabLevel_    ] = true;
    emptyCathegories[CCommalist_   ] = true;
    emptyCathegories[CColumn_      ] = true;
    emptyCathegories[CTabCommalist_] = true;
    emptyCathegories[CTabLevel_    ] = true;
    emptyCathegories[Stub_         ] = true;
  }

  void setPairCathegory(Cathegory l, Cathegory r) {
    pairCathegories[l] = r;
    pairCathegories[r] = l;
  }

};



class Spacer : public Smart4, public SpacerCathegories {
public:

  static size_t spacerId();

#ifdef DISABLE_INDEX_SPACERS
  size_t sid = spacerId();
#endif

public:
  Spacer() {}
  Spacer(const Spacer&) : Smart4() {}


  virtual ResolvedEntity* commandEntity() const { return 0; }

  virtual std::string dbConnectUname() const { return ""; }
  virtual std::string dbConnectPassword() const { return ""; }
  virtual UserContext* userContext() const { return 0; }

  /// Критерий "это длинная скобка"
  virtual bool isOInl() const { return false; }
  virtual bool isEmptyTab() const { return false; }
  virtual bool isLoc () const { return false; }

  virtual Sm::CmdCat commandCmdCathegory() const {  return CmdCat::EMPTY; }

  virtual ~Spacer() {}

  virtual int offsetAddVal() const { return 0; }

  virtual Cathegory cathegory() const = 0;
  virtual int       tabsize() const { return 0; }
  virtual void      lengthAdd(size_t) {}
  virtual void      toUnformattedString(string &/*dst*/) const {}
  virtual unsigned int length() const { return 0; }

  virtual int  level() const { return 0; }
  virtual bool isName() const { return false; }
  virtual bool isStub() const { return false; }
  virtual bool isSemicolon() const { return false; }
  virtual bool isOMultilineComment() const { return false; }
  virtual bool isCMultilineComment() const { return false; }
  virtual bool isEmpty() const { return false; }
  virtual string uniqueHeader() const { return ""; }

  virtual int getContextLength(int /*tabAtBeginOfLine*/) { return 0; }

  virtual s::tab *toSelfTab() const { return 0; }
  virtual s::statementPointer *toSelfStatementPointer() const { return 0; }

  virtual Sm::s::executeStatementPointer* toSelfExecuteStatementPointer() const { return 0; }
  virtual s::Subconstruct *toSelfSubconstruct() const { return 0; }
  virtual s::CMultiLineComment *toSelfCMultiLineComment() const { return 0; }
  virtual s::OMultiLineComment *toSelfOMultiLineComment() const { return 0; }
  virtual s::TextChunk *toSelfTextChunk() const { return 0; }
  virtual s::MultilineTextChunck *toSelfMultilineTextChunk() const { return 0; }
  virtual s::ocmd *toSelfOcmd() const { return 0; }
  virtual s::grant *toSelfGrant() const { return 0; }
  virtual s::Name* toSelfName() const { return 0; }

  virtual bool beginedFrom(int, int) const { return false; }
};




class Def  { /* Definition  */ };
class DefR { /* Definition  */ };
class Decl { /* Declaration */ };
class Ref1 { /* Reference   */ };
class Sql_ { /* Reference   */ };
class Proc { /* Reference   */ };

namespace s {

extern Def  def ;
extern DefR defR;
extern Decl decl;
extern Ref1 ref;
extern Sql_ sql ;
extern Proc proc;

}

struct skip {
  const int i;
  skip(int _i) : i(_i) {}
};

namespace s {
  class Name;
}


struct CodestreamStackItem {
  enum Cathegory { Preactions, Predeclarations, Declarations, Prefixes, Actions, Suffixes, Postactions, Userpostactions };
  Cathegory    cat;
  SpacerStream spacers;

  CodestreamStackItem(Cathegory _cat = Actions)
    : cat(_cat),
      spacers() {}

  CodestreamStackItem(const CodestreamStackItem& o)
    : cat(o.cat),
      spacers(o.spacers) {}

  void activate (SpacerStream **pspacers) { *pspacers = &spacers; }

  void join       (CodestreamStackItem &oth);
  void joinPreitem(CodestreamStackItem &oth);
  void swap       (CodestreamStackItem &oth);


  bool empty() const { return spacers.empty(); }

  void clear() { spacers.clear(); }
};


struct CodestreamStack {
  CodestreamStackItem preactions;
  CodestreamStackItem predeclarations;
  CodestreamStackItem declarations;
  CodestreamStackItem prefixes;
  CodestreamStackItem actions;
  CodestreamStackItem suffixes;
  CodestreamStackItem postactions;
  CodestreamStackItem userpostactions;

  CodestreamStackItem *activeItem;
  std::vector<CodestreamStackItem *> streamStack;

  CodestreamStack() :
      preactions     (CodestreamStackItem::Preactions     ),
      predeclarations(CodestreamStackItem::Predeclarations   ),
      declarations   (CodestreamStackItem::Declarations   ),
      prefixes       (CodestreamStackItem::Prefixes       ),
      actions        (CodestreamStackItem::Actions        ),
      suffixes       (CodestreamStackItem::Suffixes       ),
      postactions    (CodestreamStackItem::Postactions    ),
      userpostactions(CodestreamStackItem::Userpostactions),
      activeItem(&actions) {}

  CodestreamStack(const CodestreamStack &o) :
      preactions     (o.preactions     ),
      predeclarations(o.predeclarations),
      declarations   (o.declarations   ),
      prefixes       (o.prefixes       ),
      actions        (o.actions        ),
      suffixes       (o.suffixes       ),
      postactions    (o.postactions    ),
      userpostactions(o.userpostactions),
      activeItem  (&actions) { streamStack.push_back(&actions); }

  void activate(SpacerStream **pspacers, CodestreamStackItem *item) {
    item->activate(pspacers);
    activeItem = item ;
    streamStack.push_back(item);
  }

  void activatePreactions     (SpacerStream **pspacers) { activate(pspacers, &preactions     ); }
  void activatePredeclarations(SpacerStream **pspacers) { activate(pspacers, &predeclarations); }
  void activateDeclarations   (SpacerStream **pspacers) { activate(pspacers, &declarations   ); }
  void activatePrefixes       (SpacerStream **pspacers) { activate(pspacers, &prefixes       ); }
  void activateActions        (SpacerStream **pspacers) { activate(pspacers, &actions        ); }
  void activateSuffixes       (SpacerStream **pspacers) { activate(pspacers, &suffixes       ); }
  void activatePostactions    (SpacerStream **pspacers) { activate(pspacers, &postactions    ); }
  void activateUserpostactions(SpacerStream **pspacers) { activate(pspacers, &userpostactions); }
  void activatePrevious       (SpacerStream **pspacers) {
    streamStack.pop_back();
    if (streamStack.size()) {
      CodestreamStackItem *item = streamStack.back();
      item->activate(pspacers); activeItem = item;
    }
    else
      throw 999;
  }

  void joinPreactions     (CodestreamStack &oth) { actions.joinPreitem(oth.preactions     ); }
  void joinPredeclarations(CodestreamStack &oth) { actions.joinPreitem(oth.predeclarations); }
  void joinDeclarations   (CodestreamStack &oth) { actions.joinPreitem(oth.declarations   ); }
  void joinPrefixes       (CodestreamStack &oth) { actions.joinPreitem(oth.prefixes       ); }
  void joinSuffixes       (CodestreamStack &oth) { activeItem->join   (oth.suffixes       ); }
  void joinPostactions    (CodestreamStack &oth) { actions.join       (oth.postactions    ); }
  void joinUserpostactions (CodestreamStack &oth) { actions.join(oth.userpostactions); }
  void join(CodestreamStack &oth) {
    preactions     .join(oth.preactions);
    declarations   .join(oth.declarations);
    prefixes       .join(oth.prefixes);
    actions        .join(oth.actions);
    suffixes       .join(oth.suffixes);
    postactions    .join(oth.postactions);
    userpostactions.join(oth.userpostactions);
  }
};


namespace s {
  class grant;
}

class Indenter;

class SkipNameIters : public SpacerCathegories {
public:
  const std::set<Sm::Spacer::Cathegory> cathegories;

  Sm::SpacerStream::iterator skipNext(Sm::SpacerStream::iterator it, const Sm::SpacerStream::iterator &endIt) const;
  Sm::SpacerStream::iterator skipPrev(Sm::SpacerStream::iterator it, const Sm::SpacerStream::iterator &begIt) const;

  static Sm::SpacerStream::iterator previousSignificantSpacer(Sm::Codestream &str);
  static bool previousSignificantSpacerIsEndl(Sm::Codestream &str);

  SkipNameIters();
};


class Codestream {
public:
  static std::set<string> granteeClauses;

  typedef map<Sm::ResolvedEntity*,  Ptr<s::grant>, Sm::LE_ResolvedEntities> GrantTargets;
  typedef map<Sm::Privs, GrantTargets > OutGrantees;
  static OutGrantees outGrantees;
  static Codestream *mainStream;

private:
   /// Строка-буффер с выровненными сгенерированными исходными кодами
  typedef std::vector<int /*columnNum*/> ColumnsLengths;
  typedef std::map<int /*groupNum*/, ColumnsLengths> GroupColumnsLengths;
  mutable GroupColumnsLengths groupColumnsLengths;

  typedef std::vector<std::vector<Spacer::Cathegory> > CathegoriesTable;
  mutable CathegoriesTable cathegoriesTable;


protected:

  typedef SpacerStream::iterator iterator;

  std::vector<CodestreamStack> codestreamStack;
  CodestreamStack             *currentLevel;
  mutable std::string          strstreambuf;
  SpacerStream                *activespacer_;

  CodestreamState              state_;

public:

  typedef std::set<ResolvedEntity*, Sm::LE_ResolvedEntities> GranteeEntities;
  typedef std::map<Privs, std::map<UserContext*/*user*/, GranteeEntities /*entities*/, LE_ResolvedEntities> > NeedToGrantee;
  typedef std::map<ResolvedEntity* /*entity*/, std::set< Privs /* privilegie */>, LE_ResolvedEntities > UserContextSortedGranteesClauses;
  typedef std::map<UserContext* /*user*/, UserContextSortedGranteesClauses, LE_ResolvedEntities> SortedGranteesClauses;
  NeedToGrantee needToGrantee;

  void enterExpr() { state_.inExpression_.push_back(true); }
  void outExpr() { state_.inExpression_.pop_back(); }
  bool isExpr() const { return state_.inExpression_.size(); }

  Codestream();
  ~Codestream();
  Codestream(const CodestreamState &s);
//  Codestream(const Codestream &oth);

  void dbMode   (CodestreamState::CodegeneratorMode _mode) { state_.dbMode_    = _mode; }
  void namesMode(CodestreamState::NamesMode         _mode) { state_.namesMode_ = _mode; }
  void procMode (CodestreamState::ProcedureMode     _mode) { state_.procMode(_mode); }

  void setBlockEntry() { state_.blockEntry = true; }
  void clrBlockEntry() { state_.blockEntry = false; }

  CodestreamState::CodegeneratorMode dbMode()    const     { return state_.dbMode_   ; }
  CodestreamState::NamesMode         namesMode() const     { return state_.namesMode_; }
  CodestreamState::ProcedureMode     procMode()  const     { return state_.procMode(); }

  bool isSql() const { return procMode() == CodestreamState::SQL; }
  bool isProc() const { return procMode() == CodestreamState::PROC; }

  CodestreamState &state() { return state_; }
  void state(const CodestreamState &s) { state_ = s; }
  const CodestreamState &cstate() const { return state_; }

  bool isSelectedField() const { return state_.isSelectedField_; }
  void isSelectedField(bool v) { state_.isSelectedField_ = v; }
  void deleteLocations();

  unsigned int nextUniqueLevel() { return ++state_.uniqueLevel_; }

  size_t streamPosition(string &str) const { return str.length(); }

  size_t actSPos()             const { return streamPosition(activestream()); }
  string &activestream()       const { return strstreambuf; }
  SpacerStream &activespacer() const { return *activespacer_; }

  void inlMode()     { state_.outputMode_ = CodestreamState::INL; }
  void lindeskMode() { state_.outputMode_ = CodestreamState::LINDESK; }
  void callMode()    { state_.outputMode_ = CodestreamState::CALL; }

  void levelPush();
  void levelPop ();

  int  indentingLevel() const { return state_.indentingLevel_; }
  void indentingLevel(int v)  { state_.indentingLevel_ = v; }
  void incIndentingLevel(int v = 1) { state_.indentingLevel_ += v; }
  void decIndentingLevel(int v = 1) { state_.indentingLevel_ -= v; }
  // preactions declarations prefixes actions postactions
  /// Склеить actions и postacitons в actions
  void joinPostactions    () { currentLevel->joinPostactions    (*currentLevel); }
  void joinUserpostactions() { currentLevel->joinUserpostactions(*currentLevel); }
  /// Склеить actions и preactions в actions
  void joinPreactions     () { currentLevel->joinPreactions     (*currentLevel); }
  void joinPredeclarations() { currentLevel->joinPredeclarations(*currentLevel); }
  void joinDeclarations   () { currentLevel->joinDeclarations   (*currentLevel); }
  void joinPrefixes       () { currentLevel->joinPrefixes       (*currentLevel); }
  void joinSuffixes       () { currentLevel->joinSuffixes       (*currentLevel); }
  void join()                { joinPrefixes(); joinDeclarations(); joinPostactions(); }
  void actionsToPrefixes()   { currentLevel->prefixes  .join(currentLevel->actions); }
  void actionsToSuffuxes()   { currentLevel->suffixes  .join(currentLevel->actions); }
  void actionsToPreactions() { currentLevel->preactions.join(currentLevel->actions); }
  void preactionsToLevelUpAsPreitem();


  CodestreamStack  *getCurrentLevel() { return currentLevel; }

  void activatePreactions     () { currentLevel->activatePreactions     (&activespacer_); }
  void activatePredeclarations() { currentLevel->activatePredeclarations(&activespacer_); }
  void activateDeclarations   () { currentLevel->activateDeclarations   (&activespacer_); }
  void activatePrefixes       () { currentLevel->activatePrefixes       (&activespacer_); }
  void activateActions        () { currentLevel->activateActions        (&activespacer_); }
  void activateSuffixes       () { currentLevel->activateSuffixes       (&activespacer_); }
  void activatePostactions    () { currentLevel->activatePostactions    (&activespacer_); }
  void activateUserpostactions() { currentLevel->activateUserpostactions(&activespacer_); }
  void activatePrevious       () { currentLevel->activatePrevious       (&activespacer_); }

  string str();
  string strWithoutEndl();
  void directStoreInDB();

  void store(const string &filename, bool skipIfEpmpty = false);
  void skipListFromStart(unsigned int i) { state_.skipListFromStart_ = i; }
  int  skipListFromStart() const { return state_.skipListFromStart_; }

  Codestream& addSpacer(Spacer *sp);
  Codestream& addSpacerText(const string &str);

private:
  void collectSpacerToStr(SpacerStream::iterator &it, SpacerStream &s, string &strdst, UserContext *&currentUser);

  void indentTextStream();
  void createLWriter(const Sm::Spacer &sp);
};




class PushProcModeContext {
public:
  Codestream &str;
  CodestreamState::ProcedureMode oldState;

  PushProcModeContext(Codestream &str_)
    : str(str_), oldState(str_.procMode()) {}
  ~PushProcModeContext() { str.procMode(oldState); }
};



void needToGranteeToSortedGrantee(Codestream::SortedGranteesClauses &sortedGranteesClauses, Sm::Codestream::NeedToGrantee &needToGrantee);
void outputSortedGranteeClause(Codestream::SortedGranteesClauses &sortedGranteesClauses, Codestream &str, UserContext *curUcntx, bool needGranteeToCurrentActor);
void outputGranteeClause(Sm::Codestream::NeedToGrantee &needToGrantee, Codestream &str, UserContext *curUcntx, bool needGranteeToCurrentActor);

}



#endif // CODESTREAM_H
