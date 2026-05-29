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

/* FILE NAME: air_qos.h
 * PURPOSE:
 *      Define the Quailty of Service function in AIR SDK.
 * NOTES:
 *       None
 */

#ifndef AIR_QOS_H
#define AIR_QOS_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define AIR_QOS_QUEUE_MAX_NUM      (8)
#define AIR_QOS_PRI_MAX_NUM        (8)
#define AIR_QOS_QUEUE_DSCP_MAX_NUM (64)
#define AIR_QOS_SHAPER_NOSETTING   (0xffffffff)

/* DATA TYPE DECLARATIONS
 */
/* Definition of QoS Trust Mode priority */

typedef struct AIR_RATE_LIMIT_S
{
#define AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS (1U << 0)
#define AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS  (1U << 1)
    UI32_T flags;

    /*
     * The limit cover up to 2.5G
     * Rate = CIR * 32kbps
     * Range = 0..80000
     */
    UI32_T ingress_cir;
    UI32_T egress_cir;

    /*
     * Bucket Size (Bytes)
     */
    UI32_T ingress_cbs;
    UI32_T egress_cbs;
} AIR_QOS_RATE_LIMIT_CFG_T;

typedef enum
{
    AIR_QOS_RATE_DIR_INGRESS,
    AIR_QOS_RATE_DIR_EGRESS,
    AIR_QOS_RATE_DIR_LAST
} AIR_QOS_RATE_DIR_T;

typedef struct AIR_QOS_SHAPER_CFG_S
{
    /*unit is kpbs*/
    UI32_T min_rate;
    UI32_T max_rate;
} AIR_QOS_SHAPER_CFG_T;

typedef struct AIR_QOS_SHAPER_WEIGHT_S
{
    UI32_T min_weight;
    UI32_T max_weight;
} AIR_QOS_SHAPER_WEIGHT_T;

typedef enum
{
    /* The packet priority is based on port's priority. */
    AIR_QOS_TRUST_MODE_PORT,

    /*
     * The precedence of packet priority is based on 802.1p,
     * then port's priority.
     */
    AIR_QOS_TRUST_MODE_1P_PORT,

    /*
     * The precedence of packet priority is based on DSCP,
     * then port's priority.
     */
    AIR_QOS_TRUST_MODE_DSCP_PORT,

    /*
     * The precedence of packet priority is based on DSCP, 802.1p,
     * then port's priority.
     */
    AIR_QOS_TRUST_MODE_DSCP_1P_PORT,

    /*
     * The precedence of packet priority is based on 802.1p, DSCP,
     * then port's priority.
     */
    AIR_QOS_TRUST_MODE_1P_DSCP_PORT,
    AIR_QOS_TRUST_MODE_LAST
} AIR_QOS_TRUST_MODE_T;

typedef enum
{
    AIR_QOS_SHAPER_MODE_SP,
    AIR_QOS_SHAPER_MODE_WRR,
    AIR_QOS_SHAPER_MODE_WFQ,
    AIR_QOS_SHAPER_MODE_DISABLE,
    AIR_QOS_SHAPER_MODE_LAST
} AIR_QOS_SHAPER_MODE_T;

typedef enum
{
    AIR_QOS_EXCEED_SHAPER_MODE_SP,
    AIR_QOS_EXCEED_SHAPER_MODE_DROP,
    AIR_QOS_EXCEED_SHAPER_MODE_DISABLE,
    AIR_QOS_EXCEED_SHAPER_MODE_LAST
} AIR_QOS_EXCEED_SHAPER_MODE_T;

typedef enum
{
    AIR_QOS_SCH_MODE_SP,
    AIR_QOS_SCH_MODE_WRR,
    AIR_QOS_SCH_MODE_WFQ,
    AIR_QOS_SCH_MODE_LAST,
} AIR_QOS_SCH_MODE_T;
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME:   air_qos_setRateLimitEnable
 * PURPOSE:
 *      Enable or disable port rate limit.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 *      dir                  -- AIR_QOS_RATE_DIR_T
 *      enable               -- TRUE: eanble rate limit
 *                              FALSE: disable rate limit
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             enable);

