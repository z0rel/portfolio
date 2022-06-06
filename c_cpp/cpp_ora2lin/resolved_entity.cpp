#include <stdlib.h>
#include "syntaxer_context.h"
#include "resolvers.h"
#include "siter.h"
#include "i2str.h"
#include "syntaxer_context.h"
#include "semantic_datatype.h"
#include "semantic_function.h"
#include "usercontext.h"
#include "model_context.h"
#include "semantic_blockplsql.h"

using namespace std;
using namespace Sm;
using namespace PlsqlHelper;

extern SyntaxerContext syntaxerContext;
extern bool __STreeDestructionStarted__;


std::string CathegoriesOfDefinitions::ddlCathegoryToString(ScopedEntities t) {
  switch (t) {
    case SpecialKeysActor_           : return "SpecialKeysActor";
    case SelectList_                 : return "SelectList";
    case LAST_ENTITY_NUMBER          : return "LAST_ENTITY_NUMBER";
    case DatatypeRowtype_            : return "DatatypeRowtype";
    case DatatypeType_               : return "DatatypeType";
    case DatatypeIsRef_              : return "DatatypeIsRef";
    case DatatypeDefault_            : return "DatatypeDefault";
    case DatatypeNull_               : return "DatatypeNull";
    case TailObj_                    : return "TailObj";
    case UNRESOLVED__                : return "UNRESOLVED";
    case ROOT__                      : return "ROOT";
    case LValue_                     : return "LValue";
    case SqlExprSqlCursorProperties_ : return "SqlCursorProperties";
    case AnydataObject_              : return "Anydata";
    case LinterCursorField_          : return "LinterCursorField";
    case BooleanLiteral_             : return "BooleanLiteral";
    case LinterCursor_               : return "LinterCursor_";
    case ArrayConstructor_           : return "ArrayConstructor";
    case TriggerPredicateVariable_   : return "TriggerPredicateVariable";
    case VariableUndeclaredIndex_    : return "VariableUndeclaredIndex";
    case SqlSelectedField_           : return "SqlSelectedField";
    case Label_                      : return "Label_";
    case RefExpr_                    : return "RefExpr";
    case SqlExprNull_                : return "SqlExprNull";
    case SqlExprEmptyId_             : return "SqlExprEmptyId";
    case QueriedPseudoField_         : return "QueriedPseudoField";
    case AnalyticFun_                : return "AnalyticFun";
    case SqlExprPrior_               : return "SqlExprPrior";
    case SqlExprCursor_              : return "SqlExprCursor";
    case SqlExprBrackets_            : return "SqlExprBrackets";
    case SqlExprUnaryPlus_           : return "SqlExprUnaryPlus";
    case SqlExprUnaryMinus_          : return "SqlExprUnaryMinus";
    case AlgebraicCompound_          : return "AlgebraicCompound";
    case NumericSimpleInt_           : return "NumericSimpleInt";
    case NumericIntVal_              : return "NumericIntVal";
    case SqlExprDefault_             : return "SqlExprDefault";
    case TimeExpr_                   : return "TimeExpr";
    case RowId_                      : return "RowId";
    case RowNum_                     : return "RowNumExpr";
    case SqlExprCase_                : return "SqlExprCase";
    case SqlExprNewCall_             : return "SqlExprNewCall";
    case SqlExprOutherJoin_          : return "SqlExprOutherJoin";
    case SqlExprAsterisk_            : return "SqlExprAsterisk";
    case AsteriskRefExpr_            : return "AsteriskRefExpr";
    case SqlExprCursorProperties_    : return "SqlExprCursorProperties";
    case SqlExprHostCursorProperties_: return "SqlExprHostCursorProperties";
    case RefHostExpr_                : return "RefHostExpr";
    case SqlBulkRowcount_            : return "BulkRowcountExpr";
    case SqlExprCast_                : return "SqlExprCast";
    case SqlExprCastMultiset_        : return "SqlExprCastMultiset";
    case SqlExprSelectSingle_        : return "SqlExprSelectSingle";
    case SqlExprSelectBrackets_      : return "SqlExprSelectBrackets";
    case SqlExprUnion_               : return "SqlExprUnion";
    case NumericFloatVal_            : return "NumericFloatVal";
    case ExtractFrom_                : return "ExtractFrom";
    case TrimFrom_                   : return "TrimFrom";
    case SequencePseudocolumn_       : return "SequencePseudocolumn";
    case TriggerRowReference_        : return "TriggerRowReference";
    case TriggerNestedRowReference_  : return "TriggerNestedReference";
    case ModelContext_               : return "ModelContext";
    case Datatype_                   : return "Datatype";
    case FactoringItem_              : return "FactoringItem";
    case FromSingle_                 : return "FromSingle";
    case FromJoin_                   : return "FromJoin";
    case DatabaseLink_               : return "DatabaseLink";
    case EMPTY__                     : return "EMPTY";
    case Table_                      : return "Table";
    case View_                       : return "View";
    case Sequence_                   : return "Sequence";
    case CollectionMethod_           : return "CollectionMethod";
    case Synonym_                    : return "Synonym";
    case Function_                   : return "Function";
    case Package_                    : return "Package";
    case Index_                      : return "Index";
    case IndexUnique_                : return "IndexUnique";
    case Trigger_                    : return "Trigger";
    case User_                       : return "User";
    case ArgVarDeclaration_          : return "ArgVarDeclaration";
    case BlockPlSql_                 : return "BlockPlSql";
    case CaseStatement_              : return "CaseStatement";
    case Cursor_                     : return "Cursor";
    case CursorParameter_            : return "CursorParameter";
    case Exception_                  : return "Exception";
    case ExceptionPragma_            : return "ExceptionPragma";
    case FieldOfRecord_              : return "FieldOfRecord";
    case FieldOfTable_               : return "FieldOfTable";
    case ForAll_                     : return "ForAll";
    case ForOfExpression_            : return "ForOfExpression";
    case ForOfRange_                 : return "ForOfRange";
    case FunctionArgument_           : return "FunctionArgument";
    case FunctionCallArgument_       : return "FunCallArg";
    case ListConstraint_             : return "ListConstraint";
    case Loop_                       : return "Loop";
    case MemberFunction_             : return "MemberFunction";
    case MemberVariable_             : return "MemberVariable";
    case QueriedTable_               : return "QueriedTable";
    case RenameTable_                : return "RenameTable";
    case Savepoint_                  : return "Savepoint";
    case SqlEntity_                  : return "ChangedQueryEntity";
    case SqlExpr_                    : return "SqlExpr";
    case Transaction_                : return "Transaction";
    case TriggerDmlReferencing_      : return "TriggerDmlReferencing";
    case Variable_                   : return "Variable";
    case FieldOfVariable_            : return "FieldOfVariable";
    case XmlReference_               : return "XmlReference";
    case Record_                     : return "Record";
    case RefCursor_                  : return "RefCursor";
    case Varray_                     : return "Varray";
    case NestedTable_                : return "NestedTable";
    case Merge_                      : return "Merge";
    case Subtype_                    : return "Subtype";
    case FundamentalDatatype_        : return "FundamentalDatatype";
    case Object_                     : return "Object";
    case QueryBlock_                 : return "QueryBlock";
    case QueryHint_                  : return "QueryHint_";
    case ReturnInto_                 : return "ReturnInto";
    case PlExpr__                    : return "PlExpr";
    case Commit_                     : return "Commit";
    case DeleteFrom_                 : return "DeleteFrom";
    case LockTable_                  : return "DeleteTable";
    case Rollback_                   : return "Rollback";
    case SelectStatement_            : return "SelectStatement";
    case Update_                     : return "Update";
    case CursorFieldDecltype_        : return "CursorFieldDecltype";
    case MultipleValuesInsert_       : return "MultipleValuesInsert";
    case MultipleConditionalInsert_  : return "MultipleConditionalInsert";
    case Assignment_                 : return "Assignment";
    case Close_                      : return "Close";
    case ConstructExprStmt_          : return "ConstructExprStmt";
    case CursorDecltype_             : return "CursorDecltype";
    case DeclNamespace_              : return "DeclNamespace";
    case ExecuteImmediate_           : return "ExecuteImmediate";
    case Exit_                       : return "Exit";
    case Fetch_                      : return "Fetch";
    case FunctionCall_               : return "FunctionCall";
    case Goto_                       : return "Goto";
    case If_                         : return "If";
    case NullStatement_              : return "NullStatement";
    case OpenCursor_                 : return "OpenCursor";
    case PipeRow_                    : return "PipeRow";
    case Raise_                      : return "Raise";
    case Resignal_                   : return "Resignal";
    case Return_                     : return "Return";
    case WhenExpr_                   : return "WhenExpr";
    case While_                      : return "While";
    case Comparsion_                 : return "Comparsion";
    case LogicalCompound_            : return "LogicalCompound";
    case StatementsContainer_        : return "StatementsContainer";
    case SingleInsert_               : return "SingleInsert";
    case OpenFor_                    : return "OpenFor";
  }
  return "";
}

