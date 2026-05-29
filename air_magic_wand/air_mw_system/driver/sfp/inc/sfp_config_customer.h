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

/* FILE NAME:   sfp_config_customer.h
 * PURPOSE:
 *      Configure SFP.
 * NOTES:
 *
 */

#ifndef SFP_CONFIG_CUSTOMER_H
#define SFP_CONFIG_CUSTOMER_H

/* INCLUDE FILE DECLARATIONS
 */
#include "osal_type.h"
#include "sfp_pin_sda.h"
#include "sfp_pin_io.h"

/* NAMING CONSTANT DECLARATIONS
 */
/* Defined following SFF-8472-R12.4 */
/* The size of vendor name */
#define SFP_MODULE_VENDOR_NAME_SIZE    (16)
/* The size of vendor OUI */
#define SFP_MODULE_VENDOR_OUI_SIZE     (3)
/* The size of vendor PN */
#define SFP_MODULE_VENDOR_PN_SIZE      (16)
/* The size of vendor REV */
#define SFP_MODULE_VENDOR_REV_SIZE     (4)
/* The whitelist option */
#define SFP_MODULE_WHITELIST_ENABLE
#undef  SFP_MODULE_WHITELIST_RESTRICTION_ENABLE
#define SFP_AUTO_ADAPTION_SUPPORT_HSGMII

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#ifdef SFP_MODULE_WHITELIST_ENABLE
/* The bitmask to indicate the vendor information required in the whitelist check. */
typedef enum
{
    SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_NAME = 0x01,
    SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_OUI = 0x02,
    SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_PN = 0x04,
    SFP_MODULE_WHITELIST_COMPARE_MASK_VENDOR_REV = 0x08
} SFP_MODULE_WHITELIST_COMPARE_MASK_T;
#endif

/* The AN status for the system side. If the SFP module is on the whitelist, AN
 * status set for it will be configured accordingly.
 */
typedef enum
{
    SFP_MODULE_AN_OFF,
    SFP_MODULE_AN_ON,
    SFP_MODULE_AN_LAST
} SFP_MODULE_AN_T;

/* The port type for the SFP port or COMBO port configuration. */
typedef enum
{
    SFP_CONFIG_PORT_TYPE_SFP,
    SFP_CONFIG_PORT_TYPE_COMBO,
    SFP_CONFIG_PORT_TYPE_COMBO_SFP, /* Pure SFP port based on EN8804 */

    SFP_CONFIG_PORT_TYPE_LAST = 0xFF
} SFP_CONFIG_PORT_TYPE_T;

typedef enum
{
    SFP_PORT_DRIVER_PIN_SDA_NONE,
    SFP_PORT_DRIVER_PIN_SDA_I2C_PCA9542A = 0x01,  /* Default EN8851 */
    SFP_PORT_DRIVER_PIN_SDA_I2C_PCA9544A = 0x02,  /* Default EN8804 of EN8853 */

    SFP_PORT_DRIVER_PIN_SDA_CUSTOMER = 0xFFFFFFFF
} SFP_PORT_DRIVER_PIN_SDA_T;

typedef enum
{
    SFP_PORT_DRIVER_PIN_IO_NONE,
    SFP_PORT_DRIVER_PIN_IO_GPIO = 0x01,  /* Default EN8851 */
    SFP_PORT_DRIVER_PIN_IO_I2C_PCA9535 = 0x02,  /* Default EN8804 of EN8853 */

    SFP_PORT_DRIVER_PIN_IO_CUSTOMER = 0xFFFFFFFF
} SFP_PORT_DRIVER_PIN_IO_T;

typedef enum
{
    SFP_SERDES_MODE_AUTO_DETERMINE = 0,
    SFP_SERDES_MODE_FORCE_1000BASE_X  = 1,

    SFP_SERDES_MODE_LAST,
} SFP_SERDES_MODE_T;

