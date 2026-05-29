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

/* FILE NAME:  hal_coral_swc.c
 * PURPOSE:
 *  Implement switch module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_swc.h>

#include <air_port.h>
#include <air_swc.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_cfg.h>
#include <hal/switch/coral/hal_coral_acl.h>
#include <hal/switch/coral/hal_coral_dos.h>
#include <hal/switch/coral/hal_coral_ipmc.h>
#include <hal/switch/coral/hal_coral_l2.h>
#include <hal/switch/coral/hal_coral_lag.h>
#include <hal/switch/coral/hal_coral_reg.h>
#include <hal/switch/coral/hal_coral_svlan.h>
#include <hal/switch/coral/hal_coral_vlan.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_SWC_MAX_RX_JUMBO_EN                  (3)
#define HAL_CORAL_SWC_JUMBO_SIZE_OFFSET                (1)
#define HAL_CORAL_SWC_FWD_MODE_SYS_SETTING             (0)
#define HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_EXCLUDE_CPU (4)
#define HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_INCLUDE_CPU (5)
#define HAL_CORAL_SWC_FWD_MODE_CPU_ONLY                (6)
#define HAL_CORAL_SWC_FWD_MODE_DROP                    (7)
#define HAL_CORAL_SWC_PRIORITY_EN                      (1U << 3)
#define HAL_CORAL_SWC_LPDET_RX_CTRL_ISSUE_REVISION     (1)

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_CORAL_SWC_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                     \
    {                                                                      \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));              \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));    \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SWC, "hal_coral_swc.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_coral_swc_checkMacAddr
 * PURPOSE:
 *      Check MAC address is valid to be loop detect frame source mac.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Mac address is valid.
 *      AIR_E_BAD_PARAMETER      -- Mac address is invalid.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_coral_swc_checkMacAddr(
    const AIR_MAC_T mac)
{
    AIR_MAC_T bc_mac = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    AIR_MAC_T all_zero_mac = {0, 0, 0, 0, 0, 0};

    if (0 == osal_memcmp(mac, bc_mac, AIR_MAC_ADDR_LEN))
    {
        return AIR_E_BAD_PARAMETER;
    }
    else if (0 == osal_memcmp(mac, all_zero_mac, AIR_MAC_ADDR_LEN))
    {
        return AIR_E_BAD_PARAMETER;
    }
    else if (((0x01) & (mac[0])))
    {
        /* multicast */
        return AIR_E_BAD_PARAMETER;
    }
    else
    {
        return AIR_E_OK;
    }
}

