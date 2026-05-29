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

/* FILE NAME:   stp.c
 * PURPOSE:
 *      stp task & timer handle.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "ethernetif.h"
#include <sys_mgmt.h>
#include "mw_msg.h"
#include "mw_utils.h"
#include "air_acl.h"
#include "air_l2.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "sfp_port.h"
#endif
#include "mw_rate_limit.h"
#include <syncd_api_stp.h>
#include "syncd_api_lag.h"
#include "timer.h"
#include "mw_acl.h"

#include "stp.h"
#ifdef AIR_SUPPORT_RSTP
#include "rstp-sys.h"
#endif

#ifdef AIR_SUPPORT_MSTP
#include "mstp_sys.h"
#endif
#include "stp_db.h"
#include "osapi_timer.h"
#ifdef AIR_SUPPORT_IGMP_SNP
#include "igmp_snoop_msg.h"
#endif


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
NET_FILTER_T        *stp_pkt_netif = NULL;
STP_SYS_CONTEXT_T   *ptr_poll_msg = NULL;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

static inline void
_stp_checkMacRcvReady(
    void);

/* STATIC VARIABLE DECLARATIONS
 */
static threadhandle_t       _stp_task_handle    = NULL;
static timehandle_t         _stp_time_handle    = NULL;
static msghandle_t          _stp_q_handle       = NULL;
static UI16_T               _bpdu_limit_acl_id[MAX_PORT_NUM];
#ifdef AIR_SUPPORT_STP_UNIFIED_SA
static UI8_T                _stp_unified_sa_support = FALSE;
#endif /* AIR_SUPPORT_RSTP_UNIFIED_SA */
#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
/* Enable/Disable the action to clear multicast group when topology change is happened. */
static UI8_T                _stp_clear_multicast_support = FALSE;
#endif /* AIR_SUPPORT_RSTP_CLEAR_MULTICAST */
static STP_FLUSH_MAC_T     *_ptr_stp_flush_data = NULL;
static STP_QUEUE_CONTEXT_T _stp_queue_ctx;
static StackType_t          _stp_task_stack[STP_STACK_SIZE] = {0};
static StaticTask_t         _stp_task_tcb = {0};
#ifdef STP_DEBUG
static UI32_T               _stp_stm_debug_pbm = 0;
#endif

/* LOCAL SUBPROMGRAM BODIES
*/
static inline void
_stp_checkMacRcvReady(
    void)
{
    /* Check mac_rcv is ready */
    while (MW_E_OK != mac_rcv_readyGet())
    {
        STP_LOG_DBG("Mac_rcv is not ready");
        osapi_delay(STP_READY_DELAY);
    }
}

static MW_ERROR_NO_T
_stp_queue_deinit(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    rc = osapi_msgDelete(STP_QUEUE_NAME);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Stp msg queue delete failed!");
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_stp_queue_init(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    if (MW_E_OK != (rc = osapi_msgCreateStatic(STP_QUEUE_NAME,
        STP_QUEUE_LENGTH, STP_MSG_SIZE,
        (UI8_T *)(_stp_queue_ctx.msgQueueStorage),
        &(_stp_queue_ctx.msgQueueStruct))))
    {
        STP_LOG_ERR("Create msg queue for %s failed!", STP_QUEUE_NAME);
    }

    if (MW_E_OK == rc)
    {
        _stp_q_handle = osapi_msgFindHandle(STP_QUEUE_NAME);
    }

    return rc;
}

static void
_stp_rx(
    struct pbuf *ptr_pbuf)
{
    UI32_T              port_number = 0;
    UI32_T              trunk_id;
    UI32_T              bpdu_size = 0;
    UI8_T               version = stp_db_getVersion();

    if (FALSE == stp_db_getStpGlobalState())
    {
        STP_LOG_DBG("Stp global state is disabled!");
        pbuf_free(ptr_pbuf);
        return;
    }

    port_number = ptr_pbuf->stag_hdr.rx_hdr.sp;
    stp_db_getTrunkID(port_number, &trunk_id);
    if (MAX_TRUNK_NUM > trunk_id)
    {
        port_number = PLAT_MAX_PORT_NUM + (1 + trunk_id);
    }
    bpdu_size = ptr_pbuf->ether_hdr.type - SIZEOF_LLC_HDR;

    STP_LOG_DBG("Port %d recv bpdu, bpdu size %d", port_number, bpdu_size);

    if (STP_VERSION_MSTP == version)
    {
#ifdef AIR_SUPPORT_MSTP
        mstp_pkt_rx(port_number, ptr_pbuf->payload, bpdu_size);
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rstp_pkt_rx(port_number, ptr_pbuf->payload, bpdu_size);
#endif
    }

    pbuf_free(ptr_pbuf);
}

static MW_ERROR_NO_T
_stp_pkt_deinit(
    void)
{
    MW_ERROR_NO_T  rc = MW_E_OK;

    if (stp_pkt_netif)
    {
        stp_pkt_netif->state = NET_FILTER_DEREGISTER;
        rc = osapi_netRegister(stp_pkt_netif);
        if (MW_E_OK == rc)
        {
            MW_FREE(stp_pkt_netif);
        }
        else
        {
            STP_LOG_WARN("Deregister net filter to mac driver failed!");
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_stp_pkt_init(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    rc = osapi_calloc(sizeof(NET_FILTER_T), STP_MODULE_NAME, (void **)&stp_pkt_netif);
    if (MW_E_OK == rc)
    {
        osapi_memset(stp_pkt_netif->name, 0, MSG_MAX_NAME_LEN);
        osapi_snprintf((C8_T*)stp_pkt_netif->name, MSG_MAX_NAME_LEN, "%s", (C8_T*)STP_QUEUE_NAME);
        stp_pkt_netif->protocol = PROTO_BPDU;
        stp_pkt_netif->state = NET_FILTER_REGISTER;
        stp_pkt_netif->handle = _stp_q_handle;
        rc = osapi_netRegister(stp_pkt_netif);
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Register net filter to mac driver failed!");
        }
    }
    else
    {
        STP_LOG_ERR("Allocate memory failed!");
    }

    return rc;
}

#ifdef AIR_SUPPORT_RSTP
static void
_rstp_update_flush_mac_data(
    void)
{
    UI32_T i;

    if (NULL == _ptr_stp_flush_data)
    {
        return;
    }

    if (STP_VERSION_MSTP == stp_db_getVersion())
    {
        return;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if (0 == _ptr_stp_flush_data[i].portInfo[0].is_flush)
        {
            continue;
        }

        if (1 == _ptr_stp_flush_data[i].portInfo[0].flush_time)
        {
            syncd_rstp_set_flush_flag(i+1, TRUE);
            STP_LOG_DBG("Port %d flush mac.", i+1);
            _ptr_stp_flush_data[i].portInfo[0].flush_time = 0;
            _ptr_stp_flush_data[i].portInfo[0].is_flush = 0;
        }
        else if (1 < _ptr_stp_flush_data[i].portInfo[0].flush_time)
        {
            _ptr_stp_flush_data[i].portInfo[0].flush_time--;
        }
    }
}
#endif

#ifdef AIR_SUPPORT_MSTP
static void
_mstp_update_flush_mac_data(
    void)
{
    UI32_T    i, port, num;
    UI16_T    vid[MAX_VLAN_ENTRY_NUM];

    if ((TRUE == stp_db_getStpGlobalState()) && (STP_VERSION_MSTP != stp_db_getVersion()))
    {
        return;
    }

    for (i = 0; i < MAX_MSTP_INSTANCE_NUM; i++)
    {
        for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
        {
            if (0 != _ptr_stp_flush_data[port-1].portInfo[i].is_flush)
            {
                mstp_get_vlanidByFid(i, vid, &num);
                if (0 == num)
                {
                    break;
                }

                syncd_mstp_set_flush_flag(vid, num);
                STP_LOG_DBG("Instance %d vlan id %d flush mac.", i, vid);
                break;
            }
        }
    }
    osapi_memset(_ptr_stp_flush_data, 0, sizeof(STP_FLUSH_MAC_T) * PLAT_MAX_PORT_NUM);
}
#endif

static void
_stp_timer(
    TimerHandle_t xTimer)
{
    STP_PROFILING_INIT();
    STP_PROFILING_START();

#ifdef AIR_SUPPORT_RSTP
    _rstp_update_flush_mac_data();
#endif

#ifdef AIR_SUPPORT_MSTP
    _mstp_update_flush_mac_data();
#endif

    if (NULL != ptr_poll_msg && FALSE == ptr_poll_msg->stp_tick)
    {
        ptr_poll_msg->stp_tick = TRUE;
    }
    else if (NULL != ptr_poll_msg && TRUE == ptr_poll_msg->stp_tick)
    {
        STP_LOG_DBG("Previous tick event handle is not completed.");
    }
    STP_PROFILING_END("timer callback");
}

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
/* FUNCTION NAME:   _rstp_set_task_priority
 * PURPOSE:
 *      Set the RSTP task priority and delay the calling task if delay input
 *      parameter is not 0.
 *
 * INPUT:
 *      task_priority            -- New task priority
 *      delay                    -- The delay period in ms
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
_rstp_set_task_priority(
    UI32_T task_priority,
    UI32_T delay)
{
    UI32_T task_priority_old = 0;
#ifdef STP_DEBUG
    static TickType_t tickstart = 0;
    TickType_t tickend = 0;
#endif

    if (NULL == _stp_task_handle)
    {
        return;
    }

    osapi_threadPriorityGet(_stp_task_handle, &task_priority_old);
    if (task_priority_old != task_priority)
    {
        osapi_threadPrioritySet(_stp_task_handle, task_priority);

        if (task_priority_old > task_priority)
        {
#ifdef STP_DEBUG
            tickend = osapi_sysTickGet();
            STP_LOG_DBG("Restore RSTP pri to %d tickstart:%d tickend:%d duration:%d ms\n", task_priority, tickstart, tickend, tickend - tickstart);
#endif
        }
        else
        {
#ifdef STP_DEBUG
            tickstart = osapi_sysTickGet();
            STP_LOG_DBG("Increase RSTP pri to %d tickstart:%d\n", task_priority, tickstart);
#endif
        }

        if (0 != delay)
        {
            /* Suspend the calling task for a period of time in ms. */
            osapi_delay(delay);
        }
    }
}
#endif

