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

#if defined(AIR_LINUX_USER_MODE) || defined(AIR_LINUX_KERNEL_MODE)
#include <linux/miscdevice.h>
#include <linux/version.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/spinlock_types.h>
#include <linux/bitmap.h>
#include <linux/list.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#endif

#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/errno.h>

static struct i2c_client *osal_i2c_client = NULL;
#ifdef __KERNEL__
#undef pr_fmt
#define pr_fmt(fmt) "[%s] : " fmt, __func__
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define OSAL_MDC_ERR                        (printk)
#define REG_SCU_BASE_ADDRESS                (0x10005000)
#define FAMILY_ID_ADDR                      (REG_SCU_BASE_ADDRESS)
#define REVISION_ID_ADDR                    (REG_SCU_BASE_ADDRESS + 0x04)

/* DATA TYPE DECLARATIONS
 */
#if defined(AIR_LINUX_USER_MODE)
typedef struct OSAL_MDC_IOCTL_CB_S
{
    OSAL_MDC_IOCTL_CALLBACK_FUNC_T callback[OSAL_MDC_IOCTL_TYPE_LAST];
}OSAL_MDC_IOCTL_CB_T;

static DEFINE_SPINLOCK(air_lock);

#endif /* AIR_LINUX_USER_MODE */

/* EXTERN FUNCTION
*/

/* GLOBAL VARIABLE DECLARATIONS
 */

#if defined(AIR_LINUX_USER_MODE)
static OSAL_MDC_IOCTL_CB_T _osal_mdc_ioctl_cb;
static UI32_T _osal_mdc_devInited = 0;
static AML_DEV_T _osal_mdc_ioctl_dev[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM] = {};

#endif /* AIR_LINUX_USER_MODE */


/* STATIC VARIABLE DECLARATIONS
 */

/* --------------------------------------------------------------------------- Interrupt */

/* --------------------------------------------------------------------------- Driver */
static AIR_ERROR_NO_T
_osal_mdc_reverseAddr(
    const UI32_T unit,
    UI32_T       *ptr_addr)
{
    *ptr_addr = (*ptr_addr & 0xFFFF0000) >> 16 | (*ptr_addr & 0x0000FFFF) << 16;
    *ptr_addr = (*ptr_addr & 0xFF00FF00) >> 8  | (*ptr_addr & 0x00FF00FF) << 8;

    return AIR_E_OK;
}

/* FUNCTION NAME:   osal_mdc_readI2cReg
 * PURPOSE:
 *      To read data from the register of the specified chip unit.
 *
 * INPUT:
 *      unit            -- The device unit
 *      addr            -- The address of register
 *      len             -- Data size read
 *
 * OUTPUT:
 *      ptr_data        -- A pointer for the register data
 *
 * RETURN:
 *      AIR_E_OK        -- Successfully read the data.
 *      AIR_E_OTHERS    -- Failed to read the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_mdc_readI2cReg(
    const UI32_T unit,
    const UI32_T addr,
    UI32_T *ptr_data,
    const UI32_T len)
{
    unsigned long flags;
    unsigned int address = addr;
    int ret;
    struct i2c_msg msg[2] =
    {
        {
            .addr = osal_i2c_client->addr,
            .flags = 0, /* write operation */
            .len = 4,
            .buf = (UI8_T *) &address,
        },
        {
            .addr = osal_i2c_client->addr,
            .flags = I2C_M_RD, /* read operation */
            .len = 4,
            .buf = (UI8_T *) ptr_data,
        }
    };
    _osal_mdc_reverseAddr(unit, &address);

    spin_lock_irqsave(&air_lock, flags);
    ret = i2c_transfer(osal_i2c_client->adapter, msg, 2);
    spin_unlock_irqrestore(&air_lock, flags);

    if(ret < 0)
    {
        pr_err("I2C read error, address = 0x%x, ret = %d\n", addr, ret);
        return ret;
    }

    return (0);
}

