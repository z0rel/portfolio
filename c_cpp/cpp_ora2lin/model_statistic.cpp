#include <iomanip>
#include <tuple>
#include "model_statistic.h"
#include "model_context.h"
#include "semantic_datatype.h"
#include "semantic_collection.h"
#include "codegenerator.h"
#include "system_sysuser.h"
#include "semantic_function.h"
#include "semantic_expr.h"
#include "depworkflow.h"
#include "semantic_statements.h"
#include "semantic_blockplsql.h"
#include "statements_tree.h"
#include "dynamic_sql_op.h"
#include "codestream.h"
#include "resolvers.h"
#include "semantic_function.h"
#include "usercontext.h"
#include "syntaxer_context.h"
#include "dynamic_sql.h"
#include "resolvers.h"

using namespace Sm;
extern SyntaxerContext syntaxerContext;


SemanticTree::UniqueDefinitions SemanticTree::globalUniqueDefinitions;
SysDefCounters SemanticTree::globalSysDefCounters;

void ModelContext::printRecVarUsing(Sm::Codestream    &str,
                                    Type::Record      *recordDef,
                                    RecVarUsing       &record,
                                    RecordCounters    &counters) {
  str.incIndentingLevel(2);
  bool containsStructuredFields = false;

  std::vector<std::pair<Type::RecordField *, Datatype *> > structuredFields;

  if (Ptr<List<Type::RecordField> > f = recordDef->getRecordFields())
    for (Ptr<Type::RecordField> &fld : *f)
      if (Datatype *t = ResolvedEntity::getLastUnwrappedDatatype(fld->datatype))
        if (t->isCollectionType() || t->isRowTypeOf() || t->isObjectType() || t->isRecordType())
          structuredFields.push_back(make_pair(fld.object(), t));

  containsStructuredFields = structuredFields.size();
  if (containsStructuredFields) {
    ++counters.recordsThatContainsStructuredFields;
    str.incIndentingLevel(2);
    str << s::tab(str.indentingLevel()) << " == contains structured fields:" << s::endl;
    str.incIndentingLevel(2);
    for (auto &fld : structuredFields) {
      str << s::tab(str.indentingLevel()) << fld.first->getName()->toQString() << s::name;
      fld.second->linterDefinition(str);
      str << s::name << " is " << fld.second->tidDef()->ddlCathegoryToString() << s::endl;
    }
    str.decIndentingLevel(2);
    str.decIndentingLevel(2);
  }

  for (auto &var : record) {
    printRecEntityName(str, "variable", var.first->getName());
    // вывести, содержатся ли коллекции внутри и является ли это insert-предложением
    bool isFirst = false;

    SemanticTree varFlags;
    for (auto &blk : var.second)
      for (auto &stmt : blk.second)
        varFlags.flags |= stmt.second.v;

    if (varFlags.isFromNode())
      str << s::comma(&isFirst) << s::name << "used in SELECT as FROM";
    if (varFlags.isInsertingValue())
      str << s::comma(&isFirst) << s::name << "used in INSERT as VALUE";
    if (varFlags.isIntoNode())
      str << s::comma(&isFirst) << s::name << "used in SELECT as INTO";

    if (containsStructuredFields) {
      str << s::comma(&isFirst) << s::name << "contains structured fields";
      ++counters.variablesThatContainsStructuredFields;
    }
    str << s::endl;

    str.incIndentingLevel(2);
    for (auto &blk : var.second)
      for (auto &stmt : blk.second) {
        stmt.first->setNotOutStatements();
        SemanticTree f;
        f.flags = stmt.second.v;
        string s = " ";
        if (f.isIntoNode())
          s += "is into node ";
        if (f.isFromNode())
          s += "is from node ";
        if (f.isInsertingValue())
          s += "is inserting value ";
        str << s << s::name;
        StatementsTranslator::translateStatement(str, stmt.first, blk.first);
      }
    str.decIndentingLevel(2);
  }
  str.decIndentingLevel(2);
}


Sm::Codestream& ModelContext::printRecEntityName(Sm::Codestream& str, const std::string &prefix, const Ptr<Sm::Id> &name) {
  if (str.indentingLevel() < 0)
    cout << "";
  if (prefix == "WIDSET")
    cout << "";
  if (name) {
    stringstream s;
    s << name->getLLoc();

    str << s::tab(str.indentingLevel()) << prefix << ": " << name->toQString() << s::name << s.str() << ":";
  }
  else
    str << s::tab(str.indentingLevel()) << prefix << ": unnamed <unknown location>:";
  return str;
}

void ModelContext::printRecordsUsingAnalysisResult(
    Sm::Codestream          &str,
    RecordCounters          &counters,
    Rec                     &records,
    Sm::ResolvedEntity      *collectionVariable)
{
  str.incIndentingLevel(2);
  if (collectionVariable)
    printRecEntityName(str, "collection", collectionVariable->getName()) << s::endl;
  str.incIndentingLevel(2);
  for (auto &recordType : records) {
    printRecEntityName(str, "record", recordType.first->getName()) << s::endl;
    printRecVarUsing(str, recordType.first, recordType.second, counters);
  }
  str.decIndentingLevel(2);
  str.decIndentingLevel(2);
}

