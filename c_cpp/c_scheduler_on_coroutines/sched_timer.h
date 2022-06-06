#ifndef SCHED_TIMER_H
#define SCHED_TIMER_H

#include <assert.h>

#include "scheduler/scheduler.h"
#include "scheduler/sched_timer_types.h"
#include "scheduler/skip_list.h"
#include "platform/mt19937.h"


static inline void
sched_timer_init(struct sched_timer *timer, sched_time_t expired_nsec,
                 sched_timer_action_expire action_expire, sched_timer_action_expire_arg arg)
{
    static const struct sched_skiplist_node t = SCHED_SKIPLIST_NODE_INITIALIZER;
    assert(timer != NULL);
    timer->waiting_queue_node = t;
    timer->action_expire = action_expire;
    timer->action_expire_arg = arg;
    timer->timeout = expired_nsec;
    timer->expired = false;
}



static inline void
sched_timer_cancel(struct sched_rq *self, struct sched_timer *timer)
{
    if (unlikely(sched_skiplist_node_empty(&timer->waiting_queue_node)))
        return;

    sched_skiplist_delete(&self->timers_queue, &timer->waiting_queue_node);
}


static inline void
sched_timer_start_expires(struct sched_rq *self, struct sched_timer *timer)
{
    assert(timer != NULL && sched_skiplist_node_empty(&timer->waiting_queue_node));

    sched_skiplist_insert(
        /* self     = */ &self->timers_queue,
        /* ins_node = */ &timer->waiting_queue_node,
        /* key      = */ self->time_local_rq + timer->timeout,
        /* value    = */ (void*)timer,
        /* randseed = */ (sched_skiplist_randseed_t)rand_mt19937_rand_uint32(&self->random_gen));
}


static inline int
sched_timer_queue_timeout(struct sched_rq *self)
{
    int timeout = 0;
    
    if (likely(self->timers_queue.entries))
    {
        struct sched_skiplist_node *head = &self->timers_queue.head_node;
        struct sched_skiplist_node *item = head->next[0];
        
        if (item != head)
            timeout = item->key - self->time_local_rq;
  }
  return timeout > 0 ? timeout : 0;
}


#endif // SCHED_TIMER_H
