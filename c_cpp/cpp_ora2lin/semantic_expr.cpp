#include "semantic_base.h"
#include "semantic_expr.h"
#include "semantic_plsql.h"

using namespace Sm;


void pl_expr::Comparsion::checkInvariant() const {
  if (!lhs || !rhs || rhs->empty() || !*(rhs->begin()))
    throw 999;
}


pl_expr::Comparsion::Comparsion(CLoc l, PlExpr *_lhs, ComparsionOp::t _op, QuantorOp::t _q, List<PlExpr> *_rhs)
  : GrammarBase(l), lhs(_lhs), op(_op), quantor(_q), rhs(_rhs) { checkInvariant(); }


pl_expr::Comparsion::Comparsion(FLoc l, Ptr<PlExpr> _lhs, ComparsionOp::t _op, Ptr<PlExpr> _rhs, SemanticTree *n)
  : GrammarBase(l), lhs(_lhs), op(_op), rhs(new List<PlExpr>(_rhs))
{
  checkInvariant();
  if (n)
    setSemanticNode(n);
}

pl_expr::Comparsion::Comparsion(FLoc l, PlExpr *_lhs, PlExpr *_rhs)
  : GrammarBase(l), lhs(_lhs), rhs(new RHS(_rhs))
{
  checkInvariant();
}




void pl_expr::Comparsion::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(SCathegory::Comparsion);
  node->addChildForce(n);
  SNODE(n); CTREE(lhs); CollectSNode(n, rhs);
}



ComparsionOp::t pl_expr::Comparsion::getNormalizedComparsionByNot() {
  using namespace ComparsionOp;
  if (!isNot())
    return op;
  switch (op) {
    case LT: return GE;
    case LE: return GT;
    case GT: return LE;
    case GE: return LT;
    case EQ: return NEQ;
    case NEQ: return EQ;
    case IN: throw 999;
      break;
  }
  return op;
}

Ptr<SqlExpr> pl_expr::Comparsion::getRownumValue() {
  using namespace ComparsionOp;
  if (isNot() || rhs->size() > 1 || rhs->front()->toSelfRowNumExpr() || !lhs->toSelfRowNumExpr())
    throw 999;

  Ptr<PlExpr> &hs = *(rhs->begin());
  switch (op) {
    case LE:
      if (NumericValue *e = hs->toSelfNumericValue()) {
        if (e->cathegory() == NumericValue::SIMPLE_INT)
          return e;
      }
      else if (RefExpr*e = hs->toSelfRefExpr())
        return e;
      break;
    case LT:
      if (NumericValue *e = hs->toSelfNumericValue())
        if (e->cathegory() == NumericValue::SIMPLE_INT)
          return new NumericSimpleInt(e->getSIntValue() - 1);
      break;
    case EQ:
      if (NumericValue *x = hs->toSelfNumericValue())
        if (x->getSIntValue() == 1)
          return x;
      // pass
    default:
      return hs->toSelfSqlExpr();
  }
  return hs->toSelfSqlExpr();
}

bool pl_expr::Comparsion::isLimitExpr() {
  return lhs->unwrapBrackets()->toSelfRowNumExpr() ||
         rhs->front()->unwrapBrackets()->toSelfRowNumExpr();
}

void pl_expr::Comparsion::translateToConsistantOperand() {
  static const auto similarTypes = [](vector<Ptr<Datatype> > &v, bool (Datatype::*ptr)() const) -> bool {
    for (Ptr<Datatype> &t : v)
      if (!(t.object()->*ptr)() && !t->isNull())
        return false;
    return true;
  };

  if (beginedFrom(655506))
    cout << "";

  Ptr<Datatype> lt = SubqueryUnwrapper::unwrap(lhs->getDatatype());
  if (!lt)
    return; // что-то недорезолвлено

  std::vector<Ptr<Datatype> > rt;
  for (Comparsion::RHS::value_type &v : *(rhs)) {
    Ptr<Datatype> t = SubqueryUnwrapper::unwrap(v->getDatatype());
    if (!t)
      return; // что-то недорезолвлено
    rt.push_back(t);
  }
  rt.push_back(lt);

  if (similarTypes(rt, &Datatype::isCharVarchar) ||
      similarTypes(rt, &Datatype::isDateDatatype) || similarTypes(rt, &Datatype::isBool) ||
      similarTypes(rt, &Datatype::isBigint) || similarTypes(rt, &Datatype::isInt) || similarTypes(rt, &Datatype::isSmallint))
    return;

  if (op != ComparsionOp::IN && similarTypes(rt, &Datatype::isNum))
    return;

  rt.pop_back();
  SemanticTree *n = getSemanticNode();
  bool isSql = n->isSqlCode();
  bool lhsToCast = lt->isCharVarchar() || op == ComparsionOp::IN;
  Ptr<PlExpr> &castedHs = lhsToCast ? lhs : *(rhs->begin());
  Ptr<Datatype> castedType = lhsToCast ? lt : rt.front();
  Ptr<Datatype> newType    = lhsToCast ? rt.front() : lt;

  if (!castedHs || !castedType || !newType)
    throw 999;

  CastCathegory c = castedType->getCastCathegory(newType, !isSql, isSql);
  if (op == ComparsionOp::IN && c.implicit() && (rhs->size() == 1 || (*rhs->begin())->isIntValue()))
    return;
  if (c.equallyAlmost())
    return;
  CommonDatatypeCast::castAndReplace(!isSql, castedHs, castedType, newType, c);
}


