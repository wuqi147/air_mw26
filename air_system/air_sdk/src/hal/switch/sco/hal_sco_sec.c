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

/* FILE NAME:  hal_sco_sec.c
 * PURPOSE:
 *  Implement SEC module HAL function.
 *
 * NOTES:
 * 1. Only Support 3grp in EN8851 API. Hardware Support 4 grp
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/switch/sco/hal_sco_sec.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/sco/hal_sco_l2.h>
#include <hal/switch/sco/hal_sco_port.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_SEC_BSR_CONTROL_DEFAULT_VALUE (0)
#define HAL_SCO_SEC_PSC_DEFAULT_VALUE         (0x001FFF00)
#define HAL_SCO_SEC_AGC_DEFAULT_VALUE         (0x1E0F1819)
#define HAL_SCO_SEC_MACLMT_DEFAULT_VALUE      (0)
#define HAL_SCO_SEC_MAUTHC_DEFAULT_VALUE      (0x00000200)
#define HAL_SCO_SEC_GVMC_DEFAULT_VALUE        (0xFFFFFFFF)

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_SCO_SEC_SET_NEW_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                       \
    {                                                                        \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));                \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));      \
    } while (0)

#define _HAL_SCO_SEC_SET_BIT(__out__, __cond__, __offset__) \
    do                                                      \
    {                                                       \
        if (__cond__)                                       \
        {                                                   \
            (__out__) |= BIT(__offset__);                   \
        }                                                   \
        else                                                \
        {                                                   \
            (__out__) &= ~BIT(__offset__);                  \
        }                                                   \
    } while (0)

#define _HAL_SCO_SEC_SET_RANGE(__out__, __cond__, __offset__, __length__) \
    do                                                                    \
    {                                                                     \
        if (__cond__)                                                     \
        {                                                                 \
            (__out__) |= BITS_RANGE((__offset__), (__length__));          \
        }                                                                 \
        else                                                              \
        {                                                                 \
            (__out__) &= ~BITS_RANGE((__offset__), (__length__));         \
        }                                                                 \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SEC, "hal_sco_sec.c");

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   _hal_sco_sec_setStormctlTokenSize
 * PURPOSE:
 *      Set sec modules seclimit token size register value.
 * INPUT:
 *      unit        -- unit id
 *      port      -- port id
 *      type        -- pkt mode(bc/mc/uc)
 *      token       -- token size register value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      none
 */
