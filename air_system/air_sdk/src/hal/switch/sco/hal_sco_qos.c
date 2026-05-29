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

/* FILE NAME:  hal_sco_qos.c
 * PURPOSE:
 *  Implement QoS module HAL function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_qos.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_ifmon.h>
#include <hal/switch/sco/hal_sco_port.h>
#include <hal/switch/sco/hal_sco_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_QOS_UPW_DEFAULT_VALUE    (0x00234567)
#define HAL_SCO_QOS_PEM1_DEFAULT_VALUE   (0x08480140)
#define HAL_SCO_QOS_PEM2_DEFAULT_VALUE   (0x1B581210)
#define HAL_SCO_QOS_PEM3_DEFAULT_VALUE   (0x25A824A0)
#define HAL_SCO_QOS_PEM4_DEFAULT_VALUE   (0x3FF836F0)
#define HAL_SCO_QOS_PIM1_DEFAULT_VALUE   (0x09000000)
#define HAL_SCO_QOS_PIM2_DEFAULT_VALUE   (0x12489249)
#define HAL_SCO_QOS_PIM3_DEFAULT_VALUE   (0x1B6DB492)
#define HAL_SCO_QOS_PIM4_DEFAULT_VALUE   (0x2492491B)
#define HAL_SCO_QOS_PIM5_DEFAULT_VALUE   (0x36B6DB6D)
#define HAL_SCO_QOS_PIM6_DEFAULT_VALUE   (0x3FFF6DB6)
#define HAL_SCO_QOS_PIM7_DEFAULT_VALUE   (0x00000FFF)
#define HAL_SCO_QOS_GERLCR_DEFAULT_VALUE (0x00000104)
#define HAL_SCO_QOS_GIRLCR_DEFAULT_VALUE (0x00110104)
#define HAL_SCO_QOS_ERLCR_DEFAULT_VALUE  (0)
#define HAL_SCO_QOS_IRLCR_DEFAULT_VALUE  (0)
#define HAL_SCO_QOS_PCR_DEFAULT_VALUE    (0)
#define HAL_SCO_QOS_MMSCR0_DEFAULT_VALUE (0x80000000)
#define HAL_SCO_QOS_MMSCR1_DEFAULT_VALUE (0x80040000)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_QOS, "hal_sco_qos.c");

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_sco_qos_set_RateLimitToken
 * PURPOSE:
 *      Set rate limit token.
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */

static void
_hal_sco_qos_set_RateLimitToken(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T            u32dat = 0;
    UI32_T            mac_port;
    AIR_PORT_STATUS_T port_status = {0};

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    hal_sco_port_getPortStatus(unit, port, &port_status);

    /* For Ingress rate setting */
    aml_readReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));
    /* Set token period */
    u32dat &= ~BITS_RANGE(REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    switch (port_status.speed)
    {
        case AIR_PORT_SPEED_10M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_16MS, REG_RATE_TB_OFFT, REG_RATE_TB_OFFT);
            break;
        case AIR_PORT_SPEED_100M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_2MS, REG_RATE_TB_OFFT, REG_RATE_TB_OFFT);
            break;
        case AIR_PORT_SPEED_1000M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_1_4MS, REG_RATE_TB_OFFT, REG_RATE_TB_OFFT);
            break;
        case AIR_PORT_SPEED_2500M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_1_16MS, REG_RATE_TB_OFFT, REG_RATE_TB_OFFT);
            break;
        default:
            break;
    }
    aml_writeReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));

    /* Set ingress stop refill token */
    aml_readReg(unit, IBCR(mac_port), &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(REG_IGC_STOP_REFILL_TOKEN_OFFT, REG_IGC_STOP_REFILL_TOKEN_LENG);
    switch (port_status.speed)
    {
        case AIR_PORT_SPEED_10M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_REFILL_BASE - HAL_SCO_QOS_TOKEN_PERIOD_16MS,
                                 REG_IGC_STOP_REFILL_TOKEN_OFFT, REG_IGC_STOP_REFILL_TOKEN_LENG);
            break;
        case AIR_PORT_SPEED_100M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_REFILL_BASE - HAL_SCO_QOS_TOKEN_PERIOD_2MS,
                                 REG_IGC_STOP_REFILL_TOKEN_OFFT, REG_IGC_STOP_REFILL_TOKEN_LENG);
            break;
        case AIR_PORT_SPEED_1000M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_REFILL_BASE - HAL_SCO_QOS_TOKEN_PERIOD_1_4MS,
                                 REG_IGC_STOP_REFILL_TOKEN_OFFT, REG_IGC_STOP_REFILL_TOKEN_LENG);
            break;
        case AIR_PORT_SPEED_2500M:
            u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_REFILL_BASE - HAL_SCO_QOS_TOKEN_PERIOD_1_16MS,
                                 REG_IGC_STOP_REFILL_TOKEN_OFFT, REG_IGC_STOP_REFILL_TOKEN_LENG);
            break;
        default:
            break;
    }
    aml_writeReg(unit, IBCR(mac_port), &u32dat, sizeof(u32dat));
}

/* FUNCTION NAME: _hal_sco_qos_setRateLimitCfg_callback
 * PURPOSE:
 *      Set rate limit configuration when connection is changed.
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      ptr_cookie      --  Cookie for additional information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */

