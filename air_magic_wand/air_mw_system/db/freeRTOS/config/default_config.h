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

/* FILE NAME:  default_config.h
 * PURPOSE:
 *      1.Definition database structure.
 *      2.Definition group list.
 *
 * NOTES:
 */
#ifndef DEFAULT_CONFIG_H
#define DEFAULT_CONFIG_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_types.h"
#include "mw_portbmp.h"
#include "switch.h"
#ifdef AIR_SUPPORT_IPV6
#include "lwip/ip6_addr.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define ATTRIBUTE_PACK __attribute__((packed))

/* The Abitlities */
#define MAX_PORT_NUM         AIR_MAX_PORT_NUM       /* Maximum physical ports */
#define MAX_TRUNK_NUM        AIR_MAX_TRUNK_GROUP    /* Maximum trunk ports */
#define MAX_MIRROR_SESS_NUM  AIR_MAX_MIRROR_SESSION /* Maximum port-mirror sessions */
#define MAX_VLAN_ENTRY_NUM   (32)  /* Maximum VLAN entries */
#define MAX_OUI_NUM          (16)  /* Maximum OUI entries */
#define MAX_STATIC_MAC_NUM   (32)  /* Maximum Static MAC entries */
#define MAX_DYNAMIC_MAC_ADDRESS_ENTRY_NUM  (AIR_MAX_PER_REQUESTED_DYNAMIC_MAC_ADDRESS_ENTRY_NUM)  /* Maximum Dynamic MAC entries stored in DB */
#define MAX_QUEUE_NUM        (8)   /* Maximum Queue number*/
#define MAX_DSCP_NUM         (64)  /* Maximum DSCP mapping to queues */
#define MAX_SYS_NAME_SIZE    (32 + 1)  /* Maximum system name size, The string length is 32 with null terminate*/
#define MAX_HOST_NAME_SIZE   (32 + 1)  /* Maximum host name size, The string length is 32 with null terminate*/
#define MAX_VERSION_SIZE     (64)  /* Maximum FW/HW version size */
#define MAX_USER_NAME_SIZE   (32)  /* Maximum account username size */
#define MAX_PASSWORD_SIZE    (32)  /* Maximum account password size */
#define MAX_PASSWORD_HASH_SIZE (32) /* Maximum account password hash(SHA256) size */
#ifdef AIR_SUPPORT_DHCP_SNOOP
#define MAX_OPT82_ID_LEN     (64 + 1)  /* Maximum DHCP suboption length of option 82, The string length is 64 with null terminate*/
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#ifdef AIR_SUPPORT_SNMP
#define MAX_SNMP_CM_LEN      (21)   /* Maximum SNMP community length */
#endif
#define OUI_DESCR_SIZE      (10 + 1) /*The OUI entries for voice vlan, The string length is 10 with null terminate*/
#define VLAN_DESCR_SIZE     (12) /*The description length for 802.1Q vlan, The string length is 10 with null terminate*/
#define VOVLAN_OUI_SIZE     (3)
#ifdef AIR_SUPPORT_ERPS
#define MAX_ERPS_DATA_VLAN_SIZE    (17)
#define MAX_ERPS_INSTANCE_NUM      (1)
#endif
#ifdef AIR_SUPPORT_MSTP
#define MAX_MSTP_INSTANCE_NUM           (MSTP_INSTANCE_MAX_NUM)
#define MAX_MSTP_REGION_NAME_SIZE       (33)
#else
#define MAX_MSTP_INSTANCE_NUM           (1)
#endif

/**/
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/*****************************************
 *  SYS_INFO                             *
 *****************************************/
/* The fields in the SYS_INFO table */
typedef enum
{
    SYS_NAME = 1,          /* System Name */
    SYS_SW_VERSION,        /* System FW version */
    SYS_DHCP_ENABLE,       /* Enable DHCP for system interface */
    SYS_AUTODNS_ENABLE,    /* Enable AutoDNS for system interface */
    SYS_STATIC_IP_ADDR,    /* Use static IPv4 address for system interface */
    SYS_STATIC_IP_MASK,    /* The static IPv4 mask for system interface */
    SYS_STATIC_IP_GW,      /* The static gateway for system interface */
    SYS_STATIC_IP_DNS,     /* The static DNS for system interface */
#ifdef AIR_SUPPORT_SECOND_NETIF
    SYS_STATIC_IP2_ADDR,   /* Use static IPv4 address for second system interface */
    SYS_STATIC_IP2_MASK,   /* The static IPv4 mask for second system interface */
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    SYS_MGMT_VLAN,         /* Management VLAN */
#endif
#ifdef AIR_SUPPORT_IPV6
    SYS_MANUAL_IP6_PREFIX_LEN,  /* The manual IPv6 prefix length for system interface */
    SYS_MANUAL_IP6_ADDR,        /* The manual IPv6 address for system interface */
    SYS_MANUAL_IP6_DEFAULT_GW,  /* The manual IPv6 default gateway for system interface */
    SYS_IP6_CONFIG_MODE,   /* The mode of IPv6 configuration */
#endif
    SYS_INFO_LAST
} SYS_INFO_T;

