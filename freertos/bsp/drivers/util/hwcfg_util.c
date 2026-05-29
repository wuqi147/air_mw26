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

/* FILE NAME:  hwcfg_util.c
 * PURPOSE:
 *  Implement hardware utilities application.
 *
 * NOTES:
 *
 */

/* INCLUDE FILE DECLARTIONS
*/
#include "hwcfg_util.h"
#include "util.h"

/* NAMING CONSTANT DECLARATIONS
*/

/* MACRO FUNCTION DECLARATIONS
 */
#define PHY_ACCESS_COMPLETE(x)    do                            \
    {                                                           \
        while(x)                                                \
        {                                                       \
            if(0 == (io_read32(PHY_IAC) & CSR_PHY_ACS_ST_MASK)) \
            {                                                   \
                break;                                          \
            }                                                   \
            x --;                                               \
        }                                                       \
    }while(0)
/* LOCAL SUBPROGRAM SPECIFICATIONS
*/

/* EXPORTED SUBPROGRAM BODIES
 */
void mdio_writeCL22(
    const unsigned char  phy_addr,
    const unsigned char  reg,
    const unsigned short data)
{
    unsigned int tmout = PHY_ACS_TIMEOUT;
    unsigned int wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg) | SET_MDIO_DATA((unsigned short)data)\
                   | (SET_MDIO_CMD_CL22W) | (SET_MDIO_ST_CL22);

    PHY_ACCESS_COMPLETE(tmout);
    io_write32(PHY_IAC, wdata);
    wdata |= SET_MDIO_ACS_START;
    io_write32(PHY_IAC, wdata);
}

void mdio_readCL22(
    const unsigned char  phy_addr,
    const unsigned char  reg,
          unsigned short *ptr_data)
{
    unsigned int tmout = PHY_ACS_TIMEOUT;
    unsigned short tempValue = 0;
    unsigned int wdata = SET_MDIO_PHY(phy_addr) | SET_MDIO_REG(reg) \
                   | SET_MDIO_CMD_CL22R | SET_MDIO_ST_CL22;

    PHY_ACCESS_COMPLETE(tmout);
    io_write32(PHY_IAC, wdata);
    wdata |= SET_MDIO_ACS_START;
    io_write32(PHY_IAC, wdata);
    delay1ms(5);
    PHY_ACCESS_COMPLETE(tmout);
    tempValue = io_read32(PHY_IAD);
    *ptr_data = GET_MDIO_DATA(tempValue);
}

unsigned int en8801s_pbus_rd(unsigned char pid, unsigned int pbus_address)
{
    unsigned short data_high, data_low;
    mdio_writeCL22(pid, 0x1f, (unsigned short)(pbus_address >> 6));
    mdio_readCL22(pid, (unsigned short)((pbus_address >> 2) & 0x0f), &data_low);
    mdio_readCL22(pid, 0x10, &data_high);
    return((data_high << 16) | data_low);
}

void en8801s_pbus_wr(unsigned char pid, unsigned int pbus_address, unsigned int pbus_data)
{
    mdio_writeCL22(pid, 0x1f, (unsigned short)(pbus_address >> 6));
    mdio_writeCL22(pid, (unsigned short)((pbus_address >> 2) & 0x0f), (unsigned short)(pbus_data & 0xffff));
    mdio_writeCL22(pid, 0x10, (unsigned short)(pbus_data >> 16));
    return;
}