std::string Sm::toString(ResolvedEntity::ScopedEntities t) { return CathegoriesOfDefinitions::ddlCathegoryToString(t); }
std::string ResolvedEntity::ddlCathegoryToString() const { return CathegoriesOfDefinitions::ddlCathegoryToString(ddlCathegory()); }

DepArcContext::DepArcContext() {

}

DepArcContext::DepArcContext(uint32_t val) : value(val) {

}

DepArcContext::DepArcContext(const DepArcContext &oth)
  : value(oth.value), references(oth.references) {}

size_t Sm::DepArcContext::getEid() {
  static size_t cnt = 0;
  ++cnt;
  return cnt;
}

DepArcContext& DepArcContext::operator|=(const DepArcContext &oth) {
  value |= oth.value;
  references.insert(oth.references.begin(), oth.references.end());
  return *this;
}

Sm::IsSubtypeValues ResolvedEntity::isSubtype(ResolvedEntity *supertype, bool plContext) const {
 // дефолт для RefCursor, Record, etc...
 if (supertype) {
   if (supertype->ddlCathegory() == Subtype_) {
     if (Ptr<Datatype> t = supertype->getDatatype())
       return isSubtype(t, plContext);
     else
       return EXPLICIT;
   }
   if (ddlCathegory() == supertype->ddlCathegory() && eqByVEntities(supertype))
     return EXACTLY_EQUALLY;
   throw 999;
   if (eqByFields(supertype, plContext, false))
     return IMPLICIT_CAST_BY_FIELDS;
 }
 return EXPLICIT;
}

bool compareFields(Sm::EntityFields &a, Sm::EntityFields &b, bool isPlContext, bool inSqlCode)
{
  if (b.empty() || a.size() != b.size())
    return false;

  EntityFields::iterator bIt  = b.begin();
  EntityFields::iterator aIt  = a.begin();
  EntityFields::iterator bEnd = b.end();

  for (; bIt != bEnd; ++bIt, ++aIt) {
    EntityFields::value_type &aFld = *aIt;
    EntityFields::value_type &bFld = *bIt;
    if (aFld && bFld) {
      ResolvedEntity *aFldDef = aFld->definition();
      ResolvedEntity *bFldDef = bFld->definition();
      if (aFldDef && bFldDef) {
        if (aFldDef == bFldDef)
          continue;
        Ptr<Datatype> aFldT = SubqueryUnwrapper::unwrap(aFldDef->getDatatype());
        Ptr<Datatype> bFldT = SubqueryUnwrapper::unwrap(bFldDef->getDatatype());
        if (aFldT && bFldT) {
          if (aFldT == bFldT)
            continue;
          CastCathegory cat = aFldT->getCastCathegory(bFldT, isPlContext, inSqlCode);
          if (cat.implicitAlmost())
            continue;
        }
        else if (aFldDef == bFldDef)
          continue;
      }
      else if (aFld == bFld)
        continue;
    }
    return false;
  }
  return true;
}

bool ResolvedEntity::eqByFields(ResolvedEntity *supertype, bool isPlContext, bool inSqlCode) const {
 if (!supertype)
   return 0;
 EntityFields subTFields;
 EntityFields superTFields;
 supertype->getFields(superTFields);
 getFields(subTFields);

 return compareFields(superTFields, subTFields, isPlContext, inSqlCode);
}

bool ResolvedEntity::getFields(EntityFields &fields) const {
  if (Ptr<Sm::Datatype> t = getDatatype())
    return t->getFields(fields);
  return false;
}

bool ResolvedEntity::getFieldsExp(EntityFields &fields, bool isProc, const std::string &baseName/* = std::string()*/) const {
  EntityFields interFields;
  bool res = getFields(interFields);

  for (Ptr<Id> &id : interFields) {
    Ptr<Datatype> t = getLastConcreteDatatype(id->getDatatype());
    if (t && (t->isRowTypeOf() || t->isRecordType())) {
      string newBase = baseName + id->toString() + (isProc ? "_" : "$");
      id->definition()->getFieldsExp(fields, isProc, newBase);
    }
    else if (!baseName.empty()) {
      Ptr<Id> newId = new Id(baseName + id->toString(), id->definition());
      if (id->hasSpecSymbols())
        newId->setSpecSymbols();
      if (id->quoted())
         newId->setQuoted();
      fields.push_back(newId);
    }
    else
      fields.push_back(id);
  }
  return res;
}

bool ResolvedEntity::isElementaryLiteral() const {
  return isNumericLiteral();
}

bool ResolvedEntity::isElementaryType() const {
  return isSmallint() || isInt() || isBigint() || isDouble() || isReal() ||
      isDecimal() || isDateDatatype() || isClobDatatype() || isVarcharDatatype() ||
      isCharDatatype() || isNumberDatatype();
}

