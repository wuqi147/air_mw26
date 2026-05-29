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

/* FILE NAME:  gphy.c
 * PURPOSE:
 *      It provides Gigabit Ethernet BASE-T driver function.
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARATIONS
 */
#include <stdio.h>
#include "gphy.h"
#include "util.h"

#if defined(AIR_SUPPORT_ACE)
/* NAMING CONSTANT DECLARATIONS
 */
/* Initialization Tables */
#define TOTAL_NUMBER_OF_PATCH    (14)
static unsigned short eee_patch_table[TOTAL_NUMBER_OF_PATCH][2] = {
    {RgAddr_dev1Eh_reg120h, 0x8014},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg122h, 0xFFFF},
    {RgAddr_dev1Eh_reg144h, 0x0200},
    {RgAddr_dev1Eh_reg14Ah, 0xEE20},
    {RgAddr_dev1Eh_reg19Bh, 0x0111},
    {RgAddr_dev1Eh_reg234h, 0x1181},
    {RgAddr_dev1Eh_reg238h, 0x0120},
    {RgAddr_dev1Eh_reg239h, 0x0117},
    {RgAddr_dev1Fh_reg268h, 0x07F4},
    {RgAddr_dev1Eh_reg2D1h, 0x0733},
    {RgAddr_dev1Eh_reg323h, 0x0011},
    {RgAddr_dev1Eh_reg324h, 0x013F},
    {RgAddr_dev1Eh_reg326h, 0x0037}
};

#define TOTAL_NUMBER_OF_TR    (19)
static unsigned short tr_reg_table[TOTAL_NUMBER_OF_TR][3] = {
    /* data1, data2, cmd  */
    {0x55A0, 0x0000, 0x83AA},
    {0xFF3F, 0x0007, 0x83AE},
    {0x001E, 0x0000, 0x8F80},
    {0xB90A, 0x006F, 0x8F82},
    {0x0671, 0x0006, 0x8FAE},
    {0x2F00, 0x000E, 0x8FB0},
    {0x4444, 0x0044, 0x8ECC},
    {0x0004, 0x0000, 0x9686},
    {0xBAEF, 0x002E, 0x968C},
    {0x000B, 0x0000, 0x9690},
    {0x504D, 0x0000, 0x9698},
    {0x314F, 0x0002, 0x969A},
    {0x3028, 0x0000, 0x969E},
    {0x5010, 0x0000, 0x96A0},
    {0x0001, 0x0004, 0x96A2},
    {0x8670, 0x0001, 0x96A6},
    {0x024A, 0x0000, 0x96A8},
    {0x0072, 0x0000, 0x96B6},
    {0x3210, 0x0000, 0x96B8}
};
#define RgAddr_gsw_top_reg_REG_PHY_IAC    (0x10218000+0x0020)
#define RgAddr_gsw_top_reg_REG_PHY_IAD    (0x10218000+0x0024)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

typedef unsigned long    FIELD;

typedef union
{
    struct
    {
        FIELD csr_mdio_wr_data          : 16;
        FIELD csr_mdio_st               : 2;
        FIELD csr_mdio_cmd              : 2;
        FIELD csr_mdio_phy_addr         : 5;
        FIELD csr_mdio_reg_addr         : 5;
        FIELD rsv_30                    : 1;
        FIELD csr_phy_acs_st            : 1;
    } Bits;
    unsigned int Raw;
} gsw_top_reg_REG_PHY_IAC, *Pgsw_top_reg_REG_PHY_IAC;

typedef union
{
    struct
    {
        FIELD csr_mdio_rd_data          : 16;
        FIELD rsv_16                    : 16;
    } Bits;
    unsigned int Raw;
} gsw_top_reg_REG_PHY_IAD, *Pgsw_top_reg_REG_PHY_IAD;

/* GLOBAL VARIABLE DECLARATIONS
 */

/* EXPORTED SUBPROGRAM BODIES
 */

unsigned short get_gphy_reg_cl22(unsigned char phyad, unsigned char reg)
{
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;
    gsw_top_reg_REG_PHY_IAD REG_PHY_IAD_val;

    /* Wait until done */
    do
    {
      REG_PHY_IAC_val.Raw   = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Set address */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd   = 2;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = phyad;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st =   1;
    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
    /* Wait until done */
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    REG_PHY_IAD_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAD);

    return REG_PHY_IAD_val.Raw;
}

void set_gphy_reg_cl22(unsigned char phyad, unsigned char reg, unsigned short value)
{
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;

    /* Wait until done */
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Set address */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = phyad;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = reg;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = value;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
}

unsigned short get_gphy_reg_cl45(unsigned char prtid, unsigned char devid, unsigned short reg)
{
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;
    gsw_top_reg_REG_PHY_IAD REG_PHY_IAD_val;

    /* Wait until done */
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Set address */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    /* Wait until done */
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Read value */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 3;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = 0;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;
    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    /* Wait until done */
    do
    {
        REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    REG_PHY_IAD_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAD);

    return REG_PHY_IAD_val.Raw;
}

