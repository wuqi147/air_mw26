include          $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order
dir             := $(d)/common
include         $(dir)/make.mk

dir             := $(d)/phy
include         $(dir)/make.mk

dir             := $(d)/switch
include         $(dir)/make.mk

dir             := $(d)/poe
include         $(dir)/make.mk

# End subdirectories
# Local rules


include          $(AIR_SDK)/make/make_epilogue.mk

