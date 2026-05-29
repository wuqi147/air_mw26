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

/* FILE NAME:   stp_db.c
 * PURPOSE:
 *      stp db msg handle.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <sys_mgmt.h>
#include "mw_msg.h"
#include "mw_utils.h"
#include "air_port.h"
#include "stp.h"
#include "syncd_api_lag.h"


#ifdef AIR_SUPPORT_RSTP
#include "rstp-sys.h"
#endif
#ifdef AIR_SUPPORT_MSTP
#include "mstp_sys.h"
#endif
#include "stp_db.h"
#include "vlan_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
#ifdef AIR_SUPPORT_RSTP_SECURITY
RSTP_SCURITY_STATE_T rstp_port_sec_state = {0, 0, 0};
#endif

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static STP_TRUNK_INFO_T         _stp_db_trunk_group[MAX_TRUNK_NUM];
static UI8_T                    _stp_version = STP_VERSION_DEFAULT;
static UI8_T                    _stp_enable = FALSE;
#ifdef AIR_SUPPORT_RSTP
static UI32_T                   _rstp_block_bmp = 0;
#endif
#ifdef AIR_SUPPORT_STP_AUTO_COST
static UI8_T                    _stp_auto_cost_support = FALSE;
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

static inline void
_stp_checkDbReady(
    void)
{
    /* Check DB is ready */
    while (MW_E_OK != dbapi_dbisReady())
    {
        STP_LOG_INFO("DB is not ready");
        osapi_delay(STP_READY_DELAY);
    }
}

static MW_ERROR_NO_T
_stp_db_queue_create(
    const C8_T  *ptr_name)
{
    return osapi_msgCreate(ptr_name, STP_QUEUE_DYN_LENGTH, sizeof(void *));
}

static void
_stp_db_queue_delete(
    const C8_T  *ptr_name)
{
    osapi_msgDelete(ptr_name);
}

static MW_ERROR_NO_T
_stp_db_send_msg(
    UI32_T   size,
    DB_MSG_T *ptr_msg)
{
    MW_CHECK_PTR(ptr_msg);

    if (dbapi_dbisReady() != MW_E_OK)
    {
        /* This message could not be send, drop it */
        STP_LOG_INFO("DB is not ready");
        return MW_E_NOT_INITED;
    }

    if (dbapi_sendRequesttoDb(size, ptr_msg) != MW_E_OK)
    {
        /* This message could not be send, drop it */
        STP_LOG_ERR("fail to send DB");
        return MW_E_OTHERS;
    }

    return MW_E_OK;
}

MW_ERROR_NO_T
_stp_db_sendMsg(
    const C8_T                   *ptr_module,
    const C8_T                   *ptr_target,
    const UI8_T                  method,
    const DB_REQUEST_TYPE_T      req_type,
    void  *const                 ptr_data,
    const UI16_T                 size,
    DB_MSG_T                     **pptr_msg)
{
    MW_ERROR_NO_T   rc           = MW_E_OK;
    DB_MSG_T        *ptr_msg     = NULL;
    DB_PAYLOAD_T    *ptr_payload = NULL;
    UI32_T          msg_size     = 0;

    MW_PARAM_CHK((req_type.t_idx >= TABLES_LAST), MW_E_BAD_PARAMETER);

    msg_size = DB_MSG_HEADER_SIZE + DB_MSG_PAYLOAD_SIZE + size;
    rc = osapi_calloc(msg_size, ptr_module, (void **)&ptr_msg);
    if (MW_E_OK == rc)
    {
        /* message */
        osapi_strcpy(ptr_msg->cq_name, ptr_target);  /* max length of name: 3 */
        ptr_msg->method = method;
        ptr_msg->type.count = 1;
        /* payload */
        ptr_payload = stp_dbmsg_payload(ptr_msg);
        ptr_payload->request = req_type;
        ptr_payload->data_size = size;

        if (size > 0 && method != M_GET && method != M_GETFLASH)
        {
            osapi_memcpy(stp_dbpayload_data(ptr_payload), ptr_data, size);
        }
        rc = _stp_db_send_msg(msg_size, ptr_msg);
        if (rc == MW_E_OK)
        {
            if (pptr_msg)
            {
                *pptr_msg = ptr_msg;
            }
        }
        else
        {
            MW_FREE(ptr_msg);
        }
    }
    else
    {
        STP_LOG_ERR("alloc memory for db msg data failed!");
        return MW_E_NO_MEMORY;
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_db_recvMsg(
    DB_MSG_T    **pptr_msg)
{
    MW_ERROR_NO_T   rc       = MW_E_OK;
    DB_MSG_T        *ptr_msg = NULL;

    MW_CHECK_PTR(pptr_msg);
    rc = osapi_msgRecv(STP_DB_QUEUE_NAME_CMD, (UI8_T **)&ptr_msg, DB_MSG_PTR_SIZE, STP_DB_DELAY);
    if (rc != MW_E_OK)
    {
        return rc;
    }
    *pptr_msg = ptr_msg;

    return MW_E_OK;
}

static MW_ERROR_NO_T
_stp_db_getData(
    const UI8_T             method,
    const DB_REQUEST_TYPE_T req_type,
    DB_MSG_T                **pptr_msg,
    UI16_T                  *ptr_size,
    void                    **pptr_data)
{
    MW_ERROR_NO_T   rc           = MW_E_OK;
    DB_MSG_T        *ptr_msg     = NULL;

    rc = dbapi_getDataSize(req_type, ptr_size);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("get db table[%u/%u/%u] size failed(%d)",
            req_type.t_idx, req_type.f_idx, req_type.e_idx, rc);
        return rc;
    }
    else
    {
        STP_LOG_DBG("db table[%u/%u/%u] size: %u",
            req_type.t_idx, req_type.f_idx, req_type.e_idx, *ptr_size);
    }

    rc = _stp_db_sendMsg(STP_MODULE_NAME,
                             STP_DB_QUEUE_NAME_CMD,
                             method,
                             req_type,
                             NULL,
                             *ptr_size,
                             &ptr_msg);
    if (rc != MW_E_OK)
    {
        STP_LOG_ERR("send table[%u/%u/%u] msg failed(%d)",
            req_type.t_idx, req_type.f_idx, req_type.e_idx, rc);
        return rc;
    }

    STP_LOG_DBG("waitting for return msg");

    /* wait for DB response messgae */
    rc = _stp_db_recvMsg(&ptr_msg);
    if (rc != MW_E_OK)
    {
        STP_LOG_ERR("recv table[%u/%u/%u] msg failed(%d)",
            req_type.t_idx, req_type.f_idx, req_type.e_idx, rc);
        MW_FREE(ptr_msg);
        return rc;
    }

    rc = ptr_msg->type.result;
    if (rc == MW_E_OK)
    {
        *pptr_msg      = ptr_msg;
        *pptr_data     = stp_dbpayload_data(stp_dbmsg_payload(ptr_msg));
    }
    else
    {
        MW_FREE(ptr_msg);
    }
    STP_LOG_DBG("get table[%u/%u/%u] msg over.",
        req_type.t_idx, req_type.f_idx, req_type.e_idx);

    return rc;
}

static MW_ERROR_NO_T
_stp_db_updateData( /*use M_UPDATE */
    const C8_T              *ptr_name,
    const DB_REQUEST_TYPE_T req_type,
    void *const             ptr_data,
    const UI16_T            data_size)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    rc = _stp_db_sendMsg(STP_MODULE_NAME, ptr_name, M_UPDATE, req_type, ptr_data, data_size, NULL);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("send table[%u/%u/%u]to DB failed!", req_type.t_idx, req_type.f_idx, req_type.e_idx);
    }

    return rc;
}

static void
_stp_db_getTrunkID(
    UI32_T  port,
    UI32_T  *ptr_trunk_id)
{
    UI32_T  i=0;
    UI32_T  port_id = (port - 1);
    UI32_T  member = 0;

    *ptr_trunk_id = MAX_TRUNK_NUM;
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        member = _stp_db_trunk_group[i].memberBmp;
        if (0 != (member & (1 << port_id)))
        {
            *ptr_trunk_id = i;
            break;
        }
    }
}

static BOOL_T
_stp_db_checkIsLowerPort(
    UI32_T  port,
    UI32_T  trunk_id)
{
    BOOL_T  is_lower_port = FALSE;
    UI32_T  p = 0;
    UI32_T  port_id = (port - 1);
    UI32_T  member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
    {
        if (((member & (1 << p)) > 0) && (p == port_id))
        {
            is_lower_port = TRUE;
            break;
        }
        else if (((member & (1 << p)) > 0) && (p != port_id))
        {
            break;
        }
    }

    return is_lower_port;
}

static UI32_T
_stp_db_trunk_getLowerPort(
    UI32_T  trunk_id)
{
    UI32_T  p = 0;
    UI32_T  rtn_port = 0;
    UI32_T  member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
    {
        if ((member & BIT(p)) > 0)
        {
            rtn_port = (p + 1);
            break;
        }
    }

    return rtn_port;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_stp_speed_enum_to_integer(
    const AIR_PORT_SPEED_T  speed_enum,
    UI32_T                  *ptr_speed_int)
{
    switch (speed_enum)
    {
        case AIR_PORT_SPEED_10M:
            *ptr_speed_int = 10;
            break;
        case AIR_PORT_SPEED_100M:
            *ptr_speed_int = 100;
            break;
        case AIR_PORT_SPEED_1000M:
            *ptr_speed_int = 1000;
            break;
        case AIR_PORT_SPEED_2500M:
            *ptr_speed_int = 2500;
            break;
        case AIR_PORT_SPEED_5000M:
            *ptr_speed_int = 5000;
            break;
        default:
            *ptr_speed_int = 0;
            break;
    }
    return MW_E_OK;
}

#endif

static MW_ERROR_NO_T
_stp_db_get_port_oper_status(
    UI32_T  port,
    UI8_T   *ptr_status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI16_T              data_size = 0;
    DB_REQUEST_TYPE_T   req_type;
    DB_MSG_T            *ptr_msg  = NULL;
    UI8_T               *ptr_data = NULL;

    rc = _stp_db_queue_create(STP_DB_QUEUE_NAME_CMD);
    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_STATUS, port);
        rc = _stp_db_getData(M_GET, req_type, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK == rc)
        {
            osapi_memcpy(ptr_status, ptr_data, sizeof(UI8_T));
            MW_FREE(ptr_msg);
            _stp_db_queue_delete(STP_DB_QUEUE_NAME_CMD);
        }
        else
        {
            STP_LOG_ERR("Get port %u status failed!", port);
        }
    }
    else
    {
        STP_LOG_ERR("Create queue(%s) for getting port %u status failed!",
            STP_DB_QUEUE_NAME_CMD, port);
    }
    return rc;
}

static UI8_T
_stp_trunk_check_all_member_linkdown_excl_port(
    UI32_T  trunk_id,
    UI32_T  port)
{
    UI32_T          p = 0;
    UI8_T           link = 0;
    UI8_T           all_linkdown = TRUE;
    UI32_T          port_id = port - 1;
    UI32_T          member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
    {
        if ((p != port_id) && (member & BIT(p)) > 0)
        {
            _stp_db_get_port_oper_status((p+1), &link);
            if (TRUE == link)
            {
                all_linkdown = FALSE;
                break;
            }
        }
    }

    return all_linkdown;
}

static void
_stp_enable_set(
    UI8_T   enable)
{
    UI8_T   curState = _stp_enable;

    if (curState != enable)
    {
        _stp_enable = enable;
        stp_bpdu_rate_limit_ctrl(enable);
        if (STP_VERSION_MSTP == _stp_version)
        {
#ifdef AIR_SUPPORT_MSTP
            mstp_enable_set(enable);
#endif
        }
        else
        {
#ifdef AIR_SUPPORT_RSTP
            rstp_enable_set(enable);
#endif
        }
    }
}

static void
_stp_force_version_set(
    UI32_T   force_version)
{
    UI8_T  version = (UI8_T)force_version;

    if (TRUE != _stp_enable)
    {
        _stp_version = version;
        return;
    }

    STP_LOG_DBG("Set force version to %d", version);
    if (version == _stp_version)
    {
        return;
    }

#if defined(AIR_SUPPORT_RSTP) && defined(AIR_SUPPORT_MSTP)
    if (STP_VERSION_MSTP == version)
    {
        rstp_enable_set(FALSE);
        mstp_enable_set(TRUE);
    }
    else
    {
        if (STP_VERSION_MSTP == _stp_version)
        {
            mstp_enable_set(FALSE);
            rstp_enable_set(TRUE);
        }
        rstp_force_version_set(version);
    }
#endif

    if (STP_VERSION_MSTP == version)
    {
#ifdef AIR_SUPPORT_MSTP
        mstp_force_version_set(version);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rstp_force_version_set(version);
#endif
    }
    _stp_version = version;
}

