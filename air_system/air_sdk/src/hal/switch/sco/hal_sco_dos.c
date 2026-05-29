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

/* FILE NAME:  hal_sco_dos.c
 * PURPOSE:
 *    It provides HAL DoS attack prevention driver API functions.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_dos.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_DOS_ATTACK_LENGTH_CFG_DEFAULT_VALUE  (0x0000FFFF)
#define HAL_SCO_DOS_ATTACK_ACTION_CFG_DEFAULT_VALUE  (0x80000004)
#define HAL_SCO_DOS_ATTACK_PREVENT_GLB_DEFAULT_VALUE (0)
#define HAL_SCO_DOS_ATTACK_TCP_FLAG_DEFAULT_VALUE    (0)
#define HAL_SCO_DOS_ATTACK_RATE_CFG_DEFAULT_VALUE    (0)
#define HAL_SCO_DOS_ATTACK_WR_DATA_DEFAULT_VALUE     (0)

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(__ptr_port_cfg__, __attack__)                                     \
    do                                                                                                      \
    {                                                                                                       \
        if ((__ptr_port_cfg__)->__attack__.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)                  \
        {                                                                                                   \
            HAL_CHECK_MIN_MAX_RANGE(((__ptr_port_cfg__)->__attack__.rate_cfg_idx), HAL_SCO_DOS_MIN_RATE_ID, \
                                    HAL_SCO_DOS_MAX_RATE_ID);                                               \
        }                                                                                                   \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_DOS, "hal_sco_dos.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_sco_dos_tcpCfgToRegVal
 * PURPOSE:
 *      Convert TCP flag setting to val
 * INPUT:
 *      tcp_flag        -- TCP flag setting
 * OUTPUT:
 *      ptr_reg_val     -- val to be set to register
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
_hal_sco_dos_tcpCfgToRegVal(
    const UI32_T tcp_flag,
    UI32_T      *ptr_reg_val)
{
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_NS)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_NS;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_CWR)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_CWR;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_ECE)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_ECE;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_URG)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_URG;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_ACK)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_ACK;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_PSH)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_PSH;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_RST)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_RST;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_SYN)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_SYN;
    }
    if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_FIN)
    {
        *ptr_reg_val |= ATTACK_TCP_FLAG_SEL_FIN;
    }
}

/* FUNCTION NAME: _hal_sco_dos_regValToTcpCfg
 * PURPOSE:
 *      Convert reg val to TCP flag setting
 * INPUT:
 *      reg_val         -- val read from register
 * OUTPUT:
 *      ptr_tcp_cfg     -- TCP flag setting
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
_hal_sco_dos_regValToTcpCfg(
    const UI32_T reg_val,
    UI32_T      *ptr_tcp_cfg)
{
    if (reg_val & ATTACK_TCP_FLAG_SEL_NS)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_NS;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_CWR)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_CWR;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_ECE)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_ECE;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_URG)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_URG;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_ACK)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_ACK;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_PSH)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_PSH;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_RST)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_RST;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_SYN)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_SYN;
    }
    if (reg_val & ATTACK_TCP_FLAG_SEL_FIN)
    {
        *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_FIN;
    }
}

/* FUNCTION NAME: _hal_sco_dos_checkDosDone
 * PURPOSE:
 *      Check done bit of ATTACK_ACTION_CFG and ATTACK_RATE_CFG
 * INPUT:
 *      unit            -- Device ID
 *      addr_offset     -- the address of register
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_TIMEOUT
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_sco_dos_checkDosDone(
    const UI32_T unit,
    const UI32_T addr_offset)
{
    UI32_T i;
    UI32_T reg_atc;

    /* Check Done bit is 1 */
    for (i = 0; i < HAL_SCO_DOS_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, addr_offset, &reg_atc, sizeof(reg_atc));
        if (TRUE == BITS_OFF_R(reg_atc, ATTACK_CFG_DONE_OFFT, ATTACK_CFG_DONE_LENG))
        {
            break;
        }
        osal_delayUs(HAL_SCO_DOS_DELAY_US);
    }
    if (i >= HAL_SCO_DOS_MAX_BUSY_TIME)
    {
        return AIR_E_TIMEOUT;
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_sco_dos_getRateLimitUsage
 * PURPOSE:
 *      Get the dos ratelimit usage
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_cnt             --  The used count of ratelimit
 * RETURN:
 *      AIR_E_OK            --  Operation is successfull.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static void
_hal_sco_dos_getRateLimitUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   entry_idx, usage_count = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_cfg;

    for (entry_idx = 0; entry_idx <= HAL_SCO_DOS_MAX_RATE_ID; entry_idx++)
    {
        rc = hal_sco_dos_getRateLimitCfg(unit, entry_idx, &rate_cfg);
        if ((AIR_E_OK == rc) && ((0 != rate_cfg.pkt_thld) || (0 != rate_cfg.time_span) || (0 != rate_cfg.block_time) ||
                                 (0 != rate_cfg.tick_sel)))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* FUNCTION NAME: hal_sco_dos_init
 * PURPOSE:
 *      Initialization of DoS.
 * INPUT:
 *      unit    -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_init(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_deinit
 * PURPOSE:
 *      Deinitialization of DoS.
 * INPUT:
 *      unit    -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_deinit(
    const UI32_T unit)
{
    UI32_T u32dat;

    hal_sco_dos_clearActionCfg(unit);
    hal_sco_dos_clearAllRateLimitCfg(unit);
    hal_sco_dos_clearDropCnt(unit);

    u32dat = HAL_SCO_DOS_ATTACK_PREVENT_GLB_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_DOS_ATTACK_LENGTH_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_DOS_ATTACK_TCP_FLAG_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_DOS_ATTACK_ACTION_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_ACTION_CFG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_DOS_ATTACK_RATE_CFG_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_RATE_CFG, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_DOS_ATTACK_WR_DATA_DEFAULT_VALUE;
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA0, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA1, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA0, &u32dat, sizeof(u32dat));
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA1, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_setGlobalCfg
 * PURPOSE:
 *      Set DoS global state.
 * INPUT:
 *      unit     -- Device unit number
 *      enable   -- DoS global state
 *                  FALSE: Disable
 *                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_setGlobalCfg(
    const UI32_T unit,
    const BOOL_T enable)
{
    UI32_T u32dat = 0;

    /* Set to register */
    aml_readReg(unit, ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_PREVENT_GLB=0x%x\n", unit, u32dat, ATTACK_PREVENT_GLB);
    if (TRUE == enable)
    {
        u32dat |= BIT(ATTACK_PREVENT_GLB_EN_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(ATTACK_PREVENT_GLB_EN_OFFT));
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_PREVENT_GLB\n", unit, u32dat);
    aml_writeReg(unit, ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getGlobalCfg
 * PURPOSE:
 *      Get DoS global state.
 * INPUT:
 *      unit         -- Device unit number
 * OUTPUT:
 *      ptr_enable   -- DoS global state
 *                      FALSE: Disable
 *                      TRUE: Enable
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getGlobalCfg(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat = 0;

    /* Read data from register */
    aml_readReg(unit, ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_PREVENT_GLB=0x%x\n", unit, u32dat, ATTACK_PREVENT_GLB);
    if (BITS_OFF_R(u32dat, ATTACK_PREVENT_GLB_EN_OFFT, ATTACK_PREVENT_GLB_EN_LENG))
    {
        *ptr_enable = TRUE;
    }
    else
    {
        *ptr_enable = FALSE;
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_PREVENT_GLB, enable=%d\n", unit, u32dat, *ptr_enable);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_setIcmpLengthLimit
 * PURPOSE:
 *      Set DoS length attack threshold.
 * INPUT:
 *      unit    -- Device unit number
 *      length  -- DoS length attack threshold
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_setIcmpLengthLimit(
    const UI32_T unit,
    const UI32_T length)
{
    UI32_T u32dat = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(length, HAL_SCO_DOS_MIN_ATTACK_LENGTH, HAL_SCO_DOS_MAX_ATTACK_LENGTH);

    /* Set to register */
    aml_readReg(unit, ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_LENGTH_CFG=0x%x\n", unit, u32dat, ATTACK_LENGTH_CFG);
    u32dat &= ~(BITS_RANGE(ATTACK_LENGTH_THLD_OFFT, ATTACK_LENGTH_THLD_LENG));
    u32dat |= (length << ATTACK_LENGTH_THLD_OFFT);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_LENGTH_CFG\n", unit, u32dat);
    aml_writeReg(unit, ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getIcmpLengthLimit
 * PURPOSE:
 *      Get DoS length attack threshold.
 * INPUT:
 *      unit        -- Device unit number
 * OUTPUT:
 *      ptr_length  -- DoS length attack threshold
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getIcmpLengthLimit(
    const UI32_T unit,
    UI32_T      *ptr_length)
{
    UI32_T u32dat = 0;

    /* Read data from register */
    aml_readReg(unit, ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_LENGTH_CFG=0x%x\n", unit, u32dat, ATTACK_LENGTH_CFG);
    (*ptr_length) = BITS_OFF_R(u32dat, ATTACK_LENGTH_THLD_OFFT, ATTACK_LENGTH_THLD_LENG);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_LENGTH_CFG, length=%u\n", unit, u32dat, *ptr_length);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_setTcpFlagCfg
 * PURPOSE:
 *      Set DoS TCP flag which regard as attack.
 * INPUT:
 *      unit            -- Device unit number
 *      idx             -- TCP setting id
 *      ptr_tcp_cfg     -- TCP flags setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_setTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    UI32_T u32dat = 0;
    UI32_T reg_key = 0;
    UI32_T reg_msk = 0;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(idx, AIR_DOS_TCP_ID_LAST);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcp_cfg->key_flags, 0, BITS_RANGE(0, ATTACK_TCP_SETTING_LENG));
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcp_cfg->mask_flags, 0, BITS_RANGE(0, ATTACK_TCP_SETTING_LENG));

    /* Set TCP flag key */
    _hal_sco_dos_tcpCfgToRegVal(ptr_tcp_cfg->key_flags, &reg_key);
    aml_readReg(unit, ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_TCP_FLAG_CFG=0x%x\n", unit, u32dat, ATTACK_TCP_FLAG_CFG);
    u32dat &= ~(BITS_RANGE(ATTACK_TCP_SETTING_OFFT(idx), ATTACK_TCP_SETTING_LENG));
    u32dat |= (reg_key << ATTACK_TCP_SETTING_OFFT(idx));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_LENGTH_CFG\n", unit, u32dat);
    aml_writeReg(unit, ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));

    /* Set TCP flag mask */
    _hal_sco_dos_tcpCfgToRegVal(ptr_tcp_cfg->mask_flags, &reg_msk);
    aml_readReg(unit, ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_TCP_FLAG_MSK=0x%x\n", unit, u32dat, ATTACK_TCP_FLAG_MSK);
    u32dat &= ~(BITS_RANGE(ATTACK_TCP_SETTING_OFFT(idx), ATTACK_TCP_SETTING_LENG));
    u32dat |= (reg_msk << ATTACK_TCP_SETTING_OFFT(idx));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_TCP_FLAG_MSK\n", unit, u32dat);
    aml_writeReg(unit, ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getTcpFlagCfg
 * PURPOSE:
 *      Get DoS TCP setting.
 * INPUT:
 *      unit            -- Device unit number
 *      idx             -- TCP setting id
 * OUTPUT:
 *      ptr_tcp_cfg     -- TCP flag setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    UI32_T u32dat = 0;
    UI32_T reg_key = 0;
    UI32_T reg_msk = 0;

    /* Check parameter */
    HAL_CHECK_ENUM_RANGE(idx, AIR_DOS_TCP_ID_LAST);
    osal_memset(ptr_tcp_cfg, 0, sizeof(AIR_DOS_TCP_CFG_T));

    /* Read data from register */
    /* TCP flag key*/
    aml_readReg(unit, ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_TCP_FLAG_CFG=0x%x\n", unit, u32dat, ATTACK_TCP_FLAG_CFG);
    reg_key = (u32dat >> ATTACK_TCP_SETTING_OFFT(idx)) & (BITS(0, ATTACK_TCP_SETTING_LENG - 1));
    _hal_sco_dos_regValToTcpCfg(reg_key, &ptr_tcp_cfg->key_flags);

    /* TCP flag mask */
    aml_readReg(unit, ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_TCP_FLAG_MSK=0x%x\n", unit, u32dat, ATTACK_TCP_FLAG_MSK);
    reg_msk = (u32dat >> ATTACK_TCP_SETTING_OFFT(idx)) & (BITS(0, ATTACK_TCP_SETTING_LENG - 1));
    _hal_sco_dos_regValToTcpCfg(reg_msk, &ptr_tcp_cfg->mask_flags);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_setPortActionCfg
 * PURPOSE:
 *      Set DoS attack action on port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 *      ptr_action_cfg  -- Port action setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_setPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    UI32_T cfg_data = 0;
    UI32_T wr_data0 = 0, wr_data1 = 0;
    UI32_T mac_port = 0;

    /* Check parameter */
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, land);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, blat);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, length);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, ping);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_0]);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_1]);
    HAL_SCO_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_2]);

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* read action */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (mac_port << ATTACK_ACTION_CFG_PID_OFFT);
    aml_writeReg(unit, ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_ACTION_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_ACTION_CFG=0x%x timeout\n", unit, ATTACK_ACTION_CFG);
        return AIR_E_TIMEOUT;
    }

    /* action SRAM write data */
    /* land */
    if (ptr_action_cfg->land.action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data0 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_LAND_OFFT));
    }
    if (ptr_action_cfg->land.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data0 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_LAND_OFFT));
    }
    wr_data0 |= (ptr_action_cfg->land.rate_cfg_idx << ATTACK_LAND_OFFT);
    /* blat */
    if (ptr_action_cfg->blat.action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data0 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_BLAT_OFFT));
    }
    if (ptr_action_cfg->blat.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data0 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_BLAT_OFFT));
    }
    wr_data0 |= (ptr_action_cfg->blat.rate_cfg_idx << ATTACK_BLAT_OFFT);
    /* length */
    if (ptr_action_cfg->length.action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data0 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_LENGTH_OFFT));
    }
    if (ptr_action_cfg->length.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data0 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_LENGTH_OFFT));
    }
    wr_data0 |= (ptr_action_cfg->length.rate_cfg_idx << ATTACK_LENGTH_OFFT);
    /* tcp0 span across two register tables */
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data1 |= (1 << ATTACK_TCP0_DROP_OFFT);
    }
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data1 |= (1 << ATTACK_TCP0_RATE_EN_OFFT);
    }

    wr_data1 |= ((BITS_OFF_R(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].rate_cfg_idx, ATTACK_TCP0_RATE_ID_LO_LENG,
                             ATTACK_TCP0_RATE_ID_HI_LENG))
                 << ATTACK_TCP0_RATE_ID_HI_OFFT);
    wr_data0 |= ((BITS_OFF_R(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].rate_cfg_idx, 0, ATTACK_TCP0_RATE_ID_LO_LENG))
                 << ATTACK_TCP0_RATE_ID_LO_OFFT);
    /* tcp1 */
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data1 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_TCP1_OFFT));
    }
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data1 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_TCP1_OFFT));
    }
    wr_data1 |= (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].rate_cfg_idx << ATTACK_TCP1_OFFT);
    /* tcp2 */
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data1 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_TCP2_OFFT));
    }
    if (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data1 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_TCP2_OFFT));
    }
    wr_data1 |= (ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].rate_cfg_idx << ATTACK_TCP2_OFFT);
    /* ping */
    if (ptr_action_cfg->ping.action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        wr_data1 |= (1 << ATTACK_DROP_EN_OFFT(ATTACK_PING_OFFT));
    }
    if (ptr_action_cfg->ping.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        wr_data1 |= (1 << ATTACK_RATE_EN_OFFT(ATTACK_PING_OFFT));
    }
    wr_data1 |= (ptr_action_cfg->ping.rate_cfg_idx << ATTACK_PING_OFFT);

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_WR_OFFT);
    cfg_data |= (mac_port << ATTACK_ACTION_CFG_PID_OFFT);
    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               ATTACK_ACTION_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               ATTACK_ACTION_SRAM_WR_DATA1);
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data, ATTACK_ACTION_CFG);
    aml_writeReg(unit, ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_ACTION_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_ACTION_CFG=0x%x timeout\n", unit, ATTACK_ACTION_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getPortActionCfg
 * PURPOSE:
 *      Get DoS attack action on port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 * OUTPUT:
 *      ptr_action_cfg  -- Port action setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    UI32_T cfg_data = 0;
    UI32_T re_data0 = 0, re_data1 = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (mac_port << ATTACK_ACTION_CFG_PID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data, ATTACK_ACTION_CFG);
    aml_writeReg(unit, ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_ACTION_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_ACTION_CFG=0x%x timeout\n", unit, ATTACK_ACTION_CFG);
        return AIR_E_TIMEOUT;
    }

    /* Read data from register */
    aml_readReg(unit, ATTACK_ACTION_SRAM_RE_DATA0, &re_data0, sizeof(re_data0));
    aml_readReg(unit, ATTACK_ACTION_SRAM_RE_DATA1, &re_data1, sizeof(re_data1));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ACTION_SRAM_RE_DATA0=0x%x\n", unit, re_data0,
               ATTACK_ACTION_SRAM_RE_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ACTION_SRAM_RE_DATA1=0x%x\n", unit, re_data1,
               ATTACK_ACTION_SRAM_RE_DATA1);

    /* land */
    if (BITS_OFF_R(re_data0, ATTACK_DROP_EN_OFFT(ATTACK_LAND_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->land.action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data0, ATTACK_RATE_EN_OFFT(ATTACK_LAND_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->land.action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->land.rate_cfg_idx = BITS_OFF_R(re_data0, ATTACK_LAND_OFFT, ATTACK_ACTION_RATE_ID_LENG);
    /* blat */
    if (BITS_OFF_R(re_data0, ATTACK_DROP_EN_OFFT(ATTACK_BLAT_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->blat.action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data0, ATTACK_RATE_EN_OFFT(ATTACK_BLAT_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->blat.action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->blat.rate_cfg_idx = BITS_OFF_R(re_data0, ATTACK_BLAT_OFFT, ATTACK_ACTION_RATE_ID_LENG);
    /* length */
    if (BITS_OFF_R(re_data0, ATTACK_DROP_EN_OFFT(ATTACK_LENGTH_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->length.action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data0, ATTACK_RATE_EN_OFFT(ATTACK_LENGTH_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->length.action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->length.rate_cfg_idx = BITS_OFF_R(re_data0, ATTACK_LENGTH_OFFT, ATTACK_ACTION_RATE_ID_LENG);
    /* tcp0 - span across two register tables */
    if (BITS_OFF_R(re_data1, ATTACK_TCP0_DROP_OFFT, ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data1, ATTACK_TCP0_RATE_EN_OFFT, ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0].rate_cfg_idx =
        ((BITS_OFF_R(re_data1, ATTACK_TCP0_RATE_ID_HI_OFFT, ATTACK_TCP0_RATE_ID_HI_LENG)
          << ATTACK_TCP0_RATE_ID_LO_LENG) +
         BITS_OFF_R(re_data0, ATTACK_TCP0_RATE_ID_LO_OFFT, ATTACK_TCP0_RATE_ID_LO_LENG));
    /* tcp1 */
    if (BITS_OFF_R(re_data1, ATTACK_DROP_EN_OFFT(ATTACK_TCP1_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data1, ATTACK_RATE_EN_OFFT(ATTACK_TCP1_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1].rate_cfg_idx =
        BITS_OFF_R(re_data1, ATTACK_TCP1_OFFT, ATTACK_ACTION_RATE_ID_LENG);
    /* tcp2 */
    if (BITS_OFF_R(re_data1, ATTACK_DROP_EN_OFFT(ATTACK_TCP2_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data1, ATTACK_RATE_EN_OFFT(ATTACK_TCP2_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2].rate_cfg_idx =
        BITS_OFF_R(re_data1, ATTACK_TCP2_OFFT, ATTACK_ACTION_RATE_ID_LENG);
    /* ping */
    if (BITS_OFF_R(re_data1, ATTACK_DROP_EN_OFFT(ATTACK_PING_OFFT), ATTACK_ACTION_DROP_LENG))
    {
        ptr_action_cfg->ping.action_flags = AIR_DOS_ACTION_FLAGS_DROP;
    }
    if (BITS_OFF_R(re_data1, ATTACK_RATE_EN_OFFT(ATTACK_PING_OFFT), ATTACK_ACTION_RATE_EN_LENG))
    {
        ptr_action_cfg->ping.action_flags = AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }
    ptr_action_cfg->ping.rate_cfg_idx = BITS_OFF_R(re_data1, ATTACK_PING_OFFT, ATTACK_ACTION_RATE_ID_LENG);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_clearPortActionCfg
 * PURPOSE:
 *      Clear DoS attack action on specific port.
 * INPUT:
 *      unit            -- Device unit number
 *      port            -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_clearPortActionCfg(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T cfg_data = 0;
    UI32_T wr_data0 = 0, wr_data1 = 0;
    UI32_T mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_WR_OFFT);
    cfg_data |= (mac_port << ATTACK_ACTION_CFG_PID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               ATTACK_ACTION_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               ATTACK_ACTION_SRAM_WR_DATA1);
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, ATTACK_ACTION_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data, ATTACK_ACTION_CFG);
    aml_writeReg(unit, ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_ACTION_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_ACTION_CFG=0x%x timeout\n", unit, ATTACK_ACTION_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_clearActionCfg
 * PURPOSE:
 *      Clear DoS attack action on all port.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_clearActionCfg(
    const UI32_T unit)
{
    UI32_T cfg_data = 0;

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_CLEAR_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data, ATTACK_ACTION_CFG);
    aml_writeReg(unit, ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_ACTION_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_ACTION_CFG=0x%x timeout\n", unit, ATTACK_ACTION_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_setRateLimitCfg
 * PURPOSE:
 *      Set DoS rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 *      ptr_rate_cfg    -- Rate limit setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    UI32_T cfg_data = 0;
    UI32_T wr_data0 = 0, wr_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_SCO_DOS_MIN_RATE_ID, HAL_SCO_DOS_MAX_RATE_ID);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->pkt_thld, HAL_SCO_DOS_MIN_RATE_WR_CFG, HAL_SCO_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->time_span, HAL_SCO_DOS_MIN_RATE_WR_CFG, HAL_SCO_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->block_time, HAL_SCO_DOS_MIN_RATE_WR_CFG, HAL_SCO_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_ENUM_RANGE(ptr_rate_cfg->tick_sel, AIR_DOS_RATE_TICKSEL_LAST);

    /* rate SRAM write data */
    /* wdata0 - pkt num thld + time span */
    wr_data0 |= (ptr_rate_cfg->pkt_thld << ATTACK_RATE_PKT_NUM_THLD_OFFT);
    wr_data0 |= (ptr_rate_cfg->time_span << ATTACK_RATE_TIME_SPAN_OFFT);
    /* wdata1 - tick sel + block time */
    wr_data1 |= (ptr_rate_cfg->block_time << ATTACK_RATE_BLOCK_TIME_OFFT);
    if (AIR_DOS_RATE_TICKSEL_8MS == ptr_rate_cfg->tick_sel)
    {
        wr_data1 |= (1 << ATTACK_RATE_TICK_SEL_OFFT);
    }

    /* rate SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_WR_OFFT);
    cfg_data |= (rate_cfg_idx << ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               ATTACK_RATE_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               ATTACK_RATE_SRAM_WR_DATA1);
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, ATTACK_RATE_CFG);
    aml_writeReg(unit, ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_RATE_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_RATE_CFG=0x%x timeout\n", unit, ATTACK_RATE_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getRateLimitCfg
 * PURPOSE:
 *      Get DoS rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 * OUTPUT:
 *      ptr_rate_cfg    -- Rate limit setting
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    UI32_T cfg_data = 0;
    UI32_T re_data0 = 0, re_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_SCO_DOS_MIN_RATE_ID, HAL_SCO_DOS_MAX_RATE_ID);

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (rate_cfg_idx << ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, ATTACK_RATE_CFG);
    aml_writeReg(unit, ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_RATE_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_RATE_CFG=0x%x timeout\n", unit, ATTACK_RATE_CFG);
        return AIR_E_TIMEOUT;
    }

    /* Read data from register */
    aml_readReg(unit, ATTACK_RATE_SRAM_RE_DATA0, &re_data0, sizeof(re_data0));
    aml_readReg(unit, ATTACK_RATE_SRAM_RE_DATA1, &re_data1, sizeof(re_data1));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from RATE_SRAM_RE_DATA0=0x%x\n", unit, re_data0,
               ATTACK_RATE_SRAM_RE_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from RATE_SRAM_RE_DATA1=0x%x\n", unit, re_data1,
               ATTACK_RATE_SRAM_RE_DATA1);

    ptr_rate_cfg->pkt_thld = BITS_OFF_R(re_data0, ATTACK_RATE_PKT_NUM_THLD_OFFT, ATTACK_RATE_WR_CFG_LENG);
    ptr_rate_cfg->time_span = BITS_OFF_R(re_data0, ATTACK_RATE_TIME_SPAN_OFFT, ATTACK_RATE_WR_CFG_LENG);

    ptr_rate_cfg->block_time = BITS_OFF_R(re_data1, ATTACK_RATE_BLOCK_TIME_OFFT, ATTACK_RATE_WR_CFG_LENG);
    if (BITS_OFF_R(re_data1, ATTACK_RATE_TICK_SEL_OFFT, ATTACK_RATE_WR_TICK_SEL_LENG))
    {
        ptr_rate_cfg->tick_sel = AIR_DOS_RATE_TICKSEL_8MS;
    }
    else
    {
        ptr_rate_cfg->tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_clearRateLimitCfg
 * PURPOSE:
 *      Clear DoS attack rate limit rule on specific id.
 * INPUT:
 *      unit            -- Device unit number
 *      rate_cfg_idx    -- Index of rate limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_clearRateLimitCfg(
    const UI32_T unit,
    const UI32_T rate_cfg_idx)
{
    UI32_T cfg_data = 0;
    UI32_T wr_data0 = 0, wr_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_SCO_DOS_MIN_RATE_ID, HAL_SCO_DOS_MAX_RATE_ID);

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_WR_OFFT);
    cfg_data |= (rate_cfg_idx << ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               ATTACK_RATE_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               ATTACK_RATE_SRAM_WR_DATA1);
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, ATTACK_RATE_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, ATTACK_RATE_CFG);
    aml_writeReg(unit, ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_RATE_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_RATE_CFG=0x%x timeout\n", unit, ATTACK_RATE_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_clearAllRateLimitCfg
 * PURPOSE:
 *      Clear all DoS attack rate limit rules.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_clearAllRateLimitCfg(
    const UI32_T unit)
{
    UI32_T cfg_data = 0;

    /* action SRAM config */
    cfg_data |= (1 << ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << ATTACK_CFG_CLEAR_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, ATTACK_RATE_CFG);
    aml_writeReg(unit, ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    if (AIR_E_TIMEOUT == _hal_sco_dos_checkDosDone(unit, ATTACK_RATE_CFG))
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write ATTACK_RATE_CFG=0x%x timeout\n", unit, ATTACK_RATE_CFG);
        return AIR_E_TIMEOUT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getDropCnt
 * PURPOSE:
 *      Get packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      ptr_drop_cnt    -- packet drop counter
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_getDropCnt(
    const UI32_T unit,
    UI32_T      *ptr_drop_cnt)
{
    UI32_T u32dat = 0;

    /* Read data from register */
    aml_readReg(unit, ATTACK_DROP_CNT, &u32dat, sizeof(u32dat));
    (*ptr_drop_cnt) = u32dat;
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_DROP_CNT=0x%x, length=%u\n", unit, u32dat, ATTACK_DROP_CNT,
               *ptr_drop_cnt);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_clearDropCnt
 * PURPOSE:
 *      Clear packet counter of drop by DoS attack prevention.
 * INPUT:
 *      unit            -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success
 *      AIR_E_BAD_PARAMETER -- Bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_dos_clearDropCnt(
    const UI32_T unit)
{
    UI32_T u32dat;

    /* Set to register */
    aml_readReg(unit, ATTACK_DROP_CNT_CLR, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ATTACK_DROP_CNT_CLR=0x%x\n", unit, u32dat, ATTACK_DROP_CNT_CLR);
    /* Write clear bit */
    u32dat |= ATTACK_DROP_CNT_CLR_A0;
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ATTACK_DROP_CNT_CLR\n", unit, u32dat);
    aml_writeReg(unit, ATTACK_DROP_CNT_CLR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_dos_getCapacity
 * PURPOSE:
 *      Get the dos resource capacity
 *
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_dos_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_DOS_RATELIMIT == type)
    {
        *ptr_size = HAL_SCO_DOS_MAX_RATE_ID + 1;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_dos_getUsage
 * PURPOSE:
 *      Get the dos resource usage
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
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_sco_dos_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    *ptr_cnt = 0;
    if (AIR_SWC_RSRC_DOS_RATELIMIT == type)
    {
        _hal_sco_dos_getRateLimitUsage(unit, ptr_cnt);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}
