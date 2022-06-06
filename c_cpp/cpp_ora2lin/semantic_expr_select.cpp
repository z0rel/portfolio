#include "semantic_expr_select.h"
#include "semantic_collection.h"
#include "semantic_function.h"
#include "syntaxer_context.h"
#include "model_context.h"

extern SyntaxerContext syntaxerContext;

using namespace Sm;


void Subquery::pullUpOrderGroup(Subquery *child) {
  if (child) {
    if (child->groupBy) {
      groupBy = child->groupBy;
      child->groupBy = 0;
    }
    if (child->orderBy) {
      orderBy = child->orderBy;
      child->orderBy = 0;
    }
  }
}


void Subquery::translateFieldTypeIfNeed(Ptr<Datatype> fldT, Ptr<Datatype> intoT, int pos)
{
  if (intoT && fldT) {
    if (beginedFrom(86572))
      cout << "";
    if (intoT->isCollectionType())
      intoT = intoT->tidDef()->getElementDatatype();
    if (intoT->isRowTypeOf()) {
      EntityFields fields;
      intoT->getFields(fields);
      EntityFields::iterator it = fields.begin();
      std::advance(it, pos);
      intoT = ResolvedEntity::getLastConcreteDatatype((*it)->getDatatype());
    }
    CastCathegory val = intoT->getCastCathegory(fldT, true, true); // true - т.к. нужно точное приведение к переменной plsql
    val.setCastRhsToLhs();
    if (val.explicitInSelectLhsIntoRhsField()) {
      translateFieldToDatatype(pos, fldT, intoT, val);
    }
  }
}


void Subquery::translateSelectedFieldsDatatypesToIntoListTypes(IntoList::value_type* into) {
  if (beginedFrom(86572))
    cout << "";
  if (SelectSingle *s = this->toSelfSelectSingle())
    if (s->queryBlock->from && s->queryBlock->from->size() == 1) {
      if (FromSingle *f = (*(s->queryBlock->from->begin()))->toSelfFromSingle())
        if (f->reference)
          if (FromTableReference *ref = f->reference->toSelfFromTableReference())
            if (ref->id)
              if (ResolvedEntity *var = ref->id->definition())
                if (var->isFieldForMakestr())
                  return;
    }
  IntoList::value_type *l = into ? into : intoList().object();
  if (!l || l->empty())
    return;
  EntityFields flds;
  getFields(flds);
  EntityFields::iterator fldIt = flds.begin();
  int pos = 0;
  if (l->size() != flds.size()) {
    if (l->size() == 1) { /* INTO курсор или коллекция */
      EntityFields intoFlds;
      l->front()->getFields(intoFlds);
      for (EntityFields::iterator intoFldIt = intoFlds.begin(); intoFldIt != intoFlds.end() && fldIt != flds.end(); ++intoFldIt, ++fldIt, ++pos)
        translateFieldTypeIfNeed(ResolvedEntity::getLastConcreteDatatype((*fldIt)->getDatatype()),
                                 ResolvedEntity::getLastConcreteDatatype((*intoFldIt)->getDatatype()), pos);
    }
    else
      throw 999;
  }
  else /* INTO список аргументов */
    for (List<RefAbstract>::iterator intoIt = l->begin(); intoIt != l->end() && fldIt != flds.end(); ++intoIt, ++fldIt, ++pos) {

      translateFieldTypeIfNeed(ResolvedEntity::getLastConcreteDatatype((*fldIt)->getDatatype() ),
                               ResolvedEntity::getLastConcreteDatatype((*intoIt)->getDatatype()), pos);
    }
}

void Subquery::convertUnionFieldTypes(EntityFields &intoFlds, bool isFirstQuery) {
  int pos = 0;
  EntityFields subFlds;
  getFields(subFlds);

  EntityFields::iterator subFldIt = subFlds.begin();
  EntityFields::iterator intoFldIt = intoFlds.begin();
  for ( ; intoFldIt != intoFlds.end() ; ++intoFldIt, ++subFldIt, ++pos) {
    Ptr<Datatype> newType = ResolvedEntity::getLastConcreteDatatype((*intoFldIt)->getDatatype());
    Ptr<Datatype> castType = ResolvedEntity::getLastConcreteDatatype((*subFldIt)->getDatatype());
    CastCathegory cat;
    if (!newType) {
      cout << "error: unresolved new type " << getLLoc() << endl;
      continue;
    }
    if (isFirstQuery && newType->isNull()) {
      // NULL в верхнем подзапросе union приводится к INT
      newType = Datatype::mkInteger();
      cat.setCastUnion();
    }
    else
      cat = newType->getCastCathegory(castType, false, true);
    if (cat.castUnion()) {
      if (ResolvedEntity *def = (*subFldIt)->definition())
        def->setFieldNumber(pos);
      translateFieldToDatatype(pos, castType, newType, cat);
    }
  }
}