void ModelContext::printRecordsUsing() {
  Sm::CollectUsingByStatements::StmtsStack       statementsStack;
  Sm::CollectUsingByStatements::BlockPlSqlOwners ownersStack;
  Collections collections; // записи, используемые как поля коллекций
  CollectionsByRecords collectionsByRecords; // записи, построенные на основе типов коллекций
  Rec recMap; // остальные records

  set<ResolvedEntity*, Sm::LE_ResolvedEntities> collectionTypesContainsRecords;
  set<ResolvedEntity*, Sm::LE_ResolvedEntities> recordTypesContainsRecords;
  set<ResolvedEntity*, Sm::LE_ResolvedEntities> tablesContainsRecords;
  set<ResolvedEntity*, Sm::LE_ResolvedEntities> objectTypesContainsRecords;
  RecordCounters counters;

  auto collectRec = [](Rec &src, Rec &dst) {
    for (Rec::value_type &v : src)
      for (RecVarUsing::value_type &vu : v.second)
        for (RecStmtsUsing::value_type &su: vu.second)
          for (RecStmtsUsing::value_type::second_type::value_type &s : su.second)
            dst[v.first][vu.first][su.first][s.first].v |= s.second.v;
    src.clear();
  };

  auto collectRecCollection = [&collectRec](
      Rec &src, Collections &collections, ResolvedEntity *var, unsigned int *cnt = 0, set<ResolvedEntity*, Sm::LE_ResolvedEntities> *typesContainer = 0, ResolvedEntity *tDef = 0) -> void {
    if (src.empty())
      return;
    if (typesContainer) {
      ++(*cnt);
      typesContainer->insert(tDef);
    }
    collectRec(src, collections[var]);
  };


  auto lastOwner = [&ownersStack]() -> Sm::BlockPlSql* { return Sm::CollectUsingByStatements::lastOwner(&ownersStack); };
  auto lastStmt  = [&statementsStack]() -> Sm::StatementInterface* { return Sm::CollectUsingByStatements::lastStatement(&statementsStack); };
  auto checkToInsertContext = [&](bool &contextIsBuild, SemanticTree *n, unsigned int &val) -> unsigned int {
    if (contextIsBuild)
      return val;

    contextIsBuild = true;
    Sm::CommonDatatypeCast::CastContext cntx;
    n->getExpressionContext(cntx);
    SemanticTree v;
    if (cntx.intoVariableExpression)
      v.setIsIntoNode();
    if (cntx.insertingValue)
      v.setInsertingValue();
    if (cntx.fromNode)
      v.setIsFromNode();
    val = v.flags;
    return val;
  };

  Sm::ExprTR::Cond cond = [&](PlExpr *expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;
    if (SemanticTree *n = expr->getSemanticNode()) {
      bool contextIsBuild       = false;
      if (!n->reference())
        return FLAG_REPLACE_TRAVERSE_NEXT;
      IdEntitySmart::iterator endIt = n->refEnd();
      Rec rec;
      unsigned int cursorCat = 0;
      for (IdEntitySmart::iterator it = n->refBegin(); it != endIt; ++it) {
        if (ResolvedEntity *var = (*it)->unresolvedDefinition()) {
          if (Ptr<Id> n = var->getName()) {
            if (n->unresolvedDefinition())
              var = n->unresolvedDefinition();
            if (n->getLLoc().beginedFrom(732816))
              cout << "";
          }
          if (Ptr<Datatype> t = ResolvedEntity::getLastUnwrappedDatatype(var->getDatatype()))
            if (ResolvedEntity *tDef = t->tidDef()) {
              if (Sm::Type::Record *r = tDef->toSelfRecord()) // обработка Record
                rec[r][var][lastOwner()][lastStmt()].v |= checkToInsertContext(contextIsBuild, n, cursorCat);
              else if (tDef->isCollectionType()) { // обработка коллекции
                if (rec.size())
                  collectRecCollection(rec, collections, var, &counters.fieldRefInCollections, &collectionTypesContainsRecords, tDef);
                else if (Datatype *collectionNextT = ResolvedEntity::getLastUnwrappedDatatype(tDef->getNextDefinition()->getDatatype()))
                  if (ResolvedEntity *d = collectionNextT->tidDef())
                    if (Type::Record *recInCol = d->toSelfRecord())
                      collectionsByRecords[recInCol][tDef->toSelfCollectionType()][var][lastOwner()][lastStmt()].v |= checkToInsertContext(contextIsBuild, n, cursorCat);
              }
              // обработка структурной вложенности полей
              else if (tDef->isRecordType())
                collectRecCollection(rec, collections, var, &counters.fieldRefInRecords, &recordTypesContainsRecords, tDef);
              else if (tDef->isObject())
                collectRecCollection(rec, collections, var, &counters.fieldRefInObjects, &objectTypesContainsRecords, tDef);
              else if (tDef->isRowTypeOf())
                collectRecCollection(rec, collections, var, &counters.fieldRefInTables, &tablesContainsRecords, tDef);
            }
          if (rec.size())
            collectRec(rec, recMap);
        }
      }
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  CollectUsingByStatements tr(cond, &statementsStack);
  tr.blockPlSqlOwners = &ownersStack;
  replaceChildsIf(tr);

  auto countRecVarUsingProperties = [](RecVarUsing &varUsing, unsigned int &varCnt, SemanticTree &tF, RecordCounters::CursorCounters &refsCnt) {
    for (RecVarUsing::value_type &varDef : varUsing)
      for (RecStmtsUsing::value_type &ref : varDef.second) {
        varCnt += ref.second.size();
        for (RecStmtsUsing::value_type::second_type::value_type &s : ref.second) {
          tF.flags |= s.second.v;
          refsCnt.count(s.second.v);
        }
      }
  };

  for (CollectionsByRecords::value_type &v : collectionsByRecords) {
    SemanticTree f;
    counters.collectionByRecords += v.second.size();
    for (CollectionsByRecords::value_type::second_type::value_type &rec : v.second)
      countRecVarUsingProperties(rec.second, counters.collectionVarByRecords, f, counters.recordRefs);
    counters.recordTypes.count(f.flags);
  }
  for (Rec::value_type &rec : recMap) {
    SemanticTree f;
    countRecVarUsingProperties(rec.second, counters.simpleRecordVariables, f, counters.recordRefs);
    counters.recordTypes.count(f.flags);
  }


  Codestream str;
  str.state().indentingLongConstruct_ = false;
  str << "============" << s::endl
      << "Record analysis" << s::endl;

  if (syntaxerContext.fullRecordsReport) {
    if (collections.size()) {
      str << "== records, that contains in collections: ==" << s::endl;
      str.indentingLevel(2);
      if (collections.size())
        for (auto &v : collections)
          printRecordsUsingAnalysisResult(str, counters, v.second, v.first);
      else
        str << s::tab(str.indentingLevel()) << "None" << s::endl;
    }

    str << s::endl << "== collections that based on records: ==" << s::endl;
    str.incIndentingLevel(2);
    for (auto &v : collectionsByRecords) {
      printRecEntityName(str, "base record", v.first->getName()) << s::endl;
      str.incIndentingLevel(2);
      for (auto &c : v.second) {
        printRecEntityName(str, "derived collection", c.first->getName()) << s::endl;
        printRecVarUsing(str, v.first, c.second, counters);
      }
      str.decIndentingLevel(2);
    }
    str.decIndentingLevel(2);

    str << s::endl << "== records, that is not contains in collections: ==" << s::endl;
    printRecordsUsingAnalysisResult(str, counters, recMap, 0);
  }
  str << s::endl
      << "simple record variables references                          : " << int(counters.simpleRecordVariables) << s::endl
      << "simple record types                                         : " << int(recMap.size()) << s::endl
      << "record field    refernces, that contains in object types    : " << counters.fieldRefInObjects << s::endl
      << "record types             , that contains in object types    : " << int(objectTypesContainsRecords.size()) << s::endl
      << "record field    refernces, that contains in record types    : " << counters.fieldRefInRecords << s::endl
      << "record types             , that contains in record types    : " << int(recordTypesContainsRecords.size()) << s::endl
      << "record field    refernces, that contains in tables          : " << counters.fieldRefInTables << s::endl
      << "record types             , that contains in tables          : " << int(tablesContainsRecords.size()) << s::endl
      << "record field    refernces, that contains in collection types: " << counters.fieldRefInCollections << s::endl
      << "record types             , that contains in collection types: " << int(collectionTypesContainsRecords.size()) << s::endl
      << "record variable refernces, that contains structure fields   : " << counters.variablesThatContainsStructuredFields << s::endl
      << "record types             , that contains structure fields   : " << counters.recordsThatContainsStructuredFields   << s::endl
      << "record types on that based some collection types            : " << int(collectionsByRecords.size()) << s::endl
      << "collection types that based on record types                 : " << counters.collectionByRecords << s::endl
      << "collection variables that based on record types             : " << counters.collectionVarByRecords << s::endl;

  counters.recordTypes    .print(str, "record types            ");
  counters.recordRefs     .print(str, "record variable refs    ");
  counters.collectionTypes.print(str, "collection types        ");
  counters.collectionRefs .print(str, "collection variable refs");

  cout << str.str() << endl;
}

void ModelContext::RecordCounters::CursorCounters::print(Sm::Codestream &str, string prefix) {
  str << prefix << " that used in SELECT as FROM  :" << from           << s::endl
      << prefix << " that used in SELECT as INTO  :" << into           << s::endl
      << prefix << " that used in INSERT as VALUE :" << insertingValue << s::endl;
}

void ModelContext::RecordCounters::CursorCounters::count(unsigned int fl) {
  SemanticTree f;
  f.flags = fl;
  if (f.isFromNode())
    ++from;
  if (f.isInsertingValue())
    ++insertingValue;
  if (f.isIntoNode())
    ++into;
}


SysDefSemanticTree::Map::iterator addNode(SysDefCounters::EntityMap::value_type &it, SysDefSemanticTree::Map &sysDefMap)
{
  SysDefSemanticTree::Map::iterator treeIt = sysDefMap.find(it.first);
  if (treeIt == sysDefMap.end())
    treeIt = sysDefMap.insert(SysDefSemanticTree::Map::value_type(it.first, new SysDefSemanticTree(it))).first;
  return treeIt;
}

bool SysDefSemanticTree::compareChilds(Childs::value_type i, Childs::value_type j) {
  static const auto compareString = [](const string &s1, const string &s2) -> bool {
    return lexicographical_compare(s1.begin(), s1.end(), s2.begin(), s2.end());
  };

  if (i->node.first->ddlCathegory() != j->node.first->ddlCathegory())
    return compareString(Sm::toString(i->node.first->ddlCathegory()), Sm::toString(j->node.first->ddlCathegory()));
  else
    return compareString(i->node.first->getName()->toNormalizedString(), j->node.first->getName()->toNormalizedString());
}

void SysDefSemanticTree::sortChilds() {
  std::sort(childs.begin(), childs.end(), compareChilds);
  for (Childs::iterator it = childs.begin(); it != childs.end(); ++it)
    (*it)->sortChilds();
}




bool SysDefSemanticTree::formatString(Codestream &str, int tab, int /*level*/) {
  ResolvedEntity::ScopedEntities defCat = node.first->ddlCathegory();
  if (defCat == ResolvedEntity::Function_ && node.first->isSystemTemplate() &&
      node.first->getName()->toNormalizedString() == "DECODE")
    return false;
  if (defCat == ResolvedEntity::FieldOfRecord_ || defCat == ResolvedEntity::Variable_ || defCat == ResolvedEntity::NestedTable_ ||
      defCat == ResolvedEntity::FieldOfTable_  || defCat == ResolvedEntity::Function_ || defCat == ResolvedEntity::MemberFunction_) {
    str << s::tab(tab);
//    if (defCat != ResolvedEntity::Function_ && defCat != ResolvedEntity::MemberFunction_)
//      str << s::ogroup(level);
    str << s::decl << node.first << s::def << s::Comment() << ((defCat == ResolvedEntity::Variable_) ? " Variable " : "") << '<' << node.second << '>';
//    if (defCat != ResolvedEntity::Function_ && defCat != ResolvedEntity::MemberFunction_)
//      str << s::cgroup(level);
    str << s::endl;
    return true;
  }
  else {
    if (childs.size())
      str << s::endl;
    str << s::tab(tab) << *(node.first->getName()) << '\t' << toString(node.first->ddlCathegory()) << " <" << node.second << '>' << s::endl;
  }

  unsigned int newlevel = str.nextUniqueLevel();
  for (Childs::iterator it = childs.begin(); it != childs.end(); ++it)
    (*it)->formatString(str, tab + 4, newlevel);
  return true;
}


void handleReportSysDefNode(SysDefCounters::EntityMap             &sysDefCounters,
                            SysDefSemanticTree::Map               &sysDefMap,
                            SysDefCounters::EntityMap::value_type &it)
{
  SysDefSemanticTree::Map::iterator treeIt = addNode(it, sysDefMap);
  if (SemanticTree *snode = it.first->getSemanticNode()) {
    ResolvedEntity *parentDef = 0;
    SysDefCounters::EntityMap::iterator parentIt = sysDefCounters.end();
    while ((snode = snode->getParent()))
      if ( (parentDef = snode->ddlEntity()) ) {
        parentIt = sysDefCounters.find(parentDef);
        if (parentIt != sysDefCounters.end())
          break;
        else
          parentDef = 0;
      }
    if (parentDef) {
      SysDefSemanticTree::Map::iterator parentTreeIt = addNode(*parentIt, sysDefMap);
      parentTreeIt->second->childs.push_back(treeIt->second);
      treeIt->second->parent = parentTreeIt->second.object();
    }
  }
}


void ModelContext::generateReportSystemEntitiesUsing(Sm::Codestream &str) {
  SysDefCounters sysDefCounters;
  if (!rootGlobalTree)
    return;
  rootGlobalTree->collectSystemDefinitions(sysDefCounters);
  rootGlobalTree->countSystemDefinitions  (sysDefCounters);

  SysDefSemanticTree::Map sysDefMap;

  for (SysDefCounters::EntityMap::iterator it = sysDefCounters.entitiesCounters.begin(); it != sysDefCounters.entitiesCounters.end(); ++it) {
    if (!it->first->getName())
      continue;

    if (it->second)
      handleReportSysDefNode(sysDefCounters.entitiesCounters, sysDefMap, *it);
  }

  for (SysDefSemanticTree::Map::iterator it = sysDefMap.begin(); it != sysDefMap.end(); ++it) {
    if (!it->second->parent && it->second->childs.size() && it->first->ddlCathegory() != ResolvedEntity::User_)
      handleReportSysDefNode(sysDefCounters.entitiesCounters, sysDefMap, it->second->node);
  }

  Ptr<SysDefSemanticTree> root = new SysDefSemanticTree();

  for (SysDefSemanticTree::Map::iterator it = sysDefMap.begin(); it != sysDefMap.end(); ++it)
    if (!it->second->parent)
      root->childs.push_back(it->second);

  root->sortChilds();
  for (SysDefSemanticTree::Childs::iterator it = root->childs.begin(); it != root->childs.end(); ++it)
    (*it)->formatString(str, 0);

  size_t maxlength = 15;
  s::tab tab(4);
  str << "Nested Table Methods Usage Count:" << s::endl;
  for (SysDefCounters::Map::iterator it = sysDefCounters.nestedTableCounters.begin(); it != sysDefCounters.nestedTableCounters.end(); ++it)
    str << tab << it->first << (maxlength > it->first.size() ? std::string(maxlength - it->first.size(), ' ') : std::string(1, ' ')) << '<' << it->second << '>' << s::endl;

  str << s::endl << "Varray Methods Usage Count:" << s::endl;
  for (SysDefCounters::Map::iterator it = sysDefCounters.varrayCounters.begin(); it != sysDefCounters.varrayCounters.end(); ++it)
    str << tab << it->first << (maxlength > it->first.size() ? std::string(maxlength - it->first.size(), ' ') : std::string(1, ' ')) << '<' << it->second << '>' << s::endl;

  str << s::endl << "Tables fields with Varray datatype:       " << sysDefCounters.varrayFieldsCounter
      << s::endl << "Tables fields with Nested Table datatype: " << sysDefCounters.nestedTableFieldsCounter
      << s::endl << "Tables fields with Object datatype:       " << sysDefCounters.objectTableFieldsCounter
      << s::endl << "  Where objects is: " << s::endl;
  for (SysDefCounters::Map::iterator it = sysDefCounters.objectTableFields.begin(); it != sysDefCounters.objectTableFields.end(); ++it)
    str << tab << it->first << (maxlength > it->first.size() ? std::string(maxlength - it->first.size(), ' ') : std::string(1, ' ')) << '<' << it->second << '>' << s::endl;
  str << "USERENV function arguments: " << s::endl;
  for (SysDefCounters::Map::iterator it = sysDefCounters.userenvArgumentsCounters.begin(); it != sysDefCounters.userenvArgumentsCounters.end(); ++it)
    str << tab << it->first << (maxlength > it->first.size() ? std::string(maxlength - it->first.size(), ' ') : std::string(1, ' ')) << '<' << it->second << '>' << s::endl;
  str << "ANALYTIC Functions counters: " << s::endl;
  for (SysDefCounters::AnalyticMap::iterator it = sysDefCounters.analyticCounters.begin(); it != sysDefCounters.analyticCounters.end(); ++it)
    str << tab << it->first << (maxlength > it->first.size() ? std::string(maxlength - it->first.size(), ' ') : std::string(1, ' ')) << '<' << it->second.size() << '>' << s::endl;
  str << s::endl << "ANALYTIC Functions: " << s::endl;
  for (SysDefCounters::AnalyticMap::iterator it = sysDefCounters.analyticCounters.begin(); it != sysDefCounters.analyticCounters.end(); ++it)
    for (SysDefCounters::AnalyticMap::mapped_type::iterator vit = it->second.begin(); vit != it->second.end(); ++vit) {
      (*vit)->oracleDefinition(str);
      str << s::endl;
    }
  str << s::endl << " Trigger PARENT pseudorecords: " << sysDefCounters.parentTriggerPseudorecordsCounters << s::endl;
}


void ModelContext::printTablesThatWillChanged(Sm::Codestream &str) {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    if (!it->second->isSystem())
      for (UserContext::Tables::iterator tblIt = it->second->tables.begin(); tblIt != it->second->tables.end(); ++tblIt)
        if (tblIt->second->hasNumberFieldsThatWillChanged())
          str << s::Comment() << s::ref << tblIt->second << s::def << s::endl;
}


void ModelContext::printTablesWithCleanNumberFields(Sm::Codestream &str) {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    if (!it->second->isSystem())
      it->second->printTablesWithCleanNumberFields(str);
}


void ModelContext::printTablesFieldsWithCleanNumberFields(Sm::Codestream &str) {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    if (!it->second->isSystem())
      it->second->printTablesFieldsWithCleanNumberFields(str);
}


void ModelContext::printTablesFieldsWithBigNumberFields(Sm::Codestream &str) {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    if (!it->second->isSystem())
      it->second->printTablesFieldsWithBigNumberFields(str);
}


void UserContext::printTablesWithCleanNumberFields(Sm::Codestream &str) {
  for (Tables::iterator it = tables.begin(); it != tables.end(); ++it)
    if (it->second->hasCleanNumberFields()) {
      it->second->linterReference(str);
      str << s::endl;
    }
}


void UserContext::printTablesFieldsWithCleanNumberFields(Sm::Codestream &str) {
  for (Tables::iterator it = tables.begin(); it != tables.end(); ++it)
    it->second->printCleanNumberFields(str);
}


void UserContext::printTablesFieldsWithBigNumberFields(Sm::Codestream &str) {
  for (Tables::iterator it = tables.begin(); it != tables.end(); ++it)
    it->second->printBigNumberFields(str);
}


void ModelContext::printNumberPrecStatistic() {
  std::map<string, int> stat;
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    it->second->calculateNumberPrecStatistic(stat);

  for (std::map<string, int>::iterator statIt = stat.begin(); statIt != stat.end(); ++statIt)
    cout << statIt->first << " " << statIt->second << endl;
}


void UserContext::calculateNumberPrecStatistic(std::map<string, int> &stat) {
  for (Tables::iterator it = tables.begin(); it != tables.end(); ++it )
    if (!it->second->isSystem())
      it->second->calculateNumberPrecStatistic(stat);
}


void ModelContext::findTablesWithObjectFields() {
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    for (UserContext::Tables::iterator tblIt = it->second->tables.begin(); tblIt != it->second->tables.end(); ++tblIt)
      if (tblIt->second->hasObjectFields())
        cout << it->second->getName()->toQString() << "." << tblIt->second->getName()->toQString() << endl;
}


void ModelContext::outWrappedEntities() {
  Codestream str;
  for (UserDataMap::iterator it = userMap.begin(); it != userMap.end(); ++it)
    for (UserContext::WrpEntities::iterator wIt = it->second->wrpEntities.begin(); wIt != it->second->wrpEntities.end(); ++wIt)
      for (UserContext::WrpEntities::mapped_type::iterator entIt = wIt->second.begin(); entIt != wIt->second.end(); ++entIt)
        str << toString(wIt->first) << s::name << it->second->getName() << "." << entIt->second << s::endl;

  OutputCodeFile::storeCode(syntaxerContext.outFileName, str.str());
}


void SemanticTree::countSystemDefinitions(SysDefCounters &counters) {
  for (Childs::iterator it = childs.begin(); it != childs.end(); ++it) {
    switch ((*it)->nametype) {
      case SemanticTree::DECLARATION:
      case SemanticTree::DEFINITION:
        if ((*it)->cathegory == SCathegory::User && (*it)->unnamedDdlEntity != syntaxerContext.model->sysuser().object())
          (*it)->countSystemDefinitionsOnUserContext(counters);
        else
          (*it)->countSystemDefinitions(counters);
        return;
      default:
        (*it)->countSystemDefinitions(counters);
        break;
    }
  }
}


void SemanticTree::updateCounters(SysDefCounters &counters, ResolvedEntity *def) {
  static const auto getLastConcreteCollectionDefinition = [](ResolvedEntity* t) -> ResolvedEntity* {
    while (t && (t->isField() || t->ddlCathegory() == ResolvedEntity::Datatype_ || t->ddlCathegory() == ResolvedEntity::Subtype_))
      t = t->getNextDefinition();
    return t;
  };


  static const auto updateCountersForCollection = [](SysDefCounters::Map &countersMap, const std::string &name) -> void {
    SysDefCounters::Map::iterator it;

    if ((it = countersMap.find(name)) != countersMap.end())
      ++(it->second);
    else
      countersMap[name] = 1;
  };

  if (def) {
    SysDefCounters::EntityMap::iterator sysdefIt = counters.entitiesCounters.find(def);
    if (sysdefIt != counters.entitiesCounters.end())
      ++(sysdefIt->second);
    ResolvedEntity::ScopedEntities defCat = def->ddlCathegory();
    if (Sm::Type::collection_methods::CollectionMethod *m = def->toSelfCollectionMethod()) {
      if (ResolvedEntity *t = getLastConcreteCollectionDefinition(m->collectionDatatype())) {
        if (t->ddlCathegory() == ResolvedEntity::NestedTable_)
          updateCountersForCollection(counters.nestedTableCounters, m->methodName());
        else if (t->ddlCathegory() == ResolvedEntity::Varray_)
          updateCountersForCollection(counters.varrayCounters, m->methodName());
        else
          throw 999; // учесть все варианты
      }
    }
    else if (defCat == ResolvedEntity::FieldOfTable_) {
      if (ResolvedEntity *t = getLastConcreteCollectionDefinition(def)) {
        if (t->ddlCathegory() == ResolvedEntity::Object_) {
          ++counters.objectTableFieldsCounter;
          if (Ptr<Id> n = t->getName())
            updateCountersForCollection(counters.objectTableFields, n->toNormalizedString());
          else
            throw 999; // учесть все варианты
        }
        else if (t->ddlCathegory() == ResolvedEntity::NestedTable_)
          ++counters.nestedTableFieldsCounter;
        else if (t->ddlCathegory() == ResolvedEntity::Varray_)
          ++counters.varrayFieldsCounter;
        else if (t->ddlCathegory() != ResolvedEntity::FundamentalDatatype_)
          throw 999; // учесть все варианты
      }
    }
    else if (unnamedDdlEntity && unnamedDdlEntity->ddlCathegory() == ResolvedEntity::FunctionCallArgument_) {
      if (ResolvedEntity *fun = owner())
        if (fun->ddlCathegory() == ResolvedEntity::Function_ && fun->getName() && fun->getName()->toNormalizedString() == "USERENV")
          if (Ptr<Id> argname = def->getName())
            updateCountersForCollection(counters.userenvArgumentsCounters, argname->toNormalizedString());
    }
    else if (cathegory == SCathegory::AnalyticFun && unnamedDdlEntity) {
      if (Ptr<Id> n = unnamedDdlEntity->getName())
        counters.analyticCounters[n->toNormalizedString()].push_back(unnamedDdlEntity);
    }
    else if (def->isTriggerRowReferenceParent() && def != unnamedDdlEntity) {
      ++(counters.parentTriggerPseudorecordsCounters);
    }
  }
}


void SemanticTree::countSystemDefinitionsOnUserContext(SysDefCounters &counters) {
  if (!refEmpty()) {
    for (IdEntitySmart::iterator it = referenceName_->begin(); it != referenceName_->end(); ++it)
      updateCounters(counters, (*it)->unresolvedDefinition());
  }
  else if (ResolvedEntity *def = ddlEntity())
    updateCounters(counters, def);
  for (Childs::value_type c : childs)
    c->countSystemDefinitionsOnUserContext(counters);
}


int SemanticTree::countArglistRefcursorFiels() {
  int cnt = 0;
  if (nametype == DEFINITION || nametype == DECLARATION)
    if (ResolvedEntity *def = ddlEntity())
      if (Sm::Function *f = def->toSelfFunction())
        cnt += f->countArglistRefcursorFiels();
  for (Childs::iterator it = childs.begin(); it != childs.end(); ++it)
    cnt += (*it)->countArglistRefcursorFiels();
  return cnt;
}


void SemanticTree::calculateCountOfEntities() {
  auto traverseChildsForFunctionNode = [this]() -> void {
      globalSysDefCounters.inFunctionsStack.push_back(true);
      for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::calculateCountOfEntities));
      globalSysDefCounters.inFunctionsStack.pop_back();
  };
  auto traverseChildsForTriggerNode = [this, traverseChildsForFunctionNode](Sm::Trigger *def) -> void {
    globalSysDefCounters.inTriggerStack.push_back(def);
    globalSysDefCounters.triggerUsesParent = false;
    std::pair<SysDefCounters::TriggerCathegories::iterator, bool> it
        = globalSysDefCounters.triggerCathegories.insert(make_pair(def->toLogStringCahtegory(), 1));
    if (!it.second)
      ++(it.first->second);

    traverseChildsForFunctionNode();
    globalSysDefCounters.inTriggerStack.pop_back();
    globalSysDefCounters.triggerUsesParent = false;
  };
  auto traverseChildsForPackage = [this]() -> void {
      globalSysDefCounters.inPackageFlag = true;
      for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::calculateCountOfEntities));
      globalSysDefCounters.inPackageFlag = false;
  };
  auto checkOnParentField = [this](ResolvedEntity *d) -> void {
    if (nametype == REFERENCE && d && d->isTriggerRowReferenceParent())
      if (!globalSysDefCounters.triggerUsesParent) {
        globalSysDefCounters.triggerUsesParent = true;
        ++globalSysDefCounters.triggersThatUsesParentField;
      }
  };

  if (ResolvedEntity *def = unnamedDdlEntity)  {
    if (def->isSystem())
      return;
    else if (def->isSystemTemplate() || def->isSystemFlag())
      return;
    else if (def->toSelfPackage())
      return traverseChildsForPackage();
    else if (globalUniqueDefinitions.insert(def->getDefinitionFirst()).second)
      switch (def->ddlCathegory()) {
        case ResolvedEntity::MemberFunction_:
          ++globalSysDefCounters.functionsCnt;
          ++globalSysDefCounters.memberFunctionsCnt;
          return traverseChildsForFunctionNode();
        case ResolvedEntity::Function_:
          ++globalSysDefCounters.functionsCnt;
          if (globalSysDefCounters.inFunctionsStack.empty()) {
            if (globalSysDefCounters.inPackageFlag)
              ++globalSysDefCounters.packagesFunctionCnt;
            else
              ++globalSysDefCounters.freeFunctionsCnt;
          }
          return traverseChildsForFunctionNode();
        case ResolvedEntity::Trigger_:
          ++globalSysDefCounters.triggersCnt;
          return traverseChildsForTriggerNode(def->toSelfTrigger());
        case ResolvedEntity::View_:
          ++globalSysDefCounters.viewsCnt;
          break;
        case ResolvedEntity::TriggerNestedRowReference_:
        case ResolvedEntity::TriggerRowReference_:
          checkOnParentField(def);
          break;
        case ResolvedEntity::RefExpr_:
          if (Sm::RefExpr*s = def->toSelfRefExpr())
            for (Ptr<Id> &it : *(s->reference))
              checkOnParentField(it->definition());
          break;
        default:
          break;
      }
  }
  for_each(childs.begin(), childs.end(), mem_fun(&SemanticTree::calculateCountOfEntities));
}


