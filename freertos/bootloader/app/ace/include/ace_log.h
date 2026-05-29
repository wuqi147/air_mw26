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

/* FILE NAME:  ace_log.h
 * PURPOSE:
 *      Defined ACE log API list.
 *
 * NOTES:
 */

#ifndef ACE_LOG_H
#define ACE_LOG_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdint.h>

#include "pbuf.h"
#include "ace_cmd.h"

#ifdef AIR_SUPPORT_ACE_ETHERNET_LOG
/* NAMING CONSTANT DECLARATIONS
*/
#define ACE_TEXT_LOG                              (0x0F10)
#define ACE_MSG_ID_LOG                            (0x0F11)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef struct
{
    uint16_t log_seq;
    uint16_t reserved;
    uint32_t timestamp;
    uint8_t log_string[0];
} __attribute__((packed)) ACE_TEXT_LOG_PAYLOAD_T;

typedef struct
{
    ACE_COMMON_HDR_T hdr;
    ACE_TEXT_LOG_PAYLOAD_T payload;
} __attribute__((packed)) ACE_TEXT_LOG_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

extern uint32_t g_log_port;
extern uint8_t g_ace_log_enabled;
extern struct eth_hdr g_da_ether_hdr;
#endif

extern uint8_t g_ace_log_enabled;

void send_ace_log(uint8_t text[], uint32_t size_log, struct pbuf *ptr_pbuf);

#endif  /* End of ACE_LOG_H */