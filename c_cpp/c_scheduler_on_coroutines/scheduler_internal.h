#ifndef SCHEDULER_INTERNAL_H
#define SCHEDULER_INTERNAL_H

#include <assert.h>
#include <stddef.h>

#include "platform/barrier.h"
#include "scheduler/sched_task.h"
#include "scheduler/sched_timer_types.h"
#include "scheduler/skip_list.h"
#include "scheduler/cpumask_op.h"
#include "platform/spinlocks.h"
#include "platform/mt19937.h"


#define TASK_ON_RQ_QUEUED	1
#define TASK_ON_RQ_MIGRATING	2


/*
 * {de,en}queue flags:
 *
 * DEQUEUE_SLEEP  - task is no longer runnable
 * ENQUEUE_WAKEUP - task just became runnable
 *
 * SAVE/RESTORE - an otherwise spurious dequeue/enqueue, done to ensure tasks
 *                are in a known state which allows modification. Such pairs
 *                should preserve as much state as possible.
 *
 * MOVE - paired with SAVE/RESTORE, explicitly does not preserve the location
 *        in the runqueue.
 *
 * ENQUEUE_HEAD      - place at front of runqueue (tail if not specified)
 * ENQUEUE_REPLENISH - CBS (replenish runtime and postpone deadline)
 * ENQUEUE_MIGRATED  - the task was migrated during wakeup
 *
 */

#define DEQUEUE_SLEEP		0x01
#define DEQUEUE_SAVE		0x02 /* matches ENQUEUE_RESTORE */
#define DEQUEUE_MOVE		0x04 /* matches ENQUEUE_MOVE */

#define ENQUEUE_WAKEUP		0x01
#define ENQUEUE_RESTORE		0x02
#define ENQUEUE_MOVE		0x04

#define ENQUEUE_HEAD		0x08
#define ENQUEUE_REPLENISH	0x10
#define ENQUEUE_MIGRATED	0x20



#define TASK_RUNNING         0
#define TASK_INTERRUPTIBLE   1
#define TASK_UNINTERRUPTIBLE 2
#define TASK_DEAD            64
#define TASK_NEW             2048
#define TASK_NORMAL (TASK_INTERRUPTIBLE | TASK_UNINTERRUPTIBLE)

#define TASK_POLICY_RT(p)      unlikely((p)->policy == SCHED_POLICY_FIFO        \
                                        || (p)->policy == SCHED_POLICY_RR)
#define TASK_POLICY_IDLE(p)    unlikely((p)->policy == SCHED_POLICY_IDLE)
#define TASK_POLICY_ISO(p)     unlikely((p)->policy == SCHED_POLICY_ISO)

#define TASK_PRIO_NOT_RT(prio) likely((prio) >= SCHED_PRIO_MAX_RT)
#define TASK_PRIO_IDLE(p)      unlikely((p)->prio == SCHED_PRIO_IDLE)
#define TASK_PRIO_ISO(p)       unlikely((p)->prio == SCHED_PRIO_ISO)


#define sched_min(a, b)                                                                            \
    ({                                                                                             \
        __typeof__(a) _a = (a);                                                                    \
        __typeof__(b) _b = (b);                                                                    \
        _a < _b ? _a : _b;                                                                         \
    })


struct pin_cookie
{
};


struct rq_flags
{
    unsigned long flags;
    struct pin_cookie cookie;
};


void
sched_task_set_prio(sched_task_t *p, sched_prio_t prio);


void
sched_task_set_policy(sched_task_t *p, sched_policy_t sched_policy);


void
sched_task_do_wakeup(struct sched_rq *cpu, sched_task_t *p);


void
sched_wakeup_vector_extend(struct sched_wakeup_vector *container);


static inline int
normal_prio(sched_task_t *p)
{
    if (TASK_POLICY_RT(p))
        return SCHED_PRIO_MAX_RT - 1 - p->rt_priority;
    if (TASK_POLICY_IDLE(p))
        return SCHED_PRIO_IDLE;
    if (TASK_POLICY_ISO(p))
        return SCHED_PRIO_ISO;
    return SCHED_PRIO_NORMAL;
}


static inline void
assign_task_cpu(sched_task_t *task, struct sched_rq *cpu)
{
    if (sched_task_cpu(task) != cpu)
        ++task->cpu_switch_cnt;
    sched_task_cpu_set(task, cpu);
}


#define update_clocks_by_time(rq, time) (rq)->time_local_rq = (time);



static inline void
update_clocks(struct sched_rq *rq)
{
    (rq)->time_local_rq = sched_time_get(rq);
}


static inline void
set_current_rq_task(struct sched_rq *cpu, sched_task_t *p)
{
    cpu->curr = p;
}


static inline void
sched_tasks_online_inc(struct sched_rq *cpu)
{
    atomic_fetch_add_explicit(&cpu->shared->tasks_online, 1, memory_order_relaxed);
}


