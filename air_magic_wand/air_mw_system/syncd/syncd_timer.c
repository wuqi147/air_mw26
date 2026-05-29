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

/* FILE NAME:  syncd_timer.c
 * PURPOSE:
 *  Implement timers of synchronized daemon.
 *
 * NOTES:
 *
 */

#include <string.h>
#include "syncd.h"
#include "syncd_in.h"
#include "syncd_timer.h"
#include "db_api.h"

#include "osapi_timer.h"
#include "osapi_string.h"
#include "mw_utils.h"   /* for MW_CHECK_PTR */
#include "air_error.h"  /* for AIR_ERROR_NO_T */
#include "air_port.h"
#include "air_l2.h"
#include "air_swc.h"
#ifdef AIR_SUPPORT_SNMP
#include "lwip/apps/snmp.h"
#endif /* AIR_SUPPORT_SNMP */
#include "lwip/netifapi.h"
#include <syncd_api_l2.h>
#include <syncd_api_stp.h>
#ifdef AIR_SUPPORT_IGMP_SNP
#include "igmp_snoop_msg.h"
#endif /* AIR_SUPPORT_IGMP_SNP */
#include <syncd_api_vlan.h>
#include <syncd_api_port.h>
#ifdef AIR_SUPPORT_SFP
#include "sfp_util.h"
#include "sfp_port.h"
#endif
#include "air_lag.h"


/* NAMING CONSTANT DECLARATIONS
*/
#define SYNCD_MIB_COUNTER_MIN_DB_PAYLOAD_SIZE    (54)
/* MACRO FUNCTION DECLARATIONS
 */
#ifdef AIR_SUPPORT_SNMP
#define LOW_32_BIT (0xffffffffULL)
#define HIGH_32_BIT (LOW_32_BIT << 32)
#endif

