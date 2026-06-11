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

/* FILE NAME:  hal_phy.c
 * PURPOSE:
 *  Implement PHY module API function.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/common/hal_phy.h>

#include <hal/common/hal.h>
#include <hal/common/hal_cfg.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_mdio.h>
#ifdef AIR_EN_EN8801S_PHY
#include <hal/phy/en8801s/hal_en8801s_phy.h>
#endif
#ifdef AIR_EN_EN8808_PHY
#include <hal/phy/en8808/hal_en8808_phy.h>
#endif
#ifdef AIR_EN_EN8811H_PHY
#include <hal/phy/en8811h/hal_en8811h_phy.h>
#endif
#ifdef AIR_EN_AN8801SB_PHY
#include <hal/phy/an8801sb/hal_an8801sb_phy.h>
#endif
#ifdef AIR_EN_AN8811B_PHY
#include <hal/phy/an8811b/hal_an8811b_phy.h>
#endif
#ifdef AIR_EN_AN8804_PHY
#include <hal/phy/an8804/hal_an8804_phy.h>
#endif
#ifdef AIR_EN_AN8858_PHY
#include <hal/phy/an8858/hal_an8858_phy.h>
#endif
#ifdef AIR_EN_AN8808_PHY
#include <hal/phy/an8808/hal_an8808_phy.h>
#endif
#include <cmlib/cmlib_bit.h>
#ifdef AIR_EN_SCORPIO
#include <hal/switch/sco/hal_sco_serdes.h>
#endif
#ifdef AIR_EN_PEARL
#include <hal/switch/pearl/hal_pearl_serdes.h>
#endif
#ifdef AIR_EN_CORAL
#include <hal/switch/coral/hal_coral_serdes.h>
#endif
/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PHY_SYNC_WGC_UNINIT_ADDR     (0xFF)
#define HAL_PHY_SYNC_WGC_INITED_ADDR     (0xFE)
#define HAL_PHY_SYNC_WGC_MAX_I2C_ADDR    (7)
#define HAL_PHY_SYNC_WGC_DEF_DELAY_US    (50000)
#define HAL_PHY_SYNC_WGC_DEF_SYNC_US_I2C (40000)
#define HAL_PHY_SYNC_WGC_DEF_SYNC_US_SMI (100000)

/* MACRO FUNCTION DECLARATIONS
 */
#define HAL_PHY_DRIVER_INT_MAP_VECTOR_ENTRY(i) (&_hal_phy_driver_int_func_vector[i])

#define HAL_PHY_DRIVER_EXT_MAP_VECTOR_ENTRY(i) (&_hal_phy_driver_ext_func_vector[i])

#define HAL_PHY_CB_CONTEXT(__unit__, __port__) (_ext_ptr_phy_cb[__unit__][__port__])

#define HAL_PHY_CB_DRIVER_INT_FUN_VEC(__unit__, __port__)                    \
    HAL_PHY_CB_CONTEXT(__unit__, __port__).driver[HAL_PHY_PHY_TYPE_INTERNAL]

#define HAL_PHY_CB_DRIVER_EXT_FUN_VEC(__unit__, __port__)                    \
    HAL_PHY_CB_CONTEXT(__unit__, __port__).driver[HAL_PHY_PHY_TYPE_EXTERNAL]

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
DIAG_SET_MODULE_INFO(AIR_MODULE_HAL, "hal_phy.c");

