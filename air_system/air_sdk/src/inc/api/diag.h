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

/* FILE NAME:  diag.h
 * PURPOSE:
 *      1. It provides DIAG (Diagnosis) module internal API
 *      2. It provides debug console and buffer functionalities
 *
 * NOTES:
 *
 */

#ifndef DIAG_H
#define DIAG_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_error.h>
#include <air_init.h>
#include <air_module.h>
#include <air_types.h>
#include <hal/common/hal.h>

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */

extern UI32_T _ext_module_dbg_flag[AIR_MODULE_LAST];

#ifdef AIR_EN_DEBUG

#ifdef AIR_EN_COMPILER_SUPPORT_FUNCTION
#define DIAG_SET_MODULE_INFO(__module_id__, __file_name__) \
    static AIR_MODULE_T __AIR_MODULE__ = (__module_id__)
#define DIAG_PRINT(__flag__, ...)                                          \
    do                                                                     \
    {                                                                      \
        if (0 != ((__flag__) & (_ext_module_dbg_flag[__AIR_MODULE__])))    \
        {                                                                  \
            diag_print(__AIR_MODULE__, __func__, __LINE__, ##__VA_ARGS__); \
        }                                                                  \
    } while (0)
#else /* !AIR_EN_COMPILER_SUPPORT_FUNCTION */
#define DIAG_SET_MODULE_INFO(__module_id__, __file_name__) \
    static AIR_MODULE_T __AIR_MODULE__ = (__module_id__);  \
    static C8_T        *__AIR_FILE__ = (__file_name__)

#define DIAG_PRINT(__flag__, ...)                                              \
    do                                                                         \
    {                                                                          \
        if (0 != ((__flag__) & (_ext_module_dbg_flag[__AIR_MODULE__])))        \
        {                                                                      \
            diag_print(__AIR_MODULE__, __AIR_FILE__, __LINE__, ##__VA_ARGS__); \
        }                                                                      \
    } while (0)

#endif /* #ifdef AIR_EN_COMPILER_SUPPORT_FUNCTION */

#define DIAG_PRINT_RAW(__flag__, ...)                                   \
    do                                                                  \
    {                                                                   \
        if (0 != ((__flag__) & (_ext_module_dbg_flag[__AIR_MODULE__]))) \
        {                                                               \
            diag_print_raw(__AIR_MODULE__, ##__VA_ARGS__);              \
        }                                                               \
    } while (0)

#define DIAG_PRINT_HEX_BUF(__flag__, __ptr_buf__, __buf_size__)          \
    do                                                                   \
    {                                                                    \
        if (0 != ((__flag__) & (_ext_module_dbg_flag[__AIR_MODULE__])))  \
        {                                                                \
            diag_printHexBuf(__AIR_MODULE__, __ptr_buf__, __buf_size__); \
        }                                                                \
    } while (0)

#else /* !AIR_EN_DEBUG */
#define DIAG_SET_MODULE_INFO(__module_id__, __file_name__)
#define DIAG_PRINT(__flag__, ...)
#define DIAG_PRINT_HEX_BUF(__flag__, __ptr_buf__, __buf_size__)
#define DIAG_PRINT_HEX_TBL(__flag__, __ptr_buf__, __buf_word_size__)
#define DIAG_PRINT_RAW(...)
#define DIAG_PRINT_MULTI_FEILDS_INFO(...)
#define DIAG_PRINT_SINGLE_FEILD_INFO(...)
#define DIAG_PRINT_TBL_INFO(...)

#endif /* #ifdef AIR_EN_DEBUG */

#define DIAG_PRINT_BUF_SIZE    (128)
#define DIAG_LOG_ENTRY_MAX_NUM (1 * 1024)

/* DATA TYPE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME:   diag_init
 * PURPOSE:
 *      This function is used to initilize the diag module
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- operate success
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
diag_init(
    void);

/* FUNCTION NAME:   diag_deinit
 * PURPOSE:
 *      This function is to used de-initilize the diag module
 * INPUT:
 *      None
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- operate success
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
diag_deinit(
    void);

/* FUNCTION NAME:   diag_setDebugFlag
 * PURPOSE:
 *      This function is used to enable/disable module's debug message recording
 * INPUT:
 *      module_id           -- selected module identifier
 *      dbg_flag            -- filter of selected module
 *      enabled             -- enable or disable debug flag
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- operate success
 *      AIR_E_BAD_PARAMETER -- bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
diag_setDebugFlag(
    const AIR_MODULE_T module_id,
    const UI32_T       dbg_flag,
    const BOOL_T       enabled);

/* FUNCTION NAME:   diag_getDebugFlag
 * PURPOSE:
 *      This function is used to get current debug message recording status
 * INPUT:
 *      module_id           -- selected module identifier
 *      ptr_dbg_flag        -- pointer to get the debug flag stauts
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- operate success
 *      AIR_E_BAD_PARAMETER -- bad parameter
 * NOTES:
 *      None
 */
AIR_ERROR_NO_T
diag_getDebugFlag(
    const AIR_MODULE_T module_id,
    UI32_T            *ptr_dbg_flag);

/* FUNCTION NAME:   diag_print_raw
 * PURPOSE:
 *      This function is used to print
 * INPUT:
 *      module_id           -- selected module identifier
 *      ptr_fmt             -- the format specifiers
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
diag_print_raw(
    const AIR_MODULE_T module_id,
    const C8_T        *ptr_fmt,
    ...);

/* FUNCTION NAME:   diag_print
 * PURPOSE:
 *      This function is used to output debug message to console or/and save to DIAG buffer
 * INPUT:
 *      module_id           -- selected module identifier
 *      ptr_func            -- function or file name string
 *      line                -- line number
 *      ptr_fmt             -- input string pointer
 * OUTPUT:
 *      None
 * RETURN:
 *      None
 * NOTES:
 *      None
 */
void
diag_print(
    const AIR_MODULE_T module_id,
    const C8_T        *ptr_func,
    const UI32_T       line,
    const C8_T        *ptr_fmt,
    ...);
#endif /* End of DIAG_H */