static void
_stp_tick_timers(
    void)
{
    if (STP_VERSION_MSTP == stp_db_getVersion())
    {
#ifdef AIR_SUPPORT_MSTP
        mstp_tick_timers_start();
#endif
    }
    else
    {
#ifdef AIR_SUPPORT_RSTP
        rstp_tick_timers_start();
#endif
    }
}

static MW_ERROR_NO_T
_stp_port_state_polling(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req;
    UI32_T              i;
    UI8_T               state[MAX_PORT_NUM];

    if ((NULL == ptr_poll_msg) || (FALSE == ptr_poll_msg->port_state_changed))
    {
        return rc;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        state[i] = ptr_poll_msg->ptr_state[i].state[0];
    }

    req = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, STP_PORT_OPER_STATE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req, (void *)state, sizeof(UI8_T) * PLAT_MAX_PORT_NUM);
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to poll all port state to DB");
        return rc;
    }
#ifdef AIR_SUPPORT_MSTP
    req = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, MSTP_INSTANCE_PORT_OPER_STATE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req, (void *)ptr_poll_msg->ptr_state, (sizeof(UI8_T) * PLAT_MAX_PORT_NUM * MSTP_INSTANCE_MAX_NUM));
    if (MW_E_OK != rc)
    {
        STP_LOG_ERR("Fail to poll all instance port state to DB");
        return rc;
    }
#endif

    ptr_poll_msg->port_state_changed = FALSE;

    return rc;
}


static void
_stp_task(
    void *ptr_args)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    void                    *ptr_msg = NULL;
    struct pbuf             *ptr_pbuf;
    NET_MSG_T               *ptr_data = NULL;
    UI32_T                  start_tick, end_tick, dur_tick;

    stp_db_subscribe();

    _stp_checkMacRcvReady();

#ifdef AIR_SUPPORT_RSTP
    rstp_data_init();
#endif

#ifdef AIR_SUPPORT_MSTP
    mstp_data_init();
#endif

    start_tick = osapi_sysTickGet();

    /* main process */
    while (TRUE)
    {
        if ((TRUE == stp_db_getStpGlobalState()) && (NULL != ptr_poll_msg) && (TRUE == ptr_poll_msg->stp_tick))
        {
            STP_PROFILING_INIT();
            STP_PROFILING_START();
            _stp_tick_timers();
            STP_PROFILING_END("timer expire handle");
            ptr_poll_msg->stp_tick = FALSE;
        }

        rc = osapi_msgRecv(STP_QUEUE_NAME, (UI8_T **)&ptr_data, 0, STP_TASK_DELAY);
        if (MW_E_OK == rc)
        {
            STP_LOG_DBG("Received message handle with id %u", ptr_data->msg_id);
            if (MW_MSG_ID_ETHERNET_PBUF == ptr_data->msg_id)
            {
                /* Recv: mac_rcv */
                ptr_pbuf = (struct pbuf *)(ptr_data->ptr_pbuf);
                MW_FREE(ptr_data);

                STP_PROFILING_INIT();
                STP_PROFILING_START();
                _stp_rx(ptr_pbuf);
                STP_PROFILING_END("packet handle");
                STP_LOG_DBG("Bpdu handle over.");
            }
            else if (MW_MSG_ID_DB == ptr_data->msg_id)
            {
                /* Recv db notification */
                ptr_msg = (DB_MSG_T *)ptr_data;
                stp_db_queue_handle(ptr_msg);
                STP_LOG_DBG("Db msg handle over.");
                MW_FREE(ptr_msg);
            }
            else
            {
                STP_LOG_WARN("Unknown msg_id=%u\n", ptr_data->msg_id);
                MW_FREE(ptr_data);
            }
        }

        end_tick = osapi_sysTickGet();
        if (end_tick >= start_tick)
        {
            dur_tick = end_tick - start_tick;
        }
        else
        {
            dur_tick = 0xFFFFFFFF - start_tick + end_tick + 1;
        }

        if (dur_tick > STP_POLLING_INTERVAL)
        {
            start_tick = osapi_sysTickGet();
            _stp_port_state_polling();
        }
    }

    STP_LOG_ERR("Never reach here!");
}

