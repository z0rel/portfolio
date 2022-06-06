#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include "platform/architecture.h"
#include "scheduler/coro.h"

#include "scheduler/src/coro_arch.h"


#if !defined(PLATFORM_OS_WIN_LIKE)
#   include <unistd.h>
#   if defined(_POSIX_MAPPED_FILES) && _POSIX_MAPPED_FILES
#       include <sys/mman.h>
#       if defined(MAP_ANONYMOUS) || defined(MAP_ANON)
#           include "coro_alloc.h"
#           define CORO_ALLOC_MMAP 1
#       endif
#   endif
#endif


#if !defined(CORO_ALLOC_MMAP)
#   include "coro_malloc.h"
#endif

