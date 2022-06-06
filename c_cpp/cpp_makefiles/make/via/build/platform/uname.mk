ifeq ($(SHELL_UNAME),)
    SHELL_UNAME = $(shell $(UNAME))
endif

uname = $(if $(or $(findstring Windows,$(SHELL_UNAME)),\
                  $(findstring MINGW32,$(SHELL_UNAME)),\
                  $(findstring MINGW64,$(SHELL_UNAME)),\
                  $(findstring MSYS,$(SHELL_UNAME))\
              ),Windows,$(if $(findstring Linux,$(SHELL_UNAME)),Linux,Unknown))

