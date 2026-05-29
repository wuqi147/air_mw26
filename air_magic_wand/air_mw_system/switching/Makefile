ifdef CMW_CONFIG
include $(CMW_CONFIG)
else
include $(AIR_MW_DIR)/project/$(PROJECT)/make/config.mk
endif

include $(AIR_MW_SYSTEM)/make/make_prologue.mk

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LP)), ENABLED)
dir = $(d)/loop_prevention
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_IGMP_SNP)), ENABLED)
dir = $(d)/igmp_snoop
include $(dir)/Makefile
endif

dir = $(d)/mac_init
include $(dir)/Makefile

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_DHCP_SNOOP)), ENABLED)
dir = $(d)/dhcp_snoop
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_ICMP_CLIENT)), ENABLED)
dir = $(d)/icmp_client
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_VOICE_VLAN)), ENABLED)
dir = $(d)/voice_vlan
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_RSTP)), ENABLED)
dir = $(d)/stp
include $(dir)/Makefile

dir = $(d)/stp/rstp/
include $(dir)/Makefile

dir = $(d)/stp/rstp/ovs/lib
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_MSTP)), ENABLED)
dir = $(d)/stp
include $(dir)/Makefile

dir = $(d)/stp/mstp
include $(dir)/Makefile

dir = $(d)/stp/mstp/internal
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LLDPD)), ENABLED)
dir = $(d)/lldp
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_SNMP)), ENABLED)
dir = $(d)/snmp
include $(dir)/Makefile
endif

ifeq ($(findstring ENABLED, $(AIR_SUPPORT_LACP)), ENABLED)
dir = $(d)/lacp
include $(dir)/Makefile
endif

include $(AIR_MW_SYSTEM)/make/make_epilogue.mk
