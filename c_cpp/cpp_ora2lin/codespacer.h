#ifndef CODESPACER_H
#define CODESPACER_H


#include <unordered_map>
#include "codestream.h"
#include "config_converter.h"

namespace Sm {

class ResolvedEntity;
class Translator;
class StatementInterface;

template <typename T>
inline string to_string(const T) { throw 999; }

template <typename T>
inline void to_string(string&, const T) { throw 999; }



#define DEF_FUN_TO_STRING(T, printfspec, digits) \
  template <>                                    \
  inline string to_string<T>(T val) {            \
    char buf[digits];                                  \
    sprintf(buf, printfspec, val);                     \
    return buf;                                        \
  }                                                    \
  \
  template <>                                          \
  inline void to_string<T>(string &dst, T val) {       \
    char buf[digits];                                  \
    sprintf(buf, printfspec, val);                     \
    dst.append(buf);                                   \
  }                                                    \




DEF_FUN_TO_STRING(short int         , "%hi" , std::numeric_limits<unsigned int      >::digits10+16)
DEF_FUN_TO_STRING(unsigned short int, "%hu" , std::numeric_limits<unsigned int      >::digits10+16)
DEF_FUN_TO_STRING(int               , "%i"  , std::numeric_limits<unsigned int      >::digits10+16)
DEF_FUN_TO_STRING(unsigned int      , "%u"  , std::numeric_limits<unsigned int      >::digits10+16)
DEF_FUN_TO_STRING(long int          , "%li" , std::numeric_limits<long int          >::digits10+16)
DEF_FUN_TO_STRING(long unsigned int , "%lu" , std::numeric_limits<long unsigned int >::digits10+16)
DEF_FUN_TO_STRING(long long int     , "%lli", std::numeric_limits<long long int     >::digits10+16)





namespace s {

class PairSpacer : public Spacer {
public:
  PairSpacer()  {}
};

class Oquote: public Spacer {
public:
  Oquote() {}
  Cathegory cathegory()     const { return Oquote_; }
  void toUnformattedString(string &dst) const { dst.push_back('"'); }
  unsigned int length() const { return 1; }
};

class Cquote: public Spacer {
public:
  Cquote() {}
  Cathegory cathegory()     const { return Cquote_; }
  void toUnformattedString(string &dst) const { dst.push_back('"'); }
  unsigned int length() const { return 1; }
};

class DisableIndenting: public Spacer {
public:
  DisableIndenting() {}
  Cathegory cathegory()     const { return DisableIndenting_; }
};

class EnableIndenting: public Spacer {
public:
  EnableIndenting() {}
  Cathegory cathegory()     const { return EnableIndenting_; }
};


class QuerySign : public Spacer {
  virtual Cathegory cathegory() const { return Empty_; }
};


class connect    : public Spacer {
public:
  UserContext *cntx;

  Cathegory cathegory() const { return Connect_; }
  connect(UserContext *cntx);

  UserContext* userContext() const { return cntx; }
  void toUnformattedString(string &dst) const {
    dst.append("USERNAME ");
    dst.append(dbConnectUname());
    dst.push_back('/');
    dst.append(dbConnectPassword());
    dst.push_back('\n');
  }
  unsigned int length() const { return dbConnectUname().size() + dbConnectPassword().size() + 11; }

  std::string dbConnectUname() const;
  std::string dbConnectPassword() const;

  bool inconsistant() const { return !cntx; }
};

class statementPointer : public Spacer {
public:
  Sm::ResolvedEntity *stmt = 0;
  Sm::CodestreamState state;
  statementPointer(Sm::ResolvedEntity *s, Sm::CodestreamState _state) : stmt(s), state(_state) {}
  Cathegory cathegory() const { return StatementPointer_; }

  Sm::s::statementPointer* toSelfStatementPointer() const { return const_cast<Sm::s::statementPointer*>(this); }
};


class executeStatementPointer : public statementPointer {
public:
  executeStatementPointer(Sm::ResolvedEntity *s, Sm::CodestreamState _state) : statementPointer(s, _state) {}
  Cathegory cathegory() const { return ExecuteStatementPointer_; }
  Sm::s::executeStatementPointer* toSelfExecuteStatementPointer() const { return const_cast<Sm::s::executeStatementPointer*>(this); }
};


class ocreate : public Spacer {
public:
  Sm::ResolvedEntity *entity = 0;

  ResolvedEntity* commandEntity() const { return entity; }

  ocreate(Sm::ResolvedEntity *ent)
    : entity(ent) {}

  Sm::CmdCat commandCmdCathegory() const {  return CmdCat::CREATE_SPACER; }
  Cathegory cathegory()     const { return OCreate_; }
  void toUnformattedString(string &dst) const;
  unsigned int length() const;
};

class CCreate : public Spacer {
public:
  CCreate() {}
  Cathegory cathegory()     const { return CCreate_; }
};


class grant: public Spacer {
public:
  typedef std::set<string> Targets;

  std::set<Privs>  privilegies;
  ResolvedEntity  *entity;
  Targets          targets;
  std::string      entName;

  string uniqueHeader() const { return "GRANT " + entName; }

  static std::string privilegieToString(Privs priv);

