#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <time.h>

#include "scheduler_internal.h"
#include "scheduler/co_routine.h"
#include "scheduler/cpumask_op.h"
#include "scheduler/skip_list.h"
#include "scheduler/coro.h"
#include "sched_mutex_internal.h"
#include "platform/mt19937.h"

#ifdef __clang__
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wmissing-field-initializers"
#endif


/** Fast convert milliseconds to nanoseconds (in the timeslice calculation) */
//#define MS_TO_NS(TIME)            ((TIME) << 20)
#define MS_TO_100US_NOFAST(TIME)  ((TIME) * 1000)
#define NSEC_DIVISOR 1000


static inline void  update_system_monotonic_time(struct scheduler *self)
{
    uint64_t currtime = platform_monotonic_nsec((self)->monotonic_frequency);
    atomic_store_explicit(&self->time_start_offset, currtime, SCHED_TIME_SET_POLICY);
    self->time_shared = 0;
}



void
sched_wakeup_vector_extend(struct sched_wakeup_vector *container)
{
    size_t prev_size = container->max_size;
    if (unlikely(prev_size == 0))
    {
        assert(container->data == NULL);
        container->max_size = 8;
        container->data =
            (sched_task_t **)malloc(sizeof(sched_task_t *) * 8);
    }
    else
    {
        sched_task_t **prev_data = container->data;
        if (container->max_size < 128)
            container->max_size <<= 1;
        else
            container->max_size += 128;
        container->data =
            (sched_task_t **)malloc(sizeof(sched_task_t *)
                                                * container->max_size);
        memcpy(container->data, prev_data, prev_size * sizeof(sched_task_t *));
        free(prev_data);
    }
}


/**
 * Calculate the current priority, i.e. priority considered by the scheduler.
 */
static sched_prio_t
effective_prio(sched_task_t *p)
{
    p->normal_prio = normal_prio(p);
    /*
     * If the task is the realtime, keep the priority unchanged.
     * Otherwise, return the normal priority value.
     */
    if (TASK_PRIO_NOT_RT(p->prio))
        return p->normal_prio;
    return p->prio;
}


/**
 * Check whether p can be runned on the current CPU, and if isn't,
 * are there any online processors on which it can work instead.
 */
static inline bool
needs_other_cpu(sched_task_t *p, int cpu_num)
{
    if (unlikely(!sched_cpu_test_atomic(cpu_num, p->cpus_allowed)))
        return true;
    return false;
}


#define next_sibling_rq(rq) \
    ((rq)->sibling_round->next)


static inline struct sched_rq_round_list *
switch_next_sibling_rq(struct sched_rq *rq)
{
    struct sched_rq_round_list *curr = rq->sibling_round;
    struct sched_rq_round_list *next = curr->next;
    return next;
}


/**
 * Selecting tasks in the skip list is extracting the first task from
 * the sorted list, and it is O(1) operation.
 *
 * The task woken up long ago will have the earliest deadline and will be
 * ahead of others in the skiplist.
 *
 * Tasks are selected from the current runqueue and from the randomly selected neighbor.
 */
