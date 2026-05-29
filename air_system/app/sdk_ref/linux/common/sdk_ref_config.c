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

#if defined (AIR_LINUX_USER_MODE)
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#endif

#if defined (AIR_LINUX_KERNEL_MODE)
#include <linux/string.h>
#include <linux/kernel.h>
#endif

#include <air_error.h>
#include <air_types.h>
#include <air_cfg.h>
#include <osal/osal.h>
#include <osal/osal_lib.h>

#include <sdk_ref.h>

#define SDK_CFG_TYPE_NAME_MAX_LEN   (100)
#define SDK_CFG_TYPE_LINE_MAX_LEN   (100)
#define SDK_CFG_FILE_MAX_SIZE       (65536)

typedef struct
{
    AIR_CFG_TYPE_T          type;
    C8_T                    name[SDK_CFG_TYPE_NAME_MAX_LEN];
} SDK_REF_CFG_INFO_T;

static C8_T                 _sdk_ref_cfg_file[SDK_REF_FILE_NAME_LEN + 1];
static SDK_REF_CFG_INFO_T   _sdk_ref_cfg_info[] =
{
    {AIR_CFG_TYPE_IFMON_ENABLE,                     "AIR_CFG_TYPE_IFMON_ENABLE"},
    {AIR_CFG_TYPE_IFMON_THREAD_PRI,                 "AIR_CFG_TYPE_IFMON_THREAD_PRI"},
    {AIR_CFG_TYPE_IFMON_THREAD_STACK,               "AIR_CFG_TYPE_IFMON_THREAD_STACK"},
    {AIR_CFG_TYPE_PHY_ACCESS_TYPE,                  "AIR_CFG_TYPE_PHY_ACCESS_TYPE"},
    {AIR_CFG_TYPE_PHY_I2C_BUS_ID,                   "AIR_CFG_TYPE_PHY_I2C_BUS_ID"},
    {AIR_CFG_TYPE_PHY_I2C_ADDRESS,                  "AIR_CFG_TYPE_PHY_I2C_ADDRESS"},
    {AIR_CFG_TYPE_PHY_ADDRESS,                      "AIR_CFG_TYPE_PHY_ADDRESS"},
    {AIR_CFG_TYPE_PHY_LED_BEHAVIOR,                 "AIR_CFG_TYPE_PHY_LED_BEHAVIOR"},
    {AIR_CFG_TYPE_PHY_LED_TYPE,                     "AIR_CFG_TYPE_PHY_LED_TYPE"},
    {AIR_CFG_TYPE_PHY_LED_COUNT,                    "AIR_CFG_TYPE_PHY_LED_COUNT"},
    {AIR_CFG_TYPE_SERDES_POLARITY_REVERSE,          "AIR_CFG_TYPE_SERDES_POLARITY_REVERSE"},
    {AIR_CFG_TYPE_MAX_ENTRY_CNT_MAC_BASED_VLAN,     "AIR_CFG_TYPE_MAX_ENTRY_CNT_MAC_BASED_VLAN"},
    {AIR_CFG_TYPE_MAX_ENTRY_CNT_IPV4_BASED_VLAN,    "AIR_CFG_TYPE_MAX_ENTRY_CNT_IPV4_BASED_VLAN"},
    {AIR_CFG_TYPE_MAX_ENTRY_CNT_SERVICE_VLAN,       "AIR_CFG_TYPE_MAX_ENTRY_CNT_SERVICE_VLAN"},
    {AIR_CFG_TYPE_FORCE_DEVICE_ID,                  "AIR_CFG_TYPE_FORCE_DEVICE_ID"},
    {AIR_CFG_TYPE_MDIO_ENABLE,                      "AIR_CFG_TYPE_MDIO_ENABLE"},
    {AIR_CFG_TYPE_MDIO_CLOCK,                       "AIR_CFG_TYPE_MDIO_CLOCK"},
    {AIR_CFG_TYPE_SIF_LOCAL_CLOCK,                  "AIR_CFG_TYPE_SIF_LOCAL_CLOCK"},
    {AIR_CFG_TYPE_SIF_REMOTE_SLAVE_ID,              "AIR_CFG_TYPE_SIF_REMOTE_SLAVE_ID"},
    {AIR_CFG_TYPE_SIF_REMOTE_CLOCK,                 "AIR_CFG_TYPE_SIF_REMOTE_CLOCK"},
    {AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN,             "AIR_CFG_TYPE_PERIF_FORCE_GPIO_PIN"},
    {AIR_CFG_TYPE_COMBO_PORT_LED_TYPE,              "AIR_CFG_TYPE_COMBO_PORT_LED_TYPE"},
    {AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE,       "AIR_CFG_TYPE_SERDES_SYSTEM_SIDE_OP_MODE"},
    {AIR_CFG_TYPE_PORT_ADMIN_STATE,                 "AIR_CFG_TYPE_PORT_ADMIN_STATE"},
    {AIR_CFG_TYPE_SERDES_PORT_OPTION,               "AIR_CFG_TYPE_SERDES_PORT_OPTION"},
    {AIR_CFG_TYPE_L2_POLL_THREAD_PRI,               "AIR_CFG_TYPE_L2_POLL_THREAD_PRI"},
    {AIR_CFG_TYPE_L2_POLL_THREAD_STACK,             "AIR_CFG_TYPE_L2_POLL_THREAD_STACK"},
    {AIR_CFG_TYPE_L2_POLL_INTERVAL,                 "AIR_CFG_TYPE_L2_POLL_INTERVAL"},
    {AIR_CFG_TYPE_L2_POLL_ENABLE,                   "AIR_CFG_TYPE_L2_POLL_ENABLE"},
    {AIR_CFG_TYPE_SURGE_PROTECTION_MODE,            "AIR_CFG_TYPE_SURGE_PROTECTION_MODE"},
    {AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR,   "AIR_CFG_TYPE_COMBO_PORT_SERDES_LED_BEHAVIOR"},
    {AIR_CFG_TYPE_CHIP_CO_CLOCK_ENABLE,             "AIR_CFG_TYPE_CHIP_CO_CLOCK_ENABLE"},
    {AIR_CFG_TYPE_LPDET_ETH_TYPE,                   "AIR_CFG_TYPE_LPDET_ETH_TYPE"},
    {AIR_CFG_TYPE_BACKPRES_CTRL_MODE,               "AIR_CFG_TYPE_BACKPRES_CTRL_MODE"},
    {AIR_CFG_TYPE_PERIF_GPIO_AUTO_MODE,             "AIR_CFG_TYPE_PERIF_GPIO_AUTO_MODE"},
    {AIR_CFG_TYPE_PERIF_GPIO_AUTO_PATT,             "AIR_CFG_TYPE_PERIF_GPIO_AUTO_PATT"},
    {AIR_CFG_TYPE_POE_LED_BEHAVIOR,                 "AIR_CFG_TYPE_POE_LED_BEHAVIOR"},
    {AIR_CFG_TYPE_POE_RESET_SCRATCH,                "AIR_CFG_TYPE_POE_RESET_SCRATCH"},
    {AIR_CFG_TYPE_L2_FDB_FULL_BEHAVIOR,             "AIR_CFG_TYPE_L2_FDB_FULL_BEHAVIOR"},
    {AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT,         "AIR_CFG_TYPE_MAX_RETRANSMISSION_COUNT"},
    /* Last Type */
    {AIR_CFG_TYPE_LAST,                             "AIR_CFG_TYPE_LAST"}
};

