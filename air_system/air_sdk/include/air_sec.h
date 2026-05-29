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

/* FILE NAME:  air_sec.h
 * PURPOSE:
 *      It provides SEC module API.
 *
 * NOTES:
 */

#ifndef AIR_SEC_H
#define AIR_SEC_H

/* INCLUDE FILE DECLARATIONS
 */

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
/* 802.1X field */
typedef enum
{
    AIR_SEC_8021X_MODE_MAC,
    AIR_SEC_8021X_MODE_PORT,
    AIR_SEC_8021X_MODE_LAST
} AIR_SEC_8021X_MODE_T;

typedef enum
{
    AIR_SEC_8021X_AUTH_AUTH,
    AIR_SEC_8021X_AUTH_UNAUTH,
    AIR_SEC_8021X_AUTH_LAST
} AIR_SEC_8021X_AUTH_T;

typedef enum
{
    /* Forwarding according to MAC table */
    AIR_SEC_8021X_FWD_MAC,

    /* Drop */
    AIR_SEC_8021X_FWD_DROP,

    /* Trap to CPU only */
    AIR_SEC_8021X_FWD_CPU,
    AIR_SEC_8021X_FWD_LAST
} AIR_SEC_8021X_FWD_T;

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
/* 802.1x global configurations */
typedef struct AIR_SEC_8021X_CFG_S
{
#define AIR_SEC_8021X_CFG_FLAGS_PB_PORT_MOVE (1U << 0)
    UI32_T              flags;

    /* 802.1x packet forwarding rule for authenticated frame */
    AIR_SEC_8021X_FWD_T auth_fwd;

    /* 802.1x packet forwarding rule for unauthenticated frame */
    AIR_SEC_8021X_FWD_T unauth_fwd;
} AIR_SEC_8021X_CFG_T;

/* 802.1x MAC-based configurations */
typedef struct AIR_SEC_8021X_MAC_CFG_S
{
#define AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_AUTH   (1U << 0)
#define AIR_SEC_8021X_MAC_CFG_FLAGS_PORT_MOVE_UNAUTH (1U << 1)
    UI32_T flags;
} AIR_SEC_8021X_MAC_CFG_T;

/* 802.1x Port-based configurations */
typedef struct AIR_SEC_8021X_PORT_CFG_S
{
    /* Rx port authorization */
    AIR_SEC_8021X_AUTH_T rx_auth;
} AIR_SEC_8021X_PORT_CFG_T;

/* Port security global control configurations */
typedef struct AIR_SEC_MAC_LIMIT_CFG_S
{
#define AIR_SEC_MAC_LIMIT_CFG_FLAGS_EXCLUDE_MGMT    (1U << 0)
#define AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_PORT_MOVE  (1U << 1)
#define AIR_SEC_MAC_LIMIT_CFG_FLAGS_DROP_SA_FULL    (1U << 2)
#define AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_PORT_MOVE (1U << 3)
#define AIR_SEC_MAC_LIMIT_CFG_FLAGS_TOCPU_SA_FULL   (1U << 4)
    UI32_T flags;
} AIR_SEC_MAC_LIMIT_CFG_T;

/* Port security port control configurations */
typedef struct AIR_SEC_MAC_LIMIT_PORT_CFG_S
{
#define AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LRN (1U << 0)
#define AIR_SEC_MAC_LIMIT_PORT_CFG_FLAGS_SA_LMT (1U << 1)
    UI32_T flags;

    /* Rx SA allowable learning limit count */
    UI32_T sa_lmt_cnt;
} AIR_SEC_MAC_LIMIT_PORT_CFG_T;

typedef enum
{
    AIR_SEC_STORM_TYPE_BC,
    AIR_SEC_STORM_TYPE_UUC,
    AIR_SEC_STORM_TYPE_UMC,
    AIR_SEC_STORM_TYPE_LAST
} AIR_SEC_STORM_TYPE_T;

typedef enum
{
    AIR_SEC_STORM_RATE_MODE_PPS,
    AIR_SEC_STORM_RATE_MODE_BPS,
    AIR_SEC_STORM_RATE_MODE_KBPS = AIR_SEC_STORM_RATE_MODE_BPS,
    AIR_SEC_STORM_RATE_MODE_LAST
} AIR_SEC_STORM_RATE_MODE_T;

