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

/* FILE NAME:  hal_sco_vlan.h
 * PURPOSE:
 *  Define VLAN module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_VLAN_H
#define HAL_SCO_VLAN_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_swc.h>
#include <air_types.h>
#include <air_vlan.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_VLAN_FILTER_ID_MAX (15)

/*  Mac-based, IPv4-based and Selective QinQ share 64 entries
    Select QinQ ranges from index 0 ~ 15         (16 entries)
    MAC-based VLAN ranges from index 16 ~ 31     (16 entries)
    IPv4-based VLAN ranges from index 32 ~ 63    (32 entries)
*/
#define HAL_SCO_VLAN_QINQ_MAC_IPV4_MAX_ENTRY_CNT  (64)
#define HAL_SCO_VLAN_DEFAULT_ENTRY_CNT_QINQ       (16)
#define HAL_SCO_VLAN_DEFAULT_ENTRY_CNT_MAC_BASED  (16)
#define HAL_SCO_VLAN_DEFAULT_ENTRY_CNT_IPV4_BASED (32)
#define HAL_SCO_VLAN_IPV6_BASED_MAX_ENTRY_CNT     (16)
#define HAL_SCO_VLAN_PROTOCOL_BASED_MAX_ENTRY_CNT (4)

#define HAL_SCO_VLAN_MASK_LENGTH_MAC_BASED         (48)
#define HAL_SCO_VLAN_MASK_LENGTH_SUBNET_BASED_IPV6 (128)

#define HAL_SCO_VLAN_PPBV_VID_OFFSET (0)
#define HAL_SCO_VLAN_PPBV_VID_MASK   (0xfff)
#define HAL_SCO_VLAN_PPBV_PRI_OFFSET (13)
#define HAL_SCO_VLAN_PPBV_PRI_MASK   (0x7)

#define HAL_SCO_VLAN_VTCR_BUSY_WAIT_MAX_CNT (100000)
/* MACRO FUNCTION DECLARATIONS
 */
#define PTR_HAL_SCO_VLAN_VTCR_MUTEX(__unit__) \
    (&(_vlan_cb[__unit__].vtcr_mutex))

/* DATA TYPE DECLARATIONS
 */

typedef enum
{
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_READ = 0,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_WRITE,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_INVALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_VLAN_VALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_READ,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_WRITE,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_INVALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_MAC_IPV4_QINQ_VALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_READ,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_WRITE,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_INVALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_IPV6_VALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_READ,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_WRITE,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_INVALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_PROTOCOL_VALID,
    HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_LAST
} HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_T;

typedef struct HAL_SCO_VLAN_CB_S
{
    AIR_SEMAPHORE_ID_T vtcr_mutex;
} HAL_SCO_VLAN_CB_T;

typedef struct HAL_SCO_VLAN_ENTRY_S
{
    BOOL_T            valid;
    UI8_T             fid;
    BOOL_T            ivl;
    BOOL_T            copy_pri;
    UI8_T             user_pri;
    BOOL_T            eg_con;
    BOOL_T            egtag_ctl_en;
    UI8_T             egtag_ctl[AIR_PORT_NUM];
    AIR_PORT_BITMAP_T member_port;
    UI16_T            stag;
} HAL_SCO_VLAN_ENTRY_T;

