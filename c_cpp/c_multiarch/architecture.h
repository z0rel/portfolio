/**
 * @file
 * @brief Единообразное определение платформ
 */
#ifndef PLATFORM_ARCH_H
#define PLATFORM_ARCH_H


#if    defined(__arm__)             || defined(__arm__) || defined(__TARGET_ARCH_ARM)              \
    || defined(__TARGET_ARCH_THUMB) || defined(_ARM)    || defined(_M_ARM) || defined(_M_ARMT)     \
    || defined(__arm)               || defined(__aarch64__)

#   define PLATFORM_ARCH_ARM      1

#   if defined(__aarch64__)
#       define PLATFORM_ARCH_AARCH64 1
#   endif

#   if  defined(__ARM_ARCH_6__)  || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__)           \
     || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__) || defined(__ARM_ARCH_6T2__)

#       define PLATFORM_ARCH_ARM_VERSION 6

#   elif  defined(__ARM_ARCH_7__)  || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)         \
       || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__) \
       || (defined(__ARM_ARCH) && (__ARM_ARCH == 7))

#       define PLATFORM_ARCH_ARM_VERSION 7

#   else
#       error "unknown arm version"
#   endif


#elif  defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)          \
    || defined(_M_X64)    || defined(_M_AMD64)


#if defined(__amd64__) || defined(__amd64)
#   define  PLATFORM_ARCH_X86_64_GNU_C 1
#endif

#if defined(_M_X64) || defined(_M_AMD64)
#   define  PLATFORM_ARCH_X86_64_MSVC 1
#endif

#   define  PLATFORM_ARCH_X86_LIKE 1
#   define  PLATFORM_ARCH_X86_64 1

#elif  defined(i386)      || defined(__i386)   || defined(__i386__)                                \
    || defined(__i486__)  || defined(__i586__) || defined(__i686__)      || defined(__i386)        \
    || defined(__i386)    || defined(__IA32__) || defined(_M_I86)        || defined(_M_IX86)       \
    || defined(__X86__)   || defined(_X86_)    || defined(__THW_INTEL__) || defined(__I86__)       \
    || defined(__INTEL__) || defined(__386)

#   if defined(i386)      || defined(__i386)   || defined(__i386__) || defined(__i386)             \
    || defined(__i386)
#   define  PLATFORM_ARCH_X86_I386_LIKE 1
#   endif

#   define  PLATFORM_ARCH_X86_LIKE 1
#   define  PLATFORM_ARCH_X86_32 1

#elif  defined(__ia64__) || defined(_IA64)   || defined(__IA64__) || defined(__ia64)               \
    || defined(_M_IA64)  || defined(_M_IA64) || defined(_M_IA64)  || defined(__itanium__)

#   define  PLATFORM_ARCH_IA64 1

#elif  defined(__mips__) || defined(mips) || defined(_R3000) || defined(_R4000) || defined(_R5900) \
    || defined(__mips)   || defined(__mips) || defined(__MIPS__)

#   define  PLATFORM_ARCH_MIPS 1

#elif  defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)

#   define  PLATFORM_ARCH_ALPHA 1

#elif  defined(__bfin) || defined(__BFIN__)

#   define  PLATFORM_ARCH_BLACKFIN 1

#endif

#if defined(__ORDER_LITTLE_ENDIAN__)
#   define PLATFORM_ORDER_LITTLE_ENDIAN 1
#endif


#if defined(__SSE__)
#   define PLATFORM_CPU_SSE
#endif


#if defined(_WIN16) ||  defined(_WIN32) ||  defined(_WIN64) ||  defined(__WIN32__)                 \
    || defined(__TOS_WIN__) || defined(__WINDOWS__)
#   define PLATFORM_OS_WIN 1
#   define PLATFORM_OS_WIN_LIKE 1
#elif defined(macintosh) || defined(Macintosh) || defined(__APPLE__)
#   define PLATFORM_OS_APPLE 1
#endif


#if defined(__CYGWIN__)
#   define PLATFORM_OS_CYGWIN 1
#   define PLATFORM_OS_WIN_LIKE 1
#endif


#endif // PLATFORM_ARCH_H