/* DATA TYPE DECLARATIONS
*/
typedef struct SYNCD_PORT_RESUME_MATRIX_S
{
    UI8_T   resume_flag : 1;
    UI8_T   resume_time : 7;
} ATTRIBUTE_PACK SYNCD_PORT_RESUME_MATRIX_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
static void
_syncd_stp_flush_msg_handle(
    void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static SYNCD_MIB_HC_T *_syncd_mib_hc = NULL;
static UI32_T _syncd_timer_flags = BIT(SYNCD_TMR_ENABLE_BIT_LAST) - 1;
#ifdef AIR_SUPPORT_RSTP
static UI32_T _syncd_rstp_flush_flag = 0;
#endif
#ifdef AIR_SUPPORT_MSTP
static UI16_T _syncd_mstp_flush_flag[MAX_VLAN_ENTRY_NUM];
#endif
static SYNCD_PORT_RESUME_MATRIX_T  *_ptr_syncd_port_resume_timer = NULL;
/* LOCAL SUBPROGRAM BODIES
 */

/* Transform mib counter format from SDK style to syncd style */
UI64_T
_update_HCounter(
    UI64_T dst,
    UI32_T src)
{
    UI64_T ret = dst;
    UI32_T Lc = (UI32_T)(dst & SYNCD_MIB_HC_MASK);
    UI32_T Hc = (UI32_T)((UI64_T)dst >> SYNCD_MIB_HC_OFFT);

    if(Lc != src)
    {
        if(Lc > src)
        {
            ret = (((UI64_T)Hc << SYNCD_MIB_HC_OFFT) + SYNCD_MIB_HC_CNT) + src;
        }
        else
        {
            ret = ((UI64_T)Hc << SYNCD_MIB_HC_OFFT) + src;
        }
    }

    return ret;
}

void
_syncd_timer_mib64(
    SYNCD_MIB_HC_T      *ptr_hc,
    AIR_MIB_CNT_RX_T    *ptr_rx,
    AIR_MIB_CNT_TX_T    *ptr_tx)
{
    if(FALSE == ptr_hc->clrFlag)
    {
#ifdef AIR_LITE_MW
        ptr_hc->rx_ucp = _update_HCounter(ptr_hc->rx_ucp, ptr_rx->RUPC);
        ptr_hc->rx_mcp = _update_HCounter(ptr_hc->rx_mcp, ptr_rx->RMPC);
        ptr_hc->rx_bcp = _update_HCounter(ptr_hc->rx_bcp, ptr_rx->RBPC);

        ptr_hc->tx_ucp = _update_HCounter(ptr_hc->tx_ucp, ptr_tx->TUPC);
        ptr_hc->tx_mcp = _update_HCounter(ptr_hc->tx_mcp, ptr_tx->TMPC);
        ptr_hc->tx_bcp = _update_HCounter(ptr_hc->tx_bcp, ptr_tx->TBPC);
#else
        ptr_hc->rx_64 = _update_HCounter(ptr_hc->rx_64, ptr_rx->RL64PC);
        ptr_hc->rx_65 = _update_HCounter(ptr_hc->rx_65, ptr_rx->RL65PC);
        ptr_hc->rx_128 = _update_HCounter(ptr_hc->rx_128, ptr_rx->RL128PC);
        ptr_hc->rx_256 = _update_HCounter(ptr_hc->rx_256, ptr_rx->RL256PC);
        ptr_hc->rx_512 = _update_HCounter(ptr_hc->rx_512, ptr_rx->RL512PC);
        ptr_hc->rx_1024 = _update_HCounter(ptr_hc->rx_1024, ptr_rx->RL1024PC);
        ptr_hc->rx_1519 = _update_HCounter(ptr_hc->rx_1519, ptr_rx->RL1519PC);

        ptr_hc->tx_64 = _update_HCounter(ptr_hc->tx_64, ptr_tx->TL64PC);
        ptr_hc->tx_65 = _update_HCounter(ptr_hc->tx_65, ptr_tx->TL65PC);
        ptr_hc->tx_128 = _update_HCounter(ptr_hc->tx_128, ptr_tx->TL128PC);
        ptr_hc->tx_256 = _update_HCounter(ptr_hc->tx_256, ptr_tx->TL256PC);
        ptr_hc->tx_512 = _update_HCounter(ptr_hc->tx_512, ptr_tx->TL512PC);
        ptr_hc->tx_1024 = _update_HCounter(ptr_hc->tx_1024, ptr_tx->TL1024PC);
        ptr_hc->tx_1519 = _update_HCounter(ptr_hc->tx_1519, ptr_tx->TL1519PC);

#ifdef AIR_SUPPORT_SNMP
        ptr_hc->rx_ucp = _update_HCounter(ptr_hc->rx_ucp, ptr_rx->RUPC);
        ptr_hc->rx_mcp = _update_HCounter(ptr_hc->rx_mcp, ptr_rx->RMPC);
        ptr_hc->rx_bcp = _update_HCounter(ptr_hc->rx_bcp, ptr_rx->RBPC);

        ptr_hc->tx_ucp = _update_HCounter(ptr_hc->tx_ucp, ptr_tx->TUPC);
        ptr_hc->tx_mcp = _update_HCounter(ptr_hc->tx_mcp, ptr_tx->TMPC);
        ptr_hc->tx_bcp = _update_HCounter(ptr_hc->tx_bcp, ptr_tx->TBPC);
#endif
#endif
    }
    else
    {
        memset(ptr_hc, 0, sizeof(SYNCD_MIB_HC_T));
    }
}

void
_syncd_timer_mib_transform(
    DB_MIB_CNT_T        *ptr_cnt,
    SYNCD_MIB_HC_T      *ptr_hc,
    AIR_MIB_CNT_RX_T    *ptr_rx,
    AIR_MIB_CNT_TX_T    *ptr_tx)
{
    /* Rx packets */
#ifdef AIR_LITE_MW
    ptr_cnt ->rx_packets =
        ptr_hc->rx_ucp
        + ptr_hc->rx_mcp
        + ptr_hc->rx_bcp;
#else
    ptr_cnt ->rx_packets =
          ptr_hc->rx_64
        + ptr_hc->rx_65
        + ptr_hc->rx_128
        + ptr_hc->rx_256
        + ptr_hc->rx_512
        + ptr_hc->rx_1024
        + ptr_hc->rx_1519;
#endif
    /* Rx octets */
    ptr_cnt ->rx_octets = ptr_rx ->ROC;
    /* Rx errors */
    ptr_cnt ->rx_errors =
          (UI64_T)ptr_rx ->RAEPC
        + (UI64_T)ptr_rx ->RCEPC
        + (UI64_T)ptr_rx ->RUSPC
        + (UI64_T)ptr_rx ->RFEPC
        + (UI64_T)ptr_rx ->ROSPC
        + (UI64_T)ptr_rx ->RJEPC;
#ifdef AIR_SUPPORT_SNMP
    ptr_cnt ->rx_unicast_pkts = ptr_hc->rx_ucp;
    ptr_cnt ->rx_multicast_pkts = ptr_hc->rx_mcp;
    ptr_cnt ->rx_broadcast_pkts = ptr_hc->rx_bcp;
    ptr_cnt ->rx_discards_pkts = (UI64_T)ptr_rx ->RDPC;
#endif
    /* Tx packets */
#ifdef AIR_LITE_MW
    ptr_cnt ->tx_packets =
        ptr_hc->tx_ucp
        + ptr_hc->tx_mcp
        + ptr_hc->tx_bcp;
#else
    ptr_cnt ->tx_packets =
          ptr_hc->tx_64
        + ptr_hc->tx_65
        + ptr_hc->tx_128
        + ptr_hc->tx_256
        + ptr_hc->tx_512
        + ptr_hc->tx_1024
        + ptr_hc->tx_1519;
#endif
    /* Tx octets */
    ptr_cnt ->tx_octets = ptr_tx ->TOC;
    /* Tx errors */
    ptr_cnt ->tx_errors = (UI64_T)ptr_tx ->TCEPC;
#ifdef AIR_SUPPORT_SNMP
    ptr_cnt ->tx_unicast_pkts = ptr_hc->tx_ucp;
    ptr_cnt ->tx_multicast_pkts = ptr_hc->tx_mcp;
    ptr_cnt ->tx_broadcast_pkts = ptr_hc->tx_bcp;
    ptr_cnt ->tx_discards_pkts = (UI64_T)ptr_tx ->TCDPC + (UI64_T)ptr_tx ->TODPC;
#endif
}

/* Syncd base timer polling function */
static void
_syncd_base_timer(
    TimerHandle_t h_timer)
{
    MW_ERROR_NO_T rc;
    SYNCD_CFG_T *ptr_cfg;
    SYNCD_MSG_BASE_TIMER_T *ptr_msg = NULL;

    rc = osapi_timerIdGet(h_timer, (UI32_T *)(&ptr_cfg));
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Get ID. of timer failed");
        return;
    }
    if (MW_E_OK == osapi_calloc(sizeof(SYNCD_MSG_BASE_TIMER_T), SYNCD_NAME, (void **)&ptr_msg))
    {
        ptr_msg->msg_id = MW_MSG_ID_SYNCD_TIMER_BASE_TIMER_EXPIRED_NOTI;
        rc = syncd_queue_task_sendMsg((MW_MSG_T *)ptr_msg, TRUE);
        if (MW_E_OK != rc)
        {
            SYNCD_LOG_DEBUG("send base timer msg failed, msg_id:%d", ptr_msg->msg_id);
            MW_FREE(ptr_msg);
        }
    }
    else
    {
        SYNCD_LOG_DEBUG("allocate base timer msg memory failed");
    }
}

