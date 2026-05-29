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

/* FILE NAME:  lp_utils.c
 * PURPOSE:
 *    This file contains the implementation of utility functions for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_utils.h"
#include    "lp_acl.h"
#include    "lp_log.h"
#ifdef LP_MW_SUPPORT
#include    "lp_db.h"
#endif
#include    "lp_timer.h"
#include    "lp_led.h"
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
#include    "lp_hw.h"
#endif
#ifdef AIR_SUPPORT_SFP
#include    "sfp_util.h"
#include    "sfp_port.h"
#endif
#include    "hal.h"
#ifdef AIR_LITE_MW
#include    <hal/switch/pearl/hal_pearl_reg.h>
#elif AIR_EN_CORAL
#include    <hal/switch/coral/hal_coral_reg.h>
#else
#include    <hal/switch/sco/hal_sco_reg.h>
#endif
#include    <aml/aml.h>
#include    "ethernet.h"

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_LITE_MW
#define PREFIX(x) PEARL_##x
#elif AIR_EN_CORAL
#define PREFIX(x) CORAL_##x
#endif

#ifdef PREFIX
#define PMCR(p)                             PREFIX(PMCR(p))
#define MCR_FORCE_LINK_OFFT                 PREFIX(MCR_FORCE_LINK_OFFT)
#define MCR_FORCE_DUPLEX_OFFT               PREFIX(MCR_FORCE_DUPLEX_OFFT)
#define MCR_FORCE_DUPLEX_LENG               PREFIX(MCR_FORCE_DUPLEX_LENG)
#define MCR_FORCE_SPEED_OFFT                PREFIX(MCR_FORCE_SPEED_OFFT)
#define MCR_FORCE_SPEED_LENG                PREFIX(MCR_FORCE_SPEED_LENG)
#define MCR_FORCE_FLOWCTRL_TX_OFFT          PREFIX(MCR_FORCE_FLOWCTRL_TX_OFFT)
#define MCR_FORCE_FLOWCTRL_TX_LENG          PREFIX(MCR_FORCE_FLOWCTRL_TX_LENG)
#define MCR_FORCE_FLOWCTRL_RX_OFFT          PREFIX(MCR_FORCE_FLOWCTRL_RX_OFFT)
#define MCR_FORCE_FLOWCTRL_RX_LENG          PREFIX(MCR_FORCE_FLOWCTRL_RX_LENG)
#define MCR_BACK_PRESSURE_OFFT              PREFIX(MCR_BACK_PRESSURE_OFFT)
#define MCR_BACK_PRESSURE_LENG              PREFIX(MCR_BACK_PRESSURE_LENG)
#define MCR_FORCE_MODE_OFFT                 PREFIX(MCR_FORCE_MODE_OFFT)

#define PMSR(p)                             PREFIX(PMSR(p))
#define REG_PORT_DUPLEX_OFFT                PREFIX(REG_PORT_DUPLEX_OFFT)
#define REG_PORT_DUPLEX_LENG                PREFIX(REG_PORT_DUPLEX_LENG)
#define REG_PORT_SPEED_OFFT                 PREFIX(REG_PORT_SPEED_OFFT)
#define REG_PORT_SPEED_LENG                 PREFIX(REG_PORT_SPEED_LENG)
#endif /* PREFIX */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#if !defined(LP_MW_SUPPORT) && !defined(AIR_LP_USE_STP_BLOCK)
MW_ERROR_NO_T
_lp_updatePortMatrix(
    void);
