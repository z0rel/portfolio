#ifndef CO_ROUTINE_H
#define CO_ROUTINE_H

#include <assert.h>

#include "scheduler/task.h"
#include "trace.h"


struct sched_task_struct;

/**
 * @struct co_routine
 *
 * @brief represents co-routine stack frame
 *
 */
struct co_routine
{
    /* previous stack frame */
    addr_t stack_up_begin;
    addr_t stack_up_end;

    void (*run)(struct sched_task_struct *__task);

    /* routine local context to restore */
    void *lc;

    struct sched_task_error error;
};

#define CO_TYPENAME(coroutine_name) struct coroutine_name##_ctx

#define CO_DECLARE_TYPED(coroutine_name, frame_struct)                                             \
    void coroutine_name(struct sched_task_struct *__task)

#define CO_DECLARE(coroutine_name, frame_struct)                                                   \
    CO_TYPENAME(coroutine_name) frame_struct;                                                      \
    void coroutine_name(struct sched_task_struct *__task)

#define CO_ROUTINE_TYPED(coroutine_name, coroutine_typename)                                       \
    void coroutine_name(struct sched_task_struct *__task)                                          \
    {                                                                                              \
        coroutine_typename *self =                                                                 \
            (coroutine_typename *)__task->current_coroutine;

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
        __LC_FORCE_SET(self->routine.lc, __LINE__);                                                \
        goto l_yield;                                                                              \
    __LC_FORCE_ENTRY(__LINE__): ;                                                                  \
    } while (0)

#define CO_YIELD()                                                                                 \
    do                                                                                             \
    {                                                                                              \
        {                                                                                          \
            sched_time_t time_nsec_rq = sched_time_get_relaxed((__task)->cpu);                     \
            sched_stime_t account_ns = time_nsec_rq - (__task)->last_ran;                          \
            if (account_ns)                                                                        \
            {                                                                                      \
                (__task)->cpu->time_nsec_rq = time_nsec_rq;                                        \
                sched_task_time_update((__task), time_nsec_rq, account_ns);                        \
                if (sched_task_time_expired(__task))                                               \
                {                                                                                  \
                    __LC_SET(self->routine.lc, __LINE__);                                          \
                    goto l_yield;                                                                  \
                }                                                                                  \
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
            CO_TYPENAME(coroutine) *frame = (CO_TYPENAME(coroutine) *)__task->return_coroutine;    \
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
        sched_task_routine_start((__rq), (__task));                                                \
        __TRACE(__task,                                                                            \
                TRACE_CALL,                                                                        \
                __module_id,                                                                       \
                "\"entered\": \"%s\", \"at\": \"%s:%d\"",                                          \
                #coroutine,                                                                        \
                __FILE__,                                                                          \
                __LINE__);                                                                         \
    } while (0)

#define CO_SPAWN(coroutine, frame, init_code)                                                      \
    ({                                                                                             \
        struct sched_task_struct *__p = NULL;                                                      \
        sched_task_create(__task->cpu->shared, __task->cpu, &__p, true);                           \
        __p->trace_module = __task->trace_module;                                                  \
        __p->trace_level = __task->trace_level;                                                    \
        CO_START_TASK(__task->cpu, __p, TRACE_MODULE_ID, coroutine, frame, init_code);             \
        __p;                                                                                       \
    })

#define CO_SPAWN_DETACHED(coroutine, frame, init_code)                                             \
    ({                                                                                             \
        struct sched_task_struct *__p = NULL;                                                      \
        sched_task_create(__task->cpu->shared, __task->cpu, &__p, false);                          \
        __p->trace_module = __task->trace_module;                                                  \
        __p->trace_level = __task->trace_level;                                                    \
        CO_START_TASK(__task->cpu, __p, TRACE_MODULE_ID, coroutine, frame, init_code);             \
        __p;                                                                                       \
    })

#define CO_WAITFOR(child, ...)                                                                     \
    do                                                                                             \
    {                                                                                              \
        struct sched_task_struct *join_to =                                                        \
        (struct sched_task_struct *)atomic_load_explicit(&(child)->join_to, memory_order_relaxed); \
        (void)join_to;                                                                             \
        assert((struct sched_task_struct *)-1 == join_to);                                         \
                                                                                                   \
        atomic_store_explicit(&(child)->join_to, __task, memory_order_relaxed);                    \
        __LC_WAIT_CONDITION(sched_task_exited_state(child));                                       \
        /* optional finit code */                                                                  \
        __VA_ARGS__;                                                                               \
        sched_task_destroy(child);                                                                 \
                                                                                                   \
    } while (0)

#define CO_START_TASK_EX(__rq, __task, __module_id, coroutine, co_type, frame, argument_code)      \
    do                                                                                             \
    {                                                                                              \
        __CO_PREPARE((__task), coroutine, co_type, (frame), argument_code);                        \
        sched_task_routine_start((__rq), (__task));                                                \
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
        sched_task_routine_start_ex((__rq), (__task), policy, prio, affinity_mask, cpuset_size);   \
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

#define __CO_IS_DONE(routine) ((routine)->lc == (void *)-1)

#define __CO_PREPARE(__task, coroutine, coroutine_typename, frame, argument_code)                  \
    do                                                                                             \
    {                                                                                              \
        coroutine_typename *frame = NULL;                                                          \
        __co_prepare((struct co_routine **)&frame,                                                 \
                     sizeof(coroutine_typename),                                                   \
                     __task,                                                                       \
                     coroutine);                                                                   \
        assert(frame);                                                                             \
        argument_code;                                                                             \
                                                                                                   \
    } while (0)

static inline void
__co_prepare(struct co_routine **routine,
             size_t size_routine_type,
             struct sched_task_struct *p,
             void (*run)(struct sched_task_struct *__task))
{
    sched_task_routine_push(p, routine, size_routine_type);
    (*routine)->run = run;
}

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
        if (!(condition))                                                                          \
        {                                                                                          \
            sched_task_suspended_set(__task);                                                      \
            __LC_SET(self->routine.lc, __LINE__);                                                  \
            goto l_yield;                                                                          \
        }                                                                                          \
    } while (0)

/** @} */

#endif /* CO_ROUTINE_H */
