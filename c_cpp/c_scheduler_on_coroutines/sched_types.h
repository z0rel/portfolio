/**
 * @file
 * @brief scheduler types: base types of the scheduler
 */

#include <stdint.h>
#include <stddef.h>

#include "platform/mt19937_types.h"

#ifndef SCHEDULER_TYPES_H
#define SCHEDULER_TYPES_H

#define SCHED_MAX_NICE 19


#define SCHED_MIN_NICE -20


#define SCHED_NICE_WIDTH (SCHED_MAX_NICE - SCHED_MIN_NICE + 1)


#if SCHED_NICE_WIDTH != 40
#    error SCHED_NICE_WIDTH != 40
#endif


#define SCHED_PRIO_MAX_USER_RT  100
#define SCHED_PRIO_MAX_RT       (SCHED_PRIO_MAX_USER_RT + 1)
#define SCHED_PRIO_ISO          (SCHED_PRIO_MAX_RT)
#define SCHED_PRIO_NORMAL       (SCHED_PRIO_MAX_RT + 1)
#define SCHED_PRIO_IDLE         (SCHED_PRIO_MAX_RT + 2)
#define SCHED_PRIO_LIMIT        ((SCHED_PRIO_IDLE) + 1)
#define SCHED_PRIO_MAX          (SCHED_PRIO_MAX_RT + SCHED_NICE_WIDTH)
#define SCHED_PRIO_USER(p)      ((p)->static_prio - SCHED_PRIO_MAX_RT)


/** Scheduling policy NORMAL */
#define SCHED_POLICY_NORMAL   0

/** Scheduling policy FIFO */
#define SCHED_POLICY_FIFO     1

/** Scheduling policy RR */
#define SCHED_POLICY_RR       2

/** Scheduling policy BATCH */
#define SCHED_POLICY_BATCH    3

/** Scheduling policy ISO */
#define SCHED_POLICY_ISO      4

/** Scheduling policy IDLE */
#define SCHED_POLICY_IDLE     5


#define SCHED_CPUFREQ_RT      (1U << 0)

#define SCHED_TASK_POLICY_BATCH(p) (unlikely((p)->policy == SCHED_POLICY_BATCH))

/** Reschedule if less than this number of nanoseconds is left */
#define SCHED_TIME_RESCHED_NS (0)

#define SCHED_BITS_PER_BYTE      8
#define SCHED_BITS_PER_SIZE_T    (SCHED_BITS_PER_BYTE * sizeof(size_t))
#define SCHED_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

/**
 * Получить число байт в битовой маске.
 * @param bit_len
 *     Число бит в битовой маске
 * @return
 *     Число байт в битовой маске.
 */
#define SCHED_CPUMASK_BYTES(bit_len)  SCHED_DIV_ROUND_UP((bit_len), SCHED_BITS_PER_SIZE_T)


/** Тип политики планирования */
typedef uint32_t sched_policy_t;


/** Тип нумерации рабочих нитей */
typedef uint32_t sched_cpu_num;


/** Тип приоритетов рабочей нити */
typedef int32_t  sched_prio_t;


/** Тип элемента битовой маски процессоров */
typedef size_t        sched_cpu_set_chunck;


/** Тип атомарного элемента битовой маски процессоров */
typedef atomic_size_t sched_cpu_set_chunck_atomic;


/** Тип атомарной битовой маски процессоров */
typedef sched_cpu_set_chunck_atomic * sched_atomic_cpu_set_t;


/** Тип битовой маски процессоров */
typedef sched_cpu_set_chunck        * sched_cpu_set_t;


/** Тип флага "онлайн" */
typedef atomic_uint_least32_t sched_online_flag_atomic_t;


/** Объявление стуктуры рабочей нити */
struct sched_rq;


/** Тип функции шага шедулинга */
typedef void (*sched_rq_step_scheduling_loop_fun_t)(struct sched_rq *rq);


/** Тип идентификатора задачи */
typedef uint32_t sched_tid_t;


/** Объявление типа структуры задачи */
struct sched_task_t;


/** Объявление типа структуры задачи */
typedef struct sched_task_t sched_task_t;


/** Объявление структуры планировщика */
struct scheduler;


/** Тип main-функции для первоначального создания базовых пользовательских корутин планировщика */
typedef int (*sched_main_t)(struct scheduler *sched, int argc, char *argv[]);


/**
 * Объединение для возможности отладочной настройки зерна сидирования генератора случайных чисел.
 * Такая настройка необходима для возможности воспроизвести выявленное поведение планировщика.
 */
union rand_gen_init_values
{
    rand_mt19937_val_t a[2];
    size_t v;
};


/** Конфигурация запуска планировщика */
struct sched_application_config
{
    /** Seed for the random generator (srand) */
    uint64_t random_seed;

    /** Число рабочих нитей */
    sched_cpu_num runqueues_cnt;

    /** Величина кванта времени в миллисекундах */
    uint32_t timeslice_interval_msec;

    /**
     * Отладочные значения зерен сидирования генератора случайных чисел планировщика, либо NULL
     * для рандомизированной инициализации.
     */
    union rand_gen_init_values *rq_randgen_seed;
};


#define SCHED_APPLICATION_CONFIG_INITIALIZER(rand_seed, rq_cnt, ts_interval, rq_rand_seed)         \
    { (rand_seed), (rq_cnt), (ts_interval), (rq_rand_seed)}


/**
 * @brief The sched_rq_round_list struct
 *     Кольцевой список рабочих нитей
 */
struct sched_rq_round_list
{
    /** Контекст текущей рабочей нити */
    struct sched_rq *rq;

    /** Контекст следующей рабочей нити */
    struct sched_rq_round_list *next;
};


/**
 * @brief The sched_wakeup_vector struct
 *     Очередь пробуждения в стиле вектора
 */
struct sched_wakeup_vector
{
    /** Текущий размер массива задач */
    size_t size;

    /** Массив задач, поставленных на пробуждение */
    sched_task_t **data;

    /** Максимальный размер массива задач */
    size_t max_size;
};


#define SCHED_WAKEUP_VECTOR_INITIALIZER { 0, NULL, 0 }


#endif // SCHEDULER_TYPES_H
