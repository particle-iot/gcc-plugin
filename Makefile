TARGET = particle_plugin
TARGET_TYPE = lib-shared

# Don't add default 'lib' prefix to the plugin's target file
PREFIX_LIB =

SRC = src/logging/log_pass.cpp \
  src/logging/msg_index.cpp \
  src/logging/fmt_parser.cpp \
  src/plugin/plugin_base.cpp \
  src/plugin/gimple.cpp \
  src/plugin/tree.cpp \
  src/util/json.cpp \
  src/util/variant.cpp \
  src/plugin.cpp

INCLUDE_PATH = src

# Check access to GCC's tree nodes at runtime
DEFINE = ENABLE_TREE_CHECKING

LIB = boost_system \
  boost_filesystem

# Path to GCC plugin header files
GCC_PLUGIN_PATH ?= $(shell $(CXX) -print-file-name=plugin)
CXX_FLAGS += -isystem $(GCC_PLUGIN_PATH)/include

# GCC requires RTTI disabled for plugins
CXX_FLAGS += -fno-rtti

include gcc-c++.mk
