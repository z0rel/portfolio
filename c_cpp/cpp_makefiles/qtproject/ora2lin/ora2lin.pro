TEMPLATE = app
LANGUAGE = C++
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QT -= core
QT -= gui

include( sources.pri )

TARGET = $$TARGET_NAME

SRC_PATH         = $$_PRO_FILE_PWD_/$$SOURCE_DIR
LINMODULES_PATH  = $$_PRO_FILE_PWD_/$$LINMODULES_DIR
INTLIB_PATH      = $$LINMODULES_PATH/$$INTLIB_BASEDIR
EXTLIB_PATH      = $$LINMODULES_PATH/$$EXLIB_BASEDIR
DECIMALS_PATH    = $$LINMODULES_PATH/$$DECIMALS_BASEDIR
TICK_PATH        = $$LINMODULES_PATH/$$TICK_BASEDIR
DATETIME_PATH    = $$LINMODULES_PATH/$$DATETIME_BASEDIR

HDR_FIND_PATHS = $$SRC_PATH $$INTLIB_PATH $$DECIMALS_PATH $$TICK_PATH $$DATETIME_PATH $$LINMODULES_PATH/opt

for(a, EXLIBCPP    ):SOURCES += $$EXTLIB_PATH/$${a}
for(a, INTLIBSRC   ):SOURCES += $$INTLIB_PATH/$${a}
for(a, DECIMALSSRC ):SOURCES += $$DECIMALS_PATH/$${a}
for(a, TICKSRC     ):SOURCES += $$TICK_PATH/$${a}
for(a, DATETIMESRC ):SOURCES += $$DATETIME_PATH/$${a}
for(a, BASE_SOURCES):SOURCES += $$SRC_PATH/$${a}
for(a, SRC_HEADERS ):HEADERS += $$SRC_PATH/$${a}

defineReplace(addSrcHeader) {
   srcs = $$1
   paths = $$2
   restrict_srcs = $$find(srcs, .*c(pp)?$)
   new_hdrs =
   for(a, restrict_srcs) {
      x_tmp      = $$basename(a)
      new_hdr    = $$replace(x_tmp, \\.c(pp)?$, .h)
      for(p, paths) {
        hdr = $${p}/$${new_hdr}
        exists( $$hdr ) {
          new_hdrs += $$hdr
        }
      }
   }
   return($$new_hdrs)
}

HEADERS += $$addSrcHeader($$SOURCES, $$HDR_FIND_PATHS)

MAKEFILE = Makefile_qmake # !!! Не менять, иначе потрется основной Makefile, нужный для сборки без qmake !!!

INCLUDEPATH *= $$EXTLIB_PATH $$INTLIB_PATH $$DECIMALS_PATH $$TICK_PATH $$SRC_PATH $$DATETIME_PATH $$LINMODULES_PATH/$$OPT_LINMODULES_DIR



IS_GCC   = $$find(QMAKE_CC, gcc)
IS_CLANG = $$find(QMAKE_CC, clang)
!isEmpty(IS_GCC): OPTIMIZE_FLAGS *= -foptimize-strlen -finline-functions-called-once -fmerge-constants -fdevirtualize -free # -pg

QMAKE_CXXFLAGS     +=  $$COMMON_FLAGS $$COMMON_CXX_FLAGS $$OPTIMIZE_FLAGS
QMAKE_CFLAGS       +=  $$COMMON_FLAGS $$OPTIMIZE_FLAGS
QMAKE_CFLAGS_DEBUG +=  $$COMMON_FLAGS $$OPTIMIZE_FLAGS

!isEmpty(IS_GCC) {
  QMAKE_CFLAGS       += $$COMMON_SUPRESS_WARNINGS_FLAGS
  QMAKE_CFLAGS_DEBUG += $$COMMON_SUPRESS_WARNINGS_FLAGS
}

!isEmpty(IS_CLANG) {
  QMAKE_CFLAGS   += -fsanitize=address
  QMAKE_CXXFLAGS += -fsanitize=address
  QMAKE_LFLAGS   += -fsanitize=address

  exists($$_PRO_FILE_PWD_/distcc_conf.pri) {
    QMAKE_CC  = distcc clang
    QMAKE_CXX = distcc clang
  }
}

isEmpty(IS_CLANG) {
  # QMAKE_CC = distcc, QMAKE_CXX = distcc - если данный файл существует
  exists($$_PRO_FILE_PWD_/distcc_conf.pri): include($$_PRO_FILE_PWD_/distcc_conf.pri)
}



win32 {
  OBJECTS_DIR = win/tmp
  MOC_DIR     = win/tmp
  UI_DIR      = win/tmp
  RCC_DIR     = win/tmp
  DESTDIR     = win
  DEFINES *= "_VER_MAX=600" "VERSION=600" INTER_MSWINDOWS WIN32  NO_EXCEPTION
}

unix {
  OBJECTS_DIR = linux/tmp
  MOC_DIR     = linux/tmp
  UI_DIR      = linux/tmp
  RCC_DIR     = linux/tmp
  DESTDIR     = linux

  DEFINES    *= $$DEFINES_INTLIB

  ARCH        = $$system(getconf LONG_BIT)
  contains(QMAKE_CFLAGS, -m32): ARCH = 32
  eval(ARCH = 64):DEFINES *= $$DEFINES_INTLIB_64
}


FLEXSOURCES  = $$SRC_PATH/$$SQL_LEXER
BISONSOURCES = $$SRC_PATH/$$SQL_SYNTAXER

OTHER_FILES += $$FLEXSOURCES $$BISONSOURCES
exists($$_PRO_FILE_PWD_/bin/ora2lin_cfg.py) {
  OTHER_FILES += $$_PRO_FILE_PWD_/bin/ora2lin_cfg.py
}

flexsource.input    = FLEXSOURCES
flexsource.output   = sql_lexer_lex.cpp
flexsource.commands = flex --header-file=sql_lexer_lex.h -o sql_lexer_lex.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += flexsource

flexheader.input = FLEXSOURCES
flexheader.output = sql_lexer_lex.h
flexheader.commands = @true
flexheader.variable_out = HEADERS
flexheader.name = Flex Headers ${QMAKE_FILE_IN}
flexheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += flexheader

bisonsource.input = BISONSOURCES
bisonsource.output = sql_syntaxer_bison.cpp
bisonsource.commands = bison -v -x -t -d --defines=sql_syntaxer_bison.h -o sql_syntaxer_bison.cpp ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps

QMAKE_EXTRA_COMPILERS += bisonsource

bisonheader.input = BISONSOURCES
bisonheader.output = sql_syntaxer_bison.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += bisonheader

DEPENDPATH  *= $$OUT_PWD
INCLUDEPATH *= $$OUT_PWD

OTH_EXPRS = h cpp
LEX_SYN = $$SQL_LEXER_BASE $$SQL_SYNTAXER_BASE

for(a, LEX_SYN) {
  for(b, OTH_EXPRS):OTHER_FILES += $$OUT_PWD/$${a}.$${b}
  HEADERS *= $$OUT_PWD/$${a}.h
}

OTHER_FILES += .gitignore