void Subquery::formalTranslations() {
  if (isRootQuery()) {
    if (Subquery::IntoList l = intoList())
      if (!l->empty())
        translateSelectedFieldsDatatypesToIntoListTypes(l);
  }
}

void SelectSingle::formalTranslations() {
  queryBlock->extractLimit();
  Subquery::formalTranslations();
}


void SelectSingle::translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory cat) {
  if (queryBlock)
    queryBlock->translateFieldToDatatype(i, oldT, newT, cat);
}

void SelectSingle::extractLimitInQueryBlocks() { queryBlock->extractLimit(); }

void UnionQuery::translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory castedCathegory) {
  if (lhs)
    lhs->translateFieldToDatatype(i, oldT, newT, castedCathegory);
  if (rhs)
    rhs->translateFieldToDatatype(i, oldT, newT, castedCathegory);
}

void UnionQuery::extractLimitInQueryBlocks() {
  lhs->extractLimitInQueryBlocks();
  rhs->extractLimitInQueryBlocks();
}

void UnionQuery::initQueryFieldPos() {
  if (lhs)
    lhs->initQueryFieldPos();
  if (rhs)
    rhs->initQueryFieldPos();
}

void UnionQuery::convertUnionFieldTypes(EntityFields &intoFlds, bool /*isFirstQuery*/) {
  lhs->convertUnionFieldTypes(intoFlds, !isUnionChild());
  rhs->convertUnionFieldTypes(intoFlds, false);
}

void UnionQuery::formalTranslations() {
  if (beginedFrom(375737))
    cout << "";
  if (!isUnionChild_) {
    // Сконвертировать типы данных во всех ветках объединения
    EntityFields intoFlds;
    getFields(intoFlds);
    convertUnionFieldTypes(intoFlds, true);
  }

  Subquery::formalTranslations();
}


void SelectBrackets::translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory cat) {
  if (subquery) subquery->translateFieldToDatatype(i, oldT, newT, cat);
}

void SelectBrackets::sqlDefinition(Codestream &str) {
  str << s::obracket << subquery << s::cbracket;
}


void SelectBrackets::sqlDefinitionForNthField(Codestream &str, int fieldPos) {
  str << s::obracket;
  subquery->sqlDefinitionForNthField(str, fieldPos);
  str << s::cbracket;
}


void SelectBrackets::replaceChildsIf(ExprTr tr) { replace(tr, subquery, forUpdate, orderBy, groupBy); }


void ForUpdateClause::collectSNode(SemanticTree *n) const {
  if (entityList)
    for (const EntityList::value_type &i : *entityList)
      n->addChild(i->toSTreeRef(SCathegory::SelectedField));
}


void GroupingSetsClause::collectCube(const Ptr<GroupingSetsClause::CubeT> &cube, SemanticTree *n) const {
  if (cube)
    for (CubeT::const_iterator it = cube->begin(); it != cube->end(); ++it)
      if (*it)
        for (CubeT::value_type::dereferenced_type::const_iterator it2 = (*it)->begin(); it2 != (*it)->end(); ++it2)
          CTREE(it2->object());
}


void FactoringItem::linterReference(Codestream &str) {
  str << queryName;
}

bool FactoringItem::getFieldRefAsTableName(Ptr<Id> &field) {
  if (field && queryName && queryName->normalizedString() == field->normalizedString()) {
    field->definition((FactoringItem*)this);
    return true;
  }
  return false;
}


bool FactoringItem::getFields(EntityFields &fields) const {
  if (columnAliases) {
    connectAliasWithQuery();
    for (List<Id>::const_iterator it = columnAliases->begin(); it != columnAliases->end(); ++it)
      fields.push_back(*it);
  }
  else
    return subquery && subquery->getFields(fields);
  return true;
}


