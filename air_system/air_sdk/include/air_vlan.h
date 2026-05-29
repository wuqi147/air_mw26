/*******************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of Airoha Technology Corp. (C) 2021
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

/* FILE NAME:  air_vlan.h
 * PURPOSE:
 *      It provides VLAN module API.
 * NOTES:
 */

#ifndef AIR_VLAN_H
#define AIR_VLAN_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AIR_VLAN_ID_MIN     (0)
#define AIR_VLAN_ID_MAX     (4095)
#define AIR_DEFAULT_VLAN_ID (1)

#define AIR_VLAN_MAX_TAG_CFI     (1)
#define AIR_VLAN_MACBASE_MAX_IDX (64)
#define AIR_VLAN_MAX_PRIORITY    (8)
#define AIR_VLAN_MAX_MASKLEN     (48)

#define AIR_VLAN_WDOG_KICK_NUM (1000)

#define AIR_VLAN_USER_PRI_MAX_NUM (8)

#define AIR_VLAN_PROTOCOL_BASED_GROUP_ID_MIN (0)
#define AIR_VLAN_PROTOCOL_BASED_GROUP_ID_MAX (3)
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_UNTAGGED = 0,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_SWAP,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_TAGGED,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_STACK,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_LAST,
} AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T;

typedef enum
{
    /* untagged, priority-tagged and tagged */
    AIR_VLAN_ACCEPT_FRAME_TYPE_ALL = 0,

    /* tagged */
    AIR_VLAN_ACCEPT_FRAME_TYPE_TAG_ONLY,

    /* untagged and priority-tagged */
    AIR_VLAN_ACCEPT_FRAME_TYPE_UNTAG_ONLY,
    AIR_VLAN_ACCEPT_FRAME_TYPE_LAST
} AIR_VLAN_ACCEPT_FRAME_TYPE_T;

typedef enum
{
    /* user port */
    AIR_VLAN_PORT_ATTR_USER_PORT = 0,

    /* stack port       */
    AIR_VLAN_PORT_ATTR_STACK_PORT,

    /* transparent port */
    AIR_VLAN_PORT_ATTR_TRANSPARENT_PORT = 3,
    AIR_VLAN_PORT_ATTR_LAST
} AIR_VLAN_PORT_ATTR_T;

typedef enum
{
    AIR_IGR_PORT_EGS_TAG_ATTR_DISABLE = 0,
    AIR_IGR_PORT_EGS_TAG_ATTR_CONSISTENT,
    AIR_IGR_PORT_EGS_TAG_ATTR_UNTAGGED = 4,
    AIR_IGR_PORT_EGS_TAG_ATTR_SWAP,
    AIR_IGR_PORT_EGS_TAG_ATTR_TAGGED,
    AIR_IGR_PORT_EGS_TAG_ATTR_STACK,
    AIR_IGR_PORT_EGS_TAG_ATTR_LAST
} AIR_IGR_PORT_EGS_TAG_ATTR_T;

typedef enum
{
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_ETHERNET = 0,
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_RFC1042,
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LLC_OTHER,
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_SNAP_OTHER,
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_LAST,
} AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_T;

typedef struct AIR_MACBASE_VLAN_FIELD_S
{
    /* VLAN Entry Vaild */
    UI8_T     valid;

    /* MAC Address  */
    AIR_MAC_T mac;

    /* Field Mask , form hight bit to low bit */
    UI8_T     MACMaskLen;

    /* Priority */
    UI8_T     priority;

    /* Customer VID */
    UI16_T    cvid;
} AIR_MACBASE_VLAN_FIELD_T;

typedef struct AIR_VLAN_ENTRY_S
{
#define AIR_VLAN_ENTRY_FLAGS_EN_IVL       (1U << 0)
#define AIR_VLAN_ENTRY_FLAGS_EN_CP_PRI    (1U << 1)
#define AIR_VLAN_ENTRY_FLAGS_EN_EG_CON    (1U << 2)
#define AIR_VLAN_ENTRY_FLAGS_EN_ETAG_CTRL (1U << 3)
    UI32_T                            flags;
    UI16_T                            fid;
    UI16_T                            user_pri;
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T egtag_ctl[AIR_PORT_NUM];
    AIR_PORT_BITMAP_T                 port_bitmap;
    UI16_T                            stag;
} AIR_VLAN_ENTRY_T;

