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

/* FILE NAME:   air_cfg.c
 * PURPOSE:
 *      Customer configuration on AIR SDK.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>

#include <air_types.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>

/* GLOBAL VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_cfg_getValue
 * PURPOSE:
 *      This API is used to get the value of customized configuration.
 * INPUT:
 *      unit                 -- Device ID
 *      cfg_type             -- The type of config
 *                              AIR_CFG_TYPE_T
 * OUTPUT:
 *      ptr_value            -- The value of config
 *                              AIR_CFG_VALUE_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_cfg_getValue(
    const UI32_T         unit,
    const AIR_CFG_TYPE_T cfg_type,
    AIR_CFG_VALUE_T     *ptr_value)
{
    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(cfg_type, AIR_CFG_TYPE_LAST);
    HAL_CHECK_PTR(ptr_value);

    return hal_cfg_getValue(unit, cfg_type, ptr_value);
}

/* FUNCTION NAME:   air_cfg_register
 * PURPOSE:
 *      The function is to register user configuration callback to SDK.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      ptr_cfg_callback     -- Pointer to user configuration callback
 *                              AIR_CFG_GET_FUNC_T
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *
 * NOTES:
 *      1. During SDK initializtion, it will call registered user
 *         configuration callback to get configuration and apply them.
 *         If there is no registered user configuration callback or
 *         can not get specified user configuration callback, SDK will
 *         apply default setting.
 *      2. This function should be called before calling air_init
 */
AIR_ERROR_NO_T
air_cfg_register(
    const UI32_T       unit,
    AIR_CFG_GET_FUNC_T ptr_cfg_callback)
{
    /*parameter check*/
    HAL_CHECK_PTR(ptr_cfg_callback);

    /* body */
    return hal_cfg_register(unit, ptr_cfg_callback);
}
