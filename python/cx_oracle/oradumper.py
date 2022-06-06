#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cx_Oracle
import shelve
import os
import shutil
import re
import argparse
import subprocess
import stat

from os.path import join


if cx_Oracle.version < '3.0':
  raise Exception("Very old version of cx_Oracle : {0}".format(cx_Oracle.version))

import oradumper_cfg

STATUS_UNWRAPPED = 0
STATUS_WRAP9G    = 9
STATUS_WRAP11G   = 11

debugDisableWriteSources = 0
debugReloadWriteSources = 0


oraConstructorArguments = ("SYS/man@172.17.2.80:1521/RRZ11", True)

os.environ["NLS_LANG"] = ".AL32UTF8"


'''AL32UTF8 stores characters beyond U+FFFF as four bytes (exactly as Unicode defines
UTF-8). Oracle’s “UTF8” stores these characters as a sequence of two UTF-16 surrogate
characters encoded using UTF-8 (or six bytes per character)'''


def smartDecode(b, cnt = 128):
  try:
    return b.decode('utf-8')
  except:
    pass

  res = ""
  while 1:
    chunk = b[:cnt] if len(b) > cnt else b
    try:
      v = chardet.detect(chunk)
      if v['confidence'] < 0.5 and cnt > 8:
        l = len(chunk)//2
        cnt2 = cnt//2
        res = smartDecode(chunk[:l], cnt2) + smartDecode(chunk[l:], cnt2)
      else:
        enc = v['encoding']
        res = res + chunk.decode(enc)
    except:
      res = res + str(chunk)
    if len(b) > cnt:
      b = b[cnt:]
    else:
      break
  return res



" Исключение - не единичная ячейка "
class NonSingleException(Exception):
  def __init__(self, rows, query):
    Exception.__init__("Error: row length is {0} for query '{1}'".format(len(rows), query))


" Исключение - не единичный столбец "
class NonSingleColumnQuery(Exception):
  def __init__(self, rows, query):
    Exception.__init__("Error: columns count is {0} for query '{1}'".format(len(rows), query))

" Интерфейс к Oracle "
class OraInterface:
  def __init__(self, connString, isSysdba = False):
    # args = { "dsn":connString, "threaded":True }
    if isSysdba:
      self.conn = cx_Oracle.connect(connString, mode = cx_Oracle.SYSDBA)
      self.connString = [connString, "AS", "SYSDBA"]
    else:
      self.conn = cx_Oracle.connect(connString)
      self.connString = [connString]


  " Выбрать одну ячейку "
  def single(self, query):
    l = [row for row in
            self.conn.cursor().execute(query)]
    if len(l) == 0:
      return None
    elif len(l) > 1:
      raise NonSingleException(l, query)
    else:
      r = l[0]
      if len(r) == 0:
        return None
      elif len(r) > 1:
        raise NonSingleColumnQuery(r, query)
      return r[0]

  " Выбрать столбец и вернуть его как список значений "
  def column(self, query):
    l = [row for row in self.conn.cursor().execute(query)]
    if len(l) == 0:
      return []
    else:
      if len(l[0]) > 1:
        raise NonSingleColumnQuery(l[0], query)
      else:
        return [r[0] for r in l]


  " Выбрать список кортежей "
  def rows(self, query):
    return [row for row in self.conn.cursor().execute(query)]



" Запросить из базы статусы объектов, затем из них и данных конфига собрать список сущностей для дампинга "
def classificate():
  classificatedEntities = []
  for (owner, name, *wraped) in oradumper_cfg.entitities:
    l = oraInterface.rows(
        """select STATUS, OBJECT_TYPE
             from all_objects
            where owner = '{owner}' and object_name = '{name}'""".format(owner = owner, name  = name))

    if len(l) == 0:
      print("NOT EXISTS:", "{0}.{1}".format(owner, name))
      continue

    for status, cathegory in l:
      wrapStatus = 0
      if (cathegory == "PACKAGE BODY" and len(wraped) > 1):
        wrapStatus = wraped[1]
      elif (len(wraped) > 0):
        wrapStatus = wraped[0]
      classificatedEntities.append(CodeChunck(cathegory, owner, name, status, wrapStatus, silent = False))

  return classificatedEntities



