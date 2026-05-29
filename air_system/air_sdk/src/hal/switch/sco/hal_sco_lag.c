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

/* FILE NAME:  hal_sco_lag.c
 * PURPOSE:
 *    It provides HAL LAG driver API functions.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/sco/hal_sco_lag.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_LAG_PTC_DEFAULT_VALUE (0x0000007F)
#define HAL_SCO_LAG_PTG_DEFAULT_VALUE (0)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* lag configuration info */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_LAG, "hal_sco_lag.c");

static void
_hal_sco_lag_findGroupIdByPort(
    UI32_T  unit,
    UI32_T  port,
    UI32_T *ptr_lag_id)
{
    UI32_T mac_port = 0;
    UI32_T regPTG_1 = 0, regPTG_2 = 0;
    UI32_T g_id = 0, m_id = 0;
    UI32_T member_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    for (g_id = 0; g_id < HAL_SCO_LAG_MAX_GROUP_NUM; g_id++)
    {
        /* Read data from register */
        aml_readReg(unit, PTG_1(g_id), &regPTG_1, sizeof(regPTG_1));
        aml_readReg(unit, PTG_2(g_id), &regPTG_2, sizeof(regPTG_2));
        for (m_id = 0; m_id < HAL_SCO_LAG_MAX_MEMBER_PORT_NUM; m_id++)
        {
            if ((m_id < 4)) /* member p0 ~ p3 */
            {
                member_port = BITS_OFF_R(regPTG_1, PTGN_PORT_OFFSET(m_id), PTGN_PORT_LEN);
                if (member_port == mac_port)
                {
                    *ptr_lag_id = g_id;
                    break;
                }
            }
            else /* member p4 ~ p7 */
            {
                member_port = BITS_OFF_R(regPTG_2, PTGN_PORT_OFFSET(m_id), PTGN_PORT_LEN);
                if (member_port == mac_port)
                {
                    *ptr_lag_id = g_id;
                    break;
                }
            }
        }
        if (*ptr_lag_id < HAL_SCO_LAG_MAX_GROUP_NUM)
        {
            break;
        }
    }
    if (*ptr_lag_id < HAL_SCO_LAG_MAX_GROUP_NUM)
    {
        DIAG_PRINT(HAL_DBG_INFO, "port:%u, mac_port:%u, ptr_lag_id:%u\n", port, mac_port, *ptr_lag_id);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "port:%u, mac_port:%u, ptr_lag_id:NULL\n", port, mac_port);
    }
}

