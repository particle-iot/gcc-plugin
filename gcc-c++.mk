# C++ compiler
CXX = g++
CXX_FLAGS += -std=c++11 -Wall -pipe
CXX_FLAGS_RELEASE += -O2 -DNDEBUG $(CXX_FLAGS)
CXX_FLAGS_DEBUG += -O0 -g3 $(CXX_FLAGS)
CXX_FLAGS_LIB = -fPIC

# Linker (invoked via compiler command)
LD = $(CXX)
LD_FLAGS += $(CXX_FLAGS)
LD_FLAGS_RELEASE += $(LD_FLAGS)
LD_FLAGS_DEBUG += $(LD_FLAGS)
LD_FLAGS_LIB_SHARED = -shared

# Protobuf compiler
PROTOC = protoc
PROTOC_FLAGS +=
PROTOC_FLAGS_RELEASE += $(PROTOC_FLAGS)
PROTOC_FLAGS_DEBUG += $(PROTOC_FLAGS)

# Other tools
AR = ar
AR_FLAGS += -rcs
OBJCOPY = objcopy
OBJCOPY_FLAGS += --only-keep-debug
STRIP = strip
STRIP_FLAGS += --strip-debug --strip-unneeded

# Naming conventions
SUFFIX_BIN =
SUFFIX_LIB_SHARED = .so
SUFFIX_LIB_STATIC = .a
SUFFIX_DEBUG = .debug
PREFIX_LIB = lib

# Target name and type (bin, lib, lib-shared, lib-static)
TARGET ?= unnamed
TARGET_TYPE ?= bin
TARGET_BIN := $(TARGET:$(SUFFIX_BIN)=)$(SUFFIX_BIN)
TARGET_BIN_DEBUG := $(TARGET_BIN)$(SUFFIX_DEBUG)
TARGET_LIB_SHARED := $(PREFIX_LIB)$(TARGET:$(PREFIX_LIB)%=%)
TARGET_LIB_SHARED := $(TARGET_LIB_SHARED:$(SUFFIX_LIB_SHARED)=)$(SUFFIX_LIB_SHARED)
TARGET_LIB_SHARED_DEBUG := $(TARGET_LIB_SHARED)$(SUFFIX_DEBUG)
TARGET_LIB_STATIC := $(PREFIX_LIB)$(TARGET:$(PREFIX_LIB)%=%)
TARGET_LIB_STATIC := $(TARGET_LIB_STATIC:$(SUFFIX_LIB_STATIC)=)$(SUFFIX_LIB_STATIC)

# Sources and dependencies
SRC_RELEASE += $(SRC)
SRC_DEBUG += $(SRC)
INCLUDE_PATH_RELEASE += $(INCLUDE_PATH)
INCLUDE_PATH_DEBUG += $(INCLUDE_PATH)
LIB_RELEASE += $(LIB)
LIB_DEBUG += $(LIB)
LIB_PATH_RELEASE += $(LIB_PATH)
LIB_PATH_DEBUG += $(LIB_PATH)
DEFINE_RELEASE += $(DEFINE)
DEFINE_DEBUG += $(DEFINE)
PRECOMP_HEADER_RELEASE += $(PRECOMP_HEADER)
PRECOMP_HEADER_DEBUG += $(PRECOMP_HEADER)
PROTO_RELEASE += $(PROTO)
PROTO_DEBUG += $(PROTO)
PROTO_PATH_RELEASE += $(PROTO_PATH)
PROTO_PATH_DEBUG += $(PROTO_PATH)

# Build directories
RELEASE_DIR = release
DEBUG_DIR = debug
BIN_DIR = bin
LIB_DIR = lib
OBJ_DIR = obj
DEP_DIR = dep
GEN_DIR = gen

BIN_DIR_RELEASE = $(RELEASE_DIR)/$(BIN_DIR)
BIN_DIR_DEBUG = $(DEBUG_DIR)/$(BIN_DIR)
LIB_DIR_RELEASE = $(RELEASE_DIR)/$(LIB_DIR)
LIB_DIR_DEBUG = $(DEBUG_DIR)/$(LIB_DIR)
OBJ_DIR_RELEASE = $(RELEASE_DIR)/$(OBJ_DIR)
OBJ_DIR_DEBUG = $(DEBUG_DIR)/$(OBJ_DIR)
DEP_DIR_RELEASE = $(RELEASE_DIR)/$(DEP_DIR)
DEP_DIR_DEBUG = $(DEBUG_DIR)/$(DEP_DIR)
GEN_DIR_RELEASE = $(RELEASE_DIR)/$(GEN_DIR)
GEN_DIR_DEBUG = $(DEBUG_DIR)/$(GEN_DIR)

