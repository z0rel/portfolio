#ifndef MT19937_TYPES_H
#define MT19937_TYPES_H


#include <stdint.h>
#include <stddef.h>


/** Period parameters */
#define RAND_MT19937_N 624


typedef uint32_t rand_mt19937_val_t;


struct rand_mt19937
{
    /** The array for the state vector */
    rand_mt19937_val_t *p0, *p1, *pm;
    rand_mt19937_val_t x[RAND_MT19937_N];
};


/* clang-format off */
#define RAND_MT19937_INITIALIZER { NULL, NULL, NULL, {0} }
/* clang-format on */


#endif // MT19937_TYPES_H
