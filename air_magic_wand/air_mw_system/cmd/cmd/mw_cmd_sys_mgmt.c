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

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_sys_mgmt.h"
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

static MW_ERROR_NO_T _cmd_sys_mgmt_dhcp_enable(
    const C8_T  *tokens[],
    UI32_T      token_idx);
static MW_ERROR_NO_T _cmd_sys_mgmt_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx);
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
static MW_ERROR_NO_T _cmd_sys_mgmt_set_mgmt_vlan_id(
    const C8_T  *tokens[],
    UI32_T      token_idx);
#endif /* AIR_SUPPORT_MGMT_VLAN_CFG */

/* STATIC VARIABLE DECLARATIONS
 */

static const MW_CMD_VEC_T _mw_sys_mgmt_cmd_vec[] =
{
    {
        "set dhcp", 2, _cmd_sys_mgmt_dhcp_enable,
        "sysmt set dhcp mode={ enable | disable } [autodns={ enable | disable } [dns=<ip-addr>]]\n"
    },
    {
        "dump info", 2, _cmd_sys_mgmt_dump,
        "sysmt dump info\n"
    },
#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
    {
        "set mgmt-vlan", 2, _cmd_sys_mgmt_set_mgmt_vlan_id,
        "sysmt set mgmt-vlan vid={ 1 - 4094 }\n"
    },
#endif /* AIR_SUPPORT_MGMT_VLAN_CFG */
};

/* FUNCTION NAME: _cmd_sys_mgmt_dhcp_enable
 * PURPOSE:
 *      Function to enable/disable SYS MGMT DHCP and Auto DNS admin mode.
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
static MW_ERROR_NO_T _cmd_sys_mgmt_dhcp_enable(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    UI32_T mode = 0;
    UI8_T autodnsmode = 0;

    if (0 != osapi_strncmp(tokens[token_idx], "mode", osapi_strlen("mode")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    if (0 == osapi_strncmp(tokens[token_idx + 1], "enable", osapi_strlen("enable")))
    {
        mode = TRUE;
    }
    else if (0 == osapi_strncmp(tokens[token_idx + 1], "disable", osapi_strlen("disable")))
    {
        mode = FALSE;
    }
    else
    {
        return MW_CMD_E_SYNTAX_ERR;
    }

    if (1 == mode)
    {
        if (0 == osapi_strncmp(tokens[token_idx + 2], "autodns", osapi_strlen("autodns")))
        {
            if (0 == osapi_strncmp(tokens[token_idx + 3], "enable", osapi_strlen("enable")))
            {
                autodnsmode = TRUE;
            }
            else if (0 == osapi_strncmp(tokens[token_idx + 3], "disable", osapi_strlen("disable")))
            {
                autodnsmode = FALSE;
            }
            else
            {
                return MW_CMD_E_SYNTAX_ERR;
            }
            MW_CMD_OUTPUT("Set AutoDNS mode: %d\n", autodnsmode);
            sys_mgmt_autodns_enable_cmd_set(autodnsmode);
            if ((FALSE == autodnsmode) && (0 == osapi_strncmp(tokens[token_idx + 4], "dns", osapi_strlen("dns"))))
            {
                ip_addr_t dns = IPADDR4_INIT(0);

                ip_addr_set_ip4_u32_val(dns, getIpaddr((char *)tokens[token_idx + 5]));
                if (IPADDR_NONE == ip4_addr_get_u32(ip_2_ip4(&dns)))
                {
                    MW_CMD_OUTPUT("Invalid parameter.\n");
                    return MW_CMD_E_NOT_FOUND;
                }
                MW_CMD_OUTPUT("Set DNS: 0x%lx\n", ip4_addr_get_u32(ip_2_ip4(&dns)));
                dns_setserver(0, &dns);
            }
        }
    }
    MW_CMD_OUTPUT("Set DHCP Mode: %u\n", mode);
    sys_mgmt_dhcp_enable_cmd_set(mode);

    return MW_E_OK;
}

/* FUNCTION NAME: _cmd_sys_mgmt_dump
 * PURPOSE:
 *      Function to dump SYS MGMT information.
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
static MW_ERROR_NO_T _cmd_sys_mgmt_dump(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    sys_mgmt_dump();

    return MW_E_OK;
}

#ifdef AIR_SUPPORT_MGMT_VLAN_CFG
/* FUNCTION NAME: _cmd_sys_mgmt_set_mgmt_vlan_id
 * PURPOSE:
 *      Function to set MGMT VLAN id
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
static MW_ERROR_NO_T _cmd_sys_mgmt_set_mgmt_vlan_id(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    UI16_T vlan_id;

    if (0 != osapi_strncmp(tokens[token_idx], "vid", osapi_strlen("vid")))
    {
        return MW_CMD_E_NOT_FOUND;
    }

    vlan_id = atoi(tokens[token_idx+1]);
    MW_CMD_OUTPUT("Set MGMT VLAN: %d\n", vlan_id);
    sys_mgmt_set_mgmt_vlan_id(vlan_id);

    return MW_E_OK;
}
#endif /* AIR_SUPPORT_MGMT_VLAN_CFG */

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_sys_mgmt_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: System management.
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
mw_cmd_sys_mgmt_dispatcher(
    const C8_T *ptr_tokens[],
    UI32_T     token_idx)
{
    return (mw_cmd_dispatcher(ptr_tokens, token_idx, _mw_sys_mgmt_cmd_vec, sizeof(_mw_sys_mgmt_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_sys_mgmt_usager
 * PURPOSE:
 *      Command usage for magic wand command: System management.
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
mw_cmd_sys_mgmt_usager(
    void)
{
    return (mw_cmd_usager(_mw_sys_mgmt_cmd_vec, sizeof(_mw_sys_mgmt_cmd_vec) / sizeof(MW_CMD_VEC_T)));
}
