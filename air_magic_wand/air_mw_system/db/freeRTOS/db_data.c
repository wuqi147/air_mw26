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

/* FILE NAME:   db_data.c
 * PURPOSE:
 *      Database data handling functions
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "osapi_memory.h"
#include "osapi_flash.h"
#include "osapi_mutex.h"
#include "db_data.h"
#include "db_main.h"
#include "db_util.h"
#include "mw_utils.h"
#include "vlan_utils.h"
#include "mw_portbmp.h"
#include "mw_platform.h"
#include "mw_tlv.h"
#ifdef AIR_SUPPORT_POE
#include "poe_config_customer.h"
#endif
#include "inet_utils.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DB_BYTE_OFFT                 (8)
#define DB_CONFIG_MAGIC_LEN          (3)
#define DB_CONFIG_MAGIC_VER_LEN      (6)
#define DB_CONFIG_MAX_MAGIC_LEN      (DB_CONFIG_MAGIC_VER_LEN)
#define DB_FLASH_CONFIGFILE_SIZE     (32 * 1024) /* half of SystemConfigSize, store
                                                    header with factory-default and stratup-config */
#define DB_MAGIC_WORD                "Air"
#define DB_MAGIC_WORD_VER            "AIR_V1"
#define DB_CONFIG_FACTORY_PART       (SystemConfig0)
#define DB_CONFIG_STARTUP_PART       (SystemConfig1)
#define DB_GET_FLASH_DATA_PART       DB_CONFIG_FACTORY_PART
#define DB_CONFIG_VER_0              (0)
#define DB_CONFIG_VER_1              (1)

/* System operational information */
static const DB_SYS_OPER_INFO_T _sys_oper_info_dft =
{
    .sys_mac = {0x00, 0xAA, 0xBB, 0x11, 0x22, 0x66},
    .hw_version = AIR_DEFAULT_SYSTEM_NAME,
    .ip_addr = 0xC800A8C0,
    .ip_mask = 0x00FFFFFF,
    .ip_gw = 0,
    .ip_dns = 0,
#ifdef AIR_SUPPORT_SECOND_NETIF
    .if2_ip_addr = 0x01DCA8C0,
    .if2_ip_mask = 0x00FFFFFF,
#endif
#ifdef AIR_SUPPORT_IPV6
    .ip6_addr_1 = {{0}},
    .ip6_addr_2 = {{0}},
    .ip6_link_local_addr = {{0}},
    .dad_result = 0,
#endif
};

/* DATA TYPE DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static MW_ERROR_NO_T _db_regist_ctrl(UI8_T t_num, void *ptr_handle, UI16_T e_num, UI8_T f_num, ...);
static MW_ERROR_NO_T _db_ctrl_init(DB_TABLES_T *ptr_db, DB_CTRL_T *ptr_ctrl);
static MW_ERROR_NO_T _db_load_default(UI8_T *ptr_raw_db);
static MW_ERROR_NO_T _db_is_cfg_exist(UI32_T cfg_addr, UI8_T part, UI8_T  *ptr_ver);
static MW_ERROR_NO_T _db_data_init(DB_TABLES_T *ptr_db);
static MW_ERROR_NO_T _db_set_reset_factory_flag(void);
static MW_ERROR_NO_T _db_data_save_running(BOOL_T to_factory);
static MW_ERROR_NO_T _db_data_parse_cfg(UI32_T offset, UI32_T file_size, UI8_T part, UI8_T flag, DB_REQUEST_TYPE_T request, void *ptr_out_data);
static MW_ERROR_NO_T _db_data_parse_cfg_ext(UI32_T offset, UI32_T file_size, UI8_T part, UI8_T flag, DB_REQUEST_TYPE_T request, void *ptr_out_data);
static MW_ERROR_NO_T _db_parse_payload(DB_PAYLOAD_T *ptr_payload, DB_REQUEST_TYPE_T *ptr_out_req);
static MW_ERROR_NO_T
_db_parse_tlv_data(
    UI8_T type);
static MW_ERROR_NO_T _db_system_ctrl(UI8_T t_idx, UI8_T f_idx, UI8_T *ptr_data);
static MW_ERROR_NO_T
_db_data_update_data(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data);

static MW_ERROR_NO_T
_db_setDataToRawTable(
    UI8_T *ptr_raw_data,
    const UI8_T *ptr_input_data,
    const DB_REQUEST_TYPE_T req);

static MW_ERROR_NO_T
_db_getFlashCfgValue(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const UI8_T part,
    UI16_T *ptr_data_size,
    void **pptr_data);

static MW_ERROR_NO_T
_db_getDefaultCfgValue(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data);

static MW_ERROR_NO_T
_db_amendDefaultCfgValue(
    const UI8_T t_idx,
    UI8_T *ptr_raw_data);

/* STATIC VARIABLE DECLARATIONS
 */
static DB_CORE_T _db_core;

/* LOCAL SUBPROMGRAM BODIES
*/
/* Initialize the db control structure
*/
static MW_ERROR_NO_T
_db_regist_ctrl(
    UI8_T t_num,
    void *ptr_handle,
    UI16_T e_num,
    UI8_T f_num,
    ...)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_fields = NULL;
    UI8_T num = 0;
    va_list args;


    if ((NULL == ptr_handle) || (NULL == ptr_ctrl))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    if (TABLES_LAST <= t_num)
    {
        DB_LOG_ERROR("%s", "bad parameter");
        return MW_E_OTHERS;
    }

    /* Switch to table's control */
    ptr_ctrl += t_num;
    ptr_ctrl->table_handle = ptr_handle;
    ptr_ctrl->entry_num = e_num;
    ptr_ctrl->field_num = f_num;
    ptr_ctrl->factory_offt = DB_NO_FLASH_CONFIG;
    ptr_ctrl->startup_offt = DB_NO_FLASH_CONFIG;

    /* Register the size of each field in the table */
    ret = db_calloc(f_num, (void **)&(ptr_ctrl->fields_size));
    if (MW_E_OK != ret)
    {
        return ret;
    }
    ptr_fields = ptr_ctrl->fields_size;
    va_start(args, f_num);
    for (num = 0; num < f_num; num++)
    {
        *ptr_fields++ = va_arg(args, int);
    }
    va_end(args);


    return ret;
}

