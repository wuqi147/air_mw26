PLAT_SAG_PATH        = $(TOPDIR)/project/$(CONFIG_PLATFORM)
PLAT_SAG_SCRIPT      = $(TOPDIR)/tool/platform_ldsag.sh
SAG_TOOL             = $(TOPDIR)/tool/nds_ldsag
SAG_SCRIPT           = $(PLAT_SAG_PATH)/linker_script.sag

LD_FLAGS             = -T $(TARGET_LD) $(OPTIM) -static -nostartfiles
LD_FLAGS            += -Wl,--gc-sections,--undefine=__rtos_signature_freertos_v10_1_1 -mcmodel=large
LD_FLAGS            += -Wl,-Map=$(TARGET_OUTDIR)/$(TARGET).map
LIB_LIST            += $(lib)

ifeq ($(AIR_MW_SUPPORT), ENABLED)
LIB_LIST            += $(AIR_MW_SYSTEM)/build/mw_sys.a
endif

link: $(elf) FORCE
	$(Q)$(OBJDUMP) -t -x -d -C -h -S $< > $(TARGET_OUTDIR)/$(TARGET).elf.txt

$(elf): $(LIB_LIST) $(TARGET_LD) FORCE
	$(Q)$(CC) $(LD_FLAGS) -o $@ -Wl,-start-group $(LIB_LIST) -Wl,-end-group >> $(LOG_FILE) 2>&1

$(TARGET_LD): FORCE
	$(Q)rm -f $(PLAT_SAG_PATH)/platform.sag
	$(Q)$(PLAT_SAG_SCRIPT) $(TOPDIR)/bsp/platform/$(CONFIG_PLATFORM)/platform.h $(PLAT_SAG_PATH)/platform.sag
	$(Q)cd $(PLAT_SAG_PATH) && $(SAG_TOOL) $(SAG_SCRIPT) -o $(TARGET_LD) && cd - > /dev/null

FORCE:

.PHONY: link FORCE