HAL_PHY_CB_T               *_ext_ptr_phy_cb[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
static HAL_PHY_DRIVER_MAP_T _hal_phy_driver_int_func_vector[] = {
#ifdef AIR_EN_SCORPIO
    {  HAL_SCO_DEVICE_ID_EN8851C,   hal_sco_serdes_getDriver},
    {  HAL_SCO_DEVICE_ID_EN8853C,   hal_sco_serdes_getDriver},
    {  HAL_SCO_DEVICE_ID_EN8860C,   hal_sco_serdes_getDriver},
#endif
#ifdef AIR_EN_PEARL
    {HAL_PEARL_DEVICE_ID_AN8855M, hal_pearl_serdes_getDriver},
    {HAL_PEARL_DEVICE_ID_AN8855H, hal_pearl_serdes_getDriver},
#endif
#ifdef AIR_EN_CORAL
    {HAL_CORAL_DEVICE_ID_AN8858C, hal_coral_serdes_getDriver},
    {HAL_CORAL_DEVICE_ID_AN8858D, hal_coral_serdes_getDriver},
    {HAL_CORAL_DEVICE_ID_AN8858H, hal_coral_serdes_getDriver},
    {HAL_CORAL_DEVICE_ID_AN8858B, hal_coral_serdes_getDriver},
#endif
};

static HAL_PHY_DRIVER_MAP_T _hal_phy_driver_ext_func_vector[] = {
#ifdef AIR_EN_EN8801S_PHY
    { HAL_PHY_DEVICE_ID_EN8801S,  hal_en8801s_phy_getDriver},
#endif
#ifdef AIR_EN_EN8808_PHY
    {  HAL_PHY_DEVICE_ID_EN8808,   hal_en8808_phy_getDriver},
#endif
#ifdef AIR_EN_EN8811H_PHY
    { HAL_PHY_DEVICE_ID_EN8811H,  hal_en8811h_phy_getDriver},
#endif
#ifdef AIR_EN_AN8801SB_PHY
    {HAL_PHY_DEVICE_ID_AN8801SB, hal_an8801sb_phy_getDriver},
#endif
#ifdef AIR_EN_AN8804_PHY
    {  HAL_PHY_DEVICE_ID_AN8804,   hal_an8804_phy_getDriver},
#endif
#ifdef AIR_EN_AN8858_PHY
    {  HAL_PHY_DEVICE_ID_AN8858,   hal_an8858_phy_getDriver},
#endif
#ifdef AIR_EN_AN8808_PHY
    { HAL_PHY_DEVICE_ID_AN8808Q,   hal_an8808_phy_getDriver},
    { HAL_PHY_DEVICE_ID_AN8808X,   hal_an8808_phy_getDriver},
#endif
#ifdef AIR_EN_AN8811B_PHY
    { HAL_PHY_DEVICE_ID_AN8811B,  hal_an8811b_phy_getDriver},
#endif
};

/* FUNCTION NAME:   _hal_phy_ext_probe
 * PURPOSE:
 *      This API is used to initialize PHY.
 * INPUT:
 *      unit        --  Device unit number
 *      port        --  Port number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_INITED
 *
 * NOTES:
 *      Both internal init function and external init function need to execute.
 *      And internal init need to execute first.
 */
static AIR_ERROR_NO_T
_hal_phy_initPhy(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc_int_init = AIR_E_OK;
    AIR_ERROR_NO_T rc_ext_init = AIR_E_OK;

    /* Internal PHY init func */
    rc_int_init = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_init, (unit, port));
    if ((AIR_E_OK != rc_int_init) && (AIR_E_NOT_SUPPORT != rc_int_init))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d internal init failed (%d)\n", port, rc_int_init);
        return rc_int_init;
    }

    /* External PHY init func */
    rc_ext_init = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_init, (unit, port));
    if ((AIR_E_OK != rc_ext_init) && (AIR_E_NOT_SUPPORT != rc_ext_init))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d external init failed (%d)\n", port, rc_ext_init);
        return rc_ext_init;
    }

    if ((AIR_E_NOT_SUPPORT == rc_int_init) && (AIR_E_NOT_SUPPORT == rc_ext_init))
    {
        return AIR_E_NOT_INITED;
    }

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_phy_ext_probe
 * PURPOSE:
 *      This API is used to hook external PHY driver.
 *      If user has customer init for this port, use its information to hook driver.
 * INPUT:
 *      unit        --  Device unit number
 *      port        --  Port number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_phy_ext_probe(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI16_T                i;
    HAL_PHY_DRIVER_MAP_T *ptr_entry = NULL;
    HAL_PHY_DRIVER_T     *ptr_phy_driver_vector = NULL;
    UI32_T                phy_id, param = 0;
    UI16_T                reg_id_msb = 0, reg_id_lsb = 0;

    /* Read address 2,3 by MDIO to get PHY ID */
    if (HAL_IS_GPHY_PORT_VALID(unit, port) && (!HAL_IS_DEVICE_SCO_FAMILY(unit)))
    {
        rc |= hal_mdio_readC22FromIntBusByPort(unit, port, 0, MII_PHYSID1, &reg_id_msb);
        rc |= hal_mdio_readC22FromIntBusByPort(unit, port, 0, MII_PHYSID2, &reg_id_lsb);
    }
    else
    {
        rc |= hal_mdio_readC22ByPort(unit, port, MII_PHYSID1, &reg_id_msb);
        rc |= hal_mdio_readC22ByPort(unit, port, MII_PHYSID2, &reg_id_lsb);
    }

    if (AIR_E_OK == rc)
    {
        HAL_PHY_PORT_DEV_REVISION_ID(unit, port) = BITS_OFF_R(reg_id_lsb, MII_PHY_REV_ID_OFFT, MII_PHY_REV_ID_LENG);
        phy_id = (reg_id_msb << 16) | (reg_id_lsb & BITS_RANGE(MII_PHY_LSB_ID_OFFT, MII_PHY_LSB_ID_LENG));

        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d: phy_id %x revision-id %x \n", port, phy_id,
                   HAL_PHY_PORT_DEV_REVISION_ID(unit, port));
        HAL_PHY_CB_CONTEXT(unit, port).phy_dev.phy_id = phy_id;

        /* Find external func vector by PHY ID */
        for (i = 0; i < sizeof(_hal_phy_driver_ext_func_vector) / sizeof(HAL_PHY_DRIVER_MAP_T); i++)
        {
            ptr_entry = HAL_PHY_DRIVER_EXT_MAP_VECTOR_ENTRY(i);

            if (phy_id == ptr_entry->phy_id)
            {
                if (NULL != ptr_entry->phy_driver_func)
                {
                    ptr_entry->phy_driver_func(&ptr_phy_driver_vector, param);
                }
                break;
            }
        }

        if (NULL == ptr_phy_driver_vector)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d external probe not found \n", port);
            rc = AIR_E_NOT_SUPPORT;
        }
        else
        {
            HAL_PHY_CB_CONTEXT(unit, port).driver[HAL_PHY_PHY_TYPE_EXTERNAL] = ptr_phy_driver_vector;
        }
    }
    else
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d mdio read failed(%d) \n", port, rc);
    }

    return rc;
}

/* FUNCTION NAME:   _hal_phy_int_probe
 * PURPOSE:
 *      This API is used to hook internal PHY driver.
 * INPUT:
 *      unit        --  Device unit number
 *      port        --  Port number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_phy_int_probe(
    const UI32_T unit,
    const UI32_T port)
{
    UI16_T                i;
    HAL_PHY_DRIVER_MAP_T *ptr_entry = NULL;
    HAL_PHY_DRIVER_T     *ptr_phy_driver_vector = NULL;
    UI32_T                param = 0;

    if (FALSE == HAL_IS_XSGMII_PORT_VALID(unit, port))
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d not belong to XSGMII port \n", port);
        return AIR_E_OK;
    }

    HAL_AIR_PORT_TO_SERDES_ID(unit, port, param);

    /* Find internal func vector by DEVICE ID */
    for (i = 0; i < sizeof(_hal_phy_driver_int_func_vector) / sizeof(HAL_PHY_DRIVER_MAP_T); i++)
    {
        ptr_entry = HAL_PHY_DRIVER_INT_MAP_VECTOR_ENTRY(i);

        if (HAL_DEVICE_CHIP_ID(unit) == ptr_entry->phy_id)
        {
            if (NULL != ptr_entry->phy_driver_func)
            {
                ptr_entry->phy_driver_func(&ptr_phy_driver_vector, param);
            }
            break;
        }
    }

    if (NULL == ptr_phy_driver_vector)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d internal probe not found \n", port);
        return AIR_E_NOT_SUPPORT;
    }

    HAL_PHY_CB_CONTEXT(unit, port).driver[HAL_PHY_PHY_TYPE_INTERNAL] = ptr_phy_driver_vector;

    return AIR_E_OK;
}

