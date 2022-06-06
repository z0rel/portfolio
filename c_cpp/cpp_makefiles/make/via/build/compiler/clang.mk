
ifeq ($(PLATFORM),Linux)
    CLANG_C11_FLAG   = -std=gnu11 -fno-exceptions
    ifneq ($(call strip_concat,LINUX_CLANG_PATH,ABI),)
        COMPILER_PATH = $(call strip_concat,LINUX_CLANG_PATH,ABI)
        COMPILER_INCPATH = $(call strip_concat,LINUX_CLANG_INCPATH,ABI)
        COMPILER_ENVPATH = $(call strip_concat,LINUX_CLANG_ENVPATH,ABI)
        COMPILER_LIBS    = $(call strip_concat,LINUX_CLANG_LIBS,ABI)
    endif
endif


ifeq ($(PLATFORM),Windows)
    CLANG_C11_FLAG   = -std=c11 -fno-exceptions

    WINDOWS_CLANG_PATH32    ?= /mingw32/bin
    WINDOWS_CLANG_PATH64    ?= /mingw64/bin
    WINDOWS_CLANG_PATH      ?= /mingw64/bin
    WINDOWS_CLANG_ENVPATH32 ?= /mingw32/bin
    WINDOWS_CLANG_ENVPATH64 ?= /mingw64/bin
    WINDOWS_CLANG_ENVPATH   ?= /mingw64/bin

    ifneq ($(call strip_concat,WINDOWS_CLANG_PATH,ABI),)
        COMPILER_PATH = $(call strip_concat,WINDOWS_CLANG_PATH,ABI)
        COMPILER_INCPATH = $(call strip_concat,WINDOWS_CLANG_INCPATH,ABI)
        COMPILER_ENVPATH = $(call strip_concat,WINDOWS_CLANG_ENVPATH,ABI)
        COMPILER_LIBS    = $(call strip_concat,WINDOWS_CLANG_LIBS,ABI)
    endif
endif


ifeq ($(CLANG_CMD),)
    ifeq ($(COMPILER_PATH),)
        CLANG_CMD = clang
    else
        CLANG_CMD = $(COMPILER_PATH)/clang
    endif
endif


ifeq ($(CC_CMD),)
    CC_CMD = $(CLANG_CMD)
endif

 
cc_Wall =                         \
  -Wall                           \
  -Wnewline-eof                   \
  -Wexit-time-destructors         \
  -Wglobal-constructors           \
  -Wshadow                        \
  -Wmissing-prototypes            \
  -Wimplicit-function-declaration \
  -Wpadded
#  -Wunused-macros
#  -Weverything


cc_O3    = -O3 -fno-strict-aliasing
cc_O2    = -O2 -fno-strict-aliasing
cc_O0    = -O0
cc_LTO   = -flto
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


CC = $(CC_CMD) $(cc_ABI) $(CLANG_C11_FLAG) $(COMPILER_INCPATH)
LINK = $(CC) $(COMPILER_LIBS)

MKDEP = $(CC) -MM
MKDEPFIX = $(SED) -e 's|.*: |$@: |' | $(SED) -e '2,200 s|.*: |   |'

