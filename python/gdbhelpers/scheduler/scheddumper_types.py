

import sys
import os
import types
import imp

sys.path.append(os.path.split(os.path.abspath(__file__))[0])
sys.path.append('/usr/share/qtcreator/debugger')

import scheddumper_utils


import gdb
from dumper import *
from stdtypes import *

import scheddumper_helper
from scheddumper_utils import logger


try:
  logger.info('========================= new session debug =========================')
except:
  scheddumper_utils.exceptionInfo('session logger start')


def serviceUpdate(self, d):
  if self.needReload():
    imp.reload(scheddumper_helper)
  scheddumper_helper.ExceptionDecorate.serviceUpdate(self, d)


class exceptionDecorate(object):
  def __init__(self, fun):
    logger.info('000')
    self.fun = fun
    logger.info('0001')

  def __call__(self, *pargs, **kargs):
    logger.info('111')
    #obj = scheddumper_helper.ExceptionDecorate(self.fun)
    logger.info('222')
    return None #obj(*pargs, **kargs)


class tryFormatDecorate(object):
  def __init__(self, fun):
    self.fun = fun

  def __call__(self, *pargs, **kargs):
    obj = scheddumper_helper.TryFormatDecorate(self.fun)
    obj.serviceUpdate = types.MethodType(serviceUpdate, obj)
    return obj(*pargs, **kargs)


def exceptCall(fun, *pargs, **kargs):
  try:
    logger.info("-----")
    return fun(*pargs, **kargs)
  except RuntimeError as i:
    logger.info(str(i))
    raise i
  except:
    logger.info("--- ERROR in exceptCall ---")
    scheddumper_utils.exceptionInfo(str(fun))


def qdump__task_struct(d, value):
  logger.info("21")
  exceptCall(scheddumper_helper.put__task_struct, d, value)


def qdump__sched_skiplist_node(d, value):
  logger.info("11")
  exceptCall(scheddumper_helper.put__sched_skiplist_node, d, value)

#@exceptionDecorate
#def qdump__skiplist_value_type(d, value):
#  scheddumper_helper.put__task_struct(d, value)


