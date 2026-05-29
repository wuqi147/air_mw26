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
#ifdef AIR_EN_SFP_LED

/* FILE NAME:  sfp_led.c
 * PURPOSE:
 *      It provide SFP LED application API.
 * NOTES:
 */

#include <sfp_led.h>

#include <air_cfg.h>
#include <air_mib.h>
#include <air_perif.h>
#include <air_port.h>
#include <customer_ref.h>
#include <osal/osal.h>

/* MACRO FUNCTION DECLARATIONS
 */
#define SFP_LED_DBG_INFO_ENABLE (0)
#define SFP_LED_DBG_INTERVAL    (10) /* For debug use, default print debug message \
                                       every 5 seconds to prevent log flooding */

#if SFP_LED_DBG_INFO_ENABLE
#define SFP_LED_DBG_INFO(i, fmt, ...)        \
    do                                       \
    {                                        \
        if (0 == (i % SFP_LED_DBG_INTERVAL)) \
        {                                    \
            osal_printf(fmt, ##__VA_ARGS__); \
        }                                    \
    } while (0)
#else
#define SFP_LED_DBG_INFO(i, fmt, ...)
#endif

#define SFP_LED_DBG_ERROR(i, fmt, ...)       \
    do                                       \
    {                                        \
        if (0 == (i % SFP_LED_DBG_INTERVAL)) \
        {                                    \
            osal_printf(fmt, ##__VA_ARGS__); \
        }                                    \
    } while (0)

/* GLOBAL VARIABLE DECLARATIONS
 */

/* SFP LED application control block, init when application initialization */
static SFP_LED_CB_T
    *_ptr_sfp_led_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* FUNCTION NAME:   _sfp_led_checkGpioAutoPatt
 * PURPOSE:
 *      1. Check if the current GPIO auto pattern is the same as the pattern to be set.
 *      2. Set GPIO output state if necessary
 * INPUT:
 *      pattern           --  Auto pattern to be set
 * OUTPUT:
 *      gpio_output_state --  GPIO output state after pattern setting
 * RETURN
 *      BOOL              --  True: current GPIO auto pattern is the same as the pattern to be set
 * NOTES:
 *      None
 */
static BOOL_T
_sfp_led_checkGpioAutoPatt(
    const AIR_PERIF_GPIO_PATT_T  pattern,
    SFP_LED_GPIO_OUTPUT_STATE_T *ptr_gpio_output_state)
{
    BOOL_T                      check_result = FALSE;
    SFP_LED_GPIO_OUTPUT_STATE_T gpio_output_auto_patt = SFP_LED_GPIO_OUTPUT_LAST;

    switch (pattern)
    {
        case AIR_PERIF_GPIO_PATT_HZ_HALF:
            gpio_output_auto_patt = SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_HALF;
            break;
        case AIR_PERIF_GPIO_PATT_HZ_ONE:
            gpio_output_auto_patt = SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_ONE;
            break;
        case AIR_PERIF_GPIO_PATT_HZ_TWO:
            gpio_output_auto_patt = SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_TWO;
            break;
        case AIR_PERIF_GPIO_PATT_HZ_EIGHT:
            gpio_output_auto_patt = SFP_LED_GPIO_OUTPUT_AUTO_PATT_HZ_EIGHT;
            break;
        default:
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Check GPIO Auto Patt, unknown gpio pattern (%d)\n", pattern);
            /* return TRUE then won't proceed to set GPIO auto mode and GPIO auto pattern */
            return TRUE;
    }

    check_result = (*ptr_gpio_output_state == gpio_output_auto_patt) ? TRUE : FALSE;
    if (FALSE == check_result)
    {
        *ptr_gpio_output_state = gpio_output_auto_patt;
    }

    return check_result;
}

/* FUNCTION NAME:   _sfp_led_checkGpioData
 * PURPOSE:
 *      1. Check if the current GPIO data is the same as the data to be set.
 *      2. Set GPIO output state if necessary
 * INPUT:
 *      data           --  GPIO data to be set
 * OUTPUT:
 *      gpio_output_state --  GPIO output state after data setting
 * RETURN
 *      BOOL              --  True: current GPIO data is the same as the data to be set
 * NOTES:
 *      None
 */
static BOOL_T
_sfp_led_checkGpioData(
    const AIR_PERIF_GPIO_DATA_T  data,
    SFP_LED_GPIO_OUTPUT_STATE_T *ptr_gpio_output_state)
{
    BOOL_T                      check_result = FALSE;
    SFP_LED_GPIO_OUTPUT_STATE_T gpio_output_data = SFP_LED_GPIO_OUTPUT_LAST;

    if (AIR_PERIF_GPIO_DATA_LOW == data)
    {
        gpio_output_data = SFP_LED_GPIO_OUTPUT_DATA_LOW;
    }
    else if (AIR_PERIF_GPIO_DATA_HIGH == data)
    {
        gpio_output_data = SFP_LED_GPIO_OUTPUT_DATA_HIGH;
    }
    else
    {
        SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Check GPIO Data, unknown gpio data (%d)\n", data);
        /* return TRUE then won't proceed to set GPIO auto mode and GPIO data */
        return TRUE;
    }

    check_result = (*ptr_gpio_output_state == gpio_output_data) ? TRUE : FALSE;
    if (FALSE == check_result)
    {
        *ptr_gpio_output_state = gpio_output_data;
    }

    return check_result;
}

/* FUNCTION NAME:   _sfp_led_turnLedBlink
 * PURPOSE:
 *      Blink the LED.
 * INPUT:
 *      unit            --  Device unit number
 *      gpio_pin_id     --  GPIO pin id
 *      led_pattern     --  LED pattern
 * OUTPUT:
 *      gpio_output_state --  GPIO output state after operation
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      Will check the GPIO output state first.
 *      If the LED pattern is the same as current state, won't invoke PERIF API.
 */
static AIR_ERROR_NO_T
_sfp_led_turnLedBlink(
    const UI32_T                 unit,
    const UI32_T                 gpio_pin_id,
    const AIR_PERIF_GPIO_PATT_T  led_pattern,
    SFP_LED_GPIO_OUTPUT_STATE_T *ptr_gpio_output_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    /* Set GPIO auto pattern
     * If the current GPIO auto pattern is the same as the pattern to be set, then skip */
    if (FALSE == _sfp_led_checkGpioAutoPatt(led_pattern, ptr_gpio_output_state))
    {
        rc = air_perif_setGpioOutputAutoMode(unit, gpio_pin_id, TRUE);
        if (AIR_E_OK == rc)
        {
            rc = air_perif_setGpioOutputAutoPatt(unit, gpio_pin_id, led_pattern);
            SFP_LED_DBG_INFO(0, "[SFP_LED] Set GPIO output auto pattern, gpio_pin_id %d, patt %d, result %d\n",
                             gpio_pin_id, led_pattern, rc);

            if (AIR_E_OK != rc)
            {
                SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Blink LED, set output auto pattern error(%d)\n", rc);
            }
        }
        else
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Blink LED, set output auto mode error(%d)\n", rc);
        }
    }
    return rc;
}

/* FUNCTION NAME:   _sfp_led_turnLedOff
 * PURPOSE:
 *      Turn off the LED.
 * INPUT:
 *      unit            --  Device unit number
 *      gpio_pin_id     --  GPIO pin id
 *      high_active     --  High active or not
 * OUTPUT:
 *      gpio_output_state --  GPIO output state after operation
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      Will check the GPIO output state first.
 *      If the LED pattern is the same, won't invoke PERIF API.
 */
static AIR_ERROR_NO_T
_sfp_led_turnLedOff(
    const UI32_T                 unit,
    const UI32_T                 gpio_pin_id,
    const UI32_T                 high_active,
    SFP_LED_GPIO_OUTPUT_STATE_T *ptr_gpio_output_state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    AIR_PERIF_GPIO_DATA_T gpio_data;

    if (TRUE == high_active)
    {
        gpio_data = AIR_PERIF_GPIO_DATA_LOW;
    }
    else
    {
        gpio_data = AIR_PERIF_GPIO_DATA_HIGH;
    }

    /* Set GPIO data
     * If the current GPIO data is the same as the data to be set, then skip */
    if (FALSE == _sfp_led_checkGpioData(gpio_data, ptr_gpio_output_state))
    {
        rc = air_perif_setGpioOutputAutoMode(unit, gpio_pin_id, FALSE);
        if (AIR_E_OK == rc)
        {
            rc = air_perif_setGpioOutputData(unit, gpio_pin_id, gpio_data);
            SFP_LED_DBG_INFO(0, "[SFP_LED] Set GPIO output data, gpio_pin_id %d, data (%d)\n",
                             gpio_pin_id, gpio_data);
            if (AIR_E_OK != rc)
            {
                SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Off LED, high_active %d, set GPIO data error(%d)\n",
                                  high_active, rc);
            }
        }
        else
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, Off LED, high_active %d, set GPIO auto mode error(%d)\n",
                              high_active, rc);
        }
    }
    return rc;
}

