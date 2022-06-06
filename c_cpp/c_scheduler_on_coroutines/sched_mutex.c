/**
 * @file
 * @brief scheduler mutexes: simple blocking mutual exclusion locks with PI support.
 */

#include <stdlib.h>

#include "sched_mutex_internal.h"
#include "scheduler/sched_timer.h"
#include "scheduler_internal.h"
#include "scheduler/src/scheduler_internal.h"


static inline void
sched_mutex_wakeup_task(struct sched_mutex_waiter *waiter)
{
    static size_t cnt = 0;
    struct sched_rq *rq = sched_task_cpu(waiter->task);
    sched_task_t *task = waiter->task;

    ++cnt;

    lock_rq_task(rq, task);
    assert(rq == sched_task_cpu(task));

    if (waiter->timeout != NULL)
        sched_timer_cancel(rq, waiter->timeout);

    if (likely(!sched_task_state_queued(task) && !sched_task_state_running(task)))
    {
        assert(!sched_task_state_queued(task));
        assert(task != rq->curr);
        assert(!sched_task_state_running(task));

        update_task_timeslice_as_diff(rq, task);
        check_deadline(rq, task);

        enqueue_task(rq, task);

        sched_tasks_online_inc(rq);
    }

    unlock_rq_task(rq, task);
}


static inline int32_t
sched_mutex_internal_lock_ex(sched_task_t *task, struct sched_mutex *lock,
                             enum sched_mutex_chainwalk policy,
                             struct sched_timer *timeout, sched_time_t timeout_nsec)
{
    struct sched_mutex_waiter waiter;
    if (sched_mutex_internal_slowlock(task->cpu, lock, timeout, policy, &waiter, timeout_nsec))
    {
        if (likely(!waiter.retcode))
        {
            waiter.retcode = 0;
            for (;;)
            {
                if (sched_mutex_internal_try_to_take(task->cpu, lock, task, &waiter))
                    break;
                platform_spin_unlock(&(lock)->wait_lock);
                sched_mutex_internal_deschedule_current(task->cpu, task, &waiter);
                co_force_yield(task);
                platform_spin_lock(&(lock)->wait_lock, task->cpu);

                if (unlikely((timeout) != NULL && waiter.retcode == -ETIMEDOUT))
                    break;
            }
        }
        sched_mutex_internal_slowlock_tail(task->cpu, lock, timeout, &waiter);
    }
    return waiter.retcode;
}


int32_t
sched_mutex_lock(struct sched_mutex *lock, sched_task_t *task)
{
    sched_task_t *_sched_mtx_lock_p = NULL;
    (void)_sched_mtx_lock_p;
    if (unlikely(!sched_mutex_cmpxchg_acquire((lock), _sched_mtx_lock_p, __task->cpu->curr)))
        return sched_mutex_internal_lock_ex(task, lock, SCHED_MUTEX_MIN_CHAINWALK, NULL, 0);

    return 0;
}


int32_t
sched_mutex_timed_lock(struct sched_mutex *lock, sched_task_t *task, sched_time_t timeout_nsec)
{
    sched_task_t *_sched_mtx_lock_p = NULL;
    (void)_sched_mtx_lock_p;
    struct sched_timer timeout;
    if ( unlikely(!sched_mutex_cmpxchg_acquire(lock, _sched_mtx_lock_p, __task->cpu->curr)) )
        return sched_mutex_internal_lock_ex(task, lock, SCHED_MUTEX_MIN_CHAINWALK, &timeout,
                                            timeout_nsec);
    return 0;
}


int32_t
sched_mutex_timed_futex_lock(struct sched_mutex *lock, sched_task_t *task,
                             sched_time_t timeout_nsec)
{
    struct sched_timer timeout;
    return sched_mutex_internal_lock_ex(task, lock, SCHED_MUTEX_FULL_CHAINWALK, &timeout,
                                        timeout_nsec);
}


void
sched_mutex_internal_deschedule_current(struct sched_rq *self, sched_task_t *task,
                                        struct sched_mutex_waiter *waiter)
{
    struct sched_rq *rq = sched_task_cpu(task);

    lock_rq_task(rq, task);

    assert(rq->curr == task);

    sched_task_state_running_clear(task);
    sched_tasks_online_dec(rq);
    set_current_rq_task(rq, NULL);

    unlock_rq_task(rq, task);

    debug_sched_mutex_print_deadlock(self, waiter);                                  \
}


static inline void
sched_mutex_upd_has_waiters(struct sched_mutex *lock)
{
    if (sched_mutex_has_waiters(lock))
        atomic_store_explicit(&lock->has_waiters, true, memory_order_release);
    else
        atomic_store_explicit(&lock->has_waiters, false, memory_order_release);
}


/*
 * отслеживание состояния lock->owner:
 *
 * lock->owner хранить указатель sched_task_t владельца. Бит 0
 * используется для отслеживания состояния "блокировка имеет ожидающих"
 *
 * Владелец bit0
 * NULL  0 блокировка свободна (возможно быстрое приобретение)
 * NULL  1 блокировка свободна и имеет ожидающих и главный ожидающий собирается захватить ее *
 *
 * taskpointer 0 блокировка удерживается (возможно быстрое освобождение)
 * taskpointer 1 блокировка удерживается и имеет ожидающих **
 *
 * Быстрый атомарный обмен по сравнению - основа приобритения и отпускания возможны
 * только когда бит 0 lock->owner = 0.
 *
 * Это также может быть переходным состоянием при захвате блокировки с удерживаемым
 * ->wait_lock. Для предотвращения любого быстрого пути cmpxchg для блокировки,
 * нужно установить бит 0 перед захватом блокировки, и в этот небольшой промежуток
 * времени владелец может быть NULL, поэтому это может быть переходным состоянием.
 *
 * Есть небольшой промежуток времени, когда bit 0 установлен, но нет ожидающих.
 * Это может происходить при захвате блокировки по медленному пути.
 * Для предотвращения освобождение владельцем блокировки через cmpxchg, нужно
 * установить этот бит перед просмотром блокировки.
 */
static void
sched_mutex_set_owner(struct sched_mutex *lock, sched_task_t *owner)
{
    sched_mutex_upd_has_waiters(lock);

    atomic_store_explicit(&lock->owner, owner, memory_order_relaxed);
}


static inline void
clear_sched_mutex_waiters(struct sched_mutex *lock)
{
    atomic_store_explicit(&lock->has_waiters, false, memory_order_release);
}


static void
fixup_sched_mutex_waiters(struct sched_mutex *lock)
{
    if (!sched_mutex_has_waiters(lock))
        clear_sched_mutex_waiters(lock);
}


