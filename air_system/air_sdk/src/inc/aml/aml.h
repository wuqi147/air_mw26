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

/* FILE NAME:  aml.h
 * PURPOSE:
 *  1. Provide whole AML resource initialization API.
 *  2. Provide configuration access APIs.
 *  3. Provide ISR registration and deregistration APIs.
 *  4. Provide memory access.
 *  5. Provide DMA management APIs.
 *  6. Provide address translation APIs.
 * NOTES:
 */

#ifndef AML_H
#define AML_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_cfg.h>
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* #define AML_EN_CUSTOM_DMA_ADDR */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    AML_DEV_TYPE_PBUS,
    AML_DEV_TYPE_I2C,
    AML_DEV_TYPE_LAST

} AML_HW_IF_T;

typedef AIR_ERROR_NO_T (*AML_DEV_READ_FUNC_T)(
    const UI32_T unit,
    const UI32_T addr_offset,
    UI32_T      *ptr_data,
    const UI32_T len);

typedef AIR_ERROR_NO_T (*AML_DEV_WRITE_FUNC_T)(
    const UI32_T  unit,
    const UI32_T  addr_offset,
    const UI32_T *ptr_data,
    const UI32_T  len);

typedef AIR_ERROR_NO_T (*AML_DEV_ISR_FUNC_T)(
    void *ptr_data);

/* To mask the chip interrupt in kernel interrupt routine. */
typedef struct
{
    UI32_T mask_addr;
    UI32_T mask_val;
} AML_DEV_ISR_DATA_T;

/* To read or write the HW-intf registers. */
typedef struct
{
    AML_DEV_READ_FUNC_T  read_callback;
    AML_DEV_WRITE_FUNC_T write_callback;
} AML_DEV_ACCESS_T;

typedef struct
{
    UI32_T vendor;
    UI32_T family;
    UI32_T revision;
} AML_DEV_ID_T;

typedef struct
{
    AML_HW_IF_T      if_type;
    AML_DEV_ID_T     id;
    AML_DEV_ACCESS_T access;
} AML_DEV_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    UI32_T      *ptr_mode);

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
aml_init(void);

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
aml_deinit(void);

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
    UI32_T *ptr_num);

/* FUNCTION NAME:   aml_connectIsr
 * PURPOSE:
 *      To enable the system intterupt and specify the ISR handler.
 * INPUT:
 *      unit        -- the device unit
 *      handler     -- the ISR hanlder
 *      ptr_cookie  -- pointer for the data as an argument of the handler
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK     -- Successfully connect the ISR handler to the system.
 *      AIR_E_OTHERS -- Failed to connect the ISR handler to the system.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_connectIsr(
    const UI32_T        unit,
    AML_DEV_ISR_FUNC_T  handler,
    AML_DEV_ISR_DATA_T *ptr_cookie);

/* FUNCTION NAME:   aml_disconnectIsr
 * PURPOSE:
 *      To disable the system intterupt notification.
 * INPUT:
 *      unit        -- the device unit
 * OUTPUT:
 *      none
 * RETURN:
 *      AIR_E_OK     -- Successfully disconnect the ISR handler to the system.
 *      AIR_E_OTHERS -- Failed to disconnect the ISR handler to the system.
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
aml_disconnectIsr(
    const UI32_T unit);

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
    UI32_T      *ptr_revision_id);

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
    const UI32_T len);

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
    const UI32_T  len);
#endif /* #ifndef AML_H */
