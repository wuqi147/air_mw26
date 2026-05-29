/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  erps_gpio.c
 * PURPOSE:
 *    This file contains the implementation of ERPS GPIO control functions.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "osapi_thread.h"
#include    "erps.h"
#include    "erps_gpio.h"
#include    "mw_cmd_util.h"
#include    "erps_queue.h"
#include    "mw_gpio_isr.h"
#include    <air_gpio.h>
#include    "platform.h"
#include    "hwcfg_util.h"
#include    <pp_def.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
void
_erps_gpio_rx_loss_isr(
    void    *ptr_arg);

static void
_erps_gpio_rx_loss_change_noti(
    void *ptr_arg);

/* STATIC VARIABLE DECLARATIONS
 */
static ERPS_GPIO_PARAM_T    _erps_gpio_instance[ERPS_GPIO_MAX_INSTANCE] = {{0}};

/* LOCAL SUBPROGRAM BODIES
 */
void
_erps_gpio_rx_loss_isr(
    void    *ptr_arg)
{
    UI8_T           idx = 0, gpio = 0;
    I32_T           status = 0;
    I32_T           gpio_val = 0;
    I32_T           rc = AIR_E_OK;
    UI32_T          data = 0, tick_diff = 0;

    for (idx = 0; idx < ERPS_GPIO_MAX_INSTANCE; idx++)
    {
        if(TRUE == _erps_gpio_instance[idx].in_use)
        {
            gpio = _erps_gpio_instance[idx].gpio;
            air_gpio_getInterruptStatus(gpio, &status);
            air_gpio_clearInterrupt(gpio);
            if(0 != status)
            {
                _erps_gpio_instance[idx].end_tick = xTaskGetTickCountFromISR();
                tick_diff = (_erps_gpio_instance[idx].end_tick - _erps_gpio_instance[idx].start_tick);
                _erps_gpio_instance[idx].start_tick = _erps_gpio_instance[idx].end_tick;
                rc = air_gpio_getValue(gpio, &gpio_val);
                if(E_OK != rc)
                {
                    continue;
                }
                if(ERPS_GPIO_JITTER_TIMEOUT <= tick_diff)
                {
                    if(GPIO_PIN_HIGH == gpio_val)
                    {
                        data = ((_erps_gpio_instance[idx].port & 0xFF) << 24) | ((_erps_gpio_instance[idx].gpio & 0xFF) << 16) | (gpio_val & 0xFFFF);
                        _erps_gpio_rx_loss_change_noti(&data);
                    }
                }
                _erps_gpio_instance[idx].gpio_val = gpio_val;
            }
        }
    }

    return;
}

static void
_erps_gpio_rx_loss_change_noti(
    void *ptr_arg)
{
    QueueHandle_t               ptr_erps_evt_queue = erps_evt_queue_handle_get();
    ERPS_QUEUE_EVT_MSG_T        evt_msg = {0};

    evt_msg.msg_id = MW_MSG_ID_ERPS_SFP_RX_LOSS_CHANGE_NOTI;
    if(NULL != ptr_arg)
    {
        osapi_memcpy(&evt_msg.data, ptr_arg, sizeof(UI32_T));
    }
    /* This function will be called by the GPIO interrupt handler. */
    xQueueSendFromISR(ptr_erps_evt_queue, &evt_msg, NULL);

    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   erps_gpio_rx_loss_interrupt_set
 * PURPOSE:
 *      This API is used to set the RX loss interrupt for a specific port and GPIO.
 *
 * INPUT:
 *      port                 -- Port ID
 *      gpio                 -- GPIO Pin
 *      is_enable            -- Enable or disable the interrupt
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_gpio_rx_loss_interrupt_set(
    const UI16_T        port,
    const UI8_T         gpio,
    UI8_T               is_enable)
{
    UI8_T               idx = 0;
    UI8_T               other_ins_exist = FALSE;
    I32_T               rc = MW_E_OK;
    ERPS_GPIO_PARAM_T   *ptr_gpio_instance = NULL;

    if(AIR_GPIO_MAX_NUM <= gpio)
    {
        return MW_E_BAD_PARAMETER;
    }
    for (idx = 0; idx < ERPS_GPIO_MAX_INSTANCE; idx++)
    {
        if(TRUE == is_enable)
        {
            if (FALSE == _erps_gpio_instance[idx].in_use)
            {
                osapi_memset(&_erps_gpio_instance[idx], 0, sizeof(ERPS_GPIO_PARAM_T));
                ptr_gpio_instance = &_erps_gpio_instance[idx];
                ptr_gpio_instance->in_use = TRUE;
                break;
            }
            else
            {
                if(gpio == _erps_gpio_instance[idx].gpio)
                {
                    return MW_E_ALREADY_INITED;
                }
            }
        }
        else
        {
            if(gpio == _erps_gpio_instance[idx].gpio)
            {
                osapi_memset(&_erps_gpio_instance[idx], 0, sizeof(ERPS_GPIO_PARAM_T));
                break;
            }
        }
    }

    if(ERPS_GPIO_MAX_INSTANCE == idx)
    {
        return MW_E_ENTRY_NOT_FOUND;
    }

    if(TRUE == is_enable)
    {
        ptr_gpio_instance->port = port;
        ptr_gpio_instance->gpio = gpio;
        ptr_gpio_instance->gpio_val = ~0;
        ptr_gpio_instance->start_tick = osapi_sysTickGet();
        air_gpio_setInterruptEdgeDetectMode(gpio, AIR_GPIO_EDGE_TRIGGER_BOTH);
        rc = mw_gpio_isr_user_is_registered(MW_GPIO_ISR_USER_ERPS);
        if(MW_E_NOT_INITED == rc)
        {
            rc = mw_gpio_isr_register(MW_GPIO_ISR_USER_ERPS, _erps_gpio_rx_loss_isr, NULL);
        }
    }
    else
    {
        air_gpio_setInterruptEdgeDetectMode(gpio, AIR_GPIO_EDGE_TRIGGER_DISABLE);
        /* Check whether there are other instances exist */
        for (idx = 0; idx < ERPS_GPIO_MAX_INSTANCE; idx++)
        {
            if (TRUE == _erps_gpio_instance[idx].in_use)
            {
                other_ins_exist = TRUE;
                break;
            }
        }
        if(FALSE == other_ins_exist)
        {
            rc = mw_gpio_isr_unregister(MW_GPIO_ISR_USER_ERPS);
        }
    }

    return rc;
}

/* FUNCTION NAME:   erps_gpio_dump
 * PURPOSE:
 *      This API is used to dump ERPS module GPIO info.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
erps_gpio_dump(void)
{
    UI8_T               idx = 0, num = 0;
    I32_T               rc = MW_E_OK;

    MW_CMD_OUTPUT("\nERPS GPIO instance list:\n");
    for(idx = 0; idx < ERPS_GPIO_MAX_INSTANCE; idx++)
    {
        if (TRUE == _erps_gpio_instance[idx].in_use)
        {
            MW_CMD_OUTPUT("[%d]\tgpio: %2d, port: %2d, gpio_val: 0x%x\r\n",
                            idx, _erps_gpio_instance[idx].gpio, _erps_gpio_instance[idx].port, _erps_gpio_instance[idx].gpio_val);
           num ++;
        }
    }
    MW_CMD_OUTPUT("\nTotal instance num: %d\r\n", num);
    rc = mw_gpio_isr_user_is_registered(MW_GPIO_ISR_USER_ERPS);
    MW_CMD_OUTPUT("ERPS is registered GPIO isr: %s\r\n", ((MW_E_NOT_INITED == rc) ? "false" : "true"));

    return;
}
