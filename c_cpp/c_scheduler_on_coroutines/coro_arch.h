#ifndef CORO_X86_H
#define CORO_X86_H

#include "platform/architecture.h"
#include "scheduler/sched_task_flags.h"
#include "scheduler/co_routine_types.h"
#include "scheduler/sched_task_types.h"
#include "scheduler/sched_task.h"


#if defined(PLATFORM_ARCH_X86_LIKE)


#include "scheduler/scheduler.h"
#include "platform/spinlocks.h"


static __thread coro_fun_t coro_init_func;
static __thread void  *coro_init_arg;
static __thread struct coro_context *new_coro, *create_coro;


/* windows, of course, gives a shit on the amd64 ABI and uses different registers */
/* http://blogs.msdn.com/freik/archive/2005/03/17/398200.aspx */


#if defined(PLATFORM_OS_WIN_LIKE)
#    if defined(PLATFORM_ARCH_X86_64)
#        define NUM_SAVED 29
#    elif defined(PLATFORM_ARCH_X86_I386_LIKE)
#        define NUM_SAVED 7
#    else
#        error unsupported architecture
#    endif // if defined(PLATFORM_ARCH_X86_64)
#else /* linux */
#    if defined(PLATFORM_ARCH_X86_64)
#        define NUM_SAVED 6
#    elif defined(PLATFORM_ARCH_X86_I386_LIKE)
#        define NUM_SAVED 4
#    else
#        error unsupported architecture
#    endif
#endif


static
void coro_call_wrapper(coro_fun_t func, void *arg)
{
    func((void *)arg);

    sched_task_t *task =  sched_task_current(arg);
    task->coro_exited = 1;
    coro_transfer(sched_task_current_coroutine_relaxed(task), &(sched_task_cpu(task)->coro_rq_ctx));
}

static void
coro_init(void)
{
    volatile coro_fun_t func = coro_init_func;
    volatile void *arg = coro_init_arg;

    coro_transfer(new_coro, create_coro);

#if __GCC_HAVE_DWARF2_CFI_ASM && PLATFORM_ARCH_X86_64
    asm (".cfi_undefined rip");
#endif

    coro_call_wrapper(func, (void *)arg);

    /* the new coro returned. bad. just abort() for now */
    abort();
}


#if defined(PLATFORM_OS_WIN_LIKE)

static inline void
coro_init_sp_winlike(coro_context *ctx, void *sptr, size_t ssize)
{
    *--ctx->sp = 0;                    /* ExceptionList */
    *--ctx->sp = (char *)sptr + ssize; /* StackBase */
    *--ctx->sp = sptr;                 /* StackLimit */
}

#else
#   define coro_init_sp_winlike(ctx, sptr, ssize)
#endif


void
coro_create(struct sched_rq *self, struct coro_context *ctx, coro_fun_t coro, void *arg,
            void *sptr, size_t ssize)
{
    struct coro_context nctx;

    if (coro == NULL)
        return;

    ctx->sp = (void **)(ssize + (char *)sptr);

    *--ctx->sp = NULL;
    *--ctx->sp = (void *)coro_init;

    coro_init_sp_winlike(ctx, sptr, ssize);

    ctx->sp -= NUM_SAVED;
    memset(ctx->sp, 0, sizeof (*ctx->sp) * NUM_SAVED);


    platform_spin_lock(&self->shared->coro_init_lock, self);
    coro_init_func = coro;
    coro_init_arg  = arg;

    new_coro    = ctx;
    create_coro = &nctx;

    // call coro_init
    coro_transfer(create_coro, new_coro);
    platform_spin_unlock(&self->shared->coro_init_lock);
}

#elif defined(PLATFORM_ARCH_ARM)


#include <string.h>

#include "coro_internal.h"
#include "platform/architecture.h"
#include "scheduler/coro.h"
#include "likely.h"


/* The lr register position in the architecture stack */
#if defined(PLATFORM_ARCH_ARM_VERSION) && PLATFORM_ARCH_ARM_VERSION == 7
#   define NUM_SAVED 25
#   define CORO_STARTUP_SP_POS 8
#elif defined(PLATFORM_ARCH_AARCH64)
#   define NUM_SAVED 20
#   define CORO_STARTUP_SP_POS 11
#else
#   error unsupported architecture
#endif


/**
 * coro_startup, if implemented, can lift new coro parameters from the
 * saved registers. Alternatively, we can pass parameters via globals at
 * the cost of 2 additional coro_transfer calls in coro_create.
 */
void
coro_startup();


void
coro_create(struct sched_rq *self, struct coro_context *ctx, coro_func coro, void *arg, void *sptr,
            size_t ssize)
{
    (void)self;
    if (unlikely(coro == NULL))
        return;

    ctx->sp = (void **)(ssize + (char *)sptr);

    ctx->sp -= NUM_SAVED;
    memset(ctx->sp, 0, sizeof (*ctx->sp) * NUM_SAVED);

    ctx->sp[0] = coro; /* r4 on armv7 ; x19 on aarch64 */
    ctx->sp[1] = arg;  /* r5 on armv7 ; x20 on aarch64 */
    ctx->sp[CORO_STARTUP_SP_POS] = (void *)coro_startup; /* lr */
}



#else
#   error unsupported architecture
#endif // if defined(PLATFORM_ARCH_ARM)


#endif // CORO_X86_H
