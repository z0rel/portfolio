/**
 * @file
 * @brief API планировщика корутин
 */
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdatomic.h>

#include "padding_warns.h"
#include "vmspace/vm_space.h"
#include "platform/platform.h"
#include "platform/spinlocks_mcs_types.h"
#include "platform/spinlocks.h"
#include "macro_va_args.h"
#include "trace.h"
#include "scheduler/skip_list_types.h"
#include "platform/mt19937_types.h"
#include "scheduler/sched_types.h"
#include "lpoll/lpoll.h"
#include "scheduler/coro_types.h"
#include "scheduler/sched_errno.h"

#define ARRAY_TYPE vm_stack_t
#define ARRAY_NAME(NAME) stack_array_##NAME
#include "vmspace/vm_array.h"


struct sched_task_t;

/**
 * Data structure for the runqueue of each processor.
 * Its data should only be modified by the local CPU.
 */
struct sched_rq
{
    /** The pointer to the current active task */
    struct sched_task_t *curr;

    /** Shared data of the scheduler */
    struct scheduler *shared;

    /** Очередь задач для пробуждения */
    struct sched_wakeup_vector wakeup_tasks;

    /** Best queued id for use outside the lock */
    sched_skiplist_key_type_atomic_t best_key;

    /** Current time */
    sched_time_t time_local_rq;

    struct sched_skiplist skiplist;

    /** Контекст для возврата в планировщик */
    struct coro_context coro_rq_ctx;

    /** Context of system working thread */
    struct platform_thread work_cpu_ctx;

    /** Runqueue number */
    sched_cpu_num cpu_num;

    /** Number of the affined CPU */
    int32_t affined_cpu;

    /** Runqueue priority */
    sched_prio_t rq_prio;

    /** Spin-lock to synchronize access of the runqueue */
    platform_spinlock_t lock;

    /** Оптимизированная спин-блокировка */
    struct platform_spin_mcs mcs_lock_node;

    /** TODO */
    atomic_uint_least32_t expected_polls;

    /** Код ошибки */
    _Atomic(enum sched_return_codes) errno_rq;

    /** Кольцевой список соседних очередей для баллансировочной миграции задач */
    struct sched_rq_round_list *sibling_round;

    /** Очередь таймеров */
    struct sched_skiplist timers_queue;

    /** Context of the random generator */
    struct rand_mt19937 random_gen;

    /** init random seed values */
    union rand_gen_init_values random_gen_seed;

    /** Указатель на функцию шага шедулинга */
    sched_rq_step_scheduling_loop_fun_t step_scheduling_loop;

    /** TODO */
    lpoll_t *lpoll;

    PADDING_WARN_32_ITEM32(pad) /* supress padding warn */
};


/** Контекст для получения tid */
struct sched_tids_context
{
    /** Спин-блокировка для доступа к stack_array */
    platform_spinlock_t  spin;

    /** Максимальный tid */
    uint32_t max_tid;

    /** Число свободных tid */
    size_t free_tids_cnt;

    /** Выделенный стек */
    sched_tid_t *tids;

    /** Стек свободных tid-ов */
    struct coro_stack tids_stack;
};

#define SCHED_TIDS_CONTEXT_INITIALIZER \
    { PLATFORM_SPINLOCK_INITIALIZER, 0, 0, NULL, CORO_STACK_INITIALIZER }


/** Main scheduler context struct */
struct scheduler
{
    /** Локальное системное время */
    sched_time_atomic_t time_shared;

    /** Спин-блокировка для инициализации coro корутины */
    platform_spinlock_t  coro_init_lock;

    /** Начальное смещение системного времени */
    atomic_uint_least64_t time_start_offset;

    /** Системная частота (Нужно в windows) */
    uint64_t monotonic_frequency;

    /** Массив контекстов рабочих нитей */
    struct sched_rq* runqueues;

    /** Кольцевой список рабочих нитей для баллансирования нагрузки между соседними нитями */
    struct sched_rq_round_list *runqueues_round_list;

    /** Маска бездействующих рабочих нитей */
    sched_atomic_cpu_set_t cpu_idle_map;

    /** Маска активных рабочих нитей */
    sched_atomic_cpu_set_t cpu_online_mask;

