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

/* FILE NAME:  hal_pearl_sec.h
 * PURPOSE:
 *  Define SEC module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_PEARL_SEC_H
#define HAL_PEARL_SEC_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_sec.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_PEARL_SEC_MAX_BUSY_TIME (20)
#define HAL_PEARL_SEC_MAX_AGE_CNT   (BITS(0, (AAC_AGE_CNT_LENGTH - 1)))
#define HAL_PEARL_SEC_MAX_AGE_UNIT  (BITS(0, (AAC_AGE_UNIT_LENGTH - 1)))
#define HAL_PEARL_SEC_MAC_SET_NUM   (4)
#define HAL_PEARL_SEC_MAX_SIZE      (2048)

#define HAL_PEARL_SEC_STORMCTL_MAX_COUNT (255)

#define HAL_PEARL_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_64  (64)
#define HAL_PEARL_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_256 (256)
#define HAL_PEARL_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_1K  (1024)
#define HAL_PEARL_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_4K  (4096)
#define HAL_PEARL_SEC_STORMCTL_PPS_TOKEN_SIZE_UNIT_16K (16384)

#define HAL_PEARL_SEC_STORMCTL_BPS_TOKEN_SIZE_UNIT_64  (3 * 64)
#define HAL_PEARL_SEC_STORMCTL_BPS_TOKEN_SIZE_UNIT_256 (3 * 256)
#define HAL_PEARL_SEC_STORMCTL_BPS_TOKEN_SIZE_UNIT_1K  (3 * 1024)
#define HAL_PEARL_SEC_STORMCTL_BPS_TOKEN_SIZE_UNIT_4K  (3 * 4096)
#define HAL_PEARL_SEC_STORMCTL_BPS_TOKEN_SIZE_UNIT_16K (3 * 16384)

#define HAL_PEARL_SEC_STORMCTL_MAX_TOKEN_SIZE (0x3fffff)

#define HAL_SEC_STORM_RATE_MAX_VALUE (2500000)

#define HAL_PEARL_SEC_STORMCTL_DEFAULT_TOKEN_VAL      (0x4000)
#define HAL_PEARL_SEC_STORMCTL_DEFAULT_2P5G_TOKEN_VAL (0xa000)
#define HAL_PEARL_SEC_STORMCTL_TOKEN_125US            (8000)
#define HAL_PEARL_SEC_STORMCTL_RATE_CNT_LENGTH        (8)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef union HAL_PEARL_SEC_STORMCTL_STAT_S
{
    struct
    {
        UI32_T csr_strm_mode       : 1;
        UI32_T csr_strm_bc_inc     : 1;
        UI32_T csr_strm_mc_inc     : 1;
        UI32_T csr_strm_uc_inc     : 1;
        UI32_T csr_strm_drop       : 1;
        UI32_T csr_strm_perd       : 1;
        UI32_T csr_strm_knw_mc_inc : 1;
        UI32_T csr_strm_rsv0       : 1;
        UI32_T csr_strm_unit       : 3;
        UI32_T csr_strm_rsv1       : 21;
    } raw;
    UI32_T byte;
} HAL_PEARL_SEC_STORMCTL_STAT_T;

typedef union HAL_PEARL_SEC_STORMCTL_LINKSPEED_S
{
    struct
    {
        UI32_T csr_strm_10m  : 8;
        UI32_T csr_strm_100m : 8;
        UI32_T csr_strm_1g   : 8;
        UI32_T csr_strm_2p5g : 8;
    } raw;
    UI32_T byte;
} HAL_PEARL_SEC_STORMCTL_LINKSPEED_T;

typedef struct HAL_PEARL_SEC_STORMCTL_TOKENSIZE_S
{
    UI32_T csr_strm_bkt_sz_10;
    UI32_T csr_strm_bkt_sz_100;
    UI32_T csr_strm_bkt_sz_1000;
    UI32_T csr_strm_bkt_sz_2500;
} HAL_PEARL_SEC_STORMCTL_TOKENSIZE_T;

typedef enum HAL_PEARL_SEC_STORMCTL_PERIOD_S
{
    HAL_PEARL_SEC_STORMCTL_PERIOD_1S = 0,
    HAL_PEARL_SEC_STORMCTL_PERIOD_125US = 1,
    HAL_PEARL_SEC_STORMCTL_PERIOD_LAST
} HAL_PEARL_SEC_STORMCTL_PERIOD_T;

typedef enum HAL_PEARL_SEC_STORMCTL_UNIT_S
{
    HAL_PEARL_SEC_STORMCTL_UNIT_64,
    HAL_PEARL_SEC_STORMCTL_UNIT_256,
    HAL_PEARL_SEC_STORMCTL_UNIT_1000,
    HAL_PEARL_SEC_STORMCTL_UNIT_4000,
    HAL_PEARL_SEC_STORMCTL_UNIT_16000,
    HAL_PEARL_SEC_STORMCTL_UNIT_LAST
} HAL_PEARL_SEC_STORMCTL_UNIT_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: hal_pearl_sec_init
 * PURPOSE:
 *      Initialization of SEC MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_pearl_sec_deinit
 * PURPOSE:
 *      Deinitialization of SEC MAC table.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_pearl_sec_set8021xGlobalMode
 * PURPOSE:
 *      Set 802.1x authentication base on MAC/port.
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_SEC_8021X_MODE_MAC:   802.1x authentication base on MAC
 *                          AIR_SEC_8021X_MODE_PORT:  802.1x authentication base on Port
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
hal_pearl_sec_set8021xGlobalMode(
    const UI32_T               unit,
    const AIR_SEC_8021X_MODE_T mode);

/* FUNCTION NAME: hal_pearl_sec_get8021xGlobalMode
 * PURPOSE:
 *      Get 802.1x authentication base on MAC/port.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_8021X_MODE_MAC:   802.1x authentication base on MAC
 *                          AIR_SEC_8021X_MODE_PORT:  802.1x authentication base on Port
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_get8021xGlobalMode(
    const UI32_T          unit,
    AIR_SEC_8021X_MODE_T *ptr_mode);

/* FUNCTION NAME: hal_pearl_sec_set8021xGlobalCfg
 * PURPOSE:
 *      Set global configurations of 802.1x authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_cfg         --  Structure of global configuration.
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
hal_pearl_sec_set8021xGlobalCfg(
    const UI32_T               unit,
    const AIR_SEC_8021X_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_pearl_sec_get8021xGlobalCfg
 * PURPOSE:
 *      Get global configurations of 802.1x authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of global configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_get8021xGlobalCfg(
    const UI32_T         unit,
    AIR_SEC_8021X_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_pearl_sec_set8021xMacBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x MAC-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_mac_cfg     --  Structure of MAC-based configuration.
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
hal_pearl_sec_set8021xMacBasedCfg(
    const UI32_T                   unit,
    const AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

/* FUNCTION NAME: hal_pearl_sec_get8021xMacBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x MAC-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mac_cfg     --  Structure of MAC-based configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_get8021xMacBasedCfg(
    const UI32_T             unit,
    AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

/* FUNCTION NAME: hal_pearl_sec_set8021xPortBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x Port-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      ptr_port_cfg    --  Structure of Port-based configuration.
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
hal_pearl_sec_set8021xPortBasedCfg(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

/* FUNCTION NAME: hal_pearl_sec_get8021xPortBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x Port-based authentication.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_port_cfg --  Structure of Port-based configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_get8021xPortBasedCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

/* FUNCTION NAME: hal_pearl_sec_setMacLimitGlobalMode
 * PURPOSE:
 *      Set port security mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
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
hal_pearl_sec_setMacLimitGlobalMode(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME: hal_pearl_sec_getMacLimitGlobalMode
 * PURPOSE:
 *      Get port security mode.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_getMacLimitGlobalMode(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME: hal_pearl_sec_setMacLimitGlobalCfg
 * PURPOSE:
 *      Set MAC limitation global configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      ptr_cfg         --  Structure of global configuration.
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
hal_pearl_sec_setMacLimitGlobalCfg(
    const UI32_T                   unit,
    const AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_pearl_sec_getMacLimitGlobalCfg
 * PURPOSE:
 *      Get MAC limitation global configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of global configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_getMacLimitGlobalCfg(
    const UI32_T             unit,
    AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_pearl_sec_setMacLimitPortCfg
 * PURPOSE:
 *      Set MAC limitation port configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      ptr_cfg         --  Structure of port configuration.
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
hal_pearl_sec_setMacLimitPortCfg(
    const UI32_T                        unit,
    const UI32_T                        port,
    const AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_pearl_sec_getMacLimitPortCfg
 * PURPOSE:
 *      Get MAC limitation port configurations.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *
 * OUTPUT:
 *      ptr_cfg         --  Structure of port configuration.
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_pearl_sec_getMacLimitPortCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   hal_pearl_sec_setPortStormCtrl
 * PURPOSE:
 *      Enable/Disable sec modules ctrl function.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_UMC
 *                          AIR_SEC_STORM_TYPE_UUC
 *      enable          --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_setPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    const BOOL_T               enable);

/* FUNCTION NAME:   hal_pearl_sec_getPortStormCtrl
 * PURPOSE:
 *      Get sec modules ctrl function Enable/Disable.
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_UMC
 *                          AIR_SEC_STORM_TYPE_UUC
 * OUTPUT:
 *      ptr_enable      --  FALSE:Disable
 *                          TRUE: Enable
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_getPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    BOOL_T                    *ptr_enable);

/* FUNCTION NAME:   air_sec_setPortStormCtrlRate
 * PURPOSE:
 *      Set storm control rate of specific port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_MC
 *                          AIR_SEC_STORM_TYPE_UC
 *      mode            --  AIR_SEC_STORM_RATE_MODE_PPS
 *                          AIR_SEC_STORM_RATE_MODE_BPS
 *      rate            --  Storm control rate
 *                          (0 ~ 2500K, unit: packets or Kbps)
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_setPortStormCtrlRate(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_STORM_TYPE_T      type,
    const AIR_SEC_STORM_RATE_MODE_T mode,
    const UI32_T                    rate);

/* FUNCTION NAME:   air_sec_getPortStormCtrlRate
 * PURPOSE:
 *      Get storm control rate of specific port
 *
 * INPUT:
 *      unit            --  Device ID
 *      port            --  Port ID
 *      type            --  AIR_SEC_STORM_TYPE_BC
 *                          AIR_SEC_STORM_TYPE_MC
 *                          AIR_SEC_STORM_TYPE_UC
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_STORM_RATE_MODE_PPS
 *                          AIR_SEC_STORM_RATE_MODE_BPS
 *      ptr_rate        --  Storm control rate
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_getPortStormCtrlRate(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    AIR_SEC_STORM_RATE_MODE_T *ptr_mode,
    UI32_T                    *ptr_rate);

/* FUNCTION NAME:   hal_pearl_sec_setStormCtrlMgmtMode
 * PURPOSE:
 *      Set Management mode of storm control.
 *
 * INPUT:
 *      unit            --  Device ID
 *      mode            --  AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE
 *                          AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_setStormCtrlMgmtMode(
    const UI32_T                    unit,
    const AIR_SEC_STORM_CTRL_MODE_T mode);

/* FUNCTION NAME:   hal_pearl_sec_getStormCtrlMgmtMode
 * PURPOSE:
 *      Get Management mode of storm control.
 *
 * INPUT:
 *      unit            --  Device ID
 *
 * OUTPUT:
 *      ptr_mode        --  AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE
 *                          AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE
 * RETURN:
 *      AIR_E_OK                -- Successfully read the data.
 *      AIR_E_OTHERS            -- Operation failed.
 *      AIR_E_BAD_PARAMETER     -- Invalid parameter.
 *
 * NOTES:
 *      none
 */
AIR_ERROR_NO_T
hal_pearl_sec_getStormCtrlMgmtMode(
    const UI32_T               unit,
    AIR_SEC_STORM_CTRL_MODE_T *ptr_mode);

#endif /* end of HAL_PEARL_SEC_H */