static MW_ERROR_NO_T
_stp_forward_delay_set(
    UI16_T  forward_delay)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    STP_LOG_DBG("Set forward delay to %d", forward_delay);
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_forward_delay_set(forward_delay);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_forward_delay_set(forward_delay);
#endif
    }

    return rc;
}


static MW_ERROR_NO_T
_stp_max_age_set(
    UI16_T   max_age)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    STP_LOG_DBG("Set max age to %d", max_age);
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_max_age_set(max_age);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_max_age_set(max_age);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_transmit_hold_count_set(
    UI16_T  tx_hold_cnt)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    STP_LOG_DBG("Set transmit hold count to %d", tx_hold_cnt);
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_transmit_hold_count_set(tx_hold_cnt);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_transmit_hold_count_set(tx_hold_cnt);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_priority_set(
    UI16_T  priority)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    STP_LOG_DBG("Set bridge priority to %d", priority);
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_priority_set(priority);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_priority_set(priority);
#endif
    }

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
static MW_ERROR_NO_T
_stp_auto_cost_support_set(
    UI8_T   enable)
{
    MW_ERROR_NO_T rc;

    STP_LOG_DBG("Set auto cost state to %d", enable);
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_auto_cost_support_set(enable);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_auto_cost_support_set(enable);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_port_auto_cost_set(
    UI32_T  port)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    STP_LOG_DBG("Set port %d auto cost enable", port);
#ifdef AIR_SUPPORT_STP_AUTO_COST
    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_port_auto_cost_set(port);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_auto_cost_set(port);
#endif
    }
#endif

    return rc;
}
#endif

#ifdef AIR_SUPPORT_MSTP
static MW_ERROR_NO_T
_mstp_db_get_port_cost_config(
    UI32_T  port,
    UI32_T   *ptr_cost)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_COST, port_number);
    rc = stp_db_get_msg(req_type, ptr_cost);

    return rc;
}
#endif

static MW_ERROR_NO_T
_stp_port_priority_set(
    UI32_T  port,
    UI8_T   priority)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    STP_LOG_DBG("Set port %d priority to %d", port, priority);

    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_port_instance_priority_set(port, 0, priority);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_priority_set(port, priority);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_port_cost_set(
    UI32_T  port,
    UI32_T  cost)
{
    MW_ERROR_NO_T rc = MW_E_OK;
#ifdef AIR_SUPPORT_MSTP
    UI32_T        costArray[MAX_MSTP_INSTANCE_NUM];
    UI32_T        i, trunk_id;
    UI32_T        lowerPort = port;
#endif

    STP_LOG_DBG("Set port %d cost to %d", port, cost);

    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        stp_db_getTrunkID(port, &trunk_id);
        if (MAX_TRUNK_NUM > trunk_id)
        {
            port = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
            lowerPort = stp_db_trunk_getLowerPort(trunk_id);
        }
        _mstp_db_get_port_cost_config(lowerPort, costArray);

        for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
        {
            mstp_setPortPathCost(port, i, costArray[i]);
        }
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_cost_set(port, cost);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_port_admin_edge_set(
    UI32_T  port,
    UI8_T   admin_edge)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    STP_LOG_DBG("Set port %d admin edge state to %d", port, admin_edge);

    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_port_admin_edge_set(port, admin_edge);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_admin_edge_set(port, admin_edge);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_port_enable_status_set(
    UI32_T  port,
    UI8_T   status)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    STP_LOG_DBG("Set port %d status to %d", port, status);

    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_port_enable_status_set(port, status);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_enable_status_set(port, status);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_port_mcheck_set(
    UI32_T  port,
    UI8_T   perform)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    STP_LOG_DBG("Set port %d mcheck state to %d", port, perform);

    if (STP_VERSION_MSTP == _stp_version)
    {
#ifdef AIR_SUPPORT_MSTP
        rc = mstp_port_mcheck_set(port, perform);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rc = rstp_port_mcheck_set(port, perform);
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_db_trunk_port_mcheck_update(
    const UI32_T    trunk_id,
    const UI8_T     perform)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              p = 0;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
    {
        if ((member & BIT(p)) > 0)
        {
            req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_MCHECK, (p+1));
            rc = _stp_db_updateData(STP_DB_QUEUE_NAME_EMPTY, req_type, (void *)(&perform), sizeof(UI8_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send trunk member port %d mcheck to DB", p+1);
                break;
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_db_port_mcheck_update(
    const UI32_T    port,
    const UI8_T     perform)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (PLAT_MAX_PORT_NUM < port)
    {
        return _stp_db_trunk_port_mcheck_update(STP_PORT_NUMBER_TO_TRUNK_ID(port), perform);
    }

    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_MCHECK, port);
    rc = _stp_db_updateData(STP_DB_QUEUE_NAME_EMPTY, req_type, (void *)(&perform), sizeof(UI8_T));
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to send port %d mcheck to DB", port);
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_db_trunk_port_role_update(
    UI32_T      trunk_id,
    const UI8_T role)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              p = 0;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    if (MW_E_OK == rc)
    {
        for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
        {
            if ((member & BIT(p)) > 0)
            {
                req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_ROLE, (p+1));
                rc = stp_db_update_msg(req_type, (void *)(&role), sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    STP_LOG_ERR("Fail to send to DB for port %d role %d", p+1, role);
                    break;
                }
            }
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_db_lacp_handle(
    UI16_T e_idx,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI32_T              port_number = 0;
    UI32_T              trunk_id = 0;
    UI8_T               state;
    UI8_T               status;
    UI8_T               lacp_state[MAX_PORT_NUM];
    UI32_T              i;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    UI8_T               auto_enable;
    UI32_T              aggBmp[MAX_TRUNK_NUM];
#endif /* AIR_SUPPORT_STP_AUTO_COST */

    if (DB_ALL_ENTRIES != e_idx)
    {
        port_number = (UI32_T)e_idx;
        _stp_db_getTrunkID(port_number, &trunk_id);
        if (MAX_TRUNK_NUM != trunk_id)
        {
            if (LAG_MODE_STATIC == _stp_db_trunk_group[trunk_id].mode)
            {
                return MW_E_OK;
            }
            state = ((UI8_T*)ptr_data)[0];
            if (PORT_LACP_STATE_AGGREGATED == state)
            {
                BIT_SET(_stp_db_trunk_group[trunk_id].aggregatedBmp, (port_number-1));
            }
            else
            {
                BIT_DEL(_stp_db_trunk_group[trunk_id].aggregatedBmp, (port_number-1));
            }

            if (TRUE == _stp_enable)
            {
                status = (0 == _stp_db_trunk_group[trunk_id].aggregatedBmp) ? FALSE : TRUE;
                port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);

                if (STP_VERSION_MSTP == _stp_version)
                {
#ifdef AIR_SUPPORT_MSTP
                    mstp_port_mac_operational_set(port_number, status);
#endif
                }
                else
                {
#ifdef AIR_SUPPORT_RSTP
                    rstp_port_mac_operational_set(port_number, status);
#endif
                }

#ifdef AIR_SUPPORT_STP_AUTO_COST
                stp_db_get_port_auto_enable_config(port_number, &auto_enable);
                if ((TRUE == _stp_auto_cost_support) && (TRUE == auto_enable))
                {
                    _stp_port_auto_cost_set(port_number);
                }
#endif
            }
        }
    }
    else
    {
#ifdef AIR_SUPPORT_STP_AUTO_COST
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            aggBmp[i] = _stp_db_trunk_group[i].aggregatedBmp;
        }
#endif /* AIR_SUPPORT_STP_AUTO_COST */

        osapi_memcpy(lacp_state, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            port_number = i + 1;
            _stp_db_getTrunkID(port_number, &trunk_id);
            if (MAX_TRUNK_NUM != trunk_id)
            {
                if (LAG_MODE_STATIC == _stp_db_trunk_group[trunk_id].mode)
                {
                    continue;
                }
                state = lacp_state[i];
                if (PORT_LACP_STATE_AGGREGATED == state)
                {
                    BIT_SET(_stp_db_trunk_group[trunk_id].aggregatedBmp, (port_number-1));
                }
                else
                {
                    BIT_DEL(_stp_db_trunk_group[trunk_id].aggregatedBmp, (port_number-1));
                }

                if (TRUE == _stp_enable)
                {
                    status = (0 == _stp_db_trunk_group[trunk_id].aggregatedBmp) ? FALSE : TRUE;
                    port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);

                    if (STP_VERSION_MSTP == _stp_version)
                    {
#ifdef AIR_SUPPORT_MSTP
                        mstp_port_mac_operational_set(port_number, status);
#endif
                    }
                    else
                    {
#ifdef AIR_SUPPORT_RSTP
                        rstp_port_mac_operational_set(port_number, status);
#endif
                    }
                }
            }
        }
#ifdef AIR_SUPPORT_STP_AUTO_COST
        if (TRUE == _stp_enable)
        {
            for (i = 0; i < MAX_TRUNK_NUM; i++)
            {
                if (aggBmp[i] == _stp_db_trunk_group[i].aggregatedBmp)
                {
                    continue;
                }
                port_number = STP_TRUNK_ID_TO_PORT_NUMBER(i);
                stp_db_get_port_auto_enable_config(port_number, &auto_enable);
                if ((TRUE == _stp_auto_cost_support) && (TRUE == auto_enable))
                {
                    _stp_port_auto_cost_set(port_number);
                }
            }
        }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
    }

    return MW_E_OK;
}

#ifdef AIR_SUPPORT_RSTP
static MW_ERROR_NO_T
_stp_db_trunk_port_block(
    UI32_T      trunk_id,
    const UI8_T oper_state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              p = 0;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              member = 0;

    if (MW_E_OK == rc)
    {
        member = _stp_db_trunk_group[trunk_id].memberBmp;
        for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
        {
            if ((member & BIT(p)) != 0)
            {
                STP_LOG_DBG("update trunk %u member port %u", trunk_id, (p+1));
                req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_BLOCK, (p+1));
                rc = stp_db_update_msg(req_type, (void *)(&oper_state), sizeof(UI8_T));
                if (MW_E_OK != rc)
                {
                    STP_LOG_ERR("Fail to send to DB for port %d block state %d", p+1, oper_state);
                }
            }
        }
    }

    return rc;
}
#endif

static MW_ERROR_NO_T
_stp_db_msg_handle(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    UI8_T           perform = 0;

    if ((NULL == ptr_request) || (NULL == ptr_data) || (0 == data_size))
    {
        return MW_E_BAD_PARAMETER;
    }
    switch(method)
    {
        case M_CREATE:
        case M_GET:
        case M_UPDATE:
            switch(ptr_request->t_idx)
            {
                case STP_INFO:
                    switch(ptr_request->f_idx)
                    {
                        case STP_ENABLE:
                        {
                            if (stp_check_bool(*((UI8_T*)ptr_data)))
                            {
                                _stp_enable_set(*((UI8_T*)ptr_data));
                            }
                            else
                            {
                                STP_LOG_ERR("STP_ENABLE: Invalid parameter - %d",
                                    *((UI8_T*)ptr_data));
                            }
                            break; /* case STP_ENABLE */
                        }
                        case STP_FORCE_VERSION:
                        {
                            UI32_T  version = 0;

                            osapi_memcpy(&version, ptr_data, sizeof(UI32_T));
                            if ((STP_VERSION_COMPATIBILITY == version) || (STP_VERSION_DEFAULT == version)
                                || (STP_VERSION_MSTP == version))
                            {
                                _stp_force_version_set(version);
                            }
                            else
                            {
                                STP_LOG_ERR("STP_FORCE_VERSION: Invalid parameter - %d", version);
                            }
                            break; /* case STP_FORCE_VERSION */
                        }
                        case STP_FORWARD_DELAY:
                        {
                            UI16_T  fwd_delay = 0;

                            osapi_memcpy(&fwd_delay, ptr_data, sizeof(UI16_T));
                            rc = _stp_forward_delay_set(fwd_delay);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_FORWARD_DELAY: Invalid parameter - %d", fwd_delay);
                            }
                            break; /* case RSTP_FORWARD_DELAY */
                        }
                        case STP_MAX_AGE:
                        {
                            UI16_T  max_age = 0;

                            osapi_memcpy(&max_age, ptr_data, sizeof(UI16_T));
                            rc = _stp_max_age_set(max_age);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_MAX_AGE: Invalid parameter - %d", max_age);
                            }
                            break; /* case RSTP_MAX_AGE */
                        }
                        case STP_TRANSMIT_HOLD_COUNT:
                        {
                            UI16_T  tx_hold_cnt = 0;

                            osapi_memcpy(&tx_hold_cnt, ptr_data, sizeof(UI16_T));
                            rc =_stp_transmit_hold_count_set(tx_hold_cnt);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_TRANSMIT_HOLD_COUNT: Invalid parameter - %d",
                                    tx_hold_cnt);
                            }
                            break; /* case RSTP_TRANSMIT_HOLD_COUNT */
                        }
                        case STP_BRIDGE_PRIORITY:
                        {
                            UI16_T  pri = 0;

                            osapi_memcpy(&pri, ptr_data, sizeof(UI16_T));
                            rc = _stp_priority_set(pri);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_BRIDGE_PRIORITY: Invalid parameter - %d", pri);
                            }
                            break; /* case RSTP_BRIDGE_PRIORITY */
                        }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        case STP_AUTO_COST_SUPPORT:
                        {
                            if (stp_check_bool(*((UI8_T*)ptr_data)))
                            {
                                _stp_auto_cost_support_set(*((UI8_T*)ptr_data));
                            }
                            else
                            {
                                STP_LOG_ERR("STP_AUTO_COST_SUPPORT: Invalid parameter - %d",
                                    *((UI8_T*)ptr_data));
                            }
                            break; /* case RSTP_AUTO_COST_SUPPORT */
                        }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
