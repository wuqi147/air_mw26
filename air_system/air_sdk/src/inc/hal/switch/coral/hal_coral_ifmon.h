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

/* FILE NAME:  hal_coral_ifmon.h
 * PURPOSE:
 *      It provide HAL ifmon module API.
 * NOTES:
 */

#ifndef HAL_CORAL_IFMON_H
#define HAL_CORAL_IFMON_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_ifmon.h>
#include <air_types.h>
/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_CORAL_IFMON_MODE_DFLT     (AIR_IFMON_MODE_POLL)
#define HAL_CORAL_IFMON_INTERVAL_DFLT (500)

#define HAL_CORAL_IFMON_STACK_SIZE (4 * 1024)
#define HAL_CORAL_IFMON_THREAD_PRI (0)

#define HAL_CORAL_IFMON_NOTIFY_HANDLER_CNT (8)
/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_CORAL_IFMON_CFG_LOCK(unit)   hal_coral_ifmon_lockIfmonResource(unit)
#define HAL_CORAL_IFMON_CFG_UNLOCK(unit) hal_coral_ifmon_unlockIfmonResource(unit)

#define HAL_CORAL_IFMON_PORT_LOCK(unit)   hal_coral_ifmon_lockIfmonPortResource(unit)
#define HAL_CORAL_IFMON_PORT_UNLOCK(unit) hal_coral_ifmon_unlockIfmonPortResource(unit)
/* DATA TYPE DECLARATIONS
 */
typedef struct HAL_CORAL_IFMON_STAT_S
{
    AIR_PORT_BITMAP_T link_bitmap;
} HAL_CORAL_IFMON_STATE_T;

typedef struct HAL_CORAL_IFMON_NOTIFY_HANDLER_S
{
    AIR_IFMON_NOTIFY_FUNC_T notify_func;
    void                   *ptr_cookie;
} HAL_CORAL_IFMON_NOTIFY_HANDLER_T;

typedef struct HAL_CORAL_IFMON_CB_S
{
    AIR_SEMAPHORE_ID_T               sem_conf;
    AIR_SEMAPHORE_ID_T               sem_portstatus;
    BOOL_T                           monitor_state;
    AIR_THREAD_ID_T                  thread_id;
    AIR_IFMON_MODE_T                 mode;
    UI32_T                           interval;
    AIR_PORT_BITMAP_T                scan_port_bitmap;
    HAL_CORAL_IFMON_STATE_T          dev_state;
    HAL_CORAL_IFMON_STATE_T          cur_state;
    HAL_CORAL_IFMON_STATE_T          new_state;
    AIR_PORT_STATUS_T                dev_status[AIR_PORT_NUM];
    HAL_CORAL_IFMON_NOTIFY_HANDLER_T notify_handler[HAL_CORAL_IFMON_NOTIFY_HANDLER_CNT];
} HAL_CORAL_IFMON_CB_T;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_coral_ifmon_init
 *
 * PURPOSE:
 *      Initialize IfMon function.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ifmon_deinit
 *
 * PURPOSE:
 *      Deinitialize IfMon function.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ifmon_lockIfmonResource
 *
 * PURPOSE:
 *      Lock the resource of IfMon.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_lockIfmonResource(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ifmon_unlockIfmonResource
 *
 * PURPOSE:
 *      Unlock the resource of IfMon.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_unlockIfmonResource(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ifmon_lockIfmonPortResource
 *
 * PURPOSE:
 *      Lock the resource of IfMon port status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_lockIfmonPortResource(
    const UI32_T unit);

/* FUNCTION NAME: hal_coral_ifmon_unlockIfmonPortResource
 *
 * PURPOSE:
 *      Unlock the resource of IfMon port status.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  success
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_unlockIfmonPortResource(
    const UI32_T unit);

/* FUNCTION NAME:   hal_coral_ifmon_register
 * PURPOSE:
 *      To register a callback function to handle a port link change.
 * INPUT:
 *      unit                --  Device unit number
 *      notify_func         --  Callback function
 *      ptr_cookie          --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ifmon_register(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

/* FUNCTION NAME:   hal_coral_ifmon_deregister
 * PURPOSE:
 *      To deregister a callback function from callback functions.
 * INPUT:
 *      unit                  --  Device unit number
 *      notify_func           --  Callback function
 *      ptr_cookie            --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK              --  Operation is successful.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ifmon_deregister(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

/* FUNCTION NAME:   hal_coral_ifmon_setMode
 * PURPOSE:
 *      This API is used to set interface monitor mode, interface monitor
 *      port bitmap and interface monitor interval.
 * INPUT:
 *      unit                --  Device unit number
 *      mode                --  Interface monitor mode
 *      port_bitmap         --  Interface monitor port bitmap
 *      interval            --  Interface monitor polling interval in
 *                              miliseconds
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      The polling interval is valid if and only if the interface monitor
 *      polling mode is used.
 */
AIR_ERROR_NO_T
hal_coral_ifmon_setMode(
    const UI32_T            unit,
    const AIR_IFMON_MODE_T  mode,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            interval);

/* FUNCTION NAME:   hal_coral_ifmon_getMode
 * PURPOSE:
 *      This API is used to get interface monitor mode, interface monitor
 *      port bitmap and interface monitor interval.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_mode            --  Pointer for interface monitor mode
 *      ptr_port_bitmap     --  Pointer for interface monitor port bitmap
 *      ptr_interval        --  Pointer for interface monitor polling interval
 *                              in miliseconds
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *      The polling interval is valid if and only if the interface monitor
 *      polling mode is used.
 */
AIR_ERROR_NO_T
hal_coral_ifmon_getMode(
    const UI32_T       unit,
    AIR_IFMON_MODE_T  *ptr_mode,
    AIR_PORT_BITMAP_T *ptr_port_bitmap,
    UI32_T            *ptr_interval);

/* FUNCTION NAME:   hal_coral_ifmon_setMonitorState
 * PURPOSE:
 *      To set monitor state.
 * INPUT:
 *      unit                --  Device unit number
 *      enable              --  Monitor state
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_setMonitorState(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_coral_ifmon_getMonitorState
 * PURPOSE:
 *      To get monitor state.
 * INPUT:
 *      unit                --  Device unit number
 * OUTPUT:
 *      ptr_enable          --  Pointer for monitor state
 * RETURN:
 *      AIR_E_OK            --  Operation is successful.
 *      AIR_E_BAD_PARAMETER --  Bad parameter.
 * NOTES:
 *
 */
AIR_ERROR_NO_T
hal_coral_ifmon_getMonitorState(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   hal_coral_ifmon_lookup
 * PURPOSE:
 *      To look up a callback function is registered.
 * INPUT:
 *      unit                  --  Device unit number
 *      notify_func           --  Callback function
 *      ptr_cookie            --  Cookie data of callback function
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK              --  Operation is successful.
 *      AIR_E_ENTRY_NOT_FOUND --  Entry not found.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ifmon_lookup(
    const UI32_T                  unit,
    const AIR_IFMON_NOTIFY_FUNC_T notify_func,
    void                         *ptr_cookie);

/* FUNCTION NAME: hal_coral_ifmon_getPortStatus
 * PURPOSE:
 *      Get the control block link status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_ps          --  AIR_PORT_STATUS_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_coral_ifmon_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps);

#endif /* #ifndef HAL_CORAL_IFMON_H */
