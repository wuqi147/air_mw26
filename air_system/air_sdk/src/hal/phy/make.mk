include          $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order
ifeq ($(findstring en8801s,$(COMPILED_PHY_CHIPS)),en8801s)
dir             := $(d)/en8801s
include         $(dir)/make.mk
endif

ifeq ($(findstring en8808,$(COMPILED_PHY_CHIPS)),en8808)
dir             := $(d)/en8808
include         $(dir)/make.mk
endif

ifeq ($(findstring en8811h,$(COMPILED_PHY_CHIPS)),en8811h)
dir             := $(d)/en8811h
include         $(dir)/make.mk
endif

ifeq ($(findstring an8801sb,$(COMPILED_PHY_CHIPS)),an8801sb)
dir             := $(d)/an8801sb
include         $(dir)/make.mk
endif

ifeq ($(findstring an8811b,$(COMPILED_PHY_CHIPS)),an8811b)
dir             := $(d)/an8811b
include         $(dir)/make.mk
endif

ifeq ($(findstring an8804,$(COMPILED_PHY_CHIPS)),an8804)
dir             := $(d)/an8804
include         $(dir)/make.mk
endif

ifeq ($(findstring an8858,$(COMPILED_PHY_CHIPS)),an8858)
dir             := $(d)/an8858
include         $(dir)/make.mk
endif
ifeq ($(findstring an8808,$(COMPILED_PHY_CHIPS)),an8808)
dir             := $(d)/an8808
include         $(dir)/make.mk
endif
# End subdirectories
# Local rules


include          $(AIR_SDK)/make/make_epilogue.mk

