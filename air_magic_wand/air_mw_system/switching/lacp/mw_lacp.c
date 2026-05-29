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

/* FILE NAME:  mw_lacp.c
 * PURPOSE:
 *  Implement internal function of lacp module.
 *
 * NOTES:
 *
 */

#include <string.h>
#include "mw_error.h"
#include "osapi.h"
#include "osapi_memory.h"
#include "osapi_message.h"
#include "osapi_string.h"
#include "db_data.h"
#include "mw_lacp.h"
#include "sys_mgmt.h"
#include "syncd_api_lag.h"
#include "dsh_parser.h"
#include "mw_acl.h"
#include "mw_cmd_util.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/
extern const uint8_t ethermulticastaddr_slowprotocols[ETHER_ADDR_LEN];

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */
static BOOL_T lacp_timer_msg_flag = FALSE;

/* LOCAL SUBPROGRAM BODIES
 */
static void
_lacp_timer_state_machine(
    timehandle_t timer)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    lacp_tmr_msg_t *ptr_message = NULL;

    if (FALSE == lacp_timer_msg_flag)
    {
        osapi_calloc(sizeof(lacp_tmr_msg_t), LACP_MODULE, (void **)&ptr_message);
        if (NULL != ptr_message)
        {
            ptr_message->msg_id = LACP_TIMER_MSG_ID_SM;
            rc = osapi_msgSend(SYS_MGMT_DB_QUEUE_NAME, (UI8_T *)ptr_message, 0, 0);
            if (MW_E_OK != rc)
            {
                MW_FREE(ptr_message);
            }
            else
            {
                lacp_timer_msg_flag = TRUE;
            }
        }
    }
}

static I8_T
_lag_lacp_set_rate_limit(
    UI32_T unit)
{
    I8_T acl_ap_id = 0;
    AIR_DOS_RATE_LIMIT_CFG_T dos_rate_limit;
    AIR_ERROR_NO_T rc;

    for (acl_ap_id = 0; acl_ap_id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID; acl_ap_id++)
    {
        if (AIR_E_OK == air_dos_getRateLimitCfg(unit, acl_ap_id, &dos_rate_limit))
        {
            if ((dos_rate_limit.pkt_thld == 0) && (dos_rate_limit.time_span == 0) && (dos_rate_limit.block_time == 0))
            {
                dos_rate_limit.pkt_thld = PLAT_MAX_PORT_NUM * 2;
                dos_rate_limit.time_span = 1000;
                dos_rate_limit.block_time = 1;
                dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
                rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
                if (rc != AIR_E_OK)
                {
                    MW_LOG_ERROR(LACP, "Add lacp attack prevention ap-id %d failed, rc=%d !", acl_ap_id, rc);
                    return -1;
                }
                else
                {
                    MW_LOG_INFO(LACP, "lacp attack prevention setting: time span=%dms, pkt_thld=%d", dos_rate_limit.time_span, dos_rate_limit.pkt_thld);
                }
                break;
            }
        }
    }
    return acl_ap_id;
}
/* EXPORTED SUBPROGRAM BODIES
 */
