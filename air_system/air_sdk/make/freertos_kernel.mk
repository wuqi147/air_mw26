################################################################################
ARCHIVE_NAME            := air_sdk

################################################################################
TGT_OBJS_TOTAL          := $(addprefix $(AIR_SYSTEM_BUILD)/,$(subst $(AIR_SYSTEM),,$(OBJS_TOTAL)))
TGT_DEPS_TOTAL          := $(addprefix $(AIR_SYSTEM_BUILD)/,$(subst $(AIR_SYSTEM),,$(DEPS_TOTAL)))
ifeq ($(findstring ENABLED,$(AIR_EN_CABLE_DIAG)),ENABLED)
HW_UTIL_LIB             := $(AIR_SYSTEM)/air_sdk/lib/freertos/cmlib_hw_util.a
else
HW_UTIL_LIB             :=
endif
################################################################################
# Standard things
sinclude $(TGT_DEPS_TOTAL)

CFLAGS += -I$(OS_PATH)/kernel/include \
          -I$(OS_PATH)/bsp/include \
          -I$(OS_PATH)/bsp/arch/NDS32 \
          -I$(AIR_SYSTEM)/app/sdk_ref/freertos/inc

################################################################################
compile:: $(TGT_OBJS_TOTAL)
	$(AR) -cr $(AIR_SYSTEM_BUILD)/image/$(ARCHIVE_NAME).a $(TGT_OBJS_TOTAL) $(HW_UTIL_LIB)

$(AIR_SYSTEM_BUILD)/%.o: ../%.c
	$(TEST_PATH) $(dir $@) || $(MKDIR) $(dir $@)
	-$(CC) $(EXTRA_CFLAGS) $(CFLAGS) $(SDK_CFLAGS) -c -o $@ $< >> $(AIR_LOG) 2>&1
	-$(CC) -MM -MT $@ $(EXTRA_CFLAGS) $(CFLAGS) $(SDK_CFLAGS) $< > $(patsubst %.o,%.d, $@)

clean::
	$(RM) $(TGT_OBJS_TOTAL)
	$(RM) $(TGT_DEPS_TOTAL)
	$(RM) $(AIR_LOG)
	$(RM) $(AIR_SYSTEM_BUILD)/image/$(ARCHIVE_NAME).a

install::
