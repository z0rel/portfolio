#include <ios>
#include <iomanip>
#include <unordered_set>
#include "config_converter.h"
#include "semantic_flags.h"
#include "codespacer.h"
#include "lwriter.h"
#include "syntaxer_context.h"
#include "model_context.h"
#include "semantic_id.h"
#include "codegenerator.h"
#include "sorted_statistic.h"
#include "codestream_indenter.h"
#include "semantic_id_lists.h"

extern SyntaxerContext syntaxerContext;

Sm::Codestream::OutGrantees Sm::Codestream::outGrantees;

unsigned int debugSid = 0xFFFFFFFF;

std::set<string> Sm::Codestream::granteeClauses;

size_t Sm::Spacer::spacerId() {
  static const unordered_set<unsigned int> debugSids = {
    12867711,
    12867712
//     debugSid
  };
  static size_t x = 0;
  ++x;

  if (debugSids.count(x))
    cout << "";
  return x;
}


void Sm::s::InlMessage::toUnformattedString(string &dst) const {
  if (str.empty())
    return;
  dst += "! -- ";
  dst += str;
  dst.push_back('\n');
}

unsigned int Sm::s::InlMessage::length() const { return str.empty() ? 0 : 6 + str.size(); }



std::string Sm::s::connect::dbConnectUname() const {
  const Ptr<Id> &uname = cntx->getName();
  if (uname && !uname->toQString().empty())
    return uname->toQString();
  else
    return "SYSTEM";
}

std::string Sm::s::connect::dbConnectPassword() const {
  if (cntx->password)
    return cntx->password->toQString();
  else
    return "MANAGER";
}

Sm::s::connect::connect(UserContext *c)
  : cntx(c) {}

Sm::Codestream& Sm::operator<<(Codestream &os, s::connect o) {
  if (o.inconsistant())
    return os;

  os.state().currentUser_ = o.cntx;
  return os.addSpacer(new s::connect(o));
}

void Sm::s::ocreate::toUnformattedString(string &dst) const { dst.append(syntaxerContext.createStatement); }
unsigned int Sm::s::ocreate::length() const { return syntaxerContext.createStatement.size(); }

bool Sm::s::grant::inconsistant() const {
  if (!entity && !entName.empty())
    return false;
  if (entity == 0 || privilegies.empty() || targets.empty())
    return true;
  switch (entity->ddlCathegory()) {
    case ResolvedEntity::FactoringItem_:
      return true;
    default:
      return false;
  }

  return false;
}


bool Sm::s::MultilineTextChunck::isMultiline() {
  sAssert(text_.size() < lastLineLength);
  return lastLineLength != text_.size();
}




string Sm::s::grant::privilegieToString(Privs priv) {
  switch (priv) {
    case Privs::EXECUTE:    return "EXECUTE";
    case Privs::SELECT:     return "SELECT";
    case Privs::UPDATE:     return "UPDATE";
    case Privs::REFERENCES: return "REFERENCES";
    case Privs::INDEX:      return "INDEX";
    case Privs::DELETE:     return "DELETE";
    case Privs::INSERT:     return "INSERT";
  }
  return "";
}

Sm::s::grant::grant(ResolvedEntity *ent, Privs priv, ResolvedEntity *target)
  : entity(ent)
{
  privilegies.insert(priv);
  Codestream str;
  target->linterReference(str);
  targets.insert(str.str());
}

Sm::s::grant::grant(Sm::ResolvedEntity *ent, Sm::Privs priv, string target)
  : entity(ent)
{
  privilegies.insert(priv);
  targets.insert(target);
}

Sm::s::grant::grant(ResolvedEntity *ent, std::initializer_list<Privs> priv, string target)
  : privilegies(priv), entity(ent)
{
  targets.insert(target);
}

Sm::s::grant::grant(ResolvedEntity *ent, std::initializer_list<Privs> priv, std::initializer_list<string> target)
  : privilegies(priv), entity(ent), targets(target) {}

Sm::s::grant::grant(ResolvedEntity *ent, Privs priv, std::initializer_list<string> target)
  : entity(ent), targets(target)
{
  privilegies.insert(priv);
}

Sm::s::grant::grant(string entName_, Privs priv, std::initializer_list<string> target)
  : entity(NULL), targets(target), entName(entName_)
{
  privilegies.insert(priv);
}

Sm::s::grant::grant(ResolvedEntity *ent, Privs priv, Targets targets_)
  : entity(ent), targets(targets_)
{
  privilegies.insert(priv);
}

Sm::s::grant::grant(ResolvedEntity *ent, std::initializer_list<Privs> priv, Targets targets_)
  : privilegies(priv), entity(ent), targets(targets_) {}

Sm::s::grant::grant(string entName_, Privs priv, Targets targets_)
  : entity(NULL), targets(targets_), entName(entName_)
{
  privilegies.insert(priv);
}

void Sm::s::grant::baseToString(Sm::Codestream &str) const
{
  str.procMode(CodestreamState::SQL);
  str << "GRANT ";
  bool isNotFirst = false;
  for (Sm::Privs p : privilegies)
    str << s::comma(&isNotFirst) << s::grant::privilegieToString(p) << " ";
  if (entity && entity->isTable())
    str << "ON TABLE ";
  else
    str << "ON ";
  if (entity)
    entity->linterReference(str);
  else
    str << entName;
  str << " TO ";
  isNotFirst = false;

  sAssert(targets.empty());

  Targets::const_iterator it = targets.begin();
  // вывод запятой как текст, а не как s:comma для того, чтобы не индентить гранты -
  // чтобы они выводились в одну строку и можно было бы сортировать/удалять внешними средствами типа grep-а
  str << *it;
  for (++it; it != targets.end(); ++it)
    str << ", " << *it;
}

