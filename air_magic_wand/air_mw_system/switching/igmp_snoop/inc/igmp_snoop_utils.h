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

/* FILE NAME:  igmp_snoop_utils.h
 * PURPOSE:
 *      This file defines the data structure for IGMP Snooping utilities.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_UTILS_H
#define IGMP_SNOOP_UTILS_H
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "lwip/ip4_addr.h"
#include "osapi_message.h"

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef ATTRIBUTE_PACK
#define ATTRIBUTE_PACK __attribute__((packed))
#endif
#define IGMP_SNP_FORWARD_IN_VLAN                BIT(0)
#define IGMP_SNP_FORWARD_TO_ROUTER              BIT(1)
#define IGMP_SNP_FORWARD_TO_GROUP               BIT(2)

#define IGMP_SNP_WDOG_KICK_NUM                  (50)
#define IGMP_SNP_PORTBMP_STRING_LEN             (40)

/* MACRO FUNCTION DECLARATIONS
 */
#define IGMP_SNP_DMAC_IS_MCAST(dmac)                    ((dmac[0] == 0x01) && (dmac[1] == 0x00) && (dmac[2] == 0x5e))
#define IGMP_SNP_DMAC_FROM_DIP(ip4addr, dmac)      \
    dmac[0] = 0x01;                                \
    dmac[1] = 0x00;                                \
    dmac[2] = 0x5e;                                \
    dmac[3] = (ip4addr & 0x7f0000) >> 16;          \
    dmac[4] = (ip4addr & 0x00ff00) >> 8;           \
    dmac[5] = (ip4addr & 0x0000ff);

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   igmp_snp_clearAllEntry
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAM
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_clearAllEntry(
    void);

#ifdef IGMP_SNP_MW_SUPPORT
/* FUNCTION NAME:   igmp_snp_clearEntryByType
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry by specific type.
 *
 * INPUT:
 *      ptr_data      --  pointer of clear entry data structure
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_clearEntryByType(
    void *ptr_data);

#endif /* IGMP_SNP_MW_SUPPORT */

/* FUNCTION NAME:   igmp_snp_addGroupEntry
 * PURPOSE:
 *      This API is used to add a IGMP snooping group entry.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      ptr_ipaddr   --  multicast group address
 *      port_id      --  ingress front port ID
 *      sec          --  update time in seconds
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_BAD_PARAMETER
 *      MW_E_TABLE_FULL
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_addGroupEntry(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr,
    UI8_T port_id,
    UI16_T sec);

/* FUNCTION NAME:   igmp_snp_delGroupEntry
 * PURPOSE:
 *      This API is used to remove an IGMP snooping group entry.
 *
 * INPUT:
 *      pptr_grp      --  pointer to pointer of group entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      The IGMP snooping group entry will be freed.
 */
MW_ERROR_NO_T
igmp_snp_delGroupEntry(
    void **pptr_grp);

/* FUNCTION NAME:   igmp_snp_searchGroupEntry
 * PURPOSE:
 *      This API is used to search IGMP snooping group entries.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      ptr_ipaddr   --  multicast group address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of entry.
 *
 * NOTES:
 *      None
 */
struct IGMP_SNP_GROUP_LIST_S*
igmp_snp_searchGroupEntry(
    UI16_T vlan_id,
    ip4_addr_t *ptr_ipaddr);

/* FUNCTION NAME:   igmp_snp_updateGroupEntry
 * PURPOSE:
 *      This API is used to update an IGMP snooping group entry to DB.
 *
 * INPUT:
 *      ptr_grp      --  pointer of group entry
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
 *      None
 */
MW_ERROR_NO_T
igmp_snp_updateGroupEntry(
    struct IGMP_SNP_GROUP_LIST_S *ptr_grp);

/* FUNCTION NAME:   igmp_snp_searchMrouterEntry
 * PURPOSE:
 *      This API is used to look for IGMP snooping mrouter entries.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of entry.
 *
 * NOTES:
 *      None
 */
struct IGMP_SNP_MROUTER_LIST_S*
igmp_snp_searchMrouterEntry(
    UI16_T vlan_id);

/* FUNCTION NAME:   igmp_snp_convertGroupEntryToL2mcEntry
 * PURPOSE:
 *      This API is used to convert group entry to L2MC entry.
 *
 * INPUT:
 *      ptr_l2mc_data           -- pointer to the L2MC entry
 *      ptr_grp_data            -- pointer to the group entry
 *
 * OUTPUT:
 *      ptr_l2mc                -- pointer to the L2MC entry(updated)
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_convertGroupEntryToL2mcEntry(
    void    *ptr_l2mc_data,
    void    *ptr_grp_data);

/* FUNCTION NAME:   igmp_snp_convertPortbitmapToString
 * PURPOSE:
 *      This API is used to convert port bitmap to string.
 *
 * INPUT:
 *      portbmp                 -- port bitmap to convert
 *
 * OUTPUT:
 *      ptr_pbmp_str            -- pointer to the string buffer
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_convertPortbitmapToString(
    MW_PORT_BITMAP_T portbmp,
    char             *ptr_pbmp_str);

#endif  /* IGMP_SNOOP_UTILS_H */
