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

/* FILE NAME:  sfp_module_handle.c
 * PURPOSE:
 *      1. Process the events of SFP module plugging in and pulling out.
 *      2. Check if the SFP module just plugged in is on the whitelist or not.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "air_port.h"
#include "sfp_config_customer.h"
#include "sfp_module_handle.h"
#include "sfp_util.h"
#include "sfp_sff_data.h"
#include "sfp_task.h"
#include "sfp_db.h"
#include "syncd_api.h"
#include "syncd_api_port.h"
#include "sfp_module_inside_phy.h"
#include "sfp_pin.h"
#include "sfp_trunk.h"
#ifdef AIR_SUPPORT_SFP_DDM
#include "sfp_ddm.h"
#endif
#include "sfp_port.h"
#include "port_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define SFP_MODULE_WHITELIST_INVALID_INDEX    (0xFFFFFFFF)
#define SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_COUNT (1)
#define SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_MS (SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_COUNT * SFP_TASK_TIMER_BASE_INTERVAL_MS)
#define SFP_MODULE_SINGLING_RATE_1000 (1000)
#define SFP_MODULE_SINGLING_RATE_1G (1300)
#define SFP_MODULE_SINGLING_RATE_2_5G (3100)

/* MACRO FUNCTION DECLARATIONS
 */
#define SFP_MODULE_MESSAGE_BASIC_MEMBERS \
    SFP_TASK_PORT_INFO_T *ptr_port_info;

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    SFP_MODULE_MESSAGE_BASIC_MEMBERS
} SFP_MODULE_MESSAGE_BASIC_T;

typedef struct
{
    SFP_MODULE_MESSAGE_BASIC_MEMBERS
    UI8_T pin_state;
} SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T;

typedef SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T SFP_MODULE_MESSAGE_RXLOSS_PIN_STATE_CHANGE_T;

typedef struct
{
    SFP_MODULE_MESSAGE_BASIC_MEMBERS
    SFP_SFF_MEDIA_TYPE_T media_type;
    UI32_T signaling_rate;
} SFP_MODULE_MESSAGE_INFO_FOR_SERDES_MODE_OBTAINED_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
#ifdef SFP_MODULE_WHITELIST_ENABLE
static BOOL_T
_sfp_module_whitelist_compare(
    C8_T *ptr_info_supported,
    C8_T *ptr_info_read,
    UI32_T info_max_size);

static I32_T
_sfp_module_whitelist_find(
    C8_T *ptr_vendor_name,
    C8_T *ptr_vendor_oui,
    C8_T *ptr_vendor_pn,
    C8_T *ptr_vendor_rev);

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static BOOL_T
_sfp_module_whitelist_compare(
    C8_T *ptr_info_supported,
    C8_T *ptr_info_read,
    UI32_T info_max_size)
{
    I32_T length = 0, j = 0;

    if ((NULL == ptr_info_supported) || (NULL == ptr_info_read) || (0 == info_max_size))
    {
        return FALSE;
    }

    length = strlen(ptr_info_supported);

    if (info_max_size < length)
    {
        /* Invalid settings on the whitelit. It's too long. */
        SFP_LOG_ERROR("Invalid settings on the whitelist. len:%d max_size:%d", length, info_max_size);
        return FALSE;
    }

    if (0 != strncmp(ptr_info_supported, ptr_info_read, length))
    {
        return FALSE;
    }

    /* Check if info read is padding with 0x20. */
    for (; j < info_max_size - length; j++)
    {
        if (ptr_info_read[length + j] != 0x20)
        {
            SFP_LOG_ERROR("info read is not padding with 0x20.\n");
            return FALSE;
        }
    }

    return TRUE;
}


static I32_T
_sfp_module_whitelist_find(
    C8_T *ptr_vendor_name,
    C8_T *ptr_vendor_oui,
    C8_T *ptr_vendor_pn,
    C8_T *ptr_vendor_rev)
{
    I32_T i = 0, total_num = sfp_config_getWhitelistSettingsArraySize();
    BOOL_T ret = FALSE;
    const SFP_CONFIG_WHITELIST_SETTINGS_T *ptr_whitelist_info_array = sfp_config_getWhitelistSettingsArray();

    for (; i < total_num; i++)
    {
        if (ptr_whitelist_info_array[i].compare_mask != 0)
        {
            /* Compare vendor Name */
            if ((SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME & ptr_whitelist_info_array[i].compare_mask) != 0)
            {
                ret = _sfp_module_whitelist_compare((C8_T *)ptr_whitelist_info_array[i].vendor_name,
                                                    ptr_vendor_name,
                                                    SFP_MODULE_VENDOR_NAME_SIZE);
                if (FALSE == ret)
                {
                    continue;
                }
            }

            /* Compare vendor OUI */
            if ((SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI & ptr_whitelist_info_array[i].compare_mask) != 0)
            {
                if (NULL == ptr_vendor_oui)
                {
                    continue;
                }

                if (0 != memcmp(ptr_whitelist_info_array[i].vendor_oui, ptr_vendor_oui, SFP_MODULE_VENDOR_OUI_SIZE))
                {
                    continue;
                }
            }

            /* Compare vendor PN */
            if ((SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN & ptr_whitelist_info_array[i].compare_mask) != 0)
            {
                ret = _sfp_module_whitelist_compare((C8_T *)ptr_whitelist_info_array[i].vendor_pn,
                                                ptr_vendor_pn,
                                                SFP_MODULE_VENDOR_PN_SIZE);
                if (FALSE == ret)
                {
                    continue;
                }
            }

            /* Compare vendor Rev */
            if ((SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV & ptr_whitelist_info_array[i].compare_mask) != 0)
            {
                ret = _sfp_module_whitelist_compare((C8_T *)ptr_whitelist_info_array[i].vendor_rev,
                                                ptr_vendor_rev,
                                                SFP_MODULE_VENDOR_REV_SIZE);
                if (FALSE == ret)
                {
                    continue;
                }
            }

            /* Found */
            return i;
        }
    }

    return SFP_MODULE_WHITELIST_INVALID_INDEX;
}

