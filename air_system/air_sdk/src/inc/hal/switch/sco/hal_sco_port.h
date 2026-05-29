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

/* FILE NAME:  hal_sco_port.h
 * PURPOSE:
 *  Define Port module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_PORT_H
#define HAL_SCO_PORT_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_MAX_NUM_OF_PORTS        (29)
#define HAL_SCO_MAX_NUM_OF_GIGA_PORTS   (23)
#define HAL_SCO_SGMII_PORT_OFFSET_BEGIN (23)
#define HAL_SCO_SGMII_PORT_OFFSET_END   (27)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_port_init
 * PURPOSE:
 *      Inintialization functions of port.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_port_deinit
 * PURPOSE:
 *      Deinitialization of port.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_port_setSerdesMode
 * PURPOSE:
 *      Set tx ability of SGMII ports.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  AIR_PORT_SERDES_MODE_SGMII
 *                          AIR_PORT_SERDES_MODE_1000BASE_X
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Speed will change to 1000M full duplex, when serdes mode set to 1000BASE_X.
 */
AIR_ERROR_NO_T
hal_sco_port_setSerdesMode(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_SERDES_MODE_T mode);

/* FUNCTION NAME: hal_sco_port_getSerdesMode
 * PURPOSE:
 *      Get mode of sgmii ports.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_PORT_SERDES_MODE_SGMII
 *                          AIR_PORT_SERDES_MODE_1000BASE_X
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
hal_sco_port_getSerdesMode(
    const UI32_T            unit,
    const UI32_T            port,
    AIR_PORT_SERDES_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_sco_port_setPhyAutoNego
 * PURPOSE:
 *      Set the auto-negotiation mode for a specific port.(Auto or Forced)
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      auto_nego       --  AIR_PORT_PHY_AN_DISABLE
 *                          AIR_PORT_PHY_AN_ENABLE
 *                          AIR_PORT_PHY_AN_RESTART
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
hal_sco_port_setPhyAutoNego(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_PHY_AN_T auto_nego);

/* FUNCTION NAME: hal_sco_port_getPhyAutoNego
 * PURPOSE:
 *      Get the auto-negotiation mode for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_auto_nego   --  AIR_PORT_PHY_AN_DISABLE
 *                          AIR_PORT_PHY_AN_ENABLE
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyAutoNego(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_PHY_AN_T *ptr_auto_nego);

/* FUNCTION NAME: hal_sco_port_setPhyLocalAdvAbility
 * PURPOSE:
 *      Set the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      ptr_adv         --  Pointer of the AN advertisement setting
 *
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
hal_sco_port_setPhyLocalAdvAbility(
    const UI32_T                 unit,
    const UI32_T                 port,
    const AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME: hal_sco_port_getPhyLocalAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
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
hal_sco_port_getPhyLocalAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME: hal_sco_port_getPhyRemoteAdvAbility
 * PURPOSE:
 *      Get the auto-negotiation remote advertisement for a
 *      specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_adv         --  AN advertisement setting
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
hal_sco_port_getPhyRemoteAdvAbility(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_PHY_AN_ADV_T *ptr_adv);

/* FUNCTION NAME: hal_sco_port_setSpeed
 * PURPOSE:
 *      Set the speed for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      speed           --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setSpeed(
    const UI32_T           unit,
    const UI32_T           port,
    const AIR_PORT_SPEED_T speed);

/* FUNCTION NAME: hal_sco_port_getSpeed
 * PURPOSE:
 *      Get the speed for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_speed       --  AIR_PORT_SPEED_10M:  10Mbps
 *                          AIR_PORT_SPEED_100M: 100Mbps
 *                          AIR_PORT_SPEED_1000M:1Gbps
 *                          AIR_PORT_SPEED_2500M:2.5Gbps
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getSpeed(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_SPEED_T *ptr_speed);

/* FUNCTION NAME: hal_sco_port_setDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *      This setting is used on force mode only.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      duplex          --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setDuplex(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_DUPLEX_T duplex);

/* FUNCTION NAME: hal_sco_port_getDuplex
 * PURPOSE:
 *      Get the duplex for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_duplex      --  AIR_PORT_DUPLEX_HALF
 *                          AIR_PORT_DUPLEX_FULL
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getDuplex(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_DUPLEX_T *ptr_duplex);

/* FUNCTION NAME: hal_sco_port_setBackPressure
 * PURPOSE:
 *      Set the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      bckPres         --  FALSE:Disable
 *                          TRUE: Enable
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
hal_sco_port_setBackPressure(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T bckPres);

/* FUNCTION NAME: hal_sco_port_getBackPressure
 * PURPOSE:
 *      Get the back pressure configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_bckPres     --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getBackPressure(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_bckPres);

/* FUNCTION NAME: hal_sco_port_setFlowCtrl
 * PURPOSE:
 *      Set the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_PORT_DIR_TX
 *                          AIR_PORT_DIR_RX
 *                          AIR_PORT_DIR_BOTH
 *      fc_en           --  TRUE: Enable select port flow control
 *                          FALSE:Disable select port flow control
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    const BOOL_T         fc_en);

/* FUNCTION NAME: hal_sco_port_getFlowCtrl
 * PURPOSE:
 *      Get the flow control configuration for specific port.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_PORT_DIR_TX
 *                          AIR_PORT_DIR_RX
 * OUTPUT:
 *      ptr_fc_en       --  FALSE: Port flow control disable
 *                          TRUE: Port flow control enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getFlowCtrl(
    const UI32_T         unit,
    const UI32_T         port,
    const AIR_PORT_DIR_T dir,
    BOOL_T              *ptr_fc_en);

/* FUNCTION NAME: hal_sco_port_setPhyLoopBack
 * PURPOSE:
 *      Set the loop back configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      dir             --  AIR_PORT_LPBK_DIR_NEAR_END
 *                          AIR_PORT_LPBK_DIR_FAR_END
 *      lpBack          --  FALSE:Disable
 *                          TRUE: Enable
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
hal_sco_port_setPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    const BOOL_T              lpBack_en);

/* FUNCTION NAME: hal_sco_port_getPhyLoopBack
 * PURPOSE:
 *      Get the loop back configuration for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      dir             --  AIR_PORT_LPBK_DIR_NEAR_END
 *                          AIR_PORT_LPBK_DIR_FAR_END
 *
 * OUTPUT:
 *      ptr_lpBack      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyLoopBack(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_LPBK_DIR_T dir,
    BOOL_T                   *ptr_lpBack_en);

/* FUNCTION NAME: hal_sco_port_getPortStatus
 * PURPOSE:
 *      Get the physical link status for a specific port.
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
hal_sco_port_getPortStatus(
    const UI32_T       unit,
    const UI32_T       port,
    AIR_PORT_STATUS_T *ptr_ps);

/* FUNCTION NAME: hal_sco_port_setAdminState
 * PURPOSE:
 *      Set the port status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      state           --  TRUE: port enable
 *                          FALSE: port disable
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setAdminState(
    const UI32_T unit,
    const UI32_T port,
    const BOOL_T state);

/* FUNCTION NAME: hal_sco_port_getAdminState
 * PURPOSE:
 *      Get the port status for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_state       --  TRUE: port enable
 *                          FALSE: port disable
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getAdminState(
    const UI32_T unit,
    const UI32_T port,
    BOOL_T      *ptr_state);

/* FUNCTION NAME: hal_sco_port_setPhySmartSpeedDown
 * PURPOSE:
 *      Set Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      mode            --  AIR_PORT_SSD_MODE_DISABLE
 *                          AIR_PORT_SSD_MODE_2T
 *                          AIR_PORT_SSD_MODE_3T
 *                          AIR_PORT_SSD_MODE_4T
 *                          AIR_PORT_SSD_MODE_5T
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
hal_sco_port_setPhySmartSpeedDown(
    const UI32_T              unit,
    const UI32_T              port,
    const AIR_PORT_SSD_MODE_T mode);

/* FUNCTION NAME: hal_sco_port_getPhySmartSpeedDown
 * PURPOSE:
 *      Get Smart speed down feature for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_PORT_SSD_MODE_DISABLE
 *                          AIR_PORT_SSD_MODE_2T
 *                          AIR_PORT_SSD_MODE_3T
 *                          AIR_PORT_SSD_MODE_4T
 *                          AIR_PORT_SSD_MODE_5T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getPhySmartSpeedDown(
    const UI32_T         unit,
    const UI32_T         port,
    AIR_PORT_SSD_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_sco_port_setPortMatrix
 * PURPOSE:
 *      Set port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      port_bitmap     --  Matrix port bitmap
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_setPortMatrix(
    const UI32_T            unit,
    const UI32_T            port,
    const AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME: hal_sco_port_getPortMatrix
 * PURPOSE:
 *      Get port matrix from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      port_bitmap     --  Matrix port bitmap
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_getPortMatrix(
    const UI32_T      unit,
    const UI32_T      port,
    AIR_PORT_BITMAP_T port_bitmap);

/* FUNCTION NAME: hal_sco_port_setVlanMode
 * PURPOSE:
 *      Set port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      mode            --  Port vlan mode
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_setVlanMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_PORT_VLAN_MODE_T mode);

/* FUNCTION NAME: hal_sco_port_getVlanMode
 * PURPOSE:
 *      Get port-based vlan mechanism from the specified device.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *
 * OUTPUT:
 *      ptr_mode        --  Port vlan mode
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_OTHERS
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_sco_port_getVlanMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_PORT_VLAN_MODE_T *ptr_mode);

/* FUNCTION NAME:   hal_sco_port_setPhyLedOnCtrl
 * PURPOSE:
 *      Set LED state for a specific port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_setPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    const BOOL_T enable);

/* FUNCTION NAME:   hal_sco_port_getPhyLedOnCtrl
 * PURPOSE:
 *      Get LED state for a specific port.
 *
 * INPUT:
 *      unit            --  Unit id
 *      port            --  Port id
 *      led_id          --  LED ID
 *
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyLedOnCtrl(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T led_id,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_sco_port_setComboMode
 * PURPOSE:
 *      Set the combo mode to PHY or SERDES for a specific port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      combo_mode      --  AIR_PORT_COMBO_MODE_PHY
 *                          AIR_PORT_COMBO_MODE_SERDES
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
hal_sco_port_setComboMode(
    const UI32_T                unit,
    const UI32_T                port,
    const AIR_PORT_COMBO_MODE_T combo_mode);

/* FUNCTION NAME: hal_sco_port_getComboMode
 * PURPOSE:
 *      Get the combo mode for a specific port.
 *
 * INPUT:
 *      unit             --  Device ID
 *      port             --  Index of port number
 *
 * OUTPUT:
 *      ptr_combo_mode   --  AIR_PORT_COMBO_MODE_PHY
 *                           AIR_PORT_COMBO_MODE_SERDES
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      Not support on CPU port.
 */
