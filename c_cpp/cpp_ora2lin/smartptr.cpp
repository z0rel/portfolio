#include <cstddef>
#include <exception>
#include <iostream>
#include <map>
#include <typeinfo>
#include <exception>

#include "smartptr.h"

//#define SMARTPTR_DEBUG


#ifdef SMARTPTR_DEBUG1
#include "semantic_base.h"
#include "resolved_entity.h"
#include "semantic_id.h"
#include "semantic_function.h"

bool smartptrDebugStop = false;


namespace smart {

void checkLevelSpace() {
  return;
  static int open = 0;
  if (open)
    return;
  ++open;

  --open;
}


template <typename T>
class PRef {
public:
  T *ref;
  PRef(T* p) : ref(p) {}
  T* object() const { return ref; }
  operator bool() const { return ref != nullptr; }
};

template <template<class> class C>
void checkPtrConsistment(const Smart *ptr)
{
  if (ptr->isDeleted__)
    throw 999;
  checkLevelSpace();
  Smart *p = const_cast<Smart*>(ptr);
  Sm::ResolvedEntity::CathegoriesOfDefinitions::ScopedEntities cat;
  (void)cat;
  if (C<Sm::ResolvedEntity> t = dynamic_cast<Sm::ResolvedEntity*>(p)) {
    cat = t.object()->ddlCathegory();
    if (Sm::SemanticTree *node = t.object()->getSemanticNode())
      if (Ptr<Sm::Id> name = node->declarationName())
        if (name->beginedFrom(91488, 34))
          std::cout << "" << std::endl;
  }
  else if (C<Sm::VEntities> t = dynamic_cast<Sm::VEntities*>(p)) {
    Sm::VEntities *p = t.object();
    (void)p;
//    p->check();
  }
  else if (C<Sm::SemanticTree> t = dynamic_cast<Sm::SemanticTree*>(p)) {
    Sm::SemanticTree *p = t.object();
    p->childs.empty();
//    if (!p->childs.empty()) {
//      Ptr<Sm::SemanticTree> n = *(p->childs.begin());
//      (void)n;
//    }
    (void)p;
  }
//  else
//  if (C<Sm::LevelResolvedNamespace> t = dynamic_cast<Sm::LevelResolvedNamespace*>(p)) {
//    Sm::LevelResolvedNamespace *p = t.object();
//    if (p->size() > 0) {
//      Sm::LevelResolvedNamespace::value_type v =  *(p->begin());
//      (void)v;
//    }
//  }
}

void checkSmartPtr(const Smart *ptr) {
  if (smartptrDebugStop)
    return;
  checkLevelSpace();
  if (ptr == nullptr || ptr->strongRef == std::numeric_limits<SmartCount>::max())
    throw 999;
  checkPtrConsistment<Ptr>(ptr);
}

void checkSmartPtrToMax(const Smart *ptr) {
  if (smartptrDebugStop)
    return;
  checkLevelSpace();
  if (ptr != nullptr) {
    if (ptr->strongRef == std::numeric_limits<SmartCount>::max())
      throw 999;
    checkPtrConsistment<Ptr>(ptr);
  }
}

void constructorCheckSmartPtr(const Smart *ptr) {
  if (smartptrDebugStop)
    return;
  checkLevelSpace();
  if (ptr == nullptr || ptr->strongRef == std::numeric_limits<SmartCount>::max())
    throw 999;
  checkPtrConsistment<PRef>(ptr);
}


void constructorCheckSmartPtrToMax(const Smart *ptr) {
  if (smartptrDebugStop)
    return;
  checkLevelSpace();
  if (ptr != nullptr) {
    if (ptr->strongRef == std::numeric_limits<SmartCount>::max())
      throw 999;
    checkPtrConsistment<PRef>(ptr);
  }
}

}
#endif


#ifdef SMARTPTR_DEBUG


using namespace std;

using namespace Sm;
using namespace BackportHashMap;
using namespace grouping_sets;