static inline void
sched_tasks_online_dec(struct sched_rq *cpu)
{
    atomic_fetch_sub_explicit(&cpu->shared->tasks_online, 1, memory_order_relaxed);
    assert(atomic_load_explicit(&cpu->shared->tasks_online, memory_order_relaxed) >= 0);
}


#define sched_tasks_all_inc(sched) \
    atomic_fetch_add_explicit(&((sched)->tasks_all), 1, SCHED_ONLINE_GET_POLICY)


static inline void
sched_tasks_all_dec(struct sched_rq *cpu)
{
    atomic_fetch_sub_explicit(&cpu->shared->tasks_all, 1, memory_order_relaxed);
    assert(atomic_load_explicit(&cpu->shared->tasks_all, memory_order_relaxed) >= 0);
}


static inline bool
sched_task_state_running(sched_task_t *p)
{
    return atomic_load_explicit(&p->state.flags.running, SCHED_TASK_STATE_GET_POLICY);
}


static inline bool
sched_task_state_running_relaxed(sched_task_t *p)
{
    return atomic_load_explicit(&p->state.flags.running, SCHED_TASK_STATE_GET_RELAXED_POLICY);
}


static inline void
sched_task_state_running_set(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.running, true, SCHED_TASK_STATE_SET_POLICY);
}


static inline void
sched_task_state_running_clear(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.running, false, SCHED_TASK_STATE_SET_POLICY);
}


static inline void
sched_task_state_skip_next_suspend_set(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.skip_next_suspend, true, SCHED_TASK_STATE_SET_POLICY);
}


static inline void
sched_task_state_skip_next_suspend_clear(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.skip_next_suspend, false, SCHED_TASK_STATE_SET_POLICY);
}


static inline void
lock_rq_task(struct sched_rq *cpu, sched_task_t *p)
{
    platform_spin_lock(&cpu->lock, cpu);
    platform_spin_lock(&p->pi_lock, cpu);
}


static inline void
unlock_task_rq(struct sched_rq *cpu, sched_task_t *p)
{
    platform_spin_unlock(&p->pi_lock);
    platform_spin_unlock(&cpu->lock);
}


static inline void
unlock_rq_task(struct sched_rq *cpu, sched_task_t *p)
{
    platform_spin_unlock(&cpu->lock);
    platform_spin_unlock(&p->pi_lock);
}


static inline void
sched_wakeup_push(struct sched_rq *cpu, sched_task_t *p)
{
    struct sched_wakeup_vector *container = &cpu->wakeup_tasks;
    if (unlikely(container->max_size == container->size))
        sched_wakeup_vector_extend(container);
    container->data[container->size++] = p;
}


static inline void
set_best_rq_key(struct sched_rq *self, sched_skiplist_key_type_t key)
{
    assert(key != 0);
    self->best_key = key;
}


/*
 * Deadline is the some timepoint + offset, based on the priorities.
 * Deadline is the basis for:
 *  - distribution of CPU time between tasks with the same nice level,
 *  - distribution of CPU time according to the nice level.
 *
 * The CPU time proportions are set so that the task of the nice 19,
 * got 3% of the processor in comparison with the nice 0.
 */
static inline sched_time_t
prio_deadline_diff(struct sched_rq *self, int user_prio)
{
    assert(user_prio >= 0 && user_prio < SCHED_NICE_WIDTH);

    /* >> 7 <=> divide to 128 */
    return ((self->shared->prio_ratios[user_prio] * self->shared->timeslice_interval) >> 7);
}


static inline void
update_last_ran(struct sched_rq *self, sched_task_t *p)
{
    sched_time_t last_ran = self->time_local_rq;
    p->last_ran = last_ran;
    p->last_ran_deadline = last_ran + p->time_slice;
}


static inline void
update_task_timeslice_as_diff(struct sched_rq *self, sched_task_t *p)
{
    sched_time_t time_shared = sched_time_get(self);

    update_clocks_by_time(self, time_shared)

    if (likely(p->policy != SCHED_POLICY_FIFO))
        p->time_slice -= (time_shared - p->last_ran);
}


/** Timeslice value, that given to tasks of all levels of priorities. */
static inline sched_stime_t
timeslice(const struct sched_rq *self)
{
    return self->shared->timeslice_interval;
}


static inline sched_time_t
task_deadline_diff(struct sched_rq *self, sched_task_t *p)
{
    return prio_deadline_diff(self, SCHED_PRIO_USER(p));
}


/**
 * time_slice is refilled only when it is empty and when we set the new deadline.
 */
static inline void
time_slice_expired(struct sched_rq *rq, sched_task_t *p)
{
    p->time_slice = timeslice(rq);
    p->deadline = rq->time_local_rq + task_deadline_diff(rq, p);
}


static inline void
check_deadline(struct sched_rq *self, sched_task_t *p)
{
    if (sched_task_time_expired(p, p->time_slice))
        time_slice_expired(self, p);
    else
        (void)p; // for debug
}


/**
 * Removing from the runqueue.
 * Enter with the locked runqueue.
 * Deleting takes O(k), where k is the "level" of the task that was saved.
 * Usually it is < 4, maximum is 8.
 */
