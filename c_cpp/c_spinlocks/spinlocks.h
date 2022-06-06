#ifndef SPINLOCKS_H
#define SPINLOCKS_H

#include "config_sched.h"
#include "platform/spinlocks_types.h"


#ifdef CONFIG_SCHED_SPINLOCK_QUEUED

#include "platform/spinlocks_q.h"

/**
 * @brief platform_spin_lock_init
 *     Initialize spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 */
#define platform_spin_lock_init(spinlock)       platform_spin_queued_init(spinlock)


/**
 * @brief platform_spin_lock
 *     Wait and lock the spin.
 * @param spinlock
 *     Spin for locking.
 * @param cpu_num
 *     CPU, that locks spin
 * @return
 *     true, if the spin is locked, false if the spin pointer is NULL.
 */
#define platform_spin_lock(spinlock, cpu)       platform_spin_queued_lock(spinlock, cpu)


/**
 * @brief platform_spin_trylock
 *     Try to lock the spin. Debugging version.
 * @param spinlock
 *     Spin for blocking.
 * @param cpu_num
 *     CPU, that locks spin.
 * @return
 *     true, if the spin is locked, false - if not.
 */
#define platform_spin_trylock(spinlock, cpu)   platform_spin_queued_trylock(spinlock)


/**
 * @brief platform_spin_unlock
 *     Unlock spin.
 * @param spinlock
 *     Spin to unlock.
 */
#define platform_spin_unlock(spinlock)         platform_spin_queued_unlock(spinlock)



#else

#include "platform/spinlocks_ticket.h"

/**
 * @brief platform_spin_lock_init
 *     Initialize spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 */
#define platform_spin_lock_init(spinlock)           platform_spin_ticket_init(spinlock)


/**
 * @brief platform_spin_lock
 *     Wait and lock the spin.
 * @param spinlock
 *     Spin for locking.
 * @param cpu_num
 *     CPU, that locks spin
 * @return
 *     true, if the spin is locked, false if the spin pointer is NULL.
 */
#define platform_spin_lock(spinlock, cpu)       platform_spin_ticket_lock(spinlock, cpu)


/**
 * @brief platform_spin_trylock
 *     Try to lock the spin. Debugging version.
 * @param spinlock
 *     Spin for blocking.
 * @param cpu_num
 *     CPU, that locks spin.
 * @return
 *     true, if the spin is locked, false - if not.
 */
#define platform_spin_trylock(spinlock, cpu) platform_spin_ticket_do_trylock(spinlock, cpu)


/**
 * @brief platform_spin_unlock
 *     Unlock spin.
 * @param spinlock
 *     Spin to unlock.
 */
#define platform_spin_unlock(spinlock)              platform_spin_ticket_unlock(spinlock)


#endif // CONFIG_SCHED_SPINLOCK_QUEUED


#endif // SPINLOCKS_H
