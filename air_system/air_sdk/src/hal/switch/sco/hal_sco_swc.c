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

/* FILE NAME:  hal_sco_swc.c
 * PURPOSE:
 *  Implement switch module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_swc.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_mdio.h>
#include <hal/switch/sco/hal_sco_acl.h>
#include <hal/switch/sco/hal_sco_chip.h>
#include <hal/switch/sco/hal_sco_dos.h>
#include <hal/switch/sco/hal_sco_ipmc.h>
#include <hal/switch/sco/hal_sco_l2.h>
#include <hal/switch/sco/hal_sco_lag.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <hal/switch/sco/hal_sco_svlan.h>
#include <hal/switch/sco/hal_sco_vlan.h>
#include <osal/osal_lib.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_SWC_RGAC_DEFAULT_VALUE         (0x40804080)
#define HAL_SCO_SWC_AAC_DEFAULT_VALUE          (0x00125000)
#define HAL_SCO_SWC_APC_DEFAULT_VALUE          (0x40804080)
#define HAL_SCO_SWC_LPDETCR1_DEFAULT_VALUE     (0)
#define HAL_SCO_SWC_LPDETCR2_DEFAULT_VALUE     (0x00003018)
#define HAL_SCO_SWC_GMACCR_DEFAULT_VALUE       (0x00000F90)
#define HAL_SCO_SWC_SLED_CTRL1_DEFAULT_VALUE   (0x02A00019)
#define HAL_SCO_SWC_LPDET_SA_MSB_DEFAULT_VALUE (0x88740180)
#define HAL_SCO_SWC_LPDET_SA_LSB_DEFAULT_VALUE (0xC2000001)
#define HAL_SCO_SWC_ABSTC_DEFAULT_VALUE        (0)
#define HAL_SCO_SWC_HUID_HIGH                  (1)
#define HAL_SCO_SWC_HUID_LOW                   (125)

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SCO_SWC_SET_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                   \
    {                                                                    \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));            \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));  \
    } while (0)

/* DATA TYPE DECLARATIONS
 */
