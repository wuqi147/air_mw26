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

#ifndef MW_CMD_LLDP_H
#define MW_CMD_LLDP_H

#ifdef AIR_SUPPORT_LLDPD
#include "mw_error.h"
#include "mw_types.h"
#include "lldp.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#define MW_LLDP_CMD_TYPE_GLOBAL_CFG         (0)
#define MW_LLDP_CMD_TYPE_TX_INTERVAL        (1)
#define MW_LLDP_CMD_TYPE_TX_HOLD            (2)
#define MW_LLDP_CMD_TYPE_REINIT_DELAY       (3)
#define MW_LLDP_CMD_TYPE_TX_DELAY           (4)
#define MW_LLDP_CMD_TYPE_PER_SYS_PER_MAC    (5)
#define MW_LLDP_CMD_TYPE_NOT_SEND_BLOCKING  (6)

#define MW_LLDP_PORT_ADMIN_MODE_DISABLE     (0)
#define MW_LLDP_PORT_ADMIN_MODE_TX          (1)
#define MW_LLDP_PORT_ADMIN_MODE_RX          (2)
#define MW_LLDP_PORT_ADMIN_MODE_TXRX        (3)

#define MW_LLDP_RSTP_OPER_STATE_DISABLE     (0)
#define MW_LLDP_RSTP_OPER_STATE_LEARNING    (1)
#define MW_LLDP_RSTP_OPER_STATE_FORWARDING  (2)
#define MW_LLDP_RSTP_OPER_STATE_DISCARDING  (3)

#define MW_LLDP_TX_INTERVAL_MIN             (5)
#define MW_LLDP_TX_INTERVAL_MAX             (32768)
#define MW_LLDP_TX_HOLD_MIN                 (2)
#define MW_LLDP_TX_HOLD_MAX                 (10)
#define MW_LLDP_REINIT_DELAY_MIN            (1)
#define MW_LLDP_REINIT_DELAY_MAX            (10)
#define MW_LLDP_TX_DELAY_MIN                (1)
#define MW_LLDP_TX_DELAY_MAX                (8192)
#define MW_LLDP_VLAN_ID_MIN                 (1)
#define MW_LLDP_VLAN_ID_MAX                 (4094)
#define MW_LLDP_VLAN_PRI_MIN                (0)
#define MW_LLDP_VLAN_PRI_MAX                (7)
#define MW_LLDP_TTL_MIN                     (0)
#define MW_LLDP_TTL_MAX                     (65535)


/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: mw_cmd_lldp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: LLDP.
 *
 * INPUT:
 *      ptr_tokens  --  Command tokens
 *      token_idx   --  The index of 1st valid token
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
mw_cmd_lldp_dispatcher(
    const C8_T *ptr_tokens[],
    UI32_T token_idx);

/* FUNCTION NAME: mw_cmd_lldp_usager
 * PURPOSE:
 *      Command usage for magic wand command: LLDP.
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
mw_cmd_lldp_usager(
    void);


#endif/*#ifdef AIR_SUPPORT_LLDPD*/
#endif
