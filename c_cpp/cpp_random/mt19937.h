/**
 * @file
 * @brief MT19937: Pseudo-random generator, the Mersenne Twister implementation.
 */
#ifndef MT19937_H
#define MT19937_H

#include <stddef.h>


#include "likely.h"
#include "platform/mt19937_types.h"


/** Period parameter */
#define RAND_MT19937_M 397


/** Constant vector a */
#define RAND_MT19937_MATRIX_A 0X9908B0DFUL


/** Most significant w-r bits */
#define RAND_MT19937_UPPER_MASK 0x80000000UL


/** Least significant r bits */
#define RAND_MT19937_LOWER_MASK 0x7fffffffUL


/**
 * Initialize with a seed
 * @param self random generator context.
 * @param s    seed value
 */
static inline void
rand_mt19937_srand(struct rand_mt19937 *self, rand_mt19937_val_t s)
{
    int i;

    self->x[0] = s & 0xffffffffUL;

    /* for >32 bit machines */
    for (i = 1; i < RAND_MT19937_N; ++i)
        self->x[i] = (1812433253UL * (self->x[i - 1] ^ (self->x[i - 1] >> 30)) + i) & 0xffffffffUL;

    self->p0 = self->x;
    self->p1 = self->x + 1;
    self->pm = self->x + RAND_MT19937_M;
}


/**
 *  Initialize by an array with array-length
 *  @param self       random generator context.
 *  @param init_key   is the array for initializing keys.
 *  @param key_length is its length.
 */
static inline void
rand_mt19937_srand_arr(struct rand_mt19937 *self, rand_mt19937_val_t init_key[], int key_length)
{
    int i, j, k;

    rand_mt19937_srand(self, 19650218UL);
    i = 1;
    j = 0;
    for (k = (RAND_MT19937_N > key_length ? RAND_MT19937_N : key_length); k; --k)
    {
        /* non linear */
        self->x[i] =
            ((self->x[i] ^ ((self->x[i - 1] ^ (self->x[i - 1] >> 30)) * 1664525UL)) + init_key[j]
             + j)
            & 0xffffffffUL; /* for WORDSIZE > 32 machines */
        if (++i >= RAND_MT19937_N)
        {
            self->x[0] = self->x[RAND_MT19937_N - 1];
            i = 1;
        }
        if (++j >= key_length)
            j = 0;
    }
    for (k = RAND_MT19937_N - 1; k; --k)
    {
        /* non linear */
        self->x[i] = ((self->x[i] ^ ((self->x[i - 1] ^ (self->x[i - 1] >> 30)) * 1566083941UL)) - i)
                     & 0xffffffffUL; /* for WORDSIZE > 32 machines */
        if (++i >= RAND_MT19937_N)
        {
            self->x[0] = self->x[RAND_MT19937_N - 1];
            i = 1;
        }
    }
    self->x[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */
}


/**
 * Generate the random number in the interval [0,0xffffffff]
 * @param self  random generator context.
 */
static inline rand_mt19937_val_t
rand_mt19937_rand_uint32(struct rand_mt19937 *self)
{
    rand_mt19937_val_t y;

    /* Default seed */
    if (unlikely(self->p0 == NULL))
        rand_mt19937_srand(self, 5489UL);

    /* Twisted feedback */
    y = *(self->p0) =
        *(self->pm)++
        ^ (((*(self->p0) & RAND_MT19937_UPPER_MASK) | (*(self->p1) & RAND_MT19937_LOWER_MASK)) >> 1)
        ^ (-(*(self->p1) & 1) & RAND_MT19937_MATRIX_A);

    (self->p0) = (self->p1)++;
    if (unlikely(self->pm == self->x + RAND_MT19937_N))
        self->pm = self->x;

    if (unlikely(self->p1 == self->x + RAND_MT19937_N))
        self->p1 = self->x;

    /* Temper */
    y ^= y >> 11;
    y ^= y << 7  & 0X9D2C5680UL;
    y ^= y << 15 & 0XEFC60000UL;
    y ^= y >> 18;
    return y;
}


/**
 * Generate a random number on the real interval [0,1]
 * @param self  random generator context.
 */
static inline double
rand_mt19937_rand_double(struct rand_mt19937 *self)
{
    return rand_mt19937_rand_uint32(self) * (1.0 / 4294967295.0);
    /* divided by 2^32-1 */
}


/**
 * Generates a random number on the real interval [0,1)
 * @param self  random generator context.
 */
static inline double
rand_mt19937_rand_double_semiopen(struct rand_mt19937 *self)
{
    return rand_mt19937_rand_uint32(self) * (1.0 / 4294967296.0);
}


/**
 * Generate a random number on the real interval (0,1)
 * @param self  random generator context.
 */
static inline double
rand_mt19937_rand_double_open(struct rand_mt19937 *self)
{
    return (((double)rand_mt19937_rand_uint32(self)) + 0.5) * (1.0 / 4294967296.0);
    /* divided by 2^32 */
}


#endif // MT19937_H
