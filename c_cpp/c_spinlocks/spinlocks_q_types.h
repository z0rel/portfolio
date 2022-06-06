/**
 * @file
 * @brief Queued spinlock types
 */
#ifndef PLATFORM_SPINLOCKS_Q_TYPES_H
#define PLATFORM_SPINLOCKS_Q_TYPES_H

#include <stdatomic.h>

#include "config_sched.h"

/**
 * @brief The raw_qspinlock struct
 *     The queued spinlock struct, based on the msc_spinlocks
 *
 * Bitfields in the atomic value:
 *
 * When CONFIG_SCHED_NR_CPUS < 16K
 *  0- 7: locked byte
 *     8: pending
 *  9-15: not used
 * 16-17: tail index
 * 18-31: tail cpu (+1)
 *
 * When NR_CPUS >= 16K
 *  0- 7: locked byte
 *     8: pending
 *  9-10: tail index
 * 11-31: tail cpu (+1)
 */
typedef struct platform_spin_queued
{
   atomic_uint_least32_t val;
} platform_spin_queued_t;

#define PLATFORM_SPIN_QSPINLOCK_INITIALIZER { 0 }

#define PLATFORM_SPIN_Q_SET_MASK(type)   (((1U << PLATFORM_SPIN_Q_ ## type ## _BITS) - 1)          \
                                          << PLATFORM_SPIN_Q_ ## type ## _OFFSET)
#define PLATFORM_SPIN_Q_LOCKED_OFFSET    0
#define PLATFORM_SPIN_Q_LOCKED_BITS      8
#define PLATFORM_SPIN_Q_LOCKED_MASK      PLATFORM_SPIN_Q_SET_MASK(LOCKED)

#define PLATFORM_SPIN_Q_PENDING_OFFSET   (PLATFORM_SPIN_Q_LOCKED_OFFSET +                          \
                                          PLATFORM_SPIN_Q_LOCKED_BITS)
#if CONFIG_SCHED_NR_CPUS < (1U << 14)
#   define PLATFORM_SPIN_Q_PENDING_BITS  8
#else
#   define PLATFORM_SPIN_Q_PENDING_BITS  1
#endif
#define PLATFORM_SPIN_Q_PENDING_MASK     PLATFORM_SPIN_Q_SET_MASK(PENDING)

#define PLATFORM_SPIN_Q_TAIL_IDX_OFFSET  (PLATFORM_SPIN_Q_PENDING_OFFSET +                         \
                                          PLATFORM_SPIN_Q_PENDING_BITS)
#define PLATFORM_SPIN_Q_TAIL_IDX_BITS    2
#define PLATFORM_SPIN_Q_TAIL_IDX_MASK    PLATFORM_SPIN_Q_SET_MASK(TAIL_IDX)

#define PLATFORM_SPIN_Q_TAIL_CPU_OFFSET  (PLATFORM_SPIN_Q_TAIL_IDX_OFFSET +                        \
                                          PLATFORM_SPIN_Q_TAIL_IDX_BITS)
#define PLATFORM_SPIN_Q_TAIL_CPU_BITS    (32 - PLATFORM_SPIN_Q_TAIL_CPU_OFFSET)
#define PLATFORM_SPIN_Q_TAIL_CPU_MASK    PLATFORM_SPIN_Q_SET_MASK(TAIL_CPU)

#define PLATFORM_SPIN_Q_TAIL_OFFSET      PLATFORM_SPIN_Q_TAIL_IDX_OFFSET
#define PLATFORM_SPIN_Q_TAIL_MASK        (PLATFORM_SPIN_Q_TAIL_IDX_MASK |                          \
                                          PLATFORM_SPIN_Q_TAIL_CPU_MASK)

#define PLATFORM_SPIN_Q_LOCKED_VAL       (1U << PLATFORM_SPIN_Q_LOCKED_OFFSET)
#define PLATFORM_SPIN_Q_PENDING_VAL      (1U << PLATFORM_SPIN_Q_PENDING_OFFSET)


#endif // PLATFORM_QSPINLOCKS_TYPES_H
