/**
 * @file
 * @brief scheduler mutexes: blocking mutual exclusion locks with PI support
 */

#ifndef SCHED_MUTEX_COMMON_H
#define SCHED_MUTEX_COMMON_H

#include <assert.h>

#include "config_sched.h"
#include "scheduler/sched_task_types.h"
#include "scheduler/sched_mutex.h"
#include "scheduler/sched_rbtree.h"



#define WAKE_Q_TAIL ((struct sched_mutex_wake_q_node *)0x01)

/** The WAKE_Q macro declares and initializes the list head. */
#define WAKE_Q(name) struct sched_mutex_wake_q_head name = {WAKE_Q_TAIL, &name.first}


struct sched_mutex_wake_q_head;


/** Various helpers to access the waiters-tree */
static inline int
sched_mutex_has_waiters(struct sched_mutex *lock)
{
    return !RB_EMPTY_ROOT(&lock->waiters);
}


static inline struct sched_mutex_waiter *
sched_mutex_top_waiter(struct sched_mutex *lock)
{
    struct sched_mutex_waiter *w = rb_entry(lock->waiters_leftmost, struct sched_mutex_waiter,
                                            tree_entry);

    assert(w->lock == lock);
    return w;
}


static inline int
task_has_pi_waiters(sched_task_t *p)
{
    return !RB_EMPTY_ROOT(&p->pi_waiters);
}


static inline struct sched_mutex_waiter *
task_top_pi_waiter(sched_task_t *p)
{
    return rb_entry(p->pi_waiters_leftmost, struct sched_mutex_waiter, pi_tree_entry);
}


/** lock->owner state tracking: */
#define SCHED_MUTEX_HAS_WAITERS 1UL
#define SCHED_MUTEX_OWNER_MASKALL 1UL


static inline sched_task_t *
sched_mutex_owner(struct sched_mutex *lock)
{
    return atomic_load_explicit(&lock->owner, memory_order_relaxed);
}


#ifdef CONFIG_SCHED_DEBUG_MUTEXES
#include "sched_mutex_debug.h"
#else
#include "sched_mutex_nodebug.h"
#endif


#endif // SCHED_MUTEX_COMMON_H
