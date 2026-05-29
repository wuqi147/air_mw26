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

/* FILE NAME:  db_data.h
 * PURPOSE:
 *      The data handling in DB task.
 *
 * NOTES:
 */

#ifndef DB_DATA_H
#define DB_DATA_H

/* INCLUDE FILE DECLARATIONS
*/
#include "osapi.h"
#include "db_api.h"
#include "db_util.h"
#include "db_avl_tree.h"
#include "db_notify.h"
#include "db_cfgfile.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/ip6_addr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define DB_FIRST_OPER_TABLE         SYS_OPER_INFO
#define DB_PARSE_CFG_DATA           BIT(0)
#define DB_PARSE_SPECIAL_DATA       BIT(1)
#define DB_PARSE_CFG_OFFT           BIT(2)
#define DB_NO_FLASH_CONFIG          (0xFFFF)

/* DATA TYPE DECLARATIONS
*/
/* All of the tables in the datastore */
typedef struct DB_TABLES_S
{
    DB_SYS_INFO_T            sys_info;          /* System information */
    DB_ACCOUNT_INFO_T        account_info;      /* User Account */
    DB_PORT_CFG_INFO_T       port_cfg;          /* Port Configuration */
    DB_PORT_QOS_T            port_qos;          /* Port QoS configuration */
    DB_TRUNK_PORT_T          trunk_port;        /* Trunk interface configuration */
    DB_TRUNK_ALGORITHM_T     trunk_algo;        /* Trunk algorithm configuration */
#ifdef AIR_SUPPORT_LP
    DB_LOOP_PREVEN_INFO_T    loop_preven;       /* Loop Prevention configuration */
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_IGMP_SNP
    DB_IGMP_SNP_INFO_T       igmp_snp;          /* IGMP Snooping configuration */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    DB_IGMP_SNP_QUERIER_INFO_T  igmp_snp_querier;  /* IGMP Snooping querier configuration */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */
    DB_PORT_MIRROR_INFO_T    port_mirror;       /* Port mirror sessions configuration */
    DB_JUMBO_FRAME_INFO_T    jumbo_frame;       /* Jumbo Frame configuration */
    DB_VLAN_CFG_INFO_T       vlan_cfg;          /* VLAN configuration */
    DB_VLAN_ENTRY_T          vlan_entry;        /* VLANs information */
#ifdef AIR_SUPPORT_VOICE_VLAN
    DB_VOICE_VLAN_INFO_T     voice_vlan;        /* Voice VLAN configuration */
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    DB_SURVEI_VLAN_INFO_T    survei_vlan;       /* Surveillance VLAN configuration */
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    DB_OUI_ENTRY_T           oui_entry;         /* OUI for Voice VLAN or Surveillance VLAN */
#endif
    DB_QOS_INFO_T            qos_info;          /* QoS configuration */
    DB_STATIC_MAC_ENTRY_T    static_mac_entry;  /* Static MAC table */
#ifdef AIR_SUPPORT_DHCP_SNOOP
    DB_DHCP_SNP_INFO_T       dhcp_snp;          /* DHCP snooping configuration */
    DB_DHCP_SNP_PORT_INFO_T  dhcp_snp_port;     /* DHCP snooping option 82 configuration */
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    DB_SNMP_INFO_T           snmp_info;         /* snmp information */
#endif
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    DB_STP_INFO_T            stp;               /* STP configuration */
    DB_STP_PORT_INFO_T       stp_port;         /* STP port configuraion */
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
    DB_MQTTD_CFG_INFO_T      mqttd_cfg;         /* MQTTD configuration */
#endif
#ifdef AIR_SUPPORT_LLDPD
    DB_LLDP_INFO_T           lldp_info;         /* LLDP global config info*/
    DB_LLDP_PORT_INFO_T      lldp_port;         /* LLDP port config info */
#endif
#ifdef AIR_SUPPORT_POE
    DB_POE_CFG_T             poe_cfg;           /* POE config info*/
    DB_POE_PORT_CFG_T        poe_port_cfg;      /* POE config info*/
#endif
#ifdef AIR_SUPPORT_SNTP
    DB_SNTP_CFG_T            sntp_cfg;
#endif
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    DB_RSTP_PORT_SEC_T       rstp_port_sec;     /* RSTP port sec configuraion */
#endif
#ifdef AIR_SUPPORT_ERPS
    DB_ERPS_INFO_T           erps_info;         /* ERPS info */
#endif
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_REGION_CONFIG_T      mstp_region;        /* MSTP region config */
    DB_MSTP_INSTANCE_T           mstp_instance;      /* MSTP instance */
    DB_MSTP_INSTANCE_PORT_INFO_T mstp_ins_port;      /* MSTP instance port */
#endif
    DB_SYS_OPER_INFO_T       sys_oper_info;     /* System operational information */
    DB_PORT_OPER_INFO_T      port_oper;         /* Port operational information */
    DB_LAG_OPER_INFO_T       lag_oper_info;     /* LAG operational information */
    DB_LAG_MEMBER_0_T        lag_member_0;      /* LAG member 0 */
#ifdef AIR_SUPPORT_CABLE_DIAG
    DB_PORT_DIAG_T           port_diag;         /* Port Cable Diagnostic */
#endif
    DB_LOGON_INFO_T          logon_info;        /* User Logon information */
    DB_L2_MC_ENTRY_T         l2_mc_entry;       /* IGMP Snooping learned entries */
    DB_SYSTEM_T              system;            /* System control */
#ifdef AIR_SUPPORT_ICMP_CLIENT
    DB_ICMP_CLIENT_INFO_T      icmp_client_info;    /* Icmp client info */
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
    DB_SFP_MODULE_INFO_T     sfp_module_info;   /* SFP module information */
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
    DB_VOICE_OPER_INFO_T     voice_oper;        /* Voice VLAN operational status */
#endif
    DB_MIB_CNT_T             mib_cnt;           /* MIB counters */
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T dynamic_mac_address_entry_cfg;  /* Dynamic MAC address entry configuration */
    DB_DYNAMIC_MAC_ADDRESS_ENTRY_T          dynamic_mac_address_entry;      /* Dynamic MAC address entry table */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    DB_STP_PORT_OPER_T       stp_port_oper;    /* STP port operational status */
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_PORT_OPER_T mstp_ins_port_oper; /* MSTP instance port oper */
#endif
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_LLDPD
    DB_LLDP_CLIENT_INFO_T    lldp_client;       /* LLDP client info*/
#endif
#ifdef AIR_SUPPORT_POE
    DB_POE_STATUS_T          poe_status;        /* POE status*/
    DB_POE_PORT_STATUS_T     poe_port_status;   /* POE port status*/
#endif
} __attribute__((packed)) DB_TABLES_T;

