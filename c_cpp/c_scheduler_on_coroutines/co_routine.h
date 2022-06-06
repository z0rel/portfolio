#ifndef CO_ROUTINE_H
#define CO_ROUTINE_H

#include <assert.h>

#include "scheduler/sched_task.h"
#include "platform/spinlocks.h"
#include "trace.h"
#include "scheduler/co_routine_types.h"

struct sched_task_t;


#define CO_TYPENAME(coroutine_name) struct coroutine_name##_ctx

#define CO_DECLARE_TYPED(coroutine_name, frame_struct)                                             \
    void coroutine_name(sched_task_t *__task)

#define CO_DECLARE(coroutine_name, frame_struct)                                                   \
    CO_TYPENAME(coroutine_name) frame_struct;                                                      \
    void coroutine_name(sched_task_t *__task)

#define CO_ROUTINE_TYPED(coroutine_name, coroutine_typename)                                       \
    void coroutine_name(sched_task_t *__task)                                          \
    {                                                                                              \
        coroutine_typename *self =                                                                 \
            (coroutine_typename *)sched_task_current_coroutine_relaxed(__task);

#define CO_ROUTINE(coroutine_name) CO_ROUTINE_TYPED(coroutine_name, CO_TYPENAME(coroutine_name))

#define CO_BEGIN                                                                                   \
    do                                                                                             \
    {                                                                                              \
        __LC_RESUME(self->routine.lc);                                                             \
        if (0) self->routine.lc = &&l_yield;                                                       \
    } while (0)

#define CO_END }

#define CO_RETURN(...)                                                                             \
    do                                                                                             \
    {                                                                                              \
        if (unlikely(self->routine.lc == (void *)-1))                                              \
            TRACE(TRACE_CALL,                                                                      \
                  "\"left\": \"%s\", " __FIRST_ARG__(__VA_ARGS__),                                 \
                  __FUNCTION__ __REST_ARGS__(__VA_ARGS__));                                        \
        else                                                                                       \
            TRACE(TRACE_CALL,                                                                      \
                  "\"yielding\": \"%s\", " __FIRST_ARG__(__VA_ARGS__),                             \
                  __FUNCTION__ __REST_ARGS__(__VA_ARGS__));                                        \
        return;                                                                                    \
    } while (0)

#define CO_FORCE_YIELD()                                                                           \
    do                                                                                             \
    {                                                                                              \
        union sched_task_state_t state;                                                            \
        state.state_int32 = sched_task_state_int(__task);                                          \
        __LC_FORCE_SET(self->routine.lc, __LINE__);                                                \
        if (!state.flags.suspended && !state.flags.skip_next_suspend)                              \
            (__task)->time_slice = 0;                                                              \
        goto l_yield;                                                                              \
    __LC_FORCE_ENTRY(__LINE__): ;                                                                  \
    } while (0)



#define co_yield(task)                                                                             \
    do                                                                                             \
    {                                                                                              \
        sched_stime_t __expired =                                                                  \
                task->last_ran_deadline - sched_time_get_relaxed(task->cpu);                       \
        if (sched_task_time_expired(task, __expired))                                              \
            coro_transfer(sched_task_current_coroutine_relaxed(task),                              \
                          &(sched_task_cpu(task)->coro_rq_ctx));                                   \
    } while(0)


#define co_force_yield(task)                                                                       \
    coro_transfer(sched_task_current_coroutine_relaxed(task), &(sched_task_cpu(task)->coro_rq_ctx))


static inline void
sched_task_waitfor(struct sched_task_t *task, sched_task_t *child)
{
    while (!sched_task_state_exited(child))
    {
        sched_task_state_suspended_set(task);
        co_force_yield(task);
    }
}



#define CO_YIELD()                                                                                 \
    do                                                                                             \
    {                                                                                              \
        {                                                                                          \
            sched_stime_t __expired =                                                              \
                (__task)->last_ran_deadline - sched_time_get_relaxed((__task)->cpu);               \
            if (sched_task_time_expired(__task, __expired))                                        \
            {                                                                                      \
                __LC_SET(self->routine.lc, __LINE__);                                              \
                goto l_yield;                                                                      \
            }                                                                                      \
        }                                                                                          \
    __LC_ENTRY(__LINE__): ;                                                                        \
    } while (0)

#define CO_RESTART()                                                                               \
    do                                                                                             \
    {                                                                                              \
        self->routine.lc = NULL;                                                                   \
        goto l_exit;                                                                               \
    } while (0)

#define CO_TRY(callfunc) do { callfunc; error_treat(); } while(0)

#define CO_CALL_TYPED(coroutine, coroutine_typename, frame, init_code, ...)                        \
    do                                                                                             \
    {                                                                                              \
        __CO_PREPARE(__task, coroutine, coroutine_typename, frame, init_code);                     \
        TRACE(TRACE_CALL,                                                                          \
              "\"entered\": \"%s\", \"at\": \"%s:%d\"",                                            \
              #coroutine,                                                                          \
              __FILE__,                                                                            \
              __LINE__);                                                                           \
        __LC_SET(self->routine.lc, __LINE__);                                                      \
        goto l_yield;                                                                              \
    __LC_ENTRY(__LINE__):                                                                          \
        {                                                                                          \
            (void)frame;                                                                           \
            /* optional finit_code */                                                              \
            __VA_ARGS__;                                                                           \
            if (__NARG(__VA_ARGS__) < 3)                                                           \
                error_treat(); /* default error processing */                                      \
        }                                                                                          \
    } while (0)

#define CO_CALL(coroutine, frame, init_code, ...)                                                  \
    CO_CALL_TYPED(coroutine, CO_TYPENAME(coroutine), frame, init_code, __VA_ARGS__)

#define CO_START_TASK(__rq, __task, __module_id, coroutine, frame, argument_code)                  \
    do                                                                                             \
    {                                                                                              \
        __CO_PREPARE((__task), coroutine, CO_TYPENAME(coroutine), (frame), argument_code);         \
        sched_task_start((__rq), (__task));                                                \
        __TRACE(__task,                                                                            \
                TRACE_CALL,                                                                        \
                __module_id,                                                                       \
                "\"entered\": \"%s\", \"at\": \"%s:%d\"",                                          \
                #coroutine,                                                                        \
                __FILE__,                                                                          \
                __LINE__);                                                                         \
    } while (0)


// TODO: need change NULL to coro_fun
#define CO_SPAWN(coroutine, frame, init_code)                                                      \
    ({                                                                                             \
        struct sched_task_t *__p = NULL;                                                      \
        sched_task_create(__task->cpu->shared, __task->cpu, &__p, true);                           \
        __p->trace_module = __task->trace_module;                                                  \
        __p->trace_level = __task->trace_level;                                                    \
        CO_START_TASK(__task->cpu, __p, TRACE_MODULE_ID, coroutine, frame, init_code);             \
        __p;                                                                                       \
    })

#define CO_SPAWN_DETACHED(coroutine, frame, init_code)                                             \
    ({                                                                                             \
        struct sched_task_t *__p = NULL;                                                      \
        sched_task_create(__task->cpu->shared, __task->cpu, &__p, false);                          \
        __p->trace_module = __task->trace_module;                                                  \
        __p->trace_level = __task->trace_level;                                                    \
        CO_START_TASK(__task->cpu, __p, TRACE_MODULE_ID, coroutine, frame, init_code);             \
        __p;                                                                                       \
    })

#define CO_WAITFOR(child, ...)                                                                     \
    do                                                                                             \
    {                                                                                              \
        struct sched_task_t *join_to =                                                        \
        (struct sched_task_t *)atomic_load_explicit(&(child)->join_to, memory_order_relaxed); \
        (void)join_to;                                                                             \
        assert((struct sched_task_t *)-1 == join_to);                                         \
                                                                                                   \
        atomic_store_explicit(&(child)->join_to, __task, memory_order_relaxed);                    \
        __LC_WAIT_CONDITION(sched_task_state_exited(child));                                       \
        /* optional finit code */                                                                  \
        __VA_ARGS__;                                                                               \
        sched_task_destroy(child);                                                                 \
                                                                                                   \
    } while (0)

#define CO_START_TASK_EX(__rq, __task, __module_id, coroutine, co_type, frame, argument_code)      \
    do                                                                                             \
    {                                                                                              \
        __CO_PREPARE((__task), coroutine, co_type, (frame), argument_code);                        \
        sched_task_start((__rq), (__task));                                                \
        __TRACE(__task,                                                                            \
                TRACE_CALL,                                                                        \
                __module_id,                                                                       \
                "\"entered\": \"%s\", \"at\": \"%s:%d\"",                                          \
                #coroutine,                                                                        \
                __FILE__,                                                                          \
                __LINE__);                                                                         \
    } while (0)

#define CO_START_TASK_EXX(__rq,                                                                    \
                          __task,                                                                  \
                          __module_id,                                                             \
                          coroutine,                                                               \
                          frame,                                                                   \
                          policy,                                                                  \
                          prio,                                                                    \
                          affinity_mask,                                                           \
                          cpuset_size,                                                             \
                          argument_code)                                                           \
    do                                                                                             \
    {                                                                                              \
        __CO_PREPARE((__task), coroutine, CO_TYPENAME(coroutine), (frame), argument_code);         \
        sched_task_start_ex((__rq), (__task), policy, prio, affinity_mask, cpuset_size);   \
        __TRACE(__task,                                                                            \
                TRACE_CALL,                                                                        \
                __module_id,                                                                       \
                "\"entered\": \"%s\", \"at\": \"%s:%d\"",                                          \
                #coroutine,                                                                        \
                __FILE__,                                                                          \
                __LINE__);                                                                         \
    } while (0)