  grant(ResolvedEntity *ent, Privs priv, ResolvedEntity *target);
  grant(ResolvedEntity *ent, Privs priv, string target);
  grant(ResolvedEntity *ent, std::initializer_list<Privs> privs, string target);
  grant(ResolvedEntity *ent, std::initializer_list<Privs> privs, std::initializer_list<string> targets);
  grant(ResolvedEntity *ent, Privs priv, std::initializer_list<string> targets);
  grant(ResolvedEntity *ent, Privs priv, Targets targets_);
  grant(ResolvedEntity *ent, std::initializer_list<Privs> privs, Targets targets_);
  grant(string entName_, Privs priv, Targets targets_);
  grant(string entName_, Privs priv, std::initializer_list<string> targets_);

  Cathegory cathegory()     const { return GrantCmd_; }
  void toUnformattedString(string &dst) const;
  unsigned int length() const;
  s::grant *toSelfGrant() const { return const_cast<s::grant*>(this); }

  bool inconsistant() const;
  void baseToString(Sm::Codestream &str) const;
};


class OGrantCmd: public Spacer {
public:
  OGrantCmd() {}
  Cathegory cathegory()     const { return OGrantCmd_; }
};

class CGrantCmd : public Spacer {
public:
  CGrantCmd() {}
  Cathegory cathegory()     const { return CGrantCmd_; }
};

class ocmd: public Spacer {
public:
  Sm::ResolvedEntity *entity = 0;
  CmdCat cmdCathegory;
  string uniqueHeader_;

  ocmd(Sm::ResolvedEntity *ent, CmdCat cat, string _uniqueHeader = "");

  ResolvedEntity* commandEntity() const { return entity; }

  Sm::CmdCat commandCmdCathegory() const {  return cmdCathegory; }

  Cathegory cathegory()     const { return OCallCmd_; }
  string uniqueHeader() const { return uniqueHeader_; }

  s::ocmd *toSelfOcmd() const { return const_cast<ocmd*>(this); }
};

class CCmd : public Spacer {
public:
  CCmd() {}
  Cathegory cathegory()     const { return CCallCmd_; }
};


class OBracketCall: public Spacer {
public:
  OBracketCall() {}
  Cathegory cathegory()     const { return OBracketCall_; }
  void toUnformattedString(string &dst) const { dst.push_back('('); }
  unsigned int length() const { return 1; }
};

class CBracketCall: public Spacer {
public:
  CBracketCall() {}
  Cathegory cathegory()     const { return CBracketCall_; }
  void toUnformattedString(string &dst) const { dst.push_back(')'); }
  unsigned int length() const { return 1; }
};


class OBracket: public Spacer {
public:
  OBracket() {}
  Cathegory cathegory()     const { return OBracket_; }
  void toUnformattedString(string &dst) const { dst.push_back('('); }
  unsigned int length() const { return 1; }
};

class CBracket: public Spacer {
public:
  CBracket() {}
  Cathegory cathegory()     const { return CBracket_; }
  void toUnformattedString(string &dst) const { dst.push_back(')'); }
  unsigned int length() const { return 1; }
};


class OBracketInsert: public Spacer {
public:
  OBracketInsert() {}
  Cathegory cathegory()     const { return OBracketInsert_; }
  void toUnformattedString(string &dst) const { dst.push_back('('); }
  unsigned int length() const { return 1; }
};

class CBracketInsert: public Spacer {
public:
  CBracketInsert() {}
  Cathegory cathegory()     const { return CBracketInsert_; }
  void toUnformattedString(string &dst) const { dst.push_back(')'); }
  unsigned int length() const { return 1; }
};


class OBracketArglist: public Spacer {
public:
  OBracketArglist() {}
  Cathegory cathegory()     const { return OBracketArglist_; }
  void toUnformattedString(string &dst) const { dst.push_back('('); }
  unsigned int length() const { return 1; }
};


class CBracketArglist: public Spacer {
public:
  CBracketArglist() {}
  Cathegory cathegory()     const { return CBracketArglist_; }
  void toUnformattedString(string &dst) const { dst.push_back(')'); }
  unsigned int length() const { return 1; }
};


class OBracketView: public Spacer {
public:
  OBracketView() {}
  Cathegory cathegory()     const { return OBracketView_; }
  void toUnformattedString(string &dst) const { dst.push_back('('); }
  unsigned int length() const { return 1; }
};


class CBracketView: public Spacer {
public:
  CBracketView() {}
  Cathegory cathegory()     const { return CBracketView_; }
  void toUnformattedString(string &dst) const { dst.push_back(')'); }
  unsigned int length() const { return 1; }
};


class ocommalist: public Spacer {
public:
  ocommalist() {}

  Cathegory cathegory()     const { return OCommalist_; }
};

class ccommalist : public Spacer {
public:
  ccommalist() {}
  Cathegory cathegory()     const { return CCommalist_; }
};


class otabcommalist: public Spacer {
public:
  otabcommalist() {}

  Cathegory cathegory()     const { return OTabCommalist_; }
};

class ctabcommalist : public Spacer {
public:
  ctabcommalist() {}
  Cathegory cathegory()     const { return CTabCommalist_; }
};

class ocolumn: public Spacer {
public:
  ocolumn() {}

