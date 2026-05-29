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
#include <cmd/phy_cmd.h>

#include <air_error.h>
#include <air_port.h>
#include <air_types.h>
#include <cmlib/cmlib_bit.h>
#include <hal/common/hal_mdio.h>
#include <hal/common/hal_phy.h>
#include <osal/osal.h>
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
_phy_cmd_buckPbusWrite(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pbus_addr,
    const UI32_T pbus_data)
{
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x11, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x12, (UI32_T)(pbus_addr & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x13, (UI32_T)((pbus_data >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x14, (UI32_T)(pbus_data & 0xffff));
    return rv;
}

static AIR_ERROR_NO_T
_phy_cmd_buckPbusRead(
    const UI32_T unit,
    const UI32_T port,
    const UI32_T pbus_addr,
    UI32_T      *ptr_pbus_data)
{
    UI32_T         pbus_data = 0;
    UI16_T         pbus_data_low = 0, pbus_data_high = 0;
    AIR_ERROR_NO_T rv = AIR_E_OK;

    rv |= hal_mdio_writeC22ByPort(unit, port, 0x1F, 0x4);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x10, 0x0);
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x15, (UI32_T)((pbus_addr >> 16) & 0xffff));
    rv |= hal_mdio_writeC22ByPort(unit, port, 0x16, (UI32_T)(pbus_addr & 0xffff));

    rv |= hal_mdio_readC22ByPort(unit, port, 0x17, &pbus_data_high);
    rv |= hal_mdio_readC22ByPort(unit, port, 0x18, &pbus_data_low);

    pbus_data = (pbus_data_high << 16) + pbus_data_low;
    *ptr_pbus_data = pbus_data;
    return rv;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC22(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set c22 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, reg_addr = 0, reg_data = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC22(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)reg_addr, (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set c22 error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC22(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show c22 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, reg_addr = 0;
    UI16_T         reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc = hal_mdio_readC22(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)reg_addr, &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy show c22 error\n");
    }
    else
    {
        osal_printf("unit=%2u    mdio-bus=%2u    phy-addr=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, mdio_bus,
                    phy_addr, reg_addr, reg_data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC22ByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-c22 [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_writeC22ByPort(unit, port, (UI16_T)reg_addr, (UI16_T)reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy set port-c22 error\n");
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC22ByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-c22 [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0;
    UI16_T            reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_readC22ByPort(unit, port, (UI16_T)reg_addr, &reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy show port-c22 error\n");
            break;
        }
        else
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, reg_addr, reg_data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC45(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set c45 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>
     * dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, dev_type = 0, reg_addr = 0, reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC45(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)dev_type, (UI16_T)reg_addr,
                           (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set c45 error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC45(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show c45 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> dev-type=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, dev_type = 0, reg_addr = 0;
    UI16_T         reg_data = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc = hal_mdio_readC45(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)dev_type, (UI16_T)reg_addr, &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set c45 error\n");
    }
    else
    {
        osal_printf("unit=%2u  mdio-bus=%2u  phy-addr=%2u  dev-type=%2u  reg-addr=%2u  reg-data=0x%04x\n", unit,
                    mdio_bus, phy_addr, dev_type, reg_addr, reg_data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC45ByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-c45 [ unit=<UINT> ] portlist=<UINTLIST>
     * dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, dev_type = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_writeC45ByPort(unit, port, dev_type, (UI16_T)reg_addr, (UI16_T)reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy set port-c22 error\n");
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC45ByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-c45 [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, dev_type = 0, reg_addr = 0;
    UI16_T            reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_readC45ByPort(unit, port, (UI16_T)dev_type, (UI16_T)reg_addr, &reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy show port-c22 error\n");
            break;
        }
        else
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                        dev_type, reg_addr, reg_data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_showInfo(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show info [ unit=<UINT> ] portlist=<UINTLIST>
     */
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T port_bitmap;
    C8_T              buffer[11] = {};

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);

    HAL_CHECK_UNIT(unit);

    osal_printf("%10s %5s %9s %9s %11s %7s %9s %9s\n", "unit/port", "type", "i2c-addr", "phy-addr", "phy-id", "rev-id",
                "internal", "external");

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (!HAL_IS_ETH_PORT_VALID(unit, port))
        {
            continue;
        }

        osal_printf("%5d/%2d", unit, port);

        /* access type */
        switch (HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            case HAL_PHY_ACCESS_TYPE_MDIO:
                osal_printf("%8s", "mdio");
                break;
            case HAL_PHY_ACCESS_TYPE_I2C_MDIO:
                osal_printf("%8s", "i2c");
                break;
            default:
                osal_printf("%8s", "---");
                break;
        }

        /* i2c/phy addr */
        osal_snprintf(buffer, sizeof(buffer), "0x%x", HAL_PHY_PORT_DEV_I2C_ADDR(unit, port));
        osal_printf("%10s", buffer);
        osal_printf("%10d", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port));

        /* phy id, revision id */
        osal_snprintf(buffer, sizeof(buffer), "0x%x", HAL_PHY_PORT_DEV_PHY_ID(unit, port));
        osal_printf("%12s", buffer);
        osal_printf("%8d", HAL_PHY_PORT_DEV_REVISION_ID(unit, port));

        /* internal/external driver */
        if (NULL != PTR_HAL_PHY_PORT_INT_DRIVER(unit, port))
        {
            osal_printf("%10s", "Y");
        }
        else
        {
            osal_printf("%10s", "N");
        }

        if (NULL != PTR_HAL_PHY_PORT_EXT_DRIVER(unit, port))
        {
            osal_printf("%10s", "Y");
        }
        else
        {
            osal_printf("%10s", "N");
        }
        osal_printf("\n");
    }

    return AIR_E_OK;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC22FromIntBus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set c22-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> page=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, reg_addr = 0, reg_data = 0, page = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "page", &page), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC22FromIntBus(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)page, (UI16_T)reg_addr,
                                     (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set c22-by-int-bus error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC22FromIntBus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show c22-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> page=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, reg_addr = 0, page = 0;
    UI16_T         reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "page", &page), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc =
        hal_mdio_readC22FromIntBus(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)page, (UI16_T)reg_addr, &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy show c22-by-int-bus error\n");
    }
    else
    {
        osal_printf("unit=%2u    mdio-bus=%2u    phy-addr=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, mdio_bus,
                    phy_addr, reg_addr, reg_data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC22FromIntBusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-c22-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> page=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0, reg_data = 0, page = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "page", &page), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_writeC22FromIntBusByPort(unit, port, (UI16_T)page, (UI16_T)reg_addr, (UI16_T)reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy set port-c22-by-int-bus error\n");
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC22FromIntBusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-c22-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> page=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0, page = 0;
    UI16_T            reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "page", &page), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_readC22FromIntBusByPort(unit, port, (UI16_T)page, (UI16_T)reg_addr, &reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy show port-c22-by-int-bus error\n");
            break;
        }
        else
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port, reg_addr, reg_data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC45FromIntBus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set c45-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>
     * dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, dev_type = 0, reg_addr = 0, reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC45FromIntBus(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)dev_type, (UI16_T)reg_addr,
                                     (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set-from-int-bus c45 error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC45FromIntBus(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show c45-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> dev-type=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, mdio_bus = 0, phy_addr = 0, dev_type = 0, reg_addr = 0;
    UI16_T         reg_data = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    if (AIR_E_OK == dsh_checkString(tokens[token_idx], "mdio-bus"))
    {
        DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "mdio-bus", &mdio_bus), token_idx, 2);
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc = hal_mdio_readC45FromIntBus(unit, (UI16_T)mdio_bus, (UI16_T)phy_addr, (UI16_T)dev_type, (UI16_T)reg_addr,
                                    &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set c45-by-int-bus error\n");
    }
    else
    {
        osal_printf("unit=%2u  mdio-bus=%2u  phy-addr=%2u  dev-type=%2u  reg-addr=%2u  reg-data=0x%04x\n", unit,
                    mdio_bus, phy_addr, dev_type, reg_addr, reg_data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC45FromIntBusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-c45-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST>
     * dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, dev_type = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_writeC45FromIntBusByPort(unit, port, dev_type, (UI16_T)reg_addr, (UI16_T)reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy set port-c45-by-int-bus error\n");
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC45FromIntBusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-c45-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, dev_type = 0, reg_addr = 0;
    UI16_T            reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }
        rc = hal_mdio_readC45FromIntBusByPort(unit, port, (UI16_T)dev_type, (UI16_T)reg_addr, &reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy show port-c45-by-int-bus error\n");
            break;
        }
        else
        {
            osal_printf("unit=%2u    port=%2u    dev-type=%2u    reg-addr=%2u    reg-data=0x%04x\n", unit, port,
                        dev_type, reg_addr, reg_data);
        }
    }

    return rc;
}
#ifdef AIR_EN_I2C_PHY
static AIR_ERROR_NO_T
_phy_cmd_mdioWriteRegByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-i2c-reg [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }

        if (HAL_PHY_ACCESS_TYPE_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            osal_printf("***Error***, port %u access type is not i2c\n", port);
            rc = AIR_E_BAD_PARAMETER;
            break;
        }

        rc = hal_mdio_writeRegByI2c(unit, port, reg_addr, reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy set port-i2c-reg error\n");
            break;
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadRegByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-i2c-reg [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T    rc = AIR_E_OK;
    UI32_T            unit = 0, port = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        if (port == HAL_CPU_PORT(unit))
        {
            continue;
        }

        if (HAL_PHY_ACCESS_TYPE_MDIO == HAL_PHY_PORT_DEV_ACCESS_TYPE(unit, port))
        {
            osal_printf("***Error***, port %u access type is not i2c\n", port);
            rc = AIR_E_BAD_PARAMETER;
            break;
        }

        rc = hal_mdio_readRegByI2c(unit, port, reg_addr, &reg_data);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy show port-i2c-reg\n");
            break;
        }
        else
        {
            osal_printf("unit=%2u    port=%2u    reg-addr=0x%08x    reg-data=0x%08x\n", unit, port, reg_addr, reg_data);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC22ByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set i2c-c22 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, i2c_bus = 0, i2c_addr = 0, phy_addr = 0, reg_addr = 0, reg_data = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-bus", &i2c_bus), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-addr", &i2c_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC22ByI2c(unit, (UI16_T)i2c_bus, (UI16_T)i2c_addr, (UI16_T)phy_addr, (UI16_T)reg_addr,
                                (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set i2c-c22 error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC22ByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show i2c-c22 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, i2c_bus = 0, i2c_addr = 0, phy_addr = 0, reg_addr = 0;
    UI16_T         reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-bus", &i2c_bus), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-addr", &i2c_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc = hal_mdio_readC22ByI2c(unit, (UI16_T)i2c_bus, (UI16_T)i2c_addr, (UI16_T)phy_addr, (UI16_T)reg_addr, &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy show i2c-c22 error\n");
    }
    else
    {
        osal_printf("unit=%2u    i2c-bus=%2u    i2c_addr=0x%02X    phy-addr=%2u    reg-addr=%2u    reg-data=0x%04x\n",
                    unit, i2c_bus, i2c_addr, phy_addr, reg_addr, reg_data);
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioWriteC45ByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set i2c-c45 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT>
     * phy-addr=<UINT> dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, i2c_bus = 0, i2c_addr = 0, phy_addr = 0, dev_type = 0, reg_addr = 0, reg_data = 0;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-bus", &i2c_bus), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-addr", &i2c_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getHex(tokens, token_idx, "reg-data", &reg_data, sizeof(UI32_T)), token_idx, 2);

    rc = hal_mdio_writeC45ByI2c(unit, (UI16_T)i2c_bus, (UI16_T)i2c_addr, (UI16_T)phy_addr, (UI16_T)dev_type,
                                (UI16_T)reg_addr, (UI16_T)reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy set i2c-c45 error\n");
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_mdioReadC45ByI2c(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show i2c-c45 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT>
     * phy-addr=<UINT> dev-type=<UINT> reg-addr=<UINT>
     */
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, i2c_bus = 0, i2c_addr = 0, phy_addr = 0, dev_type = 0, reg_addr = 0;
    UI16_T         reg_data = 0;
    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-bus", &i2c_bus), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "i2c-addr", &i2c_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "phy-addr", &phy_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "dev-type", &dev_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);

    rc = hal_mdio_readC45ByI2c(unit, (UI16_T)i2c_bus, (UI16_T)i2c_addr, (UI16_T)phy_addr, (UI16_T)dev_type,
                               (UI16_T)reg_addr, &reg_data);
    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, phy show i2c-c45\n");
    }
    else
    {
        osal_printf(
            "unit=%2u    i2c-bus=%2u    i2c_addr=0x%02X    phy-addr=%2u    dev-type=%2u    reg-addr=%2u    "
            "reg-data=0x%04x\n",
            unit, i2c_bus, i2c_addr, phy_addr, dev_type, reg_addr, reg_data);
    }

    return rc;
}
#endif
static AIR_ERROR_NO_T
_phy_cmd_dumpPara(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy dump parameter [ unit=<UINT> ] portlist=<UINTLIST>
     */
    UI32_T            unit = 0, port = 0;
    AIR_PORT_BITMAP_T port_bitmap;
    AIR_ERROR_NO_T    rc = AIR_E_OK;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        rc = hal_phy_dumpPhyPara(unit, port);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, phy dump parameter\n");
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_setBuckPbusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy set port-buck-pbus [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT> reg-data=<HEX>
     */
    UI32_T            unit = 0, port = 0, reg_addr = 0, reg_data = 0;
    AIR_PORT_BITMAP_T port_bitmap;
    AIR_ERROR_NO_T    rc = AIR_E_OK;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-data", &reg_data), token_idx, 2);

    rc = air_ifmon_setMonitorState(unit, FALSE);
    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(port_bitmap, port)
        {
            rc = _phy_cmd_buckPbusWrite(unit, port, reg_addr, reg_data);
            if (AIR_E_OK != rc)
            {
                osal_printf("***Error***, phy set port-buck-pbus\n");
            }
        }
    }
    rc |= air_ifmon_setMonitorState(unit, TRUE);
    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_showBuckPbusByPort(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy show port-buck-pbus [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT> num=<UINT>
     */
    UI32_T            unit = 0, port = 0, reg_addr = 0, num = 0, reg_data = 0, index = 0;
    AIR_PORT_BITMAP_T port_bitmap;
    AIR_ERROR_NO_T    rc = AIR_E_OK;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "reg-addr", &reg_addr), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "num", &num), token_idx, 2);

    if ((num > 0x1000) || (num == 0))
    {
        osal_printf("***Error***, num should be 0x1 ~ 0x1000.\n");
        return AIR_E_BAD_PARAMETER;
    }

    rc = air_ifmon_setMonitorState(unit, FALSE);
    if (AIR_E_OK == rc)
    {
        AIR_PORT_FOREACH(port_bitmap, port)
        {
            for (index = 0; index < num; index++)
            {
                rc = _phy_cmd_buckPbusRead(unit, port, (reg_addr + (index * 4)), &reg_data);
                if (AIR_E_OK == rc)
                {
                    osal_printf("phy=%d, reg=0x%x, val=0x%x\n", HAL_PHY_PORT_DEV_PHY_ADDR(unit, port),
                                (reg_addr + (index * 4)), reg_data);
                }
                else
                {
                    osal_printf("***Error***, phy show port-buck-pbus\n");
                }
            }
        }
    }
    rc |= air_ifmon_setMonitorState(unit, TRUE);
    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_dumpPortCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy dump counter [ unit=<UINT> ] portlist=<UINTLIST> type={ ext | int }
     * interface={ mii | xgmii | usxgmii }
     */
    UI32_T             unit = 0, port = 0, param = 0;
    AIR_PORT_BITMAP_T  port_bitmap;
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    C8_T               str_intf[DSH_CMD_MAX_LENGTH] = {0}, str_type[DSH_CMD_MAX_LENGTH] = {0};
    HAL_PHY_PHY_TYPE_T phy_type = HAL_PHY_PHY_TYPE_LAST;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str_type), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "interface", str_intf), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* type */
    if (AIR_E_OK == dsh_checkString(str_type, "ext"))
    {
        phy_type = HAL_PHY_PHY_TYPE_EXTERNAL;
    }
    else if (AIR_E_OK == dsh_checkString(str_type, "int"))
    {
        phy_type = HAL_PHY_PHY_TYPE_INTERNAL;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    /* interface */
    if (AIR_E_OK == dsh_checkString(str_intf, "mii"))
    {
        param = HAL_PHY_SERDES_INTERFACE_MII;
    }
    else if (AIR_E_OK == dsh_checkString(str_intf, "xgmii"))
    {
        param = HAL_PHY_SERDES_INTERFACE_XGMII;
    }
    else if (AIR_E_OK == dsh_checkString(str_intf, "usxgmii"))
    {
        param = HAL_PHY_SERDES_INTERFACE_USXGMII;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        rc = hal_phy_dumpPortCnt(unit, port, phy_type, param);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, serdes dump cnt fail rc = %d\n", rc);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_phy_cmd_clearPortCnt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    /* Command format
     * phy clear counter [ unit=<UINT> ] portlist=<UINTLIST> type={ ext | int }
     */
    UI32_T             unit = 0, port = 0, param = 0;
    AIR_PORT_BITMAP_T  port_bitmap;
    AIR_ERROR_NO_T     rc = AIR_E_OK;
    C8_T               str_type[DSH_CMD_MAX_LENGTH] = {0};

    HAL_PHY_PHY_TYPE_T phy_type = HAL_PHY_PHY_TYPE_LAST;

    /* parse and get */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getPortBitmap(tokens, token_idx, "portlist", unit, &port_bitmap), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "type", str_type), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    /* type */
    if (AIR_E_OK == dsh_checkString(str_type, "ext"))
    {
        phy_type = HAL_PHY_PHY_TYPE_EXTERNAL;
    }
    else if (AIR_E_OK == dsh_checkString(str_type, "int"))
    {
        phy_type = HAL_PHY_PHY_TYPE_INTERNAL;
    }
    else
    {
        return AIR_E_BAD_PARAMETER;
    }

    param |= HAL_PHY_SERDES_READ_CLEAR;

    AIR_PORT_FOREACH(port_bitmap, port)
    {
        rc = hal_phy_dumpPortCnt(unit, port, phy_type, param);
        if (AIR_E_OK != rc)
        {
            osal_printf("***Error***, serdes clear cnt fail rc = %d\n", rc);
        }
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _phy_cmd_vec[] =
{
    {
        "set c22", 2, _phy_cmd_mdioWriteC22,
        "phy set c22 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> reg-addr=<UINT>\n"
        "reg-data=<HEX>\n"
    },
    {
        "show c22", 2, _phy_cmd_mdioReadC22,
        "phy show c22 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> reg-addr=<UINT>\n"
    },
    {
        "set port-c22", 2, _phy_cmd_mdioWriteC22ByPort,
        "phy set port-c22 [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
        "reg-data=<HEX>\n"
    },
    {
        "show port-c22", 2, _phy_cmd_mdioReadC22ByPort,
        "phy show port-c22 [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
    },
    {
        "set c45", 2, _phy_cmd_mdioWriteC45,
        "phy set c45 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> dev-type=<UINT>\n"
        "reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show c45", 2, _phy_cmd_mdioReadC45,
        "phy show c45 [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT> dev-type=<UINT>\n"
        "reg-addr=<UINT>\n"
    },
    {
        "set port-c45", 2, _phy_cmd_mdioWriteC45ByPort,
        "phy set port-c45 [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT>\n"
        "reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show port-c45", 2, _phy_cmd_mdioReadC45ByPort,
        "phy show port-c45 [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT>\n"
        "reg-addr=<UINT>\n"
    },
    {
        "show info", 2, _phy_cmd_showInfo,
        "phy show info [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set c22-by-int-bus", 2, _phy_cmd_mdioWriteC22FromIntBus,
        "phy set c22-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>\n"
        "page=<UINT> reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show c22-by-int-bus", 2, _phy_cmd_mdioReadC22FromIntBus,
        "phy show c22-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>\n"
        "page=<UINT> reg-addr=<UINT>\n"
    },
    {
        "set port-c22-by-int-bus", 2, _phy_cmd_mdioWriteC22FromIntBusByPort,
        "phy set port-c22-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> page=<UINT>\n"
        "reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show port-c22-by-int-bus", 2, _phy_cmd_mdioReadC22FromIntBusByPort,
        "phy show port-c22-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> page=<UINT>\n"
        "reg-addr=<UINT>\n"
    },
    {
        "set c45-by-int-bus", 2, _phy_cmd_mdioWriteC45FromIntBus,
        "phy set c45-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>\n"
        "dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show c45-by-int-bus", 2, _phy_cmd_mdioReadC45FromIntBus,
        "phy show c45-by-int-bus [ unit=<UINT> ] [ mdio-bus=<UINT> ] phy-addr=<UINT>\n"
        "dev-type=<UINT> reg-addr=<UINT>\n"
    },
    {
        "set port-c45-by-int-bus", 2, _phy_cmd_mdioWriteC45FromIntBusByPort,
        "phy set port-c45-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT>\n"
        "reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show port-c45-by-int-bus", 2, _phy_cmd_mdioReadC45FromIntBusByPort,
        "phy show port-c45-by-int-bus [ unit=<UINT> ] portlist=<UINTLIST> dev-type=<UINT>\n"
        "reg-addr=<UINT>\n"
    },
#ifdef AIR_EN_I2C_PHY
    {
        "set port-i2c-reg", 2, _phy_cmd_mdioWriteRegByI2c,
        "phy set port-i2c-reg [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
        "reg-data=<HEX>\n"
    },
    {
        "show port-i2c-reg", 2, _phy_cmd_mdioReadRegByI2c,
        "phy show port-i2c-reg [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
    },
    {
        "set i2c-c22", 2, _phy_cmd_mdioWriteC22ByI2c,
        "phy set i2c-c22 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT>\n"
        "reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show i2c-c22", 2, _phy_cmd_mdioReadC22ByI2c,
        "phy show i2c-c22 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT>\n"
        "reg-addr=<UINT>\n"
    },
    {
        "set i2c-c45", 2, _phy_cmd_mdioWriteC45ByI2c,
        "phy set i2c-c45 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT>\n"
        "dev-type=<UINT> reg-addr=<UINT> reg-data=<HEX>\n"
    },
    {
        "show i2c-c45", 2, _phy_cmd_mdioReadC45ByI2c,
        "phy show i2c-c45 [ unit=<UINT> ] i2c-bus=<UINT> i2c-addr=<UINT> phy-addr=<UINT>\n"
        "dev-type=<UINT> reg-addr=<UINT>\n"
    },
#endif
    {
        "dump parameter", 2, _phy_cmd_dumpPara,
        "phy dump parameter [ unit=<UINT> ] portlist=<UINTLIST>\n"
    },
    {
        "set port-buck-pbus", 2, _phy_cmd_setBuckPbusByPort,
        "phy set port-buck-pbus [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
        "reg-data=<HEX>\n"
    },
    {
        "show port-buck-pbus", 2, _phy_cmd_showBuckPbusByPort,
        "phy show port-buck-pbus [ unit=<UINT> ] portlist=<UINTLIST> reg-addr=<UINT>\n"
        "num=<UINT>\n"
    },
    {
        "dump counter", 2, _phy_cmd_dumpPortCnt,
        "phy dump counter [ unit=<UINT> ] portlist=<UINTLIST> type={ ext | int }\n"
        "interface={ mii | xgmii | usxgmii }\n"
    },
    {
        "clear counter", 2, _phy_cmd_clearPortCnt,
        "phy clear counter [ unit=<UINT> ] portlist=<UINTLIST> type={ ext | int }\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
phy_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    int rv = 0;
    rv = (dsh_dispatcher(tokens, token_idx, _phy_cmd_vec, sizeof(_phy_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}

AIR_ERROR_NO_T
phy_cmd_usager()
{
    int rv = 0;
    rv = (dsh_usager(_phy_cmd_vec, sizeof(_phy_cmd_vec) / sizeof(DSH_VEC_T)));
    return rv;
}
