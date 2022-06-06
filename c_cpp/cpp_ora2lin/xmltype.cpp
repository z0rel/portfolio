#include "xmltype.h"
#include "semantic_function.h"
#include "semantic_object.h"

namespace Sm {

/*
  Функции:
  clob LOAD_CLOB( [cursor CUR], int FIELD )
  загружает поле блоб в переменную clob
  см. seek_blob по аргументам
  возвращает считанный блоб

  bool SAVE_CLOB( [cursor CUR], int FIELD, clob VALUE )
  Записывает clob в поле блоб

  clob TOCLOB( string STR )
  Преобразует строку STR (любого строкового типа) в тип clob

  string TOCHAR( clob C )
  Преобразует clob в строку, если длина позволяет. Если слишком длинный – исключение NO_MEM

  XMLTYPE XML_LOAD( clob|string STORAGE )
  Считывает и разбирает xml из STORAGE

  clob XML_WRITEBUF( XMLTYPE XML )
  Записывает XML в текстовом виде в результирующий clob
  Для записи в строку можно использовать tochar( xml_writebuf( xml ) ), если уверены что места хватит

  XMLREF XML_GETNODE( XMLTYPE|XMLREF XML )
  Преобразует в формат ссылки. Замена оракловским:
  NewDomDocumentElement,
  GetDocumentElement,
  MakeNode,
  MakeElement

  XMLREF XML_APPENDCHILD( XMLREF XML, XMLREF CHILD )
  XMLREF XML_GETATTRIBS( XMLREF XML )
  XMLREF XML_GETCHILDREN( XMLREF XML )
  XMLREF XML_GETBYNAME( XMLREF XML, string NAME )
  XMLREF XML_GETFIRSTCHILD( XMLREF XML )
  XMLREF XML_GETLASTCHILD( XMLREF XML )
  int XML_GETLENGTH( XMLREF XML )
  XMLREF XML_GETNEXT( XMLREF XML )
  string XML_GETNAME( XMLREF XML )
  string XML_GETVALUE( XMLREF XML )
  bool XML_SETNAME( XMLREF XML, string NAME )
  bool XML_SETVALUE( XMLREF XML, string VALUE )
  bool XML_HASCHILD( XMLREF XML )
  bool XML_ISNULL( XMLREF XML )
  XMLREF XML_GETITEM( XMLREF XML, int num )
 */

void trNewDomDocumentArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> callArgs = call->callArglist;
  if (!callArgs || callArgs->empty()) {
    str << "XML_LOAD(\"<root></root>\")";
  }
  else {
    Ptr<FunCallArg> arg1 = *(callArgs->begin());
    if (arg1->getDatatype()->isClobDatatype() ||
        arg1->getDatatype()->isVarcharDatatype()) {
      str << "XML_LOAD" << s::obracket << arg1 << s::cbracket;
    }
    else {
      str << callArgs;
    }
  }
}


void trWriteToBufferName(Codestream &str, Ptr<CallArgList> callArgs) {
  if (!callArgs || callArgs->size() != 2)
    throw 999;

  Ptr<FunCallArg> arg2 = *(++callArgs->begin());
  str << arg2 << s::name << ":=" << s::name;
  if (arg2->getDatatype()->isCharVarchar())
    str << "tochar" << s::obracket;
  str << "XML_WRITEBUF";
}

void trWriteToBufferArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> callArgs = call->callArglist;
  if (callArgs->size() != 2)
    throw 999;

  Ptr<FunCallArg> arg1 = *(callArgs->begin());
  Ptr<FunCallArg> arg2 = *(++callArgs->begin());
  str << arg1;
  if (arg2->getDatatype()->isCharVarchar())
      str << s::cbracket;
}

void trWriteToClobName(Codestream &str, Ptr<CallArgList> callArgs) {
  if (!callArgs || callArgs->size() != 2)
    throw 999;

  Ptr<FunCallArg> arg2 = *(++callArgs->begin());
  str << arg2 << s::name << ":=" << s::name << "XML_WRITEBUF";
}

void trWriteToClobArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> callArgs = call->callArglist;
  if (callArgs->size() != 2)
    throw 999;

  Ptr<FunCallArg> arg1 = *(callArgs->begin());
  str << arg1;
}


SemanticTree * checkXmltypeInvariant(Ptr<Sm::Id> &call)
{
  SemanticTree *n = call->semanticNode();
  if (!call || !call->definition() || !call->definition()->toSelfMemberFunction() ||
      !call->definition()->toSelfMemberFunction()->flags.isXmlFunction() ||
      !n || !n->reference() || n->reference()->entity() != call)
    throw 999;

  return n;
}