/* FUNCTION NAME:   _hal_phy_initCfg
 * PURPOSE:
 *      This API is used to update _hal_phy_cb[unit][port].phy_dev for type, i2c address, phy address
 * INPUT:
 *      unit        --  Device unit number
 *      port        --  Port number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_ENTRY_NOT_FOUND
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_phy_initCfg(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_CFG_VALUE_T phyAddr;

    /* get PHY address */
    osal_memset(&phyAddr, 0, sizeof(AIR_CFG_VALUE_T));
    phyAddr.value = 0;
    phyAddr.param0 = port;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_ADDRESS, &phyAddr);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d phy address not found(%d)\n", port, rc);
        return rc;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d mapping phy address %x\n", port, phyAddr.value);

    HAL_PHY_CB_CONTEXT(unit, port).phy_dev.phy_addr = phyAddr.value;

    osal_memset(&phyAddr, 0, sizeof(AIR_CFG_VALUE_T));
    phyAddr.value = 0;
    phyAddr.param0 = port;
    rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_ACCESS_TYPE, &phyAddr);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d phy access type not found(%d)\n", port, rc);
        return rc;
    }
    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d mapping phy access type %x\n", port, phyAddr.value);

    if (HAL_PHY_ACCESS_TYPE_I2C_MDIO == phyAddr.value)
    {
        osal_memset(&phyAddr, 0, sizeof(AIR_CFG_VALUE_T));
        phyAddr.value = 0;
        phyAddr.param0 = port;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_I2C_ADDRESS, &phyAddr);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d phy i2c address not found(%d)\n", port, rc);
            return rc;
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d mapping phy i2c address  %x\n", port, phyAddr.value);

        HAL_PHY_CB_CONTEXT(unit, port).phy_dev.access_type = HAL_PHY_ACCESS_TYPE_I2C_MDIO;
        HAL_PHY_CB_CONTEXT(unit, port).phy_dev.i2c_addr = phyAddr.value;

        osal_memset(&phyAddr, 0, sizeof(AIR_CFG_VALUE_T));
        phyAddr.value = 1;
        phyAddr.param0 = port;
        rc = hal_cfg_getValue(unit, AIR_CFG_TYPE_PHY_I2C_BUS_ID, &phyAddr);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d phy i2c bus id not found(%d)\n", port, rc);
            return rc;
        }
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d mapping phy i2c bus id  %x\n", port, phyAddr.value);
        HAL_PHY_CB_CONTEXT(unit, port).phy_dev.i2c_bus_id = phyAddr.value;
    }
    else
    {
        HAL_PHY_CB_CONTEXT(unit, port).phy_dev.access_type = HAL_PHY_ACCESS_TYPE_MDIO;
    }

    return rc;
}

/* FUNCTION NAME:   _hal_phy_initRsrc
 * PURPOSE:
 *      This API is used to allocate phy control block memory on _hal_phy_cb[unit] and init to be default.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_phy_initRsrc(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    HAL_PHY_CB_T  *ptr_phy_cb = NULL;

    ptr_phy_cb =
        (HAL_PHY_CB_T *)osal_alloc(sizeof(HAL_PHY_CB_T) * AIR_PORT_NUM, air_module_getModuleName(AIR_MODULE_PORT));

    if (NULL != ptr_phy_cb)
    {
        /* initialize phy control block as default value */
        osal_memset(ptr_phy_cb, 0x00, sizeof(HAL_PHY_CB_T) * AIR_PORT_NUM);
        /* assign phy control block pointer to global phy_cb */
        _ext_ptr_phy_cb[unit] = ptr_phy_cb;
    }
    else
    {
        rc = AIR_E_NO_MEMORY;
    }

    return rc;
}

/* FUNCTION NAME:   _hal_phy_deinitRsrc
 * PURPOSE:
 *      This API is used to release phy control block memory resource.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_hal_phy_deinitRsrc(
    const UI32_T unit)
{
    if (NULL != _ext_ptr_phy_cb[unit])
    {
        /* free the allocated memory */
        osal_free(_ext_ptr_phy_cb[unit]);
        _ext_ptr_phy_cb[unit] = NULL;
    }

    return AIR_E_OK;
}

/* EXPORTED SUBPROGRAM BODIES
 */

/* FUNCTION NAME:   hal_phy_init
 * PURPOSE:
 *      This API is used to init phy control block and init/probe each port.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_INITED
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_init(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    AIR_ERROR_NO_T rc_int_probe = AIR_E_OK;
    AIR_ERROR_NO_T rc_ext_probe = AIR_E_OK;
    UI32_T         port = 0;

    /* init phy control block */
    _hal_phy_initRsrc(unit);

    /* init mdio control block */
    hal_mdio_initRsrc(unit);

    AIR_PORT_FOREACH(HAL_PORT_BMP_ETH(unit), port)
    {
        /* init phy config from customer init mechanism */
        _hal_phy_initCfg(unit, port);

        /* internal phy probe */
        rc_int_probe = _hal_phy_int_probe(unit, port);
        /* external phy probe */
        rc_ext_probe = _hal_phy_ext_probe(unit, port);
        if ((AIR_E_OK != rc_int_probe) && (AIR_E_OK != rc_ext_probe))
        {
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d probe failed \n", port);
            return AIR_E_NOT_INITED;
        }

        /* internal phy init */
        rc = _hal_phy_initPhy(unit, port);
        if (AIR_E_OK != rc)
        {
            DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d init failed \n", port);
            return rc;
        }
    }

    /* Synchronize LED clock */
    rc = hal_phy_syncLedClock(unit, HAL_PORT_BMP_ETH(unit), 0);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: Synchronize LED clock failed \n");
        return rc;
    }

    /* Synchronize wave generator clock */
    hal_phy_syncWaveGenClock(unit, HAL_PORT_BMP_ETH(unit), 0);
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: Synchronize wave-generator clock failed \n");
        return rc;
    }

    return rc;
}

/* FUNCTION NAME:   hal_phy_deinit
 * PURPOSE:
 *      This API is used to deinit phy control block and deinit each port.
 * INPUT:
 *      unit        --  Device unit number
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_deinit(
    const UI32_T unit)
{
    /* To-do: If needed, deinit each port */

    /* Release MDIO control block resourece */
    hal_mdio_deinitRsrc(unit);

    /* Release PHY control block resourece */
    _hal_phy_deinitRsrc(unit);

    return AIR_E_OK;
}

