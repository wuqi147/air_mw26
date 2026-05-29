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

/* FILE NAME:  hal_pearl_lag.h
 * PURPOSE:
 *  It provides HAL LAG driver structure and driver APIs.
 * NOTES:
 *
 */

#ifndef HAL_PEARL_LAG_H
#define HAL_PEARL_LAG_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_lag.h>
#include <air_port.h>
#include <air_swc.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_LAG_VALID_MIN           (0) /* Minimum LAG ID */
#define HAL_PEARL_LAG_VALID_MAX           (1) /* Maximum LAG ID */
#define HAL_PEARL_LAG_MAX_GROUP_NUM       (2) /* LAG group port range 0 - 1 */
#define HAL_PEARL_LAG_MAX_MEMBER_PORT_NUM (4) /* LAG member port range 0 - 3 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_PEARL_LAG_IS_LAG_ID_VALID(lag_id)                                        \
    (((lag_id) >= HAL_PEARL_LAG_VALID_MIN) && ((lag_id) <= HAL_PEARL_LAG_VALID_MAX))

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    HAL_PEARL_LAG_HASH_TABLE_1 = 0,
    HAL_PEARL_LAG_HASH_TABLE_2,
    HAL_PEARL_LAG_HASH_TABLE_LAST
} HAL_PEARL_LAG_HASH_TABLE_ID_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   hal_pearl_lag_init
 * PURPOSE:
 *      This API is used to init a LAG Group.member
 * INPUT:
 *      unit -- Device unit number
 * OUTPUT:
 * RETURN:
 *      AIR_E_OK --  Operation success.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 *      AIR_E_ENTRY_EXISTS -- Entry exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_lag_init(
    const UI32_T unit);

/* FUNCTION NAME:   hal_pearl_lag_getMaxGroupCnt
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
hal_pearl_lag_getMaxGroupCnt(
    const UI32_T unit,
    UI32_T      *ptr_group_cnt,
    UI32_T      *ptr_member_cnt);

/* FUNCTION NAME:   hal_pearl_lag_createGroup
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
hal_pearl_lag_createGroup(
    const UI32_T unit,
    const UI32_T lag_id);

/* FUNCTION NAME:   hal_pearl_lag_destroyGroup
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
hal_pearl_lag_destroyGroup(
    const UI32_T unit,
    const UI32_T lag_id);

/* FUNCTION NAME:   hal_pearl_lag_getGroup
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
hal_pearl_lag_getGroup(
    const UI32_T unit,
    const UI32_T lag_id,
    BOOL_T      *ptr_state);

/* FUNCTION NAME:   hal_pearl_lag_setMember
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
hal_pearl_lag_setMember(
    const UI32_T  unit,
    const UI32_T  lag_id,
    const UI32_T  member_cnt,
    const UI32_T *ptr_member);

/* FUNCTION NAME:   hal_pearl_lag_getMember
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
hal_pearl_lag_getMember(
    const UI32_T unit,
    const UI32_T lag_id,
    UI32_T      *member_cnt,
    UI32_T      *ptr_member);

/* FUNCTION NAME:   hal_pearl_lag_setHashControl
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
hal_pearl_lag_setHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl);

/* FUNCTION NAME:   hal_pearl_lag_getHashControl
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
hal_pearl_lag_getHashControl(
    const UI32_T         unit,
    AIR_LAG_HASH_CTRL_T *ptr_hash_ctrl);

/* FUNCTION NAME: hal_pearl_lag_getCapacity
 * PURPOSE:
 *      Get the lag resource capacity
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
hal_pearl_lag_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

#endif /* End of HAL_PEARL_LAG_H */