#endif

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
#if !defined(LP_MW_SUPPORT) && !defined(AIR_LP_USE_STP_BLOCK)
MW_ERROR_NO_T
_lp_updatePortMatrix(
    void)
{
    I32_T               rc = 0;
    UI32_T              port = 0;
    UI32_T              unit = 0, port_matrix = 0;
    AIR_PORT_BITMAP_T   port_bitmap = {0};
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return MW_E_OP_INCOMPLETE;
    }

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }

        if(0 != (ptr_lp_config->blocked_pbmp[0] & BIT(port)))
        {
            port_matrix = 0;
        }
        else
        {
            port_matrix = (PLAT_PORT_BMP_TOTAL[0]) & (~(ptr_lp_config->blocked_pbmp[0]));
        }

        BIT_SET(port_matrix, PLAT_CPU_PORT);
        port_bitmap[0] = port_matrix;
        rc = air_port_setPortMatrix(unit, port, port_bitmap);
    }

    return rc;
}
#endif /* LP_MW_SUPPORT && AIR_LP_USE_STP_BLOCK */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   lp_port_comparePortId
 * PURPOSE:
 *      This API is used to check if the port number is bigger than the received port number.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *      rcvPort              -- Received port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE
 *      FALSE
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_port_comparePortId(
    UI16_T chkPort,
    UI16_T rcvPort)
{
    UI8_T i = 0;
    BOOL_T ret = TRUE;
    UI32_T lowest_portbmp_TX_group = 0;
    UI32_T lowest_portbmp_RX_group = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return ret;
    }
    /* Check trunk port */
    if (LP_TRUNK_MSK & chkPort)
    {
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if (ptr_lp_config->trunk_info.group[i] & (0x01 << rcvPort))
            {
                lowest_portbmp_RX_group = (ptr_lp_config->trunk_info.group[i] & (-ptr_lp_config->trunk_info.group[i]));
                break;
            }
        }
        if (i == MAX_TRUNK_NUM)
        {
            ret = FALSE;
        }
        else
        {
            /* Both trunk member, compare the lower port */
            for (i = 0; i < MAX_TRUNK_NUM; i++)
            {
                if (ptr_lp_config->trunk_info.group[i] & (0x01 << (chkPort & ~LP_TRUNK_MSK)))
                {
                    lowest_portbmp_TX_group = (ptr_lp_config->trunk_info.group[i] & (-ptr_lp_config->trunk_info.group[i]));
                    break;
                }
            }
            if (lowest_portbmp_RX_group > lowest_portbmp_TX_group)
            {
                ret = FALSE;
            }
        }
    }
    else
    {
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if (ptr_lp_config->trunk_info.group[i] & (0x01 << rcvPort))
            {
                break;
            }
        }
        if (i == MAX_TRUNK_NUM)
        {
            /* Both not trunk member, compare the lower port */
            if (rcvPort > chkPort)
            {
                ret = FALSE;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME:   lp_port_getLinkStatus
 * PURPOSE:
 *      This API is used to get the link state of the port.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE                 -- Port is link up
 *      FALSE                -- Port is link down
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_port_getLinkStatus(
    UI16_T chkPort)
{
    AIR_PORT_STATUS_T ps;
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T ret = FALSE;

#ifdef AIR_SUPPORT_SFP
    if ((TRUE == sfp_port_is_serdesPort(0, chkPort)) ||
        (TRUE == sfp_port_is_comboPort(0, chkPort)))
    {
        rc = sfp_port_getPortStatus(0, chkPort, &ps);
    }
    else
#endif
    {
        rc = air_port_getPortStatus(0, chkPort, &ps);
    }
    if (AIR_E_OK == rc)
    {
        if (ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)
        {
            ret = TRUE;
        }
    }
    else
    {
        LP_LOG_ERROR("Get port %d port status failed !", chkPort);
    }

    return ret;
}

/* FUNCTION NAME:   lp_lag_getLinkStatus
 * PURPOSE:
 *      This API is used to get the LAG link state of the specified port.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      TRUE                 -- LAG is link up
 *      FALSE                -- LAG is link down
 *
 * NOTES:
 *      None
 */
BOOL_T
lp_lag_getLinkStatus(
    UI16_T chkPort)
{
    UI32_T portMap = 0, i = 0;
    BOOL_T ret = FALSE;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return ret;
    }
    portMap = (0x01 << chkPort);
    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << chkPort)))
        {
            portMap = ptr_lp_config->trunk_info.group[i];
            break;
        }
    }

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, i)
    {
        if (0 != (portMap & (0x01 << i)))
        {
            ret = lp_port_getLinkStatus(i);
            if (TRUE == ret)
            {
                break;
            }
        }
    }

    return ret;
}

/* FUNCTION NAME:   lp_lag_getGroupId
 * PURPOSE:
 *      This API returns the lag group ID that the port belongs to.
 *
 * INPUT:
 *      chkPort              -- Port number to be checked
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Trunk group index if valid, otherwise LP_TRUNK_NONE
 *
 * NOTES:
 *      None
 */
