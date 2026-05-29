/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:  trunkInfo.c
 * PURPOSE:
 *  Implement global function for CGI/SSI request from multiple html files.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include "web.h"
#include "osapi_memory.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "mw_utils.h"
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
/*FUNCTION NAME: ssi_get_port_trunk_info_Handle
 * PURPOSE:
 *        insert port_list and trunk_info arrays into html
 * INPUT:
 *       ptr_pcb -- the pointer which points to the TCP protocol control block
 *       apiflags -- HTTP state
 * OUTPUT:
 *       ptr_length -- return the total length of bytes number writing to html
 * RETURN:
 *       ERR_OK
 *       ERR_MEM
 *       ERR_VAL
 * NOTES:
 *       None
*/
char
ssi_get_port_trunk_info_Handle(
    int *ptr_length,
    struct tcp_pcb *ptr_pcb,
    unsigned int apiflags)
{
    char err = ERR_OK;
    UI32_T len = 0, cur_len = 0, member = 0;
    UI16_T i = 0, num = 0, size = 0, total_len = 0;
    C8_T  *ptr_tmpCur = NULL;
    C8_T  *ptr_tmpBuf = NULL;
    osapi_calloc((MW_DEFAULT_MAX_PORT_NUM * 22), HTTPD_QUEUE_CLI, (void **)&ptr_tmpCur);
    osapi_calloc((MW_DEFAULT_MAX_PORT_NUM * 2), HTTPD_QUEUE_CLI, (void **)&ptr_tmpBuf);
    do
    {
        if ((ptr_tmpBuf == NULL) || (ptr_tmpCur == NULL))
        {
            err = ERR_MEM;
            CGI_LOG_ERROR(SYSTEM, "trunk info: alloc memory fail");
            break;
        }
        /* DB variable */
        DB_MSG_T *ptr_msg = NULL;
        UI8_T *ptr_data = NULL;

        /* Port trunk status */
        err = httpd_queue_getData(PORT_CFG_INFO, PORT_TRUNK_ID, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
        if(MW_E_OK == err)
        {
            len = 0;
            cur_len = 0;
            for(num = 0; num < PLAT_MAX_PORT_NUM; num++)
            {
                len += snprintf(ptr_tmpBuf + len, (MW_DEFAULT_MAX_PORT_NUM * 2) - len, "%d,", ptr_data[num]);
                if(0 == ptr_data[num])
                {
                    cur_len += snprintf(ptr_tmpCur + cur_len, (MW_DEFAULT_MAX_PORT_NUM * 22) - cur_len, "['%d','%d'],",
                                        (num + 1), (0x01 << num));
                }
            }
            for(num = 1; num <= MAX_TRUNK_NUM; num++)
            {
                member = 0;
                for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if(num == ptr_data[i])
                    {
                        member |= (0x01 << i);
                    }
                }
                if(0 != member)
                {
                    cur_len += snprintf(ptr_tmpCur + cur_len, (MW_DEFAULT_MAX_PORT_NUM * 22) - cur_len,
                                        "['Trunk%d','%d'],", num, member);
                }
            }
            ptr_tmpBuf[len - 1] = 0;
            ptr_tmpCur[cur_len - 1] = 0;
            MW_FREE(ptr_msg);
            err = send_format_response(&total_len, ptr_pcb, apiflags,
                                       "<script>var _trunk_info=[%s];\nvar _port_list=new Map([%s]);\n</script>",
                                       ptr_tmpBuf, ptr_tmpCur);
            if (MW_E_OK != err)
            {
                CGI_LOG_ERROR(SYSTEM, "trunk info: send response fail");
                break;
            }
        }
        *ptr_length = total_len;
    } while (0);
    MW_FREE(ptr_tmpBuf);
    MW_FREE(ptr_tmpCur);
    return err;
}
