# UNCOMMENT DEBUG

# проверка на == указателя 0:
# if not d.extractPointer(value):
#

import os
import sys
import imp

import dumper_utils
from dumper_utils import exceptionInfo, logger
import ora2lin_lloc
import ora2lin_cast

import string
import re

import types
import gdb
import imp


from dumper import *
from stdtypes import *

# imp.reload(ora2lin_lloc)

global AutomaticFormat

AutomaticFormat = Utf8StringFormat

globalTraceEnable = False

REVERSE_ITERATORS_TAIL_LIST = False

class CallDeep:
  deepCall = 0

  @staticmethod
  def fmtPrefix(dfltVal = 1):
    return "".join([" " for i in range(dfltVal, CallDeep.deepCall)])

  @staticmethod
  def printMessage(message):
    if globalTraceEnable:
      logger.info("{0}{1} {2}".format(CallDeep.fmtPrefix(), message, CallDeep.deepCall))


  @staticmethod
  def inDeep():
    CallDeep.deepCall += 1
    CallDeep.printMessage("enter")

  @staticmethod
  def outDeep():
    CallDeep.printMessage("exit")
    CallDeep.deepCall -= 1


def tracerDecorate(fun):
  def wrapper(*pargs, **kargs):
    if globalTraceEnable:
      logger.info("{0}trace: {1}()".format(CallDeep.fmtPrefix(0), fun.__name__))
    CallDeep.inDeep()
    res = fun(*pargs, **kargs)
    CallDeep.outDeep()
    return res


  return wrapper


class ExceptionDecorate(object):
  def __init__(self, fun):
    self.fun = fun
    self.funname = fun.__name__
    self.reloaded = False

  def changePutPair(self, d):
    if not hasattr(d, "oldPutPair"):
      d.oldPutPair = d.putPair
      d.putPair    = types.MethodType(putDumperPair, d)
      logger.info("a = " + repr(d.oldPutPair))
      logger.info("b = " + repr(d.putPair))
    elif self.reloaded:
      d.putPair  = types.MethodType(putDumperPair, d)
      logger.info("b = " + repr(d.putPair))

    self.reloaded = False

  def needReload(self):
    i = dumper_utils.ora2lin_utils.StatTree(os.path.split(os.path.abspath(__file__))[0])
    i.scanPath()
    loaded = i.loadSavedFromDb()
    isNeedReload = (i != loaded)

    self.reloaded = isNeedReload
    if (isNeedReload):
      logger.info("--- dumper reloaded ---")
      i.saveToDb()

    return isNeedReload


  def callInfo(self, *pargs, **kargs):
    try:
      if globalTraceEnable:
        logger.info("{0}called {1} {2}".format(CallDeep.fmtPrefix(0), self.funname, repr(pargs[1])))
    except:
      logger.info("--- ERROR in callInfo ---")
      exceptionInfo(self.funname)

  def serviceUpdate(self, d):
    self.changePutPair(d)

  def __call__(self, *pargs, **kargs):
    try:
      self.callInfo(*pargs, **kargs)
      self.serviceUpdate(pargs[0])
      return self.fun(*pargs, **kargs)
    except RuntimeError as i:
      exceptionInfo(self.funname)
      raise i
    except:
      logger.info("--- ERROR in ExceptionDecorate ---")
      exceptionInfo(self.funname)


class TryFormatDecorate(ExceptionDecorate):
  def __init__(self, fun):
    ExceptionDecorate.__init__(self, fun)

  def __call__(self, *pargs, **kargs):
    try:
      self.callInfo(*pargs, **kargs)
      self.serviceUpdate(pargs[0])
      return self.fun(*pargs, **kargs)
    except:
      logger.info("--- ERROR in TryFormatDecorate ---")
      exceptionInfo(self.funname)


@ExceptionDecorate
def putDumperPair(self, pair, index = None):
   if self.pairData.useKeyAndValue:
     first  = pair["key"]
     second = pair["value"]
   else:
     first  = pair["first"]
     second = pair["second"]

   stdstrT = self.lookupType("std::string").strip_typedefs().unqualified()
   typeStr = first.type.strip_typedefs().unqualified()

   if typeStr == stdstrT:
     putStdStringHeader(self, first)
     self.putEmptyValue()
     self.putNumChild(1)
     self.putField("iname", self.currentIName)
     if self.isExpanded():
       with Children(self):
         self.putSubItem("v", second)
     return
   elif str(typeStr) in {"const Sm::ResolvedEntity *", "Sm::ResolvedEntity *"}:
     ctx = SmartPtrContext(self, first)
     putStr(self, ctx.getPtrHeader())
     self.putNumChild(1)
     if self.isExpanded():
       with Children(self):
         self.putSubItem("first" , first)
         self.putSubItem("second", second)
   else:
     self.oldPutPair(pair,index)