UI16_T
lp_lag_getGroupId(
    UI16_T chkPort)
{
    UI16_T group_idx = LP_TRUNK_NONE, idx = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return group_idx;
    }

    for (idx = 0; idx < MAX_TRUNK_NUM; idx++)
    {
        if (chkPort & (LP_TRUNK_GP1 << idx))
        {
            group_idx = idx;
            break;
        }
        if ((0x01 << chkPort) & (ptr_lp_config->trunk_info.group[idx]))
        {
            group_idx = idx;
            break;
        }
    }

    return group_idx;
}

/* FUNCTION NAME:   lp_process_rxPacket
 * PURPOSE:
 *      Process the rx lp frame..
 *
 * INPUT:
 *      ptr_pbuf
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lp_process_rxPacket(
    struct pbuf *ptr_pbuf)
{
    LP_MSG_T lpMsg;
    UI32_T timestamp = 0;
    UI32_T rx_u_portId = 0;
    UI32_T targetPort = 0, blk_portMap = 0, loop_portMap = 0, all_port_blk_bmp = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    osapi_memcpy(&lpMsg, ptr_pbuf->payload, sizeof(LP_MSG_T));
    lpMsg.opCode = htons(lpMsg.opCode);
    lpMsg.portId = htons(lpMsg.portId);
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
    lpMsg.portId = lp_hw_convertMacPortToAirPort(lpMsg.portId);
#endif
    osapi_memcpy(&timestamp, &lpMsg.timeStamp[2], sizeof(UI32_T));
    timestamp = ntohl(timestamp);
    rx_u_portId = ptr_pbuf->stag_hdr.rx_hdr.sp;
    ptr_lp_config = lp_getConfig();
    if ((NULL == ptr_lp_config) || (MW_LP_MODE_DISABLE == ptr_lp_config->lp_admin))
    {
        LP_LOG_WARN("LP disabled, do not process LDF");
        pbuf_free(ptr_pbuf);
        return;
    }
    LP_LOG_DEBUG("Recev ldf, Tx port:%d(msg trunk bits:0x%x, trunk member:0x%x), Rx port:%d(trunk id:%x, member:0x%x)",
                    lpMsg.portId & ~LP_TRUNK_MSK, (lpMsg.portId & LP_TRUNK_MSK) >> 12, (lpMsg.portId & LP_TRUNK_MSK) ? ptr_lp_config->trunk_info.group[lp_lag_getGroupId(lpMsg.portId)] : 0,
                    rx_u_portId, (lp_lag_getGroupId(rx_u_portId) + 1) & 0xf, (lp_lag_getGroupId(rx_u_portId) == LP_TRUNK_NONE) ? 0 : ptr_lp_config->trunk_info.group[lp_lag_getGroupId(rx_u_portId)]);

#ifndef AIR_HARDWARE_SEND_LDF_SUPPORT
    if ((ptr_lp_config->timestamp - timestamp) > LP_LDF_TIMESTAPE_VALID_TIME)
    {
        LP_LOG_DEBUG("Recev expired ldf, current timestame:%u, send timestamp:%u", ptr_lp_config->timestamp, timestamp);
    }
    else
#endif /* AIR_HARDWARE_SEND_LDF_SUPPORT */
    {
        if (TRUE == lp_port_comparePortId(lpMsg.portId, rx_u_portId))
        {
            /* If trunk, get trunk group port map */
            blk_portMap = lp_port_convertPortIdToBitmap(lpMsg.portId);
            loop_portMap = lp_port_convertPortIdToBitmap(rx_u_portId);
        }
        else
        {
            /* If trunk, get trunk group port map */
            blk_portMap = lp_port_convertPortIdToBitmap(rx_u_portId);
            loop_portMap = lp_port_convertPortIdToBitmap(lpMsg.portId);
        }
#ifdef LP_LOOP_DECTECTION_SUPPORT
        if (MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin)
        {
            loop_portMap |= blk_portMap;
            blk_portMap = 0;
        }
#endif /* LP_LOOP_DECTECTION_SUPPORT */

        all_port_blk_bmp = (blk_portMap | ptr_lp_config->blocked_pbmp[0]);
        LP_LOG_DEBUG("blk_portMap:0x%x --> 0x%x loop_portMap:0x%x --> 0x%x", ptr_lp_config->ptr_to_blk_pbmp[rx_u_portId - 1][0], blk_portMap, ptr_lp_config->ptr_to_loop_pbmp[rx_u_portId - 1][0], loop_portMap);
        ptr_lp_config->ptr_to_blk_pbmp[rx_u_portId - 1][0] |= blk_portMap;
        ptr_lp_config->ptr_to_loop_pbmp[rx_u_portId - 1][0] |= (loop_portMap & ~blk_portMap);
        loop_portMap &= ~all_port_blk_bmp;

        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, targetPort)
        {
            if (PLAT_CPU_PORT == targetPort)
            {
                /* Skip cpu port */
                continue;
            }
            if ((FALSE != ((blk_portMap | loop_portMap) & BIT(targetPort))) &&
                (FALSE == lp_lag_getLinkStatus(targetPort)))
            {
                LP_LOG_WARN("port:%d has been link down", targetPort);
                continue;
            }
            if (blk_portMap & BIT(targetPort))
            {
                AIR_PORT_DEL(ptr_lp_config->looped_pbmp, targetPort);
                ptr_lp_config->block_timer[targetPort - 1] = (LP_BLOCK_PERIOD / LP_TIMER_PERIOD);
                if (FALSE == AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, targetPort))
                {
                    AIR_PORT_ADD(ptr_lp_config->blocked_pbmp, targetPort);
#ifdef LP_MW_SUPPORT
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, targetPort=%d, LP_BLOCK)", targetPort);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, targetPort, sizeof(UI8_T), LP_BLOCK);
#endif
                    LP_LOG_DEBUG("->lp_port_block(TRUE, u_portId=%d)", targetPort);
                    /* Set port-matrix to disable packet.PortID RX/TX */
                    lp_port_block(TRUE, targetPort);
                    if (FALSE == AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, targetPort))
                    {
                        if (MW_E_OK == lp_led_setBehavior(TRUE, targetPort))
                        {
                            AIR_PORT_ADD(ptr_lp_config->loop_led_pbmp, targetPort);
#ifdef LP_GLOBAL_LED_SUPPORT
                            lp_led_updateGlobalLed(ptr_lp_config->loop_led_pbmp);
#endif
                        }
                    }
                }
            }

            if (loop_portMap & BIT(targetPort))
            {
                AIR_PORT_DEL(ptr_lp_config->blocked_pbmp, targetPort);
                ptr_lp_config->loop_timer[targetPort - 1] = (LP_BLOCK_PERIOD / LP_TIMER_PERIOD);
                if (FALSE == AIR_PORT_CHK(ptr_lp_config->looped_pbmp, targetPort))
                {
                    AIR_PORT_ADD(ptr_lp_config->looped_pbmp, targetPort);
#ifdef LP_LOOP_DECTECTION_SUPPORT
                    if (MW_LP_MODE_DETECTION == ptr_lp_config->lp_admin)
                    {
                        lp_port_setFlowControl(TRUE, targetPort);
                    }
#endif /* LP_LOOP_DECTECTION_SUPPORT */
#ifdef LP_MW_SUPPORT
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, targetPort=%d, LP_LOOP)", targetPort);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, targetPort, sizeof(UI8_T), LP_LOOP);
#endif /* LP_MW_SUPPORT */
                }

                if (TRUE == ptr_lp_config->looped_port_blink)
                {
                    if (FALSE == AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, targetPort))
                    {
                        if (MW_E_OK == lp_led_setBehavior(TRUE, targetPort))
                        {
                            AIR_PORT_ADD(ptr_lp_config->loop_led_pbmp, targetPort);
#ifdef LP_GLOBAL_LED_SUPPORT
                            lp_led_updateGlobalLed(ptr_lp_config->loop_led_pbmp);
#endif
                        }
                    }
                }
            }
        }
    }

    pbuf_free(ptr_pbuf);
    return;
}