static MW_ERROR_NO_T
_stp_task_deinit(
    void)
{
    if (_stp_task_handle)
    {
        osapi_processDelete(_stp_task_handle);
        _stp_task_handle = NULL;
    }

    MW_FREE(_ptr_stp_flush_data);
    MW_FREE(ptr_poll_msg->ptr_state);
    MW_FREE(ptr_poll_msg);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_stp_task_init(
    void)
{
    MW_ERROR_NO_T   rc = MW_E_OK;

    rc = osapi_malloc(sizeof(STP_SYS_CONTEXT_T), STP_MODULE_NAME, (void**)&ptr_poll_msg);
    if (MW_E_OK == rc)
    {
        osapi_memset(ptr_poll_msg, 0, sizeof(STP_SYS_CONTEXT_T));
        rc = osapi_malloc(sizeof(STP_PORT_STATE_T) * PLAT_MAX_PORT_NUM, STP_MODULE_NAME, (void**)&ptr_poll_msg->ptr_state);
        if (MW_E_OK != rc)
        {
            STP_LOG_ERR("Allocate memory for polling state failed!");
            return rc;
        }
        osapi_memset(ptr_poll_msg->ptr_state, STP_FORWARDING, sizeof(STP_PORT_STATE_T) * PLAT_MAX_PORT_NUM);
    }
    else
    {
        STP_LOG_ERR("Allocate memory for polling msg failed!");
        return rc;
    }

    rc = osapi_malloc(sizeof(STP_FLUSH_MAC_T) * PLAT_MAX_PORT_NUM, STP_MODULE_NAME, (void**)&_ptr_stp_flush_data);
    if (MW_E_OK == rc)
    {
        osapi_memset(_ptr_stp_flush_data, 0, (sizeof(STP_FLUSH_MAC_T) * PLAT_MAX_PORT_NUM));
    }
    else
    {
        STP_LOG_ERR("[STP]: allocate memory for flush mac failed!");
        osapi_free(ptr_poll_msg->ptr_state);
        osapi_free(ptr_poll_msg);
        ptr_poll_msg = NULL;
        return rc;
    }

    osapi_memset(_bpdu_limit_acl_id, 0xff, sizeof(_bpdu_limit_acl_id));

    rc = osapi_threadCreateStatic(STP_MODULE_NAME,
                                    STP_STACK_SIZE,
                                    MW_TASK_PRIORITY_STP_NORMAL,
                                    _stp_task,
                                    NULL,
                                    _stp_task_stack,
                                    &_stp_task_tcb,
                                    &_stp_task_handle);
    if (MW_E_OK == rc)
    {
        /* Create timer */
        rc = osapi_timerCreate(STP_TIMER_NAME, _stp_timer, TRUE, STP_TIMER_PERIOD, NULL, &_stp_time_handle);
        if (MW_E_OK == rc)
        {
            rc = osapi_timerStart(_stp_time_handle);
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Start stp timer failed!");
                osapi_processDelete(_stp_task_handle);
            }
        }
        else
        {
            STP_LOG_ERR("Create stp timer failed!");
            osapi_processDelete(_stp_task_handle);
        }
    }
    else
    {
        STP_LOG_ERR("Create thread for STP failed!");
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_speed_enum_to_integer
 * PURPOSE:
 *      Transfer speed
 *
 * INPUT:
 *      speed_enum          -  port speed value
 *      ptr_speed_int       -  speed num
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
stp_speed_enum_to_integer(
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
    return;
}
#endif

/* FUNCTION NAME: stp_port_instance_state_update
 * PURPOSE:
 *      Update port instance state
 *
 * INPUT:
 *      port               -  port id
 *      state              -  port state
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
stp_port_state_data_update(
    UI32_T port,
    UI8_T  state)
{
    UI32_T              member = 0;
    UI32_T              p = 0, trunk_id;

    if (NULL != ptr_poll_msg)
    {
        if (PLAT_MAX_PORT_NUM < port)
        {
            trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
            member = stp_db_trunk_getMemberBmp(trunk_id);
            for (p = 0 ; p < PLAT_MAX_PORT_NUM; p++)
            {
                if ((member & BIT(p)) != 0)
                {
                    if (ptr_poll_msg->ptr_state[p].state[0] != state)
                    {
                        ptr_poll_msg->port_state_changed = TRUE;
                        ptr_poll_msg->ptr_state[p].state[0] = state;
                    }
                }
            }
            return;
        }

        if (ptr_poll_msg->ptr_state[port-1].state[0] != state)
        {
            ptr_poll_msg->port_state_changed = TRUE;
            ptr_poll_msg->ptr_state[port-1].state[0] = state;
        }
    }
}

/* FUNCTION NAME: mstp_port_instance_state_update
 * PURPOSE:
 *      Update port instance state
 *
 * INPUT:
 *      instance           -  instance
 *      port               -  port id
 *      state              -  port state
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
mstp_port_instance_state_update(
    UI32_T instance,
    UI32_T port,
    UI8_T  state)
{
    UI32_T              member = 0;
    UI32_T              p = 0, trunk_id;

    if ((NULL == ptr_poll_msg) || (0 == port))
    {
        return;
    }
    if (PLAT_MAX_PORT_NUM < port)
    {
        trunk_id = STP_PORT_NUMBER_TO_TRUNK_ID(port);
        member = stp_db_trunk_getMemberBmp(trunk_id);
        for (p = 0 ; p < PLAT_MAX_PORT_NUM; p++)
        {
            if ((member & BIT(p)) != 0)
            {
                if (ptr_poll_msg->ptr_state[p].state[instance] != state)
                {
                    ptr_poll_msg->port_state_changed = TRUE;
                    ptr_poll_msg->ptr_state[p].state[instance] = state;
                }
            }
        }
        return;
    }

    if (ptr_poll_msg->ptr_state[port-1].state[instance] != state)
    {
        ptr_poll_msg->port_state_changed = TRUE;
        ptr_poll_msg->ptr_state[port-1].state[instance] = state;
    }
}

/* FUNCTION NAME: stp_bpdu_rate_limit_ctrl
 * PURPOSE:
 *      Setting bpdu rate limit
 *
 * INPUT:
 *      enable           -  TRUE/FALSE
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_bpdu_rate_limit_ctrl(
    UI8_T mode)
{
    MW_ERROR_NO_T               mw_rc = MW_E_OK;
    AIR_ERROR_NO_T              air_rc = AIR_E_OK;
    AIR_ACL_RULE_T              acl_rule;
    AIR_ACL_ACTION_T            action;
    UI32_T                      unit = 0, rule_id = 0, attack_id = 0;
    const struct eth_addr       pkt_da = IEEE_802_RESERVE_00;
    UI8_T                       port;

    for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
    {
        if(MW_E_OK == mw_acl_mutex_take())
        {
            if (TRUE == mode)
            {
                if ((MW_E_OK == MW_ACL_GET_AVAILABLERULE(&rule_id, &acl_rule, unit)) &&
                    (MW_E_OK == mw_rate_limit_lp_stp_port_attack_id_get(unit, port, MW_PORT_RATE_LIMIT_USER_RSTP, &attack_id)) &&
                    (MW_ACL_ID_INVALID != rule_id) && (MW_ATTACK_ID_INVALID != attack_id))
                {
                    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
                    acl_rule.rule_en = TRUE;
                    acl_rule.end = TRUE;
                    AIR_PORT_ADD(acl_rule.portmap, port);

                    osapi_memcpy(acl_rule.key.dmac, pkt_da.addr, sizeof(UI8_T) * 6);
                    acl_rule.mask.dmac=0x3f;
                    acl_rule.field_valid = (1 << AIR_ACL_DMAC_KEY);

                    air_rc = air_acl_setRule(unit, rule_id, &acl_rule);
                    if (AIR_E_OK == air_rc)
                    {
                        /* keep rule ID */
                        _bpdu_limit_acl_id[port-1] = rule_id;
                    }
                    else
                    {
                        STP_LOG_ERR("Add BPDU acl rule failed, rc=%u", air_rc);
                    }

                    if (AIR_E_OK == air_rc)
                    {
                        action.attack_rate_id = attack_id;
                        action.field_valid = (1U << AIR_ACL_ATTACK);
                        air_rc = air_acl_setAction(unit, rule_id, &action);
                        if (AIR_E_OK != air_rc)
                        {
                            STP_LOG_ERR("Add BPDU acl action failed, rc=%u", air_rc);
                        }
                    }
                }
            }
            else
            {
                air_acl_delAction(unit, _bpdu_limit_acl_id[port-1]);
                air_acl_delRule(unit, _bpdu_limit_acl_id[port-1]);
                mw_rate_limit_lp_stp_port_attack_clear(unit, port);
                _bpdu_limit_acl_id[port-1] = MW_ACL_ID_INVALID;
            }
            mw_acl_mutex_release();

            if (AIR_E_OK != air_rc)
            {
                mw_rc = MW_E_OP_INCOMPLETE;
                break;
            }
        }
    }

    return mw_rc;
}


/* FUNCTION NAME: stp_fdb_update_mac_entry_timer_by_port
 * PURPOSE:
 *      Update timer of MAC entry learned on specified port
 *
 * INPUT:
 *      port           -  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_fdb_update_mac_entry_timer_by_port(
    UI32_T  port,
    UI16_T  timer)
{
    MW_ERROR_NO_T   rc = 0;
    UI32_T          port_number = 0;

    port_number = port;
    if (PLAT_MAX_PORT_NUM < port)
    {
        port_number = stp_db_trunk_getLowerPort(STP_PORT_NUMBER_TO_TRUNK_ID(port));
    }

    if (NULL != _ptr_stp_flush_data)
    {
        _ptr_stp_flush_data[port_number-1].portInfo[0].is_flush = 1;
        _ptr_stp_flush_data[port_number-1].portInfo[0].flush_time = (timer & STP_FLUSH_MAC_TIMER_MASK);
        STP_LOG_DBG("Port %d flush mac, time %d", port_number, _ptr_stp_flush_data[port_number-1].portInfo[0].flush_time);
    }

    return rc;
}

/* FUNCTION NAME: mstp_setInstancePortFlush
 * PURPOSE:
 *      flush instance port mac table
 *
 * INPUT:
 *      port             - port Id
 *      fid              - instance idx
 *      ageTime          - mac flush age time
 *
 * OUTPUT:
 *      none
 *
 * RETURN:
 *      none
 *
 * NOTES:
 *
 */
void
mstp_setInstancePortFlush(
    unsigned int    port,
    unsigned short  fid,
    unsigned char   ageTime)
{
    UI32_T        port_number = 0;

    port_number = port;
    if (PLAT_MAX_PORT_NUM < port)
    {
        port_number = stp_db_trunk_getLowerPort(STP_PORT_NUMBER_TO_TRUNK_ID(port));
    }

    if (NULL != _ptr_stp_flush_data)
    {
        _ptr_stp_flush_data[port_number-1].portInfo[fid].is_flush = 1;
        _ptr_stp_flush_data[port_number-1].portInfo[fid].flush_time = (ageTime & STP_FLUSH_MAC_TIMER_MASK);
        STP_LOG_DBG("Port %d fid %d flush mac", port_number, fid);
    }
}

/* FUNCTION NAME: stp_sys_deinit
 * PURPOSE:
 *      Deinit STP function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_sys_deinit(
    void)
{
    _stp_task_deinit();
    _stp_pkt_deinit();
    _stp_queue_deinit();

    return MW_E_OK;
}

/* FUNCTION NAME: stp_sys_init
 * PURPOSE:
 *      Init STP function
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_sys_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_LOG_INIT_PRINTF("Initializing Spanning tree...\n");

    if (MW_E_OK == rc)
    {
        if (MW_E_OK == (rc = _stp_queue_init()))
        {
            STP_LOG_DBG("[STP]: stp queue init OK");
        }
    }

    if (MW_E_OK == rc)
    {
        if (MW_E_OK == (rc = _stp_pkt_init()))
        {
            STP_LOG_DBG("[STP]: packet init OK");
        }
    }

    if (MW_E_OK == rc)
    {
        if (MW_E_OK == (rc = _stp_task_init()))
        {
            STP_LOG_DBG("[STP]: task init OK");
        }
    }

    if (MW_E_OK != rc)
    {
        stp_sys_deinit();
        rc = MW_E_NOT_INITED;
    }
    return rc;
}

/* FUNCTION NAME: stp_cmd_setEnable
 * PURPOSE:
 *      Set global state
 *
 * INPUT:
 *      enable         -  0: disable; 1: enable
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
stp_cmd_setEnable(
    const UI8_T enable)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_ENABLE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&enable, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_setForceVersion
 * PURPOSE:
 *      Set force version
 *
 * INPUT:
 *      force_version  -  0: STP; 2: RSTP; 3: MSTP
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
stp_cmd_setForceVersion(
    const UI32_T force_version)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORCE_VERSION, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&force_version, sizeof(UI32_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_setForwardDelay
 * PURPOSE:
 *      Set forward delay
 *
 * INPUT:
 *      forward_delay  -  forward delay 4~30 sec
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
stp_cmd_setForwardDelay(
    const UI16_T forward_delay)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_FORWARD_DELAY, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&forward_delay, sizeof(UI16_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_getForwardDelay
 * PURPOSE:
 *      Get forward delay
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_fwd_delay  -  forward_delay
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
stp_cmd_getForwardDelay(
    UI16_T  *ptr_fwd_delay)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    rc = stp_db_get_forward_delay_config(ptr_fwd_delay);

    return rc;
}

/* FUNCTION NAME: stp_cmd_setMaxAge
 * PURPOSE:
 *      Set max age
 *
 * INPUT:
 *      max_age        -  max age, 6~40
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
stp_cmd_setMaxAge(
    const UI16_T max_age)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_MAX_AGE, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&max_age, sizeof(UI16_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_getMaxAge
 * PURPOSE:
 *      Get max age
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_max_age    -  max age
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
stp_cmd_getMaxAge(
    UI16_T  *ptr_max_age)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    rc = stp_db_get_max_age_config(ptr_max_age);

    return rc;
}

/* FUNCTION NAME: stp_cmd_setTransmitHoldCount
 * PURPOSE:
 *      Set transmit hold count
 *
 * INPUT:
 *      txHoldCnt      -  transmit hold count, 1~10
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
stp_cmd_setTransmitHoldCount(
    const UI16_T txHoldCnt)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_TRANSMIT_HOLD_COUNT, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&txHoldCnt, sizeof(UI16_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_setBridgePriority
 * PURPOSE:
 *      Set bridge priority
 *
 * INPUT:
 *      priority       -  priority, 0~61440 in step 4096
 *      instance       -  4bits-->fid, 12bits-->instance
 *      exist_ins      -  instance is valid
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
stp_cmd_setBridgePriority(
    const UI16_T priority,
    const UI16_T instance,
    const BOOL_T exist_ins)
{
    MW_ERROR_NO_T             rc = MW_E_OK;
    DB_REQUEST_TYPE_T         req_type;
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_TABLE_T  mstp_info = {{0},{0},{0}};
    UI32_T                    fid = MAX_MSTP_INSTANCE_NUM;
#endif

#ifdef AIR_SUPPORT_MSTP
    if (TRUE == exist_ins)
    {
        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &mstp_info);
        if (MW_E_OK == rc)
        {
            for(fid = 0; fid < MAX_MSTP_INSTANCE_NUM; fid++)
            {
                if (instance == MSTP_INSTANCE_GET_ID(mstp_info.instance[fid]))
                {
                    break;
                }
            }
        }

        if (MAX_MSTP_INSTANCE_NUM == fid)
        {
            MW_CMD_OUTPUT("Instance is not exsit\n");
            return MW_E_BAD_PARAMETER;
        }

        if (0 == instance)
        {
            /*CIST*/
            req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_BRIDGE_PRIORITY, DB_ALL_ENTRIES);
            rc = stp_db_update_msg(req_type, (void *)&priority, sizeof(UI16_T));
        }

        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, MSTP_INSTANCE_PRIORITY, (fid+1));
        rc |= stp_db_update_msg(req_type, (void *)&priority, sizeof(UI16_T));
    }
    else