void SemanticTree::checkForCurrentType(Ptr<Id> &it, Datatype *checkedType, bool (Datatype::*isTypeMf)() const, SysDefCounters::TypeLocations &container) {
  if (it)
    if (ResolvedEntity *def = it->definition()) {
      bool isDef = false;
      bool isRef = false;
      bool isDatatype = false;
      if (nametype == DEFINITION || nametype == DECLARATION) {
        if (!unnamedDdlEntity)
          return;
        isDef = true;
      }
      else if (nametype == DATATYPE_REFERENCE)
        isDatatype = true;
      else if (nametype == REFERENCE)
        isRef = true;

      if (Ptr<Datatype> dt = def->getDatatype())
        if (Ptr<Datatype> t = ResolvedEntity::getLastConcreteDatatype(dt))
          if ((t.object()->*isTypeMf)() || t->isSubtype(checkedType, isPlContext())) {
            Sm::Codestream str;
            stringstream s;
            s << it->getLLoc();
            str << it << "[";
            if (isDef)
              str << "def  ";
            else if (isRef)
              str << "ref  ";
            else if (isDatatype)
              str << "type ";
            else
              str << "     ";

            str <<  def->ddlCathegoryToString() << "]:" << s.str();
            container.insert(str.str());
          }
    }
}