/* FUNCTION NAME: lp_updatePortLoopState
 * PURPOSE:
 *      Update the loop prevention port status
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
lp_updatePortLoopState(
    void)
{
    UI8_T i = 0, port = 0;
    MW_ERROR_NO_T ret = MW_E_OK;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return ret;
    }

    for (i = 0; i < MAX_TRUNK_NUM; i++)
    {
        if (0 != (ptr_lp_config->trunk_info.group[i] & ptr_lp_config->blocked_pbmp[0]))
        {
            /* Set all group member to block port */
            for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
            {
                if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << port)))
                {
                    ptr_lp_config->block_timer[(port - 1)] = (LP_BLOCK_PERIOD / LP_TIMER_PERIOD);
                    AIR_PORT_ADD(ptr_lp_config->blocked_pbmp, port);
                    AIR_PORT_DEL(ptr_lp_config->looped_pbmp, port);
                    ptr_lp_config->ptr_to_blk_pbmp[port - 1][0] |= (0x01 << port);
                    ptr_lp_config->ptr_to_loop_pbmp[port - 1][0] &= ~(ptr_lp_config->ptr_to_blk_pbmp[port - 1][0]);
#ifdef LP_MW_SUPPORT
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, port=%d, LP_BLOCK)", port);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, port, sizeof(UI8_T), LP_BLOCK);
#endif
                    /* Set port-matrix to disable packet.PortID RX/TX */
                    lp_port_block(TRUE, port);
                    if (FALSE == AIR_PORT_CHK(ptr_lp_config->loop_led_pbmp, port))
                    {
                        if (MW_E_OK == lp_led_setBehavior(TRUE, port))
                        {
                            AIR_PORT_ADD(ptr_lp_config->loop_led_pbmp, port);
                        }
                    }
                }
            }
        }
        else if (0 != (ptr_lp_config->trunk_info.group[i] & ptr_lp_config->looped_pbmp[0]))
        {
            /* Set all group member to loop port */
            for (port = 1; port <= PLAT_MAX_PORT_NUM; port++)
            {
                if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << port)))
                {
                    ptr_lp_config->loop_timer[(port - 1)] = (LP_BLOCK_PERIOD / LP_TIMER_PERIOD);
                    AIR_PORT_ADD(ptr_lp_config->looped_pbmp, port);
                    AIR_PORT_DEL(ptr_lp_config->blocked_pbmp, port);
                    ptr_lp_config->ptr_to_loop_pbmp[port - 1][0] |= (0x01 << port);
                    ptr_lp_config->ptr_to_loop_pbmp[port - 1][0] &= ~(ptr_lp_config->ptr_to_blk_pbmp[port - 1][0]);
#ifdef LP_MW_SUPPORT
                    LP_LOG_DEBUG("->lp_db_send(M_UPDATE, PORT_LOOP_STATE, port=%d, LP_LOOP)", port);
                    lp_db_send(M_UPDATE, PORT_OPER_INFO, PORT_LOOP_STATE, port, sizeof(UI8_T), LP_LOOP);
#endif
                }
            }
        }
    }

    return ret;
}

