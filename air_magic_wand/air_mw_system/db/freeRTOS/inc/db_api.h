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

/* FILE NAME:  db_api.h
 * PURPOSE:
 *      Define the common DB API in AIR Magic Wand system.
 *
 * NOTES: DB clients use the message queue to communicate with DB task.
 *        The queue format is as following, each item in queue is 8 bytes:
 *        +-------+
 *        |ptr_msg|
 *        +-------+
 *        - ptr_msg: 4 bytes: a message pointer.
 *
 *        The data format for Request and Notification is as following:
 *        +-------+-------+-------+-------+-------+-------+------------+--------+
 *        |cq_name|method |count  |t_idx  |f_idx  |e_idx  |data_size   |raw data|
 *        +-------+-------+-------+-------+-------+-------+------------+--------+
 *        - cq_name: 4 bytes: client's queue name.
 *        - method: 1 byte: the method bitmap.
 *        - count: 1 byte: the incoming payload count in this transaction.
 *            Note that Only subscription method allowed more than 1 payload.
 *            The other request type must be 1.
 *        - t_idx: 1 byte: the enum of the table in TABLES_T.
 *        - f_idx: 1 byte: the enum of the field in the TABLE.
 *        - e_idx: 2 bytes: the entry index in the table. If there is only one
 *            entry in the field, then the value in E_IDX should be 0.
 *        - data_Size: 2 bytes: the size of the coming buffer. If there is no
 *            data in this message payload, then the value should be 0.
 *        - raw data: N bytes: the message data body.
 *            Note that if client requests to get data from DB, then client must
 *            allocates a memory to store the data and append the buffer as raw
 *            data. DB would copy the data from database to the buffer.
 *
 *        The data format for Response is as following:
 *        +-------+-------+-------+-------+-------+-------+------------+--------+
 *        |cq_name|method |result |t_idx  |f_idx  |e_idx  |data_size   |raw data|
 *        +-------+-------+-------+-------+-------+-------+------------+--------+
 *        - Result: 1 byte: the result status of the request.
 *        Note that DB would return the t_idx, f_idx, e_idx as client requested
 *        to indicate that this message responsed to which request.
 */
#ifndef DBAPI_H
#define DBAPI_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_types.h"
#include "mw_utils.h"
#include "mw_platform.h"
#include "mw_portbmp.h"
#include "../config/default_config.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define DB_ALL_FIELDS   (0)
#define DB_ALL_ENTRIES  (0)
#define DB_REQUEST_Q_NAME "dbq"
#define DB_Q_NAME_SIZE  (4)
#define DB_Q_CLIENT_LEN (32)
#define DB_Q_WAITTIME   (0xFFFFFFFF)

/* The bitmap of message method */
#define M_B_WRITE      BIT(0) /* Determine to write data if the bit set */
#define M_B_CREATE     BIT(1) /* used to create new entries or subscribe */
#define M_B_DELETE     BIT(2) /* Used to delete entries or unsubscribe */
#define M_B_SUBSCRIBED BIT(3) /* Used to subscribed or unsubscribe data*/
#define M_B_GETFLASH   BIT(4)
/* bit4:6 Reserved bits */
#define M_B_RESPONSE   BIT(7) /* The response bit */
/* Methods Value */
#define M_GET          (0x00)   /* The GET method */
#define M_GETFLASH     (M_B_GETFLASH)   /* The GET method for get flash data */
#define M_UPDATE       (M_B_WRITE) /* The Update method */
#define M_CREATE       ((M_B_WRITE) | (M_B_CREATE))  /* To create new entries */
#define M_DELETE       ((M_B_WRITE) | (M_B_DELETE))  /* To delete entries */
#ifdef  DB_SUPPORT_RESTORE_METHOD
#define M_RESTORE      ((M_B_WRITE) | (M_B_GETFLASH)) /* The restore method */
#endif
#define M_SUBSCRIBE    ((M_B_SUBSCRIBED ) | (M_B_CREATE)) /* To subscribe */
#define M_UNSUBSCRIBE  ((M_B_SUBSCRIBED ) | (M_B_DELETE)) /* To unsubscribe */
#define M_RESPONSE     (M_B_RESPONSE) /* Determine this responsed to a request*/
#define M_ACK          ((M_B_RESPONSE) | (M_B_SUBSCRIBED)) /* Responsed to a subscription */
#define M_CLIENT_REQ   (0x1F)   /* WRITE | CREATE | DELETE | SUBSCRIBE */
/* DB notification DB_MSG_T.type.count is not supposed to be greater than 127. */
#define DB_COUNT_REUSE_FLAG    BIT(7)
/* The Abitlities */
#define MAX_TRUNK_MEMBER_NUM (AIR_MAX_TRUNK_MEMBER_NUM)   /* Maximum trunk members */
#define MAX_L2MC_NUM         (AIR_IGMP_MAX_L2MC_ENTRY_NUM)  /* Maximum multicast groups */
#define MAX_8021P_NUM        8   /* Maximum CoS mapping to queues */
#define MAX_PRIORITY_NUM     8   /* Maximum priority to queues */
#ifdef AIR_SUPPORT_MQTTD
#define MAX_SUB_NUM          162 /* Maximum Subscriptions */
#else
#define MAX_SUB_NUM          128 /* Maximum Subscriptions */
#endif
#define MAX_PAIR_NUM         4   /* Maximum cable pair number */

/* The message size */
#define DB_MSG_PTR_SIZE       (4)
#define DB_MSG_HEADER_SIZE    (sizeof(DB_MSG_T) - DB_MSG_PTR_SIZE)     /*size of message header, includes cq_name+method+type.count or type.result */
#define DB_MSG_PAYLOAD_SIZE   (sizeof(DB_PAYLOAD_T) - DB_MSG_PTR_SIZE) /*size of payload header, includes t_idx+f_idx+e_idx+data_size */

/* The config name buffer size */
#define DB_MAX_KEY_SIZE     (12)    /* maximum size of key name */
#define DB_ALL_KEY          "all"   /* represent DB_ALL_FIELDS */

