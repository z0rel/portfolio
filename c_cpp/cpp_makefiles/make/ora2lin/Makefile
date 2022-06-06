# Make-файл для сборки конвертера

include sources.pri

# Пути проекта                  {


INTLIB_DIR       = $(LINMODULES_DIR)/$(INTLIB_BASEDIR)
EXLIB_DIR        = $(LINMODULES_DIR)/$(EXLIB_BASEDIR)
DECIMALS_DIR     = $(LINMODULES_DIR)/$(DECIMALS_BASEDIR)
TICK_DIR         = $(LINMODULES_DIR)/$(TICK_BASEDIR)
DATETIME_DIR     = $(LINMODULES_DIR)/$(DATETIME_BASEDIR)

DESTDIR          = bin
OBJECTS_DIR      = obj
DEPENDS_DIR      = depends

#}

# Макроопределения              {

ARCH := $(shell getconf LONG_BIT)

INTERNAL_DEFINES_INTLIB_32  = $(DEFINES_INTLIB)
INTERNAL_DEFINES_INTLIB_64  = $(DEFINES_INTLIB) $(DEFINES_INTLIB_64)

INTERNAL_DEFINES_INTLIB     = $(INTERNAL_DEFINES_INTLIB_$(ARCH):%=-D%)

# }
# Флаги компиляции              {

INCPATH            = -I$(SOURCE_DIR) -I$(LEXPATH) -I$(INTLIB_DIR) -I$(EXLIB_DIR) -I$(DECIMALS_DIR) -I$(TICK_DIR) -I$(DATETIME_DIR) -I$(LINMODULES_DIR)/$(OPT_LINMODULES_DIR)
LFLAGS             = -Wl,-O0 -g 

COPTFLAGS          = -pipe -O0 -g3 
CFLAGS             = $(COPTFLAGS) $(COMMON_FLAGS) $(INTERNAL_DEFINES_INTLIB) 
CXXFLAGS           = $(CFLAGS) $(COMMON_CXX_FLAGS)
LEXFLAGS           = $(COPTFLAGS) -Wno-sign-compare -Wno-unused-function -Wno-unused-parameter $(INTERNAL_DEFINES_INTLIB) 
LINFLAGS           = $(COMMON_SUPRESS_WARNINGS_FLAGS)
CCLINMODULESFLAGS  = $(LEXFLAGS) $(LINFLAGS)
CXXLINMODULESFLAGS = $(LEXFLAGS) -Wno-format

# }

# Псевдонимы для системных команд {

CC                 = gcc
CXX                = g++
LINK               = g++
MAKE               = make
AR                 = ar cqs
RANLIB             = 
QMAKE              = /usr/bin/qmake
TAR                = tar  -cf
COMPRESS           = gzip -9f
COPY               = cp   -f
SED                = sed
COPY_FILE          = $(COPY)
COPY_DIR           = $(COPY) -r
STRIP              = strip
INSTALL_FILE       = install -m 644 -p
INSTALL_DIR        = $(COPY_DIR)
INSTALL_PROGRAM    = install -m 755 -p
DEL_FILE           = rm -f
SYMLINK            = ln -f -s
DEL_DIR            = rm -Rf
MOVE               = mv -f
CHK_DIR_EXISTS     = test -d
MKDIR              = mkdir -p
YACC               = bison
LEX                = flex

# }
# Определение базовых переменных {

TARGET           = $(DESTDIR)/$(TARGET_NAME)

SQL_LEXER_L      = $(SOURCE_DIR)/$(SQL_LEXER)
SQL_SYNTAXER_Y   = $(SOURCE_DIR)/$(SQL_SYNTAXER)
SQL_LEXER_CXX    = $(LEXPATH)/$(SQL_LEXER_BASE).cpp
SQL_LEXER_H      = $(LEXPATH)/$(SQL_LEXER_BASE).h 
SQL_SYNTAXER_CXX = $(LEXPATH)/$(SQL_SYNTAXER_BASE).cpp
SQL_SYNTAXER_H   = $(LEXPATH)/$(SQL_SYNTAXER_BASE).h

