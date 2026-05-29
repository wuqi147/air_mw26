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

#include <air_error.h>
#include <air_types.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>

#include <sdk_ref.h>
#include <parser/dsh_parser.h>

static C8_T _sdk_ref_line_buf[DSH_CMD_MAX_INPUT] = {0};

static void
_sdk_ref_dumpError(
    const UI32_T                line,
    const UI32_T                str_len,
    C8_T                        *ptr_str)
{
    UI32_T                      idx = 0;

    /* restore the string. */
    for (idx = 0; idx < str_len; idx++)
    {
        if ('\0' == ptr_str[idx])
        {
            ptr_str[idx] = ' ';
        }
    }
    osal_printf("***Error***, execute line %d '%s' fail.\n", line, ptr_str);
}

AIR_ERROR_NO_T
sdk_ref_loadScript(
    const C8_T      *ptr_script_name)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    void            *ptr_file = NULL;
    char            *ptr_line_buf = _sdk_ref_line_buf;
    char            input_key = 0;
    UI32_T          line = 0, line_idx = 0;

    ptr_file = sdk_ref_openFile((C8_T *)ptr_script_name, SDK_REF_FILE_TYPE_READ);
    if (NULL == ptr_file)
    {
        osal_printf("***Error***, open '%s' fail.\n", ptr_script_name);
        return (AIR_E_OTHERS);
    }

    while (0 < sdk_ref_readFile(ptr_file, &input_key, 1))
    {
        if (('\n' == input_key) || ('\r' == input_key))
        {
            line++;
            ptr_line_buf[line_idx] = '\0';

            /* handle string */
            rc = dsh_handleString(ptr_line_buf);
            if (AIR_E_OK != rc)
            {
                _sdk_ref_dumpError(line, line_idx, ptr_line_buf);
                break;
            }

            /* reset buffer */
            osal_memset(ptr_line_buf, 0, DSH_CMD_MAX_INPUT);
            line_idx = 0;
        }
        else
        {
            ptr_line_buf[line_idx] = input_key;
            line_idx += 1;
        }
    }

    /* no matter last line with or without CR, push it */
    if (AIR_E_OK == rc)
    {
        line++;
        rc = dsh_handleString(ptr_line_buf);
        if (AIR_E_OK != rc)
        {
            _sdk_ref_dumpError(line, line_idx, ptr_line_buf);
        }
    }

    sdk_ref_closeFile(ptr_file);

    return (rc);
}

