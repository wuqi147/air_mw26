/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  poe_main.c
 * PURPOSE:
 *      PoE application main loop.
 * NOTES:
 */

#include <poe_main.h>
#include <poe_info.h>
#include <poe_queue.h>
#include <poe_db_relate.h>
#include <poe_watchdog.h>
#include <poe_led.h>
#include <air_cfg.h>
#include <air_perif.h>
#include <api/diag.h>
#include <cmlib/cmlib_bit.h>
#include <osapi_thread.h>
#include <osapi_memory.h>
#include <osapi_message.h>
#include <mw_msg.h>
#include <db_api.h>
#include <poe_config_customer.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* MACRO FUNCTION DECLARATIONS
 */
#define POE_APP_CHECK_PTR(__ptr__)    \
    do                                \
    {                                 \
        if (NULL == (__ptr__))        \
        {                             \
            return MW_E_NOT_SUPPORT; \
        }                             \
    } while (0)

#define POE_APP_CHECK_POWER_SUFFICIENCY(__var__) \
    (((__var__)->total_available_power) >= ((__var__)->total_required_power))

/* LOCAL SUBPROGRAM DECLARATIONS
 */
/* STATIC VARIABLE DECLARATIONS
 */
static threadhandle_t  _poe_app_task_handle = NULL;
static POE_CB_T       *_ptr_poe_control_block = NULL;

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_poe_app_setPortPriorityHelper(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const UI32_T                priority)
{
    UI8_T rc = AIR_E_OK;

    rc = air_poe_setPortPriority(unit, port, alt, priority);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/p:%d set port priority failed(%d)\n", unit, port, rc);
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_app_setPortPseHelper(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_POE_ALTERNATIVE_T alt,
    const BOOL_T                state)
{
    UI8_T rc = AIR_E_OK;

    if (TRUE == state)
    {
        rc = air_poe_setPortPse(unit, port, alt, AIR_POE_PSE_MODE_ENABLE);
    }
    else
    {
        rc = air_poe_setPortPse(unit, port, alt, AIR_POE_PSE_MODE_DISABLE);
    }

    if (AIR_E_OK == rc)
    {
        PORT_INFO(_ptr_poe_control_block, unit, port)->pse_mode = (state) ? POE_PORT_PSE_MODE_ENABLE : POE_PORT_PSE_MODE_DISABLE;
    }
    else
    {
        POE_LOG_ERROR("u:%d/p:%d set port pse failed(%d)\n", unit, port, rc);
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_app_setPortPwrLimitHelper(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T power_limit)
{
    UI8_T                        rc = AIR_E_OK;
    UI32_T                       vmain = 0, current_limit = 0;
    AIR_POE_DEVICE_MEASUREMENT_T device_meas = {0};

    rc = air_poe_getDeviceMeasurement(unit, PORT_INFO(_ptr_poe_control_block, unit, port)->device_id, &device_meas);
    if (AIR_E_OK == rc)
    {
        vmain = device_meas.supply_voltage;
        if (!(vmain >= 4800 && vmain <= 6000))
        {
            vmain = 4800;
        }

        current_limit = power_limit * 1000000 / vmain;
        rc = air_poe_setPortCurrentLimit(unit, port, current_limit);
        if (AIR_E_OK == rc)
        {
            PORT_INFO(_ptr_poe_control_block, unit, port)->available_power = power_limit;
        }
        else
        {
            POE_LOG_ERROR("u:%d/p:%d set port curr limit failed(%d)\n", unit, port, rc);
        }
    }
    else
    {
        POE_LOG_ERROR("u:%d/dev:%d get device measurement failed(%d)\n", unit,
                      PORT_INFO(_ptr_poe_control_block, unit, port)->device_id, rc);
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_app_setSysPwrLimitHelper(
    const UI32_T unit,
    const UI32_T device,
    const UI32_T power_limit)
{
    UI8_T          rc = AIR_E_OK;
    UI32_T         w_data = 0;

    w_data = power_limit * 10;
    rc = air_poe_setDevicePowerLimit(unit, device, TRUE, w_data);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/dev:%d set power limit(%d) failed(%d)\n", unit, device, w_data, rc);
    }

    return rc;
}

static void
_poe_app_setSuspendModeHelper(
    const BOOL_T suspend)
{
    UI8_T                   rc = AIR_E_OK;
    UI8_T                   unit = 0, device = 0;
    AIR_POE_POWER_UP_MODE_T mode = AIR_POE_POWER_UP_MODE_LAST;

    if (TRUE == suspend)
    {
        mode = AIR_POE_POWER_UP_MODE_MANUAL;
    }
    else
    {
        mode = AIR_POE_POWER_UP_MODE_NORMAL;
    }

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            if (0 == (UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp & BIT(device)))
            {
                continue;
            }
            rc = air_poe_setDevicePowerUpMode(unit, device, mode);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/dev:%d set mode(%d) failed(%d)\n", unit, device, mode, rc);
            }
        }
    }
    if (AIR_E_OK == rc)
    {
        _ptr_poe_control_block->system_suspend_mode = suspend;
    }
}

static void
_poe_app_setSysCfgHelper(
    const UI32_T unit,
    const UI32_T device)
{
    UI8_T                  rc = AIR_E_OK;
    AIR_POE_PWR_STRATEGY_T strategy = AIR_POE_PWR_STRATEGY_LAST;

    /* system power mode */
    if (POE_SYS_POWER_MODE_PRIORITY == _ptr_poe_control_block->system_power_mode)
    {
        strategy = AIR_POE_PWR_STRATEGY_PRIORITY;
    }
    else
    {
        strategy = AIR_POE_PWR_STRATEGY_PLUG;
    }

    rc = air_poe_setDevicePowerStrategy(unit, device, strategy);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/dev:%d set power strategy(%d) failed(%d)\n", unit, device, strategy, rc);
    }

    /* system budget */
    rc = _poe_app_setSysPwrLimitHelper(unit, device, _ptr_poe_control_block->total_available_power);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/dev:%d set system budget(%d) failed(%d)\n", unit, device,
                      _ptr_poe_control_block->total_available_power, rc);
    }
}

static void
_poe_app_ctrlPoeModule(
    const POE_APP_CTRL_MODULE_T ctrl)
{
    UI8_T          rc = AIR_E_OK;
    UI32_T         unit = 0;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        if (POE_APP_CTRL_MODULE_INIT == ctrl)
        {
            rc = air_poe_init(unit);
        }
        else
        {
            rc = air_poe_deinit(unit);
        }

        if (AIR_E_OK != rc)
        {
            POE_LOG_ERROR("u:%d ctrl poe active(%d) failed(%d)\n", unit, ctrl, rc);
        }
    }
}

/* FUNCTION NAME:   _poe_app_getPDReqPower
 * PURPOSE:
 *      Obtain PD power require power based on its PD class.(gain factor is 0.1W)
 * INPUT:
 *      pd_class             -- PD class
 * OUTPUT:
 *      None
 * RETURN:
 *      UI32_T               -- corresponds to the PD class's PD requested power
 * NOTES:
 *      None
 */
static UI32_T
_poe_app_getPDReqPower(
    const AIR_POE_CLASS_T pd_class)
{
    UI32_T pd_req_pwr = 0;

    if (AIR_POE_CLASS_0 == pd_class)
    {
        pd_req_pwr = POE_PD_CLASS_0_REQ_PWR;
    }
    else if (AIR_POE_CLASS_1 == pd_class)
    {
        pd_req_pwr = POE_PD_CLASS_1_REQ_PWR;
    }
    else if (AIR_POE_CLASS_2 == pd_class)
    {
        pd_req_pwr = POE_PD_CLASS_2_REQ_PWR;
    }
    else if (AIR_POE_CLASS_3 == pd_class)
    {
        pd_req_pwr = POE_PD_CLASS_3_REQ_PWR;
    }
    else if (AIR_POE_CLASS_4 == pd_class)
    {
        pd_req_pwr = POE_PD_CLASS_4_REQ_PWR;
    }

    return pd_req_pwr;
}