/* FUNCTION NAME:   _sfp_led_turnLedOn
 * PURPOSE:
 *      Turn on the LED.
 * INPUT:
 *      unit            --  Device unit number
 *      gpio_pin_id     --  GPIO pin id
 *      high_active     --  High active or not
 * OUTPUT:
 *      gpio_output_state --  GPIO output state after operation
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      Will check the GPIO output state first.
 *      If the LED pattern is the same, won't invoke PERIF API.
 */
static AIR_ERROR_NO_T
_sfp_led_turnLedOn(
    const UI32_T                 unit,
    const UI32_T                 gpio_pin_id,
    const UI32_T                 high_active,
    SFP_LED_GPIO_OUTPUT_STATE_T *ptr_gpio_output_state)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    AIR_PERIF_GPIO_DATA_T gpio_data;

    if (TRUE == high_active)
    {
        gpio_data = AIR_PERIF_GPIO_DATA_HIGH;
    }
    else
    {
        gpio_data = AIR_PERIF_GPIO_DATA_LOW;
    }

    /* Set GPIO data
     * If the current GPIO data is the same as the data to be set, then skip */
    if (FALSE == _sfp_led_checkGpioData(gpio_data, ptr_gpio_output_state))
    {
        rc = air_perif_setGpioOutputAutoMode(unit, gpio_pin_id, FALSE);
        if (AIR_E_OK == rc)
        {
            rc = air_perif_setGpioOutputData(unit, gpio_pin_id, gpio_data);
            SFP_LED_DBG_INFO(0, "[SFP_LED] Set GPIO output data, gpio_pin_id %d, data (%d)\n",
                             gpio_pin_id, gpio_data);
            if (AIR_E_OK != rc)
            {
                SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, On LED, high_active %d, set GPIO data error(%d)\n",
                                  high_active, rc);
            }
        }
        else
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, On LED, high_active %d, set GPIO auto mode error(%d)\n",
                              high_active, rc);
        }
    }
    return rc;
}

