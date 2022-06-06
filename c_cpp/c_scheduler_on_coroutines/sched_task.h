#ifndef SCHED_TASK_H
#define SCHED_TASK_H

#include "scheduler/sched_errno.h"
#include "scheduler/sched_task_flags.h"



/**
 * @brief sched_task_create_ex
 *     Initialize the task context. Queue the task for execution.
 * @param cpu
 * @param coro_fun
 * @param ctx_ptr
 * @param ctx_size
 * @param stack_size
 * @param join_to_parent
 * @return
 */
sched_task_t *
sched_task_create_ex(struct sched_rq *cpu, coro_fun_t coro_fun, void *ctx_ptr,
                     size_t ctx_size, size_t stack_size, bool join_to_parent, sched_task_t *parent);



static inline sched_task_t *
sched_task_create(struct sched_rq *cpu, coro_fun_t coro_fun, void *ctx_ptr, size_t ctx_size,
                  size_t stack_size)
{
    return sched_task_create_ex(cpu, coro_fun, ctx_ptr, ctx_size, stack_size, false, NULL);
}

/**
 * @brief sched_task_create_attached
 * @param cpu
 * @param coro_fun
 * @param ctx_ptr
 *     Указатель на память, которая будет скопирована в стек корутины как ее контекст
 *     Адрес в стеке затем будет передан как аргумент самой корутине.
 * @param ctx_size
 *     Размер контекста корутины.
 * @param stack_size
 * @param parent
 *     Предок корутины, который нужно разбудить по ее завершении.
 *     Если задан NULL, то корутина не будет автоматически уничтожаться.
 * @return
 */
static inline sched_task_t *
sched_task_create_attached(struct sched_rq *cpu, coro_fun_t coro_fun, void *ctx_ptr,
                           size_t ctx_size, size_t stack_size, sched_task_t *parent)
{
    return sched_task_create_ex(cpu, coro_fun, ctx_ptr, ctx_size, stack_size, true, parent);
}


/** Destroy the task context. */
void
sched_task_destroy(sched_task_t *p);


/** Initialize the scheduler context. */
bool
sched_tasks_init(struct scheduler *sched);



static inline void *
sched_get_key(sched_task_t *p, enum task_local_storage_key key)
{
    return p->task_local_storage[key];
}


static inline void
sched_set_key(sched_task_t *p, enum task_local_storage_key key, void *value)
{
    p->task_local_storage[key] = value;
}


void
sched_task_trace(sched_task_t *p, uint64_t module_mask, uint64_t level_mask);


void
sched_task_routine_pop(sched_task_t *p);


/** Start the new task with NORMAL policy and NORMAL prio*/
void
sched_task_start(struct sched_rq *cpu, sched_task_t *p);


/** Start the new task with customized policy and prio*/
void
sched_task_start_ex(struct sched_rq *cpu,
                    sched_task_t *p,
                    sched_policy_t sched_policy,
                    uint32_t rt_prio,
                    sched_cpu_set_t affined_cpu_set,
                    size_t cpuset_size);


/** Set of the task affinity mask */
void
sched_task_setaffinity(struct sched_rq *self, sched_task_t *p, size_t cpuset_size,
                       sched_cpu_set_t cpuset);


/** Get of the task affinity mask */
void
sched_task_getaffinity(struct sched_rq *self, sched_task_t *p, size_t cpuset_size,
                       sched_cpu_set_t cpuset);


/** Suspend of the task. The task will be removed from the scheduling. It goes to sleep mode. */
void
sched_task_suspend(sched_task_t *p);


/** Wake up task. The task will be queued for execution. */
void
sched_task_wakeup(sched_task_t *p);


/**
 * @brief sched_task_current
 *     Получить указатель на структуру текущей задачи по аргументу, переданному в корутину.
 *
 * @param arg
 *     Аргумент, переданный в корутину
 */
#define sched_task_current(arg) ((sched_task_t *)(arg - offsetof(sched_task_t, coro_arg)))


/** ****************************************************************************
 * @defgroup error processing facility
 *
 * @brief         This error processing facility
 *                is to be used in all co-routine implementations.
 *
 * @{
 */


