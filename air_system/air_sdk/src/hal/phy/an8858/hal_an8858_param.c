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

/* FILE NAME:  hal_an8858_param.c
 * PURPOSE:
 *  parameters for an8858
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
 */
#include <hal/common/hal.h>
#include <hal/common/hal_phy.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define an8858_PHY_PARAM_VERSION "v1.0.0"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */

/* clang-format off */
const HAL_PHY_CFG_T _hal_an8858_longreach[] =
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
const HAL_PHY_CFG_T _hal_an8858_normal[] =
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

const HAL_PHY_CFG_T _hal_an8858_pre_init_global[] =
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

const HAL_PHY_CFG_T _hal_an8858_pre_init_port[] =
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

const UI32_T _hal_an8858_longreach_size = sizeof(_hal_an8858_longreach) / sizeof(_hal_an8858_longreach[0]);
const UI32_T _hal_an8858_normal_size = sizeof(_hal_an8858_normal) / sizeof(_hal_an8858_normal[0]);
const UI32_T _hal_an8858_pre_init_global_size =
    sizeof(_hal_an8858_pre_init_global) / sizeof(_hal_an8858_pre_init_global[0]);
const UI32_T _hal_an8858_pre_init_port_size = sizeof(_hal_an8858_pre_init_port) / sizeof(_hal_an8858_pre_init_port[0]);