/* FUNCTION NAME:   lp_port_setFlowControl
 * PURPOSE:
 *      Set flow control for a port
 *
 * INPUT:
 *      block            --  TRUE to block, FALSE to unblock
 *      port             --  port number
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
lp_port_setFlowControl(
    BOOL_T block,
    UI16_T port)
{
    UI32_T unit = 0, mcr_u32dat = 0, msr_u32dat = 0;
    UI32_T mac_port = 0;
    AIR_PORT_SPEED_T    speed;
    AIR_PORT_DUPLEX_T   duplex;
    BOOL_T linkup = FALSE;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if ((0 == port) || (NULL == ptr_lp_config))
    {
        return;
    }
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);
    aml_readReg(unit, PMCR(mac_port), &mcr_u32dat, sizeof(mcr_u32dat));
    LP_LOG_DEBUG("%s port[%d] MCR = 0x%x, record MCR = 0x%x",
                    block ? "Block" : "Unblock", port, mcr_u32dat, ptr_lp_config->record_mcr[port - 1].value);

    if (block)
    {
        /* Read data from MSR register */
        if (0 != ptr_lp_config->record_mcr[port - 1].ref_cnt)
        {
            LP_LOG_WARN("port[%d] has been blocked", port);
            return;
        }
        linkup = lp_port_getLinkStatus(port);
        if (linkup)
        {
            /* Read data from MCR register */
            if ((mcr_u32dat & BITS_RANGE(MCR_FORCE_FLOWCTRL_TX_OFFT, MCR_FORCE_FLOWCTRL_TX_LENG)) || (mcr_u32dat & BITS_RANGE(MCR_FORCE_FLOWCTRL_RX_OFFT, MCR_FORCE_FLOWCTRL_RX_LENG)))
            {
                ptr_lp_config->record_mcr[port - 1].ref_cnt ++;
                ptr_lp_config->record_mcr[port - 1].value = mcr_u32dat;
                LP_LOG_DEBUG("record port[%d] MCR data to 0x%x", port, ptr_lp_config->record_mcr[port - 1].value);

                aml_readReg(unit, PMSR(mac_port), &msr_u32dat, sizeof(msr_u32dat));
                LP_LOG_DEBUG("port[%d] link status:%d, MSR = 0x%x", port, linkup, msr_u32dat);
                speed = BITS_OFF_R(msr_u32dat, REG_PORT_SPEED_OFFT, REG_PORT_SPEED_LENG);
                duplex = BITS_OFF_R(msr_u32dat, REG_PORT_DUPLEX_OFFT, REG_PORT_DUPLEX_LENG);

                mcr_u32dat &= ~BITS_RANGE(MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
                mcr_u32dat |= BITS_OFF_L(speed, MCR_FORCE_SPEED_OFFT, MCR_FORCE_SPEED_LENG);
                mcr_u32dat &= ~BITS_RANGE(MCR_FORCE_DUPLEX_OFFT, MCR_FORCE_DUPLEX_LENG);
                mcr_u32dat |= BITS_OFF_L(duplex, MCR_FORCE_DUPLEX_OFFT, MCR_FORCE_DUPLEX_LENG);
                mcr_u32dat &= ~BITS_RANGE(MCR_FORCE_FLOWCTRL_TX_OFFT, MCR_FORCE_FLOWCTRL_TX_LENG);
                mcr_u32dat &= ~BITS_RANGE(MCR_FORCE_FLOWCTRL_RX_OFFT, MCR_FORCE_FLOWCTRL_RX_LENG);
                mcr_u32dat &= ~BITS_RANGE(MCR_BACK_PRESSURE_OFFT, MCR_BACK_PRESSURE_LENG);
                mcr_u32dat |= BIT(MCR_FORCE_LINK_OFFT);
                mcr_u32dat |= BIT(MCR_FORCE_MODE_OFFT);
                LP_LOG_DEBUG("set port[%d] MCR data to 0x%x", port, mcr_u32dat);
                aml_writeReg(unit, PMCR(mac_port), &mcr_u32dat, sizeof(mcr_u32dat));
            }
            else
            {
                LP_LOG_DEBUG("port[%d] Flow Control is off, do not need to set Flow Control off", port);
            }
        }
        else
        {
            LP_LOG_DEBUG("port[%d] is link down, do not need to set Flow Control off", port);
        }
    }
    else
    {
        if (0 == ptr_lp_config->record_mcr[port - 1].ref_cnt)
        {
            LP_LOG_WARN("port[%d] do not need resume MCR", port);
            ptr_lp_config->record_mcr[port - 1].value = 0;
            return;
        }
        if (0 != ptr_lp_config->record_mcr[port - 1].value)
        {
            mcr_u32dat = ptr_lp_config->record_mcr[port - 1].value;
            LP_LOG_DEBUG("resume port[%d] MCR data to 0x%x", port, ptr_lp_config->record_mcr[port - 1].value);
            aml_writeReg(unit, PMCR(mac_port), &mcr_u32dat, sizeof(mcr_u32dat));
        }
        ptr_lp_config->record_mcr[port - 1].value = 0;
        ptr_lp_config->record_mcr[port - 1].ref_cnt = 0;
    }

    return;
}