/* MIB counter polling function */
static void
_syncd_mib_counter(
    TimerHandle_t h_timer)
{
    MW_ERROR_NO_T rc;
    SYNCD_CFG_T *ptr_cfg;
    MW_MSG_T *ptr_cnt_msg = NULL;

    rc = osapi_timerIdGet(h_timer, (UI32_T *)(&ptr_cfg));
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Get ID. of timer failed");
        return;
    }
    if (MW_E_OK == osapi_calloc(sizeof(MW_MSG_T), SYNCD_NAME, (void **)&ptr_cnt_msg))
    {
        ptr_cnt_msg->msg_id = MW_MSG_ID_SYNCD_TIMER_MIB_COUNTER_TIMER_EXPIRED_NOTI;
        rc = syncd_queue_task_sendMsg(ptr_cnt_msg, TRUE);
        if (MW_E_OK != rc)
        {
            SYNCD_LOG_DEBUG("send MIB counter msg failed, msg_id:%d, rc:%d", ptr_cnt_msg->msg_id, rc);
            MW_FREE(ptr_cnt_msg);
        }
    }
    else
    {
        SYNCD_LOG_DEBUG("allocate MIB counter msg memory failed");
    }
}

#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
static void
_syncd_stp_flush_msg_handle(
    void)
{
    UI32_T unit = 0;
    UI32_T i = 0;
    MW_ERROR_NO_T rc = MW_E_OK;
#ifdef AIR_SUPPORT_RSTP
    MW_PORT_BITMAP_T portbmp = {0};
#endif
#ifdef AIR_SUPPORT_IGMP_SNP
    IGMP_SNP_MSG_CLEAR_ENTRY_T clear_entry;
#endif

#ifdef AIR_SUPPORT_RSTP
    if (0 != _syncd_rstp_flush_flag)
    {
        for (; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (FALSE == BIT_CHK(_syncd_rstp_flush_flag, i))
            {
                continue;
            }

            rc = syncd_api_flushMacAddr(unit, i+1, SYNCD_API_L2_FLUSH_PORT);
            if (MW_E_OK == rc)
            {
                syncd_rstp_set_flush_flag(i+1, FALSE);

                MW_PORT_ADD(portbmp, i+1);
            }
        }
#ifdef AIR_SUPPORT_IGMP_SNP
        osapi_memset(&clear_entry, 0, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));
        clear_entry.type = IGMP_SNP_ENTRY_FLUSH_TYPE_PORT;
        osapi_memcpy(clear_entry.portbmp, portbmp, sizeof(MW_PORT_BITMAP_T));
        igmp_snp_msg_clearEntry((const IGMP_SNP_MSG_CLEAR_ENTRY_T *)&clear_entry);
#endif
        return;
    }
#endif

