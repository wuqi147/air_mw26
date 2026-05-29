/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

#include <mw_error.h>
#include <mw_types.h>

#include <osapi.h>
#include <osapi_string.h>

#include <mw_cmd_parser.h>
#include <mw_cmd_util.h>
#include <mw_cmd_dhcp_snoop.h>

#ifdef AIR_SUPPORT_DHCP_SNOOP
#include <dhcp_snoop.h>

/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T
_dhcpsnp_cmd_setEnable(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_setTrustPort(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_setOpt82Enable(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_setOpt82Mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_setCircuitId(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_setRemoteId(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T
_dhcpsnp_cmd_dump(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx);

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T _mw_dhcpsnp_cmd_vec[] =
{
    {
        "set mode", 2, _dhcpsnp_cmd_setEnable,
        "dhcpsnp set mode mode={ enable | disable }\n"
    },
    {
        "set trust-port", 2, _dhcpsnp_cmd_setTrustPort,
        "dhcpsnp set trust-port port=<UINT> mode={ enable | disable }\n"
    },
    {
        "set opt82", 2, _dhcpsnp_cmd_setOpt82Enable,
        "dhcpsnp set opt82 port=<UINT> mode={ enable | disable }\n"
    },
    {
        "set opt82-op", 2, _dhcpsnp_cmd_setOpt82Mode,
        "dhcpsnp set opt82-op port=<UINT> op={ keep | replace | drop }\n"
    },
    {
        "set opt82-circuit-id", 2, _dhcpsnp_cmd_setCircuitId,
        "dhcpsnp set opt82-circuit-id port=<UINT> type={ default | user-config } [id=<str>]\n"
    },
    {
        "set opt82-remote-id", 2, _dhcpsnp_cmd_setRemoteId,
        "dhcpsnp set opt82-remote-id port=<UINT> type={ mac | ip | user-config } [id=<str>]\n"
    },
    {
        "show info", 2, _dhcpsnp_cmd_dump,
        "dhcpsnp show info\n"
    }

};

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
/* cmd: dhcpsnp set mode={ enable | disable }
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setEnable(
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

    dhcp_snp_cmd_setEnable(mode);
    return MW_E_OK;
}

/* cmd: dhcpsnp trust_port portlist=<UINTLIST>
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setTrustPort(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T          port = 0;
    UI32_T          mode = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "port", osapi_strlen("port")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx], &port);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return rc;
    }

    token_idx++;

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
    if ((port < 1) || (port > PLAT_MAX_PORT_NUM))
    {
        MW_CMD_OUTPUT(" *** Error: The range of port must be in 1 - %d. ***\n", PLAT_MAX_PORT_NUM);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set Trust Port: %d\n", mode);

    dhcp_snp_cmd_setTrustPort(port, mode);
    return MW_E_OK;
}

/* cmd: dhcpsnp opt82_set port=<UINT> mode={ enable | disable }
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setOpt82Enable(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T          port = 0;
    UI32_T          mode = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "port", osapi_strlen("port")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx], &port);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return rc;
    }

    token_idx++;

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
    if ((port < 1) || (port > PLAT_MAX_PORT_NUM))
    {
        MW_CMD_OUTPUT(" *** Error: The range of port must be in 1 - %d. ***\n", PLAT_MAX_PORT_NUM);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set Option82 Enable Mode: %d\n", mode);

    dhcp_snp_cmd_setOption82Enable(port, mode);
    return MW_E_OK;
}

/* cmd: dhcpsnp opt82_set_op port=<UINT> op={ keep | replace | drop }
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setOpt82Mode(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T          port = 0;
    UI32_T          op = 0;
    MW_ERROR_NO_T   rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "port", osapi_strlen("port")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx], &port);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return rc;
    }

    token_idx++;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "op", osapi_strlen("op")))
    {
        return MW_CMD_E_NOT_FOUND;
    }
    if (0 == osapi_strncmp(ptr_tokens[token_idx], "keep", osapi_strlen("keep")))
    {
        op = OPT82_MODE_KEEP;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "replace", osapi_strlen("replace")))
    {
        op = OPT82_MODE_REPLACE;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "drop", osapi_strlen("drop")))
    {
        op = OPT82_MODE_DROP;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;
    MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);
    if ((port < 1) || (port > PLAT_MAX_PORT_NUM))
    {
        MW_CMD_OUTPUT(" *** Error: The range of port must be in 1 - %d. ***\n", PLAT_MAX_PORT_NUM);
        return MW_CMD_E_SYNTAX_ERR;
    }
    MW_CMD_OUTPUT("Set Option82 Operation: %d\n", op);

    dhcp_snp_cmd_setOption82Mode(port, op);
    return MW_E_OK;
}

/* cmd: dhcpsnp opt82_circuit_id port=<UINT> type={ default | user-config } [id=<str>]
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setCircuitId(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T          port = 0;
    UI16_T          type = 0;
    C8_T            id[MW_CMD_CMD_MAX_LENGTH] = {0};
    MW_ERROR_NO_T   rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "port", osapi_strlen("port")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx], &port);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return rc;
    }

    token_idx++;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "type", osapi_strlen("type")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    if ((port < 1) || (port > PLAT_MAX_PORT_NUM))
    {
        MW_CMD_OUTPUT(" *** Error: The range of port must be in 1 - %d. ***\n", PLAT_MAX_PORT_NUM);
        return MW_CMD_E_SYNTAX_ERR;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "default", osapi_strlen("default")))
    {
        type = OPT82_CIR_DEFAULT_ID;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "user-config", osapi_strlen("user-config")))
    {
        type = OPT82_CIR_USER_ID;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;

    if (type == OPT82_CIR_USER_ID)
    {
        if (0 == osapi_strncmp(ptr_tokens[token_idx++], "id", osapi_strlen("id")))
        {
            osapi_strncpy(id, ptr_tokens[token_idx], sizeof(id) - 1);
            id[sizeof(id) - 1] = '\0';
            ++token_idx;
        }
        MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

        if ((osapi_strlen(id) + 1) > MAX_OPT82_ID_LEN)
        {
            MW_CMD_OUTPUT(" *** Error: The length of id is too long. ***\n");
            return MW_CMD_E_SYNTAX_ERR;
        }
    }

    MW_CMD_OUTPUT("Set circuit id port: %d, type = %d, id = %s, len = %lu\n", port, type, id, osapi_strlen(id));

    dhcp_snp_cmd_setOption82CircuitId(port, type, id, osapi_strlen(id));
    return MW_E_OK;
}

/* cmd: dhcpsnp opt82_remote_id port=<UINT> type={ mac | ip | user-config } [id=<str>]
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_setRemoteId(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    UI32_T          port = 0;
    UI16_T          type = 0;
    C8_T            id[MW_CMD_CMD_MAX_LENGTH] = {0};
    MW_ERROR_NO_T   rc = MW_E_OK;

    /* Parser ptr_tokens */
    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "port", osapi_strlen("port")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    rc = osapi_strtou32(ptr_tokens[token_idx], &port);
    if (rc != MW_E_OK)
    {
        /* Conversion failed: not a valid integer string */
        return rc;
    }

    token_idx++;

    if (0 != osapi_strncmp(ptr_tokens[token_idx++], "type", osapi_strlen("type")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    if ((port < 1) || (port > PLAT_MAX_PORT_NUM))
    {
        MW_CMD_OUTPUT(" *** Error: The range of port must be in 1 - %d. ***\n", PLAT_MAX_PORT_NUM);
        return MW_CMD_E_SYNTAX_ERR;
    }

    if (0 == osapi_strncmp(ptr_tokens[token_idx], "mac", osapi_strlen("mac")))
    {
        type = OPT82_REM_DEFAULT_ID_MAC;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "ip", osapi_strlen("ip")))
    {
        type = OPT82_REM_DEFAULT_ID_IP;
    }
    else if (0 == osapi_strncmp(ptr_tokens[token_idx], "user-config", osapi_strlen("user-config")))
    {
        type = OPT82_REM_USER_ID;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }
    token_idx++;

    if (type == OPT82_REM_USER_ID)
    {
        if (0 == osapi_strncmp(ptr_tokens[token_idx++], "id", osapi_strlen("id")))
        {
            osapi_strncpy(id, ptr_tokens[token_idx], sizeof(id) - 1);
            id[sizeof(id) - 1] = '\0';
            ++token_idx;
        }
        MW_CMD_CHECK_LAST_TOKEN(ptr_tokens[token_idx]);

        if ((osapi_strlen(id) + 1) > MAX_OPT82_ID_LEN)
        {
            MW_CMD_OUTPUT(" *** Error: The length of id is too long. ***\n");
            return MW_CMD_E_SYNTAX_ERR;
        }
    }
    MW_CMD_OUTPUT("Set remote id port: %d, type = %d, id = %s, len = %lu\n", port, type, id, osapi_strlen(id));

    dhcp_snp_cmd_setOption82RemoteId(port, type, id, osapi_strlen(id));
    return MW_E_OK;
}

/* cmd: dhcpsnp dump
*/
static MW_ERROR_NO_T
_dhcpsnp_cmd_dump(
    const C8_T  *ptr_tokens[],
    UI32_T      token_idx)
{
    dhcp_snp_db_dump();
    return MW_E_OK;
}

/* STATIC VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_dhcp_snoop_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: DHCP snooping.
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
mw_cmd_dhcp_snoop_dispatcher(
    const C8_T *ptr_tokens[],
    UI32_T     token_idx)
{
    return (mw_cmd_dispatcher(ptr_tokens, token_idx, _mw_dhcpsnp_cmd_vec, sizeof(_mw_dhcpsnp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_dhcp_snoop_usager
 * PURPOSE:
 *      Command usage for magic wand command: DHCP snooping.
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
mw_cmd_dhcp_snoop_usager(
    void)
{
    return (mw_cmd_usager(_mw_dhcpsnp_cmd_vec, sizeof(_mw_dhcpsnp_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}
#endif /* AIR_SUPPORT_DHCP_SNOOP */
