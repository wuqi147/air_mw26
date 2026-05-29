include          $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order
ifeq ($(findstring an8502,$(COMPILED_POE_CHIPS)),an8502)
dir             := $(d)/an8502
include         $(dir)/make.mk
endif

ifeq ($(findstring an8503,$(COMPILED_POE_CHIPS)),an8503)
dir             := $(d)/an8503
include         $(dir)/make.mk
endif

# End subdirectories
# Local rules


include          $(AIR_SDK)/make/make_epilogue.mk
