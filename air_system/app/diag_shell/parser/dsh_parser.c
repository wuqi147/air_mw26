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

#include <parser/dsh_parser.h>

#include <air_error.h>
#include <air_init.h>
#include <air_types.h>
#include <air_ver.h>
#include <cmd/acl_cmd.h>
#include <cmd/app_cmd.h>
#include <cmd/dbg_cmd.h>
#include <cmd/diag_cmd.h>
#include <cmd/dos_cmd.h>
#include <cmd/dsh_cmd.h>
#include <cmd/ifmon_cmd.h>
#include <cmd/init_cmd.h>
#include <cmd/ipmc_cmd.h>
#include <cmd/l2_cmd.h>
#include <cmd/lag_cmd.h>
#include <cmd/mib_cmd.h>
#include <cmd/mir_cmd.h>
#include <cmd/perif_cmd.h>
#include <cmd/phy_cmd.h>
#include <cmd/poe_cmd.h>
#include <cmd/port_cmd.h>
#include <cmd/qos_cmd.h>
#include <cmd/sec_cmd.h>
#include <cmd/sflow_cmd.h>
#include <cmd/sif_cmd.h>
#include <cmd/stag_cmd.h>
#include <cmd/stp_cmd.h>
#include <cmd/svlan_cmd.h>
#include <cmd/swc_cmd.h>
#include <cmd/ver_cmd.h>
#include <cmd/vlan_cmd.h>
#include <osal/osal_lib.h>
#include <parser/dsh_util.h>

#ifdef AIR_EN_CHIP_DIAG
#include <common/cdiag_cmd.h>
#endif
#ifdef AIR_EN_API_DIAG
#include <adiag_cmd.h>
#endif

#include <string.h>

/* -------------------------------------------------------------- Macros */
#define DSH_IS_CONTORL_CH(__ch__) ((__ch__) <= 0x1F) ? TRUE : ((__ch__) == 0x7F) ? TRUE : FALSE
#define DSH_PARSER_PREFIX_LEN     (4)

void
_dsh_print_usage(
    const C8_T  *ptr_usage,
    const BOOL_T indent,
    const UI32_T index_num)
{
    UI32_T usage_len = 0, start_len = 0, tmp_len = 0, max_print_len = 0, i = 0;
    char   str_output[DSH_CMD_MAX_OUTPUT] = {0};
    BOOL_T is_first_line = TRUE, is_first_char = TRUE;

    if (NULL == ptr_usage)
    {
        return;
    }
    usage_len = osal_strlen(ptr_usage);
    /* the maximum length of the output string is defined in OSAL_PRN_BUF_SZ*/
    max_print_len = DSH_CMD_MAX_OUTPUT - DSH_PARSER_PREFIX_LEN - 1;

    while (i < usage_len)
    {
        /* detect the new line, end line or exceed the max output length */
        if ((ptr_usage[i] == '\n') || (ptr_usage[i] == '\0') || ((i - start_len) >= max_print_len))
        {
            /* edit the output string */
            tmp_len = ((i - start_len) >= max_print_len) ? max_print_len : (i - start_len);
            osal_memset(str_output, 0, sizeof(str_output));
            osal_strncpy(str_output, &ptr_usage[start_len], tmp_len);
            str_output[tmp_len] = '\0';

            /* print the output string */
            if ((TRUE == indent) && (TRUE == is_first_char))
            {
                is_first_char = FALSE;
                max_print_len = DSH_CMD_MAX_OUTPUT - 1;
                if (TRUE == is_first_line)
                {
                    is_first_line = FALSE;
                    osal_printf("%2u. %s", index_num, str_output);
                }
                else
                {
                    osal_printf("%4s%s", "", str_output);
                }
            }
            else
            {
                osal_printf("%s", str_output);
            }
            /* print the end of the line */
            if ((ptr_usage[i] == '\n') || (ptr_usage[i] == '\0'))
            {
                is_first_char = TRUE;
                max_print_len = DSH_CMD_MAX_OUTPUT - DSH_PARSER_PREFIX_LEN - 1;
                start_len++;
                osal_printf("\n");
            }
            /* increase the index */
            start_len += tmp_len;
        }
        i++;
    }
}