typedef enum
{
    /* Port matrix mode  */
    HAL_SCO_VLAN_PORT_MODE_PORT_MATRIX = 0,

    /* Fallback mode  */
    HAL_SCO_VLAN_PORT_MODE_FALLBACK,

    /* Check mode  */
    HAL_SCO_VLAN_PORT_MODE_CHECK,

    /* Security mode  */
    HAL_SCO_VLAN_PORT_MODE_SECURITY,
    HAL_SCO_VLAN_PORT_MODE_LAST
} HAL_SCO_VLAN_PORT_MODE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   hal_sco_vlan_writeData
 * PURPOSE:
 *      Write register value of VLAN
 * INPUT:
 *      unit                 -- unit id
 *      addr                 -- vlan id or index of MAC-based/QinQ/IPv4-based/IPv6-based/Protocol-based
 *      ctrl_type            -- control type of VTCR reg
 *      ptr_value0           -- value of VLNWDATA0 reg
 *      ptr_value1           -- value of VLNWDATA1 reg
 *      ptr_value2           -- value of VLNWDATA2 reg
 *      ptr_value3           -- value of VLNWDATA3 reg
 *      ptr_value4           -- value of VLNWDATA4 reg
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_TIMEOUT        -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_writeData(
    const UI32_T                               unit,
    const UI32_T                               addr,
    const HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_T ctrl_type,
    UI32_T                                    *ptr_value0,
    UI32_T                                    *ptr_value1,
    UI32_T                                    *ptr_value2,
    UI32_T                                    *ptr_value3,
    UI32_T                                    *ptr_value4);

/* FUNCTION NAME:   hal_sco_vlan_readData
 * PURPOSE:
 *      Read register value of VLAN
 * INPUT:
 *      unit                 -- unit id
 *      addr                 -- vlan id or index of MAC-based/QinQ/IPv4-based/IPv6-based/Protocol-based
 *      ctrl_type            -- control type of VTCR reg
 * OUTPUT:
 *      ptr_value0           -- value of VLNRDATA0 reg
 *      ptr_value1           -- value of VLNRDATA1 reg
 *      ptr_value2           -- value of VLNRDATA2 reg
 *      ptr_value3           -- value of VLNRDATA3 reg
 *      ptr_value4           -- value of VLNRDATA4 reg
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_TIMEOUT        -- Timeout error.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_readData(
    const UI32_T                               unit,
    const UI32_T                               addr,
    const HAL_SCO_VLAN_VTCR_ACCESS_CTRL_TYPE_T ctrl_type,
    UI32_T                                    *ptr_value0,
    UI32_T                                    *ptr_value1,
    UI32_T                                    *ptr_value2,
    UI32_T                                    *ptr_value3,
    UI32_T                                    *ptr_value4);

/* FUNCTION NAME:   hal_sco_vlan_create
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      ptr_entry   -- A pointer of struct for vlan entry configuration
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Vlan creation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      If ptr_entry is null, vlan would be created, which means the existence of vlan should be checked.
 *      Else, vlan would be init and don't care the existence of vlan for sake of speed.
 */
