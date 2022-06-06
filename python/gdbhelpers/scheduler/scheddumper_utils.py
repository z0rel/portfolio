#!/usr/bin/env python3

import sys
import traceback
import os
import stat
import shelve
import pprint
import logging


logging.basicConfig(level=logging.INFO,
                    filename='/tmp/linscheduler.log', filemode='w',
                    format='%(asctime)s %(message)s',
                    datefmt='%m-%d %H:%M')

logger = logging.getLogger('linscheduler')


class ExceptionInfo:
    def __init__(self, writefun=print, formatter=lambda x: x):
        self.writefun  = writefun
        self.formatter = formatter

    def __call__(self, text=''):
      try:
        self.writefun(self.formatter('ERROR IN {0}'.format(text)))
        exc_type, exc_value, exc_traceback = sys.exc_info()
        t = traceback.format_exception(exc_type, exc_value, exc_traceback)
        for i in t:
          self.writefun(repr(i))
      except:
        self.writefun(self.formatter('ERROR IN exceptionInfo: CRITICAL'))
        exc_type, exc_value, exc_traceback = sys.exc_info()
        t = traceback.format_exception(exc_type, exc_value, exc_traceback)
        for i in t:
          self.writefun(repr(i))


exceptionInfo = ExceptionInfo(logger.info, lambda x: '----- {0} -----'.format(x))


class MetaRepr:
  __deep = 0

  def strDict(self, attrname, attrval):
    lst  = "%s : {\n" % attrname
    MetaRepr.__deep += 1
    for (i, v) in attrval.items():
      if isinstance(v, dict):
        lst += self.strDict(i, v)
      else:
        lst += "{tab}{0} : \n{1}\n".format(
            i,
            pprint.pformat(v, indent=MetaRepr.__deep * 2 + 2, depth=MetaRepr.__deep + 1),
            tab=' ' * (MetaRepr.__deep * 2)
        )
    lst += "}\n"
    MetaRepr.__deep -= 1
    return lst

  def __str__(self):
    tab = ' ' * (MetaRepr.__deep * 2)
    MetaRepr.__deep += 1

    lst = '"{0}":{{\n'.format(str(type(self)), tab=tab)
    for key in sorted(self.__dict__):
      attrval = getattr(self, key)
      if isinstance(attrval, dict):
        lst += self.strDict(key, attrval)
      else:
        lst += "{tab}{0} = {1}\n".format(
          key, pprint.pformat(attrval, indent=MetaRepr.__deep * 2 + 2, depth=MetaRepr.__deep + 1),
          tab=tab
        )

    MetaRepr.__deep -= 1
    lst += "{tab}}},\n".format(tab=tab)

    return lst

  def __repr__(self):
    return MetaRepr.__str__(self)


def stripConfigPatches(f, config):
  if not config:
    return f
  return f.replace(config.converter_repository_path + os.sep, "").replace(
      config.model_src_repository_path + os.sep, "")


class StatTree(MetaRepr):
  dbname    = 'dumperStats.db'
  savedname = 'saved'

  def __init__(self, pathToDirectory):
    self.currDir = pathToDirectory
    self.files = {}

  def scanPath(self, directory=None):
    if not directory:
      directory = self.currDir

    for f in os.listdir(directory):
      if f == StatTree.dbname or f == '__pycache__' or (len(f) and (f[0] == '.' or f[-1] == '~')):
        continue

      pathname = directory + '/' + f
      mode     = os.stat(pathname).st_mode
      if stat.S_ISDIR(mode):
        self.scanPath(pathname)
      else:
        self.files[pathname] = os.stat(pathname).st_mtime

  def dbpath(self):
    return os.path.join(self.currDir, StatTree.dbname)

  def getDB(self):
    return shelve.open(self.dbpath())

  def saveToDb(self):
    d = self.getDB()
    d[StatTree.savedname] = self

  def loadSavedFromDb(self):
    d = self.getDB()

    try:
      return d[StatTree.savedname]
    except:
      return None

  def __eq__(self, oth):
    try:
      if len(self.files) != len(oth.files):
        return False
      for i in self.files:
        if i not in oth.files or self.files[i] != oth.files[i]:
          return False
      return True
    except:
      return False


def needReload():
  i = StatTree()
  i.scanPath()
  loaded = i.loadSavedFromDb()
  return i != loaded


def testStatTree():
  i = StatTree()
  if 1:
    print("== parse test ==")
    i.scanPath()
    print(i)

  if 0:
    print("== load empty test ==")
    try:
      os.remove(i.dbpath())
    except:
      pass
    print(i.loadSavedFromDb())

  if 0:
    print("== save test ==")
    i.saveToDb()

  if 1:
    print("== load test ==")
    loaded = i.loadSavedFromDb()
    print(loaded)

    if 1:
      print("== comparsion test ==")
      print("new " + ("=" if i == loaded else "!=") + " saved")

  print("== need reload test ==")
  print("need reload: ", needReload())



if __name__ == '__main__':
  testStatTree()