static UI8_T
_sfp_module_whitelist_getPhy2wireAddr(
    UI32_T index)
{
    const SFP_CONFIG_WHITELIST_SETTINGS_T *ptr_whitelist_info_array = sfp_config_getWhitelistSettingsArray();

    if (index < sfp_config_getWhitelistSettingsArraySize())
    {
        return ptr_whitelist_info_array[index].phy_2wire_addr;
    }

    return 0;
}

static UI8_T
_sfp_module_whitelist_getSerdesMode(
    UI32_T index)
{
    const SFP_CONFIG_WHITELIST_SETTINGS_T *ptr_whitelist_info_array = sfp_config_getWhitelistSettingsArray();

    if (index < sfp_config_getWhitelistSettingsArraySize())
    {
        return ptr_whitelist_info_array[index].serdes_mode;
    }

    return 0;
}

/* FUNCTION NAME:   _sfp_module_whitelist_check
 * PURPOSE:
 *      Check if the new SFP module just inserted is on the whitelist or not.
 * INPUT:
 *      ptr_sfp                 -- SFP port context
 * OUTPUT:
 *      ptr_index        -- Output if the new SFP module is on the whitelist or not.
 *                                 TRUE: on the whitelist
 *                                 FALSE: not on the whitelist
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_module_whitelist_check(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    UI32_T *ptr_index)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T vendor_name[SFP_MODULE_VENDOR_NAME_SIZE + 1] = {0};
    UI8_T vendor_oui[SFP_MODULE_VENDOR_OUI_SIZE+1] = {0};
    UI8_T vendor_pn[SFP_MODULE_VENDOR_PN_SIZE + 1] = {0};
    UI8_T vendor_rev[SFP_MODULE_VENDOR_REV_SIZE + 1] = {0};

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Read vendor_name, vendor_oui, vendor_pn, vendor_rev */
    ret = sfp_sff_getVendorName(unit, ptr_port_info->port, vendor_name, SFP_MODULE_VENDOR_NAME_SIZE);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    ret = sfp_sff_getVendorOui(unit, ptr_port_info->port, vendor_oui, SFP_MODULE_VENDOR_OUI_SIZE);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    ret = sfp_sff_getVendorPN(unit, ptr_port_info->port, vendor_pn, SFP_MODULE_VENDOR_PN_SIZE);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    ret = sfp_sff_getVendorRev(unit, ptr_port_info->port, vendor_rev, SFP_MODULE_VENDOR_REV_SIZE);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    *ptr_index = _sfp_module_whitelist_find((C8_T *)vendor_name, (C8_T *)vendor_oui, (C8_T *)vendor_pn, (C8_T *)vendor_rev);
    return AIR_E_OK;
}
#endif

MW_ERROR_NO_T
_sfp_module_processRXLOSPin(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    const I32_T rxlos_pin_value,
    const I32_T abs_pin_value)
{
    SFP_MSG_T *ptr_message = NULL;
    I32_T ret = AIR_E_OK;

    if (ptr_port_info->rx_loss_pin_state != rxlos_pin_value)
    {
        if (SFP_MODULE_PORT_STATE_ACTIVE == ptr_port_info->state)
        {
            if (SFP_MODULE_PIN_HIGH == rxlos_pin_value)
            {
                ptr_port_info->rx_loss_delay_count++;
                if (ptr_port_info->rx_loss_delay_count <= SFP_TASK_RX_LOSS_DELAY_COUNT)
                {
                    SFP_LOG_DEBUG("RX_LOS pin(HIGH) delay count:%d is not reach max", ptr_port_info->rx_loss_delay_count);
                    return ret;
                }
            }
            else
            {
                ptr_port_info->rx_loss_delay_count = 0;
            }


            SFP_MODULE_MESSAGE_RXLOSS_PIN_STATE_CHANGE_T *ptr_data = NULL;

            SFP_LOG_DEBUG("rx_loss_pin state changes. old:%d new:%d", ptr_port_info->rx_loss_pin_state, rxlos_pin_value);
            ptr_port_info->rx_loss_pin_state = rxlos_pin_value;

            /* Send the RX_LOS(LOW) msg when (COMBO) SFP port is in the ACTIVE state.
             * Send the RX_LOS(HIGH) msg when (COMBO) SFP port is in the ACTIVE state and before ABS PIN(HIGH) msg is handled.
             */
            ptr_message = sfp_msg_create(unit,
                                         MW_MSG_ID_SFP_SFP_RXLOS_PIN_STATE_CHANGED_NOTI,
                                         sizeof(SFP_MODULE_MESSAGE_RXLOSS_PIN_STATE_CHANGE_T));
            if (NULL != ptr_message)
            {
                ptr_data = (SFP_MODULE_MESSAGE_RXLOSS_PIN_STATE_CHANGE_T *)ptr_message->ptr_data;
                ptr_data->ptr_port_info = ptr_port_info;
                ptr_data->pin_state = ptr_port_info->rx_loss_pin_state;

                if ((SFP_MODULE_PIN_HIGH == rxlos_pin_value) && (SFP_MODULE_PIN_HIGH == abs_pin_value))
                {
                    /* ABS pin(HIGH) will be processed immediately instead of sending a message
                     * to SFP task and waiting for the message handling.
                     * RX_LOS pin(HIGH) should be processed before ABS pin(HIGH) is processed.
                     * Therefore, it need be processed immediately also.
                     */
                    sfp_module_handleMessage(ptr_message);

                    MW_FREE(ptr_message);
                }
                else
                {
                    ret = sfp_msg_send(ptr_message);
                    if (MW_E_OK != ret)
                    {
                        MW_FREE(ptr_message);
                    }
                }
            }
        }
        else if (SFP_MODULE_PIN_HIGH == rxlos_pin_value)
        {
            SFP_LOG_DEBUG("rx_loss_pin state changes. old:%d new:%d", ptr_port_info->rx_loss_pin_state, rxlos_pin_value);
            ptr_port_info->rx_loss_pin_state = rxlos_pin_value;
            ptr_port_info->rx_loss_delay_count = 0;
        }
    }
    return ret;
}

