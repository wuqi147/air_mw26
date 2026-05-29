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


/* FILE NAME:  lacp.c
 * PURPOSE:
 * It provides lacp module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#ifdef AIR_SUPPORT_LACP
#include "mw_log.h"
#include "syncd_api_lag.h"
#include "air_stp.h"
#include "air_swc.h"
#include "sys_mgmt.h"
#include "lacp.h"
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "sfp_port.h"
#endif

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

/* STATIC VARIABLE DECLARATIONS
 */
static TRUNK_MEMBER_INFO_T _lag_mem[MAX_TRUNK_NUM];
static UI32_T _lacp_sm_last_tick = 0;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T
lacp_init(
    NET_FILTER_T *ptr_lacpdu_pkt_netf)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T i;

    MW_LOG_INIT_PRINTF("Initializing LACP module...\n");
    AIR_SWC_MGMT_FRAME_CFG_T mgmt_frame_cfg = { 0 };

    if (MW_E_OK != lacp_allocate_settings_buffer())
    {
        MW_LOG_ERROR(LACP, "no memory for LACP settings buffer");
        return MW_E_NO_MEMORY;
    }
    lacp_info_t *ptr_lacp_info = lacp_get_settings();
    ptr_lacp_info->lacp_acl_id = MW_ACL_ID_INVALID;
    ptr_lacp_info->lacp_rate_limit_id = MW_ACL_ID_INVALID;
    ptr_lacp_info->lacp_max_port_num = LACP_MAX_PORTS;
    ptr_lacp_info->lacp_system_priority = LACP_SYSTEM_PRIO;
    for (i = 0;i < MAX_PORT_NUM;i++)
    {
        ptr_lacp_info->lacp_port_priority[i] = LACP_PORT_PRIO;
    }

    lag_lacp_timer_init();

#if LACP_MUTEX
    lag_lacp_mutex_init();
#endif

    mgmt_frame_cfg.flags = (AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE | AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU);
    mgmt_frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_02;
    mgmt_frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
    air_swc_setMgmtFrameCfg(0, &mgmt_frame_cfg);

    mgmt_frame_cfg.flags = (AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE | AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU);
    mgmt_frame_cfg.frame_type = AIR_SWC_MGMT_FRAME_TYPE_REV_02;
    mgmt_frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
    air_swc_setPortMgmtFrameCfg(0, 0, &mgmt_frame_cfg);

    if (AIR_E_OK != air_stag_setSpecialTagCfg(0, 0, TRUE))
    {
        MW_LOG_ERROR(LACP, "LACP set special tag config fail");
        return MW_E_NOT_INITED;
    }

    // ptr_lacpdu_pkt_netf = (NET_FILTER_T *)&(sys_mgmt_task_ctx.pkt_netf);
    ptr_lacpdu_pkt_netf->handle = osapi_msgFindHandle(SYS_MGMT_DB_QUEUE_NAME);
    if (NULL != ptr_lacpdu_pkt_netf->handle)
    {
        osapi_snprintf((C8_T *)ptr_lacpdu_pkt_netf->name, (MSG_MAX_NAME_LEN - 1), "%s", (C8_T *)SYS_MGMT_DB_QUEUE_NAME);
        ptr_lacpdu_pkt_netf->protocol = PROTO_LACP_SLOW;
        ptr_lacpdu_pkt_netf->state = NET_FILTER_REGISTER;

        ret = osapi_netRegister(ptr_lacpdu_pkt_netf);
        if (MW_E_OK != ret)
        {
            MW_LOG_ERROR(LACP, "LACP osapi_netRegister to %s fail(%d)", MAC_PKT_REGISTER, ret);
            return ret;
        }
    }
    else
    {
        MW_LOG_ERROR(LACP, "LACP can't find queue handle!");
        return MW_E_NOT_INITED;
    }
    return ret;
}

