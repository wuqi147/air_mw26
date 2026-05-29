/*******************************************************************************
 *  The software may not be copied and the information
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

/* FILE NAME:  air_init.c
 * PURPOSE:
 *    It provide init module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_init.h>

#include <air_port.h>
#include <aml/aml.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_init.h>
#include <hal/common/hal_poe.h>
#include <osal/osal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_INIT_ARRAY_SIZE(__array__) (sizeof(__array__) / sizeof(__array__[0]))

#define AIR_INIT_FUNC_CALL(__unit__, __module__, __func__)               \
    HAL_FUNC_CALL(__unit__, __module__, __func__, (__unit__, &init_cfg))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
extern AIR_INIT_WRITE_FUNC_T _ext_dsh_write_func;
extern AIR_INIT_WRITE_FUNC_T _ext_debug_write_func;
DIAG_SET_MODULE_INFO(AIR_MODULE_INIT, "air_init.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
const static AIR_MODULE_T _air_init_cmn_module[] = {
    AIR_MODULE_OSAL,
    AIR_MODULE_DIAG,
    AIR_MODULE_AML,
};

const static AIR_MODULE_T _air_init_low_level[] = {
    AIR_MODULE_HAL,
    AIR_MODULE_CHIP,
    AIR_MODULE_PERIF,
};

const static AIR_MODULE_T _air_init_task_rsrc[] = {
    AIR_MODULE_IFMON,
#ifdef AIR_EN_L2_SHADOW
    AIR_MODULE_L2,
#endif /* AIR_EN_L2_SHADOW */
#ifdef AIR_EN_POE
    AIR_MODULE_POE,
#endif
};

const static AIR_MODULE_T _air_init_module[] = {
    AIR_MODULE_SWC,
    /* AIR_MODULE_SIF should be initailized before AIR_MODULE_PORT */
    AIR_MODULE_SIF,
    AIR_MODULE_PORT,
#ifdef AIR_EN_POE
    AIR_MODULE_POE,
#endif
    AIR_MODULE_VLAN,
    AIR_MODULE_MIR,
    AIR_MODULE_STP,
    AIR_MODULE_LAG,
    AIR_MODULE_L2,
    AIR_MODULE_SEC,
    AIR_MODULE_SFLOW,
    AIR_MODULE_DOS,
    AIR_MODULE_ACL,
    AIR_MODULE_QOS,
    AIR_MODULE_STAG,
};

const static AIR_MODULE_T _air_init_task[] = {
    AIR_MODULE_IFMON,
#ifdef AIR_EN_L2_SHADOW
    AIR_MODULE_L2,
#endif /* AIR_EN_L2_SHADOW */
#ifdef AIR_EN_POE
    AIR_MODULE_POE,
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   _air_init_initSingleCmnModule
 * PURPOSE:
 *      This API is used to initialize the specified common module.
 * INPUT:
 *      module              -- The module ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *
 */
static AIR_ERROR_NO_T
_air_init_initSingleCmnModule(
    const AIR_MODULE_T module)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    switch (module)
    {
        case AIR_MODULE_OSAL:
            rc = osal_init();
            break;
        case AIR_MODULE_DIAG:
            rc = diag_init();
            break;
        case AIR_MODULE_AML:
            rc = aml_init();
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    return rc;
}

/* FUNCTION NAME:   _air_init_deinitSingleCmnModule
 * PURPOSE:
 *      This API is used to deinitialize the specified common module.
 * INPUT:
 *      module              -- The module ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *
 */
static AIR_ERROR_NO_T
_air_init_deinitSingleCmnModule(
    const AIR_MODULE_T module)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}

/* FUNCTION NAME:   _air_init_initSingleModule
 * PURPOSE:
 *      This API is used to initialize the specified module.
 * INPUT:
 *      unit                -- The unit ID
 *      module              -- The module ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *
 */
