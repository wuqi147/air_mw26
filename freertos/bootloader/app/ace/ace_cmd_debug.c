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

/* FILE NAME:   ace_cmd_debug.c
 * PURPOSE:
 *      ACE debug implementation
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_cmd_debug.h"
#include "ace_log.h"
#include "ace_main.h"

#include <stdio.h>
#include "FreeRTOS.h"
#include "util.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
*/

static void *ACE_CmdHandler_LabTest_Enable_Ethernet_Log(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);

/* GLOBAL VARIABLE DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

const ACE_SUB_HANDLER_T ace_handlers_debug[] =
{
#if defined(AIR_SUPPORT_ACE_ETHERNET_LOG)
    {ACE_DEBUG_ENABLE_ETHERNET_LOG, ACE_CmdHandler_LabTest_Enable_Ethernet_Log},
#endif
};

/* LOCAL SUBPROGRAM BODIES
*/

/* FUNCTION NAME:   ACE_CmdHandler_LabTest_Enable_Ethernet_Log
 * PURPOSE:
 *      Handle Enable ethetnet log packet.
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void *
 *
 * NOTES:
 *      None
 */
static void *ACE_CmdHandler_LabTest_Enable_Ethernet_Log(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_ACK_T *ptr_ace_ack = (ACE_PKT_ACK_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_DEBUG_ENABLE_ETHERNET_LOG, 1);
    if (NULL == ptr_ace_ack)
    {
        return NULL;
    }

    if (ptr_ace_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 1)
    {
        rc = ACE_E_BAD_PARAMETER;
        ptr_ace_ack->status = rc;
        return ptr_ace_ack;
    }

    uint8_t enable = ptr_ace_pkt->payload[0];
    g_ace_log_enabled = enable;

    ACE_DBG_PRINT("Debug_Enable_Ethernet_Log Info: enable = %x\n", enable);

    ptr_ace_ack->status = rc;
    return ptr_ace_ack;
}

/* FUNCTION NAME:   ACE_QueryCmdHandler_Debug
 * PURPOSE:
 *      Handle query debug ACE ID list function.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      ACE ID list
 *
 * RETURN:
 *      ACE_SUB_HANDLER_T *
 *
 * NOTES:
 *      None
 */
ACE_SUB_HANDLER_T *ACE_QueryCmdHandler_Debug(uint32_t *sub_handler_size)
{
    *sub_handler_size = sizeof(ace_handlers_debug) / sizeof(ACE_SUB_HANDLER_T);
    return (ACE_SUB_HANDLER_T *)ace_handlers_debug;
}

/* FUNCTION NAME:   ACE_CmdHandler_Debug
 * PURPOSE:
 *      Dispatch ACE debug function.
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      void *
 *
 * NOTES:
 *      None
 */
void *ACE_CmdHandler_Debug(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    void *ptr = NULL;
    uint32_t i = 0;

    if (ptr_ace_pkt->hdr.type == ACE_TYPE_COMMAND)
    {
        for (i = 0; i < sizeof(ace_handlers_debug) / sizeof(ACE_SUB_HANDLER_T); ++i)
        {
            if (ptr_ace_pkt->hdr.id == ace_handlers_debug[i].id)
            {
                ptr = ace_handlers_debug[i].handler(ptr_ace_pkt, ptr_pbuf);
                return ptr;
            }
        }
        /* NOT Found */
        ACE_PKT_ACK_T *ptr_ace_ack = (ACE_PKT_ACK_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ptr_ace_pkt->hdr.id, 1);
        ptr_ace_ack->status = ACE_E_NOT_SUPPORT;
        ptr = (void *)ptr_ace_ack;
    }

    return ptr;
}