/* The system information table */
typedef struct DB_SYS_INFO_S
{
    UI8_T           sys_name[MAX_SYS_NAME_SIZE]; /* System Name */
    UI8_T           sw_version[MAX_VERSION_SIZE];/* System FW version */
#define MW_DHCP_DISABLE     (0)
#define MW_DHCP_ENABLE      (1UL << 0)
#define MW_DHCP_WEB_ENABLE  (1UL << 1)
#define MW_DHCP_DONE        (1UL << 2)
    UI8_T           dhcp_enable;                 /* Enable DHCP for system interface */
#define MW_AUTODNS_DISABLE  (0)
#define MW_AUTODNS_ENABLE   (1)
    UI8_T           autodns_enable;              /* Enable AutoDNS for system interface */
    MW_IPV4_T       static_ip;                   /* Use static IPv4 address for system interface */
    MW_IPV4_T       static_mask;                 /* Use static IPv4 mask for system interface */
    MW_IPV4_T       static_gw;                   /* Use static gateway for system interface */
    MW_IPV4_T       static_dns;                  /* Use static dns for system interface */
#ifdef AIR_SUPPORT_SECOND_NETIF
    MW_IPV4_T       static_if2_ip;               /* Use static IPv4 address for second system interface */
    MW_IPV4_T       static_if2_mask;             /* Use static IPv4 mask for second system interface */
#endif
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    UI16_T          mgmt_vlan;                   /* Management VLAN */
#endif
#ifdef AIR_SUPPORT_IPV6
    UI8_T           manual_ip6_prefix_len;      /* Use manual IPv6 prefix length for system interface */
    ip6_addr_t      manual_ip6_addr;            /* Use manual IPv6 address for system interface */
    ip6_addr_t      manual_ip6_default_gw;      /* Use manual IPv6 default gateway for system interface */
    UI8_T           config_mode;                /* 1 for manual config, 0 for auto config */
#endif
} ATTRIBUTE_PACK DB_SYS_INFO_T;

/*****************************************
 *  ACCOUNT_INFO                         *
 *****************************************/
/* The fields in the ACCOUNT_INFO table */
typedef enum
{
    ACC_USERNAME = 1,      /* Administrator account username */
    ACC_PASSWD,            /* Administrator account password */
    ACC_INFO_LAST
} ACCOUNT_INFO_T;

/* The user account information table */
typedef struct DB_ACCOUNT_INFO_S
{
    UI8_T           username[MAX_USER_NAME_SIZE]; /* Administrator account username */
    UI8_T           passwd[MAX_PASSWORD_SIZE];    /* Administrator account password */
} ATTRIBUTE_PACK DB_ACCOUNT_INFO_T;

/*****************************************
 *  PORT_CFG_INFO                        *
 *****************************************/
/* The fields in the PORT_CFG_INFO table */
typedef enum
{
    PORT_TYPE = 1,         /* The type of the port */
    PORT_ADMIN_STATUS,     /* The admin state of the port */
    PORT_SETTINGS,         /* The port settings such as port mode, rate, duplx and flow control */
    PORT_ISOLATION,        /* The isolation group that port joined */
    PORT_INGRESS_RATE,     /* The ingress rate limit of the port */
    PORT_EGRESS_RATE,      /* The egress rate limit of the port */
    PORT_GREEN_ETHERNET,   /* The green ethernet mode of the port */
    PORT_EEE_ENABLE,       /* The EEE mode of the port */
    PORT_PVID,             /* The native vlan of the port */
    PORT_TRUNK_ID,         /* The trunk port ID that port joined */
    PORT_MIRROR_ID,        /* The mirror session ID that port joined */
    PORT_VLAN_LIST,        /* The list of the vlan that the port be a member */
    PORT_MAC_LIMIT,        /* The maximum MAC number of the port */
    PORT_STORM_BC_RATE,    /* The broadcast traffic rate limit */
    PORT_STORM_MC_RATE,    /* The multicast traffic rate limit */
    PORT_STORM_UC_RATE,    /* The unicast traffic rate limit */
    PORT_STORM_BC_MODE,    /* The broadcast traffic rate mode*/
    PORT_STORM_MC_MODE,    /* The multicast traffic rate mode */
    PORT_STORM_UC_MODE,    /* The unicast traffic rate mode */
    PORT_STORM_BC_CFG,     /* The broadcast traffic rate cfg */
    PORT_STORM_MC_CFG,     /* The unicast traffic rate cfg */
    PORT_STORM_UC_CFG,     /* The multicast traffic rate cfg */
    PORT_IGMP_ROUTER,      /* The static router port of IGMP snooping */
    PORT_VLAN_IG_FILTER,   /* The port-based VLAN mechanism for 802.1Q VLAN */
    PORT_CFG_INFO_LAST
} PORT_CFG_INFO_T;

/* The port configuration information table */
typedef struct DB_PORT_CFG_INFO_S
{
    UI8_T           type;             /* The type of the port */
    UI8_T           admin_status;     /* The admin state of the port */
    UI16_T          port_settings;    /* The port settings: bit15 port mode(0: AN; 1 Force); bit14 flow contrl(0: off, 1 on);
                                         other bits PORT_SETTINGS_FLAGS_X in port_uilts.h */
    UI32_T          isolation;        /* The isolation group that port joined */
    UI32_T          ingress_rate;     /* The ingress rate limit of the port */
    UI32_T          egress_rate;      /* The egress rate limit of the port */
    UI8_T           green_ethernet;   /* The green ethernet mode of the port */
    UI8_T           eee_enable;       /* The EEE mode of the port */
    UI16_T          pvid;             /* The native vlan of the port */
    UI8_T           trunk_id;         /* The trunk port ID that port joined */
    UI8_T           mirror_id;        /* The mirror session ID that port joined */
    UI32_T          vlan_list;        /* The list of the vlan that the port be a member */
    UI16_T          mac_limit;        /* The maximum MAC number of the port */
    UI32_T          storm_bc_rate;    /* The broadcast traffic rate limit */
    UI32_T          storm_mc_rate;    /* The multicast traffic rate limit */
    UI32_T          storm_uc_rate;    /* The unicast traffic rate limit */
    UI8_T           storm_bc_mode;    /* The broadcast traffic rate mode */
    UI8_T           storm_mc_mode;    /* The multicast traffic rate mode */
    UI8_T           storm_uc_mode;    /* The unicast traffic rate mode */
    UI8_T           storm_bc_cfg;     /* The broadcast traffic rate cfg */
    UI8_T           storm_mc_cfg;     /* The multicast traffic rate cfg */
    UI8_T           storm_uc_cfg;     /* The unicast traffic rate cfg */
    UI8_T           igmp_router_port; /* The static router port of IGMP snooping */
    UI8_T           vlan_ig_filter;   /* The port-based VLAN mechanism for 802.1Q VLAN */
} ATTRIBUTE_PACK DB_PORT_CFG_INFO_T;