static MW_ERROR_NO_T
_db_ctrl_init(
    DB_TABLES_T *ptr_db,
    DB_CTRL_T *ptr_ctrl)
{
#ifdef DB_DBG
    UI8_T           t_num = 0, f_num = 0;
    UI8_T           *ptr_fieldsizes = NULL;
    UI32_T          total_size = 0, table_size = 0;
    DB_CTRL_T       *ptr_cur_ctrl = ptr_ctrl;
#endif
    UI32_T          offt = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;
#ifdef AIR_SUPPORT_POE
    UI8_T                        poe_port_number = 0;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
#endif

    if ((NULL == ptr_db) || (NULL == ptr_ctrl))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    /*system information */
    _db_regist_ctrl(SYS_INFO, (void *)&(ptr_db->sys_info) + offt, 1, (SYS_INFO_LAST - 1),
            (MAX_SYS_NAME_SIZE), MAX_VERSION_SIZE, 1, 1, 4, 4, 4, 4
#ifdef AIR_SUPPORT_SECOND_NETIF
            , 4, 4
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
            ,2
#endif
#ifdef AIR_SUPPORT_IPV6
            ,1
            ,sizeof(ip6_addr_t)
            ,sizeof(ip6_addr_t)
            ,1
#endif
            );
    /* User Account */
    _db_regist_ctrl(ACCOUNT_INFO, (void *)&(ptr_db->account_info) + offt, 1, (ACC_INFO_LAST - 1),
            MAX_USER_NAME_SIZE, MAX_PASSWORD_SIZE);
    /* Port Configuration */
    _db_regist_ctrl(PORT_CFG_INFO, (void *)&(ptr_db->port_cfg) + offt, PLAT_MAX_PORT_NUM, (PORT_CFG_INFO_LAST - 1),
            1, 1, 2, 4, 4, 4, 1, 1, 2, 1, 1, 4, 2, 4, 4, 4, 1, 1, 1, 1, 1, 1, 1, 1);
    offt += sizeof(DB_PORT_CFG_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    /* Port QoS configuration */
    _db_regist_ctrl(PORT_QOS, (void *)&(ptr_db->port_qos) + offt, PLAT_MAX_PORT_NUM, (PORT_QOS_LAST - 1),
            1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    offt += sizeof(DB_PORT_QOS_T) * (PLAT_MAX_PORT_NUM -1);
    /* Trunk interface configuration */
    _db_regist_ctrl(TRUNK_PORT, (void *)&(ptr_db->trunk_port) + offt, MAX_TRUNK_NUM, (TRUNK_PORT_LAST - 1),
            5);

    offt += sizeof(DB_TRUNK_PORT_T) * (MAX_TRUNK_NUM - 1);
    /* Trunk algorithm configuration */
    _db_regist_ctrl(TRUNK_ALGORITHM, (void *)&(ptr_db->trunk_algo) + offt, 1, (TRUNK_ALGORITHM_LAST - 1),
            1);

#ifdef AIR_SUPPORT_LP
    /* Loop Prevention configuration */
    _db_regist_ctrl(LOOP_PREVEN_INFO, (void *)&(ptr_db->loop_preven) + offt, 1, (LOOP_PREVEN_INFO_LAST - 1),
            1, 4);
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_IGMP_SNP
    /* IGMP Snooping configuration */
    _db_regist_ctrl(IGMP_SNP_INFO, (void *)&(ptr_db->igmp_snp) + offt, 1, (IGMP_SNP_INFO_LAST - 1),
            1, 1, 1);
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    /* IGMP Snooping querier configuration */
    _db_regist_ctrl(IGMP_SNP_QUERIER_INFO, (void *)&(ptr_db->igmp_snp_querier) + offt, MAX_VLAN_ENTRY_NUM, (IGMP_SNP_QUERIER_INFO_LAST - 1),
            2, 1, 1, 1, 4);
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */
    /* Port mirror sessions configuration */
    _db_regist_ctrl(PORT_MIRROR_INFO, (void *)&(ptr_db->port_mirror) + offt, MAX_MIRROR_SESS_NUM, (PORT_MIRROR_INFO_LAST - 1),
            1, 1, 4, 4);
    offt += sizeof(DB_PORT_MIRROR_INFO_T) * (MAX_MIRROR_SESS_NUM - 1);
    /* Jumbo Frame configuration */
    _db_regist_ctrl(JUMBO_FRAME_INFO, (void *)&(ptr_db->jumbo_frame) + offt, 1, (JUMBO_FRAME_INFO_LAST - 1),
            4);
    /* VLAN configuration */
    _db_regist_ctrl(VLAN_CFG_INFO, (void *)&(ptr_db->vlan_cfg) + offt, 1, (VLAN_CFG_INFO_LAST - 1),
            1, 1, 1);
    /* VLANs information */
    _db_regist_ctrl(VLAN_ENTRY, (void *)&(ptr_db->vlan_entry) + offt, MAX_VLAN_ENTRY_NUM, (VLAN_ENTRY_LAST - 1),
            2,
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
            1,
#endif
            12, 4, 4, 4);
#ifdef AIR_SUPPORT_VOICE_VLAN
    /* Voice VLAN configuration */
    _db_regist_ctrl(VOICE_VLAN_INFO, (void *)&(ptr_db->voice_vlan) + offt, 1, (VOICE_VLAN_INFO_LAST - 1),
            1, 2, 1, 4
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
            ,4
#endif
            );
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    /* Surveillance VLAN configuration */
    _db_regist_ctrl(SURVEI_VLAN_INFO, (void *)&(ptr_db->survei_vlan) + offt, 1, (SURVEI_VLAN_INFO_LAST - 1),
            1, 2, 1);
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    /* OUI entries for voice vlan or surveillance vlan */
    _db_regist_ctrl(OUI_ENTRY, (void *)&(ptr_db->oui_entry) + offt, MAX_OUI_NUM, (OUI_ENTRY_LAST - 1),
            3, 1, 11);
#endif
    /* QoS configuration */
    _db_regist_ctrl(QOS_INFO, (void *)&(ptr_db->qos_info) + offt, 1, (QOS_INFO_LAST - 1),
            1, MAX_QUEUE_NUM, MAX_DSCP_NUM);
    /* Static MAC table */
    _db_regist_ctrl(STATIC_MAC_ENTRY, (void *)&(ptr_db->static_mac_entry) + offt, MAX_STATIC_MAC_NUM, (STATIC_MAC_ENTRY_LAST - 1),
            6, 2, 2);
#ifdef AIR_SUPPORT_DHCP_SNOOP
    /* DHCP snooping configuration */
    _db_regist_ctrl(DHCP_SNP_INFO, (void *)&(ptr_db->dhcp_snp) + offt, 1, (DHCP_SNP_INFO_LAST - 1),
            1);
    /* DHCP snooping option 82 configuration */
    _db_regist_ctrl(DHCP_SNP_PORT_INFO, (void *)&(ptr_db->dhcp_snp_port) + offt, PLAT_MAX_PORT_NUM, (DHCP_SNP_PORT_INFO_LAST - 1),
            1, 1, 1, 1, (MAX_OPT82_ID_LEN), 1, (MAX_OPT82_ID_LEN));
    offt += sizeof(DB_DHCP_SNP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    _db_regist_ctrl(SNMP_INFO, (void *)&(ptr_db->snmp_info) + offt, 1, (SNMP_LAST - 1),
            1, (MAX_HOST_NAME_SIZE), 1, 1, 4, MAX_SNMP_CM_LEN, MAX_SNMP_CM_LEN, MAX_SNMP_CM_LEN);
#endif
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    /* RSTP configuration */
#ifdef AIR_SUPPORT_STP_AUTO_COST
#ifdef AIR_SUPPORT_RSTP_SECURITY
    _db_regist_ctrl(STP_INFO, (void *)&(ptr_db->stp) + offt, 1, (STP_INFO_LAST - 1),
            1, 4, 2, 2, 2, 2, 1, 4, 4, 4, 4);
#else
    _db_regist_ctrl(STP_INFO, (void *)&(ptr_db->stp) + offt, 1, (STP_INFO_LAST - 1),
            1, 4, 2, 2, 2, 2, 1, 4);
#endif
#else
#ifdef AIR_SUPPORT_RSTP_SECURITY
    _db_regist_ctrl(STP_INFO, (void *)&(ptr_db->stp) + offt, 1, (STP_INFO_LAST - 1),
            1, 4, 2, 2, 2, 2, 4, 4, 4, 4);
#else
    _db_regist_ctrl(STP_INFO, (void *)&(ptr_db->stp) + offt, 1, (STP_INFO_LAST - 1),
            1, 4, 2, 2, 2, 2, 4);
#endif
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    /* RSTP port configuration */
#ifdef AIR_SUPPORT_STP_AUTO_COST
    _db_regist_ctrl(STP_PORT_INFO, (void *)&(ptr_db->stp_port) + offt, PLAT_MAX_PORT_NUM, (STP_PORT_INFO_LAST - 1),
            1, 4, 1, 1, 1, 1);
#else /* AIR_SUPPORT_STP_AUTO_COST */
    _db_regist_ctrl(STP_PORT_INFO, (void *)&(ptr_db->stp_port) + offt, PLAT_MAX_PORT_NUM, (STP_PORT_INFO_LAST - 1),
            1, 4, 1, 1, 1);
#endif
    offt += sizeof(DB_STP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
    /* MQTTD configuration */
    _db_regist_ctrl(MQTTD_CFG_INFO, (void *)&(ptr_db->mqttd_cfg) + offt, 1, (MQTTD_CFG_INFO_LAST - 1),
            1);
#endif /* AIR_SUPPORT_MQTTD */
#ifdef AIR_SUPPORT_LLDPD
    _db_regist_ctrl(LLDP_INFO, (void *)&(ptr_db->lldp_info) + offt, 1, (LLDP_INFO_LAST - 1),
            1, 1, 2, 2, 2);
    _db_regist_ctrl(LLDP_PORT_INFO, (void *)&(ptr_db->lldp_port) + offt, PLAT_MAX_PORT_NUM, (LLDP_PORT_INFO_LAST - 1),
            1);
    offt += sizeof(DB_LLDP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#endif
#ifdef AIR_SUPPORT_POE
    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL != ptr_poeCfg)
    {
        poe_port_number = ptr_poeCfg->poe_port_number;
    }
    else
    {
        poe_port_number = 1;
    }
    _db_regist_ctrl(POE_CFG, (void *)&(ptr_db->poe_cfg) + offt, 1, (POE_CFG_LAST - 1),
            2
#ifdef AIR_SUPPORT_POE_WATCHDOG
            , 2, 2
#endif
            );
    _db_regist_ctrl(POE_PORT_CFG, (void *)&(ptr_db->poe_port_cfg) + offt, poe_port_number, (POE_PORT_CFG_LAST - 1),
            1, 1, 2, 1, 1, 1
#ifdef AIR_SUPPORT_POE_WATCHDOG
            , 1
#endif
            );
    offt += sizeof(DB_POE_PORT_CFG_T) * (poe_port_number - 1);
#endif
#ifdef AIR_SUPPORT_SNTP
    _db_regist_ctrl(SNTP_CFG, (void *)&(ptr_db->sntp_cfg) + offt, 1, (SNTP_CFG_LAST - 1),
            1, 1, 4, 4, 4);
#endif
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    _db_regist_ctrl(RSTP_PORT_SECURITY, (void *)&(ptr_db->rstp_port_sec) + offt, PLAT_MAX_PORT_NUM, (RSTP_PORT_SEC_LAST - 1),
            1, 1, 1, 1, 2, 2);
    offt += sizeof(DB_RSTP_PORT_SEC_T) * (PLAT_MAX_PORT_NUM - 1);
#endif

#ifdef AIR_SUPPORT_ERPS
    _db_regist_ctrl(ERPS_INFO, (void *)&(ptr_db->erps_info) + offt, MAX_ERPS_INSTANCE_NUM, (ERPS_INFO_LAST - 1),
            1, 2, MAX_ERPS_DATA_VLAN_SIZE, 1, 1, 1, 1, 1, 1, 1, 1, 1);
#endif
#ifdef AIR_SUPPORT_MSTP
    _db_regist_ctrl(MSTP_REGION, (void *)&(ptr_db->mstp_region) + offt, 1, (MSTP_REGION_LAST - 1),
            2, MAX_MSTP_REGION_NAME_SIZE);
    _db_regist_ctrl(MSTP_INSTANCE, (void *)&(ptr_db->mstp_instance) + offt, MAX_MSTP_INSTANCE_NUM, (MSTP_INSTANCE_LAST - 1),
            2, 2, 4);
    offt += sizeof(DB_MSTP_INSTANCE_T) * (MAX_MSTP_INSTANCE_NUM - 1);

    _db_regist_ctrl(MSTP_INSTANCE_PORT, (void *)&(ptr_db->mstp_ins_port) + offt, PLAT_MAX_PORT_NUM, (MSTP_INSTANCE_PORT_LAST - 1),
            (sizeof(UI8_T)*MAX_MSTP_INSTANCE_NUM), (sizeof(UI32_T)*MAX_MSTP_INSTANCE_NUM));
    offt += sizeof(DB_MSTP_INSTANCE_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#endif

    /* System operational information */
    _db_regist_ctrl(SYS_OPER_INFO, (void *)&(ptr_db->sys_oper_info) + offt, 1, (SYS_OPER_INFO_LAST - 1),
            6, MAX_VERSION_SIZE, 4, 4, 4 ,4
#ifdef AIR_SUPPORT_SECOND_NETIF
            , 4, 4
#endif
#ifdef AIR_SUPPORT_IPV6
            ,sizeof(ip6_addr_t), sizeof(ip6_addr_t), sizeof(ip6_addr_t),
            1
#endif
            );
    /* Port operational information */
    _db_regist_ctrl(PORT_OPER_INFO, (void *)&(ptr_db->port_oper) + offt, PLAT_MAX_PORT_NUM, (PORT_OPER_INFO_LAST - 1),
            1, 1, 1, 1, 1, 1, 1, 1, 1);
    offt += sizeof(DB_PORT_OPER_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    _db_regist_ctrl(LAG_OPER_INFO, (void *)&(ptr_db->lag_oper_info) + offt, 1, (LAG_OPER_INFO_LAST - 1),
            1);
    _db_regist_ctrl(LAG_MEMBER_0_INFO, (void *)&(ptr_db->lag_member_0) + offt, MAX_TRUNK_NUM, (LAG_MEMBER_0_LAST - 1),
            1);
#ifdef AIR_SUPPORT_CABLE_DIAG
    /* Port Cable Diagnostic */
    _db_regist_ctrl(PORT_DIAG, (void *)&(ptr_db->port_diag) + offt, MAX_PAIR_NUM, (PORT_CABLE_DIAG_LAST - 1),
            1, 2, 1, 2, 1);
#endif
    /*Logon Information */
    _db_regist_ctrl(LOGON_INFO, (void *)&(ptr_db->logon_info) + offt, 1, (LOGON_INFO_LAST - 1),
            1);
    /* IGMP Snooping learned entries */
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    _db_regist_ctrl(L2_MC_ENTRY, (void *)&(ptr_db->l2_mc_entry) + offt, MAX_L2MC_NUM, (L2_MC_ENTRY_LAST - 1),
            6, 2, 4, 1);
#else
    _db_regist_ctrl(L2_MC_ENTRY, (void *)&(ptr_db->l2_mc_entry) + offt, MAX_L2MC_NUM, (L2_MC_ENTRY_LAST - 1),
            4, 2, 4, 1);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    /* System Conctrol */
    _db_regist_ctrl(SYSTEM, (void *)&(ptr_db->system) + offt, 1, (SYSTEM_LAST - 1),
            1, 1, 1);
#ifdef AIR_SUPPORT_ICMP_CLIENT
    _db_regist_ctrl(ICMP_CLIENT_INFO, (void *)&(ptr_db->icmp_client_info) + offt, 1, (ICMP_CLIENT_INFO_LAST - 1),
            (MAX_HOST_NAME_SIZE), 4, 2, 2, 2, 2, 2, 2, 2, 2);
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
    /* SFP module information */
    _db_regist_ctrl(SFP_MODULE_INFO,  (void *)&(ptr_db->sfp_module_info) + offt, SFP_MODULE_INFO_MAX_NUM, (SFP_MODULE_INFO_LAST - 1),
            1, 1, 2, 2, 2, 2, 2, 1);
    offt += sizeof(DB_SFP_MODULE_INFO_T) * (SFP_MODULE_INFO_MAX_NUM - 1);
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
    /* Voice VLAN operational state */
    _db_regist_ctrl(VOICE_OPER_INFO, (void *)&(ptr_db->voice_oper) + offt, 1, (VOICE_OPER_INFO_LAST - 1),
            4);
#endif
    /* Port operational information */
    _db_regist_ctrl(MIB_CNT, (void *)&(ptr_db->mib_cnt) + offt, PLAT_MAX_PORT_NUM, (MIB_CNT_LAST - 1),
#ifndef AIR_SUPPORT_SNMP
            8, 8, 8, 8, 8, 8
#else
            8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
#endif
            );
    offt += sizeof(DB_MIB_CNT_T) * (PLAT_MAX_PORT_NUM - 1);
    /* Dynamic MAC configuration */
    _db_regist_ctrl(DYNAMIC_MAC_ADDRESS_ENTRY_CFG, (void *)&(ptr_db->dynamic_mac_address_entry_cfg) + offt, 1, (DYNAMIC_MAC_ADDRESS_ENTRY_CFG_LAST - 1),
                1, 1);
    /* Dynamic MAC table */
    _db_regist_ctrl(DYNAMIC_MAC_ADDRESS_ENTRY, (void *)&(ptr_db->dynamic_mac_address_entry) + offt, MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM, (DYNAMIC_MAC_ADDRESS_ENTRY_LAST - 1),
                6, 2, 4, 4);

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#ifdef AIR_SUPPORT_STP_AUTO_COST
    _db_regist_ctrl(STP_PORT_OPER_INFO, (void *)&(ptr_db->stp_port_oper) + offt, PLAT_MAX_PORT_NUM, (STP_PORT_OPER_INFO_LAST - 1),
            1, 1, 1, 4);
#else
    _db_regist_ctrl(STP_PORT_OPER_INFO, (void *)&(ptr_db->stp_port_oper) + offt, PLAT_MAX_PORT_NUM, (STP_PORT_OPER_INFO_LAST - 1),
            1, 1, 1);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    offt += sizeof(DB_STP_PORT_OPER_T) * (PLAT_MAX_PORT_NUM - 1);
#ifdef AIR_SUPPORT_MSTP
#ifdef AIR_SUPPORT_STP_AUTO_COST
    _db_regist_ctrl(MSTP_PORT_OPER_INFO, (void *)&(ptr_db->mstp_ins_port_oper) + offt, PLAT_MAX_PORT_NUM, (MSTP_INSTANCE_PORT_OPER_INFO_LAST - 1),
            (1*MAX_MSTP_INSTANCE_NUM), (1*MAX_MSTP_INSTANCE_NUM), (4*MAX_MSTP_INSTANCE_NUM));
#else
    _db_regist_ctrl(MSTP_PORT_OPER_INFO, (void *)&(ptr_db->mstp_ins_port_oper) + offt, PLAT_MAX_PORT_NUM, (MSTP_INSTANCE_PORT_OPER_INFO_LAST - 1),
            (1*MAX_MSTP_INSTANCE_NUM), (1*MAX_MSTP_INSTANCE_NUM));
#endif
    offt += sizeof(DB_MSTP_INSTANCE_PORT_OPER_T) * (PLAT_MAX_PORT_NUM - 1);
#endif
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_LLDPD
    _db_regist_ctrl(LLDP_CLIENT_INFO, (void *)&(ptr_db->lldp_client) + offt, PLAT_MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT, (LLDP_CLIENT_INFO_LAST - 1),
            MAX_LLDP_STR_SIZE, MAX_LLDP_STR_SIZE, 2, MAX_LLDP_STR_SIZE, 4,
            8, 4);
    offt += LLDP_CLIENT_INFO_SIZE * (PLAT_MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT - 1);
#endif
#ifdef AIR_SUPPORT_POE
    _db_regist_ctrl(POE_STATUS, (void *)&(ptr_db->poe_status) + offt, 1, (POE_STATUS_LAST - 1),
            2, 2, 4, 4, 1);
    _db_regist_ctrl(POE_PORT_STATUS, (void *)&(ptr_db->poe_port_status) + offt, poe_port_number, (POE_PORT_STATUS_LAST - 1),
            1, 1, 2, 2, 2, 1
        );
    offt += sizeof(DB_POE_PORT_STATUS_T) * (poe_port_number - 1);
#endif

#ifdef DB_DBG
    /* Error check */
    if ((sizeof(DB_TABLES_T) + offt) != _db_core.table_size)
    {
        DB_LOG_ERROR("DB ctrl data init incomplete or incorrect. table size:%u, sizeof(DB_TABLES_T):%u, offt:%u", _db_core.table_size, sizeof(DB_TABLES_T), offt);
        rc = MW_E_BAD_PARAMETER;
    }
    for (t_num = 0; t_num < TABLES_LAST; t_num++)
    {
        table_size = 0;
        ptr_cur_ctrl = ptr_ctrl + t_num;
        ptr_fieldsizes = ptr_cur_ctrl->fields_size;
        if (NULL == ptr_fieldsizes)
        {
            DB_LOG_ERROR("table:%d's ctrl data has no field size info.", t_num);
            continue;
        }
        for (f_num = 0; f_num < ptr_cur_ctrl->field_num; f_num++)
        {
            table_size += ptr_fieldsizes[f_num];
        }
        table_size *= ptr_cur_ctrl->entry_num;
        total_size += table_size;
    }
    if (total_size != _db_core.table_size)
    {
        DB_LOG_ERROR("DB ctrl data init incomplete or incorrect. alloc table size:%u, init size:%u", _db_core.table_size, total_size);
        rc = MW_E_BAD_PARAMETER;
    }
#endif /* DB_DBG */

    return rc;
}

/* Load default data to DB datastore
*/
static MW_ERROR_NO_T
_db_load_default(
    UI8_T *ptr_raw_db)
{
    UI8_T                       *ptr_cfg_data = NULL;
    UI8_T                       t_idx = 0;
    UI16_T                      table_size = 0;
    DB_CTRL_T                   *ptr_ctrl = NULL;
    MW_ERROR_NO_T               rc = MW_E_OK;

    UNUSED(ptr_raw_db);
    for (t_idx = 0; t_idx < DB_FIRST_OPER_TABLE; t_idx++)
    {
        rc = _db_getDefaultCfgValue(t_idx, DB_ALL_FIELDS, DB_ALL_ENTRIES, &table_size, (void **)&ptr_cfg_data);
        if (MW_E_OK != rc)
        {
            DB_LOG_ERROR("Load table(%d) default config failed, rc: %d", t_idx, rc);
            continue;
        }
        ptr_ctrl = _db_core.ptr_ctrl + t_idx;
        osapi_memcpy(ptr_ctrl->table_handle, ptr_cfg_data, table_size);
        MW_FREE(ptr_cfg_data);
    }

    return rc;
}

static MW_ERROR_NO_T
_db_is_cfg_exist(
        UI32_T offset,
        UI8_T  part,
        UI8_T  *ptr_ver)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T magic[DB_CONFIG_MAX_MAGIC_LEN + 1] = {0};

    MW_CHECK_PTR(ptr_ver);
    ret = osapi_flashRead(offset, sizeof(magic), (UI8_T *)magic, part);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: read config header from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
    }
    /* Check the magic word for recognization */
    if (0 == osapi_strncmp((C8_T *)magic, DB_MAGIC_WORD, DB_CONFIG_MAGIC_LEN))
    {
        *ptr_ver = DB_CONFIG_VER_0;
    }
    else if(0 == osapi_strncmp((C8_T *)magic, DB_MAGIC_WORD_VER, DB_CONFIG_MAGIC_VER_LEN))
    {
        *ptr_ver = DB_CONFIG_VER_1;
    }
    else
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    return ret;
}

static MW_ERROR_NO_T
_db_data_init(
        DB_TABLES_T *ptr_db)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T offset = 0;
    UI8_T  part = DB_CONFIG_FACTORY_PART, flag = 0;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    DB_REQUEST_TYPE_T request = {0, 0, 0};
    DB_SYS_OPER_INFO_T *ptr_sys_oper = NULL;
    UI8_T   config_ver = 0;
    UI32_T  unit       = 0;
    UI8_T   port_mode[MAX_PORT_NUM] = {0};

    if (NULL == ptr_db)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    /* Init order: Default_config.c --> Factory config --> TLV config --> Startup config */
    _db_load_default((UI8_T*)ptr_db);
    /* Read the factory-config */
    DB_LOG_INFO("%d: Load factory-config", part);
    flag = (DB_PARSE_CFG_OFFT | DB_PARSE_CFG_DATA);
    ret = _db_is_cfg_exist(offset, part, &config_ver);
    /* Read the flash config file if existed */
    if (MW_E_OK == ret)
    {
        /* Read the header that indicate to load factory-default file or startup-config file */
        if (DB_CONFIG_VER_1 == config_ver)
        {
            ret = _db_data_parse_cfg_ext(offset, DB_FLASH_CONFIGFILE_SIZE, part, flag, request, NULL);
        }
        else
        {
            /* Compatible with old config */
            ret = _db_data_parse_cfg(offset, DB_FLASH_CONFIGFILE_SIZE, part, flag, request, NULL);
        }
    }
    else
    {
        DB_LOG_INFO("The factory-config file does not exist.");
    }
    _db_parse_tlv_data(MW_TLV_TYPE_IP_ADDRESS);
    _db_parse_tlv_data(MW_TLV_TYPE_SUBNET_MASK);
    _db_parse_tlv_data(MW_TLV_TYPE_GATEWAY);

    part = DB_CONFIG_STARTUP_PART;
    DB_LOG_INFO("Parse startup-config, flag:0x%x", flag);
    ret = _db_is_cfg_exist(offset, part, &config_ver);
    if (MW_E_OK != ret)
    {
        DB_LOG_INFO("The startup config file does not exist.");
    }
    /* Read startup config file if existed */
    else
    {
        /* Read the header that indicate to load factory-default file or startup-config file */
        if(DB_CONFIG_VER_1 == config_ver)
        {
            _db_data_parse_cfg_ext(offset, DB_FLASH_CONFIGFILE_SIZE, part, flag, request, NULL);
        }
        else
        {
            /* Compatible with old config */
            _db_data_parse_cfg(offset, DB_FLASH_CONFIGFILE_SIZE, part, flag, request, NULL);
        }
    }

    /* Initialize the operational data */
    /* System operational information */
    ptr_ctrl = _db_core.ptr_ctrl + SYS_OPER_INFO;
    ptr_sys_oper = (DB_SYS_OPER_INFO_T*)ptr_ctrl->table_handle;
    memcpy(ptr_sys_oper, &_sys_oper_info_dft, sizeof(DB_SYS_OPER_INFO_T));
    _db_parse_tlv_data(MW_TLV_TYPE_HW_VERSION);
    memcpy(&(ptr_sys_oper->ip_addr), &(ptr_db->sys_info.static_ip), sizeof(MW_IPV4_T));
    memcpy(&(ptr_sys_oper->ip_mask), &(ptr_db->sys_info.static_mask), sizeof(MW_IPV4_T));
    memcpy(&(ptr_sys_oper->ip_gw), &(ptr_db->sys_info.static_gw), sizeof(MW_IPV4_T));
    memcpy(&(ptr_sys_oper->ip_dns), &(ptr_db->sys_info.static_dns), sizeof(MW_IPV4_T));
#ifdef AIR_SUPPORT_SECOND_NETIF
    memcpy(&(ptr_sys_oper->if2_ip_addr), &(ptr_db->sys_info.static_if2_ip), sizeof(MW_IPV4_T));
    memcpy(&(ptr_sys_oper->if2_ip_mask), &(ptr_db->sys_info.static_if2_mask), sizeof(MW_IPV4_T));
#endif

    ret = port_db_initPortMode(unit, MAX_PORT_NUM, port_mode);
    if (MW_E_OK == ret)
    {
        _db_data_update_data(PORT_OPER_INFO, PORT_OPER_MODE, DB_ALL_ENTRIES, port_mode);
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_db_set_reset_factory_flag(
    void)
{
    if(0 != flash_partition_copy(SystemConfig0, SystemConfig1, DB_FLASH_CONFIGFILE_SIZE))
    {
        return MW_E_OP_INCOMPLETE;
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_db_data_save_running(
    BOOL_T to_factory)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI16_T data_size = 0, flash_offst = 0;
    UI16_T print_size = 0;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_cfg = NULL;
    UI8_T *ptr_raw = NULL;
    DB_PAYLOAD_T *ptr_tmp = NULL;
    UI8_T  part = DB_CONFIG_FACTORY_PART;
    UI8_T  field_num = 0;
    UI16_T index = 0;
    /* Allocate a temp memory to store the config text and raw data */
    ret = db_calloc(DB_BUFFER_SIZE_512, (void **)&ptr_cfg);
    if ((MW_E_OK != ret) || (NULL == ptr_cfg))
    {
        return ret;
    }

    ret = db_calloc((DB_MSG_PAYLOAD_SIZE + DB_BUFFER_SIZE_256), (void **)&ptr_tmp);
    if ((MW_E_OK != ret) || (NULL == ptr_tmp))
    {
        osapi_free(ptr_cfg);
        return ret;
    }
    ptr_raw = (UI8_T *)&(ptr_tmp->ptr_data);

    /* Point to the partition */
    if (TRUE == to_factory)
    {
        part = DB_CONFIG_FACTORY_PART;
    }
    else
    {
        part = DB_CONFIG_STARTUP_PART;
    }

    if (MW_E_OK == osapi_mutexTake(_db_core.ptr_data_mutex, DB_Q_WAITTIME))
    {
        /* Start */
        /* Print the first start '{' */
        osapi_flashWrite(INIT, 0, NULL, part);
        data_size = osapi_sprintf((C8_T *)ptr_cfg, "%-7s%c", DB_MAGIC_WORD_VER, '{');
        flash_offst += data_size;
        while (ptr_tmp->request.t_idx < DB_FIRST_OPER_TABLE)
        {
            DB_LOG_DEBUG("t_idx: %d\n", ptr_tmp->request.t_idx);
            /* Update the flash config offset */
            if (TRUE == to_factory)
            {
                ptr_ctrl->factory_offt = flash_offst;
            }
            else
            {
                ptr_ctrl->startup_offt = flash_offst;
            }
            /* Print the table name */
            print_size = DB_BUFFER_SIZE_512 - data_size;
            ret = db_cfgfile_printTable(ptr_tmp->request.t_idx, ptr_cfg, &print_size);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: write the table name (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                break;
            }
            DB_APPEND_CHAR((C8_T *)ptr_cfg, print_size, "{");
            data_size += print_size;
            flash_offst += print_size;
            field_num = db_getTableFieldsNum(ptr_tmp->request.t_idx);
            /* Start to print the entries */
            ptr_tmp->request.f_idx = 1;
            ptr_tmp->request.e_idx = 0;
            while (ptr_tmp->request.f_idx <= field_num)
            {
                db_getDataSize(ptr_tmp->request, &(ptr_tmp->data_size));
                DB_LOG_DEBUG("\tf_idx: %d, db_data_size: %d, cfg_size: %d\n", ptr_tmp->request.f_idx, ptr_tmp->data_size, data_size);
                if((DB_BUFFER_SIZE_64 < ptr_tmp->data_size) ||
                   (MAX_PORT_NUM <= (field_num * 2)))
                {
                    for(index = 0; index < ptr_ctrl->entry_num; index++)
                    {
                        ptr_tmp->request.e_idx = index + 1;
                        db_getDataSize(ptr_tmp->request, &(ptr_tmp->data_size));
                        ret = db_getData(1, M_GET, ptr_tmp);
                        if (MW_E_OK != ret)
                        {
                            DB_LOG_ERROR("%s: get next data from DB (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                            break;
                        }
                        print_size = DB_BUFFER_SIZE_512 - data_size;
                        ret = db_cfgfile_saveField(ptr_tmp->request.t_idx, ptr_tmp->request.f_idx, ptr_tmp->request.e_idx, ptr_raw, ptr_cfg, &print_size);
                        if (MW_E_OK != ret)
                        {
                            DB_LOG_ERROR("%s: write the table's data (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                            break;
                        }
                        data_size += print_size;
                        flash_offst += print_size;
                        osapi_memset(ptr_raw, 0, DB_BUFFER_SIZE_256);
                        if((data_size + print_size) >= DB_BUFFER_SIZE_512)
                        {
                            ret = osapi_flashWrite(WRITE, data_size, ptr_cfg, part);
                            if (MW_E_OK == ret)
                            {
                                osapi_memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
                                data_size = 0;
                            }
                        }
                    }
                    if (MW_E_OK != ret)
                    {
                        break;
                    }
                }
                else
                {
                    /* Get DB raw data */
                    ret = db_getData(1, M_GET, ptr_tmp);
                    if (MW_E_OK != ret)
                    {
                        DB_LOG_ERROR("%s: get next data from DB (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                        break;
                    }
                    /* Encode JSON string */
                    print_size = DB_BUFFER_SIZE_512 - data_size;
                    ret = db_cfgfile_saveField(ptr_tmp->request.t_idx, ptr_tmp->request.f_idx, ptr_tmp->request.e_idx, ptr_raw, ptr_cfg, &print_size);
                    if (MW_E_OK != ret)
                    {
                        DB_LOG_ERROR("%s: write the table's data (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                        break;
                    }
                    data_size += print_size;
                    flash_offst += print_size;
                    osapi_memset(ptr_raw, 0, DB_BUFFER_SIZE_256);
                }
                ptr_tmp->request.f_idx ++;
                ptr_tmp->request.e_idx = 0;
                if (ptr_tmp->request.f_idx > field_num)
                {
                    /* Replace the last ',' with '}' */
                    ptr_cfg[data_size - 1] = '}';
                    break;

                }
                ret = osapi_flashWrite(WRITE, data_size, ptr_cfg, part);
                if (MW_E_OK == ret)
                {
                    memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
                    data_size = osapi_snprintf((C8_T *)ptr_cfg, DB_BUFFER_SIZE_512, "\n");
                    flash_offst += data_size;
                }
            }
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: write data to flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                break;
            }
            /* End of array or table */
            ptr_tmp->request.t_idx++;
            if (ptr_tmp->request.t_idx < DB_FIRST_OPER_TABLE)
            {
                DB_APPEND_CHAR((C8_T *)ptr_cfg, data_size, ",");
                flash_offst++;
            }
            ret = osapi_flashWrite(WRITE, data_size, ptr_cfg, part);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: write data to flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                break;
            }
            osapi_memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
            data_size = osapi_snprintf((C8_T *)ptr_cfg, DB_BUFFER_SIZE_512, "\n");
            flash_offst += data_size;

            /* End of cfg file */
            if (ptr_tmp->request.t_idx >= DB_FIRST_OPER_TABLE)
            {
                print_size = osapi_snprintf((C8_T *)(ptr_cfg + data_size), DB_BUFFER_SIZE_512, "%c%c", '}', DB_ETX);
                data_size += print_size;
                flash_offst += print_size;
                break;
            }
            ptr_ctrl++;
        }
        osapi_mutexGive(_db_core.ptr_data_mutex);

        if (MW_E_OK == ret)
        {
            /* write the last data to flash if any data */
            if (data_size > 0)
            {
                ret = osapi_flashWrite(WRITE, data_size, ptr_cfg, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: write data to flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                }
            }
            ret = osapi_flashWrite(END, 0, NULL, part);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("%s: End of data writing to flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
            }
        }
    }

    /* Free the temp buffer */
    MW_FREE(ptr_cfg);
    MW_FREE(ptr_tmp);
    return ret;
}

static MW_ERROR_NO_T
_db_clear_cfgFile(
    BOOL_T to_factory)
{
    UI8_T temp_data[8] = {0};
    UI8_T  part = DB_CONFIG_FACTORY_PART, t_idx = 0;
    DB_CTRL_T *ptr_ctrl = NULL;
    /* Point to the partition */
    if (TRUE == to_factory)
    {
        part = DB_CONFIG_FACTORY_PART;
    }
    else
    {
        part = DB_CONFIG_STARTUP_PART;
    }

    if (MW_E_OK == osapi_mutexTake(_db_core.ptr_data_mutex, DB_Q_WAITTIME))
    {
        /* Start */
        osapi_flashWrite(INIT, 0, NULL, part);
        osapi_flashWrite(WRITE, 8, temp_data, part);
        osapi_flashWrite(END, 0, NULL, part);
        for (t_idx = 0; t_idx < TABLES_LAST; t_idx++)
        {
            ptr_ctrl = _db_core.ptr_ctrl + t_idx;
            if(TRUE == to_factory)
            {
                ptr_ctrl->factory_offt = DB_NO_FLASH_CONFIG;
            }
            else
            {
                ptr_ctrl->startup_offt = DB_NO_FLASH_CONFIG;
            }
        }
        osapi_mutexGive(_db_core.ptr_data_mutex);
    }

    return MW_E_OK;
}

/* Parse config file to DB */
static MW_ERROR_NO_T
_db_data_parse_cfg(
    UI32_T offset,
    UI32_T file_size,
    UI8_T  part,
    UI8_T  flag,
    DB_REQUEST_TYPE_T request,
    void   *ptr_out_data)
{
    MW_ERROR_NO_T ret = MW_E_ENTRY_NOT_FOUND;
    UI32_T read_addr = offset;
    UI32_T read_end_addr = file_size;
    UI32_T read_size = DB_BUFFER_SIZE_512;
    UI32_T remain_size = 0;
    BOOL_T valid_cfg = FALSE;
    UI16_T data_size = 0;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_cfg = NULL;
    UI8_T *ptr_raw = NULL;
    UI8_T *ptr_cfgdata = NULL;
    DB_PAYLOAD_T *ptr_tmp = NULL;
    C8_T *ptr_etoken = NULL;
    C8_T *ptr_array_start = NULL;
    C8_T *ptr_data_start = NULL;
    C8_T *ptr_data_end = NULL;
    UI32_T cfg_offset = 0;
    C8_T eof_str[3] = {'}', DB_ETX, '\0'};

    MW_PARAM_CHK((0 == file_size), MW_E_BAD_PARAMETER);
    read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
    if (read_size == 0)
    {
        return MW_E_ENTRY_NOT_FOUND;
    }

    /* Allocate a temp memory to store the config text and raw data */
    ret = db_calloc(DB_BUFFER_SIZE_512 + 1, (void **)&ptr_cfg);
    if ((MW_E_OK != ret) || (NULL == ptr_cfg))
    {
        return ret;
    }

    ret = db_calloc((DB_MSG_PAYLOAD_SIZE + DB_BUFFER_SIZE_256), (void **)&ptr_tmp);
    if ((MW_E_OK != ret) || (NULL == ptr_tmp))
    {
        osapi_free(ptr_cfg);
        return ret;
    }

    /* read from flash */
    if(0 == offset)
    {
        read_addr = 1 + DB_CONFIG_MAGIC_LEN;
    }
    ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
        osapi_free(ptr_cfg);
        osapi_free(ptr_tmp);
        return ret;
    }
    cfg_offset = read_addr;
    read_addr += read_size;
    while (read_addr < read_end_addr)
    {
        /* If found the "end of text" */
        ptr_etoken = strstr((C8_T *)ptr_cfg, eof_str);
        if ((ptr_etoken != NULL) && (ptr_etoken + osapi_strlen(eof_str) < (C8_T *) (ptr_cfg + DB_BUFFER_SIZE_512)))
        {
            /* Avoid parsing data beyond eof_str. */
            *(ptr_etoken + osapi_strlen(eof_str)) = '\0';
        }

        ptr_cfgdata = ptr_cfg;
        /* find the tabe name in config file */
        if (0 != (DB_PARSE_SPECIAL_DATA & flag))
        {
            ptr_tmp->request.t_idx = request.t_idx;
            if (NULL == ptr_out_data)
            {
                ret = MW_E_BAD_PARAMETER;
            }
            if ((DB_ALL_FIELDS != request.f_idx) || (DB_ALL_ENTRIES != request.e_idx))
            {
                ret = MW_E_BAD_PARAMETER;
                DB_LOG_ERROR("Not support specific field or entry, req: t_idx=%d, f_idx=%d, e_idx=%d", request.t_idx, request.f_idx, request.e_idx);
                break;
            }
        }
        else
        {
            ret = db_cfgfile_searchTable(&ptr_cfgdata, &(ptr_tmp->request.t_idx));
            if (MW_E_OK != ret)
            {
                if (NULL != ptr_etoken)
                {
                    break;
                }
                read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
                if (read_size == 0)
                {
                    break;
                }
                ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    break;
                }
                cfg_offset = read_addr;
                read_addr += read_size;
                continue;
            }
        }
        valid_cfg = TRUE;

        /* Found the table, go to the data */
        ptr_ctrl = _db_core.ptr_ctrl + ptr_tmp->request.t_idx;
        ptr_tmp->request.f_idx = 0;
        ptr_tmp->request.e_idx = 1;
        ptr_array_start = (C8_T *)ptr_cfgdata;

        /* Record the offset of the table */
        if (0 != (DB_PARSE_CFG_OFFT & flag))
        {
            if (DB_CONFIG_FACTORY_PART == part)
            {
                ptr_ctrl->factory_offt = (cfg_offset + (ptr_cfgdata - ptr_cfg) - 1);
            }
            else
            {
                ptr_ctrl->startup_offt = (cfg_offset + (ptr_cfgdata - ptr_cfg) - 1);
            }
        }
        if (ptr_ctrl->entry_num > 1)
        {
            /* it is an array, find the start '[' */
            ptr_array_start = strchr(ptr_array_start, '[');
            if (NULL == ptr_array_start)
            {
                /* Not found availabe data */
                valid_cfg = FALSE;
                break;
            }
        }
        while (ptr_tmp->request.e_idx <= ptr_ctrl->entry_num)
        {
            ptr_data_start = NULL;
            ptr_data_end = NULL;
            /* find the start '{' */
            ptr_data_start = strchr(ptr_array_start, '{');
            if (NULL == ptr_data_start)
            {
                /* read from flash */
                memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
                read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
                if (read_size == 0)
                {
                    valid_cfg = FALSE;
                    break;
                }
                ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    osapi_free(ptr_cfg);
                    osapi_free(ptr_tmp);
                    return ret;
                }
                cfg_offset = read_addr;
                read_addr += read_size;
                ptr_array_start = (C8_T *)ptr_cfg;
                /* Not found availabe entry */
                continue;
            }
            /* find the end '}' */
            ptr_data_end = strchr(ptr_data_start, '}');
            if (NULL == ptr_data_end)
            {
                /* Not found availabe entry */
                remain_size = (ptr_cfg + DB_BUFFER_SIZE_512 - (UI8_T *)ptr_data_start);
                memcpy(ptr_cfg, ptr_data_start, remain_size);
                ptr_array_start = (C8_T *)(ptr_cfg + remain_size);
                read_size = (UI8_T*)ptr_data_start - ptr_cfg;
                memset(ptr_array_start, 0, read_size);
                read_size = ((read_end_addr - read_addr) > read_size)? read_size : (read_end_addr - read_addr);
                if (read_size == 0)
                {
                    /* Not found availabe data */
                    valid_cfg = FALSE;
                    break;
                }
                ret = osapi_flashRead(read_addr, read_size, (UI8_T *)ptr_array_start, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    osapi_free(ptr_cfg);
                    osapi_free(ptr_tmp);
                    return ret;
                }
                cfg_offset = read_addr - (DB_BUFFER_SIZE_512 - read_size);
                read_addr += read_size;
                ptr_array_start = (C8_T *)ptr_cfg;
                continue;
            }
            data_size = ptr_data_end - ptr_data_start + 1;
            if (data_size < DB_MIN_ENTRY_LEN)
            {
                valid_cfg = FALSE;
                break;
            }

            /* Parse the config file */
            ptr_tmp->request.f_idx = 1;
            ptr_raw = (UI8_T *)&(ptr_tmp->ptr_data);
            memset(ptr_raw, 0, DB_BUFFER_SIZE_256);
            while (ptr_tmp->request.f_idx <= ptr_ctrl->field_num)
            {
                if(DB_PARSE_CFG_OFFT == flag)
                {
                    /* Only parse the config file offset */
                    break;
                }
                ret = db_cfgfile_parsing(ptr_tmp->request,
                                   ptr_tmp,
                                   *(ptr_ctrl->fields_size + (ptr_tmp->request.f_idx - 1)),
                                   ptr_raw,
                                   ptr_data_start,
                                   data_size,
                                   flag);
                if (MW_E_BAD_PARAMETER == ret)
                {
                    valid_cfg = FALSE;
                    break;
                }
                if (0 != (DB_PARSE_SPECIAL_DATA & flag))
                {
                    if(MW_E_OK == ret)
                    {
                        ret = _db_setDataToRawTable((UI8_T *)ptr_out_data, (const UI8_T *)ptr_raw, ptr_tmp->request);
                        if ((MW_E_OK != ret) && (MW_E_ALREADY_INITED != ret))
                        {
                            valid_cfg = FALSE;
                            break;
                        }
                    }
                    else
                    {
                        valid_cfg = FALSE;
                        break;
                    }
                    if (ptr_tmp->request.f_idx == ptr_ctrl->field_num)
                    {
                        break;
                    }
                }
                /* Next field */
                ptr_tmp->request.f_idx++;
            }
            if (0 != (DB_PARSE_SPECIAL_DATA & flag))
            {
                if (ptr_tmp->request.e_idx == ptr_ctrl->entry_num)
                {
                    ret = MW_E_ALREADY_INITED;
                    break;
                }
            }
            if (FALSE == valid_cfg)
            {
                break;
            }
            valid_cfg = TRUE;
            ret = MW_E_OK;
            ptr_tmp->request.e_idx++;
            if (ptr_ctrl->entry_num > 1)
            {
                /* it is an array, find the end ']' */
                if (*(ptr_data_end + 1) == ']')
                {
                    break;
                }
            }
            ptr_array_start = ptr_data_end;
        }
        if (MW_E_OK != ret)
        {
            break;
        }
        if (NULL != ptr_data_end)
        {
            /* Go to Next table */
            remain_size = (ptr_cfg + DB_BUFFER_SIZE_512 - (UI8_T *)ptr_data_end);
            memcpy(ptr_cfg, ptr_data_end, remain_size);
            ptr_array_start = (C8_T *)(ptr_cfg + remain_size);
            read_size = (UI8_T*)ptr_data_end - ptr_cfg;
            memset(ptr_array_start, 0, read_size);
            read_size = ((read_end_addr - read_addr) > read_size)? read_size : (read_end_addr - read_addr);
            if (read_size != 0)
            {
                ret = osapi_flashRead(read_addr, read_size, (UI8_T *)ptr_array_start, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    MW_FREE(ptr_cfg);
                    MW_FREE(ptr_tmp);
                    return ret;
                }
                cfg_offset = read_addr - (DB_BUFFER_SIZE_512 - read_size);
                read_addr += read_size;
            }
            ptr_array_start = (C8_T *)ptr_cfg;
            ptr_etoken = NULL;
        }

        if ((FALSE == valid_cfg) || (NULL != ptr_etoken))
        {
            break;
        }
    }

    if (TRUE == valid_cfg)
    {
        ret = MW_E_OK;
    }
    else
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    MW_FREE(ptr_cfg);
    MW_FREE(ptr_tmp);
    return ret;
}

/* Parse config file to DB */
static MW_ERROR_NO_T
_db_data_parse_cfg_ext(
    UI32_T offset,
    UI32_T file_size,
    UI8_T  part,
    UI8_T  flag,
    DB_REQUEST_TYPE_T request,
    void   *ptr_out_data)
{
    C8_T            *ptr_etoken = NULL;
    C8_T            *ptr_array_start = NULL;
    C8_T            *ptr_cfg_str = NULL;
    C8_T            *ptr_data_start = NULL, *ptr_data_end = NULL;
    C8_T            eof_str[3] = {'}', DB_ETX, '\0'};
    C8_T            *ptr_value = NULL;
    C8_T            filed_name[DB_MAX_KEY_SIZE] = {0};
    UI8_T           *ptr_cfg = NULL;
    UI8_T           *ptr_raw = NULL;
    UI8_T           *ptr_cfgdata = NULL;
    UI16_T          cfg_str_size = 0;
    UI32_T          flash_addr = 0;
    UI32_T          cfg_offset = 0;
    UI32_T          read_addr = offset;
    UI32_T          read_end_addr = file_size;
    UI32_T          read_size = DB_BUFFER_SIZE_512;
    UI32_T          remain_size = 0;
    UI32_T          data_size = 0;
    UI32_T          value_len = 0;
    UI32_T          start = 0, next = 0;
    BOOL_T          valid_cfg = FALSE;
    DB_CTRL_T       *ptr_ctrl = _db_core.ptr_ctrl;
    JSONPair_t      pair = {0};
    JSONTypes_t     value_type = 0;
    DB_PAYLOAD_T    *ptr_tmp = NULL;
    JSONStatus_t    result = 0;
    MW_ERROR_NO_T   ret = MW_E_ENTRY_NOT_FOUND;

    MW_PARAM_CHK((0 == file_size), MW_E_BAD_PARAMETER);
    if (DB_CONFIG_FACTORY_PART == part)
    {
        flash_addr = SystemConfigBase;
    }
    else if (DB_CONFIG_STARTUP_PART == part)
    {
        flash_addr = SystemConfigBase + (SystemConfigSize >> 1);
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }

    read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
    if (read_size == 0)
    {
        return MW_E_ENTRY_NOT_FOUND;
    }

    /* Allocate a temp memory to store the config text and raw data */
    ret = db_calloc(DB_BUFFER_SIZE_512 + 1, (void **)&ptr_cfg);
    if ((MW_E_OK != ret) || (NULL == ptr_cfg))
    {
        return ret;
    }

    ret = db_calloc((DB_MSG_PAYLOAD_SIZE + DB_BUFFER_SIZE_256), (void **)&ptr_tmp);
    if ((MW_E_OK != ret) || (NULL == ptr_tmp))
    {
        MW_FREE(ptr_cfg);
        return ret;
    }

    /* read from flash */
    if(0 == offset)
    {
        read_addr = 1 + DB_CONFIG_MAGIC_VER_LEN;
    }
    ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
        MW_FREE(ptr_cfg);
        MW_FREE(ptr_tmp);
        return ret;
    }
    cfg_offset = read_addr;
    read_addr += read_size;
    while (read_addr < read_end_addr)
    {
        /* If found the "end of text" */
        ptr_etoken = strstr((C8_T *)ptr_cfg, eof_str);
        if ((NULL != ptr_etoken) && (ptr_etoken + osapi_strlen(eof_str) <  (C8_T *) (ptr_cfg + DB_BUFFER_SIZE_512)))
        {
            /* Avoid parsing data beyond eof_str. */
            *(ptr_etoken + osapi_strlen(eof_str)) = '\0';
        }

        ptr_cfgdata = ptr_cfg;
        /* find the tabe name in config file */
        if (0 != (DB_PARSE_SPECIAL_DATA & flag))
        {
            ptr_tmp->request.t_idx = request.t_idx;
            if (NULL == ptr_out_data)
            {
                ret = MW_E_BAD_PARAMETER;
            }
            if ((DB_ALL_FIELDS != request.f_idx) || (DB_ALL_ENTRIES != request.e_idx))
            {
                ret = MW_E_BAD_PARAMETER;
                DB_LOG_ERROR("Not support specific field or entry, req: t_idx=%d, f_idx=%d, e_idx=%d", request.t_idx, request.f_idx, request.e_idx);
                break;
            }
        }
        else
        {
            ret = db_cfgfile_searchTable(&ptr_cfgdata, &(ptr_tmp->request.t_idx));
            if (MW_E_OK != ret)
            {
                if (NULL != ptr_etoken)
                {
                    break;
                }
                read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
                if (read_size == 0)
                {
                    break;
                }
                ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    break;
                }
                cfg_offset = read_addr;
                read_addr += read_size;
                continue;
            }
        }
        DB_LOG_DEBUG("t_idx: %d, ", ptr_tmp->request.t_idx);
        valid_cfg = TRUE;
        /* Found the table, go to the data */
        ptr_ctrl = _db_core.ptr_ctrl + ptr_tmp->request.t_idx;
        ptr_tmp->request.f_idx = 1;
        ptr_tmp->request.e_idx = 0;
        ptr_array_start = (C8_T *)ptr_cfgdata;

        /* Record the offset of the table */
        if (0 != (DB_PARSE_CFG_OFFT & flag))
        {
            if (DB_CONFIG_FACTORY_PART == part)
            {
                ptr_ctrl->factory_offt = (cfg_offset + (ptr_cfgdata - ptr_cfg) - 1);
            }
            else
            {
                ptr_ctrl->startup_offt = (cfg_offset + (ptr_cfgdata - ptr_cfg) - 1);
            }
        }
        ptr_data_start = NULL;
        ptr_data_end = NULL;
        while (read_addr < read_end_addr)
        {
            if (NULL == ptr_data_start)
            {
                /* find the start '{' */
                ptr_data_start = strchr(ptr_array_start, '{');
                if (NULL == ptr_data_start)
                {
                    /* read from flash */
                    osapi_memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
                    read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
                    if (read_size == 0)
                    {
                        valid_cfg = FALSE;
                        break;
                    }
                    ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
                    if (MW_E_OK != ret)
                    {
                        DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                        MW_FREE(ptr_cfg);
                        MW_FREE(ptr_tmp);
                        return ret;
                    }
                    cfg_offset = read_addr;
                    read_addr += read_size;
                    ptr_array_start = (C8_T *)ptr_cfg;
                    continue;
                }
                else
                {
                    ptr_array_start = ptr_data_start;
                    ptr_data_start = (C8_T *)((C8_T *)ptr_data_start - (C8_T *)ptr_cfg) + cfg_offset + flash_addr;
                }
            }

            if (NULL == ptr_data_end)
            {
                /* find the end '}' */
                ptr_data_end = strchr(ptr_array_start, '}');
                if (NULL == ptr_data_end)
                {
                    /* Not found availabe entry */
                    osapi_memset(ptr_cfg, 0, DB_BUFFER_SIZE_512);
                    read_size = ((read_end_addr - read_addr) > DB_BUFFER_SIZE_512)? DB_BUFFER_SIZE_512 : (read_end_addr - read_addr);
                    if (read_size == 0)
                    {
                        /* Not found availabe data */
                        valid_cfg = FALSE;
                        break;
                    }
                    ret = osapi_flashRead(read_addr, read_size, ptr_cfg, part);
                    if (MW_E_OK != ret)
                    {
                        DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                        MW_FREE(ptr_cfg);
                        MW_FREE(ptr_tmp);
                        return ret;
                    }
                    cfg_offset = read_addr;
                    read_addr += read_size;
                    ptr_array_start = (C8_T *)ptr_cfg;
                    continue;
                }
                else
                {
                    ptr_array_start = ptr_data_end;
                    ptr_data_end = (C8_T *)((C8_T *)ptr_data_end - (C8_T *)ptr_cfg) + cfg_offset + flash_addr;
                }
            }

            data_size = ptr_data_end - ptr_data_start + 1;
            if (data_size < DB_MIN_ENTRY_LEN)
            {
                valid_cfg = FALSE;
            }
            break;
        }
        DB_LOG_DEBUG("ptr_data_start:0x%x, ptr_data_end:0x%x, data size:%d", ptr_data_start, ptr_data_end, data_size);
        while (ptr_tmp->request.f_idx <= ptr_ctrl->field_num)
        {
            if(DB_PARSE_CFG_OFFT == flag)
            {
                /* Only parse the config file offset */
                break;
            }
            if ((NULL == ptr_data_start) || (NULL == ptr_data_end) || (data_size < DB_MIN_ENTRY_LEN))
            {
                break;
            }
            DB_LOG_DEBUG("\tf_idx: %d, ", ptr_tmp->request.f_idx);
            /* Parse the config file */
            ptr_tmp->request.e_idx = 1;
            ptr_raw = (UI8_T *)&(ptr_tmp->ptr_data);
            osapi_memset(ptr_raw, 0, DB_BUFFER_SIZE_256);
            osapi_memset(filed_name, 0, DB_MAX_KEY_SIZE);
            db_cfgfile_getFieldName(ptr_tmp->request.t_idx, ptr_tmp->request.f_idx, sizeof(filed_name), filed_name);
            result = JSON_SearchT(ptr_data_start,
                                    data_size,
                                    (const char *)filed_name,
                                    (size_t)osapi_strlen(filed_name),
                                    (char **)&ptr_value,
                                    (size_t *)&value_len,
                                    &value_type);
            if (JSONSuccess != result)
            {
                DB_LOG_WARN("filed:\"%s\" not found(%d)", filed_name, result);
                valid_cfg = FALSE;
                break;
            }
            ptr_cfg_str = ptr_value;
            cfg_str_size = value_len;
            DB_LOG_DEBUG("\"%s\", type:%d, ptr_value:0x%x, len:%d, ", filed_name, value_type, ptr_value, value_len);
            if(1 < ptr_ctrl->entry_num)
            {
                /* Check Json array */
                result = JSON_Validate(ptr_value, value_len);
                if (JSONSuccess != result)
                {
                    DB_LOG_WARN("Check t:%d f:%d Json array failed(%d)", ptr_tmp->request.t_idx, ptr_tmp->request.f_idx, result);
                    valid_cfg = FALSE;
                    break;
                }
                start = 0;
                next = 0;
            }
            while (ptr_tmp->request.e_idx <= ptr_ctrl->entry_num)
            {
                if(1 < ptr_ctrl->entry_num)
                {
                    result = JSON_Iterate(ptr_value, value_len, (size_t *) &start, (size_t *) &next, &pair);
                    if (JSONSuccess != result)
                    {
                        DB_LOG_WARN("Get t:%d f:%d entry failed(%d)", ptr_tmp->request.t_idx, ptr_tmp->request.f_idx, result);
                        valid_cfg = FALSE;
                        break;
                    }
                    DB_LOG_DEBUG( "\t\te_idx:%d, start:%d, next:%d, len:%d, ", ptr_tmp->request.e_idx, start, next, (UI32_T) pair.valueLength);
                    ptr_cfg_str = (C8_T *)pair.value;
                    cfg_str_size = (UI32_T) pair.valueLength;
                }
                ret = db_cfgfile_parsing_ext(ptr_tmp->request,
                                   ptr_tmp,
                                   *(ptr_ctrl->fields_size + (ptr_tmp->request.f_idx - 1)),
                                   ptr_raw,
                                   ptr_cfg_str,
                                   cfg_str_size,
                                   flag);
                if (MW_E_BAD_PARAMETER == ret)
                {
                    valid_cfg = FALSE;
                    break;
                }
                if (0 != (DB_PARSE_SPECIAL_DATA & flag))
                {
                    if(MW_E_OK == ret)
                    {
                        osapi_memcpy(ptr_out_data, ptr_raw, *(ptr_ctrl->fields_size + (ptr_tmp->request.f_idx - 1)));
                        ptr_out_data += *(ptr_ctrl->fields_size + (ptr_tmp->request.f_idx - 1));
                    }
                    else
                    {
                        valid_cfg = FALSE;
                        break;
                    }
                    if (ptr_tmp->request.e_idx == ptr_ctrl->entry_num)
                    {
                        break;
                    }
                }
                /* Next entry */
                osapi_memset(ptr_raw, 0, ptr_tmp->data_size);
                ptr_tmp->request.e_idx++;
            }
            if (0 != (DB_PARSE_SPECIAL_DATA & flag))
            {
                if (ptr_tmp->request.f_idx == ptr_ctrl->field_num)
                {
                    ret = MW_E_ALREADY_INITED;
                    break;
                }
            }
            if (FALSE == valid_cfg)
            {
                break;
            }
            valid_cfg = TRUE;
            ret = MW_E_OK;
            ptr_tmp->request.f_idx++;
        }
        if (MW_E_OK != ret)
        {
            break;
        }
        if (NULL != ptr_data_end)
        {
            /* Go to Next table */
            remain_size = (ptr_cfg + DB_BUFFER_SIZE_512 - (UI8_T *)ptr_array_start);
            osapi_memcpy(ptr_cfg, ptr_array_start, remain_size);
            ptr_array_start = (C8_T *)(ptr_cfg + remain_size);
            read_size = DB_BUFFER_SIZE_512 - remain_size;
            osapi_memset(ptr_array_start, 0, read_size);
            read_size = ((read_end_addr - read_addr) > read_size)? read_size : (read_end_addr - read_addr);
            if (0 != read_size)
            {
                ret = osapi_flashRead(read_addr, read_size, (UI8_T *)ptr_array_start, part);
                if (MW_E_OK != ret)
                {
                    DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
                    MW_FREE(ptr_cfg);
                    MW_FREE(ptr_tmp);
                    return ret;
                }
                cfg_offset = read_addr - (DB_BUFFER_SIZE_512 - read_size);
                read_addr += read_size;
            }
            ptr_array_start = (C8_T *)ptr_cfg;
            ptr_etoken = NULL;
        }

        if ((FALSE == valid_cfg) || (NULL != ptr_etoken))
        {
            break;
        }
    }

    if (TRUE == valid_cfg)
    {
        ret = MW_E_OK;
    }
    else
    {
        ret = MW_E_ENTRY_NOT_FOUND;
    }
    MW_FREE(ptr_cfg);
    MW_FREE(ptr_tmp);
    return ret;
}

/* Parse and verify the request payload and output to type structure
*/
static MW_ERROR_NO_T
_db_parse_payload(
    DB_PAYLOAD_T *ptr_payload,
    DB_REQUEST_TYPE_T *ptr_out_req)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    DB_REQUEST_TYPE_T req;

    if ((NULL == ptr_payload) || (NULL == ptr_out_req))
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    /* verify the requested table and field */
    memcpy((void *)&req, (const void *)&(ptr_payload->request), sizeof(DB_REQUEST_TYPE_T));
    MW_PARAM_CHK((TABLES_LAST <= req.t_idx), MW_E_BAD_PARAMETER);
    ptr_ctrl += req.t_idx;
    MW_PARAM_CHK((ptr_ctrl->field_num < req.f_idx), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((ptr_ctrl->entry_num < req.e_idx), MW_E_BAD_PARAMETER);
    memcpy((void *)ptr_out_req, (const void *)&(ptr_payload->request), sizeof(DB_REQUEST_TYPE_T));
    return MW_E_OK;
}

static MW_ERROR_NO_T
_db_parse_tlv_data(
    UI8_T type)
{
    UI32_T          tlv_type_addr = 0;
    UI8_T           tlv_value_len = 0;
    UI8_T           f_idx = 0;
    void            *ptr_tlv_data = NULL;
    MW_IPV4_T       addr = 0;
    MW_ERROR_NO_T   ret = MW_E_OK;

    ret = mw_is_tlv_data_exist(type, &tlv_type_addr);
    if(MW_E_OK != ret)
    {
        DB_LOG_WARN("Type (%d) is not exist, ret:%d", type, ret);
        return ret;
    }
    mw_read_tlv_data(sizeof(UI8_T), (tlv_type_addr + TLV_DATA_TYPE_SIZE), (void *)&tlv_value_len);
    if(0 != tlv_value_len)
    {
        ret = db_calloc(tlv_value_len, (void **)&ptr_tlv_data);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("Cannot allocate memory size (%d)", tlv_value_len);
            return ret;
        }
        switch (type)
        {
            case MW_TLV_TYPE_HW_VERSION:
                if(MAX_VERSION_SIZE >= tlv_value_len)
                {
                    if(MW_E_OK == mw_read_tlv_data(tlv_value_len, (tlv_type_addr + TLV_DATA_HEADER_SIZE), ptr_tlv_data))
                    {
                        ret = _db_data_update_data(SYS_OPER_INFO, SYS_OPER_HW_VER, DB_ALL_ENTRIES, (const void *)ptr_tlv_data);
                        DB_LOG_DEBUG("hw ver is %s", ptr_tlv_data);
                    }
                }
                break;

            case MW_TLV_TYPE_IP_ADDRESS:
            case MW_TLV_TYPE_SUBNET_MASK:
            case MW_TLV_TYPE_GATEWAY:
                if(MW_E_OK == mw_read_tlv_data(tlv_value_len, (tlv_type_addr + TLV_DATA_HEADER_SIZE), ptr_tlv_data))
                {
                    if(MW_TLV_TYPE_IP_ADDRESS == type)
                    {
                        f_idx = SYS_STATIC_IP_ADDR;
                    }
                    else if(MW_TLV_TYPE_SUBNET_MASK == type)
                    {
                        f_idx = SYS_STATIC_IP_MASK;
                    }
                    else
                    {
                        f_idx = SYS_STATIC_IP_GW;
                    }
                    addr = getIpaddr(ptr_tlv_data);
                    DB_LOG_DEBUG("type:%d addr_str:%s, addr_u32 is 0x%x", type, ptr_tlv_data, addr);
                    ret = _db_data_update_data(SYS_INFO, f_idx, DB_ALL_ENTRIES, (const void *)&addr);
                }
                break;

            default:
                break;
        }
        MW_FREE(ptr_tlv_data);
    }

    return ret;
}

static MW_ERROR_NO_T
_db_system_ctrl(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T *ptr_data)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_SYSTEM_T sys_ctrl;

    if (NULL == ptr_data)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }
    if (SYSTEM == t_idx)
    {
        /* Parse the data */
        memset((void *)&sys_ctrl, 0, sizeof(DB_SYSTEM_T));
        if (DB_ALL_FIELDS == f_idx)
        {
            memcpy((void *)&sys_ctrl, (const void *)ptr_data, sizeof(DB_SYSTEM_T));
        }
        else
        {
            switch (f_idx)
            {
                case SYS_SAVE_RUNNING:
                    sys_ctrl.save_running = *ptr_data;
                    break;
                case SYS_RESET:
                    sys_ctrl.reset = *ptr_data;
                    break;
                case SYS_RESET_FACTORY:
                    sys_ctrl.reset_factory = *ptr_data;
                    break;
                default:
                    return MW_E_BAD_PARAMETER;
            }
        }
        if (TRUE == sys_ctrl.save_running)
        {
            /* Save running to startup */
            ret = _db_data_save_running(FALSE);
            if (MW_E_OK != ret)
            {
                return ret;
            }
        }
        if (TRUE == sys_ctrl.reset_factory)
        {
            ret = _db_set_reset_factory_flag();
        }
        /* system reset would not execute in db task */
    }
    return ret;
}