MW_ERROR_NO_T
_sfp_module_processABSPin(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    const I32_T abs_pin_value)
{
    I32_T ret = AIR_E_OK;

    if (SFP_MODULE_PIN_LOW == abs_pin_value)
    {
        /* SFP module is inserted. */
        if ((SFP_MODULE_PORT_STATE_ABSENT == ptr_port_info->state) &&
            (SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_COUNT >= ptr_port_info->delay_count))
        {
            ptr_port_info->delay_count++;

            if (SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_COUNT < ptr_port_info->delay_count)
            {
                /* Delay to access SFP module just inserted to wait for it to be ready to access. If access the SFP module just after
                 * it is inserted, the garbled characters for SFF read may be returned.
                 */
                SFP_LOG_DEBUG("Delay %dms to notify SFP module is inserted", SFP_MODULE_SEND_ABSPINOBTAINED_MESSAGE_DELAY_MS);
            }
            else
            {
                return MW_E_OK;
            }
        }
    }

    if ((ptr_port_info->abs_pin_state != abs_pin_value) ||
        ((SFP_MODULE_PORT_STATE_ABSENT == ptr_port_info->state) &&
         (SFP_MODULE_PIN_LOW == abs_pin_value) &&
         (SFP_MODULE_ERROR_COUNT_LIMITATION > ptr_port_info->err_count)))
    {
        SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T *ptr_data = NULL;
        SFP_MSG_T *ptr_message = NULL;

        if (ptr_port_info->abs_pin_state != abs_pin_value)
        {
            SFP_LOG_DEBUG("Port:%d abs_pin state changes. old:%d new:%d", ptr_port_info->port, ptr_port_info->abs_pin_state, abs_pin_value);
            ptr_port_info->abs_pin_state = abs_pin_value;
        }

        ptr_message = sfp_msg_create(unit,
                                              MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI,
                                              sizeof(SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T));
        if (NULL != ptr_message)
        {
            ptr_data = (SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T *)ptr_message->ptr_data;
            ptr_data->ptr_port_info = ptr_port_info;
            ptr_data->pin_state = ptr_port_info->abs_pin_state;

            if (SFP_MODULE_PIN_HIGH == abs_pin_value)
            {
                /* SFP module is removed. It should be processed immeidately. */
                sfp_module_handleMessage(ptr_message);

                MW_FREE(ptr_message);
            }
            else
            {
                /* Process SFP module inserted event in another message to handle the removed event of other port(s) ASAP. */
                ret = sfp_msg_send(ptr_message);
                if (MW_E_OK != ret)
                {
                    MW_FREE(ptr_message);
                }
            }
        }
    }

    return ret;
}


static void
_sfp_module_pollingSFPModuleInfoForSerdesMode(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    SFP_SFF_MEDIA_TYPE_T media_type = sfp_sff_getMediaType(unit, ptr_port_info->port);
    SFP_MSG_T *ptr_message = NULL;
    SFP_MODULE_MESSAGE_INFO_FOR_SERDES_MODE_OBTAINED_T *ptr_data = NULL;
    UI32_T signaling_rate = 0;

    sfp_sff_getSignalingRate(unit, ptr_port_info->port, &signaling_rate);
    ptr_message = sfp_msg_create(unit,
                                          MW_MSG_ID_SFP_SFP_INFO_FOR_SERDES_MODE_OBTAINED_NOTI,
                                          sizeof(SFP_MODULE_MESSAGE_INFO_FOR_SERDES_MODE_OBTAINED_T));
    if (NULL != ptr_message)
    {
        MW_ERROR_NO_T ret;

        SFP_LOG_DEBUG("port:%d media_type:%d rate:%d", ptr_port_info->port, media_type, signaling_rate);
        ptr_data = (SFP_MODULE_MESSAGE_INFO_FOR_SERDES_MODE_OBTAINED_T *)ptr_message->ptr_data;
        ptr_data->ptr_port_info = ptr_port_info;
        ptr_data->media_type = media_type;
        ptr_data->signaling_rate = signaling_rate;
        ret = sfp_msg_send(ptr_message);
        if (MW_E_OK != ret)
        {
            MW_FREE(ptr_message);
        }
    }
}

static void
_sfp_module_state_setState(
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MODULE_PORT_STATE_T state)
{
    SFP_LOG_DEBUG("Change state for port:%d from %d to %d", ptr_port_info->port, ptr_port_info->state, state);
    ptr_port_info->state = state;
}