/*****************************************
 *  PORT_QOS                             *
 *****************************************/
/* The fields in the PORT_QOS table */
typedef enum
{
    PORT_QOS_PRIORITY = 1, /* The priority level of the port */
    PORT_QOS_Q0_WEIGHT_WRR,    /* The WRR weight of the queue 0 of the port */
    PORT_QOS_Q1_WEIGHT_WRR,    /* The WRR weight of the queue 1 of the port */
    PORT_QOS_Q2_WEIGHT_WRR,    /* The WRR weight of the queue 2 of the port */
    PORT_QOS_Q3_WEIGHT_WRR,    /* The WRR weight of the queue 3 of the port */
    PORT_QOS_Q4_WEIGHT_WRR,    /* The WRR weight of the queue 4 of the port */
    PORT_QOS_Q5_WEIGHT_WRR,    /* The WRR weight of the queue 5 of the port */
    PORT_QOS_Q6_WEIGHT_WRR,    /* The WRR weight of the queue 6 of the port */
    PORT_QOS_Q7_WEIGHT_WRR,    /* The WRR weight of the queue 7 of the port */
    PORT_QOS_Q0_WEIGHT_WFQ,    /* The WFQ weight of the queue 0 of the port */
    PORT_QOS_Q1_WEIGHT_WFQ,    /* The WFQ weight of the queue 1 of the port */
    PORT_QOS_Q2_WEIGHT_WFQ,    /* The WFQ weight of the queue 2 of the port */
    PORT_QOS_Q3_WEIGHT_WFQ,    /* The WFQ weight of the queue 3 of the port */
    PORT_QOS_Q4_WEIGHT_WFQ,    /* The WFQ weight of the queue 4 of the port */
    PORT_QOS_Q5_WEIGHT_WFQ,    /* The WFQ weight of the queue 5 of the port */
    PORT_QOS_Q6_WEIGHT_WFQ,    /* The WFQ weight of the queue 6 of the port */
    PORT_QOS_Q7_WEIGHT_WFQ,    /* The WFQ weight of the queue 7 of the port */
    PORT_QOS_SCHEDULE,         /* The scheduler type of the QoS policy */
    PORT_QOS_LAST
} PORT_QOS_T;

/* The port QoS information table */
typedef struct DB_PORT_QOS_S
{
    UI8_T           priority;     /* The priority level of the port */
    UI8_T           q0_weight_wrr;    /* The WRR weight of the queue 0 of the port */
    UI8_T           q1_weight_wrr;    /* The WRR weight of the queue 1 of the port */
    UI8_T           q2_weight_wrr;    /* The WRR weight of the queue 2 of the port */
    UI8_T           q3_weight_wrr;    /* The WRR weight of the queue 3 of the port */
    UI8_T           q4_weight_wrr;    /* The WRR weight of the queue 4 of the port */
    UI8_T           q5_weight_wrr;    /* The WRR weight of the queue 5 of the port */
    UI8_T           q6_weight_wrr;    /* The WRR weight of the queue 6 of the port */
    UI8_T           q7_weight_wrr;    /* The WRR weight of the queue 7 of the port */
    UI8_T           q0_weight_wfq;    /* The WFQ weight of the queue 0 of the port */
    UI8_T           q1_weight_wfq;    /* The WFQ weight of the queue 1 of the port */
    UI8_T           q2_weight_wfq;    /* The WFQ weight of the queue 2 of the port */
    UI8_T           q3_weight_wfq;    /* The WFQ weight of the queue 3 of the port */
    UI8_T           q4_weight_wfq;    /* The WFQ weight of the queue 4 of the port */
    UI8_T           q5_weight_wfq;    /* The WFQ weight of the queue 5 of the port */
    UI8_T           q6_weight_wfq;    /* The WFQ weight of the queue 6 of the port */
    UI8_T           q7_weight_wfq;    /* The WFQ weight of the queue 7 of the port */
    UI8_T           qos_schedule;     /* The scheduler type of the QoS policy */
} ATTRIBUTE_PACK DB_PORT_QOS_T;

/*****************************************
 *  TRUNK_PORT                           *
 *****************************************/
/* The fields in the TRUNK_PORT table */
typedef enum
{
    TRUNK_MEMBERS = 1,     /* The member ports of the trunk port */
    TRUNK_PORT_LAST
} TRUNK_PORT_T;

/* The trunk port table */
typedef struct TRUNK_MEMBER_INFO_S
{
    UI32_T  member_bmp;
    UI8_T   mode;
} ATTRIBUTE_PACK TRUNK_MEMBER_INFO_T;

typedef struct DB_TRUNK_PORT_S
{
    TRUNK_MEMBER_INFO_T  members;     /* The member ports of the trunk port */
} ATTRIBUTE_PACK DB_TRUNK_PORT_T;

/* The fields in the TRUNK_PORT table */
typedef enum
{
    ALGORITHM = 1,     /* The algorithm of the trunk port: MAC SA&DA/MAC DA/MAC SA */
    TRUNK_ALGORITHM_LAST
} TRUNK_LGORITHM_T;

/* The trunk algorithm table */
typedef struct DB_TRUNK_ALGORITHM_S
{
    UI8_T algorithm;
} ATTRIBUTE_PACK DB_TRUNK_ALGORITHM_T;

#ifdef AIR_SUPPORT_LP
/*****************************************
 *  LOOP_PREVEN_INFO                     *
 *****************************************/
/* The fields in the LOOP_PREVEN_INFO table */
typedef enum
{
    LOOP_PREVEN_ENABLE = 1,/* The admin status of loop prevention */
    LOOP_PREVEN_INFO_LAST
} LOOP_PREVEN_INFO_T;

/* The loop prevention information table */
typedef struct DB_LOOP_PREVEN_INFO_S
{
    UI8_T           enable;                     /* The admin status of loop prevention */
} ATTRIBUTE_PACK DB_LOOP_PREVEN_INFO_T;
#endif /* AIR_SUPPORT_LP */

