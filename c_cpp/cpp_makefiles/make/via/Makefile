TOPDIR = $(abspath .)

ifeq ($(SHELL_UNAME),)
    SHELL_UNAME = $(shell $(UNAME))
endif


ALL_COMPILERS = clang gcc 
ALL_BUILDS    = release debug profile
ALL_ABIS      = 64 32 

#ALL_COMPILERS = clang 
#ALL_BUILDS    = release 
#ALL_ABIS      = 64


config_args = $(foreach compiler,$(ALL_COMPILERS),\
                  $(foreach build,$(ALL_BUILDS),\
                      $(foreach abi,$(ALL_ABIS),\
                          COMPILER=$(compiler).BUILD=$(build).ABI=$(abi).$(1))))

ALL_CONFIGS_DISTCLEAN = $(call config_args,distclean)
ALL_CONFIGS_ALL = $(call config_args,all)
ALL_CONFIGS_SCHED_PERFORMANCE_TEST = $(call config_args,scheduler_performance_test)


TOP_NOTPARALLEL = $(ALL_CONFIGS_DISTCLEAN)

include $(TOPDIR)/config.mk

ifeq ($(PLATFORM),Windows)
    export SHELL_UNAME
    export PATH:=$(COMPILER_ENVPATH):$(PATH)
endif


.PHONY: $(MODULES_ALL) $(MODULES_CLEAN) $(MODULES_DISTCLEAN) \
        $(ALL_CONFIGS_DISTCLEAN) \
				$(ALL_CONFIGS_ALL) \
				$(ALL_CONFIGS_SCHED_PERFORMANCE_TEST) \
				all_platform.distclean all_platform.all all_scheduler.performance \
        scheduler.tests scheduler.performance


# list of modules
MODULES = scheduler platform vmspace lpoll iox coipc example

# list of taragets for all
MODULES_ALL = $(MODULES:%=%.all)

# list of taragets for clean
MODULES_CLEAN = $(MODULES:%=%.clean)

# list of taragets for distclean
MODULES_DISTCLEAN = $(MODULES:%=%.distclean)



all: $(MODULES_ALL)


scheduler.all: platform.all vmspace.all lpoll.all

vmspace.all: platform.all

coipc.all: scheduler.all

iox.all: coipc.all lpoll.all

clean: $(MODULES_CLEAN)


distclean: $(MODULES_DISTCLEAN)
	-$(RMDIR) $(BIN_DIR)
	-$(RMDIR) $(LIB_DIR)
	-$(RMDIR) $(TARGET_DIR)


$(MODULES_ALL): %.all: %
	$(MAKE) -C $< all


$(MODULES_CLEAN): %.clean: %
	$(MAKE) -C $< clean


$(MODULES_DISTCLEAN): %.distclean: %
	$(MAKE) -C $< distclean


$(ALL_CONFIGS_DISTCLEAN) $(ALL_CONFIGS_ALL) $(ALL_CONFIGS_SCHED_PERFORMANCE_TEST):
	$(MAKE) $(shell echo $@ | sed "s/\./ /g")


# test for all possible configs
scheduler_performance_test:
	  scheduler/tests/find_statistics.py --ntimes=5 \
			--result_fname=perftest_$(BUILD)_$(COMPILER)_$(PLATFORM)$(ABI).json \
			--logfile=perftest_$(BUILD)_$(COMPILER)_$(PLATFORM)$(ABI).log \
			--bin_coroutines=$(BIN_DIR)/test_find_coroutines --bin_systhreads=$(BIN_DIR)/test_find_systhread 


# consistance test
scheduler.tests: 
	  scheduler/tests/test_all.py 

# test for manual example
scheduler.performance: 
	  scheduler/tests/find_statistics.py --ntimes=5 --result_fname=perftest_gcc64.json --logfile=perftest_gcc64.log \
			--bin_coroutines=release_gcc_Linux64/bin/test_find_coroutines --bin_systhreads=release_gcc_Linux64/bin/test_find_systhread 


all_scheduler.performance: $(ALL_CONFIGS_SCHED_PERFORMANCE_TEST)
all_platform.distclean: $(ALL_CONFIGS_DISTCLEAN)
all_platform.all: $(ALL_CONFIGS_ALL)
