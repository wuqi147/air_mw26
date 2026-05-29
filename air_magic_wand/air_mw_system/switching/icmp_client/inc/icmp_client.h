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

/* FILE NAME:  icmp_client.h
 * PURPOSE:
 *  It provides ICMP Client module API and definitions.
 *
 * NOTES:
 */
#ifndef _ICMP_CLIENT_H
#define _ICMP_CLIENT_H
/* INCLUDE FILE DECLARATIONS
 */
#include <mw_error.h>
#include <osapi.h>
#include <osapi_message.h>
#include <osapi_mutex.h>
#include <osapi_thread.h>
#include "lwip/ip_addr.h"
#include "ping_queue.h"
#include "lwip/apps/ping.h"
#include "mw_log.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* NAMING CONSTANT PING */
#define ICMP_CLIENT_MODULE_NAME          "icping"
#define MW_PING_TASK_NAME                "ping_thread"
#define MW_PING_TASK_SIZE                (configMINIMAL_STACK_SIZE * 2)
#define MW_PING_TASK_PRI                 (2)

#define IPADDR_CLASS_E_HEADER            (240 & 0xFF)

/* MACRO FUNCTION DECLARATIONS
 */
#define PING_LOG_ERROR(fmt, ...)                    MW_LOG_ERROR(PING, fmt, ##__VA_ARGS__)
#define PING_LOG_WARN(fmt, ...)                     MW_LOG_WARN(PING, fmt, ##__VA_ARGS__)
#define PING_LOG_INFO(fmt, ...)                     MW_LOG_INFO(PING, fmt, ##__VA_ARGS__)
#define PING_LOG_DEBUG(fmt, ...)                    MW_LOG_DEBUG(PING, fmt, ##__VA_ARGS__)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

#if PING_USE_SOCKETS
/* FUNCTION NAME:   ping_thread_init
 * PURPOSE:
 *      Initialize ping thread
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *      MW_E_NOT_INITED
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      Ping using the socket ip
 */
MW_ERROR_NO_T
ping_thread_init(void);
#endif /* PING_USE_SOCKETS */

/* FUNCTION NAME:   ping_get_db_ping_status
 * PURPOSE:
 *      Get the status of the global varible db_ping_status
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      db_ping_status
 *
 * RETURN:
 *      &db_ping_status
 *
 * NOTES:
 *      None
 */
UI32_T *
ping_get_db_ping_status(void);

/* FUNCTION NAME:   ping_set_db_ping_status
 * PURPOSE:
 *      Set the status of the global varible _db_ping_status
 *
 * INPUT:
 *      status             -- status about ping
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_set_db_ping_status(
    UI32_T status);

/* FUNCTION NAME:   ping_create_ping_thread
 * PURPOSE:
 *      Create ping thread
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *      MW_E_NO_MEMORY
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_create_ping_thread(void);

/* FUNCTION NAME:   ping_thread_free_resource
 * PURPOSE:
 *      Free ping thread resource
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_thread_free_resource(void);

/* FUNCTION NAME:   ping_global_init
 * PURPOSE:
 *      Send icmp packet by socket
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
ping_global_init(void);
#endif /* End of _ICMP_CLIENT_H */