static void
_poe_app_dbSetSysConfig(
    void)
{
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;

    ptr_poeCfg = poe_config_getPoeCfg();
    if(NULL == ptr_poeCfg)
    {
        /* During initialization, if the current Product ID is not matched with a corresponding PoE configuration,
         * then POEMON and POEAPP tasks will be deinitialized. */
        _poe_app_ctrlPoeModule(POE_APP_CTRL_MODULE_DEINIT);
        poe_app_deinit();
    }
    /* system power strategy */
    if (_ptr_poe_control_block->total_device_cnt > 1)
    {
        _ptr_poe_control_block->power_strategy_control = POE_PWR_CONTROL_SW;
    }
    else
    {
        _ptr_poe_control_block->power_strategy_control = ptr_poeCfg->poe_power_strategy_control;
    }

    /* system power mode */
    _ptr_poe_control_block->system_power_mode = POE_SYS_POWER_MODE_PRIORITY;

    /* system budget */
    _ptr_poe_control_block->total_available_power = POE_APP_DEFAULT_SYS_AVAI_PWR;
}

/* FUNCTION NAME:   _poe_app_dbSetToDefault
 * PURPOSE:
 *      Set DB to default value.
 * INPUT:
 *      pse_enable              -- ENABLE/DISABLE all ports PSE.
 *      device_reset            -- System occurs reset.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_dbSetToDefault(
    const BOOL_T pse_enable,
    const BOOL_T device_reset)
{
    I8_T  unit = 0, port = 0;
    UI8_T idx = 0, device = 0;

    /* system config initialization */
    _ptr_poe_control_block->hw_reset_done = TRUE;
    _ptr_poe_control_block->pre_system_status = POE_SYS_STATUS_ABNORMAL;

    _poe_app_dbSetSysConfig();
    if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
    {
        for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
        {
            if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
            {
                continue;
            }
            for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
            {
                if (0 == (UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp & BIT(device)))
                {
                    continue;
                }
                _poe_app_setSysCfgHelper(unit, device);
            }
        }
    }
    else /* POE_PWR_CONTROL_SW */
    {
        /* system config */
        _ptr_poe_control_block->pri_plug_list_cnt = 0;
        _ptr_poe_control_block->total_allocated_power = 0;
        _ptr_poe_control_block->total_required_power = 0;
        _ptr_poe_control_block->total_pd_req_power = 0;
        _ptr_poe_control_block->system_suspend_mode = FALSE;
        for (port = 0; port < _ptr_poe_control_block->total_port_cnt; port++)
        {
            _ptr_poe_control_block->ptr_pri_plug_list[port].air_port = POE_PORT_INVALID;
        }

        /* switch chip config */
        for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
        {
            if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
            {
                continue;
            }
            /* config data */
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->high_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->critical_pbmp);
            /* status data */
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->event_restart_pbmp);
            AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_status_on_pbmp);
        }
    }

    /* port config initialization */
    for (unit = AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1; unit >= 0; unit--)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = POE_TOTAL_PORTS_NUM - 1; port >= 0; port--)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            PORT_INFO(_ptr_poe_control_block, unit, port)->type = POE_PORT_TYPE_AT;
            PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control = TRUE;
            PORT_INFO(_ptr_poe_control_block, unit, port)->delay_time = 0;
            PORT_INFO(_ptr_poe_control_block, unit, port)->event_restart_time = POE_APP_DEFAULT_RESTART_TIME;
            _poe_app_setPortPwrLimitHelper(unit, port, POE_APP_DEFAULT_PORT_AVAI_PWR);
            _poe_app_setPortPseHelper(unit, port, AIR_POE_ALTERNATIVE_NONE, pse_enable);

            if (TRUE == device_reset)
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags |= POE_APP_DEVICE_RESET_EVENT;
            }

            if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
            {
                /* priority */
                _poe_app_setPortPriorityHelper(unit, port, AIR_POE_ALTERNATIVE_NONE, idx);
                idx++;
            }
            else
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count = 0;
                PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count = 0;
                /* port priority */
                AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp, port);
                _ptr_poe_control_block->ptr_pri_port_list[idx].unit = unit;
                _ptr_poe_control_block->ptr_pri_port_list[idx++].air_port = port;
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_powerUpReadyPort
 * PURPOSE:
 *      Power up ports that are eligibale.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_powerUpReadyPort(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port))
            {
                AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port);

                POE_LOG_DEBUG("u:%d/p:%d start powering up\n", unit, port);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, FALSE);
                rc |= air_poe_triggerPortPowerUp(unit, port, alt);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, TRUE);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("u:%d/p:%d trigger port power up failed(%d)\n", unit, port, rc);
                }
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_kickOutPowerOnPort
 * PURPOSE:
 *      Kick out low-priority port that is already power-up.
 * INPUT:
 *      unit                -- unit id
 *      port                -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_kickOutPowerOnPort(
    const UI8_T unit,
    const UI8_T port)
{
    UI8_T                      rc = AIR_E_OK;
    UI32_T                     port_pd_req_pwr = 0;
    AIR_POE_PORT_MEASUREMENT_T port_meas = {0};
    AIR_POE_PORT_STATUS_T      status = {0};
    AIR_POE_ALTERNATIVE_T      alt = AIR_POE_ALTERNATIVE_NONE;

    PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count = POE_APP_KICK_OUT_RESTART_TIME * 1000 / POE_MON_POLLING_INTERVAL_MS;
    POE_LOG_DEBUG("u:%d/p:%d disable port pse and starts a %d second(s) restart timer due to low priority\n",
                  unit, port, POE_APP_KICK_OUT_RESTART_TIME);

    rc = _poe_app_setPortPseHelper(unit, port, alt, FALSE);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/p:%d set port pse failed(%d)\n", unit, port, rc);
    }

    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);
    rc = air_poe_getPortMeasurement(unit, port, alt, &port_meas);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/p:%d get port measurement failed(%d)\n", unit, port, rc);
    }
    _ptr_poe_control_block->total_required_power -= (port_meas.current / 100) * (port_meas.voltage / 100) / 100;
    rc = air_poe_getPortStatus(unit, port, alt, &status);
    if (AIR_E_OK != rc)
    {
        /* Treat it as class PD to avoid underestimation */
        status.pd_class = AIR_POE_CLASS_4;
        POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
    }
    port_pd_req_pwr = _poe_app_getPDReqPower(status.pd_class);
    _ptr_poe_control_block->total_pd_req_power += port_pd_req_pwr;
    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port);
    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port);
}

/* FUNCTION NAME:   _poe_app_kickOutReadyPort
 * PURPOSE:
 *      Kick out low-priority port that is ready to power up.
 * INPUT:
 *      unit                -- unit id
 *      port                -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_kickOutReadyPort(
    const UI8_T unit,
    const UI8_T port)
{
    UI8_T                 rc = AIR_E_OK;
    UI32_T                port_pd_req_pwr = 0;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    POE_LOG_DEBUG("u:%d/p:%d port removed from ready-to-power-up list, restart port\n", unit, port);
    rc |= _poe_app_setPortPseHelper(unit, port, alt, FALSE);
    rc |= _poe_app_setPortPseHelper(unit, port, alt, TRUE);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("u:%d/p:%d restart port failed(%d)\n", unit, port, rc);
    }

    rc = air_poe_getPortStatus(unit, port, alt, &status);
    if (AIR_E_OK != rc)
    {
        /* Treat it as class PD to avoid underestimation */
        status.pd_class = AIR_POE_CLASS_4;
        POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
    }
    port_pd_req_pwr = _poe_app_getPDReqPower(status.pd_class);
    _ptr_poe_control_block->total_required_power -= port_pd_req_pwr;
    _ptr_poe_control_block->total_pd_req_power += port_pd_req_pwr;
    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port);
    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port);
}