#ifdef AIR_SUPPORT_IGMP_SNP
/*****************************************
 *  IGMP_SNP_INFO                        *
 *****************************************/
/* The fields in the IGMP_SNP_INFO table */
typedef enum
{
    IGMP_SNP_ENABLE = 1,        /* The admin status of IGMP snooping */
    IGMP_SNP_RPT_SUPPRESS,      /* The report suppression mode of IGMP */
    IGMP_SNP_FAST_LEAVE,        /* The fast leave mode of IGMP */
    IGMP_SNP_INFO_LAST
} IGMP_SNP_INFO_T;

/* The igmp snooping information table */
typedef struct DB_IGMP_SNP_INFO_S
{
    UI8_T           enable;            /* The admin status of IGMP snooping */
    UI8_T           rpt_suppress;      /* The report suppression mode of IGMP */
    UI8_T           fast_leave;        /* The fast leave mode of IGMP */
} ATTRIBUTE_PACK DB_IGMP_SNP_INFO_T;

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
/*****************************************
 *  IGMP_SNP_QUERIER_INFO                *
 *****************************************/
/* The fields in the IGMP_SNP_QUERIER_INFO table */
typedef enum
{
    IGMP_SNP_QUERIER_VLAN_ID = 1,   /* The VLAN ID of IGMP snooping querier */
    IGMP_SNP_QUERIER_STATE,         /* The state of IGMP snooping querier */
    IGMP_SNP_QUERIER_ELECTION,      /* The state of IGMP snooping querier election */
    IGMP_SNP_QUERIER_VERSION,       /* The version of IGMP snooping querier */
    IGMP_SNP_QUERIER_ADDRESS,       /* The source address of IGMP snooping querier */
    IGMP_SNP_QUERIER_INFO_LAST
} IGMP_SNP_QUERIER_INFO_T;

/* The igmp snooping querier information table */
typedef struct DB_IGMP_SNP_QUERIER_INFO_S
{
    UI16_T          vlan_id[MAX_VLAN_ENTRY_NUM];          /* The VLAN ID of IGMP snooping querier */
    UI8_T           state[MAX_VLAN_ENTRY_NUM];            /* The state of IGMP snooping querier */
    UI8_T           election[MAX_VLAN_ENTRY_NUM];         /* The state of IGMP snooping querier election */
    UI8_T           version[MAX_VLAN_ENTRY_NUM];          /* The version of IGMP snooping querier */
    MW_IPV4_T       address[MAX_VLAN_ENTRY_NUM];          /* The source address of IGMP snooping querier */
} ATTRIBUTE_PACK DB_IGMP_SNP_QUERIER_INFO_T;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
#endif /* AIR_SUPPORT_IGMP_SNP */

/*****************************************
 *  PORT_MIRROR_INFO                     *
 *****************************************/
/* The fields in the PORT_MIRROR_INFO table */
typedef enum
{
    PORT_MIRROR_ENABLE= 1, /* The admin status of the port mirror session */
    PORT_MIRROR_DEST_PORT, /* The destination port of the mirror session */
    PORT_MIRROR_SRC_IN_PORT, /* The ingress traffic of the source port */
    PORT_MIRROR_SRC_EG_PORT, /* The egress traffic of the source port */
    PORT_MIRROR_INFO_LAST
} PORT_MIRROR_INFO_T;

/* The port mirror information table */
typedef struct DB_PORT_MIRROR_INFO_S
{
    UI8_T           enable;      /* The admin status of the port mirror session */
    UI8_T           dest_port;   /* The destination port of the mirror session */
    UI32_T          src_in_port; /* The ingress traffic of the source port */
    UI32_T          src_eg_port; /* The egress traffic of the source port */
} ATTRIBUTE_PACK DB_PORT_MIRROR_INFO_T;

/*****************************************
 *  JUMBO_FRAME_INFO                     *
 *****************************************/
/* The fields in the JUMBO_FRAME_INFO table */
typedef enum
{
    JUMBO_FRAME_CFG = 1,   /* The configured MTU size of Jumbo Frame */
    JUMBO_FRAME_INFO_LAST
} JUMBO_FRAME_INFO_T;

/* The jumbo frame information table */
typedef struct DB_JUMBO_FRAME_INFO_S
{
    UI32_T          cfg;                        /* The configured MTU size of Jumbo Frame */
} ATTRIBUTE_PACK DB_JUMBO_FRAME_INFO_T;

/*****************************************
 *  VLAN_CFG_INFO                        *
 *****************************************/
/* The fields in the VLAN_CFG_INFO table */
typedef enum
{
    VLAN_PORT_B_ENABLE = 1,/* The Port-based VLAN mode */
    VLAN_8021Q_B_ENABLE,   /* The 802.1Q-based VLAN mode */
    VLAN_MTU_B_ENABLE,       /* The MTU VLAN mode */
    VLAN_CFG_INFO_LAST
} VLAN_CFG_INFO_T;

/* The global VLAN configuration table */
typedef struct DB_VLAN_CFG_INFO_S
{
    UI8_T           enable_port_b;              /* The Port-based VLAN mode */
    UI8_T           enable_8021q_b;             /* The 802.1Q-based VLAN mode */
    UI8_T           enable_mtu;                 /* The MTU VLAN mode */
} ATTRIBUTE_PACK DB_VLAN_CFG_INFO_T;

/*****************************************
 *  VLAN_ENTRY                           *
 *****************************************/
/* The fields in the VLAN_ENTRY table */
typedef enum
{
    VLAN_ID = 1,           /* The ID of the VLAN */
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    VLAN_FID,
#endif
    VLAN_DESCR,            /* The description of the VLAN */
    VLAN_PORT_MEMBER,      /* The member ports of the port-based VLAN */
    VLAN_TAGGED_MEMBER,    /* The member ports of the tagged VLAN */
    VLAN_UNTAGGED_MEMBER,  /* The member ports of the untagged VLAN */
    VLAN_ENTRY_LAST
} VLAN_ENTRY_T;

