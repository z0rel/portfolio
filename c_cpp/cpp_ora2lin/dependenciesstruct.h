#ifndef DEPENDENCIESSTRUCT_H
#define DEPENDENCIESSTRUCT_H

#include <iomanip>
#include <fstream>
#include <functional>
#include <sys/stat.h>
#include "depworkflow.h"
#include "syntaxer_context.h"
#include "usercontext.h"
#include "codestream.h"
#include "model_context.h"
#include "semantic_function.h"
#include "semantic_table.h"

#include "depworkflow.h"

using namespace std;
using namespace Sm;

namespace dep {

class DependenciesBuilder : public Sm::CathegoriesOfDefinitions {
public:
  DependenciesStruct &src;
  DependenciesBuilder(DependenciesStruct &_src) : src(_src) {}

  void collectDependenciesSubgraph(Sm::ResolvedEntity *def);

private:
  void getDependenciesSubgraph(Sm::ResolvedEntity* def);
};

class LayingTypes : public Sm::CathegoriesOfDefinitions {
public:

  typedef unsigned int NodeId;
  typedef unsigned int Level;

  typedef std::vector<bool>                FlagsVisited;
  typedef std::vector<NodeId>              NumNodes;
  typedef std::vector<Sm::ResolvedEntity*> Nodes;

  typedef bool (SortFunctor)(NodeId l, NodeId r);

  typedef unordered_map<CathegoriesOfDefinitions::ScopedEntities, Level> SpecialCathegories;
};

class GraphLayingContext : public LayingTypes {
public:
  typedef std::vector<Level> NodesLevels;
  typedef std::map<Sm::ResolvedEntity*, NodeId, LE_ResolvedEntities> NodeIds;
  typedef std::pair<NodeId, NodeId> Arc;
  typedef std::vector<Arc> Arcs;

  typedef std::map<NodeId, std::set<NodeId> > Tree;
  typedef std::vector<Sm::DependEntitiesMap::iterator> Stack;

public:
  Nodes        nodes;
  Arcs         arcs;
  NumNodes     sortedNodes;

public:

  GraphLayingContext(DependenciesStruct &src);

  void representAsPyDict();
  void laying();
  void calculateLevels();
  void setLevelRecursive(NodeId node, Level level);
  void addNode(ResolvedEntity *node, NodeIds &nodeIds);
};




class SpecialSortingContext : public LayingTypes {
public:
  const SpecialCathegories frontCathegories = {
    {Sequence_        , 0},
    {Table_           , 1},
    {SpecialKeysActor_, 2},
    {IndexUnique_     , 3},
    {Index_           , 4}
  };

  const SpecialCathegories backCathegories = {
    {Trigger_       , 0}
  };

  GraphLayingContext &src;

  SpecialSortingContext(GraphLayingContext &_src);

  NumNodes filterSpecial(const SpecialCathegories &filter, const function<bool(NodeId, NodeId)> &sortFunctor);
  NumNodes layingProcView();


  // сравнение по категории и по имени
  bool operator() (NodeId l, NodeId r) const;
};


class LayingProcViewCtx : public LayingTypes {
public:
  GraphLayingContext       &src;
  FlagsVisited              flagsVisited;
  FlagsVisited              childsVisited;
  NumNodes                  layedNodes;
  std::set<ScopedEntities>  allSpecialCathegories;
  GraphLayingContext::Tree *tree = 0;

  LayingProcViewCtx(SpecialSortingContext &_src);

  void layingOnTraverse(NodeId node, unsigned int deepLevel);

  void addLayedNode(NodeId node);
};



bool ltByUserAndName(ResolvedEntity *lDef, ResolvedEntity *rDef);


class NumArc {
public:
  typedef GraphLayingContext::NodeId T;
  T from = numeric_limits<T>::max();
  T to   = numeric_limits<T>::max();

  NumArc() {}
  NumArc(const T &_from, const T &_to) : from(_from),    to(_to) {}
  NumArc(const NumArc &oth)            : from(oth.from), to(oth.to) {}

