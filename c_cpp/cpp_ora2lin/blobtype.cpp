#include "blobtype.h"
#include "semantic_function.h"
#include "semantic_object.h"
#include "system_functions.h"

namespace Sm {

/*
Тип BLOB.
VAR b BLOB;
Содержит ссылку на поле blob-типа. Данная ссылка привязана к конкретной записи (record), и может стать невалидной в результате уничтожения записи; в этом случае операции с типом BLOB начнут возвращать ошибку “запись не найдена”.

Переменные данного типа могут быть присвоены только как результат выборки blob-поля.
execute "select id, val from test_blob where id = 1" into id, b;
Переменные типа blob можно делать глобальными и членами курсоров, структур и массивов; они могут быть переданы как аргумент другой процедуре или возвращены как результат работы процедуры.

Переменную типа blob можно присвоить переменной типа clob, в этом случае в переменную clob будет считано всё содержимое; также и наоборот: если присвоить  переменную clob переменной blob – содержимое будет записано по указанной ссылке. Если ссылка не проинициализирована либо стала невалидной — будет выдана ошибка.

Функции работы с типов BLOB:

integer BLOB_SIZE( blob )
Возвращает полную длину blob-записи
где
blob – переменная типа blob

smallint READ_BLOB( blob, offset, buffer, size )
Считывает участок блоба, возвращает количество считанных байт (может быть меньше size, если реальный BLOB короче)
где
blob – переменная типа blob
offset – смещение внутри  blob'а
buffer – переменая типа BYTE
size – размер считываемого куска (не более 4000 байт)

integer READ_BLOB_INT( blob, offset )
Считывает integer-поле из blob с позиции offset

smallint READ_BLOB_SMALLINT( blob, offset )
Считывает smallint-поле из blob с позиции offset

bigint READ_BLOB_BIGINT( blob, offset )
Считывает bigint-поле из blob с позиции offset

numeric READ_BLOB_NUMERIC( blob, offset )
Считывает numeric-поле из blob с позиции offset

real READ_BLOB_REAL( blob, offset )
Считывает real-поле из blob с позиции offset

double READ_BLOB_DOUBLE( blob, offset )
Считывает double-поле из blob с позиции offset

date READ_BLOB_DATE( blob, offset )
Считывает date-поле из blob с позиции offset

varchar READ_BLOB_CHAR( blob, offset )
Считывает строку из blob с позиции offset.
!!! Строка в blob'е должна быть представленна в формате «длина-строка», то есть так, как её записывает ADD_BLOB и WRITE_BLOB !!!

nvarchar READ_BLOB_NCHAR( blob, offset )
Считывает unicode строку из blob с позиции offset.
!!! Строка в blob'е должна быть представленна в формате «длина-строка», то есть так, как её записывает ADD_BLOB и WRITE_BLOB !!!

integer ADD_BLOB( blob, value[, size] )
Дописывает в конец blob значение переменной value, возвращает количество записанных байт
где
blob – переменная типа blob
value – переменая для записи простого типа (недопустимы массивы и структуры)
size – размер записываемого куска (не более 4000 байт) для переменных типа BYTE

integer WRITE_BLOB( blob, offset, value[, size] )
Записывает в blob в позицию offset значение переменной value, возвращает количество записанных байт. Если offset больше существующего размера blob'а — участок до offset заполняется нулями
где
blob – переменная типа blob
offset – смещение внутри  blob'а
value – переменая для записи простого типа (недопустимы массивы и структуры)
size – размер записываемого куска (не более 4000 байт) для переменных типа BYTE

TRIM_BLOB( blob, size )
Обрезает существующий blob до длины size. Если текущая длина <=  size — никаких действий не производится, ошибка не генерируется.
где
blob – переменная типа blob
size – новый размер куска

integer INSTR( blob, substring[, offset[, index ] ] )
См. существующее описание INSTR; стало допустимо искать не только в строке, но и в блобе

bool COMPARE_BLOB( blob1, offset1, blob2, offset2, size )
Сравнивает участки 2х блоб-переменных. Возвращает TRUE при одинаковых отрезках. Могут быть участками одного и того же блоба.
где
blob1, blob2 – сравниваемые переменные типа blob
offset1, offset2 – смещения внутри blob'ов соответственно
size – размер сравниваемого куска
 */

#define ARGS_TR(tr) new CallarglistTranslatorSimple(tr)

static
size_t checkGetArgs(Ptr<FunCallArg> argArray[], size_t arrSize, Ptr<CallArgList> args) {
  sAssert(args->size() > arrSize);
  size_t i = 0;
  for (CallArgList::iterator it = args->begin(); it != args->end(); ++it, ++i) {
    argArray[i] = *it;
  }
  return i;
}

static
void trLobGetLengthName(Sm::Codestream &str, Ptr<Sm::CallArgList> /*args*/) {
  str << ((str.isProc()) ? "BLOB_SIZE" : "LENBLOB");
}

static
void trLobReadArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  Ptr<FunCallArg> arg[4];
  checkGetArgs(arg, 4, args);

