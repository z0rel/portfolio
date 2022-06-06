#ifndef INDENTER_H
#define INDENTER_H

#include <stack>
#include "config_converter.h"
#include "codestream.h"

#define LINE_LENGTH_MAX 111

namespace Sm {

class IndentedLineContext {
public:
  enum LineStage {
    EMPTY_STAGE,
    START_LINE,
    TAIL_LINE
  };
  Sm::SpacerStream::iterator startLineIt;

  unsigned int lineCurrentColumn = 0;
  unsigned int tabBeginOfLine_   = 0;
  LineStage    lineStage         = EMPTY_STAGE;

  void startLine(SpacerStream::iterator &_startLineIt);

  void updateLineAttributes(SpacerStream &stream, SpacerStream::iterator &currentSpacerIt);

  bool isTooLong() const { return lineCurrentColumn > 110; }

  unsigned int lineLength() const { return lineCurrentColumn; }
  unsigned int tabAtBeginOfLine() { return tabBeginOfLine_; }

};

class IndentedLine {
public:
  IndentedLineContext current;
  IndentedLineContext previous;

  unsigned int summaryLength = 0;

  Sm::SpacerStream::iterator it;
  Sm::SpacerStream &stream;
  bool isNewline = false;

  unsigned int &commentDepth;

  IndentedLine(Sm::SpacerStream::iterator _it, Sm::SpacerStream &_stream, unsigned int &_commentDepth)
    : it(_it), stream(_stream), commentDepth(_commentDepth) {}

  Sm::SpacerStream::iterator endIt() const { return stream.end(); }

  void startLine(SpacerStream::iterator &_startLineIt);

  void incAndUpdate();

  void afterIncrement(Sm::Spacer *previousSpacer = 0);

  bool isTooLong() const { return current.isTooLong(); }
  unsigned int tabAtBeginOfLine() { return current.tabAtBeginOfLine(); }
  unsigned int lineLength() const { return current.lineLength(); }

};


class Indenter;

#define INDENTER_CONTEXT_SKIP_TAIL                (1 << 0)
#define INDENTER_CONTEXT_INDENT_ONLY_LONG_CHILDS  (1 << 1)
#define INDENTER_CONTEXT_CHILDS_OFFSET            (1 << 1)

union IndenterContextFlags {
  typedef uint16_t Value;

  struct F {
    Value skipTail            :1;
    Value indentOnlyLongChilds:1;
    Value childsOffset        :1;
  };

  Value v;
  F     flags;

  IndenterContextFlags() : v(0xFFFF) {}

  bool skipTail()             const { return v & INDENTER_CONTEXT_SKIP_TAIL;               }
  bool indentOnlyLongChilds() const { return v & INDENTER_CONTEXT_INDENT_ONLY_LONG_CHILDS; }
  bool childsOffset()         const { return v & INDENTER_CONTEXT_CHILDS_OFFSET;           }

  void setSkipTail()             { v |= INDENTER_CONTEXT_SKIP_TAIL;               }
  void setIndentOnlyLongChilds() { v |= INDENTER_CONTEXT_INDENT_ONLY_LONG_CHILDS; }
  void setChildsOffset()         { v |= INDENTER_CONTEXT_CHILDS_OFFSET;           }

  void clrSkipTail()             { v &= ~INDENTER_CONTEXT_SKIP_TAIL;               }
  void clrIndentOnlyLongChilds() { v &= ~INDENTER_CONTEXT_INDENT_ONLY_LONG_CHILDS; }
  void clrChildsOffset()         { v &= ~INDENTER_CONTEXT_CHILDS_OFFSET;           }
};

typedef std::vector<unsigned int*> PositionInParent;

class IndentingContext;



class IndentingAction;


struct IndentedPos {
  unsigned int absolutePos;
  unsigned int tabsize;
  bool isMultiline = false;
  IndentingAction *act = 0;

  IndentedPos(unsigned int _p, unsigned int _tabsize, bool _isMultiline, IndentingAction *_act)
    : absolutePos(_p), tabsize(_tabsize), isMultiline(_isMultiline), act(_act) {}