#ifdef AIR_SUPPORT_MSTP
    for (i = 0; i < MAX_VLAN_ENTRY_NUM; i++)
    {
        if (0 == _syncd_mstp_flush_flag[i])
        {
            continue;
        }

        rc = syncd_api_flushMacAddr(unit, _syncd_mstp_flush_flag[i], SYNCD_API_L2_FLUSH_VLAN);
        if (MW_E_OK == rc)
        {
#ifdef AIR_SUPPORT_IGMP_SNP
            osapi_memset(&clear_entry, 0, sizeof(IGMP_SNP_MSG_CLEAR_ENTRY_T));
            clear_entry.type = IGMP_SNP_ENTRY_FLUSH_TYPE_VID;
            clear_entry.vid = _syncd_mstp_flush_flag[i];
            igmp_snp_msg_clearEntry((const IGMP_SNP_MSG_CLEAR_ENTRY_T *)&clear_entry);
#endif
            _syncd_mstp_flush_flag[i] = 0;
        }
    }
#endif
}
#endif

static void
_syncd_timer_callback(
    TimerHandle_t h_timer)
{
    static UI8_T mib_counter_flag = 0;

    _syncd_base_timer(h_timer);
    mib_counter_flag ++;
    if (mib_counter_flag >= (SYNCD_TMR_MIB_PERIOD / SYNCD_TMR_PS_PERIOD))
    {
        mib_counter_flag = 0;
        _syncd_mib_counter(h_timer);
    }
}

/* Internal initialize timer structure */
static MW_ERROR_NO_T
_syncd_timer_init(
    SYNCD_TIMER_T   *ptr_tmr,
    C8_T *          name,
    UI32_T          period,
    SYNCD_CFG_T     *ptr_cfg,
    timeCbk_t       func)
{
    MW_ERROR_NO_T rc;

    MW_CHECK_PTR(name);
    MW_CHECK_PTR(ptr_tmr);
    MW_CHECK_PTR(ptr_cfg);

    SYNCD_LOG_DEBUG("name=%s", name);
    SYNCD_LOG_DEBUG("ptr_tmr=%p", ptr_tmr);
    SYNCD_LOG_DEBUG("ptr_cfg=%p", ptr_cfg);
    SYNCD_LOG_DEBUG("func=%p", func);

    /* Set configuration */
    SYNCD_ASPRINTF(ptr_tmr ->name, name);
    ptr_tmr ->period = period;
    SYNCD_LOG_DEBUG("ptr_tmr ->name=%s", ptr_tmr ->name);
    SYNCD_LOG_DEBUG("ptr_tmr ->period=%u", ptr_tmr ->period);

    rc = osapi_calloc(sizeof(SYNCD_PORT_RESUME_MATRIX_T) * PLAT_MAX_PORT_NUM, SYNCD_NAME, (void **)&_ptr_syncd_port_resume_timer);
    if (MW_E_OK != rc)
    {
        return rc;
    }
    /* Create new timer to check port status */
    rc = osapi_timerCreate(
            ptr_tmr ->name,         /* Name of timer module */
            func,                   /* Timer callback function */
            TRUE,                   /* reload */
            ptr_tmr ->period,       /* The period of the timer (unit: ms)*/
            ptr_cfg,                /* timerid */
            &(ptr_tmr ->th));       /* timer handle */
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Create timer[%s] failed", ptr_tmr ->name);
        MW_FREE(_ptr_syncd_port_resume_timer);
        return rc;
    }

    /* Start Timer*/
    rc = osapi_timerStart(ptr_tmr ->th);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Start timer[%s] failed", ptr_tmr ->name);
        MW_FREE(_ptr_syncd_port_resume_timer);
        return rc;
    }

    return rc;
}

