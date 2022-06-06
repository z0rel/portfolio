TEMPLATE = app

# TEMPLATE = lib
CONFIG += plugin


TARGET   = behrens_gparser
CONFIG   += debug

QT       -= xml widgets core

# PYTHON_VERSION = 3.6m
# PYTHON_VERSION_BOOST = 3.6

unix {
    PYTHON_VERSION = 3.4m
    # PYTHON_VERSION_BOOST = 3.4
    PYLIBPATH =
    LIBS += -lstdc++fs -lpython$$PYTHON_VERSION # -lboost_python-$$PYTHON_VERSION_BOOST
}
win32 {
    PYTHON_VERSION = 37
    # PYTHON_VERSION_BOOST = 3.7
    PYLIBPATH = C:/Users/Win10/AppData/Local/Programs/Python/Python37
    INCLUDEPATH += ./pybind11/include
    DEPENDPATH += ./pybind11/include
    LIBS += -lstdc++fs -L $$PYLIBPATH -lpython$$PYTHON_VERSION # -lboost_python-$$PYTHON_VERSION_BOOST
}



unix {
  INCLUDEPATH *= /usr/include/python$$PYTHON_VERSION
  QMAKE_POST_LINK = ln -s libbehrens_gparser.so behrens_gparser.so
}

OBJECTS_DIR = $$OUT_PWD/tmp
MOC_DIR     = $$OUT_PWD/tmp
UI_DIR      = $$OUT_PWD/tmp
RCC_DIR     = $$OUT_PWD/tmp
DESTDIR     = $$OUT_PWD/bin

QMAKE_PRE_LINK = rm -f behrens_gparser.so
QMAKE_DISTCLEAN += behrens_gparser.so

gcc {
  QMAKE_CXXFLAGS += -std=c++17
}


unix {

DEPENDPATH +=  \
  /usr/lib64/gcc/x86_64-pc-linux-gnu/5.4.0/include/g++-v5 \
  /usr/include

BISONCMD = bison
FLEXCMD = flex

}


win32 {

DEPENDPATH +=  \
  C:/Users/Win10/AppData/Local/Programs/Python/Python37/include
INCLUDEPATH += \
  C:/Users/Win10/AppData/Local/Programs/Python/Python37/include

  BISONCMD = C:/ProgramData/chocolatey/lib/winflexbison3/tools/win_bison.exe
  FLEXCMD = C:/ProgramData/chocolatey/lib/winflexbison3/tools/win_flex.exe
}




GENERATED_PARSER_PATH = $$OUT_PWD/gen_parser

INCLUDEPATH += \
  $$_PRO_FILE_PWD_/src/compiler \
  $$_PRO_FILE_PWD_  \
  $$_PRO_FILE_PWD_/src \
  $$_PRO_FILE_PWD_/pybind11/include \
  $$GENERATED_PARSER_PATH


DEPENDPATH += \
  $$_PRO_FILE_PWD_/src/compiler \
  $$_PRO_FILE_PWD_/src \
  $$GENERATED_PARSER_PATH


# TRANSLATIONS += resources/tr/expert_ru.ts
CODECFORTR  = utf8
# RESOURCES += resources/app_resources.qrc
QMAKE_RESOURCE_FLAGS += -threshold 0 -compress 9

gcc {
  WARN_FLAGS = -Wall -Wno-unused-parameter -Wno-unused-function
}


QMAKE_CFLAGS   += $$WARN_FLAGS
QMAKE_CXXFLAGS += $$WARN_FLAGS


GEN_LEXER_SRC       = $$GENERATED_PARSER_PATH/gen_flp_lexer.cpp
GEN_LEXER_HEADER    = $$GENERATED_PARSER_PATH/gen_flp_lexer.h
GEN_PARSER_SRC      = $$GENERATED_PARSER_PATH/gen_flp_parser.cpp
GEN_PARSER_HEADER   = $$GENERATED_PARSER_PATH/gen_flp_parser.h


FLEXSOURCES  = src/compiler/compiler.l
BISONSOURCES = src/compiler/compiler.y

OTHER_FILES += \
  $$BISONSOURCES \
  $$FLEXSOURCES \
  $$GEN_LEXER_SRC \
  $$GEN_LEXER_HEADER \
  $$GEN_PARSER_SRC \
  $$GEN_PARSER_HEADER

bison.commands           = $$BISONCMD -d -t -g ${QMAKE_FILE_IN} --defines=$$GEN_PARSER_HEADER -o $$GEN_PARSER_SRC
bison.input              = BISONSOURCES
bison.output             = $$GEN_PARSER_SRC
bison.variable_out       = SOURCES
bison.name               = bison
bison.CONFIG            += target_predeps

bisonheader.commands     = @true
bisonheader.input        = BISONSOURCES
bisonheader.output       = $$GEN_PARSER_HEADER
bisonheader.variable_out = HEADERS
bisonheader.name         = bison header
bisonheader.depends      = $$GEN_PARSER_SRC

flex.commands            = $$FLEXCMD --header-file=$$GEN_LEXER_HEADER --outfile=$$GEN_LEXER_SRC ${QMAKE_FILE_IN}
flex.input               = FLEXSOURCES
flex.output              = $$GEN_LEXER_SRC
flex.variable_out        = SOURCES
flex.name                = flex
flex.CONFIG             += target_predeps

flexheader.commands      = @true
flexheader.input         = FLEXSOURCES
flexheader.output        = $$GEN_LEXER_HEADER
flexheader.variable_out  = HEADERS
flexheader.name          = flex header
flexheader.depends       = $$GEN_LEXER_SRC


QMAKE_EXTRA_COMPILERS += flex
QMAKE_EXTRA_COMPILERS += flexheader
QMAKE_EXTRA_COMPILERS += bison
QMAKE_EXTRA_COMPILERS += bisonheader

SOURCES += \
  src/main.cpp \
    src/compiler/token_table.cpp \
    src/compiler/python_module.cpp


defineReplace(addSrcHeader) {
   srcs  = $$1
   restrict_srcs = $$find(srcs, .*c(pp)?$)
   new_hdrs =
   for(a, restrict_srcs) {
      x_tmp      = $$basename(a)
      x_dir      = $$dirname(a)
      new_hdr    = $$replace(x_tmp, \\.c(pp)?$, .h)
      hdr = $${x_dir}/$${new_hdr}
      exists( $$hdr ) {
        new_hdrs += $$hdr
      }
   }
   return($$new_hdrs)
}


HEADERS += \
  $$addSrcHeader($$SOURCES) \
  src/compiler/lex_location.h \
  src/compiler/parser_union.h \
  src/compiler/smartptr.h\
  src/compiler/compiler_global.h \
    src/compiler/token_table.h \
    src/compiler/transformcontext.h \
    src/compiler/python_model.h