static MW_ERROR_NO_T
_db_data_update_data(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data)
{
    DB_CTRL_T                   *ptr_ctrl = _db_core.ptr_ctrl;
    MW_ERROR_NO_T               ret = MW_E_OK;
    const DB_REQUEST_TYPE_T     req = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx,
    };

    MW_CHECK_PTR(ptr_data);
    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);
    ptr_ctrl += req.t_idx;
    ret = _db_setDataToRawTable((UI8_T *)ptr_ctrl->table_handle, (const UI8_T *)ptr_data, req);
    return ret;
}

static MW_ERROR_NO_T
_db_setDataToRawTable(
    UI8_T *ptr_raw_data,
    const UI8_T *ptr_input_data,
    const DB_REQUEST_TYPE_T req)
{
    UI8_T           field = 0, field_size = 0, field_num;
    UI16_T          req_data_size = 0, entry_num = 0;
    MW_ERROR_NO_T   rc = MW_E_ALREADY_INITED;

    if (MW_E_OK != db_getDataSize(req, &req_data_size))
    {
        return MW_E_NOT_SUPPORT;
    }
    MW_PARAM_CHK((TABLES_LAST <= req.t_idx), MW_E_BAD_PARAMETER);
    field_num = db_getTableFieldsNum(req.t_idx);
    MW_PARAM_CHK((field_num < req.f_idx), MW_E_BAD_PARAMETER);
    entry_num = db_getTableEntriesNum(req.t_idx);
    MW_PARAM_CHK((entry_num < req.e_idx), MW_E_BAD_PARAMETER);
    if (DB_ALL_FIELDS == req.f_idx)
    {
        /* all table */
        if (DB_ALL_ENTRIES == req.e_idx)
        {
            if (0 != osapi_memcmp((const void *)ptr_raw_data, (const void *)ptr_input_data, req_data_size))
            {
                osapi_memcpy(ptr_raw_data, (const void *)ptr_input_data, req_data_size);
                rc = MW_E_OK;
            }
        }
        else
        {
            /* all fields required with specific entry */
            for (field = 0; field < field_num; field++)
            {
                field_size = db_getTableFieldSize(req.t_idx, field + 1);
                if (0 != osapi_memcmp((const void *)(ptr_raw_data + ((req.e_idx - 1) * field_size)),
                                (const void *)ptr_input_data, field_size))
                {
                    osapi_memcpy((void *)(ptr_raw_data + ((req.e_idx - 1) * field_size)), (const void *)ptr_input_data, field_size);
                    rc = MW_E_OK;
                }
                ptr_input_data += field_size;
                ptr_raw_data += (entry_num * field_size);
            }
        }
    }
    else
    {
        /* Switch the correct field */
        for (field = 1; field < req.f_idx; field++)
        {
            field_size = db_getTableFieldSize(req.t_idx, field);
            ptr_raw_data += (entry_num * field_size);
        }
        /* Switch to specific entry if needed */
        if (DB_ALL_ENTRIES != req.e_idx)
        {
            field_size = db_getTableFieldSize(req.t_idx, req.f_idx);
            ptr_raw_data += ((req.e_idx - 1) * field_size);
        }
        if (0 != osapi_memcmp((const void *)ptr_raw_data, (const void *)ptr_input_data, req_data_size))
        {
            osapi_memcpy(ptr_raw_data, (const void *)ptr_input_data, req_data_size);
            rc = MW_E_OK;
        }
    }
    return rc;
}

