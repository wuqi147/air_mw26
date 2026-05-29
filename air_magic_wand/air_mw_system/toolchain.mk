################################################################################
CROSS_COMPILE_PATH  := /mtkeda/bbn/trendchip/Andes/nds32le-elf-mculib-v3j/bin/
CROSS_COMPILE       := nds32le-elf-

CC                  := $(CROSS_COMPILE)gcc
LD                  := $(CROSS_COMPILE)ld
AR                  := $(CROSS_COMPILE)ar
STRIP               := $(CROSS_COMPILE)strip --strip-unneeded

CFLAGS              ?= -Wall -g -Os
CFLAGS              += -mcmodel=large

MW_CFLAGS           :=

LD_LIBRARY_PATH     := $(CROSS_COMPILE_PATH)/../lib
################################################################################
MAKE                := make
RM                  := rm -rf
TEST_PATH           := test -d
MKDIR               := mkdir -p
CP                  := cp
MV                  := mv

################################################################################
PATH                := $(CROSS_COMPILE_PATH):$(PATH)

################################################################################
export CROSS_COMPILE CC LD AR STRIP
export MAKE RM TEST_PATH MKDIR CP MV
export PATH
export CFLAGS EXTRA_CFLAGS
export LD_LIBRARY_PATH
