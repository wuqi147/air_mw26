/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:   sfp_config_customer.c
 * PURPOSE:
 *      Configure SFP.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_config_customer.h"
#include "sfp_sff_data.h"
#include "sfp_pin_sda.h"
#include "sfp_pin_io.h"
#include "mw_utils.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef struct
{
    UI8_T product_id;
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings;
    UI8_T port_count;
} SFP_CONFIG_PORT_PRODUCT_ID_MAP_T;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
#ifdef SFP_MODULE_WHITELIST_ENABLE
/* This is the whitelist of the SFP modules supported currently. The SFP modules
 * on the whitelist can be configured on the Port Settings webpage. The SFP modules
 * that are not on the whitelist are named unsupported SFP modules. However, they
 * may still work. But they cannot be configured on the Port Settings webpage.
 * Customers can add or delete any SFP module into or from the whitelist. If
 * customers want to add a new SFP module, customers need to make sure it can work
 * properly.
 */
static const SFP_CONFIG_WHITELIST_SETTINGS_T _sfp_config_whitelist_settings_array[] =
{
    {"FS",                                             /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "SFP-GB-GE-T",                                    /* Vendor PN */
     "F",                                              /* Vendor Rev */
     (SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV),    /* Compare mask */
     SFP_MODULE_AN_ON,                                  /* AN status */
     0,                                                 /* Rate */
     SFP_SFF_ACH_ADDRESS,                               /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                     /* Serdes mode */
    },

    {"OEM",                                            /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "WT-PD-G39-20-D",                                 /* Vendor PN */
     "",                                               /* Vendor Rev */
     SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN,      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                    /* Serdes mode */
    },

    {"OEM",                                            /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "WT-PD-G39-40L-D",                                /* Vendor PN */
     "",                                               /* Vendor Rev */
     SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN,      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                    /* Serdes mode */
    },

    {"OEM",                                            /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "WT-PD-G88-05-D",                                 /* Vendor PN */
     "",                                               /* Vendor Rev */
     SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN,      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                    /* Serdes mode */
    },

    {"OEM",                                            /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "SFP-FE-LX-SM1310",                               /* Vendor PN */
     "",                                               /* Vendor Rev */
     SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN,      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                    /* Serdes mode */
    },

    {"OEM",                                            /* Vendor name */
     {0x0, 0x0, 0x0},                                  /* Vendor OUI */
     "SFP-FE-LX-SM1550",                               /* Vendor PN */
     "",                                               /* Vendor Rev */
     SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN,      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_AUTO_DETERMINE                    /* Serdes mode */
    },

    {"ZyXEL",                                          /* Vendor name */
     {0x0, 0x0, 0xc5},                                 /* Vendor OUI */
     "SFP-1000T",                                      /* Vendor PN */
     "1.0",                                            /* Vendor Rev */
     (SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV),      /* Compare mask */
     SFP_MODULE_AN_ON,                                 /* AN status */
     0,                                                /* Rate */
     SFP_SFF_INVALID_ADDRESS,                          /* Inside PHY address */
     SFP_SERDES_MODE_FORCE_1000BASE_X                  /* Serdes mode */
    },

    {"OEM",                                             /* Vendor name */
     {0x0, 0x0, 0x0},                                   /* Vendor OUI */
     "SFP-1000BASE-T",                                  /* Vendor PN */
     "E",                                               /* Vendor Rev */
     (SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV),    /* Compare mask */
     SFP_MODULE_AN_ON,                                  /* AN status */
     0,                                                 /* Rate */
     SFP_SFF_INVALID_ADDRESS,                           /* Inside PHY address */
     SFP_SERDES_MODE_FORCE_1000BASE_X                   /* Serdes mode */
    },

    {"MEMLOGIC CORP",                                   /* Vendor name */
     {0x0, 0x0, 0x0},                                   /* Vendor OUI */
     "TL-SM331T",                                       /* Vendor PN */
     "    ",                                            /* Vendor Rev */
     (SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN |
      SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV),    /* Compare mask */
     SFP_MODULE_AN_ON,                                  /* AN status */
     0,                                                 /* Rate */
     SFP_SFF_INVALID_ADDRESS,                           /* Inside PHY address */
     SFP_SERDES_MODE_FORCE_1000BASE_X                   /* Serdes mode */
    },
};
#endif