/* FUNCTION NAME:   _poe_app_plugPriorityKickOut
 * PURPOSE:
 *      Kick out port based on Plug Priority.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_plugPriorityKickOut(
    void)
{
    I8_T   u = 0, p = 0, i = 0;
    UI8_T  unit = 0, port = 0;
    BOOL_T sufficient_power = FALSE;

    _ptr_poe_control_block->total_pd_req_power = 0;
    for (u = AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1; u >= 0; u--)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, u))
        {
            continue;
        }
        for (p = POE_TOTAL_PORTS_NUM - 1; p >= 0; p--)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, u, p))
            {
                continue;
            }
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, u)->port_ready_pbmp, p))
            {
                _poe_app_kickOutReadyPort(u, p);
                if (POE_APP_CHECK_POWER_SUFFICIENCY(_ptr_poe_control_block))
                {
                    sufficient_power = TRUE;
                    break;
                }
            }
        }
        if (TRUE == sufficient_power)
        {
            break;
        }
    }

    if (FALSE == sufficient_power)
    {
        for (i = _ptr_poe_control_block->pri_plug_list_cnt - 1; i >= 0; i--)
        {
            unit = _ptr_poe_control_block->ptr_pri_plug_list[i].unit;
            port = _ptr_poe_control_block->ptr_pri_plug_list[i].air_port;

            _poe_app_kickOutPowerOnPort(unit, port);
            if (POE_APP_CHECK_POWER_SUFFICIENCY(_ptr_poe_control_block))
            {
                break;
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_portPriorityKickOut
 * PURPOSE:
 *      Kick out port based on Port Priority.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_portPriorityKickOut(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 i = 0, unit = 0, port = 0;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    _ptr_poe_control_block->total_pd_req_power = 0;
    for (i = 0; i < _ptr_poe_control_block->total_port_cnt; i++)
    {
        unit = _ptr_poe_control_block->ptr_pri_port_list[i].unit;
        port = _ptr_poe_control_block->ptr_pri_port_list[i].air_port;

        if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port))
        {
            _poe_app_kickOutReadyPort(unit, port);
        }
        else
        {
            rc = air_poe_getPortStatus(unit, port, alt, &status);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
            }
            if (AIR_POE_POWER_STATUS_ON == status.power_status)
            {
                _poe_app_kickOutPowerOnPort(unit, port);
            }
            else
            {
                continue;
            }
        }
        if (POE_APP_CHECK_POWER_SUFFICIENCY(_ptr_poe_control_block))
        {
            break;
        }
    }
}

/* FUNCTION NAME:   _poe_app_suspendModeHandler
 * PURPOSE:
 *      Suspend mode handler function.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_suspendModeHandler(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0;
    UI32_T                port_pd_req_pwr = 0;
    BOOL_T                ready = FALSE;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    _ptr_poe_control_block->total_pd_req_power = 0;
    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp);
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            rc = air_poe_getPortStatus(unit, port, alt, &status);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
            }
            port_pd_req_pwr = _poe_app_getPDReqPower(status.pd_class);
            rc = air_poe_getPortPowerUp(unit, port, alt, &ready);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port suspend power bit failed(%d)\n", unit, port, rc);
            }

            if ((0 != port_pd_req_pwr) && (AIR_POE_SIGNATURE_GOOD == status.pd_signature) && (TRUE == ready) &&
                (AIR_POE_POWER_STATUS_OFF == status.power_status) && (0 == PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count))
            {
                if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port))
                {
                    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port);
                    POE_LOG_DEBUG("u:%d/p:%d port in ready-to-power-up list\n", unit, port);
                    _ptr_poe_control_block->total_pd_req_power += port_pd_req_pwr;
                }
                else
                {
                    if (0 == PORT_INFO(_ptr_poe_control_block, unit, port)->delay_time)
                    {
                        AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->port_ready_pbmp, port);
                        POE_LOG_DEBUG("u:%d/p:%d port in ready-to-power-up list\n", unit, port);
                        _ptr_poe_control_block->total_pd_req_power += port_pd_req_pwr;
                    }
                    else
                    {
                        PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count =
                            PORT_INFO(_ptr_poe_control_block, unit, port)->delay_time * 1000 / POE_MON_POLLING_INTERVAL_MS;
                        POE_LOG_DEBUG("u:%d/p:%d port starts a %d second(s) delay time\n", unit, port,
                                      PORT_INFO(_ptr_poe_control_block, unit, port)->delay_time);
                    }
                }
            }
        }
    }

    _ptr_poe_control_block->total_required_power =
        _ptr_poe_control_block->total_allocated_power + _ptr_poe_control_block->total_pd_req_power;
    if (!POE_APP_CHECK_POWER_SUFFICIENCY(_ptr_poe_control_block))
    {
        POE_LOG_DEBUG("since total_required_power(%d) > total_available_power(%d)\n"
                      "system power up mode: %s\n",
                      _ptr_poe_control_block->total_required_power, _ptr_poe_control_block->total_available_power,
                      POE_SYS_POWER_MODE_PRIORITY == _ptr_poe_control_block->system_power_mode ? "Port priority" :
                                                                                                 "Plug priority");
        if (POE_SYS_POWER_MODE_PRIORITY == _ptr_poe_control_block->system_power_mode)
        {
            _poe_app_portPriorityKickOut();
        }
        else
        {
            _poe_app_plugPriorityKickOut();
        }
    }

    _poe_app_powerUpReadyPort();
}

/* FUNCTION NAME:   _poe_app_leaveSuspendMode
 * PURPOSE:
 *      Leave Suspend mode to Normal mode.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_leaveSuspendMode(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    _ptr_poe_control_block->total_pd_req_power = 0;
    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        AIR_PORT_BITMAP_CLEAR(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp);
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            rc = air_poe_getPortStatus(unit, port, alt, &status);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
            }
            if ((TRUE == PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control) && (AIR_POE_POWER_STATUS_OFF == status.power_status) &&
                (!AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->event_restart_pbmp, port)))
            {
                POE_LOG_DEBUG("u:%d/p:%d restart port(Leave suspend mode)\n", unit, port);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, FALSE);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, TRUE);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("u:%d/p:%d restart port failed(%d)\n", unit, port, rc);
                }

                PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count = 0;
                PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count = 0;
                AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_checkSystemMode
 * PURPOSE:
 *      Check if the system should be suspended.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_checkSystemMode(
    void)
{
    UI8_T                      rc = AIR_E_OK;
    UI8_T                      unit = 0, port = 0;
    UI32_T                     total_remaining_power = 0;
    BOOL_T                     delay_exist = FALSE;
    AIR_POE_PORT_STATUS_T      status = {0};
    AIR_POE_PORT_MEASUREMENT_T port_meas = {0};
    AIR_POE_ALTERNATIVE_T      alt = AIR_POE_ALTERNATIVE_NONE;

    _ptr_poe_control_block->total_allocated_power = 0;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            rc = air_poe_getPortStatus(unit, port, alt, &status);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
            }
            if (AIR_POE_POWER_STATUS_ON == status.power_status)
            {
                rc = air_poe_getPortMeasurement(unit, port, alt, &port_meas);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("u:%d/p:%d get port measurement failed(%d)\n", unit, port, rc);
                }
                _ptr_poe_control_block->total_allocated_power +=
                    (port_meas.current / 100) * (port_meas.voltage / 100) / 100;
            }
            if (PORT_INFO(_ptr_poe_control_block, unit, port)->delay_time > 0)
            {
                delay_exist = TRUE;
            }
        }
    }

    _ptr_poe_control_block->total_required_power =
        _ptr_poe_control_block->total_allocated_power + _ptr_poe_control_block->total_pd_req_power;
    total_remaining_power =
        (_ptr_poe_control_block->total_available_power > _ptr_poe_control_block->total_required_power) ?
            (_ptr_poe_control_block->total_available_power - _ptr_poe_control_block->total_required_power) :
            0;
    if (((MAX_POE_SYS_PWR_LIM * _ptr_poe_control_block->total_device_cnt) >= total_remaining_power) || (TRUE == delay_exist))
    {
        if (FALSE == _ptr_poe_control_block->system_suspend_mode)
        {
            POE_LOG_DEBUG("Enter suspend mode\n");
            _poe_app_setSuspendModeHelper(TRUE);
        }
    }
    else if (((MAX_POE_SYS_PWR_LIM * _ptr_poe_control_block->total_device_cnt) + MAX_POE_SYS_PWR_BUF) <=
             total_remaining_power)
    {
        if (TRUE == _ptr_poe_control_block->system_suspend_mode)
        {
            POE_LOG_DEBUG("Leave suspend mode\n");
            _poe_app_setSuspendModeHelper(FALSE);
            _poe_app_leaveSuspendMode();
        }
    }
}

/* FUNCTION NAME:   _poe_app_checkEvent
 * PURPOSE:
 *      Check the event of each port and handle it.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_checkEvent(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0, i = 0, idx = 0;
    UI16_T                value = 0;
    BOOL_T                on_to_off_flag = FALSE;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_EVENT_T       event = {0};
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            rc = air_poe_getPortStatus(unit, port, alt, &status);
            if (AIR_E_OK != rc)
            {
                /* To prevent being mistakenly identified as 808AR IC BUG */
                status.power_status = AIR_POE_POWER_STATUS_ON;
                POE_LOG_ERROR("u:%d/p:%d get port status failed(%d)\n", unit, port, rc);
            }
            if (AIR_POE_POWER_STATUS_ON == status.power_status)
            {
                if (FALSE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_status_on_pbmp, port))
                {
                    POE_LOG_DEBUG("u:%d/p:%d port power status from OFF to ON\n", unit, port);
                    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_status_on_pbmp, port);
                    _ptr_poe_control_block->ptr_pri_plug_list[_ptr_poe_control_block->pri_plug_list_cnt].unit = unit;
                    _ptr_poe_control_block->ptr_pri_plug_list[_ptr_poe_control_block->pri_plug_list_cnt++].air_port =
                        port;
                }
            }
            else
            {
                if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_status_on_pbmp, port))
                {
                    POE_LOG_DEBUG("u:%d/p:%d port power status from ON to OFF\n", unit, port);
                    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_status_on_pbmp, port);
                    for (i = 0; i < _ptr_poe_control_block->pri_plug_list_cnt; i++)
                    {
                        if (port == _ptr_poe_control_block->ptr_pri_plug_list[i].air_port)
                        {
                            _ptr_poe_control_block->ptr_pri_plug_list[i].air_port = POE_PORT_INVALID;
                            break;
                        }
                    }
                    on_to_off_flag = TRUE;
                }
            }

            rc = air_poe_getPortEvent(unit, port, alt, &event);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d get port event failed(%d)\n", unit, port, rc);
            }
            rc = air_poe_clearPortEvent(unit, port, alt);
            if (AIR_E_OK != rc)
            {
                POE_LOG_ERROR("u:%d/p:%d clear port event failed(%d)\n", unit, port, rc);
            }
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port))
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags |= POE_APP_INSUFFICIENT_PWR_EVENT;
            }
            if (0 != event.event_flags)
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags |= event.event_flags;
                AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);

                if ((0 != event.event_flags) &
                    (AIR_POE_EVENT_FLAGS_OVERLOAD | AIR_POE_EVENT_FLAGS_SHORT_CIRCUIT |
                     AIR_POE_EVENT_FLAGS_OVER_CURRENT_LIMIT | AIR_POE_EVENT_FLAGS_OVER_VOLTAGE_LIMIT))
                {
                    rc = _poe_app_setPortPseHelper(unit, port, alt, FALSE);
                    if (AIR_E_OK != rc)
                    {
                        POE_LOG_ERROR("u:%d/p:%d set port pse failed(%d)\n", unit, port, rc);
                    }

                    PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count =
                        PORT_INFO(_ptr_poe_control_block, unit, port)->event_restart_time * 1000 / POE_MON_POLLING_INTERVAL_MS;
                    POE_LOG_DEBUG("u:%d/p:%d port power off extended(%d second(s)) due to event\n",
                                  unit, port, PORT_INFO(_ptr_poe_control_block, unit, port)->event_restart_time);
                    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->event_restart_pbmp, port);
                }
            }
            else
            {
                rc = air_poe_getDeviceInfo(unit, PORT_INFO(_ptr_poe_control_block, unit, port)->device_id, &value);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("u:%d/dev:%d get device hw revision-id failed(%d)\n", unit,
                                  PORT_INFO(_ptr_poe_control_block, unit, port)->device_id, rc);
                }

                if ((POE_PORT_PSE_MODE_ENABLE == PORT_INFO(_ptr_poe_control_block, unit, port)->pse_mode) && (TRUE == on_to_off_flag) &&
                    (FALSE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port)) &&
                    (POE_APP_REVISION_ID_IP808AR == value))
                {
                    PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags |= AIR_POE_EVENT_FLAGS_OVER_CURRENT_LIMIT;
                    rc = _poe_app_setPortPseHelper(unit, port, alt, FALSE);
                    if (AIR_E_OK != rc)
                    {
                        POE_LOG_ERROR("u:%d/p:%d set port pse failed(%d)\n", unit, port, rc);
                    }

                    PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count =
                        PORT_INFO(_ptr_poe_control_block, unit, port)->event_restart_time * 1000 / POE_MON_POLLING_INTERVAL_MS;
                    POE_LOG_DEBUG("u:%d/p:%d port power off extended(%d second(s)) due to event\n",
                                  unit, port, PORT_INFO(_ptr_poe_control_block, unit, port)->event_restart_time);
                    AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->event_restart_pbmp, port);
                    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);
                }
            }
            on_to_off_flag = FALSE;
        }
    }

    for (i = 0; i < _ptr_poe_control_block->pri_plug_list_cnt; i++)
    {
        if (POE_PORT_INVALID == _ptr_poe_control_block->ptr_pri_plug_list[i].air_port)
        {
            continue;
        }
        _ptr_poe_control_block->ptr_pri_plug_list[idx].unit = _ptr_poe_control_block->ptr_pri_plug_list[i].unit;
        _ptr_poe_control_block->ptr_pri_plug_list[idx++].air_port =
            _ptr_poe_control_block->ptr_pri_plug_list[i].air_port;
    }
    _ptr_poe_control_block->pri_plug_list_cnt = idx;
}