std::set<Sm::Id*> idManagerMap;
IdManager::~IdManager() {
  for (const Sm::Id* n : idManagerMap) {
//    cout << n->getText() << " : " << n->getLLoc().begin.line << "."  << n->getLLoc().begin.column << endl;
  }
}

NumericSimpleIntManager::~NumericSimpleIntManager() {
  for (const Sm::NumericSimpleInt* n : managerMap) {
//    cout << n->simpleInt << " : " << n->getLLoc().begin.line << "."  << n->getLLoc().begin.column << endl;
  }
}

SemanticTreeManager::~SemanticTreeManager() {
  for (const Sm::SemanticTree* n : managerMap) {
//    cout << n->__streeId__ << endl; // << ":" << ((Sm::SemanticTree*)n)->toVerboseString() << endl;
  }
}

void SemanticTreeManager::add(Sm::SemanticTree */*n*/) {
//  n->__streeId__ = streeCnt++;
//  managerMap.insert(n);
//  if (n->__streeId__ == 3983)
//    cout << 1;
}


NumericSimpleIntManager numericSimpleIntManager;
IdManager idManager;
SemanticTreeManager streeManager;

namespace smart {
void checkSmartPointer(Smart */*ptr*/) {
//  if (ResolvedEntity *t = dynamic_cast<ResolvedEntity*>(basePtr)) {
//    if (Sm::SemanticTree *node = t->getSemanticNode()) {
//      if (Ptr<Sm::Id> name = node->declarationName()) {
//       lastLine = name->getLLoc().begin.line;
//       if (name->beginedFrom(91488, 34))
//        std::cout << 11 << std::endl;
//      }
//    }
//  }
}

//void checkSmartPointer(SingleSmart */*ptr*/) {
//  if (ResolvedEntity *t = dynamic_cast<ResolvedEntity*>(basePtr)) {
//    if (Sm::SemanticTree *node = t->getSemanticNode()) {
//      if (Ptr<Sm::Id> name = node->declarationName()) {
//       lastLine = name->getLLoc().begin.line;
//       if (name->beginedFrom(91488, 34))
//        std::cout << 11 << std::endl;
//      }
//    }
//  }
//}
}


using namespace std;

class TestIdLeaks {
public:
  typedef std::unordered_set<const TestLeakType*> IdSet;
  IdSet idSet;

  ~TestIdLeaks() {
    for ( IdSet::key_type i : idSet)
      if (i) {
        cl::location l =  i->getLLoc();
        l.begin.filename = 0;
        l.end.filename = 0;
        std::cout << std::endl << l << ": " << std::endl;
      }
  }
};
static TestIdLeaks testIdLeaks;

void pushToTestLeaks(TestIdLeaks::IdSet::key_type p) {
  testIdLeaks.idSet.insert(p);
}

void popFromTestLeaks(TestIdLeaks::IdSet::key_type p) {
  testIdLeaks.idSet.erase(p);
}




namespace smart {
int lastLine = 0;
}

using namespace std;

class ClassNotFoundException : public exception {};

extern MaximalSizes maximalSizes;
extern unordered_map<const type_info*, int> Register_v;

class Register
{
public:
  template <class T>
  static void reg (T* = NULL)
  {
    //  could add other qualifiers
    Register_v[&typeid(T)] = sizeof(T);
    Register_v[&typeid(const T)] = sizeof(T);
    Register_v[&typeid(T*)] = sizeof(T);
    Register_v[&typeid(const T*)] = sizeof(T);
  }

  template <class T>
  static int getSize (const T& x)
  {
    const type_info* id = &typeid(x);
    if( Register_v.find(id) == Register_v.end() ){
      throw ClassNotFoundException();
    }
    return Register_v[id];
  }

  template <class T>
  static int getSize (T* x)
  {
    return getSize(*x);
  }

  template <class T>
  static int getSize (const T* x)
  {
    return getSize(*x);
  }
  Register();
  ~Register();
};


Register classesRegistrator;