typedef struct AIR_VLAN_MAC_BASED_ENTRY_S
{
    AIR_MAC_T mac;
    AIR_MAC_T mac_mask;
    UI16_T    vid;
    UI16_T    pri;
} AIR_VLAN_MAC_BASED_ENTRY_T;

typedef struct AIR_VLAN_SUBNET_BASED_ENTRY_S
{
    AIR_IP_ADDR_T ip_addr;
    AIR_IP_ADDR_T ip_mask;
    UI16_T        vid;
    UI16_T        pri;
} AIR_VLAN_SUBNET_BASED_ENTRY_T;

typedef union
{
    UI16_T eth_type;
    UI16_T rfc1042_eth_type;
    UI16_T llc_dsap_ssap;
    UI32_T snap_pid[2];
} AIR_VLAN_PROTOCOL_BASED_TYPE_DATA_T;

typedef struct AIR_VLAN_PROTOCOL_BASED_ENTRY_S
{
    AIR_VLAN_PROTOCOL_BASED_FRAME_TYPE_T frame_type;
    AIR_VLAN_PROTOCOL_BASED_TYPE_DATA_T  type_data;
    UI16_T                               group_id;
} AIR_VLAN_PROTOCOL_BASED_ENTRY_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_vlan_createVlan
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      ptr_entry                -- A pointer of struct for vlan entry
 *                                  configuration
 *                                  AIR_VLAN_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      If ptr_entry is null, vlan would be created, which means the
 *      existence of vlan should be checked.
 *      Else, vlan would be init and don't care the existence of vlan
 *      for sake of speed.
 */