/* The icmp client error code */
#ifdef AIR_SUPPORT_ICMP_CLIENT
#define AIR_ICMP_CLIENT_ERR_STOPPED         (0)
#define AIR_ICMP_CLIENT_ERR_SUCCESS         (1UL << 0)
#define AIR_ICMP_CLIENT_ERR_START           (1UL << 1)
#define AIR_ICMP_CLIENT_PROCESSING          (1UL << 2)
#define AIR_ICMP_CLIENT_DNS_PROCESSING      (1UL << 3)
#define AIR_ICMP_CLIENT_DNS_TIMEOUT         (1UL << 4)
#define AIR_ICMP_CLIENT_CANCELED            (1UL << 5)
#define AIR_ICMP_CLIENT_TIMEOUT             (1UL << 6)
#endif /* AIR_SUPPORT_ICMP_CLIENT */

/* The Dynamic MAC configuration info error code */
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_DEFAULT              (0)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_START                (1)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_ACTION_CONTINUE             (2)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_DONE           (4)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_START_END            (5)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_DONE        (6)
#define AIR_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_RESULT_CONTINUE_END         (7)

#ifdef AIR_SUPPORT_LLDPD
#define MAX_LLDP_STR_SIZE    64   /* Maximum LLDP str length*/
#define MAX_NETWORK_POLICY_TLV_NUM         2
#define MAX_LLDP_CLIENT_PER_PORT           1
#define LLDP_CLIENT_INFO_SIZE              210
#define MAX_LLDP_NEIGHBOR_NUM              (PLAT_MAX_PORT_NUM * MAX_LLDP_CLIENT_PER_PORT)
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* Enumerate the tables and fields in database */
/* All of the tables in the datastore */
typedef enum
{
    SYS_INFO = 0,          /* System information */
    ACCOUNT_INFO,          /* User Account */
    PORT_CFG_INFO,         /* Port Configuration */
    PORT_QOS,              /* Port QoS configuration */
    TRUNK_PORT,            /* Trunk interface configuration */
    TRUNK_ALGORITHM,       /* Trunk algorithm configuration */
#ifdef AIR_SUPPORT_LP
    LOOP_PREVEN_INFO,      /* Loop Prevention configuration */
#endif /* AIR_SUPPORT_LP */
#ifdef AIR_SUPPORT_IGMP_SNP
    IGMP_SNP_INFO,         /* IGMP Snooping configuration */
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    IGMP_SNP_QUERIER_INFO, /* IGMP Snooping querier configuration */
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */
    PORT_MIRROR_INFO,      /* Port mirror sessions configuration */
    JUMBO_FRAME_INFO,      /* Jumbo Frame configuration */
    VLAN_CFG_INFO,         /* VLAN configuration */
    VLAN_ENTRY,            /* VLANs information */
#ifdef AIR_SUPPORT_VOICE_VLAN
    VOICE_VLAN_INFO,       /* Voice VLAN configuration */
#endif
#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    SURVEI_VLAN_INFO,      /* Surveillance VLAN configuration */
#endif
#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
    OUI_ENTRY,             /* OUI entries for voice vlan or surveillance vlan */
#endif
    QOS_INFO,              /* QoS configuration */
    STATIC_MAC_ENTRY,      /* Static MAC table */
#ifdef AIR_SUPPORT_DHCP_SNOOP
    DHCP_SNP_INFO,         /* DHCP Snooping configuration */
    DHCP_SNP_PORT_INFO,   /* DHCP Snooping option 82 configuration */
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
    SNMP_INFO,             /* SNMP information */
#endif
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    STP_INFO,
    STP_PORT_INFO,
#endif /* AIR_SUPPORT_RSTP, AIR_SUPPORT_MSTP */
#ifdef AIR_SUPPORT_MQTTD
    MQTTD_CFG_INFO,        /* MQTTD configuration for C-MW */
#endif
#ifdef AIR_SUPPORT_LLDPD
    LLDP_INFO,             /* LLDP global config info*/
    LLDP_PORT_INFO,        /* LLDP port config info */
#endif
#ifdef AIR_SUPPORT_POE
    POE_CFG,
    POE_PORT_CFG,
#endif
#ifdef AIR_SUPPORT_SNTP
    SNTP_CFG,
#endif
#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_RSTP_SECURITY)
    RSTP_PORT_SECURITY,
#endif
#ifdef AIR_SUPPORT_ERPS
    ERPS_INFO,             /* ERPS global config info */
#endif
#ifdef AIR_SUPPORT_MSTP
    MSTP_REGION,
    MSTP_INSTANCE,
    MSTP_INSTANCE_PORT,
#endif
    /* Below tables will not keep in configuration file */
    SYS_OPER_INFO,         /* System operational information */
    PORT_OPER_INFO,        /* Port operational information */
    LAG_OPER_INFO,         /* LAG operational information */
    LAG_MEMBER_0_INFO,     /* LAG member 0 information  */
#ifdef AIR_SUPPORT_CABLE_DIAG
    PORT_DIAG,             /* Port Cable Diagnostic */
#endif
    LOGON_INFO,            /* User Logon information */
    L2_MC_ENTRY,           /* IGMP Snooping learned entries */
    SYSTEM,                /* System control */
#ifdef AIR_SUPPORT_ICMP_CLIENT
    ICMP_CLIENT_INFO,      /* ICMP Client info */
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SFP
    SFP_MODULE_INFO,        /* SFP Module info */
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_VOICE_VLAN
    VOICE_OPER_INFO,            /* Voice VLAN operational status */
#endif
    MIB_CNT,               /* MIB counters */
    DYNAMIC_MAC_ADDRESS_ENTRY_CFG,  /* Dynamic MAC entry configuration */
    DYNAMIC_MAC_ADDRESS_ENTRY,     /* Dynamic MAC entry table */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    STP_PORT_OPER_INFO,
#ifdef AIR_SUPPORT_MSTP
    MSTP_PORT_OPER_INFO,
#endif
#endif
#ifdef AIR_SUPPORT_LLDPD
    LLDP_CLIENT_INFO,      /* LLDP client info*/
#endif
#ifdef AIR_SUPPORT_POE
    POE_STATUS,
    POE_PORT_STATUS,
#endif
    TABLES_LAST
} TABLES_T;

