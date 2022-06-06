#ifndef SPINLOCKS_TYPES_H
#define SPINLOCKS_TYPES_H

#include "config_sched.h"

#ifdef CONFIG_SCHED_SPINLOCK_QUEUED


#include "platform/spinlocks_q_types.h"

typedef platform_spin_queued_t platform_spinlock_t;

#define PLATFORM_SPINLOCK_INITIALIZER PLATFORM_SPIN_QSPINLOCK_INITIALIZER


#else


#include "platform/spinlocks_ticket_types.h"

typedef platform_spin_ticket_t platform_spinlock_t;

#define PLATFORM_SPINLOCK_INITIALIZER PLATFORM_SPIN_TICKET_INITIALIZER


#endif // CONFIG_SCHED_SPINLOCK_QUEUED


#endif // SPINLOCKS_TYPES_H
