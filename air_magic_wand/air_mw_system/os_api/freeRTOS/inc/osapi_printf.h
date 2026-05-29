
/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2026 Airoha Technology Corp. All rights reserved.
*
*  This software/firmware and related documentation ("Airoha Software") are
*  protected under relevant copyright laws. The information contained herein is
*  confidential and proprietary to Airoha Technology Corp. ("Airoha") and/or
*  its licensors. Without the prior written permission of Airoha and/or its
*  licensors, any reproduction, modification, use or disclosure of Airoha
*  Software, and information contained herein, in whole or in part, shall be
*  strictly prohibited. You may only use, reproduce, modify, or distribute (as
*  applicable) Airoha Software if you have agreed to and been bound by the
*  applicable license agreement with Airoha ("License Agreement") and been
*  granted explicit permission to do so within the License Agreement
*  ("Permitted User"). If you are not a Permitted User, please cease any access
*  or use of Airoha Software immediately.
*
*  BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("AIROHA SOFTWARE")
*  RECEIVED FROM AIROHA AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
*  AN "AS-IS" BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES AIROHA PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH AIROHA SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
*  ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
*  THIRD PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE. AIROHA SHALL
*  ALSO NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES MADE TO RECEIVER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*  RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND AIROHA'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO AIROHA SOFTWARE RELEASED HEREUNDER WILL BE, AT
*  AIROHA'S OPTION, TO REVISE OR REPLACE AIROHA SOFTWARE AT ISSUE, OR REFUND
*  ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO AIROHA FOR
*  SUCH AIROHA SOFTWARE AT ISSUE.
*
*  The following software/firmware and/or related documentation ("Airoha
*  Software") have been modified by Airoha Corp. All revisions are subject to
*  any receiver's applicable license agreements with Airoha Corp.
*******************************************************************************/

/* FILE NAME:  osapi_printf.h
 * PURPOSE:
 *      Wrapper APIs for printf.
 *
 * NOTES:
 */

#ifndef OSAPI_PRINTF_H
#define OSAPI_PRINTF_H

/* INCLUDE FILE DECLARATIONS
 */
#include <stdarg.h>

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: xvprintf
 * PURPOSE:
 *      Format and print data to stdout and debug log (using va_list)
 *
 * INPUT:
 *      fmt             --  Format string
 *      args            --  Variable argument list
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      int             --  Number of characters printed
 *
 * NOTES:
 *      None
 */
int xvprintf(
    const char *fmt,
    va_list args);

/* FUNCTION NAME: xprintf
 * PURPOSE:
 *      Format and print data to stdout and debug log
 *
 * INPUT:
 *      fmt             --  Format string
 *      ...             --  Arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      int             --  Number of characters printed
 *
 * NOTES:
 *      None
 */
int xprintf(
    const char *fmt, ...);

/* FUNCTION NAME: uart_vprintf
 * PURPOSE:
 *      Format and print data to stdout only (skip remote debug log)
 *
 * INPUT:
 *      fmt             --  Format string
 *      args            --  Variable argument list
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      int             --  Number of characters printed
 *
 * NOTES:
 *      None
 */
int uart_vprintf(
    const char *fmt,
    va_list args);

/* FUNCTION NAME: uart_printf
 * PURPOSE:
 *      Format and print data to stdout only (skip remote debug log)
 *
 * INPUT:
 *      fmt             --  Format string
 *      ...             --  Arguments
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      int             --  Number of characters printed
 *
 * NOTES:
 *      None
 */
int uart_printf(
    const char *fmt, ...);

/* FUNCTION NAME: osapi_write_to_ram
 * PURPOSE:
 *      Write character to remote debug log if session is active
 *
 * INPUT:
 *      c            --  Character to write
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      None
 *
 * NOTES:
 *      None
 */
void osapi_write_to_ram(
    unsigned c);

#endif  /* #ifndef OSAPI_PRINTF_H */
