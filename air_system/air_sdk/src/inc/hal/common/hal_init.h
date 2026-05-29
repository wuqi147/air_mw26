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

/* FILE NAME:  hal_init.h
 * PURPOSE:
 *      It provide hal related initial APIs.
 * NOTES:
 */

#ifndef HAL_INIT_H
#define HAL_INIT_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   hal_init_initSdkPortMap
 * PURPOSE:
 *      This API is used to initialize SDK port mapping. All SDK API's port
 *      is based on those ports initialized by this API.
 * INPUT:
 *      unit                -- The unit ID
 *      port_map_cnt        -- The port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_OTHERS        -- Operation failed.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initSdkPortMap(
    const UI32_T               unit,
    const UI32_T               port_map_cnt,
    const AIR_INIT_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   hal_init_getSdkPortMap
 * PURPOSE:
 *      This API is used to get SDK port mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt    -- The pointer of port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_port_map
 *      parameter to get the port map.
 */
AIR_ERROR_NO_T
hal_init_getSdkPortMap(
    const UI32_T         unit,
    UI32_T              *ptr_port_map_cnt,
    AIR_INIT_PORT_MAP_T *ptr_port_map);

#ifdef AIR_EN_POE
/* FUNCTION NAME:   hal_init_initPoePortMap
 * PURPOSE:
 *      This API is used to initialize PoE port mapping.
 * INPUT:
 *      unit                -- The unit ID
 *      port_map_cnt        -- The port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Parameter is wrong.
 *      AIR_E_NOT_INITED    -- SDK port is not initialized.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initPoePortMap(
    const UI32_T                   unit,
    const UI32_T                   port_map_cnt,
    const AIR_INIT_POE_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   hal_init_getPoePortMap
 * PURPOSE:
 *      This API is used to get PoE port mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_port_map_cnt    -- The pointer of port mapping count
 *      ptr_port_map        -- The pointer of port mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_port_map
 *      parameter to get the port map.
 */
AIR_ERROR_NO_T
hal_init_getPoePortMap(
    const UI32_T             unit,
    UI32_T                  *ptr_port_map_cnt,
    AIR_INIT_POE_PORT_MAP_T *ptr_port_map);

/* FUNCTION NAME:   hal_init_initPoeDeviceMap
 * PURPOSE:
 *      This API is used to initialize PoE device mapping.
 * INPUT:
 *      unit                -- The unit ID
 *      device_map_cnt      -- The device mapping count
 *      ptr_device_map      -- The pointer of device mapping information
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Parameter is wrong.
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_init_initPoeDeviceMap(
    const UI32_T                     unit,
    const UI32_T                     device_map_cnt,
    const AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map);

/* FUNCTION NAME:   hal_init_getPoeDeviceMap
 * PURPOSE:
 *      This API is used to get PoE device mapping.
 * INPUT:
 *      unit                -- The unit ID
 * OUTPUT:
 *      ptr_device_map_cnt  -- The pointer of device mapping count
 *      ptr_device_map      -- The pointer of device mapping information
 * RETURN:
 *      AIR_E_OK            -- Operation success.
 *      AIR_E_BAD_PARAMETER -- Operation fail.
 *
 * NOTES:
 *      User need to allocate enough memory and pass the pointer as ptr_device_map
 *      parameter to get the device map.
 */
AIR_ERROR_NO_T
hal_init_getPoeDeviceMap(
    const UI32_T               unit,
    UI32_T                    *ptr_device_map_cnt,
    AIR_INIT_POE_DEVICE_MAP_T *ptr_device_map);
#endif /* #ifndef AIR_EN_POE */

#endif /* #ifndef HAL_INIT_H */
