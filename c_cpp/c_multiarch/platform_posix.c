#define _FILE_OFFSET_BITS 64

#include "platform/platform.h"

#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-macros"
#endif

#define _GNU_SOURCE

#ifdef __clang__
#pragma clang diagnostic pop
#endif


#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>


static void *
pthread_run_wrapper(void *ctx)
{
    ((struct platform_thread *) ctx)->thread_id = (uint_fast64_t)syscall(SYS_gettid);
    return (*((struct platform_thread *) ctx)->run_func)(((struct platform_thread *) ctx)->arg);
}


void
platform_thread_create(struct platform_thread *thread, platform_thread_run_func run_func, void *arg)
{
    assert(thread != NULL && run_func != NULL && arg != NULL);
    thread->run_func = run_func;
    thread->arg = arg;
    pthread_create(&thread->descriptor, NULL, pthread_run_wrapper, thread);
}


void
platform_thread_join(struct platform_thread *thread)
{
    assert(thread != NULL);
    pthread_join(thread->descriptor, NULL);
}


uint_fast64_t
platform_thread_id(struct platform_thread *thread)
{
    return thread->thread_id;
}


void
platform_thread_yield_imp(void)
{
   pthread_yield();
}


void
platform_thread_affine_cpu(struct platform_thread *thread, int cpu)
{
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu, &cpuset);
    pthread_setaffinity_np(thread->descriptor, sizeof(cpu_set_t), &cpuset);
}


void
platform_msleep(int msec)
{
    usleep(msec * 1000);
}


uint64_t
platform_monotonic_frequency(void)
{
    return 0;
}



uint64_t
platform_monotonic_nsec_imp(void)
{
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (uint64_t)((uint64_t)(tp.tv_sec) * 1000000000UL) + (uint64_t)(tp.tv_nsec);
}


int
platform_cpu_count(void)
{
    return (int)sysconf(_SC_NPROCESSORS_ONLN);
}


int
platform_fseek_64(FILE *stream, int64_t offset, int whence)
{
    return fseeko(stream, offset, whence);
}
