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

/* FILE NAME:  lp_db.c
 * PURPOSE:
 *    This file contains the implementation of DB for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#ifdef LP_MW_SUPPORT
#include    "lp_db.h"
#include    "lp_log.h"
#include    "lp_utils.h"
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
#include    "lp_hw.h"
#endif
#include    "lp_queue.h"
#include    "lp_led.h"
#include    "lp_acl.h"
#include    "lp_timer.h"
#include    "lp_mode.h"

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
static MW_ERROR_NO_T
_lp_db_parseMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void *ptr_data);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_lp_db_parseMsg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void *ptr_data)
{
    UI8_T               entry_num = 0;
    UI16_T              u_portId = 0, i = 0, db_idx = 0;
    UI32_T              trunk_members = 0;
    UI8_T               linkup = FALSE;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if ((NULL == ptr_data) || (NULL == ptr_lp_config))
    {
        return MW_E_BAD_PARAMETER;
    }
    switch (method)
    {
        case M_GET:
        case M_UPDATE:
        if (LOOP_PREVEN_INFO == request.t_idx)
        {
            switch (request.f_idx)
            {
                case LOOP_PREVEN_ENABLE:
                if (TRUE == ((UI8_T *)ptr_data)[0])
                {
                    lp_updateAdminMode(MW_LP_MODE_PREVENTION);
                }
                else if (FALSE == ((UI8_T *)ptr_data)[0])
                {
                    lp_updateAdminMode(MW_LP_MODE_DISABLE);
                }
                else
                {
                    LP_LOG_DEBUG("Invalid parameter - %d", ((UI8_T *)ptr_data)[0]);
                }
                break;
                default:
                    LP_LOG_DEBUG("recv unknown field: [%d]", request.f_idx);
                break;
            }
        }
        if (TRUNK_PORT == request.t_idx)
        {
            if (TRUNK_MEMBERS == request.f_idx)
            {
                UI8_T *ptr_trunk;
                DB_TRUNK_PORT_T trunk_info;

                ptr_trunk = (UI8_T *)ptr_data;
                entry_num = (((sizeof(DB_TRUNK_PORT_T) * MAX_TRUNK_NUM) == data_size) ? MAX_TRUNK_NUM : 1);
                for (i = 0; i < entry_num; i++)
                {
                    db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (i + 1) : request.e_idx);
                    osapi_memcpy(&trunk_info, (void *)(ptr_trunk + (i * sizeof(DB_TRUNK_PORT_T))), sizeof(DB_TRUNK_PORT_T));
                    trunk_members = trunk_info.members.member_bmp;
                    ptr_lp_config->trunk_info.group[db_idx - 1] = trunk_members << 1;
                    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
                    {
                        if (PLAT_CPU_PORT == u_portId)
                        {
                            continue;
                        }
                        if ((0x01 << u_portId) & (ptr_lp_config->trunk_info.group[db_idx - 1]))
                        {
                            ptr_lp_config->ptr_to_loop_pbmp[u_portId - 1][0] = 0;
                            ptr_lp_config->ptr_to_blk_pbmp[u_portId - 1][0] = 0;
                        }
                    }

                    LP_LOG_DEBUG("update trunk (id:%d member:0x%x)", db_idx, ptr_lp_config->trunk_info.group[db_idx - 1]);
                }
                lp_updatePortLoopState();
            }
        }
        if (PORT_OPER_INFO == request.t_idx)
        {
            if((DB_ALL_ENTRIES == request.e_idx) || (MW_LP_MODE_DISABLE == ptr_lp_config->lp_admin))
            {
                return MW_E_OK;
            }
            if (PORT_OPER_STATUS == request.f_idx)
            {
                osapi_memcpy(&linkup, (UI8_T *)ptr_data, sizeof(UI8_T));
                LP_LOG_DEBUG("port[%d] PORT_OPER_STATUS to link %s", request.e_idx, linkup ? "up" : "down");
                if (linkup)
                {
                    for (i = 0; i < MAX_TRUNK_NUM; i++)
                    {
                        if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << request.e_idx)))
                        {
                            if (0 != (ptr_lp_config->trunk_info.group[i] & ptr_lp_config->blocked_pbmp[0]))
                            {
                                LP_LOG_DEBUG("port[%d](id:%d member:0x%x) link up, try to set it flow ctrl off", request.e_idx, i + 1, ptr_lp_config->trunk_info.group[i]);
                                lp_port_setFlowControl(TRUE, request.e_idx);
                                break;
                            }
                        }
                    }
                    lp_port_setFlowControl(TRUE, request.e_idx);
                    ptr_lp_config->ptr_fc_resume_timer[(request.e_idx - 1)] = LP_FLOW_CONTROL_RESUME_TIME;
                }
            }
        }
        break;
        case M_ACK:
            /*
                * In loop prevent subscribe, should not be fail return
                */
        LP_LOG_DEBUG("ack result code");
        break;
        default:
        LP_LOG_DEBUG("recv unknown method");
        break;
    }

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: lp_db_subscribeModule
 * PURPOSE:
 *      Subscribe to the database module
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
lp_db_subscribeModule(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    /* Check DB is ready or not */
    do
    {
        rc = dbapi_dbisReady();
    } while (MW_E_OK != rc);

    /* Subscribe loopPreventionEn, recoveryTime and trunk */
    lp_db_send(M_SUBSCRIBE, LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, 0, 0, 0);
    lp_db_send(M_SUBSCRIBE, TRUNK_PORT, TRUNK_MEMBERS, 0, 0, 0);
    lp_db_send(M_SUBSCRIBE, PORT_OPER_INFO, PORT_OPER_STATUS, 0, 0, 0);
    return MW_E_OK;
}