  int posWithTabsize(unsigned int startCol) const;
};

class IndentingContextShared;

class IndentingContext {
private:
  uint32_t absoluteColumnPos = 0; // 4
  int16_t  tab_ = 0; // 2

public:
  IndenterContextFlags    flags;   // 2
  IndentingContextShared &shared;  // 8

private:
  void checkInvariant() const;

public:
  bool skipTailIndenting(IndentedPos &prevIndentedPos);
  bool skipChildsIndenting();

  inline int16_t tab() const { checkInvariant(); return tab_; }
  inline void setTab(int16_t _tab) { checkInvariant(); tab_ = _tab; checkInvariant(); }
  inline void incTab(int16_t _tab) { tab_ += _tab; checkInvariant(); }

  void incTabIfNeed();

  inline void setAbsoluteColumnPos(uint32_t val) {
    checkInvariant();
    absoluteColumnPos = val;
    checkInvariant();
  }

  inline void checkout(IndentingContext &oth) {
    checkInvariant();
    oth.checkInvariant();
    absoluteColumnPos = oth.absoluteColumnPos;
  }
  uint32_t absPos() const { return absoluteColumnPos; }

  inline IndentingContext(IndentingContextShared &_shared)
    : shared(_shared) {}

  inline IndentingContext(const IndentingContext &oth)
    : absoluteColumnPos(oth.absoluteColumnPos),
      tab_  (oth.tab_  ),
      flags (oth.flags ),
      shared(oth.shared) { checkInvariant(); }
};


class MultilineCommentContext {
public:
  Sm::SpacerStream::iterator begin;
  Sm::SpacerStream::iterator end;

  MultilineCommentContext(Sm::SpacerStream::iterator _begin, Sm::SpacerStream::iterator _end)
    : begin(_begin), end(_end) {}
};




class IndentingSubconstruct;

class IndentingAction : public Smart {
public:
  typedef vector<Ptr<IndentingAction> > Childs;
  typedef unsigned int Tabsize;
  typedef unsigned int Column;

  typedef void (IndentingAction::*InsertEndlAndTab)(Sm::SpacerStream &stream, SpacerStream::iterator it, unsigned int newTabsize);

  Childs childs;
  Tabsize tabsize = 0;
  IndentingAction *parent = 0;
  bool indented = false;

  static size_t getAid();
#ifdef DISABLE_INDEX_SPACERS
  size_t aid = getAid();
#endif

  virtual void offsetColumnLength(int off);

  virtual void indent(IndentingContext &ctx);
  void indentStub(IndentingContext &ctx);
  void indentSavedCopy(IndentingContext &ctx);

private:
  void indentSavedCopyBase(IndentingContext &ctx);

public:
  inline bool skipChildsIndenting(IndentingContext &ctx) const  { return ctx.skipChildsIndenting() || childs.empty(); }

  virtual ~IndentingAction() {}

  IndentingAction(Tabsize _tabsize)
    : tabsize(_tabsize) {}

  virtual Column lastIndentedCol() const { throw 999; return 0; }
  virtual Column firstIndentedCol() const { throw 999; return 0; }


  void insertEndlAndTabReal(Sm::SpacerStream &stream, SpacerStream::iterator it, unsigned int newTabsize);
  void insertEndlAndTabStub(Sm::SpacerStream &stream, SpacerStream::iterator it, unsigned int newTabsize);

  void addChild(IndentingAction *node);

  bool indentedSelfOrChilds() const;

  IndentingAction *getFirstIndentedChild();
  IndentingAction *getPrevIndented(IndentingContext &ctx,
                                   PositionInParent::reverse_iterator levelPos,
                                   bool &isMultiline);

  virtual IndentedPos getPrevIndentedPos(IndentingContext &ctx);

  virtual IndentingSubconstruct* toSelfIndentingSubconstruct() const { return 0; }
};


class IndentingContextShared {
  friend class IndentingContext;
public:
  unsigned int sourceLineSize_ = 0;
public:
  PositionInParent positionInParent;
  Sm::SpacerStream &stream;