/* FUNCTION NAME:   _sfp_led_getLedBehavior
 * PURPOSE:
 *      Get LED on/off/blink behavior based on config
 * INPUT:
 *      speed            --  Speed of the port
 *      duplex           --  Duplex of the port
 *      port_cfg         --  LED behavior config of the port
 * OUTPUT:
 *      led_on_cfg       --  Led on/off
 *      led_blink_tx_cfg --  Led blink based on tx status
 *      led_blink_rx_cfg --  Led blink based on rx status
 * RETURN
 *      None
 * NOTES:
 *      None
 */
static void
_sfp_led_getLedBehavior(
    const AIR_PORT_SPEED_T  speed,
    const AIR_PORT_DUPLEX_T duplex,
    const UI32_T            port_cfg,
    UI32_T                 *ptr_led_on_cfg,
    UI32_T                 *ptr_led_blink_tx_cfg,
    UI32_T                 *ptr_led_blink_rx_cfg)
{
    switch (speed)
    {
        case AIR_PORT_SPEED_10M:
            *ptr_led_on_cfg = port_cfg & SFP_LED_FLAGS_LED_ON_LINK_UP_10M;
            *ptr_led_blink_tx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_TX_10M;
            *ptr_led_blink_rx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_RX_10M;
            break;
        case AIR_PORT_SPEED_100M:
            *ptr_led_on_cfg = port_cfg & SFP_LED_FLAGS_LED_ON_LINK_UP_100M;
            *ptr_led_blink_tx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_TX_100M;
            *ptr_led_blink_rx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_RX_100M;
            break;
        case AIR_PORT_SPEED_1000M:
            *ptr_led_on_cfg = port_cfg & SFP_LED_FLAGS_LED_ON_LINK_UP_1000M;
            *ptr_led_blink_tx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_TX_1000M;
            *ptr_led_blink_rx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_RX_1000M;
            break;
        case AIR_PORT_SPEED_2500M:
            *ptr_led_on_cfg = port_cfg & SFP_LED_FLAGS_LED_ON_LINK_UP_2500M;
            *ptr_led_blink_tx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_TX_2500M;
            *ptr_led_blink_rx_cfg = port_cfg & SFP_LED_FLAGS_LED_BLINK_RX_2500M;
            break;
        default:
            break;
    }

    switch (duplex)
    {
        case AIR_PORT_DUPLEX_HALF:
            *ptr_led_on_cfg = *ptr_led_on_cfg | (port_cfg & SFP_LED_FLAGS_LED_ON_DUPLEX_HALF);
            break;
        case AIR_PORT_DUPLEX_FULL:
            *ptr_led_on_cfg = *ptr_led_on_cfg | (port_cfg & SFP_LED_FLAGS_LED_ON_DUPLEX_FULL);
            break;
        default:
            break;
    }
}

static AIR_PERIF_GPIO_PATT_T
_sfp_led_portPattToPerifPatt(
    AIR_PORT_PHY_LED_PATT_T port_patt)
{
    switch (port_patt)
    {
        case AIR_PORT_PHY_LED_PATT_HZ_HALF:
            return AIR_PERIF_GPIO_PATT_HZ_HALF;
        case AIR_PORT_PHY_LED_PATT_HZ_ONE:
            return AIR_PERIF_GPIO_PATT_HZ_ONE;
        case AIR_PORT_PHY_LED_PATT_HZ_TWO:
            return AIR_PERIF_GPIO_PATT_HZ_TWO;
        default:
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, port patt to perif patt, unknown port patt(%d)\n", port_patt);
            return AIR_PERIF_GPIO_PATT_HZ_HALF;
        }
    }
}

#ifdef AIR_EN_SFP_LED_WITH_THREAD
/* FUNCTION NAME:   _sfp_led_thread
 * PURPOSE:
 *      SFP LED working thread.
 *      Control LED status based on link status.
 * INPUT:
 *      ptr_argv            --  Pointer for device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      None
 * NOTES:
 *      None
 */