# SQL_LEXER_H1      = $(LEXPATH)/$(SQL_LEXER_BASE).l1
# SQL_SYNTAXER_H1   = $(LEXPATH)/$(SQL_SYNTAXER_BASE).l1
# SQL_LEXER_CXX1    = $(LEXPATH)/$(SQL_LEXER_BASE).c1
# SQL_SYNTAXER_CXX1 = $(LEXPATH)/$(SQL_SYNTAXER_BASE).c1


LEX_BASIS        = $(SQL_LEXER_BASE) $(SQL_SYNTAXER_BASE)
LEXFILES         = $(foreach obj, $(LEX_BASIS), $(LEXPATH)/$(obj).c1 $(LEXPATH)/$(obj).h1 $(LEXPATH)/$(obj).h $(LEXPATH)/$(obj).cpp)

SOURCES_BASIS    = $(basename $(notdir $(BASE_SOURCES)))

SRCOBJECTS       = $(foreach depfile, $(SOURCES_BASIS), $(OBJECTS_DIR)/$(depfile).o) 
LEXOBJECTS       = $(foreach depfile, $(LEX_BASIS)    , $(OBJECTS_DIR)/$(depfile).o)  
OBJECTS          = $(LEXOBJECTS) $(SRCOBJECTS) 

SRCDEPFILES      = $(foreach depfile, $(SOURCES_BASIS), $(DEPENDS_DIR)/$(depfile).d)
LEXDEPFILES      = $(foreach depfile, $(LEX_BASIS)    , $(DEPENDS_DIR)/$(depfile).lexd)  

DEPFILES         = $(SRCDEPFILES) $(LEXDEPFILES)

define basis
  $(basename $(notdir $(1)))
endef

INTLIBOBJECTS    = $(foreach depfile, $(call basis,$(INTLIBSRC))  , $(OBJECTS_DIR)/$(depfile).o)  
EXLIBOBJECTSCPP  = $(foreach depfile, $(call basis,$(EXLIBCPP))   , $(OBJECTS_DIR)/$(depfile).o)  
TICKOBJECTS      = $(foreach depfile, $(call basis,$(TICKSRC))    , $(OBJECTS_DIR)/$(depfile).o)  
DECIMALSOBJECTS  = $(foreach depfile, $(call basis,$(DECIMALSSRC)), $(OBJECTS_DIR)/$(depfile).o)  
DATETIMEOBJECTS  = $(foreach depfile, $(call basis,$(DATETIMESRC)), $(OBJECTS_DIR)/$(depfile).o) 

LINOBJECT        = $(INTLIBOBJECTS) $(EXLIBOBJECTSCPP) $(TICKOBJECTS) $(DECIMALSOBJECTS) $(DATETIMEOBJECTS)

# }
# Основные цели {

first: all

all: $(TARGET)

$(TARGET): $(OBJECTS) $(LINOBJECT) | $(DESTDIR)
	@echo $(OBJECTS)
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LINOBJECT) $(LIBS) $(SUBLIBS)

clean:  FORCE
	-$(DEL_FILE) *~ core *.core
	-$(DEL_FILE) obj/null obj/snull
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) $(LEXFILES)
	-$(DEL_FILE) $(DEPFILES)

distclean: clean
	$(DEL_FILE) $(TARGET) 
	$(DEL_FILE) $(LINOBJECT)
	$(DEL_DIR)  $(DEPENDS_DIR)
	$(DEL_DIR)  $(LEXPATH) 
	$(DEL_DIR)  $(OBJECTS_DIR)

#}
# Вспомогательные цели {

check:  FORCE
	@echo "===== depfiles ====="
	@echo $(DEPFILES)
	@echo "===== target ====="
	@echo $(TARGET)
	@echo "===== objects ====="
	@echo $(OBJECTS)
	@echo "===== srcdep ====="
	@echo $(SRCDEPFILES)
	@echo "===== lexdep ====="
	@echo $(LEXDEPFILES)
	@echo "===== INTLIB_DIR =====" 
	@echo $(INTLIB_DIR)
	@echo "===== DEFINES_INTLIB =====" 
	@echo $(INTERNAL_DEFINES_INTLIB)

