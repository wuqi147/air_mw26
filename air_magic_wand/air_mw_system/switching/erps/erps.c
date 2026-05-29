/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2024 Airoha Technology Corp. All rights reserved.
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

/* FILE NAME:  erps.c
 * PURPOSE:
 *    This file contains the implementation of ERPS.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "erps.h"
#include    "erps_queue.h"
#include    "erps_timer.h"
#include    "erps_gpio.h"
#include    "erps_state_machine.h"
#include    "air_stag.h"
#include    "air_stp.h"
#include    "lwip/pbuf.h"
#include    "lwip/ip4.h"
#include    "vlan_utils.h"
#include    "sys_mgmt.h"
#include    "air_l2.h"
#include    "air_dos.h"
#include    "erps_hw_init.h"
#include    <lwip/stats.h>
#include    <lwip/snmp.h>
#include    <util.h>
#include    <hal/common/hal.h>
#include    <hal/common/hal_mdio.h>
#ifdef AIR_SUPPORT_IGMP_SNP
#include    "igmp_snoop_msg.h"
#endif /* AIR_SUPPORT_IGMP_SNP */
#ifdef AIR_SUPPORT_SFP
#include    "sfp_util.h"
#include    "sfp_port.h"
#include    "sfp_pin.h"
#include    "sfp_config_customer.h"
#endif /* AIR_SUPPORT_SFP */
#include    "mw_acl.h"
#include    "mw_dos.h"


/* NAMING CONSTANT DECLARATIONS
 */
#define PSR_P3_P0_REG               (0x10218030)
#define PSR_P7_P4_REG               (0x10218034)
#define PER_PSR_PORTS               (4)
#define PER_PSR_OFFSET              (8)
#define PSR_LINK_STS_MASK           (0x00000001)
#define MAX_PHY_PORT                (8)

/* MACRO FUNCTION DECLARATIONS
 */
#define GET_PSR_LINK_STS(data, mac_port)    ((data & (PSR_LINK_STS_MASK << ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))) >> ((mac_port % PER_PSR_PORTS) * PER_PSR_OFFSET))

/* DATA TYPE DECLARATIONS
 */
typedef struct ERPS_CONTEXT_S
{
    threadhandle_t      task_handle;
    StackType_t         task_stack[ERPS_TASK_STACK_SIZE];
    StaticTask_t        task_tcb;
} ERPS_CONTEXT_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
extern void*
pdma_get_ethernet_pkt_buf(
    unsigned int index,
    unsigned int* ptr_len);

extern void
pdma_release_ethernet_pkt_buf(
    unsigned int index);

static MW_ERROR_NO_T
_erps_handle_db_erps_info(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

static MW_ERROR_NO_T
_erps_acl_add(
    ERPS_RING_T     *ptr_ring);

static MW_ERROR_NO_T
_erps_acl_del(
    ERPS_RING_T     *ptr_ring);

static MW_ERROR_NO_T
_erps_port_link_status_get(
    UI8_T          port_id,
    UI8_T          *ptr_status);

static MW_ERROR_NO_T
_erps_wtr_timer_expire(
    UI8_T           id,
    void            *ptr_arg);

static MW_ERROR_NO_T
_erps_wtb_timer_expire(
    UI8_T           id,
    void            *ptr_arg);

/* FUNCTION NAME:   _erps_hold_off_expire
 * PURPOSE:
 *      This function is used to handle the expiration of the hold-off timer.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      ptr_arg              -- Pointer to the argument
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_erps_hold_off_timer_expire(
    UI8_T           id,
    void            *ptr_arg);

static MW_ERROR_NO_T
_erps_guard_timer_expire(
    UI8_T           id,
    void            *ptr_arg);

static void
_erps_sw_timer_timeout_check(
    void);

static MW_ERROR_NO_T
_erps_link_status_change_noti_handle(
    void *ptr_arg);

static MW_ERROR_NO_T
_erps_gpio_rx_loss_change_noti_handle(
    void *ptr_arg);

static MW_ERROR_NO_T
_erps_port_admin_state_change_noti_handle(
    void *ptr_arg);

static MW_ERROR_NO_T
_erps_link_status_change_process(
    UI8_T   port_id,
    UI8_T   link_stats);

static ERPS_PORT_T *
_erps_ring_port_get(
    ERPS_RING_T     *ptr_ring,
    UI8_T           port_id);

static void
_erps_task(
    void *ptr_arg);

static MW_ERROR_NO_T
_erps_raps_msg_buffer_init(
    void);

static void
_erps_raps_msg_handle(
    void);

static void
_erps_raps_msg_input(
    UI8_T *ptr_buf,
    UI32_T len);

static void
_erps_db_msg_handle(
    DB_MSG_T *ptr_msg);

static MW_ERROR_NO_T
_erps_db_msg_process(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data);

#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
static MW_ERROR_NO_T
_erps_port_fast_link_down_init(
    void);
#endif

static MW_ERROR_NO_T
_erps_instance_init(
    ERPS_RING_T             *ptr_ring);

static MW_ERROR_NO_T
_erps_instance_update(
    ERPS_RING_T             *ptr_ring);

static MW_ERROR_NO_T
_erps_instance_deinit(
    ERPS_RING_T             *ptr_ring);

static MW_ERROR_NO_T
_erps_instance_param_check_init(
    ERPS_RING_T             *ptr_ring);

static MW_ERROR_NO_T
_erps_link_status_polling(
    void);

/* STATIC VARIABLE DECLARATIONS
 */
static UI8_T            _link_sts[MAX_PHY_PORT] = {0};
static UI8_T            _phy_air_port_map[MAX_PHY_PORT] = {0};
static UI8_T            _erps_raps_msg_tx_buffer[ERPS_RAPS_TX_BUFFER_LEN] = {0};
static ERPS_RING_T      _erps_ring_instance[ERPS_MAX_INSTANCE_NUM] = {{0}};
static ERPS_CONTEXT_T   _erps_ctx = {0};

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_erps_handle_db_erps_info(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    UI8_T                   index = 0, entry_num = 0;
    UI8_T                   is_update = FALSE;
    UI16_T                  db_idx = 0;
    ERPS_RING_T             *ptr_ring = NULL;
    ERPS_PORT_T             west_port = {0}, east_port = {0};
    MW_ERROR_NO_T           rc = MW_E_OK;
    DB_ERPS_INFO_T          *ptr_erps_info = NULL;
    ERPS_ENTRY_INFO_T       *ptr_erps_entry = NULL;

    ERPS_LOG_DEBUG("db_field:%d, db_eidx:0x%d, data_size:%d, method:%x",
                                                request.f_idx,
                                                request.e_idx,
                                                data_size,
                                                method);

    entry_num = ((DB_ALL_ENTRIES == request.e_idx) ? ERPS_MAX_INSTANCE_NUM : 1);
    switch (method)
    {
        case M_GET:
        case M_CREATE:
            if(DB_ALL_FIELDS != request.f_idx)
            {
                ERPS_LOG_WARN("Unsupported update operation, f_idx:%d, e_idx:%d", request.f_idx, request.e_idx);
                return MW_E_NOT_SUPPORT;
            }
            if(DB_ALL_ENTRIES == request.e_idx)
            {
                /* All entry */
                for(index = 0; index < entry_num; index++)
                {
                    db_idx = (index + 1);
                    ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[index]);
                    if((0 != ptr_ring->ring_id) && (0 != ptr_ring->raps_vid))
                    {
                        ERPS_LOG_ERROR("Instance already exist.");
                        continue;
                    }
                    ptr_erps_info = (DB_ERPS_INFO_T *)ptr_data;
                    if((FALSE == VLAN_IS_VID_VALID(ptr_erps_info->control_vlan[index])) ||
                       (FALSE == ERPS_RING_ID_IS_VALID(ptr_erps_info->ring_id[index])))
                    {
                        ERPS_LOG_WARN("Method:%d, invalid vlan id:%d or ring id:%d .", method, ptr_erps_info->control_vlan[index], ptr_erps_info->ring_id[index]);
                        continue;
                    }
                    ptr_ring->ring_id = ptr_erps_info->ring_id[index];
                    ptr_ring->raps_vid = ptr_erps_info->control_vlan[index];
                    osapi_memcpy(ptr_ring->data_vid, ptr_erps_info->data_vlan[index], ERPS_DATA_VLAN_SIZE);
                    ptr_ring->revertive_mode = ptr_erps_info->revertive_mode[index];
                    ptr_ring->west_port.port_id = (ptr_erps_info->west_port[index] >> 2);
                    ptr_ring->west_port.attr_role = (((ERPS_PORT_ATTR_WEST_PORT << 4) & 0xF0) | ((ptr_erps_info->west_port[index] & 0x03) & 0xF));
                    ptr_ring->east_port.port_id = (ptr_erps_info->east_port[index] >> 2);
                    ptr_ring->east_port.attr_role = (((ERPS_PORT_ATTR_EAST_PORT << 4) & 0xF0) | ((ptr_erps_info->east_port[index] & 0x03) & 0xF));
                    ptr_ring->wtr_time = ptr_erps_info->wtr_timer[index];
                    ptr_ring->guard_time = ptr_erps_info->guard_timer[index];
                    ptr_ring->hold_off_time = ptr_erps_info->hold_off_timer[index];
                    ptr_ring->db_idx = db_idx;
                    ERPS_LOG_INFO("[%d]: r_id:%d, r_vid:%d, rvt_mode:%d, w_port:%d, e_port:%d, wtr_time:%d, g_time:%d, h_time:%d",
                                                                                                        db_idx,
                                                                                                        ptr_erps_info->ring_id[index],
                                                                                                        ptr_erps_info->control_vlan[index],
                                                                                                        ptr_erps_info->revertive_mode[index],
                                                                                                        ptr_erps_info->west_port[index],
                                                                                                        ptr_erps_info->east_port[index],
                                                                                                        ptr_erps_info->wtr_timer[index],
                                                                                                        ptr_erps_info->guard_timer[index],
                                                                                                        ptr_erps_info->hold_off_timer[index]);

                    rc = _erps_instance_init(ptr_ring);
                    if(MW_E_OK != rc)
                    {
                        ERPS_LOG_ERROR("Init erps instance failed, index:%d, rc:%d", index, rc);
                    }
                }
            }
            else
            {
                /* Single entry */
                do{
                    db_idx = request.e_idx;
                    ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[db_idx - 1]);
                    if((0 != ptr_ring->ring_id) && (0 != ptr_ring->raps_vid))
                    {
                        ERPS_LOG_ERROR("Instance already exist.");
                        break;
                    }
                    ptr_erps_entry = (ERPS_ENTRY_INFO_T *)ptr_data;
                    if((FALSE == VLAN_IS_VID_VALID(ptr_erps_entry->control_vlan)) ||
                       (FALSE == ERPS_RING_ID_IS_VALID(ptr_erps_entry->ring_id)))
                    {
                        ERPS_LOG_WARN("Method:%d, invalid vlan id:%d or ring id:%d .", method,  ptr_erps_entry->control_vlan,  ptr_erps_entry->ring_id);
                        break;
                    }
                    ptr_ring->ring_id =  ptr_erps_entry->ring_id;
                    ptr_ring->raps_vid =  ptr_erps_entry->control_vlan;
                    osapi_memcpy(ptr_ring->data_vid,  ptr_erps_entry->data_vlan, ERPS_DATA_VLAN_SIZE);
                    ptr_ring->revertive_mode =  ptr_erps_entry->revertive_mode;
                    ptr_ring->west_port.port_id = (ptr_erps_entry->west_port >> 2);
                    ptr_ring->west_port.attr_role = (((ERPS_PORT_ATTR_WEST_PORT << 4) & 0xF0) | ((ptr_erps_entry->west_port & 0x03) & 0xF));
                    ptr_ring->east_port.port_id = (ptr_erps_entry->east_port >> 2);
                    ptr_ring->east_port.attr_role = (((ERPS_PORT_ATTR_EAST_PORT << 4) & 0xF0) | ((ptr_erps_entry->east_port & 0x03) & 0xF));
                    ptr_ring->wtr_time = ptr_erps_entry->wtr_timer;
                    ptr_ring->guard_time = ptr_erps_entry->guard_timer;
                    ptr_ring->hold_off_time = ptr_erps_entry->hold_off_timer;
                    ptr_ring->db_idx = db_idx;
                    ERPS_LOG_INFO("[%d]: r_id:%d, r_vid:%d, rvt_mode:%d, w_port:%d, e_port:%d, wtr_time:%d, g_time:%d, h_time:%d",
                                                                                                        db_idx,
                                                                                                        ptr_erps_entry->ring_id,
                                                                                                        ptr_erps_entry->control_vlan,
                                                                                                        ptr_erps_entry->revertive_mode,
                                                                                                        ptr_erps_entry->west_port,
                                                                                                        ptr_erps_entry->east_port,
                                                                                                        ptr_erps_entry->wtr_timer,
                                                                                                        ptr_erps_entry->guard_timer,
                                                                                                        ptr_erps_entry->hold_off_timer);

                    rc = _erps_instance_init(ptr_ring);
                    if(MW_E_OK != rc)
                    {
                        ERPS_LOG_ERROR("Init erps instance failed, index:%d, rc:%d", index, rc);
                    }
                } while(0);
            }

            break;

        case M_UPDATE:
            if((DB_ALL_FIELDS != request.f_idx) || (DB_ALL_ENTRIES == request.e_idx))
            {
                ERPS_LOG_WARN("Unsupported update operation, f_idx:%d, e_idx:%d", request.f_idx, request.e_idx);
                return MW_E_NOT_SUPPORT;
            }
            db_idx = request.e_idx;
            ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[db_idx - 1]);
            if((0 == ptr_ring->ring_id) || (0 == ptr_ring->raps_vid))
            {
                ERPS_LOG_ERROR("Instance not exist.");
                return MW_E_NOT_SUPPORT;
            }
            ptr_erps_entry = (ERPS_ENTRY_INFO_T *)ptr_data;
            if(ptr_ring->revertive_mode != ptr_erps_entry->revertive_mode)
            {
                ptr_ring->revertive_mode =  ptr_erps_entry->revertive_mode;
                is_update = TRUE;
            }
            osapi_memcpy(&west_port, &ptr_ring->west_port, sizeof(ERPS_PORT_T));
            osapi_memcpy(&east_port, &ptr_ring->east_port, sizeof(ERPS_PORT_T));
            if(ptr_ring->west_port.port_id != ( ptr_erps_entry->west_port >> 2))
            {
                erps_port_block(ptr_ring, &ptr_ring->west_port, FALSE);
                ptr_ring->west_port.port_id = ( ptr_erps_entry->west_port >> 2);
                is_update = TRUE;
            }
            if(ptr_ring->west_port.attr_role != (((ERPS_PORT_ATTR_WEST_PORT << 4) & 0xF0) | (( ptr_erps_entry->west_port & 0x03) & 0xF)))
            {
                ptr_ring->west_port.attr_role = (((ERPS_PORT_ATTR_WEST_PORT << 4) & 0xF0) | (( ptr_erps_entry->west_port & 0x03) & 0xF));
                is_update = TRUE;
            }
            if(ptr_ring->east_port.port_id != ( ptr_erps_entry->east_port >> 2))
            {
                erps_port_block(ptr_ring, &ptr_ring->east_port, FALSE);
                ptr_ring->east_port.port_id = ( ptr_erps_entry->east_port >> 2);
                is_update = TRUE;
            }
            if(ptr_ring->east_port.attr_role != (((ERPS_PORT_ATTR_EAST_PORT << 4) & 0xF0) | (( ptr_erps_entry->east_port & 0x03) & 0xF)))
            {
                ptr_ring->east_port.attr_role = (((ERPS_PORT_ATTR_EAST_PORT << 4) & 0xF0) | (( ptr_erps_entry->east_port & 0x03) & 0xF));
                is_update = TRUE;
            }
            ptr_ring->wtr_time =  ptr_erps_entry->wtr_timer;
            ptr_ring->guard_time =  ptr_erps_entry->guard_timer;
            ptr_ring->hold_off_time =  ptr_erps_entry->hold_off_timer;
            ERPS_LOG_INFO("[%d]: r_id:%d, r_vid:%d, rvt_mode:%d, w_port:%d, e_port:%d, wtr_time:%d, g_time:%d, h_time:%d",
                                                                                                db_idx,
                                                                                                ptr_erps_entry->ring_id,
                                                                                                ptr_erps_entry->control_vlan,
                                                                                                ptr_erps_entry->revertive_mode,
                                                                                                ptr_erps_entry->west_port,
                                                                                                ptr_erps_entry->east_port,
                                                                                                ptr_erps_entry->wtr_timer,
                                                                                                ptr_erps_entry->guard_timer,
                                                                                                ptr_erps_entry->hold_off_timer);
            if(TRUE == is_update)
            {
                erps_fdb_flush(ptr_ring, NULL);
                rc = _erps_instance_update(ptr_ring);
                if(MW_E_OK != rc)
                {
                    ERPS_LOG_ERROR("Update erps instance failed, index:%d, rc:%d", index, rc);
                }
            }
            break;

        case M_DELETE:
            if(DB_ALL_FIELDS != request.f_idx)
            {
                ERPS_LOG_WARN("Unsupported update operation, f_idx:%d, e_idx:%d", request.f_idx, request.e_idx);
                return MW_E_NOT_SUPPORT;
            }
            for(index = 0; index < entry_num; index++)
            {
                db_idx = ((DB_ALL_ENTRIES == request.e_idx) ? (index + 1): request.e_idx);
                ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[db_idx - 1]);
                _erps_instance_deinit(ptr_ring);
            }
            break;

        default:
            break;
    }



    return rc;
}

