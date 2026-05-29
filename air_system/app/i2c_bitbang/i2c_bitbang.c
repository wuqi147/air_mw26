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
#include <i2c_bitbang.h>

#include <air_cfg.h>
#include <air_error.h>
#include <air_perif.h>
#include <air_types.h>
#include <osal/osal.h>

#define I2C_MSG_WRITE   (0x00)
#define I2C_MSG_READ    (0x01)
#define I2C_MSG_NSTART  (0x02)
#define I2C_MSG_REVERSE (0x04)

#define I2C_BITBANG_DELAY_US (2)

#ifdef I2C_BITBANG_ERR_SHOW
#define I2C_BITBANG_ERR(fmt, ...) osal_printf("[%s]: " fmt, __func__, ##__VA_ARGS__)
#else
#define I2C_BITBANG_ERR(fmt, ...)
#endif

#define I2C_BITBANG_TAKE_SEMA(unit, channel)                                                      \
    ({                                                                                            \
        AIR_ERROR_NO_T      __rc = AIR_E_OK;                                                      \
        AIR_SEMAPHORE_ID_T *__ptr_sema = &_ptr_i2c_bitbang_cb[unit]->ptr_sema[channel];           \
        __rc = osal_takeSemaphore(__ptr_sema, AIR_SEMAPHORE_WAIT_FOREVER);                        \
        if (AIR_E_OK != __rc)                                                                     \
        {                                                                                         \
            I2C_BITBANG_ERR("chip %u takes i2c-bitbang semaphore failed, rc = %d\n", unit, __rc); \
            return __rc;                                                                          \
        }                                                                                         \
    })

#define I2C_BITBANG_GIVE_SEMA(unit, channel)                                                      \
    ({                                                                                            \
        AIR_ERROR_NO_T      __rc = AIR_E_OK;                                                      \
        AIR_SEMAPHORE_ID_T *__ptr_sema = &_ptr_i2c_bitbang_cb[unit]->ptr_sema[channel];           \
        __rc = osal_giveSemaphore(__ptr_sema);                                                    \
        if (AIR_E_OK != __rc)                                                                     \
        {                                                                                         \
            I2C_BITBANG_ERR("chip %u gives i2c-bitbang semaphore failed, rc = %d\n", unit, __rc); \
            return __rc;                                                                          \
        }                                                                                         \
    })

#define I2C_BITBANG_CHECK_UNIT(__unit__)                 \
    do                                                   \
    {                                                    \
        if (__unit__ > AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM) \
        {                                                \
            return AIR_E_BAD_PARAMETER;                  \
        }                                                \
    } while (0)

#define I2C_BITBANG_CHECK_PTR(__ptr__) \
    do                                 \
    {                                  \
        if (NULL == __ptr__)           \
        {                              \
            return AIR_E_NOT_INITED;   \
        }                              \
    } while (0)

#define I2C_BITBANG_CHECK_PARAM(__ptr_param__)                         \
    do                                                                 \
    {                                                                  \
        if ((NULL == __ptr_param__) || (0 == __ptr_param__->data_len)) \
        {                                                              \
            return AIR_E_BAD_PARAMETER;                                \
        }                                                              \
    } while (0)

typedef struct I2C_BITBANG_MSG_S
{
    UI8_T  slave_id;
    UI8_T  flag;
    UI16_T len;
    UI8_T *ptr_buf;
} I2C_BITBANG_MSG_T;

typedef struct I2C_BITBANG_PIN_S
{
    UI16_T bus;
    UI8_T  sda_pin;
    UI8_T  scl_pin;
} I2C_BITBANG_PIN_T;

typedef struct I2C_BITBANG_CB_S
{
    UI32_T              count;
    I2C_BITBANG_PIN_T  *ptr_pins;
    AIR_SEMAPHORE_ID_T *ptr_sema;
} I2C_BITBANG_CB_T;

/*
 * Config this structure
 */
static I2C_BITBANG_CB_T *_ptr_i2c_bitbang_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/*
 * Basic Operation
 * */
static inline AIR_ERROR_NO_T
_i2c_bitbang_setPin(
    const UI32_T unit,
    const UI32_T pin,
    const I32_T  data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Note: GPIO Open drain
     *
     * We cannot to set gpio to high by controlling gpio data register
     * because of the property of open drain, the only way we can do is
     * set the gpio pin to input mode, the i2c slave will set it high
     *
     * On the contrary, if the data is low, we need to set low by controll
     * the data register.
     */
    if (data)
    {
        rc = air_perif_setGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_INPUT);
    }
    else
    {
        rc = air_perif_setGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
        if (AIR_E_OK == rc)
        {
            /* set gpio output data */
            rc = air_perif_setGpioOutputData(unit, pin, AIR_PERIF_GPIO_DATA_LOW);
        }
    }

    return rc;
}