/* The fields in the SYS_OPER_INFO table */
typedef enum
{
    SYS_OPER_MAC = 1,      /* System MAC address */
    SYS_OPER_HW_VER,       /* System HW version */
    SYS_OPER_IP_ADDR,      /* The operational IP address of system interface */
    SYS_OPER_IP_MASK,      /* The operational IP Mask of system interface */
    SYS_OPER_IP_GW,        /* The operational gateway of system interface */
    SYS_OPER_IP_DNS,       /* The operational DNS of system interface */
#ifdef AIR_SUPPORT_SECOND_NETIF
    SYS_OPER_IP2_ADDR,     /* The operational IP address of second system interface */
    SYS_OPER_IP2_MASK,     /* The operational IP Mask of second system interface */
#endif
#ifdef AIR_SUPPORT_IPV6
    SYS_OPER_IP6_ADDR_1, /*  The operational IPv6 address 1 of system interface */
    SYS_OPER_IP6_ADDR_2, /*  The operational IPv6 address 2 of system interface */
    SYS_OPER_IP6_LINK_LOCAL_ADDR, /* The operational IPv6 link local address of system interface */
    SYS_IP6_DAD_RESULT,      /* The operational result of IPv6 DAD */
#endif
    SYS_OPER_INFO_LAST
} SYS_OPER_INFO_T;

/* The fields in the PORT_OPER_INFO table */
typedef enum
{
    PORT_OPER_STATUS = 1,  /* The operational status of the port */
    PORT_OPER_SPEED,       /* The operational speed of the port */
    PORT_OPER_DUPLEX,      /* The operational duplex mode of the port */
    PORT_OPER_FLOW_CTRL,   /* The operational flow ctrl mode of the port */
    PORT_LOOP_STATE,       /* The looped state of the port */
    PORT_OPER_ROUTER,      /* The dynamic router of IGMP snooping */
    PORT_MIB_COUNTER_CLEAR,  /* The clear operation of MIB counter for the port. */
    PORT_OPER_MODE,        /* The operation port mode of the port. */
    PORT_LACP_STATE,       /* The LACP state of the port */
    PORT_OPER_INFO_LAST
} PORT_OPER_INFO_T;

typedef enum
{
    LAG_PROCESS_STATE = 1,
    LAG_OPER_INFO_LAST
} LAG_OPER_INFO_T;

typedef enum
{
    MEMBER_0 = 1,
    LAG_MEMBER_0_LAST
} LAG_MEMBER_0_T;

#ifdef AIR_SUPPORT_CABLE_DIAG
/* The fields in the PORT_DIAG table */
typedef enum
{
    PORT_CABLE_SET = 1,    /* The admin mode of cable diagnosis */
    PORT_CABLE_LENGTH,
    PORT_CABLE_STATE,
    PORT_CABLE_IMPEDANCE,
    PORT_CABLE_PORT,
    PORT_CABLE_DIAG_LAST
} PORT_CABLE_DIAG_T;
#endif

/* The fields in the LOGON_INFO table */
typedef enum
{
    LOGON_FAIL_COUNT = 1,
    LOGON_INFO_LAST
} LOGON_INFO_T;

/* The fields in the L2_MC_ENTRY */
typedef enum
{
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    L2_MC_MAC_ADDR = 1,    /* The multicast MAC address */
#else
    L2_MC_IP_ADDR = 1,     /* The multicast IP address */
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    L2_MC_VID,             /* The multicast VLAN ID */
    L2_MC_PORTBITMAP,      /* The port bitmap of the VLAN */
    L2_MC_UPDATE_FLAG,     /* The update_flag is used to ensure that igmp snooping group entry can be updated to DB
                              when mac_addr/ip_addr, vid, portbitmap have not changed. */
    L2_MC_ENTRY_LAST
} L2_MC_ENTRY_T;

/* System control */
typedef enum
{
    SYS_SAVE_RUNNING = 1,  /* Save the running data to flash */
    SYS_RESET,             /* Reset the system */
    SYS_RESET_FACTORY,     /* Reset to factory default */
    SYSTEM_LAST
} SYSTEM_T;

#ifdef AIR_SUPPORT_ICMP_CLIENT
/* The fields in the ICMP_CLIENT table */
typedef enum
{
    ICMP_CLIENT_HOST_NAME = 1,
    PING_IP,
    NUM,
    STATUS,
    SEND_PACKETS,
    RECV_PACKETS,
    LOST_PACKETS,
    MIN_RTT,
    MAX_RTT,
    AVE_RTT,
    ICMP_CLIENT_INFO_LAST
} ICMP_CLIENT_INFO_T;
#endif /* AIR_SUPPORT_ICMP_CLIENT */

#ifdef AIR_SUPPORT_SFP
/*The fields in the SFP_MODULE_INFO table */
typedef enum
{
    SFP_MODULE_PORTNUM = 1,
    DIAGNOSTIC_MONITORING_TYPE,
    TEMPERATURE,
    VOLTAGE,
    CURRENT,
    OUTPUT_POWER,
    INPUT_POWER,
    LOSS_OF_SIGNAL,
    SFP_MODULE_INFO_LAST
} SFP_MODULE_INFO_T;
#endif /* AIR_SUPPORT_SFP */

#ifdef AIR_SUPPORT_VOICE_VLAN
/* The fields in the VOICE_VLAN_OPER table */
typedef enum
{
    VOICE_OPER_MEMBER_STATE = 1,   /* The joined voice VLAN port bitmap */
    VOICE_OPER_INFO_LAST
} VOICE_OPER_INFO_T;
#endif

/* The fields in the MIB_CNT table */
typedef enum
{
    MIB_CNT_RX_PACKETS = 1,     /* The amount of packets received of the port */
#ifdef AIR_SUPPORT_SNMP
    MIB_CNT_RX_UNICAST_PACKETS,
    MIB_CNT_RX_MULTICAST_PACKETS,
    MIB_CNT_RX_BROADCAST_PACKETS,
    MIB_CNT_RX_DISCARDS_PACKETS,
#endif
    MIB_CNT_RX_OCTETS,          /* The amount of octets received of the port */
    MIB_CNT_RX_ERRORS,          /* The amount of RX errors */
    MIB_CNT_TX_PACKETS,         /* The amount of packets transmitted of the port */
#ifdef AIR_SUPPORT_SNMP
    MIB_CNT_TX_UNICAST_PACKETS,
    MIB_CNT_TX_MULTICAST_PACKETS,
    MIB_CNT_TX_BROADCAST_PACKETS,
    MIB_CNT_TX_DISCARDS_PACKETS,
#endif
    MIB_CNT_TX_OCTETS,          /* The amount of octets transmitted of the port */
    MIB_CNT_TX_ERRORS,          /* The amount of TX errors */
    MIB_CNT_LAST
} MIB_CNT_T;

