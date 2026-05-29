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

/* FILE NAME:  sfp_trunk.c
 * PURPOSE:
 *      Handle trunk for SFP ports.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "air_lag.h"
#include "sfp_trunk.h"
#include "sfp_db.h"
#include "sfp_util.h"
#include "sfp_pin.h"
#include "sfp_auto_adaptation.h"
#include "sfp_module_handle.h"
#include "sfp_sff_data.h"
#include "sfp_module_inside_phy.h"
#include "sfp_port.h"


/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T *ptr_combo_mode;
    UI8_T *ptr_serdes_mode;
    UI8_T trunk_count;
} SFP_TRUNK_CONTEX_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static SFP_TRUNK_CONTEX_T _sfp_trunk_cntx;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
static BOOL_T
_sfp_trunk_is_comboSerdesValid(
    UI32_T trunk_id)
{
    AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    if (trunk_id < _sfp_trunk_cntx.trunk_count)
    {
        combo_mode = _sfp_trunk_cntx.ptr_combo_mode[trunk_id];
        serdes_mode = _sfp_trunk_cntx.ptr_serdes_mode[trunk_id];

        SFP_LOG_DEBUG("combo_mode:%d serdes_mode:%d trunk_count:%d trunk_id:%d\r\n", combo_mode, serdes_mode, _sfp_trunk_cntx.trunk_count, trunk_id);
        if (AIR_PORT_COMBO_MODE_PHY == combo_mode)
        {
            /* COMBO PHY port */
            return TRUE;
        }
        else if ((AIR_PORT_COMBO_MODE_SERDES == combo_mode) &&
                 (AIR_PORT_SERDES_MODE_LAST > serdes_mode))
        {
            /* COMBO SERDES port */
            return TRUE;
        }
        else if ((AIR_PORT_COMBO_MODE_LAST <= combo_mode) &&
                 (AIR_PORT_SERDES_MODE_LAST > serdes_mode))
        {
            /* SERDES port */
            return TRUE;
        }
    }

    return FALSE;
}

/* FUNCTION NAME:   sfp_trunk_init
 * PURPOSE:
 *      Initialize the SFP trunk context.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK         -- Initialization succeeds
 *      Other            -- Initialization fails
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_trunk_init(
    UI32_T unit)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T max_group_cnt = 0, max_member_cnt = 0;

    ret = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    _sfp_trunk_cntx.trunk_count = max_group_cnt;

    osapi_calloc(max_group_cnt * sizeof(UI8_T), SFP_TASK_NAME, (void **)&_sfp_trunk_cntx.ptr_combo_mode);
    osapi_calloc(max_group_cnt * sizeof(UI8_T), SFP_TASK_NAME, (void **)&_sfp_trunk_cntx.ptr_serdes_mode);

    if ((NULL == _sfp_trunk_cntx.ptr_combo_mode) ||
        (NULL == _sfp_trunk_cntx.ptr_serdes_mode))
    {
        MW_FREE(_sfp_trunk_cntx.ptr_combo_mode);
        MW_FREE(_sfp_trunk_cntx.ptr_serdes_mode);
        return AIR_E_NO_MEMORY;
    }

    osapi_memset(_sfp_trunk_cntx.ptr_combo_mode, AIR_PORT_COMBO_MODE_LAST, max_group_cnt);
    osapi_memset(_sfp_trunk_cntx.ptr_serdes_mode, AIR_PORT_SERDES_MODE_LAST, max_group_cnt);

    return AIR_E_OK;
}

/* FUNCTION NAME:   sfp_trunk_check
 * PURPOSE:
 *      Check if a port is a trunk member. If it is, obtain all trunk member also.
 *
 * INPUT:
 *      port               -- The port number
 *      ptr_trunk_id       -- A pointer returns the trunk ID
 *      ptr_trunk_member   -- A pointer returns the trunk member
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK           -- It is a trunk member
 *      Other              -- It is not a trunk member
 *
 * NOTES:
 *
 */
