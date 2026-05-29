
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

/* FILE NAME:   ace_cmd_storage.c
 * PURPOSE:
 *      ACE LAB test implementation
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_cmd_storage.h"
#include "ace_main.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "util.h"
#include "spinorwrite.h"

/* NAMING CONSTANT DECLARATIONS
 */

#define ACE_STORAGE_WRITE_BYTE                    (0x0400)
#define ACE_STORAGE_READ_BYTE                     (0x0401)
#define ACE_STORAGE_WRITE_PAGE                    (0x0402)
#define ACE_STORAGE_READ_PAGE                     (0x0403)
#define ACE_STORAGE_ERASE_PARTITION               (0x0404)
#define ACE_STORAGE_START_DFU                     (0x0405)
#define ACE_STORAGE_WRITE_DMEM                    (0x0406)
#define ACE_STORAGE_LOAD_DMEM                     (0x0407)

#define PAGE_SIZE                                 (0x100)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
*/

static void *ACE_CmdHandler_Storage_WriteByte(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Storage_ReadByte(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Storage_ErasePartition(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);

/* GLOBAL VARIABLE DECLARATIONS
*/

/* STATIC VARIABLE DECLARATIONS
*/

const ACE_SUB_HANDLER_T ace_handlers_storage[] =
{
    {ACE_STORAGE_WRITE_BYTE, ACE_CmdHandler_Storage_WriteByte},
    {ACE_STORAGE_READ_BYTE, ACE_CmdHandler_Storage_ReadByte},
    {ACE_STORAGE_ERASE_PARTITION, ACE_CmdHandler_Storage_ErasePartition},\
};

/* LOCAL SUBPROGRAM BODIES
*/

/* FUNCTION NAME:   ACE_CmdHandler_Storage_WriteByte
 * PURPOSE:
 *      Handle write byte packet.
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *      ptr_pbuf            --  pbuf containing the packet
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
static void *ACE_CmdHandler_Storage_WriteByte(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    uint8_t storage_type = 0;
    uint16_t write_len = 0;
    uint32_t storage_address = 0;
    memcpy(&storage_type, ptr_ace_pkt->payload, 1);
    memcpy(&write_len, ptr_ace_pkt->payload + 1, 2);
    memcpy(&storage_address, ptr_ace_pkt->payload + 3, 4);

    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_STORAGE_WRITE_BYTE, 8);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_ace_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 1)
    {
        rc = ACE_E_BAD_PARAMETER;
        ptr_pkt->payload[0] = rc;
        return ptr_pkt;
    }

    rc = (int)spinor_write((unsigned int)ptr_ace_pkt->payload + 7, storage_address, write_len);
    ptr_pkt->payload[0] = rc;
    memcpy(ptr_pkt->payload + 1, &storage_type, 1);
    memcpy(ptr_pkt->payload + 2, &write_len, 2);
    memcpy(ptr_pkt->payload + 4, &storage_address, 4);
    ACE_DBG_PRINT("[%s] status = 0x%x, storage_address = 0x%x, write_len = 0x%x\n", __FUNCTION__, (int)ptr_pkt->payload[0], (int)storage_address, (int)write_len);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Storage_ReadByte
 * PURPOSE:
 *      Handle read page packet.
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *      ptr_pbuf            --  pbuf containing the packet
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
static void *ACE_CmdHandler_Storage_ReadByte(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    uint8_t storage_type = 0;
    uint16_t read_len = 0;
    uint32_t storage_address = 0;
    memcpy(&storage_type, ptr_ace_pkt->payload, 1);
    memcpy(&read_len, ptr_ace_pkt->payload + 1, 2);
    memcpy(&storage_address, ptr_ace_pkt->payload + 3, 4);

    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_STORAGE_READ_BYTE, 8 + read_len);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_ace_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 1)
    {
        rc = ACE_E_BAD_PARAMETER;
        ptr_pkt->payload[0] = rc;
        return ptr_pkt;
    }

    ptr_pkt->payload[0] = rc;
    memcpy(ptr_pkt->payload + 1, &storage_type, 1);
    memcpy(ptr_pkt->payload + 2, &read_len, 2);
    memcpy(ptr_pkt->payload + 4, &storage_address, 4);

    uint8_t *sector_addr0 = (uint8_t *)(storage_address | HIGH_BIT_UNC);
    memcpy(ptr_pkt->payload + 8, sector_addr0, read_len);
    ACE_DBG_PRINT("[%s] status = 0x%x, storage_address = 0x%x, read_len = 0x%x\n", __FUNCTION__, (int)ptr_pkt->payload[0], (int)storage_address, (int)read_len);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Storage_ErasePartition
 * PURPOSE:
 *      Handle erase partitoin packet.
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *      ptr_pbuf            --  pbuf containing the packet
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
static void *ACE_CmdHandler_Storage_ErasePartition(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    uint8_t storage_type = 0;
    uint32_t storage_address = 0;
    memcpy(&storage_type, ptr_ace_pkt->payload, 1);
    memcpy(&storage_address, ptr_ace_pkt->payload + 1, 4);

    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_STORAGE_ERASE_PARTITION, 1);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_pkt->hdr.length - sizeof(ptr_pkt->hdr.id) < 1)
    {
        rc = ACE_E_BAD_PARAMETER;
        ptr_pkt->payload[0] = rc;
        return ptr_pkt;
    }

    memset((unsigned char *)g_flash_buf, 0xff, SPI_NOR_SECTOR_SIZE);
    rc = spinor_write_sector((unsigned int)g_flash_buf, storage_address, SPI_NOR_SECTOR_SIZE);
    ptr_pkt->payload[0] = rc;
    ACE_DBG_PRINT("[%s] status = 0x%x, storage_address = 0x%x\n", __FUNCTION__, (int)ptr_pkt->payload[0], (int)storage_address);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_QueryCmdHandler_Storage
 * PURPOSE:
 *      Handle query storage ACE ID list function.
 *
 * INPUT:
 *      pointer to handler size
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
ACE_SUB_HANDLER_T *ACE_QueryCmdHandler_Storage(uint32_t *sub_handler_size)
{
    *sub_handler_size = sizeof(ace_handlers_storage) / sizeof(ACE_SUB_HANDLER_T);
    return (ACE_SUB_HANDLER_T *)ace_handlers_storage;
}

/* FUNCTION NAME:   ACE_CmdHandler_Storage
 * PURPOSE:
 *      Dispatch storage ACE packets
 *
 * INPUT:
 *      ptr_ace_pkt         --  ACE packet
 *      ptr_pbuf            --  pbuf containing the packet
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
void *ACE_CmdHandler_Storage(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    void *ptr = NULL;
    uint32_t i = 0;

    if (ptr_ace_pkt->hdr.type == ACE_TYPE_COMMAND)
    {
        for (i = 0; i < sizeof(ace_handlers_storage) / sizeof(ACE_SUB_HANDLER_T); ++i)
        {
            if (ptr_ace_pkt->hdr.id == ace_handlers_storage[i].id)
            {
                ptr = ace_handlers_storage[i].handler(ptr_ace_pkt, ptr_pbuf);
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