pl_expr::LogicalCompound::LogicalCompound(CLoc l, PlExpr *_lhs, PlExpr *_rhs, int isOr) // OR
  : GrammarBase(l), lhs(_lhs), rhs(_rhs)
{
  if (isOr)
    __flags__.v |= (SmartCount)logical_compound::OR;
  checkInvariant();
}


void pl_expr::LogicalCompound::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(lhs); CTREE(rhs); }


void pl_expr::LogicalCompound::linterDefinition(Codestream &str) {
  if (lhs && rhs) {
    str << lhs << s::name << (isOr() ? "OR" : "AND") << s::name
        << s::subconstruct << rhs;
  }
  else if (lhs)
    str << lhs;
  else if (rhs)
    str << rhs;
}

void pl_expr::LogicalCompound::enumSubExpressions(List<PlExpr> &exprList) {
  if (lhs)
    lhs->enumSubExpressions(exprList);
  if (rhs)
    rhs->enumSubExpressions(exprList);
}


AlgebraicCompound::AlgebraicCompound(CLoc l, Ptr<SqlExpr> _lhs, algebraic_compound::t _op, Ptr<SqlExpr> _rhs, SemanticTree *n)
  : GrammarBase(l), lhs(_lhs), op(_op), rhs(_rhs)
{
  if (!lhs || !rhs)
    throw 999;
  if (n)
    setSemanticNode(n);
}


AlgebraicCompound::AlgebraicCompound(SqlExpr *_lhs, algebraic_compound::t _op, SqlExpr *_rhs)
  : SqlExpr(), lhs(_lhs), op(_op), rhs(_rhs)
{
  if (!lhs || !rhs)
    throw 999;
}

void AlgebraicCompound::enumSubExpressions(List<PlExpr> &exprList) {
  if (lhs)
    lhs->enumSubExpressions(exprList);
  if (rhs)
    rhs->enumSubExpressions(exprList);
}


Sm::IsSubtypeValues AlgebraicCompound::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  Ptr<Datatype> t = getDatatype();
  return t->isSubtype(supertype, plContext);
}


bool AlgebraicCompound::isDataDatatypeAlgebraicCompound() {
  if (lhs && rhs) {
    if (Ptr<Datatype> t = lhs->getDatatype())
      if (t->isSubtype(Datatype::mkDate(), getSemanticNode()->isPlContext()))
        return true;
    if (Ptr<Datatype> t1 = rhs->getDatatype())
      if (t1->isSubtype(Datatype::mkDate(), getSemanticNode()->isPlContext()))
        return true;
  }
  return false;
}


void AlgebraicCompound::collectSNode(SemanticTree *node) const {
  SemanticTree * n = new SemanticTree(SCathegory::AlgebraicCompound, this);
  CTREE(lhs);
  CTREE(rhs);
  setSemanticNode(n);
  node->addChild(n);
}


AlgebraicCompound::OperandTypeCathegory AlgebraicCompound::typeNumericCat(Ptr<Datatype> &t) {
  if (t->isDateDatatype())
    return DATE;
  if (t->isIntervalDatatype())
    return INTERVAL;
  else if (t->isNum())
    return NUMBER;
  else if (t->isCharVarchar())
    return VARCHAR;
  else
    return OTHER;
}



// 0 - вернуть lhs
// 1 - вернуть rhsm
// 2 - выбрать максимальный
// 3 - вернуть number (с дробной частью)
// 4 - вернуть date
const int AlgebraicCompound::stateTab[4][4] =
      //            date interval num  char
      /*date*/     {{ 3,    0,     0,   4  },
      /*interval*/  { 1,    2,     0,   -1 },
      /*num */      { 1,    1,     2,   3  },
      /*char*/      { 4,   -1,     3,   3  }};


