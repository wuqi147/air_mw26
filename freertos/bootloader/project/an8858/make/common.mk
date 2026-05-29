ADDR            = 4GB
USE_CACHEWB     = 1
DUAL_IMAGE      = 1

DEFINES         += -DCFG_$(ADDR)
DEFINES         += -DAIR_8858_SUPPORT
DEFINES         += -DDUAL_IMAGE=$(DUAL_IMAGE)

ifneq ($(shell echo | $(CC) -E -dM - | grep __NDS32_ISA_V3M__ > /dev/null && echo V3M),V3M)
ifeq ($(USE_CACHEWB),1)
DEFINES         += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITEBACK
endif
ifeq ($(USE_CACHEWT),1)
DEFINES         += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITETHROUGH
endif
endif

ifeq ($(AIR_MW_SUPPORT), ENABLED)
DEFINES         += -DAIR_MW_SUPPORT
endif
ifeq ($(FLASH_AVL_TEST),1)
DEFINES         += -DFLASH_AVL_TEST
endif
ifneq ($(BOOTMENU_COUNT_DOWN_TIME),"")
DEFINES         += -DBOOTMENU_AUTOBOOT_DELAY_TIME=$(BOOTMENU_COUNT_DOWN_TIME)
endif
ifeq ($(BOOT_SUPPORT_LED_CTRL),ENABLED)
DEFINES         += -DBOOT_LED_CTRL
ifeq ($(BOOT_SUPPORT_LED_ALL_ON),ENABLED)
DEFINES         += -DBOOT_LED_ALL_ON
endif
ifeq ($(BOOT_SUPPORT_LED_ACTIVE_HIGH),ENABLED)
DEFINES         += -DBOOT_LED_ACTIVE_HIGH
endif
endif
ifeq ($(AIR_SUPPORT_CPU_PROCESS),1)
DEFINES         += -DAIR_SUPPORT_CPU_PROCESS
endif
ifeq ($(AIR_SUPPORT_CMD),1)
DEFINES         += -DAIR_SUPPORT_CMD
endif
ifeq ($(AIR_SUPPORT_ACE),1)
ifneq ($(AIR_SUPPORT_CPU_PROCESS),1)
$(error AIR_SUPPORT_ACE shall be Enabled with AIR_SUPPORT_CPU_PROCESS !)
endif
DEFINES         += -DAIR_SUPPORT_ACE
endif
ifeq ($(AIR_SUPPORT_ACE_MAC_BURN),1)
ifneq ($(AIR_SUPPORT_ACE),1)
$(error AIR_SUPPORT_ACE_MAC_BURN shall be Enabled with AIR_SUPPORT_ACE !)
endif
DEFINES         += -DAIR_SUPPORT_ACE_MAC_BURN
endif
ifeq ($(AIR_SUPPORT_ACE_ETHERNET_LOG),1)
ifneq ($(AIR_SUPPORT_ACE),1)
$(error AIR_SUPPORT_ACE_ETHERNET_LOG shall be Enabled with AIR_SUPPORT_ACE !)
endif
DEFINES         += -DAIR_SUPPORT_ACE_ETHERNET_LOG
endif

DEFINES			+= -DUSE_PINCTRL

EXTRA_CFLAGS    += $(DEFINES)
export EXTRA_CFLAGS
