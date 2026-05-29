export # export all symbol
################################################################################
# toolchain position
################################################################################
CROSS_COMPILE_PATH := $(HOME)/nds32le-elf-mculib-v3j/bin/

################################################################################
# configure compile flags
################################################################################
CFLAGS := -Wall -Os -g -mno-ex9 -mno-ifc \
          -fomit-frame-pointer -fno-strict-aliasing -fno-builtin -funroll-loops \
          -ffunction-sections -fdata-sections
