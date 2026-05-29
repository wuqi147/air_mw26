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

/* FILE NAME:  air_swc.c
 * PURPOSE:
 *    It provide switch module API.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <air_swc.h>

#include <air_error.h>
#include <air_types.h>
#include <hal/common/hal.h>

/* GLOBAL VARIABLE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_SWC, "air_swc.c");

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   air_swc_setMgmtFrameCfg
 * PURPOSE:
 *      Set management frame config.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_cfg                  -- Management frame configuration
 *                                  AIR_SWC_MGMT_FRAME_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
air_swc_setMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_cfg);

    return HAL_FUNC_CALL(unit, swc, setMgmtFrameCfg, (unit, ptr_cfg));
}

/* FUNCTION NAME:   air_swc_getMgmtFrameCfg
 * PURPOSE:
 *      Get management frame config.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_cfg                  -- Management frame configuration
 *                                  AIR_SWC_MGMT_FRAME_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
air_swc_getMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_cfg);

    return HAL_FUNC_CALL(unit, swc, getMgmtFrameCfg, (unit, ptr_cfg));
}

/* FUNCTION NAME:   air_swc_setSystemMac
 * PURPOSE:
 *      Set the system MAC address.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- System MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
air_swc_setSystemMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(mac);

    return HAL_FUNC_CALL(unit, swc, setSystemMac, (unit, mac));
}

/* FUNCTION NAME:   air_swc_getSystemMac
 * PURPOSE:
 *      Get the system MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      mac                      -- System MAC address
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
air_swc_getSystemMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(mac);

    return HAL_FUNC_CALL(unit, swc, getSystemMac, (unit, mac));
}

/* FUNCTION NAME:   air_swc_setJumboSize
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 * INPUT:
 *      unit                     -- Device ID
 *      jumbo_size               -- Maximun jumbo packet size
 *                                  AIR_SWC_JUMBO_SIZE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_setJumboSize(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T jumbo_size)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(jumbo_size, (AIR_SWC_JUMBO_SIZE_LAST));

    return HAL_FUNC_CALL(unit, swc, setJumboSize, (unit, jumbo_size));
}

/* FUNCTION NAME:   air_swc_getJumboSize
 * PURPOSE:
 *      Get accepting jumbo frmes with specificied size.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_jumbo_size           -- Maximun jumbo packet size
 *                                  AIR_SWC_JUMBO_SIZE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getJumboSize(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_jumbo_size)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_jumbo_size);

    return HAL_FUNC_CALL(unit, swc, getJumboSize, (unit, ptr_jumbo_size));
}

/* FUNCTION NAME:   air_swc_setProperty
 * PURPOSE:
 *      Set switch property.
 * INPUT:
 *      unit                     -- Device ID
 *      property                 -- Select switch property
 *                                  AIR_SWC_PROPERTY_T
 *      param0                   -- 1: Enable
 *                                  0: Disable
 *      param1                   -- Reserved
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_setProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(property, AIR_SWC_PROPERTY_LAST);

    return HAL_FUNC_CALL(unit, swc, setProperty, (unit, property, param0, param1));
}

/* FUNCTION NAME:   air_swc_getProperty
 * PURPOSE:
 *      Get switch property.
 * INPUT:
 *      unit                     -- Device ID
 *      property                 -- Select switch property
 *                                  AIR_SWC_PROPERTY_T
 * OUTPUT:
 *      ptr_param0               -- 1: Enable
 *                                  0: Disable
 *      ptr_param1               -- Reserved
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ENUM_RANGE(property, AIR_SWC_PROPERTY_LAST);

    return HAL_FUNC_CALL(unit, swc, getProperty, (unit, property, ptr_param0, ptr_param1));
}

/* FUNCTION NAME:   air_swc_setPortMgmtFrameCfg
 * PURPOSE:
 *      Set the management frame config for a specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      ptr_cfg                  -- Management frame configuration
 *                                  AIR_SWC_MGMT_FRAME_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
air_swc_setPortMgmtFrameCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_cfg);
    HAL_CHECK_MIN_MAX_RANGE(ptr_cfg->pri, 0, AIR_SWC_PRI_MAX_NUM);

    return HAL_FUNC_CALL(unit, swc, setPortMgmtFrameCfg, (unit, port, ptr_cfg));
}

/* FUNCTION NAME:   air_swc_getPortMgmtFrameCfg
 * PURPOSE:
 *      Get the management frame config for a specific port.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      ptr_cfg                  -- Management frame configuration.
 *                                  The frame type mest be specified.
 *                                  AIR_SWC_MGMT_FRAME_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
air_swc_getPortMgmtFrameCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_cfg);

    return HAL_FUNC_CALL(unit, swc, getPortMgmtFrameCfg, (unit, port, ptr_cfg));
}

/* FUNCTION NAME:   air_swc_getCapacity
 * PURPOSE:
 *      Get resource capacity
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Resource type
 *                                  AIR_SWC_RSRC_T
 *      param                    -- Parameter if necessary
 * OUTPUT:
 *      ptr_size                 -- Size of capacity
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_size);

    return HAL_FUNC_CALL(unit, swc, getCapacity, (unit, type, param, ptr_size));
}

/* FUNCTION NAME:   air_swc_getUsage
 * PURPOSE:
 *     Get resource usage.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Resource type
 *                                  AIR_SWC_RSRC_T
 *      param                    -- Parameter if necessary
 * OUTPUT:
 *      ptr_cnt                  -- Count of usage
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_cnt);

    return HAL_FUNC_CALL(unit, swc, getUsage, (unit, type, param, ptr_cnt));
}

/* FUNCTION NAME:   air_swc_setLoopDetectFrameSrcMac
 * PURPOSE:
 *      Set the loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 *      mac                      -- Source MAC address
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_swc_setLoopDetectFrameSrcMac(
    const UI32_T    unit,
    const AIR_MAC_T mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(mac);

    return HAL_FUNC_CALL(unit, swc, setLoopDetectFrameSrcMac, (unit, mac));
}

/* FUNCTION NAME:   air_swc_getLoopDetectFrameSrcMac
 * PURPOSE:
 *      Get loop detect frame source MAC address.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      mac                      -- Source MAC address
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      It's unique and specified for loop frame.
 */