/* The array for the SFP SDA PIN driver */
static const SFP_PIN_SDA_DRIVER_FUNC_T _sfp_pin_sda_driver[] = {
    {
        /* AN8858 Default
         */
        sfp_pin_sda_driver_init0,
        sfp_pin_sda_driver_switchCtrlToPort0,
        sfp_pin_sda_driver_write0,
        sfp_pin_sda_driver_read0,
        sfp_pin_sda_driver_writeC22ByPort0,
        sfp_pin_sda_driver_readC22ByPort0,
        sfp_pin_sda_driver_writeC45ByPort0,
        sfp_pin_sda_driver_readC45ByPort0
    },
};

/* The array for the SFP IO PIN driver */
static const SFP_PIN_IO_DRIVER_FUNC_T _sfp_pin_io_driver[] = {
    {
        /* AN8858 Default */
        sfp_pin_io_driver_init0,
        sfp_pin_io_driver_switchCtrlToPort0,
        sfp_pin_io_driver_getPIONum0,
        sfp_pin_io_driver_setValue0,
        sfp_pin_io_driver_getValue0
    },
};

/* The array for the HW parmeters based on the HW schematics. */
static const SFP_HW_PARAM_T _sfp_hw_param[] = {
    {
        /* AN8858 Default */
        0x0,  /* I2C channel */
        0x70, /* PCA9542A I2C address */
        {
            0x0,
            0x0,
            0x0,
            0x0
        }
    },
};

/* Configure the SFP ports or COMBO ports for a specific HW schematics.
 * If SFP_PORT_DRIVER_CUSTOMER is set for port_driver, sfp_pin_customer.c need
 * be implemented and all parameters following port_driver are interpreted by
 * sfp_pin_customer.c.
 */

/* AN8858C RFB example */
static const SFP_CONFIG_PORT_SETTINGS_T _sfp_config_port_settings_en8858c_rfb[] = {
/*   PORT_NUM  PORT_TYPE                  PORT_DRIVER               MOD_ABS                      Tx_Disable  Rx_LOS  Tx_Fault                 MUX_CHANNEL  SDA_PIN_Driver            IO_PIN_Driver            Addr */
    {9,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_SWABSPIN(9),  6,          8,      SFP_PIN_PIONUM_INVALID,  0x5,         &_sfp_pin_sda_driver[0],  &_sfp_pin_io_driver[0],  &_sfp_hw_param[0]},
    {10,       SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_SWABSPIN(10), 7,          9,      SFP_PIN_PIONUM_INVALID,  0x4,         &_sfp_pin_sda_driver[0],  &_sfp_pin_io_driver[0],  &_sfp_hw_param[0]},
};

/* AN8858C RFB example */
static const SFP_CONFIG_PORT_SETTINGS_T _sfp_config_port_settings_en8858c_rfb_auto[] = {
/*   PORT_NUM  PORT_TYPE                  PORT_DRIVER               MOD_ABS                 Tx_Disable  Rx_LOS  Tx_Fault                 MUX_CHANNEL                 SDA_PIN_Driver  IO_PIN_Driver            Addr */
    {9,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_INVALID, 6,          8,      SFP_PIN_PIONUM_INVALID,  SFP_PIN_MUXCHANNEL_INVALID, NULL,           &_sfp_pin_io_driver[0],  NULL},
    {10,       SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_INVALID, 7,          9,      SFP_PIN_PIONUM_INVALID,  SFP_PIN_MUXCHANNEL_INVALID, NULL,           &_sfp_pin_io_driver[0],  NULL},
};

/* AN8858B RFB example */
static const SFP_CONFIG_PORT_SETTINGS_T _sfp_config_port_settings_en8858b_rfb[] = {
/*   PORT_NUM  PORT_TYPE                  PORT_DRIVER               MOD_ABS                      Tx_Disable  Rx_LOS  Tx_Fault                 MUX_CHANNEL  SDA_PIN_Driver            IO_PIN_Driver            Addr */
    {7,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_SWABSPIN(7),  6,          8,      SFP_PIN_PIONUM_INVALID,  0x5,         &_sfp_pin_sda_driver[0],  &_sfp_pin_io_driver[0],  &_sfp_hw_param[0]},
    {8,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_SWABSPIN(8),  7,          9,      SFP_PIN_PIONUM_INVALID,  0x4,         &_sfp_pin_sda_driver[0],  &_sfp_pin_io_driver[0],  &_sfp_hw_param[0]},
};

