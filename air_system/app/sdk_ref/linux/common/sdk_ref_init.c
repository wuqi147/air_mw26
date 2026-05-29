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

#include <air_error.h>
#include <air_types.h>
#include <air_init.h>
#include <aml/aml.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>
#include <hal/common/hal.h>

#include <sdk_ref.h>

extern UI32_T _ext_aml_run_mode;
extern UI32_T _ext_aml_forced_family_id;
extern UI32_T _ext_aml_forced_revision_id;

AIR_ERROR_NO_T
sdk_ref_chkRunMode(
    const C8_T      *run_mode)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;

    if (NULL != run_mode)
    {
        if (0 == osal_strncmp(run_mode, "chip", osal_strlen("chip")))
        {
            _ext_aml_run_mode = HAL_RUN_CHIP_MODE;
        }
        else if (0 == osal_strncmp(run_mode, "fpga", osal_strlen("fpga")))
        {
            _ext_aml_run_mode = HAL_RUN_FPGA_MODE;
        }
        else
        {
            osal_printf("Invalid run mode: %s\n", run_mode);
            rc = AIR_E_OTHERS;
        }
    }

    return (rc);
}

/* -------------------------------------------------------------- SDK init callback */
void
sdk_ref_write(
    const void      *ptr_buf,
    UI32_T          len)
{
    sdk_ref_printf(ptr_buf, len);
}
/* -------------------------------------------------------------- SDK init function */
AIR_ERROR_NO_T
sdk_ref_initSdk(
    void)
{
    AIR_INIT_PARAM_T    init_cmn_param;
    AIR_ERROR_NO_T      rc = AIR_E_OK;

    init_cmn_param.dsh_write_func   = (AIR_INIT_WRITE_FUNC_T) sdk_ref_write;
    init_cmn_param.debug_write_func = (AIR_INIT_WRITE_FUNC_T) sdk_ref_write;

    rc = air_init_initCmnModule(&init_cmn_param);
    if (AIR_E_OK != rc)
    {
        osal_printf("air_init fail rc:%d\n", rc);
    }

    return (rc);
}

AIR_ERROR_NO_T
sdk_ref_init(
    SDK_REF_INIT_PARAM_T   *ptr_init_param)
{
    AIR_ERROR_NO_T  rc = AIR_E_OK;

    /* r: run_mode */
    if ((TRUE == ptr_init_param->run_mode_valid) && (AIR_E_OK == rc))
    {
        rc = sdk_ref_chkRunMode(ptr_init_param->ptr_run_mode);
    }

    /* c: config */
    if ((TRUE == ptr_init_param->cfg_name_valid) && (AIR_E_OK == rc))
    {
        rc = sdk_ref_initCfg(ptr_init_param->ptr_cfg_name);
    }

    /* f: family_id */
    if (TRUE == ptr_init_param->family_id_valid)
    {
        _ext_aml_forced_family_id = ptr_init_param->family_id;
    }

    if (TRUE == ptr_init_param->revision_id_valid)
    {
        _ext_aml_forced_revision_id = ptr_init_param->revision_id;
    }

    if (AIR_E_OK == rc)
    {
        rc = sdk_ref_initSdk();
    }

    /* s: script */
    if ((TRUE == ptr_init_param->script_name_valid) && (AIR_E_OK == rc))
    {
        rc = sdk_ref_loadScript(ptr_init_param->ptr_script_name);
    }

    if (AIR_E_OK != rc)
    {
        osal_printf("sdk_ref init FAIL\n");
    }

    return  (rc);
}
