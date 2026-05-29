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
/* FILE NAME:  hal_poe.h
 * PURPOSE:
 *      It provides POE module API.
 * NOTES:
 */
#ifndef HAL_POE_H
#define HAL_POE_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_poe.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_POE_PAGE_0 (0)
#define HAL_POE_PAGE_1 (1)
#define HAL_POE_PAGE_2 (2)

#define HAL_POE_REG_LEN_1           (1)
#define HAL_POE_REG_LEN_2           (2)
#define HAL_POE_REG_ADDR_BLOCK_SIZE (16)
#define HAL_POE_BITS_PER_ADDR       (8)
#define HAL_POE_U32_NUM_PER_BLOCK   ((HAL_POE_REG_ADDR_BLOCK_SIZE * HAL_POE_BITS_PER_ADDR) / 32)

#define HAL_POE_PAGE_REG                       (0x00)
#define HAL_POE_PAGE_OFFSET                    (6)
#define HAL_POE_PAGE_MASK                      (0x3)
#define HAL_POE_HW_REVISION_REG                (0x03)
#define HAL_POE_E_FUSE_ACCESS_CONTROL          (0x50)
#define HAL_POE_E_FUSE_ACCESS_ADDRESS          (0x51)
#define HAL_POE_E_FUSE_READ_DATA               (0x53)
#define HAL_POE_E_FUSE_ACCESS_START_BIT_MASK   (0x1)
#define HAL_POE_E_FUSE_ACCESS_START_BIT_OFFSET (7)

#define HAL_POE_PORT_REG(__base__, __port__, __offt__) ((__base__) + (__port__) * (__offt__))

#define PTR_HAL_POE_DRIVER(__unit__, __dev_idx__)         \
    PTR_HAL_POE_CB_CONTEXT(__unit__, __dev_idx__)->driver
#define HAL_POE_DEVICE_INFO(__unit__, __dev_idx__)               \
    PTR_HAL_POE_CB_CONTEXT(__unit__, __dev_idx__)->poe_chip_info
#define HAL_POE_PORT_MAP_INFO(__unit__, __dev_idx__, __poe_port__)                     \
    PTR_HAL_POE_CB_CONTEXT(__unit__, __dev_idx__)->ptr_poe_port_map_info[__poe_port__]
#define HAL_POE_DEVICE_I2C_BUS_ID(__unit__, __dev_idx__)  \
    HAL_POE_DEVICE_INFO(__unit__, __dev_idx__).i2c_bus_id
#define HAL_POE_DEVICE_I2C_SLAVE_ADDR(__unit__, __dev_idx__)  \
    HAL_POE_DEVICE_INFO(__unit__, __dev_idx__).i2c_slave_addr
#define HAL_POE_DEVICE_HW_DEV_ID(__unit__, __dev_idx__)  \
    HAL_POE_DEVICE_INFO(__unit__, __dev_idx__).device_id

#define HAL_POE_AIR_PORT_TO_DEV_IDX(__unit__, __air_port__, __port_alt__, __dev_idx__)                     \
    do                                                                                                     \
    {                                                                                                      \
        if (__port_alt__ == AIR_POE_ALTERNATIVE_NONE || __port_alt__ == AIR_POE_ALTERNATIVE_PRIMARY)       \
        {                                                                                                  \
            __dev_idx__ = HAL_SDK_PORT_MAP_POE_INFO(__unit__, __air_port__).primary_port.poe_device_idx;   \
        }                                                                                                  \
        else if (__port_alt__ == AIR_POE_ALTERNATIVE_SECONDARY)                                            \
        {                                                                                                  \
            __dev_idx__ = HAL_SDK_PORT_MAP_POE_INFO(__unit__, __air_port__).secondary_port.poe_device_idx; \
        }                                                                                                  \
    } while (0)

#define HAL_POE_AIR_PORT_TO_POE_PORT(__unit__, __air_port__, __port_alt__, __poe_port__)                   \
    do                                                                                                     \
    {                                                                                                      \
        if (__port_alt__ == AIR_POE_ALTERNATIVE_NONE || __port_alt__ == AIR_POE_ALTERNATIVE_PRIMARY)       \
        {                                                                                                  \
            __poe_port__ = HAL_SDK_PORT_MAP_POE_INFO(__unit__, __air_port__).primary_port.hw_package_id;   \
        }                                                                                                  \
        else if (__port_alt__ == AIR_POE_ALTERNATIVE_SECONDARY)                                            \
        {                                                                                                  \
            __poe_port__ = HAL_SDK_PORT_MAP_POE_INFO(__unit__, __air_port__).secondary_port.hw_package_id; \
        }                                                                                                  \
    } while (0)