/* AN8858B RFB example */
static const SFP_CONFIG_PORT_SETTINGS_T _sfp_config_port_settings_en8858b_rfb_auto[] = {
/*   PORT_NUM  PORT_TYPE                  PORT_DRIVER               MOD_ABS                 Tx_Disable  Rx_LOS  Tx_Fault                 MUX_CHANNEL                 SDA_PIN_Driver  IO_PIN_Driver            Addr */
    {7,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_INVALID, 6,          8,      SFP_PIN_PIONUM_INVALID,  SFP_PIN_MUXCHANNEL_INVALID, NULL,           &_sfp_pin_io_driver[0],  NULL},
    {8,        SFP_CONFIG_PORT_TYPE_SFP,  SFP_PORT_DRIVER_DEFAULT,  SFP_PIN_PIONUM_INVALID, 7,          9,      SFP_PIN_PIONUM_INVALID,  SFP_PIN_MUXCHANNEL_INVALID, NULL,           &_sfp_pin_io_driver[0],  NULL},
};

static const SFP_CONFIG_PORT_PRODUCT_ID_MAP_T _sfp_config_port_product_id_map[] = {
    {MW_PRODUCT_ID_AN8858C_8P_2SFP, _sfp_config_port_settings_en8858c_rfb, SFP_UTIL_GETARRAYSIZE(_sfp_config_port_settings_en8858c_rfb, SFP_CONFIG_PORT_SETTINGS_T)},
    {MW_PRODUCT_ID_AN8858C_8P_2SFP_A, _sfp_config_port_settings_en8858c_rfb_auto, SFP_UTIL_GETARRAYSIZE(_sfp_config_port_settings_en8858c_rfb_auto, SFP_CONFIG_PORT_SETTINGS_T)},
    {MW_PRODUCT_ID_AN8858B_6P_2SFP, _sfp_config_port_settings_en8858b_rfb, SFP_UTIL_GETARRAYSIZE(_sfp_config_port_settings_en8858b_rfb, SFP_CONFIG_PORT_SETTINGS_T)},
    {MW_PRODUCT_ID_AN8858B_6P_2SFP_A, _sfp_config_port_settings_en8858b_rfb_auto, SFP_UTIL_GETARRAYSIZE(_sfp_config_port_settings_en8858b_rfb_auto, SFP_CONFIG_PORT_SETTINGS_T)},
};

static const SFP_CONFIG_PORT_SETTINGS_T *_sfp_port_settings_array;
static UI8_T _sfp_port_settings_array_size;

/* LOCAL SUBPROGRAM BODIES
 */

/* EXPORTED SUBPROGRAM BODIES
 */
#ifdef SFP_MODULE_WHITELIST_ENABLE
/* FUNCTION NAME:   sfp_config_getWhitelistSettingsArray
 * PURPOSE:
 *      Get the whitelist settings array.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      A pointer which points to the whitelist settings array
 *
 * NOTES:
 *      None
 */
const SFP_CONFIG_WHITELIST_SETTINGS_T *
sfp_config_getWhitelistSettingsArray(
    void)
{
    return _sfp_config_whitelist_settings_array;
}

/* FUNCTION NAME:   sfp_config_getWhitelistSettingsArraySize
 * PURPOSE:
 *      Get the size of the whitelist settings array.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      The size of the whitelist settings array
 *
 * NOTES:
 *      None
 */
UI32_T
sfp_config_getWhitelistSettingsArraySize(
    void)
{
    return SFP_UTIL_GETARRAYSIZE(_sfp_config_whitelist_settings_array, SFP_CONFIG_WHITELIST_SETTINGS_T);
}
#endif

/* FUNCTION NAME:   sfp_config_getPortSettingsArray
 * PURPOSE:
 *      Get the port settings array.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      A pointer which points to the port settings array
 *
 * NOTES:
 *      None
 */