static inline AIR_ERROR_NO_T
_i2c_bitbang_getPin(
    const UI32_T unit,
    const UI32_T pin,
    I32_T       *ptr_data)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* set gpio to input mode */
    rc = air_perif_setGpioDirection(unit, pin, AIR_PERIF_GPIO_DIRECTION_INPUT);
    if (AIR_E_OK == rc)
    {
        rc = air_perif_getGpioInputData(unit, pin, (AIR_PERIF_GPIO_DATA_T *)ptr_data);
    }

    return rc;
}

static inline void
_i2c_bitbang_setSdaLow(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    _i2c_bitbang_setPin(unit, ptr_master->sda_pin, 0);
    osal_delayUs(I2C_BITBANG_DELAY_US >> 1);
}

static inline void
_i2c_bitbang_setSdaHigh(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    _i2c_bitbang_setPin(unit, ptr_master->sda_pin, 1);
    osal_delayUs(I2C_BITBANG_DELAY_US >> 1);
}

static inline void
_i2c_bitbang_setSclLow(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    _i2c_bitbang_setPin(unit, ptr_master->scl_pin, 0);
    osal_delayUs(I2C_BITBANG_DELAY_US >> 1);
}

static inline void
_i2c_bitbang_setSclHigh(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    _i2c_bitbang_setPin(unit, ptr_master->scl_pin, 1);
    osal_delayUs(I2C_BITBANG_DELAY_US);
}

/*
 * i2c protocol
 * */
static void
_i2c_bitbang_start(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    /* Note: i2c protocol begin as start condition
     * Assert: SDA and SCL must be high first
     * the start condition is a high to low trasmission on the
     * SDA line while SCL is high
     *
     * Therefore, we need to set the sda to low first.
     */
    _i2c_bitbang_setPin(unit, ptr_master->sda_pin, 0);
    osal_delayUs(I2C_BITBANG_DELAY_US);
    _i2c_bitbang_setSclLow(unit, ptr_master);
}

static void
_i2c_bitbang_repstart(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    /* Note: repeated start is functional identical as start
     * However, repeated start is always used in transmission without stop
     * Thus, we need to set the bus line to high first and avoid trigger
     * stop condition simutaneously.
     *
     * So the step set the sda and scl to high, sda must be first
     * then do the start condition again.
     */
    _i2c_bitbang_setSdaHigh(unit, ptr_master);
    _i2c_bitbang_setSclHigh(unit, ptr_master);
    _i2c_bitbang_setPin(unit, ptr_master->sda_pin, 0);
    osal_delayUs(I2C_BITBANG_DELAY_US);
    _i2c_bitbang_setSclLow(unit, ptr_master);
}

static void
_i2c_bitbang_stop(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    /* Note: i2c protocol begin as stop condition
     * the start condition is a low to high trasmission on the
     * SDA line while SCL is high
     *
     * Therefore, we need to set the scl to high first.
     */
    _i2c_bitbang_setSdaLow(unit, ptr_master);
    _i2c_bitbang_setSclHigh(unit, ptr_master);
    _i2c_bitbang_setPin(unit, ptr_master->sda_pin, 1);
    osal_delayUs(I2C_BITBANG_DELAY_US);
}

/**
 * returns:
 * 1 if slave device ack, 0 as nack
 */
static I32_T
_i2c_bitbang_sendByte(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master,
    const UI8_T              c)
{
    I32_T i = 7, ack = 0, sb = 0;

    /* Note: i2c protocol transfer with the most significant bit first
     * Assert: SCL must be low here !!!
     */
    for (; i >= 0; i--)
    {
        sb = (c >> i) & 1;
        _i2c_bitbang_setPin(unit, ptr_master->sda_pin, sb);
        osal_delayUs(I2C_BITBANG_DELAY_US >> 1);
        _i2c_bitbang_setSclHigh(unit, ptr_master);
        _i2c_bitbang_setSclLow(unit, ptr_master);
    }

    _i2c_bitbang_setSdaHigh(unit, ptr_master);

    /* next period */
    _i2c_bitbang_setSclHigh(unit, ptr_master);

    /* check ack */
    _i2c_bitbang_getPin(unit, ptr_master->sda_pin, &ack);

    _i2c_bitbang_setSclLow(unit, ptr_master);
    return !ack;
}

