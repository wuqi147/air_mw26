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

/* FILE NAME:  hal_poe.c
 * PURPOSE:
 *  Implement POE module API function.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include <hal/common/hal_poe.h>

#include <api/diag.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_i2c.h>
#ifdef AIR_EN_AN8502_POE
#include <hal/poe/an8502/hal_an8502_poe.h>
#endif
#ifdef AIR_EN_AN8503_POE
#include <hal/poe/an8503/hal_an8503_poe.h>
#endif

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_POE_MON_STACK_SIZE          (510)
#define HAL_POE_MON_THREAD_PRI          (6)
#define HAL_POE_MON_POLLING_INTERVAL_MS (300)
#define HAL_POE_MAX_POE_PORT_CNT        (8)

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_POE_DRIVER_MAP_VECTOR_ENTRY(i) (&_hal_poe_driver_func_vector[i])

#define PTR_HAL_POE_HW_TBL_MUTEX(__unit__, __dev_idx__) (&(_poe_chip_cb[__unit__][__dev_idx__].hw_tbl_mutex))

#define PTR_HAL_POE_MON_DEV_INFO(__unit__, __dev_idx__) (_ptr_poe_mon_cb[__unit__]->ptr_dev_info[__dev_idx__])

#define PTR_HAL_POE_MON_DEV_PORT_INFO(__unit__, __dev_idx__, __port_idx__)             \
    (_ptr_poe_mon_cb[__unit__]->ptr_dev_info[__dev_idx__].ptr_port_info[__port_idx__])

/* DATA TYPE DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_POE, "hal_poe.c");

/* STATIC VARIABLE DECLARATIONS
 */
static HAL_POE_CHIP_CB_T    _poe_chip_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM][AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP];

static HAL_POE_MON_CB_T    *_ptr_poe_mon_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

static HAL_POE_DRIVER_MAP_T _hal_poe_driver_func_vector[] = {
#ifdef AIR_EN_AN8502_POE
    {HAL_POE_DEVICE_ID_IP804AR, hal_an8502_poe_getDriver},
    { HAL_POE_DEVICE_ID_AN8502, hal_an8502_poe_getDriver},
#endif
#ifdef AIR_EN_AN8503_POE
    {HAL_POE_DEVICE_ID_IP808AR, hal_an8503_poe_getDriver},
    { HAL_POE_DEVICE_ID_AN8503, hal_an8503_poe_getDriver}
#endif
};

/* LOCAL SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   _hal_poe_getEfuse
 * PURPOSE:
 *      Get eFuse value.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 *      address                 -- register address
 * OUTPUT:
 *      ptr_result              -- register value
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_poe_getEfuse(
    const UI32_T unit,
    const UI32_T device,
    UI32_T       address,
    UI8_T       *ptr_result)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI16_T         cnt = 0;
    UI32_T         data = 0;

    rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_0, HAL_POE_E_FUSE_ACCESS_ADDRESS, HAL_POE_REG_LEN_1, &address);

    if (AIR_E_OK == rc)
    {
        data = ((0x1 & HAL_POE_E_FUSE_ACCESS_START_BIT_MASK) << HAL_POE_E_FUSE_ACCESS_START_BIT_OFFSET);
        rc = hal_poe_writeReg(unit, device, HAL_POE_PAGE_0, HAL_POE_E_FUSE_ACCESS_CONTROL, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK == rc)
    {
        while (1)
        {
            rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, HAL_POE_E_FUSE_ACCESS_CONTROL, HAL_POE_REG_LEN_1, &data);
            if (AIR_E_OK != rc)
            {
                break;
            }
            if (data & 0x1)
            {
                break;
            }

            cnt++;
            if (cnt > 500)
            {
                rc = AIR_E_TIMEOUT;
                break;
            }
        }
    }
    if (AIR_E_OK == rc)
    {
        rc = hal_poe_readReg(unit, device, HAL_POE_PAGE_0, HAL_POE_E_FUSE_READ_DATA, HAL_POE_REG_LEN_1, &data);
    }
    if (AIR_E_OK != rc)
    {
        data = 0xFF;
    }
    *ptr_result = (UI8_T)(data & 0xFF);

    return rc;
}

/* FUNCTION NAME:   _hal_poe_calculateCurrentHigh
 * PURPOSE:
 *      Calculate current offset (>150mA).
 * INPUT:
 *      offset                  -- Current offset
 * OUTPUT:
 *      None
 * RETURN:
 *      UI8_T                   -- Current offset (>150mA)
 * NOTES:
 *      None
 */