static AIR_ERROR_NO_T
_hal_coral_swc_setPortMgmtFramePri(
    const UI32_T              unit,
    const UI32_T              mac_port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, priority = ptr_cfg->pri;

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI)
    {
        priority |= HAL_CORAL_SWC_PRIORITY_EN;
    }

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_IGMP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_PPPOE_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_ARP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_TAG_PAE_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_UTAG_PAE_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_DHCP_6_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_DHCP_4_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP3_TTL_0_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP3_BPDU_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_MLD_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_REV_01_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP1_REV_02_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_03_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_0E_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_10_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_20_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_21_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP2_REV_UN_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_LLDP:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                _HAL_CORAL_SWC_SET_VALUE(u32dat, priority, CORAL_MFP3_LLDP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                rc = aml_writeReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

static AIR_ERROR_NO_T
_hal_coral_swc_getPortMgmtFramePri(
    const UI32_T              unit,
    const UI32_T              mac_port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, priority = 0;

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_IGMP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_PPPOE_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_ARP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                if (BITS_OFF_R(u32dat, CORAL_MFP1_TAG_PAE_PRI_OFFT, CORAL_MFP_PRI_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_MFP1_UTAG_PAE_PRI_OFFT, CORAL_MFP_PRI_LENGTH))
                {
                    priority = BITS_OFF_R(u32dat, CORAL_MFP1_TAG_PAE_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                }
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                if (BITS_OFF_R(u32dat, CORAL_MFP2_DHCP_6_PRI_OFFT, CORAL_MFP_PRI_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_MFP2_DHCP_4_PRI_OFFT, CORAL_MFP_PRI_LENGTH))
                {
                    priority = BITS_OFF_R(u32dat, CORAL_MFP2_DHCP_6_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
                }
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP3_TTL_0_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP3_BPDU_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_MLD_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_REV_01_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP1_REV_02_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_03_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_0E_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_10_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_20_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_21_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            rc = aml_readReg(unit, CORAL_MFP2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP2_REV_UN_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_LLDP:
            rc = aml_readReg(unit, CORAL_MFP3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                priority = BITS_OFF_R(u32dat, CORAL_MFP3_LLDP_PRI_OFFT, CORAL_MFP_PRI_LENGTH);
            }
            break;
        default:
            break;
    }

    if (AIR_E_OK == rc)
    {
        if (priority & HAL_CORAL_SWC_PRIORITY_EN)
        {
            ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
        }
        else
        {
            ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
        }
        ptr_cfg->pri = priority & BITS(0, 2);
    }
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:
 *      hal_coral_swc_init
 * PURPOSE:
 *      This API is used to initialize switch related items:
 *      1. Set REV02/03/0E/20/21 as BPDU frames.
 *      2. Dynamic entry auto deleted when port link down.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    u32dat = 0;
    AIR_PORT_BITMAP_T         pbm = {0}, mac_pbm = {0};
    AIR_SWC_MGMT_FRAME_CFG_T  mgmt_cfg;
    AIR_SWC_MGMT_FRAME_TYPE_T mgmt_type;
    UI32_T                    port = 0, mac_port = 0;
    UI16_T                    u16dat = 0;
    AIR_CFG_VALUE_T           ethtype_cfg;
    AIR_CFG_VALUE_T           mtcc_cfg;
    AIR_MAC_T                 loop_src_mac = {0};

    /* Set the initial values for all management frames */
    for (mgmt_type = 0; mgmt_type < AIR_SWC_MGMT_FRAME_TYPE_LAST; mgmt_type++)
    {
        osal_memset(&mgmt_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        mgmt_cfg.frame_type = mgmt_type;
        mgmt_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
        switch (mgmt_type)
        {
            case AIR_SWC_MGMT_FRAME_TYPE_ARP:
                mgmt_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
                mgmt_cfg.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE + AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
                mgmt_cfg.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                break;
            default:
                mgmt_cfg.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE + AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        }
        hal_coral_swc_setMgmtFrameCfg(unit, &mgmt_cfg);
    }

    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
        /* Designate the following packet as a BPDU frame */
        aml_readReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
        u32dat |= BIT(CORAL_REG_REV_02_BPDU_OFFT);
        aml_writeReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));

        aml_readReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
        u32dat |= BIT(CORAL_REG_REV_0E_BPDU_OFFT);
        u32dat |= BIT(CORAL_REG_REV_03_BPDU_OFFT);
        aml_writeReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));

        aml_readReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
        u32dat |= BIT(CORAL_REG_REV_20_BPDU_OFFT);
        aml_writeReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));

        aml_readReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
        u32dat |= BIT(CORAL_REG_REV_21_BPDU_OFFT);
        aml_writeReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));

        aml_readReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
        u32dat |= BIT(CORAL_REG_BPDU_BPDU_OFFT);
        aml_writeReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
    }

    /* Enable auto flush */
    aml_readReg(unit, CORAL_AAC, &u32dat, sizeof(u32dat));
    u32dat |= BIT(CORAL_AAC_AUTO_FLUSH_OFFSET);
    aml_writeReg(unit, CORAL_AAC, &u32dat, sizeof(u32dat));

    /* Set mtcc */
    osal_memset(&mtcc_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT, &mtcc_cfg);
    aml_readReg(unit, CORAL_GMACCR, &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(CORAL_GMACCR_MTCC_OFFT, CORAL_GMACCR_MTCC_LENG);
    u32dat |= ((mtcc_cfg.value << CORAL_GMACCR_MTCC_OFFT) & BITS_RANGE(CORAL_GMACCR_MTCC_OFFT, CORAL_GMACCR_MTCC_LENG));
    aml_writeReg(unit, CORAL_GMACCR, &u32dat, sizeof(u32dat));

    AIR_PORT_BITMAP_COPY(pbm, HAL_PORT_BMP_ETH(unit));
    if (HAL_IS_PORT_VALID(unit, HAL_CPU_PORT(unit)))
    {
        AIR_PORT_DEL(pbm, HAL_CPU_PORT(unit));
    }
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, pbm, mac_pbm);

    /* Set IGMP router port bitmap */
    aml_writeReg(unit, CORAL_DRP, &mac_pbm[0], sizeof(UI32_T));

    /* Set IGMP query message forwarding port bitmap */
    aml_writeReg(unit, CORAL_QRYP, &mac_pbm[0], sizeof(UI32_T));

    /* SLED clock rate fine tune; refresh 60Hz, clock 2.08Mhz */
    u32dat = (35000 << 16) + 12;
    aml_writeReg(unit, CORAL_SLED_CTRL1, &u32dat, sizeof(u32dat));

    /* Set default rate limit include preamble/IPG/CRC */
    aml_readReg(unit, CORAL_GERLCR, &u32dat, sizeof(u32dat));
    _HAL_CORAL_SWC_SET_VALUE(u32dat, CORAL_L1_RATE_IPG_BYTE_CNT, CORAL_REG_IPG_BYTE_OFFT, CORAL_REG_IPG_BYTE_LENG);
    aml_writeReg(unit, CORAL_GERLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, CORAL_GIRLCR, &u32dat, sizeof(u32dat));
    _HAL_CORAL_SWC_SET_VALUE(u32dat, CORAL_L1_RATE_IPG_BYTE_CNT, CORAL_REG_IPG_BYTE_OFFT, CORAL_REG_IPG_BYTE_LENG);
    aml_writeReg(unit, CORAL_GIRLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
    _HAL_CORAL_SWC_SET_VALUE(u32dat, CORAL_L1_RATE_IPG_BYTE_CNT, CORAL_AGC_COMP_BNUM_OFFT, CORAL_AGC_COMP_BNUM_LENG);
    aml_writeReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));

    /* Set default CPU TX drop disable */
    if (HAL_IS_PORT_VALID(unit, HAL_CPU_PORT(unit)))
    {
        HAL_AIR_PORT_TO_MAC_PORT(unit, HAL_CPU_PORT(unit), mac_port);
        aml_readReg(unit, CORAL_CTRDDCR, &u32dat, sizeof(u32dat));
        _HAL_CORAL_SWC_SET_VALUE(u32dat, (1 << mac_port), CORAL_CTRDDCR_CPU_TX_DROP_DIS_OFFT,
                                 CORAL_CTRDDCR_CPU_TX_DROP_DIS_LENG);
        aml_writeReg(unit, CORAL_CTRDDCR, &u32dat, sizeof(u32dat));
    }

    /* Set default value about loop related register */
    osal_memset(&ethtype_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    ethtype_cfg.value = AIR_SWC_LPDET_ETH_TYPE_DEFAULT;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_LPDET_ETH_TYPE, &ethtype_cfg);
    u16dat = ethtype_cfg.value;
    /* set ldf ether type */
    aml_readReg(unit, CORAL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(CORAL_LPDET_FRAME_TYPE_OFFT, CORAL_LPDET_FRAME_TYPE_LENG);
    u32dat |= BITS_OFF_L(u16dat, CORAL_LPDET_FRAME_TYPE_OFFT, CORAL_LPDET_FRAME_TYPE_LENG);
    aml_writeReg(unit, CORAL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));

    /* set ldf source mac */
    hal_coral_swc_getSystemMac(unit, loop_src_mac);
    hal_coral_swc_setLoopDetectFrameSrcMac(unit, loop_src_mac);

    /* set over_rxpause = 1, rate_1s = 1 */
    aml_readReg(unit, CORAL_LPDETCR, &u32dat, sizeof(u32dat));
    u32dat |= (CORAL_LPDETCR_OVER_RXPAUSE | CORAL_LPDETCR_PERIOD_1S);
    aml_writeReg(unit, CORAL_LPDETCR, &u32dat, sizeof(u32dat));
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setMgmtFrameCfg
 * PURPOSE:
 *      Set management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_cfg         --  config for specific management frame
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_coral_swc_setMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         port = 0;

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI)
    {
        rc = AIR_E_OP_INVALID;
    }

    if (AIR_E_OK == rc)
    {
        if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH)
        {
            ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
            ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
            ptr_cfg->pri = AIR_SWC_PRI_MAX_NUM;
        }
        else
        {
            ptr_cfg->pri = 0;
        }

        AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
        {
            rc = hal_coral_swc_setPortMgmtFrameCfg(unit, port, ptr_cfg);
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_ERR, "Port(%u) mgmt-frame cfg set fail(%d)\n", port, rc);
                break;
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getMgmtFrameCfg
 * PURPOSE:
 *      Get management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_cfg         --  config for specific management frame
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_coral_swc_getMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   port = 0;
    BOOL_T                   first = TRUE;
    AIR_SWC_MGMT_FRAME_CFG_T mgmt_cfg;

    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        rc = hal_coral_swc_getPortMgmtFrameCfg(unit, port, ptr_cfg);
        if (AIR_E_OK == rc)
        {
            if (first)
            {
                osal_memcpy(&mgmt_cfg, ptr_cfg, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
                first = FALSE;
                if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI)
                {
                    if (AIR_SWC_PRI_MAX_NUM != ptr_cfg->pri)
                    {
                        rc = AIR_E_OP_INVALID;
                    }
                }
                else
                {
                    if (0 != ptr_cfg->pri)
                    {
                        rc = AIR_E_OP_INVALID;
                    }
                }
            }
            else
            {
                if (osal_memcmp(&mgmt_cfg, ptr_cfg, sizeof(AIR_SWC_MGMT_FRAME_CFG_T)))
                {
                    rc = AIR_E_OP_INVALID;
                }
            }
        }
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "Port(%u) mgmt-frame cfg get fail(%d)\n", port, rc);
            break;
        }
    }

    if (AIR_E_OK == rc)
    {
        if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI)
        {
            ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
            ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setPortMgmtFrameCfg
 * PURPOSE:
 *     Get management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_cfg         --  config for specific management frame
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_coral_swc_setPortMgmtFrameCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port;
    UI32_T         enable = 0, fwd = 0, as_bpdu = 0;

    HAL_CHECK_ENUM_RANGE(ptr_cfg->frame_type, AIR_SWC_MGMT_FRAME_TYPE_LAST);
    HAL_CHECK_ENUM_RANGE(ptr_cfg->forward_mode, AIR_SWC_MGMT_FWD_MODE_LAST);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU)
    {
        as_bpdu = 1;
    }
    else
    {
        as_bpdu = 0;
    }

    switch (ptr_cfg->forward_mode)
    {
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING:
            fwd = HAL_CORAL_SWC_FWD_MODE_SYS_SETTING;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
            fwd = HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
            fwd = HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_CPU_ONLY:
            fwd = HAL_CORAL_SWC_FWD_MODE_CPU_ONLY;
            break;
        case AIR_SWC_MGMT_FWD_MODE_DROP:
            fwd = HAL_CORAL_SWC_FWD_MODE_DROP;
            break;
        default:
            break;
    }

    if ((ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH) &&
        (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI))
    {
        rc = AIR_E_OP_INVALID;
    }

    if (AIR_E_OK == rc)
    {
        switch (ptr_cfg->frame_type)
        {
            case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
                rc = aml_readReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_IGMP_MANG_OFFT, CORAL_REG_IGMP_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_IGMP_BPDU_OFFT, CORAL_REG_IGMP_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_IGMP_PORT_FWD_OFFT, CORAL_REG_IGMP_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
                rc = aml_readReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_PPPOE_MANG_OFFT, CORAL_REG_PPPOE_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_PPPOE_BPDU_OFFT, CORAL_REG_PPPOE_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_PPPOE_PORT_FWD_OFFT,
                                             CORAL_REG_PPPOE_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_ARP:
                rc = aml_readReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_ARP_MANG_OFFT, CORAL_REG_ARP_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_ARP_BPDU_OFFT, CORAL_REG_ARP_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_ARP_PORT_FWD_OFFT, CORAL_REG_ARP_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_PAE:
                rc = aml_readReg(unit, CORAL_PAC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_TAG_PAE_MANG_OFFT,
                                             CORAL_REG_TAG_PAE_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_UTAG_PAE_MANG_OFFT,
                                             CORAL_REG_UTAG_PAE_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_TAG_PAE_BPDU_OFFT,
                                             CORAL_REG_TAG_PAE_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_UTAG_PAE_BPDU_OFFT,
                                             CORAL_REG_UTAG_PAE_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_TAG_PAE_PORT_FWD_OFFT,
                                             CORAL_REG_TAG_PAE_PORT_FWD_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_UTAG_PAE_PORT_FWD_OFFT,
                                             CORAL_REG_UTAG_PAE_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_PAC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
                rc = aml_readReg(unit, CORAL_DHCP(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_DHCP_6_MANG_OFFT, CORAL_REG_DHCP_6_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_DHCP_4_MANG_OFFT, CORAL_REG_DHCP_4_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_DHCP_6_BPDU_OFFT, CORAL_REG_DHCP_6_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_DHCP_4_BPDU_OFFT, CORAL_REG_DHCP_4_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_DHCP_6_PORT_FWD_OFFT,
                                             CORAL_REG_DHCP_6_PORT_FWD_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_DHCP_4_PORT_FWD_OFFT,
                                             CORAL_REG_DHCP_4_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_DHCP(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
                rc = aml_readReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_TTL_0_MANG_OFFT, CORAL_REG_TTL_0_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_TTL_0_BPDU_OFFT, CORAL_REG_TTL_0_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_TTL_0_PORT_FWD_OFFT,
                                             CORAL_REG_TTL_0_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
                rc = aml_readReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_BPDU_MANG_OFFT, CORAL_REG_BPDU_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_BPDU_BPDU_OFFT, CORAL_REG_BPDU_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_BPDU_PORT_FWD_OFFT, CORAL_REG_BPDU_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_MLD:
                rc = aml_readReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_MLD_MANG_OFFT, CORAL_REG_MLD_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_MLD_BPDU_OFFT, CORAL_REG_MLD_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_MLD_PORT_FWD_OFFT, CORAL_REG_MLD_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
                rc = aml_readReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_01_MANG_OFFT, CORAL_REG_REV_01_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_01_BPDU_OFFT, CORAL_REG_REV_01_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_01_PORT_FWD_OFFT,
                                             CORAL_REG_REV_01_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
                rc = aml_readReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_02_MANG_OFFT, CORAL_REG_REV_02_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_02_BPDU_OFFT, CORAL_REG_REV_02_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_02_PORT_FWD_OFFT,
                                             CORAL_REG_REV_02_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
                rc = aml_readReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_03_MANG_OFFT, CORAL_REG_REV_03_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_03_BPDU_OFFT, CORAL_REG_REV_03_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_03_PORT_FWD_OFFT,
                                             CORAL_REG_REV_03_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
                rc = aml_readReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_0E_MANG_OFFT, CORAL_REG_REV_0E_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_0E_BPDU_OFFT, CORAL_REG_REV_0E_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_0E_PORT_FWD_OFFT,
                                             CORAL_REG_REV_0E_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
                rc = aml_readReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_10_MANG_OFFT, CORAL_REG_REV_10_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_10_BPDU_OFFT, CORAL_REG_REV_10_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_10_PORT_FWD_OFFT,
                                             CORAL_REG_REV_10_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
                rc = aml_readReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_20_MANG_OFFT, CORAL_REG_REV_20_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_20_BPDU_OFFT, CORAL_REG_REV_20_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_20_PORT_FWD_OFFT,
                                             CORAL_REG_REV_20_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
                rc = aml_readReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_21_MANG_OFFT, CORAL_REG_REV_21_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_21_BPDU_OFFT, CORAL_REG_REV_21_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_21_PORT_FWD_OFFT,
                                             CORAL_REG_REV_21_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
                rc = aml_readReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_REV_UN_MANG_OFFT, CORAL_REG_REV_UN_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_REV_UN_BPDU_OFFT, CORAL_REG_REV_UN_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_REV_UN_PORT_FWD_OFFT,
                                             CORAL_REG_REV_UN_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            case AIR_SWC_MGMT_FRAME_TYPE_LLDP:
                rc = aml_readReg(unit, CORAL_LCC(mac_port), &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_REG_LLDP_MANG_OFFT, CORAL_REG_LLDP_MANG_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, as_bpdu, CORAL_REG_LLDP_BPDU_OFFT, CORAL_REG_LLDP_BPDU_LENGTH);
                    _HAL_CORAL_SWC_SET_VALUE(u32dat, fwd, CORAL_REG_LLDP_PORT_FWD_OFFT, CORAL_REG_LLDP_PORT_FWD_LENGTH);
                    rc = aml_writeReg(unit, CORAL_LCC(mac_port), &u32dat, sizeof(u32dat));
                }
                break;
            default:
                rc = AIR_E_BAD_PARAMETER;
                break;
        }
        if (AIR_E_OK == rc)
        {
            if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH)
            {
                ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
                ptr_cfg->pri = AIR_SWC_PRI_MAX_NUM;
            }
            rc = _hal_coral_swc_setPortMgmtFramePri(unit, mac_port, ptr_cfg);
        }
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getPortMgmtFrameCfg
 * PURPOSE:
 *     Get management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_cfg         --  config for specific management frame
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_coral_swc_getPortMgmtFrameCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port;
    UI32_T         enable = 0, fwd = 0, as_bpdu = 0;
    HAL_CHECK_ENUM_RANGE(ptr_cfg->frame_type, AIR_SWC_MGMT_FRAME_TYPE_LAST);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            rc = aml_readReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_IGMP_MANG_OFFT, CORAL_REG_IGMP_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_IGMP_BPDU_OFFT, CORAL_REG_IGMP_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_IGMP_PORT_FWD_OFFT, CORAL_REG_IGMP_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            rc = aml_readReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_PPPOE_MANG_OFFT, CORAL_REG_PPPOE_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_PPPOE_BPDU_OFFT, CORAL_REG_PPPOE_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_PPPOE_PORT_FWD_OFFT, CORAL_REG_PPPOE_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            rc = aml_readReg(unit, CORAL_APC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_ARP_MANG_OFFT, CORAL_REG_ARP_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_ARP_BPDU_OFFT, CORAL_REG_ARP_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_ARP_PORT_FWD_OFFT, CORAL_REG_ARP_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            rc = aml_readReg(unit, CORAL_PAC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                if (BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_MANG_OFFT, CORAL_REG_TAG_PAE_MANG_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_UTAG_PAE_MANG_OFFT, CORAL_REG_UTAG_PAE_MANG_LENGTH))
                {
                    enable = BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_MANG_OFFT, CORAL_REG_TAG_PAE_MANG_LENGTH);
                }
                if (BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_BPDU_OFFT, CORAL_REG_TAG_PAE_BPDU_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_UTAG_PAE_BPDU_OFFT, CORAL_REG_UTAG_PAE_BPDU_LENGTH))
                {
                    as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_BPDU_OFFT, CORAL_REG_TAG_PAE_BPDU_LENGTH);
                }
                if (BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_PORT_FWD_OFFT, CORAL_REG_TAG_PAE_PORT_FWD_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_UTAG_PAE_PORT_FWD_OFFT, CORAL_REG_UTAG_PAE_PORT_FWD_LENGTH))
                {
                    fwd = BITS_OFF_R(u32dat, CORAL_REG_TAG_PAE_PORT_FWD_OFFT, CORAL_REG_TAG_PAE_PORT_FWD_LENGTH);
                }
                rc = aml_readReg(unit, CORAL_MFP1(mac_port), &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            rc = aml_readReg(unit, CORAL_DHCP(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                if (BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_MANG_OFFT, CORAL_REG_DHCP_6_MANG_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_DHCP_4_MANG_OFFT, CORAL_REG_DHCP_4_MANG_LENGTH))
                {
                    enable = BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_MANG_OFFT, CORAL_REG_DHCP_6_MANG_LENGTH);
                }
                if (BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_BPDU_OFFT, CORAL_REG_DHCP_6_BPDU_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_DHCP_4_BPDU_OFFT, CORAL_REG_DHCP_4_BPDU_LENGTH))
                {
                    as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_BPDU_OFFT, CORAL_REG_DHCP_6_BPDU_LENGTH);
                }
                if (BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_PORT_FWD_OFFT, CORAL_REG_DHCP_6_PORT_FWD_LENGTH) ==
                    BITS_OFF_R(u32dat, CORAL_REG_DHCP_4_PORT_FWD_OFFT, CORAL_REG_DHCP_4_PORT_FWD_LENGTH))
                {
                    fwd = BITS_OFF_R(u32dat, CORAL_REG_DHCP_6_PORT_FWD_OFFT, CORAL_REG_DHCP_6_PORT_FWD_LENGTH);
                }
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            rc = aml_readReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_TTL_0_MANG_OFFT, CORAL_REG_TTL_0_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_TTL_0_BPDU_OFFT, CORAL_REG_TTL_0_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_TTL_0_PORT_FWD_OFFT, CORAL_REG_TTL_0_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            rc = aml_readReg(unit, CORAL_BPC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_BPDU_MANG_OFFT, CORAL_REG_BPDU_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_BPDU_BPDU_OFFT, CORAL_REG_BPDU_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_BPDU_PORT_FWD_OFFT, CORAL_REG_BPDU_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            rc = aml_readReg(unit, CORAL_IMMC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_MLD_MANG_OFFT, CORAL_REG_MLD_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_MLD_BPDU_OFFT, CORAL_REG_MLD_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_MLD_PORT_FWD_OFFT, CORAL_REG_MLD_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            rc = aml_readReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_01_MANG_OFFT, CORAL_REG_REV_01_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_01_BPDU_OFFT, CORAL_REG_REV_01_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_01_PORT_FWD_OFFT, CORAL_REG_REV_01_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            rc = aml_readReg(unit, CORAL_RGAC1(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_02_MANG_OFFT, CORAL_REG_REV_02_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_02_BPDU_OFFT, CORAL_REG_REV_02_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_02_PORT_FWD_OFFT, CORAL_REG_REV_02_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            rc = aml_readReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_03_MANG_OFFT, CORAL_REG_REV_03_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_03_BPDU_OFFT, CORAL_REG_REV_03_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_03_PORT_FWD_OFFT, CORAL_REG_REV_03_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            rc = aml_readReg(unit, CORAL_RGAC2(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_0E_MANG_OFFT, CORAL_REG_REV_0E_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_0E_BPDU_OFFT, CORAL_REG_REV_0E_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_0E_PORT_FWD_OFFT, CORAL_REG_REV_0E_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            rc = aml_readReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_10_MANG_OFFT, CORAL_REG_REV_10_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_10_BPDU_OFFT, CORAL_REG_REV_10_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_10_PORT_FWD_OFFT, CORAL_REG_REV_10_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            rc = aml_readReg(unit, CORAL_RGAC3(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_20_MANG_OFFT, CORAL_REG_REV_20_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_20_BPDU_OFFT, CORAL_REG_REV_20_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_20_PORT_FWD_OFFT, CORAL_REG_REV_20_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            rc = aml_readReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_21_MANG_OFFT, CORAL_REG_REV_21_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_21_BPDU_OFFT, CORAL_REG_REV_21_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_21_PORT_FWD_OFFT, CORAL_REG_REV_21_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            rc = aml_readReg(unit, CORAL_RGAC4(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_REV_UN_MANG_OFFT, CORAL_REG_REV_UN_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_REV_UN_BPDU_OFFT, CORAL_REG_REV_UN_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_REV_UN_PORT_FWD_OFFT, CORAL_REG_REV_UN_PORT_FWD_LENGTH);
            }
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_LLDP:
            rc = aml_readReg(unit, CORAL_LCC(mac_port), &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                enable = BITS_OFF_R(u32dat, CORAL_REG_LLDP_MANG_OFFT, CORAL_REG_LLDP_MANG_LENGTH);
                as_bpdu = BITS_OFF_R(u32dat, CORAL_REG_LLDP_BPDU_OFFT, CORAL_REG_LLDP_BPDU_LENGTH);
                fwd = BITS_OFF_R(u32dat, CORAL_REG_LLDP_PORT_FWD_OFFT, CORAL_REG_LLDP_PORT_FWD_LENGTH);
            }
            break;
        default:
            break;
    }
    if (AIR_E_OK == rc)
    {
        if (enable)
        {
            ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        }
        else
        {
            ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        }
        if (as_bpdu)
        {
            ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else
        {
            ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        switch (fwd)
        {
            case HAL_CORAL_SWC_FWD_MODE_SYS_SETTING:
                ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
                break;
            case HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
                ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
                break;
            case HAL_CORAL_SWC_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
                ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
                break;
            case HAL_CORAL_SWC_FWD_MODE_CPU_ONLY:
                ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
                break;
            case HAL_CORAL_SWC_FWD_MODE_DROP:
                ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
                break;
            default:
                break;
        }
        rc = _hal_coral_swc_getPortMgmtFramePri(unit, mac_port, ptr_cfg);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setSystemMac
 * PURPOSE:
 *      Set the system MAC address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      mac             --  System MAC address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
hal_coral_swc_setSystemMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat;
    UI8_T          i;

    /* SMACCR 1 */
    u32dat = 0;
    for (i = 0; i < 2; i++)
    {
        u32dat |= BITS_OFF_L(mac[i], ((1 - i) * 8), 8);
    }
    rc = aml_writeReg(unit, CORAL_SMACCR1, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        /* SMACCR 0 */
        u32dat = 0;
        for (i = 2; i < 6; i++)
        {
            u32dat |= BITS_OFF_L(mac[i], ((5 - i) * 8), 8);
        }
        aml_writeReg(unit, CORAL_SMACCR0, &u32dat, sizeof(u32dat));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getSysMac
 * PURPOSE:
 *      Get the system MAC address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      mac             --  System MAC address
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
hal_coral_swc_getSystemMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat;
    UI8_T          i;

    /* SMACCR 1 */
    rc = aml_readReg(unit, CORAL_SMACCR1, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        for (i = 0; i < 2; i++)
        {
            mac[i] = BITS_OFF_R(u32dat, ((1 - i) * 8), 8);
        }

        /* SMACCR 0 */
        rc = aml_readReg(unit, CORAL_SMACCR0, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            for (i = 2; i < 6; i++)
            {
                mac[i] = BITS_OFF_R(u32dat, ((5 - i) * 8), 8);
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setJumboSize
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      frame_len       --  AIR_SWC_JUMBO_SIZE_1518,
 *                          AIR_SWC_JUMBO_SIZE_1536,
 *                          AIR_SWC_JUMBO_SIZE_1552,
 *                          AIR_SWC_JUMBO_SIZE_2048,
 *                          AIR_SWC_JUMBO_SIZE_3072,
 *                          AIR_SWC_JUMBO_SIZE_4096,
 *                          AIR_SWC_JUMBO_SIZE_5120,
 *                          AIR_SWC_JUMBO_SIZE_6144,
 *                          AIR_SWC_JUMBO_SIZE_7168,
 *                          AIR_SWC_JUMBO_SIZE_8192,
 *                          AIR_SWC_JUMBO_SIZE_9216,
 *                          AIR_SWC_JUMBO_SIZE_12288,
 *                          AIR_SWC_JUMBO_SIZE_15360,
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
hal_coral_swc_setJumboSize(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T frame_len)
{
    UI32_T         u32dat = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: frame_len = %d\n", frame_len);
    /* Read GMACCR register to get current info */
    rc = aml_readReg(unit, CORAL_GMACCR, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        /* Clear jumbo size info */
        u32dat &= ~BITS_RANGE(CORAL_MAX_RX_PKT_LEN_OFFT, CORAL_MAX_RX_PKT_LEN_LENG);
        u32dat &= ~BITS_RANGE(CORAL_MAX_RX_JUMBO_OFFT, CORAL_MAX_RX_JUMBO_LENG);

        if (frame_len < AIR_SWC_JUMBO_SIZE_2048)
        {
            u32dat |= frame_len;
        }
        else /* AIR_SWC_JUMBO_SIZE_2048 ~ AIR_SWC_JUMBO_SIZE_15360 */
        {
            u32dat |= HAL_CORAL_SWC_MAX_RX_JUMBO_EN;
            u32dat |= ((frame_len - HAL_CORAL_SWC_JUMBO_SIZE_OFFSET) << CORAL_MAX_RX_JUMBO_OFFT);
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: GMACCR value = %x\n", u32dat);
        rc = aml_writeReg(unit, CORAL_GMACCR, &u32dat, sizeof(u32dat));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getJumboSize
 * PURPOSE:
 *      Get accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_frame_len   --  AIR_SWC_JUMBO_SIZE_1518,
 *                          AIR_SWC_JUMBO_SIZE_1536,
 *                          AIR_SWC_JUMBO_SIZE_1552,
 *                          AIR_SWC_JUMBO_SIZE_2048,
 *                          AIR_SWC_JUMBO_SIZE_3072,
 *                          AIR_SWC_JUMBO_SIZE_4096,
 *                          AIR_SWC_JUMBO_SIZE_5120,
 *                          AIR_SWC_JUMBO_SIZE_6144,
 *                          AIR_SWC_JUMBO_SIZE_7168,
 *                          AIR_SWC_JUMBO_SIZE_8192,
 *                          AIR_SWC_JUMBO_SIZE_9216,
 *                          AIR_SWC_JUMBO_SIZE_12288,
 *                          AIR_SWC_JUMBO_SIZE_15360,
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getJumboSize(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_frame_len)
{
    UI32_T         u32dat, pkt_len;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Read jumbo size info */
    rc = aml_readReg(unit, CORAL_GMACCR, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        pkt_len = BITS_OFF_R(u32dat, CORAL_MAX_RX_PKT_LEN_OFFT, CORAL_MAX_RX_PKT_LEN_LENG);
        if (HAL_CORAL_SWC_MAX_RX_JUMBO_EN == pkt_len)
        {
            pkt_len = BITS_OFF_R(u32dat, CORAL_MAX_RX_JUMBO_OFFT, CORAL_MAX_RX_JUMBO_LENG);
            *ptr_frame_len = pkt_len + HAL_CORAL_SWC_JUMBO_SIZE_OFFSET;
        }
        else
        {
            *ptr_frame_len = pkt_len;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setProperty
 * PURPOSE:
 *      Set switch property.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      property        --  Select switch property
 *                          AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH
 *                          AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL
 *                          AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE
 *                          AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE
 *      param0          --  1: Enable 0: Disable
 *      param1          --  Reserved
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_setProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1)
{
    UI32_T         u32dat = 0, type = 0, enable = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            rc = aml_readReg(unit, CORAL_AAC, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AAC 0x%x = 0x%x\n", CORAL_AAC, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, param0, CORAL_AAC_AUTO_FLUSH_OFFSET, CORAL_AAC_AUTO_FLUSH_LENGTH);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AAC 0x%x = 0x%x\n", CORAL_AAC, u32dat);
                rc = aml_writeReg(unit, CORAL_AAC, &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            type = (1 == param0) ? CORAL_L1_RATE_IPG_BYTE_CNT : CORAL_L2_RATE_IPG_BYTE_CNT;
            /* set egress rate control */
            rc = aml_readReg(unit, CORAL_GERLCR, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GERLCR 0x%x = 0x%x\n", CORAL_GERLCR, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, type, CORAL_REG_IPG_BYTE_OFFT, CORAL_REG_IPG_BYTE_LENG);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write GERLCR 0x%x = 0x%x\n", CORAL_GERLCR, u32dat);
                rc = aml_writeReg(unit, CORAL_GERLCR, &u32dat, sizeof(u32dat));
            }
            if (AIR_E_OK != rc)
            {
                break;
            }

            /* set ingress rate control */
            rc = aml_readReg(unit, CORAL_GIRLCR, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GIRLCR 0x%x = 0x%x\n", CORAL_GIRLCR, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, type, CORAL_REG_IPG_BYTE_OFFT, CORAL_REG_IPG_BYTE_LENG);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write GIRLCR 0x%x = 0x%x\n", CORAL_GIRLCR, u32dat);
                rc = aml_writeReg(unit, CORAL_GIRLCR, &u32dat, sizeof(u32dat));
            }
            if (AIR_E_OK != rc)
            {
                break;
            }

            /* set compensation byte number include/exclude L1 size */
            rc = aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, type, CORAL_AGC_COMP_BNUM_OFFT, CORAL_AGC_COMP_BNUM_LENG);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                rc = aml_writeReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            rc = aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_AGC_ACLRATE_EXC_MG_OFFT, CORAL_AGC_ACLRATE_EXC_MG_LENG);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                rc = aml_writeReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            }
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            rc = aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                _HAL_CORAL_SWC_SET_VALUE(u32dat, enable, CORAL_AGC_BCSTRM_EXC_MG_OFFT, CORAL_AGC_BCSTRM_EXC_MG_LENG);
                DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
                rc = aml_writeReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            }
            break;
        default:
            break;
    }
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: hal_coral_swc_setProperty(%d) failed(%d)\n", property, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getProperty
 * PURPOSE:
 *      Get switch property.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      property        --  Select switch property
 *                          AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH
 *                          AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL
 *                          AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE
 *                          AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE
 *
 * OUTPUT:
 *      ptr_param0      --  1: Enable 0: Disable
 *      ptr_param1      --  Reserved
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1)
{
    UI32_T         u32dat = 0, type = 0, enable = 0;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            rc = aml_readReg(unit, CORAL_AAC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AAC 0x%x = 0x%x\n", CORAL_AAC, u32dat);
            *ptr_param0 = BITS_OFF_R(u32dat, CORAL_AAC_AUTO_FLUSH_OFFSET, CORAL_AAC_AUTO_FLUSH_LENGTH);
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            /* get egress rate control */
            rc = aml_readReg(unit, CORAL_GERLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GERLCR 0x%x = 0x%x\n", CORAL_GERLCR, u32dat);
            type = BITS_OFF_R(u32dat, CORAL_REG_IPG_BYTE_OFFT, CORAL_REG_IPG_BYTE_LENG);
            *ptr_param0 = (CORAL_L1_RATE_IPG_BYTE_CNT == type) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            rc = aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
            enable = BITS_OFF_R(u32dat, CORAL_AGC_ACLRATE_EXC_MG_OFFT, CORAL_AGC_ACLRATE_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            rc = aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", CORAL_AGC, u32dat);
            enable = BITS_OFF_R(u32dat, CORAL_AGC_BCSTRM_EXC_MG_OFFT, CORAL_AGC_BCSTRM_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        default:
            break;
    }
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: hal_coral_swc_getProperty(%d) failed(%d)\n", property, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setLoopDetectCtrl
 * PURPOSE:
 *      Set the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_setLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = CORAL_LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = CORAL_LPDETRXCR;
        /* workaround for E1 version, can not set RX loop detect to disable */
        if ((HAL_CORAL_SWC_LPDET_RX_CTRL_ISSUE_REVISION == HAL_DEVICE_REV_ID(unit)) && (FALSE == enable))
        {
            rc = AIR_E_OP_INVALID;
        }
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
            if (TRUE == enable)
            {
                u32dat |= (1 << mac_port);
            }
            else
            {
                u32dat &= ~(1 << mac_port);
            }
            rc = aml_writeReg(unit, reg_addr, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", reg_addr, u32dat);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getLoopDetectCtrl
 * PURPOSE:
 *      Get the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                         *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = CORAL_LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = CORAL_LPDETRXCR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", reg_addr, u32dat);
            HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
            *ptr_enable = (TRUE == BITS_OFF_R(u32dat, mac_port, 1)) ? TRUE : FALSE;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_setLoopDetectFrameSrcMac
 * PURPOSE:
 *      Set the loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
hal_coral_swc_setLoopDetectFrameSrcMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    /* Check and set source mac */
    rc = _hal_coral_swc_checkMacAddr(mac);
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, CORAL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~CORAL_LPDET_SMAC_MASK;
            u32dat |= BITS_OFF_L(mac[0], 8, 8);
            u32dat |= BITS_OFF_L(mac[1], 0, 8);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", CORAL_LPDET_SA_MSB, u32dat);
            rc = aml_writeReg(unit, CORAL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                rc = aml_readReg(unit, CORAL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    u32dat = BITS_OFF_L(mac[2], 24, 8);
                    u32dat |= BITS_OFF_L(mac[3], 16, 8);
                    u32dat |= BITS_OFF_L(mac[4], 8, 8);
                    u32dat |= BITS_OFF_L(mac[5], 0, 8);
                    ;
                    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", CORAL_LPDET_SA_LSB, u32dat);
                    rc = aml_writeReg(unit, CORAL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
                }
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getLoopDetectFrameSrcMac
 * PURPOSE:
 *      Get loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      mac                      -- Source MAC address
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
hal_coral_swc_getLoopDetectFrameSrcMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    rc = aml_readReg(unit, CORAL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", CORAL_LPDET_SA_MSB, u32dat);
        /* Get source mac address */
        mac[0] = BITS_OFF_R(u32dat, 8, 8);
        mac[1] = BITS_OFF_R(u32dat, 0, 8);

        rc = aml_readReg(unit, CORAL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            mac[2] = BITS_OFF_R(u32dat, 24, 8);
            mac[3] = BITS_OFF_R(u32dat, 16, 8);
            mac[4] = BITS_OFF_R(u32dat, 8, 8);
            mac[5] = BITS_OFF_R(u32dat, 0, 8);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", CORAL_LPDET_SA_LSB, u32dat);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_clearLoopDetectStatus
 * PURPOSE:
 *      Clear the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_clearLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            reg_addr = 0;
    AIR_PORT_BITMAP_T setPbm = {0};

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = CORAL_LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = CORAL_LPDETRXSR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        HAL_AIR_PBMP_TO_MAC_PBMP(unit, port_bitmap, setPbm);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", reg_addr, setPbm[0]);
        rc = aml_writeReg(unit, reg_addr, &setPbm[0], sizeof(UI32_T));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getLoopDetectStatus
 * PURPOSE:
 *      Get the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            reg_addr = 0;
    AIR_PORT_BITMAP_T getPbm = {0};

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = CORAL_LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = CORAL_LPDETRXSR;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, reg_addr, &(getPbm[0]), sizeof(UI32_T));
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", reg_addr, getPbm[0]);
        if (AIR_E_OK == rc)
        {
            HAL_MAC_PBMP_TO_AIR_PBMP(unit, getPbm, port_bitmap);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getGlobalFreePages
 * PURPOSE:
 *      Get the free page link counter
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_fp_cnt      --  Free page counter
 *      ptr_min_fp_cnt  --  Minimal Free page counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getGlobalFreePages(
    const UI32_T unit,
    UI32_T      *ptr_fp_cnt,
    UI32_T      *ptr_min_fp_cnt)
{
    UI32_T         u32dat;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_fp_cnt);
    HAL_CHECK_PTR(ptr_min_fp_cnt);

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, CORAL_FPLC, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_fp_cnt = BITS_OFF_R(u32dat, CORAL_FPLC_FREE_PL_CNT_OFFT, CORAL_FPLC_FREE_PL_CNT_LENG);
        *ptr_min_fp_cnt = BITS_OFF_R(u32dat, CORAL_FPLC_MIN_FREE_PL_CNT_OFFT, CORAL_FPLC_MIN_FREE_PL_CNT_LENG);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read FPLC failed(%d)\n", rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getPortAllocatedPages
 * PURPOSE:
 *      Get the allocated free page counter in RxCtrl of specific port
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_fp_cnt      --  Free page counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getPortAllocatedPages(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_fp_cnt)
{
    UI32_T         mac_port;
    UI32_T         u32dat;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_fp_cnt);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, CORAL_FPC_RXCTRL(mac_port), &u32dat, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        *ptr_fp_cnt = BITS_OFF_R(u32dat, CORAL_FPC_RXCTRL_FP_CNT_OFFT, CORAL_FPC_RXCTRL_FP_CNT_LENG);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read FPC_RXCTRL(0x%08X) failed(%d)\n", CORAL_FPC_RXCTRL(mac_port), rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getPortUsedPages
 * PURPOSE:
 *      Get the used resource counter of specific port/queue
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      queue           --  Select queue index
 *      mode            --  0: Page counter
 *                          1: Packet counter
 * OUTPUT:
 *      ptr_cnt         --  Used resource counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      To get total number of page of the port
 *      when queue number = 8, not support total packet number
 */
AIR_ERROR_NO_T
hal_coral_swc_getPortUsedPages(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T queue,
    const UI32_T mode,
    UI32_T      *ptr_cnt)
{
    UI32_T         mac_port;
    UI32_T         u32dat;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(queue, AIR_QOS_QUEUE_MAX_NUM);
    HAL_CHECK_ENUM_RANGE(mode, 2);
    HAL_CHECK_PTR(ptr_cnt);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Set Packet/Page counter selector */
    u32dat = (1 == mode) ? BIT(CORAL_PCPCR_PACKET_SEL_OFFT) : 0;
    u32dat |= BITS_OFF_L(mac_port, CORAL_PCPCR_PORT_SEL_OFFT, CORAL_PCPCR_PORT_SEL_LENG);
    u32dat |= BITS_OFF_L(queue, CORAL_PCPCR_QUEUE_SEL_OFFT, CORAL_PCPCR_QUEUE_SEL_LENG);

    rc = aml_writeReg(unit, CORAL_PCPCR, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        /* Get Packet/Page counter probe status */
        rc = aml_readReg(unit, CORAL_PCPSR, &u32dat, sizeof(UI32_T));
        if (AIR_E_OK == rc)
        {
            *ptr_cnt = BITS_OFF_R(u32dat, CORAL_PCPSR_COUNTER_OFFT, CORAL_PCPSR_COUNTER_LENG);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read PCPSR failed(%d)\n", rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: write PCPCR=0x%x failed(%d)\n", u32dat, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getCapacity
 * PURPOSE:
 *      Get the specific type resource capacity
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
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_SWC_RSRC_LAST);
    HAL_CHECK_PTR(ptr_size);

    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
        case AIR_SWC_RSRC_ACL_ACTION:
        case AIR_SWC_RSRC_ACL_UDF:
        case AIR_SWC_RSRC_ACL_COUNTER:
        case AIR_SWC_RSRC_ACL_METER:
        case AIR_SWC_RSRC_ACL_TRTCM:
            rc = hal_coral_acl_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_DOS_RATELIMIT:
            rc = hal_coral_dos_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = hal_coral_lag_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_coral_l2_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_coral_ipmc_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_SVLAN:
            rc = hal_coral_svlan_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = hal_coral_vlan_getCapacity(unit, type, param, ptr_size);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getUsage
 * PURPOSE:
 *      Get the specific type resource usage
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
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(type, AIR_SWC_RSRC_LAST);
    HAL_CHECK_PTR(ptr_cnt);

    switch (type)
    {
        case AIR_SWC_RSRC_ACL_RULE:
        case AIR_SWC_RSRC_ACL_ACTION:
        case AIR_SWC_RSRC_ACL_UDF:
        case AIR_SWC_RSRC_ACL_COUNTER:
        case AIR_SWC_RSRC_ACL_METER:
        case AIR_SWC_RSRC_ACL_TRTCM:
            rc = hal_coral_acl_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_DOS_RATELIMIT:
            rc = hal_coral_dos_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = hal_coral_lag_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_coral_l2_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_coral_ipmc_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_SVLAN:
            rc = hal_coral_svlan_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = hal_coral_vlan_getUsage(unit, type, param, ptr_cnt);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_swc_getChipUid
 * PURPOSE:
 *      Get the chip unique ID
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_uid         --  Chip unique ID

 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_swc_getChipUid(
    const UI32_T unit,
    UI64_T      *ptr_uid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    rc = aml_readReg(unit, CORAL_REG_HUID_HIGH, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        UI64_ASSIGN(*ptr_uid, u32dat, 0);
        rc = aml_readReg(unit, CORAL_REG_HUID_LOW, &u32dat, sizeof(UI32_T));
        if (AIR_E_OK == rc)
        {
            UI64_OR(*ptr_uid, u32dat);
        }
    }
    if (0 == *ptr_uid)
    {
        rc = AIR_E_OTHERS;
    }
    return rc;
}
