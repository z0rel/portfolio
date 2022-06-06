#ifndef PLATFORM_SPINLOCKS_Q_H
#define PLATFORM_SPINLOCKS_Q_H

#include <stdatomic.h>
#include <stdint.h>
#include <stdbool.h>

#include "likely.h"
#include "platform/spinlocks_q_types.h"
#include "scheduler/sched_types.h"


/**
 * @brief platform_spin_queued_init
 *     Initialize queued spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 */
static inline void
platform_spin_queued_init(platform_spin_queued_t *spinlock)
{
    if (unlikely(spinlock == NULL))
        return;

    atomic_store_explicit(&spinlock->val, 0, memory_order_release);
}


/**
 * @brief sched_raw_queued_spin_unlock_wait
 *     Wait until the _current_ lock holder releases the lock.
 *
 * There is a very slight possibility of live-lock if the lockers keep coming
 * and the waiter is just unfortunate enough to not see any unlock state.
 *
 * @param lock
 *     Pointer to queued spinlock structure
 */
void
platform_spin_queued_unlock_wait(struct platform_spin_queued *lock);


/**
 * @brief queued_spin_is_locked
 *     Is the spinlock locked?
 * @param lock
 *     Pointer to queued spinlock structure
 * @return
 *     1 if it is locked, 0 otherwise
 */
static inline int
platform_spin_queued_is_locked(struct platform_spin_queued *lock)
{
    /*
     * See sched_raw_queued_spin_unlock_wait().
     *
     * Any !0 state indicates it is locked, even if PLATFORM_Q_LOCKED_VAL isn't immediately
     * observable.
     */
    return atomic_load_explicit(&lock->val, memory_order_relaxed);
}


/**
 * @brief sched_raw_queued_spin_value_unlocked
 *     Is the spinlock structure unlocked?
 *
 * N.B. Whenever there are tasks waiting for the lock, it is considered locked wrt the lockref code
 * to avoid lock stealing by the lockref code and change things underneath the lock. This also
 * allows some optimizations to be applied without conflict with lockref.
 *
 * @param lock
 *     queued spinlock structure
 *
 * @return
 *     1 if it is unlocked, 0 otherwise
 */
static inline int
platform_spin_queued_value_unlocked(struct platform_spin_queued lock)
{
    return !atomic_load_explicit(&lock.val, memory_order_relaxed);
}


/**
 * @brief sched_raw_queued_spin_is_contended
 *     Check if the lock is contended
 * @param lock
 *     Pointer to queued spinlock structure
 * @return
 *     1 if lock contended, 0 otherwise
 */
static inline int
platform_spin_queued_is_contended(struct platform_spin_queued *lock)
{
    return atomic_load_explicit(&lock->val, memory_order_relaxed) & ~PLATFORM_SPIN_Q_LOCKED_MASK;
}


/**
 * @brief queued_spin_trylock
 *     Try to acquire the queued spinlock.
 * @param lock
 *     Pointer to queued spinlock structure.
 * @return
 *     true if lock acquired, false if failed
 */
static inline bool
platform_spin_queued_trylock(struct platform_spin_queued *lock)
{
    uint32_t p = 0;
    if (!atomic_load_explicit(&lock->val, memory_order_relaxed) &&
        (atomic_compare_exchange_weak_explicit(&lock->val, &p, PLATFORM_SPIN_Q_LOCKED_VAL,
                                               memory_order_acquire,
                                               memory_order_relaxed)))
        return true;
    return false;
}


void
platform_spin_queued_lock_slowpath(struct platform_spin_queued *lock, struct sched_rq *rq,
                                   uint32_t val);


/**
 * @brief sched_raw_queued_spin_lock
 *     Acquire a queued spinlock
 * @param lock
 *     Pointer to queued spinlock structure
 */
static inline void
platform_spin_queued_lock(struct platform_spin_queued *lock, struct sched_rq *rq)
{
    uint32_t val = 0;

    if (likely(atomic_compare_exchange_weak_explicit(&lock->val, &val, PLATFORM_SPIN_Q_LOCKED_VAL,
                                                     memory_order_acquire,
                                                     memory_order_relaxed)))
        return;

    platform_spin_queued_lock_slowpath(lock, rq, val);
}


/**
 * @brief sched_raw_queued_spin_unlock
 *     Release a queued spinlock.
 * @param lock
 *     Pointer to queued spinlock structure.
 */
static inline void
platform_spin_queued_unlock(struct platform_spin_queued *lock)
{
    /* unlock() needs release semantics: */
    (void)atomic_fetch_sub_explicit(&lock->val, PLATFORM_SPIN_Q_LOCKED_VAL, memory_order_release);
}


#endif // PLATFORM_SPINLOCKS_Q_H
