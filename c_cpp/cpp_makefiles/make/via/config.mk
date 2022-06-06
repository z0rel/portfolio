include $(TOPDIR)/build/platform/uname.mk

# basic parameteres
PLATFORM = $(call uname)
COMPILER = gcc
BUILD    = release

# default is empty and decided by compiler, possible 64 or 32
ABI = #

include $(TOPDIR)/build/common.mk

# debug and optmization
ifeq ($(strip $(BUILD)),release)
   OPTIMIZE     = $(DEBUG) $(cc_O3) $(cc_Wall) $(cc_LTO) $(cc_O3)
   OPTIMIZE_LTO = $(cc_DEBUG) $(cc_LTO) $(cc_O3)
else
   ifeq ($(strip $(BUILD)),profile)
       OPTIMIZE     = $(cc_DEBUG) $(cc_O3) $(cc_Wall) -fno-inline -g3
       OPTIMIZE_LTO = $(cc_DEBUG)
	 else
       OPTIMIZE     = $(DEBUG) $(cc_Wall)
       OPTIMIZE_LTO = $(cc_DEBUG)
	 endif
endif


