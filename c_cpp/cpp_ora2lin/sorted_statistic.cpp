#include <vector>
#include <iomanip>

#include "sorted_statistic.h"
#include "resolved_entity.h"
#include "siter.h"
#include "syntaxer_context.h"
#include "semantic_base.h"
#include "lwriter.h"

std::map<string, Ptr<Sm::String> > Sm::StatisticNode::modelFiles;

using namespace std;

extern SyntaxerContext syntaxerContext;

void Sm::ProcError::setParam(unsigned short param) {
  w_parameter = param;
  paramCat = ProcError::WORD;
}

void Sm::ProcError::setParam(const string &param) {
  c_parameter = param;
  paramCat = ProcError::STRING;
}

void Sm::ProcError::setPosition(int r, int c) {
  row    = r;
  column = c;
}

string Sm::ProcError::toString() const {
  stringstream str;
  str << "{";
  if (codeErr)
    str << "\"code\":" << codeErr
        << ",\"r\":" << row << ",\"c\":" << column;
  if (isCodeGete)
    str << ",\"isCodeGete\":\"t\"";
  switch (paramCat) {
    case EMPTY : break;
    case WORD  : str << ",\"pw\":" << w_parameter; break;
    case STRING: str << ",\"ps\":" << PlsqlHelper::quotingAndEscapingPython(c_parameter); break;
  }

  str << "}";
  return str.str();
}

string Sm::StatisticNode::toString(bool verbose, int errorCode, const string &prefixNodes) const {
  stringstream str;
  str << "{";

  str << prefixNodes;
  if (verbose) {
    string en, ru;
    setErrDescriptions(en, ru, errorCode);
    if (!ru.empty() || !en.empty())
      str << "\"descr\":" << PlsqlHelper::quotingAndEscapingPython(!ru.empty() && verbose ? ru : en) << ",";
  }

  str << "\"code\":" << codeErr << ",";
  str << "\"lloc\":";
  if (modelEntity) {
    cl::filelocation l = modelEntity->getLLoc();
    str << "{";
    if (l.file)
      str << PlsqlHelper::quotingAndEscapingPython(*l.file);
    else
      str << "\"\"";
    str << ":[" << l.loc.begin.line << "," << l.loc.begin.column << "]";
    str << "}";
  }
  else
    str << PlsqlHelper::quotingAndEscapingPython(cl::emptyFLocation().toString());


  str << ",";
  str << "\"id\":" << PlsqlHelper::quotingAndEscapingPython(uniqueHeader);

  if (!procErrors.empty()) {
    str << ",\"procerrs\":[";
    bool isNotFirst = false;
    for (vector<ProcError>::const_iterator it = procErrors.begin(); it != procErrors.end(); ++it) {
      if (isNotFirst)
        str << ",";
      if (it->codeErr || it->isCodeGete) {
        isNotFirst = true;
        str << (*it).toString();
      }
    }
    str << "]";
  }
  str << "},";

  return str.str();
}


static bool compareStatNodeLt(const Sm::StatisticNode *l, const Sm::StatisticNode *r)  {
  if (l == r)
    return true;
  if (l->modelEntity && r->modelEntity) {
    cl::filelocation lLoc = l->modelEntity->getLLoc();
    cl::filelocation rLoc = r->modelEntity->getLLoc();

    if (lLoc.file == rLoc.file) {
      return lLoc.loc.begin.line < rLoc.loc.begin.line ||
            (lLoc.loc.begin.line == rLoc.loc.begin.line && lLoc.loc.begin.column < rLoc.loc.begin.column);
    }
    else if (lLoc.file && rLoc.file)
      return lexicographical_compare(lLoc.file->begin(), lLoc.file->end(), rLoc.file->begin(), rLoc.file->end());
  }
  return lexicographical_compare(l->uniqueHeader.begin(), l->uniqueHeader.end(), r->uniqueHeader.begin(), r->uniqueHeader.end());
}

struct CountContext {
  size_t       lines = 0;
  unsigned int count = 0;
  int          code  = 0;
  string       head;
  string       body;

