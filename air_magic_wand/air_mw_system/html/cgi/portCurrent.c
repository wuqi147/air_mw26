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

/* FILE NAME:   portCurrent.c
 * PURPOSE:
 *      SSI function of switch link status web page.
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "mw_error.h"
#include "db_api.h"
#include "httpd_queue.h"
#include "web.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_CURRENT_SPEED_10M    (0)
#define MW_CURRENT_SPEED_100M   (1)
#define MW_CURRENT_SPEED_1000M  (2)
#define MW_CURRENT_SPEED_2500M  (3)

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
MW_ERROR_NO_T
ssi_get_port_state_Handle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI16_T len = 0;

    err = send_format_response(&len, ptr_pcb, apiflags, "<script>\nvar portNum=%d;\n</script>", PLAT_MAX_PORT_NUM);
    if(MW_E_OK != err)
    {
        return err;
    }
    *ptr_length = len;

    return err;
}

MW_ERROR_NO_T
ssi_get_port_state_xmlHandle(
    I32_T *ptr_length,
    struct tcp_pcb *ptr_pcb,
    UI32_T apiflags)
{
    MW_ERROR_NO_T err = MW_E_OK;
    UI32_T total_len = 0, i = 0;
    UI16_T size = 0, len = 0;
    C8_T tmpbuf[(MAX_PORT_NUM * 2) + 1] = {0};

    /* DB variable */
    DB_MSG_T *ptr_msg = NULL;
    /* Port variable */
    UI8_T *ptr_data = NULL;

    /*
     * Status:
     * 0 - Link down
     * 1 - Link up speed 10/100/1000M
     * 2 - Link up speed 2.5G
     */
    /* Request DB for port speed */
    err = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_SPEED, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK != err)
    {
        return err;
    }
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        len += snprintf(tmpbuf + len, sizeof(tmpbuf) - len, "%d.", ((MW_CURRENT_SPEED_2500M == ptr_data[i]) ? 2 : 1));
    }
    MW_FREE(ptr_msg);

    /* Request DB for port state */
    err = httpd_queue_getData(PORT_OPER_INFO, PORT_OPER_STATUS, DB_ALL_ENTRIES, &ptr_msg, &size, (void**)&ptr_data);
    if(MW_E_OK != err)
    {
        return err;
    }
    for(i = 0; i < PLAT_MAX_PORT_NUM; i++)
    {
        if(0 == ptr_data[i])
        {
            tmpbuf[(i * 2)] = '0';
        }
    }
    MW_FREE(ptr_msg);

    err = send_format_response(&len, ptr_pcb, apiflags, tmpbuf);
    if(MW_E_OK != err)
    {
        return err;
    }
    total_len += len;

    *ptr_length = total_len;

    return err;
}

