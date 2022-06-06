/**
 * @file
 * @brief scheduler mutexes: blocking mutual exclusion locks with PI support
 */

#ifndef SCHED_MUTEX_H
#define SCHED_MUTEX_H


#include <stddef.h>

#include "scheduler/co_routine.h"
#include "scheduler/sched_mutex_types.h"
#include "scheduler/src/sched_mutex_debug.h"

#include "platform/spinlocks_ticket_types.h"



#ifdef CONFIG_SCHED_DEBUG_MUTEXES
#    define sched_mutex_init(mutex) sched_mutex_init_named(mutex, __func__)
#else
#    define sched_mutex_init(mutex) sched_mutex_init_named(mutex, NULL)
#endif


/**
 * @brief sched_mutex_init_named
 *     Инициализировать мьютекс в состоянии "разблокирован".
 *
 * @param lock
 *     Мьютекс для инициализации.
 * @param name
 *     Опциональное имя мьютекса.
 */
void
sched_mutex_init_named(struct sched_mutex *lock, const char *name);


/**
 * @brief sched_mutex_destroy
 *     Пометить мьютекс неиспользуемым. Данная функция помечает мьютекс неинициализорованным, и
 *     любое последующее использование мьютекса запрещено. Мьютекс должен быть заблокирован при
 *     вызове этой функции.
 *
 * @param lock
 *     Мьютекс для разрушения.
 */
void
sched_mutex_destroy(struct sched_mutex *lock);


/**
 * @brief sched_mutex_is_locked
 *     Is the mutex locked
 *
 * @param lock
 *     The mutex to be queried
 *
 * @return
 *     true if the mutex is locked, false if unlocked.
 */
static inline bool
sched_mutex_is_locked(struct sched_mutex *lock)
{
    return atomic_load_explicit(&lock->owner, memory_order_relaxed) != 0;
}


/**
 * @brief sched_mutex_trylock
 *     Попытаться заблокировать sched_mutex. Данная функция может быть вызвана только в контексте
 *     потока.
 * @param self
 *     Контекст очереди выполнения.
 * @param lock
 *     sched_mutex, который будет заблокирован.
 * @return
 *     Возвращает 1 при успехе и 0 при конкуренции.
 */
int
sched_mutex_trylock(struct sched_rq *self, struct sched_mutex *lock);


/**
 * @brief sched_mutex_lock
 *     Заблокировать sched_mutex.
 *
 * @param lock
 *     struct sched_mutex *, мьютекс, который будет заблокирован.
 * @param task
 *     Текущая задача
 *
 * @return
 */
int32_t
sched_mutex_lock(struct sched_mutex *lock, sched_task_t *task);



/**
 * @brief sched_mutex_timed_lock
 *     Заблокировать sched_mutex прерываемо. Структура таймаута, предоставляется вызывающим.
 * @param mtx
 *     sched_mutex для блокирования
 * @param task
 *     Текущая задача
 * @param timeout_nsec
 *     Значение таймаута
 * @return
 *     0 в случае успеха, либо -ETIMEDOUT при истечении таймаута.
 */
int32_t
sched_mutex_timed_lock(struct sched_mutex *lock, sched_task_t *task, sched_time_t timeout_nsec);



/**
 * @brief sched_mutex_timed_futex_lock
 *     Вариант фьютекса с полным обнаружением взаимоблокировки.
 * @param mtx
 *     sched_mutex для блокирования.
 * @param task
 *     Текущая задача
 * @param timeout_nsec
 *     Значение таймаута
 * @return
 *     0 в случае успеха, либо -ETIMEDOUT при истечении таймаута.
 */
int32_t
sched_mutex_timed_futex_lock(struct sched_mutex *lock, sched_task_t *task,
                             sched_time_t timeout_nsec);



/**
 * @brief sched_mutex_unlock
 *     Разблокировать sched_mutex.
 * @param self
 *     Контекст очереди выполнения.
 * @param lock
 *     Мьютекс, который будет разблокирован.
 */
void
sched_mutex_unlock(struct sched_rq *self, struct sched_mutex *lock);