/* FUNCTION NAME:   osal_mdc_writeI2cReg
 * PURPOSE:
 *      To write data to the register of the specified chip unit.
 *
 * INPUT:
 *      unit            -- The device unit
 *      addr            -- The address of register
 *      ptr_data        -- The written data
 *      len        -- Data size read
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK        -- Successfully write the data.
 *      AIR_E_OTHERS    -- Failed to write the data.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
osal_mdc_writeI2cReg(
    const UI32_T unit,
    const UI32_T addr,
    const UI32_T *ptr_data,
    const UI32_T len)
{
    int ret;
    unsigned int address = addr;
    unsigned long flags;
    unsigned char buf[8];
    int i;
    struct i2c_msg msg;

    _osal_mdc_reverseAddr(unit, &address);

    for(i = 0; i < 4; i++)
    {
        buf[i] = (unsigned char)(address >> (i * 8)) & 0xff;
    }
    for(i = 0; i < 4; i++)
    {
        buf[i + 4] = (unsigned char)(*ptr_data >> (i * 8)) & 0xff;
    }

    msg.addr = osal_i2c_client->addr;
    msg.flags = 0; /* write operation */
    msg.len = 8;
    msg.buf = buf;

    spin_lock_irqsave(&air_lock, flags);
    ret = i2c_transfer(osal_i2c_client->adapter, &msg, 1);
    spin_unlock_irqrestore(&air_lock, flags);

    if(ret < 0)
    {
        pr_err("I2C write error, address = 0x%x, data = 0x%x\n", addr, *ptr_data);
        return ret;
    }
    return 0;
}

__attribute__((unused)) AIR_ERROR_NO_T
osal_mdc_initDevice(
    AML_DEV_T *ptr_dev_list,
    UI32_T *ptr_dev_num)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, family_data = 0, revision_data = 0;

    /* I2C interface will be probed in BSP. */
    ptr_dev_list->if_type = AML_DEV_TYPE_I2C;

    /* in kernel space no body use this callback, [redundant] */
    ptr_dev_list->access.read_callback = osal_mdc_readI2cReg;
    ptr_dev_list->access.write_callback = osal_mdc_writeI2cReg;

    rc = osal_mdc_readI2cReg(unit, REVISION_ID_ADDR, &revision_data, sizeof(revision_data));
    if (AIR_E_OK != rc)
    {
        OSAL_MDC_ERR(KERN_ERR "REVISION ID read failed \n");
        return rc;
    }

    rc = osal_mdc_readI2cReg(unit, FAMILY_ID_ADDR, &family_data, sizeof(family_data));
    if (AIR_E_OK != rc)
    {
        OSAL_MDC_ERR(KERN_ERR "FAMILY ID read failed \n");
        return rc;
    }

    OSAL_MDC_ERR(KERN_ERR "FAMILY ID=%x REVISION ID=%x \n", family_data, revision_data);
    ptr_dev_list->id.family = family_data;
    ptr_dev_list->id.vendor = 0;
    ptr_dev_list->id.revision = revision_data;
    *ptr_dev_num = 1;

    return (rc);
}

__attribute__((unused)) AIR_ERROR_NO_T
osal_mdc_deinitDevice(
    void)
{
    return AIR_E_OK;
}
/* --------------------------------------------------------------------------- MDC */

