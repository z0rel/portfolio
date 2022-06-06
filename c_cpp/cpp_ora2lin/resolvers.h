#ifndef RESOLVERS_H
#define RESOLVERS_H

#include "hash_table.h"
#include "semantic_utility.h"
#include "semantic_base.h"


namespace Sm {

//bool tryResolveFunref(ResolvedEntity *fun, Sm::Id *call, double &positives);



class FunctionResolvingContext {
public:

  struct Context {
    /// Объявление функции, которую пытаемся отрезолвить
    ResolvedEntity *fun = 0;
    /// Вызов функции, который мы пытаемся отрезолвить
    Id *call = 0;

    int summaryPower = 0;

    bool hasEverything = false; // asterisk, NULL


    bool isPlContext = false;
    bool inSqlCode   = false;

    int declIsDate     = 0;
    int declIsNumber   = 0;
    int declIsVarchar2 = 0;
    Datatype *argDeclT = 0;

    unsigned int settedArgPosition = 0;

    vector<bool> settedCallArguments;

    Context(ResolvedEntity *f, Id *c)
      : fun(f), call(c) {}

    void calculateArumentSupertype();

    bool tryResolveFunref(); // ResolvedEntity *fun, Id *call, double &positives
    void updateCompatibility(IsSubtypeValues conversionArgTypePower);

    bool compareDatatypesInCall(FunCallArg   *callArg);

  public:
    bool isSelfMfArgument(const Id &arg);
  };


private:
  typedef pair<double, ResolvedEntity*> PartitialResolvedFunction;
  typedef std::vector<PartitialResolvedFunction> PartitialResolvedFunctions;
  /// список частично отрезолвленных функций
//  PartitialResolvedFunctions partitialResolvedFunctions;
  void debugAddToPartitialResolvedVector(int p, ResolvedEntity *f);

  ResolvedEntity* maximalResolvedFundecl = 0;
  int currentPower = numeric_limits<int>::min();
  bool ambiguousDeclarations = false;

  bool tryResolveFunref(ResolvedEntity *fundecl, Id *call);

  void resolveFuncallByMaximalPower(VEntities &container, Id *call);
  bool setMaxMatchedResolvedFunction(Id *call);
  static bool setPragmaFunctionReferenceDefinition(Id* call);

public:
  void resolveMemberFunction(Ptr<Id> name, Sm::ResolvedEntity *mf);

  static bool resolveSameLvlFunDecl(Id *call);
  static bool resolveMfCall(Id *call, Type::Object *obj);
  static void resolveNewCall(Id *call, Type::Object *obj);
  void updateMaximalResolvedFundecl(Context &ctx);
};

class ResolvingContext {
public:
  enum ResolvingSpecialCasesState {
    RESOLVING_NEXT,
    RESOLVING_NEXT_WITHOUT_SPECIAL_CASES,
    RESOLVING_FINISH_SUCCESS,
    RESOLVING_FINISH_FAIL
  };


  SemanticTree                *foundedNode   = 0;
  SemanticTree                *foundedLevel = 0;
  SemanticTree                *reference    = 0;
  IdEntitySmart                    *referenceId  = 0;
  SemanticTree                *startLevel   = 0;
  SemanticTree                *currentLevel = 0;
  SemanticTree                *child        = 0;
  bool                         hasUnresolvedQuotedLiteral = false;
  bool                         childIsQueryBlock          = false;
  bool                         childIsNotFrom             = false;
  bool                         childIsFrom                = false;
  bool                         childIsInsertingValues     = false;
  bool                         childIsFactoring           = false;
  bool                         childIsSelectedFieldPart   = false;
  bool                         isExpression               = false;


  std::vector<ResolvedEntity*> partitialResolvedDefs;
  unsigned int oldcnt = 0;

protected:
  IdEntitySmart::iterator           currentPartIt;
public:
  ResolvingContext();

  void push(SemanticTree *child_, SemanticTree *currLevel);

  void setReference(IdEntitySmart *reference_, SemanticTree *level);

  bool resolveMemberFunctionCall();
  bool resolveTypecallEntity();
  bool resolveFuncallEntity();
  bool resolveStartFromLevel();          // called from SemanticTree::resolveOneCallReference
  bool commonFindDeclarationFromLevel(); // called from resolveStartFromLevel
  bool commonFindDeclarationUp();        // called from self, findLevelUp, commonFindDeclarationFromLevel


  bool findInLevelNamespaceOrLevelUp();

  bool findLevelup();
  bool clearAndFindLevelup();
  bool commonFindDeclDownInNode(Sm::SemanticTree *node);

  bool resolveInQueryBlock(ResolvedEntity *q);
  bool resolveAsSelectedField(ResolvedEntity *q);
  bool isSystemTemplateLastRef();
  bool resolveAsArgumentOfCurrentFunctionNamespace();

  ResolvingSpecialCasesState resolveSpecialCases();
  ResolvingSpecialCasesState resolveCollectionAccess();
  Ptr<Sm::Type::collection_methods::AccessToItem> createCollectionAccessOperator(ResolvedEntity *currentPartDef, SemanticTree *refSNode);
  void resetPartitialResolvedDefs();
};

class LinterSemanticResolver {
public:
  void resolveCreatedObjects(LinterCreatedEntities &linterCreatedEntities);
  void resolve(IdEntitySmart &reference);
private:
  bool resolveNestedFuncByName(ResolvedEntity *parent,
      ResolvedEntity *&def,
      const string   &nestedName,
      const string   &funcName);
  bool resolveNestedFunc(ResolvedEntity *parent, const std::string &name, ResolvedEntity *&def);
  template<typename Container>
  bool resolveNestedEntityByName(Container &c,
      ResolvedEntity       *&def,
      string                &funNameS,
      string                &pkgName);
  template <typename T>
  bool resolveCreatedLinterEntity(T &container, const std::string &name, ResolvedEntity *&foundPtr);
  void resolveNestedEntity(UserContext *user, const std::string &name, ResolvedEntity *&def);

};

/* Для инициализации перед резолвингом */
void collectEqualsDeclaration(Sm::SemanticTree::Childs::iterator node_begin , Sm::SemanticTree::Childs::iterator node_end,
                              LevelResolvedNamespace *levelResolvedNamespace);

void collectEqualsDeclarationOnNode(Sm::SemanticTree *node, LevelResolvedNamespace *levelResolvedNamespace);
void updateResolvedNamespace(LevelResolvedNamespace *levelResolvedNamespace, Sm::SemanticTree *node);

/* Резолверы базовых сущностей модели */
void resolve       (ModelContext &model, const Sm::Id2 *name);
void resolveTable  (ModelContext &model, const Sm::Id2 *name);
void resolveIndex  (ModelContext &model, const Sm::Id2 *name);
void resolveTrigger(ModelContext &model, const Sm::Id2 *name);
void resolveObject (ModelContext &model, const Sm::Id2 *name);

/* DML-резолвинг - метод SemanticTree */

Ptr<Sm::FunctionArgument> findByNormalizedName(ResolvedEntity *fun, CallArgList::iterator it, unsigned int &pos);

};

#endif // RESOLVERS_H
// vim:foldmethod=syntax