/* FUNCTION NAME: _db_getFlashCfgValue
 * PURPOSE:
 *      Get the raw data of the specific request from the flash configuration.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *      part              -- The part of the flash configuration, factory or startup
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *      pptr_payload_data -- A double pointer returns the next request to be
 *                           parsed if there is any
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
static MW_ERROR_NO_T
_db_getFlashCfgValue(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const UI8_T part,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    void            *ptr_out_data = NULL;
    UI8_T           config_ver = 0;
    UI8_T           *ptr_raw_buf = NULL, *ptr_raw_data = NULL;
    UI8_T           field_num = 0;
    UI16_T          flash_offt = 0;
    UI16_T          entry_num = 0, req_data_size, table_size = 0;
    DB_REQUEST_TYPE_T req = {
                    .t_idx = t_idx,
                    .f_idx = 0,
                    .e_idx = 0};
    MW_ERROR_NO_T   rc = MW_E_NOT_SUPPORT;
    DB_CTRL_T       *ptr_ctrl = _db_core.ptr_ctrl;

    if (MW_E_OK != _db_is_cfg_exist(0, part, &config_ver))
    {
        /* config not exist */
        return MW_E_ENTRY_NOT_FOUND;
    }
    MW_CHECK_PTR(ptr_data_size);
    MW_CHECK_PTR(pptr_data);
    *ptr_data_size = 0;
    MW_PARAM_CHK((DB_FIRST_OPER_TABLE <= t_idx), MW_E_BAD_PARAMETER);
    ptr_ctrl = _db_core.ptr_ctrl + req.t_idx;
    if (DB_CONFIG_FACTORY_PART == part)
    {
        flash_offt = ptr_ctrl->factory_offt;
    }
    else
    {
        flash_offt = ptr_ctrl->startup_offt;
    }
    if (DB_NO_FLASH_CONFIG == flash_offt)
    {
        /* config not exist */
        DB_LOG_ERROR("table %d does not have %s part flash config", req.t_idx, ((DB_CONFIG_FACTORY_PART == part) ? "factory" : "startup"));
        return MW_E_NOT_SUPPORT;
    }
    field_num = db_getTableFieldsNum(t_idx);
    MW_PARAM_CHK((field_num < f_idx), MW_E_BAD_PARAMETER);
    entry_num = db_getTableEntriesNum(t_idx);
    MW_PARAM_CHK((entry_num < e_idx), MW_E_BAD_PARAMETER);

    /* Get the all table size */
    if (MW_E_OK != db_getDataSize(req, &table_size))
    {
        return MW_E_NOT_SUPPORT;
    }
    rc = db_calloc(table_size, (void **)&ptr_raw_buf);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    /* Read the config from flash */
    if(DB_CONFIG_VER_1 == config_ver)
    {
        rc = _db_data_parse_cfg_ext(flash_offt, DB_FLASH_CONFIGFILE_SIZE, part, DB_PARSE_SPECIAL_DATA, req, ptr_raw_buf);
    }
    else
    {
        rc = _db_data_parse_cfg(flash_offt, DB_FLASH_CONFIGFILE_SIZE, part, DB_PARSE_SPECIAL_DATA, req, ptr_raw_buf);
    }

    if (MW_E_OK != rc)
    {
        DB_LOG_WARN("Failed to parse flash config: t_idx = %d, f_idx = %d, e_idx= %d, part = %d, rc = %d", t_idx, f_idx, e_idx, part, rc);
        MW_FREE(ptr_raw_buf);
        return rc;
    }

    if(MW_E_OK == rc)
    {
        ptr_raw_data = ptr_raw_buf;
        /* Get request data size */
        req.f_idx = f_idx;
        req.e_idx = e_idx;
        if (MW_E_OK != db_getDataSize(req, &req_data_size))
        {
            MW_FREE(ptr_raw_buf);
            return MW_E_NOT_SUPPORT;
        }
        rc = db_calloc(req_data_size, (void **)&ptr_out_data);
        if (MW_E_OK != rc)
        {
            MW_FREE(ptr_raw_buf);
            return MW_E_NOT_SUPPORT;
        }

        db_getDataFromRawTable((UI8_T *)ptr_out_data, ptr_raw_data, req);
        *ptr_data_size = req_data_size;
        *pptr_data = ptr_out_data;
    }

    MW_FREE(ptr_raw_buf);
    return rc;
}