static inline void
dequeue_task(struct sched_rq *self, sched_task_t *p)
{
    assert(sched_task_cpu(p) == self);
    sched_skiplist_delete(&self->skiplist, &p->node);
    set_best_rq_key(self, self->skiplist.head_node.next[0]->key);
}


/**
 * Adding to the runqueue.
 * Enter with the locked runqueue.
 */
static inline void
enqueue_task(struct sched_rq *self, sched_task_t *p)
{
    rand_mt19937_val_t randseed;
    sched_skiplist_key_type_t sl_id;
    assert(sched_task_cpu(p) == self);
    assert(!sched_task_state_queued(p));

    /* p->prio in (ISO_PRIO, NORMAL_PRIO, IDLE_PRIO, PRIO_LIMIT, ... MAX_PRIO) */
    if (TASK_PRIO_NOT_RT(p->prio))
    {
        if ((TASK_POLICY_IDLE(p)) || (TASK_POLICY_ISO(p)))
            p->prio = p->normal_prio;
        else
            p->prio = SCHED_PRIO_NORMAL;
    }

    /*
     * The sorted skiplist is created based on the sl_id key.
     * Real-time and ISO tasks are inserted and sorted based on the priority value.
     * The skiplist puts tasks with same keys after already conained (in FIFO order).
     * The tasks with normal, batch, and idle prio are sorted according to their deadlines.
     * The idleprio tasks are shifted to the value of an impossibly big deadline,
     * ensuring that they are inserted
     * in later positions that still correspond to their own deadlines.
     *
     * Thus, at the top of the skiplist are tasks with the real-time priorities (0 ... ISO_PRIO),
     * and at the end - tasks with the lowest priorities (IDLE).
     * Inserting in the skiplist has O(log n) complexity.
     */
    if (p->prio <= SCHED_PRIO_ISO)
        sl_id = p->prio;
    else
    {
        sl_id = p->deadline;

        if (TASK_POLICY_IDLE(p))
        {
            if (p->prio == SCHED_PRIO_IDLE)
                sl_id |= (0xFUL << (sizeof(sl_id) * 8 - 4));
            else
                sl_id += prio_deadline_diff(self, 39); /* longest_deadline_diff */
        }
    }

    /*
     * The timer resolution is milliseconds.
     * Microseconds are masked as the random grain for insertion into the skiplist.
     */
    randseed = rand_mt19937_rand_uint32(&self->random_gen);

    assert(!sched_task_state_queued(p));
    sched_skiplist_insert(&self->skiplist, &p->node, sl_id, p, randseed);
    set_best_rq_key(self, self->skiplist.head_node.next[0]->key);
}


/**
 * task_rq_lock - lock p->pi_lock and lock the rq
 * @p resides on.
 */
static inline struct sched_rq *
lock_task_rq_strong(struct sched_rq *self, sched_task_t *p)
{
    struct sched_rq *rq;

    for (;;)
    {
        platform_spin_lock(&p->pi_lock, self);
        rq = sched_task_cpu(p);
        platform_spin_lock(&rq->lock, self);
        /*
         *	move_queued_task()		task_rq_lock()
         *
         *	ACQUIRE (rq->lock)
         *	[S] ->on_rq = MIGRATING		[L] rq = task_rq()
         *	WMB (__set_task_cpu())		ACQUIRE (rq->lock);
         *	[S] ->cpu = new_cpu		[L] task_rq()
         *					[L] ->on_rq
         *	RELEASE (rq->lock)
         *
         * If we observe the old cpu in task_rq_lock, the acquire of
         * the old rq->lock will fully serialize against the stores.
         *
         * If we observe the new cpu in task_rq_lock, the acquire will
         * pair with the WMB to ensure we must then also see migrating.
         */
        if (likely(rq == sched_task_cpu(p)))
            return rq;

        platform_spin_unlock(&rq->lock);
        platform_spin_unlock(&p->pi_lock);

        platform_cpu_relax();
    }
}


#define MAX_DL_PRIO 0


static inline bool
dl_prio(sched_prio_t prio)
{
    if (unlikely(prio < MAX_DL_PRIO))
        return true;
    return false;
}


static inline bool
dl_time_before(uint64_t a, uint64_t b)
{
    return (int64_t)((int64_t)a - (int64_t)b) < 0;
}


sched_prio_t
sched_mutex_get_effective_prio(sched_task_t *task, sched_prio_t newprio);


#define get_task_struct(tsk)                                                                       \
    do                                                                                             \
    {                                                                                              \
        atomic_fetch_add_explicit(&(tsk)->usage, 1, memory_order_relaxed);                         \
    } while (0)


static inline void
put_task_struct(sched_task_t *t)
{
    if (atomic_fetch_sub_explicit(&t->usage, 1, memory_order_relaxed) < 2)
    {
        //		__put_task_struct(t);
        // TODO: put task to free. Whether it is necessary in a user space?
    }
}


// TODO: need to implement
#define set_tsk_need_resched(tsk) do { } while (0)


/* === END SKIP === */


#endif // SCHEDULER_INTERNAL_H
