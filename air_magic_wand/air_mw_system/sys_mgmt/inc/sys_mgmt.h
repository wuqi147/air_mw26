/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  sys_mgmt.h
 * PURPOSE:
 * It provides SYS_MGMT module API and definitions.
 *
 * NOTES:
 */

#ifndef SYS_MGMT_H
#define SYS_MGMT_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_init.h"
#include "lwip/ip.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/ip6_addr.h"
#endif
#if LWIP_NETIF_EXT_STATUS_CALLBACK
#include "lwip/netif.h"
#endif
#if MW_DHCP
#include "lwip/dhcp.h"
#endif
#include "lwip/dns.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "osapi_message.h"
#include "osapi.h"
#include "inet_utils.h"
#include "db_api.h"
#include "osapi_string.h"
#include "osapi_thread.h"
#include "air_acl.h"
#include "air_port.h"
#include "air_dos.h"
#include <mw_utils.h>
#include "mw_portbmp.h"
#include "ethernetif.h"
#ifdef AIR_SUPPORT_ICMP_CLIENT
#include "icmp_client.h"
#endif /* AIR_SUPPORT_ICMP_CLIENT */
#ifdef AIR_SUPPORT_SNTP
#include "sntp.h"
#endif
#include "mw_acl_settings.h"
#ifdef AIR_SUPPORT_LACP
#include "mw_lacp.h"
#endif
#include    "mw_dos.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Uncomment the macro below to allow the debug logs. */
#define SYS_MGMT_TASK_NAME                      "SYS_MGMT"
#define SYS_MGMT_MODULE_NAME                    "sys_mgmt"
#define SYS_MGMT_DB_QUEUE_NAME                  "sym"
#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
#define SYS_MGMT_POLL_QUEUE_TIMEOUT             (200)
#endif
#ifdef AIR_SUPPORT_LACP
#define SYS_MGMT_QUEUE_LENGTH                   (AIR_MAX_SYS_MGMT_QUEUE_LEN + AIR_MAX_LACPDU_QUEUE_LEN + LACP_TIMER_QUEUE_LEN)
#else
#define SYS_MGMT_QUEUE_LENGTH                   AIR_MAX_SYS_MGMT_QUEUE_LEN
#endif
#define SYS_MGMT_DB_BLOCK_QUEUE_NAME            "syd"
#define SYS_MGMT_DB_BLOCK_QUEUE_LENGTH          (1)
#define SYS_MGMT_DB_BLOCK_QUEUE_SIZE            (4)

#define SYS_MGMT_TMR_INTERVAL                   10000 /* 10000 Milliseconds */
#define SYS_MGMT_IPV4_STR_SIZE        (16)
#define SYS_MGMT_TIMER_NAME                     "SYS_MGMT_TIMER"
#define SYS_MGMT_TIMER_PERIOD                   (128000)

#define SYS_MGMT_VLAN_ID                        (_sys_mgmt_vlan_id)
#define SYS_MGMT_VLAN_PBMP                      (_sys_mgmt_vlan_port_bmp_total)

