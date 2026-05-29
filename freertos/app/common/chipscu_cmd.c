/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Airoha Technology Corp. (C) 2022
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

#include <air_chipscu.h>
#include <cmlib/cmlib_port.h>
#include <cmlib/cmlib_bitmap.h>
#include <dsh_parser.h>
#include <dsh_util.h>
#include "chipscu_cmd.h"
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

static int
_chipscu_cmd_getCpuIntrTrigType(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int unit = 0;
    unsigned int pbm[AIR_CHIPSCU_IRQ_BITMAP_SIZE] = {0};
    AIR_CHIPSCU_INTR_SOURCE_T intr_src_idx = AIR_CHIPSCU_INTR_SOURCE_GPIO;
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T state = AIR_CHIPSCU_LEVEL_TRIGGER_HIGH;

    /*
     * Command format
     * chipscu get cpu-intr-trig-type [ unit=<UINT> ] intr-src=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "intr-src", pbm, AIR_CHIPSCU_IRQ_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    CMLIB_BITMAP_BIT_FOREACH(pbm, intr_src_idx, AIR_CHIPSCU_IRQ_BITMAP_SIZE)
    {
        printf("CPU interrupt source %u",intr_src_idx);
        rc = air_chipscu_getInterruptTriggerMode(unit, intr_src_idx, &state);
        if (E_OK == rc)
        {
            printf(" - trigger type = %s\n", (state) ? "positive edge trigger" : "high level trigger");
        }
        else
        {
            printf("***Error***, show intr_src %u trigger type error\n", intr_src_idx);
            break;
        }
        printf("\n");
    }

    return rc;
}

static int
_chipscu_cmd_setCpuIntrTrigType(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int unit = 0;
    unsigned int pbm[AIR_CHIPSCU_IRQ_BITMAP_SIZE] = {0};
    char trig_type[DSH_CMD_MAX_LENGTH] = {0};
    AIR_CHIPSCU_INTR_SOURCE_T intr_src_idx = AIR_CHIPSCU_INTR_SOURCE_GPIO;
    AIR_CHIPSCU_INTR_TRIGGER_TYPE_T state = AIR_CHIPSCU_LEVEL_TRIGGER_HIGH;

    /*
     * Command format
     * chipscu set cpu-intr-trig-type [ unit=<UINT> ] intr-src=<UINT> state={ high-level | positive-edge }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getBitmap(tokens, token_idx, "intr-src", pbm, AIR_CHIPSCU_IRQ_BITMAP_SIZE), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", trig_type), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(trig_type, "high-level"))
    {
        state = AIR_CHIPSCU_LEVEL_TRIGGER_HIGH;
    }
    else if(E_OK == dsh_checkString(trig_type, "positive-edge"))
    {
        state = AIR_CHIPSCU_EDGE_TRIGGER_RISING;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    CMLIB_BITMAP_BIT_FOREACH(pbm, intr_src_idx, AIR_CHIPSCU_IRQ_BITMAP_SIZE)
    {
        rc = air_chipscu_setInterruptTriggerMode(unit, intr_src_idx, state);
        if (E_OK != rc)
        {
            printf("***Error***, set intr_src %u trigger type error\n", intr_src_idx);
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
_chipscu_cmd_getIomuxFuncState(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int unit = 0;
    AIR_CHIPSCU_IOMUX_FUNC_T function = AIR_CHIPSCU_IOMUX_LAN0_LED0_MODE;
    int state = FALSE;

    /*
     * Command format
     * chipscu get iomux [ unit=<UINT> ] func-idx=<UINT>
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "func-idx", &function), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    printf("chipscu iomux %u", function);
    rc = air_chipscu_getIomuxFuncState(unit, function, &state);
    if (E_OK == rc)
    {
        printf(" - state = %s\n", (state) ? "enable" : "disable");
    }
    else
    {
        printf("***Error***, get function %u state error\n", function);
    }
    printf("\n");

    return rc;
}

static int
_chipscu_cmd_setIomuxFuncState(
    const char          *tokens[],
    unsigned int        token_idx)
{
    int rc = E_OK;
    unsigned int unit = 0;
    AIR_CHIPSCU_IOMUX_FUNC_T function = AIR_CHIPSCU_IOMUX_LAN0_LED0_MODE;
    int state = FALSE;
    char str[DSH_CMD_MAX_LENGTH] = {0};

    /*
     * Command format
     * chipscu set iomux [ unit=<UINT> ] func-idx=<UINT> state={ enable | disable }
     */

    /* paser tokens */
    DSH_CHECK_GET_UNIT(tokens, token_idx, &unit, 2);
    DSH_CHECK_PARAM(dsh_getUint(tokens, token_idx, "func-idx", &function), token_idx, 2);
    DSH_CHECK_PARAM(dsh_getString(tokens, token_idx, "state", str), token_idx, 2);
    DSH_CHECK_LAST_TOKEN(tokens[token_idx]);

    if (E_OK == dsh_checkString(str, "enable"))
    {
        state = TRUE;
    }
    else if (E_OK == dsh_checkString(str, "disable"))
    {
        state = FALSE;
    }
    else
    {
        return DSH_E_SYNTAX_ERR;
    }

    rc = air_chipscu_setIomuxFuncState(unit, function, state);
    if (E_OK != rc)
    {
        printf("***Error***, set function %u state error\n", function);
    }

    if (E_OK == rc)
    {
        printf("Success.\n");
    }

    return rc;
}

/* -------------------------------------------------------------- callback */
const static DSH_VEC_T  _chipscu_cmd_vec[] =
{
    /* Trig-type cmd should move to INIT or other component in the future, TBD */
    {
        "get cpu-intr-trig-type", 2, _chipscu_cmd_getCpuIntrTrigType,
        "chipscu get cpu-intr-trig-type [ unit=<UINT> ] intr-src=<UINT>\n"
        "Note: For intr-src, 0:GPIO  1:EXT_IRQ0  2:EXT_IRQ1  3:EXT_IRQ2\n"
    },
    {
        "set cpu-intr-trig-type", 2, _chipscu_cmd_setCpuIntrTrigType,
        "chipscu set cpu-intr-trig-type [ unit=<UINT> ] intr-src=<UINT> state={ high-level | positive-edge }\n"
        "Note: For intr-src, 0:GPIO  1:EXT_IRQ0  2:EXT_IRQ1  3:EXT_IRQ2\n"
    },
    {
        "get iomux", 2, _chipscu_cmd_getIomuxFuncState,
        "chipscu get iomux [ unit=<UINT> ] func-idx=<UINT>\n"
        "Note: For func-idx, 0~23:LED  28:IRQ  33~54:force GPIO\n"
    },
    {
        "set iomux", 2, _chipscu_cmd_setIomuxFuncState,
        "chipscu set iomux [ unit=<UINT> ] func-idx=<UINT> state={ enable | disable }\n"
        "Note: For func-idx, 0~23:LED  28:IRQ  33~54:force GPIO\n"
    },
};

int
chipscu_cmd_dispatcher(
    const char                  *tokens[],
    unsigned int                token_idx)
{
    return (dsh_dispatcher(tokens, token_idx, _chipscu_cmd_vec, sizeof(_chipscu_cmd_vec)/sizeof(DSH_VEC_T)));
}

int
chipscu_cmd_usager()
{
    return (dsh_usager(_chipscu_cmd_vec, sizeof(_chipscu_cmd_vec)/sizeof(DSH_VEC_T)));
}

