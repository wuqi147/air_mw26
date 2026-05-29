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

/* FILE NAME:   poe_watchdog.c
 * PURPOSE:
 *      Define poe watchdog function.
 *
 * NOTES:
 */
#ifdef AIR_SUPPORT_POE_WATCHDOG

/* INCLUDE FILE DECLARATIONS
 */
#include <poe_watchdog.h>
#include <poe_main.h>
#include <poe_info.h>
#include <poe_db_relate.h>
#include <poe_api.h>
#include <air_port.h>
#include <air_mib.h>
#include <switch.h>
/* NAMING CONSTANT DECLARATIONS
*/
/* MACRO FUNCTION DECLARATIONS
 */
/* GLOBAL VARIABLE DECLARATIONS
*/
/* STATIC VARIABLE DECLARATIONS
*/
/* LOCAL SUBPROGRAM DECLARATIONS
*/
static UI64_T
_poe_watchdog_getRxPkt(
    const UI8_T           unit,
    const UI8_T           port,
    AIR_MIB_CNT_RX_T     *ptr_rx_cnt,
    AIR_MIB_CNT_TX_T     *ptr_tx_cnt);

static void
_poe_watchdog_timerHandler(
    const UI32_T     dur_tick,
    POE_CB_T        *ptr_poe_control_block);
/* LOCAL SUBPROGRAM BODIES
*/
/* FUNCTION NAME:   _poe_watchdog_getRxPkt
 * PURPOSE:
 *      Calculate the number of received packets.
 * INPUT:
 *      unit          -- unit id
 *      port          -- port id
 * OUTPUT:
 *      None
 * RETURN:
 *      UI64_T        -- number of received packets
 * NOTES:
 *      None
 */
static UI64_T
_poe_watchdog_getRxPkt(
    const UI8_T           unit,
    const UI8_T           port,
    AIR_MIB_CNT_RX_T     *ptr_rx_cnt,
    AIR_MIB_CNT_TX_T     *ptr_tx_cnt)
{
    UI8_T                 rc = MW_E_OK;
    UI32_T                rx_pkt_cnt = 0;

    rc = air_mib_getPortCnt(unit, port , ptr_rx_cnt, ptr_tx_cnt);
    if (AIR_E_OK != rc)
    {
        POE_LOG_ERROR("Get u:%d/p:%d mib counter fail", unit, port);
    }
    else
    {
        rx_pkt_cnt = ptr_rx_cnt->RUPC + ptr_rx_cnt->RMPC + ptr_rx_cnt->RBPC;
    }

    return rx_pkt_cnt;
}