bool ResolvedEntity::isSystemPartDBMS() const {

  if (isElementaryLiteral() || isLinterQueryLiteral() || isSystem() || isElementaryLinterFunction() ||
      toSelfFundamentalDatatype() || toSelfNullType() || toSelfDefaultType())
    return true;

  ResolvedEntity *def;
  if (Sm::Datatype *d = toSelfDatatype()) {
    if ((d = SyntaxUnwrapper::unwrap(d)) &&
        (def = d->tidDdl()) &&
        (def->toSelfFundamentalDatatype() || def->toSelfNullType()))
      return true;

    if (d && d->tid && (def = d->tid->definition()) &&
        (def->toSelfNullType() || def->toSelfDefaultType()))
      return true;
  }

  return false;
}



ResolvedEntity* ResolvedEntity::getConcreteDatatype() const {
  ResolvedEntity *oldP = (ResolvedEntity*)this;
  ResolvedEntity *p = oldP->getNextDefinition();
  if (p &&
      (p->ddlCathegory() == ResolvedEntity::Function_ ||
       p->ddlCathegory() == ResolvedEntity::QueriedPseudoField_ ||
       p->ddlCathegory() == ResolvedEntity::SqlSelectedField_))
    p = p->getDatatype().object();
  while (p && p != oldP && p->ddlCathegory() != ResolvedEntity::Datatype_) {
    oldP = p;
    p = p->getNextDefinition();
    if (p &&
        (p->ddlCathegory() == ResolvedEntity::Function_ ||
         p->ddlCathegory() == ResolvedEntity::QueriedPseudoField_ ||
         p->ddlCathegory() == ResolvedEntity::SqlSelectedField_))
      p = p->getDatatype().object();
  }
  if (p)
    if (Datatype *dType = p->toSelfDatatype())
      if (dType->isTypeOf())
        p = dType->getConcreteDatatype();
  return (p == 0) ? oldP : p;
}

ResolvedEntity* ResolvedEntity::getConcreteDefinition() const {
  ResolvedEntity *oldP = (ResolvedEntity*)this;
  ResolvedEntity *p = oldP->getNextDefinition();
  while (p && p != oldP) {
    oldP = p;
    p = p->getNextDefinition();
  }
  return (p == 0) ? oldP : p;
}

bool Sm::SemanticTree::semanticResolve() const {
  if (unentered()) {
    const_cast<Sm::SemanticTree*>(this)->resolveReference();
    return true;
  }
  else if (childsNotResolved()) {
    const_cast<Sm::SemanticTree*>(this)->resolveCurrent();
    return true;
  }
  return false;
}

bool ResolvedEntity::semanticResolveBase() const {
  if (SemanticTree *t = getSemanticNode())
     return t->semanticResolve();
  return false;
}

bool Sm::FunCallArg::semanticResolve() const {
  if (argname())
    if (SemanticTree *aNode = argname()->semanticNode())
      aNode->semanticResolve();
  if (expr())
    expr()->semanticResolve();
  return semanticResolveBase();
}

bool ResolvedEntity::semanticResolve() const { return semanticResolveBase(); }

ResolvedEntity* ResolvedEntity::__tryResolveConcreteDefinition() const {
  ResolvedEntity* def = getConcreteDefinition();
  if (!def->getNextDefinition()) // Ссылка недорезолвлена
    if (def->semanticResolve())  // Но для нее есть семантическое дерево и его попытались дорезолвить.
      def = getConcreteDefinition();
  return def;
}


void  ResolvedEntity::updateNameDefinition() {
  if (Ptr<Id> name = getName())
    name->definition(this);
}


ResolvedEntity* ResolvedEntity::tryResoveConcreteDefinition() const { return __tryResolveConcreteDefinition(); }

Sm::IsSubtypeValues ResolvedEntity::isSubtypeByDatatype(Ptr<ResolvedEntity> supertype, bool plContext) const {
  if (Ptr<Datatype> t = getDatatype())
    return t->isSubtype(supertype, plContext);
  return EXPLICIT;
}

ResolvedEntity* Sm::Datatype::tryResolveConcreteDefinition() {
  if (isEverything())
    throw 999; // перед вызовом доразрешения - нужно обязательно проверять на NULL/DEFAULT - в зависимости от контекста
               // это может иметь значение "тип неопределен" или "нужно искать тип в других выражениях в той же позиции"

  return __tryResolveConcreteDefinition();
}

ResolvedEntity* ResolvedEntity::getResolvedConcreteDefinition() const {
  ResolvedEntity* p = (ResolvedEntity*)this;
  ResolvedEntity* def = p->tryResoveConcreteDefinition();
  if (!def->getNextDefinition())
    return 0;
  else return def;
}

Ptr<Sm::Datatype> ResolvedEntity::tryResolveDatatype() {
  Ptr<Datatype> t = getDatatype();
  if (!t) {
    ResolvedEntity *def = tryResoveConcreteDefinition();
    Ptr<Datatype> t = def->getDatatype();
    if (!t) {
      def->semanticResolve();
      t = def->getDatatype();
    }
  }
  if (t) {
    if (t->isNull())
      return t;
    if (t->isUnresolved())
      t->semanticResolve();
    if (ResolvedEntity *defT = t->tidDdl())
      defT->tryResoveConcreteDefinition();
  }
  return t;
}

ResolvedEntity *SemanticTree::owner() const {
  if (LevelResolvedNamespace *levelSpace = levelNamespace.object())
    if (SemanticTree *lvl = levelSpace->semanticLevel)
      return lvl->uddlEntity();
  return 0;
}

Sm::SemanticTree* ResolvedEntity::sOwner() const {
  if (SemanticTree *node = getSemanticNode())
    if (LevelResolvedNamespace *levelSpace = node->levelNamespace.object()) {
      if (node != levelSpace->semanticLevel && levelSpace->semanticLevel->ddlEntity())
        return levelSpace->semanticLevel;
      if (LevelResolvedNamespace *parentLevelNamespace = levelSpace->parent)
        return parentLevelNamespace->semanticLevel;
    }
  return 0;
}


BlockPlSql *Sm::SemanticTree::ownerBlock() {
  if (cathegory == SCathegory::BlockPlSql)
    switch (nametype) {
      case REFERENCE:
      case EXTDB_REFERENCE:
      case DATATYPE_REFERENCE:
        break;
      default:
        return unnamedDdlEntity->toSelfBlockPlSql();
    }
  else if (cathegory == SCathegory::Function &&
           nametype == NameType::DEFINITION) {
    return unnamedDdlEntity->toSelfFunction()->funBody();
  }
  if (SemanticTree *p = getParent())
    return p->ownerBlock();
  return 0;
}

Sm::BlockPlSql *ResolvedEntity::ownerPlBlock() const {
  if (Sm::SemanticTree *n = getSemanticNode())
    return n->ownerBlock();
  return NULL;
}


