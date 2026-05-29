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
#ifdef AIR_SUPPORT_LACP
#include "mw_error.h"
#include "mw_types.h"
#include "osapi.h"
#include "osapi_string.h"
#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "default_config.h"
#include "mw_lacp.h"

/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T
_lacp_cmd_show_info(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_lacp_cmd_setPortPriority(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_lacp_cmd_setSystemPriority(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_lacp_cmd_setMaxMemberNum(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_lacp_cmd_setPortTimeout(
    const C8_T *tokens[],
    UI32_T token_idx);

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_CMD_LACP_NAME    "cmd_lacp"
/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T
_mw_lacp_cmd_vec[] =
{
    {
        "show info", 2, _lacp_cmd_show_info,
        "lacp show info: show lacp information\n"
    },
    {
        "set port-priority", 2, _lacp_cmd_setPortPriority,
        "lacp set port-priority portlist=<UINTLIST> pri=<UINT>\n"
    },
    {
        "set system-priority", 2, _lacp_cmd_setSystemPriority,
        "lacp set system-priority pri=<UINT>\n"
    },
    {
        "set member", 2, _lacp_cmd_setMaxMemberNum,
        "lacp set member max-num=<UINT>\n"
    },
    {
        "set timeout", 2, _lacp_cmd_setPortTimeout,
        "lacp set timeout portlist=<UINTLIST> mode={ long | short }\n"
    },
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T
_lacp_cmd_show_info(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;

    /* Parser tokens */
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ret = mw_cmd_lacp_show_info();

    return ret;
}

static MW_ERROR_NO_T
_lacp_cmd_setPortPriority(
    const C8_T *tokens[],
    UI32_T token_idx)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0, i;
    AIR_PORT_BITMAP_T pbm = { 0 };
    UI32_T priority = 0;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    /*
     * Command format
     * lacp set port-priority portlist=<UINTLIST> priority=<UINT>
     */

    /* paser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "pri", &priority), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

#if LACP_MUTEX
    if (MW_E_OK == lag_lacp_mutex_take())
#endif
    {
        AIR_PORT_FOREACH(pbm, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            ptr_lacp_info->lacp_port_priority[port - 1] = priority;
            if (NULL != ptr_lacp_info->lag_port[port - 1])
            {
                if (ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_portid.lpi_prio != htons((UI16_T)priority))
                {
                    ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_portid.lpi_prio = htons((UI16_T)priority);
                    ptr_lacp_info->lag_port[port - 1]->lp_selected = LACP_UNSELECTED;
                }
            }
        }

        for (i = 0;i < MAX_TRUNK_NUM;i++)
        {
            if (ptr_lacp_info->lag_group[i])
            {
                lag_sort_lacp_ports(i);
            }
        }
#if LACP_MUTEX
        lag_lacp_mutex_give();
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_lacp_cmd_setSystemPriority(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T port;
    UI32_T priority = 0;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    /*
     * Command format
     * lacp set system-priority priority=<UINT>
     */

    /* paser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "pri", &priority), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    ptr_lacp_info->lacp_system_priority = priority;

#if LACP_MUTEX
    if (MW_E_OK == lag_lacp_mutex_take())
#endif
    {
        AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            if (NULL != ptr_lacp_info->lag_port[port - 1])
            {
                if (ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_systemid.lsi_prio != htons((UI16_T)priority))
                {
                    ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_systemid.lsi_prio = htons((UI16_T)priority);
                    ptr_lacp_info->lag_port[port - 1]->lp_selected = LACP_UNSELECTED;
                }
            }
        }
#if LACP_MUTEX
        lag_lacp_mutex_give();
#endif
    }

    return rc;
}

static MW_ERROR_NO_T
_lacp_cmd_setMaxMemberNum(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T max_num = 0, port;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    /*
     * Command format
     * lacp set max-member-num num=<UINT>
     */

    /* paser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getUint(tokens, token_idx, "max-num", (UI32_T *)&max_num), token_idx, 2);
    if (max_num >= 0 && max_num <= MAX_TRUNK_NUM)
    {
        ptr_lacp_info->lacp_max_port_num = max_num;
        MW_CMD_OUTPUT("Set LACP max member num: %d\n", max_num);
#if LACP_MUTEX
        if (MW_E_OK == lag_lacp_mutex_take())
#endif
        {
            AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
            {
                if (PLAT_CPU_PORT == port)
                {
                    continue;
                }
                if (NULL != ptr_lacp_info->lag_port[port - 1])
                {
                    ptr_lacp_info->lag_port[port - 1]->lp_selected = LACP_UNSELECTED;
                }
            }
#if LACP_MUTEX
            lag_lacp_mutex_give();
#endif
        }
    }

    return ret;
}

static MW_ERROR_NO_T
_lacp_cmd_setPortTimeout(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI32_T unit = 0, port;
    C8_T timeout_str[MW_CMD_CMD_MAX_LENGTH];
    AIR_PORT_BITMAP_T pbm = { 0 };
    BOOL_T timeout = FALSE;
    lacp_info_t *ptr_lacp_info = lacp_get_settings();

    /*
     * Command format
     * lacp set timeout portlist=<UINTLIST> mode={ long | short }
     */

    osapi_memset(timeout_str, 0, sizeof(timeout_str));
    /* paser tokens */
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "mode", timeout_str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (MW_E_OK == mw_cmd_checkString(timeout_str, "long"))
    {
        timeout = FALSE;
    }
    else if (MW_E_OK == mw_cmd_checkString(timeout_str, "short"))
    {
        timeout = TRUE;
    }

#if LACP_MUTEX
    if (MW_E_OK == lag_lacp_mutex_take())
#endif
    {
        AIR_PORT_FOREACH(pbm, port)
        {
            if (PLAT_CPU_PORT == port)
            {
                continue;
            }
            if (NULL != ptr_lacp_info->lag_port[port - 1])
            {
                if (timeout)
                {
                    if (!(ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_state & LACP_STATE_TIMEOUT))
                    {
                        ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_state |= LACP_STATE_TIMEOUT;
                        ptr_lacp_info->lag_port[port - 1]->lp_selected = LACP_UNSELECTED;
                    }
                }
                else
                {
                    if (ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_state & LACP_STATE_TIMEOUT)
                    {
                        ptr_lacp_info->lag_port[port - 1]->lp_actor.lip_state &= ~LACP_STATE_TIMEOUT;
                        ptr_lacp_info->lag_port[port - 1]->lp_selected = LACP_UNSELECTED;
                    }
                }
            }
        }
#if LACP_MUTEX
        lag_lacp_mutex_give();
#endif
    }

    return ret;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_lacp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: lacp.
 *
 * INPUT:
 *      tokens      --  Command tokens
 *      token_idx   --  The index of 1st valid token
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
mw_cmd_lacp_dispatcher(
    const C8_T *tokens[],
    UI32_T                      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_lacp_cmd_vec, sizeof(_mw_lacp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_lacp_usager
 * PURPOSE:
 *      Command usage for magic wand command: lacp.
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
mw_cmd_lacp_usager(
    void)
{
    return (mw_cmd_usager(_mw_lacp_cmd_vec, sizeof(_mw_lacp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

#endif