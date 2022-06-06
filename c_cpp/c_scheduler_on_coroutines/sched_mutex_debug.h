/**
 * @file
 * @brief scheduler mutexes: blocking mutual exclusion locks with PI support
 */
#ifndef SCHED_MUTEX_DEBUG_H
#define SCHED_MUTEX_DEBUG_H

struct sched_mutex;
struct sched_task_t;

void
sched_mutex_deadlock_account_unlock(struct sched_task_t *task);


void
debug_sched_mutex_init_waiter(struct sched_mutex_waiter *waiter);


void
debug_sched_mutex_free_waiter(struct sched_mutex_waiter *waiter);


void
debug_sched_mutex_init(struct sched_mutex *lock, const char *name);


void
debug_sched_mutex_lock(struct sched_mutex *lock);


void
debug_sched_mutex_unlock(struct sched_rq *self, struct sched_mutex *lock);


void
debug_sched_mutex_proxy_lock(struct sched_mutex *lock, struct sched_task_t *powner);


void
debug_sched_mutex_proxy_unlock(struct sched_mutex *lock);


void
debug_sched_mutex_deadlock(enum sched_mutex_chainwalk chwalk, struct sched_mutex_waiter *waiter,
                           struct sched_mutex *lock);


void
debug_sched_mutex_print_deadlock(struct sched_rq *self, struct sched_mutex_waiter *waiter);


#define debug_sched_mutex_reset_waiter(w) do { (w)->deadlock_lock = NULL; } while (0)


static inline bool
debug_sched_mutex_detect_deadlock(struct sched_mutex_waiter *waiter,
                                  enum sched_mutex_chainwalk walk)
{
    (void)walk;
    return (waiter != NULL);
}


static inline void
sched_mutex_print_deadlock(struct sched_rq *self, struct sched_mutex_waiter *w)
{
    debug_sched_mutex_print_deadlock(self, w);
}


#endif // SCHED_MUTEX_DEBUG_H
