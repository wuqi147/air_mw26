/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

/* FILE NAME:  switch.h
 * PURPOSE:
 * Provide definitions for the hardware functions of the switch
 *
 * NOTES:
 */

#ifndef __SWITCH_H__
#define __SWITCH_H__
/* INCLUDE FILE DECLARATIONS
 */
#include "FreeRTOS.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* System Description */
#define AIR_DEFAULT_SYSTEM_NAME     "EN8851"

/* System Port Info */
#define AIR_MAX_PORT_NUM            (28)
#define AIR_MAX_PORT_BITMAP         (0x1FFFFFFE)

/* System Trunk Group Info */
#define AIR_MAX_TRUNK_GROUP         (4)

/* System Trunk Group Member Info */
#define AIR_MAX_TRUNK_MEMBER_NUM    (8)

/* System Mirror Session info */
#define AIR_MAX_MIRROR_SESSION      (4)

/* Task priorities */
#define MW_TASK_PRIORITY_ERPS           (rtosMAX_PRIORITIES)
#define MW_TASK_PRIORITY_IFMON          (configLWIP_TASK_PRIORITY)
#define MW_TASK_PRIORITY_MAC_INIT       (configLWIP_TASK_PRIORITY)
#define MW_TASK_PRIORITY_DB             (configLWIP_TASK_PRIORITY - 2)
#define MW_TASK_PRIORITY_SYNCD          (4)
#define MW_TASK_PRIORITY_LP             (3)
#define MW_TASK_PRIORITY_STP_NORMAL     (3)
#define MW_TASK_PRIORITY_STP_HIGH       (configLWIP_TASK_PRIORITY - 1)
#define MW_TASK_PRIORITY_LLDP           (2)
#define MW_TASK_PRIORITY_IGMPSNP        (2)
#define MW_TASK_PRIORITY_DHCPSNP        (2)
#define MW_TASK_PRIORITY_SYSMGMT        (2)
#define MW_TASK_PRIORITY_VOICE_VLAN     (2)
#define MW_TASK_PRIORITY_SFP            (1)
#define MW_TASK_PRIORITY_SFP_LED        (1)

/* DB Information */
#define AIR_MAX_DB_REQUEST_LEN      (128)
#define AIR_MAX_DB_STACK_SIZE       (404)

/* MQTT Information */
#define AIR_MAX_MQTT_QUEUE_LEN      (28)
#define AIR_MAX_MQTT_QUEUE_TIMEOUT  (100)
#define AIR_MAX_MQTT_STACK_SIZE     (600)
#define AIR_MAX_MQTT_LOCK_TIME      (50)
#define AIR_MAX_MQTT_KEEP_ALIVE     (5)

/* System Management Information */
#define AIR_MAX_SYS_MGMT_QUEUE_LEN            (32)
#define AIR_MAX_SYS_MGMT_DHCP_RATE_PER_SEC    (40)

/* DHCP Snooping Information */
#define AIR_MAX_SNP_PKT_QUEUE_LEN        (64)
#define AIR_MAX_SNP_DB_QUEUE_LEN         (32)
#define AIR_MAX_SNP_DHCP_RATE_PER_SEC    (40)
#define DHCP_SNP_TASK_STACK_SIZE         (256)

/* ARP Information */
#define MW_ARP_PACKET_THRESHOLD (40)  /* ARP rate limit */

/* Ping Information*/
#define AIR_MAX_PING_QUEUE_LEN      (32)

/* IGMP Snooping Information */
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
#define AIR_IGMP_RATE_LIMIT_THLD    (40)
#else
#define AIR_IGMP_RATE_LIMIT_THLD    (80)
#endif
#define AIR_IGMP_MAX_L2MC_ENTRY_NUM (64)
#define AIR_MAX_IGMP_STACK_SIZE     (512)
#define AIR_MAX_IGMP_DB_QUEUE_LEN   (64)
#define AIR_MAX_IGMP_PKT_QUEUE_LEN  (PBUF_POOL_SIZE)

/* LLDP Information */
#define LLDP_QUEUE_LENGTH          (128+30+10)
#define AIR_SUPPORT_LLDPD_TX_SEL