static void
_sfp_led_thread(
    void *ptr_argv)
{
    UI32_T                unit = (UI32_T)(AIR_HUGE_T)ptr_argv;
    UI32_T                state = 0;
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    UI8_T                 i;     /* Loop index */
    UI32_T                j = 0; /* Thread infinite loop index */

    UI32_T                port = 0;
    UI32_T                port_cfg = 0; /* LED behavior config defined in customer_ref.c */
    UI32_T                gpio_pin_id = 0;
    UI32_T                led_id = 0;
    UI32_T                high_active = 0;      /* High active flag */

    UI32_T                led_on_cfg = 0;       /* decide to turn on led or not */
    UI32_T                led_blink_tx_cfg = 0; /* decide to blink led or not */
    UI32_T                led_blink_rx_cfg = 0; /* decide to blink led or not */

    AIR_PORT_STATUS_T     ps;                   /* port status get from air_port_getPortStatus */
    SFP_LED_PORT_STATUS_T sfp_led_ps;           /* port status get from port_status_func */
    AIR_MIB_CNT_RX_T      rx_cnt;               /* MIB Counters of Rx Event */
    AIR_MIB_CNT_TX_T      tx_cnt;               /* MIB Counters of Tx Event */

    UI16_T                rx_oct;               /* RX octets */
    UI16_T                tx_oct;               /* TX octets */

    for (j = 0;; j++)
    {
        osal_sleepTask(SFP_LED_THREAD_INTERVAL);

        for (i = 0; i < _ptr_sfp_led_cb[unit]->total_led_cfg_cnt; i++)
        {
            /* For each port, get the led behavior config and correspoding GPIO pin id */
            port = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].port;
            gpio_pin_id = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_pin_id;
            led_id = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].led_id;
            port_cfg = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].flags;

            /* Check if high active flag is defined */
            high_active = (port_cfg & SFP_LED_FLAGS_LED_ACTIVE_HIGH) ? TRUE : FALSE;

            /* Check if it's valid port id and GPIO pin id */
            if (0 != port && 0 != gpio_pin_id)
            {
                /* for each port check the control mode of the port first */
                rc = air_port_getPhyLedCtrlMode(unit, port, led_id, &state);

                if (AIR_E_OK == rc)
                {
                    SFP_LED_DBG_INFO(j, "[SFP_LED] - Unit %d, Port %d, GPIO pin id %d = %s mode\n",
                                     unit, port, gpio_pin_id,
                                     (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == state) ? "FORCE" : "PHY");

                    /* Force mode, control LED based on force state and force pattern */
                    if (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == state)
                    {
                        rc = air_port_getPhyLedForceState(unit, port, led_id, &state);
                        if (AIR_E_OK == rc)
                        {
                            SFP_LED_DBG_INFO(j, "[SFP_LED]    Port %d, led id %d, force state %d\n",
                                             port, led_id, state);

                            if (AIR_PORT_PHY_LED_STATE_FORCE_PATT == state)
                            {
                                rc = air_port_getPhyLedForcePattCfg(unit, port, led_id, &state);
                                if (AIR_E_OK == rc)
                                {
                                    SFP_LED_DBG_INFO(j, "[SFP_LED]    Port %d, led id %d, force pattern %d\n",
                                                     port, led_id, state);

                                    _sfp_led_turnLedBlink(unit, gpio_pin_id, _sfp_led_portPattToPerifPatt(state),
                                                          &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                                }
                                else
                                {
                                    SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d led_id=%d, force pattern error(%d)\n",
                                                      port, led_id, rc);
                                }
                            }
                            else if (AIR_PORT_PHY_LED_STATE_OFF == state)
                            {
                                _sfp_led_turnLedOff(unit, gpio_pin_id, FALSE,
                                                    &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                            }
                            else if (AIR_PORT_PHY_LED_STATE_ON == state)
                            {
                                _sfp_led_turnLedOn(unit, gpio_pin_id, FALSE,
                                                   &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                            }
                            else
                            {
                                SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d led_id=%d, unknown force state (%d)\n",
                                                  port, led_id, state);
                            }
                        }
                        else
                        {
                            SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d led_id=%d, get force state error(%d)\n",
                                              port, led_id, rc);
                        }
                    }
                    /* PHY mode, control LED based on link status */
                    else if (AIR_PORT_PHY_LED_CTRL_MODE_PHY == state)
                    {
                        if (NULL != _ptr_sfp_led_cb[unit]->port_status_func)
                        {
                            rc = _ptr_sfp_led_cb[unit]->port_status_func(unit, port, &sfp_led_ps);
                            ps.speed = sfp_led_ps.speed;
                            ps.flags = sfp_led_ps.flags & SFP_LED_PORT_STATUS_FLAGS_LINK_UP;
                            if (sfp_led_ps.flags & SFP_LED_PORT_STATUS_FLAGS_DUPLEX_FULL)
                            {
                                ps.duplex = AIR_PORT_DUPLEX_FULL;
                            }
                            else
                            {
                                ps.duplex = AIR_PORT_DUPLEX_HALF;
                            }
                        }
                        else
                        {
                            rc = air_port_getPortStatus(unit, port, &ps);
                        }

                        if (AIR_E_OK == rc)
                        {
                            /* Decide LED on/off/blink config based on link status */
                            _sfp_led_getLedBehavior(ps.speed, ps.duplex, port_cfg,
                                                    &led_on_cfg, &led_blink_tx_cfg, &led_blink_rx_cfg);

                            SFP_LED_DBG_INFO(j, "[SFP_LED]  Led behavior, port_cfg 0x%x, speed %d, dup %d,",
                                             port_cfg, ps.speed, ps.duplex);
                            SFP_LED_DBG_INFO(j, " on 0x%x, tx 0x%x, rx 0x%x\n",
                                             led_on_cfg, led_blink_tx_cfg, led_blink_rx_cfg);

                            /* link up and speed or duplex matches the config */
                            if ((ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) && led_on_cfg)
                            {
                                rc = air_mib_getPortCnt(unit, port, &rx_cnt, &tx_cnt);
                                rx_oct = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].rx_oct;
                                tx_oct = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].tx_oct;

                                SFP_LED_DBG_INFO(j, "[SFP_LED]     RX: Port %d, prev %d, next %d \n",
                                                 port, rx_oct, (UI16_T)rx_cnt.ROC);
                                SFP_LED_DBG_INFO(j, "[SFP_LED]     TX: Port %d, prev %d, next %d \n",
                                                 port, tx_oct, (UI16_T)tx_cnt.TOC);

                                if (AIR_E_OK == rc)
                                {
                                    /* If there is traffic and speed matcheds the config,
                                     * blink the LED */
                                    if (((rx_oct != (UI16_T)rx_cnt.ROC) && led_blink_rx_cfg) ||
                                        ((tx_oct != (UI16_T)tx_cnt.TOC) && led_blink_tx_cfg))
                                    {
                                        SFP_LED_DBG_INFO(j, "[SFP_LED]     Port %d blink\n", port);

                                        _sfp_led_turnLedBlink(unit, gpio_pin_id, AIR_PERIF_GPIO_PATT_HZ_EIGHT,
                                                              &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                                    }
                                    /* There is no traffic, or the speed not match, just turn on LED */
                                    else
                                    {
                                        SFP_LED_DBG_INFO(j, "[SFP_LED]     Port %d up\n", port);
                                        _sfp_led_turnLedOn(unit, gpio_pin_id, high_active,
                                                           &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                                    }
                                    _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].rx_oct = (UI16_T)rx_cnt.ROC;
                                    _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].tx_oct = (UI16_T)tx_cnt.TOC;
                                }
                                else
                                {
                                    SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d get mib counter error(%d)\n",
                                                      port, rc);
                                }
                            }
                            /* Link down, or the speed and duplex not match, turn off LED */
                            else
                            {
                                SFP_LED_DBG_INFO(j, "[SFP_LED]     Port %d down\n", port);
                                _sfp_led_turnLedOff(unit, gpio_pin_id, high_active,
                                                    &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                            }
                        }
                        else
                        {
                            SFP_LED_DBG_INFO(j, "[SFP_LED]*Error*, port=%d get link status error(%d)\n",
                                             port, rc);
                        }
                    }
                    else
                    {
                        SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d led id=%d, unknown control mode(%d)\n",
                                          port, led_id, state);
                    }
                }
                else
                {
                    SFP_LED_DBG_ERROR(j, "[SFP_LED]*Error*, port=%d led id=%d, get control mode error(%d)\n",
                                      port, led_id, rc);
                }
            }
        }
    }
}