static UI8_T
_hal_poe_calculateCurrentHigh(
    const UI32_T offset)
{
    UI8_T tmp = 0;

    tmp = (offset >> 4) & 0xF;
    if (tmp & 0x8)
    {
        tmp = (tmp & 0x7) + 1;
    }
    else
    {
        tmp = (tmp & 0x7) | 0x80;
    }

    return tmp;
}

/* FUNCTION NAME:   _hal_poe_calculateCurrentLow
 * PURPOSE:
 *      Calculate current offset (<=150mA).
 * INPUT:
 *      offset                  -- Current offset
 * OUTPUT:
 *      None
 * RETURN:
 *      UI8_T                   -- Current offset (<=150mA)
 * NOTES:
 *      None
 */
static UI8_T
_hal_poe_calculateCurrentLow(
    const UI32_T offset)
{
    UI8_T tmp = 0;

    tmp = offset & 0xF;
    if (tmp & 0x8)
    {
        tmp = ((tmp & 0x7) * 2) + 1;
    }
    else
    {
        tmp = ((tmp & 0x7) * 2) + 1;
        tmp = tmp | 0x80;
    }

    return tmp;
}

static UI32_T
_hal_poe_getDevicePortCnt(
    const UI32_T unit,
    const UI32_T device)
{
    UI32_T port_cnt = 0;

    switch (HAL_POE_DEVICE_HW_DEV_ID(unit, device))
    {
        case HAL_POE_DEVICE_ID_AN8502:
            port_cnt = 4;
            break;
        case HAL_POE_DEVICE_ID_IP804AR:
            port_cnt = 4;
            break;
        case HAL_POE_DEVICE_ID_IP808AR:
            port_cnt = 8;
            break;
        case HAL_POE_DEVICE_ID_AN8503:
            port_cnt = 8;
            break;
        default:
            port_cnt = 0;
    }

    return port_cnt;
}

static AIR_ERROR_NO_T
_hal_poe_setPage(
    const UI32_T unit,
    const UI32_T dev,
    const UI32_T page)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         w_data = 0;

    w_data |= ((page & HAL_POE_PAGE_MASK) << HAL_POE_PAGE_OFFSET);
    rc = hal_i2c_writeReg(unit, HAL_POE_DEVICE_I2C_BUS_ID(unit, dev), HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, dev),
                          HAL_POE_PAGE_REG, HAL_POE_REG_LEN_1, HAL_POE_REG_LEN_1, &w_data);

    return rc;
}

