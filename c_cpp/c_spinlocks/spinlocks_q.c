/**
 * @file
 * @brief Queued spinlock implementation
 */

#include <stdatomic.h>
#include <assert.h>

#include "platform/architecture.h"
#include "platform/spinlocks_mcs.h"
#include "platform/spinlocks_q.h"
#include "scheduler/scheduler.h"
#include "platform/barrier.h"

/*
 * The basic principle of a queue-based spinlock can best be understood
 * by studying a classic queue-based spinlock implementation called the
 * MCS lock. The paper below provides a good description for this kind
 * of lock.
 *
 * http://www.cise.ufl.edu/tr/DOC/REP-1992-71.pdf
 *
 * This queued spinlock implementation is based on the MCS lock, however to make
 * it fit the 4 bytes we assume spinlock_t to be, and preserve its existing
 * API, we must modify it somehow.
 *
 * In particular; where the traditional MCS lock consists of a tail pointer
 * (8 bytes) and needs the next pointer (another 8 bytes) of its own node to
 * unlock the next pending (next->locked), we compress both these: {tail,
 * next->locked} into a single uint32_t value.
 *
 * Since a spinlock disables recursion of its own context and there is a limit
 * to the contexts that can nest; namely: task, softirq, hardirq, nmi. As there
 * are at most 4 nesting levels, it can be encoded by a 2-bit number. Now
 * we can encode the tail by combining the 2-bit nesting level with the cpu
 * number. With one byte for the lock value and 3 bytes for the tail, only a
 * 32-bit word is now needed. Even though we only need 1 bit for the lock,
 * we extend it to a full byte to achieve better performance for architectures
 * that support atomic byte write.
 *
 * We also change the first spinner to spin on the lock bit instead of its
 * node; whereby avoiding the need to carry a node from lock to unlock, and
 * preserving existing lock API. This also makes the unlock code simpler and
 * faster.
 *
 * N.B. The current implementation only supports architectures that allow
 *      atomic operations on smaller 8-bit and 16-bit data types.
 *
 */


/**
 * @brief encode_tail
 *   We must be able to distinguish between no-tail and the tail at 0:0, therefore increment the
 *   cpu number by one.
 * @param cpu
 * @param idx
 */
static inline uint32_t
qspinlock_encode_tail(int32_t cpu, int32_t idx)
{
    uint32_t tail;

    assert(idx < 4);
    tail  = (cpu + 1) << PLATFORM_SPIN_Q_TAIL_CPU_OFFSET;
    tail |= idx << PLATFORM_SPIN_Q_TAIL_IDX_OFFSET; /* assume < 4 */

    return tail;
}


#define PLATFORM_Q_LOCKED_PENDING_MASK (PLATFORM_SPIN_Q_LOCKED_MASK | PLATFORM_SPIN_Q_PENDING_MASK)


/**
 * @brief The __qspinlock struct
 *
 * By using the whole 2nd least significant byte for the pending bit, we can allow better
 * optimization of the lock acquisition for the pending bit holder.
 *
 * This internal structure is also used by the set_locked function which is not restricted to
 * PLATFORM_Q_PENDING_BITS == 8.
 */
struct qspinlock_internal {
    union {
        atomic_uint_least32_t val;
#ifdef PLATFORM_ORDER_LITTLE_ENDIAN
        struct
        {
            atomic_uint_least8_t locked;
            uint8_t              pending;
        };
        struct
        {
            atomic_uint_least16_t locked_pending;
            atomic_uint_least16_t tail;
        };
#else
        struct
        {
            atomic_uint_least16_t tail;
            atomic_uint_least16_t locked_pending;
        };
        struct
        {
            uint8_t    reserved[2];
            uint8_t    pending;
            atomic_uint_least8_t locked;
        };
#endif
    };
};

static_assert(sizeof(struct qspinlock_internal) == sizeof(uint32_t),
              "bad qspinlock_internal align");