Ptr<Datatype> AlgebraicCompound::getDatatype() const {
  if (beginedFrom(498794))
    cout << "";
  if (!lhs)
    return rhs->getDatatype();
  else if (!rhs)
    return lhs->getDatatype();
  Ptr<Datatype> lhsT = SubqueryUnwrapper::unwrap(lhs->getDatatype());
  Ptr<Datatype> rhsT = SubqueryUnwrapper::unwrap(rhs->getDatatype());

  if (!lhsT)
    return rhsT;
  else if (!rhsT)
    return lhsT;

  bool isPlContext = !getSemanticNode()->isSqlCode();

  switch (op) {
    case algebraic_compound::CONCAT:
      return Datatype::getConcatVarcharDatatype(lhsT.object(), rhsT.object(), getSemanticNode()->isPlContext(), getLLoc());
    case algebraic_compound::PLUS:
    case algebraic_compound::MINUS:
    case algebraic_compound::DIVIDE:
    case algebraic_compound::MULTIPLE: {
      int lhsCat = (int)typeNumericCat(lhsT);
      int rhsCat = (int)typeNumericCat(rhsT);
      if (lhsCat >= 0 && rhsCat >= 0) {
        int val = stateTab[lhsCat][rhsCat];
        switch (val) {
          case 0: return lhsT;
          case 1: return rhsT;
          case 2: {
            if (Datatype *t = Datatype::getMaximal(lhsT, rhsT, isPlContext))
              return t;
            else if (Datatype *t = Datatype::getMaximal(rhsT, lhsT, isPlContext))
              return t;
            else
              throw 999;
          }
          case 3: return Datatype::mkNumber();
          case 4: return Datatype::mkDate();
          default:
            break;
        }
      }
    }
    case algebraic_compound::DEGREE: {
      if (lhsT->isLinterNumber() && !rhsT->isLinterNumber())
        return lhsT;
      else if (rhsT->isLinterNumber() && !lhsT->isLinterNumber())
        return rhsT;
      break;
    }
    default: break;
  }

  int st = lhsT->isSubtype(rhsT.object(), getSemanticNode()->isPlContext());
//  Codestream str;
//  str << s::iloc(getLLoc()) << " " << lhsT << opToLogString() << rhsT << " ---> ";
//  str << (st > 0 ? rhsT : lhsT) << s::endl;
//  cout << str.str() << flush;

  if (st > 0)
    return rhsT;
  else
    return lhsT;

  return Datatype::mkDefault();
}


void AlgebraicCompound::translateToConsistantOperand()  {
  if (beginedFrom(218339,21))
    cout << "";

  if (getSemanticNode() == NULL)
    return;
  Ptr<Datatype> lT = SubqueryUnwrapper::unwrap(lhs->getDatatype());
  Ptr<Datatype> rT = SubqueryUnwrapper::unwrap(rhs->getDatatype());
  Ptr<Datatype> ret = SubqueryUnwrapper::unwrap(getDatatype());
  if (!lT || !rT || !ret)
    return;

  bool isPlContext = !getSemanticNode()->isSqlCode();

  switch (op) {
    case algebraic_compound::CONCAT: {
      CastCathegory lCat = lT->getCastCathegory(ret.object(), isPlContext, !isPlContext);
      CastCathegory rCat = rT->getCastCathegory(ret.object(), isPlContext, !isPlContext);
      if (lCat.explicitAll())
        CommonDatatypeCast::castAndReplace(isPlContext, lhs, lT, ret, lCat);
      if (rCat.explicitAll())
        CommonDatatypeCast::castAndReplace(isPlContext, rhs, rT, ret, lCat);
      break;
    }
    case algebraic_compound::PLUS:
    case algebraic_compound::MINUS:
    case algebraic_compound::DIVIDE:
    case algebraic_compound::MULTIPLE:
    case algebraic_compound::DEGREE: {

      int lhsCat = (int)typeNumericCat(lT);
      int rhsCat = (int)typeNumericCat(rT);
      if (lhsCat >= 0 && rhsCat >= 0) {
        int val = stateTab[lhsCat][rhsCat];
        if (val == 3) {
          CastCathegory lCat = lT->getCastCathegory(ret.object(), isPlContext, !isPlContext);
          CastCathegory rCat = rT->getCastCathegory(ret.object(), isPlContext, !isPlContext);
          if ((AlgebraicCompound::OperandTypeCathegory)lhsCat == VARCHAR)
            CommonDatatypeCast::castAndReplace(isPlContext, lhs, lT, ret, lCat);
          if ((AlgebraicCompound::OperandTypeCathegory)rhsCat == VARCHAR)
            CommonDatatypeCast::castAndReplace(isPlContext, rhs, rT, ret, rCat);
        }
        else if (val == 4) {
          if ((AlgebraicCompound::OperandTypeCathegory)lhsCat == VARCHAR) {
            CastCathegory lCat = lT->getCastCathegory(Datatype::mkNumber(), isPlContext, !isPlContext);
            CommonDatatypeCast::castAndReplace(isPlContext, lhs, lT, Datatype::mkNumber(), lCat);
          }
          if ((AlgebraicCompound::OperandTypeCathegory)rhsCat == VARCHAR) {
            CastCathegory rCat = rT->getCastCathegory(Datatype::mkNumber(), isPlContext, !isPlContext);
            CommonDatatypeCast::castAndReplace(isPlContext, rhs, rT, Datatype::mkNumber(), rCat);
          }
        }
      }
      break;
    }
    default:
      break;
  }
}