#define HAL_POE_PORT_TO_AIR_PORT(__unit__, __dev_idx__, __poe_port__, __air_port__) \
    do                                                                              \
    {                                                                               \
        __air_port__ = HAL_POE_PORT_MAP_INFO(__unit__, __dev_idx__, __poe_port__);  \
    } while (0)

#define HAL_POE_CALC_U32_COUNT(port_cnt, ports_per_u32)    \
    (((port_cnt) + (ports_per_u32) - 1) / (ports_per_u32))

#define HAL_POE_SWAP_U16_BYTE_ORDER(val) ((((val) & 0x00FF) << 8) | (((val) & 0xFF00) >> 8))

#define HAL_POE_U32_DIFF(a, b) ((a) > (b) ? (a) - (b) : (b) - (a))

#define HAL_POE_FUNC_CALL(__unit__, __dev_idx__, __func__, __param__) ({                  \
    AIR_ERROR_NO_T __rc = AIR_E_OK;                                                       \
    if ((NULL == PTR_HAL_POE_CB_CONTEXT(__unit__, __dev_idx__)) ||                        \
        (NULL == PTR_HAL_POE_DRIVER(__unit__, __dev_idx__)) ||                            \
        (NULL == PTR_HAL_POE_DRIVER(__unit__, __dev_idx__)->hal_poe_##__func__))          \
    {                                                                                     \
        __rc = AIR_E_NOT_SUPPORT;                                                         \
    }                                                                                     \
    else                                                                                  \
    {                                                                                     \
        __rc = (PTR_HAL_POE_DRIVER(__unit__, __dev_idx__)->hal_poe_##__func__ __param__); \
    }                                                                                     \
    __rc;                                                                                 \
})

#define HAL_POE_HW_TBL_LOCK(__unit__, __dev_idx__)   hal_poe_lockHwResource(__unit__, __dev_idx__)
#define HAL_POE_HW_TBL_UNLOCK(__unit__, __dev_idx__) hal_poe_unlockHwResource(__unit__, __dev_idx__)

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI32_T                port;
    AIR_POE_ALTERNATIVE_T alt;
} HAL_POE_PORT_IND_T;

typedef struct
{
#define HAL_POE_POWER_EVENT_FLAGS_OVERLOAD            (1UL << 0)
#define HAL_POE_POWER_EVENT_FLAGS_SHORT_CIRCUIT       (1UL << 1)
#define HAL_POE_POWER_EVENT_FLAGS_DC_DISCONNECT       (1UL << 2)
#define HAL_POE_POWER_EVENT_FLAGS_VOLTAGE_BAD_EVENT   (1UL << 3)
#define HAL_POE_POWER_EVENT_FLAGS_THERMAL_SHUTDOWN    (1UL << 4)
#define HAL_POE_POWER_EVENT_FLAGS_CURRENT_LIMIT       (1UL << 5)
#define HAL_POE_POWER_EVENT_FLAGS_TRUNK_VOLTAGE_LIMIT (1UL << 6)
#define HAL_POE_POWER_EVENT_FLAGS_TEMPERATURE         (1UL << 7)
    UI32_T flags;
} HAL_POE_POWER_EVENT_T;

typedef enum
{
    HAL_POE_PAIR_A = 0,
    HAL_POE_PAIR_B,
    HAL_POE_PAIR_LAST
} HAL_POE_PAIR_T;

