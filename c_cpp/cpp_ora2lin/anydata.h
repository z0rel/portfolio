#ifndef ANYDATA_H
#define ANYDATA_H

#include "resolved_entity.h"

namespace Sm {

class Anydata
{
public:
  Sm::Type::MemberFunction *set_varchar2     = 0;
  Sm::Type::MemberFunction *set_number       = 0;
  Sm::Type::MemberFunction *set_date         = 0;
  Sm::Type::MemberFunction *set_char         = 0;
  Sm::Type::MemberFunction *getvarchar2      = 0;
  Sm::Type::MemberFunction *getnumber        = 0;
  Sm::Type::MemberFunction *getdate          = 0;
  Sm::Type::MemberFunction *convertnumber    = 0;
  Sm::Type::MemberFunction *convertvarchar2  = 0;
  Sm::Type::MemberFunction *convertnvarchar2 = 0;
  Sm::Type::MemberFunction *convertdate      = 0;

  Sm::Type::MemberFunction *accessdate       = 0;
  Sm::Type::MemberFunction *accessnumber     = 0;
  Sm::Type::MemberFunction *accessvarchar    = 0;
  Sm::Type::MemberFunction *accessvarchar2   = 0;
  Sm::Type::MemberFunction *accesschar       = 0;
  Sm::Type::MemberFunction *accessnchar      = 0;
  Sm::Type::MemberFunction *accessnvarchar   = 0;

  Sm::Type::MemberFunction *gettype          = 0;

  Sm::Variable *typecodeNumber               = 0;
  Sm::Variable *typecodeVarchar2             = 0;
  Sm::Variable *typecodeVarchar              = 0;
  Sm::Variable *typecodeNchar                = 0;
  Sm::Variable *typecodeNVarchar2            = 0;
  Sm::Variable *typecodeChar                 = 0;
  Sm::Variable *typecodeDate                 = 0;
  Sm::Variable *typecodeObject               = 0;
  Sm::Variable *typecodeRef                  = 0;

  Anydata() {}
  void initAnydataContainers();
private:
  void initMf(Sm::Type::MemberFunction **mf, IdEntitySmart name, std::string trName, CallarglistTranslator::TrFun callarglistTr = 0, Datatype *argtype = 0);
  void initCode(Sm::Variable **mf, IdEntitySmart name, int code);
};

}

#endif // ANYDATA_H
