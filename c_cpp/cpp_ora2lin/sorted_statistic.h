#ifndef SORTED_STATISTIC_H
#define SORTED_STATISTIC_H

#include <string>
#include <vector>
#include <map>
#include "lintypes.h"
#include "lex_location.h"
#include "smartptr.h"
#include "semantic_expr.h"
#include "codestream.h"


namespace Sm {
  class ResolvedEntity;

struct StmtSpacerContext;

struct IdentifiedStatements {
  Sm::s::statementPointer* stmt = 0;
  unsigned int line = 0;

  IdentifiedStatements();
  IdentifiedStatements(Sm::s::statementPointer* _stmtPtr, unsigned int _line)
    : stmt(_stmtPtr), line(_line) {}

  IdentifiedStatements(const IdentifiedStatements &oth)
    : stmt(oth.stmt), line(oth.line) {}
};

struct StmtSpacerContext {
  int start = -1;
  int end   = -1;
  Sm::s::statementPointer *stmt = 0;

  StmtSpacerContext() {}
  StmtSpacerContext(int _start, Sm::s::statementPointer *_stmt)
    : start(_start), stmt(_stmt) {}
  bool empty() { return start < 1; }
  bool inRange(int l) { return l >= start && l <= end; }

  bool isCall();
};

struct ProcError : public SpacerCathegories {
  typedef std::vector<Sm::s::statementPointer*> BlockStatements;

  enum ParameterCathegory { EMPTY, WORD, STRING };
  L_LONG codeErr  = 0;
  bool isCodeGete = false;
  int row    = 0;
  int column = 0;

  std::string username;
  std::string password;

  ParameterCathegory paramCat = EMPTY;
  L_WORD      w_parameter = 0;
  std::string c_parameter;

  void setParam(L_WORD param);
  void setParam(const std::string &param);
  void setPosition(int r, int c);

  StmtSpacerContext identifyStatement(Sm::SpacerStream::iterator begIt, Sm::SpacerStream::iterator endIt) const;

  ProcError(const string &uname, const string &passwd)
    : username(uname), password(passwd) {}
  ProcError(const string &uname, const string &passwd, L_LONG _codeErr)
    : codeErr(_codeErr), username(uname), password(passwd) {}

  std::string toString() const;

  void printErrorWithItsContext(stringstream &dst, const string &query, int contextLines, const StatisticNode *err) const;
  bool isExecuteOperatorError() const { return codeErr == PROC_E_TRANERROR && paramCat == WORD; }
};

class StatisticNode {
protected:
public:
  typedef std::vector<const Sm::StatisticNode*> ErrorsVector;
  typedef std::map<int /*code*/, ErrorsVector> ErrorsMap;

  typedef std::vector<Sm::StatisticNode> ErrorStatistic;

  static ErrorStatistic errorStatVector;
  StatisticNode() {}
  typedef std::map<string, Ptr<Sm::String> > ModelFiles;
  static ModelFiles modelFiles;

  static void cleanup();
  StatisticNode(Sm::ResolvedEntity *_modelEntity, const std::string &_query);

  bool isVirtualErrorItem(StatisticNode &vErrNode);

  std::vector<ProcError> procErrors;
  cl::filelocation virtualErrorLocation = cl::emptyFLocation();

  /// Уникальный заголовок
  std::string uniqueHeader;

  Sm::ResolvedEntity *modelEntity = 0;

  std::string query;
  L_LONG codeErr = 0;
  bool dbmsHasBeenRestart = false;

  Sm::SpacerStream::iterator beginIt;
  Sm::SpacerStream::iterator endIt;

  std::string toString(bool verbose = false, int errorCode = 0, const string &prefixNodes = "") const;

  static void printExecuteOperatorError(const StatisticNode *error, const ProcError &procErr, stringstream &str, Sm::SpacerStream::iterator it, Sm::SpacerStream::iterator endIt);
  static void printSortedStatistic(ErrorsMap &errorsMap, string suffix);
  static void printAggregatedSortedStatistic(const std::vector<Sm::StatisticNode> &stats);
  static void printProcPrefix(stringstream &prefixNodes, const ProcError &procErr);
};



class StatisticNodeSmart : public StatisticNode, public smart::Smart {
public:
  StatisticNodeSmart() : StatisticNode() {}
};

class StatisticContainer {
  std::vector<StatisticNode> items;
public:
  typedef StatisticNode value_type;

  void add(value_type &item) { items.push_back(item); }
  void printSortedItems() { throw 999; }
};

class VirtualErrors {
public:
  typedef std::map<std::string, StatisticNodeSmart*> VirtualMap;
  std::vector<smart::Ptr<StatisticNodeSmart> > vstat;
  VirtualMap mapByLoc;
  VirtualMap mapByUniqueId;
};

namespace AbstractPy {
  class NodeNumId;
  class NodeRawId;
  class Dict;
  class List;

  class Node : public smart::Smart {
  public:
    virtual AbstractPy::NodeNumId* toSelfNodeNumId() { return 0; }
    virtual AbstractPy::NodeRawId* toSelfNodeRawId() { return 0; }
    virtual AbstractPy::Dict* toSelfDict() { return 0; }
    virtual AbstractPy::List* toSelfList() { return 0; }
    virtual ~Node() {}
    cl::filelocation loc = cl::emptyFLocation();
    Node(CLoc l) : loc(l) {}
  };

  class NodeNumId : public Node {
  public:
    Ptr<NumericValue> val;
    NodeNumId(CLoc l, Ptr<NumericValue> v) : Node(l), val(v) {}
    NodeNumId(CLoc l, Ptr<NumericValue> v, int);
    AbstractPy::NodeNumId* toSelfNodeNumId() { return this; }
  };


  class NodeRawId : public Node {
  public:
    Ptr<Id> val;
    NodeRawId(CLoc l, Ptr<Id> v) : Node(l), val(v) {}
    AbstractPy::NodeRawId* toSelfNodeRawId() { return this; }
  };


  class Dict : public Node, public std::map<string, Ptr<Node> > {
  public:
    Dict(CLoc l) : Node(l) {}
    AbstractPy::Dict* toSelfDict() { return this; }
  };

  class List : public Node, public std::vector<Ptr<Node> > {
  public:
    List(CLoc l) : Node(l) {}
    AbstractPy::List* toSelfList() { return this; }
  };

  void addConfigVirtualError(Ptr<Dict> dict);
}




}




#endif // SORTED_STATISTIC_H