/* The fields in the DYNAMIC_MAC_ADDRESS_ENTRY_CFG table */
typedef enum
{
    ACTION_RESULT = 1,                            /* The action or result */
    DYNAMIC_ENTRY_COUNT,                          /* The dynamic entry count */
    DYNAMIC_MAC_ADDRESS_ENTRY_CFG_LAST
} DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T;

/* The fields in the DB_DYNAMIC_MAC_ADDRESS_ENTRY table */
typedef enum
{
    DYNAMIC_MAC_ADDRESS_ENTRY_ADDR = 1,                          /* The dynamic MAC address entry address */
    DYNAMIC_MAC_ADDRESS_ENTRY_VID,                               /* The VID of the dynamic MAC address entry */
    DYNAMIC_MAC_ADDRESS_ENTRY_PORT,                              /* The port bitmap of the dynamic MAC address entry */
    DYNAMIC_MAC_ADDRESS_ENTRY_AGE,                               /* The aging time of the dynamic MAC address entry */
    DYNAMIC_MAC_ADDRESS_ENTRY_LAST
} ATTRIBUTE_PACK DYNAMIC_MAC_ADDRESS_ENTRY_T;

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* The fields in the RSTP_PORT_OPER_INFO */
typedef enum
{
    STP_PORT_OPER_STATE = 1,
    STP_PORT_OPER_ROLE,
    STP_PORT_OPER_BLOCK,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    STP_PORT_OPER_COST,
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    STP_PORT_OPER_INFO_LAST
} STP_PORT_OPER_INFO_T;

#ifdef AIR_SUPPORT_MSTP
/* The fields in the RSTP_PORT_OPER_INFO */
typedef enum
{
    MSTP_INSTANCE_PORT_OPER_STATE = 1,
    MSTP_INSTANCE_PORT_OPER_ROLE,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    MSTP_INSTANCE_PORT_OPER_COST,
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    MSTP_INSTANCE_PORT_OPER_INFO_LAST
} MSTP_INSTANCE_PORT_OPER_INFO_T;
#endif
#endif /* AIR_SUPPORT_RSTP */

#ifdef AIR_SUPPORT_LLDPD
typedef enum
{
    CHASSIS_ID = 1,
    PORT_ID,
    TTL,
    SYSTEM_NAME,
    MED_CAPABILITY,
    MED_NET_POLICY,
    MED_EXTEN_POWER,
    LLDP_CLIENT_INFO_LAST,
} LLDP_CLIENT_INFO_T;
#endif

#ifdef AIR_SUPPORT_POE
typedef enum
{
    SUPPLY_VOLTAGE = 1,
    TOTAL_CONSUMED_POWER,
    TOTAL_CONSUMED_CURRENT,
    IS_POE_PORT,
    POE_POWER_STRATEGY,
    POE_STATUS_LAST
} ATTRIBUTE_PACK POE_STATUS_T;

typedef enum
{
    PORT_POWER_STATUS = 1,
    PORT_PD_CLASS,
    PORT_CONSUMED_POWER,
    PORT_VOLTAGE,
    PORT_CURRENT,
    PORT_POWER_EVENT,
    POE_PORT_STATUS_LAST
} ATTRIBUTE_PACK POE_PORT_STATUS_T;
#endif

/* The structure of each request or response in Queue to communicate with DB */
typedef struct DB_REQUEST_TYPE_S
{
    UI8_T           t_idx;         /* The table index */
    UI8_T           f_idx;         /* The field index */
    UI16_T          e_idx;         /* The entry index */
} ATTRIBUTE_PACK DB_REQUEST_TYPE_T;

/* The payload format in the message */
typedef struct DB_PAYLOAD_S
{
    DB_REQUEST_TYPE_T  request;    /* The type of the message */
    UI16_T          data_size;     /* The incoming data size */
    void            *ptr_data;     /* The data body, not a real pointer */
} ATTRIBUTE_PACK DB_PAYLOAD_T;

/* The message format in the queue item */
typedef struct DB_MSG_S
{
    C8_T            cq_name[DB_Q_NAME_SIZE];    /* The client's queue name string */
    UI8_T           method;        /* The method bitmap */
    union {
        UI8_T       count;         /* The data payload count in request or notification, max is 127 */
        UI8_T       result;        /* The response result with type MW_ERROR_NO_T */
    } type;
    DB_PAYLOAD_T    *ptr_payload;  /* The payload body, not a real pointer  */
} ATTRIBUTE_PACK DB_MSG_T;

/* The structure declartion of each tables */
/* Below tables will not keep in configuration file */
/* The system operational information table */
typedef struct DB_SYS_OPER_INFO_S
{
    MW_MAC_T        sys_mac;                        /* System MAC address */
    UI8_T           hw_version[MAX_VERSION_SIZE];   /* System HW version */
    MW_IPV4_T       ip_addr;                        /* The operational IP address of system interface */
    MW_IPV4_T       ip_mask;                        /* The operational IP Mask of system interface */
    MW_IPV4_T       ip_gw;                          /* The operational gateway of system interface */
    MW_IPV4_T       ip_dns;                         /* The operational DNS Server of system interface */
#ifdef AIR_SUPPORT_SECOND_NETIF
    MW_IPV4_T       if2_ip_addr;                    /* The operational IP address of second system interface */
    MW_IPV4_T       if2_ip_mask;                    /* The operational IP Mask of second system interface */
#endif
#ifdef AIR_SUPPORT_IPV6
    ip6_addr_t      ip6_addr_1;                 /* The operational dynamic IPv6 address 1 */
    ip6_addr_t      ip6_addr_2;                 /* The operational dynamic IPv6 address 1 */
    ip6_addr_t      ip6_link_local_addr;            /* The operational IPv6 link local address */
    UI8_T           dad_result;                     /* The operational DAD result from lwip */
#endif
} ATTRIBUTE_PACK DB_SYS_OPER_INFO_T;

/* The port operational information table */
typedef struct DB_PORT_OPER_INFO_S
{
    UI8_T           oper_status;        /* The operational status of the port */
    UI8_T           oper_speed;         /* The operational speed of the port */
    UI8_T           oper_duplex;        /* The operational duplex mode of the port */
    UI8_T           oper_flow_ctrl;     /* The operational flow ctrl mode of the port */
    UI8_T           loop_state;         /* The looped state of the port */
    UI8_T           oper_router;        /* The dynamic router port of IGMP snooping */
    UI8_T           mib_counter_clear;  /* The clear operation of MIB counter for the port. */
    UI8_T           oper_mode;          /* The operation port mode of the port. */
    UI8_T           lacp_state;         /* The LACP state of the port */
} ATTRIBUTE_PACK DB_PORT_OPER_INFO_T;

