#!/usr/bin/env python3
# coding=utf8

import os
import math

def transform_fname(fname, dstdir):
    name_base = 'A' + os.path.basename(fname).split('.')[0]
    new_path = os.path.join(dstdir, name_base)
    return new_path


class TransformContext:
    def __init__(self, fname, defaults, print_fun, dstdir):
        self.print = print_fun
        self.defaults_setted = False
        self.defaults_s_setted = False
        self.D = defaults
        self.m_command_prev = None
        self.m_command = None
        self.coordinates_absolute = True
        self.x_prev = 0.0
        self.y_prev = 0.0
        self.x = 0.0
        self.y = 0.0
        self.f = 0.0
        self.f_misc = 0.0
        self.f_prev = 0.0
        self.xobj_prev = None
        self.yobj_prev = None
        self.gobj_prev = set()
        self.gobj9091_prev = set()
        self.sobj_prev = set()
        self.fobj_prev = None
        self.m2225_prev = None
        self.x_rel_prev = None
        self.y_rel_prev = None
        self.x_rel = None
        self.y_rel = None

        self.fname = fname
        self.fname_tr = transform_fname(fname, dstdir)

        self.i_prev = None
        self.j_prev = None
        self.i = None
        self.j = None
        self.g010203_prev = None
        self.g010203 = XYJ('G', NumVal("01"))
        self.s_prev = None
        self.s = None
        self.cadr = None
        self.f_h_coefficient = self.D.F_M25_H / 10.0  # коэффициент прямой для вычисления значения подачи в режиме H M25 (подача 750 на 10 мм)

    def get_distance(self):
        delta_x = float(self.x_prev) - self.x
        delta_y = float(self.y_prev) - self.y
        return math.sqrt(delta_x * delta_x + delta_y * delta_y)
    
    def get_proportional_f(self, delta):
        res = int(round(self.f_h_coefficient * delta))
        return res if res < 6000 else 6000
    
    def next_m_command(self, m_command):
        self.m_command_prev = self.m_command
        self.m_command = m_command

    def next_s_command(self, s_command):
        self.s_prev = self.s
        self.s = s_command

    def update_cadr(self, cadr):
        self.cadr = cadr

    def update_i(self, i):
        self.i_prev = self.i
        self.i = i

    def update_j(self, j):
        self.j_prev = self.j
        self.j = j

    def update_g010203(self, g):
        self.g010203_prev = self.g010203
        self.g010203 = g

    def update_x(self, x):
        self.x_prev = self.x
        if self.coordinates_absolute:
            self.x = x
        else:
            self.x = self.x + x
            self.x_rel_prev = self.x_rel
            self.x_rel = self.x_rel_prev + x

    def update_f(self, f):
        self.f_prev = self.f
        self.f = f
        self.f_misc = f

    def update_y(self, y):
        self.y_prev = self.y
        if self.coordinates_absolute:
            self.y = y
        else:
            self.y = self.y + y
            self.y_rel_prev = self.y_rel
            self.y_rel = self.y_rel_prev + y

    def set_absolute(self, has_absolute):
        self.coordinates_absolute = has_absolute
        if has_absolute:
            self.x_rel_prev = None
            self.y_rel_prev = None
            self.x_rel = None
            self.y_rel = None
        else:
            self.x_rel_prev = 0.0
            self.y_rel_prev = 0.0
            self.x_rel = 0.0
            self.y_rel = 0.0


def trim_float(sfloat):
    if len(sfloat) > 2 and sfloat[-1] == '0' and sfloat[-2] == '.':
        return sfloat[:-2]
    return sfloat

class BaseCadr:
    has_number = False

    def __init__(self):
        pass

class BaseCadrN:
    has_number = True

    def __init__(self):
        pass

class CommonTransformMixin:
    def __init__(self):
        self.num = None
        self.body = None
        self.comment_empty = True
        self.has_xyj_conversion = False

    def transform(self, context):
        pass


class DisCommand(BaseCadr, CommonTransformMixin): # команда DIS
    def __init__(self, br_entry):
        self.skip_m = True
        self.br_entry = br_entry
        self.is_dis = True
        self.cat = 'DIS'
        CommonTransformMixin.__init__(self)

    def __str__(self):
        return '(DIS,"{}")'.format(str(self.br_entry))
    

class ClsCommand(BaseCadr, CommonTransformMixin):
    def __init__(self, subprogramm):
        self.skip_m = True
        self.subprogram = subprogramm
        self.cat = 'CLS'
        CommonTransformMixin.__init__(self)
    
    def __str__(self):
        return "(CLS,{})".format(self.subprogram)


class SCmd(BaseCadrN, CommonTransformMixin):
    def __init__(self, cmd):
        self.cmd = cmd
        self.skip_m = True
        self.cat = 'SCMD'
        CommonTransformMixin.__init__(self)
    
    def __str__(self):
        return self.cmd

    
class InternalRpt(BaseCadrN, CommonTransformMixin):
    def __init__(self, rpt):
        self.skip_m = True
        self.rpt = rpt
        self.cat = 'RPT'
        CommonTransformMixin.__init__(self)
    
    def __str__(self):
        return "(RPT,{})".format(self.rpt)

class InternalErp(BaseCadrN, CommonTransformMixin):
    def __init__(self):
        self.skip_m = True
        self.cat = 'ERP'
        CommonTransformMixin.__init__(self)
    
    def __str__(self):
        return "(ERP)"


class Program:
    def __init__(self):
        self.comment = None
        self.commands = []

    def toStrByte(self, context, print_fun):
        # if self.comment:
        #     return self.comment.tobytes() + "\r\n".join([str(s) for s in self.commands]).encode()
        return "\r\n".join([s.toString(context, print_fun) for s in self.commands]).encode()