  Cathegory cathegory()     const { return OColumn_; }
};

class ccolumn : public Spacer {
public:
  ccolumn() {}
  Cathegory cathegory()     const { return CColumn_; }
};

class otablevel: public Spacer {
public:
  int offsetTabsize = 0;
  otablevel(int _offsetTabsize = 0) : offsetTabsize(_offsetTabsize) {}

  int level() const { return offsetTabsize; }

  Cathegory cathegory()     const { return OTabLevel_; }
};

class ctablevel : public Spacer {
public:
  ctablevel() {}
  Cathegory cathegory()     const { return CTabLevel_; }
};


class Subconstruct : public Spacer {
public:
  Subconstruct() {}
  Cathegory cathegory() const { return Subconstruct_; }
  Subconstruct *toSelfSubconstruct() const { return const_cast<Subconstruct*>(this); }
};

class ogroup  : public Spacer {
public:
  ogroup() {}
  Cathegory cathegory()     const { return GroupOpen_; }
};

class cgroup  : public Spacer {
public:
  cgroup() {}
  Cathegory cathegory()     const { return GroupClose_; }
};

class Name    : public Spacer {
public:
  Name() {}
  Cathegory cathegory() const { return Name_; }
  bool isName() const { return true; }

  static int getContextLength(Sm::SpacerStream::iterator &it, Sm::SpacerStream &s);
  Name* toSelfName() const { return const_cast<Name*>(this); }
};

class Stub    : public Spacer {
public:
  Stub() {}
  Cathegory cathegory() const { return Stub_; }
  bool isStub() const { return true; }
};

class tab     : public Spacer {
public:
  int tabsize_;

  void checkInvariant() const { if (tabsize_ > 65535) throw 999; }

  Cathegory cathegory() const { checkInvariant(); return Tab_; }
  int tabsize() const { checkInvariant(); return tabsize_; }
  tab(int size = -1) : tabsize_(size) { checkInvariant(); }
  tab(const tab &oth) : Spacer(), tabsize_(oth.tabsize_) { checkInvariant(); }

  void toUnformattedString(string &dst) const {
    checkInvariant();
    int tabsize = tabsize_ > 0 ? tabsize_ : 0;
    for (int i = 0; i < tabsize; ++i)
      dst.push_back(' ');
  }
  virtual int getContextLength(int tabAtBeginOfLine) {
    return tabsize_ < 0 ? tabAtBeginOfLine : tabsize_;
  }

  unsigned int length() const { checkInvariant(); return tabsize_; }

  bool isEmptyTab() const { checkInvariant(); return tabsize_ == 0; }
  tab *toSelfTab() const { checkInvariant(); return const_cast<tab*>(this); }
};





class CommaCodegen   : public Spacer {
public:
  CommaCodegen() {}

  Cathegory cathegory() const { return Comma_; }
  void toUnformattedString(string &dst) const { dst.push_back(','); dst.push_back(' '); }
  unsigned int length() const { return 2; }
};


class CommaMakestr   : public Spacer {
public:
  CommaMakestr() {}

  Cathegory cathegory() const { return CommaMakestr_; }
  void toUnformattedString(string &dst) const { dst.push_back(','); dst.push_back(' '); }
  unsigned int length() const { return 2; }
};

class comma   : public CommaCodegen {
public:
  bool *isNotFirst;

  comma() : isNotFirst(0) {}
  comma(bool *isNotFirst_) : isNotFirst(isNotFirst_) {}
};

class Semicolon : public Spacer {
public:
  Cathegory cathegory() const { return Semicolon_; }
  void toUnformattedString(string &dst) const { dst.push_back(';'); }
  unsigned int length() const { return 1; }

  bool isSemicolon() const { return true; }
};

class Endl    : public Spacer {
public:
  Endl() {}
  Cathegory cathegory() const { return Endl_; }
  void toUnformattedString(string &dst) const { dst.push_back('\n'); }
  unsigned int length() const { return 1; }

};



class StubLength : public Spacer {
public:
  unsigned int length_;
  StubLength(unsigned int l)
    : length_(l) {}
  Cathegory cathegory() const { return StubLength_; }

  void toUnformattedString(string &) const {}
  unsigned int length() const { return length_; }
};

class Comment : public Spacer {
public:
  CodestreamState::ProcedureMode mode;
  Cathegory cathegory() const { return Comment_; }
  Comment() : mode(CodestreamState::SQL) {}

  Comment(CodestreamState::ProcedureMode m)
    : mode(m) {}


  static inline void commentToUnformattedString(string &dst, CodestreamState::ProcedureMode mode) {
    if (mode == CodestreamState::SQL) {
     dst.push_back('-');
     dst.push_back('-');
    }
    else {
      dst.push_back('/');
      dst.push_back('/');
    }
  }

  void toUnformattedString(string &dst) const { commentToUnformattedString(dst, mode); }
  unsigned int length() const { return 2; }

};



class OMultiLineComment : public Spacer {
public:
  CodestreamState::ProcedureMode mode;

  OMultiLineComment() : mode(CodestreamState::SQL) {}
  OMultiLineComment(CodestreamState::ProcedureMode m)
    : mode(m) {}

  Cathegory cathegory() const { return OMultiLineComment_; }