/* FUNCTION NAME:   hal_sco_lag_init
 * PURPOSE:
 *      This API is used to init a LAG Group.member
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_EXISTS -- Entry exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_init(
    const UI32_T unit)
{
    UI32_T member_index, group_index;
    UI32_T regPTG_1, regPTG_2;

    for (group_index = 0; group_index < HAL_SCO_LAG_MAX_GROUP_NUM; group_index++)
    {
        /* Read data from register from group-0 to group-7*/
        aml_readReg(unit, PTG_1(group_index), &regPTG_1, sizeof(regPTG_1));
        aml_readReg(unit, PTG_2(group_index), &regPTG_2, sizeof(regPTG_2));
        for (member_index = 0; member_index < HAL_SCO_LAG_MAX_GROUP_NUM / 2; member_index++)
        {
            /* set member port p0~p3 to AIR_PORT_INVALID and disable */
            regPTG_1 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_1 |= BITS_OFF_L(AIR_PORT_INVALID, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_1 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            regPTG_1 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            /* set member port p4~p7 to AIR_PORT_INVALID and disable */
            regPTG_2 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_2 |= BITS_OFF_L(AIR_PORT_INVALID, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_2 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            regPTG_2 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        }
        /* Write data to register */
        aml_writeReg(unit, PTG_1(group_index), &regPTG_1, sizeof(regPTG_1));
        aml_writeReg(unit, PTG_2(group_index), &regPTG_2, sizeof(regPTG_2));
    }
    DIAG_PRINT(HAL_DBG_INFO, "unit=%u Init Done\n", unit);
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_deinit
 * PURPOSE:
 *      This API is used to deinit a LAG Group.member
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_deinit(
    const UI32_T unit)
{
    UI32_T group_index, u32dat;

    u32dat = HAL_SCO_LAG_PTC_DEFAULT_VALUE;
    aml_writeReg(unit, PTC, &u32dat, sizeof(u32dat));
    u32dat = HAL_SCO_LAG_PTG_DEFAULT_VALUE;
    aml_writeReg(unit, PTGC, &u32dat, sizeof(u32dat));
    for (group_index = 0; group_index < HAL_SCO_LAG_MAX_GROUP_NUM; group_index++)
    {
        aml_writeReg(unit, PTG_1(group_index), &u32dat, sizeof(u32dat));
        aml_writeReg(unit, PTG_2(group_index), &u32dat, sizeof(u32dat));
    }
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_getMaxGroupCnt
 * PURPOSE:
 *      This API is used to get LAG maximum group.number
 * INPUT:
 *      unit            -- Device unit number
 *      ptr_group_cnt   -- Lag maximum group.number
 *      ptr_member_cnt  -- Lag maximum member.number
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_EXISTS -- Entry exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_getMaxGroupCnt(
    const UI32_T unit,
    UI32_T      *ptr_group_cnt,
    UI32_T      *ptr_member_cnt)
{
    /* Assign maximum group number and member number by chip defined*/
    *ptr_group_cnt = HAL_SCO_LAG_MAX_GROUP_NUM;
    *ptr_member_cnt = HAL_SCO_LAG_MAX_MEMBER_PORT_NUM;

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_createGroup
 * PURPOSE:
 *      This API is used to create a LAG Group.
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_EXISTS -- Entry exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_createGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    UI32_T regPTGC;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_SCO_LAG_VALID_MIN, HAL_SCO_LAG_VALID_MAX);

    /* Read data from register */
    aml_readReg(unit, PTGC, &regPTGC, sizeof(regPTGC));

    regPTGC |= BIT(lag_id);

    /* Write data to register */
    aml_writeReg(unit, PTGC, &regPTGC, sizeof(regPTGC));
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_destroyPort
 * PURPOSE:
 *      This API is used to destroy a LAG Group.
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_destroyGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    UI32_T regPTGC = 0, regPTG_1 = 0, regPTG_2 = 0;
    UI32_T member_index = 0;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_SCO_LAG_VALID_MIN, HAL_SCO_LAG_VALID_MAX);

    /* Disable LAG */
    aml_readReg(unit, PTGC, &regPTGC, sizeof(regPTGC));
    regPTGC &= ~BIT(lag_id);
    aml_writeReg(unit, PTGC, &regPTGC, sizeof(regPTGC));

    /* Clear all members of LAG */
    for (member_index = 0; member_index < HAL_SCO_LAG_MAX_GROUP_NUM / 2; member_index++)
    {
        /* set member port p0~p3 to AIR_PORT_INVALID and disable */
        regPTG_1 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        regPTG_1 |= BITS_OFF_L(AIR_PORT_INVALID, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        regPTG_1 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        regPTG_1 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        /* set member port p4~p7 to AIR_PORT_INVALID and disable */
        regPTG_2 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        regPTG_2 |= BITS_OFF_L(AIR_PORT_INVALID, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        regPTG_2 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        regPTG_2 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
    }
    aml_writeReg(unit, PTG_1(lag_id), &regPTG_1, sizeof(regPTG_1));
    aml_writeReg(unit, PTG_2(lag_id), &regPTG_2, sizeof(regPTG_2));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_getGroup
 * PURPOSE:
 *      This API is used to get a LAG Group.
 * INPUT:
 *      unit      -- Device unit number
 *      lag_id    -- LAG group id
 * OUTPUT:
 *      ptr_state -- LAG group state
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_getGroup(
    const UI32_T unit,
    const UI32_T lag_id,
    BOOL_T      *ptr_state)
{
    UI32_T regPTGC;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_SCO_LAG_VALID_MIN, HAL_SCO_LAG_VALID_MAX);

    /* Read data from register */
    aml_readReg(unit, PTGC, &regPTGC, sizeof(regPTGC));

    if (regPTGC & BIT(lag_id))
    {
        *ptr_state = TRUE;
    }
    else
    {
        *ptr_state = FALSE;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_setMember
 * PURPOSE:
 *      This API is used to set member port for a LAG entry.
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 *      member_cnt -- Member port count
 *      ptr_membe -- Member port list
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_setMember(
    const UI32_T  unit,
    const UI32_T  lag_id,
    const UI32_T  member_cnt,
    const UI32_T *ptr_member)
{
    UI32_T member_index;
    UI32_T regPTG_1, regPTG_2;
    UI32_T member_port = 0;
    UI32_T g_id = HAL_SCO_LAG_MAX_GROUP_NUM;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_SCO_LAG_VALID_MIN, HAL_SCO_LAG_VALID_MAX);
    HAL_CHECK_MIN_MAX_RANGE(member_cnt, 0, HAL_SCO_LAG_MAX_MEMBER_PORT_NUM);

    for (member_index = 0; member_index < member_cnt; member_index++)
    {
        _hal_sco_lag_findGroupIdByPort(unit, ptr_member[member_index], &g_id);
        if (g_id < HAL_SCO_LAG_MAX_GROUP_NUM && g_id != lag_id)
        {
            DIAG_PRINT(HAL_DBG_ERR, "port %u is exsit in other group.\n", ptr_member[member_index]);
            return AIR_E_OTHERS;
        }
    }

    /* Read data from register */
    aml_readReg(unit, PTG_1(lag_id), &regPTG_1, sizeof(regPTG_1));
    aml_readReg(unit, PTG_2(lag_id), &regPTG_2, sizeof(regPTG_2));

    for (member_index = 0; member_index < HAL_SCO_LAG_MAX_MEMBER_PORT_NUM; member_index++)
    {
        if ((member_index < 4) && (member_index < member_cnt)) /* member p0 ~ p3 ,ptr_member[member] has value */
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_member[member_index], member_port);
            regPTG_1 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_1 |= BITS_OFF_L(member_port, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);

            regPTG_1 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            regPTG_1 |= BITS_OFF_L(TRUE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        }
        else if ((member_index >= 4) && (member_index < member_cnt)) /* member p4 ~ p7 ,ptr_member[member] has value */
        {
            HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_member[member_index], member_port);
            regPTG_2 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
            regPTG_2 |= BITS_OFF_L(member_port, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);

            regPTG_2 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            regPTG_2 |= BITS_OFF_L(TRUE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
        }
        else /* ptr_member[member] has no value */
        {
            if (member_index < 4)
            {
                regPTG_1 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
                regPTG_1 |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);

                regPTG_1 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
                regPTG_1 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            }
            else
            {
                regPTG_2 &= ~BITS_RANGE(PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
                regPTG_2 |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);

                regPTG_2 &= ~BITS_RANGE(PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
                regPTG_2 |= BITS_OFF_L(FALSE, PTGN_PORT_EN_OFFSET(member_index), PTGN_PORT_EN_LEN);
            }
        }
    }

    /* Write data to register */
    aml_writeReg(unit, PTG_1(lag_id), &regPTG_1, sizeof(regPTG_1));
    aml_writeReg(unit, PTG_2(lag_id), &regPTG_2, sizeof(regPTG_2));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_getMember
 * PURPOSE:
 *      This API is used to get member port for a LAG entry.
 * INPUT:
 *      unit -- Device unit number
 *      lag_id -- LAG group id
 * OUTPUT:
 *      member_cnt -- Get member port count
 *      ptr_member -- Member port list
 * RETURN:
 *      AIR_E_OK -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_getMember(
    const UI32_T unit,
    const UI32_T lag_id,
    UI32_T      *member_cnt,
    UI32_T      *ptr_member)
{
    UI32_T member_index;
    UI32_T regPTG_1, regPTG_2;
    UI32_T member_port, cnt = 0;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_SCO_LAG_VALID_MIN, HAL_SCO_LAG_VALID_MAX);

    /* Read data from register */
    aml_readReg(unit, PTG_1(lag_id), &regPTG_1, sizeof(regPTG_1));
    aml_readReg(unit, PTG_2(lag_id), &regPTG_2, sizeof(regPTG_2));

    for (member_index = 0; member_index < HAL_SCO_LAG_MAX_MEMBER_PORT_NUM; member_index++)
    {
        if (member_index < 4) /* member p0 ~ p3 */
        {
            member_port = BITS_OFF_R(regPTG_1, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        }
        else /* member p4 ~ p7 */
        {
            member_port = BITS_OFF_R(regPTG_2, PTGN_PORT_OFFSET(member_index), PTGN_PORT_LEN);
        }

        if (member_port != AIR_DST_DEFAULT_PORT)
        {
            HAL_MAC_PORT_TO_AIR_PORT(unit, member_port, ptr_member[cnt]);
            cnt++;
        }
    }
    *member_cnt = cnt;
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_setHashControl
 * PURPOSE:
 *      Select a LAG hash control for a LAG group.
 * INPUT:
 *      unit --  Device unit number
 *      ptr_hash_ctrl -- Hash control setting
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK --  Operate success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_setHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    UI32_T regPTC;

    HAL_CHECK_ENUM_RANGE(ptr_hash_ctrl->hash_algo, AIR_LAG_HASH_ALGO_LAST);

    /* Read data from register */
    aml_readReg(unit, PTC, &regPTC, sizeof(regPTC));

    /* set lag hash type */
    regPTC &= ~BITS_RANGE(PTC_HASH_CONTROL_OFFT, PTC_HASH_CONTROL_LEN);
    regPTC |= BITS_OFF_L(ptr_hash_ctrl->hash_algo, PTC_HASH_CONTROL_OFFT, PTC_HASH_CONTROL_LEN);

    /* Set infomation control bit map */
    regPTC &= ~BITS_RANGE(PTC_INFO_OFFT, PTC_INFO_LEN);
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT) == AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT)
    {
        regPTC |= PTC_INFO_SEL_SP;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC) == AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC)
    {
        regPTC |= PTC_INFO_SEL_SA;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC) == AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC)
    {
        regPTC |= PTC_INFO_SEL_DA;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP) == AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP)
    {
        regPTC |= PTC_INFO_SEL_SIP;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP) == AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP)
    {
        regPTC |= PTC_INFO_SEL_DIP;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT) == AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT)
    {
        regPTC |= PTC_INFO_SEL_SPORT;
    }
    if ((ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT) == AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT)
    {
        regPTC |= PTC_INFO_SEL_DPORT;
    }

    /* Write data to register */
    aml_writeReg(unit, PTC, &regPTC, sizeof(regPTC));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_sco_lag_getHashControl
 * PURPOSE:
 *      Get hash control config from LAG group.
 * INPUT:
 *      unit --  Device unit number
 * OUTPUT:
 *      ptr_hash_ctrl -- Hash control setting
 * RETURN:
 *      AIR_E_OK --  Operate success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_lag_getHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    UI32_T regPTC;
    UI32_T lagType;

    /* Read data from register */
    aml_readReg(unit, PTC, &regPTC, sizeof(regPTC));

    lagType = BITS_OFF_R(regPTC, PTC_HASH_CONTROL_OFFT, PTC_HASH_CONTROL_LEN);
    ptr_hash_ctrl->hash_algo = lagType;

    if (regPTC & PTC_INFO_SEL_SP)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT;
    }
    if (regPTC & PTC_INFO_SEL_SA)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC;
    }
    if (regPTC & PTC_INFO_SEL_DA)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC;
    }
    if (regPTC & PTC_INFO_SEL_SIP)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP;
    }
    if (regPTC & PTC_INFO_SEL_DIP)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP;
    }
    if (regPTC & PTC_INFO_SEL_SPORT)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT;
    }
    if (regPTC & PTC_INFO_SEL_DPORT)
    {
        ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_lag_getCapacity
 * PURPOSE:
 *      Get the lag resource capacity
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
hal_sco_lag_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_LAG_GROUP == type)
    {
        *ptr_size = HAL_SCO_LAG_MAX_GROUP_NUM;
    }
    else if (AIR_SWC_RSRC_LAG_GROUP_MEMBER == type)
    {
        *ptr_size = HAL_SCO_LAG_MAX_MEMBER_PORT_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_sco_lag_getUsage
 * PURPOSE:
 *      Get the lag resource usage
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
hal_sco_lag_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         member_count = 0, member_list = 0, group_index = 0, use_count = 0;
    BOOL_T         state = FALSE;

    if (AIR_SWC_RSRC_LAG_GROUP == type)
    {
        for (group_index = 0; group_index < HAL_SCO_LAG_MAX_GROUP_NUM; group_index++)
        {
            rc = hal_sco_lag_getGroup(unit, group_index, &state);
            if (AIR_E_OK == rc)
            {
                if (TRUE == state)
                {
                    use_count++;
                }
            }
        }
        *ptr_cnt = use_count;
    }
    else if (AIR_SWC_RSRC_LAG_GROUP_MEMBER == type)
    {
        rc = hal_sco_lag_getMember(unit, param, &member_count, &member_list);
        if (AIR_E_OK == rc)
        {
            *ptr_cnt = member_count;
        }
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}
