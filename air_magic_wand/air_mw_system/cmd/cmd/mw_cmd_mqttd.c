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

#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"
#include "mw_cmd_util.h"
#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"
#include "mw_cmd_mqttd.h"

#ifdef AIR_SUPPORT_MQTTD
#include "mqttd.h"
#include "inet_utils.h"
#include "sys_mgmt.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T _mqttd_cmd_enable(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _mqttd_cmd_dump_topic(const C8_T *tokens[], UI32_T token_idx);
static MW_ERROR_NO_T _mqttd_cmd_show_state(const C8_T *tokens[], UI32_T token_idx);


/* GLOBAL VARIABLE DECLARATIONS
*/
static const MW_CMD_VEC_T _mw_mqttd_cmd_vec[] =
{
    {
        "set state", 2, _mqttd_cmd_enable,
        "mqttd set state={ enable [ server-ip=<IP address> ] | disable }\n"
    },
    {
        "dump topic", 2, _mqttd_cmd_dump_topic,
        "mqttd dump topic\n"
    },
    {
        "show state", 2, _mqttd_cmd_show_state,
        "mqttd show state\n"
    }
};

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
/* cmd: mqttd state { enable [server=<IPv4 address>] | disable }
*/
static MW_ERROR_NO_T
_mqttd_cmd_enable(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T enable;
    MW_IPV4_T value = 0;
    ip_addr_t mqttd_server_ip;

    /* Parser tokens */
    if(MW_E_OK == mw_cmd_checkString(tokens[token_idx], "enable"))
    {
        token_idx++;
        if (NULL != tokens[token_idx])
        {
            if (MW_E_OK == mw_cmd_getIpv4Addr(tokens, token_idx, "server-ip", &value))
            {
                value = ntohl(value);
                MW_CMD_OUTPUT("Server IP is: %s(%ul)\n", tokens[token_idx + 1], value);
                if ((IPADDR_ANY == value) ||
                    (IPADDR_LOOPBACK == value) ||
                    (IPADDR_BROADCAST == value) ||
                    (MW_IPV4_IS_MULTICAST(value)) ||
                    (IPADDR_NONE == value))
                {
                    MW_CMD_OUTPUT("Invalid IP address of MQTT remoter server.\n");
                    return MW_E_BAD_PARAMETER;
                }
                token_idx += 2;
            }
            MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
        }
        enable = TRUE;
        MW_CMD_OUTPUT("Start the MQTT daemon\n");
    }
    else if(MW_E_OK == mw_cmd_checkString(tokens[token_idx], "disable"))
    {
        token_idx++;
        MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);
        enable = FALSE;
        MW_CMD_OUTPUT("Stop the MQTT daemon\n");
    }
    else
    {
        return MW_E_BAD_PARAMETER;
    }
    ip_addr_set_ip4_u32_val(mqttd_server_ip, value);
    sys_mgmt_mqttd_enable_cmd_set(enable, (void *)&mqttd_server_ip);

    return ret;
}

/* cmd: mqttd dump topic
*/
static MW_ERROR_NO_T
_mqttd_cmd_dump_topic(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    mqttd_dump_topic();

    return MW_E_OK;
}

/* cmd: mqttd show state
*/
static MW_ERROR_NO_T
_mqttd_cmd_show_state(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    mqttd_show_state();

    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_mqttd_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: MQTTD.
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
mw_cmd_mqttd_dispatcher(
    const C8_T                  *tokens[],
    UI32_T                      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_mqttd_cmd_vec, sizeof(_mw_mqttd_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_mqttd_usager
 * PURPOSE:
 *      Command usage for magic wand command: MQTTD.
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
mw_cmd_mqttd_usager(
    void)
{
    return (mw_cmd_usager(_mw_mqttd_cmd_vec, sizeof(_mw_mqttd_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

#endif /* AIR_SUPPORT_MQTTD */
