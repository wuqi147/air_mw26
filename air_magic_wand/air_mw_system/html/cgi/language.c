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

/* FILE NAME:  language.c
 * PURPOSE:
 *  Implement CGI/SSI handler function for language choose page in index.html.
 *
 * NOTES:
 *
 */
#include "web.h"
#include "osapi_memory.h"
#include "db_api.h"
#include "httpd_queue.h"

/* index.html */

const char language_str[LANG_LAST][LANGUAGE_LEN] = {"chinese", "english"};

language_info_t language_info =
{
    LANG_ENGLISH,
};


MW_ERROR_NO_T cgi_set_handle_language(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    int i, j;

    /* get value from url params */
    for(i = 0; i < iNumParams; i++)
    {
        if(!strcmp(pcParam[i], "lang_sel"))
        {
            for(j = LANG_CHINESE; j < LANG_LAST; j++)
            {
                if(!strcmp(language_str[j], pcValue[i]))
                {
                    language_info.lang_idx = j;
                }
            }
        }
    }
    return MW_E_OK;
}


char ssi_get_language_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    UI16_T len = 0, total_len = 0;
    char err = 0;

    err = send_format_response(&len, pcb, apiflags, "<script>var language ='%s';\n", language_str[language_info.lang_idx]);
    if(err != ERR_OK)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "</script>");
    if(MW_E_OK != err)
    {
        return err;
    }
    *length = (int)total_len;

    return ERR_OK;
}