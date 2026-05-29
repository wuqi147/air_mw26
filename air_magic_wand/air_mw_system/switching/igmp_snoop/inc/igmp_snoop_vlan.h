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

/* FILE NAME:  igmp_soop_vlan.h
 * PURPOSE:
 *      This file defines the data structure for IGMP snooping VLAN.
 *
 * NOTES:
 */
#ifndef IGMP_SNOOP_VLAN_H
#define IGMP_SNOOP_VLAN_H
/* INCLUDE FILE DECLARATIONS
 */
#include "igmp_snoop.h"
#ifdef IGMP_SNP_MW_SUPPORT
#include "vlan_utils.h"
#include "default_config.h"
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef ATTRIBUTE_PACK
#define ATTRIBUTE_PACK __attribute__((packed))
#endif

#ifdef IGMP_SNP_MW_SUPPORT
#define IGMP_SNP_DEFAULT_VID                    (VLAN_DEFAULT_VID)
#define IGMP_SNP_MIN_VID                        (VLAN_MIN_VID)
#define IGMP_SNP_CONFIG_MAX_VID                 (VLAN_CONFIG_MAX_VID)
#define IGMP_SNP_MAX_VID                        (VLAN_MAX_VID)
#define IGMP_SNP_MAX_VLAN_ENTRY_NUM             (MAX_VLAN_ENTRY_NUM) /* Maximum VLAN entries */
#define IGMP_SNP_VLAN_TAG                       (VLAN_1Q_ENABLE)
#define IGMP_SNP_VLAN_PORT                      (VLAN_PORT_ENABLE)
#define IGMP_SNP_VLAN_MTU                       (VLAN_MTU_ENABLE)

#else
#define IGMP_SNP_MAX_VLAN_ENTRY_NUM             (1)  /* Maximum VLAN entries */
#define IGMP_SNP_DEFAULT_VID                    (1)
#define IGMP_SNP_MIN_VID                        (0)
#define IGMP_SNP_CONFIG_MAX_VID                 (4094)
#define IGMP_SNP_MAX_VID                        (4095)
#define IGMP_SNP_VLAN_TAG                       (2)
#define IGMP_SNP_VLAN_PORT                      (1)
#define IGMP_SNP_VLAN_MTU                       (3)

#endif /* IGMP_SNP_MW_SUPPORT */


/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct IGMP_SNP_VLAN_ENTRY_S
{
    UI16_T          vlan_id;            /* The ID of the VLAN */
    UI32_T          port_member;        /* The member ports of the port-based VLAN */
    UI32_T          tagged_member;      /* The member ports of the tagged VLAN */
    UI32_T          untagged_member;    /* The member ports of the untagged VLAN */
} ATTRIBUTE_PACK IGMP_SNP_VLAN_ENTRY_T;

typedef struct IGMP_SNP_VLAN_CFG_S
{
    UI8_T                   vlan_mode;                  /* The mode of VLAN */
    IGMP_SNP_VLAN_ENTRY_T   vlan_entry[IGMP_SNP_MAX_VLAN_ENTRY_NUM];
} IGMP_SNP_VLAN_CFG_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);

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
    void);

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
    UI16_T vlan_id);

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
    void);

#endif  /* IGMP_SNOOP_VLAN_H */