void
lacp_handle_pkt_and_timer_msg(
    MW_MSG_T *ptr_msg)
{
    lacp_info_t *ptr_lacp_info = lacp_get_settings();
    UI32_T tick = 0, dur_tick = 0;

    if (MW_MSG_ID_ETHERNET_PBUF == ptr_msg->msg_id)
    {
        NET_MSG_T *ptr_pkt_msg = (NET_MSG_T *)ptr_msg;
        UI32_T rx_port = ((struct pbuf *)(ptr_pkt_msg->ptr_pbuf))->stag_hdr.rx_hdr.sp;
        if (ptr_lacp_info->lag_port[rx_port - 1])
        {
#if LACP_MUTEX
            if (MW_E_OK == lag_lacp_mutex_take())
#endif
            {
                lacp_input(ptr_lacp_info->lag_port[rx_port - 1], ptr_pkt_msg->ptr_pbuf);
#if LACP_MUTEX
                lag_lacp_mutex_give();
#endif
            }
        }
        else
        {
            pbuf_free((struct pbuf *)ptr_pkt_msg->ptr_pbuf);
        }
    }
    else if ((LACP_TIMER_MSG_ID_SM == ptr_msg->msg_id) || (LACP_TIMER_MSG_ID_SD == ptr_msg->msg_id))
    {
#if LACP_MUTEX
        if (MW_E_OK == lag_lacp_mutex_take())
#endif
        {
            if (LACP_TIMER_MSG_ID_SM == ptr_msg->msg_id)
            {
                tick = osapi_sysTickGet();
                if (tick >= _lacp_sm_last_tick)
                {
                    dur_tick = tick - _lacp_sm_last_tick;
                }
                else
                {
                    dur_tick = 0xFFFFFFFF - _lacp_sm_last_tick + tick + 1;
                }
                MW_LOG_DEBUG(LACP, "LACP dur_tick = %u, last tick = %u", dur_tick, _lacp_sm_last_tick);
                if (dur_tick >= (LACP_TIMER_INTERVAL * 0.9))
                {
                    mw_lacp_tick();
                    _lacp_sm_last_tick = tick;
                }
                if (TRUE == lag_lacp_get_timer_flag())
                {
                    lag_lacp_set_timer_flag(FALSE);
                }
            }
            else
            {
                lacp_transit_expire(((lacp_tmr_msg_t *)ptr_msg)->param);
            }
#if LACP_MUTEX
            lag_lacp_mutex_give();
#endif
        }
    }
}

