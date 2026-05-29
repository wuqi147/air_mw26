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

/* FILE NAME:  igmp_snoop_port.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping port.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_PORT_H
#define IGMP_SNOOP_PORT_H
/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_portbmp.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define IGMP_SNP_ROUTER_PORT_VALID                   (1)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   igmp_snp_port_addMrouterPort
 * PURPOSE:
 *      This API is used to add a IGMP snooping mrouter entry.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
 *      port_id      --  ingress front port ID
 *      time         --  Mrouter timer in seconds
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TABLE_FULL
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_addMrouterPort(
    UI16_T vlan_id,
    UI32_T port_id,
    UI16_T time);

/* FUNCTION NAME:   igmp_snp_port_addStaticMrouterPort
 * PURPOSE:
 *      This API is used to add static mrouter port for IGMP snooping.
 *
 * INPUT:
 *      add_member   --  Port list of static router to add
 *      del_member   --  Port list of static router to remove
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_addStaticMrouterPort(
    UI32_T add_member,
    UI32_T del_member);

/* FUNCTION NAME:   igmp_snp_port_delMrouterPort
 * PURPOSE:
 *      This API is used to delete the mrouter entry for the specified port.
 *
 * INPUT:
 *      pptr_mrtr    --  the pointer to pointer of the mrouter entry
 *      port_id      --  ingress front port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 */
MW_ERROR_NO_T
igmp_snp_port_delMrouterPort(
    void    **pptr_mrtr,
    UI32_T  port_id);

/* FUNCTION NAME:   igmp_snp_port_delStaticMrouterPort
 * PURPOSE:
 *      This API is used to remove static router port of specific VID.
 *
 * INPUT:
 *      vlan_id      --  ingress VLAN ID
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
igmp_snp_port_delStaticMrouterPort(
    UI16_T vlan_id);

/* FUNCTION NAME:   igmp_snp_port_isMrouter
 * PURPOSE:
 *      This API is used to check whether the given port is a mrouter port or not.
 *
 * INPUT:
 *      port_id      --  Port number to check
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      If the port is mrouter port, MW_E_OK will be returned, otherwise MW_E_OTHERS will be returned.
 */
MW_ERROR_NO_T
igmp_snp_port_isMrouter(
    UI32_T port_id);

/* FUNCTION NAME:   igmp_snp_port_clearEntry
 * PURPOSE:
 *      This API is used to clear IGMP snooping group and mrouter entry with port_id.
 *
 * INPUT:
 *      port_id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_clearEntry(
    UI32_T port_id);

/* FUNCTION NAME:   igmp_snp_port_transferEntry
 * PURPOSE:
 *      This API is used to transfer port entry from one port to another port.
 *
 * INPUT:
 *      add_port     --  to add port
 *      del_port     --  to delete port
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
igmp_snp_port_transferEntry(
    UI32_T add_port,
    UI32_T del_port);

/* FUNCTION NAME:   igmp_snp_port_updateEntry
 * PURPOSE:
 *      This API is used to update port's group entry.
 *
 * INPUT:
 *      portbmp       --  Port bit map of to be update.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *      MW_E_TIMEOUT
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
igmp_snp_port_updateEntry(
    MW_PORT_BITMAP_T portbmp);

/* FUNCTION NAME:   igmp_snp_port_getPortBmpLinkStatus
 * PURPOSE:
 *      This API is used to update port member based on port link staus.
 *
 * INPUT:
 *      PortMap   --  port bit map of to get link status.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      new port bit map meaning the port status.
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_port_getPortBmpLinkStatus(
    UI32_T PortMap);

#endif  /* IGMP_SNOOP_PORT_H */
