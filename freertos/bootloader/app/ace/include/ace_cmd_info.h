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

/* FILE NAME:  ace_cmd_labtest.h
 * PURPOSE:
 *      It provides LAB test ACE command API and definitions.
 *
 * NOTES:
 */

#ifndef ACE_CMD_INFO_H
#define ACE_CMD_INFO_H

/* INCLUDE FILE DECLARATIONS
 */

#include "pbuf.h"
#include "ace_cmd.h"

/* NAMING CONSTANT DECLARATIONS
*/

#define EEP_CONFIG_DATA_LOCATION                  (0x2001f000)
#define EEP_CONFIG_DATA_SIZE                      (0x1000)

#define MAC_ADDRESS_LEN       6
#define ACE_FLAGS_ADDRESS_START                   (EEP_CONFIG_DATA_LOCATION + MAC_ADDRESS_LEN)
#define ACE_FLAGS_MAC_WRITE_PROTECT               (EEP_CONFIG_DATA_LOCATION + MAC_ADDRESS_LEN)
#define ACE_FLAGS_ADDRESS_ACE_ENABLE              (EEP_CONFIG_DATA_LOCATION + MAC_ADDRESS_LEN + 1)
#define ACE_PRODUCT_INFO_START                    (EEP_CONFIG_DATA_LOCATION + MAC_ADDRESS_LEN + 2)
#define ACE_PRODUCT_INFO_END                      (EEP_CONFIG_DATA_LOCATION + EEP_CONFIG_DATA_SIZE)

#define ACE_FLAGS_ACE_ENABLED                     (0xFF)
#define ACE_FLAGS_MAC_WRITE_UNPROTECT             (0xFF)


/* MACRO FUNCTION DECLARATIONS
 */

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif	/* End of BIT */

/* bits range: for example BITS(16,23) = 0xFF0000*/
#ifndef BITS
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#endif	/* End of BITS */

/* bits range: for example BITS_RANGE(16,4) = 0x0F0000*/
#ifndef BITS_RANGE
#define BITS_RANGE(offset, range)           BITS((offset), ((offset)+(range)-1))
#endif	/* End of BITS_RANGE */

/* bits offset right: for example BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef BITS_OFF_R
#define BITS_OFF_R(val, offset, range)      (((val) >> offset) & (BITS(0, (range) - 1)))
#endif	/* End of BITS_OFF_R */

/* bits offset left: for example BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef BITS_OFF_L
#define BITS_OFF_L(val, offset, range)      (((val) & (BITS(0, (range) - 1))) << (offset))
#endif	/* End of BITS_OFF_L */

/* DATA TYPE DECLARATIONS
 */

typedef struct PLAT_SYS_INFO_S
{
    unsigned char mac_addr[MAC_ADDRESS_LEN];
}PLAT_SYS_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

void *ACE_CmdHandler_Info(ACE_PKT_T *ptr_ace_pkt, struct pbuf *ptr_pbuf);
ACE_SUB_HANDLER_T *ACE_QueryCmdHandler_Info(uint32_t *sub_handler_size);

#endif  /* End of ACE_CMD_INFO_H */
