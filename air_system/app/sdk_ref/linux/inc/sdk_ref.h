/*******************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of Mediatek Inc. (C) 2013-2020
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*******************************************************************************/

#ifndef SDK_REF_H
#define SDK_REF_H

/* INCLUDE FILE DECLARATIONS
*/
#include <air_types.h>
#include <air_error.h>

/* NAMING CONSTANT DECLARATIONS
*/
#define SDK_REF_PARAMETER_LEN       (50)
#define SDK_REF_FILE_NAME_LEN       (128)

#if defined (AIR_LINUX_KERNEL_MODE)
#define SDK_REF_KNL_DRIVER_NAME     "sdk_ref"
#define SDK_REF_KNL_FILE_PATH       "/dev/"SDK_REF_KNL_DRIVER_NAME
#define SDK_REF_KNL_MAJOR_NUM       (10)
#define SDK_REF_KNL_MINOR_NUM       (251)
#define SDK_REF_KNL_WBUF_LEN        (0x4000)   /* write_buf size must be power of 2 for masking */
#define SDK_REF_KNL_WBUF_MASK       (SDK_REF_KNL_WBUF_LEN - 1)

typedef enum
{
    SDK_REF_IOCTL_TYPE_DSH_INIT = 0,
    SDK_REF_IOCTL_TYPE_DSH_WRITE,
    SDK_REF_IOCTL_TYPE_DSH_READ,
    SDK_REF_IOCTL_TYPE_WARM_BOOT,
    SDK_REF_IOCTL_TYPE_LOAD_SCRIPT,
    SDK_REF_IOCTL_TYPE_SDK_INIT,
    SDK_REF_IOCTL_TYPE_SDK_DEINIT,
    SDK_REF_IOCTL_TYPE_LAST
} SDK_REF_IOCTL_TYPE_T;

typedef union
{
    UI32_T      value;
    struct
    {
        UI32_T  type        :  4;      /* Maximum 16 IOCTL types */
        UI32_T  len         : 14;      /* Maximum len is 16383  */
        UI32_T  rsvd        : 14;
    } field;
} SDK_REF_IOCTL_CMD_T;
#endif/* end of AIR_LINUX_KERNEL_MODE */

typedef enum
{
    SDK_REF_FILE_TYPE_READ = 0,     /* r  */
    SDK_REF_FILE_TYPE_WRITE,        /* w  */
    SDK_REF_FILE_TYPE_APPEND,       /* a  */
    SDK_REF_FILE_TYPE_RW_READ,      /* r+ */
    SDK_REF_FILE_TYPE_RW_WRITE,     /* w+ */
    SDK_REF_FILE_TYPE_RW_APPEND,    /* a+ */
    SDK_REF_FILE_TYPE_LAST
} SDK_REF_FILE_TYPE_T;

typedef struct
{
    C8_T    *ptr_run_mode;      /* r: run_mode */
    BOOL_T  run_mode_valid;
    C8_T    *ptr_cfg_name;      /* c: config */
    BOOL_T  cfg_name_valid;
    C8_T    *ptr_script_name;   /* s: script */
    BOOL_T  script_name_valid;
    UI32_T  family_id;          /* f: family_id */
    BOOL_T  family_id_valid;
    UI32_T  revision_id;
    BOOL_T  revision_id_valid;
} SDK_REF_INIT_PARAM_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* -------------------------------------------------------------- Porting function */
void
sdk_ref_printf(
    const void      *ptr_buf,
    UI32_T          len);

void *
sdk_ref_openFile(
    const char                 *ptr_filename,
    const SDK_REF_FILE_TYPE_T  file_type);

void
sdk_ref_closeFile(
    void            *ptr_file);

int
sdk_ref_writeFile(
    void            *ptr_file,
    const char      *ptr_buf,
    int             count);

int
sdk_ref_readFile(
    void            *ptr_file,
    char            *ptr_buf,
    int             count);

/* -------------------------------------------------------------- APP function */
AIR_ERROR_NO_T
sdk_ref_initParser(void);

AIR_ERROR_NO_T
sdk_ref_deinitParser(void);

AIR_ERROR_NO_T
sdk_ref_getInput(
    char            *ptr_buf);

/* -------------------------------------------------------------- SDK init function */
AIR_ERROR_NO_T
sdk_ref_loadScript(
    const C8_T              *ptr_script_name);

AIR_ERROR_NO_T
sdk_ref_initCfg(
    const C8_T              *ptr_cfg_name);

AIR_ERROR_NO_T
sdk_ref_initLedCfg(
    const C8_T              *ptr_led_cfg_name);

AIR_ERROR_NO_T
sdk_ref_init(
    SDK_REF_INIT_PARAM_T    *ptr_init_param);

#endif