typedef enum
{
    HAL_POE_OPERATING_MODE_AUTO = 0,
    HAL_POE_OPERATING_MODE_MANUAL,
    HAL_POE_OPERATING_MODE_LAST
} HAL_POE_OPERATING_MODE_T;

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_INIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T device);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETPOWERUPMODE_FUNC_T)(
    const UI32_T             unit,
    const UI32_T             device,
    AIR_POE_POWER_UP_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_SETPOWERUPMODE_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  device,
    const AIR_POE_POWER_UP_MODE_T mode);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_TRIGGERPOWERUP_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETPORTPOWERUP_FUNC_T)(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    BOOL_T                   *ptr_ready);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETTYPE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_TYPE_T             *ptr_type);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_SETTYPE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_TYPE_T        type);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETPSE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    AIR_POE_PSE_MODE_T         *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_SETPSE_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const AIR_POE_PSE_MODE_T    mode);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETMEASUREMENT_FUNC_T)(
    const UI32_T                unit,
    const HAL_POE_PORT_IND_T   *ptr_poe_ind,
    const UI32_T                poe_info_cnt,
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETMEASUREMENT_FUNC_T)(
    const UI32_T                  unit,
    const UI32_T                  device,
    AIR_POE_DEVICE_MEASUREMENT_T *ptr_meas);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETSTATUS_FUNC_T)(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    AIR_POE_PORT_STATUS_T    *ptr_ps);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETEVENT_FUNC_T)(
    const UI32_T              unit,
    const HAL_POE_PORT_IND_T *ptr_poe_ind,
    const UI32_T              poe_info_cnt,
    HAL_POE_POWER_EVENT_T    *ptr_event);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETSCRATCH_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_value);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_SETSCRATCH_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T value);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETCURRENTLIMIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_current);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_SETCURRENTLIMIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T current);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETPAIRMODE_FUNC_T)(
    const UI32_T    unit,
    const UI32_T    port,
    HAL_POE_PAIR_T *ptr_pair);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_SETPAIRMODE_FUNC_T)(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_POE_PAIR_T pair);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETOPERATINGMODE_FUNC_T)(
    const UI32_T              unit,
    const UI32_T              device,
    HAL_POE_OPERATING_MODE_T *ptr_mode);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETPOWERSTRATEGY_FUNC_T)(
    const UI32_T            unit,
    const UI32_T            device,
    AIR_POE_PWR_STRATEGY_T *ptr_strategy);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_SETPOWERSTRATEGY_FUNC_T)(
    const UI32_T                 unit,
    const UI32_T                 device,
    const AIR_POE_PWR_STRATEGY_T strategy);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETPOWERLIMIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    BOOL_T      *ptr_enable,
    UI32_T      *ptr_limit);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_SETPOWERLIMIT_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    const BOOL_T enable,
    const UI32_T limit);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_GETERRORDELAY_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    UI32_T      *ptr_delay);

typedef AIR_ERROR_NO_T (*HAL_POE_DEVICE_SETERRORDELAY_FUNC_T)(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T delay);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_GETPRIORITY_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    UI32_T                     *ptr_priority);

typedef AIR_ERROR_NO_T (*HAL_POE_PORT_SETPRIORITY_FUNC_T)(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const UI32_T                priority);

typedef struct
{
    HAL_POE_DEVICE_INIT_FUNC_T             hal_poe_init;
    HAL_POE_DEVICE_GETOPERATINGMODE_FUNC_T hal_poe_getDeviceOperatingMode;
    HAL_POE_DEVICE_GETSCRATCH_FUNC_T       hal_poe_getDeviceScratch;
    HAL_POE_DEVICE_SETSCRATCH_FUNC_T       hal_poe_setDeviceScratch;
    HAL_POE_DEVICE_GETPOWERUPMODE_FUNC_T   hal_poe_getDevicePowerUpMode;
    HAL_POE_DEVICE_SETPOWERUPMODE_FUNC_T   hal_poe_setDevicePowerUpMode;
    HAL_POE_DEVICE_GETPOWERSTRATEGY_FUNC_T hal_poe_getDevicePowerStrategy;
    HAL_POE_DEVICE_SETPOWERSTRATEGY_FUNC_T hal_poe_setDevicePowerStrategy;
    HAL_POE_DEVICE_GETPOWERLIMIT_FUNC_T    hal_poe_getDevicePowerLimit;
    HAL_POE_DEVICE_SETPOWERLIMIT_FUNC_T    hal_poe_setDevicePowerLimit;
    HAL_POE_DEVICE_GETERRORDELAY_FUNC_T    hal_poe_getDeviceErrorDelay;
    HAL_POE_DEVICE_SETERRORDELAY_FUNC_T    hal_poe_setDeviceErrorDelay;
    HAL_POE_DEVICE_GETMEASUREMENT_FUNC_T   hal_poe_getDeviceMeasurement;
    HAL_POE_PORT_GETPORTPOWERUP_FUNC_T     hal_poe_getPortPowerUp; /* get multiple ports */
    HAL_POE_PORT_TRIGGERPOWERUP_FUNC_T     hal_poe_triggerPortPowerUp;
    HAL_POE_PORT_GETPAIRMODE_FUNC_T        hal_poe_getPortPairMode;
    HAL_POE_PORT_SETPAIRMODE_FUNC_T        hal_poe_setPortPairMode;
    HAL_POE_PORT_GETTYPE_FUNC_T            hal_poe_getPortType;
    HAL_POE_PORT_SETTYPE_FUNC_T            hal_poe_setPortType;
    HAL_POE_PORT_GETPSE_FUNC_T             hal_poe_getPortPse;
    HAL_POE_PORT_SETPSE_FUNC_T             hal_poe_setPortPse;
    HAL_POE_PORT_GETPRIORITY_FUNC_T        hal_poe_getPortPriority;
    HAL_POE_PORT_SETPRIORITY_FUNC_T        hal_poe_setPortPriority;
    HAL_POE_PORT_GETCURRENTLIMIT_FUNC_T    hal_poe_getPortCurrentLimit;
    HAL_POE_PORT_SETCURRENTLIMIT_FUNC_T    hal_poe_setPortCurrentLimit;
    HAL_POE_PORT_GETMEASUREMENT_FUNC_T     hal_poe_getPortMeasurement; /* get multiple ports */
    HAL_POE_PORT_GETSTATUS_FUNC_T          hal_poe_getPortStatus;      /* get multiple ports */
    HAL_POE_PORT_GETEVENT_FUNC_T           hal_poe_getPortEvent;       /* get multiple ports */
} HAL_POE_DRIVER_T;