void Sm::s::grant::toUnformattedString(string &dst) const {
  Codestream str;
  str << s::ogrant;
  baseToString(str);
  str << s::semicolon << s::endl << s::cgrant;
  dst.append(str.str());
}

unsigned int Sm::s::grant::length() const {
  string s;
  toUnformattedString(s);
  return s.size();
}


Sm::Codestream& Sm::operator<<(Codestream &str, const s::grant &o) {
  if (!Codestream::mainStream || o.inconsistant() || !syntaxerContext.model->codegenerationStarted)
    return str;

  bool alreadyOutput = false;
  if (o.entity) {
    for (Sm::Privs p : o.privilegies) {
      pair<Sm::Codestream::OutGrantees::iterator, bool> privIt =
          Sm::Codestream::outGrantees.insert(make_pair(p, Sm::Codestream::GrantTargets()));
      pair<Sm::Codestream::GrantTargets::iterator, bool> entIt =
          privIt.first->second.insert(make_pair(o.entity, (s::grant*)0));
      if (entIt.second) {
        entIt.first->second = new s::grant(o);
        if (!alreadyOutput) {
          Codestream::mainStream->addSpacer(entIt.first->second.object());
          alreadyOutput = true;
        }
      }
      else
        for (const string &trgt : o.targets)
          entIt.first->second->targets.insert(trgt);
    }
  }
  else
    Codestream::mainStream->addSpacer(new s::grant(o));
  return str;

}

// Определения статических спейсеров
namespace Sm {

namespace s {
  QuerySign        querySign;
  OBracketArglist  obracketArglist;
  CBracketArglist  cbracketArglist;
  OBracket         obracket;
  CBracket         cbracket;
  Name             name;
  Stub             stub;
  comma            comma;
  Endl             endl;
  OInlCmd          oInlCmd;
  CInlCmd          cInlCmd;
  Semicolon        semicolon;
  Oquote           oquote;
  Cquote           cquote;
  Subconstruct     subconstruct;

  OGrantCmd ogrant;
  CGrantCmd cgrant;
  CCmd    ccmd;
  CCreate ccreate;

  Def     def ;
  DefR    defR;
  Decl    decl;
  Ref1    ref;
  Sql_    sql ;
  Proc    proc;
}

int __longLineLimit__ = 110;


Sm::s::ocmd::ocmd(Sm::ResolvedEntity *ent, Sm::CmdCat cat, string _uniqueHeader)
  : entity(ent), cmdCathegory(cat), uniqueHeader_(_uniqueHeader) {}

Sm::Codestream& operator<<(Sm::Codestream &os, Sm::s::ocreate o) {
  sAssert(!o.entity);
  os.addSpacer(new s::ocreate(o));
  return os;
}

Codestream& operator<<(Codestream &os, const s::QuerySign&) {
  int n = os.state().dynamicSqlDeep_;

  if (os.state().isDynamicSignOutput_ && n > 1)  {
    os << "?";
    return os;
  }


  if (n > 1) {
    --n;
    for (int i = 0; i < n; ++i)
      os << "\\\\";
  }
  os << "?";
  return os;
}

}

void Sm::s::TextChunk::toUnformattedString(string &dst) const {
#ifdef DISABLE_INDEX_SPACERS
  if (sid == debugSid)
    cout << "";
#endif
  dst.append(text_);
}



Sm::SkipNameIters::SkipNameIters()
 : cathegories({
    Empty_,
    OColumn_, CColumn_,
    OCommalist_, CCommalist_,
    OTabCommalist_, CTabCommalist_,
    OTabLevel_, CTabLevel_,

    Name_,
    StubLength_,
    GroupOpen_, GroupClose_,
    Stub_,

    Subconstruct_,
    DisableIndenting_, EnableIndenting_,
  }) {}


Sm::SpacerStream::iterator Sm::SkipNameIters::skipNext(Sm::SpacerStream::iterator it, const Sm::SpacerStream::iterator &endIt) const {
  while (it != endIt) {
    if (cathegories.count((*it)->cathegory()))
      ++it;
    else
      return it;
  }
  return it;
}


Sm::SpacerStream::iterator Sm::SkipNameIters::skipPrev(Sm::SpacerStream::iterator it, const Sm::SpacerStream::iterator &begIt) const {
  while (it != begIt) {
    if (cathegories.count((*it)->cathegory()))
      --it;
    else
      return it;
  }
  return it;
}


Sm::SpacerStream::iterator Sm::SkipNameIters::previousSignificantSpacer(Sm::Codestream &str)
{
  static const SkipNameIters nameIters;
  SpacerStream &s = str.activespacer();
  sAssert(s.empty());
  SpacerStream::iterator it = nameIters.skipPrev(prev(s.end()), s.begin());

  return it;
}

bool Sm::SkipNameIters::previousSignificantSpacerIsEndl(Sm::Codestream &str) {
  return (*previousSignificantSpacer(str))->cathegory() == Spacer::Endl_;
}

//static Sm::SpacerStream::iterator skipNext(Sm::SpacerStream::iterator it)


