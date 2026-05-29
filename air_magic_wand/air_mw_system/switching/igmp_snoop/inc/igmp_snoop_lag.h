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

/* FILE NAME:  igmp_snoop_lag.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping LAG function.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_LAG_H
#define IGMP_SNOOP_LAG_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "default_config.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct IGMP_SNP_LAG_INFO_S
{
    TRUNK_MEMBER_INFO_T lag[MAX_TRUNK_NUM];
#ifdef AIR_SUPPORT_LACP
    UI32_T              aggregated_mbr[MAX_TRUNK_NUM]; /* The aggregated member of LACP */
    UI8_T               lacp_member0[MAX_TRUNK_NUM];
#endif /* AIR_SUPPORT_LACP */
} IGMP_SNP_LAG_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: igmp_snp_lag_initVariable
 * PURPOSE:
 *      Initialize the IGMP SNP LAG variables.
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
igmp_snp_lag_initVariable(
    void);

/* FUNCTION NAME:   igmp_snp_lag_getMinMemberPortByLagId
 * PURPOSE:
 *      This API is used to get the minimum member port of a specific LAG.
 *
 * INPUT:
 *      lag_id       --  LAG ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Minimum member port of the LAG
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_getMinMemberPortByLagId(
    UI32_T  lag_id);

/* FUNCTION NAME:   igmp_snp_lag_getMembers
 * PURPOSE:
 *      This API is used to get the member port bitmap of a specific LAG.
 *
 * INPUT:
 *      lag_id       --  LAG ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The member port bitmap of the specified LAG.
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_getMembers(
    UI32_T lag_id);

/* FUNCTION NAME:   igmp_snp_lag_updatePortBmpWithLagInfo
 * PURPOSE:
 *      This API is used to get the member port bitmap of a specific LAG.
 *
 * INPUT:
 *      PortMap      --  Pointer to the port bitmap to be updated
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The port bitmap of updated by LAG information
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_updatePortBmpWithLagInfo(
    UI32_T PortMap);

/* FUNCTION NAME:   igmp_snp_lag_updateLinkStatusBmpWithLagInfo
 * PURPOSE:
 *      This API is used to update the port bitmap based on the link status and lag information.
 *
 * INPUT:
 *      PortMap   --  port bit map of to be update.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map
 *
 * NOTES:
 *      A bit value of 1 does not guarantee that the port is linkup. The LAG contains the port may be link up.
 *      To determine the actual link status of a port, use igmp_snp_port_getPortBmpLinkStatus().
 */
UI32_T
igmp_snp_lag_updateLinkStatusBmpWithLagInfo(
    UI32_T PortMap);

/* FUNCTION NAME:   igmp_snp_lag_getPortLagId
 * PURPOSE:
 *      This API is used to get the LAG id to which a port belongs.
 *
 * INPUT:
 *      port_id       --  the port id
 *
 * OUTPUT:
 *      lag_id        --  the LAG id which a port belongs
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      If the port is not in the trunk, the LAG id '0' returned
 */
UI32_T
igmp_snp_lag_getPortLagId(
    UI32_T  port_id);

/* FUNCTION NAME:   igmp_snp_lag_getMinMemberPortByPortId
 * PURPOSE:
 *      This API is used to get the minimum member port of a specific port.
 *
 * INPUT:
 *      port_id       --  the port id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      min_port      --  port of minimum LAG port id.
 *
 * NOTES:
 *      If the port is not in the LAG, the port itself is returned
 */
UI32_T
igmp_snp_lag_getMinMemberPortByPortId(
    UI32_T   port_id);

/* FUNCTION NAME:   igmp_snp_lag_updateTxBmpBySrcPort
 * PURPOSE:
 *      This API is used to update tx port member based on LAg infomation and source port.
 *
 * INPUT:
 *      sendPortMap   --  port bit map of to send.
 *      srcPort       --  the source port of the packet to be sent.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_lag_updateTxBmpBySrcPort(
    UI32_T sendPortMap,
    UI32_T srcPort);

/* FUNCTION NAME: igmp_snp_lag_dumpInfo
 * PURPOSE:
 *      Dump IGMP SNP LAG information.
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
 *      None
 */
void
igmp_snp_lag_dumpInfo(
    void);

/* FUNCTION NAME:   igmp_snp_lag_getConfig
 * PURPOSE:
 *      This API is used for MW to get static variable of igmp_snp LAG.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of _igmp_snp_lag_info
 *
 * NOTES:
 *      None
 */
IGMP_SNP_LAG_INFO_T *
igmp_snp_lag_getConfig(
    void);

#endif  /* IGMP_SNOOP_LAG_H */