  static bool cmpByLines (const CountContext *l, const CountContext *r) { return l->count > r->count; }
};


void Sm::StatisticNode::printProcPrefix(stringstream &prefixNodes, const ProcError &procErr)
{
  prefixNodes << "\"row\":" << procErr.row;
  if (procErr.codeErr == PROC_E_TRANERROR && procErr.paramCat == ProcError::EMPTY)
    prefixNodes << ",\"error in call\"";
  prefixNodes << ",";
}

void Sm::StatisticNode::printExecuteOperatorError(
    const Sm::StatisticNode* error, const ProcError &procErr, stringstream &str, Sm::SpacerStream::iterator it, Sm::SpacerStream::iterator endIt) {
  Sm::StmtSpacerContext identifiedStmt = procErr.identifyStatement(it, endIt);
  string pname;
  (*next(it))->toUnformattedString(pname);

  if (!identifiedStmt.stmt) {
    cout << "ERROR identify 10083: " << pname << endl;
    return;

    while(1)
      procErr.identifyStatement(it, endIt);
    throw 999;
  }

  stringstream prefixNodes;
  printProcPrefix(prefixNodes, procErr);
  str << error->toString(true, procErr.codeErr, prefixNodes.str()) << endl;
  str << error->query << endl << "-------------------" << endl;

  stringstream unamePasswd;
  if (procErr.username.size())
    unamePasswd << "USERNAME " << procErr.username << "/" << procErr.password;
  else
    unamePasswd << "USERNAME " << "SYSTEM/MANAGER";

  string procName;
  (*next(it))->toUnformattedString(procName);
  Ptr<Spacer> sp = *it;
  string procOracle;


  if (ResolvedEntity *proc = sp->commandEntity()) {
    if (Ptr<Id2> n = proc->getName2()) {
      cl::filelocation l = n->id[0]->getLLoc();
      stringstream procOracleStr;
      if (n->id[1])
        procOracleStr << n->id[1]->getLLoc().textFromFile() << ".";

      procOracleStr <<  l.textFromFile() << " (" << l.toString() << ")";
      procOracle = procOracleStr.str();
    }
    else if (Ptr<Id> n = proc->getName()) {
      cl::filelocation l = n->getLLoc();
      procOracle = l.textFromFile() + " (" + l.toString() + ")";
    }
    for (string::iterator sIt = procOracle.begin(); sIt != procOracle.end(); ++sIt)
      if (*sIt == '\n' || *sIt == '\r')
        *sIt = ' ';
  }

  Sm::Codestream s;
  cl::filelocation floc = identifiedStmt.stmt->stmt->getLLoc();
  using namespace PlsqlHelper;

  s << "vvvvvvvvvvv" << s::endl;
  s << "t = {'conn':" << quotingAndEscapingPython1(unamePasswd.str()) << ","
    << "'procOra':" << quotingAndEscapingPython1(procOracle) << ","
    << "'proc':" << quotingAndEscapingPython1(procName) << ","
    << "'row':" << procErr.row << ","
    << "'col':" << procErr.column << ","
    << "'isExecute':" << (procErr.paramCat == ProcError::WORD ? 1 : 0) << ","
    << "'location':" << quotingAndEscapingPython1(floc.toString()) << ","
    << "'fname':" << quotingAndEscapingPython1(floc.fullFilename()) << ","
    << "'byteBeg':" << floc.loc.begin.bytePosition << ","
    << "'byteEnd':" << floc.loc.end.bytePosition
    << "}";
  s << s::endl;
  s.state(identifiedStmt.stmt->state);
  s.state().isErrorLog = true;
  identifiedStmt.stmt->stmt->setNotOutStatements();
  identifiedStmt.stmt->stmt->translate(s);
  s.join();
  s.joinPostactions();
  s << Sm::s::iloc(identifiedStmt.stmt->stmt->getLLoc());
  s << s::endl;
  s << "^^^^^^^^^^^" << s::endl;

  str << s.str();
}

struct StatProcErrTerminate {

};

