/**
 * @file
 * @brief scheduler mutexes: base types of the blocking mutual exclusion locks with PI support
 */

#ifndef SCHED_MUTEX_TYPES_H
#define SCHED_MUTEX_TYPES_H

#include <stdint.h>
#include <stdatomic.h>

#include "padding_warns.h"
#include "config_sched.h"
#include "scheduler/sched_rbtree_types.h"
#include "scheduler/sched_types.h"
#include "platform/spinlocks_types.h"


// TODO: вынести в errno h

/** Interrupted system call */
#define EINTR             4

/** Resource deadlock would occur */
#define EDEADLK          45

/** File locking deadlock error */
#define EDEADLOCK        56

/** Connection timed out */
#define ETIMEDOUT       145

/** Mutex was be locked */
#define ESCHED_MUTEX_LOCKED  -1


/** Constants for rt mutex functions which have a selectable deadlock detection. */
enum sched_mutex_chainwalk
{
    /** Stops the lock chain walk when there are no further PI adjustments to be made. */
    SCHED_MUTEX_MIN_CHAINWALK,
    /** Invoke deadlock detection with a full walk of the lock chain. */
    SCHED_MUTEX_FULL_CHAINWALK,
};


struct sched_mutex;
struct sched_mutex_wake_q_head;
struct sched_mutex_wake_q_node;
struct sched_mutex_waiter;


typedef _Atomic(struct sched_mutex_wake_q_node *) sched_mutex_wake_next_atomic_t;


/**
 * Wake-queues are lists of tasks with a pending wakeup, whose
 * callers have already marked the task as woken internally,
 * and can thus carry on. A common use case is being able to
 * do the wakeups once the corresponding user lock as been
 * released.
 *
 * We hold reference to each task in the list across the wakeup,
 * thus guaranteeing that the memory is still valid by the time
 * the actual wakeups are performed.
 */
struct sched_mutex_wake_q_node
{
    sched_mutex_wake_next_atomic_t next;
};


struct sched_mutex_wake_q_head
{
    sched_mutex_wake_next_atomic_t  first;
    sched_mutex_wake_next_atomic_t *lastp;
};



struct sched_mutex
{
    /** spinlock to protect the structure */
    platform_spinlock_t  wait_lock;

    atomic_bool is_pending_owner;
    atomic_bool has_waiters;
    uint8_t     pad[2]; /* supress padding warnings */

    /** rbtree root to enqueue waiters in priority order */
    struct sched_rb_root  waiters;

    /** top waiter */
    struct sched_rb_node *waiters_leftmost;

    /** the mutex owner */
    _Atomic(sched_task_t *) owner;

#ifdef CONFIG_SCHED_DEBUG_MUTEXES
    uint32_t       save_state;
    uint32_t       line;
    const char    *name;
    const char    *file;
    void          *magic;
    atomic_size_t  static_id;
#endif
};

#ifdef CONFIG_SCHED_DEBUG_MUTEXES
#    define SCHED_MUTEX_DEBUG_INITIALIZER , 0, __LINE__, __FUNCTION__, __FILE__, NULL, 0, 0
#endif


#define SCHED_MUTEX_INITIALIZER { PLATFORM_SPINLOCK_INITIALIZER, false, false { false, false }, \
            SCHED_RB_ROOT_INITIALIZER, NULL, NULL SCHED_MUTEX_DEBUG_INITIALIZER }


/**
 * This is the control structure for tasks blocked on a sched_mutex, which is allocated on the
 * kernel stack on of the blocked task.
 */
struct sched_mutex_waiter
{
    /** PI node to enqueue into the mutex waiters tree. */
    struct sched_rb_node tree_entry;

    /** PI node to enqueue into the mutex owner waiters tree. */
    struct sched_rb_node pi_tree_entry;

    /** task reference to the blocked task. */
    sched_task_t *task;

    /** Mutex on which locking is performed */
    struct sched_mutex *lock;

    /** task wakeup queue node (for wakeup by mutex unlocking) */
    struct sched_mutex_wake_q_node wake_q;

    /** Timer node for its cancelling */
    struct sched_timer *timeout;

#ifdef CONFIG_SCHED_DEBUG_MUTEXES
    sched_task_t *deadlock_task_pid;
    struct sched_mutex *deadlock_lock;
    uint32_t ip;
    uint32_t pad; /* supress padding warnings */
#endif

    int32_t retcode;
    sched_prio_t prio;
};


#ifdef CONFIG_SCHED_DEBUG_MUTEXES
#    define  SCHED_MUTEX_WAITER_DEBUG_INITIALIZER NULL, NULL, 0, 0,
#else
#    define  SCHED_MUTEX_WAITER_DEBUG_INITIALIZER
#endif


#define SCHED_MUTEX_WAITER_INITIALIZER \
  { SCHED_RB_NODE_INITIALIZER, SCHED_RB_NODE_INITIALIZER, NULL, NULL, { NULL }, NULL, \
    SCHED_MUTEX_WAITER_DEBUG_INITIALIZER 0, 0 }


#define S




struct sched_mutex_waiter;
struct sched_timer;


#endif // SCHED_MUTEX_TYPES_H