/* FUNCTION NAME: lp_db_handleMsg
 * PURPOSE:
 *      Handle the database message.
 *
 * INPUT:
 *      ptr_msg          --  pointer to the database message
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
lp_db_handleMsg(
    DB_MSG_T *ptr_msg)
{
    UI8_T index = 0;
    DB_REQUEST_TYPE_T request = { 0 };
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T xRet = MW_E_OK;

    if (NULL != ptr_msg)
    {
        LP_LOG_DEBUG("[%d]recv method - %02X", ptr_msg->type.count, ptr_msg->method);
        if (0 != (M_B_RESPONSE & ptr_msg->method))
        {
            LP_LOG_DEBUG("recv response message");
        }
        else
        {
            /* Process the notification message */
            do
            {
                xRet = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
                if (MW_E_OK == xRet)
                {
                    LP_LOG_DEBUG("index=%u, ptr_payload=%p, t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u",
                                                            index++,
                                                            ptr_data,
                                                            request.t_idx,
                                                            request.f_idx,
                                                            request.e_idx,
                                                            data_size);

                    xRet = _lp_db_parseMsg(ptr_msg->method, request, data_size, ptr_data);
                    if (MW_E_OK != xRet)
                    {
                        LP_LOG_DEBUG("handle_db_msg failed!(%d)", xRet);
                    }
                }
                /* Continue to parse the next request within the payload. */
            } while ((MW_E_OK == xRet) && (NULL != ptr_payload_data));
        }
    }
    else
    {
        LP_LOG_ERROR("msgRecv fail.[%d]", xRet);
    }

    return;
}

/* FUNCTION NAME: lp_db_queue_send
 * PURPOSE:
 *      Package the message and send it to DB. If ptr_queue_name is NULL, DB
 *      will not send any response and the message itself will be freed by DB.
 *
 * INPUT:
 *      ptr_queue_name   --  A pointer to the queue that DB will return a
 *                           response to. If it is NULL, no response is expected.
 *      method           --  the method bitmap
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      e_idx            --  the entry index in the table
 *      ptr_data         --  A pointer to the message data
 *      timeout          --  The maximun amout of time the thread will wait for
 *                           send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
lp_db_queue_send(
    const C8_T *ptr_queue_name,
    const UI8_T method,
    const UI8_T t_idx,
    const UI8_T f_idx,
    const UI16_T e_idx,
    const void *ptr_data,
    const UI32_T timeout)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T msg_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;
    DB_REQUEST_TYPE_T request = {
        .t_idx = t_idx,
        .f_idx = f_idx,
        .e_idx = e_idx };

    MW_PARAM_CHK((t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    ptr_msg = dbapi_createMsgByReq(ptr_queue_name, method, 1, &request, &msg_size, (UI8_T **) &ptr_payload);
    if (NULL == ptr_msg)
    {
        return MW_E_NO_MEMORY;
    }
    LP_LOG_DEBUG("cq_name=%s method=0x%X count=%u t_idx=%u f_idx=%u e_idx=%u",
                ptr_msg->cq_name,
                ptr_msg->method,
                ptr_msg->type.count,
                t_idx,
                f_idx,
                e_idx);

    ret = dbapi_appendMsgPayload(&request, (UI8_T *) ptr_data, &ptr_msg, &msg_size, (UI8_T **) &ptr_payload);
    if (MW_E_OK != ret)
    {
        MW_FREE(ptr_msg);
        return ret;
    }

    /* Send the message to DB */
    ret = dbapi_sendMsg(ptr_msg, timeout);
    if (MW_E_OK != ret)
    {
        /* There is no need to free the message because dbapi_sendMsg() will free
         * the message if it fails.
         */
        LP_LOG_ERROR("Fail to send the msg to DB(%d)", ret);
    }

    return ret;
}

/* FUNCTION NAME: lp_db_send
 * PURPOSE:
 *      Send a message to DB
 *
 * INPUT:
 *      method           --  the method bitmap
 *      t_idx            --  the enum of the table
 *      f_idx            --  the enum of the field
 *      port             --  the port number
 *      size             --  the size of the data
 *      ptr_data         --  A pointer to the message data
 *      timeout          --  The maximun amout of time the thread will wait for
 *                           send (unit: millisecond)
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OP_INCOMPLETE
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      ptr_data should be freed if it is allocated from the heap and it is not
 *      used any more after the function is returned.
 */
MW_ERROR_NO_T
lp_db_send(
    UI8_T method,
    UI32_T table,
    UI32_T field,
    UI16_T port,
    UI16_T size,
    LP_STATE_T state)
{
    C8_T *ptr_queue_name = LP_QUEUE_NAME;

    UNUSED(size);
    if (M_UPDATE == method)
    {
        ptr_queue_name = NULL;
    }
    return lp_db_queue_send(ptr_queue_name, method, table, field, port, &state, LP_DELAY_INFINITY);
}
#endif /* LP_MW_SUPPORT */