  IndentingAction::InsertEndlAndTab insertEndlAndTab = &IndentingAction::insertEndlAndTabReal;

  IndentingContextShared(Sm::SpacerStream &_stream, unsigned int srcLineSize)
    : sourceLineSize_(srcLineSize),
      stream         (_stream    ) {}
};



class IndentingSubconstruct: public virtual IndentingAction {
public:
  SpacerStream::iterator it;
  Column col = 0;
  int taboffset = 0;

  IndentingSubconstruct(SpacerStream::iterator _it, Column _col, Tabsize _tabsize)
    : IndentingAction(_tabsize), it(_it), col(_col) {}

  void indent(IndentingContext &ctx);

  IndentingSubconstruct* toSelfIndentingSubconstruct() const { return const_cast<IndentingSubconstruct*>(this); }
  void offsetColumnLength(int off);
  virtual Column lastIndentedCol() const;
  virtual Column firstIndentedCol() const;
};



class IndentingBracket : public IndentingAction {
public:
  SpacerStream::iterator obracket;
  SpacerStream::iterator cbracket;
  Column obracketCol = 0;
  Column cbracketCol = 0;
  int taboffset = 0;

  Column absCbracketCol() { return cbracketCol - taboffset; }
  Column absObracketCol() { return obracketCol - taboffset; }

  Spacer::Cathegory splitterCathegory() const {
    Spacer::Cathegory cat    = (*obracket)->cathegory();
    return (cat == Spacer::OBracketArglist_) ? Spacer::Semicolon_ : Spacer::Comma_;
  }

  IndentingBracket(SpacerStream::iterator o, Column oCol, Tabsize _tabsize)
    : IndentingAction(_tabsize), obracket(o), obracketCol(oCol) {}

  void indent(IndentingContext &ctx);
  IndentedPos getPrevIndentedPos(IndentingContext &ctx);
  void indentArglistBraces(IndentingContext localCtx, int cbracketTab);
  bool updateCtxForCommaList(IndentingContext &localCtx);

  void offsetColumnLength(int off);
  virtual Column lastIndentedCol() const;
  virtual Column firstIndentedCol() const;
};


class TabStack {
public:
  std::stack<int> tabstack;
  std::stack<int> assignedTabs;

  int summaryTab_ = 0;

  int tabSaved    () const { return summaryTab_; }
  int fullTabSaved() const { return summaryTab_; }
  int summaryTab  () const { return summaryTab_; }

  void pushSummary() {
    assignedTabs.push(summaryTab_);
    summaryTab_ = 0;
  }

  void popSummary() {
    summaryTab_ = assignedTabs.top();
    assignedTabs.pop();
  }

  void push(unsigned int v) {
    summaryTab_ +=  v;
    tabstack.push(v);
  }
  void pop() {
    if (!tabstack.empty()) {
      summaryTab_ -= tabstack.top();
      tabstack.pop();
    }
  }

  bool empty() const { return tabstack.empty(); }
};

class Indenter {
public:
  typedef std::vector<Ptr<IndentingBracket> > OpenBracesStack;
  typedef std::vector<MultilineCommentContext> MultilineComments;

  IndentingAction indentingActions;
  OpenBracesStack  openBracesStack;
  TabStack tabstack;

  unsigned int commentDepth = 0;
  bool needIndentingLongSubconstruct = false;

  IndentedLine currentLine;
  CodestreamState &state;

  Indenter(SpacerStream &_stream, CodestreamState &s)
    : indentingActions(0),
      currentLine(_stream.begin(), _stream, commentDepth),
      state(s) {}


  void mainIndentingLoop();
  void indentMultilineComments();
  void removeCommentsIfNeed();
  void buildIndentTree();
  void executeDelayIndenting();
  void addIndentingAction(IndentingAction *node);
  void executeLastDelayIndenting();
  void multilinePostFormat(SpacerStream::iterator it, MultilineComments &ctx);
  void createSubconstructNode(int summaryTab);
  void createBracketNode();
};


void indenterTest();


}

#endif // INDENTER_H
