
TARGET_NAME = roc_test

LIBS_LIST = -lpthread -lfcgi -lfcgi++ -ljsoncpp -lboost_unit_test_framework -lpq -lssl -lcrypto -lthrift

SPEC_INCPATH = src _result _result/include test

# Uncomment to validate of the correctness the vm image build process
# SPEC_INCPATH = _result _result/include build_env/opt/itcs/include
# LIBS_PATH = build_env/opt/itcs/lib

SOURCE_DIR  = test

BASE_SOURCES = \
  test_roc_parser.cpp \
  test_utils.cpp \
  notify_service_simulator.cpp


BASE_HEADERS += \
  test_utils.h \
  http_handler.h \
  notify_service_simulator.h


ROC_SOURCES = \
  roc_parser.cpp \
  uni_log_impl.cpp \
  http_handler_settings.cpp \
  roc_fcgi_utils.cpp \
  roc_methods.cpp \
  http_handler.cpp \
  roc_db_utils.cpp \
  roc_db_context.cpp \
  roc_nsms_worker.cpp \
  roc_nodename_generator.cpp


ROC_HEADERS = \
  roc_parser_dictionary.h \
  roc_fcgi_utils.h \
  json_utils.h


ROC_DIR = src


COMMON_CXXFLAGS =      \
  -std=c++11           \
  -fstack-protector    \
  -pedantic            \
  -Wall                \
  -Wextra              \
  -Wno-long-long       \
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
