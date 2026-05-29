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

/* FILE NAME:  hal_cfg.c
 * PURPOSE:
 *    Provide the "customer init value" function and structions.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_types.h>
#include <hal/common/hal.h>

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_HAL, "hal_cfg.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static AIR_CFG_GET_FUNC_T _cfg_getvalue_callback[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:     hal_cfg_getValue
 * PURPOSE:
 *        Get the value of customized configuration.
 * INPUT:
 *        unit              --   device unit number
 *        cfg_type          --   config type
 *        ptr_value         --   config value
 * OUTPUT:
 * RETURN:
 *        AIR_E_OK            --  Operate success.
 *        AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_cfg_getValue(
    const UI32_T         unit,
    const AIR_CFG_TYPE_T cfg_type,
    AIR_CFG_VALUE_T     *ptr_value)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    AIR_CFG_GET_FUNC_T cfg_getvalue_func;
    I32_T              temp_value;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(cfg_type, AIR_CFG_TYPE_LAST);
    HAL_CHECK_PTR(ptr_value);

    temp_value = ptr_value->value;

    cfg_getvalue_func = _cfg_getvalue_callback[unit];
    if (NULL != cfg_getvalue_func)
    {
        rc = cfg_getvalue_func(unit, cfg_type, ptr_value);
    }

    if (AIR_E_OK != rc)
    {
        ptr_value->value = temp_value;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:     hal_cfg_register
 * PURPOSE:
 *        Set the callback function which will be used to get the
 *        value of customized configuration.
 * INPUT:
 *        unit              --   device unit number
 *        ptr_cfg_callback  --   ptr to user cfg callback function
 * OUTPUT:
 *        None
 * RETURN:
 *        AIR_E_OK            --  Operate success.
 *        AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_cfg_register(
    const UI32_T             unit,
    const AIR_CFG_GET_FUNC_T ptr_cfg_callback)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_MIN_MAX_RANGE(unit, 0, AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1);
    HAL_CHECK_PTR(ptr_cfg_callback);

    _cfg_getvalue_callback[unit] = ptr_cfg_callback;

    return rc;
}
