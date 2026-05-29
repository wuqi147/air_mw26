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

/* FILE NAME:  air_perif.c
 * PURPOSE:
 *    It provides peripheral module API.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_perif.h>

#include <air_error.h>
#include <air_init.h>
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

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_perif_setGpioDirection
 * PURPOSE:
 *      This API is used to set gpio pin direction
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      direction            -- Gpio direction
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_setGpioDirection(
    const UI32_T                     unit,
    const UI32_T                     pin,
    const AIR_PERIF_GPIO_DIRECTION_T direction)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(direction, AIR_PERIF_GPIO_DIRECTION_LAST);
    return HAL_FUNC_CALL(unit, perif, setGpioDirection, (unit, pin, direction));
}

/* FUNCTION NAME:   air_perif_getGpioDirection
 * PURPOSE:
 *      This API is used to get gpio pin direction
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_direction        -- Pointer of gpio direction
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_getGpioDirection(
    const UI32_T                unit,
    const UI32_T                pin,
    AIR_PERIF_GPIO_DIRECTION_T *ptr_direction)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_direction);
    return HAL_FUNC_CALL(unit, perif, getGpioDirection, (unit, pin, ptr_direction));
}

/* FUNCTION NAME:   air_perif_setGpioOutputData
 * PURPOSE:
 *      This API is used to set gpio pin output value
 *      to data register
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      data                 -- High or low
 *                              AIR_PERIF_GPIO_DATA_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_setGpioOutputData(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_DATA_T data)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(data, AIR_PERIF_GPIO_DATA_LAST);
    return HAL_FUNC_CALL(unit, perif, setGpioOutputData, (unit, pin, data));
}

/* FUNCTION NAME:   air_perif_getGpioInputData
 * PURPOSE:
 *      This API is used to get gpio pin input value
 *      from data register
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_data             -- Pointer of gpio data
 *                              AIR_PERIF_GPIO_DATA_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_getGpioInputData(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_DATA_T *ptr_data)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_data);
    return HAL_FUNC_CALL(unit, perif, getGpioInputData, (unit, pin, ptr_data));
}

/* FUNCTION NAME:   air_perif_setGpioOutputAutoMode
 * PURPOSE:
 *      This API is used to set auto mode
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      enable               -- TRUE: enable auto mode
 *                              FALSE: disable auto mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_setGpioOutputAutoMode(
    const UI32_T unit,
    const UI32_T pin,
    const BOOL_T enable)
{
    HAL_CHECK_UNIT(unit);
    return HAL_FUNC_CALL(unit, perif, setGpioOutputAutoMode, (unit, pin, enable));
}

/* FUNCTION NAME:   air_perif_getGpioOutputAutoMode
 * PURPOSE:
 *      This API is used to get auto mode status
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_enable           -- TRUE: enable auto mode
 *                              FALSE: disable auto mode
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_getGpioOutputAutoMode(
    const UI32_T unit,
    const UI32_T pin,
    BOOL_T      *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_enable);
    return HAL_FUNC_CALL(unit, perif, getGpioOutputAutoMode, (unit, pin, ptr_enable));
}

/* FUNCTION NAME:   air_perif_setGpioOutputAutoPatt
 * PURPOSE:
 *      This API is used to set gpio auto pattern
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 *      pattern              -- Pattern configuration
 *                              AIR_PERIF_GPIO_PATT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_setGpioOutputAutoPatt(
    const UI32_T                unit,
    const UI32_T                pin,
    const AIR_PERIF_GPIO_PATT_T pattern)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(pattern, AIR_PERIF_GPIO_PATT_LAST);
    return HAL_FUNC_CALL(unit, perif, setGpioOutputAutoPatt, (unit, pin, pattern));
}

/* FUNCTION NAME:   air_perif_getGpioOutputAutoPatt
 * PURPOSE:
 *      This API is used to get gpio auto pattern
 * INPUT:
 *      unit                 -- Device unit number
 *      pin                  -- Pin number
 * OUTPUT:
 *      ptr_pattern          -- Pointer of pattern configuration
 *                              AIR_PERIF_GPIO_PATT_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_perif_getGpioOutputAutoPatt(
    const UI32_T           unit,
    const UI32_T           pin,
    AIR_PERIF_GPIO_PATT_T *ptr_pattern)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_pattern);
    return HAL_FUNC_CALL(unit, perif, getGpioOutputAutoPatt, (unit, pin, ptr_pattern));
}