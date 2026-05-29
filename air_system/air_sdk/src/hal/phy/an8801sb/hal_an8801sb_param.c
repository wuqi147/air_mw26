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

/* FILE NAME:  hal_an8801sb_param.c
 * PURPOSE:
 *  parameters for an8801sb
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
#define AN8801SB_PHY_PARAM_VERSION "v1.0.0"

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

/* GLOBAL VARIABLE DECLARATIONS
 */
/* clang-format off */
const HAL_PHY_CFG_T _hal_an8801sb_longreach[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX          device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------          ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x269,       0,          15,         0,          0x4114},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x268,       0,          15,         0,          0x0341},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x271,       0,          15,         0,          0x2c65},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x302,       0,          15,         0,          0x0014},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xc6,       0,          15,         0,          0x5faa},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x11,       0,          15,         0,          0xff00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x202,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x203,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x204,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x205,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x206,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x207,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x208,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x209,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x20a,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x20b,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x20e,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x20f,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x210,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x211,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x212,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x213,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x214,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x215,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x216,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x217,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21a,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21b,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21c,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21d,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21e,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x21f,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x220,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x221,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x222,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x223,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x226,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x227,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x228,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x229,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22a,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22b,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22c,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22d,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22e,       0,          15,         0,          0x2310},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x22f,       0,          15,         0,          0x0025},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x3e,       0,          15,         0,          0xf000},
    {HAL_PHY_ACCESS_METHOD_CL45,         0x7,       0x3c,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x1f,       0,          15,         0,          0x52b5},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x11,       0,          15,         0,          0x002b},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x12,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x10,       0,          15,         0,          0x8f80},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x1f,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,      0x330,       0,          15,         0,          0x190a},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x33,       0,          15,         0,          0x0177},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x27b,       0,          15,         0,          0x1177},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,      0x171,       0,          15,         0,            0x5e},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,      0x148,       0,          15,         0,           0x200},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x1f,       0,          15,         0,          0x52b5},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x11,       0,          15,         0,          0x8689},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x12,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x10,       0,          15,         0,          0x8f92},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x11,       0,          15,         0,          0x002b},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x10,       0,          15,         0,          0x8f80},
    {HAL_PHY_ACCESS_METHOD_CL22,         0x0,       0x1f,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1f,      0x273,       0,          15,         0,          0x3000},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0xc6,       0,          15,         0,          0x5302},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x40,       0,          15,         0,          0xc000},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x3d,       0,          15,         0,           0xc00},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,      0x170,       0,          15,         0,          0x3333},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x12,       0,           6,         0,            0x3d},
    {HAL_PHY_ACCESS_METHOD_CL45,        0x1e,       0x17,       0,           6,         0,            0x3d},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x12,       0,          15,        10,             0xa},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x17,       0,          13,         8,             0xa},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x19,       0,          13,         8,             0xa},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x21,       0,          13,         8,             0xa},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x16,       0,          15,        10,             0x5},
    {HAL_PHY_ACCESS_METHOD_CL45_INC,    0x1e,       0x18,       0,          13,         8,             0x5}
};

const HAL_PHY_CFG_T _hal_an8801sb_normal[] =
{
/*
 *   HAL_PHY_ACCESS_METHOD_XXX      device_id   reg_addr    delay_time  data_msb    data_lsb    reg_data
 *   -------------------------      ---------   --------    ----------  --------    --------    --------
 */
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x269,       0,          15,         0,          0x1114},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x268,       0,          15,         0,          0x0341},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x271,       0,          15,         0,          0x2c65},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x302,       0,          15,         0,          0x0034},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0xc6,       0,          15,         0,          0x53aa},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0x11,       0,          15,         0,          0x0f00},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x202,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x203,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x204,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x205,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x206,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x207,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x208,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x209,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x20a,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x20b,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x20e,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x20f,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x210,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x211,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x212,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x213,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x214,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x215,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x216,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x217,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21a,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21b,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21c,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21d,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21e,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x21f,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x220,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x221,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x222,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x223,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x226,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x227,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x228,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x229,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22a,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22b,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22c,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22d,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22e,       0,          15,         0,          0x2020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x22f,       0,          15,         0,          0x0020},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0x3e,       0,          15,         0,          0xc000},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x1f,       0,          15,         0,          0x52b5},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x11,       0,          15,         0,          0x001e},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x12,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x10,       0,          15,         0,          0x8f80},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x1f,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,      0x330,       0,          15,         0,          0x000a},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0x33,       0,          15,         0,          0x1177},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x27b,       0,          15,         0,          0x0147},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,      0x171,       0,          15,         0,            0x6f},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,      0x148,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x1f,       0,          15,         0,          0x52b5},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x11,       0,          15,         0,          0xa689},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x12,       0,          15,         0,             0x1},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x10,       0,          15,         0,          0x8f92},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x11,       0,          15,         0,          0x001e},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x10,       0,          15,         0,          0x8f80},
    {HAL_PHY_ACCESS_METHOD_CL22,     0x0,       0x1f,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1f,      0x273,       0,          15,         0,          0x3100},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0xc6,       0,          15,         0,          0x5faa},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0x40,       0,          15,         0,             0x0},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,       0x3d,       0,          15,         0,           0xc00},
    {HAL_PHY_ACCESS_METHOD_CL45,    0x1e,      0x170,       0,          15,         0,          0x1111}
};
/* clang-format on */
const UI32_T _hal_an8801sb_longreach_size = sizeof(_hal_an8801sb_longreach) / sizeof(_hal_an8801sb_longreach[0]);
const UI32_T _hal_an8801sb_normal_size = sizeof(_hal_an8801sb_normal) / sizeof(_hal_an8801sb_normal[0]);