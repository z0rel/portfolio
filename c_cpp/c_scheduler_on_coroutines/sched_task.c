#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>

#include "scheduler/co_routine.h"
#include "scheduler/coro.h"
#include "scheduler_internal.h"

#define SCHED_TID_PREALLOCATED_TIDS 1024


static void
sched_task_wakeup_suspended(sched_task_t *p);


void
sched_task_set_prio(sched_task_t *p, sched_prio_t prio)
{
    p->normal_prio = prio;
    p->rt_priority = prio;
    p->static_prio = prio;
}


void
sched_task_set_policy(sched_task_t *p, sched_policy_t sched_policy)
{
    p->policy = sched_policy;
}


static inline void
_task_setaffinity(sched_task_t *p, size_t cpuset_size, sched_cpu_set_t cpuset)
{
    assert(sched_task_cpu(p) != NULL);

    memcpy(p->cpus_allowed,
           cpuset,
           sched_min(SCHED_CPUMASK_BYTES(sched_task_cpu(p)->shared->cpu_count), cpuset_size));
}


static inline size_t
sched_task_struct_size(struct scheduler *self)
{
    return sizeof(sched_task_t)
            + sizeof(sched_cpu_set_chunck_atomic)
            * SCHED_CPUMASK_BYTES(self->cpu_count);
}


static void
_task_routine_start(struct sched_rq *cpu,
                    sched_task_t *p,
                    sched_policy_t sched_policy,
                    uint32_t rt_prio,
                    sched_cpu_set_t affined_cpu_set,
                    size_t cpuset_size)
{
    platform_spin_lock(&cpu->lock, cpu);
    platform_spin_lock_init(&p->pi_lock);
    platform_spin_lock(&p->pi_lock, cpu);

    assert(cpu != NULL);

    p->error = NULL;

    if (affined_cpu_set)
        assert(sched_cpu_test(cpu->cpu_num, affined_cpu_set));

    /*  Initialize task context */
    assign_task_cpu(p, cpu);

    if (affined_cpu_set)
        _task_setaffinity(p, cpuset_size, affined_cpu_set);
    else
        sched_task_fill_bitmap_positive(cpu->shared->cpu_count, p->cpus_allowed);

    /* ? Удерживать потенциальный бустинг приоритета, если вызвано из sched_setscheduler(). */
    p->prio = sched_mutex_get_effective_prio(p, p->normal_prio);

    sched_task_set_policy(p, sched_policy);

    if (TASK_POLICY_RT(p))
        p->rt_priority = rt_prio;

    sched_task_set_prio(p, normal_prio(p));

    sched_task_do_wakeup(cpu, p);

    platform_spin_unlock(&cpu->lock);
    platform_spin_unlock(&p->pi_lock);
}


// Степень делителя двойки для упаковки uint32_t tids в size_t слово стека.
#if UINT32_MAX == SIZE_MAX
#   define SCHED_TIDS_SIZE_COEFF 0
#else
#   define SCHED_TIDS_SIZE_COEFF 1
#endif


static inline void
sched_tids_extend(struct sched_tids_context *ctx)
{
   if (unlikely(ctx->tids_stack.sptr == NULL))
   {
       coro_stack_alloc(&ctx->tids_stack, SCHED_TID_PREALLOCATED_TIDS << SCHED_TIDS_SIZE_COEFF);
       ctx->tids = ctx->tids_stack.sptr;
   }
   else
   {
       struct coro_stack s;
       coro_stack_alloc(&s, ctx->tids_stack.ssze << 1);
       memcpy(&s.sptr, ctx->tids_stack.sptr, ctx->tids_stack.ssze);
       coro_stack_free(&ctx->tids_stack);
       ctx->tids_stack = s;
   }
}


sched_tid_t
sched_tids_get(struct sched_rq *rq);


void
sched_tids_put(struct sched_rq *rq, sched_tid_t tid);


sched_tid_t
sched_tids_get(struct sched_rq *rq)
{
    struct sched_tids_context *ctx = &rq->shared->tids_context;
    platform_spin_lock(&ctx->spin, rq);

    sched_tid_t res;
    if (!ctx->free_tids_cnt)
        res = ctx->max_tid++;
    else
        res = ctx->tids[--ctx->free_tids_cnt];

    platform_spin_unlock(&ctx->spin);
    return res;
}


