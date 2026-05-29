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

/* FILE NAME:  sfp_auto_adaptation.c
 * PURPOSE:
 *      1. The SFP SDA PIN driver and/or the SFP SDA IO driver are not initialized successfully. Adopt the SFP auto-adaption algorithm..
 *      2. Check if the SFP module just plugged and automatically adapt to SerDes mode.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */

#include "osal_lib.h"
#include "util.h"
#include <hal/common/hal.h>
#include "sfp_config_customer.h"
#include "sfp_module_handle.h"
#include "sfp_auto_adaptation.h"
#include "sfp_util.h"
#include "sfp_sff_data.h"
#include "sfp_task.h"
#include "sfp_db.h"
#include "sfp_trunk.h"
#include "sfp_port.h"
#include "sfp_module_inside_phy.h"
#include <hal/common/hal_mdio.h>
#include <aml/aml.h>

/* NAMING CONSTANT DECLARATIONS
 */
#ifdef AIR_LITE_MW
#define SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr) do                                      \
        {                                                                                              \
            base_addr = HAL_PEARL_SERDES_ID_S0_BASE_ADDR + (HAL_PEARL_SERDES_ADDR_OFFSER * serdes_id); \
        }while(0)
#elif AIR_EN_CORAL
#define SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr) do                                      \
        {                                                                                              \
            base_addr = (HAL_CORAL_SERDES_ID_S0 == serdes_id)?(0x10240000):(0x10220000);\
        }while(0)
#else
#define SERDES_ID_TO_SERDES_BASE_ADDRESS(__serdes_id__, __base_addr__) do                   \
        {                                                                                   \
            __base_addr__ = SERDES_ID_S0_BASE_ADDR + (SERDES_ADDR_OFFSER * __serdes_id__);  \
        }while(0)
#endif

static const UI32_T _PHY_REMOTE_SERDES_ADDR_MAPPING[4][2] = {
/*  {PHY ID,       serdes offset }*/
    {0,         0x10220000},
    {1,         0x10240000},
    {2,         0x10250000},
    {3,         0x10260000},
};

#define PHY_ID_TO_SERDES_BASE_ADDRESS(__phy_id__, __serdes_addr__)    do  \
    {                                                                     \
        UI32_T i;                                                         \
        for (i=0; i<QSGMII_LANE_PORT_NUM; i++)                            \
        {                                                                 \
            if(__phy_id__ == _PHY_REMOTE_SERDES_ADDR_MAPPING[i][0])       \
            {                                                             \
                __serdes_addr__ = _PHY_REMOTE_SERDES_ADDR_MAPPING[i][1];  \
            }                                                             \
        }                                                                 \
    }while(0)

/*Some module require >3s to complete AN. If the replacement frequency is to high ,it will never be recognized.*/
#define SFP_RETRY_SERDES_MODE_COUNT (10)
#define SFP_DELAY_CHANGE_COMBO_MODE_COUNT (1)
#define SFP_DELAY_CHANGE_100BASE_FX_COUNT (5)
#define SFP_RAND_MAX (3)

/* DATA TYPE DECLARATIONS
 */
#ifdef AIR_LITE_MW
typedef HAL_PEARL_SERDES_ID_T SERDES_ID_T;
#elif AIR_EN_CORAL
typedef HAL_CORAL_SERDES_ID_T SERDES_ID_T;
#endif
/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */

static UI8_T
_sfp_adaptation_genRand(
    UI8_T rand_max)
{
    /*Generate a [0 - rand_max] random integer*/
    UI32_T seed = (UI32_T)osal_rand();
    UI8_T rand_val = seed % (rand_max+1);
    return rand_val;
}

