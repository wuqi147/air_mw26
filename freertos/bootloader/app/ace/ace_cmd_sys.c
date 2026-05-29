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

/* FILE NAME:   ace_cmd_sys.c
 * PURPOSE:
 *      ACE System API implementation
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_cmd_sys.h"
#include "ace_cmd.h"
#include <stdio.h>
#include <string.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define ACE_SYS_TRAP_MODE                     (0x0200)
#define ACE_SYS_BOOT_MODE                     (0x0201)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static void *ACE_CmdHandler_Sys_TrapMode(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Sys_BootMode(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

const ACE_SUB_HANDLER_T ace_handlers_sys[] =
{
    {ACE_SYS_TRAP_MODE, ACE_CmdHandler_Sys_TrapMode},
    {ACE_SYS_BOOT_MODE, ACE_CmdHandler_Sys_BootMode},
};

/* LOCAL SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   ACE_QueryCmdHandler_Sys
 * PURPOSE:
 *      Handle query device info ACE ID list function.
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
ACE_SUB_HANDLER_T *ACE_QueryCmdHandler_Sys(uint32_t *sub_handler_size)
{
    *sub_handler_size = sizeof(ace_handlers_sys) / sizeof(ACE_SUB_HANDLER_T);
    return (ACE_SUB_HANDLER_T *)ace_handlers_sys;
}

/* FUNCTION NAME:   ACE_CmdHandler_Sys
 * PURPOSE:
 *      Dispatch ACE system function.
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
void *ACE_CmdHandler_Sys(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    void *ptr = NULL;
    uint32_t i = 0;
    if (ptr_ace_pkt->hdr.type == ACE_TYPE_COMMAND)
    {
        for (i = 0; i < sizeof(ace_handlers_sys) / sizeof(ACE_SUB_HANDLER_T); ++i)
        {
            if (ptr_ace_pkt->hdr.id == ace_handlers_sys[i].id)
            {
                ptr = ace_handlers_sys[i].handler(ptr_ace_pkt, ptr_pbuf);
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

/* FUNCTION NAME:   ACE_CmdHandler_Sys_TrapMode
 * PURPOSE:
 *      Handle trap mode packet.
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
static void *ACE_CmdHandler_Sys_TrapMode(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    char pattern[7] = {0};
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_SYS_TRAP_MODE, 7);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }
    ptr_pkt->payload[0] = rc;
    memcpy(ptr_pkt->payload + 1, ptr_ace_pkt->payload, 6);
    memcpy(pattern, ptr_ace_pkt->payload, 6);
    ACE_DBG_PRINT("[%s] status = 0x%x, Trap Pattern = %s\n", __FUNCTION__, ptr_pkt->payload[0], pattern);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Sys_BootMode
 * PURPOSE:
 *      Handle trap mode packet.
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
static void *ACE_CmdHandler_Sys_BootMode(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_SYS_BOOT_MODE, 2);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }
    ptr_pkt->payload[0] = rc;
    ptr_pkt->payload[1] = ptr_ace_pkt->payload[0];
    ACE_DBG_PRINT("[%s] status = 0x%x, Mode = %d\n", __FUNCTION__, ptr_pkt->payload[0], ptr_pkt->payload[1]);
    return ptr_pkt;
}

