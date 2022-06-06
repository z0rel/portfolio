#include <assert.h>
#include <algorithm>
#include "hash_table.h"
#include "system_functions.h"
#include "resolvers.h"
#include "model_context.h"
#include "syntaxer_context.h"
#include "semantic_function.h"
#include "semantic_expr.h"
#include "semantic_blockplsql.h"
#include "smart_lexer.h"


// TODO: для каждой функции реализовать их эквиваленты в Линтер
using namespace BackportHashMap;
using namespace std;

extern SyntaxerContext syntaxerContext;

namespace Sm {
  typedef FunctionArgumentContainer Arg;




typedef vector<Ptr<Function> > CreatedFunctions;

inline char squote(bool isProc) {
  return isProc ? '"' : '\'';
}

inline char squote(const Sm::Codestream &str) {
  return squote(str.isProc());
}

inline bool isEmptyArg(Ptr<FunCallArg> arg) {
  SqlExpr *sqlExpr = arg->expr()->toSelfSqlExpr();
  if (sqlExpr && sqlExpr->isEmptyId())
    return true;
  return false;
}

void trToNumber(Sm::Codestream &str, Ptr<CallArgList> args) {
  if (str.isSql() && args->size() == 1 && isEmptyArg(*args->begin()))
    str << "CAST";
  else if (str.isSql())
    str << "TO_NUMBER";
  else
    str << "tonumeric";
}


void trToNumberArgs(Ptr<Id> &call, Sm::Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  if (str.isSql() && args->size() == 1 && isEmptyArg(*args->begin()))
    str << "NULL AS NUMBER";
  else
    str << (*args->begin());
}

void trToNumberNum(Sm::Codestream &str, Ptr<CallArgList> /*args*/) {
  if (str.isSql())
    str << "CAST";
  else
    str << "tonumeric";
}

void trToNumberNumArgs(Ptr<Id> &call, Sm::Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  if (str.isSql()) {
    if (args->size() == 1 && isEmptyArg(*args->begin()))
      str << "NULL AS NUMBER";
    else
      str << (*args->begin()) << " AS NUMBER";
  }
  else {
    // tonumeric(NULL) - вернет NULL
    str << (*args->begin());
  }
}

void trTruncName(Sm::Codestream &str, Ptr<CallArgList> /*args*/) {
  if (str.procMode() == CodestreamState::SQL)
    str << "TRUNC";
  else
    str << "date_trunc";
}


void trTruncDate(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  static std::map<std::string, std::string>  formatOra2Lin = {
      {"YEAR" , "YY"},
      {"SYYYY", "YY"},
      {"YYYY" , "YY"},
      {"SYEAR", "YY"},
      {"YYY"  , "YY"},
      {"YY"   , "YY"},
      {"MONTH", "MM"},
      {"MON"  , "MM"},
      {"MM"   , "MM"},
      {"M"    , "MM"},
      {"RM"   , "MM"},
      {"DDD"  , "DD"},
      {"DD"   , "DD"},
      {"J"    , "DD"},
      {"DAY"  , "D" },
      {"DY"   , "D" },
      {"D"    , "D" },
      {"HH"   , "HH"},
      {"HH12" , "HH"},
      {"HH24" , "HH"},
      {"MI"   , "MI"},
  };

  if (argl->size() == 1) {
    str << argl << s::comma() << (str.isSql() ? "'DD'" : "\"DD\"");
    return;
  }
  else if (argl->size() == 2) {
    if (Ptr<FunCallArg> secondArg = *(++(argl->begin()))) {
      std::string sVal;
      secondArg->expr()->toNormalizedString(sVal);
      std::transform(sVal.begin(), sVal.end(), sVal.begin(), ::toupper);
      auto it = formatOra2Lin.find(sVal);
      if (it != formatOra2Lin.end())
      {
        std::string sRes;
        if (str.isSql())
          sRes = "\'" + it->second + "\'";
        else
          sRes = "\"" + it->second + "\"";
        str << *(argl->begin()) << s::comma() << sRes;
        return;
      }
    }
  }
  str << argl;
}


string numericFunname(Codestream &str) {
  return str.isSql() ? "TO_NUMBER" : "tonumeric";
}


void trArgsToNumber(Ptr<Id> &call, Codestream &str) {
  bool isFirst = false;
  if (Ptr<CallArgList> argl = call->callArglist)
    for (CallArgList::value_type &v : *argl) {
      Ptr<Datatype> datatype = Datatype::getLastConcreteDatatype(v->getDatatype());
      if (str.isSql() && datatype && datatype->isNum()) {
        if (datatype->isNumberDatatype())
          str << s::comma(&isFirst) << v;
        else
          str << s::comma(&isFirst) << "CAST" << s::obracket << v << " AS NUMBER" << s::cbracket;
      }
      else
        str << s::comma(&isFirst) << numericFunname(str) << s::obracket << v << s::cbracket;
    }
}

void argToChar(FunCallArg *arg, Codestream &str)
{
  if (arg->expr()->isNumericValue())
    str << squote(str) << arg->expr() << squote(str);
  else if (arg->getDatatype()->isClobDatatype())
    str << "tochar(" << arg->expr() << ")";
  else
    str << "to_char(" << arg->expr() << ")";
}

void firstArgToChar(Ptr<Id> &call, Codestream &str) {
  if (Ptr<CallArgList> argl = call->callArglist) {
    CallArgList::iterator it = argl->begin();
    argToChar(*it, str);
    for (++it; it != argl->end(); ++it)
      str << s::comma() << *it;
  }
}


void translateCallarglistNvlVarcharNumber(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;

  if (argl && argl->size()) {
    CallArgList::iterator it = argl->begin();
    str << (*it++);
    for ( ; it != argl->end(); ++it) {
      str << s::comma();
      argToChar(*it, str);
    }
  }
}

void translateCallarglistNvlSecondArg(Ptr<Id> call, Codestream &str, const string &funname) {
  Ptr<CallArgList> argl = call->callArglist;
  if (argl && argl->size()) {
    CallArgList::iterator it = argl->begin();
    str << (*it++);
    for ( ; it != argl->end(); ++it) {
      str << s::comma() << funname << s::obracket << (*it)->expr() << s::cbracket;
    }
  }
}



void translateCallarglistNvlNumberVarchar(Ptr<Id> &call, Codestream &str) {
  translateCallarglistNvlSecondArg(call, str, str.isSql() ? "TO_NUMBER" : "tonumeric");
}

void translateCallarglistNvlVarcharDate(Ptr<Id> &call, Codestream &str) {
  translateCallarglistNvlSecondArg(call, str, "TO_CHAR");
}

void translateCallarglistNvlDateVarchar(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  CallArgList::iterator it = argl->begin();
  str << (*it++) << s::comma();
  str << "TO_DATE" << s::obracket << (*it) << s::comma() << squote(str) << "DD.MM.YYYY:HH24:MI:SS" << squote(str) << s::cbracket;
}


void trTruncNumberArgs(Ptr<Id> &call, Codestream &str) {
  str << call->callArglist;
  if (call->callArglist->size() == 1)
    str << s::comma() << 0;
}

void trTruncToNumber(Ptr<Id> &call, Codestream &str) {
  str << (str.isSql() ? "TO_NUMBER" : "tonumeric") << s::obracket << *(call->callArglist->begin()) << s::cbracket;
  if (call->callArglist->size() > 1)
    str << s::comma() << *(call->callArglist->begin() + 1);
  else
    str << s::comma() << 0;
}

#define TR_LEN(arg) "LENGTH" << s::obracket << arg << s::cbracket
#define TR_TO_CHAR(arg) "TO_CHAR" << s::obracket << arg << s::cbracket

void trSubstrIndex(Codestream &str, Ptr<FunCallArg> arg) {
  if (!arg->getDatatype()->isNum())
    str << "tointeger" << s::obracket << arg << s::cbracket;
  else
    str << arg;
}

void trSubstrArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  CallArgList::iterator it = argl->begin();
  Ptr<FunCallArg> arg1 = *it, arg2 = *(++it);
  str << arg1;
  if (arg2->expr()->ddlCathegory() == ResolvedEntity::SqlExprUnaryMinus_) {
    str << s::comma() << TR_LEN(arg1) << arg2 << s::name << "+" << s::name << 1;
    if (argl->size() == 2) {
      str << s::comma() << 1;
      return;
    }
  }
  else if (arg2->expr()->isNumericValue() && arg2->expr()->toSelfNumericValue()->getSIntValue() == 0)
    str << s::comma() << 1;
  else {
    str << s::comma();
    trSubstrIndex(str, arg2);
  }
  if (argl->size() == 2)
    str << s::comma() << TR_LEN(arg1);
  else {
    str << s::comma();
    trSubstrIndex(str, *(++it));
  }
}

void trRegexpSubstrArgs(Ptr<Id> &call, Codestream &str) {
  CallArgList &argl = *nAssert(call->callArglist);
  str << argl[0] << " SIMILAR " << argl[1] << " ESCAPE x";
}


void trArgToChar(Ptr<Id> &call, Codestream &str) {
  str << "TO_CHAR" << s::obracket << *(call->callArglist->begin()) << s::cbracket;
}


void trSubstrToChar(Ptr<Id> &call, Codestream &str) {
  if (Ptr<CallArgList> argl = call->callArglist) {
    CallArgList::iterator it = argl->begin();
    Ptr<FunCallArg> arg1 = *it, arg2 = *(++it);
    str << TR_TO_CHAR(arg1);
    if (arg2->expr()->ddlCathegory() == ResolvedEntity::SqlExprUnaryMinus_) {
      str << s::comma() << TR_LEN(TR_TO_CHAR(arg1)) << arg2 << s::name << "+" << s::name << 1;
      if (argl->size() == 2) {
        str << s::comma() << 1;
        return;
      }
    }
    else if (arg2->expr()->isNumericValue() && arg2->expr()->toSelfNumericValue()->getSIntValue() == 0)
      str << s::comma() << 1;
    else {
      str << s::comma();
      trSubstrIndex(str, arg2);
    }
    if (argl->size() == 2)
      str << s::comma() << TR_LEN(TR_TO_CHAR(arg1));
    else {
      str << s::comma();
      trSubstrIndex(str, *(++it));
    }

  }
}



void trSubstrClobArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  const int maxStrLength = 4000;
  auto it = argl->begin();
  Ptr<FunCallArg> arg1, arg2, arg3;
  arg1 = *(it);
  arg2 = *(++it);
  if (argl->size() > 2)
    arg3 = *(++it);

  str << "GETBLOBSTR" << s::obracket << arg1->expr() << s::comma() << arg2->expr() << s::comma();
  if (arg3)
    str << arg3->expr() << s::cbracket;
  else
    str << maxStrLength << s::cbracket;
  str << s::comma() << arg2->expr() << s::comma();
  if (arg3)
    str << arg3->expr();
  else
    str << maxStrLength;
}

void trToChar(Sm::Codestream &str, Ptr<CallArgList> args) {
  Ptr<Datatype> arg1Type = (*args->begin())->getDatatype();
  if (str.isSql() && arg1Type->isNull())
    str << "CAST";
  else if (!arg1Type->isCharVarchar())
    str << "TO_CHAR";
  //Иначе не выводим функцию.
}

void trToCharArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  Ptr<Datatype> arg1Type = (*args->begin())->getDatatype();
  if (str.isSql() && arg1Type->isNull())
     str << (*args->begin()) << " AS " << s::lindef(call->getDatatype());
  else if (!arg1Type->isCharVarchar())
    str << args;
  else
    str << (*args->begin());
}

void OraTemplateData::updateTemplateNamespace(Ptr<Function> fun)
{
  createdFunctions.push_back(fun);
  SemanticTree *funSTree = fun->toSTree();
  Sm::collectEqualsDeclarationOnNode(funSTree, syntaxerContext.model->levelNamespace());

  SemanticTree *modelNode = syntaxerContext.model->getSemanticNode();
  modelNode->childs.push_back(funSTree);
  funSTree->setParentForChilds(--modelNode->childs.end());
  fun->setIsSystemTemplate();
}


Ptr<Function> OraTemplateData::addSysFun(Function *f, CallarglistTranslator *tr)
{
  Ptr<Function> fun = f;
  if (tr)
    fun->callarglistTranslator = tr;
  updateTemplateNamespace(fun);
  fun->setElementaryLinFunc();
  return fun;
}

OraTemplateData::~OraTemplateData() {}


void OraTemplateData::updateDatatypeIfGreather(int &dstPos, Datatype **dst, int &callArgPos, FunCallArg &callArg, bool pl, const ConflictSelector &conflictSelector) {
  auto setDst = [&](Datatype *t) {
    if (*dst != t) {
      *dst = t;
      dstPos = callArgPos;
    }
  };
  if (Ptr<Datatype> argT = SubqueryUnwrapper::unwrap(callArg.getDatatype())) {
    if (Datatype *t = Datatype::getMaximal(*dst, argT.object(), pl))
      return setDst(t);
    else if (conflictSelector) {
      if (Datatype *t = conflictSelector(dstPos, *dst, callArgPos, argT, pl))
        return setDst(t);
    }
    else
      throw 999;
  }
}

