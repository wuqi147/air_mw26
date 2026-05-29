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

#include <mw_error.h>
#include <mw_types.h>

#include <osapi.h>
#include <osapi_string.h>

#include <mw_cmd_parser.h>
#include <mw_cmd_util.h>
#include <mw_cmd_ver.h>
#ifdef AIR_SUPPORT_LP
#include <mw_cmd_lp.h>
#endif
#include <mw_cmd_syncd.h>
#ifdef AIR_SUPPORT_IGMP_SNP
#include <mw_cmd_igmp_snoop.h>
#endif
#include <mw_cmd_sys_mgmt.h>
#include <mw_cmd_db.h>
#include <mw_cmd_log.h>
#ifdef AIR_SUPPORT_DHCP_SNOOP
#include <mw_cmd_dhcp_snoop.h>
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
#include <mw_cmd_stp.h>
#endif /* AIR_SUPPORT_RSTP */
#ifdef AIR_SUPPORT_MQTTD
#include "mw_cmd_mqttd.h"
#endif
#ifdef AIR_SUPPORT_LLDPD
#include <mw_cmd_lldp.h>
#endif /* AIR_SUPPORT_LLDPD */
#ifdef AIR_SUPPORT_SFP
#include "mw_cmd_sfp.h"
#endif /* AIR_SUPPORT_SFP */
#ifdef AIR_SUPPORT_ERPS
#include "mw_cmd_erps.h"
#endif /* AIR_SUPPORT_ERPS */
#ifdef AIR_SUPPORT_LACP
#include "mw_cmd_lacp.h"
#endif /* AIR_SUPPORT_LACP */
#ifdef AIR_SUPPORT_VOICE_VLAN
#include <mw_cmd_vlan.h>
#endif

/* NAMING CONSTANT DECLARATIONS
 */
static MW_ERROR_NO_T
_mw_cmd_help(
    const C8_T *tokens[],
    UI32_T      token_idx);

static MW_ERROR_NO_T
_mw_cmd_usager(
    void);

/* MACRO FUNCTION DECLARATIONS
 */
#define MW_CMD_IS_CONTORL_CH(__ch__)                             \
    ((__ch__) <= 0x1F)? TRUE :                                   \
    ((__ch__) == 0x7F)? TRUE : FALSE

#define MW_CMD_PRINT_USAGE(__ptr_usage__)                        \
{                                                                \
    MW_CMD_OUTPUT("%s", __ptr_usage__);                           \
}                                                                \

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_TOP_T _mw_cmd_vec[] =
{
    /* key            description               callback function               show usage function     */
    { "help",        "Help",                    _mw_cmd_help,                   _mw_cmd_usager           },
    { "ver",         "MW Version",              mw_cmd_ver_dispatcher,          mw_cmd_ver_usager        },
    { "log",         "MW Log",                  mw_cmd_log_dispatcher,          mw_cmd_log_usager        },
#ifdef AIR_SUPPORT_LP
    { "lp",          "Loop Prevention",         mw_cmd_lp_dispatcher,           mw_cmd_lp_usager         },
#endif
    { "syncd",       "SyncD",                   mw_cmd_syncd_dispatcher,        mw_cmd_syncd_usager      },

#ifdef AIR_SUPPORT_IGMP_SNP
    { "igsn",        "IGMP Snooping",           mw_cmd_igmp_snp_dispatcher,     mw_cmd_igmp_snp_usager   },
#endif

    /* SYS_MGMT test cmds */
    { "sysmt",       "System management",       mw_cmd_sys_mgmt_dispatcher,     mw_cmd_sys_mgmt_usager   },

    /* DB test cmds */
    { "db",          "DB",                      mw_cmd_db_dispatcher,           mw_cmd_db_usager         },
#ifdef AIR_SUPPORT_DHCP_SNOOP
    /* DHCP snooping test cmds */
    { "dhcpsnp",     "DHCP snooping",           mw_cmd_dhcp_snoop_dispatcher,   mw_cmd_dhcp_snoop_usager },
#endif /* AIR_SUPPORT_DHCP_SNOOP */
#if defined(AIR_SUPPORT_RSTP) || defined(AIR_SUPPORT_MSTP)
    { "stp",         "Spanning Tree Protocol",  mw_cmd_stp_dispatcher,          mw_cmd_stp_usager        },
#endif /* AIR_SUPPORT_RSTP || AIR_SUPPORT_MSTP*/
#ifdef AIR_SUPPORT_MQTTD
    /* MQTTD test cmds */
    { "mqttd",       "MQTTD",                   mw_cmd_mqttd_dispatcher,        mw_cmd_mqttd_usager      },
#endif
#ifdef AIR_SUPPORT_SFP
    /* SFP test cmds */
    { "sfp",         "SFP",                     mw_cmd_sfp_dispatcher,          mw_cmd_sfp_usager        },
#endif
#ifdef AIR_SUPPORT_LLDPD
    {"lldp",         "LLDP",                    mw_cmd_lldp_dispatcher,         mw_cmd_lldp_usager       },
#endif
#ifdef AIR_SUPPORT_ERPS
    { "erps",        "erps",                    mw_cmd_erps_dispatcher,         mw_cmd_erps_usager       },
#endif
#ifdef AIR_SUPPORT_LACP
    { "lacp",        "LACP",                    mw_cmd_lacp_dispatcher,         mw_cmd_lacp_usager       },
#endif
#ifdef AIR_SUPPORT_VOICE_VLAN
    { "vlan",        "VLAN",                    mw_cmd_vlan_dispatcher,         mw_cmd_vlan_usager       },
#endif
};