AIR_ERROR_NO_T
hal_sco_port_getComboMode(
    const UI32_T           unit,
    const UI32_T           port,
    AIR_PORT_COMBO_MODE_T *ptr_combo_mode);

/* FUNCTION NAME:   hal_sco_port_setPhyLedCtrlMode
 * PURPOSE:
 *      Set led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Index of port number
 *      led_id          --  LED ID
 *      ctrl_mode       --  LED control mode enumeration type
 *                          AIR_PORT_PHY_LED_CTRL_MODE_T
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
hal_sco_port_setPhyLedCtrlMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       led_id,
    const AIR_PORT_PHY_LED_CTRL_MODE_T ctrl_mode);

/* FUNCTION NAME:   hal_sco_port_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode           -- LED control enumeration type
 *                                 AIR_PORT_PHY_LED_CTRL_MODE_T
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
hal_sco_port_getPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_sco_port_getPhyLedCtrlMode
 * PURPOSE:
 *      Get led control mode for a specific led of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_ctrl_mode           -- LED control enumeration type
 *                                 AIR_PORT_PHY_LED_CTRL_MODE_T
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
hal_sco_port_getPhyLedCtrlMode(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    AIR_PORT_PHY_LED_CTRL_MODE_T *ptr_ctrl_mode);

/* FUNCTION NAME:   hal_sco_port_setPhyLedForceState
 * PURPOSE:
 *      Set led force state of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      state                   -- LED force state
 *                                 AIR_PORT_PHY_LED_STATE_T
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
hal_sco_port_setPhyLedForceState(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   led_id,
    const AIR_PORT_PHY_LED_STATE_T state);

/* FUNCTION NAME:   hal_sco_port_getPhyLedForceState
 * PURPOSE:
 *      Get led force state of the port.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_state               -- LED control enumeration type
 *                                 AIR_PORT_PHY_LED_STATE_T
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
hal_sco_port_getPhyLedForceState(
    const UI32_T              unit,
    const UI32_T              port,
    const UI32_T              led_id,
    AIR_PORT_PHY_LED_STATE_T *ptr_state);

/* FUNCTION NAME:   hal_sco_port_setPhyLedForcePattCfg
 * PURPOSE:
 *      Set led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 *      state                   -- LED force pattern
 *                                 AIR_PORT_PHY_LED_PATT_T
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
hal_sco_port_setPhyLedForcePattCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  led_id,
    const AIR_PORT_PHY_LED_PATT_T pattern);

/* FUNCTION NAME:   hal_sco_port_getPhyLedForcePattCfg
 * PURPOSE:
 *      Get led force pattern.
 *
 * INPUT:
 *      unit                    -- Device ID
 *      port                    -- Index of port number
 *      led_id                  -- LED ID
 * OUTPUT:
 *      ptr_pattern             -- LED force pattern enumeration type
 *                                 AIR_PORT_PHY_LED_PATT_T
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
hal_sco_port_getPhyLedForcePattCfg(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             led_id,
    AIR_PORT_PHY_LED_PATT_T *ptr_pattern);

/* FUNCTION NAME: hal_sco_port_triggerCableTest
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
hal_sco_port_triggerCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_PAIR_T  test_pair,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

/* FUNCTION NAME: hal_sco_port_getCableTestRawData
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
 * NOTES:
 *      Support cable diagnostic dump pair information.
 */
AIR_ERROR_NO_T
hal_sco_port_getCableTestRawData(
    const UI32_T unit,
    const UI32_T port,
    UI32_T     **pptr_raw_data_all);

/* FUNCTION NAME:   hal_sco_port_setPhyOpMode
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
hal_sco_port_setPhyOpMode(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_PORT_OP_MODE_T mode);

/* FUNCTION NAME:   hal_sco_port_getPhyOpMode
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
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *      AIR_E_NOT_SUPPORT
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_port_getPhyOpMode(
    const UI32_T        unit,
    const UI32_T        port,
    AIR_PORT_OP_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_sco_port_triggerLinkDownCableTest
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
 *      Support cable diagnostic in link down mode only.
 */
AIR_ERROR_NO_T
hal_sco_port_triggerLinkDownCableTest(
    const UI32_T                unit,
    const UI32_T                port,
    AIR_PORT_CABLE_TEST_RSLT_T *ptr_test_rslt);

#endif /* end of HAL_SCO_PORT_H */