  str << arg[0] << s::comma()
      << arg[2] << s::comma()
      << arg[3] << s::comma()
      << arg[1];
}

static
void trLobWriteArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  Ptr<FunCallArg> arg[4];
  checkGetArgs(arg, 4, args);

  str << arg[0] << s::comma()
      << arg[2] << s::comma()
      << arg[3] << s::comma()
      << arg[1];
}

static
void trLobWriteAppendArgs(Ptr<Sm::Id> &call, Codestream &str) {
  Ptr<CallArgList> args = call->callArglist;
  Ptr<FunCallArg> arg[3];
  checkGetArgs(arg, 3, args);

  str << arg[0] << s::comma()
      << arg[2] << s::comma()
      << arg[1];
}

static
void trLobAppendArgs(Ptr<Sm::Id> &call, Codestream &str) {
  // TODO: Переделать трансляцию под синтаксис функции ядра
  Ptr<CallArgList> args = call->callArglist;
  Ptr<FunCallArg> arg[2];
  checkGetArgs(arg, 2, args);

  str << arg[0] << s::comma()
      << arg[1] << s::comma()
      << "BLOB_SIZE" << s::obracket << arg[1] << s::cbracket;
}

static
void trArgsCopyComp(Codestream &str, Ptr<Sm::CallArgList> args) {
  Ptr<FunCallArg> arg[5];
  checkGetArgs(arg, 5, args);

  str << arg[0] << s::comma();
  if (arg[3])
    str << arg[3] << s::comma();
  else
    str << 1 << s::comma();
  str << arg[1] << s::comma();
  if (arg[4])
    str << arg[4] << s::comma();
  else
    str << 1 << s::comma();
  if (arg[2])
    str << arg[2];
  else
    str << 2147483647; //????
}

static
void trLobCopyArgs(Ptr<Sm::Id> &call, Codestream &str) {
  trArgsCopyComp(str, call->callArglist);
}

static
void trLobSubstrArgs(Ptr<Sm::Id> &call, Codestream &str) {
  // TODO: Нужно перенести lValue в параметры
  str << call->callArglist;
}

static
void trLobCompareName(Codestream &str, Ptr<Sm::CallArgList> args) {
  str << s::obracket << "EIF [" << "COMPARE_BLOB"
      << s::obracket;
  trArgsCopyComp(str, args);
  str << s::cbracket << "]"
      << s::name << 0 << " ELSE " << 1 << s::cbracket;

}

static
void trLobInstrName(Codestream &str, Ptr<Sm::CallArgList> args) {
  str << s::obracket << "EIF [" << "INSTR"
      << s::obracket << args << s::cbracket << "]"
      << s::name << 1 << " ELSE " << 0 << s::cbracket;
}

static
void trLobCreateTempName(Codestream &str, Ptr<Sm::CallArgList> args) {
  Ptr<FunCallArg> arg1 = *args->begin();
  if (arg1->getDatatype()->isBlobDatatype()) {
    cout << "error: trLobCreateTempName: blob to clob is unimplemented " << arg1->getLLoc() << endl;
  }
  str << arg1 << " := " << "TOCLOB" << s::obracket << "\"\"" << s::cbracket;
}