int Sm::s::Name::getContextLength(Sm::SpacerStream::iterator &it, Sm::SpacerStream &s) {
  static const SkipNameIters nameIters;
  Sm::SpacerStream::iterator prevIt = nameIters.skipPrev(it, s.begin());

  Sm::Spacer::Cathegory prevCat = (*prevIt)->cathegory();
  switch (prevCat) {
    case Sm::Spacer::TextChunk_:
    case Sm::Spacer::Comment_:
    case Sm::Spacer::Comma_: // после запятой - ставить пробел
    case Sm::Spacer::CMultiLineComment_:
      return 1;
    case Sm::Spacer::Tab_:
      return 0;
    default: break;
  };

  Sm::SpacerStream::iterator nextIt  = nameIters.skipNext(it, s.end());
  Sm::Spacer::Cathegory      nextCat;
  if (nextIt == s.end())
    nextCat = Sm::Spacer::Empty_;
  else
    nextCat = (*nextIt)->cathegory();

  switch (nextCat) {
    // перед запятой - пробела не ставить
    case Sm::Spacer::TextChunk_:
    case Sm::Spacer::Comment_:
    case Sm::Spacer::OMultiLineComment_:
      return 1;
    default: break;
  };


  static const std::set<Sm::Spacer::Cathegory> obracketCats = {
    Sm::Spacer::OBracketArglist_,
    Sm::Spacer::OBracketCall_,
    Sm::Spacer::OBracketView_,
    Sm::Spacer::OBracket_,
    Sm::Spacer::OBracketInsert_,
    Sm::Spacer::Loc_,
    Sm::Spacer::ILoc_,
    Sm::Spacer::TextChunk_
  };

  static const std::set<Sm::Spacer::Cathegory> cbracketCats = {
    Sm::Spacer::CBracketArglist_,
    Sm::Spacer::CBracketCall_,
    Sm::Spacer::CBracketView_,
    Sm::Spacer::CBracket_,
    Sm::Spacer::CBracketInsert_
  };

  if (cbracketCats.count(prevCat) && obracketCats.count(nextCat))
    return 1;

  return 0;
}


bool Sm::procEndlWithoutEndl(Codestream &os, const s::procendl &v)
{
  static const SkipNameIters nameIters;

  SpacerStream &s = os.activespacer();
  bool needSemicolon = true;
  if (!s.empty()) {
    SpacerStream::iterator it = s.end();
    --it;
    Sm::SpacerStream::iterator prevIt = nameIters.skipPrev(it, s.begin());
    if ((*prevIt)->cathegory() == Spacer::Endl_) // не выводить точку с запятой для тех операторов, которые завершаются концом строки
      needSemicolon = false;
  }

  if (needSemicolon)
    os << s::semicolon << s::name;
  if (v.l.loc.begin.column != std::numeric_limits<unsigned int>::max())
    os << s::loc(v.l);
  os.joinSuffixes();

  return needSemicolon;
}

Sm::Codestream& Sm::operator<<(Codestream &os, const s::procendl &v) {
  if (procEndlWithoutEndl(os, v))
    os << s::endl;
  return os;
}

static void addTextChunk(string &strdst, Sm::SpacerStream::iterator &it, Sm::SpacerStream &s, bool isNotInl) {
  static const Sm::PairSpacerCathegories pairCathegories;
  Sm::Spacer *sIt = *it;
  if (isNotInl && sIt->isOInl()) {
    Sm::Spacer::Cathegory beginCat = sIt->cathegory();
    Sm::Spacer::Cathegory endCat   = pairCathegories.pairCathegories[sIt->cathegory()];
    int deep = 0;
    Sm::Spacer::Cathegory cat;
    for (++it; it != s.end() && ((cat = (*it)->cathegory()) != endCat || deep); ++it) {
      Sm::Spacer::Cathegory cat = (*it)->cathegory();
      if (cat == beginCat)
        ++deep;
      else if (cat == endCat)
        --deep;
    }
  }
  else {
#ifdef DISABLE_INDEX_SPACERS
    static const string spacers_log_txt = OutputCodeFile::temporaryPath(syntaxerContext.temporarySpacersFile);
#endif
    Sm::Spacer::Cathegory cat = sIt->cathegory();
    switch (cat) {
      case Sm::Spacer::Name_: {
#ifdef DISABLE_INDEX_SPACERS
        static string tmp1;
        tmp1.clear();
        tmp1 = "<name> ";
        Sm::to_string(tmp1, sIt->sid);
        tmp1.push_back(' ');
        OutputCodeFileChunck::storeCode(spacers_log_txt, tmp1 + " ", /*startEndl = */ false);
        if (sIt->sid == 6148006)
          cout << "";
#endif

        if (Sm::s::Name::getContextLength(it, s))
          strdst.push_back(' ');
      } break;
      case Sm::Spacer::CMultiLineComment_:
        break;
      default: {
#ifdef DISABLE_INDEX_SPACERS
        static string tmp;
        tmp.clear();
        sIt->toUnformattedString(tmp);
        tmp.push_back(' ');
        Sm::to_string(tmp, sIt->sid);
        tmp.push_back(' ');
        OutputCodeFileChunck::storeCode(spacers_log_txt, tmp, /*startEndl = */ false);
#endif
        sIt->toUnformattedString(strdst);
        break;
      }
    }
  }
}



/// Добавить текстовый спейсер - для представления кусков текста в списке спейсеров
Sm::Codestream& Sm::Codestream::addSpacerText(const std::string &str) {
  if (activespacer().empty())
    activespacer().push_back(new s::TextChunk(str));
  else {
    Sm::SpacerStream::iterator it = activespacer().end();
    --it;
    if (Sm::s::TextChunk *sp = (*it)->toSelfTextChunk())
      sp->text_.append(str);
    else
      activespacer().push_back(new s::TextChunk(str));
  }

  return *this;
}

