TARGET = particle_plugin
TARGET_TYPE = lib-shared

SRC = src/plugin/plugin_base.cpp \
	src/plugin/location.cpp \
	src/plugin/tree.cpp \
	src/util/variant.cpp \
	src/plugin.cpp

INCLUDE_PATH = src

# Check access to the GCC tree structures at runtime
DEFINE += ENABLE_TREE_CHECKING

GCC_PLUGIN_PATH ?= $(shell $(CXX) -print-file-name=plugin)
CXX_FLAGS += -isystem $(GCC_PLUGIN_PATH)/include

include gcc-c++.mk
