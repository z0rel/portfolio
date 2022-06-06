#ifndef DEPWORKFLOW_H
#define DEPWORKFLOW_H


#include <map>

#include "semantic_id_lists.h"
#include "model_head.h"


class DependenciesStruct {
public:
  typedef std::set<Sm::ResolvedEntity*, Sm::LE_ResolvedEntities> EntitiesForCodegen;
  typedef std::vector<Sm::ResolvedEntity *> SortedEntities;

  /// Множество сущностей для кодогенерации (скодогенрированы будут только они).
  /// В множество для каждой сущности включен контекст обхода.
  /// Это множество впоследствии преобразовывается в множество дуг
  EntitiesForCodegen entitiesForCodegen;

  UserContext *curUser = 0;

  DependenciesStruct();


  void init();
  void init(std::vector<Sm::IdEntitySmart> &outherReferences);
  void init(std::set<Sm::ResolvedEntity*, Sm::LE_ResolvedEntities> &baseReferences);

  void dumpSqlFromDb(std::string dirOrFile, bool splitFiles);
  void storeSql(SortedEntities &sortedEntities, Sm::Codestream &str, Sm::Codestream &str_vars);

  void buildTreeAndDumpSqlForDb(const char *dirOrFile, bool splitFiles);
  void layingGraph(DependenciesStruct::SortedEntities &entities);
  void checkoutUserCtx(UserContext *user, Sm::Codestream &str_vars, Sm::Codestream &str);
};




template <typename T>
inline void dumpDependenciesSql(T &container, const char *dirOrFile, bool splitFiles) {
  DependenciesStruct m;
  m.init(container);
  m.buildTreeAndDumpSqlForDb(dirOrFile, splitFiles);
}

void dumpDependenciesSql(const char *dirOrFile, bool splitFiles);

int openFile(const char* name, const char* opt, FILE **file);

class ModelActions;

class StoreBaseEntity : public Sm::CathegoriesOfDefinitions {
  Sm::Codestream &str;
  Sm::Codestream &str_vars;
  Sm::Codestream::SortedGranteesClauses &sortedGranteesClauses;
  ModelActions &modelActions;
  typedef std::map<Sm::ResolvedEntity*, int> EntityMap;
  EntityMap excludeEntities;

  struct ActionContext;

  typedef void (StoreBaseEntity::*Action)(Sm::ResolvedEntity*, ActionContext &);

  struct ActionContext {
    StoreBaseEntity::Action action = 0;
    bool   create = false;
    bool   drop   = false;
    void set(StoreBaseEntity::Action act, bool c, bool d) { action = act; create = c; drop = d; }
  };

  ActionContext actions[LAST_ENTITY_NUMBER];

  void storeInternal(Sm::Codestream &str, Sm::ResolvedEntity *item, ActionContext &ctx);

  void storeSequence(Sm::ResolvedEntity *vIt, ActionContext &ctx) { storeInternal(str, vIt, ctx); }
  void storeTable   (Sm::ResolvedEntity *vIt, ActionContext &ctx) { storeInternal(str, vIt, ctx); }
  void storeIndex   (Sm::ResolvedEntity *vIt, ActionContext &ctx) { storeInternal(str, vIt, ctx); }
  void storeSynonym (Sm::ResolvedEntity *vIt, ActionContext &ctx) { storeInternal(str, vIt, ctx); }
  void storeView    (Sm::ResolvedEntity *vIt, ActionContext &ctx);
  void storeTrigger (Sm::ResolvedEntity *vIt, ActionContext &ctx);
  void storeFunction(Sm::ResolvedEntity *vIt, ActionContext &ctx);
  void storeVariable(Sm::ResolvedEntity *vIt, ActionContext &ctx);
  void storeTabKeys (Sm::ResolvedEntity *vIt, ActionContext &ctx);

  void setupManualEntities();
  void storeManual(Sm::ManualEntity &manEntity);
public:

  StoreBaseEntity(Sm::Codestream &_str, Sm::Codestream &_str_vars, Sm::Codestream::SortedGranteesClauses &_sortedGranteesClauses);

  void store(ScopedEntities cat, Sm::ResolvedEntity *def);
  bool filterEntity(ScopedEntities cat, Sm::ResolvedEntity *def);
  void storeUndependedManual();

  void joinGrantee(ScopedEntities cat, UserContext *user);
};


#endif // DEPWORKFLOW_H
