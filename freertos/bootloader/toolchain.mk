export # export all symbol
################################################################################
# toolchain position
################################################################################
CROSS_COMPILE_PATH := $(HOME)/nds32le-elf-mculib-v3j/bin/

################################################################################
# configure compile flags
################################################################################
CFLAGS              := -Wall -Werror -Os -g -mno-ex9 -mno-ifc -MMD -fno-builtin          \
                       -fomit-frame-pointer -fno-strict-aliasing -funroll-loops \
                       -ffunction-sections -fdata-sections -mcmodel=large