static
void trLobNullArgs(Ptr<Sm::Id> &/*call*/, Codestream &/*str*/) {
  // Do nothing
}

void BlobtypeIntf::initContainers() {
  initFunc(&lobLength,      {"SYS", "DBMS_LOB", "GETLENGTH"      }, "LENBLOB",      NULL,                         &trLobGetLengthName);
  initFunc(&lobOpen,        {"SYS", "DBMS_LOB", "OPEN"           }, "// OPEN_BLOB", NULL,                          NULL);
  initFunc(&lobClose,       {"SYS", "DBMS_LOB", "CLOSE"          }, "// CLOSE_BLOB",NULL,                          NULL);
  initFunc(&lobRead,        {"SYS", "DBMS_LOB", "READ"           }, "READ_BLOB",    ARGS_TR(trLobReadArgs),        NULL);
  initFunc(&lobWrite,       {"SYS", "DBMS_LOB", "WRITE"          }, "WRITE_BLOB",   ARGS_TR(trLobWriteArgs),       NULL);
  initFunc(&lobWriteAppend, {"SYS", "DBMS_LOB", "WRITEAPPEND"    }, "ADD_BLOB",     ARGS_TR(trLobWriteAppendArgs), NULL);
  initFunc(&lobAppend,      {"SYS", "DBMS_LOB", "APPEND"         }, "ADD_BLOB",     ARGS_TR(trLobAppendArgs),      NULL);
  initFunc(&lobCopy,        {"SYS", "DBMS_LOB", "COPY"           }, "COPY_BLOB???", ARGS_TR(trLobCopyArgs),        NULL);
  initFunc(&lobTrim,        {"SYS", "DBMS_LOB", "TRIM"           }, "TRIM_BLOB",    NULL,                          NULL);
  initFunc(&lobCompare,     {"SYS", "DBMS_LOB", "COMPARE"        }, "COMPARE_BLOB", ARGS_TR(trLobNullArgs),       &trLobCompareName);
  initFunc(&lobSubstr,      {"SYS", "DBMS_LOB", "SUBSTR"         }, "SUBSTR???",    ARGS_TR(trLobSubstrArgs),      NULL);
  initFunc(&lobInstr,       {"SYS", "DBMS_LOB", "INSTR"          }, "INSTR",        ARGS_TR(trLobNullArgs),       &trLobInstrName);
  initFunc(&lobCreateTemp,  {"SYS", "DBMS_LOB", "CREATETEMPORARY"}, "CREATETEMP",   ARGS_TR(trLobNullArgs),       &trLobCreateTempName);

  auto noBraces = [](Sm::Function *f) { f->clrBracesOutput(); };
  applyFuncFlag(lobCompare, noBraces);
  applyFuncFlag(lobInstr, noBraces);
  applyFuncFlag(lobCreateTemp, noBraces);
}

void BlobtypeIntf::initFunc(Sm::Function **func, IdEntitySmart name, string trName,
                            CallarglistTranslator* callarglistTr/* = NULL*/, NameTranslator nameTr/* = NULL*/) {
  name.resolveByModelContext();
  *func = name.definition()->toSelfFunction();
  VEntities::OverloadedFunctions &ovlMap = (*func)->vEntities()->overloadedFunctions;

  for (VEntities::OverloadedFunctions::iterator it = ovlMap.begin(); it != ovlMap.end(); ++it) {
    Sm::Function *ovlFunc = it->first->toSelfFunction();

    ovlFunc->translatedName(trName);
    ovlFunc->flags.setElementaryLinterFunction();
    if (callarglistTr)
      ovlFunc->callarglistTranslator = callarglistTr;
    if (nameTr)
      ovlFunc->nameTranslator = nameTr;
  }
}

template <typename Functor>
void BlobtypeIntf::applyFuncFlag(Sm::Function *func, Functor functor) {
  VEntities::OverloadedFunctions &ovlMap = func->vEntities()->overloadedFunctions;
  for (VEntities::OverloadedFunctions::iterator it = ovlMap.begin(); it != ovlMap.end(); ++it) {
    functor(it->first->toSelfFunction());
  }
}

} //Sm
