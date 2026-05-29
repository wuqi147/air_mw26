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

/* FILE NAME:  hal_pearl_swc.c
 * PURPOSE:
 *  Implement switch module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/pearl/hal_pearl_swc.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_mdio.h>
#include <hal/switch/pearl/hal_pearl_acl.h>
#include <hal/switch/pearl/hal_pearl_dos.h>
#include <hal/switch/pearl/hal_pearl_ipmc.h>
#include <hal/switch/pearl/hal_pearl_l2.h>
#include <hal/switch/pearl/hal_pearl_lag.h>
#include <hal/switch/pearl/hal_pearl_reg.h>
#include <hal/switch/pearl/hal_pearl_svlan.h>
#include <hal/switch/pearl/hal_pearl_vlan.h>
#include <osal/osal_lib.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_PEARL_SWC_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                     \
    {                                                                      \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));              \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));    \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

#define MAX_RX_JUMBO_LEN (0x3)

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SWC, "hal_pearl_swc.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
/* FUNCTION NAME: _hal_pearl_swc_checkMacAddr
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
_hal_pearl_swc_checkMacAddr(
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
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_pearl_swc_setMgmtFrameCfg
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
hal_pearl_swc_setMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         enable = 0, pri_high = 0, fwd = 0, as_bpdu = 0;

    HAL_CHECK_ENUM_RANGE(ptr_cfg->frame_type, AIR_SWC_MGMT_FRAME_TYPE_LAST);
    HAL_CHECK_ENUM_RANGE(ptr_cfg->forward_mode, AIR_SWC_MGMT_FWD_MODE_LAST);

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE)
    {
        enable = 1;
    }
    else
    {
        enable = 0;
    }

    if (ptr_cfg->flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH)
    {
        pri_high = 1;
    }
    else
    {
        pri_high = 0;
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
            fwd = HAL_PEARL_FWD_MODE_SYS_SETTING;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
            fwd = HAL_PEARL_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
            fwd = HAL_PEARL_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_CPU_ONLY:
            fwd = HAL_PEARL_FWD_MODE_CPU_ONLY;
            break;
        case AIR_SWC_MGMT_FWD_MODE_DROP:
            fwd = HAL_PEARL_FWD_MODE_DROP;
            break;
        default:
            break;
    }

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            aml_readReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_IGMP_MANG_OFFT, PEARL_REG_IGMP_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_IGMP_BPDU_OFFT, PEARL_REG_IGMP_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_IGMP_PRI_HIGH_OFFT, PEARL_REG_IGMP_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_IGMP_PORT_FWD_OFFT, PEARL_REG_IGMP_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            aml_readReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_PPPOE_MANG_OFFT, PEARL_REG_PPPOE_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_PPPOE_BPDU_OFFT, PEARL_REG_PPPOE_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_PPPOE_PRI_HIGH_OFFT, PEARL_REG_PPPOE_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_PPPOE_PORT_FWD_OFFT, PEARL_REG_PPPOE_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            aml_readReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_ARP_MANG_OFFT, PEARL_REG_ARP_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_ARP_BPDU_OFFT, PEARL_REG_ARP_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_ARP_PRI_HIGH_OFFT, PEARL_REG_ARP_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_ARP_PORT_FWD_OFFT, PEARL_REG_ARP_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            aml_readReg(unit, PEARL_PAC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_TAG_PAE_MANG_OFFT, PEARL_REG_TAG_PAE_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_TAG_PAE_BPDU_OFFT, PEARL_REG_TAG_PAE_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_TAG_PAE_PRI_HIGH_OFFT,
                                     PEARL_REG_TAG_PAE_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_TAG_PAE_PORT_FWD_OFFT, PEARL_REG_TAG_PAE_PORT_FWD_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_UTAG_PAE_MANG_OFFT, PEARL_REG_UTAG_PAE_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_UTAG_PAE_BPDU_OFFT, PEARL_REG_UTAG_PAE_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_UTAG_PAE_PRI_HIGH_OFFT,
                                     PEARL_REG_UTAG_PAE_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_UTAG_PAE_PORT_FWD_OFFT, PEARL_REG_UTAG_PAE_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_PAC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            aml_readReg(unit, PEARL_DHCP, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_DHCP_6_MANG_OFFT, PEARL_REG_DHCP_6_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_DHCP_6_BPDU_OFFT, PEARL_REG_DHCP_6_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_DHCP_6_PRI_HIGH_OFFT,
                                     PEARL_REG_DHCP_6_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_DHCP_6_PORT_FWD_OFFT, PEARL_REG_DHCP_6_PORT_FWD_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_DHCP_4_MANG_OFFT, PEARL_REG_DHCP_4_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_DHCP_4_BPDU_OFFT, PEARL_REG_DHCP_4_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_DHCP_4_PRI_HIGH_OFFT,
                                     PEARL_REG_DHCP_4_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_DHCP_4_PORT_FWD_OFFT, PEARL_REG_DHCP_4_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_DHCP, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            aml_readReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_TTL_0_MANG_OFFT, PEARL_REG_TTL_0_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_TTL_0_BPDU_OFFT, PEARL_REG_TTL_0_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_TTL_0_PRI_HIGH_OFFT, PEARL_REG_TTL_0_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_TTL_0_PORT_FWD_OFFT, PEARL_REG_TTL_0_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            aml_readReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_BPDU_MANG_OFFT, PEARL_REG_BPDU_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_BPDU_BPDU_OFFT, PEARL_REG_BPDU_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_BPDU_PRI_HIGH_OFFT, PEARL_REG_BPDU_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_BPDU_PORT_FWD_OFFT, PEARL_REG_BPDU_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            aml_readReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_MLD_MANG_OFFT, PEARL_REG_MLD_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_MLD_BPDU_OFFT, PEARL_REG_MLD_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_MLD_PRI_HIGH_OFFT, PEARL_REG_MLD_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_MLD_PORT_FWD_OFFT, PEARL_REG_MLD_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            aml_readReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_01_MANG_OFFT, PEARL_REG_REV_01_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_01_BPDU_OFFT, PEARL_REG_REV_01_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_01_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_01_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_01_PORT_FWD_OFFT, PEARL_REG_REV_01_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            aml_readReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_02_MANG_OFFT, PEARL_REG_REV_02_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_02_BPDU_OFFT, PEARL_REG_REV_02_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_02_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_02_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_02_PORT_FWD_OFFT, PEARL_REG_REV_02_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            aml_readReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_03_MANG_OFFT, PEARL_REG_REV_03_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_03_BPDU_OFFT, PEARL_REG_REV_03_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_03_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_03_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_03_PORT_FWD_OFFT, PEARL_REG_REV_03_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            aml_readReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_0E_MANG_OFFT, PEARL_REG_REV_0E_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_0E_BPDU_OFFT, PEARL_REG_REV_0E_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_0E_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_0E_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_0E_PORT_FWD_OFFT, PEARL_REG_REV_0E_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            aml_readReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_10_MANG_OFFT, PEARL_REG_REV_10_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_10_BPDU_OFFT, PEARL_REG_REV_10_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_10_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_10_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_10_PORT_FWD_OFFT, PEARL_REG_REV_10_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            aml_readReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_20_MANG_OFFT, PEARL_REG_REV_20_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_20_BPDU_OFFT, PEARL_REG_REV_20_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_20_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_20_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_20_PORT_FWD_OFFT, PEARL_REG_REV_20_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            aml_readReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_21_MANG_OFFT, PEARL_REG_REV_21_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_21_BPDU_OFFT, PEARL_REG_REV_21_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_21_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_21_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_21_PORT_FWD_OFFT, PEARL_REG_REV_21_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            aml_readReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_REG_REV_UN_MANG_OFFT, PEARL_REG_REV_UN_MANG_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, as_bpdu, PEARL_REG_REV_UN_BPDU_OFFT, PEARL_REG_REV_UN_BPDU_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, pri_high, PEARL_REG_REV_UN_PRI_HIGH_OFFT,
                                     PEARL_REG_REV_UN_PRI_HIGH_LENGTH);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, fwd, PEARL_REG_REV_UN_PORT_FWD_OFFT, PEARL_REG_REV_UN_PORT_FWD_LENGTH);
            aml_writeReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            break;
        default:
            break;
    }
    return ret;
}

/* FUNCTION NAME: hal_pearl_swc_getMgmtFrameCfg
 * PURPOSE:
 *     Get management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
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
hal_pearl_swc_getMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T         u32dat = 0;
    UI32_T         enable = 0, pri_high = 0, fwd = 0, as_bpdu = 0;
    HAL_CHECK_ENUM_RANGE(ptr_cfg->frame_type, AIR_SWC_MGMT_FRAME_TYPE_LAST);

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            aml_readReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_IGMP_MANG_OFFT, PEARL_REG_IGMP_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_IGMP_BPDU_OFFT, PEARL_REG_IGMP_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_IGMP_PRI_HIGH_OFFT, PEARL_REG_IGMP_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_IGMP_PORT_FWD_OFFT, PEARL_REG_IGMP_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            aml_readReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_PPPOE_MANG_OFFT, PEARL_REG_PPPOE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_PPPOE_BPDU_OFFT, PEARL_REG_PPPOE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_PPPOE_PRI_HIGH_OFFT, PEARL_REG_PPPOE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_PPPOE_PORT_FWD_OFFT, PEARL_REG_PPPOE_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            aml_readReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_ARP_MANG_OFFT, PEARL_REG_ARP_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_ARP_BPDU_OFFT, PEARL_REG_ARP_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_ARP_PRI_HIGH_OFFT, PEARL_REG_ARP_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_ARP_PORT_FWD_OFFT, PEARL_REG_ARP_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            aml_readReg(unit, PEARL_PAC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_TAG_PAE_MANG_OFFT, PEARL_REG_TAG_PAE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_TAG_PAE_BPDU_OFFT, PEARL_REG_TAG_PAE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_TAG_PAE_PRI_HIGH_OFFT, PEARL_REG_TAG_PAE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_TAG_PAE_PORT_FWD_OFFT, PEARL_REG_TAG_PAE_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, PEARL_REG_UTAG_PAE_MANG_OFFT, PEARL_REG_UTAG_PAE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_UTAG_PAE_BPDU_OFFT, PEARL_REG_UTAG_PAE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_UTAG_PAE_PRI_HIGH_OFFT, PEARL_REG_UTAG_PAE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_UTAG_PAE_PORT_FWD_OFFT, PEARL_REG_UTAG_PAE_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            aml_readReg(unit, PEARL_DHCP, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_DHCP_6_MANG_OFFT, PEARL_REG_DHCP_6_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_DHCP_6_BPDU_OFFT, PEARL_REG_DHCP_6_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_DHCP_6_PRI_HIGH_OFFT, PEARL_REG_DHCP_6_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_DHCP_6_PORT_FWD_OFFT, PEARL_REG_DHCP_6_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, PEARL_REG_DHCP_4_MANG_OFFT, PEARL_REG_DHCP_4_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_DHCP_4_BPDU_OFFT, PEARL_REG_DHCP_4_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_DHCP_4_PRI_HIGH_OFFT, PEARL_REG_DHCP_4_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_DHCP_4_PORT_FWD_OFFT, PEARL_REG_DHCP_4_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            aml_readReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_TTL_0_MANG_OFFT, PEARL_REG_TTL_0_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_TTL_0_BPDU_OFFT, PEARL_REG_TTL_0_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_TTL_0_PRI_HIGH_OFFT, PEARL_REG_TTL_0_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_TTL_0_PORT_FWD_OFFT, PEARL_REG_TTL_0_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            aml_readReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_BPDU_MANG_OFFT, PEARL_REG_BPDU_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_BPDU_BPDU_OFFT, PEARL_REG_BPDU_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_BPDU_PRI_HIGH_OFFT, PEARL_REG_BPDU_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_BPDU_PORT_FWD_OFFT, PEARL_REG_BPDU_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            aml_readReg(unit, PEARL_IMMC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_MLD_MANG_OFFT, PEARL_REG_MLD_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_MLD_BPDU_OFFT, PEARL_REG_MLD_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_MLD_PRI_HIGH_OFFT, PEARL_REG_MLD_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_MLD_PORT_FWD_OFFT, PEARL_REG_MLD_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            aml_readReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_01_MANG_OFFT, PEARL_REG_REV_01_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_01_BPDU_OFFT, PEARL_REG_REV_01_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_01_PRI_HIGH_OFFT, PEARL_REG_REV_01_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_01_PORT_FWD_OFFT, PEARL_REG_REV_01_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            aml_readReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_02_MANG_OFFT, PEARL_REG_REV_02_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_02_BPDU_OFFT, PEARL_REG_REV_02_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_02_PRI_HIGH_OFFT, PEARL_REG_REV_02_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_02_PORT_FWD_OFFT, PEARL_REG_REV_02_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            aml_readReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_03_MANG_OFFT, PEARL_REG_REV_03_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_03_BPDU_OFFT, PEARL_REG_REV_03_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_03_PRI_HIGH_OFFT, PEARL_REG_REV_03_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_03_PORT_FWD_OFFT, PEARL_REG_REV_03_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            aml_readReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_0E_MANG_OFFT, PEARL_REG_REV_0E_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_0E_BPDU_OFFT, PEARL_REG_REV_0E_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_0E_PRI_HIGH_OFFT, PEARL_REG_REV_0E_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_0E_PORT_FWD_OFFT, PEARL_REG_REV_0E_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            aml_readReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_10_MANG_OFFT, PEARL_REG_REV_10_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_10_BPDU_OFFT, PEARL_REG_REV_10_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_10_PRI_HIGH_OFFT, PEARL_REG_REV_10_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_10_PORT_FWD_OFFT, PEARL_REG_REV_10_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            aml_readReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_20_MANG_OFFT, PEARL_REG_REV_20_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_20_BPDU_OFFT, PEARL_REG_REV_20_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_20_PRI_HIGH_OFFT, PEARL_REG_REV_20_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_20_PORT_FWD_OFFT, PEARL_REG_REV_20_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            aml_readReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_21_MANG_OFFT, PEARL_REG_REV_21_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_21_BPDU_OFFT, PEARL_REG_REV_21_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_21_PRI_HIGH_OFFT, PEARL_REG_REV_21_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_21_PORT_FWD_OFFT, PEARL_REG_REV_21_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            aml_readReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, PEARL_REG_REV_UN_MANG_OFFT, PEARL_REG_REV_UN_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, PEARL_REG_REV_UN_BPDU_OFFT, PEARL_REG_REV_UN_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, PEARL_REG_REV_UN_PRI_HIGH_OFFT, PEARL_REG_REV_UN_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, PEARL_REG_REV_UN_PORT_FWD_OFFT, PEARL_REG_REV_UN_PORT_FWD_LENGTH);
            break;
        default:
            break;
    }
    if (1 == enable)
    {
        ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
    }
    else
    {
        ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
    }

    if (1 == pri_high)
    {
        ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
    }
    else
    {
        ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
    }

    if (1 == as_bpdu)
    {
        ptr_cfg->flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
    }
    else
    {
        ptr_cfg->flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
    }

    switch (fwd)
    {
        case HAL_PEARL_FWD_MODE_SYS_SETTING:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
            break;
        case HAL_PEARL_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            break;
        case HAL_PEARL_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            break;
        case HAL_PEARL_FWD_MODE_CPU_ONLY:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
            break;
        case HAL_PEARL_FWD_MODE_DROP:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
            break;
        default:
            break;
    }

    return ret;
}

/* FUNCTION NAME: hal_pearl_swc_setSystemMac
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
hal_pearl_swc_setSystemMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    UI32_T u32dat;
    UI8_T  i;

    /* SMACCR 1 */
    u32dat = 0;
    for (i = 0; i < 2; i++)
    {
        u32dat |= BITS_OFF_L(mac[i], ((1 - i) * 8), 8);
    }
    aml_writeReg(unit, PEARL_SMACCR1, &u32dat, sizeof(u32dat));

    /* SMACCR 0 */
    u32dat = 0;
    for (i = 2; i < 6; i++)
    {
        u32dat |= BITS_OFF_L(mac[i], ((5 - i) * 8), 8);
    }
    aml_writeReg(unit, PEARL_SMACCR0, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_getSysMac
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
hal_pearl_swc_getSystemMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    UI32_T u32dat;
    UI8_T  i;

    /* SMACCR 1 */
    aml_readReg(unit, PEARL_SMACCR1, &u32dat, sizeof(u32dat));
    for (i = 0; i < 2; i++)
    {
        mac[i] = BITS_OFF_R(u32dat, ((1 - i) * 8), 8);
    }

    /* SMACCR 0 */
    aml_readReg(unit, PEARL_SMACCR0, &u32dat, sizeof(u32dat));
    for (i = 2; i < 6; i++)
    {
        mac[i] = BITS_OFF_R(u32dat, ((5 - i) * 8), 8);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_pearl_swc_init
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
hal_pearl_swc_init(
    const UI32_T unit)
{
    UI32_T                    u32dat = 0;
    UI16_T                    u16dat = 0;
    AIR_PORT_BITMAP_T         pbm = {0}, mac_pbm = {0};
    AIR_SWC_MGMT_FRAME_CFG_T  mgmt_cfg;
    AIR_SWC_MGMT_FRAME_TYPE_T mgmt_type;
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
            case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
                mgmt_cfg.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
                break;
            default:
                mgmt_cfg.flags = AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE + AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        }
        hal_pearl_swc_setMgmtFrameCfg(unit, &mgmt_cfg);
    }

    /* Designate the following packet as a BPDU frame */
    aml_readReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_REG_REV_02_BPDU_OFFT);
    aml_writeReg(unit, PEARL_RGAC1, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_REG_REV_0E_BPDU_OFFT);
    u32dat |= BIT(PEARL_REG_REV_03_BPDU_OFFT);
    aml_writeReg(unit, PEARL_RGAC2, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_REG_REV_20_BPDU_OFFT);
    aml_writeReg(unit, PEARL_RGAC3, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_REG_REV_21_BPDU_OFFT);
    aml_writeReg(unit, PEARL_RGAC4, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_REG_BPDU_BPDU_OFFT);
    aml_writeReg(unit, PEARL_BPC, &u32dat, sizeof(u32dat));

    /* Enable auto flush */
    aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
    u32dat |= BIT(PEARL_AAC_AUTO_FLUSH_OFFSET);
    aml_writeReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));

    /* Set ARP forward including CPU  */
    aml_readReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));
    _HAL_PEARL_SWC_SET_VALUE(u32dat, HAL_PEARL_FWD_MODE_SYS_SETTING_INCLUDE_CPU, PEARL_REG_ARP_PORT_FWD_OFFT,
                             PEARL_REG_ARP_PORT_FWD_LENGTH);
    aml_writeReg(unit, PEARL_APC, &u32dat, sizeof(u32dat));

    /* Set mtcc */
    osal_memset(&mtcc_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT, &mtcc_cfg);
    aml_readReg(unit, PEARL_GMACCR, &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(PEARL_GMACCR_MTCC_OFFT, PEARL_GMACCR_MTCC_LENG);
    u32dat |= ((mtcc_cfg.value << PEARL_GMACCR_MTCC_OFFT) & BITS_RANGE(PEARL_GMACCR_MTCC_OFFT, PEARL_GMACCR_MTCC_LENG));
    aml_writeReg(unit, PEARL_GMACCR, &u32dat, sizeof(u32dat));

    AIR_PORT_BITMAP_COPY(pbm, HAL_PORT_BMP_ETH(unit));
    if (HAL_IS_PORT_VALID(unit, HAL_CPU_PORT(unit)))
    {
        AIR_PORT_DEL(pbm, HAL_CPU_PORT(unit));
    }
    HAL_AIR_PBMP_TO_MAC_PBMP(unit, pbm, mac_pbm);

    /* Set IGMP query message forwarding port bitmap */
    aml_writeReg(unit, PEARL_DRP, &mac_pbm[0], sizeof(UI32_T));

    /* Set IGMP router port bitmap */
    aml_writeReg(unit, PEARL_QRYP, &mac_pbm[0], sizeof(UI32_T));

    /* SLED clock rate fine tune; refresh 60Hz, clock 2.08Mhz */
    u32dat = (35000 << 16) + 12;
    aml_writeReg(unit, PEARL_SLED_CTRL1, &u32dat, sizeof(u32dat));

    /* Set default rate limit include preamble/IPG/CRC */
    aml_readReg(unit, PEARL_GERLCR, &u32dat, sizeof(u32dat));
    _HAL_PEARL_SWC_SET_VALUE(u32dat, PEARL_L1_RATE_IPG_BYTE_CNT, PEARL_REG_IPG_BYTE_OFFT, PEARL_REG_IPG_BYTE_LENG);
    aml_writeReg(unit, PEARL_GERLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_GIRLCR, &u32dat, sizeof(u32dat));
    _HAL_PEARL_SWC_SET_VALUE(u32dat, PEARL_L1_RATE_IPG_BYTE_CNT, PEARL_REG_IPG_BYTE_OFFT, PEARL_REG_IPG_BYTE_LENG);
    aml_writeReg(unit, PEARL_GIRLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
    _HAL_PEARL_SWC_SET_VALUE(u32dat, PEARL_L1_RATE_IPG_BYTE_CNT, PEARL_AGC_COMP_BNUM_OFFT, PEARL_AGC_COMP_BNUM_LENG);
    aml_writeReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));

    /* Set default value about loop related register */
    osal_memset(&ethtype_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    ethtype_cfg.value = AIR_SWC_LPDET_ETH_TYPE_DEFAULT;
    hal_cfg_getValue(unit, AIR_CFG_TYPE_LPDET_ETH_TYPE, &ethtype_cfg);
    u16dat = ethtype_cfg.value;
    /* set ldf ether type */
    aml_readReg(unit, PEARL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(PEARL_LPDET_FRAME_TYPE_OFFT, PEARL_LPDET_FRAME_TYPE_LENG);
    u32dat |= BITS_OFF_L(u16dat, PEARL_LPDET_FRAME_TYPE_OFFT, PEARL_LPDET_FRAME_TYPE_LENG);
    aml_writeReg(unit, PEARL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));

    /* set ldf source mac */
    hal_pearl_swc_getSystemMac(unit, loop_src_mac);
    hal_pearl_swc_setLoopDetectFrameSrcMac(unit, loop_src_mac);

    /* set over_rxpause = 1, rate_1s = 1 */
    aml_readReg(unit, PEARL_LPDETCR, &u32dat, sizeof(u32dat));
    u32dat |= (PEARL_LPDETCR_OVER_RXPAUSE | PEARL_LPDETCR_PERIOD_1S);
    aml_writeReg(unit, PEARL_LPDETCR, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_setJumboSize
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
hal_pearl_swc_setJumboSize(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T frame_len)
{
    UI32_T u32dat = 0;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: frame_len = %d\n", frame_len);
    /* Read and clear jumbo frame info */
    aml_readReg(unit, PEARL_GMACCR, &u32dat, sizeof(u32dat));

    u32dat &= ~BITS_RANGE(PEARL_MAX_RX_PKT_LEN_OFFT, PEARL_MAX_RX_PKT_LEN_LENG);
    u32dat &= ~BITS_RANGE(PEARL_MAX_RX_JUMBO_OFFT, PEARL_MAX_RX_JUMBO_LENG);

    if (frame_len < MAX_RX_JUMBO_LEN)
    {
        u32dat |= frame_len;
    }
    else /* AIR_SWC_JUMBO_SIZE_2048 ~ AIR_SWC_JUMBO_SIZE_15360*/
    {
        u32dat |= MAX_RX_JUMBO_LEN;
        u32dat |= (((frame_len - AIR_SWC_JUMBO_SIZE_2048) + 2) << PEARL_MAX_RX_JUMBO_OFFT);
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: PEARL_GMACCR value = %x\n", u32dat);
    aml_writeReg(unit, PEARL_GMACCR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_getJumboSize
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
hal_pearl_swc_getJumboSize(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_frame_len)
{
    UI32_T u32dat = 0, pkt_len;

    /* Read and clear jumbo frame info */
    aml_readReg(unit, PEARL_GMACCR, &u32dat, sizeof(u32dat));

    pkt_len = BITS_OFF_R(u32dat, PEARL_MAX_RX_PKT_LEN_OFFT, PEARL_MAX_RX_PKT_LEN_LENG);

    if (pkt_len < MAX_RX_JUMBO_LEN)
    {
        *ptr_frame_len = pkt_len;
    }
    else
    {
        pkt_len = BITS_OFF_R(u32dat, PEARL_MAX_RX_JUMBO_OFFT, PEARL_MAX_RX_JUMBO_LENG);
        *ptr_frame_len = pkt_len + 1;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_setProperty
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
hal_pearl_swc_setProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1)
{
    UI32_T u32dat = 0, type = 0, enable = 0;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AAC 0x%x = 0x%x\n", PEARL_AAC, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, param0, PEARL_AAC_AUTO_FLUSH_OFFSET, PEARL_AAC_AUTO_FLUSH_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_AAC 0x%x = 0x%x\n", PEARL_AAC, u32dat);
            aml_writeReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            type = (1 == param0) ? PEARL_L1_RATE_IPG_BYTE_CNT : PEARL_L2_RATE_IPG_BYTE_CNT;
            /* set egress rate control */
            aml_readReg(unit, PEARL_GERLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_GERLCR 0x%x = 0x%x\n", PEARL_GERLCR, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, type, PEARL_REG_IPG_BYTE_OFFT, PEARL_REG_IPG_BYTE_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_GERLCR 0x%x = 0x%x\n", PEARL_GERLCR, u32dat);
            aml_writeReg(unit, PEARL_GERLCR, &u32dat, sizeof(u32dat));

            /* set ingress rate control */
            aml_readReg(unit, PEARL_GIRLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_GIRLCR 0x%x = 0x%x\n", PEARL_GIRLCR, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, type, PEARL_REG_IPG_BYTE_OFFT, PEARL_REG_IPG_BYTE_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_GIRLCR 0x%x = 0x%x\n", PEARL_GIRLCR, u32dat);
            aml_writeReg(unit, PEARL_GIRLCR, &u32dat, sizeof(u32dat));

            /* set compensation byte number include/exclude L1 size */
            aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, type, PEARL_AGC_COMP_BNUM_OFFT, PEARL_AGC_COMP_BNUM_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            aml_writeReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));

            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_AGC_ACLRATE_EXC_MG_OFFT, PEARL_AGC_ACLRATE_EXC_MG_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            aml_writeReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            _HAL_PEARL_SWC_SET_VALUE(u32dat, enable, PEARL_AGC_BCSTRM_EXC_MG_OFFT, PEARL_AGC_BCSTRM_EXC_MG_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            aml_writeReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            break;
        default:
            break;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_getProperty
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
hal_pearl_swc_getProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1)
{
    UI32_T u32dat = 0, type = 0, enable = 0;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            aml_readReg(unit, PEARL_AAC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AAC 0x%x = 0x%x\n", PEARL_AAC, u32dat);
            *ptr_param0 = BITS_OFF_R(u32dat, PEARL_AAC_AUTO_FLUSH_OFFSET, PEARL_AAC_AUTO_FLUSH_LENGTH);
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            /* get egress rate control */
            aml_readReg(unit, PEARL_GERLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_GERLCR 0x%x = 0x%x\n", PEARL_GERLCR, u32dat);
            type = BITS_OFF_R(u32dat, PEARL_REG_IPG_BYTE_OFFT, PEARL_REG_IPG_BYTE_LENG);
            *ptr_param0 = (PEARL_L1_RATE_IPG_BYTE_CNT == type) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            enable = BITS_OFF_R(u32dat, PEARL_AGC_ACLRATE_EXC_MG_OFFT, PEARL_AGC_ACLRATE_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            aml_readReg(unit, PEARL_AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read PEARL_AGC 0x%x = 0x%x\n", PEARL_AGC, u32dat);
            enable = BITS_OFF_R(u32dat, PEARL_AGC_BCSTRM_EXC_MG_OFFT, PEARL_AGC_BCSTRM_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        default:
            break;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_pearl_swc_setLoopDetectCtrl
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
hal_pearl_swc_setLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = PEARL_LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = PEARL_LPDETRXCR;
        if (FALSE == enable)
        {
            /* workaround, can not set RX loop detect to disable */;
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

/* FUNCTION NAME: hal_pearl_swc_getLoopDetectCtrl
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
hal_pearl_swc_getLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                         *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0, mac_port = 0, reg_addr = 0;

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = PEARL_LPDETTXCR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = PEARL_LPDETRXCR;
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

/* FUNCTION NAME: hal_pearl_swc_setLoopDetectFrameSrcMac
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
hal_pearl_swc_setLoopDetectFrameSrcMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    /* Check and set source mac */
    rc = _hal_pearl_swc_checkMacAddr(mac);
    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, PEARL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            u32dat &= ~PEARL_LPDET_SMAC_MASK;
            u32dat |= BITS_OFF_L(mac[0], 8, 8);
            u32dat |= BITS_OFF_L(mac[1], 0, 8);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", PEARL_LPDET_SA_MSB, u32dat);
            rc = aml_writeReg(unit, PEARL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
            if (AIR_E_OK == rc)
            {
                rc = aml_readReg(unit, PEARL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
                if (AIR_E_OK == rc)
                {
                    u32dat = BITS_OFF_L(mac[2], 24, 8);
                    u32dat |= BITS_OFF_L(mac[3], 16, 8);
                    u32dat |= BITS_OFF_L(mac[4], 8, 8);
                    u32dat |= BITS_OFF_L(mac[5], 0, 8);
                    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write reg:0x%08X, data:0x%08X\n", PEARL_LPDET_SA_LSB, u32dat);
                    rc = aml_writeReg(unit, PEARL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
                }
            }
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getLoopDetectFrameSrcMac
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
hal_pearl_swc_getLoopDetectFrameSrcMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    rc = aml_readReg(unit, PEARL_LPDET_SA_MSB, &u32dat, sizeof(u32dat));
    if (AIR_E_OK == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", PEARL_LPDET_SA_MSB, u32dat);
        /* Get source mac address */
        mac[0] = BITS_OFF_R(u32dat, 8, 8);
        mac[1] = BITS_OFF_R(u32dat, 0, 8);

        rc = aml_readReg(unit, PEARL_LPDET_SA_LSB, &u32dat, sizeof(u32dat));
        if (AIR_E_OK == rc)
        {
            mac[2] = BITS_OFF_R(u32dat, 24, 8);
            mac[3] = BITS_OFF_R(u32dat, 16, 8);
            mac[4] = BITS_OFF_R(u32dat, 8, 8);
            mac[5] = BITS_OFF_R(u32dat, 0, 8);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read reg:0x%08X, data:0x%08X\n", PEARL_LPDET_SA_LSB, u32dat);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_clearLoopDetectStatus
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
hal_pearl_swc_clearLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            reg_addr = 0;
    AIR_PORT_BITMAP_T setPbm = {0};

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = PEARL_LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = PEARL_LPDETRXSR;
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

/* FUNCTION NAME: hal_pearl_swc_getLoopDetectStatus
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
hal_pearl_swc_getLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            reg_addr = 0;
    AIR_PORT_BITMAP_T getPbm = {0};

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        reg_addr = PEARL_LPDETTXSR;
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        reg_addr = PEARL_LPDETRXSR;
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

/* FUNCTION NAME: hal_pearl_swc_getGlobalFreePages
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
hal_pearl_swc_getGlobalFreePages(
    const UI32_T unit,
    UI32_T      *ptr_fp_cnt,
    UI32_T      *ptr_min_fp_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_fp_cnt);
    HAL_CHECK_PTR(ptr_min_fp_cnt);

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, PEARL_FPLC, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read PEARL_FPLC failed(%d)\n", rc);
        return rc;
    }

    *ptr_fp_cnt = BITS_OFF_R(u32dat, PEARL_FPLC_FREE_PL_CNT_OFFT, PEARL_FPLC_FREE_PL_CNT_LENG);
    *ptr_min_fp_cnt = BITS_OFF_R(u32dat, PEARL_FPLC_MIN_FREE_PL_CNT_OFFT, PEARL_FPLC_MIN_FREE_PL_CNT_LENG);
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getPortAllocatedPages
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
hal_pearl_swc_getPortAllocatedPages(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_fp_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         mac_port;
    UI32_T         u32dat;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_fp_cnt);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, PEARL_FPC_RXCTRL(mac_port), &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read PEARL_FPC_RXCTRL(0x%08X) failed(%d)\n", PEARL_FPC_RXCTRL(mac_port), rc);
        return rc;
    }

    *ptr_fp_cnt = BITS_OFF_R(u32dat, PEARL_FPC_RXCTRL_FP_CNT_OFFT, PEARL_FPC_RXCTRL_FP_CNT_LENG);
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getPortUsedPages
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
 *      None
 */

AIR_ERROR_NO_T
hal_pearl_swc_getPortUsedPages(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T queue,
    const UI32_T mode,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         mac_port;
    UI32_T         u32dat;

    /* Parameters checking */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(queue, AIR_QOS_QUEUE_MAX_NUM);
    HAL_CHECK_ENUM_RANGE(mode, 2);
    HAL_CHECK_PTR(ptr_cnt);

    /* Change to MAC port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Set Packet/Page counter selector */
    u32dat = (1 == mode) ? BIT(PEARL_PCPCR_PACKET_SEL_OFFT) : 0;
    u32dat |= BITS_OFF_L(mac_port, PEARL_PCPCR_PORT_SEL_OFFT, PEARL_PCPCR_PORT_SEL_LENG);
    u32dat |= BITS_OFF_L(queue, PEARL_PCPCR_QUEUE_SEL_OFFT, PEARL_PCPCR_QUEUE_SEL_LENG);

    rc = aml_writeReg(unit, PEARL_PCPCR, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: write PEARL_PCPCR=0x%x failed(%d)\n", u32dat, rc);
        return rc;
    }

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, PEARL_PCPSR, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read PEARL_PCPSR failed(%d)\n", rc);
        return rc;
    }

    *ptr_cnt = BITS_OFF_R(u32dat, PEARL_PCPSR_COUNTER_OFFT, PEARL_PCPSR_COUNTER_LENG);
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getCapacity
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
hal_pearl_swc_getCapacity(
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
            rc = hal_pearl_acl_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_DOS_RATELIMIT:
            rc = hal_pearl_dos_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = hal_pearl_lag_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_pearl_l2_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_pearl_ipmc_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_SVLAN:
            rc = hal_pearl_svlan_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = hal_pearl_vlan_getCapacity(unit, type, param, ptr_size);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getUsage
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
hal_pearl_swc_getUsage(
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
        case AIR_SWC_RSRC_DOS_RATELIMIT:
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = AIR_E_NOT_SUPPORT;
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_pearl_l2_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_pearl_ipmc_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_SVLAN:
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = AIR_E_NOT_SUPPORT;
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_pearl_swc_getChipUid
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
hal_pearl_swc_getChipUid(
    const UI32_T unit,
    UI64_T      *ptr_uid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    rc = aml_readReg(unit, PEARL_REG_HUID_HIGH, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK == rc)
    {
        UI64_ASSIGN(*ptr_uid, u32dat, 0);
        rc = aml_readReg(unit, PEARL_REG_HUID_LOW, &u32dat, sizeof(UI32_T));
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