Ptr<Datatype> OraTemplateData::getMaximalArgumentDatatype(TemplateArglist &callArgList, bool pl, const ConflictSelector &conflictSelector)
{
  TemplateArglist::iterator it = callArgList.begin();
  int tPos = 0;
  int itPos = 0;
  if (Datatype *t = SubqueryUnwrapper::unwrap((*it)->getDatatype())) {
    for (++it; it != callArgList.end(); ++it, ++itPos)
      OraTemplateData::updateDatatypeIfGreather(tPos, &t, itPos, **it, pl, conflictSelector);
    return t;
  }
  throw 999;
  return 0;
}

Ptr<Datatype> OraTemplateData::getImplicitCastedOracleRettype(Ptr<Datatype> t1, Ptr<Datatype> t2, bool isPlContext, Ptr<Sm::CallarglistTranslator> &tr) {
  static const auto isNumAndSubnum = [](Ptr<Datatype> &t1, Ptr<Datatype> &t2) -> bool {
    return t1->isNumberDatatype() &&
        (t2->isBigint() || t2->isInt() || t2->isSmallint() || t2->isFloat() || t2->isDouble() || t2->isReal());
  };
  static const auto numberVarchar = [](Ptr<Datatype> &t1, Ptr<Datatype> &t2) -> bool {
    return t1->isNum() && t2->isCharVarchar();
  };
  static const auto setTr = [](Ptr<Sm::CallarglistTranslator> &tr, Sm::CallarglistTranslator::TrFun fun) -> void {
    if (fun)
      tr = new CallarglistTranslatorSimple(fun);
    else
      throw 999;
  };

  if (t1->isNull())
    return t2;
  if (t2->isNull())
    return t1;

  if (Datatype *retT = Datatype::getMaximal(t1, t2, isPlContext))
    return retT;

  if (numberVarchar(t1, t2)) {
    // 2й аргумент нужно транслировать в цифру
    setTr(tr, translateCallarglistNvlNumberVarchar);
    return t1;
  }
  else if (numberVarchar(t2, t1)) {
    // 2й аргумент нужно транслировать в текст
    setTr(tr, translateCallarglistNvlVarcharNumber);
    return t1;
  }
  else if (t1->isCharVarchar() && t2->isDateDatatype()) {
    setTr(tr, translateCallarglistNvlVarcharDate);
    return t1;
  }
  else if (t1->isDateDatatype() && t2->isCharVarchar()) {
    setTr(tr, translateCallarglistNvlDateVarchar);
    return t1;
  }

  if (isNumAndSubnum(t1, t2))
    return t1;
  if (isNumAndSubnum(t2, t1))
    return t2;

  throw 999;
}

bool OraTemplateData::checkAdjust(TemplateArglist &callArgList, Mf mf, int pos) {
  if (Ptr<Datatype> t = SubqueryUnwrapper::unwrap(callArgList[pos]->getDatatype()))
    return (t.object()->*mf)();
  return false;
}


Ptr<Datatype> OraTemplateData::makeSubstrRettypeByFirstArg(Datatype *t, TemplateArglist &callArgList, unsigned int len) {
  Ptr<Datatype> rettype;
  if (len > 2) {
    long int n = callArgList[2]->expr()->getSIntValue();
    if (n > 0 && n < 4000)
      rettype = Datatype::mkVarchar2(n);
  }
  else {
    if (t->precision) {
      long int n = callArgList[1]->expr()->getSIntValue();
      if (n > 0) {
        if (n == 1)
          --n;
        long int l = static_cast<long int>(t->precision) - n;
        if (!l)
          rettype = Datatype::mkNull();
        if (l > 0 && l < 4000)
          rettype = Datatype::mkVarchar2(l);
      }
    }
  }
  if (!rettype)
    rettype = Datatype::mkVarchar2();
  return rettype;
}


bool OraTemplateData::checkAdjust(TemplateArglist &callArgList, Mf mf) {
  for (TemplateArglist::value_type &v : callArgList) {
    if (Ptr<Datatype> t = SubqueryUnwrapper::unwrap(v->getDatatype()))
      if (!(t.object()->*mf)())
        return false;
  }
  return true;
}

bool OraTemplateData::isAdjustChar(TemplateArglist &callArgList) { return checkAdjust(callArgList, &Datatype::isCharDatatype); }

bool OraTemplateData::isAdjustVarchar(TemplateArglist &callArgList) { return checkAdjust(callArgList, &Datatype::isVarcharDatatype); }
bool OraTemplateData::isAdjustVarchar(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isVarcharDatatype, pos); }


bool OraTemplateData::isAdjustCharVarchar(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isCharVarchar, pos); }
bool OraTemplateData::isAdjustChar(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isCharDatatype, pos); }
bool OraTemplateData::isAdjustNull(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isNull, pos); }
bool OraTemplateData::isAdjustAnydata(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isAnydata, pos); }
bool OraTemplateData::isAdjustBool(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isBool, pos); }
bool OraTemplateData::isAdjustClob(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isClobDatatype, pos); }
bool OraTemplateData::isAdjustBlob(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isBlobDatatype, pos); }

bool OraTemplateData::isAdjustDate(TemplateArglist &callArgList) { return checkAdjust(callArgList, &Datatype::isDateDatatype); }
bool OraTemplateData::isAdjustDate(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isDateDatatype, pos); }

bool OraTemplateData::isAdjustNumber(TemplateArglist &callArgList, int pos) { return checkAdjust(callArgList, &Datatype::isNumberDatatype, pos); }

bool OraTemplateData::isAdjustNum(TemplateArglist &callArgList, int pos) {
 return checkAdjust(callArgList, &Datatype::isNum, pos);
}

bool OraTemplateData::isAdjustNum(TemplateArglist &callArgList) {
 int end = callArgList.size();
 for (int i = 0; i < end; ++i)
   if (!isAdjustNum(callArgList, i))
     return false;
 return true;
}