void Sm::CodestreamStackItem::join(CodestreamStackItem &oth) {
  if (oth.spacers.empty())
    return;
  spacers.splice(spacers.end(), oth.spacers);
  oth.clear();
}

void Sm::CodestreamStackItem::joinPreitem(CodestreamStackItem &oth) {
  oth.join(*this);
  this->swap(oth);
}

void Sm::CodestreamStackItem::swap(CodestreamStackItem &oth) { spacers.swap(oth.spacers); }

void Sm::Codestream::levelPush() {
  codestreamStack.push_back(CodestreamStack());
  currentLevel = &(codestreamStack.back());
  activateActions();
}

void Sm::Codestream::preactionsToLevelUpAsPreitem() {
  if (codestreamStack.size() > 1) {
    std::vector<CodestreamStack>::iterator previousLevel = codestreamStack.end();
    previousLevel -= 2;
    previousLevel->preactions.joinPreitem(currentLevel->preactions);
  }

}


void Sm::Codestream::levelPop() {
  if (codestreamStack.size() <= 1)
    return;
  std::vector<CodestreamStack>::iterator poppedItem    = --(codestreamStack.end());
  std::vector<CodestreamStack>::iterator activatedItem = poppedItem; --activatedItem;
  activatedItem->join(*poppedItem);
  codestreamStack.pop_back();
  currentLevel = &(codestreamStack.back());
  activateActions();
}


void Sm::Codestream::indentTextStream() {
//  return;
  Indenter ctx(activespacer(), this->state());
  ctx.needIndentingLongSubconstruct = state_.indentingLongConstruct_;
  ctx.mainIndentingLoop();
  ctx.indentMultilineComments();
  ctx.removeCommentsIfNeed();
}


Sm::Codestream::Codestream()
  : codestreamStack   (1, CodestreamStack()),
    currentLevel      (&*(codestreamStack.begin())),
    activespacer_     (&(currentLevel->actions.spacers)) {}

Sm::Codestream::~Codestream() {
  if (mainStream == this)
    mainStream = 0;
}

Sm::Codestream::Codestream(const CodestreamState &s)
  : Codestream() { state_ = s; }


void Sm::Codestream::deleteLocations() {
  SpacerStream &s = activespacer();
  for (SpacerStream::iterator it = s.begin(); it != s.end(); ) {
    if ((*it)->cathegory() == Spacer::OMultiLineComment_) {
      SpacerStream::iterator nIt = next(it);
      if (nIt != s.end() && (*nIt)->cathegory() == Spacer::TextChunk_) {
        nIt = next(nIt);
        if (nIt != s.end() && (*nIt)->cathegory() == Spacer::Loc_) {
          for (int i = 0; i < 5; ++i)
            it = s.erase(it);
          continue;
        }
      }
    }
    if ((*it)->isLoc())
      it = s.erase(it);
    else
      ++it;
  }
}


void Sm::Codestream::collectSpacerToStr(
    SpacerStream::iterator &it,
    SpacerStream &s,
    string &strdst,
    UserContext *&currentUser)
{
  switch ((*it)->cathegory()) {
    case Spacer::Connect_: {
      SpacerStream::iterator nextIt = it;
      ++nextIt;
      if (nextIt == s.end())
        break;
      if ((*nextIt)->cathegory() == Spacer::Connect_)
        break;
      if (currentUser != (*it)->userContext()) {
        currentUser = (*it)->userContext();
        addTextChunk(strdst, it, s, state_.isNotInl_);
      }
      break;
    }
    default:
      addTextChunk(strdst, it, s, state_.isNotInl_);
      break;
  }
}

std::string Sm::Codestream::str() {

  // Добавление переносов и групп
  string strdst;
  indentTextStream();
  state_.isNotInl_ = !state_.inl();
  SpacerStream &s = activespacer();
  UserContext *currentUser = syntaxerContext.model->getSystemUser().object();

  SpacerStream::iterator endIt = s.end();
  for (SpacerStream::iterator it = s.begin(); it != endIt; ++it)
    collectSpacerToStr(it, s, strdst, currentUser);

  return strdst;
}

void Sm::Codestream::store(const string &filename, bool skipIfEpmpty) {
  if (syntaxerContext.model->modelActions.directCreateInDB)
    directStoreInDB();
  else {

    // Добавление переносов и групп
    string strdst;
    indentTextStream();
    state_.isNotInl_ = !state_.inl();
    SpacerStream &s = activespacer();
    UserContext *currentUser = syntaxerContext.model->getSystemUser().object();

    SpacerStream::iterator endIt = s.end();
    for (SpacerStream::iterator it = s.begin(); it != endIt; ++it) {
      collectSpacerToStr(it, s, strdst, currentUser);
      if (skipIfEpmpty && strdst.empty())
        continue;
      OutputCodeFileChunck::storeCode(filename, strdst, /*startEndl = */ false);
      strdst.clear();
    }
    OutputCodeFileChunck::flush();
  }
}

Sm::Codestream &Sm::Codestream::addSpacer(Sm::Spacer *sp) {
  activespacer().push_back(sp);
  return *this;
}




string Sm::Codestream::strWithoutEndl() {
  string s = str();
  std::replace(s.begin(), s.end(), '\n', ' ');
  std::replace(s.begin(), s.end(), '\r', ' ');
  std::replace(s.begin(), s.end(), '\t', ' ');
  return s;
}




namespace Sm {

