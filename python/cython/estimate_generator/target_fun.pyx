# distutils: language = c++
# cython: language_level=3

cimport numpy as np
ctypedef np.float64_t DTYPE_t
from cython.operator cimport dereference as deref, preincrement as inc
from libcpp.vector cimport vector
import sys

from xlcommercial.calc.estimate.misc import UninitializedException
from parsing_context cimport AggregatorObject
from aggregate_commercial_internal cimport Cathegory

from xlcommercial.calc.estimate.misc import StopMinimization
from xlcommercial.calc.estimate.prices import generate_json_commercial_minimizer

cdef extern from "cmath":
    double fabs(double x)
    double sqrt(double x)


cdef class TargetFun:
    cdef public int stage
    cdef public int eq_cnt
    cdef public bint prev_val_setted
    cdef public DTYPE_t prev_val
    cpdef public object founded_x0
    cdef public list dst_aggr_works
    cdef public vector[DTYPE_t] target_values
    cdef public object dogovor
    cdef public double last_result
    cdef public int ncnt

    def __cinit__(self,
                  int stage,
                  list dst_aggr_works,
                  target_values,
                  object dogovor):
        cdef float val
        self.ncnt = 0
        self.stage = stage
        self.eq_cnt = 0
        self.prev_val = 0.0
        self.prev_val_setted = False
        self.founded_x0 = None
        self.dst_aggr_works = dst_aggr_works
        self.dogovor = dogovor
        self.last_result = -10000

        for val in target_values:
            self.target_values.push_back(val)

    cpdef double target_fun(self, x0) except*:
        cdef Cathegory current_wrk
        cdef int i = 0
        cdef int idx
        cdef object agctx = self.dogovor.agctx
        cdef DTYPE_t fval
        cdef DTYPE_t target_val
        cdef vector[DTYPE_t].iterator vec_it
        cdef DTYPE_t diff
        cdef DTYPE_t sumval = 0.0


        for current_wrk in self.dst_aggr_works:
            current_wrk.set_local_correcture(x0[i])
            inc(i)

        agctx.update_summaries_in_correctures() # slow - need if transp correctures
        self.dogovor.generate()

        idx = 0
        vec_it = self.target_values.begin()
        cdef DTYPE_t s1 = 0.0
        cdef DTYPE_t s2 = 0.0
        if self.stage == 1:
            for current_wrk in self.dst_aggr_works:
                target_val = deref(vec_it)
                fval = current_wrk.cb_itog_data[current_wrk.cb_itog_data_idx, 2]
                diff = fval - target_val
                sumval += diff
                inc(vec_it)
        elif self.stage == 2:
            for current_wrk in self.dst_aggr_works:
                target_val = deref(vec_it)
                fval = current_wrk.cb_itog_data[current_wrk.cb_itog_data_idx, 2]
                diff = fval - target_val
                sumval += diff * diff
                # print(current_wrk.cathegory_val, fval, target_val, diff)

                s1 +=target_val
                s2 +=fval
                inc(vec_it)
        else:
            for current_wrk in self.dst_aggr_works:
                target_val = deref(vec_it)
                if current_wrk.cb_itog_data is None:
                    raise UninitializedException()
                fval = current_wrk.cb_itog_data[current_wrk.cb_itog_data_idx, 2]
                diff = fval - target_val
                sumval += fabs(diff)
                inc(vec_it)


        # if 0:
        #     if self.ncnt < 9:
        #         print(int(round(sumval)), " ", end="")
        #         sys.stdout.flush()
        #     else:
        #         print(int(round(sumval)))
        #         self.ncnt = 0
        #     self.ncnt += 1

        # nonlocal prev_val
        # nonlocal founded_x0
        # if self.prev_val_setted and self.prev_val == sumval:
        #     # nonlocal eq_cnt
        #     self.eq_cnt += 1
        #     if self.eq_cnt > 500:
        #         self.founded_x0 = x0[:]
        #         raise StopMinimization
        # else:
        #     self.prev_val = sumval
        #     self.prev_val_setted = True
        #     self.eq_cnt = 0

        cdef double ressum
        if self.stage == 1:
            ressum = fabs(sumval)
        elif self.stage == 2:
            ressum = sqrt(sumval)
        else:
            ressum = fabs(sumval)

        # if fabs(round(ressum, 1)) < 0.03:
        #     self.founded_x0 = x0[:]
        #     raise StopMinimization
        self.last_result = ressum

        return ressum