Ptr<Function> OraTemplateData::findCreatedFunByRettype(Datatype *t, bool pl, size_t arglistSize) {
  for (Ptr<Function> &fun : createdFunctions)
    if (fun->arglistSize() == arglistSize) {
      if (t->isSubtype(fun->getDatatype().object(), pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY)
        return fun;
    }
  return 0;
}

Ptr<Function> OraTemplateData::createStandardSysFunction(Ptr<Datatype> rettype, size_t callArglistSize, bool isPlContext, const std::initializer_list<string> &argnames, CallarglistTranslator *tr) {
  if (Ptr<Function> f = findCreatedFunByRettype(rettype, isPlContext, callArglistSize))
    return f;
  else {
    Ptr<Arglist> funArgList = new Arglist();
    for (const string &n : argnames)
      funArgList->push_back(new FunctionArgument(new Id(string(n)), rettype, 0));
    return addSysFun(new Function(funname, funArgList, rettype), tr);
  }
}

Ptr<Function> OraTemplateData::createStandardSysFunction(TemplateArglist &callArgList, bool isPlContext, const std::initializer_list<string> &argnames, CallarglistTranslator *tr) {
  if (Ptr<Datatype> t = getMaximalArgumentDatatype(callArgList, isPlContext))
    return createStandardSysFunction(t, callArgList.size(), isPlContext, argnames, tr);
  return 0;
}


bool OraTemplateData::callArglistLenNEQ(TemplateArglist &callArgList, unsigned int len) { return callArgList.size() != len || callArgListUnresolved(callArgList); }
bool OraTemplateData::callArglistLenZGT(TemplateArglist &callArgList, unsigned int len) { return callArgList.empty() || callArgList.size() > len || callArgListUnresolved(callArgList); }
bool OraTemplateData::callArglistLenGTLT(TemplateArglist &callArgList, unsigned int len, unsigned int lowLen) { return callArgList.size() > len || callArgList.size() < lowLen || callArgListUnresolved(callArgList); }

bool OraTemplateData::callArgListUnresolved(TemplateArglist &callArgList) {
  for (TemplateArglist::value_type &v : callArgList)
    if (!(v->getDatatype()))
      return true;
  return false;
}

/**
 * @brief eqByArgs
 *   Проверка точного соответствия списка типов вызова и списка типов в объявлении по следующим правилам:
 *     Длина списка аргументов объявления (declArgs) совпадает с ожидаемой длиной (declArglistLen)
 *     первые argsForCmp элементов списка типов вызова (t) - точно совпадают с типами соответствующих аргументов из объявления.
 * @param t               список типов вызова
 * @param declArgs        список аргументов объявления
 * @param pl              вызов выполняется внутри кода pl/sql или в sql-конструкции
 * @param declArglistLen  ожидаемая длина списка аргументов объявления
 * @param argsForCmp      количество аргументов сначала для точного сравнения
 * @return @c true, если условия сравнения выполнены
 */
template <unsigned int argsForCmp>
inline bool eqByArgs(Datatype* /*t*/[], Arglist */*declArgs*/, bool /*pl*/);



template <>
inline bool eqByArgs<1>(Datatype* t[], Arglist *declArgs, bool pl) {
  Datatype *callT, *declT;
  return ((callT = t[0]) && (declT = (*declArgs)[0]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY);
}

template <>
inline bool eqByArgs<2>(Datatype* t[], Arglist *declArgs, bool pl) {
  Datatype *callT, *declT;
  return ((callT = t[0]) && (declT = (*declArgs)[0]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY) &&
         ((callT = t[1]) && (declT = (*declArgs)[1]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY);
}

template <>
inline bool eqByArgs<3>(Datatype* t[], Arglist *declArgs, bool pl) {
  Datatype *callT, *declT;
  return ((callT = t[0]) && (declT = (*declArgs)[0]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY) &&
         ((callT = t[1]) && (declT = (*declArgs)[1]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY) &&
         ((callT = t[2]) && (declT = (*declArgs)[2]->getDatatype()) && callT->isSubtype(declT, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY);
}


template <unsigned int declArglistLen, unsigned int argsForCmp>
inline Ptr<Function> findByCreatedExactlyArglist(Datatype* t[], bool pl, vector<Ptr<Function> > &createdFunctions) {
  for (Ptr<Function> &fun : createdFunctions) {
    Arglist *declArgs = fun->funArglist().object();
    if (declArgs && (declArgs->size() == declArglistLen) && eqByArgs<argsForCmp>(t, declArgs, pl))
      return fun;
  }
  return 0;
}



/*declArglistLen - ожидаемая длина списка аргументов объявления*/
/*argsForCmp     - аргументов для сравнения*/
template <unsigned int declArglistLen, unsigned int argsForCmp, unsigned int clSize>
struct ByExactlyArglist {
  static inline Ptr<Function> find(Datatype* t[], bool pl, TemplateArglist &callArgList, vector<Ptr<Function> > &createdFunctions);
};

template <unsigned int declArglistLen, unsigned int argsForCmp>
struct ByExactlyArglist<declArglistLen, argsForCmp, 1> {
  static inline Ptr<Function> find(Datatype* t[], bool pl, TemplateArglist &callArgList, vector<Ptr<Function> > &createdFunctions) {
    t[0] = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    return findByCreatedExactlyArglist<declArglistLen, argsForCmp>(t, pl, createdFunctions);
  }
};

template <unsigned int declArglistLen, unsigned int argsForCmp>
struct ByExactlyArglist<declArglistLen, argsForCmp, 2> {
  static inline Ptr<Function> find(Datatype* t[], bool pl, TemplateArglist &callArgList, vector<Ptr<Function> > &createdFunctions) {
    t[0] = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    t[1] = SubqueryUnwrapper::unwrap(callArgList[1]->getDatatype());
    return findByCreatedExactlyArglist<declArglistLen, argsForCmp>(t, pl, createdFunctions);
  }
};

template <unsigned int declArglistLen, unsigned int argsForCmp>
struct ByExactlyArglist<declArglistLen, argsForCmp, 3> {
  static inline Ptr<Function> find(Datatype* t[], bool pl, TemplateArglist &callArgList, vector<Ptr<Function> > &createdFunctions) {
    t[0] = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    t[1] = SubqueryUnwrapper::unwrap(callArgList[1]->getDatatype());
    t[2] = SubqueryUnwrapper::unwrap(callArgList[2]->getDatatype());
    return findByCreatedExactlyArglist<declArglistLen, argsForCmp>(t, pl, createdFunctions);
  }
};






class CreateDecodeFun : public OraTemplateData {
public:
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext);
  Ptr<CallarglistTranslator> tr;

  CreateDecodeFun(const string &funname) : OraTemplateData(funname) {
    tr = new CallarglistTranslatorSimple(CreateDecodeFun::trDecodeArgs);
  }
protected:
  Ptr<Function> findDecodeFunction(Datatype **signature, size_t arglistSize, bool isPlContext);
  static void trDecodeArgs(Ptr<Id> &call, Codestream &str);
};


/*
 * DECODE(expr, {search, result}+ [, default])
 *
 * Назначение:
 * DECODE последовательно сравнивает значения expr с каждым значением search. Если expr равен
 * search, то Oracle Database возвращает соответствующий результат. Если совпадения не найдены, то
 * Oracle возаращает default. Если default опущен - то Oracle возвращает null.
 * Аргументы могут быть любыми числовыми типами numeric (NUMBER, BINARY_FLOAT, или BINARY_DOUBLE), или символьными типами.
 *
 * Если expr и search являются символьными данными, то Oracle сравнивает их, используя
 * немягкую семантику сравнения. expr, search, и result могут иметь любые типы данных -
 * CHAR, VARCHAR2, NCHAR, или NVARCHAR2. Возвращаемая строка имеет тип данных
 * VARCHAR2 и ту же кодировку, как и первый параметр результата.
 *
 * Если первая пара search-result является numeric, то Oracle сравнивает все
 * выражения search-result и первое expr определяет аргумент с высочайшим числовым
 * приоритетом, неявно преобразуя оставшиеся аргументы в этот тип данныхб и
 * возвращает этот тип данных.
 *
 * Значения search, result, и default могут быть выведены из выражения. Oracle
 * Database использует схему короткого вычисления. СУБД вычисляет каждое искомое значение только перед
 * сравнением его с expr, и это приоритетнее чем вычисление всех исхомых значений перед
 * сравнением любого из них с expr. В итоге, Oracle никогда не вычисляет search,
 * если предыдущий search равен expr.
 *
 * Oracle автоматически преобразует expr и каждое значение search к типу данных первого значения search до сравнения.
 * Oracle автоматически преобразует возвращаемое значение к тому же типу данных, что и первый result.
 * Если первый result имеет тип данных - CHAR, или если первый result - есть null, то Oracle преобразует возвращаемое значение к
 * типу данных - VARCHAR2.
 * В функции DECODE, Oracle рассматривает 2 nulls как эквивалентные. Если expr равен null, то Oracle возвращает результат первого search, который также равен null.
 * Максимальное число компонентов в функции DECODE, включаяя expr, searches, results, и default, равно 255.
 */
Ptr<Function> CreateDecodeFun::findDecodeFunction(Datatype **signature, size_t arglistSize, bool isPlContext)
{
  for (vector<Ptr<Function> >::iterator it = createdFunctions.begin(); it != createdFunctions.end(); ++it) {
    {
      unsigned int defLstSize = (*it)->arglistSize();
      if (arglistSize % 2) { // нечетное число аргументов в вызове => вызов без default
        if (defLstSize != arglistSize && defLstSize != (arglistSize+1)) // количество с default
          continue;
      }
      else if (defLstSize != arglistSize) // четное число аргументов в вызове => вызов с default
        continue;
    }

    FunArgList &defLst = *(*it)->getArglist();
    Sm::IsSubtypeValues::IsSubtypeCathegory cast[3];
    for (int i = 0; i < 3; ++i) {
      if (Datatype *t = signature[i])
        cast[i] = t->isSubtype(defLst[i]->getDatatype().object(), isPlContext).val;
      else
        return 0;
    }

    if (cast[0] == Sm::IsSubtypeValues::EXACTLY_EQUALLY &&
        cast[1] == Sm::IsSubtypeValues::EXACTLY_EQUALLY &&
        cast[2] == Sm::IsSubtypeValues::EXACTLY_EQUALLY)
      return *it;
  }
  return 0;
}



Ptr<Function> CreateDecodeFun::constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
  (void)reference;

  if (reference->beginedFrom(584027))
    cout << "";

  if (callArgList.size() < 3)
    return 0;

  // 0 - декодируемый тип expr   - переменной выбора
  // 1 - декодируемый тип search - переменных, с которыми сравнивается переменная выбора (первое выражение)
  // 2 - возвращаемый тип

  TemplateArglist::iterator it = callArgList.begin();
  Datatype *exprT   = SubqueryUnwrapper::unwrap((*it)->getDatatype());
  Datatype *searchT = SubqueryUnwrapper::unwrap((*(++it))->getDatatype());

  exprT = searchT;

  reference->setResolvedFunction();
  Ptr<FunCallArg> arg3 = *(++it);
  Datatype *rettype = SubqueryUnwrapper::unwrap(arg3->getDatatype());
  if (!rettype || rettype->isNull() || arg3->expr()->isNumericValue() || arg3->expr()->isQuotedSqlExprId())
    if (Datatype *defaultT = SubqueryUnwrapper::unwrap(callArgList.back()->tryResolveDatatype()))
      if (!defaultT->isDefault() && defaultT->isResolved() && !defaultT->isNull() &&
          !(arg3->expr()->isQuotedSqlExprId() && defaultT->isNum()))
        rettype = defaultT;

  if ((!rettype || rettype->isNull()) && it != callArgList.end()) {
    // Тип не определен, пытаемся определить из списка остальных аргументов максимальный тип.
    Ptr<Datatype> maxType;
    for (++it; it != callArgList.end(); ++it, (it != callArgList.end()) ? ++it : it) {
      Ptr<Datatype> datatype = SubqueryUnwrapper::unwrap((*it)->getDatatype());
      if (datatype && !datatype->isNull())
        Datatype::getMaximal(maxType, maxType.object(), datatype.object(), isPlContext);
    }
    if (maxType)
      rettype = maxType;
  }

  if (!rettype || rettype->isNull())
    // Тип не определен, берем тип сравниваемого выражения. Это возможно неправильно.
    rettype = exprT;

  {
    Datatype* signature[3] = {exprT, searchT, rettype};
    Ptr<Function> f = findDecodeFunction(signature, callArgList.size(), isPlContext);
    if (f)
      return f;
  }

  Ptr<FunArgList> argList = new FunArgList;
  argList->push_back(new FunctionArgument(new Id("expr"), exprT));

  int endI = callArgList.size();
  if (!(endI % 2))  // Если есть default, то в уменьшить длину выходного списка
    --endI;
  endI /= 2; // количество аргументов без default, деленное на 2

  ++endI;
  for (int i = 1; i < endI; ++i) {
    argList->push_back(new FunctionArgument(new Id("select" + Sm::to_string(i)), searchT));
    argList->push_back(new FunctionArgument(new Id("result" + Sm::to_string(i)), rettype));
  }
  argList->push_back(new FunctionArgument(new Id(string("defaultValue")), rettype, new NullExpr));

  Ptr<Function> decodeFun = new Function(new Id2(string(funname)), argList, rettype);
  updateTemplateNamespace(decodeFun);
  decodeFun->callarglistTranslator = this->tr;
  return decodeFun;
}


void CreateDecodeFun::trDecodeArgs(Ptr<Id> &call, Codestream &str) {
  if (call->beginedFrom(584026))
    cout << "";
  Ptr<CallArgList> argl = call->callArglist;
  if (!argl || argl->size() < 3) {
    str << argl;
    return;
  }

  bool first = false;
  CallArgList::iterator it = argl->begin();
  str << s::comma(&first) << (*it++);
  str << s::comma(&first) << (*it++);

  Ptr<Datatype> resType = call->definition()->getDatatype();

  for (size_t i = 3; it != argl->end(); ++it, ++i) {
    Ptr<Datatype> curDatatype = ResolvedEntity::getLastConcreteDatatype((*it)->getDatatype());
    if (((i % 2) != 0 || i == argl->size()) && resType && curDatatype) {
      bool              isNum = curDatatype->isNum();
      CastCathegory     castCat = resType->getCastCathegory(curDatatype, str.isProc(), str.isSql());
      if (curDatatype->isNull() || castCat.explicitInReturn() ||
          (isNum && (castCat.castUnion() || ((*it)->expr()->isNumericValue() && !resType->isInt())))
         ) {
        str << s::comma(&first) << "CAST" << s::obracket << (*it) << " AS ";
        resType->linterDefinition(str);
        str << s::cbracket;
        continue;
      }
    }
    str << s::comma(&first) << (*it);
  }
}





bool isPlContext(TemplateArglist *callArgList) { return (*(callArgList->begin()))->getSemanticNode()->isPlContext(); }



class CreateGreatestLeastFun : public OraTemplateData {
public:
  typedef map<int, Datatype*> BadDatatypeArguments;

  class ConflSelector {
  public:
    BadDatatypeArguments badDatatypeArguments;
    Datatype *firstArgumentT;

    ConflSelector(Datatype *firstArgument);

    Datatype* operator() (int lhsPos, Datatype* lhs, int rhsPos, Datatype* rhs, bool pl);
  };

  class CallarglistTralslatorGreatestLeast : public CallarglistTranslator {
  public:
    enum FirstDatatypeCathegory { NUMERIC, VARCHAR  };
    BadDatatypeArguments badDatatypeArguments;
    FirstDatatypeCathegory cat;

    CallarglistTralslatorGreatestLeast(BadDatatypeArguments &args, Datatype *firstDatatypeCat);
    void operator()(smart::Ptr<Id> &call, Codestream &str);
  };

  CreateGreatestLeastFun(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &/*reference*/, TemplateArglist &callArgList, bool isPlContext);
};


/*
- GREATEST возвращает наибольшее значение из списка одного или нескольких
выражений.

- LEAST возвращает наименьшее значение из списка одного или нескольких
выражений.

Oracle использует первый expr, для определения возвращаемого типа.
Если первое expr является числовым, то Oracle определяет аргумент с наивысшим
числовым старшинством (?типа), неявно преобразует остальные аргументы к этому
типу данных перед сравнением, и возвращает этот тип данных.  Если первое
выражение не является числом, то каждое expr после первого неявно преобразуется
до сравнения в тип данных первого expr.

Oracle сравнивает каждое expr, используя немягкую семантику сравнения.
Сравнение данных - двоичное по умолчанию и лингвистическое если параметр
NLS_COMP установлен в LINGUISTIC и параметр NLS_SORT имеет значение, отличное
от BINARY.  Символьное сравнение основано на числовых кодах символов в наборе
символов базы данных и осуществляется на цельных строках, рассматриваемых как
последовательность байтов, а не посимвольно.  Если значение, возвращенное этой
функцией является символьными данными, то ее тип данных - VARCHAR2 если первое
выражение имеет символьный тип данных, и и NVARCHAR2 если первое выражение
имеет национальный символьный тип данных.
*/
Ptr<Function> CreateGreatestLeastFun::constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
  if (callArgList.empty())
    return 0;

  CallArgList::value_type front = callArgList.front();
  Ptr<Datatype> frontDatatype = front->getDatatype();
  if (!frontDatatype)
    return 0;

  ConflSelector selector(frontDatatype);

  reference->setResolvedFunction();
  Ptr<Datatype> t = getMaximalArgumentDatatype(callArgList, isPlContext, selector);
  if (t->isNumberDatatype())
    t = Datatype::mkDouble(); // Linter возвращает double тип
  if (Ptr<Function> f = findCreatedFunByRettype(t, isPlContext, callArgList.size()))
    return f;

  Ptr<CallarglistTranslator> tr;
  if (!selector.badDatatypeArguments.empty())
    tr = new CallarglistTralslatorGreatestLeast(selector.badDatatypeArguments, selector.firstArgumentT);

  Ptr<FunArgList> argList = new FunArgList;
  char buf[64];
  for (unsigned int i = 0; i < callArgList.size(); ++i) {
    sprintf(buf, "expr%u", i+1);
    argList->push_back(new FunctionArgument(new Id(string(buf)), t));
  }

  return addSysFun(new Function(new Id2(string(funname)), argList, t), tr);
}


class CreateNullif : public OraTemplateData {
public:
  CreateNullif(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (!callArglistLenNEQ(callArgList, 2))
      if (Ptr<Function> f = createStandardSysFunction(callArgList, isPlContext, {"expr1", "expr2"})) {
        reference->setResolvedFunction();
        return f;
      }
    return 0;
  }
};

void trBitandArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  if (str.isProc()) {
    str << s::obracket << *(args->begin()) << s::cbracket;
    str << " & ";
    str << s::obracket << *(++args->begin()) << s::cbracket;
  }
  else {
    Ptr<FunCallArg> arg1 = (*args->begin());
    Ptr<Datatype> arg1Type = arg1->getDatatype();
    if (!arg1Type->isSmallint() && !arg1Type->isInt() && !arg1Type->isBigint())
      str << "CAST" << s::obracket << arg1 << " AS INT" << s::cbracket
          << s::comma() << (*++args->begin());
    else
      str << args;
  }
}

void trBitandName(Codestream &str, Ptr<CallArgList> /*args*/) {
  if (str.isProc())
    str << "";
  else
    str << "BITAND";
}

class CreateBitand : public OraTemplateData {
  Ptr<CallarglistTranslator> trArgs;
public:
  CreateBitand(const string &funname) : OraTemplateData(funname), trArgs(new CallarglistTranslatorSimple(trBitandArgs)) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 2))
      return 0;

    reference->setResolvedFunction();
    Datatype* t[2] = {
      SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype()),
      SubqueryUnwrapper::unwrap(callArgList[1]->getDatatype())
    };
    Ptr<Datatype> rettype;
    if (t[0]->isNull() || t[1]->isNull())
      rettype = Datatype::mkNull();
    else if (isAdjustNum(callArgList)) {
      Ptr<CallarglistTranslator> tr;
      rettype = getImplicitCastedOracleRettype(t[0], t[1], isPlContext, tr);
      if (tr)
        throw 999;
    }
    else
      throw 999;

    Ptr<Function> f;
    if (f = findCreatedFunByRettype(rettype, isPlContext, callArgList.size()))
      return f;
    f = addSysFun(new Function(string(funname), {Arg("expr1", rettype), Arg("expr2", rettype)}, rettype));
    f->callarglistTranslator = trArgs;
    f->nameTranslator = &trBitandName;
    return f;
  }
};


class CreateNumericTemplate : public OraTemplateData {
public:
  CreateNumericTemplate(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 1))
      return 0;
    if (!isAdjustNum(callArgList, 0))
      throw 999;
    if (Ptr<Function> f = createStandardSysFunction(callArgList, isPlContext, {"expr"})) {
      reference->setResolvedFunction();
      return f;
    }
    return 0;
  }
};

