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

/* FILE NAME:  lp_log.c
 * PURPOSE:
 *    This file contains the implementation of log for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_log.h"
#include    "lp_led.h"
#include    "lp_acl.h"
#include    "lp_utils.h"
#include    <cmlib/cmlib_bitmap.h>
#include    <cmlib/cmlib_port.h>
#include    <osal/osal_lib.h>

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
#ifndef LP_MW_SUPPORT
UI8_T lp_debug_level = LP_DEBUG_LEVEL_ERROR;
#endif

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   lp_log_dumpPortList
 * PURPOSE:
 *      This API is used to print the port list in bitmap format.
 *
 * INPUT:
 *      ptr_str              -- Pointer to the string to be printed
 *      pbm                  -- Bitmap to be printed
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
lp_log_dumpPortList(
    const C8_T                 *ptr_str,
    const AIR_PORT_BITMAP_T    pbm)
{
    UI32_T cnt = 0, port = 0, first = 0, span = 0;

    if(NULL == ptr_str)
    {
        return;
    }
    CMLIB_PORT_BITMAP_COUNT(pbm, cnt);
    if (0 == cnt)
    {
        osal_printf("%snull", ptr_str);
    }
    else
    {
        first = 1;
        CMLIB_BITMAP_BIT_FOREACH(pbm, port, AIR_PORT_BITMAP_SIZE)
        {
            osal_printf("%s%d", first ? ptr_str : ",", port);
            first = 0;

            for (span = 1; (++port < AIR_PORT_NUM) && CMLIB_BITMAP_BIT_CHK(pbm, port); span++);

            if (span > 1)
            {
                osal_printf("-%d", port - 1);
            }
        }
    }

    return;
}

/* FUNCTION NAME:   lp_log_dumpInfo
 * PURPOSE:
 *      This API is used to dump the loop prevention information.
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
lp_log_dumpInfo()
{
    UI32_T              unit = 0;
    UI32_T              u_portId, index;
    AIR_PORT_BITMAP_T   port_matrix_pbmap;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;
#ifdef LP_MW_SUPPORT
    MW_LOG_LEVEL_T      lp_debug_level = MW_LOG_LEVEL_OFF;

    mw_log_get_level(MW_LOG_MODULE_LP, &lp_debug_level);
#endif
    MW_CMD_OUTPUT("debug level:%d\n", lp_debug_level);
    ptr_lp_config = lp_getConfig();
    if (NULL == ptr_lp_config)
    {
        return;
    }

    MW_CMD_OUTPUT("\nLoop Prevention Admin: %d\n", ptr_lp_config->lp_admin);
    lp_led_dumpInfo();
    lp_acl_dumpInfo();
    MW_CMD_OUTPUT("\n");
    lp_log_dumpPortList("PLAT_PORT_BMP_TOTAL: ", &(PLAT_PORT_BMP_TOTAL[0]));
    lp_log_dumpPortList(", loop ports: ", &(ptr_lp_config->looped_pbmp[0]));
    lp_log_dumpPortList(", block ports: ", &(ptr_lp_config->blocked_pbmp[0]));
    lp_log_dumpPortList(", loop led ports: ", &(ptr_lp_config->loop_led_pbmp[0]));
    MW_CMD_OUTPUT("\n");
    for (index = 0; index < MAX_TRUNK_NUM; index++)
    {
        MW_CMD_OUTPUT("  Trunk[%d] member: ", index + 1);
        if (ptr_lp_config->trunk_info.group[index] == 0)
        {
            MW_CMD_OUTPUT("no member\n");
            continue;
        }
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
        {
            if (PLAT_CPU_PORT == u_portId)
            {
                continue;
            }
            if ((0x01 << u_portId) & (ptr_lp_config->trunk_info.group[index]))
            {
                MW_CMD_OUTPUT("%d, ", u_portId);
            }
        }
        MW_CMD_OUTPUT("\n");
    }

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, u_portId)
    {
        if (PLAT_CPU_PORT == u_portId)
        {
            /* Skip cpu port */
            continue;
        }

        air_port_getPortMatrix(unit, u_portId, port_matrix_pbmap);
        MW_CMD_OUTPUT("  Port %2d: %8s, %2ds, %9s, %2ds, %4s, Trunk id:%2X, FC REST:%ds, MCR:0x%8x, ",
                        u_portId,
                        (AIR_PORT_CHK(ptr_lp_config->looped_pbmp, u_portId) ? "LOOP" : "NOT_LOOP"),
                        ptr_lp_config->loop_timer[u_portId - 1],
                        (AIR_PORT_CHK(ptr_lp_config->blocked_pbmp, u_portId) ? "BLOCK" : "NOT_BLOCK"),
                        ptr_lp_config->block_timer[u_portId - 1],
                        (lp_port_getLinkStatus(u_portId) ? "UP" : "DOWN"),
                        (lp_lag_getGroupId(u_portId) == LP_TRUNK_NONE) ? (LP_TRUNK_NONE & 0xff) : lp_lag_getGroupId(u_portId) + 1,
                        ptr_lp_config->ptr_fc_resume_timer[u_portId - 1],
                        ptr_lp_config->record_mcr[u_portId - 1].value);
        lp_log_dumpPortList("PORTMATRIX: ", &(port_matrix_pbmap[0]));
        lp_log_dumpPortList(", TO LP: ", &(ptr_lp_config->ptr_to_loop_pbmp[u_portId - 1][0]));
        lp_log_dumpPortList(", TO BLK: ", &(ptr_lp_config->ptr_to_blk_pbmp[u_portId - 1][0]));
        MW_CMD_OUTPUT("\n");
    }
    return;
}
