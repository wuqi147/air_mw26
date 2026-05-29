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

/* FILE NAME:  igmp_snoop_log.c
 * PURPOSE:
 *    This file contains the implementation of IGMP snooping logging functions.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop_log.h"
#include "igmp_snoop.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "igmp_snoop_lag.h"
#endif
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_timer.h"
#include "igmp_snoop_utils.h"
#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
#include "igmp_querier_log.h"
#endif
#include "lwip/stats.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
#ifndef IGMP_SNP_MW_SUPPORT
UI8_T igmpsnp_debug_level = IGMP_SNP_DEBUG_LEVEL_DISABLE;
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   igmp_snp_log_dumpInfo
 * PURPOSE:
 *      This API is used to dump IGMP snooping group and mrouter entry.
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
void
igmp_snp_log_dumpInfo(
    void)
{
    UI16_T i;
    UI8_T  pbmp_str[IGMP_SNP_PORTBMP_STRING_LEN];
    UI32_T port = 0;
    struct IGMP_SNP_GROUP_LIST_S   *ptr_grp = NULL;
    struct IGMP_SNP_MROUTER_LIST_S *ptr_mrtr = NULL;
    IGMP_SNP_LIST_T     *ptr_igmp_snp = NULL;
#ifdef IGMP_SNP_MW_SUPPORT
    MW_LOG_LEVEL_T      igmpsnp_debug_level = MW_LOG_LEVEL_OFF;

    mw_log_get_level(MW_LOG_MODULE_IGMP_SNP, &igmpsnp_debug_level);
#endif
    ptr_igmp_snp = igmp_snp_getConfig();
    if (NULL == ptr_igmp_snp)
    {
        return;
    }
    MW_CMD_OUTPUT("\nIGMP SNOOP:\n");
    MW_CMD_OUTPUT("\tAdmin Mode: %s\n", ptr_igmp_snp->cfg_info.enable ? "Enable" : "Disable");
    MW_CMD_OUTPUT("\tFast Leave Mode: %s\n", ptr_igmp_snp->cfg_info.fast_leave ? "Enable" : "Disable");
    MW_CMD_OUTPUT("\tReport Supp Mode: %s\n", ptr_igmp_snp->cfg_info.rpt_suppress ? "Enable" : "Disable");
    MW_CMD_OUTPUT("\tigmp_snp_acl_id: %d\n", ptr_igmp_snp->cfg_info.igmp_acl_id);
#ifdef AIR_SUPPORT_PIM_AWARE
    MW_CMD_OUTPUT("\tpim_acl_id: %d\n", ptr_igmp_snp->cfg_info.pim_acl_id);
#endif
    MW_CMD_OUTPUT("\tDebug Level: %d\n\n", igmpsnp_debug_level);
#ifdef AIR_SUPPORT_IGMPV3_AWARE
    MW_CMD_OUTPUT("\tigmpv3 aware: %s\n", ptr_igmp_snp->cfg_info.igmpv3_aware ? "Enable" : "Disable");
#endif
    igmp_snp_vlan_dumpInfo();

#ifdef IGMP_SNP_MW_SUPPORT
    igmp_snp_lag_dumpInfo();
#endif

#if IGMP_SNP_LIMIT_GROUP_NUM_ON_PORT
    MW_CMD_OUTPUT("\n\tNum of Group Entry on Port\n");
    for(i = 1; i <= PLAT_MAX_PORT_NUM; i++)
    {
        if (ptr_igmp_snp->port_group_num[(i-1)] != 0)
        {
            MW_CMD_OUTPUT("Port %d, port_group_num=%d\n", i, ptr_igmp_snp->port_group_num[(i-1)]);
        }
    }
#endif

    MW_CMD_OUTPUT("\tNum of Group Entry: %d\n", CSLIST_TOTAL(&ptr_igmp_snp->group_head));
    i = 0;
    CSLIST_FOREACH(ptr_grp, &ptr_igmp_snp->group_head, next)
    {
        igmp_snp_convertPortbitmapToString(ptr_grp->portbmp, (char*)pbmp_str);

#ifdef AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE
        MW_CMD_OUTPUT("\t[%d]: vid=%d, mac=%02x-%02x-%02x-%02x-%02x-%02x, portbmp=%s(0x%x)",
                i, ptr_grp->vlan_id,
                ptr_grp->mac_address[0],
                ptr_grp->mac_address[1],
                ptr_grp->mac_address[2],
                ptr_grp->mac_address[3],
                ptr_grp->mac_address[4],
                ptr_grp->mac_address[5],
                pbmp_str, ptr_grp->portbmp[0]);
#else
        MW_CMD_OUTPUT("\t[%d]: vid=%d, group=%s, portbmp=%s(0x%x)",
                i, ptr_grp->vlan_id,
                ip4addr_ntoa(&ptr_grp->group_address),
                pbmp_str, ptr_grp->portbmp[0]);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */
#ifdef IGMP_SNP_MW_SUPPORT
        MW_CMD_OUTPUT(", db_idx=%d\n", ptr_grp->db_idx);
#else
        MW_CMD_OUTPUT("\n");
#endif
        port = 0;
        MWLIB_BITMAP_BIT_FOREACH(ptr_grp->portbmp, port, MW_PORT_BITMAP_SIZE)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            MW_CMD_OUTPUT("\t\t[Port %d]: Timer=%d Sec\n", port, (ptr_grp->timer[(port - 1)] / IGMP_SNP_TMR_RATE_S));
        }

        if (ptr_igmp_snp->cfg_info.rpt_suppress)
        {
            MW_CMD_OUTPUT("\t\tReport Suppression Flag=%d,\tTimer=%d Sec,\tgTime=%d Sec\n",
                        ptr_grp->rpt_suppress_flag,
                        (ptr_grp->rpt_suppress_timer / IGMP_SNP_TMR_RATE_S),
                        (ptr_igmp_snp->glb_suppress_timer / IGMP_SNP_TMR_RATE_S));
        }
        i++;
    }

    i = 0;
    MW_CMD_OUTPUT("\tNum of Mrouter Entry: %d\n", CSLIST_TOTAL(&ptr_igmp_snp->mrouter_head));
    CSLIST_FOREACH(ptr_mrtr, &ptr_igmp_snp->mrouter_head, next)
    {
        igmp_snp_convertPortbitmapToString(ptr_mrtr->portbmp, (char*)pbmp_str);
        MW_CMD_OUTPUT("\t[%d]: vid=%d, portbmp=%s(0x%x)\n",
                i, ptr_mrtr->vlan_id, pbmp_str, ptr_mrtr->portbmp[0]);

        port = 0;
        MWLIB_BITMAP_BIT_FOREACH(ptr_mrtr->portbmp, port, MW_PORT_BITMAP_SIZE)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            MW_CMD_OUTPUT("\t\t[Port %d]: Timer=%d Sec\n",
                            port,
                            ((IGMP_SNP_STATIC_MROUTER == ptr_mrtr->timer[(port - 1)]) ? IGMP_SNP_STATIC_MROUTER : (ptr_mrtr->timer[(port - 1)] / IGMP_SNP_TMR_RATE_S)));
        }
        i++;
    }

#ifdef AIR_SUPPORT_IGMP_SNP_QUERIER
    igmp_querier_dumpInfo();
#endif /* AIR_SUPPORT_IGMP_SNP_QUERIER */

    IGMPSNP_STATS_DISPLAY();

    return;
}