/* SFP module is removed. */
static AIR_ERROR_NO_T
_sfp_module_state_commonHandleABSPinStateHigh(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T tx_disable_pin = 0;
    AIR_PORT_COMBO_MODE_T new_combo_mode = AIR_PORT_COMBO_MODE_PHY;
    AIR_PORT_COMBO_MODE_T trunk_combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_SERDES_MODE_T new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
    AIR_PORT_SERDES_MODE_T trunk_serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* Reset the port */
    _sfp_module_state_setState(ptr_port_info, SFP_MODULE_PORT_STATE_ABSENT);
    ptr_port_info->delay_count = 0;
    ptr_port_info->err_count = 0;
    ptr_port_info->phy_2wire_addr = SFP_SFF_INVALID_ADDRESS;
    ptr_port_info->rx_loss_pin_state = SFP_MODULE_PIN_HIGH;
    ptr_port_info->abs_pin_state = SFP_MODULE_PIN_HIGH;
    sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_TX_DISABLE, &tx_disable_pin);
    sfp_pin_io_setValue(unit, ptr_port_info->port, tx_disable_pin, SFP_MODULE_PIN_HIGH);

    /* Change combo/serdes mode if needed */
    if (FALSE == sfp_trunk_determineComboSerdesMode(unit, ptr_port_info->port))
    {
        /* Port is in a trunk.
         * Neither COMBO mode nor SERDES mode is not allowed to be changed when trunk combo_mode and/or serdes_mode is set.
         */
        sfp_trunk_getComboSerdes(unit, ptr_port_info->port, &trunk_combo_mode, &trunk_serdes_mode);
        if (((TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port)) &&
              (trunk_combo_mode != new_combo_mode)) ||
            (trunk_serdes_mode != new_serdes_mode))
        {
            SFP_LOG_WARN("The port:%d is in a Trunk with combo_mode:%d serdes_mode:%d. It cannot be changed to combo mode:%d serdes_mode:%d!",
                      ptr_port_info->port, trunk_combo_mode, trunk_serdes_mode, new_combo_mode, new_serdes_mode);
            return AIR_E_NOT_SUPPORT;
        }

        /* Do not affect a COMBO PHY port */
        if ((TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port)) ||
            (TRUE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)))
        {
            /* COMBO SFP port or SFP port */
            sfp_clear_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
        }
    }
    else
    {
        if ((TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port)) &&
            (FALSE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)))
        {
            /* It has changed to a COMBO PHY port already. */
            SFP_LOG_WARN("port:%d has been a COMBO PHY port already", ptr_port_info->port);
            return ret;
        }

        sfp_phy_compensateLinkDown(unit, ptr_port_info->port);

        if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
        {
            BOOL_T admin_state = FALSE;

            air_port_getAdminState(unit, ptr_port_info->port, &admin_state);
            ret = sfp_port_setComboMode(unit, ptr_port_info->port, admin_state, FALSE, new_combo_mode);
        }
        ret |= air_port_setSerdesMode(unit, ptr_port_info->port, new_serdes_mode);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("port:%d Failed to set Combo mode:%d or/and serdes mode:%d. ret:%d",
                      ptr_port_info->port, new_combo_mode, new_serdes_mode, ret);
        }

        sfp_db_updatePortMode(unit, ptr_port_info, new_combo_mode, new_serdes_mode);

        if ((TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port)) &&
            (FALSE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)))
        {
            /* COMBO PHY port */
            sfp_module_state_initPortBasedOnDB(ptr_port_info);
            /* Even if error occurs, still set port inited. */
            sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
        }
        else
        {
            /* COMBO SFP port or SFP port */
            sfp_clear_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
        }
    }

    return ret;
}

static AIR_ERROR_NO_T
_sfp_module_state_absentHandleABSPinStateObtained(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T *ptr_data = ptr_message->ptr_data;
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T tx_disable_pin = SFP_PIN_PIONUM_INVALID, abs_pin = 0;
    I32_T data = 0;

    ret = sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_ABS, &abs_pin);
    ret |= sfp_pin_io_getValue(unit, ptr_port_info->port, abs_pin, &data);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    /* SFP module may have be removed when processing this message. */
    if ((SFP_MODULE_PIN_LOW == ptr_data->pin_state) && (SFP_MODULE_PIN_LOW == data))
    {
        /* SFP module is detected. */
        if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
        {
            AIR_PORT_COMBO_MODE_T new_combo_mode = AIR_PORT_COMBO_MODE_SERDES;
            AIR_PORT_COMBO_MODE_T trunk_combo_mode = AIR_PORT_COMBO_MODE_LAST;

            if (FALSE == sfp_trunk_determineComboSerdesMode(unit, ptr_port_info->port))
            {
                /* Port is in a trunk.
                 * COMBO mode is not allowed to be changed when trunk combo_mode is set.
                 */
                sfp_trunk_getComboSerdes(unit, ptr_port_info->port, &trunk_combo_mode, NULL);
                if (trunk_combo_mode != new_combo_mode)
                {
                    _sfp_module_state_setState(ptr_port_info, SFP_MODULE_PORT_STATE_PENDING);
                    SFP_LOG_WARN("The port:%d is in a Trunk with combo mode:%d. It cannot be changed to combo mode:%d!",
                              ptr_port_info->port, trunk_combo_mode, new_combo_mode);
                    return AIR_E_NOT_SUPPORT;
                }
            }
            else
            {
                BOOL_T admin_state = FALSE;

                sfp_phy_compensateLinkDown(unit, ptr_port_info->port);
                air_port_getAdminState(unit, ptr_port_info->port, &admin_state);
                ret = sfp_port_setComboMode(unit, ptr_port_info->port, admin_state, FALSE, new_combo_mode);
            }

            if (AIR_E_OK != ret)
            {
                SFP_LOG_ERROR("Port:%d failed to set COMBO mode to SERDES.", ptr_port_info->port);
            }
            else
            {
                sfp_clear_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
            }
        }

        if (AIR_E_OK == ret)
        {
            sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_TX_DISABLE, &tx_disable_pin);
            if (SFP_PIN_PIONUM_INVALID != tx_disable_pin)
            {
                /* TX_DISABLE pin exists. */
                ret = sfp_pin_io_setValue(unit, ptr_port_info->port, tx_disable_pin, SFP_MODULE_PIN_LOW);
                if (AIR_E_OK != ret)
                {
                    SFP_LOG_ERROR("Port:%d failed to get/set tx-disable pin:%d to low. ret:%d", ptr_port_info->port, tx_disable_pin, ret);
                }
            }
        }

        if (AIR_E_OK == ret)
        {
            ptr_port_info->err_count = 0;
            ptr_port_info->delay_count = 0;
            _sfp_module_state_setState(ptr_port_info, SFP_MODULE_PORT_STATE_PRESENT);
            _sfp_module_pollingSFPModuleInfoForSerdesMode(unit, ptr_port_info);
        }
        else
        {
            ptr_port_info->err_count++;
            if (SFP_MODULE_ERROR_COUNT_LIMITATION <= ptr_port_info->err_count)
            {
                SFP_LOG_ERROR("Failed to process ABS pin state at ABSENT state for %d times.", ptr_port_info->err_count);
            }
        }
    }
    else
    {
        _sfp_module_state_commonHandleABSPinStateHigh(unit, ptr_port_info);
        SFP_LOG_WARN("ABSENT state with ABS PIN state changing to HIGH.");
    }

    return ret;
}

static AIR_ERROR_NO_T
_sfp_module_state_commonHandleABSPinStateObtained(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    SFP_MODULE_MESSAGE_ABSPIN_STATE_OBTAINED_T *ptr_data = ptr_message->ptr_data;
    AIR_ERROR_NO_T ret = AIR_E_OK;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (SFP_MODULE_PIN_HIGH == ptr_data->pin_state)
    {
        ret = _sfp_module_state_commonHandleABSPinStateHigh(unit, ptr_port_info);
    }
    else
    {
        ret = AIR_E_OP_INVALID;
        SFP_LOG_WARN("State:%d with ABS PIN state changing to LOW.", ptr_port_info->state);
    }

    return ret;
}