AIR_ERROR_NO_T
sfp_trunk_check(
    UI32_T port,
    UI32_T *ptr_trunk_id,
    UI32_T *ptr_trunk_member)
{
    BOOL_T is_trunk = FALSE;
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_T trunk_id = 0;
    UI32_T unit = 0;
    UI32_T max_group_cnt = 0, max_member_cnt = 0, member_cnt = 0, member_idx = 0;
    UI32_T *ptr_member = NULL, trunk_member = 0;

    if ((port == 0) || (port > PLAT_MAX_PORT_NUM))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (NULL != ptr_trunk_member)
    {
        *ptr_trunk_member = 0;
    }

    if (NULL != ptr_trunk_id)
    {
        *ptr_trunk_id = 0xFF;
    }

    ret = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    osapi_calloc(sizeof(UI32_T) * max_member_cnt, SFP_TASK_NAME, (void **)&ptr_member);
    if (NULL == ptr_member)
    {
        return AIR_E_NO_MEMORY;
    }

    for (trunk_id = 0; trunk_id < max_group_cnt; trunk_id++)
    {
        ret = air_lag_getMember(unit, trunk_id, &member_cnt, ptr_member);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_INFO("ret:%d member_cnt:%d\n", ret, member_cnt);
            continue;
        }

        trunk_member = 0;
        /* Check if port is the member of the current trunk. */
        for (member_idx = 0; member_idx < member_cnt; member_idx++)
        {
            trunk_member |= BIT(ptr_member[member_idx] - 1);

            if (port == ptr_member[member_idx])
            {
               is_trunk = TRUE;
            }
        }

        if (TRUE == is_trunk)
        {
            break;
        }
    }

    MW_FREE(ptr_member);

    if (TRUE == is_trunk)
    {
        if (NULL != ptr_trunk_member)
        {
            *ptr_trunk_member = trunk_member;
        }

        if (NULL != ptr_trunk_id)
        {
            *ptr_trunk_id = trunk_id;
        }

        return AIR_E_OK;
    }

    return AIR_E_OTHERS;
}

BOOL_T
sfp_trunk_determineComboSerdesMode(
    const UI32_T unit,
    const UI32_T port)
{
    I32_T data = 0, ret = AIR_E_OTHERS, i = 0;
    UI32_T trunk_id = 0xFF, trunk_member = 0;
    UI8_T rx_los_pin = SFP_MODULE_PIN_HIGH;

    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboPort(unit, port)))
    {
        /* It is a PHY port. */
        return FALSE;
    }

    ret = sfp_trunk_check(port, &trunk_id, &trunk_member);
    if (AIR_E_OK != ret)
    {
        /* Need determine combo mode and/or serdes mode for a port is not a trunk member. */
        return TRUE;
    }

    if (FALSE == _sfp_trunk_is_comboSerdesValid(trunk_id))
    {
        /* No link up port within the trunk */
        return TRUE;
    }

    if (AIR_PORT_SERDES_MODE_100BASE_FX == _sfp_trunk_cntx.ptr_serdes_mode[trunk_id])
    {
        for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
        {
            if (0 != (trunk_member & BIT(i)))
            {
                ret = sfp_pin_io_getPIONum(unit, (i + 1), SFP_PIN_TYPE_RX_LOSS, &rx_los_pin);
                ret |= sfp_pin_io_getValue(unit, (i + 1), rx_los_pin, &data);
                if ((MW_E_OK == ret) && (SFP_MODULE_PIN_LOW == data))
                {
                    break;
                }
                else if (MW_E_OK != ret)
                {
                    SFP_LOG_WARN("RX_LOS pin does not exist for 100BASE_FX. port:%d", i + 1);
                }
            }
        }

        if (i >= PLAT_MAX_PORT_NUM)
        {
            /* All ports are removed when it is 100BASE_FX. */
            return TRUE;
        }
    }

    return FALSE;
}