typedef AIR_ERROR_NO_T (*HAL_POE_DRIVER_FUNC_T)(
    HAL_POE_DRIVER_T **pptr_hal_poe_driver);

typedef struct
{
    UI16_T                poe_dev_id;      /* POE device ID, used to find corresponding driver */
    HAL_POE_DRIVER_FUNC_T poe_driver_func; /* POE driver handler function pointer */
} HAL_POE_DRIVER_MAP_T;

typedef struct HAL_POE_CHIP_INFO_S
{
    UI8_T  i2c_bus_id;
    UI8_T  i2c_slave_addr;
    UI16_T device_id;
    UI16_T revision_id;
} HAL_POE_CHIP_INFO_T;

typedef struct HAL_POE_CB_S
{
    HAL_POE_CHIP_INFO_T poe_chip_info;
    UI32_T             *ptr_poe_port_map_info; /* poe chip port to air port map info */
    HAL_POE_DRIVER_T   *driver;
} HAL_POE_CB_T;

typedef struct HAL_POE_CHIP_CB_S
{
    AIR_SEMAPHORE_ID_T hw_tbl_mutex;
} HAL_POE_CHIP_CB_T;

typedef struct HAL_POE_MON_DEV_PORT_INFO_S
{
    UI16_T current;
    UI16_T voltage;
    UI16_T temperature;
    UI16_T power_event;
    UI8_T  pd_signature  : 3;
    UI8_T  pd_class      : 4;
    UI8_T  state_machine : 1;
    UI8_T  power_status  : 1;
} HAL_POE_MON_DEV_PORT_INFO_T;

typedef struct HAL_POE_MON_DEV_INFO_S
{
    HAL_POE_MON_DEV_PORT_INFO_T *ptr_port_info;
} HAL_POE_MON_DEV_INFO_T;

typedef struct HAL_POE_MON_CB_S
{
    AIR_THREAD_ID_T         thread_id;
    HAL_POE_MON_DEV_INFO_T *ptr_dev_info;
} HAL_POE_MON_CB_T;

/* GLOBAL VARIABLE EXTERN DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const UI8_T  bit_size);
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
    UI32_T      *ptr_result);

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
    const UI32_T offset);

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
    UI8_T       *ptr_offset);

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
    UI8_T       *ptr_offset);

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
    BOOL_T      *ptr_state);

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
    const UI32_T unit);

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
    const UI32_T unit);

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
    const UI32_T unit);

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
    const UI32_T unit);

/* FUNCTION NAME:   hal_poe_init
 * PURPOSE:
 *      This API is used to init POE.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_init(
    const UI32_T unit);

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
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_poe_deinit(
    const UI32_T unit);

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
    UI32_T      *ptr_val);

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
    UI32_T      *ptr_val);

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
    const UI32_T device);

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
    const UI32_T device);

/* FUNCTION NAME:   hal_poe_getPortPowerUp
 * PURPOSE:
 *      Get state machine of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      air_port                -- port id
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
    BOOL_T                     *ptr_ready);

/* FUNCTION NAME:   hal_poe_getPortMeasurement
 * PURPOSE:
 *      Get measurement of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      air_port                -- port id
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
    AIR_POE_PORT_MEASUREMENT_T *ptr_meas);

/* FUNCTION NAME:   hal_poe_getPortStatus
 * PURPOSE:
 *      Get status of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      air_port                -- port id
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
    AIR_POE_PORT_STATUS_T      *ptr_ps);

/* FUNCTION NAME:   hal_poe_getPortEvent
 * PURPOSE:
 *      Get event of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      air_port                -- port id
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
    AIR_POE_EVENT_T            *ptr_event);

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
    const AIR_POE_ALTERNATIVE_T alt);

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
    UI16_T      *ptr_value);

#endif /* End of HAL_POE_H */
