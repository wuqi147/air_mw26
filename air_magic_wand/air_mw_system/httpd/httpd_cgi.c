/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE AIROHA SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. AIROHA SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE AIROHA SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT AIROHA'S OPTION, TO REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  AIROHA FOR SUCH AIROHA SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

/* FILE NAME:  httpd_cgi.c
 * PURPOSE:
 *  Implement CGI/SSI handler functions for Lwip httpd
 *
 * NOTES:
 *
 */
#include "web.h"
#ifdef AIR_SUPPORT_SFP_DDM
#include "sfp_pin.h"
#endif
/* NAMING CONSTANT DECLARATIONS
 */
/* This enum table is one-to-one mapping to ppcTAGs table !! */
typedef enum
{
    TAG_ERRORTYPE = 0,              /*index.html                        ssi_get_errtype_info_Handle */
    TAG_SYSNAME,                    /*sys_SystemInfoRpm.html            ssi_get_system_name_Handle*/
    TAG_IPSETTING,                  /*sys_Ipsetting.html                ssi_get_ip_address_Handle*/
    TAG_SYSACCOUNT,                 /*sys_Account.html                  ssi_get_ip_netmask_Handle*/
    TAG_PORT_SETTING,               /*sys_portsetting.html              ssi_get_port_setting_info_Handle*/
#ifdef AIR_SUPPORT_IGMP_SNP
    TAG_IGMP_MODE,                  /*cfg_IgmpSnoopingRpm.html          ssi_get_igmp_mode_info_Handle*/
    TAG_REPORTSU_MODE,              /*cfg_IgmpSnoopingRpm.html          ssi_get_reportSu_mode_info_Handle*/
    TAG_IGMP_SNOOPING_INFO,         /*cfg_IgmpSnoopingRpm.html          ssi_get_igmp_snooping_info_Handle*/
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    TAG_IGMP_SNP_QUERIER,           /*igmp_snp_querier.xml              ssi_get_igmp_snp_querier_info_xmlHandle*/
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
    TAG_PORTTRUNK,                  /*cfg_PortTrunkRpm.html             ssi_get_porttrunk_info_Handle*/
    TAG_LAG_PROC_STATE,             /*lagProcState.xml                ssi_get_lag_process_state_xmlHandle*/
#ifdef AIR_SUPPORT_LP
    TAG_LOOPPREVENTION,             /*cfg_LoopPreventionRpm.html        ssi_get_loopprevention_info_Handle*/
#endif
    TAG_MIRRORINGPORT,              /*cfg_PortMirrorRpm.html            ssi_get_mirroringport_info_Handle*/
    TAG_BANDWIDTH,                  /*cfg_BandWidthControl.html         ssi_get_bandwidth_info_Handle*/
    TAG_JUMBOFRAME,                 /*cfg_JumboFrame.html               ssi_get_jumboframe_info_Handle*/
    TAG_EEECONFIG,                  /*cfg_EEEConfig.html                ssi_get_eeeconfig_info_Handle*/
    TAG_PVLANCONFIG,                /*vlan_portBased.html               ssi_get_port_vlan_info_Handle*/
    TAG_PVLANCONFIGEXTRA,           /*vlan_portBased.html               ssi_get_port_vlan_extra_info_Handle*/
    TAG_QVLANCONFIG,                /*vlan_8021q.html                   ssi_get_8021q_vlan_info_Handle*/
    TAG_QVLANCONFIGPVID,            /*vlan_8021q.html                   ssi_get_8021q_vlan_pvid_info_Handle*/
    TAG_QVLANCONFIGEXTRA,           /*vlan_8021q.html                   ssi_get_8021q_vlan_extra_info_Handle*/
    TAG_MVLANCONFIG,                /*vlan_mtu.html                     ssi_get_mtu_vlan_info_Handle*/
#ifdef AIR_SUPPORT_VOICE_VLAN
    TAG_VOICEVLAN,                  /*vlan_Voice.html                   ssi_get_voice_vlan_info_Handle*/
    TAG_VOICEVLANEXTRA,             /*vlan_Voice.html                   ssi_get_voice_vlan_extra_info_Handle*/
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    TAG_SURVEILVLAN,                /*vlan_Surveillance.html            ssi_get_serveil_vlan_info_Handle*/
#endif
    TAG_QOSBASICINFO,               /*qos_BasicRpm.html                 ssi_get_qos_basic_info_Handle*/
    TAG_QOSADVANCEINFO,             /*qos_AdvanceRpm.html               ssi_get_qos_advance_info_Handle*/
    TAG_STORMCTRLINFO,              /*security_StormControlRpm.html     ssi_get_storm_ctrl_info_Handle*/
    TAG_STATICMACINFO,              /*security_Macaddress.html          ssi_get_static_mac_info_Handle*/
    TAG_STATICMACENTRY,             /*static macentry.xml               ssi_get_static_mac_address_entry_xmlHandle*/
    TAG_PORTMAXMACNUM,              /*security_Macaddress.html          ssi_get_port_maxmac_info_Handle*/
    TAG_VLANFORMAC,                 /*security_Macaddress.html          ssi_get_vlan_mac_info_Handle*/
    TAG_MONITORPORTSTAT,            /*security_Monitor.html             ssi_get_port_data_info_Handle*/
#ifdef AIR_SUPPORT_CABLE_DIAG
    TAG_CABLERESULT,                /*tools_Cablediag.html              ssi_get_cable_diag_info_Handle*/
#endif
    TAG_SYSLANGUAGE,                /*index.html                        ssi_get_language_Handle*/
    TAG_CURLINK,                    /*curLink.xml                       ssi_get_port_state_xmlHandle*/
    TAG_MIBSTATE,                   /*mibState.xml                      ssi_get_mib_state_xmlHandle*/
#ifdef AIR_SUPPORT_IGMP_SNP
    TAG_L2MCENTRY,                  /*l2mcentry.xml                     ssi_get_l2mc_entry_xmlHandle*/
#endif
    TAG_VLANENTRY,                  /*vlanEntry.xml                     ssi_get_vlan_entry_xmlHandle*/
    TAG_FUSTATE,                    /*fuState.xml                       ssi_get_fuState_xmlHandle*/
    TAG_PORTCUR,                    /*portCur.xml                       ssi_get_portCur_xmlHandle*/
    TAG_DHCPSTATE,                  /*dhcpState.xml                     ssi_get_dhcpState_xmlHandle*/
#ifdef AIR_SUPPORT_IPV6
    TAG_DADRESULT,                  /* dadResult.xml                    ssi_get_dadResult_xmlHandle */
#endif
#ifdef AIR_SUPPORT_CABLE_DIAG
    TAG_CABLECUR,                   /*cableCur.xml                      ssi_get_cableCur_xmlHandle*/
#endif
    TAG_INDEXRE,                    /*index_re.html                     ssi_get_connection_info_Handle*/
    TAG_COOKIE,                     /*index_re.html                     ssi_get_cookie_info_Handle*/
#ifdef AIR_SUPPORT_DHCP_SNOOP
    TAG_DHCPSNOOP,                  /*security_DhcpSnooping.html        ssi_get_dhcp_snooping_info_Handle*/
    TAG_DHCPSNPOPT82,               /*opt82info.xml                     ssi_get_dhcp_snooping_opt82_info_Handle*/
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    TAG_SNMP,                       /*cfg_SnmpRpm.html                  ssi_get_snmp_info_Handle*/
#endif
#ifdef AIR_SUPPORT_ICMP_CLIENT
    TAG_PINGINFOSETTING,            /*tools_icmpClient.html             ssi_get_ping_info_Handle*/
    TAG_PINGCUR,                    /*pingCur.xml                       ssi_get_pinginfo_xmlHandle*/
#endif /* AIR_SUPPORT_ICMP_CLIENT */
    TAG_CLEARMIBCOUNTER,            /*clearmibcnt.xml                   ssi_get_clear_mib_counter_xmlHandle*/
    TAG_SAVESETTING,                /*saveSetting.xml                   ssi_set_save_setting_xmlHandle*/
    TAG_DYNAMICMACINFO,             /*security_Maccfg.html              ssi_get_dynamic_mac_address_entry_info_Handle*/
    TAG_DYNAMICMACENTRY,            /*dynamicmacentry.xml               ssi_get_dynamic_mac_address_entry_xmlHandle*/
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    TAG_RSTP,                       /*cfg_spanningTree.html             ssi_get_spanning_tree_info_Handle*/
    TAG_RSTP_PORT,                  /*cfg_spanningTree.html             ssi_get_spanning_tree_port_info_Handle*/
#ifdef AIR_SUPPORT_MSTP
    TAG_MSTP_REG,                   /*cfg_spanningTreeInstance.html     ssi_get_spanning_tree_region_info_Handle*/
    TAG_MSTP,                       /*cfg_spanningTreeInstance.html     ssi_get_spanning_tree_instance_info_Handle*/
    TAG_MSTP_PORT_STATE,            /*cfg_spanningTreeInstance.html     ssi_get_spanning_tree_instance_port_state_Handle*/
    TAG_MSTP_PORT_ROLE,             /*cfg_spanningTreeInstance.html     ssi_get_spanning_tree_instance_port_role_Handle*/
    TAG_MSTP_PORT_COST_ENTRY,       /*stpInstanceCost.xml               ssi_get_spanning_tree_instance_port_cost_xmlHandle*/
#endif /*AIR_SUPPORT_MSTP*/
#ifdef AIR_SUPPORT_RSTP_SECURITY
    TAG_STPSEC,                     /*cfg_spanningTreeSecurity.html     ssi_get_spanning_tree_security_info_Handle*/
#endif
#endif
    TAG_MENUDSNP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUIGSNP,                  /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENULOOP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUSNMP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUVOICE,                  /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUSURVEIL,                /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUICMP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENURSTP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_MSTP
    TAG_MENUMSTP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
#endif
    TAG_MENUSTPSEC,                 /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUMQTTD,                  /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUDDM,                    /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUDIAG,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_MQTTD
    TAG_MQTTD,                      /*cfg_CloudConnection.html          ssi_get_mqttd_info_Handle*/
    TAG_MQTTRET,                    /*mqttdret.xml                      ssi_get_mqttd_info_xmlHandle*/
#endif
    TAG_MENULLDP,                   /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENULLDP_NEIGHBOR,          /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_LLDPD
    TAG_LLDP,                       /*cfg_LldpConfigRpm.html            ssi_get_lldp_config_Handle*/
    TAG_LLDP_NEIGHBOR,              /*cfg_LldpNeighborRpm.html          ssi_get_lldp_neighbor_handle*/
    TAG_NEIGHBOR_INFO,              /*lldpNeighborInfo.xml              ssi_get_neighbor_info_xmlHandle*/
#endif
#ifdef AIR_SUPPORT_SFP_DDM
    TAG_SFPSTART,                   /*sys_OpticalModuleStatus.html      ssi_get_sfp_port_data_info_Handle*/
    TAG_SFP,                        /*sfpState.xml                      ssi_get_sfp_module_info_Handle*/
#endif /* AIR_SUPPORT_SFP_DDM */
    TAG_PORT_TRUNK_INFO,            /*all html operating trunk port     ssi_get_port_trunk_info_Handle*/
    TAG_FWCHUNK,                    /*fwChunk.xml                       ssi_get_fw_info_xmlHandle*/
    TAG_CHUNKSIZE,                  /*any html need chunk size          ssi_get_chunk_info_Handle*/

    TAG_CUSTOMER_LOGIN_LOGO,        /*index.html                        ssi_get_customer_login_logo_Handle*/
    TAG_CUSTOMER_LOGIN_COPYRIGHT,   /*index.html                        ssi_get_customer_login_copyright_Handle*/
    TAG_CUSTOMER_HOMEPAGE_LOGO,     /*top.html                          ssi_get_customer_homepage_logo_Handle*/
    TAG_CUSTOMER_ADDRBAR_FAVICON,   /*homepage.html/index.html          ssi_get_customer_addrbar_favicon_Handle*/
    TAG_CUSTOMER_ADDRBAR_PRO_VER,   /*homepage.html                     ssi_get_customer_addrbar_production_version_Handle*/
    TAG_CUSTOMER_HOMEPAGE_PRO_VER,  /*product.html                      ssi_get_customer_homepage_product_version_Handle*/
    TAG_CUSTOMER_MENU_COPYRIGHT,    /*menu.html                         ssi_get_customer_menu_copyright_Handle*/
    TAG_TOPLED,                     /*Top_led.html                      ssi_get_topLed_info_Handle*/
    TAG_TOPLEDCUR,                  /*topLed.xml                        ssi_get_topLed_info_xmlHandle*/
    TAG_MENUPOE,                    /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUPOE_CONFIG,             /*menu.html                         ssi_get_menu_option_info_Handle*/
    TAG_MENUPOE_WATCHDOG,           /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_POE
    TAG_POE,                        /*poeConfig.html                    ssi_get_poe_info_Handle*/
    TAG_POESTATE,                   /*poeState.xml                      ssi_get_poe_state_xmlHandle*/
#ifdef AIR_SUPPORT_POE_WATCHDOG
    TAG_POEWATCHDOG,                /*poeWatchDog.html                  ssi_get_poe_watchdog_Handle*/
#endif
#endif
    TAG_ERASETIME,                  /*tools_FirmwareUpdate.html         ssi_get_eraseTime_info_Handle*/
    TAG_TOP_LOGIN_FORM_BACKGROUND,  /*index.html                        ssi_get_topLoginFormBackground_info_Handle*/
    TAG_BOTTOM_LOGIN_FORM_BACKGROUND,  /*index.html                     ssi_get_bottomLoginFormBackground_info_Handle*/
    TAG_LOGIN_FORM_BORDER,          /*index.html                        ssi_get_loginFormBorder_info_Handle*/
    TAG_LOGIN_FORM_FONT,            /*index.html                        ssi_get_loginFormFont_info_Handle*/
    TAG_TOP_BANNER_BACKGROUND,      /*homepage.html                     ssi_get_topBannerBackground_info_Handle*/
    TAG_PORT_PANEL_BACKGROUND,      /*homepage.html                     ssi_get_portPanelBackground_info_Handle*/
    TAG_PORT_NUMBER_FONT,           /*Top_led.html                      ssi_get_portNumberFont_info_Handle*/
    TAG_DEVICE_TYPE_FONT,           /*product.html                      ssi_get_deviceTypeFont_info_Handle*/
    TAG_MENU_BACKGROUND,            /*homepage.html                     ssi_get_menuBackground_info_Handle*/
    TAG_MENU_TEXT,                  /*menu.html                         ssi_get_menuText_info_Handle*/
    TAG_MENU_HOVER_BACKGROUND,      /*menu.html                         ssi_get_menuHoverBackground_info_Handle*/
    TAG_SELECTED_MENU_BACKGROUND,   /*menu.html                         ssi_get_selectedMenuBackground_info_Handle*/
    TAG_SUBPAGE_BACKGROUND,         /*all html                          ssi_get_subPageBackground_info_Handle*/
    TAG_SUBPAGE_FONT,               /*all html                          ssi_get_subPageFont_info_Handle*/
    TAG_SUBPAGE_FUNCTION_GROUP_TITLE_FONT, /*all html                   ssi_get_subPageFunctionGroupTitleFont_info_Handle*/
    TAG_SUBPAGE_FUNCTION_GROUP_BODER, /*all html                        ssi_get_subPageFunctionGroupBorder_info_Handle*/
    TAG_SUBPAGE_TABLE_HEADER_BACKGROUND, /*all html                     ssi_get_subPageTableHeaderBackground_info_Handle*/
    TAG_SUBPAGE_TABLE_BORDER,       /*all html                          ssi_get_subPageTableBorder_info_Handle*/
    TAG_SUBPAGE_DIVIDER,            /*all html                          ssi_get_subPageDivider_info_Handle*/
    TAG_MENU_HOVER_FONT,            /*menu.html                         ssi_get_menuHoverFont_info_Handle*/
#ifdef AIR_SUPPORT_SNTP
    TAG_SYSTIME,                    /*sysTime.xml                       ssi_get_systime_info_handle*/
    TAG_TIMECUR,                    /*timeCur.xml                       ssi_get_timeCur_xmlHandle */
#endif
    TAG_MENUSNTP,                   /*menu.html                         ssi_get_menu_option_info_Handle */
    TAG_MENUERPS,                   /*menu.html                         ssi_get_menu_option_info_Handle */
#ifdef AIR_SUPPORT_ERPS
    TAG_ERPS,                       /*cfg_erps.html                     ssi_get_erps_info_Handle*/
    TAG_ERPSSTATE,                  /*erpsState.xml                     ssi_get_erps_state_xmlHandle*/
#endif /* AIR_SUPPORT_ERPS */
    TAG_HTTPD_SECHDR,               /*httpd security header tag         121:ssi_get_sechdr_info_Handle*/
    TAG_JS,                         /*javascript tag                    122:ssi_get_js_info_Handle*/
    TAG_CSS,                        /*css tag                           123:ssi_get_css_info_Handle*/
    TAG_JS_SECHDR,                  /*js security header tag            124:ssi_get_jshdr_info_Handle*/
    TAG_PORT_MODE,                  /*sys_portsetting.html              ssi_get_portmode_info_Handle*/
} SSI_TAGS_MAP_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/
const tCGI CGIURLs[] =
{
    {"/conf_restore.cgi",           "/fwChunk.xml",                  cgi_set_handle_conf_restore},
    {"/logon.cgi",                  "/homepage.html",                cgi_set_handle_logon},
    {"/languagechange.cgi",         "/index.html",                   cgi_set_handle_language},
    {"/system_name_set.cgi",        "/sys_SystemInfoRpm.html",       cgi_set_handle_systemInfoRpm},
    {"/ip_setting.cgi",             "/sys_Ipsetting.html",           cgi_set_handle_ipsetting},
    {"/usr_account_set.cgi",        "/sys_Account.html",             cgi_set_handle_account},
    {"/port_setting.cgi",           "/sys_portsetting.html",         cgi_set_handle_portSetting},
#ifdef AIR_SUPPORT_IGMP_SNP
    {"/igmpSnooping.cgi",           "/cfg_IgmpSnoopingRpm.html",     cgi_set_handle_igmpSnoopingRpm},
#endif

    {"/port_trunk_set.cgi",         "/cfg_PortTrunkRpm.html",        cgi_set_handle_portTrunk},
    {"/port_trunk_delete.cgi",      "/cfg_PortTrunkRpm.html",        cgi_set_handle_portTrunk},
    {"/port_trunk_algo_set.cgi",    "/cfg_PortTrunkRpm.html",        cgi_set_handle_portTrunkAlgo},

#ifdef AIR_SUPPORT_LP
    {"/loop_prevention_set.cgi",    "/cfg_LoopPreventionRpm.html",   cgi_set_handle_loopPrevention},
#endif
    {"/mirror_port_set.cgi",        "/cfg_PortMirrorRpm.html",       cgi_set_handle_mirrorPortSet},
    {"/port_speed_set.cgi",         "/cfg_BandWidthControl.html",    cgi_set_handle_bandWidthControl},
    {"/jumboframeSet.cgi",          "/cfg_JumboFrame.html",          cgi_set_handle_jumboFrame},
    {"/eeeSet.cgi",                 "/cfg_EEEConfig.html",           cgi_set_handle_eeeConfig},
    {"/pvlanModeSet.cgi",           "/vlan_PortBased.html",          cgi_set_handle_pvlanModeSet},
    {"/pvlanSet.cgi",               "/vlan_PortBased.html",          cgi_set_handle_pVlanSet},
    {"/qvlanModeSet.cgi",           "/vlan_8021q.html",              cgi_set_handle_qvlanModeSet},
    {"/qvlanSet.cgi",               "/vlan_8021q.html",              cgi_set_handle_qVlanSet},
    {"/mvlanModeSet.cgi",           "/vlan_mtu.html",                cgi_set_handle_mvlanModeSet},
    {"/mvlanSet.cgi",               "/vlan_mtu.html",                cgi_set_handle_mVlanSet},
    {"/qvlanPvidSet.cgi",           "/vlan_8021q.html",              cgi_set_handle_qvlanPvidSet},
#ifdef AIR_SUPPORT_VOICE_VLAN
    {"/voicevlanSet.cgi",           "/vlan_Voice.html",              cgi_set_handle_voiceVlanSet},
    {"/voiceOuiSet.cgi",            "/vlan_Voice.html",              cgi_set_handle_voiceOuiSet},
    {"/voiceOuiDel.cgi",            "/vlan_Voice.html",              cgi_set_handle_voiceOuiDel},
    {"/voicePortSet.cgi",           "/vlan_Voice.html",              cgi_set_handle_voicePortSet},
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    {"/surveilvlanSet.cgi",         "/vlan_Surveillance.html",       cgi_set_handle_surveilvlanSet},
    {"/surveilOuiSet.cgi",          "/vlan_Surveillance.html",       cgi_set_handle_surveilOuiSet},
    {"/surveilOuiDel.cgi",          "/vlan_Surveillance.html",       cgi_set_handle_surveilOuiDel},
#endif
    {"/qos_policy_mode_set.cgi",    "/qos_BasicRpm.html",            cgi_set_handle_qos_scheduler},
    {"/qos_queue_weight_set.cgi",   "/qos_BasicRpm.html",            cgi_set_handle_qos_scheduler},
    {"/qos_base_mode_set.cgi",      "/qos_AdvanceRpm.html",          cgi_set_handle_qos_priority},
    {"/qos_advance_set.cgi",        "/qos_AdvanceRpm.html",          cgi_set_handle_qos_priority},
    {"/maxmacnumSet.cgi",           "/security_Maccfg.html",         cgi_set_handle_maxMacNumSet},
    {"/addstaticmac.cgi",           "/security_Maccfg.html",         cgi_set_handle_addStaticMac},
    {"/delstaticmac.cgi",           "/security_Maccfg.html",         cgi_set_handle_delStaticMac},
    {"/storm_ctrl_set.cgi",         "/security_StormControlRpm.html",cgi_set_handle_stormCtrl},
#ifdef AIR_SUPPORT_CABLE_DIAG
    {"/cablediag.cgi",              "/tools_Cablediag.html",         cgi_set_handle_cableDiag},
#endif
    {"/reset.cgi",                  "/tools_ResetToFactory.html",    cgi_set_handle_reset_factory},
    {"/reboot.cgi",                 "/tools_reboot.xml",             cgi_set_handle_system_reboot},
    {"/fupgrade.cgi",               "/fwChunk.xml",                  cgi_set_handle_fwupdate},
    {"/logout.cgi",                 "/index.html",                   cgi_set_handle_logout},
#ifdef AIR_SUPPORT_DHCP_SNOOP
    {"/dhcpSnoopEnable.cgi",        "/security_DhcpSnooping.html",   cgi_set_handle_dhcpSnooping},
    {"/dhcpSnoopPortSet.cgi",       "/security_DhcpSnooping.html",   cgi_set_handle_dhcpSnooping},
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    {"/snmp.cgi",                   "/cfg_SnmpRpm.html",             cgi_set_handle_snmpConfig},
#endif
#ifdef AIR_SUPPORT_ICMP_CLIENT
    {"/ping_info_set.cgi",           "/tools_icmpClient.html",       cgi_set_handle_pinginfo},
#endif /* AIR_SUPPORT_ICMP_CLIENT */
    {"/clear_mib_counter.cgi",      "/clearmibcnt.xml",              cgi_set_handle_clearMibCounter},
    {"/freshdynamicmac.cgi",        "/dynamicmacentry.xml",          cgi_set_handle_freshDynamicMac},
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    {"/stpCfg.cgi",                 "/cfg_SpanningTree.html",        cgi_set_handle_spanningTree},
    {"/stpPortCfg.cgi",             "/cfg_SpanningTree.html",        cgi_set_handle_spanningTree},
#ifdef AIR_SUPPORT_MSTP
    {"/stpRegCfg.cgi",              "/cfg_SpanningTreeInstance.html",cgi_set_handle_regionInstance},
    {"/stpInsCfg.cgi",              "/cfg_SpanningTreeInstance.html",cgi_set_handle_spanningTreeInstance},
    {"/stpInsPortCfg.cgi",          "/cfg_SpanningTreeInstance.html",cgi_set_handle_spanningTreePortInstance},
#endif /*AIR_SUPPORT_MSTP*/
#ifdef AIR_SUPPORT_RSTP_SECURITY
    {"/stpSecPortCfg.cgi",          "/cfg_SpanningTreeSecurity.html",cgi_set_handle_spanningTreeSecurity},
#endif
#endif
#ifdef AIR_SUPPORT_MQTTD
    {"/mqttdSet.cgi",               "/mqttdret.xml",                 cgi_set_handle_mqttd},
    {"/saveRunning.cgi",            "/saveSetting.xml",              cgi_set_handle_save_running},
#endif
#ifdef AIR_SUPPORT_LLDPD
    {"/lldpConfig.cgi",             "/cfg_LldpConfigRpm.html",       cgi_set_handle_lldpconfig},
    {"/lldpPortConfig.cgi",         "/cfg_LldpConfigRpm.html",       cgi_set_handle_lldpportconfig},
#endif
#ifdef AIR_SUPPORT_POE
    {"/poeSetting.cgi",             "/poeConfig.html",               cgi_set_handle_poeConfig},
    {"/poePortSetting.cgi",         "/poeConfig.html",               cgi_set_handle_poePortConfig},
#ifdef AIR_SUPPORT_POE_WATCHDOG
    {"/poeWatchDogConfig.cgi",      "/poe_WatchDog.html",            cgi_set_handle_poeWatchDogConfig},
    {"/poeWatchDog.cgi",            "/poe_WatchDog.html",            cgi_set_handle_poeWatchDog},
#endif
#endif
#ifdef AIR_SUPPORT_SNTP
    {"/system_sntp_set.cgi",        "/cfg_Systime.html",             cgi_set_systime_info_handle},
#endif
#ifdef AIR_SUPPORT_ERPS
    {"/erpsSetting.cgi",            "/cfg_erps.html",                    cgi_set_handle_erpsConfig},
#endif /* AIR_SUPPORT_ERPS */
    {NULL, NULL, NULL}
};

