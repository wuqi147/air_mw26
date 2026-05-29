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

/* FILE NAME:  hal_sco_i2c.h
 * PURPOSE:
 * NOTES:
 */

#ifndef HAL_I2C_H
#define HAL_I2C_H

/* INCLUDE FILE DECLARATIONS
 */
#include <air_i2c.h>

/* NAMING CONSTANT DECLARATIONS
 */
#define I2C_MAX_NUM_BUS     2

#define NULL_HANDLE NULL
#define I2C_WAIT_FOREVER    0xffff

#define I2C_BUS_ON          1
#define I2C_BUS_OFF         0

#define CLK_100K            199
#define CLK_400K            49
#define CLK_1M              19

/* I2C has 2 data port register. Total 8 bytes, so page read/write unit is 8 bytes. */
#define I2C_PAGE_SIZE       ((unsigned int)8)

#define  I2C_OK             (0)
#define  I2C_DEGLITCHLEVEL  (0x03)

/* SCL Stretch Enable bit */
#define SCL_STRETCH                   ((unsigned int)1 << 0)
/* SM0EN: Serial interface master 0 enable. */
#define SM0EN                         ((unsigned int)1 << 1)
/* SDA_STATE:  SDA Value (1:high, 0:low)    */
#define SDA_STATE                     ((unsigned int)1 << 2)
/* SCL_STATE:  SDA Value (1:high, 0:low) */
#define SCL_STATE                     ((unsigned int)1 << 3)
/* CS_STATUS:  SCL stretching status  */
#define CS_STATUS                     ((unsigned int)1 << 4)

#define DEGLITCH_EN                   ((unsigned int)1 << 5)

typedef enum
{
    I2C_CTL_NORMAL    =  0,       /* Normal I2C operation         */
    I2C_CTL_NO_START  =  1 << 1,  /* Do not generate start signal */
    I2C_CTL_NO_STOP   =  1 << 2,  /* Do not generate stop signal  */
} I2C_BUS_CONTROL_T;

#define IO_VAL 0x0
#define RW_PERMISC_REG                ((unsigned int)(IO_VAL+0x0000))

#define SIF1_SW_RST                   ((unsigned int)1<<29 )
#define SIF0_SW_RST                   ((unsigned int)1<<28 )

#define SIF0_SW_RST_MSK               (0x10000000)
#define SIF1_SW_RST_MSK               (0x20000000)

typedef enum
{
    I2C_LOG_NO = 0,
    I2C_LOG_ERROR,
    I2C_LOG_WARNING,
    I2C_LOG_TRACE,
    I2C_LOG_DBG
} I2C_LOG_LEVEL;

/* MACRO FUNCTION DECLARATIONS
 */
/* deglitch set */
#define DEGLITCH_SET(x)               ((((unsigned int)(x)) & 0x00FF) << 8)
#define DEGLITCH_SET_MSK              (0x00FF<< 8)

/* CLK_DIV: Parameter of divider to divide 27M for SCL.  */
#define ODRAIN                        ((unsigned int)1 << 31)
#define CLK_DIV(x)                    ((((unsigned int)(x)) & 0x0FFF) << 16)
#define CLK_DIV_MSK                   (0x0FFF << 16)

/* TRI: Trigger serial interface. Read back as serial interface busy.  */
#define TRI                           ((unsigned int)1 << 0)
#define TRI_MSK                       ((unsigned int)1 << 0)
/* PGLEN: Page length of sequential read/write. The maximum is 8 bytes. Set 0 as 1 bytes.  */
#define PGLEN(x)                      ((((unsigned int)(x)) & 0x0007) << 8)
#define PGLEN_MSK                     (0x0700)

#define ITS                           ((unsigned int)1 << 0)
/* I2C_MODE: start ==>data==>stop, start==>data, data==>stop, only data  */
#define I2C_MODE(x)                   ((((unsigned int)(x)) & 0x0007) << 4)
                typedef enum {
                    MODE_START        = 0x1,
                    MODE_WR_DATA      = 0x2,
                    MODE_STOP         = 0x3,
                    MODE_RD_DATA_LAST = 0x4,
                    MODE_RD_DATA      = 0x5,
                } I2C_MODE_T;
#define I2C_MODE_MSK                  ((unsigned int)0x70)
/*  ACK:    Acknowledge bits. */
#define ACK(x)                        ((((unsigned int)(x)) & 0x00FF0000) >> 16)

/* #bytes of word address */
#define WORD_LEN(x)                   ((((unsigned int)(x)) & 0x3) << 6)
/* HD1T: tDH = 1T @ ACK, STOP1, CHANGE state.*/
#define HD1T                          ((unsigned int)1 << 4)
/* WR: Write cycle. */
#define WR                            ((unsigned int)1 << 1)
/* CURAR: Current address read or random read. */
#define CURAR                         ((unsigned int)1 << 2)

#define IS_BUS_FREE(x,y)              ((x == 1) && (y == 1))

#define ENABLE_I2C_DEBUG              (0)
#if ENABLE_I2C_DEBUG
#define LOG(Level,fmt,args...)          \
    do{                                 \
        if (Level <= I2CDebugLevel)     \
        {                               \
            printf("%s:"fmt,__func__,##args);\
        }                               \
    }while(0)
#else
#define LOG(...)
#endif

/* DATA TYPE DECLARATIONS
 */
struct i2cBusMap {
    unsigned int onOff;
    unsigned int baseAddr;
};

typedef struct I2C_BUS_STATUS_S
{
    unsigned char I2C_BUS_SDA;
    unsigned char I2C_BUS_SCL;
} I2C_BUS_STATUS_T;

typedef struct I2C_STRUCT_S
{
    unsigned int Unit;
    unsigned int i2cClock;
    unsigned int i2cBusBaseAddr;
} I2C_STRUCT_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */
/* FUNCTION NAME: hal_sco_i2c_open
 * PURPOSE:
 *      Initial I2C register.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETERW
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_open(
    const unsigned int unit,
    const unsigned int channel);

/* FUNCTION NAME: hal_sco_i2c_setClock
 * PURPOSE:
 *      Set clock speed
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      clock           --  i2c clock speed
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETER
 *      E_NOT_INITED
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_setClock(
    const unsigned int unit,
    const unsigned int channel,
    const unsigned int clock);

/* FUNCTION NAME: hal_sco_i2c_getClock
 * PURPOSE:
 *      Get clock speed
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *
 * OUTPUT:
 *      *ptr_clock      --  i2c clock speed
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETER
 *      E_NOT_INITED
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_getClock(
    const unsigned int unit,
    const unsigned int channel,
    unsigned int *ptr_clock);

/* FUNCTION NAME: hal_sco_i2c_read
 * PURPOSE:
 *      Read data block from a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      ptr_i2c_parm    --  I2C parameter
 *      sid             --  Slave device ID
 *      sch             --  Slave device I2C master channel
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETER
 *      E_NOT_INITED
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_read(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_I2C_parm,
    const unsigned int sid,
    const unsigned int sch);

/* FUNCTION NAME: hal_sco_i2c_write
 * PURPOSE:
 *      Write data block to a specified channel
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      ptr_i2c_parm    --  I2C parameter
 *      sid             --  Slave device ID
 *      sch             --  Slave device I2C master channel
 *
 * OUTPUT:
 *      ptr_I2C_parm    --  I2C parameter
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETER
 *      E_NOT_INITED
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_write(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_I2C_parm,
    const unsigned int sid,
    const unsigned int sch);

#endif  /* End of HAL_I2C_H */