/**
 * @brief sched_mutex_futex_unlock
 *     Вариант фьютека для sched_mutex_unlock
 *
 * @param self
 *     Контекст очереди выполнения.
 *
 * @param lock
 *     Мьютекс для разблокирования.
 *
 * @param wqh
 * @return  true/false, указывающие, требует ли приоритет коррекции или нет.
 */
bool
sched_mutex_futex_unlock(struct sched_rq *self, struct sched_mutex *lock,
                         struct sched_mutex_wake_q_head *wqh);


/*
 * Вызывается sched_setscheduler() для получения приоритета, который будет эффективным после
 * изменения.
 */
int
sched_mutex_get_effective_prio(sched_task_t *task, int newprio);


/*
 * Вычислить приоритет задачи из приоритета дерева ожидающего.
 *
 * Вернуть task->normal_prio, когда дерево ожидающего пусто или когда ожидающему не разрешено
 * выполнять повышение приоритетов.
 */
sched_prio_t
sched_mutex_getprio(sched_task_t *task);


/**
 * @brief sched_mutex_setprio
 *     Установить текущий приоритет задачи.
 *     Эта функция изменяет 'эффективный' приоритет задачи. Она не трогает ->normal_prio как
 *     __setscheduler().
 *     Используется по коду sched_mutex для реализации логики наследования приоритета.
 *     Вызов необходим, только если приоритет задачи изменяется.
 *
 * @param p
 *     Указатель на структуру задачи
 * @param prio
 *     Значение приоритета (внутриядерная форма).
 */
void
sched_mutex_setprio(struct sched_rq *self, sched_task_t *p, sched_prio_t prio);


/*
 * Перепроверить цепочку pi, в случае если мы получили установку приоритетов.
 * Вызывается из sched_setscheduler
 */
void
sched_mutex_adjust_pi(struct sched_rq *self, sched_task_t *task);


/**
 * @brief sched_mutex_internal_suspend_task
 *     Снять с планирования задачу, которая будет ждать мьютекс
 * @param task
 *     Задача, которую нужно будет снять с планирования
 * @param waiter
 *     Контекст ожидания для обнаружения deadlocks
 */
void
sched_mutex_internal_deschedule_current(struct sched_rq *self, sched_task_t *task,
                                        struct sched_mutex_waiter *waiter);


/**
 * @brief sched_mutex_internal_slowlock
 *     Функция блокирования медленным способом.
 * @param self
 *     Контекст очереди выполнения
 * @param lock
 *     Мьютекс
 * @param timeout
 *     Контекст таймаута
 * @param chwalk
 *     Способ обхода цепочки повышения приоритетов
 * @param waiter
 *     Контекст ожидания мьютекса
 * @param timeout_val
 *     Значение таймаута (в наносекундах TODO)
 * @return
 *     0, если мьютекс захвачен, 1 - если нет и в waiter->retcode сохранена причина невозможности
 *     захвата мьютекса
 */
int
sched_mutex_internal_slowlock(struct sched_rq *self, struct sched_mutex *lock,
                              struct sched_timer *timeout, enum sched_mutex_chainwalk chwalk,
                              struct sched_mutex_waiter *waiter, sched_time_t timeout_val);

/**
 * @brief sched_mutex_internal_slowlock_tail
 *     Функция завершения блокирования медленным способом.
 * @param self
 *     Контекст очереди выполнения
 * @param lock
 *     Мьютекс
 * @param timeout
 *     Контекст таймаута
 * @param waiter
 *     Контекст ожидания мьютекса
 */
void
sched_mutex_internal_slowlock_tail(struct sched_rq *self, struct sched_mutex *lock,
                                   struct sched_timer *timeout, struct sched_mutex_waiter *waiter);


/**
 * @brief sched_mutex_internal_try_to_take
 *     Попытаться захватить sched_mutex. Должна быть вызвана с удерживаемой lock->wait_lock и
 *     отключенными прерываниями.
 *
 * @param lock
 *     Замок, который будет приобретен.
 * @param task
 *     Задача, которая хочет получить блокировку.
 * @param waiter
 *     Ожидающий, поставленный в очередь wait tree блокировки, если callsite вызывает
 *     task_blocked_on_lock(), иначе NULL.
 * @return
 *     true или false
 */
bool
sched_mutex_internal_try_to_take(struct sched_rq *self, struct sched_mutex *lock,
                                 sched_task_t *task,
                                 struct sched_mutex_waiter *waiter);


