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

/* FILE NAME:  aml.c
 * PURPOSE:
 *  1. Provide whole AML resource initialization API.
 *  2. Provide configuration access APIs.
 *  3. Provide ISR registration and deregistration APIs.
 *  4. Provide memory access.
 *  5. Provide DMA management APIs.
 *  6. Provide address translation APIs.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <aml/aml.h>

#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <osal/osal_mdc.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define IAC_MAX_BUSY_TIME (1000)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    AML_DEV_T dev[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
    UI32_T    dev_num;
} AML_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_AML, "aml.c");
UI32_T          _ext_aml_run_mode = HAL_RUN_CHIP_MODE;
UI32_T          _ext_aml_forced_family_id = 0x0;
UI32_T          _ext_aml_forced_revision_id = 0x0;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static AML_CB_T _aml_cb;

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_aml_deinitRsrc(
    void)
{
    return (AIR_E_OK);
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   aml_getRunMode
 * PURPOSE:
 *      To get current SDK running mode.
 * INPUT:
 *      unit        -- the device unit
 * OUTPUT:
 *      ptr_mode    -- current running mode
 * RETURN:
 *      AIR_E_OK    -- Successfully get the running mode.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_getRunMode(
    const UI32_T unit,
    UI32_T      *ptr_mode)
{
    (*ptr_mode) = _ext_aml_run_mode;
    return (AIR_E_OK);
}

/* FUNCTION NAME:   aml_deinit
 * PURPOSE:
 *      To de-initialize the DMA memory and interface-related kernel source.
 * INPUT:
 *      none
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK    -- Successfully de-initialize AML module.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
aml_deinit(
    void)
{
    _aml_deinitRsrc();
    return (AIR_E_OK);
}

/* FUNCTION NAME:   aml_getNumberOfChip
 * PURPOSE:
 *      To get the number of chips connected to host CPU.
 * INPUT:
 *      none
 * OUTPUT:
 *      ptr_num     -- pointer for the chip number
 * RETURN:
 *      AIR_E_OK    -- Successfully get the number of chips.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_getNumberOfChip(
    UI32_T *ptr_num)
{
    HAL_CHECK_PTR(ptr_num);

    *ptr_num = _aml_cb.dev_num;
    return (AIR_E_OK);
}

/* FUNCTION NAME:   aml_getDeviceInfo
 * PURPOSE:
 *      To get the family/revision ID of the specified chip unit.
 * INPUT:
 *      unit            -- the device unit
 * OUTPUT:
 *      ptr_family_id   -- pointer for the family ID
 *      ptr_revision_id -- pointer for the revision ID
 * RETURN:
 *      AIR_E_OK            -- Successfully get the IDs.
 *      AIR_E_BAD_PARAMETER -- Invalid input parameter.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_getDeviceInfo(
    const UI32_T unit,
    UI32_T      *ptr_family_id,
    UI32_T      *ptr_revision_id)
{
    /* Cannot check unit here, since HAL isn't initialized completely. */
    HAL_CHECK_PTR(ptr_family_id);
    HAL_CHECK_PTR(ptr_revision_id);

    *ptr_family_id = _aml_cb.dev[unit].id.family;
    *ptr_revision_id = _aml_cb.dev[unit].id.revision;
    return (AIR_E_OK);
}

/* FUNCTION NAME:   aml_readReg
 * PURPOSE:
 *      To read data from the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      addr_offset -- the address of register
 *      len         -- data size read
 * OUTPUT:
 *      ptr_data    -- pointer for the register data
 * RETURN:
 *      AIR_E_OK     -- Successfully read the data.
 *      AIR_E_OTHERS -- Failed to read the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_readReg(
    const UI32_T unit,
    const UI32_T addr_offset,
    UI32_T      *ptr_data,
    const UI32_T len)
{
    AIR_ERROR_NO_T rv;
    HAL_CHECK_PTR(ptr_data);

    rv = _aml_cb.dev[unit].access.read_callback(unit, addr_offset, ptr_data, len);
    if (AIR_E_OK == rv)
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) Read Reg[0x%X] : 0x%08X\n", unit, addr_offset, *ptr_data);
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, " (%u) rv=(%d)\n", unit, rv);
    }
    return rv;
}

/* FUNCTION NAME:   aml_writeReg
 * PURPOSE:
 *      To write data to the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      addr_offset -- the address of register
 *      ptr_data    -- pointer for the written data
 *      len         -- data size read
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK     -- Successfully write the data.
 *      AIR_E_OTHERS -- Failed to write the data.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_writeReg(
    const UI32_T  unit,
    const UI32_T  addr_offset,
    const UI32_T *ptr_data,
    const UI32_T  len)
{
    HAL_CHECK_PTR(ptr_data);

    DIAG_PRINT(HAL_DBG_INFO, "(%u)Write Reg[0x%X] = 0x%08X\n", unit, addr_offset, *ptr_data);
    return _aml_cb.dev[unit].access.write_callback(unit, addr_offset, ptr_data, len);
}

/* FUNCTION NAME:   _aml_initRsrc
 * PURPOSE:
 *      To initialize the control block for the probed device.
 * INPUT:
 *      none
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK        -- Successfully initialize AML control block.
 *      AIR_E_OTHERS    -- Failed to initialize AML control block.
 * NOTES:
 *      none
 */
static AIR_ERROR_NO_T
_aml_initRsrc(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit;

    osal_memset(&_aml_cb, 0x0, sizeof(AML_CB_T));
    rc = osal_mdc_initDevice(_aml_cb.dev, &_aml_cb.dev_num);
    if (AIR_E_OK == rc)
    {
        if (0 != _aml_cb.dev_num)
        {
            DIAG_PRINT(HAL_DBG_INFO, "probe device num=%d\n", _aml_cb.dev_num);
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR, "probe device failed\n");
            rc = AIR_E_OTHERS;
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "probe device failed\n");
    }

    if (AIR_E_OK == rc)
    {
        /* Force to assign the device ID. */
        if (0x0 != _ext_aml_forced_family_id)
        {
            for (unit = 0; unit < _aml_cb.dev_num; unit++)
            {
                _aml_cb.dev[unit].id.family = _ext_aml_forced_family_id;
            }
        }

        /* Force to assign the revision ID. */
        if (0x0 != _ext_aml_forced_revision_id)
        {
            for (unit = 0; unit < _aml_cb.dev_num; unit++)
            {
                _aml_cb.dev[unit].id.revision = _ext_aml_forced_revision_id;
            }
        }
    }

    return (rc);
}

/* FUNCTION NAME:   aml_init
 * PURPOSE:
 *      To initialize the DMA memory and interface-related kernel source.
 * INPUT:
 *      none
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK        -- Successfully initialize AML module.
 *      AIR_E_OTHERS    -- Failed to initialize AML module.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_init(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    rc = _aml_initRsrc();
    if (AIR_E_OK != rc)
    {
        _aml_deinitRsrc();
    }
    return (rc);
}
