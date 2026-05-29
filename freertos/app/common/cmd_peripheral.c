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
#include <dsh_parser.h>
#include <dsh_util.h>
#include <string.h>
#include "gpio_cmd.h"
#include "i2c_cmd.h"
#include "chipscu_cmd.h"
#include <pp_def.h>

/* -------------------------------------------------------------- Macros */
#define DSH_IS_CONTORL_CH(__ch__)                                \
    ((__ch__) <= 0x1F)? TRUE :                                   \
    ((__ch__) == 0x7F)? TRUE : FALSE

#define DSH_PRINT_USAGE(__ptr_usage__)                           \
{                                                                \
    unsigned int    __len__ = strlen((__ptr_usage__));           \
    unsigned int    __print_idx__ = 0;                           \
                                                                 \
    while (__print_idx__ < __len__)                              \
    {                                                            \
        printf("%s", (__ptr_usage__) + __print_idx__);           \
        __print_idx__ += DSH_CMD_MAX_OUTPUT - 1;                 \
    }                                                            \
}                                                                \

/* -------------------------------------------------------------- Top */
const DSH_VEC_TOP_T   _dsh_peripheral_vec[] =
{
/*     key        description               callback function   show usage function    */
#ifdef CONFIG_DRIVER_I2C
    { "i2c",      "I2C",                  i2c_cmd_dispatcher,     i2c_cmd_usager      },
#endif
    { "gpio",     "GPIO",                 gpio_cmd_dispatcher,    gpio_cmd_usager     },
    { "chipscu",  "ChipScu",              chipscu_cmd_dispatcher, chipscu_cmd_usager  },
};

const unsigned int _dsh_peripheral_vec_num = sizeof(_dsh_peripheral_vec) / sizeof(DSH_VEC_TOP_T);

/* -------------------------------------------------------------- exported to modules */

/* -------------------------------------------------------------- exported API */
int
dsh_peripheral_handleString(
    char                *ptr_str)
{
    int                 rc = E_OK;
    unsigned int        idx = 0;
    char                *tokens[DSH_CMD_MAX_TOKENS] = {0};

    if (FALSE == DSH_IS_CONTORL_CH(ptr_str[0]))
    {
        memset(tokens, 0, sizeof(char *) * DSH_CMD_MAX_TOKENS);
        rc = dsh_splitString(tokens, ptr_str);
        if (E_OK != rc)
        {
            printf("***Error***, Input too many tokens.\n");
            return rc;
        }

        /* check empty string */
        if (NULL == tokens[0])
        {
            /* show air command list when command is : air */
            return (DSH_E_KEY_MISMATCH);
        }

        /* check the key */
        for (idx = 0; idx < _dsh_peripheral_vec_num; idx++)
        {
            if (E_OK == dsh_checkString(tokens[0], _dsh_peripheral_vec[idx].ptr_key))
            {
                /* callback with token_idx 1 */
                rc = _dsh_peripheral_vec[idx].callback((const char **)tokens, 1);
                if (DSH_E_NOT_FOUND == rc)
                {
                    _dsh_peripheral_vec[idx].usage();
                }
                return (rc);
            }
        }

        /* miss all of the keys */
        return (DSH_E_KEY_MISMATCH);
    }

    return (E_OK);
}

void
dsh_peripheral_parseString(
    char                *ptr_str)
{
    int                 rc = E_OK;
    unsigned int        idx = 0, num = 1;

    if (FALSE == DSH_IS_CONTORL_CH(ptr_str[0]))
    {
        rc = dsh_peripheral_handleString(ptr_str);
        if (DSH_E_KEY_MISMATCH == rc)
        {
            printf("***Unrecognized command***\n");
            printf("air support command : \n");
            for (idx = 0; idx < _dsh_peripheral_vec_num; idx++)
            {
                printf("%2d. ", num++);
                printf("%s \n", _dsh_peripheral_vec[idx].ptr_key);
                printf("\n");
            }
        }
    }
}