/* FUNCTION NAME:   _sfp_led_initThread
 * PURPOSE:
 *      To initialize SFP LED working thread.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_led_initThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = osal_createThread(
        SFP_LED_APP_NAME,
        (UI32_T)SFP_LED_STACK_SIZE,
        (UI32_T)SFP_LED_THREAD_PRI,
        _sfp_led_thread,
        (void *)((AIR_HUGE_T)unit),
        &(_ptr_sfp_led_cb[unit]->thread_id));

    SFP_LED_DBG_INFO(0, "[SFP_LED]-  Unit %d, create thread result %d\n", unit, rc);

    return rc;
}
#endif

/* FUNCTION NAME:   _sfp_led_deinitThread
 * PURPOSE:
 *      To deinitialize SFP LED working thread.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_led_deinitThread(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = osal_destroyThread(&(_ptr_sfp_led_cb[unit]->thread_id));
    return rc;
}

/* FUNCTION NAME:   _sfp_led_initRsrc
 * PURPOSE:
 *      To initialize SFP LED control block.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_led_initRsrc(
    const UI32_T unit)
{
    SFP_LED_CB_T  *ptr_cb = NULL;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         size = 0;

    size = sizeof(SFP_LED_CB_T);
    ptr_cb = (SFP_LED_CB_T *)osal_alloc(size, SFP_LED_APP_NAME);

    SFP_LED_DBG_INFO(0, "[SFP_LED] Size of control block %d\n", size);

    if (NULL != ptr_cb)
    {
        osal_memset(ptr_cb, 0, size);
        _ptr_sfp_led_cb[unit] = ptr_cb;
    }
    else
    {
        rc = AIR_E_NO_MEMORY;
    }

    return rc;
}

/* FUNCTION NAME:   _sfp_led_deinitRsrc
 * PURPOSE:
 *      To deinitialize SFP LED control block.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_led_deinitRsrc(
    const UI32_T unit)
{
    SFP_LED_CB_T  *ptr_cb = NULL;
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (NULL != _ptr_sfp_led_cb[unit]->ptr_sfp_port_info)
    {
        osal_free(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info);
    }

    _ptr_sfp_led_cb[unit]->ptr_sfp_port_info = NULL;

    ptr_cb = _ptr_sfp_led_cb[unit];

    if (NULL != ptr_cb)
    {
        osal_free(ptr_cb);
    }
    _ptr_sfp_led_cb[unit] = NULL;

    return rc;
}

/* FUNCTION NAME:   _sfp_led_initCfg
 * PURPOSE:
 *      To initialize SFP LED application default configuration from customer_ref.c.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_led_initCfg(
    const UI32_T               unit,
    const UI32_T               led_cfg_cnt,
    SFP_LED_PORT_MAP_T        *ptr_sfp_port_map,
    SFP_LED_PORT_STATUS_FUNC_T port_status_func)
{
    AIR_CFG_VALUE_T led_behavior;

    /* If there is one invalid port, rc will be BAD_PARAMETER
     * And then application initialization fail. */
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    UI8_T           i, j;
    UI32_T          port = 0, temp_port_id;
    UI32_T          gpio_pin_id = 0, led_id;

    _ptr_sfp_led_cb[unit]->total_led_cfg_cnt = led_cfg_cnt;
    _ptr_sfp_led_cb[unit]->ptr_sfp_port_info =
        (SFP_LED_PORT_INFO_T *)osal_alloc(led_cfg_cnt * sizeof(SFP_LED_PORT_INFO_T), SFP_LED_APP_NAME);
    _ptr_sfp_led_cb[unit]->port_status_func = port_status_func;

    for (i = 0; i < led_cfg_cnt; i++)
    {
        port = ptr_sfp_port_map[i].port;
        gpio_pin_id = ptr_sfp_port_map[i].gpio_pin_id;

        /* find led id of the same port, start from 0 */
        led_id = 0;
        temp_port_id = port;
        for (j = 0; j < i; j++)
        {
            if (temp_port_id == ptr_sfp_port_map[j].port)
            {
                led_id++;
            }
        }

        _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].port = port;
        /* store led id, and led pin id */
        _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_pin_id = gpio_pin_id;
        _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].led_id = led_id;

        osal_memset(&led_behavior, 0, sizeof(AIR_CFG_VALUE_T));
        led_behavior.value = 0xFFFF; /* Default invalid value */
        led_behavior.param0 = ptr_sfp_port_map[i].port;
        led_behavior.param1 = led_id;

        rc = air_cfg_getValue(0, AIR_CFG_TYPE_PHY_LED_BEHAVIOR, &led_behavior);

        /* If config value is valid, init behavior flags in control block */
        if (0xFFFF != led_behavior.value)
        {
            _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].flags = led_behavior.value;
            SFP_LED_DBG_INFO(0, "[SFP_LED] port=%d , config = 0x%x\n", ptr_sfp_port_map[i].port, led_behavior.value);
        }
        /* If config value is invalid, show error message to user and initialization fail */
        else
        {
            rc = AIR_E_BAD_PARAMETER;
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, port=%d init cfg error(%d), CHECK sfp_port_map[] or _customer_ref_cfg[]\n",
                              ptr_sfp_port_map[i].port, rc);
            return rc;
        }

        /* reset control mode, force state, pattern config, and set led pin id as output
           before infinite loop start */
        rc = air_port_setPhyLedCtrlMode(unit, port, led_id, AIR_PORT_PHY_LED_CTRL_MODE_PHY);
        if (AIR_E_OK != rc)
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, port=%d led id = %d, set phy led ctrl mode error(%d)\n",
                              port, led_id, rc);
        }

        rc = air_port_setPhyLedForceState(unit, port, led_id, AIR_PORT_PHY_LED_STATE_FORCE_PATT);
        if (AIR_E_OK != rc)
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, port=%d led id = %d, set phy led force state error(%d)\n",
                              port, led_id, rc);
        }

        rc = air_port_setPhyLedForcePattCfg(unit, port, led_id, AIR_PORT_PHY_LED_PATT_HZ_HALF);
        if (AIR_E_OK != rc)
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, port=%d led id = %d, set phy led pattern cfg error(%d)\n",
                              port, led_id, rc);
        }

        rc = air_perif_setGpioDirection(unit, gpio_pin_id, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
        if (AIR_E_OK != rc)
        {
            SFP_LED_DBG_ERROR(0, "[SFP_LED]*Error*, GPIO pin id=%d set GPIO direction error(%d)\n",
                              gpio_pin_id, rc);
        }
    }
    return rc;
}