#endif
    {
        (void)exist_ins;
        if (0 != instance)
        {
            return MW_E_BAD_PARAMETER;
        }
        req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_BRIDGE_PRIORITY, DB_ALL_ENTRIES);
        rc = stp_db_update_msg(req_type, (void *)&priority, sizeof(UI16_T));
    }

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortPriority
 * PURPOSE:
 *      Set port priority
 *
 * INPUT:
 *      port           -  port number
 *      priority       -  priority, 0~240 in step 16
 *      instance       -  4bits-->fid, 12bits-->instance
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
stp_cmd_setPortPriority(
    const UI32_T    port,
    const UI8_T     priority,
    const UI16_T    instance,
    const BOOL_T    exist_ins)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_TABLE_T  mstp_info = {{0},{0},{0}};
    UI32_T                    fid = MAX_MSTP_INSTANCE_NUM;
    UI8_T                     ppri[MAX_MSTP_INSTANCE_NUM] = {0};
#endif

#ifdef AIR_SUPPORT_MSTP
    if (TRUE == exist_ins)
    {
        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &mstp_info);
        if (MW_E_OK == rc)
        {
            for(fid = 0; fid < MAX_MSTP_INSTANCE_NUM; fid++)
            {
                if (instance == MSTP_INSTANCE_GET_ID(mstp_info.instance[fid]))
                {
                    break;
                }
            }
        }

        if (MAX_MSTP_INSTANCE_NUM == fid)
        {
            MW_CMD_OUTPUT("Instance is not exsit\n");
            return MW_E_BAD_PARAMETER;
        }

        if (0 == instance)
        {
            /*CIST*/
            req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_PRIORITY, port);
            rc = stp_db_update_msg(req_type, (void *)&priority, sizeof(UI8_T));
            if (MW_E_OK != rc)
            {
                STP_LOG_ERR("Fail to update data");
            }
        }
        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_PRIORITY, port);
        rc = stp_db_get_msg(req_type, ppri);
        if (MW_E_OK == rc)
        {
            ppri[fid] = priority;
            rc = stp_db_update_msg(req_type, (void *)ppri, sizeof(UI8_T) * MAX_MSTP_INSTANCE_NUM);
        }
    }
    else
#endif
    {
        (void)exist_ins;
        if (0 != instance)
        {
            return MW_E_BAD_PARAMETER;
        }
        req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_PRIORITY, port);
        rc = stp_db_update_msg(req_type, (void *)&priority, sizeof(UI8_T));
    }

    return rc;
}

/* FUNCTION NAME: stp_cmd_setPortCost
 * PURPOSE:
 *      Set port cost
 *
 * INPUT:
 *      port           -  port number
 *      cost           -  cost, 1~200000000
 *      instance       -  4bits-->fid, 12bits-->instance
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
stp_cmd_setPortCost(
    const UI32_T    port,
    const UI32_T    cost,
    const UI16_T    instance,
    const BOOL_T    exist_ins)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_TABLE_T  mstp_info = {{0},{0},{0}};
    UI32_T                    fid = MAX_MSTP_INSTANCE_NUM;
    UI32_T                    pcos[MAX_MSTP_INSTANCE_NUM] = {0};
#endif

#ifdef AIR_SUPPORT_MSTP
    if (TRUE == exist_ins)
    {
        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &mstp_info);
        if (MW_E_OK == rc)
        {
            for(fid = 0; fid < MAX_MSTP_INSTANCE_NUM; fid++)
            {
                if (instance == MSTP_INSTANCE_GET_ID(mstp_info.instance[fid]))
                {
                    break;
                }
            }
        }

        if (MAX_MSTP_INSTANCE_NUM == fid)
        {
            MW_CMD_OUTPUT("Instance is not exsit\n");
            return MW_E_BAD_PARAMETER;
        }

        if (0 == instance)
        {
            /*CIST*/
            req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_COST, port);
            rc = stp_db_update_msg(req_type, (void *)&cost, sizeof(UI32_T));
        }

        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, MSTP_INSTANCE_PORT_COST, port);
        rc = stp_db_get_msg(req_type, pcos);
        if (MW_E_OK == rc)
        {
            pcos[fid] = cost;
            rc = stp_db_update_msg(req_type, (void *)pcos, sizeof(UI32_T) * MAX_MSTP_INSTANCE_NUM);
        }
    }
    else
#endif
    {
        (void)exist_ins;
        if (0 != instance)
        {
            return MW_E_BAD_PARAMETER;
        }
        req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_COST, port);
        rc = stp_db_update_msg(req_type, (void *)&cost, sizeof(UI32_T));
    }

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortAdminEdge
 * PURPOSE:
 *      Set port admin edge
 *
 * INPUT:
 *      port           -  port number
 *      enable         -  0: disable; 1: enable
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
stp_cmd_setPortAdminEdge(
    const UI32_T    port,
    const UI8_T     admin_edge)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_ADMIN_EDGE, port);
    rc = stp_db_update_msg(req_type, (void *)&admin_edge, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: stp_cmd_setPortMCheck
 * PURPOSE:
 *      Set port mcheck
 *
 * INPUT:
 *      port           -  port number
 *      perform        -  0: do not trigger mcheck; 1: trigger mcheck
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
stp_cmd_setPortMCheck(
    const UI32_T    port,
    const UI8_T     perform)
{
    return stp_db_port_mcheck_update(port, perform);
}

/* FUNCTION NAME: stp_cmd_setPortEnableStatus
 * PURPOSE:
 *      Set port enable status
 *
 * INPUT:
 *      port           -  port number
 *      status         -  enable, disable
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
stp_cmd_setPortEnableStatus(
    const UI32_T    port,
    const UI8_T     status)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_STATUS, port);
    rc = stp_db_update_msg(req_type, (void *)&status, sizeof(UI8_T));

    return rc;
}

#ifdef AIR_SUPPORT_STP_AUTO_COST
/* FUNCTION NAME: stp_cmd_setPortAutoCostMode
 * PURPOSE:
 *      Set port auto cost mode
 *
 * INPUT:
 *      port           -  port number
 *      mode           -  auto cost mode
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
stp_cmd_setPortAutoCostMode(
    const UI32_T    port,
    const UI8_T     mode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    if (TRUE == stp_db_getAutoCostState())
    {
        req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_AUTO_COST_ENABLE, port);
        rc = stp_db_update_msg(req_type, (void *)&mode, sizeof(UI8_T));
    }
    else
    {
        rc = MW_E_OP_INVALID;
    }

    return rc;
}

/* FUNCTION NAME: stp_cmd_setAutoCostSupportMode
 * PURPOSE:
 *      Set port auto cost support mode
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setAutoCostSupportMode(
    const UI8_T     mode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(STP_INFO, STP_AUTO_COST_SUPPORT, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&mode, sizeof(UI8_T));

    return rc;
}
#endif /* AIR_SUPPORT_STP_AUTO_COST */

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
/* FUNCTION NAME: stp_cmd_setUnifiedSASupportMode
 * PURPOSE:
 *      Set unified SA of BPDU support mode
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setUnifiedSASupportMode(
    const UI8_T     mode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    _stp_unified_sa_support = mode;

    return rc;
}
#endif /* AIR_SUPPORT_STP_UNIFIED_SA */

#ifdef AIR_SUPPORT_STP_CLEAR_MULTICAST
/* FUNCTION NAME: stp_cmd_setClearMulticastSupportMode
 * PURPOSE:
 *      Set support mode to clear multicast group when topology change is happended.
 *
 * INPUT:
 *      mode           -  auto cost mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OP_INVALID
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_cmd_setClearMulticastSupportMode(
    const UI8_T     mode)
{
    MW_ERROR_NO_T       rc = MW_E_OK;

    _stp_clear_multicast_support = mode;

    return rc;
}

/* FUNCTION NAME: stp_remove_multicast_group
 * PURPOSE:
 *      Remove multicast group with specified port
 *
 * INPUT:
 *      port           -  port number
 *      timer          -  new timer for the MAC entry
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_remove_multicast_group(
    UI32_T  port)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
#ifdef AIR_SUPPORT_IGMP_SNP
    uint32_t        port_number = 0;
#endif /* AIR_SUPPORT_IGMP_SNP */

    if (TRUE == _stp_clear_multicast_support)
    {
        STP_LOG_DBG("Port %d remove multicast group", port);
#ifdef AIR_SUPPORT_IGMP_SNP
        port_number = port;
        if (PLAT_MAX_PORT_NUM < port)
        {
            port_number = stp_db_trunk_getLowerPort(STP_PORT_NUMBER_TO_TRUNK_ID(port));
        }
        IGMP_SNP_MSG_CLEAR_ENTRY_T clear_entry;

        osapi_memset(&clear_entry, 0, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));
        clear_entry.type = IGMP_SNP_ENTRY_FLUSH_TYPE_PORT;
        MW_PORT_ADD(clear_entry.portbmp, port_number);
        igmp_snp_msg_clearEntry((const IGMP_SNP_MSG_CLEAR_ENTRY_T *)&clear_entry);
#endif /* AIR_SUPPORT_IGMP_SNP */
    }

    return rc;
}
#endif /* AIR_SUPPORT_RSTP_CLEAR_MULTICAST*/