class CodeChunck:

  def __init__(self, cathegory, owner, name, status, wrap, silent, errfile):
    self.cathegory  = cathegory
    self.owner      = owner
    self.name       = name
    self.status     = status
    self.wrap       = STATUS_UNWRAPPED
    self.silent     = silent
    self.fail       = False
    self.sourceText = self.extractSelfCode()
    self.wrap       = self.detectWrapStatus()
    self.errfile    = errfile
    if self.wrap != STATUS_UNWRAPPED:
      self.sourceText = self.extractSelfCode()

    self.body = self.getBody()

  def getBody(self):
    if self.cathegory not in ["PACKAGE", "TYPE"]:
      return None
    bodyCat = self.cathegory + " BODY"
    bodyRows = oraInterface.rows(
      """select OBJECT_NAME, STATUS
           from all_objects
          where owner = '{owner}' and object_type = '{cathegory}' and
                object_name ='{objname}'""".format(owner = self.owner, cathegory = bodyCat, objname = self.name))
    if len(bodyRows) == 0:
      return None
    assert len(bodyRows) == 1

    return CodeChunck(
      cathegory = bodyCat,
      owner     = self.owner,
      name      = bodyRows[0][0],
      status    = bodyRows[0][1],
      wrap      = STATUS_UNWRAPPED,
      silent    = False,
      errfile   = self.errfile
    )


  wrappedKw = re.compile(".* wrapped[ ]*$", re.IGNORECASE)

  def detectWrapStatus(self):
    l = self.sourceText.strip().split("\n")
    if len(l) < 2:
      return 0
    m = CodeChunck.wrappedKw.match(l[0])
    if not m:
      return 0
    wrapHead = l[1]
    if wrapHead == '0':
      return STATUS_WRAP9G
    elif wrapHead == 'a000000':
      return STATUS_WRAP11G

    print(self.owner, self.name, " is wrapped!!!")
    print(l[1].encode())
    assert 0

    return STATUS_UNWRAPPED


  def fullname(self):
    return "{0}.{1}".format(self.owner, self.name)

  def initSourceText(self):
    self.sourceText = self.extractSelfCode()


  def extractWrappedSource(self):
    query = """select "TEXT" from all_source where OWNER = '{owner}' and TYPE = '{cathegory}' and NAME = '{name}' order by LINE ASC""".format(
      owner     = self.owner,
      cathegory = self.cathegory,
      name      = self.name
    )
    s = "".join(oraInterface.column(query))

    pat = re.compile('^({cathegory})(\s*)({name})'.format(
      cathegory = self.cathegory,
      name      = self.name), re.IGNORECASE)
    s = pat.sub("CREATE OR REPLACE \\g<1>\\g<2>{owner}.\\g<3>".format(owner = self.owner), s, count = 1)
    return s


  def unwrap11gCode(self):
    query = """select COLUMN_VALUE from table(sys.kt_unwrap.unwrap('{name}', '{cathegory}', '{owner}'))""".format(
      name      = self.name,
      cathegory = self.cathegory,
      owner     = self.owner
    )
    try:
      print (query)
      s = "\n".join([s if s != None else "" for s in oraInterface.column(query)])
      head = re.compile("^({0})\s+({1})".format(self.cathegory, self.name), re.IGNORECASE)
      if head.match(s):
        s = head.sub("CREATE OR REPLACE \\g<1> {owner}.\\g<2>".format(owner=self.owner), s, count = 1)
      pat1 = re.compile("\\bCREATE\\b", re.IGNORECASE)
      pat2 = re.compile("\\bWRAPPED\\b", re.IGNORECASE)
      if len(s) == 0 or not pat1.search(s) or pat2.search(s):
        msg = "bad unwrapped: " + s
        print(msg)
        self.errfile.write(msg + "\n")

      return s
    except cx_Oracle.DatabaseError as err:
      print(str(err))
      self.errfile.write((str(err) + "\n"))

    self.fail = True
    return self.extractWrappedSource()


  def extractSelfCode(self):
    print("get code for {0}, {1}".format(self.cathegory, self.fullname()))
    if self.wrap == STATUS_WRAP9G:
      result = self.extractWrappedSource()
      return result
    elif self.wrap == STATUS_WRAP11G:
      return self.unwrap11gCode()

    result = "".join([i.read() for i in oraInterface.column(
        "select dbms_metadata.get_ddl('{cathegory}', '{name}', '{owner}') from dual".format(
           cathegory = self.cathegory.replace(' ', '_'),
           name      = self.name,
           owner     = self.owner))])
    return result


  def statusStr(self):
    if self.status != 'VALID':
      return "-- STATUS = {0}\n\n/\n\n".format(self.status)
    return ""

  def __str__(self):
    if self.wrap != STATUS_WRAP9G:
      return self.statusStr() + self.sourceText.strip() + "\n;\n\n\n"
    else:
      return self.sourceText + "\n/\n"




