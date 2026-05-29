/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

#ifdef AIR_SUPPORT_LP
/* FILE NAME:   cli_lp.c
 * PURPOSE:
 *      Implement the basic functions of cli loop prevention operation.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "lp.h"
#include "cli.h"
#include "cli_db.h"
#include "cli_internal.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
UI32_T
cli_cmd_lp_showCfg(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    I32_T show_mode)
{
    UI8_T               *ptr_config = NULL, *ptr_data = NULL;
    UI8_T               default_config = 0, config = 0;
    UI16_T              data_size = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = MW_E_OK;
    UI32_T              len = 0;

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }
    rc = dbapi_getFactoryDefault(LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &data_size, (void **)&ptr_config);
    if (MW_E_OK != rc)
    {
        return len;
    }
    osapi_memcpy(&default_config, ptr_config, sizeof(UI8_T));
    MW_FREE(ptr_config);

    if (CLI_SHOW_RUNNING_CONFIG == show_mode)
    {
        rc = cli_queue_getData(LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&config, ptr_data, sizeof(UI8_T));
        MW_FREE(ptr_msg);
    }
    else
    {
        rc = dbapi_getStartUp(LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &data_size, (void **)&ptr_config);
        if (MW_E_OK != rc)
        {
            return len;
        }
        osapi_memcpy(&config, ptr_config, sizeof(UI8_T));
        MW_FREE(ptr_config);
    }

    if (config != default_config)
    {
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "!\n");
        if (ENABLE == config)
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "loopback-detection enable\n");
        }
        else
        {
            len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "no loopback-detection enable\n");
        }
    }

    return len;
}

UI32_T
cli_cmd_lp_showState(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI8_T               *ptr_data = NULL, enable = 0;
    UI16_T              data_size = 0;
    UI32_T              port = 0;
    DB_MSG_T            *ptr_msg = NULL;
    MW_ERROR_NO_T       rc = MW_E_OK;
    C8_T                lp_state_str[8] = {0};
    UI32_T              len = 0;

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }
    rc = cli_queue_getData(LOOP_PREVEN_INFO, LOOP_PREVEN_ENABLE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return len;
    }
    osapi_memcpy(&enable, ptr_data, sizeof(UI8_T));
    MW_FREE(ptr_msg);

    rc = cli_queue_getData(PORT_OPER_INFO, PORT_LOOP_STATE, DB_ALL_ENTRIES, &ptr_msg, &data_size, (void **)&ptr_data);
    if (MW_E_OK != rc)
    {
        return len;
    }

    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "Loopback detection: %s\n", (0 == enable ? "Disabled" : "Enabled"));
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "\n");
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-9s %-18s %-18s\n", "Interface", "Loopback Detection", "Loopback Detection");
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-9s %-18s %-18s\n", "", "Admin State", "Operational Status");
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%-9s %-18s %-18s\n", "---------", "------------------", "------------------");

    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if (PLAT_CPU_PORT == port)
        {
            continue;
        }
        osapi_memset(lp_state_str, 0, sizeof(lp_state_str));
        if (LP_LOOP == ptr_data[(port - 1)])
        {
            osapi_snprintf(lp_state_str, sizeof(lp_state_str), "loop");
        }
        else if (LP_BLOCK == ptr_data[(port - 1)])
        {
            osapi_snprintf(lp_state_str, sizeof(lp_state_str), "blocked");
        }
        else
        {
            osapi_snprintf(lp_state_str, sizeof(lp_state_str), "normal");
        }
        len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "%9d %18s %18s\n", port, (0 == enable ? "Disabled" : "Enabled"), lp_state_str);
    }
    len += osapi_snprintf(ptr_out_buf + len, out_buf_len - len, "\n");

    MW_FREE(ptr_msg);
    return len;
}

UI32_T
cli_cmd_lp_setLoopPreventionState(
    C8_T *ptr_out_buf,
    UI32_T out_buf_len,
    BOOL_T no_flag,
    I32_T argc,
    C8_T **pptr_argv,
    I32_T token_idx)
{
    UI8_T   enable = TRUE;
    C8_T    *ptr_cgi_str = NULL;
    UI32_T  len = 0;

    if ((NULL == ptr_out_buf) || (0 == out_buf_len))
    {
        return len;
    }
    enable = ((TRUE == no_flag) ? FALSE : TRUE);
    if (MW_E_OK != osapi_calloc(CLI_CGI_STR_MAX_LEN, CLI_TASK_NAME, (void **)&ptr_cgi_str))
    {
        CLI_LOG_ERROR("Failed to allocate memory for Loopback Detection CGI string");
        return len;
    }
    osapi_snprintf(ptr_cgi_str, CLI_CGI_STR_MAX_LEN, "/loop_prevention_set.cgi?lpEn=%s", ((TRUE == enable) ? "1" : "0"));
    cli_cgi_proxy(ptr_cgi_str, CLI_CGI_STR_MAX_LEN);
    MW_FREE(ptr_cgi_str);
    return len;
}

#endif /* AIR_SUPPORT_LP */
