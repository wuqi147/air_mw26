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

/* FILE NAME:  hal_an8808_param.c
 * PURPOSE:
 *  parameters for an8808
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/common/hal.h>
#include <hal/common/hal_phy.h>
#include <hal/phy/an8808/hal_an8808_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define AN8808_PHY_PARAM_VERSION "v1.0.0"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* clang-format off */
const HAL_PHY_CFG_T _hal_an8808_longreach[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX          device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------          ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x148,      0,          15,         0,           0x200},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x148,      0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           3,         3,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           2,         2,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           1,         1,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           0,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x646,      0,           7,         4,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x648,      0,           7,         4,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,          13,        12,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,          11,        10,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64a,      0,          15,         0,          0x1177},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x33,      0,          15,         0,           0x177},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0xc6,      0,          15,         0,          0x5faa},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x3e,      0,          15,         0,          0xf000},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x3d,      0,          15,         0,           0xc00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x12,      0,           5,         0,            0x3f},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x17,      0,           5,         0,            0x3f},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x500,      0,          15,         0,          0x1f4a},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x501,      0,          15,         0,          0xcaf4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x502,      0,          15,         0,          0x1649},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x503,      0,          15,         0,          0x2d2c},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x504,      0,          15,         0,          0xf0ba},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x505,      0,          15,         0,            0x34},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x506,      0,          15,         0,            0x34},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x507,      0,          15,         0,            0x66},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x508,      0,          15,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64b,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64c,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64d,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64e,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64f,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x650,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x651,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x652,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x653,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x654,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x655,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x656,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x657,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x658,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x659,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65a,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65b,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65c,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65d,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65e,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65f,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x660,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x661,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x662,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x663,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x664,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x665,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x666,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x667,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x668,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x669,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66a,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66b,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66c,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66d,      0,          15,         0,          0x2523},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66e,      0,          15,         0,          0x1025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66f,      0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x670,      0,          15,         0,            0x25},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x646,      0,          15,        12,             0x4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x647,      0,           7,         4,             0x4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,           9,         5,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,           4,         0,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x11,      0,          15,         0,          0xff00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x23,      0,          15,         0,           0x220},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x24,      0,          15,         0,           0x220},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x25,      0,          15,         0,           0x220},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x26,      0,          15,         0,           0x220},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,        0x1f,      0,          15,         0,          0x52b5},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,        0x11,      0,          15,         0,            0x2b},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,        0x10,      0,          15,         0,          0x8f80},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,        0x1f,      0,          15,         0,             0x0},
};

const HAL_PHY_CFG_T _hal_an8808_normal[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX      device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------      ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x148,      0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           3,         3,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           2,         2,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           1,         1,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x645,      0,           0,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x646,      0,           7,         4,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x648,      0,           7,         4,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,          13,        12,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,          11,        10,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64a,      0,          15,         0,            0x47},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x33,      0,          15,         0,          0x1177},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0xc6,      0,          15,         0,          0x5faa},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x3e,      0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x3d,      0,          15,         0,           0x800},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x500,      0,          15,         0,          0x1f4a},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x501,      0,          15,         0,          0xcaf4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x502,      0,          15,         0,          0x1649},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x503,      0,          15,         0,          0x2d2c},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x504,      0,          15,         0,          0xf0ba},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x505,      0,          15,         0,            0x34},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x506,      0,          15,         0,            0x34},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x507,      0,          15,         0,            0x66},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x508,      0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64b,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64c,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64d,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64e,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x64f,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x650,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x651,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x652,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x653,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x654,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x655,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x656,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x657,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x658,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x659,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65a,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65b,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65c,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65d,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65e,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x65f,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x660,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x661,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x662,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x663,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x664,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x665,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x666,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x667,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x668,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x669,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66a,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66b,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66c,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66d,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66e,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x66f,      0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x670,      0,          15,         0,            0x20},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x646,      0,          15,        12,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x647,      0,           7,         4,             0x3},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,           9,         5,             0x4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x649,      0,           4,         0,             0x4},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x11,      0,          15,         0,          0x0f00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x23,      0,          15,         0,           0x880},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x24,      0,          15,         0,           0x880},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x25,      0,          15,         0,           0x880},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,        0x26,      0,          15,         0,           0x880},
};

const HAL_PHY_CFG_T _hal_an8808_pre_init_global[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX      device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------      ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x268,      0,          15,         0,           0x7f1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x269,      0,          15,         0,          0x3111},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26a,      0,          15,         0,          0x4000},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26b,      0,          15,         0,            0x74},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26c,      0,          15,         0,          0x4444},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26d,      0,          15,         0,          0x4444},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26e,      0,          15,         0,            0xf7},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x26f,      0,          15,         0,          0x7667},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x271,      0,          15,         0,          0x3c04},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x272,      0,          15,         0,           0xc2b},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x700,      0,          15,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x701,      0,          15,         0,          0x1003},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x702,      0,          15,         0,           0x1f6},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,       0x703,      0,          15,         0,          0x3111},

};

const HAL_PHY_CFG_T _hal_an8808_pre_init_port[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX      device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------      ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xe7,       0,          15,         0,          0x2222},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xe9,       0,          15,         0,             0x5},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xfe,       0,          15,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x171,      0,          15,         0,            0x64},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x13,       0,          15,         0,          0x4040},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xd8,       0,          15,         0,          0x1010},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xd9,       0,          15,         0,          0x0100},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xda,       0,          15,         0,          0x0100},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x3c,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x3d,       0,          15,         0,          0x0800},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x3e,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x11,       0,          15,         0,          0x0f00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x170,      0,           5,         4,             0x2},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x630,      0,          15,         0,          0x1000},
};
/* clang-format on */