void Sm::ProcError::printErrorWithItsContext(stringstream &dst, const string &query, int contextLines, const StatisticNode */*err*/) const {
  int line = 1;
  for (string::const_iterator sIt = query.begin(); sIt != query.end(); ++sIt) {
    char c = *sIt;
    if (c == '\n') {
      ++line;
      if (line == row)
        dst << endl << "vvvvvvvvvvvvv";
    }
    if (abs(line - row) <= contextLines)
      dst << c;
  }
}

void Sm::StatisticNode::printSortedStatistic(ErrorsMap &errorsMap, string suffix) {
  typedef vector<CountContext*> Head;
  Head head;
  static const set<int> fullProcErrors = {
    PROC_E_NOMEM,        // не хватает памяти
    PROC_E_TR_MISMATCH,  // несоответствие числа команд BEGIN TRANSACTION и COMMIT/ROLLBACK TRANSACTION
    PROC_E_DUPEXCEPTHDR, // обработка исключения уже описана
  };
  for (ErrorsMap::value_type &errors : errorsMap) {
    if (errors.second.empty())
      continue;

    int errCode = errors.first;
    string en, ru;
    setErrDescriptions(en, ru, errCode);

    CountContext *ctx = new CountContext();
    ctx->count = errors.second.size();
    ctx->head  = ru.empty() ? en : ru;
    ctx->code  = errCode;

    stringstream str;
    str << "=== " << Sm::to_string(errCode) << " " << ctx->head << " (" << Sm::to_string(errors.second.size()) << ") ===" << endl;
    std::sort(errors.second.begin(), errors.second.end(), compareStatNodeLt);
    for (const Sm::StatisticNode* error : errors.second) {
      if (errCode < PROC_ERR_BASE || fullProcErrors.count(errCode))
        str << error->toString(true, errCode) << endl << error->query;
      else {
        try {
          for (vector<ProcError>::const_iterator it = error->procErrors.begin(); it != error->procErrors.end(); ++it) {
            const ProcError &procErr = *it;
            if (procErr.codeErr == PROC_E_TRANERROR && procErr.paramCat == ProcError::WORD)
              printExecuteOperatorError(error, procErr, str, error->beginIt, error->endIt);
            else {
              stringstream prefixNodes;
              StatisticNode::printProcPrefix(prefixNodes, procErr);
              str << error->toString(true, error->procErrors.size() ? error->procErrors.front().codeErr : 7200, prefixNodes.str()) << endl;
              procErr.printErrorWithItsContext(str, error->query, 5, error);
            }
          }
        }
        catch (StatProcErrTerminate /*procErrTerminate*/) {
          str << endl;
        }
      }
      str << endl << endl;
    }
    str << endl;

    ctx->body = str.str();
    for (string::iterator it = ctx->body.begin(); it != ctx->body.end(); ++it)
      if (*it == '\n')
        ++ctx->lines;

    head.push_back(ctx);
  }

  stringstream str;
  size_t offset = head.size() + 3;
  std::sort(head.begin(), head.end(), CountContext::cmpByLines);
  for (Head::value_type &ctx : head) {
    str << "line: "  << setw(10) << left << Sm::to_string(offset)
        << "count: " << setw(10) << left << Sm::to_string(ctx->count)
        << "code: "  << setw(10) << left << ctx->code << " " << ctx->head << endl;
    offset += ctx->lines;
  }
  str << endl << endl;
  for (Head::value_type &ctx : head) {
    str << ctx->body;
    delete ctx;
  }

  OutputCodeFile::storeCode(syntaxerContext.sortedErrorLogBase + suffix, str.str());
}