depfiles: $(DEPFILES) 

# }
# Создание сборочных каталогов {

$(LEXPATH) $(DEPENDS_DIR) $(OBJECTS_DIR) $(DESTDIR):
	$(MKDIR) $@

#}
# Генерация файлов для flex и bison {


LEX_GENCMD   = $(LEX) --yylineno
YACC_GENCMD  = $(YACC) -v -d -t --graph=$(LEXPATH)/$(SQL_SYNTAXER_BASE).dot

LEX_HEADARG  = --header-file
YACC_HEADARG = --defines



# 1 h       <= $(SQL_LEXER_H)        $(LEXPATH)/$(SQL_LEXER_BASE).h  
#           <= $(SQL_SYNTAXER_H)     $(LEXPATH)/$(SQL_SYNTAXER_BASE).h 
# 2 h1      <= $(SQL_LEXER_H1)       $(LEXPATH)/$(SQL_LEXER_BASE).l1 
#           <= $(SQL_SYNTAXER_H1)    $(LEXPATH)/$(SQL_SYNTAXER_BASE).l1 
# 3 gencmd  <= $(LEX) --yylineno 
#           <= $(YACC) -v -d -t --graph=$(LEXPATH)/$(SQL_SYNTAXER_BASE).dot 
# 4 headarg <= $(LEX_HEADARG)        --header-file 
#           <= $(YACC_HEADARG)       --defines
# 5         <= $(SQL_LEXER_CXX1)     $(LEXPATH)/$(SQL_LEXER_BASE).c1    
#           <= $(SQL_SYNTAXER_CXX1)  $(LEXPATH)/$(SQL_SYNTAXER_BASE).c1 
# 6         <= $(SQL_LEXER_CXX)      $(LEXPATH)/$(SQL_LEXER_BASE).cpp 
#           <= $(SQL_SYNTAXER_CXX)   $(LEXPATH)/$(SQL_SYNTAXER_BASE).cpp 
# 7         <= $(SQL_LEXER_L)        $(SOURCE_DIR)/$(SQL_LEXER) 
#           <= $(SQL_SYNTAXER_Y)     $(SOURCE_DIR)/$(SQL_SYNTAXER) 
define lexHeadGen
	@$(3) $(4)=$(1) -o $(6) $(7)
endef

# define lexHeadGen
# 	@$(3) $(4)=$(1) -o $(6) $(7)
# 
# 	@$(3) $(4)=$(2) -o $(5) $(7)
# 	$(3) -o $(6) $(7)                       
#   @if [ -f $(1) ];                       \
#   then                                   \
#     export zzz=$$(diff '$(2)' '$(1)' | wc -m); \
#     if [ $$zzz -gt 0 ];                  \
#     then                                 \
#       $(DEL_FILE) $(1);                  \
#       $(MOVE) $(2) $(1);                 \
#     fi;                                  \
#   else                                   \
#       $(MOVE) $(2) $(1);                 \
# 	fi
# endef

LEXHEADS = $(SQL_LEXER_CXX) $(SQL_SYNTAXER_CXX)


get_cxxflags: 
	@echo -c $(CXXFLAGS)

get_includepath: 
	@echo $(INCPATH)

get_srcbase:
	@echo $(SOURCES_BASIS)


heads: 
	$(LEX_GENCMD)  $(LEX_HEADARG)=$(SQL_LEXER_H)     -o $(SQL_LEXER_CXX)    $(SQL_LEXER_L)
	$(YACC_GENCMD) $(YACC_HEADARG)=$(SQL_SYNTAXER_H) -o $(SQL_SYNTAXER_CXX) $(SQL_SYNTAXER_Y)


$(SQL_LEXER_CXX):    $(SQL_LEXER_L)    | $(LEXPATH)
	$(call lexHeadGen,$(SQL_LEXER_H),$(SQL_LEXER_H1),$(LEX_GENCMD),$(LEX_HEADARG),$(SQL_LEXER_CXX1),$(SQL_LEXER_CXX),$(SQL_LEXER_L))
	