/* FUNCTION NAME: _db_getDefaultCfgValue
 * PURPOSE:
 *      Get default value from default.c for specific request.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *      pptr_payload_data -- A double pointer returns the next request to be
 *                           parsed if there is any
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
static MW_ERROR_NO_T
_db_getDefaultCfgValue(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    void            *ptr_out_data = NULL;
    UI8_T           *ptr_raw_buf = NULL, *ptr_raw_data = NULL;
    UI8_T           field_num = 0, field = 0, field_size = 0;
    UI16_T          entry_num = 0, req_data_size, table_size = 0;
    UI32_T          raw_offt = 0, field_offt = 0;
    DB_REQUEST_TYPE_T req = {
                    .t_idx = t_idx,
                    .f_idx = 0,
                    .e_idx = 0};
    MW_ERROR_NO_T   rc = MW_E_NOT_SUPPORT;
    DB_DEFAULT_CONFIG_TABLE_T *ptr_config = NULL;

    MW_CHECK_PTR(ptr_data_size);
    MW_CHECK_PTR(pptr_data);
    *ptr_data_size = 0;
    MW_PARAM_CHK((DB_FIRST_OPER_TABLE <= t_idx), MW_E_BAD_PARAMETER);
    field_num = db_getTableFieldsNum(t_idx);
    MW_PARAM_CHK((field_num < f_idx), MW_E_BAD_PARAMETER);
    entry_num = db_getTableEntriesNum(t_idx);
    MW_PARAM_CHK((entry_num < e_idx), MW_E_BAD_PARAMETER);

    /* Get the all table size */
    if (MW_E_OK != db_getDataSize(req, &table_size))
    {
        return MW_E_NOT_SUPPORT;
    }
    ptr_config = db_getDefaultCfgTable(t_idx);
    if ((NULL == ptr_config) || (NULL == ptr_config->ptr_table))
    {
        return MW_E_NOT_SUPPORT;
    }
    rc = db_calloc(table_size, (void **)&ptr_raw_buf);
    if (MW_E_OK != rc)
    {
        return MW_E_NOT_SUPPORT;
    }

    /* Load the default configuration value to the raw buffer */
    if (ptr_config->size == table_size)
    {
        osapi_memcpy(ptr_raw_buf, ptr_config->ptr_table, table_size);
    }
    else
    {
        for (field = 0; field < field_num; field++)
        {
            field_size = db_getTableFieldSize(t_idx, field + 1);
            raw_offt += _db_copy_raw((ptr_raw_buf + raw_offt), (void *)((UI8_T *)ptr_config->ptr_table + field_offt), field_size, entry_num);
            field_offt += field_size;
        }
    }

    /* Raw data correction for specific table */
    rc = _db_amendDefaultCfgValue(t_idx, ptr_raw_buf);
    if(MW_E_OK == rc)
    {
        ptr_raw_data = ptr_raw_buf;
        /* Get request data size */
        req.t_idx = t_idx;
        req.f_idx = f_idx;
        req.e_idx = e_idx;
        if (MW_E_OK != db_getDataSize(req, &req_data_size))
        {
            MW_FREE(ptr_raw_buf);
            return MW_E_NOT_SUPPORT;
        }
        rc = db_calloc(req_data_size, (void **)&ptr_out_data);
        if (MW_E_OK != rc)
        {
            MW_FREE(ptr_raw_buf);
            return MW_E_NOT_SUPPORT;
        }

        db_getDataFromRawTable((UI8_T *)ptr_out_data, ptr_raw_data, req);
        *ptr_data_size = req_data_size;
        *pptr_data = ptr_out_data;
    }

    MW_FREE(ptr_raw_buf);
    return rc;
}