static AIR_ERROR_NO_T
_osal_mdc_ioctl_RegReadCallback(
    const UI32_T unit,
    void *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    OSAL_MDC_IOCTL_I2C_DATA_S *i2c = (OSAL_MDC_IOCTL_I2C_DATA_S*)ptr_data;

    if(AML_DEV_TYPE_I2C == _osal_mdc_ioctl_dev[i2c->dev_num].if_type)
    {
        rc = osal_mdc_readI2cReg(unit, i2c->addr, &(i2c->data), sizeof(i2c->data));
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_osal_mdc_ioctl_RegWriteCallback(
    const UI32_T unit,
    void *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    OSAL_MDC_IOCTL_I2C_DATA_S *i2c = (OSAL_MDC_IOCTL_I2C_DATA_S*)ptr_data;

    if(AML_DEV_TYPE_I2C == _osal_mdc_ioctl_dev[i2c->dev_num].if_type)
    {
        rc = osal_mdc_writeI2cReg(unit, i2c->addr, &(i2c->data), sizeof(i2c->data));
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_osal_mdc_ioctl_initDeviceCallback(
    const UI32_T unit,
    void *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    OSAL_MDC_IOCTL_DEV_DATA_T *ptr_ioctl_data = (OSAL_MDC_IOCTL_DEV_DATA_T*)ptr_data;
    UI32_T idx = 0;

    if(0 == _osal_mdc_devInited)
    {
        rc = osal_mdc_initDevice(_osal_mdc_ioctl_dev, &ptr_ioctl_data->dev_num);
    }
    else
    {
        /* For I2C, device number = 1 */
        ptr_ioctl_data->dev_num = 1;
    }
    if(AIR_E_OK == rc)
    {
        for(idx = 0; idx < ptr_ioctl_data->dev_num; idx ++)
        {
            ptr_ioctl_data->if_type = _osal_mdc_ioctl_dev[idx].if_type;
            ptr_ioctl_data->id[idx].family = _osal_mdc_ioctl_dev[idx].id.family;
            ptr_ioctl_data->id[idx].vendor = _osal_mdc_ioctl_dev[idx].id.vendor;
            ptr_ioctl_data->id[idx].revision = _osal_mdc_ioctl_dev[idx].id.revision;
        }
    }

    _osal_mdc_devInited = 1;

    return (rc);
}

static AIR_ERROR_NO_T
_osal_mdc_registerIoctlCallback(
    const OSAL_MDC_IOCTL_TYPE_T type,
    const OSAL_MDC_IOCTL_CALLBACK_FUNC_T func)
{
    OSAL_MDC_IOCTL_CB_T *ptr_cb = &_osal_mdc_ioctl_cb;
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;

    if (type < OSAL_MDC_IOCTL_TYPE_LAST)
    {
        if (NULL == ptr_cb->callback[type])
        {
            ptr_cb->callback[type] = func;
            rc = AIR_E_OK;
        }
        else
        {
            OSAL_MDC_ERR(KERN_ERR "register ioctl callback failed, type=%d exist\n", type);
        }
    }
    else
    {
        OSAL_MDC_ERR(KERN_ERR "register ioctl callback failed, type=%d >= max=%d\n",
                     type, OSAL_MDC_IOCTL_TYPE_LAST);
    }
    return (rc);
}

static AIR_ERROR_NO_T
_osal_mdc_ioctl_deinitDeviceCallback(
    const UI32_T unit,
    void *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = osal_mdc_deinitDevice();

    return rc;
}

static AIR_ERROR_NO_T
_osal_mdc_initIoctl(void)
{
    memset(&_osal_mdc_ioctl_cb, 0x0, sizeof(OSAL_MDC_IOCTL_CB_T));

    _osal_mdc_registerIoctlCallback(OSAL_MDC_IOCTL_TYPE_INIT_DEV,
                                    _osal_mdc_ioctl_initDeviceCallback);

    _osal_mdc_registerIoctlCallback(OSAL_MDC_IOCTL_TYPE_DEINIT_DEV,
                                    _osal_mdc_ioctl_deinitDeviceCallback);

    _osal_mdc_registerIoctlCallback(OSAL_MDC_IOCTL_TYPE_REG_READ,
                                    _osal_mdc_ioctl_RegReadCallback);

    _osal_mdc_registerIoctlCallback(OSAL_MDC_IOCTL_TYPE_REG_WRITE,
                                    _osal_mdc_ioctl_RegWriteCallback);

    return (AIR_E_OK);
}

static long
_osal_mdc_ioctl(
    struct file *filp,
    unsigned int cmd,
    unsigned long arg)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    int linux_rc = 0;
    OSAL_MDC_IOCTL_CB_T *ptr_cb = &_osal_mdc_ioctl_cb;
    OSAL_MDC_IOCTL_CMD_T *ptr_cmd = (OSAL_MDC_IOCTL_CMD_T*)&cmd;
    UI32_T unit = ptr_cmd->field.unit;
    OSAL_MDC_IOCTL_TYPE_T type = ptr_cmd->field.type;
    OSAL_MDC_IOCTL_ACCESS_T access = ptr_cmd->field.access;
    UI32_T data_size = ptr_cmd->field.size;
    UI8_T *ptr_buf = NULL;

    if(OSAL_MDC_IOCTL_TYPE_LAST > type)
    {
        ptr_buf = kmalloc(data_size, GFP_KERNEL);
        if(NULL == ptr_buf)
        {
            OSAL_MDC_ERR(KERN_ERR "[%s] malloc memory fail.\n", __func__);
            linux_rc = -EFAULT;
        }
        else
        {
            if(0 != copy_from_user(ptr_buf, (int __user*)arg, data_size))
            {
                OSAL_MDC_ERR(KERN_ERR "[%s] copy_from_user fail, arg=%lu, size=%u\n", __func__, arg, data_size);
                linux_rc = -EFAULT;
            }
            if(OSAL_MDC_IOCTL_ACCESS_WRITE == access)
            {
                if( AIR_E_OK != ptr_cb->callback[type](unit, (void*)ptr_buf))
                {
                    linux_rc = -EFAULT;
                }
            }
            else if(OSAL_MDC_IOCTL_ACCESS_READ == access)
            {

                rc = ptr_cb->callback[type](unit, (void*)ptr_buf);
                if(AIR_E_OK == rc)
                {
                    if(0 != copy_to_user((int __user*)arg, ptr_buf, data_size))
                    {
                        OSAL_MDC_ERR(KERN_ERR "[%s] copy_to_user fail, arg=%lu, size=%u\n", __func__, arg, data_size);
                        linux_rc = -EFAULT;
                    }
                }
                else
                {
                    linux_rc = -EFAULT;
                }
            }

            kfree(ptr_buf);
        }
    }
    else
    {
        OSAL_MDC_ERR(KERN_ERR "[%s] invalid ioctl, cmd=%u, arg=%lu, type=%d\n", __func__, cmd, arg, type);
        linux_rc = -EFAULT;
    }

    return (linux_rc);
}

typedef enum
{
    OSAL_DEV_NOT_USE = 0,
    OSAL_DEV_EXCLUSIVE_OPEN,
    OSAL_DEV_LAST
} OSAL_DEV_STATUS_T;

static atomic_t already_open = ATOMIC_INIT(OSAL_DEV_NOT_USE);

static int
_osal_mdc_open(
    struct inode *ptr_inode,
    struct file *ptr_file)
{
    if(atomic_cmpxchg(&already_open, OSAL_DEV_NOT_USE, OSAL_DEV_EXCLUSIVE_OPEN))
        return -EBUSY;

    try_module_get(THIS_MODULE);
    return (0);
}

static int
_osal_mdc_release(
    struct inode *ptr_inode,
    struct file *ptr_file)
{
    atomic_set(&already_open, OSAL_DEV_NOT_USE);
    module_put(THIS_MODULE);
    return (0);
}

static struct file_operations _osal_mdc_fops =
{
    .owner          = THIS_MODULE,
    .open           = _osal_mdc_open,
    .release        = _osal_mdc_release,
    .unlocked_ioctl = _osal_mdc_ioctl,
};

static struct miscdevice _osal_mdc_misc =
{
    .minor  = OSAL_MDC_DRIVER_MISC_MINOR_NUM,
    .name   = OSAL_MDC_DRIVER_NAME,
    .fops   = &_osal_mdc_fops,
};

static int i2c_driver_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    osal_i2c_client = client;
    pr_info("I2C client probe successful\n");
    return 0;
}

static int i2c_driver_remove(struct i2c_client *client)
{
    pr_info("I2C client remove successful\n");
    return 0;
}

static struct of_device_id of_i2c_driver[] = {
    {.compatible = "airoha,an885x"},
    {}
};
MODULE_DEVICE_TABLE(of, of_i2c_driver);

static const struct i2c_device_id an885x_id[] = {
    {"an885x", 0},
    {}
};

static struct i2c_driver osal_i2c_driver = {
    .driver = {
        .name = "an885x",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(of_i2c_driver),
    },
    .probe = i2c_driver_probe,
    .remove = i2c_driver_remove,
    .id_table = an885x_id,
};

static int __init
osal_mdc_module_init(void)
{
    int rc;

    spin_lock_init(&air_lock);

    rc = misc_register(&_osal_mdc_misc);
    if(0 != rc)
    {
        OSAL_MDC_ERR(KERN_ERR "register dev %s failed. rc = %d\n", OSAL_MDC_DRIVER_NAME, rc);
        goto err_misc;
    }

    rc = i2c_add_driver(&osal_i2c_driver);
    if(0 != rc)
    {
        OSAL_MDC_ERR(KERN_ERR "Fail to register i2c client, rc = %d\n", rc);
        goto err_i2c;
    }

    if(osal_i2c_client == NULL)
    {
        OSAL_MDC_ERR(KERN_ERR "Do not probe i2c_client, deinit the misc driver and i2c client driver\r\n");
        goto err_probe;
    }

    /* Register IOCTL callback functions. */
    _osal_mdc_initIoctl();

    return (rc);

err_probe:
    i2c_del_driver(&osal_i2c_driver);
err_i2c:
    misc_deregister(&_osal_mdc_misc);
err_misc:
    return (-ENXIO);
}

static void __exit
osal_mdc_module_exit(void)
{
    misc_deregister(&_osal_mdc_misc);
    i2c_del_driver(&osal_i2c_driver);
}

module_init(osal_mdc_module_init);
module_exit(osal_mdc_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AIROHA");
MODULE_DESCRIPTION("SDK Kernel Module");
