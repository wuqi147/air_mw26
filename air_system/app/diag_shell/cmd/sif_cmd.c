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
#include <cmd/sif_cmd.h>

#include <air_error.h>
#include <air_sif.h>
#include <air_types.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

#define SIF_CMD_COMPARE_STR_LENGTH(__size__, __len__, __txt__)                 \
    do                                                                         \
    {                                                                          \
        if ((__size__) != (__len__))                                           \
        {                                                                      \
            osal_printf("***Error***, %s length dose not match !\n", __txt__); \
            return (AIR_E_BAD_PARAMETER);                                      \
        }                                                                      \
    } while (0)

#define SIF_CMD_CHECK_ADDR_LENGTH(__size__, __max__)                                     \
    do                                                                                   \
    {                                                                                    \
        if ((__size__) > (__max__))                                                      \
        {                                                                                \
            osal_printf("***Error***, maximum address length is %d bytes !\n", __max__); \
            return (AIR_E_BAD_PARAMETER);                                                \
        }                                                                                \
    } while (0)

#define SIF_CMD_CHECK_DATA_LENGTH(__size__, __max__)                                  \
    do                                                                                \
    {                                                                                 \
        if ((__size__) > (__max__))                                                   \
        {                                                                             \
            osal_printf("***Error***, maximum data length is %d bytes !\n", __max__); \
            return (AIR_E_BAD_PARAMETER);                                             \
        }                                                                             \
    } while (0)

#define SIF_CMD_CHECK_DATA_STR_LENGTH(__size__, __max__)                                  \
    do                                                                                    \
    {                                                                                     \
        if ((__size__) > (__max__))                                                       \
        {                                                                                 \
            osal_printf("***Error***, the length must not greater then string data !\n"); \
            return (AIR_E_BAD_PARAMETER);                                                 \
        }                                                                                 \
    } while (0)

static AIR_ERROR_NO_T
_sif_cmd_read(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_SIF_INFO_T  sif_info, sif_remote_info;
    AIR_SIF_PARAM_T sif_param;

    UI32_T          unit = 0;
    UI32_T          mask = 0xffffffff;
    UI32_T          ch, sid, idx, j, dsh_len, sch, ssid = 0;
    UI32_T          addr_len = 0, rxlen = 0, word_addr = 0;
    UI8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI8_T           addr[AIR_SIF_MAX_DATA_LEN] = {0};
    UI8_T           rx_data[AIR_SIF_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "bus", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "slave-id", &sid), token_idx, 2);

    /* we need to add remote_slave_id and remote_channel */
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "remote-bus", &sch), token_idx, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "remote-slave-id", &ssid), token_idx, 2);

    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "addr-len", &addr_len), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "addr", (C8_T *)str), token_idx, 2);

    SIF_CMD_CHECK_ADDR_LENGTH(addr_len, AIR_SIF_MAX_ADDR_LEN);

    if (0 != str[0])
    {
        if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, addr, &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
        SIF_CMD_COMPARE_STR_LENGTH(dsh_len, addr_len, "addr");
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "data-len", &rxlen), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    SIF_CMD_CHECK_DATA_LENGTH(rxlen, AIR_SIF_MAX_DATA_LEN);

    for (idx = 0; idx < addr_len; idx++)
    {
        word_addr |= (addr[idx] << (idx * 8));
    }

    if (4 == addr_len)
    {
        word_addr = (addr[0] << (0)) | (addr[1] << (8)) | (addr[2] << (16)) | (addr[3] << (24));
    }
    else
    {
        for (idx = 0; idx < addr_len % 4; idx++)
        {
            word_addr |= addr[idx] << (idx * 8);
        }
    }

    sif_info.channel = ch;
    sif_info.slave_id = sid;

    if (ssid)
    {
        sif_remote_info.channel = sch;
        sif_remote_info.slave_id = ssid;
    }

    sif_param.addr_len = addr_len;
    sif_param.addr = word_addr;
    sif_param.data_len = rxlen;
    if (4 < sif_param.data_len)
    {
        sif_param.info.ptr_data = (UI32_T *)rx_data;
    }
    else
    {
        sif_param.info.data = (UI32_T)*rx_data;
    }

    if (0 == ssid)
    {
        rc = air_sif_read(unit, &sif_info, &sif_param);
    }
    else
    {
        rc = air_sif_readByRemote(unit, &sif_info, &sif_remote_info, &sif_param);
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, show sif fail !\n");
        return (rc);
    }
    if (4 < sif_param.data_len)
    {
        for (idx = 0; idx < sif_param.data_len / 16; idx++)
        {
            osal_printf("data[0x%02x] = 0x%08x 0x%08x 0x%08x 0x%08x\n", idx * 16, *(sif_param.info.ptr_data + 4 * idx),
                        *(sif_param.info.ptr_data + 4 * idx + 1), *(sif_param.info.ptr_data + 4 * idx + 2),
                        *(sif_param.info.ptr_data + 4 * idx + 3));
        }
        if (sif_param.data_len % 16)
        {
            osal_printf("data[0x%02x] = ", idx * 16);
            for (j = 0; j < (sif_param.data_len % 16) / 4; j++)
            {
                osal_printf("0x%08x ", *(sif_param.info.ptr_data + 4 * idx + j));
            }
            if ((sif_param.data_len % 16) % 4)
            {
                osal_printf("0x%x ", (*(sif_param.info.ptr_data + 4 * idx + j) &
                                      (mask >> (4 - ((sif_param.data_len % 16) % 4)) * 8)));
            }
            osal_printf("\n");
        }
    }
    else
    {
        osal_printf("data[0x00] = 0x%x ", (sif_param.info.data));
    }

    osal_printf("\n");
    return (rc);
}