/* SSH Information */
#define SSH_TASK_NAME            "ssh"
#define SSH_TASK_PRIORITY        5
#define SSH_STACK_SIZE           1125
#define SSH_QUEUE_NAME           "shq"
#ifdef AIR_SUPPORT_REMOTE_DEBUG
#ifdef AIR_SUPPORT_CLI
#define SSH_CLI_MSG_MAX_LEN      6
#define SSH_QUEUE_LEN            (REMOTE_DEBUG_LOG_MAX_ENTRY_NUM + SSH_CLI_MSG_MAX_LEN)
#else
#define SSH_QUEUE_LEN            (REMOTE_DEBUG_LOG_MAX_ENTRY_NUM)
#endif
#else
#define SSH_QUEUE_LEN            (10)
#endif
#define SSH_MAX_CLIENT_NUM       1
#define SSH_QUEUE_SEND_WAIT_TIME  100

/* Loop Prevention Information */
#define AIR_MAX_LP_DB_QUEUE_LEN     (64)
#define AIR_MAX_LP_PKT_QUEUE_LEN    (PBUF_POOL_SIZE)

/* STP Information */
#define STP_QUEUE_LENGTH_PTK       (96)
#define STP_QUEUE_LENGTH_DB        (80)
#define AIR_SUPPORT_STP_AUTO_COST
#define AIR_SUPPORT_STP_UNIFIED_SA
#define AIR_SUPPORT_STP_CLEAR_MULTICAST
#define MSTP_INSTANCE_MAX_NUM       (4)

/* Syncd Information*/
#define AIR_MAX_SYNCD_STACK_SIZE    (1024)
#define AIR_MAX_SYNCD_TMR_PS        (500)

/* System Managemnt Information */
#define AIR_MAX_SYSMGMT_STACK_SIZE    (512)

/* SFP Information */
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
#define SFP_TASK_STACK_SIZE (512)
#define SFP_MSG_QUEUE_LENGTH (10)
#endif
#define SFP_MODULE_INFO_MAX_NUM (4)

/* Firmware Update Web Page Info */
#define AIR_WEBPAGE_UPGRADE_TIME    (195)

/* Definition for ACL ENTRY ID */
#define MW_MAX_NUM_OF_ACL_ENTRY             (AIR_MAX_NUM_OF_ACL_ENTRY)

/* Definition for static ACL IDs. */
#define MW_ACL_ID_STATIC_MAX                (MW_ACL_ID_STATIC_LAST - 1)

/* MAC management */
#define AIR_MAX_PER_WEBPAGE_DYNAMIC_MAC_ADDRESS_ENTRY_NUM       (64)
#define AIR_MAX_PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM     (16)

/* Definition for dynamic ACL ID.
 * It is suggested to find an available ID in the dynamic ACL ID range,
 * [MW_ACL_ID_DYNAMIC_MIN, MW_ACL_ID_DYNAMIC_MAX], when the ACL ID does
 * not depend on any other ACL ID.
 */
#define MW_ACL_ID_DYNAMIC_MIN               (MW_ACL_ID_STATIC_MAX + 1)
#define MW_ACL_ID_DYNAMIC_MAX               (AIR_MAX_NUM_OF_ACL_ENTRY - 1)

/* Definition for dynamic Attack Rate ID.
 * It is suggested to find an available ID in the dynamic Attack Rate,
 * ID range [MW_ATTACK_ID_DYNAMIC_MIN, MW_ATTACK_ID_DYNAMIC_MAX], when
 * the Attack Rate ID does not depend on any other ACL ID.
 */
#define MW_ATTACK_ID_DYNAMIC_MIN               (0)
#define MW_ATTACK_ID_DYNAMIC_MAX               (AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID - 1)

/* Definition for PoE config.
 */
#define POE_MAX_PWR                               (2400)
#define POE_MAX_POE_DELAY_TIME                    (10)
#define POE_MON_POLLING_INTERVAL_MS               (300)           /* 300 ms */
#define POE_WATCHDOG_MIB_CNT_POLLING_INTERVAL     (600)           /* 600 ms */
#define POE_WATCHDOG_PERIODIC_UPDATE_INTERVAL     (5000)          /* 600 ms */
#define POE_MAX_LED_SOLID_INTERVAL                (3000)          /* 3000 ms */
#define POE_MAX_LED_SOLID_COUNT                   (POE_MAX_LED_SOLID_INTERVAL / POE_MON_POLLING_INTERVAL_MS)


