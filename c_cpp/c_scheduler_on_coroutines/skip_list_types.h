#ifndef SKIP_LIST_TYPES_H
#define SKIP_LIST_TYPES_H


#include <stdatomic.h>
#include <stdint.h>

#include "padding_warns.h"


#define SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS 8




#define SCHED_SKIPLIST_MAX_LEVEL (SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS - 1)

#define SCHED_TIME_MAX UINT32_MAX
#define SCHED_STIME_MAX INT32_MAX

#define SCHED_SKIPLIST_KEY_MAX SCHED_TIME_MAX


typedef uint32_t sched_time_t;


typedef atomic_uint_least32_t sched_time_atomic_t;


typedef sched_time_atomic_t atomic_sched_sys_nsec_t;


typedef int32_t sched_stime_t;


typedef atomic_int_least32_t sched_skiplist_entries_cnt_t;


typedef uint32_t sched_skiplist_randseed_t;


typedef atomic_uint_least32_t sched_skiplist_randseed_atomic_t;


typedef sched_time_t sched_skiplist_key_type_t;


typedef sched_time_atomic_t sched_skiplist_key_type_atomic_t;


typedef void * sched_skiplist_value_type;


struct sched_skiplist_node
{
    /** Level in this structure */
    uint32_t level;

    sched_skiplist_key_type_t key;
    sched_skiplist_value_type value;

    struct sched_skiplist_node *next[SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS];
    struct sched_skiplist_node *prev[SCHED_SKIPLIST_MAX_NUMBER_OF_LEVELS];
};


#define SCHED_SKIPLIST_NODE_INITIALIZER                     \
    { /* clang-format off */                                \
        0, 0, NULL,                                      \
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }, \
        { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }  \
    } /* clang-format on */


struct sched_skiplist
{
    /** Number of elements in the container */
    sched_skiplist_entries_cnt_t entries;

    /** The maximum level of the skiplist (it is to 1 more than the number of levels in the list) */
    int32_t max_level;

    /** Top level of the list */
    struct sched_skiplist_node head_node;
};


#define SCHED_SKIPLIST_INITIALIZER { 0, 0, SCHED_SKIPLIST_NODE_INITIALIZER }


#endif // SKIP_LIST_TYPES_H
