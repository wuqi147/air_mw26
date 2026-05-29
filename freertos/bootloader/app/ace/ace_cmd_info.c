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

/* FILE NAME:   ace_cmd_info.c
 * PURPOSE:
 *      ACE info implementation
 *
 * NOTES:
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "ace_cmd_info.h"
#include "ace_main.h"

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "util.h"
#include "spinorwrite.h"

extern void sys_info_init(void);

/* NAMING CONSTANT DECLARATIONS
 */

#define ACE_WRITE_MAC_ADDRESS                     (0x0600)
#define ACE_READ_MAC_ADDRESS                      (0x0601)
#define ACE_WRITE_ACE_CONTROL_FLAGS               (0x0602)
#define ACE_READ_ACE_CONTROL_FLAGS                (0x0603)
#define ACE_QUERY_ACE_GROUP_LIST                  (0x0604)
#define ACE_QUERY_ACE_ID_LIST_BY_GROUP            (0x0605)
#define ACE_READ_PRODUCT_INFO                     (0x0606)
#define ACE_WRITE_PRODUCT_INFO                    (0x0607)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
struct ace_config_flags
{
    uint8_t mac_protect;
    uint8_t ace_enable;
};

/* LOCAL SUBPROGRAM DECLARATIONS
*/

#if defined(AIR_SUPPORT_ACE_MAC_BURN)
static void *ACE_CmdHandler_Info_WriteMacAddress(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_ReadMacAddress(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_WriteAceControlFlags(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_ReadAceControlFlags(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
#endif
static void *ACE_CmdHandler_Info_QueryAceGroupList(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_QueryAceIdListByGroup(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_WriteProductInfo(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
static void *ACE_CmdHandler_Info_ReadProductInfo(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);

/* GLOBAL VARIABLE DECLARATIONS
*/

extern PLAT_SYS_INFO_T g_sys_info;

/* STATIC VARIABLE DECLARATIONS
*/

const ACE_SUB_HANDLER_T ace_handlers_info[] =
{
#if defined(AIR_SUPPORT_ACE_MAC_BURN)
    {ACE_WRITE_MAC_ADDRESS, ACE_CmdHandler_Info_WriteMacAddress},
    {ACE_READ_MAC_ADDRESS, ACE_CmdHandler_Info_ReadMacAddress},
    {ACE_WRITE_ACE_CONTROL_FLAGS, ACE_CmdHandler_Info_WriteAceControlFlags},
    {ACE_READ_ACE_CONTROL_FLAGS, ACE_CmdHandler_Info_ReadAceControlFlags},
#endif
    {ACE_QUERY_ACE_GROUP_LIST, ACE_CmdHandler_Info_QueryAceGroupList},
    {ACE_QUERY_ACE_ID_LIST_BY_GROUP, ACE_CmdHandler_Info_QueryAceIdListByGroup},
    {ACE_WRITE_PRODUCT_INFO, ACE_CmdHandler_Info_WriteProductInfo},
    {ACE_READ_PRODUCT_INFO, ACE_CmdHandler_Info_ReadProductInfo},
};

/* LOCAL SUBPROGRAM BODIES
*/

#if defined(AIR_SUPPORT_ACE_MAC_BURN)
/* FUNCTION NAME:   ACE_CmdHandler_Info_WriteMacAddress
 * PURPOSE:
 *      Handle write MAC packet.
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
static void *ACE_CmdHandler_Info_WriteMacAddress(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    uint32_t i;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_WRITE_MAC_ADDRESS, 7);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 6)
    {
        rc = ACE_E_BAD_PARAMETER;
        *(ptr_pkt->payload) = rc;
        return ptr_pkt;
    }

    uint8_t *sector_addr0 = (uint8_t *)(ACE_FLAGS_MAC_WRITE_PROTECT | HIGH_BIT_UNC);
    if ((*sector_addr0) != ACE_FLAGS_MAC_WRITE_UNPROTECT)
    {
        ACE_DBG_PRINT("ACE_CmdHandler_Info_WriteMacAddress Info\n");
        ACE_DBG_PRINT("ACE MAC write protect = %x\n", (*sector_addr0));

        ptr_pkt->payload[0] = ACE_E_NOT_SUPPORT;
        return ptr_pkt;
    }

    rc = spinor_write((unsigned int)ptr_ace_pkt->payload, EEP_CONFIG_DATA_LOCATION, MAC_ADDRESS_LEN);
    sys_info_init();

    memcpy(ptr_pkt->payload + 1, g_sys_info.mac_addr, MAC_ADDRESS_LEN);
    ACE_DBG_PRINT("ACE_CmdHandler_Info_WriteMacAddress Info\n");
    ACE_DBG_PRINT("mac_addr = ");
    for (i = 0; i < MAC_ADDRESS_LEN; ++i)
    {
        ACE_DBG_PRINT("%x ", g_sys_info.mac_addr[i]);
        if (ptr_ace_pkt->payload[i] != g_sys_info.mac_addr[i])
        {
            rc = ACE_E_BAD_PARAMETER;
        }
    }
    ACE_DBG_PRINT("\n");
    *(ptr_pkt->payload) = rc;
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info_ReadMacAddress
 * PURPOSE:
 *      Handle read MAC packet.
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
static void *ACE_CmdHandler_Info_ReadMacAddress(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_READ_MAC_ADDRESS, 7);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    sys_info_init();

    memcpy(ptr_pkt->payload + 1, g_sys_info.mac_addr, MAC_ADDRESS_LEN);
    ACE_DBG_PRINT("ACE_CmdHandler_Info_ReadMacAddress Info\n");
    *(ptr_pkt->payload) = rc;
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info_WriteAceControlFlags
 * PURPOSE:
 *      Write ACE control flags command
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
static void *ACE_CmdHandler_Info_WriteAceControlFlags(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    int rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_WRITE_ACE_CONTROL_FLAGS, 3);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 2)
    {
        rc = ACE_E_BAD_PARAMETER;
        *(ptr_pkt->payload) = rc;
        return ptr_pkt;
    }

    struct ace_config_flags flags;
    flags.ace_enable = ptr_ace_pkt->payload[0];
    flags.mac_protect = ptr_ace_pkt->payload[1];

    rc = (int)spinor_write((unsigned int)&flags, ACE_FLAGS_ADDRESS_START, 2);
    ptr_pkt->payload[0] = rc;

    uint8_t *sector_addr0 = (uint8_t *)(ACE_FLAGS_ADDRESS_START | HIGH_BIT_UNC);
    ptr_pkt->payload[1] = sector_addr0[1];
    ptr_pkt->payload[2] = sector_addr0[2] ;
    ACE_DBG_PRINT("[%s] status = 0x%x, ACE enable = 0x%x, MAC write protect = 0x%x\n", __FUNCTION__, ptr_pkt->payload[0], ptr_pkt->payload[1], ptr_pkt->payload[2]);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info_ReadAceControlFlags
 * PURPOSE:
 *      Read ACE control flags packet.
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
static void *ACE_CmdHandler_Info_ReadAceControlFlags(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_READ_ACE_CONTROL_FLAGS, 3);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    uint8_t *sector_addr0 = (uint8_t *)(ACE_FLAGS_ADDRESS_ACE_ENABLE | HIGH_BIT_UNC);
    struct ace_config_flags *flags = (struct ace_config_flags *)sector_addr0;
    ptr_ace_pkt->payload[1] = flags->ace_enable;
    ptr_ace_pkt->payload[2] = flags->mac_protect;
    ptr_pkt->payload[0] = rc;
    ACE_DBG_PRINT("[%s] status = 0x%x, ACE enable = 0x%x, MAC write protect = 0x%x\n", __FUNCTION__, ptr_pkt->payload[0], ptr_pkt->payload[1], ptr_pkt->payload[2]);
    return ptr_pkt;
}
#endif

/* FUNCTION NAME:   ACE_CmdHandler_Info_QueryACE_GroupList
 * PURPOSE:
 *      Query ACE group list
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
static void *ACE_CmdHandler_Info_QueryAceGroupList(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    uint32_t idx = 1, i = 0, group_cnt = 0;

    group_cnt = getAceHandlerSize();
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_QUERY_ACE_GROUP_LIST, 1 + group_cnt * sizeof(ptr_ace_pkt->hdr.id));
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    for (i = 0; i < group_cnt; ++i)
    {
        memcpy(ptr_pkt->payload + idx, &(ace_handlers[i].id_start), sizeof(ptr_ace_pkt->hdr.id));
        idx += sizeof(ptr_ace_pkt->hdr.id);
    }

    ptr_pkt->payload[0] = rc;
    ACE_DBG_PRINT("[%s] status = 0x%x, ACE group count = 0x%x\n", __FUNCTION__, ptr_pkt->payload[0], (int)group_cnt);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info_QueryAceIdListByGroup
 * PURPOSE:
 *      Query ACE ID list by group
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
static void *ACE_CmdHandler_Info_QueryAceIdListByGroup(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    uint32_t idx = 1, i = 0;
    uint32_t id_count = 0;
    uint16_t gid;

    memcpy(&gid, ptr_ace_pkt->payload, 2);

    ACE_SUB_HANDLER_T *sub_handlers = NULL;
    for (i = 0; i < getAceHandlerSize(); ++i)
    {
        if (gid == ace_handlers[i].id_start)
        {
            sub_handlers = ace_handlers[i].handler_query_id_list(&id_count);
        }
    }
    ACE_DBG_PRINT("[%s] ACE group id = 0x%x, id count = %d\n", __FUNCTION__, (int)gid, (int)id_count);

    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_QUERY_ACE_ID_LIST_BY_GROUP, 3 + id_count * sizeof(sub_handlers[i].id));
    if (NULL == ptr_pkt)
    {
        return NULL;
    }

    if (ptr_pkt->hdr.length - sizeof(ptr_ace_pkt->hdr.id) < 2)
    {
        rc = ACE_E_BAD_PARAMETER;
        *(ptr_pkt->payload) = rc;
        return ptr_pkt;
    }

    memcpy(ptr_pkt->payload + idx, &gid, sizeof(gid));
    idx += sizeof(gid);

    for (i = 0; i < id_count; ++i)
    {
        memcpy(ptr_pkt->payload + idx, &(sub_handlers[i].id), sizeof(sub_handlers[i].id));
        idx += sizeof(ptr_ace_pkt->hdr.id);
    }

    ptr_pkt->payload[0] = rc;
    ACE_DBG_PRINT("[%s] status = 0x%x, ACE group id = 0x%x, id count = %x\n", __FUNCTION__, ptr_pkt->payload[0], (int)gid, (int)id_count);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_QueryCmdHandler_Info
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
ACE_SUB_HANDLER_T *ACE_QueryCmdHandler_Info(uint32_t *sub_handler_size)
{
    *sub_handler_size = sizeof(ace_handlers_info) / sizeof(ACE_SUB_HANDLER_T);
    return (ACE_SUB_HANDLER_T *)ace_handlers_info;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info
 * PURPOSE:
 *      Handle device info function.
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
void *ACE_CmdHandler_Info(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    void *ptr = NULL;
    uint32_t i = 0;

    if (ptr_ace_pkt->hdr.type == ACE_TYPE_COMMAND)
    {
        for (i = 0; i < sizeof(ace_handlers_info) / sizeof(ACE_SUB_HANDLER_T); ++i)
        {
            if (ptr_ace_pkt->hdr.id == ace_handlers_info[i].id)
            {
                ptr = ace_handlers_info[i].handler(ptr_ace_pkt, ptr_pbuf);
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

/* FUNCTION NAME:   ACE_CmdHandler_Info_ReadProductInfo
 * PURPOSE:
 *      Read ACE product info TLV format packet.
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
static void *ACE_CmdHandler_Info_ReadProductInfo(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    uint8_t read_tag = ptr_ace_pkt->payload[0];
    ACE_ERR_T rc = ACE_E_OK;

    uint8_t *sector_addr0 = (uint8_t *)(ACE_PRODUCT_INFO_START | HIGH_BIT_UNC);
    int pos = 0;
    uint8_t tag = 0xFF;
    uint8_t len = 0;
    while (pos < EEP_CONFIG_DATA_SIZE)
    {
        tag = sector_addr0[pos];
        if (tag == 0xFF)
        {
            len = 0;
            break;
        }
        pos += 1;
        len = sector_addr0[pos];
        pos += 1;
        if (tag == read_tag)
        {
            break;
        }
        pos += len;
    }
    ACE_PKT_T *ptr_pkt =NULL;
    ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_READ_PRODUCT_INFO, len + 3);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }
    if (tag == 0xFF)
    {
        rc = ACE_E_ENTRY_NOT_FOUND;
    }
    else
    {
        memcpy(ptr_pkt->payload + 3, sector_addr0 + pos, len);
    }
    ptr_pkt->payload[0] = rc;
    ptr_pkt->payload[1] = read_tag;
    ptr_pkt->payload[2] = len;
    ACE_DBG_PRINT("[%s] status = 0x%x, product tag = 0x%x, len = %d\n", __FUNCTION__, ptr_pkt->payload[0], ptr_pkt->payload[1], ptr_pkt->payload[2]);
    return ptr_pkt;
}

/* FUNCTION NAME:   ACE_CmdHandler_Info_WriteProductInfo
 * PURPOSE:
 *      Read ACE control flags packet.
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
static void *ACE_CmdHandler_Info_WriteProductInfo(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf)
{
    ACE_ERR_T rc = ACE_E_OK;
    ACE_PKT_T *ptr_pkt = (ACE_PKT_T *)ACE_ClaimPacket(ACE_TYPE_RESPONSE, ACE_WRITE_PRODUCT_INFO, ptr_ace_pkt->hdr.length + 1);
    if (NULL == ptr_pkt)
    {
        return NULL;
    }
    uint8_t write_tag = ptr_ace_pkt->payload[0];
    uint8_t write_len = ptr_ace_pkt->payload[1];

    uint8_t *sector_addr0 = (uint8_t *)(ACE_PRODUCT_INFO_START | HIGH_BIT_UNC);

    // int info_pos = 0;
    // int info_len = 0;
    // for (info_pos = 0; info_pos < EEP_CONFIG_DATA_SIZE; info_pos++)
    // {
    //     if (sector_addr0[info_pos] == 0xFF)
    //     {
    //         if (info_pos > 0)
    //         {
    //             info_len = info_pos;
    //         }
    //         break;
    //     }
    // }
    // uint8_t *info_ptr = NULL;
    // if (info_len > 0)
    // {
    //     info_ptr = pvPortMalloc(info_len);
    //     memcpy(info_ptr, sector_addr0, info_len);
    //     ACE_DBG_PRINT("[%s] info_pos = %d, info_len = %d\n", __FUNCTION__, info_pos, info_len);
    //     ACE_DBG_PRINT(
    //         "%02X %02X %02X %02X %02X %02X\n"
    //         "%02X %02X %02X %02X %02X %02X\n",
    //         sector_addr0[0],
    //         sector_addr0[1],
    //         sector_addr0[2],
    //         sector_addr0[3],
    //         sector_addr0[4],
    //         sector_addr0[5],
    //         sector_addr0[6],
    //         sector_addr0[7],
    //         sector_addr0[8],
    //         sector_addr0[9],
    //         sector_addr0[10],
    //         sector_addr0[11]);
    // }

    int pos = 0;
    uint8_t tag = 0xFF;
    uint8_t len = 0;
    while (pos < EEP_CONFIG_DATA_SIZE)
    {
        tag = sector_addr0[pos];
        if (tag == 0xFF)
        {
            break;
        }
        pos += 1;
        len = sector_addr0[pos];
        pos += 1;
        if (tag == write_tag)
        {
            pos -= 2;
            break;
        }
        pos += len;
    }
    ACE_DBG_PRINT("[%s] write pos = %d\n", __FUNCTION__, pos);
    if (pos + write_len > EEP_CONFIG_DATA_SIZE)
    {
        rc = ACE_E_TABLE_FULL;
    }
    else
    {
        rc = (int)spinor_write((unsigned int)ptr_ace_pkt->payload, ACE_PRODUCT_INFO_START + pos, write_len + 2);
        memcpy(ptr_pkt->payload + 1, sector_addr0 + pos, write_len + 2);
    }
    ptr_pkt->payload[0] = rc;
    ACE_DBG_PRINT("[%s] status = 0x%x, product tag = 0x%x, len = %d\n", __FUNCTION__, ptr_pkt->payload[0], ptr_pkt->payload[1], ptr_pkt->payload[2]);
    return ptr_pkt;
}