static AIR_ERROR_NO_T
_hal_poe_initPoe(
    const UI32_T unit,
    const UI8_T  device_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* PoE init func */
    rc = HAL_POE_FUNC_CALL(unit, device_idx, init, (unit, device_idx));
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: device %d init failed (%d)\n", device_idx, rc);
        return rc;
    }

    if (AIR_E_NOT_SUPPORT == rc)
    {
        return AIR_E_NOT_INITED;
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_poe_probe(
    const UI32_T unit,
    const UI8_T  device_idx)
{
    I8_T                  i;
    HAL_POE_DRIVER_MAP_T *ptr_entry = NULL;
    HAL_POE_DRIVER_T     *ptr_poe_driver_vector = NULL;

    if (0 != sizeof(_hal_poe_driver_func_vector))
    {
        /* Find func vector by DEVICE ID */
        for (i = 0; i < sizeof(_hal_poe_driver_func_vector) / sizeof(HAL_POE_DRIVER_MAP_T); i++)
        {
            ptr_entry = HAL_POE_DRIVER_MAP_VECTOR_ENTRY(i);

            if (HAL_POE_DEVICE_HW_DEV_ID(unit, device_idx) == ptr_entry->poe_dev_id)
            {
                if (NULL != ptr_entry->poe_driver_func)
                {
                    ptr_entry->poe_driver_func(&ptr_poe_driver_vector);
                }
                break;
            }
        }
    }

    if (NULL == ptr_poe_driver_vector)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: device %d probe not found \n", device_idx);
        return AIR_E_NOT_SUPPORT;
    }

    PTR_HAL_POE_CB_CONTEXT(unit, device_idx)->driver = ptr_poe_driver_vector;

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_hal_poe_initRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI8_T              device_idx = 0;
    HAL_POE_CHIP_CB_T *ptr_poe_chip_cb = NULL;

    HAL_CHECK_UNIT(unit);

    for (device_idx = 0; device_idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device_idx++)
    {
        if (HAL_POE_DEVICE_BMP(unit) & (1 << (device_idx)))
        {
            ptr_poe_chip_cb = &(_poe_chip_cb[unit][device_idx]);
            osal_memset(ptr_poe_chip_cb, 0, sizeof(HAL_POE_CHIP_CB_T));

            /* create mutex lock */
            rc = osal_createSemaphore("POE_HW_TABLE", AIR_SEMAPHORE_BINARY, PTR_HAL_POE_HW_TBL_MUTEX(unit, device_idx),
                                      air_module_getModuleName(AIR_MODULE_POE));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_ERR, "PoE device %d init rsrc failed, rc=%d\n", device_idx, rc);
                break;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_poe_deinitRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI8_T              device_idx = 0;
    HAL_POE_CHIP_CB_T *ptr_poe_chip_cb = NULL;

    HAL_CHECK_UNIT(unit);

    for (device_idx = 0; device_idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device_idx++)
    {
        if (HAL_POE_DEVICE_BMP(unit) & (1 << (device_idx)))
        {
            /* destory mutex lock */
            rc = osal_destroySemaphore(PTR_HAL_POE_HW_TBL_MUTEX(unit, device_idx));
            if (AIR_E_OK != rc)
            {
                DIAG_PRINT(HAL_DBG_WARN, "PoE device %d destroy semaphore failed, rc=(%d)\n", device_idx, rc);
                return rc;
            }

            /* reset control block */
            ptr_poe_chip_cb = &(_poe_chip_cb[unit][device_idx]);
            osal_memset(ptr_poe_chip_cb, 0, sizeof(HAL_POE_CHIP_CB_T));

            /* destory chip control block */
            if (NULL != PTR_HAL_POE_CB_CONTEXT(unit, device_idx))
            {
                /* free the allocated memory */
                osal_free(PTR_HAL_POE_CB_CONTEXT(unit, device_idx));
                PTR_HAL_POE_CB_CONTEXT(unit, device_idx) = NULL;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_poe_initDriver(
    const UI32_T unit,
    const UI8_T  device_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         hw_rev_data = 0;

    /* update poe hardware info */
    rc = hal_poe_readReg(unit, device_idx, HAL_POE_PAGE_1, HAL_POE_HW_REVISION_REG, HAL_POE_REG_LEN_2, &hw_rev_data);
    if (AIR_E_OK == rc)
    {
        hw_rev_data =
            (hw_rev_data & 0xffff0000) | ((hw_rev_data & 0x0000ff00) >> 8) | ((hw_rev_data & 0x000000ff) << 8);
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: device %d hw revision %x\n", device_idx, hw_rev_data);
        HAL_POE_DEVICE_HW_DEV_ID(unit, device_idx) = hw_rev_data;

        /* probe the driver */
        rc = _hal_poe_probe(unit, device_idx);
        if (AIR_E_OK == rc)
        {
            /* poe init */
            rc = _hal_poe_initPoe(unit, device_idx);
        }
        else
        {
            rc = AIR_E_NOT_INITED;
        }
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }

    return rc;
}

static AIR_ERROR_NO_T
_hal_poe_deinitDriver(
    const UI32_T unit,
    const UI8_T  device_idx)
{
    /* detach the driver */
    PTR_HAL_POE_CB_CONTEXT(unit, device_idx)->driver = NULL;
    /* clear poe hardware info */
    HAL_POE_DEVICE_HW_DEV_ID(unit, device_idx) = 0;

    return AIR_E_OK;
}

static void
_hal_poe_thread(
    void *ptr_argv)
{
    UI32_T                     unit = (UI32_T)(AIR_HUGE_T)ptr_argv;
    UI32_T                     interval = HAL_POE_MON_POLLING_INTERVAL_MS;
    UI32_T                     device = 0, poe_port = 0, port_cnt = 0, air_port = 0;
    HAL_POE_PORT_IND_T         port_ind[HAL_POE_MAX_POE_PORT_CNT];
    AIR_POE_PORT_MEASUREMENT_T meas[HAL_POE_MAX_POE_PORT_CNT];
    AIR_POE_PORT_STATUS_T      ps[HAL_POE_MAX_POE_PORT_CNT];
    HAL_POE_POWER_EVENT_T      event[HAL_POE_MAX_POE_PORT_CNT];
    BOOL_T                     ready[HAL_POE_MAX_POE_PORT_CNT];

    osal_memset(port_ind, 0, sizeof(port_ind));
    osal_memset(meas, 0, sizeof(meas));
    osal_memset(ps, 0, sizeof(ps));
    osal_memset(event, 0, sizeof(event));
    osal_memset(ready, 0, sizeof(ready));

    while (AIR_E_OK == osal_isRunThread())
    {
        osal_sleepTask(interval);
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            if (HAL_POE_DEVICE_BMP(unit) & (1 << (device)))
            {
                port_cnt = _hal_poe_getDevicePortCnt(unit, device);

                for (poe_port = 0; poe_port < port_cnt; poe_port++)
                {
                    HAL_POE_PORT_TO_AIR_PORT(unit, device, poe_port, air_port);
                    port_ind[poe_port].port = air_port;
                    port_ind[poe_port].alt = AIR_POE_ALTERNATIVE_PRIMARY;
                }
                HAL_POE_FUNC_CALL(unit, device, getPortMeasurement, (unit, port_ind, port_cnt, meas));
                HAL_POE_FUNC_CALL(unit, device, getPortStatus, (unit, port_ind, port_cnt, ps));
                HAL_POE_FUNC_CALL(unit, device, getPortEvent, (unit, port_ind, port_cnt, event));
                HAL_POE_FUNC_CALL(unit, device, getPortPowerUp, (unit, port_ind, port_cnt, ready));

                /* update db */
                for (poe_port = 0; poe_port < port_cnt; poe_port++)
                {
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).current = meas[poe_port].current;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).voltage = meas[poe_port].voltage;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).temperature = meas[poe_port].temperature;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).pd_signature = ps[poe_port].pd_signature;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).pd_class = ps[poe_port].pd_class;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).power_status = ps[poe_port].power_status;
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).state_machine = ready[poe_port];
                    PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).power_event |= (UI16_T)event[poe_port].flags;
                }
            }
        }
    }
}

