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
#include <cmd/swc_cmd.h>

#include <air_error.h>
#include <air_port.h>
#include <air_qos.h>
#include <air_types.h>
#include <cmlib/cmlib_bit.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define SWC_CMD_PRI_INVALID_VALUE (-1)
#define SWC_CMD_PRI_MAX_VALUE     (7)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
const static C8_T *_swd_frame_type[] = {
    "igmp",   "pppoe",  "arp",    "pae",    "dhcp",   "bpdu",   "ttl-0",  "mld",  "rev-01",
    "rev-02", "rev-03", "rev-0e", "rev-10", "rev-20", "rev-21", "rev-un", "lldp",
};
const static C8_T *_swd_fwd_ctrl[AIR_SWC_MGMT_FRAME_TYPE_LAST] = {
    "default", "cpu-include", "cpu-exclude", "cpu-only", "drop",
};
const static C8_T *_swd_enable[AIR_SWC_MGMT_FRAME_TYPE_LAST] = {
    "disable",
    "enable",
};
const static C8_T *_swd_property[] = {
    "mac-auto-flush",
    "l1-rate-ctrl",
    "acl-rate-ctrl-mgmt-frame",
    "storm-ctrl-mgmt-frame",
};

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_swc_cmd_setMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0;
    C8_T                     mode_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     type_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     pri_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     fwd_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     bpdu_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                   mng_type = 0;
    AIR_SWC_MGMT_FRAME_CFG_T frame_cfg;

    /*
     * Command format
     * swc set mgmt-frame [ unit=<UNIT> ]
     * type={ igmp | pppoe | arp | pae | dhcp | ttl-0 | bpdu | mld | lldp |
     * rev-01 | rev-02 | rev-03 | rev-0e | rev-10 | rev-20 | rev-21 | rev-un }
     * { [ mode={ enable | disable } ] [ pri-high={ enable | disable } ]
     * [ forward={ default | cpu-exclude | cpu-include | cpu-only | drop } ] }
     * [ as-bpdu={ enable | disable } ] }
     * Note: rev-xx = 01-80-C2-00-00-xx of destination mac, rev-un = others
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "pri-high", pri_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "forward", fwd_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "as-bpdu", bpdu_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (osal_strlen(mode_str) == 0 && osal_strlen(pri_str) == 0 && osal_strlen(fwd_str) == 0 &&
        osal_strlen(bpdu_str) == 0)
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(type_str, "igmp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_IGMP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "pppoe"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_PPPOE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "arp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_ARP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "pae"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_PAE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "dhcp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_DHCP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "ttl-0"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_TTL_0;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "bpdu"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_BPDU;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "mld"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_MLD;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-01"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_01;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-02"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_02;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-03"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_03;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-0e"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_0E;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-10"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_10;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-20"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_20;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-21"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_21;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-un"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_UN;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "lldp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    osal_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
    frame_cfg.frame_type = mng_type;
    air_swc_getMgmtFrameCfg(unit, &frame_cfg);

    if (osal_strlen(mode_str) != 0)
    {
        if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        }
        else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }

    if (osal_strlen(pri_str) != 0)
    {
        if (AIR_E_OK == dsh_checkString(pri_str, "enable"))
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        }
        else if (AIR_E_OK == dsh_checkString(pri_str, "disable"))
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }

    if (osal_strlen(fwd_str) != 0)
    {
        if (AIR_E_OK == dsh_checkString(fwd_str, "default"))
        {
            frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
        }
        else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-exclude"))
        {
            frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
        }
        else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-include"))
        {
            frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
        }
        else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-only"))
        {
            frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
        }
        else if (AIR_E_OK == dsh_checkString(fwd_str, "drop"))
        {
            frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    if (osal_strlen(bpdu_str) != 0)
    {
        if (AIR_E_OK == dsh_checkString(bpdu_str, "enable"))
        {
            frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else if (AIR_E_OK == dsh_checkString(bpdu_str, "disable"))
        {
            frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }

    rc = air_swc_setMgmtFrameCfg(unit, &frame_cfg);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set mng-frm-cfg error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_showMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0, mng_type = 0;
    AIR_SWC_MGMT_FRAME_CFG_T frame_cfg;
    UI32_T                   enable = 0, pri_high = 0, as_bpdu = 0;

    /*
     * Command format
     * swc show mgmt-frame [ unit=<UNIT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    osal_printf("frame  management  pri-high  as-bpdu  forward control\n");
    osal_printf("-----------------------------------------------------\n");

    for (mng_type = AIR_SWC_MGMT_FRAME_TYPE_IGMP; mng_type < AIR_SWC_MGMT_FRAME_TYPE_LAST; mng_type++)
    {
        osal_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        frame_cfg.frame_type = mng_type;
        rc = air_swc_getMgmtFrameCfg(unit, &frame_cfg);
        if (AIR_E_OK == rc)
        {
            if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE)
            {
                enable = 1;
            }
            else
            {
                enable = 0;
            }

            if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_PRI_HIGH)
            {
                pri_high = 1;
            }
            else
            {
                pri_high = 0;
            }

            if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU)
            {
                as_bpdu = 1;
            }
            else
            {
                as_bpdu = 0;
            }
            osal_printf("%6s %10s %9s %8s %12s\n", _swd_frame_type[mng_type], _swd_enable[enable],
                        _swd_enable[pri_high], _swd_enable[as_bpdu], _swd_fwd_ctrl[frame_cfg.forward_mode]);
        }
        else
        {
            osal_printf("***Error***, show mng-frm-cfg error\n");
        }
    }
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setPortMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0, port = 0;
    C8_T                     mode_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     type_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     fwd_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     bpdu_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                     pri_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                   mng_type = 0;
    I32_T                    pri = SWC_CMD_PRI_INVALID_VALUE;
    AIR_SWC_MGMT_FRAME_CFG_T frame_cfg;
    AIR_PORT_BITMAP_T        pbm = {0};

    /*
     * Command format
     * swc set port-mgmt-frame [ unit=<UINT> ] portlist=<UINTLIST>
     * type={ igmp | pppoe | arp | pae | dhcp | ttl-0 | bpdu | mld | lldp |
     * rev-01 | rev-02 | rev-03 | rev-0e | rev-10 | rev-20 | rev-21 | rev-un }
     * { [ mode={ enable | disable } ] [ force-pri={ enable pri=<UINT> | disable } ]
     * [ forward={ default | cpu-exclude | cpu-include | cpu-only | drop } ]
     * [ as-bpdu={ enable | disable } ] }
     * Note: rev-xx = 01-80-C2-00-00-xx of destination mac, rev-un = others
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "force-pri", pri_str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(pri_str, "enable"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "pri", (UI32_T *)&pri), token_idx, 2);
    }
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "forward", fwd_str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "as-bpdu", bpdu_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if ((osal_strlen(mode_str) == 0) && (osal_strlen(pri_str) == 0) && (osal_strlen(fwd_str) == 0) &&
        (osal_strlen(bpdu_str) == 0))
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(type_str, "igmp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_IGMP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "pppoe"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_PPPOE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "arp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_ARP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "pae"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_PAE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "dhcp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_DHCP;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "ttl-0"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_TTL_0;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "bpdu"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_BPDU;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "mld"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_MLD;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-01"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_01;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-02"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_02;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-03"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_03;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-0e"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_0E;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-10"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_10;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-20"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_20;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-21"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_21;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rev-un"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_REV_UN;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "lldp"))
    {
        mng_type = AIR_SWC_MGMT_FRAME_TYPE_LLDP;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
        frame_cfg.frame_type = mng_type;
        air_swc_getPortMgmtFrameCfg(unit, port, &frame_cfg);

        if (osal_strlen(mode_str) != 0)
        {
            if (AIR_E_OK == dsh_checkString(mode_str, "enable"))
            {
                frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
            }
            else if (AIR_E_OK == dsh_checkString(mode_str, "disable"))
            {
                frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }

        if (osal_strlen(pri_str) != 0)
        {
            if (AIR_E_OK == dsh_checkString(pri_str, "enable"))
            {
                if ((SWC_CMD_PRI_MAX_VALUE >= pri) && (SWC_CMD_PRI_INVALID_VALUE != pri))
                {
                    frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
                    frame_cfg.pri = pri;
                }
                else
                {
                    return AIR_E_BAD_PARAMETER;
                }
            }
            else if (AIR_E_OK == dsh_checkString(pri_str, "disable"))
            {
                if (SWC_CMD_PRI_INVALID_VALUE == pri)
                {
                    frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI;
                    frame_cfg.pri = 0;
                }
                else
                {
                    return AIR_E_BAD_PARAMETER;
                }
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }

        if (osal_strlen(fwd_str) != 0)
        {
            if (AIR_E_OK == dsh_checkString(fwd_str, "default"))
            {
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING;
            }
            else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-exclude"))
            {
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_EXCLUDE_CPU;
            }
            else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-include"))
            {
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_SYS_SETTING_INCLUDE_CPU;
            }
            else if (AIR_E_OK == dsh_checkString(fwd_str, "cpu-only"))
            {
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_CPU_ONLY;
            }
            else if (AIR_E_OK == dsh_checkString(fwd_str, "drop"))
            {
                frame_cfg.forward_mode = AIR_SWC_MGMT_FWD_MODE_DROP;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }

        if (osal_strlen(bpdu_str) != 0)
        {
            if (AIR_E_OK == dsh_checkString(bpdu_str, "enable"))
            {
                frame_cfg.flags |= AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
            }
            else if (AIR_E_OK == dsh_checkString(bpdu_str, "disable"))
            {
                frame_cfg.flags &= ~AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }

        rc = air_swc_setPortMgmtFrameCfg(unit, port, &frame_cfg);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port-mng-frm-cfg error\n");
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_showPortMgmtFrame(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T           rc = AIR_E_OK;
    UI32_T                   unit = 0, port = 0, mng_type = 0;
    AIR_SWC_MGMT_FRAME_CFG_T frame_cfg;
    UI32_T                   enable = 0, as_bpdu = 0;
    AIR_PORT_BITMAP_T        pbm = {0};

    /*
     * Command format
     * swc show mgmt-frame [ unit=<UNIT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %d\n", unit);
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("port %d\n", port);
        osal_printf("frame  management  priority  as-bpdu  forward control\n");
        osal_printf("-----------------------------------------------------\n");

        for (mng_type = AIR_SWC_MGMT_FRAME_TYPE_IGMP; mng_type < AIR_SWC_MGMT_FRAME_TYPE_LAST; mng_type++)
        {
            osal_memset(&frame_cfg, 0, sizeof(AIR_SWC_MGMT_FRAME_CFG_T));
            frame_cfg.frame_type = mng_type;
            rc = air_swc_getPortMgmtFrameCfg(unit, port, &frame_cfg);
            if (AIR_E_OK == rc)
            {
                if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_ENABLE)
                {
                    enable = 1;
                }
                else
                {
                    enable = 0;
                }
                if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_AS_BPDU)
                {
                    as_bpdu = 1;
                }
                else
                {
                    as_bpdu = 0;
                }
                if (frame_cfg.flags & AIR_SWC_MGMT_FRAME_CFG_FLAGS_SET_PRI)
                {
                    osal_printf("%6s %10s %9d %8s %12s\n", _swd_frame_type[mng_type], _swd_enable[enable],
                                frame_cfg.pri, _swd_enable[as_bpdu], _swd_fwd_ctrl[frame_cfg.forward_mode]);
                }
                else
                {
                    osal_printf("%6s %10s %9s %8s %12s\n", _swd_frame_type[mng_type], _swd_enable[enable], "---",
                                _swd_enable[as_bpdu], _swd_fwd_ctrl[frame_cfg.forward_mode]);
                }
            }
            else
            {
                osal_printf("***Error***, show port-mng-frm-cfg error\n");
            }
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setSysMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_MAC_T      mac;

    /*
     * Command format
     * swc set sys-mac [ unit=<UINT> ] mac=<MACADDR>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "mac", &mac), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_setSystemMac(unit, mac);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set sys-mac error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_getSysMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_MAC_T      mac;

    /*
     * Command format
     * swc show sys-mac [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_getSystemMac(unit, mac);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show sys-mac error\n");
    }
    else
    {
        osal_printf("system mac address : %02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2], mac[3], mac[4],
                    mac[5]);
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setJumbo(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    C8_T                 str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_SWC_JUMBO_SIZE_T jlen = AIR_SWC_JUMBO_SIZE_LAST;

    UI32_T               i = 0;
    C8_T str_jumbo[AIR_SWC_JUMBO_SIZE_LAST][6] = {"1518", "1536", "1552", "2048", "3072",  "4096", "5120",
                                                  "6144", "7168", "8192", "9216", "12288", "15360"};
    /*
     * Command format
     * swc set jumbo [ unit=<UINT> ]
     * jumbo-len={ 1518 | 1536 | 1552 | 2048 | 3072 | 4096 | 5120 | 6144 | 7168 | 8192 | 9216 | 12288 | 15360 }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "jumbo-len", str), token_idx, 2);

    for (i = 0; i <= AIR_SWC_JUMBO_SIZE_LAST; i++)
    {
        if (AIR_E_OK == dsh_checkString(str, str_jumbo[i]))
        {
            jlen = i;
            break;
        }
    }
    if (AIR_SWC_JUMBO_SIZE_LAST == i)
    {
        return AIR_E_BAD_PARAMETER;
    }

    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_setJumboSize(unit, jlen);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set jumbo error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_showJumbo(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0;
    AIR_SWC_JUMBO_SIZE_T jlen = AIR_SWC_JUMBO_SIZE_LAST;

    C8_T str_jumbo[AIR_SWC_JUMBO_SIZE_LAST][6] = {"1518", "1536", "1552", "2048", "3072",  "4096", "5120",
                                                  "6144", "7168", "8192", "9216", "12288", "15360"};
    /*
     * Command format
     * swc show jumbo [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("unit %u\n", unit);
    rc = air_swc_getJumboSize(unit, &jlen);
    if (AIR_E_OK == rc)
    {
        osal_printf(" - jumbo len = %s\n", str_jumbo[jlen]);
    }
    else
    {
        osal_printf("***Error***, show jumbo error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setProperty(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    C8_T               state_str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T               type_str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T             enable = 0, param = 0;
    AIR_SWC_PROPERTY_T type = AIR_SWC_PROPERTY_LAST;

    /*
     * Command format
     * swc set property [ unit=<UNIT> ]
     * type={ mac-auto-flush | l1-rate-ctrl | acl-rate-ctrl-mgmt-frame | storm-ctrl-mgmt-frame } state={ enable | disable }\n"
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", state_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(type_str, "mac-auto-flush"))
    {
        type = AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "l1-rate-ctrl"))
    {
        type = AIR_SWC_PROPERTY_ENABLE_L1_RATE_CTRL;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "acl-rate-ctrl-mgmt-frame"))
    {
        type = AIR_SWC_PROPERTY_ACL_RATE_CTRL_MGMT_FRAME_INCLUDE;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "storm-ctrl-mgmt-frame"))
    {
        type = AIR_SWC_PROPERTY_STORM_CTRL_MGMT_FRAME_INCLUDE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(state_str, "enable"))
    {
        enable = 1;
    }
    else if (AIR_E_OK == dsh_checkString(state_str, "disable"))
    {
        enable = 0;
    }

    rc = air_swc_setProperty(unit, type, enable, param);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set switch property error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_showProperty(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0;
    UI32_T             enable = 0, param = 0;
    AIR_SWC_PROPERTY_T type = AIR_SWC_PROPERTY_LAST;

    /*
     * Command format
     * swc show property [ unit=<UNIT> ]
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    osal_printf("                property      state \n");
    osal_printf("------------------------------------\n");

    for (type = AIR_SWC_PROPERTY_ENABLE_MAC_AUTO_FLUSH; type < AIR_SWC_PROPERTY_LAST; type++)
    {
        rc = air_swc_getProperty(unit, type, &enable, &param);
        if (AIR_E_OK == rc)
        {
            osal_printf("%24s %10s\n", _swd_property[type], _swd_enable[enable]);
        }
        else
        {
            osal_printf("***Error***, show switch property error\n");
        }
    }
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setLpdetSrcMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_MAC_T      mac;

    /*
     * Command format
     * swc set lpdet-src-mac [ unit=<UINT> ] mac=<MACADDR>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getMacAddr(tokens, token_idx, "mac", &mac), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_setLoopDetectFrameSrcMac(unit, mac);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set lpdet-src-mac error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_getLpdetSrcMac(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;
    AIR_MAC_T      mac;

    /*
     * Command format
     * swc show lpdet-src-mac [ unit=<UINT> ]
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_swc_getLoopDetectFrameSrcMac(unit, mac);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show lpdet-src-mac error\n");
    }
    else
    {
        osal_printf("lpdet frame source mac address : %02x-%02x-%02x-%02x-%02x-%02x\n", mac[0], mac[1], mac[2], mac[3],
                    mac[4], mac[5]);
    }

    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_setLpdetCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    C8_T                      sdr[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                      str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                    unit = 0, port = 0;
    BOOL_T                    enable = FALSE;
    AIR_PORT_BITMAP_T         pbm = {0};
    AIR_SWC_LPDET_CTRL_TYPE_T type = AIR_SWC_LPDET_CTRL_TYPE_LAST;
    /*
     * Command format
     * swc set lpdet-ctrl [ unit=<UINT> ] portlist=<UINTLIST> type={ tx-lp-frame | rx-lp-alarm } mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", sdr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdr, "tx-lp-frame"))
    {
        type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
    }
    else if (AIR_E_OK == dsh_checkString(sdr, "rx-lp-alarm"))
    {
        type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        enable = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        enable = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_swc_setLoopDetectCtrl(unit, port, type, enable);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u lpdet-ctrl error(%d)\n", port, rc);
            break;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_getLpdetCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0, port = 0;
    BOOL_T                    tx_enable = FALSE, rx_enable = FALSE;
    AIR_PORT_BITMAP_T         pbm = {0};
    C8_T                      type_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_SWC_LPDET_CTRL_TYPE_T type = AIR_SWC_LPDET_CTRL_TYPE_LAST;
    /*
     * Command format
     * swc show lpdet-ctrl [ unit=<UINT> ] portlist=<UINTLIST> [ type={ tx-lp-frame | rx-lp-alarm } ]
     */

    osal_strncpy(type_str, "both", sizeof("both"));
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(type_str, "both"))
    {
        /* default show tx and rx */
        osal_printf("%10s %11s %13s\n", "unit/port", "tx-loop-frame", "rx-loop-alarm");
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "tx-lp-frame"))
    {
        osal_printf("%10s %11s\n", "unit/port", "tx-loop-frame");
        type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rx-lp-alarm"))
    {
        osal_printf("%10s %11s\n", "unit/port", "rx-loop-alarm");
        type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
        {
            rc |= air_swc_getLoopDetectCtrl(unit, port, type, &tx_enable);
        }
        else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
        {
            rc |= air_swc_getLoopDetectCtrl(unit, port, type, &rx_enable);
        }
        else
        {
            rc |= air_swc_getLoopDetectCtrl(unit, port, AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME, &tx_enable);
            rc |= air_swc_getLoopDetectCtrl(unit, port, AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM, &rx_enable);
        }

        if (AIR_E_OK == rc)
        {
            if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
            {
                osal_printf(" %15s", (TRUE == tx_enable) ? "enable" : "disable");
            }
            else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
            {
                osal_printf(" %15s", (TRUE == rx_enable) ? "enable" : "disable");
            }
            else
            {
                osal_printf(" %15s %13s", (TRUE == tx_enable) ? "enable" : "disable",
                            (TRUE == rx_enable) ? "enable" : "disable");
            }
        }
        else
        {
            osal_printf("***Error***, get port=%u lpdet-ctrl error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_clearLpdetStatus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    C8_T                      sdr[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T                    unit = 0;
    AIR_PORT_BITMAP_T         pbm = {0};
    AIR_SWC_LPDET_CTRL_TYPE_T type = AIR_SWC_LPDET_CTRL_TYPE_LAST;

    /*
     * Command format
     * swc clear lpdet-status [ unit=<UINT> ] portlist=<UINTLIST> type={ tx-lp-frame | rx-lp-alarm }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", sdr), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdr, "tx-lp-frame"))
    {
        type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
    }
    else if (AIR_E_OK == dsh_checkString(sdr, "rx-lp-alarm"))
    {
        type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_swc_clearLoopDetectStatus(unit, type, pbm);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, clear lpdet-status error\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_swc_cmd_getLpdetStatus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T            rc = AIR_E_OK;
    UI32_T                    unit = 0, port = 0;
    AIR_PORT_BITMAP_T         tx_pbm = {0}, rx_pbm = {0}, pbm = {0};
    C8_T                      type_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_SWC_LPDET_CTRL_TYPE_T type = AIR_SWC_LPDET_CTRL_TYPE_LAST;

    /*
     * Command format
     * swc show lpdet-status [ unit=<UINT> ] portlist=<UINTLIST> [ type={ tx-lp-frame | rx-lp-alarm } ]
     */

    osal_strncpy(type_str, "both", sizeof("both"));
    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "type", type_str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(type_str, "both"))
    {
        /* default show tx and rx */
        osal_printf("%10s %11s %13s\n", "unit/port", "tx-loop-frame", "rx-loop-alarm");
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "tx-lp-frame"))
    {
        osal_printf("%10s %11s\n", "unit/port", "tx-loop-frame");
        type = AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME;
    }
    else if (AIR_E_OK == dsh_checkString(type_str, "rx-lp-alarm"))
    {
        osal_printf("%10s %11s\n", "unit/port", "rx-loop-alarm");
        type = AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
    {
        rc |= air_swc_getLoopDetectStatus(unit, type, tx_pbm);
    }
    else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
    {
        rc |= air_swc_getLoopDetectStatus(unit, type, rx_pbm);
    }
    else
    {
        rc |= air_swc_getLoopDetectStatus(unit, AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME, tx_pbm);
        rc |= air_swc_getLoopDetectStatus(unit, AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM, rx_pbm);
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show lpdet-status error\n");
    }
    else
    {
        AIR_PORT_FOREACH(pbm, port)
        {
            osal_printf("%5d/%2d", unit, port);
            if (AIR_SWC_LPDET_CTRL_TYPE_TX_LP_FRAME == type)
            {
                osal_printf(" %15s", (AIR_PORT_CHK(tx_pbm, port)) ? "active" : "inactive");
            }
            else if (AIR_SWC_LPDET_CTRL_TYPE_RX_LP_ALARM == type)
            {
                osal_printf(" %15s", (AIR_PORT_CHK(rx_pbm, port)) ? "loop" : "normal");
            }
            else
            {
                osal_printf(" %15s %13s", (AIR_PORT_CHK(tx_pbm, port)) ? "active" : "inactive",
                            (AIR_PORT_CHK(rx_pbm, port)) ? "loop" : "normal");
            }
            osal_printf("\n");
        }
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T _swc_cmd_vec[] =
{
    {
        "set mgmt-frame", 2, _swc_cmd_setMgmtFrame,
        "swc set mgmt-frame [ unit=<UINT> ] \n"
        "type={ igmp | pppoe | arp | pae | dhcp | ttl-0 | bpdu | mld | lldp |\n"
        "rev-01 | rev-02 | rev-03 | rev-0e | rev-10 | rev-20 | rev-21 | rev-un }\n"
        "{ [ mode={ enable | disable } ] [ pri-high={ enable | disable } ]\n"
        "[ forward={ default | cpu-exclude | cpu-include | cpu-only | drop } ]\n"
        "[ as-bpdu={ enable | disable } ] }\n"
    },
    {
        "show mgmt-frame", 2, _swc_cmd_showMgmtFrame,
        "swc show mgmt-frame [ unit=<UINT> ]\n"
    },
    {
        "set port-mgmt-frame", 2, _swc_cmd_setPortMgmtFrame,
        "swc set port-mgmt-frame [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ igmp | pppoe | arp | pae | dhcp | ttl-0 | bpdu | mld | lldp |\n"
        "rev-01 | rev-02 | rev-03 | rev-0e | rev-10 | rev-20 | rev-21 | rev-un }\n"
        "{ [ mode={ enable | disable } ] [ force-pri={ enable pri=<UINT> | disable } ]\n"
        "[ forward={ default | cpu-exclude | cpu-include | cpu-only | drop } ]\n"
        "[ as-bpdu={ enable | disable } ] }\n"
    },
    {
        "show port-mgmt-frame", 2, _swc_cmd_showPortMgmtFrame,
        "swc show port-mgmt-frame [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set jumbo", 2, _swc_cmd_setJumbo,
        "swc set jumbo [ unit=<UINT> ] jumbo-len={ 1518 | 1536 | 1552 | 2048 | 3072 |\n"
        "4096 | 5120 | 6144 | 7168 | 8192 | 9216 | 12288 | 15360 }\n"
    },
    {
        "show jumbo", 2, _swc_cmd_showJumbo,
        "swc show jumbo [ unit=<UINT> ]\n"
    },
    {
        "set sys-mac", 2, _swc_cmd_setSysMac,
        "swc set sys-mac [ unit=<UINT> ] mac=<MACADDR>\n"
    },
    {
        "show sys-mac", 2, _swc_cmd_getSysMac,
        "swc show sys-mac [ unit=<UINT> ]\n"
    },
    {
        "set property", 2, _swc_cmd_setProperty,
        "swc set property [ unit=<UINT> ] \n"
        "type={ mac-auto-flush | l1-rate-ctrl | acl-rate-ctrl-mgmt-frame |\n"
        "storm-ctrl-mgmt-frame }\n"
        "state={ enable | disable }\n"
    },
    {
        "show property", 2, _swc_cmd_showProperty,
        "swc show property [ unit=<UINT> ]\n"
    },
    {
        "set lpdet-src-mac", 2, _swc_cmd_setLpdetSrcMac,
        "swc set lpdet-src-mac [ unit=<UINT> ] mac=<MACADDR>\n"
    },
    {
        "show lpdet-src-mac", 2, _swc_cmd_getLpdetSrcMac,
        "swc show lpdet-src-mac [ unit=<UINT> ]\n"
    },
    {
        "set lpdet-ctrl", 2, _swc_cmd_setLpdetCtrl,
        "swc set lpdet-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ tx-lp-frame | rx-lp-alarm } mode={ enable | disable }\n"
    },
    {
        "show lpdet-ctrl", 2, _swc_cmd_getLpdetCtrl,
        "swc show lpdet-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "[ type={ tx-lp-frame | rx-lp-alarm } ]\n"
    },
    {
        "show lpdet-status", 2, _swc_cmd_getLpdetStatus,
        "swc show lpdet-status [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "[ type={ tx-lp-frame | rx-lp-alarm } ]\n"
    },
    {
        "clear lpdet-status", 2, _swc_cmd_clearLpdetStatus,
        "swc clear lpdet-status [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "type={ tx-lp-frame | rx-lp-alarm }\n"
    },
 };
/* clang-format on */

AIR_ERROR_NO_T
swc_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _swc_cmd_vec, sizeof(_swc_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
swc_cmd_usager()
{
    return (dsh_usager(_swc_cmd_vec, sizeof(_swc_cmd_vec) / sizeof(DSH_VEC_T)));
}
