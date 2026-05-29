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

/* FILE NAME:   poe_db_relate.c
 * PURPOSE:
 *      Define poe db function.
 *
 * NOTES:
 */


/* INCLUDE FILE DECLARATIONS
 */
#include <poe_db_relate.h>
#include <poe_main.h>
#include <poe_queue.h>
#include <poe_info.h>
#include <db_api.h>
#include <db_data.h>
#include <osapi_message.h>
#include <hal_poe.h>
#include <air_init.h>
/* NAMING CONSTANT DECLARATIONS
*/
/* MACRO FUNCTION DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
*/
static POE_PORT_INFO_T    *_ptr_port_info[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM] = {NULL};
static POE_DEVICE_INFO_T  *_ptr_device_info[AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP] = {NULL};
static UI8_T               _poe_port_cnt[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM] = {0};
/* STATIC VARIABLE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
static MW_ERROR_NO_T
_poe_db_appendMsgPayload(
    DB_REQUEST_TYPE_T *ptr_request,
    UI8_T *ptr_data,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload);

static MW_ERROR_NO_T
_poe_db_parseDBData(
    const UI8_T port_cnt,
    UI8_T *ptr_poe_port_cfg,
    UI8_T field_idx,
    UI8_T entry_idx,
    void *ptr_data);

static MW_ERROR_NO_T
_poe_db_processSystemCfgAllFileds(
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block);

static MW_ERROR_NO_T
_poe_db_processSystemCfg(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block);

static MW_ERROR_NO_T
_poe_db_processPortCfgAllFileds(
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block);

static MW_ERROR_NO_T
_poe_db_processPortCfg(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block);

static MW_ERROR_NO_T
_poe_db_processMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block);

static MW_ERROR_NO_T
_poe_db_sendMsg(
    DB_MSG_T *ptr_msg,
    UI32_T size);
/* LOCAL SUBPROGRAM BODIES
*/
static MW_ERROR_NO_T
_poe_db_appendMsgPayload(
    DB_REQUEST_TYPE_T *ptr_request,
    UI8_T *ptr_data,
    DB_MSG_T **pptr_msg,
    UI16_T *ptr_msg_size,
    UI8_T **pptr_shifted_payload)
{
    UI8_T         rc = MW_E_OK;
    UI32_T        payload_size = 0;

    if (NULL == (*pptr_msg))
    {
        rc =  MW_E_BAD_PARAMETER;
    }

    if ((MW_E_OK == rc) && (((*pptr_msg)->type.count) & DB_COUNT_REUSE_FLAG))
    {
        rc = dbapi_sendMsg((*pptr_msg), MSG_TIMEOUT_WAIT_INDEFINITELY);
        if (MW_E_OK == rc)
        {
            payload_size = sizeof(DB_POE_STATUS_T) + sizeof(DB_POE_PORT_STATUS_T);
            (*pptr_msg) = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, ptr_msg_size, pptr_shifted_payload);
            if (NULL == (*pptr_msg))
            {
                rc = MW_E_NO_MEMORY;
            }
        }
        else
        {
            (*pptr_msg) = NULL;
        }
    }

    if (MW_E_OK == rc)
    {
        rc = dbapi_appendMsgPayload(ptr_request, ptr_data, pptr_msg, ptr_msg_size, pptr_shifted_payload);
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_db_parseDBData(
    const UI8_T port_cnt,
    UI8_T *ptr_poe_port_cfg,
    UI8_T field_idx,
    UI8_T entry_idx,
    void *ptr_data)
{
    UI8_T         rc = MW_E_OTHERS;
    UI32_T        offset = 0, i = 0;

    if (entry_idx == 0)
    {
        rc = MW_E_BAD_PARAMETER;
    }
    else
    {
        for (i = PORT_POWER_CONTROL; i < POE_PORT_CFG_LAST; i++)
        {
            if (i == field_idx)
            {
                offset += db_getTableFieldSize(POE_PORT_CFG, i) * (entry_idx - 1);
                osapi_memcpy(ptr_data, ptr_poe_port_cfg + offset, db_getTableFieldSize(POE_PORT_CFG, i));
                rc =  MW_E_OK;
                break;
            }

            offset += db_getTableFieldSize(POE_PORT_CFG, i) * port_cnt;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_db_processSystemCfgAllFileds(
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T rc = MW_E_OK;
    UI16_T total_available_power = 0;
#ifdef AIR_SUPPORT_POE_WATCHDOG
    UI16_T tmp_value = 0;
    UI32_T new_value = 0;
#endif /* AIR_SUPPORT_POE_WATCHDOG */

    /* TOTAL_AVAILABLE_POWER */
    osapi_memcpy((UI8_T *)&total_available_power, (UI8_T *)ptr_data, sizeof(UI16_T));
    rc = poe_setSystemPowerBudget((UI32_T)total_available_power);
    if (MW_E_OK != rc)
    {
        POE_LOG_ERROR("Set System Power Budget failed(%d)", rc);
    }
#ifdef AIR_SUPPORT_POE_WATCHDOG
    /* POE_WATCHDOG_PERIOD */
    osapi_memcpy(&tmp_value, ptr_data + sizeof(UI16_T), sizeof(UI16_T));
    new_value = (UI32_T)tmp_value * 1000;

    if (ptr_poe_control_block->poe_watchdog_period != new_value)
    {
        POE_LOG_INFO("PoE Watchdog Period changed from %u ms to %u ms",
            ptr_poe_control_block->poe_watchdog_period, new_value);
        ptr_poe_control_block->poe_watchdog_period = new_value;
        ptr_poe_control_block->poe_watchdog_updated = TRUE;
    }

    /* POE_WATCHDOG_THRESHOLD */
    osapi_memcpy(&tmp_value, ptr_data + sizeof(UI16_T) + sizeof(UI16_T), sizeof(UI16_T));
    if (ptr_poe_control_block->poe_watchdog_threshold != tmp_value)
    {
        POE_LOG_INFO("PoE Watchdog Packet Threshold changed from %u to %u",
            ptr_poe_control_block->poe_watchdog_threshold, tmp_value);
        ptr_poe_control_block->poe_watchdog_threshold = tmp_value;
        ptr_poe_control_block->poe_watchdog_updated = TRUE;
    }
#endif /* AIR_SUPPORT_POE_WATCHDOG */

    return rc;
}

static MW_ERROR_NO_T
_poe_db_processSystemCfg(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T rc = MW_E_OK;

    switch (ptr_request->f_idx)
    {
        case TOTAL_AVAILABLE_POWER:
        {
            UI16_T total_available_power = 0;

            osapi_memcpy((UI8_T *)&total_available_power, (UI8_T *)ptr_data, sizeof(UI16_T));
            rc = poe_setSystemPowerBudget((UI32_T)total_available_power);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set System Power Budget failed(%d)", rc);
            }
            break;
        }
#ifdef AIR_SUPPORT_POE_WATCHDOG
        case POE_WATCHDOG_PERIOD:
        {
            UI16_T tmp_value = 0;
            UI32_T new_value = 0;

            osapi_memcpy(&tmp_value, ptr_data, sizeof(UI16_T));
            new_value = (UI32_T)tmp_value * 1000;

            if (ptr_poe_control_block->poe_watchdog_period != new_value)
            {
                POE_LOG_INFO("PoE Watchdog Period changed from %u ms to %u ms",
                    ptr_poe_control_block->poe_watchdog_period, new_value);
                ptr_poe_control_block->poe_watchdog_period = new_value;
                ptr_poe_control_block->poe_watchdog_updated = TRUE;
            }
            break;
        }
        case POE_WATCHDOG_THRESHOLD:
        {
            UI16_T tmp_value = 0;

            osapi_memcpy(&tmp_value, ptr_data, sizeof(UI16_T));
            if (ptr_poe_control_block->poe_watchdog_threshold != tmp_value)
            {
                POE_LOG_INFO("PoE Watchdog Packet Threshold changed from %u to %u",
                    ptr_poe_control_block->poe_watchdog_threshold, tmp_value);
                ptr_poe_control_block->poe_watchdog_threshold = tmp_value;
                ptr_poe_control_block->poe_watchdog_updated = TRUE;
            }
            break;
        }
#endif /* AIR_SUPPORT_POE_WATCHDOG */
        case DB_ALL_FIELDS:
        {
            rc = _poe_db_processSystemCfgAllFileds(ptr_data, ptr_poe_control_block);
            break;
        }
        default:
        {
            POE_LOG_DEBUG("recv unknown field: [%d]", ptr_request->f_idx);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_db_processPortCfgAllFileds(
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T rc = MW_E_OK;
    UI8_T                port = 0, data = 0;
    UI16_T               port_available_power = 0;
    UI32_T               cfg_size = 0;
    BOOL_T               pse_ctrl = FALSE;
    DB_POE_PORT_CFG_T   *ptr_poe_port_cfg = NULL;
    POE_PORT_PRIORITY_T  priority = POE_PORT_PRIORITY_LAST;
    POE_PORT_TYPE_T      type = POE_PORT_TYPE_LAST;
    POE_PWR_CONTROL_T    strategy = POE_PWR_CONTROL_LAST;

    cfg_size = sizeof(DB_POE_PORT_CFG_T) * ptr_poe_control_block->total_port_cnt;
    rc = osapi_calloc(cfg_size, POE_APP_TASK_NAME, (void **)&ptr_poe_port_cfg);
    if (MW_E_OK != rc)
    {
        POE_LOG_ERROR("allocate memory failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }
    osapi_memcpy((UI8_T *)ptr_poe_port_cfg, (UI8_T *)ptr_data, cfg_size);

    for (port = 0; port < AIR_MAX_PORT_NUM; port++)
    {
        if (NULL != PORT_INFO(ptr_poe_control_block, 0, port))
        {
            /* PORT_POWER_CONTROL */
            _poe_db_parseDBData(ptr_poe_control_block->total_port_cnt, (UI8_T *)ptr_poe_port_cfg, PORT_POWER_CONTROL, port, &data);
            if (POE_PORT_PSE_CTRL_EN == data)
            {
                pse_ctrl = TRUE;
            }
            else if (POE_PORT_PSE_CTRL_DIS == data)
            {
                pse_ctrl = FALSE;
            }
            else
            {
                POE_LOG_ERROR("PORT_POWER_CONTROL value is invalid");
            }
            rc = poe_setPortPseControl(0, port, pse_ctrl);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] pse control failed(%d)", port, rc);
            }

            /* PORT_AVAILABLE_POWER */
            _poe_db_parseDBData(ptr_poe_control_block->total_port_cnt, (UI8_T *)ptr_poe_port_cfg, PORT_AVAILABLE_POWER, port, &port_available_power);
            rc = poe_setPortPowerLimit(0, port, port_available_power);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] available power failed(%d)", port, rc);
            }

            /* PORT_PRIORITY */
            rc = poe_getSystemPowerStrategy(&strategy);
            if ((AIR_E_OK == rc) && (POE_PWR_CONTROL_SW == strategy))
            {
                _poe_db_parseDBData(ptr_poe_control_block->total_port_cnt, (UI8_T *)ptr_poe_port_cfg, PORT_PRIORITY, port, &data);
                if (POE_PORT_PRI_LOW == data)
                {
                    priority = POE_PORT_PRIORITY_LOW;
                }
                else if (POE_PORT_PRI_HIGH == data)
                {
                    priority = POE_PORT_PRIORITY_HIGH;
                }
                else if (POE_PORT_PRIO_CRITICAL == data)
                {
                    priority = POE_PORT_PRIORITY_CRITICAL;
                }
                else
                {
                    POE_LOG_ERROR("PORT_PRIORITY value is invalid");
                }
                rc = poe_setPortPriority(0, port, priority);
                if (MW_E_OK != rc)
                {
                    POE_LOG_ERROR("Set port[%d] priority failed(%d)", port, rc);
                }
            }

            /* PORT_AF_AT_MODE */
            _poe_db_parseDBData(ptr_poe_control_block->total_port_cnt, (UI8_T *)ptr_poe_port_cfg, PORT_AF_AT_MODE, port, &data);
            if (POE_AF_MODE == data)
            {
                type = POE_PORT_TYPE_AF;
            }
            else if (POE_AT_MODE == data)
            {
                type = POE_PORT_TYPE_AT;
            }
            else
            {
                POE_LOG_ERROR("PORT_AF_AT_MODE value is invalid");
            }
            rc = poe_setPortType(0, port, type);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] type failed(%d)", port, rc);
            }
#ifdef AIR_SUPPORT_POE_WATCHDOG
            /* PORT_POE_WATCHDOG_ENABLE */
            _poe_db_parseDBData(ptr_poe_control_block->total_port_cnt, (UI8_T *)ptr_poe_port_cfg, PORT_POE_WATCHDOG_ENABLE, port, &data);
            PORT_INFO(ptr_poe_control_block, 0, port)->poe_watchdog_mode =
                (data != 0) ? TRUE : FALSE;
            POE_LOG_INFO("port[%d] PoE Watchdog mode set to %s",
                        port,
                        (data != 0) ? "ENABLED" : "DISABLED");
#endif /* AIR_SUPPORT_POE_WATCHDOG */
        }
    }
    MW_FREE(ptr_poe_port_cfg);

    return rc;
}

static MW_ERROR_NO_T
_poe_db_processPortCfg(
    const DB_REQUEST_TYPE_T *ptr_request,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T rc = MW_E_OK;

    switch (ptr_request->f_idx)
    {
        case PORT_POWER_CONTROL:
        {
            UI8_T data = 0;
            BOOL_T pse_ctrl = FALSE;

            osapi_memcpy((UI8_T *)&data, (UI8_T *)ptr_data, sizeof(UI8_T));
            if (POE_PORT_PSE_CTRL_EN == data)
            {
                pse_ctrl = TRUE;
            }
            else if (POE_PORT_PSE_CTRL_DIS == data)
            {
                pse_ctrl = FALSE;
            }
            else
            {
                POE_LOG_ERROR("PORT_POWER_CONTROL value is invalid");
            }
            rc = poe_setPortPseControl(0, ptr_request->e_idx, pse_ctrl);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] pse control failed(%d)", ptr_request->e_idx, rc);
            }
            else
            {
                POE_LOG_INFO("port[%d] pse control set to %s successfully",
                                ptr_request->e_idx,
                                pse_ctrl ? "ENABLED" : "DISABLED");
            }
            break;
        }
        case PORT_AVAILABLE_POWER:
        {
            UI16_T port_available_power = 0;

            osapi_memcpy((UI8_T *)&port_available_power, (UI8_T *)ptr_data, sizeof(UI16_T));
            rc = poe_setPortPowerLimit(0, ptr_request->e_idx, port_available_power);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] available power failed(%d)", ptr_request->e_idx, rc);
            }
            else
            {
                POE_LOG_INFO("port[%d] available power set to %d successfully",
                                ptr_request->e_idx,
                                port_available_power);
            }
            break;
        }
        case PORT_PRIORITY:
        {
            UI8_T data = 0;
            POE_PORT_PRIORITY_T priority = POE_PORT_PRIORITY_LAST;
            POE_PWR_CONTROL_T strategy = POE_PWR_CONTROL_LAST;

            rc = poe_getSystemPowerStrategy(&strategy);
            if ((AIR_E_OK == rc) && (POE_PWR_CONTROL_SW == strategy))
            {
                osapi_memcpy((UI8_T *)&data, (UI8_T *)ptr_data, sizeof(UI8_T));
                if (POE_PORT_PRI_LOW == data)
                {
                    priority = POE_PORT_PRIORITY_LOW;
                }
                else if (POE_PORT_PRI_HIGH == data)
                {
                    priority = POE_PORT_PRIORITY_HIGH;
                }
                else if (POE_PORT_PRIO_CRITICAL == data)
                {
                    priority = POE_PORT_PRIORITY_CRITICAL;
                }
                else
                {
                    POE_LOG_ERROR("PORT_PRIORITY value is invalid");
                }
                rc = poe_setPortPriority(0, ptr_request->e_idx, priority);
                if (MW_E_OK != rc)
                {
                    POE_LOG_ERROR("Set port[%d] priority failed(%d)", ptr_request->e_idx, rc);
                }
                else
                {
                    POE_LOG_INFO("port[%d] priority set to %d successfully",
                                ptr_request->e_idx,
                                priority);
                }
            }
            break;
        }
        case PORT_AF_AT_MODE:
        {
            UI8_T data = 0;
            POE_PORT_TYPE_T type = POE_PORT_TYPE_LAST;

            osapi_memcpy((UI8_T *)&data, (UI8_T *)ptr_data, sizeof(UI8_T));
            if (POE_AF_MODE == data)
            {
                type = POE_PORT_TYPE_AF;
            }
            else if (POE_AT_MODE == data)
            {
                type = POE_PORT_TYPE_AT;
            }
            else
            {
                POE_LOG_ERROR("PORT_AF_AT_MODE value is invalid");
            }
            rc = poe_setPortType(0, ptr_request->e_idx, type);
            if (MW_E_OK != rc)
            {
                POE_LOG_ERROR("Set port[%d] type failed(%d)", rc);
            }
            else
            {
                POE_LOG_INFO("port[%d] port type set to %d successfully",
                                ptr_request->e_idx,
                                type);
            }
            break;
        }
