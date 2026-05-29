ifneq ($(CONFIG_NDS32),)
CROSS_COMPILE_PATH  ?= $(HOME)/nds32le-elf-mculib-v3j/bin
CROSS_COMPILE       := $(CROSS_COMPILE_PATH)/nds32le-elf-
PATH                := $(CROSS_COMPILE_PATH):$(PATH)
CC                  = $(CROSS_COMPILE)gcc
OBJDUMP             = $(CROSS_COMPILE)objdump
OBJCOPY             = $(CROSS_COMPILE)objcopy
AR                  = $(CROSS_COMPILE)ar
AS                  = $(CROSS_COMPILE)as

DEBUG               := 0
ifeq ($(DEBUG),1)
OPTIM               = -O0 -g3 -mno-ex9 -mno-ifc
else
OPTIM               = -Os -g -mno-ex9 -mno-ifc
endif

CFLAGS              ?= -Wall -Werror $(OPTIM) -MMD -fno-builtin                 \
                       -fomit-frame-pointer -fno-strict-aliasing -funroll-loops \
                       -ffunction-sections -fdata-sections -mcmodel=large

ifeq ($(shell $(CC) -fno-delete-null-pointer-checks -E - 2>/dev/null >/dev/null </dev/null ; echo $$?),0)
CFLAGS              += -fno-delete-null-pointer-checks
endif
endif

export CROSS_COMPILE_PATH CC AR OPTIM CFLAGS