  bool operator< (const NumArc &r) const { return from < r.from || (from == r.from && to < r.to); }
  bool operator== (const NumArc &r) const { return from == r.from && to == r.to; }
};


class PtrArc {
public:
  typedef Sm::ResolvedEntity* T;
  T from = 0;
  T to   = 0;

  PtrArc() {}

  PtrArc(const T &_from, const T &_to)
    : from(_from), to(_to) {}

  PtrArc(const PtrArc &oth)
    : from(oth.from), to(oth.to) {}

  bool operator< (const PtrArc &r) const {
    return Sm::LE_ResolvedEntities::lt(from, r.from) ||
           (from == r.from && Sm::LE_ResolvedEntities::lt(to, r.to));
  }
  bool operator== (const PtrArc &r) const { return from == r.from && to == r.to; }
};


typedef vector<PtrArc> PtrArcs;
typedef vector<Sm::ResolvedEntity*> PtrNodes;

typedef vector<NumArc> NumArcs;
typedef GraphLayingContext::NumNodes NumNodes;


template<typename T>
void transformSortAndUnique(T &v) {
  std::stable_sort(v.begin(), v.end());
  v.erase(unique(v.begin(), v.end()), v.end());
}


template<>
void transformSortAndUnique(vector<Sm::ResolvedEntity*> &v) {
  std::stable_sort(v.begin(), v.end(), Sm::LE_ResolvedEntities());
  v.erase(unique(v.begin(), v.end()), v.end());
}


template <typename T>
std::vector<typename T::T> fromNodes(const std::vector<T> &arcs) {
  std::vector<typename T::T> out;
  for (const typename std::vector<T>::value_type &it : arcs)
    out.push_back(it.from);

  transformSortAndUnique(out);
  return out;
}

template <typename T>
std::vector<typename T::T> toNodes(const std::vector<T> &arcs) {
  std::vector<typename T::T> out;
  for (const typename std::vector<T>::value_type &it : arcs)
    out.push_back(it.to);

  transformSortAndUnique(out);
  return out;
}


template<typename T>
T intersect(const T &a, const T &b) {
  T res(max(a.size(), b.size()));
  typename T::iterator it = set_intersection(a.begin(), a.end(), b.begin(), b.end(), res.begin());
  res.resize(it - res.begin());
  return res;
}

template<>
PtrNodes intersect(const PtrNodes &a, const PtrNodes &b) {
  PtrNodes res(max(a.size(), b.size()));
  typename PtrNodes::iterator it =
      set_intersection(a.begin(), a.end(), b.begin(), b.end(), res.begin(), Sm::LE_ResolvedEntities());
  res.resize(it - res.begin());
  return res;
}


template<typename T>
T diff(const T &a, const T &b) {
  T res(max(a.size(), b.size()));
  typename T::iterator it = set_difference(a.begin(), a.end(), b.begin(), b.end(), res.begin());
  res.resize(it - res.begin());
  return res;
}


template<>
PtrNodes diff(const PtrNodes &a, const PtrNodes &b) {
  PtrNodes res(max(a.size(), b.size()));
  typename PtrNodes::iterator it
      = set_difference(a.begin(), a.end(), b.begin(), b.end(), res.begin(), Sm::LE_ResolvedEntities());
  res.resize(it - res.begin());
  return res;
}


template<typename T>
vector<typename T::T> getConnectedNodes(const vector<T> &arcs)
{
  vector<typename T::T> to = toNodes(arcs);
  vector<typename T::T> connectedNodes = fromNodes(arcs);
  connectedNodes.insert(connectedNodes.end(), to.begin(), to.end());
  transformSortAndUnique(connectedNodes);

  return connectedNodes;
}


PtrArcs makeArcs(DependenciesStruct &src);
PtrNodes getAllNodes(DependenciesStruct &src);
PtrNodes orphan(DependenciesStruct &src);
PtrNodes getRootNodes(DependenciesStruct &src);
NumArcs graphToArcs(GraphLayingContext &src);


void addArc(PtrArcs &arcs, ResolvedEntity* from, ResolvedEntity *to);

}

#endif // DEPENDENCIESSTRUCT_H