bool ResolvedEntity::isCodeBlockEntity() const {
  if (strongRef > 20000) {
    cout << "ERROR: isCodeBlockEntity: entity with eid=" << eid_ << "is already destroyed" << endl;
    return false;
  }

  ScopedEntities cat = ddlCathegory();
  switch (cat) {
    case ModelContext_:
    case Table_:
    case DatabaseLink_:
    case View_:
    case Sequence_:
    case CollectionMethod_:
    case Synonym_:
    case Package_:
    case Index_:
    case IndexUnique_:
    case Trigger_:
    case User_:
    case Record_:
    case RefCursor_:
    case Varray_:
    case NestedTable_:
    case Merge_:
    case Datatype_:
    case FieldOfTable_:
    case FieldOfRecord_:
    case FromSingle_:
    case FromJoin_:
    case FactoringItem_:
    case MemberVariable_:
    case Exception_:
    case SequencePseudocolumn_:
    case MemberFunction_:
    case LValue_:
    case QueriedPseudoField_:
    case RefExpr_:
      if (ResolvedEntity *d = getNextDefinition())
        return d != this && d->isCodeBlockEntity();
      else
        return false;
    case Variable_:
      if (isPackageVariable())
        return false;
      else
        return true;
    case Function_:
      if (!isElementaryLinterFunction())
        return true;
      else
        return false;
    default:
      if (toSelfStatementInterface())
        return false;

      return true;
  }
}


void ResolvedEntity::checkToGrantOtherUser(Privs privilegie, Sm::Codestream &str) {
  if (UserContext *usr = userContext())
    if (!(usr->isSystem()) /*&& usr != str.currentUser_ */) {
      Codestream *s = str.mainStream ? str.mainStream : &str;
      s->needToGrantee[privilegie][usr].insert((ResolvedEntity*)this);
    }
}

bool ResolvedEntity::containsInSystemContext() const {
  if (ResolvedEntity *uCntx = userContext())
    return uCntx->isSystem();
  else
    return false;
}



ResolvedEntity* ResolvedEntity::getResolvedNextDefinition() {
  if (ResolvedEntity* def = getNextDefinition())
    return def;
  semanticResolve();
  return getNextDefinition();
}


bool ResolvedEntity::containsEntity(ResolvedEntity *entity) {
  if (this == entity)
    return true;
  else if (ResolvedEntity *next = getNextDefinition()) {
    if (next != this)
      return next->containsEntity(entity);
  }
  return false;
}


bool Sm::Function::equallyByArglists(ResolvedEntity *oth) const {
  Ptr<Arglist> othLst = oth->funArglist();
  if (arglist && othLst) {
    if (arglist->size() != othLst->size())
      return false;

    Arglist::const_iterator lstIt    = arglist->begin();
    Arglist::const_iterator othLstIt = othLst->begin();
    for (; lstIt != arglist->end() && othLstIt != othLst->end(); ++lstIt, ++othLstIt) {
      Ptr<Datatype> t1 = (*lstIt)		->getDatatype();
      Ptr<Datatype> t2 = (*othLstIt)->getDatatype();
      if (t1 && t2 && t1->isExactlyEqually(t2))
        continue;
      else if (!t1 && !t2)
        continue;
      else
        return false;
    }
  }
  else if ((arglist && !arglist->empty()) || (othLst && !othLst->empty()))
    return false;

  Ptr<Datatype> currentT = getDatatype();
  Ptr<Datatype> othT     = oth->getDatatype();

  if (!currentT)
    return !othT;
  else if (!othT)
    return !currentT;

  return (currentT->isExactlyEqually(othT));
}

void Sm::Function::identificateOverloaded() {
  if (overloadedId_ < 0 && vEntities())
    if (Ptr<LevelResolvedNamespace> lvlNamespace = levelNamespace())
      if (Ptr<Id> name = getName()) {
        // Получить область одинаковых имен для данного имени в данном пространстве имен
        LevelResolvedNamespace::iterator equallyNames = lvlNamespace->find(name->toNormalizedString());
        if (equallyNames != lvlNamespace->end())
          if (Ptr<VEntities> G = equallyNames->second)
            G->identificateOverloadedFunctions((Function*)this);
      }
}

void ResolvedEntity::getStatementsWithEntity(ResolvedEntity* entity, Sm::StatementInterface *ownerStmt, FoundedStatements &outList) {
  if (SemanticTree *node = getSemanticNode())
    if (node->containsEntity(entity))
      outList.push_back(ownerStmt);
}


LevelResolvedNamespace::~LevelResolvedNamespace() {
  if (!syntaxerContext.model) // фаза глобальной очистки
    return;
  if (parent)
    for (Childs::iterator it = parent->childs.begin(); it != parent->childs.end(); ++it)
      if (*it == this) {
        parent->childs.erase(it);
        break;
      }
}

void LevelResolvedNamespace::deleteIncludedDeclarationsFromSet(UniqueEntitiesMap &declSet) const {
  if (size())
    for (UniqueEntitiesMap::iterator it = declSet.begin(); it != declSet.end(); ) {
      if (Ptr<Id> n = it->first->getName()) {
        LevelResolvedNamespace::const_iterator spIt = this->find(n->toNormalizedString());
        if (spIt != this->end() && spIt->second && spIt->second->findByPtr(n->definition())) {
          it = declSet.erase(it);
          continue;
        }
      }
      ++it;
    }
  for (Childs::const_iterator it = childs.begin(); it != childs.end(); ++it)
    (*it)->deleteIncludedDeclarationsFromSet(declSet);
}

bool LevelResolvedNamespace::add(::Sm::Id *name) {
  ResolvedEntity *def = name->unresolvedDefinition();
  LevelResolvedNamespace::iterator it = this->find(def->translatedName());
  int i = -1;
  if (it != this->end()) {
    if ((i = it->second->compareWithFront(def)) < 0) {
      // -1 - соответствующий список пуст, можно добавить в начало, 0 -
      // 0 - с начала соответствующего списка уже стоит другой элемент
      // 1 - в начале соответствующего списка стоит тот же элемент
      addWithoutFind(name);
      return true;
    }
    else
      return i > 0;
  }
  addWithoutFind(name);
  return true;
}

void LevelResolvedNamespace::addWithoutFind(::Sm::Id *name) {
  ResolvedEntity *definition = name->unresolvedDefinition();
  if (!definition)
    throw 999;
  Ptr<VEntities>  ent   = new VEntities();  ent  ->add(definition);
  if (!definition->toSelfRefCursor())
    insert(LevelResolvedNamespace::value_type(definition->translatedName(), LevelResolvedNamespace::value_type::second_type(ent)));
}