static AIR_ERROR_NO_T
_hal_sco_sec_setStormctlTokenSize(
    UI32_T                           unit,
    UI8_T                            port,
    HAL_SCO_SEC_STORMCTL_TOKENSIZE_T token,
    AIR_SEC_STORM_TYPE_T             type)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            rc = aml_writeReg(unit, ARL_PORT_BSR_TOKEN_10_ADDR(port), &token.csr_strm_bkt_sz_10, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR_TOKEN_100_ADDR(port), &token.csr_strm_bkt_sz_100, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR_TOKEN_1000_ADDR(port), &token.csr_strm_bkt_sz_1000, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR_TOKEN_2500_ADDR(port), &token.csr_strm_bkt_sz_2500, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UMC:
            rc = aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_10_ADDR(port), &token.csr_strm_bkt_sz_10, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_100_ADDR(port), &token.csr_strm_bkt_sz_100, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_1000_ADDR(port), &token.csr_strm_bkt_sz_1000, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_2500_ADDR(port), &token.csr_strm_bkt_sz_2500, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            rc = aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_10_ADDR(port), &token.csr_strm_bkt_sz_10, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_100_ADDR(port), &token.csr_strm_bkt_sz_100, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_1000_ADDR(port), &token.csr_strm_bkt_sz_1000, sizeof(UI32_T));
            rc = aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_2500_ADDR(port), &token.csr_strm_bkt_sz_2500, sizeof(UI32_T));
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : set sec stormctl tokensize unit=%d port=%d grp=%d FAIL!!!\n", unit, port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO,
               "[Dbg] : set sec stormctl tokensize unit=%d port=%d grp=%d\n"
               "stat_reghex_10=%x stat_reghex_100=%x stat_reghex_1000=%x stat_reghex_2500=%x end\n",
               unit, port, type, token.csr_strm_bkt_sz_10, token.csr_strm_bkt_sz_100, token.csr_strm_bkt_sz_1000,
               token.csr_strm_bkt_sz_2500);
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME: hal_sco_sec_init
 * PURPOSE:
 *      Initialization of SEC MAC table.
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
hal_sco_sec_init(
    const UI32_T unit)
{
    UI32_T                       value = 0;
    UI32_T                       token_10m = HAL_SCO_SEC_STORMCTL_10M_TOKEN_VAL;
    UI32_T                       token_100m = HAL_SCO_SEC_STORMCTL_100M_TOKEN_VAL;
    UI32_T                       token_1000m = HAL_SCO_SEC_STORMCTL_1000M_TOKEN_VAL;
    UI32_T                       token_2p5g = HAL_SCO_SEC_STORMCTL_2500M_TOKEN_VAL;
    UI32_T                       port;

    AIR_SEC_8021X_CFG_T          config;
    AIR_SEC_MAC_LIMIT_PORT_CFG_T mac_lmt_port_cfg;

    osal_memset(&config, 0, sizeof(AIR_SEC_8021X_CFG_T));
    config.flags |= AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE;
    config.auth_fwd = AIR_SEC_8021X_FWD_MAC;
    config.unauth_fwd = AIR_SEC_8021X_FWD_DROP;
    hal_sco_sec_set8021xGlobalCfg(unit, &config);

    /*Init for bc storm ctl*/
    /*set bc storm value to default*/
    /*set cfg&data value to 0*/
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg] : do sec init unit=%d start\n", unit);
    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
    {
        aml_writeReg(unit, ARL_PORT_BSR_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_ADDR(port), &value, sizeof(UI32_T));

        aml_writeReg(unit, ARL_PORT_BSR_EXT_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_EXT_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_EXT_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_EXT_ADDR(port), &value, sizeof(UI32_T));

        /*set token to default*/
        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_10_ADDR(port), &token_10m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_10_ADDR(port), &token_10m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_10_ADDR(port), &token_10m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_10_ADDR(port), &token_10m, sizeof(UI32_T));

        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_100_ADDR(port), &token_100m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_100_ADDR(port), &token_100m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_100_ADDR(port), &token_100m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_100_ADDR(port), &token_100m, sizeof(UI32_T));

        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_1000_ADDR(port), &token_1000m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_1000_ADDR(port), &token_1000m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_1000_ADDR(port), &token_1000m, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_1000_ADDR(port), &token_1000m, sizeof(UI32_T));

        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_2500_ADDR(port), &token_2p5g, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_2500_ADDR(port), &token_2p5g, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_2500_ADDR(port), &token_2p5g, sizeof(UI32_T));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_2500_ADDR(port), &token_2p5g, sizeof(UI32_T));
    }

    /* set the default value of port SA learning function is enabled */
    /* set the default value of port SA learning limit is disabled */
    /* set the default value of port SA learning limit as global SA learning limit */
    mac_lmt_port_cfg.flags = AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN;
    mac_lmt_port_cfg.sa_lmt_cnt = HAL_SCO_L2_MAX_ENTRY_NUM;
    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
    {
        hal_sco_sec_setMacLimitPortCfg(unit, port, &mac_lmt_port_cfg);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_deinit
 * PURPOSE:
 *      Deinitialization of SEC MAC table.
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
hal_sco_sec_deinit(
    const UI32_T unit)
{
    UI32_T port, u32dat;
    AIR_PORT_FOREACH(HAL_PORT_BMP_TOTAL(unit), port)
    {
        u32dat = HAL_SCO_SEC_BSR_CONTROL_DEFAULT_VALUE;
        aml_writeReg(unit, ARL_PORT_BSR_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR_EXT_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_EXT_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_EXT_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_EXT_ADDR(port), &u32dat, sizeof(u32dat));

        u32dat = HAL_SCO_SEC_STORMCTL_DEFAULT_TOKEN_VAL;
        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_10_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_10_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_10_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_10_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_100_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_100_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_100_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_100_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_1000_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_1000_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_1000_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_1000_ADDR(port), &u32dat, sizeof(u32dat));

        u32dat = HAL_SCO_SEC_STORMCTL_DEFAULT_2P5G_TOKEN_VAL;
        aml_writeReg(unit, ARL_PORT_BSR_TOKEN_2500_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR1_TOKEN_2500_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR2_TOKEN_2500_ADDR(port), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, ARL_PORT_BSR3_TOKEN_2500_ADDR(port), &u32dat, sizeof(u32dat));

        u32dat = HAL_SCO_SEC_PSC_DEFAULT_VALUE;
        aml_writeReg(unit, PSC(port), &u32dat, sizeof(u32dat));
    }
    u32dat = HAL_SCO_SEC_AGC_DEFAULT_VALUE;
    aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SEC_MACLMT_DEFAULT_VALUE;
    aml_writeReg(unit, MACLMTC, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, MACLMTTH, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SEC_MAUTHC_DEFAULT_VALUE;
    aml_writeReg(unit, MAUTHC, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_SEC_GVMC_DEFAULT_VALUE;
    aml_writeReg(unit, GVMC, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_set8021xGlobalMode
 * PURPOSE:
 *      Set 802.1x authentication base on MAC/port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_SEC_8021X_MODE_MAC:   802.1x authentication base on MAC
 *                          AIR_SEC_8021X_MODE_PORT:  802.1x authentication base on Port
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
hal_sco_sec_set8021xGlobalMode(
    const UI32_T               unit,
    const AIR_SEC_8021X_MODE_T mode)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    if (AIR_SEC_8021X_MODE_MAC == mode)
    {
        u32dat |= BIT(MAUTHC_MAC_AUTH_OFFSET);
    }
    else
    {
        u32dat &= ~BIT(MAUTHC_MAC_AUTH_OFFSET);
    }

    /* Write register */
    aml_writeReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_get8021xGlobalMode
 * PURPOSE:
 *      Get 802.1x authentication base on MAC/port.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_8021X_MODE_MAC:   802.1x authentication base on MAC
 *                          AIR_SEC_8021X_MODE_PORT:  802.1x authentication base on Port
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_get8021xGlobalMode(
    const UI32_T          unit,
    AIR_SEC_8021X_MODE_T *ptr_mode)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, MAUTHC_MAC_AUTH_OFFSET, MAUTHC_MAC_AUTH_LENGTH))
    {
        (*ptr_mode) = AIR_SEC_8021X_MODE_MAC;
    }
    else
    {
        (*ptr_mode) = AIR_SEC_8021X_MODE_PORT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_set8021xGlobalCfg
 * PURPOSE:
 *      Set global configurations of 802.1x authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_cfg         --  Structure of global configuration.
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
hal_sco_sec_set8021xGlobalCfg(
    const UI32_T               unit,
    const AIR_SEC_8021X_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    /* Check parameters */
    HAL_CHECK_ENUM_RANGE(ptr_cfg->auth_fwd, AIR_SEC_8021X_FWD_LAST);
    HAL_CHECK_ENUM_RANGE(ptr_cfg->unauth_fwd, AIR_SEC_8021X_FWD_LAST);

    /* Read register */
    aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE), AGC_MAC_MOVE_EN_OFFT);

    /* Write register */
    aml_writeReg(unit, AGC, &u32dat, sizeof(u32dat));

    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    switch (ptr_cfg->auth_fwd)
    {
        case AIR_SEC_8021X_FWD_MAC:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_AUTH_FW_MAC, MAUTHC_AUTH_FW_OFFSET, MAUTHC_AUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_DROP:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_AUTH_FW_DROP, MAUTHC_AUTH_FW_OFFSET, MAUTHC_AUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_CPU:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_AUTH_FW_CPU, MAUTHC_AUTH_FW_OFFSET, MAUTHC_AUTH_FW_LENGTH);
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : Unexpected value unit=%d auth_fwd=%x\n", unit, ptr_cfg->auth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    switch (ptr_cfg->unauth_fwd)
    {
        case AIR_SEC_8021X_FWD_MAC:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_UNAUTH_FW_MAC, MAUTHC_UNAUTH_FW_OFFSET, MAUTHC_UNAUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_DROP:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_UNAUTH_FW_DROP, MAUTHC_UNAUTH_FW_OFFSET, MAUTHC_UNAUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_CPU:
            _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, MAUTHC_UNAUTH_FW_CPU, MAUTHC_UNAUTH_FW_OFFSET, MAUTHC_UNAUTH_FW_LENGTH);
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : Unexpected value unit=%d unauth_fwd=%x\n", unit, ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    /* Write register */
    aml_writeReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_get8021xGlobalCfg
 * PURPOSE:
 *      Get global configurations of 802.1x authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of global configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_get8021xGlobalCfg(
    const UI32_T         unit,
    AIR_SEC_8021X_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T fwd_mode = 0;

    osal_memset(ptr_cfg, 0, sizeof(AIR_SEC_8021X_CFG_T));

    /* Read register */
    aml_readReg(unit, AGC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, AGC_MAC_MOVE_EN_OFFT, AGC_MAC_MOVE_EN_LENG))
    {
        ptr_cfg->flags |= AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE;
    }

    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    fwd_mode = BITS_OFF_R(u32dat, MAUTHC_AUTH_FW_OFFSET, MAUTHC_AUTH_FW_LENGTH);
    switch (fwd_mode)
    {
        case MAUTHC_AUTH_FW_MAC:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_MAC;
            break;
        case MAUTHC_AUTH_FW_DROP:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_DROP;
            break;
        case MAUTHC_AUTH_FW_CPU:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_CPU;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : Unexpected value unit=%d auth fwd_mode=%x\n", unit, ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    fwd_mode = BITS_OFF_R(u32dat, MAUTHC_UNAUTH_FW_OFFSET, MAUTHC_UNAUTH_FW_LENGTH);
    switch (fwd_mode)
    {
        case MAUTHC_UNAUTH_FW_MAC:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_MAC;
            break;
        case MAUTHC_UNAUTH_FW_DROP:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_DROP;
            break;
        case MAUTHC_UNAUTH_FW_CPU:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_CPU;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : Unexpected value unit=%d unauth fwd_mode=%x\n", unit,
                       ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_set8021xMacBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x MAC-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_cfg     --  Structure of MAC-based configuration.
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
hal_sco_sec_set8021xMacBasedCfg(
    const UI32_T                   unit,
    const AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg)
{
    UI32_T            u32dat = 0;
    AIR_PORT_BITMAP_T mac_pbmp;

    /* Permission of port moving */
    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_mac_cfg->flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH),
                         MAUTHC_AUTH_PORT_MOVE_OFFSET);

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_mac_cfg->flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH),
                         MAUTHC_UNAUTH_PORT_MOVE_OFFSET);

    /* Write register */
    aml_writeReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    /* Guset VLAN member */
    AIR_PORT_BITMAP_CLEAR(mac_pbmp);
    u32dat = BITS_OFF_L(mac_pbmp[0], GVMC_GMEM_OFFSET, GVMC_GMEM_LENGTH);

    /* Write register */
    aml_writeReg(unit, GVMC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_get8021xMacBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x MAC-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mac_cfg     --  Structure of MAC-based configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_get8021xMacBasedCfg(
    const UI32_T             unit,
    AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg)
{
    UI32_T            u32dat = 0;
    AIR_PORT_BITMAP_T mac_pbmp;

    osal_memset(ptr_mac_cfg, 0, sizeof(AIR_SEC_8021X_MAC_CFG_T));

    /* Permission of port moving */
    /* Read register */
    aml_readReg(unit, MAUTHC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, MAUTHC_AUTH_PORT_MOVE_OFFSET, MAUTHC_AUTH_PORT_MOVE_LENGTH))
    {
        ptr_mac_cfg->flags |= AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH;
    }
    if (!!BITS_OFF_R(u32dat, MAUTHC_UNAUTH_PORT_MOVE_OFFSET, MAUTHC_UNAUTH_PORT_MOVE_LENGTH))
    {
        ptr_mac_cfg->flags |= AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH;
    }

    /* Guset VLAN member */
    /* Clear MAC port bitmap */
    AIR_PORT_BITMAP_CLEAR(mac_pbmp);
    /* Read register */
    aml_readReg(unit, GVMC, &u32dat, sizeof(u32dat));
    mac_pbmp[0] = BITS_OFF_R(u32dat, GVMC_GMEM_OFFSET, GVMC_GMEM_LENGTH);
    /* No used code for compiling check */
    AIR_PORT_BITMAP_COPY(mac_pbmp, mac_pbmp);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_set8021xPortBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x Port-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      ptr_port_cfg    --  Structure of Port-based configuration.
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
hal_sco_sec_set8021xPortBasedCfg(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Check parameters */
    HAL_CHECK_ENUM_RANGE(ptr_port_cfg->rx_auth, AIR_SEC_8021X_AUTH_LAST);

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read register */
    aml_readReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, ptr_port_cfg->rx_auth, PSC_RX_LOCK_OFFSET, PSC_RX_LOCK_LENGTH);

    /* Write register */
    aml_writeReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_get8021xPortBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x Port-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_port_cfg --  Structure of Port-based configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_get8021xPortBasedCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    osal_memset(ptr_port_cfg, 0, sizeof(AIR_SEC_8021X_PORT_CFG_T));

    /* Read register */
    aml_readReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    ptr_port_cfg->rx_auth = BITS_OFF_R(u32dat, PSC_RX_LOCK_OFFSET, PSC_RX_LOCK_LENGTH);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_setMacLimitGlobalMode
 * PURPOSE:
 *      Set port security mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
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
hal_sco_sec_setMacLimitGlobalMode(
    const UI32_T unit,
    const BOOL_T enable)
{
    UI32_T u32dat = 0;

    DIAG_PRINT(HAL_DBG_INFO, "enable=%01X\n", enable);
    /* Read register */
    aml_readReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_BIT(u32dat, (TRUE == enable), MACLMTC_EN_OFFSET);

    /* Write register */
    aml_writeReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_getMacLimitGlobalMode
 * PURPOSE:
 *      Get port security mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_getMacLimitGlobalMode(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, MACLMTC_EN_OFFSET, MACLMTC_EN_LENGTH))
    {
        (*ptr_enable) = TRUE;
    }
    else
    {
        (*ptr_enable) = FALSE;
    }
    DIAG_PRINT(HAL_DBG_INFO, "ptr_enable=%01X\n", (*ptr_enable));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_setMacLimitGlobalCfg
 * PURPOSE:
 *      Set MAC limitation global configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_cfg         --  Structure of global configuration.
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
hal_sco_sec_setMacLimitGlobalCfg(
    const UI32_T                   unit,
    const AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    /* Check parameters */
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);

    /* Read register */
    aml_readReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT), MACLMTC_EXC_MNG_OFFSET);

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE),
                         MACLMTC_PORT_MV_DROP_OFFSET);

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL),
                         MACLMTC_SA_FULL_DROP_OFFSET);

    /* Write register */
    aml_writeReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    /* Read register */
    aml_readReg(unit, MACLMTTH, &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_RANGE(u32dat, (ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE),
                           MACLMTTH_PORT_MV_CPU_THR_OFFSET, MACLMTTH_PORT_MV_CPU_THR_LENGTH);

    _HAL_SCO_SEC_SET_RANGE(u32dat, (ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL),
                           MACLMTTH_SA_FULL_CPU_THR_OFFSET, MACLMTTH_SA_FULL_CPU_THR_LENGTH);

    /* Write register */
    aml_writeReg(unit, MACLMTTH, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_getMacLimitGlobalCfg
 * PURPOSE:
 *      Get MAC limitation global configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of global configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_getMacLimitGlobalCfg(
    const UI32_T             unit,
    AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    osal_memset(ptr_cfg, 0, sizeof(AIR_SEC_MAC_LIMIT_CFG_T));

    /* Read register */
    aml_readReg(unit, MACLMTC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, MACLMTC_EXC_MNG_OFFSET, MACLMTC_EXC_MNG_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT;
    }
    if (!!BITS_OFF_R(u32dat, MACLMTC_PORT_MV_DROP_OFFSET, MACLMTC_PORT_MV_DROP_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE;
    }
    if (!!BITS_OFF_R(u32dat, MACLMTC_SA_FULL_DROP_OFFSET, MACLMTC_SA_FULL_DROP_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL;
    }

    /* Read register */
    aml_readReg(unit, MACLMTTH, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, MACLMTTH_PORT_MV_CPU_THR_OFFSET, MACLMTTH_PORT_MV_CPU_THR_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE;
    }
    if (!!BITS_OFF_R(u32dat, MACLMTTH_SA_FULL_CPU_THR_OFFSET, MACLMTTH_SA_FULL_CPU_THR_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL;
    }
    DIAG_PRINT(HAL_DBG_INFO, "config.flags=%08X\n", ptr_cfg->flags);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_setMacLimitPortCfg
 * PURPOSE:
 *      Set MAC limitation port configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      ptr_cfg         --  Structure of port configuration.
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
hal_sco_sec_setMacLimitPortCfg(
    const UI32_T                        unit,
    const UI32_T                        port,
    const AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Check parameters */
    HAL_CHECK_MIN_MAX_RANGE(ptr_cfg->sa_lmt_cnt, 0, HAL_SCO_L2_MAX_ENTRY_NUM);

    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->sa_lmt_cnt=%08X\n", ptr_cfg->sa_lmt_cnt);

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read register */
    aml_readReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    _HAL_SCO_SEC_SET_BIT(u32dat, !(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN), PSC_DIS_LRN_OFFSET);

    _HAL_SCO_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT), PSC_SA_CNT_EN_OFFSET);

    _HAL_SCO_SEC_SET_NEW_VALUE(u32dat, ptr_cfg->sa_lmt_cnt, PSC_SA_CNT_LMT_OFFSET, PSC_SA_CNT_LMT_LENGTH);

    /* Write register */
    aml_writeReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_sec_getMacLimitPortCfg
 * PURPOSE:
 *      Get MAC limitation port configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of port configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_sec_getMacLimitPortCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    osal_memset(ptr_cfg, 0, sizeof(AIR_SEC_MAC_LIMIT_PORT_CFG_T));

    /* Read register */
    aml_readReg(unit, PSC(mac_port), &u32dat, sizeof(u32dat));

    if (!BITS_OFF_R(u32dat, PSC_DIS_LRN_OFFSET, PSC_DIS_LRN_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN;
    }
    if (!!BITS_OFF_R(u32dat, PSC_SA_CNT_EN_OFFSET, PSC_SA_CNT_EN_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
    }

    ptr_cfg->sa_lmt_cnt = BITS_OFF_R(u32dat, PSC_SA_CNT_LMT_OFFSET, PSC_SA_CNT_LMT_LENGTH);

    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->sa_lmt_cnt=%08X\n", ptr_cfg->sa_lmt_cnt);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_sec_setPortStormCtrl
 * PURPOSE:
 *      Enable/Disable sec modules ctrl function.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_UMC
 *                          AIR_SEC_STORM_TYPE_UUC
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_setPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    const BOOL_T               enable)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    HAL_SCO_SEC_STORMCTL_STAT_T stat;
    UI32_T                      mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_drop = enable;
                stat.raw.csr_strm_bc_inc = 1;
                stat.raw.csr_strm_mc_inc = 0;
                stat.raw.csr_strm_uc_inc = 0;
            }
            /* Write register */
            rc = aml_writeReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UMC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_drop = enable;
                stat.raw.csr_strm_bc_inc = 0;
                stat.raw.csr_strm_mc_inc = 1;
                stat.raw.csr_strm_uc_inc = 0;
            }
            /* Write register */
            rc = aml_writeReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_drop = enable;
                stat.raw.csr_strm_bc_inc = 0;
                stat.raw.csr_strm_mc_inc = 0;
                stat.raw.csr_strm_uc_inc = 1;
            }
            /* Write register */
            rc = aml_writeReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: set sec stormctl en unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec stormctl en unit=%d port=%d grp=%d stat_reghex=%x end\n", unit, mac_port,
               type, stat.byte);
    return rc;
}

/* FUNCTION NAME:   hal_sco_sec_getPortStormCtrl
 * PURPOSE:
 *      Get sec modules ctrl function Enable/Disable.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_UMC
 *                          AIR_SEC_STORM_TYPE_UUC
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_getPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    BOOL_T                    *ptr_enable)
{
    AIR_ERROR_NO_T              rc = AIR_E_OK;
    HAL_SCO_SEC_STORMCTL_STAT_T stat;
    UI32_T                      mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Check parameters */
    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_enable = stat.raw.csr_strm_drop;
            }
            break;

        case AIR_SEC_STORM_TYPE_UMC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_enable = stat.raw.csr_strm_drop;
            }
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                *ptr_enable = stat.raw.csr_strm_drop;
            }
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: get sec stormctl en unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get sec stormctl en unit=%d port=%d grp=%d stat_reghex=%x end\n", unit, mac_port,
               type, stat.byte);
    return rc;
}