static inline int
sched_mutex_waiter_less(struct sched_mutex_waiter *left, struct sched_mutex_waiter *right)
{

    if (left->prio < right->prio)
        return 1;

    return (left->task->deadline < right->task->deadline);


    /*
     * Если у обоих ожидающих есть dl_prio(), мы проверяем дедлайны связанных задач.
     *
     * Если левый ожидающий имеет dl_prio() и мы не возвратили 1 выше, то правый ожидающий
     * имеет dl_prio() также.
     *
     */
    //    if (dl_prio(left->prio))
    //        return dl_time_before(left->task->dl.deadline, right->task->dl.deadline);

    return 0;
}

static void
sched_mutex_enqueue(struct sched_mutex *lock, struct sched_mutex_waiter *waiter)
{
    struct sched_rb_node **link = &lock->waiters.sched_rb_node;
    struct sched_rb_node *parent = NULL;
    struct sched_mutex_waiter *entry;
    int leftmost = 1;

    while (*link)
    {
        parent = *link;
        entry = rb_entry(parent, struct sched_mutex_waiter, tree_entry);
        if (sched_mutex_waiter_less(waiter, entry))
        {
            link = &parent->rb_left;
        }
        else
        {
            link = &parent->rb_right;
            leftmost = 0;
        }
    }

    if (leftmost)
        lock->waiters_leftmost = &waiter->tree_entry;

    rb_link_node(&waiter->tree_entry, parent, link);
    rb_insert_color(&waiter->tree_entry, &lock->waiters);
}


static void
sched_mutex_dequeue(struct sched_mutex *lock, struct sched_mutex_waiter *waiter)
{
    if (RB_EMPTY_NODE(&waiter->tree_entry))
        return;

    if (lock->waiters_leftmost == &waiter->tree_entry)
        lock->waiters_leftmost = rb_next(&waiter->tree_entry);

    rb_erase(&waiter->tree_entry, &lock->waiters);
    RB_CLEAR_NODE(&waiter->tree_entry);
}


static void
sched_mutex_enqueue_pi(sched_task_t *task, struct sched_mutex_waiter *waiter)
{
    struct sched_rb_node **link = &task->pi_waiters.sched_rb_node;
    struct sched_rb_node *parent = NULL;
    struct sched_mutex_waiter *entry;
    int leftmost = 1;

    while (*link)
    {
        parent = *link;
        entry = rb_entry(parent, struct sched_mutex_waiter, pi_tree_entry);
        if (sched_mutex_waiter_less(waiter, entry))
        {
            link = &parent->rb_left;
        }
        else
        {
            link = &parent->rb_right;
            leftmost = 0;
        }
    }

    if (leftmost)
        task->pi_waiters_leftmost = &waiter->pi_tree_entry;

    rb_link_node(&waiter->pi_tree_entry, parent, link);
    rb_insert_color(&waiter->pi_tree_entry, &task->pi_waiters);
}


static void
sched_mutex_dequeue_pi(sched_task_t *task, struct sched_mutex_waiter *waiter)
{
    if (RB_EMPTY_NODE(&waiter->pi_tree_entry))
        return;

    if (task->pi_waiters_leftmost == &waiter->pi_tree_entry)
        task->pi_waiters_leftmost = rb_next(&waiter->pi_tree_entry);

    rb_erase(&waiter->pi_tree_entry, &task->pi_waiters);
    RB_CLEAR_NODE(&waiter->pi_tree_entry);
}


sched_prio_t
sched_mutex_getprio(sched_task_t *task)
{
    if (likely(!task_has_pi_waiters(task)))
        return task->normal_prio;

    return sched_min(task_top_pi_waiter(task)->prio, task->normal_prio);
}


sched_prio_t
sched_mutex_get_effective_prio(sched_task_t *task, sched_prio_t newprio)
{
    if (!task_has_pi_waiters(task))
        return newprio;

    if (task_top_pi_waiter(task)->task->prio <= newprio)
        return task_top_pi_waiter(task)->task->prio;
    return newprio;
}


/*
 * Изменить приоритет задачи после того, как ее pi_waiters были изменены.
 *
 * Это может быть как бустинг так и анбустинг. task->pi_lock должна удерживаться.
 */
static void
__sched_mutex_adjust_prio(struct sched_rq *self, sched_task_t *task)
{
    sched_prio_t prio = sched_mutex_getprio(task);

    if (task->prio != prio || dl_prio(prio))
        sched_mutex_setprio(self, task, prio);
}


/*
 * Скорректировать приоритет задачи (отменить бустинг). Вызывается из пути
 * выхода из sched_mutex_slowunlock() и sched_mutex_slowlock().
 *
 * (Замечание: мы делаем это за вне защиты lock->wait_lock, чтобы разрешить получение
 * блокировки во время и ли до того как мы скорректируем приоритет задачи.
 * Мы не используем здесь spin_xx_mutex(), так как мы находимся вне пути отладки.)
 */
static inline void
sched_mutex_adjust_prio(struct sched_rq *self, sched_task_t *task)
{
    platform_spin_lock(&task->pi_lock, self);
    __sched_mutex_adjust_prio(self, task);
    platform_spin_unlock(&task->pi_lock);
}


/*
 * Обнаружение взаимоблокировок зависит:
 *
 * Если CONFIG_SCHED_DEBUG_MUTEXES=n, обнаружение взаимоблокировок проводится только если аргумент
 * обнаружения это == SCHED_MUTEX_FULL_CHAINWALK.
 *
 * Если CONFIG_SCHED_DEBUG_MUTEXES=y, обнаружение взаимоблокировок выполняется всегда независимо от
 * аргумента обнаружения.
 *
 * Если аргумент ожидающего равен NULL, это означает что путь дебустинга и обнаружения
 * взаимоблокировок отключен независимо от аргумента обнаружения и настроек конфигурации.
 */
static bool
sched_mutex_cond_detect_deadlock(struct sched_mutex_waiter *waiter,
                                 enum sched_mutex_chainwalk chwalk)
{
   /*
    * Это просто функция-обертка для следующего вызова, поскольку
    * debug_sched_mutex_detect_deadlock() выглядит волшебной фичей отладки и я хотел бы сохранить
    * функцию cond в главном исходном файле вместо наличия двух одинаковых в заголовочных.
    */
    return debug_sched_mutex_detect_deadlock(waiter, chwalk);
}


static inline struct sched_mutex *
task_blocked_on_lock(sched_task_t *p)
{
    return p->pi_blocked_on ? p->pi_blocked_on->lock : NULL;
}


