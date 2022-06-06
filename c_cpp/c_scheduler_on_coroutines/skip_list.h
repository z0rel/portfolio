#ifndef SCHED_SKIP_LIST_H
#define SCHED_SKIP_LIST_H

#include <stdbool.h>

#include "scheduler/skip_list_types.h"


void
sched_skiplist_init(struct sched_skiplist *skiplist);


void
sched_skiplist_insert(struct sched_skiplist *l, struct sched_skiplist_node *node,
                      const sched_skiplist_key_type_t key,
                      const sched_skiplist_value_type value,
                      sched_skiplist_randseed_t randseed);


void
sched_skiplist_delete(struct sched_skiplist *l, struct sched_skiplist_node *node);


static inline bool
sched_skiplist_node_empty(struct sched_skiplist_node *node)
{
    return (!node->next[0]);
}



#endif /* SCHED_SKIP_LIST_H */
