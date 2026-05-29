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

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <pthread.h>

#include <air_error.h>
#include <air_types.h>
#include <air_init.h>
#include <air_cfg.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>

#include <sdk_ref.h>
#include <parser/dsh_parser.h>

typedef struct
{
    FILE                    *ptr_file;
} SDK_REF_FILE_T;

static char                 _sdk_ref_input_string[DSH_CMD_MAX_INPUT];
static pthread_mutex_t      _sdk_ref_write_lock = PTHREAD_MUTEX_INITIALIZER;

static AIR_ERROR_NO_T
_sdk_ref_chkFamilyId(
    const char              *ptr_family_id,
    SDK_REF_INIT_PARAM_T    *ptr_init_param)
{
    char                    param[SDK_REF_PARAMETER_LEN];
    char                    *ptr_ch = NULL;
    char                    *ptr_r = NULL;
    UI32_T                  len = 0;

    osal_memset(param, 0x0, SDK_REF_PARAMETER_LEN);

    if (NULL != ptr_family_id)
    {
        len = osal_strlen(ptr_family_id);
        osal_strncpy(param, ptr_family_id, len);
        param[len] = '\0';

        ptr_r = (char *)param;
        ptr_ch = strsep(&ptr_r, ":");

        if ((NULL == strchr(ptr_ch, 'E')) && (NULL == strchr(ptr_ch, 'e')))
        {
            ptr_init_param->family_id = strtol(ptr_ch, NULL, 0);
            ptr_init_param->family_id_valid = TRUE;
        }
        else
        {
            ptr_init_param->revision_id = strtol(ptr_ch + 1, NULL, 0);
            ptr_init_param->revision_id_valid = TRUE;
        }

        ptr_ch = strsep(&ptr_r, ":");
        if (NULL != ptr_ch)
        {
            if ((NULL == strchr(ptr_ch, 'P')) && (NULL == strchr(ptr_ch, 'p')))
            {
                ptr_init_param->revision_id = strtol(ptr_ch + 1, NULL, 0);
                ptr_init_param->revision_id_valid = TRUE;
            }
            else
            {
                ptr_init_param->revision_id = AIR_INVALID_ID;
                ptr_init_param->revision_id_valid = TRUE;
            }
        }
    }

    return (AIR_E_OK);
}

void
sdk_ref_printf(
    const void      *ptr_buf,
    UI32_T          len)
{
    pthread_mutex_lock(&_sdk_ref_write_lock);
    printf("%.*s", len, (char *)ptr_buf);
    pthread_mutex_unlock(&_sdk_ref_write_lock);
}

void *
sdk_ref_openFile(
    const char                 *ptr_filename,
    const SDK_REF_FILE_TYPE_T  file_type)
{
    SDK_REF_FILE_T  *ptr_cb = NULL;

    ptr_cb = (SDK_REF_FILE_T *)osal_alloc(sizeof(SDK_REF_FILE_T),"sdk_ref");
    if (NULL == ptr_cb)
    {
        return NULL;
    }

    switch (file_type)
    {
        case SDK_REF_FILE_TYPE_READ:
            ptr_cb->ptr_file = fopen(ptr_filename, "r");
            break;
        case SDK_REF_FILE_TYPE_WRITE:
            ptr_cb->ptr_file = fopen(ptr_filename, "w");
            break;
        case SDK_REF_FILE_TYPE_APPEND:
            ptr_cb->ptr_file = fopen(ptr_filename, "a");
            break;
        case SDK_REF_FILE_TYPE_RW_READ:
            ptr_cb->ptr_file = fopen(ptr_filename, "r+");
            break;
        case SDK_REF_FILE_TYPE_RW_WRITE:
            ptr_cb->ptr_file = fopen(ptr_filename, "w+");
            break;
        case SDK_REF_FILE_TYPE_RW_APPEND:
            ptr_cb->ptr_file = fopen(ptr_filename, "a+");
            break;
        default:
            ptr_cb->ptr_file = NULL;
            break;
    }

    if (NULL == ptr_cb->ptr_file)
    {
        osal_free(ptr_cb);
        return NULL;
    }

    return (void *)ptr_cb;
}

void
sdk_ref_closeFile(
    void            *ptr_file)
{
    SDK_REF_FILE_T  *ptr_cb = (SDK_REF_FILE_T *)ptr_file;

    if (NULL == ptr_cb)
    {
        return ;
    }

    fclose(ptr_cb->ptr_file);
    osal_free(ptr_cb);
}

int
sdk_ref_writeFile(
    void            *ptr_file,
    const char      *ptr_buf,
    int             count)
{
    SDK_REF_FILE_T  *ptr_cb = (SDK_REF_FILE_T *)ptr_file;

    if (NULL == ptr_file || NULL == ptr_buf || 0 >= count)
    {
        return 0;
    }

    return fwrite(ptr_buf, sizeof(I8_T), count, ptr_cb->ptr_file);
}