/* FUNCTION NAME:   air_qos_getRateLimitEnable
 * PURPOSE:
 *      Get port rate limit state.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 *      dir                  -- The direction of rate limit
 *                              AIR_QOS_RATE_DIR_T
 * OUTPUT:
 *      ptr_enable           -- TRUE: eanble rate limit
 *                              FALSE: disable rate limit
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_enable);

/* FUNCTION NAME:   air_qos_setRateLimitCfg
 * PURPOSE:
 *      Set per port rate limit.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 *      ptr_cfg              -- Setting value of Rate limit
 *                              AIR_QOS_RATE_LIMIT_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_qos_getRateLimitCfg
 * PURPOSE:
 *      Get per port rate limit.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 * OUTPUT:
 *      ptr_cfg              -- Setting value of Rate limit
 *                              AIR_QOS_RATE_LIMIT_CFG_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME:   air_qos_setRateLimitExcludeMgmt
 * PURPOSE:
 *      Exclude/Include management frames to rate limit control.
 * INPUT:
 *      unit                 -- Device unit number
 *      dir                  -- The direction of rate limit
 *                              AIR_QOS_RATE_DIR_T
 *      enable               -- TRUE: Exclude management frame
 *                              FALSE:Include management frame
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             enable);

/* FUNCTION NAME:   air_qos_getRateLimitExcludeMgmt
 * PURPOSE:
 *      Get rate limit control exclude/include management frames.
 * INPUT:
 *      unit                 -- Device unit number
 *      dir                  -- The direction of rate limit
 *                              AIR_QOS_RATE_DIR_T
 * OUTPUT:
 *      ptr_enable           -- TRUE: Exclude management frame
 *                              FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_enable);

/* FUNCTION NAME:   air_qos_setTrustMode
 * PURPOSE:
 *      Set qos trust mode value.
 * INPUT:
 *      unit                 -- Device unit number
 *      mode                 -- The value of Qos trust mode
 *                              AIR_QOS_TRUST_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setTrustMode(
    const UI32_T               unit,
    const AIR_QOS_TRUST_MODE_T mode);

/* FUNCTION NAME:   air_qos_getTrustMode
 * PURPOSE:
 *      Get qos trust mode value.
 * INPUT:
 *      unit                 -- Device unit number
 * OUTPUT:
 *      ptr_mode             -- The value of Qos trust mode
 *                              AIR_QOS_TRUST_MODE_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getTrustMode(
    const UI32_T          unit,
    AIR_QOS_TRUST_MODE_T *ptr_mode);

/* FUNCTION NAME:   air_qos_setPriToQueue
 * PURPOSE:
 *      Set qos pri to queue mapping.
 * INPUT:
 *      unit                 -- Device unit number
 *      pri                  -- Qos pri value
 *      queue                -- Qos Queue value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setPriToQueue(
    const UI32_T unit,
    const UI32_T pri,
    const UI32_T queue);

/* FUNCTION NAME:   air_qos_getPriToQueue
 * PURPOSE:
 *      Get qos pri to queue mapping.
 * INPUT:
 *      unit                 -- Device unit number
 *      pri                  -- Qos pri value
 * OUTPUT:
 *      ptr_queue            -- Qos pri mapping Queue value
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getPriToQueue(
    const UI32_T unit,
    const UI32_T pri,
    UI32_T      *ptr_queue);

/* FUNCTION NAME:   air_qos_setDscpToPri
 * PURPOSE:
 *      Set qos dscp to pri mapping.
 * INPUT:
 *      unit                 -- Device unit number
 *      dscp                 -- Qos dscp value
 *      pri                  -- Qos pri value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setDscpToPri(
    const UI32_T unit,
    const UI32_T dscp,
    const UI32_T pri);

/* FUNCTION NAME:   air_qos_getDscpToPri
 * PURPOSE:
 *      Get qos dscp to pri mapping.
 * INPUT:
 *      unit                 -- Device unit number
 *      dscp                 -- Qos dscp value
 * OUTPUT:
 *      ptr_pri              -- Qos dscp mapping pri value
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getDscpToPri(
    const UI32_T unit,
    const UI32_T dscp,
    UI32_T      *ptr_pri);

/* FUNCTION NAME:   air_qos_setScheduleMode
 * PURPOSE:
 *      Set schedule mode of a port queue.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port id
 *      queue                -- Queue id
 *      sch_mode             -- Type of schedule mode.
 *                              AIR_QOS_SCH_MODE_T
 *      weight               -- weight for WRR/WFQ
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      Weight default value is 1, only for WRR/WFQ mode
 */
AIR_ERROR_NO_T
air_qos_setScheduleMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             queue,
    const AIR_QOS_SCH_MODE_T sch_mode,
    const UI32_T             weight);

/* FUNCTION NAME:   air_qos_getScheduleMode
 * PURPOSE:
 *      Get schedule mode of a port queue.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Port id
 *      queue                -- Queue id
 * OUTPUT:
 *      ptr_sch_mode         -- Type of schedule mode.
 *                              AIR_QOS_SCH_MODE_T
 *      ptr_weight           -- weight for WRR/WFQ
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getScheduleMode(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        queue,
    AIR_QOS_SCH_MODE_T *ptr_sch_mode,
    UI32_T             *ptr_weight);

/* FUNCTION NAME:   air_qos_setPortPriority
 * PURPOSE:
 *      Set port based qos priority.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 *      pri                  -- Qos priority value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setPortPriority(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pri);

/* FUNCTION NAME:   air_qos_getPortPriority
 * PURPOSE:
 *      Get port based qos priority.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 * OUTPUT:
 *      ptr_pri              -- Qos priority value
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getPortPriority(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_pri);

/* FUNCTION NAME:   air_qos_setPortTrustMode
 * PURPOSE:
 *      Set QoS trust mode value of specific port.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 *      mode                 -- The value of trust mode
 *                              AIR_QOS_TRUST_MODE_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_setPortTrustMode(
    const UI32_T               unit,
    const UI32_T               port,
    const AIR_QOS_TRUST_MODE_T mode);

/* FUNCTION NAME:   air_qos_getPortTrustMode
 * PURPOSE:
 *      Get QoS trust mode value of specific port.
 * INPUT:
 *      unit                 -- Device unit number
 *      port                 -- Select port number
 * OUTPUT:
 *      ptr_mode             -- The value of trust mode
 *                              AIR_QOS_TRUST_MODE_T
 * RETURN:
 *      AIR_E_OK             -- Operation success.
 *      AIR_E_BAD_PARAMETER  -- Parameter is wrong.
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
air_qos_getPortTrustMode(
    const UI32_T          unit,
    const UI32_T          port,
    AIR_QOS_TRUST_MODE_T *ptr_mode);
#endif /* End of AIR_QOS_H */