std::string LevelResolvedNamespace::getUniqueName(std::string prefix) {
  int suf = 0;
  char buf[16];
  std::string n;
  do {
    suf++;
    sprintf(buf, "%i", suf);
    n = prefix;
    n.append(buf);
  } while (count(n) > 0 || (transform(n.begin(), n.end(), n.begin(), ::toupper), count(n) > 0));
  return prefix+buf;
}

void LevelResolvedNamespace::resolveOverloaded() {
  for (BaseType::iterator it = this->begin(); it != this->end(); ++it)
    if (it->second)
      it->second->resolveOverloaded(); // вызов у VEntities
  for (Childs::value_type &child : childs)
    child->resolveOverloaded(); // вызов у других LevelResolvedNamespace - для которых текущий узел является владельцем
}

void LevelResolvedNamespace::check() {
  for (value_type &it : *this)
    if (it.second)
      it.second->check();
  for (Childs::value_type &cit : childs)
    cit->check();
}

void VEntities::check(Container &c) {
  for (Container::value_type &mit : c)
    if (SemanticTree *node = mit->getSemanticNode()) {
      if (node->nametype == Sm::SemanticTree::REFERENCE /*&& !node->alias()*/)
        throw 999; // Ссылок в пространстве имен определений быть не должно по построению
    }
    else
      throw 999; // семантический узел должен быть задан.
}

void EquallyEntities::check(EquallyEntities::Container &c) {
  for (ResolvedEntity *mit : c)
    if (SemanticTree *node = mit->getSemanticNode()) {
      if (node->nametype == Sm::SemanticTree::REFERENCE /*&& !node->alias()*/)
        throw 999; // Ссылок в пространстве имен определений быть не должно по построению
    }
    else
      throw 999; // семантический узел должен быть задан.
}

void VEntities::check(Ptr<EquallyEntities> c) {
  if (!c)
    return;
  c->check(c->declarations);
  c->check(c->definitions);
}

void VEntities::check() {
  if ((functions.size() && (variables || others.size())) || (variables && others.size()))
    throw 999;
  check(functions);
  check(variables);
}

void VEntities::addWithoutOverloadResolving(ResolvedEntity* def) {
  if (def) {
    if (def->isFunction())
      functions.add(def);
    else if (def->isVariable()) {
      if (!variables)
        variables = new EquallyEntities();
      variables->insert(def);
    }
    else {
      if (def->ddlCathegory() == ResolvedEntity::ModelContext_)
        return;
      others.insert(def);
    }
  }
}

void VEntities::resolveOverloaded() {
  setOwerloadResolvingEntered();
  for (Container::value_type &f : functions)
    resolveOverloadedForce(f);
}

std::pair<EquallyEntities::Container::iterator, bool> EquallyEntities::insert(const EquallyEntities::Container::value_type &v) {
  if (key == 0)
    key = v;
  if (v->isDefinition()) {
    if (!key->isDefinition())
      key = v;
    return definitions.insert(v);
  }
  return declarations.insert(v);
}

void VEntities::resolveOverloaded(ResolvedEntity* def) { // private
  setOwerloadResolvingEntered();
  if (def && def->isFunction())
    resolveOverloadedForce(def);
}

void VEntities::addOverloadedNode(ResolvedEntity* def) { // private
  size_t id = overloadedFunctions.size();
  def->overloadedId(id);
  Ptr<EquallyEntities> funs = new EquallyEntities();
  funs->insert(def);
  overloadedFunctions[def] = funs;
  overloadedNodes[id] = funs;
}

void VEntities::resolveOverloadedForce(ResolvedEntity* def) { // private
  // Индексируются по eid-у, который генерируется в порядке построения модели
  for (OverloadedFunctions::iterator it = overloadedFunctions.begin(); it != overloadedFunctions.end(); ++it) {
    OverloadedFunctions::mapped_type &currentOverloadingNode = it->second;
    ResolvedEntity *f = it->first;
    f->setVEntities(this);
    // Если найдена перегрузка с такой же сигнатурой
    if (f->equallyByArglists(def)) {
      if (!currentOverloadingNode)
        throw 999; // множество функций не задано, в механизме инициализации - ошибка.

      // Если добавили в контейнер найденной перегруженной функции
      // (могли и не добавить, если eid тот же)
      if (currentOverloadingNode->insert(def).second) {
        // установить ее индекс перегрузки, взятый из ранее найденной перегруженной функции
        def->overloadedId(f->overloadedId());
        // Если эта перегрузка является определением и ранее не было определений
        // Подмена определения, если это необходимо
        // Изменения порядка в overloaded-functions не должно влиять на индексацию перегруженных функций, т.к. новые индексы перегрузки
        // генерируются при росте контейнера перегруженных функций
        if (!f->isDefinition() && def->isDefinition()) {
          OverloadedFunctions::mapped_type savedPtr = currentOverloadingNode; // предотвращение удаления smartPtr
          overloadedFunctions.erase(it);       // удалить узел по старому уникальному глобальному индексу
          // Подменить главный узел-определение (который до этого момента был объявлением) на настоящее определение
          overloadedFunctions[def] = savedPtr; // добавить узел по новому уникальному глобальному индексу
        }
      }
      return; // Найдена функция с такой же сигнатурой, перегрузка добавлена в ее узел
    }
  }
  // Функций с такой сигнатурой не найдено, будет создаваться новый узел перегрузки
  addOverloadedNode(def);
}

void VEntities::add(ResolvedEntity* def) {
  addWithoutOverloadResolving(def);
  resolveOverloaded(def);
}

void VEntities::identificateOverloadedFunctions(Sm::Function *src) {
  // вычисление максимального идентификатора перегрузки
  std::vector<ResolvedEntity*> overloaded;
  int maxOverId = -1;
  for (ResolvedEntity *it : functions) {
    int v = it->overloadedId();
    if (v > maxOverId)
      maxOverId = v;
    if (v < 0 && src->equallyByArglists(it))
      overloaded.push_back(it);
  }
  ++maxOverId;
  // обновление подходящих перегруженных функций.
  for (ResolvedEntity *it : overloaded)
    it->overloadedId(maxOverId);
}

bool VEntities::count(ResolvedEntity *d) const {
  if (d) {
    if (d->isFunction())
      return overloadedFunctions.count(d->getDefinitionFirst());
    else if (d->isVariable() && variables)
      return variables->definitions.count(d);
    else
      return others.count(d->getDefinitionFirst());
  }
  return false;
}

GlobalEntitiesCnt ResolvedEntity::eid() const {
  const ResolvedEntity *p = this;
  if (p == 0)
    throw 999;
  return eid_;
}

GlobalEntitiesCnt ResolvedEntity::nextGlobalCnt() {
  ++globalEntitiesCounter;
  if (globalEntitiesCounter == 3447919)
    cout << "";
  return globalEntitiesCounter;
}

void ResolvedEntity::clrSRef() {
  if (__STreeDestructionStarted__)
    return;
  if (SemanticTree *n = getSemanticNode()) {
    setSemanticNode(0);
    n->clearDefinitions();
    n->deleteFromParent();
    delete n;
  }
}

