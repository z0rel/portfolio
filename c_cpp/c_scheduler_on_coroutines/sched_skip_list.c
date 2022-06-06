#include "scheduler/skip_list_types.h"
#include "scheduler/skip_list.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "include/scheduler/cpumask_op.h"




void
sched_skiplist_init(struct sched_skiplist *skiplist)
{
    int i;

    skiplist->entries = 0;
    skiplist->max_level = 0;
    skiplist->head_node.key = SCHED_SKIPLIST_KEY_MAX;
    skiplist->head_node.level = 0;
    skiplist->head_node.value = NULL;


    /* At initialization, the main node of the skiplist looped on itself */
    for (i = 0; i < SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS; i++)
        skiplist->head_node.next[i] = skiplist->head_node.prev[i] = &skiplist->head_node;
}


void
sched_skiplist_insert(struct sched_skiplist *self, struct sched_skiplist_node *ins_node,
                      const sched_skiplist_key_type_t key, const sched_skiplist_value_type value,
                      sched_skiplist_randseed_t randseed)
{
    assert(key != 0);
    struct sched_skiplist_node *update_nodes[SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS];
    {
        int level = self->max_level;
        /*
         * Selecting of position for inserting the element by each level
         * [0 ... skiplist->max_level] of the list
         */
        {
            struct sched_skiplist_node *upd_node = &self->head_node;
            do
            {
                struct sched_skiplist_node *it;
                for (it = upd_node->next[level]; it->key <= key; it = upd_node->next[level])
                {
                    assert(it && it != it->next[level]);
                    upd_node = it;
                }
                update_nodes[level] = upd_node;
            } while (--level >= 0);
        }
    }

    /* Increase the number of entities */
    self->entries++;

    {
        /* random level: level = random([0, ..., skiplist->max_level]) */
        /* level = find_first_bit(&randseed, SCHED_SKIPLIST_MAX_LEVEL) */
        int level = __builtin_ctz(randseed); /* TODO: */
        if (level > SCHED_SKIPLIST_MAX_LEVEL)
        {
            level = SCHED_SKIPLIST_MAX_LEVEL;
        }
        if (level > self->max_level)
        {
            level = ++self->max_level; /* Increase the number of levels in the skiplist */
            update_nodes[level] = &self->head_node; /* update.append(skiplist->header) */
        }

        ins_node->level = level;
        ins_node->key = key;
        ins_node->value = value;

        /* random level: level = [0, ..., random([0, ..., skiplist->max_level+1])] */
        do
        {
            /* inserting ins_node between update[level] and update->next[level] */
            struct sched_skiplist_node *prev_item = update_nodes[level];
            struct sched_skiplist_node *next_item = prev_item->next[level];
            prev_item->next[level] = ins_node;
            next_item->prev[level] = ins_node;
            ins_node->prev[level] = prev_item;
            ins_node->next[level] = next_item;
        } while (--level >= 0);
    }
}


void
sched_skiplist_delete(struct sched_skiplist *l, struct sched_skiplist_node *node)
{
    static const struct sched_skiplist_node empty_node = SCHED_SKIPLIST_NODE_INITIALIZER;

    int k, m = node->level;
    assert(node != &l->head_node);

    for (k = 0; k <= m; k++)
    {
        assert(node->prev[k] != NULL);
        assert(node->next[k] != NULL);

        node->prev[k]->next[k] = node->next[k];
        node->next[k]->prev[k] = node->prev[k];
    }
    *node = empty_node;
    if (m == l->max_level)
    {
        while (l->head_node.next[m] == &l->head_node && l->head_node.prev[m] == &l->head_node
               && m > 0)
        {
            m--;
        }
        l->max_level = m;
    }
    l->entries--;
}