class Ora2linGdbMemoryError(Exception) : pass

def getTextFromStdStringBase(d, value):
  #logger.info("============> {0}".format(value.__dict__))
  data = value["_M_dataplus"]["_M_p"]
  sizePtr = data.cast(d.sizetType().pointer())
  try:
    size = int(sizePtr[-3])
    return str(data.string('utf-8','strict',size));
  except gdb.MemoryError:
    pass
  raise Ora2linGdbMemoryError()


def getTextFromStdString(d, value):
  #logger.info("============> {0}".format(value.__dict__))
  try:
    return getTextFromStdStringBase(d, value)
  except Ora2linGdbMemoryError:
    return "<invalid str>"



def getEncodingType(displayFormat = AutomaticFormat):
    if displayFormat == Latin1StringFormat or displayFormat == SeparateLatin1StringFormat:
        return "latin1"
    else:
        return "utf8"


def putStdString(d, data, size, limit):
  bytelen = size
  elided, shown = bytelen, limit
  mem = d.readMemory(data, shown)

  displayType = DisplayLatin1String

  displayFormat = AutomaticFormat
  d.putValue(mem, getEncodingType(displayFormat))

  if displayFormat == Latin1StringFormat or displayFormat == Utf8StringFormat:
      d.putDisplay(displayFormat, StopDisplay)
  elif displayFormat == SeparateLatin1StringFormat or displayFormat == SeparateUtf8StringFormat:
      d.putField("editformat", displayType)
      elided, shown = d.computeLimit(bytelen, 100000)
      d.putField("editvalue", d.readMemory(data, shown))


@tracerDecorate
def putStdStringHeader(d, value):
   data = value["_M_dataplus"]["_M_p"]

   # We can't lookup the std::string::_Rep type without crashing LLDB,
   # so hard-code assumption on member position
   # struct { size_type _M_length, size_type _M_capacity, int _M_refcount; }
   sizePtr = data.cast(d.sizetType().pointer())
   size = int(sizePtr[-3])
   alloc = int(sizePtr[-2])
   refcount = int(sizePtr[-1]) & 0xffffffff

   d.check(refcount >= -1) # Can be -1 accoring to docs.
   d.check(0 <= size and size <= alloc and alloc <= 100*1000*1000)

   elided, shown = d.computeLimit(size, d.displayStringLimit)
   putStdString(d, sizePtr, elided, shown)


@tracerDecorate
def putTextHead(d, textNode):
  textNodePtr = d.extractPointer(textNode)
  if textNodePtr:
    putStdStringHeader(d, textNode.dereference())


@tracerDecorate
def putPointer(d, value, type, displayFormat = AutomaticFormat):
  d.putValue(d.hexencode(str(value)), "utf8") #Hex2EncodedUtf8WithoutQuotes)
  d.putType(type)

  if displayFormat == Latin1StringFormat or displayFormat == Utf8StringFormat:
    d.putDisplay(displayFormat, StopDisplay)
  elif displayFormat == SeparateLatin1StringFormat or displayFormat == SeparateUtf8StringFormat:
    d.putField("editformat", displayType)
    elided, shown = d.computeLimit(bytelen, 100000)
    d.putField("editvalue", d.readMemory(data, shown))


def getIdHead(d, value):
  try:
    return getTextFromStdStringBase(d, value["text"])
  except Ora2linGdbMemoryError:
    pass
  try:
    return getTextFromStdStringBase(d, value["normalizedStr"])
  except Ora2linGdbMemoryError:
    return "<invalid str>"


def getIdHeadUnwrapped(d, value):
  return getIdHead(d, value)
  idDefNode = value["idDefinition_"]

  p = d.extractPointer(idDefNode)
  if p:
    ctx = SmartPtrContext(d, idDefNode)
    return ctx.getPtrHeader()

  return getIdHead(d, value)


@tracerDecorate
def extractNameTextNode(d, node, nameField):
  fieldName = node[nameField]
  ptrIdType = d.lookupType("smart::Ptr<Sm::Id>")
  idType = d.lookupType("Sm::Id")
  fieldNameContent = fieldName.cast(ptrIdType)["smartPtr"]
  return fieldNameContent.dereference().cast(idType)["text"]


