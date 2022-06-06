#ifndef PARSERDFA_H
#define PARSERDFA_H

#if defined(_MSC_VER) && (_MSC_VER < 1400)
#pragma warning (disable : 4786)
#endif

#include <stdio.h>
#include <vector>
#include <string>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <initializer_list>
#include "smartptr.h"
#include "siter.h"

#define LEX_DFA_CHECK_SYMBOL_TO_SPEC(f, c) \
   { \
     int f1 = LexerDFA::LexerDfa::acceptIdTable.flags[c]; \
     if (f1) \
       f |= f1; \
   }


namespace LexerDFA {

struct AceptIdTable {
  bool state[0x100];
  unsigned int flags[0x100];

  int  toupper[0x100];
  bool isalpha[0x100];
  int  alphaUpper[0x100];

  AceptIdTable();
};

enum LexerDfaState  {
  MULTIGROUP      = (1 << 0),
  POSTPREVIEW     = (1 << 1),
  PREVIEW_FINALLY = (1 << 2),
  NEEDENDSTR      = (1 << 3),
  FINALLY         = (1 << 4)
};


typedef int action_t;
typedef unsigned int BranchNum;

inline std::string toString( int i ) {
  char numBuf[std::numeric_limits<int>::digits10 + 2];
  return std::string(numBuf, sprintf(numBuf, "%i", i));
}

struct Statistics {
  unsigned short                               groupsCounter;
  std::vector<BranchNum>                       branchCounters;
  std::vector<std::map<char, unsigned char> > &mappedIndices; // indexing chars by level or branch
  std::vector<std::map<char, unsigned char> >  mappedIndicesLevel ; // indexing chars by level
  std::vector<std::map<char, unsigned char> >  mappedIndicesBranch; // indexing chars by branch
  std::map<std::string, std::string>           groups;

  Statistics();

  BranchNum getNextBranchNum(unsigned int level, unsigned int size);
};


class DfaTree : public Smart {
  Statistics &statistics;

  void add(const char *str, size_t size, action_t action, unsigned short nextFlags, unsigned int level);
  bool addGroup(const char *&str, size_t &size, action_t action, unsigned short nextFlags, unsigned int level);
  void finalize(action_t action, unsigned short nextFlags);
  void getNextGroup(const char *& str, size_t &size, unsigned short &futureFlags, bool &unneed, std::set<char> &result);
  void updateBranchNum(unsigned int size, unsigned int level, Statistics &statistics);
  void addMayBeIgnorecase(char c, std::set<char> & result);

  DfaTree(char _c, Statistics &_statistics, unsigned int level, BranchNum branchNum, BranchNum parentBranchNum, unsigned short nextFlags, bool _ignorecase);

public:
  DfaTree(Statistics &_statistics);

  std::map<char, smart::Ptr<DfaTree> > nextStates;

  char           c          = 0;
  action_t       action     = 0;
  BranchNum      branchNum  = 0;
  unsigned int   level_     = 0;
  unsigned short flags      = 0;
  unsigned short groupId    = 0;
  bool           ignorecase = false;

  void addSymbol(const char *str, size_t size, action_t action, unsigned short nextFlags, unsigned int level);
  void add      (const char *str, size_t size, action_t action) { ignorecase = false; add(str, size, action, 0, 0); }
  void addTokenI(const char *str, size_t size, action_t action) { ignorecase = true ; add(str, size, action, 0, 0); }
};

struct AcceptState {
  const char  *str    = 0;
  unsigned int level  = 0;
  BranchNum    branch = UINT_MAX;
  unsigned int mapIdx = 0;

  inline AcceptState() {}

  inline AcceptState(const char * _str)
    : str(_str) {}

  inline AcceptState(const AcceptState &o)
    : str(o.str), level(o.level), branch(o.branch), mapIdx(o.mapIdx) {}

  inline AcceptState(const char *_str, unsigned int _level, BranchNum _branch, unsigned int _mapIdx)
    : str(_str), level(_level), branch(_branch), mapIdx(_mapIdx) {}

  inline void assign(const char *_str, unsigned int _level, BranchNum _branch, unsigned int _mapIdx) {
     str    = _str;
     level  = _level;
     branch = _branch;
     mapIdx = _mapIdx;
  }
};


class LexerDfa {

public:
  struct State {
    inline State() {}

    State(BranchNum _nextBranch, unsigned short _flags, unsigned short _groupId, char /*_c*/)
      : nextBranch(_nextBranch), flags(_flags), groupId(_groupId) {}

    BranchNum      nextBranch = UINT_MAX;
    unsigned short flags      = 0;
    unsigned short groupId    = 0;
  };

  class UcharTable {
    unsigned char table[0x100];
  public:
    UcharTable();

    inline void insert(unsigned int idx, unsigned char value) { table[idx] = value; }

    inline const unsigned char& operator[] (unsigned int i) const { return table[i]; }
    inline       unsigned char& operator[] (unsigned int i)       { return table[i]; }
  };


  typedef std::initializer_list<std::pair<std::string, action_t>    > Keyword2TokenInitializerList;
  typedef std::initializer_list<std::pair<std::string, std::string> > RegexGroupInitializerList;

  typedef std::map<std::string, action_t   > Keyword2TokenMap;
  typedef std::map<std::string, std::string> RegexGroupMap;

  typedef std::map<action_t, std::string> Token2KeywordMap;

  typedef std::vector<AcceptState> AcceptStack;

  // [<level>][<branch>][<mappedCharIndex>]
  typedef std::vector<std::vector<std::vector<State> > > TransitionCube;


  static AceptIdTable acceptIdTable;

  // action by index <level><branch>
  std::vector<std::vector<std::vector<action_t> > > finalStates;

  mutable AcceptStack fullAcceptStack;
  mutable AcceptStack previewAcceptStack;

  // [<level>][0x100]
  std::vector<UcharTable> mappedCharTable;

  /// Таблица переходов конечного автомата
  TransitionCube transitionCube;

  /// Вспомогательная таблица токенов, для того, чтобы можно было восстановить ключевое слово по токену
  Token2KeywordMap token2keyword;

  LexerDfa(Keyword2TokenInitializerList keyword2TokenMap,
           RegexGroupInitializerList groups = RegexGroupInitializerList());
  LexerDfa(const Keyword2TokenMap &keyword2TokenMap, RegexGroupMap groups = RegexGroupMap());


  void build(Statistics &stat, DfaTree &tree);
  void printDfaCharTable();
  void printTransitionLay(unsigned int level = 0);

  size_t size(bool needPrint = false) const;
  ~LexerDfa();
private:
  void makeTransition(DfaTree &tree, unsigned int level = 0);
};

}


#endif // PARSERDFA_H
