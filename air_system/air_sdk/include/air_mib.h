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

/* FILE NAME:  air_mib.h
 * PURPOSE:
 *      Provide the interface for packet TX/RX configuration.
 *
 * NOTES:
 *
 */

#ifndef AIR_PKT_H
#define AIR_PKT_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING DECLARATIONS
 */
typedef struct AIR_MIB_CNT_TX_S
{
    /* TX Collision Drop Packet */
    UI32_T TCDPC;

    /* TX FCS Error Packet */
    UI32_T TCEPC;

    /* TX Unicast Packet */
    UI32_T TUPC;

    /* TX Multicast Packet */
    UI32_T TMPC;

    /* TX Broadcast Packet */
    UI32_T TBPC;

    /* TX Collision Event Count */
    UI32_T TCEC;

    /* TX Single Collision Event Count */
    UI32_T TSCEC;

    /* TX Multiple Conllision Event Count */
    UI32_T TMCEC;

    /* TX Deferred Event Count */
    UI32_T TDEC;

    /* TX Late Collision Event Count */
    UI32_T TLCEC;

    /* TX Excessive Collision Event Count */
    UI32_T TXCEC;

    /* TX Pause Packet */
    UI32_T TPPC;

    /* TX Packet Length 64 bytes */
    UI32_T TL64PC;

    /* TX Packet Length 65 ~ 127 bytes */
    UI32_T TL65PC;

    /* TX Packet Length 128 ~ 255 bytes */
    UI32_T TL128PC;

    /* TX Packet Length 256 ~ 511 bytes */
    UI32_T TL256PC;

    /* TX Packet Length 512 ~ 1023 bytes */
    UI32_T TL512PC;

    /* TX Packet Length 1024 ~ 1518 bytes */
    UI32_T TL1024PC;

    /* TX Packet Length 1519 ~ max bytes */
    UI32_T TL1519PC;

    /* TX Octets (64 bit-width)*/
    UI64_T TOC;

    /* TX Oversize Drop Packet(large than 16383 bytes) */
    UI32_T TODPC;
} AIR_MIB_CNT_TX_T;

typedef struct AIR_MIB_CNT_RX_S
{
    /* RX Drop Packet */
    UI32_T RDPC;

    /* RX filtering Packet */
    UI32_T RFPC;

    /* RX Unicast Packet */
    UI32_T RUPC;

    /* RX Multicast Packet */
    UI32_T RMPC;

    /* RX Broadcast Packet */
    UI32_T RBPC;

    /* RX Alignment Error Packet */
    UI32_T RAEPC;

    /* RX CRC Packet */
    UI32_T RCEPC;

    /* RX Undersize Packet */
    UI32_T RUSPC;

    /* RX Fragment Error Packet */
    UI32_T RFEPC;

    /* RX Oversize Packet */
    UI32_T ROSPC;

    /* RX Jabber Error Packet */
    UI32_T RJEPC;

    /* RX Pause Packet */
    UI32_T RPPC;

    /* RX Packet Length 64 bytes */
    UI32_T RL64PC;

    /* RX Packet Length 65 ~ 127 bytes */
    UI32_T RL65PC;

    /* RX Packet Length 128 ~ 255 bytes */
    UI32_T RL128PC;

    /* RX Packet Length 256 ~ 511 bytes */
    UI32_T RL256PC;

    /* RX Packet Length 512 ~ 1023 bytes */
    UI32_T RL512PC;

    /* RX Packet Length 1024 ~ 1518 bytes */
    UI32_T RL1024PC;

    /* RX Packet Length 1519 ~ max bytes */
    UI32_T RL1519PC;

    /* RX Octets (64 bit-width)*/
    UI64_T ROC;

    /* RX CTRL Drop Packet */
    UI32_T RCDPC;

    /* RX Ingress Drop Packet */
    UI32_T RIDPC;

    /* RX ARL Drop Packet */
    UI32_T RADPC;
} AIR_MIB_CNT_RX_T;

typedef struct AIR_MIB_CNT_S
{
    /* Flow CTRL Drop Packet */
    UI32_T FCDPC;

    /* WRED Drop Packet */
    UI32_T WRDPC;

    /* Mirror Drop Packet */
    UI32_T MRDPC;

    /* RX sFlow Sampling Packet */
    UI32_T RSFSPC;

    /* RX sFlow Total Packet */
    UI32_T RSFTPC;
} AIR_MIB_CNT_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_mib_clearAllCnt
 * PURPOSE:
 *      Clear all MIB counters on all ports.
 * INPUT:
 *      unit                 -- Device ID
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_clearAllCnt(
    const UI32_T unit);

/* FUNCTION NAME:   air_mib_clearPortCnt
 * PURPOSE:
 *      Clear MIB counters for the specific port.
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_clearPortCnt(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   air_mib_getPortCnt
 * PURPOSE:
 *      Get the MIB counter for a specific port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 * OUTPUT:
 *      ptr_rx_cnt           -- MIB Counters of Rx Event
 *                              AIR_MIB_CNT_RX_T
 *      ptr_tx_cnt           -- MIB Counters of Tx Event
 *                              AIR_MIB_CNT_TX_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getPortCnt(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_MIB_CNT_RX_T *ptr_rx_cnt,
    AIR_MIB_CNT_TX_T *ptr_tx_cnt);

/* FUNCTION NAME:   air_mib_getFeatureCnt
 * PURPOSE:
 *      Get the MIB Feature counter for a specific port.
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 * OUTPUT:
 *      ptr_cnt              -- MIB Counters
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_mib_getFeatureCnt(
    const UI32_T   unit,
    const UI32_T   port,
    AIR_MIB_CNT_T *ptr_cnt);

#endif /* End of AIR_MIB_H */