@tracerDecorate
def getNameDatatypeGrammarBase(d, value, name, datatype):
  nameValue     = value[name]
  nameCtx     = PtrCleanIdContext(d, nameValue)

  llocHead = getFLocationNodeHead(d, value["lloc"])

  if datatype:
    datatypeValue = value[datatype]
    datatypeCtx = PtrCleanIdContext(d, datatypeValue)
    datatypeCtx.datatypeLoc = False
    return "{0} {1} // {2}".format(nameCtx.getPtrHeader(), datatypeCtx.getPtrHeader(), llocHead)
  else:
    return "{0} // {1}".format(nameCtx.getPtrHeader(), llocHead)


@tracerDecorate
def getMemberVariableHead(d, value):
  return getNameDatatypeGrammarBase(d, value, "name", "datatype")


@tracerDecorate
def getVariableHead(d, value):
  return getNameDatatypeGrammarBase(d, value, "name", "datatype")


@tracerDecorate
def getSelectedFieldHead(d, value):
  return getNameDatatypeGrammarBase(d, value, "fieldName", None)


@tracerDecorate
def getFLocationNodeHead(d, locValue):
  fnameNode = locValue["file"]
  locNode   = locValue["loc"]
  if not d.extractPointer(fnameNode):
    dumperLLoc = ora2lin_lloc.LLoc(d, locNode)
  else:
    dumperLLoc = ora2lin_lloc.LLoc(d, locNode, getTextFromStdString(d, fnameNode.dereference()))

  return str(dumperLLoc)


@tracerDecorate
def dumpFLocationNode(d, value, locValue, prefix = "", outPlain = True):
  head = prefix + getFLocationNodeHead(d, locValue)
  putStr(d, head)
  if outPlain:
    d.putPlainChildren(value)



class IdListBase:
  def idAttr(i):
      return "id" + str(i)


  def locAttr(i):
      return "id" + str(i) + "_lloc"


  def setDefault(self, attr):
     self.__dict__[attr]     = ""
     self.__dict__[attr + "_lloc"] = ora2lin_lloc.LLoc()


  def setAttrFromValue(self, d, attr, idNodeSmartPtr, idType):
    ptrExtracted = None
    try:
      ptrExtracted = d.extractPointer(idNodeSmartPtr)
    except gdb.MemoryError:
      pass

    if not ptrExtracted:
      self.setDefault(attr)
      return


    idNode = idNodeSmartPtr.dereference().cast(idType)
    s = getIdHeadUnwrapped(d, idNode)

    if not s:
      s = ""

    callArgListNode = idNode["callArglist"]["smartPtr"]
    if d.extractPointer(callArgListNode):
      s += "()"

    self.__dict__[attr] = s

    lnode = idNode["lloc"]
    self.__dict__[attr + "_lloc"] = ora2lin_lloc.LLoc(d, lnode["loc"], ora2lin_lloc.LLoc.getFilename(d, lnode))


  def setAttr(self, d, attr, ptr, ptrIdType):
    if not ptr:
      self.setDefault(attr)
      return

    gvPtrId = gdb.Value(ptr)

    try:
      val = gvPtrId.cast(ptrIdType)
    except gdb.error as err:
      logger.info("exception type  = {0} {1}".format(str(type(ptrIdType)), str(ptrIdType)))
      svT = str(gvPtrId.dynamic_type)
      svV = str(gvPtrId)
      logger.info("exception value = {0} {1} {2}".format(str(type(gvPtrId)), svV, svT))
      if (svT == "long long"):
        logger.info("value hex = {0}".format(hex(int(svV))))
      raise
    idType = d.templateArgument(ptrIdType, 0)
    self.setAttrFromValue(d, attr, val, idType)


  class LLocCtx:
    def  __init__(self):
      self.fname   = ""
      self.line    = 0
      self.colList = []

    def update(self, lloc):
      self.fname    = lloc.fname
      self.line     = lloc.ibLin
      self.colList += [ lloc.ibCol ]

    def __str__(self):
      if not self.colList:
        return ""
      s = " "
      if self.fname:
        s += "%s:" % self.fname
      s += "%i" % self.line
      if len(self.colList) == 1:
        s += ",%i" % self.colList[0]
      else:
        isNotFirst = False
        s += "["
        for i in self.colList:
          if isNotFirst:
            s += ","
          else:
            isNotFirst = True
          s += str(i)
        s += "]"
      return s


class PtrId(IdListBase):
  def __init__(self, d, value):
    ptrIdType = d.lookupType("Sm::Id*")
    self.setAttr(d, "id0", value["smartPtr"], ptrIdType)


  def baseStr(self):
    s = ""
    if self.id0:
      s += self.id0
    return s


  def llocStr(self):
    llocCtx = IdListBase.LLocCtx()
    if self.id0:
      llocCtx.update(self.id0_lloc)
    return str(llocCtx)


  def __str__(self):
    return self.baseStr() + self.llocStr()