/* FUNCTION NAME:   air_sec_setPortStormCtrlRate
 * PURPOSE:
 *      Set storm control rate of specific port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_MC
 *                          AIR_SEC_STORM_TYPE_UC
 *      mode            --  AIR_SEC_STORM_RATE_MODE_PPS
 *                          AIR_SEC_STORM_RATE_MODE_BPS
 *      rate            --  Storm control rate
 *                          (0 ~ 2500K, unit: packets or Kbps)
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_setPortStormCtrlRate(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_STORM_TYPE_T      type,
    const AIR_SEC_STORM_RATE_MODE_T mode,
    const UI32_T                    rate)
{
    AIR_ERROR_NO_T                   rc = AIR_E_OK;
    HAL_SCO_SEC_STORMCTL_STAT_T      stat;
    UI32_T                           mac_port = 0;

    UI32_T                           token_size = 0;
    UI32_T                           count = 0;
    HAL_SCO_SEC_STORMCTL_UNIT_T      strm_unit = HAL_SCO_SEC_STORMCTL_UNIT_LAST;
    HAL_SCO_SEC_STORMCTL_LINKSPEED_T speed;
    HAL_SCO_SEC_STORMCTL_TOKENSIZE_T token;
    UI32_T units[] = {HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_64, HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_256,
                      HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_1K, HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_4K,
                      HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_16K};
    UI32_T i, unit_cnt = 0;
    UI32_T set_rate = 0;

    /* Check parameters */
    HAL_CHECK_MIN_MAX_RANGE(rate, 0, HAL_SEC_STORM_RATE_MAX_VALUE);

    unit_cnt = sizeof(units) / sizeof(UI32_T);

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    DIAG_PRINT(HAL_DBG_INFO, "rate=%d\n", rate);
    /* Calcuate the storm control rate count/unit */
    /* rate:
     *  0       : 0
     *  1-64    : 64
     *  65-128  : 128*/
    for (i = 0; i < unit_cnt; i++)
    {
        if (rate > 0)
        {
            set_rate = ((rate - 1) / units[i] + 1) * units[i];
        }
        count = set_rate / units[i];
        DIAG_PRINT(HAL_DBG_INFO, "count=%d, set_rate=%d, unit=%d\n", count, set_rate, units[i]);
        if (count > (BIT(HAL_SCO_SEC_STORMCTL_RATE_CNT_LENGTH) - 1))
        {
            continue;
        }
        strm_unit = i;
        DIAG_PRINT(HAL_DBG_INFO, "strm_unit = %d\n", strm_unit);
        break;
    }

    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            rc = aml_readReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_unit = strm_unit;
                stat.raw.csr_strm_bc_inc = 1;
                stat.raw.csr_strm_mc_inc = 0;
                stat.raw.csr_strm_uc_inc = 0;
                stat.raw.csr_strm_mode = mode;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));

            /*set count*/
            rc = aml_readReg(unit, ARL_PORT_BSR_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                speed.raw.csr_strm_10m = count;
                speed.raw.csr_strm_100m = count;
                speed.raw.csr_strm_1g = count;
                speed.raw.csr_strm_2p5g = count;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));

            break;

        case AIR_SEC_STORM_TYPE_UMC:
            rc = aml_readReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_unit = strm_unit;
                stat.raw.csr_strm_bc_inc = 0;
                stat.raw.csr_strm_mc_inc = 1;
                stat.raw.csr_strm_uc_inc = 0;
                stat.raw.csr_strm_mode = mode;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));

            /*set count*/
            rc = aml_readReg(unit, ARL_PORT_BSR1_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                speed.raw.csr_strm_10m = count;
                speed.raw.csr_strm_100m = count;
                speed.raw.csr_strm_1g = count;
                speed.raw.csr_strm_2p5g = count;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR1_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            rc = aml_readReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                stat.raw.csr_strm_unit = strm_unit;
                stat.raw.csr_strm_bc_inc = 0;
                stat.raw.csr_strm_mc_inc = 0;
                stat.raw.csr_strm_uc_inc = 1;
                stat.raw.csr_strm_mode = mode;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));

            /*set count*/
            rc = aml_readReg(unit, ARL_PORT_BSR2_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                speed.raw.csr_strm_10m = count;
                speed.raw.csr_strm_100m = count;
                speed.raw.csr_strm_1g = count;
                speed.raw.csr_strm_2p5g = count;
            }
            rc = aml_writeReg(unit, ARL_PORT_BSR2_EXT_ADDR(mac_port), &speed.byte, sizeof(UI32_T));
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: set sec stormctl stat unit=%d port=%d grp=%d FAIL!!!\n", unit, port, type);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec stormctl stat unit=%d port=%d grp=%d stat_reghex=%x end\n", unit, port,
               type, stat.byte);

    /*set token*/
    if (AIR_SEC_STORM_RATE_MODE_PPS == mode)
    {
        switch (strm_unit)
        {
            case HAL_SCO_SEC_STORMCTL_UNIT_64:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_64 * count;
                break;

            case HAL_SCO_SEC_STORMCTL_UNIT_256:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_256 * count;
                break;

            case HAL_SCO_SEC_STORMCTL_UNIT_1000:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_1K * count;
                break;

            case HAL_SCO_SEC_STORMCTL_UNIT_4000:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_4K * count;
                break;

            case HAL_SCO_SEC_STORMCTL_UNIT_16000:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_16K * count;
                break;

            default:
                token_size = HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_16K * count;
        }

        if (token_size > HAL_SCO_SEC_STORMCTL_MAX_TOKEN_SIZE)
        {
            token_size = HAL_SCO_SEC_STORMCTL_MAX_TOKEN_SIZE;
        }

        token.csr_strm_bkt_sz_10 = token_size;
        token.csr_strm_bkt_sz_100 = token_size;
        token.csr_strm_bkt_sz_1000 = token_size;
        token.csr_strm_bkt_sz_2500 = token_size;
    }
    else
    {
        token.csr_strm_bkt_sz_10 = HAL_SCO_SEC_STORMCTL_10M_TOKEN_VAL;
        token.csr_strm_bkt_sz_100 = HAL_SCO_SEC_STORMCTL_100M_TOKEN_VAL;
        token.csr_strm_bkt_sz_1000 = HAL_SCO_SEC_STORMCTL_1000M_TOKEN_VAL;
        token.csr_strm_bkt_sz_2500 = HAL_SCO_SEC_STORMCTL_2500M_TOKEN_VAL;
    }

    rc = _hal_sco_sec_setStormctlTokenSize(unit, mac_port, token, type);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : set sec stormctl rate unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                   type);
    }
    return rc;
}