    /** Стек освободившихся идентификаторов задач */
    struct sched_tids_context tids_context;

    /** Worker threads count */
    sched_cpu_num cpu_count;

    /** Число активных задач. Т.е. число задач, не находящихся в состоянии сна. */
    atomic_int_least32_t tasks_online;

    /** Число незавершенных задач */
    atomic_int_least32_t tasks_all;

    /**
     * The time of all tasks with the same priority of the cyclic algorithm.
     * The value is set in milliseconds, and set to the minimum: 6 ms.
     */
    uint32_t timeslice_interval;

    /**
     * Max number of times we'll walk the boosting chain:
     * Максимальное число раз, когда мы пройдем по цепочке бустинга:
     */
    int_least32_t mutex_max_lock_depth; // TODO: сделать atomic

    /** Флаг "планировщик активен", т.е. признак того что планировщик не завершил свою работу */
    sched_online_flag_atomic_t online;

    /** Контекст системного потока для измерения времени */
    struct platform_thread timer_ctx;

    /** Relative length of the deadline for each priority level (nice) */
    int32_t prio_ratios[SCHED_NICE_WIDTH];

    PADDING_WARN_32_ITEM32(pad)
};


/** Type of pointer to execution loop function */
typedef void * (*schedule_cpu_loop_fun)(void *cpu_sched_rq);



/**
 * @brief sched_application_run
 *     Выполнить приложение на основе планировщика
 * @param argc
 *     Число аргументов командной строки
 * @param argv
 *     Массив аргументов командной строки
 * @param sched_main_fun
 *     Главная функция для запуска рабочих нитей и ожидания завершения.
 * @param config
 *     Конфигурация планировщика
 * @return
 *     Код возврата от sched_main_fun
 */
int
sched_application_run(int argc, char *argv[], sched_main_t sched_main_fun,
                      struct sched_application_config *config);



/**
 * @brief sched_init
 *    Initialize the general context of the scheduler
 * @param self
 *    Контекст планировщика
 * @param cpu_max
 *    Число рабочих нитей
 * @param timeslice_interval
 *    Размер кванта времени в миллисекундах
 */
void
sched_init(struct scheduler *self, struct sched_application_config *config);


/**
 * @brief sched_run_current
 *     Выполнить шаг корутины и обработать изменение ее состояния
 * @param self
 *     Контекст рабочей нити
 */
void
sched_run_current(struct sched_rq *self);


/**
 * Perform schedulig iteration for the local runqueue
 * @param self
 *     Pointer to scheduler context
 */
void
schedule(struct sched_rq *self);


/**
 * @brief run_scheduling_on_cpus
 *     Запустить процесс планирования на рабочих нитях. Вызывать после инициализации контекстов.
 * @param scheduler
 *     Контекст планировщика.
 * @param cpu_loop_fun
 *     Функция, исполняющая системный поток рабочей нити.
 */
void
run_scheduling_on_cpus(struct scheduler *scheduler, schedule_cpu_loop_fun cpu_loop_fun);


/**
 * Destroy of the scheduler
 * @param self
 *     Pointer to scheduler context
 */
void
sched_destroy(struct scheduler *self);