static void
_hal_sco_qos_setRateLimitCfg_callback(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie)
{
    if (TRUE == link)
    {
        _hal_sco_qos_set_RateLimitToken(unit, port);
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_sco_qos_init
 * PURPOSE:
 *      Initialization functions of QoS.
 *
 * INPUT:
 *      unit            --  Device ID
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
hal_sco_qos_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    AIR_QOS_RATE_LIMIT_CFG_T rl_cfg;
    osal_memset(&rl_cfg, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
    /* Set egress rate limit of CPU port = 0x20 * 32Kbps = 1Mbps */
    if (HAL_IS_PORT_VALID(unit, HAL_CPU_PORT(unit)))
    {
        rl_cfg.flags = AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS;
        rl_cfg.egress_cir = 0x20;
        rc = hal_sco_qos_setRateLimitCfg(unit, HAL_CPU_PORT(unit), &rl_cfg);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "Configure egress limit control on CPU port failed!(%d)\n", rc);
        }
    }

    /* Set trust mode default value */
    rc = hal_sco_qos_setTrustMode(unit, AIR_QOS_TRUST_MODE_1P_PORT);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "Configure trust mode failed!(%d)\n", rc);
    }

    /* add callback function for check link status */
    rc = hal_sco_ifmon_register(unit, _hal_sco_qos_setRateLimitCfg_callback, NULL);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "register set rate limit cfg callback function fail, unit %u rc = %d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_qos_deinit
 * PURPOSE:
 *      Deinitialization functions of QoS.
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
hal_sco_qos_deinit(
    const UI32_T unit)
{
    UI32_T u32dat, mac_port;

    u32dat = HAL_SCO_QOS_UPW_DEFAULT_VALUE;
    aml_writeReg(unit, UPW, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PEM1_DEFAULT_VALUE;
    aml_writeReg(unit, PEM1, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PEM2_DEFAULT_VALUE;
    aml_writeReg(unit, PEM2, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PEM3_DEFAULT_VALUE;
    aml_writeReg(unit, PEM3, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PEM4_DEFAULT_VALUE;
    aml_writeReg(unit, PEM4, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM1_DEFAULT_VALUE;
    aml_writeReg(unit, PIM1, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM2_DEFAULT_VALUE;
    aml_writeReg(unit, PIM2, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM3_DEFAULT_VALUE;
    aml_writeReg(unit, PIM3, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM4_DEFAULT_VALUE;
    aml_writeReg(unit, PIM4, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM5_DEFAULT_VALUE;
    aml_writeReg(unit, PIM5, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM6_DEFAULT_VALUE;
    aml_writeReg(unit, PIM6, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_PIM7_DEFAULT_VALUE;
    aml_writeReg(unit, PIM7, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_GERLCR_DEFAULT_VALUE;
    aml_writeReg(unit, GERLCR, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_QOS_GIRLCR_DEFAULT_VALUE;
    aml_writeReg(unit, GIRLCR, &u32dat, sizeof(u32dat));
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), mac_port)
    {
        u32dat = HAL_SCO_QOS_ERLCR_DEFAULT_VALUE;
        aml_writeReg(unit, ERLCR(mac_port), &u32dat, sizeof(u32dat));
        u32dat = HAL_SCO_QOS_IRLCR_DEFAULT_VALUE;
        aml_writeReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));
        u32dat = HAL_SCO_QOS_PCR_DEFAULT_VALUE;
        aml_writeReg(unit, PCR(mac_port), &u32dat, sizeof(u32dat));
        u32dat = HAL_SCO_QOS_MMSCR0_DEFAULT_VALUE;
        aml_writeReg(unit, MMSCR0_Q0(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q1(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q2(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q3(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q4(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q5(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q6(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR0_Q7(mac_port), &u32dat, sizeof(u32dat));
        u32dat = HAL_SCO_QOS_MMSCR1_DEFAULT_VALUE;
        aml_writeReg(unit, MMSCR1_Q0(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q1(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q2(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q3(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q4(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q5(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q6(mac_port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, MMSCR1_Q7(mac_port), &u32dat, sizeof(u32dat));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_setRateLimitEnable
 * PURPOSE:
 *      Enable or disable port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 *      rate_en         --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
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
hal_sco_qos_setRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             enable)
{
    UI32_T u32dat = 0, reg = 0;
    UI32_T mac_port;

    /* Get ingress / egress register value */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    if (AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = ERLCR(mac_port);
    }
    else if (AIR_QOS_RATE_DIR_INGRESS == dir)
    {
        reg = IRLCR(mac_port);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_WARN, "Not Support this dir %d yet\n", dir);
        return AIR_E_BAD_PARAMETER;
    }

    aml_readReg(unit, reg, &u32dat, sizeof(u32dat));
    if (TRUE == enable)
    {
        u32dat |= BIT(REG_RATE_EN_OFFT);
        /* Enable tobke bucket mode */
        u32dat |= BIT(REG_TB_EN_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(REG_RATE_EN_OFFT));
        /* Disable tobke bucket mode */
        u32dat &= ~(BIT(REG_TB_EN_OFFT));
    }
    aml_writeReg(unit, reg, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_getRateLimitEnable
 * PURPOSE:
 *      Get port rate limit state.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_RATE_DIR_INGRESS
 *                          AIR_RATE_DIR_EGRESS
 * OUTPUT:
 *      ptr_rate_en     --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_rate_en)
{
    UI32_T u32dat = 0, reg = 0, ret = 0;
    UI32_T mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /* Get ingress / egress register value */
    if (AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = ERLCR(mac_port);
    }
    else
    {
        reg = IRLCR(mac_port);
    }
    aml_readReg(unit, reg, &u32dat, sizeof(u32dat));

    ret = (u32dat & BIT(REG_RATE_EN_OFFT));
    if (!ret)
    {
        *ptr_rate_en = FALSE;
    }
    else
    {
        *ptr_rate_en = TRUE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_setRateLimitCfg
 * PURPOSE:
 *      Set per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      ptr_cfg         --  AIR_QOS_RATE_LIMIT_CFG_T
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
hal_sco_qos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(ptr_cfg->egress_cir, HAL_SCO_QOS_MAX_CIR);
    HAL_CHECK_ENUM_RANGE(ptr_cfg->ingress_cir, HAL_SCO_QOS_MAX_CIR);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* For Egress rate setting */
    /* Set egress rate CIR */
    aml_readReg(unit, ERLCR(mac_port), &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    u32dat |= ptr_cfg->egress_cir;
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    /* Set token period to 1/4ms */
    u32dat &= ~BITS_RANGE(REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    u32dat |= BITS_OFF_L(HAL_SCO_QOS_TOKEN_PERIOD_1_4MS, REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    if (ptr_cfg->flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS)
    {
        /* Enable ratelimit mode*/
        u32dat |= BIT(REG_RATE_EN_OFFT);
    }
    aml_writeReg(unit, ERLCR(mac_port), &u32dat, sizeof(u32dat));

    /* For Ingress rate setting */
    /* Set ingress rate CIR */
    aml_readReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));
    u32dat &= ~BITS_RANGE(REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    u32dat |= ptr_cfg->ingress_cir;
    /* Enable tobke bucket mode */
    u32dat |= BIT(REG_TB_EN_OFFT);
    if (ptr_cfg->flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS)
    {
        /* Enable ratelimit mode*/
        u32dat |= BIT(REG_RATE_EN_OFFT);
    }
    aml_writeReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_getRateLimitCfg
 * PURPOSE:
 *      Get per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_cfg          --  AIR_QOS_RATE_LIMIT_CFG_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port;
    UI32_T tick;
    I32_T  scale;
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* For Egress rate info */
    aml_readReg(unit, ERLCR(mac_port), &u32dat, sizeof(u32dat));
    ptr_cfg->egress_cir = BITS_OFF_R(u32dat, REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    ptr_cfg->egress_cbs = ptr_cfg->egress_cir;

    /* For Ingress rate info */
    aml_readReg(unit, IRLCR(mac_port), &u32dat, sizeof(u32dat));
    ptr_cfg->ingress_cir = BITS_OFF_R(u32dat, REG_RATE_CIR_OFFT, REG_RATE_CIR_LENG);
    ptr_cfg->ingress_cbs = BITS_OFF_R(u32dat, REG_RATE_CBS_OFFT, REG_RATE_CBS_LENG);
    tick = BITS_OFF_R(u32dat, REG_RATE_TB_OFFT, REG_RATE_TB_LENG);
    /* Calculate the scale factor of bucket size */
    scale = (I32_T)tick - HAL_SCO_QOS_TOKEN_PERIOD_1_4MS;
    if (scale < 0)
    {
        scale *= (-1);
        ptr_cfg->ingress_cbs = ptr_cfg->ingress_cir / (1 << scale);
    }
    else
    {
        /* The maximum tick is 128 ms */
        if (scale > 9)
        {
            scale = 9;
        }
        ptr_cfg->ingress_cbs = ptr_cfg->ingress_cir * (1 << scale);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_setRateLimitExcludeMgmt
 * PURPOSE:
 *      Exclude/Include management frames to rate limit control.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_RATE_DIR_INGRESS
 *                          AIR_RATE_DIR_EGRESS
 *      exclude         --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
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
hal_sco_qos_setRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             exclude)
{
    UI32_T u32dat = 0, reg = 0;

    if (AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = GERLCR;
    }
    else
    {
        reg = GIRLCR;
    }

    /* Set to register */
    aml_readReg(unit, reg, &u32dat, sizeof(u32dat));
    if (TRUE == exclude)
    {
        u32dat |= BIT(REG_MFRM_EX_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(REG_MFRM_EX_OFFT));
    }
    aml_writeReg(unit, reg, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_getRateLimitExcludeMgmt
 * PURPOSE:
 *      Get rate limit control exclude/include management frames.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_RATE_DIR_INGRESS
 *                          AIR_RATE_DIR_EGRESS
 * OUTPUT:
 *      ptr_exclude     --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_exclude)
{
    UI32_T reg = 0, u32dat = 0;

    if (AIR_QOS_RATE_DIR_EGRESS == dir)
    {
        reg = GERLCR;
    }
    else
    {
        reg = GIRLCR;
    }

    /* Set to register */
    aml_readReg(unit, reg, &u32dat, sizeof(u32dat));
    if (BITS_OFF_R(u32dat, REG_MFRM_EX_OFFT, REG_MFRM_EX_LENG))
    {
        *ptr_exclude = TRUE;
    }
    else
    {
        *ptr_exclude = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_setTrustMode
 * PURPOSE:
 *      Set qos trust mode value.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      mode            --  Qos support mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setTrustMode(
    const UI32_T               unit,
    const AIR_QOS_TRUST_MODE_T mode)
{
    UI32_T                  rc = AIR_E_OTHERS;
    HAL_SCO_QOS_QUEUE_UPW_T stat;

    /*get register val*/
    rc = aml_readReg(unit, UPW, &(stat.byte), sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get rate trust weight success, UPW hex is %x\n", stat.byte);
    stat.byte = HAL_SCO_QOS_QUEUE_DEFAULT_VAL;
    if (AIR_E_OK == rc)
    {
        switch (mode)
        {
            case AIR_QOS_TRUST_MODE_PORT:
                stat.raw.csr_port_weight = HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_1P_PORT:
                stat.raw.csr_1p_weight = HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_port_weight = HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_DSCP_PORT:
                stat.raw.csr_dscp_weight = HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_port_weight = HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_DSCP_1P_PORT:
                stat.raw.csr_dscp_weight = HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_1p_weight = HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT;
                stat.raw.csr_port_weight = HAL_SCO_QOS_QUEUE_TRUST_LOW_WEIGHT;
                break;

            case AIR_QOS_TRUST_MODE_1P_DSCP_PORT:
                stat.raw.csr_1p_weight = HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT;
                stat.raw.csr_dscp_weight = HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT;
                stat.raw.csr_port_weight = HAL_SCO_QOS_QUEUE_TRUST_LOW_WEIGHT;
                break;

            default:
                DIAG_PRINT(HAL_DBG_WARN, "Not Support this mode %d yet\n", mode);
                return AIR_E_BAD_PARAMETER;
        }
    }

    /*set register val*/
    rc = aml_writeReg(unit, UPW, &stat.byte, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: set rate trust mode failed  rc is %d\n", rc);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set rate trust mode %d weight success, UPW hex is %x\n", mode, stat.byte);
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getTrustMode
 * PURPOSE:
 *      Get qos trust mode value.
 *
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_weight      --  All Qos weight value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getTrustMode(
    const UI32_T                unit,
    AIR_QOS_TRUST_MODE_T *const ptr_mode)
{
    UI32_T                  rc = AIR_E_OTHERS;
    HAL_SCO_QOS_QUEUE_UPW_T stat;

    /*get register val*/
    stat.byte = 0;
    *ptr_mode = AIR_QOS_TRUST_MODE_1P_PORT;
    rc = aml_readReg(unit, UPW, &(stat.byte), sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: get rate trust mode failed  rc is %d\n", rc);
    }
    else
    {
        if (HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_1p_weight)
        {
            if (HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_port_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_1P_PORT;
            }
            else if (HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_dscp_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_1P_DSCP_PORT;
            }
        }
        else if (HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_dscp_weight)
        {
            if (HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_1p_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_DSCP_1P_PORT;
            }
            else if (HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT == stat.raw.csr_port_weight)
            {
                *ptr_mode = AIR_QOS_TRUST_MODE_DSCP_PORT;
            }
        }
        else if (HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT == stat.raw.csr_port_weight)
        {
            *ptr_mode = AIR_QOS_TRUST_MODE_PORT;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Not support this trust mode successd, UPW hex is %x\n", stat.byte);
        }
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get trust mode success, UPW hex is %x\n", stat.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setPriToQueue
 * PURPOSE:
 *      Set qos pri to queue mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 *      queue           --  Qos Queue value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setPriToQueue(
    const UI32_T unit,
    const UI32_T pri,
    const UI32_T queue)
{
    UI32_T                  rc = AIR_E_OTHERS;
    HAL_SCO_QOS_QUEUE_PEM_T stat;

    /*get register val*/
    switch (pri / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEM1, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM1, &stat.byte, sizeof(UI32_T));
            break;

        case 1:
            rc = aml_readReg(unit, PEM2, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM2, &stat.byte, sizeof(UI32_T));
            break;

        case 2:
            rc = aml_readReg(unit, PEM3, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM3, &stat.byte, sizeof(UI32_T));
            break;

        case 3:
            rc = aml_readReg(unit, PEM4, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    stat.raw.csr_que_cpu_h = queue;
                }
                else
                {
                    stat.raw.csr_que_cpu_l = queue;
                }
            }
            rc = aml_writeReg(unit, PEM4, &stat.byte, sizeof(UI32_T));
            break;

        default:
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: Not Support this pri %d yet\n", pri);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set pri %d to queue %d success, PEM hex is %x\n", pri, queue, stat.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getPriToQueue
 * PURPOSE:
 *      Get qos pri to queue mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 * OUTPUT:
 *      ptr_queue       --  Qos pri mapping Queue value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getPriToQueue(
    const UI32_T  unit,
    const UI32_T  pri,
    UI32_T *const ptr_queue)
{
    UI32_T                  rc = AIR_E_OTHERS;
    HAL_SCO_QOS_QUEUE_PEM_T stat;

    /*get register val*/
    stat.byte = 0;
    switch (pri / 2)
    {
        case 0:
            rc = aml_readReg(unit, PEM1, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 1:
            rc = aml_readReg(unit, PEM2, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 2:
            rc = aml_readReg(unit, PEM3, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        case 3:
            rc = aml_readReg(unit, PEM4, &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                if (1 == pri % 2)
                {
                    *ptr_queue = stat.raw.csr_que_cpu_h;
                }
                else
                {
                    *ptr_queue = stat.raw.csr_que_cpu_l;
                }
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: Not Support this pri %d yet\n", pri);
            return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: get pri to queue failed  rc is %d\n", rc);
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get pri %d to queue %d mode success, PEM hex is %x\n", pri, *ptr_queue, stat.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setDscpToPri
 * PURPOSE:
 *      Set qos dscp to pri mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Qos dscp value
 *      pri             --  Qos pri value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setDscpToPri(
    const UI32_T unit,
    const UI32_T dscp,
    const UI32_T pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T reg = 0;

    /*get register val*/
    switch (dscp / 10)
    {
        case 0:
            rc = aml_readReg(unit, PIM1, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << 3 * (dscp % 10);
                rc = aml_writeReg(unit, PIM1, &reg, sizeof(UI32_T));
            }
            break;

        case 1:
            rc = aml_readReg(unit, PIM2, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM2, &reg, sizeof(UI32_T));
            }
            break;

        case 2:
            rc = aml_readReg(unit, PIM3, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM3, &reg, sizeof(UI32_T));
            }
            break;

        case 3:
            rc = aml_readReg(unit, PIM4, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM4, &reg, sizeof(UI32_T));
            }
            break;

        case 4:
            rc = aml_readReg(unit, PIM5, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM5, &reg, sizeof(UI32_T));
            }
            break;

        case 5:
            rc = aml_readReg(unit, PIM6, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM6, &reg, sizeof(UI32_T));
            }
            break;

        case 6:
            rc = aml_readReg(unit, PIM7, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                reg &= ~(HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10));
                reg |= pri << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
                rc = aml_writeReg(unit, PIM7, &reg, sizeof(UI32_T));
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this dscp %d to pri, rc is %d\n", dscp, rc);
            return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "set dscp to pri failed ,rc is %d\n", rc);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "set dscp  %u to pri %u success, PIM hex is %x\n", dscp, pri, reg);
    }
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getDscpToPri
 * PURPOSE:
 *      Get qos dscp to pri mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Qos dscp value
 * OUTPUT:
 *      ptr_pri         --  Qos dscp mapping pri value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getDscpToPri(
    const UI32_T  unit,
    const UI32_T  dscp,
    UI32_T *const ptr_pri)
{
    UI32_T rc = AIR_E_OTHERS;
    UI32_T reg;

    /*get register val*/
    switch (dscp / 10)
    {
        case 0:
            rc = aml_readReg(unit, PIM1, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 1:
            rc = aml_readReg(unit, PIM2, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 2:
            rc = aml_readReg(unit, PIM3, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 3:
            rc = aml_readReg(unit, PIM4, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 4:
            rc = aml_readReg(unit, PIM5, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 5:
            rc = aml_readReg(unit, PIM6, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        case 6:
            rc = aml_readReg(unit, PIM7, &reg, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_pri = (reg & (HAL_SCO_QOS_QUEUE_PIM_MASK << HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10))) >>
                           HAL_SCO_QOS_QUEUE_PIM_WIDTH * (dscp % 10);
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this dscp %d to pri, rc is %d\n", dscp, rc);
            return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: get dscp %d to pri failed, rc is %d\n", dscp, rc);
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get dscp %u to pri %d success, PIM hex is %d \n", dscp, *ptr_pri, reg);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setScheduleMode
 * PURPOSE:
 *      Set schedule mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      sch_mode        --  AIR_QOS_SCH_MODE_T
 *      weight          --  weight for WRR/WFQ
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Weight default value is 1, only for WRR/WFQ mode
 */
AIR_ERROR_NO_T
hal_sco_qos_setScheduleMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             queue,
    const AIR_QOS_SCH_MODE_T sch_mode,
    const UI32_T             weight)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;
    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    /*Para check*/
    if (AIR_QOS_SHAPER_NOSETTING != weight)
    {
        HAL_CHECK_MIN_MAX_RANGE(weight, HAL_SCO_QOS_SHAPER_RATE_MIN_WEIGHT, HAL_SCO_QOS_SHAPER_RATE_MAX_WEIGHT);
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Get para*/
    switch (sch_mode)
    {
        case AIR_QOS_SCH_MODE_SP:
            min_v.raw.min_sp_wrr_q = 1;
            min_v.raw.min_rate_en = 0;
            break;

        case AIR_QOS_SCH_MODE_WRR:
            min_v.raw.min_sp_wrr_q = 0;
            min_v.raw.min_rate_en = 0;
            min_v.raw.min_weight = weight - 1;
            break;

        case AIR_QOS_SCH_MODE_WFQ:
            min_v.raw.min_sp_wrr_q = 1;
            min_v.raw.min_rate_en = 1;
            min_v.raw.min_rate_man = 0;
            min_v.raw.min_rate_exp = 0;

            max_v.raw.max_rate_en = 0;
            max_v.raw.max_sp_wfq_q = 0;
            max_v.raw.max_weight = weight - 1;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this mode %d num, please check again\n", sch_mode);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send to driver*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_writeReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_writeReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_writeReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_writeReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_writeReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_writeReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_writeReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_writeReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "Set schedule mode success,port is %d, queue is %d, min hex is %x, max hex is %x\n", port,
               queue, min_v.byte, max_v.byte);

    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getScheduleMode
 * PURPOSE:
 *      Get schedule mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_sch_mode    --  AIR_QOS_SCH_MODE_T
 *      ptr_weight      --  weight for WRR/WFQ
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *     None
 */
AIR_ERROR_NO_T
hal_sco_qos_getScheduleMode(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        queue,
    AIR_QOS_SCH_MODE_T *ptr_sch_mode,
    UI32_T             *ptr_weight)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;

    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    /*Read register data*/
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send para*/
    if ((min_v.raw.min_rate_en) && HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ == max_v.raw.max_sp_wfq_q)
    {
        *ptr_sch_mode = AIR_QOS_SCH_MODE_WFQ;
        *ptr_weight = max_v.raw.max_weight + 1;
    }
    else
    {
        if (HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR == min_v.raw.min_sp_wrr_q)
        {
            *ptr_sch_mode = AIR_QOS_SCH_MODE_WRR;
            *ptr_weight = min_v.raw.min_weight + 1;
        }
        else if (HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP == min_v.raw.min_sp_wrr_q)
        {
            *ptr_sch_mode = AIR_QOS_SCH_MODE_SP;
            *ptr_weight = AIR_QOS_SHAPER_NOSETTING;
        }
    }
    DIAG_PRINT(HAL_DBG_INFO, "Get schedule mode success,port is %d, queue is %d, min hex is %x, max hex is %x\n", port,
               queue, min_v.byte, max_v.byte);

    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setShaperMode
 * PURPOSE:
 *      Set min/max shaper mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      min             --  Min shaper mode
 *      max             --  Max shaper mode
 *      exceed          --  Exceed shaper mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       queue,
    const AIR_QOS_SHAPER_MODE_T        min,
    const AIR_QOS_SHAPER_MODE_T        max,
    const AIR_QOS_EXCEED_SHAPER_MODE_T exceed)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;
    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Get para*/
    if (AIR_QOS_SHAPER_MODE_SP == min)
    {
        min_v.raw.min_sp_wrr_q = HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP;
        min_v.raw.min_rate_en = TRUE;
        if (AIR_QOS_SHAPER_MODE_SP == max)
        {
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                /*exceed drop mode need  ratelimit value*/
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_WFQ == max)
        {
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_DISABLE == max)
        {
            /*No max mode ,No exceed*/
            min_v.raw.min_rate_en = FALSE;
            max_v.raw.max_rate_en = FALSE;
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Not support this value for max shaper, please check again\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_QOS_SHAPER_MODE_WRR == min)
    {
        min_v.raw.min_sp_wrr_q = HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR;
        min_v.raw.min_rate_en = TRUE;
        if (AIR_QOS_SHAPER_MODE_SP == max)
        {
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                /*exceed drop mode need  ratelimit value*/
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_WFQ == max)
        {
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_DISABLE == max)
        {
            /*No max mode ,No exceed*/
            min_v.raw.min_rate_en = FALSE;
            max_v.raw.max_rate_en = FALSE;
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Not support this value for max shaper, please check again\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_QOS_SHAPER_MODE_DISABLE == min)
    {
        min_v.raw.min_rate_en = FALSE;
        min_v.raw.min_sp_wrr_q = HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP;
        if (AIR_QOS_SHAPER_MODE_SP == max)
        {
            /*use max mode, need min ratelimit & value == 0*/
            min_v.raw.min_rate_exp = FALSE;
            min_v.raw.min_rate_man = FALSE;
            min_v.raw.min_rate_en = TRUE;
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                /*exceed drop mode need  ratelimit value*/
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_WFQ == max)
        {
            /*use max mode, need min ratelimit & value == 0*/
            min_v.raw.min_rate_exp = FALSE;
            min_v.raw.min_rate_man = FALSE;
            min_v.raw.min_rate_en = TRUE;
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ;
            max_v.raw.max_rate_en = TRUE;
            if (AIR_QOS_EXCEED_SHAPER_MODE_SP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
            }
            else if (AIR_QOS_EXCEED_SHAPER_MODE_DROP == exceed)
            {
                max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_DROP;
            }
            else
            {
                /*No need exceed , So no need max ratelimit*/
                max_v.raw.max_rate_en = FALSE;
            }
        }
        else if (AIR_QOS_SHAPER_MODE_DISABLE == max)
        {
            /*No max mode ,No exceed*/
            max_v.raw.max_rate_en = FALSE;
            max_v.raw.max_sp_wfq_q = HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP;
            max_v.raw.max_excess_en = HAL_SCO_QOS_MAX_EXCESS_SP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Not support this value for max shaper, please check again\n");
            return AIR_E_BAD_PARAMETER;
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_WARN, "Not support this value for min shaper, please check again\n");
        return AIR_E_BAD_PARAMETER;
    }

    /*Send to driver*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_writeReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_writeReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_writeReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_writeReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_writeReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_writeReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_writeReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_writeReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "Set shaper mode success,port is %d, queue is %d, min hex is %x, max hex is %x\n", port,
               queue, min_v.byte, max_v.byte);

    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getShaperMode
 * PURPOSE:
 *      Get min/max shaper mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_min         --  min shaper mode
 *      ptr_max         --  min shaper mode
 *      ptr_exceed      --  exceed shaper mode
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperMode(
    const UI32_T                        unit,
    const UI32_T                        port,
    const UI32_T                        queue,
    AIR_QOS_SHAPER_MODE_T *const        ptr_min,
    AIR_QOS_SHAPER_MODE_T *const        ptr_max,
    AIR_QOS_EXCEED_SHAPER_MODE_T *const ptr_exceed)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;

    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    /*Read register data*/
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send para */
    /*Min shaper setting*/
    /*Only min_ratelimit==0 means min disable*/
    if ((!min_v.raw.min_rate_man) && (!min_v.raw.min_rate_exp) && (min_v.raw.min_rate_en))
    {
        *ptr_min = AIR_QOS_SHAPER_MODE_DISABLE;
    }
    else
    {
        if (HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR == min_v.raw.min_sp_wrr_q)
        {
            *ptr_min = AIR_QOS_SHAPER_MODE_WRR;
        }
        else if (HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP == min_v.raw.min_sp_wrr_q)
        {
            *ptr_min = AIR_QOS_SHAPER_MODE_SP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Invalid port %d queue %d min shaper value", port, queue);
            *ptr_min = AIR_QOS_SHAPER_MODE_DISABLE;
            return AIR_E_BAD_PARAMETER;
        }
    }

    /*Max shaper setting*/
    /*No en min rate, no max mode*/
    if (!min_v.raw.min_rate_en)
    {
        *ptr_max = AIR_QOS_SHAPER_MODE_DISABLE;
    }
    else
    {
        if (HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ == max_v.raw.max_sp_wfq_q)
        {
            *ptr_max = AIR_QOS_SHAPER_MODE_WFQ;
        }
        else if (HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP == max_v.raw.max_sp_wfq_q)
        {
            *ptr_max = AIR_QOS_SHAPER_MODE_SP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Invalid port %d queue %d max shaper value", port, queue);
            return AIR_E_BAD_PARAMETER;
        }
    }

    /*Exceed value*/
    if (!max_v.raw.max_rate_en)
    {
        *ptr_exceed = AIR_QOS_EXCEED_SHAPER_MODE_DISABLE;
    }
    else
    {
        if (HAL_SCO_QOS_MAX_EXCESS_SP == max_v.raw.max_excess_en)
        {
            *ptr_exceed = AIR_QOS_EXCEED_SHAPER_MODE_SP;
        }
        else if (HAL_SCO_QOS_MAX_EXCESS_DROP == max_v.raw.max_excess_en)
        {
            *ptr_exceed = AIR_QOS_EXCEED_SHAPER_MODE_DROP;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Invalid port %d queue %d exceed shaper value", port, queue);
            return AIR_E_BAD_PARAMETER;
        }
    }

    DIAG_PRINT(HAL_DBG_INFO, "Get port %d queue %d shaper value , min hex is %x, max hex is %x\n", port, queue,
               min_v.byte, max_v.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setShaperRate
 * PURPOSE:
 *      Set shaper rate of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      ptr_shaper_cfg  --  AIR_QOS_SHAPER_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      ratelimit value = rate_man * 10 ^ rate_exp
 *      rate_man range 0 ~ 2^17
 *      rate_exp range 0 ~ 4, AIR_QOS_SHAPER_NOSETTING value means no setting
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperRate(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;

    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    /*Para check*/

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Get para*/
    /*Min check*/
    if (AIR_QOS_SHAPER_NOSETTING != ptr_shaper_cfg->min_rate)
    {
        /*for new feature, use max man & min exp to check boundary*/
        if (ptr_shaper_cfg->min_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN)
        {
            min_v.raw.min_rate_exp = 0;
            min_v.raw.min_rate_man = ptr_shaper_cfg->min_rate;
        }
        else if ((ptr_shaper_cfg->min_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10) &&
                 ptr_shaper_cfg->min_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN)
        {
            min_v.raw.min_rate_exp = 1;
            min_v.raw.min_rate_man = ptr_shaper_cfg->min_rate / 10;
        }
        else if ((ptr_shaper_cfg->min_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 100) &&
                 ptr_shaper_cfg->min_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10)
        {
            min_v.raw.min_rate_exp = 2;
            min_v.raw.min_rate_man = ptr_shaper_cfg->min_rate / 100;
        }
        else if ((ptr_shaper_cfg->min_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 1000) &&
                 ptr_shaper_cfg->min_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 100)
        {
            min_v.raw.min_rate_exp = 3;
            min_v.raw.min_rate_man = ptr_shaper_cfg->min_rate / 1000;
        }
        else if ((ptr_shaper_cfg->min_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10000) &&
                 ptr_shaper_cfg->min_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 1000)
        {
            min_v.raw.min_rate_exp = 4;
            min_v.raw.min_rate_man = ptr_shaper_cfg->min_rate / 10000;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate out of range, plz check parameter\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    /*Max check*/
    if (AIR_QOS_SHAPER_NOSETTING != ptr_shaper_cfg->max_rate)
    {
        /*for new feature, use max man & min exp to check boundary*/
        if (ptr_shaper_cfg->max_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN)
        {
            max_v.raw.max_rate_exp = 0;
            max_v.raw.max_rate_man = ptr_shaper_cfg->max_rate;
        }
        else if ((ptr_shaper_cfg->max_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10) &&
                 ptr_shaper_cfg->max_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN)
        {
            max_v.raw.max_rate_exp = 1;
            max_v.raw.max_rate_man = ptr_shaper_cfg->max_rate / 10;
        }
        else if ((ptr_shaper_cfg->max_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 100) &&
                 ptr_shaper_cfg->max_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10)
        {
            max_v.raw.max_rate_exp = 2;
            max_v.raw.max_rate_man = ptr_shaper_cfg->max_rate / 100;
        }
        else if ((ptr_shaper_cfg->max_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 1000) &&
                 ptr_shaper_cfg->max_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 100)
        {
            max_v.raw.max_rate_exp = 3;
            max_v.raw.max_rate_man = ptr_shaper_cfg->max_rate / 1000;
        }
        else if ((ptr_shaper_cfg->max_rate <= HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 10000) &&
                 ptr_shaper_cfg->max_rate > HAL_SCO_QOS_SHAPER_RATE_MAX_MAN * 1000)
        {
            max_v.raw.max_rate_exp = 4;
            max_v.raw.max_rate_man = ptr_shaper_cfg->max_rate / 10000;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate out of range, plz check parameter\n");
            return AIR_E_BAD_PARAMETER;
        }
    }

    /*Send to driver*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_writeReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_writeReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_writeReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_writeReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_writeReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_writeReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_writeReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_writeReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper rate failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "Set shaper rate port %d queue %d success, min hex is %X, max hex is %x\n", port, queue,
               min_v.byte, max_v.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getShaperRate
 * PURPOSE:
 *      Get shaper rate of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_shaper_cfg  --  AIR_QOS_SHAPER_CFG_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperRate(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;
    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send para*/
    /*Min shaper*/
    ptr_shaper_cfg->min_rate = min_v.raw.min_rate_man;
    while (0 != min_v.raw.min_rate_exp)
    {
        ptr_shaper_cfg->min_rate = ptr_shaper_cfg->min_rate * 10;
        min_v.raw.min_rate_exp--;
    }

    /*Max shaper*/
    ptr_shaper_cfg->max_rate = max_v.raw.max_rate_man;
    while (0 != max_v.raw.max_rate_exp)
    {
        ptr_shaper_cfg->max_rate = ptr_shaper_cfg->max_rate * 10;
        max_v.raw.max_rate_exp--;
    }

    DIAG_PRINT(HAL_DBG_INFO, "Get shaper rate port %d queue %d success, min hex is %X, max hex is %x\n", port, queue,
               min_v.byte, max_v.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setShaperWeight
 * PURPOSE:
 *      Set shaper weight of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      weight          --  AIR_QOS_SHAPER_WEIGHT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      ratelimit weight range 0~127
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperWeight(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  queue,
    const AIR_QOS_SHAPER_WEIGHT_T weight)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;

    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    /*Para check*/
    if (AIR_QOS_SHAPER_NOSETTING != weight.min_weight)
    {
        HAL_CHECK_MIN_MAX_RANGE(weight.min_weight, HAL_SCO_QOS_SHAPER_RATE_MIN_WEIGHT,
                                HAL_SCO_QOS_SHAPER_RATE_MAX_WEIGHT);
    }

    if (AIR_QOS_SHAPER_NOSETTING != weight.max_weight)
    {
        HAL_CHECK_MIN_MAX_RANGE(weight.max_weight, HAL_SCO_QOS_SHAPER_RATE_MIN_WEIGHT,
                                HAL_SCO_QOS_SHAPER_RATE_MAX_WEIGHT);
    }

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Get para*/
    /*Min check*/
    if (AIR_QOS_SHAPER_NOSETTING != weight.min_weight)
    {
        min_v.raw.min_weight = weight.min_weight - 1;
    }

    /*Max check*/
    if (AIR_QOS_SHAPER_NOSETTING != weight.max_weight)
    {
        max_v.raw.max_weight = weight.max_weight - 1;
    }

    /*Send to driver*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_writeReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_writeReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_writeReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_writeReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_writeReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_writeReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_writeReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_writeReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_writeReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Set shaper weight failed, rc is %d\n", rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again\n", queue);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "Set shaper weight port %d queue %d success, min hex is %X, max hex is %x\n", port, queue,
               min_v.byte, max_v.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_getShaperWeight
 * PURPOSE:
 *      Get shaper weight of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_weight      --  AIR_QOS_SHAPER_WEIGHT_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperWeight(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   queue,
    AIR_QOS_SHAPER_WEIGHT_T *const ptr_weight)
{
    UI32_T                   rc = AIR_E_OK;
    UI32_T                   mac_port;

    HAL_SCO_QOS_SHAPER_MIN_T min_v;
    HAL_SCO_QOS_SHAPER_MAX_T max_v;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    /*Read register data*/
    switch (queue)
    {
        case HAL_SCO_QOS_QUEUE_0:
            rc += aml_readReg(unit, MMSCR0_Q0(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q0(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_1:
            rc += aml_readReg(unit, MMSCR0_Q1(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q1(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_2:
            rc += aml_readReg(unit, MMSCR0_Q2(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q2(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_3:
            rc += aml_readReg(unit, MMSCR0_Q3(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q3(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_4:
            rc += aml_readReg(unit, MMSCR0_Q4(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q4(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_5:
            rc += aml_readReg(unit, MMSCR0_Q5(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q5(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_6:
            rc += aml_readReg(unit, MMSCR0_Q6(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q6(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        case HAL_SCO_QOS_QUEUE_7:
            rc += aml_readReg(unit, MMSCR0_Q7(mac_port), &min_v.byte, sizeof(UI32_T));
            rc += aml_readReg(unit, MMSCR1_Q7(mac_port), &max_v.byte, sizeof(UI32_T));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "Get port %d queue %d failed, rc is %d", port, queue, rc);
                return AIR_E_OTHERS;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "Not Support this queue %d num, please check again", queue);
            return AIR_E_BAD_PARAMETER;
    }

    /*Send para*/
    /*Min shaper*/
    ptr_weight->min_weight = min_v.raw.min_weight + 1;

    /*Max shaper*/
    ptr_weight->max_weight = max_v.raw.max_weight + 1;

    DIAG_PRINT(HAL_DBG_INFO, "Get shaper weight port %d queue %d success, min hex is %X, max hex is %x\n", port, queue,
               min_v.byte, max_v.byte);
    return rc;
}

/* FUNCTION NAME: hal_sco_qos_setPortPriority
 * PURPOSE:
 *      Set port based qos priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      pri             --  Qos priority value
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
hal_sco_qos_setPortPriority(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pri)
{
    UI32_T regPCR;
    UI32_T mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PCR(mac_port), &regPCR, sizeof(UI32_T));
    regPCR &= ~PCR_PORT_PRI_MASK;
    regPCR |= (pri & PCR_PORT_PRI_RELMASK) << PCR_PORT_PRI_OFFT;
    aml_writeReg(unit, PCR(mac_port), &regPCR, sizeof(UI32_T));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_qos_getPortPriority
 * PURPOSE:
 *      Get port based qos priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 * OUTPUT:
 *      ptr_pri         --  Qos priority value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getPortPriority(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_pri)
{
    UI32_T regPCR;
    UI32_T mac_port;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, PCR(mac_port), &regPCR, sizeof(UI32_T));
    *ptr_pri = (regPCR >> PCR_PORT_PRI_OFFT) & PCR_PORT_PRI_RELMASK;

    return AIR_E_OK;
}