# General compiler output
OBJ_SRC_RELEASE = $(addprefix $(OBJ_DIR_RELEASE)/, $(SRC_RELEASE:=.o))
OBJ_SRC_DEBUG = $(addprefix $(OBJ_DIR_DEBUG)/, $(SRC_DEBUG:=.o))
DEP_RELEASE = $(addprefix $(DEP_DIR_RELEASE)/, $(SRC_RELEASE:=.d))
DEP_DEBUG = $(addprefix $(DEP_DIR_DEBUG)/, $(SRC_DEBUG:=.d))

CXX_FLAGS_RELEASE += $(INCLUDE_PATH_RELEASE:%=-I%) $(DEFINE_RELEASE:%=-D%)
CXX_FLAGS_DEBUG += $(INCLUDE_PATH_DEBUG:%=-I%) $(DEFINE_DEBUG:%=-D%)
LD_FLAGS_RELEASE += $(LIB_PATH_RELEASE:%=-L%) $(LIB_RELEASE:%=-l%)
LD_FLAGS_DEBUG += $(LIB_PATH_DEBUG:%=-L%) $(LIB_DEBUG:%=-l%)

INCLUDE_PATH_RELEASE := $(GEN_DIR_RELEASE) $(INCLUDE_PATH_RELEASE)
INCLUDE_PATH_DEBUG := $(GEN_DIR_DEBUG) $(INCLUDE_PATH_DEBUG)

# Library-specific compiler options
ifneq ($(filter $(TARGET_TYPE), lib lib-shared lib-static),)
    CXX_FLAGS += $(CXX_FLAGS_LIB)
endif

# Protobuf compiler output
OBJ_PROTO_RELEASE = $(addprefix $(OBJ_DIR_RELEASE)/, $(PROTO_RELEASE:.proto=.pb.cc.o))
OBJ_PROTO_DEBUG = $(addprefix $(OBJ_DIR_DEBUG)/, $(PROTO_DEBUG:.proto=.pb.cc.o))
GEN_PROTO_CC_RELEASE = $(addprefix $(GEN_DIR_RELEASE)/, $(PROTO_RELEASE:.proto=.pb.cc))
GEN_PROTO_CC_DEBUG = $(addprefix $(GEN_DIR_DEBUG)/, $(PROTO_DEBUG:.proto=.pb.cc))
GEN_PROTO_H_RELEASE = $(addprefix $(GEN_DIR_RELEASE)/, $(PROTO_RELEASE:.proto=.pb.h))
GEN_PROTO_H_DEBUG = $(addprefix $(GEN_DIR_DEBUG)/, $(PROTO_DEBUG:.proto=.pb.h))

PROTOC_FLAGS_RELEASE += $(PROTO_PATH_RELEASE:%=-I%)
PROTOC_FLAGS_DEBUG += $(PROTO_PATH_DEBUG:%=-I%)

INCLUDE_PATH_RELEASE := $(addprefix $(GEN_DIR_RELEASE)/, $(PROTO_PATH_RELEASE)) $(INCLUDE_PATH_RELEASE)
INCLUDE_PATH_DEBUG := $(addprefix $(GEN_DIR_DEBUG)/, $(PROTO_PATH_DEBUG)) $(INCLUDE_PATH_DEBUG)

# Precompiled headers
OBJ_PRECOMP_HEADER_RELEASE = $(addprefix $(OBJ_DIR_RELEASE)/, $(PRECOMP_HEADER_RELEASE:=.gch))
OBJ_PRECOMP_HEADER_DEBUG = $(addprefix $(OBJ_DIR_DEBUG)/, $(PRECOMP_HEADER_DEBUG:=.gch))
DEP_RELEASE += $(addprefix $(DEP_DIR_RELEASE)/, $(PRECOMP_HEADER_RELEASE:=.d))
DEP_DEBUG += $(addprefix $(DEP_DIR_DEBUG)/, $(PRECOMP_HEADER_DEBUG:=.d))

