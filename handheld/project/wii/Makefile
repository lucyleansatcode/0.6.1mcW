# Wii (devkitPPC + libogc) target bootstrap.
# Usage: make -C handheld/project/wii

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

TARGET      := boot
BUILD       := build
SOURCES_DIR := ../../src
INCLUDES    := -I$(SOURCES_DIR) -I../../lib/include -I$(LIBOGC_INC)

# Wii build intentionally compiles only GX backend sources and excludes GL backends.
EXCLUDED_SOURCES := \
	$(SOURCES_DIR)/client/renderer/gles.cpp \
	$(SOURCES_DIR)/client/renderer/renderer_gl.cpp

SOURCES := \
	$(SOURCES_DIR)/main_wii.cpp \
	$(SOURCES_DIR)/client/renderer/RenderBackend.cpp \
	$(SOURCES_DIR)/client/renderer/renderer_gx.cpp

CXXFLAGS += -DWII -D__WII__ -DPOSIX -fno-exceptions -fno-rtti $(INCLUDES)
LDFLAGS  += -L$(LIBOGC_LIB)
LIBS     := -logc -lbte -lm -lz -lfat

OBJECTS := $(patsubst $(SOURCES_DIR)/%.cpp,$(BUILD)/%.o,$(SOURCES))

all: $(TARGET).dol

$(TARGET).elf: $(OBJECTS)
	@echo linking $@
	$(LD) $(OBJECTS) -o $@ $(LDFLAGS) $(LIBS)

$(TARGET).dol: $(TARGET).elf
	@echo converting $< to $@
	elf2dol $< $@

$(BUILD)/%.o: $(SOURCES_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol

.PHONY: all clean

endif