typedef struct DB_LAG_OPER_INFO_S
{
    UI8_T           process_state;                        /* The admin mode of cable diagnosis */
} ATTRIBUTE_PACK DB_LAG_OPER_INFO_T;

typedef struct DB_LAG_MEMBER_0_S
{
    UI8_T           member0[MAX_TRUNK_NUM];                        /* The admin mode of cable diagnosis */
} ATTRIBUTE_PACK DB_LAG_MEMBER_0_T;

#ifdef AIR_SUPPORT_CABLE_DIAG
/* The cable diagnostic table */
typedef struct DB_PORT_DIAG_S
{
    UI8_T           set[MAX_PAIR_NUM];                        /* The admin mode of cable diagnosis */
    UI16_T          length[MAX_PAIR_NUM];
    UI8_T           state[MAX_PAIR_NUM];
    UI16_T          impedance[MAX_PAIR_NUM];
    UI8_T           port[MAX_PAIR_NUM];
} ATTRIBUTE_PACK DB_PORT_DIAG_T;
#endif

/* The logon information table */
typedef struct DB_LOGON_INFO_S
{
    UI8_T           logon_fail_count;         /* The login failure count */
} ATTRIBUTE_PACK DB_LOGON_INFO_T;

/* The IGMP entries */
typedef struct DB_L2_MC_ENTRY_S
{
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    MW_MAC_T        mac_addr[MAX_L2MC_NUM];     /* The multicast MAC address */
#else
    MW_IPV4_T       ip_addr[MAX_L2MC_NUM];      /* The multicast IP address */
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    UI16_T          vid[MAX_L2MC_NUM];          /* The multicast VLAN ID */
    UI32_T          portbitmap[MAX_L2MC_NUM];   /* The port bitmap of the VLAN */
    UI8_T           update_flag[MAX_L2MC_NUM];  /* The update_flag is used to ensure that igmp snooping group entry can be updated to DB
                                                   when mac_addr/ip_addr, vid, portbitmap have not changed. */
} ATTRIBUTE_PACK DB_L2_MC_ENTRY_T;

/* System control */
typedef struct DB_SYSTEM_S
{
    UI8_T           save_running;               /* Save the running data to flash */
    UI8_T           reset;                      /* Reset the system */
    UI8_T           reset_factory;              /* Reset to factory default */
} ATTRIBUTE_PACK DB_SYSTEM_T;

#ifdef AIR_SUPPORT_ICMP_CLIENT
/* ICMP Client info */
typedef struct DB_ICMP_CLIENT_INFO_S
{
    UI8_T           host_name[MAX_HOST_NAME_SIZE];  /* Host Name */
    MW_IPV4_T       ping_ip;                            /* To Ping IP */
    UI16_T          num;                                /* Num of pings */
    UI16_T          status;                             /* Ping Status */
    UI16_T          send_packets;                       /* Num of Sent ICMP Echo Packets */
    UI16_T          recv_packets;                       /* Num of Received ICMP Reply Packets */
    UI16_T          lost_packets;                       /* Num of Lost ICMP Reply Packets */
    UI16_T          min_rtt;                            /* Minimum Round Trip Time  */
    UI16_T          max_rtt;                            /* Maximum Round Trip Time  */
    UI16_T          ave_rtt;                            /* Average Round Trip Time  */
} ATTRIBUTE_PACK DB_ICMP_CLIENT_INFO_T;
#endif /* AIR_SUPPORT_ICMP_CLIENT */

#ifdef AIR_SUPPORT_SFP
/* SFP Module info */
typedef struct DB_SFP_MODULE_INFO_S
{
    UI8_T port_num;
    UI8_T diagnostic_monitoring_type;
    I16_T temperature;
    UI16_T voltage;
    UI16_T current;
    UI16_T output_power;
    UI16_T input_power;
    UI8_T loss_of_signal;
} ATTRIBUTE_PACK DB_SFP_MODULE_INFO_T;
#endif /* AIR_SUPPORT_SFP */

#ifdef AIR_SUPPORT_VOICE_VLAN
/* The fields in the VOICE_VLAN_OPER table */
typedef struct DB_VOICE_OPER_INFO_S
{
    UI32_T          member_state;               /* The joined voice VLAN port bitmap */
} ATTRIBUTE_PACK DB_VOICE_OPER_INFO_T;
#endif

/* The MIB counters table */
typedef struct DB_MIB_CNT_S
{
    UI64_T          rx_packets;         /* The amount of packets received of the port */
#ifdef AIR_SUPPORT_SNMP
    UI64_T          rx_unicast_pkts;    /* The amount of RX unicast packets of the port */
    UI64_T          rx_multicast_pkts;  /* The amount of RX multicast packets of the port */
    UI64_T          rx_broadcast_pkts;  /* The amount of RX broadcast packets of the port */
    UI64_T          rx_discards_pkts;   /* The amount of RX discard packets of the port */
#endif
    UI64_T          rx_octets;          /* The amount of octets received of the port */
    UI64_T          rx_errors;          /* The amount of RX errors */
    UI64_T          tx_packets;         /* The amount of packets transmitted of the port */
#ifdef AIR_SUPPORT_SNMP
    UI64_T          tx_unicast_pkts;    /* The amount of TX unicast packets of the port */
    UI64_T          tx_multicast_pkts;  /* The amount of TX multicast packets of the port */
    UI64_T          tx_broadcast_pkts;  /* The amount of TX broadcast packets of the port */
    UI64_T          tx_discards_pkts;   /* The amount of TX discard packets of the port */
#endif
    UI64_T          tx_octets;          /* The amount of octets transmitted of the port */
    UI64_T          tx_errors;          /* The amount of TX errors */
} ATTRIBUTE_PACK DB_MIB_CNT_T;

/* The dynamic mac entry configuration table */
typedef struct DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_S
{
    UI8_T          action_result;                 /* The action or result */
    UI8_T          dynamic_entry_count;           /* The dynamic entry count */
} ATTRIBUTE_PACK DB_DYNAMIC_MAC_ADDRESS_ENTRY_CFG_T;