void set_gphy_reg_cl45(unsigned char prtid, unsigned char devid, unsigned short reg, unsigned short value)
{
    gsw_top_reg_REG_PHY_IAC REG_PHY_IAC_val;

    /* Wait until done */
    do
    {
      REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Set address */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = reg;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);

    /* Wait until done */
    do
    {
      REG_PHY_IAC_val.Raw = io_read32(RgAddr_gsw_top_reg_REG_PHY_IAC);
    }
    while(REG_PHY_IAC_val.Bits.csr_phy_acs_st);

    /* Write value */
    REG_PHY_IAC_val.Bits.csr_mdio_st = 0;
    REG_PHY_IAC_val.Bits.csr_mdio_cmd = 1;
    REG_PHY_IAC_val.Bits.csr_mdio_phy_addr = prtid;
    REG_PHY_IAC_val.Bits.csr_mdio_reg_addr = devid;
    REG_PHY_IAC_val.Bits.csr_mdio_wr_data = value;
    REG_PHY_IAC_val.Bits.csr_phy_acs_st = 1;

    io_write32(RgAddr_gsw_top_reg_REG_PHY_IAC, REG_PHY_IAC_val.Raw);
}

void TR_RegWr(unsigned short phyadd, unsigned short tr_reg_addr, unsigned int tr_data)
{
    set_gphy_reg_cl22(phyadd, 0x1F, 0x52b5);       /* page select */
    set_gphy_reg_cl22(phyadd, 0x11, (unsigned short)(tr_data & 0xffff));
    set_gphy_reg_cl22(phyadd, 0x12, (unsigned short)(tr_data >> 16));
    set_gphy_reg_cl22(phyadd, 0x10, (unsigned short)(tr_reg_addr | TrReg_WR));
    set_gphy_reg_cl22(phyadd, 0x1F, 0x0);          /* page resetore */
    return;
}

static void set_gphy_TrReg(unsigned char prtid, unsigned short parm_1, unsigned short parm_2, unsigned short parm_3)
{
    set_gphy_reg_cl22(prtid, RgAddr_TrReg11h, parm_1);
    set_gphy_reg_cl22(prtid, RgAddr_TrReg12h, parm_2);
    set_gphy_reg_cl22(prtid, RgAddr_TrReg10h, parm_3);
}

void gphy_init(void)
{
    unsigned char   port = 1, index = 0;
    unsigned short  data = 0;
    unsigned int    hwtrap = 0, phy_base = 0;

    hwtrap = io_read32(HW_TRAP_READ_REG);
    phy_base = (hwtrap >> PHY_BASE_ADDR_SHIFT) & PHY_BASE_ADDR_MASK;            /* get PHY base address */
    for (port = 1; port <= 8; port++)
    {
        data = get_gphy_reg_cl22(phy_base + port, MII_BMCR);
        set_gphy_reg_cl22(phy_base + port, MII_BMCR, data & ~(BMCR_PDOWN));     /* PHY power on */

        /* Change EEE RG default value */
        for (index = 0; index < TOTAL_NUMBER_OF_PATCH; index++)
        {
            set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, eee_patch_table[index][0], eee_patch_table[index][1]);
        }

        set_gphy_reg_cl22(phy_base + port, RgAddr_dev1Fh, CL22_Page_TrReg);     /* change CL22page to LpiReg(0x3) */
        for (index = 0; index < TOTAL_NUMBER_OF_TR; index++)
        {
            set_gphy_TrReg(phy_base + port, tr_reg_table[index][0], tr_reg_table[index][1], tr_reg_table[index][2]);
        }
        /* change CL22page to LpiReg(0x3) */
        set_gphy_reg_cl22(phy_base + port, 0x1f, CL22_Page_LpiReg);
        /* Fine turn SigDet for B2B LPI link down issue */
        set_gphy_reg_cl22(phy_base + port, RgAddr_LpiReg1Ch, 0x0c92);
        /* Enable "lpi_quit_waitafesigdet_en" for LPI link down issue */
        set_gphy_reg_cl22(phy_base + port, RgAddr_LpiReg1Dh, 0x0001);
        /* change CL22page to Reg(0x0) */
        set_gphy_reg_cl22(phy_base + port, 0x1f, CL22_Page_Reg);

        /* GPHY Rx low pass filter */
        set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0xc7, 0xd000);

        /* patch for RX ADC offset */
        set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0x171, 0x6f);

        data = get_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0x39);
        data = data & ~(1 << 14);
        set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0x39 , data);

        data = get_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC2, 0x107);
        data = data & ~(1 << 12);
        set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC2, 0x107, data);

        set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0x3d , 0xc00);

        /* patch for echo cancellation */
        if (hwtrap & HW_TRAP_SURGE_BIT)
        {
            set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0xe7 , 0x6666);
        }
        else
        {
            set_gphy_reg_cl45(phy_base + port, MMD_DEV_VSPEC1, 0xe7 , 0x4444);
        }
    }
}
#endif