/** ****************************************************************************
 * @defgroup internal definitions
 * @{
 */

#define ___CONCAT(x, y)       x ## y

#define __LC_ENTRY(line)       ___CONCAT(__LC_, line)
#define __LC_FORCE_ENTRY(line) ___CONCAT(__FORCE_, line)
#define __LC_ERR_ENTRY(line)   ___CONCAT(__ERR_, line)
#define __LC_SET(lc, line)     (lc) = &&__LC_ENTRY(line);
#define __LC_FORCE_SET(lc, line) (lc) = &&__LC_FORCE_ENTRY(line);
#define __LC_ERR_SET(lc, line) (lc) = &&__LC_ERR_ENTRY(line);

#define sched_task_is_done(task) ((task)->coro_arg == (void*)SIZE_MAX)

#define __CO_PREPARE(__task, coroutine, coroutine_typename, frame, argument_code)                  \
    do                                                                                             \
    {                                                                                              \
        coroutine_typename *frame = NULL;                                                          \
        assert(frame);                                                                             \
        argument_code;                                                                             \
                                                                                                   \
    } while (0)


#define __LC_RESUME(lc)                                                                            \
    do                                                                                             \
    {                                                                                              \
        void *llc = lc;                                                                            \
        (lc) = (void *)-1; /* state routine as not yielded */                                      \
        if ((llc) != NULL)                                                                         \
            goto *(llc);                                                                           \
    } while (0)

#define __LC_WAIT_CONDITION(condition)                                                             \
    do                                                                                             \
    {                                                                                              \
    __LC_ENTRY(__LINE__):                                                                          \
        platform_spin_lock(&(__task)->pi_lock, __task->cpu);                                       \
        if ((condition))                                                                           \
        {                                                                                          \
            platform_spin_unlock(&(__task)->pi_lock);                                              \
            break;                                                                                 \
        }                                                                                          \
        else                                                                                       \
        {                                                                                          \
            sched_task_state_suspended_set(__task);                                                \
            __LC_SET(self->routine.lc, __LINE__);                                                  \
            platform_spin_unlock(&(__task)->pi_lock);                                             \
            goto l_yield;                                                                          \
        }                                                                                          \
    } while (true)

/** @} */

#endif /* CO_ROUTINE_H */
