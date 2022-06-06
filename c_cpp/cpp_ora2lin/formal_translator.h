#ifndef FORMAL_TRANSLATOR
#define FORMAL_TRANSLATOR

#include "model_context.h"
#include "model_head.h"

namespace Sm {
  namespace pl_expr {
    class Comparsion;
    class VariableField;
  }
}

struct LimitExprNode {
  int isNot = 0;
  int currentQueryDepth = 0;
  int limitExprDeep = 0;

  Ptr<Sm::pl_expr::Comparsion> cmp;
  LimitExprNode(int isNot, int queryDepth, int _limitExprDeep, Ptr<Sm::pl_expr::Comparsion> &_cmp);
  LimitExprNode();
};


void translateFieldComparsion(Sm::Codestream &str, Sm::FLoc lhsLoc, Sm::FLoc rhsLoc, Sm::EntityFields &lhsFields, Sm::EntityFields &rhsFields);

namespace Sm {
  class ConstructExprStmt;
}

class FormalTranslator {
  ModelContext &cntx_;
public:
  typedef Sm::PlExpr* (*Translator)(Sm::PlExpr*);
  std::unordered_set<Sm::Assignment*> refcursorAssignment;

  FormalTranslator(ModelContext &cntx);

  void formalTranslations();


  void initializeDeclarationsNames();
  void transformFunctions();


  // Преобразовать ссылки. Вызывает translateReferencesCondition, которая устанавливает транслятор и возвращает флаг необходимости преобразования
  // Если транслятор возвращен и преобразование нужно - перезаписывает выражение-ссылку новой, полученной из транслятора
  void translateReferences();

  int translateReferencesCondition(Sm::PlExpr *expr, FormalTranslator::Translator &translator);


  void translateStatements();


  void translateVariablesDynamicInitializers();
  void extractLimit1();


  // Преобразование вызовов функций с аргументами-рефкурсорами и проверка согласованности полей в других вызовах
  void translateFunctionCallsWithRefcursorArgs(Sm::PlExpr *expr);
  // Сопоставить все декларации пространств имен динамических выражений с ссылками на них в спецификаторах построения динамических выражений.
  void prepareDynamicExpressions();
  // Обработка явых деклараций полей курсора
  void translateCursorFieldDecltype();

  void translateAssignmentRefcursorFromFunctions();
  // Инициализация полей рефкурсоров, инициализируемых внутри вызовов функций
  void translateCallWithRefcursorArg(Sm::PlExpr *expr, Ptr<Sm::Id> &call, unsigned int pos, Sm::VariableCursorFields *srcVar);
  // Инициализация полей рефкурсоров из присваиваний
  bool translateRefcursorAssignments(Sm::Assignment *assignment);
  // Инициализация полей рефкурсоров из возврата значения
  bool translateRefcursorReturn(Sm::Function *fun);
  // Приведение типов фактических аргрументов к типам формальных в вызовах функций и в коллекциях.
  bool translateFunctionArgs(Sm::PlExpr *expr);
  // Преобразование запросов для рефкурсора для совпадения типов полей запроса и Fetch
  bool translateRefCursorQuery(Sm::OpenFor *openFor);

  bool translateReference(Sm::PlExpr* expr);
  bool translateQuery(Sm::PlExpr* expr);

  // Разворачивание структурированных ссылок, и их прединициализация - вставкой присваиваний оператором-владельцем выражения
  void translateStructuredFieldReference(Sm::PlExpr* expr);
  // Преобразование курсоров в курсорные переменные
  void translateCursorToCursorVariables(Sm::PlExpr* expr);

  void translateDynamicTables();

  static Sm::PlExpr* translateFunctionCallToItsRealDatatype(Sm::PlExpr* expr);

  typedef std::set<Sm::CursorDecltype*, Sm::LE_ResolvedEntities>  CursorDecltypes;

  void castAssignmentRvalue(Sm::Assignment *assignment);

  bool updateOpenForDecltypeStatements(Sm::OpenFor *openFor, CursorDecltypes &cursorDecltypes);
  void setFieldsOnUnopenedVariables(CursorDecltypes &cursorDecltypes);

  void setTranslatedName(Sm::ResolvedEntity *def, bool isPackageVairable, std::vector<Sm::Id*> &trNames, UserContext *currentUser);

  void traverseUserDeclarations(Sm::DeclActor tr, UserContext **currentUser = 0);
  void traverseAllDeclarations(Sm::DeclActor tr, UserContext **currentUser = 0);
  Sm::FunctionArgument* extractFunDef(Ptr<Sm::Id> &call, unsigned int pos, Sm::Function **funDef);
  void removeCursorFromTopBlockNamespace(string curName, Sm::BlockPlSql *topBlock);
  Sm::BlockPlSql *getTopBlock(Sm::PlExpr* expr);
};

#endif // FORMAL_TRANSLATOR