static inline sched_task_t *
earliest_deadline_task(struct sched_rq *self)
{
    struct sched_rq *locked_rq = NULL;
    sched_task_t *earliest_task = NULL;

    sched_skiplist_key_type_t best_key = SCHED_SKIPLIST_KEY_MAX;

    {
        const struct sched_skiplist_node *self_skiplist_head = &self->skiplist.head_node;
        const struct sched_skiplist_node *skiplist_item = self_skiplist_head->next[0];
        sched_skiplist_key_type_t k;

        if (likely(skiplist_item != self_skiplist_head
                   && (k = skiplist_item->key) < SCHED_TIME_MAX))
        {
            best_key = k;
            earliest_task = (sched_task_t *)(skiplist_item->value);
            assert(earliest_task != NULL);
            assert(sched_task_cpu(earliest_task) == self);
            if (sched_task_state_running_relaxed(earliest_task))
                earliest_task = NULL;
        }
    }

    do
    {
        struct sched_rq *other_rq = switch_next_sibling_rq(self)->rq;

        if (unlikely(other_rq == self
                     || !atomic_load_explicit(&other_rq->skiplist.entries, memory_order_relaxed)
                     || atomic_load_explicit(&other_rq->best_key, memory_order_relaxed)
                            >= best_key))
            continue;

        if (likely(platform_spin_trylock(&other_rq->lock, self->cpu_num)))
        {
            const struct sched_skiplist_node *other_rq_skiplist_head;
            const struct sched_skiplist_node *skiplist_item;

            if (unlikely(!other_rq->skiplist.entries || other_rq->best_key >= best_key))
            {
                platform_spin_unlock(&other_rq->lock);
                continue;
            }

            other_rq_skiplist_head = &other_rq->skiplist.head_node;
            skiplist_item = other_rq_skiplist_head;

            while ((skiplist_item = skiplist_item->next[0]) != other_rq_skiplist_head
                   && skiplist_item->key < best_key)
            {
                sched_task_t *p = (sched_task_t *)(skiplist_item->value);

                /* Ensure that affinity is valid */
                if (sched_task_state_running_relaxed(p) || needs_other_cpu(p, self->cpu_num))
                    continue;

                assert(sched_task_cpu(p) == other_rq);

                /* From this point, p is the best so far */
                assert(locked_rq == NULL);
                if (locked_rq)
                    platform_spin_unlock(&locked_rq->lock);
                locked_rq = other_rq;

                best_key = skiplist_item->key;
                earliest_task = p;
                assert(earliest_task != NULL);
                break;
            }

            if (other_rq != locked_rq)
                platform_spin_unlock(&other_rq->lock);
        }
    } while (0);

    if (likely(earliest_task != NULL))
    {
        platform_spin_lock(&earliest_task->pi_lock, self);

        /*
         * Remove the task from the runqueue and put it on the CPU,
         * for which it will become the running task
         */
        assert(!sched_task_state_running_relaxed(earliest_task));
        assert(sched_task_cpu(earliest_task) == self || sched_task_cpu(earliest_task) == locked_rq);
        dequeue_task(locked_rq ? locked_rq : self, earliest_task);

        if (sched_task_cpu(earliest_task) != self)
            assign_task_cpu(earliest_task, self);
        platform_spin_unlock(&earliest_task->pi_lock);
    }

    if (locked_rq)
        platform_spin_unlock(&locked_rq->lock);

    return earliest_task;
}


static inline void
schedule_next_task(struct sched_rq *self)
{
    sched_task_t *next;
    if (likely((next = earliest_deadline_task(self))))
    {
        set_current_rq_task(self, next);
        sched_task_state_running_set(next);
    }
}


static inline void
schedule_wakeup_tasks(struct sched_rq *self)
{
    size_t wakeup_size;

    wakeup_size = self->wakeup_tasks.size;
    if (unlikely(wakeup_size))
    {
        size_t j;
        sched_task_t **data = self->wakeup_tasks.data;

        for (j = 0; j < wakeup_size; ++j)
        {
            sched_task_t *task = data[j];
            platform_spin_lock(&task->pi_lock, self);
            assert(sched_task_state_suspended(task));
            assert(sched_task_state_onwakeup_relaxed(task));

            sched_task_state_suspended_clear(task);
            sched_task_state_onwakeup_clear_relaxed(task);
            sched_task_do_wakeup(self, task);
            platform_spin_unlock(&task->pi_lock);
        }
        self->wakeup_tasks.size = 0;
    }
}


