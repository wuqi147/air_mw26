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

/* FILE NAME:  hal_coral_sec.c
 * PURPOSE:
 *  Implement SEC module HAL function.
 *
 * NOTES:
 * 1. Only Support 3grp in EN8851 API. Hardware Support 4 grp
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_sec.h>

#include <air_error.h>
#include <air_types.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/coral/hal_coral_l2.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define _HAL_CORAL_SEC_SET_NEW_VALUE(__out__, __val__, __offset__, __length__) \
    do                                                                         \
    {                                                                          \
        (__out__) &= ~BITS_RANGE((__offset__), (__length__));                  \
        (__out__) |= BITS_OFF_L((__val__), (__offset__), (__length__));        \
    } while (0)

#define _HAL_CORAL_SEC_SET_BIT(__out__, __cond__, __offset__) \
    do                                                        \
    {                                                         \
        if (__cond__)                                         \
        {                                                     \
            (__out__) |= BIT(__offset__);                     \
        }                                                     \
        else                                                  \
        {                                                     \
            (__out__) &= ~BIT(__offset__);                    \
        }                                                     \
    } while (0)

#define _HAL_CORAL_SEC_SET_RANGE(__out__, __cond__, __offset__, __length__) \
    do                                                                      \
    {                                                                       \
        if (__cond__)                                                       \
        {                                                                   \
            (__out__) |= BITS_RANGE((__offset__), (__length__));            \
        }                                                                   \
        else                                                                \
        {                                                                   \
            (__out__) &= ~BITS_RANGE((__offset__), (__length__));           \
        }                                                                   \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SEC, "hal_coral_sec.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _hal_coral_sec_calTokenSizeBps
 * PURPOSE:
 *      Calculate sec stormctrl bps token size value.
 * INPUT:
 *      unit        -- unit id
 *      port        -- port id
 *      token       -- token quantity
 * OUTPUT:
 *      bkt_sz      -- bucket size
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      if jumbo frame max size has been CHANGED, please reset hal_coral_sec_setStormctlLinkspdUnit() to change
 *      the token size automatically.
 */
