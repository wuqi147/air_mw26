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

#include <air_i2c.h>
#include <dsh_parser.h>
#include <dsh_util.h>
#include "i2c_cmd.h"
#include <pp_def.h>

#define I2C_COMPARE_STR_LENGTH(__size__, __len__, __txt__) do       \
    {                                                               \
        if ((__size__) != (__len__))                                \
        {                                                           \
            printf("***Error***, %s length dose not match !\n", __txt__);\
            return(E_BAD_PARAMETER);                                \
        }                                                           \
    } while(0)

#define I2C_CHECK_ADDR_LENGTH(__size__, __op__, __max__) do         \
    {                                                               \
        if ((__size__) __op__ (__max__))                            \
        {                                                           \
            printf("***Error***, Maximum address length is %d bytes !\n",__max__);\
            return(E_BAD_PARAMETER);                                \
        }                                                           \
    } while(0)

#define I2C_CHECK_DATA_LENGTH(__size__, __op__, __max__) do         \
    {                                                               \
        if ((__size__) __op__ (__max__))                            \
        {                                                           \
            printf("***Error***, Maximum data length is %d bytes !\n",__max__);\
            return(E_BAD_PARAMETER);                                \
        }                                                           \
    } while(0)

#define I2C_CHECK_DATA_STR_LENGTH(__size__, __op__, __max__) do     \
    {                                                               \
        if ((__size__) __op__ (__max__))                            \
        {                                                           \
            printf("***Error***, The length must not greater then string data !\n");\
            return(E_BAD_PARAMETER);                                \
        }                                                           \
    } while(0)

static int
_i2c_cmd_open(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int   unit = 0;
    unsigned int   ch;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "ch", &ch), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_i2c_open(unit,ch);
    if (E_OK != rc)
    {
        printf("***Error***, Open the I2C device-%d failed !\n", ch);
        return (rc);
    }
    printf("Open I2C device-%d ok !\n", ch);
    return (rc);
}

static int
_i2c_cmd_setClock(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    char           str[DSH_CMD_MAX_LENGTH] = {0};
    unsigned int   unit = 0;
    unsigned int   ch, clk;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "ch", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "speed", str), token_idx, 2);
    if (E_OK == dsh_checkString(str, "100k"))
    {
        clk = I2C_Clock_100K;
    }
    else if (E_OK == dsh_checkString(str, "400k"))
    {
        clk = I2C_Clock_400K;
    }
    else if (E_OK == dsh_checkString(str, "1m"))
    {
        clk = I2C_Clock_1M;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_i2c_setClock(unit, ch, clk);
    if (E_OK != rc)
    {
        printf("***Error***, set I2C clock fail\n");
        return (rc);
    }
    printf("Set I2C clock ok !\n");
    return (rc);
}

static int
_i2c_cmd_getClock(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int   unit = 0;
    unsigned int   ch, clk;

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "ch", &ch), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    rc = air_i2c_getClock(unit, ch, &clk);
    if (E_OK != rc)
    {
        printf("***Error***, Get I2C clock fail\n");
        return (rc);
    }
    printf("Current I2C clock is ");
    if (clk == I2C_Clock_100K)
    {
        printf("100K\n");
    }
    else if (clk == I2C_Clock_400K)
    {
        printf("400K\n");
    }
    else if (clk == I2C_Clock_1M)
    {
        printf("1M\n");
    }
    else
    {
        printf("Unknow\n");
    }
    return (rc);
}

static int
_i2c_cmd_read(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int   rc = E_OK;
    AIR_I2C_Master_T i2c_parm;

    unsigned int     unit = 0;
    unsigned int     ch, sid, idx, dsh_len;
    unsigned int     addrlen = 0, rxlen = 0;
    unsigned char    str[DSH_CMD_MAX_LENGTH] = {0};
    unsigned char    addr[DIAG_I2C_MAX_DATA_LEN] = {0};
    unsigned char    rxData[DIAG_I2C_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "ch", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &sid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "addrlen", &addrlen), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "addr", (char*)str), token_idx, 2);

    I2C_CHECK_ADDR_LENGTH(addrlen, >, DIAG_I2C_MAX_ADDR_LEN);
    if(0 != str[0])
    {
        if(E_OK != dsh_transStrToBytes((char*)str, addr , &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
        I2C_COMPARE_STR_LENGTH(dsh_len, addrlen, "Address");
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "datalen", &rxlen), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);
    I2C_CHECK_DATA_LENGTH(rxlen, >, DIAG_I2C_MAX_DATA_LEN);

    i2c_parm.deviceID = sid;
    i2c_parm.regAddrLen = addrlen;
    i2c_parm.regAddr = addr;
    i2c_parm.dataLen = rxlen;
    i2c_parm.buffPtr = rxData;

    rc = air_i2c_read(unit, ch , &i2c_parm);
    if (E_NOT_INITED == rc)
    {
        printf("***Error***, i2c not open !\n");
        return (rc);
    }
    if (E_OK != rc)
    {
        printf("***Error***, read i2c fail !\n");
        return (rc);
    }
    printf("\nRead : 0x");
    for (idx = i2c_parm.dataLen; idx > 0; idx--)
    {
        printf("%02x ",(unsigned char)*(i2c_parm.buffPtr + idx - 1));
    }
    printf("\n");
    return (rc);
}