/* FUNCTION NAME:   hal_phy_setAdminState
 * PURPOSE:
 *      This API is used to set port state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      state           --  Port state
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setAdminState(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_ADMIN_STATE_T state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setAdminState, (unit, port, state));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setAdminState, (unit, port, state));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set adminstate failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getAdminState
 * PURPOSE:
 *      This API is used to get port state.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_state       --  Port state
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getAdminState(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_ADMIN_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getAdminState, (unit, port, ptr_state));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getAdminState, (unit, port, ptr_state));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get adminstate failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get adminstate=%d \n", port, *ptr_state);
    return rc;
}

/* FUNCTION NAME:   hal_phy_setAutoNego
 * PURPOSE:
 *      This API is used to set port auto-negotiation.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      auto_nego       --  Auto-negotiation
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setAutoNego(
    const UI32_T              unit,
    const UI32_T              port,
    const HAL_PHY_AUTO_NEGO_T auto_nego)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setAutoNego, (unit, port, auto_nego));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setAutoNego, (unit, port, auto_nego));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set autoNego failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getAutoNego
 * PURPOSE:
 *      This API is used to get port auto-negotiation.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_auto_nego   --  Auto-negotiation
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getAutoNego(
    const UI32_T         unit,
    const UI32_T         port,
    HAL_PHY_AUTO_NEGO_T *ptr_auto_nego)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getAutoNego, (unit, port, ptr_auto_nego));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getAutoNego, (unit, port, ptr_auto_nego));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get autoNego failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get autoNego=%d \n", port, *ptr_auto_nego);
    return rc;
}

/* FUNCTION NAME:   hal_phy_setLocalAdvAbility
 * PURPOSE:
 *      This API is used to set port local advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLocalAdvAbility(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setLocalAdvAbility, (unit, port, ptr_adv));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setLocalAdvAbility, (unit, port, ptr_adv));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set local adv failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getLocalAdvAbility
 * PURPOSE:
 *      This API is used to get port local advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLocalAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getLocalAdvAbility, (unit, port, ptr_adv));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getLocalAdvAbility, (unit, port, ptr_adv));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get local adv failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get local adv=%d \n", port, ptr_adv->flags);
    return rc;
}

/* FUNCTION NAME:   hal_phy_getRemoteAdvAbility
 * PURPOSE:
 *      This API is used to get port remote advertisment ability.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_adv         --  Advertisement ability
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getRemoteAdvAbility(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_AN_ADV_T *ptr_adv)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getRemoteAdvAbility, (unit, port, ptr_adv));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getRemoteAdvAbility, (unit, port, ptr_adv));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get remote adv failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get remote adv=%d \n", port, ptr_adv->flags);
    return rc;
}

/* FUNCTION NAME:   hal_phy_setSpeed
 * PURPOSE:
 *      This API is used to set port speed.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      speed           --  Port speed
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setSpeed(
    const UI32_T          unit,
    const UI32_T          port,
    const HAL_PHY_SPEED_T speed)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setSpeed, (unit, port, speed));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setSpeed, (unit, port, speed));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set speed failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getSpeed
 * PURPOSE:
 *      This API is used to get port speed.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_speed       --  Port speed
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getSpeed(
    const UI32_T     unit,
    const UI32_T     port,
    HAL_PHY_SPEED_T *ptr_speed)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getSpeed, (unit, port, ptr_speed));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getSpeed, (unit, port, ptr_speed));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get speed failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get spd=%d \n", port, *ptr_speed);
    return rc;
}

/* FUNCTION NAME:   hal_phy_setDuplex
 * PURPOSE:
 *      This API is used to set port duplex.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      duplex          --  Port duplex
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setDuplex(
    const UI32_T           unit,
    const UI32_T           port,
    const HAL_PHY_DUPLEX_T duplex)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setDuplex, (unit, port, duplex));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setDuplex, (unit, port, duplex));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set duplex failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getDuplex
 * PURPOSE:
 *      This API is used to get port duplex.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_duplex      --  Port duplex
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getDuplex(
    const UI32_T      unit,
    const UI32_T      port,
    HAL_PHY_DUPLEX_T *ptr_duplex)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getDuplex, (unit, port, ptr_duplex));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getDuplex, (unit, port, ptr_duplex));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get duplex failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get dpx=%d \n", port, *ptr_duplex);
    return rc;
}

/* FUNCTION NAME:   hal_phy_getLinkStatus
 * PURPOSE:
 *      This API is used to get port link status.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_status      --  Link Status
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLinkStatus(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LINK_STATUS_T *ptr_status)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getLinkStatus, (unit, port, ptr_status));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getLinkStatus, (unit, port, ptr_status));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get link status failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get flags=%d, spd=%d, dpx=%d \n", port, ptr_status->flags,
               ptr_status->speed, ptr_status->duplex);
    return rc;
}

/* FUNCTION NAME:   hal_phy_setLoopBack
 * PURPOSE:
 *      This API is used to set the loop back configuration for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  HAL_PHY_LPBK_NEAR_END
 *                          HAL_PHY_LPBK_FAR_END
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    const BOOL_T         enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setLoopBack, (unit, port, lpbk_type, enable));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setLoopBack, (unit, port, lpbk_type, enable));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d type %d set loopback to %d failed (%d)\n", port, lpbk_type, enable, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getLoopBack
 * PURPOSE:
 *      This API is used to get the loop back configuration for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      lpbk_type       --  HAL_PHY_LPBK_NEAR_END
 *                          HAL_PHY_LPBK_FAR_END
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLoopBack(
    const UI32_T         unit,
    const UI32_T         port,
    const HAL_PHY_LPBK_T lpbk_type,
    BOOL_T              *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getLoopBack, (unit, port, lpbk_type, ptr_enable));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getLoopBack, (unit, port, lpbk_type, ptr_enable));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get loopback failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d type %d get state=%d \n", port, lpbk_type, (*ptr_enable));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setSmartSpeedDown
 * PURPOSE:
 *      This API is used to set port smart speed down feature for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ssd_mode        --  HAL_PHY_SSD_MODE_DISABLE
 *                          HAL_PHY_SSD_MODE_2T
 *                          HAL_PHY_SSD_MODE_3T
 *                          HAL_PHY_SSD_MODE_4T
 *                          HAL_PHY_SSD_MODE_5T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setSmartSpeedDown(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_SSD_MODE_T ssd_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setSmartSpeedDown, (unit, port, ssd_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setSmartSpeedDown, (unit, port, ssd_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set ssd mode to %d failed (%d)\n", port, ssd_mode, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getSmartSpeedDown
 * PURPOSE:
 *      This API is used to get port smart speed down setting for a specific port.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 * OUTPUT:
 *      ptr_ssd_mode    --  HAL_PHY_SSD_MODE_DISABLE
 *                          HAL_PHY_SSD_MODE_2T
 *                          HAL_PHY_SSD_MODE_3T
 *                          HAL_PHY_SSD_MODE_4T
 *                          HAL_PHY_SSD_MODE_5T
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getSmartSpeedDown(
    const UI32_T        unit,
    const UI32_T        port,
    HAL_PHY_SSD_MODE_T *ptr_ssd_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getSmartSpeedDown, (unit, port, ptr_ssd_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getSmartSpeedDown, (unit, port, ptr_ssd_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get smart speed down failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get mode=%d \n", port, (*ptr_ssd_mode));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setLedOnCtrl
 * PURPOSE:
 *      This API is used to set control of port LED.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setLedOnCtrl, (unit, port, led_id, enable));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setLedOnCtrl, (unit, port, led_id, enable));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d led %d set state to %d failed (%d)\n", port, led_id, enable, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getLedOnCtrl
 * PURPOSE:
 *      This API is used to get port LED control setting.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      led_id          --  LED ID
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getLedOnCtrl, (unit, port, led_id, ptr_enable));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getLedOnCtrl, (unit, port, led_id, ptr_enable));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d with led %d get failed (%d)\n", port, led_id, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d with led %d get state=%d \n", port, led_id, (*ptr_enable));
    return rc;
}

/* FUNCTION NAME:   hal_phy_testTxCompliance
 * PURPOSE:
 *      This API is used to set the Tx compliance mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      mode            --  bist mode
 *
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_testTxCompliance(
    const UI32_T                       unit,
    const UI32_T                       port,
    const HAL_PHY_TX_COMPLIANCE_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);
    HAL_CHECK_ENUM_RANGE(mode, HAL_PHY_TX_COMPLIANCE_MODE_LAST);

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_testTxCompliance, (unit, port, mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_testTxCompliance, (unit, port, mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set Tx Compliance mode %d failed (%d)\n", port, mode, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      combo_mode      --  HAL_PHY_COMBO_MODE_PHY
 *                          HAL_PHY_COMBO_MODE_SERDES
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_phy_setComboMode(
    const UI32_T               unit,
    const UI32_T               port,
    const HAL_PHY_COMBO_MODE_T combo_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setComboMode, (unit, port, combo_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setComboMode, (unit, port, combo_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set combo mode to %d failed (%d)\n", port, combo_mode, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port.
 *
 * INPUT:
 *      unit             --  Device ID
 *      port             --  Index of port number
 *
 * OUTPUT:
 *      ptr_combo_mode   --  HAL_PHY_COMBO_MODE_PHY
 *                           HAL_PHY_COMBO_MODE_SERDES
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_phy_getComboMode(
    const UI32_T          unit,
    const UI32_T          port,
    HAL_PHY_COMBO_MODE_T *ptr_combo_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getComboMode, (unit, port, ptr_combo_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getComboMode, (unit, port, ptr_combo_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get combo mode failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get combo mode=%d \n", port, (*ptr_combo_mode));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setSerdesMode
 * PURPOSE:
 *      This API is used to set serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      serdes_mode     --  Serdes mode
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setSerdesMode(
    const UI32_T                unit,
    const UI32_T                port,
    const HAL_PHY_SERDES_MODE_T serdes_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setSerdesMode, (unit, port, serdes_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setSerdesMode, (unit, port, serdes_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set serdes mode to %d failed (%d)\n", port, serdes_mode, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getSerdesMode
 * PURPOSE:
 *      This API is used to get serdes mode.
 * INPUT:
 *      unit            --  Device unit number
 *      port            --  Port number
 *      ptr_serdes_mode --  Serdes mode
 *
 * OUTPUT:
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getSerdesMode(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_SERDES_MODE_T *ptr_serdes_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getSerdesMode, (unit, port, ptr_serdes_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getSerdesMode, (unit, port, ptr_serdes_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get serdes mode failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get mode=%d \n", port, (*ptr_serdes_mode));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setLedCtrlMode
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      ctrl_mode       --  LED control mode enumeration type
 *                          HAL_PHY_LED_CTRL_MODE_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const HAL_PHY_LED_CTRL_MODE_T ctrl_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setLedCtrlMode, (unit, port, led_id, ctrl_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setLedCtrlMode, (unit, port, led_id, ctrl_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d led %d set led control mode to %d failed (%d)\n", port, led_id,
                   ctrl_mode, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      ptr_ctrl_mode   --  LED control enumeration type
 *                          HAL_PHY_LED_CTRL_MODE_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getLedCtrlMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    HAL_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getLedCtrlMode, (unit, port, led_id, ptr_ctrl_mode));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getLedCtrlMode, (unit, port, led_id, ptr_ctrl_mode));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get led control mode failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get led control mode =%d \n", port, (*ptr_ctrl_mode));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      state           --  LED force state
 *                          HAL_PHY_LED_STATE_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    const HAL_PHY_LED_STATE_T state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyLedForceState, (unit, port, led_id, state));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setPhyLedForceState, (unit, port, led_id, state));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d led %d set led state to %d failed (%d)\n", port, led_id, state, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      ptr_state       --  LED force state enumeration type
 *                          HAL_PHY_LED_STATE_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyLedForceState(
    const UI32_T         unit,
    const UI32_T         port,
    const UI32_T         led_id,
    HAL_PHY_LED_STATE_T *ptr_state)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyLedForceState, (unit, port, led_id, ptr_state));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getPhyLedForceState, (unit, port, led_id, ptr_state));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get led state failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get led state =%d \n", port, (*ptr_state));
    return rc;
}

/* FUNCTION NAME:   hal_phy_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 *      pattern         --  LED force pattern
 *                          HAL_PHY_LED_PATT_T
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    const HAL_PHY_LED_PATT_T pattern)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyLedForcePattCfg, (unit, port, led_id, pattern));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_setPhyLedForcePattCfg, (unit, port, led_id, pattern));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d led %d set led force pattern to %d failed (%d)\n", port, led_id,
                   pattern, rc);
    }
    return rc;
}

/* FUNCTION NAME:   hal_phy_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED id
 * OUTPUT:
 *      ptr_pattern     --  LED force pattern enumeration type
 *                          HAL_PHY_LED_PATT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyLedForcePattCfg(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        led_id,
    HAL_PHY_LED_PATT_T *ptr_pattern)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyLedForcePattCfg, (unit, port, led_id, ptr_pattern));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_getPhyLedForcePattCfg, (unit, port, led_id, ptr_pattern));
        if (AIR_E_NOT_SUPPORT == rc)
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: port %d not found external/internal function \n", port);
            return rc;
        }
    }

    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get led force pattern failed (%d)\n", port, rc);
        return rc;
    }

    DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: port %d get led force pattern =%d \n", port, (*ptr_pattern));
    return rc;
}

/* FUNCTION NAME: hal_phy_triggerCableTest
 * PURPOSE:
 *      Get cable status.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *      test_pair       --  Select test pair
 *                          AIR_PORT_CABLE_TEST_PAIR_A
 *                          AIR_PORT_CABLE_TEST_PAIR_B
 *                          AIR_PORT_CABLE_TEST_PAIR_C
 *                          AIR_PORT_CABLE_TEST_PAIR_D
 *                          AIR_PORT_CABLE_TEST_PAIR_ALL
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          AIR_PORT_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic in speed 1G only.
 */