static MW_ERROR_NO_T
_erps_acl_add(
    ERPS_RING_T     *ptr_ring)
{
    I32_T                       rc = 0;
    UI32_T                      acl_rule_id = MW_ACL_ID_INVALID, acl_ap_id = MW_ACL_ID_INVALID;
    AIR_MAC_T                   dmac = {0x01, 0x19, 0xA7, 0x00, 0x00, 0x00};
    const UI32_T                unit = 0;
    AIR_ACL_RULE_T              acl_rule;
    AIR_ACL_ACTION_T            action;
    AIR_DOS_RATE_LIMIT_CFG_T    dos_rate_limit;

    MW_CHECK_PTR(ptr_ring);
    if(FALSE == VLAN_IS_VID_VALID(ptr_ring->raps_vid))
    {
        return MW_E_NOT_INITED;
    }
    if(MW_E_OK == mw_acl_mutex_take())
    {
        mw_dos_setGlobalCfg(unit, TRUE);
        rc = MW_ATTACK_ID_GET_AVAILABLERULE(&acl_ap_id, &dos_rate_limit, unit);
        if(MW_E_OK == rc)
        {
            dos_rate_limit.pkt_thld = ERPS_DOS_RATE_LIMIT_PKT_THLD;
            dos_rate_limit.time_span = 1000;
            dos_rate_limit.block_time = 1;
            dos_rate_limit.tick_sel = AIR_DOS_RATE_TICKSEL_1MS;
            rc = air_dos_setRateLimitCfg(unit, acl_ap_id, &dos_rate_limit);
            if (AIR_E_OK != rc)
            {
                mw_acl_mutex_release();
                ERPS_LOG_ERROR("Add ERPS(ring_id=%d, vlan_id=%d) ACL ap-id %d failed, rc=%d!\n",
                                                                    ptr_ring->ring_id, ptr_ring->raps_vid, acl_ap_id, rc);
                return MW_E_OP_INCOMPLETE;
            }
        }
        mw_acl_mutex_release();
        if(MW_E_OK != rc)
        {
            ERPS_LOG_ERROR("Get free ACL ap-id failed !\n");
            return MW_E_ENTRY_REACH_END;
        }
    }

    dmac[5] = ptr_ring->ring_id;
    if(MW_E_OK == mw_acl_mutex_take())
    {
        /* Add erps acl */
        rc = MW_ACL_GET_AVAILABLERULE(&acl_rule_id, &acl_rule, unit);
        if (MW_E_OK == rc)
        {
            osapi_memset(&acl_rule, 0, sizeof(AIR_ACL_RULE_T));
            acl_rule.rule_en = TRUE;
            AIR_PORT_ADD(acl_rule.portmap, ptr_ring->east_port.port_id);
            AIR_PORT_ADD(acl_rule.portmap, ptr_ring->west_port.port_id);
            acl_rule.end = TRUE;
            acl_rule.key.etype = ERPS_ETHTYPE_RAPS;
            acl_rule.mask.etype = 0x3;
            osapi_memcpy(acl_rule.key.dmac, dmac, MAC_ADDRESS_LEN);
            acl_rule.mask.dmac = 0x3f;
            acl_rule.key.ctag = ptr_ring->raps_vid;
            acl_rule.mask.ctag = 0x1;
            acl_rule.field_valid |= (BIT(AIR_ACL_ETYPE_KEY) | BIT(AIR_ACL_DMAC_KEY) | BIT(AIR_ACL_CTAG_KEY));
            rc = air_acl_setRule(unit, acl_rule_id, &acl_rule);
            if (AIR_E_OK == rc)
            {
                osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
#ifdef ERPS_DEBUG_LOG
                action.acl_mib_id = MW_ACL_GEN_MIBID(acl_rule_id); /* Set mib id as acl rule id. */
                action.field_valid |= BIT(AIR_ACL_MIB);
#endif /* ERPS_DEBUG_LOG */
                action.port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
                action.pri_user = MW_ACL_RX_PRIORITY_RAPS;
                action.attack_rate_id = acl_ap_id;
                action.field_valid |= (BIT(AIR_ACL_FW_PORT) | BIT(AIR_ACL_PRI) | BIT(AIR_ACL_ATTACK));
                rc = air_acl_setAction(unit, acl_rule_id, &action);
                if (AIR_E_OK == rc)
                {
                    ptr_ring->ring_acl_id = acl_rule_id;
                }
                else
                {
                    ERPS_LOG_ERROR("Add ERPS(ring_id=%d, vlan_id=%d) ACL rule entry-id %d action fail, rc=%d.\n",
                                                                            ptr_ring->ring_id, ptr_ring->raps_vid, acl_rule_id, rc);

                    air_acl_delRule(unit, acl_rule_id);
                }
            }
            else
            {
                ERPS_LOG_ERROR("Add ERPS(ring_id=%d, vlan_id=%d) ACL rule entry-id %d fail, rc=%d.\n",
                                                                ptr_ring->ring_id, ptr_ring->raps_vid, acl_rule_id, rc);
            }
        }
        else
        {
            ERPS_LOG_ERROR("Get free ACL rule-id for ERPS failed rc:%d\n", rc);
        }

        mw_acl_mutex_release();
        if (AIR_E_OK != rc)
        {
            return rc;
        }
    }

    ERPS_LOG_INFO("Add ERPS(ring_id=%d, vlan_id=%d) ACL Done, acl_rule_id:%d, rc:%d",
                                            ptr_ring->ring_id, ptr_ring->raps_vid, acl_rule_id, rc);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_erps_acl_del(
    ERPS_RING_T     *ptr_ring)
{
    UI32_T              unit = 0, acl_ap_id = MW_ACL_ID_INVALID;
    AIR_ERROR_NO_T      rc;
    AIR_ACL_ACTION_T    action;

    MW_CHECK_PTR(ptr_ring);

    if(MW_E_OK == mw_acl_mutex_take())
    {
        mw_dos_setGlobalCfg(unit, FALSE);
        osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
        rc = air_acl_getAction(unit, ptr_ring->ring_acl_id, &action);
        if (AIR_E_OK == rc)
        {
            acl_ap_id = action.attack_rate_id;
            air_dos_clearRateLimitCfg(unit, acl_ap_id);
        }
        rc = air_acl_delAction(unit, ptr_ring->ring_acl_id);
        if(AIR_E_OK != rc)
        {
            ERPS_LOG_ERROR("Failed to delete ERPS ACL rule entry-id %d action, rc %d.", ptr_ring->ring_acl_id, rc);
        }
        rc = air_acl_delRule(unit, ptr_ring->ring_acl_id);
        if(AIR_E_OK != rc)
        {
            ERPS_LOG_ERROR("Failed to delete ERPS ACL rule entry-id %d , rc %d.", ptr_ring->ring_acl_id, rc);
        }
        mw_acl_mutex_release();
        ptr_ring->ring_acl_id = MW_ACL_ID_INVALID;
    }

    return MW_E_OK;
}

static MW_ERROR_NO_T
_erps_port_link_status_get(
    UI8_T          port_id,
    UI8_T          *ptr_status)
{
    UI32_T              phy_id = 0, unit = 0, psr = ~0;
    AIR_ERROR_NO_T      air_rc = AIR_E_OK;
    AIR_PORT_STATUS_T   p_state;

    *ptr_status = 0;
    /* Get current port status */
    if(TRUE == MW_PORT_CHK(PLAT_PORT_BMP_GPHY, port_id))
    {
        HAL_AIR_PORT_TO_PHY_PORT(unit, port_id, phy_id);
        if(phy_id >= MAX_PHY_PORT)
        {
            return MW_E_BAD_PARAMETER;
        }
        /* REG is accesssed temporarily. SDK API will be used in the future. */
        if(PER_PSR_PORTS <= phy_id)
        {
            psr = io_read32(PSR_P7_P4_REG);
            *ptr_status = GET_PSR_LINK_STS(psr, phy_id);
        }
        else
        {
            psr = io_read32(PSR_P3_P0_REG);
            *ptr_status = GET_PSR_LINK_STS(psr, phy_id);
        }
    }
    else
    {
#ifdef AIR_SUPPORT_SFP
        if ((TRUE == sfp_port_is_serdesPort(unit, port_id)) ||
            (TRUE == sfp_port_is_comboPort(unit, port_id)))
        {
            air_rc = sfp_port_getPortStatus(unit, port_id, &p_state);
        }
        else
#endif
        {
            air_rc = air_port_getPortStatus(unit, port_id, &p_state);
        }

        if(AIR_E_OK != air_rc)
        {
            ERPS_LOG_ERROR("Failed to get port status, port:%d, rc:%d", port_id, air_rc);
            return MW_E_NOT_SUPPORT;
        }
        *ptr_status = (p_state.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) ? 1 : 0;
    }

    return MW_E_OK;
}

/* FUNCTION NAME:   _erps_wtr_timer_expire
 * PURPOSE:
 *      This function is used to handle the expiration of the WTR timer.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      ptr_arg              -- Pointer to the argument
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
_erps_wtr_timer_expire(
    UI8_T           id,
    void            *ptr_arg)
{
    ERPS_RING_T     *ptr_ring = NULL;

    ERPS_LOG_INFO("WTR timer timeout.");
    if(NULL != ptr_arg)
    {
        ptr_ring = (ERPS_RING_T *)ptr_arg;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_WTR_EXPIRES, NULL);
        ptr_ring->wtr_timer_idx = ERPS_INVALID_TIMER_IDX;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   _erps_wtb_timer_expire
 * PURPOSE:
 *      This function is used to handle the expiration of the WTB timer.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      ptr_arg              -- Pointer to the argument
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_erps_wtb_timer_expire(
    UI8_T           id,
    void            *ptr_arg)
{
    ERPS_RING_T     *ptr_ring = NULL;

    ERPS_LOG_INFO("WTB timer timeout.");
    if(NULL != ptr_arg)
    {
        ptr_ring = (ERPS_RING_T *)ptr_arg;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_WTB_EXPIRES, NULL);
        ptr_ring->wtb_timer_idx = ERPS_INVALID_TIMER_IDX;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   _erps_hold_off_expire
 * PURPOSE:
 *      This function is used to handle the expiration of the hold-off timer.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      ptr_arg              -- Pointer to the argument
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_erps_hold_off_timer_expire(
    UI8_T           id,
    void            *ptr_arg)
{
    ERPS_RING_T     *ptr_ring = NULL;

    ERPS_LOG_INFO("Hold-off timer timeout.");
    if(NULL != ptr_arg)
    {
        ptr_ring = (ERPS_RING_T *)ptr_arg;
        /* Check port link status */
        ERPS_LOG_INFO("Port:%d, link stats:%s", ptr_ring->west_port.port_id, (ptr_ring->west_port.is_failed ? "down":"up"));
        if(TRUE == ptr_ring->west_port.is_failed)
        {
            erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->west_port));
        }
        ERPS_LOG_INFO("Port:%d, link stats:%s", ptr_ring->east_port.port_id, (ptr_ring->east_port.is_failed ? "down":"up"));
        if(TRUE == ptr_ring->east_port.is_failed)
        {
            erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->east_port));
        }
        ptr_ring->hold_off_timer_idx = ERPS_INVALID_TIMER_IDX;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   _erps_guard_expire
 * PURPOSE:
 *      This function is used to handle the expiration of the guard timer.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      ptr_arg              -- Pointer to the argument
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
static MW_ERROR_NO_T
_erps_guard_timer_expire(
    UI8_T           id,
    void            *ptr_arg)
{
    ERPS_RING_T     *ptr_ring = NULL;

    ERPS_LOG_INFO("Guard timer timeout.");
    if(NULL != ptr_arg)
    {
        ptr_ring = (ERPS_RING_T *)ptr_arg;
        ptr_ring->guard_timer_idx = ERPS_INVALID_TIMER_IDX;
    }
    return MW_E_OK;
}