/* The dynamic mac entries table */
typedef struct DB_DYNAMIC_MAC_ADDRESS_ENTRY_S
{
    MW_MAC_T        mac_addr[MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM]; /* The dynamic MAC address entry address */
    UI16_T          vid[MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM];      /* The VID of the dynamic MAC address entry */
    UI32_T          port[MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM];     /* The port bitmap of the dynamic MAC address entry */
    UI32_T          age[MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM];      /* The aging time of the dynamic MAC address entry */
} ATTRIBUTE_PACK DB_DYNAMIC_MAC_ADDRESS_ENTRY_T;

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* The fields in the RSTP_PORT_OPER_INFO table */
typedef struct DB_RSTP_PORT_OPER_S
{
    UI8_T           state;
    UI8_T           role;
    UI8_T           block;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI32_T          oper_cost;
#endif
} ATTRIBUTE_PACK DB_STP_PORT_OPER_T;

#ifdef AIR_SUPPORT_MSTP
/* The fields in the RSTP_PORT_OPER_INFO table */
typedef struct DB_MSTP_INSTANCE_PORT_OPER_S
{
    UI8_T  state[MAX_MSTP_INSTANCE_NUM];
    UI8_T  role[MAX_MSTP_INSTANCE_NUM];
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI32_T oper_cost[MAX_MSTP_INSTANCE_NUM];
#endif
} ATTRIBUTE_PACK DB_MSTP_INSTANCE_PORT_OPER_T;
#endif
#endif /* AIR_SUPPORT_RSTP */

#ifdef AIR_SUPPORT_LLDPD
typedef struct MED_CAPABILITIES_S
{
    UI16_T             med_capability;
    UI16_T             med_device_type;
}ATTRIBUTE_PACK MED_CAPABILITIES_T;

typedef union
{
    struct _NETWORK_POLICY
    {
#ifdef __BIG_ENDIAN
        UI32_T            dscp:6;
        UI32_T            l2_pri:3;
        UI32_T            vlan_id:12;
        UI32_T            reserved:1;
        UI32_T            tagged_flag:1;
        UI32_T            unknown_policy_flag:1;
        UI32_T            app_type:8;
#else
        UI32_T            app_type:8;
        UI32_T            unknown_policy_flag:1;
        UI32_T            tagged_flag:1;
        UI32_T            reserved:1;
        UI32_T            vlan_id:12;
        UI32_T            l2_pri:3;
        UI32_T            dscp:6;
#endif
    }info;
    UI32_T        net_policy;
} NETWORK_POLICY_T;

typedef union
{
    struct _EXTEN_POW_INFO
    {
#ifdef __BIG_ENDIAN
        UI16_T             reserved:8;
        UI16_T             power_pri:4;
        UI16_T             power_source:2;
        UI16_T             power_type:2;
#else
        UI16_T             power_type:2;
        UI16_T             power_source:2;
        UI16_T             power_pri:4;
        UI16_T             reserved:8;
#endif
    }info;
    UI16_T        p_info;
} EXTEN_POW_INFO_T;

typedef struct EXTEN_POW_VIA_MDI_S
{
    EXTEN_POW_INFO_T  power_info;
    UI16_T            power_value;
}ATTRIBUTE_PACK EXTEN_POW_VIA_MDI_T;

typedef struct DB_LLDP_CLIENT_INFO_S
{
    C8_T              chassis_id[MAX_LLDP_STR_SIZE];
    C8_T              port_id[MAX_LLDP_STR_SIZE];
    UI16_T            time_to_live;
    C8_T              system_name[MAX_LLDP_STR_SIZE];
    MED_CAPABILITIES_T med_capabi;
    NETWORK_POLICY_T  med_network_policy[MAX_NETWORK_POLICY_TLV_NUM];
    EXTEN_POW_VIA_MDI_T med_exten_power;
} ATTRIBUTE_PACK DB_LLDP_CLIENT_INFO_T;
#endif

#ifdef AIR_SUPPORT_POE
typedef struct DB_POE_STATUS_S
{
    UI16_T  supply_voltage;
    UI16_T  total_consumed_power;
    UI32_T  total_consumed_current;
    UI32_T  is_poe_port;
    UI8_T   poe_power_strategy;
} ATTRIBUTE_PACK DB_POE_STATUS_T;

typedef struct DB_POE_PORT_STATUS_S
{
    UI8_T   port_power_status;
    UI8_T   port_pd_class;
    UI16_T  port_consumed_power;
    UI16_T  port_voltage;
    UI16_T  port_current;
    UI8_T   port_power_event;
} ATTRIBUTE_PACK DB_POE_PORT_STATUS_T;
#endif

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: dbapi_dbisReady
 * PURPOSE:
 *      Get the state of DB task
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      The return value will be MW_E_OK if the database is initialized,
 *      otherwise return MW_E_NOT_INITED.
 */
MW_ERROR_NO_T
dbapi_dbisReady(
    void);

/* FUNCTION NAME: dbapi_getDataSize
 * PURPOSE:
 *      Get the buffer size of the request type
 *
 * INPUT:
 *      req          -- A request type data
 *
 * OUTPUT:
 *      total_size   -- The required data size of
 *                      the request type
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getDataSize(
    DB_REQUEST_TYPE_T req,
    UI16_T *total_size);

/* FUNCTION NAME: dbapi_setMsgHeader
 * PURPOSE:
 *      Fill in the message header
 *
 * INPUT:
 *      client_qname     -- The name of the queue
 *      method           -- The method of this request message
 *      pcount           -- The requests number of this message
 *
 * OUTPUT:
 *      ptr_input        -- The pointer points to the message head
 *
 * RETURN:
 *      The header size
 *
 * NOTES:
 *
 */
UI16_T
dbapi_setMsgHeader(
    void *ptr_input,
    const C8_T *client_qname,
    const UI8_T method,
    const UI8_T pcount);


/* FUNCTION NAME: dbapi_setMsgPayload
 * PURPOSE:
 *      Fill in the data payload
 *
 * INPUT:
 *      method           -- The method of this request message
 *      in_t_idx         -- The enum of the table
 *      in_f_idx         -- The enum of the field
 *      in_e_idx         -- The entry index in the table
 *      ptr_raw_data     -- The pointer of raw data
 *
 * OUTPUT:
 *      ptr_input        -- The pointer points to the request head of the payload
 *
 * RETURN:
 *      The request total size
 *
 * NOTES:
 *
 */