bool VEntities::checkRefPtrEquality(SemanticTree *sNode, Ptr<Sm::Id> &reference) const
{
  if (const Ptr<Sm::Id> &refEntity = sNode->refEntity())
    if (reference.object() == refEntity.object())
      return true;
  return false;
}

bool VEntities::findByPtr(ResolvedEntity *def) const {
  // TODO: неоптимально. Вектор сортированный по позиции в пространстве имен, можно искать за логарифмическое время
  if (def) {
    if (def->isFunction()) {
      GlobalOverloadedNodes::const_iterator it = overloadedNodes.find(def->overloadedId());
      return (it != overloadedNodes.end() && (
            it->second->definitions.count(def) ||
            it->second->declarations.count(def))
          ) || overloadedFunctions.count(def->getDefinitionFirst());
    }
    else if (def->isVariable() && variables)
      return variables->declarations.count(def) || variables->definitions.count(def);
    else
      return others.count(def->getDefinitionFirst());
  }
  return false;
}


bool LE_ResolvedEntities::lt(const ResolvedEntity *l, const ResolvedEntity *r) {
  if (l && r)
    return l->eid() < r->eid();
  return (!l && r);
}

bool LE_ResolvedEntities::operator() (const ResolvedEntity *l, const ResolvedEntity *r) const {
  return lt(l, r);
}


bool VEntities::assignDeclForce(Sm::Id *reference, ResolvedEntity *def, SemanticTree *&referenceNode, SemanticTree *&definitionNode) const {
  reference->definition(def);
  referenceNode = definitionNode;
  return true;
}


void VEntities::setSpecialNamesTranslator(ResolvedEntity *d, Sm::NameTranslator nameTr, Sm::CallarglistTranslator *callTr, bool forceElementaryLinter) {
  d->callarglistTR(callTr);
  d->nameTR(nameTr);
  for (VEntities::Container::iterator it = functions.begin(); it != functions.end(); ++it) {
    (*it)->callarglistTR(callTr);
    (*it)->nameTR(nameTr);
    if (forceElementaryLinter)
      if (Function *f = (*it)->toSelfFunction())
        f->setElementaryLinFunc();
  }
}

ResolvedEntity *VEntities::findFieldWithoutAlias() {
  if (others.empty())
    return 0;
  if ((*others.begin())->getAlias())
    return 0;
  return *others.begin();
}

ResolvedEntity *VEntities::findFieldDefinition() {
  if (others.empty())
    return 0;
  for (ResolvedEntity *f : others)
    if (f->getName())
      return f;
  return *others.begin();
}

ResolvedEntity *VEntities::findVariableDefinition() {
  if (variables)
    return variables->key;
  return 0;
}




int VEntities::compareWithFront(ResolvedEntity *def) const {
  if (def->isFunction() && functions.size())
    return functions.front() == def ? 1 : 0; // true - данное определение уже добавлено, false - без поиска добавить не получится.
  else if (def->isVariable() && variables)
    return variables->definitions.count(def) ? 1 : 0;
  else
    return others.count(def->getDefinitionFirst()) > 0 ? 1 : -1;
  return -1;
}


bool ResolvedEntity::isSystem() const {
  if (isSystemFlag())
    return true;
  if (syntaxerContext.model->sysusers.count((ResolvedEntity*)this))
    return true;
  if (Ptr<Id2> n2 = getName2())
    if (n2->uname())
      if (ResolvedEntity *def = n2->uname()->definition())
        if (syntaxerContext.model->sysusers.count((ResolvedEntity*)def))
          return true;
  if (ResolvedEntity *own = owner())
    if (own->isSystem())
      return true;

  if (toSelfPackage())
    return false;
  return !hasLinterEquivalent();
}

bool ResolvedEntity::isCompositeType() const {
  return isRecordType() || isCollectionType() || isObjectType() || isRowTypeOf();
}


ResolvedEntity* ResolvedEntity::owner() const {
  if (SemanticTree *parentNode = sOwner()) {
    ResolvedEntity *p = parentNode->uddlEntity();
    if (p == this) {
      if (ddlCathegory() == ModelContext_)
        return 0;
      throw 999;
    }
    return p;
  }
  return 0;
}

UserContext* ResolvedEntity::userContext() const {
  if (ResolvedEntity *def = owner())
    return def->userContext();
  return 0;
}

ResolvedEntity *ResolvedEntity::ownerVariable() const {
  if (ResolvedEntity *def = owner())
    return def->ownerPackage();
  return 0;
}

ResolvedEntity *ResolvedEntity::ownerPackage() const {
  if (ResolvedEntity *def = owner())
    return def->ownerPackage();
  return 0;
}

bool ResolvedEntity::isScopeEntity() const {
  ScopedEntities ent = ddlCathegory();
  switch (ent) {
  case Package_:
  case Object_:
    return true;
  default:
    return false;
  }
  return false;
}

Ptr<Datatype> ResolvedEntity::getDatatypeWithSetFieldStruct(Ptr<Datatype> &thisDatatype, Ptr<Sm::Id> entityName) const {
  if (!thisDatatype) {
    if (!entityName->definition())
      entityName->definition(this);
      entityName->semanticNode(getSemanticNode());
      thisDatatype = new Datatype(entityName);
      thisDatatype->setSemanticNode(getSemanticNode());
      if (!isObjectType() && !isAnydata())
        thisDatatype->setIsRowtypeOf();
  }
  return thisDatatype;
}

// TODO: доработать для других вариантов
Ptr<Sm::Datatype> ResolvedEntity::getDatatypeOfCollectionItem() const { return getDatatype(); }


Sm::ResolvedEntity *ResolvedEntity::ownerFunction() const {
  if (ResolvedEntity *def = owner())
    return def->ownerFunction();
  return 0;
}

Sm::BlockPlSql* ResolvedEntity::maximalCodeBlock(Sm::BlockPlSql *start) const {
  if (ResolvedEntity *def = owner())
    return def->maximalCodeBlock(start);
  return start;
}

Sm::BlockPlSql* Sm::BlockPlSql::maximalCodeBlock(Sm::BlockPlSql *start) const {
  if (ResolvedEntity *def = owner()) {
    if (def->ddlCathegory() == Package_)
      return start;
    return def->maximalCodeBlock((BlockPlSql*)this);
  }
  return (BlockPlSql*)this;
}




Ptr<LevelResolvedNamespace> ResolvedEntity::internalNamespace() const {
  SemanticTree *n = getSemanticNode();
  return n ? n->levelNamespace : Ptr<LevelResolvedNamespace>(0);
}


ResolvedEntity* ResolvedEntity::getNextNondatatypeDefinition() const {
  ResolvedEntity* e = (ResolvedEntity*)this;
  while (e && (e->toSelfDatatype() || e->toSelfSubtype()))
    e = e->getNextDefinition();
  return e;
}