static AIR_ERROR_NO_T
_air_init_initSingleModule(
    const UI32_T       unit,
    const AIR_MODULE_T module)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    HAL_CHIP_INIT_PARAM_T chip_init_param;

    switch (module)
    {
        case AIR_MODULE_HAL:
            rc = hal_init(unit);
            break;
        case AIR_MODULE_CHIP:
            rc = HAL_FUNC_CALL(unit, chip, init, (unit, &chip_init_param));
            break;
        case AIR_MODULE_VLAN:
            rc = HAL_FUNC_CALL(unit, vlan, init, (unit));
            break;
        case AIR_MODULE_MIR:
            rc = HAL_FUNC_CALL(unit, mir, init, (unit));
            break;
        case AIR_MODULE_STP:
            rc = HAL_FUNC_CALL(unit, stp, init, (unit));
            break;
        case AIR_MODULE_PORT:
            rc = HAL_FUNC_CALL(unit, port, init, (unit));
            break;
        case AIR_MODULE_LAG:
            rc = HAL_FUNC_CALL(unit, lag, init, (unit));
            break;
        case AIR_MODULE_L2:
            rc = HAL_FUNC_CALL(unit, l2, init, (unit));
            break;
        case AIR_MODULE_SEC:
            rc = HAL_FUNC_CALL(unit, sec, init, (unit));
            break;
        case AIR_MODULE_SFLOW:
            rc = HAL_FUNC_CALL(unit, sflow, init, (unit));
            break;
        case AIR_MODULE_STAG:
            rc = HAL_FUNC_CALL(unit, stag, init, (unit));
            break;
        case AIR_MODULE_DOS:
            rc = HAL_FUNC_CALL(unit, dos, init, (unit));
            break;
        case AIR_MODULE_ACL:
            rc = HAL_FUNC_CALL(unit, acl, init, (unit));
            break;
        case AIR_MODULE_SWC:
            rc = HAL_FUNC_CALL(unit, swc, init, (unit));
            break;
        case AIR_MODULE_IFMON:
            rc = HAL_FUNC_CALL(unit, ifmon, init, (unit));
            break;
        case AIR_MODULE_QOS:
            rc = HAL_FUNC_CALL(unit, qos, init, (unit));
            break;
        case AIR_MODULE_SIF:
            rc = HAL_FUNC_CALL(unit, sif, init, (unit));
            break;
        case AIR_MODULE_PERIF:
            rc = HAL_FUNC_CALL(unit, perif, init, (unit));
            break;
        case AIR_MODULE_POE:
            rc = hal_poe_init(unit);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "u=%u, init module=%s, rc=%s\n", unit, air_module_getModuleName(module),
                   air_error_getString(rc));
    }

    return rc;
}

/* FUNCTION NAME:   _air_init_deinitSingleModule
 * PURPOSE:
 *      This API is used to deinitialize the specified module.
 * INPUT:
 *      unit                -- The unit ID
 *      module              -- The module ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *
 */