/* The VLAN entry information table */
typedef UI8_T       VLAN_DESCR_T[VLAN_DESCR_SIZE];
typedef struct DB_VLAN_ENTRY_S
{
    UI16_T          vlan_id[MAX_VLAN_ENTRY_NUM];          /* The ID of the VLAN */
#if defined(AIR_SUPPORT_ERPS) || defined(AIR_SUPPORT_MSTP)
    UI8_T           vlan_fid[MAX_VLAN_ENTRY_NUM];         /* The FID of the VLAN */
#endif
    VLAN_DESCR_T    descr[MAX_VLAN_ENTRY_NUM];            /* The description of the VLAN */
    UI32_T          port_member[MAX_VLAN_ENTRY_NUM];      /* The member ports of the port-based VLAN */
    UI32_T          tagged_member[MAX_VLAN_ENTRY_NUM];    /* The member ports of the tagged VLAN */
    UI32_T          untagged_member[MAX_VLAN_ENTRY_NUM];  /* The member ports of the untagged VLAN */
} ATTRIBUTE_PACK DB_VLAN_ENTRY_T;

/*****************************************
 *  VOICE_VLAN_INFO                      *
 *****************************************/
/* The fields in the VOICE_VLAN_INFO table */
typedef enum
{
    VOICE_VLAN_STATE = 1,  /* The state of the Voice VLAN */
    VOICE_VLAN_ID,         /* The ID of the Voice VLAN */
    VOICE_VLAN_PRIORITY,   /* The priority level of the Voice VLAN */
#ifdef AIR_SUPPORT_VOICE_VLAN
    VOICE_PORT_MODE,       /* The mode of port joining Voice VLAN */
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    VOICE_SECURITY_MODE,   /* Enable to match OUI and Voice VLAN packets only */
#endif
#endif
    VOICE_VLAN_INFO_LAST
} VOICE_VLAN_INFO_T;

#ifdef AIR_SUPPORT_VOICE_VLAN
/* The voice vlan information table */
typedef struct DB_VOICE_VLAN_INFO_S
{
    UI8_T           vlan_state;                 /* The state of the Voice VLAN */
    UI16_T          vlan_id;                    /* The ID of the Voice VLAN */
    UI8_T           vlan_priority;              /* The priority level of the Voice VLAN */
#ifdef AIR_SUPPORT_VOICE_VLAN
#define VOICE_PORT_MODE_MANUAL    0
#define VOICE_PORT_MODE_AUTO      1
    UI32_T          port_mode;                  /* The mode of each port joining Voice VLAN */
#ifdef AIR_SUPPORT_VOVLAN_SECURITY
    UI32_T          security_mode;              /* Per port enable to match OUI and Voice VLAN packets only */
#endif
#endif
} ATTRIBUTE_PACK DB_VOICE_VLAN_INFO_T;
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
/*****************************************
 *  SURVEI_VLAN_INFO                     *
 *****************************************/
/* The fields in the SURVEI_VLAN_INFO table */
typedef enum
{
    SURVEI_VLAN_STATE = 1, /* The state of the Surveillance VLAN */
    SURVEI_VLAN_ID,        /* The ID of the Surveillance VLAN */
    SURVEI_VLAN_PRIORITY,  /* The priority level of the Surveillance VLAN */
    SURVEI_VLAN_INFO_LAST
} SURVEI_VLAN_INFO_T;

/* The surveillance vlan information table */
typedef struct DB_SURVEI_VLAN_INFO_S
{
    UI8_T           vlan_state;                 /* The state of the Surveillance VLAN */
    UI16_T          vlan_id;                    /* The ID of the Surveillance VLAN */
    UI8_T           vlan_priority;              /* The priority level of the Surveillance VLAN */
} ATTRIBUTE_PACK DB_SURVEI_VLAN_INFO_T;
#endif

#if defined(AIR_SUPPORT_VOICE_VLAN) || defined(AIR_SUPPORT_SURVEILLANCE_VLAN)
/*****************************************
 *  OUI_ENTRY                            *
 *****************************************/
/* The fields in the OUI_ENTRY table */
typedef enum
{
    OUI_MAC = 1,           /* The MSB 3 bytes of MAC address */
    OUI_TYPE,              /* Voice vlan or Surveillance vlan */
    OUI_DESCR,             /* The description of the OUI */
    OUI_ENTRY_LAST
} OUI_ENTRY_T;
/* The OUI entries for voice vlan or surveillance vlan */
typedef UI8_T       VOVLAN_OUI_T[VOVLAN_OUI_SIZE];
typedef C8_T        OUI_DESCR_T[OUI_DESCR_SIZE];/* string length is 10 with null terminate */
typedef struct DB_OUI_ENTRY_S
{
    VOVLAN_OUI_T    mac[MAX_OUI_NUM];           /* The MSB 3 bytes of MAC address */
    UI8_T           type[MAX_OUI_NUM];          /* Voice VLAN or Surveillance VLAN */
    OUI_DESCR_T     descr[MAX_OUI_NUM];         /* The description of the OUI */
} ATTRIBUTE_PACK DB_OUI_ENTRY_T;
#endif

/*****************************************
 *  QOS_INFO                             *
 *****************************************/
/* The fields in the QOS_INFO table */
typedef enum
{
    QOS_MODE = 1,          /* The priority mode of the QOS */
    QOS_PRI_TO_QUEUE,      /* The PRI mapping to the queue values */
    QOS_DSCP_TO_PRI,       /* The DSCP priority mapping to the priority */
    QOS_INFO_LAST
} QOS_INFO_T;

/* The global QoS configuration table */
typedef UI8_T       DSCP_Q_T[MAX_DSCP_NUM];
typedef UI8_T       PRI_Q_T[MAX_QUEUE_NUM];
typedef struct DB_QOS_INFO_S
{
    UI8_T           qos_mode;                    /* The priority mode of the QOS */
    PRI_Q_T         pri_to_queue;                /* The PRI mapping to the queue values */
    DSCP_Q_T        dscp_to_pri;                 /* The DSCP priority mapping to the priority */
} ATTRIBUTE_PACK DB_QOS_INFO_T;