/* FUNCTION NAME: sfp_led_init
 *
 * PURPOSE:
 *      Initialize SFP LED application.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_led_init(
    const UI32_T               unit,
    const UI32_T               led_cfg_cnt,
    SFP_LED_PORT_MAP_T        *ptr_sfp_port_map,
    SFP_LED_PORT_STATUS_FUNC_T port_status_func)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (NULL != _ptr_sfp_led_cb[unit] &&
        TRUE == _ptr_sfp_led_cb[unit]->sfp_led_inited)
    {
        return AIR_E_ALREADY_INITED;
    }

    if (unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM)
    {
        rc = _sfp_led_initRsrc(unit);

        if (AIR_E_OK == rc)
        {
            rc = _sfp_led_initCfg(unit, led_cfg_cnt, ptr_sfp_port_map, port_status_func);
        }

#ifdef AIR_EN_SFP_LED_WITH_THREAD
        if (AIR_E_OK == rc)
        {
            rc = _sfp_led_initThread(unit);
        }
#endif
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == rc)
    {
        _ptr_sfp_led_cb[unit]->sfp_led_inited = TRUE;
        _ptr_sfp_led_cb[unit]->thread_runnning_cnt = 0;
    }

    return rc;
}

/* FUNCTION NAME: sfp_led_deinit
 *
 * PURPOSE:
 *      Deinitialize SFP LED application.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_led_deinit(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM)
    {
        rc = _sfp_led_deinitThread(unit);

        if (AIR_E_OK == rc)
        {
            rc = _sfp_led_deinitRsrc(unit);
        }
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }
    return rc;
}

/* FUNCTION NAME: sfp_led_updateLedStatus
 *
 * PURPOSE:
 *      Update LED status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_NOT_SUPPORT   --  Not support.
 * NOTES:
 *      If SFP LED thread is create by SFP LED app, then return not support.
 *
 */
