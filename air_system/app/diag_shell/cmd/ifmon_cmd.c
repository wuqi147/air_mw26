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

#include <cmd/ifmon_cmd.h>

#include <air_cfg.h>
#include <air_error.h>
#include <air_ifmon.h>
#include <air_types.h>
#include <cmlib/cmlib_bitmap.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define IFMON_CMD_DEFAULT_INTERVAL (500)

/* MACRO FUNCTION DECLARATIONS
 */
#define IFMON_CMD_PRINT_FILED_NAME(__name__) osal_printf("%-20s: ", (__name__))

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_ifmon_cmd_setMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    AIR_IFMON_MODE_T  mode = AIR_IFMON_MODE_POLL;
    AIR_PORT_BITMAP_T portlist;
    UI32_T            interval = IFMON_CMD_DEFAULT_INTERVAL;

    /* ifmon set [ unit=<UINT> ] portlist=<UINTLIST> [ interval=<UINT> ] */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "interval", &interval), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ifmon_setMode(unit, mode, portlist, interval);
    if (rc != AIR_E_OK)
    {
        osal_printf("***Error***, set mode fail.\n");
    }

    return rc;
}

void
_ifmon_cmd_showPortlist(
    AIR_PORT_BITMAP_T *ptr_bitmap)
{
    UI32_T port = 0;
    UI32_T cont_port = 0;
    UI32_T first_print = 1;

    while (port < AIR_PORT_NUM)
    {
        if (AIR_PORT_CHK(*ptr_bitmap, port))
        {
            cont_port = port + 1;
            while (cont_port < AIR_PORT_NUM)
            {
                if (!AIR_PORT_CHK(*ptr_bitmap, cont_port))
                {
                    break;
                }
                cont_port++;
            }

            if (first_print)
            {
                first_print = 0;
            }
            else
            {
                osal_printf(",");
            }

            osal_printf("%d", port);
            if ((cont_port - port) > 1)
            {
                osal_printf("-%d", cont_port - 1);
            }
            port = cont_port;
        }

        port++;
    }

    return;
}

static AIR_ERROR_NO_T
_ifmon_cmd_showMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_IFMON_MODE_T  mode = AIR_IFMON_MODE_POLL;
    AIR_PORT_BITMAP_T portlist;
    BOOL_T            enable = FALSE;
    UI32_T            unit = 0;
    UI32_T            interval = IFMON_CMD_DEFAULT_INTERVAL;

    /* ifmon show [ unit=<UINT> ] */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ifmon_getMonitorState(unit, &enable);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get monitor state fail.\n");
        return rc;
    }

    IFMON_CMD_PRINT_FILED_NAME("ifmon monitor state");
    osal_printf("%s\n", (TRUE == enable) ? "enable" : "disable");

    rc = air_ifmon_getMode(unit, &mode, &portlist, &interval);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get mode fail.\n");
        return rc;
    }

    IFMON_CMD_PRINT_FILED_NAME("ifmon mode");
    osal_printf("%s\n", (AIR_IFMON_MODE_INTR == mode) ? "interrupt" : "polling");

    IFMON_CMD_PRINT_FILED_NAME("ifmon interval");
    osal_printf("%u ms\n", interval);

    IFMON_CMD_PRINT_FILED_NAME("ifmon port list");
    _ifmon_cmd_showPortlist(&portlist);
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_ifmon_cmd_setMonitorState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    BOOL_T         enable = FALSE;
    UI32_T         unit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};

    /* ifmon monitor [ unit=<UINT> ] mode={ enable | disable } */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        enable = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        enable = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_ifmon_setMonitorState(unit, enable);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set monitor state fail.\n");
        return rc;
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
static const DSH_VEC_T  _ifmon_cmd_vec[] =
{
    {
        "set", 1, _ifmon_cmd_setMode,
        "ifmon set [ unit=<UINT> ] portlist=<UINTLIST> [ interval=<UINT> ]\n"
    },
    {
        "monitor", 1, _ifmon_cmd_setMonitorState,
        "ifmon monitor [ unit=<UINT> ] mode={ enable | disable }\n"
    },
    {
        "show", 1, _ifmon_cmd_showMode,
        "ifmon show [ unit=<UINT> ]\n"
    }
};
/* clang-format on */

AIR_ERROR_NO_T
ifmon_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    if (DSH_E_NOT_FOUND ==
        dsh_dispatcher(tokens, token_idx, _ifmon_cmd_vec, sizeof(_ifmon_cmd_vec) / sizeof(DSH_VEC_T)))
    {
        return ifmon_cmd_usager();
    }
    return AIR_E_OK;
}

AIR_ERROR_NO_T
ifmon_cmd_usager()
{
    return dsh_usager(_ifmon_cmd_vec, sizeof(_ifmon_cmd_vec) / sizeof(DSH_VEC_T));
}