void SemanticTree::collectTypeUsing(Datatype *t, bool (Datatype::*isTypeMf)() const, SysDefCounters::TypeLocations &container) {
  if (referenceName_)
    for (Ptr<Id> &it : *referenceName_)
      checkForCurrentType(it, t, isTypeMf, container);

  for (SemanticTree *n : childs)
    n->collectTypeUsing(t, isTypeMf, container);
}


void SemanticTree::collectXmltypeUsing() { collectTypeUsing(Datatype::mkXmltype(), &Datatype::isXmlType, globalSysDefCounters.xmltypeLocations); }


void SemanticTree::collectAnydataUsing() { collectTypeUsing(Datatype::mkAnydata(), &Datatype::isAnydata, globalSysDefCounters.anydataLocations); }


void SysDefCounters::printFunCounters() {
  cout << "Views         = " << viewsCnt            << endl
       << "Free funs     = " << freeFunctionsCnt    << endl
       << "Member funs   = " << memberFunctionsCnt  << endl
       << "Packages funs = " << packagesFunctionCnt << endl
       << "All funs      = " << functionsCnt        << endl
       << "Triggers      = " << triggersCnt         << endl
       << "Triggers, that using parent pseudofield = " << triggersThatUsesParentField << endl;


  cout << "Triggers cathegories: " << endl;
  for (const TriggerCathegories::value_type &v : triggerCathegories)
    cout << v.first << " = " << v.second << endl;

//  cout << "XMLTYPE locations: " << int(xmltypeLocations.size()) << " items" << endl;
//  for (const string &str : xmltypeLocations)
//    cout << "  " << str << endl;

  cout << "ANYDATA locations: " << int(anydataLocations.size()) << " items" << endl;
  for (const string &str : anydataLocations)
    cout << "  " << str << endl;
}


void SemanticTree::collectSystemDefinitionsOnSysuser(SysDefCounters &counters) {
  if (ResolvedEntity *def = ddlEntity())
    counters.entitiesCounters[def] = 0;
  if (unnamedDdlEntity)
    counters.entitiesCounters[unnamedDdlEntity] = 0;

  for (Childs::value_type c : childs)
    c->collectSystemDefinitionsOnSysuser(counters);
}

void SemanticTree::collectSystemDefinitions(SysDefCounters &counters) {
  bool notSkip = true;

  if (ResolvedEntity *def = unnamedDdlEntity) {
    if (syntaxerContext.model->sysusers.find(def) !=syntaxerContext.model->sysusers.end())
      return collectSystemDefinitionsOnSysuser(counters);
    switch (def->ddlCathegory()) {
      case ResolvedEntity::ModelContext_:
        notSkip = false;
        break;
      case ResolvedEntity::User_:
        switch (nametype) {
          case DECLARATION:
          case DEFINITION:
            return;
          default:
            break;
        }
        break;
      case ResolvedEntity::Synonym_:
        if (ResolvedEntity *trgt = def->getTarget()->definition())
          if (ResolvedEntity *trgtParent = trgt->getSemanticNode()->childNamespace->parent->semanticLevel->unnamedDdlEntity)
            if (syntaxerContext.model->sysusers.find(trgtParent) == syntaxerContext.model->sysusers.end())
              notSkip = false;
        break;
      default:
        break;
    }
    if (notSkip)
      counters.entitiesCounters[def] = 0;
  }
  if (notSkip)
    if (ResolvedEntity *def = ddlEntity())
      if (def->ddlCathegory() != ResolvedEntity::ModelContext_)
        counters.entitiesCounters[def] = 0;

  for (Childs::value_type c : childs)
    c->collectSystemDefinitions(counters);
}



void ModelContext::printTablesAndViewsCounters() {
  int cntOfTableFields = 0;
  int cntOfViewFields = 0;
  int cntOfTables = 0;
  int cntOfViews = 0;
  Sm::EntityFields flds;
  for (ModelContext::UserDataMap::value_type &usr: syntaxerContext.model->userMap) {
    if (usr.second->isSystem())
      continue;
    cntOfTables  += usr.second->tables.size();
    for (UserContext::Tables::value_type &tbl : usr.second->tables)  {
      flds.clear();
      tbl.second->getFields(flds);
      cntOfTableFields += flds.size();
    }
    cntOfViews +=  usr.second->views.size();
    for (UserContext::Views::value_type &tbl : usr.second->views)  {
      flds.clear();
      tbl.second->getFields(flds);
      cntOfViewFields += flds.size();
    }
  }
  cout << "Tables       : " << cntOfTables      << endl
       << "Tables fields: " << cntOfTableFields << endl
       << "Views        : " << cntOfViews       << endl
       << "Views fields : " << cntOfViewFields  << endl;
}

void ModelStatistic::resolveDescrErrors() {
  using namespace PlsqlHelper;
  stringstream str;
  str << "result = [" << endl;
  for (IdEntitySmart &it : syntaxerContext.model->modelActions.refsToCodegen) {
    std::vector<string> n;
    for (IdEntitySmart::reverse_iterator rIt = it.rbegin(); rIt != it.rend(); ++rIt) {
      Ptr<Id> id = *rIt;
      n.push_back(id->toString());
    }
    str << "  (" << quotingAndEscapingPython(pyJoin(".", n)) << ",";
    it.resolveByModelContext();
    if (!it.definition()) {
      LinterSemanticResolver lsr;
      lsr.resolve(it);
    }
    if (ResolvedEntity *d = it.definition())
      str << ds::fullnameByStack(d);
    else
      str << " ('UNRESOLVED', " << quotingAndEscapingPython(it.toNormalizedString()) << ")";
    str << ")," << endl;
  }
  str << "]" << endl;

  OutputCodeFileChunck::storeCode(OutputCodeFile::temporaryPath("resolved_errnames.py"), str.str());
}