/**
 * @def error_get()
 *
 * @brief         returns pointer to error structure in current task
 *                if present or NULL
 *
 * @retval        none NULL pointer to struct sched_task_error
 *
 * @retval        NULL  no error
 *
 *
 */
#define error_get() __task->error


#define ERROR_MSG_SIZE 256
/**
 * @def error_set(sys_err, our_err, ...)
 *
 * @brief         sets error presence with its attributes
 *
 * @param[in]     sys_err        system error code.
 * @param[in]     our_err        our error code.
 * @param[in]     ...            arguments for debug format string
 *
 */
#define error_set(sys_err, our_err, ...)                                                           \
    do                                                                                             \
    {                                                                                              \
        addr_t addr; /* value to ignore */                                                         \
        int size = (__FIRST_ARG__(__VA_ARGS__) "")[0] ? ERROR_MSG_SIZE : sizeof("\n");             \
        assert(!__task->error);                                                                    \
        __task->error = /* ATTENTION: the allocated memory freed on routine pop only */            \
            vm_stack_alloc(__task->stack, sizeof(struct sched_task_error) + size, &addr);          \
        __task->error->frame = sched_task_current_coroutine(__task);                               \
        __task->error->sys_errno = sys_err;                                                        \
        __task->error->our_errno = our_err;                                                        \
        __task->error->line = __LINE__;                                                            \
        __task->error->file = __FILE__;                                                            \
        snprintf(__task->error->msg,                                                               \
                 size,                                                                             \
                 __FIRST_ARG__(__VA_ARGS__) "\n" __REST_ARGS__(__VA_ARGS__));                      \
        __task->error->msg[size - 1] = 0;                                                          \
    } while (0)


/**
 * @def error_treat()
 *
 * @brief         checks error presence,
 *                saves context,
 *                jumps to 'l_error:' to process error
 *
 */
#define error_treat()                                                                              \
    do                                                                                             \
    {                                                                                              \
        if (unlikely(__task->error && (__task->error->sys_errno || __task->error->our_errno)))     \
        {                                                                                          \
            __LC_ERR_SET(__task->error->lc, __LINE__);                                             \
            goto l_error;                                                                          \
        __LC_ERR_ENTRY(__LINE__):                                                                  \
            ;                                                                                      \
        }                                                                                          \
    } while (0)


/**
 * @def error_clean()
 *
 * @brief         cleans the error
 *
 */
#define error_clean() __task->error = NULL


/**
 * @def error_resume()
 *
 * @brief         resumes normal processing without error cleaning
 *
 */
#define error_resume() __LC_RESUME(__task->error->lc)


/**
 * @def error_clean_resume()
 *
 * @brief         resumes normal processing with error cleaning
 *
 */
#define error_clean_resume()                                                                       \
    do                                                                                             \
    {                                                                                              \
        struct sched_task_error *error = __task->error;                                            \
        error_clean();                                                                             \
        __LC_RESUME(error->lc);                                                                    \
    } while (0)


/**
 * @def error_push(sys_err, our_err, ...)
 *
 * @brief         pushes onto stack new error interpretation
 *
 * @param[in]     sys_err        new system error code.
 * @param[in]     our_err        new our error code.
 * @param[in]     ...            arguments for debug format string
 *
 */
#define error_push(sys_err, our_err, ...)                                                          \
    do                                                                                             \
    {                                                                                              \
        struct sched_task_error *error = __task->error;                                            \
        assert(error);                                                                             \
        __task->error = NULL;                                                                      \
        error_set(sys_err, our_err, __FIRST_ARG__(__VA_ARGS__) "" __REST_ARGS__(__VA_ARGS__));     \
        __task->error->next = error;                                                               \
    } while (0);


/**
 * @def error_pop(sys_err, our_err, ...)
 *
 * @brief         pops from stack current error interpretation
 *                restoring the previous one.
 *
 */
#define error_pop()                                                                                \
    do                                                                                             \
    {                                                                                              \
        assert(__task->error && __task->error->next);                                              \
        __task->error = __task->error->next;                                                       \
    } while (0);


/** @} */


#endif // SCHED_TASK_H
