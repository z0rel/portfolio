#ifndef SCHED_TIMER_TYPES_H
#define SCHED_TIMER_TYPES_H

#include <stdbool.h>
#include <stddef.h>
#include "padding_warns.h"
#include "scheduler/skip_list_types.h"

struct sched_timer;

typedef void (*sched_timer_action_expire)(struct sched_timer*);

typedef void* sched_timer_action_expire_arg;


/** simple sleeper structure */
struct sched_timer
{
    struct sched_skiplist_node waiting_queue_node;

    sched_timer_action_expire action_expire;

    sched_timer_action_expire_arg action_expire_arg;

    sched_time_t timeout;

    bool expired;

    PADDING_WARN_SUPRESS_TAIL(pad, sizeof(uint32_t)-1);
};

#define SCHED_TIMER_INITIALIZER { SCHED_SKIPLIST_NODE_INITIALIZER, NULL, NULL, 0, false, \
                                  { false } }


#endif // SCHED_TIMER_TYPES_H