int
sdk_ref_readFile(
    void            *ptr_file,
    char            *ptr_buf,
    int             count)
{
    SDK_REF_FILE_T  *ptr_cb = (SDK_REF_FILE_T *)ptr_file;

    if (NULL == ptr_file || NULL == ptr_buf || 0 >= count)
    {
        return 0;
    }

    return fread(ptr_buf, 1, count, ptr_cb->ptr_file);
}

int
main(
    int             argc,
    char            *argv[])
{
    char            config_name[SDK_REF_FILE_NAME_LEN];
    char            script_name[SDK_REF_FILE_NAME_LEN];
    char            run_mode[SDK_REF_PARAMETER_LEN];
    char            *ptr_input = (char *)_sdk_ref_input_string;
    int             input_len = 0;

    SDK_REF_INIT_PARAM_T init_param;

    osal_memset(config_name, 0x0, SDK_REF_FILE_NAME_LEN);
    osal_memset(script_name, 0x0, SDK_REF_FILE_NAME_LEN);
    osal_memset(run_mode,  0x0, SDK_REF_PARAMETER_LEN);
    osal_memset(&init_param, 0x0, sizeof(SDK_REF_INIT_PARAM_T));

    while (1)
    {
        int     c = 0;
        int     option_index = 0;
        int     len = 0;

        struct option long_options[] =
        {
            {"forced_id",   required_argument,  0, 'f'  }, /* force id   */
            {"run_mode",    required_argument,  0, 'r'  },
            {"config",      required_argument,  0, 'c'  }, /* air-cfg    */
            {"script",      required_argument,  0, 's'  }, /* dsh script */
            {"help",        no_argument,        0, 'h'  },
            {0,             0,                  0,  0   }
        };

        c = getopt_long(argc, argv, "hf:r:c:s:", long_options, &option_index);
        if (-1 == c)
        {
            break;
        }

        switch (c)
        {
            case 'f':
                _sdk_ref_chkFamilyId(optarg, &init_param);
                break;

            case 'r':
                len = osal_strlen(optarg);
                if (len < SDK_REF_PARAMETER_LEN)
                {
                    osal_strncpy(run_mode, optarg, len);
                    run_mode[len] = '\0';
                    init_param.ptr_run_mode = run_mode;
                    init_param.run_mode_valid = TRUE;
                }
                break;

            case 'c':
                len = osal_strlen(optarg);
                if (len < SDK_REF_FILE_NAME_LEN)
                {
                    osal_strncpy(config_name, optarg, len);
                    config_name[len] = '\0';
                    init_param.ptr_cfg_name = config_name;
                    init_param.cfg_name_valid = TRUE;
                }
                break;

            case 's':
                len = osal_strlen(optarg);
                if (len < SDK_REF_FILE_NAME_LEN)
                {
                    osal_strncpy(script_name, optarg, len);
                    script_name[len] = '\0';
                    init_param.ptr_script_name = script_name;
                    init_param.script_name_valid= TRUE;
                }
                break;

            case 'h':
            default:
                printf("Options:\n");
                printf("    [ -r | --run_mode <SDK_RUN_MODE> ] Assign SDK run mode\n");
                printf("        <SDK_RUN_MODE> : chip, fpga\n");
                printf("    [ -c | --config <cfg_filename> ] Assign sku cfg filename\n");
                printf("    [ -s | --script <script_filename> ] Assign script filename\n");
                printf("    [ -f | --forced_id <forced_id> ] Assign SDK the forced family ID and revision ID\n");
                printf("        <forced_id> : could be <family_id>, <revision_id>, or <family_id>:<revision_id>\n");
                printf("            e.g. \"0x8853\", \"e2\", \"0x8851:e3\"\n");
                exit(-1);
        }
    }

    /* init sdk */
    setbuf(stdout, NULL);
    sdk_ref_initParser();
    sdk_ref_init(&init_param);

    /* diag shell loop */
    dsh_init();
    while (1)
    {
        osal_printf("AIR#");
        memset(ptr_input, 0x0, DSH_CMD_MAX_INPUT);
        sdk_ref_getInput((char *)ptr_input);
        input_len = strlen(ptr_input);
        if ((4 == input_len) && (0 == strncmp("exit", ptr_input, input_len)))
        {
            break;
        }
        if (0 == strncmp("load script name=", ptr_input, 17))
        {
            sdk_ref_loadScript(ptr_input + 17);
            ptr_input[0] = '\0';
        }

        /* inject buffer to sdk */
        dsh_parseString(ptr_input);
    }

    /* deinit sdk */
    air_deinit();
    sdk_ref_deinitParser();
    exit(0);

    return (0);
}