/**
 * @brief sched_mutex_adjust_prio_chain
 *   Скорректировать цепочку приоритетов. Также используется для обнаружения взаимоблокировок.
 *   Уменьшает использование задачи на единицу - может таким образом освободить задачу.
 *
 * Основы обхода цепочки и защищенная область
 *
 * [R] refcount на задаче
 * [P] task->pi_lock удерживается
 * [L] sched_mutex->wait_lock удерживается
 *
 *         Описание шага               Защищено по
 *    аргументы функции:
 *    @task         [R]
 *    @orig_lock    if != NULL      @top_task заблокирована на it
 *    @next_lock    Не защищено.    Не может быть разыменовано. Используется только для сравнения.
 *    @orig_waiter  if != NULL      @top_task является заблокированной на нем
 *    @top_task     current, или в случае прокси блокировки - защищено вызывающим кодом
 *    again:
 *      loop_sanity_check();
 *    retry:
 * [1]      lock(task->pi_lock);                         [R] acquire [P]
 * [2]      waiter = task->pi_blocked_on;                [P]
 * [3]      check_exit_conditions_1();                   [P]
 * [4]      lock = waiter->lock;                         [P]
 * [5]      if (!try_lock(lock->wait_lock)) {            [P] try to acquire [L]
 *        unlock(task->pi_lock);                 release [P]
 *        goto retry;
 *      }
 * [6]      check_exit_conditions_2();                   [P] + [L]
 * [7]      requeue_lock_waiter(lock, waiter);           [P] + [L]
 * [8]      unlock(task->pi_lock);               release [P]
 *      put_task_struct(task);                   release [R]
 * [9]      check_exit_conditions_3();                   [L]
 * [10]      task = owner(lock);                         [L]
 *      get_task_struct(task);                           [L] acquire [R]
 *      lock(task->pi_lock);                             [L] acquire [P]
 * [11]      requeue_pi_waiter(tsk, waiters(lock));      [P] + [L]
 * [12]      check_exit_conditions_4();                  [P] + [L]
 * [13]      unlock(task->pi_lock);              release [P]
 *      unlock(lock->wait_lock);                 release [L]
 *      goto again;
 *
 * @param task
 *    Задача, владеющая мьютексом (владельцем) для которого вероятно необходим цепочечный обход.
 *
 * @param chwalk
 *    Мы должны выполнять обнаружение взаимоблокировки?
 *
 * @param orig_lock
 *    Может быть NULL, если мы обходим цепочку для проверки вещей для задачи, которые
 *    только что получили корректировку своего приоритета и являются ожидающими на
 *    мьютексе).
 *
 * @param next_lock
 *    Мьютекс, на котором владелец @orig_lock был заблокирован перед тем, как мы
 *    удалили его pi_lock. Никогда не разыменовывается, используется только для
 *    сравнения обнаружения изменения цепочки блокировки.
 *
 * @param orig_waiter
 *    Структура sched_mutex_waiter для задачи, которая только что передала свой приоритет
 *    владельцу мьютекса  (может иметь значение NULL в описанном выше случае или если
 *    главный ожидающий ушел и мы фактически сделали дебустинг владельца).
 *
 * @param top_task
 *    Текущий главный ожидающий
 *
 * @return  Возвращает 0 или -EDEADLK.
 */