class Id2(IdListBase):
  def __init__(self, d, value):
    ptrIdType = d.lookupType("Sm::Id*")
    id0 = d.extractPointer(value["id"])
    id1 = d.extractPointer(value["id"], offset=ptrIdType.sizeof)

    self.setAttr(d, "id0", id0, ptrIdType)
    self.setAttr(d, "id1", id1, ptrIdType)

  def baseStr(self):
    s = ""
    if self.id1:
      s += self.id1 + "."
    if self.id0:
      s += self.id0
    return s

  def llocStr(self):
    llocCtx = IdListBase.LLocCtx()
    if self.id1:
      llocCtx.update(self.id1_lloc)
    if self.id0:
      llocCtx.update(self.id0_lloc)
    return str(llocCtx)


  def __str__(self):
    return self.baseStr() + self.llocStr()


class VectorBase:
  def __init__(self, d, value):
    impl   = value["_M_impl"]

    alloc  = impl["_M_end_of_storage"]
    start  = impl["_M_start"]
    finish = impl["_M_finish"]
    self.size   = toInteger(finish - start)

    self.invalid = False
    if (not (0 <= self.size and self.size <= 1000 * 1000 * 1000) or
        not (finish <= alloc)
       ):
      self.invalid = True
    else:
      d.checkPointer(start)
      d.checkPointer(finish)
      d.checkPointer(alloc)

      self.addrBase = toInteger(start)


class IdEntity(IdListBase, VectorBase):
  def __init__(self, d, value):
    VectorBase.__init__(self, d, value)
    if self.invalid:
      return
    type   = d.lookupType("smart::Ptr<Sm::Id>")
    idType = d.templateArgument(type, 0)
    for i in range(0, self.size):
      address = self.addrBase + i * type.sizeof
      val = gdb.Value(address).cast(type.pointer()).dereference()
      self.setAttrFromValue(d, IdListBase.idAttr(i), val["smartPtr"], idType)

  def baseStr(self):
    if self.invalid:
      return "<invalid ids>"
    s = ""
    isNotFirst = False
    if self.size:
      for i in range(self.size-1, -1, -1):
        if isNotFirst:
          s += "."
        else:
          isNotFirst = True
        s += self.__dict__[IdListBase.idAttr(i)]
    return s

  def llocStr(self):
    if self.invalid:
      return "<invalid ids>"
    llocCtx = IdListBase.LLocCtx()
    if self.size:
      for i in range(self.size-1, -1, -1):
        llocCtx.update(self.__dict__[IdListBase.locAttr(i)])
    return str(llocCtx)

  def __str__(self):
    if self.invalid:
      return "<invalid ids>"
    return self.baseStr() + self.llocStr()


@tracerDecorate
def putStr(d, s):
  strGdbVal, l = gdb.Value(s), len(s)
  putStdString(d, strGdbVal, l, l)


def putSmartPtrNode(d, value):
  d.putNumChild(1)
  if d.isExpanded():
    with Children(d):
      d.putSubItem("smartPtr", value)


def putIdEntityNode(d, value):
  #DEBUG
  idEntity = IdEntity(d, value)
  putStr(d, str(idEntity))

  d.putPlainChildren(value)


@tracerDecorate
def putPtrIdEntityNode(d, value):
  putStr(d, str(IdEntity(d, value)))
  putSmartPtrNode(d, value)


def putId2Node(d, value):
  putStr(d, str(Id2(d, value)))
  d.putPlainChildren(value)


@tracerDecorate
def putPtrId2Node(d, value):
  putStr(d, str(Id2(d, value)))
  putSmartPtrNode(d, value)


@tracerDecorate
def getDatatypeHeader(d, value, needLoc = True):
  tidNode = value["tid"]["smartPtr"]
  if not d.extractPointer(value):
    return ""

  base = IdEntity(d, tidNode)

  locStr = lambda : " %s" % base.llocStr() if needLoc else ""

  prec  = str(value["precision"])
  scale = str(value["scale"])
  if prec == "0":
    if scale == "65535":
      return base.baseStr() + locStr()
    else:
      return "%s(0,%s)%s" % (base.baseStr(), scale, locStr())
  else:
    if scale == "65535":
      return "%s(%s)%s" % (base.baseStr(), prec, locStr())
    else:
      return "%s(%s,%s)%s" % (base.baseStr(), prec, scale, locStr())
  logger.info(str(prec))
  return str(base)


