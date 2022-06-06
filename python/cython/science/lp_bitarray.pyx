
from cpython.mem cimport PyMem_Malloc, PyMem_Free
from cython.operator cimport preincrement
from bitarray._bitarray import _bitarray as native_bitarray

cdef size_t offsets_by_sizeof[17]
offsets_by_sizeof[0] = 0
offsets_by_sizeof[1] = 3
offsets_by_sizeof[2] = 4
offsets_by_sizeof[3] = 4
offsets_by_sizeof[4] = 5
offsets_by_sizeof[5] = 5
offsets_by_sizeof[6] = 5
offsets_by_sizeof[7] = 5
offsets_by_sizeof[8] = 6
offsets_by_sizeof[9] = 6
offsets_by_sizeof[10] = 6
offsets_by_sizeof[11] = 6
offsets_by_sizeof[12] = 6
offsets_by_sizeof[13] = 6
offsets_by_sizeof[14] = 6
offsets_by_sizeof[15] = 6
offsets_by_sizeof[16] = 7

#ctypedef size_t TYPE
#ctypedef size_t TYPE

cdef extern from "<string.h>":
    void * memcpy ( void * destination, const void * source, size_t num )
    void * memset ( void * ptr, int value, size_t num )


cdef class bitarray:
    def __cinit__(self, Py_ssize_t length):
        self.length = length
        self.arr = <TYPE*>PyMem_Malloc(length * sizeof(TYPE))

    cpdef clear(self):
        cdef TYPE *parr
        cdef Py_ssize_t i

        parr = self.arr
        for i in range(0, self.length):
            parr[0] = 0
            preincrement(parr)

    def __dealloc__(self):
        PyMem_Free(self.arr)

    cpdef bytes hash_value(self):
        cdef char* c_string = NULL
        cdef bytes py_string
        c_string = <char*>self.arr
        py_string = c_string[:self.length*sizeof(TYPE)]
        return py_string

    cdef void str_hash_value(self, string &dst):
        dst.clear()
        dst.assign(<char*>self.arr, self.length*sizeof(TYPE))

    cpdef TYPE bit_test(self, TYPE index):
        cdef TYPE bindex
        cdef int bitpos

        bindex = index >> offsets_by_sizeof[sizeof(TYPE)]
        bitpos = index - (bindex << offsets_by_sizeof[sizeof(TYPE)])

        return self.arr[bindex] & (1LL << bitpos) != 0

    #def __getitem__(self, item):
    #    return self.bit_test(item) != False

    cpdef void bit_set(self, TYPE index):
        cdef TYPE bindex
        cdef int bitpos

        bindex = index >> offsets_by_sizeof[sizeof(TYPE)]
        bitpos = index - (bindex << offsets_by_sizeof[sizeof(TYPE)])

        self.arr[bindex] |= (1LL << bitpos)

    cpdef void bit_clr(self, TYPE index):
        cdef TYPE bindex
        cdef int bitpos

        bindex = index >> offsets_by_sizeof[sizeof(TYPE)]
        bitpos = index - (bindex << offsets_by_sizeof[sizeof(TYPE)])

        self.arr[bindex] &= ~(1LL << bitpos)

    cpdef int any(self):
        cdef size_t cnt
        cdef TYPE bindex, bitpos
        cdef int end_index
        cdef TYPE *parr

        cnt = 0
        parr = self.arr
        end_index = sizeof(TYPE) << 3
        for bindex in range(0, self.length):
            for bitpos in range(0, end_index):
                if parr[0] & (1LL << bitpos):
                    return True

            preincrement(parr)
        return False

    cpdef size_t count(self):
        cdef size_t cnt
        cdef TYPE bindex, bitpos
        cdef int end_index
        cdef TYPE *parr

        cnt = 0
        parr = self.arr
        end_index = sizeof(TYPE) << 3
        for bindex in range(0, self.length):
            for bitpos in range(0, end_index):
                if parr[0] & (1LL << bitpos):
                    preincrement(cnt)

            preincrement(parr)
        return cnt

    cpdef bitarray copy(self):
        cdef bitarray newobj
        newobj = bitarray.__new__(bitarray, self.length)
        memcpy(newobj.arr, self.arr, self.length * sizeof(TYPE))
        return newobj

    def to_bytelist(self):
        return ["{0:b}".format(self.arr[x]) for x in range(0, self.length)]

    def get_length(self):
        return self.length

    def __iand__(self, bitarray y):
        cdef Py_ssize_t cmplen
        cdef size_t i
        cdef TYPE *dst
        cdef TYPE *src
        dst = self.arr
        src = y.arr

        cmplen = min(self.length, y.length)
        for i in range(0, cmplen):
            dst[0] &= src[0]
            preincrement(dst)
            preincrement(src)

        return self

    def __and__(bitarray x, bitarray y):
        copyobj, other = (x,y) if x.length > y.length else (y,x)
        copied_obj = copyobj.copy()
        copied_obj &= other
        return copied_obj


cpdef bitarray make_bitarray(Py_ssize_t bit_length):
    cdef TYPE chuncks_length
    cdef bitarray obj
    chuncks_length = 1 + (bit_length >> offsets_by_sizeof[sizeof(TYPE)])
    obj = bitarray.__new__(bitarray, chuncks_length)
    obj.clear()
    return obj



class native_inf_bitarray(native_bitarray):
    def bit_test(self, index):
        return self[index]

    def bit_set(self, index):
        self[index] = True


    def hash_value(self):
        return self.tobytes()


def native_make_bitarray(bit_length):
    obj = native_inf_bitarray(bit_length)
    obj.setall(False)
    return obj