/* Мы можем ускорить получение/освобождение, если нет состояния отладки */
#ifndef CONFIG_SCHED_DEBUG_MUTEXES

#define sched_mutex_cmpxchg_acquire(l, c, n)                                                       \
    (atomic_compare_exchange_strong_explicit(                                                      \
        &(l)->owner, (size_t*)(&c), (size_t)(n), memory_order_acquire, memory_order_relaxed))


#define sched_mutex_cmpxchg_release(l, c, n)                                                       \
    (atomic_compare_exchange_strong_explicit(                                                      \
        &(l)->owner, (size_t*)(&c), (size_t)(n), memory_order_release, memory_order_relaxed))


static inline int
sched_mutex_debug_check_no_locks_freed(const void *from, unsigned long len)
{
    (void)from;
    (void)len;
    return 0;
}


#define sched_mutex_debug_check_no_locks_held(task) do { } while (0)


#define sched_mutex_debug_task_free(t) do { } while (0)


/*
 * Вызывающие должны удерживать ->wait_lock -- которая является всей целью, так как мы
 * заставляем все будущие потоки, пытающиеся [Rmw] блокировку, идти по медленному пути.
 * Как таковой, расслабленной семантики достаточно.
 */
static inline void
sched_mutex_internal_mark_waiters(struct sched_mutex *lock)
{
    atomic_store_explicit(&lock->has_waiters, true, memory_order_release);
    //    size_t owner;

    //    do
    //    {
    //        owner = lock->owner;
    //    } while (
    //        atomic_compare_exchange_weak_explicit(
    //                 &lock->owner, &owner, owner | SCHED_MUTEX_HAS_WAITERS,
    //                 memory_order_acquire, memory_order_relaxed) != owner);
}


/*
 * Безопасный быстрый путь известной разблокировки:
 * 1) Очистить бит waiters.
 * 2) Уничтожить lock->wait_lock
 * 3) Попытаться разблокировать блокировку с помощью cmpxchg
 */
static inline bool
sched_mutex_internal_unlock_safe(struct sched_mutex *lock)
{
    sched_task_t *owner = sched_mutex_owner(lock);

    clear_sched_mutex_waiters(lock);
    platform_spin_unlock(&lock->wait_lock);
    /*
     * Если между разблокировкой и cmpxchg появляется новый ожидающий, мы имеем 2 ситуации:
     *
     * unlock(wait_lock);
     *                    lock(wait_lock);
     * cmpxchg(p, owner, 0) == owner
     *                    mark_sched_mutex_waiters(lock);
     *                    acquire(lock);
     * Или:
     *
     * unlock(wait_lock);
     *                    lock(wait_lock);
     *                    mark_sched_mutex_waiters(lock);
     *
     * cmpxchg(p, owner, 0) != owner
     *                    enqueue_waiter();
     *                    unlock(wait_lock);
     * lock(wait_lock);
     * wake waiter();
     * unlock(wait_lock);
     *                    lock(wait_lock);
     *                    acquire(lock);
     */
    return sched_mutex_cmpxchg_release(lock, owner, NULL);
}


#else
// #    define sched_mutex_cmpxchg_relaxed(l, c, n) (0)


#define sched_mutex_cmpxchg_acquire(l, c, n) (0)
#define sched_mutex_cmpxchg_release(l, c, n) (0)


int
sched_mutex_debug_check_no_locks_freed(const void *from, unsigned long len);


void
sched_mutex_debug_check_no_locks_held(sched_task_t *task);


void
sched_mutex_debug_task_free(sched_task_t *tsk);


static inline void
sched_mutex_internal_mark_waiters(struct sched_mutex *lock)
{
    atomic_store_explicit(&lock->has_waiters, true, memory_order_release);
}


/* Простая версия с медленным путем: lock->owner защищена lock->wait_lock. */
static inline bool
sched_mutex_internal_unlock_safe(struct sched_mutex *lock)
{
    lock->owner = NULL;
    lock->has_waiters = false;
    lock->is_pending_owner = false;
    platform_spin_unlock(&lock->wait_lock);
    return true;
}


#endif


#endif // SCHED_MUTEX_H
