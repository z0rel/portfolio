/**
 * @file
 * @brief Types of the spinlock syncronization primitives.
 */
#ifndef SPINLOCK_TICKET_TYPES_H
#define SPINLOCK_TICKET_TYPES_H


#include <stdatomic.h>

#include "config_sched.h"

#ifndef CONFIG_SCHED_DEBUG_SPINLOCK


/** Spinlocks for scheduler */
typedef struct platform_spin_ticket
{
    /** spinlock value */
    volatile atomic_int_least32_t spin;
} platform_spin_ticket_t;


#define PLATFORM_SPIN_TICKET_INITIALIZER { 0 }


#else


#include <pthread.h>
#include <assert.h>


/** Spinlocks for debugging of the scheduler */
typedef struct platform_spin_ticket
{
    /** spinlock value */
    volatile atomic_int spin;

    uint32_t pad; // supress padding warning

    /** debug spinlock mutex */
    pthread_mutex_t pth_spinlock;

    /** debug spinlock id */
    atomic_int id;

    /** spinlock locking line */
    int line;
} platform_spin_ticket_t;

#define PLATFORM_SPIN_TICKET_UNLOCKED_ID -1

#define PLATFORM_SPIN_TICKET_INITIALIZER \
    { 0, 0, PTHREAD_MUTEX_INITIALIZER, PLATFORM_SPIN_TICKET_UNLOCKED_ID, 0 }


#endif


#endif // SPINLOCK_TICKET_TYPES_H
