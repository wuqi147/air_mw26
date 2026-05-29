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

#include <cmd/diag_cmd.h>

#include <air_error.h>
#include <air_init.h>
#include <air_port.h>
#include <air_swc.h>
#include <air_types.h>
#include <air_ver.h>
#include <aml/aml.h>
#include <cmlib/cmlib_hw_util.h>
#include <hal/common/hal.h>
#include <hal/common/hal_cmn_phy.h>
#include <hal/common/hal_phy.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

static C8_T *ptr_port_type_str[AIR_INIT_PORT_TYPE_LAST] = {"baset", "xsgmii", "cpu", "baset"};

/* -------------------------------------------------------------- internal functions */

static AIR_ERROR_NO_T
_diag_cmd_showInfoSdk(
    const UI32_T unit)
{
    C8_T version[10];
    osal_memset(version, 0, sizeof(version));

    if (!HAL_IS_UNIT_VALID(unit))
    {
        osal_printf("***Error***, unit %d is invalid.\n", unit);
        return (AIR_E_BAD_PARAMETER);
    }

    osal_printf("sdk version        : %s\n", AIR_VER_SDK);
#ifdef AIR_EN_CABLE_DIAG
    osal_printf("hw library version : %s\n", cmlib_hw_util_version());
#endif
    osal_printf("built time         : %s %s\n", __DATE__, __TIME__);

    return (AIR_E_OK);
}

static AIR_ERROR_NO_T
_diag_cmd_showInfoChip(
    const UI32_T unit)
{
    C8_T           buf_device_id[8];
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI64_T         chip_uid;

    if (!HAL_IS_UNIT_VALID(unit))
    {
        osal_printf("***Error***, unit %d is invalid.\n", unit);
        return (AIR_E_BAD_PARAMETER);
    }

    /* display chip information */
    osal_printf("family id       : 0x%04x\n", HAL_DEVICE_FAMILY_ID(unit));

    osal_printf("device id       : ");
    if (HAL_IS_DEVICE_SCO_FAMILY(unit))
    {
        switch (HAL_DEVICE_CHIP_ID(unit))
        {
            case HAL_SCO_DEVICE_ID_EN8851C:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8851c");
                break;
            case HAL_SCO_DEVICE_ID_EN8851E:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8851e");
                break;
            case HAL_SCO_DEVICE_ID_EN8853C:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8853c");
                break;
            case HAL_SCO_DEVICE_ID_EN8860C:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8860c");
                break;
            default:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "invalid");
        }
    }
    else if (HAL_IS_DEVICE_PEARL_FAMILY(unit))
    {
        switch (HAL_DEVICE_CHIP_ID(unit))
        {
            case HAL_PEARL_DEVICE_ID_AN8855M:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8855m");
                break;
            case HAL_PEARL_DEVICE_ID_AN8855H:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8855h");
                break;
            default:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "invalid");
        }
    }
    else if (HAL_IS_DEVICE_CORAL_FAMILY(unit))
    {
        switch (HAL_DEVICE_CHIP_ID(unit))
        {
            case HAL_CORAL_DEVICE_ID_AN8858C:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858c");
                break;
            case HAL_CORAL_DEVICE_ID_AN8858E:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858e");
                break;
            case HAL_CORAL_DEVICE_ID_AN8858H:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858h");
                break;
            case HAL_CORAL_DEVICE_ID_AN8858B:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858b");
                break;
            case HAL_CORAL_DEVICE_ID_AN8858F:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858f");
                break;
            case HAL_CORAL_DEVICE_ID_AN8858D:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "8858d");
                break;
            default:
                osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "invalid");
        }
    }
    else
    {
        osal_snprintf(buf_device_id, sizeof(buf_device_id), "%s", "invalid");
    }
    osal_printf("%s (0x%02x)\n", buf_device_id, HAL_DEVICE_CHIP_ID(unit));
    osal_printf("revision id     : 0x%02x\n", HAL_DEVICE_REV_ID(unit));
    rc = air_swc_getChipUid(unit, &chip_uid);
    if (AIR_E_OK == rc)
    {
        osal_printf("unique id       : 0x%08x%08x\n", UI64_HI(chip_uid), UI64_LOW(chip_uid));
    }
    else
    {
        osal_printf("unique id       : ---\n");
    }
    osal_printf("total port count: %d\n", HAL_TOTAL_PORT_NUM(unit));

    return (AIR_E_OK);
}