void Case::translateToConsistantOperand()  {
  if (!cases || !cases->size() || getSemanticNode() == NULL)
    return;

  if (beginedFrom(617283))
    cout << "";

  Ptr<Datatype> retType = SubqueryUnwrapper::unwrap(getDatatype());
  if (!retType || retType->isNull())
    return;

  bool isPlContext = !getSemanticNode()->isSqlCode();

  for (List<CaseIfThen>::reference caseif : *cases) {
    Ptr<Datatype> actType = SubqueryUnwrapper::unwrap(caseif->action->getDatatype());
    CastCathegory cat = retType->getCastCathegory(actType.object(), isPlContext, !isPlContext);
    if (!cat.equallyAlmost() && cat.lengthCathegory() != CastCathegory::VARCHAR) {
      cat.setCastRhsToLhs();
      CommonDatatypeCast::castAndReplace(isPlContext, caseif->action, actType, retType, cat);
    }
  }

  if (elseClause) {
    Ptr<Datatype> elseType = SubqueryUnwrapper::unwrap(elseClause->getDatatype());
    CastCathegory cat = retType->getCastCathegory(elseType.object(), isPlContext, !isPlContext);
    if (!cat.equallyAlmost() && cat.lengthCathegory() != CastCathegory::VARCHAR) {
      cat.setCastRhsToLhs();
      CommonDatatypeCast::castAndReplace(isPlContext, elseClause, elseType, retType, cat);
    }
  }
}

void pl_expr::Between::translateToConsistantOperand()  {
  if (!value)
    return;

  if (beginedFrom(494263))
    cout << "";

  Ptr<Datatype> valType = SubqueryUnwrapper::unwrap(value->getDatatype());
  if (!valType || valType->isNull())
    return;

  CastCathegory cat;
  Ptr<Datatype> boundType;
  bool isPlContext = !getSemanticNode()->isSqlCode();

  boundType = SubqueryUnwrapper::unwrap(low->getDatatype());
  cat = valType->getCastCathegory(boundType.object(), isPlContext, !isPlContext);
  if (!cat.equallyAlmost() && cat.lengthCathegory() != CastCathegory::VARCHAR) { //cat.explicitInReturn() || !cat.equallyBasePriority())
    cat.setCastRhsToLhs();
    CommonDatatypeCast::castAndReplace(isPlContext, low, boundType, valType, cat);
  }

  boundType = SubqueryUnwrapper::unwrap(high->getDatatype());
  cat = valType->getCastCathegory(boundType.object(), isPlContext, !isPlContext);
  if (!cat.equallyAlmost() && cat.lengthCathegory() != CastCathegory::VARCHAR) { // cat.explicitInReturn() || !cat.equallyBasePriority())
    cat.setCastRhsToLhs();
    CommonDatatypeCast::castAndReplace(isPlContext, high, boundType, valType, cat);
  }
}


CollectionAccess::CollectionAccess(CLoc l, Ptr<IdEntitySmart> _collectionRef, List<SqlExpr> *_indices)
  : GrammarBase(l), collectionRef(_collectionRef), indices(_indices) { checkIdEntitySmart(collectionRef); }


Ptr<Datatype> CollectionAccess::getDatatype() const {
  ResolvedEntity *d = 0;
  return (d = collectionRef->definition()) ? d->getDatatypeOfCollectionItem() : Ptr<Datatype>();
}


Sm::IsSubtypeValues CollectionAccess::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  ResolvedEntity *d = 0;
  return (d = collectionRef->definition()) ? d->isSubtype(supertype, plContext) : EXPLICIT;
}


void CollectionAccess::collectSNode(SemanticTree *n) const { SNODE(n); n->addChild(collectionRef->toSTreeRef(SCathegory::CollectionAccess)); CollectSNode(n, indices); }


bool CollectionAccess::getFieldRef(Ptr<Id> &field) {
  ResolvedEntity *d;
  return field && (d = collectionRef->definition()) && d->getFieldRef(field);
}


ResolvedEntity *CollectionAccess::getNextDefinition() const {
  ResolvedEntity *d;
  return (d = collectionRef->definition()) ? d->getNextDefinition() : (ResolvedEntity*)0;
}