/*****************************************
 *  STATIC_MAC_ENTRY                     *
 *****************************************/
/* The fields in the STATIC_MAC_ENTRY table */
typedef enum
{
    STATIC_MAC_ADDR = 1,   /* The static MAC address */
    STATIC_MAC_VID,        /* The VID of the static MAC */
    STATIC_MAC_PORT,       /* The port of the static MAC */
    STATIC_MAC_ENTRY_LAST
} STATIC_MAC_ENTRY_T;

/* The static mac entries table */
typedef struct DB_STATIC_MAC_ENTRY_S
{
    MW_MAC_T        mac_addr[MAX_STATIC_MAC_NUM]; /* The static MAC address */
    UI16_T          vid[MAX_STATIC_MAC_NUM];      /* The VID of the static MAC */
    UI16_T          port[MAX_STATIC_MAC_NUM];     /* The port of the static MAC */
} ATTRIBUTE_PACK DB_STATIC_MAC_ENTRY_T;

#ifdef AIR_SUPPORT_DHCP_SNOOP
/*****************************************
 *  DHCP_SNP_INFO                        *
 *****************************************/
/* The fields in the DHCP_SNP_INFO table */
typedef enum
{
    DHCP_SNP_ENABLE = 1,   /* The admin status of DHCP snooping */
    DHCP_SNP_INFO_LAST,
} DHCP_SNP_INFO_T;

/* The fields in the DHCP_SNP_PORT_INFO table */
typedef enum
{
    DHCP_SNP_TRUST_PORT = 1, /* The trust port of DHCP snooping */
    DHCP_SNP_OPT82,          /* Option 82 of DHCP snooping */
    DHCP_SNP_OPT82_MODE,     /* Option 82 operation mode */
    DHCP_SNP_OPT82_CIRCUIT_ID_TYPE,
    DHCP_SNP_OPT82_CIRCUIT_ID,
                             /* The circuit id of option 82 */
    DHCP_SNP_OPT82_REMOTE_ID_TYPE,
    DHCP_SNP_OPT82_REMOTE_ID,
                             /* The remote id of option 82 */
    DHCP_SNP_PORT_INFO_LAST,
} DHCP_SNP_PORT_INFO_T;

/* The DHCP snooping entries table */
typedef struct DB_DHCP_SNP_INFO_S
{
    UI8_T           enable;
} ATTRIBUTE_PACK DB_DHCP_SNP_INFO_T;

typedef UI8_T       OPT82_ID_T[MAX_OPT82_ID_LEN];
typedef struct DB_DHCP_SNP_PORT_INFO_S
{
    UI8_T           trust_port;
    UI8_T           opt82_enable;
    UI8_T           opt82_mode;
#define OPT82_MODE_KEEP           0
#define OPT82_MODE_REPLACE        1
#define OPT82_MODE_DROP           2
    UI8_T           circuit_id_type;
#define OPT82_CIR_DEFAULT_ID      0     /* Option 82 circuit default id */
#define OPT82_CIR_USER_ID         255   /* Option 82 circuit user config id */
    OPT82_ID_T      circuit_id;
    UI8_T           remote_id_type;
#define OPT82_REM_DEFAULT_ID_MAC  0     /* Option 82 remote default id: MAC */
#define OPT82_REM_DEFAULT_ID_IP   1     /* Option 82 remote default id: IP */
#define OPT82_REM_USER_ID         255   /* Option 82 remote user config id */
    OPT82_ID_T      remote_id;
} ATTRIBUTE_PACK DB_DHCP_SNP_PORT_INFO_T;
#endif

#ifdef AIR_SUPPORT_SNMP
/* The fields in the SNMP_INFO table */
typedef enum
{
    SNMP_VERSION = 1,      /* SNMP version */
    SNMP_TRAP_HOSTNAME,    /* SNMP trap Host name */
    SNMP_TRAP_EN,          /* SNMP trap enable */
    SNMP_TRAP_TYPE,        /* SNMP trap type*/
    SNMP_TRAP_DST_IP,      /* SNMP trap destination ip */
    SNMP_READ_COMMUNITY,   /* SNMP read community string */
    SNMP_WRITE_COMMUNITY,  /* SNMP write community string */
    SNMP_TRAP_COMMUNITY,    /* SNMP trap community string */
    SNMP_LAST
} SNMP_INFO_T;

typedef struct DB_SNMP_INFO_S
{
    UI8_T           version;
    UI8_T           trap_hostname[MAX_HOST_NAME_SIZE];
    UI8_T           trap_enable;
    UI8_T           trap_type;
    MW_IPV4_T       trap_dst_ip;
    UI8_T           read_community[MAX_SNMP_CM_LEN];
    UI8_T           set_community[MAX_SNMP_CM_LEN];
    UI8_T           trap_community[MAX_SNMP_CM_LEN];
} ATTRIBUTE_PACK DB_SNMP_INFO_T;
#endif

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
typedef enum
{
    STP_ENABLE = 1,
    STP_FORCE_VERSION,
    STP_FORWARD_DELAY,
    STP_MAX_AGE,
    STP_TRANSMIT_HOLD_COUNT,
    STP_BRIDGE_PRIORITY,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    STP_AUTO_COST_SUPPORT,
#endif
#ifdef  AIR_SUPPORT_RSTP_SECURITY
    RSTP_PORT_BPDU_GUARD_STATE,
    RSTP_PORT_ROOT_GUARD_STATE,
    RSTP_PORT_LOOP_GUARD_STATE,
#endif
    STP_PORT_MODE,

    STP_INFO_LAST
} STP_INFO_T;

typedef enum
{
    STP_PORT_PRIORITY = 1,
    STP_PORT_COST,
#ifdef AIR_SUPPORT_STP_AUTO_COST
    STP_PORT_AUTO_COST_ENABLE,
#endif
    STP_PORT_ADMIN_EDGE,
    STP_PORT_MCHECK,
    STP_PORT_STATUS,
    STP_PORT_INFO_LAST
} STP_PORT_INFO_T;