#if PLATFORM_SPIN_Q_PENDING_BITS == 8

/**
 * @brief clear_pending_set_locked
 *     Take ownership and clear the pending bit.
 *     Lock stealing is not allowed if this function is used.
 *     *,1,0 -> *,0,1
 * @param lock
 *     Pointer to queued spinlock structure.
 */
static inline void
qspinlock_clear_pending_set_locked(struct platform_spin_queued *lock)
{
    struct qspinlock_internal *l = (void *)lock;

    atomic_store_explicit(&l->locked_pending, PLATFORM_SPIN_Q_LOCKED_VAL, memory_order_relaxed);
}


/**
 * @brief xchg_tail
 *     Put in the new queue tail code word & retrieve previous one
 *     p,*,* -> n,*,* ; prev = xchg(lock, node)
 * @param lock
 *     Pointer to queued spinlock structure
 * @param tail
 *     The new queue tail code word
 * @return
 *     The previous queue tail code word
 */
static inline
uint32_t qspinlock_xchg_tail(struct platform_spin_queued *lock, uint32_t tail)
{
    struct qspinlock_internal *l = (void *)lock;

    /*
     * Use release semantics to make sure that the MCS node is properly
     * initialized before changing the tail code.
     */
    return (uint32_t)atomic_exchange_explicit(&l->tail, tail >> PLATFORM_SPIN_Q_TAIL_OFFSET,
                                              memory_order_release) << PLATFORM_SPIN_Q_TAIL_OFFSET;
}

#else /* PLATFORM_Q_PENDING_BITS == 8 */


/**
 * @brief clear_pending_set_locked
 *     Take ownership and clear the pending bit.
 *     * ,1,0 -> *,0,1
 * @param lock
 *     Pointer to queued spinlock structure
 */
static inline void
qspinlock_clear_pending_set_locked(struct raw_qspinlock *lock)
{
    atomic_fetch_add_explicit(&lock->val,
                              -PLATFORM_Q_PENDING_VAL + PLATFORM_Q_LOCKED_VAL,
                              memory_order_relaxed);
}


/**
 * @brief xchg_tail
 *     Put in the new queue tail code word & retrieve previous one
 * @param lock
 *     Pointer to queued spinlock structure
 * @param tail
 *     The new queue tail code word
 * @return
 *     The previous queue tail code word
 *     p,*,* -> n,*,* ; prev = xchg(lock, node)
 */
static inline uint32_t
qspinlock_xchg_tail(struct raw_qspinlock *lock, uint32_t tail)
{
    uint32_t val = atomic_load_explicit(&lock->val, memory_order_relaxed);
    uint32_t new_val = (val & PLATFORM_Q_LOCKED_PENDING_MASK) | tail;
    do
    {
        /*
         * Use release semantics to make sure that the MCS node is properly initialized before
         * changing the tail code.
         */
    } while(!(atomic_compare_exchange_weak_explicit(&lock->val, &val, new_val,
                                                    memory_order_release,
                                                    memory_order_relaxed)))
    return val;
}
#endif /* PLATFORM_Q_PENDING_BITS == 8 */



