#ifndef CORO_TYPES_H
#define CORO_TYPES_H


#include <stddef.h>

#include "platform/architecture.h"
#include "config_sched.h"
#include "padding_warns.h"

struct sched_rq;


/** This is the type for the initialization function of a new coroutine. */
typedef void (*coro_fun_t)(void *);


struct coro_context;

/**
 * This function creates a new coroutine. Apart from a pointer to an
 * uninitialised coro_context, it expects a pointer to the entry function
 * and the single pointer value that is given to it as argument.
 *
 * Allocating/deallocating the stack is your own responsibility.
 *
 * As a special case, if coro, arg, sptr and ssze are all zero,
 * then an "empty" coro_context will be created that is suitable
 * as an initial source for coro_transfer.
 *
 * This function is not reentrant, but putting a mutex around it
 * will work.
 *
 * @param ctx   an uninitialised coro_context
 * @param coro  the coroutine code to be executed
 * @param arg   a single pointer passed to the coro
 * @param sptr  start of stack area
 * @param ssze  size of stack area in bytes
 */
void
coro_create(struct sched_rq *self, struct coro_context *ctx, coro_fun_t coro, void *arg,
            void *sptr, size_t ssze);



/*
 * -DCORO_USE_VALGRIND
 *
 *    If defined, then libcoro will include valgrind/valgrind.h and register
 *    and unregister stacks with valgrind.
 *
 * -CORO_GUARDPAGES=n
 *
 *    libcoro will try to use the specified number of guard pages to protect against
 *    stack overflow. If n is 0, then the feature will be disabled. If it isn't
 *    defined, then libcoro will choose a suitable default. If guardpages are not
 *    supported on the platform, then the feature will be silently disabled.
 */


/*
 * The only allowed operations on these struct members is to read the
 * "sptr" and "ssze" members to pass it to coro_create, to read the "sptr"
 * member to see if it is false, in which case the stack isn't allocated,
 * and to set the "sptr" member to 0, to indicate to coro_stack_free to
 * not actually do anything.
 */
struct coro_stack
{
    void  *sptr;
    size_t ssze;

#if CONFIG_SCHED_USE_VALGRIND
    int valgrind_id;

    PADDING_WARN_64_ITEM32(pad);
#endif
};

#if CONFIG_SCHED_USE_VALGRIND
#   define CORO_STACK_INITIALIZER { NULL, 0, 0 PADDING_WARN_64_ITEM32_INITIALIZER }
#else
#   define CORO_STACK_INITIALIZER { NULL, 0 }
#endif


/**
 * A coroutine state is saved in the following structure. Treat it as an
 * opaque type. errno and sigmask might be saved, but don't rely on it,
 * implement your own switching primitive if you need that.
 */
struct coro_context
{
    void **sp; /* must be at offset 0 */
};

#define CORO_CONTEXT_INITIALIZER { NULL }


#endif // CORO_TYPES_H