class CreateSum : public OraTemplateData {
  Ptr<CallarglistTranslator> tr;
public:
  CreateSum(const string &funname) : OraTemplateData(funname), tr(new CallarglistTranslatorSimple(trArgsToNumber)) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    (void)reference;
    if (reference->beginedFrom(94173))
      cout << "";
    if (callArglistLenNEQ(callArgList, 1))
      return 0;
    if (!isAdjustNum(callArgList, 0) && !isAdjustDate(callArgList, 0)) {
      if (isAdjustVarchar(callArgList) || isAdjustChar(callArgList)) {
        Ptr<Function> f = createStandardSysFunction(Datatype::mkNumber(), 1, isPlContext, {"expr"});
        f->callarglistTranslator = tr;
        reference->setResolvedFunction();
        return f;
      }
      throw 999;
    }
    if (Ptr<Function> f = createStandardSysFunction(callArgList, isPlContext, {"expr"})) {
      reference->setResolvedFunction();
      return f;
    }
    return 0;
  }
};

class CreateUpperLower : public OraTemplateData {
  Ptr<CallarglistTranslator> tr;
public:
  CreateUpperLower(const string &funname) : OraTemplateData(funname), tr(new CallarglistTranslatorSimple(trArgToChar)) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    (void)reference;

    if (callArglistLenNEQ(callArgList, 1))
      return 0;
    bool convertToChar = false;
    if (!isAdjustVarchar(callArgList, 0) && !(convertToChar = isAdjustNum(callArgList, 0))) {
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "error: upper/lower call arglist arguments has not VARCHAR or NUMBER datatype: " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }

    Ptr<Datatype> rettype, argtype;
    argtype = callArgList[0]->getDatatype();
    if (convertToChar)
      rettype = Datatype::mkVarchar2(36);
    else
      rettype = argtype;
    reference->setResolvedFunction();
    Datatype* t[1] = {argtype};
    if (Ptr<Function> f = ByExactlyArglist<1, 1, 1>::find(t, isPlContext, callArgList, createdFunctions))
      return f;
    else
      return addSysFun(new Function(string(funname), { Arg("expr" , argtype) }, rettype), convertToChar ? tr : nullptr);
  }
};


class CreateTrimTemplate : public OraTemplateData {
  Ptr<CallarglistTranslator> firstArg2Char;
public:
  CreateTrimTemplate(const string &funname)
    : OraTemplateData(funname),
      firstArg2Char(new CallarglistTranslatorSimple(firstArgToChar)) {}

  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 1))
      return 0;
    bool argToChar  = isAdjustNum(callArgList, 0) || isAdjustDate(callArgList, 0) || isAdjustClob(callArgList, 0);

    if (!argToChar && !isAdjustVarchar(callArgList, 0) && !isAdjustClob(callArgList, 0)) {
      cout << "error: bad or unresolved trim argument: " << callArgList[0]->getLLoc().locText();
      return 0;
    }

    reference->setResolvedFunction();
    Datatype *rettype = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    if (argToChar)
      rettype = Datatype::mkVarchar2(126);

    Datatype* t[1] = {0};
    if (Ptr<Function> f = ByExactlyArglist</*declArglistLen=*/1,1,1>::find(t, isPlContext, callArgList, createdFunctions))
      return f;
    else
      return addSysFun(new Function(string(funname), { Arg("expr" , rettype) }, rettype), argToChar ? firstArg2Char : nullptr);
  }
};




class NvlTemplate : public OraTemplateData {
public:
  NvlTemplate(const string &funname) : OraTemplateData(funname) {}

  int argCat(TemplateArglist &callArgList, int p = 0) {
    if (isAdjustVarchar(callArgList, p) || isAdjustChar(callArgList, p))
      return 0;
    else if (isAdjustNum(callArgList, p))
      return 1;
    else if (isAdjustDate(callArgList, p))
      return 2;
    else if (isAdjustBool(callArgList, p))
      return 3;
    else if (isAdjustNull(callArgList, p))
      return 4;
    else if (isAdjustAnydata(callArgList, p))
      return 5;
    else if (isAdjustBlob(callArgList, p))
      return 6;
    else if (isAdjustClob(callArgList, p))
      return 7;
    else
      return -1;
  }

  bool badArglist(TemplateArglist &callArgList, int p = 0) {
    static const int stateTable[8][8] =
    //              varchar num date bool null anydata
    /* varchar */{ {   1,    1,  1,   0,   1,   0,   0,   0   },
    /* num     */  {   1,    1,  0,   0,   1,   0,   0,   0   },
    /* date    */  {   1,    0,  1,   0,   1,   0,   0,   0   },
    /* bool    */  {   0,    0,  0,   1,   1,   0,   0,   0   },
    /* null    */  {   1,    1,  1,   1,   1,   1,   0,   0   },
    /* anydata */  {   0,    0,  0,   0,   1,   1,   0,   0   },
    /* blob */     {   0,    0,  0,   0,   1,   0,   1,   0   },
    /* clob */     {   0,    0,  0,   0,   1,   0,   0,   1   }};
    int t1 = argCat(callArgList, p);
    int t2 = argCat(callArgList, p+1);
    return t1 < 0 || t2 < 0 || stateTable[t1][t2] == 0;
  }
};



class CreateNvl : public NvlTemplate {
public:
  CreateNvl(const string &funname) : NvlTemplate(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (reference->beginedFrom(161248,73))
      cout << "";
    if (callArglistLenNEQ(callArgList, 2))
      return Ptr<Function>(0);

    Datatype* t[2] = {0, 0};
    Ptr<Function> fun = ByExactlyArglist<2, 2, 2>::find(t, isPlContext, callArgList, createdFunctions);
    if (badArglist(callArgList, 0)) {
      // badArglist(callArgList, 0);
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "error: nvl bad arglist: " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }

    reference->setResolvedFunction();

    if (fun)
      return fun;
    Ptr<Sm::CallarglistTranslator> tr;
    Ptr<Datatype> rettype = getImplicitCastedOracleRettype(t[0], t[1], isPlContext, tr);
    return addSysFun(new Function(funname, {Arg("expr1", t[0]), Arg("expr2", t[1])}, rettype), tr);
  }
};


class CreateNvl2 : public NvlTemplate {
public:
  CreateNvl2(const string &funname) : NvlTemplate(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 3))
      return Ptr<Function>(0);

    Datatype* t[3] = {0, 0, 0};
    Ptr<Function> fun = ByExactlyArglist<3, 3, 3>::find(t, isPlContext, callArgList, createdFunctions);
    if (badArglist(callArgList, 1))
      throw 999;

    auto argNullCast = [&isPlContext](Ptr<FunCallArg> arg, Ptr<Datatype> castToType) {
      CastCathegory v;
      if (isPlContext)
        v.setProcCastState();
      else
        v.setSqlCastState();
      Ptr<PlExpr> newExpr = CommonDatatypeCast::cast(arg->expr(), arg->getDatatype(), castToType, v);
      if (newExpr != arg->expr())
        arg->setExpr(newExpr);
    };

    reference->setResolvedFunction();

    if (t[1]->isNull())
      argNullCast(callArgList[1], t[2]);
    else if (t[2]->isNull())
      argNullCast(callArgList[2], t[1]);

    if (fun)
      return fun;
    Ptr<Sm::CallarglistTranslator> tr;
    Ptr<Datatype> rettype = getImplicitCastedOracleRettype(t[1], t[2], isPlContext, tr);
    return addSysFun(new Function(funname, { Arg("nullcond", t[0]), Arg("expr2", t[1]), Arg("expr3", t[2]) }, rettype), tr);
  }
};


class CreateMinMax : public OraTemplateData {
public:
  CreateMinMax(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArglist, bool isPlContext) {
    if (reference->beginedFrom(23695, 29))
      cout << "";
    if (callArglistLenZGT(callArglist, 2)) // == 0 || > 2
      return Ptr<Function>(0);

    unsigned int len = callArglist.size();

    if (!isAdjustDate(callArglist) && !isAdjustNum(callArglist) && !isAdjustVarchar(callArglist))
      throw 999;
    // NUM, DATE, VARCHAR

    reference->setResolvedFunction();
    Datatype* t[2] = {0, 0};
    {
      Ptr<Function> fun;
      if ((len == 1 && (fun = ByExactlyArglist<1, 1, 1>::find(t, isPlContext, callArglist, createdFunctions)))  ||
          (len == 2 && (fun = ByExactlyArglist<2, 2, 2>::find(t, isPlContext, callArglist, createdFunctions)))
         )
        return fun;
    }


    Ptr<CallarglistTranslator> tr;
    if (len == 1)
      return addSysFun(new Function(funname, {Arg("expr", t[0])}, t[0]));
    else {
      Ptr<Function> f = addSysFun(new Function(funname, {Arg("expr1", t[0]), Arg("expr2", t[1])}, getImplicitCastedOracleRettype(t[0], t[1], isPlContext, tr)));
      if (tr)
        throw 999;
      return f;
    }
  }
};


class CreateLtrimRtrim : public OraTemplateData {
public:
  CreateLtrimRtrim(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    if (callArglistLenZGT(callArgList, 2)) // == 0 || > 2
      return 0;

    Datatype* t[2] = {
      SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype()),
      callArgList.size() > 1 ? SubqueryUnwrapper::unwrap(callArgList[1]->getDatatype()) : 0
    };
    t[1] = (!t[1] ? t[0] : t[1])->getNextDefinition()->getDatatype();

    reference->setResolvedFunction();
    if (Ptr<Function> fun = findByCreatedExactlyArglist</*ожидаемая длина списка аргументов объявления*/ 2, /*аргументов для сравнения*/ 2>(t, pl, createdFunctions))
      return fun;

    if (!isAdjustVarchar(callArgList) && !isAdjustChar(callArgList)) {
      cl::filelocation l = callArgList[0]->getLLoc();
      if (callArgList.size() > 1)
        l.loc = l.loc + callArgList[1]->getLLoc().loc;

      cout << "error: ltrim/rtrim call arglist arguments has not VARCHAR datatype: " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }

    return addSysFun(new Function(funname, { Arg("str", t[0]), Arg("str1", t[1], new NullExpr()) }, t[0]));
  }
};


class CreateConnectByRoot : public OraTemplateData {
public:
  CreateConnectByRoot(const string &funname) : OraTemplateData(funname) {}

  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    if (callArglistLenNEQ(callArgList, 1)) // != 1
      return Ptr<Function>(0);
    reference->setResolvedFunction();
    Ptr<Datatype> rettype = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    if (Ptr<Function> f = findCreatedFunByRettype(rettype, pl, /*ожидаемая длина списка аргументов объявления*/ 1))
      return f;

    return addSysFun(new Function(funname, {Arg("expr", rettype)}, rettype));
  }
};

class CreateToNumber      : public OraTemplateData {
public:
  Ptr<Function> toNumberVarchar;
  Ptr<Function> toNumberNum;

  Arg fmt;
  Arg nls;

  CreateToNumber(const string &funname, GlobalFunctions *globalFuns)
    : OraTemplateData(funname),
      fmt("fmt"     , Datatype::mkVarchar2(), SqlExpr::literal("")),
      nls("nlsparam", Datatype::mkVarchar2(), SqlExpr::literal(""))
  {
    toNumberVarchar = globalFuns->add(new Sm::Function("TO_NUMBER", { Arg("value", Datatype::mkVarchar2()), fmt, nls }, Datatype::mkNumber(), true));
    toNumberVarchar->nameTranslator = trToNumber;
    toNumberVarchar->callarglistTranslator = new CallarglistTranslatorSimple(trToNumberArgs);
    toNumberVarchar->setElementaryLinFunc();

    toNumberNum = globalFuns->add(new Sm::Function("TO_NUMBER", { Arg("value", Datatype::mkNumber()), fmt, nls }, Datatype::mkNumber(), true));
    toNumberNum->nameTranslator = trToNumberNum;
    toNumberNum->callarglistTranslator = new CallarglistTranslatorSimple(trToNumberNumArgs);
    toNumberNum->setElementaryLinFunc();
  }
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    (void)reference;
    if (callArglistLenZGT(callArgList, 3)) // == 0 || > 3
      return Ptr<Function>(0);

    reference->setResolvedFunction();
    Ptr<Datatype> rettype = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    if (rettype->isCharVarchar() || rettype->isNull())
      return toNumberVarchar;

    if (!rettype->isNum()) {
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "error: to_number call arglist is not VARCHAR or NUMBER datatype: " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }

    bool isNum      = false;
    bool isBigint   = false;
    bool isInt      = false;
    bool isSmallint = false;
    if (!(isNum      = rettype->isNumberDatatype()) &&
        !(isBigint   = rettype->isBigint()) &&
        !(isInt      = rettype->isInt()) &&
        !(isSmallint = rettype->isSmallint()))
      return toNumberNum;

    if (isNum && ((!rettype->scaleIsNotSet() && rettype->scale) || !rettype->precision))
      return toNumberNum;
    else if (isBigint)
      rettype = Datatype::mkNumber(12);
    else if (isInt)
      rettype = Datatype::mkNumber(10);
    else if (isSmallint)
      rettype = Datatype::mkNumber(5);

    if (Ptr<Function> f = findCreatedFunByRettype(rettype, pl, /*ожидаемая длина списка аргументов объявления*/ 3))
      return f;