#ifdef AIR_SUPPORT_STP_UNIFIED_SA
/* FUNCTION NAME: stp_get_unified_sa_support
 * PURPOSE:
 *      compute oper cost
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ptr_auto_cost_support    - ptr_support
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
stp_get_unified_sa_support(
    UI8_T   *ptr_support)
{
    *ptr_support = _stp_unified_sa_support;

    return MW_E_OK;
}
#endif

#ifdef AIR_SUPPORT_RSTP_SECURITY
/* FUNCTION NAME: rstp_cmd_setPortBpduGuardState
 * PURPOSE:
 *      Set port bpdu guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  bpdu guard enable state
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
rstp_cmd_setPortBpduGuardState(
    const UI32_T    port,
    const UI8_T     state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_BPDU_GUARD_ENABLE, port);
    rc = stp_db_update_msg(req_type, (void *)&state, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortTcGuardState
 * PURPOSE:
 *      Set port tc guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  tc guard enable state
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
rstp_cmd_setPortTcGuardState(
    const UI32_T    port,
    const UI8_T     state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_ENABLE, port);
    rc = stp_db_update_msg(req_type, (void *)&state, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortTcGuardInterval
 * PURPOSE:
 *      Set port tc guard interval
 *
 * INPUT:
 *      port           -  port number
 *      interval       -  tc guard interval
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
rstp_cmd_setPortTcGuardInterval(
    const UI32_T    port,
    const UI16_T     interval)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_INTERVAL, port);
    rc = stp_db_update_msg(req_type, (void *)&interval, sizeof(UI16_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortTcGuardThreshold
 * PURPOSE:
 *      Set port tc guard threshold
 *
 * INPUT:
 *      port           -  port number
 *      threshold       -  tc guard threshold
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
rstp_cmd_setPortTcGuardThreshold(
    const UI32_T    port,
    const UI16_T     threshold)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_TC_GUARD_THRESHOLD, port);
    rc = stp_db_update_msg(req_type, (void *)&threshold, sizeof(UI16_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortRootGuardState
 * PURPOSE:
 *      Set port root guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  root guard enable state
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
rstp_cmd_setPortRootGuardState(
    const UI32_T    port,
    const UI8_T     state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_ROOT_GUARD_ENABLE, port);
    rc = stp_db_update_msg(req_type, (void *)&state, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_setPortLoopGuardState
 * PURPOSE:
 *      Set port loop guard state
 *
 * INPUT:
 *      port           -  port number
 *      state          -  loop guard enable state
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
rstp_cmd_setPortLoopGuardState(
    const UI32_T    port,
    const UI8_T     state)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;

    req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, RSTP_PORT_SEC_LOOP_GUARD_ENABLE, port);
    rc = stp_db_update_msg(req_type, (void *)&state, sizeof(UI8_T));

    return rc;
}

/* FUNCTION NAME: rstp_cmd_showSecInfo
 * PURPOSE:
 *      Show security info
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
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_cmd_showSecInfo(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI32_T              port = 0;
    UI32_T              trunk_id = 0;
    DB_RSTP_PORT_SEC_T  port_info_list[PLAT_MAX_PORT_NUM];
    UI32_T              member;

    if (MW_E_OK == rc)
    {
        for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
        {
            req_type = stp_dbmsg_init_reqtype(RSTP_PORT_SECURITY, DB_ALL_FIELDS, port);
            rc = stp_db_get_msg(req_type, &port_info_list[port-1]);
            if (MW_E_OK != rc)
            {
                break;
            }
        }

        if (MW_E_OK == rc)
        {
            MW_CMD_OUTPUT("%5s  %15s  %15s  %15s  %15s  %12s  %12s\n", "Port", "Loop Protect", "Root Protect", "BPDU Guard", "TC Guard", "inteval", "threshold");
            MW_CMD_OUTPUT("-----------------------------------------------------------------------------------------------------\n");

            for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
            {
                stp_db_getTrunkID(port, &trunk_id);
                if (trunk_id < MAX_TRUNK_NUM)
                {
                    continue;
                }

                MW_CMD_OUTPUT("%5u  %15s  %15s  %15s  %15s  %12d  %12d\n",
                    port,
                    port_info_list[port-1].loop_status ? "Enable" : "Disable",
                    port_info_list[port-1].root_status ? "Enable" : "Disable",
                    port_info_list[port-1].bpdu_status ? "Enable" : "Disable",
                    port_info_list[port-1].tc_status ? "Enable" : "Disable",
                    port_info_list[port-1].tc_interval,
                    port_info_list[port-1].tc_threshold);
            }

            MW_CMD_OUTPUT("\n%5s  %15s  %15s  %15s  %15s  %12s  %12s\n", "Trunk", "Loop Protect", "Root Protect", "BPDU Guard", "TC Guard", "inteval", "threshold");
            MW_CMD_OUTPUT("-----------------------------------------------------------------------------------------------------\n");
            for (trunk_id = 0 ; trunk_id < MAX_TRUNK_NUM ; trunk_id++)
            {
                member = stp_db_trunk_getMemberBmp(trunk_id);
                if (0 != member)
                {
                    port = stp_db_trunk_getLowerPort(trunk_id);
                    if (0 == port)
                    {
                        continue;
                    }

                    MW_CMD_OUTPUT("%5u  %15s  %15s  %15s  %15s  %12d  %12d\n",
                        trunk_id+1,
                        port_info_list[port-1].loop_status ? "Enable" : "Disable",
                        port_info_list[port-1].root_status ? "Enable" : "Disable",
                        port_info_list[port-1].bpdu_status ? "Enable" : "Disable",
                        port_info_list[port-1].tc_status ? "Enable" : "Disable",
                        port_info_list[port-1].tc_interval,
                        port_info_list[port-1].tc_threshold);
                }
            }
        }
    }

    return rc;
}

#endif

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME: mstp_cmd_setRegionConfig
 * PURPOSE:
 *      Set region name
 *
 * INPUT:
 *      ptr_name    - region name
 *      revision    - revision id
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
mstp_cmd_setRegionConfig(
    const UI8_T    *ptr_name,
    const UI16_T    revision)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    DB_REQUEST_TYPE_T       req_type;
    DB_MSTP_REGION_CONFIG_T info;

    osapi_memset(&info, 0, sizeof(DB_MSTP_REGION_CONFIG_T));
    osapi_strncpy((C8_T *)info.region_name, (C8_T *)ptr_name, MAX_MSTP_REGION_NAME_SIZE);
    info.revision = revision;

    req_type = stp_dbmsg_init_reqtype(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES);
    rc = stp_db_update_msg(req_type, (void *)&info, sizeof(DB_MSTP_REGION_CONFIG_T));

    return rc;
}

#ifdef STP_DEBUG
/* FUNCTION NAME: mstp_cmd_setInstanceVlan
 * PURPOSE:
 *      Set instance mapping vlan
 *
 * INPUT:
 *      instance        - instance id
 *      ptr_vlanlist    - instance mapping vlan list
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
mstp_cmd_setInstanceVlan(
    const UI16_T    instance,
    const UI8_T     *ptr_vlanlist)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_MSTP_INSTANCE_T  ins_info;
    UI8_T               fid = MAX_MSTP_INSTANCE_NUM;
    UI16_T              vid[MAX_VLAN_ENTRY_NUM];
    UI32_T              vlanBmp = 0, i, idx, num;
    MSTP_INSTANCE_CONFIG_T info;

    memset(&ins_info, 0, sizeof(DB_MSTP_INSTANCE_T));
    memset(&vid, 0, sizeof(UI16_T) * MAX_VLAN_ENTRY_NUM);

    rc = mstp_db_get_instance_vlan_config(&info);
    if (MW_E_OK != rc)
    {
        return rc;
    }

    for(fid = 0; fid < MAX_MSTP_INSTANCE_NUM; fid++)
    {
        if (instance == MSTP_INSTANCE_GET_ID(info.instance[fid]))
        {
            break;
        }
    }

    if (MAX_MSTP_INSTANCE_NUM == fid)
    {
        MW_CMD_OUTPUT("Instance is not exsit\n");
        return MW_E_BAD_PARAMETER;
    }

    mstp_get_vid_by_vlanstr((UI8_T *)ptr_vlanlist, vid, &num);
    for (i = 0; i < num; i++)
    {
        if (0 == vid[i])
        {
            break;
        }

        mstp_GetMstVlanIdxByVid(vid[i], &idx);
        BIT_SET(vlanBmp, idx);
        BIT_DEL(info.vlanbmp[0], idx);
    }
    info.vlanbmp[fid] = vlanBmp;
    info.priority[fid] = STP_DEFAULT_PRIORITY;
    info.instance[fid] = instance;

    rc = mstp_db_update_instance_vlan_config(&info);

    return rc;
}
#endif

/* FUNCTION NAME: mstp_cmd_showMcidInfo
 * PURPOSE:
 *      Show mcid info
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
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mstp_cmd_showMcidInfo(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    UI16_T              instance;
    UI32_T              i = 0;
    DB_MSTP_INSTANCE_T  ins_info[MSTP_INSTANCE_MAX_NUM];
    DB_MSTP_REGION_CONFIG_T      info;
    C8_T                         buf[128] = {0};
    UI16_T                       vid[MAX_VLAN_ENTRY_NUM] = {0};
    UI32_T                       num;

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &info);
        if (MW_E_OK != rc)
        {
            return rc;
        }

        osapi_memset(ins_info, 0, sizeof(DB_MSTP_INSTANCE_T) * MSTP_INSTANCE_MAX_NUM);
        for (i = 0; i < MSTP_INSTANCE_MAX_NUM; i++)
        {
            req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, (i+1));
            rc = stp_db_get_msg(req_type, &ins_info[i]);
            if (MW_E_OK != rc)
            {
                break;
            }
        }
        if (MW_E_OK == rc)
        {
            mstp_get_mcid_digest((UI8_T *)buf);
            MW_CMD_OUTPUT("\n");
            MW_CMD_OUTPUT("Region Name: %s\n", info.region_name);
            MW_CMD_OUTPUT("Revision: %u\n", info.revision);
            MW_CMD_OUTPUT("Digest: \n");
            for (i = 0; i < 16; i++)
            {
                MW_CMD_OUTPUT(" %02x ", (UI8_T)(buf[i]));
            }
            MW_CMD_OUTPUT("\n");

            MW_CMD_OUTPUT("%5s  %19s  %8s  %12s \n", "Instance", "Vlan list", "Fid", "Priority");
            MW_CMD_OUTPUT("-----------------------------------------------------\n");

            for (i = 0; i < MSTP_INSTANCE_MAX_NUM; i++)
            {
                num = 0;
                instance = ins_info[i].instance;
                if (0 != ins_info[i].vlanbmp)
                {
                    mstp_get_mstVlanArrayByBmp(ins_info[i].vlanbmp, vid, &num);
                    mstp_getMstVlanStrByArray(num, vid, (UI8_T *)buf);
                }

                MW_CMD_OUTPUT("%8u  %19s  %8u  %12u \n", (instance & 0xfff),
                    (0 == ins_info[i].vlanbmp) ? "-" : buf,
                    ((instance & 0xf000) >> 12), ins_info[i].priority);
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: mstp_cmd_showInfo
 * PURPOSE:
 *      Show info
 *
 * INPUT:
 *      fid    -  fwd id
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
mstp_cmd_showInfo(
    UI32_T  fid)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    DB_STP_INFO_T       info;
    DB_MSTP_INSTANCE_PORT_INFO_T *port_info_list;
    DB_MSTP_INSTANCE_PORT_OPER_T *port_oper_list;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    UI32_T              trunk_id = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    const UI8_T         BUF_SIZE = 32;
    C8_T                buf[BUF_SIZE];
    UI8_T               autocost[PLAT_MAX_PORT_NUM];
#endif
    UI32_T              member;
    DB_MSTP_INSTANCE_T  mstp_info;
    DB_MSTP_REGION_CONFIG_T  mstp_region;
    C8_T                     str[128] = {0};
    UI16_T                   vid[MAX_VLAN_ENTRY_NUM] = {0};
    UI32_T                   num;

    data_size = sizeof(DB_MSTP_INSTANCE_PORT_INFO_T) * PLAT_MAX_PORT_NUM;
    osapi_calloc(data_size, STP_MODULE_NAME, (void **)&port_info_list);
    if (NULL == port_info_list)
    {
        STP_LOG_ERR("alloc memory failed.\n");
        return MW_E_NO_MEMORY;
    }

    data_size = sizeof(DB_MSTP_INSTANCE_PORT_OPER_T) * PLAT_MAX_PORT_NUM;
    osapi_calloc(data_size, STP_MODULE_NAME, (void **)&port_oper_list);
    if (NULL == port_oper_list)
    {
        STP_LOG_ERR("alloc memory failed.\n");
        MW_FREE(port_info_list);
        return MW_E_NO_MEMORY;
    }

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(STP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &info);
        if (MW_E_OK == rc)
        {
            for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
            {
                req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE_PORT, DB_ALL_FIELDS, port);
                rc = stp_db_get_msg(req_type, &port_info_list[port-1]);
                if (MW_E_OK != rc)
                {
                    break;
                }
            }
            if (MW_E_OK == rc)
            {
                for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
                {
                    req_type = stp_dbmsg_init_reqtype(MSTP_PORT_OPER_INFO, DB_ALL_FIELDS, port);
                    rc = stp_db_get_msg(req_type, &port_oper_list[port-1]);
                    if (MW_E_OK != rc)
                    {
                        break;
                    }
                }
            }
            if (MW_E_OK == rc)
            {
                req_type = stp_dbmsg_init_reqtype(MSTP_REGION, DB_ALL_FIELDS, DB_ALL_ENTRIES);
                rc = stp_db_get_msg(req_type, &mstp_region);
            }
            if (MW_E_OK == rc)
            {
                req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, (fid+1));
                rc = stp_db_get_msg(req_type,&mstp_info);
            }
#ifdef AIR_SUPPORT_STP_AUTO_COST
            if (MW_E_OK == rc)
            {
                req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, STP_PORT_AUTO_COST_ENABLE, DB_ALL_ENTRIES);
                rc = stp_db_get_msg(req_type, autocost);
            }
#endif
            if (MW_E_OK == rc)
            {
                MW_CMD_OUTPUT("Global State: %s\n", info.enable ? "Enable" : "Disable");
                MW_CMD_OUTPUT("Force Version: %s\n", (info.force_version == STP_VERSION_COMPATIBILITY) ? "STP" : ((info.force_version == STP_VERSION_DEFAULT) ? "RSTP" : "MSTP"));
                MW_CMD_OUTPUT("Forward Delay: %u\n", info.forward_delay);
                MW_CMD_OUTPUT("Max Age: %u\n", info.max_age);
                MW_CMD_OUTPUT("Transmit Hold Count: %u\n", info.transmit_hold_count);
                MW_CMD_OUTPUT("Bridge Priority: %u\n", mstp_info.priority);
                MW_CMD_OUTPUT("Region Name: %s\n", mstp_region.region_name);
                MW_CMD_OUTPUT("Revision: %u\n", mstp_region.revision);
                MW_CMD_OUTPUT("Instance: %u\n", MSTP_INSTANCE_GET_ID(mstp_info.instance));
                MW_CMD_OUTPUT("Fid: %u\n", MSTP_INSTANCE_GET_FID(mstp_info.instance));
                if (0 != mstp_info.vlanbmp)
                {
                    mstp_getMstVlanArrayByBmp(mstp_info.vlanbmp, vid, &num);
                    mstp_getMstVlanStrByArray(num, vid, (UI8_T *)str);
                }
                MW_CMD_OUTPUT("Vlan list: %s\n", (0 == mstp_info.vlanbmp) ? "-" : str);

                MW_CMD_OUTPUT("\n");
#ifdef AIR_SUPPORT_STP_AUTO_COST
                if (TRUE == stp_db_getAutoCostState())
                {
                    MW_CMD_OUTPUT("%5s  %10s  %15s  %7s  %10s\n", "Port","Priority", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("-------------------------------------------------------\n");
                }
                else
                {
                    MW_CMD_OUTPUT("%5s  %10s  %9s  %7s  %10s\n", "Port", "Priority", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("-------------------------------------------------------\n");
                }
#else
                MW_CMD_OUTPUT("%5s  %10s  %9s  %7s  %10s\n", "Port", "Priority", "Cost", "State", "Role");
                MW_CMD_OUTPUT("-------------------------------------------------------\n");
#endif
                for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
                {
                    stp_db_getTrunkID(port, &trunk_id);
                    if (trunk_id < MAX_TRUNK_NUM)
                    {
                        continue;
                    }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                    memset(buf, 0, BUF_SIZE);
                    if (FALSE == stp_db_getAutoCostState())
                    {
                        osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                    }
                    else
                    {
                        if (FALSE == stp_db_getStpGlobalState())
                        {
                            if (FALSE == autocost[port-1])
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                            }
                            else
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                            }
                        }
                        else
                        {
                            if (FALSE == autocost[port-1])
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                            }
                            else
                            {
                                if (STP_ROLE_DISABLED == port_oper_list[port-1].role)
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                }
                                else
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u%s", port_oper_list[port-1].oper_cost[fid], "(Auto)");
                                }
                            }
                        }
                    }
                    if (TRUE == stp_db_getAutoCostState())
                    {
                        MW_CMD_OUTPUT("%5u  %10u  %15s  %7s  %10s\n",
                            port,
                            port_info_list[port-1].priority[fid],
                            buf,
                            (port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                                port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                                port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                            (port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                                port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                                port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                                port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                                port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                    }
                    else
                    {
                        MW_CMD_OUTPUT("%5u  %10u  %9s  %7s  %10s\n",
                        port,
                        port_info_list[port-1].priority[fid],
                        buf,
                        (port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                        (port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                    }
#else
                    MW_CMD_OUTPUT("%5s  %10s  %9s  %7s  %10s\n", "Port", "Priority", "Cost", "State", "Role");

                    MW_CMD_OUTPUT("%5u  %10u  %9u  %7s  %10s\n",
                        port,
                        port_info_list[port-1].priority[fid],
                        port_info_list[port-1].cost[fid],
                        (port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                        (port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
#endif
                }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                if (TRUE == stp_db_getAutoCostState())
                {
                    MW_CMD_OUTPUT("\n%5s  %10s  %15s  %7s  %10s\n", "Trunk", "Priority", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("-------------------------------------------------------\n");
                }
                else
                {
                    MW_CMD_OUTPUT("\n%5s  %10s  %9s  %7s  %10s\n", "Trunk", "Priority", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("-------------------------------------------------------\n");
                }
#else
                MW_CMD_OUTPUT("\n%5s  %10s  %9s  %7s  %10s\n", "Trunk", "Priority", "Cost", "State", "Role");
                MW_CMD_OUTPUT("-------------------------------------------------------\n");
#endif
                for (trunk_id = 0 ; trunk_id < MAX_TRUNK_NUM ; trunk_id++)
                {
                    member = stp_db_trunk_getMemberBmp(trunk_id);
                    if (0 != member)
                    {
                        port = stp_db_trunk_getLowerPort(trunk_id);
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        memset(buf, 0, BUF_SIZE);
                        if (FALSE == stp_db_getAutoCostState())
                        {
                            osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                        }
                        else
                        {
                            if (FALSE == stp_db_getStpGlobalState())
                            {
                                if (FALSE == autocost[port-1])
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                                }
                                else
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                }
                            }
                            else
                            {
                                if (FALSE == autocost[port-1])
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost[fid]);
                                }
                                else
                                {
                                    if (STP_ROLE_DISABLED == port_oper_list[port-1].role[fid])
                                    {
                                        osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                    }
                                    else
                                    {
                                        osapi_snprintf(buf, BUF_SIZE, "%u%s", port_oper_list[port-1].oper_cost[fid], "(Auto)");
                                    }
                                }
                            }
                        }
                        if (TRUE == stp_db_getAutoCostState())
                        {
                            MW_CMD_OUTPUT("%5u  %10u  %15s  %7s  %10s\n",
                                (trunk_id + 1),
                                port_info_list[port-1].priority[fid],
                                buf,
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                                    port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                                    port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                        }
                        else
                        {
                            MW_CMD_OUTPUT("%5u  %10u  %9s  %7s  %10s\n",
                                (trunk_id + 1),
                                port_info_list[port-1].priority[fid],
                                buf,
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                                    port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                                    port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                                    port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                        }
#else
                        MW_CMD_OUTPUT("%5u  %10u  %9u  %7s  %10s\n",
                        (trunk_id + 1),
                        port_info_list[port-1].priority[fid],
                        port_info_list[port-1].cost[fid],
                        (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                            port_oper_list[port-1].state[fid] == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state[fid] == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state[fid] == STP_LEARNING ? "Learn" : "Disable"),
                        (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_MASTER ? "Master" :
                            port_oper_list[port-1].role[fid] == STP_ROLE_BACKUP ? "Backup" : "Disable"));
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                    }
                }
            }
        }
    }

    MW_FREE(port_info_list);
    MW_FREE(port_oper_list);

    return rc;
}

/* FUNCTION NAME: mstp_cmd_showInstancePort
 * PURPOSE:
 *      Show instance port param
 *
 * INPUT:
 *      fid                 - instance index
 *      portId              - port id
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
mstp_cmd_showInstancePort(
    UI32_T  portId,
    UI32_T  fid)
{
    mstp_show_port_instance_param(fid, portId);
    return MW_E_OK;
}
#endif

/* FUNCTION NAME: stp_cmd_showInfo
 * PURPOSE:
 *      Show info
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
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_OTHERS
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
rstp_cmd_showInfo(
    void)
{
    MW_ERROR_NO_T       rc = MW_E_OK;
    DB_REQUEST_TYPE_T   req_type;
    DB_STP_INFO_T       info;
    DB_STP_PORT_INFO_T  port_info_list[PLAT_MAX_PORT_NUM];
    DB_STP_PORT_OPER_T  port_oper_list[PLAT_MAX_PORT_NUM];
    UI32_T              port = 0;
    UI32_T              trunk_id = 0;
#ifdef AIR_SUPPORT_STP_AUTO_COST
    const UI8_T         BUF_SIZE = 32;
    C8_T                buf[BUF_SIZE];
#endif
    UI32_T              member;

    if (MW_E_OK == rc)
    {
        req_type = stp_dbmsg_init_reqtype(STP_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES);

        rc = stp_db_get_msg(req_type, &info);
        if (MW_E_OK == rc)
        {
            for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
            {
                req_type = stp_dbmsg_init_reqtype(STP_PORT_INFO, DB_ALL_FIELDS, port);
                rc = stp_db_get_msg(req_type, &port_info_list[port-1]);
                if (MW_E_OK != rc)
                {
                    break;
                }
            }
            if (MW_E_OK == rc)
            {
                for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
                {
                    req_type = stp_dbmsg_init_reqtype(STP_PORT_OPER_INFO, DB_ALL_FIELDS, port);
                    rc = stp_db_get_msg(req_type, &port_oper_list[port-1]);
                    if (MW_E_OK != rc)
                    {
                        break;
                    }
                }
            }
            if (MW_E_OK == rc)
            {
                MW_CMD_OUTPUT("Global State: %s\n", info.enable ? "Enable" : "Disable");
                MW_CMD_OUTPUT("Force Version: %s\n", (info.force_version == STP_VERSION_COMPATIBILITY) ? "STP" : ((info.force_version == STP_VERSION_DEFAULT) ? "RSTP" : "MSTP"));
                MW_CMD_OUTPUT("Forward Delay: %u\n", info.forward_delay);
                MW_CMD_OUTPUT("Max Age: %u\n", info.max_age);
                MW_CMD_OUTPUT("Transmit Hold Count: %u\n", info.transmit_hold_count);
                MW_CMD_OUTPUT("Bridge Priority: %u\n", info.priority);
                MW_CMD_OUTPUT("\n");
#ifdef AIR_SUPPORT_STP_AUTO_COST
                if (TRUE == stp_db_getAutoCostState())
                {
                    MW_CMD_OUTPUT("%5s  %8s  %8s  %10s  %7s  %15s  %7s  %10s\n", "Port", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("------------------------------------------------------------------------------------\n");
                }
                else
                {
                    MW_CMD_OUTPUT("%5s  %8s  %8s  %10s  %7s  %9s  %7s  %10s\n", "Port", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("------------------------------------------------------------------------------\n");
                }
#else
                MW_CMD_OUTPUT("%5s  %8s  %8s  %10s  %7s  %9s  %7s  %10s\n", "Port", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                MW_CMD_OUTPUT("------------------------------------------------------------------------------\n");
#endif
                for(port = 1 ; port <= PLAT_MAX_PORT_NUM ; port++)
                {
                    stp_db_getTrunkID(port, &trunk_id);
                    if (trunk_id < MAX_TRUNK_NUM)
                    {
                        continue;
                    }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                    memset(buf, 0, BUF_SIZE);
                    if (FALSE == stp_db_getAutoCostState())
                    {
                        osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                    }
                    else
                    {
                        if (FALSE == stp_db_getStpGlobalState())
                        {
                            if (FALSE == port_info_list[port-1].auto_cost_enable)
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                            }
                            else
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                            }
                        }
                        else
                        {
                            if (FALSE == port_info_list[port-1].auto_cost_enable)
                            {
                                osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                            }
                            else
                            {
                                if (STP_ROLE_DISABLED == port_oper_list[port-1].role)
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                }
                                else
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u%s", port_oper_list[port-1].oper_cost, "(Auto)");
                                }
                            }
                        }
                    }
                    if (TRUE == stp_db_getAutoCostState())
                    {
                        MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %15s  %7s  %10s\n",
                            port,
                            port_info_list[port-1].status ? "Enable" : "Disable",
                            ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                                (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                            port_info_list[port-1].priority,
                            port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                            buf,
                            (port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                                port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                                port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                            (port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                                port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                                port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                                port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                    }
                    else
                    {
                        MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %9s  %7s  %10s\n",
                        port,
                        port_info_list[port-1].status ? "Enable" : "Disable",
                        ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                            (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                        port_info_list[port-1].priority,
                        port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                        buf,
                        (port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                        (port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                    }
#else
                    MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %9u  %7s  %10s\n",
                        port,
                        port_info_list[port-1].status ? "Enable" : "Disable",
                        ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                            (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                        port_info_list[port-1].priority,
                        port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                        port_info_list[port-1].cost,
                        (port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                        (port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
#endif
                }
#ifdef AIR_SUPPORT_STP_AUTO_COST
                if (TRUE == stp_db_getAutoCostState())
                {
                    MW_CMD_OUTPUT("\n%5s  %8s  %8s  %10s  %7s  %15s  %7s  %10s\n", "Trunk", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("------------------------------------------------------------------------------------\n");
                }
                else
                {
                    MW_CMD_OUTPUT("\n%5s  %8s  %8s  %10s  %7s  %9s  %7s  %10s\n", "Trunk", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                    MW_CMD_OUTPUT("------------------------------------------------------------------------------\n");
                }
#else
                MW_CMD_OUTPUT("\n%5s  %8s  %8s  %10s  %7s  %9s  %7s  %10s\n", "Trunk", "Status", "Mode", "Priority", "Edge", "Cost", "State", "Role");
                MW_CMD_OUTPUT("------------------------------------------------------------------------------\n");
#endif
                for (trunk_id = 0 ; trunk_id < MAX_TRUNK_NUM ; trunk_id++)
                {
                    member = stp_db_trunk_getMemberBmp(trunk_id);
                    if (0 != member)
                    {
                        port = stp_db_trunk_getLowerPort(trunk_id);
#ifdef AIR_SUPPORT_STP_AUTO_COST
                        memset(buf, 0, BUF_SIZE);
                        if (FALSE == stp_db_getAutoCostState())
                        {
                            osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                        }
                        else
                        {
                            if (FALSE == stp_db_getStpGlobalState())
                            {
                                if (FALSE == port_info_list[port-1].auto_cost_enable)
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                                }
                                else
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                }
                            }
                            else
                            {
                                if (FALSE == port_info_list[port-1].auto_cost_enable)
                                {
                                    osapi_snprintf(buf, BUF_SIZE, "%u", port_info_list[port-1].cost);
                                }
                                else
                                {
                                    if (STP_ROLE_DISABLED == port_oper_list[port-1].role)
                                    {
                                        osapi_snprintf(buf, BUF_SIZE, "%s", "Auto");
                                    }
                                    else
                                    {
                                        osapi_snprintf(buf, BUF_SIZE, "%u%s", port_oper_list[port-1].oper_cost, "(Auto)");
                                    }
                                }
                            }
                        }
                        if (TRUE == stp_db_getAutoCostState())
                        {
                            MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %15s  %7s  %10s\n",
                                (trunk_id + 1),
                                port_info_list[port-1].status ? "Enable" : "Disable",
                                ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                                    (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                                port_info_list[port-1].priority,
                                port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                                buf,
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                                    port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                                    port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                                    port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                                    port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                                    port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                        }
                        else
                        {
                            MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %9s  %7s  %10s\n",
                                (trunk_id + 1),
                                port_info_list[port-1].status ? "Enable" : "Disable",
                                ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                                    (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                                port_info_list[port-1].priority,
                                port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                                buf,
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                                    port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                                    port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                                (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                                    port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                                    port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                                    port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                                    port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
                        }
#else
                        MW_CMD_OUTPUT("%5u  %8s  %8s  %10u  %7s  %9u  %7s  %10s\n",
                        (trunk_id + 1),
                        port_info_list[port-1].status ? "Enable" : "Disable",
                        ((info.force_version == STP_VERSION_MSTP) ? (((info.portMode >> (port-1)) & 0x1)? "Mstp" : "Stp") :
                            (((info.portMode >> (port-1)) & 0x1)? "Rstp" : "Stp")),
                        port_info_list[port-1].priority,
                        port_info_list[port-1].admin_edge ? "Enable" : "Disable",
                        port_info_list[port-1].cost,
                        (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                            port_oper_list[port-1].state == STP_FORWARDING ? "Forward" :
                            port_oper_list[port-1].state == STP_DISCARDING ? "Discard" :
                            port_oper_list[port-1].state == STP_LEARNING ? "Learn" : "Disable"),
                        (FALSE == stp_db_getStpGlobalState() ? "Disable" :
                            port_oper_list[port-1].role == STP_ROLE_ROOT ? "Root" :
                            port_oper_list[port-1].role == STP_ROLE_DESIGNATED ? "Designated" :
                            port_oper_list[port-1].role == STP_ROLE_ALTERNATE ? "Alternate" :
                            port_oper_list[port-1].role == STP_ROLE_BACKUP ? "Backup" : "Disable"));
#endif /* AIR_SUPPORT_STP_AUTO_COST */
                    }
                }
            }
        }
    }

    return rc;
}