void
sched_tids_put(struct sched_rq *rq, sched_tid_t tid)
{
    static_assert(sizeof(sched_tid_t) == 4, "bad tids divisor, sched_tid_t is not uint32_t");

    struct sched_tids_context *ctx = &rq->shared->tids_context;
    platform_spin_lock(&ctx->spin, rq);

    if (unlikely((ctx->free_tids_cnt >> SCHED_TIDS_SIZE_COEFF) == ctx->tids_stack.ssze))
        sched_tids_extend(ctx);

    ctx->tids[ctx->free_tids_cnt++] = tid;

    platform_spin_unlock(&ctx->spin);
}


static inline void*
coro_stack_push(struct coro_stack *stack, size_t sz)
{
    void *p = stack->sptr;

    stack->sptr += sz;
    assert(stack->ssze >= sz);
    stack->ssze -= sz;

    return p;
}



sched_task_t *
sched_task_create_ex(struct sched_rq *cpu, coro_fun_t coro_fun,
                     void *ctx_ptr, size_t ctx_size, size_t stack_size, bool join_to_parent,
                     sched_task_t *parent)
{
    static const struct coro_context initval_coroutine = CORO_CONTEXT_INITIALIZER;

    static const sched_task_t initval_task_struct = {
        /* current_coroutine   */ NULL,
        /* coro_arg            */ NULL,
        /* coro_stack          */ CORO_STACK_INITIALIZER,
        /* coro_result         */ 0,
        /* trace_module        */ 0,
        /* trace_level         */ 0,
        /* prio                */ 0,
        /* static_prio         */ 0,
        /* normal_prio         */ 0,
        /* rt_priority         */ 0,
        /* policy              */ 0,
        /* tid                 */ 0,
        /* time_slice          */ 0,
        /* deadline            */ 0,
        /* last_ran            */ 0,
        /* last_ran_deadline   */ 0,
        /* cpu                 */ NULL,
        /* node    skiplist    */ SCHED_SKIPLIST_NODE_INITIALIZER,
        /* node_io skiplist    */ SCHED_SKIPLIST_NODE_INITIALIZER,
        /* error               */ 0,
        /* pi_lock             */ PLATFORM_SPINLOCK_INITIALIZER,
        /* cpu_switch_cnt      */ 0,
        /* state               */ SCHED_TASK_STATE_T_INITIALIZER,
        /* usage               */ 0,
        /* valgrind_stack_id   */ SCHED_TASK_VALGRIND_STACK_ID_INITIALIZER
        /* join_to             */ NULL,
        /* task_local_storage  */ { NULL, NULL },
        /* pi_waiters          */ SCHED_RB_ROOT_INITIALIZER,
        /* pi_waiters_leftmost */ NULL,
        /* pi_blocked_on       */ NULL,
    };


    sched_task_t *join_to = NULL;
    struct scheduler *shared = cpu->shared;
    struct coro_context *coro_ctx;
    struct coro_stack sp;
    struct coro_stack sp_wrk;
    void *ctx_ptr_stack = NULL;
    sched_task_t *p;

    if (join_to_parent)
        join_to = (parent == NULL) ? (sched_task_t *)-1 : parent;

    {
        enum sched_return_codes err = coro_stack_alloc(&sp, stack_size);
        if (err != SCHED_EOK)
        {
            atomic_store_explicit(&(cpu->errno_rq), SCHED_EOK, memory_order_relaxed);
            return NULL;
        }
    }

    sp_wrk = sp;

    p = (sched_task_t *)(coro_stack_push(&sp_wrk, sched_task_struct_size(shared)));
    *p = initval_task_struct;
    p->coro_stack = sp;
    p->tid = sched_tids_get(cpu);

    sched_task_set_policy(p, SCHED_POLICY_NORMAL);
    sched_task_set_prio(p, normal_prio(p));

    if (ctx_ptr != NULL && ctx_size)
    {
        ctx_ptr_stack = coro_stack_push(&sp_wrk, ctx_size);
        memcpy(ctx_ptr_stack, ctx_ptr, ctx_size);
    }

    coro_ctx = (struct coro_context *)(coro_stack_push(&sp_wrk, sizeof(struct coro_context)));
    *coro_ctx = initval_coroutine;

    p->coro_arg = ctx_ptr_stack;

    sched_task_current_coroutine_set(p, coro_ctx);

    coro_create(cpu, coro_ctx, coro_fun, &(p->coro_arg), sp_wrk.sptr, sp_wrk.ssze);

    sched_tasks_all_inc(shared);

    atomic_store_explicit(&(p->state.state_int32), 0, SCHED_TASK_STATE_SET_POLICY);
    atomic_store_explicit(&p->join_to, join_to, memory_order_relaxed);
    atomic_store_explicit(&(cpu->errno_rq), SCHED_EOK, memory_order_relaxed);

    return p;
}