AIR_ERROR_NO_T
_hal_poe_initThreadRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI8_T                        device = 0, dev_cnt = 0;
    UI32_T                       port_cnt = 0;
    HAL_POE_MON_CB_T            *ptr_cb = NULL;
    HAL_POE_MON_DEV_INFO_T      *ptr_dev_info = NULL;
    HAL_POE_MON_DEV_PORT_INFO_T *ptr_port_info = NULL;

    ptr_cb = (HAL_POE_MON_CB_T *)osal_alloc(sizeof(HAL_POE_MON_CB_T), air_module_getModuleName(AIR_MODULE_POE));
    if (NULL == ptr_cb)
    {
        DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc mem size=%zu failed\n", unit, sizeof(HAL_POE_MON_CB_T));
        rc = AIR_E_NO_MEMORY;
    }
    else
    {
        osal_memset(ptr_cb, 0, sizeof(HAL_POE_MON_CB_T));
        _ptr_poe_mon_cb[unit] = ptr_cb;
        device = HAL_POE_DEVICE_BMP(unit);
        while (device)
        {
            device >>= 1;
            dev_cnt++;
        }
        ptr_dev_info = (HAL_POE_MON_DEV_INFO_T *)osal_alloc(dev_cnt * sizeof(HAL_POE_MON_DEV_INFO_T),
                                                            air_module_getModuleName(AIR_MODULE_POE));
        if (NULL == ptr_dev_info)
        {
            DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc poe dev mem size=%zu failed\n", unit,
                       sizeof(HAL_POE_CB_T) * (device + 1));
            osal_free(_ptr_poe_mon_cb[unit]);
            _ptr_poe_mon_cb[unit] = NULL;
            rc = AIR_E_NO_MEMORY;
        }
        else
        {
            osal_memset(ptr_dev_info, 0x00, dev_cnt * sizeof(HAL_POE_MON_DEV_INFO_T));
            _ptr_poe_mon_cb[unit]->ptr_dev_info = ptr_dev_info;

            for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
            {
                if (HAL_POE_DEVICE_BMP(unit) & (1 << (device)))
                {
                    port_cnt = _hal_poe_getDevicePortCnt(unit, device);

                    ptr_port_info = (HAL_POE_MON_DEV_PORT_INFO_T *)osal_alloc(
                        port_cnt * sizeof(HAL_POE_MON_DEV_PORT_INFO_T), air_module_getModuleName(AIR_MODULE_POE));
                    if (NULL == ptr_port_info)
                    {
                        DIAG_PRINT(HAL_DBG_ERR, "u=%u, alloc poe dev port mem size=%zu failed\n", unit,
                                   sizeof(HAL_POE_MON_DEV_PORT_INFO_T) * port_cnt);
                        while (device > 0)
                        {
                            device--;
                            if (NULL != _ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info)
                            {
                                osal_free(_ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info);
                                _ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info = NULL;
                            }
                        }
                        if (NULL != _ptr_poe_mon_cb[unit]->ptr_dev_info)
                        {
                            osal_free(_ptr_poe_mon_cb[unit]->ptr_dev_info);
                            _ptr_poe_mon_cb[unit]->ptr_dev_info = NULL;
                        }
                        if (NULL != _ptr_poe_mon_cb[unit])
                        {
                            osal_free(_ptr_poe_mon_cb[unit]);
                            _ptr_poe_mon_cb[unit] = NULL;
                        }
                        rc = AIR_E_NO_MEMORY;
                        break;
                    }
                    else
                    {
                        osal_memset(ptr_port_info, 0x00, port_cnt * sizeof(HAL_POE_MON_DEV_PORT_INFO_T));
                        _ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info = ptr_port_info;
                    }
                }
            }
        }
    }

    return rc;
}