static AIR_ERROR_NO_T
_air_init_deinitSingleModule(
    const UI32_T       unit,
    const AIR_MODULE_T module)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, deinit module=%s\n", unit, air_module_getModuleName(module));

    switch (module)
    {
        case AIR_MODULE_HAL:
            rc = hal_deinit(unit);
            break;
        case AIR_MODULE_CHIP:
            rc = HAL_FUNC_CALL(unit, chip, deinit, (unit));
            break;
        case AIR_MODULE_VLAN:
            rc = HAL_FUNC_CALL(unit, vlan, deinit, (unit));
            break;
        case AIR_MODULE_MIR:
            rc = HAL_FUNC_CALL(unit, mir, deinit, (unit));
            break;
        case AIR_MODULE_STP:
            rc = HAL_FUNC_CALL(unit, stp, deinit, (unit));
            break;
        case AIR_MODULE_LAG:
            rc = HAL_FUNC_CALL(unit, lag, deinit, (unit));
            break;
        case AIR_MODULE_L2:
            rc = HAL_FUNC_CALL(unit, l2, deinit, (unit));
            break;
        case AIR_MODULE_PORT:
            rc = HAL_FUNC_CALL(unit, port, deinit, (unit));
            break;
        case AIR_MODULE_SEC:
            rc = HAL_FUNC_CALL(unit, sec, deinit, (unit));
            break;
        case AIR_MODULE_SFLOW:
            rc = HAL_FUNC_CALL(unit, sflow, deinit, (unit));
            break;
        case AIR_MODULE_STAG:
            rc = HAL_FUNC_CALL(unit, stag, deinit, (unit));
            break;
        case AIR_MODULE_DOS:
            rc = HAL_FUNC_CALL(unit, dos, deinit, (unit));
            break;
        case AIR_MODULE_ACL:
            rc = HAL_FUNC_CALL(unit, acl, deinit, (unit));
            break;
        case AIR_MODULE_SWC:
            rc = HAL_FUNC_CALL(unit, swc, deinit, (unit));
            break;
        case AIR_MODULE_IFMON:
            rc = HAL_FUNC_CALL(unit, ifmon, deinit, (unit));
            break;
        case AIR_MODULE_QOS:
            rc = HAL_FUNC_CALL(unit, qos, deinit, (unit));
            break;
        case AIR_MODULE_SIF:
            rc = HAL_FUNC_CALL(unit, sif, deinit, (unit));
            break;
        case AIR_MODULE_PERIF:
            rc = HAL_FUNC_CALL(unit, perif, deinit, (unit));
            break;
        case AIR_MODULE_POE:
            rc = hal_poe_deinit(unit);
            break;
        default:
            rc = AIR_E_BAD_PARAMETER;
            break;
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_WARN, "u=%u, deinit module=%s, rc=%s\n", unit, air_module_getModuleName(module),
                   air_error_getString(rc));
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   air_init_initCmnModule
 * PURPOSE:
 *      This API is used to initialize the common modules.
 * INPUT:
 *      ptr_init_param          -- The sdk_demo callback functions.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initCmnModule(
    AIR_INIT_PARAM_T *ptr_init_param)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = _air_init_cmn_module;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    /* Hook the diag output function. */
    _ext_dsh_write_func = ptr_init_param->dsh_write_func;
    _ext_debug_write_func = ptr_init_param->debug_write_func;

    /* Init common modules. */
    module_num = AIR_INIT_ARRAY_SIZE(_air_init_cmn_module);

    for (module = 0; module < module_num; module++)
    {
        rc = _air_init_initSingleCmnModule(*ptr_module);

        if (AIR_E_OK == rc)
        {
            ptr_module++;
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            ptr_module++;
            rc = AIR_E_OK;
        }
        else
        {
            break;
        }
    }

    return rc;
}

