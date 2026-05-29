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

/* FILE NAME:  igmp_snoop.h
 * PURPOSE:
 * It provides IGMP Snooping module API and definitions.
 *
 * NOTES:
 */

#ifndef IGMP_SNOOP_H
#define IGMP_SNOOP_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_queue.h"
#include "mw_portbmp.h"
#include "lwip/ip4_addr.h"
#include "switch.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMP_SNP_MW_SUPPORT                     (1)     /* Define if magicwand support is enabled */
//#define IGMP_SNP_CUSTOMER_CONFIG_SUPPORT        (1)     /* Define if customer config support is enabled */

#define IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT        (1)
#define IGMP_SNP_TASK_STACK_SIZE                (AIR_MAX_IGMP_STACK_SIZE)
#define IGMP_SNP_TASK_DELAY                     (10)
#define IGMP_SNP_SEMA_DELAY                     (0xFFFFFFFF) /* infinity */

#define IGMP_SNP_TASK_NAME                      "IGMP_SNP"
#define IGMP_SNP_MODULE_NAME                    "igsn"

#ifdef  IGMP_SNP_MW_SUPPORT
#define IGMP_SNP_MAX_L2MC_ENTRY_NUM             (AIR_IGMP_MAX_L2MC_ENTRY_NUM) /* Maximum L2MC entries */
#else
#define IGMP_SNP_MAX_L2MC_ENTRY_NUM             (256) /* Maximum L2MC entries */
#endif /* IGMP_SNP_MW_SUPPORT */

#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
#define IGMP_SNP_MAX_IPMC_GROUP_NUM_ON_PORT     (IGMP_SNP_MAX_L2MC_ENTRY_NUM) /* Maximum multicast groups per port */
#endif

#define IGMP_SNP_ROBUSTNESS                     (2)
#define IGMP_SNP_QUERY_INTERVAL                 (125)  /* Seconds */
#define IGMP_SNP_QUERY_RESP_TIME                (10)   /* Seconds */
#define IGMP_SNP_GROUP_MEMBERSHIP_INTERVAL      ((IGMP_SNP_ROBUSTNESS*IGMP_SNP_QUERY_INTERVAL)+IGMP_SNP_QUERY_RESP_TIME)/* 260 Seconds */
#define IGMP_SNP_OTHER_QUERIER_PRESENT_INTERVAL ((IGMP_SNP_ROBUSTNESS*IGMP_SNP_QUERY_INTERVAL)+IGMP_SNP_QUERY_RESP_TIME/2)
#define IGMP_SNP_STATIC_MROUTER                 (0xFFFF)
#define IGMP_SNP_LAST_MEM_QUERY_CNT             (2)
#define IGMP_SNP_LAST_MEM_QUERY_INTERVAL        (1)
#define IGMP_SNP_RPT_SUPP_TMP_INTERVAL          (1)
#define IGMP_SNP_MAX_TMR_EXPIRE_CNT             (255)

#define IGMP_SNP_IPPROTO_IGMP                   (2)
#define IGMP_SNP_IPPROTO_PIM                    (103)

#ifndef ATTRIBUTE_PACK
#define ATTRIBUTE_PACK __attribute__((packed))
#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct IGMP_SNP_GROUP_LIST_S
{
    UI16_T             vlan_id;
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    MW_MAC_T           mac_address;
#else
    ip4_addr_t         group_address;
#endif
    MW_PORT_BITMAP_T   portbmp;
    UI32_T             leave_flag_bmp;    /* bit map for leave flag for each port */
    UI16_T             timer[AIR_MAX_PORT_NUM];
    UI8_T              rpt_suppress_flag;
    UI8_T              rpt_suppress_timer;
#ifdef IGMP_SNP_MW_SUPPORT
    UI8_T              db_idx;
#endif

    CSLIST_ENTRY(IGMP_SNP_GROUP_LIST_S) next;
} IGMP_SNP_GROUP_LIST_T;

typedef struct IGMP_SNP_MROUTER_LIST_S
{
    UI16_T             vlan_id;
    MW_PORT_BITMAP_T   portbmp;
    UI16_T             timer[AIR_MAX_PORT_NUM];

    CSLIST_ENTRY(IGMP_SNP_MROUTER_LIST_S) next;
} IGMP_SNP_MROUTER_LIST_T;

/* The igmp snooping information table */
typedef struct IGMP_SNP_CFG_INFO_S
{
    UI8_T               enable;                     /* The admin status of IGMP snooping */
    UI8_T               rpt_suppress;               /* The report suppression mode of IGMP */
    UI8_T               fast_leave;                 /* The fast leave mode of IGMP */
#ifdef AIR_SUPPORT_IGMPV3_AWARE
    UI8_T               igmpv3_aware;               /* The v3 version packet aware function of IGMP snooping */
#endif
    UI32_T              igmp_acl_id;                /* The ACL ID of IGMP */
#ifdef AIR_SUPPORT_PIM_AWARE
    UI32_T              pim_acl_id;                 /* The ACL ID of PIM */
#endif
    MW_PORT_BITMAP_T    static_router;              /* The static router port of IGMP */
} IGMP_SNP_CFG_INFO_T;

typedef struct IGMP_SNP_LIST_S
{
    IGMP_SNP_CFG_INFO_T     cfg_info;
#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
    UI16_T port_group_num[AIR_MAX_PORT_NUM];
#endif
    UI8_T glb_suppress_timer;

    CSLIST_HEAD(_igmp_snp_group_list, IGMP_SNP_GROUP_LIST_S) group_head;
    CSLIST_HEAD(_igmp_snp_mrouter_list, IGMP_SNP_MROUTER_LIST_S) mrouter_head;
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    CSLIST_HEAD(_igmp_snp_querier_list, IGMP_QUERIER_LIST_S) querier_head;
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */
} IGMP_SNP_LIST_T;


typedef struct IGMP_SNP_L2_MC_ENTRY_S
{
#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
    MW_MAC_T        mac_addr;     /* The multicast MAC address */
#else
    MW_IPV4_T       ip_addr;      /* The multicast IP address */
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
    UI16_T          vid;          /* The multicast VLAN ID */
    UI32_T          portbmp;      /* The port bitmap of the VLAN */
    UI8_T           update_flag;  /* The update_flag is used to ensure that igmp snooping group entry can be updated to DB
                                     when mac_addr/ip_addr, vid, portbitmap have not changed. */
} ATTRIBUTE_PACK IGMP_SNP_L2_MC_ENTRY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   igmp_snp_deinit
 * PURPOSE:
 *      Free the resources in IGMP snooping init function.
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
igmp_snp_deinit(
    void);

/* FUNCTION NAME:   igmp_snp_init
 * PURPOSE:
 *      This IGMP snooping init function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_init(
    void);

/* FUNCTION NAME:   igmp_snp_getConfig
 * PURPOSE:
 *      This API is used to MW to get static variable of igmp_snp.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of igmp_snp
 *
 * NOTES:
 *      None
 */
IGMP_SNP_LIST_T *
igmp_snp_getConfig(
    void);

#endif /* End of IGMP_SNOOP_H */