static void
sched_mutex_adjust_prio_chain(struct sched_rq *self,
                              sched_task_t *task,
                              enum sched_mutex_chainwalk chwalk,
                              struct sched_mutex *orig_lock,
                              struct sched_mutex *next_lock,
                              struct sched_mutex_waiter *orig_waiter,
                              sched_task_t *top_task)
{
    struct sched_mutex_waiter *waiter, *top_waiter = orig_waiter;
    struct sched_mutex_waiter *prerequeue_top_waiter;
    int depth = 0;
    struct sched_mutex *lock;
    bool detect_deadlock;
    bool requeue = true;
    orig_waiter->retcode = 0;

    detect_deadlock = sched_mutex_cond_detect_deadlock(orig_waiter, chwalk);

    /*
     * (Де)бустинг является пошаговым подходом с большим количеством подводных камней.
     * Мы хотим, чтобы это было вытесняемым и мы хотим удерживать максимум две блокировки за шаг.
     * Таким образом, мы должны тщательно проверить, изменяются ли вещи под нас.
     */

again:
    /* Мы ограничиваем длину цепи блокировки для каждого вызова. */
    if (++depth > sched_task_cpu(task)->shared->mutex_max_lock_depth)
    {
        put_task_struct(task);
        orig_waiter->retcode = -EDEADLK;
        return;
    }


    /*
     * Мы полностью выгружамы здесь и только удерживаем refcount на @task. Таким образом все может
     * измениться под нас, поскольку вызывающий или наш собственный код ниже (goto retry/again)
     * сбросили все блокировки.
     */
retry:
    /* [1] Задача не может уйти, поскольку мы сделали get_task() ранее. */
    platform_spin_lock(&task->pi_lock, self);

    /* [2] Получить ожидающего, на котором заблокирована @task. */
    waiter = task->pi_blocked_on;

    /* [3] check_exit_conditions_1() защищена с помощью task->pi_lock. */

    /*
     * Проверить, был ли достигнут конец повышающей цепочки или состояние цепочки изменилось, в то
     * время как мы отбросили блокировки.
     */
    if (!waiter)
        goto out_unlock_pi;

    /*
     * Проверить состояние orig_waiter. После того, как мы сбросили блокировки, предыдущий владелец
     * блокировки может освободить блокировку.
     */
    if (orig_waiter && !sched_mutex_owner(orig_lock))
        goto out_unlock_pi;

    /*
     * Мы полностью сбросили все блокировки после получения refcount на @task, поэтому задача может
     * переместиться на цепочке блокирования или даже полностью покинуть цепочку и блокироваться
     * теперь на несвязанной блокировке или на @orig_lock.
     *
     * Мы храним блокировку на которой @task была заблокирована в @next_lock,
     * поэтомы мы можем обнаружить изменение цепочки.
     */
    if (next_lock != waiter->lock)
        goto out_unlock_pi;

    /*
     * Сбросить, когда задача не имеет ожидающих.
     * Обратите внимание, что top_waiter может быть NULL, когда мы находимся в deboosting режиме!
     */
    if (top_waiter)
    {
        if (!task_has_pi_waiters(task))
            goto out_unlock_pi;
        /*
         * Если обнаружение взаимоблокировок отключено, мы останавливаемя здесь, если мы
         * не являемся главным pi ожидающим да задаче. Если обнаружение взаимоблокировки включено,
         * мы продолжаем, но останавливаем requeueing (переочередность) в обходе цепочки.
         *
         */
        if (top_waiter != task_top_pi_waiter(task))
        {
            if (!detect_deadlock)
                goto out_unlock_pi;
            else
                requeue = false;
        }
    }

    /*
     * Если приоритет ожидающего совпадает с приоритетом задачи, то нет необходимости
     * в дополнительной корректировке приоритета. Если обнаружение взаимоблокировок выключено,
     * мы останавливаем обход цепочки. Если оно включено, мы продолжаем, но останавливаем requeueing
     * в обходе цепочки.
     */
    if (waiter->prio == task->prio)
    {
        if (!detect_deadlock)
            goto out_unlock_pi;
        else
            requeue = false;
    }

    /*
     * [4] Получить следующую блокировку
     */
    lock = waiter->lock;
    /*
     * [5] Нам нужно выполнить здесь trylock, поскольку мы удерживаем task->pi_lock, что является
     * обратным порядком блокирования по сравнению с другими операциям sched_mutex.
     */
    if (!platform_spin_trylock(&lock->wait_lock, -1))
    {
        platform_spin_unlock(&task->pi_lock);
        platform_cpu_relax();
        goto retry;
    }

    /*
     * [6] check_exit_conditions_2() защищена с помощью task->pi_lock и lock->wait_lock.
     *
     * Обнаружение взаимоблокировок. Если блокировка та же, что и оригинальная блокировка,
     * которая заставила нас пройти цепочку блокирования, или если текущая блокировка принадлежит
     * задаче, которая инициировала обход цепи, то мы обнаружили взаимоблокировку.
     */
    if (lock == orig_lock || sched_mutex_owner(lock) == top_task)
    {
        debug_sched_mutex_deadlock(chwalk, orig_waiter, lock);
        platform_spin_unlock(&lock->wait_lock);
        orig_waiter->retcode = -EDEADLK;
        goto out_unlock_pi;
    }

    /*
     * Если мы просто следим за цепочкой блокировок для обнаружения взаимоблокировок,
     * не нужно выполнять все операции переочереди (requeue). Чтобы избежать кучу условных
     * операторов вокруг различных мест ниже, просто сделаем проврки минимального обхода цепочки.
     */
    if (!requeue)
    {
        /* Здесь нет requeue[7]. Просто отпустить @task [8] */
        platform_spin_unlock(&task->pi_lock);
        put_task_struct(task);

        /*
         * [9] check_exit_conditions_3 защищена с помощью lock->wait_lock.
         * Если нет владельца блокировки, завершить цепочку.
         */
        if (!sched_mutex_owner(lock))
        {
            platform_spin_unlock(&lock->wait_lock);
            orig_waiter->retcode = 0;
            return;
        }

        /* [10] Захватить следующую задачу, т.е. владельца @lock */
        task = sched_mutex_owner(lock);
        get_task_struct(task);
        platform_spin_lock(&task->pi_lock, self);

        /*
         * Здесь нет requeue [11]. Мы просто обнаруживаем взаимоблокировки.
         *
         * [12] Хранить, блокирует ли владелец сам себя. Решение принимается после снятия блокировок
         */
        next_lock = task_blocked_on_lock(task);
        /* Получение главного ожидающего для следующей итерации. */
        top_waiter = sched_mutex_top_waiter(lock);

        /* [13] Удалить блокировки */
        platform_spin_unlock(&task->pi_lock);
        platform_spin_unlock(&lock->wait_lock);

        /* Если владелец не заблокирован, завершить цепочку */
        if (!next_lock)
            goto out_put_task;
        goto again;
    }

    /*
     * Сохранить текущего главного ожидающего перед выполнением операции requeue на @lock. Это нужно
     * нам для бустинга/дебустинга решения ниже.
     */
    prerequeue_top_waiter = sched_mutex_top_waiter(lock);

    /* [7] Requeue ожидающего в дереве ожидающего блокировки  */
    sched_mutex_dequeue(lock, waiter);
    waiter->prio = task->prio;
    sched_mutex_enqueue(lock, waiter);

    /* [8] Отпустить задачу */
    platform_spin_unlock(&task->pi_lock);
    put_task_struct(task);

    /*
     * [9] check_exit_conditions_3 защищена с помощью lock->wait_lock.
     *
     * Мы должны прервать обход цепочки если там нет владельца блокировки, даже в случае обнаружения
     * взаимоблокировок, так как нам некуда здесь идти. Это конец цепи, которую мы обходим.
     */
    if (!sched_mutex_owner(lock))
    {
        /*
         * Если requeue [7] выше изменила главного ожидающего, то нужно разбудить нового главного
         * ожидающего и попытаться получить блокировку.
         */
        if (prerequeue_top_waiter != sched_mutex_top_waiter(lock))
            sched_mutex_wakeup_task(sched_mutex_top_waiter(lock));
        platform_spin_unlock(&lock->wait_lock);
        orig_waiter->retcode = 0;
        return;
    }

    /* [10] Захватить следующую задачу, т.е. владельца @lock */
    task = sched_mutex_owner(lock);
    get_task_struct(task);
    platform_spin_lock(&task->pi_lock, self);

    /* [11] requeue pi ожидающих, если это нужно */
    if (waiter == sched_mutex_top_waiter(lock))
    {
        /*
         * Ожидающий стал новым главным (наиболее приоритетным) ожидающим на блокировке. Заменить
         * предыдущего главного ожидающего в pi waiters tree задачи владельца на этого ожидающего
         * и скорректировать приоритет владельца.
         */
        sched_mutex_dequeue_pi(task, prerequeue_top_waiter);
        sched_mutex_enqueue_pi(task, waiter);
        __sched_mutex_adjust_prio(self, task);
    }
    else if (prerequeue_top_waiter == waiter)
    {
        /*
         * Ожидающий был главным ожидающим на блокировке, но он больше не является таким. Замените
         * ожидающего в pi waiters tree задачи владельца на нового главного (наиболее приоритетного)
         * ожидающего и скорректируйте приоритет владельца. Новый главный ожидающий хранится в
         * @waiter, так что @waiter == @top_waiter оценивается в истину ниже и мы продолжаем,
         * чтобы сделать дебустинг остатка цепочки.
         */
        sched_mutex_dequeue_pi(task, waiter);
        waiter = sched_mutex_top_waiter(lock);
        sched_mutex_enqueue_pi(task, waiter);
        __sched_mutex_adjust_prio(self, task);
    }

    /*
     * [12] check_exit_conditions_4() защищена по task->pi_lock и lock->wait_lock.
     *
     * Фактическое решение принимается после того как мы удалили блокировку.
     *
     * Проверка, является ли задача, которая владеет текущей блокировкой также
     * pi заблокированной сама по себе. Если да, то мы сохраняем
     * указатель на блокировку для обнаружения выше изменения цепи блокирования.
     * После того как мы удалили task->pi_lock, ссылка на next_lock не может быть разыменована.
     */
    next_lock = task_blocked_on_lock(task);
    /* Сохранить главного ожидающего @lock для решения о конце обхода цепочки ниже. */
    top_waiter = sched_mutex_top_waiter(lock);

    /* [13] Сбросить блокировки */
    platform_spin_unlock(&task->pi_lock);
    platform_spin_unlock(&lock->wait_lock);

    /*
     * Принять фактическое решение о выходе [12], основываясь на сохраненных значениях.
     *
     * Мы достигли конца цепочки блокирования. Остановиться прямо здесь. Нет
     * смысла возвращаться, только для того, чтобы понять это.
     */
    if (!next_lock)
        goto out_put_task;

    /*
     * Если текущий ожидающий не является главным ожидающим на блокировке,
     * то мы можем остановить здесь обход цепочки, если мы не находимся в режиме
     * обнаружения взаимоблокировок.
     */
    if (!detect_deadlock && waiter != top_waiter)
        goto out_put_task;

    goto again;

out_unlock_pi:
    platform_spin_unlock(&task->pi_lock);
out_put_task:
    put_task_struct(task);
}