    Ptr<Function> f = addSysFun(new Function(funname, {Arg("expr", rettype), fmt, nls}, rettype));
    f->nameTranslator = trToNumberNum;
    f->callarglistTranslator = new CallarglistTranslatorSimple(trToNumberNumArgs);
    f->setElementaryLinFunc();
    return f;
  }

};


class CreateToChar      : public OraTemplateData {
public:
  Ptr<Function> toCharNumber;
  Ptr<Function> toCharDate;
  Ptr<Function> toCharIntervalDTS;
  Ptr<Function> toCharIntervalYTM;

  Arg fmt;
  Arg nls;

  Ptr<Function> setFlags(Ptr<Function> fun) {
    fun->flags.setSysToChar();
    return fun;
  }

  CreateToChar(const string &funname, GlobalFunctions *globalFuns)
    : OraTemplateData(funname),
      fmt("fmt"     , Datatype::mkVarchar2(), SqlExpr::literal("")),
      nls("nlsparam", Datatype::mkVarchar2(), SqlExpr::literal(""))
  {
    toCharNumber      = globalFuns->add(setFlags(new Sm::Function("TO_CHAR", { Arg("value", Datatype::mkNumber()             ), fmt, nls }, Datatype::mkVarchar2(126))));
    toCharNumber->setElementaryLinFunc();
    toCharDate        = globalFuns->add(setFlags(new Sm::Function("TO_CHAR", { Arg("value", Datatype::mkDate()               ), fmt, nls }, Datatype::mkVarchar2(syntaxerContext.model->modelActions.dateToCharDefaultLength))));
    toCharDate->setElementaryLinFunc();
    toCharIntervalDTS = globalFuns->add(setFlags(new Sm::Function("TO_CHAR", { Arg("value", Datatype::mkIntervalDayToSecond()), fmt, nls }, Datatype::mkVarchar2(syntaxerContext.model->modelActions.dateToCharDefaultLength))));
    toCharIntervalYTM = globalFuns->add(setFlags(new Sm::Function("TO_CHAR", { Arg("value", Datatype::mkIntervalYearToMonth()), fmt, nls }, Datatype::mkVarchar2(syntaxerContext.model->modelActions.dateToCharDefaultLength))));
  }

  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    (void)reference;
    if (callArglistLenZGT(callArgList, 3)) // == 0 || > 3
      return Ptr<Function>(0);

    unsigned int clLen = callArgList.size();
    if ((clLen > 1 && !isAdjustCharVarchar(callArgList, 1)) || (clLen > 2 && !isAdjustCharVarchar(callArgList, 2)))
      throw 999;

    Datatype *rettype = SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    if (!rettype) {
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "unresolved to_char argument " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }
    reference->setResolvedFunction();

    if (rettype->isDateDatatype())
      return toCharDate;
    else if (rettype->isNum())
      return toCharNumber;

    if (rettype->isNull()) {
      rettype = Datatype::mkVarchar2(std::numeric_limits<uint16_t>::max());
      rettype->precision = 0;
      rettype->setEmptyVarchar();
    }
    if (!rettype->isCharVarchar()) {
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "error: to_char unsupported rettype: " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }


    Datatype *t[2] = { rettype, Datatype::mkVarchar2() };
    if (t[0]->isNull())
      throw 999;

    Ptr<Function> f;
    if ((f = findCreatedFunByRettype(rettype, pl, /*ожидаемая длина списка аргументов объявления*/ 3)).valid())
      return f;

    f = addSysFun(setFlags(new Function(funname, {Arg("expr", t[0]), fmt, nls}, t[0])));
    f->nameTranslator = trToChar;
    f->callarglistTranslator = new CallarglistTranslatorSimple(trToCharArgs);
    return f;
  }
};



class CreateTrunc       : public OraTemplateData {
public:
  Ptr<CallarglistTranslator> trDateArgs;
  Ptr<CallarglistTranslator> trToNumber;
  Ptr<CallarglistTranslator> trNumArgs;

  Ptr<Function> truncDate;
  CreateTrunc(const string &funname, GlobalFunctions *globalFuns)
    : OraTemplateData(funname),
      trDateArgs(new CallarglistTranslatorSimple(trTruncDate      )),
      trToNumber(new CallarglistTranslatorSimple(trTruncToNumber  )),
      trNumArgs (new CallarglistTranslatorSimple(trTruncNumberArgs))
  {
    // TRUNC(value Date, precision String := 'D') return Date
    truncDate = globalFuns->add(new Sm::Function("TRUNC", { Arg("value", Datatype::mkDate()), Arg("precision", Datatype::mkVarchar2(), SqlExpr::literal("D"))}, Datatype::mkDate()));
    truncDate->callarglistTranslator = trDateArgs;
    truncDate->nameTranslator = trTruncName;
    truncDate->flags.setElementaryLinterFunction();
  }
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    if (callArglistLenZGT(callArgList, 2)) // == 0 || > 2
      return Ptr<Function>(0);

    reference->setResolvedFunction();
    if (isAdjustDate(callArgList, 0))
      return truncDate;

    unsigned int clLen = callArgList.size();

    Datatype* t[2] = {0, 0};
    {
      Ptr<Function> fun;
      if (((clLen == 1) && (fun = ByExactlyArglist<1, 1, 1>::find(t, pl, callArgList, createdFunctions))) ||
          ((clLen == 2) && (fun = ByExactlyArglist<2, 2, 2>::find(t, pl, callArgList, createdFunctions)))
         )
        return fun;
    }

    Ptr<CallarglistTranslator> tr = (clLen == 1) ? trNumArgs : nullptr;
    if (!isAdjustNum(callArgList)) {
      if (!t[0]) {
        cout << "error: unresolved trunc argument: " << callArgList[0]->getLLoc().locText();
        return 0;
      }
      if (t[0]->isNull())
        t[0] = Datatype::mkNumber(4);
      else if (t[0]->isCharVarchar())
        tr = trToNumber;
      else
        throw 999;
    }

    Ptr<Function> f;

    static const auto clearScale = [](Datatype **t) {
      if ((*t)->isNumberDatatype()) {
        if (!(*t)->precision)
          *t = Datatype::mkNumber(12);
        else if ((*t)->scale)
          *t = Datatype::mkNumber((*t)->precision);
      }
      else if ((*t)->isReal() || (*t)->isDouble())
        *t = Datatype::mkNumber(12);

    };


    if (clLen == 1) {
      clearScale(&(t[0]));
      Datatype *resT = (t[0]->isCharVarchar()) ? Datatype::mkNumber() : t[0];
      return addSysFun(new Function(funname, {Arg("expr", t[0])}, resT), tr); // arg2 = 0
    }
    else {
      if (long int v = callArgList[1]->expr()->getSIntValue())
        if (v != numeric_limits<long int>::min() && v <= 0)
          clearScale(&(t[0]));
      return addSysFun(new Function(funname, {Arg("expr", t[0]), Arg("fmt", t[1])}, Datatype::mkNumber()), tr);
    }
    return f;
  }
};


class CreateSubstr : public OraTemplateData {

public:
  Ptr<CallarglistTranslator> trArgs;
  Ptr<CallarglistTranslator> trClobArgs;
  Ptr<CallarglistTranslator> trToChar;

  CreateSubstr *toSelfCreateSubstr() { return this; }

  CreateSubstr(const string &funname)
    : OraTemplateData(funname),
      trArgs(    new CallarglistTranslatorSimple(trSubstrArgs)),
      trClobArgs(new CallarglistTranslatorSimple(trSubstrClobArgs)),
      trToChar(  new CallarglistTranslatorSimple(trSubstrToChar))  {}


  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    if (callArglistLenGTLT(callArgList, 3, 2)) // > 3 || < 2
      return Ptr<Function>(0);

    Datatype* t =  SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());

    if (!t) {
      cl::filelocation l = callArgList[0]->getLLoc();
      cout << "unresolved substr argument datatype : " << l << ": " << SmartLexer::textFromFile(l) << endl;
      return 0;
    }

    bool isVarcharDatatype = t->isVarcharDatatype() || t->isCharDatatype();
    bool isClobDatatype    = t->isClobDatatype();
    bool toCharConvert     = isAdjustNum(callArgList, 0) || t->isDateDatatype();

    if (!isVarcharDatatype && !isClobDatatype && !toCharConvert)
      throw 999;

    reference->setResolvedFunction();
    Ptr<Datatype> rettype = makeSubstrRettypeByFirstArg(t, callArgList, callArgList.size());

    if (t)
      t = t->getNextDefinition()->getDatatype();

    for (Ptr<Function> &fun : createdFunctions)
      if (fun->arglistSize() == 3)
        if (rettype->isSubtype(fun->getDatatype().object(), pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY &&
            (*(fun->arglistBegin()))->getDatatype()->isSubtype(t, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY)
          return fun;

    Ptr<Function> f = addSysFun(
        new Function(funname, {
          Arg("str"             , t),
          Arg("position"        , Datatype::mkInteger()),
          Arg("substring_length", Datatype::mkInteger(), new NumericSimpleInt(1))
        }, rettype));

    if (isVarcharDatatype)
      f->callarglistTranslator = trArgs;
    else if (isClobDatatype)
      f->callarglistTranslator = trClobArgs;
    else if (toCharConvert )
      f->callarglistTranslator = trToChar;
    return f;
  }
};


class CreateRegexpSubstr : public OraTemplateData {
public:
  Ptr<CallarglistTranslator> trArgs;

  CreateRegexpSubstr(const string &funname)
    : OraTemplateData(funname),
      trArgs(new CallarglistTranslatorSimple(trRegexpSubstrArgs)) {}

  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool pl) {
    if (callArglistLenGTLT(callArgList, 5, 2)) // > 3 || < 2
      return Ptr<Function>(0);

    Datatype* t =  SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype());
    bool isVarcharDatatype = t->isVarcharDatatype() || t->isCharDatatype();

    reference->setResolvedFunction();
    Ptr<Datatype> rettype = makeSubstrRettypeByFirstArg(t, callArgList, callArgList.size());
    if (!isVarcharDatatype)
      t = rettype.object();

    for (Ptr<Function> &fun : createdFunctions)
      if (fun->arglistSize() == 5)
        if (rettype->isSubtype(fun->getDatatype().object(), pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY &&
            (*(fun->arglistBegin()))->getDatatype()->isSubtype(t, pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY)
          return fun;

    Ptr<Function> f = addSysFun(
      new Function(funname, {
        Arg("source_string"   , t),
        Arg("pattern"         , Datatype::mkVarchar2()),
        Arg("position"        , Datatype::mkInteger() , new NumericSimpleInt(1)),
        Arg("occurence"       , Datatype::mkInteger() , new NumericSimpleInt(1)),
        Arg("math_prameter"   , Datatype::mkVarchar2(), SqlExpr::literal("")),
      }, rettype));

    f->callarglistTranslator = trArgs;
    return f;
  }

};


Ptr<Function> GlobalFunctions::getSubstr(bool pl, unsigned int len) {
  GlobalFunctions::OraTemplatesFunMap::iterator it = oraTemplatesFunMap.find("SUBSTR");
  Ptr<Datatype> rettype = Datatype::mkVarchar2(len);
  if (it != oraTemplatesFunMap.end()) {
    for (Ptr<Function> &fun : it->second->createdFunctions)
      if (fun->arglistSize() == 3)
        if (rettype->isSubtype(fun->getDatatype().object(), pl) == Sm::IsSubtypeValues::EXACTLY_EQUALLY) {
          switch ((*(fun->arglistBegin()))->getDatatype()->isSubtype(Datatype::mkVarchar2(), pl).val) {
            case Sm::IsSubtypeValues::EXACTLY_EQUALLY:
            case Sm::IsSubtypeValues::IMPLICIT_CAST_HIGH:
            case Sm::IsSubtypeValues::IMPLICIT_CAST_BY_FIELDS:
              return fun;
            default:
              break;
          }
          return fun;
        }

    Ptr<Function> f = it->second->addSysFun(
       new Function(string("SUBSTR"), {
         Arg("str" , Datatype::mkVarchar2()),
         Arg("position"        , Datatype::mkInteger()),
         Arg("substring_length", Datatype::mkInteger(), new NumericSimpleInt(1))
       }, rettype));

    if (CreateSubstr *substr = it->second->toSelfCreateSubstr())
      f->callarglistTranslator = substr->trArgs;
    else
      throw 999;
    return f;
  }
  throw 999;
  return 0;
}


class CreateNlsUpperInitcap : public OraTemplateData {
public:
  CreateNlsUpperInitcap(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenZGT(callArgList, 2)) // == 0 || > 2
      return Ptr<Function>(0);

    if (!isAdjustVarchar(callArgList) && !isAdjustChar(callArgList))
      throw 999;

    reference->setResolvedFunction();
    Datatype* t[2] = {0, 0};
    {
      unsigned int l = callArgList.size();
      Ptr<Function> fun;
      if (((l == 1) && (fun = ByExactlyArglist<2, 1, 1>::find(t, isPlContext, callArgList, createdFunctions))) &&
          ((l == 2) && (fun = ByExactlyArglist<2, 1, 2>::find(t, isPlContext, callArgList, createdFunctions)))
         )
        return fun;
    }