static AIR_ERROR_NO_T
_diag_cmd_showInfoPort(
    const UI32_T unit)
{
    UI32_T              word_cnt, port;
    HAL_SDK_PORT_MAP_T *ptr_port_map_entry;
    C8_T                buf_xsgmii_id[8], buf_gphy_id[8];

    if (!HAL_IS_UNIT_VALID(unit))
    {
        osal_printf("***Error***, unit %d is invalid.\n", unit);
        return (AIR_E_BAD_PARAMETER);
    }

    /* display total AIR port bitmap information */
    osal_printf("air port bitmap total  : ");
    for (word_cnt = 0; word_cnt < AIR_PORT_BITMAP_SIZE; word_cnt++)
    {
        osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_total[(AIR_PORT_BITMAP_SIZE - (1 + word_cnt))]);
    }
    osal_printf("\n");

    /* display gphy AIR port bitmap information */
    osal_printf("air port bitmap gphy   : ");
    for (word_cnt = 0; word_cnt < AIR_PORT_BITMAP_SIZE; word_cnt++)
    {
        osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_gphy[(AIR_PORT_BITMAP_SIZE - (1 + word_cnt))]);
    }
    osal_printf("\n");

    /* display xsgmii AIR port bitmap information */
    osal_printf("air port bitmap xsgmii : ");
    for (word_cnt = 0; word_cnt < AIR_PORT_BITMAP_SIZE; word_cnt++)
    {
        osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_xsgmii[(AIR_PORT_BITMAP_SIZE - (1 + word_cnt))]);
    }
    osal_printf("\n");

    /* display combo AIR port bitmap information */
    osal_printf("air port bitmap combo  : ");
    for (word_cnt = 0; word_cnt < AIR_PORT_BITMAP_SIZE; word_cnt++)
    {
        osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_combo[(AIR_PORT_BITMAP_SIZE - (1 + word_cnt))]);
    }
    osal_printf("\n");

    /* display cascade AIR port bitmap information */
    osal_printf("air port bitmap cascade: ");
    for (word_cnt = 0; word_cnt < AIR_PORT_BITMAP_SIZE; word_cnt++)
    {
        osal_printf("%08x ", PTR_HAL_EXT_CHIP_INFO(unit)->port_bitmap_cascade[(AIR_PORT_BITMAP_SIZE - (1 + word_cnt))]);
    }
    osal_printf("\n");

    ptr_port_map_entry = &(PTR_HAL_EXT_CHIP_INFO(unit)->ptr_sdk_port_map_info[0]);

    osal_printf("air port map info      :\n");
    osal_printf("%10s %7s %12s %12s %10s %10s\n", "unit/port", "type", "max-speed", "xsgmii-id", "gphy-id", "mac-pid");

    for (port = 0; port < AIR_PORT_NUM; port++)
    {
        if (ptr_port_map_entry->valid)
        {
            if (ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_XSGMII)
            {
                /* xsgmii port */
                osal_snprintf(buf_xsgmii_id, sizeof(buf_xsgmii_id), "%d",
                              ptr_port_map_entry->xsgmii_port.xsgmii_pkg_id);
                osal_snprintf(buf_gphy_id, sizeof(buf_gphy_id), "%d", ptr_port_map_entry->xsgmii_port.phy_pkg_id);
            }
            else if (ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_BASET)
            {
                /* gphy port */
                osal_snprintf(buf_xsgmii_id, sizeof(buf_xsgmii_id), "%s", "---");
                osal_snprintf(buf_gphy_id, sizeof(buf_gphy_id), "%d", ptr_port_map_entry->baset_port.phy_pkg_id);
            }
            else if (ptr_port_map_entry->port_type == AIR_INIT_PORT_TYPE_ENHANCED_BASET)
            {
                /* gphy enhanced port */
                osal_snprintf(buf_xsgmii_id, sizeof(buf_xsgmii_id), "%s", "---");
                osal_snprintf(buf_gphy_id, sizeof(buf_gphy_id), "%d",
                              ptr_port_map_entry->enhanced_baset_port.phy_pkg_id);
            }
            else
            {
                /* cpu port */
                osal_snprintf(buf_xsgmii_id, sizeof(buf_xsgmii_id), "%s", "---");
                osal_snprintf(buf_gphy_id, sizeof(buf_gphy_id), "%s", "---");
            }

            osal_printf("%4d/%2d %10s %10d %11s %11s %9d\n", unit, port,
                        ptr_port_type_str[ptr_port_map_entry->port_type], ptr_port_map_entry->max_speed, buf_xsgmii_id,
                        buf_gphy_id, ptr_port_map_entry->mac_port);
        }
        ptr_port_map_entry++;
    }

    return (AIR_E_OK);
}

