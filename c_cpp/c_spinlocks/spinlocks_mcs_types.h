#ifndef PLATFORM_SPINLOCKS_MCS_TYPES_H
#define PLATFORM_SPINLOCKS_MCS_TYPES_H

#include <stdint.h>
#include <stdatomic.h>
#include <stddef.h>

struct platform_spin_mcs;


typedef _Atomic(struct platform_spin_mcs *) platform_spin_mcs_next_t;


struct platform_spin_mcs {
    /** next waiting node */
    platform_spin_mcs_next_t next;

    /** 1 если блокировка получена */
    atomic_int_least32_t locked;

    /** счетчик вложений, см. qspinlock.c */
    atomic_int_least32_t count;
};

#define PLATFORM_SPIN_MCS_INITIALIZER { NULL, 0, 0 }


#endif // PLATFORM_SPINLOCKS_MCS_TYPES_H
