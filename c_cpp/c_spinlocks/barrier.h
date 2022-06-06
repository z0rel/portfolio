#ifndef PLATFORM_BARRIER_H
#define PLATFORM_BARRIER_H

#include "platform/architecture.h"


/** Macro, ported from the Linux kernel */
#define platform_memory_barrier() asm volatile ("":::"memory")

#if defined(PLATFORM_ARCH_ARM)

#   define platform_asm_sev()  asm volatile ("sev" : : : "memory")
#   define platform_asm_wfe()  asm volatile ("wfe" : : : "memory")
#   if PLATFORM_ARCH_ARM_VERSION >= 7
#       define platform_asm_dsb(option) asm volatile ("dsb " #option : : : "memory")
#       define platform_asm_dmb(option) asm volatile ("dmb " #option : : : "memory")
#   elif PLATFORM_ARCH_ARM_VERSION == 6
#       define platform_asm_dsb(x) asm volatile ("mcr p15, 0, %0, c7, c10, 4"::"r" (0) : "memory")
#       define platform_asm_dmb(x) asm volatile ("mcr p15, 0, %0, c7, c10, 5"::"r" (0) : "memory")
#   else
#       define platform_asm_dsb(x) asm volatile ("mcr p15, 0, %0, c7, c10, 4"::"r" (0) : "memory")
#       define platform_asm_dmb(x) asm volatile ("" : : : "memory")
#   endif
#   define platform_asm_smb_mb()  platform_asm_dmb(ish)

#   if PLATFORM_ARCH_ARM_VERSION == 6
#       define platform_cpu_relax() platform_asm_smb_mb()
#   else
#       define platform_cpu_relax() platform_memory_barrier()
#   endif

#elif defined(PLATFORM_ARCH_X86_32)

#   if defined(PLATFORM_CPU_SSE)
#       define platform_memory_barrier_all()   asm volatile("mfence":::"memory", "cc")
#       define platform_memory_barrier_read()  asm volatile("lfence":::"memory", "cc")
#       define platform_memory_barrier_write() asm volatile("sfence":::"memory", "cc")
#   else
#       define platform_memory_barrier_all() asm volatile("lock; addl $0,0(%%esp)":::"memory", "cc")
#       define platform_memory_barrier_read()  platform_memory_barrier_all()
#       define platform_memory_barrier_write() platform_memory_barrier_all()

#endif
#elif defined(PLATFORM_ARCH_X86_64)

#   define platform_memory_barrier_all()   asm volatile("mfence":::"memory")
#   define platform_memory_barrier_read()  asm volatile("lfence":::"memory")
#   define platform_memory_barrier_write() asm volatile("sfence":::"memory")

#endif


/* fallback define of the platform_cpu_relax */
#ifndef platform_cpu_relax
#   define platform_cpu_relax() platform_memory_barrier()
#endif


/* TODO: для arm64 в ядре есть оптимизация цикла while */
/**
 * Подождать, пока не выполнится cond_expr, выполняя проверки по событию изменения ptr.
 * После выполнения cond_expr вернуть текущее значение ptr
 * @param ptr
 *     Переменная-триггер
 * @param cond_expr
 *     Условие для проверки по изменению ptr
 */
#define platform_smp_cond_load_acquire(ptr, cond_expr)                                             \
({                                                                                                 \
    typeof(ptr) __PTR = (ptr);                                                                     \
    typeof(*ptr) VAL;                                                                              \
    while (true)                                                                                   \
    {                                                                                              \
        VAL = atomic_load_explicit(__PTR, memory_order_acquire);                                   \
        if (cond_expr)                                                                             \
            break;                                                                                 \
        while (atomic_load_explicit(__PTR, memory_order_relaxed) == VAL)                           \
            platform_cpu_relax();                                                                  \
    }                                                                                              \
    VAL;                                                                                           \
})


// ported from read_barrier_depends  kernel definition
#if   defined(PLATFORM_ARCH_BLACKFIN)
#   define platform_memory_barrier_read_depends()  do { barrier(); smp_check_barrier(); } while (0)
#elif defined(PLATFORM_ARCH_ALPHA)
#   define platform_memory_barrier_read_depends()  asm volatile("mb":::"memory")
#else
#   define platform_memory_barrier_read_depends()  do { } while (0)
#endif



/*
 *  TODO: для cpu_relax - Необходимо определить особое поведение для некоторых архитектур
 * ./arch/sparc/include/asm/processor_64.h:#define cpu_relax()     asm volatile("\n99:\n\t"
 * ./arch/ia64/include/asm/processor.h:#define cpu_relax() ia64_hint(ia64_hint_pause)
 *
 * ./arch/powerpc/include/asm/processor.h:#define cpu_relax() \
 *      do { HMT_low(); HMT_medium(); barrier(); } while (0)
 * ./arch/hexagon/include/asm/processor.h:#define cpu_relax() __vmyield()
 * ./arch/c6x/include/asm/processor.h:#define cpu_relax()          do { } while (0)
 */


/*
 *  TODO: нужно перенести реализацию smb_mb под каждую архитектуру. См.
 * /arch/arm64/include/asm/barrier.h
 * аналогично: /arch/mips, /arch/s390, /arch/sh/, /arch/metag/,  /arch/powerpc/, /arch/ia64/,
 * /arch/alpha/,
 * /include/asm-generic/barrier.h
 */



#endif // BARRIER_H