void Sm::StatisticNode::printAggregatedSortedStatistic(const std::vector<Sm::StatisticNode> &stats) {
  // отсортировать вектор по локейшнам

  ErrorsMap nonprocErrors;
  ErrorsMap procErrors;
  ErrorsMap executeOpErrors;

  for (const ErrorStatistic::value_type &v : stats) {
    if (v.codeErr != 7200)
      nonprocErrors[v.codeErr].push_back(&v);
    else if (v.procErrors.empty())
      procErrors[v.codeErr].push_back(&v);
    else {
      const ProcError &procErr = v.procErrors.front();
      if (procErr.isExecuteOperatorError())
        executeOpErrors[procErr.codeErr].push_back(&v);
      else
        procErrors[procErr.codeErr].push_back(&v);
    }
  }

  if (syntaxerContext.sortedErrorLogBase.empty()) {
    cout << "ERROR: sorted_error_log_base must be set nonempty in config" << endl;
    return;
  }

  printSortedStatistic(nonprocErrors  , "_nonproc.sql");
  printSortedStatistic(procErrors     , "_proc.sql");
  printSortedStatistic(executeOpErrors, "_execute_op.sql");
}



bool Sm::StmtSpacerContext::isCall() {
  return stmt && stmt->stmt &&
      (stmt->stmt->toSelfAssignment() ||
       stmt->stmt->toSelfFunctionCall() ||
       stmt->stmt->toSelfFetch() ||
       stmt->stmt->toSelfForOfRange());
}





Sm::StmtSpacerContext Sm::ProcError::identifyStatement(Sm::SpacerStream::iterator it, Sm::SpacerStream::iterator endIt) const {
  if (!row || row < 0)
    throw 999; // реализовать возврат самого первого call или execute;
  if (codeErr != PROC_E_TRANERROR)
    throw 999; // сюда попадать не должно
  string pname;
  (*next(it))->toUnformattedString(pname);
  bool isErr = pname == "PROCEDURE EXCHANGE1C.XML_SPR_DESCRIPTION_ADDINXMLKAGENTINFO";

  StmtSpacerContext previousCtx;
  StmtSpacerContext minDistanceCtx;

  int minDistance = numeric_limits<int>::max();
  int prevMinDistance = minDistance;

  int dbgCnt = 0;
  for (int line = -1; it != endIt; ++it) {
    dbgCnt++;
    string s;
    Ptr<Spacer> sp = *it;
    sp->toUnformattedString(s);

    for (string::iterator sIt = s.begin(); sIt != s.end(); ++sIt)
      if (*sIt == '\n')
        ++line;

    int distance = line - row;
    prevMinDistance = minDistance;
    if (distance < minDistance || minDistance <= 0) {
      minDistance = distance;
      minDistanceCtx = previousCtx;
    }

    Sm::s::statementPointer *stmt;
    if (!(stmt = sp->toSelfStatementPointer()))
      continue;

    if (isErr)
      cout << "";

    if (previousCtx.stmt && prevMinDistance >= 0 && prevMinDistance <= minDistance)  // убывание дистанции от строки ошибки прекратилось. Выбрать последний Execute или
      return previousCtx;
    else {
      switch (paramCat) {
        case WORD:
          if (Sm::s::executeStatementPointer *ex = sp->toSelfExecuteStatementPointer())
            previousCtx = StmtSpacerContext(line, ex);
          break;
        case EMPTY:
          if (!sp->toSelfExecuteStatementPointer()) {
            StmtSpacerContext ctx(line, stmt);
            if (ctx.isCall())
              previousCtx = ctx;
          } break;
        default: throw 999; break;
      }
    }
  }
  if (minDistanceCtx.stmt)
    return minDistanceCtx;
  return StmtSpacerContext();
}

Sm::AbstractPy::NodeNumId::NodeNumId(CLoc l, Ptr<Sm::NumericValue> v, int)
  : Node(l), val(v)
{
  v->neg();
}