void ModelContext::calculateStatistics(DependenciesStruct::SortedEntities *sortedEntities) {
  ModelStatistic stat(this, sortedEntities);
  stat.calculate();
}

void ModelStatistic::calculate() {
  if (syntaxerContext.dependencyAnalyzerOutFileName.size())
    getConfiguredDependEntities();
  if (!syntaxerContext.generatePythonRepr.empty() && !syntaxerContext.codegenEntitiesBytelocation)
    generatePythonRepr();
  if (syntaxerContext.codegenEntitiesBytelocation)
    codegenBytelocations();
  if (syntaxerContext.exportEntitiesLocation)
    cntx_->extractLocFiles();
  if (syntaxerContext.exportSkippedSyntax)
    generateSkippedReduction();

//  extractExecuteImmediateAndOpenCursor();

  if (syntaxerContext.generateFullStatistic) {
    Codestream s;
    cntx_->generateReportSystemEntitiesUsing(s);
    cout << endl
         << " == USING OF SYSTEM ENTITIES == " << endl
         << endl << s.str() << endl << endl;

    cout << " == FUN COUNTERS == " << endl;

    cntx_->rootGlobalTree->calculateCountOfEntities();
    Sm::SemanticTree::globalSysDefCounters.printFunCounters();
    cout << endl << endl;

    cout << " == TABLE AND VIEWS COUNTERS == " << endl;
    cntx_->printTablesAndViewsCounters();
    cout << endl << endl;

    cntx_->printBlobInserts();

    cout << endl << endl;

    cntx_->printRecordsUsing();
    cntx_->rootGlobalTree->printComplexReferences(string());
  }
  return;
}





void ModelStatistic::generateSkippedReduction() {

  using namespace PlsqlHelper;
  typedef cl::position::BytePosition BytePosition;
  typedef std::set<BytePosition> BisonLines;
  // typedef tuple<BytePosition, BytePosition> BegEnd;

  struct BegEnd {
    cl::filelocation floc;
    BytePosition left()  const { return this->floc.loc.begin.bytePosition; }
    BytePosition right() const { return this->floc.loc.end.bytePosition; }

    void right(const cl::filelocation &src) { floc.loc.end = src.loc.end; }

    bool operator< (const BegEnd &oth) const { return make_tuple(left(), right()) < make_tuple(oth.left(), oth.right()); }
    BegEnd(const cl::filelocation &_floc) : floc(_floc) {}
    BegEnd(const BegEnd &oth) : floc(oth.floc) {}
    BegEnd() {}
  };
  typedef map<BegEnd, BisonLines > FileSkippedPositionsMap;

  typedef vector<pair<BegEnd, BisonLines > > FileSkippedPositionsVector;

  typedef map<const string*, FileSkippedPositionsMap > LocationsByFile;

  if (syntaxerContext.skippedLocations.empty())
    return;

  LocationsByFile skippedByFiles;
  for (syntaxer_context::SkippedLocation &it: syntaxerContext.skippedLocations)
    skippedByFiles[it.floc.file][BegEnd(it.floc)].insert(it.line);

  stringstream pyDict;
  pyDict << "skippedLocations = {" << endl;
  for (LocationsByFile::value_type &v : skippedByFiles) {

    FileSkippedPositionsVector localVector;
    for (FileSkippedPositionsMap::value_type &it : v.second)
      localVector.push_back(make_pair(it.first, it.second));

    if (!localVector.empty()) {
      // склеивание идущих друг за другом позиций
      for (FileSkippedPositionsVector::iterator it = next(localVector.begin()); it != localVector.end(); ) {
        FileSkippedPositionsVector::value_type &prevIt = *prev(it);
        BegEnd &prevKey = prevIt.first;
        BegEnd &currKey = it->first;
        if (prevKey.right() == currKey.left() || prevKey.right() + 1 == currKey.left()) { // склеивание идущих друг за другом позиций
          prevKey.right(currKey.floc);
          prevIt.second.insert(it->second.begin(), it->second.end());
          it = localVector.erase(it);
        }
        else
          ++it;
      }
      // свертка оснований
      FileSkippedPositionsVector::iterator baseIt = localVector.begin();
      for (FileSkippedPositionsVector::iterator it = next(localVector.begin()); it != localVector.end(); ) {
        BegEnd &current = it->first;
        BegEnd *basePtr = &(baseIt->first);
        if (basePtr->left() < current.left() && current.right() <= basePtr->right()) { // текущий диапазон входит в предыдущее основание
          baseIt->second.insert(it->second.begin(), it->second.end());
          it = localVector.erase(it);
        }
        else {
          baseIt = it;
          ++it;
        }
      }
    }

    string fullname = cl::filelocation::fullFilename(v.first);
    if (fullname == syntaxerContext.model->modelActions.oracleSystemSource)
      continue;
    pyDict << "  " << quotingAndEscapingPython(cl::filelocation::fullFilename(v.first)) << ":[ " << endl;
    for (FileSkippedPositionsVector::value_type &it : localVector) {
      cl::filelocation &itFloc = it.first.floc;
      pyDict << "    (" << setw(8) << itFloc.loc.begin.bytePosition << ','
                        << setw(8) << itFloc.loc.end.bytePosition   <<  ", [" << pyJoin(",", it.second) << "],"
                        << quotingAndEscapingPython(itFloc.toString())
                        << ")," << endl;
    }
    pyDict << "  ]," << endl;
  }
  pyDict << "  " << "'..model_repo..':" << quotingAndEscapingPython(syntaxerContext.converterRepositoryPath);
  pyDict << "}" << endl;


  OutputCodeFileChunck::storeCode(syntaxerContext.exportSkippedSyntaxFile, pyDict.str());
}



struct CodegenLocationContext {
  std::vector<ResolvedEntity*> ownerEntities;
  Codestream str;

  typedef  std::map<string, int> Fnames;
  Fnames fnames;

  void pushOwnerStack(ResolvedEntity* def, bool &f) {
    f = true;
    ownerEntities.push_back(def);
  }

  void traverseModel(SemanticTree *n);

  string fullnameByStack();
};

string CodegenLocationContext::fullnameByStack() {
  stringstream str;
  for (ResolvedEntity *n : ownerEntities)
    str << n->getName()->toString() << ".";
  string res = str.str();
  if (!res.empty())
    res.pop_back();
  return res;
}

void CodegenLocationContext::traverseModel(SemanticTree *n) {
  bool needPopStack = false;
  if (n->nametype == SemanticTree::DEFINITION) {
    ResolvedEntity *def = n->unnamedDdlEntity;
    string cathegory;
    if (def->toSelfView()) {
      cathegory = "VIEW";
      pushOwnerStack(def, needPopStack);
    }
    else if (def->toSelfTrigger()) {
      cathegory = "TRIGGER";
      pushOwnerStack(def, needPopStack);
    }
    else if (def->toSelfFunction()) {
      cathegory = "PROCEDURE";
      pushOwnerStack(def, needPopStack);
    }
    else if (def->toSelfPackage() || def->toSelfUserContext())
      pushOwnerStack(def, needPopStack);


    if (!cathegory.empty()) {
      cl::filelocation l =  def->getLLoc();
      string fullname = syntaxerContext.parsedFilenames.findFullname(l.file);
      pair<Fnames::iterator, bool> it = fnames.insert(Fnames::value_type(fullname, fnames.size()));
      int key = it.first->second;
      str << "(\"" << cathegory << "\", " << key << ", " << l.loc.begin.bytePosition << ","  << l.loc.end.bytePosition << ", ";
      str << "{" << "'name':'" << def->getName()->toString() << "'," << "'fullname':" << "'" << fullnameByStack() << "'})" << s::comma();
    }
  }
  for (SemanticTree::Childs::value_type &c : n->childs)
    traverseModel(c);

  if (needPopStack)
    ownerEntities.pop_back();
}


void ModelStatistic::codegenBytelocations() {
  CodegenLocationContext ctx;

  ctx.str << s::endl << "entitiesByLoc = [" << s::ocommalist();
  ctx.traverseModel(syntaxerContext.model->rootGlobalTree);

  ctx.str << s::ccommalist() << "]" << s::endl << s::endl;

  ctx.str << "filesMap = {" << s::ocommalist();
  for (CodegenLocationContext::Fnames::value_type &v : ctx.fnames)
    ctx.str << v.second << ":" << "\"" << v.first << "\"" << s::comma();
  ctx.str << s::ccommalist() << "}" << s::endl << s::endl;

  OutputCodeFileChunck::storeCode(syntaxerContext.generatePythonRepr, ctx.str.str());
}



void ModelStatistic::generatePythonRepr() {
  Codestream str;
  str << "# -*- coding: cp1251 -*- " << s::endl << s::endl;
  str << "tables = [ " << s::endl;
  for (UserDataMap::value_type &uctx : syntaxerContext.model->userMap)
    for (UserContext::Tables::value_type &tbl : uctx.second->tables) {
      str << s::tab(2);
      tbl.second->pythonDefinition(str);
      str << ',' << s::endl;
    }
  str << "]" << s::endl << s::endl;

  str << "views = [ " << s::endl;
  for (UserDataMap::value_type &uctx : syntaxerContext.model->userMap)
    for (UserContext::Views::value_type &tbl : uctx.second->views) {
      str << s::tab(2);
      tbl.second->pythonDefinition(str);
      str << ',' << s::endl;
    }
  str << "]" << s::endl << s::endl;
  OutputCodeFileChunck::storeCode(syntaxerContext.generatePythonRepr, str.str());
}

string DependenciesAnalyzer::name(ResolvedEntity *def)  {
  Codestream str;
  str.state().procMode(CodestreamState::SQL);
  def->sqlReference(str);
  return PlsqlHelper::quotingAndEscapingPython1(str.str());
}