static void
_erps_sw_timer_timeout_check(
    void)
{
    UI8_T           index = 0;
    ERPS_RING_T     *ptr_ring = NULL;

    for (index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        if(TRUE == _erps_ring_instance[index].is_send_raps)
        {
            ptr_ring = (ERPS_RING_T *)&_erps_ring_instance[index];
            if(0 < ptr_ring->send_raps_time_remain)
            {
                ptr_ring->send_raps_time_remain--;
                if(0 == ptr_ring->send_raps_time_remain)
                {
                    /* Timeout, send RAPS message */
                    ERPS_LOG_DEBUG("Ring:%d, timeout, send RAPS msg", ptr_ring->ring_id);
                    erps_raps_msg_send(ptr_ring, ptr_ring->raps_request, ptr_ring->raps_status);
                    ptr_ring->send_raps_time_remain = ERPS_SEND_RAPS_MSG_INTERVAL;
                }
            }
        }
    }
    return;
}

static MW_ERROR_NO_T
_erps_link_status_change_noti_handle(
    void *ptr_arg)
{
    UI8_T           port_id = 0, link_stats = 0;
    UI32_T          psr_p3_p0 = ~0, psr_p7_p4 = ~0, int_staust = 0, phy_id = 0;

    MW_CHECK_PTR(ptr_arg);
    osapi_memcpy(&int_staust, ptr_arg, sizeof(UI32_T));
    /* REG is accesssed temporarily. SDK API will be used in the future. */
    psr_p3_p0 = io_read32(PSR_P3_P0_REG);
    psr_p7_p4 = io_read32(PSR_P7_P4_REG);
    for(phy_id = 0; phy_id < MAX_PHY_PORT; phy_id++)
    {
        if(0 != (int_staust & (1 << phy_id)))
        {
            if(PER_PSR_PORTS <= phy_id)
            {
                link_stats = GET_PSR_LINK_STS(psr_p7_p4, phy_id);
            }
            else
            {
                link_stats = GET_PSR_LINK_STS(psr_p3_p0, phy_id);
            }
            if (_link_sts[phy_id] != link_stats)
            {
                _link_sts[phy_id] = link_stats;
                port_id = _phy_air_port_map[phy_id];
                ERPS_LOG_INFO("Port:%d change to %s", port_id, (link_stats ? "LINK UP" : "LINK DOWN"));
                _erps_link_status_change_process(port_id, link_stats);
            }
        }
    }
    return MW_E_OK;
}

static MW_ERROR_NO_T
_erps_gpio_rx_loss_change_noti_handle(
    void *ptr_arg)
{
    UI8_T           port_id = 0, gpio = 0, link_stats = 0;
    UI32_T          data = 0;
    UI32_T          pin_value = 0;

    MW_CHECK_PTR(ptr_arg);
    osapi_memcpy(&data, ptr_arg, sizeof(UI32_T));

    port_id = (data >> 24) & 0xFF;
    gpio = (data >> 16) & 0xFF;
    pin_value = ((data >> 0) & 0xFFFF) ? 1 : 0;

    link_stats = (1 == pin_value) ? 0 : 1;
    ERPS_LOG_INFO("Port:%d(GPIO:%d) change to %s", port_id, gpio, (link_stats ? "LINK UP" : "LINK DOWN"));
    _erps_link_status_change_process(port_id, link_stats);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_erps_port_admin_state_change_noti_handle(
    void *ptr_arg)
{
    UI8_T           port_id = 0, link_stats = 0;
    UI32_T          data = 0;

    MW_CHECK_PTR(ptr_arg);
    osapi_memcpy(&data, ptr_arg, sizeof(UI32_T));

    port_id = (data >> 16) & 0xFFFF;
    link_stats = ((data >> 0) & 0xFFFF) ? 1 : 0;

    ERPS_LOG_INFO("Port:%d admin state change to %s", port_id, (link_stats ? "Enable" : "Disable"));
    _erps_link_status_change_process(port_id, link_stats);

    return MW_E_OK;
}

static MW_ERROR_NO_T
_erps_link_status_change_process(
    UI8_T   port_id,
    UI8_T   link_stats)
{
    UI8_T               index = 0;
    ERPS_PORT_T         *ptr_port = NULL, *ptr_another_port = NULL;
    ERPS_RING_T         *ptr_ring = NULL;

    for (index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        if((0 != _erps_ring_instance[index].ring_id) &&
            (0 != _erps_ring_instance[index].raps_vid))
        {
            ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[index]);
            if(port_id == ptr_ring->west_port.port_id)
            {
                ptr_port = &(ptr_ring->west_port);
                ptr_another_port = &(ptr_ring->east_port);
            }
            else if(port_id == ptr_ring->east_port.port_id)
            {
                ptr_port = &(ptr_ring->east_port);
                ptr_another_port = &(ptr_ring->west_port);
            }
            else
            {
                /* Port is not ring port */
                ERPS_LOG_WARN("Port:%d is not ring(%d) port", port_id, ptr_ring->ring_id);
                continue;
            }
            if(0 == link_stats)
            {
                ptr_port->is_failed = TRUE;
                if(0 != ptr_ring->hold_off_time)
                {
                    /* Start hold-off timer */
                    erps_hold_off_timer_stop(ptr_ring);
                    erps_hold_off_timer_start(ptr_ring);
                }
                else
                {
                    /* Local SF */
                    erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, ptr_port);
                }
            }
            else
            {
                erps_hold_off_timer_stop(ptr_ring);
                ptr_port->is_failed = FALSE;
                if(TRUE == ptr_another_port->is_failed)
                {
                    /* Local SF */
                    ERPS_LOG_DEBUG("Port:%d is still failed.", ptr_another_port->port_id);
                    erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, ptr_another_port);
                }
                else
                {
                    /* Local clear SF */
                    erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_CLEAR_SF, ptr_port);
                }
            }
        }
    }
    return MW_E_OK;
}