    Ptr<Function> f = addSysFun(new Function(funname, {Arg("str", t[0]), Arg("nlsparam", t[1], new NullExpr()) }, t[0]));
    f->setElementaryLinFunc();
    if (funname == "NLS_INITCAP")
      f->translatedName("INITCAP");
    else
      f->translatedName("UPPER");
    return f;
  }
};

void trConcatArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  if (str.isProc()) {
    CallArgList::iterator it = args->begin();
    str << (*it);
    for (++it; it != args->end(); ++it)
      str << s::name << "+" << s::name << (*it);
  }
  else
    str << args;
}

void trConcatName(Codestream &str, Ptr<CallArgList> /*args*/) {
  if (str.isProc())
    str << "";
  else
    str << "CONCAT";
}

class CreateConcat : public OraTemplateData {
  Ptr<CallarglistTranslator> trArgs;
public:
  CreateConcat(const string &funname) : OraTemplateData(funname), trArgs(new CallarglistTranslatorSimple(trConcatArgs)) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 2)) // != 2
      return Ptr<Function>(0);

    reference->setResolvedFunction();
    Datatype* t[2] = {0, 0};
    if (Ptr<Function> fun = ByExactlyArglist<2, 2, 2>::find(t, isPlContext, callArgList, createdFunctions))
      return fun;

    if (!isAdjustVarchar(callArgList) && !isAdjustChar(callArgList))
      throw 999;

    uint16_t length = 0;
    if (t[0]->precision && t[1]->precision) {
      length = t[0]->precision + t[1]->precision;
      if (length >= 4000)
        length = 0;
    }

    Ptr<Datatype> rettype;
    if (t[0]->isVarcharDatatype() || t[1]->isVarcharDatatype())
      rettype = length ? Datatype::mkVarchar2(length) : Datatype::mkVarchar2();
    else
      rettype = length ? Datatype::mkChar(length) : Datatype::mkChar();

    Ptr<Function> f = addSysFun(new Function(funname, {Arg("str1", t[0]), Arg("str2", t[1])}, rettype));
    f->callarglistTranslator = trArgs;
    f->nameTranslator = trConcatName;
    return f;
  }
};


class CreateLRpad : public OraTemplateData {
public:
  Ptr<CallarglistTranslator> tr;
  CreateLRpad(const string &funname)
    : OraTemplateData(funname),
      tr(new CallarglistTranslatorSimple(firstArgToChar)){}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (reference->beginedFrom(199827))
      cout << "";
    if (callArglistLenGTLT(callArgList, 3, 2)) // > 3 || < 2
      return Ptr<Function>(0);

    reference->setResolvedFunction();
    Datatype* t[3] = {
      SubqueryUnwrapper::unwrap(callArgList[0]->getDatatype()),
      SubqueryUnwrapper::unwrap(callArgList[1]->getDatatype()),
      Datatype::mkVarchar2(1),
    };

    bool isVarchar = t[0]->isCharVarchar();
    bool isClob    = t[0]->isClobDatatype();
    bool isNum     = t[0]->isNum();

    if (!isVarchar && !isClob && !isNum)
      throw 999;

    Ptr<Datatype> rettype;
    if (Ptr<PlExpr> n = callArgList[1]->expr()) {
      if (long int nVal = n->getSIntValue())
        if (nVal > 0 && nVal < 4000)
          rettype = Datatype::mkVarchar2(nVal);
    }
    else if (!t[1]->isNum())
      throw 999;


    if (!rettype)
      rettype = (isVarchar || isNum) ? Datatype::mkVarchar2() : (isClob ? Datatype::mkClob() : nullptr);
    if (!rettype)
      throw 999;

    {
      unsigned int l = callArgList.size();
      Ptr<Function> f;
      if (((l == 2) && (f = ByExactlyArglist<3, 2, 2>::find(t, isPlContext, callArgList, createdFunctions))) ||
          ((l == 3) && (f = ByExactlyArglist<3, 2, 3>::find(t, isPlContext, callArgList, createdFunctions)))
         )
        return f;
    }

    return addSysFun(new Sm::Function(string(funname), { Arg("expr1", t[0]), Arg("n", Datatype::mkInteger()), Arg("expr2", t[2], SqlExpr::literal(" ")) }, rettype),
                     isNum ? tr : nullptr
                     );
  }
};

Sm::Function* GlobalFunctions::add(Ptr<Function> fn) {
  fn->setIsSystem();
  fn->setElementaryLinFunc();
  SemanticTree *node = fn->toSTree();
  semanticNodes.push_back(node);
  // collectEqualsDeclarationOnNode(node, syntaxerContext.model->levelNamespace);
  systemFunctionsMap[fn->getName()->toNormalizedString()].push_back(fn);
  return fn.object();
}


//Sm::Function* GlobalFunctions::addNvl(Ptr<Datatype> t) { return add(new Function("NVL", { Arg("val1", t), Arg("val2", t) }, t)); }
//void GlobalFunctions::addNvl2(Ptr<Datatype> t) { add(new Function("NVL2", { Arg("val1", t), Arg("val2", t), Arg("val3", t) }, t)); }


//void GlobalFunctions::addRPad(string name, Ptr<Datatype> t) {
//  Id *dflt = new Id(" ", 0, true, false);
//  dflt->setSQuoted();
//  dflt->definition(Datatype::mkVarchar2(dflt->toString().size()));
//  add(new Sm::Function
//      (
//        string(name),
//        {
//          Arg("expr1", t),
//          Arg("n"    , Datatype::mkInteger()),
//          Arg("expr2", t, new RefExpr(dflt))
//        },
//        t
//      )
//     );
//}


void trRoundArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  str << argl;
  if (argl->size() == 1)
    str << s::comma() << 0;
}

void trDateRoundName(Codestream &str, Ptr<CallArgList> /*args*/) {
  if (str.isSql())
    str << "ROUND";
  else
    str << "date_round";
}

void trDateRoundArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  str << argl;
  if (argl->size() == 1)
    str << s::comma() << "'D'";
}


namespace ora_contexts {
  static const std::map<string, string> db_ctx =
  {
    {"CLIENTID"  , "N"},
    {"DTCH"      , "V"},
    {"CLIENTNAME", "V"},
    {"DBPROFILE" , "V"},
    {"USERID"    , "N"},
    {"PERSID"    , "N"},
    {"SDK"       , "N"},
    {"INSTANCE"  , "V"},
    {"REFERENCE" , "N"}
  };
  static const std::map<string, string> operlog_context =
  {
    {"PERSONAL_ID"         , "V"},
    {"UPDATE_DATABASE_ID"  , "N"},
    {"CURRENT_BUILD_UPDATE", "N"}
  };
  static const std::map<string, string> userenv_context =
  {
    {"SESSION_USER", "V"},
    {"HOST",         "V"},
    {"SESSIONID",    "N"},
    {"USERID",       "N"},
    {"TERMINAL",     "V"},
    {"SID",          "N"},
    {"ENTRYID",      "N"},
    {"SCHEMAID",     "N"}
  };
  typedef std::map<string, std::map<string, string> > MapContexts;
  static const MapContexts contexts =
  {
    {"DB_CTX"         , db_ctx         },
    {"OPERLOG_CONTEXT", operlog_context},
    {"USERENV"        , userenv_context}
  };
}



bool getSysContextArgName(Ptr<FunCallArg> fc, string &dst, bool checkQuoted) {
  Ptr<Id> name = fc->getName();
  if (!name || (checkQuoted && !name->quoted())) {
    cl::filelocation l = fc->getLLoc();
    string text = SmartLexer::textFromFile(l);
    cout << endl << "error: sys_context has not constant name: " << l << ": " << text << endl;
    return false;
  }
  dst = name->toNormalizedString();
  return true;
}

template <class ArgList>
bool initCtxParamNames(ArgList *args, std::string &ctxName, std::string &paramName) {
  if (args && args->size() >= 2) {
    Ptr<FunCallArg> arg1 = *(  (args->begin()));
    Ptr<FunCallArg> arg2 = *(++(args->begin()));

    if (!getSysContextArgName(arg1, ctxName, /*checkQuoted=*/ true))
      return false;
    transform(ctxName.begin(), ctxName.end(), ctxName.begin(), ::toupper);

    if (!getSysContextArgName(arg2, paramName, /*checkQuoted=*/ false))
      return false;
    transform(paramName.begin(), paramName.end(), paramName.begin(), ::toupper);
    return true;
  }
  return false;
}


std::string trByCtxNameParamName(std::string funname, std::string ctxName, std::string paramName) {
  ora_contexts::MapContexts::const_iterator ctx = ora_contexts::contexts.find(ctxName);
  if (ctx != ora_contexts::contexts.end()) {
    ora_contexts::MapContexts::mapped_type::const_iterator suffix = ctx->second.find(paramName);
    if (suffix != (ctx->second.end())) {
      funname.push_back('_');
      return funname + suffix->second;
    }
  }
  return funname;
}


std::string trByContextFunSecondArg(std::string funname, Ptr<CallArgList> args) {
  std::string ctxName, paramName;
  if (!initCtxParamNames(args.object(), ctxName, paramName))
    return funname;
  return trByCtxNameParamName(funname, ctxName, paramName);
}

Ptr<Datatype> sysContextType(std::string ctxName, std::string paramName) {
  ora_contexts::MapContexts::const_iterator ctx = ora_contexts::contexts.find(ctxName);
  if (ctx != ora_contexts::contexts.end()) {
    ora_contexts::MapContexts::mapped_type::const_iterator suffix = ctx->second.find(paramName);
    if (suffix != ctx->second.end() && suffix->second == "V")
      return Datatype::mkVarchar();
  }
  return Datatype::mkNumber();
}

Ptr<Datatype> sysContextType(TemplateArglist &args) {
  std::string ctxName, paramName;
  if (initCtxParamNames(&args, ctxName, paramName))
    return sysContextType(ctxName, paramName);
  return Datatype::mkNumber();
}


void trSysContextName(Sm::Codestream &str, Ptr<CallArgList> args) {
  str << trByContextFunSecondArg("SYS.SYS_CONTEXT", args);
}

class CreateSysContext : public OraTemplateData {
public:
  CreateSysContext(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (callArglistLenNEQ(callArgList, 2))
      throw 999;

    Ptr<Datatype> t = sysContextType(callArgList);
    if (Ptr<Function> f = findCreatedFunByRettype(t, isPlContext, callArgList.size()))
      return f;

    Ptr<Function> f =  new Sm::Function(funname, {
        Arg("_namespace", Datatype::mkVarchar2()),
        Arg("_parameter", Datatype::mkVarchar2()),
        Arg("length", Datatype::mkInteger(), new Sm::NumericSimpleInt(256)) }, t);
    f->setIsSysUnsupported();
    f->flags.clrElementaryLinterFunction();
    f->nameTranslator = trSysContextName;
    updateTemplateNamespace(f);
    reference->setResolvedFunction();
    return f;
  }
};

void trUserenvName(Sm::Codestream &str, Ptr<CallArgList> args) {
  Ptr<FunCallArg> arg1 = *(args->begin());
  if (arg1->beginedFrom(454396))
    cout << "";
  string paramName = arg1->getName()->toNormalizedString();
  std::transform(paramName.begin(), paramName.end(), paramName.begin(), ::toupper);
  str << trByCtxNameParamName("SYS.USERENV", "USERENV", paramName);
}

class CreateUserEnv : public OraTemplateData {
public:
  CreateUserEnv(const string &funname) : OraTemplateData(funname) {}
  Ptr<Function> constructor(Ptr<Id> &reference, TemplateArglist &callArgList, bool isPlContext) {
    if (reference->beginedFrom(454396))
      cout << "";
    if (callArglistLenNEQ(callArgList, 1))
      return 0;
    Ptr<PlExpr> expr = callArgList[0]->expr();
    if (!expr)
      return 0;

    string argname;
    expr->toNormalizedString(argname);
    std::transform(argname.begin(), argname.end(), argname.begin(), ::toupper);

    Ptr<Datatype> t = sysContextType("USERENV", argname);
    reference->setResolvedFunction();

    if (Ptr<Function> f = findCreatedFunByRettype(t, isPlContext, callArgList.size()))
      return f;

    Ptr<Function> userenvFun = new Function(string(funname), { Arg("parameter", t) }, t);
    userenvFun->setIsSysUnsupported();
    userenvFun->clrElementaryLinFunc();
    userenvFun->nameTranslator = trUserenvName;
    updateTemplateNamespace(userenvFun);
    return userenvFun;
  }
};

void zz(Sm::Codestream &str, Ptr<CallArgList> args) {
  str << args->front()->expr() << ", ";
  string s;
  if (1)
    str << "\'" << s << "\'";
}


void trPower(Sm::Codestream &str, Ptr<CallArgList> args) {
  if (str.isProc() && args && args->size() == 2)
    str << *(args->begin()) << " ^ " << *(++(args->begin()));
  else
    str << "POWER" << s::obracket << args << s::cbracket;
}


void trPowerArglist(Ptr<Id> &, Codestream &) {}


