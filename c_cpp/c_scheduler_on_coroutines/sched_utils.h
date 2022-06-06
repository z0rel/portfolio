#ifndef SCHED_UTILS_H
#define SCHED_UTILS_H


#include <stdatomic.h>


struct sched_int_median
{
    int val;
    int item;
};


struct sched_int_median_result
{
    double val;
    int item;
    int stub; /* supress padding warnings */
};


struct sched_int_median_result
sched_get_median(atomic_int *array, int i_size);


#endif // SCHED_UTILS_H
