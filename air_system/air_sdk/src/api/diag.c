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

/* FILE NAME:  diag.c
 * PURPOSE:
 *      1. It provides DIAG (Diagnosis) module internal API.
 *      2. It provides debug console and buffer functionalities.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <api/diag.h>

#include <air_error.h>
#include <air_init.h>
#include <air_module.h>
#include <air_types.h>
#include <hal/common/hal.h>
#include <hal/common/hal_dbg.h>
#include <hal/common/hal_drv.h>
#include <osal/osal.h>

/* #define DIAG_PRINT_MODULE_NAME */

/* NAMING CONSTANT DECLARATIONS
 */

/* MACRO FUNCTION DECLARATIONS
 */
#define DIAG_LOG_LOCK()   osal_takeSemaphore(&(_diag_log_cb.sema), AIR_SEMAPHORE_WAIT_FOREVER)
#define DIAG_LOG_UNLOCK() osal_giveSemaphore(&(_diag_log_cb.sema))
#define DIAG_LOG_INIT_CHECK()       \
    do                              \
    {                               \
        if (!(_diag_log_cb.inited)) \
        {                           \
            return;                 \
        }                           \
    } while (0)

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

AIR_INIT_WRITE_FUNC_T _ext_debug_write_func = NULL;

/* LOCAL SUBPROGRAM DECLARATIONS
 */

/* STATIC VARIABLE DECLARATIONS
 */
/* DIAG_SET_MODULE_INFO(AIR_MODULE_DIAG, "diag.c"); */

/* the strucnt below reference to air_module.h */
UI32_T                _ext_module_dbg_flag[AIR_MODULE_LAST] = {
    HAL_DBG_ERR, /* AIR_MODULE_VLAN,              */
    HAL_DBG_ERR, /* AIR_MODULE_STP,               */
    HAL_DBG_ERR, /* AIR_MODULE_MIR,               */
    HAL_DBG_ERR, /* AIR_MODULE_PORT,              */
    HAL_DBG_ERR, /* AIR_MODULE_DIAG,              */
    HAL_DBG_ERR, /* AIR_MODULE_OSAL,              */
    HAL_DBG_ERR, /* AIR_MODULE_AML,               */
    HAL_DBG_ERR, /* AIR_MODULE_HAL,               */
    HAL_DBG_ERR, /* AIR_MODULE_CHIP,              */
    HAL_DBG_ERR, /* AIR_MODULE_LAG,               */
    HAL_DBG_ERR, /* AIR_MODULE_INIT,              */
    HAL_DBG_ERR, /* AIR_MODULE_IPMC,              */
    HAL_DBG_ERR, /* AIR_MODULE_L2,                */
    HAL_DBG_ERR, /* AIR_MODULE_LED,               */
    HAL_DBG_ERR, /* AIR_MODULE_MIB,               */
    HAL_DBG_ERR, /* AIR_MODULE_QOS,               */
    HAL_DBG_ERR, /* AIR_MODULE_SWC,               */
    HAL_DBG_ERR, /* AIR_MODULE_SEC,               */
    HAL_DBG_ERR, /* AIR_MODULE_SFLOW,             */
    HAL_DBG_ERR, /* AIR_MODULE_SVLAN,             */
    HAL_DBG_ERR, /* AIR_MODULE_STAG,              */
    HAL_DBG_ERR, /* AIR_MODULE_DOS,               */
    HAL_DBG_ERR, /* AIR_MODULE_ACL,               */
    HAL_DBG_ERR, /* AIR_MODULE_IFMON              */
    HAL_DBG_ERR, /* AIR_MODULE_MDIO,              */
    HAL_DBG_ERR, /* AIR_MODULE_PHY,               */
    HAL_DBG_ERR, /* AIR_MODULE_SIF,               */
    HAL_DBG_ERR, /* AIR_MODULE_PERIF,             */
    HAL_DBG_ERR, /* AIR_MODULE_POE,               */
};

/* LOCAL SUBPROGRAM SPECIFICATIONS
 */

/* LOCAL SUBPROGRAM BODIES
 */
static AIR_ERROR_NO_T
_diag_initRsrc(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}

static AIR_ERROR_NO_T
_diag_deinitRsrc(
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    return rc;
}

