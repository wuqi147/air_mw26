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

/* FILE NAME:  osal_mdc.h
 * PURPOSE:
 * 1. Provide device operate from AML interface
 * NOTES:
 *
 */

#ifndef OSAL_MDC_H
#define OSAL_MDC_H

#include <air_cfg.h>
#include <air_error.h>
#include <air_types.h>
#include <aml/aml.h>
#include <hal/common/hal_dev.h>
/* #define OSAL_MDC_EN_MSI */
/* #define OSAL_MDC_DMA_RESERVED_MEM_CACHEABLE */
/* #define OSAL_MDC_EN_TEST */

/* NAMING CONSTANT DECLARATIONS
 */
#if defined(AIR_FREERTOS)
#define OSAL_MDC_PBUS_WIDTH (4)
#elif defined(AIR_LINUX_USER_MODE)
#define OSAL_MDC_DRIVER_NAME           "air_dev"
#define OSAL_MDC_DRIVER_MISC_MAJOR_NUM (10)
#define OSAL_MDC_DRIVER_MISC_MINOR_NUM (250)
#else /* AIR_LINUX_KERNEL_MODE*/

#endif

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
#if defined(AIR_LINUX_USER_MODE)

/* Data type of IOCTL argument for device initialization */
#pragma pack(push, 1)
typedef struct
{
    AML_DEV_ID_T id[AIR_CFG_MAXIMUM_CHIPS_PER_SYSTEM];
    AML_HW_IF_T  if_type;
    UI32_T       dev_num;
} OSAL_MDC_IOCTL_DEV_DATA_T;
#pragma pack(pop)

typedef struct OSAL_MDC_IOCTL_I2C_DATA_T
{
    UI32_T addr;
    UI32_T data;
    UI32_T dev_num;
} OSAL_MDC_IOCTL_I2C_DATA_S;

typedef enum OSAL_MDC_IOCTL_ACCESS_S
{
    OSAL_MDC_IOCTL_ACCESS_NONE = 0,
    OSAL_MDC_IOCTL_ACCESS_READ,
    OSAL_MDC_IOCTL_ACCESS_WRITE,
    OSAL_MDC_IOCTL_ACCESS_READ_WRITE,
    OSAL_MDC_IOCTL_ACCESS_LAST
} OSAL_MDC_IOCTL_ACCESS_T;

typedef enum OSAL_MDC_IOCTL_TYPE_S
{
    OSAL_MDC_IOCTL_TYPE_INIT_DEV = 0,
    OSAL_MDC_IOCTL_TYPE_DEINIT_DEV,
    OSAL_MDC_IOCTL_TYPE_REG_READ,
    OSAL_MDC_IOCTL_TYPE_REG_WRITE,
    OSAL_MDC_IOCTL_TYPE_LAST
} OSAL_MDC_IOCTL_TYPE_T;

typedef union
{
    UI32_T value;
    struct
    {
        UI32_T access : 2;  /* 0:read, 1:write, 2:read and write, 3:none */
        UI32_T unit   : 6;  /* Maximum unit number is 64.                */
        UI32_T size   : 14; /* Maximum IOCTL data size is 16KB.          */
        UI32_T type   : 10; /* Maximum 1024 IOCTL types                  */
    } field;
} OSAL_MDC_IOCTL_CMD_T;

typedef AIR_ERROR_NO_T (*OSAL_MDC_IOCTL_CALLBACK_FUNC_T)(
    const UI32_T unit,
    void        *ptr_data);

#endif
/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

AIR_ERROR_NO_T
osal_mdc_initDevice(
    AML_DEV_T *ptr_dev_list,
    UI32_T    *ptr_dev_num);

AIR_ERROR_NO_T
osal_mdc_deinitDevice(
    void);

#endif /* End of OSAL_MDC_H */