static BOOL_T
_sfp_adaptation_serdesRxIsSync(
    const UI32_T serdes_reg_pcs_state_value)
{
    if (0 != (serdes_reg_pcs_state_value & SFP_SERDES_PCS_STATE_REG_2_BIT5_RXSYNC))
    {
        return TRUE;
    }

    return FALSE;
}
#if defined(AIR_LITE_MW)
static BOOL_T
_sfp_adaptation_sigDet(
    const UI32_T serdes_reg_sig_det)
{
    if (0 != (serdes_reg_sig_det & SFP_SERDES_RX_CTRL_14_BIT23_SIGDET_CAL_RDY))
    {
        return TRUE;
    }

    return FALSE;
}
#elif defined(AIR_EN_CORAL)
static MW_ERROR_NO_T
_sfp_auto_adaption_coral_readReg(
    const UI32_T unit,
    const UI32_T port,
    UI32_T *ptr_reg_val)
{
    I32_T ret = MW_E_OK;
    UI32_T access_addr = 0;
    SERDES_ID_T serdes_id = SFP_SERDES_ID_LAST;
    UI32_T base_addr = 0;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    air_port_getSerdesMode(unit, port, &serdes_mode);

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

    if (HAL_CORAL_SERDES_ID_S0 == serdes_id)
    {
        access_addr = SFP_SERDES_DEBUG_XPON_0;
    }
    else
    {
        access_addr = base_addr + SFP_SERDES_RX_CTRL_14;
    }

    ret = aml_readReg(unit, access_addr, ptr_reg_val, sizeof(UI32_T));

    SFP_LOG_DEBUG("port:%d serdes_mode:%d SerDes_id:%d access_addr:0x%x reg_value:0x%x",
              port, serdes_mode, serdes_id, access_addr, *ptr_reg_val);

    return ret == AIR_E_OK ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

static BOOL_T
_sfp_adaptation_coral_regCheck(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T serdes_reg_val)
{
    SERDES_ID_T serdes_id = SFP_SERDES_ID_LAST;

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
    if (HAL_CORAL_SERDES_ID_S0 == serdes_id)
    {
        if (0 != (serdes_reg_val & SFP_SERDES_RO_DEBUG_0_BIT30_CDR_LCK2DATA))
        {
            return TRUE;
        }
    }
    else
    {
        if (0 != (serdes_reg_val & SFP_SERDES_RX_CTRL_14_BIT23_SIGDET_CAL_RDY))
        {
            return TRUE;
        }
    }

    return FALSE;
}
#else
static BOOL_T
_sfp_adaptation_pmaDataLock(
    const UI32_T serdes_reg_pma_cdr_lck2ref)
{
    if (0 == (serdes_reg_pma_cdr_lck2ref & SFP_SERDES_RO_DEBUG_0_BIT0_CDR_LCK2REF))
    {
        return TRUE;
    }

    return FALSE;
}
#endif
/* FUNCTION NAME:   _sfp_adaptation_setSerdesMode
 * PURPOSE:
 *      Set the Serdes mode for a specific SFP port and some necessary operations.
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 *      new_serdes_mode      -- The SerDes mode to be set.
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_NO_MEMORY          -- Insufficient memory
 *      AIR_E_OP_INVALID         -- Operation is invalid
 *      AIR_E_NOT_INITED         -- Not initialized
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_sfp_adaptation_setSerdesMode(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info,
    const AIR_PORT_SERDES_MODE_T new_serdes_mode)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;

    if ((NULL == ptr_port_info) || (AIR_PORT_SERDES_MODE_LAST <= new_serdes_mode))
    {
        return AIR_E_BAD_PARAMETER;
    }

    ptr_port_info->auto_adaption_delay_count = 0;

    ret |= air_port_setAdminState(unit, ptr_port_info->port, FALSE);
    SFP_LOG_DEBUG("set port %d admin state disable.\n", ptr_port_info->port);

    if (AIR_PORT_SERDES_MODE_100BASE_FX == new_serdes_mode)
    {
        /* Set system side AN */
        ret |= air_port_setPhyAutoNego(unit, ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Set system side AN fail. port:%d AN:%d ret:%d", ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE, ret);
        }
    }
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
    else if (AIR_PORT_SERDES_MODE_HSGMII == new_serdes_mode)
    {
        /* Set system side AN */
        ret |= air_port_setPhyAutoNego(unit, ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Set system side AN fail. port:%d AN:%d ret:%d", ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE, ret);
        }
    }
#endif

    ret |= air_port_setSerdesMode(unit, ptr_port_info->port, new_serdes_mode);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Failed to set tx_dis to HIGH or failed to set Combo mode or/and serdes mode. ret:%d", ret);
    }

    if (AIR_PORT_SERDES_MODE_100BASE_FX == new_serdes_mode)
    {
        ret = air_port_setSpeed(unit, ptr_port_info->port, AIR_PORT_SPEED_100M);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Failed to set speed. ret:%d", ret);
        }
    }
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
    else if (AIR_PORT_SERDES_MODE_HSGMII == new_serdes_mode)
    {
        ret = air_port_setSpeed(unit, ptr_port_info->port, AIR_PORT_SPEED_2500M);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Failed to set speed-2500M. ret:%d", ret);
        }
    }
#endif
    else
    {
        /* Set system side AN */
        ret |= air_port_setPhyAutoNego(unit, ptr_port_info->port, AIR_PORT_PHY_AN_ENABLE);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Set system side AN fail. port:%d AN:%d ret:%d", ptr_port_info->port, AIR_PORT_PHY_AN_DISABLE, ret);
        }
    }

    ret |= air_port_setAdminState(unit, ptr_port_info->port, TRUE);
    SFP_LOG_DEBUG("set port %d admin state enable.\n", ptr_port_info->port);

    ret |= sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, new_serdes_mode);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Failed to update port serdes mode to DB. ret:%d", ret);
    }

    return ret;
}

/* FUNCTION NAME:   _sfp_adaptation_tryOtherSerdesMode
 * PURPOSE:
 *      Since some SFP module need to be in different SerDes mode to read the correct AN, try switching to different SerDes modes to read th AN.
 * INPUT:
 *      unit                 -- Device unit number
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 *      AIR_E_OTHERS             -- Other errors.
 * NOTES:
 *      None.
 */
static AIR_ERROR_NO_T
_sfp_adaptation_tryOtherSerdesMode(
    const UI32_T unit,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_SERDES_MODE_T new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII, old_serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);

    ret = air_port_getSerdesMode(unit, ptr_port_info->port, &old_serdes_mode);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Failed to get serdes mode. ret:%d", ret);
        return ret;
    }

    if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
    {
        /* Check 8804 serdes port, it not support hsgmii */
        if(AIR_PORT_SERDES_MODE_SGMII == old_serdes_mode)
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
        }
        else
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
        }
    }
    else
    {
        if(AIR_PORT_SERDES_MODE_SGMII == old_serdes_mode)
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
        }
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
        else if(AIR_PORT_SERDES_MODE_1000BASE_X == old_serdes_mode)
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_HSGMII;
        }
        else if(AIR_PORT_SERDES_MODE_HSGMII == old_serdes_mode)
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
        }