void
schedule(struct sched_rq *self)
{
    sched_task_t *prev, *next;

    platform_spin_lock(&self->lock, self);

    update_clocks(self);
    schedule_wakeup_tasks(self);
    prev = self->curr;

    if (unlikely(prev == NULL))
    {
        /* Local rq time must be updated before return. */
        schedule_next_task(self);
        platform_spin_unlock(&self->lock);
        return;
    }
    else if (unlikely(sched_task_state_suspended(prev)))
    {
        /*
         * prev исполнялась на предыдущем шаге и не могла быть поставлена в очередь.
         * Если срабатывает данный assert - значит где-то разрушается память.
         */
        assert(!sched_task_state_queued(prev));
        if (unlikely(sched_task_state_onwakeup_relaxed(prev)))
        {
            sched_task_state_suspended_clear(prev);
            sched_task_state_onwakeup_clear(prev);
        }
        else
        {
            sched_task_state_running_clear(prev);
            sched_tasks_online_dec(self);
            set_current_rq_task(self, NULL);
            schedule_next_task(self);
            platform_spin_unlock(&self->lock);
            return;
        }
    }
    else if (!(sched_task_time_expired(prev, prev->time_slice)))
    {
        platform_spin_unlock(&self->lock);
        return;
    }

    assert(!sched_task_state_exited(self->curr));

    check_deadline(self, prev);

    if (unlikely(!next_sibling_rq(self)->rq->skiplist.entries))
    {
        switch_next_sibling_rq(self);
        if (unlikely(!self->skiplist.entries))
        {
            /* It is likely that the skiplist of the sibling runqueue is also empty.
             * If not, the task will be dequeued at the next schedule() call.
             */
            platform_spin_unlock(&self->lock);
            return;
        }
        /* else - sl_self_empty != false
         * Do nothing. Self skiplist is nonempty, and it is likely that the switched next sibling
         * runqueue skiplist is non empty.
         */
    } /* else: skiplist of the next sibling runqueue is nonempty */
    else if (unlikely(!self->skiplist.entries))
    {
        /* Self skiplist is empty.
         * It is likely that the tasks in the sibling runqueue skiplist is affined to it.
         * To minimize the amount of task migrations between runqueues, we will continue with
         * the probability = 1/32
         */
        if (!(rand_mt19937_rand_uint32(&self->random_gen) & 0x1f))
        {
            platform_spin_unlock(&self->lock);
            return;
        }
    } /* else: skiplist of the self runqueue is nonempty */

    enqueue_task(self, prev);

    if (unlikely(!(next = earliest_deadline_task(self))))
        dequeue_task(self, prev);
    else if (likely(prev != next))
    {
        set_current_rq_task(self, next);
        assert(!sched_task_state_running(next));
        sched_task_state_running_set(next);
        sched_task_state_running_clear(prev);
    }

    platform_spin_unlock(&self->lock);
}


void
sched_task_do_wakeup(struct sched_rq *cpu, sched_task_t *p)
{
    sched_tasks_online_inc(cpu);

    update_clocks(cpu);

    assert(cpu->curr != p && !sched_task_state_exited(p));
    if (cpu->curr == NULL)
    {
        set_current_rq_task(cpu, p);
        sched_task_state_running_set(p);
        time_slice_expired(cpu, p);
        update_last_ran(cpu, p);
        return;
    }

    sched_task_t *curr_task = cpu->curr;

    /* Priority boosting is not passed to the child */
    p->prio = curr_task->normal_prio;

    /* Reduce the timeslice of the current task to value of the elapsed time interval */
    update_task_timeslice_as_diff(cpu, curr_task);
    p->last_ran = curr_task->last_ran;

    /*
     * Divide the remaining time quantum of the current task in half with the created child.
     * Thus, creation of the new task will not increase the time remaining until the execution
     * of already existing tasks with the large deadline.
     */
    if (unlikely(curr_task->policy == SCHED_POLICY_FIFO)
        || (curr_task->time_slice >>= 1) < SCHED_TIME_RESCHED_NS)
        time_slice_expired(cpu, p);
    else
    {
        p->time_slice = curr_task->time_slice;
        /*
             * Equal deadlines are needed to insert the child immediately
             * after the current task in the skiplist
             */
        p->deadline = curr_task->deadline;
        p->last_ran_deadline = p->last_ran + p->time_slice;
    }

    p->prio = effective_prio(p);
    enqueue_task(cpu, p);
}