#ifndef CONFIG_SCHED_PROFILE_HELLGRIND
#    define SCHED_TIME_GET_POLICY                 memory_order_acquire
#    define SCHED_TIME_GET_RELAXED_POLICY         memory_order_relaxed
#    define SCHED_TIME_SET_POLICY                 memory_order_release
#    define SCHED_ONLINE_GET_POLICY               memory_order_relaxed
#    define SCHED_ONLINE_SET_POLICY               memory_order_relaxed
#    define SCHED_TASK_CURRENT_GET_POLICY         memory_order_acquire
#    define SCHED_TASK_CURRENT_GET_RELAXED_POLICY memory_order_relaxed
#    define SCHED_TASK_CURRENT_SET_POLICY         memory_order_release
#    define SCHED_TASK_CPU_GET_POLICY             memory_order_relaxed
#    define SCHED_TASK_CPU_SET_POLICY             memory_order_relaxed
#    define SCHED_TASK_STATE_GET_POLICY           memory_order_acquire
#    define SCHED_TASK_STATE_GET_RELAXED_POLICY   memory_order_relaxed
#    define SCHED_TASK_STATE_SET_POLICY           memory_order_release
#    define SCHED_TASK_STATE_SET_RELAXED_POLICY   memory_order_relaxed
#else
#    define SCHED_TIME_GET_POLICY                 memory_order_seq_cst
#    define SCHED_TIME_GET_RELAXED_POLICY         memory_order_seq_cst
#    define SCHED_TIME_SET_POLICY                 memory_order_seq_cst
#    define SCHED_ONLINE_GET_POLICY               memory_order_seq_cst
#    define SCHED_ONLINE_SET_POLICY               memory_order_seq_cst
#    define SCHED_TASK_CURRENT_GET_POLICY         memory_order_seq_cst
#    define SCHED_TASK_CURRENT_GET_RELAXED_POLICY memory_order_seq_cst
#    define SCHED_TASK_CURRENT_SET_POLICY         memory_order_seq_cst
#    define SCHED_TASK_CPU_GET_POLICY             memory_order_seq_cst
#    define SCHED_TASK_CPU_SET_POLICY             memory_order_seq_cst
#    define SCHED_TASK_STATE_GET_POLICY           memory_order_seq_cst
#    define SCHED_TASK_STATE_GET_RELAXED_POLICY   memory_order_seq_cst
#    define SCHED_TASK_STATE_SET_POLICY           memory_order_seq_cst
#    define SCHED_TASK_STATE_SET_RELAXED_POLICY   memory_order_seq_cst
#endif


/**
 * Atomically get expected events counter
 * @param  rq  current runqueue
 * @return expected events count
 */
#define sched_expected_polls_get(rq) \
    atomic_load_explicit(&(rq)->expected_polls, memory_order_relaxed)


/**
 * Atomically increment expected events counter
 * @param  rq  current runqueue
 * @return expected events count
 */
#define sched_expected_polls_inc(rq, diff) \
    atomic_fetch_add_explicit(&(rq)->expected_polls, (diff), memory_order_relaxed)


/**
 * Atomically decrement expected events counter
 * @param  rq  current runqueue
 * @return expected events count
 */
#define sched_expected_polls_dec(rq, diff) \
    atomic_fetch_sub_explicit(&(rq)->expected_polls, (diff), memory_order_relaxed)

/**
 * Atomically get the shared scheduler context
 * @param  rq  current runqueue
 * @return shared scheduler context
 */
#define sched_shared(rq) \
    atomic_load_explicit(&(rq)->shared, memory_order_relaxed)


/**
 * Atomically get the current global time
 * @param  rq  current runqueue
 * @return current global time
 */
#define sched_time_get(rq) \
    atomic_load_explicit(&(rq)->shared->time_shared, SCHED_TIME_GET_POLICY)


/**
 *  Get of the scheduler "online" status
 *  @param  sched scheduler
 *  @return scheduler "online" status - true (online) or false (offline)
 */
#define sched_time_get_relaxed(rq) \
    atomic_load_explicit(&(rq)->shared->time_shared, SCHED_TIME_GET_RELAXED_POLICY)


/**
 *  Get of the scheduler "online" status
 *  @param  sched     shared scheduler context
 *  @return scheduler "online" status - true (online) or false (offline)
 */
#define sched_online(sched) \
    atomic_load_explicit(&(sched)->online, SCHED_ONLINE_GET_POLICY)


/**
 *  Change the scheduler "online" status
 *  @param sched shared scheduler context
 *  @param val   true (online) or false (offline)
 */
#define sched_online_set(sched, val) \
    atomic_store_explicit(&(sched)->online, (val), SCHED_ONLINE_SET_POLICY)


/**
 *  Get of the count of the all scheduled tasks
 *  @param  sched     shared scheduler context
 *  @return tasks count
 */
#define sched_tasks_all(sched) \
    atomic_load_explicit(&(sched)->tasks_all, SCHED_ONLINE_GET_POLICY)


/**
 *  Get of the count of the online scheduled tasks
 *  @param  sched     shared scheduler context
 *  @return tasks count
 */
#define sched_tasks_online(sched) \
    atomic_load_explicit(&(sched)->tasks_online, SCHED_ONLINE_GET_POLICY)


#endif /* SCHEDULER_H */