#endif
        else
        {
            new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
        }
    }

    if (new_serdes_mode != old_serdes_mode)
    {
        ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, new_serdes_mode);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
        }

        SFP_LOG_DEBUG("switch to other serdes mode:0x%x.", new_serdes_mode);
    }

    return ret;
}

static void
_sfp_auto_adaption_checkComboSerdesChange(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T db_set,
    const UI32_T old_state,
    const UI32_T new_state,
    const AIR_PORT_COMBO_MODE_T old_combo_mode,
    const AIR_PORT_SERDES_MODE_T old_serdes_mode)
{
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = sfp_task_getPortInfo(port);
    AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    AIR_ERROR_NO_T ret = AIR_E_OTHERS;

    SFP_LOG_DEBUG("port:%d db_set:%d old_state:%d new_state:%d old_combo_mode:%d old_serdes_mode:%d",
              port, db_set, old_state, new_state, old_combo_mode, old_serdes_mode);

    if ((NULL == ptr_taskPortInfo) || (TRUE == db_set))
    {
        return;
    }

    if (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != new_state)
    {
        return;
    }

    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        ret = air_port_getComboMode(unit, port, &combo_mode);
        if (AIR_E_OK != ret)
        {
            return;
        }
    }

    if ((TRUE == sfp_port_is_comboSerdesPort(unit, port)) ||
        (TRUE == sfp_port_is_serdesPort(unit, port)))
    {
        ret = air_port_getSerdesMode(unit, port, &serdes_mode);
        if (AIR_E_OK != ret)
        {
            return;
        }
    }

    /* Check COMBO port */
    if ((AIR_PORT_COMBO_MODE_LAST != old_combo_mode) &&
        (old_combo_mode == combo_mode) &&
        ((AIR_PORT_COMBO_MODE_PHY == combo_mode) ||
         ((AIR_PORT_COMBO_MODE_SERDES == combo_mode) &&
          (old_serdes_mode == serdes_mode))))
    {
        return;
    }

    /* Check SFP port */
    if ((AIR_PORT_COMBO_MODE_LAST == old_combo_mode) &&
        (old_serdes_mode == serdes_mode))
    {
        return;
    }

    SFP_LOG_WARN("Compensate initPortBasedOnDB");
    sfp_module_state_initPortBasedOnDB(ptr_taskPortInfo);
    sfp_set_port_inited(ptr_taskPortInfo->port, SFP_PORT_INITED_BITMASK_ALL);
}

/* FUNCTION NAME:   sfp_adaptation_polling
 * PURPOSE:
 *      Polling SerDes register state information for each SFP port about PCS state and AN state to determine what SerDes mode to set.
 * INPUT:
 *      unit                 -- Device unit number
 *      port_index           -- The SFP id index
 *      ptr_port_info        -- A pointer to the corresponding port node of
 *                              sfp_task_ctx.port_info array
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong
 *      AIR_E_NOT_INITED     -- Not initialized
 *      AIR_E_NO_MEMORY      -- Insufficient memory
 *      AIR_E_OP_INCOMPLETE  -- Operation is not completed
 *      AIR_E_OTHERS         -- Other errors.
 * NOTES:
 *      None.
 */
AIR_ERROR_NO_T
sfp_adaptation_polling(
    const UI32_T unit,
    const UI8_T port_index,
    SFP_TASK_PORT_INFO_T *ptr_port_info)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    AIR_PORT_SERDES_MODE_T new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII, old_serdes_mode = AIR_PORT_SERDES_MODE_LAST;
    UI32_T serdes_reg_an5_value = 0;
#if defined(AIR_LITE_MW) || defined(AIR_EN_CORAL)
    UI32_T serdes_reg_sig_det = 0;
#else
    UI32_T serdes_reg_pma_cdr_lck2ref = 0;