bool
sched_mutex_internal_try_to_take(struct sched_rq *self, struct sched_mutex *lock,
                                 sched_task_t *task,
                                 struct sched_mutex_waiter *waiter)
{
    /*
     * Прежде чем проверить, можем ли мы получить @lock, мы устанавливаем бит
     * SCHED_MUTEX_HAS_WAITERS в @lock->owner. Это заставляет все другие задачи, которые пытаются
     * модифицировать @lock медленным способом - сериализоваться на @lock->wait_lock.
     *
     * Бит SCHED_MUTEX_HAS_WAITERS может иметь переходное состояние, как объясняется в верхней
     * части этого файла если и только если:
     *
     * Есть владелец блокировки. Вызывающий должен fixup переходного состояния, если он выполняет
     * trylock или покидает функцию блокировки из за сигнала или тайм-аута.
     *
     * - @task получает блокировку и нет никаких других ожидающих. Это будет отменено в
     * sched_mutex_set_owner(@task) в конце этой функции.
     */
    sched_mutex_internal_mark_waiters(lock);

    /*  Если @lock имеет владельца, отказаться. */
    if (sched_mutex_owner(lock) != NULL)
        return 0;

    /*
     * Если @waiter != NULL, @task уже поставил в очередь ожидающего в @lock waiter tree.
     * Если @waiter == NULL, то это является попыткой trylock.
     */
    if (waiter != NULL)
    {
        /* Если waiter не является ожидающим с наивысшим приоритетом для @lock, сдаться. */
        if (waiter != sched_mutex_top_waiter(lock))
            return 0;

        /* Мы можем получить блокировку. Удалить ожидающего из lock waiters tree. */
        sched_mutex_dequeue(lock, waiter);
    }
    else
    {
        /*
         * Если блокировка уже имеет ожидающих, мы проверяем, имеет ли @task
         * право захватить блокировку.
         *
         * Если нет других ожидающих, @task может получить блокировку.
         * @task->pi_blocked_on is NULL, поэтому не нужно снимать ее с очереди.
         */
        if (sched_mutex_has_waiters(lock))
        {
            /*
             * Если @task->prio больше либо равна приоритету главного ожидающего (просмотр в ядре),
             * @task потеряна.
             */
            if (task->prio >= sched_mutex_top_waiter(lock)->prio)
                return 0;

            /*
             * Текущий главный ожидающий остается в очереди. Мы не имеем изменения чего угодно в
             * порядке блокировки ожидающих.
             */
        }
        else
        {
            /*
             * Нет ожидающих. Взять блокировку без танцев с pi_lock. @task->pi_blocked_on равна NULL
             * и мы не имеем ожидающих для помещения в @task pi waiters tree.
             */
            goto takeit;
        }
    }

    /*
     * Очистим @task->pi_blocked_on. Требует защиты по @task->pi_lock.
     * Избыточная операция для случая @waiter == NULL, но условия
     * являются более дорогими, чем избыточное сохранение.
     */
    platform_spin_lock(&task->pi_lock, self);
    task->pi_blocked_on = NULL;
    /*
     * Завершим захват блокировки. @task является новым владельцем. Если существуют другие
     * ожидающие, мы должны вставить ожидающего с наибольшим приоритетом в дерево @task->pi_waiters.
     */
    if (sched_mutex_has_waiters(lock))
        sched_mutex_enqueue_pi(task, sched_mutex_top_waiter(lock));
    platform_spin_unlock(&task->pi_lock);

takeit:
    /* Мы получили блокировку. */
    debug_sched_mutex_lock(lock);

    /*
     * Это либо сохраняет бит SCHED_MUTEX_HAS_WAITERS, если там все еще есть ожидающие, либо очищает
     * его.
     */
    sched_mutex_set_owner(lock, task);

    return 1;
}


/**
 * @brief task_blocks_on_sched_mutex
 *      Блокирование задачи на блокировке. Подготовить ожидающего и распространить цепочку pi.
 *      Нужно вызывать с удерживаемой lock->wait_lock и отключенными прерываниями.
 * @param lock
 *     Мьютекс для блокирования.
 * @param waiter
 *     Структура ожидающего
 * @param task
 *     Задача, на которой требуется провести блокирование
 * @param chwalk
 *     Политика распространения цепочки ожидания
 * @return 0 или -EDEADLK.
 */
static void
task_blocks_on_sched_mutex(struct sched_rq *self,
                           struct sched_mutex *lock,
                           struct sched_mutex_waiter *waiter,
                           sched_task_t *task,
                           enum sched_mutex_chainwalk chwalk)
{
    sched_task_t *owner = sched_mutex_owner(lock);
    struct sched_mutex_waiter *top_waiter = waiter;
    struct sched_mutex *next_lock;
    int chain_walk = 0;

