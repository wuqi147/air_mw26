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

#ifndef DSH_PARSER_H
#define DSH_PARSER_H

#include <air_error.h>
#include <air_types.h>

/* parameters */
#define DSH_CMD_MAX_INPUT  (1536) /* maximum size of UI8_T string */
#define DSH_CMD_MAX_OUTPUT (256)  /* maximum size of UI8_T string */

#define DSH_CMD_MAX_TOKENS (128)  /* maximum number of tokens     */
#define DSH_CMD_MAX_LENGTH (128)  /* maximum length of tokens     */

#define DSH_CMD_MAX_WORDS (256)   /* maximum size of UI32_T array */
#define DSH_CMD_MAX_BYTES (256)   /* maximum size of UI8_T array  */

/* parser */
#define DSH_E_BASE         (10000) /* magic number */
#define DSH_E_KEY_MISMATCH (DSH_E_BASE + 1)
#define DSH_E_NOT_FOUND    (DSH_E_BASE + 2)
#define DSH_E_SYNTAX_ERR   (DSH_E_BASE + 3)

typedef AIR_ERROR_NO_T (*DSH_CALLBACK_T)(
    const C8_T *ptr_tokens[],
    UI32_T      token_idx);

typedef AIR_ERROR_NO_T (*DSH_USAGE_T)(void);

typedef struct
{
    C8_T          *ptr_key;   /* key */
    UI32_T         token_num; /* token numbers */
    DSH_CALLBACK_T callback;  /* callback function */
    C8_T          *ptr_usage; /* usage */
} DSH_VEC_T;

typedef struct
{
    C8_T          *ptr_key;  /* key */
    C8_T          *ptr_desc; /* description */
    DSH_CALLBACK_T callback; /* callback function */
    DSH_USAGE_T    usage;    /* show usage function */
} DSH_VEC_TOP_T;

/* -------------------------------------------------------------- exported to modules */
AIR_ERROR_NO_T
dsh_dispatcher(
    const C8_T     *ptr_tokens[],
    UI32_T          token_idx,
    const DSH_VEC_T vec[],
    const UI32_T    vec_num);

AIR_ERROR_NO_T
dsh_usager(
    const DSH_VEC_T vec[],
    const UI32_T    vec_num);

/* -------------------------------------------------------------- exported API */
AIR_ERROR_NO_T
dsh_handleString(
    C8_T *ptr_str);

void
dsh_parseString(
    C8_T *ptr_str);

void
dsh_init(void);

#endif /* end of DSH_PARSER_H */