/* FUNCTION NAME:   air_init_deinitCmnModule
 * PURPOSE:
 *      This API is used to deinitialize the common modules.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitCmnModule(
    void)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = NULL;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    module_num = AIR_INIT_ARRAY_SIZE(_air_init_cmn_module);
    ptr_module = &_air_init_cmn_module[module_num - 1];

    for (module = 0; module < module_num; module++)
    {
        rc = _air_init_deinitSingleCmnModule(*ptr_module);

        if (AIR_E_OK == rc)
        {
            ptr_module--;
        }
        else if (AIR_E_NOT_INITED == rc)
        {
            ptr_module--;
            rc = AIR_E_OK;
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            ptr_module--;
            rc = AIR_E_OK;
        }
        else
        {
            break;
        }
    }

    return rc;
}

/* FUNCTION NAME:   air_init_initLowLevel
 * PURPOSE:
 *      This API is used to initialize the low level modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry is not found.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initLowLevel(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = _air_init_low_level;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (unit >= AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM)
    {
        DIAG_PRINT(HAL_DBG_WARN, "u=%u, invalid unit >= max unit (%u), rc=%d\n", unit, AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM,
                   rc);
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, init low level\n", unit);

    if (AIR_E_OK == rc)
    {
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_low_level);

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_initSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module++;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module++;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init low level %s failed, rc=%d\n", unit, air_module_getModuleName(*ptr_module),
                   rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_deinitLowLevel
 * PURPOSE:
 *      This API is used to deinitialize the low level modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitLowLevel(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = NULL;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, deinit low level\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_LOW_LEVEL;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_low_level);
        ptr_module = &_air_init_low_level[module_num - 1];

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_deinitSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module--;
            }
            else if (AIR_E_NOT_INITED == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }

        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit low level %s failed, rc=%d\n", unit,
                       air_module_getModuleName(*ptr_module), rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit low level failed, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_initTaskRsrc
 * PURPOSE:
 *      This API is used to initialize the task resources of the
 *      modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_ALREADY_INITED    -- Module is reinitialized.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initTaskRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = _air_init_task_rsrc;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, init task rsrc\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_TASK_RSRC;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_task_rsrc);

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_initSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module++;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module++;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init task rsrc %s failed, rc=%d\n", unit, air_module_getModuleName(*ptr_module),
                   rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_deinitTaskRsrc
 * PURPOSE:
 *      This API is used to deinitialize the task resources of the
 *      modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitTaskRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = NULL;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, deinit task rsrc\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_TASK_RSRC;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_task_rsrc);
        ptr_module = &_air_init_task_rsrc[module_num - 1];

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_deinitSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module--;
            }
            else if (AIR_E_NOT_INITED == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }

        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit task rsrc %s failed, rc=%d\n", unit,
                       air_module_getModuleName(*ptr_module), rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit task rsrc failed, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_initModule
 * PURPOSE:
 *      This API is used to initialize the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NO_MEMORY         -- No memory is available.
 *      AIR_E_TABLE_FULL        -- Table is full.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = _air_init_module;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, init module\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_MODULE;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_module);

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_initSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module++;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module++;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init module %s failed, rc=%d\n", unit, air_module_getModuleName(*ptr_module),
                   rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_deinitModule
 * PURPOSE:
 *      This API is used to deinitialize the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = NULL;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, deinit module\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_MODULE;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_module);
        ptr_module = &_air_init_module[module_num - 1];

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_deinitSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module--;
            }
            else if (AIR_E_NOT_INITED == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }

        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit module %s failed, rc=%d\n", unit,
                       air_module_getModuleName(*ptr_module), rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit module failed, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_initTask
 * PURPOSE:
 *      This API is used to initialize the tasks of the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_ALREADY_INITED    -- Module is reinitialized.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initTask(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = _air_init_task;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, init task\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_TASK;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_task);

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_initSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module++;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module++;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, init task %s failed, rc=%d\n", unit, air_module_getModuleName(*ptr_module), rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_deinitTask
 * PURPOSE:
 *      This API is used to deinitialize the tasks of the modules.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_deinitTask(
    const UI32_T unit)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    const AIR_MODULE_T *ptr_module = NULL;
    UI32_T              module = 0;
    UI32_T              module_num = 0;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        rc = AIR_E_NOT_INITED;
    }

    DIAG_PRINT(HAL_DBG_INFO, "u=%u, deinit task\n", unit);

    if (AIR_E_OK == rc)
    {
        HAL_INIT_STAGE(unit) = HAL_INIT_STAGE_TASK;
        module_num = AIR_INIT_ARRAY_SIZE(_air_init_task);
        ptr_module = &_air_init_task[module_num - 1];

        for (module = 0; module < module_num; module++)
        {
            rc = _air_init_deinitSingleModule(unit, *ptr_module);

            if (AIR_E_OK == rc)
            {
                ptr_module--;
            }
            else if (AIR_E_NOT_INITED == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                ptr_module--;
                rc = AIR_E_OK;
            }
            else
            {
                break;
            }
        }

        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit task %s failed, rc=%d\n", unit, air_module_getModuleName(*ptr_module),
                       rc);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, deinit task failed, rc=%d\n", unit, rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_getUnitNum
 * PURPOSE:
 *      This API is used to get the unit numbers.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_num                 -- The unit numbers
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_getUnitNum(
    UI32_T *ptr_num)
{
    return aml_getNumberOfChip(ptr_num);
}

/* FUNCTION NAME:   air_deinit
 * PURPOSE:
 *      This API is used to deinitialize the AIR SDK.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_OTHERS            -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_deinit(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    UI32_T         unit = 0;
    UI32_T         unit_num = 0;

    if (AIR_E_OK == rc)
    {
        rc = air_init_getUnitNum(&unit_num);
    }

    for (unit = 0; unit < unit_num; unit++)
    {
        if ((AIR_E_OK == rc) || (AIR_E_NOT_INITED == rc) || (AIR_E_NOT_SUPPORT == rc))
        {
            rc = air_init_deinitTask(unit);
        }

        if ((AIR_E_OK == rc) || (AIR_E_NOT_INITED == rc) || (AIR_E_NOT_SUPPORT == rc))
        {
            rc = air_init_deinitModule(unit);
        }

        if ((AIR_E_OK == rc) || (AIR_E_NOT_INITED == rc) || (AIR_E_NOT_SUPPORT == rc))
        {
            rc = air_init_deinitTaskRsrc(unit);
        }

        if ((AIR_E_OK == rc) || (AIR_E_NOT_INITED == rc) || (AIR_E_NOT_SUPPORT == rc))
        {
            rc = air_init_deinitLowLevel(unit);
        }
    }

    if ((AIR_E_OK == rc) || (AIR_E_NOT_INITED == rc) || (AIR_E_NOT_SUPPORT == rc))
    {
        rc = air_init_deinitCmnModule();
    }

    return (rc);
}

/* FUNCTION NAME:   air_init_setModuleDebugFlag
 * PURPOSE:
 *      This API is used to set debug flag on each module. Once
 *      module's debug flag has been set, the corresponding debug
 *      messages will be dumpped by debug_write_func.
 * INPUT:
 *      unit                    -- The unit ID
 *      module_id               -- The module ID
 *      dbg_flag                -- The debug flag defined by
 *                                 AIR_INIT_DBG_FLAG_XXX
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      The unit ID is meaningless on this API.
 */
