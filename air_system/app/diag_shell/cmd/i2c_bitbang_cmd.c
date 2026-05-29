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
#ifdef AIR_EN_I2C_BITBANG
#include <cmd/i2c_bitbang_cmd.h>

#include <air_error.h>
#include <air_types.h>
#include <i2c_bitbang.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

#define I2C_BITBANG_CMD_COMPARE_STR_LENGTH(__size__, __len__, __txt__)         \
    do                                                                         \
    {                                                                          \
        if ((__size__) != (__len__))                                           \
        {                                                                      \
            osal_printf("***Error***, %s length dose not match !\n", __txt__); \
            return (AIR_E_BAD_PARAMETER);                                      \
        }                                                                      \
    } while (0)

#define I2C_BITBANG_CMD_CHECK_ADDR_LENGTH(__size__, __max__)                             \
    do                                                                                   \
    {                                                                                    \
        if ((__size__) > (__max__))                                                      \
        {                                                                                \
            osal_printf("***Error***, maximum address length is %d bytes !\n", __max__); \
            return (AIR_E_BAD_PARAMETER);                                                \
        }                                                                                \
    } while (0)

#define I2C_BITBANG_CMD_CHECK_DATA_LENGTH(__size__, __max__)                          \
    do                                                                                \
    {                                                                                 \
        if ((__size__) > (__max__))                                                   \
        {                                                                             \
            osal_printf("***Error***, maximum data length is %d bytes !\n", __max__); \
            return (AIR_E_BAD_PARAMETER);                                             \
        }                                                                             \
    } while (0)

#define I2C_BITBANG_CMD_CHECK_DATA_STR_LENGTH(__size__, __max__)                          \
    do                                                                                    \
    {                                                                                     \
        if ((__size__) > (__max__))                                                       \
        {                                                                                 \
            osal_printf("***Error***, the length must not greater then string data !\n"); \
            return (AIR_E_BAD_PARAMETER);                                                 \
        }                                                                                 \
    } while (0)

#define I2C_BITBANG_BYTE_DUMP_BOUNDARY (16)

static void
_i2c_bitbang_byte_dump(
    UI32_T  unit,
    UI32_T *ptr_array,
    UI32_T  byte_cnt)
{
    UI32_T i = 0, array_index, byte_index, boundary;
    UI8_T  byte;

    osal_printf("           ");
    for (i = 0; i < I2C_BITBANG_BYTE_DUMP_BOUNDARY; i++)
    {
        if (i == 8)
        {
            osal_printf("   ");
        }
        osal_printf("%02x ", i);
    }
    osal_printf("\n");

    osal_printf("===========");
    for (i = 0; i < I2C_BITBANG_BYTE_DUMP_BOUNDARY; i++)
    {
        if (i == 8)
        {
            osal_printf("===");
        }
        osal_printf("===");
    }
    osal_printf("\n");

    boundary = I2C_BITBANG_BYTE_DUMP_BOUNDARY *
               ((byte_cnt + I2C_BITBANG_BYTE_DUMP_BOUNDARY - 1) / I2C_BITBANG_BYTE_DUMP_BOUNDARY);
    for (i = 0; i < boundary; i++)
    {
        if ((i % 16) == 0)
        {
            osal_printf("data[0x%02x] ", i);
        }

        if (i % 16 == 8)
        {
            osal_printf("   ");
        }

        if (byte_cnt > i)
        {
            array_index = i / sizeof(UI32_T);
            byte_index = i % sizeof(UI32_T);
            byte = (ptr_array[array_index] >> (byte_index * 8)) & 0xff;
            osal_printf("%02x ", byte);
        }
        else
        {
            osal_printf("-- ");
        }

        if ((i + 1) % 16 == 0)
        {
            osal_printf("\n");
        }
    }
}

