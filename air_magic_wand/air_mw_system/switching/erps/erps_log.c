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

/* FILE NAME:  erps_log.c
 * PURPOSE:
 *    This file contains the implementation of ERPS log.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "erps_log.h"
#include    "erps_timer.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
const C8_T* const erps_record_role_vec[] =
{
    "transfer",
    "neighbor",
    "owner",
};
const C8_T* const erps_record_ring_state_vec[] =
{
    "Idle",
    "Protected",
    "Manual Switch",
    "Force Switch",
    "Pending",
    "Init",
};
const C8_T* const erps_record_port_state_vec[] =
{
    "Forward",
    "Discard",
};
const C8_T* const erps_record_ring_req_vec[] =
{
    "Clear",
    "FS",
    "R-APS(FS)",
    "SF",
    "Clear SF",
    "R-APS(SF)",
    "R-APS(MS)",
    "MS",
    "WTR Expires",
    "WTR Running",
    "WTB Expires",
    "WTB Running",
    "R-APS(NR,NB)",
    "R-APS(NR)",
    "None",
};

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void
_erps_port_dump(
    ERPS_PORT_T     *ptr_port);

static void
_erps_ring_timer_dump(
    ERPS_RING_T     *ptr_ring);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static void
_erps_port_dump(
    ERPS_PORT_T     *ptr_port)
{
    MW_CMD_OUTPUT("\t\tport id: %d\n", ptr_port->port_id);
    MW_CMD_OUTPUT("\t\tport role: %s\n", erps_record_role_vec[ERPS_PORT_GET_ROLE(ptr_port)]);
    MW_CMD_OUTPUT("\t\tfailed: %s\n", (ptr_port->is_failed ? "true" : "false"));
    MW_CMD_OUTPUT("\t\tport state: %s\n", erps_record_port_state_vec[ptr_port->port_state]);
    MW_CMD_OUTPUT("\t\t<node id, bpr>:<%02x-%02x-%02x-%02x-%02x-%02x, %d>\n",
                            ptr_port->node_id_bpr.node_id[0],
                            ptr_port->node_id_bpr.node_id[1],
                            ptr_port->node_id_bpr.node_id[2],
                            ptr_port->node_id_bpr.node_id[3],
                            ptr_port->node_id_bpr.node_id[4],
                            ptr_port->node_id_bpr.node_id[5],
                            ptr_port->node_id_bpr.bpr);

    return;
}

static void
_erps_ring_timer_dump(
    ERPS_RING_T     *ptr_ring)
{
    MW_CMD_OUTPUT("\tring timer:\n");
    MW_CMD_OUTPUT("\t\tguard time: %dms\n", ptr_ring->guard_time * 10);
    MW_CMD_OUTPUT("\t\tguard timer idx:%d\n", ptr_ring->guard_timer_idx);
    erps_timer_dump_timer(ptr_ring->guard_timer_idx);
    MW_CMD_OUTPUT("\t\twtr time: %dmin\n", ptr_ring->wtr_time);
    MW_CMD_OUTPUT("\t\twtr timer idx:%d\n", ptr_ring->wtr_timer_idx);
    erps_timer_dump_timer(ptr_ring->wtr_timer_idx);
    MW_CMD_OUTPUT("\t\twtb timer idx:%d\n", ptr_ring->wtb_timer_idx);
    erps_timer_dump_timer(ptr_ring->wtb_timer_idx);
    MW_CMD_OUTPUT("\t\thold-off time: %dms\n", ptr_ring->hold_off_time * 100);
    MW_CMD_OUTPUT("\t\thold-off timer idx:%d\n", ptr_ring->hold_off_timer_idx);
    erps_timer_dump_timer(ptr_ring->hold_off_timer_idx);

    return;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   erps_dump
 * PURPOSE:
 *      This API is used to dump ERPS info.
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
erps_dump(void)
{
    UI8_T                   index = 0, num = 0;
    ERPS_RING_T             *ptr_ring = NULL;
    MW_LOG_LEVEL_T          erps_debug_level = MW_LOG_LEVEL_OFF;

    mw_log_get_level(MW_LOG_MODULE_ERPS, &erps_debug_level);
    MW_CMD_OUTPUT("\nERPS:\n");
    MW_CMD_OUTPUT("Debug Level: %d\n", erps_debug_level);
    MW_CMD_OUTPUT("PLAT_PORT_BMP_GPHY: 0x%X\n", PLAT_PORT_BMP_GPHY[0]);

    MW_CMD_OUTPUT("ERPS instance list:\n");
    for(index = 0; index < ERPS_MAX_INSTANCE_NUM; index++)
    {
        ptr_ring = erps_instance_get_by_index(index);
        if((NULL != ptr_ring) &&
           (0 != ptr_ring->ring_id) &&
           (0 != ptr_ring->raps_vid))
        {
            MW_CMD_OUTPUT("[%d]\tring id: %d, raps vid: %d, acl id: %d, data vid: %s\n",
                            index, ptr_ring->ring_id, ptr_ring->raps_vid, ptr_ring->ring_acl_id, ptr_ring->data_vid);
            MW_CMD_OUTPUT("\tring state: %s\n", erps_record_ring_state_vec[ptr_ring->ring_state]);
            MW_CMD_OUTPUT("\tlocal request: %s\n", erps_record_ring_req_vec[ptr_ring->local_request]);
            MW_CMD_OUTPUT("\twest port:\n");
            _erps_port_dump(&(ptr_ring->west_port));
            MW_CMD_OUTPUT("\teast port:\n");
            _erps_port_dump(&(ptr_ring->east_port));
            MW_CMD_OUTPUT("\trpl port: %s\n", ((NULL == ptr_ring->ptr_rpl_port) ? "none" : (ptr_ring->ptr_rpl_port == &(ptr_ring->west_port) ? "west" : "east")));
            MW_CMD_OUTPUT("\tnode type: %s, revertive mode: %s, node id: %02x-%02x-%02x-%02x-%02x-%02x\n",
                                        erps_record_role_vec[ptr_ring->node_type],
                                        (ptr_ring->revertive_mode ? "true" : "false"),
                                        ptr_ring->node_id[0],
                                        ptr_ring->node_id[1],
                                        ptr_ring->node_id[2],
                                        ptr_ring->node_id[3],
                                        ptr_ring->node_id[4],
                                        ptr_ring->node_id[5]);
            MW_CMD_OUTPUT("\tsending raps: %s\n", (ptr_ring->is_send_raps ? "true" : "false"));
            MW_CMD_OUTPUT("\t\traps req: 0x%x, raps status: 0x%x, time reamin: %ds\n",
                                        ptr_ring->raps_request,
                                        ptr_ring->raps_status,
                                        ptr_ring->send_raps_time_remain);
            MW_CMD_OUTPUT("\tdb idx: %d\n", ptr_ring->db_idx);
            _erps_ring_timer_dump(ptr_ring);
            MW_CMD_OUTPUT("\r\n");
            num ++;
        }
    }
    MW_CMD_OUTPUT("Total instance num:%d\r\n", num);

    return;
}

/* FUNCTION NAME:   erps_pkt_data_dump
 * PURPOSE:
 *      This API is used to dump packet data.
 *
 * INPUT:
 *      ptr_addr             -- Pointer to packet data
 *      len                  -- Length of packet data
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
erps_pkt_data_dump(
    const UI8_T *ptr_addr,
    UI32_T len)
{
    I32_T n, m, c, r;
    UI8_T temp[16];

    MW_CMD_OUTPUT("\n");

    for( n = len; n > 0; )
    {
        MW_CMD_OUTPUT("%08x ", (UI32_T)ptr_addr);
        r = n < 16? n: 16;
        osapi_memcpy((void *) temp, (void *) ptr_addr, r);
        ptr_addr += r;
        for( m = 0; m < r; ++m )
        {
            MW_CMD_OUTPUT("%c", (m & 3) == 0 && m > 0? '.': ' ');
            MW_CMD_OUTPUT("%02x", temp[m]);
        }
        for(; m < 16; ++m )
            MW_CMD_OUTPUT("   ");
        MW_CMD_OUTPUT("  |");
        for( m = 0; m < r; ++m )
        {
            c = temp[m];
            MW_CMD_OUTPUT("%c", ' ' <= c && c <= '~'? c: '.');
        }
        n -= r;
        for(; m < 16; ++m )
                MW_CMD_OUTPUT(" ");
        MW_CMD_OUTPUT("|\n");
    }
    MW_CMD_OUTPUT("\n");

    return;
}
