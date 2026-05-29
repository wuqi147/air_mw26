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

/* FILE NAME:  hal_drv.c
 * PURPOSE:
 *  Provide HAL driver structure and driver help APIs.
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal_drv.h>

#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_dev.h>

#if defined(AIR_EN_SCORPIO)
#include <hal/switch/sco/hal_sco_drv.h>
#endif
#if defined(AIR_EN_PEARL)
#include <hal/switch/pearl/hal_pearl_drv.h>
#endif
#if defined(AIR_EN_CORAL)
#include <hal/switch/coral/hal_coral_drv.h>
#endif

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_DEVICE_DRIVER_MAP_VECTOR_ENTRY(i) (&_hal_device_to_driver_func_vector[i])

#define HAL_DEVICE_DRIVER_MAP_CNT                                                              \
    (sizeof(_hal_device_to_driver_func_vector) / sizeof(_hal_device_to_driver_func_vector[0]))

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_HAL, "hal_drv.c");

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static HAL_DEVICE_DRIVER_MAP_T _hal_device_to_driver_func_vector[] = {
/* each element is a FAMILY-ID and DRIVER-INIT-FUNC mapping entry */
#if defined(AIR_EN_SCORPIO)
    {  HAL_CHIP_FAMILY_ID_SCO,   hal_sco_drv_initDriver},
#endif
#if defined(AIR_EN_PEARL)
    {HAL_CHIP_FAMILY_ID_PEARL, hal_pearl_drv_initDriver},
#endif
#if defined(AIR_EN_CORAL)
    {HAL_CHIP_FAMILY_ID_CORAL, hal_coral_drv_initDriver}
#endif
};

// extern UI32_T _ext_hal_ari_debug_flag;

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_drv_initDeviceDriver
 * PURPOSE:
 *      hal_drv_initDeviceDriver() is an API that will use device ID and
 *      revision ID to find correct device driver.
 *
 * INPUT:
 *      device_id       -- The device ID of this device.
 *      revision_id     -- The revision ID of this device.
 * OUTPUT:
 *      pptr_hal_driver -- The pointer (address) of the HAL device driver
 *                         that used for this device with the device_id and
 *                         revision_id.
 * RETURN:
 *      AIR_E_OK        -- Initialize the device driver successfully.
 *      AIR_E_OTHERS    -- Fail to initialize the device driver.
 *
 * NOTES:
 *      None
 *
 */
AIR_ERROR_NO_T
hal_drv_initDeviceDriver(
    const UI32_T   family_id,
    const UI32_T   revision_id,
    HAL_DRIVER_T **pptr_hal_driver)
{
    HAL_DEVICE_DRIVER_MAP_T *ptr_entry = NULL;
    UI16_T                   i;

    /* use device_id to find this device's init driver function */
    for (i = 0; i < HAL_DEVICE_DRIVER_MAP_CNT; i++)
    {
        ptr_entry = HAL_DEVICE_DRIVER_MAP_VECTOR_ENTRY(i);

        if (family_id == ptr_entry->family_id)
        {
            return ptr_entry->hal_initDriver(revision_id, pptr_hal_driver);
        }
    }

    DIAG_PRINT(HAL_DBG_ERR, "hook family id=0x%x rev=0x%x driver failed\n", family_id, revision_id);

    return AIR_E_ENTRY_NOT_FOUND;
}
