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

/* FILE NAME:  hal_sco_swc.h
 * PURPOSE:
 *  Define switch module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_SWC_H
#define HAL_SCO_SWC_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_swc.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_SCO_FWD_MODE_SYS_SETTING             (0)
#define HAL_SCO_FWD_MODE_SYS_SETTING_EXCLUDE_CPU (4)
#define HAL_SCO_FWD_MODE_SYS_SETTING_INCLUDE_CPU (5)
#define HAL_SCO_FWD_MODE_CPU_ONLY                (6)
#define HAL_SCO_FWD_MODE_DROP                    (7)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: hal_sco_swc_setMgmtFrameCfg
 * PURPOSE:
 *      Set management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      ptr_cfg         --  config for specific management frame
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_sco_swc_setMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_sco_swc_getMgmtFrameCfg
 * PURPOSE:
 *     Get management frame config.
 *
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_cfg         --  config for specific management frame
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      REV_xx = 01-80-C2-00-00-xx of destination mac
 *      REV_UN = others
 */
AIR_ERROR_NO_T
hal_sco_swc_getMgmtFrameCfg(
    const UI32_T              unit,
    AIR_SWC_MGMT_FRAME_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_sco_swc_setSystemMac
 * PURPOSE:
 *      Set the system MAC address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      mac             --  System MAC address
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
hal_sco_swc_setSystemMac(
    const UI32_T    unit,
    const AIR_MAC_T mac);

/* FUNCTION NAME: hal_sco_swc_getSysMac
 * PURPOSE:
 *      Get the system MAC address.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      mac             --  System MAC address
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      It's unique and specified for pause frame.
 */
AIR_ERROR_NO_T
hal_sco_swc_getSystemMac(
    const UI32_T unit,
    AIR_MAC_T    mac);

/* FUNCTION NAME:
 *      hal_sco_swc_init
 * PURPOSE:
 *      This API is used to initialize switch related items:
 *      1. Set REV02/03/0E/20/21 as BPDU frames.
 *      2. Dynamic entry auto deleted when port link down.
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_swc_setJumboSize
 * PURPOSE:
 *      Set accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      frame_len       --  AIR_SWC_JUMBO_SIZE_1518,
 *                          AIR_SWC_JUMBO_SIZE_1536,
 *                          AIR_SWC_JUMBO_SIZE_1552,
 *                          AIR_SWC_JUMBO_SIZE_2048,
 *                          AIR_SWC_JUMBO_SIZE_3072,
 *                          AIR_SWC_JUMBO_SIZE_4096,
 *                          AIR_SWC_JUMBO_SIZE_5120,
 *                          AIR_SWC_JUMBO_SIZE_6144,
 *                          AIR_SWC_JUMBO_SIZE_7168,
 *                          AIR_SWC_JUMBO_SIZE_8192,
 *                          AIR_SWC_JUMBO_SIZE_9216,
 *                          AIR_SWC_JUMBO_SIZE_12288,
 *                          AIR_SWC_JUMBO_SIZE_15360,
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

/* FUNCTION NAME:
 *      hal_sco_swc_deinit
 * PURPOSE:
 *      This API is used to deinitialize switch related items:
 * INPUT:
 *      unit             -- unit id
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_deinit(
    const UI32_T unit);

AIR_ERROR_NO_T
hal_sco_swc_setJumboSize(
    const UI32_T               unit,
    const AIR_SWC_JUMBO_SIZE_T frame_len);

/* FUNCTION NAME: hal_sco_swc_getJumboSize
 * PURPOSE:
 *      Get accepting jumbo frmes with specificied size.
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_frame_len   --  AIR_SWC_JUMBO_SIZE_1518,
 *                          AIR_SWC_JUMBO_SIZE_1536,
 *                          AIR_SWC_JUMBO_SIZE_1552,
 *                          AIR_SWC_JUMBO_SIZE_2048,
 *                          AIR_SWC_JUMBO_SIZE_3072,
 *                          AIR_SWC_JUMBO_SIZE_4096,
 *                          AIR_SWC_JUMBO_SIZE_5120,
 *                          AIR_SWC_JUMBO_SIZE_6144,
 *                          AIR_SWC_JUMBO_SIZE_7168,
 *                          AIR_SWC_JUMBO_SIZE_8192,
 *                          AIR_SWC_JUMBO_SIZE_9216,
 *                          AIR_SWC_JUMBO_SIZE_12288,
 *                          AIR_SWC_JUMBO_SIZE_15360,
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getJumboSize(
    const UI32_T          unit,
    AIR_SWC_JUMBO_SIZE_T *ptr_frame_len);

/* FUNCTION NAME: hal_sco_swc_setProperty
 * PURPOSE:
 *      Set switch property.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      property        --  Select switch property
 *                          AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH
 *                          AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL
 *      param0          --  1: Enable 0: Disable
 *      param1          --  Reserved
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
hal_sco_swc_setProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    const UI32_T             param0,
    const UI32_T             param1);

/* FUNCTION NAME: hal_sco_swc_getProperty
 * PURPOSE:
 *      Get switch property.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      property        --  Select switch property
 *                          AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH
 *                          AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL
 *
 * OUTPUT:
 *      ptr_param0      --  1: Enable 0: Disable
 *      ptr_param1      --  Reserved
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getProperty(
    const UI32_T             unit,
    const AIR_SWC_PROPERTY_T property,
    UI32_T                  *ptr_param0,
    UI32_T                  *ptr_param1);

/* FUNCTION NAME: hal_sco_swc_getGlobalFreePages
 * PURPOSE:
 *      Get the free page link counter
 *
 * INPUT:
 *      unit            --  Select device ID
 *
 * OUTPUT:
 *      ptr_fp_cnt      --  Free page counter
 *      ptr_min_fp_cnt  --  Minimal Free page counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getGlobalFreePages(
    const UI32_T unit,
    UI32_T      *ptr_fp_cnt,
    UI32_T      *ptr_min_fp_cnt);

/* FUNCTION NAME: hal_sco_swc_getPortAllocatedPages
 * PURPOSE:
 *      Get the allocated free page counter in RxCtrl of specific port
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_fp_cnt      --  Free page counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getPortAllocatedPages(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_fp_cnt);

/* FUNCTION NAME: hal_sco_swc_getPortUsedPages
 * PURPOSE:
 *      Get the used resource counter of specific port/queue
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      queue           --  Select queue index
 *      mode            --  0: Page counter
 *                          1: Packet counter
 * OUTPUT:
 *      ptr_cnt         --  Used resource counter
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_OTHERS
 *
 * NOTES:
 *      None
 */

AIR_ERROR_NO_T
hal_sco_swc_getPortUsedPages(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T queue,
    const UI32_T mode,
    UI32_T      *ptr_cnt);

/* FUNCTION NAME: hal_sco_swc_getCapacity
 * PURPOSE:
 *      Get the specific type resource capacity
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_size        --  Size of capacity
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getCapacity(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_size);

/* FUNCTION NAME: hal_sco_swc_getUsage
 * PURPOSE:
 *      Get the specific type resource usage
 * INPUT:
 *      unit            --  Select device ID
 *      type            --  Select resource type
 *                          AIR_SWC_RSRC_T
 *      param           --  Parameter if necessary
 * OUTPUT:
 *      ptr_cnt         --  Count of usage
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getUsage(
    const UI32_T         unit,
    const AIR_SWC_RSRC_T type,
    const UI32_T         param,
    UI32_T              *ptr_cnt);

/* FUNCTION NAME: hal_sco_swc_getChipUid
 * PURPOSE:
 *      Get the chip unique ID
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_uid         --  Chip unique ID

 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_swc_getChipUid(
    const UI32_T unit,
    UI64_T      *ptr_uid);

#endif /* end of HAL_SCO_SWC_H */