static AIR_ERROR_NO_T
_sdk_ref_getCfgValue(
    const char              *ptr_in_str,
    UI32_T                  *ptr_in_value)
{
    if (('0' == ptr_in_str[0]) && (('x' == ptr_in_str[1]) || ('X' == ptr_in_str[1])))
    {
        sscanf(ptr_in_str + 2, "%x", ptr_in_value);
    }
    else
    {
        sscanf(ptr_in_str, "%d", ptr_in_value);
    }

    return  AIR_E_OK;
}

static AIR_ERROR_NO_T
_sdk_ref_getCfgType(
    const char              *ptr_cfg_type_name,
    AIR_CFG_TYPE_T          *ptr_cfg_type)
{
    UI32_T  index;
    UI32_T  size = sizeof(_sdk_ref_cfg_info) / sizeof(SDK_REF_CFG_INFO_T);

    *ptr_cfg_type = 0xffffff;

    for (index = 0; index < size; index++)
    {
        if (0 == osal_strncmp(ptr_cfg_type_name, _sdk_ref_cfg_info[index].name,
                    osal_strlen(_sdk_ref_cfg_info[index].name)))
        {
            *ptr_cfg_type = _sdk_ref_cfg_info[index].type;
            break;
        }
    }

    return  AIR_E_OK;

}

