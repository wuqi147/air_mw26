include         $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order

ifeq ($(OS_TYPE),freertos)
dir             := $(d)/freertos
include         $(dir)/make.mk
endif

ifeq ($(OS_TYPE),linux)
ifeq ($(WORK_SPACE),USER_SPACE)
dir             := $(d)/linux_user
include         $(dir)/make.mk
endif
endif
# End subdirectories
# Local rules

include         $(AIR_SDK)/make/make_epilogue.mk
