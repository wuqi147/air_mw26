/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:   default_config.c
 * PURPOSE:
 *      1.Definition default value of each group.
 *      2.Definition name of each Json member.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "default_config.h"
#include "db_data.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/*****************************************
 *  SYS_INFO                             *
 *****************************************/
const DB_SYS_INFO_T _sys_info_dft =
{
    .sys_name = AIR_DEFAULT_SYSTEM_NAME,
    .sw_version = "1.0",
    .dhcp_enable = 0,
    .autodns_enable = 0,
    .static_ip = 0xC800A8C0,
    .static_mask = 0x00FFFFFF,
    .static_gw = 0,
    .static_dns = 0,
#ifdef AIR_SUPPORT_SECOND_NETIF
    .static_if2_ip = 0x01dcA8C0,
    .static_if2_mask = 0x00FFFFFF,
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    .mgmt_vlan = 1,
#endif
#ifdef AIR_SUPPORT_IPV6
    .manual_ip6_prefix_len = 0,
    .manual_ip6_addr = {{0}},
    .manual_ip6_default_gw = {{0}},
    .config_mode = 0, /* 0: auto mode, 1: manual mode */
#endif
};

/*****************************************
 *  ACCOUNT_INFO                         *
 *****************************************/
/* User Account use SHA-256 */
const DB_ACCOUNT_INFO_T _account_info_dft =
{
    .username = "admin",
    .passwd = {0x8c,0x69,0x76,0xe5,0xb5,0x41,0x04,0x15,0xbd,0xe9,0x08,0xbd,0x4d,0xee,0x15,0xdf,
               0xb1,0x67,0xa9,0xc8,0x73,0xfc,0x4b,0xb8,0xa8,0x1f,0x6f,0x2a,0xb4,0x48,0xa9,0x18} /*admin */
};

/*****************************************
 *  PORT_CFG_INFO                        *
 *****************************************/
const DB_PORT_CFG_INFO_T _port_cfg_info_dft =
{
    .type = 0,
    .admin_status = 1,
    .port_settings = 0x401F, /* AN, FC ON, All speeds: 0b0100 0000 0001 1111 */
    .isolation = AIR_MAX_PORT_BITMAP,
    .ingress_rate = 0,
    .egress_rate = 0,
    .green_ethernet = 0,
    .eee_enable = 0,
    .pvid = 1,
    .trunk_id = 0,
    .mirror_id = 0,
    .vlan_list = 1,
    .mac_limit = 0,
    .storm_bc_rate = 0,
    .storm_mc_rate = 0,
    .storm_uc_rate = 0,
    .storm_bc_mode = 0,
    .storm_mc_mode = 0,
    .storm_uc_mode = 0,
    .storm_bc_cfg = 0,
    .storm_mc_cfg = 0,
    .storm_uc_cfg = 0,
    .igmp_router_port = 0,
    .vlan_ig_filter = 0
};

/*****************************************
 *  PORT_QOS                             *
 *****************************************/
const DB_PORT_QOS_T _port_qos_dft =
{
    .priority = 0,
    .q0_weight_wrr = 1,
    .q1_weight_wrr = 2,
    .q2_weight_wrr = 3,
    .q3_weight_wrr = 4,
    .q4_weight_wrr = 5,
    .q5_weight_wrr = 6,
    .q6_weight_wrr = 7,
    .q7_weight_wrr = 8,
    .q0_weight_wfq = 1,
    .q1_weight_wfq = 2,
    .q2_weight_wfq = 3,
    .q3_weight_wfq = 4,
    .q4_weight_wfq = 5,
    .q5_weight_wfq = 6,
    .q6_weight_wfq = 7,
    .q7_weight_wfq = 8,
    .qos_schedule = 0
};

/*****************************************
 *  TRUNK_PORT                           *
 *****************************************/
const DB_TRUNK_PORT_T _trunk_port_dft =
{
     .members = {0, 0},
};
const DB_TRUNK_ALGORITHM_T _trunk_algo_dft =
{
     .algorithm = 0,
};

#ifdef AIR_SUPPORT_LP
/*****************************************
 *  LOOP_PREVEN_INFO                     *
 *****************************************/
const DB_LOOP_PREVEN_INFO_T _loop_preven_info_dft =
{
    .enable = 0,
};
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_IGMP_SNP
/*****************************************
 *  IGMP_SNP_INFO                        *
 *****************************************/