const char* const ppcTAGs[] =
{
    "errorType",            /*index.html                        ssi_get_errtype_info_Handle */
    "sysName",              /*sys_SystemInfoRpm.html            ssi_get_system_name_Handle*/
    "ipSetting",            /*sys_Ipsetting.html                ssi_get_ip_address_Handle*/
    "sysAccount",           /*sys_Account.html                  ssi_get_ip_netmask_Handle*/
    "port_setting",         /*sys_portsetting.html              ssi_get_port_setting_info_Handle*/
#ifdef AIR_SUPPORT_IGMP_SNP
    "igmp_mode",            /*cfg_IgmpSnoopingRpm.html          ssi_get_igmp_mode_info_Handle*/
    "reportSu_mode",        /*cfg_IgmpSnoopingRpm.html          ssi_get_reportSu_mode_info_Handle*/
    "igmp_snooping_info",   /*cfg_IgmpSnoopingRpm.html          ssi_get_igmp_snooping_info_Handle*/
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    "igmp_snp_querier",     /*igmp_snp_querier.xml              ssi_get_igmp_snp_querier_info_xmlHandle*/
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
    "porttrunk",            /*cfg_PortTrunkRpm.html             ssi_get_porttrunk_info_Handle*/
    "lagProcState",         /*lagProcState.xml                ssi_get_lag_process_state_xmlHandle*/
#ifdef AIR_SUPPORT_LP
    "loopprevention",       /*cfg_LoopPreventionRpm.html        ssi_get_loopprevention_info_Handle*/
#endif
    "mirroringport",        /*cfg_PortMirrorRpm.html            ssi_get_mirroringport_info_Handle*/
    "bandwidth",            /*cfg_BandWidthControl.html         ssi_get_bandwidth_info_Handle*/
    "jumboframe",           /*cfg_JumboFrame.html               ssi_get_jumboframe_info_Handle*/
    "eeeconfig",            /*cfg_EEEConfig.html                ssi_get_eeeconfig_info_Handle*/
    "pvlanconfig",          /*vlan_PortBased.html               ssi_get_port_vlan_info_Handle*/
    "pvlanconfigextra",     /*vlan_PortBased.html               ssi_get_port_vlan_extra_info_Handle*/
    "qvlanconfig",          /*vlan_8021q.html                   ssi_get_8021q_vlan_info_Handle*/
    "qvlanconfigpvid",      /*vlan_8021q.html                   ssi_get_8021q_vlan_pvid_info_Handle*/
    "qvlanconfigextra",     /*vlan_8021q.html                   ssi_get_8021q_vlan_extra_info_Handle*/
    "mvlanconfig",          /*vlan_mtu.html                     ssi_get_mtu_vlan_info_Handle*/
#ifdef AIR_SUPPORT_VOICE_VLAN
    "voicevlan",            /*vlan_Voice.html                   ssi_get_voice_vlan_info_Handle*/
    "voicevlanextra",       /*vlan_Voice.html                   ssi_get_voice_vlan_info_Handle*/
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    "Surveilvlan",          /*vlan_Surveillance.html            ssi_get_serveil_vlan_info_Handle*/
#endif
    "qosBasicInfo",         /*qos_BasicRpm.html                 ssi_get_qos_basic_info_Handle*/
    "qosAdvanceInfo",       /*qos_AdvanceRpm.html               ssi_get_qos_advance_info_Handle*/
    "stormCtrlInfo",        /*security_StormControlRpm.html     ssi_get_storm_ctrl_info_Handle*/
    "staticmacinfo",        /*security_Maccfg.html              ssi_get_static_mac_info_Handle*/
    "staticmacentry",       /*staticcmacentry.xml               ssi_get_static_mac_address_entry_xmlHandle*/
    "portmaxmacnum",        /*security_Maccfg.html              ssi_get_port_maxmac_info_Handle*/
    "vlanformac",           /*security_Maccfg.html              ssi_get_vlan_mac_info_Handle*/
    "monitorportstat",      /*security_Monitor.html             ssi_get_port_data_info_Handle*/
#ifdef AIR_SUPPORT_CABLE_DIAG
    "cableresult",          /*tools_Cablediag.html              ssi_get_cable_diag_info_Handle*/
#endif
    "syslanguage",          /*index.html                        ssi_get_language_Handle*/
    "curLink",              /*curLink.xml                       ssi_get_port_state_xmlHandle*/
    "mibState",             /*mibState.xml                      ssi_get_mib_state_xmlHandle*/
#ifdef AIR_SUPPORT_IGMP_SNP
    "l2mcentry",            /*l2mcentry.xml                     ssi_get_l2mc_entry_xmlHandle*/
#endif
    "vlanEntry",            /*vlanEntry.xml                     ssi_get_vlan_entry_xmlHandle*/
    "fuState",              /*fuState.xml                       ssi_get_fuState_xmlHandle*/
    "portCur",              /*portCur.xml                       ssi_get_portCur_xmlHandle*/
    "dhcpState",            /*dhcpState.xml                     ssi_get_dhcpState_xmlHandle*/
#ifdef AIR_SUPPORT_IPV6
    "dadResult",            /* dadResult.xml                    ssi_get_dad_result_xmlHandle*/
#endif
#ifdef AIR_SUPPORT_CABLE_DIAG
    "cableCur",             /*cableCur.xml                      ssi_get_cableCur_xmlHandle*/
#endif
    "index_re",             /*index_re.html                     ssi_get_connection_info_Handle*/
    "cookie",               /*index_re.html                     ssi_get_cookie_info_Handle*/
#ifdef AIR_SUPPORT_DHCP_SNOOP
    "dhcpsnpinfo",          /*security_DhcpSnooping.html        ssi_get_dhcp_snooping_info_Handle*/
    "opt82info",            /*opt82info.xml                     ssi_get_dhcp_snooping_opt82_info_Handle*/
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    "snmp_info",             /*cfg_SnmpRpm.html                 ssi_get_snmp_info_Handle*/
#endif
#ifdef AIR_SUPPORT_ICMP_CLIENT
    "pinginfoSetting",      /*tools_icmpClient.html             ssi_get_ping_info_Handle*/
    "pingCur",              /*pingCur.xml                       ssi_get_pinginfo_xmlHandle*/
#endif /* AIR_SUPPORT_ICMP_CLIENT */
    "clearmibcnt",          /*clearmibcnt.xml                   ssi_get_clear_mib_counter_xmlHandle*/
    "saveSetting",          /*saveSetting.xml                   ssi_set_save_setting_xmlHandle*/
    "dynamicmacinfo",       /*security_Maccfg.html              ssi_get_dynamic_mac_address_entry_info_Handle*/
    "dynamicmacentry",      /*dynamicmacentry.xml               ssi_get_dynamic_mac_address_entry_xmlHandle*/
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    "stpinfo",              /*cfg_SpanningTree.html             ssi_get_spanning_tree_info_Handle*/
    "stpportinfo",          /*cfg_SpanningTree.html             ssi_get_spanning_tree_port_info_Handle*/
#ifdef AIR_SUPPORT_MSTP
    "stpReginfo",           /*cfg_SpanningTreeInstance.html     ssi_get_spanning_tree_region_info_Handle*/
    "stpInsinfo",           /*cfg_SpanningTreeInstance.html     ssi_get_spanning_tree_instance_info_Handle*/
    "stpInsPortState",      /*cfg_SpanningTreeInstance.html     ssi_get_spanning_tree_instance_port_state_Handle*/
    "stpInsPortRole",       /*cfg_SpanningTreeInstance.html     ssi_get_spanning_tree_instance_port_role_Handle*/
    "stpInstanceCost",      /*stpInstanceCost.xml               ssi_get_spanning_tree_instance_port_cost_xmlHandle*/
#endif /*AIR_SUPPORT_MSTP*/
#ifdef AIR_SUPPORT_RSTP_SECURITY
    "stpsecinfo",           /*cfg_SpanningTreeSecurity.html     ssi_get_spanning_tree_security_info_Handle*/
#endif
#endif
    "menuDsnp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuIgsnp",            /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuLoop",             /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuSnmp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuVoice",            /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuSurveil",          /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuIcmp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuRstp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_MSTP
    "menuMstp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
#endif /*AIR_SUPPORT_MSTP*/
    "menuStpSecurity",      /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuMqttd",            /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuDDM",              /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuDiag",             /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_MQTTD
    "cloudconnection",      /*cfg_CloudConnection.html          ssi_get_mqttd_info_Handle*/
    "mqttdret",             /*mqttdret.xml                      ssi_get_mqttd_info_xmlHandle*/
#endif
    "menuLldp",             /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuLldpNeighbor",     /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_LLDPD
    "lldpconfig",           /*cfg_LldpConfigRpm.html            ssi_get_lldp_config_Handle*/
    "lldpneighbor",         /*cfg_LldpNeighborRpm.html          ssi_get_lldp_neighbor_handle*/
    "lldpNeighborInfo",     /*lldpNeighborInfo.xml              ssi_get_neighbor_info_xmlHandle*/
#endif
#ifdef AIR_SUPPORT_SFP_DDM
    "sfpStart",             /*sys_OpticalModuleStatus.html      ssi_get_sfp_port_data_info_Handle*/
    "sfpState",             /*sfpState.xml                      ssi_get_sfp_module_info_Handle*/
#endif /* AIR_SUPPORT_SFP_DDM */
    "portTrunkInfo",        /*all html operating trunk port     ssi_get_port_trunk_info_Handle*/
    "fwChunk",              /*fwChunk.xml                       ssi_get_fw_info_xmlHandle*/
    "chunksize",            /*ayn html need chunk size          ssi_get_chunk_info_Handle*/

    "custLoginLogo",        /*index.html                        ssi_get_customer_login_logo_Handle*/
    "custLoginCopyright",   /*index.html                        ssi_get_customer_login_copyright_Handle*/
    "custHomepageLogo",     /*top.html                          ssi_get_customer_homepage_logo_Handle*/
    "custAddrbarFavicon",   /*homepage.html/index.html          ssi_get_customer_addrbar_favicon_Handle*/
    "custAddrbarProVer",    /*homepage.html                     ssi_get_customer_addrbar_production_version_Handle*/
    "custHomepageProVer",   /*product.html                      ssi_get_customer_homepage_product_version_Handle*/
    "custMenuCopyright",    /*menu.html                         ssi_get_customer_menu_copyright_Handle*/
    "top_led",              /*Top_led.html                      ssi_get_topLed_info_Handle*/
    "topLed",               /*topLed.xml                        ssi_get_topLed_info_xmlHandle*/
    "menuPoe",              /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuPoeConfig",        /*menu.html                         ssi_get_menu_option_info_Handle*/
    "menuPoeWatchDog",      /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_POE
    "poeCfg",               /*poeConfig.html                    ssi_get_poe_info_Handle*/
    "poeState",             /*poeState.xml                      ssi_get_poe_state_xmlHandle*/
#ifdef AIR_SUPPORT_POE_WATCHDOG
    "poeWatchDog",          /*poe_WatchDog.html                 ssi_get_poe_watchdog_Handle*/
#endif
#endif
    "eraseTime",            /*tools_FirmwareUpdate.htm          ssi_get_eraseTime_info_Handle*/
    "topLgFmBkGd",          /*index.html                        ssi_get_topLoginFormBackground_info_Handle*/
    "bottomLgFmBkGd",       /*index.html                        ssi_get_bottomLoginFormBackGround_info_Handle*/
    "loginFormBdr",         /*index.html                        ssi_get_loginFormBorder_info_Handle*/
    "loginFormFont",        /*index.html                        ssi_get_loginFormFont_info_Handle*/
    "topBnrBkGd",           /*homepage.html                     ssi_get_topBannerBackground_info_Handle*/
    "portPnlBkGd",          /*homepage.html                     ssi_get_portPanelBackground_info_Handle*/
    "portNumFont",          /*Top_led.html                      ssi_get_portNumberFont_info_Handle*/
    "devTypeFont",          /*product.html                      ssi_get_deviceTypeFont_info_Handle*/
    "menuBkGd",             /*homepage.html                     ssi_get_menuBackground_info_Handle*/
    "menuText",             /*menu.html                         ssi_get_menuText_info_Handle*/
    "menuHvrBkGd",          /*menu.html                         ssi_get_menuHoverBackground_info_Handle*/
    "selctedMnuBkGd",       /*menu.html                         ssi_get_selectedMenuBackground_info_Handle*/
    "subPageBkGd",          /*all html                          ssi_get_subPageBackground_info_Handle*/
    "subPageFont",          /*all html                          ssi_get_subPageFont_info_Handle*/
    "subPgFcGrpTtl",        /*all html                          ssi_get_subPageFunctionGroupTitleFont_info_Handle*/
    "subPgFcGrpBdr",        /*all html                          ssi_get_subPageFunctionGroupBorder_info_Handle*/
    "subPageThdBkGd",       /*all html                          ssi_get_subPageTableHeaderBackground_info_Handle*/
    "subPageTblBr",         /*all html                          ssi_get_subPageTableBorder_info_Handle*/
    "subPageDivider",       /*all html                          ssi_get_subPageDivider_info_Handle*/
    "menuHvrFont",          /*menu.html                         ssi_get_menuHoverFont_info_Handle*/
#ifdef AIR_SUPPORT_SNTP
    "sysTime",              /*sysTime.xml                       ssi_get_systime_info_handle*/
    "timeCur",              /*timeCur.xml                       ssi_get_timeCur_xmlHandle*/
#endif
    "menuSystime",          /*cfg_Systime.xml                   ssi_get_menu_option_info_Handle*/
    "menuErps",             /*menu.html                         ssi_get_menu_option_info_Handle*/
#ifdef AIR_SUPPORT_ERPS
    "erpsCfg",              /*cfg_erps.html                     ssi_get_erps_info_Handle*/
    "erpsState",            /*erpsState.xml                     ssi_get_erps_state_xmlHandle*/
#endif /* AIR_SUPPORT_ERPS */
    "sechdr",               /*security header for all html      121:ssi_get_sechdr_info_Handle*/
    "js",                   /*javascript for all html           122:ssi_get_js_info_Handle*/
    "css",                  /*style css for all html            123:ssi_get_css_info_Handle*/
    "jshdr",                /*security header for javascript    124:ssi_get_jshdr_info_Handle*/
    "portMode",             /*sys_portsetting.html              ssi_get_portmode_info_Handle*/
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: SSIHandler
 * PURPOSE:
 *      SSI handler functions of specific TAG.
 *
 * INPUT:
 *      iIndex      - Number of TAGs.
 *      ptr_pcb     - A pointer of tcp_pcb.
 *      apiflags    - A flags used for tcp_ip.
 *
 * OUTPUT:
 *      ptr_length  - A pointer of length of TAGs.
 *
 * RETURN:
 *      ERR_OK
 *      ERR_MEM
 *
 * NOTES:
 *      None
 */
u16_t
SSIHandler(
#if LWIP_HTTPD_SSI_RAW
    const char* ssi_tag_name,
#else /* LWIP_HTTPD_SSI_RAW */
    int iIndex,
#endif /* LWIP_HTTPD_SSI_RAW */
    char *pcInsert, int iInsertLen
#if LWIP_HTTPD_SSI_MULTIPART
    , u16_t current_tag_part, u16_t *next_tag_part
#endif /* LWIP_HTTPD_SSI_MULTIPART */
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
    , void *connection_state
#endif /* LWIP_HTTPD_FILE_STATE */
    , char *ptr_chuck_enable)
{
#if LWIP_HTTPD_SSI_RAW
    return 0;
#endif

    s8_t ret = ERR_ARG;
    HTTPD_TAG_PARAM_T tag_param = {0};
    MENU_OPTION_T opList = MENU_OPTION_NULL;
    int len = 0;

    tag_param.ptr_tag_insert = pcInsert;
    tag_param.tag_insert_max_len = iInsertLen;
#if LWIP_HTTPD_SSI_MULTIPART
    tag_param.current_tag_part = current_tag_part;
    tag_param.next_tag_part = HTTPD_LAST_TAG_PART;
#endif
#if defined(LWIP_HTTPD_FILE_STATE) && LWIP_HTTPD_FILE_STATE
    tag_param.ptr_connection_state = connection_state;
#endif
    tag_param.tag_insert_len = 0;
    tag_param.chunk_enable = TRUE;

    switch (iIndex)
    {
        case TAG_HTTPD_SECHDR:
            ret = ssi_get_sechdr_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_JS:
            ret = ssi_get_js_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CSS:
            ret = ssi_get_css_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_JS_SECHDR:
            ret = ssi_get_jshdr_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_ERRORTYPE:
            ret = ssi_get_errtype_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SYSNAME:
            ret = ssi_get_system_name_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_IPSETTING:
            ret = ssi_get_ipsetting_Handle(&tag_param);
            break;
        case TAG_SYSACCOUNT:
            ret = ssi_get_username_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PORT_SETTING:
            ret = ssi_get_port_setting_info_Handle(&tag_param);
            break;
        case TAG_PORT_MODE:
            ret = ssi_get_portmode_info_Handle(&tag_param);
            break;
#ifdef AIR_SUPPORT_IGMP_SNP
        case TAG_L2MCENTRY:
            ret = ssi_get_l2mc_entry_xmlHandle(&tag_param);
            break;
        case TAG_IGMP_SNOOPING_INFO:
            ret = ssi_get_igmp_snooping_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
        case TAG_IGMP_SNP_QUERIER:
            ret = ssi_get_igmp_snp_querier_info_xmlHandle(&tag_param);
            break;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
        case TAG_PORTTRUNK:
            ret = ssi_get_porttrunk_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_LAG_PROC_STATE:
            ret = ssi_get_lag_process_state_xmlHandle(&tag_param);
            break;
#ifdef AIR_SUPPORT_LP
        case TAG_LOOPPREVENTION:
            ret = ssi_get_loopprevention_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
        case TAG_MIRRORINGPORT:
            ret = ssi_get_mirroringport_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_BANDWIDTH:
            ret = ssi_get_bandwidth_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_JUMBOFRAME:
            ret = ssi_get_jumboframe_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_EEECONFIG:
            ret = ssi_get_eeeconfig_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PVLANCONFIG:
            ret = ssi_get_port_vlan_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PVLANCONFIGEXTRA:
            ret = ssi_get_port_vlan_extra_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_QVLANCONFIG:
            ret = ssi_get_8021q_vlan_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_QVLANCONFIGPVID:
            ret = ssi_get_8021q_vlan_pvid_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_QVLANCONFIGEXTRA:
            ret = ssi_get_8021q_vlan_extra_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MVLANCONFIG:
            ret = ssi_get_mtu_vlan_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_VOICE_VLAN
        case TAG_VOICEVLAN:
            ret = ssi_get_voice_vlan_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_VOICEVLANEXTRA:
            ret = ssi_get_voice_vlan_extra_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
        case TAG_SURVEILVLAN:
            ret = ssi_get_serveil_vlan_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
        case TAG_QOSBASICINFO:
            ret = ssi_get_qos_basic_info_Handle(&tag_param);
            break;
        case TAG_QOSADVANCEINFO:
            ret = ssi_get_qos_advance_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_STORMCTRLINFO:
            ret = ssi_get_storm_ctrl_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_STATICMACINFO:
            ret = ssi_get_static_mac_info_Handle(&tag_param);
            break;
        case TAG_STATICMACENTRY:
            ret = ssi_get_static_mac_address_entry_xmlHandle(&tag_param);
            break;
        case TAG_PORTMAXMACNUM:
            ret = ssi_get_port_maxmac_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_VLANFORMAC:
            ret = ssi_get_vlan_mac_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MONITORPORTSTAT:
            ret = ssi_get_port_data_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_CABLE_DIAG
        case TAG_CABLERESULT:
            ret = ssi_get_cable_diag_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
        case TAG_SYSLANGUAGE:
            ret = ssi_get_language_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CURLINK:
            ret = ssi_get_port_state_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MIBSTATE:
            ret = ssi_get_mib_state_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_VLANENTRY:
            ret = ssi_get_vlan_entry_xmlHandle(&tag_param);
            break;
        case TAG_FUSTATE:
            ret = ssi_get_fuState_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PORTCUR:
            ret = ssi_get_portCur_xmlHandle(&tag_param);
            break;
        case TAG_DHCPSTATE:
            ret = ssi_get_dhcpState_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_IPV6
        case TAG_DADRESULT:
            ret = ssi_get_dadResult_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#ifdef AIR_SUPPORT_CABLE_DIAG
        case TAG_CABLECUR:
            ret = ssi_get_cableCur_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
        case TAG_INDEXRE:
            ret = ssi_get_connection_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_COOKIE:
            ret = ssi_get_cookie_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_DHCP_SNOOP
        case TAG_DHCPSNOOP:
            ret = ssi_get_dhcp_snooping_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_DHCPSNPOPT82:
            ret = ssi_get_dhcp_snooping_opt82_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
        case TAG_SNMP:
            ret = ssi_get_snmp_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#ifdef AIR_SUPPORT_ICMP_CLIENT
        case TAG_PINGINFOSETTING:
            ret = ssi_get_ping_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PINGCUR:
            ret = ssi_get_pinginfo_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP_DDM
        case TAG_SFPSTART:
            ret = ssi_get_sfp_port_data_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SFP:
            ret = ssi_get_sfp_module_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif /* AIR_SUPPORT_SFP_DDM */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
        case TAG_RSTP:
            ret = ssi_get_spanning_tree_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_RSTP_PORT:
            ret = ssi_get_spanning_tree_port_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_MSTP
        case TAG_MSTP_REG:
            ret = ssi_get_spanning_tree_region_info_Handle(&tag_param);
            break;
        case TAG_MSTP:
            ret = ssi_get_spanning_tree_instance_info_Handle(&tag_param);
            break;
        case TAG_MSTP_PORT_STATE:
            ret = ssi_get_spanning_tree_instance_port_state_Handle(&tag_param);
            break;
        case TAG_MSTP_PORT_ROLE:
            ret = ssi_get_spanning_tree_instance_port_role_Handle(&tag_param);
            break;
        case TAG_MSTP_PORT_COST_ENTRY:
            ret = ssi_get_spanning_tree_instance_port_cost_xmlHandle(&tag_param);
            break;
#endif /*AIR_SUPPORT_MSTP*/
#ifdef AIR_SUPPORT_RSTP_SECURITY
        case TAG_STPSEC:
            ret = ssi_get_spanning_tree_security_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#endif /* AIR_SUPPORT_RSTP */
        case TAG_CLEARMIBCOUNTER:
            ret = ssi_get_clear_mib_counter_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SAVESETTING:
            ret = ssi_set_save_setting_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_DYNAMICMACINFO:
            ret = ssi_get_dynamic_mac_address_entry_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_DYNAMICMACENTRY:
            ret = ssi_get_dynamic_mac_address_entry_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENUDSNP:
#ifdef AIR_SUPPORT_DHCP_SNOOP
            opList = MENU_OPTION_DHCPSNOOP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUIGSNP:
#ifdef AIR_SUPPORT_IGMP_SNP
            opList = MENU_OPTION_IGSNP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENULOOP:
#ifdef AIR_SUPPORT_LP
            opList = MENU_OPTION_LOOP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUSNMP:
#ifdef AIR_SUPPORT_SNMP
            opList = MENU_OPTION_SNMP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUVOICE:
#ifdef AIR_SUPPORT_VOICE_VLAN
            opList = MENU_OPTION_VOICE;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUSURVEIL:
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
            opList = MENU_OPTION_SURVEILLANCE;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUICMP:
#ifdef AIR_SUPPORT_ICMP_CLIENT
            opList = MENU_OPTION_ICMPC;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENURSTP:
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
            opList = MENU_OPTION_RSTP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#ifdef AIR_SUPPORT_MSTP
        case TAG_MENUMSTP:
            opList = MENU_OPTION_MSTP;
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#endif /*AIR_SUPPORT_MSTP*/
        case TAG_MENUSTPSEC:
#ifdef AIR_SUPPORT_RSTP_SECURITY
            opList = MENU_OPTION_STP_SECURITY;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUMQTTD:
#ifdef AIR_SUPPORT_MQTTD
            opList = MENU_OPTION_MQTTD;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUDDM:
#ifdef AIR_SUPPORT_SFP_DDM
            if ((TRUE == sfp_check_ready()) && (TRUE == sfp_pin_sda_isAnyPortReady()))
            {
                opList = MENU_OPTION_DDM;
            }
#endif /* AIR_SUPPORT_SFP_DDM */
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUDIAG:
#ifdef AIR_SUPPORT_CABLE_DIAG
            opList = MENU_OPTION_DIAG;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#ifdef AIR_SUPPORT_MQTTD
        case TAG_MQTTD:
            ret = ssi_get_mqttd_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MQTTRET:
            ret = ssi_get_mqttd_info_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;

#endif
        case TAG_MENULLDP:
#ifdef AIR_SUPPORT_LLDPD
            opList = MENU_OPTION_LLDP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENULLDP_NEIGHBOR:
#ifdef AIR_SUPPORT_LLDPD
            opList = MENU_OPTION_LLDP_NEIGHBOR;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#ifdef AIR_SUPPORT_LLDPD
        case TAG_LLDP:
            ret = ssi_get_lldp_config_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_LLDP_NEIGHBOR:
            ret = ssi_get_lldp_neighbor_handle(&tag_param);
            break;
        case TAG_NEIGHBOR_INFO:
            ret = ssi_get_neighbor_info_xmlHandle(&tag_param);
            break;
#endif
        case TAG_PORT_TRUNK_INFO:
            ret = ssi_get_port_trunk_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_FWCHUNK:
            ret = ssi_get_fw_info_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CHUNKSIZE:
            ret = ssi_get_chunk_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_LOGIN_LOGO:
            ret = ssi_get_customer_login_logo_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_LOGIN_COPYRIGHT:
            ret = ssi_get_customer_login_copyright_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_HOMEPAGE_LOGO:
            ret = ssi_get_customer_homepage_logo_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_ADDRBAR_FAVICON:
            ret = ssi_get_customer_addrbar_favicon_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_ADDRBAR_PRO_VER:
            ret = ssi_get_customer_addrbar_production_version_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_HOMEPAGE_PRO_VER:
            ret = ssi_get_customer_homepage_product_version_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_CUSTOMER_MENU_COPYRIGHT:
            ret = ssi_get_customer_menu_copyright_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_TOPLED:
            ret = ssi_get_topLed_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_TOPLEDCUR:
            ret = ssi_get_topLed_info_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENUPOE:
#ifdef AIR_SUPPORT_POE
            opList = MENU_OPTION_POE;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUPOE_CONFIG:
#ifdef AIR_SUPPORT_POE
            opList = MENU_OPTION_POE_CONFIG;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUPOE_WATCHDOG:
#ifdef AIR_SUPPORT_POE
#ifdef AIR_SUPPORT_POE_WATCHDOG
            opList = MENU_OPTION_POE_WATCHDOG;
#endif
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#ifdef AIR_SUPPORT_POE
        case TAG_POE:
            ret = ssi_get_poe_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_POESTATE:
            ret = ssi_get_poe_state_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_POE_WATCHDOG
        case TAG_POEWATCHDOG:
            ret = ssi_get_poe_watchdog_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
#endif
        case TAG_ERASETIME:
            ret = ssi_get_eraseTime_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_TOP_LOGIN_FORM_BACKGROUND:
            ret = ssi_get_topLoginFormBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_BOTTOM_LOGIN_FORM_BACKGROUND:
            ret = ssi_get_bottomLoginFormBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_LOGIN_FORM_BORDER:
            ret = ssi_get_loginFormBorder_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_LOGIN_FORM_FONT:
            ret = ssi_get_loginFormFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_TOP_BANNER_BACKGROUND:
            ret = ssi_get_topBannerBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PORT_PANEL_BACKGROUND:
            ret = ssi_get_portPanelBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_PORT_NUMBER_FONT:
            ret = ssi_get_portNumberFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_DEVICE_TYPE_FONT:
            ret = ssi_get_deviceTypeFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENU_BACKGROUND:
            ret = ssi_get_menuBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENU_TEXT:
            ret = ssi_get_menuText_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENU_HOVER_BACKGROUND:
            ret = ssi_get_menuHoverBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SELECTED_MENU_BACKGROUND:
            ret = ssi_get_selectedMenuBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_BACKGROUND:
            ret = ssi_get_subPageBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_FONT:
            ret = ssi_get_subPageFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_FUNCTION_GROUP_TITLE_FONT:
            ret = ssi_get_subPageFunctionGroupTitleFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_FUNCTION_GROUP_BODER:
            ret = ssi_get_subPageFunctionGroupBorder_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_TABLE_HEADER_BACKGROUND:
            ret = ssi_get_subPageTableHeaderBackground_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_TABLE_BORDER:
            ret = ssi_get_subPageTableBorder_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_SUBPAGE_DIVIDER:
            ret = ssi_get_subPageDivider_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_MENU_HOVER_FONT:
            ret = ssi_get_menuHoverFont_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#ifdef AIR_SUPPORT_SNTP
        case TAG_SYSTIME:
            ret = ssi_get_systime_info_handle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
        case TAG_TIMECUR:
            ret = ssi_get_timeCur_xmlHandle(&len, (struct tcp_pcb *)&tag_param, 0);
            break;
#endif
        case TAG_MENUSNTP:
#ifdef AIR_SUPPORT_SNTP
            opList = MENU_OPTION_SNTP;
#endif
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
        case TAG_MENUERPS:
#ifdef AIR_SUPPORT_ERPS
            opList = MENU_OPTION_ERPS;
#endif /* AIR_SUPPORT_ERPS */
            ret = ssi_get_menu_option_info_Handle(&len, (struct tcp_pcb *)&tag_param, 0, opList);
            break;
#ifdef AIR_SUPPORT_ERPS
        case TAG_ERPS:
            ret = ssi_get_erps_info_Handle(&tag_param);
            break;
        case TAG_ERPSSTATE:
            ret = ssi_get_erps_state_xmlHandle(&tag_param);
            break;
#endif /* AIR_SUPPORT_ERPS */
        default:
            break;
    }

    if ((MW_E_OK == ret) && (0 != tag_param.tag_insert_len))
    {
#ifdef AIR_SUPPORT_HTTPD_SECURE
        if (TAG_JS != iIndex)
        {
            const char *tag = "<script";
            const u32_t tag_len = strlen(tag);
            const u32_t repl_len = strlen("<script nonce=''") + HTTPD_NONCE_LEN;
            char *pos = NULL;

            if ((pos = strstr(tag_param.ptr_tag_insert, tag)) != NULL)
            {
                if (memcmp(pos, tag, tag_len) == 0)
                {
                    memmove(pos + repl_len, pos + tag_len, strlen(pos + tag_len));
                    snprintf(pos, repl_len, "<script nonce='%s'", http_nonce_buf);
                    pos[repl_len - 1] = '\'';
                    tag_param.tag_insert_len += (repl_len - tag_len);
                }
            }
        }
#endif /* AIR_SUPPORT_HTTPD_SECURE */
#if LWIP_HTTPD_SSI_MULTIPART
        *next_tag_part = tag_param.next_tag_part;
#endif
        *ptr_chuck_enable = tag_param.chunk_enable;
    }
    else
    {
#if LWIP_HTTPD_SSI_MULTIPART
        *next_tag_part = HTTPD_LAST_TAG_PART;
#endif
        tag_param.tag_insert_len = 0;
        ret = (MW_E_OK == ret) ? MW_E_BAD_PARAMETER : ret;
    }

    CGI_LOG_DEBUG(HTTPD, "tagID:%5d max_len:%5d tag_insert_len:%5d chunk_enable:%d ret:%d", iIndex, iInsertLen, tag_param.tag_insert_len, *ptr_chuck_enable, ret);

    return (u16_t)tag_param.tag_insert_len;
}

/* FUNCTION NAME: get_numCgiHandler
 * PURPOSE:
 *      Parser max number of CGI handler.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Number of CGI handler functions.
 *
 * NOTES:
 *      None
 */
int
get_numCgiHandler()
{
    return (sizeof(CGIURLs)/sizeof(tCGI) - 1);
}

/* FUNCTION NAME: get_numSsiTag
 * PURPOSE:
 *      Parser max number of SSI TAGs.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Number of SSI TAGs.
 *
 * NOTES:
 *      None
 */
int
get_numSsiTag()
{
    return (sizeof(ppcTAGs)/sizeof(ppcTAGs[0]));
}