void pl_expr::Submultiset::collectSNode(SemanticTree *n) const {
  SNODE(n);
  if (exprEntity)
    n->addChild(exprEntity->toSTreeRef(SCathegory::Entity));
  if (submultisetEntity)
    n->addChild(submultisetEntity->toSTreeRef(SCathegory::Entity));
}

void pl_expr::Submultiset::replaceChildsIf(ExprTr tr) {
  if (exprEntity)
    replace(tr, exprEntity);
  if (submultisetCathegory)
    replace(tr, submultisetCathegory);
}


bool ExtractExpr::identifyKeyword(Id *kw, ExtractedEntity *p) {
  if (!kw)
    return false;
  int x = -1;
  if (kw->isKeyword(SM_STRING("HOUR")))
    x = (int)ExtractedEntity::HOUR;
  else if (kw->isKeyword(SM_STRING("MINUTE")))
    x = (int)ExtractedEntity::MINUTE;
  else if (kw->isKeyword(SM_STRING("TIMEZONE_HOUR")))
    x = (int)ExtractedEntity::TIMEZONE_HOUR;
  else if (kw->isKeyword(SM_STRING("TIMEZONE_MINUTE")))
    x = (int)ExtractedEntity::TIMEZONE_MINUTE;
  else if (kw->isKeyword(SM_STRING("TIMEZONE_REGION")))
    x = (int)ExtractedEntity::TIMEZONE_REGION;
  else if (kw->isKeyword(SM_STRING("TIMEZONE_ABBR")))
    x = (int)ExtractedEntity::TIMEZONE_ABBR;
  if (x != -1) {
    if (p)
      *p = (ExtractedEntity)x;
    return true;
  }
  return false;
}


void ExtractExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(fromEntity); }


Ptr<Datatype> ExtractExpr::getDatatype() const {
  switch (extractedEntity_) {
    case ExtractedEntity::TIMEZONE_REGION:
    case ExtractedEntity::TIMEZONE_ABBR:
      return Datatype::mkVarchar2(128);
    default:
      return Datatype::mkNumber(5);
  }
  return Datatype::mkNumber(5);
}


void NewCall::collectSNode(SemanticTree *n) const {
  objectTypeRef->entity()->callArglist = argList.object();
  SemanticTree *node = objectTypeRef->toSNodeDatatypeRef(SCathegory::Constructor);
  SNODE(node);
  CollectSNode(node, argList);
  n->addChild(node);
}


bool NewCall::getFields(EntityFields &fields) const { return objectTypeRef && objectTypeRef->definition() && objectTypeRef->definition()->getFields(fields); }


Ptr<Datatype> NewCall::getDatatype() const {
  if (objectTypeRef)
    if (ResolvedEntity *def = objectTypeRef->definition())
      return def->getDatatype();
  return Ptr<Datatype>();
}


Sm::IsSubtypeValues NewCall::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (objectTypeRef)
    if (ResolvedEntity *def = objectTypeRef->definition())
      return def->isSubtype(supertype, plContext);
  return EXPLICIT;
}


bool NewCall::getFieldRef(Ptr<Id> &field) {
  return objectTypeRef && objectTypeRef->definition() && objectTypeRef->definition()->getFieldRef(field);
}


ResolvedEntity *NewCall::getNextDefinition() const {
  return (objectTypeRef && objectTypeRef->definition()) ? objectTypeRef->definition()->getNextDefinition() : (ResolvedEntity*)0;
}


SemanticTree *AnalyticFun::toSTreeBase() const {
  SemanticTree *n = new SemanticTree(SCathegory::AnalyticFun);
  SNODE(n);
  CollectSNode(n, callarglist);
  CTREE(orderByClause);
  n->unnamedDdlEntity = const_cast<AnalyticFun*>(this);
  return n;
}


Ptr<Datatype> CursorSqlProperties::getDatatypeByProperty(cursor_properties::Properties property) {
  switch (property) {
    case cursor_properties::CURSOR_FOUND   :
    case cursor_properties::CURSOR_ISOPEN  :
    case cursor_properties::CURSOR_NOTFOUND: return Datatype::mkBoolean();
    case cursor_properties::CURSOR_ROWCOUNT: return Datatype::mkInteger();
  }
  return Ptr<Datatype>();
}


CursorProperties::CursorProperties(CLoc l, Ptr<IdEntitySmart> _reference, cursor_properties::Properties cursor_property)
  : GrammarBase(l), RefAbstract(_reference), CursorSqlProperties(cursor_property) {}


CursorSqlProperties::CursorSqlProperties(CLoc l, cursor_properties::Properties cursor_property)
  : GrammarBase(l), property(cursor_property) {}

