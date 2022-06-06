/**
 * @file
 * scheduler mutexes: blocking mutual exclusion locks with PI support
 */


#include <stdio.h>
#include <string.h>

#include "config_sched.h"
#include "sched_mutex_internal.h"


#ifdef CONFIG_SCHED_DEBUG_MUTEXES

static platform_spinlock_t debug_sl = PLATFORM_SPINLOCK_INITIALIZER;


static void
printf_task(sched_task_t *p)
{
    if (p)
        printf("%5d [%p, %3d]", p->tid, p, p->prio);
    else
        printf("<none>");
}


static void
printf_lock(struct sched_mutex *lock, int print_owner)
{
    if (lock->name)
        printf(" [%p] {%s}\n", lock, lock->name);
    else
        printf(" [%p] {%s:%d}\n", lock, lock->file, lock->line);

    if (print_owner && sched_mutex_owner(lock))
    {
        sched_task_t *owner = atomic_load_explicit(&lock->owner, memory_order_relaxed);
        bool has_waiters = atomic_load_explicit(&lock->has_waiters, memory_order_relaxed);

        printf(".. ->owner: %p, tid: %u, has_waiters: %i\n",
               owner, owner == NULL ? UINT32_MAX : owner->tid, has_waiters);
        printf(".. held by:  ");
        printf_task(sched_mutex_owner(lock));
        printf("\n");
    }
}


void
sched_mutex_debug_task_free(sched_task_t *task)
{
    assert(!RB_EMPTY_ROOT(&task->pi_waiters));
    assert(task->pi_blocked_on);
}


/*
 * We fill out the fields in the waiter to store the information about
 * the deadlock. We print when we return. act_waiter can be NULL in
 * case of a remove waiter operation.
 */
void
debug_sched_mutex_deadlock(enum sched_mutex_chainwalk chwalk,
                           struct sched_mutex_waiter *act_waiter,
                           struct sched_mutex *lock)
{
    sched_task_t *task;

    if (chwalk == SCHED_MUTEX_FULL_CHAINWALK || !act_waiter)
        return;

    task = sched_mutex_owner(act_waiter->lock);
    if (task && task != sched_task_cpu(task)->curr)
    {
        act_waiter->deadlock_task_pid = task;
        act_waiter->deadlock_lock = lock;
    }
}


void
debug_sched_mutex_print_deadlock(struct sched_rq *self, struct sched_mutex_waiter *waiter)
{
    sched_task_t *task;
    sched_task_t *current;

    if (!waiter->deadlock_lock)
    {
        return;
    }

    platform_spin_lock(&debug_sl, self);
    task = waiter->deadlock_task_pid;
    if (!task)
    {
        platform_spin_unlock(&debug_sl);
        return;
    }
    current = sched_task_cpu(task)->curr;

    printf("\n============================================\n");
    printf("[ BUG: circular locking deadlock detected! ]\n");
    printf("--------------------------------------------\n");
    printf("task %d is deadlocking current task %d\n\n", task->tid, current->tid);
    printf("\n");
    printf("1) %d is trying to acquire this lock:\n", current->tid);
    printf_lock(waiter->lock, 1);
    printf("\n");
    printf("2) %d is blocked on this lock:\n", task->tid);
    printf_lock(waiter->deadlock_lock, 1);


    printf("\n");
    printf("%d's [blocked] stackdump:\n\n", task->tid);
    printf("\n");
    printf("%d's [current] stackdump:\n\n", current->tid);
    platform_spin_unlock(&debug_sl);

    printf("[ turning off deadlock detection."
           "Please report this trace. ]\n\n");
}


void
debug_sched_mutex_lock(struct sched_mutex *lock)
{
    (void)lock;
}


void
debug_sched_mutex_unlock(struct sched_rq *self, struct sched_mutex *lock)
{
    assert(sched_mutex_owner(lock) == self->curr);
}


void
debug_sched_mutex_proxy_lock(struct sched_mutex *lock, sched_task_t *powner)
{
    (void)lock;
    (void)powner;
}


void
debug_sched_mutex_proxy_unlock(struct sched_mutex *lock)
{
    assert(sched_mutex_owner(lock) != NULL);
}


void
debug_sched_mutex_init_waiter(struct sched_mutex_waiter *waiter)
{
//    memset(waiter, 0x11, sizeof(*waiter));
    waiter->deadlock_task_pid = NULL;
}


void
debug_sched_mutex_free_waiter(struct sched_mutex_waiter *waiter)
{
    // put_pid(waiter->deadlock_task_pid);
     memset(waiter, 0x22, sizeof(*waiter));
}


void
debug_sched_mutex_init(struct sched_mutex *lock, const char *name)
{
    /*
	 * Make sure we are not reinitializing a held lock:
	 */
    // debug_check_no_locks_freed((void *)lock, sizeof(*lock));
    lock->name = name;
}




void
sched_mutex_deadlock_account_unlock(sched_task_t *task)
{
    (void)task;
}


#endif // CONFIG_SCHED_DEBUG_MUTEXES
