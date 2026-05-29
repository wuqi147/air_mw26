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

/* FILE NAME:  hal_sco_qos.h
 * PURPOSE:
 *  Define QOS module HAL function.
 *
 * NOTES:
 *
 */

#ifndef HAL_SCO_QOS_H
#define HAL_SCO_QOS_H

/* INCLUDE FILE DECLARTIONS
 */
#include <air_error.h>
#include <air_port.h>
#include <air_qos.h>
#include <air_types.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define HAL_SCO_QOS_MAX_TOKEN                          (128)
#define HAL_SCO_QOS_MAX_CIR                            (80001)
#define HAL_SCO_QOS_TOKEN_REFILL_BASE                  (16)
#define HAL_SCO_QOS_TOKEN_PERIOD_1_16MS                (3)
#define HAL_SCO_QOS_TOKEN_PERIOD_1_4MS                 (5)
#define HAL_SCO_QOS_TOKEN_PERIOD_2MS                   (8)
#define HAL_SCO_QOS_TOKEN_PERIOD_4MS                   (9)
#define HAL_SCO_QOS_TOKEN_PERIOD_16MS                  (11)
#define HAL_SCO_QOS_L1_RATE_LIMIT                      (0x18)
#define HAL_SCO_QOS_L2_RATE_LIMIT                      (0x04)
#define HAL_SCO_QOS_QUEUE_PIM_WIDTH                    (3)
#define HAL_SCO_QOS_QUEUE_PIM_MASK                     (7)
#define HAL_SCO_QOS_QUEUE_DEFAULT_VAL                  (0x222227)
#define HAL_SCO_QOS_QUEUE_TRUST_HIGH_WEIGHT            (6)
#define HAL_SCO_QOS_QUEUE_TRUST_MID_WEIGHT             (5)
#define HAL_SCO_QOS_QUEUE_TRUST_LOW_WEIGHT             (4)
#define HAL_SCO_QOS_SHAPER_RATE_MAX_EXP                (4)
#define HAL_SCO_QOS_SHAPER_RATE_MAX_MAN                (0x1ffff)
#define HAL_SCO_QOS_SHAPER_RATE_MIN_WEIGHT             (1)
#define HAL_SCO_QOS_SHAPER_RATE_MAX_WEIGHT             (128)
#define HAL_SCO_QOS_QUEUE_0                            (0)
#define HAL_SCO_QOS_QUEUE_1                            (1)
#define HAL_SCO_QOS_QUEUE_2                            (2)
#define HAL_SCO_QOS_QUEUE_3                            (3)
#define HAL_SCO_QOS_QUEUE_4                            (4)
#define HAL_SCO_QOS_QUEUE_5                            (5)
#define HAL_SCO_QOS_QUEUE_6                            (6)
#define HAL_SCO_QOS_QUEUE_7                            (7)
#define HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_SP  (1)
#define HAL_SCO_QOS_MIN_TRAFFIC_ARBITRATION_SCHEME_WRR (0)
#define HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_SP  (1)
#define HAL_SCO_QOS_MAX_TRAFFIC_ARBITRATION_SCHEME_WFQ (0)
#define HAL_SCO_QOS_MAX_EXCESS_SP                      (1)
#define HAL_SCO_QOS_MAX_EXCESS_DROP                    (0)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef union HAL_SCO_QOS_QUEUE_UPW_S
{
    struct
    {
        UI32_T csr_acl_weight  : 3;
        UI32_T                 : 1;
        UI32_T csr_stag_weight : 3; /*Not use yet*/
        UI32_T                 : 1;
        UI32_T csr_1p_weight   : 3;
        UI32_T                 : 1;
        UI32_T csr_dscp_weight : 3;
        UI32_T                 : 1;
        UI32_T csr_port_weight : 3;
        UI32_T                 : 1;
        UI32_T csr_arl_weight  : 3;
        UI32_T                 : 9;
    } raw;
    UI32_T byte;
} HAL_SCO_QOS_QUEUE_UPW_T;

typedef union HAL_SCO_QOS_QUEUE_PEM_S
{
    struct
    {
        UI32_T csr_dscp_pri_l : 6; /*Not use yet*/
        UI32_T csr_que_lan_l  : 2; /*Not use yet*/
        UI32_T csr_que_cpu_l  : 3;
        UI32_T csr_tag_pri_l  : 3; /*Not use yet*/
        UI32_T                : 2;
        UI32_T csr_dscp_pri_h : 6; /*Not use yet*/
        UI32_T csr_que_lan_h  : 2; /*Not use yet*/
        UI32_T csr_que_cpu_h  : 3;
        UI32_T csr_tag_pri_h  : 3; /*Not use yet*/
        UI32_T                : 2;
    } raw;
    UI32_T byte;
} HAL_SCO_QOS_QUEUE_PEM_T;