static const UI32_T     _mw_cmd_vec_num = sizeof(_mw_cmd_vec) / sizeof(MW_CMD_VEC_TOP_T);

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */
static MW_ERROR_NO_T
_mw_cmd_help(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    UI32_T idx = 0;

    MW_CMD_OUTPUT("\n");
    MW_CMD_OUTPUT("MW debug command: \n");

    for (idx = 0; idx < _mw_cmd_vec_num; idx++)
    {
        MW_CMD_OUTPUT("   %-12s  %-s command \n", _mw_cmd_vec[idx].ptr_key, _mw_cmd_vec[idx].ptr_desc);
    }

    MW_CMD_OUTPUT("\n");
    return MW_E_OK;
}

static MW_ERROR_NO_T
_mw_cmd_usager(
    void)
{
    MW_CMD_OUTPUT("***unexpected error***\n");
    return MW_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_dispatcher
 * PURPOSE:
 *      Command usage for magic wand command.
 *
 * INPUT:
 *      ptr_tokens  --  Pointer of command tokens
 *      token_idx   --  The index of 1st valid token
 *      vec         --  Command vector array
 *      vec_num     --  The number of command vector
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
mw_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T token_idx,
    const MW_CMD_VEC_T vec[],
    const UI32_T vec_num)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T idx = 0, jdx = 0, len = 0;
    UI32_T tok_num = 0, tok_idx = 0, tok_len = 0; /* reset per vector */
    C8_T *ptr_key = NULL;

    /* for each vector */
    for (idx = 0; idx < vec_num; idx++)
    {
        rc = MW_E_OK;
        tok_idx = token_idx;
        tok_num = vec[idx].token_num;   /* e.g. 2        */
        ptr_key = vec[idx].ptr_key;     /* e.g. "set rx" */
        tok_len = 0;

        /* for each token, e.g. "set" */
        for (jdx = 0; jdx < tok_num; jdx++)
        {
            if (NULL == tokens[tok_idx])
            {
                rc = MW_CMD_E_KEY_MISMATCH;
                break;
            }

            /* loop compare the vec-key and token */
            len = osapi_strlen(tokens[tok_idx]);

            if (0 != osapi_strncmp(ptr_key, tokens[tok_idx], len))
            {
                rc = MW_CMD_E_KEY_MISMATCH;
                break;
            }
            ptr_key += len + 1;
            tok_idx += 1;
            tok_len += len + 1;
        }

        /* if token len is not match, e.g. "set" + "rx" + 2 - 1 != "set rx" */
        if (MW_E_OK == rc)
        {
            if ((tok_len - 1) != osapi_strlen(vec[idx].ptr_key))
            {
                rc = MW_CMD_E_KEY_MISMATCH;
            }
        }

        /* if fully-match */
        if (MW_E_OK == rc)
        {
            rc = vec[idx].callback(tokens, tok_idx);
            if ((MW_E_BAD_PARAMETER == rc) || (MW_CMD_E_SYNTAX_ERR == rc))
            {
                MW_CMD_OUTPUT("syntax error:\n");
                MW_CMD_PRINT_USAGE(vec[idx].ptr_usage);
                MW_CMD_OUTPUT("\n");
            }

            return (rc);
        }
    }

    /* miss all of the vectors */
    return (MW_CMD_E_NOT_FOUND);
}