static ERPS_PORT_T *
_erps_ring_port_get(
    ERPS_RING_T     *ptr_ring,
    UI8_T           port_id)
{
    ERPS_PORT_T     *ptr_port = NULL;

    if(port_id == ptr_ring->west_port.port_id)
    {
        ptr_port = &(ptr_ring->west_port);
    }
    else if(port_id == ptr_ring->east_port.port_id)
    {
        ptr_port = &(ptr_ring->east_port);
    }

    return  ptr_port;
}

/* FUNCTION NAME:   _erps_task
 * PURPOSE:
 *      This function implements the task of ERPS.
 *
 * INPUT:
 *      ptr_arg
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
static void
_erps_task(
    void *ptr_arg)
{
    UI32_T                  start_tick, end_tick, dur_tick;
    MW_MSG_T                *ptr_msg = NULL;
    MW_ERROR_NO_T           rc = MW_E_OK;
    QueueSetMemberHandle_t  xActivatedMember = NULL;
    ERPS_QUEUE_CONTEXT_T    *ptr_queue_ctx = NULL;

    UNUSED(ptr_arg);
    ERPS_LOG_DEBUG("Check DB is ready or not...");
    /* Check DB is ready or not */
    do
    {
        osapi_delay(5);
        rc = dbapi_dbisReady();
    }while(MW_E_OK != rc);
#ifdef AIR_SUPPORT_SFP
    if (0 != sfp_config_getPortSettingsArraySize())
    {
        do
        {
            osapi_delay(5);
        }while(TRUE != sfp_check_ready());
    }
#endif
    /* Subscribe DB */
    erps_queue_db_send(ERPS_MW_MSG_QUEUE_NAME, M_SUBSCRIBE, ERPS_INFO, DB_ALL_FIELDS, DB_ALL_ENTRIES, NULL, 0);
    ERPS_LOG_DEBUG("erps subscribe DB done!");

    ptr_queue_ctx = erps_queue_context_get();
    start_tick = osapi_sysTickGet();
    while(1)
    {
        osapi_msgPoll(ERPS_QUEUE_SET_NAME, ERPS_POLLING_DELAY, &xActivatedMember);
        if(xActivatedMember == ((QueueSetMemberHandle_t)&(ptr_queue_ctx->pktQueueStruct)))
        {
            _erps_raps_msg_handle();
        }
        else if(xActivatedMember == ((QueueSetMemberHandle_t)&(ptr_queue_ctx->mwQueueStruct)))
        {
            rc = osapi_msgRecv(ERPS_MW_MSG_QUEUE_NAME, (UI8_T **)&ptr_msg, 0, ERPS_TASK_DELAY);
            if(MW_E_OK == rc)
            {
                if(MW_MSG_ID_DB == ptr_msg->msg_id)
                {
                    _erps_db_msg_handle((DB_MSG_T *)ptr_msg);
                }
                else
                {
                    /* unknown notification */
                    ERPS_LOG_WARN("Receive unknown notification msg_id:%d", ptr_msg->msg_id);
                }
                MW_FREE(ptr_msg);
            }
        }
        else if(xActivatedMember == ((QueueSetMemberHandle_t)&(ptr_queue_ctx->evtQueueStruct)))
        {
            ERPS_QUEUE_EVT_MSG_T evt_msg = {0};

            rc = osapi_msgRecv(ERPS_EVT_QUEUE_NAME, (UI8_T **)&evt_msg, 0, ERPS_TASK_DELAY);
            if(MW_E_OK == rc)
            {
                switch (evt_msg.msg_id)
                {
                    case MW_MSG_ID_ERPS_LINK_STATUS_CHANGE_NOTI:
                        _erps_link_status_change_noti_handle((void *)&(evt_msg.data));
                        break;

                    case MW_MSG_ID_ERPS_SW_TIMER_EXPIRED_NOTI:
                        _erps_sw_timer_timeout_check();
                        break;

                    case MW_MSG_ID_ERPS_HW_TIMER_EXPIRED_NOTI:
                        erps_hw_timer_timeout_check();
                        break;

                    case MW_MSG_ID_ERPS_SFP_RX_LOSS_CHANGE_NOTI:
                        _erps_gpio_rx_loss_change_noti_handle((void *) &(evt_msg.data));
                        break;

                    case MW_MSG_ID_ERPS_PORT_ADMIN_STATE_CHANGE_NOTI:
                        _erps_port_admin_state_change_noti_handle((void *) &(evt_msg.data));
                        break;

                    default:
                        /* unknown notification */
                        ERPS_LOG_WARN("Recv unknown notification msg_id:%d", evt_msg.msg_id);
                        break;
                }
                /* No need free message memory */
            }
        }

        end_tick = osapi_sysTickGet();
        dur_tick = ((end_tick >= start_tick) ? (end_tick - start_tick) : (0xFFFFFFFF - start_tick + end_tick + 1));
        if (dur_tick > ERPS_POLLING_DELAY)
        {
            start_tick = osapi_sysTickGet();
            _erps_link_status_polling();
        }
    }
}

static MW_ERROR_NO_T
_erps_raps_msg_buffer_init(
    void)
{
    UI16_T                      offset = 0;
    UI16_T                      eth_type = lwip_htons(ERPS_ETHTYPE_RAPS);
    UI32_T                      u32dat = 0;
    ERPS_RAPS_MSG_T             *ptr_raps_msg = NULL;

    /* Ethernet MAC address */
    _erps_raps_msg_tx_buffer[0] = 0x01;
    _erps_raps_msg_tx_buffer[1] = 0x19;
    _erps_raps_msg_tx_buffer[2] = 0xA7;
    _erps_raps_msg_tx_buffer[3] = 0x00;
    _erps_raps_msg_tx_buffer[4] = 0x00;
    _erps_raps_msg_tx_buffer[5] = 0x01;   /* Ring ID */
    osapi_memcpy((_erps_raps_msg_tx_buffer + MAC_ADDRESS_LEN), _mw_mac_addr, MAC_ADDRESS_LEN);
    offset += MAC_ADDRESS_LEN * 2;

    /* Stag header */
    offset += SIZEOF_STAG_HDR;

    /* VLAN header, num:1 */
    u32dat = 0;
    u32dat |= (ETHTYPE_VLAN << 16);     /* TPID */
    u32dat |= (7 << 13);                /* User priority */
    u32dat |= (0 << 12);                /* CFI */
    u32dat |= (1 << 0);                 /* Default VLAN ID */
    u32dat = lwip_htonl(u32dat);
    osapi_memcpy((void *)(_erps_raps_msg_tx_buffer + offset), &u32dat, SIZEOF_VLAN_HDR);
    offset += SIZEOF_VLAN_HDR;

    /* Ethernet type */
    osapi_memcpy((void *)(_erps_raps_msg_tx_buffer + offset), &eth_type, SIZEOF_ETH_TYPE);
    offset += SIZEOF_ETH_TYPE;

    /* R-APS message */
    ptr_raps_msg = (ERPS_RAPS_MSG_T *)(_erps_raps_msg_tx_buffer + offset);
    ptr_raps_msg->mel_ver        = ((ERPS_RAPS_MEL << 5) | 1 << 0);
    ptr_raps_msg->op_code        = ERPS_RAPS_OP_CODE;
    ptr_raps_msg->flags          = 0;
    ptr_raps_msg->tlv_offset     = 32;
    ptr_raps_msg->req_state_sub_code  = ((ERPS_RAPS_REQ_NR << 4) | 0 << 0);
    ptr_raps_msg->status         = 0;
    osapi_memcpy(ptr_raps_msg->node_id, _mw_mac_addr, ERPS_RAPS_NODE_ID_LEN); /* Node ID */
    ptr_raps_msg->end_tlv        = 0;

    return MW_E_OK;
}

static void
_erps_raps_msg_handle(
    void)
{
    UI8_T                   *ptr_buf = NULL;
    UI32_T                  index = 0, len = 0;
    MW_ERROR_NO_T           rc = MW_E_OK;

    rc = osapi_msgRecv(ERPS_PKT_QUEUE_NAME, (UI8_T **)&index, 0, ERPS_TASK_DELAY);
    if(MW_E_OK == rc)
    {
        ptr_buf = (UI8_T *)pdma_get_ethernet_pkt_buf(index, &len);
        if(NULL != ptr_buf)
        {
            ERPS_LOG_DEBUG("ERPS recv packet msg, ptr_buf:%x, len:%d", (UI32_T)ptr_buf, len);
            _erps_raps_msg_input(ptr_buf, len);
            pdma_release_ethernet_pkt_buf(index);
        }
    }
    return;
}