void trChr(Sm::Codestream &str, Ptr<CallArgList> args) {
  if (str.procMode() == CodestreamState::PROC && args && args->size() == 1)
    if (Ptr<PlExpr> val = args->front()->expr()) {
      if (val->isIntValue()) {
        unsigned int v = (unsigned char)((char)(val->getSIntValue()));
        char q = str.isProc() ? '"' : '\'';
        switch (v) {
          case 9 : str << q << "\\t" << q; break;
          case 10: str << q << "\\n" << q; break;
          case 13: str << q << "\\r" << q; break;
          default: {
            char buf[128];
            sprintf(buf, "\\x%x", v);
            str << q << buf << q;
            break;
          }
        }
      }
      else
        str << "CHR"
            << s::obracket
              << "HEXTORAW"
              << s::obracket
                << "RAWTOHEX"
                << s::obracket
                  << val->unwrapBrackets()
                << s::cbracket
              << s::cbracket
            << s::cbracket;
      return;
    }
  str << "CHR" << s::obracket << args << s::cbracket;
}


void trChrArglist(Ptr<Id> &, Codestream &) {}


void trExtract(Sm::Codestream &str, Ptr<CallArgList>) {
  if (str.isProc())
    str << "DATESPLIT";
  else
    str << "EXTRACT";
}


void trExtractArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
   static const string argLinter[(int)ExtractedEntity::TIMEZONE_ABBR + 1] = {
       // DAY, YEAR, MONTH, SECOND, HOUR, MINUTE, TIMEZONE_HOUR, TIMEZONE_MINUTE, TIMEZONE_REGION, TIMEZONE_ABBR
       "D", "Y", "M", "SS", "HH", "MI", "HH", "MI", "", ""
   };

   if (str.isProc() && args->size() > 0) {
     Ptr<FunCallArg> arg = *args->begin();
     if (arg->expr())
       if (ExtractExpr *extractFrom = arg->expr()->toSelfExtractFromExpression()) {
         if (extractFrom->extractedEntity_ >= ExtractedEntity::TIMEZONE_REGION)
           throw 999;
         str << extractFrom->fromEntity << s::comma() << "\"" << argLinter[(int)extractFrom->extractedEntity_] << "\"";
         return;
       }
   }

   str << args;
}


void trDbmsOperlogSetOperlogContext(Sm::Codestream &str, Ptr<CallArgList> args) {
  std::string funname = "SYS.dbms_session_set_context";
  if (args && args->size() >= 1) {
    Ptr<FunCallArg> arg1 = *(  (args->begin()));
    std::string paramName = arg1->getName()->toNormalizedString();
    transform(paramName.begin(), paramName.end(), paramName.begin(), ::toupper);
    str << trByCtxNameParamName(funname, "OPERLOG_CONTEXT", paramName);
  }
  else
    str << "UPDATES.DBMS_OPERLOG_SET_OPERLOG_CONTEXT";
}


void trDbmsOperlogSetOperlogContextCallarglist(Ptr<Id> &call, Codestream &str) {
  str << "\"operlog_context\"";
  if (call->callArglist && call->callArglist->size())
    str << s::comma() << s::name << call->callArglist;
}


void trDbmsSessionSetContextName(Sm::Codestream &str, Ptr<CallArgList> args) {
  std::string ctxName, paramName;
  str << "SYS.dbms_session_set_context";
  if (initCtxParamNames(args.object(), ctxName, paramName)) {
    ora_contexts::MapContexts::const_iterator ctx = ora_contexts::contexts.find(ctxName);
    if (ctx != ora_contexts::contexts.end()) {
      ora_contexts::MapContexts::mapped_type::const_iterator suffix = ctx->second.find(paramName);
      if (suffix != (ctx->second.end())) {
        str << '_' << suffix->second;
        return;
      }
    }
  }
  if (args && args->size() >= 3) {
    Ptr<FunCallArg> arg3 = *(next(next(args->begin())));
    if (arg3->expr()->isSubtype(Datatype::mkVarchar2(), false))
      str << "_V";
    else if (arg3->expr()->isSubtype(Datatype::mkNumber() , false))
      str << "_N";
  }
}


void translateCallarglistRaise_Application_Error(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  if (argl && argl->size()) {
    CallArgList::iterator it;
    it = argl->begin();
    Ptr<FunCallArg> arg1 = (*it);
    if (arg1->expr()->isQuotedSqlExprId()) {
      string s;
      arg1->expr()->toNormalizedString(s);
      str << s;
    }
    else
      str << *it;
    if (argl->size() > 1) {
      ++it;
      str << " /* " << s::DisableIndenting() << *it << " */" << s::EnableIndenting() << s::endl
          << s::tab() << s::tab(11); // raise_error = 11 char
    }
  }
}


void trReplaceClobArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  str << argl;
  if (str.procMode() == CodestreamState::PROC && !str.state().queryForExecute_) {
    cout << "error: trReplaceClobArgs in not dynamic PROC mode: " << call->getLLoc() << endl;
  }

}


void trUidName(Sm::Codestream &str, Ptr<Sm::CallArgList> /*args*/) {
  str << (str.isProc() ? "USERID" : "LINTER_USER_ID");
}

void trUserName(Sm::Codestream &str, Ptr<Sm::CallArgList> /*args*/) {
  str << (str.isProc() ? "USERNAME" : "USER");
}

void trToDateVarcharArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  Ptr<FunCallArg> arg1 = *argl->begin();
  if (arg1->expr()->isNull())
    str << (str.isProc() ? "\"\"" : "''");
  else {
    str << argl;
    if (argl->size() == 1) {
      string qstr;
      if (arg1->expr()->isQuotedSqlExprId()) {
        int dd, mm, yyyy;
        arg1->expr()->toNormalizedString(qstr);
        if (sscanf(qstr.c_str(), "%d-%d-%d", &dd, &mm, &yyyy) == 3) {
          str << s::comma() << squote(str) << "DD-MM-YYYY" << squote(str);
          return;
        }
      }
      str << s::comma() << squote(str) << "DD.MM.YYYY:HH24:MI:SS" << squote(str);
    }
  }
}


void trToDateArgs(Ptr<Id> &call, Codestream &str) {
  //TODO: Возможно, если не указан параметр 2, для оптимизации скорости исполнения нужно заменить на CAST (SQL), пусто (PROC).
  Ptr<CallArgList> argl = call->callArglist;
  str << (str.isProc() ? "dtoa" : "to_char") << s::obracket << (*argl->begin());
  if (argl->size() == 2)
    str << s::comma() << *(++argl->begin());
  str << s::cbracket;
  if (argl->size() == 2)
     str << s::comma() << *(++argl->begin());
}

void trToDateIntArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  Ptr<FunCallArg> arg1 = *argl->begin();
  Ptr<Datatype> t = arg1->getDatatype();
  if ((arg1->expr()->isNumericValue() && arg1->expr()->toSelfNumericValue()->getSIntValue() == 0) ||
      (t && t->isNull())) {
    if (str.isProc())
      str << "\"\"";
    else
      str << "''";
  }
  else {
    argToChar(arg1, str);
    if (argl->size() > 1)
      str << s::comma() << *(++argl->begin());
  }
}

void trLastDayCharArgs(Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  str << "to_date" << s::obracket << argl;
  str << s::comma() << squote(str) << "DD.MM.YYYY:HH24:MI:SS" << squote(str);
  str << s::cbracket;
}


