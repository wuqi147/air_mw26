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

/* FILE NAME:  osal_mdc.c
 * PURPOSE:
 * 1. Provide device operate from AML interface
 * NOTES:
 *
 */
#include <osal/osal_mdc.h>

#include <osal/osal_lib.h>
/* DATA TYPE DECLARATIONS
 */
/* SCU Register Base */
#define REG_SCU_BASE_ADDRESS (0x10005000)
#define FAMILY_ID_ADDR       (REG_SCU_BASE_ADDRESS)
#define REVISION_ID_ADDR     (REG_SCU_BASE_ADDRESS + 0x04)

typedef struct
{
    UI32_T             unit;
    struct pci_dev    *ptr_pci_dev;
    UI32_T            *ptr_mmio_virt_addr;
    int                irq;
    AML_DEV_ISR_FUNC_T isr_callback;
    void              *ptr_isr_data;

} OSAL_MDC_DEV_T;

typedef struct
{
    OSAL_MDC_DEV_T dev[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
    UI32_T         dev_num;

} OSAL_MDC_CB_T;

/* re-define the interface to align OSAL_MDC's implementation with the prototype of CMLIB */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static OSAL_MDC_CB_T _osal_mdc_cb;

/* To let system callback function to access AML database */
static AML_DEV_T    *_ptr_osal_mdc_dev;

/* EXTERN FUNCTION
 */
extern unsigned int
io_read32(unsigned int addr);
extern void
io_write32(unsigned int addr, unsigned int vlaue);

/* --------------------------------------------------------------------------- Interrupt */

/* --------------------------------------------------------------------------- Driver */

/* FUNCTION NAME:   osal_mdc_readPbusReg
 * PURPOSE:
 *      To read data from the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      offset      -- the address of register
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
osal_mdc_readPbusReg(
    const UI32_T unit,
    const UI32_T offset,
    UI32_T      *ptr_data,
    const UI32_T len)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         idx;
    UI32_T         count;

    if (OSAL_MDC_PBUS_WIDTH == len)
    {
        *ptr_data = io_read32(offset);
    }
    else
    {
        if (0 == (len % OSAL_MDC_PBUS_WIDTH))
        {
            count = len / OSAL_MDC_PBUS_WIDTH;
            for (idx = 0; idx < count; idx++)
            {
                *(ptr_data + idx) = io_read32(offset + (idx * 4));
            }
        }
        else
        {
            rc = AIR_E_OTHERS;
        }
    }

    return (rc);
}

/* FUNCTION NAME:   osal_mdc_writePbusReg
 * PURPOSE:
 *      To write data to the register of the specified chip unit.
 * INPUT:
 *      unit        -- the device unit
 *      offset -- the address of register
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
osal_mdc_writePbusReg(
    const UI32_T  unit,
    const UI32_T  offset,
    const UI32_T *ptr_data,
    const UI32_T  len)
{
    UI32_T         idx;
    UI32_T         count;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (OSAL_MDC_PBUS_WIDTH == len)
    {
        io_write32(offset, *ptr_data);
    }
    else
    {
        if (0 == (len % OSAL_MDC_PBUS_WIDTH))
        {
            count = len / OSAL_MDC_PBUS_WIDTH;
            for (idx = 0; idx < count; idx++)
            {
                io_write32(offset + (idx * 4), *(ptr_data + idx));
            }
        }
        else
        {
            rc = AIR_E_OTHERS;
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_osal_mdc_probe(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         family_data = 0, revision_data = 0;

    /* Current only allow 1 unit
     * To-do: handle multiple chips */

    rc = osal_mdc_readPbusReg(unit, FAMILY_ID_ADDR, &family_data, OSAL_MDC_PBUS_WIDTH);
    if (AIR_E_OK != rc)
    {
        osal_printf("FAMILY ID read failed \n");
        return rc;
    }

    rc = osal_mdc_readPbusReg(unit, REVISION_ID_ADDR, &revision_data, OSAL_MDC_PBUS_WIDTH);
    if (AIR_E_OK != rc)
    {
        osal_printf("REVISION ID read failed \n");
        return rc;
    }

    _ptr_osal_mdc_dev->if_type = AML_DEV_TYPE_PBUS;
    _ptr_osal_mdc_dev->id.vendor = 0;
    _ptr_osal_mdc_dev->id.family = family_data;
    _ptr_osal_mdc_dev->id.revision = revision_data;
    _ptr_osal_mdc_dev->access.read_callback = osal_mdc_readPbusReg;
    _ptr_osal_mdc_dev->access.write_callback = osal_mdc_writePbusReg;

    /* increment device_number */
    _osal_mdc_cb.dev_num++;
    _ptr_osal_mdc_dev++;

    return rc;
}

/* --------------------------------------------------------------------------- MDC */

AIR_ERROR_NO_T
osal_mdc_initDevice(
    AML_DEV_T *ptr_dev_list,
    UI32_T    *ptr_dev_num)
{
    OSAL_MDC_CB_T *ptr_cb = &_osal_mdc_cb;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    _ptr_osal_mdc_dev = ptr_dev_list;
    osal_memset(ptr_cb, 0x0, sizeof(OSAL_MDC_CB_T));

    /* register device */
#if defined(AIR_FREERTOS)

    /* unit:0 */
    rc = _osal_mdc_probe();
#endif

    /* return dev_num */
    *ptr_dev_num = ptr_cb->dev_num;

    return (rc);
}