Ptr<Id> SelectedField::getName() const {
  if (alias_)
    return alias_;
  return fieldName;
}

bool SelectedField::skipLevelNamespaceUpdating() const {
  if (Ptr<Id> n = expr_->getName())
    if (n->squoted())
      return true;
  return expr_->skipSelectedFieldExprNamespaceUpdating() || getAlias();
}

bool SelectList::isSingleField() {
  if (isAsterisk_ || !fields)
    throw 999;

  if (fields->size() != 1)
    return false;

  SelectedField *f = fields->front();

  EntityFields fields;
  if (AsteriskRefExpr *expr = f->expr()->toSelfAsteriskRefExpr()) {
    expr->getFields(fields);
    return fields.size() == 1;
  }
  else if (AsteriskExpr *ae = f->expr()->toSelfAsteriskExpr()) {
    ae->getFields(fields);
    return fields.size() == 1;
  }
  return true;
}

bool QueryBlock::isSingleField() {
  if (!selectList)
    return 0;
  if (selectList->isAsterisk()) {
    EntityFields f;
    getFields(f);
    return f.size() == 1;
  }
  else
    return selectList->isSingleField();
}


Ptr<Datatype> SelectedField::getDatatype() const {
  if (expr_) {
    if (ResolvedEntity *d = expr_->getNextDefinition())
      if (d == static_cast<const ResolvedEntity*>(this))
        return 0;
    Ptr<Datatype> t = expr_->getDatatype();
    if (t)
      if (ResolvedEntity *d = t->getNextDefinition())
        if (QueryBlock *qb = d->toSelfQueryBlock()) {
            EntityFields flds;
            qb->getFields(flds);
            if (flds.size() == 1)
              return (*(flds.begin()))->getDatatype();
        }
    return t;


  }
  return 0;
}

ResolvedEntity *SelectedField::getFieldDDLContainer() const {
  return expr_->getNextDefinition()->getFieldDDLContainer();
}


SelectList::SelectList(CLoc l, SelectList::SelectedFields *_fields)
  : GrammarBaseSmart(l), fields(_fields), isAsterisk_(!_fields)
{
  if (fields)
    for (SelectedFields::iterator it = fields->begin(); it != fields->end(); ++it)
      if (Ptr<Id> n = (*it)->toAsterisk())
        asterisksList.push_back(n); //
}


Sm::IsSubtypeValues SelectList::isSubtypeSingle(Ptr<ResolvedEntity> supertype, bool plContext) const {
  if (!isAsterisk_ && fields && fields->size() == 1) {
    Ptr<SelectedField> f = fields->front();
    if (Ptr<Datatype> t = f->expr()->tryResolveDatatype())
      return t->isSubtype(supertype, plContext);
  }
  return Sm::IsSubtypeValues::EXPLICIT;
}

SemanticTree *SelectList::toSTree() const {
  if (isAsterisk_ || semanticNode)
    return 0;

  semanticNode = new SemanticTree(SCathegory::SelectedField, SemanticTree::NEW_LEVEL, this);
  semanticNode->setIsList();
  CollectSNode(semanticNode, fields);
  setSemanticNode(semanticNode);
  return semanticNode;
}


FromSingle::FromSingle(CLoc l, Ptr<QueriedTable> _reference, Ptr<FlashbackQueryClause> _flashback, Ptr<Id> _alias)
  : GrammarBase(l), reference(_reference), flashback(_flashback), alias(_alias) {
  if (alias)
    alias->definition(this);
}

void FromSingle::collectSNode(SemanticTree *n) const {
  if (!referenceName)
    initReferenceName();

  SemanticTree *node = 0;
  if (reference) {
    node = reference->toSTree();
    n->addChildForce(node);
    setSemanticNode(node);
    node->setIsFromNode();
    if (alias) {
      SemanticTree *a = alias->toSNodeDecl(SCathegory::Table, this);
      a->setIsFromNode();
      node->alias(a);
    }
  }
  else if (alias) {
    node = alias->toSNodeDecl(SCathegory::Table, this);
    n->addChildForce(node);
    setSemanticNode(node);
    node->setIsFromNode();
  }
  else
    return;
  CTREE2(node, flashback);
  CollectSNode(node, partitionExprList);
}


