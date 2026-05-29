include $(AIR_MW_SYSTEM)/make/make_prologue.mk

ifeq ($(AIR_SUPPORT_POE), ENABLED)
dir = $(d)/poe
include $(dir)/Makefile
endif

ifeq ($(AIR_SUPPORT_SFP), ENABLED)
dir = $(d)/sfp
include $(dir)/Makefile
endif

dir = $(d)/led
include $(dir)/Makefile

include $(AIR_MW_SYSTEM)/make/make_epilogue.mk