CursorSqlProperties::CursorSqlProperties(cursor_properties::Properties cursor_property)
  : property(cursor_property) {}



SemanticTree *CursorProperties::toSTreeBase() const {
  SemanticTree *n = reference->toSTreeRef(SCathegory::Expr_CursorProperty );
  n->unnamedDdlEntity = const_cast<CursorProperties*>(this);
  return n;
}


void CursorProperties::collectSNode(SemanticTree *n) const { ANODE2(n, this); }
void CursorSqlProperties::collectSNode(SemanticTree *n) const { setSemanticNode(n); }


HostCursorPropertiesExpr::HostCursorPropertiesExpr(CLoc l, Ptr<IdEntitySmart> _reference, cursor_properties::Properties cursor_property)
  : GrammarBase(l), RefAbstract(_reference), property(cursor_property) {}


SemanticTree *HostCursorPropertiesExpr::toSTreeBase() const {
  SemanticTree *n = reference->toSTreeRef(SCathegory::Expr_HostCursorProperty);
  n->unnamedDdlEntity = const_cast<HostCursorPropertiesExpr*>(this);
  return n;
}


void HostCursorPropertiesExpr::collectSNode(SemanticTree *n) const { ANODE2(n, this); }


SemanticTree *RefHostExpr::toSTreeBase() const {
  SemanticTree *node = reference->toSTreeRef(SCathegory::Expr_HostSecondRef);
  node->unnamedDdlEntity = const_cast<RefHostExpr*>(this);
  return node;
}

bool RefHostExpr::getFieldRef(Ptr<Id> &field) {
  if (Sm::checkToRowidPseudocolumn(field))
    return true;
  return RefAbstract::getFieldRef(field);
}


void RefHostExpr::collectSNode(SemanticTree *n) const { ANODE2(n, this); }

void RefHostExpr::replaceChildsIf(ExprTr tr) {
  replace(tr, reference);
  if (secondRef)
    replace(tr, secondRef);
}


SemanticTree *OutherJoinExpr::toSTreeBase() const {
  SemanticTree *n = reference->toSTreeRef(SCathegory::Expr_OutherJoin);
  n->unnamedDdlEntity = const_cast<OutherJoinExpr*>(this);
  return n;
}


void OutherJoinExpr::collectSNode(SemanticTree *n) const { ANODE2(n, this); }


RefExpr* RefExpr::toSimpleFunctionCall() const {
  if (ResolvedEntity *def = refDefinition())
    if (def->toSelfFunction())
      return const_cast<RefExpr*>(this);
  return 0;
}


Id *RefAbstract::getSelectedFieldName() const {
  Id* ent = refEntity();
  return ent->callArglist ? nullptr : ent;
}

Subquery *RefAbstract::getSelectQuery() {
  if (ResolvedEntity *def = refDefinition())
    return def->getSelectQuery();
  return NULL;
}

Sm::IsSubtypeValues RefAbstract::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (ResolvedEntity *def = refDefinition())
    return def->isSubtype(supertype, plContext);
  return EXPLICIT;
}


Exception *RefAbstract::getExceptionDef() const {
  if (ResolvedEntity *def = refDefinition())
    return def->getExceptionDef();
  return 0;
}


void RefAbstract::setDateLiteral() { refEntity()->setDateLiteral(true); }

void RefAbstract::setSQuotedLiteral() { refEntity()->setSQuoted(); }


bool RefAbstract::getFieldRef(Ptr<Id> &field) {
  if (field) {
    field->setCalledArgument();
    ResolvedEntity *d = refDefinition();
    if (d)
      return d->getFieldRef(field);
    semanticResolve();
    return (d = refDefinition()) && d->getFieldRef(field);
  }
  return false;
}


bool RefExpr::isCursorVariable() const {
  ResolvedEntity *def = refDefinition();
  return def && def->isCursorVariable();
}


bool RefExpr::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  if (ResolvedEntity *def = refDefinition())
    return def->isExactlyEquallyByDatatype(oth);
  return false;
}


Sm::IsSubtypeValues RefExpr::isSubtype(ResolvedEntity *v, bool pl) const {
  if (ResolvedEntity *d = refDefinition())
    return d->isSubtype(v, pl);
  return EXPLICIT;
}


bool RefExpr::isFuncall(bool includeSys) const {
  if (ResolvedEntity *def = refDefinition())
    return (def->isFunction() && (includeSys || !def->isElementaryLinterFunction())) || def->isMethod();
  return false;
}


Ptr<CallArgList> RefExpr::callArglist() const {
  if (Id *e = refEntity())
    return e->callArglist;
  return 0;
}


bool RefExpr::isUnsupportedSysFuncall() {
  if (ResolvedEntity *def = refDefinition())
    return def->isUnsupportedSysFuncall();
  return false;
}


