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

/* FILE NAME:   eeeConfig.c
 * PURPOSE:
 *      CGI and SSI function of EEE configuration web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "osapi.h"

#include "mw_utils.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "mw_log.h"
#include "web.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROMGRAM BODIES
*/

/* EXPORTED SUBPROGRAM BODIES
 */
//==========================================================================================
//  CGI handler
//==========================================================================================
MW_ERROR_NO_T
cgi_set_handle_eeeConfig(
    int iIndex,
    int iNumParams,
    char *pcParam[],
    char *pcValue[])
{
    int i;
    int rc = MW_E_OK;
    signed char state = -1;
    int portBit = -1;

    /* expected CGI format:
     * eeeSet.cgi?state=1=&portBit=5 */

    /* get value from url params */
    for (i = 0; i < iNumParams; i++)
    {
        if (!strcmp(pcParam[i], "state")){
            state = atoi(pcValue[i]);
        }
        if (!strcmp(pcParam[i], "portBit")){
            portBit = atoi(pcValue[i]);
        }
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++ )
    {
        if (portBit & BIT(i))
        {
            /* parser params to db format */
            CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d>send to DB port =%d state=%d\n", __func__, __LINE__, i+1, state);

            rc = httpd_queue_setData(M_UPDATE, PORT_CFG_INFO, PORT_EEE_ENABLE, i+1, &state, sizeof(state));
        }
    }

    return rc;
}


//==========================================================================================
//  SSI handler
//==========================================================================================
char ssi_get_eeeconfig_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    char err = 0;
    int total_len = 0, i = 0;
    MW_ERROR_NO_T mw_rc = MW_E_OK;
    DB_MSG_T *ptr_msg;
    UI16_T data_size, len = 0;
    UI8_T *ptr_data;
    UI32_T portMap = 0;

    /*  expected javascript string
     *  <script>
     *  var eeeinfo_ds = {
     *      portNum:28,
     *      enable_bit:0x0,
     *  }
     *  </script>
     */

    CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> IN ===>\n", __func__, __LINE__);

    err = send_format_response(&len, pcb, apiflags,
            "<script>var eeeinfo_ds = { portNum:%d, enable_bit:0x",
            PLAT_MAX_PORT_NUM);
    if (ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    mw_rc = httpd_queue_getData(
            PORT_CFG_INFO, PORT_EEE_ENABLE, DB_ALL_ENTRIES,
            &ptr_msg, &data_size, (void**)&ptr_data);
    if (MW_E_OK != mw_rc)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> Error out\n", __func__, __LINE__);
        return ERR_INPROGRESS;
    }

    for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> port[%u]=%u\n", __func__, __LINE__,
                      i+1, ptr_data[i]);
        if (TRUE == ptr_data[i])
        {
            portMap |= BIT(i);
        }
    }

    /* Release allocated memory before return */
    MW_FREE(ptr_msg);

    err = send_format_response(&len, pcb, apiflags,
            "%X,};</script>",
            portMap);
    if (ERR_OK != err)
    {
        CGI_LOG_ERROR(PORT_SETTING, "<%s:%d> Error out\n", __func__, __LINE__);
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(PORT_SETTING, "<%s:%d> <=== OUT\n", __func__, __LINE__);

    return ERR_OK;
}