void
lag_sort_lacp_ports(
    UI8_T group_idx)
{
    UI8_T i, j, max_index, temp;
    UI8_T temp_ports[MAX_TRUNK_MEMBER_NUM];
    struct lacp_port *port_i, *port_j;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    osapi_memcpy(temp_ports, ptr_lacp_info->lag_group[group_idx]->port_prio_descending, MAX_TRUNK_MEMBER_NUM);

    for (i = 0; i < ptr_lacp_info->lag_group[group_idx]->member_cnt - 1; i++)
    {
        port_i = ptr_lacp_info->lag_port[ptr_lacp_info->lag_group[group_idx]->port_prio_descending[i] - 1];
        if ((NULL == port_i) || (port_i->group_id != group_idx))
        {
            continue;
        }
        max_index = ptr_lacp_info->lag_group[group_idx]->port_prio_descending[i] - 1;
        for (j = i + 1; j < ptr_lacp_info->lag_group[group_idx]->member_cnt; j++)
        {
            port_j = ptr_lacp_info->lag_port[ptr_lacp_info->lag_group[group_idx]->port_prio_descending[j] - 1];
            if (NULL == port_j || port_j->group_id != group_idx)
            {
                continue;
            }
            if (port_j->lp_actor.lip_portid.lpi_prio > ptr_lacp_info->lag_port[max_index]->lp_actor.lip_portid.lpi_prio ||
                (port_j->lp_actor.lip_portid.lpi_prio == ptr_lacp_info->lag_port[max_index]->lp_actor.lip_portid.lpi_prio &&
                 port_j->port_id < ptr_lacp_info->lag_port[max_index]->port_id))
            {
                max_index = ptr_lacp_info->lag_group[group_idx]->port_prio_descending[j] - 1;
            }
        }

        if (max_index != ptr_lacp_info->lag_group[group_idx]->port_prio_descending[i] - 1)
        {
            temp = temp_ports[i];
            temp_ports[i] = temp_ports[j];
            temp_ports[j] = temp;
        }
    }

    osapi_memcpy(ptr_lacp_info->lag_group[group_idx]->port_prio_descending, temp_ports, MAX_TRUNK_MEMBER_NUM);
    for (i = 0; i < ptr_lacp_info->lag_group[group_idx]->member_cnt; i++)
    {
        MW_LOG_DEBUG(LACP, "group[%d] port_prio_descending[%d]=%d", group_idx + 1, i, ptr_lacp_info->lag_group[group_idx]->port_prio_descending[i]);
    }
}

void
lacp_timer_suppress_distributing(
    timehandle_t timer)
{
    MW_ERROR_NO_T   rc = MW_E_OK;
    lacp_tmr_msg_t *ptr_message = NULL;
    UI32_T group;

    osapi_timerIdGet(timer, &group);
    osapi_calloc(sizeof(lacp_tmr_msg_t), LACP_MODULE, (void **)&ptr_message);
    if (NULL != ptr_message)
    {
        ptr_message->msg_id = LACP_TIMER_MSG_ID_SD;
        ptr_message->param = group;
        rc = osapi_msgSend(SYS_MGMT_DB_QUEUE_NAME, (UI8_T *)ptr_message, 0, 0);
        if (MW_E_OK != rc)
        {
            MW_FREE(ptr_message);
        }
    }
}

MW_ERROR_NO_T
lag_lacp_timer_init(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    rc = osapi_timerCreate(LACP_TIMER, _lacp_timer_state_machine, TRUE, LACP_TIMER_INTERVAL, NULL, &ptr_lacp_info->lacp_sm_timer_handle);
    if ((MW_E_OK != rc) || (NULL == ptr_lacp_info->lacp_sm_timer_handle))
    {
        MW_LOG_ERROR(LACP, "Create LACP state machine timer fail! rc:%d", rc);
        return MW_E_NO_MEMORY;
    }

    if (MW_E_OK != osapi_timerStart(ptr_lacp_info->lacp_sm_timer_handle))
    {
        MW_LOG_ERROR(LACP, "Start LACP state machine timer fail! rc:%d", rc);
        osapi_timerDelete(ptr_lacp_info->lacp_sm_timer_handle);
        return MW_E_OP_STOPPED;
    }

    return rc;
}

#if LACP_MUTEX
MW_ERROR_NO_T
lag_lacp_mutex_init(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL != ptr_lacp_info->lacp_mutex)
    {
        return MW_E_ALREADY_INITED;
    }
    ret = osapi_mutexCreate(LACP_MUTEX_NAME, &ptr_lacp_info->lacp_mutex);
    if (MW_E_OK != ret)
    {
        ptr_lacp_info->lacp_mutex = NULL;
        MW_LOG_ERROR(LACP, "%s: failed to create mutex(ret = %u)\n", LACP_MUTEX_NAME, (UI8_T)(ret));
    }
    return ret;
}