AIR_ERROR_NO_T
air_init_setModuleDebugFlag(
    const UI32_T       unit,
    const AIR_MODULE_T module_id,
    const UI32_T       dbg_flag)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         enable;

    HAL_CHECK_ENUM_RANGE(module_id, AIR_MODULE_LAST);

    /* check if user has invlaid bit flag setting */
    if (0 != ((~(AIR_INIT_DBG_FLAG_ERR | AIR_INIT_DBG_FLAG_WARN | AIR_INIT_DBG_FLAG_INFO)) & dbg_flag))
    {
        return AIR_E_BAD_PARAMETER;
    }

    enable = (dbg_flag & AIR_INIT_DBG_FLAG_ERR) ? TRUE : FALSE;

    rc = diag_setDebugFlag(module_id, HAL_DBG_ERR, enable);

    if (AIR_E_OK == rc)
    {
        enable = (dbg_flag & AIR_INIT_DBG_FLAG_WARN) ? TRUE : FALSE;

        rc = diag_setDebugFlag(module_id, HAL_DBG_WARN, enable);
    }

    if (AIR_E_OK == rc)
    {
        enable = (dbg_flag & AIR_INIT_DBG_FLAG_INFO) ? TRUE : FALSE;

        rc = diag_setDebugFlag(module_id, HAL_DBG_INFO, enable);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_getModuleDebugFlag
 * PURPOSE:
 *      This API is used to get debug flag setting from each module.
 * INPUT:
 *      unit                    -- The unit ID
 *      module_id               -- The module ID
 * OUTPUT:
 *      ptr_dbg_flag            -- The debug flag defined by
 *                                 AIR_INIT_DBG_FLAG_XXX
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      The unit ID is meaningless on this API.
 */
AIR_ERROR_NO_T
air_init_getModuleDebugFlag(
    const UI32_T       unit,
    const AIR_MODULE_T module_id,
    UI32_T            *ptr_dbg_flag)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         flag;

    HAL_CHECK_ENUM_RANGE(module_id, AIR_MODULE_LAST);
    HAL_CHECK_PTR(ptr_dbg_flag);

    rc = diag_getDebugFlag(module_id, &flag);

    if (AIR_E_OK == rc)
    {
        if (flag & HAL_DBG_ERR)
        {
            *ptr_dbg_flag |= AIR_INIT_DBG_FLAG_ERR;
        }
        else
        {
            *ptr_dbg_flag &= (~AIR_INIT_DBG_FLAG_ERR);
        }

        if (flag & HAL_DBG_WARN)
        {
            *ptr_dbg_flag |= AIR_INIT_DBG_FLAG_WARN;
        }
        else
        {
            *ptr_dbg_flag &= (~AIR_INIT_DBG_FLAG_WARN);
        }

        if (flag & HAL_DBG_INFO)
        {
            *ptr_dbg_flag |= AIR_INIT_DBG_FLAG_INFO;
        }
        else
        {
            *ptr_dbg_flag &= (~AIR_INIT_DBG_FLAG_INFO);
        }
    }

    return rc;
}