bool RefExpr::isException() const {
  if (ResolvedEntity *def = refDefinition())
    return def->isException();
  return false;
}


void RefExpr::translatedNames(std::vector<string> **str) {
  if (ResolvedEntity *def = refDefinition())
    def->translatedNames(str);
}


Ptr<Datatype> TimeExpr::getDatatype() const {
  if (interval) {
    Ptr<Datatype> t;
    switch (interval->intervalCathegory) {
      case TimeExprInterval::DAY_TO_SECOND:
        t = Datatype::mkIntervalDayToSecond();
        break;
      case TimeExprInterval::YEAR_TO_MONTH:
        t = Datatype::mkIntervalYearToMonth();
        break;
    }
    t->precision = interval->day_or_year;
    t->scale     = interval->second;
    return t;
  }
  else if (base)
    return base->getDatatype();
  return Ptr<Datatype>();
}


void TimeExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(base); CTREE(timezone); }


TrimFromExpr::TrimFromExpr(CLoc l, SqlExpr *_charExpr, TrimFromExpr::ExtractedEntity ent)
  : GrammarBase(l),
    charExpr(_charExpr),
    extractedEntity_(ent) {}


bool TrimFromExpr::identifyKeyword(Ptr<Id> kw, TrimFromExpr::ExtractedEntity *p) {
  if (!kw)
    return false;
  int x = -1;
  if (kw->isKeyword(SM_STRING("LEADING")))
    x = (int)LEADING;
  else if (kw->isKeyword(SM_STRING("TRAILING")))
    x = (int)TRAILING;
  else if (kw->isKeyword(SM_STRING("BOTH")))
    x = (int)BOTH;
  if (x != -1) {
    if (p)
      *p = (ExtractedEntity)x;
    return true;
  }
  return false;
}


void TrimFromExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(charExpr); CTREE(fromEntity); }


Id *RowIdExpr::getSelectedFieldName() const {
  if (!id) {
    id = new Id("ROWID", const_cast<RowIdExpr*>(this));
    id->loc(getLLoc());
  }
  return id.object();
}


Id *RowNumExpr::getSelectedFieldName() const {
  if (!id) {
    id = new Id("ROWNUM", const_cast<RowNumExpr*>(this));
    id->loc(getLLoc());
  }
  return id.object();
}

Ptr<Datatype> AsteriskRefExpr::getDatatype() const { return reference->getDatatype(); }

AsteriskRefExpr::AsteriskRefExpr(CLoc l, Ptr<IdEntitySmart> _reference)
  : GrammarBase(l), RefAbstract(_reference) {}

AsteriskExpr::AsteriskExpr(CLoc l)
  : GrammarBase(l) {}


SemanticTree *AsteriskRefExpr::toSTreeBase() const {
  SemanticTree *n = reference->toSTreeRef(SCathegory::Expr_Asterisk);
  n->unnamedDdlEntity = const_cast<AsteriskRefExpr*>(this);
  return n;
}


SemanticTree *BulkRowcountExpr::toSTreeBase() const {
  SemanticTree *n = reference->toSTreeRef(SCathegory::BulkRowcount);
  n->unnamedDdlEntity = const_cast<BulkRowcountExpr*>(this);
  return n;
}


BulkRowcountExpr::BulkRowcountExpr(CLoc l, Ptr<NumericValue> _argval) : GrammarBase(l), RefAbstract(Ptr<IdEntitySmart>()), argval(_argval) {}


TrimFromExpr::TrimFromExpr(CLoc l, CLoc chExprLoc, Id *_charExpr, SqlExpr *_fromEntity, TrimFromExpr::ExtractedEntity ent)
  : GrammarBase(l),
    charExpr(new Sm::RefExpr(chExprLoc, _charExpr)),
    fromEntity(_fromEntity),
    extractedEntity_(ent) {}


void PriorExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(prior); }


void CursorExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(cursor); }


void CursorExpr::replaceChildsIf(ExprTr tr) { replace(tr, cursor); }


RefExpr* Brackets::toSimpleFunctionCall() const {
  if (isNot()) {
    const_cast<Brackets*>(this)->toggleNot();
    const_cast<Brackets*>(this)->brackets->toggleNot();
  }
  return brackets->toSimpleFunctionCall();
}


void Brackets::collectSNode(SemanticTree *n) const {
  CTREE(brackets);
  SNODE(brackets->getSemanticNode());
}

Ptr<PlExpr> Brackets::unwrapBrackets(int *isNot) const {
  if (isNot)
    *isNot = *isNot ^ isNotAsInt();
  return brackets->unwrapBrackets(isNot);
}


void UnaryPlus::collectSNode(SemanticTree *node) const {
  SemanticTree * n = new SemanticTree(SCathegory::UnaryOp, this);
  SNODE(n);
  CTREE(op);
  node->addChild(n);
}


