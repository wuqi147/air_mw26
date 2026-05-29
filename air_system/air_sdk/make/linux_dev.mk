###############################################################################
MODULE_NAME                := air_dev

###############################################################################
KNL_OBJS_TOTAL             := ./src/osal/linux_kernel/osal_mdc.o

KBUILD_EXTRA_SYMBOLS       += $(AIR_SDK)/Module.symvers

obj-m                      := $(MODULE_NAME).o
$(MODULE_NAME)-objs        += $(KNL_OBJS_TOTAL)

###############################################################################
# To create the folders to put .o in AIR_SYSTEM_BUILD
folder:
	$(TEST_PATH) $(AIR_SYSTEM_BUILD)/air_sdk/ || $(MKDIR) $(AIR_SYSTEM_BUILD)/air_sdk/
	$(foreach FOLDER, $(sort $(dir $(addprefix $(AIR_SYSTEM_BUILD)/air_sdk/,$(KNL_OBJS_TOTAL)))), $(TEST_PATH) $(FOLDER) || $(MKDIR) $(FOLDER);)

# To build the air_dev.ko in AIR_SYSTEM_BUILD/air_sdk/
compile:: folder
	touch $(AIR_SYSTEM_BUILD)/air_sdk/Makefile
	$(MAKE) -C $(OS_PATH) M=$(AIR_SYSTEM_BUILD)/air_sdk/ src=$(shell pwd) modules ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) AIR_SDK=$(AIR_SDK) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" KBUILD_EXTRA_SYMBOLS=$(KBUILD_EXTRA_SYMBOLS) >> $(AIR_LOG) 2>&1
	# export the Module.symvers to air_sdk/ for other modules
	$(CP) $(AIR_SYSTEM_BUILD)/air_sdk/Module.symvers $(AIR_SYSTEM)/air_sdk/

# To copy the air_dev.ko to AIR_SYSMTE_BUILD/image/
install::
	$(CP) $(AIR_SYSTEM_BUILD)/air_sdk/$(MODULE_NAME).ko $(AIR_SYSTEM_BUILD)/image/$(MODULE_NAME).ko

clean::
	$(RM) $(AIR_SYSTEM_BUILD)/image/$(MODULE_NAME).ko
