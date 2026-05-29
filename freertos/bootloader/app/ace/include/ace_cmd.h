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

/* FILE NAME:  ace_cmd.h
 * PURPOSE:
 *      It provides Airoha Command Environment module API and definitions.
 *
 * NOTES:
 */

#ifndef ACE_CMD_H
#define ACE_CMD_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdint.h>

#include "pbuf.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* ACE type */
#define ACE_TYPE_COMMAND          (0x5A)
#define ACE_TYPE_RESPONSE         (0x5B)
#define ACE_TYPE_NOTIFICATION     (0x5D)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef union
{
    struct
    {
        uint8_t ch_byte: 4;
        uint8_t app_id: 4;
    } field;
    uint8_t value;
} ACE_CH_BYTE;

typedef struct
{
    ACE_CH_BYTE    pktId;
    uint8_t     type;
    uint16_t    length;
    uint16_t    id;
} __attribute__((packed)) ACE_COMMON_HDR_T, *PTR_ACE_COMMON_HDR_T;

typedef struct
{
    ACE_COMMON_HDR_T hdr;
    uint8_t payload[0];

} __attribute__((packed)) ACE_PKT_T;

typedef struct
{
    ACE_COMMON_HDR_T hdr;
    uint8_t status;
} __attribute__((packed)) ACE_PKT_ACK_T;

typedef struct
{
    uint16_t id;
    void *(*handler)(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
} ACE_SUB_HANDLER_T;

typedef struct
{
    uint16_t id_start;
    uint16_t id_end;
    void *(*handler)(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
    ACE_SUB_HANDLER_T *(*handler_query_id_list)(uint32_t *sub_handler_size);
} ACE_HANDLER_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

extern const ACE_HANDLER_T ace_handlers[];
uint32_t getAceHandlerSize();

void *ACE_CmdHandler(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
void *ACE_ClaimPacket(uint8_t ace_type, uint16_t ace_id, uint16_t dat_len);
void ACE_FreePacket(void *data);
void ACE_send(ACE_PKT_ACK_T *ptr_ace_pkt, struct pbuf *ptr_pbuf_in);

#endif  /* End of ACE_CMD_H */