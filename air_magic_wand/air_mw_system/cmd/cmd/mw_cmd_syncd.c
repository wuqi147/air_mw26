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

#include "mw_error.h"
#include "mw_types.h"

#include "osapi.h"
#include "osapi_string.h"
#include "osapi_message.h"

#include "mw_cmd_parser.h"
#include "mw_cmd_util.h"

#include "mw_cmd_syncd.h"
#include "syncd.h"
#include "syncd_in.h"
#include "db_api.h"

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
*/

/* LOCAL SUBPROGRAM SPECIFICATIONS
*/
static MW_ERROR_NO_T
_syncd_cmd_getEnable(
    C8_T    *str,
    BOOL_T  *is_enable)
{
    MW_ERROR_NO_T  rc = MW_E_OK;

    if (MW_E_OK == mw_cmd_checkString(str, "on"))
    {
        *is_enable = TRUE;
    }
    else if(MW_E_OK == mw_cmd_checkString(str, "off"))
    {
        *is_enable = FALSE;
    }
    else
    {
        rc = MW_CMD_E_SYNTAX_ERR;
    }

    return rc;
}

static MW_ERROR_NO_T
_syncd_cmd_timer(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    MW_ERROR_NO_T rc = MW_E_OK;
    UI32_T flags = 0;
    C8_T *ptr_str = NULL;
    BOOL_T is_enable = FALSE;

    /* Allocate string buffer */
    rc = osapi_calloc(MW_CMD_CMD_MAX_LENGTH,
                      "cmd_dbg",
                      (void **)&ptr_str);
    if (MW_E_OK != rc)
    {
        MW_CMD_OUTPUT("Fail(%d)\n", rc);
        return rc;
    }

    /*  cmd: syncd timer { all | port | mib }={ on | off }
     */
    if (mw_cmd_getString(tokens, token_idx, "all", ptr_str) == MW_E_OK)
    {
        flags = BIT(SYNCD_TMR_ENABLE_BIT_LAST) - 1;
    }
    else if (mw_cmd_getString(tokens, token_idx, "port", ptr_str) == MW_E_OK)
    {
        flags = SYNCD_TMR_ENABLE_PORT;
    }
    else if (mw_cmd_getString(tokens, token_idx, "mib", ptr_str) == MW_E_OK)
    {
        flags = SYNCD_TMR_ENABLE_MIB;
    }
    else
    {
        rc = MW_CMD_E_SYNTAX_ERR;
    }

    if (MW_E_OK != rc)
    {
        MW_FREE(ptr_str);
        return rc;
    }

    token_idx += 2;
    if (NULL != tokens[token_idx])
    {
        MW_FREE(ptr_str);
        return (MW_CMD_E_SYNTAX_ERR);
    }

    rc = _syncd_cmd_getEnable(ptr_str, &is_enable);
    if (MW_E_OK != rc)
    {
        MW_CMD_OUTPUT("Fail(%d)\n", rc);
        MW_FREE(ptr_str);
        return rc;
    }

    rc = syncd_timer_setFlag(flags, is_enable);
    if (MW_E_OK != rc)
    {
        MW_CMD_OUTPUT("Fail(%d)\n", rc);
    }
    else
    {
        MW_CMD_OUTPUT("Succeed!\r\n");
    }

    MW_FREE(ptr_str);

    return rc;
}

/* STATIC VARIABLE DECLARATIONS
 */
static const MW_CMD_VEC_T _syncd_cmd_vec[] =
{
    {
        "set timer", 2, _syncd_cmd_timer,
        "syncd set timer { all | port | mib }={ on | off }\n"
    },
};

/* EXPORTED SUBPROGRAM BODIES
 */
/* FUNCTION NAME: mw_cmd_syncd_dispatcher
 * PURPOSE:
 *      Function dispatcher for magic wand command: syncd.
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
mw_cmd_syncd_dispatcher(
    const C8_T  *tokens[],
    UI32_T      token_idx)
{
    return (mw_cmd_dispatcher(tokens, token_idx, _syncd_cmd_vec, sizeof(_syncd_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

/* FUNCTION NAME: mw_cmd_syncd_usager
 * PURPOSE:
 *      Command usage for magic wand command: syncd.
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
mw_cmd_syncd_usager(
    void)
{
    return (mw_cmd_usager(_syncd_cmd_vec, sizeof(_syncd_cmd_vec)/sizeof(MW_CMD_VEC_T)));
}