const DB_IGMP_SNP_INFO_T _igmp_snp_info_dft =
{
    .enable = 0,
    .rpt_suppress = 0,
    .fast_leave = 0
};

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
/*****************************************
 *  IGMP_SNP_QUERIER_INFO                *
 *****************************************/
const DB_IGMP_SNP_QUERIER_INFO_T _igmp_snp_querier_info_dft =
{
    .vlan_id =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .state =     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .election =  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    .version =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .address =   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */

/*****************************************
 *  PORT_MIRROR_INFO                     *
 *****************************************/
const DB_PORT_MIRROR_INFO_T _port_mirror_info_dft =
{
    .enable = 0,
    .dest_port = 0,
    .src_in_port = 0,
    .src_eg_port = 0
};

/*****************************************
 *  JUMBO_FRAME_INFO                     *
 *****************************************/
const DB_JUMBO_FRAME_INFO_T _jumbo_frame_info_dft =
{
    .cfg = 0
};

/*****************************************
 *  VLAN_CFG_INFO                        *
 *****************************************/
const DB_VLAN_CFG_INFO_T _vlan_cfg_info_dft =
{
    .enable_port_b = 1,
    .enable_8021q_b = 0,
    .enable_mtu = 0
};

/*****************************************
 *  VLAN_ENTRY                           *
 *****************************************/
const DB_VLAN_ENTRY_T _vlan_entry_dft =
{
    .vlan_id = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,0,0,0,0},
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    .vlan_fid = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
#endif
    .descr = {"Default","","","","","","","","","","","","","","","","","","","","","","","","","","","","","","",""},
    .port_member = {AIR_MAX_PORT_BITMAP,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .tagged_member = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .untagged_member = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

#ifdef AIR_SUPPORT_VOICE_VLAN
/*****************************************
 *  VOICE_VLAN_INFO                      *
 *****************************************/
const DB_VOICE_VLAN_INFO_T _voice_vlan_info_dft =
{
    .vlan_state = 0,
    .vlan_id = 0,
    .vlan_priority = 0
    ,.port_mode = 0xFFFFFFF
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    ,.security_mode = 0
#endif
};
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/*****************************************
 *  SURVEI_VLAN_INFO                     *
 *****************************************/
const DB_SURVEI_VLAN_INFO_T _survei_vlan_info_dft =
{
    .vlan_state = 0,
    .vlan_id = 0,
    .vlan_priority = 0
};
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
/*****************************************
 *  OUI_ENTRY                            *
 *****************************************/
const DB_OUI_ENTRY_T _oui_entry_dft =
{
    .mac = {{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},
            {0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0},{0,0,0}},
    .type = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .descr = {"","","","","","","","","","","","","","","",""}
};
#endif
/*****************************************
 *  QOS_INFO                             *
 *****************************************/
const DB_QOS_INFO_T _qos_info_dft =
{
    .qos_mode = 0,
    .pri_to_queue = {1,0,2,3,4,5,6,7},
    .dscp_to_pri = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/*****************************************
 *  STATIC_MAC_ENTRY                     *
 *****************************************/
const DB_STATIC_MAC_ENTRY_T _static_mac_entry_dft =
{
    .mac_addr = {{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                 {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                 {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                 {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                 {0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},{0,0,0,0,0,0},
                 {0,0,0,0,0,0},{0,0,0,0,0,0}},
    .vid = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    .port = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};

/*****************************************
 *  DHCP_SNP_INFO                        *
 *****************************************/
#ifdef AIR_SUPPORT_DHCP_SNOOP
const DB_DHCP_SNP_INFO_T _dhcp_snp_info_dft =
{
    .enable = 0,
};

const DB_DHCP_SNP_PORT_INFO_T _dhcp_snp_port_dft =
{
    .trust_port = 0,
    .opt82_enable = 0,
    .opt82_mode = 0,
    .circuit_id_type = 0,
    .circuit_id = {0},
    .remote_id_type = 0,
    .remote_id = {0}
};
#endif
#ifdef AIR_SUPPORT_SNMP
const DB_SNMP_INFO_T _snmp_info_dft =
{
    .version = 0x0,
    .trap_hostname = "\0",
    .trap_enable = 0x0,
    .trap_type = 0x0,
    .trap_dst_ip = 0x0100A8C0,
    .read_community = "public",
    .set_community = "private",
    .trap_community = "public"
};
#endif
#ifdef AIR_SUPPORT_LLDPD
const DB_LLDP_INFO_T _lldp_info_dft =
{
    .enable = 1,
    .tx_hold_multipler = 4,
    .tx_interval = 30,
    .reinit_delay = 2,
    .tx_delay = 2
};

const DB_LLDP_PORT_INFO_T _lldp_port_info_dft =
{
    .enable = 3,
};
#endif
/*****************************************
 *  RSTP                                 *
 *****************************************/
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
const DB_STP_INFO_T _stp_info_dft =
{
    .enable                  = 0,
#ifdef AIR_SUPPORT_RSTP
    .force_version           = 2,
#else
    .force_version           = 3,
#endif
    .forward_delay           = 15,
    .max_age                 = 20,
    .transmit_hold_count     = 6,
    .priority                = 32768,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    .auto_cost_support       = 0,
#endif  /* AIR_SUPPORT_STP_AUTO_COST */
    .portMode                = 0,
#ifdef AIR_SUPPORT_RSTP_SECURITY
    .bg_state                = 0,
    .rg_state                = 0,
    .lg_state                = 0,
#endif
};

const DB_STP_PORT_INFO_T _stp_port_info_dft =
{
    .priority                = 128,
    .cost                    = 20000,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    .auto_cost_enable        = 0,
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    .admin_edge              = 0,
    .mcheck                  = 0,
    .status                  = 0,
};

#ifdef AIR_SUPPORT_RSTP_SECURITY
const DB_RSTP_PORT_SEC_T _rstp_port_sec_dft =
{
    .bpdu_status             = 0,
    .tc_status               = 0,
    .root_status             = 0,
    .loop_status             = 0,
    .tc_interval             = 5,
    .tc_threshold            = 0,
};
#endif

#ifdef AIR_SUPPORT_MSTP
const DB_MSTP_REGION_CONFIG_T _mstp_region_dft =
{
    .revision                = 0,
    .region_name             = {0},
};

const DB_MSTP_INSTANCE_T _mstp_instance_dft =
{
    .instance                = 0,
    .priority                = 32768,
    .vlanbmp                 = 0,
};

const DB_MSTP_SINGLE_INSTANCE_PORT_INFO_T _mstp_instance_port_dft =
{
    .priority = 128,
    .cost     = 20000,
};
#endif

#endif /* AIR_SUPPORT_RSTP */

/*****************************************
 *  MQTTD                                *
 *****************************************/
#ifdef AIR_SUPPORT_MQTTD
const DB_MQTTD_CFG_INFO_T _mqttd_cfg_info_dft =
{
#ifdef AIR_SUPPORT_MQTTD_STARTONBOOT
    .enable = 1
#else
    .enable = 0
#endif
};
#endif /* AIR_SUPPORT_MQTTD */

#ifdef AIR_SUPPORT_POE
const DB_POE_CFG_T _poe_cfg_dft =
{
    .total_available_power = 1200,
#ifdef AIR_SUPPORT_POE_WATCHDOG
    .poe_watchdog_period    = 300,
    .poe_watchdog_threshold = 0,
#endif

};
const DB_POE_PORT_CFG_T _poe_port_cfg_dft =
{
    .port_power_control        = 1,
    .port_available_power_mode = 0,
    .port_available_power      = 360,
    .port_priority             = 0,
    .port_delay_time           = 0,
    .port_af_at_mode = 1,
#ifdef AIR_SUPPORT_POE_WATCHDOG
    .port_poe_watchdog_enable  = 0,
#endif
};
#endif

#ifdef AIR_SUPPORT_SNTP
const DB_SNTP_CFG_T _sntp_cfg_dft =
{
    .sntp_mode = 0,
    .sntp_timezone = 0,
    .sntp_server1 = 0,
    .sntp_server2 = 0,
    .sntp_server3 = 0,
};
#endif

#ifdef AIR_SUPPORT_ERPS
const DB_ERPS_INFO_T _erps_info_dft =
{
    .ring_id = {0},
    .control_vlan = {0},
    .data_vlan = {""},
    .revertive_mode = {0},
    .west_port = {0},
    .east_port = {0},
    .wtr_timer = {5},
    .guard_timer = {50},
    .hold_off_timer = {0},
    .west_port_state = {0},
    .east_port_state = {0},
    .instance_state = {0},
};
#endif /*AIR_SUPPORT_ERPS*/
/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static const DB_DEFAULT_CONFIG_TABLE_T _db_default_config_table[DB_FIRST_OPER_TABLE] =
{
    {&_sys_info_dft, sizeof(_sys_info_dft)},
    {&_account_info_dft, sizeof(_account_info_dft)},
    {&_port_cfg_info_dft, sizeof(_port_cfg_info_dft)},
    {&_port_qos_dft, sizeof(_port_qos_dft)},
    {&_trunk_port_dft, sizeof(_trunk_port_dft)},
    {&_trunk_algo_dft, sizeof(_trunk_algo_dft)},
#ifdef AIR_SUPPORT_LP
    {&_loop_preven_info_dft, sizeof(_loop_preven_info_dft)},
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_IGMP_SNP
    {&_igmp_snp_info_dft, sizeof(_igmp_snp_info_dft)},
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    {&_igmp_snp_querier_info_dft, sizeof(_igmp_snp_querier_info_dft)},
#endif
#endif /* AIR_SUPPORT_IGMP_SNP */
    {&_port_mirror_info_dft, sizeof(_port_mirror_info_dft)},
    {&_jumbo_frame_info_dft, sizeof(_jumbo_frame_info_dft)},
    {&_vlan_cfg_info_dft, sizeof(_vlan_cfg_info_dft)},
    {&_vlan_entry_dft, sizeof(_vlan_entry_dft)},
#ifdef AIR_SUPPORT_VOICE_VLAN
    {&_voice_vlan_info_dft, sizeof(_voice_vlan_info_dft)},
#endif /* AIR_SUPPORT_VOICE_VLAN */
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    {&_survei_vlan_info_dft, sizeof(_survei_vlan_info_dft)},
#endif /* AIR_SUPPORT_SURVEILLANCE_VLAN */
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    {&_oui_entry_dft, sizeof(_oui_entry_dft)},
#endif
    {&_qos_info_dft, sizeof(_qos_info_dft)},
    {&_static_mac_entry_dft, sizeof(_static_mac_entry_dft)},
#ifdef AIR_SUPPORT_DHCP_SNOOP
    {&_dhcp_snp_info_dft, sizeof(_dhcp_snp_info_dft)},
    {&_dhcp_snp_port_dft, sizeof(_dhcp_snp_port_dft)},
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    {&_snmp_info_dft, sizeof(_snmp_info_dft)},
#endif /* AIR_SUPPORT_SNMP */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    {&_stp_info_dft, sizeof(_stp_info_dft)},
    {&_stp_port_info_dft, sizeof(_stp_port_info_dft)},
#endif /* AIR_SUPPORT_RSTP || MSTP */
#ifdef AIR_SUPPORT_MQTTD
    {&_mqttd_cfg_info_dft, sizeof(_mqttd_cfg_info_dft)},
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_LLDPD
    {&_lldp_info_dft, sizeof(_lldp_info_dft)},
    {&_lldp_port_info_dft, sizeof(_lldp_port_info_dft)},
#endif /* AIR_SUPPORT_LLDPD */
#ifdef AIR_SUPPORT_POE
    {&_poe_cfg_dft, sizeof(_poe_cfg_dft)},
    {&_poe_port_cfg_dft, sizeof(_poe_port_cfg_dft)},
#endif /* AIR_SUPPORT_POE */
#ifdef AIR_SUPPORT_SNTP
    {&_sntp_cfg_dft, sizeof(_sntp_cfg_dft)},
#endif /* AIR_SUPPORT_SNTP */
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    {&_rstp_port_sec_dft, sizeof(_rstp_port_sec_dft)},
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_ERPS
    {&_erps_info_dft, sizeof(_erps_info_dft)},
#endif /* AIR_SUPPORT_ERPS */
#ifdef AIR_SUPPORT_MSTP
    {&_mstp_region_dft, sizeof(_mstp_region_dft)},
    {&_mstp_instance_dft, sizeof(_mstp_instance_dft)},
    {&_mstp_instance_port_dft, sizeof(_mstp_instance_port_dft)},
#endif /* AIR_SUPPORT_MSTP */
};
/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: db_getDefaultCfgTable
 * PURPOSE:
 *      This function is used to get the default configuration table.
 *
 * INPUT:
 *      t_idx                -- Table index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to the default configuration table.
 *
 * NOTES:
 *
 */
DB_DEFAULT_CONFIG_TABLE_T *
db_getDefaultCfgTable(
    const UI8_T t_idx)
{
    DB_DEFAULT_CONFIG_TABLE_T *ptr_config_table = NULL;

    if (DB_FIRST_OPER_TABLE > t_idx)
    {
        ptr_config_table = (DB_DEFAULT_CONFIG_TABLE_T *) &(_db_default_config_table[t_idx]);
    }

    return ptr_config_table;
}