UI16_T
dbapi_setMsgPayload(
    UI8_T method,
    UI8_T in_t_idx,
    UI8_T in_f_idx,
    UI16_T in_e_idx,
    void *ptr_raw_data,
    void *ptr_input);

/* FUNCTION NAME: dbapi_sendRequesttoDb
 * PURPOSE:
 *      Send a request to DB request queue
 *
 * INPUT:
 *      size            -- Size of the data in message buffer
 *      ptr_msg         -- A pointer to the item to be tranmitted
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_sendRequesttoDb(
    UI32_T size,
    DB_MSG_T *ptr_msg);

/* FUNCTION NAME: dbapi_sendMsg
 * PURPOSE:
 *      Send a request to DB request queue
 *
 * INPUT:
 *      ptr_msg         -- A pointer to the item to be tranmitted
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      This API will free the message if failed to send.
 *
 */
MW_ERROR_NO_T
dbapi_sendMsg(
    DB_MSG_T *ptr_msg,
    const UI32_T timeout);

/* FUNCTION NAME: dbapi_recvMsg
 * PURPOSE:
 *      Receive DB message from client's queue.
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      pptr_out_msg    -- The pointer of pointer points to the received message
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_recvMsg(
    const C8_T *client_qname,
    DB_MSG_T **pptr_out_msg,
    const UI32_T timeout);

/* FUNCTION NAME: dbapi_getData
 * PURPOSE:
 *      Get data from DB
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      ptr_msg         -- The pointer points to the request
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      DB is a single thread and uses a FIFO queue to manupliate internal data.
 *      The client queue may get notifications from another update if the caller
 *      has any subscriptions from DB. Which means this API is only suitable for
 *      the caller that HAS NO ANY SUBSCRIPTIONS.
 *      This API will not free the requested message. User should free it in the task.
 *
 */
MW_ERROR_NO_T
dbapi_getData(
    const C8_T *client_qname,
    DB_MSG_T *ptr_msg,
    const UI32_T timeout);

/* FUNCTION NAME: dbapi_setData
 * PURPOSE:
 *      Update data to DB
 *
 * INPUT:
 *      client_qname    -- The name of the queue
 *      ptr_msg         -- The pointer points to the request
 *      timeout         -- The waiting time of queue
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      DB is a single thread and uses a FIFO queue to manupliate internal data.
 *      The client queue may get notifications from another update if the caller
 *      has any subscriptions from DB. Which means this API is only suitable for
 *      the caller that HAS NO ANY SUBSCRIPTIONS.
 *      This API will free the message automatically. The client queue may get
 *      notifications from another updates. This API will free it directly.
 *
 */
MW_ERROR_NO_T
dbapi_setData(
    const C8_T *client_qname,
    DB_MSG_T *ptr_msg,
    const UI32_T timeout);

/* FUNCTION NAME: dbapi_showCfgFile
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
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Show the config file in console
 *
 */
MW_ERROR_NO_T
dbapi_showCfgFile(
    BOOL_T showstartup);

/* FUNCTION NAME: dbapi_saveCfgFile
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
 *      MW_E_NOT_INITED
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      Save the config
 *
 */
MW_ERROR_NO_T
dbapi_saveCfgFile(
    BOOL_T savestartup);

/* FUNCTION NAME: dbapi_getCfgFile
 * PURPOSE:
 *      Check the startup config file exist and return the size
 *
 * INPUT:
 *      ptr_file               -- The pointer of the config file
 *
 * OUTPUT:
 *      ptr_file_size          -- The size of the config file
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
dbapi_getCfgFile(
    UI8_T *ptr_file,
    UI32_T *ptr_file_size);

/* FUNCTION NAME: dbapi_dumpRaw
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
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_dumpRaw(
    UI8_T cfg_type,
    UI8_T t_idx,
    UI8_T f_idx);

#ifdef DB_SUPPORT_RESTORE_METHOD
/* FUNCTION NAME: dbapi_restoreTable
 * PURPOSE:
 *      Restore the specific table from the database
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      e_idx           -- The entry index
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_restoreTable(
    const UI8_T   t_idx,
    const UI8_T   f_idx,
    const UI16_T  e_idx);
#endif

/* FUNCTION NAME: dbapi_clrCfgFile
 * PURPOSE:
 *      Clear the config file in Flash
 *
 * INPUT:
 *      clrstartup           -- Clear the startup-config or the factory-defaults
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      Clear the config
 *
 */
MW_ERROR_NO_T
dbapi_clrCfgFile(
    BOOL_T clrstartup);

/* FUNCTION NAME: dbapi_getTableName
 * PURPOSE:
 *      Return the table name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_tbltext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getTableName(
    UI8_T t_idx,
    UI8_T data_size,
    C8_T *ptr_tbltext);

/* FUNCTION NAME: dbapi_getFieldName
 * PURPOSE:
 *      Return the Field name of the specific key
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *      data_size       -- The maximum buffer size
 *
 * OUTPUT:
 *      ptr_fldtext     -- The output name string
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldName(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T data_size,
    C8_T *ptr_fldtext);

/* FUNCTION NAME: dbapi_getFieldsNum
 * PURPOSE:
 *      Return the Fields number of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      ptr_fldnum      -- The output fields number
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldsNum(
    UI8_T t_idx,
    UI8_T *ptr_fldnum);

/* FUNCTION NAME: dbapi_getFieldSize
 * PURPOSE:
 *      Return the specific Field size of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *      f_idx           -- The field index
 *
 * OUTPUT:
 *      ptr_fldsize     -- The output field size
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldSize(
    UI8_T t_idx,
    UI8_T f_idx,
    UI8_T *ptr_fldsize);

/* FUNCTION NAME: dbapi_getEntriesNum
 * PURPOSE:
 *      Return the entries number of the specific table
 *
 * INPUT:
 *      t_idx           -- The table index
 *
 * OUTPUT:
 *      ptr_entriesnum  -- The entreies number
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getEntriesNum(
    UI8_T t_idx,
    UI16_T *ptr_entriesnum);

/* FUNCTION NAME: dbapi_getTableIdx
 * PURPOSE:
 *      Return the table index of the specific table name
 *
 * INPUT:
 *      table_name      -- The table name string
 *
 * OUTPUT:
 *      ptr_tblidx      -- The table index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getTableIdx(
    C8_T *table_name,
    UI8_T *ptr_tblidx);

/* FUNCTION NAME: dbapi_getFieldIdx
 * PURPOSE:
 *      Return the Field index of the specific field name
 *
 * INPUT:
 *      t_idx           -- The table index
 *      field_name      -- The field name
 *
 * OUTPUT:
 *      ptr_fldidx      -- The field index
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_getFieldIdx(
    UI8_T t_idx,
    C8_T *field_name,
    UI8_T *ptr_fldidx);

/* FUNCTION NAME: dbapi_dumpSubTree
 * PURPOSE:
 *      Dump the DB Current Subscriptions
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
dbapi_dumpSubTree(
    void);

/* FUNCTION NAME: dbapi_dumpTableCtrl
 * PURPOSE:
 *      Dump the DB Current control structure information
 *
 * INPUT:
 *      detail          -- Detail level of the dump
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
dbapi_dumpTableCtrl(
    UI8_T   detail);

/* FUNCTION NAME: dbapi_createMsg
 * PURPOSE:
 *      Create a message for DB and set its header. Its payload size is indicated
 *      by payload_size inputted directly.
 *
 * INPUT:
 *      ptr_queue_name  --  A pointer to the queue name
 *      method          --  The method of the message
 *      request_count   --  The total number of requests which is intended be
 *                          filled in the message payload
 *      payload_size    --  The total size of the message payload
 *
 * OUTPUT:
 *      ptr_msg_size    --  A pointer returns the total size of the message
 *      pptr_payload    --  A double pointer returns the start address of the
 *                          message payload
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If request_count is not clear when calling the function, a non-zero value
 *      can be filled in temporarily and then use dbapi_appendMsgPayload()
 *      to set the message payload. dbapi_appendMsgPayload() will update
 *      the count member of the header based on how many requests have been set
 *      into the message payload.
 */