Register::Register() {
  Register::reg<Type::collection_methods::AccessToItem>();
  Register::reg<alter_table::AddConstraints>();
  Register::reg<alter_table::AddFields>();
  Register::reg<alter_table::AddRefConstraint>();
  Register::reg<AlgebraicCompound>();
  Register::reg<AllowedMasks>();
  Register::reg<Sm::alter_table::AlterFieldsBase>();
  Register::reg<AlterTable>();
  Register::reg<AlterTableCommand>();
  Register::reg<AlterUser>();
  Register::reg<AnalyticFun>();
  Register::reg<ArgumentNameRef>();
  Register::reg<Type::ArrayConstructor>();
  Register::reg<Sm::FlashbackQueryClause::AsOf>();
  Register::reg<Assignment>();
  Register::reg<AsteriskExpr>();
  Register::reg<constraint::Attribute>();
  Register::reg<pragma::AutonomousTransaction>();
  Register::reg<pl_expr::Between>();
  Register::reg<BlockPlSql>();
  Register::reg<BooleanLiteral>();
  Register::reg<Sm::pl_expr::ComparsionList::BracketedPlExprList>();
  Register::reg<pl_expr::Brackets>();
  Register::reg<Brackets>();
  Register::reg<s::CInlCmd>();
  Register::reg<s::CMultiLineComment>();
  Register::reg<Case>();
  Register::reg<CaseStatement>();
  Register::reg<Cast>();
  Register::reg<CastMultiset>();
  Register::reg<constraint::CheckCondition>();
  Register::reg<Close>();
  Register::reg<Codestream>();
  Register::reg<CodestreamStack>();
  Register::reg<CodestreamStackItem>();
  Register::reg<CodestreamState>();
  Register::reg<CollectSemanticInterface>();
  Register::reg<CollectionAccess>();
  Register::reg<Type::collection_methods::CollectionMethod>();
  Register::reg<Type::CollectionType>();
  Register::reg<s::Comment>();
  Register::reg<Commit>();
  Register::reg<pl_expr::Comparsion>();
  Register::reg<pl_expr::ComparsionList>();
  Register::reg<alter_user::Connect>();
  Register::reg<Constraint>();
  Register::reg<Sm::constraint::ConstraintState>();
  Register::reg<Type::collection_methods::Count>();
  Register::reg<s::Cquote>();
  Register::reg<Cube>();
  Register::reg<pl_expr::CurrentOf>();
  Register::reg<Sm::Cursor>();
  Register::reg<Sm::CursorExpr>();
  Register::reg<CursorParameter>();
  Register::reg<CursorProperties>();
  Register::reg<DatabaseLink>();
  Register::reg<Datatype>();
  Register::reg<Decl>();
  Register::reg<Declaration>();
  Register::reg<Def>();
  Register::reg<DefR>();
  Register::reg<DefaultExpr>();
  Register::reg<Type::collection_methods::Delete>();
  Register::reg<DeleteFrom>();
  Register::reg<s::DisableIndenting>();
  Register::reg<trigger::DmlEvents>();
  Register::reg<GlobalDatatype::DmpType>();
  Register::reg<alter_table::DropConstraint>();
  Register::reg<alter_table::DropFields>();
  Register::reg<alter_table::DropKey>();
  Register::reg<EmptyIdExpr>();
  Register::reg<table::EnableDisableConstraint>();
  Register::reg<s::EnableIndenting>();
  Register::reg<table::EncryptSpec>();
  Register::reg<s::Endl>();
  Register::reg<BasicEntityAttributes>();
  Register::reg<EntityAttributes>();
  Register::reg<ChangedQueryEntityRef>();
  Register::reg<EquallyEntities>();
  Register::reg<dmperr::ErrorCodes>();
  Register::reg<dmperr::ErrorInfo>();
  Register::reg<trigger::DmlEvents::Event>();
  Register::reg<Exception>();
  Register::reg<pragma::ExceptionInit>();
  Register::reg<ExecuteImmediate>();
  Register::reg<pl_expr::Exists>();
  Register::reg<Type::collection_methods::Exists>();
  Register::reg<Exit>();
  Register::reg<Type::collection_methods::Extend>();
  Register::reg<ExtractExpr>();
  Register::reg<FactoringItem>();
  Register::reg<Fetch>();
  Register::reg<merge::FieldAssignment>();
  Register::reg<table::FieldDefinition>();
  Register::reg<update::FieldFromExpr>();
  Register::reg<table::field_property::FieldProperty>();
  Register::reg<update::FieldsFromSubquery>();
  Register::reg<Type::collection_methods::First>();
  Register::reg<FlashbackQueryClause>();
  Register::reg<GlobalDatatype::SysTypeInfo::FldType>();
  Register::reg<ForAll>();
  Register::reg<ForOfExpression>();
  Register::reg<ForOfRange>();
  Register::reg<ForUpdateClause>();
  Register::reg<constraint::ForeignKey>();
  Register::reg<From>();
  Register::reg<FromJoin>();
  Register::reg<FromSingle>();
  Register::reg<trigger::Funcall>();
  Register::reg<Function>();
  Register::reg<FunctionArgument>();
  Register::reg<Sm::FunctionCall>();
  Register::reg<FunCallArg>();
  Register::reg<GlobalDatatype::FundamentalDatatype>();
  Register::reg<GlobalDatatype::GlobalDatatype>();
  Register::reg<GlobalFunctions>();
  Register::reg<Goto>();
  Register::reg<Grammar>();
  Register::reg<GrammarBaseSmart>();
  Register::reg<GrammarBase>();
  Register::reg<GroupBy>();
  Register::reg<GroupByRollupCubes>();
  Register::reg<GroupBySqlExpr>();
  Register::reg<HString>();
  Register::reg<GlobalDatatype::HasImplicitConversion>();
  Register::reg<HierarhicalClause>();
  Register::reg<HostCursorPropertiesExpr>();
  Register::reg<RefHostExpr>();
  Register::reg<Id>();
  Register::reg<Id2>();
  Register::reg<IdEntity>();
  Register::reg<IdEntitySmart>();
  Register::reg<If>();
  Register::reg<Case::IfThen>();
  Register::reg<Index>();
  Register::reg<pragma::Inline>();
  Register::reg<Insert>();
  Register::reg<insert::MultipleConditionalInsert::InsertConditional>();
  Register::reg<insert::InsertFrom>();
  Register::reg<insert::InsertFromSubquery>();
  Register::reg<insert::InsertFromValues>();
  Register::reg<InsertInterface>();
  Register::reg<insert::InsertValues>();
  Register::reg<Sm::insert::MultipleConditionalInsert::InsertWhenThen>();
  Register::reg<insert::InsertingExpressionListValues>();
  Register::reg<insert::InsertingRecordValues>();
  Register::reg<insert::InsertingValues>();
  Register::reg<Sm::TimeExpr::Interval>();
  Register::reg<insert::Into>();
  Register::reg<pl_expr::IsEmpty>();
  Register::reg<pl_expr::IsInfinite>();
  Register::reg<pl_expr::IsNan>();
  Register::reg<pl_expr::IsNull>();
  Register::reg<Type::JavaExternalSpec>();
  Register::reg<Join>();
  Register::reg<JoinCondition>();
  Register::reg<JoinOnDefault>();
  Register::reg<JoinOnFieldList>();
  Register::reg<alter_table::KeyFields>();
  Register::reg<LValue>();
  Register::reg<Label>();
  Register::reg<Type::collection_methods::Last>();
  Register::reg<LevelResolvedNamespace>();
  Register::reg<pl_expr::Like>();
  Register::reg<Type::collection_methods::Limit>();
  Register::reg<LinterCursor>();
  Register::reg<LinterCursorField>();
  Register::reg<LinterWriter>();
  Register::reg<List<Constraint>>();
  Register::reg<List<Id2>>();
  Register::reg<List<Id>>();
  Register::reg<LockTable>();
  Register::reg<pl_expr::LogicalCompound>();
  Register::reg<Loop>();
  Register::reg<LoopBounds>();
  Register::reg<alter_table::ManipulateFields>();
  Register::reg<merge::MatchedUpdate>();
  Register::reg<Type::MemberFunction>();
  Register::reg<Type::MemberInterface>();
  Register::reg<pl_expr::MemberOf>();
  Register::reg<Type::MemberVariable>();
  Register::reg<Merge>();
  Register::reg<ModelActions>();
  Register::reg<ModelContext>();
  Register::reg<alter_table::ModifyConstraint>();
  Register::reg<alter_table::ModifyFields>();
  Register::reg<alter_table::ModifyKey>();
  Register::reg<insert::MultipleConditionalInsert>();
  Register::reg<insert::MultipleValuesInsert>();
  Register::reg<dstring::NString>();
  Register::reg<s::Name>();
  Register::reg<table::field_property::NestedTable::NestedName>();
  Register::reg<Type::NestedTable>();
  Register::reg<NewCall>();
  Register::reg<Type::collection_methods::Next>();
  Register::reg<trigger::NonDmlEvents>();
  Register::reg<merge::NotMatchedInsert>();
  Register::reg<constraint::NotNull>();
  Register::reg<NullExpr>();
  Register::reg<NullStatement>();
  Register::reg<GlobalDatatype::NumberDatatype>();
  Register::reg<NumericFloat>();
  Register::reg<NumericInt>();
  Register::reg<NumericSimpleInt>();
  Register::reg<NumericValue>();
  Register::reg<s::OInlCmd>();
  Register::reg<s::OMultiLineComment>();
  Register::reg<Type::Object>();
  Register::reg<table::field_property::ObjectField>();
  Register::reg<table::field_property::ObjectProperties>();
  Register::reg<view::ObjectReference>();
  Register::reg<Type::ObjectType>();
  Register::reg<Sm::pl_expr::OfTypes::OfType>();
  Register::reg<pl_expr::OfTypes>();
  Register::reg<table::OidIndex>();
  Register::reg<OpenCursor>();
  Register::reg<OpenFor>();
  Register::reg<s::Oquote>();
  Register::reg<GlobalFunctions::OraTemplateData>();
  Register::reg<OrderBy>();
  Register::reg<Sm::OrderBy::OrderByItem>();
  Register::reg<OutherJoinExpr>();
  Register::reg<Package>();
  Register::reg<s::PairSpacer>();
  Register::reg<s::PairSpacerClose>();
  Register::reg<s::PairSpacerOpen>();
  Register::reg<PartitialResolvedFunction>();
  Register::reg<pl_expr::Path>();
  Register::reg<Sm::table::field_property::PhysicalProperties>();
  Register::reg<PipeRow>();
  Register::reg<PlExpr>();
  Register::reg<pragma::Pragma>();
  Register::reg<constraint::PrimaryKey>();
  Register::reg<Type::collection_methods::PriorExpr>();
  Register::reg<Proc>();
  Register::reg<QueriedTable>();
  Register::reg<QueryBlock>();
  Register::reg<QueryPseudoField>();
  Register::reg<ChangedQueryRestricted>();
  Register::reg<Raise>();
  Register::reg<Type::Record>();
  Register::reg<Type::RecordField>();
  Register::reg<Ref1>();
  Register::reg<Type::RefCursor>();
  Register::reg<Sm::view::XmlReference::Reference>();
  Register::reg<constraint::ReferencedKey>();
  Register::reg<Sm::trigger::DmlEvents::Referencing>();
  Register::reg<pl_expr::RegexpLike>();
  Register::reg<alter_table::RenameField>();
  Register::reg<alter_table::RenameTable>();
  Register::reg<ResolvedEntity>();
  Register::reg<ResolvedEntityLoc>();
  Register::reg<ResolvedEntitySNode>();
  Register::reg<ResolvingContext>();
  Register::reg<pragma::RestrictReferences>();
  Register::reg<Return>();
  Register::reg<ReturnInto>();
  Register::reg<Rollback>();
  Register::reg<Rollup>();
  Register::reg<RowIdExpr>();
  Register::reg<constraint::RowMovement>();
  Register::reg<RowNumExpr>();
  Register::reg<Savepoint>();
  Register::reg<SelectBrackets>();
  Register::reg<SelectList>();
  Register::reg<STreeNode>();
  Register::reg<SelectSingle>();
  Register::reg<SelectStatement>();
  Register::reg<SelectedField>();
  Register::reg<SemanticInterface>();
  Register::reg<SemanticInterfaceBase>();
  Register::reg<SemanticTree>();
  Register::reg<s::Semicolon>();
  Register::reg<Sequence>();
  Register::reg<SequenceBody>();
  Register::reg<SequencePseudocolumn>();
  Register::reg<pragma::SeriallyReusable>();
  Register::reg<update::SetClause>();
  Register::reg<update::SetRowRecord>();
  Register::reg<update::SetUpdatingList>();
  Register::reg<RefExpr>();
  Register::reg<Single>();
  Register::reg<insert::SingleInsert>();
  Register::reg<SingleSmart>();
  Register::reg<Smart>();
  Register::reg<Spacer>();
  Register::reg<Sql>();
  Register::reg<BulkRowcountExpr>();
  Register::reg<ChangedQueryEntity>();
  Register::reg<SqlExpr>();
  Register::reg<RefAbstract>();
  Register::reg<SqlStatement>();
  Register::reg<SqlStatementInterface>();
  Register::reg<Statement>();
  Register::reg<StatementInterface>();
  Register::reg<StatementWithLabelPrefixedList>();
  Register::reg<dstring::String>();
  Register::reg<GlobalDatatype::StringDatatype>();
  Register::reg<s::Stub>();
  Register::reg<pl_expr::Submultiset>();
  Register::reg<Subquery>();
  Register::reg<table::SubstitutableProperty>();
  Register::reg<Subtype>();
  Register::reg<Synonym>();
  Register::reg<SyntaxerContext>();
  Register::reg<SysDefCounters>();
  Register::reg<Sm::SysDual>();
  Register::reg<Sm::SysRefcursor>();
  Register::reg<GlobalDatatype::SysTypeInfo>();
  Register::reg<SystemTable>();
  Register::reg<Table>();
  Register::reg<ChangedQueryTableCollectionExpr>();
  Register::reg<FromTableDynamic>();
  Register::reg<table::TableProperties>();
  Register::reg<FromTableReference>();
  Register::reg<FromTableSubquery>();
  Register::reg<Tablesample>();
  Register::reg<TestIdLeaks>();
  Register::reg<s::TextChunk>();
  Register::reg<TimeExpr>();
  Register::reg<Sm::TimeExpr::Timezone>();
  Register::reg<Tok2string>();
  Register::reg<Transaction>();
  Register::reg<TranslatedName>();
  Register::reg<Translator>();
  Register::reg<Trigger>();
  Register::reg<trigger::TriggerAbstractRowReference>();
  Register::reg<trigger::TriggerAction>();
  Register::reg<trigger::TriggerActionInterface>();
  Register::reg<trigger::TriggerCode>();
  Register::reg<trigger::TriggerEvents>();
  Register::reg<trigger::TriggerNestedRowReference>();
  Register::reg<trigger::TriggerPredicateVariable>();
  Register::reg<trigger::TriggerRowReference>();
  Register::reg<Type::collection_methods::Trim>();
  Register::reg<TrimFromExpr>();
  Register::reg<UnaryMinus>();
  Register::reg<UnaryPlus>();
  Register::reg<UnionQuery>();
  Register::reg<constraint::Unique>();
  Register::reg<Update>();
  Register::reg<Sm::DatabaseLink::UserAuthentication>();
  Register::reg<UserContext>();
  Register::reg<alter_user::UserProxyConnect>();
  Register::reg<alter_user::UserRole>();
  Register::reg<alter_user::UserRoles>();
  Register::reg<alter_user::UserSettings>();
  Register::reg<VEntities>();
  Register::reg<Variable>();
  Register::reg<VariableUndeclaredIndex>();
  Register::reg<Type::Varray>();
  Register::reg<table::field_property::VarrayField>();
  Register::reg<Sm::FlashbackQueryClause::VersionBetween>();
  Register::reg<View>();
  Register::reg<view::ViewConstraint>();
  Register::reg<view::ViewConstraints>();
  Register::reg<view::ViewProperties>();
  Register::reg<view::ViewQRestriction>();
  Register::reg<WhenExpr>();
  Register::reg<WhereClause>();
  Register::reg<While>();
  Register::reg<table::field_property::XmlField>();
  Register::reg<view::XmlReference>();
  Register::reg<s::cgroup>();
  Register::reg<s::comma>();
  Register::reg<commalevel>();
  Register::reg<dectab>();
  Register::reg<dynamic_hashed_cchar>();
  Register::reg<s::endsavetab>();
  Register::reg<s::iloc>();
  Register::reg<inctab>();
  Register::reg<itostr_helper>();
  Register::reg<s::loc>();
  Register::reg<cl::location>();
  Register::reg<s::ogroup>();
  Register::reg<yy::position>();
  Register::reg<s::procendl>();
  Register::reg<sized_cchar_base>();
  Register::reg<skip>();
  Register::reg<static_hashed_cchar>();
  Register::reg<s::subconstruct>();
  Register::reg<s::tab>();
  Register::reg<temporary_hashed_cchar>();

}



