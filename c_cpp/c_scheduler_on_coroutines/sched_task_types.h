#ifndef SHED_TASK_H
#define SHED_TASK_H

#include <assert.h>

#include "scheduler/scheduler.h"
#include "scheduler/sched_rbtree_types.h"
#include "scheduler/skip_list.h"
#include "scheduler/coro_types.h"
#include "scheduler/co_routine_types.h"


#define TASK_STATE_SETWAKEUP (1 << 0)
#define TASK_STATE_SUSPENDED (1 << 1)
#define TASK_STATE_RUNNING   (1 << 2)
#define TASK_STATE_EXITED    (1 << 3)
#define TASK_STATE_ONWAKEUP  (1 << 4)


enum task_local_storage_key
{
    /* co ipc message */
    TASK_KEY_MSG      = 0,
    TASK_KEY_MSG_STAT = 1,
    TASK_KEY_MAX      = 2
};


/** Контекст состояния задачи */
struct sched_task_state_flags
{
    /** Задача выполняется в настоящий момент */
    atomic_bool running;

    /** Задача погружена в сон */
    atomic_bool suspended;

    /** На следующем шаге пропустить обработку пользовательского погружения задачи в сон,
     * а также однократно не перепланировать задачу, если из нее осуществлен выход по
     * co_force_yield.
     *
     * Флаг устанавливается вызовом wakeup и сбрасывается перед каждым исполнением задачи.
     *
     * Если данный флаг установлен:
     *   - не обнулять квант времени по FORCE_YIELD (приведет к тому, что если задача не исчерпала
     *   свой квант времени - она сразу будет исполнена заново, без возможной постановки в очередь)
     *
     * Если на выполняющейся задаче установлен данный флаг, а затем запрошено пользовательское
     * погружение задачи в сон (такое возможно например, в CO_WAITFOR если child завершился после
     * проверки exited родителем, но до выхода родителя):
     * - не обрабатывать погружнение задчи в сон в течение одного следующего шага (не относится к
     *   мьютексам). Это позволяет избежать бесконечного ожидания родительской корутиной.
     */
    atomic_bool skip_next_suspend;

    /** Задача находится в состоянии пробуждения */
    atomic_bool onwakeup;
};


/** Контекст состояния задачи для быстрого доступа */
union sched_task_state_t
{
    atomic_uint_least32_t state_int32;
    struct sched_task_state_flags flags;
};

#define SCHED_TASK_STATE_T_INITIALIZER { 0 }

static_assert(sizeof(struct sched_task_state_flags) == sizeof(uint32_t),
              "bad task_state union size");
static_assert(sizeof(union sched_task_state_t) == sizeof(uint32_t),
              "bad task_state union size");


typedef _Atomic(struct co_routise *) sched_task_co_routine_ptr_atomic;


typedef _Atomic(struct sched_rq *) sched_task_cpu_atomic;

typedef _Atomic(struct coro_context *) sched_task_current_coroutine_atomic;


/** The scheduler task context structure */
struct sched_task_t
{
    sched_task_current_coroutine_atomic current_coroutine;

    /**
     * Аргумент функции корутины, адрес которого передается ей при вызове.
     * Аргумент дублирует данные стека корутины для доступа к контексту задачи из пользовательской
     * функции
     */
    void *coro_arg;

    /** Программный стек для исполнения корутины */
    struct coro_stack   coro_stack;

    /** Признак завершения корутины */
    size_t coro_exited;

    /** trace parameters */
    uint64_t trace_module;
    uint64_t trace_level;

    /** Task priority */
    sched_prio_t prio;
    sched_prio_t static_prio;
    sched_prio_t normal_prio;
    sched_prio_t rt_priority;

    /** Политика планирования задачи */
    sched_policy_t policy;

    /** Идентификатор задачи */
    sched_tid_t tid;

    /** Квант времени задачи */
    sched_stime_t time_slice;

    /** Дедлайн задачи */
    sched_time_t deadline;

    /** Время последнего запуска задачи */
    sched_time_t last_ran;

    /** Время последнего запуска задачи + оставшийся квант времени */
    sched_time_t last_ran_deadline;

    /** Current CPU */
    sched_task_cpu_atomic cpu;

    /** skiplist node */
    struct sched_skiplist_node node;

    /** io node */
    struct sched_skiplist_node node_io;

    /** TODO */
    struct sched_task_error *error;

    /** Protection of the PI data structures: */
    platform_spinlock_t pi_lock;

    /** Число миграций задачи */
    uint32_t cpu_switch_cnt; // TODO: вынести в отладочную секцию

    /** Флаги состояния задачи */
    union sched_task_state_t state;

    /** Счетчик использований задачи в мьютексах */
    atomic_int_least32_t usage;

#if CONFIG_SCHED_USE_VALGRIND
    /** Идентификатор стека valgrind для профилирования */
    size_t valgrind_stack_id;
#endif

    /** TODO */
    _Atomic(sched_task_t *) join_to;

    /** task local storage */
    void *task_local_storage[TASK_KEY_MAX];

    /** PI waiters blocked on a sched_mutex held by this task */
    struct sched_rb_root  pi_waiters;
    struct sched_rb_node *pi_waiters_leftmost;

    /** Deadlock detection and priority inheritance handling */
    struct sched_mutex_waiter *pi_blocked_on;

    /** Маска допустимых для использования рабочих нитей */
    sched_cpu_set_chunck_atomic cpus_allowed[];
};

#if CONFIG_SCHED_USE_VALGRIND
#   define SCHED_TASK_VALGRIND_STACK_ID_INITIALIZER 1,
#else
#   define SCHED_TASK_VALGRIND_STACK_ID_INITIALIZER
#endif




#endif /* SHED_TASK_H */