static AIR_ERROR_NO_T
_sfp_module_state_presentHandleInfoForSerdesModeObtained(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    I32_T data = 0;
    UI8_T pin = 0;
    SFP_MODULE_MESSAGE_INFO_FOR_SERDES_MODE_OBTAINED_T *ptr_data = ptr_message->ptr_data;
    AIR_PORT_SERDES_MODE_T new_serdes_mode = AIR_PORT_SERDES_MODE_LAST, old_serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    UI32_T index = SFP_MODULE_WHITELIST_INVALID_INDEX;
#ifdef SFP_MODULE_WHITELIST_RESTRICTION_ENABLE
    BOOL_T on_whitelist = FALSE;
#endif

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    ret = air_port_getSerdesMode(unit, ptr_port_info->port, &old_serdes_mode);
    ret |= sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_ABS, &pin);
    ret |= sfp_pin_io_getValue(unit, ptr_port_info->port, pin, &data);
    if ((AIR_E_OK != ret) || (SFP_MODULE_PIN_HIGH == data))
    {
        /* SFP module may have be removed when processing this message. */
        return ret;
    }

    if ((SFP_SFF_MEDIA_TYPE_LAST == ptr_data->media_type) ||
        (0 == ptr_data->signaling_rate))
    {
        /* Fail to get all information. */
        ptr_port_info->err_count++;
        if (SFP_MODULE_ERROR_COUNT_LIMITATION <= ptr_port_info->err_count)
        {
            SFP_LOG_WARN("Fail to get info for serdes mode. port:%d media_type:%d rate:%d",
                      ptr_port_info->port, ptr_data->media_type, ptr_data->signaling_rate);
        }
        else
        {
            /* Retry */
            return AIR_E_OK;
        }
    }
    else
    {
        /* Determine serdes mode */
        if (SFP_MODULE_SINGLING_RATE_1000 > ptr_data->signaling_rate)
        {
            if (TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port))
            {
                /* EN8804 does not support 100BASE_FX */
                new_serdes_mode = AIR_PORT_SERDES_MODE_100BASE_FX;
            }
        }
        else if ((SFP_MODULE_SINGLING_RATE_1000 <= ptr_data->signaling_rate) &&
                 (SFP_MODULE_SINGLING_RATE_1G >= ptr_data->signaling_rate))
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
            if (SFP_SFF_MEDIA_TYPE_COPPER == ptr_data->media_type)
            {
                new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
            }
        }
        else if ((SFP_MODULE_SINGLING_RATE_1G < ptr_data->signaling_rate) &&
                 (SFP_MODULE_SINGLING_RATE_2_5G >= ptr_data->signaling_rate))
        {
            if (TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port))
            {
                /* EN8804 does not support HSGMII */
                new_serdes_mode = AIR_PORT_SERDES_MODE_HSGMII;
            }
        }

#ifdef AIR_LITE_MW
        /* 8855 not support 100base-fx */
        if (AIR_PORT_SERDES_MODE_100BASE_FX == new_serdes_mode)
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
        }