#ifdef AIR_EN_SFP_LED_WITH_THREAD
AIR_ERROR_NO_T
sfp_led_updateLedStatus(
    UI32_T unit)
{
    return AIR_E_NOT_SUPPORT;
}
#else
AIR_ERROR_NO_T
sfp_led_updateLedStatus(
    UI32_T unit)
{
    UI32_T                state = 0;
    AIR_ERROR_NO_T        rc = AIR_E_OK;

    UI8_T                 i;            /* Loop index */
    UI32_T                port = 0;
    UI32_T                port_cfg = 0; /* LED behavior config defined in customer_ref.c */
    UI32_T                gpio_pin_id = 0;
    UI32_T                led_id = 0;
    UI32_T                high_active = 0;      /* High active flag */

    UI32_T                led_on_cfg = 0;       /* decide to turn on led or not */
    UI32_T                led_blink_tx_cfg = 0; /* decide to blink led or not */
    UI32_T                led_blink_rx_cfg = 0; /* decide to blink led or not */

    AIR_PORT_STATUS_T     ps;                   /* port status get from air_port_getPortStatus */
    SFP_LED_PORT_STATUS_T sfp_led_ps;           /* port status get from port_status_func */
    AIR_MIB_CNT_RX_T      rx_cnt;               /* MIB Counters of Rx Event */
    AIR_MIB_CNT_TX_T      tx_cnt;               /* MIB Counters of Tx Event */

    UI16_T                rx_oct;               /* RX octets */
    UI16_T                tx_oct;               /* TX octets */

    if (NULL == _ptr_sfp_led_cb[unit] ||
        FALSE == _ptr_sfp_led_cb[unit]->sfp_led_inited)
    {
        return AIR_E_NOT_SUPPORT;
    }

    for (i = 0; i < _ptr_sfp_led_cb[unit]->total_led_cfg_cnt; i++)
    {
        /* For each port, get the led behavior config and correspoding GPIO pin id */
        port = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].port;
        gpio_pin_id = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_pin_id;
        led_id = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].led_id;
        port_cfg = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].flags;

        /* Check if high active flag is defined */
        high_active = (port_cfg & SFP_LED_FLAGS_LED_ACTIVE_HIGH) ? TRUE : FALSE;

        /* Check if it's valid port id and GPIO pin id */
        if (0 != port && 0 != gpio_pin_id)
        {
            /* for each port check the control mode of the port first */
            rc = air_port_getPhyLedCtrlMode(unit, port, led_id, &state);

            if (AIR_E_OK == rc)
            {
                SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                 "[SFP_LED] - Unit %d, Port %d, GPIO pin id %d = %s mode\n",
                                 unit, port, gpio_pin_id,
                                 (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == state) ? "FORCE" : "PHY");

                /* Force mode, control LED based on force state and force pattern */
                if (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == state)
                {
                    rc = air_port_getPhyLedForceState(unit, port, led_id, &state);
                    if (AIR_E_OK == rc)
                    {
                        SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                         "[SFP_LED]    Port %d, led id %d, force state %d\n",
                                         port, led_id, state);

                        if (AIR_PORT_PHY_LED_STATE_FORCE_PATT == state)
                        {
                            rc = air_port_getPhyLedForcePattCfg(unit, port, led_id, &state);
                            if (AIR_E_OK == rc)
                            {
                                SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                                 "[SFP_LED]    Port %d, led id %d, force pattern %d\n",
                                                 port, led_id, state);

                                _sfp_led_turnLedBlink(unit, gpio_pin_id, _sfp_led_portPattToPerifPatt(state),
                                                      &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                            }
                            else
                            {
                                SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                                  "[SFP_LED]*Error*, port=%d led_id=%d, force pattern error(%d)\n",
                                                  port, led_id, rc);
                            }
                        }
                        else if (AIR_PORT_PHY_LED_STATE_OFF == state)
                        {
                            _sfp_led_turnLedOff(unit, gpio_pin_id, FALSE,
                                                &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                        }
                        else if (AIR_PORT_PHY_LED_STATE_ON == state)
                        {
                            _sfp_led_turnLedOn(unit, gpio_pin_id, FALSE,
                                               &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                        }
                        else
                        {
                            SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                              "[SFP_LED]*Error*, port=%d led_id=%d, unknown force state (%d)\n",
                                              port, led_id, state);
                        }
                    }
                    else
                    {
                        SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                          "[SFP_LED]*Error*, port=%d led_id=%d, get force state error(%d)\n",
                                          port, led_id, rc);
                    }
                }
                /* PHY mode, control LED based on link status */
                else if (AIR_PORT_PHY_LED_CTRL_MODE_PHY == state)
                {
                    if (NULL != _ptr_sfp_led_cb[unit]->port_status_func)
                    {
                        rc = _ptr_sfp_led_cb[unit]->port_status_func(unit, port, &sfp_led_ps);
                        ps.speed = sfp_led_ps.speed;
                        ps.flags = sfp_led_ps.flags & SFP_LED_PORT_STATUS_FLAGS_LINK_UP;
                        if (sfp_led_ps.flags & SFP_LED_PORT_STATUS_FLAGS_DUPLEX_FULL)
                        {
                            ps.duplex = AIR_PORT_DUPLEX_FULL;
                        }
                        else
                        {
                            ps.duplex = AIR_PORT_DUPLEX_HALF;
                        }
                    }
                    else
                    {
                        rc = air_port_getPortStatus(unit, port, &ps);
                    }

                    if (AIR_E_OK == rc)
                    {
                        /* Decide LED on/off/blink config based on link status */
                        _sfp_led_getLedBehavior(ps.speed, ps.duplex, port_cfg,
                                                &led_on_cfg, &led_blink_tx_cfg, &led_blink_rx_cfg);

                        SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                         "[SFP_LED]  Led behavior, port_cfg 0x%x, speed %d, dup %d,",
                                         port_cfg, ps.speed, ps.duplex);
                        SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                         " on 0x%x, tx 0x%x, rx 0x%x\n",
                                         led_on_cfg, led_blink_tx_cfg, led_blink_rx_cfg);

                        /* link up and speed or duplex matches the config */
                        if ((ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) && led_on_cfg)
                        {
                            rc = air_mib_getPortCnt(unit, port, &rx_cnt, &tx_cnt);
                            rx_oct = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].rx_oct;
                            tx_oct = _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].tx_oct;

                            SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                             "[SFP_LED]     RX: Port %d, prev %d, next %d \n",
                                             port, rx_oct, (UI16_T)rx_cnt.ROC);
                            SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                             "[SFP_LED]     TX: Port %d, prev %d, next %d \n",
                                             port, tx_oct, (UI16_T)tx_cnt.TOC);

                            if (AIR_E_OK == rc)
                            {
                                /* If there is traffic and speed matcheds the config,
                                 * blink the LED */
                                if (((rx_oct != (UI16_T)rx_cnt.ROC) && led_blink_rx_cfg) ||
                                    ((tx_oct != (UI16_T)tx_cnt.TOC) && led_blink_tx_cfg))
                                {
                                    SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                                     "[SFP_LED]     Port %d blink\n", port);

                                    _sfp_led_turnLedBlink(unit, gpio_pin_id, AIR_PERIF_GPIO_PATT_HZ_EIGHT,
                                                          &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                                }
                                /* There is no traffic, or the speed not match, just turn on LED */
                                else
                                {
                                    SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                                     "[SFP_LED]     Port %d up\n", port);
                                    _sfp_led_turnLedOn(unit, gpio_pin_id, high_active,
                                                       &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                                }
                                _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].rx_oct = (UI16_T)rx_cnt.ROC;
                                _ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].tx_oct = (UI16_T)tx_cnt.TOC;
                            }
                            else
                            {
                                SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                                  "[SFP_LED]*Error*, port=%d get mib counter error(%d)\n",
                                                  port, rc);
                            }
                        }
                        /* Link down, or the speed and duplex not match, turn off LED */
                        else
                        {
                            SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                             "[SFP_LED]     Port %d down\n", port);
                            _sfp_led_turnLedOff(unit, gpio_pin_id, high_active,
                                                &(_ptr_sfp_led_cb[unit]->ptr_sfp_port_info[i].gpio_output_state));
                        }
                    }
                    else
                    {
                        SFP_LED_DBG_INFO(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                         "[SFP_LED]*Error*, port=%d get link status error(%d)\n",
                                         port, rc);
                    }
                }
                else
                {
                    SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                      "[SFP_LED]*Error*, port=%d led id=%d, unknown control mode(%d)\n",
                                      port, led_id, state);
                }
            }
            else
            {
                SFP_LED_DBG_ERROR(_ptr_sfp_led_cb[unit]->thread_runnning_cnt,
                                  "[SFP_LED]*Error*, port=%d led id=%d, get control mode error(%d)\n",
                                  port, led_id, rc);
            }
        }
    }
    _ptr_sfp_led_cb[unit]->thread_runnning_cnt++;
    return AIR_E_OK;
}
#endif /*AIR_EN_SFP_LED_WITH_THREAD*/

#endif /*#ifdef AIR_EN_SFP_LED*/