#ifdef SFP_MODULE_WHITELIST_ENABLE
/* The information needed for an SFP module to be listed on the whitellist. */
typedef struct
{
    /* The vendor name of the SFP module. It is needed if vendor name bitmask
     * is set on compare_mask.
     */
    UI8_T vendor_name[SFP_MODULE_VENDOR_NAME_SIZE + 1];
    /* The vendor OUI of the SFP module. It is needed if vendor OUI bitmask is
     * set on compare_mask.
     */
    UI8_T vendor_oui[SFP_MODULE_VENDOR_OUI_SIZE];
    /* The vendor PN of the SFP module. It is needed if vendor PN bitmask is set
     * on compare_mask.
     */
    UI8_T vendor_pn[SFP_MODULE_VENDOR_PN_SIZE + 1];
    /* The vendor REV of the SFP module. It is needed if vendor REV bitmask is
     * set on compare_mask.
     */
    UI8_T vendor_rev[SFP_MODULE_VENDOR_REV_SIZE + 1];
    /* SFP_MODULE_WHITELIST_COMPARE_MASK_T. It indicates the vendor information
     * required in the whitelist check. At least one bitmask need be set.
     */
    UI8_T compare_mask;
    /* It is AN status for the system side. */
    SFP_MODULE_AN_T an_status;
    /* The unit is Mbps. It will be ignored if an_status is not SFP_MODULE_AN_ON. */
    UI16_T rate;
    /* The inside PHY address. If there is no inside PHY, SFP_SFF_INVALID_ADDRESS
     * need be used.
     */
    UI8_T phy_2wire_addr;
    /* The SERDES mode to be set. */
    SFP_SERDES_MODE_T serdes_mode;
} SFP_CONFIG_WHITELIST_SETTINGS_T;
#endif

/* The port driver type. */
typedef enum
{
    /* The port driver is chosen from _sfp_pin_sda_driver[] and _sfp_pin_io_driver[]. */
    SFP_PORT_DRIVER_DEFAULT,
    /* The port driver is implemented by sfp_pin_customer.c. */
    SFP_PORT_DRIVER_CUSTOMER,

    SFP_PORT_DRIVER_LAST
} SFP_PORT_DRIVER_T;

/* Config a SFP port or COMBO port. */
typedef struct
{
    /* The port number */
    UI8_T port;
    /* The port type. Please refer to SFP_CONFIG_PORT_TYPE_T. */
    UI8_T port_type;
    /* The port driver type. Please refer to SFP_PORT_DRIVER_T. */
    UI8_T port_driver;
    /* The I/O port number used by the MOD-DEF0(MOD_ABS) pin of the SFP connector. If the pin
     * is not used, set it to SFP_PIN_PIONUM_INVALID. */
    UI8_T mod_abs;
    /* The I/O port number used by the TX Disable pin of the SFP connector. If the pin is not
     * used, set it to SFP_PIN_PIONUM_INVALID. */
    UI8_T tx_disable;
    /* The I/O port number used by the LOS pin of the SFP connector. If the pin is not used,
     * set it to SFP_PIN_PIONUM_INVALID. */
    UI8_T rx_loss;
    /* The I/O port number used by the TX Fault pin of the SFP connector. If the pin is not
     * used, set it to SFP_PIN_PIONUM_INVALID. */
    UI8_T tx_fault;
    /* The channel of the I2C multiplexer that the port is used. If no I2C multiplexer is used,
     * set it to SFP_PIN_MUXCHANNEL_INVALID. */
    UI8_T mux_channel;
    /* The SDA pin driver used to access the SDA pin. It is selected from _sfp_pin_sda_driver[]
     * array. If SFP_PORT_DRIVER_CUSTOMER is configured for port_driver, it will be ignored. */
    const SFP_PIN_SDA_DRIVER_FUNC_T *ptr_sdaPinDriver;
    /* The I/O pin driver used to access the I/O pins. It is selected from _sfp_pin_io_driver[]
     * array. If SFP_PORT_DRIVER_CUSTOMER is configured for port_driver, it will be ignored. */
    const SFP_PIN_IO_DRIVER_FUNC_T *ptr_ioPinDriver;
    /* The HW related parameters used by the SDA pin driver and the I/O pin driver. It is selected
     * from _sfp_hw_param[] array. If SFP_PORT_DRIVER_CUSTOMER is configured for port_driver, it
     * will be ignored. */
    const SFP_HW_PARAM_T *ptr_hwParam;
} SFP_CONFIG_PORT_SETTINGS_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
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
    void);

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
    void);
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
    void);

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
    void);

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
    const UI32_T port);

const SFP_CONFIG_PORT_SETTINGS_T *
sfp_port_getPortSettings(
    const UI32_T unit,
    const UI32_T port);

#endif /*End of SFP_CONFIG_CUSTOMER_H */

