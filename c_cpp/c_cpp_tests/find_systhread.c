#include <stdio.h>

#include <memory.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>

#include "scheduler/co_routine.h"
#include "platform/platform.h"
#include "scheduler/tests/find_shared.h"


static void* test_run_fun(void *arg)
{
    struct test_case_find_stress_ctx *self = (struct test_case_find_stress_ctx *)arg;
    size_t i;
    double result = INT32_MIN;

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
        }

        assert(yield_iter_next <= self->array_len);
        if (yield_iter_next < self->array_len)
            for (; i < self->array_len; ++i)
                result += sqrt(self->test_array[i] * self->test_array[i + 1]);
    }
    else
    {
        if (self->array_len)
        {
            self->array_len--;
            for (i = 0; i < self->array_len; ++i)
                result += sqrt(self->test_array[i] * self->test_array[i + 1]);
        }
    }

    (*self->result) = result;
    return NULL;
}


int
main(int argc, char *argv[])
{
    struct test_find_config_ctx cfg = test_find_config(argc, argv);

    test_array_t test_array = (test_array_t)malloc(cfg.cfg_array.array_size * sizeof(int32_t));
    unsigned int i;
    int retcode;
    uint64_t cpu_freq, nsec_start, nsec_end;
    struct test_case_find_stress_ctx pool_ctx[cfg.cfg_shared.tasks_count];
    struct platform_thread threads[cfg.cfg_shared.tasks_count];
    double results[cfg.cfg_shared.tasks_count];

    int32_t res = INT32_MIN;
    ssize_t chunck_arr_len;

    if (init_test_array(&cfg.cfg_array, test_array))
        return -1;


    if ((retcode = calc_chunck_arr_len(&cfg, &chunck_arr_len)))
        return retcode;

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
    {
        pool_ctx[i].array_len = chunck_arr_len;
        pool_ctx[i].test_array = test_array + calc_offset_pos(chunck_arr_len, i, &cfg);
        pool_ctx[i].yield_interval = cfg.yield_interval;
        pool_ctx[i].result = &(results[i]);

    }

    cpu_freq = platform_monotonic_frequency();
    (void)cpu_freq;
    nsec_start = platform_monotonic_nsec(cpu_freq);

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
        platform_thread_create(&(threads[i]), test_run_fun, &(pool_ctx[i]));

    for (i = 0; i < cfg.cfg_shared.tasks_count; ++i)
    {
        platform_thread_join(&(threads[i]));
        if (results[i] > res)
            res = results[i];
    }

    nsec_end = platform_monotonic_nsec(cpu_freq);

    free(test_array);
    test_find_print_results(res, nsec_start, nsec_end, &cfg.cfg_array, chunck_arr_len,
                            cfg.chunck_offset);
    return 0;
}