/* -------------------------------------------------------------- Top */
#ifdef AIR_EN_DUMB_FIRMWARE_SUPPORT
const DSH_VEC_TOP_T _dsh_vec[] = {
    /* key    description            callback function     show usage function */
    { "init", "Chip initialization",  init_cmd_dispatcher,  init_cmd_usager},
    { "port",                "Port",  port_cmd_dispatcher,  port_cmd_usager},
    {  "mib",                 "Mib",   mib_cmd_dispatcher,   mib_cmd_usager},
    { "vlan",                "Vlan",  vlan_cmd_dispatcher,  vlan_cmd_usager},
    {  "qos",                 "QoS",   qos_cmd_dispatcher,   qos_cmd_usager},
    {  "swc",      "Switch setting",   swc_cmd_dispatcher,   swc_cmd_usager},
    {"ifmon",               "ifmon", ifmon_cmd_dispatcher, ifmon_cmd_usager},
    { "diag",          "diag debug",  diag_cmd_dispatcher,  diag_cmd_usager},
    {  "phy",                 "PHY",   phy_cmd_dispatcher,   phy_cmd_usager},
    {  "sif",                 "I2C",   sif_cmd_dispatcher,   sif_cmd_usager},
    {  "stp",                 "STP",   stp_cmd_dispatcher,   stp_cmd_usager},
    {  "lag",                 "lag",   lag_cmd_dispatcher,   lag_cmd_usager},
    { "ipmc",                "IPMC",  ipmc_cmd_dispatcher,  ipmc_cmd_usager},
    {  "ver",             "Version",   ver_cmd_dispatcher,   ver_cmd_usager},
    {  "sec",            "Security",   sec_cmd_dispatcher,   sec_cmd_usager},
    {  "dos",                 "DoS",   dos_cmd_dispatcher,   dos_cmd_usager},
    {  "acl",                 "Acl",   acl_cmd_dispatcher,   acl_cmd_usager},
#ifdef AIR_EN_POE
    {  "poe",                 "poe",   poe_cmd_dispatcher,   poe_cmd_usager},
#endif
    {  "app",                 "app",   app_cmd_dispatcher,   app_cmd_usager},
};
#else
const DSH_VEC_TOP_T _dsh_vec[] = {
    /* key    description            callback function     show usage function */
    { "init", "chip initialization",  init_cmd_dispatcher,  init_cmd_usager},
    {  "dbg",               "debug",   dbg_cmd_dispatcher,   dbg_cmd_usager},
    { "port",                "port",  port_cmd_dispatcher,  port_cmd_usager},
    {   "l2",                  "l2",    l2_cmd_dispatcher,    l2_cmd_usager},
    {  "mib",                 "mib",   mib_cmd_dispatcher,   mib_cmd_usager},
    { "vlan",                "vlan",  vlan_cmd_dispatcher,  vlan_cmd_usager},
    {  "mir",              "mirror",   mir_cmd_dispatcher,   mir_cmd_usager},
    {  "qos",                 "qos",   qos_cmd_dispatcher,   qos_cmd_usager},
    {  "swc",      "switch setting",   swc_cmd_dispatcher,   swc_cmd_usager},
    {"ifmon",               "ifmon", ifmon_cmd_dispatcher, ifmon_cmd_usager},
    { "diag",          "diag debug",  diag_cmd_dispatcher,  diag_cmd_usager},
    {  "phy",                 "phy",   phy_cmd_dispatcher,   phy_cmd_usager},
    {  "sif",                 "i2c",   sif_cmd_dispatcher,   sif_cmd_usager},
    {"perif",           "pripheral", perif_cmd_dispatcher, perif_cmd_usager},
    {  "stp",                 "stp",   stp_cmd_dispatcher,   stp_cmd_usager},
    {  "lag",                 "lag",   lag_cmd_dispatcher,   lag_cmd_usager},
    { "ipmc",        "ip multicast",  ipmc_cmd_dispatcher,  ipmc_cmd_usager},
    {  "ver",             "version",   ver_cmd_dispatcher,   ver_cmd_usager},
    {  "sec",            "security",   sec_cmd_dispatcher,   sec_cmd_usager},
    {"sflow",               "sflow", sflow_cmd_dispatcher, sflow_cmd_usager},
    {"svlan",        "service vlan", svlan_cmd_dispatcher, svlan_cmd_usager},
    { "stag",             "cpu tag",  stag_cmd_dispatcher,  stag_cmd_usager},
    {  "dos",                 "dos",   dos_cmd_dispatcher,   dos_cmd_usager},
    {  "acl",                 "acl",   acl_cmd_dispatcher,   acl_cmd_usager},
#ifdef AIR_EN_CHIP_DIAG
    {"cdiag",           "chip diag", cdiag_cmd_dispatcher, cdiag_cmd_usager},
#endif
#ifdef AIR_EN_API_DIAG
    {"adiag",            "api diag", adiag_cmd_dispatcher, adiag_cmd_usager},
#endif
#ifdef AIR_EN_POE
    {  "poe",                 "poe",   poe_cmd_dispatcher,   poe_cmd_usager},
#endif
    {  "app",                 "app",   app_cmd_dispatcher,   app_cmd_usager},
    { "help",                "help",         dsh_cmd_help,   dsh_cmd_usager},
};
#endif
const UI32_T _dsh_vec_num = sizeof(_dsh_vec) / sizeof(DSH_VEC_TOP_T);

