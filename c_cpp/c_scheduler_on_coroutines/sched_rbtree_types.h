#ifndef SCHED_MUTEX_RBTREE_TYPES_H
#define SCHED_MUTEX_RBTREE_TYPES_H

#include <stddef.h>


struct sched_rb_node
{
    size_t __rb_parent_color;
    struct sched_rb_node *rb_right;
    struct sched_rb_node *rb_left;
};


#define  SCHED_RB_NODE_INITIALIZER { 0, NULL, NULL }


struct sched_rb_root
{
    struct sched_rb_node *sched_rb_node;
};


#define SCHED_RB_ROOT_INITIALIZER { NULL }


#endif // SCHED_MUTEX_RBTREE_TYPES_H