AIR_ERROR_NO_T
air_swc_getLoopDetectFrameSrcMac(
    const UI32_T unit,
    AIR_MAC_T    mac)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(mac);

    return HAL_FUNC_CALL(unit, swc, getLoopDetectFrameSrcMac, (unit, mac));
}

/* FUNCTION NAME:   air_swc_setLoopDetectCtrl
 * PURPOSE:
 *      Set the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      enable                   -- FALSE: Disable
 *                                  TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_setLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const BOOL_T                    enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);

    return HAL_FUNC_CALL(unit, swc, setLoopDetectCtrl, (unit, port, type, enable));
}

/* FUNCTION NAME:   air_swc_getLoopDetectCtrl
 * PURPOSE:
 *      Get the loop detect control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- FALSE: Disable
 *                                  TRUE: Enable
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getLoopDetectCtrl(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    BOOL_T                         *ptr_enable)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT(unit, port);
    HAL_CHECK_PTR(ptr_enable);

    return HAL_FUNC_CALL(unit, swc, getLoopDetectCtrl, (unit, port, type, ptr_enable));
}

/* FUNCTION NAME:   air_swc_clearLoopDetectStatus
 * PURPOSE:
 *      Clear the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_clearLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    const AIR_PORT_BITMAP_T         port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PORT_BITMAP(unit, port_bitmap);

    return HAL_FUNC_CALL(unit, swc, clearLoopDetectStatus, (unit, type, port_bitmap));
}

/* FUNCTION NAME:   air_swc_getLoopDetectStatus
 * PURPOSE:
 *      Get the loop detect status.
 * INPUT:
 *      unit                     -- Device ID
 *      type                     -- Loop detect control type
 *                                  AIR_SWC_LPDET_CTRL_TYPE_T
 * OUTPUT:
 *      port_bitmap              -- Loop status port bitmap
 *                                  AIR_PORT_BITMAP_T
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getLoopDetectStatus(
    const UI32_T                    unit,
    const AIR_SWC_LPDET_CTRL_TYPE_T type,
    AIR_PORT_BITMAP_T               port_bitmap)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(port_bitmap);

    return HAL_FUNC_CALL(unit, swc, getLoopDetectStatus, (unit, type, port_bitmap));
}

/* FUNCTION NAME:   air_swc_getChipUid
 * PURPOSE:
 *     Get the chip unique ID
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_uid                  -- Chip unique ID
 * RETURN:
 *      AIR_E_OK                 -- Operation Success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_OTHERS             -- Other errors.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_swc_getChipUid(
    const UI32_T unit,
    UI64_T      *ptr_uid)
{
    HAL_CHECK_UNIT(unit);
    HAL_CHECK_PTR(ptr_uid);

    return HAL_FUNC_CALL(unit, swc, getChipUid, (unit, ptr_uid));
}