$(SQL_SYNTAXER_CXX): $(SQL_SYNTAXER_Y) | $(LEXPATH)
	$(call lexHeadGen,$(SQL_SYNTAXER_H),$(SQL_SYNTAXER_H1),$(YACC_GENCMD),$(YACC_HEADARG),$(SQL_SYNTAXER_CXX1),$(SQL_SYNTAXER_CXX),$(SQL_SYNTAXER_Y))
#	$(DEL_FILE) $(LEXPATH)/location.hh
#	$(DEL_FILE) $(LEXPATH)/position.hh


# }
# Генерация файлов зависимостей для обычных исходников {

define dependsGenCmd
	@set -e; \
	if [ -e "$@" ]; then $(DEL_FILE) "$@"; fi; \
	$(CXX) -MM $(INCPATH) $(CXXFLAGS) $< \
	  | $(SED) 's,\($*\)\.o[ :]*,$(OBJECTS_DIR)/\1.o $(DEPENDS_DIR)/\1.$(1): ,g' > $@ 
endef

$(SRCDEPFILES): $(DEPENDS_DIR)/%.d: $(SOURCE_DIR)/%.cpp | $(LEXHEADS) $(DEPENDS_DIR)
	@$(call dependsGenCmd,d)

$(LEXDEPFILES): $(DEPENDS_DIR)/%.lexd: $(LEXPATH)/%.cpp | $(LEXHEADS) $(DEPENDS_DIR)
	@$(call dependsGenCmd,lexd)
#}
# Правила сборки объектных файлов {

$(SRCOBJECTS): $(OBJECTS_DIR)/%.o: $(SOURCE_DIR)/%.cpp      | $(LEXHEADS) $(OBJECTS_DIR) 
	$(CXX) -c $(CXXFLAGS)                  $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<
                                                                                    
$(LEXOBJECTS): $(OBJECTS_DIR)/%.o: $(LEXPATH)/%.cpp         | $(LEXHEADS) $(OBJECTS_DIR) 
	$(CXX) -c $(CXXFLAGS) $(LEXFLAGS)      $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

$(EXLIBOBJECTSCPP): $(OBJECTS_DIR)/%.o: $(EXLIB_DIR)/%.cpp  | $(OBJECTS_DIR)
	$(CXX) -c $(CXXFLAGS) $(CXXLINMODULESFLAGS) $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

$(INTLIBOBJECTS): $(OBJECTS_DIR)/%.o: $(INTLIB_DIR)/%.c       | $(OBJECTS_DIR)
	$(CC)  -c $(CFLAGS) $(CCLINMODULESFLAGS)  $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

$(DECIMALSOBJECTS): $(OBJECTS_DIR)/%.o: $(DECIMALS_DIR)/%.c | $(OBJECTS_DIR)
	$(CC)  -c $(CFLAGS) $(CCLINMODULESFLAGS)  $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

$(TICKOBJECTS): $(OBJECTS_DIR)/%.o: $(TICK_DIR)/%.c         | $(OBJECTS_DIR)
	$(CC)  -c $(CFLAGS) $(CCLINMODULESFLAGS)  $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

$(DATETIMEOBJECTS): $(OBJECTS_DIR)/%.o: $(DATETIME_DIR)/%.c         | $(OBJECTS_DIR)
	$(CC)  -c $(CFLAGS) $(CCLINMODULESFLAGS)  $(INCPATH) -o $(OBJECTS_DIR)/$(*F).o $<

#}
# Ограничения на генерацию зависимостей для некоторых целей {
# Когда создаются эти цели - не включать зависимости
NODEPS:=clean distclean tags svn check heads
# Не создавать зависимости, когда выполняется очистка или вспомогательная цель
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS)))) 
  # Скорее всего - эти файлы не существуют. 
  # Make создаст их и очистит впоследствии.
  -include $(DEPFILES) 
endif 

#}

FORCE:

# vim:foldmethod=marker:foldmarker={,}
