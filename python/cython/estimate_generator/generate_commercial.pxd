# distutils: language = c++
# cython: language_level=3


cimport numpy as np
cimport generate_commercial
ctypedef np.float64_t DTYPE_t
cimport aggregate_commercial_internal

cpdef double iround(double value, double precision) except*
cpdef double ifloor(double value, double precision) except*
cpdef double iceil(double value, double precision) except*
cpdef int cround(double value) except*
cpdef int rur_round(double val, double DIV_COEFF) except*
cpdef double c_accumulate_round_val(x, prev_accum, double DIV_COEFF, double ROUND_COEFF, bint CONV_TYPE) except*

ctypedef void(*expand_mf)(aggregate_commercial_internal.Cathegory, generate_commercial.ItogClass, int) except*

cdef class ItogClass:
    cdef str k0
    cdef str k1
    cdef str k2

    cdef public list cathegories
    cdef public list descriptions
    cdef list fields
    cdef np.ndarray percent
    cpdef public np.ndarray data
    cdef public double naklad_percent
    cdef list container
    cdef double DIV_COEFF
    cdef double ROUND_COEFF
    cdef bint CONV_TYPE

    cpdef object c_init(self, object agctx, object cfg, object container, list fields, str itog_str)
    cdef int add_naklad_np_column(self, prev_accum, bint is_minimizer_solver) except *
    cpdef init_for_works(self, prev_accum, double naklad_percent, bint is_minimizer_solver)
    cpdef init_for_compl(self, prev_accum, double naklad_percent, bint is_minimizer_solver)
    cpdef init_for_transp(self, prev_accum, double naklad_percent, bint is_minimizer_solver)
    cpdef init_for_nakl(self, prev_accum, bint is_minimizer_solver)

    cpdef list convert_np_to_list(self)
    cpdef set_percent(self, dogovor_price)
    cdef correct_rest(self, int i0, object prev_accum)
    cdef void _clear_empty_summary_row(self) except*
    cdef void _init_common(self, prev_accum, double naklad_percent, bint is_minimizer_solver, expand_mf mf) except*

