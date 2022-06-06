
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "scheduler/co_routine.h"
#include "scheduler/cpumask_op.h"
#include "platform/platform.h"


const unsigned int wt_cnt = 1;
const unsigned int thread_pool_size = 1;
const unsigned int timeslise_msec = 10;



struct co_main_ctx
{
    int argc;
    PADDING_WARN_64_ITEM32(pad);
    char** argv;
};


void
co_main(void *arg);


static inline void
test_run_fun(struct sched_task_t *task)
{
    int i;
    for (i = 0; i < 2; i++)
        co_force_yield(task);
}


void
co_main(void *arg)
{
    struct sched_task_t *task = sched_task_current(arg);
    struct co_main_ctx *self = *(struct co_main_ctx **)arg;
    (void)self;

    test_run_fun(task);
}


static void *
test_schedule_cpu_loop(void *cpu_sched_rq)
{
    struct sched_rq *rq = (struct sched_rq *) cpu_sched_rq;
    sched_cpu_set_atomic(rq->cpu_num, rq->shared->cpu_online_mask);
    platform_thread_affine_cpu(&rq->work_cpu_ctx, rq->affined_cpu);

    sched_run_current(rq);

    while (sched_tasks_all(rq->shared))
        rq->step_scheduling_loop(rq);

    return NULL;
}


int
sched_main(struct scheduler *sched, int argc, char *argv[]);


int
sched_main(struct scheduler *sched, int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    unsigned int i;
    uint64_t cpu_freq, nsec_start;

    for (i = 0; i < thread_pool_size; ++i)
    {
        struct co_main_ctx ctx = { argc PADDING_WARN_64_ITEM32_INITIALIZER, argv };
        sched_task_t *t = sched_task_create(&sched->runqueues[thread_pool_size % wt_cnt], co_main,
                                            &ctx, sizeof(ctx), 0);
        sched_task_start(&(sched->runqueues[i % wt_cnt]), t);
    }

    cpu_freq = platform_monotonic_frequency();
    (void)cpu_freq;

    nsec_start = platform_monotonic_nsec(cpu_freq);

    run_scheduling_on_cpus(sched, &test_schedule_cpu_loop);

    printf("%"PRIu64, platform_monotonic_nsec(cpu_freq) - nsec_start);
    return 0;
}


int
main(int argc, char *argv[])
{
    struct sched_application_config sched_app_cfg =  SCHED_APPLICATION_CONFIG_INITIALIZER(
        0, wt_cnt, timeslise_msec, NULL);

    return sched_application_run(argc, argv, sched_main, &sched_app_cfg);
}
