
#include <assert.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "scheduler/co_routine.h"
#include "scheduler/cpumask_op.h"


void
test_run_fun1(void *arg);


void
test_run_fun1(void *arg)
{
    (void)arg;
    printf("an: sucess\n");
}


void
test_run_fun(void *arg);


void
test_run_fun(void *arg)
{
    struct sched_task_t *task = sched_task_current(arg);

    printf("coro started\n");

    sched_task_t *t = sched_task_create_attached(task->cpu, test_run_fun1, NULL, 0,
                                                 /* stack size = */ 10 * 1024, task);
    sched_task_start(task->cpu, t);

    // TODO: to static inline co_waitfor function
    sched_task_waitfor(task, t);

    sched_task_destroy(t);
    printf("exited\n");
    fflush(stdout);
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
    const unsigned int thread_pool_size = 1;
    unsigned int i;
    uint64_t cpu_freq, nsec_start;
    sched_task_t *tasks[thread_pool_size];

    (void)argc;
    (void)argv;

    for (i = 0; i < thread_pool_size; ++i)
        tasks[i] = sched_task_create(&sched->runqueues[i % sched->cpu_count],
                                     test_run_fun,
                                     NULL,
                                     0,
                                     CONFIG_SCHED_TASK_TESTS_STACK_SIZE_DEFAULT);

    for (i = 0; i < thread_pool_size; ++i)
        sched_task_start(&(sched->runqueues[i % sched->cpu_count]), tasks[i]);

    cpu_freq = platform_monotonic_frequency();
    (void)cpu_freq;

    nsec_start = platform_monotonic_nsec(cpu_freq);
    printf("program started\n");
    fflush(stdout);

    run_scheduling_on_cpus(sched, &test_schedule_cpu_loop);

    printf("%"PRIu64, platform_monotonic_nsec(cpu_freq) - nsec_start);

    return 0;
}


int
main(int argc, char *argv[])
{
    (void)argc;
    (void)argv;

    const unsigned int wt_cnt = 1;
    const unsigned int timeslise_msec = 10;

    struct sched_application_config sched_app_cfg =  SCHED_APPLICATION_CONFIG_INITIALIZER(
        0, wt_cnt, timeslise_msec, NULL);

    return sched_application_run(argc, argv, sched_main, &sched_app_cfg);
}