static void
_erps_raps_msg_input(
    UI8_T *ptr_buf,
    UI32_T len)
{
    UI8_T                       request = 0, status = 0, i = 0;
    UI8_T                       ring_id = 0, recv_port = 0, need_flush_fdb = FALSE;
    UI8_T                       *ptr_data = ptr_buf;
    UI16_T                      vlan_id = 0;
    UI32_T                      u32dat  = 0;
    ERPS_RING_T                 *ptr_ring = NULL;
    ERPS_PORT_T                 *ptr_port = NULL;
    MW_LOG_LEVEL_T              erps_debug_level = MW_LOG_LEVEL_OFF;
    ERPS_RAPS_MSG_T             *ptr_raps_msg = NULL;
    ERPS_NODE_ID_BPR_T          node_id_bpr = {{0}};
    AIR_STAG_RX_PARA_T          stag_rx = {0};
    ERPS_RING_REQUEST_T         ring_request =  ERPS_RING_REQ_LAST;
    void                        *ptr_arg = NULL;


    if(NULL == ptr_buf)
    {
        return;
    }
    /* Dump raw data */
    if ((MW_E_OK == mw_log_get_level(MW_LOG_MODULE_ERPS, &erps_debug_level)) &&
        (MW_LOG_LEVEL_DEBUG <= erps_debug_level))
    {
        MW_LOG_RAW(ERPS, MW_LOG_LEVEL_DEBUG, "Recv RAPS message:\n");
        erps_pkt_data_dump(ptr_buf, len);
    }
    /* Skip ethernet mac header */
    ptr_data += MAC_ADDRESS_LEN * 2;
    /* STAG header */
    u32dat = get32(ptr_data);
    u32dat = htonl(u32dat);
    if(AIR_E_OK !=  air_stag_decodeRxStag(0, (const UI8_T*)&u32dat, sizeof(u32dat), &stag_rx))
    {
        return;
    }
    ptr_data += SIZEOF_STAG_HDR;
    /* Check VLAN header */
    for(i = PKT_FIRST_VLAN_TAG; i < PKT_MAX_VLAN_TAG; i++)
    {
        u32dat = get32(ptr_data);
        if((ETHTYPE_VLAN == BITS_OFF_R(u32dat, 16, 16)) || (0x88A8 == BITS_OFF_R(u32dat, 16, 16)) || (ETHTYPE_QINQ == BITS_OFF_R(u32dat, 16, 16)))
        {
            /* Skip VLAN tag */
            ptr_data += SIZEOF_VLAN_HDR;
            if(PKT_FIRST_VLAN_TAG == i)
            {
                vlan_id = BITS_OFF_R(u32dat, 0, 12);
            }
        }
        else
        {
            break;
        }
    }
    /* Skip ethernet type */
    ptr_data += SIZEOF_ETH_TYPE;
    ptr_raps_msg = (ERPS_RAPS_MSG_T *)ptr_data;
    ring_id = ptr_buf[5];
    ptr_ring = erps_instance_list_search(ring_id, vlan_id);
    if(NULL == ptr_ring)
    {
        ERPS_LOG_WARN("R-APS msg not belong to any instance, ring_id:%d, vlan_id:%d", ring_id, vlan_id);
        return;
    }
    if(0 == osapi_memcmp(ptr_raps_msg->node_id, ptr_ring->node_id, ERPS_RAPS_NODE_ID_LEN))
    {
        ERPS_LOG_DEBUG("Receive own R-APS msg, ring_id:%d, vlan_id:%d", ring_id, vlan_id);
        /* Spec 10.1.1, R-APS message whose node ID field value corresponds to the local node ID are ignored by this process. */
        return;
    }
    recv_port = stag_rx.sp;
    ptr_port = _erps_ring_port_get(ptr_ring, recv_port);
    if(NULL == ptr_port)
    {
        ERPS_LOG_ERROR("Recv R-APS msg from unknown port:%d", recv_port);
        return;
    }
    status  = ptr_raps_msg->status;
    osapi_memcpy(&(node_id_bpr.node_id), ptr_raps_msg->node_id, ERPS_RAPS_NODE_ID_LEN); /* Node ID */
    node_id_bpr.bpr = ((ERPS_RAPS_STATUS_BPR & status) ? 1 : 0);
    request = (ptr_raps_msg->req_state_sub_code >> 4) & 0xF;

    ERPS_LOG_DEBUG("vid:%d, ring_id:%d, mel:%d, ver:%d,req_state:%d, sub_code:%d, status:%d,node_id:%2x-%2x-%2x-%2x-%2x-%2x",
                                        vlan_id,
                                        ring_id,
                                        (ptr_raps_msg->mel_ver >> 5), (ptr_raps_msg->mel_ver & 0x1F),
                                        request, (ptr_raps_msg->req_state_sub_code) & 0xF,
                                        status,
                                        ptr_raps_msg->node_id[0], ptr_raps_msg->node_id[1], ptr_raps_msg->node_id[2],
                                        ptr_raps_msg->node_id[3], ptr_raps_msg->node_id[4], ptr_raps_msg->node_id[5]);
    switch (request)
    {
        case ERPS_RAPS_REQ_NR:
            if(0 != (status & ERPS_RAPS_STATUS_RB))
            {
                ring_request = ERPS_RING_REQ_RAPS_NR_RB;
            }
            else
            {
                ring_request = ERPS_RING_REQ_RAPS_NR;
                ptr_arg = (void *)ptr_raps_msg->node_id;
                /* Spec 10.1.10, the flush logic deletes the current(node ID, BPR) pair on receiving ring port */
                osapi_memset(&(ptr_port->node_id_bpr), 0, sizeof(ERPS_NODE_ID_BPR_T));
            }
            break;
        case ERPS_RAPS_REQ_MS:
            ring_request = ERPS_RING_REQ_RAPS_MS;
            break;
        case ERPS_RAPS_REQ_SF:
            ring_request = ERPS_RING_REQ_RAPS_SF;
            break;
        case ERPS_RAPS_REQ_FS:
            ring_request = ERPS_RING_REQ_RAPS_FS;
            break;

        default:
            ERPS_LOG_DEBUG("Unknown RAPS request: %d", request);
            return;
    }

    if(ERPS_RING_REQ_RAPS_NR != ring_request)
    {
        /* 10.1.10 Flush logic */
        if(0 != osapi_memcmp(&(ptr_port->node_id_bpr), &node_id_bpr, sizeof(ERPS_NODE_ID_BPR_T)))
        {
            ERPS_PORT_T                 *ptr_another_port = NULL;

            osapi_memcpy(&(ptr_port->node_id_bpr), &node_id_bpr, sizeof(ERPS_NODE_ID_BPR_T));
            ptr_another_port = ((&(ptr_ring->east_port) == ptr_port) ? &(ptr_ring->west_port) : &(ptr_ring->east_port));
            /* Compare another port's (node id, bpr) */
            if((0 == (status & ERPS_RAPS_STATUS_DNF)) &&
               (0 != osapi_memcmp(&(ptr_another_port->node_id_bpr), &node_id_bpr, sizeof(ERPS_NODE_ID_BPR_T))))
            {
                need_flush_fdb = TRUE;
            }
        }
    }

    if(TRUE == need_flush_fdb)
    {
        erps_fdb_flush(ptr_ring, ptr_port);
    }
    if(ERPS_INVALID_TIMER_IDX != ptr_ring->guard_timer_idx)
    {
        /* Spec 10.1.5, while the guard timer is running, any received R-APS request/state and status information,
           is blocked and not forwarded to the prioity logic */
        ERPS_LOG_DEBUG("Guard timer already running, ring_id:%d, vlan_id:%d", ring_id, vlan_id);
        return;
    }
    erps_state_machine_handle(ptr_ring, ring_request, ptr_arg);

    return;

}

static void
_erps_db_msg_handle(
    DB_MSG_T *ptr_msg)
{
    UI8_T i = 0;
    DB_REQUEST_TYPE_T request = {0};
    UI16_T data_size = 0;
    UI8_T *ptr_data = NULL;
    UI8_T *ptr_payload_data = NULL;
    MW_ERROR_NO_T rc = MW_E_OK;

    if(NULL != ptr_msg)
    {
        ERPS_LOG_DEBUG("[%d]recv method - %02X", ptr_msg->type.count, ptr_msg->method);
        if(0 != (M_B_RESPONSE & ptr_msg->method))
        {
            ERPS_LOG_DEBUG("Response message!");
        }
        else
        {
            /* Process the notification message */
            do {
                rc = dbapi_parseMsg(ptr_msg, ptr_msg->type.count, &request, &data_size, &ptr_data, &ptr_payload_data);
                if (MW_E_OK == rc)
                {
                    ERPS_LOG_DEBUG("index=%u, ptr_payload=%p, t_idx=%u, f_idx=%u, e_idx=%u, data_size=%u",
                                                            i++,
                                                            ptr_data,
                                                            request.t_idx,
                                                            request.f_idx,
                                                            request.e_idx,
                                                            data_size);

                    rc = _erps_db_msg_process(ptr_msg->method, request, data_size, ptr_data);
                    if (MW_E_OK != rc)
                    {
                        ERPS_LOG_ERROR("Handle DB msg failed!(%d)", rc);
                    }
                }
                /* Continue to parse the next request within the payload. */
            } while ((MW_E_OK == rc) && (NULL != ptr_payload_data));
        }
    }
    else
    {
        ERPS_LOG_ERROR("Message from DB is NULL!");
    }

    return;
}

static MW_ERROR_NO_T
_erps_db_msg_process(
    const UI8_T method,
    const DB_REQUEST_TYPE_T request,
    const UI16_T data_size,
    const void  *ptr_data)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    if((NULL == ptr_data) || (0 == data_size))
    {
        return MW_E_BAD_PARAMETER;
    }
    switch(request.t_idx)
    {
        case ERPS_INFO:
            rc = _erps_handle_db_erps_info(method, request, data_size, ptr_data);
            break;

        default:
            ERPS_LOG_WARN("Unknown DB table %d.", request.t_idx);
            break;
    }

    return rc;
}

#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
static MW_ERROR_NO_T
_erps_port_fast_link_down_init(
    void)
{
    I32_T               rc = AIR_E_OK;
    UI32_T              unit = 0, port = 0;
    const UI32_T        dev_type = 30;
    const UI32_T        reg_addr1 = 0x300, reg_data1 = 0x3F1F;
    const UI32_T        reg_addr2 = 0x301, reg_data2 = 0x3F3F;
    const UI32_T        reg_addr3 = 0x302, reg_data3 = 0x214E;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_GPHY, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        /* HAL API is called temporarily. SDK API will be used in the future. */
        rc =  hal_mdio_writeC45ByPort(unit, port, dev_type, (UI16_T)reg_addr1, (UI16_T)reg_data1);
        rc |= hal_mdio_writeC45ByPort(unit, port, dev_type, (UI16_T)reg_addr2, (UI16_T)reg_data2);
        rc |= hal_mdio_writeC45ByPort(unit, port, dev_type, (UI16_T)reg_addr3, (UI16_T)reg_data3);
        if(AIR_E_OK != rc)
        {
            ERPS_LOG_ERROR("Phy set port-c22 error, rc:%d\n", rc);
            break;
        }
    }

    return rc;
}
#endif /* !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL) */

static MW_ERROR_NO_T
_erps_instance_init(
    ERPS_RING_T             *ptr_ring)
{
    UI8_T                   link_status = 0;
    MW_ERROR_NO_T           rc = MW_E_OK;
    AIR_PORT_BITMAP_T       portmap = {0};

    _erps_instance_param_check_init(ptr_ring);
    rc = _erps_acl_add(ptr_ring);
    osapi_memcpy(ptr_ring->node_id, _mw_mac_addr, MAC_ADDRESS_LEN);

    rc = erps_state_machine_init(ptr_ring);
    ERPS_LOG_DEBUG("Instance init done, rc:%d\n", rc);

    /* Enable ring port link status interrupt */
    AIR_PORT_ADD(portmap, ptr_ring->west_port.port_id);
    AIR_PORT_ADD(portmap, ptr_ring->east_port.port_id);
    erps_port_interrupt_set(portmap, TRUE);

    /* Check port link status */
    _erps_port_link_status_get(ptr_ring->west_port.port_id, &link_status);
    if(0 == link_status)
    {
        ptr_ring->west_port.is_failed = TRUE;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->west_port));
    }
    _erps_port_link_status_get(ptr_ring->east_port.port_id, &link_status);
    if(0 == link_status)
    {
        ptr_ring->east_port.is_failed = TRUE;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->east_port));
    }

    return  rc;
}

static MW_ERROR_NO_T
_erps_instance_update(
    ERPS_RING_T             *ptr_ring)
{
    UI8_T                   link_status = 0;
    UI32_T                  unit = 0;
    I32_T                   rc = 0;
    AIR_ACL_RULE_T          acl_rule;
    AIR_PORT_BITMAP_T       portmap = {0};

    _erps_instance_param_check_init(ptr_ring);
    /* Update acl rule portbmp */
    if(MW_E_OK == mw_acl_mutex_take())
    {
        rc = air_acl_getRule(unit, ptr_ring->ring_acl_id, &acl_rule);
        if (AIR_E_OK == rc)
        {
            AIR_PORT_BITMAP_COPY(portmap, acl_rule.portmap);
            AIR_PORT_BITMAP_CLEAR(acl_rule.portmap);
            AIR_PORT_ADD(acl_rule.portmap, ptr_ring->west_port.port_id);
            AIR_PORT_ADD(acl_rule.portmap, ptr_ring->east_port.port_id);
            air_acl_setRule(unit, ptr_ring->ring_acl_id, &acl_rule);
        }
        else
        {
            ERPS_LOG_WARN("Failed to get ERPS ACL rule entry-id %d rule, rc %d.", ptr_ring->ring_acl_id, rc);
        }
        mw_acl_mutex_release();
    }
    /* Disable ring port link status interrupt */
    erps_port_interrupt_set(portmap, FALSE);
    AIR_PORT_BITMAP_CLEAR(portmap);
    /* Unblock ring port */
    erps_ring_port_unblock(ptr_ring);

    rc = erps_state_machine_init(ptr_ring);

    /* Enable ring port link status interrupt */
    AIR_PORT_ADD(portmap, ptr_ring->west_port.port_id);
    AIR_PORT_ADD(portmap, ptr_ring->east_port.port_id);
    erps_port_interrupt_set(portmap, TRUE);
    ERPS_LOG_DEBUG("Instance update done, rc:%d\n", rc);

    /* Check port link status */
    _erps_port_link_status_get(ptr_ring->west_port.port_id, &link_status);
    if(0 == link_status)
    {
        ptr_ring->west_port.is_failed = TRUE;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->west_port));
    }
    _erps_port_link_status_get(ptr_ring->east_port.port_id, &link_status);
    if(0 == link_status)
    {
        ptr_ring->east_port.is_failed = TRUE;
        erps_state_machine_handle(ptr_ring, ERPS_RING_REQ_SF, &(ptr_ring->east_port));
    }

    return  rc;
}

