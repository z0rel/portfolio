#include "platform/platform.h"

#include <assert.h>


static DWORD WINAPI
winapi_run_wrapper(CONST LPVOID ctx)
{
    ((struct platform_thread *) ctx)->thread_id = GetCurrentThreadId();
    void *result = (*((struct platform_thread *) ctx)->run_func)(
                      ((struct platform_thread *) ctx)->arg);
    ExitThread((DWORD)((size_t)result));
}


void
platform_thread_create(struct platform_thread *thread, platform_thread_run_func run_func, void *arg)
{
    assert(thread != NULL && run_func != NULL && arg != NULL);
    thread->run_func = run_func;
    thread->arg = arg;
    thread->descriptor = CreateThread(NULL, 0, &winapi_run_wrapper, thread, 0, NULL);
}


void
platform_thread_join(struct platform_thread *thread)
{
    assert(thread != NULL);
    WaitForSingleObject(thread->descriptor, INFINITE);
}


uint_fast64_t
platform_thread_id(struct platform_thread *thread)
{
    return thread->thread_id;
}


void
platform_msleep(int msec)
{
    Sleep(msec);
}


uint64_t
platform_monotonic_frequency()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}


uint64_t
platform_monotonic_nsec_imp(uint64_t frequency)
{
    LARGE_INTEGER qpl;
    QueryPerformanceCounter(&qpl);
    return (qpl.QuadPart * 1000000000) / frequency;
}


int
platform_fseek_64(FILE *stream, int64_t offset, int whence)
{
    return _fseeki64(stream, offset, whence);
}


int
platform_cpu_count(void)
{
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors;
}


void
platform_thread_affine_cpu(struct platform_thread *thread, int cpu)
{
    DWORD_PTR mask = 1 << cpu;
    SetThreadAffinityMask(thread->descriptor,  mask);
}