void
lacp_handle_db_msg(
    const UI8_T method,
    const DB_REQUEST_TYPE_T *ptr_request,
    const UI16_T data_size,
    const void *ptr_data)
{
    UI32_T unit = 0;
    UI8_T oper_status, oper_speed_or_fc, oper_duplex;
    UI16_T new_key;
    UI8_T i, idx, entry_num = 1;
    AIR_ERROR_NO_T air_rc;
    AIR_PORT_STATUS_T port_status;
    TRUNK_MEMBER_INFO_T trunk_member = { 0 };
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (PORT_OPER_INFO == ptr_request->t_idx)
    {
        if (ptr_request->e_idx == DB_ALL_ENTRIES)
        {
            entry_num = PLAT_MAX_PORT_NUM;
        }
        for (i = 0;i < entry_num;i++)
        {
            idx = ((DB_ALL_ENTRIES == ptr_request->e_idx) ? i : ptr_request->e_idx - 1);
            struct lacp_port *lp = ptr_lacp_info->lag_port[idx];
            if (NULL == lp)
            {
                continue;
            }

            if (ptr_request->f_idx == PORT_OPER_STATUS)
            {
                osapi_memcpy(&oper_status, ptr_data, sizeof(UI8_T));
                if (0 == oper_status)
                {
                    MW_LOG_INFO(LACP, "port[%d] link down, disable LACP port", idx + 1);
                    lacp_port_disable(lp);
                }
                else
                {
                    MW_LOG_INFO(LACP, "port[%d] link up, try to enable LACP port", idx + 1);
#ifdef AIR_SUPPORT_SFP
                    if ((TRUE == sfp_port_is_serdesPort(unit, idx + 1)) ||
                        (TRUE == sfp_port_is_comboPort(unit, idx + 1)))
                    {
                        air_rc = sfp_port_getPortStatus(unit, idx + 1, &port_status);
                    }
                    else
#endif
                    {
                        air_rc = air_port_getPortStatus(unit, idx + 1, &port_status);
                    }
                    if (AIR_E_OK == air_rc)
                    {
                        if (AIR_PORT_DUPLEX_FULL == port_status.duplex)
                        {
                            new_key = lacp_mw_compose_key(lp);
                            MW_LOG_INFO(LACP, "LACP port[%d] old key=0x%x, new key=0x%x", idx + 1, lp->lp_actor.lip_key, new_key);
                            if (new_key != lp->lp_actor.lip_key)
                            {
                                lp->lp_selected = LACP_UNSELECTED;
                                lp->lp_actor.lip_key = new_key;
                            }
                            lacp_port_enable(lp);
                            if (DB_ALL_ENTRIES != ptr_request->e_idx)
                            {
                                if (lp->lp_actor.lip_state & LACP_STATE_ACTIVITY)
                                {
                                    lacp_sm_assert_ntt(lp);
                                }
                            }
                        }
                        else
                        {
                            MW_LOG_INFO(LACP, "port[%d] is in Half duplex mode, cant be enabled", idx + 1);
                        }
                    }
                    else
                    {
                        MW_LOG_ERROR(LACP, "port[%d] get duplex mode fail", idx + 1);
                    }
                }
            }
            else if ((ptr_request->f_idx == PORT_OPER_SPEED) || (ptr_request->f_idx == PORT_OPER_FLOW_CTRL))
            {
                osapi_memcpy(&oper_speed_or_fc, ptr_data, sizeof(UI8_T));
                MW_LOG_INFO(LACP, "port[%d] %s changed to 0x%x", idx + 1, (ptr_request->f_idx == PORT_OPER_SPEED) ? "speed" : "flow ctrl", oper_speed_or_fc);
                new_key = lacp_mw_compose_key(lp);
                MW_LOG_INFO(LACP, "LACP port[%d] old key=0x%x, new key=0x%x", idx + 1, lp->lp_actor.lip_key, new_key);
                if (new_key != lp->lp_actor.lip_key)
                {
                    lp->lp_selected = LACP_UNSELECTED;
                    lp->lp_actor.lip_key = new_key;
                }
            }
            else if (ptr_request->f_idx == PORT_OPER_DUPLEX)
            {
                osapi_memcpy(&oper_duplex, ptr_data, sizeof(UI8_T));
                MW_LOG_INFO(LACP, "port[%d] duplex mode is changed to %d", idx + 1, oper_duplex);
                if (AIR_PORT_DUPLEX_HALF == oper_duplex)
                {
                    MW_LOG_INFO(LACP, "port[%d] is half duplex, disable LACP port", idx + 1);
                    lacp_port_disable(lp);
                }
            }
        }
    }
    else if (TRUNK_PORT == ptr_request->t_idx)
    {
        entry_num = (((sizeof(TRUNK_MEMBER_INFO_T) * MAX_TRUNK_NUM) == data_size) ? MAX_TRUNK_NUM : 1);
        for (i = 0; i < entry_num; i++)
        {
            osapi_memcpy(&trunk_member, (((TRUNK_MEMBER_INFO_T *)ptr_data) + i), sizeof(TRUNK_MEMBER_INFO_T));
            if (1 == entry_num)
            {
                i = ptr_request->e_idx - 1;
            }
            MW_LOG_INFO(LACP, "%s: LAG configuration: group %d, member 0x%x, mode %d", __func__, i + 1, trunk_member.member_bmp, trunk_member.mode);

            if (_lag_mem[i].member_bmp == 0 && trunk_member.member_bmp != 0)
            {
                if (trunk_member.mode != LAG_MODE_STATIC)
                {
                    lag_lacp_create_group(i, trunk_member.mode, trunk_member.member_bmp);
                }
            }
            else if (_lag_mem[i].member_bmp != 0 && trunk_member.member_bmp == 0)
            {
                if (_lag_mem[i].mode != LAG_MODE_STATIC)
                {
                    lag_lacp_destroy_group(i);
                }
            }
            _lag_mem[i].member_bmp = trunk_member.member_bmp;
            _lag_mem[i].mode = trunk_member.mode;
        }
    }
}
#endif /* AIR_SUPPORT_LACP */