static MW_ERROR_NO_T
_erps_instance_deinit(
    ERPS_RING_T             *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    AIR_PORT_BITMAP_T       portmap = {0};

    /* Stop WTR, WTB, guard, hold-off timer */
    erps_wtr_timer_stop(ptr_ring);
    erps_wtb_timer_stop(ptr_ring);
    erps_guard_timer_stop(ptr_ring);
    erps_hold_off_timer_stop(ptr_ring);

    /* Disable ring port link status interrupt */
    AIR_PORT_ADD(portmap, ptr_ring->west_port.port_id);
    AIR_PORT_ADD(portmap, ptr_ring->east_port.port_id);
    erps_port_interrupt_set(portmap, FALSE);

    /* Unblock ring port */
    erps_ring_port_unblock(ptr_ring);
    rc |= _erps_acl_del(ptr_ring);
    osapi_memset(ptr_ring, 0, sizeof(ERPS_RING_T));
    ERPS_LOG_DEBUG("Instance deinit done, rc:%d\n", rc);

    return rc;
}

static MW_ERROR_NO_T
_erps_instance_param_check_init(
    ERPS_RING_T             *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    MW_CHECK_PTR(ptr_ring);
    ptr_ring->node_type = ERPS_ROLE_TRANSFER;
    ptr_ring->ptr_rpl_port = NULL;
    if(FALSE == ERPS_WTR_TIMER_TIME_IS_VALID(ptr_ring->wtr_time))
    {
        /* Default WTR time is 5 minutes, step: 1min */
        ptr_ring->wtr_time = 5;
    }
    ptr_ring->wtr_timer_idx = ERPS_INVALID_TIMER_IDX;
    ptr_ring->wtb_timer_idx = ERPS_INVALID_TIMER_IDX;
    if(FALSE == ERPS_HOLD_OFF_TIMER_TIME_IS_VALID(ptr_ring->hold_off_time))
    {
        /* Default hold-off time is 0ms, hold-off time step: 100ms */
        ptr_ring->hold_off_time = 0;
    }
    ptr_ring->hold_off_timer_idx = ERPS_INVALID_TIMER_IDX;
    if(FALSE == ERPS_GUARD_TIMER_TIME_IS_VALID(ptr_ring->guard_time))
    {
        /* Default guard time is 500ms, guard time step: 10ms */
        ptr_ring->guard_time = 50;
    }
    ptr_ring->guard_timer_idx = ERPS_INVALID_TIMER_IDX;

    do
    {
        if(ERPS_ROLE_OWNER == ERPS_PORT_GET_ROLE(&(ptr_ring->west_port)))
        {
            ptr_ring->node_type = ERPS_ROLE_OWNER;
            ptr_ring->ptr_rpl_port = &(ptr_ring->west_port);
            break;
        }
        if(ERPS_ROLE_OWNER == ERPS_PORT_GET_ROLE(&(ptr_ring->east_port)))
        {
            ptr_ring->node_type = ERPS_ROLE_OWNER;
            ptr_ring->ptr_rpl_port = &(ptr_ring->east_port);
            break;
        }
        if(ERPS_ROLE_NEIGHBOR == ERPS_PORT_GET_ROLE(&(ptr_ring->west_port)))
        {
            ptr_ring->node_type = ERPS_ROLE_NEIGHBOR;
            ptr_ring->ptr_rpl_port = &(ptr_ring->west_port);
            break;
        }
        if(ERPS_ROLE_NEIGHBOR == ERPS_PORT_GET_ROLE(&(ptr_ring->east_port)))
        {
            ptr_ring->node_type = ERPS_ROLE_NEIGHBOR;
            ptr_ring->ptr_rpl_port = &(ptr_ring->east_port);
            break;
        }

    } while (0);

    return rc;
}

static MW_ERROR_NO_T
_erps_link_status_polling(
    void)
{
    UI8_T                   index = 0, port_id = 0;
    UI8_T                   link_stats = 0;
    I32_T                   rc = 0;
    ERPS_RING_T             *ptr_ring = NULL;

    for(index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        ptr_ring = erps_instance_get_by_index(index);
        if((NULL != ptr_ring) &&
           (0 != ptr_ring->ring_id) &&
           (0 != ptr_ring->raps_vid))
        {
            /* Only polling not build in GPHY port */
            port_id = ptr_ring->west_port.port_id;
            if(FALSE == MW_PORT_CHK(PLAT_PORT_BMP_GPHY, port_id))
            {
                _erps_port_link_status_get(port_id, &link_stats);
                if(link_stats == ptr_ring->west_port.is_failed)
                {
                    ERPS_LOG_INFO("Port:%d change to %s", port_id, (link_stats ? "LINK UP" : "LINK DOWN"));
                    rc = _erps_link_status_change_process(port_id, link_stats);
                }
            }

            port_id = ptr_ring->east_port.port_id;
            if(FALSE == MW_PORT_CHK(PLAT_PORT_BMP_GPHY, port_id))
            {
                /* Only polling not build in GPHY port */
                _erps_port_link_status_get(port_id, &link_stats);
                if(link_stats == ptr_ring->east_port.is_failed)
                {
                    ERPS_LOG_INFO("Port:%d change to %s", port_id, (link_stats ? "LINK UP" : "LINK DOWN"));
                    rc = _erps_link_status_change_process(port_id, link_stats);
                }
            }
        }
    }

    return rc;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   erps_init
 * PURPOSE:
 *      This ERPS init function.
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
erps_init(void)
{
    UI32_T          port = 0, phy_id = 0, unit = 0;
    MW_ERROR_NO_T   ret = MW_E_OK;

    if(NULL != _erps_ctx.task_handle)
    {
        ERPS_LOG_ERROR("erps task already created\n");
        return MW_E_NOT_INITED;
    }
    MW_LOG_INIT_PRINTF("Initializing ERPS module...\n");
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
    _erps_port_fast_link_down_init();
#endif
    erps_port_interrupt_set(PLAT_PORT_BMP_GPHY, FALSE);
    /* Init PHY id --> AIR port port map */
    AIR_PORT_FOREACH(PLAT_PORT_BMP_GPHY, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        /* HAL API is called temporarily. SDK API will be used in the future. */
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
        if(MAX_PHY_PORT > phy_id)
        {
            _phy_air_port_map[phy_id] = port;
        }
    }
    _erps_raps_msg_buffer_init();
    do
    {
        ret = erps_queue_init();
        if(MW_E_OK != ret)
        {
            break;
        }
        /* Create ERPS task(static) */
        ret = osapi_threadCreateStatic(ERPS_TASK_NAME,
                                    ERPS_TASK_STACK_SIZE,
                                    MW_TASK_PRIORITY_ERPS,
                                    _erps_task,
                                    NULL,
                                    _erps_ctx.task_stack,
                                    &(_erps_ctx.task_tcb),
                                    &(_erps_ctx.task_handle));
        if(MW_E_OK != ret)
        {
            ERPS_LOG_ERROR("Create static thread for ERPS failed, ret:%d.", ret);
            break;
        }
        ret = erps_sw_timer_init();
    }while (0);

    if(MW_E_OK != ret)
    {
        ERPS_LOG_ERROR("Init ERPS failed, ret:%d.", ret);
        erps_resource_free();
        return MW_E_NOT_INITED;
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_resource_free
 * PURPOSE:
 *      Free the resources in ERPS init function.
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
erps_resource_free(void)
{
    threadhandle_t      ptr_erps_task_handle = NULL;

    erps_queue_deinit();
    if(NULL != _erps_ctx.task_handle)
    {
        ptr_erps_task_handle = _erps_ctx.task_handle;
        _erps_ctx.task_handle = NULL;
        osapi_threadDelete(ptr_erps_task_handle);
    }
    erps_sw_timer_deinit();

    return MW_E_OK;
}

/* FUNCTION NAME:   erps_instance_list_search
 * PURPOSE:
 *      This API is used to search erps instance entries.
 *
 * INPUT:
 *      ring_id              -- ring id of the erps instance
 *      vlan_id              -- control vlan id of the erps instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to erps instance.
 *
 * NOTES:
 *      None
 */
ERPS_RING_T *
erps_instance_list_search(
    UI8_T           ring_id,
    UI16_T          vlan_id)
{
    UI8_T       index = 0;
    ERPS_RING_T *ptr_ring = NULL;

    if((FALSE == VLAN_IS_VID_VALID(vlan_id)) ||
       (FALSE == ERPS_RING_ID_IS_VALID(ring_id)))
    {
        return ptr_ring;
    }
    for (; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        if ((ring_id == _erps_ring_instance[index].ring_id) &&
            (vlan_id == _erps_ring_instance[index].raps_vid))
        {
            ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[index]);
            break;
        }
    }
    return ptr_ring;
}

/* FUNCTION NAME:   erps_instance_get_by_index
 * PURPOSE:
 *      This API is used to get erps instance by index.
 *
 * INPUT:
 *      index                -- index of the erps instance
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Pointer to erps instance.
 *
 * NOTES:
 *      None
 */
ERPS_RING_T *
erps_instance_get_by_index(
    UI8_T           index)
{
    ERPS_RING_T *ptr_ring = NULL;

    if(index < ERPS_MAX_INSTANCE_NUM)
    {
        ptr_ring = (ERPS_RING_T *)&(_erps_ring_instance[index]);
    }
    return ptr_ring;
}

/* FUNCTION NAME:   erps_port_interrupt_set
 * PURPOSE:
 *      This API is used to set port interrupt state.
 *
 * INPUT:
 *      portmap              -- bit map of ports which need to enable/disable interrupt
 *      is_enable            -- TRUE for enable, FALSE for disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK.
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_port_interrupt_set(
    AIR_PORT_BITMAP_T   portmap,
    UI8_T               is_enable)
{
    UI8_T               port_id = 0, phy_id_bitmap = 0;
    UI32_T              phy_id = 0, unit = 0;

    AIR_PORT_FOREACH(portmap, port_id)
    {
        if(PLAT_CPU_PORT == port_id)
        {
            /* Skip cpu port */
            continue;
        }
        if(TRUE == MW_PORT_CHK(PLAT_PORT_BMP_GPHY, port_id))
        {
            HAL_AIR_PORT_TO_PHY_PORT(unit, port_id, phy_id);
            if(phy_id < MAX_PHY_PORT)
            {
                phy_id_bitmap |= BIT(phy_id);
            }
        }
        else
        {
#ifdef AIR_SUPPORT_SFP
            if ((TRUE == sfp_port_is_serdesPort(unit, port_id)) ||
                (TRUE == sfp_port_is_comboPort(unit, port_id)))
            {
                UI8_T   rx_los_pin = SFP_PIN_PIONUM_INVALID;

                sfp_pin_io_getPIONum(unit, port_id, SFP_PIN_TYPE_RX_LOSS, &rx_los_pin);
                if(SFP_PIN_PIONUM_INVALID == rx_los_pin)
                {
                    continue;
                }
                erps_gpio_rx_loss_interrupt_set(port_id, rx_los_pin, is_enable);
                ERPS_LOG_INFO("port_id: %d, rx_los_pin: %d, is_enable: %d", port_id, rx_los_pin, is_enable);
            }
            else
#endif
            {
                /* Skip other ports */
                continue;
            }
        }
    }
    if(0 != phy_id_bitmap)
    {
        erps_gsw_isr_port_set(phy_id_bitmap, is_enable);
    }
    ERPS_LOG_INFO("portmap=0x%X, phy_id_bitmap=0x%X, is_enable=%d", portmap[0], phy_id_bitmap, is_enable);
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_check_buffer_is_raps
 * PURPOSE:
 *      Check if buffer is raps message or not.
 *
 * INPUT:
 *      ptr_raw_data         -- Pointer to raw data
 *      len                  -- Pength of raw data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Address of RAPS message packet header if it's a valid RAPS message, otherwise NULL.
 *
 * NOTES:
 *      None
 */
