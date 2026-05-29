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

/* FILE NAME:   port_utils.h
 * PURPOSE:
 *      Declare the common interfaces for port.
 * NOTES:
 */

#ifndef PORT_UTILS_H
#define PORT_UTILS_H

/* INCLUDE FILE DECLARATIONS
 */
#include "mw_error.h"
#include "mw_types.h"
#include "mw_utils.h"
#include "mw_platform.h"
#include "air_port.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define PORT_SETTINGS_PORT_MODE_AN       (0)
#define PORT_SETTINGS_PORT_MODE_FORCE    (1)

#define PORT_SETTINGS_FLOW_CTRL_OFF      (0)
#define PORT_SETTINGS_FLOW_CTRL_ON       (1)

#define PORT_SETTINGS_FLOW_CTRL_FORCE    (1)
#define PORT_MODE_BASIC_PORT_TYPE_OFFSET           (6) /* port type bit7-6 */
#define PORT_MODE_XSGMII_TYPE_OFFSET               (4) /* XSGMII type bit5-4 */
#define PORT_MODE_COMBO_MODE_OFFSET                (3) /* COMBO mode bit3 */
#define PORT_MODE_SERDES_MODE_OFFSET               (0) /* SERDES mode bit2-0 */

#define PORT_MODE_BASIC_PORT_TYPE_BITMASK       (0x03) /* port type bit7-6 */
#define PORT_MODE_XSGMII_TYPE_BITMASK           (0x03) /* XSGMII type bit5-4 */
#define PORT_MODE_COMBO_MODE_BITMASK            (0x01) /* COMBO mode bit3 */
#define PORT_MODE_SERDES_MODE_BITMASK           (0x07) /* SERDES mode bit2-0 */

#define PORT_SETTINGS_RATE_DUPLEX_10HFDX        BIT(0)
#define PORT_SETTINGS_RATE_DUPLEX_10FUDX        BIT(1)
#define PORT_SETTINGS_RATE_DUPLEX_100HFDX       BIT(2)
#define PORT_SETTINGS_RATE_DUPLEX_100FUDX       BIT(3)
#define PORT_SETTINGS_RATE_DUPLEX_1000FUDX      BIT(4)
#define PORT_SETTINGS_RATE_DUPLEX_2500M         BIT(5)

#define PORT_SETTINGS_PORT_MODE_BITMASK         BIT(15)
#define PORT_SETTINGS_FLOW_CONTROL_BITMASK      BIT(14)
#define PORT_SETTINGS_FLOW_CONTROL_FORCE_BIT    BIT(13)

#define PORT_SETTINGS_SPEED_ABILITY_BITMASK    (PORT_SETTINGS_RATE_DUPLEX_10HFDX | \
                                                PORT_SETTINGS_RATE_DUPLEX_10FUDX | \
                                                PORT_SETTINGS_RATE_DUPLEX_100HFDX | \
                                                PORT_SETTINGS_RATE_DUPLEX_100FUDX | \
                                                PORT_SETTINGS_RATE_DUPLEX_1000FUDX | \
                                                PORT_SETTINGS_RATE_DUPLEX_2500M)

#define PORT_SETTINGS_AIR_SPEED_ABILITY_BITMASK    (AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX | \
                                                    AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX | \
                                                    AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX | \
                                                    AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX | \
                                                    AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX | \
                                                    AIR_PORT_PHY_AN_ADV_FLAGS_2500M)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* Align with AIR_PORT_SERDES_MODE_T. Value range: 3 bits => bit2-0 of port_mode. */
typedef enum
{
    PORT_DB_SERDES_MODE_SGMII,
    PORT_DB_SERDES_MODE_1000BASE_X,
    PORT_DB_SERDES_MODE_HSGMII,
    PORT_DB_SERDES_MODE_100BASE_FX,

    PORT_DB_SERDES_MODE_UNKNOWN = 0x07
} PORT_DB_SERDES_MODE_T;

