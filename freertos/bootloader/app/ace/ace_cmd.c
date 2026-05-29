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

/* FILE NAME:   ace_cmd.c
 * PURPOSE:
 *      Airoha Command Environment implementation
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_cmd.h"
#include "ace_log.h"
#include "ace_main.h"
#include "ace_cmd_sys.h"
#include "ace_cmd_info.h"
#include "ace_cmd_storage.h"
#include "ace_cmd_debug.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define ACE_ID_SYS_START          (0x0200)
#define ACE_ID_SYS_END            (0x02FF)
#define ACE_ID_STORAGE_START      (0x0400)
#define ACE_ID_STORAGE_END        (0x04FF)
#define ACE_ID_DEBUG_START        (0x0500)
#define ACE_ID_DEBUG_END          (0x05FF)
#define ACE_ID_INFO_START         (0x0600)
#define ACE_ID_INFO_END           (0x06FF)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

const ACE_HANDLER_T ace_handlers[] =
{
    {ACE_ID_SYS_START, ACE_ID_SYS_END, ACE_CmdHandler_Sys, ACE_QueryCmdHandler_Sys},
    {ACE_ID_INFO_START, ACE_ID_INFO_END, ACE_CmdHandler_Info, ACE_QueryCmdHandler_Info},
    {ACE_ID_STORAGE_START, ACE_ID_STORAGE_END, ACE_CmdHandler_Storage, ACE_QueryCmdHandler_Info},
#ifdef AIR_SUPPORT_ACE_ETHERNET_LOG
    {ACE_ID_DEBUG_START, ACE_ID_DEBUG_END, ACE_CmdHandler_Debug, ACE_QueryCmdHandler_Debug},
#endif
};

/* LOCAL SUBPROGRAM BODIES
*/

void *ACE_ClaimPacket(uint8_t ace_type, uint16_t ace_id, uint16_t data_len)
{
    int pkt_len = sizeof(ACE_COMMON_HDR_T) + data_len;
    if (0 == g_ace_log_enabled)
    {
        ACE_DBG_PRINT("claim packet length: %d\n", pkt_len);
    }
    void *ptr_pkt = pvPortMalloc(pkt_len);
    if (NULL != ptr_pkt)
    {
        memset(ptr_pkt, 0, pkt_len);
        ACE_PKT_T *ptr_ace_ack = (ACE_PKT_T *)(ptr_pkt);
        ptr_ace_ack->hdr.pktId.value = 0x05;
        ptr_ace_ack->hdr.type = ace_type;
        ptr_ace_ack->hdr.length = sizeof(ptr_ace_ack->hdr.id) + data_len;
        ptr_ace_ack->hdr.id = ace_id;
        if (0 == g_ace_log_enabled)
        {
            ACE_DBG_PRINT("Claim Packet Success\n");
        }
        return ptr_pkt;
    }
    if (0 == g_ace_log_enabled)
    {
        ACE_DBG_PRINT("Claim Packet Fail\n");
    }
    return NULL;
}

void ACE_FreePacket(void *data)
{
    vPortFree(data);
}

/* FUNCTION NAME:   getAceHandlerSize
 * PURPOSE:
 *      Get ACE handler table size
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      Size of handler table
 *
 * RETURN:
 *      void *
 *
 * NOTES:
 *      None
 */
uint32_t getAceHandlerSize()
{
    return sizeof(ace_handlers) / sizeof(ACE_HANDLER_T);
}

void *ACE_CmdHandler(ACE_PKT_T *ptr_ace_cmd, struct pbuf *ptr_pbuf)
{
    uint32_t i;
    void *ptr = NULL;

    if (NULL == ptr_ace_cmd)
    {
         return NULL;
    }

    ACE_DBG_PRINT("ACE ID: 0x%x\n", ptr_ace_cmd->hdr.id);

    for (i = 0; i < sizeof(ace_handlers) / sizeof(ACE_HANDLER_T); ++i)
    {
        if (ptr_ace_cmd->hdr.id >= ace_handlers[i].id_start && ptr_ace_cmd->hdr.id <= ace_handlers[i].id_end)
        {
            ptr = ace_handlers[i].handler(ptr_ace_cmd, ptr_pbuf);
            return ptr;
        }
    }

    /* NOT Found */
    ACE_PKT_ACK_T *ptr_ace_ack = (ACE_PKT_ACK_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ptr_ace_cmd->hdr.id, 1);
    ptr_ace_ack->status = ACE_E_NOT_SUPPORT;
    ptr = (void *)ptr_ace_ack;
    return ptr;
}