#endif
    UI32_T serdes_reg_pcs_state_value = 0;
    I32_T loss_of_signal = SFP_MODULE_PIN_HIGH;
    UI8_T rx_los_pin = SFP_PIN_PIONUM_INVALID;
    BOOL_T pcs_state_rx_sync = 0;
    BOOL_T admin_state = FALSE;
    AIR_PORT_COMBO_MODE_T old_combo_mode = AIR_PORT_COMBO_MODE_LAST;
    UI32_T old_state = SFP_TASK_PORT_ADAPTION_STATE_LAST;
    BOOL_T db_set = FALSE;
    AIR_PORT_STATUS_T ps = {0};

    if (NULL == ptr_port_info)
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* The SFP SDA PIN driver and/or the SFP SDA IO driver are initialized successfully. skip the SFP auto-adaption algorithm. */
    if(TRUE == sfp_pin_checkInitState(ptr_port_info->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        return AIR_E_OK;
    }

    if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
    {
        ret = air_port_getComboMode(unit, ptr_port_info->port, &old_combo_mode);
        if (AIR_E_OK != ret)
        {
            return ret;
        }
    }

    if ((TRUE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)) ||
        (TRUE == sfp_port_is_serdesPort(unit, ptr_port_info->port)))
    {
        ret = air_port_getSerdesMode(unit, ptr_port_info->port, &old_serdes_mode);
        if (AIR_E_OK != ret)
        {
            return ret;
        }
    }

    if (FALSE == sfp_trunk_determineComboSerdesMode(unit, ptr_port_info->port))
    {
        SFP_LOG_DEBUG("port:%d Determine:%d\r\n", ptr_port_info->port, FALSE);
        return ret;
    }

    old_state = ptr_port_info->auto_adaption_state;

    if (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != ptr_port_info->auto_adaption_state)
    {
        sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
    }

    sfp_pin_io_getPIONum(unit, ptr_port_info->port, SFP_PIN_TYPE_RX_LOSS, &rx_los_pin);

    if (FALSE == sfp_port_is_comboPort(unit, ptr_port_info->port))
    {
        ret = sfp_pin_io_getValue(unit, ptr_port_info->port, rx_los_pin, &loss_of_signal);
        if (AIR_E_OK == ret)
        {
            if (SFP_MODULE_PIN_HIGH == loss_of_signal)
            {
                /* SFP module is removed. set defult SerDes mode to 1000base-x*/
                ptr_port_info->auto_adaption_delay_count = 0;
                ptr_port_info->auto_adaption_serdes_matched = FALSE;
                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                if (AIR_PORT_SERDES_MODE_1000BASE_X != old_serdes_mode)
                {
                    ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_1000BASE_X);
                    if (AIR_E_OK != ret)
                    {
                        SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                    }
                    else
                    {
                        sfp_module_state_initPortBasedOnDB(ptr_port_info);
                        db_set = TRUE;
                        sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                    }
                }

                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
                _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                         ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);
                return AIR_E_OK;
            }
        }
    }

    ret = sfp_adaption_readReg(unit, ptr_port_info->port, SFP_SERDES_PCS_STATE_REG_2, &serdes_reg_pcs_state_value);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Port:%d failed to read SerDes register reg_addr:0x%x ret:%d", ptr_port_info->port, SFP_SERDES_PCS_STATE_REG_2, ret);
        _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                 ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);

        return ret;
    }
#if defined(AIR_LITE_MW)
    ret = sfp_adaption_readReg(unit, ptr_port_info->port, SFP_SERDES_RX_CTRL_14, &serdes_reg_sig_det);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Port:%d failed to read SerDes register reg_addr:0x%x ret:%d", ptr_port_info->port, SFP_SERDES_RX_CTRL_14, ret);
    }
#elif defined(AIR_EN_CORAL)
    ret = _sfp_auto_adaption_coral_readReg(unit, ptr_port_info->port, &serdes_reg_sig_det);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Port:%d failed to read coral SerDes register ret:%d", ptr_port_info->port, ret);
    }
#else
    ret = sfp_adaption_readReg(unit, ptr_port_info->port, SFP_SERDES_RO_DEBUG_0, &serdes_reg_pma_cdr_lck2ref);
    if (AIR_E_OK != ret)
    {
        SFP_LOG_ERROR("Port:%d failed to read SerDes register reg_addr:0x%x ret:%d", ptr_port_info->port, SFP_SERDES_RO_DEBUG_0, ret);
    }
#endif

#if defined(AIR_LITE_MW)
    if ((TRUE == _sfp_adaptation_serdesRxIsSync(serdes_reg_pcs_state_value)) &&
        (TRUE == _sfp_adaptation_sigDet(serdes_reg_sig_det)))
#elif defined(AIR_EN_CORAL)
    if ((TRUE == _sfp_adaptation_serdesRxIsSync(serdes_reg_pcs_state_value)) &&
        (TRUE == _sfp_adaptation_coral_regCheck(unit, ptr_port_info->port, serdes_reg_sig_det)))
#else
    if ((TRUE == _sfp_adaptation_serdesRxIsSync(serdes_reg_pcs_state_value)) &&
        (TRUE == _sfp_adaptation_pmaDataLock(serdes_reg_pma_cdr_lck2ref)))