static MW_ERROR_NO_T
_db_amendDefaultCfgValue(
    const UI8_T t_idx,
    UI8_T *ptr_raw_data)
{
    UI8_T           field = 0, field_size = 0;
    UI16_T          entry_num = 0, idx = 0;
    UI32_T          raw_offt = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    MW_CHECK_PTR(ptr_raw_data);

    entry_num = db_getTableEntriesNum(t_idx);
    /* Raw data correction for specific table */
    switch (t_idx)
    {
        case PORT_CFG_INFO:
        {
            UI16_T  port_setting = 0;
            UI32_T  port_isolation = 0;

            for (field = 1; field < PORT_SETTINGS; field++)
            {
                field_size = db_getTableFieldSize(t_idx, field);
                raw_offt += (entry_num * field_size);
            }
            for (idx = 0; idx < PLAT_MAX_PORT_NUM; idx++)
            {
                osapi_memcpy(&port_setting, ptr_raw_data + raw_offt, sizeof(port_setting));
                (void)port_correctPortSpeedDbCfg(0, idx+1, &port_setting);
                osapi_memcpy(ptr_raw_data + raw_offt, &port_setting, sizeof(port_setting));
                raw_offt += sizeof(port_setting);
            }
            osapi_memcpy(&port_isolation, ptr_raw_data + raw_offt, sizeof(port_isolation));
            port_isolation &= _mw_port_bmp_total[0];
            raw_offt += _db_copy_raw((ptr_raw_data + raw_offt), (void *)&port_isolation, sizeof(port_isolation), entry_num);
            break;
        }

        case VLAN_ENTRY:
        {
            DB_VLAN_ENTRY_T *ptr_vlan = NULL;

            ptr_vlan = (DB_VLAN_ENTRY_T *) ptr_raw_data;
            for(idx = 0; idx < MAX_VLAN_ENTRY_NUM; idx++)
            {
                    ptr_vlan->port_member[idx] &= _mw_port_bmp_total[0];
                    ptr_vlan->tagged_member[idx] &= _mw_port_bmp_total[0];
                    ptr_vlan->untagged_member[idx] &= _mw_port_bmp_total[0];
            }
            break;

        }

#ifdef AIR_SUPPORT_VOICE_VLAN
        case VOICE_VLAN_INFO:
        {
            DB_VOICE_VLAN_INFO_T *ptr_voice = NULL;

            ptr_voice = (DB_VOICE_VLAN_INFO_T *) ptr_raw_data;
            ptr_voice->port_mode &= (_mw_port_bmp_total[0] >> 1);
            break;
        }
#endif /* AIR_SUPPORT_VOICE_VLAN */

#ifdef AIR_SUPPORT_MSTP
        case MSTP_REGION:
        {
            osapi_sprintf((C8_T *)ptr_raw_data + offsetof(DB_MSTP_REGION_CONFIG_T, region_name),
                            "%02x:%02x:%02x:%02x:%02x:%02x", _mw_mac_addr[0], _mw_mac_addr[1], _mw_mac_addr[2],
                            _mw_mac_addr[3], _mw_mac_addr[4], _mw_mac_addr[5]);
            break;
        }

        case MSTP_INSTANCE:
        {
            UI32_T    vlanBmp = 1;

            raw_offt += offsetof(DB_MSTP_INSTANCE_T, vlanbmp) * entry_num;
            _db_copy_raw((ptr_raw_data + raw_offt), &vlanBmp, sizeof(UI32_T), 1);
            break;
        }

        case MSTP_INSTANCE_PORT:
        {
            UI32_T          i, j;
            DB_DEFAULT_CONFIG_TABLE_T *ptr_config = NULL;
            DB_MSTP_SINGLE_INSTANCE_PORT_INFO_T *ptr_config_table = NULL;

            ptr_config = db_getDefaultCfgTable(t_idx);
            if ((NULL == ptr_config) || (NULL == ptr_config->ptr_table))
            {
                rc = MW_E_NOT_SUPPORT;
                break;
            }
            ptr_config_table = (DB_MSTP_SINGLE_INSTANCE_PORT_INFO_T *)ptr_config->ptr_table;
            /* Port Instance priority */
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                for (j = 0; j < MAX_MSTP_INSTANCE_NUM; j++)
                {
                    osapi_memcpy(ptr_raw_data + raw_offt, &(ptr_config_table->priority), sizeof(UI8_T));
                    raw_offt += sizeof(UI8_T);
                }
            }

            /* Port Instance cost */
            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                for (j = 0; j < MAX_MSTP_INSTANCE_NUM; j++)
                {
                    osapi_memcpy(ptr_raw_data + raw_offt, &(ptr_config_table->cost), sizeof(UI32_T));
                    raw_offt += sizeof(UI32_T);
                }
            }

            break;
        }
#endif /* AIR_SUPPORT_MSTP */

        default:
            break;
    }

    return rc;
}

/* FUNCTION NAME: _db_copy_raw
 * PURPOSE:
 *      Copy raw data from one buffer to another buffer
 *
 * INPUT:
 *      ptr_value        -- A pointer to the source buffer
 *      size             -- size of the entry to copy
 *      entry_num        -- number of entries
 *
 * OUTPUT:
 *      ptr_addr         -- A pointer to the destination buffer
 *
 * RETURN:
 *      The bytes copied
 *
 * NOTES:
 *
 */
UI32_T
_db_copy_raw(
    UI8_T *ptr_addr,
    const void  *ptr_value,
    UI8_T size,
    UI8_T entry_num)
{
    UI16_T idx = 0;
    UI32_T offt = 0;
    UI8_T i = 0;

    for(idx = 0; idx < entry_num; idx++)
    {
        for(i = 0; i < size; i++)
        {
            *(ptr_addr + i) = *((UI8_T *)ptr_value + i);
        }
        ptr_addr += size;
        offt += size;
    }
    return offt;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: db_getDataSize
 * PURPOSE:
 *      Get the buffer size of the request type
 *
 * INPUT:
 *      req              -- A request type data
 *
 * OUTPUT:
 *      ptr_total_size   -- The required data size of the request type
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_getDataSize(
    DB_REQUEST_TYPE_T req,
    UI16_T *ptr_total_size)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T field = 0;
    UI16_T field_size = 0;
    UI8_T *ptr_field = NULL;


    if (NULL == ptr_total_size)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return MW_E_OTHERS;
    }

    /* verify the requested table and field */
    MW_PARAM_CHK((TABLES_LAST <= req.t_idx), MW_E_BAD_PARAMETER);
    ptr_ctrl += req.t_idx;
    MW_PARAM_CHK((ptr_ctrl->field_num < req.f_idx), MW_E_BAD_PARAMETER);
    MW_PARAM_CHK((ptr_ctrl->entry_num < req.e_idx), MW_E_BAD_PARAMETER);

    if (DB_ALL_FIELDS != req.f_idx)
    {
        /* specific field number */
        ptr_field = ptr_ctrl->fields_size;
        ptr_field += (req.f_idx - 1);
        field_size = *ptr_field;
    }
    else
    {
        /* all fields required */
        ptr_field = ptr_ctrl->fields_size;
        for (field = 0; field < ptr_ctrl->field_num; field++)
        {
            field_size += *ptr_field;
            ptr_field++;
        }
    }

    *ptr_total_size = field_size;
    if (DB_ALL_ENTRIES == req.e_idx)
    {
        /* all entries */
        *ptr_total_size = ((ptr_ctrl->entry_num) * field_size);
    }

    return MW_E_OK;
}

/* FUNCTION NAME: db_getData
 * PURPOSE:
 *      Get the data from database
 *
 * INPUT:
 *      count           --  The count of request payloads
 *      method          --  Method of get type
 *      ptr_payload     --  A pointer to the request payload
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The failed index of requests if encountered any problem, otherwise return 0
 *
 * NOTES:
 *
 */
UI8_T
db_getData(
    UI8_T count,
    UI8_T method,
    DB_PAYLOAD_T *ptr_payload)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_raw_data = NULL;
    void *ptr_out_data = NULL;
    DB_PAYLOAD_T *ptr_p = NULL;
    DB_REQUEST_TYPE_T req;
    UI16_T total_size = 0;
    UI8_T pidx = 1;

    if (NULL == ptr_payload)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return pidx;
    }

    ptr_out_data = (void *)ptr_payload;
    for (pidx = 1; pidx <= count; pidx++)
    {
        ptr_p = (DB_PAYLOAD_T *)ptr_out_data;
        /* parse and verify the request type */
        ret = _db_parse_payload(ptr_p, &req);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s", "failed to parse request");
            return pidx;
        }

        /* Check the data size */
        ret = db_getDataSize(req, &total_size);
        if (MW_E_OK != ret)
        {
            return pidx;
        }
        if (0 != memcmp(&(ptr_p->data_size), &total_size, sizeof(ptr_p->data_size)))
        {
            /* ptr_payload->data_size != total_size */
            DB_LOG_ERROR("%s: the request buffer size is not correct, ptr_p->data_size:%d, total_size:%d", DB_INTERNAL_ERROR, ptr_p->data_size, total_size);
            return pidx;
        }

        ptr_ctrl = _db_core.ptr_ctrl + req.t_idx;
        ptr_out_data += DB_MSG_PAYLOAD_SIZE;
        if(M_GET == method)
        {
            /* copy the raw data to client's buffer */
            ptr_raw_data = ptr_ctrl->table_handle;

            db_getDataFromRawTable((UI8_T *)ptr_out_data, ptr_raw_data, req);
            ptr_out_data += total_size;
        }
        else
        {
            UI16_T size = 0;
            void *ptr_data = NULL;

            ret = db_getFactoryDefault(req.t_idx, req.f_idx, req.e_idx, &size, (void **) &ptr_data);
            if (MW_E_OK == ret)
            {
                if (size != total_size)
                {
                    DB_LOG_ERROR("size not match, return size = %d, required size = %d", size, total_size);
                    MW_FREE(ptr_data);
                    return pidx;
                }
                osapi_memcpy(ptr_out_data, ptr_data, size);
                ptr_out_data += total_size;
                MW_FREE(ptr_data);
            }
        }
    }

    return ret;
}

/* FUNCTION NAME: db_setData
 * PURPOSE:
 *      Update and write the data to database
 *
 * INPUT:
 *      count           --  The count of request payloads
 *      method          --  The type of the update
 *      ptr_payload     --  A pointer to the request payload
 *
 * OUTPUT:
 *      ptr_updatedIdx  --  A pointer to the updated index array
 *
 * RETURN:
 *      The failed index of the requests which counts from 1 if encountered any error
 *      Otherwise return 0
 *
 * NOTES:
 *      the method is not used currently, since the database is fixed allocated.
 *
 */