  Codestream& operator<<(Codestream &os, s::loc o) {
    os << s::OMultiLineComment() << s::name;
    os.addSpacer(new s::loc(o));
    os << s::name << s::CMultiLineComment();
    return os;
  }

namespace s {



__EntityTr__::__EntityTr__(ResolvedEntity *tr)
  : obj(static_cast<ResolvedEntity*>(tr)) {}
}


Codestream& operator<<(Codestream &os, const s::cdef & v) {
  CodestreamState::NamesMode oldmode = os.namesMode();
  os.namesMode(CodestreamState::DEFINITION);
  if (os.isProc())
    v.obj->linterDefinition(os);
  else
    v.obj->sqlDefinition(os);
  os.namesMode(oldmode);
  return os;
}

Codestream& operator<<(Codestream &os, const s::CRef & v) {
  CodestreamState::NamesMode oldmode = os.namesMode();
  os.namesMode(CodestreamState::REFERENCE);
  if (os.isProc())
    v.obj->linterReference(os);
  else
    v.obj->sqlReference(os);
  os.namesMode(oldmode);
  return os;
}

Codestream& operator<<(Codestream &os, const s::CRefId &v) {
  Ptr<Id> idRef = v.obj;
  bool isNotFirst = false;
  ResolvedEntity::ScopedEntities prevCat = ResolvedEntity::EMPTY__;
  bool userAlreadyOutput = false;
  translateIdReference(os, idRef, isNotFirst, prevCat, userAlreadyOutput);
  return os;
}


Codestream& operator<<(Codestream &os, const s::linref & v) { v.obj->linterReference(os);  return os; }
Codestream& operator<<(Codestream &os, const s::lindef & v) { v.obj->linterDefinition(os); return os; }
Codestream& operator<<(Codestream &os, const s::lindecl& v) { v.obj->linterDefinition(os); return os; }

Codestream& operator<<(Codestream &os, const s::sqlref & v) { v.obj->sqlReference(os);     return os; }
Codestream& operator<<(Codestream &os, const s::sqldef & v) { v.obj->sqlDefinition(os);    return os; }
Codestream& operator<<(Codestream &os, const s::sqldecl& v) { v.obj->sqlDefinition(os);    return os; }


}


struct LCommandContext {
  typedef Sm::SpacerStream::iterator iterator;
  typedef Sm::Spacer::Cathegory      Cathegory;
  typedef Sm::SpacerStream           SpacerStream;


  iterator            beginIt;
  Cathegory           cat;
  SpacerStream       &s;
  const string       &commandPrefix;
  bool                needLinCmd;
  Sm::CmdCat          cmdCat = Sm::CmdCat::EMPTY;
  Sm::ResolvedEntity *entity = 0;
  string              uniqueHeader;
  string             &query;

  LCommandContext(string       &_query,
                  iterator     &_it,
                  Cathegory    _cat,
                  Sm::CmdCat   _commandCathegory,
                  SpacerStream &_s,
                  const string &_commandPrefix,
                  bool         _needLinCmd,
                  std::string  _uniqueHeader)
  : beginIt      (_it),
    cat          (_cat),
    s            (_s),
    commandPrefix(_commandPrefix),
    needLinCmd   (_needLinCmd),
    cmdCat       (_commandCathegory),
    uniqueHeader (_uniqueHeader),
    query        (_query)

  { entity = (*beginIt)->commandEntity(); }
};


struct StoreInDBContext {
  /* typedef-ы статистики ошибок */
//  typedef std::set<Sm::ResolvedEntity*> StoreInDBContext::*EntitySet;
  typedef std::set<Sm::ResolvedEntity*> *EntitySet;
  typedef std::set<Sm::ResolvedEntity*> CreatedEntityContainer;
  typedef map<Sm::CmdCat, pair<string, pair<bool /*has setted unique id */, pair<EntitySet, EntitySet> > > > OCmdAssocData;

  /* подключения к базе по call-интерфейсу */
  typedef std::map<std::string, std::pair<UserContext*, LinterWriter*> > LWriterMap;
  typedef std::pair<UserContext*, LinterWriter*> CurrentConnection;

  size_t executedCount            = 0;
  int    previousPercent          = 0;
  int    column                   = 0;
  time_t begin                    = 0;
  size_t commandsCount            = 0;
  time_t oldTimeUsed              = 0;
  double totalSpeed               = 0;
  int    totalSpeedCount          = 0;
  double timeTotal                = 0;
  int    numDigitsOfCommandsCount = 1;

  CurrentConnection currentConnection;
  LinterWriter     *systemLWriter = 0;

  LWriterMap lwriterMap;

  Sm::SpacerStream  &s;

  bool isNotInl = false;

  // static - т.к. создание сущностей может происходить на нескольких этапах (инициализаторы и кодогенерация) и важен общий показатель
  static CreatedEntityContainer allProcedures;
  static CreatedEntityContainer allViews;
  static CreatedEntityContainer allTriggers;
  static CreatedEntityContainer createdProcedures;
  static CreatedEntityContainer createdViews;
  static CreatedEntityContainer createdTriggers;

  StoreInDBContext(Sm::SpacerStream &activeSpacer, bool _isNotInl)
    : s(activeSpacer), isNotInl(_isNotInl) {}

  void startTimeCount() { time(&begin); }
  void createLWriter(const Sm::Spacer &sp, bool isSystemLwriter = false);

  void createSystemLWriter();

  /// Подсчет количества команд в потоке
  void countOfCommands();
  /// Вывод прогресса - сколько процентов сделано, и сколько времени осталось
  void outProgress();

  void doStorageLoop();
  void executeCollectedCommand(const LCommandContext &ctx, Sm::SpacerStream::iterator &endIt);