MW_ERROR_NO_T
lag_lacp_mutex_take(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL != ptr_lacp_info->lacp_mutex)
    {
        ret = osapi_mutexTake(ptr_lacp_info->lacp_mutex, LACP_MUTEX_WAITTIME);
    }
    return ret;
}

MW_ERROR_NO_T
lag_lacp_mutex_give(
    void)
{
    MW_ERROR_NO_T ret = MW_E_OTHERS;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL != ptr_lacp_info->lacp_mutex)
    {
        ret = osapi_mutexGive(ptr_lacp_info->lacp_mutex);
    }
    return ret;
}
#endif

/* FUNCTION NAME:   lag_lacp_create_port
 * PURPOSE:
 *      Initialize.
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
lag_lacp_create_port(
    UI8_T group_idx,
    UI8_T mode,
    UI16_T port)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL == ptr_lacp_info->lag_port[port - 1])
    {
        MW_LOG_INFO(LACP, "create port[%d] mode:%s", port, mode == LAG_MODE_LACP_ACTIVE ? "Active" : "Passive");
        lacp_port_create(&ptr_lacp_info->lag_port[port - 1], group_idx + 1, port);
        if (NULL != ptr_lacp_info->lag_port[port - 1])
        {
            if (LAG_MODE_LACP_PASSIVE == mode)
            {
                ptr_lacp_info->lag_port[port - 1]->lp_state &= ~LACP_STATE_ACTIVITY;
            }
            else if (LAG_MODE_LACP_ACTIVE == mode)
            {
                ptr_lacp_info->lag_port[port - 1]->lp_state |= LACP_STATE_ACTIVITY;
            }
        }
    }

    return rc;
}

MW_ERROR_NO_T
lag_lacp_destroy_port(
    UI16_T port)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    MW_LOG_INFO(LACP, "delete port[%d] port:0x%lx", port, (uintptr_t)ptr_lacp_info->lag_port[port - 1]);

    if (NULL != ptr_lacp_info->lag_port[port - 1])
    {
        lacp_port_destroy(ptr_lacp_info->lag_port[port - 1]);
        ptr_lacp_info->lag_port[port - 1] = NULL;
        ptr_lacp_info->lacpdu_tx_count[port - 1] = 0;
        ptr_lacp_info->lacpdu_rx_count[port - 1] = 0;
        ptr_lacp_info->markerpdu_tx_count[port - 1] = 0;
        ptr_lacp_info->markerpdu_rx_count[port - 1] = 0;
    }

    return rc;
}
/* FUNCTION NAME:   lag_lacp_create_group
 * PURPOSE:
 *      Initialize.
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
lag_lacp_create_group(
    UI8_T idx,
    UI8_T mode,
    UI32_T member)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T i = 0, j = 0;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL == ptr_lacp_info->lag_group[idx])
    {
        rc = osapi_calloc(sizeof(struct lacp_softc), LACP_MODULE, (void **)&ptr_lacp_info->lag_group[idx]);
        if ((MW_E_OK != rc) || (NULL == ptr_lacp_info->lag_group[idx]))
        {
            MW_LOG_ERROR(LACP, "create group(id:%d, member:0x%x) fail because of memory allocation fail!", idx + 1, member);
            return MW_E_NO_MEMORY;
        }
        ptr_lacp_info->lag_group[idx]->group_id = idx + 1;
        ptr_lacp_info->lag_group[idx]->member = member;
        TAILQ_INIT(&ptr_lacp_info->lag_group[idx]->lsc_aggregators);
        ptr_lacp_info->lag_group[idx]->lsc_active_aggregator = NULL;
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if ((1 << i) & member)
            {
                lag_lacp_create_port(idx, mode, i + 1);
                ptr_lacp_info->lag_group[idx]->port_prio_descending[j++] = i + 1;
                ptr_lacp_info->lag_group[idx]->member_cnt++;
            }
        }
        lag_lacp_add_acl(member);

        lag_sort_lacp_ports(idx);
        lacp_update_port_state();
        MW_LOG_INFO(LACP, "create group(id:%d, member:0x%x) success", ptr_lacp_info->lag_group[idx]->group_id, ptr_lacp_info->lag_group[idx]->member);
    }

    return rc;
}

/* FUNCTION NAME:   lag_lacp_destroy_group
 * PURPOSE:
 *      Initialize.
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
lag_lacp_destroy_group(
    UI8_T idx)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI8_T i;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    if (NULL != ptr_lacp_info->lag_group[idx])
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if ((1 << i) & ptr_lacp_info->lag_group[idx]->member)
            {
                lag_lacp_destroy_port(i + 1);
            }
        }
        lag_lacp_del_acl(ptr_lacp_info->lag_group[idx]->member);
        if (NULL != ptr_lacp_info->lacp_sd_timer_handle[idx])
        {
            osapi_timerDelete(ptr_lacp_info->lacp_sd_timer_handle[idx]);
            ptr_lacp_info->lacp_sd_timer_handle[idx] = NULL;
            MW_LOG_INFO(LACP, "Delete group[%d] suppress distributing timer", idx + 1);
        }
        MW_LOG_INFO(LACP, "Destroy group(id:%d, member:0x%x) success", ptr_lacp_info->lag_group[idx]->group_id, ptr_lacp_info->lag_group[idx]->member);
        MW_FREE(ptr_lacp_info->lag_group[idx]);
        lacp_update_port_state();
    }

    return rc;
}

MW_ERROR_NO_T
lag_lacp_add_acl(
    UI32_T port_bmp)
{
    AIR_ACL_RULE_T     acl_rule;
    AIR_ACL_ACTION_T   action;
    UI32_T             unit = 0;
    MW_ERROR_NO_T      rc = MW_E_OK;
    AIR_ERROR_NO_T     air_rc = AIR_E_OK;
    UI32_T             acl_attack_prevention_id = 0;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));

    mw_dos_setGlobalCfg(unit, TRUE);

    rc = mw_acl_mutex_take();
    if (MW_E_OK == rc)
    {
        if (MW_ACL_ID_INVALID == ptr_lacp_info->lacp_acl_id)
        {
            acl_attack_prevention_id = _lag_lacp_set_rate_limit(unit);
            rc = MW_ACL_GET_AVAILABLERULE(&ptr_lacp_info->lacp_acl_id, &acl_rule, unit);
            if (MW_E_OK == rc)
            {
                acl_rule.rule_en = TRUE;
                acl_rule.end = TRUE;
                osapi_memcpy(acl_rule.key.dmac, ethermulticastaddr_slowprotocols, 6);
                acl_rule.mask.dmac = 0x3f;
                acl_rule.portmap[0] = (port_bmp << 1);
                acl_rule.key.etype = ETHTYPE_LACP_SLOW;
                acl_rule.mask.etype = 0x3;
                acl_rule.field_valid = BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_DMAC_KEY);
                if ((acl_attack_prevention_id >= 0) && (acl_attack_prevention_id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID))
                {
                    action.attack_rate_id = acl_attack_prevention_id;
                    action.field_valid |= BIT(AIR_ACL_ATTACK);
                }
                air_rc = air_acl_setRule(unit, ptr_lacp_info->lacp_acl_id, &acl_rule);
                if (air_rc == AIR_E_OK)
                {
                    osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
                    action.acl_mib_id = MW_ACL_GEN_MIBID(ptr_lacp_info->lacp_acl_id);
                    action.field_valid |= (1U << AIR_ACL_MIB);
                    action.field_valid |= (1U << AIR_ACL_ATTACK);
                    if ((acl_attack_prevention_id >= 0) && (acl_attack_prevention_id < AIR_MAX_NUM_OF_ATTACK_PREVENTION_ID))
                    {
                        action.attack_rate_id = acl_attack_prevention_id;
                        action.field_valid |= BIT(AIR_ACL_ATTACK);
                        ptr_lacp_info->lacp_rate_limit_id = acl_attack_prevention_id;
                        MW_LOG_INFO(LACP, "ACL attack prevention id %d set", acl_attack_prevention_id);
                    }
                    air_rc = air_acl_setAction(unit, ptr_lacp_info->lacp_acl_id, &action);
                    if (AIR_E_OK == air_rc)
                    {
                        MW_LOG_INFO(LACP, "ACL rule %d is created by port bmp:0x%x", ptr_lacp_info->lacp_acl_id, port_bmp);
                    }
                    else
                    {
                        air_acl_delRule(unit, ptr_lacp_info->lacp_acl_id);
                    }
                }
            }
        }
        else
        {
            air_rc = air_acl_getRule(unit, ptr_lacp_info->lacp_acl_id, &acl_rule);
            if (air_rc == AIR_E_OK)
            {
                acl_rule.portmap[0] |= (port_bmp << 1);
                air_rc = air_acl_setRule(unit, ptr_lacp_info->lacp_acl_id, &acl_rule);
                if (air_rc == AIR_E_OK)
                {
                    MW_LOG_INFO(LACP, "ACL rule %d is updated by adding port bmp:0x%x(total port bmp:0x%x)", ptr_lacp_info->lacp_acl_id, port_bmp, acl_rule.portmap[0]);
                }
            }
        }
        mw_acl_mutex_release();
    }

    return rc;
}

MW_ERROR_NO_T
lag_lacp_del_acl(
    UI32_T port_bmp)
{
    UI32_T             unit = 0;
    MW_ERROR_NO_T      rc = MW_E_OK;
    AIR_ERROR_NO_T     air_rc = AIR_E_OK;
    AIR_ACL_RULE_T     acl_rule;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));

    rc = mw_acl_mutex_take();
    if (MW_E_OK == rc)
    {
        if (MW_ACL_ID_INVALID != ptr_lacp_info->lacp_acl_id)
        {
            air_rc = air_acl_getRule(unit, ptr_lacp_info->lacp_acl_id, &acl_rule);
            if (air_rc == AIR_E_OK)
            {
                acl_rule.portmap[0] &= ~(port_bmp << 1);
                if (acl_rule.portmap[0] != 0)
                {
                    air_rc = air_acl_setRule(unit, ptr_lacp_info->lacp_acl_id, &acl_rule);
                    if (air_rc == AIR_E_OK)
                    {
                        MW_LOG_INFO(LACP, "ACL rule %d is updated by removing port bmp:0x%x(total port bmp:0x%x)", ptr_lacp_info->lacp_acl_id, port_bmp, acl_rule.portmap[0]);
                    }
                }
                else
                {
                    air_acl_delRule(unit, ptr_lacp_info->lacp_acl_id);
                    air_acl_delAction(unit, ptr_lacp_info->lacp_acl_id);
                    MW_LOG_INFO(LACP, "ACL rule %d is deleted", ptr_lacp_info->lacp_acl_id);
                    ptr_lacp_info->lacp_acl_id = MW_ACL_ID_INVALID;
                    if (ptr_lacp_info->lacp_rate_limit_id != MW_ACL_ID_INVALID)
                    {
                        air_dos_clearRateLimitCfg(unit, ptr_lacp_info->lacp_rate_limit_id);
                        MW_LOG_INFO(LACP, "ACL attack prevention id %d deleted", ptr_lacp_info->lacp_rate_limit_id);
                        ptr_lacp_info->lacp_rate_limit_id = MW_ACL_ID_INVALID;
                    }
                }
            }
        }
        mw_acl_mutex_release();
    }

    return rc;
}

BOOL_T
lag_lacp_get_timer_flag(
    void)
{
    return lacp_timer_msg_flag;
}

void
lag_lacp_set_timer_flag(
    BOOL_T flag)
{
    lacp_timer_msg_flag = flag;
}

MW_ERROR_NO_T
mw_cmd_lacp_show_info(
    void)
{
    UI8_T i, j, k, count;
    struct lacp_aggregator *la;
    struct lacp_port *lp;
    UI32_T unit = 0;
    AIR_ERROR_NO_T air_rc;
    AIR_STP_STATE_T air_stp_state;
    BOOL_T list_empty = TRUE;
    C8_T *ptr_buf;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();
    UI8_T *ptr_member0 = syncd_api_lag_get_member0();

    osapi_calloc(LACP_LAGIDSTR_MAX + 1, LACP_MODULE, (void **)&ptr_buf);
    MW_CMD_OUTPUT("\n*********LACP information*********\n");
    MW_CMD_OUTPUT("LACP ACL ID: %d\n", ptr_lacp_info->lacp_acl_id);
    MW_CMD_OUTPUT("LACP Attack Prevention ID: %d\n", ptr_lacp_info->lacp_rate_limit_id);
    MW_CMD_OUTPUT("LACP max member number: %d\n", ptr_lacp_info->lacp_max_port_num);
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        count = 0;
        if (ptr_lacp_info->lag_group[i])
        {
            MW_CMD_OUTPUT("LAG Group[ID: %d]\n", ptr_lacp_info->lag_group[i]->group_id);
            /* MW_CMD_OUTPUT("  Mode: %s\n", ptr_lacp_info->lag_group[i]->mode == LACP_MODE_ACTIVE ? "Active" : "Passive"); */
            MW_CMD_OUTPUT("  Member port bitmap: 0x%X, member count: %d, member 0: %d\n", ptr_lacp_info->lag_group[i]->member, ptr_lacp_info->lag_group[i]->member_cnt, ptr_member0[i]);
            MW_CMD_OUTPUT("  Priority descending portlist= ");
            for (k = 0; k < ptr_lacp_info->lag_group[i]->member_cnt; k++)
            {
                MW_CMD_OUTPUT("%d,", ptr_lacp_info->lag_group[i]->port_prio_descending[k]);
            }
            MW_CMD_OUTPUT("\n");
            TAILQ_FOREACH(la, &ptr_lacp_info->lag_group[i]->lsc_aggregators, la_q)
            {
                MW_CMD_OUTPUT("  Aggregator[%d, %s active] id: %s\n",
                    count++, ptr_lacp_info->lag_group[i]->lsc_active_aggregator == la ? "is" : "not", lacp_format_lagid_aggregator(la, ptr_buf, LACP_LAGIDSTR_MAX + 1));
                MW_CMD_OUTPUT("    Port count: %d\n", la->la_nports);
                MW_CMD_OUTPUT("    Portlist: ");
                TAILQ_FOREACH(lp, &la->la_ports, lp_dist_q)
                {
                    MW_CMD_OUTPUT("%d,", lp->port_id);
                }
                MW_CMD_OUTPUT("\n");
            }
            for (j = 0; j < PLAT_MAX_PORT_NUM; j++)
            {
                if (ptr_lacp_info->lag_port[j] && ptr_lacp_info->lag_port[j]->group_id == ptr_lacp_info->lag_group[i]->group_id)
                {
                    MW_CMD_OUTPUT("    Port[%d]:\n", ptr_lacp_info->lag_port[j]->port_id);
                    MW_CMD_OUTPUT("      System ID/Priority: %s/%d\n", lacp_format_mac(ptr_lacp_info->lag_port[j]->lp_actor.lip_systemid.lsi_mac, ptr_buf, LACP_LAGIDSTR_MAX + 1), ntohs(ptr_lacp_info->lag_port[j]->lp_actor.lip_systemid.lsi_prio));
                    MW_CMD_OUTPUT("      Actor State:  %s\n", lacp_format_state(ptr_lacp_info->lag_port[j]->lp_actor.lip_state, ptr_buf, LACP_LAGIDSTR_MAX + 1));
                    MW_CMD_OUTPUT("      Parner State: %s\n", lacp_format_state(ptr_lacp_info->lag_port[j]->lp_partner.lip_state, ptr_buf, LACP_LAGIDSTR_MAX + 1));
                    MW_CMD_OUTPUT("      Actor Priority:   %d\n", ntohs(ptr_lacp_info->lag_port[j]->lp_actor.lip_portid.lpi_prio));
                    MW_CMD_OUTPUT("      Partner Priority: %d\n", ntohs(ptr_lacp_info->lag_port[j]->lp_partner.lip_portid.lpi_prio));
                    MW_CMD_OUTPUT("      Actor Key:   0x%x\n", ptr_lacp_info->lag_port[j]->lp_actor.lip_key);
                    MW_CMD_OUTPUT("      Partner Key: 0x%x\n", ptr_lacp_info->lag_port[j]->lp_partner.lip_key);
                    MW_CMD_OUTPUT("      SELECTED state: %s\n", ptr_lacp_info->lag_port[j]->lp_selected == LACP_SELECTED ? "Selected" : "Unselected");
                    MW_CMD_OUTPUT("      MUX state: %s\n", lacp_format_mux_state(ptr_lacp_info->lag_port[j]->lp_mux_state));
                    MW_CMD_OUTPUT("      Flags: 0x%x\n", ptr_lacp_info->lag_port[j]->lp_flags);
                    MW_CMD_OUTPUT("      Timer[Current While/Periodic/Wait While]: [%d/%d/%d]\n", ptr_lacp_info->lag_port[j]->lp_timer[0], ptr_lacp_info->lag_port[j]->lp_timer[1], ptr_lacp_info->lag_port[j]->lp_timer[2]);
                }
            }
        }
        else
        {
            MW_CMD_OUTPUT("LAG Group[%d] is not exist\n\n", i);
        }
    }

    MW_CMD_OUTPUT("Blocked portlist: ");
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        air_rc = air_stp_getPortState(unit, (UI32_T)(i + 1), 0, &air_stp_state);
        if (AIR_E_OK == air_rc && air_stp_state == AIR_STP_STATE_LISTEN)
        {
            MW_CMD_OUTPUT("%d,", i + 1);
            if (list_empty)
            {
                list_empty = FALSE;
            }
        }
    }
    if (list_empty)
    {
        MW_CMD_OUTPUT("None");
    }
    MW_CMD_OUTPUT("\n");
    list_empty = TRUE;
    MW_CMD_OUTPUT("Unblocked portlist: ");
    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        air_rc = air_stp_getPortState(unit, (UI32_T)(i + 1), 0, &air_stp_state);
        if (AIR_E_OK == air_rc && air_stp_state == AIR_STP_STATE_FORWARD)
        {
            MW_CMD_OUTPUT("%d,", i + 1);
            if (list_empty)
            {
                list_empty = FALSE;
            }
        }
    }
    if (list_empty)
    {
        MW_CMD_OUTPUT("None");
    }
    MW_CMD_OUTPUT("\n\n");
    MW_CMD_OUTPUT("SDK LAG info:\n");
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        osapi_memset(ptr_buf, 0, LACP_LAGIDSTR_MAX + 1);
        snprintf(ptr_buf, LACP_LAGIDSTR_MAX + 1, "lag show group lag-id=%d", i);
        dsh_parseString(ptr_buf);
    }
    MW_FREE(ptr_buf);
    MW_CMD_OUTPUT("\n\n");

    return MW_E_OK;
}
