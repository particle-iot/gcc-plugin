TARGET = particle_plugin
TARGET_TYPE = lib-shared

GCC_PLUGIN_PATH = $(shell $(CXX) -print-file-name=plugin)
CXX_FLAGS += -isystem $(GCC_PLUGIN_PATH)/include

SRC = src/util/string.cpp \
	src/plugin.cpp

INCLUDE_PATH = src

include gcc-c++.mk