static AIR_ERROR_NO_T
_sif_cmd_write(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;
    AIR_SIF_INFO_T  sif_info, sif_remote_info;
    AIR_SIF_PARAM_T sif_param;

    UI32_T          unit = 0;
    UI32_T          ch, sid, addr_len, idx, dsh_len, txlen, sch, ssid = 0;
    UI32_T          word_addr = 0, word_data = 0;
    UI8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI8_T           addr[AIR_SIF_MAX_DATA_LEN] = {0};
    UI8_T           tx_data[AIR_SIF_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "bus", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "slave-id", &sid), token_idx, 2);

    /* we need to add remote_slave_id and remote_channel */
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "remote-bus", &sch), token_idx, 2);
    DSH_CHECK_OPT(dsh_getUint(tokens, token_idx, "remote-slave-id", &ssid), token_idx, 2);

    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "addr-len", &addr_len), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "addr", (C8_T *)str), token_idx, 2);

    SIF_CMD_CHECK_ADDR_LENGTH(addr_len, AIR_SIF_MAX_ADDR_LEN);
    if (0 != str[0])
    {
        if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, addr, &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
        SIF_CMD_COMPARE_STR_LENGTH(dsh_len, addr_len, "addr");
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "data-len", &txlen), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "data", (C8_T *)str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    SIF_CMD_CHECK_DATA_LENGTH(txlen, AIR_SIF_MAX_DATA_LEN);
    if (AIR_E_OK != dsh_transStrToBytes((C8_T *)str, tx_data, &dsh_len))
    {
        return (DSH_E_SYNTAX_ERR);
    }
    SIF_CMD_CHECK_DATA_STR_LENGTH(txlen, dsh_len);

    for (idx = 0; idx < addr_len; idx++)
    {
        word_addr |= addr[idx] << (idx * 8);
    }

    if (4 == addr_len)
    {
        word_addr = (addr[0] << (0)) | (addr[1] << (8)) | (addr[2] << (16)) | (addr[3] << (24));
    }
    else
    {
        for (idx = 0; idx < addr_len % 4; idx++)
        {
            word_addr |= addr[idx] << (idx * 8);
        }
    }

    sif_info.channel = ch;
    sif_info.slave_id = sid;

    if (ssid)
    {
        sif_remote_info.channel = sch;
        sif_remote_info.slave_id = ssid;
    }

    sif_param.addr_len = addr_len;
    sif_param.addr = word_addr;
    sif_param.data_len = txlen;

    if (4 < sif_param.data_len)
    {
        sif_param.info.ptr_data = (UI32_T *)tx_data;
    }
    else
    {
        for (idx = 0; idx < sif_param.data_len; idx++)
        {
            word_data |= tx_data[idx] << (idx * 8);
        }
        sif_param.info.data = word_data;
    }

    if (0 == ssid)
    {
        rc = air_sif_write(unit, &sif_info, &sif_param);
    }
    else
    {
        rc = air_sif_writeByRemote(unit, &sif_info, &sif_remote_info, &sif_param);
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("***Error***, set sif fail !\n");
    }
    return rc;
}

/* clang-format off */
const static DSH_VEC_T  _sif_cmd_vec[] =
{
    {
        "set", 1, _sif_cmd_write,
        "sif set [ unit=<UINT> ] bus=<UINT> slave-id=<UINT>\n"
        "[ remote-bus=<UINT> remote-slave-id=<HEX> ]\n"
        "addr-len=<UINT> addr=<HEX> data-len=<UINT> data=<HEX>\n"
    },
    {
        "show", 1, _sif_cmd_read,
        "sif show [ unit=<UINT> ] bus=<UINT> slave-id=<UINT>\n"
        "[ remote-bus=<UINT> remote-slave-id=<HEX> ]\n"
        "addr-len=<UINT> addr=<HEX> data-len=<UINT>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
sif_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _sif_cmd_vec, sizeof(_sif_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
sif_cmd_usager()
{
    return (dsh_usager(_sif_cmd_vec, sizeof(_sif_cmd_vec) / sizeof(DSH_VEC_T)));
}