AIR_ERROR_NO_T
hal_phy_triggerCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_PAIR_T  test_pair,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_triggerCableTest, (unit, port, test_pair, ptr_test_rslt));
    if ((AIR_E_OK != rc) && (AIR_E_TIMEOUT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, pair %d trigger cable test failed(%d)\n", port, test_pair, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getCableTestRawData
 * PURPOSE:
 *      Get cable ec training 4 pair raw date.
 *
 * INPUT:
 *      unit                --  Device ID
 *      port                --  Select port number
 *      test_pair           --  Select test pair
 *
 * OUTPUT:
 *      pptr_raw_data_all   --  Cable diagnostic raw information
 *
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 *
 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_phy_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getCableTestRawData, (unit, port, pptr_raw_data_all));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, cable test raw data get failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_setPhyLedGlbCfg
 * PURPOSE:
 *      Set LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyLedGlbCfg, (unit, port, ptr_glb_cfg));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, set PHY LED global configuration failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getPhyLedGlbCfg
 * PURPOSE:
 *      Get LED global configuration.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_glb_cfg              -- Global configuration
 *                                  HAL_PHY_LED_GLB_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyLedGlbCfg(
    const UI32_T           unit,
    const UI32_T           port,
    HAL_PHY_LED_GLB_CFG_T *ptr_glb_cfg)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyLedGlbCfg, (unit, port, ptr_glb_cfg));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, get PHY LED global configuration failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_setPhyLedBlkEvent
 * PURPOSE:
 *      Set LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 *      evt_flags                -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const UI32_T evt_flags)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyLedBlkEvent, (unit, port, led_id, evt_flags));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, set PHY LED blink evnets failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getPhyLedBlkEvent
 * PURPOSE:
 *      Get LED blinking event combination.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      led_id                   -- LED ID
 * OUTPUT:
 *      ptr_evt_flags            -- Blinking event combination
 *                                  Refer to HAL_PHY_LED_BLK_EVT_FLAGS_XXX
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyLedBlkEvent(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    UI32_T      *ptr_evt_flags)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyLedBlkEvent, (unit, port, led_id, ptr_evt_flags));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, get PHY LED blink evnets failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_setPhyLedDuration
 * PURPOSE:
 *      Set LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 *      time                     -- Duration time, unit: ms
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    const UI32_T                      time)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyLedDuration, (unit, port, mode, time));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, set PHY LED duration time failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getPhyLedDuration
 * PURPOSE:
 *      Get LED duration
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Duration mode
 *                                  HAL_PHY_LED_BLK_CTRL_MODE_T
 * OUTPUT:
 *      ptr_time                 -- Duration time, unit: ms
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyLedDuration(
    const UI32_T                      unit,
    const UI32_T                      port,
    const HAL_PHY_LED_BLK_CTRL_MODE_T mode,
    UI32_T                           *ptr_time)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyLedDuration, (unit, port, mode, ptr_time));
    if (AIR_E_OK != rc)
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, get PHY LED duration time failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_syncLedClock
 * PURPOSE:
 *      Synchronize LED clock
 * INPUT:
 *      unit                     -- Device ID
 *      port_bitmap              -- Configured port bitmap
 *                                  AIR_PORT_BITMAP_T
 *      delay                    -- Delay time(Unit:us)
 *                                  0: Default delay time
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_syncLedClock(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            delay)
{
    UI32_T                port = 0;
    UI32_T                led_id = 0;
    UI32_T                reg_val = 0;
    UI16_T                blk_duration[AIR_PORT_NUM] = {0};
    UI16_T                blk_event[HAL_PHY_LED_MAX_CNT][AIR_PORT_NUM];
    AIR_PORT_BITMAP_T     led_group_port_bitmap;
    HAL_PHY_LED_GLB_CFG_T led_glb_cfg;
#ifdef AIR_EN_AN8808_PHY
    AIR_PORT_BITMAP_T an8808_led_group_port_bitmap;

    AIR_PORT_BITMAP_CLEAR(an8808_led_group_port_bitmap);
#endif
    osal_memset(blk_event, 0, sizeof(blk_event));

    /* Parameters initialization */
    AIR_PORT_BITMAP_CLEAR(led_group_port_bitmap);

    /* Select only one port as configured port for each LED group */
    AIR_PORT_FOREACH(port_bitmap, port)
    {
        switch (HAL_PHY_PORT_DEV_PHY_ID(unit, port))
        {
#ifdef AIR_EN_EN8808_PHY
            case HAL_PHY_DEVICE_ID_EN8808:
                AIR_PORT_ADD(led_group_port_bitmap, port);
                port += 3;
                break;
#endif
#ifdef AIR_EN_EN8801S_PHY
            case HAL_PHY_DEVICE_ID_EN8801S:
                AIR_PORT_ADD(led_group_port_bitmap, port);
                break;
#endif
#ifdef AIR_EN_EN8811H_PHY
            case HAL_PHY_DEVICE_ID_EN8811H:
                AIR_PORT_ADD(led_group_port_bitmap, port);
                break;
#endif
#ifdef AIR_EN_AN8808_PHY
            case HAL_PHY_DEVICE_ID_AN8808Q:
            case HAL_PHY_DEVICE_ID_AN8808X:
                AIR_PORT_ADD(an8808_led_group_port_bitmap, port);
                port += 3;
                break;
#endif
            default:
                DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unit %u Port %u - Unregconized PHY ID(%X)\n", unit, port,
                           HAL_PHY_PORT_DEV_PHY_ID(unit, port));
                break;
        }
    }

    /* Load original LED configuration */
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        hal_phy_getPhyLedDuration(unit, port, HAL_PHY_LED_BLK_CTRL_MODE_BLK, &reg_val);
        blk_duration[port] = (UI16_T)reg_val;
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_getPhyLedBlkEvent(unit, port, led_id, &reg_val);
            blk_event[led_id][port] = (UI16_T)reg_val;
        }
    }

    /* Disable blinking event for all LED */
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_setPhyLedBlkEvent(unit, port, led_id, 0);
        }
    }

    /* Set BLINK duration with high frequency for all LED */
    led_glb_cfg.flags = HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD + HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN +
                        HAL_PHY_LED_GLB_CFG_FLAGS_FAST_CLOCK;
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        hal_phy_setPhyLedGlbCfg(unit, port, &led_glb_cfg);
        hal_phy_setPhyLedDuration(unit, port, HAL_PHY_LED_BLK_CTRL_MODE_BLK, 1);
    }

    /* Delay to make sure LED clock reloaded */
    if (delay)
    {
        osal_delayUs(delay);
    }
    else
    {
        osal_delayUs(HAL_PHY_LED_CLOCK_TICK_US * HAL_PHY_LED_FAST_DURATION);
    }

    /* Hold LED clock */
    led_glb_cfg.flags = HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD;
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        hal_phy_setPhyLedGlbCfg(unit, port, &led_glb_cfg);
    }

    /* Set BLINK duration with original frequency for all LED */
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        /* Recovery blinking duration for all LED */
        hal_phy_setPhyLedDuration(unit, port, HAL_PHY_LED_BLK_CTRL_MODE_BLK, (UI32_T)blk_duration[port]);
        /* Recovery blinking event for all LED */
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_setPhyLedBlkEvent(unit, port, led_id, (UI32_T)blk_event[led_id][port]);
        }
    }

    /* Release LED clock */
    led_glb_cfg.flags = HAL_PHY_LED_GLB_CFG_FLAGS_ENHANCE_MOD + HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN;
    AIR_PORT_FOREACH(led_group_port_bitmap, port)
    {
        hal_phy_setPhyLedGlbCfg(unit, port, &led_glb_cfg);
    }

