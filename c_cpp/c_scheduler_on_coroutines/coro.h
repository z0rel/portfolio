#ifndef CORO_H
#define CORO_H

#include <stdbool.h>
#include <string.h>

#include "platform/architecture.h"
#include "scheduler/coro_types.h"


/*
 * Try to allocate a stack of at least the given size and return true if
 * successful, or false otherwise.
 *
 * The size is *NOT* specified in bytes, but in units of sizeof (void *),
 * i.e. the stack is typically 4(8) times larger on 32 bit(64 bit) platforms
 * then the size passed in.
 *
 * If size is 0, then a "suitable" stack size is chosen (usually 1-2MB).
 */
enum sched_return_codes
coro_stack_alloc(struct coro_stack *stack, unsigned int size);


/*
 * Free the stack allocated by coro_stack_alloc again. It is safe to
 * call this function on the coro_stack structure even if coro_stack_alloc
 * failed.
 */
void
coro_stack_free(struct coro_stack *stack);


#if !defined(PLATFORM_ARCH_ARM)

/**
 * The following prototype defines the coroutine switching function. It is
 * sometimes implemented as a macro, so watch out.
 *
 * This function is thread-safe and reentrant.
 */
void __attribute__((__noinline__, __regparm__(2)))
coro_transfer(struct coro_context *prev, struct coro_context *next);

#else

/**
 * The following prototype defines the coroutine switching function. It is
 * sometimes implemented as a macro, so watch out.
 *
 * This function is thread-safe and reentrant.
 */
void __attribute__((__noinline__))
coro_transfer(struct coro_context *prev, struct coro_context *next);

#endif // #if !defined(PLATFORM_ARCH_ARM)


/* This function is thread-safe and reentrant. */
# define coro_destroy(ctx) (void *)(ctx)



#endif // CORO_H