    /*
     * Раннее обнаружение взаимоблокировки. Мы на самом деле не хотим
     * поставить задачу в очередь самой к себе, чтобы потом распутывать этот беспорядок.
     * Это не только оптимизация. Мы отбрасывам блокировки, таким образом,
     * что другой ожидающий может прийти до того, как обход цепочки обнаружит
     * взаимоблокировку. Поэтому, другой будет обнаруживать взаимоблокировки
     * и вернет -EDEADLOCK, что неверно, так как другой ожидающий не находится
     * в ситуации взаимоблокировки.
     */
    if (owner == task)
    {
        waiter->retcode = -EDEADLK;
        return;
    }

    platform_spin_lock(&task->pi_lock, self);
    __sched_mutex_adjust_prio(self, task);
    waiter->prio = task->prio;

    /* Получить ожидающего с наибольшим приоритетом на блокировке */
    if (sched_mutex_has_waiters(lock))
        top_waiter = sched_mutex_top_waiter(lock);
    sched_mutex_enqueue(lock, waiter);

    task->pi_blocked_on = waiter;

    platform_spin_unlock(&task->pi_lock);

    if (!owner)
    {
        waiter->retcode = 0;
        return;
    }

    platform_spin_lock(&owner->pi_lock, self);
    if (waiter == sched_mutex_top_waiter(lock))
    {
        sched_mutex_dequeue_pi(owner, top_waiter);
        sched_mutex_enqueue_pi(owner, waiter);

        __sched_mutex_adjust_prio(self, owner);
        if (owner->pi_blocked_on)
            chain_walk = 1;
    }
    else if (sched_mutex_cond_detect_deadlock(waiter, chwalk))
        chain_walk = 1;

    /* Хранить блокировку на которой заблокирован владелец или NULL. */
    next_lock = task_blocked_on_lock(owner);

    platform_spin_unlock(&owner->pi_lock);
    /*
     * Даже если полное обнаружение взаимоблокировки включено, если владелец
     * не заблокирован сам себя, мы можем избежать обнаружения этого в
     * процессе обхода блокировки.
     */
    if (!chain_walk || !next_lock)
    {
        waiter->retcode = 0;
        return;
    }

    /*
     * Владелец не может исчезнуть, удерживая блокировку, поэтому
     * структура owner защищена wati_lock.
     *
     * Потеряется в sched_mutex_adjust_prio_chain()!
     */
    get_task_struct(owner);

    platform_spin_unlock(&lock->wait_lock);

    sched_mutex_adjust_prio_chain(self, owner, chwalk, lock, next_lock, waiter, task);

    platform_spin_lock(&lock->wait_lock, self);
}


/*
 * Удалить главного ожидающего из pi waiter tree текущей задачи и
 * поставить это в очередь.
 *
 * Вызывается с удерживаемым lock->wait_lock и отключенными прерываниями.
 */
static void
mark_wakeup_next_waiter(struct sched_rq *self,
                        struct sched_mutex_wake_q_head *wake_q,
                        struct sched_mutex *lock)
{
    struct sched_mutex_waiter *waiter;

    platform_spin_lock(&self->curr->pi_lock, self);

    waiter = sched_mutex_top_waiter(lock);

    /*
     * Удалить его из current->pi_waiters. Мы не корректируем
     * возможный бустинг приоритетов прямо сейчас. Мы выполняем
     * пробуждение в бустированном режиме и возвращаемся надад в нормальный
     * после того как отпустим lock->wait_lock.
     */
    sched_mutex_dequeue_pi(self->curr, waiter);

    /*
     * Когда мы разбудим главного ожидающего, и ожидающий останется
     * поставленным в очередь на блокировке до тех пор, пока он не получит ее,
     * блокировка очевидно имеет ожидающих.
     * Просто установите этот бит здесь, и это будет дополнительным преимуществом,
     * заставляя все новые задачи идти по медленному пути, удостоверяясь, что никаккая
     * задача более низкого приоритета чем главный ожидающий может украсть эту блокировку.
     */
    atomic_store_explicit(&lock->has_waiters, true, memory_order_release);
    lock->owner = NULL;
    lock->is_pending_owner = false;

    platform_spin_unlock(&self->curr->pi_lock);

    {
        struct sched_mutex_wake_q_node *node = &waiter->wake_q;
        struct sched_mutex_wake_q_node *p = NULL;
        /*
         * Атомарно захватить задачу. Если wake_q != NULL, то она уже поставлена в очередь.
         * Либо нами либо кем-то еще и будет разбужена в ходе этого.
         */
        if (atomic_compare_exchange_strong_explicit(&node->next, &p, WAKE_Q_TAIL,
                                                    memory_order_acquire, memory_order_relaxed))
        {
            get_task_struct(waiter->task);

            /* The head is context local, there can be no concurrency. */
            *wake_q->lastp = node;
            wake_q->lastp = &node->next;
        }
    }
}


/*
 * Удалить ожидающего из блокировки и отказаться.
 *
 * Должан быть вызвана с удерживаемой lock->wait_lock и отключенными прерываниями.
 */
static void
remove_waiter(struct sched_rq *self, struct sched_mutex *lock, struct sched_mutex_waiter *waiter)
{
    bool is_top_waiter = (waiter == sched_mutex_top_waiter(lock));
    sched_task_t *owner = sched_mutex_owner(lock);
    struct sched_mutex *next_lock;

    platform_spin_lock(&self->curr->pi_lock, self);
    sched_mutex_dequeue(lock, waiter);
    self->curr->pi_blocked_on = NULL;
    platform_spin_unlock(&self->curr->pi_lock);

    /*
     * Только обновить приоритет, если ожидающий был наиболее приоритетным
     * ожидающим блокировки и есть владелец для обновления.
     */
    if (!owner || !is_top_waiter)
        return;

    platform_spin_lock(&owner->pi_lock, self);

    sched_mutex_dequeue_pi(owner, waiter);

    if (sched_mutex_has_waiters(lock))
        sched_mutex_enqueue_pi(owner, sched_mutex_top_waiter(lock));

    __sched_mutex_adjust_prio(self, owner);

    /* Сохранить блокировку, на котороый заблокирован владелец или NULL */
    next_lock = task_blocked_on_lock(owner);

    platform_spin_unlock(&owner->pi_lock);

    /* Не обходить цепочку, если задача владелц не заблокирована сама. */
    if (!next_lock)
        return;

    /* Потеряется в sched_mutex_adjust_prio_chain()! */
    get_task_struct(owner);

    platform_spin_unlock(&lock->wait_lock);

    sched_mutex_adjust_prio_chain(self,
        owner, SCHED_MUTEX_MIN_CHAINWALK, lock, next_lock, NULL, self->curr);

    platform_spin_lock(&lock->wait_lock, self);
}


