################################################################################
all: compile install

################################################################################
AIR_SYSTEM      := $(shell pwd)

################################################################################
ifdef SDK_CONFIG
include $(SDK_CONFIG)
endif

ifndef AIR_SYSTEM_BUILD
include $(AIR_SYSTEM)/config.mk
endif
include $(AIR_SYSTEM)/build.mk
ifndef LOG_FILE
LOG_FILE = $(AIR_SYSTEM)/log.Makefile
endif
MK_OPTIONS += AIR_LOG=$(LOG_FILE)
################################################################################
OUTPUT_DIR_EXIST:
	$(TEST_PATH) $(AIR_SYSTEM_BUILD)/image || $(MKDIR) $(AIR_SYSTEM_BUILD)/image

################################################################################
compile: OUTPUT_DIR_EXIST
	$(foreach PKG, $(BUILD_LIST), cd $$$(PKG) && $(MAKE) compile $(MK_OPTIONS) || exit 1;)

install:
	$(foreach PKG, $(BUILD_LIST), cd $$$(PKG) && $(MAKE) install $(MK_OPTIONS) || exit 1;)

clean:
	$(foreach PKG, $(BUILD_LIST), cd $$$(PKG) && $(MAKE) clean $(MK_OPTIONS) || exit 1;)
	$(RM) $(AIR_SYSTEM_BUILD)

distclean:
	$(foreach PKG, $(BUILD_LIST), cd $$$(PKG) && $(MAKE) distclean $(MK_OPTIONS) || exit 1;)
	$(RM) $(AIR_SYSTEM_BUILD)

################################################################################
.PHONY: all compile install clean
.NOTPARALLEL: all compile install clean

