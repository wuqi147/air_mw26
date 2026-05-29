# Recursive Build
include $(AIR_MW_SYSTEM)/make/make_prologue.mk

dir = $(AIR_MW_SYSTEM)/html
include $(AIR_MW_SYSTEM)/html/Makefile

dir = $(AIR_MW_SYSTEM)/init
include $(AIR_MW_SYSTEM)/init/Makefile

dir = $(AIR_MW_SYSTEM)/os_api
include $(AIR_MW_SYSTEM)/os_api/Makefile

dir = $(AIR_MW_SYSTEM)/util
include $(AIR_MW_SYSTEM)/util/Makefile

dir = $(AIR_MW_SYSTEM)/switching
include $(AIR_MW_SYSTEM)/switching/Makefile

dir = $(AIR_MW_SYSTEM)/db
include $(AIR_MW_SYSTEM)/db/Makefile

dir = $(AIR_MW_SYSTEM)/syncd
include $(AIR_MW_SYSTEM)/syncd/Makefile

dir = $(AIR_MW_SYSTEM)/httpd
include $(AIR_MW_SYSTEM)/httpd/Makefile

dir = $(AIR_MW_SYSTEM)/cmd
include $(AIR_MW_SYSTEM)/cmd/Makefile

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_CLI)), ENABLED)
dir = $(AIR_MW_SYSTEM)/cli
include $(AIR_MW_SYSTEM)/cli/Makefile
endif

dir = $(AIR_MW_SYSTEM)/sys_mgmt
include $(AIR_MW_SYSTEM)/sys_mgmt/Makefile

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_TELNET)), ENABLED)
dir = $(AIR_MW_SYSTEM)/telnet
include $(AIR_MW_SYSTEM)/telnet/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SSH)), ENABLED)
dir = $(AIR_MW_SYSTEM)/ssh
include $(AIR_MW_SYSTEM)/ssh/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_MQTTD)), ENABLED)
dir = $(AIR_MW_SYSTEM)/mqttd
include $(AIR_MW_SYSTEM)/mqttd/Makefile
endif

dir = $(AIR_MW_SYSTEM)/driver
include $(AIR_MW_SYSTEM)/driver/Makefile

ifeq ($(AIR_SUPPORT_POE), ENABLED)
dir = $(AIR_MW_SYSTEM)/poe
include $(AIR_MW_SYSTEM)/poe/Makefile
endif

ifeq ($(AIR_SUPPORT_ERPS), ENABLED)
dir = $(AIR_MW_SYSTEM)/switching/erps
include $(AIR_MW_SYSTEM)/switching/erps/Makefile
endif

ifeq ($(AIR_SUPPORT_LACP), ENABLED)
dir = $(AIR_MW_SYSTEM)/switching/lacp
include $(AIR_MW_SYSTEM)/switching/lacp/Makefile
endif

dir = $(AIR_MW_SYSTEM)/ref
include $(AIR_MW_SYSTEM)/ref/Makefile

dir = $(AIR_MW_DIR)/project/$(PROJECT)/customer
include $(dir)/Makefile

include $(AIR_MW_SYSTEM)/make/make_epilogue.mk

################################################################################
TGT_OBJS_TOTAL := $(subst $(AIR_MW_DIR)/project/$(PROJECT)/,,$(OBJS_TOTAL))
TGT_DEPS_TOTAL := $(subst $(AIR_MW_DIR)/project/$(PROJECT)/,,$(DEPS_TOTAL))

TGT_OBJS_TOTAL := $(addprefix $(AIR_MW_BUILD)/,$(subst $(AIR_MW_SYSTEM),,$(TGT_OBJS_TOTAL)))
TGT_DEPS_TOTAL := $(addprefix $(AIR_MW_BUILD)/,$(subst $(AIR_MW_SYSTEM),,$(TGT_DEPS_TOTAL)))

ARCHIVE_NAME   := mw_sys
################################################################################
sinclude $(TGT_DEPS_TOTAL)

# Internal build
ifeq ("$(origin AIR_MW_SUPPORT)", "command line")
EXTRA_MW_CFLAGS := -Werror
endif

CFLAGS         += $(MW_CFLAGS)
CFLAGS         += $(MW_SYS_INC)

# compile here and archive to a lib_mw.a
all: $(TGT_OBJS_TOTAL)
	@echo "    AR    $(ARCHIVE_NAME).a"
	@$(AR) -cr $(AIR_MW_BUILD)/$(ARCHIVE_NAME).a $(TGT_OBJS_TOTAL)

quiet_cmd_cc = -$(Q)$(CC) $(CFLAGS) $(EXTRA_MW_CFLAGS) -c -o $@ $< 1>&2 2>> $(AIR_LOG)
      cmd_cc = -$(CC) $(CFLAGS) $(EXTRA_MW_CFLAGS) -c -o $@ $< >> $(AIR_LOG) 2>&1

quiet = $(if $(Q),$(quiet_$(1)),$($(1)))

$(AIR_MW_BUILD)/%.o: $(AIR_MW_SYSTEM)/%.c
	@$(MKDIR) $(dir $@)
	@echo "    CC    $(notdir $@)"
	$(call quiet,cmd_cc)
	-$(Q)$(CC) -MM -MT $@ $(CFLAGS) $< > $(patsubst %.o,%.d, $@)

$(AIR_MW_BUILD)/%.o: $(AIR_MW_DIR)/project/$(PROJECT)/%.c
	@$(MKDIR) $(dir $@)
	@echo "    CC    $(notdir $@)"
	$(call quiet,cmd_cc)
	-$(Q)$(CC) -MM -MT $@ $(CFLAGS) $< > $(patsubst %.o,%.d, $@)

clean:
	@echo "Clean build folder"
	@rm -rf $(AIR_MW_BUILD)

FORCE:

.PHONY: FORCE all clean