#ifdef AIR_SUPPORT_RSTP_SECURITY
                        case RSTP_PORT_BPDU_GUARD_STATE:
                            if (rstp_port_sec_state.bg_state != *((UI32_T*)ptr_data))
                            {
                                rstp_port_sec_state.bg_state = *((UI32_T*)ptr_data);
                            }
                            break;
#endif
                        default:
                        {
                            STP_LOG_WARN("unknown field id(%d) of table id(%d)\n",
                                ptr_request->f_idx,
                                ptr_request->t_idx);
                            break;
                        }
                    }
                    break; /* case STP_INFO */
                case STP_PORT_INFO:
                {
                    if (DB_ALL_FIELDS == ptr_request->f_idx)
                    {
                        if (DB_ALL_ENTRIES != ptr_request->e_idx)
                        {
                            DB_STP_PORT_INFO_T p_info = {0};
#ifdef AIR_SUPPORT_STP_AUTO_COST
                            UI8_T               link = 0;
#endif /* AIR_SUPPORT_STP_AUTO_COST */

                            osapi_memcpy(&p_info, ptr_data, sizeof(DB_STP_PORT_INFO_T));
                            rc = _stp_port_priority_set(ptr_request->e_idx, p_info.priority);
                            if (MW_E_OK == rc)
                            {
#ifdef AIR_SUPPORT_STP_AUTO_COST
                                if (TRUE == _stp_auto_cost_support)
                                {
                                    if (TRUE == p_info.auto_cost_enable)
                                    {
                                        stp_db_get_port_status(ptr_request->e_idx, &link);
                                        if (TRUE == link)
                                        {
                                            _stp_port_auto_cost_set(ptr_request->e_idx);
                                        }
                                    }
                                    else
                                    {
                                        rc = _stp_port_cost_set(ptr_request->e_idx, p_info.cost);
                                        if (MW_E_OK != rc)
                                        {
                                            STP_LOG_ERR("STP_PORT_COST: Invalid parameter - %d", p_info.cost);
                                        }
                                    }
                                }
                                else
                                {
                                    rc = _stp_port_cost_set(ptr_request->e_idx, p_info.cost);
                                    if (MW_E_OK != rc)
                                    {
                                        STP_LOG_ERR("STP_PORT_COST: Invalid parameter - %d", p_info.cost);
                                    }
                                }
#else
                                rc = _stp_port_cost_set(ptr_request->e_idx, p_info.cost);
                                if (MW_E_OK != rc)
                                {
                                    STP_LOG_ERR("STP_PORT_COST: Invalid parameter - %d", p_info.cost);
                                }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                                if (MW_E_OK == rc)
                                {
                                    rc = _stp_port_admin_edge_set(ptr_request->e_idx, p_info.admin_edge);
                                    if (MW_E_OK != rc)
                                    {
                                        STP_LOG_ERR("STP_PORT_ADMIN_EDGE: Invalid parameter - %d", p_info.admin_edge);
                                    }
                                }
                                if (MW_E_OK == rc)
                                {
                                    rc = _stp_port_enable_status_set(ptr_request->e_idx, p_info.status);
                                    if (MW_E_OK != rc)
                                    {
                                        STP_LOG_ERR("STP_PORT_STATUS: Invalid parameter - %d", p_info.status);
                                    }
                                }
                                if (MW_E_OK == rc)
                                {
                                    STP_LOG_DBG("Db recv port:%u mcheck=%u", ptr_request->e_idx, p_info.mcheck);
                                    if (TRUE == p_info.mcheck)
                                    {
                                        rc = _stp_port_mcheck_set(ptr_request->e_idx, p_info.mcheck);
                                        if (MW_E_OK != rc)
                                        {
                                            STP_LOG_ERR("STP_PORT_MCHECK: Invalid patameter - %d", p_info.mcheck);
                                        }
                                        else
                                        {
                                            /* Clear mcheck field */
                                            _stp_db_port_mcheck_update(ptr_request->e_idx, perform);
                                        }
                                    }
                                }
                            }
                            else
                            {
                                STP_LOG_ERR("STP_PORT_PRIORITY: Invalid parameter - %d", p_info.priority);
                            }
                        }
                    }
                    else /* DB_ALL_FIELDS != f_idx*/
                    {
                        UI32_T  first_port=0, last_port=0, port=0, i=0;
                        UI32_T  cost_arr[PLAT_MAX_PORT_NUM];
                        UI8_T   priority_arr[PLAT_MAX_PORT_NUM];
                        UI8_T   admin_edge_arr[PLAT_MAX_PORT_NUM];
                        UI8_T   mcheck_arr[PLAT_MAX_PORT_NUM];
                        UI8_T   enable_status[PLAT_MAX_PORT_NUM];
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        UI8_T   auto_arr[PLAT_MAX_PORT_NUM];
                        UI8_T   auto_enable = 0;
                        UI8_T   link = 0;
                        UI32_T  lower_port = 0, trunk_id = 0;
#endif /* AIR_SUPPORT_STP_AUTO_COST */

                        osapi_memset(cost_arr, 0, sizeof(UI32_T) * PLAT_MAX_PORT_NUM);
                        osapi_memset(priority_arr, 0, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        osapi_memset(auto_arr, 0, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                        if (DB_ALL_ENTRIES == ptr_request->e_idx)
                        {
                            first_port = 1;
                            last_port = PLAT_MAX_PORT_NUM;

                            if (STP_PORT_PRIORITY == ptr_request->f_idx)
                            {
                                osapi_memcpy(priority_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
                            else if (STP_PORT_COST == ptr_request->f_idx)
                            {
                                osapi_memcpy(cost_arr, ptr_data, sizeof(UI32_T) * PLAT_MAX_PORT_NUM);
                            }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                            else if (STP_PORT_AUTO_COST_ENABLE == ptr_request->f_idx)
                            {
                                osapi_memcpy(auto_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                            else if (STP_PORT_ADMIN_EDGE == ptr_request->f_idx)
                            {
                                osapi_memcpy(admin_edge_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
                            else if (STP_PORT_MCHECK == ptr_request->f_idx)
                            {
                                osapi_memcpy(mcheck_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
                            else if (STP_PORT_STATUS == ptr_request->f_idx)
                            {
                                osapi_memcpy(enable_status, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
                        }
                        else /* DB_ALL_ENTRIES != ptr_request->e_idx */
                        {
                            first_port = last_port = ptr_request->e_idx;

                            if (STP_PORT_PRIORITY == ptr_request->f_idx)
                            {
                                osapi_memcpy(priority_arr, ptr_data, sizeof(UI8_T));
                            }
                            else if (STP_PORT_COST == ptr_request->f_idx)
                            {
                                osapi_memcpy(cost_arr, ptr_data, sizeof(UI32_T));
                            }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                            else if (STP_PORT_AUTO_COST_ENABLE == ptr_request->f_idx)
                            {
                                osapi_memcpy(auto_arr, ptr_data, sizeof(UI8_T));
                            }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                            else if (STP_PORT_ADMIN_EDGE == ptr_request->f_idx)
                            {
                                osapi_memcpy(admin_edge_arr, ptr_data, sizeof(UI8_T));
                            }
                            else if (STP_PORT_MCHECK == ptr_request->f_idx)
                            {
                                osapi_memcpy(mcheck_arr, ptr_data, sizeof(UI8_T));
                            }
                            else if (STP_PORT_STATUS == ptr_request->f_idx)
                            {
                                osapi_memcpy(enable_status, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            }
                        }

                        if (STP_PORT_PRIORITY == ptr_request->f_idx)
                        {
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                rc |= _stp_port_priority_set(port, priority_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_PORT_PRIORITY: Invalid parameter");
                            }
                        }
                        else if (STP_PORT_COST == ptr_request->f_idx)
                        {
                            for (port = first_port, i = 0; port <= last_port; port++, i++)
                            {
#ifdef AIR_SUPPORT_STP_AUTO_COST
                                if (TRUE == _stp_auto_cost_support)
                                {
                                    lower_port = port;
                                    _stp_db_getTrunkID(port, &trunk_id);
                                    if (MAX_TRUNK_NUM > trunk_id)
                                    {
                                        lower_port = _stp_db_trunk_getLowerPort(trunk_id);
                                    }
                                    stp_db_get_port_auto_enable_config(lower_port, &auto_enable);
                                    if (TRUE == auto_enable)
                                    {
                                        continue;
                                    }
                                }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                                rc |= _stp_port_cost_set(port, cost_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_PORT_COST: Invalid parameter");
                            }
                        }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        else if (STP_PORT_AUTO_COST_ENABLE == ptr_request->f_idx)
                        {
                            if (TRUE == _stp_auto_cost_support)
                            {
                                for (port = first_port, i = 0; port <= last_port ; port++, i++)
                                {
                                    stp_db_get_port_status(port, &link);
                                    if (TRUE == link)
                                    {
                                        if (FALSE == auto_arr[i])
                                        {
#ifdef AIR_SUPPORT_RSTP
                                            rstp_set_port_auto_cost_config(port);
#endif
#ifdef AIR_SUPPORT_MSTP
                                            mstp_set_port_auto_cost_config(port);
#endif
                                        }
                                        else
                                        {
#ifdef AIR_SUPPORT_RSTP
                                            rstp_port_auto_cost_set(port);
#endif
#ifdef AIR_SUPPORT_MSTP
                                            mstp_port_auto_cost_set(port);
#endif
                                        }
                                    }
                                }
                            }
                        }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                        else if (STP_PORT_ADMIN_EDGE == ptr_request->f_idx)
                        {
                            for (port = first_port, i = 0; port <= last_port; port++, i++)
                            {
                                rc |= _stp_port_admin_edge_set(port, admin_edge_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_PORT_ADMIN_EDGE: Invalid parameter");
                            }
                        }
                        else if (STP_PORT_STATUS == ptr_request->f_idx)
                        {
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                rc |= _stp_port_enable_status_set(port, enable_status[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_PORT_ENABLE_STATUS: Invalid parameter");
                            }
                        }
                        else if (STP_PORT_MCHECK == ptr_request->f_idx)
                        {
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                if (TRUE == mcheck_arr[i])
                                {
                                    rc |= _stp_port_mcheck_set(port, mcheck_arr[i]);
                                    if (MW_E_OK == rc)
                                    {
                                        /* Clear mcheck field */
                                        _stp_db_port_mcheck_update(port, perform);
                                    }
                                }
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("STP_PORT_MCHECK: Invalid parameter");
                            }
                        }
                        else
                        {
                            STP_LOG_WARN("unknown field id(%d) of table id(%d)\n",
                                    ptr_request->f_idx, ptr_request->t_idx);
                        }
                    }

                    break; /* case RSTP_PORT_INFO */
                }
                case TRUNK_PORT:
                {
                    if (TRUNK_MEMBERS == ptr_request->f_idx)
                    {
                        const UI16_T        e_idx = ptr_request->e_idx; /* trunk index */
                        DB_TRUNK_PORT_T     members[MAX_TRUNK_NUM];
                        UI8_T               i = 0;

                        osapi_memset(members, 0, sizeof(DB_TRUNK_PORT_T) * MAX_TRUNK_NUM);
                        if (DB_ALL_ENTRIES == e_idx)
                        {
                            osapi_memcpy(&members, ptr_data, sizeof(DB_TRUNK_PORT_T) * MAX_TRUNK_NUM);
                        }
                        else
                        {
                            osapi_memcpy(&members, ptr_data, sizeof(DB_TRUNK_PORT_T));
                        }

                        /* re-construct STP instances */
                        if (TRUE == _stp_enable)
                        {
#ifdef AIR_SUPPORT_RSTP_SECURITY
                            DB_REQUEST_TYPE_T   req;
                            rstp_port_sec_state.lg_state = 0;
                            req = stp_dbmsg_init_reqtype(STP_INFO, RSTP_PORT_LOOP_GUARD_STATE, DB_ALL_ENTRIES);
                            rc = stp_db_update_msg(req, (void *)(&rstp_port_sec_state.lg_state), sizeof(UI32_T));
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("Fail to update port loop protect state");
                            }
#endif
                            if (STP_VERSION_MSTP == _stp_version)
                            {
#ifdef AIR_SUPPORT_MSTP
                                mstp_trunk_update(e_idx, (void *)ptr_data);
#endif
                            }
                            else
                            {
#ifdef AIR_SUPPORT_RSTP
                                rstp_trunk_update(e_idx, (void *)ptr_data);
#endif
                            }
                        }
                        else
                        {
                            if (DB_ALL_ENTRIES == e_idx)
                            {
                                for (i = 0; i < MAX_TRUNK_NUM; i++)
                                {
                                    _stp_db_trunk_group[i].memberBmp = members[i].members.member_bmp;
                                    _stp_db_trunk_group[i].mode = members[i].members.mode;
                                }
                            }
                            else
                            {
                                _stp_db_trunk_group[e_idx-1].memberBmp = members[0].members.member_bmp;
                                _stp_db_trunk_group[e_idx-1].mode = members[i].members.mode;
                            }
                        }
                        stp_db_clear_all_port_block();
                        for (i = 0; i < MAX_TRUNK_NUM; i++)
                        {
                            STP_LOG_DBG("Trunk %d member %u, mode %d", _stp_db_trunk_group[i].memberBmp,
                                _stp_db_trunk_group[i].mode);
                        }
                    }
                    else
                    {
                        STP_LOG_WARN("unknown field id(%d) of table id(%d)\n",
                                ptr_request->f_idx, ptr_request->t_idx);
                    }
                    break; /* case TRUNK_PORT */
                }
                case PORT_OPER_INFO:
                {
                    if (PORT_OPER_STATUS == ptr_request->f_idx)
                    {
                        UI8_T   link = ((UI8_T*)ptr_data)[0];
                        UI8_T   all_linkdown = FALSE;
                        UI32_T  port_number = 0, trunk_id = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        UI8_T   auto_enable = 0;
                        UI32_T  lower_port = 0;
#endif
                        UI8_T duplex, is_p2p;

                        if (DB_ALL_ENTRIES != ptr_request->e_idx)
                        {
                            port_number = ptr_request->e_idx;
                            _stp_db_getTrunkID(port_number, &trunk_id);
                            if (TRUE == link)
                            {
                                if (MAX_TRUNK_NUM > trunk_id)
                                {
                                    port_number =STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                                }
                                if (TRUE == _stp_enable)
                                {
#ifdef AIR_SUPPORT_STP_AUTO_COST
                                    if (TRUE == _stp_auto_cost_support)
                                    {
                                        lower_port = ptr_request->e_idx;
                                        if (MAX_TRUNK_NUM > trunk_id)
                                        {
                                            lower_port = _stp_db_trunk_getLowerPort(trunk_id);
                                            stp_db_get_port_auto_enable_config(lower_port, &auto_enable);
                                        }
                                        else
                                        {
                                            stp_db_get_port_auto_enable_config(ptr_request->e_idx, &auto_enable);
                                        }

                                        if (TRUE == auto_enable)
                                        {
                                            _stp_port_auto_cost_set(ptr_request->e_idx);
                                        }
                                    }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                                    stp_db_get_port_duplex(ptr_request->e_idx, &duplex);
                                    is_p2p = (AIR_PORT_DUPLEX_FULL == duplex) ? TRUE : FALSE;
#ifdef AIR_SUPPORT_MSTP
                                    if (STP_VERSION_MSTP == _stp_version)
                                    {
                                        if (MAX_TRUNK_NUM > trunk_id)
                                        {
                                            STP_LOG_DBG("Trunk %d member port %d link up, mode %d", trunk_id,
                                                ptr_request->e_idx, _stp_db_trunk_group[trunk_id].mode);
                                            mstp_trunk_link_status_set(port_number, TRUE, _stp_db_trunk_group[trunk_id].mode);
                                        }
                                        else
                                        {
                                            STP_LOG_DBG("Port %d link up, p2p %d", port_number, is_p2p);
                                            mstp_port_link_status_set(port_number, is_p2p, TRUE);
                                        }

                                        break;
                                    }
#endif
#ifdef AIR_SUPPORT_RSTP
                                    if (MAX_TRUNK_NUM > trunk_id)
                                    {
                                        STP_LOG_DBG("Trunk %d member port %d link up, mode %d", trunk_id,
                                            ptr_request->e_idx, _stp_db_trunk_group[trunk_id].mode);
                                        rstp_trunk_link_status_set(port_number, TRUE, _stp_db_trunk_group[trunk_id].mode);
                                    }
                                    else
                                    {
                                        STP_LOG_DBG("Port %d link up, p2p %d", port_number, is_p2p);
                                        rstp_port_link_status_set(port_number, is_p2p, TRUE);
                                    }
#endif
                                }
                            }
                            else if (FALSE == link)
                            {
                                STP_LOG_DBG("Db recv port %d link down", port_number);
                                if (MAX_TRUNK_NUM > trunk_id)
                                {
                                    port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
#ifdef AIR_SUPPORT_STP_AUTO_COST
                                    if (TRUE == _stp_auto_cost_support)
                                    {
                                        lower_port = _stp_db_trunk_getLowerPort(trunk_id);
                                        stp_db_get_port_auto_enable_config(lower_port, &auto_enable);

                                        if (TRUE == auto_enable)
                                        {
                                            _stp_port_auto_cost_set(ptr_request->e_idx);
                                        }
                                    }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                                    all_linkdown = _stp_trunk_check_all_member_linkdown_excl_port(trunk_id, ptr_request->e_idx);
                                    if (FALSE == all_linkdown)
                                    {
                                        break; /* case PORT_OPER_STATUS break */
                                    }
                                }
                                if (TRUE == _stp_enable)
                                {
#ifdef AIR_SUPPORT_MSTP
                                    if (STP_VERSION_MSTP == _stp_version)
                                    {
                                        mstp_port_mac_operational_set(port_number, FALSE);
                                        break;
                                    }
#endif
#ifdef AIR_SUPPORT_RSTP
                                    rstp_port_mac_operational_set(port_number, FALSE);
#endif
                                }
                            }
                            else
                            {
                                STP_LOG_WARN("Unknown data of e_idx(%u)/f_idx(%u)/t_idx(%u).",
                                    ptr_request->e_idx, ptr_request->f_idx, ptr_request->t_idx);
                            }
                        }
                        else
                        {
                            STP_LOG_WARN("Not Support.");
                        }
                    }
                    else if (PORT_OPER_DUPLEX == ptr_request->f_idx)
                    {
                        UI8_T   duplex = ((UI8_T*)ptr_data)[0];
                        UI8_T   is_p2p = FALSE;
                        UI8_T   all_linkdown = FALSE;
                        UI32_T  port_number = 0, trunk_id = 0;

                        if (DB_ALL_ENTRIES != ptr_request->e_idx)
                        {
                            port_number = ptr_request->e_idx;
                            _stp_db_getTrunkID(port_number, &trunk_id);
                            if (AIR_PORT_DUPLEX_HALF == duplex || AIR_PORT_DUPLEX_FULL == duplex)
                            {
                                is_p2p = (AIR_PORT_DUPLEX_FULL == duplex) ? TRUE : FALSE;
                                STP_LOG_DBG("Port %d duplex change is_p2p %d", port_number, is_p2p);
                                if (MAX_TRUNK_NUM > trunk_id)
                                {
                                    is_p2p = TRUE;
                                    port_number = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                                    all_linkdown = _stp_trunk_check_all_member_linkdown_excl_port(trunk_id, ptr_request->e_idx);
                                    if (FALSE == all_linkdown)
                                    {
                                        break;
                                    }
                                }
                                if (TRUE == _stp_enable)
                                {
#ifdef AIR_SUPPORT_MSTP
                                    if (STP_VERSION_MSTP == _stp_version)
                                    {
                                        mstp_port_p2p_mac_set(port_number, is_p2p);
                                        break;
                                    }
#endif
#ifdef AIR_SUPPORT_RSTP
                                    rstp_port_p2p_mac_set(port_number, is_p2p);
#endif
                                }
                            }
                            else
                            {
                                STP_LOG_WARN("Unknown data of e_idx(%u)/f_idx(%u)/t_idx(%u).",
                                    ptr_request->e_idx, ptr_request->f_idx, ptr_request->t_idx);
                            }
                        }
                        else
                        {
                            STP_LOG_WARN("Not Support.");
                        }
                    }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                    else if (PORT_OPER_SPEED == ptr_request->f_idx)
                    {
                        UI8_T   speed_enum = ((UI8_T*)ptr_data)[0];
                        UI8_T   auto_enable = FALSE;
                        UI32_T  port_number = 0, trunk_id = 0;

                        if (TRUE == _stp_auto_cost_support)
                        {
                            if (DB_ALL_ENTRIES != ptr_request->e_idx)
                            {
                                port_number = ptr_request->e_idx;
                                _stp_db_getTrunkID(port_number, &trunk_id);
                                if (speed_enum < AIR_PORT_SPEED_LAST)
                                {
                                    if (MAX_TRUNK_NUM == trunk_id)
                                    {
                                        stp_db_get_port_auto_enable_config(port_number, &auto_enable);
                                        if ((TRUE == auto_enable) && (TRUE == _stp_enable))
                                        {
#ifdef AIR_SUPPORT_MSTP
                                            if (STP_VERSION_MSTP == _stp_version)
                                            {
                                                mstp_port_oper_cost_set(port_number);
                                                break;
                                            }
#endif
#ifdef AIR_SUPPORT_RSTP
                                            rstp_port_oper_cost_set(port_number);

#endif
                                        }
                                    }
                                }
                                else
                                {
                                    STP_LOG_WARN("Unknown data of e_idx(%u)/f_idx(%u)/t_idx(%u).",
                                        ptr_request->e_idx, ptr_request->f_idx, ptr_request->t_idx);
                                }
                            }
                            else
                            {
                                STP_LOG_WARN("Not Support.");
                            }
                        }
                    }
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                    else if (PORT_LACP_STATE == ptr_request->f_idx)
                    {
                        _stp_db_lacp_handle(ptr_request->e_idx, data_size, ptr_data);
                    }
                    else
                    {
                        STP_LOG_WARN("unknown field id(%d) of table id(%d)",
                                ptr_request->f_idx, ptr_request->t_idx);
                    }
                    break; /* case PORT_OPER_INFO */
                }
#ifdef AIR_SUPPORT_RSTP_SECURITY
                case RSTP_PORT_SECURITY:
                    if (DB_ALL_FIELDS == ptr_request->f_idx)
                    {
                        DB_RSTP_PORT_SEC_T p_info = {0};

                        osapi_memcpy(&p_info, ptr_data, sizeof(DB_RSTP_PORT_SEC_T));
                        if (DB_ALL_ENTRIES != ptr_request->e_idx)
                        {
                            STP_LOG_DBG("Port %d bpdu status set to %d", ptr_request->e_idx, p_info.bpdu_status);
                            rc = rstp_port_bpdu_status_set(ptr_request->e_idx, p_info.bpdu_status);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("BPDU_STATUS: Invalid parameter - %d", p_info.bpdu_status);
                            }

                            STP_LOG_DBG("Port %d tc protect status set to %d", ptr_request->e_idx, p_info.tc_status);
                            rc = rstp_port_tc_status_set(ptr_request->e_idx, p_info.tc_status);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("TC_STATUS: Invalid parameter - %d", p_info.tc_status);
                            }

                            STP_LOG_DBG("Port %d tc protect interval set to %d", ptr_request->e_idx, p_info.tc_interval);
                            rc = rstp_port_tc_interval_set(ptr_request->e_idx, p_info.tc_interval);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("TC_INTERVAL: Invalid parameter - %d", p_info.tc_interval);
                            }

                            STP_LOG_DBG("Port %d tc protect threshold set to %d", ptr_request->e_idx, p_info.tc_threshold);
                            rc = rstp_port_tc_threshold_set(ptr_request->e_idx, p_info.tc_threshold);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("TC_THRESHOLD: Invalid parameter - %d", p_info.tc_threshold);
                            }

                            STP_LOG_DBG("Port %d root protect status set to %d", ptr_request->e_idx, p_info.root_status);
                            rc = rstp_port_root_guard_status_set(ptr_request->e_idx, p_info.root_status);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("ROOT_STATUS: Invalid parameter - %d", p_info.root_status);
                            }

                            STP_LOG_DBG("Port %d loop protect status set to %d", ptr_request->e_idx, p_info.loop_status);
                            rc = rstp_port_loop_guard_status_set(ptr_request->e_idx, p_info.loop_status);
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("LOOP_STATUS: Invalid parameter - %d", p_info.loop_status);
                            }
                        }
                        else
                        {
                            UI8_T   status_arr[PLAT_MAX_PORT_NUM];
                            UI16_T  value_arr[PLAT_MAX_PORT_NUM];
                            UI16_T  last_port, port;
                            UI16_T offset = 0;

                            last_port = PLAT_MAX_PORT_NUM;

                            osapi_memset(status_arr, 0, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            osapi_memset(value_arr, 0, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);

                            osapi_memcpy(status_arr, ptr_data + offset, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d bpdu status set to %d", port, status_arr[port-1]);
                                rc |= rstp_port_bpdu_status_set(port, status_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_BPDU: Invalid parameter");
                            }
                            offset += sizeof(UI8_T) * PLAT_MAX_PORT_NUM;

                            osapi_memcpy(status_arr, ptr_data + offset, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d tc protect status set to %d", port, status_arr[port-1]);
                                rc |= rstp_port_tc_status_set(port, status_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_GUARD: Invalid parameter");
                            }
                            offset += sizeof(UI8_T) * PLAT_MAX_PORT_NUM;

                            osapi_memcpy(status_arr, ptr_data + offset, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d root protect status set to %d", port, status_arr[port-1]);
                                rc |= rstp_port_root_guard_status_set(port, status_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_ROOT_GUARD: Invalid parameter");
                            }
                            offset += sizeof(UI8_T) * PLAT_MAX_PORT_NUM;

                            osapi_memcpy(status_arr, ptr_data + offset, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d tc protect status set to %d", port, status_arr[port-1]);
                                rc |= rstp_port_loop_guard_status_set(port, status_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_LOOP_GUARD: Invalid parameter");
                            }
                            offset += sizeof(UI8_T) * PLAT_MAX_PORT_NUM;

                            osapi_memcpy(value_arr, ptr_data + offset, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d tc protect interval set to %d", port, value_arr[port-1]);
                                rc |= rstp_port_tc_interval_set(port, value_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_INTERVAL: Invalid parameter");
                            }
                            offset += sizeof(UI16_T) * PLAT_MAX_PORT_NUM;

                            osapi_memcpy(value_arr, ptr_data + offset, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);
                            for (port = 1; port <= last_port ; port++)
                            {
                                STP_LOG_DBG("Port %d tc protect threshold set to %d", port, value_arr[port-1]);
                                rc |= rstp_port_tc_threshold_set(port, value_arr[port-1]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_THRESHOLD: Invalid parameter");
                            }
                        }
                    }
                    else
                    {
                        UI32_T  first_port = 0, last_port = 0, port = 0, i = 0;
                        UI8_T   status_arr[PLAT_MAX_PORT_NUM];
                        UI16_T  value_arr[PLAT_MAX_PORT_NUM];

                        osapi_memset(status_arr, 0, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                        osapi_memset(value_arr, 0, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);

                        if (DB_ALL_ENTRIES == ptr_request->e_idx)
                        {
                            first_port = 1;
                            last_port = PLAT_MAX_PORT_NUM;
                        }
                        else /* DB_ALL_ENTRIES != ptr_request->e_idx */
                        {
                            first_port = last_port = ptr_request->e_idx;
                        }
                        if (RSTP_PORT_SEC_BPDU_GUARD_ENABLE == ptr_request->f_idx)
                        {
                            osapi_memcpy(status_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d bpdu status set to %d", port, status_arr[i]);
                                rc |= rstp_port_bpdu_status_set(port, status_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_BPDU: Invalid parameter");
                            }
                        }
                        else if (RSTP_PORT_SEC_TC_GUARD_ENABLE == ptr_request->f_idx)
                        {
                            osapi_memcpy(status_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d tc protect status set to %d", port, status_arr[i]);
                                rc |= rstp_port_tc_status_set(port, status_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_GUARD: Invalid parameter");
                            }
                        }
                        else if (RSTP_PORT_SEC_TC_GUARD_INTERVAL == ptr_request->f_idx)
                        {
                            osapi_memcpy(value_arr, ptr_data, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d tc protect interval set to %d", port, value_arr[i]);
                                rc |= rstp_port_tc_interval_set(port, value_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_INTERVAL: Invalid parameter");
                            }
                        }
                        else if (RSTP_PORT_SEC_TC_GUARD_THRESHOLD == ptr_request->f_idx)
                        {
                            osapi_memcpy(value_arr, ptr_data, sizeof(UI16_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d tc protect threshold set to %d", port, value_arr[i]);
                                rc |= rstp_port_tc_threshold_set(port, value_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_TC_THRESHOLD: Invalid parameter");
                            }
                        }
                        else if (RSTP_PORT_SEC_ROOT_GUARD_ENABLE == ptr_request->f_idx)
                        {
                            osapi_memcpy(status_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d root protect status set to %d", port, status_arr[i]);
                                rc |= rstp_port_root_guard_status_set(port, status_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_ROOT_GUARD: Invalid parameter");
                            }
                        }
                        else if (RSTP_PORT_SEC_LOOP_GUARD_ENABLE == ptr_request->f_idx)
                        {
                            osapi_memcpy(status_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
                            for (port = first_port, i = 0; port <= last_port ; port++, i++)
                            {
                                STP_LOG_DBG("Port %d loop protect status set to %d", port, status_arr[i]);
                                rc |= rstp_port_loop_guard_status_set(port, status_arr[i]);
                            }
                            if (MW_E_OK != rc)
                            {
                                STP_LOG_ERR("RSTP_SEC_LOOP_GUARD: Invalid parameter");
                            }
                        }
                    }
                    break; /* case RSTP_PORT_SECURITY */
                case PORT_CFG_INFO:
                    if (PORT_ADMIN_STATUS != ptr_request->f_idx)
                    {
                        return rc;
                    }

                    if (DB_ALL_ENTRIES != ptr_request->e_idx)
                    {
                        UI8_T state;
                        UI32_T port;

                        port = ptr_request->e_idx;
                        osapi_memcpy(&state, ptr_data, sizeof(UI8_T));
                        if ((true == state) && ((rstp_port_sec_state.bg_state >> (port - 1)) & 0x1))
                        {
                            stp_db_port_bpdu_state_update(port, false);
                        }
                    }
                    else
                    {
                        UI8_T   state_arr[PLAT_MAX_PORT_NUM];
                        UI32_T  port = 0, i = 0;

                        osapi_memcpy(state_arr, ptr_data, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);

                        for (port = 1, i = 0; port <= PLAT_MAX_PORT_NUM ; port++, i++)
                        {
                            if ((true == state_arr[i]) && ((rstp_port_sec_state.bg_state >> (port - 1)) & 0x1))
                            {
                                stp_db_port_bpdu_state_update(ptr_request->e_idx, false);
                            }
                        }
                    }
                    break;
#endif
#ifdef AIR_SUPPORT_MSTP
                case MSTP_REGION:
                    {
                        if (DB_ALL_FIELDS == ptr_request->f_idx)
                        {
                            DB_MSTP_REGION_CONFIG_T info = {0};

                            osapi_memcpy(&info, ptr_data, sizeof(DB_MSTP_REGION_CONFIG_T));
                            STP_LOG_DBG("Set mstp region, revision %d, region name %s\n",
                                info.revision, info.region_name);
                            mstp_set_region_config(info.revision, info.region_name);
                        }
                        else
                        {
                            STP_LOG_WARN("unknown field id(%d) of table id(%d)\n",
                                    ptr_request->f_idx, ptr_request->t_idx);
                            return rc;
                        }
                    }
                    break;
                case MSTP_INSTANCE:
                    {
                        UI16_T                  *ptr_ins;
                        UI16_T                  *ptr_pri;
                        UI32_T                  *ptr_vlanBmp, num;
                        UI32_T                  i = 0, idx = 0;
                        UI16_T                  vid[MAX_VLAN_ENTRY_NUM] = {0};
                        MST_INSTANCE_VLAN_ENTRY *ptr_entry;

                        mstp_clear_mstTableInstance();

                        if (DB_ALL_FIELDS == ptr_request->f_idx)
                        {
                            if (DB_ALL_ENTRIES == ptr_request->e_idx)
                            {
                                ptr_ins = (UI16_T *)ptr_data;
                                ptr_pri = (UI16_T *)((UI8_T *)ptr_data + (sizeof(UI16_T) * MAX_MSTP_INSTANCE_NUM));
                                ptr_vlanBmp = (UI32_T *)((UI8_T *)ptr_data + (2 * sizeof(UI16_T) * MAX_MSTP_INSTANCE_NUM));

                                for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
                                {
                                    idx = (ptr_ins[i] & 0xf000) >> 12;
                                    if ((0 == idx) && (0 != i))
                                    {
                                        continue;
                                    }
                                    STP_LOG_DBG("Mstp config, instance %d, fid %d, priority %d, vlanBmp %d",
                                        (ptr_ins[i] & 0xfff), idx, ptr_pri[i], ptr_vlanBmp[i]);

                                    num = 0;
                                    if (0 != ptr_vlanBmp[i])
                                    {
                                        mstp_get_mstVlanArrayByBmp(ptr_vlanBmp[i], vid, &num);
                                    }
                                    for (idx = 0; idx < num; idx++)
                                    {
                                        if (0 == vid[idx])
                                        {
                                            break;
                                        }
                                        ptr_entry = mstp_get_mstTableByVid(vid[idx]);
                                        if (NULL == ptr_entry)
                                        {
                                            continue;
                                        }
                                        ptr_entry->instance = ptr_ins[i];
                                    }
                                }
                                if (STP_VERSION_MSTP != stp_db_getVersion())
                                {
                                    break;
                                }
                                mstp_update_all_vlan_port();
                                for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
                                {
                                    idx = (ptr_ins[i] & 0xf000) >> 12;
                                    if ((0 == idx) && (0 != i))
                                    {
                                        continue;
                                    }
                                    mstp_instance_priority_set(ptr_ins[i], ptr_pri[i]);
                                }
                                mstp_set_instanceMappingVlan();
                            }
                        }
                    }
                    break;

                case MSTP_INSTANCE_PORT:
                    {
                        DB_MSTP_INSTANCE_PORT_INFO_T *ptr_port_info;
                        UI32_T     port_id, trunk_id, port_num;
                        UI32_T     treeIdx, len, offset;
                        UI8_T      ppri[MAX_MSTP_INSTANCE_NUM];
                        UI32_T     pcost[MAX_MSTP_INSTANCE_NUM];

                        if (DB_ALL_FIELDS == ptr_request->f_idx)
                        {
                            ptr_port_info = (DB_MSTP_INSTANCE_PORT_INFO_T *)ptr_data;
                            if (DB_ALL_ENTRIES != ptr_request->e_idx)
                            {
                                port_id = ptr_request->e_idx;
                                _stp_db_getTrunkID(port_id, &trunk_id);
                                if (MAX_TRUNK_NUM > trunk_id)
                                {
                                    port_id = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                                }
                                for (treeIdx = 0; treeIdx < MAX_MSTP_INSTANCE_NUM; treeIdx++)
                                {
                                    STP_LOG_DBG("Set port %d fid %d priority %d cost %d", port_id, treeIdx,
                                        ptr_port_info->priority[treeIdx], ptr_port_info->cost[treeIdx]);
                                    mstp_set_portPriority(port_id, treeIdx, ptr_port_info->priority[treeIdx]);
                                    mstp_set_portPathCost(port_id, treeIdx, ptr_port_info->cost[treeIdx]);
                                }
                            }
                            else
                            {
                                offset = sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM * PLAT_MAX_PORT_NUM;
                                for (port_id = 0; port_id < PLAT_MAX_PORT_NUM; port_id++)
                                {
                                    len = sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM;
                                    osapi_memcpy(ppri, ((UI8_T *)ptr_data)+(port_id * len), len);
                                    len = sizeof(UI32_T) * MAX_MSTP_INSTANCE_NUM;
                                    osapi_memcpy(pcost, ((UI8_T *)ptr_data)+(port_id * len)+offset, len);
                                    port_num = port_id+1;
                                    _stp_db_getTrunkID(port_num, &trunk_id);
                                    if (MAX_TRUNK_NUM > trunk_id)
                                    {
                                        port_num = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                                    }

                                    for (treeIdx = 0; treeIdx < MAX_MSTP_INSTANCE_NUM; treeIdx++)
                                    {
                                        STP_LOG_DBG("Set port %d fid %d priority %d cost %d", port_num, treeIdx,
                                            ppri[treeIdx], pcost[treeIdx]);
                                        mstp_set_portPriority(port_num, treeIdx, ppri[treeIdx]);
                                        mstp_set_portPathCost(port_num, treeIdx, pcost[treeIdx]);
                                    }
                                }
                            }
                        }
                        else
                        {
                            UI32_T  port = ptr_request->e_idx;
                            UI8_T   *ptr_pri;
                            UI32_T  *ptr_cost;

                            _stp_db_getTrunkID(port, &trunk_id);
                            if (MAX_TRUNK_NUM > trunk_id)
                            {
                                port = STP_TRUNK_ID_TO_PORT_NUMBER(trunk_id);
                            }

                            if (DB_ALL_ENTRIES != ptr_request->e_idx)
                            {
                                if (MSTP_INSTANCE_PORT_PRIORITY == ptr_request->f_idx)
                                {
                                    ptr_pri = (UI8_T *)ptr_data;

                                    for (treeIdx = 0; treeIdx < MAX_MSTP_INSTANCE_NUM; treeIdx++)
                                    {
                                        STP_LOG_DBG("Set port %d fid %d priority %d", port, treeIdx,
                                            ptr_pri[treeIdx]);
                                        mstp_set_portPriority(port, treeIdx, ptr_pri[treeIdx]);
                                    }
                                }
                                else if (MSTP_INSTANCE_PORT_COST == ptr_request->f_idx)
                                {
                                    ptr_cost = (UI32_T *)ptr_data;

                                    for (treeIdx = 0; treeIdx < MAX_MSTP_INSTANCE_NUM; treeIdx++)
                                    {
                                        STP_LOG_DBG("Set port %d fid %d cost %d", port, treeIdx,
                                            ptr_cost[treeIdx]);
                                        mstp_set_portPathCost(port, treeIdx, ptr_cost[treeIdx]);
                                    }
                                }
                            }
                        }
                    }
                    break;
                    case VLAN_ENTRY:
                    {
                        mstp_update_all_vlan_msg(method, ptr_request->f_idx, ptr_request->e_idx, (void *)ptr_data);
                    }
                    break;
                    case VLAN_CFG_INFO:
                    {
                        DB_VLAN_CFG_INFO_T *ptr_vlan_cfg = (DB_VLAN_CFG_INFO_T *)ptr_data;
                        UI8_T              mode = 0;

                        if (DB_ALL_FIELDS == ptr_request->f_idx)
                        {
                            if (DB_ALL_ENTRIES == ptr_request->e_idx)
                            {
                                if (TRUE == ptr_vlan_cfg->enable_port_b)
                                {
                                    mode = VLAN_PORT_ENABLE;
                                }
                                else if (TRUE == ptr_vlan_cfg->enable_8021q_b)
                                {
                                    mode = VLAN_1Q_ENABLE;
                                }
                                else if (TRUE == ptr_vlan_cfg->enable_mtu)
                                {
                                    mode = VLAN_MTU_ENABLE;
                                }

                                mstp_update_vlan_mode_change(mode);
                            }
                        }
                    }
                    break;
#endif
                default:
                    STP_LOG_WARN("Do not handle field id(%d) of table id(%d)",
                            ptr_request->f_idx, ptr_request->t_idx);
                    break;
            }
            break; /* M_UPDATE */
        case M_RESPONSE:
            STP_LOG_DBG("response result code");
            break;
        case M_ACK:
            STP_LOG_DBG("ack result code");
            break;
        case M_DELETE:
#ifdef AIR_SUPPORT_MSTP
            switch(ptr_request->t_idx)
            {
                case VLAN_ENTRY:
                    {
                        mstp_update_vlan_del_msg(ptr_request->f_idx, ptr_request->e_idx);
                    }
                    break;
                default:
                    break;
            }
#endif
            STP_LOG_DBG("delete result code");
            break;
        default:
            break;
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: stp_db_getStpGlobalState
 * PURPOSE:
 *      Get stp status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getStpGlobalState(
    void)
{
    return _stp_enable;
}

/* FUNCTION NAME: stp_db_getAutoCostState
 * PURPOSE:
 *      Get stp auto cost config status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getAutoCostState(
    void)
{
    UI8_T   state = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    state = _stp_auto_cost_support;
#endif

    return state;
}

/* FUNCTION NAME: stp_db_setAutoCostState
 * PURPOSE:
 *      Set stp auto cost config status
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
void
stp_db_setAutoCostState(
    UI8_T enable)
{
#ifdef AIR_SUPPORT_STP_AUTO_COST
    _stp_auto_cost_support = enable;
#endif
}

/* FUNCTION NAME: stp_db_getVersion
 * PURPOSE:
 *      Get stp version mode
 *
 * INPUT:
 *      void
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      stp_force_version
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_getVersion(
    void)
{
    return _stp_version;
}

/* FUNCTION NAME: stp_db_setVersion
 * PURPOSE:
 *      Set stp version mode
 *
 * INPUT:
 *      version       - version mode
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
stp_db_setVersion(
    UI8_T  version)
{
    _stp_version = version;
}


/* FUNCTION NAME: stp_db_getTrunkID
 * PURPOSE:
 *      Get trunk id by port id.
 *
 * INPUT:
 *      port             -  port id
 *
 * OUTPUT:
 *      ptr_trunk_id     -  trunk id
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_getTrunkID(
    UI32_T  port,
    UI32_T  *ptr_trunk_id)
{
    MW_CHECK_PTR(ptr_trunk_id);

    _stp_db_getTrunkID(port, ptr_trunk_id);
    return MW_E_OK;
}

/* FUNCTION NAME: stp_db_checkIsLowerPort
 * PURPOSE:
 *      Check port is lower port of trunk or not.
 *
 * INPUT:
 *      port             -  port id
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE or FALSE
 *
 * NOTES:
 *      None
 */

BOOL_T
stp_db_checkIsLowerPort(
    UI32_T  port,
    UI32_T  trunk_id)

{
    return _stp_db_checkIsLowerPort(port, trunk_id);
}

/* FUNCTION NAME: stp_db_trunk_getLowerPort
 * PURPOSE:
 *      Get the lower port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Lower port id
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getLowerPort(
    UI32_T  trunk_id)
{
    return _stp_db_trunk_getLowerPort(trunk_id);
}

/* FUNCTION NAME: stp_db_trunk_getMode
 * PURPOSE:
 *      Get the mode of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk mode
 *
 * NOTES:
 *      None
 */
UI8_T
stp_db_trunk_getMode(
    UI32_T  trunk_id)
{
    UI8_T   mode;

    mode = _stp_db_trunk_group[trunk_id].mode;
    return mode;
}

/* FUNCTION NAME: stp_db_trunk_setMode
 * PURPOSE:
 *      Set the mode of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk mode
 *
 * NOTES:
 *      None
 */
void
stp_db_trunk_setMode(
    UI32_T  trunk_id,
    UI8_T   mode)
{
    _stp_db_trunk_group[trunk_id].mode = mode;
}

/* FUNCTION NAME: stp_db_trunk_getMemberBmp
 * PURPOSE:
 *      Get the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk member
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getMemberBmp(
    UI32_T  trunk_id)
{
    UI32_T   member;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    return member;
}

/* FUNCTION NAME: stp_db_trunk_setMemberBmp
 * PURPOSE:
 *      Set the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *      member           -  member port
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
stp_db_trunk_setMemberBmp(
    UI32_T  trunk_id,
    UI32_T   member)
{
    _stp_db_trunk_group[trunk_id].memberBmp = member;
}


/* FUNCTION NAME: stp_db_trunk_getAggBmp
 * PURPOSE:
 *      Get the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk member
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_trunk_getAggBmp(
    UI32_T  trunk_id)
{
    UI32_T   member;

    member = _stp_db_trunk_group[trunk_id].aggregatedBmp;
    return member;
}

/* FUNCTION NAME: stp_db_trunk_setAggBmp
 * PURPOSE:
 *      Set the member port of trunk.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *      member           -  member port
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
stp_db_trunk_setAggBmp(
    UI32_T  trunk_id,
    UI32_T   member)
{
     _stp_db_trunk_group[trunk_id].aggregatedBmp = member;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_trunk_compute_oper_cost
 * PURPOSE:
 *      Calc trunk oper cost.
 *
 * INPUT:
 *      trunk_id         -  trunk id
 *
 * OUTPUT:
 *      ptr_oper_cost    -  trunk oper cost
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
stp_trunk_compute_oper_cost(
    UI32_T  trunk_id,
    UI32_T  *ptr_oper_cost)
{
    UI32_T    p, member;
    UI32_T    cost = 0;
    UI8_T     link, speed_enum = 0;
    UI32_T    cost_int;

    if (LAG_MODE_STATIC == _stp_db_trunk_group[trunk_id].mode)
    {
        member = _stp_db_trunk_group[trunk_id].memberBmp;
        for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
        {
            if ((member & BIT(p)) > 0)
            {
                stp_db_get_port_status((p+1), &link);
                if (TRUE == link)
                {
                    stp_db_get_port_speed((p+1), &speed_enum);
                    _stp_speed_enum_to_integer(speed_enum, &cost_int);
                    cost += cost_int;
                }
            }
        }
    }
    else
    {
        member = _stp_db_trunk_group[trunk_id].aggregatedBmp;
        for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
        {
            if ((member & BIT(p)) > 0)
            {
                stp_db_get_port_status((p+1), &link);
                if (TRUE == link)
                {
                    stp_db_get_port_speed((p+1), &speed_enum);
                    _stp_speed_enum_to_integer(speed_enum, &cost_int);
                    cost += cost_int;
                }
            }
        }
    }

    *ptr_oper_cost = (cost >= 10) ? (2000000/(cost/10)) : 2000000;
    STP_LOG_DBG("Trunk %d calc oper cost %d", trunk_id, *ptr_oper_cost);
    return;
}
#endif

/* FUNCTION NAME: stp_db_get_msg
 * PURPOSE:
 *      Get msg by db
 *
 * INPUT:
 *      req             -  table idx
 *                      -  field idx
 *                      -  entry idx
 *
 * OUTPUT:
 *      ptr_outData     -  output data
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_msg(
    DB_REQUEST_TYPE_T   req,
    void                *ptr_outData)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg  = NULL;
    UI8_T               *ptr_data = NULL;

    MW_CHECK_PTR(ptr_outData);

    rc = _stp_db_queue_create(STP_DB_QUEUE_NAME_CMD);
    if (MW_E_OK == rc)
    {
        rc = _stp_db_getData(M_GET, req, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK == rc)
        {
            osapi_memcpy(ptr_outData, ptr_data, data_size);
            MW_FREE(ptr_msg);
            _stp_db_queue_delete(STP_DB_QUEUE_NAME_CMD);
        }
    }
    else
    {
        STP_LOG_ERR("Create queue(%s) for db failed!", STP_DB_QUEUE_NAME_CMD);
    }

    return rc;
}

/* FUNCTION NAME: stp_db_update_msg
 * PURPOSE:
 *      Update msg by db
 *
 * INPUT:
 *      req             -  table idx
 *                      -  field idx
 *                      -  entry idx
 *      ptr_inData      -  input data
 *      len             -  data length
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_update_msg(
    DB_REQUEST_TYPE_T   req,
    void *const         ptr_inData,
    const UI16_T        len)
{
    return _stp_db_updateData(STP_DB_QUEUE_NAME_EMPTY, req, ptr_inData, len);
}

/* FUNCTION NAME: stp_db_port_update_allEntries
 * PURPOSE:
 *      Update t_id, f_id, DB_ALL_ENTRIES to DB.
 *
 * INPUT:
 *      t_id           -  DB table ID
 *      f_id           -  DB field ID
 *      ptr_data       -  A pointer to data for each entry.
 *                        Its type must be aligned with DB the field.
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_update_allEntries(
    const UI8_T t_id,
    const UI8_T f_id,
    const void *ptr_data)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    DB_REQUEST_TYPE_T req;
    UI8_T *ptr_data_all = NULL;
    UI16_T data_size = 0, data_size_all = 0;
    UI32_T i = 0, entry_num = 0;

    if (f_id == DB_ALL_FIELDS)
    {
        return MW_E_NOT_SUPPORT;
    }

    req = stp_dbmsg_init_reqtype(t_id, f_id, 1);
    rc = db_getDataSize(req, &data_size);

    req = stp_dbmsg_init_reqtype(t_id, f_id, DB_ALL_ENTRIES);
    rc |= db_getDataSize(req, &data_size_all);

    if ((MW_E_OK != rc) || (0 == data_size) || (0 == data_size_all))
    {
        return MW_E_BAD_PARAMETER;
    }

    entry_num = data_size_all / data_size;

    osapi_calloc(data_size_all, STP_MODULE_NAME, (void **)&ptr_data_all);
    if (NULL != ptr_data_all)
    {
        for (i = 0; i < entry_num; i++)
        {
            osapi_memcpy(ptr_data_all + (i * data_size), ptr_data, data_size);
        }

        rc = _stp_db_updateData(STP_DB_QUEUE_NAME_EMPTY, req, (void *)ptr_data_all, data_size_all);
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to update table(%d) field(%d) ALL_ENTRIES to DB", t_id, f_id);
        }

        MW_FREE(ptr_data_all);
    }

    return rc;
}

/* FUNCTION NAME: stp_db_port_role_update
 * PURPOSE:
 *      Update port role.
 *
 * INPUT:
 *      port         -  port id
 *      role         -  role
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_role_update(
    const UI32_T    port,
    const UI8_T     role)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (PLAT_MAX_PORT_NUM < port)
    {
        return _stp_db_trunk_port_role_update(STP_PORT_NUMBER_TO_TRUNK_ID(port), role);
    }

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_ROLE, port);
        rc = stp_db_update_msg(req_type, (void *)(&role), sizeof(role));
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to update port %d role %d", port, role);
        }
    }

    return rc;
}

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: mstp_db_port_role_update
 * PURPOSE:
 *      Send to DB to update port instance role
 *
 * INPUT:
 *      port           -  port number
 *      ptr_role       -  port role
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_db_port_role_update(
    const UI32_T    port,
    UI8_T           *ptr_role)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              i;
    UI32_T              member = 0, trunk_id;

    if (PLAT_MAX_PORT_NUM < port)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        member = stp_db_trunk_getMemberBmp(trunk_id);
        for (i = 0 ; i < PLAT_MAX_PORT_NUM; i++)
        {
            if ((member & BIT(i)) > 0)
            {
                req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_ROLE, (i+1));
                rc = stp_db_update_msg(req_type, (void *)ptr_role, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);
                if (MW_E_OK != rc)
                {
                    STP_LOG_ERR("Fail to send port %d instance role to DB", port);
                    break;
                }
            }
        }

        return rc;
    }

    req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_ROLE, port);
    rc = stp_db_update_msg(req_type, (void *)ptr_role, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to send port %d instance role to DB", port);
    }

    return rc;
}
#endif
/* FUNCTION NAME: stp_db_get_bpdu_egress_port
 * PURPOSE:
 *      Get egress port to transmit BPDU.
 *
 * INPUT:
 *      port_number    -  port number of RSTP port instance
 *
 * OUTPUT:
 *      ptr_egr_port   -  port to egress BPDU
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
void
stp_db_get_bpdu_egress_port(
    UI32_T  port_number,
    UI32_T  *ptr_egr_port)
{
    UI32_T  trunk_id = 0;
    UI32_T  p = 0;
    UI32_T  member;
    UI8_T   mode;

    *ptr_egr_port = port_number;
    if (PLAT_MAX_PORT_NUM < port_number)
    {
        trunk_id = port_number - 1 - PLAT_MAX_PORT_NUM;
        mode = _stp_db_trunk_group[trunk_id].mode;
        if (LAG_MODE_STATIC != mode)
        {
            member =  _stp_db_trunk_group[trunk_id].aggregatedBmp;
        }
        else
        {
            member =  _stp_db_trunk_group[trunk_id].memberBmp;
        }

        for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
        {
            /* Get lower port of trunk members */
            if ((member & BIT(p)) > 0)
            {
                *ptr_egr_port = p + 1;
                break;
            }
        }
    }
}

/* FUNCTION NAME: stp_db_get_lag_lower_port
 * PURPOSE:
 *      Get the lower port of specify lag.
 *
 * INPUT:
 *      port_number    -  port number of RSTP port instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      lower port
 *
 * NOTES:
 *      None
 */
UI32_T
stp_db_get_lag_lower_port(
    UI32_T  port_number)
{
    UI32_T  port, trunk_id;

    port = port_number;
    if (PLAT_MAX_PORT_NUM < port_number)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port_number);
        port = _stp_db_trunk_getLowerPort(trunk_id);
    }

    return port;
}

/* FUNCTION NAME: stp_db_get_port_status
 * PURPOSE:
 *      Get port status.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_status    - port status
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_status(
    UI32_T  port,
    UI8_T   *ptr_status)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_REQUEST_TYPE_T           req_type;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_STATUS, port_number);
    rc = stp_db_get_msg(req_type, ptr_status);

    return rc;
}

/* FUNCTION NAME: stp_db_get_port_duplex
 * PURPOSE:
 *      Get port duplex.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_duplex    - port duplex
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_duplex(
    UI32_T  port,
    UI8_T   *ptr_duplex)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_REQUEST_TYPE_T           req_type;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_DUPLEX, port_number);
    rc = stp_db_get_msg(req_type, ptr_duplex);
    return rc;
}

/* FUNCTION NAME: stp_db_get_port_speed
 * PURPOSE:
 *      Get port speed.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_speed    - port speed
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_speed(
    UI32_T  port,
    UI8_T   *ptr_speed)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_REQUEST_TYPE_T           req_type;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_SPEED, port_number);
    rc = stp_db_get_msg(req_type, ptr_speed);

    return rc;
}

/* FUNCTION NAME: stp_db_get_port_enable_status
 * PURPOSE:
 *      Get port stp status.
 *
 * INPUT:
 *      port          -  port num
 *
 * OUTPUT:
 *      ptr_status    - port status
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_enable_status(
    UI32_T  port,
    UI8_T   *ptr_status)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0, trunk_id;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_STATUS, port_number);
    rc = stp_db_get_msg(req_type, ptr_status);

    return rc;
}

/* FUNCTION NAME: stp_db_get_forward_delay_config
 * PURPOSE:
 *      Get forward delay.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_fwd_delay    - fwd delay
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_forward_delay_config(
    UI16_T  *ptr_fwd_delay)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORWARD_DELAY, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_fwd_delay);

    return rc;
}

/* FUNCTION NAME: stp_db_get_max_age_config
 * PURPOSE:
 *      Get max age.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_max_age    - max age
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_max_age_config(
    UI16_T  *ptr_max_age)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_MAX_AGE, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_max_age);

    return rc;
}

/* FUNCTION NAME: stp_db_get_transmit_hold_count_config
 * PURPOSE:
 *      Get max age.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_tx_hold_cnt    - tx hold count
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_transmit_hold_count_config(
    UI16_T  *ptr_tx_hold_cnt)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_TRANSMIT_HOLD_COUNT, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_tx_hold_cnt);

    return rc;
}

/* FUNCTION NAME: stp_db_get_bridge_priority_config
 * PURPOSE:
 *      Get bridge priority.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_pri    - bridge priority
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_bridge_priority_config(
    UI16_T  *ptr_pri)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_BRIDGE_PRIORITY, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_pri);

    return rc;
}

/* FUNCTION NAME: stp_db_get_port_priority_config
 * PURPOSE:
 *      Get port priority.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_pri    - prot priority
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_priority_config(
    UI32_T  port,
    UI8_T   *ptr_pri)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }
    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_PRIORITY, port_number);
    rc = stp_db_get_msg(req_type, ptr_pri);

    return rc;
}

/* FUNCTION NAME: stp_db_get_port_cost_config
 * PURPOSE:
 *      Get port cost.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_cost    - prot cost
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_cost_config(
    UI32_T  port,
    UI32_T   *ptr_cost)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }
    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_COST, port_number);
    rc = stp_db_get_msg(req_type, ptr_cost);

    return rc;
}

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: mstp_db_get_instance_vlan_config
 * PURPOSE:
 *      Get instance-vlan-mapping config.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_info    - instance-vlan-mapping config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_db_get_instance_vlan_config(
    MSTP_INSTANCE_CONFIG_T *ptr_info)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    DB_REQUEST_TYPE_T       req_type;

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc = stp_db_get_msg(req_type, ptr_info);

    return rc;
}
#endif

/* FUNCTION NAME: stp_trunk_check_link_status
 * PURPOSE:
 *      Get the link status of trunk.
 *
 * INPUT:
 *      trunk_id      - trunk id
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE/FALSE
 *
 * NOTES:
 *      None
 */
UI8_T
stp_trunk_check_link_status(
    UI32_T trunk_id)
{
    UI32_T      p = 0;
    UI8_T       link = 0;
    UI32_T      member = 0;

    if (LAG_MODE_STATIC == _stp_db_trunk_group[trunk_id].mode)
    {
        member = _stp_db_trunk_group[trunk_id].memberBmp;
    }
    else
    {
        member = _stp_db_trunk_group[trunk_id].aggregatedBmp;
    }

    for (p = 0; p < PLAT_MAX_PORT_NUM; p++)
    {
        if ((member & BIT(p)) > 0)
        {
            stp_db_get_port_status(p+1, &link);
            if (true == link)
            {
                return true;
            }
        }
    }

    return false;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_db_get_port_auto_enable_config
 * PURPOSE:
 *      Get port auto cost config.
 *
 * INPUT:
 *      port             - port id
 *
 * OUTPUT:
 *      ptr_auto_enable  - config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_auto_enable_config(
    UI32_T  port,
    UI8_T   *ptr_auto_enable)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_REQUEST_TYPE_T           req_type;

    if (TRUE == _stp_auto_cost_support)
    {
        port_number = port;
        if (port > PLAT_MAX_PORT_NUM)
        {
            trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
            port_number = _stp_db_trunk_getLowerPort(trunk_id);
        }

        req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_AUTO_COST_ENABLE, port_number);
        rc = stp_db_get_msg(req_type, ptr_auto_enable);
    }
    else
    {
        *ptr_auto_enable = FALSE;
    }

    return rc;
}

/* Converts the link speed to a port path cost [Table 17-3]. */
UI32_T
stp_db_convert_speed_to_cost(unsigned int speed)
{
    UI32_T value;

    value = speed >= 10000000 ? 2 /* 10 Tb/s. */
          : speed >= 1000000 ? 20 /* 1 Tb/s. */
          : speed >= 100000 ? 200 /* 100 Gb/s. */
          : speed >= 10000 ? 2000 /* 10 Gb/s. */
          : speed >= 1000 ? 20000 /* 1 Gb/s. */
          : speed >= 100 ? 200000 /* 100 Mb/s. */
          : speed >= 10 ? 2000000 /* 10 Mb/s. */
          : speed >= 1 ? 20000000 /* 1 Mb/s. */
          : 200000000; /* <=100 Kbps. */

    return value;
}

/* FUNCTION NAME: stp_db_port_compute_oper_cost
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      port                     -  port number
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto_cost_support
 *      ptr_oper_cost            -  oper cost
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_compute_oper_cost(
    UI32_T  port,
    UI8_T  *ptr_auto_cost_support,
    UI32_T  *ptr_oper_cost)
{
    UI32_T  trunk_id = 0;
    UI8_T   auto_enable = 0, speed_enum = 0;
    UI32_T  speed_int = 0, config_cost = 0, auto_cost = 0;

    stp_db_get_port_cost_config(port, &config_cost);
    stp_db_get_port_auto_enable_config(port, &auto_enable);

    if (TRUE == _stp_auto_cost_support)
    {
        if (FALSE == auto_enable)
        {
            *ptr_oper_cost = config_cost;
        }
        else
        {
            if (port > PLAT_MAX_PORT_NUM)
            {
                trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
                stp_trunk_compute_oper_cost(trunk_id, &auto_cost);
            }
            else
            {
                stp_db_get_port_speed(port, &speed_enum);
                stp_speed_enum_to_integer(speed_enum, &speed_int);
                auto_cost = stp_db_convert_speed_to_cost(speed_int);
            }
            *ptr_oper_cost = auto_cost;
        }
    }
    else
    {
        *ptr_oper_cost = config_cost;
    }

    if (NULL != ptr_auto_cost_support)
    {
        *ptr_auto_cost_support = _stp_auto_cost_support;
    }

    STP_LOG_DBG("port=%u, auto_enable=%u, auto_cost=%u, oper_cost=%u",
        port, auto_enable, auto_cost, *ptr_oper_cost);

    return MW_E_OK;
}

/* FUNCTION NAME: stp_db_port_compute_oper_cost
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      port                     -  port number
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto_cost_support
 *      ptr_oper_cost            -  oper cost
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_db_trunk_port_oper_cost_update(
    UI32_T          trunk_id,
    const UI32_T    oper_cost)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              p = 0;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              member = 0;

    member = _stp_db_trunk_group[trunk_id].memberBmp;
    for (p = 0 ; p < PLAT_MAX_PORT_NUM ; p++)
    {
        if ((member & BIT(p)) > 0)
        {
            req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, (p+1));
            rc = stp_db_update_msg(req_type, (void *)(&oper_cost), sizeof(UI32_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to send port %d cost %d to DB", p+1, oper_cost);
                break;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: stp_db_port_compute_oper_cost
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      port                     -  port number
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - auto_cost_support
 *      ptr_oper_cost            -  oper cost
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_db_port_oper_cost_update(
    const UI32_T    port,
    const UI32_T    oper_cost)
{
    DB_REQUEST_TYPE_T     req_type;
    MW_ERROR_NO_T         rc = MW_E_OK;

    if (TRUE == _stp_auto_cost_support)
    {
        if (PLAT_MAX_PORT_NUM < port)
        {
            return rstp_db_trunk_port_oper_cost_update(((port - 1) - PLAT_MAX_PORT_NUM), oper_cost);
        }

        req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_COST, port);
        rc = stp_db_update_msg(req_type, (void *)(&oper_cost), sizeof(UI32_T));
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to send port %d cost %d update data", port, oper_cost);
        }
    }

    return rc;
}

#endif

/* FUNCTION NAME: stp_db_get_port_admin_edge_config
 * PURPOSE:
 *      Get port admin edge config.
 *
 * INPUT:
 *      port             - port id
 *
 * OUTPUT:
 *      ptr_admin_edge  - config
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_get_port_admin_edge_config(
    UI32_T  port,
    UI8_T   *ptr_admin_edge)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_REQUEST_TYPE_T           req_type;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_ADMIN_EDGE, port_number);
    rc = stp_db_get_msg(req_type, ptr_admin_edge);

    return rc;
}

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: stp_db_port_bpdu_state_update
 * PURPOSE:
 *      Update port bpdu protect state.
 *
 * INPUT:
 *      port             - port id
 *      bpdu_state       - state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_bpdu_state_update(
    UI32_T      port,
    UI8_T       bpdu_state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T state;

    state = rstp_port_sec_state.bg_state;
    if (true == bpdu_state)
    {
        state |= bpdu_state << (port - 1);
    }
    else
    {
        state &= ~(1 << (port - 1));
    }

    req_type = stp_dbmsg_init_reqtype(STP_INFO, RSTP_PORT_BPDU_GUARD_STATE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)(&state), sizeof(state));
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to update port %d bpdu state %d", port, bpdu_state);
        return rc;
    }

    rstp_port_sec_state.bg_state = state;

    return rc;
}

/* FUNCTION NAME: stp_set_port_bpdu_down
 * PURPOSE:
 *      Setting port bpdu protect down.
 *
 * INPUT:
 *      port             - port id
 *      bpdu_state       - state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_set_port_bpdu_down(
    UI32_T  port,
    UI8_T   bpdu_state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI8_T               status = false;
    UI32_T              trunk_id, port_num;
    UI32_T              member;

    /* if trunk, down all ports */
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        member = _stp_db_trunk_group[trunk_id].memberBmp;
        for (port_num = 0; port_num < PLAT_MAX_PORT_NUM; port_num++)
        {
            if ((member & BIT(port_num)) > 0)
            {
                req_type = stp_dbmsg_init_reqtype(PORT_CFG_INFO, PORT_ADMIN_STATUS, (port_num + 1));
                rc = stp_db_update_msg(req_type, (void *)(&status), sizeof(status));
                if (MW_E_OK != rc)
                {
                    STP_LOG_ERR("Fail to update port %d admin down to db", (port_num + 1));
                    break;
                }
                rc = stp_db_port_bpdu_state_update((port_num + 1), bpdu_state);
                if (MW_E_OK != rc)
                {
                    break;
                }
            }
        }
    }
    else
    {
        req_type = stp_dbmsg_init_reqtype(PORT_CFG_INFO, PORT_ADMIN_STATUS, port);
        rc = stp_db_update_msg(req_type, (void *)(&status), sizeof(status));
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to update port %d admin down to db", port);
            return rc;
        }

        rc = stp_db_port_bpdu_state_update(port, bpdu_state);
    }

    return rc;
}
#endif

/* FUNCTION NAME: stp_db_update_port_block
 * PURPOSE:
 *      Update port block state.
 *
 * INPUT:
 *      port             - port id
 *      oper_block       - state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_update_port_block(
    const UI32_T    port,
    const BOOL_T    oper_block)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
#ifdef AIR_SUPPORT_RSTP
    DB_REQUEST_TYPE_T   req_type;
    UI8_T               block = (TRUE == oper_block) ? 1 : 0;

    _rstp_block_bmp = oper_block ? (_rstp_block_bmp | 1 << (port - 1)) : (_rstp_block_bmp & ~(1 << (port - 1)));

    if (PLAT_MAX_PORT_NUM < port)
    {
        return _stp_db_trunk_port_block(((port - 1) - PLAT_MAX_PORT_NUM), oper_block);
    }

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_BLOCK, port);
        rc = stp_db_update_msg(req_type, (void *)(&block), sizeof(UI8_T));
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Fail to send port %d block state %d to db", port, oper_block);
        }
    }
#endif

    return rc;
}

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_db_port_update_inconsistent
 * PURPOSE:
 *      Update port inconsistent state to db.
 *
 * INPUT:
 *      port           -  Port ID.
 *      state          -  inconsistent state.
 *      inc_type       -  root-inc/loop-inc.
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_db_port_update_inconsistent(
    const UI32_T port,
    const UI8_T  state,
    const UI8_T  inc_type)
{
    UI32_T              *ptr_state;
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req;
    UI8_T               field;
    UI32_T              trunk_id, port_number;

    if (RSTP_INCONSISTENT_ROOT == inc_type)
    {
        ptr_state = &rstp_port_sec_state.rg_state;
        field = RSTP_PORT_ROOT_GUARD_STATE;
    }
    else if (RSTP_INCONSISTENT_LOOP == inc_type)
    {
        ptr_state = &rstp_port_sec_state.lg_state;
        field = RSTP_PORT_LOOP_GUARD_STATE;
    }
    else
    {
        STP_LOG_WARN("Port %d update inconsistent state, type %d is invalid\n", inc_type);
        return MW_E_BAD_PARAMETER;
    }

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    /* check portid */
    if (0 == port_number)
    {
        return MW_E_BAD_PARAMETER;
    }

    if ((((*ptr_state) >> (port_number - 1)) & 0x1) == state)
    {
        return rc;
    }

    if (true == state)
    {
        (*ptr_state) |= state << (port_number - 1);
    }
    else
    {
        (*ptr_state) &= ~(1 << (port_number - 1));
    }

    req = stp_dbmsg_init_reqtype(STP_INFO, field, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req, (void *)(ptr_state), sizeof(UI32_T));

    return rc;
}

/* FUNCTION NAME: rstp_get_port_security_param
 * PURPOSE:
 *      Get security data
 *
 * INPUT:
 *      port           -  port number
 *
 * OUTPUT:
 *      sec_info       -  security info
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_get_port_security_param(
    UI32_T  port,
    DB_RSTP_PORT_SEC_T *sec_info)
{
    MW_ERROR_NO_T               rc = MW_E_OK;
    DB_REQUEST_TYPE_T           req_type;
    UI32_T                      port_number = 0;
    UI32_T                      trunk_id;
    DB_RSTP_PORT_SEC_T          info;

    port_number = port;
    if (port > PLAT_MAX_PORT_NUM)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        port_number = _stp_db_trunk_getLowerPort(trunk_id);
    }

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, DB_ALL_FIELDS, port_number);
        rc = stp_db_get_msg(req_type, &info);
        if (MW_E_OK == rc)
        {
            sec_info->bpdu_status = info.bpdu_status;
            sec_info->tc_status = info.tc_status;
            sec_info->tc_interval = info.tc_interval;
            sec_info->tc_threshold = info.tc_threshold;
            sec_info->root_status = info.root_status;
            sec_info->loop_status = info.loop_status;
        }
        else
        {
            STP_LOG_ERR("Get port %d security config failed!", port);
        }
    }

    return rc;
}

#endif

/* FUNCTION NAME: stp_db_clear_all_port_block
 * PURPOSE:
 *      Clear port block state.
 *
 * INPUT:
 *      port             - port id
 *      oper_block       - state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_clear_all_port_block(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
#ifdef AIR_SUPPORT_RSTP
    UI8_T               data = FALSE;

    if (0 != _rstp_block_bmp)
    {
        _rstp_block_bmp = 0;
        rc = stp_db_port_update_allEntries(STP_PORT_OPER_INFO, STP_PORT_OPER_BLOCK, &data);
    }
#endif
    return rc;
}

/* FUNCTION NAME: stp_db_clear_all_port_block
 * PURPOSE:
 *      Clear port block state.
 *
 * INPUT:
 *      port             - port id
 *      oper_block       - state
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_ERROR_NO_T
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_mcheck_update(
    UI32_T    port,
    UI8_T     perform)
{
    return _stp_db_port_mcheck_update(port, perform);
}

/* FUNCTION NAME: stp_db_port_mode_update
 * PURPOSE:
 *      Send to DB to update port mode
 *
 * INPUT:
 *      portMode       -  port mode bmp
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_db_port_mode_update(
    const UI32_T    portMode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_PORT_MODE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)(&portMode), sizeof(portMode));
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to send port stp mode %u to db", portMode);
    }

    return rc;
}

/* FUNCTION NAME: stp_db_subscribe
 * PURPOSE:
 *      Subscribe db msg for stp.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
void
stp_db_subscribe(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_REQUEST_TYPE_T req_type = {0};

    _stp_checkDbReady();

#ifdef AIR_SUPPORT_MSTP
    req_type = stp_dbmsg_init_reqtype(VLAN_CFG_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(VLAN_ENTRY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#endif

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORCE_VERSION, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_ENABLE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORWARD_DELAY, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_MAX_AGE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_TRANSMIT_HOLD_COUNT, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_BRIDGE_PRIORITY, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#ifdef AIR_SUPPORT_RSTP_SECURITY
    req_type = stp_dbmsg_init_reqtype(STP_INFO, RSTP_PORT_BPDU_GUARD_STATE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#endif
    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(TRUNK_PORT, TRUNK_MEMBERS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_DUPLEX, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_LACP_STATE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

#ifdef AIR_SUPPORT_STP_AUTO_COST
    req_type = stp_dbmsg_init_reqtype(PORT_OPER_INFO, PORT_OPER_SPEED, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_AUTO_COST_SUPPORT, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#endif

#ifdef AIR_SUPPORT_RSTP_SECURITY
    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_BPDU_GUARD_ENABLE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_ENABLE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_ROOT_GUARD_ENABLE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_LOOP_GUARD_ENABLE, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_INTERVAL, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_THRESHOLD, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(PORT_CFG_INFO, PORT_ADMIN_STATUS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#endif

#ifdef AIR_SUPPORT_MSTP
    req_type = stp_dbmsg_init_reqtype(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);

    req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc |= _stp_db_sendMsg(STP_MODULE_NAME, STP_QUEUE_NAME, M_SUBSCRIBE, req_type, NULL, 0, NULL);
#endif

    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Db subscribe failed, rc = %d\n", rc);
    }

    osapi_memset(_stp_db_trunk_group, 0, sizeof(STP_TRUNK_INFO_T) * MAX_TRUNK_NUM);
    return;
}

/* FUNCTION NAME: stp_db_queue_handle
 * PURPOSE:
 *      Handle db msg for stp.
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
stp_db_queue_handle(
    DB_MSG_T    *ptr_msg)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;

    do {
        rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
        if (MW_E_OK == rc)
        {
            STP_LOG_DBG("Db handle msg t_id %d, f_id %d, e_id %d\n", request.t_idx, request.f_idx, request.e_idx);
            rc = _stp_db_msg_handle(ptr_msg->method, &request, data_size, ptr_data);
        }
    } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));

    return MW_E_OK;
}
