
import gdb
import ora2lin_dumper
from dumper import Children

class LLoc:
    def getFilename(d, llocValue):
      fNode = llocValue["file"]

      p = d.extractPointer(fNode)
      if p == 0:
        return ""
      else:
        return ora2lin_dumper.getTextFromStdString(d, fNode.dereference())


    def __init__(self, d = None, value = None, fname = ""):
        if not d:
          (self.ibLin, self.ibCol, self.ibByte, self.ieLin, self.ieCol, self.ieByte) = [0, 0, 0, 0, 0, 0]
          self.fname = ""
          return

        self.d     = d
        self.value = value
        self.llocparts = [
                           ("beg line"  , value["begin"]["line"]  ),
                           ("beg column", value["begin"]["column"]),
                           ("beg byte"  , value["begin"]["bytePosition"]),
                           ("end line"  , value["end"]["line"]    ),
                           ("end column", value["end"]["column"]  ),
                           ("end byte"  , value["end"]["bytePosition"]),
                         ]
        # получение int-значений по адресу переменных объектов, являющихся частями локейшна
        l = []
        for n, val in self.llocparts:
            l.append(0 if not val.address else d.extractInt(val.address))

        [self.ibLin, self.ibCol, self.ibByte, self.ieLin, self.ieCol, self.ieByte] = l
        self.fname = fname

    def __str__(self):
        if self.ibLin == self.ieLin:
           s = "%i,%i-%i"    % (self.ibLin, self.ibCol, self.ieCol)
        else:
           s = "%i,%i-%i,%i" % (self.ibLin, self.ibCol, self.ieLin, self.ieCol)
        return (s + "/" + self.fname) if self.fname else s


    def putHead(self, prefix = ""):
      strSelf = prefix + str(self)
      strGdbVal, l = gdb.Value(strSelf), len(strSelf)
      ora2lin_dumper.putStdString(self.d, strGdbVal, l, l)

    def putExpanded(self):
      self.d.putNumChild(1)
      if self.d.isExpanded():
        with Children(self.d):
          for name, val in self.llocparts:
            self.d.putSubItem(name, val)