class FunctionArglistTypes(IdEntity):
  def __init__(self, d, value):
    self.strList = ""
    if not d.extractPointer(value):
      return
    VectorBase.__init__(self, d, value)
    if self.invalid:
      return
    type   = d.lookupType("smart::Ptr<Sm::FunctionArgument>")
    argType = d.templateArgument(type, 0)
    isNotFirst = False
    s = ""
    for i in range(0, self.size):
      address = self.addrBase + i * type.sizeof
      val = gdb.Value(address).cast(type.pointer()).dereference()
      if isNotFirst:
        s += ","
      else:
        isNotFirst = True
      if d.extractPointer(val["smartPtr"]):
        s += getDatatypeHeader(d, val["smartPtr"]["datatype"]["smartPtr"], False)


    self.strList = s

  def __str__(self):
    if self.invalid:
      return "<invalid>"
    return self.strList


@tracerDecorate
def getFunctionHeader(d, value):
   name    = Id2(d, value["name"]["smartPtr"])
   rettype = getDatatypeHeader(d, value["rettype"]["smartPtr"], False)
   types = FunctionArglistTypes(d, value["arglist"]["smartPtr"])
   return name.baseStr() + "(" + types.strList + ") " + rettype + name.llocStr()

@tracerDecorate
def getViewHeader(d, value):
   name    = Id2(d, value["name"]["smartPtr"])
   return name.baseStr() + " // VIEW " + name.llocStr()


@tracerDecorate
def getFunctionArgumentHeader(d, value):
   name    = PtrId(d, value["name"])
   argtype = getDatatypeHeader(d, value["datatype"]["smartPtr"], False)
   return name.baseStr() + " " + argtype


@tracerDecorate
def getFieldDefinitionHeader(d, value):
   name    = PtrId(d, value["name"])
   argtype = getDatatypeHeader(d, value["datatype"]["smartPtr"], False)
   return name.baseStr() + " " + argtype + "/" + name.llocStr()



semanticTreeMappedNametypes = {
  "EMPTY"          : "",
  "NEW_LEVEL"      : "LVL ",
  "REFERENCE"      : "REF ",
  "EXTDB_REFERENCE": "EXTDB REF ",
  "DECLARATION"    : "DECL ",
  "DEFINITION"     : "DECL ",

  "Sm::SemanticNameType::EMPTY"          : "",
  "Sm::SemanticNameType::NEW_LEVEL"      : "LVL ",
  "Sm::SemanticNameType::REFERENCE"      : "REF ",
  "Sm::SemanticNameType::EXTDB_REFERENCE": "EXTDB REF ",
  "Sm::SemanticNameType::DECLARATION"    : "DECL ",
  "Sm::SemanticNameType::DEFINITION"     : "DECL ",
}


@tracerDecorate
def getSemanticTreeHeader(d, value):
  nameNode = value["referenceName_"]["smartPtr"]
  if not d.extractPointer(nameNode):
    cat = str(value["cathegory"]).replace("Sm::", "")
    nametype = str(value["nametype"]).replace("Sm::SemanticTree::", "")
    return semanticTreeMappedNametypes[nametype] + cat
  name = IdEntity(d, nameNode)
  return "{0} /{1}".format(name.baseStr(), name.llocStr().strip())


@tracerDecorate
def getRefExprHeader(d, value):
  name = IdEntity(d, value["reference"]["smartPtr"])
  return "{0} /{1}".format(name.baseStr(), name.llocStr().strip())


def putLevelResolvedNamespace(d, value):
  semanticNode = value["semanticLevel"]
  if d.extractPointer(semanticNode):
    putStr(d, getSemanticTreeHeader(d, semanticNode))
  d.putPlainChildren(value)


def putResolvedEntity(d, resolvedEntityValue):
  logger.info("!!!!!!1")
  ptr     = resolvedEntityValue
  dyntype = ptr.dynamic_type
  value   = ptr.cast(dyntype)
  # value   = rttiPtr.dereference()
  logger.info(str(dyntype))
  if str(dyntype) == "Sm::table::FieldDefinition":
    putStr(d, getFieldDefinitionHeader(d, value))
  d.putPlainChildren(value)


def putResolvedEntityLoc(d, resolvedEntityValue):
  logger.info("!!!!!!1")
  ptr     = resolvedEntityValue
  dyntype = ptr.dynamic_type
  value   = ptr.cast(dyntype)
  logger.info(str(dyntype))
  # value   = rttiPtr.dereference()
  if str(dyntype) == "Sm::table::FieldDefinition":
    putStr(d, getFieldDefinitionHeader(d, value))
  else:
    dumpFLocationNode(d, resolvedEntityValue, resolvedEntityValue["lloc"])
  d.putPlainChildren(value)