UI8_T
db_setData(
    UI8_T count,
    UI8_T method,
    DB_PAYLOAD_T *ptr_payload,
    UI8_T *ptr_updatedIdx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_raw_data = NULL;
    void *ptr_out_data = NULL;
    DB_PAYLOAD_T *ptr_p = NULL;
    DB_REQUEST_TYPE_T req;
    UI16_T total_size = 0;
    UI8_T pidx = 1;

    if (NULL == ptr_payload)
    {
        DB_LOG_ERROR("%s: parameter is null", DB_INTERNAL_ERROR);
        return pidx;
    }

    ptr_out_data = (void *)ptr_payload;
    for (pidx = 1; pidx <= count; pidx++)
    {
        if (ptr_updatedIdx != NULL)
        {
            ptr_updatedIdx[pidx - 1] = FALSE;
        }
        ptr_p = (DB_PAYLOAD_T *)ptr_out_data;
        /* parse and verify the request type */
        ret = _db_parse_payload(ptr_p, &req);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s", "failed to parse request");
            return pidx;
        }

        /* Check the data size */
        ret = db_getDataSize(req, &total_size);
        if (MW_E_OK != ret)
        {
            return pidx;
        }
        if (0 != memcmp(&(ptr_p->data_size), &total_size, sizeof(ptr_p->data_size)))
        {
            /* ptr_payload->data_size != total_size */
            DB_LOG_ERROR("the request buffer size is not correct. 0x%x != 0x%x", ptr_p->data_size, total_size);
            DB_LOG_ERROR("t:%d f:%d e:%d pidx:%d count:%d", req.t_idx, req.f_idx, req.e_idx, pidx, count);
            return pidx;
        }

        /* copy the raw data to client's buffer */
        ptr_ctrl = _db_core.ptr_ctrl + req.t_idx;
        ptr_raw_data = ptr_ctrl->table_handle;
        ptr_out_data += DB_MSG_PAYLOAD_SIZE;

        /* The system command table */
        if (SYSTEM == req.t_idx)
        {
            _db_system_ctrl(req.t_idx, req.f_idx, ptr_out_data);
            ptr_out_data += total_size;
            continue;
        }
        /* The other tables */
        if (MW_E_OK == osapi_mutexTake(_db_core.ptr_data_mutex, DB_Q_WAITTIME))
        {
            ret = _db_setDataToRawTable(ptr_raw_data, (const UI8_T *)ptr_out_data, req);
            osapi_mutexGive(_db_core.ptr_data_mutex);
            if (MW_E_OK == ret)
            {
                if (NULL != ptr_updatedIdx)
                {
                    ptr_updatedIdx[pidx - 1] = TRUE;
                }
                ptr_out_data += total_size;
            }
            else if (MW_E_ALREADY_INITED == ret)
            {
                /* To update the data same as the current data */
                ptr_out_data += total_size;
            }
            else
            {
                DB_LOG_ERROR("Set raw data to DB failed, ret:%d", ret);
                DB_LOG_ERROR("t:%d f:%d e:%d pidx:%d count:%d", req.t_idx, req.f_idx, req.e_idx, pidx, count);
                return pidx;
            }
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME: db_resetData
 * PURPOSE:
 *      Reset the data in the database
 *
 * INPUT:
 *      method         --  The method to reset the data, inclueding delete, restore method.
 *      ptr_msg        --  A pointer to the request message
 *
 * OUTPUT:
 *      ptr_notify     --  A pointer to the notify message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OP_STOPPED
 *      MW_E_BAD_PARAMETER
 *
 *
 * NOTES:
 *      None
 *
 */
MW_ERROR_NO_T
db_resetData(
    const UI8_T     method,
    DB_MSG_T        *ptr_msg,
    DB_NOTIFY_T     *ptr_notify)
{
    void                    *ptr_out_data = NULL, *ptr_notify_data = NULL;
    UI8_T                   pidx = 0, pcount = 0;
    UI8_T                   *ptr_updatedIdx = NULL;
    UI8_T                   *ptr_raw_data = NULL, *ptr_reset_data = NULL;
    UI16_T                  data_size = 0, total_size = 0;
    DB_MSG_T                *ptr_notify_msg = NULL;
    DB_CTRL_T               *ptr_ctrl = _db_core.ptr_ctrl;
    DB_PAYLOAD_T            *ptr_p = NULL;
    MW_ERROR_NO_T           ret = MW_E_OK;
    DB_REQUEST_TYPE_T       req;

    MW_CHECK_PTR(ptr_msg);
    MW_CHECK_PTR(ptr_notify);
#ifdef DB_SUPPORT_RESTORE_METHOD
    if ((M_DELETE != method) && (M_RESTORE != method))
#else
    if (M_DELETE != method)
#endif
    {
        return MW_E_BAD_PARAMETER;
    }
    ptr_out_data = (void *)&(ptr_msg->ptr_payload);
    for (pidx = 0; pidx < ptr_msg->type.count; pidx++)
    {
        ptr_p = (DB_PAYLOAD_T *)ptr_out_data;
        /* parse and verify the request type */
        ret = _db_parse_payload(ptr_p, &req);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("failed to parse request");
            break;
        }
        if ((TABLES_LAST <= req.t_idx) ||
#ifdef DB_SUPPORT_RESTORE_METHOD
            ((M_RESTORE == method) && (DB_FIRST_OPER_TABLE <= req.t_idx)) ||
#endif
            (0 != ptr_p->data_size))
        {
            DB_LOG_ERROR("the request parameter(method:%d, t:%d/f:%d/e:%d, size:%d) is not correct", method, req.t_idx, req.f_idx, req.e_idx, ptr_p->data_size);
            break;
        }

        /* Get the data size */
        ret = db_getDataSize(req, &data_size);
        if (MW_E_OK != ret)
        {
            break;
        }
        total_size += DB_MSG_PAYLOAD_SIZE + data_size;
        ptr_out_data += DB_MSG_PAYLOAD_SIZE;
        /* fill in the data size of the data */
        osapi_memcpy((void *)&(ptr_p->data_size), (const void *)&data_size, sizeof(ptr_p->data_size));
    }
    if(0 == pidx)
    {
        /* If an error occurs when parsing the first payload, the subsquent flows are skipped */
        return MW_E_OP_STOPPED;
    }
    pcount = pidx;
    ret = db_calloc(pcount, (void **)&(ptr_updatedIdx));
    if (MW_E_OK != ret)
    {
        return ret;
    }
    /* Generate the notify message */
    ret = db_calloc(DB_MSG_HEADER_SIZE + total_size, (void **)&(ptr_notify_msg));
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_updatedIdx);
        return ret;
    }
    /* copy the header */
    osapi_memcpy((void *)ptr_notify_msg, (const void *)ptr_msg, DB_MSG_HEADER_SIZE);
    /* copy the raw data to notification buffer */
    ptr_notify_data = (void *)&(ptr_notify_msg->ptr_payload);
    ptr_out_data = (void *)&(ptr_msg->ptr_payload);
    for (pidx = 0; pidx < pcount; pidx++)
    {
        /* Catch the header information */
        osapi_memcpy((void *)&req, (const void *)ptr_out_data, sizeof(DB_REQUEST_TYPE_T));
        osapi_memcpy((void *)&data_size, (const void *)(ptr_out_data + sizeof(DB_REQUEST_TYPE_T)), sizeof(data_size));

        /* copy the payload header */
        osapi_memcpy(ptr_notify_data, (const void *)ptr_out_data, DB_MSG_PAYLOAD_SIZE);
        ptr_notify_data += DB_MSG_PAYLOAD_SIZE;
        ptr_out_data += DB_MSG_PAYLOAD_SIZE;

        /* reset the raw data as client's request */
        ptr_ctrl = _db_core.ptr_ctrl + req.t_idx;
        ptr_raw_data = ptr_ctrl->table_handle;
#ifdef DB_SUPPORT_RESTORE_METHOD
        if (M_RESTORE == method)
        {
            /* Reset data is factory default */
            ret = db_getFactoryDefault(req.t_idx, req.f_idx, req.e_idx, &total_size, (void **)&ptr_reset_data);
            if (MW_E_OK != ret)
            {
                DB_LOG_ERROR("Get table(%d) factory-default config failed, rc: %d", req.t_idx, ret);
                MW_FREE(ptr_updatedIdx);
                MW_FREE(ptr_notify_msg);
                return ret;
            }
            if (total_size != data_size)
            {
                DB_LOG_ERROR("Get (%d/%d/%d) factory-default config error, total_size: %d, data_size: %d", req.t_idx, req.f_idx, req.e_idx, total_size, data_size);
                MW_FREE(ptr_reset_data);
                MW_FREE(ptr_updatedIdx);
                MW_FREE(ptr_notify_msg);
                return MW_E_BAD_PARAMETER;
            }
            osapi_memcpy(ptr_notify_data, ptr_reset_data, data_size);
        }
        else
#endif /* DB_SUPPORT_RESTORE_METHOD */
        {
            /* Reset data is all 0 */
            ret = db_calloc(data_size, (void **)&(ptr_reset_data));
            if (MW_E_OK != ret)
            {
                MW_FREE(ptr_updatedIdx);
                MW_FREE(ptr_notify_msg);
                return ret;
            }
            db_getDataFromRawTable((UI8_T *)ptr_notify_data, ptr_raw_data, req);
        }

        if (MW_E_OK == osapi_mutexTake(_db_core.ptr_data_mutex, DB_Q_WAITTIME))
        {
            ret = _db_setDataToRawTable(ptr_raw_data, (const UI8_T *)ptr_reset_data, req);
            osapi_mutexGive(_db_core.ptr_data_mutex);
            if (MW_E_OK == ret)
            {
                ptr_updatedIdx[pidx] = TRUE;
                ptr_notify_data += data_size;
            }
            else if (MW_E_ALREADY_INITED == ret)
            {
                /* To update the data same as the current data */
                ptr_notify_data += data_size;
            }
            else
            {
                DB_LOG_WARN("Restore raw data to DB failed, ret:%d", ret);
                DB_LOG_WARN("t:%d f:%d e:%d pidx:%d", req.t_idx, req.f_idx, req.e_idx, pidx);
                MW_FREE(ptr_reset_data);
                MW_FREE(ptr_updatedIdx);
                MW_FREE(ptr_notify_msg);
                return ret;
            }
        }

        MW_FREE(ptr_reset_data);
    }

    ret = db_genNotification(pcount + 1, ptr_notify_msg, ptr_notify, ptr_updatedIdx);
    MW_FREE(ptr_updatedIdx);
    MW_FREE(ptr_notify_msg);
    return ret;
}

/* FUNCTION NAME: db_freeData
 * PURPOSE:
 *      Free the db data and subscriptions
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_freeData(
    void)
{


    /* Release the ctrl memory */
    if (NULL != _db_core.ptr_ctrl)
    {
        osapi_free(_db_core.ptr_ctrl);
    }

    /* Release the datastore memory */
    if (NULL != _db_core.ptr_db)
    {
        osapi_free(_db_core.ptr_db);
    }

    /* Rotate to release the subscriptions */
    if (NULL != _db_core.ptr_sub)
    {
        osapi_free(_db_core.ptr_sub);
    }

    /* Release the mutex */
    if (NULL != _db_core.ptr_data_mutex)
    {
        osapi_mutexDelete(_db_core.ptr_data_mutex);
    }

    return MW_E_OK;
}

/* FUNCTION NAME: db_initData
 * PURPOSE:
 *      Initialize the db data and subscriptions
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_initData(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T msize = 0;
#ifdef AIR_SUPPORT_POE
    UI8_T                        poe_port_number = 0;
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
#endif



    /* Initialize DB core */
    memset(&_db_core, 0, sizeof(DB_CORE_T));
    _db_core.ptr_db = NULL;
    _db_core.ptr_ctrl = NULL;
    _db_core.sub_count = 0;
    _db_core.ptr_sub = NULL;

    /* Check PLAT_MAX_PORT_NUM with MAX_PORT_NUM size */
    if (PLAT_MAX_PORT_NUM > MAX_PORT_NUM)
    {
        DB_LOG_ERROR("%s: Invalid MAX_PORT_NUM define", DB_INTERNAL_ERROR);
        return MW_E_NOT_SUPPORT;
    }

    /* Create Mutex for db data */
    ret = osapi_mutexCreate(DB_MAIN_TASK_NAME, &(_db_core.ptr_data_mutex));
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("%s: failed to create mutex(ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
        return ret;
    }

    /* Allocate db raw data memory */
    msize = sizeof(DB_TABLES_T);

    /* Caculate relate struct of Port based */
    msize += sizeof(DB_PORT_CFG_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    msize += sizeof(DB_PORT_QOS_T) * (PLAT_MAX_PORT_NUM - 1);
    msize += sizeof(DB_TRUNK_PORT_T) * (MAX_TRUNK_NUM - 1);
    msize += sizeof(DB_PORT_MIRROR_INFO_T) * (AIR_MAX_MIRROR_SESSION - 1);
#ifdef AIR_SUPPORT_DHCP_SNOOP
    msize += sizeof(DB_DHCP_SNP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#endif
    msize += sizeof(DB_PORT_OPER_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
#ifdef AIR_SUPPORT_SFP
    msize += sizeof(DB_SFP_MODULE_INFO_T) * (SFP_MODULE_INFO_MAX_NUM - 1);
#endif
    msize += sizeof(DB_MIB_CNT_T) * (PLAT_MAX_PORT_NUM - 1);
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    msize += sizeof(DB_STP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    msize += sizeof(DB_STP_PORT_OPER_T) * (PLAT_MAX_PORT_NUM - 1);
#ifdef AIR_SUPPORT_RSTP_SECURITY
    msize += sizeof(DB_RSTP_PORT_SEC_T) * (PLAT_MAX_PORT_NUM - 1);
#endif
#ifdef AIR_SUPPORT_MSTP
    msize += sizeof(DB_MSTP_INSTANCE_T) * (MAX_MSTP_INSTANCE_NUM - 1);
    msize += sizeof(DB_MSTP_INSTANCE_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    msize += sizeof(DB_MSTP_INSTANCE_PORT_OPER_T) * (PLAT_MAX_PORT_NUM - 1);
#endif
#endif /* AIR_SUPPORT_RSTP || AIR_SUPPORT_MSTP */
#ifdef AIR_SUPPORT_LLDPD
    msize += sizeof(DB_LLDP_PORT_INFO_T) * (PLAT_MAX_PORT_NUM - 1);
    msize += LLDP_CLIENT_INFO_SIZE * (PLAT_MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT - 1);
#endif
#ifdef AIR_SUPPORT_POE
    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL != ptr_poeCfg)
    {
        poe_port_number = ptr_poeCfg->poe_port_number;
    }
    else
    {
        poe_port_number = 1;
    }
    msize += sizeof(DB_POE_PORT_CFG_T) * (poe_port_number - 1);
    msize += sizeof(DB_POE_PORT_STATUS_T) * (poe_port_number - 1);
#endif

    _db_core.table_size = msize;

    ret = db_calloc(msize, (void **)&(_db_core.ptr_db));
    if (MW_E_OK != ret)
    {
        osapi_mutexDelete(_db_core.ptr_data_mutex);
        return ret;
    }

    /* Allocate db ctrl memory */
    msize = sizeof(DB_CTRL_T) * TABLES_LAST;
    ret = db_calloc(msize, (void **)&(_db_core.ptr_ctrl));
    if (MW_E_OK != ret)
    {
        osapi_free(_db_core.ptr_db);
        osapi_mutexDelete(_db_core.ptr_data_mutex);
        return ret;
    }

    /* Initialize the DB control structure */
    _db_ctrl_init(_db_core.ptr_db, _db_core.ptr_ctrl);

    /* Initialize the DB data structrue */
    _db_data_init(_db_core.ptr_db);


    return ret;
}

/* FUNCTION NAME: db_showFlashCfgData
 * PURPOSE:
 *      Show the config file in Flash
 *
 * INPUT:
 *      showstartup           -- Show the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_showFlashCfgData(
    BOOL_T showstartup)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T read_size = 0;
    UI8_T  part = DB_CONFIG_FACTORY_PART;
    C8_T *ptr_etoken = NULL;
    UI8_T *ptr_read_data = NULL;
    C8_T eof_str[3] = {'}', DB_ETX, 0};
    UI8_T  config_ver = 0;

    if (TRUE == showstartup)
    {
        part = DB_CONFIG_STARTUP_PART;
        MW_CMD_OUTPUT("\r\nshow the startup-config:\n");
    }
    else
    {
        MW_CMD_OUTPUT("\r\nshow the factory-defaults:\n");
    }

    ret = _db_is_cfg_exist(read_size, part, &config_ver);
    if (MW_E_OK != ret)
    {
        DB_LOG_ERROR("The config file does not exist.");
        return ret;
    }

    read_size = ((DB_CONFIG_VER_1 == config_ver) ? (DB_CONFIG_MAGIC_VER_LEN + 1) : (DB_CONFIG_MAGIC_LEN + 1));

    ret = db_calloc((DB_MAX_PRINTED_SIZE + 1), (void **)&ptr_read_data);
    if (MW_E_OK != ret)
    {
        return ret;
    }

    /* Find the EOF flag */
    while (TRUE)
    {
        ret = osapi_flashRead(read_size, DB_MAX_PRINTED_SIZE, ptr_read_data, part);
        if (MW_E_OK != ret)
        {
            DB_LOG_ERROR("%s: read config file from flash (ret = %u)", DB_INTERNAL_ERROR, (UI8_T)(ret));
            break;
        }

        ptr_etoken = strstr((C8_T *)ptr_read_data, eof_str);
        if  (NULL != ptr_etoken)
        {
            if (ptr_etoken <= ((C8_T *)ptr_read_data + DB_MAX_PRINTED_SIZE))
            {
                ptr_etoken++;
                *ptr_etoken = '\0';
            }
        }
        MW_CMD_OUTPUT("%s", ptr_read_data);
        read_size += DB_MAX_PRINTED_SIZE;
        if ((NULL != ptr_etoken) || (read_size >= DB_FLASH_CONFIGFILE_SIZE))
        {
            break;
        }
        memset(ptr_read_data, 0, DB_MAX_PRINTED_SIZE);
    }
    osapi_free(ptr_read_data);
    MW_CMD_OUTPUT("\n");
    return ret;
}

/* FUNCTION NAME: db_saveFlashCfgData
 * PURPOSE:
 *      Save the running DB to config file in Flash
 *
 * INPUT:
 *      savestartup           -- Save the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_saveFlashCfgData(
    BOOL_T savestartup)
{
    BOOL_T to_factory = (savestartup == FALSE)? TRUE : FALSE;

    return _db_data_save_running(to_factory);
}

/* FUNCTION NAME: db_getFlashCfg
 * PURPOSE:
 *      Check the config file exist and return the file size
 *
 * INPUT:
 *      ptr_file               -- The pointer of the config file
 *
 * OUTPUT:
 *      file_size              -- The size of the config file
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_getFlashCfg(
    UI8_T *ptr_file,
    UI32_T *ptr_file_size)
{
    C8_T eof_str[3] = {'}', DB_ETX, 0};
    C8_T *ptr_etoken = NULL;

    MW_CHECK_PTR(ptr_file);
    MW_CHECK_PTR(ptr_file_size);

    /* Check the magic word for recognization */
    if ((0 != osapi_strncmp((C8_T *)ptr_file, DB_MAGIC_WORD, DB_CONFIG_MAGIC_LEN)) &&
        (0 != osapi_strncmp((C8_T *)ptr_file, DB_MAGIC_WORD_VER, DB_CONFIG_MAGIC_VER_LEN)))
    {
        DB_LOG_INFO("The config file does not exist.");
        return MW_E_ENTRY_NOT_FOUND;
    }

    /* Check the end token word for recognization */
    ptr_etoken = strstr((C8_T *)ptr_file, eof_str);
    if (NULL == ptr_etoken)
    {
        DB_LOG_INFO("The config file does not exist.");
        return MW_E_ENTRY_NOT_FOUND;
    }
    if ((ptr_etoken + 2) <= ((C8_T *)ptr_file + DB_FLASH_CONFIGFILE_SIZE))
    {
        ptr_etoken += 2;
    }

    *ptr_file_size = ((UI8_T *)ptr_etoken) - ptr_file;
    return MW_E_OK;
}

/* FUNCTION NAME: db_dumpRaw
 * PURPOSE:
 *      Dump the DB Raw data for sepcific table and field
 *
 * INPUT:
 *      cfg_type        -- The config type
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      For RD Debugging only.
 *
 */
MW_ERROR_NO_T
db_dumpRaw(
    UI8_T cfg_type,
    UI8_T t_idx,
    UI8_T f_idx)
{
    UI16_T  get_data_size = 0;
    UI32_T idx = 0, i = 0;
    UI32_T raw_size = _db_core.table_size;
    UI8_T *ptr_raw = (UI8_T *)_db_core.ptr_db;
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    C8_T rawdata[4] = {0};
    C8_T *ptr_rawstr = NULL;
    void *ptr_raw_data = NULL;
    MW_ERROR_NO_T   rc = MW_E_OK;

    if(MW_E_OK != db_calloc(DB_BUFFER_SIZE_128, (void **)&ptr_rawstr))
    {
        return MW_E_NO_MEMORY;
    }

    if (TABLES_LAST != t_idx)
    {
        C8_T cfg_type_str[8] = {0};
        C8_T key_str[DB_MAX_KEY_SIZE] = {0};
        UI16_T data_size = 0;
        UI8_T *ptr_field = NULL;
        DB_REQUEST_TYPE_T request =
        {
            .t_idx = t_idx,
            .f_idx = f_idx,
            .e_idx = 0
        };
        db_getDataSize(request, &data_size);
        raw_size = data_size;
        ptr_ctrl += t_idx;
        MW_PARAM_CHK((ptr_ctrl->field_num < f_idx), MW_E_BAD_PARAMETER);
        if (DB_CFG_TYPE_RUNNING == cfg_type)
        {
            ptr_raw = ptr_ctrl->table_handle;
            osapi_snprintf(cfg_type_str, sizeof(cfg_type_str), "Running");
        }
        else
        {
            if (DB_CFG_TYPE_STARTUP == cfg_type)
            {
                rc = _db_getFlashCfgValue(t_idx, DB_ALL_FIELDS, DB_ALL_ENTRIES, DB_CONFIG_STARTUP_PART, &get_data_size, (void **)&ptr_raw_data);
                if (MW_E_OK != rc)
                {
                    MW_CMD_OUTPUT("Get startup config failed(%d)\n", rc);
                }
                osapi_snprintf(cfg_type_str, sizeof(cfg_type_str), "Startup");
            }
            else if (DB_CFG_TYPE_FACTORY == cfg_type)
            {
                rc = _db_getFlashCfgValue(t_idx, DB_ALL_FIELDS, DB_ALL_ENTRIES, DB_CONFIG_FACTORY_PART, &get_data_size, (void **)&ptr_raw_data);
                if (MW_E_OK != rc)
                {
                    MW_CMD_OUTPUT("Get factory config failed(%d)\n", rc);
                }
                osapi_snprintf(cfg_type_str, sizeof(cfg_type_str), "Factory");
            }
            else
            {
                rc = _db_getDefaultCfgValue(t_idx, DB_ALL_FIELDS, DB_ALL_ENTRIES, &get_data_size, (void **)&ptr_raw_data);
                if (MW_E_OK != rc)
                {
                    MW_CMD_OUTPUT("Get default config failed(%d)\n", rc);
                }
                osapi_snprintf(cfg_type_str, sizeof(cfg_type_str), "Default");
            }
            if (MW_E_OK == rc)
            {
                ptr_raw = ptr_raw_data;
            }
            else
            {
                MW_FREE(ptr_rawstr);
                return MW_E_OP_INCOMPLETE;
            }
        }
        ptr_field = ptr_ctrl->fields_size;
        db_cfgfile_getTableName(t_idx, sizeof(key_str), key_str);
        if (DB_ALL_FIELDS != f_idx)
        {
            C8_T fkey_str[DB_MAX_KEY_SIZE] = {0};
            db_cfgfile_getFieldName(t_idx, f_idx, sizeof(fkey_str), fkey_str);
            UI8_T field_size = *ptr_field;
            /* Switch the correct field */
            for (i= 1; i < f_idx; i++)
            {
                ptr_raw += (ptr_ctrl->entry_num * field_size);
                ptr_field++;
                field_size = *ptr_field;
            }
            MW_CMD_OUTPUT("Dump DB %s Raw data [T: %s(%d) / F: %s(%d)]:\n", cfg_type_str, key_str, t_idx, fkey_str, f_idx);
        }
        else
        {
            MW_CMD_OUTPUT("Dump DB %s Raw data [T: %s(%d)]:\n", cfg_type_str, key_str, t_idx);
        }
    }
    else
    {
        MW_CMD_OUTPUT("Dump DB Raw data All:\n");
    }

    while (idx < raw_size)
    {
        osapi_memset(ptr_rawstr, 0, DB_BUFFER_SIZE_128);
        osapi_snprintf(ptr_rawstr, DB_BUFFER_SIZE_128, "%08X: ", idx);
        for (i = 0; i < 16; i++)
        {
            osapi_sprintf(rawdata, " %02X", *ptr_raw);
            osapi_strcat(ptr_rawstr, rawdata);
            ptr_raw++;
            idx++;
            if (idx >= raw_size)
            {
                break;
            }
        }
        MW_CMD_OUTPUT("    %s\n", ptr_rawstr);
    }
    MW_FREE(ptr_rawstr);
    MW_FREE(ptr_raw_data);
    return MW_E_OK;
}

/* FUNCTION NAME: db_clrFlashCfgData
 * PURPOSE:
 *      Erase the config file in Flash
 *
 * INPUT:
 *      clrstartup           -- Clear the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
db_clrFlashCfgData(
    BOOL_T clrstartup)
{
    BOOL_T to_factory = (clrstartup == FALSE)? TRUE : FALSE;

    return _db_clear_cfgFile(to_factory);
}

/* FUNCTION NAME: db_getDbCoreCtrl
 * PURPOSE:
 *      Get the DB core control structure
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ptr_ctrl
 *
 * NOTES:
 *
 */
DB_CTRL_T *
db_getDbCoreCtrl(
    void)
{
    return _db_core.ptr_ctrl;
}

/* FUNCTION NAME: db_getTableFieldsNum
 * PURPOSE:
 *      Get the DB Specific table fields number
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      field_num       -- Total fields number
 *
 * NOTES:
 *
 */
UI8_T
db_getTableFieldsNum(
    UI8_T t_idx)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T field_num = 0;

    if (t_idx < TABLES_LAST)
    {
        ptr_ctrl += t_idx;
        field_num = ptr_ctrl->field_num;
    }
    return field_num;

}

