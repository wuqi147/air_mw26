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

/* FILE NAME:  hal_coral_dos.c
 * PURPOSE:
 *    It provides HAL DoS attack prevention driver API functions.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_dos.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/switch/coral/hal_coral_reg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(__ptr_port_cfg__, __attack__)                                     \
    do                                                                                                        \
    {                                                                                                         \
        if ((__ptr_port_cfg__)->__attack__.action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)                    \
        {                                                                                                     \
            HAL_CHECK_MIN_MAX_RANGE(((__ptr_port_cfg__)->__attack__.rate_cfg_idx), HAL_CORAL_DOS_MIN_RATE_ID, \
                                    HAL_CORAL_DOS_MAX_RATE_ID);                                               \
        }                                                                                                     \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_DOS, "hal_coral_dos.c");

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME: _hal_coral_dos_tcpCfgToRegVal
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
_hal_coral_dos_tcpCfgToRegVal(
    const UI32_T tcp_flag,
    UI32_T      *ptr_reg_val)
{
    if (NULL != ptr_reg_val)
    {
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_NS)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_NS;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_CWR)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_CWR;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_ECE)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_ECE;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_URG)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_URG;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_ACK)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_ACK;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_PSH)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_PSH;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_RST)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_RST;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_SYN)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_SYN;
        }
        if (tcp_flag & AIR_DOS_TCP_CFG_FLAGS_FIN)
        {
            *ptr_reg_val |= CORAL_ATTACK_TCP_FLAG_SEL_FIN;
        }
    }
}

/* FUNCTION NAME: _hal_coral_dos_regValToTcpCfg
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
_hal_coral_dos_regValToTcpCfg(
    const UI32_T reg_val,
    UI32_T      *ptr_tcp_cfg)
{
    if (NULL != ptr_tcp_cfg)
    {
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_NS)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_NS;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_CWR)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_CWR;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_ECE)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_ECE;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_URG)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_URG;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_ACK)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_ACK;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_PSH)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_PSH;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_RST)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_RST;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_SYN)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_SYN;
        }
        if (reg_val & CORAL_ATTACK_TCP_FLAG_SEL_FIN)
        {
            *ptr_tcp_cfg |= AIR_DOS_TCP_CFG_FLAGS_FIN;
        }
    }
}

/* FUNCTION NAME: _hal_coral_dos_checkDosDone
 * PURPOSE:
 *      Check done bit of CORAL_ATTACK_ACTION_CFG and CORAL_ATTACK_RATE_CFG
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
_hal_coral_dos_checkDosDone(
    const UI32_T unit,
    const UI32_T addr_offset)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         i;
    UI32_T         reg_atc;

    /* Check Done bit is 1 */
    for (i = 0; i < HAL_CORAL_DOS_MAX_BUSY_TIME; i++)
    {
        aml_readReg(unit, addr_offset, &reg_atc, sizeof(reg_atc));
        if (0 != BITS_OFF_R(reg_atc, CORAL_ATTACK_CFG_DONE_OFFT, CORAL_ATTACK_CFG_DONE_LENG))
        {
            break;
        }
        osal_delayUs(HAL_CORAL_DOS_DELAY_US);
    }
    if (i >= HAL_CORAL_DOS_MAX_BUSY_TIME)
    {
        rc = AIR_E_TIMEOUT;
    }
    return rc;
}