const UI32_T _hal_an8808_longreach_size = sizeof(_hal_an8808_longreach) / sizeof(_hal_an8808_longreach[0]);
const UI32_T _hal_an8808_normal_size = sizeof(_hal_an8808_normal) / sizeof(_hal_an8808_normal[0]);
const UI32_T _hal_an8808_pre_init_global_size =
    sizeof(_hal_an8808_pre_init_global) / sizeof(_hal_an8808_pre_init_global[0]);
const UI32_T _hal_an8808_pre_init_port_size = sizeof(_hal_an8808_pre_init_port) / sizeof(_hal_an8808_pre_init_port[0]);

/* clang-format off */
const HAL_QSGMII_CFG_T _hal_an8808q_qp_parm[] =
{
/*
 *   reg_addr       data_msb    data_lsb    reg_data
 *   --------       ---------   --------    --------
 */
    { 0x1022E328,         20,         16,        0x1 },   /* RG_DA_QP_TX_FIR_C1_FORCE */
    { 0x1022E328,         21,         21,        0x1 },   /* RG_DA_QP_TX_FIR_C1_SEL */
    { 0x1022E328,         20,         16,        0x5 },   /* RG_DA_QP_TX_FIR_C1_FORCE */
    { 0x1022E328,         21,         21,        0x1 },   /* RG_DA_QP_TX_FIR_C1_SEL */
    { 0x1022E328,         28,         24,        0x0 },   /* RG_DA_QP_TX_FIR_C2_FORCE */
    { 0x1022E328,         29,         29,        0x1 },   /* RG_DA_QP_TX_FIR_C2_SEL */
    { 0x1022E32C,          5,          0,        0x3 },   /* RG_DA_QP_TX_FIR_C0B_FORCE */
    { 0x1022E32C,          6,          6,        0x1 },   /* RG_DA_QP_TX_FIR_C0B_SEL */
    { 0x1022EF30,          1,          1,        0x1 },   /* RG_QP_CHPEN */
    { 0x1022A330,          3,          2,        0x2 },   /* RG_TPHY_SPEED */
    { 0x1022E408,          1,          0,        0x3 },   /* RG_DA_QP_PLL_BC_INTF */
    { 0x1022E408,          4,          2,        0x5 },   /* RG_DA_QP_PLL_BPA_INTF */
    { 0x1022E408,          7,          6,        0x1 },   /* RG_DA_QP_PLL_BPB_INTF */
    { 0x1022E408,         10,          8,        0x3 },   /* RG_DA_QP_PLL_BR_INTF */
    { 0x1022E408,         29,         29,        0x1 },   /* RG_DA_QP_PLL_RICO_SEL_INTF */
    { 0x1022E408,         13,         12,        0x0 },   /* RG_DA_QP_PLL_FBKSEL_INTF */
    { 0x1022E410,          2,          2,        0x0 },   /* RG_DA_QP_PLL_ICOLP_EN_INTF */
    { 0x1022E408,         14,         14,        0x0 },   /* RG_DA_QP_PLL_ICOIQ_EN_INTF */
    { 0x1022E408,         19,         16,        0x4 },   /* RG_DA_QP_PLL_IR_INTF */
    { 0x1022E408,         21,         20,        0x1 },   /* RG_DA_QP_PLL_KBAND_PREDIV_INTF */
    { 0x1022E408,         25,         24,        0x1 },   /* RG_DA_QP_PLL_PFD_OFFSET_INTF */
    { 0x1022E408,         26,         26,        0x0 },   /* RG_DA_QP_PLL_PFD_OFFSET_EN_INTF */
    { 0x1022E408,         22,         22,        0x1 },   /* RG_DA_QP_PLL_PCK_SEL_INTF */
    { 0x1022E408,         27,         27,        0x0 },   /* RG_DA_QP_PLL_PHY_CK_EN_INTF */
    { 0x1022E408,         28,         28,        0x0 },   /* RG_DA_QP_PLL_POSTDIV_EN_INTF */
    { 0x1022E410,          4,          3,        0x0 },   /* RG_DA_QP_PLL_SDM_HREN_INTF */
    { 0x1022E408,         30,         30,        0x0 },   /* RG_DA_QP_PLL_SDM_IFM_INTF */
    { 0x1022E208,         17,         16,        0x1 },   /* RG_NCPO_ANA_MSB */
    { 0x1022E230,         30,          0, 0x48000000 },   /* RG_LCPLL_NCPO_VALUE */
    { 0x1022E248,         30,          0, 0x48000000 },   /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    { 0x1022E23C,         24,         24,        0x0 },   /* RG_LCPLL_NCPO_CHG */
    { 0x1022E414,          8,          8,        0x0 },   /* RG_DA_QP_PLL_SDM_DI_EN_INTF */
    { 0x1022E40C,         15,          0,        0x0 },   /* RG_DA_QP_PLL_SSC_DELTA_INTF */
    { 0x1022E410,          1,          0,        0x0 },   /* RG_DA_QP_PLL_SSC_DIR_DLY_INTF */
    { 0x1022E40C,         31,         16,        0x0 },   /* RG_DA_QP_PLL_SSC_PERIOD_INTF */
    { 0x1022E414,          9,          9,        0x0 },   /* RG_DA_QP_PLL_TDC_TXCK_SEL_INTF */
    { 0x1022EF40,          3,          3,        0x1 },   /* RG_QP_PLL_SSC_PHASE_INI */
    { 0x1022EF40,          4,          4,        0x1 },   /* RG_QP_PLL_SSC_TRI_EN */
    { 0x1022EF3C,         26,         25,        0x0 },   /* RG_QP_PLL_PREDIV */
    { 0x1022E100,         12,         12,        0x0 },   /* RG_QP_EQ_RX500M_CK_SEL */
    { 0x1022EF28,          0,          0,        0x0 },   /* RG_QP_TX_MODE_16B_EN */
    { 0x1022EF28,         31,         16,      0x300 },   /* RG_QP_TX_RESERVE */
    { 0x1022E000,          0,          0,        0x0 },   /* RG_FVAL_TX_EIDLE_LP_OFF */
    { 0x1022EF04,         11,         11,        0x1 },   /* RG_QP_CDR_PD_10B_EN */
    { 0x1022EF04,         10,         10,        0x0 },   /* RG_QP_CDR_PD_EDGE_DIS */
    { 0x1022EF0C,          5,          4,        0x0 },   /* RG_QP_CDR_LPF_RATIO */
    { 0x1022EF08,         26,         24,        0x4 },   /* RG_QP_CDR_LPF_KP_GAIN */
    { 0x1022EF08,         22,         20,        0x4 },   /* RG_QP_CDR_LPF_KI_GAIN */
    { 0x1022EF14,         28,         25,        0x1 },   /* RG_QP_CDR_PR_BETA_SEL */
    { 0x1022EF14,         31,         29,        0x6 },   /* RG_QP_CDR_PR_BUF_IN_SR */
    { 0x1022EF18,         12,          8,        0xC },   /* RG_QP_CDR_PR_DAC_BAND */
    { 0x1022EF1C,          5,          0,       0x19 },   /* RG_QP_CDR_PR_KBAND_DIV_PCIE */
    { 0x1022EF1C,          6,          6,        0x0 },   /* RG_QP_CDR_PR_KBAND_PCIE_MODE */
    { 0x1022EF20,         12,          6,       0x21 },   /* RG_QP_CDR_PHYCK_DIV */
    { 0x1022EF20,         17,         16,        0x2 },   /* RG_QP_CDR_PHYCK_SEL */
    { 0x1022EF20,         13,         13,        0x0 },   /* RG_QP_CDR_PHYCK_RSTB */
    { 0x1022EF1C,         30,         30,        0x0 },   /* RG_QP_CDR_PR_XFICK_EN */
    { 0x1022EF18,         26,         24,        0x4 },   /* RG_QP_CDR_PR_KBAND_DIV */
    { 0x1022EF00,         17,         16,        0x2 },   /* RG_QP_SIGDET_HF */
    { 0x1022E690,         23,         23,        0x1 },   /* RG_LINK_ERRO_EN */
    { 0x1022E690,         24,         24,        0x0 },   /* RG_LINK_NE_EN */
    { 0x1022E690,         26,         26,        0x1 },   /* RG_QP_EQ_RETRAIN_ONLY_EN */
    { 0x1022E614,          7,          0,       0x6F },   /* RG_QP_RX_PI_CAL_EN_H_DLY */
    { 0x1022E614,         13,          8,       0x3F },   /* RG_QP_RX_SAOSC_EN_H_DLY */
    { 0x1022E6DC,         12,          0,      0x150 },   /* RG_QP_EQ_EN_DLY */
    { 0x1022E630,         28,         16,      0x150 },   /* RG_QP_RX_EQ_EN_H_DLY */
    { 0x1022E124,          2,          0,        0x7 },   /* RG_QP_EQ_LEQOSC_DLYCNT */
    { 0x1022E648,         27,         16,      0x200 },   /* RG_DA_QP_SAOSC_DONE_TIME */
    { 0x1022E690,          0,          0,        0x1 },   /* RG_QP_SIG_LINKDOWN_SEL */
    { 0x1022E694,         31,          0,  0x17D7840 },   /* RG_QP_LINK_ERRO_CNT */
    { 0x1022E63C,         29,         10,        0xA },   /* RG_FREDET_CHK_CYCLE */
    { 0x1022E640,         19,          0,       0x64 },   /* RG_FREDET_GOLDEN_CYCLE */
    { 0x1022E644,         19,          0,     0x2710 },   /* RG_FREDET_TOLERATE_CYCLE */
    { 0x1022E654,          0,          0,        0x1 },   /* RG_FORCE_FREQ_LOCK_SEL */
    { 0x1022E654,          1,          1,        0x1 },   /* RG_FORCE_FREQ_LOCK */
    { 0x10228000,         31,          0, 0x00010020 },   /* xsgmii_msg_tx_ctrl_0 */
    { 0x10228100,         31,          0,  0x0010011 },   /* xsgmii_msg_rx_ctrl_0 */
    { 0x10228520,          4,          4,        0x1 },   /* RG_FORCE_RXC_SEL */
    { 0x1022A324,          1,          1,        0x1 },   /* RG_QSGMII_AN_EN */
    { 0x10220000,         31,          0, 0x00001140 },   /* sgmii_reg_an0 */
    { 0x10221000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p1 */
    { 0x10222000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p2 */
    { 0x10223000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p3 */
    { 0x10220034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13 */
    { 0x10221034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p1 */
    { 0x10222034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p2 */
    { 0x10223034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p3 */
    { 0x10220060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE */
    { 0x10221060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p1 */
    { 0x10222060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p2 */
    { 0x10223060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p3 */
    { 0x1022002C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER */
    { 0x1022102C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p1 */
    { 0x1022202C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p2 */
    { 0x1022302C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p3 */
    { 0x10220010,         31,          0,     0x1801 },   /* sgmii_reg_an_4 */
    { 0x10221010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p1 */
    { 0x10222010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p2 */
    { 0x10223010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p3 */
    { 0x10220A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE */
    { 0x10221A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p1 */
    { 0x10222A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p2 */
    { 0x10223A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p3 */
    { 0x10229100,         31,          0, 0x90000000 },   /* rate_adp_p0_ctrl_0 */
    { 0x10229000,         31,          0,  0xC000C00 },   /* rg_rate_adapt_ctrl_0 */
    { 0x10229300,          0,          0,        0x1 },   /* RG_P0_RA_AN_EN */
    { 0x10229300,          1,          1,        0x1 },   /* RG_P1_RA_AN_EN */
    { 0x10229300,          2,          2,        0x1 },   /* RG_P2_RA_AN_EN */
    { 0x10229300,          3,          3,        0x1 },   /* RG_P3_RA_AN_EN */
    { 0x1022C158,         12,          8,        0x3 },   /* RG_QSGMII_TX_FIFO_RD_THR */
    { 0x1022C158,         20,         16,        0x0 },   /* RG_QSGMII_TX_FIFO_WR_THR */
    { 0x1022C15C,         12,          8,        0x3 },   /* RG_QSGMII_RX_FIFO_RD_THR */
    { 0x1022C15C,         20,         16,        0x0 },   /* RG_QSGMII_RX_FIFO_WR_THR */
    { 0x10220B88,         31,          0,   0x200008 },   /* pcs_dec_erro */
    { 0x10220B84,          0,          0,        0x0 },   /* RG_DET_PAD_EN */
    { 0x1022E004,          7,          7,        0x1 },   /* RG_FORCE_TX_BIT_INVERSE */
    { 0x10270F00,         31,          0, 0x04020808 },   /* sys_config0 */
    { 0x10271F00,         31,          0, 0x04020808 },   /* sys_config0_p1 */
    { 0x10272F00,         31,          0, 0x04020808 },   /* sys_config0_p2 */
    { 0x10273F00,         31,          0, 0x04020808 },   /* sys_config0_p3 */
    { 0x10270F04,         31,          0, 0x04020808 },   /* sys_config1 */
    { 0x10271F04,         31,          0, 0x04020808 },   /* sys_config1_p1 */
    { 0x10272F04,         31,          0, 0x04020808 },   /* sys_config1_p2 */
    { 0x10273F04,         31,          0, 0x04020808 },   /* sys_config1_p3 */
    { 0x1022E400,          0,          0,        0x1 },   /* RG_PHYA_AUTO_INIT */
}; /* ENB-SE1-250415-April_AN8858_Init_Script_QP_QSGMII_Master-R02.xlsx */

const HAL_QSGMII_CFG_T _hal_an8808q_pxp_parm[] =
{
/*
 *   reg_addr       data_msb    data_lsb    reg_data
 *   --------       ---------   --------    --------
 */
    { 0x10250114,         25,         24,        0x1 },   /* RG_XPON_RX_FE_50OHMS_SEL */
    { 0x1025177C,          8,          8,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TX_TERM_SEL */
    { 0x1025177C,          2,          0,        0x1 },   /* RG_FORCE_DA_PXP_TX_TERM_SEL */
    { 0x10250000,         28,         24,       0x10 },   /* RG_XPON_CMN_TRIM */
    { 0x10251364,         12,          8,        0x1 },   /* HW_DA_PXP_TX_FIR_CN1 */
    { 0x10251364,         20,         16,        0x5 },   /* HW_DA_PXP_TX_FIR_C1 */
    { 0x10251364,         26,         24,        0x0 },   /* HW_DA_PXP_TX_FIR_C2 */
    { 0x10251364,          5,          0,        0x4 },   /* HW_DA_PXP_TX_FIR_C0B */
    { 0x10251958,         16,         16,        0x0 },   /* USGMII_TXPMA_SW_RST_N */
    { 0x10251958,         17,         17,        0x0 },   /* USGMII_RXPMA_SW_RST_N */
    { 0x10251990,          0,          0,        0x1 },   /* RG_SIGDET_EN_SEL */
    { 0x10251460,          5,          5,        0x1 },   /* RG_SW_REF_RST_N */
    { 0x10251460,          1,          1,        0x1 },   /* RG_SW_RX_RST_N */
    { 0x10251460,          3,          3,        0x1 },   /* RG_SW_PMA_RST_N */
    { 0x10251460,          6,          6,        0x1 },   /* RG_SW_TX_FIFO_RST_N */
    { 0x10251460,          0,          0,        0x1 },   /* RG_SW_RX_FIFO_RST_N */
    { 0x10251950,         14,         12,        0x4 },   /* RG_FORCE_XSI_MODE */
    { 0x10251950,          8,          8,        0x1 },   /* RG_FORCE_SEL_XSI_MODE */
    { 0x10251414,         11,          9,        0x1 },   /* RG_XFI_RX_MODE */
    { 0x10251414,          5,          3,        0x1 },   /* RG_XFI_TX_MODE */
    { 0x1025011C,          0,          0,        0x1 },   /* RG_XPON_RX_FE_VCM_GEN_PWDB */
    { 0x102500A8,         17,         16,        0x1 },   /* RG_XPON_PLL_LDO_CKDRV_VSEL */
    { 0x102500A8,          8,          8,        0x1 },   /* RG_XPON_PLL_LDO_CKDRV_EN */
    { 0x10250000,          0,          0,        0x1 },   /* RG_XPON_CMN_EN */
    { 0x10250004,          8,          8,        0x1 },   /* RG_XPON_CMN_BYPASS_LPF */
    { 0x10250004,         16,         16,        0x0 },   /* RG_XPON_CMN_BGBYPASS_LPF */
    { 0x10250084,         25,         24,        0x1 },   /* RG_XPON_TXPLL_LDO_VCO_OUT */
    { 0x10250084,         17,         16,        0x1 },   /* RG_XPON_TXPLL_LDO_OUT */
    { 0x10250088,         25,         24,        0x0 },   /* RG_XPON_TXPLL_LDO_VCO_VTP */
    { 0x10250088,         16,         16,        0x0 },   /* RG_XPON_TXPLL_LDO_VCO_VTP_EN */
    { 0x10250088,         10,          8,        0x0 },   /* RG_XPON_TXPLL_VTP */
    { 0x10250088,          0,          0,        0x1 },   /* RG_XPON_TXPLL_VTP_EN */
    { 0x10250064,         24,         24,        0x1 },   /* RG_XPON_TXPLL_PLL_RSTB */
    { 0x10250064,         18,         16,        0x4 },   /* RG_XPON_TXPLL_RST_DLY */
    { 0x10250064,          0,          0,        0x0 },   /* RG_XPON_TXPLL_REFIN_INTERNAL */
    { 0x10250064,          9,          8,        0x0 },   /* RG_XPON_TXPLL_REFIN_DIV */
    { 0x10251854,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    { 0x10251854,         16,         16,        0x0 },   /* RG_FORCE_DA_PXP_TXPLL_EN */
    { 0x10250068,          0,          0,       0x00 },   /* RG_XPON_TXPLL_SDM_DI_EN */
    { 0x10250068,          9,          8,       0x00 },   /* RG_XPON_TXPLL_SDM_DI_LS */
    { 0x10250068,         16,         16,       0x00 },   /* RG_XPON_TXPLL_SDM_IFM */
    { 0x10250068,         25,         24,       0x00 },   /* RG_XPON_TXPLL_SDM_MODE */
    { 0x1025006C,          1,          0,       0x03 },   /* RG_XPON_TXPLL_SDM_ORD */
    { 0x1025006C,         16,         16,       0x00 },   /* RG_XPON_TXPLL_SDM_HREN */
    { 0x1025006C,          8,          8,       0x00 },   /* RG_XPON_TXPLL_SDM_OUT */
    { 0x10250080,         31,         16,       0x00 },   /* RG_XPON_TXPLL_SSC_DELTA */
    { 0x10250080,         15,          0,       0x00 },   /* RG_XPON_TXPLL_SSC_DELTA1 */
    { 0x1025007C,         16,         16,       0x00 },   /* RG_XPON_TXPLL_SSC_TRI_EN */
    { 0x1025007C,          8,          8,       0x00 },   /* RG_XPON_TXPLL_SSC_PHASE_INI */
    { 0x1025007C,          0,          0,       0x00 },   /* RG_XPON_TXPLL_SSC_EN */
    { 0x10250084,         15,          0,       0x00 },   /* RG_XPON_TXPLL_SSC_PERIOD */
    { 0x10250098,          0,          0,       0x00 },   /* RG_XPON_TXPLL_CHP_DOUBLE_EN */
    { 0x10250098,          8,          8,       0x00 },   /* RG_XPON_TXPLL_CHP_IOFST_EN */
    { 0x10250050,          5,          0,       0x24 },   /* RG_XPON_TXPLL_CHP_IBIAS */
    { 0x10250050,         13,          8,       0x00 },   /* RG_XPON_TXPLL_CHP_IOFST */
    { 0x10250050,         20,         16,       0x0A },   /* RG_XPON_TXPLL_LPF_BR */
    { 0x10250050,         28,         24,       0x1F },   /* RG_XPON_TXPLL_LPF_BC */
    { 0x10250054,          4,          0,       0x1F },   /* RG_XPON_TXPLL_LPF_BP */
    { 0x10250054,         12,          8,       0x16 },   /* RG_XPON_TXPLL_LPF_BWR */
    { 0x10250054,         20,         16,       0x18 },   /* RG_XPON_TXPLL_LPF_BWC */
    { 0x10250074,         25,         24,       0x03 },   /* RG_XPON_TXPLL_VCO_CFIX */
    { 0x10250078,          0,          0,       0x01 },   /* RG_XPON_TXPLL_VCO_HALFLSB_EN */
    { 0x10250078,         10,          8,       0x07 },   /* RG_XPON_TXPLL_VCO_SCAPWR */
    { 0x10250078,         26,         24,       0x04 },   /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_H */
    { 0x10250078,         29,         27,       0x00 },   /* RG_XPON_TXPLL_VCO_VCOVAR_BIAS_L */
    { 0x10250078,         18,         16,       0x04 },   /* RG_XPON_TXPLL_VCO_TCLVAR */
    { 0x10251794,         24,         24,       0x01 },   /* RG_FORCE_SEL_DA_PXP_TXPLL_SDM_PCW */
    { 0x10251798,         30,          0, 0x32000000 },   /* RG_FORCE_DA_PXP_TXPLL_SDM_PCW */
    { 0x10251048,         30,          0, 0x64000000 },   /* RG_LCPLL_PON_HRDDS_PCW_NCPO_GPON */
    { 0x1025104C,         30,          0, 0x64000000 },   /* RG_LCPLL_PON_HRDDS_PCW_NCPO_EPON */
    { 0x10250058,         17,         16,       0x00 },   /* RG_XPON_TXPLL_KBAND_KFC */
    { 0x10250058,         25,         24,       0x03 },   /* RG_XPON_TXPLL_KBAND_KF */
    { 0x1025005C,          1,          0,       0x01 },   /* RG_XPON_TXPLL_KBAND_KS */
    { 0x10250058,         10,          8,       0x02 },   /* RG_XPON_TXPLL_KBAND_DIV */
    { 0x10250058,          7,          0,       0xE4 },   /* RG_XPON_TXPLL_KBAND_CODE */
    { 0x10250054,         24,         24,       0x00 },   /* RG_XPON_TXPLL_KBAND_OPTION */
    { 0x10250094,         24,         24,       0x00 },   /* RG_XPON_TXPLL_VCO_KBAND_MEAS_EN */
    { 0x10251858,          8,          8,       0x01 },   /* RG_FORCE_SEL_DA_PXP_TXPLL_KBAND_LOAD_EN */
    { 0x10251858,          0,          0,       0x00 },   /* RG_FORCE_DA_PXP_TXPLL_KBAND_LOAD_EN */
    { 0x1025005C,         17,         16,       0x00 },   /* RG_XPON_TXPLL_MMD_PREDIV_MODE */
    { 0x1025005C,          8,          8,       0x01 },   /* RG_XPON_TXPLL_POSTDIV_EN */
    { 0x10250074,         17,         16,       0x00 },   /* RG_XPON_TXPLL_VCODIV */
    { 0x10250098,         16,         16,       0x00 },   /* RG_XPON_TXPLL_TCL_BYP_EN */
    { 0x10250094,          4,          0,       0x0F },   /* RG_XPON_TXPLL_TCL_KBAND_VREF */
    { 0x10250070,          2,          0,       0x03 },   /* RG_XPON_TXPLL_TCL_AMP_GAIN */
    { 0x10250070,         12,          8,       0x0B },   /* RG_XPON_TXPLL_TCL_AMP_VREF */
    { 0x10250074,         10,          8,       0x00 },   /* RG_XPON_TXPLL_TCL_LPF_BW */
    { 0x10250074,          0,          0,       0x01 },   /* RG_XPON_TXPLL_TCL_LPF_EN */
    { 0x1025006C,         24,         24,       0x01 },   /* RG_XPON_TXPLL_TCL_AMP_EN */
    { 0x102500B4,          1,          0,        0x2 },   /* RG_XPON_TX_SER_LOADSEL */
    { 0x102500C4,          0,          0,        0x1 },   /* RG_XPON_TX_CKLDO_EN */
    { 0x102500C4,         10,          8,        0x3 },   /* RG_XPON_TX_CKLDO_LVR */
    { 0x102500C4,         16,         16,        0x0 },   /* RG_XPON_TX_RXDET_METHOD */
    { 0x102500C4,         24,         24,        0x1 },   /* RG_XPON_TX_DMEDGEGEN_EN */
    { 0x102500C8,          8,          8,        0x1 },   /* RG_XPON_TX_TERMCAL_SELPN */
    { 0x102500C8,         31,         24,        0x0 },   /* RG_XPON_TX_RESERVED */
    { 0x10251874,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TX_CKIN_SEL */
    { 0x10251874,         16,         16,        0x1 },   /* RG_FORCE_DA_PXP_TX_CKIN_SEL */
    { 0x1025177C,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TX_CKIN_DIVISOR */
    { 0x1025177C,         19,         16,        0x4 },   /* RG_FORCE_DA_PXP_TX_CKIN_DIVISOR */
    { 0x10251784,          8,          8,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TX_RATE_CTRL */
    { 0x10251784,          1,          0,        0x2 },   /* RG_FORCE_DA_PXP_TX_RATE_CTRL */
    { 0x10251260,          0,          0,        0x1 },   /* TX_TOP_RST_B */
    { 0x102518EC,          0,          0,        0x1 },   /* RG_FORCE_SEL_PMA_TX_FIFO_WR */
    { 0x102518EC,          4,          4,        0x1 },   /* RG_FORCE_PMA_TX_FIFO_WR */
    { 0x1025199C,         26,         26,        0x0 },   /* SPEED_SEL_1 */
    { 0x10251320,         24,         24,        0x0 },   /* RG_DISB_BLWC_OFFSET */
    { 0x1025148C,          0,          0,        0x0 },   /* RG_DISB_LEQ */
    { 0x102500DC,          8,          8,        0x0 },   /* RG_XPON_CDR_PD_EDGE_DIS */
    { 0x10250118,          8,          8,        0x1 },   /* RG_XPON_RX_FE_VB_EQ1_EN */
    { 0x10250118,         16,         16,        0x1 },   /* RG_XPON_RX_FE_VB_EQ2_EN */
    { 0x10250118,         24,         24,        0x1 },   /* RG_XPON_RX_FE_VB_EQ3_EN */
    { 0x1025188C,          1,          0,        0x3 },   /* RG_FORCE_DA_PXP_RX_FE_GAIN_CTRL */
    { 0x1025188C,          8,          8,        0x1 },   /* RG_FORCE_SEL_DA_PXP_RX_FE_GAIN_CTRL */
    { 0x102500D4,         15,          0,     0x18B0 },   /* RG_XPON_RX_REV_0 */
    { 0x102500D4,         19,         18,        0x0 },   /* RG_XPON_RX_REV_1 */
    { 0x10250120,         17,          8,      0x3FF },   /* RG_XPON_RX_OSCAL_FORCE */
    { 0x10250114,          9,          8,        0x2 },   /* RG_XPON_RX_SIGDET_PEAK */
    { 0x10250114,         20,         16,        0x2 },   /* RG_XPON_RX_SIGDET_VTH_SEL */
    { 0x10250110,         25,         24,        0x3 },   /* RG_XPON_RX_SIGDET_LPF_CTRL */
    { 0x10251840,          0,          0,        0x0 },   /* RG_FORCE_DA_PXP_RX_OSCAL_EN */
    { 0x10251840,          8,          8,        0x1 },   /* RG_FORCE_SEL_DA_PXP_RX_OSCAL_EN */
    { 0x1025176C,         17,         16,        0x1 },   /* RG_FORCE_DA_PXP_RX_OSR_SEL */
    { 0x1025176C,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_RX_OSR_SEL */
    { 0x10251814,         16,         16,        0x1 },   /* RG_FORCE_DA_PXP_CDR_INJCK_SEL */
    { 0x10251814,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_CDR_INJCK_SEL */
    { 0x102500E8,          1,          0,        0x1 },   /* RG_XPON_CDR_LPF_RATIO */
    { 0x102500F8,          6,          0,        0x8 },   /* RG_XPON_CDR_PR_BETA_DAC */
    { 0x102500FC,         20,         16,        0x8 },   /* RG_XPON_CDR_PR_DAC_BAND */
    { 0x1025010C,         19,         19,        0x0 },   /* RG_XPON_CDR_PR_CAP_EN */
    { 0x1025010C,         18,         16,        0x7 },   /* RG_XPON_CDR_PR_BUF_IN_SR */
    { 0x102500D8,          7,          0,        0xB },   /* RG_XPON_RX_PHYCK_DIV */
    { 0x102500D8,          9,          8,        0x1 },   /* RG_XPON_RX_PHYCK_SEL */
    { 0x102500D8,         16,         16,        0x1 },   /* RG_XPON_RX_PHYCK_RSTB */
    { 0x1025010C,          2,          2,        0x1 },   /* RG_XPON_CDR_PR_XFICK_EN */
    { 0x102500CC,          8,          8,        0x1 },   /* RG_XPON_RX_BUSBIT_SEL_FORCE */
    { 0x102500CC,          0,          0,        0x1 },   /* RG_XPON_RX_BUSBIT_SEL */
    { 0x102500CC,         24,         24,        0x1 },   /* RG_XPON_RX_PHY_CK_SEL_FORCE */
    { 0x102500CC,         16,         16,        0x0 },   /* RG_XPON_RX_PHY_CK_SEL */
    { 0x102500D4,         30,         28,        0x5 },   /* RG_XPON_RX_REV_1 */
    { 0x102500D4,         26,         24,        0x5 },   /* RG_XPON_RX_REV_1 */
    { 0x102500D4,         22,         20,        0x5 },   /* RG_XPON_RX_REV_1 */
    { 0x102500D4,         17,         17,        0x0 },   /* RG_XPON_RX_REV_1 */
    { 0x102518B8,          0,          0,        0x1 },   /* RG_FEOS_8B_MODE */
    { 0x1025148C,         15,          8,        0xF },   /* RG_L2D_TRIG_EQ_EN_TIME */
    { 0x10251090,         31,         16,      0xA00 },   /* RG_RX_PICAL_END */
    { 0x10251090,         15,          0,        0x2 },   /* RG_RX_PICAL_START */
    { 0x1025109C,         31,         16,      0x2E0 },   /* RG_RX_SDCAL_END */
    { 0x1025109C,         15,          0,        0x2 },   /* RG_RX_SDCAL_START */
    { 0x10251094,         31,         16,      0x2E0 },   /* RG_RX_PDOS_END */
    { 0x10251094,         15,          0,        0x2 },   /* RG_RX_PDOS_START */
    { 0x10251098,         31,         16,      0x2E0 },   /* RG_RX_FEOS_END */
    { 0x10251098,         15,          0,        0x2 },   /* RG_RX_FEOS_START */
    { 0x10251100,         31,         16,       0x5A },   /* RG_RX_RDY  */
    { 0x10251100,         15,          0,        0x5 },   /* RG_RX_BLWC_RDY_EN */
    { 0x10251148,         29,         23,       0x40 },   /* RG_EQ_BLWC_CNT_BOT_LIM */
    { 0x10251148,         22,         16,       0x3F },   /* RG_EQ_BLWC_CNT_TOP_LIM */
    { 0x10251148,         11,          8,        0xA },   /* RG_EQ_BLWC_GAIN */
    { 0x10251170,         25,         24,        0x1 },   /* RG_KBAND_KFC */
    { 0x10251170,         18,          8,       0xA5 },   /* RG_FPKDIV  */
    { 0x10251170,          2,          0,        0x4 },   /* RG_KBAND_PREDIV */
    { 0x10251178,         18,         16,        0x0 },   /* RG_CK_RATE */
    { 0x10251374,          1,          0,        0x2 },   /* RG_XPON_RX_RATE_CTRL */
    { 0x10251580,          1,          0,        0x2 },   /* RG_PON_TX_RATE_CTRL */
    { 0x1025199C,         24,         24,        0x1 },   /* RG_LCK2REF_COND */
    { 0x10251994,         15,         15,        0x0 },   /* RG_XPON_SIG_OUT_SEL */
    { 0x1025199C,         25,         25,        0x1 },   /* RG_RETRAIN_DEC_ERR_EN */
    { 0x1025133C,          0,          0,        0x0 },   /* RG_DISB_FBCK_LOCK */
    { 0x10251330,          0,          0,        0x1 },   /* RG_FORCE_FBCK_LOCK */
    { 0x10240A00,         30,         30,        0x0 },   /* RG_TBI_10B_MODE */
    { 0x10248000,         31,          0,    0x10020 },   /* xsgmii_msg_tx_ctrl_0 */
    { 0x10248100,         31,          0,  0x0010011 },   /* xsgmii_msg_rx_ctrl_0 */
    { 0x10248520,          4,          4,        0x1 },   /* RG_FORCE_RXC_SEL */
    { 0x10240000,         31,          0, 0x00001140 },   /* sgmii_reg_an0 */
    { 0x10241000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p1 */
    { 0x10242000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p2 */
    { 0x10243000,         31,          0, 0x00001140 },   /* sgmii_reg_an0_p3 */
    { 0x10240034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13 */
    { 0x10241034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p1 */
    { 0x10242034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p2 */
    { 0x10243034,         31,          0, 0x3112010B },   /* sgmii_reg_an_13_p3 */
    { 0x10240060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE */
    { 0x10241060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p1 */
    { 0x10242060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p2 */
    { 0x10243060,          0,          0,        0x1 },   /* RG_FORCE_AN_DONE_p3 */
    { 0x1024002C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER */
    { 0x1024102C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p1 */
    { 0x1024202C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p2 */
    { 0x1024302C,         19,          0,    0x186A0 },   /* SGMII_LINK_TIMER_p3 */
    { 0x10240010,         31,          0,     0x1801 },   /* sgmii_reg_an_4 */
    { 0x10241010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p1 */
    { 0x10242010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p2 */
    { 0x10243010,         31,          0,     0x1801 },   /* sgmii_reg_an_4_p3 */
    { 0x10240A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE */
    { 0x10241A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p1 */
    { 0x10242A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p2 */
    { 0x10243A14,          8,          8,        0x1 },   /* RG_SGMII_MAC_MODE_p3 */
    { 0x10249100,         31,          0, 0x90000000 },   /* rate_adp_p0_ctrl_0 */
    { 0x10249000,         31,          0,  0xC000C00 },   /* rg_rate_adapt_ctrl_0 */
    { 0x10249300,          0,          0,        0x1 },   /* RG_P0_RA_AN_EN */
    { 0x10249300,          1,          1,        0x1 },   /* RG_P1_RA_AN_EN */
    { 0x10249300,          2,          2,        0x1 },   /* RG_P2_RA_AN_EN */
    { 0x10249300,          3,          3,        0x1 },   /* RG_P3_RA_AN_EN */
    { 0x1024C158,         12,          8,        0x3 },   /* RG_QSGMII_TX_FIFO_RD_THR */
    { 0x1024C158,         20,         16,        0x0 },   /* RG_QSGMII_TX_FIFO_WR_THR */
    { 0x1024C15C,         12,          8,        0x3 },   /* RG_QSGMII_RX_FIFO_RD_THR */
    { 0x1024C15C,         20,         16,        0x0 },   /* RG_QSGMII_RX_FIFO_WR_THR */
    { 0x10240B88,         31,          0,   0x200008 },   /* pcs_dec_erro */
    { 0x10240B84,          0,          0,        0x0 },   /* RG_DET_PAD_EN */
    { 0x10274F00,         31,          0, 0x04020808 },   /* sys_config0 */
    { 0x10275F00,         31,          0, 0x04020808 },   /* sys_config0 */
    { 0x10276F00,         31,          0, 0x04020808 },   /* sys_config0 */
    { 0x10277F00,         31,          0, 0x04020808 },   /* sys_config0 */
    { 0x10274F04,         31,          0, 0x04020808 },   /* sys_config1 */
    { 0x10275F04,         31,          0, 0x04020808 },   /* sys_config1 */
    { 0x10276F04,         31,          0, 0x04020808 },   /* sys_config1 */
    { 0x10277F04,         31,          0, 0x04020808 },   /* sys_config1 */
    { 0x10251854,         24,         24,        0x1 },   /* RG_FORCE_SEL_DA_PXP_TXPLL_EN */
    { 0x10251854,         16,         16,        0x1 },   /* RG_FORCE_DA_PXP_TXPLL_EN */
    { 0x10250060,          8,          8,        0x1 },   /* RG_XPON_TXPLL_PHY_CK2_EN */
    { 0x10250060,          0,          0,        0x1 },   /* RG_XPON_TXPLL_PHY_CK1_EN */
    { 0x10251460,         17,         17,        0x1 },   /* RG_SW_XFI_RXMAC_RST_N */
    { 0x10251460,         16,         16,        0x1 },   /* RG_SW_XFI_TXMAC_RST_N */
    { 0x10251460,         15,         15,        0x1 },   /* RG_SW_PONOLT_RXMAC_RST_N */
    { 0x10251460,         13,         13,        0x1 },   /* RG_SW_PONOLT_TXMAC_RST_N */
    { 0x10251460,         11,         11,        0x1 },   /* RG_SW_HSG_RXPCS_RST_N */
    { 0x10251460,         10,         10,        0x1 },   /* RG_SW_HSG_TXPCS_RST_N */
    { 0x10251460,          8,          8,        0x1 },   /* RG_SW_XFI_RXPCS_RST_N */
    { 0x10251460,          7,          7,        0x1 },   /* RG_SW_XFI_TXPCS_RST_N */
    { 0x10251460,          4,          4,        0x1 },   /* RG_SW_ALLPCS_RST_N */
    { 0x10251990,          0,          0,        0x0 },   /* RG_SIGDET_EN_SEL */
    { 0x10251460,          2,          2,        0x1 },   /* RG_SW_TX_RST_N */
    { 0x10251958,         16,         16,        0x1 },   /* USGMII_TXPMA_SW_RST_N */
    { 0x10251958,         17,         17,        0x1 },   /* USGMII_RXPMA_SW_RST_N */
    { 0x1025120C,         24,         24,        0x1 },   /* RG_RO_TOGGLE */
    { 0x1025120C,         24,         24,        0x0 },   /* RG_RO_TOGGLE */
}; /* ENB-SE1-250415-April_AN8858_Init_Script_PXP_QSGMII_Master-R02.xlsx */
/* clang-format on */

const unsigned int _hal_an8808q_pxp_size = sizeof(_hal_an8808q_pxp_parm) / sizeof(_hal_an8808q_pxp_parm[0]);
const unsigned int _hal_an8808q_qp_size = sizeof(_hal_an8808q_qp_parm) / sizeof(_hal_an8808q_qp_parm[0]);