/* FUNCTION NAME: db_getTableEntriesNum
 * PURPOSE:
 *      Get the DB Specific table entries number
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      entry_num
 *
 * NOTES:
 *
 */
UI16_T
db_getTableEntriesNum(
    UI8_T t_idx)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI16_T entry_num = 0;

    if (t_idx < TABLES_LAST)
    {
        ptr_ctrl += t_idx;
        entry_num = ptr_ctrl->entry_num;
    }
    return entry_num;
}

/* FUNCTION NAME: db_getTableFieldSize
 * PURPOSE:
 *      Get the DB Specific field size in spefic table
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      field_size
 *
 * NOTES:
 *      the f_idx is count start from 1
 *
 */
UI8_T
db_getTableFieldSize(
    UI8_T t_idx,
    UI8_T f_idx)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    UI8_T *ptr_fieldsizes = NULL;
    UI8_T field_size = 0;

    if (t_idx < TABLES_LAST)
    {
        ptr_ctrl += t_idx;
        ptr_fieldsizes = ptr_ctrl->fields_size;
        if (f_idx <= ptr_ctrl->field_num)
        {
            field_size = ptr_fieldsizes[f_idx - 1];
        }
    }
    return field_size;
}

/* FUNCTION NAME: db_getDbSubTreeRoot
 * PURPOSE:
 *      Get the DB Subscription Tree Root
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      ptr_sub
 *
 * NOTES:
 *
 */
DB_AVL_NODE_T **
db_getDbSubTreeRoot(
    void)
{
    return &(_db_core.ptr_sub);
}

/* FUNCTION NAME: db_getDbSubCount
 * PURPOSE:
 *      Get the DB Subscriber count
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      sub_count
 *
 * NOTES:
 *
 */
UI16_T
db_getDbSubCount(
    void)
{
    return _db_core.sub_count;
}

/* FUNCTION NAME: db_updateDbSubCount
 * PURPOSE:
 *      Update the DB Subscriber count and return the value
 *
 * INPUT:
 *      increase    -- To increase the count if set to TRUE, otherwise decrease
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      sub_count
 *
 * NOTES:
 *
 */
UI16_T
db_updateDbSubCount(
    UI8_T increase)
{
    if (TRUE == increase)
    {
        _db_core.sub_count++;
    }
    else
    {
        _db_core.sub_count--;
    }
    return _db_core.sub_count;
}

/* FUNCTION NAME: db_dumpTableCtrl
 * PURPOSE:
 *      Dump the DB current control structure information
 *
 * INPUT:
 *      detail      -- Detail level of the dump
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *      For RD Debugging only.
 *
 */
MW_ERROR_NO_T
db_dumpTableCtrl(
    UI8_T   detail)
{
    DB_CTRL_T *ptr_ctrl = _db_core.ptr_ctrl;
    DB_TABLES_T *ptr_db = _db_core.ptr_db;
    UI8_T *ptr_fieldsizes = NULL;
    C8_T key_str[DB_MAX_KEY_SIZE] = {0};
    UI8_T t_num = 0, f_num = 0;
    UI16_T table_size = 0;

    MW_CMD_OUTPUT("Dump Table control\n");
    MW_CMD_OUTPUT("Total Tables Number: %d, Total table size: %d\n", TABLES_LAST, _db_core.table_size);
    while(t_num < TABLES_LAST)
    {
        f_num = 0;
        table_size = 0;
        ptr_fieldsizes = ptr_ctrl->fields_size;
        memset(key_str, 0, sizeof(key_str));
        db_cfgfile_getTableName(t_num, sizeof(key_str), key_str);
        while (f_num < ptr_ctrl->field_num)
        {
            table_size += ptr_fieldsizes[f_num];
            f_num++;
        }
        table_size *= ptr_ctrl->entry_num;
        if(DB_CTRL_DUMP_LEVEL_ALL == detail)
        {
            MW_CMD_OUTPUT("(%d).\t%s(0x%08X),\tfields:%3u, entries:%5u, table size: %u, factory offt:0x%04X, startup offt:0x%04X\n",
                    t_num, key_str,
                    (UI32_T)(ptr_ctrl->table_handle - (void *)ptr_db) ,
                    ptr_ctrl->field_num,
                    ptr_ctrl->entry_num,
                    table_size,
                    ptr_ctrl->factory_offt,
                    ptr_ctrl->startup_offt);
            f_num = 0;
            while (f_num < ptr_ctrl->field_num)
            {
                f_num++;
                memset(key_str, 0, sizeof(key_str));
                db_cfgfile_getFieldName(t_num, f_num, sizeof(key_str), key_str);
                MW_CMD_OUTPUT("\tf(%d): %10s\tsize: %d\n",
                    f_num, key_str, ptr_fieldsizes[f_num - 1]);
            }
        }
        else if(DB_CTRL_DUMP_LEVEL_TABLE_SIZE == detail)
        {
            MW_CMD_OUTPUT("(%d).\t%10s,table size:%6u\n", t_num, key_str, table_size);
        }
        else
        {
            MW_CMD_OUTPUT("Invalid detail value.\n");
            return MW_E_BAD_PARAMETER;
        }
        t_num++;
        ptr_ctrl++;
    }
    return MW_E_OK;
}

/* FUNCTION NAME: db_getFactoryDefault
 * PURPOSE:
 *      Get default value from factory default for specific request.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
db_getFactoryDefault(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    MW_CHECK_PTR(ptr_data_size);
    MW_CHECK_PTR(pptr_data);
    MW_PARAM_CHK((DB_FIRST_OPER_TABLE <= t_idx), MW_E_BAD_PARAMETER);
    /* Get config from factory partition */
    rc = _db_getFlashCfgValue(t_idx, f_idx, e_idx, DB_CONFIG_FACTORY_PART, ptr_data_size, pptr_data);
    if (MW_E_OK != rc)
    {
        DB_LOG_INFO("The factory config is not available, try to get default config.");
        /* Get config from default partition */
        rc = _db_getDefaultCfgValue(t_idx, f_idx, e_idx, ptr_data_size, pptr_data);
    }
    return rc;
}

/* FUNCTION NAME: db_getStartUp
 * PURPOSE:
 *      Get the startup value from the startup-config for specific request.
 *
 * INPUT:
 *      t_idx             -- The index of table
 *      f_idx             -- The index of field
 *      e_idx             -- The index of entry
 *
 * OUTPUT:
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
db_getStartUp(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data)
{
    MW_CHECK_PTR(ptr_data_size);
    MW_CHECK_PTR(pptr_data);
    return _db_getFlashCfgValue(t_idx, f_idx, e_idx, DB_CONFIG_STARTUP_PART, ptr_data_size, pptr_data);
}

/* FUNCTION NAME: db_getDataFromRawTable
 * PURPOSE:
 *      Get data by specific request form a DB table raw data
 *
 * INPUT:
 *      ptr_raw_data      -- A pointer to the starting address of the raw data in a DB table
 *      req               -- A specific request
 *
 * OUTPUT:
 *      ptr_out_data      -- A pointer returns the raw data of the request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
db_getDataFromRawTable(
    UI8_T *ptr_out_data,
    const UI8_T *ptr_raw_data,
    const DB_REQUEST_TYPE_T req)
{
    UI8_T           field = 0, field_size = 0, field_num = 0;
    UI16_T          req_data_size = 0, entry_num = 0;

    MW_CHECK_PTR(ptr_out_data);
    MW_CHECK_PTR(ptr_raw_data);
    if (MW_E_OK != db_getDataSize(req, &req_data_size))
    {
        return MW_E_NOT_SUPPORT;
    }
    MW_PARAM_CHK((TABLES_LAST <= req.t_idx), MW_E_BAD_PARAMETER);
    field_num = db_getTableFieldsNum(req.t_idx);
    MW_PARAM_CHK((field_num < req.f_idx), MW_E_BAD_PARAMETER);
    entry_num = db_getTableEntriesNum(req.t_idx);
    MW_PARAM_CHK((entry_num < req.e_idx), MW_E_BAD_PARAMETER);
    if (DB_ALL_FIELDS == req.f_idx)
    {
        /* all table */
        if (DB_ALL_ENTRIES == req.e_idx)
        {
            osapi_memcpy(ptr_out_data, (const void *)ptr_raw_data, req_data_size);
        }
        else
        {
            /* all fields required with specific entry */
            for (field = 0; field < field_num; field++)
            {
                field_size = db_getTableFieldSize(req.t_idx, field + 1);
                osapi_memcpy(ptr_out_data, (const void *)(ptr_raw_data + ((req.e_idx - 1) * field_size)), field_size);
                ptr_out_data += field_size;
                ptr_raw_data += (entry_num * field_size);
            }
        }
    }
    else
    {
        /* Switch the correct field */
        for (field = 1; field < req.f_idx; field++)
        {
            field_size = db_getTableFieldSize(req.t_idx, field);
            ptr_raw_data += (entry_num * field_size);
        }
        /* Switch to specific entry if needed */
        if (DB_ALL_ENTRIES != req.e_idx)
        {
            field_size = db_getTableFieldSize(req.t_idx, req.f_idx);
            ptr_raw_data += ((req.e_idx - 1) * field_size);
        }
        /* Copy data to output buffer */
        osapi_memcpy(ptr_out_data, (const void *)ptr_raw_data, req_data_size);
    }
    return MW_E_OK;
}

/* FUNCTION NAME: db_isStartUpConfigExist
 * PURPOSE:
 *      Check if the startup configuration exists in the database.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_ENTRY_NOT_FOUND
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
db_isStartUpConfigExist(
    void)
{
    UI8_T           config_ver = 0;
    UI32_T          offset = 0;

    return _db_is_cfg_exist(offset, DB_CONFIG_STARTUP_PART, &config_ver);
}