DB_MSG_T *
dbapi_createMsg(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T request_count,
    const UI16_T payload_size,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_payload);

/* FUNCTION NAME: dbapi_createMsgByReq
 * PURPOSE:
 *      Create a message for DB and set its header. Its payload size will be
 *      calculated by the request array inputted.
 *
 * INPUT:
 *      ptr_queue_name  --  A pointer to the queue name
 *      method          --  The method of the message
 *      request_count   --  The total number of requests within the request array
 *      request[]       --  The request array which are supposed to be set in the
 *                          message payload later
 *
 * OUTPUT:
 *      ptr_msg_size    --  A pointer returns the total size of the message
 *      pptr_payload    --  A double pointer returns the start address of the
 *                          message payload
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Please be aware that the requests set after this function returns should
 *      be exactly the same as the requests inputted through the request array.
 */
DB_MSG_T *
dbapi_createMsgByReq(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    UI8_T request_count,
    DB_REQUEST_TYPE_T request[],
    UI16_T *ptr_msg_size,
    UI8_T **pptr_payload);

/* FUNCTION NAME: dbapi_setMsgPayload
 * PURPOSE:
 *      Append a request after the last request of the message payload. If the
 *      message is not large enough, the message will be enlarged by reallocating.
 *
 * INPUT:
 *      ptr_request             -- A pointer to the request to be appended
 *      ptr_data                -- A pointer to the request raw data
 *      pptr_msg                -- A double pointer to the DB message
 *      ptr_msg_size            -- A pointer to the size of the DB message
 *      pptr_shifted_payload    -- A double pointer to the position of the payload
 *                                 to store the current request. If it is the first
 *                                 request, the start address of the ptr_payload
 *                                 member of the DB message should be inputted
 *
 * OUTPUT:
 *      pptr_msg                -- A double pointer returns the DB message which
 *                                 may be reallocated
 *      ptr_msg_size            -- A pointer returns the size of the DB message
 *                                 which may be reallocated
 *      pptr_shifted_payload    -- A double pointer returns the position of the
 *                                 payload to store the next request
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If any error returns, the message should be freed by the caller.
 *      To avoid triggering the reallocation frequently, it is suggested to
 *      allocate large enough buffer for the DB message.
 */
MW_ERROR_NO_T
dbapi_appendMsgPayload(
    DB_REQUEST_TYPE_T *ptr_request,
    UI8_T *ptr_data,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload);

/* FUNCTION NAME: dbapi_parseMsg
 * PURPOSE:
 *      Parse a DB_MSG_T notification.
 *
 * INPUT:
 *      ptr_msg           -- A pointer points to a DB_MSG_T message with one or
 *                           more requests.
 *      count             -- The total number of requests within the message
 *      pptr_payload_data -- A double pointer points to the request to be parsed
 *                           if it is the subsequent parsing for ptr_msg. If it is
 *                           the first parsing for ptr_msg, its value is ignored.
 *
 * OUTPUT:
 *      ptr_request       -- A pointer returns the request parsed
 *      ptr_data_size     -- A pointer returns the size of the raw data for the
 *                           request
 *      pptr_data         -- A double pointer returns the raw data of the request
 *      pptr_payload_data -- A double pointer returns the next request to be
 *                           parsed if there is any
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. (*pptr_data) points to the data within the buffer pointed by ptr_msg.
 *      2. DB_MSG_T.type.count may be changed when the function returns. As a
 *         result, the message can only be parsed once.
 */
MW_ERROR_NO_T
dbapi_parseMsg(
    DB_MSG_T *ptr_msg,
    UI8_T count,
    DB_REQUEST_TYPE_T *ptr_request,
    UI16_T *ptr_data_size,
    UI8_T **pptr_data,
    UI8_T **pptr_payload_data);

/* FUNCTION NAME: dbapi_getFactoryDefault
 * PURPOSE:
 *      Get the factory default value for specific request.
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
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
dbapi_getFactoryDefault(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data);

/* FUNCTION NAME: dbapi_getStartUp
 * PURPOSE:
 *      Get the startup configuration value for specific request.
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
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      1. (*pptr_data) points to the address of the allocated buffer, need to free it after use.
 */
MW_ERROR_NO_T
dbapi_getStartUp(
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    UI16_T *ptr_data_size,
    void **pptr_data);

/* FUNCTION NAME: dbapi_getDataFromRawTable
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
dbapi_getDataFromRawTable(
    UI8_T *ptr_out_data,
    const UI8_T *ptr_raw_data,
    const DB_REQUEST_TYPE_T req);

/* FUNCTION NAME: dbapi_isStartUpConfigExist
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
dbapi_isStartUpConfigExist(
    void);

#endif  /* End of DBAPI_H */
