/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2023
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

/* FILE NAME:   sfp_task.h
 * PURPOSE:
 *      SFP task implementation.
 * NOTES:
 */

#ifndef SFP_TASK_H
#define SFP_TASK_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_utils.h"
#include "mw_msg.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_TASK_NAME "SFP"
#define SFP_TASK_PORT_INDEX_INVALID (0xFF)
#define SFP_TASK_TIMER_BASE_INTERVAL_MS (200)
#define SFP_TASK_RX_LOSS_DELAY_COUNT    (2)

#define SFP_TASK_MODULE_STATUS_INSERTED              BIT(0)
#define SFP_TASK_MODULE_STATUS_SWRXLOS_IMPLEMENTED   BIT(1)
#define SFP_TASK_MODULE_STATUS_SWRXLOS_HIGH          BIT(2)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    SFP_PORT_INITED_BITMASK_ADMIN_STATUS = 0x01,
    SFP_PORT_INITED_BITMASK_ADMIN_SPEED = 0x02,
    SFP_PORT_INITED_BITMASK_ADMIN_FLOW_CTRL = 0x04,
    SFP_PORT_INITED_BITMASK_ADMIN_SPD_ABILITY = 0x08,
    SFP_PORT_INITED_BITMASK_EEE_ENABLE = 0x10,

    SFP_PORT_INITED_BITMASK_ALL = (SFP_PORT_INITED_BITMASK_ADMIN_STATUS |
                                   SFP_PORT_INITED_BITMASK_ADMIN_SPEED |
                                   SFP_PORT_INITED_BITMASK_ADMIN_FLOW_CTRL |
                                   SFP_PORT_INITED_BITMASK_ADMIN_SPD_ABILITY |
                                   SFP_PORT_INITED_BITMASK_EEE_ENABLE)
} SFP_PORT_INITED_BITMASK_T;

typedef struct
{
    UI8_T port;    /* The port number */
    UI8_T pin_state;    /* The initialization status of the SDA pin and the IO pins. */
    UI8_T port_inited;    /* Init port based on PORT_CFG_INFO DB settings. */
    UI8_T trunk_member_status;
    UI8_T abs_pin_state;
    UI8_T rx_loss_pin_state;

    UI8_T state;    /* The module state for SFP with I2C. Please refer to SFP_MODULE_PORT_STATE_T. */
    UI8_T delay_count;
    UI8_T err_count;
    UI8_T phy_2wire_addr;
    UI8_T module_status;
    UI8_T rx_loss_delay_count;

    UI8_T auto_adaption_state;
    UI8_T auto_adaption_delay_count;
    UI8_T auto_adaption_retry_count;
    UI8_T auto_adaption_serdes_matched;
} SFP_TASK_PORT_INFO_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
#ifdef AIR_SUPPORT_SFP_WITH_THREAD
/* FUNCTION NAME:   sfp_task_create
 * PURPOSE:
 *      Create the SFP task.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_task_create(
    void);
#endif

/* FUNCTION NAME:   sfp_task_getPortIndex
 * PURPOSE:
 *      Get the index of a port in the array _sfp_task_ctx.port_info.
 *
 * INPUT:
 *      port                         -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      SFP_TASK_PORT_INDEX_INVALID  -- Failed
 *      Other value                  -- The index of the port
 *
 * NOTES:
 *      None.
 */
UI8_T
sfp_task_getPortIndex(
    UI8_T port);

/* FUNCTION NAME:   sfp_init
 * PURPOSE:
 *      Initialize SFP.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_queue_name           -- The name of the queue receives SFP messages
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK
 *      MW_E_NO_MEMORY
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      The message will be freed before the API returns.
 */
MW_ERROR_NO_T
sfp_init(
    const UI32_T unit,
    const C8_T * const ptr_queue_name);

/* FUNCTION NAME:   sfp_task_getPortInfo
 * PURPOSE:
 *      Get the port information of a port.
 *
 * INPUT:
 *      port                -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      Not NULL            -- A pointer points to the port information
 *      NULL                -- Failed
 *
 * NOTES:
 *      None.
 */
SFP_TASK_PORT_INFO_T *
sfp_task_getPortInfo(
    UI8_T port);

/* FUNCTION NAME:   sfp_handle_msg
 * PURPOSE:
 *      Handle the SFP messages.
 *
 * INPUT:
 *      ptr_message         -- The pointer points to the message to be handled.
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      The message will be freed before the API returns.
 */
void
sfp_handle_msg(
    MW_MSG_T *ptr_message);


/* FUNCTION NAME:   sfp_check_ready
 * PURPOSE:
 *      Check if SFP task is initialized successfully.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE               -- SFP task is initialized successfully.
 *      FALSE              -- SFP task has not been initialized successfully
 *
 * NOTES:
 *      None.
 */
BOOL_T
sfp_check_ready(
    void);

/* FUNCTION NAME:   sfp_set_port_inited
 * PURPOSE:
 *      Set flag(s) to indicate the port setting(s) which is(are) complete for
 *      a port.
 *
 * INPUT:
 *      port                 -- Port number
 *      add_bits             -- Flag(s) to be added
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NOT_SUPPORT    -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_set_port_inited(
    UI8_T port,
    SFP_PORT_INITED_BITMASK_T add_bits);

/* FUNCTION NAME:   sfp_clear_port_inited
 * PURPOSE:
 *      Clear flag(s) that is(are) used to indicate the port setting(s) which
 *      is(are) complete for a port.
 *
 * INPUT:
 *      port                 -- Port number
 *      delete_bits          -- Flag(s) to be cleared
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NOT_SUPPORT    -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
sfp_clear_port_inited(
    UI8_T port,
    SFP_PORT_INITED_BITMASK_T delete_bits);

/* FUNCTION NAME:   sfp_get_port_inited
 * PURPOSE:
 *      Check if all port settings are complete for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE                 -- All port settings are set.
 *      FALSE                -- Not all port settings are set.
 *
 * NOTES:
 *      None.
 */
BOOL_T
sfp_get_port_inited(
    UI8_T port);

#ifndef AIR_SUPPORT_SFP_WITH_THREAD
/* FUNCTION NAME:   sfp_handle_timerExpired
 * PURPOSE:
 *      Handle sfp base timer expired.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_handle_timerExpired(
    void);
#endif

#endif /* End of SFP_TASK_H */

