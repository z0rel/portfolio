#include <assert.h>
#include <stdio.h>
#include <stddef.h>

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include "scheduler/co_routine.h"
#include "find_shared.h"
#include "scheduler/cpumask_op.h"


void
test_run_fun_coro(void *arg);


void
test_run_fun_coro(void *arg)
{
    // TODO: определять task по смещению arg
    sched_task_t *task = sched_task_current(arg);
    struct test_case_find_stress_ctx *self = *(struct test_case_find_stress_ctx **)arg;
    double result = INT32_MIN;

    size_t i;

    if (self->yield_interval) // Тест количества итераций с YIELD_FORCE
    {
        size_t yield_iter_next;
        size_t yield_iter_max;
        size_t yield_iter_count;

        yield_iter_max = (self->array_len / self->yield_interval) + 1;

        i = 0;
        yield_iter_next = 0;
        for (yield_iter_count = 0; yield_iter_count < yield_iter_max; ++yield_iter_count)
        {
            yield_iter_next = yield_iter_count * self->yield_interval;
            for (; i < yield_iter_next; ++i)
                result += sqrt(self->test_array[i] * self->test_array[i + 1]);
            co_force_yield(task);
        }

        assert(yield_iter_next <= self->array_len);
        if (yield_iter_next < self->array_len)
        {
            for (; i < self->array_len; ++i)
                result += sqrt(self->test_array[i] * self->test_array[i + 1]);
            co_force_yield(task);
        }
    }
    else
    {
        if (self->array_len)
        {
            self->array_len--;
            for (i = 0; i < self->array_len; ++i)
            {
                result += sqrt(self->test_array[i] * self->test_array[i + 1]);
                co_yield(task);
            }
        }
    }
    *(self->result) = result;
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
    struct test_find_config_ctx cfg = test_find_config(argc, argv);

    sched_task_t *tasks[cfg.cfg_shared.tasks_count];
    double results[cfg.cfg_shared.tasks_count];

    unsigned int i;
    uint64_t cpu_freq, nsec_start, nsec_end;

    // struct test_run_fun_ctx pool_ctx[config_ctx.thread_pool_size];

    int32_t res = INT32_MIN;


    ssize_t chunck_arr_len;

    {
        int retcode;
        if ((retcode = calc_chunck_arr_len(&cfg, &chunck_arr_len)))
            return retcode;
    }

    int32_t *test_array = (int32_t *)malloc(cfg.cfg_array.array_size * sizeof(int32_t));
    if (init_test_array(&cfg.cfg_array, test_array))
        return -1;

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
    {
        struct sched_rq *rq = &sched->runqueues[i % cfg.cfg_shared.wt_cnt];

        results[i] = INT32_MIN;
        struct test_case_find_stress_ctx ctx = {
            /* test_array     = */ test_array + calc_offset_pos(chunck_arr_len, i, &cfg),
            /* array_len      = */ chunck_arr_len,
            /* yield_interval = */ cfg.yield_interval,
            /* result         = */ &(results[i])
        };

        tasks[i] = sched_task_create(rq, test_run_fun_coro,  &ctx, sizeof(ctx),
                                     /* stack size = */ 2 * 1024);
    }

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
        sched_task_start(&(sched->runqueues[i % cfg.cfg_shared.wt_cnt]), tasks[i]);


    cpu_freq = platform_monotonic_frequency();
    (void)cpu_freq;
    nsec_start = platform_monotonic_nsec(cpu_freq);

    run_scheduling_on_cpus(sched, &test_schedule_cpu_loop);

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
        if (results[i] > res)
            res = results[i];

    nsec_end = platform_monotonic_nsec(cpu_freq);


    free(test_array);
    test_find_print_results(res, nsec_start, nsec_end, &cfg.cfg_array, chunck_arr_len,
                            cfg.chunck_offset);
    return 0;
}


int
main(int argc, char *argv[])
{
    struct test_find_config_ctx cfg = test_find_config(argc, argv);
    struct sched_application_config sched_app_cfg =  SCHED_APPLICATION_CONFIG_INITIALIZER(
        cfg.cfg_array.random_seed, cfg.cfg_shared.wt_cnt, cfg.cfg_shared.timeslise_msec, NULL);

    return sched_application_run(argc, argv, sched_main, &sched_app_cfg);
}