class Cadr(BaseCadrN):
    def __init__(self, num=None, body=None, m_command=None, skip_m=False):
        self.num = None
        if body:
            self.body = body
        else:
            self.body = []
        self.has_number = True
        self.m_command = m_command
        self.error = False
        self.skip_m = skip_m
        self.src_cadr = None
        self.label = ""
        self.is_subprogram = False

    @staticmethod
    def generate_label(label):
        assert(len(label) < 6)
        while len(label) < 6:
            label = label + 'L'
        return label

    def set_label(self, label):
        self.label = '"' + self.generate_label(label) + '"'

    def toString(self, context, print_fun):
        if self.num is not None:
            snum = "N{} ".format(self.num)
        else:
            snum = ""
        # snum = "" # TODO
        if context.D.VERBOSE_FROM_CADR and self.src_cadr:
            try:
                suffix = ' (' + self.src_cadr.ltext(context.fname) + ')'
            except AttributeError:
                suffix = ' ({})'.format(self.src_cadr.src_cadr.ltext(context.fname) if self.src_cadr.src_cadr else "")
            
        else:
            suffix = ''

        if not self.m_command:
            res = "{0}{1}".format(snum, " ".join([str(s) for s in self.body]))
        elif self.body:
            res = "{0}{1} {2}".format(snum, " ".join([str(s) for s in self.body]), " ".join([str(s) for s in self.m_command]))
        else:
            res = "{0}{1}".format(snum, " ".join([str(s) for s in self.m_command]))
        if self.error:
            res = res + " <- ERROR IN CADR!"
            print_fun('{} error in cadr {} {}', context.fname, context.fname_tr, self.src_cadr.ltext(context.fname) if self.src_cadr else res)
        return self.label + res + suffix


XY_CAT_100 = {"X", "Y", "I", "J"}
G_CAT_SAVE = {1, 2, 3}

def g_conversions(self, context):
    if self.val.ivalue == 90:
        context.set_absolute(True)    
    elif self.val.ivalue == 91:
        context.set_absolute(False)    
    elif self.val.ivalue in G_CAT_SAVE:
        context.update_g010203(self)


XY_CAT = {
    "X": (lambda self, context: context.update_x(self.val.value)), 
    "Y": (lambda self, context: context.update_y(self.val.value)), 
    "I": (lambda self, context: context.update_i(self)), 
    "F": (lambda self, context: context.update_f(self.val.value)), 
    "J": (lambda self, context: context.update_j(self)),
    "G": g_conversions,
    "S": (lambda self, context: context.next_s_command(self))
}

class XYJ:
    """ x y h f s e i j tr g"""

    def __init__(self, cat, val):
        if cat == 'S' and val.value > 2:
            raise Exception("S-val > 2, S" + val.svalue)

        self.cat = cat
        self.val = val

    def copy(self):
        new_val = self.val.copy()
        return XYJ(self.cat, new_val)

    def transform(self, context):
        if self.cat in XY_CAT_100:
            self.val.div100()
        elif self.cat == 'E': # E<соток на ход> преобразуется в F<соток на ход / 100>) * 175 ходов в минуту * коэффициент коррекции
            self.cat = 'F'
            fval = int(round((self.val.value / 100.0) * context.D.STEPS_BY_MINUTE * context.D.CORRECTION_COEFFICIENT, 0))
            context.update_f(fval)
            self.val.set_transformed(fval)

        
        act = XY_CAT.get(self.cat, None)
        if act:
            act(self, context)

        return self

    def __str__(self):
        assert self.val
        return "{}{}".format(self.cat, str(self.val))

    def __format__(self, fmt):
        return str(self)


class NumVal:
    def __init__(self, svalue):
        if svalue is not None:
            self._plus_sign = (svalue and len(svalue) and svalue[0] == '+') 
            self._value = float(svalue) if svalue else None
            self._ivalue = int(svalue) if svalue else None
        else:
            self._plus_sign = False
            self._value = None
            self._ivalue = None
        self._svalue = svalue
        self._transformed = False
        self._need_plus = False

    @property
    def ivalue(self):
        return self._ivalue

    @property
    def value(self):
        return self._value

    def div100(self):
        self._transformed = True
        self._value = round(self.value / 100.0, 2)

    @classmethod
    def num(cls, num):
        instance = cls(None)
        instance._value = num
        instance._transformed = True
        return instance

    @classmethod
    def plus(cls, num):
        instance = cls(None)
        instance._value = num
        instance._transformed = True
        instance._need_plus = True
        return instance
    
    def copy(self):
        instance = NumVal(None)
        instance._plus_sign = self._plus_sign 
        instance._value = self._value
        instance._ivalue = self._ivalue
        instance._svalue = self._svalue
        instance._transformed = self._transformed
        instance._need_plus = self._need_plus
        return instance


    def __str__(self):
        if not self._transformed:
            return self._svalue
        if self._need_plus:
            if self._value > 0:
                return "+" + trim_float(str(round(self._value, 2)))
            else:
                return trim_float(str(round(self._value, 2)))
        return trim_float(str(round(self._value, 2)))

    def __format__(self, fmt):
        return str(self)

    def set_transformed(self, new_val):
        self._value = new_val
        self._ivalue = int(round(new_val))
        self._transformed = True
        return self


class XYJ_SPEC(XYJ):
    def __init__(self, cat, val):
        self.comment_empty = True
        self.has_xyj_conversion = False
        XYJ.__init__(self, cat, val)

    def __str__(self):
        if not self.val:
            return "{}".format(self.cat)
        return "{}{}".format(self.cat, str(self.val))
