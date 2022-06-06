#ifndef SCHED_MUTEX_RBTREE_H
#define SCHED_MUTEX_RBTREE_H

#include <stddef.h>

#include "platform/utils.h"
#include "scheduler/sched_rbtree_types.h"


#define rb_parent(r) ((struct sched_rb_node *)((r)->__rb_parent_color & ~3))


#define RB_ROOT (struct sched_rb_root) { NULL }


#define rb_entry(ptr, type, member) container_of(ptr, type, member)


#define RB_EMPTY_ROOT(root) ((root)->sched_rb_node == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node) ((node)->__rb_parent_color == (size_t)(node))


#define RB_CLEAR_NODE(node) ((node)->__rb_parent_color = (size_t)(node))


void
rb_insert_color(struct sched_rb_node *, struct sched_rb_root *);


void
rb_erase(struct sched_rb_node *, struct sched_rb_root *);


/* Find logical next and previous nodes in a tree */
struct sched_rb_node *
rb_next(const struct sched_rb_node *);


struct sched_rb_node *
rb_prev(const struct sched_rb_node *);


struct sched_rb_node *
rb_first(const struct sched_rb_root *);


struct sched_rb_node *
rb_last(const struct sched_rb_root *);


/* Postorder iteration - always visit the parent after its children */
struct sched_rb_node *
rb_first_postorder(const struct sched_rb_root *);


struct sched_rb_node *
rb_next_postorder(const struct sched_rb_node *);


/* Fast replacement of a single node without remove/rebalance/add/rebalance */
void
rb_replace_node(struct sched_rb_node *victim, struct sched_rb_node *newnode,
                struct sched_rb_root *root);


static inline void
rb_link_node(struct sched_rb_node *node, struct sched_rb_node *parent,
             struct sched_rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
}


#define rb_entry_safe(ptr, type, member)                                                           \
    ({                                                                                             \
        typeof(ptr) ____ptr = (ptr);                                                               \
        ____ptr ? rb_entry(____ptr, type, member) : NULL;                                          \
    })


/*
 * Handy for checking that we are not deleting an entry that is
 * already in a list, found in block/{blk-throttle,cfq-iosched}.c,
 * probably should be moved to lib/rbtree.c...
 */
static inline void
rb_erase_init(struct sched_rb_node *n, struct sched_rb_root *root)
{
    rb_erase(n, root);
    RB_CLEAR_NODE(n);
}


#endif // SCHED_MUTEX_RBTREE_H
