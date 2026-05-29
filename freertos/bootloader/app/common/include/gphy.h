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

/* FILE NAME:  gphy.h
 * PURPOSE:
 *      It provides GPHY_H module API.
 * NOTES:
 */

#ifndef GPHY_H
#define GPHY_H

#if defined(AIR_SUPPORT_ACE)
/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
*/
typedef struct
{
    unsigned short DATA_Lo;
    unsigned char  DATA_Hi;
}TR_DATA_T;

/* CL22 Reg Support Page Select */
#define RgAddr_Reg1Fh               (0x1f)
#define CL22_Page_Reg               (0x0000)
#define CL22_Page_ExtReg            (0x0001)
#define CL22_Page_MiscReg           (0x0002)
#define CL22_Page_LpiReg            (0x0003)
#define CL22_Page_tReg              (0x02A3)
#define CL22_Page_TrReg             (0x52B5)

/* TokenRing Reg Access */
#define TrReg_PKT_XMT_STA           (0x8000)
#define TrReg_WR                    (0x8000)
#define TrReg_RD                    (0xA000)

#define RgAddr_dev1Eh_reg120h       (0x0120)
#define RgAddr_dev1Eh_reg122h       (0x0122)
#define RgAddr_dev1Eh_reg144h       (0x0144)
#define RgAddr_dev1Eh_reg14Ah       (0x014a)
#define RgAddr_dev1Eh_reg19Bh       (0x019b)
#define RgAddr_dev1Eh_reg234h       (0x0234)
#define RgAddr_dev1Eh_reg238h       (0x0238)
#define RgAddr_dev1Eh_reg239h       (0x0239)
#define RgAddr_dev1Fh_reg268h       (0x0268)
#define RgAddr_dev1Eh_reg323h       (0x0323)
#define RgAddr_dev1Eh_reg324h       (0x0324)
#define RgAddr_dev1Eh_reg326h       (0x0326)

#define RgAddr_dev1Eh_reg20Eh       (0x020e)
#define DSP_FRE_RP_FSM_EN           (1 << 4)
#define DSP_FRE_DW_AUTO_INC         (1 << 2)
#define DSP_FRE_WR_EN               (1 << 1)
#define DSP_FRE_SW_RST              (1 << 0)

#define RgAddr_dev1Eh_reg2D1h       (0x02d1)
#define RG_LPI_VCO_EEE_STGO_EN      (1 << 10)
#define RG_LPI_TR_READY             (1 << 9)
#define RG_LPI_SKIP_SD_SLV_TR       (1 << 8)
#define VCO_SLICER_THRES_H          (0x33)

#define RgAddr_dev1Fh               (0x1f)
#define RgAddr_LpiReg1Ch            (0x1c)
#define RgAddr_LpiReg1Dh            (0x1d)
#define RgAddr_TrReg10h             (0x10)
#define RgAddr_TrReg11h             (0x11)
#define RgAddr_TrReg12h             (0x12)

#define MII_BMCR                    (0)
#define BMCR_PDOWN                  (0x0800)

/* CL45 Reg Support DEVID */
#define MMD_DEV_ANEG                (0x07)
#define MMD_DEV_VSPEC1              (0x1E)
#define MMD_DEV_VSPEC2              (0x1F)

#define PHY_BASE_ADDR_MASK          (0x0018)
#define PHY_BASE_ADDR_SHIFT         (9)

#define HW_TRAP_READ_REG            (0x100000a8)
#define HW_TRAP_SURGE_BIT           (1 << 14)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */

unsigned short get_gphy_reg_cl22(unsigned char phyad, unsigned char reg);
void set_gphy_reg_cl22(unsigned char phyad, unsigned char reg, unsigned short value);
unsigned short get_gphy_reg_cl45(unsigned char prtid, unsigned char devid, unsigned short reg);
void set_gphy_reg_cl45(unsigned char prtid, unsigned char devid, unsigned short reg, unsigned short value);
void TR_RegWr(unsigned short phyadd, unsigned short tr_reg_addr, unsigned int tr_data);
void gphy_init(void);
#endif
#endif  /* End of GPHY_H */
