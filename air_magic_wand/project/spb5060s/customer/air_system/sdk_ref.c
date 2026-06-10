/*******************************************************************************
*  Copyright Statement:
*
*  (C) 2025 Airoha Technology Corp. All rights reserved.
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

#include <air_init.h>
#include <customer_ref.h>
#include <osal/osali.h>
#include <osal/osal_lib.h>
#include <stdio.h>
#include <string.h>
#include <sdk_ref.h>
#ifdef AIR_EN_SFP_LED
#include <sfp_led.h>
#endif
#ifdef AIR_EN_I2C_BITBANG
#include <i2c_bitbang.h>
#endif
#include "mw_log.h"

#ifdef AIR_EN_POE
AIR_ERROR_NO_T
sdk_ref_initPoeMap(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

    rc = customer_ref_initPoeDeviceMap(unit);
    if (AIR_E_OK != rc)
    {
        return rc;
    }

    rc = customer_ref_initPoePortMap(unit);
    return rc;
}
#endif

AIR_ERROR_NO_T
sdk_ref_initApp(
    const UI32_T unit)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;

#ifdef AIR_EN_SFP_LED
    rc = customer_ref_initSfpLed(unit);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize SFP LED!\n");
    }
#endif

#ifdef AIR_EN_I2C_BITBANG
    rc = customer_ref_initAppI2cBitbang(unit);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize I2C Bitbang!\n");
    }
#endif

    return rc;
}

void
sdk_ref_write(
    const void *ptr_buf,
    UI32_T len)
{
    C8_T buf[OSAL_PRN_BUF_SZ];

    /* length not include '\0', it need add 1*/
    snprintf(buf, len + 1, "%s", (const char *)ptr_buf);
    MW_LOG_CONSOLE_PRINTF("%s", buf);
}

AIR_ERROR_NO_T sdk_ref_init(void)
{
    AIR_ERROR_NO_T rc = AIR_E_OK;
    AIR_INIT_PARAM_T init_cmn;

    MW_LOG_CONSOLE_PRINTF("Initializing common modules...\n");
    memset(&init_cmn, 0, sizeof(init_cmn));
    init_cmn.dsh_write_func = sdk_ref_write;
    init_cmn.debug_write_func = sdk_ref_write;
    rc = air_init_initCmnModule(&init_cmn);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize common modules!\n");
        return rc;
    }

    /* register customer configuration callback for customized SDK setting */
    rc = customer_ref_registerConfig(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to register customer configuration callback!\n");
        return rc;
    }

    /*air init*/
    MW_LOG_CONSOLE_PRINTF("Initializing low level modules...\n");
    rc = air_init_initLowLevel(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize low level modules!\n");
        return rc;
    }

    /* initialize port mapping */
    rc = customer_ref_initPortMap(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize SDK port mapping!\n");
        return rc;
    }

    MW_LOG_CONSOLE_PRINTF("Initializing sdk task resources...\n");
#ifdef AIR_EN_POE
    /* initialize poe mapping */
    rc = sdk_ref_initPoeMap(0);
    if (AIR_E_NOT_SUPPORT == rc)
    {
        /* Ignore no Poe devices and port config error */
        rc = AIR_E_OK;
    }
    else if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize poe port & device mapping!\n");
        return rc;
    }
#endif

    rc = air_init_initTaskRsrc(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize SDK task resources!\n");
        return rc;
    }

    /* SDK module initialization */
    MW_LOG_CONSOLE_PRINTF("Initializing sdk modules...\n");
    rc = air_init_initModule(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize SDK modules!\n");
        return rc;
    }

    MW_LOG_CONSOLE_PRINTF("Initializing sdk tasks...\n");
    rc = air_init_initTask(0);
    if (AIR_E_OK != rc)
    {
        MW_LOG_CONSOLE_PRINTF("Error: Failed to initialize SDK tasks!\n");
        return rc;
    }

    MW_LOG_CONSOLE_PRINTF("Initializing sdk apps...\n");
    rc = sdk_ref_initApp(0);

    return rc;
}