  void toUnformattedString(string &dst) const { Comment::commentToUnformattedString(dst, mode); }
  unsigned int length() const { return 2; }
  bool isOMultilineComment() const { return true; }

  OMultiLineComment *toSelfOMultiLineComment() const { return const_cast<OMultiLineComment*>(this); }
};

class CMultiLineComment : public Spacer {
public:
  CMultiLineComment() {}
  Cathegory cathegory() const { return CMultiLineComment_; }
  void toUnformattedString(string &) const {}
  bool isCMultilineComment() const { return true; }
  s::CMultiLineComment *toSelfCMultiLineComment() const { return const_cast<CMultiLineComment*>(this); }
};


class  InlMessage : public Spacer {
  string str;
public:
  InlMessage(std::string &&s) : str(s) {}
  Cathegory cathegory()     const { return InlMessage_; }
  void toUnformattedString(string &dst) const;
  unsigned int length() const;
};


class  OInlCmd : public Spacer {
public:
  Cathegory cathegory()     const { return OInl_; }
  bool isOInl() const { return true; }
};

class  CInlCmd : public Spacer {
public:
  Cathegory      cathegory()           const { return CInl_; }
};

class  loc : public Spacer {
  cl::filelocation loc_ = cl::emptyFLocation();
public:
  loc(const cl::filelocation &pos) { loc_ = pos; }
  Cathegory cathegory() const { return Loc_; }
  bool isLoc() const { return true; }
  void toUnformattedString(string &dst) const {
    stringstream str; str << loc_;
    dst.append(str.str());
  }
  unsigned int length() const {
    stringstream str; str << loc_;
    return str.str().size();
  }

  bool beginedFrom(int line, int column) const { return loc_.loc.beginedFrom(line, column); }
};

class  iloc : public Spacer {
  cl::filelocation loc_ = cl::emptyFLocation();
public:
  iloc(const cl::filelocation &pos) { loc_ = pos; }
  Cathegory cathegory() const { return ILoc_; }
  bool isLoc() const { return true; }
  void toUnformattedString(string &dst) const {
    dst += "// ";
    stringstream str; str << loc_;
    dst += str.str();
    dst += " //";
  }
  unsigned int length() const {
    stringstream str; str << loc_;
    return str.str().size() + 6;
  }
  bool beginedFrom(int line, int column) const { return loc_.loc.beginedFrom(line, column); }
};

class TextChunk : public Spacer {
public:
  std::string text_;

  TextChunk(const std::string &text) : text_(text) {}
  Cathegory cathegory() const { return TextChunk_; }
  unsigned int length() const { return text_.length(); }

  void toUnformattedString(string &dst) const;
  TextChunk *toSelfTextChunk() const { return const_cast<TextChunk*>(this); }

};


class MultilineTextChunck : public Spacer {
public:
  std::string text_;
  unsigned int lastLineLength = 0;
  unsigned int previousAbsolutePos = 0;

  MultilineTextChunck(int _lastLineLength)
    : lastLineLength(_lastLineLength) {}

  bool isMultiline();

  Cathegory cathegory() const { return MultilineTextChunck_; }
  unsigned int length() const { return lastLineLength; }
  void toUnformattedString(string &dst) const { dst.append(text_); }

  MultilineTextChunck *toSelfMultilineTextChunk() const { return const_cast<MultilineTextChunck*>(this); }

  static int getLastLineLength(const std::string &text);
};


struct __EntityTr__ {
  Translator *obj = 0;
  __EntityTr__(Translator *tr) : obj(tr) {}
  __EntityTr__(ResolvedEntity *tr);
};




struct cdef : public __EntityTr__ {
  cdef(Translator     *tr) : __EntityTr__(tr) {}
  cdef(ResolvedEntity *tr) : __EntityTr__(tr) {}
};


struct CRef : public __EntityTr__ {
  CRef(Translator     *tr) : __EntityTr__(tr) {}
  CRef(ResolvedEntity *tr) : __EntityTr__(tr) {}
};



struct CRefId {
  Sm::Id *obj = 0;
  CRefId(Id *tr) : obj(tr) { if (!obj) throw 999; }
};

inline CRef   cref(Translator     *tr) { return CRef(tr); }
inline CRef   cref(ResolvedEntity *tr) { return CRef(tr); }
inline CRefId cref(Sm::Id *tr)         { return CRefId(tr); }

struct linref : public __EntityTr__ {
  linref(Translator     *tr) : __EntityTr__(tr) {}
  linref(ResolvedEntity *tr) : __EntityTr__(tr) {}
};


struct lindef : public __EntityTr__ {
  lindef(Translator     *tr) : __EntityTr__(tr) {}
  lindef(ResolvedEntity *tr) : __EntityTr__(tr) {}
};

struct lindecl : public __EntityTr__ {
  lindecl(Translator     *tr) : __EntityTr__(tr) {}
  lindecl(ResolvedEntity *tr) : __EntityTr__(tr) {}
};


struct sqlref : public __EntityTr__ {
  sqlref(Translator     *tr) : __EntityTr__(tr) {}
  sqlref(ResolvedEntity *tr) : __EntityTr__(tr) {}
};


struct sqldef : public __EntityTr__ {
  sqldef(Translator     *tr) : __EntityTr__(tr) {}
  sqldef(ResolvedEntity *tr) : __EntityTr__(tr) {}
};

struct sqldecl : public __EntityTr__ {
  sqldecl(Translator     *tr) : __EntityTr__(tr) {}
  sqldecl(ResolvedEntity *tr) : __EntityTr__(tr) {}
};


struct procendl {
  cl::filelocation l = cl::emptyFLocation();
  procendl(const cl::filelocation &oth) { l = oth; }
  procendl() { l.loc.initProcendl(); }
};


extern QuerySign    querySign;
extern Subconstruct subconstruct;
extern OBracketArglist obracketArglist;
extern CBracketArglist cbracketArglist;
extern OBracket   obracket;
extern CBracket   cbracket;
extern Name       name;
extern Stub       stub;
extern Endl       endl;
extern OInlCmd    oInlCmd;
extern CInlCmd    cInlCmd;
extern Semicolon  semicolon;
extern Oquote     oquote;
extern Cquote     cquote;

extern OGrantCmd ogrant;
extern CGrantCmd cgrant;

extern CCmd    ccmd;
extern CCreate ccreate;


template <typename M, typename T>
inline smart::Ptr<T> getUniqueContainerItem(std::unordered_map<M, smart::Ptr<T> > &container, M level) {
  typedef std::unordered_map<M, smart::Ptr<T> > Container;
  typename Container::iterator it = container.find(level);
  if (it != container.end())
    return it->second;
  else {
    smart::Ptr<T> copy = new T(level);
    container.insert(typename Container::value_type(level, copy));
    return copy;
  }
}



class StaticSpacers : public Smart {
public:
  typedef std::unordered_map<unsigned int, Ptr<StubLength> > StubLengthMap;
  typedef std::unordered_map<int, Ptr<tab> > TabMap;
  typedef std::unordered_map<int, Ptr<s::otablevel> > TabLevelMap;

