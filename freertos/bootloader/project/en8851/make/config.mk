################################################################################
# Configure the bootloader menu count down time
################################################################################
BOOTMENU_COUNT_DOWN_TIME        = 2
export BOOTMENU_COUNT_DOWN_TIME
################################################################################
# ACE burn MAC command
################################################################################
# BOOT_SUPPORT_ACE_CTRL = ENABLED
ifeq ($(BOOT_SUPPORT_ACE_CTRL),ENABLED)
AIR_SUPPORT_CPU_PROCESS         = 1
AIR_SUPPORT_ACE                 = 1
AIR_SUPPORT_ACE_MAC_BURN        = 1
AIR_SUPPORT_ACE_ETHERNET_LOG    = 1
export AIR_SUPPORT_CPU_PROCESS AIR_SUPPORT_ACE AIR_SUPPORT_ACE_MAC_BURN AIR_SUPPORT_ACE_ETHERNET_LOG
endif