# distutils: language = c++
# cython: language_level=3

from cython.operator cimport dereference as deref, preincrement as inc
import numpy as np
cimport numpy as np
from libcpp.utility cimport pair
from aggregate_commercial_internal cimport Cathegory
cimport aggregate_commercial_internal

# ctypedef void (*expand_mf)(Cathegory, generate_commercial.ItogClass dst, int idx) except*


cdef extern from "cmath":
    double c_round "round"(double x)
    double c_floor "floor"(double x)
    double c_ceil "ceil"(double x)

cpdef double iround(double value, double precision) except*:
    #assert(precision == 10000.0 or precision == 1000.0 or precision == 100.0 or precision == 10.0 or precision == 1.0 or precision == 0.1)
    return c_round(value * precision) / precision

cpdef double ifloor(double value, double precision) except*:
    #assert(precision == 10000.0 or precision == 1000.0 or precision == 100.0 or precision == 10.0 or precision == 1.0 or precision == 0.1)
    return c_floor(value * precision) / precision

cpdef double iceil(double value, double precision) except*:
    #assert(precision == 10000.0 or precision == 1000.0 or precision == 100.0 or precision == 10.0 or precision == 1.0 or precision == 0.1)
    return c_ceil(value * precision) / precision

cpdef int cround(double value) except*:
    return int(c_round(value))

cpdef int rur_round(double val, double DIV_COEFF) except*:
    return int(cround(val / DIV_COEFF))

cdef str format_cathegory(double x):
    cdef str fmt
    if not x:
        return ""
    if iround(x, 1000) == iround(x, 10000):
        fmt = "{0:.3f}"
        if iround(x, 1000) == round(x, 100):
            fmt = "{0:.2f}"
            if iround(x, 100) == round(x, 10):
                fmt = "{0:.1f}"
                if iround(x, 10) == iround(x, 1):
                    fmt = "{0}"
                    x = int(x)
    else:
        fmt =  "{0:.4f}"
    return fmt.format(x).replace('.', ',')


cdef int c_set_accumulated_round_np(np.ndarray[DTYPE_t, ndim=2] src, int src_col, int dst_col, prev_accum, bint is_minimizer_solver, double DIV_COEFF, double ROUND_COEFF, bint CONV_TYPE) except*:
    """Установить в ключе-приемнике округленное значение из ключа источника с накоплением и распределением остатка округления"""

    cdef Py_ssize_t len_src = len(src) - 1
    cdef Py_ssize_t last_i = len(src) - 1
    cdef int i0 = -1
    cdef Py_ssize_t num = 0
    cdef DTYPE_t x_r
    cdef DTYPE_t x
    cdef DTYPE_t t
    cdef DTYPE_t offset = 0.0
    cdef Py_ssize_t idx
    cdef DTYPE_t summary = 0.0
    t =  prev_accum.t # t

    if not is_minimizer_solver:
        for num in range(len_src):
            x = src[num, src_col]
            x_r = iround(x, ROUND_COEFF)

            t += x - x_r
            if t >= 1000.0 or (num == last_i and t > 500.0):
                result = x_r / DIV_COEFF + 1.0
                t -= 1000.0
            elif t <= -1000.0 or (num == last_i and t < -500.0):
                result = x_r / DIV_COEFF - 1.0
                t += 1000.0
            else:
                result = x_r / DIV_COEFF

            if result:
                i0 = num
            # print(num, result, offset, x, x_r)
            summary += result
            src[num, dst_col] = result

    else:
        for num in range(len_src):
            x_r = src[num, src_col] / DIV_COEFF
            src[num, dst_col] = x_r
            summary += x_r

    src[len_src, dst_col] = summary

    prev_accum.t = t
    return i0


cpdef double c_accumulate_round_val(x, prev_accum, double DIV_COEFF, double ROUND_COEFF, bint CONV_TYPE) except*:
    cdef double t
    cdef double x_r
    cdef int offset
    cdef double ret
    t = prev_accum.t
    x_r = iround(x, ROUND_COEFF)

    t += x - x_r
    offset = 0
    if t >= 1000.0:
        offset = 1
        t -= 1000
    elif t <= -1000.0:
        offset = -1
        t += 1000

    if CONV_TYPE:
        ret = float(x_r / DIV_COEFF) + offset
    else:
        ret = int(x_r / DIV_COEFF) + offset

    prev_accum.t = t
    return ret