#ifdef AIR_EN_AN8808_PHY
    /* AN8808 LED reset clock */
    led_glb_cfg.flags = 0;
    AIR_PORT_FOREACH(an8808_led_group_port_bitmap, port)
    {
        hal_phy_setPhyLedGlbCfg(unit, port, &led_glb_cfg);
    }
    led_glb_cfg.flags = HAL_PHY_LED_GLB_CFG_FLAGS_CLOCK_EN;
    AIR_PORT_FOREACH(an8808_led_group_port_bitmap, port)
    {
        hal_phy_setPhyLedGlbCfg(unit, port, &led_glb_cfg);
    }
#endif
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_phy_syncWaveGenClock
 * PURPOSE:
 *      Synchronize wave generator clock
 * INPUT:
 *      unit                     -- Device ID
 *      port_bitmap              -- Configured port bitmap
 *                                  AIR_PORT_BITMAP_T
 *      delay                    -- Delay time(Unit:us)
 *                                  0: Default delay time
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_syncWaveGenClock(
    const UI32_T            unit,
    const AIR_PORT_BITMAP_T port_bitmap,
    const UI32_T            delay)
{
    UI32_T             port = 0, async_port = 0;
    UI32_T             led_id = 0;
    UI32_T             led_sync_delay = HAL_PHY_SYNC_WGC_DEF_SYNC_US_I2C;
    UI8_T              i2c_addr, i2c_init_idx, i2c_init_map[HAL_PHY_SYNC_WGC_MAX_I2C_ADDR];
    HAL_PHY_LED_PATT_T wave_gen_period[HAL_PHY_LED_MAX_CNT][AIR_PORT_NUM];
    AIR_PORT_BITMAP_T  wave_gen_port_bitmap;

    osal_memset(wave_gen_period, 0, sizeof(wave_gen_period));

    /* Parameters initialization */
    AIR_PORT_BITMAP_CLEAR(wave_gen_port_bitmap);

    /* Select only one port as configured port for each LED group */
    osal_memset(i2c_init_map, HAL_PHY_SYNC_WGC_UNINIT_ADDR, HAL_PHY_SYNC_WGC_MAX_I2C_ADDR);
    AIR_PORT_FOREACH(port_bitmap, port)
    {
        i2c_addr = HAL_PHY_PORT_DEV_I2C_ADDR(unit, port);
#ifdef AIR_EN_AN8808_PHY
        if ((HAL_PHY_DEVICE_ID_AN8808Q == HAL_PHY_CB_CONTEXT(unit, port).phy_dev.phy_id) ||
            (HAL_PHY_DEVICE_ID_AN8808X == HAL_PHY_CB_CONTEXT(unit, port).phy_dev.phy_id))
        {
            i2c_addr = HAL_PHY_PORT_DEV_PHY_ADDR(0, port);
            /* use fake i2c address to fit with below process */
            i2c_addr = ((i2c_addr - 1) & ~0x07) + 0x01;
            led_sync_delay = HAL_PHY_SYNC_WGC_DEF_SYNC_US_SMI;
        }
#endif
        for (i2c_init_idx = 0; i2c_init_idx < HAL_PHY_SYNC_WGC_MAX_I2C_ADDR; i2c_init_idx++)
        {
            if (i2c_addr == i2c_init_map[i2c_init_idx])
            {
                /* Log i2c_addr as inited if it had been record */
                i2c_addr = HAL_PHY_SYNC_WGC_INITED_ADDR;
                break;
            }
            else if (HAL_PHY_SYNC_WGC_UNINIT_ADDR == i2c_init_map[i2c_init_idx])
            {
                /* Record i2c_addr into init_map if found the un-init filed */
                i2c_init_map[i2c_init_idx] = i2c_addr;
                switch (HAL_PHY_PORT_DEV_PHY_ID(unit, port))
                {
#ifdef AIR_EN_EN8808_PHY
                    case HAL_PHY_DEVICE_ID_EN8808:
                        AIR_PORT_ADD(wave_gen_port_bitmap, port);
                        break;
#endif
#ifdef AIR_EN_EN8801S_PHY
                    case HAL_PHY_DEVICE_ID_EN8801S:
                        break;
#endif
#ifdef AIR_EN_EN8811H_PHY
                    case HAL_PHY_DEVICE_ID_EN8811H:
                        break;
#endif
#ifdef AIR_EN_AN8801SB_PHY
                    case HAL_PHY_DEVICE_ID_AN8801SB:
                        break;
#endif
#ifdef AIR_EN_AN8808_PHY
                    case HAL_PHY_DEVICE_ID_AN8808Q:
                    case HAL_PHY_DEVICE_ID_AN8808X:
                        AIR_PORT_ADD(wave_gen_port_bitmap, port);
                        break;
#endif
                    default:
                        DIAG_PRINT(HAL_DBG_INFO, "[Dbg]: Unit %u Port %u - Unregconized PHY ID(%X)\n", unit, port,
                                   HAL_PHY_PORT_DEV_PHY_ID(unit, port));
                        break;
                }

                break;
            }
            /* the else case of i2c_init_map[] means this element has been used by other chip */
        }
        if (!CMLIB_BITMAP_BIT_CHK(wave_gen_port_bitmap, port))
        {
            continue;
        }
        else if (HAL_PHY_SYNC_WGC_INITED_ADDR == i2c_addr)
        {
            /* skip following step if i2c_addr had been inited */
            continue;
        }
        else if (0 == i2c_addr)
        {
            /* the I2C address of master chip will read as 0,
             * this port should be synchronize at the end. */
            async_port = port;
        }
        else
        {
            DIAG_PRINT(HAL_DBG_WARN, "[Dbg]: Unit(%u) Port(%u) I2C-Addr(%X) is unexpected.\n", unit, port, i2c_addr);
            continue;
        }
    }

    /* Load original wave generator pattern */
    AIR_PORT_FOREACH(wave_gen_port_bitmap, port)
    {
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_getPhyLedForcePattCfg(unit, port, led_id, &(wave_gen_period[led_id][port]));
        }
    }

    /* Speed up wave generator pattern for all LED */
    AIR_PORT_FOREACH(wave_gen_port_bitmap, port)
    {
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_setPhyLedForcePattCfg(unit, port, led_id, HAL_PHY_LED_PATT_HZ_ZERO);
        }
    }

    /* Wait a minute to make sure wave generator clock reloaded */
    osal_delayUs(HAL_PHY_SYNC_WGC_DEF_DELAY_US);
    /* Recovery wave generator pattern for all LED */
    if (async_port)
    {
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_setPhyLedForcePattCfg(unit, async_port, led_id, wave_gen_period[led_id][async_port]);
        }

        /* Delay for a while to synchronize with other ports */
        if (delay)
        {
            osal_delayUs(delay);
        }
        else
        {
            osal_delayUs(led_sync_delay);
        }
    }

    AIR_PORT_FOREACH(wave_gen_port_bitmap, port)
    {
        if (async_port == port)
        {
            continue;
        }
        for (led_id = 0; led_id < HAL_PHY_LED_MAX_CNT; led_id++)
        {
            hal_phy_setPhyLedForcePattCfg(unit, port, led_id, wave_gen_period[led_id][port]);
        }
    }
    return AIR_E_OK;
}