/**
 * In this function, we only support 8 bit address
 */
static AIR_ERROR_NO_T
_i2c_bitbang_sendAddr(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master,
    I2C_BITBANG_MSG_T       *ptr_msg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    I32_T          ret;
    UI8_T          addr = (ptr_msg->slave_id << 1) |
                 (ptr_msg->flag & I2C_MSG_READ);

    ret = _i2c_bitbang_sendByte(unit, ptr_master, addr);
    if (ret != 1)
    {
        I2C_BITBANG_ERR("slave_id send failed, no ack\n");
        rc = AIR_E_OTHERS;
    }

    return rc;
}

/**
 * returns:
 * ret > 0: return the length of successful trasmission
 * ret < 0: return the error code of transmission
 */
static I32_T
_i2c_bitbang_sendBytes(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master,
    I2C_BITBANG_MSG_T       *ptr_msg)
{
    const UI8_T *temp = ptr_msg->ptr_buf;
    UI32_T       count = ptr_msg->len, wrcount = 0, ret = 0;

    while (count > 0)
    {
        ret = _i2c_bitbang_sendByte(unit, ptr_master, *temp);
        if (ret > 0)
        {
            temp++;
            count--;
            wrcount++;
        }
        else
        {
            I2C_BITBANG_ERR("transmission failed, no ack\n");
            return -1;
        }
    }
    return wrcount;
}

static I32_T
_i2c_bitbang_recvByte(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master)
{
    I32_T  i = 0, tmp = 0;
    UI32_T ib = 0;

    /* Here the SCL pin must be low */
    _i2c_bitbang_setSdaHigh(unit, ptr_master);
    for (; i <= 7; i++)
    {
        _i2c_bitbang_setSclHigh(unit, ptr_master);
        ib <<= 1;
        _i2c_bitbang_getPin(unit, ptr_master->sda_pin, &tmp);
        if (1 == tmp)
        {
            ib |= 0x01;
        }
        _i2c_bitbang_setPin(unit, ptr_master->scl_pin, 0);
        osal_delayUs((i == 7) ? (I2C_BITBANG_DELAY_US >> 1) : I2C_BITBANG_DELAY_US);
    }

    /* Here, SCL is low */
    return ib;
}

static I32_T
_i2c_bitbang_recvBytes(
    const UI32_T             unit,
    const I2C_BITBANG_PIN_T *ptr_master,
    I2C_BITBANG_MSG_T       *ptr_msg)
{
    I32_T  inval, rdcount = 0, count = ptr_msg->len;
    UI8_T *temp = ptr_msg->ptr_buf;

    while (count > 0)
    {
        inval = _i2c_bitbang_recvByte(unit, ptr_master);
        *temp = inval;
        rdcount++, temp++, count--;

        /* after the byte we get, we need to return the ack here
         * if count == 0, that is the last one, we need to send nack
         */
        if (count)
        {
            _i2c_bitbang_setSdaLow(unit, ptr_master);
        }
        else
        {
            _i2c_bitbang_setSdaHigh(unit, ptr_master);
        }
        osal_delayUs(I2C_BITBANG_DELAY_US >> 1);
        _i2c_bitbang_setSclHigh(unit, ptr_master);
        osal_delayUs(I2C_BITBANG_DELAY_US);
        _i2c_bitbang_setSclLow(unit, ptr_master);
    }

    return rdcount;
}

/* Reverse the message here for the endian issue
 */
static void
_i2c_bitbang_reverseMsg(
    const UI32_T       unit,
    I2C_BITBANG_MSG_T *ptr_msg)
{
    I32_T start = 0, end = ptr_msg->len - 1;
    while (end > start)
    {
        ptr_msg->ptr_buf[start] ^= ptr_msg->ptr_buf[end];
        ptr_msg->ptr_buf[end] ^= ptr_msg->ptr_buf[start];
        ptr_msg->ptr_buf[start] ^= ptr_msg->ptr_buf[end];

        start++, end--;
    }
}

/* Core function
 * return:
 *  AIR_E_OTHERS       : send slave id error
 *  AIR_E_OP_INCOMPLETE: the trasmission is error
 * Note:
 *  We do not support 10 bit address
 * */
