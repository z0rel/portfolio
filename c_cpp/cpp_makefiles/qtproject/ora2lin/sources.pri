# Файл с параметрами, общими и для qmake и для make

TARGET_NAME = ora2lin

# Исходники                     {
BASE_SOURCES = anydata.cpp                 \
               codegenerator.cpp           \
               codegen_function.cpp        \
               codestream.cpp              \
               config_converter.cpp        \
               crossplatform_func.cpp      \
               dependenciesstruct.cpp      \
               error_codes.cpp             \
               formal_translator.cpp       \
               lexsubtokenizer.cpp         \
               linter_package_add_data.cpp \
               lwriter.cpp                 \
               main.cpp                    \
               model_context.cpp           \
               model_statistic.cpp         \
               object_type.cpp             \
               ora2call.cpp                \
               ora2text.cpp                \
               parserdfa.cpp               \
               resolved_entity.cpp         \
               resolvers.cpp               \
               semantic_base.cpp           \
               semantic_collection.cpp     \
               semantic_expr.cpp           \
               semantic_expr_select.cpp    \
               semantic_function.cpp       \
               semantic_id.cpp             \
               semantic_plsql.cpp          \
               semantic_relations.cpp      \
               semantic_statements.cpp     \
               semantic_table.cpp          \
               semantic_tree.cpp           \
               semantic_types.cpp          \
               smart_lexer.cpp             \
               smartptr.cpp                \
               syntaxer_context.cpp        \
               system_datatype.cpp         \
               system_functions.cpp        \
               test_resolvers.cpp          \
               xmltype.cpp                 \
               dynamic_sql.cpp             \
               dynamic_sql_op.cpp		       \
               sorted_statistic.cpp        \
               codestream_indenter.cpp     \
               blobtype.cpp                \
               lindb.cpp                   \
               lindumper.cpp               \
               lindumper_lowlev.cpp        \
               sentence_transformer.cpp


# Заголовочные файлы для QtCreator.
# Добавлять только те, для которых отстутствуют .с или .cpp файлы в каком-либо подкаталоге проекта.
# Заголовочные файлы, для которых существует одноименный .c или .cpp файл - подтягиваются
# QtCreator-ом автоматически (с помощью функции addSrcHeader в файле ora2lin.pro)

SRC_HEADERS =  app_conf.h               \
               codespacer.h             \
               constants.h              \
               dstring.h                \
               hash_fun.h               \
               hash_table.h             \
               hashtable_internal.h     \
               i2str.h                  \
               lexlib.h                 \
               lexlib_internal.h        \
               lex_location.h           \
               lex_position.h           \
               model_head.h             \
               parserdfa_internal.h     \
               project_optimization.h   \
               semantic_blockplsql.h    \
               semantic_datatype.h      \
               semantic_flags.h         \
               semantic_id_lists.h      \
               semantic_object.h        \
               semantic_sql.h           \
               semantic_utility.h       \
               siter.h                  \
               smartptr_internal.h      \
               smartptr_union.h         \
               statements_tree.h        \
               synlib.h                 \
               syntaxer_external.h      \
               syntaxer_union.h         \
               system_sysuser.h         \
               usercontext.h            \
               dynamic_sql.h            \
               sorted_statistic.h       \
               yylex_decl.h             \
               codestream_indenter.h    \
               blobtype.h               \
               lindb.h                  \
               lindumper.h              \
               blobtype.h               \
               depworkflow.h            \
               dynamic_sql.h            \
               dynamic_sql_op.h         \
               lindumper_lowlev.h       \
               lexlib_internal_header.h \
               sentence_transformer.h



SQL_LEXER        = lexer.l
SQL_SYNTAXER     = syntaxer.y

SQL_LEXER_BASE   = sql_lexer_lex
SQL_SYNTAXER_BASE= sql_syntaxer_bison

#}


# Исходники библиотеки intlib    {
EXLIBCPP           = exlib.cpp experr.cpp
INTLIBSRC          = encode.c  int64.c  intlib.c  lnlsutf.c viewtext.c  # linctrl.c
# }
# Исходники библиотеки decimals {
DECIMALSSRC        = dadd.c    dcmp.c     dcopy.c  ddiv.c     \
                     dentd.c   dfromdbl.c dfroml.c dfromstr.c \
                     dminmax.c dmuld.c    dnegd.c  dokstat.c  \
                     dround.c  dsetstat.c dstat.c  dsubd.c    \
                     dtodbl.c  dtol.c     dtostr.c dectbl.c
# }
# Исходники библиотеки tick     {
TICKSRC            = tadd.c     tdateday.c tdatetic.c tday.c     \
                     tdaynum.c  tformat.c  tname.c    tstrtick.c \
                     ttickdat.c ttickstr.c tweekday.c
# }
# Исходники библиотеки datetime {
DATETIMESRC        = dttots.c tstostr.c data.c data1.c tsfrdy.c test.c \
                     strtots.c tsaddhou.c tsvalid.c tsdays.c strupr.c 
# }

SOURCE_DIR         = src
LEXPATH            = flex_bison
LINMODULES_DIR     = linmodules

OPT_LINMODULES_DIR = opt
EXLIB_BASEDIR      = exlib
INTLIB_BASEDIR     = intlib
DECIMALS_BASEDIR   = decimals
TICK_BASEDIR       = tick
DATETIME_BASEDIR   = datetime

# общие флаги для intlib
DEFINES_INTLIB     = ENGLISH                \
                     LARGE_EXCHANGE         \
                     LINUX                  \
                     LIN_PTHREAD            \
                     NO_ATFORK              \
                     SOCKETS                \
                     SUPP_LONGLONG          \
                     SUPP_POLL              \
                     SVR4                   \
                     SVR42                  \
                     UNIX                   \
                     _ENGLISH               \
                     _LARGEFILE_SOURCE      \
                     _USED_TERM_H           \
                     _VER_MAX=610           \
                     _notrus


# флаги, необходимые для 64-х разрядной сборки
DEFINES_INTLIB_64  = _LARGEFILE64_SOURCE \
                     __64BIT_ARCH__



COMMON_FLAGS     = -Wall -W -D_REENTRANT -Wno-deprecated  
COMMON_SUPRESS_WARNINGS_FLAGS = -Wno-unused-value                  \
                                -Wno-format                        \
                                -Wno-missing-field-initializers    \
                                -Wno-unused-but-set-variable       \
                                -Wno-sign-compare                  \
                                -Wno-unused-variable               \
                                -Wno-missing-braces                \
                                -Wno-implicit-function-declaration \
                                -Wno-return-type                   \
                                -Wno-implicit-function-declaration \
                                -Wno-pointer-to-int-cast           \
                                -Wno-uninitialized                 \
                                -Wno-switch                        \
                                -Wno-unused-parameter              \
                                -Wno-pointer-sign                  \
                                -Wno-char-subscripts               \
                                -Wno-old-style-declaration         \
                                -Wno-unused-parameter              \
                                -Wno-return-type                   \
                                -Wno-unused-function               \
                                -Wno-unused-parameter              \
                                -Wno-type-limits                   \
                                -Wno-parentheses                   \
                                -Wno-shift-overflow                \
                                -Wno-shift-count-overflow          


COMMON_CXX_FLAGS = -std=gnu++11

LIBS             = -lpthread -lgmp


# vim:foldmethod=marker:foldmarker={,}:syntax=make: 

HEADERS += \
    $$PWD/src/syntaxer_internal.h