#endif
    {
        pcs_state_rx_sync = TRUE;
        ret = sfp_adaption_readReg(unit, ptr_port_info->port, SFP_SERDES_AN_REG_5, &serdes_reg_an5_value);
        if (AIR_E_OK != ret)
        {
            SFP_LOG_ERROR("Port:%d failed to read SerDes register reg_addr:0x%x ret:%d", ptr_port_info->port, SFP_SERDES_AN_REG_5, ret);
        }
    }

    if (FALSE == pcs_state_rx_sync)
    {
        BOOL_T admin_state = FALSE;

        /*SFP module is removed or port state set to disable*/
        if (0 != ptr_port_info->auto_adaption_retry_count)
        {
            ptr_port_info->auto_adaption_retry_count = 0;
        }

        air_port_getAdminState(unit, ptr_port_info->port, &admin_state);

        if (TRUE == admin_state)
        {
            if ((TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port)) &&
                (FALSE == sfp_port_is_pureComboSerdesPort(unit, ptr_port_info->port)))
            {
                /* Case 1. There is no SFP transceiver inserted for a combo SFP port. */
                if ((TRUE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port)) ||
                    (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != ptr_port_info->auto_adaption_state))
                {
                    if (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != ptr_port_info->auto_adaption_state)
                    {
                        SFP_LOG_DEBUG("auto_adaption_state:%d", ptr_port_info->auto_adaption_state);
                    }
                    if (SFP_DELAY_CHANGE_COMBO_MODE_COUNT <= ptr_port_info->auto_adaption_delay_count)
                    {
                        ptr_port_info->auto_adaption_delay_count = 0;
                        ret = sfp_port_setComboMode(unit, ptr_port_info->port, admin_state, TRUE, AIR_PORT_COMBO_MODE_PHY);
                        ret |= air_port_setSerdesMode(unit, ptr_port_info->port, AIR_PORT_SERDES_MODE_SGMII);
                        ret |= sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_PHY, AIR_PORT_SERDES_MODE_SGMII);
                        if (AIR_E_OK != ret)
                        {
                            SFP_LOG_ERROR("Failed to update port serdes mode to DB. ret:%d", ret);
                        }

                        sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                        sfp_clear_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                    }
                    else
                    {
                        ptr_port_info->auto_adaption_delay_count++;
                    }
                }

                if (FALSE == sfp_get_port_inited(ptr_port_info->port))
                {
                    sfp_module_state_initPortBasedOnDB(ptr_port_info);
                    db_set = TRUE;
                    sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                }
            }
            else if (FALSE == sfp_port_is_comboPort(unit, ptr_port_info->port))
            {
                /* To support a 100base-fx SFP transceiver, the RX_LOS pin of an SFP port must be accessible. */
                if (rx_los_pin != SFP_PIN_PIONUM_INVALID)
                {
                    /* Case 2. There is a 100base-fx SFP transceiver or no SFP transceiver inserted for a SFP port with an accessible RX_LOS pin. */
                    if (TRUE == ptr_port_info->auto_adaption_serdes_matched)
                    {
                        if (SFP_TASK_PORT_ADAPTION_STATE_ACTIVE != ptr_port_info->auto_adaption_state)
                        {
                            sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                        }
                        SFP_LOG_DEBUG("Port %d serdes %d is matched",ptr_port_info->port,old_serdes_mode);
                    }
                    else
                    {
                        if (0 == ptr_port_info->auto_adaption_delay_count)
                        {
                            if (AIR_PORT_SERDES_MODE_1000BASE_X == old_serdes_mode)
                            {
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
                                ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_HSGMII);
                                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                                if (AIR_E_OK != ret)
                                {
                                    SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                                }
                                ptr_port_info->auto_adaption_delay_count = SFP_DELAY_CHANGE_100BASE_FX_COUNT;
                            }
                            else if (AIR_PORT_SERDES_MODE_HSGMII == old_serdes_mode)
                            {
#endif
                                ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_100BASE_FX);
                                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                                if (AIR_E_OK != ret)
                                {
                                    SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                                }
                                ptr_port_info->auto_adaption_delay_count = SFP_DELAY_CHANGE_100BASE_FX_COUNT;
                            }
                            else
                            {
                                ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_1000BASE_X);
                                if (AIR_E_OK != ret)
                                {
                                    SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                                }
                                else
                                {
                                    sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
                                    sfp_module_state_initPortBasedOnDB(ptr_port_info);
                                    db_set = TRUE;
                                    sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                                }
                                ptr_port_info->auto_adaption_delay_count = SFP_DELAY_CHANGE_100BASE_FX_COUNT;
                            }
                        }
                        else
                        {
                            air_port_getPortStatus(unit, ptr_port_info->port, &ps);
                            if (ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)
                            {
                                ptr_port_info->auto_adaption_serdes_matched = TRUE;
                                ptr_port_info->auto_adaption_delay_count = 0;
                            }
                            else
                            {
                                ptr_port_info->auto_adaption_delay_count--;
                            }
                        }
                    }
                }
                else
                {
                    /* Case 3. There is no SFP transceiver inserted for a SFP port without an accessible RX_LOS pin. */
                    if (AIR_PORT_SERDES_MODE_1000BASE_X != old_serdes_mode)
                    {
                        ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_1000BASE_X);
                        if (AIR_E_OK != ret)
                        {
                            SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                        }
                        else
                        {
                            sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
                            sfp_module_state_initPortBasedOnDB(ptr_port_info);
                            db_set = TRUE;
                            sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                        }
                    }
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
                    else
                    {
                        ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_HSGMII);
                        if (AIR_E_OK != ret)
                        {
                            SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                        }
                        else
                        {
                            sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
                            sfp_module_state_initPortBasedOnDB(ptr_port_info);
                            db_set = TRUE;
                            sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                        }
                    }