static int
_i2c_cmd_write(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int   rc = E_OK;
    AIR_I2C_Master_T i2c_parm;

    unsigned int     unit = 0;
    unsigned int     ch, sid, addrlen, dsh_len, txlen;
    unsigned char    str[DSH_CMD_MAX_LENGTH] = {0};
    unsigned char    addr[DIAG_I2C_MAX_DATA_LEN] = {0};
    unsigned char    txData[DIAG_I2C_MAX_DATA_LEN] = {0};

    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "ch", &ch), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "sid", &sid), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "addrlen", &addrlen), token_idx, 2);
    DSH_CHECK_OPT(dsh_getString(tokens, token_idx, "addr", (char*)str), token_idx, 2);

    I2C_CHECK_ADDR_LENGTH(addrlen, >, DIAG_I2C_MAX_ADDR_LEN);
    if(0 != str[0])
    {
        if(E_OK != dsh_transStrToBytes((char*)str, addr , &dsh_len))
        {
            return (DSH_E_SYNTAX_ERR);
        }
        I2C_COMPARE_STR_LENGTH(dsh_len, addrlen, "Address");
    }
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "datalen", &txlen), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "data", (char*)str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    I2C_CHECK_DATA_LENGTH(txlen, >, DIAG_I2C_MAX_DATA_LEN);
    if(E_OK != dsh_transStrToBytes((char*)str, txData ,&dsh_len))
    {
        return (DSH_E_SYNTAX_ERR);
    }
    I2C_CHECK_DATA_STR_LENGTH(txlen, >, dsh_len);

    i2c_parm.deviceID = sid;
    i2c_parm.regAddrLen = addrlen;
    i2c_parm.regAddr = addr;
    i2c_parm.dataLen = txlen;
    i2c_parm.buffPtr = txData;

    rc = air_i2c_write(unit, ch , &i2c_parm);

    if (E_NOT_INITED == rc)
    {
        printf("***Error***, i2c not open !\n");
        return (rc);
    }
    if (E_OK != rc)
    {
        printf("***Error***, write i2c fail !\n");
        return (rc);
    }
    printf("\nWrite i2c ok !\n");
    return (rc);
}

/* -------------------------------------------------------------- callback */
const static DSH_VEC_T  _i2c_cmd_vec[] =
{
    {
        "open", 1, _i2c_cmd_open,
        "i2c open [ unit=<UINT> ] ch={ 0 | 1 }\n"
    },
    {
        "read", 1, _i2c_cmd_read,
        "i2c read [ unit=<UINT> ] ch={ 0 | 1 } sid=<UINT> addrlen=<UINT> [addr=<0x1.02.34...>] datalen=<UINT>\n"
    },
    {
        "write", 1, _i2c_cmd_write,
        "i2c write [ unit=<UINT> ] ch={ 0 | 1 } sid=<UINT> addrlen=<UINT> [addr=<0x1.02.34...>] datalen=<UINT> data=<0x1.02.34...>\n"
    },
    {
        "setClock", 1, _i2c_cmd_setClock,
        "i2c setClock [ unit=<UINT> ] ch={ 0 | 1 } speed={ 100K | 400K | 1M }\n"
    },
    {
        "getClock", 1, _i2c_cmd_getClock,
        "i2c getClock [ unit=<UINT> ] ch={ 0 | 1 }\n"
    },
};

int
i2c_cmd_dispatcher(
    const char                  *tokens[],
    unsigned int                token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _i2c_cmd_vec, sizeof(_i2c_cmd_vec)/sizeof(DSH_VEC_T)));
}

int
i2c_cmd_usager()
{
    return (dsh_usager(_i2c_cmd_vec, sizeof(_i2c_cmd_vec)/sizeof(DSH_VEC_T)));
}