/* FUNCTION NAME: _hal_coral_dos_setActionCfg
 * PURPOSE:
 *      Fill the configured data into the corresponding buffer
 * INPUT:
 *      attack_type     -- the type of attack
 *      ptr_action      -- the pointer of the attack action
 * OUTPUT:
 *      ptr_wr_data     -- the pointer of SRAM buffer
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_coral_dos_setActionCfg(
    const UI32_T            attack_type,
    const AIR_DOS_ACTION_T *ptr_action,
    UI32_T                 *ptr_wr_data)
{
    UI32_T  attack_offset, offset, len;
    UI32_T *ptr_data;

    HAL_CHECK_PTR(ptr_action);
    HAL_CHECK_PTR(ptr_wr_data);
    HAL_CHECK_ENUM_RANGE(attack_type, HAL_CORAL_DOS_ATTACK_LAST);

    /* Get the action offset start bit */
    attack_offset = attack_type * CORAL_ATTACK_ACTION_LENG;

    /* Set the action flags */
    if (ptr_action->action_flags & AIR_DOS_ACTION_FLAGS_DROP)
    {
        /* Enable drop */
        offset = CORAL_ATTACK_DROP_EN_OFFT(attack_offset);
        ptr_data = ptr_wr_data + (offset / CORAL_ATTACK_ACTION_REG_LENG);
        *ptr_data |= BIT(offset % CORAL_ATTACK_ACTION_REG_LENG);
    }

    if (ptr_action->action_flags & AIR_DOS_ACTION_FLAGS_RATE_LIMIT)
    {
        /* Enable rate limit */
        offset = CORAL_ATTACK_RATE_EN_OFFT(attack_offset);
        ptr_data = ptr_wr_data + (offset / CORAL_ATTACK_ACTION_REG_LENG);
        *ptr_data |= BIT(offset % CORAL_ATTACK_ACTION_REG_LENG);
    }

    /* set the user configured rate id */
    offset = CORAL_ATTACK_RATE_EN_OFFT(attack_offset);
    ptr_data = ptr_wr_data + (attack_offset / CORAL_ATTACK_ACTION_REG_LENG);
    if ((offset % CORAL_ATTACK_ACTION_REG_LENG) < CORAL_ATTACK_ACTION_RATE_ID_LENG)
    {
        /* LSB */
        offset = attack_offset % CORAL_ATTACK_ACTION_REG_LENG;
        len = CORAL_ATTACK_ACTION_REG_LENG - offset;
        *ptr_data |= BITS_OFF_L(ptr_action->rate_cfg_idx, offset, len);
        /* MSB */
        ptr_data++;
        offset = len;
        len = CORAL_ATTACK_ACTION_RATE_ID_LENG - offset;
        *ptr_data |= BITS_OFF_R(ptr_action->rate_cfg_idx, offset, len);
    }
    else
    {
        offset = attack_offset % CORAL_ATTACK_ACTION_REG_LENG;
        *ptr_data |= BITS_OFF_L(ptr_action->rate_cfg_idx, offset, CORAL_ATTACK_ACTION_RATE_ID_LENG);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_coral_dos_getActionCfg
 * PURPOSE:
 *      Get the configuration from the corresponding SRAM data
 * INPUT:
 *      attack_type     -- the type of attack
 *      ptr_rd_data     -- the pointer of SRAM buffer
 * OUTPUT:
 *      ptr_action      -- the pointer of the attack action
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_coral_dos_getActionCfg(
    const UI32_T      attack_type,
    UI32_T           *ptr_rd_data,
    AIR_DOS_ACTION_T *ptr_action)
{
    UI32_T  attack_offset, offset, len;
    UI32_T *ptr_data;

    HAL_CHECK_PTR(ptr_rd_data);
    HAL_CHECK_PTR(ptr_action);
    HAL_CHECK_ENUM_RANGE(attack_type, HAL_CORAL_DOS_ATTACK_LAST);

    attack_offset = attack_type * CORAL_ATTACK_ACTION_LENG;

    /* Get drop */
    offset = CORAL_ATTACK_DROP_EN_OFFT(attack_offset);
    ptr_data = ptr_rd_data + (offset / CORAL_ATTACK_ACTION_REG_LENG);
    if (0 != (*ptr_data & BIT(offset % CORAL_ATTACK_ACTION_REG_LENG)))
    {
        ptr_action->action_flags |= AIR_DOS_ACTION_FLAGS_DROP;
    }

    /* Get rate limit */
    offset = CORAL_ATTACK_RATE_EN_OFFT(attack_offset);
    ptr_data = ptr_rd_data + (offset / CORAL_ATTACK_ACTION_REG_LENG);
    if (0 != (*ptr_data & BIT(offset % CORAL_ATTACK_ACTION_REG_LENG)))
    {
        ptr_action->action_flags |= AIR_DOS_ACTION_FLAGS_RATE_LIMIT;
    }

    /* Get rate id, cross two registers if rate_en offset < 7 */
    if ((offset % CORAL_ATTACK_ACTION_REG_LENG) < CORAL_ATTACK_ACTION_RATE_ID_LENG)
    {
        /* LSB */
        ptr_data = ptr_rd_data + (attack_offset / CORAL_ATTACK_ACTION_REG_LENG);
        offset = attack_offset % CORAL_ATTACK_ACTION_REG_LENG;
        len = CORAL_ATTACK_ACTION_REG_LENG - offset;
        ptr_action->rate_cfg_idx = BITS_OFF_R(*ptr_data, offset, len);
        /* MSB */
        ptr_data++;
        offset = len;
        len = CORAL_ATTACK_ACTION_RATE_ID_LENG - offset;
        ptr_action->rate_cfg_idx |= BITS_OFF_L(*ptr_data, offset, len);
    }
    else
    {
        offset = attack_offset % CORAL_ATTACK_ACTION_REG_LENG;
        ptr_action->rate_cfg_idx = BITS_OFF_R(*ptr_data, offset, CORAL_ATTACK_ACTION_RATE_ID_LENG);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: _hal_coral_dos_getRateLimitUsage
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
_hal_coral_dos_getRateLimitUsage(
    const UI32_T unit,
    UI32_T      *ptr_cnt)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   entry_idx, usage_count = 0;
    AIR_DOS_RATE_LIMIT_CFG_T rate_cfg;

    for (entry_idx = 0; entry_idx <= HAL_CORAL_DOS_MAX_RATE_ID; entry_idx++)
    {
        rc = hal_coral_dos_getRateLimitCfg(unit, entry_idx, &rate_cfg);
        if ((AIR_E_OK == rc) && ((0 != rate_cfg.pkt_thld) || (0 != rate_cfg.time_span) || (0 != rate_cfg.block_time) ||
                                 (0 != rate_cfg.tick_sel)))
        {
            usage_count++;
        }
    }
    *ptr_cnt = usage_count;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: hal_coral_dos_init
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
hal_coral_dos_init(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_deinit
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
hal_coral_dos_deinit(
    const UI32_T unit)
{
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_setGlobalCfg
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
hal_coral_dos_setGlobalCfg(
    const UI32_T unit,
    const BOOL_T enable)
{
    UI32_T u32dat = 0;

    /* Set to register */
    aml_readReg(unit, CORAL_ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_PREVENT_GLB=0x%x\n", unit, u32dat,
               CORAL_ATTACK_PREVENT_GLB);
    if (TRUE == enable)
    {
        u32dat |= BIT(CORAL_ATTACK_PREVENT_GLB_EN_OFFT);
    }
    else
    {
        u32dat &= ~(BIT(CORAL_ATTACK_PREVENT_GLB_EN_OFFT));
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_PREVENT_GLB\n", unit, u32dat);
    aml_writeReg(unit, CORAL_ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_getGlobalCfg
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
hal_coral_dos_getGlobalCfg(
    const UI32_T unit,
    BOOL_T      *ptr_enable)
{
    UI32_T u32dat = 0;

    HAL_CHECK_PTR(ptr_enable);

    /* Read data from register */
    aml_readReg(unit, CORAL_ATTACK_PREVENT_GLB, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_PREVENT_GLB=0x%x\n", unit, u32dat,
               CORAL_ATTACK_PREVENT_GLB);
    if (BITS_OFF_R(u32dat, CORAL_ATTACK_PREVENT_GLB_EN_OFFT, CORAL_ATTACK_PREVENT_GLB_EN_LENG))
    {
        *ptr_enable = TRUE;
    }
    else
    {
        *ptr_enable = FALSE;
    }
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_PREVENT_GLB, enable=%d\n", unit, u32dat, *ptr_enable);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_setIcmpLengthLimit
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
hal_coral_dos_setIcmpLengthLimit(
    const UI32_T unit,
    const UI32_T length)
{
    UI32_T u32dat = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(length, HAL_CORAL_DOS_MIN_ATTACK_LENGTH, HAL_CORAL_DOS_MAX_ATTACK_LENGTH);

    /* Set to register */
    aml_readReg(unit, CORAL_ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_LENGTH_CFG=0x%x\n", unit, u32dat,
               CORAL_ATTACK_LENGTH_CFG);
    u32dat &= ~(BITS_RANGE(CORAL_ATTACK_LENGTH_THLD_OFFT, CORAL_ATTACK_LENGTH_THLD_LENG));
    u32dat |= (length << CORAL_ATTACK_LENGTH_THLD_OFFT);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_LENGTH_CFG\n", unit, u32dat);
    aml_writeReg(unit, CORAL_ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_getIcmpLengthLimit
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
hal_coral_dos_getIcmpLengthLimit(
    const UI32_T unit,
    UI32_T      *ptr_length)
{
    UI32_T u32dat = 0;

    HAL_CHECK_PTR(ptr_length);

    /* Read data from register */
    aml_readReg(unit, CORAL_ATTACK_LENGTH_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_LENGTH_CFG=0x%x\n", unit, u32dat,
               CORAL_ATTACK_LENGTH_CFG);
    (*ptr_length) = BITS_OFF_R(u32dat, CORAL_ATTACK_LENGTH_THLD_OFFT, CORAL_ATTACK_LENGTH_THLD_LENG);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_LENGTH_CFG, length=%u\n", unit, u32dat, *ptr_length);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_setTcpFlagCfg
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
hal_coral_dos_setTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    UI32_T u32dat = 0;
    UI32_T reg_key = 0;
    UI32_T reg_msk = 0;

    /* Check parameter */
    HAL_CHECK_PTR(ptr_tcp_cfg);
    HAL_CHECK_ENUM_RANGE(idx, AIR_DOS_TCP_ID_LAST);
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcp_cfg->key_flags, 0, BITS_RANGE(0, CORAL_ATTACK_TCP_SETTING_LENG));
    HAL_CHECK_MIN_MAX_RANGE(ptr_tcp_cfg->mask_flags, 0, BITS_RANGE(0, CORAL_ATTACK_TCP_SETTING_LENG));

    /* Set TCP flag key */
    _hal_coral_dos_tcpCfgToRegVal(ptr_tcp_cfg->key_flags, &reg_key);
    aml_readReg(unit, CORAL_ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_TCP_FLAG_CFG=0x%x\n", unit, u32dat,
               CORAL_ATTACK_TCP_FLAG_CFG);
    u32dat &= ~(BITS_RANGE(CORAL_ATTACK_TCP_SETTING_OFFT(idx), CORAL_ATTACK_TCP_SETTING_LENG));
    u32dat |= (reg_key << CORAL_ATTACK_TCP_SETTING_OFFT(idx));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_LENGTH_CFG\n", unit, u32dat);
    aml_writeReg(unit, CORAL_ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));

    /* Set TCP flag mask */
    _hal_coral_dos_tcpCfgToRegVal(ptr_tcp_cfg->mask_flags, &reg_msk);
    aml_readReg(unit, CORAL_ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_TCP_FLAG_MSK=0x%x\n", unit, u32dat,
               CORAL_ATTACK_TCP_FLAG_MSK);
    u32dat &= ~(BITS_RANGE(CORAL_ATTACK_TCP_SETTING_OFFT(idx), CORAL_ATTACK_TCP_SETTING_LENG));
    u32dat |= (reg_msk << CORAL_ATTACK_TCP_SETTING_OFFT(idx));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_TCP_FLAG_MSK\n", unit, u32dat);
    aml_writeReg(unit, CORAL_ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_getTcpFlagCfg
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
hal_coral_dos_getTcpFlagCfg(
    const UI32_T       unit,
    const UI32_T       idx,
    AIR_DOS_TCP_CFG_T *ptr_tcp_cfg)
{
    UI32_T u32dat = 0;
    UI32_T reg_key = 0;
    UI32_T reg_msk = 0;

    /* Check parameter */
    HAL_CHECK_PTR(ptr_tcp_cfg);
    HAL_CHECK_ENUM_RANGE(idx, AIR_DOS_TCP_ID_LAST);
    osal_memset(ptr_tcp_cfg, 0, sizeof(AIR_DOS_TCP_CFG_T));

    /* Read data from register */
    /* TCP flag key*/
    aml_readReg(unit, CORAL_ATTACK_TCP_FLAG_CFG, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_TCP_FLAG_CFG=0x%x\n", unit, u32dat,
               CORAL_ATTACK_TCP_FLAG_CFG);
    reg_key = (u32dat >> CORAL_ATTACK_TCP_SETTING_OFFT(idx)) & (BITS(0, CORAL_ATTACK_TCP_SETTING_LENG - 1));
    _hal_coral_dos_regValToTcpCfg(reg_key, &ptr_tcp_cfg->key_flags);

    /* TCP flag mask */
    aml_readReg(unit, CORAL_ATTACK_TCP_FLAG_MSK, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_TCP_FLAG_MSK=0x%x\n", unit, u32dat,
               CORAL_ATTACK_TCP_FLAG_MSK);
    reg_msk = (u32dat >> CORAL_ATTACK_TCP_SETTING_OFFT(idx)) & (BITS(0, CORAL_ATTACK_TCP_SETTING_LENG - 1));
    _hal_coral_dos_regValToTcpCfg(reg_msk, &ptr_tcp_cfg->mask_flags);
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_setPortActionCfg
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
hal_coral_dos_setPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         wr_data[3] = {0};
    UI32_T         mac_port = 0;

    /* Check parameter */
    HAL_CHECK_PTR(ptr_action_cfg);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, land);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, blat);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, length);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, ping);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_0]);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_1]);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp[AIR_DOS_TCP_ID_2]);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, tcp_hdr_min);
    HAL_CORAL_DOS_CHECK_PORT_ACTION_CFG(ptr_action_cfg, ip_land);
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* read action */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (mac_port << CORAL_ATTACK_ACTION_CFG_PID_OFFT);
    aml_writeReg(unit, CORAL_ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_ACTION_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read CORAL_ATTACK_ACTION_CFG=0x%x timeout\n", unit, CORAL_ATTACK_ACTION_CFG);
    }
    else
    {
        /* land */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_LAND, &(ptr_action_cfg->land), wr_data);
        /* blat */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_BLAT, &(ptr_action_cfg->blat), wr_data);
        /* length */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_LENGTH, &(ptr_action_cfg->length), wr_data);
        /* tcp0 */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_TCP0, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0]), wr_data);
        /* tcp1 */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_TCP1, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1]), wr_data);
        /* tcp2 */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_TCP2, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2]), wr_data);
        /* ping */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_PING, &(ptr_action_cfg->ping), wr_data);
        /* tcp hdr min */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_TCP_HDR_MIN, &(ptr_action_cfg->tcp_hdr_min), wr_data);
        /* ip land */
        _hal_coral_dos_setActionCfg(HAL_CORAL_DOS_ATTACK_IP_LAND, &(ptr_action_cfg->ip_land), wr_data);

        /* action SRAM config */
        cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
        cfg_data |= (1 << CORAL_ATTACK_CFG_WR_OFFT);
        cfg_data |= (mac_port << CORAL_ATTACK_ACTION_CFG_PID_OFFT);
        /* Write data to register */
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA0=0x%x\n", unit, wr_data[0],
                   CORAL_ATTACK_ACTION_SRAM_WR_DATA0);
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA1=0x%x\n", unit, wr_data[1],
                   CORAL_ATTACK_ACTION_SRAM_WR_DATA1);
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA1=0x%x\n", unit, wr_data[2],
                   CORAL_ATTACK_ACTION_SRAM_WR_DATA2);
        aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA0, &wr_data[0], sizeof(wr_data[0]));
        aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA1, &wr_data[1], sizeof(wr_data[1]));
        aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA2, &wr_data[2], sizeof(wr_data[2]));
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data,
                   CORAL_ATTACK_ACTION_CFG);
        aml_writeReg(unit, CORAL_ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
        rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_ACTION_CFG);
        if (AIR_E_TIMEOUT == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_ACTION_CFG=0x%x timeout\n", unit,
                       CORAL_ATTACK_ACTION_CFG);
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_getPortActionCfg
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
hal_coral_dos_getPortActionCfg(
    const UI32_T               unit,
    const UI32_T               port,
    AIR_DOS_PORT_ACTION_CFG_T *ptr_action_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         re_data[3] = {0};
    UI32_T         mac_port = 0;

    HAL_CHECK_PTR(ptr_action_cfg);
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (mac_port << CORAL_ATTACK_ACTION_CFG_PID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data,
               CORAL_ATTACK_ACTION_CFG);
    aml_writeReg(unit, CORAL_ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_ACTION_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_ACTION_CFG=0x%x timeout\n", unit, CORAL_ATTACK_ACTION_CFG);
    }
    else
    {
        /* Read data from register */
        aml_readReg(unit, CORAL_ATTACK_ACTION_SRAM_RE_DATA0, &re_data[0], sizeof(re_data[0]));
        aml_readReg(unit, CORAL_ATTACK_ACTION_SRAM_RE_DATA1, &re_data[1], sizeof(re_data[1]));
        aml_readReg(unit, CORAL_ATTACK_ACTION_SRAM_RE_DATA2, &re_data[2], sizeof(re_data[2]));
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ACTION_SRAM_RE_DATA0=0x%x\n", unit, re_data[0],
                   CORAL_ATTACK_ACTION_SRAM_RE_DATA0);
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ACTION_SRAM_RE_DATA1=0x%x\n", unit, re_data[1],
                   CORAL_ATTACK_ACTION_SRAM_RE_DATA1);
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from ACTION_SRAM_RE_DATA2=0x%x\n", unit, re_data[2],
                   CORAL_ATTACK_ACTION_SRAM_RE_DATA2);
        /* land */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_LAND, re_data, &(ptr_action_cfg->land));
        /* blat */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_BLAT, re_data, &(ptr_action_cfg->blat));
        /* length */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_LENGTH, re_data, &(ptr_action_cfg->length));
        /* tcp0 */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_TCP0, re_data, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_0]));
        /* tcp1 */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_TCP1, re_data, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_1]));
        /* tcp2 */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_TCP2, re_data, &(ptr_action_cfg->tcp[AIR_DOS_TCP_ID_2]));
        /* ping */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_PING, re_data, &(ptr_action_cfg->ping));
        /* tcp hdr min */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_TCP_HDR_MIN, re_data, &(ptr_action_cfg->tcp_hdr_min));
        /* ip land */
        _hal_coral_dos_getActionCfg(HAL_CORAL_DOS_ATTACK_IP_LAND, re_data, &(ptr_action_cfg->ip_land));
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_clearPortActionCfg
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
hal_coral_dos_clearPortActionCfg(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         wr_data0 = 0, wr_data1 = 0, wr_data2 = 0;
    UI32_T         mac_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << CORAL_ATTACK_CFG_WR_OFFT);
    cfg_data |= (mac_port << CORAL_ATTACK_ACTION_CFG_PID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               CORAL_ATTACK_ACTION_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               CORAL_ATTACK_ACTION_SRAM_WR_DATA1);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to ACTION_SRAM_WR_DATA2=0x%x\n", unit, wr_data2,
               CORAL_ATTACK_ACTION_SRAM_WR_DATA2);
    aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));
    aml_writeReg(unit, CORAL_ATTACK_ACTION_SRAM_WR_DATA2, &wr_data2, sizeof(wr_data2));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data,
               CORAL_ATTACK_ACTION_CFG);
    aml_writeReg(unit, CORAL_ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_ACTION_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_ACTION_CFG=0x%x timeout\n", unit, CORAL_ATTACK_ACTION_CFG);
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_dos_clearActionCfg
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
hal_coral_dos_clearActionCfg(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << CORAL_ATTACK_CFG_CLEAR_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_ACTION_CFG=0x%x\n", unit, cfg_data,
               CORAL_ATTACK_ACTION_CFG);
    aml_writeReg(unit, CORAL_ATTACK_ACTION_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_ACTION_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_ACTION_CFG=0x%x timeout\n", unit, CORAL_ATTACK_ACTION_CFG);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_setRateLimitCfg
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
hal_coral_dos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         wr_data0 = 0, wr_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_CORAL_DOS_MIN_RATE_ID, HAL_CORAL_DOS_MAX_RATE_ID);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->pkt_thld, HAL_CORAL_DOS_MIN_RATE_WR_CFG, HAL_CORAL_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->time_span, HAL_CORAL_DOS_MIN_RATE_WR_CFG, HAL_CORAL_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_MIN_MAX_RANGE(ptr_rate_cfg->block_time, HAL_CORAL_DOS_MIN_RATE_WR_CFG, HAL_CORAL_DOS_MAX_RATE_WR_CFG);
    HAL_CHECK_ENUM_RANGE(ptr_rate_cfg->tick_sel, AIR_DOS_RATE_TICKSEL_LAST);

    /* rate SRAM write data */
    /* wdata0 - pkt num thld + time span */
    wr_data0 |= (ptr_rate_cfg->pkt_thld << CORAL_ATTACK_RATE_PKT_NUM_THLD_OFFT);
    wr_data0 |= (ptr_rate_cfg->time_span << CORAL_ATTACK_RATE_TIME_SPAN_OFFT);
    /* wdata1 - tick sel + block time */
    wr_data1 |= (ptr_rate_cfg->block_time << CORAL_ATTACK_RATE_BLOCK_TIME_OFFT);
    if (AIR_DOS_RATE_TICKSEL_8MS == ptr_rate_cfg->tick_sel)
    {
        wr_data1 |= (1 << CORAL_ATTACK_RATE_TICK_SEL_OFFT);
    }

    /* rate SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << CORAL_ATTACK_CFG_WR_OFFT);
    cfg_data |= (rate_cfg_idx << CORAL_ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               CORAL_ATTACK_RATE_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               CORAL_ATTACK_RATE_SRAM_WR_DATA1);
    aml_writeReg(unit, CORAL_ATTACK_RATE_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, CORAL_ATTACK_RATE_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, CORAL_ATTACK_RATE_CFG);
    aml_writeReg(unit, CORAL_ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_RATE_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_RATE_CFG=0x%x timeout\n", unit, CORAL_ATTACK_RATE_CFG);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_getRateLimitCfg
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
hal_coral_dos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              rate_cfg_idx,
    AIR_DOS_RATE_LIMIT_CFG_T *ptr_rate_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         re_data0 = 0, re_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_CORAL_DOS_MIN_RATE_ID, HAL_CORAL_DOS_MAX_RATE_ID);

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (rate_cfg_idx << CORAL_ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, CORAL_ATTACK_RATE_CFG);
    aml_writeReg(unit, CORAL_ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_RATE_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_RATE_CFG=0x%x timeout\n", unit, CORAL_ATTACK_RATE_CFG);
    }
    else
    {
        /* Read data from register */
        aml_readReg(unit, CORAL_ATTACK_RATE_SRAM_RE_DATA0, &re_data0, sizeof(re_data0));
        aml_readReg(unit, CORAL_ATTACK_RATE_SRAM_RE_DATA1, &re_data1, sizeof(re_data1));
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from RATE_SRAM_RE_DATA0=0x%x\n", unit, re_data0,
                   CORAL_ATTACK_RATE_SRAM_RE_DATA0);
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from RATE_SRAM_RE_DATA1=0x%x\n", unit, re_data1,
                   CORAL_ATTACK_RATE_SRAM_RE_DATA1);

        ptr_rate_cfg->pkt_thld =
            BITS_OFF_R(re_data0, CORAL_ATTACK_RATE_PKT_NUM_THLD_OFFT, CORAL_ATTACK_RATE_WR_CFG_LENG);
        ptr_rate_cfg->time_span = BITS_OFF_R(re_data0, CORAL_ATTACK_RATE_TIME_SPAN_OFFT, CORAL_ATTACK_RATE_WR_CFG_LENG);

        ptr_rate_cfg->block_time =
            BITS_OFF_R(re_data1, CORAL_ATTACK_RATE_BLOCK_TIME_OFFT, CORAL_ATTACK_RATE_WR_CFG_LENG);
        if (BITS_OFF_R(re_data1, CORAL_ATTACK_RATE_TICK_SEL_OFFT, CORAL_ATTACK_RATE_WR_TICK_SEL_LENG))
        {
            ptr_rate_cfg->tick_sel = AIR_DOS_RATE_TICKSEL_8MS;
        }
        else
        {
            ptr_rate_cfg->tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_clearRateLimitCfg
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
hal_coral_dos_clearRateLimitCfg(
    const UI32_T unit,
    const UI32_T rate_cfg_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;
    UI32_T         wr_data0 = 0, wr_data1 = 0;

    /* Check parameter */
    HAL_CHECK_MIN_MAX_RANGE(rate_cfg_idx, HAL_CORAL_DOS_MIN_RATE_ID, HAL_CORAL_DOS_MAX_RATE_ID);

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << CORAL_ATTACK_CFG_WR_OFFT);
    cfg_data |= (rate_cfg_idx << CORAL_ATTACK_RATE_CFG_RATE_ID_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA0=0x%x\n", unit, wr_data0,
               CORAL_ATTACK_RATE_SRAM_WR_DATA0);
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to RATE_SRAM_WR_DATA1=0x%x\n", unit, wr_data1,
               CORAL_ATTACK_RATE_SRAM_WR_DATA1);
    aml_writeReg(unit, CORAL_ATTACK_RATE_SRAM_WR_DATA0, &wr_data0, sizeof(wr_data0));
    aml_writeReg(unit, CORAL_ATTACK_RATE_SRAM_WR_DATA1, &wr_data1, sizeof(wr_data1));

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, CORAL_ATTACK_RATE_CFG);
    aml_writeReg(unit, CORAL_ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_RATE_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_RATE_CFG=0x%x timeout\n", unit, CORAL_ATTACK_RATE_CFG);
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_dos_clearAllRateLimitCfg
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
hal_coral_dos_clearAllRateLimitCfg(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         cfg_data = 0;

    /* action SRAM config */
    cfg_data |= (1 << CORAL_ATTACK_CFG_EN_OFFT);
    cfg_data |= (1 << CORAL_ATTACK_CFG_CLEAR_OFFT);

    /* Write data to register */
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_RATE_CFG=0x%x\n", unit, cfg_data, CORAL_ATTACK_RATE_CFG);
    aml_writeReg(unit, CORAL_ATTACK_RATE_CFG, &cfg_data, sizeof(cfg_data));
    rc = _hal_coral_dos_checkDosDone(unit, CORAL_ATTACK_RATE_CFG);
    if (AIR_E_TIMEOUT == rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "u=%u, write CORAL_ATTACK_RATE_CFG=0x%x timeout\n", unit, CORAL_ATTACK_RATE_CFG);
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_getDropCnt
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
hal_coral_dos_getDropCnt(
    const UI32_T unit,
    UI32_T      *ptr_drop_cnt)
{
    UI32_T u32dat = 0;

    HAL_CHECK_PTR(ptr_drop_cnt);

    /* Read data from register */
    aml_readReg(unit, CORAL_ATTACK_DROP_CNT, &u32dat, sizeof(u32dat));
    (*ptr_drop_cnt) = u32dat;
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_DROP_CNT=0x%x, length=%u\n", unit, u32dat,
               CORAL_ATTACK_DROP_CNT, *ptr_drop_cnt);

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_clearDropCnt
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
hal_coral_dos_clearDropCnt(
    const UI32_T unit)
{
    UI32_T u32dat;

    /* Set to register */
    aml_readReg(unit, CORAL_ATTACK_DROP_CNT_CLR, &u32dat, sizeof(u32dat));
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, read 0x%x from CORAL_ATTACK_DROP_CNT_CLR=0x%x\n", unit, u32dat,
               CORAL_ATTACK_DROP_CNT_CLR);
    /* Write clear bit */
    u32dat |= CORAL_ATTACK_DROP_CNT_CLR_A0;
    DIAG_PRINT(HAL_DBG_INFO, "u=%u, write 0x%x to CORAL_ATTACK_DROP_CNT_CLR\n", unit, u32dat);
    aml_writeReg(unit, CORAL_ATTACK_DROP_CNT_CLR, &u32dat, sizeof(u32dat));
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_coral_dos_getCapacity
 * PURPOSE:
 *      Get the dos resource capacity
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
hal_coral_dos_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_DOS_RATELIMIT == type)
    {
        *ptr_size = HAL_CORAL_DOS_MAX_RATE_ID + 1;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_dos_getUsage
 * PURPOSE:
 *      Get the dos resource usage
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
hal_coral_dos_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    *ptr_cnt = 0;
    if (AIR_SWC_RSRC_DOS_RATELIMIT == type)
    {
        _hal_coral_dos_getRateLimitUsage(unit, ptr_cnt);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}