#endif

        if (new_serdes_mode != AIR_PORT_SERDES_MODE_LAST)
        {
#ifdef SFP_MODULE_WHITELIST_RESTRICTION_ENABLE
            /* Check if it is on the whitelist */
            ret = _sfp_module_whitelist_check(unit, ptr_port_info, &index);
            if ((AIR_E_OK == ret) && (index < sfp_config_getWhitelistSettingsArraySize()))
            {
                on_whitelist = TRUE;
                ptr_port_info->phy_2wire_addr = _sfp_module_whitelist_getPhy2wireAddr(index);
                SFP_LOG_DEBUG("SFP module of port:%d is on the whitelist.", ptr_port_info->port);
            }
            else
#endif
            {
                /* It is not on the whitelist. */
                ptr_port_info->phy_2wire_addr = SFP_SFF_INVALID_ADDRESS;
                SFP_LOG_DEBUG("SFP module of port:%d is not on the whitelist.", ptr_port_info->port);
            }

            if ((AIR_PORT_SERDES_MODE_SGMII == new_serdes_mode) ||
                (AIR_PORT_SERDES_MODE_HSGMII == new_serdes_mode))
            {
#ifdef SFP_MODULE_WHITELIST_RESTRICTION_ENABLE
                if (FALSE == on_whitelist)
#endif
                {
                    /* Check if its inside PHY can be accessed by the default address */
                    BOOL_T admin_state = FALSE;

                    ptr_port_info->phy_2wire_addr = SFP_SFF_DEFAULT_ADDRESS;
                    ret = sfp_phy_getAdminState(unit, ptr_port_info->port, &admin_state);
                    if (AIR_E_OK != ret)
                    {
                        /* SFP module cannot be accessed via 2 wire bi-directional serial interface at address SFP_SFF_DEFAULT_ADDRESS. */
                        ptr_port_info->phy_2wire_addr = SFP_SFF_INVALID_ADDRESS;
                        SFP_LOG_DEBUG("Inside PHY of port:%d cannot be accessed at address 0x:%x", ptr_port_info->port, SFP_SFF_DEFAULT_ADDRESS);
                    }
                }

                if ((AIR_PORT_SERDES_MODE_SGMII == new_serdes_mode) &&
                    (SFP_SFF_INVALID_ADDRESS == ptr_port_info->phy_2wire_addr))
                {
                    /* No inside PHY. */
                    new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
                    SFP_LOG_DEBUG("port:%d Change SGMII to 1000base-x because its inside PHY cannot be accessed", ptr_port_info->port);
                }
#ifdef SFP_MODULE_WHITELIST_ENABLE
                /* Check serdes mode if it is on the whitelist */
                ret = _sfp_module_whitelist_check(unit, ptr_port_info, &index);
                if ((AIR_E_OK == ret) && (index < sfp_config_getWhitelistSettingsArraySize()))
                {
                    if (SFP_SERDES_MODE_FORCE_1000BASE_X == _sfp_module_whitelist_getSerdesMode(index))
                    {
                        new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
                        ptr_port_info->phy_2wire_addr = _sfp_module_whitelist_getPhy2wireAddr(index);
                        SFP_LOG_DEBUG("SFP module of port:%d required serdes mode is %d.",
                            ptr_port_info->port, new_serdes_mode);
                    }
                }
#endif
            }
        }
    }

    if (FALSE == sfp_trunk_determineComboSerdesMode(unit, ptr_port_info->port))
    {
        AIR_PORT_SERDES_MODE_T trunk_serdes_mode = AIR_PORT_SERDES_MODE_LAST;

        /* Port is in a trunk.
         * SERDES mode is not allowed to be changed when trunk serdes_mode is set.
         */
        if (AIR_PORT_SERDES_MODE_LAST == new_serdes_mode)
        {
            /* An unknown SFP module should not be in a trunk. */
            return AIR_E_NOT_SUPPORT;
        }

        sfp_trunk_getComboSerdes(unit, ptr_port_info->port, NULL, &trunk_serdes_mode);
        /* trunk_serdes_mode should be equal to old_serdes_mode. */
        if (trunk_serdes_mode != new_serdes_mode)
        {
            /* When the port is in a Trunk, its serdes mode cannot be changed. */
            ptr_port_info->phy_2wire_addr = SFP_SFF_INVALID_ADDRESS;
            ptr_port_info->err_count = 0;
            _sfp_module_state_setState(ptr_port_info, SFP_MODULE_PORT_STATE_PENDING);
            SFP_LOG_WARN("The port:%d is in a Trunk with serdes_mode:%d. It cannot be changed to serdes_mode:%d!",
                      ptr_port_info->port, trunk_serdes_mode, new_serdes_mode);
            return AIR_E_NOT_SUPPORT;
        }
    }

    /* Disable MAC:
     * 1. SGMII with accessible inside PHY:
     *    1.1 sfp_port_setX() APIs will only set the PHY side.
     *    1.2 MAC is enabled when it is link up at the PHY side. For more details, refer to _sfp_phy_linkStatusChange_setMAC().
     * 2. Others:
     *    2.1 MAC is enabled when handling RX_LOS(LOW) msg. For more details, refer to _sfp_module_state_activeHandleForRxLossPinStateObtained().
     */
    air_port_setAdminState(unit, ptr_port_info->port, FALSE);
    sfp_phy_compensateLinkDown(unit, ptr_port_info->port);

    if (old_serdes_mode != new_serdes_mode)
    {
        if (AIR_PORT_SERDES_MODE_LAST == new_serdes_mode)
        {
            if (AIR_PORT_SERDES_MODE_1000BASE_X != old_serdes_mode)
            {
                /* Set serdes mode to 1000BASE_X for a totally unsupported SFP module. */
                air_port_setSerdesMode(unit, ptr_port_info->port, AIR_PORT_SERDES_MODE_1000BASE_X);
            }
        }
        else
        {
            if  (((TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port)) ||
                   (TRUE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port))) &&
                 ((AIR_PORT_SERDES_MODE_100BASE_FX == new_serdes_mode) ||
                   (AIR_PORT_SERDES_MODE_HSGMII == new_serdes_mode)))
            {
                /* Set Force mode before change to 100BASE_FX/HSGMII. */
                air_port_setPhyAutoNego(unit, ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE);
            }
            air_port_setSerdesMode(unit, ptr_port_info->port, new_serdes_mode);
        }
    }

    SFP_LOG_DEBUG("port:%d Change serdes mode from %d to %d", ptr_port_info->port, old_serdes_mode, new_serdes_mode);

    /* Do not block a totally unsupported SFP module. */

    /* Try to make SFP module work even if it is not on the whitelist.
     * SFP module will not be blocked even if it is not on the whitelist. However, it will
     * be set to unknown in DB and it cannot be configured on webpage.
     */
    ptr_port_info->err_count = 1; /* Reuse it to indicate 1st link up after SFP module is inserted. */
    ptr_port_info->delay_count = 0;
    _sfp_module_state_setState(ptr_port_info, SFP_MODULE_PORT_STATE_ACTIVE);
    sfp_module_state_initPortBasedOnDB(ptr_port_info);
    /* Even if error occurs, still set port inited. */
    sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);

#ifdef AIR_SUPPORT_SFP_DDM
    sfp_ddm_polling(ptr_message->unit, ptr_port_info);
#endif  /* AIR_SUPPORT_SFP_DDM */

#ifdef SFP_MODULE_WHITELIST_RESTRICTION_ENABLE
    if (FALSE == on_whitelist)
    {
        /* A totally unsupported SFP module or an SFP module not on the whitelist. */
        new_serdes_mode = SFP_DB_PORT_SERDES_MODE_UNKNOWN;
    }
#endif
    /* Update the port mode after DB INIT settings are configured to avoid updating DB settings by user while configuring the DB INIT settings. */
    sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, new_serdes_mode);

    SFP_LOG_DEBUG("media_type:%d signaling_rate:%d serdes_mode:%d port:%d\n", ptr_data->media_type, ptr_data->signaling_rate, new_serdes_mode, ptr_port_info->port);
    return ret;
}