void DependenciesAnalyzer::addArcAndTraverseDeep(
    Sm::ResolvedEntity *to,
    Sm::ResolvedEntity *from,
    const DepArcContext &arcCtx,
    bool needRecursiveBuild)
{
  std::pair<From::iterator, bool> it  = dependenciesGraph[to].insert(From::value_type(from, arcCtx));
  if (it.second) {
    nodes.insert(from);
    if (needRecursiveBuild)
      buildGraph(from);
  }
  else
    it.first->second |= arcCtx;
}

DependenciesAnalyzer::DependenciesAnalyzer() {}

void DependenciesAnalyzer::buildGraph(Sm::ResolvedEntity *to) {
  using namespace Sm;
  bool fromIsView = to->toSelfView() != 0;

  for (DependEntitiesMap::value_type &v : to->lazyAttributes()->inArcs) {
    // Пересоздвать все вью, использующие вью (если baseEntity - это вью)
    // или вызывающие процедуру (если baseEntity - это процедура)
    Trigger  *trig = nullptr;
    Function *proc = nullptr;

    if (View *view = v.first->toSelfView())
      addArcAndTraverseDeep(to, view, v.second, /*needRecursiveBuild=*/true);
    else if ((proc = v.first->toSelfFunction()) || (trig = v.first->toSelfTrigger())) {
      // Процедуру или триггер нужно добавлять в зависящие, если baseEntity - это вью и
      // этот вью стоит в операторе EXECUTE, но без DIRECT
      if (fromIsView && v.second.isExecuteWithoutDirect())
        addArcAndTraverseDeep(to, v.first, v.second, true);

    }
  }
}

const char *DependenciesAnalyzer::toPythonCategory(Sm::ResolvedEntity *ent) const {
  if (Sm::Function *func = ent->toSelfFunction())
    if (func->flags.isXmlFunction())
      return NULL;
  switch (ent->ddlCathegory()) {
  case ResolvedEntity::View_:
    return "b'VIEW'";
  case ResolvedEntity::Function_:
  case ResolvedEntity::MemberFunction_:
    return "b'PROCEDURE'";
  case ResolvedEntity::Trigger_:
    return "b'TRIGGER'";
  case ResolvedEntity::Sequence_:
    return "b'SEQUENCE'";
  case ResolvedEntity::Table_:
    return "b'TABLE'";
  /*
  case ResolvedEntity::IndexUnique_:
      return "b'UNIQUE'";
  case ResolvedEntity::Index_:
    return "b'INDEX'";
  case ResolvedEntity::Variable_:
    return "b'VARIABLE'";
    */
  default:
    return NULL;
  }
}

string DependenciesAnalyzer::toPythonDepDict(GraphNodes &nodes) {
  // сгенерировать выходной словарь
  typedef std::map<string, vector<string> > NamesMap;
  NamesMap namesMap;

  for (Sm::ResolvedEntity* ent : nodes) {
    const char *catName = toPythonCategory(ent);
    if (!catName)
      continue;
    Codestream str;
    str.state().procMode(CodestreamState::SQL);

    str << "b'";
    ent->sqlReference(str);
    str << "'";
    string trStr = str.str();
    namesMap[catName].push_back(trStr);
  }

  Codestream outmap;
  outmap << "{";
  bool isNotFirst = false;
  for (const NamesMap::value_type &item : namesMap) {
    outmap << s::comma(&isNotFirst) << s::endl << s::tab(2) << item.first << ": {" << s::endl;
    bool valIsNotFirst = false;
    outmap << s::ocolumn();
    for (const string &val : item.second)
      outmap << s::comma(&valIsNotFirst) << s::tab(4) << val;
    outmap << s::ccolumn();
    outmap << s::endl << "  }";
  }
  outmap << s::endl << "}";
  return outmap.str();
}

string DependenciesAnalyzer::toPythonDepList(DependenciesStruct::SortedEntities &nodes) {
  Codestream outmap;
  outmap.procMode(CodestreamState::SQL);
  outmap << "[";
  bool isNotFirst = false;
  for (const DependenciesStruct::SortedEntities::value_type &item : nodes) {
    const char *catName = toPythonCategory(item);
    if (!catName)
      continue;
    outmap << s::comma(&isNotFirst) << s::endl << s::tab(2);
    outmap << "(" << catName << s::comma();
    outmap << "b'" << s::CRef(item) << "')";
  }
  outmap << s::endl << "]";
  return outmap.str();
}


string DependenciesAnalyzer::toPythonDepGraphDict() {
  Codestream outmap;
  outmap << "[";
  bool isNotFirst = false;
  for (DependenciesGraph::value_type &v : dependenciesGraph) {
    outmap << s::comma(&isNotFirst) << s::endl << s::tab(2)
             <<  "{\"to\":" << name(v.first) << ", \"from\": [" << s::endl;
    outmap << s::otablevel(4);
    outmap << s::tab(4);
    outmap << s::ocolumn();
    bool idIsNotFirst = false;
    for (From::value_type &f : v.second) {
      outmap << s::comma(&idIsNotFirst) << "{\"id\":" << name(f.first) << ", \"refs\":[";
      bool refIsNotfirst = false;
      outmap << s::ocolumn();
      for (DepArcContext::References::value_type r : f.second.references)
        outmap << s::comma(&refIsNotfirst) <<
                  "{\"refid\":" << name(r) << ","
               << "\"loc\":" << "'" << r->getLLoc().toString() << "'" << "}";

      outmap << s::ccolumn();
      outmap << "]}";
    }
    outmap << s::ccolumn();
    outmap << s::ctablevel();
    outmap << "]}";
  }
  outmap << s::endl << "]";

  return outmap.str();
}

void DependenciesAnalyzer::layingBySortedEntities(DependenciesStruct::SortedEntities *sortedEntities) {
  Sm::sAssert(!sortedEntities);
  layedNodes.clear();

  GraphNodes nodesCopy = this->nodes;
  for (DependenciesStruct::SortedEntities::iterator it = sortedEntities->begin(); it != sortedEntities->end(); ++it) {
    ResolvedEntity *ent = (*it)->getDefinitionFirst();
    GraphNodes::iterator fIt = nodesCopy.find(ent);
    if (fIt != nodesCopy.end()) {
      nodesCopy.erase(fIt);
      layedNodes.push_back(ent);
    }
  }
  Sm::sAssert(!nodesCopy.empty());
}




/// Получить все сущности, которые зависят от сущностей, заданных в конфиге
void ModelStatistic::getConfiguredDependEntities() {
  // получить множество зависимостей
  DependenciesAnalyzer depGraph;
  bool allGraph = syntaxerContext.model->modelActions.entitiesForDependFind.size() == 0;

  if (!allGraph) {
    if (syntaxerContext.model->modelActions.dependEntitiesLinter) {
      LinterSemanticResolver lsr;
      for (IdEntitySmart &id : syntaxerContext.model->modelActions.entitiesForDependFind) {
        lsr.resolve(id);
        ResolvedEntity *def = id.entity()->unresolvedDefinition();
        if (def) {
          depGraph.baseEntities.insert(def);
          depGraph.buildGraph(def);
        }
        else {
          //cout << "Unresolved entity for depend find " << id << endl;
        }
      }
    }
    else {
      for (IdEntitySmart &id : syntaxerContext.model->modelActions.entitiesForDependFind) {
        id.resolveByModelContext();
        ResolvedEntity *def = id.entity()->unresolvedDefinition();
        if (def) {
          depGraph.baseEntities.insert(def);
          depGraph.buildGraph(def);
        }
        else {
          //cout << "Unresolved entity for depend find " << id << endl;
        }
      }
    }
    depGraph.layingBySortedEntities(this->sortedEntities);
  }

  if (!syntaxerContext.dependencyAnalyzerOutFileName.empty()) {
    stringstream result;
    result << "baseEntities = " << depGraph.toPythonDepDict(depGraph.baseEntities) << endl << endl;
    if (allGraph && sortedEntities)
      result << "values = "     << depGraph.toPythonDepList(*sortedEntities) << endl << endl;
    else
      result << "values = "     << depGraph.toPythonDepList(depGraph.layedNodes  ) << endl << endl;

    if (syntaxerContext.dependencyAnalyzerOutDepGraph)
      result << "depGraph = " << depGraph.toPythonDepGraphDict();
    OutputCodeFileChunck::storeCode(syntaxerContext.dependencyAnalyzerOutFileName, result.str());
  }
}


void ModelStatistic::showComparsionLists() {


}


void ModelStatistic::showComparsionDatatypes() {
  using namespace pl_expr;

  typedef map<string, int> Stat;
  Stat stat;

  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;

    if (Comparsion *c = expr->toSelfComparion()) {
      Codestream str;

      Ptr<Datatype> lt = SubqueryUnwrapper::unwrap(c->lhs->getDatatype().object());
      std::vector<Ptr<Datatype> > rt;
      for (Comparsion::RHS::value_type &v : *(c->rhs))
        rt.push_back(SubqueryUnwrapper::unwrap(v->getDatatype().object()));
      rt.push_back(lt);

      static const auto similarTypes = [](vector<Ptr<Datatype> > &v, bool (Datatype::*ptr)() const) -> bool {
        for (Ptr<Datatype> &t : v)
          if (!(t.object()->*ptr)() && !t->isNull())
            return false;
        return true;
      };
      if (similarTypes(rt, &Datatype::isCharVarchar) || similarTypes(rt, &Datatype::isNum) || similarTypes(rt, &Datatype::isDateDatatype) ||
          similarTypes(rt, &Datatype::isBool) || similarTypes(rt, &Datatype::isBigint))
        return FLAG_REPLACE_TRAVERSE_NEXT;

      rt.pop_back();
      str << lt;
      str << " " << (c->isNot() ? ComparsionOp::toInvertedString(c->op) : ComparsionOp::toString(c->op))
          << " " << QuantorOp::toString(c->quantor);
      bool isNotFirst = false;
      bool needBr = rt.size() > 1;
      if (needBr)
        str << "(";
      for (Ptr<Datatype> &v : rt) {
        if (isNotFirst)
          str << " ";
        else
          isNotFirst = true;
        str << " " << SubqueryUnwrapper::unwrap(v->getDatatype());
      }
      if (needBr)
        str << ")";

      if (lt->isDateDatatype() || rt.front()->isDateDatatype())
        str << " " << s::iloc(expr->getLLoc());


      string s = str.str();
      pair<Stat::iterator, bool> it = stat.insert(make_pair(s, 1));
      if (!it.second)
        ++it.first->second;
    }
    return FLAG_REPLACE_TRAVERSE_NEXT;
  };


  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr*  { return e; };
  ExprTR tr(trFun, cond);
  cntx_->replaceChildsIf(tr);
  for (Stat::value_type &v : stat)
    cout << setw(5) << v.second << " " << v.first << endl;

  exit(0);
}