/* FUNCTION NAME: hal_phy_setPhyOpMode
 * PURPOSE:
 *      Set Phy operation mode.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      mode                     -- Phy operation mode
 *                                  AIR_PORT_OP_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_setPhyOpMode(
    const UI32_T            unit,
    const UI32_T            port,
    const HAL_PHY_OP_MODE_T mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_setPhyOpMode, (unit, port, mode));
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d set phy-op-mode failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_getPhyOpMode
 * PURPOSE:
 *      Get Phy operation mode.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *      ptr_mode                 -- Phy operation mode enumeration type
 *                                  AIR_PORT_OP_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_getPhyOpMode(
    const UI32_T       unit,
    const UI32_T       port,
    HAL_PHY_OP_MODE_T *ptr_mode)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_getPhyOpMode, (unit, port, ptr_mode));
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d get phy-op-mode failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_dumpPhyPara
 * PURPOSE:
 *      Dump Phy parameters.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_dumpPhyPara(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_dumpPhyPara, (unit, port));
    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d dump phy parameters failed (%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_triggerLinkDownCableTest
 * PURPOSE:
 *      Trigger to perform link down cable diagnosis.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_test_rslt   --  Cable diagnostic information
 *                          AIR_PORT_CABLE_TEST_RSLT_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Support cable diagnostic link down mode only.
 */
