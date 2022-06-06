

import sys
import io
import os
import types
import imp

sys.path.append(os.path.split(os.path.abspath(__file__))[0])

import dumper_utils
from dumper_utils import exceptionInfo, logger
dumper_utils.initPatches()

import gdb
from dumper import *
from stdtypes import *

import ora2lin_dumper
import ora2lin_lloc

try:
  logger.info('========================= new session debug =========================')
except:
  exceptionInfo('session logger start')

#if 0:
#  import threading
#  import rpdb2
#
#  class Thr(threading.Thread):
#    def __init__(self):
#      threading.Thread.__init__(self)
#
#    def run(self):
#      rpdb2.start_embedded_debugger("123")
#
#
#  debugThread = Thr()
#  debugThread.start()


def serviceUpdate(self, d):
  if self.needReload():
    imp.reload(ora2lin_dumper)
  ora2lin_dumper.ExceptionDecorate.serviceUpdate(self, d)


class exceptionDecorate:
  def __init__(self, fun):
    self.fun = fun

  def __call__(self, *pargs, **kargs):
    obj = ora2lin_dumper.ExceptionDecorate(self.fun)
    obj.serviceUpdate = types.MethodType(serviceUpdate, obj)
    #debugThread.pargs = pargs
    #debugThread.kargs = kargs
    return obj(*pargs, **kargs)


class tryFormatDecorate(object):
  def __init__(self, fun):
    self.fun = fun

  def __call__(self, *pargs, **kargs):
    obj = ora2lin_dumper.TryFormatDecorate(self.fun)
    obj.serviceUpdate = types.MethodType(serviceUpdate, obj)
    #debugThread.pargs = pargs
    #debugThread.kargs = kargs
    return obj(*pargs, **kargs)


def qform__Sm__HString():
    return "Inline,In Separate Window"


@exceptionDecorate
def qdump__Sm__HString(d, value):
    qdump__std__stringHelper1(d, value, 1)


def qform__Sm__String():
    return "Inline,In Separate Window"


@exceptionDecorate
def qdump__Sm__String(d, value):
    qdump__std__stringHelper1(d, value, 1)


@exceptionDecorate
def qdump__cl__position(d, value):
  lin = value["line"]
  col = value["column"]

  strval = str(d.extractInt(lin.address)) + "," + str(d.extractInt(col.address))
  strGdbVal = gdb.Value(strval)

  l = len(strval)
  ora2lin_dumper.putStdString(d, strGdbVal, l, l)
  d.putNumChild(1)
  if d.isExpanded():
    with Children(d):
      d.putSubItem("line", lin)
      d.putSubItem("column", col)


@exceptionDecorate
def qdump__cl__location(d, value):
  dumperLLoc = ora2lin_lloc.LLoc(d, value)
  dumperLLoc.putHead()
  dumperLLoc.putExpanded()


@exceptionDecorate
def qdump__cl__filelocation(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value)


@exceptionDecorate
def qdump__Sm__SqlExprId(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value["lloc"])


@exceptionDecorate
def qdump__Sm__SqlExpr(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value["lloc"])


@exceptionDecorate
def qdump__Sm__PlExpr(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value["lloc"])


@exceptionDecorate
def qdump__Sm__GrammarBase(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value["lloc"])


@exceptionDecorate
def qdump__Sm__Declaration(d, value):
    ora2lin_dumper.dumpFLocationNode(d, value, value["lloc"])


@exceptionDecorate
def qdump__Sm__Id(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIdHead(d, value))
  ora2lin_dumper.putIdChilds(d, value)


@tryFormatDecorate
def tryPutHeader(d, value, action):
  ora2lin_dumper.putStr(d, action(d, value))


@exceptionDecorate
def qdump__Sm__Function(d, value):
  tryPutHeader(d, value, ora2lin_dumper.getFunctionHeader)
  d.putPlainChildren(value)

@exceptionDecorate
def qdump__Sm__View(d, value):
  tryPutHeader(d, value, ora2lin_dumper.getViewHeader)
  d.putPlainChildren(value)

@exceptionDecorate
def qdump__Sm__SemanticTree(d, value):
  tryPutHeader(d, value, ora2lin_dumper.getSemanticTreeHeader)
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__RefExpr(d, value):
  tryPutHeader(d, value, ora2lin_dumper.getRefExprHeader)
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IdEntitySmart(d, value):
  ora2lin_dumper.putIdEntityNode(d, value)


@exceptionDecorate
def qdump__Sm__Id2(d, value):
  ora2lin_dumper.putId2Node(d, value)


@exceptionDecorate
def qdump__Sm__Variable(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getVariableHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__VariableField(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getVariableHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__Type__MemberVariable(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getMemberVariableHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__SelectedField(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getSelectedFieldHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__ResolvedEntity(d, value):
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__ResolvedEntityLoc(d, value):
  ora2lin_dumper.putResolvedEntityLoc(d, value)


@exceptionDecorate
def qdump__Sm__ResolvedEntitySNodeLoc(d, value):
  ora2lin_dumper.putResolvedEntityLoc(d, value)


@exceptionDecorate
def qdump__Sm__table__FieldDefinition(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getFieldDefinitionHeader(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__Datatype(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getDatatypeHeader(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentedLineContext(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentedLineContextHeader(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentedLine(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentedLineHeader(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__smart__Ptr(d, value):
  ora2lin_dumper.putSmartPtrSubitems(d, value)


@exceptionDecorate
def qdump__std___List_iterator(d, value):
  ora2lin_dumper.PutListIterators.putIterator(d, value)


@exceptionDecorate
def qdump__std___List_const_iterator(d, value):
  ora2lin_dumper.PutListIterators.putIterator(d, value)


@exceptionDecorate
def qdump__std___List_node(d, value):
  ora2lin_dumper.PutListIterators.putNode(d, value)


@exceptionDecorate
def qdump__Sm__IndentingBracket(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentingBracketsHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentingSubconstruct(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentingSubconstructHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentedPos(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentedPosHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentingContext(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentingContextHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__IndentingAction(d, value):
  ora2lin_dumper.putStr(d, ora2lin_dumper.getIndentingActionHead(d, value))
  d.putPlainChildren(value)


@exceptionDecorate
def qdump__Sm__CastCathegory(d, value):
  ora2lin_dumper.putCastCathegory(d, value)


@exceptionDecorate
def qdump__std__map__iterator(d, value):
  ora2lin_dumper.stdMapIterator(d, value)


@exceptionDecorate
def qdump__std__map__const_iterator(d, value):
  ora2lin_dumper.stdMapIterator(d, value)


@exceptionDecorate
def qdump__std___Rb_tree_iterator(d, value):
  ora2lin_dumper.stdMapIterator(d, value)


@exceptionDecorate
def qdump__Sm__LevelResolvedNamespace(d, value):
  ora2lin_dumper.putLevelResolvedNamespace(d, value)