typedef enum
{
    AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_INCLUDE = 0,
    AIR_SEC_STORM_CTRL_MODE_MGMT_FRAME_EXCLUDE,
    AIR_SEC_STORM_CTRL_MODE_LAST
} AIR_SEC_STORM_CTRL_MODE_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_sec_set8021xGlobalMode
 * PURPOSE:
 *      Set 802.1x authentication base on MAC/port.
 * INPUT:
 *      unit                     -- Device ID
 *      mode                     -- 802.1x authentication mode
 *                                  AIR_SEC_8021X_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_set8021xGlobalMode(
    const UI32_T               unit,
    const AIR_SEC_8021X_MODE_T mode);

/* FUNCTION NAME:   air_sec_get8021xGlobalMode
 * PURPOSE:
 *      Get 802.1x authentication base on MAC/port.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_mode                 -- 802.1x authentication mode
 *                                  AIR_SEC_8021X_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_get8021xGlobalMode(
    const UI32_T          unit,
    AIR_SEC_8021X_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_sec_set8021xGlobalCfg
 * PURPOSE:
 *      Set global configurations of 802.1x authentication.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_cfg                  -- Structure of global configuration
 *                                  AIR_SEC_8021X_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_set8021xGlobalCfg(
    const UI32_T               unit,
    const AIR_SEC_8021X_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_get8021xGlobalCfg
 * PURPOSE:
 *      Get global configurations of 802.1x authentication.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_cfg                  -- Structure of global configuration
 *                                  AIR_SEC_8021X_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_get8021xGlobalCfg(
    const UI32_T         unit,
    AIR_SEC_8021X_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_set8021xMacBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x MAC-based authentication.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_mac_cfg              -- MAC-based configuration
 *                                  AIR_SEC_8021X_MAC_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_set8021xMacBasedCfg(
    const UI32_T                   unit,
    const AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

/* FUNCTION NAME:   air_sec_get8021xMacBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x MAC-based authentication.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_mac_cfg              -- MAC-based configuration
 *                                  AIR_SEC_8021X_MAC_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_get8021xMacBasedCfg(
    const UI32_T             unit,
    AIR_SEC_8021X_MAC_CFG_T *ptr_mac_cfg);

/* FUNCTION NAME:   air_sec_set8021xPortBasedCfg
 * PURPOSE:
 *      Set configurations of 802.1x Port-based authentication.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      ptr_port_cfg             -- Port-based configuration
 *                                  AIR_SEC_8021X_PORT_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_set8021xPortBasedCfg(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

/* FUNCTION NAME:   air_sec_get8021xPortBasedCfg
 * PURPOSE:
 *      Get configurations of 802.1x Port-based authentication.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_port_cfg             -- Port-based configuration
 *                                  AIR_SEC_8021X_PORT_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_get8021xPortBasedCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_SEC_8021X_PORT_CFG_T *ptr_port_cfg);

/* FUNCTION NAME:   air_sec_setMacLimitGlobalMode
 * PURPOSE:
 *      Set MAC limit global mode.
 * INPUT:
 *      unit                     -- Device ID
 *      enable                   -- TRUE
 *                                  FALSE
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setMacLimitGlobalMode(
    const UI32_T unit,
    const BOOL_T enable);

/* FUNCTION NAME:   air_sec_getMacLimitGlobalMode
 * PURPOSE:
 *      Get MAC limit global mode.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_enable               -- TRUE
 *                                  FALSE
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getMacLimitGlobalMode(
    const UI32_T unit,
    BOOL_T      *ptr_enable);

/* FUNCTION NAME:   air_sec_setMacLimitGlobalCfg
 * PURPOSE:
 *      Set MAC limitation global configurations.
 * INPUT:
 *      unit                     -- Device ID
 *      ptr_cfg                  -- Global configuration
 *                                  AIR_SEC_MAC_LIMIT_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setMacLimitGlobalCfg(
    const UI32_T                   unit,
    const AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_getMacLimitGlobalCfg
 * PURPOSE:
 *      Get MAC limitation global configurations.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_cfg                  -- Global configuration
 *                                  AIR_SEC_MAC_LIMIT_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getMacLimitGlobalCfg(
    const UI32_T             unit,
    AIR_SEC_MAC_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_setMacLimitPortCfg
 * PURPOSE:
 *      Set MAC limitation port configurations.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      ptr_cfg                  -- Port configuration
 *                                  AIR_SEC_MAC_LIMIT_PORT_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setMacLimitPortCfg(
    const UI32_T                        unit,
    const UI32_T                        port,
    const AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_getMacLimitPortCfg
 * PURPOSE:
 *      Get MAC limitation port configurations.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 * OUTPUT:
 *      ptr_cfg                  -- Port configuration
 *                                  AIR_SEC_MAC_LIMIT_PORT_CFG_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getMacLimitPortCfg(
    const UI32_T                  unit,
    const UI32_T                  port,
    AIR_SEC_MAC_LIMIT_PORT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_sec_setPortStormCtrl
 * PURPOSE:
 *      Enable/Disable sec modules ctrl function.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Storm type
 *                                  AIR_SEC_STORM_TYPE_T
 *      enable                   -- TRUE
 *                                  FALSE
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    const BOOL_T               enable);

/* FUNCTION NAME:   air_sec_getPortStormCtrl
 * PURPOSE:
 *      Get sec modules ctrl function Enable/Disable.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Storm type
 *                                  AIR_SEC_STORM_TYPE_T
 * OUTPUT:
 *      ptr_enable               -- TRUE
 *                                  FALSE
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getPortStormCtrl(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    BOOL_T                    *ptr_enable);

/* FUNCTION NAME:   air_sec_setPortStormCtrlRate
 * PURPOSE:
 *      Set rate & mode of storm control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Storm type
 *                                  AIR_SEC_STORM_TYPE_T
 *      mode                     -- Storm rate limit mode
 *                                  AIR_SEC_STORM_RATE_MODE_T
 *      rate                     -- Storm control rate
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setPortStormCtrlRate(
    const UI32_T                    unit,
    const UI32_T                    port,
    const AIR_SEC_STORM_TYPE_T      type,
    const AIR_SEC_STORM_RATE_MODE_T mode,
    const UI32_T                    rate);

/* FUNCTION NAME:   air_sec_getPortStormCtrlRate
 * PURPOSE:
 *      Get rate & mode of storm control.
 * INPUT:
 *      unit                     -- Device ID
 *      port                     -- Port ID
 *      type                     -- Storm type
 *                                  AIR_SEC_STORM_TYPE_T
 * OUTPUT:
 *      ptr_mode                 -- Storm rate limit mode
 *                                  AIR_SEC_STORM_RATE_MODE_T
 *      ptr_rate                 -- Storm control rate
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getPortStormCtrlRate(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_SEC_STORM_TYPE_T type,
    AIR_SEC_STORM_RATE_MODE_T *ptr_mode,
    UI32_T                    *ptr_rate);

/* FUNCTION NAME:   air_sec_setStormCtrlMgmtMode
 * PURPOSE:
 *      Set Management mode of storm control.
 * INPUT:
 *      unit                     -- Device ID
 *      mode                     -- Configure include/exclude
 *                                  management frame
 *                                  AIR_SEC_STORM_CTRL_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_setStormCtrlMgmtMode(
    const UI32_T                    unit,
    const AIR_SEC_STORM_CTRL_MODE_T mode);

/* FUNCTION NAME:   air_sec_getStormCtrlMgmtMode
 * PURPOSE:
 *      Get Management mode of storm control.
 * INPUT:
 *      unit                     -- Device ID
 * OUTPUT:
 *      ptr_mode                 -- Configure include/exclude
 *                                  management frame
 *                                  AIR_SEC_STORM_CTRL_MODE_T
 * RETURN:
 *      AIR_E_OK                 -- Operation success.
 *      AIR_E_OTHERS             -- Other errors.
 *      AIR_E_BAD_PARAMETER      -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_sec_getStormCtrlMgmtMode(
    const UI32_T               unit,
    AIR_SEC_STORM_CTRL_MODE_T *ptr_mode);

#endif /* End of AIR_SEC_H */