#endif
                }
            }
            _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                     ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);
            return ret;
        }
        else
        {
            /* Case 4. Admin state is disabled. */
            ptr_port_info->auto_adaption_delay_count = 0;

            /* Admin state is FALSE. Change to 1000BASE_X in order to get valid pcs_state value. */
            if ((FALSE == sfp_port_is_comboPort(unit, ptr_port_info->port)) && (AIR_PORT_SERDES_MODE_1000BASE_X != old_serdes_mode))
            {
                /* RX_SYNC is 0 because admin state is disabled. Change to 1000base-x to obtain valid RX_SYNC. */
                ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, AIR_PORT_SERDES_MODE_1000BASE_X);
                SFP_LOG_DEBUG("Change to 1000base-x from %d to get valid RX_SYNC. ret:%d", old_serdes_mode, ret);

                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_TRY);
                sfp_module_state_initPortBasedOnDB(ptr_port_info);
                db_set = TRUE;
                sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);

                sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, AIR_PORT_SERDES_MODE_1000BASE_X);
            }

            _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                      ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);

            return ret;
        }
    }
    else
    {
        /* SFP module is inserted. */
        ret = sfp_port_getAdminState(unit, ptr_port_info->port, &admin_state);
        if(AIR_E_OK != ret)
        {
             SFP_LOG_ERROR("Failed to get Admin State. port:%d ret:%d", ptr_port_info->port, ret);
        }

        if (TRUE == admin_state)
        {
            if (FALSE == sfp_get_port_inited(ptr_port_info->port))
            {
                sfp_module_state_initPortBasedOnDB(ptr_port_info);
                db_set = TRUE;
                sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
            }
        }

        if (TRUE == sfp_port_is_comboPort(unit, ptr_port_info->port))
        {
            if (FALSE == sfp_port_is_comboSerdesPort(unit, ptr_port_info->port))
            {
                ret = air_port_setAdminState(unit, ptr_port_info->port, FALSE);
                if (AIR_E_OK != ret)
                {
                    SFP_LOG_ERROR("Failed to set admin state. ret:%d", ret);
                }
                ret = air_port_setComboMode(unit, ptr_port_info->port, AIR_PORT_COMBO_MODE_SERDES);
                if (AIR_E_OK != ret)
                {
                    SFP_LOG_ERROR("Port:%d failed to get/set COMBO mode to SERDES.", ptr_port_info->port);
                }
                else
                {
                    if (TRUE == sfp_get_port_inited(ptr_port_info->port))
                    {
                        sfp_clear_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                        sfp_phy_compensateLinkDown(unit, ptr_port_info->port);
                    }
                    if (AIR_PORT_SERDES_MODE_LAST == old_serdes_mode)
                    {
                        ret = air_port_getSerdesMode(unit, ptr_port_info->port, &old_serdes_mode);
                    }
                    ret |= sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, old_serdes_mode);
                    if (AIR_E_OK != ret)
                    {
                        SFP_LOG_ERROR("Failed to update port:%d serdes mode:%d to DB. ret:%d", ret, ptr_port_info->port, old_serdes_mode);
                    }
                }
                ret = air_port_setAdminState(unit, ptr_port_info->port, TRUE);
                if (AIR_E_OK != ret)
                {
                    SFP_LOG_ERROR("Failed to set admin state. ret:%d", ret);
                }
            }
        }
#ifdef SFP_AUTO_ADAPTION_SUPPORT_HSGMII
        else
        {
            /* 8804 not support HSGMII,
               Only HSGMII may pass the check */
            if (0 == (serdes_reg_pcs_state_value & SFP_SERDES_PCS_STATE_AN_DONE_BIT0))
            {
                sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
                new_serdes_mode = old_serdes_mode;
                if ((AIR_PORT_SERDES_MODE_HSGMII != old_serdes_mode) && (AIR_PORT_SERDES_MODE_LAST != old_serdes_mode))
                {
                    new_serdes_mode = AIR_PORT_SERDES_MODE_HSGMII;
                }
                if (new_serdes_mode != old_serdes_mode)
                {
                    ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, new_serdes_mode);
                    if (AIR_E_OK != ret)
                    {
                        SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
                    }
                    else
                    {
                        sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, new_serdes_mode);
                        sfp_module_state_initPortBasedOnDB(ptr_port_info);
                        db_set = TRUE;
                        sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
                    }
                }

                SFP_LOG_DEBUG("port:%d state:%d (rx_loss:%d) (pcs_state value:0x%x rx_sync:%d) (AN5 value:0x%x) (serdes mode|old:%d new:%d)\n",
                          ptr_port_info->port ,ptr_port_info->auto_adaption_state ,loss_of_signal ,serdes_reg_pcs_state_value, pcs_state_rx_sync,
                          serdes_reg_an5_value, old_serdes_mode, new_serdes_mode);

                _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                         ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);
                return AIR_E_OK;
            }
        }