void createContainerIfNotExists(const std::type_info& t, std::pair<MaximalSizes::iterator, bool> it)
{
  if (it.second) {
    it.first->second = new DataOfTypes();
    it.first->second->name  = t.name();
    std::unordered_map<const type_info*, int>::iterator szit = Register_v.find(&t);
    if (szit != Register_v.end())
      it.first->second->size  = szit->second;
    else
      it.first->second->size = 0;
    it.first->second->maxItems = 0;
  }
}

void addSmartptr(const smart::Smart *ptr)
{
  try {
    if (ptr) {
      const std::type_info& t = typeid(*ptr);
      std::pair<MaximalSizes::iterator, bool> it = maximalSizes.insert(MaximalSizes::value_type(t.hash_code(), 0));
      createContainerIfNotExists(t, it);
      it.first->second->instances.insert(ptr);
      if (it.first->second->maxItems < it.first->second->instances.size())
        it.first->second->maxItems = it.first->second->instances.size();
    }
  }
  catch (bad_typeid) { }
}

void eraseSmartptr(const smart::Smart *ptr)
{
  try {
    if (ptr) {
      const std::type_info& t = typeid(*ptr);
      std::pair<MaximalSizes::iterator, bool> it = maximalSizes.insert(MaximalSizes::value_type(t.hash_code(), 0));
      createContainerIfNotExists(t, it);
      it.first->second->instances.erase(ptr);
    }
  }
  catch (bad_typeid) {}
}