/* Align with AIR_INIT_PORT_TYPE_T. Value range: 2 bits => bit7-6 of port_mode. */
typedef enum
{
    PORT_DB_BASIC_TYPE_BASET,
    PORT_DB_BASIC_TYPE_XSGMII,
    PORT_DB_BASIC_TYPE_CPU,

    PORT_DB_BASIC_TYPE_LAST
} PORT_DB_BASIC_TYPE_T;

/* Align with AIR_INIT_XSGMII_PORT_T.flags:
 * AIR_INIT_XSGMII_PORT_FLAGS_CPU
 * AIR_INIT_XSGMII_PORT_FLAGS_COMBO
 * Value range: 2 bits => bit5-4 of port_mode.
 */
typedef enum
{
    PORT_DB_XSGMII_TYPE_PHY = 0,
    PORT_DB_XSGMII_TYPE_CPU = 1,
    PORT_DB_XSGMII_TYPE_COMBO = 2,

    PORT_DB_XSGMII_TYPE_LAST
} PORT_DB_XSGMII_TYPE_T;

/* Align with AIR_PORT_COMBO_MODE_T. Value range: 1 bit => bit3 of port_mode. */
typedef enum
{
    PORT_DB_COMBO_MODE_PHY,
    PORT_DB_COMBO_MODE_SERDES,

    PORT_DB_COMBO_MODE_LAST
} PORT_DB_COMBO_MODE_T;

/* Port type */
typedef enum
{
    PORT_DB_TYPE_COPPER,
    PORT_DB_TYPE_SERDES,
    PORT_DB_TYPE_COMBO_COPPER,
    PORT_DB_TYPE_COMBO_SERDES,

    PORT_DB_TYPE_LAST
} PORT_DB_TYPE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
void
port_db_updateSerdesModeFlags(
    UI8_T *ptr_port_mode,
    UI8_T serdes_mode);

MW_ERROR_NO_T
port_checkPortSettingsValid(
    const UI16_T portSettings);

MW_ERROR_NO_T
port_genPortSettings(
    UI8_T portMode,
    UI16_T speedAbility,
    UI8_T flowCtrl,
    UI16_T *ptr_portSettings);

MW_ERROR_NO_T
port_parsePortSettings(
    const UI16_T portSettings,
    UI8_T *ptr_portMode,
    UI16_T *ptr_speedAbility,
    UI8_T *ptr_flowCtrl);

/* FUNCTION NAME:   port_correctPortMode
 * PURPOSE:
 *      Correct the speed for a specific port with a certain serdes_mode. It is
 *      needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the speed to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the speed corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctPortMode(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI8_T *ptr_portMode);

/* FUNCTION NAME:   port_correctSpeedAbility
 * PURPOSE:
 *      Correct the speed ability for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the speed ability to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the speed ability corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctSpeedAbility(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI16_T *ptr_speedAbility);

/* FUNCTION NAME:   port_correctFlowctrl
 * PURPOSE:
 *      Correct the flow control setting for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 *      ptr_speed        -- A pointer points to the flow control setting to be corrected
 * OUTPUT:
 *      ptr_speed        -- A pointer returns the flow control setting corrected
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void
port_correctFlowctrl(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI8_T *ptr_flowCtrl);

/* FUNCTION NAME:   port_correctFlowctrl
 * PURPOSE:
 *      Correct the port setting for a specific port with a certain serdes_mode.
 *      It is needed when serdes mode changes.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      serdes_mode      -- The serdes mode of the port
 * OUTPUT:
 *      ptr_portSettings -- A pointer returns the port setting corrected
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
port_correctPortSettings(
    const UI32_T unit,
    const UI32_T port,
    AIR_PORT_SERDES_MODE_T serdes_mode,
    UI16_T *ptr_portSettings);

/* Convert MW used speed ability to AIR used speed ability defined in AIR_PORT_PHY_AN_ADV_T. */
void
port_covertSpeedAbility(
    UI16_T *ptr_speedAbility);

/* FUNCTION NAME:   port_check25GCopperPort
 * PURPOSE:
 *      Check port is weather 2.5G speed or not.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             -  2.5G port
 *      FALSE            -  not 2.5G port
 *
 * NOTES:
 *      None
 */
UI8_T
port_check25GCopperPort(
    const UI32_T unit,
    const UI32_T port);