@tracerDecorate
def putIdChilds(d, value):
  d.putPlainChildren(value)


def getUIntValue(d, value):
  if not value.address:
    return 0
  else:
    return d.extractUInt(value.address)

def getIntValue(d, value):
  if not value.address:
    return 0
  else:
    return d.extractInt(value.address)

def getUShortValue(d, value):
  if not value.address:
    return 0
  else:
    return d.extractUShort(value.address)

def getShortValue(d, value):
  if not value.address:
    return 0
  else:
    return d.extractShort(value.address)


def getIndentingActionHead(d, value):
  childs     = VectorBase(d, value["childs"])
  return "tab:{tab} indt:{isIndented} chlds:{childs} aid:{aid}".format(
      isIndented = "1" if getIntValue(d, value["indented"]) else "0",
      childs     = childs.size,
      tab        = getIntValue(d, value["tabsize"]),
      aid        = getIntValue(d, value["aid"])
  )

@tracerDecorate
def getIndentingBracketsHead(d, value):
  cbracketCol = getIntValue(d, value["cbracketCol"])
  obracketCol = getIntValue(d, value["obracketCol"])
  return "({0},{1}) {2}".format(obracketCol, cbracketCol, getIndentingActionHead(d, value))


@tracerDecorate
def getIndentingSubconstructHead(d, value):
    return "col:{col} {flags}".format(
      col   = getIntValue(d, value["col"]),
      flags = getIndentingActionHead(d, value)
  )


def getIndentedLineContextHeader(d, value):
  col = getIntValue(d, value["lineCurrentColumn"])
  tab = getIntValue(d, value["tabBeginOfLine_"])
  return "{0}|{1}".format(tab,col)


def getIndentedPosHead(d, value):
  tabsize = getIntValue(d, value["tabsize"])
  absPos  = getIntValue(d, value["absolutePos"])
  return "col:{absCol} tab:{tab} ".format(
      absCol = absPos,
      tab    = tabsize
  )




def getTabSpacerHead(d, value):
  tabsize = getIntValue(d, value["tabsize_"])
  return "# => {0}".format(tabsize if tabsize else "<default>")



def getIndentingContextHead(d, value):
  tabsize = getShortValue(d, value["tab_"])
  absPos  = getIntValue(d, value["absoluteColumnPos"])
  flags   = getUShortValue(d, value["flags"]["v"])
  strFlags = []
  if flags & (1 << 0):
    strFlags.append("skipTail")
  if flags & (1 << 1):
    strFlags.append("indentOnlyLongChild")
  if flags & (1 << 2):
    strFlags.append("childsOffset")

  base = "col:{col} tab:{tab}".format(
      col = absPos,
      tab = tabsize
  )
  if strFlags:
    base = "{0} {{{1}}}".format(base, " ".join(strFlags))

  return base


def getIndentedLineHeader(d, value):
  prevHead = getIndentedLineContextHeader(d, value["previous"])
  currHead = getIndentedLineContextHeader(d, value["current"])
  return "({0}) <= ({1})".format(currHead,prevHead)


