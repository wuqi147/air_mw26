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

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <osal/osal_mdc.h>
#include <osal/osal_lib.h>
#include "osal/osal.h"
#include "api/diag.h"
#include <cmlib/cmlib_bit.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_OSAL, "osal_mdc.c");

/* NAMING CONSTANT DECLARATIONS
 */
#define OSAL_MDC_DEV_FILE_PATH  "/dev/"OSAL_MDC_DRIVER_NAME

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T dev_num;
    int dev_fd;
} OSAL_MDC_CB_T;

/* GLOBAL VARIABLE DECLARATIONS
 */
static OSAL_MDC_CB_T    _osal_mdc_cb;

/* STATIC VARIABLE DECLARATIONS
 */


/* LOCAL SUBPROGRAM DECLARATIONS
 */
static AIR_ERROR_NO_T
_osal_mdc_ioctl(
    const UI32_T unit,
    const OSAL_MDC_IOCTL_ACCESS_T access,
    const OSAL_MDC_IOCTL_TYPE_T type,
    const UI32_T data_size,
    void *ptr_data);

static AIR_ERROR_NO_T
_osal_mdc_attachAccessCallback(
    const UI32_T dev_num,
    AML_DEV_T *ptr_dev_list);

/* EXPORTED SUBPROGRAM BODIES
 */

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
    const UI32_T        unit,
    const UI32_T        offset,
    UI32_T              *ptr_data,
    const UI32_T        len)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    OSAL_MDC_IOCTL_I2C_DATA_S i2c;

    i2c.dev_num = 0;
    i2c.addr = offset;
    i2c.data = 0;

    rc = _osal_mdc_ioctl(unit, OSAL_MDC_IOCTL_ACCESS_READ,
                         OSAL_MDC_IOCTL_TYPE_REG_READ,
                         sizeof(OSAL_MDC_IOCTL_I2C_DATA_S), (void*)&i2c);
    if(AIR_E_OK == rc)
    {
        *ptr_data = i2c.data;
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "[%s] ioctl read reg fail.\n", __func__);
    }

    DIAG_PRINT(HAL_DBG_INFO, "ReadPbusReg  <0x%08x>:0x%08x \r\n", offset, *ptr_data);

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
    const UI32_T        unit,
    const UI32_T        offset,
    const UI32_T        *ptr_data,
    const UI32_T        len)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    DIAG_PRINT(HAL_DBG_INFO, "WritePbusReg [0x%08x]=0x%08x \r\n", offset, *ptr_data);

    OSAL_MDC_IOCTL_I2C_DATA_S i2c;

    i2c.dev_num = 0;
    i2c.addr = offset;
    i2c.data = *ptr_data;

    rc = _osal_mdc_ioctl(unit, OSAL_MDC_IOCTL_ACCESS_WRITE,
            OSAL_MDC_IOCTL_TYPE_REG_WRITE,
            sizeof(OSAL_MDC_IOCTL_I2C_DATA_S), (void*)&i2c);
    if(AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[%s] ioctl read reg fail.\n", __func__);
    }

    return (rc);
}