UI8_T *
erps_check_buffer_is_raps(
    const void     *ptr_raw_data,
    UI32_T          len)
{
    UI8_T           i = 0, *ptr_data = NULL;
    UI8_T           raps_format[5] = {0x01, 0x19, 0xA7, 0x00, 0x00};
    UI16_T          eth_type = 0;
    UI32_T          u32dat  = 0;
    ERPS_RAPS_MSG_T *ptr_raps_msg = NULL;

    ptr_data = (UI8_T *)ptr_raw_data;
    if(NULL != ptr_raw_data)
    {
        if(MW_E_OK != erps_available_instance_check())
        {
            /* No available ERPS instance */
            return NULL;
        }
        if(0 != osapi_memcmp(ptr_data, raps_format, sizeof(raps_format)))
        {
            return NULL;
        }
        /* Skip ethernet mac header */
        ptr_data += MAC_ADDRESS_LEN * 2;
        /* Skip STAG header */
        ptr_data += SIZEOF_STAG_HDR;
        /* Check VLAN header */
        for(i = PKT_FIRST_VLAN_TAG; i < PKT_MAX_VLAN_TAG; i++)
        {
            u32dat = get32(ptr_data);
            if((ETHTYPE_VLAN == BITS_OFF_R(u32dat, 16, 16)) || (0x88A8 == BITS_OFF_R(u32dat, 16, 16)) || (ETHTYPE_QINQ == BITS_OFF_R(u32dat, 16, 16)))
            {
                /* Skip VLAN tag */
                ptr_data += SIZEOF_VLAN_HDR;
            }
            else
            {
                break;
            }
        }
        /* Check ethernet type */
        eth_type = get16(ptr_data);
        if(ERPS_ETHTYPE_RAPS != eth_type)
        {
            return NULL;
        }
        ptr_data += SIZEOF_ETH_TYPE;
        ptr_raps_msg = (ERPS_RAPS_MSG_T *)ptr_data;
        if(ERPS_RAPS_OP_CODE == ptr_raps_msg->op_code)
        {
            return ptr_data;
        }
    }

    return NULL;
}

/* FUNCTION NAME:   erps_check_port_is_ring_port
 * PURPOSE:
 *      Check if port is in ring.
 *
 * INPUT:
 *      port_id              -- Pength of raw data
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_check_port_is_ring_port(
    const UI32_T    port_id)
{
    UI8_T                   index = 0;
    ERPS_RING_T             *ptr_ring = NULL;
    MW_ERROR_NO_T           rc = MW_E_ENTRY_NOT_FOUND;

    for(index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        ptr_ring = erps_instance_get_by_index(index);
        if((NULL != ptr_ring) &&
           (0 != ptr_ring->ring_id) &&
           (0 != ptr_ring->raps_vid))
        {
            if((port_id == ptr_ring->west_port.port_id) ||
               (port_id == ptr_ring->east_port.port_id))
            {
                rc = MW_E_OK;
                break;
            }
            else
            {
                /* Port is not ring port */
                ERPS_LOG_WARN("Port:%d is not ring(%d) port", port_id, ptr_ring->ring_id);
                continue;
            }
        }
    }

    return rc;
}

/* FUNCTION NAME:   erps_fdb_flush
 * PURPOSE:
 *      This function is used to flush the FDB entry for the specified port.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      ptr_port             -- Pointer to the erps port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_fdb_flush(
    ERPS_RING_T     *ptr_ring,
    ERPS_PORT_T     *ptr_port)
{
    UI32_T unit = 0;
    AIR_ERROR_NO_T  rc = AIR_E_OK;

    UNUSED(ptr_port);
    MW_CHECK_PTR(ptr_ring);

    /* Spec 9.6, An FDB flush consists of removing MAC address learned on the ring ports of the protected Ethernet ring
           from the Ethernet ring node's FDB */
    rc = air_l2_flushMacAddr(unit, AIR_L2_MAC_FLUSH_TYPE_PORT, ptr_ring->west_port.port_id);
    rc |= air_l2_flushMacAddr(unit, AIR_L2_MAC_FLUSH_TYPE_PORT, ptr_ring->east_port.port_id);
    ERPS_LOG_INFO("Port:%d, %d flush FDB, rc:%d", ptr_ring->west_port.port_id, ptr_ring->east_port.port_id, rc);
#if defined(AIR_SUPPORT_IGMP_SNP) && defined(AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE)
    /* When IGMP SNP is use MAC based, need to update the MAC table */
    MW_PORT_BITMAP_T portbmp = {0};

    MW_PORT_ADD(portbmp, ptr_ring->west_port.port_id);
    MW_PORT_ADD(portbmp, ptr_ring->east_port.port_id);
    igmp_snp_msg_updatePortGroupEntry(portbmp);
#endif /* AIR_SUPPORT_IGMP_SNP_SEARCH_MAC_TABLE */

    return MW_E_OK;
}

/* FUNCTION NAME:   erps_port_block
 * PURPOSE:
 *      This function is used to block or unblock the specified port.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      ptr_port             -- Pointer to the erps port
 *      is_block             -- TRUE: block the port, FALSE: unblock the port
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_port_block(
    ERPS_RING_T     *ptr_ring,
    ERPS_PORT_T     *ptr_port,
    UI8_T           is_block)
{
    UI8_T               port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
    UI8_T               f_idx = WEST_PORT_STATE, port_state = ERPS_PORT_STATE_DISCARD;
    UI32_T              unit = 0, fid = 0;
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    ERPS_PORT_T         *ptr_another_port = NULL;
    AIR_STP_STATE_T     state = AIR_STP_STATE_LISTEN;
    AIR_ACL_ACTION_T    action;


    MW_CHECK_PTR(ptr_ring);
    MW_CHECK_PTR(ptr_port);
    if(((TRUE == is_block) && (ERPS_PORT_STATE_DISCARD == ptr_port->port_state)) ||
       ((FALSE == is_block) && (ERPS_PORT_STATE_FORWARD == ptr_port->port_state)))
    {
        ERPS_LOG_DEBUG("Port[%d] already %s.", ptr_port->port_id, (is_block ? "blocked" : "unblocked"));
        return MW_E_OK;
    }
    if(TRUE == is_block)
    {
        port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
        state = AIR_STP_STATE_LISTEN;
        port_state = ERPS_PORT_STATE_DISCARD;
    }
    else
    {
        port_fw = MW_ACL_ACT_PORT_FW_CPU_ONLY;
        ptr_another_port = ((&(ptr_ring->east_port) == ptr_port) ? &(ptr_ring->west_port) : &(ptr_ring->east_port));
        if(ERPS_PORT_STATE_FORWARD == ptr_another_port->port_state)
        {
            port_fw = MW_ACL_ACT_PORT_FW_CPU_INCLUDE;
        }
        state = AIR_STP_STATE_FORWARD;
        port_state = ERPS_PORT_STATE_FORWARD;
    }
    /* Block control VLAN */
    if(MW_E_OK == mw_acl_mutex_take())
    {
        osapi_memset(&action, 0, sizeof(AIR_ACL_ACTION_T));
        rc = air_acl_getAction(unit, ptr_ring->ring_acl_id, &action);
        if (AIR_E_OK == rc)
        {
            action.port_fw = port_fw;
            rc = air_acl_setAction(unit, ptr_ring->ring_acl_id, &action);
        }
        if (AIR_E_OK != rc)
        {
            ERPS_LOG_WARN("Failed to get/set ERPS ACL rule entry-id %d action, rc %d.", ptr_ring->ring_acl_id, rc);
        }
        mw_acl_mutex_release();
    }

    fid = ptr_ring->db_idx;
    /* Block data VLAN */
    rc = air_stp_setPortState(unit, ptr_port->port_id, fid, state);
    if (AIR_E_OK != rc)
    {
        ERPS_LOG_WARN("Failed to set STP port state, rc %d.", rc);
    }
    ptr_port->port_state = port_state;

    if(ERPS_PORT_STATE_DISCARD == port_state)
    {
        /* 10.1.10 Flush logic: When the ring port is changed to be blocked, clear (node_id, bpr) pair on both ring ports */
        osapi_memset(&(ptr_ring->west_port.node_id_bpr), 0, sizeof(ERPS_NODE_ID_BPR_T));
        osapi_memset(&(ptr_ring->east_port.node_id_bpr), 0, sizeof(ERPS_NODE_ID_BPR_T));
#ifdef AIR_SUPPORT_IGMP_SNP
        IGMP_SNP_MSG_CLEAR_ENTRY_T clear_entry;

        osapi_memset(&clear_entry, 0, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));
        clear_entry.type = IGMP_SNP_ENTRY_FLUSH_TYPE_PORT;
        MW_PORT_ADD(clear_entry.portbmp, ptr_port->port_id);
        igmp_snp_msg_clearEntry((const IGMP_SNP_MSG_CLEAR_ENTRY_T *)&clear_entry);
        ERPS_LOG_DEBUG("Port:%d is blocked, clear igmp snp port group entry.", ptr_port->port_id);
#endif /* AIR_SUPPORT_IGMP_SNP */
    }

    if(ERPS_PORT_ATTR_EAST_PORT == ERPS_PORT_GET_ATTR(ptr_port))
    {
        f_idx = EAST_PORT_STATE;
    }
    erps_queue_db_send(ERPS_MW_MSG_QUEUE_NAME, M_UPDATE, ERPS_INFO, f_idx, ptr_ring->db_idx, &port_state, 0);
    ERPS_LOG_INFO("%s port:%d, rc:%d", (is_block ? "block" : "unblock"), ptr_port->port_id ,rc);

    return  MW_E_OK;
}

/* FUNCTION NAME:   erps_non_fail_port_unblock
 * PURPOSE:
 *      This function is used to unblock non fail erps ports.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_non_fail_port_unblock(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    MW_CHECK_PTR(ptr_ring);
    if(FALSE == ptr_ring->west_port.is_failed)
    {
        rc = erps_port_block(ptr_ring, &ptr_ring->west_port, FALSE);
        ERPS_LOG_DEBUG("Unblock west port, rc:%d", rc);
    }
    if(FALSE == ptr_ring->east_port.is_failed)
    {
        rc |= erps_port_block(ptr_ring, &ptr_ring->east_port, FALSE);
        ERPS_LOG_DEBUG("Unblock east port, rc:%d", rc);
    }
    return rc;
}

/* FUNCTION NAME:   erps_ring_port_unblock
 * PURPOSE:
 *      This function is used to unblock erps ports.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_ring_port_unblock(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    rc = erps_port_block(ptr_ring, &ptr_ring->west_port, FALSE);
    rc |= erps_port_block(ptr_ring, &ptr_ring->east_port, FALSE);
    ERPS_LOG_DEBUG("Unblock ring port, rc:%d", rc);

    return rc;
}

/* FUNCTION NAME:   erps_wtr_timer_start
 * PURPOSE:
 *      This function is used to start WTR timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtr_timer_start(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    UI32_T                  time_ms = 0;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->wtr_timer_idx)
    {
        ERPS_LOG_WARN("WTR timer already started");
        return MW_E_ENTRY_EXISTS;
    }
    /* WTR time step: 1min */
    time_ms = ptr_ring->wtr_time * 60 * 1000;
    rc = erps_hw_timer_start(&(ptr_ring->wtr_timer_idx),
                    time_ms,
                    (void *)ptr_ring,
                    _erps_wtr_timer_expire);

    ptr_ring->local_request = ERPS_RING_REQ_WTR_RUNNING;
    ERPS_LOG_INFO("Start WTR timer, rc:%d.", rc);
    return rc;
}