/* FUNCTION NAME:   air_init_initSdkPortMap
 * PURPOSE:
 *      This API is used to initialize SDK port mapping. All SDK API's
 *      port is based on those ports initialized by this API.
 * INPUT:
 *      unit                    -- The unit ID
 *      port_map_cnt            -- The port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry is not found.
 *      AIR_E_ENTRY_EXISTS      -- Entry already exists.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initSdkPortMap(
    const UI32_T               unit,
    const UI32_T               port_map_cnt,
    const AIR_INIT_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_port_map);

    if (0 != port_map_cnt)
    {
        rc = hal_init_initSdkPortMap(unit, port_map_cnt, ptr_port_map);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid port map cnt=%u, rc=%d\n", unit, port_map_cnt, rc);
    }

    return rc;
}

/* FUNCTION NAME:   air_init_getSdkPortMap
 * PURPOSE:
 *      This API is used to get SDK port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt        -- The pointer of port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_port_map parameter to get the port map.
 */
AIR_ERROR_NO_T
air_init_getSdkPortMap(
    const UI32_T         unit,
    UI32_T              *ptr_port_map_cnt,
    AIR_INIT_PORT_MAP_T *ptr_port_map)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_port_map_cnt);
    HAL_CHECK_PTR(ptr_port_map);

    return hal_init_getSdkPortMap(unit, ptr_port_map_cnt, ptr_port_map);
}

/* FUNCTION NAME:   air_init_initPoePortMap
 * PURPOSE:
 *      This API is used to initialize PoE port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 *      port_map_cnt            -- The port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- SDK port is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initPoePortMap(
    const UI32_T                   unit,
    const UI32_T                   port_map_cnt,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

#ifdef AIR_EN_POE
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_port_map);

    if (0 != port_map_cnt)
    {
        rc = hal_init_initPoePortMap(unit, port_map_cnt, ptr_port_map);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid poe port map cnt=%u, rc=%d\n", unit, port_map_cnt, rc);
    }
#else
    rc = AIR_E_NOT_SUPPORT;
#endif

    return rc;
}

/* FUNCTION NAME:   air_init_getPoePortMap
 * PURPOSE:
 *      This API is used to get PoE port mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt        -- The pointer of port mapping count
 *      ptr_port_map            -- The pointer of port mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_port_map parameter to get the port map.
 */
AIR_ERROR_NO_T
air_init_getPoePortMap(
    const UI32_T             unit,
    UI32_T                  *ptr_port_map_cnt,
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

#ifdef AIR_EN_POE
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_port_map_cnt);
    HAL_CHECK_PTR(ptr_port_map);

    rc = hal_init_getPoePortMap(unit, ptr_port_map_cnt, ptr_port_map);
#else
    rc = AIR_E_NOT_SUPPORT;
#endif

    return rc;
}

/* FUNCTION NAME:   air_init_initPoeDeviceMap
 * PURPOSE:
 *      This API is used to initialize PoE device mapping.
 * INPUT:
 *      unit                    -- The unit ID
 *      device_map_cnt          -- The device mapping count
 *      ptr_device_map          -- The pointer of device mapping
 *                                 information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_init_initPoeDeviceMap(
    const UI32_T                     unit,
    const UI32_T                     device_map_cnt,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

#ifdef AIR_EN_POE
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_device_map);

    if (0 != device_map_cnt)
    {
        rc = hal_init_initPoeDeviceMap(unit, device_map_cnt, ptr_device_map);
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, invalid poe device map cnt=%u, rc=%d\n", unit, device_map_cnt, rc);
    }
#else
    rc = AIR_E_NOT_SUPPORT;
#endif

    return rc;
}

/* FUNCTION NAME:   air_init_getPoeDeviceMap
 * PURPOSE:
 *      This API is used to get PoE device mapping.
 * INPUT:
 *      unit                    -- The unit ID
 * OUTPUT:
 *      ptr_device_map_cnt      -- The pointer of device mapping count
 *      ptr_device_map          -- The pointer of device mapping
 *                                 information
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as
 *      ptr_device_map parameter to get the device map.
 */
AIR_ERROR_NO_T
air_init_getPoeDeviceMap(
    const UI32_T               unit,
    UI32_T                    *ptr_device_map_cnt,
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

#ifdef AIR_EN_POE
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_device_map_cnt);
    HAL_CHECK_PTR(ptr_device_map);

    rc = hal_init_getPoeDeviceMap(unit, ptr_device_map_cnt, ptr_device_map);
#else
    rc = AIR_E_NOT_SUPPORT;
#endif

    return rc;
}
