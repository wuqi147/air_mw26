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

/* FILE NAME:  air_lag.c
 * PURPOSE:
 *    It provide LAG module API.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_lag.h>

#include <api/diag.h>
#include <hal/common/hal.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
/*
DIAG_SET_MODULE_INFO(AIR_MODULE_LAG, "air_lag.c");
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_lag_getMaxGroupCnt
 * PURPOSE:
 *      This API is used to get LAG maximum group number.
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_group_cnt        -- Lag maximum group number
 *      ptr_member_cnt       -- Lag maximum member number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_ENTRY_EXISTS   -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getMaxGroupCnt(
    const UI32_T unit,
    UI32_T      *ptr_group_cnt,
    UI32_T      *ptr_member_cnt)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_group_cnt);
    HAL_CHECK_PTR(ptr_member_cnt);

    rc = HAL_FUNC_CALL(unit, lag, getMaxGroupCnt, (unit, ptr_group_cnt, ptr_member_cnt));
    return rc;
}

/* FUNCTION NAME:   air_lag_createGroup
 * PURPOSE:
 *      This API is used to create a LAG Group.
 * INPUT:
 *      unit                 -- Device unit number
 *      lag_id               -- LAG group id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_createGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);

    rc = HAL_FUNC_CALL(unit, lag, createGroup, (unit, lag_id));
    return rc;
}

/* FUNCTION NAME:   air_lag_destroyGroup
 * PURPOSE:
 *      This API is used to destroy a LAG Group.
 * INPUT:
 *      unit                 -- Device unit number
 *      lag_id               -- LAG group id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_destroyGroup(
    const UI32_T unit,
    const UI32_T lag_id)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);

    rc = HAL_FUNC_CALL(unit, lag, destroyGroup, (unit, lag_id));
    return rc;
}

/* FUNCTION NAME:   air_lag_getGroup
 * PURPOSE:
 *      This API is used to get a LAG Group.
 * INPUT:
 *      unit                 -- Device unit number
 *      lag_id               -- LAG group id
 * OUTPUT:
 *      ptr_state            -- LAG group state
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getGroup(
    const UI32_T unit,
    const UI32_T lag_id,
    BOOL_T      *ptr_state)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_state);

    rc = HAL_FUNC_CALL(unit, lag, getGroup, (unit, lag_id, ptr_state));
    return rc;
}

/* FUNCTION NAME:   air_lag_setMember
 * PURPOSE:
 *      This API is used to set member port for a LAG entry.
 * INPUT:
 *      unit                 -- Device unit number
 *      lag_id               -- LAG group id
 *      member_cnt           -- Member port count
 *      ptr_membe            -- Member port list
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_setMember(
    const UI32_T  unit,
    const UI32_T  lag_id,
    const UI32_T  member_cnt,
    const UI32_T *ptr_member)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_member);

    rc = HAL_FUNC_CALL(unit, lag, setMember, (unit, lag_id, member_cnt, ptr_member));
    return rc;
}

/* FUNCTION NAME:   air_lag_getMember
 * PURPOSE:
 *      This API is used to get member port for a LAG entry.
 * INPUT:
 *      unit                 -- Device unit number
 *      lag_id               -- LAG group id
 * OUTPUT:
 *      member_cnt           -- Get member port count
 *      ptr_member           -- Member port list
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getMember(
    const UI32_T unit,
    const UI32_T lag_id,
    UI32_T      *member_cnt,
    UI32_T      *ptr_member)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(member_cnt);
    HAL_CHECK_PTR(ptr_member);

    rc = HAL_FUNC_CALL(unit, lag, getMember, (unit, lag_id, member_cnt, ptr_member));
    return rc;
}

/* FUNCTION NAME:   air_lag_setHashControl
 * PURPOSE:
 *      Select a LAG hash control for a LAG group.
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_hash_ctrl        -- Hash control setting
 *                              AIR_LAG_HASH_CTRL_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_setHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_hash_ctrl);

    rc = HAL_FUNC_CALL(unit, lag, setHashControl, (unit, ptr_hash_ctrl));
    return rc;
}

/* FUNCTION NAME:   air_lag_getHashControl
 * PURPOSE:
 *      Get hash control config from LAG group.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_hash_ctrl        -- Hash control setting
 *                              AIR_LAG_HASH_CTRL_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_lag_getHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_hash_ctrl);

    rc = HAL_FUNC_CALL(unit, lag, getHashControl, (unit, ptr_hash_ctrl));
    return rc;
}