AIR_ERROR_NO_T
_hal_poe_deinitThreadRsrc(
    const UI32_T unit)
{
    UI8_T device = 0, dev_cnt = 0;

    /* calculate the total number of devices */
    device = HAL_POE_DEVICE_BMP(unit);
    while (device)
    {
        device >>= 1;
        dev_cnt++;
    }

    /* release memory */
    for (device = 0; device < dev_cnt; device++)
    {
        if (NULL != _ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info)
        {
            osal_free(_ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info);
            _ptr_poe_mon_cb[unit]->ptr_dev_info[device].ptr_port_info = NULL;
        }
    }

    if (NULL != _ptr_poe_mon_cb[unit]->ptr_dev_info)
    {
        osal_free(_ptr_poe_mon_cb[unit]->ptr_dev_info);
        _ptr_poe_mon_cb[unit]->ptr_dev_info = NULL;
    }

    if (NULL != _ptr_poe_mon_cb[unit])
    {
        osal_free(_ptr_poe_mon_cb[unit]);
        _ptr_poe_mon_cb[unit] = NULL;
    }

    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   hal_poe_handleFloat
 * PURPOSE:
 *      Handling the fractional part: bits to decimal
 * INPUT:
 *      float_bit               -- Float binary representation
 *      bit_size                -- number of bits
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T                  -- Decimal representation of the float
 * NOTES:
 *      None
 */
UI32_T
hal_poe_handleFloat(
    const UI32_T float_bit,
    const UI8_T  bit_size)
{
    UI8_T               i;
    UI32_T              tmp = 0x0;
    static const UI32_T bit_table[] = {51200, 25600, 12800, 6400, 3200, 1600, 800, 400};

    for (i = 0; i < bit_size; i++)
    {
        if ((float_bit >> (bit_size - i - 1)) & 0x1)
        {
            tmp = tmp + bit_table[i];
        }
    }

    return tmp;
}

/* FUNCTION NAME:   hal_poe_getFixPortCurrent
 * PURPOSE:
 *      Fix port current with offset.
 * INPUT:
 *      poe_port                -- Poe_port
 *      offset                  -- Current offset
 * OUTPUT:
 *      ptr_result              -- Result port current
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getFixPortCurrent(
    const UI8_T  poe_port,
    const UI32_T offset,
    UI32_T      *ptr_result)
{
    UI8_T  tmp8 = 0;
    UI32_T port_current = *ptr_result;
    BOOL_T fixed = TRUE;

    if (port_current <= 150)
    {
        tmp8 = _hal_poe_calculateCurrentLow(offset);
        if (tmp8 & 0x80)
        {
            if (port_current <= (tmp8 & ~0x80))
            {
                port_current = 0;
                fixed = FALSE;
            }
            if (fixed)
            {
                port_current = port_current - (UI32_T)(tmp8 & ~0x80);
            }
        }
        else
        {
            port_current = port_current + (UI32_T)tmp8;
        }
        if (fixed && poe_port < 4)
        {
            port_current = port_current + 2;
        }
    }
    else
    {
        tmp8 = _hal_poe_calculateCurrentHigh(offset);
        if (tmp8 & 0x80)
        {
            port_current = port_current - (UI32_T)(tmp8 & ~0x80);
        }
        else
        {
            port_current = port_current + (UI32_T)tmp8;
        }
    }

    *ptr_result = port_current;

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_poe_calculateVoltageOffset
 * PURPOSE:
 *      Calculate voltage offset.
 * INPUT:
 *      offset                  -- Voltage offset
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T                  -- Calculated voltage offset
 * NOTES:
 *      None
 */
UI32_T
hal_poe_calculateVoltageOffset(
    const UI32_T offset)
{
    UI32_T tmp = 0;

    if (offset & 0x8)
    {
        tmp = (offset & 0x7) * 100;
    }
    else
    {
        tmp = ((offset & 0x7) + 1) * 100;
        tmp = tmp | 0x8000;
    }

    return tmp;
}

/* FUNCTION NAME:   hal_poe_getVoltageOffset
 * PURPOSE:
 *      Get PSE voltage offset.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_offset              -- voltage offset
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getVoltageOffset(
    const UI32_T unit,
    const UI32_T device,
    UI8_T       *ptr_offset)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          tmp = 0, result = 0;

    rc = _hal_poe_getEfuse(unit, device, 0x1F, &tmp);
    if (rc == AIR_E_OK)
    {
        tmp = (tmp & 0xC0) >> 4;
        result = tmp;

        rc = _hal_poe_getEfuse(unit, device, 0x1E, &tmp);
    }
    if (rc == AIR_E_OK)
    {
        tmp = (tmp & 0xC0) >> 6;
        result = result | tmp;

        *ptr_offset = result;
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_getCurrentOffset
 * PURPOSE:
 *      Get PSE current offset.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_offset              -- current offset
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getCurrentOffset(
    const UI32_T unit,
    const UI32_T device,
    UI8_T       *ptr_offset)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          tmp = 0, result = 0;

    rc = _hal_poe_getEfuse(unit, device, 0x2B, &tmp);
    if (rc == AIR_E_OK)
    {
        tmp = tmp & 0xF0;
        result = tmp;

        rc = _hal_poe_getEfuse(unit, device, 0x23, &tmp);
    }
    if (rc == AIR_E_OK)
    {
        tmp = (tmp & 0xC0) >> 4;
        result = result | tmp;

        rc = _hal_poe_getEfuse(unit, device, 0x22, &tmp);
    }
    if (rc == AIR_E_OK)
    {
        tmp = (tmp & 0xC0) >> 6;
        result = result | tmp;

        *ptr_offset = result;
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_getOffsetState
 * PURPOSE:
 *      Get PSE offset state register.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_state               -- offset state
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getOffsetState(
    const UI32_T unit,
    const UI32_T device,
    BOOL_T      *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          tmp = 0;

    rc = _hal_poe_getEfuse(unit, device, 0x21, &tmp);
    if (AIR_E_OK == rc)
    {
        if ((tmp >> 6) == 0x1)
        {
            *ptr_state = TRUE;
        }
        else
        {
            *ptr_state = FALSE;
        }
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_initThread
 * PURPOSE:
 *      This API is used to initialize POE thread.
 * INPUT:
 *      unit        --  Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *      AIR_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_initThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_MODULE)
        {
            rc = _hal_poe_initThreadRsrc(unit);
            if (AIR_E_OK == rc)
            {
                rc = osal_createThread("POEMON", HAL_POE_MON_STACK_SIZE, HAL_POE_MON_THREAD_PRI, _hal_poe_thread,
                                       (void *)((AIR_HUGE_T)unit), &(_ptr_poe_mon_cb[unit]->thread_id));
            }
        }
        else
        {
            rc = AIR_E_NOT_SUPPORT;
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_POE) |= (HAL_INIT_STAGE_TASK);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_deinitThread
 * PURPOSE:
 *      This API is used to deinitialize POE thread.
 * INPUT:
 *      unit        --  Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *      AIR_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_deinitThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        rc = osal_stopThread(&(_ptr_poe_mon_cb[unit]->thread_id));
        if (AIR_E_OK == rc)
        {
            rc = osal_destroyThread(&(_ptr_poe_mon_cb[unit]->thread_id));
        }
        if (AIR_E_OK == rc)
        {
            rc = _hal_poe_deinitThreadRsrc(unit);
        }
    }

    HAL_MODULE_INITED(unit, AIR_MODULE_POE) &= (~(HAL_INIT_STAGE_TASK));

    return rc;
}

/* FUNCTION NAME:   hal_poe_initModule
 * PURPOSE:
 *      This API is used to initialize POE module.
 * INPUT:
 *      unit        --  Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *      AIR_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_initModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device_idx = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_MODULE)
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (AIR_E_OK == rc)
    {
        for (device_idx = 0; device_idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device_idx++)
        {
            if (HAL_POE_DEVICE_BMP(unit) & (1 << (device_idx)))
            {
                /* init poe driver */
                rc = _hal_poe_initDriver(unit, device_idx);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: device %d init driver failed, rc=%d\n", device_idx, rc);
                    break;
                }
            }
        }

        if (AIR_E_OK != rc)
        {
            hal_poe_deinitModule(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_POE) |= (HAL_INIT_STAGE_MODULE);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_deinitModule
 * PURPOSE:
 *      This API is used to deinitialize POE module.
 * INPUT:
 *      unit        --  Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *      AIR_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_deinitModule(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI8_T          device_idx = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_MODULE)
    {
        for (device_idx = 0; device_idx < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device_idx++)
        {
            if (HAL_POE_DEVICE_BMP(unit) & (1 << (device_idx)))
            {
                /* deinit poe driver */
                rc = _hal_poe_deinitDriver(unit, device_idx);
                if (AIR_E_OK != rc)
                {
                    DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: device %d deinit driver failed, rc=%d\n", device_idx, rc);
                    break;
                }
            }
        }
    }

    HAL_MODULE_INITED(unit, AIR_MODULE_POE) &= (~(HAL_INIT_STAGE_MODULE));

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_poe_init
 * PURPOSE:
 *      This API is used to init POE.
 * INPUT:
 *      unit        --  Device ID
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *      AIR_E_ALREADY_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE(unit))
    {
        rc = AIR_E_ALREADY_INITED;
    }

    if (!HAL_POE_DEVICE_BMP(unit))
    {
        rc = AIR_E_NOT_SUPPORT;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_poe_initRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = hal_poe_initModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = hal_poe_initThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_POE) |= HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_deinit
 * PURPOSE:
 *      This API is used to deinit POE.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (!(HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE(unit)))
    {
        rc = AIR_E_NOT_INITED;
    }

    if (AIR_E_OK == rc)
    {
        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK_RSRC)
        {
            rc = _hal_poe_deinitRsrc(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_MODULE)
        {
            rc = hal_poe_deinitModule(unit);
        }

        if (HAL_INIT_STAGE(unit) & HAL_INIT_STAGE_TASK)
        {
            rc = hal_poe_deinitThread(unit);
        }
    }

    if (AIR_E_OK == rc)
    {
        HAL_MODULE_INITED(unit, AIR_MODULE_POE) &= ~HAL_INIT_STAGE(unit);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_readReg
 * PURPOSE:
 *      This API is used to get register data.
 * INPUT:
 *      unit                     --  Device unit number
 *      dev                      --  Poe device ID
 *      page                     --  Page number
 *      addr                     --  Register address
 *      len                      --  Register address length
 *
 * OUTPUT:
 *      ptr_val                  --  Register data
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_readReg(
    const UI32_T unit,
    const UI32_T dev,
    const UI32_T page,
    const UI32_T addr,
    const UI32_T len,
    UI32_T      *ptr_val)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_HW_TBL_LOCK(unit, dev);
    rc = _hal_poe_setPage(unit, dev, page);
    if (AIR_E_OK == rc)
    {
        rc = hal_i2c_readReg(unit, HAL_POE_DEVICE_I2C_BUS_ID(unit, dev), HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, dev), addr,
                             HAL_POE_REG_LEN_1, len, ptr_val);
    }
    HAL_POE_HW_TBL_UNLOCK(unit, dev);

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Device %d read page[%d] addr[0x%x] fail!\n", dev, page, addr);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_writeReg
 * PURPOSE:
 *      This API is used to set register data.
 * INPUT:
 *      unit                     --  Device unit number
 *      dev                      --  Poe device ID
 *      page                     --  Page number
 *      addr                     --  Register address
 *      len                      --  Register address length
 *      ptr_val                  --  Register data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_writeReg(
    const UI32_T unit,
    const UI32_T dev,
    const UI32_T page,
    const UI32_T addr,
    const UI32_T len,
    UI32_T      *ptr_val)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_POE_HW_TBL_LOCK(unit, dev);
    rc = _hal_poe_setPage(unit, dev, page);
    if (AIR_E_OK == rc)
    {
        rc = hal_i2c_writeReg(unit, HAL_POE_DEVICE_I2C_BUS_ID(unit, dev), HAL_POE_DEVICE_I2C_SLAVE_ADDR(unit, dev),
                              addr, HAL_POE_REG_LEN_1, len, ptr_val);
    }
    HAL_POE_HW_TBL_UNLOCK(unit, dev);

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "Device %d write page[%d] addr[0x%x] fail!\n", dev, page, addr);
    }

    return rc;
}