  TabMap tabMap;
  TabLevelMap tabLevelMap;
  StubLengthMap stubLengthMap;

  Ptr<s::Subconstruct> subconstruct = new s::Subconstruct();
  Ptr<s::ocommalist> ocommalist = new s::ocommalist();
  Ptr<s::ccommalist> ccommalist = new s::ccommalist();
  Ptr<CCreate> ccreate = new CCreate();
  Ptr<CCmd> ccmd = new CCmd();
  Ptr<OGrantCmd> oGrantCmd = new OGrantCmd();
  Ptr<CGrantCmd> cGrantCmd = new CGrantCmd();
  Ptr<OBracket>        obracket = new OBracket();
  Ptr<CBracket>        cbracket = new CBracket();
  Ptr<CBracketArglist> cbracketArglist = new CBracketArglist();
  Ptr<OBracketArglist> obracketArglist = new OBracketArglist();
  Ptr<OBracketView>    obracketView = new OBracketView();
  Ptr<CBracketView>    cbracketView = new CBracketView();
  Ptr<s::OBracketCall> obracketCall = new s::OBracketCall();
  Ptr<s::CBracketCall> cbracketCall = new s::CBracketCall();

  Ptr<s::OBracketInsert> obracketInsert = new s::OBracketInsert();
  Ptr<s::CBracketInsert> cbracketInsert = new s::CBracketInsert();

  Ptr<s::ctablevel> ctabLevel    = new s::ctablevel();

  Ptr<s::otabcommalist> oTabCommaList = new s::otabcommalist();
  Ptr<s::ctabcommalist> cTabCommaList = new s::ctabcommalist();

  Ptr<Endl> endl = new Endl();
  Ptr<Name> name = new Name();
  Ptr<Stub> stub = new Stub();
  Ptr<Oquote> oquote = new Oquote();
  Ptr<Cquote> cquote = new Cquote();
  Ptr<s::ogroup> ogroup = new s::ogroup();
  Ptr<s::cgroup> cgroup = new s::cgroup();
  Ptr<DisableIndenting> disableIndenting = new s::DisableIndenting();
  Ptr<EnableIndenting> enableIndenting = new s::EnableIndenting();

  Ptr<OInlCmd> oInlCmd = new OInlCmd();
  Ptr<CInlCmd> cInlCmd = new CInlCmd();

  Ptr<s::Semicolon>    semicolon    = new s::Semicolon();
  Ptr<s::CommaCodegen> comma        = new s::CommaCodegen();
  Ptr<s::CommaMakestr> commaMakestr = new s::CommaMakestr();

  Ptr<s::Comment> commentProc = new s::Comment(CodestreamState::PROC);
  Ptr<s::Comment> commentSql  = new s::Comment(CodestreamState::SQL);
  Ptr<s::OMultiLineComment> omultilineCommentProc  = new s::OMultiLineComment(CodestreamState::PROC);
  Ptr<s::OMultiLineComment> omultilineCommentSql   = new s::OMultiLineComment(CodestreamState::SQL);


  Ptr<s::CMultiLineComment> cmultilineComment = new s::CMultiLineComment();
  Ptr<s::ocolumn> ocolumn = new s::ocolumn();
  Ptr<s::ccolumn> ccolumn = new s::ccolumn();