  /* обработка статистики ошибок */
  void setUniqueHeader(const std::string &uniqueHeader, Sm::StatisticNode &statNode, const LCommandContext &ctx);
  void addToStatistics(const LCommandContext &ctx, Sm::StatisticNode &statNode, bool operationResult);

  template <Sm::Spacer::Cathegory closeCat, Sm::Spacer::Cathegory errCat, Sm::Spacer::Cathegory errCat2, bool needCheckToUniqueGrantee>
  void collectSqlCommand(Sm::SpacerStream::iterator &it);


  void clearLWriterMap();
  ~StoreInDBContext() { clearLWriterMap(); }
};

StoreInDBContext::CreatedEntityContainer StoreInDBContext::allProcedures;
StoreInDBContext::CreatedEntityContainer StoreInDBContext::allViews;
StoreInDBContext::CreatedEntityContainer StoreInDBContext::allTriggers;
StoreInDBContext::CreatedEntityContainer StoreInDBContext::createdProcedures;
StoreInDBContext::CreatedEntityContainer StoreInDBContext::createdViews;
StoreInDBContext::CreatedEntityContainer StoreInDBContext::createdTriggers;



void Sm::Codestream::directStoreInDB() {

  // Добавление переносов и групп
  indentTextStream();

  state_.isNotInl_ = !state_.inl();
  StoreInDBContext storeCtx(activespacer(), state_.isNotInl_);

  storeCtx.createSystemLWriter();
  storeCtx.countOfCommands();
  storeCtx.startTimeCount();
  cerr << endl;
  storeCtx.doStorageLoop();
  storeCtx.clearLWriterMap();

  static const auto outResult = [](size_t created, size_t total) -> std::string {
    stringstream s;
    s << created << "/" << total << " (" << (total ? ((100 * created) / total) : 0) << "%)";
    return s.str();
  };

  cout << endl << endl
       << "Created procedures " << outResult(StoreInDBContext::createdProcedures.size(), StoreInDBContext::allProcedures.size()) << endl
       << "Created views      " << outResult(StoreInDBContext::createdViews     .size(), StoreInDBContext::allViews     .size()) << endl
       << "Created triggers   " << outResult(StoreInDBContext::createdTriggers  .size(), StoreInDBContext::allTriggers  .size()) << endl;
}


void StoreInDBContext::countOfCommands() {
  using namespace Sm;
  for (Sm::SpacerStream::iterator it = s.begin(); it != s.end(); ++it) {
    Sm::Spacer::Cathegory cat = (*it)->cathegory();
    switch (cat) {
      case Sm::Spacer::OCreate_:
      case Sm::Spacer::OCallCmd_:
      case Sm::Spacer::GrantCmd_:
        ++commandsCount;
      default:
        break;
    }
  }

  int x = commandsCount;
  while ( x /= 10 )
    ++numDigitsOfCommandsCount;
}

void StoreInDBContext::outProgress() {
  using namespace Sm;
  ++executedCount;
  double progress = (100.0 * executedCount) / commandsCount;
  time_t currentTime;
  time(&currentTime);
  time_t timeUsed = currentTime - begin;

  if (executedCount && timeUsed && timeUsed != oldTimeUsed) {
    totalSpeed += double(executedCount) / timeUsed;
    ++totalSpeedCount;
    timeTotal = double(commandsCount) / (totalSpeed/totalSpeedCount);
    oldTimeUsed = timeUsed;
  }

  int iprogress = progress;
  if (iprogress > previousPercent) {
    //      double timeLeft = (100.0 * timeUsed) / progress - timeUsed;
    double timeLeft = timeTotal - timeUsed;

    if (!column)
      cerr << "---";
    cerr << " [" << setw(3) << iprogress << "%] "
         << right << setw(numDigitsOfCommandsCount)
         << executedCount
         << "/" << commandsCount << " "
         << fixed << setprecision(2) <<
            timeUsed << "s/" << timeLeft << "s";
    ++column;
    if (column > 2) {
      cerr << endl;
      column = 0;
    }
    previousPercent = iprogress;
  }
}


template <Sm::Spacer::Cathegory closeCat, Sm::Spacer::Cathegory errCat, Sm::Spacer::Cathegory errCat2, bool needCheckToUniqueGrantee>
void StoreInDBContext::collectSqlCommand(Sm::SpacerStream::iterator &it) {
  using namespace Sm;

  string strdst;
  (*it)->toUnformattedString(strdst);

  LCommandContext ctx(strdst,
                      it,
                      (*it)->cathegory(),
                      (*it)->commandCmdCathegory(),
                      s,
                      strdst,
                      true,
                      (*it)->uniqueHeader()
                     );

  size_t distance = 1;

  Sm::SpacerStream::iterator endIt = ctx.s.end();
  for (++it; it != endIt; ++it, ++distance) {
    Sm::Spacer::Cathegory cat = (*it)->cathegory();
    switch (cat) {
      case Spacer::OCreate_:
      case Spacer::OCallCmd_:
      case Spacer::Connect_:
      case errCat:
      case errCat2: {
        std::vector<Spacer*> vPrev;
        std::vector<Spacer*> vNext;
        int i = 0;
        for (SpacerStream::iterator dbgIt = it; dbgIt != ctx.beginIt && dbgIt != ctx.s.begin() && i < 1000; --dbgIt, ++i)
          vPrev.push_back(dbgIt->object());
        i = 0;
        for (SpacerStream::iterator dbgIt = it; dbgIt != ctx.s.end() && i < 1000; ++dbgIt, ++i)
          vNext.push_back(dbgIt->object());
        continue;
        throw 999;
      }
      case closeCat: {
        strdst.push_back(';');
        if (ctx.uniqueHeader.empty())
          ctx.uniqueHeader = strdst;
        executeCollectedCommand(ctx, it);
        strdst.clear();
        return;
      }
      default:
        addTextChunk(strdst, it, ctx.s, isNotInl);
    }
  }
  throw 999;
}