void trGetClobValueArgs(Ptr<Sm::Id> &call, Codestream &str) {
  SemanticTree *n = checkXmltypeInvariant(call);
  Ptr<IdEntitySmart> ref = n->reference();
  IdEntitySmart trId(next(ref->begin()), ref->end());
  str << trId;
}

void trGetStringValueArgs(Ptr<Sm::Id> &call, Codestream &str) {
  SemanticTree *n = checkXmltypeInvariant(call);
  Ptr<IdEntitySmart> ref = n->reference();
  IdEntitySmart trId(next(ref->begin()), ref->end());
  str << "XML_WRITEBUF" << s::obracket << trId << s::cbracket;
}


void outExtractXmltypeArgs(Codestream &str, IdEntitySmart &ref, std::vector<string>::iterator it, std::vector<string>::iterator begIt) {
  Ptr<Sm::Id> identifier = new Sm::Id(string(*it));
  identifier->setQuoted();
  identifier->setSQuoted();
  if (it == begIt)
    str << "XML_GETNODE" << s::obracket << ref << s::cbracket << s::comma() << identifier;
  else {
    str << "XML_GETBYNAME" << s::obracket;
    outExtractXmltypeArgs(str, ref, prev(it), begIt);
    str << s::cbracket << s::comma() << identifier ;
  }
}


void trExtractXmltypeArgs(Ptr<Sm::Id> &call, Codestream &str) {
  SemanticTree *n = checkXmltypeInvariant(call);
  if (!call->callArglist || call->callArglist->size() != 1)
    throw 999;


  Ptr<IdEntitySmart> ref = n->reference();
  IdEntitySmart trId(next(ref->begin()), ref->end());

  Ptr<PlExpr> expr = call->callArglist->front()->expr();
  string addresNode;
  if (Ptr<RefExpr> x = expr->toSelfRefExpr()) {
    if (x->reference->size() > 1 || !x->refEntity()->definition()->isVarcharDatatype())
      throw 999;
    addresNode = x->refEntity()->toString();
    if (addresNode[0] != '/' || addresNode[1] != '/')
      throw 999;
    addresNode.erase(addresNode.begin(), addresNode.begin()+2);
  }
  else
    throw 999;
  std::vector<string> paths;

  string::size_type pos = 0, oldPos = 0;
  do  {
    pos = addresNode.find_first_of('/', pos);
    paths.push_back(addresNode.substr(oldPos, pos));
    if (pos != string::npos) {
      ++pos;
      oldPos = pos;
      if (oldPos >= addresNode.size())
        pos = string::npos;
    }
  } while (pos != string::npos);

  outExtractXmltypeArgs(str, trId, prev(paths.end()), paths.begin());
}