/* FUNCTION NAME:   _osal_mdc_ioctl
 * PURPOSE:
 *      osal mdc io control interface.
 *
 * INPUT:
 *      unit            -- The device unit
 *      access          -- Access direction of io control
 *      type            -- Type of io control
 *      data_size       -- Size of data
 *      ptr_data        -- A pointer of data
 *
 * OUTPUT:
 *      ptr_data        -- A pointer of data
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_osal_mdc_ioctl(
    const UI32_T unit,
    const OSAL_MDC_IOCTL_ACCESS_T access,
    const OSAL_MDC_IOCTL_TYPE_T type,
    const UI32_T data_size,
    void *ptr_data)
{
    OSAL_MDC_CB_T *ptr_cb = &_osal_mdc_cb;
    OSAL_MDC_IOCTL_CMD_T cmd;
    int linux_rc;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    cmd.value = 0x0;
    cmd.field.access  = access;
    cmd.field.type    = type;
    cmd.field.unit    = unit;
    cmd.field.size    = data_size;

    if (ptr_cb->dev_fd > 0)
    {
        linux_rc = ioctl(ptr_cb->dev_fd, (long unsigned int)cmd.value, ptr_data);
        if (-1 == linux_rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "do ioctl failed, type=%d, errno=%d", type, errno);
            rc = AIR_E_OTHERS;
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "Error: Open dev %s first\n", OSAL_MDC_DEV_FILE_PATH);
        rc = AIR_E_OP_INVALID;
    }

    return (rc);
}

/* FUNCTION NAME:   _osal_mdc_attachAccessCallback
 * PURPOSE:
 *      Hook device call back function
 *
 * INPUT:
 *      dev_num         -- Total device number
 *      ptr_dev_list    -- A pointer of device list
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_osal_mdc_attachAccessCallback(
    const UI32_T dev_num,
    AML_DEV_T *ptr_dev_list)
{
    UI32_T idx;

    for (idx = 0; idx < dev_num; idx++)
    {
        ptr_dev_list[idx].access.read_callback  = osal_mdc_readPbusReg;
        ptr_dev_list[idx].access.write_callback = osal_mdc_writePbusReg;
    }

    return (AIR_E_OK);
}


/* --------------------------------------------------------------------------- MDC */

AIR_ERROR_NO_T
osal_mdc_initDevice(
    AML_DEV_T           *ptr_dev_list,
    UI32_T              *ptr_dev_num)
{
    OSAL_MDC_CB_T *ptr_cb = &_osal_mdc_cb;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    OSAL_MDC_IOCTL_DEV_DATA_T ioctl_data;
    UI32_T unit = 0, idx = 0;

     /* Create and open the I/O device. */
    if (0 != mknod(OSAL_MDC_DEV_FILE_PATH,
                   S_IFCHR,
                   makedev(OSAL_MDC_DRIVER_MISC_MAJOR_NUM, OSAL_MDC_DRIVER_MISC_MINOR_NUM)))
    {
        if (EEXIST == errno)
        {
            /* the mknod API may return fail if the OSAL_MDC_DEV_FILE_PATH is created in rootfs by default */
        }
        else
        {
            DIAG_PRINT(HAL_DBG_ERR,
                       "mknod failed, path=%s, major num=%d, minor num=%d, errno=%d\n",
                       OSAL_MDC_DEV_FILE_PATH, OSAL_MDC_DRIVER_MISC_MAJOR_NUM,
                       OSAL_MDC_DRIVER_MISC_MINOR_NUM, errno);
            return (AIR_E_OTHERS);
        }
    }

    osal_memset(ptr_cb, 0x0, sizeof(OSAL_MDC_CB_T));

    ptr_cb->dev_fd = open(OSAL_MDC_DEV_FILE_PATH, O_RDWR | O_SYNC );
    if (ptr_cb->dev_fd > 0)
    {
        /* Init the interface. */
        rc = _osal_mdc_ioctl(unit, OSAL_MDC_IOCTL_ACCESS_READ,
                             OSAL_MDC_IOCTL_TYPE_INIT_DEV,
                             sizeof(OSAL_MDC_IOCTL_DEV_DATA_T),
                             (void *)&ioctl_data);
        if (AIR_E_OK == rc)
        {
            *ptr_dev_num = ioctl_data.dev_num;
            ptr_cb->dev_num = ioctl_data.dev_num;

            /* Update the database of AML. */
            for(idx = 0; idx < ioctl_data.dev_num; idx ++)
            {
                ptr_dev_list[idx].if_type = ioctl_data.if_type;
                ptr_dev_list[idx].id.family = ioctl_data.id[idx].family;
                ptr_dev_list[idx].id.vendor = ioctl_data.id[idx].vendor;
                ptr_dev_list[idx].id.revision = ioctl_data.id[idx].revision;
            }

            /* Hook callback functions. */
            _osal_mdc_attachAccessCallback(*ptr_dev_num, ptr_dev_list);
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_ERR, "open dev %s failed\n", OSAL_MDC_DEV_FILE_PATH);
        rc = AIR_E_OP_INVALID;
    }

    return (rc);
}

