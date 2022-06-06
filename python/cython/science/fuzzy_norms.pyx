# -*- coding: utf-8 -*-

cdef extern from "float.h":
    cdef int DBL_EPSILON

from libc.math cimport log

cdef double eps = DBL_EPSILON

DEF p = 2.0

cdef inline int isnull(double x):
    return abs(x) < eps

cdef class TNorm:
    cpdef double t(self, double a, double b):
        return 0.0

    cpdef double s(self, double a, double b):
        return 0.0

cdef class Lukasiewicz(TNorm):
    """ Łukasiewicz t-norm.
    The name comes from the fact that the t-norm is the standard semantics for strong conjunction
    in Łukasiewicz fuzzy logic. It is a nilpotent Archimedean t-norm, pointwise smaller than the product t-norm.
    """
    cpdef double t(self, double a, double b):
       return max(a + b - 1.0, 0.0)

    cpdef double s(self, double a, double b):
        return min(a + b, 1.0)


cdef class MinMax(TNorm):
    """ Minimum t-norm (Gödel t-norm).
    Minimum t-norm, also called the Gödel t-norm, as it is the standard semantics for conjunction
    in Gödel fuzzy logic. Besides that, it occurs in most t-norm based fuzzy logics as the standard
    semantics for weak conjunction. It is the pointwise largest t-norm.
    """
    cpdef double t(self, double a, double b):
        return min(a, b)

    cpdef double s(self, double a, double b):
        return max(a, b)


cdef class Prod(TNorm):
    """ Product t-norm
    (the ordinary product of real numbers). Besides other uses, the product t-norm is the standard
    semantics for strong conjunction in product fuzzy logic. It is a strict Archimedean t-norm.
    """
    cpdef double t(self, double a, double b):
        return a * b

    cpdef double s(self, double a, double b):
        return a + b - a * b


cdef class Drastic(TNorm):
    """ Drastic t-norm
    The name reflects the fact that the drastic t-norm is the pointwise smallest t-norm (see the
    properties of t-norms below). It is a right-continuous Archimedean t-norm.
    """
    cpdef double t(self, double a, double b):
        return a if isnull(b - 1.0) else (b if isnull(a - 1.0) else 0.0)

    cpdef double s(self, double a, double b):
        return a if isnull(b) else (b if isnull(a) else 1.0)


cdef class Hamacher(TNorm):
    """ Hamacher t-norms
    The family of Hamacher t-norms, introduced by Horst Hamacher in the late 1970s. Hamacher
    t-norms are the only t-norms which are rational functions. The family is strictly decreasing and
    continuous with respect to p.
    """
    cpdef double t(self, double a, double b):
        ab = a * b
        return  ab / (p + (1.0 - p) * (a + b - ab))

    cpdef double s(self, double a, double b):
        ab = a * b
        return  (a + b - (2.0 - p) * ab) / (1.0 - (1.0 - p) * ab)


cdef class Dombi(TNorm):
    """ Dombi t-norms
    The family of Dombi t-norms, introduced by József Dombi (1982). The family is strictly increasing
    and continuous with respect to p.
    """
    cpdef double t(self, double a, double b):
        if isnull(a) or isnull(b):
            return 0.0
        sa = 1.0 / a - 1.0
        sb = 1.0 / b - 1.0
        ss = 0.0 if isnull(sa) else pow(sa, p) + (0.0 if isnull(sb) else pow(sb, p))
        return 1.0 / ( 1.0 + (0.0 if abs(ss) < eps else pow(ss, 1.0 / p)) )

    cpdef double s(self, double a, double b):
        if isnull(a) or isnull(b):
            return 1.0;
        sa = 1.0 / a - 1.0
        sb = 1.0 / b - 1.0
        if isnull(sa) or isnull(sb):
            return 0.0
        ss = pow(sa, -p) + pow(sb, -p)
        return 1.0 / ( 1.0 + (0.0 if isnull(ss) else pow(ss, 1.0 / p)))


cdef class Yager(TNorm):
    """ Yager t-norms
    The family of Yager t-norms, introduced in the early 1980s by Ronald R. Yager.
    The family is strictly increasing and continuous with respect to p.
    """
    cpdef double t(self, double a, double b):
        sa = 0.0 if abs(1.0 - a) < eps else pow(1.0 - a, p)
        sb = 0.0 if abs(1.0 - b) < eps else pow(1.0 - b, p)
        ss = sa + sb
        return max( 1.0 - (0.0 if abs(ss) < eps else pow(ss, 1.0 / p)), 0.0)

    cpdef double s(self, double a, double b):
        sa = 0.0 if isnull(a) else pow(a, p)
        sb = 0.0 if isnull(b) else pow(b, p)
        ss = sa + sb
        return min((0.0 if isnull(ss) else pow(ss, 1.0 / p)), 1.0)


cdef class Frank(TNorm):
    """ Frank t-norms
    The family of Frank t-norms, introduced by M.J. Frank in the late 1970s. The family is strictly
    decreasing and continuous with respect to p.
    """
    cpdef double t(self, double a, double b):
        sa = 0.0 if abs(a) < eps else pow(p, a) - 1.0
        sb = 0.0 if abs(b) < eps else pow(p, b) - 1.0
        return log( 1.0 + sa * sb / (p - 1.0) ) / log(p)

    cpdef double s(self, double a, double b):
        sa = 0.0 if isnull(1.0 - a) else pow(p, 1.0 - a) - 1.0
        sb = 0.0 if isnull(1.0 - b) else pow(p, 1.0 - b) - 1.0
        return 1.0 - log(1.0 + (sa + sb) / (p - 1.0)) / log(p)

    cpdef valid(self):
        return p > eps and not isnull(p - 1.0)


cdef class Sugeno(TNorm):
    """ Sugeno–Weber t-norms
    The family of Sugeno–Weber t-norms was introduced in the early 1980s by Siegfried Weber; the dual
    t-conorms were defined already in the early 1970s by Michio Sugeno. The family is strictly increasing
    and continuous with respect to p.
    """
    cpdef double t(self, double a, double b):
        return max((a + b - 1.0 + a * b) / (1.0 + p), 0.0)

    cpdef double s(self, double a, double b):
        return min(a + b + p * a * b, 1.0)


cdef class Nilpmin(TNorm):
    """ Nilpotent minimum
    is a standard example of a t-norm which is left-continuous, but not continuous. Despite its
    name, the nilpotent minimum is not a nilpotent t-norm.
    """
    cpdef double t(self, double a, double b):
        return min(a, b) if ((a + b) > 1.0) else 0.0

    cpdef double s(self, double a, double b):
        return max(a, b) if ((a + b) < 1.0) else 1.0


