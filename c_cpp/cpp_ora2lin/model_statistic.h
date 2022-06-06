#ifndef MODEL_STATISTIC
#define MODEL_STATISTIC

#include <set>
#include "depworkflow.h"

class ModelContext;

namespace Sm {
  class ResolvedEntity;
  class DepArcContext;
}

class ModelStatistic {
  ModelContext *cntx_;
  DependenciesStruct::SortedEntities *sortedEntities;
public:
  ModelStatistic(ModelContext *context, DependenciesStruct::SortedEntities *_sortedEntities)
    : cntx_(context), sortedEntities(_sortedEntities) {}

  void calculate();

  void generateSkippedReduction();

  void codegenBytelocations();
  void resolveDescrErrors();

  void showToCharArguments();
  void showComparsionDatatypes();
  void showComparsionLists();

  void extractExecuteImmediateAndOpenCursor();

  void getConfiguredDependEntities();
  void generatePythonRepr();
};


class DependenciesAnalyzer {
public:
  typedef std::map< /*from*/ Sm::ResolvedEntity*, /*arc context*/ Sm::DepArcContext> From;
  typedef std::map</*to*/ Sm::ResolvedEntity*, From> DependenciesGraph;
  typedef std::set<Sm::ResolvedEntity*> GraphNodes;

  DependenciesGraph                  dependenciesGraph;
  GraphNodes                         nodes;
  GraphNodes                         baseEntities;

  DependenciesStruct::SortedEntities layedNodes;

  DependenciesAnalyzer();

  /// Получить все сущности, которые зависят от baseEntity, и поместить их в nodes.
  /// Кроме того, заполнить дугами граф зависимостей dependenciesGraph
  void buildGraph(Sm::ResolvedEntity *toNode);

  void addArcAndTraverseDeep(Sm::ResolvedEntity *from, Sm::ResolvedEntity *to, const Sm::DepArcContext &arcCtx, bool needRecursiveBuild);
  void layingBySortedEntities(DependenciesStruct::SortedEntities *sortedEntities);

  std::string toPythonDepDict(GraphNodes &nodes);
  std::string toPythonDepList(DependenciesStruct::SortedEntities &nodes);
  std::string toPythonDepGraphDict();
  const char *toPythonCategory(Sm::ResolvedEntity *def) const;
  bool filterByCategory(Sm::ResolvedEntity *def) const;

private:
  std::string name(Sm::ResolvedEntity *def);

};




#endif // MODEL_STATISTIC