/* FUNCTION NAME:   lp_port_block
 * PURPOSE:
 *      Block/Unblock port.
 *
 * INPUT:
 *      block            --  TRUE to block, FALSE to unblock
 *      u_portId         --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void lp_port_block(
    BOOL_T block,
    UI16_T u_portId)
{
    UI32_T unit = 0;
    AIR_SEC_MAC_LIMIT_PORT_CFG_T sec_p_cfg;

    LP_LOG_DEBUG("block=%d, u_portId=%d", block, u_portId);
#ifndef LP_MW_SUPPORT
    air_l2_flushMacAddr(unit, AIR_L2_MAC_FLUSH_TYPE_PORT, u_portId);
#ifdef AIR_LP_USE_STP_BLOCK
    UI32_T            fid = 0;
    AIR_STP_STATE_T   state = AIR_STP_STATE_LISTEN;

    state = (block ? AIR_STP_STATE_LISTEN : AIR_STP_STATE_FORWARD);
    if (AIR_E_OK != air_stp_setPortState(unit, u_portId, fid, state))
    {
        LP_LOG_ERROR("Stp set forward to un-Block port(%u) failed", u_portId);
    }
#else
    if (MW_E_OK != _lp_updatePortMatrix())
    {
        LP_LOG_ERROR("_update_port_matrix error");
    }
#endif  /* AIR_LP_USE_STP_BLOCK */
#endif  /* LP_MW_SUPPORT  */

    if (block)
    {
        /* Disable SA-learning of Block port */
        memset(&sec_p_cfg, 0, sizeof(sec_p_cfg));
        air_sec_getMacLimitPortCfg(unit, u_portId, &sec_p_cfg);
        sec_p_cfg.flags &= (~AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN);
        if (AIR_E_OK != air_sec_setMacLimitPortCfg(unit, u_portId, &sec_p_cfg))
        {
            LP_LOG_ERROR("Disable SA-learning of Block port(%u) failed", u_portId);
        }
    }
    else
    {
        /* Enable SA-learning of un-Block port */
        memset(&sec_p_cfg, 0, sizeof(sec_p_cfg));
        air_sec_getMacLimitPortCfg(unit, u_portId, &sec_p_cfg);
        sec_p_cfg.flags |= (AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN);
        if (AIR_E_OK != air_sec_setMacLimitPortCfg(unit, u_portId, &sec_p_cfg))
        {
            LP_LOG_ERROR("Enable SA-learning of Block port(%u) failed", u_portId);
        }
    }
    lp_port_setFlowControl(block, u_portId);

    return;
}