Sm::Datatype* ResolvedEntity::getLastConcreteDatatype(ResolvedEntity *curr) { return SubqueryUnwrapper::unwrap(curr); }
Sm::Datatype* ResolvedEntity::getLastUnwrappedDatatype(ResolvedEntity *curr) { return SyntaxUnwrapper::unwrap(curr); }

Ptr<Datatype> ResolvedEntity::getUnforeignDatatype() const { return getDatatype(); }


Datatype *ResolvedEntity::unwrappedDatatype() const { return getLastUnwrappedDatatype(getDatatype()); }


int Sm::Datatype::getLengthForPlSqlVarcharImplicitConversion() {
  if (isVarcharDatatype() || isCharDatatype()) {
    if (ResolvedEntity *d = getNextDefinition())
      if (!d->toSelfFundamentalDatatype())
        return -1;
    if (!precision)
      return 4000;
    return precision;
  }
  if (isBigint()) { return 20; }
  else if (isInt()) { return 10; }
  else if (isSmallint()) { return 5; }
  else if (isReal()) { return 20; }
  else if (isDouble()) { return 40; }
  else if (isNumberDatatype()) {
    if (precision)
      return (scale && scaleIsSet()) ? (precision > 30 ? 30 : precision) + 1 : precision;
    else
      return (scale && scaleIsSet()) ? 1 + (scale > 30 ? 30 : scale) : 20;
  }
  else if (isDateDatatype())
    return syntaxerContext.dateToCharDefaultLength;
  else if (isClobDatatype())
    return 4000;

  return -1;
}


Sm::Datatype* Sm::Datatype::getConcatVarcharDatatype(Datatype *l, Datatype *r, bool /*isPlContext*/, const FLoc &loc) {
  if (!r)
    return l;
  else if (!l)
    return r;
  if (loc.loc.beginedFrom(189186,21))
    cout << "";

  // нужно корректно находить длину возвращаемого значения функций
  Datatype* lhs = ResolvedEntity::getLastConcreteDatatype(l);
  Datatype* rhs = ResolvedEntity::getLastConcreteDatatype(r);
  if (!rhs)
    rhs = ResolvedEntity::getLastConcreteDatatype(r);
  if (!rhs)
    return lhs;
  if (!lhs)
    return rhs;


  ResolvedEntity *unrefLhs = lhs->getNextDefinition();
  ResolvedEntity *unrefRhs = rhs->getNextDefinition();

  auto getTypeLength = [] (ResolvedEntity *unrefTHS, Datatype* ths) -> int {
    if (unrefTHS->toSelfFundamentalDatatype())
      return ths->getLengthForPlSqlVarcharImplicitConversion();
    else {
      if (Datatype *lT = unrefTHS->toSelfDatatype()) {
        if (lT->isExactlyEquallyByDatatype(Datatype::mkNumber()))
          return (ths->scaleIsSet() && ths->scale) ? ths->precision + 1 + ths->scale : ths->precision;
        else if (lT->isBigint())
          return 10;
        else if (lT->isInt())
          return 10;
        else if (lT->isSmallint())
          return 5;
        else if (lT->isReal())
          return 20;
        else if (lT->isFloat() || lT->isDouble())
          return 40;
      }
    }
    return -1;
  };

  if (unrefLhs && unrefRhs) {
    int lhsLen = getTypeLength(unrefLhs, lhs);
    int rhsLen = getTypeLength(unrefRhs, rhs);
    if (lhsLen >= 0 && rhsLen >= 0)
      return Datatype::mkVarchar2(lhsLen + rhsLen > 4000 ? 4000 : lhsLen + rhsLen);
  }
  Codestream str;
  str << "Error: unknown implicit conversion in concat " << " lhs = ";
  lhs->oracleDefinition(str);
  str << "; rhs = ";
  rhs->oracleDefinition(str);
  str << "/" << s::iloc(loc);
  cout << str.str() << endl;

  return Datatype::mkVarchar2();
}


ResolvedEntity::~ResolvedEntity() {
  if (basicEntityAttributes)
    delete basicEntityAttributes;
}


ResolvedEntity *ResolvedEntity::unwrapReference() const {
  if (RefExpr*def = toSelfRefExpr()) {
    if (ResolvedEntity *d = def->getNextDefinition())
      return d->unwrapReference();
    else
      return 0;
  }
  return const_cast<ResolvedEntity*>(this);
}

bool ResolvedEntity::isSqlCode() const {
  if (SemanticTree *n = getSemanticNode())
    return n->isSqlCode();
  throw 999;

}



void ResolvedEntity::outputUnimplementedMethod(const std::string &methodName) const {
  std::cout << "ERROR: method " << methodName << " is unimplemented for " << Sm::toString(ddlCathegory()) << std::endl;
}

void ResolvedEntity::translateVariableType(Codestream &, ResolvedEntity *, bool) {
  outputUnimplementedMethod("translateVariableType");
}

void ResolvedEntity::translateObjRef(Codestream &, Type::RefInfo *) {
  outputUnimplementedMethod("translateObjRef");
}

void ResolvedEntity::translateLocalObjects(Codestream &) {
  outputUnimplementedMethod("translateLocalObjects");
}

string ResolvedEntity::getObjectTempTable(ResolvedEntity *) {
  outputUnimplementedMethod("getObjectTempTable"); return std::string();
}

void ResolvedEntity::translatedName(const string &v) {
  if (this->eid_ == 4912595)
    cout << "";
  sqlName(v);
}

string ResolvedEntity::translatedName() const {
  return sqlName();
}

void ResolvedEntity::linterDefinitionKeys(Codestream &) {
  outputUnimplementedMethod("linterDefinitionKeys");
}

void ResolvedEntity::setFieldNumber(int) {
  outputUnimplementedMethod("setFieldNumber");
}

void ResolvedEntity::clrOpenCursorCommand() {
  outputUnimplementedMethod("clrOpenCursorCommand");
}

void ResolvedEntity::setOpenCursorCommand(OpenCursor *) {
  outputUnimplementedMethod("setOpenCursorCommand");
}

void ResolvedEntity::setSquoted() {
  outputUnimplementedMethod("setSquoted");
}

void ResolvedEntity::identificateOverloaded() {
  outputUnimplementedMethod("identificateOverloaded");
}

bool ResolvedEntity::isExactlyEquallyByDatatype(ResolvedEntity *) {
  outputUnimplementedMethod("isExactlyEquallyByDatatype");
  return false;
}

BlockPlSql *ResolvedEntity::childCodeBlock() const {
  outputUnimplementedMethod("childCodeBlock");
  return 0;
}

Function *ResolvedEntity::getDefaultConstructor() const {
  outputUnimplementedMethod("getDefaultConstructor"); return 0;
}

Subquery *ResolvedEntity::getSelectQuery() {
  outputUnimplementedMethod("getSelectQuery"); return 0;
}