void
sched_task_destroy(sched_task_t *p)
{
    assert(sched_task_cpu(p) && !sched_task_state_running(p));
    sched_tids_put(p->cpu, p->tid);
    coro_stack_free(&p->coro_stack);
}


void
sched_task_routine_pop(sched_task_t *p)
{
    struct sched_rq *cpu = sched_task_cpu(p);
    sched_task_t *join_to_local;

    /* we can't have active error in done task */
    assert(!p->error);

    lock_rq_task(cpu, p);

    sched_task_current_coroutine_set(p, NULL); // sched_task_exited_set(p);

    assert(cpu == sched_task_cpu(p) && (cpu->curr == p || cpu->curr == NULL));

    set_current_rq_task(cpu, NULL);

    sched_task_state_running_clear(p);

    sched_tasks_online_dec(cpu);
    sched_tasks_all_dec(cpu);

    join_to_local =
        (sched_task_t *)atomic_load_explicit(&p->join_to, memory_order_relaxed);

    if (!join_to_local)
    {
        unlock_task_rq(cpu, p);
        sched_task_destroy(p);
    }
    else
    {
        unlock_task_rq(cpu, p);
        if ((sched_task_t *)-1 != join_to_local)
            sched_task_wakeup_suspended(join_to_local);
    }
}


void
sched_task_trace(sched_task_t *p, uint64_t module_mask, uint64_t level_mask)
{
    p->trace_module = module_mask;
    p->trace_level = level_mask;
}


void
sched_task_start(struct sched_rq *cpu, sched_task_t *p)
{
    _task_routine_start(cpu,
                        p,
                        /*sched_policy=*/SCHED_POLICY_NORMAL,
                        /*rt_priority =*/SCHED_PRIO_NORMAL,
                        /*affined_cpu_set=*/ NULL, 0);
}


void
sched_task_start_ex(struct sched_rq *cpu,
                            sched_task_t *p,
                            sched_policy_t sched_policy,
                            uint32_t rt_prio,
                            sched_cpu_set_t affined_cpu_set,
                            size_t cpuset_size)
{
    _task_routine_start(cpu, p, sched_policy, rt_prio, affined_cpu_set, cpuset_size);
}



void
sched_task_setaffinity(struct sched_rq *self, sched_task_t *p, size_t cpuset_size,
                       sched_cpu_set_t cpuset)
{
    assert(p != NULL);

    platform_spin_lock(&p->pi_lock, self);
    assert(sched_task_cpu(p) != NULL);

    memcpy(p->cpus_allowed,
           cpuset,
           sched_min(SCHED_CPUMASK_BYTES(sched_task_cpu(p)->shared->cpu_count), cpuset_size));

    platform_spin_unlock(&p->pi_lock);
}


void
sched_task_getaffinity(struct sched_rq *self, sched_task_t *p, size_t cpuset_size,
                       sched_cpu_set_t cpuset)
{
    assert(p != NULL);

    platform_spin_lock(&p->pi_lock, self);
    assert(sched_task_cpu(p) != NULL);

    assert(cpuset_size >= SCHED_CPUMASK_BYTES(sched_task_cpu(p)->shared->cpu_count));
    memcpy(cpuset,
           p->cpus_allowed,
           sched_min(SCHED_CPUMASK_BYTES(sched_task_cpu(p)->shared->cpu_count), cpuset_size));

    platform_spin_unlock(&p->pi_lock);
}


