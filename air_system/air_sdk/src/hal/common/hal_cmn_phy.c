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

/* FILE NAME:  hal_cmn_phy.c
 * PURPOSE:
 *  Implement common PHY module API function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/common/hal_cmn_phy.h>

#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_hw_util.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SET_VALUE(__out__, __val__, __offset__, __length__)        \
    do                                                                  \
    {                                                                   \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));           \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__)); \
    } while (0)

#define _HAL_CMN_PHY_FLAG_SET_BIT(__flags__, __flag__, __output__, __bit__) \
    do                                                                      \
    {                                                                       \
        if (__flags__ & __flag__)                                           \
        {                                                                   \
            __output__ |= BIT(__bit__);                                     \
        }                                                                   \
        else                                                                \
        {                                                                   \
            __output__ &= ~BIT(__bit__);                                    \
        }                                                                   \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_PHY, "hal_cmn_phy.c");

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

static AIR_ERROR_NO_T
_hal_cmn_phy_applyDiscretePara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x000, 0x010a);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x001, 0x01bc);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x002, 0x01c3);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x003, 0x0183);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x004, 0x0200);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x005, 0x0206);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x006, 0x0380);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x007, 0x03ba);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x008, 0x03ca);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x009, 0x0312);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x00a, 0x0203);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x00b, 0x0002);

    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x023, 0x0882);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x024, 0x0882);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x025, 0x0882);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x026, 0x0882);

    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x269, 0x1414);

    return rv;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: hal_cmn_phy_triggerCableTest
 * PURPOSE:
 *      Get cable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *      test_pair       --  Select test pair
 *                          HAL_PHY_CABLE_TEST_PAIR_A
 *                          HAL_PHY_CABLE_TEST_PAIR_B
 *                          HAL_PHY_CABLE_TEST_PAIR_C
 *                          HAL_PHY_CABLE_TEST_PAIR_D
 *                          HAL_PHY_CABLE_TEST_PAIR_ALL
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          HAL_PHY_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
hal_cmn_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
#ifdef AIR_EN_CABLE_DIAG
    UI32_T pair = 0;

    HAL_CHECK_ENUM_RANGE(test_pair, HAL_PHY_CABLE_TEST_PAIR_LAST);
    switch (test_pair)
    {
        case HAL_PHY_CABLE_TEST_PAIR_A:
            pair = HAL_CABLE_PAIR_A;
            break;
        case HAL_PHY_CABLE_TEST_PAIR_B:
            pair = HAL_CABLE_PAIR_B;
            break;
        case HAL_PHY_CABLE_TEST_PAIR_C:
            pair = HAL_CABLE_PAIR_C;
            break;
        case HAL_PHY_CABLE_TEST_PAIR_D:
            pair = HAL_CABLE_PAIR_D;
            break;
        case HAL_PHY_CABLE_TEST_PAIR_ALL:
            pair = HAL_CABLE_PAIR_ALL;
            break;
        default:
            break;
    }
    ret = cmlib_hw_util_triggerCableDiag(unit, port, pair, ptr_test_rslt);
#else
    ret = AIR_E_NOT_SUPPORT;
#endif
    return ret;
}

/* FUNCTION NAME: hal_cmn_phy_getCableTestRawData
 * PURPOSE:
 *      Get cable ec training 4 pair raw date.
 *
 * INPUT:
 *      unit                --  Device ID
 *      port                --  Select port number
 *      test_pair           --  Select test pair
 *
 * OUTPUT:
 *      pptr_raw_data_all   --  Cable diagnostic raw information
 *
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_cmn_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
#ifdef AIR_EN_CABLE_DIAG
    UI8_T pair;
    HAL_CHECK_PTR(pptr_raw_data_all);

    for (pair = 0; pair < HAL_PHY_CABLE_TEST_PAIR_ALL; pair++)
    {
        ret = cmlib_hw_util_getCableDiagRawData(unit, port, pair, pptr_raw_data_all[pair]);
    }
#else
    ret = AIR_E_NOT_SUPPORT;
#endif
    return ret;
}

/* FUNCTION NAME: hal_cmn_phy_setPhyLedGlbCfg
 * PURPOSE:
 *      Set LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16_val;
    HAL_CHECK_PTR(ptr_glb_cfg);

    /* Read original value */
    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, (UI16_T)HAL_CMN_PHY_LED_BASIC_CTRL, &u16_val);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Read CL45 unit %u, port %u, dev-0x%X reg-0x%X fail\n", unit, port, MMD_DEV_VSPEC2,
                   HAL_CMN_PHY_LED_BASIC_CTRL);
        return rc;
    }

    /* LED enhance mode */
    _HAL_CMN_PHY_FLAG_SET_BIT(ptr_glb_cfg->flags, HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD, u16_val,
                              HAL_CMN_PHY_LED_ENHANCE_MOD_OFFSET);

    /* LED clock enable */
    _HAL_CMN_PHY_FLAG_SET_BIT(ptr_glb_cfg->flags, HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN, u16_val,
                              HAL_CMN_PHY_LED_CLK_EN_OFFSET);

    /* LED fast frequency */
    _HAL_CMN_PHY_FLAG_SET_BIT(ptr_glb_cfg->flags, HAL_PHY_LED_GLB_CFG_FLAGS_FAST_CLOCK, u16_val,
                              HAL_CMN_PHY_LED_FAST_CLK_OFFSET);

    /* Write new value */
    rc = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, (UI16_T)HAL_CMN_PHY_LED_BASIC_CTRL, u16_val);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Write CL45 unit %u, port %u, dev-0x%X reg-0x%X fail\n", unit, port, MMD_DEV_VSPEC2,
                   HAL_CMN_PHY_LED_BASIC_CTRL);
    }

    return rc;
}

