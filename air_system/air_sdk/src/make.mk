include         $(AIR_SDK)/make/make_prologue.mk

# Subdirectories, in random order
dir             := $(d)/aml
include         $(dir)/make.mk

dir             := $(d)/api
include         $(dir)/make.mk

dir             := $(d)/cmlib
include         $(dir)/make.mk

dir             := $(d)/hal
include         $(dir)/make.mk

dir             := $(d)/osal
include         $(dir)/make.mk
# End subdirectories
# Local rules

include         $(AIR_SDK)/make/make_epilogue.mk