static AIR_ERROR_NO_T
_sfp_module_state_activeHandleForRxLossPinStateObtained(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    I32_T ret = AIR_E_OK;
    AIR_PORT_SERDES_MODE_T serdes_mode;
    SFP_MODULE_MESSAGE_RXLOSS_PIN_STATE_CHANGE_T *ptr_data = ptr_message->ptr_data;

    if (NULL == ptr_port_info)
    {
        ret = AIR_E_BAD_PARAMETER;
        return ret;
    }

    ret = air_port_getSerdesMode(unit, ptr_port_info->port, &serdes_mode);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Failed to get serdes mode. ret:%d", ret);
        return ret;
    }

    /* When state is ACTIVE, sfp_port_checkInsidePHYReady() is accurate. */
    ret = sfp_port_checkInsidePHYReady(unit, ptr_port_info->port);
    if (MW_E_OK != ret)
    {
        if (SFP_MODULE_PIN_HIGH == ptr_data->pin_state)
        {
#ifndef AIR_EN_CORAL
            ret = air_port_setAdminState(unit, ptr_port_info->port, FALSE);
            SFP_LOG_DEBUG("RX_LOS(HIGH) set port %d MAC admin state disable.\n", ptr_port_info->port);
#endif
        }
        else
        {
            DB_MSG_T *pptr_out_msg = NULL;
            UI16_T data_size = 0;
            UI8_T *ptr_data = NULL;

            ret = sfp_db_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, ptr_port_info->port, &pptr_out_msg, &data_size, (void **)&ptr_data);
            if (AIR_E_OK != ret)
            {
                SFP_LOG_ERROR("port:%d Fail to get PORT_ADMIN_STATUS.\n", ptr_port_info->port);
                return ret;
            }

            if (1 == ptr_data[0])
            {
                ret = air_port_setAdminState(unit, ptr_port_info->port, TRUE);
                SFP_LOG_DEBUG("RX_LOS(LOW) set port %d MAC admin state enable.\n", ptr_port_info->port);
            }
            MW_FREE(pptr_out_msg);
        }
    }

    return ret;
}

static void
_sfp_module_state_handleAbsent(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        {
            _sfp_module_state_absentHandleABSPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }
}

static void
_sfp_module_state_handleUnknown(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        {
            _sfp_module_state_commonHandleABSPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }
}

static void
_sfp_module_state_handleActive(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        {
            _sfp_module_state_commonHandleABSPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        case MW_MSG_ID_SFP_SFP_RXLOS_PIN_STATE_CHANGED_NOTI:
        {
            _sfp_module_state_activeHandleForRxLossPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }
}

static AIR_ERROR_NO_T
_sfp_module_state_handlePresent(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        {
            _sfp_module_state_commonHandleABSPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        case MW_MSG_ID_SFP_SFP_INFO_FOR_SERDES_MODE_OBTAINED_NOTI:
        {
            _sfp_module_state_presentHandleInfoForSerdesModeObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }

    return AIR_E_OK;
}

static void
_sfp_module_state_handlePending(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_message->msg_id)
    {
        case MW_MSG_ID_SFP_SFP_ABS_PIN_STATE_OBTAINED_NOTI:
        {
            _sfp_module_state_commonHandleABSPinStateObtained(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }
}

static void
_sfp_module_state_machine(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    SFP_MSG_T *ptr_message)
{
    switch (ptr_port_info->state)
    {
        case SFP_MODULE_PORT_STATE_ABSENT:
        {
            _sfp_module_state_handleAbsent(unit, ptr_port_info, ptr_message);
            break;
        }

        case SFP_MODULE_PORT_STATE_PRESENT:
        {
            _sfp_module_state_handlePresent(unit, ptr_port_info, ptr_message);
            break;
        }

        case SFP_MODULE_PORT_STATE_UNKNOWN:
        {
            _sfp_module_state_handleUnknown(unit, ptr_port_info, ptr_message);
            break;
        }

        case SFP_MODULE_PORT_STATE_ACTIVE:
        {
            _sfp_module_state_handleActive(unit, ptr_port_info, ptr_message);
            break;
        }

        case SFP_MODULE_PORT_STATE_PENDING:
        {
            _sfp_module_state_handlePending(unit, ptr_port_info, ptr_message);
            break;
        }

        default:
        {
            break;
        }
    }
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME:   sfp_module_handleMessage
 * PURPOSE:
 *      Handle the messages related to SFP module handling.
 *
 * INPUT:
 *      ptr_message          -- The message to be handled
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_handleMessage(
    SFP_MSG_T *ptr_message)
{
    SFP_MODULE_MESSAGE_BASIC_T *ptr_basic_message = (SFP_MODULE_MESSAGE_BASIC_T *)ptr_message->ptr_data;

    _sfp_module_state_machine(ptr_message->unit, ptr_basic_message->ptr_port_info, ptr_message);
}

/* FUNCTION NAME:   sfp_module_pollingSFPModuleStatus
 * PURPOSE:
 *      Poll the SFP module status for an serdes port or a combo serdes port to
 *      check if there is an SFP module just inserted or removed. The polling
 *      interval is SFP_TASK_TIMER_BASE_INTERVAL_MS.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer points to the port information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_pollingSFPModuleStatus(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI8_T pin = 0;
    I32_T abs_pin_value = 0, rxlos_pin_value = 0;

    if (NULL == ptr_port_info)
    {
        return;
    }

    /* The SFP SDA PIN driver and/or the SFP SDA IO driver are not initialized successfully. Adopt the SFP auto-adaption algorithm. */
    if (FALSE == sfp_pin_checkInitState(ptr_port_info->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        return;
    }

    ret = sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_RX_LOSS, &pin);
    ret |= sfp_pin_io_getValue(unit, ptr_port_info->port, pin, &rxlos_pin_value);
    ret |= sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_ABS, &pin);
    ret |= sfp_pin_io_getValue(unit, ptr_port_info->port, pin, &abs_pin_value);
    if (ptr_port_info->abs_pin_state != abs_pin_value)
    {
        SFP_LOG_DEBUG("Polling port:%d abs_pin:%d pin_state:%d-%d ret:%d", ptr_port_info->port, pin, ptr_port_info->abs_pin_state, abs_pin_value, ret);
    }

    if (AIR_E_OK == ret)
    {
        /* Check RX_LOS pin */
        _sfp_module_processRXLOSPin(unit, ptr_port_info, rxlos_pin_value, abs_pin_value);

        /* Check ABS pin */
        _sfp_module_processABSPin(unit, ptr_port_info, abs_pin_value);
    }

    if (SFP_MODULE_PORT_STATE_PRESENT == ptr_port_info->state)
    {
        _sfp_module_pollingSFPModuleInfoForSerdesMode( unit, ptr_port_info);
    }
}

/* FUNCTION NAME:   sfp_module_state_getState
 * PURPOSE:
 *      Get the SFP module state for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the SFP module state
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_module_state_getState(
    UI8_T port,
    SFP_MODULE_PORT_STATE_T *ptr_state)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        *ptr_state = ptr_port_info->state;
    }
    else
    {
        *ptr_state = SFP_MODULE_PORT_STATE_LAST;
    }
}

/* FUNCTION NAME:   sfp_module_state_initPortBasedOnDB
 * PURPOSE:
 *      Initialize the port based on the port settings in DB. Port need be
 *      initalized when its combo mode or serdes mode changes.
 *
 * INPUT:
 *      ptr_port_info        -- A pointer points to the port information
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_module_state_initPortBasedOnDB(
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    DB_MSG_T *pptr_out_msg = NULL;
    UI16_T data_size = 0, portSettings = 0;
    void *ptr_data = NULL;
    I32_T ret;
    SYNCD_API_ARG_T *ptr_api_arg = NULL;
    DB_REQUEST_TYPE_T *ptr_req_type = NULL;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    UI32_T unit = 0;
    UI8_T db_admin_state = 1;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    if ((TRUE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)) ||
        (TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port)))
    {
        ret = air_port_getSerdesMode(unit, ptr_port_info->port, &serdes_mode);
        if (AIR_E_OK != ret)
        {
            return ret;
        }
    }
    else
    {
        serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
    }

    osapi_calloc(sizeof(SYNCD_API_ARG_T), "SFP", (void **)&ptr_api_arg);
    osapi_calloc(sizeof(DB_REQUEST_TYPE_T), "SFP", (void **)&ptr_req_type);
    if ((NULL == ptr_api_arg) || (NULL == ptr_req_type))
    {
        osapi_free(ptr_api_arg);
        osapi_free(ptr_req_type);
        return AIR_E_NO_MEMORY;
    }

    ptr_api_arg->method = M_GET;
    ptr_api_arg->ptr_type = ptr_req_type;
    ptr_api_arg->ptr_type->t_idx = PORT_CFG_INFO;
    ptr_api_arg->ptr_type->e_idx = ptr_port_info->port;

    do {
        /* Set Admin status to FALSE first before setting. It need be set to TRUE after setting if admin status in DB is TRUE. */
        sfp_port_setAdminState(unit, ptr_port_info->port, FALSE);

        /* SYNCD will not be notified for this GET method. */
        ret = sfp_db_queue_getData(PORT_CFG_INFO, PORT_ADMIN_STATUS, ptr_port_info->port, &pptr_out_msg, &data_size, &ptr_data);
        if (AIR_E_OK != ret)
        {
            break;
        }
        db_admin_state = *(UI8_T *)ptr_data;
        MW_FREE(pptr_out_msg);

        ret = sfp_db_queue_getData(PORT_CFG_INFO, PORT_SETTINGS, ptr_port_info->port, &pptr_out_msg, &data_size, &ptr_data);
        if (AIR_E_OK != ret)
        {
            break;
        }
        osapi_memcpy(&portSettings, (UI8_T *)ptr_data, sizeof(UI16_T));
        port_correctPortSettings(unit, ptr_port_info->port, serdes_mode, &portSettings);
        osapi_memcpy((UI8_T *)ptr_data, &portSettings, sizeof(UI16_T));
        ptr_api_arg->data_size = data_size;
        ptr_api_arg->ptr_data = ptr_data;
        ptr_api_arg->ptr_type->f_idx = PORT_SETTINGS;
        ret = syncd_api_port_settings(ptr_api_arg);
        MW_FREE(pptr_out_msg);
        if (AIR_E_OK != ret)
        {
            break;
        }

        ret = sfp_db_queue_getData(PORT_CFG_INFO, PORT_EEE_ENABLE, ptr_port_info->port, &pptr_out_msg, &data_size, &ptr_data);
        if (AIR_E_OK != ret)
        {
            break;
        }
        ptr_api_arg->data_size = data_size;
        ptr_api_arg->ptr_data = ptr_data;
        ptr_api_arg->ptr_type->f_idx = PORT_EEE_ENABLE;

        ret = syncd_api_port_eeeConfig(ptr_api_arg);
        MW_FREE(pptr_out_msg);
        if (AIR_E_OK != ret)
        {
            break;
        }
    } while(0);

    if (1 == db_admin_state)
    {
        /* Set admin status to TRUE:
         * 1. admin status in DB is true.
         * 2. admin status in DB is not obtained successfully.
         */
        ret = sfp_port_setAdminState(unit, ptr_port_info->port, TRUE);
    }

    if (AIR_E_OK == ret)
    {
        sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ADMIN_STATUS);
    }

    osapi_free(ptr_api_arg);
    osapi_free(ptr_req_type);

    return ret;
}