Sm::GlobalFunctions::GlobalFunctions()
{
  Sm::Function *f;

  oraTemplatesFunMap["DECODE"         ] = new CreateDecodeFun       ("DECODE"    );
  oraTemplatesFunMap["GREATEST"       ] = new CreateGreatestLeastFun("GREATEST"  );
  oraTemplatesFunMap["LEAST"          ] = new CreateGreatestLeastFun("LEAST"     );
  oraTemplatesFunMap["LTRIM"          ] = new CreateLtrimRtrim     ("LTRIM"      );
  oraTemplatesFunMap["RTRIM"          ] = new CreateLtrimRtrim     ("RTRIM"      );
  oraTemplatesFunMap["TRUNC"          ] = new CreateTrunc          ("TRUNC", this);
  oraTemplatesFunMap["SUBSTR"         ] = new CreateSubstr         ("SUBSTR"     );
  oraTemplatesFunMap["REGEXP_SUBSTR"  ] = new CreateRegexpSubstr   ("SUBSTRING"  );


  oraTemplatesFunMap["TO_CHAR"        ] = new CreateToChar         ("TO_CHAR", this);
  oraTemplatesFunMap["NLS_UPPER"      ] = new CreateNlsUpperInitcap("NLS_UPPER"  );
  oraTemplatesFunMap["NLS_INITCAP"    ] = new CreateNlsUpperInitcap("NLS_INITCAP");
  oraTemplatesFunMap["CONCAT"         ] = new CreateConcat         ("CONCAT"     );
  oraTemplatesFunMap["RPAD"           ] = new CreateLRpad          ("RPAD"       );
  oraTemplatesFunMap["LPAD"           ] = new CreateLRpad          ("LPAD"       );
  oraTemplatesFunMap["BITAND"         ] = new CreateBitand         ("BITAND"     );
  oraTemplatesFunMap["NULLIF"         ] = new CreateNullif         ("NULLIF"     );
  oraTemplatesFunMap["AVG"            ] = new CreateNumericTemplate("AVG"        );
  oraTemplatesFunMap["CEIL"           ] = new CreateNumericTemplate("CEIL"       );
  oraTemplatesFunMap["SUM"            ] = new CreateSum            ("SUM"        );
  oraTemplatesFunMap["ABS"            ] = new CreateNumericTemplate("ABS"        );
  oraTemplatesFunMap["UPPER"          ] = new CreateUpperLower     ("UPPER"      );
  oraTemplatesFunMap["LOWER"          ] = new CreateUpperLower     ("LOWER"      );
  oraTemplatesFunMap["TRIM"           ] = new CreateTrimTemplate   ("TRIM"       );
  oraTemplatesFunMap["MIN"            ] = new CreateMinMax         ("MIN"        );
  oraTemplatesFunMap["MAX"            ] = new CreateMinMax         ("MAX"        );
  oraTemplatesFunMap["NVL"            ] = new CreateNvl            ("NVL"        );
  oraTemplatesFunMap["NVL2"           ] = new CreateNvl2           ("NVL2"       );
  oraTemplatesFunMap["USERENV"        ] = new CreateUserEnv        ("USERENV"    );
  oraTemplatesFunMap["TO_NUMBER"      ] = new CreateToNumber       ("TO_NUMBER", this);
  oraTemplatesFunMap["CONNECT_BY_ROOT"] = new CreateConnectByRoot  ("CONNECT_BY_ROOT");
  oraTemplatesFunMap["SYS_CONTEXT"    ] = new CreateSysContext     ("SYS_CONTEXT");

  Datatype *Varchar2            = Datatype::mkVarchar2();
  Datatype *Boolean             = Datatype::mkBoolean();
  Datatype *Integer             = Datatype::mkInteger();
  Datatype *Number              = Datatype::mkNumber();
  Datatype *Char                = Datatype::mkChar();
  Datatype *Clob                = Datatype::mkClob();
  Datatype *Float               = Datatype::mkFloat();
  Datatype *Double              = Datatype::mkDouble();
  Datatype *Raw                 = Datatype::mkRaw();
  Datatype *Date                = Datatype::mkDate();
  Datatype *Blob                = Datatype::mkBlob();
  Datatype *Timestamp           = Datatype::mkTimestamp();
  Datatype *TimestampTimezone   = Datatype::mkTimestampTimezone();
  Datatype *IntervalDayToSecond = Datatype::mkIntervalDayToSecond();
  Datatype *NChar               = Datatype::mkNChar();
  Datatype *NVarchar2           = Datatype::mkNVarchar2();


  f = add(new Sm::Function("EXTRACT", { Arg("value", Number)   }, Datatype::mkNumber(5)  ));
  f->nameTranslator = trExtract;
  f->callarglistTranslator = new CallarglistTranslatorSimple(trExtractArgs);

  f =  add(new Sm::Function("EXTRACT", { Arg("value", Varchar2) }, Datatype::mkVarchar2(128)));
  f->nameTranslator = trExtract;
  f->callarglistTranslator = new CallarglistTranslatorSimple(trExtractArgs);

  add(new Sm::Function("MONTHS_BETWEEN", { Arg("date1", Date    ), Arg("date2", Date    ) }, Number)); // Дробное количество месяцев

  add(new Sm::Function("TRANSLATE", { Arg("expr", Varchar2), Arg("from_string", Varchar2), Arg("to_string", Varchar2) }, Varchar2))->setElementaryLinFunc();

  add(new Sm::Function("SYS_CONNECT_BY_PATH", { Arg("col", Varchar2), Arg("ch", Varchar2) }, Varchar2));

  // Replace(str String, str1 String, str2 String hidden_dflt = '') return String");
      add(new Sm::Function("REPLACE", { Arg("value", Varchar2), Arg("search_string", Varchar2), Arg("replacement_string", Varchar2, new NullExpr())}, Varchar2, true));

  f = add(new Sm::Function("REPLACE", { Arg("value", Clob    ), Arg("search_string", Varchar2), Arg("replacement_string", Varchar2, new NullExpr())}, Clob    , true));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trReplaceClobArgs);

  f = add(new Sm::Function("UID"    , {}, Integer));
  f->nameTranslator = trUidName;
  f = add(new Sm::Function("USER"   , {}, Varchar2, true));
  f->nameTranslator = trUserName;
  f = add(new Sm::Function("LAST_DAY", { Arg("d", Varchar2) }, Date));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trLastDayCharArgs);
  add(new Sm::Function("LAST_DAY", { Arg("d", Date)     }, Date))->setElementaryLinFunc();

  add(new Sm::Function("ASCII", { Arg("c", Char)      }, Datatype::mkNumber(4)));
  add(new Sm::Function("ASCII", { Arg("c", Varchar2)  }, Datatype::mkNumber(4), true));
  add(new Sm::Function("ASCII", { Arg("c", NChar)     }, Datatype::mkNumber(4)));
  add(new Sm::Function("ASCII", { Arg("c", NVarchar2) }, Datatype::mkNumber(4)));

  Ptr<FunArgList> argList = new FunArgList;
  argList->push_back(new FunctionArgument(new Id("str"   ), Varchar2));
  argList->push_back(new FunctionArgument(new Id("substr"), Varchar2));
  argList->push_back(new FunctionArgument(new Id("pos"   ), Integer, new Sm::NumericSimpleInt(1)));
  argList->push_back(new FunctionArgument(new Id("occur" ), Integer, new Sm::NumericSimpleInt(1)));
  add(new Sm::Function(new Id2("INSTR" ), argList, Integer))->flags.setElementaryLinterFunction();
  add(new Sm::Function(new Id2("INSTRB"), argList, Integer));
  add(new Sm::Function(new Id2("INSTRC"), argList, Integer));
  add(new Sm::Function(new Id2("INSTR2"), argList, Integer));
  add(new Sm::Function(new Id2("INSTR4"), argList, Integer));

  add(new Sm::Function("SIGN" , { Arg("n", Number)    }, Datatype::mkNumber(4)));

  f = add(new Sm::Function(new Id2("SYSTIMESTAMP")  , 0, TimestampTimezone));
  f->translatedName("SYSDATE");
  f = add(new Sm::Function(new Id2("SQLCODE")       , 0, Integer));
  f->translatedName("ERRCODE");
  f = add(new Sm::Function(new Id2("EMPTY_CLOB")    , 0, Clob));
  f->translatedName("NULL");
  f->clrBracesOutput();
  f = add(new Sm::Function(new Id2("EMPTY_BLOB")    , 0, Blob));
  f->translatedName("NULL");
  f->clrBracesOutput();
  f = add(new Sm::Function(new Id2("LOCALTIMESTAMP"), 0, Timestamp));
  f->translatedName("to_localtime(SYSDATE())");
  f->clrBracesOutput();


  Ptr<CallarglistTranslator> trPowerArgs = new CallarglistTranslatorSimple(trPowerArglist);
  auto addPower = [&](Datatype *rettype) {
    f = add(new Sm::Function("POWER",     { Arg("n2", rettype), Arg("n3", rettype) }, rettype));
    f->nameTranslator = trPower;
    f->callarglistTranslator = trPowerArgs;
    f->clrBracesOutput();
  };
  addPower(Number);
  addPower(Double);
  addPower(Float);
  addPower(Datatype::mkReal());

  add(new Sm::Function("NEXT_DAY", { Arg("dt", Date), Arg("dayname", Varchar2) }, Date));

  // To_Date(date String, format String := 'DD.MM.YY:HH24:MI:SS') return Date
  f = add(new Sm::Function("TO_DATE", { Arg("date", Varchar2), Arg("format", Varchar2, SqlExpr::literal("DD.MM.YY:HH24:MI:SS")) }, Date, true));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trToDateVarcharArgs);
  f = add(new Sm::Function("TO_DATE", { Arg("date", Date), Arg("format", Varchar2, SqlExpr::literal("DD.MM.YY:HH24:MI:SS")) }, Date));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trToDateArgs);
  f = add(new Sm::Function("TO_DATE", { Arg("date", Integer), Arg("format", Varchar2, SqlExpr::literal("DD.MM.YY:HH24:MI:SS")) }, Date));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trToDateIntArgs);

  // Length(str String) return Integer
  add(new Sm::Function("LENGTH", { Arg("str", Varchar2) }, Integer, true));
  f = add(new Sm::Function("LENGTH", { Arg("num", Number) }, Integer, true));
  f->callarglistTranslator = new CallarglistTranslatorSimple(firstArgToChar);
  f = add(new Sm::Function("LENGTH", { Arg("num", Integer) }, Integer, true));
  f->callarglistTranslator = new CallarglistTranslatorSimple(firstArgToChar);

  // SysDate   return Date"
  add(new Sm::Function("SYSDATE", {}, Date, true));

  add(new Sm::Function("NUMTODSINTERVAL", { Arg("n", Number), Arg("interval_unit", Varchar2) }, IntervalDayToSecond));

  add(new Sm::Function("COUNT", { Arg("value", Number)   }, Integer));
  add(new Sm::Function("COUNT", { Arg("value", Integer)  }, Integer));
  add(new Sm::Function("COUNT", { Arg("value", Varchar2) }, Integer));

  // Chr(value Integer) return String
  f = add(new Sm::Function("CHR", { Arg("value", Integer) }, Datatype::mkVarchar2(1), true));
  f->nameTranslator = trChr;
  f->callarglistTranslator = new CallarglistTranslatorSimple(trChrArglist);
  f->clrBracesOutput();

  // Raise_application_error(val1 Integer)
  f = add(new Sm::Function("RAISE_APPLICATION_ERROR", { Arg("val1", Integer) }));
  f->translatedName("raise_error");
  f->callarglistTranslator = new CallarglistTranslatorSimple(translateCallarglistRaise_Application_Error);
  f->flags.setElementaryLinterFunction();

  // TODO: далее идут функции, для которых нужно будет писать собственную реализацию
  f = add(new Sm::Function("RAISE_APPLICATION_ERROR", { Arg("error_number", Number), Arg("message", Varchar2), Arg("replace_prev", Boolean, SqlExpr::boolFalse()) }, 0, true));

  f->translatedName("raise_error");
  f->callarglistTranslator = new CallarglistTranslatorSimple(translateCallarglistRaise_Application_Error);

  add(new Sm::Function("LOG", { Arg("n1", Number), Arg("n2", Number) }, Number));
  add(new Sm::Function("LOG", { Arg("n1", Float) , Arg("n2", Float)  }, Double));
  add(new Sm::Function("LOG", { Arg("n1", Double), Arg("n2", Double) }, Double));

  f =  add(new Sm::Function("SQLERRM", { Arg("error_number", Integer, new NullExpr()) }, Varchar2, true));

  add(new Sm::Function("FLOOR"   , { Arg("num", Number)   }, Number, true));
  add(new Sm::Function("LN"      , { Arg("num", Number)   }, Number));
  add(new Sm::Function("LN"      , { Arg("num", Float)    }, Double));
  add(new Sm::Function("EXP"     , { Arg("num", Number)   }, Number));
  add(new Sm::Function("EXP"     , { Arg("num", Float)    }, Double));
  add(new Sm::Function("HEXTORAW", { Arg("c"  , Char)     }, Raw));
  add(new Sm::Function("HEXTORAW", { Arg("c"  , Varchar2) }, Raw));

  add(new Sm::Function("ADD_MONTHS", { Arg("src_date", Date), Arg("added_months", Integer) }, Date));
  add(new Sm::Function("MOD", { Arg("n1", Integer), Arg("n2", Integer) }, Integer))->setElementaryLinFunc();
  f = add(new Sm::Function("MOD", { Arg("n1", Varchar2), Arg("n2", Integer) }, Integer));
  f->setElementaryLinFunc();
  f->callarglistTranslator = new CallarglistTranslatorSimple(trArgsToNumber);
  f =  add(new Sm::Function("ROUND", { Arg("n", Number), Arg("toRound", Integer, new NumericSimpleInt(0)) }, Double, true));
  f->callarglistTranslator = new CallarglistTranslatorSimple(trRoundArgs);

  f =  add(new Sm::Function("ROUND", { Arg("n", Date), Arg("toRound", Varchar2, SqlExpr::literal("D")) }, Date, true));
  f->nameTranslator = trDateRoundName;
  f->callarglistTranslator = new CallarglistTranslatorSimple(trDateRoundArgs);


  add(new Sm::Function("TO_DSINTERVAL", { Arg("fmt_str", Varchar2) }, IntervalDayToSecond));
  f = add(new Sm::Function("MONTHS_BETWEEN", { Arg("date1", Date), Arg("date2", Date) }, Number, false));
  f->translatedName("SYS.MONTHS_BETWEEN");
  f->flags.clrElementaryLinterFunction();



  f = add(new Sm::Function("REGEXP_REPLACE", {
      Arg("source_char"   , Varchar2),
      Arg("pattern"       , Varchar2),
      Arg("replace_string", Varchar2, SqlExpr::literal("")),
      Arg("position"      , Integer, new NumericSimpleInt(1)),
      Arg("occurence"     , Integer, new NumericSimpleInt(0)),
      Arg("math_parameter", Integer, SqlExpr::literal(""))
    }, Varchar2));
  f->translatedName("SYS.REGEXP_REPLACE");
}

GlobalFunctions::~GlobalFunctions() {
  for (SemanticNodes::value_type &v : semanticNodes)
    delete v;
  systemFunctionsMap.clear();
  oraTemplatesFunMap.clear();
}



void Sm::GlobalFunctions::addLinterFunctions() {
  linterLenblob = add(new Sm::Function("lenblob", { Arg("name"  , Datatype::mkVarchar2()), Arg("mode", Datatype::mkInteger(), new Sm::NumericSimpleInt(0)) }, Datatype::mkInteger()));
  linterGettext = add(new Sm::Function("gettext", { Arg("name"  , Datatype::mkVarchar2()),
                                                    Arg("offset", Datatype::mkInteger()),
                                                    Arg("len"   , Datatype::mkInteger())}, Datatype::mkVarchar2())
        );
  linterToDate = add(new Sm::Function("to_date", {Arg("val", Datatype::mkVarchar2()), Arg("fmt", Datatype::mkVarchar2(), SqlExpr::literal(string(defaultDateFormat)))}, Datatype::mkDate()));

}


void Sm::GlobalFunctions::addSpecialFun() {
  // !!!: Ограниченная версия UPDATEXML
  if (syntaxerContext.model->modelActions.oracleSysdepsParsed) {
    Datatype *Varchar2           = Datatype::mkVarchar2();
    if (Datatype *Xmltype        = Datatype::mkXmltype()) {
      add(new Sm::Function("UPDATEXML"   , { Arg("xmlinstance"     , Xmltype), Arg("xmlpath", Varchar2), Arg("updval", Xmltype)  }, Xmltype));
      add(new Sm::Function("UPDATEXML"   , { Arg("xmlinstance"     , Xmltype), Arg("xmlpath", Varchar2), Arg("updval", Varchar2) }, Xmltype));
      add(new Sm::Function("EXTRACTVALUE", { Arg("XmlType_instance", Xmltype), Arg("xmlpath", Varchar2), Arg("namespace_string", Varchar2, new NullExpr()) }, Varchar2));
    }
  }
}

OraTemplateData::OraTemplateData(const string &_funname) : funname(_funname) {}

CallarglistTranslatorSimple::CallarglistTranslatorSimple(CallarglistTranslatorSimple::TrFun _tr) : tr(_tr) { if (!tr) throw 999; }

void CallarglistTranslatorSimple::operator()(smart::Ptr<Id> &call, Codestream &str) {
  tr(call, str);
}

CreateGreatestLeastFun::ConflSelector::ConflSelector(Datatype *firstArgument)
  : firstArgumentT(firstArgument)
{
  if (!firstArgumentT)
    throw 9999;
}

Datatype *CreateGreatestLeastFun::ConflSelector::operator()(int lhsPos, Datatype *lhs, int rhsPos, Datatype *rhs, bool pl) {
  Datatype* lt = 0;
  Datatype* rt = 0;
  if (lhs)
    if (!(lt = Datatype::getMaximal(firstArgumentT, lhs, pl)))
      badDatatypeArguments[lhsPos] = lt;
  if (rhs)
    if (!(rt = Datatype::getMaximal(firstArgumentT, rhs, pl)))
      badDatatypeArguments[rhsPos] = lt;

  return lt && lt != firstArgumentT ? lt : (rt && rt != firstArgumentT ? rt : nullptr);
}

CreateGreatestLeastFun::CallarglistTralslatorGreatestLeast::CallarglistTralslatorGreatestLeast(CreateGreatestLeastFun::BadDatatypeArguments &args, Datatype *firstDatatypeCat) {
  if (!(firstDatatypeCat = StructureSubqueryUnwrapper::unwrap(firstDatatypeCat)))
    throw 999;
  badDatatypeArguments.swap(args);
  if (firstDatatypeCat->isNumberDatatype())
    cat = NUMERIC;
  else if (firstDatatypeCat->isCharVarchar())
    cat = VARCHAR;
  else
    throw 999;
}

void CreateGreatestLeastFun::CallarglistTralslatorGreatestLeast::operator()(smart::Ptr<Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  int i = 0;
  bool isNotFirst = false;
  for (CallArgList::value_type &v : *args) {
    str << s::comma(&isNotFirst);
    if (badDatatypeArguments.count(i)) {
      switch (cat) {
        case NUMERIC:
          str << numericFunname(str) << s::obracket << v << s::cbracket;
        case VARCHAR:
          argToChar(v.object(), str);
      }
    }
    ++i;
  }
}


//GlobalFunctions::OraTemplateData::OraTemplateData() : constructor(0) {}


//GlobalFunctions::OraTemplateData::OraTemplateData(GlobalFunctions::CreateFunctionByArglist c) : constructor(c) {}


}
