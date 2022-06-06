/**
 * @file
 * @brief MCS lock defines
 *
 * Этот файл содержит основные структуры данных и API определения MCS блокировок.
 *
 * Блокировка MCS (предложенная Mellor-Crummey и Scott) является простой
 * спин-блокировкой с желаемыми свойствами быть справедливой и для каждого
 * процессора, пытающегося получить блокировку, выполняет спининг на
 * локальной переменной.
 * Это позволяет избежать дорогостоящих отказов кэша, которым подвергнуты общие
 * реализации спин-блокировок вида "протестируй и установи".
 */
#ifndef PLATFORM_SPINLOCKS_MCS_H
#define PLATFORM_SPINLOCKS_MCS_H

#include <stddef.h>

#include "likely.h"
#include "platform/barrier.h"
#include "platform/spinlocks_mcs_types.h"


#if defined(PLATFORM_ARCH_ARM)

/* MCS spin-locking. */
#   define platform_mcs_spin_lock_contended(lock)                                                  \
    do                                                                                             \
    {                                                                                              \
        /* Ensure prior stores are observed before we enter wfe. */                                \
        platform_asm_smp_mb();                                                                     \
        while (!(atomic_load_explicit((lock), memory_order_acquire)))                              \
            platform_asm_wfe();                                                                    \
    } while (0)                                                                                    \


#   define platform_mcs_spin_unlock_contended(lock)                                                \
    do                                                                                             \
    {                                                                                              \
        atomic_store_explicit((lock), 1, memory_order_release);                                    \
        platform_asm_dsb(ishst);                                                                   \
        platform_asm_sev();                                                                        \
    } while (0)

#elif defined(PLATFORM_ARCH_X86_LIKE)

    /*
     * Замечание: пары atomic_load_excplicit(...memory_order_acquire)/
     *                 atomic_store_excplicit(...memory_order_release) недостаточно для
     * формирования полного барьера памяти между несколькими процессорами для
     * многих архитектур (исключая x86) для mcs_unlock и mcs_lock.
     * Для приложений, которым необходим полный барьер между несколькими
     * процессорами с парой mcs_unlock и mcs_lock, после mcs_lock нужно
     * использовать platform_asm_smp_mb() (см. barrier.h).
     */

    /*
     * Использование atomic_load_explicit(...memory_order_acquire) предоставляет memory barierr,
     * обеспечивающий выполнение последующих операций после приобретения блокировки
     */
#   define platform_mcs_spin_lock_contended(l)                                                     \
    do                                                                                             \
    {                                                                                              \
        while (!(atomic_load_explicit((l), memory_order_acquire)))                                 \
            platform_cpu_relax();                                                                  \
    } while (0)


    /* atomic_store_explicit(...memory_order_release) предоставляет memory barierr обеспечивающий
     * завершение всех операций в критической секции до разблокирования.
     */
#   define platform_mcs_spin_unlock_contended(l)                                                   \
        atomic_store_explicit((l), 1, memory_order_release)

#endif

/*
 * Чтобы получить блокировку, вызывающий обязан объявить локальный узел и передать в эту
 * функцию помимо самой блокировки, еще и ссылку узла. Если блокировка уже
 * была получена кем-то еще, то поток управления будет вращаться на этом
 * node->locked до тех пор, пока предыдущий владелец блокировки не установит
 * node->locked в mcs_spin_unlock().
 */
static inline void
platform_mcs_spin_lock(platform_spin_mcs_next_t *lock, struct platform_spin_mcs *node)
{
    struct platform_spin_mcs *prev;

    /* Init node */
    node->locked = 0;
    node->next   = NULL;

    /*
     * Мы полагаемся на полный барьер с глобальной транзитивностью,
     * подразумеваемый xchg() ниже, чтобы упорядочить инициализационное
     * сохранение перед любым наблюдением @node. А также для предоставления
     * упорядочивания ACQUIRE, связанного с примитивом LOCK.
     */
    prev = atomic_exchange_explicit(lock, node, memory_order_seq_cst);
    if (likely(prev == NULL))
    {
        /*
         * Полученная блокировка не должна устанавливать node->locked в 1.  Потоки
         * вращаются только на своем собственном значении node->locked для захвата
         * блокировки. Однако, поскольку этот поток может сразу захватить
         * блокировку и не переходить к вращению на своем node->locked, это
         * значение не будет использовано.  Если для проверки состояния блокировки
         * необходим отладочный режим, здесь следует установить значение
         * node->locked.
         */
        return;
    }
    atomic_store_explicit(&prev->next, node, memory_order_relaxed);

    /* Ждать, пока держатель блокировки не отпустит ее */
    platform_mcs_spin_lock_contended(&node->locked);
}


/*
 * Отпустить блокировку. Вызывающий должен передать соответствующий node,
 * который был использован для получения блокировки
 */
static inline void
platform_mcs_spin_unlock(platform_spin_mcs_next_t *lock, struct platform_spin_mcs *node)
{
    struct platform_spin_mcs *next = atomic_load_explicit(&node->next, memory_order_relaxed);

    if (likely(!next))
    {
        /* Отпустить блокировку, установив ее в NULL */
        struct platform_spin_mcs *p = node;
        if (likely(atomic_compare_exchange_strong_explicit(lock, &p, NULL,
                                                           memory_order_release,
                                                           memory_order_relaxed)))
            return;
        /* Ждать пока не установлен следующий указатель */
        while (!(next = atomic_load_explicit(&node->next, memory_order_relaxed)))
            platform_cpu_relax();
    }

    /* Передать блокировку следующему ожидающему */
    platform_mcs_spin_unlock_contended(&next->locked);
}


#endif // PLATFORM_SPINLOCKS_MCS_H