/* FUNCTION NAME:   sfp_module_fakeModuleRemoved
 * PURPOSE:
 *      Fake that an SFP module is removed even if there is no SFP module inserted at all.
 *      It will reset the (COMBO) SFP port and detect the SFP module again if there is one.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Index of port number
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong
 *
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_module_fakeModuleRemoved(
    const UI32_T unit,
    const UI32_T port)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);
    I32_T rxlos_pin_value = SFP_MODULE_PIN_HIGH, abs_pin_value = SFP_MODULE_PIN_HIGH;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* RX_LOS High */
    _sfp_module_processRXLOSPin(unit, ptr_port_info, rxlos_pin_value, abs_pin_value);

    /* ABS High */
    if (SFP_MODULE_PIN_LOW != ptr_port_info->abs_pin_state)
    {
        /* Even if there is no SFP transceiver inserted, still need to process ABS_PIN(HIGH) to change to COMBO PHY mode. */
        SFP_LOG_WARN("Force port:%d ABS PIN from HIGH to LOW", ptr_port_info->port);
        ptr_port_info->abs_pin_state = SFP_MODULE_PIN_LOW;
    }
    _sfp_module_processABSPin(unit, ptr_port_info, abs_pin_value);

    return AIR_E_OK;
}

void
sfp_module_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port)
{
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = sfp_task_getPortInfo(port);

    if (NULL == ptr_taskPortInfo)
    {
        return;
    }

    if (TRUE != sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        /* Not SFP with I2C */
        return;
    }

    sfp_module_fakeModuleRemoved(unit, port);
}