class PtrContext:
  pointerAttrname = "smartPtr"
  ptrMatch = re.compile("^smart::Ptr")

  def getPointerAttrname(self, value):
    return value[PtrContext.pointerAttrname]

  def __init__(self, d, value):
    node     = self.getPointerAttrname(value)
    self.ptr = node

    d.checkPointer(self.ptr)
    p = d.extractPointer(node)
    if not p or p < 0xFFFF:
      self.invalid = True
      return
    else:
      self.invalid = False

    self.dyntype    = self.ptr.dynamic_type
    self.rttiPtr    = self.ptr.cast(self.dyntype)
    self.rttiValue  = self.rttiPtr.dereference()
    self.strDyntype = str(self.rttiValue.type)
    self.datatypeLoc = True
    self.d = d
    self.value = value

  def idHead(self, value):
    return getIdHeadUnwrapped(self.d, value)



  def strEndSaveTab(self):
    savedTabDeep = PutSmartPtrSubitems.savedTabDeep
    PutSmartPtrSubitems.savedTabDeep -= 1
    if PutSmartPtrSubitems.savedTabDeep < -1:
      PutSmartPtrSubitems.savedTabDeep = -1
      return "# end savetab " + str(savedTabDeep) + " ERROR OPEN PAIR"
    else:
      return "# end savetab " + str(savedTabDeep)

  def strSavetab(self):
    PutSmartPtrSubitems.savedTabDeep += 1
    return "# beg savetab " + str(PutSmartPtrSubitems.savedTabDeep)


  actionsMap = {
      'Sm::Id2'                    : lambda self: str(Id2(self.d, self.ptr)),
      'Sm::IdEntitySmart'          : lambda self: str(IdEntity(self.d, self.ptr)),
      'Sm::Id'                     : lambda self: self.idHead(self.rttiValue.cast(self.ttype)),
      "Sm::s::TextChunk"           : lambda self: str(getTextFromStdString(self.d, self.rttiValue["text_"])),
      "Sm::s::loc"                 : lambda self: "# -- " + getFLocationNodeHead(self.d, self.rttiValue["loc_"]) ,
      "Sm::s::Semicolon"           : lambda self: "# ;",
      "Sm::s::Name"                : lambda self: "# <name>",
      "Sm::s::Obracket"            : lambda self: "# (",
      "Sm::s::Cbracket"            : lambda self: "# )",
      "Sm::s::Comment"             : lambda self: "# //",
      "Sm::s::OMultiLineComment"   : lambda self: "# /*",
      "Sm::s::CMultiLineComment"   : lambda self: "# */",
      "Sm::s::Endl"                : lambda self: "# \\n",
      "Sm::s::comma"               : lambda self: "# ,",
      "Sm::s::tab"                 : lambda self: getTabSpacerHead(self.d, self.rttiValue),
      "Sm::s::endsavetab"          : lambda self: self.strEndSaveTab(),
      "Sm::s::savetab"             : lambda self: self.strSavetab(),
      'Sm::IndentingBracket'       : lambda self: getIndentingBracketsHead(self.d, self.rttiValue),
      'Sm::IndentingSubconstruct'  : lambda self: getIndentingSubconstructHead(self.d, self.rttiValue),
      'Sm::Datatype'               : lambda self: getDatatypeHeader(self.d, self.rttiValue, self.datatypeLoc),
      'Sm::Function'               : lambda self: getFunctionHeader(self.d, self.rttiValue),
      'Sm::Type::MemberFunction'   : lambda self: getFunctionHeader(self.d, self.rttiValue),
      'Sm::FunctionArgument'       : lambda self: getFunctionArgumentHeader(self.d, self.rttiValue),
      'Sm::table::FieldDefinition' : lambda self: getFieldDefinitionHeader(self.d, self.rttiValue),
      'Sm::Type::RecordField'      : lambda self: getFieldDefinitionHeader(self.d, self.rttiValue),
      'Sm::SemanticTree'           : lambda self: getSemanticTreeHeader(self.d, self.rttiValue),
      'Sm::RefExpr'                : lambda self: getRefExprHeader(self.d, self.rttiValue),
      'Sm::Variable'               : lambda self: getVariableHead(self.d, self.rttiValue),
      'Sm::VariableField'          : lambda self: getVariableHead(self.d, self.rttiValue),
      'Sm::Type::MemberVariable'   : lambda self: getMemberVariableHead(self.d, self.rttiValue),
      'UserContext'                : lambda self: PtrCleanIdContext(self.d, self.rttiValue["username"]).getPtrHeader() + " // USER",
      'Sm::View'                   : lambda self: Id2(self.d, self.rttiValue["name"]["smartPtr"]).baseStr() + " // VIEW",
      'Sm::Table'                  : lambda self: Id2(self.d, self.rttiValue["name"]["smartPtr"]).baseStr() + " // TBL",
      'Sm::Package'                : lambda self: Id2(self.d, self.rttiValue["name"]["smartPtr"]).baseStr() + " // PKG",
  }


  def getPtrHeader(self):
    if self.invalid:
      return "<invalid ptr>"

    self.ttype = self.d.templateArgument(self.value.type, 0)
    strDyntype = self.strDyntype

    try:
      return PtrContext.actionsMap[self.strDyntype](self)
    except KeyError:
      if str(self.ttype) == 'Sm::Spacer':
        return "# " + self.strDyntype
    return ""


  @tracerDecorate
  def put(self, d, value):
    smartPtrValue = self.getPointerAttrname(value)
    p = d.extractPointer(smartPtrValue)
    if not p:
      putPointer(d, value, smartPtrValue, value.type)
      d.putPlainChildren(value)
      return

    head = self.getPtrHeader()

    if len(head):
      putStr(d, head)
    else:
      d.putEmptyValue()

    if not self.invalid and self.strDyntype == 'Sm::Id':
      putIdChilds(d, self.rttiValue)
    else:
      d.putPlainChildren(value)
      #putSmartPtrNode(d, rttiValue)



class SmartPtrContext(PtrContext):
  def getPointerAttrname(self, value):
    return value

  def __init__(self, d, value):
    PtrContext.__init__(self, d, value)