const SFP_CONFIG_PORT_SETTINGS_T *
sfp_config_getPortSettingsArray(
    void)
{
    static UI8_T inited = FALSE;

    if (FALSE == inited)
    {
        MW_PRODUCT_ID_T product_id = mw_get_productID();
        UI32_T i = 0, count = SFP_UTIL_GETARRAYSIZE(_sfp_config_port_product_id_map, SFP_CONFIG_PORT_PRODUCT_ID_MAP_T);

        inited = TRUE;
        for (; i < count; i++)
        {
            if (product_id == _sfp_config_port_product_id_map[i].product_id)
            {
                _sfp_port_settings_array = _sfp_config_port_product_id_map[i].ptr_portSettings;
                _sfp_port_settings_array_size = _sfp_config_port_product_id_map[i].port_count;
                SFP_LOG_INFO("Matched: map[%d]:0x%p port_count:%d product_id:%d", i, _sfp_port_settings_array, _sfp_port_settings_array_size, product_id);
                break;
            }
        }

        if (i >= count)
        {
            SFP_LOG_INFO("No SFP port");
        }
    }

    return _sfp_port_settings_array;
}

/* FUNCTION NAME:   sfp_config_getPortSettingsArraySize
 * PURPOSE:
 *      Get the size of the port settings array.
 *
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      The size of the port settings array
 *
 * NOTES:
 *      None
 */
UI32_T
sfp_config_getPortSettingsArraySize(
    void)
{
    if (NULL == _sfp_port_settings_array)
    {
        /* Init first */
        sfp_config_getPortSettingsArray();
    }

    return _sfp_port_settings_array_size;
}

/* FUNCTION NAME:   sfp_config_checkPortDriverSettings
 * PURPOSE:
 *      Check if the port settings for a port are valid or not.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port number
 * OUTPUT:
 *      None
 * RETURN:
 *      MW_E_OK             -- The port settings are valid
 *      MW_E_BAD_PARAMETER  -- Invalid parameter
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
sfp_config_checkPortDriverSettings(
    const UI32_T unit,
    const UI32_T port)
{
    const SFP_CONFIG_PORT_SETTINGS_T *ptr_portSettings = sfp_port_getPortSettings(unit, port);
    UI8_T *ptr_sdaDriverArrayEdge = (UI8_T *)(_sfp_pin_sda_driver + SFP_UTIL_GETARRAYSIZE(_sfp_pin_sda_driver, SFP_PIN_SDA_DRIVER_FUNC_T));
    UI8_T *ptr_ioDriverArrayEdge = (UI8_T *)(_sfp_pin_io_driver + SFP_UTIL_GETARRAYSIZE(_sfp_pin_io_driver, SFP_PIN_IO_DRIVER_FUNC_T));
    UI8_T *ptr_hwParamArrayEdge = (UI8_T *)(_sfp_hw_param + SFP_UTIL_GETARRAYSIZE(_sfp_hw_param, SFP_HW_PARAM_T));

    if (NULL == ptr_portSettings)
    {
        return MW_E_BAD_PARAMETER;
    }

    if (SFP_PORT_DRIVER_LAST <= ptr_portSettings->port_driver)
    {
        SFP_LOG_ERROR("Invalid port_driver for port:%d", port);
        return MW_E_BAD_PARAMETER;
    }
    else if (SFP_PORT_DRIVER_DEFAULT == ptr_portSettings->port_driver)
    {
        if (((ptr_portSettings->ptr_sdaPinDriver != NULL) &&
              (((UI8_T *)ptr_portSettings->ptr_sdaPinDriver < (UI8_T *)_sfp_pin_sda_driver) ||
              ((UI8_T *)ptr_portSettings->ptr_sdaPinDriver >= ptr_sdaDriverArrayEdge))) ||
             ((ptr_portSettings->ptr_ioPinDriver != NULL) &&
              (((UI8_T *)ptr_portSettings->ptr_ioPinDriver < (UI8_T *)_sfp_pin_io_driver) ||
              ((UI8_T *)ptr_portSettings->ptr_ioPinDriver >= ptr_ioDriverArrayEdge))) ||
             ((ptr_portSettings->ptr_hwParam != NULL) &&
              (((UI8_T *)ptr_portSettings->ptr_hwParam < (UI8_T *)_sfp_hw_param) ||
              ((UI8_T *)ptr_portSettings->ptr_hwParam >= ptr_hwParamArrayEdge))))
        {
            SFP_LOG_ERROR("Port settings error for port:%d SDA driver:0x%p IO driver:0x%p HW param:0x%p",
                      port, ptr_portSettings->ptr_sdaPinDriver, ptr_portSettings->ptr_ioPinDriver, ptr_portSettings->ptr_hwParam);
            return MW_E_BAD_PARAMETER;
        }
    }

    return MW_E_OK;
}