/* FUNCTION NAME: hal_cmn_phy_getPhyLedGlbCfg
 * PURPOSE:
 *      Get LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         u16_val;
    HAL_CHECK_PTR(ptr_glb_cfg);

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, (UI16_T)HAL_CMN_PHY_LED_BASIC_CTRL, &u16_val);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Read CL45 unit %u, port %u, dev-0x%X reg-0x%X fail\n", unit, port, MMD_DEV_VSPEC2,
                   HAL_CMN_PHY_LED_BASIC_CTRL);
        return rc;
    }

    osal_memset(ptr_glb_cfg, 0, sizeof(HAL_PHY_LED_GLB_CFG_T));

    if (u16_val & BIT(HAL_CMN_PHY_LED_ENHANCE_MOD_OFFSET))
    {
        ptr_glb_cfg->flags |= HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD;
    }

    if (u16_val & BIT(HAL_CMN_PHY_LED_CLK_EN_OFFSET))
    {
        ptr_glb_cfg->flags |= HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN;
    }

    if (u16_val & BIT(HAL_CMN_PHY_LED_FAST_CLK_OFFSET))
    {
        ptr_glb_cfg->flags |= HAL_PHY_LED_GLB_CFG_FLAGS_FAST_CLOCK;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_cmn_phy_setPhyLedBlkEvent
 * PURPOSE:
 *      Set LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 *      evt_flags                -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         reg_addr;

    switch (led_id)
    {
        case 0:
            reg_addr = HAL_CMN_PHY_LED_0_BLK_MASK;
            break;
        case 1:
            reg_addr = HAL_CMN_PHY_LED_1_BLK_MASK;
            break;
        case 2:
            reg_addr = HAL_CMN_PHY_LED_2_BLK_MASK;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "Unit %u port %u, invalid LED ID %u\n", unit, port, led_id);
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, (UI16_T)evt_flags);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Write CL45 unit %u, port %u, dev-0x%X reg-0x%X data-0x%X fail\n", unit, port,
                   MMD_DEV_VSPEC2, reg_addr, evt_flags);
    }

    return rc;
}

/* FUNCTION NAME: hal_cmn_phy_getPhyLedBlkEvent
 * PURPOSE:
 *      Get LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_evt_flags            -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         reg_addr;
    UI16_T         val;
    HAL_CHECK_PTR(ptr_evt_flags);
    switch (led_id)
    {
        case 0:
            reg_addr = HAL_CMN_PHY_LED_0_BLK_MASK;
            break;
        case 1:
            reg_addr = HAL_CMN_PHY_LED_1_BLK_MASK;
            break;
        case 2:
            reg_addr = HAL_CMN_PHY_LED_2_BLK_MASK;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "Unit %u port %u, invalid LED ID %u\n", unit, port, led_id);
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &val);
    if (AIR_E_OK == rc)
    {
        *ptr_evt_flags = (UI32_T)val;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "Write CL45 unit %u, port %u, dev-0x%X reg-0x%X fail\n", unit, port, MMD_DEV_VSPEC2,
                   reg_addr);
    }

    return rc;
}

/* FUNCTION NAME: hal_cmn_phy_setPhyLedDuration
 * PURPOSE:
 *      Set LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 *      time                     -- Duration time, unit: ms
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         reg_addr;

    switch (mode)
    {
        case HAL_PHY_LED_BLK_CTRL_MODE_ON:
            reg_addr = HAL_CMN_PHY_LED_ON_DRA;
            break;
        case HAL_PHY_LED_BLK_CTRL_MODE_BLK:
            reg_addr = HAL_CMN_PHY_LED_BLK_DRA;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "Unit %u port %u, invalid duration mode%u\n", unit, port, mode);
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, (UI16_T)time);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Write CL45 unit %u, port %u, dev-0x%X reg-0x%X data-0x%X fail\n", unit, port,
                   MMD_DEV_VSPEC2, reg_addr, time);
    }

    return rc;
}

/* FUNCTION NAME: hal_cmn_phy_getPhyLedDuration
 * PURPOSE:
 *      Get LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 * OUTPUT:
 *      ptr_time                 -- Duration time, unit: ms
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         reg_addr;
    UI16_T         val;

    HAL_CHECK_PTR(ptr_time);
    switch (mode)
    {
        case HAL_PHY_LED_BLK_CTRL_MODE_ON:
            reg_addr = HAL_CMN_PHY_LED_ON_DRA;
            break;
        case HAL_PHY_LED_BLK_CTRL_MODE_BLK:
            reg_addr = HAL_CMN_PHY_LED_BLK_DRA;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "Unit %u port %u, invalid duration mode%u\n", unit, port, mode);
            return AIR_E_BAD_PARAMETER;
    }

    rc = hal_mdio_readC45ByPort(unit, port, MMD_DEV_VSPEC2, reg_addr, &val);
    if (AIR_E_OK == rc)
    {
        *ptr_time = (UI32_T)val;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "Write CL45 unit %u, port %u, dev-0x%X reg-0x%X fail\n", unit, port, MMD_DEV_VSPEC2,
                   reg_addr);
    }

    return rc;
}

/* FUNCTION NAME:   hal_cmn_phy_testTxCompliance
 * PURPOSE:
 *      This API is used to set the Tx compliance mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- BIST mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_cmn_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;
    UI16_T         page = 0;
    UI16_T         u16dat = 0;

    /* Backup page of CL22 */
    rv |= hal_mdio_readC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, &page);
    rv |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, HAL_CMN_PHY_CL22_PAGE_REG);
    rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, 0x0);
    switch (mode)
    {
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B:
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x8000);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x100);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A == mode))
            {
                u16dat = 0x5010;
            }
            else
            {
                u16dat = 0x5018;
            }
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x33, 0x177);
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC2, 0x27b, 0x1177);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0xd2;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0x2;
            }
            else
            {
                u16dat = 0x0;
            }
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x1a3, u16dat);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0x110e;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0x1100;
            }
            else
            {
                u16dat = 0x0;
            }
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x1a4, u16dat);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B == mode))
            {
                u16dat = 0x0;
            }
            else if ((HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A == mode) ||
                     (HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B == mode))
            {
                u16dat = 0xf842;
            }
            else
            {
                /* 10M sine pair a/b*/
                u16dat = 0xf840;
            }
            rv |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, HAL_CMN_PHY_CL22_PAGE_EXTREG);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_AUX_CTRL_STA, u16dat);
            rv |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, HAL_CMN_PHY_CL22_PAGE_REG);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x0);
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE:
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE == mode))
            {
                rv |= _hal_cmn_phy_applyDiscretePara(unit, port);
                rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, BMCR_RESET);
                osal_delayUs(1000);
            }
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x2100);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                u16dat = 0x5010;
            }
            else
            {
                u16dat = 0x5018;
            }
            rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x145, u16dat);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x0);
            if ((HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A == mode) ||
                (HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE == mode))
            {
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x23, 0x882);
            }
            else
            {
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x24, 0x882);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3:
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
            {
                u16dat = 0x2700;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2 == mode)
            {
                u16dat = 0x4700;
            }
            else
            {
                u16dat = 0x6700;
            }
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, u16dat);
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1 == mode)
            {
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x14, 0x4444);
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3 == mode)
            {
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x143, 0x200);
            }
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1040);
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C:
        case HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D:
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_CTRL1000, 0x8700);
            rv |= hal_mdio_writeC22ByPort(unit, port, MII_BMCR, 0x1040);
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A == mode)
            {
                u16dat = 0x7878;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B == mode)
            {
                u16dat = 0xb8b8;
            }
            else if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C == mode)
            {
                u16dat = 0xd8d8;
            }
            else
            {
                /* HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D */
                u16dat = 0xe8e8;
            }
            if (HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4 != mode)
            {
                rv |= hal_mdio_writeC45ByPort(unit, port, MMD_DEV_VSPEC1, 0x3e, u16dat);
            }
            break;
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5:
        case HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6:
            return AIR_E_NOT_SUPPORT;
        default:
            /* Unrecognized argument */
            return AIR_E_BAD_PARAMETER;
    }
    /* Restore page of CL22 */
    rv |= hal_mdio_writeC22ByPort(unit, port, HAL_CMN_PHY_MII_PAGE_SELECT, page);
    if (AIR_E_OK != rv)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Port %u test tx compliance mode %u failed! ret=%u\n", port, mode, rv);
    }
    return rv;
}

/* FUNCTION NAME: hal_cmn_phy_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          HAL_PHY_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic in link down mode only.
 */
AIR_ERROR_NO_T
hal_cmn_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
#ifdef AIR_EN_CABLE_DIAG
    ret = cmlib_hw_util_triggerLinkDownCableDiag(unit, port, ptr_test_rslt);
#else
    ret = AIR_E_NOT_SUPPORT;
#endif
    return ret;
}