typedef struct DB_CTRL_S
{
    void                     *table_handle;    /* The raw data stored */
    UI8_T                    field_num;        /* amount fields in table */
    UI16_T                   entry_num;        /* amount entries in table */
    UI8_T                    *fields_size;     /* Each field size in table */
    UI16_T                   factory_offt;     /* Data start offset of flash start position*/
    UI16_T                   startup_offt;     /* Data start offset of flash start position*/
} DB_CTRL_T;

typedef struct DB_CORE_S
{
    DB_TABLES_T              *ptr_db;          /* The raw data stored */
    DB_CTRL_T                *ptr_ctrl;        /* The control info of each table */
    UI16_T                   sub_count;        /* The subsciption count */
    DB_AVL_NODE_T            *ptr_sub;         /* The subscriptions stored */
    semaphorehandle_t        ptr_data_mutex;   /* The mutext to protect DB data */
    UI32_T                   table_size;
} DB_CORE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
*/
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
    UI8_T entry_num);

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
    UI16_T *ptr_total_size);

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
 *      The failed index of the requests which counts from 1 if encountered any error
 *      Otherwise return 0
 *
 * NOTES:
 *      the method is not used currently, since the database is fixed allocated.
 *
 */
UI8_T
db_getData(
    UI8_T count,
    UI8_T method,
    DB_PAYLOAD_T *ptr_payload);

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
    DB_NOTIFY_T     *ptr_notify);

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
    UI8_T *ptr_updatedIdx);

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
    void);

/* FUNCTION NAME: db_initData
 * PURPOSE:
 *      Initialize the db data and variables
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
    void);

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
    BOOL_T showstartup);

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
    BOOL_T savestartup);

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
 *      Only allow to access the startup config
 *
 */
MW_ERROR_NO_T
db_getFlashCfg(
    UI8_T *ptr_file,
    UI32_T *ptr_file_size);

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
    UI8_T f_idx);

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
    BOOL_T clrstartup);

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
    void);

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
 *      field_num
 *
 * NOTES:
 *
 */
UI8_T
db_getTableFieldsNum(
    UI8_T t_idx);

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
    UI8_T t_idx);

/* FUNCTION NAME: db_getTableFieldSize
 * PURPOSE:
 *      Get the field size in specific table
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
 *
 */
UI8_T
db_getTableFieldSize(
    UI8_T t_idx,
    UI8_T f_idx);

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
    void);

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
    void);

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
    UI8_T increase);

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
    UI8_T   detail);

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
    void **pptr_data);

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
    void **pptr_data);

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
    const DB_REQUEST_TYPE_T req);

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
    void);

#endif  /* End of DB_DATA_H */
