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

#include <air_gpio.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_bitmap.h>
#include <dsh_parser.h>
#include <dsh_util.h>
#include "gpio_cmd.h"
#include <pp_def.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
*/

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM BODIES
*/
/* STATIC VARIABLE DECLARATIONS
 */
const static char *_air_gpio_intr_edge_string [] =
{
    "DISABLED",
    "RISING",
    "FALLING",
    "BOTH"
};

const static char *_air_gpio_intr_level_string [] =
{
    "DISABLED",
    "HIGH",
    "LOW"
};

static int
_gpio_cmd_getDirection(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    unsigned char mode = 0;

    /*
     * Command format
     * gpio get dir pin=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getDirection(pin, &mode);
        if (E_OK == rc)
        {
            printf(" - direction = %s\n", (mode) ? "output" : "input");
        }
        else
        {
            printf("***Error***, show pin %u direction error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_setDirection(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    char dir[DSH_CMD_MAX_LENGTH] = {0};
    unsigned char mode = AIR_GPIO_DIRECTION_INPUT;

    /*
     * Command format
     * gpio set dir pin=<UINTLIST> state=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", dir), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(dir, "in"))
    {
        mode = AIR_GPIO_DIRECTION_INPUT;
    }
    else if(E_OK == dsh_checkString(dir, "out"))
    {
        mode = AIR_GPIO_DIRECTION_OUTPUT;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        rc = air_gpio_setDirection(pin, mode);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u direction error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

static int
_gpio_cmd_getOutputEnable(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    int state = 0;

    /*
     * Command format
     * gpio get out-enable pin=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getOutputEnable(pin, &state);
        if (E_OK == rc)
        {
            printf(" - output enable state = %s\n", (state) ? "enable" : "disable");
        }
        else
        {
            printf("***Error***, show pin %u output enable state error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_setOutputEnable(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    char str[DSH_CMD_MAX_LENGTH] = {0};
    int mode = FALSE;

    /*
     * Command format
     * gpio set out-enable pin=<UINTLIST> state={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(str, "enable"))
    {
        mode = TRUE;
    }
    else if (E_OK == dsh_checkString(str, "disable"))
    {
        mode = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        rc = air_gpio_setOutputEnable(pin, mode);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u output enable state error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

static int
_gpio_cmd_getValue(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    int value = 0;

    /*
     * Command format
     * gpio get value pin=<UINTLIST>
     */

     /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getValue(pin, &value);
        if (E_OK == rc)
        {
            printf(" - State = %s\n", (value) ? "HIGH" : "LOW");
        }
        else
        {
            printf("***Error***, show pin %u state error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_setValue(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    char str[DSH_CMD_MAX_LENGTH] = {0};
    int value = FALSE;

    /*
     * Command format
     * gpio set value pin=<UINTLIST> state={ high | low }
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(str, "high"))
    {
        value = TRUE;
    }
    else if (E_OK == dsh_checkString(str, "low"))
    {
        value = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        rc = air_gpio_setValue(pin, value);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u state error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

static int
_gpio_cmd_getInterruptEdgeDetectMode(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    unsigned char mode = 0;

    /*
     * Command format
     * gpio get edge pin=<UINTLIST>
     */

     /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getInterruptEdgeDetectMode(pin, &mode);
        if (E_OK == rc)
        {
            printf(" - interrupt edge trigger mode = %s\n", _air_gpio_intr_edge_string[mode]);
        }
        else
        {
            printf("***Error***, show pin %u intr edge triggered mode error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_setInterruptEdgeDetectMode(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    char str[DSH_CMD_MAX_LENGTH] = {0};
    unsigned char mode = AIR_GPIO_EDGE_TRIGGER_DISABLE;

    /*
     * Command format
     * gpio set edge pin=<UINTLIST> state={ disable | rising | falling | both }
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(str, "disable"))
    {
        mode = AIR_GPIO_EDGE_TRIGGER_DISABLE;
    }
    else if (E_OK == dsh_checkString(str, "rising"))
    {
        mode = AIR_GPIO_EDGE_TRIGGER_RISING;
    }
    else if (E_OK == dsh_checkString(str, "falling"))
    {
        mode = AIR_GPIO_EDGE_TRIGGER_FALLING;
    }
    else if (E_OK == dsh_checkString(str, "both"))
    {
        mode = AIR_GPIO_EDGE_TRIGGER_BOTH;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        rc = air_gpio_setInterruptEdgeDetectMode(pin, mode);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u interrupt edge trigger error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

static int
_gpio_cmd_getInterruptLevelDetectMode(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    unsigned char mode = 0;

    /*
     * Command format
     * gpio get level pin=<UINTLIST>
     */

     /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getInterruptLevelDetectMode(pin, &mode);
        if (E_OK == rc)
        {
            printf(" - interrupt level trigger mode = %s\n", _air_gpio_intr_level_string[mode]);
        }
        else
        {
            printf("***Error***, show pin %u intr level triggered mode error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_setInterruptLevelDetectMode(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    char str[DSH_CMD_MAX_LENGTH] = {0};
    unsigned char mode = AIR_GPIO_LEVEL_TRIGGER_DISABLE;

    /*
     * Command format
     * gpio set level pin=<UINTLIST> state={ disable | high | low }
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(str, "disable"))
    {
        mode = AIR_GPIO_LEVEL_TRIGGER_DISABLE;
    }
    else if (E_OK == dsh_checkString(str, "high"))
    {
        mode = AIR_GPIO_LEVEL_TRIGGER_HIGH;
    }
    else if (E_OK == dsh_checkString(str, "low"))
    {
        mode = AIR_GPIO_LEVEL_TRIGGER_LOW;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        rc = air_gpio_setInterruptLevelDetectMode(pin, mode);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u interrupt level trigger error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

static int
_gpio_cmd_getInterruptStatus(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};
    int status = 0;

    /*
     * Command format
     * gpio get intr pin=<UINTLIST>
     */

     /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        printf(" GPIO pin %u", pin);
        rc = air_gpio_getInterruptStatus(pin, &status);
        if (E_OK == rc)
        {
            printf(" - Interrupt = %s\n", (status) ? "asserted" : "de-asserted");
        }
        else
        {
            printf("***Error***, show pin %u interrupt state error\n", pin);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_gpio_cmd_clearInterrupt(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int          rc = E_OK;
    unsigned int pin = 0;
    unsigned int pbm[AIR_GPIO_BITMAP_SIZE] = {0};

    /*
     * Command format
     * gpio clear intr pin=<UINTLIST>
     */

    /* paser tokens */
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "pin", pbm, AIR_GPIO_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, pin, AIR_GPIO_BITMAP_SIZE)
    {
        /* Set interrupt status as TRUE to clear interrupt */
        rc = air_gpio_clearInterrupt(pin);
        if (E_OK != rc)
        {
            printf("***Error***, set pin %u interrupt state error\n", pin);
            break;
        }
    }
    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
const static DSH_VEC_T  _gpio_cmd_vec[] =
{
    {
        "get dir", 2, _gpio_cmd_getDirection,
        "gpio get dir pin=<UINTLIST>\n"
    },
    {
        "set dir", 2, _gpio_cmd_setDirection,
        "gpio set dir pin=<UINTLIST> state={ in | out }\n"
    },
    {
        "get out-enable", 2, _gpio_cmd_getOutputEnable,
        "gpio get out-enable pin=<UINTLIST>\n"
    },
    {
        "set out-enable", 2, _gpio_cmd_setOutputEnable,
        "gpio set out-enable pin=<UINTLIST> state={ enable | disable }\n"
    },
    {
        "get value", 2, _gpio_cmd_getValue,
        "gpio get value pin=<UINTLIST>\n"
    },
    {
        "set value", 2, _gpio_cmd_setValue,
        "gpio set value pin=<UINTLIST> state={ high | low }\n"
    },
    {
        "get edge", 2, _gpio_cmd_getInterruptEdgeDetectMode,
        "gpio get edge pin=<UINTLIST>\n"
    },
    {
        "set edge", 2, _gpio_cmd_setInterruptEdgeDetectMode,
        "gpio set edge pin=<UINTLIST> state={ disable | rising | falling | both }\n"
    },
    {
        "get level", 2, _gpio_cmd_getInterruptLevelDetectMode,
        "gpio get level pin=<UINTLIST>\n"
    },
    {
        "set level", 2, _gpio_cmd_setInterruptLevelDetectMode,
        "gpio set level pin=<UINTLIST> state={ disable | high | low }\n"
    },
    {
        "get intr", 2, _gpio_cmd_getInterruptStatus,
        "gpio get intr pin=<UINTLIST>\n"
    },
    {
        "clear intr", 2, _gpio_cmd_clearInterrupt,
        "gpio clear intr pin=<UINTLIST>\n"
    },
};

int
gpio_cmd_dispatcher(
    const char                  *tokens[],
    unsigned int                token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _gpio_cmd_vec, sizeof(_gpio_cmd_vec)/sizeof(DSH_VEC_T)));
}

int
gpio_cmd_usager()
{
    return (dsh_usager(_gpio_cmd_vec, sizeof(_gpio_cmd_vec)/sizeof(DSH_VEC_T)));
}