void XmltypeIntf::initContainers() {
  initFunc(&xmlExtract,             {"SYS", "XMLTYPE"    , "EXTRACT"              }, "XML_GETBYNAME",   new CallarglistTranslatorSimple(trExtractXmltypeArgs));
  initFunc(&xmlGetClobVal,          {"SYS", "XMLTYPE"    , "GETCLOBVAL"           }, "XML_WRITEBUF",    new CallarglistTranslatorSimple(trGetClobValueArgs));
  initFunc(&xmlGetStringVal,        {"SYS", "XMLTYPE"    , "GETSTRINGVAL"         }, "tochar",          new CallarglistTranslatorSimple(trGetStringValueArgs));
  initFunc(&xmlAppendChild,         {"XDB", "DBMS_XMLDOM", "APPENDCHILD"          }, "XML_APPENDCHILD"  );
  initFunc(&xmlGetAttributes,       {"XDB", "DBMS_XMLDOM", "GETATTRIBUTES"        }, "XML_GETATTRIBS"    );
  initFunc(&xmlGetChildNodes,       {"XDB", "DBMS_XMLDOM", "GETCHILDNODES"        }, "XML_GETCHILDREN"  );
  initFunc(&xmlGetDocumentElement,  {"XDB", "DBMS_XMLDOM", "GETDOCUMENTELEMENT"   }, "XML_GETNODE"      );
  initFunc(&xmlGetFirstChild,       {"XDB", "DBMS_XMLDOM", "GETFIRSTCHILD"        }, "XML_GETFIRSTCHILD");
  initFunc(&xmlGetLastChild,        {"XDB", "DBMS_XMLDOM", "GETLASTCHILD"         }, "XML_GETLASTCHILD" );
  initFunc(&xmlGetLength,           {"XDB", "DBMS_XMLDOM", "GETLENGTH"            }, "XML_GETLENGTH"    );
  initFunc(&xmlGetNextSibling,      {"XDB", "DBMS_XMLDOM", "GETNEXTSIBLING"       }, "XML_GETNEXT"      );
  initFunc(&xmlGetNodeName,         {"XDB", "DBMS_XMLDOM", "GETNODENAME"          }, "XML_GETNAME"      );
  initFunc(&xmlGetNodeValue,        {"XDB", "DBMS_XMLDOM", "GETNODEVALUE"         }, "XML_GETVALUE"     );
  initFunc(&xmlGetTagName,          {"XDB", "DBMS_XMLDOM", "GETTAGNAME"           }, "XML_GETNAME"      );
  initFunc(&xmlHasChildNodes,       {"XDB", "DBMS_XMLDOM", "HASCHILDNODES"        }, "XML_HASCHILD"     );
  initFunc(&xmlIsNull,              {"XDB", "DBMS_XMLDOM", "ISNULL"               }, "XML_ISNULL"       );
  initFunc(&xmlItem,                {"XDB", "DBMS_XMLDOM", "ITEM"                 }, "XML_GETITEM"      );
  initFunc(&xmlMakeElement,         {"XDB", "DBMS_XMLDOM", "MAKEELEMENT"          }, "XML_GETNODE"      );
  initFunc(&xmlMakeNode,            {"XDB", "DBMS_XMLDOM", "MAKENODE"             }, "XML_GETNODE"      );
  initFunc(&xmlSetNodeValue,        {"XDB", "DBMS_XMLDOM", "SETNODEVALUE"         }, "XML_SETVALUE"     );
  initFunc(&xmlGetElementsByTagName,{"XDB", "DBMS_XMLDOM", "GETELEMENTSBYTAGNAME" }, "XML_GETBYNAME"    );
  initFunc(&xmlNewDomDocument,      {"XDB", "DBMS_XMLDOM", "NEWDOMDOCUMENT"       }, "XML_GETNODE"      , new CallarglistTranslatorSimple(trNewDomDocumentArgs));
  initFunc(&xmlWriteToBuffer,       {"XDB", "DBMS_XMLDOM", "WRITETOBUFFER"        }, "XML_WRITEBUF"     , new CallarglistTranslatorSimple(trWriteToBufferArgs), trWriteToBufferName);
  initFunc(&xmlWriteToClob,         {"XDB", "DBMS_XMLDOM", "WRITETOCLOB"          }, "XML_WRITEBUF"     , new CallarglistTranslatorSimple(trWriteToClobArgs),   trWriteToClobName  );

  initType(&xmlDomDocument,   {"XDB", "DBMS_XMLDOM", "DOMDOCUMENT"     }, "XMLREF", XmlDomDocument_  );
  initType(&xmlDomElement,    {"XDB", "DBMS_XMLDOM", "DOMELEMENT"      }, "XMLREF", XmlDomElement_   );
  initType(&xmlNamedNodeMAp,  {"XDB", "DBMS_XMLDOM", "DOMNAMEDNODEMAP" }, "XMLREF", XmlDomNodeMap_   );
  initType(&xmlDomNode,       {"XDB", "DBMS_XMLDOM", "DOMNODE"         }, "XMLREF", XmlDomNode_      );
  initType(&xmlDomNodeList,   {"XDB", "DBMS_XMLDOM", "DOMNODELIST"     }, "XMLREF", XmlDomNodeList_  );
}

void XmltypeIntf::initFunc(Sm::Function **func, IdEntitySmart name, string trName, CallarglistTranslator* callarglistTr/* = NULL*/, NameTranslator nameTr/* = NULL*/) {
  name.resolveByModelContext();
  *func = name.definition()->toSelfFunction();
  VEntities::OverloadedFunctions &ovlMap = (*func)->vEntities()->overloadedFunctions;

  for (VEntities::OverloadedFunctions::iterator it = ovlMap.begin(); it != ovlMap.end(); ++it) {
    Sm::Function *ovlFunc = it->first->toSelfFunction();

    ovlFunc->translatedName(trName);
    ovlFunc->flags.setXmlFunction();
    ovlFunc->flags.setElementaryLinterFunction();
    if (callarglistTr)
      ovlFunc->callarglistTranslator = callarglistTr;
    if (nameTr)
      ovlFunc->nameTranslator = nameTr;
  }
}

void XmltypeIntf::initType(Sm::Type::Record **typ, IdEntitySmart name, string/* trName*/, int xmlType) {
  name.resolveByModelContext();
  *typ = name.definition()->toSelfRecord();
  (*typ)->xmlType_ = xmlType;
}

} //Sm
