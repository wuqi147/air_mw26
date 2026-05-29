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

#include <cmd/stp_cmd.h>

#include <air_error.h>
#include <air_port.h>
#include <air_stp.h>
#include <air_types.h>
#include <cmd/port_cmd.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>
/* NAMING CONSTANT DECLARATIONS
 */
#define STP_CMD_INVALID_ENTRY (0xFFFFFFFF)
#define STP_CMD_STG_NUM       (16)

/* MACRO FUNCTION DECLARATIONS
 */
#define STP_CMD_STR_STATUS(status) (1 == status) ? "Enable" : "Disable"

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static void
_stp_cmd_printPortList(
    const C8_T             *ptr_str,
    const AIR_PORT_BITMAP_T pbm)
{
    C8_T pbmp_str[CMLIB_UTIL_IP_ADDR_STR_SIZE];

    CMD_STRING_PORTLIST(pbmp_str, pbm);
    osal_printf("%s%s\n", ptr_str, pbmp_str);
}

static void
_stp_cmd_printStgInfo(
    const UI32_T            unit,
    const UI32_T            stg,
    const UI32_T            status,
    const AIR_PORT_BITMAP_T dis_pbm,
    const AIR_PORT_BITMAP_T lrn_pbm,
    const AIR_PORT_BITMAP_T fwd_pbm,
    const AIR_PORT_BITMAP_T lsn_pbm)
{
    osal_printf("group-id : %u\n", stg);
    _stp_cmd_printPortList(" - disable portlist = ", dis_pbm);
    _stp_cmd_printPortList(" - listen  portlist = ", lsn_pbm);
    _stp_cmd_printPortList(" - learn   portlist = ", lrn_pbm);
    _stp_cmd_printPortList(" - forward portlist = ", fwd_pbm);
    osal_printf("\n");
}

static AIR_ERROR_NO_T
_stp_cmd_getPortState(
    const UI32_T      unit,
    const UI32_T      stg,
    AIR_PORT_BITMAP_T dis_pbm,
    AIR_PORT_BITMAP_T lrn_pbm,
    AIR_PORT_BITMAP_T fwd_pbm,
    AIR_PORT_BITMAP_T lsn_pbm)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_STP_STATE_T state;
    UI32_T          port;

    AIR_PORT_BITMAP_CLEAR(dis_pbm);
    AIR_PORT_BITMAP_CLEAR(lrn_pbm);
    AIR_PORT_BITMAP_CLEAR(fwd_pbm);
    AIR_PORT_BITMAP_CLEAR(lsn_pbm);
    AIR_PORT_FOREACH(HAL_PORT_BMP(unit), port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            /* skip CPU port */
            continue;
        }

        rc = air_stp_getPortState(unit, port, stg, &state);

        if (AIR_E_OK == rc)
        {
            switch (state)
            {
                case AIR_STP_STATE_LEARN:
                    CMLIB_BITMAP_BIT_ADD(lrn_pbm, port);
                    break;

                case AIR_STP_STATE_FORWARD:
                    CMLIB_BITMAP_BIT_ADD(fwd_pbm, port);
                    break;

                case AIR_STP_STATE_LISTEN:
                    CMLIB_BITMAP_BIT_ADD(lsn_pbm, port);
                    break;

                case AIR_STP_STATE_DISABLE:
                    CMLIB_BITMAP_BIT_ADD(dis_pbm, port);
                    break;
                default:
                    break;
            }
        }
    }
    return rc;
}

/***********************************
 * Command
 ***********************************/
static AIR_ERROR_NO_T
_stp_cmd_showGroup(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * stp show group [ unit=<UINT> ] group-id=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, stg = STP_CMD_INVALID_ENTRY;
    UI32_T            status = 0;
    AIR_PORT_BITMAP_T dis_pbm, lrn_pbm, fwd_pbm, lsn_pbm;

    /* parse */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "group-id", &stg), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* config */
    if (STP_CMD_INVALID_ENTRY != stg)
    {
        rc = _stp_cmd_getPortState(unit, stg, dis_pbm, lrn_pbm, fwd_pbm, lsn_pbm);
        status = 1;
        if (AIR_E_OK == rc)
        {
            osal_printf("unit %u\n", unit);
            _stp_cmd_printStgInfo(unit, stg, status, dis_pbm, lrn_pbm, fwd_pbm, lsn_pbm);
        }
    }
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show stp %u group error\n", stg);
    }

    return rc;
}

static AIR_ERROR_NO_T
_stp_cmd_setPortState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * stp set port-state [ unit=<UINT> ] group-id=<UINT> portlist=<UINTLIST>
     * state={ disable | listen | learn | forward }
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    AIR_STP_STATE_T   state = AIR_STP_STATE_LAST;
    AIR_PORT_BITMAP_T pbm;
    UI32_T            unit = 0, stg = STP_CMD_INVALID_ENTRY;
    UI32_T            port;
    C8_T              state_str[DSH_CMD_MAX_LENGTH] = {0};

    /*[For tokens idx check function]
     * CMD: air stp set port-state group-id=0 portlist=1 state=2
     * idx:   0   1   2      3          4    5     6    7   8   9
     *
    */

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "group-id", &stg), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", state_str), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(state_str, "disable"))
    {
        state = AIR_STP_STATE_DISABLE;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "listen"))
    {
        state = AIR_STP_STATE_LISTEN;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "learn"))
    {
        state = AIR_STP_STATE_LEARN;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "forward"))
    {
        state = AIR_STP_STATE_FORWARD;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* config */
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_stp_setPortState(unit, port, stg, state);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set stp port=%u port-state error\n", port);
            break;
        }
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _stp_cmd_vec[] =
{
/* (0:disable,1:listen,2:learn,3:forward) */
    {
        "set port-state", 2, _stp_cmd_setPortState,
        "stp set port-state [ unit=<UINT> ] group-id=<UINT> portlist=<UINTLIST>\n"
        "state={ disable | listen | learn | forward }\n"
    },
    {
        "show group", 2, _stp_cmd_showGroup,
        "stp show group [ unit=<UINT> ] group-id=<UINT>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
stp_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    int rv = 0;
    rv = (dsh_dispatcher(tokens, token_idx, _stp_cmd_vec, sizeof(_stp_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}

AIR_ERROR_NO_T
stp_cmd_usager()
{
    int rv = 0;
    rv = (dsh_usager(_stp_cmd_vec, sizeof(_stp_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}
