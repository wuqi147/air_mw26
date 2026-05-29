include         $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order
dir             := $(d)/parser
include         $(dir)/make.mk

dir             := $(d)/cmd
include         $(dir)/make.mk

ifeq ($(findstring ENABLED,$(AIR_EN_CHIP_DIAG)),ENABLED)
dir             := $(d)/chip_diag
include         $(dir)/make.mk
endif

ifeq ($(findstring ENABLED,$(AIR_EN_API_DIAG)),ENABLED)
dir             := $(d)/api_diag
include         $(dir)/make.mk
endif
# End subdirectories
# Local rules

include         $(AIR_SDK)/make/make_epilogue.mk