AIR_ERROR_NO_T
_i2c_bitbang_xfer(
    const UI32_T       unit,
    I2C_BITBANG_PIN_T *ptr_master,
    I2C_BITBANG_MSG_T *ptr_msgs,
    UI32_T             count)
{
    I32_T              i = 0, ret = 0, rc = AIR_E_OK;
    I2C_BITBANG_MSG_T *ptr_msg;

    _i2c_bitbang_start(unit, ptr_master);

    for (; i < count; i++)
    {
        ptr_msg = &ptr_msgs[i];

        if (!(ptr_msg->flag & I2C_MSG_NSTART))
        {
            if (i)
            {
                _i2c_bitbang_repstart(unit, ptr_master);
            }

            rc = _i2c_bitbang_sendAddr(unit, ptr_master, ptr_msg);
            if (rc != AIR_E_OK)
            {
                I2C_BITBANG_ERR("put address failed\n");
                break;
            }
        }

        if (ptr_msg->flag & I2C_MSG_READ)
        {
            ret = _i2c_bitbang_recvBytes(unit, ptr_master, ptr_msg);
            if (ret != ptr_msg->len)
            {
                I2C_BITBANG_ERR("read bytes length error, get length %d, expect %d",
                                ret, ptr_msg->len);
                rc = AIR_E_OP_INCOMPLETE;
                break;
            }
        }
        else
        {
            if (ptr_msg->flag & I2C_MSG_REVERSE)
            {
                /* reverse data for endian issue */
                _i2c_bitbang_reverseMsg(unit, ptr_msg);
            }

            ret = _i2c_bitbang_sendBytes(unit, ptr_master, ptr_msg);
            if (ret != ptr_msg->len)
            {
                I2C_BITBANG_ERR("write bytes length error, get length %d, expect %d",
                                ret, ptr_msg->len);
                rc = AIR_E_OP_INCOMPLETE;
                break;
            }
        }
    }

    _i2c_bitbang_stop(unit, ptr_master);

    return rc;
}

static I2C_BITBANG_PIN_T *
_i2c_bitbang_getMaster(
    const UI32_T unit,
    const UI32_T channel)
{
    if ((_ptr_i2c_bitbang_cb[unit] == NULL) || (_ptr_i2c_bitbang_cb[unit]->count == 0))
    {
        I2C_BITBANG_ERR("_ptr_i2c_bitbang_cb[%u] does not init\n", unit);
        return NULL;
    }

    if (channel < _ptr_i2c_bitbang_cb[unit]->count)
    {
        return &_ptr_i2c_bitbang_cb[unit]->ptr_pins[channel];
    }
    else
    {
        I2C_BITBANG_ERR("Bad channel number(%u), valid channel number is 0 ", channel);

        if (1 == _ptr_i2c_bitbang_cb[unit]->count)
        {
            osal_printf("\n");
        }
        else
        {
            osal_printf("- %u\n", _ptr_i2c_bitbang_cb[unit]->count - 1);
        }

        return NULL;
    }
}

/*
 * EXPORT APIs
 * */