SemanticTree *FromTableDynamic::toSTreeBase() const {
  SemanticTree *n = new SemanticTree(SCathegory::FromTableDynamic, SemanticTree::DECLARATION);
  CTREE(dynamicTable)
      return n;
}


bool FromTableDynamic::getFieldRefAsTableName(Ptr<Id> &field) {
  if (field) {
    if (Ptr<Id> name = getName()) {
      if (name->normalizedString() == field->normalizedString()) {
        field->definition((FromTableDynamic*)this);
      }
    }
  }
  return false;
}


FromTableReference::FromTableReference(Ptr<Id2> _id, Ptr<Id> _dblink, Ptr<Tablesample> _sample, CLoc l)
  : GrammarBase(l), QueriedTable(), id(_id), dblink(_dblink), sample(_sample)
{
  if (id && id->entity())
    tableReference = new Id(*(id->entity()));
  else
    tableReference = new Id(string(0, 1));
  tableReference->definition(this);
}

bool FromTableReference::isDual() const {
  if (id)
    if (ResolvedEntity *def = id->definition())
      return def->isDual();
  return false;
}


FromTableSubquery::FromTableSubquery(CLoc l, Ptr<Subquery> q, bool _hasThe, bool _isWith)
  : GrammarBase(l), QueriedTable(), subquery(q), hasThe(_hasThe), isWith(_isWith) {}

bool FromTableSubquery::getFieldRefAsTableName(Ptr<Id> &field) {
  if (field)
    if (Ptr<Id> name = subquery->getName())
      if (name->normalizedString() == field->normalizedString()) {
        field->definition((FromTableSubquery*)this);
        return true;
      }
  return false;
}

SemanticTree *FromTableSubquery::toSTreeBase() const {
  if (subquery)
    return subquery->toSTree();
  else
    return 0;
}

void FromTableSubquery::replaceChildsIf(ExprTr tr) { replace(tr, subquery); }


void OrderBy::collectSNode(SemanticTree *node) const {
  SemanticTree *n = new SemanticTree(SCathegory::OrderBy, SemanticTree::NEW_LEVEL);
  SemanticTree *partitionNode = new SemanticTree(SCathegory::PartitionList);
  n->addChildForce(partitionNode);
  CollectSNode(partitionNode, partitionBy);
  CollectSNode(n, orderList);
  node->addChild(n);
}

void QueryBlock::translateFieldToDatatype(int i, Ptr<Datatype> &oldT, Ptr<Datatype> &newT, CastCathegory cat) {
  if (beginedFrom(86572))
    cout << "";
  if (selectList && selectList->fields && selectList->fields->size()) {
    if (selectList->isAsterisk_ || selectList->fields->front()->expr()->isAsterisk()) {
      if (selectList->fields->size() > 1) {
        cout << "error: selected asterisk field is not single field: " << getLLoc() << endl;
        return;
      }
      EntityFields flds;
      getFields(flds);
      FLoc l = cl::emptyFLocation();
      if (selectList->fields->empty())
        l = getLLoc();
      else {
        Ptr<SelectedField> asteriskFld = selectList->fields->front();
        l = asteriskFld->getLLoc();
        asteriskFld->setSemanticNode(0);
        asteriskFld->expr_->setSemanticNode(0);
      }

      if (!selectedListNode_) {
        selectedListNode_ = new SemanticTree(SCathegory::SelectedField, SemanticTree::NEW_LEVEL);
        selectedListNode_->setIsList();
        if (SemanticTree *n = getSemanticNode())
          n->addChildForce(selectedListNode_);
        else
          throw 999;
      }
      else {
        if (SemanticTree *asteriskNode = selectedListNode_->childs.front())
           delete asteriskNode;
        selectedListNode_->childs.clear();
      }
      for (SelectList::SelectedFields::value_type &v : *(selectList->fields))
        syntaxerContext.model->delayDeletedEntities.push_back(v.object());

      Ptr<IdRef> tableRef;
      if (AsteriskRefExpr *asteriskExpr = selectList->fields->front()->expr()->toSelfAsteriskRefExpr()) {
        tableRef = asteriskExpr->reference;
      }

      selectList->fields->clear();
      selectList->isAsterisk_ = false;
      int posNum = 0;
      for (EntityFields::value_type &fld : flds) {
        Ptr<SelectedField> newField = new SelectedField(l, new RefExpr(l, new Id(*fld)));
        newField->fieldNumber_ = posNum;
        newField->asteriksRef = tableRef;
        selectList->fields->push_back(newField);
        newField->collectSNode(selectedListNode_);
        ++posNum;
      }
    }
    Ptr<SelectedField> f = (*(selectList->fields))[i];
    //if (f->fieldNumber_ != i)
    //  throw 999;
    Ptr<SqlExpr> expr = f->expr_;
    if (!f->alias_)
      if (Id *name = f->expr_->getSelectedFieldName()) {
        // если то же имя в пространстве имен запроса соответствует другому определению - кинуть исключение
        ResolvedEntity *d = f->expr_->getNextDefinition();
        if (!d || !d->getNextDefinition() || !d->isFieldForMakestr())
          f->alias_ = new Id(name->toNormalizedString(), f);
      }
    if (!cat.explicitInSelectLhsIntoRhsField() && !cat.castUnion())
      throw 999;
    Ptr<SqlExpr> copy = f->expr_;
    (void)copy;
    cat.setSqlCastState();
    Ptr<PlExpr> castedExpr = CommonDatatypeCast::cast(expr.object(), oldT.object(), newT.object(), cat);
    if (SqlExpr *expr = castedExpr->toSelfSqlExpr())
      f->expr_ = expr;
    else
      throw 999;
  }
  else
    throw 999;
}