INCLUDE_PATH_RELEASE := $(sort $(dir $(OBJ_PRECOMP_HEADER_RELEASE))) $(INCLUDE_PATH_RELEASE)
INCLUDE_PATH_DEBUG := $(sort $(dir $(OBJ_PRECOMP_HEADER_DEBUG))) $(INCLUDE_PATH_DEBUG)

# Directories for intermediate files
COMMON_DIR_RELEASE = $(sort $(dir $(OBJ_SRC_RELEASE)) $(dir $(OBJ_PRECOMP_HEADER_RELEASE)) \
     $(dir $(OBJ_PROTO_RELEASE)) $(dir $(GEN_PROTO_H_RELEASE)) $(dir $(GEN_PROTO_CC_RELEASE)) \
     $(dir $(DEP_RELEASE)))
COMMON_DIR_DEBUG = $(sort $(dir $(OBJ_SRC_DEBUG)) $(dir $(OBJ_PRECOMP_HEADER_DEBUG)) \
     $(dir $(OBJ_PROTO_DEBUG)) $(dir $(GEN_PROTO_H_DEBUG)) $(dir $(GEN_PROTO_CC_DEBUG)) \
     $(dir $(DEP_DEBUG)))

.PHONY: bin-prepare bin-release bin-debug lib-release lib-debug \
    lib-shared-prepare lib-shared-release lib-shared-debug \
    lib-static-prepare lib-static-release lib-static-debug \
    gen-release gen-debug prepare clean distclean all

all: debug release
release: $(TARGET_TYPE)-release
debug: $(TARGET_TYPE)-debug
prepare: $(TARGET_TYPE)-prepare
lib-release: lib-shared-release lib-static-release
lib-debug: lib-shared-debug lib-static-debug

clean:
	@rm -f $(OBJ_SRC_RELEASE) $(OBJ_SRC_DEBUG) \
	    $(OBJ_PRECOMP_HEADER_RELEASE) $(OBJ_PRECOMP_HEADER_DEBUG) \
	    $(OBJ_PROTO_RELEASE) $(OBJ_PROTO_DEBUG) \
	    $(GEN_PROTO_CC_RELEASE) $(GEN_PROTO_CC_DEBUG) \
	    $(GEN_PROTO_H_RELEASE) $(GEN_PROTO_H_DEBUG) \
	    $(DEP_RELEASE) $(DEP_DEBUG)

distclean:
	@rm -rf $(RELEASE_DIR) $(DEBUG_DIR)

# Helper rules forcing complete dependencies generation prior to the build
gen-release: $(GEN_PROTO_CC_RELEASE) $(GEN_PROTO_H_RELEASE) $(OBJ_PRECOMP_HEADER_RELEASE)
gen-debug: $(GEN_PROTO_CC_DEBUG) $(GEN_PROTO_H_DEBUG) $(OBJ_PRECOMP_HEADER_DEBUG)

