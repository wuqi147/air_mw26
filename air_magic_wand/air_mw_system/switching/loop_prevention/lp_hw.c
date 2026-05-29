/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  lp_hw.c
 * PURPOSE:
 *    This file contains the implementation of hardware related functions for loop prevention.
 *
 * NOTES:
 *
 */
/* INCLUDE FILE DECLARATIONS
 */
#include    "lp.h"
#include    "lp_hw.h"
#include    "lp_log.h"
#include    "hal.h"
#include    "air_swc.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
#ifdef AIR_HARDWARE_SEND_LDF_SUPPORT
/* FUNCTION NAME:   lp_hw_sendLdf
 * PURPOSE:
 *      Send lp detect frame by hardware.
 *
 * INPUT:
 *      enable          --  Enable or disable hardward send lp detect frame
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *
 * NOTES:
 *      None
 */
void
lp_hw_sendLdf(
    BOOL_T  enable)
{
    UI32_T          unit = 0, port = 0;
    AIR_MAC_T       src_mac = {0};
    AIR_SWC_LPDET_CTRL_TYPE_T   type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;

    if (AIR_E_OK != air_swc_getSystemMac(unit, src_mac))
    {
        LP_LOG_ERROR("Get sys-mac error");
        return;
    }
    air_swc_setLoopDetectFrameSrcMac(unit, src_mac);
    AIR_PORT_FOREACH(PLAT_PORT_BMP_TOTAL, port)
    {
        if(PLAT_CPU_PORT == port)
        {
            /* Skip cpu port */
            continue;
        }
        air_swc_setLoopDetectCtrl(unit, port, type, enable);
    }
    LP_LOG_INFO("%s hw send ldf", (enable ? "Enable" : "Disable"));

    return;
}

/* FUNCTION NAME:   lp_hw_convertMacPortToAirPort
 * PURPOSE:
 *      This API is used to convert the mac port number to air port number.
 *
 * INPUT:
 *      mac_port         --  mac port number
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      air_port         --  air port number
 *
 * NOTES:
 *      None
 */
UI16_T
lp_hw_convertMacPortToAirPort(
    UI32_T   mac_port)
{
    UI16_T  air_port = 0;
    UI32_T  i = 0;
    UI32_T  unit = 0, port = 0;
    LP_CONFIG_INFO_T    *ptr_lp_config = NULL;

    /* HAL API is called temporarily. SDK API will be used in the future. */
    HAL_MAC_PORT_TO_AIR_PORT(unit, mac_port, port);
    air_port = port;
    LP_LOG_DEBUG("mac_port = %d, air_port = %d.", mac_port, air_port);
    ptr_lp_config = lp_getConfig();
    if (NULL != ptr_lp_config)
    {
        for (i = 0; i < MAX_TRUNK_NUM; i++)
        {
            if (0 == ptr_lp_config->trunk_info.group[i])
            {
                continue;
            }
            if (0 != (ptr_lp_config->trunk_info.group[i] & (0x01 << air_port)))
            {
                air_port |= (LP_TRUNK_GP1 << i);
                LP_LOG_DEBUG("air_port with trunk mask: 0x%x.", air_port);
                break;
            }
        }
    }

    return air_port;
}
#endif /* AIR_HARDWARE_SEND_LDF_SUPPORT */