#ifndef AIR_HARDWARE_SEND_LDF_SUPPORT
/* FUNCTION NAME:   lp_sendLdf
 * PURPOSE:
 *      Send lp detect frame to all ports.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
lp_sendLdf()
{
    struct pbuf *ptr_pbuf = NULL;
    LP_MSG_T    *lpMsg = NULL;
    UI16_T        u_portId = 0;
    UI8_T *payload_0 = NULL, i = 0;
    UI16_T        pbuf_len_0 = 0;
    UI16_T        pbuf_tot_len_0 = 0;
    UI32_T        timestamp = 0;
    UI32_T        trunk_all = 0;
    AIR_MAC_T mac;
    UI32_T unit = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if ((AIR_E_OK != air_swc_getSystemMac(unit, mac)) || (NULL == ptr_lp_config))
    {
        LP_LOG_ERROR("Prameter error.");
        return;
    }

    timestamp = htonl(ptr_lp_config->timestamp);
    ptr_pbuf = pbuf_alloc(PBUF_TRANSPORT, 64, PBUF_RAM);
    if (ptr_pbuf)
    {
        osapi_memset(ptr_pbuf->payload, 0, 64);
        ptr_pbuf->ether_hdr.dest.addr[0] = 0xff;
        ptr_pbuf->ether_hdr.dest.addr[1] = 0xff;
        ptr_pbuf->ether_hdr.dest.addr[2] = 0xff;
        ptr_pbuf->ether_hdr.dest.addr[3] = 0xff;
        ptr_pbuf->ether_hdr.dest.addr[4] = 0xff;
        ptr_pbuf->ether_hdr.dest.addr[5] = 0xff;

        osapi_memcpy(ptr_pbuf->ether_hdr.src.addr, mac, sizeof(AIR_MAC_T));

        ptr_pbuf->stag_hdr.tx_hdr.mode = STAG_HDR_MODE_INSERT;
        ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.opc = 0;
        ptr_pbuf->vlan_num = 1;
        ptr_pbuf->vlan_hdr.tpid = 0x8100;
        ptr_pbuf->vlan_hdr.priority = 7;
        ptr_pbuf->vlan_hdr.vid = 1;

        payload_0 = ptr_pbuf->payload;
        pbuf_len_0 = ptr_pbuf->len;
        pbuf_tot_len_0 = ptr_pbuf->tot_len;

        lpMsg = (LP_MSG_T *)ptr_pbuf->payload;
        lpMsg->opCode = 0;
        lpMsg->moduleId[0] = 0x0;
        lpMsg->moduleId[1] = LP_MODULE_ID;
        lpMsg->moduleId[2] = 0x0;
        lpMsg->moduleId[3] = 0x0;
        lpMsg->moduleId[4] = 0x0;
        lpMsg->moduleId[5] = 0x0;
        osapi_memset(lpMsg->timeStamp, 0, 6);
        osapi_memcpy(&lpMsg->timeStamp[2], &timestamp, sizeof(UI32_T));
        if (0 == (ptr_lp_config->timestamp % LP_LDF_TIMESTAPE_VALID_TIME))
        {
            LP_LOG_DEBUG("lp send ldf, timestamp:%u", ntohl(timestamp));
        }

        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            trunk_all |= ptr_lp_config->trunk_info.group[i];
        }
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
        {
            if (PLAT_CPU_PORT == u_portId)
            {
                /* Skip cpu port */
                continue;
            }

            if (0 != (trunk_all & (0x01 << u_portId)))
            {
                /* Skip trunk group */
                continue;
            }

            ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << u_portId);
            ptr_pbuf->len = pbuf_len_0;
            ptr_pbuf->payload = payload_0;
            ptr_pbuf->tot_len = pbuf_tot_len_0;

            lpMsg->portId = htons(u_portId);
            ethernet_output_use_default_netif(ptr_pbuf, ETHTYPE_LP);
        }

        /* Send one frame of lower port ID for each Trunk group */
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if (0 == ptr_lp_config->trunk_info.group[i])
            {
                continue;
            }
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
            {
                if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << u_portId)))
                {
                    ptr_pbuf->stag_hdr.tx_hdr.tx_stag_insert.dp = (1 << u_portId);
                    ptr_pbuf->len = pbuf_len_0;
                    ptr_pbuf->payload = payload_0;
                    ptr_pbuf->tot_len = pbuf_tot_len_0;

                    u_portId |= (LP_TRUNK_GP1 << i);
                    lpMsg->portId = htons(u_portId);
                    ethernet_output_use_default_netif(ptr_pbuf, ETHTYPE_LP);
                    break;
                }
            }
        }
        pbuf_free(ptr_pbuf);
    }

    return;
}
#endif