/* FUNCTION NAME:   air_sec_getPortStormCtrlRate
 * PURPOSE:
 *      Get storm control rate of specific port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_MC
 *                          AIR_SEC_STORM_TYPE_UC
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_STORM_RATE_MODE_PPS
 *                          AIR_SEC_STORM_RATE_MODE_BPS
 *      ptr_rate        --  Storm control rate
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_getPortStormCtrlRate(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    AIR_SEC_STORM_RATE_MODE_T *ptr_mode,
    UI32_T                    *ptr_rate)
{
    AIR_ERROR_NO_T                   rc = AIR_E_OK;
    HAL_SCO_SEC_STORMCTL_STAT_T      stat;
    UI32_T                           mac_port = 0;
    HAL_SCO_SEC_STORMCTL_UNIT_T      strm_unit = HAL_SCO_SEC_STORMCTL_UNIT_LAST;

    HAL_SCO_SEC_STORMCTL_LINKSPEED_T speed;
    UI32_T                           count = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                strm_unit = stat.raw.csr_strm_unit;
                *ptr_mode = stat.raw.csr_strm_mode;
            }
            /*  hal_sco_sec_setPortStormCtrlRate() will set storm control rate of all speed.
             *  So choice any one speed to calculate rate */
            /*get spd*/
            rc = aml_readReg(unit, ARL_PORT_BSR_EXT_ADDR(mac_port), &(speed.byte), sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                count = speed.raw.csr_strm_1g;
            }
            break;
        case AIR_SEC_STORM_TYPE_UMC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR1_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                strm_unit = stat.raw.csr_strm_unit;
                *ptr_mode = stat.raw.csr_strm_mode;
            }
            /*get spd*/
            rc = aml_readReg(unit, ARL_PORT_BSR1_EXT_ADDR(mac_port), &(speed.byte), sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                count = speed.raw.csr_strm_1g;
            }
            break;
        case AIR_SEC_STORM_TYPE_UUC:
            /* Read register */
            rc = aml_readReg(unit, ARL_PORT_BSR2_ADDR(mac_port), &stat.byte, sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                strm_unit = stat.raw.csr_strm_unit;
                *ptr_mode = stat.raw.csr_strm_mode;
            }
            /*get spd*/
            rc = aml_readReg(unit, ARL_PORT_BSR2_EXT_ADDR(mac_port), &(speed.byte), sizeof(UI32_T));
            if (AIR_E_OK == rc)
            {
                count = speed.raw.csr_strm_1g;
            }
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg] : get sec stormctl stat unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg] : get sec stormctl stat unit=%d port=%d grp=%d stat_reghex=%x end\n", unit,
               mac_port, type, stat.byte);

    switch (strm_unit)
    {
        case HAL_SCO_SEC_STORMCTL_UNIT_64:
            *ptr_rate = count * HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_64;
            break;
        case HAL_SCO_SEC_STORMCTL_UNIT_256:
            *ptr_rate = count * HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_256;
            break;
        case HAL_SCO_SEC_STORMCTL_UNIT_1000:
            *ptr_rate = count * HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_1K;
            break;
        case HAL_SCO_SEC_STORMCTL_UNIT_4000:
            *ptr_rate = count * HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_4K;
            break;
        case HAL_SCO_SEC_STORMCTL_UNIT_16000:
            *ptr_rate = count * HAL_SCO_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_16K;
            break;
        default:
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg] : Analyse storm control unit(%d) fail.\n", strm_unit);
            break;
    }
    if ((*ptr_rate) > HAL_SEC_STORM_RATE_MAX_VALUE)
    {
        *ptr_rate = HAL_SEC_STORM_RATE_MAX_VALUE;
    }
    return rc;
}