#define MAX_RX_JUMBO_LEN (0x3)

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SWC, "hal_sco_swc.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_sco_swc_setMgmtFrameCfg
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
hal_sco_swc_setMgmtFrameCfg(
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
            fwd = HAL_SCO_FWD_MODE_SYS_SETTING;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
            fwd = HAL_SCO_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
            fwd = HAL_SCO_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            break;
        case AIR_SWC_MGMT_FWD_MODE_CPU_ONLY:
            fwd = HAL_SCO_FWD_MODE_CPU_ONLY;
            break;
        case AIR_SWC_MGMT_FWD_MODE_DROP:
            fwd = HAL_SCO_FWD_MODE_DROP;
            break;
        default:
            break;
    }

    switch (ptr_cfg->frame_type)
    {
        case AIR_SWC_MGMT_FRAME_TYPE_IGMP:
            aml_readReg(unit, IMC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_IGMP_REP_MANG_OFFT, REG_IGMP_REP_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_IGMP_REP_BPDU_OFFT, REG_IGMP_REP_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_IGMP_REP_PRI_HIGH_OFFT, REG_IGMP_REP_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_IGMP_REP_PORT_FWD_OFFT, REG_IGMP_REP_PORT_FWD_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_IGMP_QRY_MANG_OFFT, REG_IGMP_QRY_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_IGMP_QRY_BPDU_OFFT, REG_IGMP_QRY_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_IGMP_QRY_PRI_HIGH_OFFT, REG_IGMP_QRY_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_IGMP_QRY_PORT_FWD_OFFT, REG_IGMP_QRY_PORT_FWD_LENGTH);
            aml_writeReg(unit, IMC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            aml_readReg(unit, APC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_PPPOE_MANG_OFFT, REG_PPPOE_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_PPPOE_BPDU_OFFT, REG_PPPOE_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_PPPOE_PRI_HIGH_OFFT, REG_PPPOE_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_PPPOE_PORT_FWD_OFFT, REG_PPPOE_PORT_FWD_LENGTH);
            aml_writeReg(unit, APC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            aml_readReg(unit, APC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_ARP_MANG_OFFT, REG_ARP_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_ARP_BPDU_OFFT, REG_ARP_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_ARP_PRI_HIGH_OFFT, REG_ARP_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_ARP_PORT_FWD_OFFT, REG_ARP_PORT_FWD_LENGTH);
            aml_writeReg(unit, APC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            aml_readReg(unit, PAC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_TAG_PAE_MANG_OFFT, REG_TAG_PAE_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_TAG_PAE_BPDU_OFFT, REG_TAG_PAE_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_TAG_PAE_PRI_HIGH_OFFT, REG_TAG_PAE_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_TAG_PAE_PORT_FWD_OFFT, REG_TAG_PAE_PORT_FWD_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_UTAG_PAE_MANG_OFFT, REG_UTAG_PAE_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_UTAG_PAE_BPDU_OFFT, REG_UTAG_PAE_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_UTAG_PAE_PRI_HIGH_OFFT, REG_UTAG_PAE_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_UTAG_PAE_PORT_FWD_OFFT, REG_UTAG_PAE_PORT_FWD_LENGTH);
            aml_writeReg(unit, PAC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            aml_readReg(unit, DHCP, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_DHCP_6_MANG_OFFT, REG_DHCP_6_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_DHCP_6_BPDU_OFFT, REG_DHCP_6_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_DHCP_6_PRI_HIGH_OFFT, REG_DHCP_6_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_DHCP_6_PORT_FWD_OFFT, REG_DHCP_6_PORT_FWD_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_DHCP_4_MANG_OFFT, REG_DHCP_4_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_DHCP_4_BPDU_OFFT, REG_DHCP_4_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_DHCP_4_PRI_HIGH_OFFT, REG_DHCP_4_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_DHCP_4_PORT_FWD_OFFT, REG_DHCP_4_PORT_FWD_LENGTH);
            aml_writeReg(unit, DHCP, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            aml_readReg(unit, BPC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_TTL_0_MANG_OFFT, REG_TTL_0_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_TTL_0_BPDU_OFFT, REG_TTL_0_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_TTL_0_PRI_HIGH_OFFT, REG_TTL_0_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_TTL_0_PORT_FWD_OFFT, REG_TTL_0_PORT_FWD_LENGTH);
            aml_writeReg(unit, BPC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            aml_readReg(unit, BPC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_BPDU_MANG_OFFT, REG_BPDU_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_BPDU_BPDU_OFFT, REG_BPDU_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_BPDU_PRI_HIGH_OFFT, REG_BPDU_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_BPDU_PORT_FWD_OFFT, REG_BPDU_PORT_FWD_LENGTH);
            aml_writeReg(unit, BPC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            aml_readReg(unit, MMC, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_MLD_REP_MANG_OFFT, REG_MLD_REP_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_MLD_REP_BPDU_OFFT, REG_MLD_REP_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_MLD_REP_PRI_HIGH_OFFT, REG_MLD_REP_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_MLD_REP_PORT_FWD_OFFT, REG_MLD_REP_PORT_FWD_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_MLD_QRY_MANG_OFFT, REG_MLD_QRY_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_MLD_QRY_BPDU_OFFT, REG_MLD_QRY_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_MLD_QRY_PRI_HIGH_OFFT, REG_MLD_QRY_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_MLD_QRY_PORT_FWD_OFFT, REG_MLD_QRY_PORT_FWD_LENGTH);
            aml_writeReg(unit, MMC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            aml_readReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_01_MANG_OFFT, REG_REV_01_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_01_BPDU_OFFT, REG_REV_01_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_01_PRI_HIGH_OFFT, REG_REV_01_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_01_PORT_FWD_OFFT, REG_REV_01_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            aml_readReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_02_MANG_OFFT, REG_REV_02_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_02_BPDU_OFFT, REG_REV_02_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_02_PRI_HIGH_OFFT, REG_REV_02_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_02_PORT_FWD_OFFT, REG_REV_02_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            aml_readReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_03_MANG_OFFT, REG_REV_03_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_03_BPDU_OFFT, REG_REV_03_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_03_PRI_HIGH_OFFT, REG_REV_03_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_03_PORT_FWD_OFFT, REG_REV_03_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            aml_readReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_0E_MANG_OFFT, REG_REV_0E_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_0E_BPDU_OFFT, REG_REV_0E_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_0E_PRI_HIGH_OFFT, REG_REV_0E_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_0E_PORT_FWD_OFFT, REG_REV_0E_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            aml_readReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_10_MANG_OFFT, REG_REV_10_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_10_BPDU_OFFT, REG_REV_10_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_10_PRI_HIGH_OFFT, REG_REV_10_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_10_PORT_FWD_OFFT, REG_REV_10_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            aml_readReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_20_MANG_OFFT, REG_REV_20_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_20_BPDU_OFFT, REG_REV_20_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_20_PRI_HIGH_OFFT, REG_REV_20_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_20_PORT_FWD_OFFT, REG_REV_20_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            aml_readReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_21_MANG_OFFT, REG_REV_21_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_21_BPDU_OFFT, REG_REV_21_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_21_PRI_HIGH_OFFT, REG_REV_21_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_21_PORT_FWD_OFFT, REG_REV_21_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            aml_readReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, REG_REV_UN_MANG_OFFT, REG_REV_UN_MANG_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, as_bpdu, REG_REV_UN_BPDU_OFFT, REG_REV_UN_BPDU_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, pri_high, REG_REV_UN_PRI_HIGH_OFFT, REG_REV_UN_PRI_HIGH_LENGTH);
            _HAL_SCO_SWC_SET_VALUE(u32dat, fwd, REG_REV_UN_PORT_FWD_OFFT, REG_REV_UN_PORT_FWD_LENGTH);
            aml_writeReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            break;
        default:
            break;
    }
    return ret;
}

/* FUNCTION NAME: hal_sco_swc_getMgmtFrameCfg
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
hal_sco_swc_getMgmtFrameCfg(
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
            aml_readReg(unit, IMC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_IGMP_REP_MANG_OFFT, REG_IGMP_REP_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_IGMP_REP_BPDU_OFFT, REG_IGMP_REP_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_IGMP_REP_PRI_HIGH_OFFT, REG_IGMP_REP_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_IGMP_REP_PORT_FWD_OFFT, REG_IGMP_REP_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, REG_IGMP_QRY_MANG_OFFT, REG_IGMP_QRY_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_IGMP_QRY_BPDU_OFFT, REG_IGMP_QRY_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_IGMP_QRY_PRI_HIGH_OFFT, REG_IGMP_QRY_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_IGMP_QRY_PORT_FWD_OFFT, REG_IGMP_QRY_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PPPOE:
            aml_readReg(unit, APC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_PPPOE_MANG_OFFT, REG_PPPOE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_PPPOE_BPDU_OFFT, REG_PPPOE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_PPPOE_PRI_HIGH_OFFT, REG_PPPOE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_PPPOE_PORT_FWD_OFFT, REG_PPPOE_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_ARP:
            aml_readReg(unit, APC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_ARP_MANG_OFFT, REG_ARP_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_ARP_BPDU_OFFT, REG_ARP_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_ARP_PRI_HIGH_OFFT, REG_ARP_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_ARP_PORT_FWD_OFFT, REG_ARP_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_PAE:
            aml_readReg(unit, PAC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_TAG_PAE_MANG_OFFT, REG_TAG_PAE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_TAG_PAE_BPDU_OFFT, REG_TAG_PAE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_TAG_PAE_PRI_HIGH_OFFT, REG_TAG_PAE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_TAG_PAE_PORT_FWD_OFFT, REG_TAG_PAE_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, REG_UTAG_PAE_MANG_OFFT, REG_UTAG_PAE_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_UTAG_PAE_BPDU_OFFT, REG_UTAG_PAE_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_UTAG_PAE_PRI_HIGH_OFFT, REG_UTAG_PAE_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_UTAG_PAE_PORT_FWD_OFFT, REG_UTAG_PAE_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_DHCP:
            aml_readReg(unit, DHCP, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_DHCP_6_MANG_OFFT, REG_DHCP_6_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_DHCP_6_BPDU_OFFT, REG_DHCP_6_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_DHCP_6_PRI_HIGH_OFFT, REG_DHCP_6_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_DHCP_6_PORT_FWD_OFFT, REG_DHCP_6_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, REG_DHCP_4_MANG_OFFT, REG_DHCP_4_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_DHCP_4_BPDU_OFFT, REG_DHCP_4_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_DHCP_4_PRI_HIGH_OFFT, REG_DHCP_4_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_DHCP_4_PORT_FWD_OFFT, REG_DHCP_4_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_TTL_0:
            aml_readReg(unit, BPC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_TTL_0_MANG_OFFT, REG_TTL_0_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_TTL_0_BPDU_OFFT, REG_TTL_0_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_TTL_0_PRI_HIGH_OFFT, REG_TTL_0_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_TTL_0_PORT_FWD_OFFT, REG_TTL_0_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_BPDU:
            aml_readReg(unit, BPC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_BPDU_MANG_OFFT, REG_BPDU_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_BPDU_BPDU_OFFT, REG_BPDU_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_BPDU_PRI_HIGH_OFFT, REG_BPDU_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_BPDU_PORT_FWD_OFFT, REG_BPDU_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_MLD:
            aml_readReg(unit, MMC, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_MLD_REP_MANG_OFFT, REG_MLD_REP_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_MLD_REP_BPDU_OFFT, REG_MLD_REP_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_MLD_REP_PRI_HIGH_OFFT, REG_MLD_REP_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_MLD_REP_PORT_FWD_OFFT, REG_MLD_REP_PORT_FWD_LENGTH);
            enable = BITS_OFF_R(u32dat, REG_MLD_QRY_MANG_OFFT, REG_MLD_QRY_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_MLD_QRY_BPDU_OFFT, REG_MLD_QRY_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_MLD_QRY_PRI_HIGH_OFFT, REG_MLD_QRY_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_MLD_QRY_PORT_FWD_OFFT, REG_MLD_QRY_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_01:
            aml_readReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_01_MANG_OFFT, REG_REV_01_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_01_BPDU_OFFT, REG_REV_01_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_01_PRI_HIGH_OFFT, REG_REV_01_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_01_PORT_FWD_OFFT, REG_REV_01_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_02:
            aml_readReg(unit, RGAC1, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_02_MANG_OFFT, REG_REV_02_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_02_BPDU_OFFT, REG_REV_02_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_02_PRI_HIGH_OFFT, REG_REV_02_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_02_PORT_FWD_OFFT, REG_REV_02_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_03:
            aml_readReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_03_MANG_OFFT, REG_REV_03_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_03_BPDU_OFFT, REG_REV_03_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_03_PRI_HIGH_OFFT, REG_REV_03_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_03_PORT_FWD_OFFT, REG_REV_03_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_0E:
            aml_readReg(unit, RGAC2, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_0E_MANG_OFFT, REG_REV_0E_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_0E_BPDU_OFFT, REG_REV_0E_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_0E_PRI_HIGH_OFFT, REG_REV_0E_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_0E_PORT_FWD_OFFT, REG_REV_0E_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_10:
            aml_readReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_10_MANG_OFFT, REG_REV_10_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_10_BPDU_OFFT, REG_REV_10_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_10_PRI_HIGH_OFFT, REG_REV_10_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_10_PORT_FWD_OFFT, REG_REV_10_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_20:
            aml_readReg(unit, RGAC3, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_20_MANG_OFFT, REG_REV_20_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_20_BPDU_OFFT, REG_REV_20_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_20_PRI_HIGH_OFFT, REG_REV_20_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_20_PORT_FWD_OFFT, REG_REV_20_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_21:
            aml_readReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_21_MANG_OFFT, REG_REV_21_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_21_BPDU_OFFT, REG_REV_21_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_21_PRI_HIGH_OFFT, REG_REV_21_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_21_PORT_FWD_OFFT, REG_REV_21_PORT_FWD_LENGTH);
            break;
        case AIR_SWC_MGMT_FRAME_TYPE_REV_UN:
            aml_readReg(unit, RGAC4, &u32dat, sizeof(u32dat));
            enable = BITS_OFF_R(u32dat, REG_REV_UN_MANG_OFFT, REG_REV_UN_MANG_LENGTH);
            as_bpdu = BITS_OFF_R(u32dat, REG_REV_UN_BPDU_OFFT, REG_REV_UN_BPDU_LENGTH);
            pri_high = BITS_OFF_R(u32dat, REG_REV_UN_PRI_HIGH_OFFT, REG_REV_UN_PRI_HIGH_LENGTH);
            fwd = BITS_OFF_R(u32dat, REG_REV_UN_PORT_FWD_OFFT, REG_REV_UN_PORT_FWD_LENGTH);
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
        case HAL_SCO_FWD_MODE_SYS_SETTING:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
            break;
        case HAL_SCO_FWD_MODE_SYS_SETTING_INCLUDE_CPU:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            break;
        case HAL_SCO_FWD_MODE_SYS_SETTING_EXCLUDE_CPU:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            break;
        case HAL_SCO_FWD_MODE_CPU_ONLY:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
            break;
        case HAL_SCO_FWD_MODE_DROP:
            ptr_cfg->forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
            break;
        default:
            break;
    }

    return ret;
}

/* FUNCTION NAME: hal_sco_swc_setSystemMac
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
hal_sco_swc_setSystemMac(
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
    aml_writeReg(unit, SMACCR1, &u32dat, sizeof(u32dat));

    /* SMACCR 0 */
    u32dat = 0;
    for (i = 2; i < 6; i++)
    {
        u32dat |= BITS_OFF_L(mac[i], ((5 - i) * 8), 8);
    }
    aml_writeReg(unit, SMACCR0, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_getSysMac
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
hal_sco_swc_getSystemMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    UI32_T u32dat;
    UI8_T  i;

    /* SMACCR 1 */
    aml_readReg(unit, SMACCR1, &u32dat, sizeof(u32dat));
    for (i = 0; i < 2; i++)
    {
        mac[i] = BITS_OFF_R(u32dat, ((1 - i) * 8), 8);
    }

    /* SMACCR 0 */
    aml_readReg(unit, SMACCR0, &u32dat, sizeof(u32dat));
    for (i = 2; i < 6; i++)
    {
        mac[i] = BITS_OFF_R(u32dat, ((5 - i) * 8), 8);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_swc_init
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
hal_sco_swc_init(
    const UI32_T unit)
{
    UI32_T                    u32dat = 0;
    AIR_SWC_MGMT_FRAME_CFG_T  mgmt_cfg;
    AIR_SWC_MGMT_FRAME_TYPE_T mgmt_type;
    AIR_CFG_VALUE_T           mtcc_cfg;

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
        hal_sco_swc_setMgmtFrameCfg(unit, &mgmt_cfg);
    }

    /* Designate the following packet as a BPDU frame */
    aml_readReg(unit, RGAC1, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_REV_02_BPDU_OFFT);
    aml_writeReg(unit, RGAC1, &u32dat, sizeof(u32dat));

    aml_readReg(unit, RGAC2, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_REV_0E_BPDU_OFFT);
    u32dat |= BIT(REG_REV_03_BPDU_OFFT);
    aml_writeReg(unit, RGAC2, &u32dat, sizeof(u32dat));

    aml_readReg(unit, RGAC3, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_REV_20_BPDU_OFFT);
    aml_writeReg(unit, RGAC3, &u32dat, sizeof(u32dat));

    aml_readReg(unit, RGAC4, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_REV_21_BPDU_OFFT);
    aml_writeReg(unit, RGAC4, &u32dat, sizeof(u32dat));

    aml_readReg(unit, BPC, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_BPDU_BPDU_OFFT);
    aml_writeReg(unit, BPC, &u32dat, sizeof(u32dat));

    /* Enable auto flush */
    aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));
    u32dat |= BIT(AAC_AUTO_FLUSH_OFFSET);
    aml_writeReg(unit, AAC, &u32dat, sizeof(u32dat));

    /* Set ARP forward including CPU  */
    aml_readReg(unit, APC, &u32dat, sizeof(u32dat));
    _HAL_SCO_SWC_SET_VALUE(u32dat, HAL_SCO_FWD_MODE_SYS_SETTING_INCLUDE_CPU, REG_ARP_PORT_FWD_OFFT,
                           REG_ARP_PORT_FWD_LENGTH);
    aml_writeReg(unit, APC, &u32dat, sizeof(u32dat));

    /* Set loop detection period to 1 second */
    aml_readReg(unit, LPDETCR2, &u32dat, sizeof(u32dat));
    u32dat |= BIT(REG_LPDET_PERIOD_OFFT);
    aml_writeReg(unit, LPDETCR2, &u32dat, sizeof(u32dat));

    /* Set mtcc */
    osal_memset(&mtcc_cfg, 0, sizeof(AIR_CFG_VALUE_T));
    hal_cfg_getValue(unit, AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT, &mtcc_cfg);
    aml_readReg(unit, GMACCR, &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(GMACCR_MTCC_OFFT, GMACCR_MTCC_LENG);
    u32dat |= ((mtcc_cfg.value << GMACCR_MTCC_OFFT) & BITS_RANGE(GMACCR_MTCC_OFFT, GMACCR_MTCC_LENG));
    aml_writeReg(unit, GMACCR, &u32dat, sizeof(u32dat));

    /* SLED clock rate fine tune; refresh 60Hz, clock 2.08Mhz */
    u32dat = (16000 << 16) + 12;
    aml_writeReg(unit, SLED_CTRL1, &u32dat, sizeof(u32dat));

    /* Set default rate limit include preamble/IPG/CRC */
    aml_readReg(unit, GERLCR, &u32dat, sizeof(u32dat));
    _HAL_SCO_SWC_SET_VALUE(u32dat, L1_RATE_IPG_BYTE_CNT, REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
    aml_writeReg(unit, GERLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, GIRLCR, &u32dat, sizeof(u32dat));
    _HAL_SCO_SWC_SET_VALUE(u32dat, L1_RATE_IPG_BYTE_CNT, REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
    aml_writeReg(unit, GIRLCR, &u32dat, sizeof(u32dat));

    aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
    _HAL_SCO_SWC_SET_VALUE(u32dat, L1_RATE_IPG_BYTE_CNT, AGC_COMP_BNUM_OFFT, AGC_COMP_BNUM_LENG);
    aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME:
 *      hal_sco_swc_deinit
 * PURPOSE:
 *      This API is used to deinitialize switch related items:
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
hal_sco_swc_deinit(
    const UI32_T unit)
{
    UI32_T u32dat = 0;

    u32dat = HAL_SCO_SWC_RGAC_DEFAULT_VALUE;
    aml_writeReg(unit, RGAC1, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, RGAC2, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, RGAC3, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, RGAC4, &u32dat, sizeof(u32dat));

    u32dat = HAL_SCO_SWC_AAC_DEFAULT_VALUE;
    aml_writeReg(unit, AAC, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SWC_APC_DEFAULT_VALUE;
    aml_writeReg(unit, APC, &u32dat, sizeof(u32dat));

    u32dat = HAL_SCO_SWC_LPDETCR1_DEFAULT_VALUE;
    aml_writeReg(unit, LPDETCR1, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SWC_LPDETCR2_DEFAULT_VALUE;
    aml_writeReg(unit, LPDETCR2, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SWC_LPDET_SA_MSB_DEFAULT_VALUE;
    aml_writeReg(unit, LPDET_SA_MSB, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SWC_LPDET_SA_LSB_DEFAULT_VALUE;
    aml_writeReg(unit, LPDET_SA_LSB, &u32dat, sizeof(u32dat));

    u32dat = HAL_SCO_SWC_GMACCR_DEFAULT_VALUE;
    aml_writeReg(unit, GMACCR, &u32dat, sizeof(u32dat));

    u32dat = HAL_SCO_SWC_SLED_CTRL1_DEFAULT_VALUE;
    aml_writeReg(unit, SLED_CTRL1, &u32dat, sizeof(u32dat));

    u32dat = HAL_SCO_SWC_ABSTC_DEFAULT_VALUE;
    aml_writeReg(unit, ABSTC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_setJumboSize
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
hal_sco_swc_setJumboSize(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T frame_len)
{
    UI32_T u32dat = 0;
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: frame_len = %d\n", frame_len);
    /* Read and clear jumbo frame info */
    aml_readReg(unit, GMACCR, &u32dat, sizeof(u32dat));

    u32dat &= ~BITS_RANGE(MAX_RX_PKT_LEN_OFFT, MAX_RX_PKT_LEN_LENG);
    u32dat &= ~BITS_RANGE(MAX_RX_JUMBO_OFFT, MAX_RX_JUMBO_LENG);

    if (frame_len < MAX_RX_JUMBO_LEN)
    {
        u32dat |= frame_len;
    }
    else /* AIR_SWC_JUMBO_SIZE_2048 ~ AIR_SWC_JUMBO_SIZE_15360*/
    {
        u32dat |= MAX_RX_JUMBO_LEN;
        u32dat |= (((frame_len - AIR_SWC_JUMBO_SIZE_2048) + 2) << MAX_RX_JUMBO_OFFT);
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: GMACCR value = %x\n", u32dat);
    aml_writeReg(unit, GMACCR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_getJumboSize
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
hal_sco_swc_getJumboSize(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_frame_len)
{
    UI32_T u32dat = 0, pkt_len;

    /* Read and clear jumbo frame info */
    aml_readReg(unit, GMACCR, &u32dat, sizeof(u32dat));

    pkt_len = BITS_OFF_R(u32dat, MAX_RX_PKT_LEN_OFFT, MAX_RX_PKT_LEN_LENG);

    if (pkt_len < MAX_RX_JUMBO_LEN)
    {
        *ptr_frame_len = pkt_len;
    }
    else
    {
        pkt_len = BITS_OFF_R(u32dat, MAX_RX_JUMBO_OFFT, MAX_RX_JUMBO_LENG);
        *ptr_frame_len = pkt_len + 1;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_setProperty
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
hal_sco_swc_setProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1)
{
    UI32_T u32dat = 0, type = 0, enable = 0;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AAC 0x%x = 0x%x\n", AAC, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, param0, AAC_AUTO_FLUSH_OFFSET, AAC_AUTO_FLUSH_LENGTH);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AAC 0x%x = 0x%x\n", AAC, u32dat);
            aml_writeReg(unit, AAC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            type = (1 == param0) ? L1_RATE_IPG_BYTE_CNT : L2_RATE_IPG_BYTE_CNT;
            /* set egress rate control */
            aml_readReg(unit, GERLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GERLCR 0x%x = 0x%x\n", GERLCR, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, type, REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write GERLCR 0x%x = 0x%x\n", GERLCR, u32dat);
            aml_writeReg(unit, GERLCR, &u32dat, sizeof(u32dat));

            /* set ingress rate control */
            aml_readReg(unit, GIRLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GIRLCR 0x%x = 0x%x\n", GIRLCR, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, type, REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write GIRLCR 0x%x = 0x%x\n", GIRLCR, u32dat);
            aml_writeReg(unit, GIRLCR, &u32dat, sizeof(u32dat));

            /* set compensation byte number include/exclude L1 size */
            aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", AGC, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, type, AGC_COMP_BNUM_OFFT, AGC_COMP_BNUM_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", AGC, u32dat);
            aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));

            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", AGC, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, AGC_ACLRATE_EXC_MG_OFFT, AGC_ACLRATE_EXC_MG_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", AGC, u32dat);
            aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            enable = (1 == param0) ? 0 : 1;
            aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", AGC, u32dat);
            _HAL_SCO_SWC_SET_VALUE(u32dat, enable, AGC_BCSTRM_EXC_MG_OFFT, AGC_BCSTRM_EXC_MG_LENG);
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: write AGC 0x%x = 0x%x\n", AGC, u32dat);
            aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));
            break;
        default:
            break;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_getProperty
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
hal_sco_swc_getProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1)
{
    UI32_T u32dat = 0, type = 0, enable = 0;

    switch (property)
    {
        case AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH:
            aml_readReg(unit, AAC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AAC 0x%x = 0x%x\n", AAC, u32dat);
            *ptr_param0 = BITS_OFF_R(u32dat, AAC_AUTO_FLUSH_OFFSET, AAC_AUTO_FLUSH_LENGTH);
            break;
        case AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL:
            /* get egress rate control */
            aml_readReg(unit, GERLCR, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read GERLCR 0x%x = 0x%x\n", GERLCR, u32dat);
            type = BITS_OFF_R(u32dat, REG_IPG_BYTE_OFFT, REG_IPG_BYTE_LENG);
            *ptr_param0 = (L1_RATE_IPG_BYTE_CNT == type) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE:
            aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", AGC, u32dat);
            enable = BITS_OFF_R(u32dat, AGC_ACLRATE_EXC_MG_OFFT, AGC_ACLRATE_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        case AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE:
            aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: read AGC 0x%x = 0x%x\n", AGC, u32dat);
            enable = BITS_OFF_R(u32dat, AGC_BCSTRM_EXC_MG_OFFT, AGC_BCSTRM_EXC_MG_LENG);
            *ptr_param0 = (0 == enable) ? 1 : 0;
            break;
        default:
            break;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_swc_getGlobalFreePages
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
hal_sco_swc_getGlobalFreePages(
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
    rc = aml_readReg(unit, FPLC, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read FPLC failed(%d)\n", rc);
        return rc;
    }

    *ptr_fp_cnt = BITS_OFF_R(u32dat, FPLC_FREE_PL_CNT_OFFT, FPLC_FREE_PL_CNT_LENG);
    *ptr_min_fp_cnt = BITS_OFF_R(u32dat, FPLC_MIN_FREE_PL_CNT_OFFT, FPLC_MIN_FREE_PL_CNT_LENG);
    return rc;
}

/* FUNCTION NAME: hal_sco_swc_getPortAllocatedPages
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
hal_sco_swc_getPortAllocatedPages(
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
    rc = aml_readReg(unit, FPC_RXCTRL(mac_port), &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read FPC_RXCTRL(0x%08X) failed(%d)\n", FPC_RXCTRL(mac_port), rc);
        return rc;
    }

    *ptr_fp_cnt = BITS_OFF_R(u32dat, FPC_RXCTRL_FP_CNT_OFFT, FPC_RXCTRL_FP_CNT_LENG);
    return rc;
}

/* FUNCTION NAME: hal_sco_swc_getPortUsedPages
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
hal_sco_swc_getPortUsedPages(
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
    u32dat = (1 == mode) ? BIT(PCPCR_PACKET_SEL_OFFT) : 0;
    u32dat |= BITS_OFF_L(mac_port, PCPCR_PORT_SEL_OFFT, PCPCR_PORT_SEL_LENG);
    u32dat |= BITS_OFF_L(queue, PCPCR_QUEUE_SEL_OFFT, PCPCR_QUEUE_SEL_LENG);

    rc = aml_writeReg(unit, PCPCR, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: write PCPCR=0x%x failed(%d)\n", u32dat, rc);
        return rc;
    }

    /* Get Packet/Page counter probe status */
    rc = aml_readReg(unit, PCPSR, &u32dat, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: read PCPSR failed(%d)\n", rc);
        return rc;
    }

    *ptr_cnt = BITS_OFF_R(u32dat, PCPSR_COUNTER_OFFT, PCPSR_COUNTER_LENG);
    return rc;
}

/* FUNCTION NAME: hal_sco_swc_getCapacity
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
hal_sco_swc_getCapacity(
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
            rc = hal_sco_acl_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_DOS_RATELIMIT:
            rc = hal_sco_dos_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = hal_sco_lag_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_sco_l2_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_sco_ipmc_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_SVLAN:
            rc = hal_sco_svlan_getCapacity(unit, type, param, ptr_size);
            break;
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = hal_sco_vlan_getCapacity(unit, type, param, ptr_size);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_swc_getUsage
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
hal_sco_swc_getUsage(
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
            rc = hal_sco_acl_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_DOS_RATELIMIT:
            rc = hal_sco_dos_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_LAG_GROUP:
        case AIR_SWC_RSRC_LAG_GROUP_MEMBER:
            rc = hal_sco_lag_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_L2_FDB:
            rc = hal_sco_l2_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_IPMC_FDB:
            rc = hal_sco_ipmc_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_SVLAN:
            rc = hal_sco_svlan_getUsage(unit, type, param, ptr_cnt);
            break;
        case AIR_SWC_RSRC_VLAN_MAC:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV4:
        case AIR_SWC_RSRC_VLAN_SUBNET_IPV6:
        case AIR_SWC_RSRC_VLAN_PROTOCOL:
            rc = hal_sco_vlan_getUsage(unit, type, param, ptr_cnt);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_swc_getChipUid
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
hal_sco_swc_getChipUid(
    const UI32_T unit,
    UI64_T      *ptr_uid)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         u32dat = 0;

    rc = hal_sco_chip_readEfuse(unit, HAL_SCO_SWC_HUID_HIGH, &u32dat);
    if (AIR_E_OK == rc)
    {
        UI64_ASSIGN(*ptr_uid, u32dat, 0);
        rc = hal_sco_chip_readEfuse(unit, HAL_SCO_SWC_HUID_LOW, &u32dat);
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