void
sched_task_suspend(sched_task_t *p)
{
    assert(p != NULL);

    do
    {
        struct sched_rq *cpu = sched_task_cpu(p);

        if (unlikely(sched_task_state_flag_suspended_relaxed(p) || sched_task_state_exited(p)))
            return;

        assert(cpu != NULL);

        lock_rq_task(cpu, p);

        if (unlikely(sched_task_cpu(p) != cpu))
        {
            unlock_task_rq(cpu, p);
            continue;
        }

        if (!sched_task_state_running_relaxed(p))
        {
            /* Если задача не находится в skiplist - значит она должна уже быть suspended */
            if (!sched_task_state_queued(p))
            {
                assert(sched_task_state_suspended(p));
                unlock_task_rq(cpu, p);
                break;
            }
            else
            {
                /* Если задача находится в skiplist - ее можно безопасно оттуда извлечь, т.к.
                   runqueue заблокирована. Но она не должна уже suspended. */
                assert(!sched_task_state_suspended(p));
                dequeue_task(cpu, p);
                sched_tasks_online_dec(cpu);
            }
        }

        sched_task_state_suspended_set(p);

        unlock_task_rq(cpu, p);
        break;
    } while (true);
}


static inline void
sched_task_wakeup_step(struct sched_rq *cpu, sched_task_t *p)
{
    union sched_task_state_t state;
    state.state_int32 = sched_task_state_int(p);

    /* No need in wakeup if the task is running or on wakeup list. */
    if (unlikely(!state.flags.suspended || state.flags.onwakeup ||  sched_task_state_exited(p)))
    {
        if (state.flags.running)
            sched_task_state_skip_next_suspend_set(p);
    }
    else
    {

        assert(!sched_task_state_queued(p));

        sched_task_state_onwakeup_set(p);

        /* If the task has not yet passed to the suspended state, then it was
         * not removed from the scheduling.
         * The scheduler itself will queue the task to skiplist.
         */
        if (likely(!sched_task_state_running_relaxed(p)))
            sched_wakeup_push(cpu, p);
    }
}


void
sched_task_wakeup(sched_task_t *p)
{
    assert(p != NULL);

    do
    {
        union sched_task_state_t state;
        struct sched_rq *cpu = sched_task_cpu(p);
        state.state_int32 = sched_task_state_int(p);

        if (unlikely(!state.flags.suspended || sched_task_state_exited(p)))
        {
            if (state.flags.running)
                sched_task_state_skip_next_suspend_set(p);
            break;
        }

        assert(cpu != NULL);

        lock_rq_task(cpu, p);

        if (likely(sched_task_cpu(p) != cpu))
        {
            /* The task after suspending should save cpu */
            assert(sched_task_state_running_relaxed(p));
        }
        else
        {
            /* The status of the task might have changed after acquiring the locks:
             * The task could not have time to go into suspend state.
             * It could also go to the exited state.
             */
            sched_task_wakeup_step(cpu, p);
            unlock_task_rq(cpu, p);
            break;
        }

        unlock_task_rq(cpu, p);
    } while (true);
}


static void
sched_task_wakeup_suspended(sched_task_t *p)
{
    assert(p != NULL);

    do
    {
        struct sched_rq *cpu = sched_task_cpu(p);
        assert(cpu != NULL);
        lock_rq_task(cpu, p);

        assert(sched_task_state_suspended(p));
        assert(!sched_task_state_exited(p));

        if (likely(sched_task_cpu(p) != cpu))
        {
            /* The task after suspending should save cpu */
            assert(sched_task_state_running_relaxed(p));
        }
        else
        {
            /* The status of the task might have changed after acquiring the locks:
             * The task could not have time to go into suspend state.
             * It could also go to the exited state.
             */
            sched_task_wakeup_step(cpu, p);
            unlock_task_rq(cpu, p);
            break;
        }

        unlock_task_rq(cpu, p);
    } while (true);
}