def extractViews(self, fname, fmode, classificatedEntities):
  classificatedEntities = [entity for entity in classificatedEntities if entity.cathegory == "VIEW"]
  with open(fname, fmode) as f:
    for node in sorted(classificatedEntities, key = lambda v: (v.owner, v.name)):
      node.initSourceText()
      f.write((str(node)))


def unwrDir(base):
  return os.path.join(base, "unwr")


nodesMap = {}





# def main1():
#   nodesMap = {}
#   # dumpDb = shelve.open("oradump")
#   classificatedList = classificate()
#
#   extractViews(oraInterface, os.path.join("out", "entities.sql"), "w", classificatedList)
#   nodesMap = extractPlCode("out", "entities.sql", "a", classificatedList)
#   collect9g("out", "entities.sql", nodesMap)


def createIfNotExistDir(l):
  if not l:
    return

  for i in range(1, len(l)+1):
    patch = os.path.join(*l[:i])
    try:
      os.stat(patch)
    except FileNotFoundError:
      os.mkdir(patch)


class OraDumper:
  def __init__(self, owners, outDir, outEntitiesDir, outSourcesFname,
               out9GFname, outConcatFname, sqlpluscmd):
    self.owners          = owners
    self.outDir          = outDir
    self.outEntitiesDir  = outEntitiesDir
    self.sourcesFname    = outSourcesFname
    self.out9GFname      = out9GFname
    self.concatFname     = outConcatFname
    self.concatAllBase   = "concat_all_"
    self.entities9G      = []
    self.wrap9gIdx       = 0
    self.unwr9OutDir     = self.outEntitiesDir + ["unwr"]
    self.sqlplusOutDir   = self.outEntitiesDir + ["sqlplus"]
    self.sqlpluscmd      = sqlpluscmd


  def createIfNotExists(self):
    createIfNotExistDir(self.outDir)
    createIfNotExistDir(self.outEntitiesDir)

  def nameStatusQuery(self, owner, cathegory):
    return oraInterface.rows(
      """select OBJECT_NAME, STATUS
           from all_objects
          where owner = '{owner}' and object_type = '{cathegory}'""".format(owner = owner, cathegory = cathegory))

  def writeCP1251(self, f, errF, cathegory, nodename, s):
    enc = 'cp1251'
    try:
      f.write(s.encode(enc))
      return
    except UnicodeEncodeError as err:
      msg = "warning: {cathegory} {nodename} has bad encoding: {errstr}".format(
        cathegory = cathegory,
        nodename  = nodename,
        errstr    = str(err)
      )
      errF.write(msg + "\n")
      print(msg)
    f.write(s.encode(enc, 'ignore'))

  def write9gNode(self, node, errF):
    createIfNotExistDir(self.unwr9OutDir)

    node.wrap9gFname = "S{0:03}.SQ".format(self.wrap9gIdx)

    with open(os.path.join(*(self.unwr9OutDir + [node.wrap9gFname])), "wb") as outWr:
      self.writeCP1251(outWr,errF,node.cathegory,node.fullname(), str(node))

    self.wrap9gIdx += 1
    self.entities9G.append(node)

  def outFull9GFname(self):
    return os.path.join(*(self.unwr9OutDir + [self.out9GFname]))

  def write9GBat(self):
    if self.wrap9gIdx == 0:
      return
    wrap9gCmdList = [ ".\REWRAP.EXE " + node.wrap9gFname for node in self.entities9G]
    wrap9gCmdList.append("pause")

    with open(os.path.join(*(self.unwr9OutDir + ["unwr.bat"])), "w") as f:
      f.write("\r\n".join(wrap9gCmdList))

    with open(self.outFull9GFname(), "w") as f:
      f.write("wrappedFiles = [\n")
      q3 = "\"\"\"";
      for node in self.entities9G:
        f.write("  ({q}{owner}{q},{q}{name}{q},'{cathegory}',{q}{filename}{q}),\n".format(
            q         = "\"\"\"",
            owner     = node.owner,
            name      = node.name,
            filename  = node.wrap9gFname,
            cathegory = node.cathegory))
      f.write("]\n")

    for i in ["REWRAP.EXE", "REWRAP.DBF", "REWRAP.NTX", "REWRAP.PRT"]:
      shutil.copyfile(os.path.join("unwrap", "rewrap9g", i), os.path.join(*(self.unwr9OutDir + [i])))


  def writeNode(self, node, f, errF):
    if node.wrap == STATUS_WRAP9G:
      self.write9gNode(node, errF)
    else:
      self.writeCP1251(f,errF,node.cathegory,node.fullname(),"\n")
      self.writeCP1251(f,errF,node.cathegory,node.fullname(),str(node))

    if node.body:
      if node.wrap == STATUS_UNWRAPPED and node.body.wrap == STATUS_UNWRAPPED:
        return
      self.writeNode(node.body, f, errF)


  def outErrname(self):
    return os.path.join(*(self.outEntitiesDir + ["errors_big.sql"]))


  def uploadInBigFile(self):
    availCat = getAllObjectCathegories(m3_users)
    print("available cathegories is:", ", ".join(availCat))


    self.createIfNotExists()
    with open(self.outErrname(), "w") as errF:
      for owner in self.owners:
        outFname = os.path.join(*(self.outEntitiesDir + [owner + "_big.sql"]))
        with open(outFname, "wb") as f:
          for cathegory in availCat:
            for (name, status) in self.nameStatusQuery(owner, cathegory):
              try:
                node = CodeChunck(
                  cathegory = cathegory,
                  owner     = owner,
                  name      = name,
                  status    = status,
                  wrap      = STATUS_UNWRAPPED,
                  silent    = False,
                  errfile   = errF
                )
                self.writeNode(node, f, errF)
              except cx_Oracle.DatabaseError as err:
                print(str(err))
                errF.write(str(err) + "\n")

    self.write9GBat()

  def sourcesFullFname(self):
    return os.path.join(*(self.outDir + [self.sourcesFname]))


  def writeSourcesVarFile(self, outFnames):
    if debugDisableWriteSources:
      return
    elif debugReloadWriteSources:
      outFnamesOth = self.loadOutFnames()
      for i in outFnames:
        if i not in outFnamesOth:
          outFnamesOth.append(i)
      outFnames = outFnamesOth

    with open(self.sourcesFullFname(), "w") as f:
      f.write("sources += [\n")
      for i in outFnames:
        f.write("  \"\"\"" + i + "\"\"\",\n")
      f.write("]\n")

  def nodeOutFname(self, cathegory, owner, name):
    return (cathegory + "_" + owner + "_" + name + ".sql").replace(' ', '_')

  def nodeOutFullFname(self, cathegory, owner, name):
    return os.path.join(owner, self.nodeOutFname(cathegory, owner, name))

  sysusers = [
      "APEX_030200",
      "APPQOSSYS",
      "CTXSYS",
      "EXFSYS",
      "FLOWS_FILES",
      "LBACSYS",
      "MDSYS",
      "OLAPSYS",
      "ORDDATA",
      "ORDSYS",
      "SYS",
      "SYSMAN",
      "SYSTEM",
      "WMSYS",
      "XDB"]

  def uploadSplitted(self):
    self.createIfNotExists()

    outFnames = []
    patSysSynonym = re.compile(" FOR[ ]+(\")?({sysusers})(\")?\\.".format(sysusers = "|".join(OraDumper.sysusers)))

    availCat = getAllObjectCathegories(m3_users)
    print("available cathegories is:", ", ".join(availCat))

    with open(self.outErrname(), "w") as errF:
      for owner in self.owners:
        print("========= owner = ", owner)
        for cathegory in availCat:
          for (name, status) in self.nameStatusQuery(owner, cathegory):
            try:
              outFname = self.nodeOutFname(cathegory, owner, name)
              if "/" in outFname:
                continue
              node =  CodeChunck(
                  cathegory = cathegory,
                  owner     = owner,
                  name      = name,
                  status    = status,
                  wrap      = STATUS_UNWRAPPED,
                  silent    = False,
                  errfile   = errF
                )
              node.basedir  = owner
              node.outFname = outFname
              if patSysSynonym.search(str(node)):
                continue

              outFnames.append(self.nodeOutFullFname(node.cathegory, node.owner, node.name))

              outDir = self.outEntitiesDir + [owner]
              createIfNotExistDir(outDir)
              with open(os.path.join(*(outDir + [node.outFname])), "wb") as f:
                self.writeNode(node, f, errF)

            except cx_Oracle.DatabaseError as err:
              print(str(err))
              errF.write(str(err) + "\n")


    self.writeSourcesVarFile(outFnames)
    self.write9GBat()

  def loadOutFnames(self):
    codeGlobals = {"sources":[]}
    codeLocals = {}
    with open(self.sourcesFullFname(), "rb") as f:
      codeObject = compile(f.read(), '<string>', 'exec')
      exec(codeObject, codeGlobals, codeLocals)
      outFnames = codeGlobals["sources"]
    return outFnames


  def unwr9Gconcat(self):
    outFnames = self.loadOutFnames()

    codeGlobals = {}
    codeLocals = {}
    with open(self.outFull9GFname(), "rb") as f:
      codeObject = compile(f.read(), '<string>', 'exec')
      exec(codeObject, codeGlobals, codeLocals)
      wrappedFiles = codeLocals["wrappedFiles"]

    wrappedPat = re.compile(b"^.* wrapped[ ]*$")

    with open(self.outErrname() + "unwrconcat.sql", "w") as errF:
      for owner, name, cathegory, filename in wrappedFiles:
        def writeErrMsg(fmt):
          errMsg = fmt.format(cathegory, owner, name, filename)
          print(errMsg)
          errF.write(errMsg)

        fullSrcName = os.path.join(*(self.unwr9OutDir + [filename + "L"]))
        try:
          os.stat(fullSrcName)
        except FileNotFoundError:
          s = "file not found (not unwrapped) {0}\n".format(fullSrcName)
          print(s)
          errF.write(s)

        subsPat = re.compile(
            b"(CREATE OR REPLACE\s+" + cathegory.encode() + b"\s+)(" + name.encode() + b"\s+)", re.IGNORECASE)

        with open(fullSrcName, "rb") as f:
          fileText = f.read()
          if len(fileText) == 0:
            writeErrMsg("error: {0} {1}.{2} from {3} is not unwrapped (empty)\n")
            continue
          needContinue = False

          for l in fileText.split(b"\n"):
            if wrappedPat.match(l):
              writeErrMsg("error: {0} {1}.{2} from {3} is not unwrapped (wrapped)\n")
              needContinue = True
              break
          if needContinue:
            continue

          unwrText = subsPat.sub(b"\\g<1>" + owner.encode() + b".\\g<2>", fileText, count = 1)

        dstCathegory = cathegory
        if dstCathegory == 'PACKAGE BODY':
          dstCathegory = 'PACKAGE'
        elif dstCathegory == 'TYPE BODY':
          dstCathegory = 'TYPE'

        dstfname = self.nodeOutFullFname(dstCathegory, owner, name)
        if dstfname in outFnames:
          outFnames.append(dstfname)

        fullDstfname = os.path.join(*(self.outEntitiesDir + [dstfname]))

        with open(fullDstfname, "ab") as dstF:
          dstF.write(unwrText)

        # print(owner, name, cathegory, filename, "==>", dstfname)

    self.writeSourcesVarFile(outFnames)


  def concatSources(self):
    codeGlobals = {"sources":[]}
    codeLocals = {}
    with open(self.sourcesFullFname(), "r") as f:
      codeObject = compile(f.read(),  '<string>', 'exec')
      exec(codeObject, codeGlobals, codeLocals)

    with open(os.path.join(*((self.outDir + [self.concatFname]))), "wb") as outF:
      for fname in codeGlobals['sources']:
        with open(os.path.join(*(self.outEntitiesDir + [fname])), "rb") as inF:
          outF.write(inF.read())


  def concatUsersIntoSingle(self, outDir, concatFname):
    pat = re.compile("{0}.*".format(self.concatAllBase))

    with open(join(join(*self.outDir), concatFname), "wb") as dst:
      for f in sorted(os.listdir(outDir)):
        if not stat.S_ISDIR(os.stat(join(outDir, f)).st_mode) and (pat.match(f) or f == 'descr_tables_result.sql'):
          with open(join(outDir, f), "rb") as src:
            dst.write(src.read())
            dst.write(b"\n")



  def concatSourcesByUser(self):
    codeGlobals = {"sources":[]}
    codeLocals = {}
    with open(self.sourcesFullFname(), "r") as f:
      codeObject = compile(f.read(),  '<string>', 'exec')
      exec(codeObject, codeGlobals, codeLocals)

    cathegories = [
      "SEQUENCE", "TABLE", "INDEX", "SYNONYM", "TYPE", "VIEW", "FUNCTION", "PROCEDURE", "PACKAGE", "TRIGGER"
    ]
    singleFileCathegories = set(["SEQUENCE", "TABLE", "INDEX", "SYNONYM"])

    userFileCathegories = {
      "TYPE":0,
      "VIEW":1,
      "FUNCTION":2,
      "PROCEDURE":3,
      "PACKAGE":4,
      "TRIGGER":5
    }
    singleFileEntities = {}
    userFileEntities = {}

    for fname in codeGlobals['sources']:
      user, fname = os.path.split(fname)

      splitPat = re.compile("({cathegory})_{user}_(.*)\\.sql".format(
          cathegory = "|".join(cathegories),
          user = user
      ))
      m = splitPat.match(fname)
      cathegory = m.group(1)
      name      = m.group(2)

      if cathegory in singleFileCathegories:
        try:
          singleFileEntities[cathegory].append((user, name, fname))
        except KeyError:
          singleFileEntities[cathegory] = [(user, name, fname)]
      else:
        try:
          userFileEntities[user].append((cathegory, name, fname))
        except KeyError:
          userFileEntities[user] = [(cathegory, name, fname)]


    def transformName(n):
      if n == "index":
        return "indices.sql"
      else:
        return n + "s.sql"

    def writeNodeToFile(outF, user, fname):
      try:
        with open(os.path.join(*(self.outEntitiesDir + [user, fname])), "rb") as inF:
          outF.write(inF.read())
      except FileNotFoundError as err:
        print(err)


    for cathegory, nodes in singleFileEntities.items():
      print(cathegory, len(nodes))
      with open(os.path.join(*((self.outDir + [self.concatAllBase + transformName(cathegory.lower())]))), "wb") as outF:
        userMap = {}
        for (user, name, fname) in nodes:
          try:
            userMap[user].append((name, fname))
          except KeyError:
            userMap[user] = [(name, fname)]
        for user in sorted(userMap.keys()):
          if user != "PUBLIC":
            outF.write(b"\nCONNECT \"" + user.encode() + b"\";\n\n")
          for (name, fname) in sorted(userMap[user], key = lambda x: x[0]):
            writeNodeToFile(outF, user, fname)
                                         # , key= lambda x: (x[0], x[1])
    for user, nodes in userFileEntities.items():
      print(user, len(nodes))
      with open(os.path.join(*((self.outDir + [self.concatAllBase + user.lower() + ".sql"]))), "wb") as outF:
        if user != "PUBLIC":
          outF.write(b"CONNECT \"" + user.encode() + b"\";\n\n")
        for (cathegory, name, fname) in sorted(nodes, key= lambda x: (userFileCathegories[x[0]], x[1])):
          writeNodeToFile(outF, user, fname)


  def describeTablesBySqlplus(self):
    createIfNotExistDir(self.sqlplusOutDir)
    os.chdir(os.path.join(*self.sqlplusOutDir))
    sqlplusScript    = "descr_tables.sql"
    sqlplusScriptOut = "descr_tables_out.sql"
    with open(sqlplusScript, "w") as f:
      f.write("""
SET ECHO OFF
SET FEEDBACK OFF
SET LINESIZE 1000
SET LONG 900000
SET LONGCHUNKSIZE 1000
SET PAGES 9999
SET SERVEROUT ON
SET TERMOUT OFF
SET TRIMOUT ON
SET TRIMSPOOL ON
SET VERIFY OFF
SET WRAP OFF

SPOOL {0} REPLACE

""".format(sqlplusScriptOut)
      )
      for owner in self.owners:
        tables = [table for table, status in self.nameStatusQuery(owner, "TABLE")]
        for table in sorted(tables):
          fullname = "{0}.{1}".format(owner, table)
          f.write("prompt == start describe table {0}\n".format(fullname))
          f.write("descr {0}\n".format(fullname))
          f.write("prompt == end describe table\n\n")
      f.write("exit\n")

    arglist = [self.sqlpluscmd] + oraInterface.connString + ["@" + sqlplusScript]
    print("executing ", str(arglist))
    sqlplusOut = subprocess.Popen(arglist, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
    print("".join([l.decode() for l in sqlplusOut.stdout.readlines()]))
    self.parseSqlplusOutFile(sqlplusScriptOut, "descr_tables_result.sql")


  def parseSqlplusOutFile(self, parsingFile, outfile):
    startPat = re.compile(b'^== start describe table (.*)$')
    endPat   = re.compile(b'^== end describe table$')
    fieldPat = re.compile(b'[ ]*(.*[^ ])[ ]+((NOT NULL)?)[ ]+([^ ].*)$')

    with open(parsingFile, "rb") as f:
      l = f.readlines()

    it = iter(l)
    with open(outfile, "wb") as f:
      try:
        while True:
          line = next(it)
          m = startPat.match(line)
          # парсинг описания таблицы
          if m:
            fulltabname = m.group(1)
            f.write(b'CREATE OR REPLACE TABLE ' + fulltabname + b' (\n')
            next(it) # строка Name
            next(it) # строка ---
            fieldsStr = bytearray(b'')
            while True:
              line = next(it)
              fieldMatch = fieldPat.match(line)
              if not fieldMatch:
                line = next(it) # пропуск пустой строки
                assert endPat.match(line)
                if fieldsStr[-1] == ord(','):
                  fieldsStr[-1] = ord('\n')
                f.write(fieldsStr)
                f.write(b');\n')
                break
              fieldname = fieldMatch.group(1)
              notNull   = fieldMatch.group(3)
              datatype  = fieldMatch.group(4)

              if ord('?') in fieldname:
                fieldname = b'"' + fieldname + b'"'
              if datatype == b"BINARY FILE LOB":
                datatype = b"BFILE"


              if len(fieldsStr) > 0 and fieldsStr[-1] == ord(','):
                fieldsStr += b'\n'

              fieldsStr += b'  ' + fieldname + b' ' + datatype
              if notNull and len(notNull) > 0:
                fieldsStr += b' NOT NULL'
              fieldsStr += b','



      except StopIteration:
        pass



def printEncQ():
  for l in oraInterface.rows("SELECT * FROM NLS_DATABASE_PARAMETERS"):
    print(l)



class WorkflowConfig:
  def __init__(self):
    argp = argparse.ArgumentParser(description="Инструмент для выгрузки сущностей из Oracle")
    argp.add_argument('--concat'    , action='store_true', help='Склеить ранее разбитую выгрузку сущностей')
    argp.add_argument('--uconcat'   , action='store_true', help='Склеить ранее разбитую выгрузку сущностей по пользователям')
    argp.add_argument('--gendiff'   , action='store_true', help='Сгенерировать diff между старой и измененной версией')
    argp.add_argument('--unwrconcat', action='store_true', help='Склеить разврапленные сущности с пофайловым разбиением')
    argp.add_argument('--splitted'  , action='store_true', help='Выгрузить сущности по одной на файл в отдельный каталог')
    argp.add_argument('--descr'     , action='store_true', help='Выгрузить описания таблиц через sqlplus descr')
    argp.add_argument('--encq'      , action='store_true', help='Запросить данные о кодировке базы')
    self.cmdargs = argp.parse_args()
    self.argp = argp

  def gendiff(self):
    return self.cmdargs.gendiff

  def needConcatFiles(self):
    return self.cmdargs.concat

  def needSplittedUpload(self):
    return self.cmdargs.splitted

  def sqlplusTables(self):
    return self.cmdargs.descr

  def unwrconcat(self):
    return self.cmdargs.unwrconcat

  def encq(self):
    return self.cmdargs.encq

  def uconcat(self):
    return self.cmdargs.uconcat


def getAllObjectCathegories(users):
    l = oraInterface.column(
      """select DISTINCT OBJECT_TYPE
           from all_objects
          where owner in ({0})""".format("'" + "','".join(users) + "'"))
    priorities = {
      "SEQUENCE"         : 1,
      "TABLE"            : 2,
      "INDEX"            : 3,
      "SYNONYM"          : 4,
      "TYPE"             : 5,
      "TYPE BODY"        : 6,
      "MATERIALIZED VIEW": 7,
      "VIEW"             : 8,
      "FUNCTION"         : 9,
      "PROCEDURE"        :10,
      "PACKAGE"          :11,
      "PACKAGE BODY"     :12,
      "TRIGGER"          :13,
    }

    #debugFiter = ["SEQUENCE", "TABLE", "INDEX", "SYNONYM", "VIEW", "TRIGGER", "FUNCTION", "PROCEDURE", "TYPE"] #, "PACKAGE", "VIEW", "FUNCTION", "PROCEDURE"]
    debugFiter = ["SEQUENCE", "TABLE", "INDEX", "SYNONYM", "PACKAGE", "FUNCTION", "PROCEDURE", "TYPE", "TRIGGER"]
    debugFiter = []
    filterEntities = set(["PACKAGE BODY", "TYPE BODY"] + debugFiter)
    res = sorted([i for i in l if i not in filterEntities], key=lambda x:priorities.get(x, 0))
    return list(res)



def genDiff(old, new, diffFilename):
  arglist = ['/usr/bin/diff', '-U', '0', '-ibB', "-W160", old, new]

  result = subprocess.Popen(arglist, stdout = subprocess.PIPE, stderr = subprocess.STDOUT)
  with open(diffFilename, 'wb') as outFile:
    for s in result.stdout.readlines():
      outFile.write(s)

def connect():
  global oraInterface
  oraInterface = OraInterface(*oraConstructorArguments)

def main():
  m3_users = [
    "MODELLE",
    "UPDATES",
    "M3_GP",
    "M2_WORKFLOW",
    "EXTERNALDATA",
    "M2_ALL",
    "M2_SETUP",
    "M2_HISTORY",
    "M3CONVERT",
    "EXCHANGE1C",
    "SCOTT",
    "M2_SYS",
    "PUBLIC"
  ]
  #m3_users = [ "PUBLIC" ]

  oraDumper = OraDumper(m3_users,
    ["out", "big"],
    ["out", "big", "entities"],
    outSourcesFname = "sourcesList_clean.py",
    out9GFname      = "entities9G.py",
    outConcatFname  = "concat_src_clean.sql",
    sqlpluscmd      = "/usr/bin/sqlplus"
  )

  cfg = WorkflowConfig();

  if cfg.needConcatFiles():
    oraDumper.concatSources()
  elif cfg.needSplittedUpload():
    connect()
    oraDumper.uploadSplitted()
  elif cfg.sqlplusTables():
    connect()
    oraDumper.describeTablesBySqlplus()
  elif cfg.unwrconcat():
    oraDumper.unwr9Gconcat()
  elif cfg.encq():
    printEncQ()
  elif cfg.uconcat():
    oraDumper.concatSourcesByUser()
  elif cfg.gendiff():
    changedPath = "/home/artem/devs/ora2lin/scripts/oradumper/backup_changed"
    initFname    = "concat_src_init.sql"
    changedFname = "concat_src_changed.sql"
    oraDumper.concatUsersIntoSingle(join(*oraDumper.outDir), initFname)
    oraDumper.concatUsersIntoSingle(changedPath, changedFname)
    genDiff(join(join(*oraDumper.outDir), initFname), join(join(*oraDumper.outDir), changedFname),
            join(join(*oraDumper.outDir), "concat_src.diff"))
  else:
    connect()
    oraDumper.uploadInBigFile()


if __name__ == '__main__':
  main()


# TODO: написать код для получения точных типов данных таблиц через sqlplus descr


