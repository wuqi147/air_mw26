include          $(AIR_SDK)/make/make_prologue.mk

ifeq ($(findstring scorpio,$(COMPILED_SWITCH_CHIPS)),scorpio)
# Subdirectories, in random order
dir             := $(d)/sco
include         $(dir)/make.mk
endif

ifeq ($(findstring pearl,$(COMPILED_SWITCH_CHIPS)),pearl)
dir             := $(d)/pearl
include         $(dir)/make.mk
endif

ifeq ($(findstring coral,$(COMPILED_SWITCH_CHIPS)),coral)
dir             := $(d)/coral
include         $(dir)/make.mk
endif
# End subdirectories
# Local rules


include          $(AIR_SDK)/make/make_epilogue.mk

