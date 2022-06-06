/**
 * @file
 * @brief Spinlock syncronization primitives.
 */
#ifndef SPINLOCKS_TICKET_H
#define SPINLOCKS_TICKET_H


#include "spinlocks_ticket_types.h"
#include "likely.h"

#ifndef CONFIG_SCHED_DEBUG_SPINLOCK


/**
 * @brief platform_spin_lock_init_impl
 *     Initialize spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 */
static inline void
platform_spin_ticket_init(platform_spin_ticket_t *spinlock)
{
    if (unlikely(spinlock == NULL))
        return;

    atomic_store_explicit(&spinlock->spin, 0, memory_order_release);
}


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
#define platform_spin_ticket_lock(spinlock, cpu_num) \
    platform_spin_ticket_lock_nodbg(spinlock)


/**
 * @brief platform_spin_lock_impl
 *     Grab the spin
 * @param spinlock
 *     Spin for blocking
 * @return
 *     true if the spin is locked, false if an error occurred.
 */
static inline bool
platform_spin_ticket_lock_nodbg(platform_spin_ticket_t *spinlock)
{
    int_least32_t tmp_int;

    if (unlikely(spinlock == NULL))
        return false;

    do
    {
        tmp_int = 0;
    } while (atomic_compare_exchange_weak_explicit(&spinlock->spin,
                                                   &tmp_int,
                                                   1,
                                                   memory_order_acquire,
                                                   memory_order_relaxed) == false);

    return true;
}


/**
 * @brief platform_spin_ticket_do_trylock
 *     Try to lock the spin. Debugging version.
 * @param spinlock
 *     Spin for blocking.
 * @param cpu_num
 *     CPU, that locks spin.
 * @return
 *     true, if the spin is locked, false - if not.
 */
#define platform_spin_ticket_do_trylock(spinlock, cpu_num) \
    platform_spin_ticket_do_trylock_nodbg(spinlock)


/**
 * @brief platform_spin_trylock_impl
 *     Try to lock the spin.
 * @param spinlock
 *     Spin for blocking.
 * @return
 *     true, if the spin is locked, false - if not.
 */
static inline bool
platform_spin_ticket_do_trylock_nodbg(platform_spin_ticket_t *spinlock)
{
    int_least32_t tmp_int = 0;

    if (unlikely(spinlock == NULL))
        return false;

    return atomic_compare_exchange_weak_explicit(&spinlock->spin, &tmp_int, 1,
                                                 memory_order_acquire, memory_order_relaxed);
}


/**
 * @brief platform_spin_unlock_impl
 *     Unlock spin.
 * @param spinlock
 *     Spin to unlock.
 */
static inline void
platform_spin_ticket_unlock(platform_spin_ticket_t *spinlock)
{
    if (unlikely(spinlock == NULL))
        return;

    atomic_store_explicit(&spinlock->spin, 0, memory_order_release);
}

#else // defined CONFIG_SCHED_DEBUG_SPINLOCK - For debug of the sync primitives in the valgrind.


/**
 * @brief platform_spin_lock_init
 *     Initialize spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 */
#define platform_spin_ticket_init(spinlock) \
    platform_spin_ticket_init_dbg((spinlock), __LINE__)


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
#define platform_spin_ticket_lock(spinlock, cpu_num) \
    platform_spin_ticket_lock_dbg((spinlock), (cpu_num), __LINE__)


/**
 * @brief platform_spin_ticket_do_trylock
 *     Try to lock the spin. Debugging version.
 * @param spinlock
 *     Spin for blocking.
 * @param cpu_num
 *     CPU, that locks spin.
 * @return
 *     true, if the spin is locked, false - if not.
 */
#define platform_spin_ticket_do_trylock(spinlock, cpu_num) \
    platform_spin_ticket_do_trylock_dbg((spinlock), (cpu_num), __LINE__)


/**
 * @brief platform_spin_ticket_unlock
 *     Unlock spin.
 * @param spinlock
 *     Spin to unlock.
 */
#define platform_spin_ticket_unlock(spinlock) \
    platform_spin_ticket_unlock_dbg((spinlock), __LINE__)


/**
 * @brief platform_spin_ticket_init_dbg
 *     Initialize spinlock.
 * @param spinlock
 *     Spinlok for initialization.
 * @param line
 *     The line in which the initialization occurs.
 */
static inline void
platform_spin_ticket_init_dbg(platform_spin_ticket_t *spinlock, int line)
{
    if (unlikely(spinlock == NULL))
        return;

    pthread_mutex_init(&spinlock->pth_spinlock, NULL);

    atomic_store_explicit(&spinlock->spin, 0, memory_order_release);
    spinlock->id = PLATFORM_SPIN_TICKET_UNLOCKED_ID;
    spinlock->line = line;
}


/**
 * @brief platform_spin_ticket_lock_dbg
 *     Wait and lock the spin.
 * @param spinlock
 *     Spin for locking.
 * @param lock_id
 *     Lock identifier.
 * @param line
 *     Line in which the spin is locked.
 * @return
 *     true, if the spin is locked, false if an error occurred.
 */
static inline bool
platform_spin_ticket_lock_dbg(platform_spin_ticket_t *spinlock, int lock_id, int line)
{
    int id;
    if (unlikely(spinlock == NULL))
        return false;

    pthread_mutex_lock(&spinlock->pth_spinlock);
    id = atomic_load_explicit(&spinlock->id, memory_order_relaxed);
    assert(id == PLATFORM_SPIN_TICKET_UNLOCKED_ID);
    atomic_store_explicit(&spinlock->id, lock_id, memory_order_release);
    spinlock->line = line;
    spinlock->spin = 1;
    return true;
}


/**
 * @brief platform_spin_ticket_do_trylock_dbg
 *     Try to lock the spin. Debugging version.
 * @param spinlock
 *     Spin for blocking.
 * @param lock_id
 *     Lock identifier.
 * @param line
 *     The line from which the mutex is locked.
 * @return
 *     true, if the spin is locked, false - if not.
 */
static inline bool
platform_spin_ticket_do_trylock_dbg(platform_spin_ticket_t *spinlock, int lock_id, int line)
{
    if (unlikely(spinlock == NULL))
        return false;

    if (!pthread_mutex_trylock(&spinlock->pth_spinlock))
    {
        int id = atomic_load_explicit(&spinlock->id, memory_order_relaxed);
        assert(id == PLATFORM_SPIN_TICKET_UNLOCKED_ID);
        atomic_store_explicit(&spinlock->id, lock_id, memory_order_release);
        spinlock->line = line;
        spinlock->spin = 1;
        return true;
    }
    return false;
}


/**
 * @brief platform_spin_ticket_unlock_dbg
 *     Unlock spin. Debug version.
 * @param spinlock
 *     Spin to unlock.
 * @param line
 *     The line in which the mutex is unblocked
 */
static inline void
platform_spin_ticket_unlock_dbg(platform_spin_ticket_t *spinlock, int line)
{
    if (unlikely(spinlock == NULL))
        return;

    atomic_store_explicit(&spinlock->spin, 0, memory_order_release);
    atomic_store_explicit(&spinlock->id, PLATFORM_SPIN_TICKET_UNLOCKED_ID, memory_order_release);
    spinlock->line = line;

    pthread_mutex_unlock(&spinlock->pth_spinlock);
}


#endif


#endif // SPINLOCKS_TICKET_H
