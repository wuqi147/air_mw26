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

/* FILE NAME:   pp_def.h
 * PURPOSE:
 *      Define the chip peripheral references.
 * NOTES:
 */

#ifndef PP_DEF_H
#define PP_DEF_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING CONSTANT DECLARATIONS
 */
#ifndef FALSE
#define FALSE               0
#endif

#ifndef TRUE
#define TRUE                1
#endif

#ifndef NULL
#define NULL                (void *)0
#endif

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif	/* End of BIT */

/* bits range: for example BITS(16,23) = 0xFF0000*/
#ifndef BITS
#define BITS(m, n)   (~(BIT(m) - 1) & ((BIT(n) - 1) | BIT(n)))
#endif	/* End of BITS */

/* bits range: for example BITS_RANGE(16,4) = 0x0F0000*/
#ifndef BITS_RANGE
#define BITS_RANGE(offset, range)           BITS((offset), ((offset)+(range)-1))
#endif	/* End of BITS_RANGE */

/* bits offset right: for example BITS_OFF_R(0x1234, 8, 4) = 0x2 */
#ifndef BITS_OFF_R
#define BITS_OFF_R(val, offset, range)      (((val) >> offset) & (BITS(0, (range) - 1)))
#endif	/* End of BITS_OFF_R */

/* bits offset left: for example BITS_OFF_L(0x1234, 8, 4) = 0x400 */
#ifndef BITS_OFF_L
#define BITS_OFF_L(val, offset, range)      (((val) & (BITS(0, (range) - 1))) << (offset))
#endif	/* End of BITS_OFF_L */

/* CHIP SCU Base */
#define REG_CHIP_SCU_BASE_ADDRESS     (0x10000000)

/* CPU ctrl infra Register Base */
#define REG_CPU_CTRL_INFRA_ADDRESS    (0x10002000)

#define CPU_INTERRUPT_TRIGGER_TYPE    (REG_CPU_CTRL_INFRA_ADDRESS + 0x0054)

#define CPU_INTR_TRIG_TYPE_LENG       (1)

/* Reset Control Register */
#define CHIPSCU_SCREG_WF0             (0x10005010)
#define CHIPSCU_SYS_CTRL1             (0x10005020)
#define CHIPSCU_RST_CTRL1             (0x100050C0)

#define SYS_SOFT_RESET_DECT_BIT       (30)
#define SYSTEM_SOFTWARE_RESET         (1)

/* I2C Register Base */
#define REG_I2C0_BASE_ADDRESS         (0x10008000)
#define REG_I2C1_BASE_ADDRESS         (0x10022000)

#define I2C_BASE                      0x0
/* SM0CTL0_REG: Serial interface master 0 control 0 register. */
#define RW_SM0_CTRL0_REG              ((unsigned int)(I2C_BASE + 0x0040))
/* SM0CTL1_REG: Serial interface master 0 control 1 register. */
#define RW_SM0_CTRL1_REG              ((unsigned int)(I2C_BASE + 0x0044))
/*  SM0D0_REG: Serial interface master 0 data port 0 register. */
#define RW_SM0_D0_REG                 ((unsigned int)(I2C_BASE + 0x0050))
/* SM0D1_REG: Serial interface master 0 data port 1 register. */
#define RW_SM0_D1_REG                 ((unsigned int)(I2C_BASE + 0x0054))
/* PINTEN_REG: Peripheral interrupt enable register. */
#define RW_PINTEN_REG                 ((unsigned int)(I2C_BASE + 0x005C))
#define I2C_M1_INTEN                  ((unsigned int)1 << 1)
#define I2C_M0_INTEN                  ((unsigned int)1 << 0)
/* PINTST_REG: Peripheral interrupt status register. */
#define RW_PINTST_REG                 ((unsigned int)(I2C_BASE + 0x0060))
#define I2C_M1_INTST                  ((unsigned int)1 << 1)
#define I2C_M0_INTST                  ((unsigned int)1 << 0)
/* PINTCLR_REG: Peripheral interrupt clear register. */
#define RW_PINTCLR_REG                ((unsigned int)(I2C_BASE + 0x0064))
#define I2C_M1_INTCLR                 ((unsigned int)1 << 1)
#define I2C_M0_INTCLR                 ((unsigned int)1 << 0)

/* ARL Register Base */
#define REG_APB_BASE_ADDRESS          (0x1000A000)

/* Register GROUP TYPE 1 */
#define GPIOCTRL                      ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0300))
#define GPIOCTRL1                     ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0320))
#define GPIOCTRL2                     ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0360))
#define GPIOCTRL3                     ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0364))

#define GPIO_CTRL_OFFSET              (2)
#define GPIO_CTRL_LENGTH              (2)

#define GPIOINTEDG                    ((unsigned int)(REG_APB_BASE_ADDRESS + 0x030C))
#define GPIOINTEDG1                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0380))
#define GPIOINTEDG2                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0384))
#define GPIOINTEDG3                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0388))

#define GPIO_EDGE_INT_CTRL_OFFSET     (2)
#define GPIO_EDGE_INT_CTRL_LENGTH     (2)

#define GPIOINTLVL                    ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0310))
#define GPIOINTLVL1                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x038C))
#define GPIOINTLVL2                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0390))
#define GPIOINTLVL3                   ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0394))

#define GPIO_LEVEL_INT_CTRL_OFFSET    (2)
#define GPIO_LEVEL_INT_CTRL_LENGTH    (2)


/* Register GROUP TYPE 2 */
#define GPIOOE                        ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0314))
#define GPIOOE1                       ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0378))

#define GPIO_OUTPUT_ENABLE_LENGTH     (1)

#define GPIODATA                      ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0304))
#define GPIODATA1                     ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0370))

#define GPIO_DATA_LENGTH              (1)

#define GPIOINT                       ((unsigned int)(REG_APB_BASE_ADDRESS + 0x0308))
#define GPIOINT1                      ((unsigned int)(REG_APB_BASE_ADDRESS + 0x037C))

#define GPIO_INT_LENGTH               (1)

/* DATA TYPE DECLARATIONS
 */
enum
{
    E_OK = 0,
    E_BAD_PARAMETER,
    E_ENTRY_NOT_FOUND,
    E_NOT_INITED,
    E_LAST
};

#endif  /* PP_DEF_H */