cdef class ItogClass:

    def __cinit__(self):
        pass

    cpdef object c_init(self, object agctx, object cfg, object container, list fields, str itog_str):
        cdef Cathegory wrk
        cdef int idx = 0
        self.k0='all_with_nakl'
        self.k1='nakl'
        self.k2='clear'
        self.cathegories = []
        self.descriptions = []
        self.fields = fields
        self.percent = None
        self.container = container
        self.data = np.empty(shape=(len(self.container)+1, 3), dtype=np.float64)
        self.naklad_percent = 0.0
        self.DIV_COEFF = cfg.DIV_COEFF
        self.ROUND_COEFF = cfg.ROUND_COEFF
        self.CONV_TYPE = cfg.CONV_TYPE

        for wrk in self.container:
            wrk.append_cathegory_and_description(self, idx)
            inc(idx)

        self.cathegories.append(None)
        self.descriptions.append(itog_str)
        return self

    def __getstate__(self):
        return (self.k0, self.k1, self.k2, self.cathegories, self.descriptions, self.fields, self.percent,
                self.data, self.naklad_percent, self.container, self.DIV_COEFF, self.ROUND_COEFF, self.CONV_TYPE)

    def __setstate__(self, state):
        (self.k0, self.k1, self.k2, self.cathegories, self.descriptions, self.fields, self.percent,
                self.data, self.naklad_percent, self.container, self.DIV_COEFF, self.ROUND_COEFF, self.CONV_TYPE) = state

    cdef void _clear_empty_summary_row(self) except*:
        cdef np.ndarray[DTYPE_t, ndim=2] data = self.data
        data[0, 0] = 0.0
        data[0, 1] = 0.0
        data[0, 2] = 0.0

    cdef void _init_common(self, prev_accum, double naklad_percent, bint is_minimizer_solver, expand_mf mf) except*:
        cdef Cathegory wrk
        cdef int i = 0
        cdef int i0

        self.naklad_percent = naklad_percent

        for wrk in self.container:
            mf(wrk, self, i)
            inc(i)

        if prev_accum is None:
            self._clear_empty_summary_row()
        else:
            i0 = self.add_naklad_np_column(prev_accum, is_minimizer_solver)
            self.correct_rest(i0, prev_accum)

    cpdef init_for_works(self, prev_accum, double naklad_percent, bint is_minimizer_solver):
        self._init_common(prev_accum, naklad_percent, is_minimizer_solver, Cathegory.expand_works_obj)
        return self

    cpdef init_for_compl(self, prev_accum, double naklad_percent, bint is_minimizer_solver):
        self._init_common(prev_accum, naklad_percent, is_minimizer_solver, Cathegory.expand_compl_obj)
        return self

    cpdef init_for_transp(self, prev_accum, double naklad_percent, bint is_minimizer_solver):
        self._init_common(prev_accum, naklad_percent, is_minimizer_solver, Cathegory.expand_transp_obj)
        return self

    cpdef init_for_nakl(self, prev_accum, bint is_minimizer_solver):
        self.k0='generate_naklad_compl'
        self.k1='generate_naklad_fot'
        self.k2='generate_naklad_itogo'
        self._init_common(prev_accum, 0.0, is_minimizer_solver, Cathegory.expand_nakl_obj)
        return self

    cdef correct_rest(self, int i0, object prev_accum):
        cdef np.ndarray[DTYPE_t, ndim=2] data = self.data
        if i0 >= 0:
            rest = int(data[-1, 2]) % 10
            if rest == 1:
                data[-1, 2] -= 1
                data[i0, 2] -= 1
                prev_accum.t -= 1.0
            elif rest == 9:
                data[-1, 2] += 1
                data[i0, 2] += 1
                prev_accum.t += 1.0

    cdef int add_naklad_np_column(self, prev_accum, bint is_minimizer_solver) except *:
        cdef int i0

        with prev_accum(self.k0):  # столбец с накладными
            i0 = c_set_accumulated_round_np(self.data,  # src
                                            2, 2,
                                            # dst_key_tousand_rur = 'ФОТ тыс. руб.' или 'Комплектующие, тыс. руб.'
                                            prev_accum, is_minimizer_solver, self.DIV_COEFF, self.ROUND_COEFF, self.CONV_TYPE)

        with prev_accum(self.k0):  # -> 'Накладные тыс.руб'
            c_set_accumulated_round_np(self.data,  # 'Накладные'
                                       1, 1,
                                       prev_accum, is_minimizer_solver, self.DIV_COEFF, self.ROUND_COEFF, self.CONV_TYPE)

        with prev_accum(self.k0):  # столбец исходных показателей
            c_set_accumulated_round_np(self.data,  # 'ФОТ', 'Комплектующие', 'Транспортные'
                                       0, 0,
                                       prev_accum, is_minimizer_solver, self.DIV_COEFF, self.ROUND_COEFF, self.CONV_TYPE)
        return i0


    cpdef list convert_np_to_list(self):
        cdef list result = [self.fields]
        cdef Py_ssize_t len_src = len(self.cathegories) - 1
        cdef Py_ssize_t idx

        for idx in range(len_src):
            # print(self.cathegories, self.descriptions, self.data, self.percent)
            result.append([format_cathegory(self.cathegories[idx]), idx + 1, self.descriptions[idx],
                                           *(self.data[idx]), self.percent[idx]])

        result.append(['', len_src + 1, self.descriptions[len_src], *(self.data[len_src]), self.percent[len_src]])

        return result

    cpdef set_percent(self, dogovor_price):
        """
           Установить столбец процента показателя от стоимости договора
        :param dogovor_price: итоговая стоимость договора
        """
        self.percent = 100.0 * np.around(self.data[:, 2] / dogovor_price, 3)



