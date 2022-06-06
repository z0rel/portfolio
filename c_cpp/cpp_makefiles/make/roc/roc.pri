
TARGET_NAME = roc_service

LIBS_LIST = -lpthread -lfcgi -lfcgi++ -lpq -ljsoncpp -lthrift

SPEC_INCPATH = _result _result/include

# Uncomment to validate of the correctness the vm image build process
# SPEC_INCPATH = _result _result/include build_env/opt/itcs/include
# LIBS_PATH = build_env/opt/itcs/lib

SOURCE_DIR  = src

BASE_SOURCES =               \
  http_handler.cpp           \
  roc_fcgi_utils.cpp         \
  uni_log_impl.cpp           \
  roc_parser.cpp             \
  roc_methods.cpp            \
  roc_db_utils.cpp           \
  roc_db_context.cpp         \
  http_handler_settings.cpp  \
  roc_nsms_worker.cpp        \
  roc_nodename_generator.cpp \
  main.cpp


BASE_HEADERS = \
    roc_logger.h \
    roc_parser_dictionary.h \
    json_utils.h

COMMON_CXXFLAGS =      \
  -std=c++11           \
  -fstack-protector    \
  -pedantic            \
  -Wall                \
  -Wextra              \
  -Wno-long-long       \
  -Weffc++             \
  -Wold-style-cast     \
  -Wconversion         \
  -Wctor-dtor-privacy  \
  -Woverloaded-virtual \
  -Werror


COMMON_DEFINES = \
  -DITCS_PROJECT_NAME=rollout_center


THRIFT_NSMS = idl/nsms_api_1_0.thrift
THRIFT_NSMS_GEN_BASE = NSMSService nsms_api_1_0_constants nsms_api_1_0_types


THRIFT_NOTIFY = roc_notify_1_0.thrift
THRIFT_NOTIFY_GEN_BASE = NotifyService roc_notify_1_0_constants roc_notify_1_0_types