/* FUNCTION NAME:   _poe_app_restartTimerHandler
 * PURPOSE:
 *      Port restart timer handler.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_restartTimerHandler(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            if ((0 != PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count) && (--PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count == 0))
            {
                POE_LOG_DEBUG("u:%d/p:%d restart timer expired, enable port pse\n", unit, port);
                if (TRUE == PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control)
                {
                    rc = _poe_app_setPortPseHelper(unit, port, alt, TRUE);
                    if (AIR_E_OK != rc)
                    {
                        POE_LOG_ERROR("u:%d/p:%d set port pse failed(%d)\n", unit, port, rc);
                    }
                }
                else
                {
                    POE_LOG_DEBUG("u:%d/p:%d enable port pse stopped since pse_control is FALSE\n",
                                  unit, port);
                }
                AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->event_restart_pbmp, port);
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_delayTimerHandler
 * PURPOSE:
 *      Port delay timer handler.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_delayTimerHandler(
    void)
{
    UI8_T                 rc = AIR_E_OK;
    UI8_T                 unit = 0, port = 0;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            if (NULL == PORT_INFO(_ptr_poe_control_block, unit, port))
            {
                continue;
            }
            if ((0 != PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count) && (--PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count == 0))
            {
                POE_LOG_DEBUG("u:%d/p:%d delay timer expired, restarting port\n", unit, port);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, FALSE);
                rc |= _poe_app_setPortPseHelper(unit, port, alt, TRUE);
                if (AIR_E_OK != rc)
                {
                    POE_LOG_ERROR("u:%d/p:%d restart port failed(%d)\n", unit, port, rc);
                }
                AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);
            }
        }
    }
}

/* FUNCTION NAME:   _poe_app_checkDeviceReset
 * PURPOSE:
 *      Handle system status check.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                -- Continue to next iteration
 *      FALSE               -- Execute the remaining statements
 * NOTES:
 *      None
 */
static BOOL_T
_poe_app_checkDeviceReset(
    void)
{
    UI8_T          rc = AIR_E_OK;
    UI8_T          unit = 0, device = 0;
    UI32_T         scratch_val = 0;
    BOOL_T         any_device_reset = FALSE;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            if (0 != (UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp & BIT(device)))
            {
                rc = air_poe_getDeviceScratch(unit, device, &scratch_val);
                if ((AIR_E_OK != rc) || (POE_APP_RESET_FLAG != scratch_val))
                {
                    POE_LOG_ERROR("u:%d/dev:%d check device scratch value(%d) failed(%d)\n", unit,
                                  device, scratch_val, rc);
                    any_device_reset = TRUE;
                    break;
                }
            }
        }
        if (TRUE == any_device_reset)
        {
            break;
        }
    }

    return any_device_reset;
}

