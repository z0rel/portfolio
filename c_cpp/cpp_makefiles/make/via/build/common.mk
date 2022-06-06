SED    = sed
RM     = rm -f
RMDIR  = rmdir
MKDIR  = mkdir -p
UNAME  = uname
AR     = ar
ARFLAGS= rv
RANLIB = ranlib
EXPORT = export

strip_concat = $($(strip $(1))$($(strip $(2))))

include $(TOPDIR)/build/platform/$(strip $(PLATFORM)).mk
include $(TOPDIR)/build/compiler/$(strip $(COMPILER)).mk

.DEFAULT_GOAL := all

TARGET_DIR = $(TOPDIR)/$(BUILD)_$(strip $(COMPILER))_$(strip $(PLATFORM))$(strip $(ABI))
BIN_DIR = $(TARGET_DIR)/bin
LIB_DIR = $(TARGET_DIR)/lib

ifeq ($(strip $(BUILD)),release)
    # RELEASE
    DEBUG  = -DNDEBUG
else
    ifeq ($(strip $(BUILD)),profile)
	      # profile
        DEBUG  = -DNDEBUG
		else
        # DEBUG
        DEBUG  = $(cc_DEBUG) -DDEBUG
	  endif
endif

.SUFFIXES:

.PHONY: all clean distclean test

.NOTPARALLEL: clean all distclean $(TOP_NOTPARALLEL)

$(TARGET_DIR)/bin     \
$(TARGET_DIR)/lib     \
$(TARGET_DIR)/include:
	$(MKDIR) $@
