#ifndef SCHED_TASK_FLAGS_H
#define SCHED_TASK_FLAGS_H

#include "scheduler/sched_task_types.h"

/** Получить указатель на текущую корутину */
#define sched_task_current_coroutine(task) \
    atomic_load_explicit(&(task)->current_coroutine, SCHED_TASK_CURRENT_GET_POLICY)


/** Получить указатель на текущую корутину */
#define sched_task_current_coroutine_relaxed(task) \
    atomic_load_explicit(&(task)->current_coroutine, SCHED_TASK_CURRENT_GET_RELAXED_POLICY)


/** Установить указатель на текущую корутину */
#define sched_task_current_coroutine_set(task, ptr) \
    atomic_store_explicit(&(task)->current_coroutine, ptr, SCHED_TASK_CURRENT_SET_POLICY)


/** Получить указатель на рабочую нить текущей задачи */
#define sched_task_cpu(task) \
    atomic_load_explicit(&(task)->cpu, SCHED_TASK_CPU_GET_POLICY)


/** Установить указатель на рабочую нить текущей задачи */
#define sched_task_cpu_set(task, cpu_ptr) \
    atomic_store_explicit(&(task)->cpu, (cpu_ptr), SCHED_TASK_CPU_SET_POLICY)


/** Атомарно получить int32 представляющий состояние текущей задачи */
#define sched_task_state_int(task) \
    atomic_load_explicit(&(task)->state.state_int32, SCHED_TASK_STATE_GET_POLICY)


/** Атомарно установить int32 представляющий состояние текущей задачи */
#define sched_task_state_int_set(state_i32) \
    atomic_store_explicit(&(task)->state.state_int32, (state_i32), SCHED_TASK_STATE_SET_POLICY)


/** Атомарно получить флаг текущей задачи "приостановлена" */
#define sched_task_state_flag_suspended_relaxed(task) \
    atomic_load_explicit(&(task)->state.flags.suspended, SCHED_TASK_STATE_GET_RELAXED_POLICY)


/**
 * @def sched_task_time_expired(p)
 *
 * @brief         The SCHED_BATCH tasks are marked as insensitive to delays.
 *                Their timeslice is refilled every time they are rescheduled,
 *                but they get a new later deadline,
 *                to less influence on SCHED_NORMAL tasks
 */
#define sched_task_time_expired(p, timeslice) \
    (timeslice) < SCHED_TIME_RESCHED_NS || SCHED_TASK_POLICY_BATCH(p)


/**
 * Skiplist contains only tasks that are pending execution.
 *
 * Running tasks, and tasks pending event - are not contained in the skiplist.
 */
static inline bool
sched_task_state_queued(sched_task_t *p)
{
    return !sched_skiplist_node_empty(&p->node);
}


static inline bool
sched_task_state_exited(sched_task_t *p)
{
    return sched_task_current_coroutine(p) == NULL;
}


static inline bool
sched_task_exited_state_fast(sched_task_t *p)
{
    return sched_task_current_coroutine_relaxed(p) != NULL;
}


static inline bool
sched_task_state_suspended(sched_task_t *p)
{
    union sched_task_state_t state;
    state.state_int32 = sched_task_state_int(p);
    return state.flags.suspended && !state.flags.skip_next_suspend;
}


static inline bool
sched_task_state_suspended_force(sched_task_t *p)
{
    return atomic_load_explicit(&p->state.flags.suspended, SCHED_TASK_STATE_GET_POLICY);
}


static inline void
sched_task_state_suspended_set(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.suspended, true, SCHED_TASK_STATE_SET_POLICY);
}


/** Атомарно очистить флаг текущей задачи "приостановлена" */
static inline void
sched_task_state_suspended_clear(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.suspended, false, SCHED_TASK_STATE_SET_POLICY);
}


static inline bool
sched_task_state_onwakeup(sched_task_t *p)
{
    return atomic_load_explicit(&(p->state.flags.onwakeup), SCHED_TASK_STATE_GET_POLICY);
}

static inline bool
sched_task_state_onwakeup_relaxed(sched_task_t *p)
{
    return atomic_load_explicit(&p->state.flags.onwakeup, SCHED_TASK_STATE_GET_RELAXED_POLICY);
}


static inline void
sched_task_state_onwakeup_set(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.onwakeup, true, SCHED_TASK_STATE_SET_POLICY);
}


/** Атомарно очистить флаг текущей задачи "на пробуждении" */
static inline void
sched_task_state_onwakeup_clear(sched_task_t *p)
{
    atomic_store_explicit(&(p->state.flags.onwakeup), false, SCHED_TASK_STATE_SET_POLICY);
}


static inline void
sched_task_state_onwakeup_clear_relaxed(sched_task_t *p)
{
    atomic_store_explicit(&p->state.flags.onwakeup, false, SCHED_TASK_STATE_SET_RELAXED_POLICY);
}


#endif // SCHED_TASK_FLAGS_H
