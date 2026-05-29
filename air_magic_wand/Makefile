################################################################################
all: compile

################################################################################
AIR_MW_DIR := $(shell pwd)
AIR_MW_SYSTEM := $(AIR_MW_DIR)/air_mw_system
AIR_SDK_DIR := $(AIR_MW_DIR)/../air_system

export AIR_MW_DIR AIR_MW_SYSTEM
################################################################################
# Include config, include and compile flags
################################################################################
ifdef CMW_CONFIG
include $(CMW_CONFIG)
else
include $(AIR_MW_DIR)/project/$(PROJECT)/make/config.mk
endif

include $(AIR_MW_SYSTEM)/toolchain.mk
include $(AIR_MW_SYSTEM)/make/inc_path.mk
include $(AIR_MW_SYSTEM)/make/common.mk
include $(AIR_SDK_DIR)/build/air_cflags.mk
CFLAGS += $(AIR_CFLAGS)

################################################################################
ifndef LOG_FILE
LOG_FILE    = $(AIR_MW_SYSTEM)/log.Makefile
endif

AIR_LOG     = $(LOG_FILE)

export AIR_LOG

.PHONY: all


################################################################################
# Compile
# 1. copy necessary files
# 2. compile tools
# 3. compile source files and archive to mw_sys.a
################################################################################
# compile all source file first
compile: fsdata
	$(MAKE) -f $(AIR_MW_SYSTEM)/build.mk all
	@echo "Magic Wand compile pass"

fsdata: tools
ifeq ($(findstring AIR_SUPPORT_HTTPD, $(CFLAGS)), AIR_SUPPORT_HTTPD)
	$(AIR_MW_SYSTEM)/html/tools/makefsdata/makefsdata.bin -11
	mv fsdata.c $(APP_LWIP_ROOT_DIR)/$(APP_LWIP_APPS_HTTP_DIR)
endif

# compile http tools first
tools: prerequisite
ifeq ($(findstring AIR_SUPPORT_HTTPD, $(CFLAGS)), AIR_SUPPORT_HTTPD)
	$(MAKE) -C $(AIR_MW_SYSTEM)/html/tools/makefsdata all
endif

prerequisite: FORCE
	@cp $(AIR_MW_DIR)/project/$(PROJECT)/customer/air_magic_wand/air_mw_system/ref/product_ref.c $(AIR_MW_SYSTEM)/ref/product_ref.c
	@cp $(AIR_MW_DIR)/project/$(PROJECT)/customer/air_magic_wand/air_mw_system/html/html_config_customer.c $(AIR_MW_SYSTEM)/html/cgi/html_config_customer.c
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SFP)), ENABLED)
	@cp $(AIR_MW_DIR)/project/$(PROJECT)/customer/air_magic_wand/air_mw_system/driver/sfp/sfp_config_customer.c $(AIR_MW_SYSTEM)/driver/sfp/sfp_config_customer.c
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_ERPS)), ENABLED)
	@cp $(AIR_MW_DIR)/project/$(PROJECT)/customer/air_magic_wand/air_mw_system/switching/erps/erps_config_customer.c $(AIR_MW_SYSTEM)/switching/erps/erps_config_customer.c
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_POE)), ENABLED)
	@cp $(AIR_MW_DIR)/project/$(PROJECT)/customer/air_magic_wand/air_mw_system/driver/poe/poe_config_customer.c $(AIR_MW_SYSTEM)/driver/poe/poe_config_customer.c
endif

ifeq ($(findstring AIR_EN_CABLE_DIAG, $(CFLAGS)), AIR_EN_CABLE_DIAG)
ifneq ($(findstring AIR_SUPPORT_CABLE_DIAG, $(CFLAGS)), AIR_SUPPORT_CABLE_DIAG)
	$(error SDK enable CABLE_DIAG but MW didn't!)
endif
endif

ifeq ($(findstring AIR_SUPPORT_CABLE_DIAG, $(CFLAGS)), AIR_SUPPORT_CABLE_DIAG)
ifneq ($(findstring AIR_EN_CABLE_DIAG, $(CFLAGS)), AIR_EN_CABLE_DIAG)
	$(error MW enable CABLE_DIAG but SDK didn't!)
endif
endif

ifeq ($(findstring AIR_EN_SFP_LED, $(CFLAGS)), AIR_EN_SFP_LED)
ifneq ($(findstring AIR_SUPPORT_SFP, $(CFLAGS)), AIR_SUPPORT_SFP)
	$(error SDK enable SFP LED but MW didn't!)
endif
endif

FORCE:

clean:
	@$(MAKE) -C $(AIR_MW_SYSTEM)/html/tools/makefsdata clean
	@$(MAKE) -f $(AIR_MW_SYSTEM)/build.mk clean

.PHONY: FORCE all clean tools prerequisite
.NOTPARALLEL: all clean