AIR_ERROR_NO_T
air_vlan_createVlan(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_VLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_destroyVlan
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_destroyVlan(
    const UI32_T unit,
    const UI16_T vid);

/* FUNCTION NAME:   air_vlan_getVlan
 * PURPOSE:
 *      Get the VLAN entry when searching VLAN table.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_entry                -- vlan entry based on vid
 *                                  AIR_VLAN_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getVlan(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_VLAN_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_setFid
 * PURPOSE:
 *      Set the filter id of the vlan to the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      fid                      -- filter id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setFid(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T fid);

/* FUNCTION NAME:   air_vlan_getFid
 * PURPOSE:
 *      Get the filter id of the vlan from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id to be created
 * OUTPUT:
 *      ptr_fid                  -- filter id
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getFid(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_fid);

/* FUNCTION NAME:   air_vlan_addMemberPort
 * PURPOSE:
 *      Add one vlan member to the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      port                     -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_addMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

/* FUNCTION NAME:   air_vlan_delMemberPort
 * PURPOSE:
 *      Delete one vlan member from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      port                     -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_delMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

/* FUNCTION NAME:   air_vlan_setMember
 * PURPOSE:
 *      Replace the vlan members in the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      port_bitmap              -- member port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setMember(
    const UI32_T            unit,
    const UI16_T            vid,
    const AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   air_vlan_getMember
 * PURPOSE:
 *      Get the vlan members from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      port_bitmap              -- member port bitmap
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getMember(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   air_vlan_setIngressTagKeeping
 * PURPOSE:
 *      Set per vlan egress tag consistent.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      enable                   -- enable vlan egress tag consistent
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   air_vlan_getIngressTagKeeping
 * PURPOSE:
 *      Get per vlan egress tag consistent.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_enable               -- enable vlan egress tag consistent
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_vlan_setVlanEgressTagCtrl
 * PURPOSE:
 *      Set per vlan egress tag control.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      enable                   -- enable vlan egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setVlanEgressTagCtrl(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   air_vlan_getVlanEgressTagCtrl
 * PURPOSE:
 *      Get per vlan egress tag control.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_enable               -- enable vlan egress tag control
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getVlanEgressTagCtrl(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_vlan_setPortEgressTagCtrl
 * PURPOSE:
 *      Set egress tag control of the specific port in the specific
 *      VLAN from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      port                     -- port id
 *      tag_ctl                  -- egress tag control
 *                                  AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortEgressTagCtrl(
    const UI32_T                            unit,
    const UI16_T                            vid,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_ctl);

/* FUNCTION NAME:   air_vlan_getPortEgressTagCtrl
 * PURPOSE:
 *      Get egress tag control of the specific port in the specific
 *      VLAN from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_tag_ctl              -- egress tag control
 *                                  AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPortEgressTagCtrl(
    const UI32_T                       unit,
    const UI16_T                       vid,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_tag_ctl);

/* FUNCTION NAME:   air_vlan_setPortAcceptFrameType
 * PURPOSE:
 *      Set accept VLAN frame type of the port from the specified
 *      device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      type                     -- accept frame type
 *                                  AIR_VLAN_ACCEPT_FRAME_TYPE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortAcceptFrameType(
    const UI32_T                       unit,
    const UI32_T                       port,
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type);

/* FUNCTION NAME:   air_vlan_getPortAcceptFrameType
 * PURPOSE:
 *      Get accept VLAN frame type of the port from the specified
 *      device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_type                 -- accept frame type
 *                                  AIR_VLAN_ACCEPT_FRAME_TYPE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPortAcceptFrameType(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *ptr_type);

/* FUNCTION NAME:   air_vlan_setPortAttr
 * PURPOSE:
 *      Set vlan port attribute from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      attr                     -- vlan port attr
 *                                  AIR_VLAN_PORT_ATTR_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortAttr(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_VLAN_PORT_ATTR_T attr);

/* FUNCTION NAME:   air_vlan_getPortAttr
 * PURPOSE:
 *      Get vlan port attribute from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_attr                 -- vlan port attr
 *                                  AIR_VLAN_PORT_ATTR_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
air_vlan_getPortAttr(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_VLAN_PORT_ATTR_T *ptr_attr);

/* FUNCTION NAME:   air_vlan_setIngressPortTagAttr
 * PURPOSE:
 *      Set vlan incoming port egress tag attribute from the specified
 *      device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      attr                     -- egress tag attr
 *                                  AIR_IGR_PORT_EGS_TAG_ATTR_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setIngressPortTagAttr(
    const UI32_T                      unit,
    const UI32_T                      port,
    const AIR_IGR_PORT_EGS_TAG_ATTR_T attr);

/* FUNCTION NAME:   air_vlan_getIngressPortTagAttr
 * PURPOSE:
 *      Get vlan incoming port egress tag attribute from the specified
 *      device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_attr                 -- egress tag attr
 *                                  AIR_IGR_PORT_EGS_TAG_ATTR_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getIngressPortTagAttr(
    const UI32_T                 unit,
    const UI32_T                 port,
    AIR_IGR_PORT_EGS_TAG_ATTR_T *ptr_attr);

/* FUNCTION NAME:   air_vlan_setPortEgressTagAttr
 * PURPOSE:
 *      Set vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      attr                     -- egress tag attr
 *                                  AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortEgressTagAttr(
    const UI32_T                            unit,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T attr);

/* FUNCTION NAME:   air_vlan_getPortEgressTagAttr
 * PURPOSE:
 *      Get vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_attr                 -- egress tag attr
 *                                  AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPortEgressTagAttr(
    const UI32_T                       unit,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_attr);

/* FUNCTION NAME:   air_vlan_setPortCvid
 * PURPOSE:
 *      Set PVID of the port from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      cvid                     -- native vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortCvid(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T cvid);

/* FUNCTION NAME:   air_vlan_getPortCvid
 * PURPOSE:
 *      Get PVID of the port from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_cvid                 -- native vlan id
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPortCvid(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_cvid);

/* FUNCTION NAME:   air_vlan_setPriorityCopy
 * PURPOSE:
 *      Set the state of copying of CTAG priority per VLAN.
 *      Enable to copy priority in CTAG as STAG priority;
 *      Disable to use user priority which is configured in
 *      air_vlan_setUserPriority.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      enable                   -- enable copy priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPriorityCopy(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   air_vlan_getPriorityCopy
 * PURPOSE:
 *      Get per vlan copy priority state.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_enable               -- enable copy priority
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPriorityCopy(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_vlan_setUserPriority
 * PURPOSE:
 *      Set user priority value in vlan table for STAG when copy
 *      priority is disabled.
 *      Copy priority is configured in air_vlan_setPriorityCopy.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      pri                      -- user priority value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    const UI8_T  pri);

/* FUNCTION NAME:   air_vlan_getUerPriority
 * PURPOSE:
 *      Get per vlan user priority value.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_pri                  -- user priority value
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    UI8_T       *ptr_pri);

/* FUNCTION NAME:   air_vlan_setVlanStag
 * PURPOSE:
 *      Set the service tag id in vlan table for vlan stack vlan to the
 *      specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *      stag                     -- stag id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setVlanStag(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T stag);

/* FUNCTION NAME:   air_vlan_getVlanStag
 * PURPOSE:
 *      Get the service tag id in vlan table for vlan stack vlan to the
 *      specified device.
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 * OUTPUT:
 *      ptr_stag                 -- stag id
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getVlanStag(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_stag);

/* FUNCTION NAME:   air_vlan_setPortSvid
 * PURPOSE:
 *      Set the VID of the service tag of the specified port from the
 *      specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      svid                     -- service vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setPortSvid(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T svid);

/* FUNCTION NAME:   air_vlan_getPortSvid
 * PURPOSE:
 *      Get the VID of the service tag of the specified port from the
 *      specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_psvid                -- service vlan id
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getPortSvid(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_psvid);

/* FUNCTION NAME:   air_vlan_addMacBasedVlan
 * PURPOSE:
 *      Add MAC-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 *      ptr_entry                -- MAC-based VLAN entry
 *                                  AIR_VLAN_MAC_BASED_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_addMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_getMacBasedVlan
 * PURPOSE:
 *      Get MAC-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      ptr_entry                -- MAC-based VLAN entry
 *                                  AIR_VLAN_MAC_BASED_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_delMacBasedVlan
 * PURPOSE:
 *      Delete MAC-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_delMacBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   air_vlan_addSubnetBasedVlan
 * PURPOSE:
 *      Add Subnet-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 *      ptr_entry                -- Subnet-based VLAN entry
 *                                  AIR_VLAN_SUBNET_BASED_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_addSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_getSubnetBasedVlan
 * PURPOSE:
 *      Get Subnet-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      ptr_entry                -- Subnet-based VLAN entry
 *                                  AIR_VLAN_SUBNET_BASED_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_delSubnetBasedVlan
 * PURPOSE:
 *      Delete Subnet-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_delSubnetBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   air_vlan_addProtocolBasedVlan
 * PURPOSE:
 *      Add Protocol-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 *      ptr_entry                -- Protocol-based VLAN entry
 *                                  AIR_VLAN_PROTOCOL_BASED_ENTRY_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_EXISTS       -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_addProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_getProtocolBasedVlan
 * PURPOSE:
 *      Get Protocol-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      ptr_entry                -- Protocol-based VLAN entry
 *                                  AIR_VLAN_PROTOCOL_BASED_ENTRY_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   air_vlan_delProtocolBasedVlan
 * PURPOSE:
 *      Delete Protocol-based VLAN entry
 * INPUT:
 *      unit                     -- unit id
 *      idx                      -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND    -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_delProtocolBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   air_vlan_setProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Set Protocol-based VLAN port attribute
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      group_id                 -- group id
 *      vid                      -- vlan id
 *      pri                      -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    const UI16_T vid,
    const UI16_T pri);

/* FUNCTION NAME:   air_vlan_getProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Get Protocol-based VLAN port attribute
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      group_id                 -- group id
 * OUTPUT:
 *      ptr_vid                  -- vlan id
 *      ptr_pri                  -- priority
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    UI16_T      *ptr_vid,
    UI16_T      *ptr_pri);

/* FUNCTION NAME:   air_vlan_setIngressVlanFilter
 * PURPOSE:
 *      Set ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 *      enable                   -- TRUE: vlan filter enable
 *                                  FALSE: vlan filter disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_setIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   air_vlan_getIngressVlanFilter
 * PURPOSE:
 *      Get ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                     -- unit id
 *      port                     -- port id
 * OUTPUT:
 *      ptr_enable               -- TRUE: vlan filter enable
 *                                  FALSE: vlan filter disable
 * RETURN:
 *      AIR_E_OK                 -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_vlan_getIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

#endif /* End of AIR_VLAN_H */