bool QueryBlock::hasDynamicQueryExpr() const {
  if (beginedFrom(135242))
    cout << "";
  if (limit && limit->toSelfRefAbstract()) {
    if (ResolvedEntity *def = limit->toSelfRefAbstract()->refDefinition())
      return def->isVariable();
  }
  return false;
}

void QueryBlock::replaceChildsIf(ExprTr tr) {
  if (where)
     where->__flags__.setWhereExpr();
  replace(tr, factoringList, selectList, intoList, from, where, tailSpec, orderBy);
}

QueryPseudoField::QueryPseudoField(Ptr<Id> _fieldName, ResolvedEntity *_owner)
  : fieldName(_fieldName), owner_(_owner) { fieldName->definition(this); }

QueryPseudoField::QueryPseudoField(Ptr<Id> _fieldName, ResolvedEntity *_owner, ResolvedEntity *fieldDefinition)
  : fieldName(_fieldName), owner_(_owner), fieldDefinition_(fieldDefinition) { fieldName->definition(this); }

bool QueryPseudoField::isNonblockPseudoField() const {
  if (fieldDefinition_ && fieldDefinition_->ddlCathegory() == ResolvedEntity::Datatype_)
    return true;
  return false;
}

void QueryPseudoField::checkDef() const {
  if (fieldDefinition_) {
    fieldDefinition_->semanticResolve();
    return;
  }
  ResolvedEntity *o = owner_;
  o->semanticResolve();
}

Ptr<Datatype> QueryPseudoField::getDatatype() const {
  checkDef();
  if (isColumnValue) {
    if (!fieldDefinition_)
      throw 999;
    Ptr<Datatype> t = SubqueryUnwrapper::unwrap(fieldDefinition_->getDatatype());
    if (!t)
      throw 999;
    ResolvedEntity *d  = t->tid->entity()->definition();
    if (!d)
      throw 999;
    if (Type::CollectionType* c = d->toSelfCollectionType())
      return c->mappedType();
    else if (d->toSelfFundamentalDatatype())
      return t;
    throw 999;
  }
  return fieldDefinition_ ? fieldDefinition_->getDatatype() : Ptr<Datatype>();
}

bool QueryPseudoField::getFieldRef(Ptr<Id> &field) {
  checkDef();
  return fieldDefinition_ && fieldDefinition_->getFieldRef(field);
}

Sm::IsSubtypeValues QueryPseudoField::isSubtype(ResolvedEntity *supertype, bool plContext) const {
  if (eqByVEntities(supertype))
    return EXACTLY_EQUALLY;
  return fieldDefinition_->isSubtype(supertype, plContext);
}

bool QueryPseudoField::isExactlyEquallyByDatatype(ResolvedEntity *oth) {
  checkDef();
  return fieldDefinition_ ? fieldDefinition_->isExactlyEquallyByDatatype(oth) : false;
}

