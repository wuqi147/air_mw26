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

#include <cmd/mib_cmd.h>

#include <air_error.h>
#include <air_mib.h>
#include <air_port.h>
#include <air_types.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* -------------------------------------------------------------- counter */
static void
_pkt_cmd_printTxCnt(
    const UI32_T            port,
    const AIR_MIB_CNT_TX_T *ptr_cnt)
{
    UI32_T numHigh, numMed, numLow, tmp;

    osal_printf("tx port %d:\n", port);
    osal_printf(" tx collision drop packet          : %u\n", ptr_cnt->TCDPC);
    osal_printf(" tx fcs packet                     : %u\n", ptr_cnt->TCEPC);
    osal_printf(" tx unicast packet                 : %u\n", ptr_cnt->TUPC);
    osal_printf(" tx multicast packet               : %u\n", ptr_cnt->TMPC);
    osal_printf(" tx broadcast packet               : %u\n", ptr_cnt->TBPC);
    osal_printf(" tx collision event count          : %u\n", ptr_cnt->TCEC);
    osal_printf(" tx single collision event count   : %u\n", ptr_cnt->TSCEC);
    osal_printf(" tx multiple conllision event count: %u\n", ptr_cnt->TMCEC);
    osal_printf(" tx deferred event count           : %u\n", ptr_cnt->TDEC);
    osal_printf(" tx late collision event count     : %u\n", ptr_cnt->TLCEC);
    osal_printf(" tx excessive collision event count: %u\n", ptr_cnt->TXCEC);
    osal_printf(" tx pause packet                   : %u\n", ptr_cnt->TPPC);
    osal_printf(" tx packet length 64 bytes         : %u\n", ptr_cnt->TL64PC);
    osal_printf(" tx packet length 65 ~ 127 bytes   : %u\n", ptr_cnt->TL65PC);
    osal_printf(" tx packet length 128 ~ 255 bytes  : %u\n", ptr_cnt->TL128PC);
    osal_printf(" tx packet length 256 ~ 511 bytes  : %u\n", ptr_cnt->TL256PC);
    osal_printf(" tx packet length 512 ~ 1023 bytes : %u\n", ptr_cnt->TL512PC);
    osal_printf(" tx packet length 1024 ~1518 bytes : %u\n", ptr_cnt->TL1024PC);
    osal_printf(" tx packet length 1519 ~ max bytes : %u\n", ptr_cnt->TL1519PC);
    osal_printf(" tx octets count                   : ");
    numHigh = ptr_cnt->TOC / 10000000000;
    tmp = ptr_cnt->TOC % 10000000000;
    numMed = tmp / 1000000000;
    tmp = tmp % 1000000000;
    numLow = tmp % 1000000000;
    if (numHigh)
    {
        osal_printf("%u%u%u\n", numHigh, numMed, numLow);
    }
    else if (numMed)
    {
        osal_printf("%u%u\n", numMed, numLow);
    }
    else
    {
        osal_printf("%u\n", numLow);
    }
    osal_printf(" tx oversize drop packet           : %u\n", ptr_cnt->TODPC);
    osal_printf("\n");
}

static void
_pkt_cmd_printRxCnt(
    const UI32_T            port,
    const AIR_MIB_CNT_RX_T *ptr_cnt)
{
    UI32_T numHigh, numMed, numLow, tmp;

    osal_printf("rx port %d:\n", port);
    osal_printf(" rx drop packet                   : %u\n", ptr_cnt->RDPC);
    osal_printf(" rx filtering packet              : %u\n", ptr_cnt->RFPC);
    osal_printf(" rx unicast packet                : %u\n", ptr_cnt->RUPC);
    osal_printf(" rx multicast packet              : %u\n", ptr_cnt->RMPC);
    osal_printf(" rx broadcast packet              : %u\n", ptr_cnt->RBPC);
    osal_printf(" rx alignment error packet        : %u\n", ptr_cnt->RAEPC);
    osal_printf(" rx crc packet                    : %u\n", ptr_cnt->RCEPC);
    osal_printf(" rx undersize packet              : %u\n", ptr_cnt->RUSPC);
    osal_printf(" rx fragment error packet         : %u\n", ptr_cnt->RFEPC);
    osal_printf(" rx oversize packet               : %u\n", ptr_cnt->ROSPC);
    osal_printf(" rx jabber error packet           : %u\n", ptr_cnt->RJEPC);
    osal_printf(" rx pause packet                  : %u\n", ptr_cnt->RPPC);
    osal_printf(" rx packet length 64 bytes        : %u\n", ptr_cnt->RL64PC);
    osal_printf(" rx packet length 65 ~ 127 bytes  : %u\n", ptr_cnt->RL65PC);
    osal_printf(" rx packet length 128 ~ 255 bytes : %u\n", ptr_cnt->RL128PC);
    osal_printf(" rx packet length 256 ~ 511 bytes : %u\n", ptr_cnt->RL256PC);
    osal_printf(" rx packet length 512 ~ 1023 bytes: %u\n", ptr_cnt->RL512PC);
    osal_printf(" rx packet length 1024 ~1518 bytes: %u\n", ptr_cnt->RL1024PC);
    osal_printf(" rx packet length 1519 ~ max bytes: %u\n", ptr_cnt->RL1519PC);
    osal_printf(" rx octets count                  : ");
    numHigh = ptr_cnt->ROC / 10000000000;
    tmp = ptr_cnt->ROC % 10000000000;
    numMed = tmp / 1000000000;
    tmp = tmp % 1000000000;
    numLow = tmp % 1000000000;
    if (numHigh)
    {
        osal_printf("%u%u%u\n", numHigh, numMed, numLow);
    }
    else if (numMed)
    {
        osal_printf("%u%u\n", numMed, numLow);
    }
    else
    {
        osal_printf("%u\n", numLow);
    }
    osal_printf(" rx ctrl drop packet              : %u\n", ptr_cnt->RCDPC);
    osal_printf(" rx ingress drop packet           : %u\n", ptr_cnt->RIDPC);
    osal_printf(" rx arl drop packet               : %u\n", ptr_cnt->RADPC);
    osal_printf("\n");
}