/* FUNCTION NAME:   _diag_printBuffer
 * PURPOSE:
 *      This function is used determine whether to print the message to buffer or console
 *      Based on the user-configured buffer mode, this function decides the print length
 *      and invoke other API to perform memory copy
 * INPUT:
 *      ptr_buf             -- the source content to print
 *      len                 -- the length of data to be printed
 * OUTPUT:
 *      None
 * RETURN:
 *      AIR_E_OK            -- operate success
 * NOTES:
 *      None
 */
static AIR_ERROR_NO_T
_diag_printBuffer(
    C8_T        *ptr_buf,
    const UI32_T len)
{
    if (NULL != _ext_debug_write_func)
    {
        _ext_debug_write_func(ptr_buf, len);
    }

    return (AIR_E_OK);
}

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
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _diag_initRsrc();

    return rc;
}

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
    void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = _diag_deinitRsrc();

    return rc;
}

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
    const BOOL_T       enabled)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    HAL_CHECK_ENUM_RANGE(module_id, AIR_MODULE_LAST);

    if (TRUE == enabled)
    {
        _ext_module_dbg_flag[module_id] |= dbg_flag;
    }
    else
    {
        _ext_module_dbg_flag[module_id] &= ~dbg_flag;
    }

    return (rc);
}

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
    UI32_T            *ptr_dbg_flag)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    if (AIR_MODULE_LAST > module_id)
    {
        if (NULL == ptr_dbg_flag)
        {
            rc = AIR_E_BAD_PARAMETER;
        }
        else
        {
            *ptr_dbg_flag = _ext_module_dbg_flag[module_id];
        }
    }
    else
    {
        rc = AIR_E_BAD_PARAMETER;
    }

    return (rc);
}

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
    ...)
{
    OSAL_VA_LIST ap;
    UI32_T       str_len = 0;
    C8_T         str_buf[DIAG_PRINT_BUF_SIZE];
    UI32_T       avbl_buf_len = DIAG_PRINT_BUF_SIZE;

    OSAL_VA_START(ap, ptr_fmt);
    str_len = osal_vsnprintf(str_buf, avbl_buf_len, ptr_fmt, ap);
    OSAL_VA_END(ap);

    /* Since osal_vsnprintf always return the exact size of string (not the string size write to buffer),
     * we must check if the user string length exceeds the remaing buffer size to konw the exact size write to buffer.
     */
    if (str_len >= (avbl_buf_len - 1))
    {
        str_len = avbl_buf_len;
    }
    _diag_printBuffer(str_buf, str_len);
}

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
    ...)
{
    OSAL_VA_LIST ap;
    UI32_T       str_len = 0;
    C8_T         str_buf[DIAG_PRINT_BUF_SIZE];
    UI32_T       prefix_len = 0;
    UI32_T       avbl_buf_len = DIAG_PRINT_BUF_SIZE;

#if defined(DIAG_PRINT_MODULE_NAME)
    prefix_len =
        osal_snprintf(str_buf, avbl_buf_len, "%s:<%s:%d>", air_module_getModuleName(module_id), ptr_func, line);
#else
    prefix_len = osal_snprintf(str_buf, avbl_buf_len, "<%s:%d>", ptr_func, line);
#endif

    if (prefix_len >= (avbl_buf_len - 1))
    {
        /* abnormal case, only have prefix msg
         * avoid buffer overflow, just printf out prefix msg + null terminator
         * and won't append debug msg.
         */
        _diag_printBuffer(str_buf, avbl_buf_len);
    }
    else
    {
        /* normal case, append debug msg
         * calculate available buffer size
         */
        avbl_buf_len -= prefix_len;

        OSAL_VA_START(ap, ptr_fmt);
        str_len = osal_vsnprintf(str_buf + prefix_len, avbl_buf_len, ptr_fmt, ap);
        OSAL_VA_END(ap);

        /* Since osal_vsnprintf always return the exact size of string (not the string size write to buffer),
         * we must check if the user string length exceeds the remaing buffer size to konw the exact size write
         * to buffer.
         */
        if (str_len >= (avbl_buf_len - 1))
        {
            str_len = avbl_buf_len;
        }

        _diag_printBuffer(str_buf, prefix_len + str_len);
    }
}