/* FUNCTION NAME:   lp_getBlockPbmp
 * PURPOSE:
 *      Get lp block port bitmap.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      pbmp
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
lp_getBlockPbmp(
    AIR_PORT_BITMAP_T pbmp)
{
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL != ptr_lp_config)
    {
        AIR_PORT_BITMAP_COPY(pbmp, ptr_lp_config->blocked_pbmp);
    }
    return;
}

/* FUNCTION NAME:   lp_port_convertPortIdToBitmap
 * PURPOSE:
 *      Block/Unblock port.
 *
 * INPUT:
 *      port_id          --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      Bitmap value
 *
 * NOTES:
 *      None
 */
UI32_T
lp_port_convertPortIdToBitmap(
    UI16_T port_id)
{
    /* If trunk, get trunk group index */
    UI16_T group_idx = lp_lag_getGroupId(port_id);
    UI32_T port_map = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return port_map;
    }

    if (LP_TRUNK_NONE != group_idx)
    {
        port_map = ptr_lp_config->trunk_info.group[group_idx];
    }
    else
    {
        port_map = (0x01 << port_id);
    }
    return port_map;
}

/* FUNCTION NAME: lp_port_delFromLoopPbmpGroup
 * PURPOSE:
 *      This function is used to delete the port from the to loop bitmap group.
 *
 * INPUT:
 *      port             --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
lp_port_delFromLoopPbmpGroup(
    UI16_T port)
{
    UI8_T i;
    LP_CONFIG_INFO_T    *ptr_lp_config = lp_getConfig();

    if (NULL != ptr_lp_config)
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (AIR_PORT_CHK(ptr_lp_config->ptr_to_loop_pbmp[i], port))
            {
                AIR_PORT_DEL(ptr_lp_config->ptr_to_loop_pbmp[i], port);
            }
        }
    }
}

/* FUNCTION NAME: lp_port_delFromBlockPbmpGroup
 * PURPOSE:
 *      This function is used to delete the port from the to block bitmap group.
 *
 * INPUT:
 *      port             --  port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
lp_port_delFromBlockPbmpGroup(
    UI16_T port)
{
    UI8_T i;
    LP_CONFIG_INFO_T    *ptr_lp_config = lp_getConfig();

    if (NULL != ptr_lp_config)
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (AIR_PORT_CHK(ptr_lp_config->ptr_to_blk_pbmp[i], port))
            {
                AIR_PORT_DEL(ptr_lp_config->ptr_to_blk_pbmp[i], port);
            }
        }
    }
}