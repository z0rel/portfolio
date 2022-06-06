/**
 * @file
 * @brief scheduiler-mutexes: blocking mutual exclusion locks with PI support
 */

#define sched_mutex_deadlock_check(l)           (0)
#define sched_mutex_deadlock_account_unlock(l)  do { } while (0)
#define debug_sched_mutex_init_waiter(w)        do { } while (0)
#define debug_sched_mutex_free_waiter(w)        do { } while (0)
#define debug_sched_mutex_lock(l)               do { } while (0)
#define debug_sched_mutex_proxy_lock(l,p)       do { } while (0)
#define debug_sched_mutex_proxy_unlock(l)       do { } while (0)
#define debug_sched_mutex_unlock(l)             do { } while (0)
#define debug_sched_mutex_init(m, n)            do { } while (0)
#define debug_sched_mutex_deadlock(d, a ,l)     do { } while (0)
#define debug_sched_mutex_print_deadlock(w)     do { } while (0)
#define debug_sched_mutex_reset_waiter(w)       do { } while (0)


static inline void
sched_mutex_print_deadlock(struct sched_mutex_waiter *w)
{
    (void)w;
    printf("rtmutex deadlock detected\n");
}


static inline bool
debug_sched_mutex_detect_deadlock(struct sched_mutex_waiter *w, enum sched_mutex_chainwalk walk)
{
    (void)w;
    return walk == SCHED_MUTEX_FULL_CHAINWALK;
}
