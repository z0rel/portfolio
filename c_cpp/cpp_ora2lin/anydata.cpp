#include "semantic_function.h"
#include "anydata.h"
#include "syntaxer_context.h"
#include "model_context.h"


extern SyntaxerContext syntaxerContext;



namespace Sm {

void trGettype(Sm::Codestream &str, Ptr<CallArgList> /*args*/) {
  str << "SYS.ANYDATA_GETTYPE";
}

void checkAnydataArglist(Ptr<Sm::Id> &call)
{
  if (!call || !call->callArglist || call->callArglist->empty() || !call->callArglist->front())
    throw 999;
  if (Ptr<IdEntitySmart> sm = call->callArglist->front()->expr()->unwrapBrackets()->getMajorIdEntity())
    if (ResolvedEntity *d = sm->definition())
      if (Sm::Type::MemberFunction *mf =  d->toSelfMemberFunction())
        if (mf->flags.isAnydataMember())
          throw 999;
}

void trAnydataMethodArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> argl = call->callArglist;
  if (argl && argl->size())
    if (Ptr<Datatype> t = argl->front()->getDatatype())
      if (t->isAnydata()) {
        checkAnydataArglist(call);
        str << argl->front();
        if (argl->size() > 1)
          throw 999;
        return;
      }

   if (Sm::SemanticTree *node = call->semanticNode()) {
     IdEntitySmart* ref = node->reference();
     ref->pop_front();
     if (!ref->empty())
       str << *ref;
   }
   return;
}



void trAnydataMethodArgsCheck(Ptr<Sm::Id> &call, Codestream &str)
{
  checkAnydataArglist(call);
  str << call->callArglist;
}



void Anydata::initAnydataContainers() {
  initMf(&convertnvarchar2, {"SYS", "ANYDATA", "CONVERTNVARCHAR2"}, "ANYDATA_CONVERTNVARCHAR", trAnydataMethodArgsCheck, Datatype::mkNVarchar2());
  initMf(&convertnvarchar2, {"SYS", "ANYDATA", "CONVERTNVARCHAR2"}, "ANYDATA_CONVERTVARCHAR2", trAnydataMethodArgsCheck, Datatype::mkVarchar2());

  initMf(&set_varchar2,     {"SYS", "ANYDATA", "SETVARCHAR2"     }, "ANYDATA_SET_VARCHAR2"   , trAnydataMethodArgsCheck);
  initMf(&set_number,       {"SYS", "ANYDATA", "SETNUMBER"       }, "ANYDATA_SET_NUMBER"     , trAnydataMethodArgsCheck);
  initMf(&set_date,         {"SYS", "ANYDATA", "SETDATE"         }, "ANYDATA_SET_DATE"       , trAnydataMethodArgsCheck);
  initMf(&getvarchar2,      {"SYS", "ANYDATA", "GETVARCHAR2"     }, "ANYDATA_GETVARCHAR2"    , trAnydataMethodArgsCheck);
  initMf(&getnumber,        {"SYS", "ANYDATA", "GETNUMBER"       }, "ANYDATA_GETNUMBER"      , trAnydataMethodArgsCheck);
  initMf(&getdate,          {"SYS", "ANYDATA", "GETDATE"         }, "ANYDATA_GETDATE"        , trAnydataMethodArgsCheck);
  initMf(&convertnumber,    {"SYS", "ANYDATA", "CONVERTNUMBER"   }, "ANYDATA_CONVERTNUMBER"  , trAnydataMethodArgsCheck);
  initMf(&convertvarchar2,  {"SYS", "ANYDATA", "CONVERTVARCHAR2" }, "ANYDATA_CONVERTVARCHAR2", trAnydataMethodArgsCheck);
  initMf(&convertdate,      {"SYS", "ANYDATA", "CONVERTDATE"     }, "ANYDATA_CONVERTDATE"    , trAnydataMethodArgsCheck);
  initMf(&convertdate,      {"SYS", "ANYDATA", "CONVERTDATE"     }, "ANYDATA_CONVERTDATE"    , trAnydataMethodArgsCheck);

  initMf(&accessdate,       {"SYS", "ANYDATA", "ACCESSDATE"      }, "ANYDATA_ACCESSDATE"     , trAnydataMethodArgs);
  initMf(&accessnumber,     {"SYS", "ANYDATA", "ACCESSNUMBER"    }, "ANYDATA_ACCESSNUMBER"   , trAnydataMethodArgs);
  initMf(&accessvarchar,    {"SYS", "ANYDATA", "ACCESSVARCHAR"   }, "ANYDATA_ACCESSVARCHAR"  , trAnydataMethodArgs);
  initMf(&accessvarchar2,   {"SYS", "ANYDATA", "ACCESSVARCHAR2"  }, "ANYDATA_ACCESSVARCHAR"  , trAnydataMethodArgs);
  initMf(&accessnchar,      {"SYS", "ANYDATA", "ACCESSNCHAR"     }, "ANYDATA_ACCESSNCHAR"    , trAnydataMethodArgs);
  initMf(&accessnvarchar,   {"SYS", "ANYDATA", "ACCESSNVARCHAR2" }, "ANYDATA_ACCESSNVARCHAR" , trAnydataMethodArgs);
  initMf(&accesschar,       {"SYS", "ANYDATA", "ACCESSCHAR"      }, "ANYDATA_ACCESSCHAR"     , trAnydataMethodArgs);
  initMf(&gettype,          {"SYS", "ANYDATA", "GETTYPE"         }, "ANYDATA_GETTYPE"        , trAnydataMethodArgs);
  gettype->nameTranslator      = trGettype;

  initCode(&typecodeNumber,    {"SYS", "DBMS_TYPES", "TYPECODE_NUMBER"   }, 1 );
  initCode(&typecodeVarchar2,  {"SYS", "DBMS_TYPES", "TYPECODE_VARCHAR2" }, 2 );
  initCode(&typecodeVarchar,   {"SYS", "DBMS_TYPES", "TYPECODE_VARCHAR"  }, 2 );
  initCode(&typecodeNchar,     {"SYS", "DBMS_TYPES", "TYPECODE_NCHAR"    }, 5 );
  initCode(&typecodeNVarchar2, {"SYS", "DBMS_TYPES", "TYPECODE_NVARCHAR2"}, 6 );
  initCode(&typecodeChar,      {"SYS", "DBMS_TYPES", "TYPECODE_CHAR"     }, 4 );
  initCode(&typecodeDate,      {"SYS", "DBMS_TYPES", "TYPECODE_DATE"     }, 3 );
  initCode(&typecodeObject,    {"SYS", "DBMS_TYPES", "TYPECODE_OBJECT"   }, -1);
  initCode(&typecodeRef,       {"SYS", "DBMS_TYPES", "TYPECODE_REF"      }, -2);
}