static AIR_ERROR_NO_T
_sdk_ref_getCfg(
    const UI32_T            in_unit,
    const AIR_CFG_TYPE_T    in_cfg_type,
    AIR_CFG_VALUE_T         *ptr_in_value)
{
    AIR_ERROR_NO_T  rc;
    void            *ptr_cfg_file;
    UI32_T          cfg_unit;
    AIR_CFG_VALUE_T cfg_value;
    AIR_CFG_TYPE_T  cfg_type;

    C8_T            cfg_unit_str   [SDK_CFG_TYPE_NAME_MAX_LEN];
    C8_T            cfg_type_name  [SDK_CFG_TYPE_NAME_MAX_LEN];
    C8_T            cfg_param0_str [SDK_CFG_TYPE_NAME_MAX_LEN];
    C8_T            cfg_param1_str [SDK_CFG_TYPE_NAME_MAX_LEN];
    C8_T            cfg_value_str  [SDK_CFG_TYPE_NAME_MAX_LEN];

    UI32_T          index;
    C8_T            *ptr_file_buf;
    C8_T            *ptr_ori_file_buf;
    C8_T            *ptr_line;

    /* try to find the configuration file */
    ptr_cfg_file = sdk_ref_openFile(_sdk_ref_cfg_file, SDK_REF_FILE_TYPE_READ);
    if (NULL == ptr_cfg_file)
    {
        return  AIR_E_OTHERS;
    }

    /* read the file into bufer */
    ptr_file_buf = osal_alloc(SDK_CFG_FILE_MAX_SIZE,"sdk_ref");
    ptr_ori_file_buf = ptr_file_buf;
    if (NULL == ptr_ori_file_buf)
    {
        sdk_ref_closeFile(ptr_cfg_file);
        return  AIR_E_NO_MEMORY;
    }

    osal_memset(ptr_file_buf, 0x0, SDK_CFG_FILE_MAX_SIZE);
    index = sdk_ref_readFile(ptr_cfg_file, ptr_file_buf, SDK_CFG_FILE_MAX_SIZE);
    ptr_file_buf[SDK_CFG_FILE_MAX_SIZE - 1] = '\0';

    if ((SDK_CFG_FILE_MAX_SIZE <= index) || (0 == index))
    {
        osal_free(ptr_ori_file_buf);
        sdk_ref_closeFile(ptr_cfg_file);
        return  AIR_E_OTHERS;
    }

    /* loop-read the every line to parse the information to find out matched entry */
    rc = AIR_E_ENTRY_NOT_FOUND;
    while ((ptr_line = strsep(&ptr_file_buf,"\n")) != NULL)
    {
        /* filter out the line what is length is longer than SDK_CFG_TYPE_LINE_MAX_LEN */
        index = osal_strlen(ptr_line);
        if (index >= SDK_CFG_TYPE_LINE_MAX_LEN)
        {
            continue;
        }

        /* try to check the first number's validaty.
         * filter out the space bar. the first value must be digital number.
         * the lines Beginning with # are comment lines
         */
        for (index = 0; index < SDK_CFG_TYPE_LINE_MAX_LEN; index++)
        {
            if (ptr_line[index] != ' ')
            {
                break;
            }
        }
        if (index >= SDK_CFG_TYPE_LINE_MAX_LEN)
        {
            continue;
        }

        if ((ptr_line[index] > '9') || (ptr_line[index] < '0'))
        {
            continue;
        }

        /* get the formatted value from the buffer */
        osal_memset(cfg_unit_str,   0x0, SDK_CFG_TYPE_NAME_MAX_LEN);
        osal_memset(cfg_type_name,  0x0, SDK_CFG_TYPE_NAME_MAX_LEN);
        osal_memset(cfg_param0_str, 0x0, SDK_CFG_TYPE_NAME_MAX_LEN);
        osal_memset(cfg_param1_str, 0x0, SDK_CFG_TYPE_NAME_MAX_LEN);
        osal_memset(cfg_value_str,  0x0, SDK_CFG_TYPE_NAME_MAX_LEN);

        sscanf(ptr_line, "%s %s %s %s %s",
               cfg_unit_str,
               cfg_type_name,
               cfg_param0_str,
               cfg_param1_str,
               cfg_value_str);
        if (0 != osal_strncmp(cfg_type_name, "AIR_CFG_TYPE", 12))
        {
            continue;
        }

        _sdk_ref_getCfgValue(cfg_unit_str, &cfg_unit);
        _sdk_ref_getCfgType(cfg_type_name, &cfg_type);
        _sdk_ref_getCfgValue(cfg_param0_str, &cfg_value.param0);
        _sdk_ref_getCfgValue(cfg_param1_str, &cfg_value.param1);
        _sdk_ref_getCfgValue(cfg_value_str, (UI32_T *)&cfg_value.value);

        /* match the unit, param0, param1 and type */
        if ((cfg_value.param0 == ptr_in_value->param0) &&
            (cfg_value.param1 == ptr_in_value->param1) &&
            (cfg_unit == in_unit) &&
            (cfg_type == in_cfg_type))
        {
            rc = AIR_E_OK;
            ptr_in_value->value = cfg_value.value;
            break;
        }
    }

    osal_free(ptr_ori_file_buf);
    sdk_ref_closeFile(ptr_cfg_file);

    return  rc;

}

AIR_ERROR_NO_T
sdk_ref_initCfg(
    const C8_T      *ptr_name)
{
    UI32_T          unit;
    UI32_T          length;
    AIR_ERROR_NO_T  rc = AIR_E_OK;

    if (NULL == ptr_name)
    {
        osal_printf("ptr_name file not found, use default config\n");
    }
    else
    {
        length = osal_strlen(ptr_name);
        if (length >= SDK_REF_FILE_NAME_LEN)
        {
            osal_printf("ptr_name too long, use default config\n");
        }
        else
        {
            /* try to check if  the configuration file is existed */

            /* save the file name, will be used later */
            osal_memset(_sdk_ref_cfg_file, 0x0, SDK_REF_FILE_NAME_LEN);
            osal_strncpy(_sdk_ref_cfg_file, ptr_name, length);

            /* register to SDK only if the configuration file exists */
            for (unit = 0; ((unit < AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM) && (AIR_E_OK == rc)); unit++)
            {
                rc = air_cfg_register(unit, _sdk_ref_getCfg);
            }
        }
    }

    return (rc);
}