void
sched_mutex_adjust_pi(struct sched_rq *self, sched_task_t *task)
{
    struct sched_mutex_waiter *waiter;
    struct sched_mutex *next_lock;

    platform_spin_lock(&task->pi_lock, self);

    waiter = task->pi_blocked_on;
    if (!waiter || (waiter->prio == task->prio && !dl_prio(task->prio)))
    {
        platform_spin_unlock(&task->pi_lock);
        return;
    }
    next_lock = waiter->lock;
    platform_spin_unlock(&task->pi_lock);

    /* Потеряется в sched_mutex_adjust_prio_chain()! */
    get_task_struct(task);

    sched_mutex_adjust_prio_chain(self, task, SCHED_MUTEX_MIN_CHAINWALK, NULL, next_lock, NULL,
                                  task);
}


static void
sched_mutex_handle_deadlock(struct sched_rq *self, struct sched_mutex_waiter *w)
{
    int retcode = w->retcode;
    /*
     * Если результат не равен -EDEADLOCK или вызывающий запросил обнаружение взаимоблокировок
     * - ничего здесь не делать.
     */
    if (retcode != -EDEADLOCK && retcode != -EDEADLK)
        return;

    /* Громко сообщить и прекратить задание прямо здесь. */
    if (!w->deadlock_lock)
        printf("self deadlock detected\n");
    else
        debug_sched_mutex_print_deadlock(self, w);
    exit(-1);
}


/* Функция получения медленного пути try-lock */
static inline int
sched_mutex_slowtrylock(struct sched_rq *self, struct sched_mutex *lock)
{
    int ret;

    /*
     * Если блокировка уже имеет владельца, мы не сможем получить блокировку.
     * Это можно сделать, не принимая @lock->wait_lock, поскольку оно
     * только читается и это в любом случае является trylock.
     */
    if (sched_mutex_owner(lock))
        return 0;

    /*
     * Мьютекс в настоящий момент не имеет владельца. Заблокировать
     * wait lock и попытаться получить блокировку. Для поддержки
     * ранних вызовов загрузки мы используем здесь irqsave.
     */
    platform_spin_lock(&lock->wait_lock, self);

    ret = sched_mutex_internal_try_to_take(self, lock, self->curr, NULL);

    /*
     * try_to_take_sched_mutex() устанавливает бит блокировки ожидающего
     * беззаговорочно. Очистите его.
     */
    fixup_sched_mutex_waiters(lock);

    platform_spin_unlock(&lock->wait_lock);

    return ret;
}


/*
 * Медленный путь освобождения sched_mutex.
 * Возвращает, должна ли текущая задача отменить потенциальное повышение приоритетов.
 */
static bool
sched_mutex_slowunlock(struct sched_rq *self,
                       struct sched_mutex *lock,
                       struct sched_mutex_wake_q_head *wake_q)
{
    (void)self;
    /* для поддержки ранних загрузочных вызовов требуется irqsave */
    platform_spin_lock(&lock->wait_lock, self);

    {
        sched_task_t *owner;
        (void)owner;
        //  debug_sched_mutex_unlock(self, lock);
        assert((owner = sched_mutex_owner(lock)) == self->curr);
    }

    sched_mutex_deadlock_account_unlock(self->curr);

    /*
     * Мы должны быть здесь осторожными, если включен быстрый путь.
     * Если у нас нет ожидающих, поставленных в очередь, мы не
     * можем установить владельца в NULL здесь из за того, что:
     *
     * foo->lock->owner = NULL;
     *            sched_mutex_lock(foo->lock);   <- fast path
     *            free = atomic_dec_and_test(foo->refcnt);
     *            sched_mutex_unlock(foo->lock); <- fast path
     *            if (free)
     *                kfree(foo);
     * raw_spin_unlock(foo->lock->wait_lock);
     *
     * Таким образом, для ядра с включенным быстрым путем:
     *
     * Ничего не может установить бит ожидающих до тех пор, пока
     * мы удерживаем lock->wait_lock. Таким образом, мы делаем
     * следующую последовательномсть:
     *
     *    owner = sched_mutex_owner(lock);
     *    clear_sched_mutex_waiters(lock);
     *    raw_spin_unlock(&lock->wait_lock);
     *    if (cmpxchg(&lock->owner, owner, 0) == owner)
     *        return;
     *    goto retry;
     *
     * Вариант с отключенным быстрым путем является простым поскольку
     * весь доступ к lock->owner сериализуется через lock->wait_lock:
     *
     *    lock->owner = NULL;
     *    raw_spin_unlock(&lock->wait_lock);
     */
    while (!sched_mutex_has_waiters(lock))
    {
        /* Удаление lock->wait_lock ! */
        if (sched_mutex_internal_unlock_safe(lock) == true)
            return false;
        /* Повторная блокировка sched_mutex и попытка выполнить это снова */
        platform_spin_lock(&lock->wait_lock, self);
    }

    /*
     * Путь пробуждения следующего ожидающего не страдает от вышеуказанной гонки.
     * См. комментарии там.
     *
     * Поставить следующего ожидающего на пробуждение в очередь для пробуждения после того, как
     * мы отпустим wait_lock.
     */
    mark_wakeup_next_waiter(self, wake_q, lock);

    platform_spin_unlock(&lock->wait_lock);

    /* Проверить бустинг PI */
    return true;
}


static inline int
sched_mutex_fasttrylock(struct sched_rq *self, struct sched_mutex *lock,
                        int (*slowfn)(struct sched_rq *self, struct sched_mutex *lock))
{
    sched_task_t *p = NULL;
    (void)p;
    if (likely(sched_mutex_cmpxchg_acquire(lock, p, self->curr)))
        return 1;
    return slowfn(self, lock);
}


static inline void
sched_mutex_fastunlock(struct sched_rq *self,
                       struct sched_mutex *lock,
                       bool (*slowfn)(struct sched_rq *self,
                                      struct sched_mutex *lock,
                                      struct sched_mutex_wake_q_head *wqh))
{
    struct sched_mutex_wake_q_head wake_q = {WAKE_Q_TAIL, &wake_q.first};
    sched_task_t *tmp_curr = self->curr;
    (void)tmp_curr;

    if (likely(sched_mutex_cmpxchg_release(lock, tmp_curr, NULL)))
        sched_mutex_deadlock_account_unlock(self->curr);
    else
    {
        bool deboost = slowfn(self, lock, &wake_q);

        sched_mutex_wake_next_atomic_t node = wake_q.first;

        while (node != WAKE_Q_TAIL)
        {
            struct sched_mutex_waiter *waiter;

            waiter = container_of(node, struct sched_mutex_waiter, wake_q);
            assert(waiter != NULL);
            /* task can safely be re-inserted now */
            node = atomic_load_explicit(&node->next, memory_order_relaxed);
            atomic_store_explicit(&waiter->wake_q.next, NULL, memory_order_release);

            sched_mutex_wakeup_task(waiter);
        }

        /* Отменить бустинг PI, если это необходимо: */
        if (deboost)
            sched_mutex_adjust_prio(self, self->curr);
    }
}