/* FUNCTION NAME:   hal_sco_sec_setStormCtrlMgmtMode
 * PURPOSE:
 *      Set Management mode of storm control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE
 *                          AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_setStormCtrlMgmtMode(
    const UI32_T                    unit,
    const AIR_SEC_STORM_CTRL_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         AGC_val;

    /* Read register */
    rc = aml_readReg(unit, AGC, &AGC_val, sizeof(UI32_T));
    _HAL_SCO_SEC_SET_BIT(AGC_val, (AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE == mode), CSR_BCSTRM_EXC_MG);
    /* Write register */
    rc = aml_writeReg(unit, AGC, &AGC_val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec mngmode unit=%d mode=%d end\n", unit, mode);
    return rc;
}

/* FUNCTION NAME:   hal_sco_sec_getStormCtrlMgmtMode
 * PURPOSE:
 *      Get Management mode of storm control.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE
 *                          AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_sec_getStormCtrlMgmtMode(
    const UI32_T               unit,
    AIR_SEC_STORM_CTRL_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         AGC_val;

    /* Read register */
    rc = aml_readReg(unit, AGC, &AGC_val, sizeof(UI32_T));

    if (AIR_E_OK == rc)
    {
        *ptr_mode = BITS_OFF_R(AGC_val, CSR_BCSTRM_EXC_MG, 1);
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get sec mngmode unit=%d mode=%d end\n", unit, *ptr_mode);
    return rc;
}
