# UNCOMMENT DEBUG

# проверка на == указателя 0:
# if not d.extractPointer(value):
#

import types
import gdb
from scheddumper_utils import logger, exceptionInfo


from dumper import *
from stdtypes import *

global AutomaticFormat

AutomaticFormat = Utf8StringFormat

globalTraceEnable = False

REVERSE_ITERATORS_TAIL_LIST = False


class CallDeep:
  deepCall = 0

  @staticmethod
  def fmtPrefix(dfltVal=1):
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

  def __call__(self, *pargs, **kargs):
    try:
      logger.info("-----")
      return self.fun(*pargs, **kargs)
    except RuntimeError as i:
      exceptionInfo(self.funname)
      raise i
    except:
      logger.info("--- ERROR in ExceptionDecorate ---")
      exceptionInfo(self.funname)


def getEncodingType(displayFormat=AutomaticFormat):
    if displayFormat == Latin1StringFormat or displayFormat == SeparateLatin1StringFormat:
        return "latin1"
    else:
        return "utf8"


@tracerDecorate
def putPointer(d, value, type, displayFormat = AutomaticFormat):
  d.putValue(d.hexencode(str(value)), "utf8") #Hex2EncodedUtf8WithoutQuotes)
  d.putType(type)

  if displayFormat == Latin1StringFormat or displayFormat == Utf8StringFormat:
    d.putDisplay("latin1:separate", StopDisplay)
  elif displayFormat == SeparateLatin1StringFormat or displayFormat == SeparateUtf8StringFormat:
    d.putField("editformat", "latin1")
    elided, shown = d.computeLimit(bytelen, 100000)
    d.putField("editvalue", d.readMemory(data, shown))


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


def putStdString(d, data, size, limit):
  bytelen = size
  elided, shown = bytelen, limit
  mem = d.readMemory(data, shown)

  displayFormat = AutomaticFormat
  d.putValue(mem, getEncodingType(displayFormat))
  d.putField("editformat", "latin1")
  elided, shown = d.computeLimit(bytelen, 100000)
  d.putField("editvalue", d.readMemory(data, shown))


def putStr(d, s):
  strGdbVal, l = gdb.Value(s), len(s)
  putStdString(d, strGdbVal, l, l)


def task_struct_format(value):
  return "t:{0},wt:{1}".format(str(value["debug_task_id"]), str(value["wt"]))


def put__sched_skiplist_node(d, value):
  task_struct = value["value"]
  if task_struct == 0:
    head = "l:{0},NULL".format(str(value["level"]))
  else:
    head = "l:{0},{1}".format(str(value["level"]), task_struct_format(task_struct))
  strGdbVal, l = gdb.Value(head), len(head)
  putStdString(d, strGdbVal, l, l)
  d.putPlainChildren(value)


def put__task_struct(d, value):
  tid = task_struct_format(value)
  strGdbVal, l = gdb.Value(tid), len(tid)
  putStdString(d, strGdbVal, l, l)
  d.putPlainChildren(value)