void ModelStatistic::showToCharArguments() {
  struct Attribute {
    int sqlCount = 0;
    int plCount = 0;

    int sum() const { return sqlCount + plCount; }

    std::vector<string> locs;
    Attribute();

    Attribute(int sql, int pl)
      : sqlCount(sql), plCount(pl) {}

    bool operator>(const Attribute &oth) const {
      int s1 = sum();
      int s2 = oth.sum();
      if (s1 > s2)
        return true;
      else if (s1 == s2)
        return sqlCount > oth.sqlCount || (sqlCount == oth.sqlCount && plCount > oth.plCount);
      else
        return false;

    }
  };
                         /*sql, pl */
  typedef std::pair<string, Attribute > Item;
  typedef map<Item::first_type, Item::second_type> Stat;
  Stat stat;

  ExprTR::Cond cond = [&](PlExpr* expr, ExprTr, bool construct) -> int {
    if (construct)
      return FLAG_REPLACE_TRAVERSE_NEXT;

    if (Ptr<IdEntitySmart> ent = expr->getMajorIdEntity())
      if (Ptr<Id> fun = ent->entity())
        if (ResolvedEntity *f = fun->definition())
          if (f->toSelfFunction())
            if (fun->toNormalizedString() == "TO_CHAR") {
              if (fun->callArglist && fun->callArglist->size() > 1) {
                SemanticTree *n = expr->getSemanticNode();
                bool isSql = n && n->isSqlCode();

                Codestream str;
                for (CallArgList::iterator it = next(fun->callArglist->begin()); it != fun->callArglist->end(); ++it)
                  str << *it << " ";
                string s = str.str();
                std::pair<Stat::iterator,bool> it = stat.insert(Stat::value_type(s, Stat::mapped_type(isSql ? 1 : 0, isSql ? 0 : 1)) );
                if (!it.second) {
                  if (isSql)
                    ++it.first->second.sqlCount;
                  else
                    ++it.first->second.plCount;
                }
                if (s[0] != '"') {
                  stringstream str;
                  str << expr->getLLoc();
                  it.first->second.locs.push_back(str.str());
                }
              }
            }

    return FLAG_REPLACE_TRAVERSE_NEXT;
  };

  ExprTR::Tr trFun = [&](PlExpr *e) -> PlExpr*  { return e; };
  ExprTR tr(trFun, cond);
  cntx_->replaceChildsIf(tr);

  typedef vector<Item> VStat;
  VStat vstat(stat.begin(), stat.end());

  std::sort(vstat.begin(), vstat.end(), [](Item v1, Item v2) -> bool { return v1.second > v2.second; } );

  for (VStat::value_type &v : vstat) {
    cout << "sql: " << setw(4) << v.second.sqlCount << " proc:" << setw(4) << v.second.plCount << " " << v.first << " ";
    if (v.second.locs.size())
      for (string &s : v.second.locs)
        cout << s << " ";
    cout << endl;
  }

  exit(0);
}



static StatementContext printFunctionHeader(PlExpr *expr, Codestream &str) {
  str << "--------- ";
  StatementContext stmtContext = expr->getOwnerStatementContext();
  BlockPlSql *topBlock = 0;
  if (BlockPlSql *b = stmtContext.statement->getOwnerBlock())
    topBlock = b->getTopBlock();
  if (!topBlock)
    throw 999;
  stmtContext.topBlock = topBlock;
  SemanticTree *n = topBlock->getSemanticNode();
  if (n)
    n = n->getParent();
  else
    throw 999;
  if (!n->unnamedDdlEntity)
    throw 999;
  if (n->unnamedDdlEntity->toSelfFunction() || n->unnamedDdlEntity->toSelfTrigger())
    str << n->unnamedDdlEntity->getName() << " " << s::iloc(n->unnamedDdlEntity->getLLoc());
  else
    throw 999;
  str << s::endl;
  return stmtContext;
}

void translateSelfStatements(Codestream &str, const string &header, std::vector<PlExpr*> &execImmStrings)
{
  str.state().isModelStatistic = true;
  str.state().procMode(CodestreamState::PROC);

  std::vector<string> heads;
  for (PlExpr *expr : execImmStrings) {
    Codestream dbgHStr;
    dbgHStr.state().isModelStatistic = true;
    dbgHStr.state().procMode(CodestreamState::PROC);
    StatementContext cntx = printFunctionHeader(expr, dbgHStr);
    Codestream dbgStr;
    dbgStr.state().isModelStatistic = true;
    dbgStr.state().procMode(CodestreamState::PROC);
    StatementsTranslator::translateStatement(dbgStr, cntx.statement, cntx.topBlock);
    string s = dbgStr.str() + " " + dbgHStr.str();
    std::replace(s.begin(), s.end(), '\n', ' ');
    std::replace(s.begin(), s.end(), '\r', ' ');
    heads.push_back(s);
  }
  std::sort(heads.begin(), heads.end());
  for (string &s : heads)
    str << s << s::endl;
  str << s::endl << s::endl << s::endl << "===== " << header << " WITH DEPENDENCIES =====" << s::endl << s::endl << s::endl;
}

//static void printSqlStatementStatistics(std::vector<PlExpr*> &execImmStrings, const string &filename, const string &header) {
//  Codestream str;
//  translateSelfStatements(str, header, execImmStrings);

//  for (PlExpr *expr : execImmStrings) {
//    printFunctionHeader(expr, str);
//    str.incIndentingLevel(2);

//    std::set<StatementInterface*> translatedOps;
//    pair<StatementInterface*, BlockPlSql*> v = extractAndOutputDependencyVariables(expr, str, translatedOps);
//    str.decIndentingLevel(2);
//    translateStatement(str, v.first, v.second);
//  }
//  OutputCodeFile::storeCode(filename, str.str());
//}

//static void printSqlExecImmStatistics(std::vector<PlExpr*> &execImmStrings, const string &filename, const string &header) {
//  Codestream str;
//  translateSelfStatements(str, header, execImmStrings);
//  OutputCodeFile::storeCode(filename, str.str());
//}

void ModelStatistic::extractExecuteImmediateAndOpenCursor() { throw 999; }

using namespace insert;
using namespace update;


void ModelContext::printBlobInserts() {
  auto printExprBlob = [](const char *stype, Ptr<SqlExpr> &expr) {
    if (Ptr<Datatype> datatype = expr->getDatatype())
      if (datatype->isBlobDatatype() || datatype->isClobDatatype()) {
        string exprStr;
        expr->toNormalizedString(exprStr);
        cout << stype << "  " << expr->getLLoc()
             << (datatype->isClobDatatype() ? "  CLOB  " : "  BLOB  ")
             << exprStr << endl;
      }
  };

  Sm::StmtTrCond cond = [&](StatementInterface *lastStmt, bool constructor, list<Ptr<StatementInterface> > &/*stmts*/, list<Ptr<StatementInterface> >::iterator &/*it*/) -> int {
    if (!constructor)
      return FLAG_REPLACE_TRAVERSE_ONLY;
    else if (SingleInsert *curStmt = lastStmt->toSelfSingleInsert()) {
      if (curStmt->data->cathegoryInsertFrom() != InsertFrom::VALUES)
        return FLAG_REPLACE_TRAVERSE_ONLY;

      InsertFromValues *ifv = static_cast<InsertFromValues*>(curStmt->data.object());
      if (ifv->value->cathegoryInsertingValues() != InsertingValues::EXPRESSION_LIST)
        return FLAG_REPLACE_TRAVERSE_ONLY;

      InsertingExpressionListValues *valueList = static_cast<InsertingExpressionListValues*>(ifv->value.object());
      if (valueList->exprList) {
        for (Ptr<SqlExpr> &expr : *valueList->exprList) {
          printExprBlob("INSERT", expr);
        }
      }
    }
    else if (Sm::Update *curStmt = lastStmt->toSelfUpdate()) {
      if (curStmt->setClause->cathegorySetClause() != SetClause::UPDATING_LIST)
         return FLAG_REPLACE_TRAVERSE_ONLY;

      SetUpdatingList *updList = static_cast<SetUpdatingList*>(curStmt->setClause.object());
      if (updList->updatingList)
        for (Ptr<PlExpr> &item : *updList->updatingList)
          if (FieldFromExpr *fieldExpr = item->toSelfFieldFromExpr())
            printExprBlob("UPDATE", fieldExpr->expr);
    }

    return FLAG_REPLACE_TRAVERSE_ONLY;
  };

  cout << "======= BLOB in inserts, updates =======" << endl;

  StmtTr tr = [](StatementInterface *s, list<Ptr<StatementInterface> > *, list<Ptr<StatementInterface> >::iterator *) -> StatementInterface* { return s; };
  replaceStatementsIf(tr, cond);

  cout << "================= END ==================" << endl;
}

void printEntityNodeLoc(Codestream &str,
                        const string   &cathegory,
                        string         &uname,
                        const string   &ename,
                        cl::filelocation &l, size_t eid) {
  str << "    ('" << cathegory << "', '"
      << uname << "', '" << ename << "', ";

  string fullname = syntaxerContext.parsedFilenames.findFullname(l.file);
  str << l.loc.begin.bytePosition << ", " << l.loc.end.bytePosition << ", " << l.loc.begin.line << ", " << l.loc.end.line
      << ", " << eid << ", '" << fullname << "')," << s::endl;
}

void printEntityNodeLoc(Codestream &str,
                        const string   &cathegory,
                        string         &uname,
                        const string   &ename,
                        ResolvedEntity *e) {
  str << "    ('" << cathegory << "', '"
      << uname << "', '" << ename << "', ";
  cl::filelocation l = e->getLLoc();

  string fullname = syntaxerContext.parsedFilenames.findFullname(l.file);
  str << l.loc.begin.bytePosition << ", " << l.loc.end.bytePosition << ", " << l.loc.begin.line << ", " << l.loc.end.line
      << ", " << e->eid() << ", '" << fullname << "')," << s::endl;
}