#ifdef AIR_SUPPORT_POE_WATCHDOG
        case PORT_POE_WATCHDOG_ENABLE:
        {
            UI8_T data = 0;

            osapi_memcpy((UI8_T *)&data, (UI8_T *)ptr_data, sizeof(UI8_T));
            PORT_INFO(ptr_poe_control_block, 0, ptr_request->e_idx)->poe_watchdog_mode =
                (data != 0) ? TRUE : FALSE;
            POE_LOG_INFO("port[%d] PoE Watchdog mode set to %s",
                            ptr_request->e_idx,
                            (data != 0) ? "ENABLED" : "DISABLED");
            break;
        }
#endif /* AIR_SUPPORT_POE_WATCHDOG */
        case DB_ALL_FIELDS:
        {
            rc = _poe_db_processPortCfgAllFileds(ptr_data, ptr_poe_control_block);
            break;
        }
        default:
        {
            POE_LOG_DEBUG("recv unknown field: [%d]", ptr_request->f_idx);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_db_processMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void  *ptr_data,
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T rc = MW_E_OK;

    if ((NULL == ptr_request) || (NULL == ptr_data) || (0 == data_size))
    {
        return MW_E_BAD_PARAMETER;
    }

    if ((M_GET == method) || (M_UPDATE == method))
    {
        if (POE_PORT_CFG == ptr_request->t_idx)
        {
            rc = _poe_db_processPortCfg(ptr_request, ptr_data, ptr_poe_control_block);
        }
        else if (POE_CFG == ptr_request->t_idx)
        {
            rc = _poe_db_processSystemCfg(ptr_request, ptr_data, ptr_poe_control_block);
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_poe_db_sendMsg(
    DB_MSG_T *ptr_msg,
    UI32_T size)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_CHECK_PTR(ptr_msg);

    rc = dbapi_dbisReady();
    if (MW_E_OK != rc)
    {
        /* This message could not be send, drop it */
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = dbapi_sendRequesttoDb(size, ptr_msg);
        if (MW_E_OK != rc)
        {
            /* This message could not be send, drop it */
            POE_LOG_ERROR("dbapi_sendRequesttoDb() failed");
            MW_FREE(ptr_msg);
        }
    }

    return rc;
}
/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME: poe_db_sendMsgToDB
 * PURPOSE:
 *      Send message to DB.
 *
 * INPUT:
 *      method                     -- method of the message
 *      t_idx                      -- index of the Table
 *      f_idx                      -- index of the field
 *      e_idx                      -- index of the entry
 *      ptr_data                   -- pointer to the data
 *      size                       -- size of the data
 *
 * OUTPUT:
 *      pptr_out_msg               -- pointer to the output message
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *
 */
MW_ERROR_NO_T
poe_db_sendMsgToDB(
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI16_T size,
    DB_MSG_T **pptr_out_msg)
{
    MW_ERROR_NO_T    rc = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T           msg_size = 0;

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(msg_size,
                      POE_APP_TASK_NAME,
                      (void **)&ptr_msg);
    if (MW_E_OK != rc)
    {
        POE_LOG_ERROR("allocate memory failed(%d)", rc);
        return MW_E_NO_MEMORY;
    }

    /* message */
    if (M_SUBSCRIBE == method || M_UNSUBSCRIBE == method)
    {
        osapi_strncpy(ptr_msg->cq_name, POE_QUEUE_NAME, DB_Q_NAME_SIZE);
    }
    else if (M_GET == method)
    {
        osapi_strncpy(ptr_msg->cq_name, POE_DB_GET_QUEUE_NAME, DB_Q_NAME_SIZE);
    }
    ptr_msg->method = method;
    ptr_msg->type.count = 1;

    /* payload */
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg->ptr_payload);
    ptr_payload->request.t_idx = t_idx;
    ptr_payload->request.f_idx = f_idx;
    ptr_payload->request.e_idx = e_idx;
    ptr_payload->data_size = size;
    POE_LOG_DEBUG("[%p]send method:0x%x, table:%d, field:%d, entry:%d, queue:[%s], data:[%d], size:[%d]",
                  ptr_msg,
                  method,
                  ptr_payload->request.t_idx,
                  ptr_payload->request.f_idx,
                  ptr_payload->request.e_idx,
                  ptr_msg->cq_name,
                  ((UI8_T *)&(ptr_payload->ptr_data))[0],
                  ptr_payload->data_size);
    if ((size > 0) && (method != M_GET))
    {
        memcpy(&(ptr_payload->ptr_data), ptr_data, size);
    }

    /* Send message to DB */
    rc = _poe_db_sendMsg(ptr_msg, msg_size);
    if (rc == MW_E_OK)
    {
        if (NULL != pptr_out_msg)
        {
            *pptr_out_msg = ptr_msg;
        }
    }
    else
    {
        POE_LOG_ERROR("Send message to DB failed(%d)", rc);
        osapi_free(ptr_msg);
        return MW_E_OP_INCOMPLETE;
    }

    return MW_E_OK;
}

/* FUNCTION NAME: poe_db_handleMsg
 * PURPOSE:
 *      Process message received from DB
 *
 * INPUT:
 *      ptr_msg                    -- pointer to the database message
 *
 * OUTPUT:
 *      ptr_poe_control_block      -- Pointer to the related PoE data
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
poe_db_handleMsg(
    POE_CB_T *ptr_poe_control_block,
    DB_MSG_T *ptr_msg)
{
    MW_ERROR_NO_T      rc = MW_E_OK;
    UI8_T              req_cnt = 0;
    DB_REQUEST_TYPE_T  request = { 0 };
    UI16_T             parse_data_size = 0;
    UI8_T             *ptr_parse_data = NULL;
    UI8_T             *ptr_parse_payload_data = NULL;

    req_cnt = (ptr_msg->method != M_RESPONSE) ? ptr_msg->type.count : 1;
    do
    {
        rc = dbapi_parseMsg(ptr_msg, req_cnt, &request, &parse_data_size, &ptr_parse_data, &ptr_parse_payload_data);
        if (MW_E_OK == rc)
        {
            POE_LOG_DEBUG("recv method:0x%x, [t/f/e]:[%d/%d/%d]",
                      ptr_msg->method, request.t_idx, request.f_idx, request.e_idx);
            rc = _poe_db_processMsg(ptr_msg->method, &request, parse_data_size, ptr_parse_data, ptr_poe_control_block);
            if (MW_E_OK != rc)
            {
                POE_LOG_WARN("handle dbmsg: failed!");
            }
        }
    } while ((MW_E_OK == rc) && (NULL != ptr_parse_payload_data));
}

/* FUNCTION NAME: poe_db_periodicallyUpdate
 * PURPOSE:
 *      Periodically update PoE DB.
 *
 * INPUT:
 *      dur_tick                   -- Duration in system ticks
 *
 * OUTPUT:
 *      ptr_poe_control_block      -- Pointer to the related PoE data
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *
 */
void
poe_db_periodicallyUpdate(
    const UI32_T     dur_tick,
    POE_CB_T        *ptr_poe_control_block)
{
    UI8_T                ret = 0, unit = 0, port = 0, poe_port = 0, poe_device_idx = 0, i = 0, j = 0, k = 0;
    UI8_T               *ptr_payload = NULL;
    UI16_T               msg_size = 0;
    UI32_T               data = 0, payload_size = 0;
    DB_REQUEST_TYPE_T    request = {0};
    DB_MSG_T            *ptr_multi_msg = NULL;
    POE_PORT_ALT_T       alt = POE_PORT_ALT_NONE;
    POE_SYS_SUPPLY_T     supply = {0};
    POE_PORT_OP_STATUS_T status = {0};
    POE_PORT_MEAS_T      meas = {0};

    ptr_poe_control_block->poe_periodic_update_dur += dur_tick;
    if (ptr_poe_control_block->poe_periodic_update_dur >= POE_WATCHDOG_PERIODIC_UPDATE_INTERVAL)
    {
        ptr_poe_control_block->poe_periodic_update_dur = 0;

        payload_size = sizeof(DB_POE_STATUS_T) + sizeof(DB_POE_PORT_STATUS_T);
        ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, &msg_size, &ptr_payload);
        if (NULL == ptr_multi_msg)
        {
            POE_LOG_ERROR("Fail to create DB messge. ret=%d", ret);
            ret = MW_E_OTHERS;
        }

        if (MW_E_OK == ret)
        {
            ret = poe_getSystemPowerSupply(&supply);
        }
        if (MW_E_OK == ret)
        {
            request.t_idx = POE_STATUS;
            request.f_idx = TOTAL_CONSUMED_POWER;
            request.e_idx = DB_ALL_ENTRIES;
            data = supply.allocated_power;
            ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != ret)
            {
                POE_LOG_ERROR("Fail to append PORT_CONSUMED_POWER msg payload. ret=%d", ret);
                MW_FREE(ptr_multi_msg);
            }

            if (MW_E_OK == ret)
            {
                request.t_idx = POE_STATUS;
                request.f_idx = SUPPLY_VOLTAGE;
                request.e_idx = DB_ALL_ENTRIES;
                data = supply.supply_voltage;
                ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                if (MW_E_OK != ret)
                {
                    POE_LOG_ERROR("Fail to append PORT_CONSUMED_POWER msg payload. ret=%d", ret);
                    MW_FREE(ptr_multi_msg);
                }
            }

            if (MW_E_OK == ret)
            {
                request.t_idx = POE_STATUS;
                request.f_idx = TOTAL_CONSUMED_CURRENT;
                request.e_idx = DB_ALL_ENTRIES;
                data = supply.supply_voltage == 0 ? 0 : (supply.allocated_power * 10000 / supply.supply_voltage);
                ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                if (MW_E_OK != ret)
                {
                    POE_LOG_ERROR("Fail to append PORT_CONSUMED_POWER msg payload. ret=%d", ret);
                    MW_FREE(ptr_multi_msg);
                }
            }
        }

        if ((MW_E_OK == ret) && (POE_PWR_CONTROL_HW == ptr_poe_control_block->power_strategy_control))
        {
            for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
            {
                if (NULL != _ptr_device_info[i])
                {
                    if (POE_DEVICE_ID_AN8503 == _ptr_device_info[i]->revision_id)
                    {
                        ret = hal_poe_readReg(0, i,
                                              HAL_POE_PAGE_1,
                                              POE_AN8503_TOTAL_POWER_LIMITER_EVENT,
                                              HAL_POE_REG_LEN_1,
                                              &data);
                        if(AIR_E_OK == ret)
                        {
                            _ptr_device_info[i]->insufficient_pbmp = data;
                            data = POE_REG_ADDRESS_MASK;
                            ret = hal_poe_writeReg(0, i,
                                                   HAL_POE_PAGE_1,
                                                   POE_AN8503_TOTAL_POWER_LIMITER_EVENT,
                                                   HAL_POE_REG_LEN_1,
                                                   &data);
                        }
                    }
                    else if ((POE_DEVICE_ID_IP804AR == _ptr_device_info[i]->revision_id) ||
                             (POE_DEVICE_ID_AN8502 == _ptr_device_info[i]->revision_id))
                    {
                        ret = hal_poe_readReg(0, i,
                                              HAL_POE_PAGE_1,
                                              POE_AN8502_IP804AR_TOTAL_POWER_LIMITER_EVENT,
                                              HAL_POE_REG_LEN_1,
                                              &data);
                        if(AIR_E_OK == ret)
                        {
                            _ptr_device_info[i]->insufficient_pbmp = data;
                            data = POE_REG_ADDRESS_MASK;
                            ret = hal_poe_writeReg(0, i,
                                                   HAL_POE_PAGE_1,
                                                   POE_AN8502_IP804AR_TOTAL_POWER_LIMITER_EVENT,
                                                   HAL_POE_REG_LEN_1,
                                                   &data);
                        }
                    }
                }
            }
        }

        if (MW_E_OK == ret)
        {
            for (i = 0; i < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; i++)
            {
                for (j = 0; j < _poe_port_cnt[i]; j++)
                {
                    unit = _ptr_port_info[i][j].unit;
                    port = _ptr_port_info[i][j].port;
                    poe_port = _ptr_port_info[i][j].poe_port;
                    poe_device_idx = _ptr_port_info[i][j].poe_device_idx;

                    ret = poe_getPortStatus(unit, port, alt, &status);
                    if (MW_E_OK == ret)
                    {
                        ret = poe_clearPortEvent(unit, port);
                    }
                    if (MW_E_OK == ret)
                    {
                        request.t_idx = POE_PORT_STATUS;
                        request.f_idx = PORT_POWER_STATUS;
                        request.e_idx = port;
                        data = status.power_status;
                        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            POE_LOG_ERROR("Fail to append PORT_POWER_STATUS msg payload. ret=%d", ret);
                            MW_FREE(ptr_multi_msg);
                        }
                    }
                    if (MW_E_OK == ret)
                    {
                        data = status.pd_class;
                        request.t_idx = POE_PORT_STATUS;
                        request.f_idx = PORT_PD_CLASS;
                        request.e_idx = port;
                        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            POE_LOG_ERROR("Fail to append PORT_PD_CLASS msg payload. ret=%d", ret);
                            MW_FREE(ptr_multi_msg);
                        }
                    }
                    if (MW_E_OK == ret)
                    {
                        if (POE_PORT_POWER_STATUS_ON == status.power_status)
                        {
                            data = POE_NO_POWER_EVENT;
                            request.t_idx = POE_PORT_STATUS;
                            request.f_idx = PORT_POWER_EVENT;
                            request.e_idx = port;
                            ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                            if (MW_E_OK != ret)
                            {
                                POE_LOG_ERROR("Fail to append PORT_POWER_EVENT msg payload. ret=%d", ret);
                                MW_FREE(ptr_multi_msg);
                            }
                        }
                        else
                        {
                            data = POE_NO_POWER_EVENT;
                            for (k = 0; k < POE_PWR_EVT_BIT_LEN; k++)
                            {
                                if (0 != (status.event_flags & BIT(k)))
                                {
                                    data = POE_NO_POWER_EVENT + k + 1;
                                }
                            }
                            if ((POE_PWR_CONTROL_HW == ptr_poe_control_block->power_strategy_control) &&
                                (0 != (_ptr_device_info[poe_device_idx]->insufficient_pbmp & BIT(poe_port))))
                            {
                                data = POE_INSUFFICIENT_POWER;
                            }
                            if (POE_NO_POWER_EVENT != data)
                            {
                                request.t_idx = POE_PORT_STATUS;
                                request.f_idx = PORT_POWER_EVENT;
                                request.e_idx = port;
                                ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                                if (MW_E_OK != ret)
                                {
                                    POE_LOG_ERROR("Fail to append PORT_POWER_EVENT msg payload. ret=%d", ret);
                                    MW_FREE(ptr_multi_msg);
                                }
                            }
                        }

                        ret = poe_getPortMeasurement(unit, port, alt, &meas);
                    }
                    if (MW_E_OK == ret)
                    {
                        request.t_idx = POE_PORT_STATUS;
                        request.f_idx = PORT_CURRENT;
                        request.e_idx = port;
                        data = meas.current;
                        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            POE_LOG_ERROR("Fail to append PORT_CURRENT msg payload. ret=%d", ret);
                            MW_FREE(ptr_multi_msg);
                        }
                    }
                    if (MW_E_OK == ret)
                    {
                        request.t_idx = POE_PORT_STATUS;
                        request.f_idx = PORT_VOLTAGE;
                        request.e_idx = port;
                        data = meas.voltage;
                        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            POE_LOG_ERROR("Fail to append PORT_VOLTAGE msg payload. ret=%d", ret);
                            MW_FREE(ptr_multi_msg);
                        }
                    }
                    if (MW_E_OK == ret)
                    {
                        request.t_idx = POE_PORT_STATUS;
                        request.f_idx = PORT_CONSUMED_POWER;
                        request.e_idx = port;
                        data = meas.power;
                        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
                        if (MW_E_OK != ret)
                        {
                            POE_LOG_ERROR("Fail to append PORT_CONSUMED_POWER msg payload. ret=%d", ret);
                            MW_FREE(ptr_multi_msg);
                        }
                    }
                    if (MW_E_OK != ret)
                    {
                        break;
                    }
                }
                if (MW_E_OK != ret)
                {
                    break;
                }
            }
        }

        if (MW_E_OK == ret)
        {
            if (ptr_multi_msg->type.count > 0)
            {
                ret = dbapi_sendMsg(ptr_multi_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
                if (MW_E_OK != ret)
                {
                    POE_LOG_ERROR("PoE send update DB data fail\n");
                }
            }
            else
            {
                MW_FREE(ptr_multi_msg);
            }
        }
    }

}

