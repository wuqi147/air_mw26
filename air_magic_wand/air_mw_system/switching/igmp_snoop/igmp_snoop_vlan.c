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

/* FILE NAME:  igmp_soop_vlan.c
 * PURPOSE:
 *    This file contains the implementation of IGMP Snooping VLAN related functions.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#include "igmp_snoop_vlan.h"
#include "igmp_snoop_log.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static IGMP_SNP_VLAN_CFG_T _igmp_snp_vlan_cfg;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: igmp_snp_vlan_initVariable
 * PURPOSE:
 *      Initialize the IGMP SNP VLAN variables.
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
igmp_snp_vlan_initVariable(
    void)
{
    osapi_memset(&_igmp_snp_vlan_cfg, 0, sizeof(IGMP_SNP_VLAN_CFG_T));

#ifndef IGMP_SNP_MW_SUPPORT
    _igmp_snp_vlan_cfg.vlan_entry[0].vlan_id = IGMP_SNP_DEFAULT_VID;
    _igmp_snp_vlan_cfg.vlan_entry[0].port_member = _mw_port_bmp_total[0];
    _igmp_snp_vlan_cfg.vlan_entry[0].tagged_member = _mw_cpu_port;
    _igmp_snp_vlan_cfg.vlan_entry[0].untagged_member = (_mw_port_bmp_total[0] & (~BIT(_mw_cpu_port)));
#endif
    return MW_E_OK;
}

/* FUNCTION NAME: igmp_snp_vlan_dumpInfo
 * PURPOSE:
 *      Dump IGMP SNP VLAN information.
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
igmp_snp_vlan_dumpInfo(
    void)
{
    UI32_T i = 0;

    MW_CMD_OUTPUT("\tVLAN INFO: (mode: %d)\n", _igmp_snp_vlan_cfg.vlan_mode);
    for (i = 0; i < IGMP_SNP_MAX_VLAN_ENTRY_NUM; i++)
    {
        if (_igmp_snp_vlan_cfg.vlan_entry[i].vlan_id != 0)
        {
            MW_CMD_OUTPUT("\t[%d]: vid=%d, portbmp=0x%x, tag_mbr=0x%x, untag_mbr=0x%x\n",
                i, _igmp_snp_vlan_cfg.vlan_entry[i].vlan_id, _igmp_snp_vlan_cfg.vlan_entry[i].port_member, _igmp_snp_vlan_cfg.vlan_entry[i].tagged_member, _igmp_snp_vlan_cfg.vlan_entry[i].untagged_member);
        }
    }
    return;
}


/* FUNCTION NAME:   igmp_snp_vlan_getMember
 * PURPOSE:
 *      This API is used to get vlan member based on vlan id.
 *
 * INPUT:
 *      vlan_id       --  ingress VLAN ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      port bit map of vlan member
 *
 * NOTES:
 *      None
 */
UI32_T
igmp_snp_vlan_getMember(
    UI16_T vlan_id)
{
    UI32_T              index = 0, member = 0;

    for (index = 0; index < IGMP_SNP_MAX_VLAN_ENTRY_NUM; index++)
    {
        if (_igmp_snp_vlan_cfg.vlan_entry[index].vlan_id == vlan_id)
        {
            if((IGMP_SNP_VLAN_PORT == _igmp_snp_vlan_cfg.vlan_mode) || (IGMP_SNP_VLAN_MTU == _igmp_snp_vlan_cfg.vlan_mode))
            {
                member = _igmp_snp_vlan_cfg.vlan_entry[index].port_member;
            }
            else
            {
                member = (_igmp_snp_vlan_cfg.vlan_entry[index].tagged_member | _igmp_snp_vlan_cfg.vlan_entry[index].untagged_member);
            }
            break;
        }
    }

    return member;
}

/* FUNCTION NAME:   igmp_snp_vlan_getConfig
 * PURPOSE:
 *      This API is used for MW to get static variable of igmp_snp VLAN.
 *
 * INPUT:
 *      None.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      pointer of _igmp_snp_vlan_cfg
 *
 * NOTES:
 *      None
 */
IGMP_SNP_VLAN_CFG_T *
igmp_snp_vlan_getConfig(
    void)
{
    return (IGMP_SNP_VLAN_CFG_T *)&_igmp_snp_vlan_cfg;
}

