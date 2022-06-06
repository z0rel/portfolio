#ifndef CPUMASK_OP_H
#define CPUMASK_OP_H

#include <stdint.h>
#include <stdatomic.h>
#include "scheduler/scheduler.h"


#define SCHED_BITMAP_BITS_PER_TYPE(type_name) (sizeof(type_name) << 3)


/**
 * Get the mask of zeros in the lower positions and the ones in the high-order
 * positions on the "nbits" boundary:
 * (0xFFFF, 3, 16) -> '0b1111111111111111000'
 */
#define SCHED_BITMAP_FIRST_WORD_MASK(mask, nbits, mask_len) ((mask) << ((nbits) & ((mask_len) -1)))


/**
 * Get the mask of ones in the lower positions and the zeros in the high-order
 * positions on the "nbits" boundary:
 * (0xFFFF, 3, 16) -> '0b111'
 */
#define SCHED_BITMAP_LAST_WORD_MASK(mask, nbits, mask_len) ((mask) >> (-(nbits) & ((mask_len) -1)))


#define SCHED_CPUMASK_BIT_ARRAY_ITEM(nr) ((nr) / SCHED_BITS_PER_SIZE_T)


#define SCHED_CPUMASK_BIT_MASK(nr)  (((sched_cpu_set_chunck)1) << ((nr) % SCHED_BITS_PER_SIZE_T))


static inline void
sched_cpu_clear_atomic(sched_cpu_num cpu, sched_atomic_cpu_set_t cpu_set)
{
    atomic_fetch_and_explicit(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu),
                              ~SCHED_CPUMASK_BIT_MASK(cpu),
                              memory_order_relaxed);
}


static inline void
sched_cpu_set_atomic(sched_cpu_num cpu, sched_atomic_cpu_set_t cpu_set)
{
    atomic_fetch_or_explicit(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu),
                             SCHED_CPUMASK_BIT_MASK(cpu),
                             memory_order_relaxed);
}


static inline size_t
sched_cpu_test_atomic(sched_cpu_num cpu, sched_atomic_cpu_set_t cpu_set)
{
    return SCHED_CPUMASK_BIT_MASK(cpu)
            & atomic_load_explicit(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu),
                                   memory_order_relaxed);
}


static inline size_t
sched_cpu_test(sched_cpu_num cpu, sched_cpu_set_t cpu_set)
{
    return SCHED_CPUMASK_BIT_MASK(cpu) & *(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu));
}


static inline void
sched_cpu_clear(sched_cpu_num cpu, sched_cpu_set_t cpu_set)
{
    *(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu)) &= ~SCHED_CPUMASK_BIT_MASK(cpu);
}


static inline void
sched_cpu_set(sched_cpu_num cpu, sched_cpu_set_t cpu_set)
{
    *(cpu_set + SCHED_CPUMASK_BIT_ARRAY_ITEM(cpu)) |= SCHED_CPUMASK_BIT_MASK(cpu);
}


static inline void
sched_task_fill_bitmap_positive(sched_cpu_num cpu_count, sched_atomic_cpu_set_t cpumask)
{
    sched_cpu_set_chunck_atomic *curr = cpumask;
    sched_cpu_set_chunck_atomic *end = curr + SCHED_CPUMASK_BYTES(cpu_count) - 1;
    while (curr != end)
        *(curr++) = ~((sched_cpu_set_chunck) 0);
    *end = SCHED_BITMAP_LAST_WORD_MASK(~((sched_cpu_set_chunck)0),
                                       cpu_count % SCHED_BITS_PER_SIZE_T,
                                       SCHED_BITMAP_BITS_PER_TYPE(sched_cpu_set_chunck));
}


#endif // CPUMASK_OP_H