/*
 * Various notes on spin_is_locked() and spin_unlock_wait(), which are
 * 'interesting' functions:
 *
 * PROBLEM: some architectures have an interesting issue with atomic ACQUIRE
 * operations in that the ACQUIRE applies to the LOAD _not_ the STORE (ARM64,
 * PPC). Also qspinlock has a similar issue per construction, the setting of
 * the locked byte can be unordered acquiring the lock proper.
 *
 * This gets to be 'interesting' in the following cases, where the /should/s
 * end up false because of this issue.
 *
 *
 * CASE 1:
 *
 * So the spin_is_locked() correctness issue comes from something like:
 *
 *   CPU0                CPU1
 *
 *   global_lock();            local_lock(i)
 *     spin_lock(&G)              spin_lock(&L[i])
 *     for (i)                  if (!spin_is_locked(&G)) {
 *       spin_unlock_wait(&L[i]);        smp_acquire__after_ctrl_dep();
 *                        return;
 *                      }
 *                      // deal with fail
 *
 * Where it is important CPU1 sees G locked or CPU0 sees L[i] locked such
 * that there is exclusion between the two critical sections.
 *
 * The load from spin_is_locked(&G) /should/ be constrained by the ACQUIRE from
 * spin_lock(&L[i]), and similarly the load(s) from spin_unlock_wait(&L[i])
 * /should/ be constrained by the ACQUIRE from spin_lock(&G).
 *
 * Similarly, later stuff is constrained by the ACQUIRE from CTRL+RMB.
 *
 *
 * CASE 2:
 *
 * For spin_unlock_wait() there is a second correctness issue, namely:
 *
 *   CPU0                CPU1
 *
 *   flag = set;
 *   smp_mb();                spin_lock(&l)
 *   spin_unlock_wait(&l);        if (!flag)
 *                      // add to lockless list
 *                    spin_unlock(&l);
 *   // iterate lockless list
 *
 * Which wants to ensure that CPU1 will stop adding bits to the list and CPU0
 * will observe the last entry on the list (if spin_unlock_wait() had ACQUIRE
 * semantics etc..)
 *
 * Where flag /should/ be ordered against the locked store of l.
 */


/**
 * @brief sched_raw_queued_spin_unlock_wait
 * can (load-)ACQUIRE the lock before issuing an _unordered_ store to set PLATFORM_Q_LOCKED_VAL.
 *
 * This means that the store can be delayed, but no later than the store-release from the unlock.
 * This means that simply observing PLATFORM_Q_LOCKED_VAL is not sufficient to determine if the
 * lock is acquired.
 *
 * There are two paths that can issue the unordered store:
 *
 *  (1) clear_pending_set_locked():    *,1,0 -> *,0,1
 *
 *  (2) set_locked():            t,0,0 -> t,0,1 ; t != 0
 *      atomic_cmpxchg_relaxed():    t,0,0 -> 0,0,1
 *
 * However, in both cases we have other !0 state we've set before to queue ourseves:
 *
 * For (1) we have the atomic_cmpxchg_acquire() that set PLATFORM_Q_PENDING_VAL, our load is
 * constrained by that ACQUIRE to not pass before that, and thus must observe the store.
 *
 * For (2) we have a more intersting scenario. We enqueue ourselves using xchg_tail(), which ends up
 * being a RELEASE. This in itself is not sufficient, however that is followed by an
 * smp_cond_acquire() on the same word, giving a RELEASE->ACQUIRE ordering. This again constrains
 * our load and guarantees we must observe that store.
 *
 * Therefore both cases have other !0 state that is observable before the unordered locked byte
 * store comes through. This means we can use that to wait for the lock store, and then wait for
 * an unlock.
 *
 * @param lock
 */
void
platform_spin_queued_unlock_wait(struct platform_spin_queued *lock)
{
    uint32_t val;

    while (true)
    {
        val = atomic_load_explicit(&lock->val, memory_order_relaxed);

        if (!val) /* not locked, we're done */
            goto done;

        if (val & PLATFORM_SPIN_Q_LOCKED_MASK) /* locked, go wait for unlock */
            break;

        /* not locked, but pending, wait until we observe the lock */
        platform_cpu_relax();
    }

    /* any unlock is good */
    while (atomic_load_explicit(&lock->val, memory_order_relaxed) & PLATFORM_SPIN_Q_LOCKED_MASK)
        platform_cpu_relax();

done:
    platform_memory_barrier_read();
}


