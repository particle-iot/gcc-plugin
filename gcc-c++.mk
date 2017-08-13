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

# Other tools
AR = ar
AR_FLAGS += -rcs

# Naming conventions
SUFFIX_BIN ?=
SUFFIX_LIB_SHARED ?= .so
SUFFIX_LIB_STATIC ?= .a
PREFIX_LIB ?= lib

# Target name and type (bin, lib, shared-lib, static-lib)
TARGET ?= unnamed
TARGET_TYPE ?= bin
TARGET_BIN := $(TARGET:$(SUFFIX_BIN)=)$(SUFFIX_BIN)
TARGET_LIB_SHARED := $(PREFIX_LIB)$(TARGET:$(PREFIX_LIB)%=%)
TARGET_LIB_SHARED := $(TARGET_LIB_SHARED:$(SUFFIX_LIB_SHARED)=)$(SUFFIX_LIB_SHARED)
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
PCH_RELEASE += $(PCH)
PCH_DEBUG += $(PCH)

# Build directories
RELEASE_DIR = release
DEBUG_DIR = debug
BIN_DIR = bin
LIB_DIR = lib
OBJ_DIR = obj
DEP_DIR = dep

BIN_DIR_RELEASE = $(RELEASE_DIR)/$(BIN_DIR)
BIN_DIR_DEBUG = $(DEBUG_DIR)/$(BIN_DIR)
LIB_DIR_RELEASE = $(RELEASE_DIR)/$(LIB_DIR)
LIB_DIR_DEBUG = $(DEBUG_DIR)/$(LIB_DIR)
OBJ_DIR_RELEASE = $(RELEASE_DIR)/$(OBJ_DIR)
OBJ_DIR_DEBUG = $(DEBUG_DIR)/$(OBJ_DIR)
DEP_DIR_RELEASE = $(RELEASE_DIR)/$(DEP_DIR)
DEP_DIR_DEBUG = $(DEBUG_DIR)/$(DEP_DIR)

# General compiler output
OBJ_SRC_RELEASE = $(addprefix $(OBJ_DIR_RELEASE)/, $(SRC_RELEASE:=.o))
OBJ_SRC_DEBUG = $(addprefix $(OBJ_DIR_DEBUG)/, $(SRC_DEBUG:=.o))
DEP_RELEASE = $(addprefix $(DEP_DIR_RELEASE)/, $(SRC_RELEASE:=.d))
DEP_DEBUG = $(addprefix $(DEP_DIR_DEBUG)/, $(SRC_DEBUG:=.d))

CXX_FLAGS_RELEASE += $(INCLUDE_PATH_RELEASE:%=-I%) $(DEFINE_RELEASE:%=-D%)
CXX_FLAGS_DEBUG += $(INCLUDE_PATH_DEBUG:%=-I%) $(DEFINE_DEBUG:%=-D%)
LD_FLAGS_RELEASE += $(LIB_PATH_RELEASE:%=-L%) $(LIB_RELEASE:%=-l%)
LD_FLAGS_DEBUG += $(LIB_PATH_DEBUG:%=-L%) $(LIB_DEBUG:%=-l%)

# Library-specific compiler options
ifneq ($(filter $(TARGET_TYPE), lib shared-lib static-lib),)
    CXX_FLAGS += $(CXX_FLAGS_LIB)
endif

# Precompiled headers
OBJ_PCH_RELEASE = $(addprefix $(OBJ_DIR_RELEASE)/, $(PCH_RELEASE:=.gch))
OBJ_PCH_DEBUG = $(addprefix $(OBJ_DIR_DEBUG)/, $(PCH_DEBUG:=.gch))
DEP_RELEASE += $(addprefix $(DEP_DIR_RELEASE)/, $(PCH_RELEASE:=.d))
DEP_DEBUG += $(addprefix $(DEP_DIR_DEBUG)/, $(PCH_DEBUG:=.d))

INCLUDE_PATH_RELEASE := $(sort $(dir $(OBJ_PCH_RELEASE))) $(INCLUDE_PATH_RELEASE)
INCLUDE_PATH_DEBUG := $(sort $(dir $(OBJ_PCH_DEBUG))) $(INCLUDE_PATH_DEBUG)

# Directories for intermediate files
COMMON_DIR_RELEASE = $(sort $(dir $(OBJ_SRC_RELEASE)) $(dir $(OBJ_PCH_RELEASE)) $(dir $(DEP_RELEASE)))
COMMON_DIR_DEBUG = $(sort $(dir $(OBJ_SRC_DEBUG)) $(dir $(OBJ_PCH_DEBUG)) $(dir $(DEP_DEBUG)))