typedef union HAL_SCO_QOS_SHAPER_MIN_S
{
    struct
    {
        UI32_T min_rate_man : 17;
        UI32_T min_reserve  : 2;
        UI32_T min_rate_en  : 1;
        UI32_T min_rate_exp : 4;
        UI32_T min_weight   : 7;
        UI32_T min_sp_wrr_q : 1;
    } raw;
    UI32_T byte;
} HAL_SCO_QOS_SHAPER_MIN_T;

typedef union HAL_SCO_QOS_SHAPER_MAX_S
{
    struct
    {
        UI32_T max_rate_man  : 17;
        UI32_T max_reserve   : 1;
        UI32_T max_excess_en : 1;
        UI32_T max_rate_en   : 1;
        UI32_T max_rate_exp  : 4;
        UI32_T max_weight    : 7;
        UI32_T max_sp_wfq_q  : 1;
    } raw;
    UI32_T byte;
} HAL_SCO_QOS_SHAPER_MAX_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_qos_init
 * PURPOSE:
 *      Initialization functions of QoS.
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
hal_sco_qos_init(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_qos_deinit
 * PURPOSE:
 *      Deinitialization functions of QoS.
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
hal_sco_qos_deinit(
    const UI32_T unit);

/* FUNCTION NAME: hal_sco_qos_setRateLimitEnable
 * PURPOSE:
 *      Enable or disable port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 *      enable         --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
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
hal_sco_qos_setRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             enable);

/* FUNCTION NAME: hal_sco_qos_getRateLimitEnable
 * PURPOSE:
 *      Get port rate limit state.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 * OUTPUT:
 *      ptr_enable     --  TRUE: eanble rate limit
 *                          FALSE: disable rate limit
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitEnable(
    const UI32_T             unit,
    const UI32_T             port,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_enable);

/* FUNCTION NAME: hal_sco_qos_setRateLimitCfg
 * PURPOSE:
 *      Set per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      ptr_cfg         --  AIR_QOS_RATE_LIMIT_CFG_T
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
hal_sco_qos_setRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_sco_qos_getRateLimitCfg
 * PURPOSE:
 *      Get per port rate limit.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *
 * OUTPUT:
 *      ptr_cfg         --  AIR_QOS_RATE_LIMIT_CFG_T
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitCfg(
    const UI32_T              unit,
    const UI32_T              port,
    AIR_QOS_RATE_LIMIT_CFG_T *ptr_cfg);

/* FUNCTION NAME: hal_sco_qos_setRateLimitExcludeMgmt
 * PURPOSE:
 *      Exclude/Include management frames to rate limit control.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 *      exclude         --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
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
hal_sco_qos_setRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    const BOOL_T             exclude);

/* FUNCTION NAME: hal_sco_qos_getRateLimitExcludeMgmt
 * PURPOSE:
 *      Get rate limit control exclude/include management frames.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dir             --  AIR_QOS_RATE_DIR_INGRESS
 *                          AIR_QOS_RATE_DIR_EGRESS
 * OUTPUT:
 *      ptr_exclude     --  TRUE: Exclude management frame
 *                          FALSE:Include management frame
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getRateLimitExcludeMgmt(
    const UI32_T             unit,
    const AIR_QOS_RATE_DIR_T dir,
    BOOL_T                  *ptr_exclude);

/* FUNCTION NAME: hal_sco_qos_setTrustMode
 * PURPOSE:
 *      Set qos trust mode value.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      mode            --  Qos support mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setTrustMode(
    const UI32_T               unit,
    const AIR_QOS_TRUST_MODE_T mode);

/* FUNCTION NAME: hal_sco_qos_getTrustMode
 * PURPOSE:
 *      Get qos trust mode value.
 *
 * INPUT:
 *      unit            --  Select device ID
 * OUTPUT:
 *      ptr_mode        --  Qos trust mode value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getTrustMode(
    const UI32_T                unit,
    AIR_QOS_TRUST_MODE_T *const ptr_mode);

/* FUNCTION NAME: hal_sco_qos_setPriToQueue
 * PURPOSE:
 *      Set qos pri to queue mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 *      queue           --  Qos Queue value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setPriToQueue(
    const UI32_T unit,
    const UI32_T pri,
    const UI32_T queue);

/* FUNCTION NAME: hal_sco_qos_getPriToQueue
 * PURPOSE:
 *      Get qos pri to queue mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      pri             --  Qos pri value
 * OUTPUT:
 *      ptr_queue       --  Qos pri mapping Queue value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getPriToQueue(
    const UI32_T  unit,
    const UI32_T  pri,
    UI32_T *const ptr_queue);

/* FUNCTION NAME: hal_sco_qos_setDscpToPri
 * PURPOSE:
 *      Set qos dscp to pri mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Qos dscp value
 *      pri             --  Qos pri value
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setDscpToPri(
    const UI32_T unit,
    const UI32_T dscp,
    const UI32_T pri);

/* FUNCTION NAME: hal_sco_qos_getDscpToPri
 * PURPOSE:
 *      Get qos dscp to pri mapping.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      dscp            --  Qos dscp value
 * OUTPUT:
 *      ptr_pri         --  Qos dscp mapping pri value
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getDscpToPri(
    const UI32_T  unit,
    const UI32_T  dscp,
    UI32_T *const ptr_pri);

/* FUNCTION NAME: hal_sco_qos_setScheduleMode
 * PURPOSE:
 *      Set schedule mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      sch_mode        --  AIR_QOS_SCH_MODE_T
 *      weight          --  weight for WRR/WFQ
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      Weight default value is 1, only for WRR/WFQ mode
 */
AIR_ERROR_NO_T
hal_sco_qos_setScheduleMode(
    const UI32_T             unit,
    const UI32_T             port,
    const UI32_T             queue,
    const AIR_QOS_SCH_MODE_T sch_mode,
    const UI32_T             weight);

/* FUNCTION NAME: hal_sco_qos_getScheduleMode
 * PURPOSE:
 *      Get schedule mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_sch_mode    --  AIR_QOS_SCH_MODE_T
 *      ptr_weight      --  weight for WRR/WFQ
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *     None
 */
AIR_ERROR_NO_T
hal_sco_qos_getScheduleMode(
    const UI32_T        unit,
    const UI32_T        port,
    const UI32_T        queue,
    AIR_QOS_SCH_MODE_T *ptr_sch_mode,
    UI32_T             *ptr_weight);

/* FUNCTION NAME: hal_sco_qos_setShaperMode
 * PURPOSE:
 *      Set min/max shaper mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      min             --  Min shaper mode
 *      max             --  Max shaper mode
 *      exceed          --  Exceed shaper mode
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperMode(
    const UI32_T                       unit,
    const UI32_T                       port,
    const UI32_T                       queue,
    const AIR_QOS_SHAPER_MODE_T        min,
    const AIR_QOS_SHAPER_MODE_T        max,
    const AIR_QOS_EXCEED_SHAPER_MODE_T exceed);

/* FUNCTION NAME: hal_sco_qos_getShaperMode
 * PURPOSE:
 *      Get min/max shaper mode of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_min         --  min shaper mode
 *      ptr_max         --  min shaper mode
 *      ptr_exceed      --  exceed shaper mode
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperMode(
    const UI32_T                        unit,
    const UI32_T                        port,
    const UI32_T                        queue,
    AIR_QOS_SHAPER_MODE_T *const        ptr_min,
    AIR_QOS_SHAPER_MODE_T *const        ptr_max,
    AIR_QOS_EXCEED_SHAPER_MODE_T *const ptr_exceed);

/* FUNCTION NAME: hal_sco_qos_setShaperRate
 * PURPOSE:
 *      Set shaper rate of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      ptr_shaper_cfg  --  AIR_QOS_SHAPER_CFG_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      ratelimit value = rate_man * 10 ^ rate_exp
 *      rate_man range 0 ~ 2^17
 *      rate_exp range 0 ~ 4, AIR_QOS_SHAPER_NOSETTING value means no setting
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperRate(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg);

/* FUNCTION NAME: hal_sco_qos_getShaperRate
 * PURPOSE:
 *      Get shaper rate of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_shaper_cfg  --  AIR_QOS_SHAPER_CFG_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperRate(
    const UI32_T          unit,
    const UI32_T          port,
    const UI32_T          queue,
    AIR_QOS_SHAPER_CFG_T *ptr_shaper_cfg);

/* FUNCTION NAME: hal_sco_qos_setShaperWeight
 * PURPOSE:
 *      Set shaper weight of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 *      weight          --  AIR_QOS_SHAPER_WEIGHT_T
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      ratelimit weight range 0~127
 */
AIR_ERROR_NO_T
hal_sco_qos_setShaperWeight(
    const UI32_T                  unit,
    const UI32_T                  port,
    const UI32_T                  queue,
    const AIR_QOS_SHAPER_WEIGHT_T weight);

/* FUNCTION NAME: hal_sco_qos_getShaperWeight
 * PURPOSE:
 *      Get shaper weight of a port queue.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Port id
 *      queue           --  Queue id
 * OUTPUT:
 *      ptr_weight      --  AIR_QOS_SHAPER_WEIGHT_T
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getShaperWeight(
    const UI32_T                   unit,
    const UI32_T                   port,
    const UI32_T                   queue,
    AIR_QOS_SHAPER_WEIGHT_T *const ptr_weight);

/* FUNCTION NAME: hal_sco_qos_setPortPriority
 * PURPOSE:
 *      Set port based qos priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 *      pri             --  Qos priority value
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
hal_sco_qos_setPortPriority(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pri);

/* FUNCTION NAME: hal_sco_qos_getPortPriority
 * PURPOSE:
 *      Get port based qos priority.
 *
 * INPUT:
 *      unit            --  Select device ID
 *      port            --  Select port number
 * OUTPUT:
 *      ptr_pri         --  Qos priority value
 *
 * RETURN:
 *      AIR_E_OK
 *      AIR_E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
hal_sco_qos_getPortPriority(
    const UI32_T unit,
    const UI32_T port,
    UI32_T      *ptr_pri);

#endif /* end of HAL_SCO_QOS_H */