/* FUNCTION NAME:   _poe_watchdog_timerHandler
 * PURPOSE:
 *      PoE watchdog timer handler function.
 * INPUT:
 *      dur_tick                  -- Duration in system ticks
 * OUTPUT:
 *      ptr_poe_control_block     -- Pointer to the related PoE data
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static void
_poe_watchdog_timerHandler(
    const UI32_T     dur_tick,
    POE_CB_T        *ptr_poe_control_block)
{
    UI8_T unit = 0, port = 0;

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < AIR_MAX_PORT_NUM; port++)
        {
            if (NULL == PORT_INFO(ptr_poe_control_block, unit, port))
            {
                continue;
            }
            if (PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer > 0)
            {
                if (PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer >= dur_tick)
                {
                    PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer -= dur_tick;
                }
                else
                {
                    PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer = 0;
                }
            }
        }
    }
}
/* EXPORTED SUBPROGRAM BODIES
*/
/* FUNCTION NAME:   poe_watchdog_handler
 * PURPOSE:
 *      PoE Watchdog handler function.
 * INPUT:
 *      dur_tick                   -- Duration in system ticks
 * OUTPUT:
 *      ptr_poe_control_block      -- Pointer to the related PoE data
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
poe_watchdog_handler(
    const UI32_T     dur_tick,
    POE_CB_T        *ptr_poe_control_block)
{
    UI8_T                 rc = MW_E_OK;
    UI8_T                 unit = 0, port = 0;
    UI64_T                rx_packet = 0;
    BOOL_T                get_mib_cnt = FALSE;
    POE_PORT_ALT_T        alt = POE_PORT_ALT_NONE;
    POE_PORT_OP_STATUS_T  status = {0};
    C8_T                  rx_pkt_diff_string[21] = { 0 };
    AIR_MIB_CNT_RX_T     *ptr_rx_cnt = NULL;
    AIR_MIB_CNT_TX_T     *ptr_tx_cnt = NULL;

    /* update the watchdog timer */
    _poe_watchdog_timerHandler(dur_tick, ptr_poe_control_block);

    ptr_poe_control_block->poe_mib_counter_dur += dur_tick;
    if (ptr_poe_control_block->poe_mib_counter_dur >= POE_WATCHDOG_MIB_CNT_POLLING_INTERVAL)
    {
        rc = osapi_calloc(sizeof(AIR_MIB_CNT_RX_T), POE_APP_TASK_NAME, (void **)&ptr_rx_cnt);
        rc |= osapi_calloc(sizeof(AIR_MIB_CNT_TX_T), POE_APP_TASK_NAME, (void **)&ptr_tx_cnt);
        if (MW_E_OK != rc)
        {
            POE_LOG_ERROR("Failed to allocate memory for ptr_rx_cnt or ptr_tx_cnt\n");
        }
        else
        {
            get_mib_cnt = TRUE;
            ptr_poe_control_block->poe_mib_counter_dur = 0;
        }
    }

    for (unit = 0; unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM; unit++)
    {
        if (NULL == UNIT_INFO(ptr_poe_control_block, unit))
        {
            continue;
        }
        for (port = 0; port < AIR_MAX_PORT_NUM; port++)
        {
            if (NULL == PORT_INFO(ptr_poe_control_block, unit, port))
            {
                continue;
            }
            if (TRUE == PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_mode)
            {
                rc = poe_getPortStatus(unit, port, alt, &status);
                if (MW_E_OK != rc)
                {
                    POE_LOG_ERROR("get port status error(%d)", rc);
                    continue;
                }
                if (POE_PORT_POWER_STATUS_ON == status.power_status)
                {
                    if ((FALSE == PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_pre_mode) ||
                        (TRUE == ptr_poe_control_block->poe_watchdog_updated))
                    {
                        PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff = 0;
                        PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer =
                            ptr_poe_control_block->poe_watchdog_period;
                    }
                    if (FALSE == AIR_PORT_CHK(UNIT_INFO(ptr_poe_control_block, unit)->wd_pwr_status_on_pbmp, port))
                    {
                        POE_LOG_INFO("u:%d/p:%d power on",
                                       unit, port);
                        AIR_PORT_ADD(UNIT_INFO(ptr_poe_control_block, unit)->wd_pwr_status_on_pbmp, port);
                        PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer =
                            ptr_poe_control_block->poe_watchdog_period;
                    }

                    if (TRUE == get_mib_cnt)
                    {
                        rx_packet = _poe_watchdog_getRxPkt(unit, port, ptr_rx_cnt, ptr_tx_cnt);
                        if (rx_packet >= PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt)
                        {
                            PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff =
                                rx_packet - PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt;
                        }
                        else
                        {
                            PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff =
                                UINT64_MAX - PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt + rx_packet + 1;
                        }
                        PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt = rx_packet;
                        if (PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff >
                            (UI64_T)ptr_poe_control_block->poe_watchdog_threshold)
                        {
                            POE_LOG_INFO(
                                "u:%d/p:%d received %s packet(s) and reload Watchdog timer",
                                unit, port,
                                _printUI64_T(PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff,
                                                rx_pkt_diff_string,
                                                sizeof(rx_pkt_diff_string)));
                            PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer =
                                ptr_poe_control_block->poe_watchdog_period;
                            PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff = 0;
                        }
                    }
                    if (0 == PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer)
                    {
                        POE_LOG_INFO(
                            "u:%d/p:%d received %s packet(s) and restart port",
                            unit, port,
                            _printUI64_T(PORT_INFO(ptr_poe_control_block, unit, port)->rx_pkt_diff,
                                            rx_pkt_diff_string,
                                            sizeof(rx_pkt_diff_string)));
                        rc = poe_setPortPseControl(unit, port, FALSE);
                        if (MW_E_OK != rc)
                        {
                            POE_LOG_ERROR(
                                "set u:%d/p:%d pse control to DISABLE error(%d)",
                                unit, port, rc);
                        }
                        rc = poe_setPortPseControl(unit, port, TRUE);
                        if (MW_E_OK != rc)
                        {
                            POE_LOG_ERROR(
                                "set u:%d/p:%d pse control to ENABLE error(%d)",
                                unit, port, rc);
                        }
                        PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer =
                            ptr_poe_control_block->poe_watchdog_period;
                        POE_LOG_INFO(
                            "restart u:%d/p:%d and reload Watchdog timer count to %d",
                            unit, port, ptr_poe_control_block->poe_watchdog_period);
                    }
                }
                else
                {
                    if (TRUE == AIR_PORT_CHK(UNIT_INFO(ptr_poe_control_block, unit)->wd_pwr_status_on_pbmp, port))
                    {
                        POE_LOG_INFO("u:%d/p:%d power off",
                                       unit, port);
                        AIR_PORT_DEL(UNIT_INFO(ptr_poe_control_block, unit)->wd_pwr_status_on_pbmp, port);
                    }
                    PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_timer = 0;
                }
                PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_pre_mode = TRUE;
            }
            else
            {
                PORT_INFO(ptr_poe_control_block, unit, port)->poe_watchdog_pre_mode = FALSE;
            }
        }
    }

    ptr_poe_control_block->poe_watchdog_updated = FALSE;
    MW_FREE(ptr_rx_cnt);
    MW_FREE(ptr_tx_cnt);
}
#endif /* AIR_SUPPORT_POE_WATCHDOG */