static AIR_ERROR_NO_T
_diag_cmd_showInfo(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0;

    /* cmd: diag show info [ unit=<UINT> ] { sdk | chip | port } */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);

    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "sdk"))
    {
        token_idx += 1;

        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

        rc = _diag_cmd_showInfoSdk(unit);
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "chip"))
    {
        token_idx += 1;

        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

        rc = _diag_cmd_showInfoChip(unit);
    }
    else if (AIR_E_OK == dsh_checkString(tokens[token_idx], "port"))
    {
        token_idx += 1;
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

        rc = _diag_cmd_showInfoPort(unit);
    }
    else
    {
        return (DSH_E_SYNTAX_ERR);
    }

    return (rc);
}

static AIR_ERROR_NO_T
_diag_cmd_setAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, addr = 0, data = 0;

    /* cmd: diag set addr [ unit=<UINT> ] addr=<HEX> data=<HEX> */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "addr", &addr, sizeof(UI32_T)), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "data", &data, sizeof(UI32_T)), token_idx, 2);

    rc = aml_writeReg(unit, addr, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, diag set addr error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_diag_cmd_showAddr(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, addr = 0, data = 0;

    /* cmd: diag show addr [ unit=<UINT> ] addr=<HEX> */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "addr", &addr, sizeof(UI32_T)), token_idx, 2);

    rc = aml_readReg(unit, addr, &data, sizeof(UI32_T));
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, diag show addr error\n");
    }
    else
    {
        osal_printf("addr=0x%08x\t\tdata=0x%08x\n", addr, data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_diag_cmd_testTxCompliance(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T               rc = AIR_E_OK;
    UI32_T                       unit = 0;
    UI32_T                       port;
    HAL_PHY_TX_COMPLIANCE_MODE_T mode = HAL_PHY_TX_COMPLIANCE_MODE_LAST;
    C8_T                         st[DSH_CMD_MAX_LENGTH] = {0};
    C8_T                         mode_str[DSH_CMD_MAX_LENGTH] = {0};
    AIR_IFMON_MODE_T             if_mode = AIR_IFMON_MODE_POLL;
    AIR_PORT_BITMAP_T            portlist;
    UI32_T                       interval = 0;

    osal_strncpy(mode_str, "tradition", sizeof("tradition"));

    /*
     * diag test tx-compliance [ unit=<UINT> ] port=<UINT>
     *                    type={ 2500m-tm1 | 2500m-tm2 | 2500m-tm3 |
     *                           2500m-tm4 tone={ 1 | 2 | 3 | 4 | 5 } |
     *                           2500m-tm5 | 2500m-tm6 |
     *                           1000m-tm1 | 1000m-tm2 | 1000m-tm3 |
     *                           1000m-tm4 pair={ a | b | c | d | all } |
     *                           100m pair={ a | b } [ mode={ tradition | discrete } ] |
     *                           10m-sine pair={ a | b } |
     *                           10m-random pair={ a | b } |
     *                           10m-nlp pair={ a | b } }
     */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "port", &port), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", st), token_idx, 2);

    if (AIR_E_OK == dsh_checkString(st, "2500m-tm1"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM1;
    }
    else if (AIR_E_OK == dsh_checkString(st, "2500m-tm2"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM2;
    }
    else if (AIR_E_OK == dsh_checkString(st, "2500m-tm3"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM3;
    }
    else if (AIR_E_OK == dsh_checkString(st, "2500m-tm4"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "tone", st), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "1"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_1;
        }
        else if (AIR_E_OK == dsh_checkString(st, "2"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_2;
        }
        else if (AIR_E_OK == dsh_checkString(st, "3"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_3;
        }
        else if (AIR_E_OK == dsh_checkString(st, "4"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_4;
        }
        else if (AIR_E_OK == dsh_checkString(st, "5"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM4_TONE_5;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_E_OK == dsh_checkString(st, "2500m-tm5"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM5;
    }
    else if (AIR_E_OK == dsh_checkString(st, "2500m-tm6"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_2500M_TM6;
    }
    else if (AIR_E_OK == dsh_checkString(st, "1000m-tm1"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM1;
    }
    else if (AIR_E_OK == dsh_checkString(st, "1000m-tm2"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM2;
    }
    else if (AIR_E_OK == dsh_checkString(st, "1000m-tm3"))
    {
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM3;
    }
    else if (AIR_E_OK == dsh_checkString(st, "1000m-tm4"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", st), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "a"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_A;
        }
        else if (AIR_E_OK == dsh_checkString(st, "b"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_B;
        }
        else if (AIR_E_OK == dsh_checkString(st, "c"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_C;
        }
        else if (AIR_E_OK == dsh_checkString(st, "d"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4_PAIR_D;
        }
        else if (AIR_E_OK == dsh_checkString(st, "all"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_1000M_TM4;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_E_OK == dsh_checkString(st, "100m"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", st), token_idx, 2);
        DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "mode", mode_str), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "a"))
        {
            if (AIR_E_OK == dsh_checkString(mode_str, "tradition"))
            {
                mode = HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A;
            }
            else if (AIR_E_OK == dsh_checkString(mode_str, "discrete"))
            {
                mode = HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_A_DISCRETE;
            }
            else
            {
                return AIR_E_BAD_PARAMETER;
            }
        }
        else if (AIR_E_OK == dsh_checkString(st, "b"))
        {
            if (AIR_E_OK == dsh_checkString(mode_str, "tradition"))
            {
                mode = HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B;
            }
            else if (AIR_E_OK == dsh_checkString(mode_str, "discrete"))
            {
                mode = HAL_PHY_TX_COMPLIANCE_MODE_100M_PAIR_B_DISCRETE;
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
    else if (AIR_E_OK == dsh_checkString(st, "10m-sine"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", st), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "a"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_A;
        }
        else if (AIR_E_OK == dsh_checkString(st, "b"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_SINE_PAIR_B;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_E_OK == dsh_checkString(st, "10m-random"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", st), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "a"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_A;
        }
        else if (AIR_E_OK == dsh_checkString(st, "b"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_RANDOM_PAIR_B;
        }
        else
        {
            return AIR_E_BAD_PARAMETER;
        }
    }
    else if (AIR_E_OK == dsh_checkString(st, "10m-nlp"))
    {
        DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pair", st), token_idx, 2);
        DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
        if (AIR_E_OK == dsh_checkString(st, "a"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_A;
        }
        else if (AIR_E_OK == dsh_checkString(st, "b"))
        {
            mode = HAL_PHY_TX_COMPLIANCE_MODE_10M_NLP_PAIR_B;
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

    rc = air_ifmon_getMode(unit, &if_mode, &portlist, &interval);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, ifmon get mode fail.\n");
        return rc;
    }

    AIR_PORT_DEL(portlist, port);
    rc = air_ifmon_setMode(unit, if_mode, portlist, interval);
    if (rc != AIR_E_OK)
    {
        osal_printf("***Error***, ifmon set mode fail.\n");
    }

    rc = hal_phy_testTxCompliance(unit, port, mode);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, port(%u) tx-compliance set-mode(%s) fail(%d)\n", port, st, rc);
    }

    return rc;
}

static AIR_ERROR_NO_T
_diag_cmd_analyzeFreePage(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0;
    UI32_T            fp_cnt = 0, min_fp_cnt = 0;
    UI32_T            queue = 0, mode = 0;
    UI32_T            cur_fp_cnt = 0;
    AIR_PORT_BITMAP_T pbm = {0};

    /* cmd: diag analyze free-page [ unit=<UINT> ] [ portlist=<UINTLIST> ] */

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_OPT(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    osal_printf(" %-32s: %u\n", "unit", unit);

    /* Global Free Page information */
    rc = HAL_FUNC_CALL(unit, swc, getGlobalFreePages, (unit, &fp_cnt, &min_fp_cnt));
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get global free pages fail(%d)\n", rc);
        return rc;
    }
    cur_fp_cnt += fp_cnt;
    osal_printf(" %-32s: %u\n", "free page link counter", fp_cnt);
    osal_printf(" %-32s: %u\n", "minimal free page link counter", min_fp_cnt);
    osal_printf("\n");

    /* Port Free Page information */
    AIR_PORT_FOREACH(HAL_PORT_BMP(unit), port)
    {
        if (AIR_PORT_CHK(pbm, port))
        {
            osal_printf(" - port = %u\n", port);
        }
        rc = HAL_FUNC_CALL(unit, swc, getPortAllocatedPages, (unit, port, &fp_cnt));
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get allocated pages port=%d fail(%d)\n", port, rc);
            return rc;
        }
        cur_fp_cnt += fp_cnt;
        if (AIR_PORT_CHK(pbm, port))
        {
            osal_printf(" - %-30s: %u\n", "free page in rx-ctrl", fp_cnt);
        }

        for (mode = 0; mode < 2; mode++)
        {
            switch (mode)
            {
                case 0:
                    if (AIR_PORT_CHK(pbm, port))
                    {
                        osal_printf(" - %-30s\n\t", "used pages in queue");
                    }
                    break;
                case 1:
                    if (AIR_PORT_CHK(pbm, port))
                    {
                        osal_printf(" - %-30s\n\t", "packets in queue");
                    }
                    break;
                default:
                    osal_printf("***Error***, not support mode=%d \n", mode);
                    return AIR_E_NOT_SUPPORT;
            }
            for (queue = 0; queue < AIR_QOS_QUEUE_MAX_NUM; queue++)
            {
                rc = HAL_FUNC_CALL(unit, swc, getPortUsedPages, (unit, port, queue, mode, &fp_cnt));
                if (AIR_E_OK != rc)
                {
                    osal_printf("***Error***, get used pages port=%d queue=%d mode=%d fail(%d)\n", port, queue, mode,
                                rc);
                    return rc;
                }
                if (AIR_PORT_CHK(pbm, port))
                {
                    osal_printf("q%u=%-4u ", queue, fp_cnt);
                }
                if (0 == mode)
                {
                    cur_fp_cnt += fp_cnt;
                }
            }
            if (AIR_PORT_CHK(pbm, port))
            {
                osal_printf("\n");
            }
        }
        if (AIR_PORT_CHK(pbm, port))
        {
            osal_printf("\n");
        }
    }

    /* CPU port Free Page information */
    port = 0;
    rc = HAL_FUNC_CALL(unit, swc, getPortAllocatedPages, (unit, port, &fp_cnt));
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, get allocated pages port=%d fail(%d)\n", port, rc);
        return rc;
    }
    cur_fp_cnt += fp_cnt;
    for (queue = 0; queue < AIR_QOS_QUEUE_MAX_NUM; queue++)
    {
        rc = HAL_FUNC_CALL(unit, swc, getPortUsedPages, (unit, port, queue, 0, &fp_cnt));
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, get used pages port=%d queue=%d mode=%d fail(%d)\n", port, queue, 0, rc);
            return rc;
        }
        cur_fp_cnt += fp_cnt;
    }
    osal_printf("\n %-32s: %u\n", "total page counter", cur_fp_cnt);

    return rc;
}

static AIR_ERROR_NO_T
_diag_cmd_analyzeCableTest(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, port = 0, output = 0;
    enum
    {
        AIR_OUTPUT_VIEW,
        AIR_OUTPUT_TXT,
        AIR_OUTPUT_DUMP_LAST
    };
    AIR_PORT_BITMAP_T pbm = {0};
    C8_T              str[DSH_CMD_MAX_LENGTH] = {0};
    UI32_T          **ptr_cable_all;
    UI32_T            i, j;
    /*
     * Command format
     * diag analyze cable-test [ unit=<UINT> ] portlist=<UINTLIST> output={ view | txt }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &pbm), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "output", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "view"))
    {
        output = AIR_OUTPUT_VIEW;
    }
    else if (AIR_E_OK == dsh_checkString(str, "txt"))
    {
        output = AIR_OUTPUT_TXT;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    ptr_cable_all = (UI32_T **)osal_alloc(sizeof(UI32_T *) * AIR_PORT_CABLE_TEST_PAIR_ALL, "cmd");
    if (NULL == ptr_cable_all)
    {
        osal_printf("***Error***, allocate memory fail\n");
        return AIR_E_NO_MEMORY;
    }
    osal_memset(ptr_cable_all, 0, sizeof(UI32_T) * AIR_PORT_CABLE_TEST_PAIR_ALL);

    for (i = 0; i < AIR_PORT_CABLE_TEST_PAIR_ALL; i++)
    {
        ptr_cable_all[i] = osal_alloc(sizeof(UI32_T) * HAL_PHY_EC_ALL_TAPS, "cmd");
        if (NULL == ptr_cable_all[i])
        {
            osal_printf("***Error***, allocate memory fail\n");
            rc = AIR_E_NO_MEMORY;
            break;
        }
        else
        {
            osal_memset(ptr_cable_all[i], 0, sizeof(UI32_T) * HAL_PHY_EC_ALL_TAPS);
        }
    }

    if (AIR_E_OK == rc)
    {
        osal_printf("unit %u", unit);
        AIR_PORT_FOREACH(pbm, port)
        {
            osal_printf(" - port = %u\n", port);
            rc = HAL_FUNC_CALL(unit, port, getcableTestRawData, (unit, port, ptr_cable_all));

            if (AIR_OUTPUT_TXT == output)
            {
                osal_printf("cable diagnostic raw data xml :\n");
            }
            else
            {
                osal_printf("pair-a  pair-b  pair-c  pair-d\n");
            }
            osal_printf("==============================\n");

            for (j = 0; j < HAL_PHY_EC_ALL_TAPS; j++)
            {
                for (i = 0; i < AIR_PORT_CABLE_TEST_PAIR_ALL; i++)
                {
                    if (AIR_OUTPUT_TXT == output)
                    {
                        osal_printf("%x,", ptr_cable_all[i][j]);
                    }
                    else
                    {
                        osal_printf("%x \t", ptr_cable_all[i][j]);
                    }
                }
                osal_printf("\n");
            }

            osal_printf("==============================\n");
            if (AIR_E_OK == rc)
            {
                osal_printf("***Success\n");
            }
            else
            {
                osal_printf("***Error***, analyze cable-test error\n");
            }
            osal_printf("\n\n");
        }
    }

    for (i = 0; i < AIR_PORT_CABLE_TEST_PAIR_ALL; i++)
    {
        if (NULL != ptr_cable_all[i])
        {
            osal_free(ptr_cable_all[i]);
        }
    }
    osal_free(ptr_cable_all);
    return rc;
}

static AIR_ERROR_NO_T
_diag_cmd_syncLedClock(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * diag sync led-clock [ unit=<UINT> ] portlist=<UINTLIST> [ delay=<UINT>(ms) ]
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0;
    UI32_T            delay = 0; /* Unit: us */
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "delay"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "delay", &delay), token_idx, 2);
        delay *= 1000;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = hal_phy_syncLedClock(unit, port_bitmap, delay);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, sync led clock error\n");
    }

    rc = hal_phy_syncWaveGenClock(unit, port_bitmap, delay);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, sync wavegen clock error\n");
        return rc;
    }
    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
static const DSH_VEC_T  _diag_cmd_vec[] =
{
    {
        "show info", 2, _diag_cmd_showInfo,
        "diag show info [ unit=<UINT> ] { sdk | chip | port }\n",
    },
    {
        "set addr", 2, _diag_cmd_setAddr,
        "diag set addr [ unit=<UINT> ] addr=<HEX> data=<HEX>\n",
    },
    {
        "show addr", 2, _diag_cmd_showAddr,
        "diag show addr [ unit=<UINT> ] addr=<HEX>\n",
    },
    {
        "test tx-compliance", 2, _diag_cmd_testTxCompliance,
        "diag test tx-compliance [ unit=<UINT> ] port=<UINT>\n"
        "type={ 2500m-tm1 | 2500m-tm2 | 2500m-tm3 |\n"
        "       2500m-tm4 tone={ 1 | 2 | 3 | 4 | 5 } |\n"
        "       2500m-tm5 | 2500m-tm6 |\n"
        "       1000m-tm1 | 1000m-tm2 | 1000m-tm3 |\n"
        "       1000m-tm4 pair={ a | b | c | d | all } |\n"
        "       100m pair={ a | b } [ mode={ tradition | discrete } ] |\n"
        "       10m-sine pair={ a | b } |\n"
        "       10m-random pair={ a | b } |\n"
        "       10m-nlp pair={ a | b } }\n"
    },
    {
        "analyze free-page", 2, _diag_cmd_analyzeFreePage,
        "diag analyze free-page [ unit=<UINT> ] [ portlist=<UINTLIST> ]\n",
    },
    {
        "analyze cable-test", 2, _diag_cmd_analyzeCableTest,
        "diag analyze cable-test [ unit=<UINT> ] portlist=<UINTLIST> output={ view | txt }\n",

    },
    {
        "sync led-clock", 2, _diag_cmd_syncLedClock,
        "diag sync led-clock [ unit=<UINT> ] portlist=<UINTLIST> [ delay=<UINT>(ms) ]\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
diag_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _diag_cmd_vec, sizeof(_diag_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
diag_cmd_usager()
{
    return (dsh_usager(_diag_cmd_vec, sizeof(_diag_cmd_vec) / sizeof(DSH_VEC_T)));
}