#ifdef AIR_SUPPORT_RSTP_SECURITY
typedef enum
{
    RSTP_PORT_SEC_BPDU_GUARD_ENABLE = 1,
    RSTP_PORT_SEC_TC_GUARD_ENABLE,
    RSTP_PORT_SEC_ROOT_GUARD_ENABLE,
    RSTP_PORT_SEC_LOOP_GUARD_ENABLE,
    RSTP_PORT_SEC_TC_GUARD_INTERVAL,
    RSTP_PORT_SEC_TC_GUARD_THRESHOLD,
    RSTP_PORT_SEC_LAST
} RSTP_PORT_SECURITY_T;
#endif

#ifdef AIR_SUPPORT_MSTP
typedef enum
{
    MSTP_REVISION = 1,
    MSTP_REGION_NAME,

    MSTP_REGION_LAST
}MSTP_REGION_CONFIG_T;

typedef enum
{
    MSTP_INSTANCE_ID = 1,
    MSTP_INSTANCE_PRIORITY,
    MSTP_INSTANCE_VLANLIST,

    MSTP_INSTANCE_LAST
}MSTP_INSTANCE_T;

typedef enum
{
    MSTP_INSTANCE_PORT_PRIORITY = 1,
    MSTP_INSTANCE_PORT_COST,

    MSTP_INSTANCE_PORT_LAST,
}MSTP_INSTANCE_PORT_T;
#endif


typedef struct
{
    UI8_T   enable;
    UI32_T  force_version;
    UI16_T  forward_delay;
    UI16_T  max_age;
    UI16_T  transmit_hold_count;
    UI16_T  priority;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI8_T   auto_cost_support;
#endif
#ifdef AIR_SUPPORT_RSTP_SECURITY
    UI32_T  bg_state;
    UI32_T  rg_state;
    UI32_T  lg_state;
#endif
    UI32_T  portMode;
} ATTRIBUTE_PACK DB_STP_INFO_T;

typedef struct
{
    UI8_T   priority;
    UI32_T  cost;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI8_T   auto_cost_enable;
#endif
    UI8_T   admin_edge;
    UI8_T   mcheck;
    UI8_T   status;
} ATTRIBUTE_PACK DB_STP_PORT_INFO_T;

#ifdef AIR_SUPPORT_RSTP_SECURITY
typedef struct
{
    UI8_T   bpdu_status;
    UI8_T   tc_status;
    UI8_T   root_status;
    UI8_T   loop_status;
    UI16_T  tc_interval;
    UI16_T  tc_threshold;
} ATTRIBUTE_PACK DB_RSTP_PORT_SEC_T;
#endif

#ifdef AIR_SUPPORT_MSTP
typedef struct
{
    UI16_T  revision;
    UI8_T   region_name[MAX_MSTP_REGION_NAME_SIZE];
}ATTRIBUTE_PACK DB_MSTP_REGION_CONFIG_T;

typedef struct
{
    UI16_T  instance;
    UI16_T  priority;
    UI32_T  vlanbmp;
}ATTRIBUTE_PACK DB_MSTP_INSTANCE_T;

typedef struct
{
    UI16_T  instance[MSTP_INSTANCE_MAX_NUM];
    UI16_T  priority[MSTP_INSTANCE_MAX_NUM];
    UI32_T  vlanbmp[MSTP_INSTANCE_MAX_NUM];
}ATTRIBUTE_PACK DB_MSTP_INSTANCE_TABLE_T;

typedef struct
{
    UI8_T   priority[MAX_MSTP_INSTANCE_NUM];
    UI32_T  cost[MAX_MSTP_INSTANCE_NUM];
}ATTRIBUTE_PACK DB_MSTP_INSTANCE_PORT_INFO_T;

typedef struct
{
    UI8_T   priority;
    UI32_T  cost;
}ATTRIBUTE_PACK DB_MSTP_SINGLE_INSTANCE_PORT_INFO_T;

#endif

#endif /* AIR_SUPPORT_RSTP */

#ifdef AIR_SUPPORT_MQTTD
typedef enum
{
    MQTTD_CFG_ENABLE = 1,
    MQTTD_CFG_INFO_LAST
} MQTTD_CFG_INFO_T;

typedef struct
{
    UI8_T   enable;
} ATTRIBUTE_PACK DB_MQTTD_CFG_INFO_T;
#endif /* AIR_SUPPORT_MQTTD */

#ifdef AIR_SUPPORT_LLDPD
/* The LLD info table */
typedef enum
{
    GLOBAL_ENABLE = 1,
    TX_HOLD,
    TX_INTERVAL,
    REINIT_DELAY,
    TX_DELAY,
    LLDP_INFO_LAST,
} LLDP_INFO_T;

typedef struct DB_LLDP_INFO_S
{
    UI8_T          enable;  /*global enable:0 is disable, 1 is enable*/
    UI8_T          tx_hold_multipler;
    UI16_T         tx_interval;
    UI16_T         reinit_delay;  /*if port's adminstatus becomes from "enabled" to "disabled", exec reinit after reinit_delay*/
    UI16_T         tx_delay;  /*the minimum delay between successive lldp transmissions because of value or status change*/
}ATTRIBUTE_PACK DB_LLDP_INFO_T;

typedef enum
{
    LLDP_ENABLE = 1,
    LLDP_PORT_INFO_LAST,
} LLDP_PORT_INFO_T;

typedef struct DB_LLDP_PORT_INFO_S
{
    UI8_T           enable; /*0:disable;1:tx only;2:rx only;3:tx&rx enable*/
}ATTRIBUTE_PACK DB_LLDP_PORT_INFO_T;
#endif

#ifdef AIR_SUPPORT_POE
/* The fields in the SNMP_INFO table */
typedef enum
{
    TOTAL_AVAILABLE_POWER = 1,
#ifdef AIR_SUPPORT_POE_WATCHDOG
    POE_WATCHDOG_PERIOD,
    POE_WATCHDOG_THRESHOLD,
#endif
    POE_CFG_LAST
} POE_CFG_T;