#endif
    }

    if (0 != serdes_reg_an5_value)
    {
        sfp_auto_adaption_setState(unit, ptr_port_info->port, SFP_TASK_PORT_ADAPTION_STATE_ACTIVE);
        /*Set the correct SerDes mode according to the value of AN5*/
        new_serdes_mode = old_serdes_mode;

        if (TRUE == ((serdes_reg_an5_value & SFP_SERDES_AN_REG_5_BIT0) && (serdes_reg_an5_value & SFP_SERDES_AN_REG_5_BIT15)))
        {
            if ((AIR_PORT_SERDES_MODE_SGMII != old_serdes_mode) && (AIR_PORT_SERDES_MODE_LAST != old_serdes_mode))
            {
                new_serdes_mode = AIR_PORT_SERDES_MODE_SGMII;
            }
        }
        else
        {
            if ((AIR_PORT_SERDES_MODE_1000BASE_X != old_serdes_mode) && (AIR_PORT_SERDES_MODE_LAST != old_serdes_mode))
            {
                new_serdes_mode = AIR_PORT_SERDES_MODE_1000BASE_X;
            }
        }

        if (new_serdes_mode != old_serdes_mode)
        {
            ret = _sfp_adaptation_setSerdesMode(unit, ptr_port_info, new_serdes_mode);
            if (AIR_E_OK != ret)
            {
                SFP_LOG_ERROR("Failed to set serdes mode. ret:%d", ret);
            }
            else
            {
                sfp_db_updatePortMode(unit, ptr_port_info, AIR_PORT_COMBO_MODE_SERDES, new_serdes_mode);
                sfp_module_state_initPortBasedOnDB(ptr_port_info);
                db_set = TRUE;
                sfp_set_port_inited(ptr_port_info->port, SFP_PORT_INITED_BITMASK_ALL);
            }
        }
    }
    else
    {
        /*1.It proves that the current SerDes mode is incorrect. Then try other SerDes mode until a non-0 AN5 is read to determine
         *the correct SerDes mode.
         *2.Each count plus a random value is to avoid conflicts caused by the same device being connected and switching SerDes mode at the same rhythm.
         */
        if (SFP_RETRY_SERDES_MODE_COUNT <= (ptr_port_info->auto_adaption_retry_count + _sfp_adaptation_genRand(SFP_RAND_MAX)))
        {
            ptr_port_info->auto_adaption_retry_count = 0;
            ret = _sfp_adaptation_tryOtherSerdesMode(unit, ptr_port_info);
            if (AIR_E_OK != ret)
            {
                SFP_LOG_ERROR("Failed to switch to other serdes mode. ret:%d", ret);
            }
            _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                                     ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);
            return ret;
        }
        ptr_port_info->auto_adaption_retry_count++;
    }
    SFP_LOG_DEBUG("port:%d state:%d (rx_loss:%d) (pcs_state value:0x%x rx_sync:%d) (AN5 value:0x%x) (serdes mode|old:%d new:%d)\n",
              ptr_port_info->port ,ptr_port_info->auto_adaption_state ,loss_of_signal ,serdes_reg_pcs_state_value, pcs_state_rx_sync,
              serdes_reg_an5_value, old_serdes_mode, new_serdes_mode);

    _sfp_auto_adaption_checkComboSerdesChange(unit, ptr_port_info->port, db_set, old_state,
                                             ptr_port_info->auto_adaption_state, old_combo_mode, old_serdes_mode);

    return AIR_E_OK;
}

/* FUNCTION NAME:   sfp_auto_adaptation_getState
 * PURPOSE:
 *      Get the SFP auto_adaptation state for a port.
 *
 * INPUT:
 *      port                 -- Port number
 * OUTPUT:
 *      ptr_state            -- A pointer returns the SFP auto_adaptation state
 * RETURN:
 *      None
 *
 * NOTES:
 *      None.
 */
void
sfp_auto_adaptation_getState(
    UI8_T port,
    SFP_TASK_PORT_ADAPTION_STATE_T *ptr_state)
{
    SFP_TASK_PORT_INFO_T *ptr_port_info = sfp_task_getPortInfo(port);

    if (NULL != ptr_port_info)
    {
        *ptr_state = ptr_port_info->auto_adaption_state;
    }
    else
    {
        *ptr_state = SFP_TASK_PORT_ADAPTION_STATE_LAST;
    }
}

MW_ERROR_NO_T
sfp_auto_adaption_setState(
    const UI32_T unit,
    const UI32_T port,
    UI8_T state)
{
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = sfp_task_getPortInfo(port);

    if (NULL == ptr_taskPortInfo)
    {
        return MW_E_BAD_PARAMETER;
    }

    SFP_LOG_DEBUG("Set auto state:%d for port:%d\r\n", state, port);
    ptr_taskPortInfo->auto_adaption_state = state;
    return MW_E_OK;
}

MW_ERROR_NO_T
sfp_auto_adaption_init(
    const UI32_T unit)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettingsArray = sfp_config_getPortSettingsArray();
    UI32_T port_count = sfp_config_getPortSettingsArraySize(), i = 0;
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;
    MW_ERROR_NO_T ret = MW_E_OK;
    UI8_T tx_disable_pin = SFP_PIN_PIONUM_INVALID;

    if ((NULL != ptr_portSettingsArray) && (0 != port_count))
    {
        for (; i < port_count; i++)
        {
            ptr_taskPortInfo = sfp_task_getPortInfo(ptr_portSettingsArray[i].port);
            if (NULL == ptr_taskPortInfo)
            {
                ret = MW_E_OTHERS;
                break;
            }

            if (TRUE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
            {
                SFP_LOG_DEBUG("Do not use SFP auto adpation. port:%d\r\n", ptr_taskPortInfo->port);
                continue;
            }

            sfp_auto_adaption_setState(unit, ptr_taskPortInfo->port, SFP_TASK_PORT_ADAPTION_STATE_IDLE);

            if (TRUE == sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_IO_INIT_SUCCEED))
            {
                tx_disable_pin = SFP_PIN_PIONUM_INVALID;
                sfp_pin_io_getPIONum(unit, ptr_taskPortInfo->port, SFP_PIN_TYPE_TX_DISABLE, &tx_disable_pin);
                if (SFP_PIN_PIONUM_INVALID != tx_disable_pin)
                {
                    /* TX_DISABLE pin exists. */
                    ret = sfp_pin_io_setValue(unit, ptr_taskPortInfo->port, tx_disable_pin, SFP_MODULE_PIN_LOW);
                    SFP_LOG_DEBUG("Port:%d set tx-disable pin:%d to low. ret:%d", ptr_taskPortInfo->port, tx_disable_pin, ret);
                    if (MW_E_OK != ret)
                    {
                        break;
                    }
                }
            }

            if (TRUE != sfp_port_is_comboPort(unit, ptr_taskPortInfo->port))
            {
                sfp_module_state_initPortBasedOnDB(ptr_taskPortInfo);
                sfp_set_port_inited(ptr_taskPortInfo->port, SFP_PORT_INITED_BITMASK_ALL);
            }
        }
    }

    return ret;
}

