TARGET = particle_plugin
TARGET_TYPE = shared-lib

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

# GCC requires RTTI disabled for plugins
CXX_FLAGS += -fno-rtti

# Path to GCC plugin header files
GCC_PLUGIN_INCLUDE_PATH ?= $(shell $(CXX) -print-file-name=plugin)/include
CXX_FLAGS += -isystem $(GCC_PLUGIN_INCLUDE_PATH)

# Other dependencies
INCLUDE_PATH += $(BOOST_INCLUDE_PATH) $(RAPIDJSON_INCLUDE_PATH)
LIB_PATH += $(BOOST_LIB_PATH)

# Ignore undefined symbols (macOS)
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LD_FLAGS += -Wl,-flat_namespace -Wl,-undefined,suppress
endif

include gcc-c++.mk
