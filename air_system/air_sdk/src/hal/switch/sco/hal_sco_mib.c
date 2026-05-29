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

/* FILE NAME:  hal_sco_mib.c
 * PURPOSE:
 * It provides hal mib module API.
 *
 * NOTES:
 *
 */

/*****************************************************************************
 * INCLUDE FILE DECLARATIONS
 *****************************************************************************
 */

#include <hal/switch/sco/hal_sco_mib.h>

#include <aml/aml.h>
#include <api/diag.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/switch/sco/hal_sco_reg.h>
#include <osal/osal.h>

/* DIAG_SET_MODULE_INFO(AIR_MODULE_MIB, "hal_sco_mib.c"); */
/*****************************************************************************
 * NAMING CONSTANT DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * MACRO VLAUE DECLARATIONS
 *****************************************************************************
 */
#define MIB_READ_DATA(unit, port, mib, reg, val)               \
    do                                                         \
    {                                                          \
        aml_readReg(unit, MIB_##reg(port), &val, sizeof(val)); \
        mib->reg = val;                                        \
    } while (0)
/*****************************************************************************
 * MACRO FUNCTION DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * DATA TYPE DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * GLOBAL VARIABLE DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * STATIC VARIABLE DECLARATIONS
 ******************************************************************************
 */

/*****************************************************************************
 * LOCAL SUBPROGRAM DECLARATIONS
 *****************************************************************************
 */
/* FUNCTION NAME: hal_sco_mib_clearAllCnt
 * PURPOSE:
 *      Clear all MIB counters on all ports.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_mib_clearAllCnt(
    const UI32_T unit)
{
    UI32_T u32dat;

    /* Write data to register */
    aml_readReg(unit, MIB_CCR, &u32dat, sizeof(u32dat));
    /* Restart MIB counter */
    u32dat &= ~MIB_CCR_MIB_ENABLE;
    aml_writeReg(unit, MIB_CCR, &u32dat, sizeof(u32dat));
    u32dat |= MIB_CCR_MIB_ENABLE;
    aml_writeReg(unit, MIB_CCR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mib_clearPortCnt
 * PURPOSE:
 *      Clear MIB counters for the specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_mib_clearPortCnt(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T u32dat, mac_port;

    /* translate air port to hardware mac port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    aml_readReg(unit, MIB_PCLR, &u32dat, sizeof(u32dat));
    /* Restart MIB counter */
    u32dat |= MIB_PCLR_PORT(mac_port);
    aml_writeReg(unit, MIB_PCLR, &u32dat, sizeof(u32dat));
    u32dat &= ~MIB_PCLR_PORT(mac_port);
    aml_writeReg(unit, MIB_PCLR, &u32dat, sizeof(u32dat));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mib_getPortCnt
 * PURPOSE: Get the MIB counter for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_rx_cnt      --  MIB Counters of Rx Event
 *      ptr_tx_cnt      --  MIB Counters of Tx Event
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_mib_getPortCnt(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_MIB_CNT_RX_T *ptr_rx_cnt,
    AIR_MIB_CNT_TX_T *ptr_tx_cnt)
{
    UI32_T u32dat, u32dat_h, mac_port;

    /* translate air port to hardware mac port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read Tx MIB Counter */
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TCDPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TCEPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TUPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TMPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TBPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TCEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TSCEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TMCEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TDEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TLCEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TXCEC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TPPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL64PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL65PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL128PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL256PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL512PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL1024PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TL1519PC, u32dat);
    aml_readReg(unit, MIB_TOCL(mac_port), &u32dat, sizeof(u32dat));
    aml_readReg(unit, MIB_TOCH(mac_port), &u32dat_h, sizeof(u32dat_h));
    ptr_tx_cnt->TOC = (UI64_T)(u32dat | ((UI64_T)u32dat_h << 32));
    MIB_READ_DATA(unit, mac_port, ptr_tx_cnt, TODPC, u32dat);

    /* Read Rx MIB Counter */
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RDPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RFPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RUPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RMPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RBPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RAEPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RCEPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RUSPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RFEPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, ROSPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RJEPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RPPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL64PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL65PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL128PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL256PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL512PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL1024PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RL1519PC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RCDPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RIDPC, u32dat);
    MIB_READ_DATA(unit, mac_port, ptr_rx_cnt, RADPC, u32dat);
    aml_readReg(unit, MIB_ROCL(mac_port), &u32dat, sizeof(u32dat));
    aml_readReg(unit, MIB_ROCH(mac_port), &u32dat_h, sizeof(u32dat_h));
    ptr_rx_cnt->ROC = (UI64_T)(u32dat | ((UI64_T)u32dat_h << 32));

    return AIR_E_OK;
}

/* FUNCTION NAME: hal_sco_mib_getFeatureCnt
 * PURPOSE: Get the Feature counter for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_cnt         --  MIB Counters
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_mib_getFeatureCnt(
    const UI32_T   unit,
    const UI32_T   port,
    AIR_MIB_CNT_T *ptr_cnt)
{
    UI32_T u32dat, mac_port;

    /* translate air port to hardware mac port */
    HAL_AIR_PORT_TO_MAC_PORT(unit, port, mac_port);

    /* Read Feature MIB Counter */
    /* Flow control drop Counter */
    MIB_READ_DATA(unit, mac_port, ptr_cnt, FCDPC, u32dat);
    /* WREF drop Counter*/
    MIB_READ_DATA(unit, mac_port, ptr_cnt, WRDPC, u32dat);
    /* Mirror drop counter*/
    MIB_READ_DATA(unit, mac_port, ptr_cnt, MRDPC, u32dat);
    /* Rx sflow sampling packet counter */
    MIB_READ_DATA(unit, mac_port, ptr_cnt, RSFSPC, u32dat);
    /* Rx sflow total packet counter */
    MIB_READ_DATA(unit, mac_port, ptr_cnt, RSFTPC, u32dat);

    return AIR_E_OK;
}