/* FUNCTION NAME:   erps_wtr_timer_stop
 * PURPOSE:
 *      This function is used to stop WTR timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtr_timer_stop(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->wtr_timer_idx)
    {
        rc = erps_hw_timer_stop(ptr_ring->wtr_timer_idx, NULL);
        if(MW_E_OK == rc)
        {
            ptr_ring->wtr_timer_idx = ERPS_INVALID_TIMER_IDX;
        }
        ERPS_LOG_INFO("Stop WTR timer, rc:%d.", rc);
    }
    return rc;
}

/* FUNCTION NAME:   erps_wtb_timer_start
 * PURPOSE:
 *      This function is used to start WTB timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtb_timer_start(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    UI32_T                  time_ms = 0;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->wtb_timer_idx)
    {
        ERPS_LOG_WARN("WTB timer already started");
        return MW_E_ENTRY_EXISTS;
    }
    /* WTB time is guard time + 5s, Guard time step: 10ms */
    time_ms = ptr_ring->guard_time * 10 + 5000;
    rc = erps_hw_timer_start(&(ptr_ring->wtb_timer_idx),
                    time_ms,
                    (void *)ptr_ring,
                    _erps_wtb_timer_expire);
    ptr_ring->local_request = ERPS_RING_REQ_WTB_RUNNING;
    ERPS_LOG_INFO("Start WTB timer, rc:%d.", rc);
    return rc;
}

/* FUNCTION NAME:   erps_wtb_timer_stop
 * PURPOSE:
 *      This function is used to stop WTB timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_wtb_timer_stop(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->wtb_timer_idx)
    {
        rc = erps_hw_timer_stop(ptr_ring->wtb_timer_idx, NULL);
        if(MW_E_OK == rc)
        {
            ptr_ring->wtb_timer_idx = ERPS_INVALID_TIMER_IDX;
        }
        ERPS_LOG_INFO("Stop WTB timer, rc:%d.", rc);
    }
    return rc;
}

/* FUNCTION NAME:   erps_hold_off_timer_start
 * PURPOSE:
 *      This function is used to start hold-off timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_hold_off_timer_start(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    UI32_T                  time_ms = 0;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->hold_off_timer_idx)
    {
        ERPS_LOG_WARN("Hold-off timer already started");
        return MW_E_ENTRY_EXISTS;
    }
    /* Hold-off time step: 100ms */
    time_ms = ptr_ring->hold_off_time * 100;

    rc = erps_hw_timer_start(&(ptr_ring->hold_off_timer_idx),
                    time_ms,
                    (void *)ptr_ring,
                    _erps_hold_off_timer_expire);

    ERPS_LOG_INFO("Start hold-off timer, rc:%d.", rc);
    return rc;
}

/* FUNCTION NAME:   erps_hold_off_timer_stop
 * PURPOSE:
 *      This function is used to stop hold-off timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_hold_off_timer_stop(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->hold_off_timer_idx)
    {
        rc = erps_hw_timer_stop(ptr_ring->hold_off_timer_idx, NULL);
        if(MW_E_OK == rc)
        {
            ptr_ring->hold_off_timer_idx = ERPS_INVALID_TIMER_IDX;
        }
        ERPS_LOG_INFO("Stop hold-off timer, rc:%d.", rc);
    }
    return rc;
}

/* FUNCTION NAME:   erps_guard_timer_start
 * PURPOSE:
 *      This function is used to start guard timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_ENTRY_EXISTS
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_guard_timer_start(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;
    UI32_T                  time_ms = 0;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->guard_timer_idx)
    {
        ERPS_LOG_WARN("Guard timer already started");
        return MW_E_ENTRY_EXISTS;
    }
    /* Guard time step: 10ms */
    time_ms = ptr_ring->guard_time * 10;
    rc = erps_hw_timer_start(&(ptr_ring->guard_timer_idx),
                    time_ms,
                    (void *)ptr_ring,
                    _erps_guard_timer_expire);

    ERPS_LOG_INFO("Start guard timer, rc:%d.", rc);
    return rc;
}

/* FUNCTION NAME:   erps_guard_timer_stop
 * PURPOSE:
 *      This function is used to stop guard timer for ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OP_INCOMPLETE
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_guard_timer_stop(
    ERPS_RING_T     *ptr_ring)
{
    MW_ERROR_NO_T           rc = MW_E_OK;

    if(ERPS_INVALID_TIMER_IDX != ptr_ring->wtb_timer_idx)
    {
        rc = erps_hw_timer_stop(ptr_ring->guard_timer_idx, NULL);
        if(MW_E_OK == rc)
        {
            ptr_ring->guard_timer_idx = ERPS_INVALID_TIMER_IDX;
        }
        ERPS_LOG_INFO("Stop guard timer, rc:%d.", rc);
    }
    return rc;
}

/* FUNCTION NAME:   erps_tx_start
 * PURPOSE:
 *      This function is used to start transmit on the ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
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
erps_tx_start(
    ERPS_RING_T             *ptr_ring)
{
    if(FALSE == ptr_ring->is_send_raps)
    {
        ptr_ring->is_send_raps = TRUE;
        ptr_ring->send_raps_time_remain = ERPS_SEND_RAPS_MSG_INTERVAL;
        erps_sw_timer_start();
        ERPS_LOG_DEBUG("Ring:%d start tx.", ptr_ring->ring_id);
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_tx_stop
 * PURPOSE:
 *      This function is used to stop transmit on the ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
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
erps_tx_stop(
    ERPS_RING_T             *ptr_ring)
{
    UI8_T index = 0;
    UI8_T is_stop_sw_timer = TRUE;

    if(TRUE == ptr_ring->is_send_raps)
    {
        ptr_ring->is_send_raps = FALSE;
        ptr_ring->send_raps_time_remain = 0;
        ptr_ring->raps_request = 0;
        ptr_ring->raps_status = 0;
        ERPS_LOG_DEBUG("Ring:%d stop tx.", ptr_ring->ring_id);
        for (; index < ERPS_MAX_INSTANCE_NUM; index++)
        {
            if((0 != _erps_ring_instance[index].ring_id) &&
                (0 != _erps_ring_instance[index].raps_vid) &&
                (TRUE == _erps_ring_instance[index].is_send_raps))
            {
                is_stop_sw_timer = FALSE;
                break;
            }
        }
        if(TRUE == is_stop_sw_timer)
        {
            /* No instance need to send RAPS message */
            erps_sw_timer_stop();
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_raps_msg_send
 * PURPOSE:
 *      This function is used to send RAPS message.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      request              -- Request type
 *      status               -- Status type
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_OP_INCOMPLETE
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_raps_msg_send(
    ERPS_RING_T             *ptr_ring,
    ERPS_RAPS_REQUEST_T     request,
    UI8_T                   status)
{
    UI8_T                       raps_msg_burst = 1;
    UI32_T                      u32dat = 0, len = sizeof(u32dat);
    MW_LOG_LEVEL_T              erps_debug_level = MW_LOG_LEVEL_OFF;
    AIR_ERROR_NO_T              air_rc = AIR_E_OK;
    ERPS_RAPS_MSG_T             *ptr_raps_msg = NULL;
    AIR_STAG_TX_PARA_T          stag_tx = {0};
    struct netif                *ptr_pxNetIf = netif_default;

    /* Ring ID */
    _erps_raps_msg_tx_buffer[5] = (ptr_ring->ring_id & 0xff);
    osapi_memcpy((_erps_raps_msg_tx_buffer + MAC_ADDRESS_LEN), _mw_mac_addr, MAC_ADDRESS_LEN);

    /* Stag header */
    u32dat = 0;
    stag_tx.opc = AIR_STAG_OPC_PORTMAP;
    stag_tx.pbm[0] = (1 << ptr_ring->west_port.port_id) | (1 << ptr_ring->east_port.port_id);     /* Send port bit map */
    air_rc = air_stag_encodeTxStag(0, AIR_STAG_MODE_INSERT, &stag_tx, (UI8_T *)&u32dat, (UI32_T *)&len);
    if (air_rc != AIR_E_OK)
    {
        ERPS_LOG_ERROR("Stag encode fail!, air_rc:%d", air_rc);
        return MW_E_OP_INCOMPLETE;
    }
    osapi_memcpy((void *)(_erps_raps_msg_tx_buffer + MAC_ADDRESS_LEN * 2), &u32dat, SIZEOF_STAG_HDR);

    /* VLAN header, num:1 */
    u32dat = 0;
    u32dat |= (ETHTYPE_VLAN << 16);         /* TPID */
    u32dat |= (7 << 13);                    /* User priority */
    u32dat |= (0 << 12);                    /* CFI */
    u32dat |= ((ptr_ring->raps_vid) << 0);  /* Instance VLAN ID */
    u32dat = lwip_htonl(u32dat);
    osapi_memcpy((void *)(_erps_raps_msg_tx_buffer + MAC_ADDRESS_LEN * 2 + SIZEOF_STAG_HDR), &u32dat, SIZEOF_VLAN_HDR);

    /* R-APS message */
    ptr_raps_msg = (ERPS_RAPS_MSG_T *)(_erps_raps_msg_tx_buffer + (MAC_ADDRESS_LEN * 2) + SIZEOF_STAG_HDR + SIZEOF_VLAN_HDR + SIZEOF_ETH_TYPE);
    ptr_raps_msg->req_state_sub_code  = ((request << 4) | 0 << 0);
    ptr_raps_msg->status         = status;
    osapi_memcpy(ptr_raps_msg->node_id, ptr_ring->node_id, ERPS_RAPS_NODE_ID_LEN);

    if((request != ptr_ring->raps_request) ||
           (status  != ptr_ring->raps_status))
    {
        raps_msg_burst = 3;
        ptr_ring->raps_status  = status;
        ptr_ring->raps_request = request;
        ptr_ring->send_raps_time_remain = ERPS_SEND_RAPS_MSG_INTERVAL;
    }
    ERPS_LOG_DEBUG("Send RAPS message to portbmp 0x%x, request %d, status 0x%x, burst %d.\n",
                                            stag_tx.pbm[0],
                                            request,
                                            status,
                                            raps_msg_burst);

    while (raps_msg_burst > 0)
    {
        erpsLowLevelOutput(ptr_pxNetIf, _erps_raps_msg_tx_buffer, ERPS_RAPS_TX_BUFFER_LEN);
        raps_msg_burst --;
    }

    /* Dump raw data */
    if ((MW_E_OK == mw_log_get_level(MW_LOG_MODULE_ERPS, &erps_debug_level)) &&
        (MW_LOG_LEVEL_DEBUG <= erps_debug_level))
    {
        MW_LOG_RAW(ERPS, MW_LOG_LEVEL_DEBUG, "Send RAPS message:\n");
        erps_pkt_data_dump(_erps_raps_msg_tx_buffer, ERPS_RAPS_TX_BUFFER_LEN);
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_ring_state_set
 * PURPOSE:
 *      This function is used to set the state of ERPS ring.
 *
 * INPUT:
 *      ptr_ring             -- Pointer to the ERPS ring
 *      state                -- State of ERPS ring
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
erps_ring_state_set(
    ERPS_RING_T             *ptr_ring,
    ERPS_RING_STATE_T       state)
{
    if(state != ptr_ring->ring_state)
    {
        ptr_ring->ring_state = state;
        ERPS_LOG_INFO("New ring state:%s", erps_record_ring_state_vec[ptr_ring->ring_state]);
        /* Update state to DB */
        erps_queue_db_send(ERPS_MW_MSG_QUEUE_NAME, M_UPDATE, ERPS_INFO, INSTANCE_STATE, ptr_ring->db_idx, &(ptr_ring->ring_state), 0);
    }
    return MW_E_OK;
}

/* FUNCTION NAME:   erps_available_instance_check
 * PURPOSE:
 *      This API is used to check if there is available erps instance.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
erps_available_instance_check(
    void)
{
    UI8_T       index = 0;
    ERPS_RING_T *ptr_ring = NULL;

    for(index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        ptr_ring = erps_instance_get_by_index(index);
        if((NULL != ptr_ring) && (0 != ptr_ring->ring_id) && (0 != ptr_ring->raps_vid))
        {
            return MW_E_OK;
        }
    }
    return MW_E_ENTRY_NOT_FOUND;
}