static void
sched_rq_step_scheduling_loop(struct sched_rq *rq)
{
    schedule(rq);
    sched_run_current(rq);
}


void
sched_destroy(struct scheduler *self)
{
    sched_cpu_num i;
    const unsigned int rq_cnt = self->cpu_count == 1 ? 2 : self->cpu_count;

    for (i = 0; i < rq_cnt; ++i)
    {
        struct sched_rq *local_cpu_rq = &self->runqueues[i];

        lpoll_close(local_cpu_rq->lpoll);
        free(local_cpu_rq->lpoll);
        if (local_cpu_rq->wakeup_tasks.data != NULL)
            free(local_cpu_rq->wakeup_tasks.data);
    }

    coro_stack_free(&self->tids_context.tids_stack);

    free(self->runqueues_round_list);
    free(self->runqueues);
    free(self->cpu_idle_map);
    free(self->cpu_online_mask);
}



int
sched_application_run(int argc, char *argv[], sched_main_t sched_main_fun,
                      struct sched_application_config *config)
{
    struct scheduler scheduler =
    { /* clang-format off */
        /* time_shared          = */ 0,
        /* coro_init_lock       = */ PLATFORM_SPINLOCK_INITIALIZER,
        /* time_start_offset    = */ 0,
        /* monotonic_frequency  = */ 0,
        /* runqueues            = */ NULL,
        /* runqueues_round_list = */ NULL,
        /* cpu_idle_map         = */ NULL,
        /* cpu_online_mask      = */ NULL,
        /* tids_context         = */ SCHED_TIDS_CONTEXT_INITIALIZER,
        /* cpu_count            = */ 0,
        /* tasks_online         = */ 0,
        /* tasks_all            = */ 0,
        /* timeslice_interval   = */ 6,
        /* mutex_max_lock_depth = */ 1024,
        /* online               = */ 0,
        /* timer_ctx            = */ PLATFORM_THREAD_INITIALIZER,

        /* prio_ratios          = */ { /* next = (11 / 10)*prev */
             128,  140,  154,  169,  185,  203,  223,  245,  269,  295,
             324,  356,  391,  430,  473,  520,  572,  629,  691,  760,
             836,  919, 1010, 1111, 1222, 1344, 1478, 1625, 1787, 1965,
            2161, 2377, 2614, 2875, 3162, 3478, 3825, 4207, 4627, 5089
        }
    }; /* clang-format on */

    int retcode;

    srand(config->random_seed);

//    vm_init_start();
//    vm_init_finish();
    lpoll_module_init();

    sched_init(&scheduler, config);

    retcode = (*sched_main_fun)(&scheduler, argc, argv);

    sched_destroy(&scheduler);

//    vm_deinit();
//    os_swap_close();

    return retcode;
}

static inline void
init_sched_rq_runqueues_round_list(struct sched_rq_round_list *self, struct sched_rq* rq,
                                   struct sched_rq_round_list *next)
{
    self->rq = rq;
    self->next = next;
}