void StoreInDBContext::doStorageLoop() {
  using namespace Sm;
  for (SpacerStream::iterator it = s.begin(); it != s.end(); ++it) {
    Spacer::Cathegory cat = (*it)->cathegory();
    SpacerStream::iterator begIt = it;
    switch (cat) {
      case Spacer::InlMessage_:
        continue;
      case Spacer::OCreate_: {
        collectSqlCommand<Spacer::CCreate_, Spacer::CCallCmd_, Spacer::GrantCmd_, false>(it);
        outProgress();
        break;
      }
      case Spacer::OCallCmd_: {
        collectSqlCommand<Spacer::CCallCmd_, Spacer::CCreate_, Spacer::GrantCmd_, false>(it);
        outProgress();
        break;
      }
      case Spacer::GrantCmd_: {
        string strdst;
        (*it)->toUnformattedString(strdst);
        LCommandContext ctx(strdst, begIt, cat, CmdCat::GRANT, s, "", true, "");
        executeCollectedCommand(ctx, it);
        outProgress();
        break;
      }
      case Spacer::Connect_:
        createLWriter(**it);
        break;
      default: {
        switch ((*it)->cathegory()) {
          case Spacer::OGrantCmd_:
            throw 999;
          case Spacer::Endl_:
          case Spacer::Semicolon_:
          case Spacer::InlMessage_:
            break;
          default: {
            std::vector<Spacer*> v;
            int i = 0;
            for (SpacerStream::iterator dbgIt = next(it); dbgIt != s.begin() && i < 10; ++dbgIt, ++i)
              v.push_back(dbgIt->object());
            i = 0;
            for (SpacerStream::iterator dbgIt = it; dbgIt != s.begin() && i < 1000; --dbgIt, ++i)
              v.push_back(dbgIt->object());
//            string s;
//            (*it)->toUnformattedString(s);
//            cout << s;
            break;
          }
        }
      }
    }
  }
}



void StoreInDBContext::executeCollectedCommand(const LCommandContext &ctx, Sm::SpacerStream::iterator &endIt) {
  using namespace Sm;
  if (ctx.needLinCmd) {
    StatisticNode node(ctx.entity, ctx.query);
    node.beginIt = ctx.beginIt;
    node.endIt   = endIt;
    node.uniqueHeader = ctx.uniqueHeader;
    bool res = currentConnection.second->notSelectQuery(ctx.query, &node);
    addToStatistics(ctx, node, res);

    if (currentConnection.second->linterCallInterface.dbNeedToRestart()) {
      cout << "ERROR ERROR ERROR !!! RDBMS need to restart!" << endl
           << "ERROR ERROR ERROR !!! RDBMS need to restart!" << endl
           << "ERROR ERROR ERROR !!! RDBMS need to restart!" << endl;

      if (!syntaxerContext.emulateErrorsInCallInterface) {
        system(syntaxerContext.stopLinterCommand.c_str());
        system(syntaxerContext.startLinterCommand.c_str());
      }

      clearLWriterMap();
      CurrentConnection oldConn = currentConnection;
      createSystemLWriter();
      createLWriter(s::connect(oldConn.first));
    }
  }
}

void StoreInDBContext::setUniqueHeader(const string &uniqueHeader, Sm::StatisticNode &statNode, const LCommandContext &ctx) {
  statNode.uniqueHeader = uniqueHeader;
  Sm::Codestream refStr;
  Sm::sAssert(!ctx.entity);
  ctx.entity->sqlReference(refStr);
  statNode.uniqueHeader.append(refStr.str());
}

