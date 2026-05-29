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

/* FILE NAME:  air_mib.c
 * PURPOSE:
 *      It provide pkt module api.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include <air_mib.h>

#include <hal/common/hal.h>
#include <osal/osal.h>

DIAG_SET_MODULE_INFO(AIR_MODULE_MIB, "air_mib.c");

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
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);

    rc = HAL_FUNC_CALL(unit, mib, clearAllCnt, (unit));

    return rc;
}

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
    const UI32_T port)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);

    rc = HAL_FUNC_CALL(unit, mib, clearPortCnt, (unit, port));

    return rc;
}

/* FUNCTION NAME:   air_mib_getPortCnt
 * PURPOSE:
 *      Get the MIB counter for a specific port.
 *
 * INPUT:
 *      unit                 -- Device ID
 *      port                 -- Index of port number
 * OUTPUT:
 *      ptr_rx_cnt           -- MIB Counters of Rx Event
 *      ptr_tx_cnt           -- MIB Counters of Tx Event
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
    AIR_MIB_CNT_TX_T *ptr_tx_cnt)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_rx_cnt);
    HAL_CHECK_PTR(ptr_tx_cnt);

    rc = HAL_FUNC_CALL(unit, mib, getPortCnt, (unit, port, ptr_rx_cnt, ptr_tx_cnt));

    return rc;
}

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
    AIR_MIB_CNT_T *ptr_cnt)
{
    AIR_ERROR_NO_T rc;

    /* parameter sanity check */
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_cnt);

    rc = HAL_FUNC_CALL(unit, mib, getFeatureCnt, (unit, port, ptr_cnt));

    return rc;
}