/* MACRO FUNCTION DECLARATIONS
 */
#define MW_ACL_GET_AVAILABLERULE(id, rule, unit)         mw_acl_getAvailableRule((id), (rule), (unit), MW_ACL_ID_DYNAMIC_MIN)
#define MW_ACL_GET_AVAILABLERULE_NEXT(id, rule, unit)    mw_acl_getAvailableRule((id), (rule), (unit), ((*(id)) + 1))

#define MW_ATTACK_ID_GET_AVAILABLERULE(id, rule, unit)  mw_acl_getAvailableAttackID((id), (rule), (unit), MW_ATTACK_ID_DYNAMIC_MIN)

/* DATA TYPE DECLARATIONS
 */
/* Enum for static ACL IDs.
 * It is suggested to define ACL IDs that depend on each other as the static ACL IDs.
 */
typedef enum
{
    MW_ACL_ID_STATIC_MIN = 0,

    MW_ACL_ID_STATIC_UNICAST_ARP_TAGGED = MW_ACL_ID_STATIC_MIN,
    MW_ACL_ID_STATIC_UNICAST_ARP_PRIORITY_TAGGED,
    MW_ACL_ID_STATIC_UNICAST_ARP_LOW,
    MW_ACL_ID_STATIC_UNICAST_ARP,

    MW_ACL_ID_STATIC_BCAST_ARP_TAGGED,
    MW_ACL_ID_STATIC_BCAST_ARP_PRIORITY_TAGGED,
    MW_ACL_ID_STATIC_BCAST_ARP_LOW,
    MW_ACL_ID_STATIC_BCAST_ARP,

#ifdef AIR_SUPPORT_IPV6
    MW_ACL_ID_STATIC_ICMPV6_ND_TAG,
    MW_ACL_ID_STATIC_ICMPV6_ND_PRIORITY_TAG,
    MW_ACL_ID_STATIC_ICMPV6_ND_UNTAG_LOW,
    MW_ACL_ID_STATIC_ICMPV6_ND_UNTAG,
#endif

#ifdef AIR_SUPPORT_IPV6
    MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_TAG,
    MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_PRIORITY_TAG,
    MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_UNTAG_LOW,
    MW_ACL_ID_STATIC_IPV6_ALL_NODE_ADDESS_UNTAG,

    MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_TAG,
    MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_PRIORITY_TAG,
    MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_UNTAG_LOW,
    MW_ACL_ID_STATIC_IPV6_SOLICITED_NODE_ADDESS_UNTAG,
#endif

    MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_TAG,
    MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_PRIORITY_TAG,
    MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_UNTAG_LOW, /* UNTAG_LOW and UNAG should be in the same 1-sub-block */
    MW_ACL_ID_STATIC_SYSTEM_MAC_ADDRESS_UNTAG,

    MW_ACL_ID_STATIC_DHCP_SNP_REQ,  /* priority : SNP_REQ > SNP_ACK > ACK */
    MW_ACL_ID_STATIC_DHCP_SNP_ACK,

    MW_ACL_ID_STATIC_DHCP_ACK_TAGGED,
    MW_ACL_ID_STATIC_DHCP_ACK_PRIORITY_TAGGED,
    MW_ACL_ID_STATIC_DHCP_ACK_LOW,
    MW_ACL_ID_STATIC_DHCP_ACK,

    MW_ACL_ID_STATIC_LAST
} MW_ACL_ID_STATIC_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* Check config */
#if (MW_TASK_PRIORITY_DB) <= (MW_TASK_PRIORITY_SYNCD)
#error "MW_TASK_PRIORITY_DB should be larger than MW_TASK_PRIORITY_SYNCD"
#endif

#define GDMPSRAM_DATA_EX
#define GDMPSRAM_BSS_EX

#endif /* End of __SWITCH_H__ */