void
sched_init(struct scheduler *self, struct sched_application_config *config)
{
    static const struct sched_rq intval_rq =
    { /* clang-format off */
        /* curr                  = */ NULL,
        /* shared                = */ NULL,
        /* wakeup_tasks          = */ SCHED_WAKEUP_VECTOR_INITIALIZER,
        /* best_key              = */ 0,
        /* time_local_rq         = */ 0,
        /* skiplist              = */ {0, 0, SCHED_SKIPLIST_NODE_INITIALIZER },
        /* coro_rq_ctx           = */ CORO_CONTEXT_INITIALIZER,
        /* work_cpu_ctx          = */ PLATFORM_THREAD_INITIALIZER,
        /* cpu_num               = */ 0,
        /* affined_cpu           = */ 0,
        /* rq_prio               = */ 0,
        /* lock                  = */ PLATFORM_SPINLOCK_INITIALIZER,
        /* mcs_lock_node         = */ PLATFORM_SPIN_MCS_INITIALIZER,
        /* expected_polls        = */ 0,
        /* errno_rq              = */ SCHED_EOK,
        /* sibling_round         = */ NULL,
        /* timers_queue          = */ SCHED_SKIPLIST_INITIALIZER,
        /* random_gen            = */ RAND_MT19937_INITIALIZER,
        /* random_gen_seed       = */ {{ 0, 0 }},
        /* step_scheduling_loop  = */ NULL,
        /* lpoll                 = */ NULL
        /* pad                   = */ PADDING_WARN_32_ITEM32_INITIALIZER
    }; /* clang-format on */

    sched_cpu_num i;
    sched_cpu_num j;
    sched_cpu_num cpu_max = config->runqueues_cnt;
    uint32_t timeslice_interval = config->timeslice_interval_msec;

    const unsigned int rq_cnt = cpu_max == 1 ? 2 : cpu_max;

    int sys_cpu_count = platform_cpu_count();

    (self)->runqueues = (struct sched_rq *)malloc(sizeof(struct sched_rq) * rq_cnt);

    (self)->monotonic_frequency = platform_monotonic_frequency();

    update_system_monotonic_time(self);

    self->timeslice_interval = MS_TO_100US_NOFAST(timeslice_interval);

    self->cpu_idle_map = (sched_atomic_cpu_set_t)malloc(sizeof(sched_cpu_set_chunck_atomic)
                                                           * SCHED_CPUMASK_BYTES(cpu_max));
    self->cpu_online_mask = (sched_atomic_cpu_set_t)malloc(sizeof(sched_cpu_set_chunck_atomic)
                                                              * SCHED_CPUMASK_BYTES(cpu_max));

    for (i = 0; i < SCHED_CPUMASK_BYTES(cpu_max); ++i)
    {
        self->cpu_idle_map[i] = 0;
        self->cpu_online_mask[i] = 0;
    }

    sched_online_set(self, 1);

    self->runqueues_round_list =
        (struct sched_rq_round_list *)malloc(sizeof(struct sched_rq_round_list) * cpu_max);


    for (j = 0; j < cpu_max - 1; ++j)
        init_sched_rq_runqueues_round_list(&(self->runqueues_round_list[j]),
                                           &(self->runqueues[j]),
                                           &(self->runqueues_round_list[j + 1]));

    init_sched_rq_runqueues_round_list(&(self->runqueues_round_list[cpu_max - 1]),
                                       &(self->runqueues[cpu_max - 1]),
                                       &(self->runqueues_round_list[0]));

    self->cpu_count = cpu_max;

    for (i = 0; i < rq_cnt; ++i)
    {
        struct sched_rq *local_cpu_rq = &self->runqueues[i];

        *local_cpu_rq = intval_rq;

        local_cpu_rq->shared      = self;
        local_cpu_rq->cpu_num     = i;
        local_cpu_rq->affined_cpu = i % sys_cpu_count;

        local_cpu_rq->step_scheduling_loop = sched_rq_step_scheduling_loop;

        sched_skiplist_init(&local_cpu_rq->skiplist);
        sched_skiplist_init(&local_cpu_rq->timers_queue);

        platform_spin_lock_init(&local_cpu_rq->lock);

        local_cpu_rq->time_local_rq = 0;
        local_cpu_rq->best_key = SCHED_SKIPLIST_KEY_MAX;

        if (i < cpu_max)
        {
            sched_cpu_set_atomic(local_cpu_rq->cpu_num, local_cpu_rq->shared->cpu_idle_map);
        }


        if (config->rq_randgen_seed != NULL)
            local_cpu_rq->random_gen_seed.v = config->rq_randgen_seed[i].v;
        else
        {
            local_cpu_rq->random_gen_seed.a[0] = platform_monotonic_nsec(self->monotonic_frequency)
                                                   & 0xFFFFFFFF;
            local_cpu_rq->random_gen_seed.a[1] = clock();
        }

        rand_mt19937_srand_arr(&local_cpu_rq->random_gen, local_cpu_rq->random_gen_seed.a,
                               SIZEOF_TABLE(local_cpu_rq->random_gen_seed.a));

        local_cpu_rq->lpoll = (lpoll_t *)(malloc(lpoll_self_size())); /* FIXME: malloc */
        lpoll_init(local_cpu_rq->lpoll, NULL, 0); /* FIXME: additional fd buffer can be not NULL */
    }

    for (j = 0; j < cpu_max; ++j)
    {
        struct sched_rq *rq = &(self->runqueues[j]);
        rq->sibling_round = self->runqueues_round_list[j].next;
    }
}



