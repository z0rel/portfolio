#ifndef SRC_BLOBTYPE_H_
#define SRC_BLOBTYPE_H_

#include "resolved_entity.h"

namespace Sm {

class BlobtypeIntf {
public:
  Sm::Function *lobLength       = 0;
  Sm::Function *lobOpen         = 0;
  Sm::Function *lobClose        = 0;
  Sm::Function *lobRead         = 0;
  Sm::Function *lobWrite        = 0;
  Sm::Function *lobWriteAppend  = 0;
  Sm::Function *lobAppend       = 0;
  Sm::Function *lobCopy         = 0;
  Sm::Function *lobTrim         = 0;
  Sm::Function *lobCompare      = 0;
  Sm::Function *lobSubstr       = 0;
  Sm::Function *lobInstr        = 0;
  Sm::Function *lobCreateTemp   = 0;

  BlobtypeIntf() {;}
  ~BlobtypeIntf() {;}

  void initContainers();

private:
  void initFunc(Sm::Function **func, IdEntitySmart name, string trName, CallarglistTranslator *callarglistTr = 0, NameTranslator nameTr = NULL);
  template <typename Functor>
  void applyFuncFlag(Sm::Function *func, Functor functor);
};

} //Sm

#endif /* SRC_BLOBTYPE_H_ */
