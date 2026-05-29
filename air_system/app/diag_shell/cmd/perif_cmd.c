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

#include <cmd/perif_cmd.h>

#include <air_error.h>
#include <air_perif.h>
#include <air_types.h>
#include <cmlib/cmlib_bitmap.h>
#include <cmlib/cmlib_port.h>
#include <hal/common/hal.h>
#include <osal/osal.h>
#include <parser/dsh_parser.h>
#include <parser/dsh_util.h>

#define PERIF_CMD_BITMAP_SIZE (2UL)

static AIR_ERROR_NO_T
_perif_cmd_setGpioDataDirection(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};
    UI16_T         direction = AIR_PERIF_GPIO_DIRECTION_INPUT;
    C8_T           dir[DSH_CMD_MAX_LENGTH] = {0};

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "direction", dir), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(dir, "input"))
    {
        direction = AIR_PERIF_GPIO_DIRECTION_INPUT;
    }
    else if (AIR_E_OK == dsh_checkString(dir, "output"))
    {
        direction = AIR_PERIF_GPIO_DIRECTION_OUTPUT;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_setGpioDirection(unit, pin, direction);
        if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else if (AIR_E_OK != rc && AIR_E_NOT_INITED != rc)
        {
            osal_printf("***Error***, set gpio pin %u direction error\n", pin);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_showGpioDataDirection(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0, direction = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_getGpioDirection(unit, pin, &direction);
        if (AIR_E_OK == rc)
        {
            osal_printf("gpio pin %02u", pin);
            osal_printf(" - direction = %s\n", (direction) ? "output" : "input");
        }
        else if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_setGpioOutputData(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI16_T         data = AIR_PERIF_GPIO_DATA_LOW;

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "data", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "high"))
    {
        data = AIR_PERIF_GPIO_DATA_HIGH;
    }
    else if (AIR_E_OK == dsh_checkString(str, "low"))
    {
        data = AIR_PERIF_GPIO_DATA_LOW;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_setGpioOutputData(unit, pin, data);

        if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else if (AIR_E_OK != rc && AIR_E_NOT_INITED != rc)
        {
            osal_printf("***Error***, set gpio pin %u data error\n", pin);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_showGpioInputData(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0, data = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_getGpioInputData(unit, pin, &data);
        if (AIR_E_OK == rc)
        {
            osal_printf("gpio pin %02u", pin);
            osal_printf(" - data = %s\n", (data) ? "high" : "low");
        }
        else if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else
        {
            osal_printf("***Error***, show gpio pin %u data error\n", pin);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_setGpioAutoMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    BOOL_T         mode = 0;

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "mode", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "enable"))
    {
        mode = 1;
    }
    else if (AIR_E_OK == dsh_checkString(str, "disable"))
    {
        mode = 0;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_setGpioOutputAutoMode(unit, pin, mode);

        if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else if (AIR_E_OK != rc && AIR_E_NOT_INITED != rc)
        {
            osal_printf("***Error***, set gpio pin %u data error\n", pin);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_showGpioAutoMode(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0;
    BOOL_T         mode = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_getGpioOutputAutoMode(unit, pin, &mode);
        if (AIR_E_OK == rc)
        {
            osal_printf("gpio pin %02u", pin);
            osal_printf(" - auto mode = %s\n", (mode) ? "enable" : "disable");
        }

        else if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else
        {
            osal_printf("***Error***, show gpio pin %u data error\n", pin);
        }
    }
    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_setGpioAutoPatt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};
    C8_T           str[DSH_CMD_MAX_LENGTH] = {0};
    UI16_T         pattern = AIR_PERIF_GPIO_PATT_LAST;

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "pattern", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (AIR_E_OK == dsh_checkString(str, "0.5hz"))
    {
        pattern = AIR_PERIF_GPIO_PATT_HZ_HALF;
    }
    else if (AIR_E_OK == dsh_checkString(str, "1hz"))
    {
        pattern = AIR_PERIF_GPIO_PATT_HZ_ONE;
    }
    else if (AIR_E_OK == dsh_checkString(str, "2hz"))
    {
        pattern = AIR_PERIF_GPIO_PATT_HZ_TWO;
    }
    else if (AIR_E_OK == dsh_checkString(str, "8hz"))
    {
        pattern = AIR_PERIF_GPIO_PATT_HZ_EIGHT;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_setGpioOutputAutoPatt(unit, pin, pattern);

        if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else if (AIR_E_OK != rc && AIR_E_NOT_INITED != rc)
        {
            osal_printf("***Error***, set gpio pin %u data error\n", pin);
        }
    }

    return rc;
}

static AIR_ERROR_NO_T
_perif_cmd_showGpioAutoPatt(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    UI32_T         unit = 0, pin = 0, pattern = 0;
    UI32_T         pbm[PERIF_CMD_BITMAP_SIZE] = {0};

    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin-number", pbm, PERIF_CMD_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, PERIF_CMD_BITMAP_SIZE)
    {
        rc = air_perif_getGpioOutputAutoPatt(unit, pin, &pattern);
        if (AIR_E_OK == rc)
        {
            osal_printf("gpio pin %02u", pin);

            if (pattern == AIR_PERIF_GPIO_PATT_HZ_HALF)
            {
                osal_printf(" - auto pattern = 0.5 hz\n");
            }
            else if (pattern == AIR_PERIF_GPIO_PATT_HZ_ONE)
            {
                osal_printf(" - auto pattern = 1 hz\n");
            }
            else if (pattern == AIR_PERIF_GPIO_PATT_HZ_TWO)
            {
                osal_printf(" - auto pattern = 2 hz\n");
            }
            else if (pattern == AIR_PERIF_GPIO_PATT_HZ_EIGHT)
            {
                osal_printf(" - auto pattern = 8 hz\n");
            }
            else
            {
                osal_printf(" *** Pattern ERROR ***\n");
            }
        }

        else if (AIR_E_NOT_INITED == rc)
        {
            osal_printf("***Error***, gpio pin %u not init\n", pin);
        }
        else
        {
            osal_printf("***Error***, show gpio pin %u data error\n", pin);
        }
    }
    return rc;
}

/* -------------------------------------------------------------- callback */
/* clang-format off */
const static DSH_VEC_T  _perif_cmd_vec[] =
{
    {
        "set gpio data-direction", 3, _perif_cmd_setGpioDataDirection,
        "perif set gpio data-direction [ unit=<UINT> ] pin-number=<UINTLIST>\n"
        "direction={ input | output }\n"
    },
    {
        "show gpio data-direction", 3, _perif_cmd_showGpioDataDirection,
        "perif show gpio data-direction [ unit=<UINT> ] pin-number=<UINTLIST>\n"
    },
    {
        "set gpio output-data", 3, _perif_cmd_setGpioOutputData,
        "perif set gpio output-data [ unit=<UINT> ] pin-number=<UINTLIST>\n"
        "data={ low | high }\n"
    },
    {
        "show gpio input-data", 3, _perif_cmd_showGpioInputData,
        "perif show gpio input-data [ unit=<UINT> ] pin-number=<UINTLIST>\n"
    },
    {
        "set gpio auto-mode", 3, _perif_cmd_setGpioAutoMode,
        "perif set gpio auto-mode [ unit=<UINT> ] pin-number=<UINTLIST>\n"
        "mode={ enable | disable }\n"
    },
    {
        "show gpio auto-mode", 3, _perif_cmd_showGpioAutoMode,
        "perif show gpio auto-mode [ unit=<UINT> ] pin-number=<UINTLIST>\n"
    },
    {
        "set gpio auto-pattern", 3, _perif_cmd_setGpioAutoPatt,
        "perif set gpio auto-pattern [ unit=<UINT> ] pin-number=<UINTLIST>\n"
        "pattern={ 0.5hz | 1hz | 2hz | 8hz }\n"
    },
    {
        "show gpio auto-pattern", 3, _perif_cmd_showGpioAutoPatt,
        "perif show gpio auto-pattern [ unit=<UINT> ] pin-number=<UINTLIST>\n"
    },
};
/* clang-format on */

AIR_ERROR_NO_T
perif_cmd_dispatcher(
    const C8_T *tokens[],
    UI32_T      token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _perif_cmd_vec, sizeof(_perif_cmd_vec) / sizeof(DSH_VEC_T)));
}

AIR_ERROR_NO_T
perif_cmd_usager()
{
    return (dsh_usager(_perif_cmd_vec, sizeof(_perif_cmd_vec) / sizeof(DSH_VEC_T)));
}
