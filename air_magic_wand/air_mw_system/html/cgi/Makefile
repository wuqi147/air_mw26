ifdef CMW_CONFIG
include $(CMW_CONFIG)
else
include $(AIR_MW_DIR)/project/$(PROJECT)/make/config.mk
endif

include $(AIR_MW_SYSTEM)/make/make_prologue.mk

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_VOICE_VLAN)), ENABLED)
ifneq ($(findstring ENABLED, $(AIR_SUPPORT_SURVEILLANCE_VLAN)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/vlanVoiceSurveil.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/vlanVoiceSurveil.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/vlanVoiceSurveil.d, $(DEPS_TOTAL))
endif
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_IGMP_SNP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/igmpSnoopingRpm.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/igmpSnoopingRpm.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/igmpSnoopingRpm.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_LP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/loopPrevention.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/loopPrevention.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/loopPrevention.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_DHCP_SNOOP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/dhcpSnooping.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/dhcpSnooping.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/dhcpSnooping.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_ICMP_CLIENT)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/icmpClient.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/icmpClient.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/icmpClient.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_SNMP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/snmpRpm.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/snmpRpm.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/snmpRpm.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_RSTP)), ENABLED)
ifneq ($(findstring ENABLED, $(AIR_SUPPORT_MSTP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/spanningTree.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/spanningTree.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/spanningTree.d, $(DEPS_TOTAL))
endif
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_MSTP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/spanningTreeInstance.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/spanningTreeInstance.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/spanningTreeInstance.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_RSTP_SECURITY)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/spanningTreeSecurity.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/spanningTreeSecurity.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/spanningTreeSecurity.d, $(DEPS_TOTAL))
endif
ifneq ($(findstring ENABLED, $(AIR_SUPPORT_MQTTD)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/mqttdCfg.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/mqttdCfg.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/mqttdCfg.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_LLDPD)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/lldpConfigRpm.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/lldpConfigRpm.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/lldpConfigRpm.d, $(DEPS_TOTAL))
SRCS_TOTAL      := $(filter-out $(d)/lldpNeighborRpm.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/lldpNeighborRpm.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/lldpNeighborRpm.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_SFP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/opticalModuleStatus.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/opticalModuleStatus.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/opticalModuleStatus.d, $(DEPS_TOTAL))
endif


ifneq ($(findstring ENABLED, $(AIR_SUPPORT_POE)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/poeSetting.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/poeSetting.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/poeSetting.d, $(DEPS_TOTAL))
SRCS_TOTAL      := $(filter-out $(d)/poeWatchDog.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/poeWatchDog.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/poeWatchDog.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_POE_WATCHDOG)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/poeWatchDog.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/poeWatchDog.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/poeWatchDog.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_CABLE_DIAG)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/cableDiagnostic.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/cableDiagnostic.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/cableDiagnostic.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_SNTP)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/systime.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/systime.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/systime.d, $(DEPS_TOTAL))
endif

ifneq ($(findstring ENABLED, $(AIR_SUPPORT_ERPS)), ENABLED)
SRCS_TOTAL      := $(filter-out $(d)/erpsSetting.c, $(SRCS_TOTAL))
OBJS_TOTAL      := $(filter-out $(d)/erpsSetting.o, $(OBJS_TOTAL))
DEPS_TOTAL      := $(filter-out $(d)/erpsSetting.d, $(DEPS_TOTAL))
endif

include $(AIR_MW_SYSTEM)/make/make_epilogue.mk
