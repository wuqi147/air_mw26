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
/* FILE NAME:   mw_cmd_sfp.c
 * PURPOSE:
 *      Implementation the commands for SFP module.
 * NOTES:
 *
 */

#ifdef AIR_SUPPORT_SFP
/* INCLUDE FILE DECLARATIONS
 */
#include "sfp_port.h"
#include "mw_cmd_util.h"
#include "air_port.h"
#include "mw_cmd_parser.h"
#include <hal/common/hal.h>
#include "sfp_module_inside_phy.h"
#include "sfp_util.h"

/* NAMING CONSTANT DECLARATIONS
 */
#define MW_CMD_SFP_NAME    "cmd_sfp"
/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* LOCAL SUBPROGRAM DECLARATIONS
 */
static MW_ERROR_NO_T
_sfp_cmd_setAnMode(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showAnMode(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setLocalAdv(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showLocalAdv(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showRemoteAdv(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setSpeed(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showSpeed(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setDuplex(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showDuplex(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setBckPres(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showBckPres(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setFlowCtrl(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showFlowCtrl(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showLink(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showPhyLink(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_setAdminState(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_showAdminState(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_start(
    const C8_T *tokens[],
    UI32_T token_idx);

static MW_ERROR_NO_T
_sfp_cmd_stop(
    const C8_T *tokens[],
    UI32_T token_idx);


/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T _mw_cmd_sfp_vec[] =
{
    {
        "set an-mode", 2, _sfp_cmd_setAnMode,
        "sfp set an-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show an-mode", 2, _sfp_cmd_showAnMode,
        "sfp show an-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set local-adv", 2, _sfp_cmd_setLocalAdv,
        "sfp set local-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "   10h={ enable | disable }\n"
        "   10f={ enable | disable }\n"
        "   100h={ enable | disable }\n"
        "   100f={ enable | disable }\n"
        "   1000f={ enable | disable }\n"
        "   2500f={ enable | disable }\n"
        "   pause={ enable | disable }\n"
        "   eee={ enable | disable }\n"
    },
    {
        "show local-adv", 2, _sfp_cmd_showLocalAdv,
        "sfp show local-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show remote-adv", 2, _sfp_cmd_showRemoteAdv,
        "sfp show remote-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set speed", 2, _sfp_cmd_setSpeed,
        "sfp set speed [ unit=<UINT> ] portlist=<UINTLIST> speed={ 10m | 100m | 1000m | 2500m }\n"
    },
    {
        "show speed", 2, _sfp_cmd_showSpeed,
        "sfp show speed [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set duplex", 2, _sfp_cmd_setDuplex,
        "sfp set duplex [ unit=<UINT> ] portlist=<UINTLIST> duplex={ full | half }\n"
    },
    {
        "show duplex", 2, _sfp_cmd_showDuplex,
        "sfp show duplex [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set back-pres", 2, _sfp_cmd_setBckPres,
        "sfp set back-pres [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show back-pres", 2, _sfp_cmd_showBckPres,
        "sfp show back-pres [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set flow-ctrl", 2, _sfp_cmd_setFlowCtrl,
        "sfp set flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST> dir={ rx | tx } mode={ enable | disable }\n"
    },
    {
        "show flow-ctrl", 2, _sfp_cmd_showFlowCtrl,
        "sfp show flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show link", 2, _sfp_cmd_showLink,
        "sfp show link [ unit=<UINT> ] portlist=<UINTLIST> \n"
    },
    {
        "show phy-link", 2, _sfp_cmd_showPhyLink,
        "sfp show phy-link [ unit=<UINT> ] portlist=<UINTLIST> \n"
    },
    {
        "set admin-state", 2, _sfp_cmd_setAdminState,
        "sfp set admin-state [ unit=<UINT> ] portlist=<UINTLIST> state={ enable | disable }\n"
    },
    {
        "show admin-state", 2, _sfp_cmd_showAdminState,
        "sfp show admin-state [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "start task", 2, _sfp_cmd_start,
        "sfp start task\n"
    },
    {
        "stop task", 2, _sfp_cmd_stop,
        "sfp stop task\n"
    },
};

/* LOCAL SUBPROGRAM BODIES
 */
static MW_ERROR_NO_T
_sfp_cmd_setAnMode(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T enable = FALSE;
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port set an-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "mode", str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        enable = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        enable = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setPhyAutoNego(unit, port, enable);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u anMode error(%d)\n", port, rc);
            break;
        }
        sfp_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showAnMode(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_PHY_AN_T auto_nego;

    /*
     * Command format
     * port show an-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %11s\n", "unit/port", "auto-nego");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getPhyAutoNego(unit, port, &auto_nego);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %13s", (AIR_PORT_PHY_AN_ENABLE == auto_nego) ? "enable" : "disable");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show anMode error(%d)\n", rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_setLocalAdv(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_PHY_AN_ADV_T adv;

    /*
     * Command format
     * port set local-adv [ unit=<UINT> ] portlist=<UINTLIST>
     *  10h={ enable | disable}
     *  10f={ enable | disable}
     *  100h={ enable | disable}
     *  100f={ enable | disable}
     *  1000f={ enable | disable}
     *  2500f={ enable | disable}
     *  pause={ enable | disable}
     *  eee={ enable | disable}
     */

    osal_memset(&adv, 0, sizeof(AIR_PORT_PHY_AN_ADV_T));

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "10h", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "10f", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "100h", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "100f", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "1000f", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "2500f", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_2500M;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "pause", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= (AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE | AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE | AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "eee", str), token_idx, 2);
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setPhyLocalAdvAbility(unit, port, &adv);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u localAdv error(%d)\n", port, rc);
            break;
        }
        sfp_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showLocalAdv(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_PHY_AN_ADV_T adv;

    /*
     * Command format
     * port show local-adv [ unit=<UINT> ] portlist=<UINTLIST>
     */

    osal_memset(&adv, 0, sizeof(AIR_PORT_PHY_AN_ADV_T));

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %8s %8s %8s %8s %8s %8s %8s %8s\n",
                "unit/port", "10h", "10f", "100h", "100f", "1000f", "2500f", "pause", "eee");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_getPhyLocalAdvAbility(unit, port, &adv);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT("%5d/%2d", unit, port);
            if (0 == adv.flags)
            {
                MW_CMD_OUTPUT(" %10s %8s %8s %8s %8s %8s %8s %8s",
                            "---", "---", "---", "---", "---", "---", "---", "---");
            }
            else
            {
                MW_CMD_OUTPUT(" %10s", (AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_2500M & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", ((AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE & adv.flags) ||
                                     (AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE & adv.flags)) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_EEE & adv.flags) ? "enable" : "disable");
            }
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT("%5d/%2d %10s %8s %8s %8s %8s %8s %8s %8s", unit, port,
                        "---", "---", "---", "---", "---", "---", "---", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u localAdv error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showRemoteAdv(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_PHY_AN_ADV_T adv;

    /*
     * Command format
     * port show remote-adv [ unit=<UINT> ] portlist=<UINTLIST>
     */

    osal_memset(&adv, 0, sizeof(AIR_PORT_PHY_AN_ADV_T));

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %8s %8s %8s %8s %8s %8s %8s %8s\n",
                "unit/port", "10h", "10f", "100h", "100f", "1000f", "2500f", "pause", "eee");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_getPhyRemoteAdvAbility(unit, port, &adv);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT("%5d/%2d", unit, port);
            if (0 == adv.flags)
            {
                MW_CMD_OUTPUT(" %10s %8s %8s %8s %8s %8s %8s %8s",
                            "---", "---", "---", "---", "---", "---", "---", "---");
            }
            else
            {
                MW_CMD_OUTPUT(" %10s", (AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_2500M & adv.flags) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", ((AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE & adv.flags) ||
                                     (AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE & adv.flags)) ? "enable" : "disable");
                MW_CMD_OUTPUT(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_EEE & adv.flags) ? "enable" : "disable");
            }
        }
         else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT("%5d/%2d %10s %8s %8s %8s %8s %8s %8s %8s", unit, port,
                        "---", "---", "---", "---", "---", "---", "---", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u remoteAdv error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_setSpeed(
    const C8_T *tokens[],
    UI32_T token_idx)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    AIR_PORT_SPEED_T speed = AIR_PORT_SPEED_LAST;

    /*
     * Command format
     * port set speed [ unit=<UINT> ] portlist=<UINTLIST> speed={ 10m | 100m | 1000m | 2500m }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "speed", str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(str, "10m"))
    {
        speed = AIR_PORT_SPEED_10M;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "100m"))
    {
        speed = AIR_PORT_SPEED_100M;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "1000m"))
    {
        speed = AIR_PORT_SPEED_1000M;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "2500m"))
    {
        speed = AIR_PORT_SPEED_2500M;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setSpeed(unit, port, speed);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u speed error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showSpeed(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_SPEED_T speed = 0;

    /*
     * Command format
     * port show speed [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %8s\n", "unit/port", "speed");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getSpeed(unit, port, &speed);
        if(AIR_E_OK == rc)
        {
            if(AIR_PORT_SPEED_10M == speed)
            {
                MW_CMD_OUTPUT(" %10s", "10m");
            }
            else if(AIR_PORT_SPEED_100M == speed)
            {
                MW_CMD_OUTPUT(" %10s", "100m");
            }
            else if(AIR_PORT_SPEED_1000M == speed)
            {
                MW_CMD_OUTPUT(" %10s", "1000m");
            }
            else if(AIR_PORT_SPEED_2500M == speed)
            {
                MW_CMD_OUTPUT(" %10s", "2500m");
            }
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT(" %10s", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u speed error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_setDuplex(
    const C8_T *tokens[],
    UI32_T token_idx)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    AIR_PORT_DUPLEX_T duplex = AIR_PORT_DUPLEX_LAST;

    /*
     * Command format
     * port set duplex [ unit=<UINT> ] portlist=<UINTLIST> duplex={ full | half }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "duplex", str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(str, "full"))
    {
        duplex = AIR_PORT_DUPLEX_FULL;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "half"))
    {
        duplex = AIR_PORT_DUPLEX_HALF;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setDuplex(unit, port, duplex);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u duplex error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showDuplex(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_DUPLEX_T duplex = AIR_PORT_DUPLEX_LAST;

    /*
     * Command format
     * port show duplex [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %8s\n", "unit/port", "duplex");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getDuplex(unit, port, &duplex);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %10s", (AIR_PORT_DUPLEX_HALF == duplex) ? "half" : "full");
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT(" %10s", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u duplex error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_setBckPres(
    const C8_T *tokens[],
    UI32_T token_idx)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    BOOL_T bckPres = FALSE;

    /*
     * Command format
     * port set back-pres [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "mode", str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        bckPres = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        bckPres = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setBackPressure(unit, port, bckPres);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u back pressure error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showBckPres(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T bckPres = FALSE;

    /*
     * Command format
     * port show back-pres [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %15s\n", "unit/port", "back-pressure");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getBackPressure(unit, port, &bckPres);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %17s", (TRUE == bckPres) ? "enable" : "disable");
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT(" %17s", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u back pressure error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_setFlowCtrl(
    const C8_T *tokens[],
    UI32_T token_idx)
{

    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    C8_T sdir[MW_CMD_CMD_MAX_LENGTH] = {0};
    AIR_PORT_DIR_T dir = AIR_PORT_DIR_TX;
    BOOL_T mode = FALSE;

    /*
     * Command format
     * port set flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST> dir={ rx | tx } mode={ enable | disable }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "dir", sdir), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "mode", str), token_idx, 2);

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);


    if(MW_E_OK == mw_cmd_checkString(sdir, "rx"))
    {
        dir = AIR_PORT_DIR_RX;
    }
    else if(MW_E_OK == mw_cmd_checkString(sdir, "tx"))
    {
        dir = AIR_PORT_DIR_TX;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        mode = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setFlowCtrl(unit, port, dir, mode);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, set port=%u flow control error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showFlowCtrl(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T state = FALSE;

    /*
     * Command format
     * port show flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %13s %13s\n", "unit/port", "rx-flowctrl", "tx-flowctrl");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getFlowCtrl(unit, port, AIR_PORT_DIR_RX, &state);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %15s", (TRUE == state) ? "enable" : "disable");
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT(" %15s", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u flow control error(%d)\n", port, rc);
            break;
        }
        rc = sfp_port_getFlowCtrl(unit, port, AIR_PORT_DIR_TX, &state);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %13s", (TRUE == state) ? "enable" : "disable");
        }
        else if(AIR_E_NOT_SUPPORT == rc)
        {
            MW_CMD_OUTPUT(" %13s", "---");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u flow control error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showLink(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    UI32_T speed = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_STATUS_T ps;

    /*
     * Command format
     * port show link [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("sfp port link status      :\n");
    MW_CMD_OUTPUT("unit %u\n", unit);
    MW_CMD_OUTPUT("%10s %6s %10s %11s %11s\n",
            "unit/port", "link", "speed", "duplex", "flow-ctrl");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_getPortStatus(unit, port, &ps);
        if(AIR_E_OK == rc)
        {
            if(ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)
            {
                if(AIR_PORT_SPEED_10M == ps.speed)
                {
                    speed = 10;
                }
                else if(AIR_PORT_SPEED_100M == ps.speed)
                {
                    speed = 100;
                }
                else if(AIR_PORT_SPEED_1000M == ps.speed)
                {
                    speed = 1000;
                }
                else
                {
                    speed = 2500;
                }

                MW_CMD_OUTPUT("%5d/%2d %8s %10d %11s %11s \n", unit, port,
                        "up", speed, ((AIR_PORT_DUPLEX_FULL == ps.duplex) ? "full" : "half"),
                        ((ps.flags & (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)) ? "on" : "off"));
            }
            else
            {
                MW_CMD_OUTPUT("%5d/%2d %8s %10s %11s %11s \n", unit, port,
                        "down", "---", "---", "---");
            }
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u link status error(%d)\n", port, rc);
            break;
        }
    }
    MW_CMD_OUTPUT("\n");

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showPhyLink(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    UI32_T speed = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    SFP_PHY_LINK_STATUS_T status;

    /*
     * Command format
     * port show link [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("sfp port link status      :\n");
    MW_CMD_OUTPUT("unit %u\n", unit);
    MW_CMD_OUTPUT("%10s %6s %10s %11s\n",
            "unit/port", "link", "speed", "duplex");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_phy_getLinkStatus(unit, port, &status);
        if(AIR_E_OK == rc)
        {
            if(status.flags & SFP_PHY_LINK_STATUS_FLAGS_LINK_UP)
            {
                if(AIR_PORT_SPEED_10M == status.speed)
                {
                    speed = 10;
                }
                else if(AIR_PORT_SPEED_100M == status.speed)
                {
                    speed = 100;
                }
                else if(AIR_PORT_SPEED_1000M == status.speed)
                {
                    speed = 1000;
                }
                else
                {
                    speed = 2500;
                }

                MW_CMD_OUTPUT("%5d/%2d %8s %10d %11s \n", unit, port,
                        "up", speed, ((AIR_PORT_DUPLEX_FULL == status.duplex) ? "full" : "half"));
            }
            else
            {
                MW_CMD_OUTPUT("%5d/%2d %8s %10s %11s\n", unit, port,
                        "down", "---", "---");
            }
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u link status error(%d)\n", port, rc);
            break;
        }
    }
    MW_CMD_OUTPUT("\n");

    return rc;
}


static MW_ERROR_NO_T
_sfp_cmd_setAdminState(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T str[MW_CMD_CMD_MAX_LENGTH] = {0};
    BOOL_T state = FALSE;

    /*
     * Command format
     * port set admin-state [ unit=<UINT> ] portlist=<UINTLIST> state={ enable | disable }
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getString(tokens, token_idx, "state", str), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    if(MW_E_OK == mw_cmd_checkString(str, "enable"))
    {
        state = TRUE;
    }
    else if(MW_E_OK != mw_cmd_checkString(str, "disable"))
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = sfp_port_setAdminState(unit, port, state);
        if(AIR_E_OK != rc)
        {
            MW_CMD_OUTPUT("***Error***, %s port=%u error(%d)\n", ((TRUE == state) ? "enable" : "disable"), port, rc);
            break;
        }
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_showAdminState(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T state = FALSE;

    /*
     * Command format
     * port show admin-state [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    MW_CMD_CHECK_OPT(mw_cmd_getUint(tokens, token_idx, "unit", &unit), tokens, 2);
    MW_CMD_CHECK_PARAM(mw_cmd_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    MW_CMD_OUTPUT("%10s %13s\n", "unit/port", "admin-state");
    AIR_PORT_FOREACH(pbm, port)
    {
        MW_CMD_OUTPUT("%5d/%2d", unit, port);
        rc = sfp_port_getAdminState(unit, port, &state);
        if(AIR_E_OK == rc)
        {
            MW_CMD_OUTPUT(" %15s", (TRUE == state) ? "enable" : "disable");
        }
        else
        {
            MW_CMD_OUTPUT("***Error***, show port=%u status error(%d)\n", port, rc);
            break;
        }
        MW_CMD_OUTPUT("\n");
    }

    return rc;
}

static MW_ERROR_NO_T
_sfp_cmd_start(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    /*
     * Command format
     * sfp start task
     */

    /* paser tokens */
    if(MW_E_OK == mw_cmd_checkString(tokens[token_idx], "task"))
    {
        token_idx++;
    }

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    return sfp_cmd_startSFP();
}

static MW_ERROR_NO_T
_sfp_cmd_stop(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    /*
     * Command format
     * sfp stop task
     */

    /* paser tokens */
    if(MW_E_OK == mw_cmd_checkString(tokens[token_idx], "task"))
    {
        token_idx++;
    }

    MW_CMD_CHECK_LAST_TOKEN(tokens[token_idx]);

    return sfp_cmd_stopSFP();
}

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_sfp_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: SFP.
 *
 * INPUT:
 *      tokens      --  Command tokens
 *      token_idx   --  The index of 1st valid token
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_cmd_sfp_dispatcher(
    const C8_T *tokens[],
    UI32_T token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _mw_cmd_sfp_vec, sizeof(_mw_cmd_sfp_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_sfp_usager
 * PURPOSE:
 *      Command usage for magic wand command: SFP.
 *
 * INPUT:
 *      None
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      MW_E_OK
 *
 * NOTES:
 *      None
 */
MW_ERROR_NO_T
mw_cmd_sfp_usager(
    void)
{
    return (mw_cmd_usager(_mw_cmd_sfp_vec, sizeof(_mw_cmd_sfp_vec)/sizeof(MW_CMD_VEC_T)));
}

#endif /* AIR_SUPPORT_SFP */