typedef struct DB_POE_CFG_S
{
    UI16_T  total_available_power;
#ifdef AIR_SUPPORT_POE_WATCHDOG
    UI16_T  poe_watchdog_period;
    UI16_T  poe_watchdog_threshold;
#endif
} ATTRIBUTE_PACK DB_POE_CFG_T;

typedef enum
{
    PORT_POWER_CONTROL = 1,
    PORT_AVAILABLE_POWER_MODE,
    PORT_AVAILABLE_POWER,
    PORT_PRIORITY,
    PORT_DELAY_TIME,
    PORT_AF_AT_MODE,
#ifdef AIR_SUPPORT_POE_WATCHDOG
    PORT_POE_WATCHDOG_ENABLE,
#endif
    POE_PORT_CFG_LAST
} POE_PORT_CFG_T;

typedef struct DB_POE_PORT_CFG_S
{
    UI8_T   port_power_control;
#define POE_PORT_PSE_CTRL_EN                             (1)
#define POE_PORT_PSE_CTRL_DIS                            (0)
#define POE_PORT_PWR_STATUS_ON                           (1)
    UI8_T   port_available_power_mode;
#define POE_PORT_AVAI_PWR_SELF_DEFINED                   (1)
#define POE_PORT_AVAI_PWR_DEFAULT                        (0)
    UI16_T  port_available_power;
    UI8_T   port_priority;
#define POE_PORT_PRIO_CRITICAL                           (2)
#define POE_PORT_PRI_HIGH                                (1)
#define POE_PORT_PRI_LOW                                 (0)
    UI8_T   port_delay_time;
    UI8_T   port_af_at_mode;
#define POE_AF_MODE                                      (0)
#define POE_AT_MODE                                      (1)
#define POE_MAX_POE_PORT_AT_PWR                          (300)
#define POE_MAX_POE_PORT_AF_PWR                          (154)
#ifdef AIR_SUPPORT_POE_WATCHDOG
    UI8_T   port_poe_watchdog_enable;
#define POE_PORT_WATCHDOG_EN                             (1)
#define POE_PORT_WATCHDOG_DIS                            (0)
#endif
} ATTRIBUTE_PACK DB_POE_PORT_CFG_T;
#endif

#ifdef AIR_SUPPORT_SNTP
typedef enum
{
    SNTP_CLOCK_MODE = 1,
    SNTP_TIMEZONE,
    SNTP_SERVER_1,
    SNTP_SERVER_2,
    SNTP_SERVER_3,
    SNTP_CFG_LAST
} SNTP_CFG_T;

typedef struct DB_SNTP_CFG_S
{
    UI8_T sntp_mode;
    UI8_T sntp_timezone;
    MW_IPV4_T sntp_server1;
    MW_IPV4_T sntp_server2;
    MW_IPV4_T sntp_server3;
} ATTRIBUTE_PACK DB_SNTP_CFG_T;
#endif

#ifdef AIR_SUPPORT_ERPS
/* The fields in the ERPS_INFO table */
typedef enum
{
    RING_ID = 1,
    CONTROL_VLAN,
    DATA_VLAN,
    REVERTIVE_MODE,
    WEST_PORT,
    EAST_PORT,
    WTR_TIMER,
    GUARD_TIMER,
    HOLD_OFF_TIMER,
    WEST_PORT_STATE,
    EAST_PORT_STATE,
    INSTANCE_STATE,
    ERPS_INFO_LAST
} ERPS_INFO_T;
typedef struct DB_ERPS_INFO_S
{
    UI8_T       ring_id[MAX_ERPS_INSTANCE_NUM];
    UI16_T      control_vlan[MAX_ERPS_INSTANCE_NUM];
    UI8_T       data_vlan[MAX_ERPS_INSTANCE_NUM][MAX_ERPS_DATA_VLAN_SIZE];
#define MW_ERPS_REVERTIVE_MODE_DISABLE (0)
#define MW_ERPS_REVERTIVE_MODE_ENABLE  (1)
    UI8_T       revertive_mode[MAX_ERPS_INSTANCE_NUM];
#define MW_ERPS_ROLE_TRANSFER (0)
#define MW_ERPS_ROLE_NEIGHBOR (1)
#define MW_ERPS_ROLE_OWNER    (2)
    UI8_T       west_port[MAX_ERPS_INSTANCE_NUM];
    UI8_T       east_port[MAX_ERPS_INSTANCE_NUM];
    UI8_T       wtr_timer[MAX_ERPS_INSTANCE_NUM];          /* in mins */
    UI8_T       guard_timer[MAX_ERPS_INSTANCE_NUM];        /* in 10ms */
    UI8_T       hold_off_timer[MAX_ERPS_INSTANCE_NUM];     /* in 100ms */
#define MW_ERPS_PORT_STATE_FORWARDING  (0)
#define MW_ERPS_PORT_STATE_DISCARDING  (1)
    UI8_T       west_port_state[MAX_ERPS_INSTANCE_NUM];
    UI8_T       east_port_state[MAX_ERPS_INSTANCE_NUM];
#define MW_ERPS_INSTANCE_STATE_IDLE             (0)
#define MW_ERPS_INSTANCE_STATE_PROTECTION       (1)
#define MW_ERPS_INSTANCE_STATE_MANUAL_SWITCH    (2)
#define MW_ERPS_INSTANCE_STATE_FORCED_SWITCH    (3)
#define MW_ERPS_INSTANCE_STATE_PENDING          (4)
#define MW_ERPS_INSTANCE_STATE_INIT             (5)
    UI8_T       instance_state[MAX_ERPS_INSTANCE_NUM];
} ATTRIBUTE_PACK DB_ERPS_INFO_T;
#endif

typedef struct DB_DEFAULT_CONFIG_TABLE_S
{
    const void  *ptr_table;
    UI16_T      size;
} ATTRIBUTE_PACK DB_DEFAULT_CONFIG_TABLE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI8_T t_idx);

#endif /* End of DEFAULT_CONFIG_H */