void Sm::AbstractPy::addConfigVirtualError(Ptr<Dict> dict) {
  sAssert(!dict);

  smart::Ptr<StatisticNodeSmart> stat = new StatisticNodeSmart();
  for (Dict::value_type &i : *dict) {
    if (i.first == "code") {
      NodeNumId *n = i.second->toSelfNodeNumId();
      sAssert(!n);
      stat->codeErr = n->val->getSIntValue();
    }
    else if (i.first == "lloc") {
      NodeRawId *rId  = i.second->toSelfNodeRawId();
      if (!rId || rId->val->toString() != "0,0")
      {
        Dict *n = i.second->toSelfDict();
        sAssert(!n || n->empty() || n->size() > 1);
        Dict::value_type v = *(n->begin());
        string fname = v.first;
        List *lst = v.second->toSelfList();
        sAssert(!lst || lst->empty());
        NodeNumId *pyLine = (*lst)[0]->toSelfNodeNumId();
        sAssert(!pyLine);
        stat->virtualErrorLocation.loc.begin.line = pyLine->val->getSIntValue();
        if (lst->size() > 1) {
          NodeNumId *pyCol = (*lst)[1]->toSelfNodeNumId();
          sAssert(!pyCol);
          stat->virtualErrorLocation.loc.begin.column = pyCol->val->getSIntValue();
        }
        std::pair<StatisticNode::ModelFiles::iterator, bool> it =
            StatisticNode::modelFiles.insert(StatisticNode::ModelFiles::value_type(fname, 0));

        if (it.second)
          it.first->second = new Sm::String(fname);

        stat->virtualErrorLocation.file = it.first->second.object();
      }
    }
    else if (i.first == "id") {
      NodeRawId *n = i.second->toSelfNodeRawId();
      sAssert(!n);
      stat->uniqueHeader = n->val->toString();
    }
    else if (i.first == "procerrs") {
      List *n = i.second->toSelfList();
      sAssert(!n);
      for (List::value_type &v : *n) {
        Dict *d = v->toSelfDict();
        sAssert(!d);
        ProcError err("SYSTEM", "MANAGER");
        for (Dict::value_type &errItem : *d) {
          if (errItem.first == "code") {
            NodeNumId *n = errItem.second->toSelfNodeNumId();
            sAssert(!n);
            err.codeErr = n->val->getSIntValue();
          }
          else if (errItem.first == "r") {
            NodeNumId *n = errItem.second->toSelfNodeNumId();
            sAssert(!n);
            err.row = n->val->getSIntValue();
          }
          else if (errItem.first == "c") {
            NodeNumId *n = errItem.second->toSelfNodeNumId();
            sAssert(!n);
            err.column = n->val->getSIntValue();
          }
          else if (errItem.first == "pw") {
            NodeNumId *n = errItem.second->toSelfNodeNumId();
            sAssert(!n);
            err.paramCat = ProcError::WORD;
            err.w_parameter = n->val->getSIntValue();
          }
          else if (errItem.first == "ps") {
            NodeRawId *n = errItem.second->toSelfNodeRawId();
            sAssert(!n);
            err.paramCat = ProcError::STRING;
            err.c_parameter = n->val->toString();
          }
          else
            throw 999;
        }
        stat->procErrors.push_back(err);
      }
    }
    else
      throw 999;
  }
  syntaxerContext.virtualErrors.vstat.push_back(stat);
  syntaxerContext.virtualErrors.mapByLoc[stat->virtualErrorLocation.toString()] = stat;
  syntaxerContext.virtualErrors.mapByUniqueId[stat->uniqueHeader] = stat;
}


void Sm::StatisticNode::cleanup() {
  for (ModelFiles::value_type &v : modelFiles)
    v.second = 0;
}

Sm::StatisticNode::StatisticNode(Sm::ResolvedEntity *_modelEntity, const string &_query)
  : modelEntity(_modelEntity), query(_query) {}



bool Sm::StatisticNode::isVirtualErrorItem(StatisticNode &vErrNode) {
  if (!modelEntity)
    return false;
  cl::filelocation loc = modelEntity->getLLoc();

  loc.loc.end = cl::emptyFLocation().loc.end;
  VirtualErrors::VirtualMap::iterator lIt = syntaxerContext.virtualErrors.mapByLoc.find(loc.toString());
  if (lIt != syntaxerContext.virtualErrors.mapByLoc.end()) {
    vErrNode = *(lIt->second);
    return true;
  }

  VirtualErrors::VirtualMap::iterator uIt = syntaxerContext.virtualErrors.mapByUniqueId.find(uniqueHeader);
  if (uIt != syntaxerContext.virtualErrors.mapByUniqueId.end()) {
    vErrNode = *(uIt->second);
    return true;
  }
  return false;
}