bool ResolvedEntity::isDynamicUsing() const {
  return !syntaxerContext.translateReferences || (__flags__.v & FLAG_RESOLVED_ENTITY_IS_DYNAMIC_USING);
}


void Unwrapper::updateCounters(ResolvedEntity **unref, ResolvedEntity **curr) const {
  *curr = *unref;
  *unref = (*unref)->getNextDefinition();
}

Datatype* Unwrapper::unwrapSingleFieldStructure(ResolvedEntity *structType) const {
  EntityFields f;
  structType->getFields(f);
  if (f.size() == 1) // проверить на одно поле, если поле одно - вернуть его тип, иначе - вернуть себя как тип данных
    return unwrapDatatype(f.front()->definition());
  else
    return structType->getDatatype();
}

Ptr<Datatype> Unwrapper::unwrapStruct(ResolvedEntity *structType) const {
  switch (structType->ddlCathegory()) {
    case ResolvedEntity::Function_:
    case ResolvedEntity::MemberFunction_:
      if (Function *f = structType->toSelfFunction()) {
        if (!f->flags.isRettypeConversionEntered())
          f->inferenceReducedRettype();
        if (f->reducedRettype)
          return f->reducedRettype;
        else {
          if (Datatype *t = unwrapDatatype(f->getDatatype()))
            return t;
          else
            return f->getDatatype();
        }
      }
      else
        return f->getDatatype();
    case ResolvedEntity::SqlSelectedField_:
      return unwrapDatatype(structType->getDatatype());
    case ResolvedEntity::SqlExprUnion_:
    case ResolvedEntity::SqlExprSelectSingle_:
    case ResolvedEntity::SqlExprSelectBrackets_:
    case ResolvedEntity::QueryBlock_:
      return unwrapSubquery(structType);
    case ResolvedEntity::Table_:
    case ResolvedEntity::View_:
    case ResolvedEntity::Record_:
    case ResolvedEntity::LinterCursor_:
    case ResolvedEntity::Cursor_:
      return unwrapStructuredType(structType);
    case ResolvedEntity::RefCursor_:
      throw 999; // данную ситуацию нужно или обработать или реализовать вывод с сообщением об ошибке
      break;
    case ResolvedEntity::Object_:
    case ResolvedEntity::NestedTable_:
    case ResolvedEntity::Varray_:
      return structType->getDatatype();
    default:
      break;
  }
  return 0;
}

Datatype* Unwrapper::unwrapDatatype(const ResolvedEntity *currPtr) const {
  if (!currPtr)
    return 0;
  ResolvedEntity *curr = const_cast<ResolvedEntity*>(currPtr);
  ResolvedEntity *unref = curr->getNextDefinition();
  for (; curr; updateCounters(&unref, &curr)) {
    if (curr->toSelfSqlExpr()) {
      Ptr<Datatype> t = curr->getDatatype();
      return unwrapDatatype(t.object());
    }
    if (!unref) {
      switch (curr->ddlCathegory()) {
        case ResolvedEntity::AlgebraicCompound_:
        case ResolvedEntity::RefCursor_:
          return curr->getDatatype().object();
        default:
          return 0;
      }
    }
    else if (curr == unref) {
      if (Ptr<Datatype> t = unwrapStruct(curr))
        return t;
      return curr->getDatatype();
    }
    else if (curr->isDatatype()) {
      if (unref->isFundamentalDatatype() || unref->isObject())
        return curr->toSelfDatatype();
    }
    else if (Ptr<Datatype> t = unwrapStruct(curr))
      return t;
  }
  return 0;
}

Datatype *Unwrapper::unwrapDatatype(const Ptr<Datatype> &curr) const { return unwrapDatatype(curr.object()); }

Datatype *StructureSubqueryUnwrapper::unwrapStructuredType(ResolvedEntity *structType) const  { return unwrapSingleFieldStructure(structType); }

Datatype *StructureSubqueryUnwrapper::unwrapSubquery(ResolvedEntity *structType) const  { return unwrapSingleFieldStructure(structType); }

Datatype *SyntaxUnwrapper::unwrapStructuredType(ResolvedEntity *structType) const { return structType->getDatatype(); }

Datatype *SyntaxUnwrapper::unwrapSubquery(ResolvedEntity *structType) const  { return structType->getDatatype(); }

Datatype *SubqueryUnwrapper::unwrapStructuredType(ResolvedEntity *structType) const  { return structType->getDatatype(); }

Datatype *SubqueryUnwrapper::unwrapSubquery(ResolvedEntity *structType) const { return unwrapSingleFieldStructure(structType); }


Datatype *SyntaxUnwrapper::unwrap(ResolvedEntity *structType) {
  static const SyntaxUnwrapper unwr;
  return unwr.unwrapDatatype(structType);
}

Datatype *SyntaxUnwrapper::unwrap(const Ptr<Datatype> &t) { return unwrap(t.object()); }

Datatype *StructureSubqueryUnwrapper::unwrap(ResolvedEntity *structType) {
  static const StructureSubqueryUnwrapper unwr;
  return unwr.unwrapDatatype(structType);
}

Datatype *StructureSubqueryUnwrapper::unwrap(const Ptr<Datatype> &t) { return unwrap(t.object()); }

Datatype *SubqueryUnwrapper::unwrap(const ResolvedEntity *structType) {
  static const SubqueryUnwrapper unwr;
  return unwr.unwrapDatatype(structType);
}

Datatype *SubqueryUnwrapper::unwrap(const Ptr<Datatype> &t) { return unwrap(t.object()); }

bool operator==(const rRef &a, const rRef &b) {
  if (a.ref != b.ref) {
    if (a.ref == nullptr || b.ref == nullptr)
      return false;
    return a.ref->eqByVEntities(b.ref);
  }
  return true;
}

SemanticTree *ResolvedEntitySNode::toSTreeGenerateStage() const {
  if (getSemanticNode())
    return 0;
  else
    return setSemanticNode(toSTreeBase());
}

SemanticTree *ResolvedEntitySNode::toSTreeTransformStage() const {
  if (SemanticTree *n = getSemanticNode())
    return n;
  else
    return setSemanticNode(toSTreeBase());
}

void ResolvedEntitySNode::pushTransformStage() {
  toSTreeActionStack.push(&ResolvedEntitySNode::toSTreeTransformStage);
  toSTreeAction = &ResolvedEntitySNode::toSTreeTransformStage;
}

void ResolvedEntitySNode::pushGenerateStage() {
  toSTreeActionStack.push(&ResolvedEntitySNode::toSTreeGenerateStage);
  toSTreeAction = &ResolvedEntitySNode::toSTreeGenerateStage;
}

void ResolvedEntitySNode::popStage() {
  toSTreeActionStack.pop();
  if (toSTreeActionStack.empty())
    throw 999;
  toSTreeAction = toSTreeActionStack.top();
}