/* Internal free timer structure */
static MW_ERROR_NO_T
_syncd_timer_free(
    SYNCD_TIMER_T *ptr_tmr)
{
    MW_CHECK_PTR(ptr_tmr);

    SYNCD_LOG_DEBUG("Free Timer:%s", ptr_tmr ->name);

    /* Delete timer */
    osapi_timerDelete(ptr_tmr ->th);

    /* free name */
    MW_FREE(ptr_tmr ->name);

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   syncd_port_status_handle
 * PURPOSE
 *       Handle port status.
 *
 * INPUT:
 *      ptr_cfg       --  A pointer points to port status msg
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_TIMEOUT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_port_status_handle(
    SYNCD_CFG_T *ptr_cfg)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    AIR_PORT_STATUS_T p_state;
    UI32_T unit = 0, i_array = 0;
    UI32_T u_port = 0;
    BOOL_T admin_state;
    BOOL_T lp_block;
    int fc_cur = 0, fc_cfg = 0, link_cur = 0, link_cfg = 0;
    struct netif *ptr_netif = netif_find_default();
    static u32_t link_status_bitmap = 0; /* bit value: 1, link up; 0, link down. */
    DB_MSG_T *ptr_msg = NULL;
    UI16_T    msg_size = 0;
    UI8_T    *ptr_payload = NULL;
    UI16_T    payload_size = DB_MSG_PAYLOAD_SIZE + 1;
    UI8_T     need_send = FALSE;

    ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);
    if (NULL == ptr_msg)
    {
        SYNCD_LOG_ERROR("Create msg failed!");
        return MW_E_NO_MEMORY;
    }

    for (u_port = 1; u_port <= PLAT_MAX_PORT_NUM; u_port++)
    {
        if(!(_syncd_timer_flags & SYNCD_TMR_ENABLE_PORT))
        {
            continue;
        }
        i_array = u_port - 1;
        if (MW_E_OK != syncd_api_getPortAdmin(i_array, &admin_state))
        {
            SYNCD_LOG_ERROR("syncd_api_getPortAdmin(%d) failed", i_array);
            continue;
        }
        if (MW_E_OK != syncd_api_getPortLpBlock(i_array, &lp_block))
        {
            SYNCD_LOG_ERROR("syncd_api_getPortLpBlock(%d) failed", i_array);
            continue;
        }
        if (!(ptr_cfg ->ptr_port[i_array].port_status.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) &&
                ( (FALSE == admin_state) || (TRUE == lp_block) ) )
        {
            /* Do not detect port status if admin state & current state are disable */
            continue;
        }

        memset(&p_state, 0, sizeof(AIR_PORT_STATUS_T));

        /* Get current port status */
#ifdef AIR_SUPPORT_SFP
        if ((TRUE == sfp_port_is_serdesPort(unit, u_port)) ||
            (TRUE == sfp_port_is_comboPort(unit, u_port)))
        {
            air_rc = sfp_port_getPortStatus(unit, u_port, &p_state);
        }
        else
#endif
        {
            air_rc = air_port_getPortStatus(unit, u_port, &p_state);
        }

        if(AIR_E_OK != air_rc)
        {
            SYNCD_LOG_DEBUG("P[%02u] Get link status failed", u_port);
            continue;
        }

        link_cur = (p_state.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) ? 1 : 0;
        link_cfg = (ptr_cfg ->ptr_port[i_array].port_status.flags & AIR_PORT_STATUS_FLAGS_LINK_UP) ? 1 : 0;
        fc_cur = (p_state.flags & (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)) ? 1 : 0;
        fc_cfg = (ptr_cfg ->ptr_port[i_array].port_status.flags & (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)) ? 1 : 0;
        if (NULL != ptr_netif)
        {
            if ((0 == link_cfg) && (1 == link_cur))
            {
                /* A new port is link up. */
                link_status_bitmap |= (0x01 << (u_port - 1));
                if (FALSE == netif_is_flag_set(ptr_netif, NETIF_FLAG_LINK_UP))
                {
                    netifapi_netif_set_link_up(ptr_netif);
                    SYNCD_LOG_DEBUG("netif_set_link_up port_id:%d\n", u_port);
                }
            }
            else if ((1 == link_cfg) && (0 == link_cur))
            {
                /* A port is link down. */
                link_status_bitmap &= ~(0x01 << (u_port - 1));
                if ((0 == link_status_bitmap) && (TRUE == netif_is_flag_set(ptr_netif, NETIF_FLAG_LINK_UP)))
                {
                    netifapi_netif_set_link_down(ptr_netif);
                    SYNCD_LOG_DEBUG("netif_set_link_down port_id:%d\n", u_port);
                }
            }
        }

        /* TODO: Update current flow control status, but EN8851 does not support */
        /* Send request to DB if port status changed */
        if (TRUE == SYNCD_OPER_CHECK_CHANGE(need_send,
                    link_cfg,
                    link_cur,
                    PORT_OPER_INFO, PORT_OPER_STATUS, u_port, &ptr_msg, &msg_size, &ptr_payload))
        {
            if (link_cur)
            {
                ptr_cfg ->ptr_port[i_array].port_status.flags |= AIR_PORT_STATUS_FLAGS_LINK_UP;
            }
            else
            {
                ptr_cfg ->ptr_port[i_array].port_status.flags &= ~AIR_PORT_STATUS_FLAGS_LINK_UP;
#ifdef AIR_SUPPORT_SFP
                if ((TRUE == sfp_port_is_serdesPort(unit, u_port)) ||
                    (TRUE == sfp_port_is_comboSerdesPort(unit, u_port)))
                {
                    AIR_PORT_T  lag_id = 0;
                    AIR_ERROR_NO_T air_ret = AIR_E_OK;
                    UI32_T min_trunk_port = u_port;
                    UI32_T max_group_cnt = 0, max_member_cnt = 0, member_cnt = 0, member_idx = 0;
                    UI32_T *ptr_member = NULL;
                    BOOL_T  b_trunk_is_link_down = TRUE;

                    /* The HW cannot sense that sfp port is link down, so need flush the dynamic mac address in MW side when sfp port is link down */
                    if(MW_E_OK != syncd_api_check_port_is_in_lag(u_port, &lag_id))
                    {
                        /* sfp port is not in the any trunk */
                        syncd_api_flushMacAddr(unit, u_port, SYNCD_API_L2_FLUSH_PORT);
                    }
                    else
                    {
                        /* check trunk's other port link status */
                        do
                        {
                            air_ret = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
                            if(AIR_E_OK != air_ret)
                            {
                                break;
                            }
                            osapi_calloc(sizeof(UI32_T) * max_member_cnt, "syncd", (void**)&ptr_member);
                            if (NULL != ptr_member)
                            {
                                air_ret = air_lag_getMember(unit, lag_id, &member_cnt, ptr_member);
                                if (AIR_E_OK == air_ret)
                                {
                                    min_trunk_port = ptr_member[0];
                                    for (member_idx = 0; member_idx < member_cnt; member_idx++)
                                    {
                                        if(u_port == ptr_member[member_idx])
                                        {
                                            continue;
                                        }
                                        if(0 != (AIR_PORT_STATUS_FLAGS_LINK_UP & (ptr_cfg->ptr_port[(ptr_member[member_idx] - 1)].port_status.flags)))
                                        {
                                            b_trunk_is_link_down = FALSE;
                                            break;
                                        }
                                    }
                                }
                                MW_FREE(ptr_member);
                            }
                        } while (0);

                        if(TRUE == b_trunk_is_link_down)
                        {
                            SYNCD_LOG_DEBUG("Trunk is link down, clear min_trunk_port:%d mac addr", min_trunk_port);
                            syncd_api_flushMacAddr(unit, min_trunk_port, SYNCD_API_L2_FLUSH_PORT);
                        }
                    }
                }
#endif /* AIR_SUPPORT_SFP */
            }
            SYNCD_LOG_DEBUG("Interface[%02u].link=%u", u_port, p_state.flags);
        }
        if (TRUE == SYNCD_OPER_CHECK_CHANGE(need_send,
                    ptr_cfg ->ptr_port[i_array].port_status.speed,
                    p_state.speed,
                    PORT_OPER_INFO, PORT_OPER_SPEED, u_port, &ptr_msg, &msg_size, &ptr_payload))
        {
            SYNCD_LOG_DEBUG("Interface[%02u].duplex=%u", u_port, p_state.duplex);
        }
        if (TRUE == SYNCD_OPER_CHECK_CHANGE(need_send,
                    ptr_cfg ->ptr_port[i_array].port_status.duplex,
                    p_state.duplex,
                    PORT_OPER_INFO, PORT_OPER_DUPLEX, u_port, &ptr_msg, &msg_size, &ptr_payload))
        {
            SYNCD_LOG_DEBUG("Interface[%02u].speed=%u", u_port, p_state.speed);
        }
        if (TRUE == SYNCD_OPER_CHECK_CHANGE(need_send,
                    fc_cfg,
                    fc_cur,
                    PORT_OPER_INFO, PORT_OPER_FLOW_CTRL, u_port, &ptr_msg, &msg_size, &ptr_payload))
        {
            if (fc_cur)
            {
                ptr_cfg ->ptr_port[i_array].port_status.flags |= (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX);
            }
            else
            {
                ptr_cfg ->ptr_port[i_array].port_status.flags &= ~(AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX);
            }
            SYNCD_LOG_DEBUG("Interface[%02u].flowCtrl=%u", u_port, p_state.flags);
        }
    }

    if ((TRUE == need_send) && (NULL != ptr_msg))
    {
        rc = syncd_queue_db_setMultiData(ptr_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
    }
    else
    {
        MW_FREE(ptr_msg);
    }

    return rc;
}

