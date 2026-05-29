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

#ifdef AIR_SUPPORT_LLDPD
#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_lldp.h"
#include "db_data.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

MW_ERROR_NO_T
_mw_cmd_lldp_glo_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

MW_ERROR_NO_T
_mw_cmd_lldp_tx_interval(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

MW_ERROR_NO_T
_mw_cmd_lldp_tx_hold(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

MW_ERROR_NO_T
_mw_cmd_lldp_reinit_delay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

MW_ERROR_NO_T
_mw_cmd_lldp_tx_delay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
MW_ERROR_NO_T
_mw_cmd_lldp_per_sys_per_mac(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

MW_ERROR_NO_T
_mw_cmd_lldp_not_send_blocking(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif /*AIR_SUPPORT_LLDPD_TX_SEL*/

MW_ERROR_NO_T _mw_cmd_lldp_port_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

#ifdef AIR_SUPPORT_VOICE_VLAN
MW_ERROR_NO_T _mw_cmd_lldp_voice_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
MW_ERROR_NO_T _mw_cmd_lldp_survei_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif

MW_ERROR_NO_T _mw_cmd_lldp_dump(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
MW_ERROR_NO_T mw_cmd_lldp_rstp_port_oper_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
#endif

/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T _mw_cmd_lldp_vec[] =
{
    {
        "set global-cfg", 2, _mw_cmd_lldp_glo_cfg,
        "lldp set global-cfg mode={ enable | disable }\n"
    },
    {
        "set tx-interval", 2, _mw_cmd_lldp_tx_interval,
        "lldp set tx-interval value=<5-32768>\n"
    },
    {
        "set tx-hold", 2, _mw_cmd_lldp_tx_hold,
        "lldp set tx-hold value=<2-10>\n"
    },
    {
        "set reinit-delay", 2, _mw_cmd_lldp_reinit_delay,
        "lldp set reinit-delay value=<1-10>\n"
    },
    {
        "set tx-delay", 2, _mw_cmd_lldp_tx_delay,
        "lldp set tx-delay value=<1-8192>\n"
    },
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    {
        "set per-sys-per-mac", 2, _mw_cmd_lldp_per_sys_per_mac,
        "lldp set per-sys-per-mac mode={ enable | disable }\n"
    },
    {
        "set not-send-blocking", 2, _mw_cmd_lldp_not_send_blocking,
        "lldp set not-send-blocking mode={ enable | disable }\n"
    },
#endif /*AIR_SUPPORT_LLDPD_TX_SEL*/
    {
        "set port-admin", 2, _mw_cmd_lldp_port_cfg,
        "lldp set port-admin portlist=<UNITLIST> mode={ disable | tx | rx | txrx }\n"
    },
#ifdef AIR_SUPPORT_VOICE_VLAN
    {
        "set voice-vlan", 2, _mw_cmd_lldp_voice_cfg,
        "lldp set voice-vlan mode={ enable | disable } vlan_id=<1-4094> vlan_pri=<0-7>\n"
    },
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
    {
        "set survei-vlan", 2, _mw_cmd_lldp_survei_cfg,
        "lldp set survei-vlan mode={ enable | disable } vlan_id=<1-4094> vlan_pri=<0-7>\n"
    },
#endif

    {
        "dump port-info", 2, _mw_cmd_lldp_dump,
        "lldp dump port-info portlist=<UINTLIST> ttl=<0-65535>\n"
    },
#ifdef AIR_SUPPORT_LLDPD_TX_SEL
    {
        "set rstp-oper-cfg", 2, mw_cmd_lldp_rstp_port_oper_cfg,
        "lldp set rstp-oper-cfg portlist=<UINTLIST> state={ disable | learning | forwarding | discarding }"
    },
#endif
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

MW_ERROR_NO_T
_mw_cmd_lldp_glo_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI8_T mode = 0;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "enable", osapi_strlen("enable")))
    {
        mode = TRUE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        mode = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Enable Mode: %d\n", mode);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_GLOBAL_CFG, mode);

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_cmd_lldp_tx_interval(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              value = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "value", osapi_strlen("value")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    rc = osapi_strtou32(ptr_tokens[token_idx++], &value);
    if(MW_E_OK != rc)
    {
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if ((MW_LLDP_TX_INTERVAL_MIN > value) || (MW_LLDP_TX_INTERVAL_MAX < value))
    {
        MW_CMD_OUTPUT(" *** Error: The range of tx-interval must be in %d - %d. ***\n", MW_LLDP_TX_INTERVAL_MIN, MW_LLDP_TX_INTERVAL_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set TX Interval: %d\n", value);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_TX_INTERVAL, value);

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_cmd_lldp_tx_hold(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              value = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "value", osapi_strlen("value")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    rc = osapi_strtou32(ptr_tokens[token_idx++], &value);
    if(MW_E_OK != rc)
    {
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if ((MW_LLDP_TX_HOLD_MIN > value) || (MW_LLDP_TX_HOLD_MAX < value))
    {
        MW_CMD_OUTPUT(" *** Error: The range of tx-hold must be in %d - %d. ***\n", MW_LLDP_TX_HOLD_MIN, MW_LLDP_TX_HOLD_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set TX Hold: %d\n", value);
    lldp_cmd_set(MW_LLDP_CMD_TYPE_TX_HOLD, value);

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_cmd_lldp_reinit_delay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              value = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "value", osapi_strlen("value")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    rc = osapi_strtou32(ptr_tokens[token_idx++], &value);
    if(MW_E_OK != rc)
    {
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if ((MW_LLDP_REINIT_DELAY_MIN > value) || (MW_LLDP_REINIT_DELAY_MAX < value))
    {
        MW_CMD_OUTPUT(" *** Error: The range of reinit delay must be in %d - %d. ***\n", MW_LLDP_REINIT_DELAY_MIN, MW_LLDP_REINIT_DELAY_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set Reinit Delay: %d\n", value);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_REINIT_DELAY, value);

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_cmd_lldp_tx_delay(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              value = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "value", osapi_strlen("value")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    rc = osapi_strtou32(ptr_tokens[token_idx++], &value);
    if(MW_E_OK != rc)
    {
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if ((MW_LLDP_TX_DELAY_MIN > value) || (MW_LLDP_TX_DELAY_MAX < value))
    {
        MW_CMD_OUTPUT(" *** Error: The range of tx delay must be in %d - %d. ***\n", MW_LLDP_TX_DELAY_MIN, MW_LLDP_TX_DELAY_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set Tx Delay: %d\n", value);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_TX_DELAY, value);

    return MW_E_OK;
}

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
MW_ERROR_NO_T
_mw_cmd_lldp_per_sys_per_mac(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI8_T mode = 0;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    if (0 == osapi_strncmp(ptr_tokens[token_idx], "enable", osapi_strlen("enable")))
    {
        mode = TRUE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        mode = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Per Sys Per Mac Mode: %d\n", mode);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_PER_SYS_PER_MAC, mode);

    return MW_E_OK;
}

MW_ERROR_NO_T
_mw_cmd_lldp_not_send_blocking(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI8_T mode = 0;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    if (0 == osapi_strncmp(ptr_tokens[token_idx], "enable", osapi_strlen("enable")))
    {
        mode = TRUE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        mode = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Not Send Blocking Mode: %d\n", mode);

    lldp_cmd_set(MW_LLDP_CMD_TYPE_NOT_SEND_BLOCKING, mode);

    return MW_E_OK;
}
#endif /*AIR_SUPPORT_LLDPD_TX_SEL*/

MW_ERROR_NO_T _mw_cmd_lldp_port_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              unit = 0;
    UI32_T              port = 0;
    UI32_T              mode = 0;
    AIR_PORT_BITMAP_T   pbm = {0};

    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        MW_CMD_OUTPUT("command[mode] error");
        return MW_CMD_E_NOT_FOUND;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        mode = MW_LLDP_PORT_ADMIN_MODE_DISABLE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "txrx", osapi_strlen("txrx")))
    {
        mode = MW_LLDP_PORT_ADMIN_MODE_TXRX;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "tx", osapi_strlen("tx")))
    {
        mode = MW_LLDP_PORT_ADMIN_MODE_TX;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "rx", osapi_strlen("rx")))
    {
        mode = MW_LLDP_PORT_ADMIN_MODE_RX;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Port Cfg Mode: %d\r\n", mode);

    AIR_PORT_FOREACH(pbm, port)
    {
        lldp_port_cmd_set(port, mode);
    }

    return MW_E_OK;
}

#ifdef AIR_SUPPORT_VOICE_VLAN
MW_ERROR_NO_T _mw_cmd_lldp_voice_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              state = 0;
    UI32_T              vlanid = 0;
    UI32_T              vlanpri = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        MW_CMD_OUTPUT("command[mode] error");
        return MW_CMD_E_NOT_FOUND;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "enable", osapi_strlen("enable")))
    {
        state = TRUE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        state = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "vlan_id", osapi_strlen("vlan_id")))
    {
        MW_CMD_OUTPUT("command[vlan_id] error");
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx++], &vlanid);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return MW_CMD_E_SYNTAX_ERR;
    }
    if ((MW_LLDP_VLAN_ID_MIN > vlanid) || (MW_LLDP_VLAN_ID_MAX < vlanid))
    {
        MW_CMD_OUTPUT(" *** Error: The range of vlan id must be in %d - %d. ***\n", MW_LLDP_VLAN_ID_MIN, MW_LLDP_VLAN_ID_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "vlan_pri", osapi_strlen("vlan_pri")))
    {
        MW_CMD_OUTPUT("command[vlan_pri] error");
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx++], &vlanpri);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return MW_CMD_E_SYNTAX_ERR;
    }
    if (MW_LLDP_VLAN_PRI_MAX < vlanpri)
    {
        MW_CMD_OUTPUT(" *** Error: The range of vlan priority must be in %d - %d. ***\n", MW_LLDP_VLAN_PRI_MIN, MW_LLDP_VLAN_PRI_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Voice Vlan Cfg State: %d, Vlan_id: %d, Vlan_pri: %d\r\n", state, vlanid, vlanpri);

    lldp_voice_cfg_set(state, vlanid, vlanpri);

    return MW_E_OK;
}
#endif

#ifdef AIR_SUPPORT_SURVEILLANCE_VLAN
MW_ERROR_NO_T _mw_cmd_lldp_survei_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              state = 0;
    UI32_T              vlanid = 0;
    UI32_T              vlanpri = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "mode", osapi_strlen("mode")))
    {
        MW_CMD_OUTPUT("command[mode] error");
        return MW_CMD_E_NOT_FOUND;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "enable", osapi_strlen("enable")))
    {
        state = TRUE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        state = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "vlan_id", osapi_strlen("vlan_id")))
    {
        MW_CMD_OUTPUT("command[vlan_id] error");
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx++], &vlanid);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return MW_CMD_E_SYNTAX_ERR;
    }
    if ((MW_LLDP_VLAN_ID_MIN > vlanid) || (MW_LLDP_VLAN_ID_MAX < vlanid))
    {
        MW_CMD_OUTPUT(" *** Error: The range of vlan id must be in %d - %d. ***\n", MW_LLDP_VLAN_ID_MIN, MW_LLDP_VLAN_ID_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "vlan_pri", osapi_strlen("vlan_pri")))
    {
        MW_CMD_OUTPUT("command[vlanpri] error");
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx++], &vlanpri);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return MW_CMD_E_SYNTAX_ERR;
    }
    if (MW_LLDP_VLAN_PRI_MAX < vlanpri)
    {
        MW_CMD_OUTPUT(" *** Error: The range of vlan priority must be in %d - %d. ***\n", MW_LLDP_VLAN_PRI_MIN, MW_LLDP_VLAN_PRI_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    MW_CMD_OUTPUT("Set Surveillance Vlan Cfg State: %d, Vlan_id: %d, Vlan_pri: %d\r\n", state, vlanid, vlanpri);

    lldp_survei_cfg_set(state, vlanid, vlanpri);

    return MW_E_OK;
}
#endif

MW_ERROR_NO_T _mw_cmd_lldp_dump(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              unit = 0;
    UI32_T              port = 0;
    UI32_T              ttl = 0;
    MW_ERROR_NO_T       rc = MW_E_OK;
    AIR_PORT_BITMAP_T   pbm = {0};

    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "ttl", osapi_strlen("ttl")))
    {
        MW_CMD_OUTPUT("command[mode] error");
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx++], &ttl);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return MW_CMD_E_SYNTAX_ERR;
    }
    if ((MW_LLDP_TTL_MIN > ttl) || (MW_LLDP_TTL_MAX < ttl))
    {
        MW_CMD_OUTPUT(" *** Error: The range of TTL must be in %d - %d. ***\n", MW_LLDP_TTL_MIN, MW_LLDP_TTL_MAX);
        return MW_CMD_E_SYNTAX_ERR;
    }

    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

    MW_CMD_OUTPUT("Dump Client Info, TTL %d\r\n", ttl);

    AIR_PORT_FOREACH(pbm, port)
    {
        lldp_dump_cmd(port, ttl);
    }



    return MW_E_OK;
}