static AIR_ERROR_NO_T
_i2c_bitbang_cmd_read(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    I2C_BITBANG_PARAM_T i2c_bitbang_param;

    UI32_T              unit = 0;
    UI32_T              ch, sid, idx, dsh_len;
    UI32_T              addr_len = 0, rxlen = 0, word_addr = 0;
    UI8_T               str[DSH_CMD_MAX_LENGTH] = {0};
    UI8_T               addr[I2C_BITBANG_MAX_ADDR_LEN] = {0};
    UI8_T               rx_data[I2C_BITBANG_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "bus", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "slave-id", &sid), token_idx, 2);

    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "addr-len", &addr_len), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "addr", (C8_T *)str), token_idx, 2);

    I2C_BITBANG_CMD_CHECK_ADDR_LENGTH(addr_len, I2C_BITBANG_MAX_ADDR_LEN);

    if (str[0] != 0)
    {
        if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, addr, &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "data-len", &rxlen), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    I2C_BITBANG_CMD_CHECK_DATA_LENGTH(rxlen, I2C_BITBANG_MAX_DATA_LEN);

    for (idx = 0; idx < addr_len; idx++)
    {
        word_addr |= (addr[idx] << (idx * 8));
    }

    i2c_bitbang_param.addr_len = addr_len;
    i2c_bitbang_param.addr = word_addr;
    i2c_bitbang_param.data_len = rxlen;
    if (4 < i2c_bitbang_param.data_len)
    {
        i2c_bitbang_param.info.ptr_data = (UI32_T *)rx_data;
    }
    else
    {
        i2c_bitbang_param.info.data = (UI32_T)*rx_data;
    }

    /* i2c_bitbang read */
    rc = i2c_bitbang_read(unit, ch, sid, &i2c_bitbang_param);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show i2c_bitbang fail !\n");
        return (rc);
    }
    if (4 < i2c_bitbang_param.data_len)
    {
        _i2c_bitbang_byte_dump(unit, i2c_bitbang_param.info.ptr_data, i2c_bitbang_param.data_len);
    }
    else
    {
        osal_printf("data = 0x%x ", (i2c_bitbang_param.info.data));
    }

    osal_printf("\n");
    return (rc);
}

static AIR_ERROR_NO_T
_i2c_bitbang_cmd_write(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T      rc = AIR_E_OK;
    I2C_BITBANG_PARAM_T i2c_bitbang_param;

    UI32_T              unit = 0;
    UI32_T              ch, sid, addr_len = 0, idx, dsh_len, txlen;
    UI32_T              word_addr = 0, word_data = 0;
    UI8_T               str[DSH_CMD_MAX_LENGTH] = {0};
    UI8_T               addr[I2C_BITBANG_MAX_ADDR_LEN] = {0};
    UI8_T               tx_data[I2C_BITBANG_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "bus", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "slave-id", &sid), token_idx, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "addr-len", &addr_len), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "addr", (C8_T *)str), token_idx, 2);

    I2C_BITBANG_CMD_CHECK_ADDR_LENGTH(addr_len, I2C_BITBANG_MAX_ADDR_LEN);
    if (str[0] != 0)
    {
        if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, addr, &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "data-len", &txlen), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "data", (C8_T *)str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    I2C_BITBANG_CMD_CHECK_DATA_LENGTH(txlen, I2C_BITBANG_MAX_DATA_LEN);
    if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, tx_data, &dsh_len))
    {
        return (DSH_E_SYNTAX_ERR);
    }
    I2C_BITBANG_CMD_CHECK_DATA_STR_LENGTH(txlen, dsh_len);

    for (idx = 0; idx < addr_len; idx++)
    {
        word_addr |= addr[idx] << (idx * 8);
    }

    i2c_bitbang_param.addr_len = addr_len;
    i2c_bitbang_param.addr = word_addr;
    i2c_bitbang_param.data_len = txlen;

    if (4 < i2c_bitbang_param.data_len)
    {
        i2c_bitbang_param.info.ptr_data = (UI32_T *)tx_data;
    }
    else
    {
        for (idx = 0; idx < i2c_bitbang_param.data_len; idx++)
        {
            word_data |= tx_data[idx] << (idx * 8);
        }
        i2c_bitbang_param.info.data = word_data;
    }

    rc = i2c_bitbang_write(unit, ch, sid, &i2c_bitbang_param);

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set i2c_bitbang fail !\n");
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T _i2c_bitbang_cmd_vec[] =
{
    {
        "set", 1, _i2c_bitbang_cmd_write,
        "app i2c-bitbang set [ unit=<UINT> ] bus=<UINT> slave-id=<UINT> "
        "[ addr-len=<UINT> addr=<HEX> ] data-len=<UINT> data=<HEX>\n"
    },
    {
        "show", 1, _i2c_bitbang_cmd_read,
        "app i2c-bitbang show [ unit=<UINT> ] bus=<UINT> slave-id=<UINT> "
        "[ addr-len=<UINT> addr=<HEX> ] data-len=<UINT>\n"
    }
};
/* clang-format on */

AIR_ERROR_NO_T
i2c_bitbang_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    if (DSH_E_NOT_FOUND ==
        dsh_dispatcher(tokens, token_idx, _i2c_bitbang_cmd_vec, sizeof(_i2c_bitbang_cmd_vec) / sizeof(DSH_VEC_T)))
    {
        return (i2c_bitbang_cmd_usager());
    }
    return (AIR_E_OK);
}

AIR_ERROR_NO_T
i2c_bitbang_cmd_usager()
{
    return (dsh_usager(_i2c_bitbang_cmd_vec, sizeof(_i2c_bitbang_cmd_vec) / sizeof(DSH_VEC_T)));
}
#endif
