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
#include <cmd/port_cmd.h>

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
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
_port_cmd_setSerdesMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 unit = 0, port = 0;
    AIR_PORT_SERDES_MODE_T mode = AIR_PORT_SERDES_MODE_SGMII;
    AIR_PORT_BITMAP_T      pbm = {0};
    C8_T                   str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port set serdes-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ sgmii | 1000base-x | hsgmii }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "sgmii"))
    {
        mode = AIR_PORT_SERDES_MODE_SGMII;
    }
    else if (AIR_E_OK == dsh_checkString(str, "1000base-x"))
    {
        mode = AIR_PORT_SERDES_MODE_1000BASE_X;
    }
    else if (AIR_E_OK == dsh_checkString(str, "100base-fx"))
    {
        mode = AIR_PORT_SERDES_MODE_100BASE_FX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "hsgmii"))
    {
        mode = AIR_PORT_SERDES_MODE_HSGMII;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setSerdesMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_printf("***Error***, set port=%u serdes mode not support(%d)\n", port, rc);
            }
            else
            {
                osal_printf("***Error***, set port=%u serdes mode error(%d)\n", port, rc);
            }

            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showSerdesMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T         rc = AIR_E_OK;
    UI32_T                 unit = 0, port = 0;
    AIR_PORT_SERDES_MODE_T mode = AIR_PORT_SERDES_MODE_SGMII;
    AIR_PORT_BITMAP_T      pbm = {0};

    /*
     * Command format
     * port show serdes-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %12s\n", "unit/port", "serdes-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getSerdesMode(unit, port, &mode);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5d/%2d", unit, port);
            if (AIR_PORT_SERDES_MODE_SGMII == mode)
            {
                osal_printf(" %14s", "sgmii");
            }
            else if (AIR_PORT_SERDES_MODE_HSGMII == mode)
            {
                osal_printf(" %14s", "hsgmii");
            }
            else if (AIR_PORT_SERDES_MODE_100BASE_FX == mode)
            {
                osal_printf(" %14s", "100base-fx");
            }
            else
            {
                osal_printf(" %14s", "1000base-x");
            }
        }
        else
        {
            osal_printf("***Error***, show port=%u serdes mode error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setAnMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T            enable = FALSE;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port set an-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

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
        rc = air_port_setPhyAutoNego(unit, port, enable);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u anMode error(%d)\n", port, rc);
            break;
        }
        air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showAnMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_PHY_AN_T auto_nego;

    /*
     * Command format
     * port show an-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %11s\n", "unit/port", "auto-nego");
    AIR_PORT_FOREACH(pbm, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            /* Skip cpu port */
            continue;
        }
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getPhyAutoNego(unit, port, &auto_nego);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %13s", (AIR_PORT_PHY_AN_ENABLE == auto_nego) ? "enable" : "disable");
        }
        else
        {
            osal_printf("***Error***, show anMode error(%d)\n", rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLocalAdv(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    C8_T                  str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_BITMAP_T     pbm = {0};
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
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);

    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "10h", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "10f", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "100h", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "100f", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "1000f", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "2500f", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_2500M;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_2500M);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pause", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= (AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE | AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE | AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "eee", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        adv.flags |= AIR_PORT_PHY_AN_ADV_FLAGS_EEE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        adv.flags &= ~(AIR_PORT_PHY_AN_ADV_FLAGS_EEE);
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setPhyLocalAdvAbility(unit, port, &adv);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u localAdv error(%d)\n", port, rc);
            break;
        }
        air_port_setPhyAutoNego(unit, port, AIR_PORT_PHY_AN_RESTART);
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLocalAdv(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_PORT_PHY_AN_ADV_T adv;

    /*
     * Command format
     * port show local-adv [ unit=<UINT> ] portlist=<UINTLIST>
     */

    osal_memset(&adv, 0, sizeof(AIR_PORT_PHY_AN_ADV_T));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s %8s %8s %8s %8s %8s %8s %8s\n", "unit/port", "10h", "10f", "100h", "100f", "1000f", "2500f",
                "pause", "eee");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getPhyLocalAdvAbility(unit, port, &adv);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5d/%2d", unit, port);
            if (0 == adv.flags)
            {
                osal_printf(" %10s %8s %8s %8s %8s %8s %8s %8s", "---", "---", "---", "---", "---", "---", "---",
                            "---");
            }
            else
            {
                osal_printf(" %10s", (AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_2500M & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", ((AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE & adv.flags) ||
                                     (AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE & adv.flags)) ?
                                        "enable" :
                                        "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_EEE & adv.flags) ? "enable" : "disable");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%5d/%2d %10s %8s %8s %8s %8s %8s %8s %8s", unit, port, "---", "---", "---", "---", "---",
                        "---", "---", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u localAdv error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showRemoteAdv(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_BITMAP_T     pbm = {0};
    AIR_PORT_PHY_AN_ADV_T adv;

    /*
     * Command format
     * port show remote-adv [ unit=<UINT> ] portlist=<UINTLIST>
     */

    osal_memset(&adv, 0, sizeof(AIR_PORT_PHY_AN_ADV_T));

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s %8s %8s %8s %8s %8s %8s %8s\n", "unit/port", "10h", "10f", "100h", "100f", "1000f", "2500f",
                "pause", "eee");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getPhyRemoteAdvAbility(unit, port, &adv);
        if (AIR_E_OK == rc)
        {
            osal_printf("%5d/%2d", unit, port);
            if (0 == adv.flags)
            {
                osal_printf(" %10s %8s %8s %8s %8s %8s %8s %8s", "---", "---", "---", "---", "---", "---", "---",
                            "---");
            }
            else
            {
                osal_printf(" %10s", (AIR_PORT_PHY_AN_ADV_FLAGS_10HFDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_10FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100HFDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_100FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_1000FUDX & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_2500M & adv.flags) ? "enable" : "disable");
                osal_printf(" %8s", ((AIR_PORT_PHY_AN_ADV_FLAGS_SYM_PAUSE & adv.flags) ||
                                     (AIR_PORT_PHY_AN_ADV_FLAGS_ASYM_PAUSE & adv.flags)) ?
                                        "enable" :
                                        "disable");
                osal_printf(" %8s", (AIR_PORT_PHY_AN_ADV_FLAGS_EEE & adv.flags) ? "enable" : "disable");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("%5d/%2d %10s %8s %8s %8s %8s %8s %8s %8s", unit, port, "---", "---", "---", "---", "---",
                        "---", "---", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u remoteAdv error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setSpeed(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_SPEED_T  speed = AIR_PORT_SPEED_LAST;

    /*
     * Command format
     * port set speed [ unit=<UINT> ] portlist=<UINTLIST> speed={ 10m | 100m | 1000m | 2500m }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "speed", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "10m"))
    {
        speed = AIR_PORT_SPEED_10M;
    }
    else if (AIR_E_OK == dsh_checkString(str, "100m"))
    {
        speed = AIR_PORT_SPEED_100M;
    }
    else if (AIR_E_OK == dsh_checkString(str, "1000m"))
    {
        speed = AIR_PORT_SPEED_1000M;
    }
    else if (AIR_E_OK == dsh_checkString(str, "2500m"))
    {
        speed = AIR_PORT_SPEED_2500M;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setSpeed(unit, port, speed);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u speed error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showSpeed(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_SPEED_T  speed = 0;

    /*
     * Command format
     * port show speed [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s\n", "unit/port", "speed");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getSpeed(unit, port, &speed);
        if (AIR_E_OK == rc)
        {
            if (AIR_PORT_SPEED_10M == speed)
            {
                osal_printf(" %10s", "10m");
            }
            else if (AIR_PORT_SPEED_100M == speed)
            {
                osal_printf(" %10s", "100m");
            }
            else if (AIR_PORT_SPEED_1000M == speed)
            {
                osal_printf(" %10s", "1000m");
            }
            else if (AIR_PORT_SPEED_2500M == speed)
            {
                osal_printf(" %10s", "2500m");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %10s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u speed error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setDuplex(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_DUPLEX_T duplex = AIR_PORT_DUPLEX_LAST;

    /*
     * Command format
     * port set duplex [ unit=<UINT> ] portlist=<UINTLIST> duplex={ full | half }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "duplex", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "full"))
    {
        duplex = AIR_PORT_DUPLEX_FULL;
    }
    else if (AIR_E_OK == dsh_checkString(str, "half"))
    {
        duplex = AIR_PORT_DUPLEX_HALF;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setDuplex(unit, port, duplex);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u duplex error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showDuplex(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_DUPLEX_T duplex = AIR_PORT_DUPLEX_LAST;

    /*
     * Command format
     * port show duplex [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %8s\n", "unit/port", "duplex");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getDuplex(unit, port, &duplex);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %10s", (AIR_PORT_DUPLEX_HALF == duplex) ? "half" : "full");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %10s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u duplex error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setBckPres(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T            bckPres = FALSE;

    /*
     * Command format
     * port set back-pres [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        bckPres = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        bckPres = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setBackPressure(unit, port, bckPres);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u back pressure error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showBckPres(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T            bckPres = FALSE;

    /*
     * Command format
     * port show back-pres [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %15s\n", "unit/port", "back-pressure");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getBackPressure(unit, port, &bckPres);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %17s", (TRUE == bckPres) ? "enable" : "disable");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %17s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u back pressure error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setFlowCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T              sdir[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_DIR_T    dir = AIR_PORT_DIR_TX;
    BOOL_T            mode = FALSE;

    /*
     * Command format
     * port set flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST> dir={ rx | tx } mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", sdir), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdir, "rx"))
    {
        dir = AIR_PORT_DIR_RX;
    }
    else if (AIR_E_OK == dsh_checkString(sdir, "tx"))
    {
        dir = AIR_PORT_DIR_TX;
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
        rc = air_port_setFlowCtrl(unit, port, dir, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u flow control error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showFlowCtrl(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T            state = FALSE;

    /*
     * Command format
     * port show flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %13s %13s\n", "unit/port", "rx-flowctrl", "tx-flowctrl");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getFlowCtrl(unit, port, AIR_PORT_DIR_RX, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %15s", (TRUE == state) ? "enable" : "disable");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %15s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u flow control error(%d)\n", port, rc);
            break;
        }
        rc = air_port_getFlowCtrl(unit, port, AIR_PORT_DIR_TX, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %13s", (TRUE == state) ? "enable" : "disable");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %13s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u flow control error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLpBack(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0, port = 0;
    AIR_PORT_BITMAP_T   pbm = {0};
    C8_T                str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                sdir[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_LPBK_DIR_T dir = AIR_PORT_LPBK_DIR_LAST;
    BOOL_T              mode = FALSE;

    /*
     * Command format
     * port set loop-back [ unit=<UINT> ] portlist=<UINTLIST> dir={ far | near } mode={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "dir", sdir), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(sdir, "far"))
    {
        dir = AIR_PORT_LPBK_DIR_FAR_END;
    }
    else if (AIR_E_OK == dsh_checkString(sdir, "near"))
    {
        dir = AIR_PORT_LPBK_DIR_NEAR_END;
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
        rc = air_port_setPhyLoopBack(unit, port, dir, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u loopback error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLpBack(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T            state = FALSE;

    /*
     * Command format
     * port show loop-back [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %14s %15s\n", "unit/port", "loopback-far", "loopback-near");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getPhyLoopBack(unit, port, AIR_PORT_LPBK_DIR_FAR_END, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %16s", (TRUE == state) ? "enable" : "disable");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %16s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u loop back error(%d)\n", port, rc);
            break;
        }
        rc = air_port_getPhyLoopBack(unit, port, AIR_PORT_LPBK_DIR_NEAR_END, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %15s", (TRUE == state) ? "enable" : "disable");
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %15s", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u loop back error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLink(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    UI32_T            speed = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    AIR_PORT_STATUS_T ps;

    /*
     * Command format
     * port show link [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("air port link status      :\n");
    osal_printf("unit %u\n", unit);
    osal_printf("%10s %6s %10s %11s %11s %11s\n", "unit/port", "link", "speed", "duplex", "flow-ctrl", "eee");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getPortStatus(unit, port, &ps);
        if (AIR_E_OK == rc)
        {
            if (ps.flags & AIR_PORT_STATUS_FLAGS_LINK_UP)
            {
                if (AIR_PORT_SPEED_10M == ps.speed)
                {
                    speed = 10;
                }
                else if (AIR_PORT_SPEED_100M == ps.speed)
                {
                    speed = 100;
                }
                else if (AIR_PORT_SPEED_1000M == ps.speed)
                {
                    speed = 1000;
                }
                else if (AIR_PORT_SPEED_2500M == ps.speed)
                {
                    speed = 2500;
                }
                else
                {
                    speed = 5000;
                }

                osal_printf(
                    "%5d/%2d %8s %10d %11s %11s %11s\n", unit, port, "up", speed,
                    ((AIR_PORT_DUPLEX_FULL == ps.duplex) ? "full" : "half"),
                    (!!(ps.flags & (AIR_PORT_STATUS_FLAGS_FLOW_CTRL_TX | AIR_PORT_STATUS_FLAGS_FLOW_CTRL_RX)) ? "on" :
                                                                                                                "off"),
                    (!!(ps.flags & (AIR_PORT_STATUS_FLAGS_EEE)) ? "on" : "off"));
            }
            else
            {
                osal_printf("%5d/%2d %8s %10s %11s %11s %11s\n", unit, port, "down", "---", "---", "---", "---");
            }
        }
        else
        {
            osal_printf("***Error***, show port=%u link status error(%d)\n", port, rc);
            break;
        }
    }
    osal_printf("\n");

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setAdminState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T            state = FALSE;

    /*
     * Command format
     * port set admin-state [ unit=<UINT> ] portlist=<UINTLIST> state={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "enable"))
    {
        state = TRUE;
    }
    else if (AIR_E_OK != dsh_checkString(st, "disable"))
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setAdminState(unit, port, state);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, %s port=%u error(%d)\n", ((TRUE == state) ? "enable" : "disable"), port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showAdminState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};
    BOOL_T            state = FALSE;

    /*
     * Command format
     * port show admin-state [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %13s\n", "unit/port", "admin-state");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getAdminState(unit, port, &state);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %15s", (TRUE == state) ? "enable" : "disable");
        }
        else
        {
            osal_printf("***Error***, show port=%u status error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setSmtSpdDwn(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0, port = 0;
    AIR_PORT_BITMAP_T   pbm = {0};
    C8_T                st[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_SSD_MODE_T retry = 0;

    /*
     * Command format
     * port set smart-speed-down [ unit=<UINT> ] portlist=<UINTLIST>
     * mode={ disable | retry2 | retry3 | retry4 | retry5 }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "disable"))
    {
        retry = AIR_PORT_SSD_MODE_DISABLE;
    }
    else if (AIR_E_OK == dsh_checkString(st, "retry2"))
    {
        retry = AIR_PORT_SSD_MODE_2T;
    }
    else if (AIR_E_OK == dsh_checkString(st, "retry3"))
    {
        retry = AIR_PORT_SSD_MODE_3T;
    }
    else if (AIR_E_OK == dsh_checkString(st, "retry4"))
    {
        retry = AIR_PORT_SSD_MODE_4T;
    }
    else if (AIR_E_OK == dsh_checkString(st, "retry5"))
    {
        retry = AIR_PORT_SSD_MODE_5T;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setPhySmartSpeedDown(unit, port, retry);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u smart speed down error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_prot_cmd_showSmtSpdDwn(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    UI32_T              unit = 0, port = 0;
    AIR_PORT_BITMAP_T   pbm = {0};
    AIR_PORT_SSD_MODE_T retry = 0;

    /*
     * Command format
     * port show smart-speed-down [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %9s %12s\n", "unit/port", "mode", "retry-time");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getPhySmartSpeedDown(unit, port, &retry);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %11s", (AIR_PORT_SSD_MODE_DISABLE != retry) ? "enable" : "disable");
            if (AIR_PORT_SSD_MODE_DISABLE != retry)
            {
                osal_printf(" %12d", retry);
            }
            else
            {
                osal_printf(" %12s", "---");
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf(" %11s %12s", "---", "---");
        }
        else
        {
            osal_printf("***Error***, show port=%u smart speed down error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setPortMatrix(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0}, matrix = {0};

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "matrix", unit, &matrix), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setPortMatrix(unit, port, matrix);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port matrix error(%d)\n", rc);
            return rc;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showPortMatrix(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0}, matrix = {0};

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s   %-16s\n", "unit/port", "port-matrix");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getPortMatrix(unit, port, matrix);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get port matrix error(%d)\n", rc);
            return rc;
        }
        osal_printf("%5d/%2d%5s", unit, port, "");
        CMD_PRINT_PORTLIST(matrix);
        osal_printf("\n");
    }
    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setVlanMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    C8_T                 str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T               unit = 0, port = 0;
    AIR_PORT_VLAN_MODE_T mode = AIR_PORT_VLAN_MODE_LAST;
    AIR_PORT_BITMAP_T    pbm = {0};

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(str, "matrix"))
    {
        mode = AIR_PORT_VLAN_MODE_PORT_MATRIX;
    }
    else if (AIR_E_OK == dsh_checkString(str, "fallback"))
    {
        mode = AIR_PORT_VLAN_MODE_FALLBACK;
    }
    else if (AIR_E_OK == dsh_checkString(str, "check"))
    {
        mode = AIR_PORT_VLAN_MODE_CHECK;
    }
    else if (AIR_E_OK == dsh_checkString(str, "security"))
    {
        mode = AIR_PORT_VLAN_MODE_SECURITY;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setVlanMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port vlan mode error(%d)\n", rc);
            return rc;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showVlanMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T       rc = AIR_E_OK;
    UI32_T               unit = 0, port = 0;
    AIR_PORT_VLAN_MODE_T mode;
    AIR_PORT_BITMAP_T    pbm = {0};

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %11s\n", "unit/port", "vlan-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_getVlanMode(unit, port, &mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get port vlan mode error(%d)\n", rc);
            return rc;
        }

        osal_printf("%5d/%2d", unit, port);

        switch (mode)
        {
            case AIR_PORT_VLAN_MODE_PORT_MATRIX:
                osal_printf(" %13s", "matrix");
                break;
            case AIR_PORT_VLAN_MODE_FALLBACK:
                osal_printf(" %13s", "fallback");
                break;
            case AIR_PORT_VLAN_MODE_CHECK:
                osal_printf(" %13s", "check");
                break;
            case AIR_PORT_VLAN_MODE_SECURITY:
                osal_printf(" %13s", "security");
                break;
            default:
                osal_printf(" %13s", "unknown");
                break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLedOn(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T            state;

    /*
     * Command format
     * port set led-ctrl [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST> force-on={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "force-on", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "enable"))
    {
        state = TRUE;
    }
    else if (AIR_E_OK == dsh_checkString(st, "disable"))
    {
        state = FALSE;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_setPhyLedOnCtrl(unit, port, led_id, state);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port=%u LED=%d state error(%d)\n", port, led_id, rc);
                return rc;
            }
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLedOn(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0, led_num = 0, print_cnt = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    BOOL_T            state;
    AIR_CFG_VALUE_T   led_count;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));

    /*
     * Command format
     * port show led-ctrl [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_COUNT(lbm, led_num, AIR_PORT_BITMAP_SIZE);

    air_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    if (led_num > led_count.value)
    {
        return AIR_E_BAD_PARAMETER;
    }

    osal_printf("%10s", "unit/port");
    AIR_PORT_FOREACH(lbm, led_id)
    {
        osal_snprintf(str, DSH_CMD_MAX_LENGTH, "led-%d", led_id);
        osal_printf(" %9s", str);
    }
    osal_printf("\n");

    AIR_PORT_FOREACH(pbm, port)
    {
        print_cnt = 0;
        osal_printf("%5d/%2d", unit, port);
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_getPhyLedOnCtrl(unit, port, led_id, &state);
            if (AIR_E_OK == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, (TRUE == state) ? "on" : "off");
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "---");
            }
            else
            {
                osal_printf("***Error***, show port=%u LED=%d state error(%d)\n", port, led_id, rc);
                return rc;
            }
            if (0 == print_cnt)
            {
                osal_printf(" %11s", str);
            }
            else
            {
                osal_printf(" %9s", str);
            }
            print_cnt++;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setComboMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_COMBO_MODE_T mode = AIR_PORT_COMBO_MODE_SERDES;
    AIR_PORT_BITMAP_T     pbm = {0};
    C8_T                  str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port set combo-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ serdes | phy }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "serdes"))
    {
        mode = AIR_PORT_COMBO_MODE_SERDES;
    }
    else if (AIR_E_OK == dsh_checkString(str, "phy"))
    {
        mode = AIR_PORT_COMBO_MODE_PHY;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setComboMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u combo mode error(%d)\n", port, rc);
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showComboMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T        rc = AIR_E_OK;
    UI32_T                unit = 0, port = 0;
    AIR_PORT_COMBO_MODE_T mode = AIR_PORT_COMBO_MODE_SERDES;
    AIR_PORT_BITMAP_T     pbm = {0};

    /*
     * Command format
     * port show combo-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %12s\n", "unit/port", "combo-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getComboMode(unit, port, &mode);
        if (AIR_E_OK == rc)
        {
            osal_printf(" %14s", (AIR_PORT_COMBO_MODE_SERDES == mode) ? "serdes" : "phy");
        }
        else
        {
            osal_printf("***Error***, show port=%u combo mode error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLedCtrlMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T            state;

    /*
     * Command format
     * port set led-ctrl-mode [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST> force-mode={ phy | force }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "force-mode", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "force"))
    {
        state = AIR_PORT_PHY_LED_CTRL_MODE_FORCE;
    }
    else if (AIR_E_OK == dsh_checkString(st, "phy"))
    {
        state = AIR_PORT_PHY_LED_CTRL_MODE_PHY;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_setPhyLedCtrlMode(unit, port, led_id, state);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port=%u LED=%d control mode error(%d)\n", port, led_id, rc);
                return rc;
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLedCtrlMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0, led_num = 0, print_cnt = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    UI32_T            state;
    AIR_CFG_VALUE_T   led_count;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));

    /*
     * Command format
     * port show led-force-state [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_COUNT(lbm, led_num, AIR_PORT_BITMAP_SIZE);

    air_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    if (led_num > led_count.value)
    {
        return AIR_E_BAD_PARAMETER;
    }

    osal_printf("%10s", "unit/port");
    AIR_PORT_FOREACH(lbm, led_id)
    {
        osal_snprintf(str, DSH_CMD_MAX_LENGTH, "led-%d", led_id);
        osal_printf(" %9s", str);
    }
    osal_printf("\n");

    AIR_PORT_FOREACH(pbm, port)
    {
        print_cnt = 0;
        osal_printf("%5d/%2d", unit, port);
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_getPhyLedCtrlMode(unit, port, led_id, &state);
            if (AIR_E_OK == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, (AIR_PORT_PHY_LED_CTRL_MODE_FORCE == state) ? "force" : "phy");
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "---");
            }
            else
            {
                osal_printf("***Error***, show port=%u LED=%d control mode error(%d)\n", port, led_id, rc);
                return rc;
            }
            if (0 == print_cnt)
            {
                osal_printf(" %11s", str);
            }
            else
            {
                osal_printf(" %9s", str);
            }
            print_cnt++;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLedForceState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T            state;

    /*
     * Command format
     * port set led-force-state [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST> state={ pattern | off | on }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "pattern"))
    {
        state = AIR_PORT_PHY_LED_STATE_FORCE_PATT;
    }
    else if (AIR_E_OK == dsh_checkString(st, "off"))
    {
        state = AIR_PORT_PHY_LED_STATE_OFF;
    }
    else if (AIR_E_OK == dsh_checkString(st, "on"))
    {
        state = AIR_PORT_PHY_LED_STATE_ON;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_setPhyLedForceState(unit, port, led_id, state);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port=%u LED=%d force state error(%d)\n", port, led_id, rc);
                return rc;
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLedForceState(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0, led_num = 0, print_cnt = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    UI32_T            state;
    AIR_CFG_VALUE_T   led_count;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));

    /*
     * Command format
     * port show led-force-state [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_COUNT(lbm, led_num, AIR_PORT_BITMAP_SIZE);

    air_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    if (led_num > led_count.value)
    {
        return AIR_E_BAD_PARAMETER;
    }

    osal_printf("%10s", "unit/port");
    AIR_PORT_FOREACH(lbm, led_id)
    {
        osal_snprintf(str, DSH_CMD_MAX_LENGTH, "led-%d", led_id);
        osal_printf(" %10s", str);
    }
    osal_printf("\n");

    AIR_PORT_FOREACH(pbm, port)
    {
        print_cnt = 0;
        osal_printf("%5d/%2d", unit, port);
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_getPhyLedForceState(unit, port, led_id, &state);
            if (AIR_E_OK == rc)
            {
                if (AIR_PORT_PHY_LED_STATE_FORCE_PATT == state)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "pattern");
                }
                else if (AIR_PORT_PHY_LED_STATE_OFF == state)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "off");
                }
                else if (AIR_PORT_PHY_LED_STATE_ON == state)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "on");
                }
                else
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "unknown");
                }
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "---");
            }
            else
            {
                osal_printf("***Error***, show port=%u LED=%d force state error(%d)\n", port, led_id, rc);
                return rc;
            }
            if (0 == print_cnt)
            {
                osal_printf(" %12s", str);
            }
            else
            {
                osal_printf(" %10s", str);
            }
            print_cnt++;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setLedForcePatt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    C8_T              st[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T            state;

    /*
     * Command format
     * port set led-force-pattern [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST> pattern={ 0.5hz | 1hz | 2hz }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pattern", st), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(st, "0.5hz"))
    {
        state = AIR_PORT_PHY_LED_PATT_HZ_HALF;
    }
    else if (AIR_E_OK == dsh_checkString(st, "1hz"))
    {
        state = AIR_PORT_PHY_LED_PATT_HZ_ONE;
    }
    else if (AIR_E_OK == dsh_checkString(st, "2hz"))
    {
        state = AIR_PORT_PHY_LED_PATT_HZ_TWO;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_setPhyLedForcePattCfg(unit, port, led_id, state);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, set port=%u LED=%d force pattern error(%d)\n", port, led_id, rc);
                return rc;
            }
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showLedForcePatt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, led_id = 0, led_num = 0, print_cnt = 0;
    AIR_PORT_BITMAP_T pbm = {0}, lbm = {0};
    UI32_T            pattern;
    AIR_CFG_VALUE_T   led_count;
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};

    osal_memset(&led_count, 0, sizeof(AIR_CFG_VALUE_T));

    /*
     * Command format
     * port show led-force-pattern [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "ledlist", unit, &lbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_COUNT(lbm, led_num, AIR_PORT_BITMAP_SIZE);

    air_cfg_getValue(unit, AIR_CFG_TYPE_PHY_LED_COUNT, &led_count);
    if (led_num > led_count.value)
    {
        return AIR_E_BAD_PARAMETER;
    }

    osal_printf("%10s", "unit/port");
    AIR_PORT_FOREACH(lbm, led_id)
    {
        osal_snprintf(str, DSH_CMD_MAX_LENGTH, "led-%d", led_id);
        osal_printf(" %9s", str);
    }
    osal_printf("\n");

    AIR_PORT_FOREACH(pbm, port)
    {
        print_cnt = 0;
        osal_printf("%5d/%2d", unit, port);
        AIR_PORT_FOREACH(lbm, led_id)
        {
            rc = air_port_getPhyLedForcePattCfg(unit, port, led_id, &pattern);
            if (AIR_E_OK == rc)
            {
                if (AIR_PORT_PHY_LED_PATT_HZ_HALF == pattern)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "0.5hz");
                }
                else if (AIR_PORT_PHY_LED_PATT_HZ_ONE == pattern)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "1hz");
                }
                else if (AIR_PORT_PHY_LED_PATT_HZ_TWO == pattern)
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "2hz");
                }
                else
                {
                    osal_snprintf(str, DSH_CMD_MAX_LENGTH, "unknown");
                }
            }
            else if (AIR_E_NOT_SUPPORT == rc)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "---");
            }
            else
            {
                osal_printf("***Error***, show port=%u LED=%d force pattern error(%d)\n", port, led_id, rc);
                return rc;
            }
            if (0 == print_cnt)
            {
                osal_printf(" %11s", str);
            }
            else
            {
                osal_printf(" %9s", str);
            }
            print_cnt++;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_triggerCableTest(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     unit = 0, port = 0, pair = 0;
    AIR_PORT_BITMAP_T          pbm = {0};
    C8_T                       str[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                       str_out[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                       str_enable[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_CABLE_TEST_RSLT_T cable;
    UI8_T                      i = 0;
    AIR_TIME_T                 beg = 0, end = 0;

    /*
     * Command format
     * port trigger cable-test [ unit=<UINT> ] portlist=<UINTLIST> pair={ a | b | c | d | all }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", str), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "cost-time", str_enable), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&cable, 0, sizeof(AIR_PORT_CABLE_TEST_RSLT_T));
    if (AIR_E_OK == dsh_checkString(str, "a"))
    {
        pair = AIR_PORT_CABLE_TEST_PAIR_A;
    }
    else if (AIR_E_OK == dsh_checkString(str, "b"))
    {
        pair = AIR_PORT_CABLE_TEST_PAIR_B;
    }
    else if (AIR_E_OK == dsh_checkString(str, "c"))
    {
        pair = AIR_PORT_CABLE_TEST_PAIR_C;
    }
    else if (AIR_E_OK == dsh_checkString(str, "d"))
    {
        pair = AIR_PORT_CABLE_TEST_PAIR_D;
    }
    else if (AIR_E_OK == dsh_checkString(str, "all"))
    {
        pair = AIR_PORT_CABLE_TEST_PAIR_ALL;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    if (AIR_PORT_CABLE_TEST_PAIR_ALL == pair)
    {
        osal_printf("%18s %15s %15s %15s\n", "pair-a", "pair-b", "pair-c", "pair-d");
        osal_printf("%10s %7s %7s %7s %7s %7s %7s %7s %7s\n", "unit/port", "status", "length", "status", "length",
                    "status", "length", "status", "length");
    }
    else
    {
        osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "pair-%s", str);
        osal_printf("%18s\n", str_out);
        osal_printf("%10s %7s %7s\n", "unit/port", "status", "length");
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        osal_getTime(&beg);
        rc = air_port_triggerCableTest(unit, port, pair, &cable);
        osal_getTime(&end);
        if (AIR_E_OK == rc)
        {
            if (AIR_PORT_CABLE_TEST_PAIR_ALL == pair)
            {
                for (i = 0; i < AIR_PORT_CABLE_MAX_PAIR; i++)
                {
                    if (AIR_PORT_CABLE_STATUS_OPEN == cable.status[i])
                    {
                        osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "open");
                    }
                    else if (AIR_PORT_CABLE_STATUS_SHORT == cable.status[i])
                    {
                        osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "short");
                    }
                    else if (AIR_PORT_CABLE_STATUS_NORMAL == cable.status[i])
                    {
                        osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "normal");
                    }
                    if (0 == i)
                    {
                        osal_printf(" %9s", str_out);
                    }
                    else
                    {
                        osal_printf(" %7s", str_out);
                    }
                    osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "%d.%dm", (cable.length[i] / 10),
                                  (cable.length[i] % 10));
                    osal_printf(" %7s", str_out);
                }
            }
            else
            {
                if (AIR_PORT_CABLE_STATUS_OPEN == cable.status[pair])
                {
                    osal_printf(" %9s", "open");
                }
                else if (AIR_PORT_CABLE_STATUS_SHORT == cable.status[pair])
                {
                    osal_printf(" %9s", "short");
                }
                else if (AIR_PORT_CABLE_STATUS_NORMAL == cable.status[pair])
                {
                    osal_printf(" %9s", "normal");
                }
                osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "%d.%dm", (cable.length[pair] / 10),
                              (cable.length[pair] % 10));
                osal_printf(" %7s", str_out);
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("***Error***, only support cable test for 1G link-up speed and port must be enabled\n");
        }
        else if (AIR_E_TIMEOUT == rc)
        {
            osal_printf("***Error***, trigger cable-test time out\n");
        }
        else
        {
            osal_printf("***Error***, trigger cable-test error\n");
        }
        osal_printf("\n");
    }
    if (AIR_E_OK == dsh_checkString(str_enable, "enable"))
    {
        osal_printf("Cost Time = %dms\n", (end - beg));
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_setOpMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0, port = 0;
    AIR_PORT_BITMAP_T  pbm = {0};
    AIR_PORT_OP_MODE_T mode = AIR_PORT_OP_MODE_LAST;
    C8_T               str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port set phy-op-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ normal | long-reach }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "normal"))
    {
        mode = AIR_PORT_OP_MODE_NORMAL;
    }
    else if (AIR_E_OK == dsh_checkString(str, "long-reach"))
    {
        mode = AIR_PORT_OP_MODE_LONG_REACH;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(pbm, port)
    {
        rc = air_port_setPhyOpMode(unit, port, mode);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, set port=%u phy-op-mode error(%d)\n", port, rc);
            return rc;
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_showOpMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    UI32_T             unit = 0, port = 0;
    AIR_PORT_OP_MODE_T mode = AIR_PORT_OP_MODE_LAST;
    AIR_PORT_BITMAP_T  pbm = {0};
    C8_T               str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * port show phy-op-mode [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf("%10s %13s\n", "unit/port", "phy-op-mode");
    AIR_PORT_FOREACH(pbm, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            /* Skip cpu port */
            continue;
        }
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_getPhyOpMode(unit, port, &mode);
        if (AIR_E_OK == rc)
        {
            if (AIR_PORT_OP_MODE_NORMAL == mode)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "normal");
            }
            else if (AIR_PORT_OP_MODE_LONG_REACH == mode)
            {
                osal_snprintf(str, DSH_CMD_MAX_LENGTH, "long-reach");
            }
            osal_printf(" %15s", str);
        }
        else
        {
            osal_printf("***Error***, show port=%u phy-op-mode error(%d)\n", port, rc);
            break;
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_triggerLinkDownCableTest(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T             rc = AIR_E_OK;
    UI32_T                     unit = 0, port = 0, pair = 0;
    AIR_PORT_BITMAP_T          pbm = {0};
    C8_T                       str_out[DSH_CMD_MAX_LENGTH] = {0};
    AIR_PORT_CABLE_TEST_RSLT_T cable;

    /*
     * Command format
     * port trigger link-down-cable-test [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_memset(&cable, 0, sizeof(AIR_PORT_CABLE_TEST_RSLT_T));

    /* In fast cable test only return 1 pair result */

    osal_printf("%10s %7s %7s\n", "unit/port", "status", "length");

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("%5d/%2d", unit, port);
        rc = air_port_triggerLinkDownCableTest(unit, port, &cable);
        if (AIR_E_OK == rc)
        {
            if (AIR_PORT_CABLE_STATUS_OPEN == cable.status[pair])
            {
                osal_printf(" %9s", "open");
                osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "%dm", (cable.length[pair]));
                osal_printf(" %7s", str_out);
            }
            else if (AIR_PORT_CABLE_STATUS_SHORT == cable.status[pair])
            {
                osal_printf(" %9s", "short");
                osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "%dm", (cable.length[pair]));
                osal_printf(" %7s", str_out);
            }
            else if (AIR_PORT_CABLE_STATUS_NORMAL == cable.status[pair])
            {
                osal_printf(" %9s", "normal");
                osal_snprintf(str_out, DSH_CMD_MAX_LENGTH, "%s", "---");
                osal_printf(" %7s", str_out);
            }
        }
        else if (AIR_E_NOT_SUPPORT == rc)
        {
            osal_printf("***Error***, only support cable test in link down mode\n");
        }
        else
        {
            osal_printf("***Error***, trigger cable-test error\n");
        }
        osal_printf("\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_port_cmd_dumpDebug(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /*
     * Command format
     * port dump port-debug [ unit=<UINT> ] portlist=<UINTLIST>
     */

    /* paser token */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    AIR_PORT_FOREACH(pbm, port)
    {
        osal_printf("unit = %u, port = %u\n", unit, port);
        rc = HAL_FUNC_CALL(unit, port, dumpDebugInfo, (unit, port));
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, port dump parameter\n");
        }
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T _port_cmd_vec[] =
{
    {
        "set serdes-mode", 2, _port_cmd_setSerdesMode,
        "port set serdes-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ sgmii | 1000base-x | 100base-fx | hsgmii }\n"
    },
    {
        "show serdes-mode", 2, _port_cmd_showSerdesMode,
        "port show serdes-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set an-mode", 2, _port_cmd_setAnMode,
        "port set an-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show an-mode", 2, _port_cmd_showAnMode,
        "port show an-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set local-adv", 2, _port_cmd_setLocalAdv,
        "port set local-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "10h={ enable | disable }\n"
        "10f={ enable | disable }\n"
        "100h={ enable | disable }\n"
        "100f={ enable | disable }\n"
        "1000f={ enable | disable }\n"
        "2500f={ enable | disable }\n"
        "pause={ enable | disable }\n"
        "eee={ enable | disable }\n"
    },
    {
        "show local-adv", 2, _port_cmd_showLocalAdv,
        "port show local-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show remote-adv", 2, _port_cmd_showRemoteAdv,
        "port show remote-adv [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set speed", 2, _port_cmd_setSpeed,
        "port set speed [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "speed={ 10m | 100m | 1000m | 2500m }\n"
    },
    {
        "show speed", 2, _port_cmd_showSpeed,
        "port show speed [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set duplex", 2, _port_cmd_setDuplex,
        "port set duplex [ unit=<UINT> ] portlist=<UINTLIST> duplex={ full | half }\n"
    },
    {
        "show duplex", 2, _port_cmd_showDuplex,
        "port show duplex [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set back-pres", 2, _port_cmd_setBckPres,
        "port set back-pres [ unit=<UINT> ] portlist=<UINTLIST> mode={ enable | disable }\n"
    },
    {
        "show back-pres", 2, _port_cmd_showBckPres,
        "port show back-pres [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set flow-ctrl", 2, _port_cmd_setFlowCtrl,
        "port set flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST> dir={ rx | tx }\n"
        "mode={ enable | disable }\n"
    },
    {
        "show flow-ctrl", 2, _port_cmd_showFlowCtrl,
        "port show flow-ctrl [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set loop-back", 2, _port_cmd_setLpBack,
        "port set loop-back [ unit=<UINT> ] portlist=<UINTLIST> dir={ far | near }\n"
        "mode={ enable | disable }\n"
    },
    {
        "show loop-back", 2, _port_cmd_showLpBack,
        "port show loop-back [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "show link", 2, _port_cmd_showLink,
        "port show link [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set admin-state", 2, _port_cmd_setAdminState,
        "port set admin-state [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "state={ enable | disable }\n"
    },
    {
        "show admin-state", 2, _port_cmd_showAdminState,
        "port show admin-state [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set smart-speed-down", 2, _port_cmd_setSmtSpdDwn,
        "port set smart-speed-down [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ disable | retry2 | retry3 | retry4 | retry5 }\n"
    },
    {
        "show smart-speed-down", 2, _prot_cmd_showSmtSpdDwn,
        "port show smart-speed-down [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set port-matrix", 2, _port_cmd_setPortMatrix,
        "port set port-matrix [ unit=<UINT> ] portlist=<UINTLIST> matrix=<UINTLIST>\n"
    },
    {
        "show port-matrix", 2, _port_cmd_showPortMatrix,
        "port show port-matrix [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set vlan-mode", 2, _port_cmd_setVlanMode,
        "port set vlan-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ matrix | fallback | check | security }\n"
    },
    {
        "show vlan-mode", 2, _port_cmd_showVlanMode,
        "port show vlan-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set led-ctrl", 2, _port_cmd_setLedOn,
        "port set led-ctrl [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
        "force-on={ enable | disable }\n"
    },
    {
        "show led-ctrl", 2, _port_cmd_showLedOn,
        "port show led-ctrl [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
    },
    {
        "set combo-mode", 2, _port_cmd_setComboMode,
        "port set combo-mode [ unit=<UINT> ] portlist=<UINTLIST> mode={ serdes | phy }\n"
    },
    {
        "show combo-mode", 2, _port_cmd_showComboMode,
        "port show combo-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set led-ctrl-mode", 2, _port_cmd_setLedCtrlMode,
        "port set led-ctrl-mode [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
        "force-mode={ phy | force }\n"
    },
    {
        "show led-ctrl-mode", 2, _port_cmd_showLedCtrlMode,
        "port show led-ctrl-mode [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
    },
    {
        "set led-force-state", 2, _port_cmd_setLedForceState,
        "port set led-force-state [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
        "state={ pattern | off | on }\n"
    },
    {
        "show led-force-state", 2, _port_cmd_showLedForceState,
        "port show led-force-state [ unit=<UINT> ] portlist=<UINTLIST> ledlist=<UINTLIST>\n"
    },
    {
        "set led-force-pattern", 2, _port_cmd_setLedForcePatt,
        "port set led-force-pattern [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "ledlist=<UINTLIST> pattern={ 0.5hz | 1hz | 2hz }\n"
    },
    {
        "show led-force-pattern", 2, _port_cmd_showLedForcePatt,
        "port show led-force-pattern [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "ledlist=<UINTLIST>\n"
    },
    {
        "set phy-op-mode", 2, _port_cmd_setOpMode,
        "port set phy-op-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "mode={ normal | long-reach }\n"
    },
    {
        "show phy-op-mode", 2, _port_cmd_showOpMode,
        "port show phy-op-mode [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "trigger cable-test", 2, _port_cmd_triggerCableTest,
        "port trigger cable-test [ unit=<UINT> ] portlist=<UINTLIST>\n"
        "pair={ a | b | c | d | all }\n"
    },
    {
        "trigger link-down-cable-test", 2, _port_cmd_triggerLinkDownCableTest,
        "port trigger link-down-cable-test [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "dump port-debug", 2, _port_cmd_dumpDebug,
        "port dump port-debug [ unit=<UINT> ] portlist=<UINTLIST>\n"
    }
 };
/* clang-format on */

AIR_ERROR_NO_T
port_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _port_cmd_vec, sizeof(_port_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
port_cmd_usager()
{
    return (dsh_usager(_port_cmd_vec, sizeof(_port_cmd_vec) / sizeof(DSH_VEC_T)));
}