/* FUNCTION NAME:   hal_poe_lockHwResource
 * PURPOSE:
 *      Lock the resource of hardware table access.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_lockHwResource(
    const UI32_T unit,
    const UI32_T device)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_POE);

    return HAL_COMMON_LOCK_RESOURCE(PTR_HAL_POE_HW_TBL_MUTEX(unit, device), AIR_SEMAPHORE_WAIT_FOREVER);
}

/* FUNCTION NAME:   hal_poe_unlockHwResource
 * PURPOSE:
 *      Unlock the resource of hardware table access.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation is successfull.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_unlockHwResource(
    const UI32_T unit,
    const UI32_T device)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_INIT(unit, AIR_MODULE_POE);

    return HAL_COMMON_FREE_RESOURCE(PTR_HAL_POE_HW_TBL_MUTEX(unit, device));
}

/* FUNCTION NAME:   hal_poe_getPortPowerUp
 * PURPOSE:
 *      Get state machine of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_ready               -- state suspended before entering power up
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getPortPowerUp(
    const UI32_T                unit,
    const UI32_T                air_port,
    const AIR_POE_ALTERNATIVE_T alt,
    BOOL_T                     *ptr_ready)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device = 0, poe_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, air_port, alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, air_port, alt, poe_port);

        *ptr_ready = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).state_machine;
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

/* FUNCTION NAME:   hal_poe_getPortMeasurement
 * PURPOSE:
 *      Get measurement of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_meas                -- measurement data
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getPortMeasurement(
    const UI32_T                unit,
    const UI32_T                air_port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device = 0, poe_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, air_port, alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, air_port, alt, poe_port);

        ptr_meas->current = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).current;
        ptr_meas->voltage = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).voltage;
        ptr_meas->temperature = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).temperature;
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

/* FUNCTION NAME:   hal_poe_getPortStatus
 * PURPOSE:
 *      Get status of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_ps                  -- status
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getPortStatus(
    const UI32_T                unit,
    const UI32_T                air_port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PORT_STATUS_T      *ptr_ps)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device = 0, poe_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, air_port, alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, air_port, alt, poe_port);

        ptr_ps->pd_class = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).pd_class;
        ptr_ps->pd_signature = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).pd_signature;
        ptr_ps->power_status = PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).power_status;
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

/* FUNCTION NAME:   hal_poe_getPortEvent
 * PURPOSE:
 *      Get event of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      ptr_event               -- event flags
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_getPortEvent(
    const UI32_T                unit,
    const UI32_T                air_port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_EVENT_T            *ptr_event)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device = 0, poe_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, air_port, alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, air_port, alt, poe_port);

        ptr_event->event_flags = (UI32_T)PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).power_event;
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

/* FUNCTION NAME:   hal_poe_clearPortEvent
 * PURPOSE:
 *      Clear event records of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- role of the alternative
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_clearPortEvent(
    const UI32_T                unit,
    const UI32_T                air_port,
    const AIR_POE_ALTERNATIVE_T alt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         device = 0, poe_port = 0;

    if (HAL_MODULE_INITED(unit, AIR_MODULE_POE) & HAL_INIT_STAGE_TASK)
    {
        HAL_POE_AIR_PORT_TO_DEV_IDX(unit, air_port, alt, device);
        HAL_POE_AIR_PORT_TO_POE_PORT(unit, air_port, alt, poe_port);

        PTR_HAL_POE_MON_DEV_PORT_INFO(unit, device, poe_port).power_event = 0;
    }
    else
    {
        rc = AIR_E_NOT_SUPPORT;
    }
    return rc;
}

/* FUNCTION NAME:   hal_poe_getDeviceInfo
 * PURPOSE:
 *      Get revision ID of the PoE device.
 * INPUT:
 *      unit                    -- unit id
 *      device                  -- device id
 * OUTPUT:
 *      ptr_value               -- device revision ID
 * RETURN:
 *      AIR_E_OK                -- Successfully write the data.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      APP PoE requires it for a workaround.
 */
AIR_ERROR_NO_T
hal_poe_getDeviceInfo(
    const UI32_T unit,
    const UI32_T device,
    UI16_T      *ptr_value)
{
    *ptr_value = HAL_POE_DEVICE_HW_DEV_ID(unit, device);
    return AIR_E_OK;
}
