#include <assert.h>
#include <stdio.h>

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include "scheduler/co_routine.h"
#include "find_shared.h"
#include "scheduler/cpumask_op.h"


struct test_affined_wakeup_ctx
{
    sched_task_t *self_ctx;

    uint32_t affined_cpu;
    int32_t suspend_pass;

    uint32_t sleep_iteration_interval;
    uint32_t schedule_deschedule_cnt_max;

    atomic_uint_least32_t schedule_deschedule_cnt;

    atomic_bool need_to_sleep;
    atomic_bool task_finished;
    atomic_bool running_flag;
    atomic_bool pad;
};


struct test_wakeup_manager_ctx
{
    struct test_affined_wakeup_ctx *tasks_for_wakeup;
    size_t ntasks;
    ssize_t suspend_pass_max;
};


void
test_affined_wakeup(void *arg);


void
test_wakeup_manager(void *arg);


static void *
test_schedule_cpu_loop(void *cpu_sched_rq);


int
sched_main(struct scheduler *sched, int argc, char *argv[]);


void
test_affined_wakeup(void *arg)
{
    sched_task_t *task = sched_task_current(arg);
    struct test_affined_wakeup_ctx *self = **(struct test_affined_wakeup_ctx ***)arg;

    self->need_to_sleep = false;

    size_t i;
    for (i = 0; atomic_load(&self->schedule_deschedule_cnt) < self->schedule_deschedule_cnt_max; ++i)
    {
        while (self->need_to_sleep)
            co_force_yield(task);

        if (self->running_flag)
        {
            printf("{\"task\": %4i, \"cpu\": %3u, \"state\": \"running\"},\n",
                   task->tid,
                   task->cpu->cpu_num);
            fflush(stdout);
            self->running_flag = false;
        }

        if (i % self->sleep_iteration_interval == 0)
            self->need_to_sleep = true;
        assert(task->cpu->cpu_num == self->affined_cpu);

        co_yield(task);
    }
    atomic_store(&self->task_finished, true);
    printf("{\"task\": %4i, \"cpu\": %3u, \"state\": \"finished\"},\n",
           task->tid,
           task->cpu->cpu_num);
    fflush(stdout);
}


void
test_wakeup_manager(void *arg)
{
    sched_task_t *task = sched_task_current(arg);
    struct test_wakeup_manager_ctx *self = *(struct test_wakeup_manager_ctx **)arg;

    bool has_running = true;
    while (has_running)
    {
        unsigned int j;
        fflush(stdout);
        has_running = false;
        for (j = 0; j < self->ntasks; ++j)
        {
            struct test_affined_wakeup_ctx *p = &(self->tasks_for_wakeup[j]);
            if (atomic_load(&p->task_finished))
                continue;

            has_running = true;
            if (atomic_load(&p->need_to_sleep))
            {
                printf("{\"task\": %4i, \"cpu\": %3u, \"state\": \"scheduled\"},\n",
                       p->self_ctx->tid, p->affined_cpu);

                fflush(stdout);
                sched_task_suspend(p->self_ctx);
                assert(!sched_task_state_queued(p->self_ctx) ||
                       (sched_task_state_onwakeup(p->self_ctx)
                        && sched_task_state_suspended(p->self_ctx)));
                atomic_store(&p->need_to_sleep, false);
                p->suspend_pass++;
            }
            else if (p->suspend_pass)
            {
                p->suspend_pass++;
                if (p->suspend_pass > self->suspend_pass_max)
                {
                    p->suspend_pass = 0;
                    atomic_fetch_add_explicit(&p->schedule_deschedule_cnt, 1,
                                              memory_order_relaxed);
                    printf("{\"task\": %4i, \"cpu\": %3u, \"state\": \"descheduled\"},\n",
                           p->self_ctx->tid, p->affined_cpu);

                    fflush(stdout);
                    p->running_flag = true;
                    sched_task_wakeup(p->self_ctx);
                }
            }
        }
        co_yield(task);
    }

    sched_online_set(task->cpu->shared, 0);
    printf("{\"task\": %4i, \"cpu\": %3u, \"state\": \"test_wakeup_manager finished\"},\n",
           task->tid,
           task->cpu->cpu_num);
}


static void *
test_schedule_cpu_loop(void *cpu_sched_rq)
{
    struct sched_rq *rq = (struct sched_rq *) cpu_sched_rq;
    sched_cpu_set_atomic(rq->cpu_num, rq->shared->cpu_online_mask);
    platform_thread_affine_cpu(&rq->work_cpu_ctx, rq->affined_cpu);

    sched_run_current(rq);

    while (sched_online(rq->shared))
    {
        schedule(rq);
        sched_run_current(rq);
    }
    return NULL;
}