/* FUNCTION NAME:   syncd_stp_flush_fdb_handle
 * PURPOSE
 *       Handle stp flush fdb.
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
syncd_stp_flush_fdb_handle(
    void)
{
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    _syncd_stp_flush_msg_handle();
#endif
}

/* FUNCTION NAME:   syncd_port_matrix_resume_handle
 * PURPOSE
 *       Handle port matrix resuming.
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
syncd_port_matrix_resume_handle(
    void)
{
    struct netif *ptr_netif = netif_find_default();
    AIR_PORT_BITMAP_T   port_bitmap = {0};
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
    UI32_T seed = 0;
    UI8_T  rand_val = 0;
#endif
    UI8_T  port = 0;

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }

        if((NULL != ptr_netif) && (TRUE == netif_is_flag_set(ptr_netif, NETIF_FLAG_LINK_UP)) &&
           (FALSE == _ptr_syncd_port_resume_timer[(port - 1)].resume_flag))
        {
#if !defined(AIR_LITE_MW) && !defined(AIR_EN_CORAL)
            seed = (UI32_T)osapi_rand();
            rand_val = (seed % SYNCD_TMR_RESUME_PERIOD) + 1;
            _ptr_syncd_port_resume_timer[(port - 1)].resume_time = rand_val + 1;
#else
            _ptr_syncd_port_resume_timer[(port - 1)].resume_time = SYNCD_TMR_RESUME_PERIOD + 1;
#endif
            _ptr_syncd_port_resume_timer[(port - 1)].resume_flag = TRUE;
        }

        if(0 < _ptr_syncd_port_resume_timer[(port - 1)].resume_time)
        {
            _ptr_syncd_port_resume_timer[(port - 1)].resume_time--;
            if(0 == _ptr_syncd_port_resume_timer[(port - 1)].resume_time)
            {
                osapi_memset(port_bitmap, 0, sizeof(AIR_PORT_BITMAP_T));
                AIR_PORT_ADD(port_bitmap, port);
                syncd_api_update_port_matrix(port_bitmap);
                _ptr_syncd_port_resume_timer[(port - 1)].resume_flag = TRUE;
            }
        }
    }
}

/* FUNCTION NAME:   syncd_mib_counter_msg_handle
 * PURPOSE
 *      Polling port mib counter.
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
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_mib_counter_msg_handle(
    void)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    AIR_ERROR_NO_T air_rc = AIR_E_OK;
    AIR_PORT_STATUS_T port_status;
    UI32_T unit = 0, i_array = 0, u_port = 1;
    AIR_MIB_CNT_RX_T mib_rx;
    AIR_MIB_CNT_TX_T mib_tx;
    DB_MIB_CNT_T cnt;
    BOOL_T clear_flag = FALSE, need_send_msg = FALSE;
    UI16_T msg_size = 0;
    UI8_T *ptr_payload = NULL;
    UI16_T payload_size = SYNCD_MIB_COUNTER_MIN_DB_PAYLOAD_SIZE;
    DB_REQUEST_TYPE_T request;
    DB_MSG_T *ptr_msg = NULL;
#if SYNCD_DEBUG_ON
    TickType_t tickstart = 0, tickend = 0, checked_port = 0;
#endif

#if SYNCD_DEBUG_ON
    tickstart = osapi_sysTickGet();
#endif

    ptr_msg = dbapi_createMsg(NULL, M_UPDATE, 1, payload_size, &msg_size, &ptr_payload);

    if (NULL != ptr_msg)
    {
        for (u_port = 1; PLAT_MAX_PORT_NUM >= u_port; u_port++)
        {
            if(!(_syncd_timer_flags & SYNCD_TMR_ENABLE_MIB) || NULL == _syncd_mib_hc)
            {
                continue;
            }
            i_array = u_port - 1;

            /* Get new MIB counter */
            air_rc = air_mib_getPortCnt(unit, u_port, &mib_rx, &mib_tx);
            if(AIR_E_OK != air_rc)
            {
                SYNCD_LOG_DEBUG("P[%02u] Get MIB Rx/Tx counter failed", u_port);
                continue;
            }

            /* Transform SDK MIB counter to DB format */
            clear_flag = _syncd_mib_hc[i_array].clrFlag;
            _syncd_timer_mib64(&_syncd_mib_hc[i_array], &mib_rx, &mib_tx);
            _syncd_timer_mib_transform(&cnt, &_syncd_mib_hc[i_array], &mib_rx, &mib_tx);
            air_rc = air_port_getPortStatus(unit, u_port, &port_status);
            if(AIR_E_OK != air_rc)
            {
                SYNCD_LOG_DEBUG("P[%02u] Get Port Status failed", u_port);
                continue;
            }

            if (FALSE == clear_flag && (0 == (port_status.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)))
            {
                continue;
            } /* Update MIB counter when MIB counter is cleared now or port is link-up state*/

            request.t_idx = MIB_CNT;
            request.f_idx = DB_ALL_FIELDS;
            request.e_idx = u_port;
            rc = dbapi_appendMsgPayload(&request, (UI8_T *)&cnt, &ptr_msg, &msg_size, &ptr_payload);
            if (MW_E_OK != rc)
            {
                break;
            }
            else if (FALSE == need_send_msg)
            {
                need_send_msg = TRUE;
            }