/* FUNCTION NAME: poe_chip_init
 * PURPOSE:
 *      Initialization PoE chip
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
poe_chip_init(
    void)
{
    UI8_T     ret = AIR_E_OK;
    UI8_T     i = 0;
    UI32_T    data = 0;

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        if (NULL != _ptr_device_info[i])
        {
            ret = hal_poe_readReg(0, i,
                                  HAL_POE_PAGE_0,
                                  POE_808AR_V24_CHECK_VERSION,
                                  HAL_POE_REG_LEN_1,
                                  &data);
            /*--------------------------------------------------------------
             * After confirming that the PSE is either 808ARv24 or 804BR,
             * enable the DISABLE IVT AUTO CLEAR bit to prevent
             * the port current limit event from being cleared
             *--------------------------------------------------------------*/
            if ((AIR_E_OK == ret) &&
                (((POE_DEVICE_ID_IP808AR == _ptr_device_info[i]->revision_id) && (POE_CHECK_VERSION_MASK && data)) ||
                    (POE_DEVICE_ID_AN8503 == _ptr_device_info[i]->revision_id)))
            {
                ret = hal_poe_readReg(0, i,
                                      HAL_POE_PAGE_1,
                                      POE_808AR_V24_808BR_DISABLE_IVT_AUTO_CLEAR,
                                      HAL_POE_REG_LEN_1,
                                      &data);
                if (AIR_E_OK == ret)
                {
                    data |= (POE_DISABLE_IVT_AUTO_CLEAR << POE_DISABLE_IVT_AUTO_CLEAR_OFFSET);
                    data &= 0xFF;

                    ret = hal_poe_writeReg(0, i,
                                           HAL_POE_PAGE_1,
                                           POE_808AR_V24_808BR_DISABLE_IVT_AUTO_CLEAR,
                                           HAL_POE_REG_LEN_1,
                                           &data);
                }
            }

            if (AIR_E_OK == ret)
            {
                ret = hal_poe_readReg(0, i,
                                        HAL_POE_PAGE_2,
                                        POE_I2C_BUS_TIMEOUT_CONTROL,
                                        HAL_POE_REG_LEN_1,
                                        &data);
                if (AIR_E_OK == ret)
                {
                    data &= ~(POE_TIMEOUT_SELETCION_MASK);
                    data |= (POE_TIMEOUT_SELECTION_DISABLE << POE_TIMEOUT_SELETCION_OFFSET);
                    data &= 0xFF;

                    ret = hal_poe_writeReg(0, i,
                                            HAL_POE_PAGE_2,
                                            POE_I2C_BUS_TIMEOUT_CONTROL,
                                            HAL_POE_REG_LEN_1,
                                            &data);
                }
            }

        }
    }
}