/* FUNCTION NAME: stp_cmd_showInfo
 * PURPOSE:
 *      Show info
 *
 * INPUT:
 *      instance    -  instance id
 *      exsit_fid   - fid is valid
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
stp_cmd_showInfo(
    UI32_T  instance,
    BOOL_T  exsit_fid)
{
    MW_ERROR_NO_T rc;

#ifdef AIR_SUPPORT_MSTP
    DB_MSTP_INSTANCE_TABLE_T  mstp_info = {{0},{0},{0}};
    UI32_T                    fid = MAX_MSTP_INSTANCE_NUM;
    DB_REQUEST_TYPE_T         req_type;

    if (TRUE == exsit_fid)
    {
        req_type = stp_dbmsg_init_reqtype(MSTP_INSTANCE, DB_ALL_FIELDS, DB_ALL_ENTRIES);
        rc = stp_db_get_msg(req_type, &mstp_info);
        if (MW_E_OK == rc)
        {
            for(fid = 0; fid < MAX_MSTP_INSTANCE_NUM; fid++)
            {
                if (instance == MSTP_INSTANCE_GET_ID(mstp_info.instance[fid]))
                {
                    break;
                }
            }
        }

        if (MAX_MSTP_INSTANCE_NUM == fid)
        {
            MW_CMD_OUTPUT("Instance is not exsit\n");
            return MW_E_BAD_PARAMETER;
        }
        rc = mstp_cmd_showInfo(fid);
    }
    else
#endif
    {
        (void)instance;
        (void)exsit_fid;
        rc = rstp_cmd_showInfo();
    }

    return rc;
}

#ifdef STP_DEBUG
MW_ERROR_NO_T
stp_cmd_showUsedEntry(
    void)
{
    UI32_T i;

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        MW_CMD_OUTPUT("ACL Rule: %u\n", _bpdu_limit_acl_id[i]);
    }
#ifdef AIR_SUPPORT_MSTP
    mstp_show_port_member();
#endif

    return MW_E_OK;
}

MW_ERROR_NO_T
stp_cmd_stm_port_debug(
    UI32_T  port,
    UI8_T   enable)
{
    UI8_T i = 0;

    if (MAX_PORT_NUM < port)
    {
        port = stp_db_get_lag_lower_port(port);
    }

    if (FALSE == enable)
    {
        _stp_stm_debug_pbm &= ~(BIT(port));
    }
    else
    {
        _stp_stm_debug_pbm |= BIT(port);
    }

    for(i = 1; i <= MAX_PORT_NUM; i++){
        MW_CMD_OUTPUT("stp_sm_dbg_port[%u]=%u\n", i, (_stp_stm_debug_pbm & BIT(i)) ? 1 : 0);
    }

    return MW_E_OK;
}

unsigned char
stp_get_stm_dbg_flag(
    unsigned int portIdx)
{
    if (portIdx > (MAX_PORT_NUM + MAX_TRUNK_NUM))
    {
        return 0;
    }

    if (MAX_PORT_NUM < portIdx)
    {
        portIdx = stp_db_get_lag_lower_port(portIdx);
    }

    return ((_stp_stm_debug_pbm & BIT(portIdx)) ? 1 : 0);
}
#endif

#ifdef AIR_SUPPORT_RSTP
/* FUNCTION NAME: rstp_increase_task_priority
 * PURPOSE:
 *      Increase the RSTP task priority
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
rstp_increase_task_priority(
    void)
{
    /* Increase always no matter if RSTP is enabled or not to avoid timing issue
     * of enabling RSTP during the TLS handshake.
     */
    _rstp_set_task_priority(MW_TASK_PRIORITY_STP_HIGH, 100);
}

/* FUNCTION NAME: rstp_restore_task_priority
 * PURPOSE:
 *      Restore the RSTP task priority
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
rstp_restore_task_priority(
    void)
{
    _rstp_set_task_priority(MW_TASK_PRIORITY_STP_NORMAL, 0);
}

/* FUNCTION NAME: rstp_get_task_priority
 * PURPOSE:
 *      Get the RSTP task priority
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      The RSTP task priority
 *
 * NOTES:
 *      None
 */
UI32_T
rstp_get_task_priority(
    void)
{
    UI32_T task_priority = MW_TASK_PRIORITY_STP_NORMAL;

    osapi_threadPriorityGet(_stp_task_handle, &task_priority);

    return task_priority;
}
#endif