AIR_ERROR_NO_T
hal_phy_triggerLinkDownCableTest(
    const UI32_T               unit,
    const UI32_T               port,
    HAL_PHY_CABLE_TEST_RSLT_T *ptr_test_rslt)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_triggerLinkDownCableTest, (unit, port, ptr_test_rslt));
    if ((AIR_E_OK != rc) && (AIR_E_TIMEOUT != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d, link down trigger cable test failed(%d)\n", port, rc);
    }
    return rc;
}

/* FUNCTION NAME: hal_phy_dumpPortCnt
 * PURPOSE:
 *      Dump port deubg counter.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Phy type
 *      param                    -- Parameter
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 *      AIR_E_NOT_SUPPORT        -- Feature is not supported.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_dumpPortCnt(
    const UI32_T             unit,
    const UI32_T             port,
    const HAL_PHY_PHY_TYPE_T type,
    const UI32_T             param)

{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);

    if (HAL_PHY_PHY_TYPE_INTERNAL == type)
    {
        rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_dumpPortCnt, (unit, port, type, param));
    }
    else
    {
        rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_dumpPortCnt, (unit, port, type, param));
    }

    if ((AIR_E_OK != rc) && (AIR_E_NOT_SUPPORT != rc))
    {
        DIAG_PRINT(HAL_DBG_ERR, "[Dbg]: port %d dump serdes counter failed (%d)\n", port, rc);
    }

    return rc;
}

/* FUNCTION NAME: hal_phy_dumpDebugInfo
 * PURPOSE:
 *      Dump port deubg information.
 *
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_phy_dumpDebugInfo(
    const UI32_T unit,
    const UI32_T port)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_UNIT(unit);
    HAL_CHECK_ETH_PORT(unit, port);

    /* internal serdes */
    rc = HAL_PHY_INT_FUNC_CALL(unit, port, hal_phy_dumpDebugInfo, (unit, port));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("No internal serdes driver\n");
    }
    /* external phy */
    rc = HAL_PHY_EXT_FUNC_CALL(unit, port, hal_phy_dumpDebugInfo, (unit, port));
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("No external phy driver\n");
    }

    rc = AIR_E_OK;
    return rc;
}
