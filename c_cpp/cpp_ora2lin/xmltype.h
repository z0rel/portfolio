#ifndef SRC_XMLTYPE_H_
#define SRC_XMLTYPE_H_

#include "resolved_entity.h"

namespace Sm {

class XmltypeIntf {
public:
  enum XmlDomType {
     XmlDomDocument_ = 1,
     XmlDomElement_,
     XmlDomNodeMap_,
     XmlDomNode_,
     XmlDomNodeList_
  };

  Sm::Function *xmlExtract              = 0;
  Sm::Function *xmlGetClobVal           = 0;
  Sm::Function *xmlGetStringVal         = 0;

  Sm::Function *xmlAppendChild          = 0;
  Sm::Function *xmlGetAttributes        = 0;
  Sm::Function *xmlGetChildNodes        = 0;
  Sm::Function *xmlGetDocumentElement   = 0;
  Sm::Function *xmlGetElementsByTagName = 0;
  Sm::Function *xmlGetFirstChild        = 0;
  Sm::Function *xmlGetLastChild         = 0;
  Sm::Function *xmlGetLength            = 0;
  Sm::Function *xmlGetNextSibling       = 0;
  Sm::Function *xmlGetNodeName          = 0;
  Sm::Function *xmlGetNodeValue         = 0;
  Sm::Function *xmlGetTagName           = 0;
  Sm::Function *xmlHasChildNodes        = 0;
  Sm::Function *xmlIsNull               = 0;
  Sm::Function *xmlItem                 = 0;
  Sm::Function *xmlMakeElement          = 0;
  Sm::Function *xmlMakeNode             = 0;
  Sm::Function *xmlNewDomDocument       = 0;
  Sm::Function *xmlSetNodeValue         = 0;
  Sm::Function *xmlWriteToBuffer        = 0;
  Sm::Function *xmlWriteToClob          = 0;

  Sm::Type::Record *xmlDomDocument      = 0;
  Sm::Type::Record *xmlDomElement       = 0;
  Sm::Type::Record *xmlNamedNodeMAp     = 0;
  Sm::Type::Record *xmlDomNode          = 0;
  Sm::Type::Record *xmlDomNodeList      = 0;

  XmltypeIntf() {;}
  ~XmltypeIntf() {;}

  void initContainers();

private:
  void initFunc(Sm::Function **func, IdEntitySmart name, string trName, CallarglistTranslator *callarglistTr = 0, NameTranslator nameTr = NULL);
  void initType(Sm::Type::Record **typ, IdEntitySmart name, string trName, int xmlType);
};

} //Sm

#endif /* SRC_XMLTYPE_H_ */