AIR_ERROR_NO_T
hal_sco_vlan_create(
    const UI32_T            unit,
    const UI16_T            vid,
    AIR_VLAN_ENTRY_T *const ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_destroy
 * PURPOSE:
 *      Destroy the vlan in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK     -- Successfully read the data.
 *      AIR_E_OTHERS -- Vlan destroy failed.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_destroy(
    const UI32_T unit,
    const UI16_T vid);

/* FUNCTION NAME: hal_sco_vlan_getVlan
 * PURPOSE:
 *      Get the VLAN entry when searching VLAN table.
 *
 * INPUT:
 *      unit                     -- unit id
 *      vid                      -- vlan id
 *
 * OUTPUT:
 *      ptr_entry                -- vlan entry based on vid
 *
 * RETURN:
 *      AIR_E_OK                 -- Successfully read the data.
 *      AIR_E_ENTRY_NOT_FOUND    -- Valid vlan entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getVlan(
    const UI32_T            unit,
    const UI16_T            vid,
    AIR_VLAN_ENTRY_T *const ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_setFid
 * PURPOSE:
 *      Set the filter id of the vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      fid         -- filter id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setFid(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T fid);

/* FUNCTION NAME:   hal_sco_vlan_getFid
 * PURPOSE:
 *      Get the filter id of the vlan from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id to be created
 * OUTPUT:
 *      ptr_fid     -- filter id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getFid(
    const UI32_T unit,
    const UI16_T vid,
    UI16_T      *ptr_fid);

/* FUNCTION NAME:   hal_sco_vlan_addMemberPort
 * PURPOSE:
 *      Add one vlan member to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_addMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

/* FUNCTION NAME:   hal_sco_vlan_delMemberPort
 * PURPOSE:
 *      Delete one vlan member from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_delMemberPort(
    const UI32_T unit,
    const UI16_T vid,
    const UI32_T port);

/* FUNCTION NAME:   hal_sco_vlan_setMemberPort
 * PURPOSE:
 *      Replace the vlan members in the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port_bitmap -- member port bitmap
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setMemberPort(
    const UI32_T            unit,
    const UI16_T            vid,
    const AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   hal_sco_vlan_getMemberPort
 * PURPOSE:
 *      Get the vlan members from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      port_bitmap -- member port bitmap
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getMemberPort(
    const UI32_T      unit,
    const UI16_T      vid,
    AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME:   hal_sco_vlan_setIVL
 * PURPOSE:
 *      Set L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable IVL
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setIVL(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_vlan_getIVL
 * PURPOSE:
 *      Get L2 lookup mode IVL/SVL for L2 traffic.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable IVL
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getIVL(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_sco_vlan_setIngressTagKeeping
 * PURPOSE:
 *      Set per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag consistent
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_vlan_getIngressTagKeeping
 * PURPOSE:
 *      Get per vlan egress tag consistent.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag consistent
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getIngressTagKeeping(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_sco_vlan_setEgsTagCtlEnable
 * PURPOSE:
 *      Set per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable vlan egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setEgsTagCtlEnable(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_vlan_getEgsTagCtlEnable
 * PURPOSE:
 *      Get per vlan egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable vlan egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getEgsTagCtlEnable(
    const UI32_T unit,
    const UI16_T vid,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_sco_vlan_setPortEgsTagCtl
 * PURPOSE:
 *      Set vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 *      tag_ctl     -- egress tag control
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortEgsTagCtl(
    const UI32_T                            unit,
    const UI16_T                            vid,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T tag_ctl);

/* FUNCTION NAME:   hal_sco_vlan_getPortEgsTagCtl
 * PURPOSE:
 *      Get vlan port egress tag control.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      port        -- port id
 * OUTPUT:
 *      ptr_tag_ctl -- egress tag control
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortEgsTagCtl(
    const UI32_T                       unit,
    const UI16_T                       vid,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_tag_ctl);

/* FUNCTION NAME:   hal_sco_vlan_setPortAcceptFrameType
 * PURPOSE:
 *      Set vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      type        -- accept frame type
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortAcceptFrameType(
    const UI32_T                       unit,
    const UI32_T                       port,
    const AIR_VLAN_ACCEPT_FRAME_TYPE_T type);

/* FUNCTION NAME:   hal_sco_vlan_getPortAcceptFrameType
 * PURPOSE:
 *      Get vlan accept frame type of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_type    -- accept frame type
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortAcceptFrameType(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_VLAN_ACCEPT_FRAME_TYPE_T *ptr_type);

/* FUNCTION NAME:   hal_sco_vlan_setPortAttr
 * PURPOSE:
 *      Set vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- vlan port attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortAttr(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_VLAN_PORT_ATTR_T attr);

/* FUNCTION NAME:   hal_sco_vlan_getPortAttr
 * PURPOSE:
 *      Get vlan port attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- vlan port attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortAttr(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_VLAN_PORT_ATTR_T *ptr_attr);

/* FUNCTION NAME:   hal_sco_vlan_setIgrPortTagAttr
 * PURPOSE:
 *      Set vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setIgrPortTagAttr(
    const UI32_T                      unit,
    const UI32_T                      port,
    const AIR_IGR_PORT_EGS_TAG_ATTR_T attr);

/* FUNCTION NAME:   hal_sco_vlan_getIgrPortTagAttr
 * PURPOSE:
 *      Get vlan incoming port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getIgrPortTagAttr(
    const UI32_T                 unit,
    const UI32_T                 port,
    AIR_IGR_PORT_EGS_TAG_ATTR_T *ptr_attr);

/* FUNCTION NAME:   hal_sco_vlan_setPortEgsTagAttr
 * PURPOSE:
 *      Set vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      attr        -- egress tag attr
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortEgsTagAttr(
    const UI32_T                            unit,
    const UI32_T                            port,
    const AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T attr);

/* FUNCTION NAME:   hal_sco_vlan_getPortEgsTagAttr
 * PURPOSE:
 *      Get vlan port egress tag attribute from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_attr    -- egress tag attr
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortEgsTagAttr(
    const UI32_T                       unit,
    const UI32_T                       port,
    AIR_VLAN_PORT_EGS_TAG_CTRL_TYPE_T *ptr_attr);

/* FUNCTION NAME:   hal_sco_vlan_setPortPVID
 * PURPOSE:
 *      Set PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      pvid        -- native vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortPVID(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T pvid);

/* FUNCTION NAME:   hal_sco_vlan_getPortPVID
 * PURPOSE:
 *      Get PVID of the port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_pvid    -- native vlan id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortPVID(
    const UI32_T unit,
    const UI32_T port,
    UI16_T      *ptr_pvid);

/* FUNCTION NAME:   hal_sco_vlan_init
 * PURPOSE:
 *      This API is used to init vlan
 * INPUT:
 *      unit -- Device unit number
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_vlan_deinit
 * PURPOSE:
 *      This API is used to deinit vlan
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_deinit(
    const UI32_T unit);

/* FUNCTION NAME:   hal_sco_vlan_setCopyPriority
 * PURPOSE:
 *      Set the state of copying of CTAG priority per VLAN.
 *      Enable to copy priority in CTAG as STAG priority;
 *      Disable to use user priority which is configured in air_vlan_setUserPriority.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      enable      -- enable copy priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setCopyPriority(
    const UI32_T unit,
    const UI16_T vid,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_vlan_getCopyPriority
 * PURPOSE:
 *      Get per vlan copy priority state.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_enable  -- enable copy priority
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getCopyPriority(
    const UI32_T  unit,
    const UI16_T  vid,
    BOOL_T *const ptr_enable);

/* FUNCTION NAME:   hal_sco_vlan_setUserPriority
 * PURPOSE:
 *      Set user priority value in vlan table for STAG when copy priority is disabled.
 *      Copy priority is configured in air_vlan_setPriorityCopy.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      pri         -- user priority value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_setUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    const UI8_T  pri);

/* FUNCTION NAME:   hal_sco_vlan_getUserPriority
 * PURPOSE:
 *      Get per vlan user priority value.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_pri     -- user priority value
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_vlan_getUserPriority(
    const UI32_T unit,
    const UI16_T vid,
    UI8_T *const ptr_pri);

/* FUNCTION NAME:   hal_sco_vlan_setStag
 * PURPOSE:
 *      Set the service tag id in vlan table for vlan stack vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 *      stag        -- stag id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_setStag(
    const UI32_T unit,
    const UI16_T vid,
    const UI16_T stag);

/* FUNCTION NAME:   hal_sco_vlan_getStag
 * PURPOSE:
 *      Get the service tag id in vlan table for vlan stack vlan to the specified device.
 * INPUT:
 *      unit        -- unit id
 *      vid         -- vlan id
 * OUTPUT:
 *      ptr_stag    -- stag id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getStag(
    const UI32_T  unit,
    const UI16_T  vid,
    UI16_T *const ptr_stag);

/* FUNCTION NAME:   hal_sco_vlan_setPortPSVID
 * PURPOSE:
 *      Set the VID of the service tag of the specified port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      psvid       -- service vlan id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_setPortPSVID(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T psvid);

/* FUNCTION NAME:   hal_sco_vlan_getPortPSVID
 * PURPOSE:
 *      Get the VID of the service tag of the specified port from the specified device.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 * OUTPUT:
 *      ptr_psvid   -- service vlan id
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getPortPSVID(
    const UI32_T  unit,
    const UI32_T  port,
    UI16_T *const ptr_psvid);

/* FUNCTION NAME:   hal_sco_vlan_getServiceVlanMaxEntryCnt
 * PURPOSE:
 *      Create the vlan in the specified device.
 * INPUT:
 *      unit                    -- unit id
 * OUTPUT:
 *      ptr_entry_cnt           -- Max entry count
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getServiceVlanMaxCnt(
    const UI32_T unit,
    UI32_T      *ptr_entry_cnt);

/* FUNCTION NAME:   hal_sco_vlan_addMacBasedVlan
 * PURPOSE:
 *      Add MAC-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- MAC-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_addMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_sco_vlan_getMacBasedVlan
 * PURPOSE:
 *      Get a MAC based entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      ptr_entry       --  MAC based entry
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getMacBasedVlan(
    const UI32_T                unit,
    const UI32_T                idx,
    AIR_VLAN_MAC_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME: hal_sco_vlan_delMacBasedVlan
 * PURPOSE:
 *      Delete MAC based entry.by index
 *
 * INPUT:
 *      unit            --  Device ID
 *      idx             --  Entry Index ID
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_delMacBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   hal_sco_vlan_addSubnetBasedVlan
 * PURPOSE:
 *      Add Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- Subnet-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_addSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_getSubnetBasedVlan
 * PURPOSE:
 *      Get Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      ptr_entry               -- Subnet-based VLAN entry
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getSubnetBasedVlan(
    const UI32_T                   unit,
    const UI32_T                   idx,
    AIR_VLAN_SUBNET_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_delSubnetBasedVlan
 * PURPOSE:
 *      Delete Subnet-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_NOT_SUPPORT       -- Feature not support.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_delSubnetBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   hal_sco_vlan_addProtocolBasedVlan
 * PURPOSE:
 *      Add Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 *      ptr_entry               -- Protocol-based VLAN entry
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_EXISTS      -- Entry exist.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_addProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_getProtocolBasedVlan
 * PURPOSE:
 *      Get Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      ptr_entry               -- Protocol-based VLAN entry
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getProtocolBasedVlan(
    const UI32_T                     unit,
    const UI32_T                     idx,
    AIR_VLAN_PROTOCOL_BASED_ENTRY_T *ptr_entry);

/* FUNCTION NAME:   hal_sco_vlan_delProtocolBasedVlan
 * PURPOSE:
 *      Delete Protocol-based VLAN entry
 * INPUT:
 *      unit                    -- unit id
 *      idx                     -- index
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_delProtocolBasedVlan(
    const UI32_T unit,
    const UI32_T idx);

/* FUNCTION NAME:   hal_sco_vlan_setProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Set Protocol-based VLAN port attribute
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      group_id                -- group id
 *      vid                     -- vlan id
 *      pri                     -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_setProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    const UI16_T vid,
    const UI16_T pri);

/* FUNCTION NAME:   hal_sco_vlan_getProtocolBasedVlanPortAttr
 * PURPOSE:
 *      Set Protocol-based VLAN port attribute
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      group_id                -- group id
 * OUTPUT:
 *      ptr_vid                 -- vlan id
 *      ptr_pri                 -- priority
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getProtocolBasedVlanPortAttr(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T group_id,
    UI16_T      *ptr_vid,
    UI16_T      *ptr_pri);

/* FUNCTION NAME:   hal_sco_vlan_setIngressVlanFilter
 * PURPOSE:
 *      Set ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      enable                  -- TRUE: vlan filter enable
 *                                 FALSE: vlan filter disable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_setIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_vlan_getIngressVlanFilter
 * PURPOSE:
 *      Get ingress filter mode of the specified port from the specified device.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_enable              -- TRUE: vlan filter enable
 *                                 FALSE: vlan filter disable
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_NOT_SUPPORT       -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_vlan_getIngressVlanFilter(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_sco_vlan_getCapacity
 * PURPOSE:
 *      Get the vlan resource capacity
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 */
AIR_ERROR_NO_T
hal_sco_vlan_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_sco_vlan_getUsage
 * PURPOSE:
 *      Get the vlan resource usage
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_cnt         --  Count of usage
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 */
AIR_ERROR_NO_T
hal_sco_vlan_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

#endif /* end of HAL_SCO_VLAN_H */