/**
 * @brief queued_spin_lock_slowpath
 *     Acquire the queued spinlock
 *
 * (queue tail, pending bit, lock value)
 *
 *              fast     :    slow                                  :    unlock
 *                       :                                          :
 * uncontended  (0,0,0) -:--> (0,0,1) ------------------------------:--> (*,*,0)
 *                       :       | ^--------.------.             /  :
 *                       :       v           \      \            |  :
 * pending               :    (0,1,1) +--> (0,1,0)   \           |  :
 *                       :       | ^--'              |           |  :
 *                       :       v                   |           |  :
 * uncontended           :    (n,x,y) +--> (n,0,0) --'           |  :
 *   queue               :       | ^--'                          |  :
 *                       :       v                               |  :
 * contended             :    (*,x,y) +--> (*,0,0) ---> (*,0,1) -'  :
 *   queue               :         ^--'                             :
 *
 * @param lock
 *     Pointer to queued spinlock structure
 * @param val
 *     Current value of the queued spinlock 32-bit word
 */
void
platform_spin_queued_lock_slowpath(struct platform_spin_queued *lock,
                                    struct sched_rq *rq, uint32_t old_val)
{
    struct platform_spin_mcs *prev, *next, *node;
    uint32_t new_val, old, tail;
    int idx;

    static_assert(CONFIG_SCHED_NR_CPUS < (1U << PLATFORM_SPIN_Q_TAIL_CPU_BITS), "cpus count owerflow");

    /* wait for in-progress pending->locked hand-overs 0,1,0 -> 0,0,1 */
    if (old_val == PLATFORM_SPIN_Q_PENDING_VAL)
        while ((old_val = atomic_load_explicit(&lock->val, memory_order_relaxed))
                == PLATFORM_SPIN_Q_PENDING_VAL)
            platform_cpu_relax();


    /*
     * trylock || pending
     *
     * 0,0,0 -> 0,0,1 ; trylock
     * 0,0,1 -> 0,1,1 ; pending
     */
    do
    {
        /* If we observe any contention; queue. */
        if (old_val & ~PLATFORM_SPIN_Q_LOCKED_MASK)
            goto queue;

        new_val = PLATFORM_SPIN_Q_LOCKED_VAL;
        if (old_val == new_val)
            new_val |= PLATFORM_SPIN_Q_PENDING_VAL;

     /* Acquire semantic is required here as the function may return immediately if the lock was
      * free. */
    } while (!atomic_compare_exchange_weak_explicit(&lock->val, &old_val, new_val,
                                                    memory_order_acquire,
                                                    memory_order_relaxed));

    /* we won the trylock */
    if (new_val == PLATFORM_SPIN_Q_LOCKED_VAL)
        return;

    /*
     * we're pending, wait for the owner to go away.
     *
     * *,1,1 -> *,1,0
     *
     * this wait loop must be a load-acquire such that we match the
     * store-release that clears the locked bit and create lock
     * sequentiality; this is because not all clear_pending_set_locked()
     * implementations imply full barriers.
     */
    platform_smp_cond_load_acquire(&lock->val, !(VAL & PLATFORM_SPIN_Q_LOCKED_MASK));

    /*
     * take ownership and clear the pending bit.
     *
     * *,1,0 -> *,0,1
     */
    qspinlock_clear_pending_set_locked(lock);
    return;

    /* End of pending bit optimistic spinning and beginning of MCS queuing. */
queue:
    node = &rq->mcs_lock_node;
//    idx = atomic_load_explicit(&(node->count), memory_order_relaxed);
//    atomic_fetch_add_explicit(&(node->count), 1, memory_order_relaxed);
    assert(node->count == 0);
    idx = 0;
    tail = qspinlock_encode_tail(rq->cpu_num, idx);

//    node += idx;
    node->locked = 0;
    node->next = NULL;

    /*
     * We touched a (possibly) cold cacheline in the per-cpu queue node; attempt the trylock once
     * more in the hope someone let go while we weren't watching.
     */
    if (platform_spin_queued_trylock(lock))
        goto release;

    /*
     * We have already touched the queueing cacheline; don't bother with pending stuff.
     * p,*,* -> n,*,*
     * RELEASE, such that the stores to @node must be complete.
     */
    old = qspinlock_xchg_tail(lock, tail);
    next = NULL;

