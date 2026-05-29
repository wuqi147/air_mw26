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

#include <cmd/sflow_cmd.h>

#include <air_port.h>
#include <air_sflow.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

static AIR_ERROR_NO_T
_sflow_cmd_setSampling(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T bitmap = {0};
    UI32_T            rate = 0, number = 0;

    /* cmd: sflow set sampling [ unit=<UINT> ] portlist=<UINTLIST> rate=<UNIT> number=<UNIT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "rate", &rate), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "number", &number), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(bitmap, port)
    {
        rc = air_sflow_setSampling(unit, port, rate, number);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u sampling error\n", port);
        }
    }

    return (rc);
}

static AIR_ERROR_NO_T
_sflow_cmd_showSampling(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T bitmap = {0};
    UI32_T            rate = 0, number = 0;

    /* cmd: sflow show sampling [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &bitmap), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    AIR_PORT_FOREACH(bitmap, port)
    {
        osal_printf(" - port = %u\n", port);
        osal_printf(" - sflow sampling ");
        rc = air_sflow_getSampling(unit, port, &rate, &number);
        if (AIR_E_OK == rc)
        {
            osal_printf("rate = %u, number = %u\n", rate, number);
        }
        else
        {
            osal_printf("***Error***, get port=%u sampling error\n", port);
            break;
        }
        osal_printf("\n");
    }

    return (rc);
}

/* clang-format off */
const static DSH_VEC_T  _sflow_cmd_vec[] =
{
    {
        "set sampling", 2, _sflow_cmd_setSampling,
        "sflow set sampling [ unit=<UINT> ] portlist=<UINTLIST> rate=<UINT> number=<UINT>\n"
    },
    {
        "show sampling", 2, _sflow_cmd_showSampling,
        "sflow show sampling [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
sflow_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _sflow_cmd_vec, sizeof(_sflow_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
sflow_cmd_usager()
{
    return (dsh_usager(_sflow_cmd_vec, sizeof(_sflow_cmd_vec) / sizeof(DSH_VEC_T)));
}