#ifdef AIR_SUPPORT_REMOTE_DEBUG
#define SYS_MGMT_TASK_DELAY                     (250)
#else
#if defined(AIR_SUPPORT_SFP) && !defined(AIR_SUPPORT_SFP_WITH_THREAD)
#define SYS_MGMT_TASK_DELAY                     (200)
#else
#define SYS_MGMT_TASK_DELAY                     (MSG_TIMEOUT_WAIT_INDEFINITELY)
#endif
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* The system information table */
typedef struct SYS_MGMT_S
{
    UI8_T           dhcp_enable;                /* Enable DHCP for system interface */
    UI8_T           autodns_enable;             /* Enable AutoDNS for system interface */
    ip4_addr_t       static_ip;                  /* Use static IPv4 address for system interface */
    ip4_addr_t       static_mask;                /* Use static IPv4 mask for system interface */
    ip4_addr_t       static_gw;                  /* Use static gateway for system interface */
    ip4_addr_t       static_dns;                 /* Use static dns for system interface */
    MW_IPV4_T        oper_ip;                    /* The operational IP address of system interface */
    MW_IPV4_T        oper_mask;                  /* The operational IP Mask of system interface */
    MW_IPV4_T        oper_gw;                    /* The operational gateway of system interface */
    MW_IPV4_T        oper_dns;                   /* The operational dns of system interface */
    ip4_addr_t       def_ip;                     /* The default IP address of system interface */
    ip4_addr_t       def_mask;                   /* The default IP Mask of system interface */
    ip4_addr_t       def_gw;                     /* The default gateway of system interface */
    ip4_addr_t       def_dns;                    /* The default dns of system interface */
    ip4_addr_t       temp_dns;                   /* The cached DNS server obtained by DHCP of system interface */
#ifdef AIR_SUPPORT_SECOND_NETIF
    ip4_addr_t       static_if2_ip;              /* Use static IPv4 address for second system interface */
    ip4_addr_t       static_if2_mask;            /* Use static IPv4 mask for second system interface */
    MW_IPV4_T        oper_if2_ip;                /* The operational IP address of second system interface */
    MW_IPV4_T        oper_if2_mask;              /* The operational IP Mask of second system interface */
    ip4_addr_t       def_if2_ip;                 /* The default IP address of system interface */
    ip4_addr_t       def_if2_mask;               /* The default IP Mask of system interface */
#endif
} ATTRIBUTE_PACK SYS_MGMT_T;

typedef struct SYS_MGMT_TASK_S
{
    threadhandle_t    task_handle;
    StaticTask_t      task_tcb;
    StackType_t       task_stack[AIR_MAX_SYSMGMT_STACK_SIZE];
#ifdef AIR_SUPPORT_LACP
    NET_FILTER_T      pkt_netf;
#endif
    staticMsghandle_t queue_handle;                        /* Task queue is a static queue. */
    void              *queue_pool[SYS_MGMT_QUEUE_LENGTH];  /* Task queue is a static queue. */
    staticMsghandle_t db_queue_handle;                        /* Task queue is a static queue. */
    void              *db_queue_pool[SYS_MGMT_DB_BLOCK_QUEUE_LENGTH];  /* Task queue is a static queue. */
} SYS_MGMT_TASK_T;

typedef struct SYS_MGMT_VLAN_INFO_S
{
    UI8_T                        vlan_mode;
    UI16_T                       vlan_id;
    AIR_PORT_BITMAP_T            vlan_pbmp_total;
    AIR_PORT_BITMAP_T            vlan_pbmp_total_untag;
} SYS_MGMT_VLAN_INFO_T;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
MW_ERROR_NO_T sys_mgmt_init(void);
void sys_mgmt_dump();
void sys_mgmt_dhcp_enable_cmd_set(UI8_T enable);
#ifdef AIR_SUPPORT_MQTTD
void sys_mgmt_mqttd_enable_cmd_set(UI8_T enable, void *server_ip);
#endif

/* FUNCTION NAME: sys_mgmt_get_uni_acl_ap_id
 * PURPOSE:
 *      Get the unicast ACL Attack Prevention ID.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The unicast ACL Attack Prevention ID.
 *
 * NOTES:
 *      None
 */
UI32_T
sys_mgmt_get_uni_acl_ap_id(
    void);

/* FUNCTION NAME: sys_mgmt_set_uni_acl_ap_id
 * PURPOSE:
 *      Set the unicast ACL Attack Prevention ID.
 *
 * INPUT:
 *      id          --  The unicast ACL Attack Prevention ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
sys_mgmt_set_uni_acl_ap_id(
    UI32_T id);

/* FUNCTION NAME:   sys_mgmt_get_mgmt_vid_pbmp
 * PURPOSE:
 *      Management VLAN id and pbmp get function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_vlan_mode           : vlan mode
 *      ptr_vid                 : management VLAN id
 *      pbmp_total              : management VLAN pbmp for tag packet
 *      ptr_pbmp_total_untag    : management VLAN pbmp for untag packet
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */

