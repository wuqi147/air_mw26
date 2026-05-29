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

/* FILE NAME:  syncd_log.h
 * PURPOSE:
 *      It provides logging definition for syncd moudle.
 *
 * NOTES:
 */

#ifndef _SYNCD_LOG_H_
#define _SYNCD_LOG_H_

/* INCLUDE FILE DECLARATIONS
 */
#include <string.h>
#include "syncd.h"
/* for DB_MSG_T */
#include "db_api.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
*/
#define SYNCD_LOG_ERROR(fmt, ...)    MW_LOG_ERROR(SYNCD, "%s[%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)
#define SYNCD_LOG_WARN(fmt, ...)     MW_LOG_WARN(SYNCD, "%s[%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)
#define SYNCD_LOG_INFO(fmt, ...)     MW_LOG_INFO(SYNCD, "%s[%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)
#define SYNCD_LOG_DEBUG(fmt, ...)    MW_LOG_DEBUG(SYNCD, "%s[%d] "fmt, __func__, __LINE__, ##__VA_ARGS__)

#define SYNCD_LOG_DUMP_UI64(val, str)    syncd_log_show_ui64(__FILENAME__, __LINE__, __func__, (val), str)
#define SYNCD_LOG_DUMP_MACADDR(mac, str) syncd_log_show_macAddr(__FILENAME__, __LINE__, __func__, (mac), (str))
#define SYNCD_LOG_DUMP_DBMSG(msg, str)   syncd_log_dump_dbMsg(__FILENAME__, __LINE__, __func__, (msg), (str))

/* DATA TYPE DECLARATIONS
*/

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    const C8_T *ptr_str);

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
    const C8_T *ptr_str);

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
    const C8_T *ptr_str);

#endif  /* _SYNCD_LOG_H_ */