#ifdef AIR_SUPPORT_LLDPD_TX_SEL
MW_ERROR_NO_T mw_cmd_lldp_rstp_port_oper_cfg(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T              unit = 0;
#ifdef AIR_SUPPORT_RSTP
    UI32_T              port = 0;
    UI32_T              state = 0;
#endif
    AIR_PORT_BITMAP_T   pbm = {0};

    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(ptr_tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "state", osapi_strlen("state")))
    {
        MW_CMD_OUTPUT("command[state] error");
        return MW_CMD_E_NOT_FOUND;
    }

#ifdef AIR_SUPPORT_RSTP
    if (0 == osapi_strncmp(ptr_tokens[token_idx], "disable", osapi_strlen("disable")))
    {
        state = MW_LLDP_RSTP_OPER_STATE_DISABLE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "learning", osapi_strlen("learning")))
    {
        state = MW_LLDP_RSTP_OPER_STATE_LEARNING;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "forwarding", osapi_strlen("forwarding")))
    {
        state = MW_LLDP_RSTP_OPER_STATE_FORWARDING;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "discarding", osapi_strlen("discarding")))
    {
        state = MW_LLDP_RSTP_OPER_STATE_DISCARDING;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;

    MW_CMD_OUTPUT("Set RSTP Port Oper Cfg State: %d\r\n", state);

    AIR_PORT_FOREACH(pbm, port)
    {
        lldp_rstp_port_cmd_set(port, state);
    }

    return MW_E_OK;
#else
    MW_CMD_OUTPUT("Not Support\r\n");

    return MW_E_NOT_SUPPORT;
#endif
}
#endif /* AIR_SUPPORT_LLDPD_TX_SEL */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_lldp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: LLDP.
 *
 * INPUT:
 *      ptr_tokens  --  Command tokens
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
mw_cmd_lldp_dispatcher(
    const C8_T *ptr_tokens[],
    UI32_T token_idx)
{
    return (mw_cmd_dispatcher(ptr_tokens, token_idx, _mw_cmd_lldp_vec, sizeof(_mw_cmd_lldp_vec)/sizeof(MW_CMD_VEC_T)));
}


/* FUNCTION NAME: mw_cmd_lldp_usager
 * PURPOSE:
 *      Command usage for magic wand command: LLDP.
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
mw_cmd_lldp_usager(
    void)
{
    return (mw_cmd_usager(_mw_cmd_lldp_vec, sizeof(_mw_cmd_lldp_vec)/sizeof(MW_CMD_VEC_T)));
}


#endif/*#ifdef AIR_SUPPORT_LLDPD*/

