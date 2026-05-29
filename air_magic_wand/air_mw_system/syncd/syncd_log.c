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

/* FILE NAME:  syncd_log.c
 * PURPOSE:
 *  Implement logging function for synchronization daemon.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "syncd.h"
#include "syncd_in.h"
#include "syncd_log.h"
#include "mw_types.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

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
/* FUNCTION NAME: syncd_log_dump_dbMsg
 * PURPOSE:
 *      Dump message data.
 *
 * INPUT:
 *      file        --  File name of debugging function, use __FILE__.
 *      line        --  Line number of debugging function, use __LINE__.
 *      func        --  Function name of debugging function, use __func__.
 *      ptr_msg     --  Pointer of dubugging message.
 *      str         --  Display string if need, can also be a null pointer, in which case it's not used.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_log_dump_dbMsg(
    const C8_T *ptr_file,
    const UI32_T line,
    const C8_T *ptr_func,
    const DB_MSG_T *ptr_msg,
    const C8_T *ptr_str)
{
    UI32_T i = 0;
    UI8_T count = 1;
    UI32_T msg_size = 0;
    UI32_T dump_size = 0;
    DB_PAYLOAD_T *ptr_payload = NULL;

    MW_CHECK_PTR(ptr_file);
    MW_CHECK_PTR(ptr_func);
    MW_CHECK_PTR(ptr_msg);

    if (!(ptr_msg ->method & M_B_RESPONSE))
    {
        count = ptr_msg ->type.count;
    }
    ptr_payload = (DB_PAYLOAD_T *)&(ptr_msg ->ptr_payload);
    msg_size = DB_MSG_HEADER_SIZE;
    for (i = 0; i < count; i++)
    {
        msg_size += DB_MSG_PAYLOAD_SIZE + (ptr_payload ->data_size);
        ptr_payload = (DB_PAYLOAD_T *)(((C8_T *)ptr_payload) + DB_MSG_PAYLOAD_SIZE + (ptr_payload ->data_size));
    }

    /* Only dump first 20 bytes to avoid log flooding */
    dump_size = msg_size;
    if (dump_size > 20)
    {
        dump_size = 20;
    }

    MW_LOG_DEBUG(SYNCD, "<%s:%d>(%s) count=%u, size=%u, ", ptr_file, line, ptr_func, count, msg_size);
    MW_LOG_DEBUG(SYNCD, "%s = ", (NULL != ptr_str) ? ptr_str : "ptr_msg");
    for (i = 0; i < dump_size; i++)
    {
        if (!(i%16))
        {
            MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "\n");
        }
        MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, " %02X", ((UI8_T *)ptr_msg)[i]);
    }
    MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "\n");
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_log_show_ui64
 * PURPOSE:
 *      Print unsigned long long data.
 *
 * INPUT:
 *      file        --  File name of debugging function, use __FILE__.
 *      line        --  Line number of debugging function, use __LINE__.
 *      func        --  Function name of debugging function, use __func__.
 *      val         --  Display value.
 *      str         --  Display string if need, can also be a null pointer, in which case it's not used.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_log_show_ui64(
    const C8_T *ptr_file,
    const UI32_T line,
    const C8_T *ptr_func,
    const UI64_T val,
    const C8_T *ptr_str)
{
    UI64_T u64_tmp = 0;
    UI32_T u32_H = 0, u32_M = 0, u32_L = 0;

    MW_CHECK_PTR(ptr_file);
    MW_CHECK_PTR(ptr_func);

    u32_H = (val) / 10000000000;
    u64_tmp = (val) % 10000000000;
    u32_M = u64_tmp / 100;
    u32_L = u64_tmp % 100;
    MW_LOG_DEBUG(SYNCD, "<%s:%d>(%s)%s=", ptr_file, line, ptr_func, (NULL != ptr_str) ? ptr_str : "UI64");
    if (0 != u32_H)
    {
        MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%u", u32_H);
        MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%08u", u32_M);
        MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%02u\n", u32_L);
    }
    else
    {
        if (0 != u32_M)
        {
            MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%u", u32_M);
            MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%02u\n", u32_L);
        }
        else
        {
            MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "%u\n", u32_L);
        }
    }
    return MW_E_OK;
}

/* FUNCTION NAME: syncd_log_show_macAddr
 * PURPOSE:
 *      Print MAC address.
 *
 * INPUT:
 *      file        --  File name of debugging function, use __FILE__.
 *      line        --  Line number of debugging function, use __LINE__.
 *      func        --  Function name of debugging function, use __func__.
 *      mac         --  Display MAC address.
 *      str         --  Display string if need, can also be a null pointer, in which case it's not used.
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
syncd_log_show_macAddr(
    const C8_T *ptr_file,
    const UI32_T line,
    const C8_T *ptr_func,
    const MW_MAC_T mac,
    const C8_T *ptr_str)
{
    UI32_T i = 0;

    MW_CHECK_PTR(ptr_file);
    MW_CHECK_PTR(ptr_func);

    MW_LOG_DEBUG(SYNCD, "<%s:%d>(%s)%s=", ptr_file, line, ptr_func, (NULL != ptr_str) ? ptr_str : "MAC addr");
    for (i = 0; i < 6; i++)
    {
        MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, " %02X", (UI8_T)mac[i]);
    }
    MW_LOG_RAW(SYNCD, MW_LOG_LEVEL_DEBUG, "\n");
    return MW_E_OK;
}

