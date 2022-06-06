
ifeq ($(PLATFORM),Linux)
    GCC_C11_FLAG   = -std=gnu11
    ifneq ($(call strip_concat,LINUX_GCC_PATH,ABI),)
        COMPILER_PATH = $(call strip_concat,LINUX_GCC_PATH,ABI)
        COMPILER_INCPATH = $(call strip_concat,LINUX_GCC_INCPATH,ABI)
        COMPILER_ENVPATH = $(call strip_concat,LINUX_GCC_ENVPATH,ABI)
        COMPILER_LIBS    = $(call strip_concat,LINUX_GCC_LIBS,ABI)
    endif
endif

ifeq ($(PLATFORM),Windows)
    GCC_C11_FLAG   = -std=c11

    WINDOWS_GCC_PATH32    ?= /mingw32/bin
    WINDOWS_GCC_PATH64    ?= /mingw64/bin
    WINDOWS_GCC_PATH      ?= /mingw64/bin
    WINDOWS_GCC_ENVPATH32 ?= /mingw32/bin
    WINDOWS_GCC_ENVPATH64 ?= /mingw64/bin
    WINDOWS_GCC_ENVPATH   ?= /mingw64/bin

    ifneq ($(call strip_concat,WINDOWS_GCC_PATH,ABI),)
        COMPILER_PATH = $(call strip_concat,WINDOWS_GCC_PATH,ABI)
        COMPILER_INCPATH = $(call strip_concat,WINDOWS_GCC_INCPATH,ABI)
        COMPILER_ENVPATH = $(call strip_concat,WINDOWS_GCC_ENVPATH,ABI)
        COMPILER_LIBS    = $(call strip_concat,WINDOWS_GCC_LIBS,ABI)
    endif
endif

ifeq ($(GCC_CMD),)
    ifeq ($(COMPILER_PATH),)
        GCC_CMD = gcc
        AR = gcc-ar
        NM = gcc-ar
    else
        GCC_CMD = $(COMPILER_PATH)/gcc
        AR = $(COMPILER_PATH)gcc-ar
				NM = $(COMPILER_PATH)gcc-nm
    endif
endif

# for lto
RANLIB = /bin/true


ifeq ($(CC_CMD),)
    CC_CMD = $(GCC_CMD)
endif


cc_Wall =                         \
  -Wall                           \
  -Wvariadic-macros               \
  -Wshadow                        \
  -Wimplicit-function-declaration \
  -Wmissing-prototypes
# -Weverything 
# -Wunused-macros \

cc_O3    = -O3 -fno-strict-aliasing
cc_O2    = -O2 -fno-strict-aliasing
cc_O0    = -O0
cc_LTO   = -flto #-ffat-lto-objects
cc_DEBUG = -g3


ifeq ($(ABI),64)
    cc_ABI = -m64
endif


ifeq ($(ABI),32)
    cc_ABI = -m32
endif


ifeq ($(ABI),)
    cc_ABI = #default
endif

CC   = $(CC_CMD) $(cc_ABI) $(GCC_C11_FLAG)
LINK = $(CC) $(COMPILER_LIBS)

MKDEP = $(CC) -MM
MKDEPFIX = $(SED) -e 's|.*: |$@: |' | $(SED) -e '2,200 s|.*: |   |'