/* FUNCTION NAME: mw_cmd_usager
 * PURPOSE:
 *      Command usage for magic wand command.
 *
 * INPUT:
 *      vec         --  Command vector array
 *      vec_num     --  The number of command vector
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
mw_cmd_usager(
    const MW_CMD_VEC_T vec[],
    const UI32_T vec_num)
{
    UI32_T idx = 0, num = 1;

    MW_CMD_OUTPUT("\n");
    if (vec_num > 0)
    {
        MW_CMD_OUTPUT("Usages:\n");
    }
    for (idx = 0; idx < vec_num; idx++)
    {
        if (NULL != vec[idx].ptr_usage)
        {
            MW_CMD_OUTPUT("%2d. ", num++);
            MW_CMD_PRINT_USAGE(vec[idx].ptr_usage);
            MW_CMD_OUTPUT("\n");
        }
    }

    return (MW_E_OK);
}

/* FUNCTION NAME: mw_cmd_handleString
 * PURPOSE:
 *      String handler for magic wand command.
 *
 * INPUT:
 *      ptr_str     --  Pointer of input string
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_CMD_E_KEY_MISMATCH
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_cmd_handleString(
    C8_T *ptr_str)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T idx = 0;
    C8_T *tokens[MW_CMD_CMD_MAX_TOKENS] = { 0 };

    if (FALSE == MW_CMD_IS_CONTORL_CH(ptr_str[0]))
    {
        osapi_memset(tokens, 0, sizeof(C8_T *) * MW_CMD_CMD_MAX_TOKENS);
        rc = mw_cmd_splitString(tokens, ptr_str);
        if (MW_E_OK != rc)
        {
            MW_LOG_ERROR(CMD, "Input too many tokens.\n");
            return rc;
        }

        /* check empty string */
        if (NULL == tokens[0])
        {
            /* show air command list when command is : air */
            return (MW_CMD_E_KEY_MISMATCH);
        }

        /* check the key */
        for (idx = 0; idx < _mw_cmd_vec_num; idx++)
        {
            if (MW_E_OK == mw_cmd_checkString(tokens[0], _mw_cmd_vec[idx].ptr_key))
            {
                /* callback with token_idx 1 */
                rc = _mw_cmd_vec[idx].callback((const C8_T **)tokens, 1);
                if (MW_CMD_E_NOT_FOUND == rc)
                {
                    _mw_cmd_vec[idx].usage();
                }
                return (rc);
            }
        }

        /* miss all of the keys */
        return (MW_CMD_E_KEY_MISMATCH);
    }

    return (MW_E_OK);
}

/* FUNCTION NAME: mw_cmd_parseString
 * PURPOSE:
 *      String parser for magic wand command.
 *
 * INPUT:
 *      ptr_str     --  Pointer of input string
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
mw_cmd_parseString(
    C8_T *ptr_str)
{
    MW_ERROR_NO_T rc = MW_E_OK;

    if (FALSE == MW_CMD_IS_CONTORL_CH(ptr_str[0]))
    {
        rc = mw_cmd_handleString(ptr_str);
        if (MW_CMD_E_KEY_MISMATCH == rc)
        {
            MW_CMD_OUTPUT("***Unrecognized command***\n");
            _mw_cmd_help(NULL, 0);
        }
    }
}


