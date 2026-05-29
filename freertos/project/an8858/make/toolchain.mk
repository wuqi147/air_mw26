export # export all symbol
################################################################################
# toolchain position
################################################################################
CROSS_COMPILE_PATH ?= /mtkeda/airoha/enb/toolchain/andes/nds32le-elf-mculib-v3j/bin/
CROSS_COMPILE      := nds32le-elf-

CC                 := $(CROSS_COMPILE)gcc
OBJDUMP            := $(CROSS_COMPILE)objdump
OBJCOPY            := $(CROSS_COMPILE)objcopy
AR                 := $(CROSS_COMPILE)ar
AS                 := $(CROSS_COMPILE)as

DEBUG = 0
ifeq ($(DEBUG),1)
	OPTIM   := -O0 -g3 -mno-ex9 -mno-ifc
else
	OPTIM   := -Os -g -mno-ex9 -mno-ifc
endif

CFLAGS ?= -Wall $(OPTIM) \
          -fomit-frame-pointer -fno-strict-aliasing -fno-builtin -funroll-loops \
          -ffunction-sections -fdata-sections

TEST_PATH          := test -d
MKDIR              := mkdir -p

PATH               := $(CROSS_COMPILE_PATH):$(PATH)

TOOLCHAIN_INC_PATH := $(CROSS_COMPILE_PATH)/../lib/gcc/nds32le-elf/4.9.4/include