# Executable binary
bin-prepare:
	@mkdir -p $(BIN_DIR_RELEASE) $(BIN_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
bin-release: bin-prepare gen-release $(BIN_DIR_RELEASE)/$(TARGET_BIN)
$(BIN_DIR_RELEASE)/$(TARGET_BIN): $(OBJ_SRC_RELEASE) $(OBJ_PROTO_RELEASE)
	$(LD) $^ $(LD_FLAGS_RELEASE) -o $@
	$(OBJCOPY) $(OBJCOPY_FLAGS) $@ $(BIN_DIR_RELEASE)/$(TARGET_BIN_DEBUG)
	@chmod a-x $(BIN_DIR_RELEASE)/$(TARGET_BIN_DEBUG)
	$(STRIP) $(STRIP_FLAGS) $@
bin-debug: bin-prepare gen-debug $(BIN_DIR_DEBUG)/$(TARGET_BIN)
$(BIN_DIR_DEBUG)/$(TARGET_BIN): $(OBJ_SRC_DEBUG) $(OBJ_PROTO_DEBUG)
	$(LD) $^ $(LD_FLAGS_DEBUG) -o $@

# Shared library
lib-shared-prepare:
	@mkdir -p $(LIB_DIR_RELEASE) $(LIB_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
lib-shared-release: lib-shared-prepare gen-release $(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED)
$(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED): $(OBJ_SRC_RELEASE) $(OBJ_PROTO_RELEASE)
	$(LD) $^ $(LD_FLAGS_RELEASE) $(LD_FLAGS_LIB_SHARED) -o $@
	$(OBJCOPY) $(OBJCOPY_FLAGS) $@ $(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED_DEBUG)
	@chmod a-x $(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED_DEBUG)
	$(STRIP) $(STRIP_FLAGS) $@
lib-shared-debug: lib-shared-prepare gen-debug $(LIB_DIR_DEBUG)/$(TARGET_LIB_SHARED)
$(LIB_DIR_DEBUG)/$(TARGET_LIB_SHARED): $(OBJ_SRC_DEBUG) $(OBJ_PROTO_DEBUG)
	$(LD) $^ $(LD_FLAGS_DEBUG) $(LD_FLAGS_LIB_SHARED) -o $@

# Static library
lib-static-prepare:
	@mkdir -p $(LIB_DIR_RELEASE) $(LIB_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
lib-static-release: lib-static-prepare gen-release $(LIB_DIR_RELEASE)/$(TARGET_LIB_STATIC)
$(LIB_DIR_RELEASE)/$(TARGET_LIB_STATIC): $(OBJ_SRC_RELEASE) $(OBJ_PROTO_RELEASE)
	$(AR) $(AR_FLAGS) $@ $^
lib-static-debug: lib-static-prepare gen-debug $(LIB_DIR_DEBUG)/$(TARGET_LIB_STATIC)
$(LIB_DIR_DEBUG)/$(TARGET_LIB_STATIC): $(OBJ_SRC_DEBUG) $(OBJ_PROTO_DEBUG)
	$(AR) $(AR_FLAGS) $@ $^

# Primary object files
$(OBJ_SRC_RELEASE): $(OBJ_DIR_RELEASE)/%.o: %
	$(CXX) $(CXX_FLAGS_RELEASE) -MMD -MF $(DEP_DIR_RELEASE)/$(<:=.d) -c -o $@ $<
$(OBJ_SRC_DEBUG): $(OBJ_DIR_DEBUG)/%.o: %
	$(CXX) $(CXX_FLAGS_DEBUG) -MMD -MF $(DEP_DIR_DEBUG)/$(<:=.d) -c -o $@ $<

# Precompiled headers
$(OBJ_PRECOMP_HEADER_RELEASE): $(OBJ_DIR_RELEASE)/%.gch: %
	$(CXX) $(CXX_FLAGS_RELEASE) -MMD -MF $(DEP_DIR_RELEASE)/$(<:=.d) -c -o $@ $<
$(OBJ_PRECOMP_HEADER_DEBUG): $(OBJ_DIR_DEBUG)/%.gch: %
	$(CXX) $(CXX_FLAGS_DEBUG) -MMD -MF $(DEP_DIR_DEBUG)/$(<:=.d) -c -o $@ $<

# Protocol files
$(OBJ_PROTO_RELEASE): $(OBJ_DIR_RELEASE)/%.o: $(GEN_DIR_RELEASE)/%
	$(CXX) $(CXX_FLAGS_RELEASE) -c -o $@ $<
$(GEN_PROTO_H_RELEASE): $(GEN_DIR_RELEASE)/%.pb.h: %.proto
	$(PROTOC) --proto_path=$(dir $<) $(PROTOC_FLAGS_RELEASE) --cpp_out=$(dir $@) $<
$(GEN_PROTO_CC_RELEASE): %.pb.cc: %.pb.h

$(OBJ_PROTO_DEBUG): $(OBJ_DIR_DEBUG)/%.o: $(GEN_DIR_DEBUG)/%
	$(CXX) $(CXX_FLAGS_DEBUG) -c -o $@ $<
$(GEN_PROTO_H_DEBUG): $(GEN_DIR_DEBUG)/%.pb.h: %.proto
	$(PROTOC) --proto_path=$(dir $<) $(PROTOC_FLAGS_DEBUG) --cpp_out=$(dir $@) $<
$(GEN_PROTO_CC_DEBUG): %.pb.cc: %.pb.h

# Include automatic prerequisites
-include $(DEP_RELEASE)
-include $(DEP_DEBUG)