  Ptr<s::Comment> comment(Sm::CodestreamState::ProcedureMode m) { return (m == CodestreamState::PROC) ? commentProc : commentSql; }
  Ptr<s::Comment> comment(Sm::CodestreamState &s) { return (s.procMode() == CodestreamState::PROC) ? commentProc : commentSql; }

  Ptr<s::OMultiLineComment> omultilineComment(Sm::CodestreamState::ProcedureMode m) {
#ifndef DISABLE_INDEX_SPACERS
    return (m == CodestreamState::PROC) ? omultilineCommentProc : omultilineCommentSql;
#else
    return new s::OMultiLineComment(m);
#endif
  }

  Ptr<s::OMultiLineComment> omultilineComment(Sm::CodestreamState &s) { return omultilineComment(s.procMode()); }

  inline Ptr<s::tab> getUniqueTabSpacer(int level) { return getUniqueContainerItem(tabMap, level); }



};

extern Ptr<StaticSpacers> staticSpacers;



}





Codestream& operator<<(Codestream &os, s::connect o);

Codestream& operator<<(Codestream &os, s::ocreate o);
inline Codestream& operator<<(Codestream &os, const s::CCreate &) { return os.addSpacer(s::staticSpacers->ccreate); }

inline Codestream& operator<<(Codestream &os, s::ocmd o) { return os.addSpacer(new s::ocmd(o)); }
inline Codestream& operator<<(Codestream &os, const s::CCmd &) { return os.addSpacer(s::staticSpacers->ccmd); }

Codestream& operator<<(Codestream &os, const s::grant &o);

inline Codestream& operator<<(Codestream &os, const s::OGrantCmd &) { return os.addSpacer(s::staticSpacers->oGrantCmd); }
inline Codestream& operator<<(Codestream &os, const s::CGrantCmd &) { return os.addSpacer(s::staticSpacers->cGrantCmd); }


#ifndef DISABLE_INDEX_SPACERS

inline Codestream& operator<<(Codestream &os, const s::OBracketInsert    &) { return os.addSpacer(s::staticSpacers->obracketInsert); }
inline Codestream& operator<<(Codestream &os, const s::CBracketInsert    &) { return os.addSpacer(s::staticSpacers->cbracketInsert); }
inline Codestream& operator<<(Codestream &os, const s::CBracketArglist   &) { return os.addSpacer(s::staticSpacers->cbracketArglist); }
inline Codestream& operator<<(Codestream &os, const s::OBracketArglist   &) { return os.addSpacer(s::staticSpacers->obracketArglist); }
inline Codestream& operator<<(Codestream &os, const s::OBracketView      &) { return os.addSpacer(s::staticSpacers->obracketView); }
inline Codestream& operator<<(Codestream &os, const s::CBracketView      &) { return os.addSpacer(s::staticSpacers->cbracketView); }
inline Codestream& operator<<(Codestream &os, const s::CBracket          &) { return os.addSpacer(s::staticSpacers->cbracket); }
inline Codestream& operator<<(Codestream &os, const s::OBracket          &) { return os.addSpacer(s::staticSpacers->obracket); }
inline Codestream& operator<<(Codestream &os, const s::OBracketCall      &) { return os.addSpacer(s::staticSpacers->obracketCall); }
inline Codestream& operator<<(Codestream &os, const s::CBracketCall      &) { return os.addSpacer(s::staticSpacers->cbracketCall); }
inline Codestream& operator<<(Codestream &os, const s::otabcommalist     &) { return os.addSpacer(s::staticSpacers->oTabCommaList); }
inline Codestream& operator<<(Codestream &os, const s::ctabcommalist     &) { return os.addSpacer(s::staticSpacers->cTabCommaList); }
inline Codestream& operator<<(Codestream &os, const s::Subconstruct      &) { return os.addSpacer(s::staticSpacers->subconstruct); }
inline Codestream& operator<<(Codestream &os, const s::otablevel        &o) { return os.addSpacer(getUniqueContainerItem(s::staticSpacers->tabLevelMap, o.offsetTabsize)); }
inline Codestream& operator<<(Codestream &os, const s::ctablevel         &) { return os.addSpacer(s::staticSpacers->ctabLevel); }
inline Codestream& operator<<(Codestream &os, const s::StubLength       &o) { return os.addSpacer(getUniqueContainerItem(s::staticSpacers->stubLengthMap, o.length_)); }
inline Codestream& operator<<(Codestream &os, const s::CommaMakestr      &) { return os.addSpacer(s::staticSpacers->commaMakestr); }
inline Codestream& operator<<(Codestream &os, const s::CommaCodegen      &) { return os.addSpacer(s::staticSpacers->comma); }
inline Codestream& operator<<(Codestream &os, const s::CMultiLineComment &) { return os.addSpacer(s::staticSpacers->cmultilineComment); }
inline Codestream& operator<<(Codestream &os, const s::ocolumn           &) { return os.addSpacer(s::staticSpacers->ocolumn); }
inline Codestream& operator<<(Codestream &os, const s::ccolumn           &) { return os.addSpacer(s::staticSpacers->ccolumn); }

inline Sm::Codestream& operator<<(Codestream &os, s::comma o) {
  if (o.isNotFirst == nullptr || *o.isNotFirst)
    return os.addSpacer(s::staticSpacers->comma);
  else
    *o.isNotFirst = true;
  return os;
}

inline s::Name* getSpacerName() { return s::staticSpacers->name; }

inline Codestream& operator<<(Codestream &os, const s::Semicolon  &) { return os.addSpacer(s::staticSpacers->semicolon); }

inline Codestream& operator<<(Codestream &os, const s::Endl &) { return os.addSpacer(s::staticSpacers->endl); }
inline Codestream& operator<<(Codestream &os, const s::Name &) { return os.addSpacer(getSpacerName()); }
inline Codestream& operator<<(Codestream &os, const s::ocommalist &) { return os.addSpacer(s::staticSpacers->ocommalist); }
inline Codestream& operator<<(Codestream &os, const s::ccommalist &) { return os.addSpacer(s::staticSpacers->ccommalist); }

#else

inline s::Name* getSpacerName() { return new s::Name(); }

inline Codestream& operator<<(Codestream &os, const s::Endl &o) { return os.addSpacer(new s::Endl(o)); }
inline Codestream& operator<<(Codestream &os, const s::Name &) { return os.addSpacer(getSpacerName()); }

inline Codestream& operator<<(Codestream &os, const s::OBracketInsert &o) { return os.addSpacer(new s::OBracketInsert(o)); }
inline Codestream& operator<<(Codestream &os, const s::CBracketInsert &o) { return os.addSpacer(new s::CBracketInsert(o)); }

inline Codestream& operator<<(Codestream &os, const s::OBracketArglist &o) { return os.addSpacer(new s::OBracketArglist(o)); }
inline Codestream& operator<<(Codestream &os, const s::CBracketArglist &o) { return os.addSpacer(new s::CBracketArglist(o)); }

inline Codestream& operator<<(Codestream &os, const s::OBracketView &o) { return os.addSpacer(new s::OBracketView(o)); }
inline Codestream& operator<<(Codestream &os, const s::CBracketView &o) { return os.addSpacer(new s::CBracketView(o)); }

inline Codestream& operator<<(Codestream &os, const s::CBracket &o) { return os.addSpacer(new s::CBracket(o)); }
inline Codestream& operator<<(Codestream &os, const s::OBracket &o) { return os.addSpacer(new s::OBracket(o)); }

inline Codestream& operator<<(Codestream &os, const s::OBracketCall &o) { return os.addSpacer(new s::OBracketCall(o)); }
inline Codestream& operator<<(Codestream &os, const s::CBracketCall &o) { return os.addSpacer(new s::CBracketCall(o)); }

inline Codestream& operator<<(Codestream &os, const s::Subconstruct &o) { return os.addSpacer(new s::Subconstruct(o)); }

inline Sm::Codestream& operator<<(Codestream &os, const s::CommaCodegen& ) { return os.addSpacer(s::staticSpacers->comma); }
inline Sm::Codestream& operator<<(Codestream &os, const s::CommaMakestr& o) { return os.addSpacer(new s::CommaMakestr(o)); }

inline Sm::Codestream& operator<<(Codestream &os, s::comma o) {
  if (o.isNotFirst == nullptr || *o.isNotFirst)
    return os.addSpacer(new s::CommaCodegen(o));
  else
    *o.isNotFirst = true;
  return os;
}

inline Codestream& operator<<(Codestream &os, const s::Semicolon        &o) { return os.addSpacer(new s::Semicolon(o)); }
inline Codestream& operator<<(Codestream &os, const s::ocolumn          &o) { return os.addSpacer(new s::ocolumn(o)); }
inline Codestream& operator<<(Codestream &os, const s::ccolumn          &o) { return os.addSpacer(new s::ccolumn(o)); }
inline Codestream& operator<<(Codestream &os, const s::ocommalist       &o) { return os.addSpacer(new s::ocommalist(o)); }
inline Codestream& operator<<(Codestream &os, const s::ccommalist       &o) { return os.addSpacer(new s::ccommalist(o)); }
inline Codestream& operator<<(Codestream &os, const s::otabcommalist    &o) { return os.addSpacer(new s::otabcommalist(o)); }
inline Codestream& operator<<(Codestream &os, const s::ctabcommalist    &o) { return os.addSpacer(new s::ctabcommalist(o)); }
inline Codestream& operator<<(Codestream &os, const s::otablevel        &o) { return os.addSpacer(new s::otablevel(o)); }
inline Codestream& operator<<(Codestream &os, const s::ctablevel        &o) { return os.addSpacer(new s::ctablevel(o)); }
inline Codestream& operator<<(Codestream &os, const s::StubLength       &o) { return os.addSpacer(new s::StubLength(o)); }
inline Codestream& operator<<(Codestream &os, const s::CMultiLineComment &) { return os.addSpacer(new Sm::s::CMultiLineComment()); }


#endif


Codestream& operator<<(Codestream &os, const s::QuerySign      &);

inline Codestream& operator<<(Codestream &os, const s::Comment           &) { return os.addSpacer(s::staticSpacers->comment(os.state())); }
inline Codestream& operator<<(Codestream &os, const s::OMultiLineComment &) { return os.addSpacer(s::staticSpacers->omultilineComment(os.state())); }

inline Codestream& operator<<(Codestream &os, const s::tab &o) {
  return os.addSpacer(s::staticSpacers->getUniqueTabSpacer((o.tabsize_ < 0) ? os.indentingLevel() : o.tabsize_) );
}


inline Codestream& operator<<(Codestream &os, const s::Stub   &) { return os.addSpacer(s::staticSpacers->stub); }
inline Codestream& operator<<(Codestream &os, const s::Oquote &) { return os.addSpacer(s::staticSpacers->oquote); }
inline Codestream& operator<<(Codestream &os, const s::Cquote &) { return os.addSpacer(s::staticSpacers->cquote); }
inline Codestream& operator<<(Codestream &os, const s::ogroup &) { return os.addSpacer(s::staticSpacers->ogroup); }
inline Codestream& operator<<(Codestream &os, const s::cgroup &) { return os.addSpacer(s::staticSpacers->cgroup); }


inline Codestream& operator<<(Codestream &os, const s::DisableIndenting &) { return os.addSpacer(s::staticSpacers->disableIndenting); }
inline Codestream& operator<<(Codestream &os, const s::EnableIndenting  &) { return os.addSpacer(s::staticSpacers->enableIndenting); }
inline Codestream& operator<<(Codestream &os, const s::OInlCmd    &) { return os.addSpacer(s::staticSpacers->oInlCmd); }
inline Codestream& operator<<(Codestream &os, const s::CInlCmd    &) { return os.addSpacer(s::staticSpacers->cInlCmd); }

inline Codestream& operator<<(Codestream &os, s::executeStatementPointer o) { return os.addSpacer(new s::executeStatementPointer(o)); }
inline Codestream& operator<<(Codestream &os, s::statementPointer o) { return os.addSpacer(new s::statementPointer(o)); }
inline Codestream& operator<<(Codestream &os, s::InlMessage   o) { return os.addSpacer(new s::InlMessage  (o)); }
inline Codestream& operator<<(Codestream &os, s::iloc         o) { return os.addSpacer(new s::iloc        (o)); }

Codestream& operator<<(Codestream &os, s::loc o);

inline Codestream& operator<<(Codestream &os, const char   *str) { return os.addSpacerText(str); }
inline Codestream& operator<<(Codestream &os, const string &str) { return os.addSpacerText(str); }
inline Codestream& operator<<(Codestream &os, char            c) { return os.addSpacerText(std::string(1, c)); }
inline Codestream& operator<<(Codestream &os, unsigned char   v) { return os.addSpacerText(std::string(1, v)); }
inline Codestream& operator<<(Codestream &os, unsigned int    v) { return os.addSpacerText(Sm::to_string(v)); }
inline Codestream& operator<<(Codestream &os, int             v) { return os.addSpacerText(Sm::to_string(v)); }
inline Codestream& operator<<(Codestream &os, long            v) { return os.addSpacerText(Sm::to_string(v)); }
inline Codestream& operator<<(Codestream &os, unsigned long   v) { return os.addSpacerText(Sm::to_string(v)); }
inline Codestream& operator<<(Codestream &os, short           v) { return os.addSpacerText(Sm::to_string(v)); }
inline Codestream& operator<<(Codestream &os, unsigned short  v) { return os.addSpacerText(Sm::to_string(v)); }

inline Codestream& operator<<(Codestream &os, const skip   &s) { os.skipListFromStart(s.i); return os; }
inline Codestream& operator<<(Codestream &os, const Def    & ) { os.namesMode(CodestreamState::DEFINITION ); return os; }
inline Codestream& operator<<(Codestream &os, const Decl   & ) { os.namesMode(CodestreamState::DECLARATION); return os; }
inline Codestream& operator<<(Codestream &os, const Sql_   & ) { os.procMode (CodestreamState::SQL        ); return os; }
inline Codestream& operator<<(Codestream &os, const Proc   & ) { os.procMode (CodestreamState::PROC       ); return os; }
inline Codestream& operator<<(Codestream &os, const Ref1   & ) { os.namesMode(CodestreamState::REFERENCE  ); return os; }

struct inctab     { int val; inctab(int v)     : val(v) {} };
struct dectab     { int val; dectab(int v)     : val(v) {} };

inline Codestream& operator<<(Codestream &os, const inctab    & v) { os.incIndentingLevel(v.val); return os; }
inline Codestream& operator<<(Codestream &os, const dectab    & v) { os.decIndentingLevel(v.val); return os; }


Codestream& operator<<(Codestream &os, const s::CRef & v);
Codestream& operator<<(Codestream &os, const s::linref & v);
Codestream& operator<<(Codestream &os, const s::lindef & v);
Codestream& operator<<(Codestream &os, const s::lindecl& v);
Codestream& operator<<(Codestream &os, const s::sqlref & v);
Codestream& operator<<(Codestream &os, const s::sqldef & v);
Codestream& operator<<(Codestream &os, const s::sqldecl& v);

Codestream& operator<<(Codestream &os, const s::CRefId &v);


bool procEndlWithoutEndl(Codestream &os, const s::procendl &v);

Codestream& operator<<(Codestream &os, const s::procendl &v);

}

#define trError(str, op) \
  do { \
    std::stringstream s; \
    op; \
    std::cout << s.str() << std::endl; \
    str << " -- " << s.str() << Sm::s::endl; \
  } while (0)

#endif // CODESPACER_H
