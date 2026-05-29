################################################################################
MW_SYS_INC  = -I$(AIR_MW_SYSTEM)/init/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/util/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/mac_init/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/syncd/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/cmd/inc/cmd
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/cmd/inc/parser
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/cli/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/httpd/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/sys_mgmt/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/snmp/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/telnet/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/ssh/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/html/cgi
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/db/freeRTOS/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/db/freeRTOS/config
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/driver/led/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/driver/led/sfp_led/inc
MW_SYS_INC += -I$(AIR_MW_DIR)/project/$(PROJECT)

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SFP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/driver/sfp/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_POE)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/poe/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/driver/poe/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_ERPS)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/erps/inc
endif
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/ref

ifeq ($(OS_MODULE), freertos)
MW_SYS_INC += $(INC)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/html/cgi
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/os_api/freeRTOS/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/db/freeRTOS/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/db/freeRTOS/config
MW_SYS_INC += -I$(OS_PATH)/bsp/platform/$(CONFIG_PLATFORM)
MW_SYS_INC += -I$(OS_PATH)/project/$(PROJECT)/customer
MW_SYS_INC += -I$(OS_PATH)/project/$(PROJECT)
MW_SYS_INC += -I$(OS_PATH)/kernel/include
MW_SYS_INC += -I$(OS_PATH)/bsp/include
MW_SYS_INC += -I$(OS_PATH)/bsp/arch/NDS32
MW_SYS_INC += -I$(OS_PATH)/app/btn_reset/inc
MW_SYS_INC += -I$(OS_PATH)/app/lwip
MW_SYS_INC += -I$(OS_PATH)/app/lwip/ports/AIR_8851/include
MW_SYS_INC += -I$(OS_PATH)/app/lwip/lwip-2.1.2/src/include
MW_SYS_INC += -I$(OS_PATH)/app/lwip/lwip-2.1.2/src/include/lwip/apps
MW_SYS_INC += -I$(OS_PATH)/app/lwip/lwip-2.1.2/src/include/lwip/prot
MW_SYS_INC += -I$(AIR_SDK_PATH)/air_sdk/include
MW_SYS_INC += -I$(AIR_SDK_PATH)/air_sdk/src/inc/osal
MW_SYS_INC += -I$(AIR_SDK_PATH)/air_sdk/src/inc
MW_SYS_INC += -I$(AIR_SDK_PATH)/air_sdk/src/inc/hal/common
MW_SYS_INC += -I$(AIR_SDK_PATH)/app/sfp_led
MW_SYS_INC += -I$(AIR_SDK_PATH)/app/i2c_bitbang
else
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/os_api/linux/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/db/linux/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/loop_prevention/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LLDPD)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/lldp/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_IGMP_SNP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/igmp_snoop/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_DHCP_SNOOP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/dhcp_snoop/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_ICMP_CLIENT)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/icmp_client/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SNMP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/snmp/inc
endif
ifeq ($(findstring ENABLED, $(AIR_SUPPORT_RSTP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/stp/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/stp/rstp/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/stp/rstp/ovs/include
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_MSTP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/stp/inc
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/stp/mstp/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SFP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/driver/sfp/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_MQTTD)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/mqttd/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_VOICE_VLAN)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/voice_vlan/inc
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LACP)), ENABLED)
MW_SYS_INC += -I$(AIR_MW_SYSTEM)/switching/lacp/inc
endif

export MW_SYS_INC
