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
#include <cmd/qos_cmd.h>

#include <air_qos.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>
/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_qos_cmd_setRateLimitCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T              sdr[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T            dir = 0;
    BOOL_T            mode = FALSE;

    /*
     * Command format
     * qos set rate-limit-ctrl [ unit=<UNIT> ] portlist=<UNITLIST>
     * dir={ ingress | egress }
     * mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", sdr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdr, "ingress"))
    {
        dir = AIR_QOS_RATE_DIR_INGRESS;
    }
    else if (AIR_E_OK == dsh_checkString(sdr, "egress"))
    {
        dir = AIR_QOS_RATE_DIR_EGRESS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        mode = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_qos_setRateLimitEnable(unit, port, dir, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u rate limit ctrl error\n", port);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_showRateLimitCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    UI32_T            dir = 0;
    BOOL_T            state = 0;

    /*
     * Command format
     * qos show rate-limit-ctrl [ unit=<UNIT> ] portlist=<UNITLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf(" - port = %u\n", port);
        dir = AIR_QOS_RATE_DIR_EGRESS;
        rc = air_qos_getRateLimitEnable(unit, port, dir, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" - egress = ");
            if (TRUE == state)
            {
                osal_printf("enable\n");
            }
            else
            {
                osal_printf("disable\n");
            }
        }
        else
        {
            osal_printf("***Error***, show port=%u rate limit ctrl error\n", port);
            break;
        }
        dir = AIR_QOS_RATE_DIR_INGRESS;
        rc = air_qos_getRateLimitEnable(unit, port, dir, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" - ingress = ");
            if (TRUE == state)
            {
                osal_printf("enable\n");
            }
            else
            {
                osal_printf("disable\n");
            }
        }
        else
        {
            osal_printf("***Error***, show port=%u rate limit error\n", port);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setRateLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0, port = 0;
    AIR_PORT_BITMAP_T        pbm = {0};
    AIR_QOS_RATE_LIMIT_CFG_T rate, cfg;

    /*
     * Command format
     * qos set rate-limit [ unit=<UINT> ] portlist=<UINTLIST>
     * [ igrs-rate=<UINT> ] [ igrs-bucket=<UINT> ]
     * [ egrs-rate=<UINT> ] [ egrs-bucket=<UINT> ]
     * Note: Limit rate = rate * 32Kbps
     */

    osal_memset(&rate, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    if (AIR_E_OK == dsh_getUint(tokens, token_idx, "igrs-rate", &rate.ingress_cir))
    {
        rate.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS;
        token_idx += 2;
    }
    if (AIR_E_OK == dsh_getUint(tokens, token_idx, "igrs-bucket", &rate.ingress_cbs))
    {
        token_idx += 2;
    }
    if (AIR_E_OK == dsh_getUint(tokens, token_idx, "egrs-rate", &rate.egress_cir))
    {
        rate.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS;
        token_idx += 2;
    }
    if (AIR_E_OK == dsh_getUint(tokens, token_idx, "egrs-bucket", &rate.egress_cbs))
    {
        token_idx += 2;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_memset(&cfg, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
        rc = air_qos_getRateLimitCfg(unit, port, &cfg);
        if (AIR_E_OK == rc)
        {
            if ((rate.flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS))
            {
                cfg.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_INGRESS;
                cfg.ingress_cir = rate.ingress_cir;
                cfg.ingress_cbs = rate.ingress_cbs;
            }
            if ((rate.flags & AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS))
            {
                cfg.flags |= AIR_QOS_RATE_LIMIT_CFG_FLAGS_ENABLE_EGRESS;
                cfg.egress_cir = rate.egress_cir;
                cfg.egress_cbs = rate.egress_cbs;
            }
        }
        else
        {
            osal_printf("***Error***, get port=%u ratelimit error\n", port);
            break;
        }
        rc = air_qos_setRateLimitCfg(unit, port, &cfg);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u ratelimit error\n", port);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_showRateLimit(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0, port = 0;
    AIR_PORT_BITMAP_T        pbm = {0};
    AIR_QOS_RATE_LIMIT_CFG_T rate;
    UI32_T                   bucket = 0;

    /*
     * Command format
     * qos show rate-limit [ unit=<UNIT> ] portlist=<UNITLIST>
     */

    osal_memset(&rate, 0, sizeof(AIR_QOS_RATE_LIMIT_CFG_T));
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf(" - port = %u\n", port);
        rc = air_qos_getRateLimitCfg(unit, port, &rate);
        if (AIR_E_OK == rc)
        {
            osal_printf(" - egress rate = %u kbps\n", (rate.egress_cir * 32));
            bucket = rate.egress_cbs;
            osal_printf(" - egress bucket = %u bytes\n", bucket);
            osal_printf(" - ingress rate = %u kbps\n", (rate.ingress_cir * 32));
            bucket = rate.ingress_cbs;
            osal_printf(" - ingress bucket = %u bytes\n", bucket);
        }
        else
        {
            osal_printf("***Error***, show port=%u rate limit error\n", port);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setRateLimitMgmtCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T           sdr[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T         dir = 0;
    BOOL_T         state = FALSE;

    /*
     * Command format
     * qos set rate-limit-exmgmt-frm [ unit=<UINT> ] dir={ ingress | egress } type={ include | exclude }
     */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", sdr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdr, "ingress"))
    {
        dir = AIR_QOS_RATE_DIR_INGRESS;
    }
    else if (AIR_E_OK == dsh_checkString(sdr, "egress"))
    {
        dir = AIR_QOS_RATE_DIR_EGRESS;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(str, "exclude"))
    {
        state = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "include"))
    {
        state = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_qos_setRateLimitExcludeMgmt(unit, dir, state);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        osal_printf("***Error***, only support include mode on this chip\n");
    }
    else if (AIR_E_OK != rc)

    {
        osal_printf("***Error***, set rate limit mgmt ctrl error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_showRateLimitMgmtCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    UI32_T         dir = 0;
    BOOL_T         state = 0;

    /*
     * Command format
     * qos show rate-limit-exmgmt-frm [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    dir = AIR_QOS_RATE_DIR_EGRESS;
    rc = air_qos_getRateLimitExcludeMgmt(unit, dir, &state);
    if (AIR_E_OK == rc)
    {
        osal_printf(" - egress management frame = ");
        if (TRUE == state)
        {
            osal_printf("exclude\n");
        }
        else
        {
            osal_printf("include\n");
        }
    }
    else
    {
        osal_printf("***Error***, show rate limit mgmt ctrl error\n");
    }
    dir = AIR_QOS_RATE_DIR_INGRESS;
    rc = air_qos_getRateLimitExcludeMgmt(unit, dir, &state);
    if (AIR_E_OK == rc)
    {
        osal_printf(" - ingress management frame = ");
        if (TRUE == state)
        {
            osal_printf("exclude\n");
        }
        else
        {
            osal_printf("include\n");
        }
    }
    else
    {
        osal_printf("***Error***, show rate limit mgmt ctrl error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setTrustMode(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T       rc = AIR_E_OTHERS;
    UI32_T               unit = 0;
    C8_T                 mode[DSH_CMD_MAX_LENGTH] = {0};

    AIR_QOS_TRUST_MODE_T mode_t;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode, "port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "1p-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_1P_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "dscp-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_DSCP_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "dscp-1p-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_DSCP_1P_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "1p-dscp-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_1P_DSCP_PORT;
    }
    else
    {
        osal_printf("***Error***, not support this trust mode %s yet\n", mode);
        return DSH_E_SYNTAX_ERR;
    }

    /*Send to driver*/
    rc = air_qos_setTrustMode(unit, mode_t);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set trust mode %s failed, rc is %d\n", mode, rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_getTrustMode(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T       rc = AIR_E_OTHERS;
    UI32_T               unit = 0;

    AIR_QOS_TRUST_MODE_T mode_t = AIR_QOS_TRUST_MODE_1P_PORT;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    rc = air_qos_getTrustMode(unit, &mode_t);
    if (AIR_E_OK == rc)
    {
        switch (mode_t)
        {
            case AIR_QOS_TRUST_MODE_PORT:
            {
                osal_printf("trust-mode is port\n");
                break;
            }
            case AIR_QOS_TRUST_MODE_1P_PORT:
            {
                osal_printf("trust-mode is 1p-port\n");
                break;
            }
            case AIR_QOS_TRUST_MODE_DSCP_PORT:
            {
                osal_printf("trust-mode is dscp-port\n");
                break;
            }
            case AIR_QOS_TRUST_MODE_DSCP_1P_PORT:
            {
                osal_printf("trust-mode is dscp-1p-port\n");
                break;
            }
            case AIR_QOS_TRUST_MODE_1P_DSCP_PORT:
            {
                osal_printf("trust-mode is 1p-dscp-port\n");
                break;
            }
            default:
            {
                osal_printf("trust-mode is 1p\n");
                return rc;
            }
        }
    }
    else
    {
        osal_printf("***Error***, get trust mode failed, rc is %d\n", rc);
        return rc;
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setPriQueueMapping(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    UI32_T         unit = 0;
    UI32_T         pri;
    UI32_T         queue;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    rc = air_qos_setPriToQueue(unit, pri, queue);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set priority %d to queue %d mapping failed, rc is %d\n", pri, queue, rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_getPriQueueMapping(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    UI32_T         unit = 0;
    UI32_T         pri = 0;
    UI32_T         queue;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    osal_printf("priority to queue mapping list:\n");
    for (; pri < AIR_QOS_QUEUE_MAX_NUM; pri++)
    {
        osal_printf("pri %d\t----\t", pri);
        rc = air_qos_getPriToQueue(unit, pri, &queue);
        if (AIR_E_OK == rc)
        {
            osal_printf("queue %d\n", queue);
        }
        else
        {
            osal_printf("***Error***, get pri to queue mapping fail(%d)\n", rc);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setDSCPRemark(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    UI32_T         unit = 0;
    UI32_T         dscp;
    UI32_T         pri;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dscp", &dscp), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    rc = air_qos_setDscpToPri(unit, dscp, pri);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set dscp to pri mapping failed rc is %d\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_getDSCPRemark(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T rc = AIR_E_OTHERS;
    UI32_T         unit = 0;
    UI32_T         dscp;
    UI32_T         pri;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dscp", &dscp), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    rc = air_qos_getDscpToPri(unit, dscp, &pri);
    if (AIR_E_OK == rc)
    {
        osal_printf("dscp %d ---- pri %d\n", dscp, pri);
    }
    else
    {
        osal_printf("***Error***, get dscp to pri mapping failed rc is %d\n", rc);
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setScheduleMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OTHERS;
    AIR_PORT_BITMAP_T  portlist = {0};
    UI32_T             port;
    UI32_T             unit = 0;
    UI32_T             queue;
    UI32_T             weight = AIR_QOS_SHAPER_NOSETTING;
    C8_T               mode[DSH_CMD_MAX_LENGTH] = {0};

    AIR_QOS_SCH_MODE_T sch_mode;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "sch-mode", mode), token_idx, 2);
    if (AIR_E_OK == dsh_getUint(tokens, token_idx, "weight", &weight))
    {
        token_idx += 2;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode, "sp"))
    {
        sch_mode = AIR_QOS_SCH_MODE_SP;
        if (AIR_QOS_SHAPER_NOSETTING != weight)
        {
            osal_printf("[Warning] sp schedule mode no need weight\n");
        }
    }
    else if (AIR_E_OK == dsh_checkString(mode, "wrr"))
    {
        sch_mode = AIR_QOS_SCH_MODE_WRR;
        if (AIR_QOS_SHAPER_NOSETTING == weight)
        {
            osal_printf("[Warning] no weight value input , please check\n");
        }
    }
    else if (AIR_E_OK == dsh_checkString(mode, "wfq"))
    {
        sch_mode = AIR_QOS_SCH_MODE_WFQ;
        if (AIR_QOS_SHAPER_NOSETTING == weight)
        {
            osal_printf("[Warning] no weight value input , please check\n");
        }
    }
    else
    {
        osal_printf("***Error***, unknown schedule mode, please check again\n");
        return AIR_E_BAD_PARAMETER;
    }

    /*Send to driver*/
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_qos_setScheduleMode(unit, port, queue, sch_mode, weight);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port %u schedule mode failed, rc is %d \n", port, rc);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_getScheduleMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OTHERS;
    AIR_PORT_BITMAP_T  portlist = {0};
    UI32_T             port;
    UI32_T             unit = 0;
    UI32_T             queue;
    UI32_T             weight = AIR_QOS_SHAPER_NOSETTING;

    AIR_QOS_SCH_MODE_T sch_mode;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "queue", &queue), token_idx, 2);

    /*Send to driver*/
    AIR_PORT_FOREACH(portlist, port)
    {
        osal_printf("port %d: \n", port);
        rc = air_qos_getScheduleMode(unit, port, queue, &sch_mode, &weight);
        if (AIR_E_OK == rc)
        {
            osal_printf("queue %d  setting is: ", queue);
            if (AIR_QOS_SCH_MODE_SP == sch_mode)
            {
                osal_printf("sp\n");
            }
            else if (AIR_QOS_SCH_MODE_WRR == sch_mode)
            {
                osal_printf("wrr\n");
                osal_printf("weight is: --  %d \n", weight);
            }
            else if (AIR_QOS_SCH_MODE_WFQ == sch_mode)
            {
                osal_printf("wfq\n");
                osal_printf("weight is: --  %d \n", weight);
            }
            else
            {
                osal_printf("unknown\n");
                return AIR_E_BAD_PARAMETER;
            }
        }
        else
        {
            osal_printf("***Error***, get shaper port %d mode queue %d failed, rc is %d\n", port, queue, rc);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setPortPriority(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T    rc = AIR_E_OTHERS;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T portlist = {0};
    UI32_T            port = 0;
    UI32_T            pri = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", &pri), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_qos_setPortPriority(unit, port, pri);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port %u priority failed, rc is %d \n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_showPortPriority(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T    rc = AIR_E_OTHERS;
    UI32_T            unit = 0;
    AIR_PORT_BITMAP_T portlist = {0};
    UI32_T            port = 0;
    UI32_T            pri = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /*Send to driver*/
    AIR_PORT_FOREACH(portlist, port)
    {
        rc = air_qos_getPortPriority(unit, port, &pri);
        if (AIR_E_OK == rc)
        {
            osal_printf("port %d priority: %d \n", port, pri);
        }
        else
        {
            osal_printf("***Error***, get port %d priority failed, rc is %d \n", port, rc);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_setPortTrustMode(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    AIR_PORT_BITMAP_T    portlist = {0};
    UI32_T               port = 0;
    C8_T                 mode[DSH_CMD_MAX_LENGTH] = {0};
    AIR_QOS_TRUST_MODE_T mode_t;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", mode), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(mode, "port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "1p-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_1P_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "dscp-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_DSCP_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "dscp-1p-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_DSCP_1P_PORT;
    }
    else if (AIR_E_OK == dsh_checkString(mode, "1p-dscp-port"))
    {
        mode_t = AIR_QOS_TRUST_MODE_1P_DSCP_PORT;
    }
    else
    {
        osal_printf("***Error***, not support this trust mode %s yet\n", mode);
        rc = AIR_E_BAD_PARAMETER;
    }
    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(portlist, port)
        {
            /*Send to driver*/
            rc = air_qos_setPortTrustMode(unit, port, mode_t);
            if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("***Error***, per port trust mode is not supported on this chip\n");
                break;
            }
            else if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port %d trust mode %s failed, rc is %d\n", port, mode, rc);
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_qos_cmd_getPortTrustMode(
    const C8_T *tokens[],
    UI32_T      token_idx)

{
    AIR_ERROR_NO_T       rc = AIR_E_OTHERS;
    AIR_PORT_BITMAP_T    portlist = {0};
    UI32_T               port = 0;
    UI32_T               unit = 0;

    AIR_QOS_TRUST_MODE_T mode_t = AIR_QOS_TRUST_MODE_1P_PORT;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &portlist), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(portlist, port)
    {
        /*Send to driver*/
        rc = air_qos_getPortTrustMode(unit, port, &mode_t);
        if (AIR_E_OK == rc)
        {
            osal_printf("port %d: ", port);
            switch (mode_t)
            {
                case AIR_QOS_TRUST_MODE_PORT:
                    osal_printf("port\n");
                    break;
                case AIR_QOS_TRUST_MODE_1P_PORT:
                    osal_printf("1p-port\n");
                    break;
                case AIR_QOS_TRUST_MODE_DSCP_PORT:
                    osal_printf("dscp-port\n");
                    break;
                case AIR_QOS_TRUST_MODE_DSCP_1P_PORT:
                    osal_printf("dscp-1p-port\n");
                    break;
                case AIR_QOS_TRUST_MODE_1P_DSCP_PORT:
                    osal_printf("1p-dscp-port\n");
                    break;
                default:
                    osal_printf("1p\n");
                    break;
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("***Error***, per port trust mode is not supported on this chip\n");
            break;
        }
        else
        {
            osal_printf("***Error***, get port %d trust mode failed, rc is %d\n", port, rc);
        }
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T _qos_cmd_vec[] =
{
    {
        "set rate-limit-ctrl", 2, _qos_cmd_setRateLimitCtrl,
        "qos set rate-limit-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "dir={ ingress | egress }\n"
        "mode={ enable | disable }\n"
    },
    {
        "show rate-limit-ctrl", 2, _qos_cmd_showRateLimitCtrl,
        "qos show rate-limit-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set rate-limit", 2, _qos_cmd_setRateLimit,
        "qos set rate-limit [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "[ igrs-rate=<UINT> ] [ igrs-bucket=<UINT> ]\n"
        "[ egrs-rate=<UINT> ] [ egrs-bucket=<UINT> ]\n"
    },
    {
        "show rate-limit", 2, _qos_cmd_showRateLimit,
        "qos show rate-limit [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set rate-limit-mgmt-ctrl", 2, _qos_cmd_setRateLimitMgmtCtrl,
        "qos set rate-limit-mgmt-ctrl [ unit=<UINT> ] dir={ ingress | egress }\n"
        "type={ include | exclude }\n"
    },
    {
        "show rate-limit-mgmt-ctrl", 2, _qos_cmd_showRateLimitMgmtCtrl,
        "qos show rate-limit-mgmt-ctrl [ unit=<UINT> ]\n"
    },
    {
        "set trust-mode", 2, _qos_cmd_setTrustMode,
        "qos set trust-mode [ unit=<UINT> ]\n"
        "mode={ port | 1p-port | dscp-port | dscp-1p-port | 1p-dscp-port } \n"
    },
    {
        "show trust-mode", 2, _qos_cmd_getTrustMode,
        "qos show trust-mode [ unit=<UINT> ]\n"
    },
    {
        "set pri-to-queue", 2, _qos_cmd_setPriQueueMapping,
        "qos set pri-to-queue [ unit=<UINT> ] pri=<UINT> queue=<UINT>\n"
    },
    {
        "show pri-to-queue", 2, _qos_cmd_getPriQueueMapping,
        "qos show pri-to-queue [ unit=<UINT> ]\n"
    },
    {
        "set dscp-to-pri", 2, _qos_cmd_setDSCPRemark,
        "qos set dscp-to-pri [ unit=<UINT> ] dscp=<UINT> pri=<UINT>\n"
    },
    {
        "show dscp-to-pri", 2, _qos_cmd_getDSCPRemark,
        "qos show dscp-to-pri [ unit=<UINT> ] dscp=<UINT>\n"
    },
    {
        "set schedule-mode", 2, _qos_cmd_setScheduleMode,
        "qos set schedule-mode [ unit=<UINT> ] portlist=<UINTLIST> queue=<UINT>\n"
        "sch-mode={ sp | wrr | wfq } [ weight=<UINT> ]\n"
    },
    {
        "show schedule-mode", 2, _qos_cmd_getScheduleMode,
        "qos show schedule-mode [ unit=<UINT> ] portlist=<UINTLIST> queue=<UINT>\n"
    },
    {
        "set port-priority", 2, _qos_cmd_setPortPriority,
        "qos set port-priority [ unit=<UINT> ] portlist=<UINTLIST> pri=<UINT>\n"
    },
    {
        "show port-priority", 2, _qos_cmd_showPortPriority,
        "qos show port-priority [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set port-trust-mode", 2, _qos_cmd_setPortTrustMode,
        "qos set port-trust-mode [ unit=<UINT>] portlist=<UINTLIST>\n"
        "mode={ port | 1p-port | dscp-port | dscp-1p-port | 1p-dscp-port }\n"
    },
    {
        "show port-trust-mode", 2, _qos_cmd_getPortTrustMode,
        "qos show port-trust-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
 };
/* clang-format on */

AIR_ERROR_NO_T
qos_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _qos_cmd_vec, sizeof(_qos_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
qos_cmd_usager()
{
    return (dsh_usager(_qos_cmd_vec, sizeof(_qos_cmd_vec) / sizeof(DSH_VEC_T)));
}