class PtrCleanIdContext(PtrContext):

  def idHead(self, value):
    return getIdHead(self.d, value)

  def __init__(self, d, value):
    PtrContext.__init__(self, d, value)


def putSmartPtrSubitems(d, value):
   ctx = PtrContext(d, value)
   ctx.put(d, value)


class PutListIterators:

  @staticmethod
  def getNodeContent(d, node, value):
    ttype = d.templateArgument(value.type, 0)
    nodeType = d.lookupType("std::_List_node<" + str(ttype) + " >*")
    return (node.cast(nodeType), nodeType)


  @staticmethod
  def putIterator(d, value):
    CallDeep.inDeep()
    node = value["_M_node"]
    nodeContent, nodeType = PutListIterators.getNodeContent(d, node, value)

    nodeData = nodeContent["_M_data"]
    if PtrContext.ptrMatch.match(str(nodeData.type)):
      ctx = PtrContext(d, nodeData)
      head = ctx.getPtrHeader()
      if len(head):
        putStr(d, head)

    PutListIterators.putSubitems(d, nodeContent, nodeType)
    CallDeep.outDeep()


  @staticmethod
  def putNode(d, value):
    nodeContent, nodeType = PutListIterators.getNodeContent(d, value, value)
    PutListIterators.putSubitems(d, nodeContent, nodeType)


  @staticmethod
  def putSubitems(d, nodeContent, nodeType):
    d.putNumChild(1)
    d.putEmptyValue()
    if d.isExpanded():
      with Children(d):
        d.putSubItem("v", nodeContent["_M_data"])
        #logger.info(str(nodeType))

        d.putSubItem(">", nodeContent["_M_next"].cast(nodeType))
        d.putSubItem("<", nodeContent["_M_prev"]["_M_prev"].cast(nodeType))

        #d.putSubItem("<1", nodeContent["_M_prev"].cast(nodeType))
        #d.putSubItem("<", nodeContent["_M_prev"])
        PutListIterators.printChildrenRecursive(
            d,
            nodeContent,
            previous = REVERSE_ITERATORS_TAIL_LIST,
            cntLimit = 900
        )


  @staticmethod
  def printChildrenRecursive(d, value, previous = True, cntLimit = 900, cnt = 0):
    iterString = "_M_prev" if previous else "_M_next"
    for cnt in range(0, cntLimit):
      node = value[iterString]
      nodeContent, nodeType = PutListIterators.getNodeContent(d, node, value.dereference())
      d.putSubItem(str(cnt), nodeContent["_M_data"])
      value = nodeContent



@tracerDecorate
def getCastCathegoryHeader(d, value):

  imp.reload(ora2lin_cast)
  n = ora2lin_cast.__dict__
  m = re.compile("CAST_.*")
  resultSet = set()
  val = getUIntValue(d, value["val"])
  for i in n:
    v = n[i]
    if v and type(v) == type(1) and m.match(i) and (val & v) == v:
      resultSet.add((i,v))

  return resultSet


@tracerDecorate
def putCastCathegory(d, value):
  resultSet = getCastCathegoryHeader(d, value)

  d.putEmptyValue(-99)
  d.putNumChild(1)
  if d.currentIName in d.expandedINames:
    with Children(d):
      d.putFields(value)
      for pair in sorted(resultSet):
        name = pair[0]
        d.putSubItem(name, gdb.Value(name))



def stdMapIterator(d, value):
  node = value["_M_node"]
  mappedType = d.templateArgument(value.type, 0)
  nodeTypeName = str(value.type).replace("_Rb_tree_iterator", "_Rb_tree_node", 1)
  nodeTypeName = nodeTypeName.replace("_Rb_tree_const_iterator", "_Rb_tree_node", 1)
  nodeType = d.lookupType(nodeTypeName + '*')
  data = node.cast(nodeType)
  data = data.dereference()
  data = data["_M_storage"]

  d.putNumChild(1)
  d.putEmptyValue()
  if d.isExpanded():
    with Children(d):
      first = d.childWithName(data, "first")
      if first:
        d.putSubItem("first", first.cast(mappedType))
        d.putSubItem("second", data["second"])
      else:
        d.putSubItem("value", data.cast(mappedType))
      with SubItem(d, "node"):
        d.putNumChild(1)
        d.putEmptyValue()
        d.putType(" ")
        if d.isExpanded():
          with Children(d):
            d.putSubItem("color", node["_M_color"])
            d.putSubItem("left", node["_M_left"])
            d.putSubItem("right", node["_M_right"])
            d.putSubItem("parent", node["_M_parent"])