/* -------------------------------------------------------------- exported to modules */
AIR_ERROR_NO_T
dsh_dispatcher(
    const C8_T     *tokens[],
    UI32_T          token_idx,
    const DSH_VEC_T vec[],
    const UI32_T    vec_num)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         idx = 0, jdx = 0, len = 0;
    UI32_T         tok_num = 0, tok_idx = 0;
    I32_T          tok_len = 0; /* reset per vector */
    C8_T          *ptr_key = NULL;

    /* for each vector */
    for (idx = 0; idx < vec_num; idx++)
    {
        rc = AIR_E_OK;
        tok_idx = token_idx;
        tok_num = vec[idx].token_num; /* e.g. 2        */
        ptr_key = vec[idx].ptr_key;   /* e.g. "set rx" */
        tok_len = 0;

        /* for each token, e.g. "set" */
        for (jdx = 0; jdx < tok_num; jdx++)
        {
            if (NULL == tokens[tok_idx])
            {
                rc = DSH_E_KEY_MISMATCH;
                break;
            }

            /* loop compare the vec-key and token */
            len = osal_strlen(tokens[tok_idx]);

            if (0 != osal_strncmp(ptr_key, tokens[tok_idx], len))
            {
                rc = DSH_E_KEY_MISMATCH;
                break;
            }
            ptr_key += len + 1;
            tok_idx += 1;
            tok_len += len + 1;
        }

        /* if token len is not match, e.g. "set" + "rx" + 2 - 1 != "set rx" */
        if (AIR_E_OK == rc)
        {
            if ((tok_len - 1) != osal_strlen(vec[idx].ptr_key))
            {
                rc = DSH_E_KEY_MISMATCH;
            }
        }

        /* if fully-match */
        if (AIR_E_OK == rc)
        {
            rc = vec[idx].callback(tokens, tok_idx);
            if ((AIR_E_BAD_PARAMETER == rc) || (DSH_E_SYNTAX_ERR == rc))
            {
                osal_printf("syntax error:\n");
                _dsh_print_usage(vec[idx].ptr_usage, FALSE, 0);
                osal_printf("\n");
            }
            return (rc);
        }
    }

    /* miss all of the vectors */
    return (DSH_E_NOT_FOUND);
}

AIR_ERROR_NO_T
dsh_usager(
    const DSH_VEC_T vec[],
    const UI32_T    vec_num)
{
    UI32_T idx = 0, num = 1;

    osal_printf("\n");
    if (vec_num > 0)
    {
        osal_printf("Usages:\n");
    }
    for (idx = 0; idx < vec_num; idx++)
    {
        if (NULL != vec[idx].ptr_usage)
        {
            _dsh_print_usage(vec[idx].ptr_usage, TRUE, num++);
            osal_printf("\n");
        }
    }

    return (AIR_E_OK);
}

/* -------------------------------------------------------------- exported API */
AIR_ERROR_NO_T
dsh_handleString(
    C8_T *ptr_str)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         idx = 0;
    C8_T          *tokens[DSH_CMD_MAX_TOKENS] = {0};

    if (FALSE == DSH_IS_CONTORL_CH(ptr_str[0]))
    {
        osal_memset(tokens, 0, sizeof(C8_T *) * DSH_CMD_MAX_TOKENS);
        rc = dsh_splitString(tokens, ptr_str);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, Input too many tokens.\n");
            return rc;
        }

        /* check empty string */
        if (NULL == tokens[0])
        {
            /* show air command list when command is : air */
            return (DSH_E_KEY_MISMATCH);
        }

        /* check the key */
        for (idx = 0; idx < _dsh_vec_num; idx++)
        {
            if (AIR_E_OK == dsh_checkString(tokens[0], _dsh_vec[idx].ptr_key))
            {
                /* callback with token_idx 1 */
                rc = _dsh_vec[idx].callback((const C8_T **)tokens, 1);
                if (DSH_E_NOT_FOUND == rc)
                {
                    _dsh_vec[idx].usage();
                }
                return (rc);
            }
        }

        /* miss all of the keys */
        return (DSH_E_KEY_MISMATCH);
    }

    return (AIR_E_OK);
}

void
dsh_parseString(
    C8_T *ptr_str)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         idx = 0, num = 1;

    if (FALSE == DSH_IS_CONTORL_CH(ptr_str[0]))
    {
        rc = dsh_handleString(ptr_str);
        if (DSH_E_KEY_MISMATCH == rc)
        {
            osal_printf("***Unrecognized command***\n");
            osal_printf("air support command : \n");
            for (idx = 0; idx < _dsh_vec_num; idx++)
            {
                osal_printf("%2d. ", num++);
                osal_printf("%s \n", _dsh_vec[idx].ptr_key);
                osal_printf("\n");
            }
        }
    }
}

void
dsh_init(
    void)
{
    osal_printf("SDK: %s\n", AIR_VER_SDK);
    osal_printf("Build: "__DATE__
                ", "__TIME__
                "\n");
    osal_printf("Airoha Technology Corp. (C) 2021\n");
}