/* FUNCTION NAME:   _poe_app_hardwareReset
 * PURPOSE:
 *      Handle Hardware reset and system status check.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_app_hardwareReset(
    void)
{
    const POE_CONFIG_SETTINGS_T *ptr_poeCfg = NULL;
    UI8_T                        rc = AIR_E_OK;
    UI8_T                        i = 0;

    ptr_poeCfg = poe_config_getPoeCfg();
    if (NULL != ptr_poeCfg)
    {
        for (i = 0; i < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; i++)
        {
            if (NULL == UNIT_INFO(_ptr_poe_control_block, i))
            {
                continue;
            }
            rc |= air_perif_setGpioDirection(i, ptr_poeCfg->poe_reset_pin, AIR_PERIF_GPIO_DIRECTION_OUTPUT);
            rc |= air_perif_setGpioOutputData(i, ptr_poeCfg->poe_reset_pin, AIR_PERIF_GPIO_DATA_LOW);
            osapi_delay(POE_MON_POLLING_INTERVAL_MS);
            rc |= air_perif_setGpioOutputData(i, ptr_poeCfg->poe_reset_pin, AIR_PERIF_GPIO_DATA_HIGH);
        }
        if (AIR_E_OK == rc)
        {
            POE_LOG_DEBUG("all poe chip hardware reset done\n");
            _ptr_poe_control_block->hw_reset_done = TRUE;
        }
        else
        {
            POE_LOG_ERROR("poe chip hardware reset failed(%d)\n", rc);
        }
    }
}

static void
_poe_app_thread(
    void *ptr_argv)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T start_tick = 0, end_tick = 0, dur_tick = 0;
    UI16_T poe_msg_timeout = POE_MON_POLLING_INTERVAL_MS;
    MW_MSG_T *ptr_mw_msg = NULL;

    _ptr_poe_control_block->hw_reset_done = TRUE;
    _ptr_poe_control_block->reset_init_done = FALSE;
    _ptr_poe_control_block->pre_system_status = POE_SYS_STATUS_ABNORMAL;
    _ptr_poe_control_block->poe_hw_reset_cnt = POE_APP_HW_RESET_COUNT;
    _ptr_poe_control_block->poe_init_ctrl_module_cnt = POE_APP_INIT_CTRL_MODULE_COUNT;

    do
    {
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

    start_tick = osapi_sysTickGet();
    while (1)
    {
        rc = osapi_msgRecv(POE_QUEUE_NAME, (UI8_T **)&ptr_mw_msg, 0, poe_msg_timeout);
        if (MW_E_OK == rc)
        {
            if (MW_MSG_ID_DB == ptr_mw_msg->msg_id)
            {
                poe_db_handleMsg(_ptr_poe_control_block, (DB_MSG_T *)ptr_mw_msg);
            }
            MW_FREE(ptr_mw_msg);
        }

        end_tick = osapi_sysTickGet();
        dur_tick = end_tick - start_tick;

        if (dur_tick >= POE_MON_POLLING_INTERVAL_MS)
        {
            start_tick = osapi_sysTickGet();
            poe_msg_timeout = POE_MON_POLLING_INTERVAL_MS;
            if (FALSE == _ptr_poe_control_block->hw_reset_done)
            {
                if (_ptr_poe_control_block->poe_hw_reset_cnt > 0)
                {
                    _ptr_poe_control_block->poe_hw_reset_cnt--;
                    _poe_app_hardwareReset();
                }
                else
                {
                    poe_app_deinit();
                }
            }
            else
            {
                if (FALSE == _poe_app_checkDeviceReset())
                {
                    if (POE_SYS_STATUS_ABNORMAL == _ptr_poe_control_block->pre_system_status)
                    {
                        _poe_app_dbSetToDefault(TRUE, FALSE);
                        _ptr_poe_control_block->pre_system_status = POE_SYS_STATUS_NORMAL;
                        _ptr_poe_control_block->poe_hw_reset_cnt = POE_APP_HW_RESET_COUNT;
                        _ptr_poe_control_block->poe_init_ctrl_module_cnt = POE_APP_INIT_CTRL_MODULE_COUNT;
                    }
                    if (FALSE == _ptr_poe_control_block->reset_init_done)
                    {
                        poe_db_sendMsgToDB(M_SUBSCRIBE, POE_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0, NULL);
                        poe_db_sendMsgToDB(M_SUBSCRIBE, POE_PORT_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0, NULL);
                        _ptr_poe_control_block->reset_init_done = TRUE;
                        poe_db_init(_ptr_poe_control_block);
                        poe_chip_init();
                    }
                    if (POE_PWR_CONTROL_SW == _ptr_poe_control_block->power_strategy_control)
                    {
                        _poe_app_delayTimerHandler();
                        _poe_app_restartTimerHandler();
                        _poe_app_checkEvent();
                        _poe_app_checkSystemMode();
                        if (TRUE == _ptr_poe_control_block->system_suspend_mode)
                        {
                            _poe_app_suspendModeHandler();
                        }
                    }
                    poe_db_periodicallyUpdate(dur_tick, _ptr_poe_control_block);
#ifdef AIR_SUPPORT_POE_WATCHDOG
                    poe_watchdog_handler(dur_tick, _ptr_poe_control_block);
#endif /* AIR_SUPPORT_POE_WATCHDOG */
                    poe_maxLed_handler(_ptr_poe_control_block);
                }
                else
                {
                    if (POE_SYS_STATUS_ABNORMAL == _ptr_poe_control_block->pre_system_status)
                    {
                        if (_ptr_poe_control_block->poe_init_ctrl_module_cnt > 0)
                        {
                            _ptr_poe_control_block->poe_init_ctrl_module_cnt--;
                            _poe_app_ctrlPoeModule(POE_APP_CTRL_MODULE_INIT);
                        }
                        else
                        {
                            poe_app_deinit();
                        }
                    }
                    else
                    {
                        _poe_app_dbSetToDefault(FALSE, TRUE);
                        _poe_app_ctrlPoeModule(POE_APP_CTRL_MODULE_DEINIT);
                        poe_db_deinit();
                        poe_db_sendMsgToDB(M_UNSUBSCRIBE, POE_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0, NULL);
                        poe_db_sendMsgToDB(M_UNSUBSCRIBE, POE_PORT_CFG, DB_ALL_FIELDS, DB_ALL_ENTRIES, 0, 0, NULL);
                        _ptr_poe_control_block->pre_system_status = POE_SYS_STATUS_ABNORMAL;
                        _ptr_poe_control_block->hw_reset_done = FALSE;
                        _ptr_poe_control_block->reset_init_done = FALSE;
                    }
                }
            }
        }
        else
        {
            poe_msg_timeout = POE_MON_POLLING_INTERVAL_MS - dur_tick;
        }
    }
}

static MW_ERROR_NO_T
_poe_app_initThread(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    rc = osapi_processCreate(POE_APP_TASK_NAME,
                             POE_APP_STACK_SIZE,
                             POE_APP_THREAD_PRI,
                             _poe_app_thread,
                             NULL,
                             &_poe_app_task_handle);

    return rc;
}