/* FUNCTION NAME:   i2c_bitbang_init
 * PURPOSE:
 *      Initialize the I2C bit-bang feature
 * INPUT:
 *      unit                 -- Device unit number
 *      count                -- Count of config array
 *      ptr_cfg              -- Pointer of i2c bitbang pin config
 *                              I2C_BITBANG_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_ALREADY_INITED -- Init func was called
 *      AIR_E_NO_MEMORY      -- Malloc failed
 *      AIR_E_BAD_PARAMETER  -- Customer_ref not config
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_init(
    const UI32_T       unit,
    const UI32_T       count,
    I2C_BITBANG_CFG_T *ptr_cfg)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              i = 0, j = 0, size = 0;

    AIR_CFG_VALUE_T     gpio_cfg;
    AIR_SEMAPHORE_ID_T *ptr_i2c_bitbang_sema_id;

    I2C_BITBANG_CHECK_UNIT(unit);
    I2C_BITBANG_CHECK_PTR(ptr_cfg);

    if (_ptr_i2c_bitbang_cb[unit] != NULL)
    {
        I2C_BITBANG_ERR("repeated init\n");
        return AIR_E_ALREADY_INITED;
    }

    size = sizeof(I2C_BITBANG_CB_T);
    _ptr_i2c_bitbang_cb[unit] = osal_alloc(size, "i2c bitbang");
    if (NULL == _ptr_i2c_bitbang_cb[unit])
    {
        I2C_BITBANG_ERR("malloc control block failed, required size = %u\n", size);
        rc = AIR_E_NO_MEMORY;
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(_ptr_i2c_bitbang_cb[unit], 0x00, size);

        for (; i < count; i++)
        {
            if (unit != ptr_cfg[i].unit)
            {
                continue;
            }

            /* get the unit pins to malloc it */
            _ptr_i2c_bitbang_cb[unit]->count++;
        }

        size = _ptr_i2c_bitbang_cb[unit]->count * sizeof(AIR_SEMAPHORE_ID_T);
        ptr_i2c_bitbang_sema_id = osal_alloc(size, "i2c bitbang");
        if (NULL == ptr_i2c_bitbang_sema_id)
        {
            I2C_BITBANG_ERR("malloc semaphore failed, required size = %u\n", size);
            rc = AIR_E_NO_MEMORY;
        }
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(ptr_i2c_bitbang_sema_id, 0, size);

        _ptr_i2c_bitbang_cb[unit]->ptr_sema = ptr_i2c_bitbang_sema_id;
        rc = osal_createSemaphore("I2C_BITBANG", AIR_SEMAPHORE_BINARY,
                                  _ptr_i2c_bitbang_cb[unit]->ptr_sema, "i2c-bitbang");
    }

    /* malloc the pins */
    if (AIR_E_OK == rc)
    {
        size = _ptr_i2c_bitbang_cb[unit]->count * sizeof(I2C_BITBANG_PIN_T);
        _ptr_i2c_bitbang_cb[unit]->ptr_pins = osal_alloc(size, "i2c bitbang");
        if (NULL == _ptr_i2c_bitbang_cb[unit]->ptr_pins)
        {
            I2C_BITBANG_ERR("malloc pins failed, required size = %u\n", size);
            rc = AIR_E_NO_MEMORY;
        }
    }

    if (AIR_E_OK == rc)
    {
        osal_memset(_ptr_i2c_bitbang_cb[unit]->ptr_pins, 0x00, size);
    }

    for (i = 0; (i < count) && (AIR_E_OK == rc) &&
                (j < _ptr_i2c_bitbang_cb[unit]->count);
         i++)
    {
        if (unit != ptr_cfg[i].unit)
        {
            continue;
        }

        /* check the sda_pin and scl_pin are GPIO */
        gpio_cfg.param0 = ptr_cfg[i].sda_pin;
        gpio_cfg.param1 = 0;
        gpio_cfg.value = 0;

        rc = air_cfg_getValue(0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN, &gpio_cfg);
        if (gpio_cfg.value != 1)
        {
            I2C_BITBANG_ERR("sda_pin does not config gpio\n");
            rc = AIR_E_BAD_PARAMETER;
        }

        if (AIR_E_OK == rc)
        {
            _ptr_i2c_bitbang_cb[unit]->ptr_pins[j].sda_pin = ptr_cfg[i].sda_pin;

            gpio_cfg.param0 = ptr_cfg[i].scl_pin;
            gpio_cfg.value = 0;
            rc = air_cfg_getValue(0, AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN, &gpio_cfg);
            if (gpio_cfg.value != 1)
            {
                I2C_BITBANG_ERR("scl_pin does not config gpio\n");
                rc = AIR_E_BAD_PARAMETER;
            }
        }

        /* set these gpio pin to input direction */
        if (AIR_E_OK == rc)
        {
            _ptr_i2c_bitbang_cb[unit]->ptr_pins[j].scl_pin = ptr_cfg[i].scl_pin;

            rc = air_perif_setGpioDirection(unit, ptr_cfg[i].scl_pin,
                                            AIR_PERIF_GPIO_DIRECTION_INPUT);
        }

        if (AIR_E_OK == rc)
        {
            rc = air_perif_setGpioDirection(unit, ptr_cfg[i].sda_pin,
                                            AIR_PERIF_GPIO_DIRECTION_INPUT);
        }

        j++;
    }

    if (rc != AIR_E_OK)
    {
        i2c_bitbang_deinit(unit);
    }

    return rc;
}