static void
_pkt_cmd_printFeatureCnt(
    const UI32_T         port,
    const AIR_MIB_CNT_T *ptr_cnt)
{
    osal_printf("port %d feature counter:\n", port);
    osal_printf(" flow contol drop packet          : %u\n", ptr_cnt->FCDPC);
    osal_printf(" wred drop packet                 : %u\n", ptr_cnt->WRDPC);
    osal_printf(" mirror drop packet               : %u\n", ptr_cnt->MRDPC);
    osal_printf(" rx sflow sampling packet         : %u\n", ptr_cnt->RSFSPC);
    osal_printf(" rx sflow total packet            : %u\n", ptr_cnt->RSFTPC);
    osal_printf("\n");
}

static AIR_ERROR_NO_T
_mib_cmd_showCounter(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T   rc = AIR_E_OK;
    UI32_T           unit = 0;
    C8_T             type_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T           port;
    AIR_MIB_CNT_RX_T rx_cnt;
    AIR_MIB_CNT_TX_T tx_cnt;

    osal_strncpy(type_str, "both", sizeof("both"));
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_mib_getPortCnt(unit, port, &rx_cnt, &tx_cnt);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get mib counter fail\n");
        return (rc);
    }

    osal_printf("unit %u\n", unit);
    if (AIR_E_OK == dsh_checkString(type_str, "both"))
    {
        /* default show rx and rx */
        _pkt_cmd_printTxCnt(port, &tx_cnt);
        _pkt_cmd_printRxCnt(port, &rx_cnt);
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "tx"))
    {
        _pkt_cmd_printTxCnt(port, &tx_cnt);
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rx"))
    {
        _pkt_cmd_printRxCnt(port, &rx_cnt);
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    return (rc);
}

static AIR_ERROR_NO_T
_mib_cmd_showFeatureCounter(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         port;
    AIR_MIB_CNT_T  feature_cnt;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_mib_getFeatureCnt(unit, port, &feature_cnt);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get mib counter fail\n");
        return (rc);
    }

    osal_printf("unit %u\n", unit);
    _pkt_cmd_printFeatureCnt(port, &feature_cnt);

    return (rc);
}

static AIR_ERROR_NO_T
_mib_cmd_clearCounter(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         port = 0;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (port == 0) /* clear all port counter */
    {
        rc = air_mib_clearAllCnt(unit);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear all mib counter fail\n");
            return (rc);
        }
        osal_printf("clear all mib counter success");
    }
    else /* clear specified port counter */
    {
        rc = air_mib_clearPortCnt(unit, port);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, clear port %d mib counter fail\n", port);
            return (rc);
        }
        osal_printf("clear port %d mib counter success\n", port);
    }

    return (rc);
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _mib_cmd_vec[] =
{
    {
        "show counter", 2, _mib_cmd_showCounter,
        "mib show counter [ unit=<UINT> ] port=<UINT> [ type={ tx | rx } ]\n"
    },
    {
        "show feature-counter", 2, _mib_cmd_showFeatureCounter,
        "mib show feature-counter [ unit=<UINT> ] port=<UINT> \n"
    },
    {
        "clear counter", 2, _mib_cmd_clearCounter,
        "mib clear counter [ unit=<UINT> ] [ port=<UINT> ]\n"
    }
};
/* clang-format on */

AIR_ERROR_NO_T
mib_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _mib_cmd_vec, sizeof(_mib_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
mib_cmd_usager()
{
    return (dsh_usager(_mib_cmd_vec, sizeof(_mib_cmd_vec) / sizeof(DSH_VEC_T)));
}