.PHONY: bin-prepare bin-release bin-debug lib-release lib-debug \
    shared-lib-prepare shared-lib-release shared-lib-debug \
    static-lib-prepare static-lib-release static-lib-debug \
    gen-release gen-debug prepare clean distclean all

all: debug release
release: $(TARGET_TYPE)-release
debug: $(TARGET_TYPE)-debug
prepare: $(TARGET_TYPE)-prepare
lib-release: shared-lib-release static-lib-release
lib-debug: shared-lib-debug static-lib-debug

clean:
	rm -f $(OBJ_SRC_RELEASE) $(OBJ_SRC_DEBUG) \
	    $(OBJ_PCH_RELEASE) $(OBJ_PCH_DEBUG) \
	    $(DEP_RELEASE) $(DEP_DEBUG)

distclean:
	rm -rf $(RELEASE_DIR) $(DEBUG_DIR)

# Helper rules forcing complete dependencies generation
gen-release: $(OBJ_PCH_RELEASE)
gen-debug: $(OBJ_PCH_DEBUG)

# Executable binary
bin-prepare:
	mkdir -p $(BIN_DIR_RELEASE) $(BIN_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
bin-release: bin-prepare gen-release $(BIN_DIR_RELEASE)/$(TARGET_BIN)
$(BIN_DIR_RELEASE)/$(TARGET_BIN): $(OBJ_SRC_RELEASE)
	$(LD) $^ $(LD_FLAGS_RELEASE) -o $@
bin-debug: bin-prepare gen-debug $(BIN_DIR_DEBUG)/$(TARGET_BIN)
$(BIN_DIR_DEBUG)/$(TARGET_BIN): $(OBJ_SRC_DEBUG)
	$(LD) $^ $(LD_FLAGS_DEBUG) -o $@

# Shared library
shared-lib-prepare:
	mkdir -p $(LIB_DIR_RELEASE) $(LIB_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
shared-lib-release: shared-lib-prepare gen-release $(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED)
$(LIB_DIR_RELEASE)/$(TARGET_LIB_SHARED): $(OBJ_SRC_RELEASE)
	$(LD) $^ $(LD_FLAGS_RELEASE) $(LD_FLAGS_LIB_SHARED) -o $@
shared-lib-debug: shared-lib-prepare gen-debug $(LIB_DIR_DEBUG)/$(TARGET_LIB_SHARED)
$(LIB_DIR_DEBUG)/$(TARGET_LIB_SHARED): $(OBJ_SRC_DEBUG)
	$(LD) $^ $(LD_FLAGS_DEBUG) $(LD_FLAGS_LIB_SHARED) -o $@

# Static library
static-lib-prepare:
	mkdir -p $(LIB_DIR_RELEASE) $(LIB_DIR_DEBUG) $(COMMON_DIR_RELEASE) $(COMMON_DIR_DEBUG)
static-lib-release: static-lib-prepare gen-release $(LIB_DIR_RELEASE)/$(TARGET_LIB_STATIC)
$(LIB_DIR_RELEASE)/$(TARGET_LIB_STATIC): $(OBJ_SRC_RELEASE)
	$(AR) $(AR_FLAGS) $@ $^
static-lib-debug: static-lib-prepare gen-debug $(LIB_DIR_DEBUG)/$(TARGET_LIB_STATIC)
$(LIB_DIR_DEBUG)/$(TARGET_LIB_STATIC): $(OBJ_SRC_DEBUG)
	$(AR) $(AR_FLAGS) $@ $^

# Primary object files
$(OBJ_SRC_RELEASE): $(OBJ_DIR_RELEASE)/%.o: %
	$(CXX) $(CXX_FLAGS_RELEASE) -MMD -MF $(DEP_DIR_RELEASE)/$(<:=.d) -c -o $@ $<
$(OBJ_SRC_DEBUG): $(OBJ_DIR_DEBUG)/%.o: %
	$(CXX) $(CXX_FLAGS_DEBUG) -MMD -MF $(DEP_DIR_DEBUG)/$(<:=.d) -c -o $@ $<

# Precompiled headers
$(OBJ_PCH_RELEASE): $(OBJ_DIR_RELEASE)/%.gch: %
	$(CXX) $(CXX_FLAGS_RELEASE) -MMD -MF $(DEP_DIR_RELEASE)/$(<:=.d) -c -o $@ $<
$(OBJ_PCH_DEBUG): $(OBJ_DIR_DEBUG)/%.gch: %
	$(CXX) $(CXX_FLAGS_DEBUG) -MMD -MF $(DEP_DIR_DEBUG)/$(<:=.d) -c -o $@ $<

# Generated dependencies
-include $(DEP_RELEASE)
-include $(DEP_DEBUG)