void QueryPseudoField::linterDefinition(Codestream &str) {
  if (isColumnValue)
    throw 999;
  str << fieldName;
}

void QueryPseudoField::linterReference(Codestream &str) {
  if (isColumnValue)
    str << "VAL__";
  else
    translateAsFunArgumentReference(str);
}

void QueryPseudoField::sqlReference(Codestream &str) {
  if (isColumnValue)
    str << "VAL__";
  else
    str << fieldName;
}


FromJoin::FromJoin(CLoc l, Ptr<From> _lhs, Ptr<Join> _op, Ptr<From> _rhs)
  : GrammarBase(l), lhs(_lhs), op(_op), rhs(_rhs) {}


FlashbackQueryClause::FlashbackQueryClause(CLoc l, flashback_query::ScnOrTimestamp aot, Ptr<SqlExpr> e, flashback_query::ScnOrTimestamp vbt, Ptr<SqlExpr> r1, Ptr<SqlExpr> r2)
  : GrammarBaseSmart(l), asOf(aot, e), versionBetweenAsOf(vbt, r1, r2) {}

FlashbackQueryClause::FlashbackQueryClause(CLoc l, flashback_query::ScnOrTimestamp aot, Ptr<SqlExpr> e)
  : GrammarBaseSmart(l), asOf(aot, e)  {}

FlashbackQueryClause::FlashbackQueryClause(CLoc l)
  : GrammarBaseSmart(l) {}


OrderBy::OrderBy(CLoc l, Ptr<OrderBy::OrderByList> ord, bool sibl)
  : GrammarBaseSmart(l), orderList(ord), siblings(sibl) {}

OrderBy::~OrderBy() {}

void OrderBy::replaceChildsIf(ExprTr tr) { replace(tr, partitionBy, orderList); }


const std::map<string, QueryHint::Cathegory> QueryHint::kwMap = {
  {"LEADING"   , QueryHint::LEADING   },
  {"USE_NL"    , QueryHint::USE_NL    },
  {"INDEX_DESC", QueryHint::INDEX_DESC},
  {"NO_MERGE"  , QueryHint::NO_MERGE  },
  {"ORDERED"   , QueryHint::ORDERED   },
  {"FIRST_ROWS", QueryHint::FIRST_ROWS},
  {"ALL_ROWS"  , QueryHint::ALL_ROWS  },
  {"CHOOSE"    , QueryHint::CHOOSE    },
  {"INLINE"    , QueryHint::INLINE    },
};


QueryHint::QueryHint(CLoc l, List<Id> *_columnLists)
  : GrammarBase(l), argList(_columnLists) {}

QueryHint::QueryHint(CLoc l, Cathegory cat)
  : GrammarBase(l), cathegory(cat) {}

void QueryHint::parseCathegory(Ptr<Id> cat) {
  string s = cat->toNormalizedString();
  std::map<string, Cathegory>::const_iterator it = kwMap.find(s);
  if (it != kwMap.end())
    cathegory = it->second;
  else
    cout << "UNSUPPORTED HINT: " << s << endl;
}

QueryHint* QueryHint::checkFrontKeyword(Ptr<Id> cat) {
  string s = cat->toNormalizedString();
  if (s == "RULE" || s == "RUULE" || s == "FIRS_ROWS" || s == "FIRST_ROW")
    return 0;

  std::map<string, Cathegory>::const_iterator it = kwMap.find(s);
  if (it != kwMap.end())
    return new QueryHint(cat->getLLoc(), it->second);

  cout << "UNSUPPORTED HINT: " << s << endl;
  return 0;
}



void QueryTailData::collectSNode(SemanticTree *n) const {
  if (hierarhicalSpec) {
    SemanticTree *hierarhNode = new SemanticTree(SCathegory::HierarhicalClause, SemanticTree::NEW_LEVEL);
    CTREE2(hierarhNode, hierarhicalSpec);
    n->addChild(hierarhNode);
  }
  CollectSNode(n, groupBy);
  CTREE(having);
}

void GroupBySqlExpr::sqlDefinition(Codestream &str) {
  if (beginedFrom(1146049,19))
    cout << "";
  str << sqlExpr;
}