static void
sched_mutex_timeout_callback(struct sched_timer *timer)
{
    struct sched_mutex_waiter *waiter = (struct sched_mutex_waiter *)(timer->action_expire_arg);
    assert(waiter != NULL && waiter->task != NULL && timer->expired);
    waiter->retcode = -ETIMEDOUT;                                                \
    sched_mutex_wakeup_task(waiter);
}


int
sched_mutex_internal_slowlock(struct sched_rq *self, struct sched_mutex *lock,
                              struct sched_timer *timeout, enum sched_mutex_chainwalk chwalk,
                              struct sched_mutex_waiter *waiter, sched_time_t timeout_val)
{
    static const struct sched_mutex_waiter
            initializer_sched_mutex_waiter = SCHED_MUTEX_WAITER_INITIALIZER;
    sched_task_t *task;

    *waiter = initializer_sched_mutex_waiter; // waiter.retcode = 0

    // debug_sched_mutex_init_waiter(waiter);
    RB_CLEAR_NODE(&(waiter->pi_tree_entry));
    RB_CLEAR_NODE(&(waiter->tree_entry));

    /*
     * Технически, мы можем здесь использовать raw_spin_[un]lock(),
     * но это можно вызвать в ранней начальной загрузке, если быстрый
     * путь cmpxchg() отключен (отладка, нет архитектурной поддержки).
     * В этом случае, мы будем получать rtmutex с удерживаемой lock->wait_lock.
     * Но мы не можем беззаговорочно включить прерывания в этом
     * раннем загрузочном случае. Поэтому, нужно использовать
     * варианты irqsave/restore.
     */
    platform_spin_lock(&lock->wait_lock, self);

    /* Попытайтесь снова получить блокировку: */
    if (sched_mutex_internal_try_to_take(self, lock, self->curr, NULL))
    {
        platform_spin_unlock(&lock->wait_lock);
        return 0;
    }

    task = self->curr;
    waiter->task = task;
    waiter->lock = lock;

    /* Установить timer, когда timeout != NULL */
    if (unlikely(timeout != NULL))
    {
        sched_timer_init(timeout, timeout_val, sched_mutex_timeout_callback,
                         (sched_timer_action_expire_arg)waiter);
        waiter->timeout = timeout;
        sched_timer_start_expires(self, timeout);
    }

    task_blocks_on_sched_mutex(self, lock, waiter, task, chwalk);
    return 1;
}


void
sched_mutex_internal_slowlock_tail(struct sched_rq *self, struct sched_mutex *lock,
                          struct sched_timer *timeout, struct sched_mutex_waiter *waiter)
{
    if (unlikely(waiter->retcode))
    {
        if (sched_mutex_has_waiters(lock))
            remove_waiter(self, lock, waiter);
        sched_mutex_handle_deadlock(self, waiter);
    }

    /*
     * try_to_take_sched_mutex() устанавливает бит
     * ожидающего беззаговорочно. Нам, возможно,
     * придется это согласовать.
     */
    fixup_sched_mutex_waiters(lock);

    platform_spin_unlock(&lock->wait_lock);

    /* Удалить таймер ожидания: */
    if (unlikely(timeout != NULL && !timeout->expired))
        sched_timer_cancel(self, timeout);
}


int
sched_mutex_trylock(struct sched_rq *self, struct sched_mutex *lock)
{
    return sched_mutex_fasttrylock(self, lock, sched_mutex_slowtrylock);
}


void
sched_mutex_unlock(struct sched_rq *self, struct sched_mutex *lock)
{
    sched_mutex_fastunlock(self, lock, sched_mutex_slowunlock);
}


bool
sched_mutex_futex_unlock(struct sched_rq *rq, struct sched_mutex *lock,
                         struct sched_mutex_wake_q_head *wqh)
{
    sched_task_t *tmp_curr = rq->curr;
    (void)tmp_curr;
    if (likely(sched_mutex_cmpxchg_release(lock, tmp_curr, NULL)))
    {
        sched_mutex_deadlock_account_unlock(rq->curr);
        return false;
    }
    return sched_mutex_slowunlock(rq, lock, wqh);
}


void
sched_mutex_destroy(struct sched_mutex *lock)
{
#ifdef CONFIG_SCHED_DEBUG_MUTEXES
    lock->magic = NULL;
#endif
}


void
sched_mutex_init_named(struct sched_mutex *lock, const char *name)
{
    static size_t id;
    id++;
    (void)name;

    lock->static_id = id;
    lock->owner = NULL;
    lock->has_waiters = false;
    lock->is_pending_owner = false;
    platform_spin_lock_init(&lock->wait_lock);
    lock->waiters = RB_ROOT;
    lock->waiters_leftmost = NULL;

    debug_sched_mutex_init(lock, name);
}


void
sched_mutex_setprio(struct sched_rq *self_rq, sched_task_t *p, sched_prio_t prio)
{
    struct sched_rq *self;
    sched_prio_t oldprio;

    assert(p != NULL && prio >= 0 && prio < SCHED_PRIO_MAX);

    self = lock_task_rq_strong(self_rq, p);

    /*
     * Бустинг неактивной задачи - nono в общем. Есть одно исключение,
     * когда PREEMPT_RT и NOHZ активны:
     *
     * idle задачи вызывают get_next_timer_interrupt() и удерживают
     * колесо таймера base->lock на процессоре и другой процессор хочет получить таймер
     * (вероятно, чтобы отменить его). Мы можем безопасно игнорировать запрос
     * бустинга, так как бездействующий процессор выполняет этот
     * код с отключенными прерываниями и будет полностью блокировать
     * защищеннуюю секцию без того, чтобы быть прерванным. Так что нет
     * никакой реальной необходимости в повышении (boost).
     */
    oldprio = p->prio;
    if (sched_task_state_running_relaxed(p))
    {
        p->prio = prio;
        if (prio > oldprio)
            set_tsk_need_resched(p);
    }
    else if (sched_task_state_queued(p))
    {
        dequeue_task(self, p);

        p->prio = prio;

        enqueue_task(self, p);
        if (prio < oldprio)
            set_tsk_need_resched(self->curr);
    }

    unlock_rq_task(self, p);
}