void Anydata::initMf(Type::MemberFunction **mf, IdEntitySmart name, string trName, Sm::CallarglistTranslator::TrFun callarglistTr, Datatype *argtype) {
  if (argtype) {
    Ptr<Sm::Id> var = new Id("arg1", argtype);
    Ptr<RefExpr> refExpr = new Sm::RefExpr(var);
    Ptr<FunCallArgExpr> callExpr = new FunCallArgExpr(refExpr);
    Ptr<CallArgList> callarglist = new CallArgList(callExpr.object());
    name.entity()->callArglist = callarglist;

    ResolvedEntity *def = 0;
    for (IdEntitySmart::reverse_iterator rit = name.rbegin(); rit != name.rend(); ++rit) {
      if (rit == name.rbegin())
        syntaxerContext.model->getFieldRef(*rit);
      else if (def)
        def->getFieldRef(*rit);
      else
        break;
      def = (*rit)->definition();
    }
  }
  else
    name.resolveByModelContext();

  *mf = name.definition()->toSelfMemberFunction();
  VEntities::OverloadedFunctions &ovlMap = (*mf)->vEntities()->overloadedFunctions;

  for (VEntities::OverloadedFunctions::iterator it = ovlMap.begin(); it != ovlMap.end(); ++it) {
    Sm::Type::MemberFunction *ovlFunc = it->first->toSelfMemberFunction();
    sAssert(!ovlFunc);
    ovlFunc->translatedName(trName);
    ovlFunc->flags.setAnydataMember();
    if (callarglistTr)
      ovlFunc->callarglistTranslator = new CallarglistTranslatorSimple(callarglistTr);
  }
}

void Anydata::initCode(Variable **var, IdEntitySmart name, int code) {
  name.resolveByModelContext();
  *var = name.definition()->toSelfVariable();
  (*var)->anydataCode(code);
}

}
