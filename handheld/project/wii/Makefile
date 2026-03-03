# Wii (devkitPPC + libogc) target bootstrap.
# Usage: make -C handheld/project/wii

# Windows Git Bash / MSYS2 default location for devkitPro.
# Override via environment when installed elsewhere.
DEVKITPRO ?= /c/devkitPro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
WII_RULES := $(DEVKITPPC)/wii_rules

ifeq (,$(wildcard $(WII_RULES)))
all:
	@echo "devkitPPC toolchain not found."
	@echo "Set DEVKITPRO/DEVKITPPC before building, for example:"
	@echo "  export DEVKITPRO=/c/devkitPro"
	@echo "  export DEVKITPPC=\$$DEVKITPRO/devkitPPC"
	@echo "Then rerun: make -C handheld/project/wii"

clean:
	@echo "Nothing to clean (toolchain not configured)."

.PHONY: all clean

else

include $(WII_RULES)

TARGET      := minecraftwii
BUILD       := build
SOURCES_DIR := ../../src
INCLUDES    := -I$(SOURCES_DIR) -I../../lib/include -I$(LIBOGC_INC)

# Wii source selection uses an allowlist:
#   1) platform-independent engine sources
#   2) explicit Wii platform and entry files
# This prevents Windows/OpenGL translation units from being compiled.
CORE_DIRS := \
	client \
	locale \
	nbt \
	network \
	platform \
	raknet \
	server \
	util \
	world

CORE_TOPLEVEL_SRCS := \
	$(SOURCES_DIR)/NinecraftApp.cpp \
	$(SOURCES_DIR)/Performance.cpp \
	$(SOURCES_DIR)/SharedConstants.cpp

CORE_TREE_SRCS := $(foreach dir,$(CORE_DIRS),$(shell find $(SOURCES_DIR)/$(dir) -name '*.cpp'))
CORE_SRCS := $(sort $(CORE_TOPLEVEL_SRCS) $(CORE_TREE_SRCS))

# Exclude OpenGL backend units from the Wii target.
CORE_SRCS := $(filter-out \
	$(SOURCES_DIR)/client/renderer/gles.cpp \
	$(SOURCES_DIR)/util/PerfRenderer.cpp,\
	$(CORE_SRCS))

WII_SRCS := \
	$(SOURCES_DIR)/AppPlatform_wii.cpp \
	$(SOURCES_DIR)/main_wii.cpp

SOURCES := $(CORE_SRCS) $(WII_SRCS)

CXXFLAGS += -DWII -DPOSIX -DGFX_SMALLER_CHUNKS -fno-exceptions -fno-rtti $(INCLUDES)
LDFLAGS  += -L$(LIBOGC_LIB)
LIBS     := -logc -lbte -lm -lz -lfat

OBJECTS := $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

all: $(TARGET).elf

$(TARGET).elf: $(OBJECTS)
	@echo linking $@
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

$(BUILD)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf

.PHONY: all clean

endif