int
sched_main(struct scheduler *sched, int argc, char *argv[])
{
    struct test_affinity_wakeup_config_ctx cfg_ctx = test_affinity_wakeup_config(argc, argv);

    sched_task_t **tasks =
            malloc(sizeof(sched_task_t*) * (cfg_ctx.cfg_shared.tasks_count + 1));
    struct test_affined_wakeup_ctx *pool_ctx =
            malloc(sizeof(struct test_affined_wakeup_ctx) * cfg_ctx.cfg_shared.tasks_count);

    struct test_wakeup_manager_ctx *wakeup_manager_ctx = NULL;

    (void)wakeup_manager_ctx;

    unsigned int i;

    size_t cpuset_size = SCHED_CPUMASK_BYTES(cfg_ctx.cfg_shared.wt_cnt);

    for (i = 0; i < cfg_ctx.cfg_shared.tasks_count; ++i)
    {
        int work_thread_num = i % cfg_ctx.cfg_shared.wt_cnt;

        struct test_affined_wakeup_ctx ctx =
        {
            /* self_ctx                    = */ NULL,
            /* affined_cpu                 = */ work_thread_num,
            /* suspend_pass                = */ 0,
            /* sleep_iteration_interval    = */ cfg_ctx.task_sleep_interval,
            /* schedule_deschedule_cnt_max = */ cfg_ctx.sleep_wakeup_per_task,
            /* schedule_deschedule_cnt     = */ 0,
            /* need_to_sleep               = */ false,
            /* task_finished               = */ false,
            /* running_flag                = */ false,
            /* pad                         = */ false
        };
        struct test_affined_wakeup_ctx *ctx_ptr;
        pool_ctx[i] = ctx;
        ctx_ptr = &pool_ctx[i];


        tasks[i] = sched_task_create_attached(&sched->runqueues[work_thread_num],
                                              test_affined_wakeup,
                                              &ctx_ptr, sizeof(ctx_ptr),
                                              CONFIG_SCHED_TASK_TESTS_STACK_SIZE_DEFAULT, NULL);
        pool_ctx[i].self_ctx = tasks[i];
    }

    {
        struct test_wakeup_manager_ctx ctx =
        {
            /* tasks_for_wakeup = */ pool_ctx,
            /* ntasks           = */ cfg_ctx.cfg_shared.tasks_count,
            /* suspend_pass_max = */ cfg_ctx.task_suspend_pass
        };
        i = cfg_ctx.cfg_shared.tasks_count;

        tasks[cfg_ctx.cfg_shared.tasks_count] =
                sched_task_create_attached(&sched->runqueues[0],
                                           test_wakeup_manager, &ctx, sizeof(ctx),
                                           CONFIG_SCHED_TASK_TESTS_STACK_SIZE_DEFAULT, NULL);
    }

    for (i = 0; i < cfg_ctx.cfg_shared.tasks_count; ++i)
    {
        unsigned int j;
        int work_thread_num = i % cfg_ctx.cfg_shared.wt_cnt;

        sched_cpu_set_chunck cpu_set[cpuset_size];
        for (j = 0; j < cpuset_size; ++j)
            cpu_set[j] = 0;
        sched_cpu_set(work_thread_num, cpu_set);


        sched_task_start_ex(&(sched->runqueues[work_thread_num]), tasks[i], SCHED_POLICY_NORMAL,
                            SCHED_PRIO_NORMAL, cpu_set, cpuset_size);
    }

    sched_task_start_ex(&(sched->runqueues[0]), tasks[cfg_ctx.cfg_shared.tasks_count],
                        SCHED_POLICY_NORMAL, SCHED_PRIO_NORMAL, NULL, 0);

    run_scheduling_on_cpus(sched, &test_schedule_cpu_loop);

    for (i = 0; i < cfg_ctx.cfg_shared.tasks_count + 1; ++i)
    {
        assert(sched_task_state_exited(tasks[i]));
        sched_task_destroy(tasks[i]);
    }

    fflush(stdout);

    free(pool_ctx);
    free(tasks);
    return 0;
}


int
main(int argc, char *argv[])
{
    struct test_affinity_wakeup_config_ctx cfg_ctx = test_affinity_wakeup_config(argc, argv);

    struct sched_application_config sched_app_cfg = SCHED_APPLICATION_CONFIG_INITIALIZER(
          /*rand_seed = */ 0, cfg_ctx.cfg_shared.wt_cnt, cfg_ctx.cfg_shared.timeslise_msec, NULL);

    return sched_application_run(argc, argv, sched_main, &sched_app_cfg);
}