MW_ERROR_NO_T
sys_mgmt_get_mgmt_vid_pbmp(
    UI8_T *ptr_vlan_mode,
    UI16_T *ptr_vid,
    AIR_PORT_BITMAP_T *ptr_pbmp_total,
    AIR_PORT_BITMAP_T *ptr_pbmp_total_untag);

#ifdef AIR_SUPPORT_SNTP
extern portBASE_TYPE sntp_on;
extern int g_local_timezone;
#endif


/* FUNCTION NAME: sys_mgmt_language_init()
 * PURPOSE:
 *      Initialize system language.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_language_init(void);

/* FUNCTION NAME: sys_mgmt_queue_recv
 * PURPOSE:
 *      Receive data from specific message queue which depending on name
 *
 * INPUT:
 *      ptr_name        --  A pointer to the queue name
 *      timeout         --  The maximun amout of time the thread
 *                          will wait for send (unit: millisecond)
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer to the message buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sys_mgmt_queue_recv(
    const C8_T *ptr_name,
    UI8_T **pptr_msg,
    const UI32_T timeout);

/* FUNCTION NAME: sys_mgmt_queue_getData
 * PURPOSE:
 *      Get the raw data based on t_idx, f_idx and e_idx from DB. It is a
 *      blocking API.
 *
 * INPUT:
 *      method          --  The method of the getting request to DB
 *      t_idx           --  The table index
 *      f_idx           --  The field index
 *      e_idx           --  The entry index
 *
 * OUTPUT:
 *      pptr_msg        --  A double pointer returns a db message received from DB
 *      ptr_data_size   --  A pointer returns the size of raw data obtained from DB
 *      pptr_data       --  A double pointer returns the raw data obtained from DB
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      1. If MW_E_OK is returned, the db message returned by pptr_msg need be
 *      freed after use.
 *      2. dbapi_parseMsg() cannot be used for the message returned by
 *         pptr_msg.
 */
MW_ERROR_NO_T
sys_mgmt_queue_getData(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_data_size,
    void **pptr_data);

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
/* FUNCTION NAME:   sys_mgmt_set_mgmt_vlan_id
 * PURPOSE:
 *      This API is used to set MGMT VLAN ID.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void sys_mgmt_set_mgmt_vlan_id(UI16_T vlan_id);
#endif /* AIR_SUPPORT_MGMT_VLAN_CFG */

/* FUNCTION NAME:   sys_mgmt_get_dhcp_enable_settings
 * PURPOSE:
 *      This API is used to return sys_mgmt dhcp mode.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      BOOL_T
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
BOOL_T
sys_mgmt_get_dhcp_enable_settings();

/* FUNCTION NAME:   sys_mgmt_convert_port_mac
 * PURPOSE:
 *      This API is used to convert mac address to port mac(mac + port ID).
 *
 * INPUT:
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_mac         --  Pointer to the mac address
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
sys_mgmt_convert_port_mac(
    AIR_MAC_T mac,
    UI32_T port);

/* FUNCTION NAME:   sys_mgmt_autodns_enable_cmd_set
 * PURPOSE:
 *      This API is used for mw cmd to enable/disable SYS MGMT Auto DNS admin mode.
 *
 * INPUT:
 *      enable       --  Auto DNS mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
sys_mgmt_autodns_enable_cmd_set(
    UI8_T enable);

/* FUNCTION NAME: sys_mgmt_queue_send
 * PURPOSE:
 *      package message and call sending function to DB.
 *
 * INPUT:
 *      method      --  the method bitmap
 *      t_idx       --  the enum of the table
 *      f_idx       --  the enum of the field
 *      e_idx       --  the entry index in the table
 *      ptr_data    --  pointer to message data
 *      size        --  size of ptr_data
 *      ptr_name    --  name of module
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
 *      The input parameters are depend on structure of DB.
 *      Please refer to db_api.h
 */
MW_ERROR_NO_T
sys_mgmt_queue_send(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size);

#endif /* End of SYS_MGMT_H */