void
sfp_trunk_handleLinkStatusChange(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T link,
    void *ptr_cookie)
{
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    UI32_T trunk_id = 0xFF, trunk_member = 0, i = 0;
    AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_COMBO_MODE_T member_combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    AIR_PORT_SERDES_MODE_T member_serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = sfp_task_getPortInfo(port), *ptr_taskPortInfoTmp = NULL;
    BOOL_T auto_adaption = FALSE;

    if (NULL == ptr_taskPortInfo)
    {
        return;
    }

    auto_adaption = (TRUE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED)) ? FALSE : TRUE;

    ret = sfp_trunk_check(port, &trunk_id, &trunk_member);
    if (AIR_E_OK != ret)
    {
        /* Do not process if it is not a trunk member. */
        return;
    }

    if ((FALSE == sfp_port_is_serdesPort(unit, port)) &&
        (FALSE == sfp_port_is_comboPort(unit, port)))
    {
        /* It is a PHY port */
        return;
    }

    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        ret |= air_port_getComboMode(unit, port, &combo_mode);
    }
    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
        (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
    {
        ret |= air_port_getSerdesMode(unit, port, &serdes_mode);
    }
    if (AIR_E_OK != ret)
    {
        return;
    }

    if (TRUE == link)
    {
        /* Link Up */
        if (FALSE == _sfp_trunk_is_comboSerdesValid(trunk_id))
        {
            SFP_LOG_INFO("Port:%d is the first linkup port within Trunk:%d trunk_member:0x%x combo_mode:%d serdes_mode:%d\r\n",
                      port, trunk_id, trunk_member, combo_mode, serdes_mode);
            if (TRUE == sfp_port_is_comboPort(unit, port))
            {
                _sfp_trunk_cntx.ptr_combo_mode[trunk_id] = combo_mode;
            }
            if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
            {
                _sfp_trunk_cntx.ptr_serdes_mode[trunk_id] = serdes_mode;
            }

            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if ((0 != (trunk_member & BIT(i))) && (port != i + 1))
                {
                    ptr_taskPortInfoTmp = sfp_task_getPortInfo(i + 1);
                    if (NULL == ptr_taskPortInfoTmp)
                    {
                        continue;
                    }

                    if (TRUE == sfp_port_is_comboPort(unit, port))
                    {
                        if (TRUE == sfp_port_is_comboPort(unit, i + 1))
                        {
                            air_port_getComboMode(unit, i + 1, &member_combo_mode);
                        }
                    }
                    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                        (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
                    {
                        if ((TRUE == sfp_port_is_serdesPort(unit, i + 1)) ||
                            (TRUE == sfp_port_is_comboSerdesPort(unit, i + 1)))
                        {
                            air_port_getSerdesMode(unit, i + 1, &member_serdes_mode);
                        }
                    }
                    if ((member_combo_mode == combo_mode) &&
                        (member_serdes_mode == serdes_mode))
                    {
                        continue;
                    }

                    sfp_phy_compensateLinkDown(unit, i + 1);

                    /* Set COMBO mode to the same as the one of 1st link up trunk member port. */
                    if (TRUE == sfp_port_is_comboPort(unit, port))
                    {
                        if (TRUE == sfp_port_is_comboPort(unit, i + 1))
                        {
                            sfp_port_setComboMode(unit, i + 1, TRUE, auto_adaption, combo_mode);
                            if (AIR_PORT_COMBO_MODE_PHY == combo_mode)
                            {
                                air_port_setSerdesMode(unit, i + 1, AIR_PORT_SERDES_MODE_SGMII);
                            }
                        }
                        else
                        {
                            SFP_LOG_ERROR("Different media type in a trunk. port1:%d combo_mode:%d port2 is not a combo port",
                                port, combo_mode);
                            return;
                        }
                    }

                    /* Set SERDES mode to the same as the one of 1st link up trunk member port. */
                    if ((TRUE == sfp_port_is_serdesPort(unit, port)) ||
                        (TRUE == sfp_port_is_comboSerdesPort(unit, port)))
                    {
                        if ((TRUE == sfp_port_is_serdesPort(unit, i + 1)) ||
                            (TRUE == sfp_port_is_comboSerdesPort(unit, i + 1)))
                        {
                            if (AIR_PORT_SERDES_MODE_100BASE_FX == serdes_mode)
                            {
                                /* Set Force mode before change to 100BASE_FX. */
                                air_port_setPhyAutoNego(unit, i + 1, AIR_PORT_PHY_AN_DISABLE);
                            }
                            air_port_setSerdesMode(unit, i + 1, serdes_mode);
                            SFP_LOG_DEBUG("Set serdes_mode:%d for port:%d\r\n", serdes_mode, i + 1);
                        }
                        else
                        {
                            SFP_LOG_ERROR("Different media type in a trunk. port1:%d serdes_mode:%d port2 is not a COMBO SFP port or SFP port:%d",
                                      port, serdes_mode, i + 1);
                            return;
                        }
                    }
                    sfp_db_updatePortMode(unit, ptr_taskPortInfoTmp, combo_mode, serdes_mode);

                    if (TRUE == auto_adaption)
                    {
                        sfp_auto_adaption_setState(unit, i + 1, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                        sfp_module_state_initPortBasedOnDB(ptr_taskPortInfoTmp);
                        /* Even if error occurs, still set port inited. */
                        sfp_set_port_inited(i + 1, SFP_PORT_INITED_BITMASK_ALL);
                    }
                    else
                    {
                        if ((TRUE == sfp_port_is_comboPort(unit, i + 1)) &&
                            (FALSE == sfp_port_is_comboSerdesPort(unit, i + 1)))
                        {
                            /* COMBO PHY port */
                            sfp_module_state_initPortBasedOnDB(ptr_taskPortInfoTmp);
                            /* Even if error occurs, still set port inited. */
                            sfp_set_port_inited(i + 1, SFP_PORT_INITED_BITMASK_ALL);
                        }

                        /* The SFP modules inserted to other Trunk members within the same trunk need be detected again.
                         * This need be invoked after COMBO PHY port is initialized based on DB.
                         */
                        sfp_module_fakeModuleRemoved(unit, i + 1);
                    }
                }
            }
        } /* Else just return */
    }
    else
    {
        /* Link Down */
        if (TRUE == _sfp_trunk_is_comboSerdesValid(trunk_id))
        {
            BOOL_T all_link_down = TRUE;
            AIR_PORT_STATUS_T port_status = {0};

            for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
            {
                if ((0 != (trunk_member & BIT(i))) && (port != i + 1))
                {
                    ret = sfp_port_getPortStatus(unit, i + 1, &port_status);
                    if (AIR_E_OK == ret)
                    {
                        if (0 != (port_status.flags & AIR_PORT_STATUS_FLAGS_LINK_UP))
                        {
                            all_link_down = FALSE;
                            break;
                        }
                    }
                }
            }

            if (TRUE == all_link_down)
            {
                SFP_LOG_DEBUG("All trunk_member is link down. Port:%d Trunk:%d trunk_member:0x%x\r\n", port, trunk_id, trunk_member);
                _sfp_trunk_cntx.ptr_combo_mode[trunk_id] = AIR_PORT_COMBO_MODE_LAST;
                _sfp_trunk_cntx.ptr_serdes_mode[trunk_id] = AIR_PORT_SERDES_MODE_LAST;

                for (i = 0; i < PLAT_MAX_PORT_NUM; i++)
                {
                    if (0 != (trunk_member & BIT(i)))
                    {
                        if (FALSE == auto_adaption)
                        {
                            /* The SFP modules inserted to other Trunk members within the same trunk need be detected again.
                             * This need be invoked after _sfp_trunk_cntx.ptr_combo_mode and _sfp_trunk_cntx.ptr_serdes_mode
                             * are reseted.
                             */
                            sfp_module_fakeModuleRemoved(unit, i + 1);
                        }
                    }
                }
            }
        } /* Else just return */
    }
}

void
sfp_trunk_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port)
{
    UI32_T trunk_id = 0;
    UI32_T max_group_cnt = 0, max_member_cnt = 0, member_cnt = 0;
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    UI32_T *ptr_member = NULL;

    ret = air_lag_getMaxGroupCnt(unit, &max_group_cnt, &max_member_cnt);
    if (AIR_E_OK != ret)
    {
        return;
    }

    osapi_calloc(sizeof(UI32_T) * max_member_cnt, SFP_TASK_NAME, (void **)&ptr_member);
    if (NULL == ptr_member)
    {
        return;
    }

    for (trunk_id = 0; trunk_id < max_group_cnt; trunk_id++)
    {
        ret = air_lag_getMember(unit, trunk_id, &member_cnt, ptr_member);
        if (AIR_E_OK != ret)
        {
            continue;
        }

        if (0 == member_cnt)
        {
            SFP_LOG_DEBUG("Handle Trunk delete: trunk_id:%d port:%d", trunk_id, port);
            _sfp_trunk_cntx.ptr_combo_mode[trunk_id] = AIR_PORT_COMBO_MODE_LAST;
            _sfp_trunk_cntx.ptr_serdes_mode[trunk_id] = AIR_PORT_SERDES_MODE_LAST;
        }
    }

    MW_FREE(ptr_member);

    sfp_auto_adaption_handleTrunkDeleted(unit, port);
    sfp_module_handleTrunkDeleted(unit, port);
}

AIR_ERROR_NO_T
sfp_trunk_getComboSerdes(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode,
    AIR_PORT_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;
    UI32_T trunk_id = 0xFF;

    if ((NULL == ptr_combo_mode) && (NULL == ptr_serdes_mode))
    {
        return AIR_E_BAD_PARAMETER;
    }

    ret = sfp_trunk_check(port, &trunk_id, NULL);
    if (AIR_E_OK != ret)
    {
        return ret;
    }

    if (NULL != ptr_combo_mode)
    {
        *ptr_combo_mode = _sfp_trunk_cntx.ptr_combo_mode[trunk_id];
    }

    if (NULL != ptr_serdes_mode)
    {
        *ptr_serdes_mode = _sfp_trunk_cntx.ptr_serdes_mode[trunk_id];
    }

    return AIR_E_OK;
}

