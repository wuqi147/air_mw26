ifndef FREERTOS_CONFIG
include config.mk
else
include $(FREERTOS_CONFIG)
endif

# Define toolchain based-directory
include toolchain.mk

TOPDIR              := $(shell pwd)/
BUILD_DIR            = $(shell pwd)/project/$(PROJECT)/
AIR_SDK_DIR         := $(TOPDIR)/../air_system
LOG_FILE            := $(BUILD_DIR)/log.Makefile

include $(BUILD_DIR)/make/config.mk
include $(TOPDIR)/project/make/directory.mk
include	$(BUILD_DIR)/make/common.mk
include	$(BUILD_DIR)/make/platform.mk
include $(BUILD_DIR)/make/toolchain.mk
include $(TOPDIR)/project/make/include_path.mk
# MW include
ifneq ("$(wildcard $(AIR_MW_DIR))", "")
ifdef CMW_CONFIG
include $(CMW_CONFIG)
else
include $(AIR_MW_PROJECT)/make/config.mk
endif
include $(AIR_MW_SYSTEM)/make/common.mk
CFLAGS += $(MW_CFLAGS)
endif

OUTDIR              := $(BUILD_DIR)/build
SRCDIR              := $(TOPDIR)
MAKEFLAGS           := --no-print-directory -rR -j16

TARGET              ?= airRTOSSystem
TARGET_SRCDIR       := $(SRCDIR)/app/ $(SRCDIR)/bsp/ $(SRCDIR)/kernel/ $(SRCDIR)/project/
TARGET_OUTDIR       := $(OUTDIR)/
TARGET_LIB          := $(TARGET_OUTDIR)/built-in.a
TARGET_ELF          := $(TARGET_OUTDIR)/$(TARGET).elf
TARGET_BIN          := $(TARGET_OUTDIR)/$(TARGET).bin
TARGET_IMG          := $(TARGET_OUTDIR)/$(TARGET).img
TARGET_LD           := $(TOPDIR)/project/$(PROJECT)/linker_script.ld

LIB_LIST            := $(AIR_SDK_DIR)/build/image/air_sdk.a

CFLAGS += $(EXTRA_FLAGS)
PHONY               := all build clean

all: build
	$(Q)cp $(TARGET_IMG) $(TOPDIR); cp $(TARGET_BIN) $(TOPDIR); cp $(TARGET_ELF) $(TOPDIR)
	$(Q)echo ============== $(PROJECT) ==============
	$(Q)du -b $(TARGET).img

build: $(TARGET_BIN)
	$(Q)gcc -o $(TOPDIR)/tool/fwheader $(TOPDIR)/tool/fwheader.c
	$(Q)$(TOPDIR)/tool/fwheader $< $(TARGET_IMG) >>/dev/null 2>&1

$(TARGET_BIN): $(TARGET_ELF)
	$(Q)$(OBJCOPY) -O binary -S $< $@

$(TARGET_ELF): lib
	$(Q)$(MAKE) $(link) elf=$(TARGET_ELF) lib=$(TARGET_LIB)

PHONY += $(TARGET_SRCDIR)
lib: $(TARGET_LIB) FORCE

$(TARGET_LIB): $(TARGET_SRCDIR)
	$(Q)rm -f $@; $(AR) cDPrSsT $@ $(addsuffix built-in.a, $(patsubst $(SRCDIR)/%, $(OUTDIR)/%, $^))

$(TARGET_SRCDIR): mw_system FORCE
	$(Q)$(MAKE) $(build)=$@

mw_system: air_sdk
ifeq ("$(wildcard $(AIR_MW_DIR))", "")
	echo "Not find air_mw_system"
else
	echo "Find air_mw_system"
	$(Q)$(MAKE) -C $(AIR_MW_DIR) clean all
endif

air_sdk: FORCE
ifeq ("$(wildcard $(AIR_MW_DIR))", "")
	$(Q)$(MAKE) -C $(AIR_SDK_DIR) compile
else
	$(Q)$(MAKE) -C $(AIR_SDK_DIR) compile "AIR_EN_SDK_REF=DISABLED"
endif

clean:
	$(Q)rm -rf $(OUTDIR) $(LOG_FILE) $(TARGET_LD) $(TOPDIR)/tool/fwheader *.img *.bin *.elf
	$(Q)$(MAKE) -C $(AIR_SDK_DIR) clean

PHONY += FORCE
FORCE:

.PHONY: $(PHONY)
.NOTPARALLEL: all clean