void UnaryMinus::collectSNode(SemanticTree *node) const {
  SemanticTree * n = new SemanticTree(SCathegory::UnaryOp, this);
  SNODE(n);
  CTREE(op);
  node->addChild(n);
}

void UnaryMinus::linterDefinition(Sm::Codestream &s) {
  if (NumericValue *val = op->toSelfNumericValue())
    if (val->isStringLiteral()) {
      val->resetStringType();
      s << (s.isProc() ? '"' : '\'');
      s << '-' << op;
      s << (s.isProc() ? '"' : '\'');
      val->setStringType();
      return;
    }
  s << '-' << op;
}

Ptr<PlExpr> UnaryMinus::deepCopy()  {
  return new UnaryMinus(getLLoc(), op->deepCopy());
}


Case::Case(CLoc l, List<CaseIfThen> *_cases, PlExpr *_elseClause, PlExpr *src)
  : GrammarBase(l), source(src), cases(_cases), elseClause(_elseClause) {}


Case::Case(FLoc l, Ptr<PlExpr> caseIf, Ptr<PlExpr> caseThen, Ptr<PlExpr> caseElse, SemanticTree *n)
  : GrammarBase(l), cases(new List<CaseIfThen>(new CaseIfThen(l, caseIf, caseThen))),
    elseClause(caseElse)
{
  if (n)
    setSemanticNode(n);
}


void Case::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(source); CollectSNode(n, cases); CTREE(elseClause); }


void Cast::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(castedExpr); ANODE(toType); }


void CastMultiset::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(castedQuery); ANODE(toType); }


void CastMultiset::replaceChildsIf(ExprTr tr) { replace(tr, castedQuery, toType); }


void pl_expr::OfTypes::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(entity); CollectSNode(n, types); }


void pl_expr::RegexpLike::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(sourceChar); CTREE(pattern); }


void pl_expr::Like::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(char1); CTREE(char2); CTREE(esc_char); }


void pl_expr::Between::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(value); CTREE(low); CTREE(high) }


pl_expr::MemberOf::MemberOf(CLoc l, Ptr<SqlExpr> _memberItem, Ptr<IdEntitySmart> _nestedTableRef)
  : GrammarBase(l), memberedItem(_memberItem), nestedTableRef(_nestedTableRef)
{
  if (!nestedTableRef || nestedTableRef->empty())
    throw 999;
}

void pl_expr::MemberOf::collectSNode(SemanticTree *n) const {
  SNODE(n); CTREE(memberedItem); n->addChild(nestedTableRef->toSTreeRef(SCathegory::NestedTableInstance));
}


RefExpr* pl_expr::BracketsLogicalExpr::toSimpleFunctionCall() const {
  if (isNot()) {
    const_cast<BracketsLogicalExpr*>(this)->toggleNot();
    const_cast<BracketsLogicalExpr*>(this)->brackets->toggleNot();
  }
  return brackets->toSimpleFunctionCall();
}


void pl_expr::BracketsLogicalExpr::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(brackets); }

Ptr<PlExpr> pl_expr::BracketsLogicalExpr::unwrapBrackets(int *isNot) const {
  if (isNot)
    *isNot = *isNot ^ isNotAsInt();
  return brackets->unwrapBrackets(isNot);
}


void pl_expr::Exists::collectSNode(SemanticTree *n) const {
  SemanticTree *node = new SemanticTree(SCathegory::Exists);
  n->addChildForce(node);
  SNODE(node); CTREE2(node, query);
}


void pl_expr::Exists::replaceChildsIf(ExprTr tr) { replace(tr, query); }


void pl_expr::IsInfinite::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(expr); }


void pl_expr::IsNan::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(expr); }


void pl_expr::IsNull::collectSNode(SemanticTree *n) const { SNODE(n); CTREE(expr); }


pl_expr::IsEmpty::IsEmpty(CLoc l, Ptr<IdEntitySmart> _entityRef)
  : GrammarBase(l), entityRef(_entityRef)
{
  if (!entityRef || entityRef->empty())
    throw 999;
}

void pl_expr::IsEmpty::collectSNode(SemanticTree *n) const {
  SemanticTree *node = entityRef->toSTreeRef(SCathegory::Entity);
  SNODE(node);
  n->addChild(node);
}


CaseIfThen::CaseIfThen(CLoc l, PlExpr *cond, PlExpr *act)
  : GrammarBaseSmart(l), condition(cond), action(act) {}


CaseIfThen::CaseIfThen(CLoc l, Ptr<PlExpr> cond, Ptr<PlExpr> act)
  : GrammarBaseSmart(l), condition(cond), action(act) {}