void printEntityNodeLoc(Codestream &str, const string &cathegory, string &uname, ResolvedEntity *e) {
  printEntityNodeLoc(str, cathegory, uname, e->getName()->toNormalizedString(), e);
}

void printPackageNodeLoc(Codestream &str, string& uname, Package *p) {
  string name = p->getName()->toNormalizedString();
  if (name == "SR_PKG")
    cout << "";
  for (Package::Container::value_type &c : p->heads)
    printEntityNodeLoc(str, "PACKAGE", uname, name, c);
  for (Ptr<BlockPlSql> &c : p->bodies) {
    for (Ptr<Declaration> &decl : c->declarations) {
      string cathegory = decl->ddlCathegoryToString();
      if (!decl->toSelfFunction())
        continue;
      transform(cathegory.begin(), cathegory.end(), cathegory.begin(), ::toupper);
      printEntityNodeLoc(str, cathegory, uname, name, c);
    }
    if (!c->statements.empty()) {
      cl::filelocation loc = c->statements.front()->getLLoc();
      loc.loc.end = c->statements.back()->getLLoc().loc.end;
      printEntityNodeLoc(str, "PACKAGE INITIALIZER", uname, name, loc, c->eid());
    }

  }
}

void ModelContext::extractLocFiles() {
  Codestream str;
  str << "# -*- coding: cp1251 -*- " << s::endl << s::endl;
  str << "entities = [ " << s::endl;

  for (UserDataMap::value_type &it : userMap) {
    string username = it.second->getName()->toNormalizedString();
    for (UserContext::Views::value_type &vIt : it.second->views)
      printEntityNodeLoc(str, "VIEW", username, vIt.second.object());
    for (UserContext::Functions::value_type &vIt : it.second->functions)
      printEntityNodeLoc(str, "FUNCTION", username, vIt.second.object());
    for (UserContext::Packages::value_type &vIt : it.second->packages)
      printPackageNodeLoc(str, username, vIt.second);
    for (UserContext::Triggers::value_type &vIt : it.second->triggers)
      printEntityNodeLoc(str, "TRIGGER", username, vIt.second.object());
    // TODO: - Доделать для объектов
  }
  str << "  ]" << s::endl;
  OutputCodeFile::storeCode(syntaxerContext.exportEntitiesLocationFile, str.str());
}



void SemanticTree::printComplexReferences(string tab/*=string()*/) {
  static size_t countOfMultiArray = 0;
  static int level = 0;
  string str;
  if (refEntity() && !(refDefinition())) {
    return;
  }

  ++level;

  if (!refEmpty() && referenceName_->size() > 0 && referenceName_->majorObjectRef(NULL)) {
    bool prnt = false;
    bool multiarr = false;
    ResolvedEntity *prevDef = NULL;
    for (auto it = referenceName_->begin(); it != referenceName_->end(); ++it) {
      if ((*it)->definition()->isCollectionAccessor()) {
        if (prevDef && prevDef->isCollectionAccessor()) {
          cout << "Multiarray ->";
          multiarr = true;
        }
        prnt = true;
      }
      prevDef = (*it)->definition();
    }

    if (multiarr)
      countOfMultiArray++;

    if (prnt)
      referenceName_->toStringWithType(str);
  }
  if (str.size() && !syntaxerContext.generateFullStatistic)
    cout << tab << str << endl << flush;

  for (SemanticTree::Childs::iterator it = childs.begin(); it != childs.end(); ++it)
    (*it)->printComplexReferences(nametype != EMPTY ? tab + "  " : tab);

  if (--level == 0) {
    cout << "Summary count of multiarrays " << countOfMultiArray << endl;
  }
}

// TODO: необходимо ли это задействовать в генерации полной статистики?
void SemanticTree::printCursorVariables() {
  Codestream str;

  std::function<bool (Ptr<Datatype> datatype)> hasCursorRecord = [&](Ptr<Datatype> datatype) -> bool {
    datatype = Datatype::getLastConcreteDatatype(datatype);
    if (!datatype)
      return false;
    if (datatype->isRefCursor() || datatype->isRowTypeOf() || datatype->isRecordType())
      return true;
    if (!datatype->isObjectType())
      return false;

    if (Ptr<Datatype> mappedType = datatype->getNextDefinition()->mappedType()) {
      if (hasCursorRecord(mappedType))
        return true;

      EntityFields fields;
      mappedType->getFields(fields);
      for (EntityFields::reference f : fields)
        if (hasCursorRecord(f->getDatatype()))
          return true;
    }
    return false;
  };

  auto isComplexVar = [&](SemanticTree *n) -> bool {
    if (n->nametype != SemanticTree::DECLARATION || n->cathegory != SCathegory::Variable)
      return false;
    Variable *var = n->refDefinition()->toSelfVariable();
    //if (!var->isPackageVariable())
    //  return false;
    if (var->toSelfVariableField())
      return false;
    return var->isRefCursor() || hasCursorRecord(var->getDatatype());
  };

  SemanticTree::EnumList varList;
  varList.reserve(200);
  enumNodes(isComplexVar, varList);

  // Вывод переменных и их типов.

  for (SemanticTree *varNode : varList) {
    Variable *var = varNode->refDefinition()->toSelfVariable();
    switch (var->owner()->ddlCathegory()) {
    case ResolvedEntity::Package_:
      str << s::linref(var->userContext()) << "." << var->owner()->getName()->toNormalizedString();
      break;
    case ResolvedEntity::BlockPlSql_: {
      BlockPlSql *block = var->owner()->toSelfBlockPlSql()->getTopBlock();
      str << s::linref(block->owner());
    } break;
    default:
      str << s::linref(var->owner());
      break;
    }
    str << "\t";
    str << s::def << var;
    str << s::endl;
  }
  str << "=== end declarations. count " << varList.size() << "===" << s::endl;

  /*
  // Вывод ссылок, участвующих в открытии курсора.
  ResolvedEntity *needDef;
  auto isRefTo = [&needDef](SemanticTree *n) -> bool {
    if (n->nametype != SemanticTree::REFERENCE ||
        n->cathegory != SCathegory::RefAbstract)
      return false;

    if (Ptr<Id> major = n->reference()->majorEntity())
      if (major->definition() == needDef)
        if (n->getParent()->cathegory == SCathegory::OpenCursor ||
            n->getParent()->cathegory == SCathegory::StatementOpenFor)
          return true;
    return false;
  };

  SemanticTree::EnumList refList;
  refList.reserve(200);
  for (SemanticTree *varNode : varList) {
    needDef = varNode->declarationNameDef();
    //enumNodes(isRefTo, refList);
    needDef->owner()->getSemanticNode()->enumNodes(isRefTo, refList);
  }

  for (SemanticTree *refNode : refList) {
    str << *refNode->reference() << s::endl;
  }

  str << "=== end ===" << s::endl;
  */
  cout << str.str();
}

void SemanticTree::printSystemFunctions() {
  struct EntityProps {
    int count;
    union {
      struct {
        unsigned isPL       : 1;
        unsigned isSQL      : 1;
        unsigned isSQLinPL  : 1;
      };
      unsigned v;
    } flags;
    EntityProps() { count = 0; flags.v = 0; }
  };
  typedef std::map<ResolvedEntity*, EntityProps, LE_ResolvedEntities> FunctionMap;
  typedef std::map<string, int> NameMap;
  typedef std::multimap<int, string> CountMap;
  FunctionMap traversed;
  NameMap packagedFuncs;
  NameMap globalFuncs;
  EnumList funcNodes;

  auto isSystemFuncRef = [](SemanticTree *node) -> bool {
    static bool isSysUser = false;
    if (node->cathegory == SCathegory::User)
      isSysUser = node->ddlEntity()->isSystem();
    if (isSysUser)
      return false; // Игнорируем референсы функций внутри системных пользователей

    if (node->nametype == REFERENCE)
      if (ResolvedEntity *def = node->refDefinition()) {
        if (def->isFunction() && (def->isSystem() || def->isSystemTemplate()) &&
            !def->isMethod() && !def->isConstructor() && !def->isObjectType())
          return true;
      }
    return false;
  };

  auto printSortedFuncs = [](const NameMap &funcs) {
    CountMap countMap;

    cout << "COUNT:" << "\t" << "NAME:" << std::endl;
    // Пересортируем для вывода по количеству
    for (NameMap::const_reference namePair : funcs) {
      countMap.insert(CountMap::value_type(namePair.second, namePair.first));
    }

    for (CountMap::reverse_iterator cit = countMap.rbegin(); cit != countMap.rend(); ++cit) {
      cout << cit->first << "\t" << cit->second << std::endl;
    }
  };

  enumNodes(isSystemFuncRef, funcNodes);
  for (SemanticTree *node : funcNodes) {
    ResolvedEntity *def = node->refDefinition();
    traversed[def].count += 1;
    traversed[def].flags.isPL       |= node->isPlContext();
    traversed[def].flags.isSQL      |= node->isNotPlContext();
    traversed[def].flags.isSQLinPL  |= node->isSqlCodeInPlCode();
  }

  cout << "System functions" << std::endl;
  for (FunctionMap::const_reference p : traversed) {
    Codestream s;
    s.dbMode(CodestreamState::ORACLE);
    Function *f = p.first->toSelfFunction();
    s << f->funArglist();

    string funcName;
    if (f->ownerPackage())
      funcName = f->ownerPackage()->getName()->toNormalizedString() + ".";
    funcName += f->getName()->toNormalizedString();

    cout << funcName << '(' << s.str() << ')';// << std::endl;
    //cout << " has linter analog " << p.first->isElementaryLinterFunction();
    cout << "  count " << p.second.count;// << std::endl;
    if (p.second.flags.isPL)
      cout << "  inPl ";
    if (p.second.flags.isSQL)
      cout << " inSql ";
    if (p.second.flags.isSQLinPL)
      cout << " inSqlInPL ";
    cout << std::endl;

    if (f->ownerPackage())
      packagedFuncs[funcName] += p.second.count;
    else
      globalFuncs[funcName] += p.second.count;
  }

  cout << "All system funcs count " << traversed.size() << std::endl << std::endl;
  cout << "System functions sorted by count:" << std::endl;
  printSortedFuncs(globalFuncs);
  cout << std::endl;
  cout << "System package functions sorted by count:" << std::endl;
  printSortedFuncs(packagedFuncs);
}