    /* if there was a previous node; link it and wait until reaching the head of the waitqueue. */
    if (old & PLATFORM_SPIN_Q_TAIL_MASK)
    {
        {
            struct scheduler* sched = rq->shared;

            uint32_t cpu = (old >> PLATFORM_SPIN_Q_TAIL_CPU_OFFSET) - 1;
            // int32_t idx = (old &  PLATFORM_Q_TAIL_IDX_MASK) >> PLATFORM_Q_TAIL_IDX_OFFSET;
            assert((old &  PLATFORM_SPIN_Q_TAIL_IDX_MASK) >> PLATFORM_SPIN_Q_TAIL_IDX_OFFSET == 0);
            assert(sched->cpu_count > cpu);

            // prev = per_cpu_ptr(&mcs_nodes[idx], cpu);
            prev = &sched->runqueues[cpu].mcs_lock_node;
        }

        /*
         * The above xchg_tail() is also a load of @lock which generates, through decode_tail(),
         * a pointer.
         * The address dependency matches the RELEASE of xchg_tail() such that the access to @prev
         * must happen after.
         */
        platform_memory_barrier_read_depends();

        atomic_store_explicit(&prev->next, node, memory_order_relaxed);

        platform_mcs_spin_lock_contended(&node->locked);

        /*
         * While waiting for the MCS lock, the next pointer may have
         * been set by another lock waiter. We optimistically load
         * the next pointer & prefetch the cacheline for writing
         * to reduce latency in the upcoming MCS unlock operation.
         */
        next = atomic_load_explicit(&node->next, memory_order_relaxed);
        if (next)
            __builtin_prefetch(next, 1, 3);
    }

    /*
     * we're at the head of the waitqueue, wait for the owner & pending to
     * go away.
     *
     * *,x,y -> *,0,0
     *
     * this wait loop must use a load-acquire such that we match the
     * store-release that clears the locked bit and create lock
     * sequentiality; this is because the set_locked() function below
     * does not imply a full barrier.
     *
     */

    old_val = platform_smp_cond_load_acquire(&lock->val, !(VAL & PLATFORM_Q_LOCKED_PENDING_MASK));

    /*
     * claim the lock:
     *
     * n,0,0 -> 0,0,1 : lock, uncontended
     * *,0,0 -> *,0,1 : lock, contended
     *
     * If the queue head is the only one in the queue (lock value == tail),
     * clear the tail code and grab the lock. Otherwise, we only need
     * to grab the lock.
     */
    while (true)
    {
        /* In the PV case we might already have PLATFORM_Q_LOCKED_VAL set */
        if ((old_val & PLATFORM_SPIN_Q_TAIL_MASK) != tail)
        {
            /* Set the lock bit and own the lock     *,*,0 -> *,0,1 */
            struct qspinlock_internal *l = (void *)lock;
            atomic_store_explicit(&l->locked, PLATFORM_SPIN_Q_LOCKED_VAL, memory_order_relaxed);
            break;
        }

        /*
         * The smp_cond_load_acquire() call above has provided the
         * necessary acquire semantics required for locking. At most
         * two iterations of this loop may be ran.
         */
        if (atomic_compare_exchange_weak_explicit(&lock->val, &old_val, PLATFORM_SPIN_Q_LOCKED_VAL,
                                                  memory_order_relaxed, memory_order_relaxed))
            goto release;    /* No contention */
    };

    /* contended path; wait for next if not observed yet, release. */
    if (!next)
        while (!(next = atomic_load_explicit(&node->next, memory_order_relaxed)))
            platform_cpu_relax();

    platform_mcs_spin_unlock_contended(&next->locked);

release:
    /* release the node */
    return;
//    atomic_fetch_sub_explicit(&rq->mcs_lock_node.count, 1, memory_order_relaxed);
}