static AIR_ERROR_NO_T
_hal_coral_sec_calTokenSizeBps(
    UI32_T  unit,
    UI32_T  token,
    UI32_T *bkt_sz)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         jumbo_len;
    UI32_T         regval;

    /*for tokenSize in bps, we use 125us period(8000/s), So we need token size over frame length */
    /*get jumbo frame length setting in register*/
    aml_readReg(unit, CORAL_GMACCR, &regval, sizeof(UI32_T));

    switch (regval & 0x3)
    {
        case 0:
            jumbo_len = 1522;
            break;

        case 1:
            jumbo_len = 1536;
            break;

        case 2:
            jumbo_len = 1552;
            break;

        case 3:
            switch (BITS_OFF_R(regval, 4, 4))
            {
                case 0:
                case 1:
                case 2:
                    jumbo_len = 2 * 1024;
                    break;
                case 3:
                    jumbo_len = 3 * 1024;
                    break;
                case 4:
                    jumbo_len = 4 * 1024;
                    break;
                case 5:
                    jumbo_len = 5 * 1024;
                    break;
                case 6:
                    jumbo_len = 6 * 1024;
                    break;
                case 7:
                    jumbo_len = 7 * 1024;
                    break;
                case 8:
                    jumbo_len = 8 * 1024;
                    break;
                case 9:
                    jumbo_len = 9 * 1024;
                    break;
                case 10:
                    jumbo_len = 12 * 1024;
                    break;
                case 11:
                    jumbo_len = 15 * 1024;
                    break;
                default:
                    jumbo_len = 16 * 1024;
            }
            break;
        default:
            jumbo_len = 1518;
    }
    if ((HAL_CORAL_SEC_STORMCTL_MAX_TOKEN_SIZE < (token * 1.5)) &&
        (HAL_CORAL_SEC_STORMCTL_MAX_TOKEN_SIZE < (jumbo_len * 1.5)))
    {
        *bkt_sz = HAL_CORAL_SEC_STORMCTL_MAX_TOKEN_SIZE;
    }
    else if (token > jumbo_len)
    {
        *bkt_sz = token * 1.5;
    }
    else
    {
        *bkt_sz = jumbo_len * 1.5;
    }
    DIAG_PRINT(HAL_DBG_INFO, "GMACCR=0x%x jumbo_len=%u, token=%u, bkt_sz=%u\n", regval, jumbo_len, token, *bkt_sz);
    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_sec_init
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
hal_coral_sec_init(
    const UI32_T unit)
{
    UI32_T              value = 0;
    UI32_T              default_bucket_size = 0x4000;
    UI32_T              port;

    AIR_SEC_8021X_CFG_T config;

    osal_memset(&config, 0, sizeof(AIR_SEC_8021X_CFG_T));
    config.flags |= AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE;
    config.auth_fwd = AIR_SEC_8021X_FWD_MAC;
    config.unauth_fwd = AIR_SEC_8021X_FWD_DROP;
    hal_coral_sec_set8021xGlobalCfg(unit, &config);

    /*Init for Storm Control */
    /*set bc storm value to default*/
    /*set cfg&data value to 0*/
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: do sec init unit=%d start\n", unit);
    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
    {
        aml_writeReg(unit, CORAL_ARL_PORT_BSR_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSR1_ADDR(port), &value, sizeof(UI32_T));

        aml_writeReg(unit, CORAL_ARL_PORT_BSR_EXT1_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSR1_EXT1_ADDR(port), &value, sizeof(UI32_T));

        aml_writeReg(unit, CORAL_ARL_PORT_BSR_EXT2_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSR1_EXT2_ADDR(port), &value, sizeof(UI32_T));

        aml_writeReg(unit, CORAL_ARL_PORT_BSR_EXT3_ADDR(port), &value, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSR1_EXT3_ADDR(port), &value, sizeof(UI32_T));

        /*set token to default*/
        aml_writeReg(unit, CORAL_ARL_PORT_BSTBS_ADDR(port), &default_bucket_size, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSTBS_EXT1_ADDR(port), &default_bucket_size, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSTBS_EXT2_ADDR(port), &default_bucket_size, sizeof(UI32_T));
        aml_writeReg(unit, CORAL_ARL_PORT_BSTBS_EXT3_ADDR(port), &default_bucket_size, sizeof(UI32_T));
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_deinit
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
hal_coral_sec_deinit(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_set8021xGlobalMode
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
hal_coral_sec_set8021xGlobalMode(
    const UI32_T               unit,
    const AIR_SEC_8021X_MODE_T mode)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    if (AIR_SEC_8021X_MODE_MAC == mode)
    {
        u32dat |= BIT(CORAL_MAUTHC_MAC_AUTH_OFFSET);
    }
    else
    {
        u32dat &= ~BIT(CORAL_MAUTHC_MAC_AUTH_OFFSET);
    }

    /* Write register */
    aml_writeReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_get8021xGlobalMode
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
hal_coral_sec_get8021xGlobalMode(
    const UI32_T          unit,
    AIR_SEC_8021X_MODE_T *ptr_mode)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_MAUTHC_MAC_AUTH_OFFSET, CORAL_MAUTHC_MAC_AUTH_LENGTH))
    {
        (*ptr_mode) = AIR_SEC_8021X_MODE_MAC;
    }
    else
    {
        (*ptr_mode) = AIR_SEC_8021X_MODE_PORT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_set8021xGlobalCfg
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
hal_coral_sec_set8021xGlobalCfg(
    const UI32_T               unit,
    const AIR_SEC_8021X_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    /* Check parameters */
    HAL_CHECK_ENUM_RANGE(ptr_cfg->auth_fwd, AIR_SEC_8021X_FWD_LAST);
    HAL_CHECK_ENUM_RANGE(ptr_cfg->unauth_fwd, AIR_SEC_8021X_FWD_LAST);

    /* Read register */
    aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE),
                           CORAL_AGC_MAC_MOVE_EN_OFFT);

    /* Write register */
    aml_writeReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));

    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    switch (ptr_cfg->auth_fwd)
    {
        case AIR_SEC_8021X_FWD_MAC:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_AUTH_FW_MAC, CORAL_MAUTHC_AUTH_FW_OFFSET,
                                         CORAL_MAUTHC_AUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_DROP:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_AUTH_FW_DROP, CORAL_MAUTHC_AUTH_FW_OFFSET,
                                         CORAL_MAUTHC_AUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_CPU:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_AUTH_FW_CPU, CORAL_MAUTHC_AUTH_FW_OFFSET,
                                         CORAL_MAUTHC_AUTH_FW_LENGTH);
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unexpected value unit=%d auth_fwd=%x\n", unit, ptr_cfg->auth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    switch (ptr_cfg->unauth_fwd)
    {
        case AIR_SEC_8021X_FWD_MAC:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_UNAUTH_FW_MAC, CORAL_MAUTHC_UNAUTH_FW_OFFSET,
                                         CORAL_MAUTHC_UNAUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_DROP:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_UNAUTH_FW_DROP, CORAL_MAUTHC_UNAUTH_FW_OFFSET,
                                         CORAL_MAUTHC_UNAUTH_FW_LENGTH);
            break;
        case AIR_SEC_8021X_FWD_CPU:
            _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, CORAL_MAUTHC_UNAUTH_FW_CPU, CORAL_MAUTHC_UNAUTH_FW_OFFSET,
                                         CORAL_MAUTHC_UNAUTH_FW_LENGTH);
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unexpected value unit=%d unauth_fwd=%x\n", unit, ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    /* Write register */
    aml_writeReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_get8021xGlobalCfg
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
hal_coral_sec_get8021xGlobalCfg(
    const UI32_T         unit,
    AIR_SEC_8021X_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T fwd_mode = 0;

    osal_memset(ptr_cfg, 0, sizeof(AIR_SEC_8021X_CFG_T));

    /* Read register */
    aml_readReg(unit, CORAL_AGC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_AGC_MAC_MOVE_EN_OFFT, CORAL_AGC_MAC_MOVE_EN_LENG))
    {
        ptr_cfg->flags |= AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE;
    }

    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    fwd_mode = BITS_OFF_R(u32dat, CORAL_MAUTHC_AUTH_FW_OFFSET, CORAL_MAUTHC_AUTH_FW_LENGTH);
    switch (fwd_mode)
    {
        case CORAL_MAUTHC_AUTH_FW_MAC:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_MAC;
            break;
        case CORAL_MAUTHC_AUTH_FW_DROP:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_DROP;
            break;
        case CORAL_MAUTHC_AUTH_FW_CPU:
            ptr_cfg->auth_fwd = AIR_SEC_8021X_FWD_CPU;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unexpected value unit=%d auth fwd_mode=%x\n", unit, ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    fwd_mode = BITS_OFF_R(u32dat, CORAL_MAUTHC_UNAUTH_FW_OFFSET, CORAL_MAUTHC_UNAUTH_FW_LENGTH);
    switch (fwd_mode)
    {
        case CORAL_MAUTHC_UNAUTH_FW_MAC:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_MAC;
            break;
        case CORAL_MAUTHC_UNAUTH_FW_DROP:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_DROP;
            break;
        case CORAL_MAUTHC_UNAUTH_FW_CPU:
            ptr_cfg->unauth_fwd = AIR_SEC_8021X_FWD_CPU;
            break;
        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unexpected value unit=%d unauth fwd_mode=%x\n", unit, ptr_cfg->unauth_fwd);
            return AIR_E_BAD_PARAMETER;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_set8021xMacBasedCfg
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
hal_coral_sec_set8021xMacBasedCfg(
    const UI32_T                   unit,
    const AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg)
{
    UI32_T            u32dat = 0;
    AIR_PORT_BITMAP_T mac_pbmp;

    /* Permission of port moving */
    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_mac_cfg->flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH),
                           CORAL_MAUTHC_AUTH_PORT_MOVE_OFFSET);

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_mac_cfg->flags & AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH),
                           CORAL_MAUTHC_UNAUTH_PORT_MOVE_OFFSET);

    /* Write register */
    aml_writeReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    /* Guset VLAN member */
    AIR_PORT_BITMAP_CLEAR(mac_pbmp);
    u32dat = BITS_OFF_L(mac_pbmp[0], CORAL_GVMC_GMEM_OFFSET, CORAL_GVMC_GMEM_LENGTH);

    /* Write register */
    aml_writeReg(unit, CORAL_GVMC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_get8021xMacBasedCfg
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
hal_coral_sec_get8021xMacBasedCfg(
    const UI32_T             unit,
    AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg)
{
    UI32_T            u32dat = 0;
    AIR_PORT_BITMAP_T mac_pbmp;

    osal_memset(ptr_mac_cfg, 0, sizeof(AIR_SEC_8021X_MAC_CFG_T));

    /* Permission of port moving */
    /* Read register */
    aml_readReg(unit, CORAL_MAUTHC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_MAUTHC_AUTH_PORT_MOVE_OFFSET, CORAL_MAUTHC_AUTH_PORT_MOVE_LENGTH))
    {
        ptr_mac_cfg->flags |= AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH;
    }
    if (!!BITS_OFF_R(u32dat, CORAL_MAUTHC_UNAUTH_PORT_MOVE_OFFSET, CORAL_MAUTHC_UNAUTH_PORT_MOVE_LENGTH))
    {
        ptr_mac_cfg->flags |= AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH;
    }

    /* Guset VLAN member */
    /* Clear MAC port bitmap */
    AIR_PORT_BITMAP_CLEAR(mac_pbmp);
    /* Read register */
    aml_readReg(unit, CORAL_GVMC, &u32dat, sizeof(u32dat));
    mac_pbmp[0] = BITS_OFF_R(u32dat, CORAL_GVMC_GMEM_OFFSET, CORAL_GVMC_GMEM_LENGTH);
    /* No used code for compiling check */
    AIR_PORT_BITMAP_COPY(mac_pbmp, mac_pbmp);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_set8021xPortBasedCfg
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
hal_coral_sec_set8021xPortBasedCfg(
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
    aml_readReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, ptr_port_cfg->rx_auth, CORAL_PSC_RX_LOCK_OFFSET, CORAL_PSC_RX_LOCK_LENGTH);

    /* Write register */
    aml_writeReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_get8021xPortBasedCfg
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
hal_coral_sec_get8021xPortBasedCfg(
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
    aml_readReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    ptr_port_cfg->rx_auth = BITS_OFF_R(u32dat, CORAL_PSC_RX_LOCK_OFFSET, CORAL_PSC_RX_LOCK_LENGTH);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_setMacLimitGlobalMode
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
hal_coral_sec_setMacLimitGlobalMode(
    const UI32_T unit,
    const BOOL_T enable)
{
    UI32_T u32dat = 0;

    DIAG_PRINT(HAL_DBG_INFO, "enable=%01X\n", enable);
    /* Read register */
    aml_readReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_BIT(u32dat, (TRUE == enable), CORAL_MACLMTC_EN_OFFSET);

    /* Write register */
    aml_writeReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_getMacLimitGlobalMode
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
hal_coral_sec_getMacLimitGlobalMode(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat = 0;

    /* Read register */
    aml_readReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTC_EN_OFFSET, CORAL_MACLMTC_EN_LENGTH))
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

/* FUNCTION NAME: hal_coral_sec_setMacLimitGlobalCfg
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
hal_coral_sec_setMacLimitGlobalCfg(
    const UI32_T                   unit,
    const AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    /* Check parameters */
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);

    /* Read register */
    aml_readReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT),
                           CORAL_MACLMTC_EXC_MNG_OFFSET);

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE),
                           CORAL_MACLMTC_PORT_MV_DROP_OFFSET);

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL),
                           CORAL_MACLMTC_SA_FULL_DROP_OFFSET);

    /* Write register */
    aml_writeReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    /* Read register */
    aml_readReg(unit, CORAL_MACLMTTH, &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_RANGE(u32dat, (ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE),
                             CORAL_MACLMTTH_PORT_MV_CPU_THR_OFFSET, CORAL_MACLMTTH_PORT_MV_CPU_THR_LENGTH);

    _HAL_CORAL_SEC_SET_RANGE(u32dat, (ptr_cfg->flags & AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL),
                             CORAL_MACLMTTH_SA_FULL_CPU_THR_OFFSET, CORAL_MACLMTTH_SA_FULL_CPU_THR_LENGTH);

    /* Write register */
    aml_writeReg(unit, CORAL_MACLMTTH, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_getMacLimitGlobalCfg
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
hal_coral_sec_getMacLimitGlobalCfg(
    const UI32_T             unit,
    AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;

    osal_memset(ptr_cfg, 0, sizeof(AIR_SEC_MAC_LIMIT_CFG_T));

    /* Read register */
    aml_readReg(unit, CORAL_MACLMTC, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTC_EXC_MNG_OFFSET, CORAL_MACLMTC_EXC_MNG_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT;
    }
    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTC_PORT_MV_DROP_OFFSET, CORAL_MACLMTC_PORT_MV_DROP_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE;
    }
    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTC_SA_FULL_DROP_OFFSET, CORAL_MACLMTC_SA_FULL_DROP_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL;
    }

    /* Read register */
    aml_readReg(unit, CORAL_MACLMTTH, &u32dat, sizeof(u32dat));

    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTTH_PORT_MV_CPU_THR_OFFSET, CORAL_MACLMTTH_PORT_MV_CPU_THR_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE;
    }
    if (!!BITS_OFF_R(u32dat, CORAL_MACLMTTH_SA_FULL_CPU_THR_OFFSET, CORAL_MACLMTTH_SA_FULL_CPU_THR_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL;
    }
    DIAG_PRINT(HAL_DBG_INFO, "config.flags=%08X\n", ptr_cfg->flags);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_setMacLimitPortCfg
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
hal_coral_sec_setMacLimitPortCfg(
    const UI32_T                        unit,
    const UI32_T                        port,
    const AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg)
{
    UI32_T u32dat = 0;
    UI32_T mac_port = 0;

    /* Check parameters */
    HAL_CHECK_MIN_MAX_RANGE(ptr_cfg->sa_lmt_cnt, 0, HAL_CORAL_L2_MAX_ENTRY_NUM);

    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->sa_lmt_cnt=%08X\n", ptr_cfg->sa_lmt_cnt);

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read register */
    aml_readReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    _HAL_CORAL_SEC_SET_BIT(u32dat, !(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN),
                           CORAL_PSC_DIS_LRN_OFFSET);

    _HAL_CORAL_SEC_SET_BIT(u32dat, !!(ptr_cfg->flags & AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT),
                           CORAL_PSC_SA_CNT_EN_OFFSET);

    _HAL_CORAL_SEC_SET_NEW_VALUE(u32dat, ptr_cfg->sa_lmt_cnt, CORAL_PSC_SA_CNT_LMT_OFFSET, CORAL_PSC_SA_CNT_LMT_LENGTH);

    /* Write register */
    aml_writeReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_sec_getMacLimitPortCfg
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
hal_coral_sec_getMacLimitPortCfg(
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
    aml_readReg(unit, CORAL_PSC(mac_port), &u32dat, sizeof(u32dat));

    if (!BITS_OFF_R(u32dat, CORAL_PSC_DIS_LRN_OFFSET, CORAL_PSC_DIS_LRN_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN;
    }
    if (!!BITS_OFF_R(u32dat, CORAL_PSC_SA_CNT_EN_OFFSET, CORAL_PSC_SA_CNT_EN_LENGTH))
    {
        ptr_cfg->flags |= AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT;
    }

    ptr_cfg->sa_lmt_cnt = BITS_OFF_R(u32dat, CORAL_PSC_SA_CNT_LMT_OFFSET, CORAL_PSC_SA_CNT_LMT_LENGTH);

    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->flags=%08X\n", ptr_cfg->flags);
    DIAG_PRINT(HAL_DBG_INFO, "ptr_cfg->sa_lmt_cnt=%08X\n", ptr_cfg->sa_lmt_cnt);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_coral_sec_setPortStormCtrl
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
hal_coral_sec_setPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    const BOOL_T               enable)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    HAL_CORAL_SEC_STORMCTL_BSR_T reg_bsr;
    UI32_T                       mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            reg_bsr.raw.csr_strm_drop = enable;
            reg_bsr.raw.csr_strm_bc_inc = 1;
            reg_bsr.raw.csr_strm_mc_inc = 0;
            reg_bsr.raw.csr_strm_uc_inc = 0;
            /* Write register */
            aml_writeReg(unit, CORAL_ARL_PORT_BSR_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UMC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_EXT1_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            reg_bsr.raw.csr_strm_drop = enable;
            reg_bsr.raw.csr_strm_bc_inc = 0;
            reg_bsr.raw.csr_strm_mc_inc = 1;
            reg_bsr.raw.csr_strm_uc_inc = 0;
            /* Write register */
            aml_writeReg(unit, CORAL_ARL_PORT_BSR_EXT1_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_EXT2_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            reg_bsr.raw.csr_strm_drop = enable;
            reg_bsr.raw.csr_strm_bc_inc = 0;
            reg_bsr.raw.csr_strm_mc_inc = 0;
            reg_bsr.raw.csr_strm_uc_inc = 1;
            /* Write register */
            aml_writeReg(unit, CORAL_ARL_PORT_BSR_EXT2_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: set sec stormctl en unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec stormctl en unit=%d port=%d grp=%d stat_reghex=%x end\n", unit, mac_port,
               type, reg_bsr.byte);
    return rc;
}

/* FUNCTION NAME:   hal_coral_sec_getPortStormCtrl
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
hal_coral_sec_getPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    BOOL_T                    *ptr_enable)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    HAL_CORAL_SEC_STORMCTL_BSR_T reg_bsr;
    UI32_T                       mac_port = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Check parameters */
    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            *ptr_enable = reg_bsr.raw.csr_strm_drop;
            break;

        case AIR_SEC_STORM_TYPE_UMC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_EXT1_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            *ptr_enable = reg_bsr.raw.csr_strm_drop;
            break;

        case AIR_SEC_STORM_TYPE_UUC:
            /* Read register */
            aml_readReg(unit, CORAL_ARL_PORT_BSR_EXT2_ADDR(mac_port), &reg_bsr.byte, sizeof(UI32_T));
            *ptr_enable = reg_bsr.raw.csr_strm_drop;
            break;

        default:
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: get sec stormctl en unit=%d port=%d grp=%d FAIL!!!\n", unit, mac_port,
                       type);
            return AIR_E_BAD_PARAMETER;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get sec stormctl en unit=%d port=%d grp=%d stat_reghex=%x end\n", unit, mac_port,
               type, reg_bsr.byte);
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
 *                          (unit: packets or Kbps)
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
hal_coral_sec_setPortStormCtrlRate(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_STORM_TYPE_T      type,
    const AIR_SEC_STORM_RATE_MODE_T mode,
    const UI32_T                    rate)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    UI32_T                        mac_port = 0;
    UI32_T                        addr_bsr = 0, addr_bsr1 = 0, addr_bstbs = 0;
    HAL_CORAL_SEC_STORMCTL_BSR_T  reg_bsr;
    HAL_CORAL_SEC_STORMCTL_BSR1_T reg_bsr1;
    UI32_T                        reg_bstbs;

    UI32_T                        max_token_unit = 0;
    UI32_T                        base = 0;
    UI32_T                        token_unit = 0, token = 0, count = 0;

    /* Check parameters */
    if (AIR_SEC_STORM_RATE_MODE_KBPS == mode)
    {
        /* check the max BPS rate */
        HAL_CHECK_MIN_MAX_RANGE(rate, 0, HAL_CORAL_SEC_STORMCTL_MAX_RATE_KBPS);
        max_token_unit = HAL_CORAL_SEC_STORM_MAX_UNIT_KBPS;
    }
    else if (AIR_SEC_STORM_RATE_MODE_PPS == mode)
    {
        /* check the max PPS rate */
        HAL_CHECK_MIN_MAX_RANGE(rate, 0, HAL_CORAL_SEC_STORMCTL_MAX_RATE_PPS);
        max_token_unit = HAL_CORAL_SEC_STORM_MAX_UNIT_PPS_AVERAGE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Check the register */
    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            addr_bsr = CORAL_ARL_PORT_BSR_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_ADDR(mac_port);
            addr_bstbs = CORAL_ARL_PORT_BSTBS_ADDR(mac_port);
            break;
        case AIR_SEC_STORM_TYPE_UMC:
            addr_bsr = CORAL_ARL_PORT_BSR_EXT1_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_EXT1_ADDR(mac_port);
            addr_bstbs = CORAL_ARL_PORT_BSTBS_EXT1_ADDR(mac_port);
            break;
        case AIR_SEC_STORM_TYPE_UUC:
            addr_bsr = CORAL_ARL_PORT_BSR_EXT2_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_EXT2_ADDR(mac_port);
            addr_bstbs = CORAL_ARL_PORT_BSTBS_EXT2_ADDR(mac_port);
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }

    DIAG_PRINT(HAL_DBG_INFO, "rate=%d\n", rate);

    /* Calcuate the token unit by input parameter:rate */
    for (token_unit = 0; token_unit <= max_token_unit; token_unit++)
    {
        if (AIR_SEC_STORM_RATE_MODE_KBPS == mode)
        {
            /* Formula: rate = 8000-tick * (token << unit) * 8-bit */
            base = HAL_CORAL_SEC_STORMCTL_KBPS_BASE << token_unit;
            if ((rate / base) > BITS_RANGE(0, HAL_CORAL_SEC_STORMCTL_BPS_TOKEN_LENGTH))
            {
                continue;
            }
        }
        else if (AIR_SEC_STORM_RATE_MODE_PPS == mode)
        {
            /* Formula: rate = count * (pos_token << unit) + (8000-tick - count) * (neg_token << unit) */
            base = HAL_CORAL_SEC_STORMCTL_PPS_BASE << token_unit;
            if ((rate / base) > BITS_RANGE(0, HAL_CORAL_SEC_STORMCTL_PPS_AVG_TOKEN_LENGTH))
            {
                continue;
            }
            count = (rate % base) >> token_unit;
        }
        token = rate / base;
        break;
    }
    DIAG_PRINT(HAL_DBG_INFO, "base=%u, rate=%u, unit=%u, token=%u, count=%u\n", base, rate, token_unit, token, count);

    /* Set storm control mode & base */
    aml_readReg(unit, addr_bsr, &reg_bsr.byte, sizeof(UI32_T));
    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            reg_bsr.raw.csr_strm_bc_inc = 1;
            reg_bsr.raw.csr_strm_mc_inc = 0;
            reg_bsr.raw.csr_strm_uc_inc = 0;
            break;
        case AIR_SEC_STORM_TYPE_UMC:
            reg_bsr.raw.csr_strm_bc_inc = 0;
            reg_bsr.raw.csr_strm_mc_inc = 1;
            reg_bsr.raw.csr_strm_uc_inc = 0;
            break;
        case AIR_SEC_STORM_TYPE_UUC:
            reg_bsr.raw.csr_strm_bc_inc = 0;
            reg_bsr.raw.csr_strm_mc_inc = 0;
            reg_bsr.raw.csr_strm_uc_inc = 1;
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }
    reg_bsr.raw.csr_strm_unit = token_unit;
    reg_bsr.raw.csr_strm_mode = mode;

    aml_writeReg(unit, addr_bsr, &reg_bsr.byte, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec stormctl BSR unit=%d port=%d grp=%d\n", unit, mac_port, type);

    /*set token size & tick count */
    reg_bsr1.byte = 0;
    if (AIR_SEC_STORM_RATE_MODE_KBPS == mode)
    {
        reg_bsr1.bps_raw.token = token;
    }
    else if (AIR_SEC_STORM_RATE_MODE_PPS == mode)
    {
        if (!count)
        {
            reg_bsr1.pps_avg_raw.pos_token = token;
        }
        else
        {
            reg_bsr1.pps_avg_raw.pos_token = token + 1;
        }
        reg_bsr1.pps_avg_raw.neg_token = token;
        reg_bsr1.pps_avg_raw.count = count;
    }
    aml_writeReg(unit, addr_bsr1, &reg_bsr1.byte, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec stormctl stat unit=%u port=%u type=%u BSR=0x%x BSR1=0x%x\n", unit,
               mac_port, type, reg_bsr.byte, reg_bsr1.byte);

    /* calculate the storm control bucket size */
    if (AIR_SEC_STORM_RATE_MODE_KBPS == mode)
    {
        rc = _hal_coral_sec_calTokenSizeBps(unit, (token << token_unit), &reg_bstbs);
    }
    else if (AIR_SEC_STORM_RATE_MODE_PPS == mode)
    {
        /* Formula : bucket_size = 80 * pos_token << token_unit, Accumulate 125us * 80 = 10ms */
        reg_bstbs = (reg_bsr1.pps_avg_raw.pos_token << token_unit) * 80;
    }

    if (reg_bstbs > HAL_CORAL_SEC_STORMCTL_MAX_TOKEN_SIZE)
    {
        reg_bstbs = HAL_CORAL_SEC_STORMCTL_MAX_TOKEN_SIZE;
    }

    aml_writeReg(unit, addr_bstbs, &reg_bstbs, sizeof(UI32_T));
    DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: set sec stormctl BSTBS unit=%d port=%d type=%d\n", unit, mac_port, type);
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
hal_coral_sec_getPortStormCtrlRate(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    AIR_SEC_STORM_RATE_MODE_T *ptr_mode,
    UI32_T                    *ptr_rate)
{
    AIR_ERROR_NO_T                rc = AIR_E_OK;
    UI32_T                        mac_port = 0;
    UI32_T                        addr_bsr = 0, addr_bsr1 = 0;
    HAL_CORAL_SEC_STORMCTL_BSR_T  reg_bsr;
    HAL_CORAL_SEC_STORMCTL_BSR1_T reg_bsr1;

    UI32_T                        token_unit = 0, token = 0, pos_token = 0, neg_token = 0, count = 0;

    /* Translate Port bitmap */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Check the register */
    switch (type)
    {
        case AIR_SEC_STORM_TYPE_BC:
            addr_bsr = CORAL_ARL_PORT_BSR_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_ADDR(mac_port);
            break;
        case AIR_SEC_STORM_TYPE_UMC:
            addr_bsr = CORAL_ARL_PORT_BSR_EXT1_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_EXT1_ADDR(mac_port);
            break;
        case AIR_SEC_STORM_TYPE_UUC:
            addr_bsr = CORAL_ARL_PORT_BSR_EXT2_ADDR(mac_port);
            addr_bsr1 = CORAL_ARL_PORT_BSR1_EXT2_ADDR(mac_port);
            break;
        default:
            return AIR_E_BAD_PARAMETER;
    }
    /* Read storm control mode & token unit */
    aml_readReg(unit, addr_bsr, &reg_bsr.byte, sizeof(UI32_T));
    token_unit = reg_bsr.raw.csr_strm_unit;
    *ptr_mode = reg_bsr.raw.csr_strm_mode;

    /* Read storm control token counter */
    aml_readReg(unit, addr_bsr1, &reg_bsr1.byte, sizeof(UI32_T));
    if (AIR_SEC_STORM_RATE_MODE_KBPS == *ptr_mode)
    {
        token = reg_bsr1.bps_raw.token;
        *ptr_rate = token * (HAL_CORAL_SEC_STORMCTL_KBPS_BASE << token_unit);
        if ((*ptr_rate) > HAL_CORAL_SEC_STORMCTL_MAX_RATE_KBPS)
        {
            *ptr_rate = HAL_CORAL_SEC_STORMCTL_MAX_RATE_KBPS;
        }
    }
    else if (AIR_SEC_STORM_RATE_MODE_PPS == *ptr_mode)
    {
        pos_token = reg_bsr1.pps_avg_raw.pos_token;
        neg_token = reg_bsr1.pps_avg_raw.neg_token;
        count = reg_bsr1.pps_avg_raw.count;
        *ptr_rate = (count * (pos_token << token_unit)) + ((8000 - count) * (neg_token << token_unit));
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get sec stormctl stat unit=%d port=%d grp=%d BSR=0x%x BSR1=0x%x\n", unit, mac_port,
               type, reg_bsr.byte, reg_bsr1.byte);

    return rc;
}

/* FUNCTION NAME:   hal_coral_sec_setStormCtrlMgmtMode
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
hal_coral_sec_setStormCtrlMgmtMode(
    const UI32_T                    unit,
    const AIR_SEC_STORM_CTRL_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         AGC_val;

    /* Read register */
    aml_readReg(unit, CORAL_AGC, &AGC_val, sizeof(UI32_T));
    _HAL_CORAL_SEC_SET_BIT(AGC_val, (AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE == mode), CORAL_CSR_BCSTRM_EXC_MG);
    /* Write register */
    aml_writeReg(unit, CORAL_AGC, &AGC_val, sizeof(UI32_T));

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: set sec mngmode unit=%d mode=%d end\n", unit, mode);
    return rc;
}

/* FUNCTION NAME:   hal_coral_sec_getStormCtrlMgmtMode
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
hal_coral_sec_getStormCtrlMgmtMode(
    const UI32_T               unit,
    AIR_SEC_STORM_CTRL_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         AGC_val;

    /* Read register */
    aml_readReg(unit, CORAL_AGC, &AGC_val, sizeof(UI32_T));

    if (AIR_E_OK == rc)
    {
        *ptr_mode = BITS_OFF_R(AGC_val, CORAL_CSR_BCSTRM_EXC_MG, 1);
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: get sec mngmode unit=%d mode=%d end\n", unit, *ptr_mode);
    return rc;
}
