# handheld/project/wii/Makefile
# Wii (devkitPPC + libogc) target bootstrap — updated to use wii_rules and whitelist sources.
# Usage: run inside devkitPro MINGW64 shell:
#   cd /e/code/0.6.1mcW-main
#   make -C handheld/project/wii

DEVKITPRO ?= /c/devkitPro
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
WII_RULES := $(DEVKITPPC)/wii_rules

ifeq (,$(wildcard $(WII_RULES)))
all:
	@echo "devkitPPC wii_rules not found at '$(WII_RULES)'."
	@echo "Make sure you're running the devkitPro MINGW64 shell and DEVKITPRO/DEVKITPPC are set."
	@echo "Example:"
	@echo "  export DEVKITPRO=/c/devkitPro"
	@echo "  export DEVKITPPC=\$$DEVKITPRO/devkitPPC"
	@echo "Then rerun: make -C handheld/project/wii"
clean:
	@echo "Nothing to clean (wii_rules not configured)."
.PHONY: all clean

else

include $(WII_RULES)

# Target artifact names
TARGET      := boot
BUILD       := build
SOURCES_DIR := ../../src
# Wii is GX-only; do not pull OpenGL ES headers/libraries into this target.
INCLUDES    := -I$(SOURCES_DIR) -I$(LIBOGC_INC)

CXXFLAGS += -DWII -D__WII__ -DRENDER_BACKEND_GX -DPOSIX -fno-exceptions -fno-rtti $(INCLUDES)
LDFLAGS  += -L$(LIBOGC_LIB)
LIBS     := -logc -lbte -lm -lz -lfat

# CORE_SOURCES: everything except platform entry points and desktop GL backends
CORE_SOURCES := $(shell find $(SOURCES_DIR) -name '*.cpp' \
	! -name 'AppPlatform_*.cpp' \
	! -name 'main_*.cpp' \
	! -name '*_win32.cpp' \
	! -name 'render_compat.cpp' \
	! -name 'renderer_gl.cpp' \
)

# Platform and backend files we explicitly want for Wii
PLATFORM_SOURCES := \
	$(SOURCES_DIR)/AppPlatform_wii.cpp \
	$(SOURCES_DIR)/main_wii.cpp \
	$(SOURCES_DIR)/client/renderer/RenderBackend.cpp \
	$(SOURCES_DIR)/client/renderer/renderer_gx.cpp

SOURCES := $(CORE_SOURCES) $(PLATFORM_SOURCES)

# Convert source list to object list under build/
OBJECTS := $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

all: $(TARGET).dol

# Link ELF then convert to DOL
$(TARGET).elf: $(OBJECTS)
	@echo linking $@
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)
	@echo converting $@ -> $(TARGET).dol
	elf2dol $@ $(TARGET).dol

# Make default target the DOL file produced by elf2dol
$(TARGET).dol: $(TARGET).elf
	@echo "Built $@"

# Object compile rule (keeps source tree structure under build/)
$(BUILD)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean
clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol

.PHONY: all clean

endif