#if SYNCD_DEBUG_ON
            checked_port++;
#endif
        }

        if (TRUE == need_send_msg && MW_E_OK == rc && NULL != ptr_msg)
        {
            rc = syncd_queue_db_setMultiData(ptr_msg, SYNCD_QUEUE_DB_NONBLOCK_WITHOUTRSP);
        }
        else
        {
            MW_FREE(ptr_msg);
        }
    }

#if SYNCD_DEBUG_ON
    tickend = osapi_sysTickGet();
    SYNCD_LOG_DEBUG("Total port:%d: tickstart:%d; tickend:%d; consumption:%d; tick_per_ms:%d", checked_port, tickstart, tickend, tickend - tickstart, portTICK_PERIOD_MS);
#endif

    return rc;
}

/* FUNCTION NAME:   syncd_timer_init
 * PURPOSE
 *      Initialize all timer in syncd.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_timer_init(
    SYNCD_CFG_T *ptr_cfg)
{
    MW_ERROR_NO_T rc;

    MW_CHECK_PTR(ptr_cfg);
    SYNCD_LOG_DEBUG("ptr_cfg=%p", ptr_cfg);

    /* base timer */
    SYNCD_LOG_DEBUG("func=%p", _syncd_base_timer);
    rc = _syncd_timer_init(
        &(ptr_cfg ->tmr),
        SYNCD_TMR_PS_NAME,
        SYNCD_TMR_PS_PERIOD,
        ptr_cfg,
        _syncd_timer_callback);
    if (MW_E_OK != rc)
    {
        return MW_E_NOT_INITED;
    }

    /* mib conuter */
    rc = osapi_calloc(
            sizeof(SYNCD_MIB_HC_T) * PLAT_MAX_PORT_NUM,
            SYNCD_NAME,
            (void **)&_syncd_mib_hc);
    if (MW_E_OK != rc)
    {
        SYNCD_LOG_ERROR("Allocate 64 bit mib counter(size:%d) failed!", sizeof(SYNCD_MIB_HC_T) * PLAT_MAX_PORT_NUM);
        return MW_E_NOT_INITED;
    }