/* FUNCTION NAME:   i2c_bitbang_deinit
 * PURPOSE:
 *      De-initialize the I2C bit-bang feature
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_deinit(
    const UI32_T unit)
{
    I2C_BITBANG_CHECK_UNIT(unit);

    if (_ptr_i2c_bitbang_cb[unit]->ptr_pins != NULL)
    {
        osal_free(_ptr_i2c_bitbang_cb[unit]->ptr_pins);
    }

    if (_ptr_i2c_bitbang_cb[unit]->ptr_sema != NULL)
    {
        osal_free(_ptr_i2c_bitbang_cb[unit]->ptr_sema);
    }

    if (_ptr_i2c_bitbang_cb[unit] != NULL)
    {
        osal_free(_ptr_i2c_bitbang_cb[unit]);
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   i2c_bitbang_write
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang write operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- No ack.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_NOT_INITED     -- I2C bus is not inited.
 *      AIR_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_write(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI16_T         slave_id,
    I2C_BITBANG_PARAM_T *ptr_param)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    I2C_BITBANG_PIN_T *ptr_master = _i2c_bitbang_getMaster(unit, channel);
    I2C_BITBANG_MSG_T  i2c_msgs[2];

    I2C_BITBANG_CHECK_UNIT(unit);
    I2C_BITBANG_CHECK_PTR(ptr_master);
    I2C_BITBANG_CHECK_PARAM(ptr_param);

    i2c_msgs[0].slave_id = slave_id;
    i2c_msgs[0].flag = I2C_MSG_WRITE | I2C_MSG_REVERSE;
    i2c_msgs[0].len = ptr_param->addr_len;
    i2c_msgs[0].ptr_buf = (UI8_T *)&ptr_param->addr;

    i2c_msgs[1].slave_id = slave_id;
    i2c_msgs[1].flag = I2C_MSG_WRITE | I2C_MSG_NSTART;
    i2c_msgs[1].len = ptr_param->data_len;

    if (4 < ptr_param->data_len)
    {
        i2c_msgs[1].ptr_buf = (UI8_T *)ptr_param->info.ptr_data;
    }
    else
    {
        i2c_msgs[1].ptr_buf = (UI8_T *)&ptr_param->info.data;
    }

    I2C_BITBANG_TAKE_SEMA(unit, channel);
    rc = _i2c_bitbang_xfer(unit, ptr_master, i2c_msgs, 2);
    I2C_BITBANG_GIVE_SEMA(unit, channel);

    return rc;
}

/* FUNCTION NAME:   i2c_bitbang_read
 * PURPOSE:
 *      This API is used to do the I2C-Bitbang read operation
 * INPUT:
 *      unit                 -- Device unit number
 *      channel              -- i2c bitbang channel
 *      slave-id             -- target device id
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * OUTPUT:
 *      ptr_param            -- Pointer of i2c parameter
 *                              I2C_BITBANG_PARAM_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_OTHERS         -- No ack.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 *      AIR_E_NOT_INITED     -- I2C bus is not inited.
 *      AIR_E_OP_INCOMPLETE  -- I2C transmission is error
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
i2c_bitbang_read(
    const UI32_T         unit,
    const UI16_T         channel,
    const UI16_T         slave_id,
    I2C_BITBANG_PARAM_T *ptr_param)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    I2C_BITBANG_PIN_T *ptr_master = _i2c_bitbang_getMaster(unit, channel);
    I2C_BITBANG_MSG_T  i2c_msgs[2];

    I2C_BITBANG_CHECK_UNIT(unit);
    I2C_BITBANG_CHECK_PTR(ptr_master);
    I2C_BITBANG_CHECK_PARAM(ptr_param);

    i2c_msgs[0].slave_id = slave_id;
    i2c_msgs[0].flag = I2C_MSG_WRITE | I2C_MSG_REVERSE;
    i2c_msgs[0].len = ptr_param->addr_len;
    i2c_msgs[0].ptr_buf = (UI8_T *)&ptr_param->addr;

    i2c_msgs[1].slave_id = slave_id;
    i2c_msgs[1].flag = I2C_MSG_READ;
    i2c_msgs[1].len = ptr_param->data_len;

    if (4 < ptr_param->data_len)
    {
        i2c_msgs[1].ptr_buf = (UI8_T *)ptr_param->info.ptr_data;
    }
    else
    {
        i2c_msgs[1].ptr_buf = (UI8_T *)&ptr_param->info.data;
    }

    I2C_BITBANG_TAKE_SEMA(unit, channel);
    if (0 == ptr_param->addr_len)
    {
        rc = _i2c_bitbang_xfer(unit, ptr_master, &i2c_msgs[1], 1);
    }
    else
    {
        rc = _i2c_bitbang_xfer(unit, ptr_master, i2c_msgs, 2);
    }
    I2C_BITBANG_GIVE_SEMA(unit, channel);

    return rc;
}