/* FUNCTION NAME: poe_db_init
 * PURPOSE:
 *      Initialization resource and update db for PoE.
 *
 * INPUT:
 *      ptr_poe_control_block -- pointer to the control block
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_db_init(
    POE_CB_T *ptr_poe_control_block)
{
    UI8_T                      ret = 0, i = 0, j = 0, port = 0;
    UI32_T                     count = 0, idx = 0;
    UI8_T                     *ptr_payload = NULL;
    UI16_T                     msg_size = 0;
    UI32_T                     data = 0, is_poe_port = 0, payload_size = 0;
    UI16_T                     revision_id = 0;
    DB_REQUEST_TYPE_T          request = {0};
    DB_MSG_T                  *ptr_multi_msg = NULL;
    AIR_INIT_POE_PORT_MAP_T   *ptr_port_map = NULL;
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map = NULL;
    POE_PWR_CONTROL_T          strategy = POE_PWR_CONTROL_LAST;

    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    ptr_multi_msg = dbapi_createMsg(NULL, M_UPDATE, 0, payload_size, &msg_size, &ptr_payload);

    ret = poe_getSystemPowerStrategy(&strategy);
    if (AIR_E_OK == ret)
    {
        if (POE_PWR_CONTROL_HW == strategy)
        {
            data = 0;
        }
        else
        {
            data = 1;
        }
        request.t_idx = POE_STATUS;
        request.f_idx = POE_POWER_STRATEGY;
        request.e_idx = DB_ALL_ENTRIES;
        ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
        if (MW_E_OK != ret)
        {
            POE_LOG_ERROR("update POE_POWER_STRATEGY failed(%d)", ret);
        }
    }
    else
    {
        POE_LOG_ERROR("get power strategy error(%d)", ret);
    }
    for (port = 0; port < AIR_MAX_PORT_NUM; port++)
    {
        if (NULL == PORT_INFO(ptr_poe_control_block, 0, port))
        {
            continue;
        }
        is_poe_port |= BIT(port);
    }
    data = is_poe_port >> 1;
    request.t_idx = POE_STATUS;
    request.f_idx = IS_POE_PORT;
    request.e_idx = DB_ALL_ENTRIES;
    ret = _poe_db_appendMsgPayload(&request, (UI8_T *)&data, &ptr_multi_msg, &msg_size, &ptr_payload);
    if (MW_E_OK != ret)
    {
        POE_LOG_ERROR("Fail to append IS_POE_PORT msg payload. ret=%d", ret);
        MW_FREE(ptr_multi_msg);
    }
    if (MW_E_OK == ret)
    {
        if (ptr_multi_msg->type.count > 0)
        {
            ret = dbapi_sendMsg(ptr_multi_msg, MSG_TIMEOUT_WAIT_INDEFINITELY);
            if (MW_E_OK != ret)
            {
                POE_LOG_ERROR("PoE send update DB data fail\n");
            }
        }
        else
        {
            MW_FREE(ptr_multi_msg);
        }
    }

    ret = osapi_calloc(sizeof(AIR_INIT_POE_PORT_MAP_T) * AIR_MAX_PORT_NUM, POE_APP_TASK_NAME, (void **)&ptr_port_map);
    if (MW_E_OK != ret)
    {
        POE_LOG_ERROR("allocate memory to port map faild(%d)", ret);
        ret = MW_E_NO_MEMORY;
    }

    if (MW_E_OK == ret)
    {
        /* Get PoE port mapping */
        for (i = 0; i < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; i++)
        {
            ret = air_init_getPoePortMap(i, &count, ptr_port_map);
            if ((AIR_E_OK == ret) && (0 != count))
            {
                _poe_port_cnt[i] = count;
                ret = osapi_calloc((sizeof(POE_PORT_INFO_T) * count), POE_APP_TASK_NAME, (void **)&_ptr_port_info[i]);
                if (MW_E_OK != ret)
                {
                    POE_LOG_ERROR("allocate memory to port info faild(%d)", ret);
                    ret = AIR_E_NO_MEMORY;
                }
                else
                {
                    for (j = 0; j < count; j++)
                    {
                        _ptr_port_info[i][j].unit = ptr_port_map[j].unit;
                        _ptr_port_info[i][j].port = ptr_port_map[j].port;
                        _ptr_port_info[i][j].poe_port = ptr_port_map[j].primary_port.hw_package_id;
                        _ptr_port_info[i][j].poe_device_idx = ptr_port_map[j].primary_port.poe_device_idx;
                    }
                }
            }
            if (AIR_E_NO_MEMORY == ret)
            {
                poe_db_deinit();
                break;
            }
        }
    }
    if (NULL != ptr_port_map)
    {
        MW_FREE(ptr_port_map);
    }

    /* allocate resource */
    osapi_calloc(sizeof(AIR_INIT_POE_DEVICE_MAP_T) * AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP,
                air_module_getModuleName(AIR_MODULE_POE),
                (void **)&ptr_device_map);
    if (NULL == ptr_device_map)
    {
        POE_LOG_ERROR("allocate memory failed\n");
        return MW_E_NO_MEMORY;
    }

    /* get device map info */
    ret = air_init_getPoeDeviceMap(0, &count, ptr_device_map);
    if (AIR_E_OK != ret)
    {
        POE_LOG_ERROR("get poe device map failed\n");
    }
    else
    {
        for (idx = 0; idx < count; idx++)
        {
            ret = osapi_calloc((sizeof(POE_DEVICE_INFO_T)), POE_APP_TASK_NAME, (void **)&_ptr_device_info[ptr_device_map[idx].poe_device_idx]);
            if (MW_E_OK != ret)
            {
                POE_LOG_ERROR("allocate memory to device info faild(%d)", ret);
                ret = AIR_E_NO_MEMORY;
            }
            else
            {
                /* hardware revision id */
                ret = air_poe_getDeviceInfo(0, ptr_device_map[idx].poe_device_idx, &revision_id);
            }
            if (AIR_E_OK != ret)
            {
                POE_LOG_ERROR("get PoE HW revision id failed\n");
            }
            else
            {
                _ptr_device_info[ptr_device_map[idx].poe_device_idx]->revision_id = revision_id;
            }
        }
    }

    MW_FREE(ptr_device_map);

    return ret;
}

/* FUNCTION NAME: poe_db_deinit
 * PURPOSE:
 *      Deinitialization resource.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
poe_db_deinit(
    void)
{
    UI8_T i = 0;

    for (i = 0; i < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; i++)
    {
        MW_FREE(_ptr_port_info[i]);
    }

    for (i = 0; i < AIR_CFG_MAXIMUM_POE_DEVICES_PER_CHIP; i++)
    {
        MW_FREE(_ptr_device_info[i]);
    }

    return MW_E_OK;
}