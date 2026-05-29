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
/* FILE NAME:   jumboFrame.c
 * PURPOSE:
 *      CGI and SSI function of jumboFrame web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "osapi_string.h"
#include "osapi_memory.h"

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
typedef enum
{
    JUMBO_FRAME_LEN_NONE = 0,
    JUMBO_FRAME_LEN_2K,
    JUMBO_FRAME_LEN_3K,
    JUMBO_FRAME_LEN_4K,
    JUMBO_FRAME_LEN_5K,
    JUMBO_FRAME_LEN_6K,
    JUMBO_FRAME_LEN_7K,
    JUMBO_FRAME_LEN_8K,
    JUMBO_FRAME_LEN_9K,
    JUMBO_FRAME_LEN_12K,
    JUMBO_FRAME_LEN_15K,

    JUMBO_FRAME_LEN_LAST,
}JUMBO_FRAME_LEN_T;

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
MW_ERROR_NO_T cgi_set_handle_jumboFrame(int iIndex, int iNumParams, char *pcParam[], char *pcValue[])
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI16_T i;
    UI32_T jumboframe_en = 0, mtu_size = 0;
    BOOL_T get_val = FALSE;

    CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> ============== enter ================\n", __func__, __LINE__);

    /* Parser name=value from cgi parameter */
    for(i = 0; i < iNumParams; i++) {
        if(!strcmp(pcParam[i], "jumboframe_en")) {
            jumboframe_en = atoi(pcValue[i]);
            get_val = TRUE;
        }
        if(!strcmp(pcParam[i], "mtu_size")) {
            mtu_size = atoi(pcValue[i]);
            get_val = TRUE;
        }
    }
    if(FALSE == get_val) return MW_E_OK;

    CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> jumbo state = %d, mtu =%d\n", __func__, __LINE__, jumboframe_en, mtu_size);

    /* Check value is valid or not */
    if((TRUE == jumboframe_en) && (0 == mtu_size))
    {
        CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> Invalid jumbo state = %d with mtu =%d\n", __func__, __LINE__, jumboframe_en, mtu_size);
        return MW_E_BAD_PARAMETER;
    }

    /* Update db */
    rc = httpd_queue_setData(M_UPDATE, JUMBO_FRAME_INFO, JUMBO_FRAME_CFG, DB_ALL_ENTRIES, &mtu_size, sizeof(mtu_size));
    if (MW_E_OK != rc)
    {
        CGI_LOG_ERROR(JUMBO_FRAME, "<%s:%d> Update DB Jumbo info failed(%d)\n", __func__, __LINE__, rc);
    }

    CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> ============== leave ================\n", __func__, __LINE__);

    return rc;
}

char ssi_get_jumboframe_info_Handle(int *length, struct tcp_pcb *pcb, unsigned int apiflags)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    C8_T err = 0;
    UI32_T total_len = 0;
    DB_MSG_T *ptr_msg = NULL;
    UI16_T size = 0, len = 0;
    UI32_T *ptr_jumbo = NULL;
    UI8_T  maxJumbo = JUMBO_FRAME_LEN_15K;

    CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> ==============: enter ================\n", __func__, __LINE__);

    err = send_format_response(&len, pcb, apiflags, "<script>");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

#ifdef AIR_LITE_MW
    maxJumbo = JUMBO_FRAME_LEN_9K;
#endif

    /* Request DB for jumbo frame info */
    rc = httpd_queue_getData(JUMBO_FRAME_INFO, JUMBO_FRAME_CFG, DB_ALL_ENTRIES, &ptr_msg, &size, (void **)&ptr_jumbo);
    if(MW_E_OK == rc)
    {
        CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> get jumbo frame data success, ptr_msg =%p, size =%d\n", __func__, __LINE__, ptr_msg, size);
    }
    else
    {
        CGI_LOG_ERROR(JUMBO_FRAME, "<%s:%d> get jumbo frame data failed!\n", __func__, __LINE__);
        return ERR_VAL;
    }

    err = send_format_response(&len, pcb, apiflags,
                                "var jumboinfo_ds = { enable:%d, mtusize:%d, maxJumbo:%d};",
                                ((FALSE == (*ptr_jumbo))? FALSE : TRUE),
                                (*ptr_jumbo), maxJumbo);
    MW_FREE(ptr_msg);
    if (ERR_OK != err)
    {
        return err;
    }
    total_len += len;

    err = send_format_response(&len, pcb, apiflags, "</script>");
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *length = total_len;

    CGI_LOG_DEBUG(JUMBO_FRAME, "<%s:%d> ==============: leave ================\n", __func__, __LINE__);

    return err;
}