void
sfp_auto_adaption_handleTrunkDeleted(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_PORT_STATUS_T port_status = {0};
    SFP_TASK_PORT_INFO_T *ptr_taskPortInfo = NULL;

    ptr_taskPortInfo = sfp_task_getPortInfo(port);
    if (NULL == ptr_taskPortInfo)
    {
        return;
    }

    if (FALSE != sfp_pin_checkInitState(ptr_taskPortInfo->pin_state, SFP_PIN_SDA_INIT_SUCCEED | SFP_PIN_IO_INIT_SUCCEED))
    {
        /* Not SFP auto adaption */
        return;
    }

    air_port_getPortStatus(unit, port, &port_status);
    if (0 == (port_status.flags & AIR_PORT_STATUS_FLAGS_LINK_UP))
    {
        if ((TRUE == sfp_port_is_comboSerdesPort(unit, port)) ||
            (TRUE == sfp_port_is_serdesPort(unit, port)))
        {
            /* In order to get valid pcs_state value. */
            air_port_setSerdesMode(unit, port, AIR_PORT_SERDES_MODE_1000BASE_X);
            sfp_db_updatePortMode(unit, ptr_taskPortInfo, AIR_PORT_COMBO_MODE_SERDES, AIR_PORT_SERDES_MODE_1000BASE_X);
        }
    }
}

MW_ERROR_NO_T
sfp_adaption_readReg(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    UI32_T *ptr_reg_val)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T access_addr = 0;
    SERDES_ID_T serdes_id = SFP_SERDES_ID_LAST;
    UI32_T base_addr = 0;
#if (!defined(AIR_LITE_MW)) && (!defined(AIR_EN_CORAL))
    UI32_T phy_id = 0;
#endif
    AIR_PORT_COMBO_MODE_T combo_mode = AIR_PORT_COMBO_MODE_LAST;
    AIR_PORT_SERDES_MODE_T serdes_mode = AIR_PORT_SERDES_MODE_LAST;

    if (NULL == ptr_reg_val)
    {
        return MW_E_BAD_PARAMETER;
    }

    air_port_getSerdesMode(unit, port, &serdes_mode);

#if (!defined(AIR_LITE_MW)) && (!defined(AIR_EN_CORAL))
    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        air_port_getComboMode(unit, port, &combo_mode);
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, base_addr);

        access_addr = base_addr + reg_addr;
        ret = hal_mdio_readRegByI2c(unit, port, access_addr, ptr_reg_val);
    }
    else
#endif
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

        access_addr = base_addr + reg_addr;
        ret = aml_readReg(unit, access_addr, ptr_reg_val, sizeof(UI32_T));
    }
    SFP_LOG_DEBUG("port:%d combo_mode:%d serdes_mode:%d SerDes_id:%d access_addr:0x%x reg_value:0x%x",
              port, combo_mode, serdes_mode, serdes_id, access_addr, *ptr_reg_val);

    return ret == AIR_E_OK ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

MW_ERROR_NO_T
sfp_writeReg(
    const UI32_T unit,
    const UI32_T port,
    const UI16_T reg_addr,
    const UI32_T reg_val)
{
    AIR_ERROR_NO_T ret = AIR_E_OK;
    UI32_T access_addr = 0;
    SERDES_ID_T serdes_id = SFP_SERDES_ID_LAST;
    UI32_T base_addr = 0;
#if (!defined(AIR_LITE_MW)) && (!defined(AIR_EN_CORAL))
    UI32_T phy_id = 0;
#endif

#if (!defined(AIR_LITE_MW)) && (!defined(AIR_EN_CORAL))
    if (TRUE == sfp_port_is_comboPort(unit, port))
    {
        HAL_AIR_PORT_TO_PHY_PORT(unit, port, phy_id);
        PHY_ID_TO_SERDES_BASE_ADDRESS(phy_id, base_addr);

        access_addr = base_addr + reg_addr;
        ret = hal_mdio_writeRegByI2c(unit, port, access_addr, reg_val);
    }
    else
#endif
    {
        HAL_AIR_PORT_TO_SERDES_ID(unit, port, serdes_id);
        SERDES_ID_TO_SERDES_BASE_ADDRESS(serdes_id, base_addr);

        access_addr = base_addr + reg_addr;
        ret = aml_writeReg(unit, access_addr, &reg_val, sizeof(UI32_T));
    }
    SFP_LOG_DEBUG("port:%d SerDes_id:%d reg_addr:0x%x reg_value:0x%x",
              port, serdes_id, access_addr, reg_val);

    return ret == AIR_E_OK ? MW_E_OK : MW_E_OP_INCOMPLETE;
}