Smart::Smart() : strongRef(0)           {
  __flags__.v = 0;
}
Smart::Smart(const Smart &o) : strongRef(o.strongRef) /*weakRef(o.weakRef), isDeleted__(0),  offsetThisPointer__(0) */ {
  __flags__.v = o.__flags__.v;
  addSmartptr(this);
}

Smart::~Smart() {
  strongRef = std::numeric_limits<SmartCount>::max();
  eraseSmartptr(this);
}

Register::~Register() {
  std::map<size_t, string> classes;
  for (MaximalSizes::value_type & it : maximalSizes)
    classes[it.second->maxItems * (it.second->size ? it.second->size : 1)] = it.second->name;

  for (std::map<size_t, string>::value_type &mapIt : classes)
    cout << mapIt.first << " : " << mapIt.second << endl;
}



void smartDoAction(const smart::Smart *ptr, bool isInc) {
  static int cnt = 0;
  if (!ptr)
    return;

  if (isInc && ptr->strongRef < 2)
    addSmartptr(ptr);
//  if (Sm::SemanticTree *p = dynamic_cast<Sm::SemanticTree*>((smart::Smart*)ptr))
  {

//    if (p->__streeId__ == 3983)
//      cnt++;
//    if (p->beginedFrom(1896,20))
//      cnt++;
//    if (p->beginedFrom(246,8) && cnt == 9)
//      cout << cnt++ << endl;
  }
}


#endif