static MW_ERROR_NO_T
_poe_app_deinitThread(
    void)
{
    threadhandle_t thread_handler = NULL;

    thread_handler = _poe_app_task_handle;
    if (NULL != _poe_app_task_handle)
    {
        POE_LOG_WARN("Delete PoE task!");
        _poe_app_task_handle = NULL;
        osapi_threadDelete(thread_handler);
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_poe_app_constructChipInfo(
    const UI32_T                     unit,
    const UI32_T                     port_cnt,
    const UI32_T                     device_cnt,
    const AIR_INIT_POE_PORT_MAP_T   *ptr_port_map,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map)
{
    MW_ERROR_NO_T    xRet = MW_E_OK;
    UI8_T            i = 0;
    POE_CHIP_INFO_T *ptr_chip_info = NULL;
    POE_PORT_MAP_T  *ptr_port_map_info = NULL;

    xRet = osapi_calloc(sizeof(POE_CHIP_INFO_T), POE_APP_TASK_NAME, (void **)&ptr_chip_info);
    if (MW_E_OK != xRet)
    {
        return MW_E_NO_MEMORY;
    }
    else
    {
        _ptr_poe_control_block->ptr_chip_info[unit] = ptr_chip_info;

        for (i = 0; i < port_cnt; i++)
        {
            xRet = osapi_calloc(sizeof(POE_PORT_MAP_T), POE_APP_TASK_NAME, (void **)&ptr_port_map_info);
            if (MW_E_OK != xRet)
            {
                xRet = MW_E_NO_MEMORY;
                break;
            }
            else
            {
                PORT_INFO(_ptr_poe_control_block, unit, ptr_port_map[i].port) = ptr_port_map_info;
                PORT_INFO(_ptr_poe_control_block, unit, ptr_port_map[i].port)->device_id = ptr_port_map[i].primary_port.poe_device_idx;
            }
        }

        if (MW_E_OK == xRet)
        {
            for (i = 0; i < device_cnt; i++)
            {
                UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp |= (1 << ptr_device_map[i].poe_device_idx);
            }

            _ptr_poe_control_block->total_port_cnt += port_cnt;
            _ptr_poe_control_block->total_device_cnt += device_cnt;
        }
    }

    return xRet;
}

static MW_ERROR_NO_T
_poe_app_initChipInfo(
    const UI8_T unit)
{
    MW_ERROR_NO_T              xRet = MW_E_OK;
    AIR_ERROR_NO_T             air_rc = AIR_E_OK;
    UI32_T                     port_cnt = 0, device_cnt = 0;
    AIR_INIT_POE_PORT_MAP_T   *ptr_port_map = NULL;
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map = NULL;

    xRet = osapi_calloc(sizeof(AIR_INIT_POE_PORT_MAP_T) * POE_TOTAL_PORTS_NUM, POE_APP_TASK_NAME, (void **)&ptr_port_map);
    xRet |= osapi_calloc(sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP, POE_APP_TASK_NAME, (void **)&ptr_device_map);
    if (MW_E_OK != xRet)
    {
        xRet = MW_E_NO_MEMORY;
    }
    else
    {
        air_rc |= air_init_getPoePortMap(unit, &port_cnt, ptr_port_map);
        air_rc |= air_init_getPoeDeviceMap(unit, &device_cnt, ptr_device_map);

        if ((AIR_E_OK == air_rc) && (0 != port_cnt) && (0 != device_cnt))
        {
            xRet = _poe_app_constructChipInfo(unit, port_cnt, device_cnt, ptr_port_map, ptr_device_map);
        }
        else
        {
            xRet = MW_E_NOT_SUPPORT;
        }
    }

    MW_FREE(ptr_port_map);
    MW_FREE(ptr_device_map);

    return xRet;
}

static MW_ERROR_NO_T
_poe_app_deinitChipInfo(
    const UI8_T unit)
{
    UI8_T port = 0;

    if (NULL != _ptr_poe_control_block->ptr_chip_info[unit])
    {
        for (port = 0; port < POE_TOTAL_PORTS_NUM; port++)
        {
            MW_FREE(PORT_INFO(_ptr_poe_control_block, unit, port));
        }
        MW_FREE(_ptr_poe_control_block->ptr_chip_info[unit]);
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_poe_app_initPriPortInfo(
    void)
{
    MW_ERROR_NO_T    xRet = MW_E_OK;
    PRI_PORT_INFO_T *ptr_pri_port_list = NULL;
    PRI_PORT_INFO_T *ptr_pri_plug_list = NULL;

    if (_ptr_poe_control_block->total_port_cnt > 0)
    {
        xRet = osapi_calloc(sizeof(PRI_PORT_INFO_T) * (_ptr_poe_control_block->total_port_cnt),
                            POE_APP_TASK_NAME,
                            (void **)&ptr_pri_port_list);
        if (MW_E_OK != xRet)
        {
            xRet =  MW_E_NO_MEMORY;
        }
        else
        {
            _ptr_poe_control_block->ptr_pri_port_list = ptr_pri_port_list;

            xRet = osapi_calloc(sizeof(PRI_PORT_INFO_T) * (_ptr_poe_control_block->total_port_cnt),
                                POE_APP_TASK_NAME,
                                (void **)&ptr_pri_plug_list);
            if (MW_E_OK != xRet)
            {
                xRet |= MW_E_NO_MEMORY;
            }
            else
            {
                _ptr_poe_control_block->ptr_pri_plug_list = ptr_pri_plug_list;
            }
        }
    }
    else
    {
        xRet = MW_E_NOT_SUPPORT;
    }


    return xRet;
}

static MW_ERROR_NO_T
_poe_app_deinitPriPortInfo(
    void)
{
    MW_FREE(_ptr_poe_control_block->ptr_pri_plug_list);
    MW_FREE(_ptr_poe_control_block->ptr_pri_port_list);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_poe_app_deinitRsrc(
    void)
{
    UI8_T unit = 0;

    _poe_app_deinitPriPortInfo();

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        _poe_app_deinitChipInfo(unit);
    }

    MW_FREE(_ptr_poe_control_block);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_poe_app_initRsrc(
    void)
{
    MW_ERROR_NO_T  xRet = MW_E_OK;
    UI8_T          unit = 0;
    UI32_T         avail_chip_cnt = 0;
    POE_CB_T      *ptr_sys_info = NULL;

    xRet = osapi_calloc(sizeof(POE_CB_T), POE_APP_TASK_NAME, (void **)&ptr_sys_info);
    if (MW_E_OK != xRet)
    {
        return MW_E_NO_MEMORY;
    }
    else
    {
        _ptr_poe_control_block = ptr_sys_info;

        for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
        {
            xRet = _poe_app_initChipInfo(unit);
            if (MW_E_NOT_SUPPORT == xRet)
            {
                continue;
            }
            else if (MW_E_OK != xRet)
            {
                avail_chip_cnt = 0;
                break;
            }
            else
            {
                avail_chip_cnt++;
            }
        }

        if (0 != avail_chip_cnt)
        {
            xRet = _poe_app_initPriPortInfo();
        }
    }

    /* An error occurs, the allocated memory needs to be released */
    if (MW_E_OK != xRet)
    {
        _poe_app_deinitRsrc();
    }

    return xRet;
}

/* GLOBAL VARIABLE DECLARATIONS
 */
/* FUNCTION NAME: poe_app_init
 *
 * PURPOSE:
 *      Initialize PoE application.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             --  Operation is successful.
 *      MW_E_BAD_PARAMETER  --  Bad parameter.
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_app_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if ((_ptr_poe_control_block != NULL) && (0 != _ptr_poe_control_block->inited))
    {
        rc = MW_E_ALREADY_INITED;
    }
    if (MW_E_OK == rc)
    {
        rc = _poe_app_initRsrc();
    }
    if (MW_E_OK == rc)
    {
        rc = poe_queue_init();
    }
    if (MW_E_OK == rc)
    {
        rc = poe_maxLed_init();
    }
    if (MW_E_OK == rc)
    {
        rc = _poe_app_initThread();
    }
    if (MW_E_OK == rc)
    {
        _ptr_poe_control_block->inited = 1;
    }
    return rc;
}

/* FUNCTION NAME: poe_app_deinit
 *
 * PURPOSE:
 *      De-initialize PoE application.
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             --  Operation is successful.
 *      MW_E_BAD_PARAMETER  --  Bad parameter.
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_app_deinit(
    void)
{
    UI8_T rc = MW_E_OK;

    rc = _poe_app_deinitRsrc();
    if (MW_E_OK == rc)
    {
        rc = poe_queue_deinit();
    }
    if (MW_E_OK == rc)
    {
        rc = _poe_app_deinitThread();
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_setPortPseControl
 * PURPOSE:
 *      Set pse control of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      enable                  -- TRUE: enable pse control
 *                                 FALSE:disable pse control
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T enable)
{
    UI8_T                 rc = AIR_E_OK;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;

    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (enable != PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control)
    {
        if (TRUE == enable)
        {
            if ((POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control) ||
                (0 == PORT_INFO(_ptr_poe_control_block, unit, port)->restart_count))
            {
                rc = _poe_app_setPortPseHelper(unit, port, alt, enable);
            }
            if (AIR_E_OK == rc)
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control = TRUE;
            }
        }
        else
        {
            rc = _poe_app_setPortPseHelper(unit, port, alt, enable);
            if (AIR_E_OK == rc)
            {
                if (POE_PWR_CONTROL_SW == _ptr_poe_control_block->power_strategy_control)
                {
                    PORT_INFO(_ptr_poe_control_block, unit, port)->delay_count = 0;
                    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->delay_done_pbmp, port);
                    AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->pwr_insufficient_pbmp, port);
                }
                PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control = FALSE;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getPortPseControl
 * PURPOSE:
 *      Get pse control settings of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_enable              -- TRUE: enable pse control
 *                                 FALSE:disable pse control
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortPseControl(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_enable)
{
    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    *ptr_enable = PORT_INFO(_ptr_poe_control_block, unit, port)->pse_control;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_setPortType
 * PURPOSE:
 *      Set operation mode of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      type                    -- poe type
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                -- Successfully write the data.
 *      MW_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setPortType(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_TYPE_T type)
{
    UI8_T                 rc = AIR_E_OK;
    AIR_POE_ALTERNATIVE_T alt = AIR_POE_ALTERNATIVE_NONE;
    AIR_POE_PORT_STATUS_T status = {0};
    AIR_POE_TYPE_T        air_type = AIR_POE_TYPE_LAST;

    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (POE_PORT_TYPE_AF == type)
    {
        air_type = AIR_POE_TYPE_AF;
    }
    else
    {
        air_type = AIR_POE_TYPE_AT;
    }

    rc = air_poe_getPortStatus(unit, port, alt, &status);
    if (AIR_E_OK == rc)
    {
        if (AIR_POE_POWER_STATUS_ON == status.power_status)
        {
            rc = air_poe_setPortPse(unit, port, alt, AIR_POE_PSE_MODE_DISABLE);
            rc |= air_poe_setPortType(unit, port, alt, air_type);
            rc |= air_poe_setPortPse(unit, port, alt, AIR_POE_PSE_MODE_ENABLE);
        }
        else
        {
            rc = air_poe_setPortType(unit, port, alt, air_type);
        }
    }

    if (AIR_E_OK == rc)
    {
        PORT_INFO(_ptr_poe_control_block, unit, port)->type = type;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getPortType
 * PURPOSE:
 *      Get operation mode of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_type                -- poe type
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortType(
    const UI32_T     unit,
    const UI32_T     port,
    POE_PORT_TYPE_T *ptr_type)
{
    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    *ptr_type = PORT_INFO(_ptr_poe_control_block, unit, port)->type;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_setPortPowerLimit
 * PURPOSE:
 *      Set power limit of the specified port. If the port's power consumption
 *      exceeds this level, the port will be disconnected.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      power_limit             -- power limit
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T power_limit)
{
    UI8_T rc = MW_E_OK;

    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (power_limit <= POE_APP_DEFAULT_PORT_AVAI_PWR)
    {
        rc = _poe_app_setPortPwrLimitHelper(unit, port, power_limit);
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getPortPowerLimit
 * PURPOSE:
 *      Get power limit of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_power_limit         -- power limit
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortPowerLimit(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_power_limit)
{
    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    *ptr_power_limit = PORT_INFO(_ptr_poe_control_block, unit, port)->available_power;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_setPortPriority
 * PURPOSE:
 *      Set priority of the specified port. It is used when the system
 *      power_mode is set to POWER_MODE_PRIORITY.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      priority                -- priority
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setPortPriority(
    const UI32_T              unit,
    const UI32_T              port,
    const POE_PORT_PRIORITY_T priority)
{
    I8_T  u = 0, p = 0;
    UI8_T idx = 0;

    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
    {
        return AIR_E_NOT_SUPPORT;
    }

    /* config data */
    if (POE_PORT_PRIORITY_CRITICAL == priority)
    {
        AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->critical_pbmp, port);
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->high_pbmp, port);
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp, port);
    }
    else if (POE_PORT_PRIORITY_HIGH == priority)
    {
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->critical_pbmp, port);
        AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->high_pbmp, port);
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp, port);
    }
    else
    {
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->critical_pbmp, port);
        AIR_PORT_DEL(UNIT_INFO(_ptr_poe_control_block, unit)->high_pbmp, port);
        AIR_PORT_ADD(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp, port);
    }

    /* status data */
    /* low priority ports */
    for (u = AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1; u >= 0; u--)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, u))
        {
            continue;
        }
        for (p = POE_TOTAL_PORTS_NUM - 1; p >= 0; p--)
        {
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, u)->low_pbmp, p))
            {
                _ptr_poe_control_block->ptr_pri_port_list[idx].unit = u;
                _ptr_poe_control_block->ptr_pri_port_list[idx++].air_port = p;
            }
        }
    }
    /* high priority ports */
    for (u = AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1; u >= 0; u--)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, u))
        {
            continue;
        }
        for (p = POE_TOTAL_PORTS_NUM - 1; p >= 0; p--)
        {
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, u)->high_pbmp, p))
            {
                _ptr_poe_control_block->ptr_pri_port_list[idx].unit = u;
                _ptr_poe_control_block->ptr_pri_port_list[idx++].air_port = p;
            }
        }
    }
    /* critical priority ports */
    for (u = AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM - 1; u >= 0; u--)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, u))
        {
            continue;
        }
        for (p = POE_TOTAL_PORTS_NUM - 1; p >= 0; p--)
        {
            if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, u)->critical_pbmp, p))
            {
                _ptr_poe_control_block->ptr_pri_port_list[idx].unit = u;
                _ptr_poe_control_block->ptr_pri_port_list[idx++].air_port = p;
            }
        }
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_getPortPriority
 * PURPOSE:
 *      Get priority of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      ptr_priority            -- priority
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortPriority(
    const UI32_T         unit,
    const UI32_T         port,
    POE_PORT_PRIORITY_T *ptr_priority)
{
    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
    {
        return AIR_E_NOT_SUPPORT;
    }

    if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->low_pbmp, port))
    {
        *ptr_priority = POE_PORT_PRIORITY_LOW;
    }
    else if (TRUE == AIR_PORT_CHK(UNIT_INFO(_ptr_poe_control_block, unit)->high_pbmp, port))
    {
        *ptr_priority = POE_PORT_PRIORITY_HIGH;
    }
    else
    {
        *ptr_priority = POE_PORT_PRIORITY_CRITICAL;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_setSystemPowerMode
 * PURPOSE:
 *      Set the power-on strategy of the system.
 * INPUT:
 *      power_mode              -- power mode
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_NOT_SUPPORT        -- Not supported.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setSystemPowerMode(
    const POE_SYS_POWER_MODE_T power_mode)
{
    UI8_T rc = MW_E_OK;
    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    if (POE_PWR_CONTROL_SW == _ptr_poe_control_block->power_strategy_control)
    {
        _ptr_poe_control_block->system_power_mode = power_mode;
    }
    else
    {
        rc = MW_E_NOT_SUPPORT;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getSystemPowerMode
 * PURPOSE:
 *      Get the power-on strategy of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_power_mode          -- power mode
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerMode(
    POE_SYS_POWER_MODE_T *ptr_power_mode)
{
    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    *ptr_power_mode = _ptr_poe_control_block->system_power_mode;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_setSystemPowerBudget
 * PURPOSE:
 *      Set the power budget of the system.
 * INPUT:
 *      power_limit             -- power limit
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_setSystemPowerBudget(
    const UI32_T power_limit)
{
    UI8_T          rc = MW_E_OK;
    UI8_T          unit = 0, device = 0;

    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    if (power_limit <= _ptr_poe_control_block->total_port_cnt * 300)
    {
        if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
        {
            for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
            {
                if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
                {
                    continue;
                }
                for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
                {
                    if (0 == (UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp & BIT(device)))
                    {
                        continue;
                    }

                    _poe_app_setSysPwrLimitHelper(unit, device, power_limit);
                }
            }
        }
        _ptr_poe_control_block->total_available_power = power_limit;
    }
    else
    {
        rc = MW_E_BAD_PARAMETER;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getSystemPowerBudget
 * PURPOSE:
 *      Set the power budget of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_power_limit         -- power limit
 * RETURN:
 *      MW_E_OK                -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerBudget(
    UI32_T *ptr_power_limit)
{
    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    *ptr_power_limit = _ptr_poe_control_block->total_available_power;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_getPortStatus
 * PURPOSE:
 *      Get the status of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- port alternative
 * OUTPUT:
 *      ptr_status              -- port status
 * RETURN:
 *      MW_E_OK                -- Successfully write the data.
 *      MW_E_BAD_PARAMETER     -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortStatus(
    const UI32_T          unit,
    const UI32_T          port,
    const POE_PORT_ALT_T  alt,
    POE_PORT_OP_STATUS_T *ptr_status)
{
    UI8_T                 rc = AIR_E_OK;
    AIR_POE_PORT_STATUS_T air_status = {0};
    AIR_POE_EVENT_T       air_event = {0};
    AIR_POE_ALTERNATIVE_T air_alt = AIR_POE_ALTERNATIVE_LAST;

    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    if (POE_PORT_ALT_PRIMARY == alt)
    {
        air_alt = AIR_POE_ALTERNATIVE_PRIMARY;
    }
    else if (POE_PORT_ALT_SECONDARY == alt)
    {
        air_alt = AIR_POE_ALTERNATIVE_SECONDARY;
    }
    else
    {
        air_alt = AIR_POE_ALTERNATIVE_NONE;
    }

    rc = air_poe_getPortStatus(unit, port, air_alt, &air_status);
    if (AIR_E_OK == rc)
    {
        /* PD signature */
        if (AIR_POE_SIGNATURE_BAD == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_BAD;
        }
        else if (AIR_POE_SIGNATURE_GOOD == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_GOOD;
        }
        else if (AIR_POE_SIGNATURE_OPEN == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_OPEN;
        }
        else if (AIR_POE_SIGNATURE_LARGE == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_LARGE;
        }
        else if (AIR_POE_SIGNATURE_LOW == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_LOW;
        }
        else if (AIR_POE_SIGNATURE_HIGH == air_status.pd_signature)
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_HIGH;
        }
        else
        {
            ptr_status->pd_signature = POE_PORT_SIGNATURE_LAST;
        }

        /* PD class */
        if (AIR_POE_CLASS_0 == air_status.pd_class)
        {
            ptr_status->pd_class = POE_PORT_CLASS_0;
        }
        else if (AIR_POE_CLASS_1 == air_status.pd_class)
        {
            ptr_status->pd_class = POE_PORT_CLASS_1;
        }
        else if (AIR_POE_CLASS_2 == air_status.pd_class)
        {
            ptr_status->pd_class = POE_PORT_CLASS_2;
        }
        else if (AIR_POE_CLASS_3 == air_status.pd_class)
        {
            ptr_status->pd_class = POE_PORT_CLASS_3;
        }
        else if (AIR_POE_CLASS_4 == air_status.pd_class)
        {
            ptr_status->pd_class = POE_PORT_CLASS_4;
        }
        else
        {
            ptr_status->pd_class = POE_PORT_CLASS_LAST;
        }

        /* port power status */
        if (AIR_POE_POWER_STATUS_OFF == air_status.power_status)
        {
            ptr_status->power_status = POE_PORT_POWER_STATUS_OFF;
        }
        else
        {
            ptr_status->power_status = POE_PORT_POWER_STATUS_ON;
        }

        /* port pse status */
        ptr_status->pse_mode = PORT_INFO(_ptr_poe_control_block, unit, port)->pse_mode;

        /* port event */
        if (POE_PWR_CONTROL_HW == _ptr_poe_control_block->power_strategy_control)
        {
            rc |= air_poe_getPortEvent(unit, port, air_alt, &air_event);
            rc |= air_poe_clearPortEvent(unit, port, alt);
            if (AIR_E_OK == rc)
            {
                PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags |= air_event.event_flags;
            }
        }
        ptr_status->event_flags = PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getPortMeasurement
 * PURPOSE:
 *      Get the measurement of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 *      alt                     -- port alternative
 * OUTPUT:
 *      ptr_meas                -- port measurement
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 *      MW_E_BAD_PARAMETER      -- Invalid parameter.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getPortMeasurement(
    const UI32_T         unit,
    const UI32_T         port,
    const POE_PORT_ALT_T alt,
    POE_PORT_MEAS_T     *ptr_meas)
{
    UI8_T                      rc = AIR_E_OK;
    AIR_POE_PORT_MEASUREMENT_T air_port_meas = {0};
    AIR_POE_ALTERNATIVE_T      air_alt = AIR_POE_ALTERNATIVE_LAST;

    if (POE_PORT_ALT_PRIMARY == alt)
    {
        air_alt = AIR_POE_ALTERNATIVE_PRIMARY;
    }
    else if (POE_PORT_ALT_SECONDARY == alt)
    {
        air_alt = AIR_POE_ALTERNATIVE_SECONDARY;
    }
    else
    {
        air_alt = AIR_POE_ALTERNATIVE_NONE;
    }

    rc = air_poe_getPortMeasurement(unit, port, air_alt, &air_port_meas);
    if (AIR_E_OK == rc)
    {
        ptr_meas->current = air_port_meas.current;
        ptr_meas->voltage = air_port_meas.voltage;
        ptr_meas->temperature = air_port_meas.temperature;
        ptr_meas->power = (air_port_meas.current / 100) * (air_port_meas.voltage / 100) / 100;
    }

    return rc;
}

/* FUNCTION NAME:   poe_app_getSystemPowerSupply
 * PURPOSE:
 *      Get the power supply of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_supply              -- port supply
 * RETURN:
 *      MW_E_OK                 -- Successfully write the data.
 *      MW_E_OP_INVALID         -- Operation is invalid.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerSupply(
    POE_SYS_SUPPLY_T *ptr_supply)
{
    UI8_T                        rc = AIR_E_OP_INVALID;
    UI32_T                       unit = 0, device = 0;
    AIR_POE_DEVICE_MEASUREMENT_T device_meas = {0};

    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(_ptr_poe_control_block, unit))
        {
            continue;
        }
        for (device = 0; device < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; device++)
        {
            if (0 == (UNIT_INFO(_ptr_poe_control_block, unit)->device_pbmp & BIT(device)))
            {
                continue;
            }
            rc = air_poe_getDeviceMeasurement(unit, device, &device_meas);
            if (AIR_E_OK == rc)
            {
                break;
            }
            else
            {
                POE_LOG_ERROR("u:%d/dev:%d get device measurement failed(%d)\n", unit, device, rc);
            }
        }
        if (AIR_E_OK == rc)
        {
            break;
        }
    }

    if (POE_PWR_CONTROL_SW == _ptr_poe_control_block->power_strategy_control)
    {
        ptr_supply->allocated_power = _ptr_poe_control_block->total_allocated_power;
    }
    else /* POE_PWR_CONTROL_HW */
    {
        ptr_supply->allocated_power = device_meas.consume_power / 10;
    }
    ptr_supply->available_power = _ptr_poe_control_block->total_available_power - ptr_supply->allocated_power;
    ptr_supply->supply_voltage = device_meas.supply_voltage;

    return rc;
}

/* FUNCTION NAME:   poe_app_getSystemPowerStrategy
 * PURPOSE:
 *      Get the power strategy control of the system.
 * INPUT:
 *      None
 * OUTPUT:
 *      ptr_strategy            -- port strategy
 * RETURN:
 *      MW_E_OK                 -- Successfully get the data.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_getSystemPowerStrategy(
    POE_PWR_CONTROL_T *ptr_strategy)
{
    POE_APP_CHECK_PTR(_ptr_poe_control_block);

    *ptr_strategy = _ptr_poe_control_block->power_strategy_control;

    return MW_E_OK;
}

/* FUNCTION NAME:   poe_app_clearPortEvent
 * PURPOSE:
 *      Clear the PoE events of the specified port.
 * INPUT:
 *      unit                    -- unit id
 *      port                    -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK                 -- Successfully clear the event.
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_app_clearPortEvent(
    const UI32_T unit,
    const UI32_T port)
{
    POE_APP_CHECK_PTR(PORT_INFO(_ptr_poe_control_block, unit, port));

    PORT_INFO(_ptr_poe_control_block, unit, port)->event_flags = 0;

    return MW_E_OK;
}

/* FUNCTION NAME: poe_app_getPoeMaxPower
 * PURPOSE:
 *      get poe max power budget value
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      UI32_T                   -- max power budget
 * NOTES:
 *      None
 */
UI32_T
poe_app_getPoeMaxPower(
    void)
{
    return POE_MAX_PWR;
}

/* FUNCTION NAME: poe_app_checkPseStatus
 * PURPOSE:
 *      check that PoE is working normally
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      BOOL_T                  -- TRUE: PSE working normally.
 *                                 FALSE:PSE working abnormally.
 * NOTES:
 *
 */
BOOL_T
poe_app_checkPseStatus(
    void)
{
    BOOL_T pse_status = FALSE;

    if ((NULL != _ptr_poe_control_block) && (POE_SYS_STATUS_NORMAL == _ptr_poe_control_block->pre_system_status))
    {
        pse_status = TRUE;
    }

    return pse_status;
}