/* FUNCTION NAME:   port_correctPortSpeedDbCfg
 * PURPOSE:
 *      Correct DB default config for high speed port.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 * OUTPUT:
 *      None
 * RETURN:
 *      TRUE             -  The config changed for high speed port
 *      FALSE            -  The config not change
 *
 * NOTES:
 *      Exclude sfp port
 */
UI8_T
port_correctPortSpeedDbCfg(
    const UI32_T unit,
    const UI32_T port,
    UI16_T *ptr_cfg);

/* FUNCTION NAME:   port_getInterfaceShortName
 * PURPOSE:
 *      Get port short nanme.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      name_size        -- ptr_name size
 * OUTPUT:
 *      ptr_name         -- Short name
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      gi1/2.5gi1
 */
MW_ERROR_NO_T
port_getInterfaceShortName(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T name_size,
    C8_T  *ptr_name);

/* FUNCTION NAME:   port_getInterfaceFullName
 * PURPOSE:
 *      Get port short nanme.
 *
 * INPUT:
 *      unit             -- Device unit number
 *      port             -- Port index
 *      name_size        -- ptr_name size
 * OUTPUT:
 *      ptr_name         -- Short name
 * RETURN:
 *      MW_E_OK
 *      MW_E_BAD_PARAMETER
 *
 * NOTES:
 *      GigabitEthernet1/2.5GigabitEthernet
 */
MW_ERROR_NO_T
port_getInterfaceFullName(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T name_size,
    C8_T  *ptr_name);

/* FUNCTION NAME:   port_db_initPortMode
 * PURPOSE:
 *      Initialize port_oper_mode for all ports to DB.
 *
 * INPUT:
 *      unit                 -- Device unit number
 *      port_num             -- Port num for ptr_mode
 * OUTPUT:
 *      ptr_mode             -- Port mode
 * RETURN:
 *      MW_E_OK             -- Operation success
 *      MW_E_NO_MEMORY      -- Insufficient memory
 *      MW_E_BAD_PARAMETER  -- Parameter is wrong
 *      MW_E_OTHERS         -- Other errors
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_initPortMode(
    const UI32_T unit,
    const UI32_T port_num,
    UI8_T       *ptr_mode);

/* FUNCTION NAME:   port_db_ParsePortMode
 * PURPOSE:
 *      Parse port_oper_mode DB data.
 *
 * INPUT:
 *      port_mode              -- The port_oper_mode DB data
 * OUTPUT:
 *      ptr_basic_port_type    -- A pointer to the basic port type parsed from the DB data
 *      ptr_xsgmii_type        -- A pointer to XSGMII type parsed from the DB data
 *      ptr_combo_mode         -- A pointer to COMBO mode parsed from the DB data
 *      ptr_serdes_mode        -- A pointer to SERDES mode parsed from the DB data
 * RETURN:
 *      MW_E_OK               -- Operation success
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_parsePortMode(
    const UI8_T port_mode,
    PORT_DB_BASIC_TYPE_T *ptr_basic_port_type,
    PORT_DB_XSGMII_TYPE_T *ptr_xsgmii_type,
    PORT_DB_COMBO_MODE_T *ptr_combo_mode,
    PORT_DB_SERDES_MODE_T *ptr_serdes_mode);

/* FUNCTION NAME:   port_db_getPortType
 * PURPOSE:
 *      Extract port type from the port_oper_mode DB data.
 *
 * INPUT:
 *      unit                   -- Device unit number
 *      port                   -- Port number
 *      port_mode              -- The port_oper_mode DB data
 * OUTPUT:
 *      ptr_port_type          -- A pointer to port type parsed from the DB data
 * RETURN:
 *      MW_E_OK                -- Operation success
 *      MW_E_NOT_SUPPORT       -- Not supported
 *
 * NOTES:
 *      None.
 */
MW_ERROR_NO_T
port_db_getPortType(
    const UI32_T unit,
    const UI32_T port,
    const UI8_T port_mode,
    PORT_DB_TYPE_T *ptr_port_type);

#endif  /* End of PORT_UTILS_H */
