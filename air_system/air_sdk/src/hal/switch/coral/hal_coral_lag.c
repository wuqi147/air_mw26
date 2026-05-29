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

/* FILE NAME:  hal_coral_lag.c
 * PURPOSE:
 *    It provides HAL LAG driver API functions.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/switch/coral/hal_coral_lag.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal.h>
#include <hal/switch/coral/hal_coral_reg.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_LAG, "hal_coral_lag.c");

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_hal_coral_lag_findGroupIdByPort(
    UI32_T  unit,
    UI32_T  port,
    UI32_T *ptr_lag_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         mac_port = 0, reg_data = 0, group_id = 0, member_index = 0, member_port = 0;

    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    for (group_id = 0; group_id < HAL_CORAL_LAG_MAX_GROUP_NUM; group_id++)
    {
        rc = aml_readReg(unit, CORAL_PTG_1(group_id), &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            for (member_index = 0; member_index < HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM; member_index++)
            {
                member_port = BITS_OFF_R(reg_data, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                if (member_port == mac_port)
                {
                    *ptr_lag_id = group_id;
                    break;
                }
            }
            if (HAL_CORAL_LAG_MAX_GROUP_NUM > *ptr_lag_id)
            {
                break;
            }
        }
    }
    if (HAL_CORAL_LAG_MAX_GROUP_NUM > *ptr_lag_id)
    {
        DIAG_PRINT(HAL_DBG_INFO, "port %u, mac_port %u, ptr_lag_id %u\n", port, mac_port, *ptr_lag_id);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "port %u, mac_port %u, ptr_lag_id NULL\n", port, mac_port);
    }
    return rc;
}
/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_coral_lag_init
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
hal_coral_lag_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         member_index = 0, group_index = 0;
    UI32_T         reg_data = 0;

    for (group_index = 0; group_index < HAL_CORAL_LAG_MAX_GROUP_NUM; group_index++)
    {
        /* Read data from register from group-0 to group-1*/
        rc = aml_readReg(unit, CORAL_PTG_1(group_index), &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            for (member_index = 0; member_index < HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM; member_index++)
            {
                /* set member port p0~p3 to AIR_PORT_INVALID and disable */
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data |= BITS_OFF_L(AIR_PORT_INVALID, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
                reg_data |= BITS_OFF_L(FALSE, CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
            }
            /* Write data to register */
            rc = aml_writeReg(unit, CORAL_PTG_1(group_index), &reg_data, sizeof(reg_data));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "unit %u LAG group %u init done.\n", unit, group_index);
            }
        }
    }

    if (AIR_E_OK == rc)
    {
        rc = aml_readReg(unit, CORAL_PTC, &reg_data, sizeof(reg_data));
        reg_data |= CORAL_PTC_SP_SEL;
        if (AIR_E_OK == rc)
        {
            rc = aml_writeReg(unit, CORAL_PTC, &reg_data, sizeof(reg_data));
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_getMaxGroupCnt
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
hal_coral_lag_getMaxGroupCnt(
    const UI32_T unit,
    UI32_T      *ptr_group_cnt,
    UI32_T      *ptr_member_cnt)
{
    /* Assign maximum group number and member number by chip defined*/
    *ptr_group_cnt = HAL_CORAL_LAG_MAX_GROUP_NUM;
    *ptr_member_cnt = HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM;
    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_coral_lag_createGroup
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
hal_coral_lag_createGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_CORAL_LAG_VALID_MIN, HAL_CORAL_LAG_VALID_MAX);
    rc = aml_readReg(unit, CORAL_PTGC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        /* Enabled LAG */
        reg_data |= BIT(lag_id);
        rc = aml_writeReg(unit, CORAL_PTGC, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "unit %u create LAG group %u.\n", unit, lag_id);
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_destroyPort
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
hal_coral_lag_destroyGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, member_index = 0;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_CORAL_LAG_VALID_MIN, HAL_CORAL_LAG_VALID_MAX);

    /* Disable LAG */
    rc = aml_readReg(unit, CORAL_PTGC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        reg_data &= ~BIT(lag_id);
        rc = aml_writeReg(unit, CORAL_PTGC, &reg_data, sizeof(reg_data));
        if (AIR_E_OK == rc)
        {
            reg_data = 0;
            /* Clear all members of LAG */
            for (member_index = 0; member_index < HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM; member_index++)
            {
                /* set member port p0~p3 to AIR_PORT_INVALID and disable */
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data |= BITS_OFF_L(AIR_PORT_INVALID, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
                reg_data |= BITS_OFF_L(FALSE, CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
            }
            rc = aml_writeReg(unit, CORAL_PTG_1(lag_id), &reg_data, sizeof(reg_data));
            if (AIR_E_OK == rc)
            {
                DIAG_PRINT(HAL_DBG_INFO, "unit %u destroy LAG group %u.\n", unit, lag_id);
            }
        }
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_getGroup
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
hal_coral_lag_getGroup(
    const UI32_T unit,
    const UI32_T lag_id,
    BOOL_T      *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_CORAL_LAG_VALID_MIN, HAL_CORAL_LAG_VALID_MAX);
    rc = aml_readReg(unit, CORAL_PTGC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        *ptr_state = (reg_data & BIT(lag_id)) ? TRUE : FALSE;
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_setMember
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
hal_coral_lag_setMember(
    const UI32_T  unit,
    const UI32_T  lag_id,
    const UI32_T  member_cnt,
    const UI32_T *ptr_member)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         member_index = 0, reg_data = 0, member_port = 0;
    UI32_T         group_id = HAL_CORAL_LAG_MAX_GROUP_NUM;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_CORAL_LAG_VALID_MIN, HAL_CORAL_LAG_VALID_MAX);
    HAL_CHECK_MIN_MAX_RANGE(member_cnt, 0, HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM);

    for (member_index = 0; member_index < member_cnt; member_index++)
    {
        rc = _hal_coral_lag_findGroupIdByPort(unit, ptr_member[member_index], &group_id);
        if (AIR_E_OK == rc)
        {
            if (group_id < HAL_CORAL_LAG_MAX_GROUP_NUM && group_id != lag_id)
            {
                DIAG_PRINT(HAL_DBG_ERR, "port %u is exsit in other group.\n", ptr_member[member_index]);
                return AIR_E_OTHERS;
            }
        }
    }

    rc = aml_readReg(unit, CORAL_PTG_1(lag_id), &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        for (member_index = 0; member_index < HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM; member_index++)
        {
            if (member_index < member_cnt) /* ptr_member[member] has value */
            {
                HAL_AIR_PORT_TO_MAC_PORT(unit, ptr_member[member_index], member_port);
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data |= BITS_OFF_L(member_port, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);

                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
                reg_data |= BITS_OFF_L(TRUE, CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
            }
            else /* ptr_member[member] has no value */
            {
                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
                reg_data |= BITS_OFF_L(AIR_DST_DEFAULT_PORT, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);

                reg_data &= ~BITS_RANGE(CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
                reg_data |= BITS_OFF_L(FALSE, CORAL_PTGN_PORT_EN_OFFSET(member_index), CORAL_PTGN_PORT_EN_LEN);
            }
        }
        rc = aml_writeReg(unit, CORAL_PTG_1(lag_id), &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_getMember
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
hal_coral_lag_getMember(
    const UI32_T unit,
    const UI32_T lag_id,
    UI32_T      *member_cnt,
    UI32_T      *ptr_member)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, member_index = 0, member_port = 0, cnt = 0;

    /* Mistake proofing */
    HAL_CHECK_MIN_MAX_RANGE(lag_id, HAL_CORAL_LAG_VALID_MIN, HAL_CORAL_LAG_VALID_MAX);
    rc = aml_readReg(unit, CORAL_PTG_1(lag_id), &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        for (member_index = 0; member_index < HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM; member_index++)
        {
            member_port = BITS_OFF_R(reg_data, CORAL_PTGN_PORT_OFFSET(member_index), CORAL_PTGN_PORT_LEN);
            if (AIR_DST_DEFAULT_PORT != member_port)
            {
                HAL_MAC_PORT_TO_AIR_PORT(unit, member_port, ptr_member[cnt]);
                cnt++;
            }
        }
        *member_cnt = cnt;
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_setHashControl
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
hal_coral_lag_setHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0;

    HAL_CHECK_ENUM_RANGE(ptr_hash_ctrl->hash_algo, AIR_LAG_HASH_ALGO_LAST);

    rc = aml_readReg(unit, CORAL_PTC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        reg_data &= ~BITS_RANGE(CORAL_PTC_HASH_CONTROL_OFFT, CORAL_PTC_HASH_CONTROL_LEN);
        reg_data |= BITS_OFF_L(ptr_hash_ctrl->hash_algo, CORAL_PTC_HASH_CONTROL_OFFT, CORAL_PTC_HASH_CONTROL_LEN);

        reg_data &= ~BITS_RANGE(CORAL_PTC_INFO_OFFT, CORAL_PTC_INFO_LEN);
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT))
        {
            reg_data |= CORAL_PTC_INFO_SEL_SP;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC))
        {
            reg_data |= CORAL_PTC_INFO_SEL_SA;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC))
        {
            reg_data |= CORAL_PTC_INFO_SEL_DA;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP))
        {
            reg_data |= CORAL_PTC_INFO_SEL_SIP;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP))
        {
            reg_data |= CORAL_PTC_INFO_SEL_DIP;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT))
        {
            reg_data |= CORAL_PTC_INFO_SEL_SPORT;
        }
        if (AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT == (ptr_hash_ctrl->flags & AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT))
        {
            reg_data |= CORAL_PTC_INFO_SEL_DPORT;
        }
        rc = aml_writeReg(unit, CORAL_PTC, &reg_data, sizeof(reg_data));
    }
    return rc;
}

/* FUNCTION NAME:   hal_coral_lag_getHashControl
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
hal_coral_lag_getHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         reg_data = 0, hashType = 0;

    rc = aml_readReg(unit, CORAL_PTC, &reg_data, sizeof(reg_data));
    if (AIR_E_OK == rc)
    {
        hashType = BITS_OFF_R(reg_data, CORAL_PTC_HASH_CONTROL_OFFT, CORAL_PTC_HASH_CONTROL_LEN);
        ptr_hash_ctrl->hash_algo = hashType;
        if (reg_data & CORAL_PTC_INFO_SEL_SP)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_IGR_PORT;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_SA)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_MAC;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_DA)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_MAC;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_SIP)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_IP;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_DIP)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_IP;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_SPORT)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_SRC_PORT;
        }
        if (reg_data & CORAL_PTC_INFO_SEL_DPORT)
        {
            ptr_hash_ctrl->flags |= AIR_LAG_HASH_CTRL_FLAGS_EN_DST_PORT;
        }
    }
    return rc;
}

/* FUNCTION NAME: hal_coral_lag_getCapacity
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
 */
AIR_ERROR_NO_T
hal_coral_lag_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_SWC_RSRC_LAG_GROUP == type)
    {
        *ptr_size = HAL_CORAL_LAG_MAX_GROUP_NUM;
    }
    else if (AIR_SWC_RSRC_LAG_GROUP_MEMBER == type)
    {
        *ptr_size = HAL_CORAL_LAG_MAX_MEMBER_PORT_NUM;
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME: hal_coral_lag_getUsage
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
 */
AIR_ERROR_NO_T
hal_coral_lag_getUsage(
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
        for (group_index = 0; group_index < HAL_CORAL_LAG_MAX_GROUP_NUM; group_index++)
        {
            rc = hal_coral_lag_getGroup(unit, group_index, &state);
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
        rc = hal_coral_lag_getMember(unit, param, &member_count, &member_list);
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
