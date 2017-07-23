TARGET = particle_plugin
TARGET_TYPE = lib-shared

SRC = src/logging/log_pass.cpp \
	src/plugin/plugin_base.cpp \
	src/plugin/tree.cpp \
	src/util/variant.cpp \
	src/plugin.cpp

INCLUDE_PATH = src

# Check access to the GCC tree structures at runtime
DEFINE = ENABLE_TREE_CHECKING

# Path to GCC plugin header files
GCC_PLUGIN_PATH ?= $(shell $(CXX) -print-file-name=plugin)
CXX_FLAGS += -isystem $(GCC_PLUGIN_PATH)/include

# GCC requires RTTI disabled for plugins
CXX_FLAGS += -fno-rtti

include gcc-c++.mk
