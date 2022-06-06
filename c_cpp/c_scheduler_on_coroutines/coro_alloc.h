#ifndef CORO_MMAP_H
#define CORO_MMAP_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/mman.h>

#include "likely.h"
#include "config_sched.h"
#include "scheduler/sched_errno.h"

#include "platform/architecture.h"


#if !defined(PLATFORM_OS_WIN_LIKE)
#   include <unistd.h>
#   if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES
#       include <sys/mman.h>
#       if defined(MAP_ANON) && !defined(MAP_ANONYMOUS)
#           define MAP_ANONYMOUS MAP_ANON
#           define CORO_ALLOC_MMAP 1
#       elif defined(MAP_ANONYMOUS)
#           define CORO_ALLOC_MMAP 1
#       else
#           error "MAP_ANONYMOUS or MAP_ANON is undefined"
#       endif
#   endif
#endif

//#if defined(CORO_ALLOC_MMAP)
//#    undef CORO_ALLOC_MMAP
//#endif


#if CONFIG_SCHED_USE_VALGRIND
#   include <valgrind/valgrind.h>

#   define CORO_VALGRIND_STACK_DEREGISTER(arg)  VALGRIND_STACK_DEREGISTER(arg)

#   define CORO_VALGRIND_STACK_REGISTER(stack, base, ssze)                                         \
        (stack)->valgrind_id = VALGRIND_STACK_REGISTER(                                            \
                                  (char *)base,                                                    \
                                  ((char*)base) + ssze - CORO_GUARDPAGES * coro_pagesize())
#else
#   define CORO_VALGRIND_STACK_DEREGISTER(arg)
#   define CORO_VALGRIND_STACK_REGISTER(id, base, ssze)
#endif // #if CORO_USE_VALGRIND


#define SET_STACK_BASE(stack, base) (stack)->sptr = (base)

static inline size_t
coro_stack_alloc_init_ssze(struct coro_stack *stack, unsigned int size);


#if defined(CORO_ALLOC_MMAP)


static inline size_t
coro_pagesize(void)
{
    static size_t pagesize = 0;

    if (unlikely(!pagesize))
        pagesize = sysconf(_SC_PAGESIZE);

    return pagesize;
}


// based on the unistd definition
#   if _POSIX_MEMORY_PROTECTION
#       define CORO_GUARDPAGES 4
#       define CORO_MPROTECT(base)  mprotect(base, CORO_GUARDPAGES * coro_pagesize(), PROT_NONE)
#   else
#       define CORO_GUARDPAGES 0
#       define CORO_MPROTECT(base)
#   endif // #if _POSIX_MEMORY_PROTECTION


enum sched_return_codes
coro_stack_alloc(struct coro_stack *stack, unsigned int size)
{
    size_t ssze = coro_stack_alloc_init_ssze(stack, size);

    /* mmap supposedly does allocate-on-write for us */
    void *base = mmap(0, ssze, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1,
                      0);

    if (base == (void *)-1)
    {
        /* some systems don't let us have executable heap */
        /* we assume they won't need executable stack in that case */
        base = mmap(0, ssze, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

        if (base == (void *)-1)
            return SCHED_ENOMEM;
    }

    CORO_MPROTECT(base);

    base = (void*)((char *)base + CORO_GUARDPAGES * coro_pagesize());

    CORO_VALGRIND_STACK_REGISTER(stack, base, ssze);
    SET_STACK_BASE(stack, base);
    return SCHED_EOK;
}

// truncate: mprotect - PROT_NONE; mprotect READ WRITE EXEC;
void
coro_stack_free(struct coro_stack *stack)
{
    CORO_VALGRIND_STACK_DEREGISTER(stack->valgrind_id);

    if (stack->sptr)
        munmap((void*)((char *)stack->sptr - CORO_GUARDPAGES * coro_pagesize()),
               stack->ssze                 + CORO_GUARDPAGES * coro_pagesize());
}


static inline size_t
coro_stack_alloc_init_ssze(struct coro_stack *stack, unsigned int size)
{
    if (!size)
      size = CONFIG_SCHED_CORO_DEFAULT_STACKSIZE;

    stack->sptr = 0;
    stack->ssze = (((size_t)size * sizeof (void *) + coro_pagesize() - 1) / coro_pagesize())
                  * coro_pagesize();

    size_t ssze = stack->ssze + CORO_GUARDPAGES * coro_pagesize();

    return ssze;
}


#else // !defined(CORO_ALLOC_MMAP)

#define CORO_GUARDPAGES 0
#define coro_pagesize() 0


enum sched_return_codes
coro_stack_alloc(struct coro_stack *stack, unsigned int size)
{
    size_t ssze = coro_stack_alloc_init_ssze(stack, size);

    void *base = malloc(ssze);
    if (!base)
        return SCHED_ENOMEM;

    CORO_VALGRIND_STACK_REGISTER(stack, base, ssze);
    SET_STACK_BASE(stack, base);
    return SCHED_EOK;
}


void
coro_stack_free(struct coro_stack *stack)
{
    CORO_VALGRIND_STACK_DEREGISTER(stack->valgrind_id);
    free(stack->sptr);
}


static inline size_t
coro_stack_alloc_init_ssze(struct coro_stack *stack, unsigned int size)
{
    if (!size)
      size = CONFIG_SCHED_CORO_DEFAULT_STACKSIZE;

    stack->sptr = 0;
    stack->ssze = ((size_t)size * sizeof (void *) - 1);

    size_t ssze = stack->ssze + CORO_GUARDPAGES * coro_pagesize();

    return ssze;
}



#endif // if defined(CORO_ALLOC_MMAP)






#endif // CORO_MMAP_H