void StoreInDBContext::addToStatistics(const LCommandContext &ctx, Sm::StatisticNode &statNode, bool operationResult) {
  typedef std::map<Sm::ResolvedEntity::ScopedEntities, std::pair<std::string, pair<EntitySet, EntitySet> > > AssocData;
  static const AssocData assocData = {
    {Sm::ResolvedEntity::Function_      , {"PROCEDURE ", {&StoreInDBContext::createdProcedures, &StoreInDBContext::allProcedures} } },
    {Sm::ResolvedEntity::MemberFunction_, {"PROCEDURE ", {&StoreInDBContext::createdProcedures, &StoreInDBContext::allProcedures} } },
    {Sm::ResolvedEntity::View_          , {"VIEW "     , {&StoreInDBContext::createdViews     , &StoreInDBContext::allViews     } } },
    {Sm::ResolvedEntity::Trigger_       , {"TRIGGER "  , {&StoreInDBContext::createdTriggers  , &StoreInDBContext::allTriggers  } } },
    {Sm::ResolvedEntity::Variable_      , {"VARIABLE " , {0, 0} } },
    {Sm::ResolvedEntity::Table_         , {"TABLE "    , {0, 0} } },
    {Sm::ResolvedEntity::Index_         , {"INDEX "    , {0, 0} } },
    {Sm::ResolvedEntity::IndexUnique_   , {"INDEX "    , {0, 0} } },
    {Sm::ResolvedEntity::Sequence_      , {"SEQUENCE " , {0, 0} } }
  };

  static const OCmdAssocData oCmdAssocData = {
    { Sm::CmdCat::DROP_USER       , {"DROP USER "       , {false, {0, 0} } } },
    { Sm::CmdCat::DROP_INDEX      , {"DROP INDEX "      , {false, {0, 0} } } },
    { Sm::CmdCat::DROP_TABLE      , {"DROP TABLE "      , {false, {0, 0} } } },
    { Sm::CmdCat::ALTER_TABLE     , {"ALTER TABLE "     , {false, {0, 0} } } },
    { Sm::CmdCat::VARIABLE        , {"VARIABLE "        , {true , {0, 0} } } },
    { Sm::CmdCat::TABLE_FOR_OBJECT, {"TABLE FOR OBJECT ", {true , {0, 0} } } },
    { Sm::CmdCat::INDEX_FOR_OBJECT, {"INDEX FOR OBJECT ", {false, {0, 0} } } },
    { Sm::CmdCat::PROC            , {"PROCEDURE "       , {true , {&StoreInDBContext::createdProcedures, &StoreInDBContext::allProcedures } } } },
    { Sm::CmdCat::VIEW            , {"VIEW "            , {true , {&StoreInDBContext::createdViews,      &StoreInDBContext::allViews      } } } },
    { Sm::CmdCat::VIEW            , {"TRIGGER "         , {true , {&StoreInDBContext::createdTriggers  , &StoreInDBContext::allTriggers   } } } },
  };

  EntitySet successContainer = 0;
  EntitySet allContainer     = 0;

  if (ctx.cmdCat == Sm::CmdCat::CREATE_SPACER) {
    AssocData::const_iterator it = assocData.find(ctx.entity->ddlCathegory());
    Sm::sAssert(it == assocData.end());
    setUniqueHeader(it->second.first, statNode, ctx);
    successContainer = it->second.second.first;
    allContainer     = it->second.second.second;
  }
  else if (ctx.cmdCat == Sm::CmdCat::GRANT) {
    Sm::s::grant *grnt = (*ctx.beginIt)->toSelfGrant();
    Sm::sAssert(!grnt);
    Sm::Codestream s;
    grnt->baseToString(s);
    statNode.uniqueHeader = s.str();
  }
  else {
    OCmdAssocData::const_iterator it = oCmdAssocData.find(ctx.cmdCat);
    if (it != oCmdAssocData.end()) {
      if (!it->second.second.first)
        setUniqueHeader(it->second.first, statNode, ctx);
      successContainer = it->second.second.second.first;
      allContainer     = it->second.second.second.second;
    }
  };

  if (allContainer && ctx.entity)
    allContainer->insert(ctx.entity->getDefinitionFirst());
//    (this->*allContainer).insert(ctx.entity->getDefinitionFirst());
  if (operationResult) { // для успешной операции - нужно посчитать созданные сущности по категориям
    if (successContainer)
      successContainer->insert(ctx.entity->getDefinitionFirst());
//      (this->*successContainer).insert(ctx.entity->getDefinitionFirst());
  }
  else {
    cout << statNode.toString() << endl;
    Sm::StatisticNode::errorStatVector.push_back(statNode);

    if (ctx.beginIt == ctx.s.begin())
      return;

    Sm::SpacerStream::iterator it;
    it = ctx.beginIt;
    --it;
    while (it != ctx.s.begin() && (*it)->cathegory() == Sm::Spacer::InlMessage_)
      --it;
    ++it;
    string s;
    for (; it != ctx.beginIt; ++it) {
      (*it)->toUnformattedString(s);
      cout << s << flush;
      s.clear();
    }
  }
}

void StoreInDBContext::clearLWriterMap() {
  for (LWriterMap::value_type &v : lwriterMap)
    delete v.second.second;
  lwriterMap.clear();
}

void StoreInDBContext::createLWriter(const Sm::Spacer &sp, bool isSystemLwriter) {
  using namespace Sm;

  string name;
  sp.toUnformattedString(name);
  LWriterMap::iterator writerIt = lwriterMap.find(name);
  if (writerIt != lwriterMap.end()) {
    currentConnection = writerIt->second;
    return;
  }
  UserContext *userCntx = sp.userContext();
  sAssert(!userCntx);

  LinterWriter *lwriter = new LinterWriter(sp.dbConnectUname(), sp.dbConnectPassword(), syntaxerContext.linterNodename);
  if (!isSystemLwriter && !systemLWriter) {
    cout << "ERROR: bad connection for SYSTEM user" << endl;
    return;
  }
  switch (lwriter->badConnectionError) {
    case NORMAL:
      break;
    case Invalid_User_Name: {
      if (isSystemLwriter) {
        cout << "ERROR: bad connection to SYSTEM user" << endl;
        return;
      }
      Codestream str[2];
      userCntx->linterDefinition(str[0]);
      userCntx->grantResourceDefinition(str[1]);
      for (int i = 0; i < 2; ++i) {
        string query = str[i].str();
        StatisticNode node(userCntx, query);
        systemLWriter->notSelectQuery(query, &node);
        if (node.codeErr != NORMAL) {
          cout << node.toString() << endl;
          return;
        }
      }
      lwriter = new LinterWriter(sp.dbConnectUname(), sp.dbConnectPassword(), syntaxerContext.linterNodename);
      if (lwriter->badConnectionError != NORMAL) {
        cout << "Error: critical call interface error" << endl;
        return;
      }
    }
    default:
      break;
  }

  currentConnection = make_pair(sp.userContext(), lwriter);
  lwriterMap[name] = currentConnection;
}

void StoreInDBContext::createSystemLWriter() {
  createLWriter(Sm::s::connect(syntaxerContext.model->getSystemUser().object()), true);
  systemLWriter = currentConnection.second;
}


