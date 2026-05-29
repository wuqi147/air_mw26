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

/* FILE NAME:   air_ifmon.h
 * PURPOSE:
 *      It provides IFMON module API.
 * NOTES:
 */

#ifndef AIR_IFMON_H
#define AIR_IFMON_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* Interface Monitor Mode */
typedef enum
{
    AIR_IFMON_MODE_INTR = 0,
    AIR_IFMON_MODE_POLL,
    AIR_IFMON_MODE_LAST
} AIR_IFMON_MODE_T;

typedef void (*AIR_IFMON_NOTIFY_FUNC_T)(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void        *ptr_cookie);

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   air_ifmon_setMode
 * PURPOSE:
 *      This API is used to set interface monitor port bitmap, and
 *      interface monitor interval.
 * INPUT:
 *      unit                    -- Device unit number
 *      mode                    -- Interface monitor mode
 *                              -- AIR_IFMON_MODE_T
 *      port_bitmap             -- Interface monitor port bitmap
 *      interval_ms             -- Interface monitor polling interval
 *                                 in miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      The polling interval is valid if and only if the interface
 *      monitor polling mode is used. Only polling mode is supported
 *      for now.
 */
AIR_ERROR_NO_T
air_ifmon_setMode(
    const UI32_T            unit,
    const AIR_IFMON_MODE_T  mode,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            interval_ms);

/* FUNCTION NAME:   air_ifmon_getMode
 * PURPOSE:
 *      This API is used to get interface monitor port bitmap, and
 *      interface monitor interval.
 * INPUT:
 *      unit                    -- Device unit number
 * OUTPUT:
 *      ptr_mode                -- Pointer for interface monitor mode
 *                              -- AIR_IFMON_MODE_T
 *      ptr_port_bitmap         -- Pointer for interface monitor port
 *                                 bitmap
 *      ptr_interval_ms         -- Pointer for interface monitor
 *                                 polling interval in miliseconds
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      The polling interval is valid if and only if the interface
 *      monitor polling mode is used.
 */
AIR_ERROR_NO_T
air_ifmon_getMode(
    const UI32_T       unit,
    AIR_IFMON_MODE_T  *ptr_mode,
    AIR_PORT_BITMAP_T *ptr_port_bitmap,
    UI32_T            *ptr_interval_ms);

/* FUNCTION NAME:   air_ifmon_register
 * PURPOSE:
 *      This API is used to register a callback function to handle a
 *      port link change.
 * INPUT:
 *      unit                    -- Device unit number
 *      notify_func             -- Callback function
 *      ptr_cookie              -- Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_TABLE_FULL        -- Table is full.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ifmon_register(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

/* FUNCTION NAME:   air_ifmon_deregister
 * PURPOSE:
 *      This API is used to deregister a callback function from
 *      callback functions.
 * INPUT:
 *      unit                    -- Device unit number
 *      notify_func             -- Callback function
 *      ptr_cookie              -- Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 *      AIR_E_ENTRY_NOT_FOUND   -- Entry is not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ifmon_deregister(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

/* FUNCTION NAME:   air_ifmon_setMonitorState
 * PURPOSE:
 *      To set monitor state.
 * INPUT:
 *      unit                    -- Device unit number
 *      enable                  -- Monitor state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ifmon_setMonitorState(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:   air_ifmon_getMonitorState
 * PURPOSE:
 *      To get monitor state.
 * INPUT:
 *      unit                    -- Device unit number
 * OUTPUT:
 *      ptr_enable              -- Pointer for monitor state
 * RETURN:
 *      AIR_E_OK                -- Operation success.
 *      AIR_E_BAD_PARAMETER     -- Parameter is wrong.
 *      AIR_E_NOT_INITED        -- Module is not initialized.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_ifmon_getMonitorState(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

#endif /* End of AIR_IFMON_H */