#ifdef AIR_SUPPORT_MSTP
    osapi_memset(_syncd_mstp_flush_flag, 0, sizeof(_syncd_mstp_flush_flag));
#endif
    return MW_E_OK;
}

/* FUNCTION NAME:   syncd_timer_free
 * PURPOSE:
 *      Delete all timer in syncd.
 *
 * INPUT:
 *      ptr_cfg     --  global config pointer of syncd.
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
syncd_timer_free(
    SYNCD_CFG_T *ptr_cfg)
{
    _syncd_timer_free(&(ptr_cfg ->tmr));
    return;
}

/* FUNCTION NAME:   syncd_timer_setFlag
 * PURPOSE:
 *      Set flags of syncd timer.
 *
 * INPUT:
 *      flags       --  Enable bits of timer in SyncD
 *      state       --  State of timer in SyncD
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
syncd_timer_setFlag(
    const UI32_T flags,
    const BOOL_T state)
{
    MW_PARAM_CHK((flags & ~(BIT(SYNCD_TMR_ENABLE_BIT_LAST) - 1)), MW_E_BAD_PARAMETER);
    if (TRUE == state)
    {
        _syncd_timer_flags |= flags;
    }
    else
    {
        _syncd_timer_flags &= ~flags;
    }
    SYNCD_LOG_DEBUG("flag=0x%lX", _syncd_timer_flags);
    return MW_E_OK;
}

/* FUNCTION NAME:   syncd_mib_clear_high_32bit
 * PURPOSE:
 *      Delete high 32bit of mib counter in syncd.
 *
 * INPUT:
 *      port                 -- Index of port number
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
syncd_mib_set_clear_flag(
     UI32_T port)
{
    if(NULL != _syncd_mib_hc)
    {
        _syncd_mib_hc[port - 1].clrFlag = TRUE;
    }
}

#ifdef AIR_SUPPORT_RSTP
/* FUNCTION NAME:   syncd_rstp_set_flush_flag
 * PURPOSE:
 *      Set port's flush flag.
 *
 * INPUT:
 *      port                 -- Index of port number
 *      flag                 -  set---TRUE,unset---FALSE
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
syncd_rstp_set_flush_flag(
     UI32_T port,
     UI8_T  flag)
{
    if (TRUE == flag)
    {
        BIT_SET(_syncd_rstp_flush_flag, port - 1);
    }
    else if (FALSE == flag)
    {
        _syncd_rstp_flush_flag &= ~((1 << (port - 1)));
    }
}
#endif

#ifdef AIR_SUPPORT_MSTP
/* FUNCTION NAME:   syncd_mstp_set_flush_flag
 * PURPOSE:
 *      Set vlan flush flag.
 *
 * INPUT:
 *      ptr_vid              -  VLANs
 *      num                  -  num of VLANs
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
syncd_mstp_set_flush_flag(
     UI16_T *ptr_vid,
     UI32_T num)
{
    UI32_T    i, j;

    for (i = 0; i < num; i++)
    {
        for (j = 0; j < MAX_VLAN_ENTRY_NUM; j++)
        {
            if (ptr_vid[i] == _syncd_mstp_flush_flag[j])
            {
                break;
            }

            if (0 == _syncd_mstp_flush_flag[j])
            {
                 _syncd_mstp_flush_flag[j] = ptr_vid[i];
                 break;
            }
        }
    }
}
#endif

