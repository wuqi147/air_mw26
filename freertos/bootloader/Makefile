ifeq ($(proj),)
ifndef BOOTLOADER_CONFIG
include config.mk
else
include $(BOOTLOADER_CONFIG)
endif
else
PROJECT = $(proj)
endif

# Define toolchain based-directory
include toolchain.mk

################################################################################
all: build

################################################################################
# -rR : Do not use make's built-in rules and variables
MAKEFLAGS           += -rR --no-print-directory -j4
# Use 'make ... V=1' to see the full commands
Q                   = @
QUIET               = quiet_
ifeq ("$(origin V)","command line")
ifeq ($(V),1)
Q                   =
QUIET               =
endif
endif
export QUIET Q

################################################################################
# Use 'make proj=...' to see the full commands
PROJ                = $(PROJECT)
ifeq ($(PROJ),)
$(error "please specify the project name using 'proj=...'")
endif
export PROJ

################################################################################
# path variables
SRC_DIR             := .
APP_DIR             := $(SRC_DIR)/app
BSP_DIR             := $(SRC_DIR)/bsp
KERNEL_DIR          := $(SRC_DIR)/kernel
PROJ_DIR            := $(SRC_DIR)/project
TOOL_DIR            := $(SRC_DIR)/../tool
MAKE_DIR            := $(PROJ_DIR)/$(PROJ)/make
OUT_DIR             := $(PROJ_DIR)/$(PROJ)/build
SDK_DIR             := $(SRC_DIR)/..
LOG_FILE            := $(PROJ_DIR)/$(PROJ)/log.Makefile
TARGET              ?= airRTOSBoot
TARGET_OUT_DIR      := $(OUT_DIR)
TARGET_LIB          := $(TARGET_OUT_DIR)/built-in.a
TARGET_ELF          := $(TARGET_OUT_DIR)/$(TARGET).elf
OUT_DIR             := $(shell mkdir -p $(OUT_DIR) && cd $(OUT_DIR) && pwd)

ifeq ($(OUT_DIR),)
$(error "create output directory: $(OUT_DIR) failed")
endif

export SRC_DIR APP_DIR BSP_DIR KERNEL_DIR PROJ_DIR TOOL_DIR MAKE_DIR SDK_DIR LOG_FILE TARGET TARGET_OUT_DIR

include $(MAKE_DIR)/config.mk
include $(MAKE_DIR)/platform.mk
include $(MAKE_DIR)/common.mk
include $(MAKE_DIR)/build_include.mk
include $(MAKE_DIR)/inc_path.mk
include $(MAKE_DIR)/toolchain.mk

################################################################################
# top directory we need build
build-dirs          := $(APP_DIR)/ $(BSP_DIR)/ $(KERNEL_DIR)/ $(PROJ_DIR)/

build: $(TARGET_ELF)
	$(call cmd-only,objcopy)
	$(call cmd-only,objdump)
	$(Q)echo ============== $(PROJ) ==============
	$(Q)du -b $(TARGET).bin

$(TARGET_ELF): $(TARGET_LIB) FORCE
	$(Q)$(MAKE) -f $(MAKE_DIR)/link.mk elf=$(TARGET_ELF) lib=$(TARGET_LIB)

$(TARGET_LIB): $(build-dirs)
	$(call cmd,lib)

$(build-dirs):
	$(Q)$(MAKE) $(build)=$@


################################################################################
clean:
	$(Q)$(MAKE) -f $(MAKE_DIR)/link.mk clean
	$(call cmd,clean)

#add phony targer 'FORCE' into dependencies to force a target to run command
PHONY               += $(build-dirs) clean FORCE
FORCE:

.PHONY: $(PHONY)
.NOTPARALLEL: clean all