static void *
sched_time_count_thread_fun(void *arg)
{
    struct scheduler *self = (struct scheduler *)arg;
    sched_time_atomic_t *time_shared = &self->time_shared;
    atomic_uint_least64_t *time_start_offset  = &self->time_start_offset;

    uint64_t monotoinc_freq = self->monotonic_frequency;
    (void)monotoinc_freq;

    uint32_t prev = 0;
    (void)prev;
    while (sched_online(self))
    {
        uint64_t iter_time_nsec = platform_monotonic_nsec(monotoinc_freq);
        assert(iter_time_nsec >= *time_start_offset);
        uint32_t saved_val = (uint32_t)((iter_time_nsec - *time_start_offset) / NSEC_DIVISOR);
        assert(saved_val > prev);
        prev = saved_val;

        atomic_store_explicit(time_shared, saved_val, SCHED_TIME_SET_POLICY);
        platform_msleep(1);
    }
    return NULL;
}


static inline void
sched_timer_queue_expires(struct sched_rq *self)
{
    if (self->timers_queue.entries)
    {
        struct sched_skiplist_node *head = &self->timers_queue.head_node;
        struct sched_skiplist_node *item = head->next[0];

        for (; item != head && item->key < self->time_local_rq; item = head->next[0])
        {
            struct sched_timer *t = (struct sched_timer *)(item->value);

            sched_skiplist_delete(&self->timers_queue, item);

            t->expired = true;
            if (likely(t->action_expire != NULL))
                (*t->action_expire)(t);
        }
    }
}


void
sched_run_current(struct sched_rq *self)
{
    sched_task_t *p = self->curr;
    struct coro_context *current_coroutine;

    update_clocks(self);

    sched_timer_queue_expires(self);

    if (unlikely(p == NULL || (current_coroutine = sched_task_current_coroutine(p)) == NULL))
        return;

    assert(sched_task_state_running(p));

    sched_task_state_skip_next_suspend_clear(p);

    update_last_ran(self, p);

    /* Run coroutine */
    coro_transfer(&self->coro_rq_ctx, current_coroutine);

    if (!p->coro_exited)
        update_task_timeslice_as_diff(self, p); /* Local rq time updated in this call */
    else
    {
        sched_task_routine_pop(p);
        update_clocks_by_time(self, sched_time_get(self))
    }
}


void
run_scheduling_on_cpus(struct scheduler *self, schedule_cpu_loop_fun cpu_loop_fun)
{
    sched_cpu_num i;

    sched_online_set(self, 1);

    update_system_monotonic_time(self);
    platform_thread_create(&self->timer_ctx, sched_time_count_thread_fun, (void *)self);

    {
        sched_cpu_num cpu_count = self->cpu_count;
        for (i = 0; i < cpu_count; ++i)
            platform_thread_create(&self->runqueues[i].work_cpu_ctx,
                                   cpu_loop_fun,
                                   (void *)&self->runqueues[i]);

        for (i = 0; i < cpu_count; ++i)
            platform_thread_join(&self->runqueues[i].work_cpu_ctx);
    }

    sched_online_set(self, 0);
    platform_thread_join(&self->timer_ctx);
}


#ifdef __clang__
#pragma clang diagnostic pop
#endif
