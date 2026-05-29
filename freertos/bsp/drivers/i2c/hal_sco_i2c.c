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

/* FILE NAME:  hal_sco_i2c.c
 * PURPOSE:
 * It provides hal i2c module API.
 *
 * NOTES:
 *
 */

/*****************************************************************************
 * INCLUDE FILE DECLARATIONS
 *****************************************************************************
 */

#include <pp_def.h>
#include "hal_sco_i2c.h"
#include <util.h>


/* DIAG_SET_MODULE_INFO(AIR_MODULE_I2C, "hal_sco_i2c.c"); */
/*****************************************************************************
 * NAMING CONSTANT DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * MACRO VLAUE DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * MACRO FUNCTION DECLARATIONS
 *****************************************************************************
 */


/*****************************************************************************
 * DATA TYPE DECLARATIONS
 *****************************************************************************
 */

/*****************************************************************************
 * GLOBAL VARIABLE DECLARATIONS
 *****************************************************************************
 */
struct i2cBusMap i2cBusMap[I2C_MAX_NUM_BUS] =
{
     /* onOff */    /* baseAddr */
    {I2C_BUS_ON,    REG_I2C0_BASE_ADDRESS},
    {I2C_BUS_ON,    REG_I2C1_BASE_ADDRESS},
};
/*****************************************************************************
 * STATIC VARIABLE DECLARATIONS
 ******************************************************************************
 */
static int I2CDebugLevel = I2C_LOG_DBG;
static I2C_STRUCT_T* i2cStruct[I2C_MAX_NUM_BUS] = { NULL };
static I2C_STRUCT_T bus_control[I2C_MAX_NUM_BUS];

/*****************************************************************************
 * LOCAL SUBPROGRAM DECLARATIONS
 *****************************************************************************
 */
static void _ReverseBytesOrder(unsigned char *ptr_str, unsigned int ptr_size)
{
    unsigned int  half_len = 0;
    unsigned int  idx = 0;
    unsigned char temp;

    half_len = ptr_size / 2;
    if (half_len != 0)
    {
        ptr_size--;
        for (idx = 0; idx < half_len; idx++)
        {
            temp = *(ptr_str + idx);
            *(ptr_str + idx) = *(ptr_str + ptr_size - idx);
            *(ptr_str + ptr_size - idx) = temp;
        }
    }
    return;
}

static void I2C_Init(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch);

unsigned int I2C_get_base_addr(unsigned char ChID)
{
    return i2cStruct[ChID]->i2cBusBaseAddr;
}

static void I2C_WRITE32(unsigned char unit, unsigned char ChID, unsigned int u4Offset, unsigned int u4Value, unsigned int sid, unsigned int sch)
{
    unsigned int RegAddr;
    unsigned int RegValue32 = u4Value;
    int rc = I2C_OK;
    AIR_I2C_Master_T i2c_parm;
    i2c_parm.deviceID = sid;
    i2c_parm.regAddrLen = 4;
    i2c_parm.dataLen = 4;
    i2c_parm.buffPtr = (unsigned char *)&RegValue32;
    i2c_parm.regAddr = (unsigned char *)&RegAddr;

    if (0 == sid)
    {
        RegAddr = I2C_get_base_addr(ChID) + u4Offset;
        io_write32(RegAddr, u4Value);
    }
    else
    {
        RegAddr = I2C_get_base_addr(sch) + u4Offset;
        rc = hal_sco_i2c_write(unit, ChID, &i2c_parm, 0, sch);
        if (E_OK != rc)
        {
            LOG(I2C_LOG_ERROR, "I2C write failed\n");
        }
    }
}

static unsigned int I2C_READ32(unsigned char unit, unsigned char ChID, unsigned int u4Offset, unsigned int sid, unsigned int sch)
{
    unsigned int RegAddr;
    unsigned int RegValue32;
    int rc = I2C_OK;
    AIR_I2C_Master_T i2c_parm;
    i2c_parm.deviceID = sid;
    i2c_parm.regAddrLen = 4;
    i2c_parm.dataLen = 4;
    i2c_parm.buffPtr = (unsigned char *)&RegValue32;
    i2c_parm.regAddr = (unsigned char *)&RegAddr;

    if (0 == sid)
    {
        RegAddr = I2C_get_base_addr(ChID) + u4Offset;
        RegValue32 = io_read32(RegAddr);
    }
    else
    {
        RegAddr = I2C_get_base_addr(sch) + u4Offset;
        rc = hal_sco_i2c_read(unit, ChID, &i2c_parm, 0, 0);
        if (E_OK != rc)
        {
            LOG(I2C_LOG_ERROR, "I2C read failed\n");
        }
    }
    return RegValue32;
}

static void I2C_WRITE32MSK(unsigned char unit, unsigned char ChID, unsigned int u4Offset, unsigned int u4Value, unsigned int u4Mask, unsigned int sid, unsigned int sch)
{
    unsigned int RegAddr;
    unsigned int RegValue32;
    int rc = I2C_OK;
    AIR_I2C_Master_T i2c_parm;
    i2c_parm.deviceID = sid;
    i2c_parm.regAddrLen = 4;
    i2c_parm.dataLen = 4;
    i2c_parm.buffPtr = (unsigned char *)&RegValue32;
    i2c_parm.regAddr = (unsigned char *)&RegAddr;

    if (0 == sid)
    {
        RegAddr = I2C_get_base_addr(ChID) + u4Offset;
        RegValue32 = io_read32(RegAddr);
    }
    else
    {
        RegAddr = I2C_get_base_addr(sch) + u4Offset;
        rc = hal_sco_i2c_read(unit, ChID, &i2c_parm, 0, 0);
        if (E_OK != rc)
        {
            LOG(I2C_LOG_ERROR, "I2C read failed\n");
        }
    }
    RegValue32 = (RegValue32 & ~(u4Mask)) | (u4Value & u4Mask);
    if (0 == sid)
    {
        io_write32(RegAddr, RegValue32);
    }
    else
    {
        rc = hal_sco_i2c_write(unit, ChID, &i2c_parm, 0, 0);
        if (E_OK != rc)
        {
            LOG(I2C_LOG_ERROR, "I2C write failed\n");
        }
    }
}

static void I2C_DisableAndClearInterrupt(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    I2C_WRITE32MSK(unit, ChID, RW_PINTEN_REG, 0, I2C_M0_INTEN, sid, sch);
    I2C_WRITE32MSK(unit, ChID, RW_PINTCLR_REG, I2C_M0_INTCLR, I2C_M0_INTCLR, sid, sch);

    I2C_WRITE32MSK(unit, ChID, RW_PINTEN_REG, 0, I2C_M1_INTEN, sid, sch);
    I2C_WRITE32MSK(unit, ChID, RW_PINTCLR_REG, I2C_M1_INTCLR, I2C_M1_INTCLR, sid, sch);
}

static void I2C_EnableAndClearInterrupt(unsigned char unit,unsigned char ChID, unsigned int sid, unsigned int sch)
{
    I2C_WRITE32MSK(unit, ChID, RW_PINTCLR_REG, I2C_M0_INTCLR, I2C_M0_INTCLR, sid, sch);
    I2C_WRITE32MSK(unit, ChID, RW_PINTEN_REG, I2C_M0_INTEN, I2C_M0_INTEN, sid, sch);
}

/* Fill control register */
static void I2C_Control(unsigned char unit, unsigned char ChID, unsigned int clock, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk;

    u4Reg = CLK_DIV(clock);

    u4Reg |= DEGLITCH_EN;
    u4Reg |= DEGLITCH_SET(I2C_DEGLITCHLEVEL);
    u4Msk = (unsigned int) (CLK_DIV_MSK | DEGLITCH_SET_MSK | ODRAIN | SCL_STRETCH | DEGLITCH_EN);

    I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL0_REG, u4Reg, u4Msk, sid, sch);
    i2cStruct[ChID]->i2cClock = clock;
}

/* Check I2C bus status */
static I2C_BUS_STATUS_T I2C_CheckBus (unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg;
    I2C_BUS_STATUS_T tStatus;

    memset((void *)(&tStatus), 0, sizeof(I2C_BUS_STATUS_T));
    u4Reg = I2C_READ32(unit, ChID, RW_SM0_CTRL0_REG, sid, sch);
    tStatus.I2C_BUS_SDA = (unsigned char) ((u4Reg & SDA_STATE) ? 1 : 0);
    tStatus.I2C_BUS_SCL = (unsigned char) ((u4Reg & SCL_STATE) ? 1 : 0);

    return tStatus;
};

/* Wait tirgger bit becomes 0 */
static signed int I2C_WaitHwDone(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg;
    unsigned int loopcount = 0;

    u4Reg = TRI;

    while (u4Reg & TRI)
    {
        u4Reg = I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch);
        if(++loopcount >= 0xffff)
        {
            LOG(I2C_LOG_DBG, "\r\nCTRL0_REG=%x", I2C_READ32(unit, ChID, RW_SM0_CTRL0_REG, sid, sch));
            LOG(I2C_LOG_DBG, "\r\nCTRL1_REG=%x", I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
            LOG(I2C_LOG_DBG, "\nPolling timeout!\n");
            return I2C_ERROR_TIMEOUT;
        }
    }

    return E_OK;
}

/* Send one stop bit */
static unsigned int I2C_PutStopBit(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk;

    LOG(I2C_LOG_DBG, "Stop !\n");
    I2C_EnableAndClearInterrupt(unit, ChID, sid, sch);
    u4Reg = TRI | I2C_MODE(MODE_STOP);
    u4Msk = TRI_MSK | I2C_MODE_MSK;
    I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL1_REG, u4Reg, u4Msk, sid, sch);
    return I2C_WaitHwDone(unit, ChID, sid, sch);
}
/* Send one start bit */
static unsigned int I2C_PutStartBit(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk;

    LOG(I2C_LOG_DBG, "Start !\n");
    I2C_EnableAndClearInterrupt(unit, ChID, sid, sch);
    u4Reg = TRI | I2C_MODE(MODE_START);
    u4Msk = TRI_MSK | I2C_MODE_MSK;
    I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL1_REG, u4Reg, u4Msk, sid, sch);
    return I2C_WaitHwDone(unit, ChID, sid, sch);
}

/* Send one data byte
   Return: 1 success, 0 fail, <0 error.
*/
static unsigned int I2C_PutByte(unsigned char unit, unsigned char ChID, unsigned char u1Data, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk, u4Ack;
    signed int  i4Ret = 0;

    I2C_EnableAndClearInterrupt(unit, ChID, sid, sch);
    /* fill data port */
    I2C_WRITE32(unit, ChID, RW_SM0_D0_REG, u1Data, sid, sch);
    /* fill control word */
    u4Reg = TRI | I2C_MODE(MODE_WR_DATA) | PGLEN(0);          /* PGLEN(0) = 1 byte */
    u4Msk = TRI_MSK | I2C_MODE_MSK | PGLEN_MSK;
    I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL1_REG, u4Reg, u4Msk, sid, sch);
    LOG(I2C_LOG_DBG, "\r\nCTRL0_REG=%x", I2C_READ32(unit, ChID, RW_SM0_CTRL0_REG, sid, sch));
    LOG(I2C_LOG_DBG, "\r\nCTRL1_REG=%x", I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
    LOG(I2C_LOG_DBG, "\r\nDATA0_REG=%x", I2C_READ32(unit, ChID, RW_SM0_D0_REG, sid, sch));
    /* wait for hardware */
    i4Ret = I2C_WaitHwDone(unit, ChID, sid, sch);
    if(i4Ret != I2C_OK)
    {
        LOG(I2C_LOG_DBG, "I2C put byte: sema time out \n");
        return i4Ret;
    }
    u4Ack = ACK(I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
    LOG(I2C_LOG_DBG, "CTRL1_REG=%x \n", I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
    return ((u4Ack & 0x01) ? 1 : 0);
}

static signed int I2C_GetMultipleBytes(unsigned char unit, unsigned char ChID, unsigned char *pu1Data, signed int i4Bytes, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk, u4D0, u4D1;
    signed int  i4Ret, i4RetTimeout, i, j, i4SplitBytes, i4TotalLoops;

    i4Ret = 0;
    i4RetTimeout = 0;
    i4TotalLoops = (i4Bytes + I2C_PAGE_SIZE - 1) / I2C_PAGE_SIZE;
    LOG(I2C_LOG_DBG, "i4TotalLoops=%d ", i4TotalLoops);
    for (i = 0; i < i4TotalLoops; i++)
    {
        /* Last <= I2C_PAGE_SIZE bytes */
        if (i == (i4TotalLoops - 1))
        {
            i4SplitBytes = i4Bytes % I2C_PAGE_SIZE;
        }
        else
        {
            i4SplitBytes = (signed int) I2C_PAGE_SIZE;
        }
        if (i4SplitBytes == 0)
        {
            i4SplitBytes = (signed int) I2C_PAGE_SIZE;
        }
        if (i == (i4TotalLoops - 1))
        {
            u4Reg = TRI | I2C_MODE(MODE_RD_DATA_LAST) | PGLEN(i4SplitBytes - 1);
        }
        else
        {
            u4Reg = TRI | I2C_MODE(MODE_RD_DATA) | PGLEN(i4SplitBytes - 1);
        }
        I2C_EnableAndClearInterrupt(unit, ChID, sid, sch);
        /* fill control word */
        u4Msk = TRI_MSK | I2C_MODE_MSK | PGLEN_MSK;
        I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL1_REG, u4Reg, u4Msk, sid, sch);
        /* wait for hardware */
        i4RetTimeout = I2C_WaitHwDone(unit, ChID, sid, sch);
        if(i4RetTimeout != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "I2C get mutiple byte: sema time out ");
            return i4RetTimeout;
        }
        u4D0 = I2C_READ32(unit, ChID, RW_SM0_D0_REG, sid, sch);
        u4D1 = I2C_READ32(unit, ChID, RW_SM0_D1_REG, sid, sch);
        LOG(I2C_LOG_DBG, "index=%d,u4D0=%x,u4D1=%x,i4SplitBytes=%d\n",i ,u4D0 ,u4D1, i4SplitBytes);
        LOG(I2C_LOG_DBG, "CTRL0_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_CTRL0_REG, sid, sch));
        LOG(I2C_LOG_DBG, "CTRL1_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
        /* save to buffer */
        for (j = 0; j < ((i4SplitBytes > 4) ? 4 : i4SplitBytes); j++)
        {
            *pu1Data = (unsigned char) (u4D0 & 0xFF);
            u4D0 >>= 8;
            pu1Data++;
            i4Ret++;
        }
        if (i4SplitBytes > 4)
        {
            for (j = 0; j < (i4SplitBytes - 4); j++)
            {
                *pu1Data = (unsigned char) (u4D1 & 0xFF);
                u4D1 >>= 8;
                pu1Data++;
                i4Ret++;
            }
        }
    }

    return i4Ret;
}

/* Send multiple data bytes
 * Return: how many bytes were written, 0 fail, <1 error
 */
static signed int I2C_PutMultipleBytes(unsigned char unit, unsigned char ChID, unsigned char *pu1Data, signed int i4Bytes, unsigned int sid, unsigned int sch)
{
    unsigned int u4Reg, u4Msk, u4Ack, u4D0, u4D1, AckAllBits, AckCheckBit;
    signed int  i4Ret, i4RetTimeout, i, j, i4SplitBytes, i4TotalLoops;

    i4Ret = 0;
    i4RetTimeout = 0;
    i4TotalLoops = (i4Bytes + I2C_PAGE_SIZE - 1) / I2C_PAGE_SIZE;
    LOG(I2C_LOG_DBG, "\ni4TotalLoops=%d\n", i4TotalLoops);
    for (i = 0; i < i4TotalLoops; i++)
    {
        /* Last <= I2C_PAGE_SIZE bytes */
        if (i == (i4TotalLoops - 1))
        {
            i4SplitBytes = i4Bytes % I2C_PAGE_SIZE;
        }
        else
        {
            i4SplitBytes = (signed int) I2C_PAGE_SIZE;
        }
        if (i4SplitBytes == 0)
        {
            i4SplitBytes = (signed int) I2C_PAGE_SIZE;
        }
        u4D0 = 0;
        u4D1 = 0;
        /* setup data bytes */
        for (j = 0; j < ((i4SplitBytes > 4) ? 4 : i4SplitBytes); j++)
        {
            u4D0 |= ((*(pu1Data + j) << (8 * j)) & (0xFF << (8 * j)));
        }
        if (i4SplitBytes > 4)
        {
            for (j = 0; j < (i4SplitBytes - 4); j++)
            {
                u4D1 |= ((*(pu1Data + 4 + j) << (8 * j)) & (0xFF << (8 * j)));
            }
        }
        pu1Data += i4SplitBytes;
        I2C_EnableAndClearInterrupt(unit, ChID, sid, sch);
        LOG(I2C_LOG_DBG, "index=%d,u4D0=%x,u4D1=%x,i4SplitBytes=%d\n", i, u4D0, u4D1, i4SplitBytes);
        /* fill data port */
        I2C_WRITE32(unit, ChID, RW_SM0_D0_REG, u4D0, sid, sch);
        I2C_WRITE32(unit, ChID, RW_SM0_D1_REG, u4D1, sid, sch);
        /* fill control word, use old mode */
        u4Reg = TRI | I2C_MODE(MODE_WR_DATA) | PGLEN(i4SplitBytes - 1);
        u4Msk = TRI_MSK | I2C_MODE_MSK | PGLEN_MSK;
        I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL1_REG, u4Reg, u4Msk, sid, sch);

        LOG(I2C_LOG_DBG, "CTRL0_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_CTRL0_REG, sid, sch));
        LOG(I2C_LOG_DBG, "CTRL1_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
        LOG(I2C_LOG_DBG, "DATA0_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_D0_REG, sid, sch));
        LOG(I2C_LOG_DBG, "DATA1_REG=%x\n", I2C_READ32(unit, ChID, RW_SM0_D1_REG, sid, sch));

        /* wait for hardware */
        i4RetTimeout = I2C_WaitHwDone(unit, ChID, sid, sch);
        if(i4RetTimeout != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "I2C put mutiple byte: sema time out\n");
            return i4RetTimeout;
        }
        u4Ack = ACK(I2C_READ32(unit, ChID, RW_SM0_CTRL1_REG, sid, sch));
        u4Ack &= 0xFF;
        /* count how many acks */
        AckAllBits = u4Ack;
        if(AckAllBits != 0)
        {
            AckCheckBit = 1;
            for(u4Ack = 1; u4Ack < 9; u4Ack++)
            {
                if(AckAllBits == AckCheckBit) break;
                AckCheckBit = (AckCheckBit << 1) | 1;
            }
            if(u4Ack == 9)
            {
                LOG(I2C_LOG_DBG, "I2C put mutiple byte: Abnormal device ack sequence 0x%x.\n ", AckAllBits);
                u4Ack = 0;
            }
        }
        i4Ret += u4Ack;
        if (u4Ack == 0)
        {
            return i4Ret;
        }
    }
    return i4Ret;
}

static signed int I2C_OptWrite(unsigned char unit, unsigned char ChID, unsigned char u1DevAddr, unsigned char *pu1DataBuf,
                                    unsigned int u4DataLen, I2C_BUS_CONTROL_T bus_ctrl, unsigned int sid, unsigned int sch)
{
    signed int i4Cnt = 0, i4Ret = 0;

    if (!(bus_ctrl & I2C_CTL_NO_START))
    {
        i4Ret = I2C_PutStartBit(unit, ChID, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Start Bit error, %d.\n", i4Ret);
            return i4Ret;
        }
        LOG(I2C_LOG_DBG, "Put slave id !\n");
        if (I2C_PutByte(unit, ChID, u1DevAddr, sid, sch) != 1)
        {
            LOG(I2C_LOG_DBG, "slave id 0x%x does not ack.\n", u1DevAddr);
            if (!(bus_ctrl & I2C_CTL_NO_STOP))
            {
                i4Ret = I2C_PutStopBit(unit, ChID, sid, sch);
                if (i4Ret != I2C_OK)
                {
                    LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
                    return i4Ret;
                }
            }
            return I2C_ERROR_NODEV;
        }
    }
    LOG(I2C_LOG_DBG, "send data !\n");
    i4Cnt = I2C_PutMultipleBytes(unit, ChID, pu1DataBuf, (signed int) u4DataLen, sid, sch);
    if (i4Cnt != (signed int) u4DataLen)
    {
        LOG(I2C_LOG_DBG, "only %d byte were written, expected %d.\n", i4Cnt, u4DataLen);
    }
    if (!(bus_ctrl & I2C_CTL_NO_STOP))
    {
        i4Ret = I2C_PutStopBit(unit, ChID, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n",i4Ret);
            return i4Ret;
        }
    }
    return i4Cnt;
}

static signed int I2C_OptRead(unsigned char unit, unsigned char ChID, unsigned char u1DevAddr, unsigned char *pu1DataBuf,
                                    unsigned int u4DataLen, I2C_BUS_CONTROL_T bus_ctrl, unsigned int sid, unsigned int sch)
{
    signed int i4Cnt = 0, i4Ret = 0;

    if (!(bus_ctrl & I2C_CTL_NO_START))
    {
        i4Ret = I2C_PutStartBit(unit, ChID, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Start Bit error, %d.\n", i4Ret);
            return i4Ret;
        }
        LOG(I2C_LOG_DBG, "Put slave id !\n");
        if (I2C_PutByte(unit, ChID, u1DevAddr, sid, sch) != 1)
        {
            LOG(I2C_LOG_DBG, "slave id 0x%x does not ack.\n", u1DevAddr);
            if (!(bus_ctrl & I2C_CTL_NO_STOP))
            {
                i4Ret = I2C_PutStopBit(unit, ChID, sid, sch);
                if ( i4Ret != I2C_OK)
                {
                    LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
                    return i4Ret;
                }
            }
            return I2C_ERROR_NODEV;
        }
    }
    LOG(I2C_LOG_DBG, "get data !\n");
    i4Cnt = I2C_GetMultipleBytes(unit, ChID, pu1DataBuf, (signed int)u4DataLen, sid, sch);
    if (i4Cnt != (signed int) u4DataLen)
    {
        LOG(I2C_LOG_DBG, "only %d byte were read, expected %d.\n", i4Cnt, u4DataLen);
    }
    if (!(bus_ctrl & I2C_CTL_NO_STOP))
    {
        i4Ret = I2C_PutStopBit(unit, ChID, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n",i4Ret);
            return i4Ret;
        }
    }

    return i4Cnt;
}

static void I2C_Init(unsigned char unit, unsigned char ChID, unsigned int sid, unsigned int sch)
{
    I2C_DisableAndClearInterrupt(unit, ChID, sid, sch);
    /* Enable I2C masters */
    I2C_WRITE32MSK(unit, ChID, RW_SM0_CTRL0_REG, SM0EN, SM0EN, sid, sch);
    I2C_Control(unit, ChID, CLK_100K, sid, sch);
}

/* FUNCTION NAME: hal_sco_i2c_open
 * PURPOSE:
 *      Initial i2c register.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of i2c channel
 *
 * OUTPUT:
 *      None
 *
 * RETURN:
 *      E_OK
 *      E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
int
hal_sco_i2c_open(
    const unsigned int unit,
    const unsigned int channel)
{
    if(channel >= I2C_MAX_NUM_BUS)
    {
        return E_BAD_PARAMETER;
    }
    if(i2cBusMap[channel].onOff == I2C_BUS_OFF)  /* config disabled or for slave */
    {
        return E_BAD_PARAMETER;
    }
    i2cStruct[channel] = &bus_control[channel];
    i2cStruct[channel]->Unit = unit;
    i2cStruct[channel]->i2cClock = CLK_100K,
    i2cStruct[channel]->i2cBusBaseAddr = i2cBusMap[channel].baseAddr;
    I2C_Init(unit, channel, 0, 0);
    return E_OK;
}

/* FUNCTION NAME: hal_sco_i2c_setClock
 * PURPOSE:
 *      Set clock speed
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of i2c channel
 *      clock     --  i2c clock speed
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
    const unsigned int clock)
{
    unsigned int  ClkDivTbl[]= { CLK_100K, CLK_400K, CLK_1M };    /* clock divider */

    if(channel >= I2C_MAX_NUM_BUS)
    {
        return E_BAD_PARAMETER;
    }
    if(i2cStruct[channel] == NULL)
    {
        return E_NOT_INITED;
    }
    if(clock > I2C_Clock_1M)
    {
        return E_BAD_PARAMETER;
    }
    I2C_Control(unit, channel, ClkDivTbl[clock], 0, 0);
    return E_OK;
}

/* FUNCTION NAME: hal_sco_i2c_getClock
 * PURPOSE:
 *      Get clock speed
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of i2c channel
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
    unsigned int *ptr_clock)
{
    if(channel >= I2C_MAX_NUM_BUS)
    {
        return E_BAD_PARAMETER;
    }
    if(i2cStruct[channel] == NULL)
    {
        return E_NOT_INITED;
    }
    if(i2cStruct[channel]->i2cClock == CLK_100K)
    {
        *ptr_clock = I2C_Clock_100K;
    }
    else if(i2cStruct[channel]->i2cClock == CLK_400K)
    {
        *ptr_clock = I2C_Clock_400K;
    }
    else if(i2cStruct[channel]->i2cClock == CLK_1M)
    {
        *ptr_clock = I2C_Clock_1M;
    }
    else
    {
        return E_BAD_PARAMETER;
    }
    return E_OK;
}

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
 * OUTPUT:
 *      ptr_i2c_parm    --  I2C parameter
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
    AIR_I2C_Master_T *ptr_i2c_parm,
    const unsigned int sid,
    const unsigned int sch)
{
    I2C_BUS_STATUS_T BusStatus;
    signed int  i4Ret = I2C_ERROR_BUS_BUSY;
    unsigned int DevID = 0;
    unsigned char addr[ptr_i2c_parm->regAddrLen];

    if(channel >= I2C_MAX_NUM_BUS)
    {
        return E_BAD_PARAMETER;
    }
    if(i2cStruct[channel] == NULL)
    {
        return E_NOT_INITED;
    }
    if(ptr_i2c_parm->dataLen == 0)
    {
        return E_BAD_PARAMETER;
    }
    if(ptr_i2c_parm->buffPtr == 0)
    {
        return E_BAD_PARAMETER;
    }

    /* slave id <<1 to dev register */
    DevID = ptr_i2c_parm->deviceID << 1;

    /* check bus status */
    BusStatus = I2C_CheckBus(unit, channel, sid, sch);
    if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA) == FALSE)       /* bus is busy */
    {
        i4Ret = I2C_PutStopBit(unit, channel, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
        }
        BusStatus = I2C_CheckBus(unit, channel, sid, sch);                          /* check bus status, again. */
        if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA) == FALSE)   /* still bus busy... */
        {
            i4Ret = I2C_ERROR_BUS_BUSY;
            LOG(I2C_LOG_DBG, "Bus busy !\n");
        }
    }
    if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA) == TRUE)        /* bus free */
    {
        if (ptr_i2c_parm->regAddrLen)
        {
            memcpy(addr, ptr_i2c_parm->regAddr, ptr_i2c_parm->regAddrLen);
            _ReverseBytesOrder(addr, ptr_i2c_parm->regAddrLen);
            i4Ret = I2C_OptWrite(unit, channel,DevID, addr, ptr_i2c_parm->regAddrLen, I2C_CTL_NO_STOP, sid, sch);
            if (i4Ret == (signed int) ptr_i2c_parm->regAddrLen)
            {
                i4Ret = I2C_OptRead(unit, channel, DevID | 0x1, ptr_i2c_parm->buffPtr, ptr_i2c_parm->dataLen, I2C_CTL_NORMAL, sid, sch);
                if (ptr_i2c_parm->dataLen != (unsigned int) i4Ret)
                {
                    i4Ret= I2C_ERROR_DATA;
                    LOG(I2C_LOG_DBG, "Error data(with slave id) !\n");
                }
            }
            else
            {
                i4Ret = I2C_PutStopBit(unit, channel, sid, sch);
                if (i4Ret != I2C_OK)
                {
                    LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
                }
                i4Ret = I2C_ERROR_SUBADDR;
                LOG(I2C_LOG_DBG, "Error sub-addr ! \n");
            }
        }
        else      /* No sub-addr */
        {
            i4Ret = I2C_OptRead(unit, channel, DevID | 0x1, ptr_i2c_parm->buffPtr, ptr_i2c_parm->dataLen, I2C_CTL_NORMAL, sid, sch);
            if (ptr_i2c_parm->dataLen != (unsigned int) i4Ret)
            {
                i4Ret = I2C_ERROR_DATA;
                LOG(I2C_LOG_DBG, "Error data(no sub-addr)!\n");
            }
        }
    }
    if(i4Ret >= 0)
    {
        return E_OK;
    }
    return E_BAD_PARAMETER;
}

/* FUNCTION NAME: hal_sco_i2c_write
 * PURPOSE:
 *      Write data block to a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      ptr_i2c_parm    --  I2C parameter
 *      sid             --  Slave device ID
 *      sch             --  Slave device I2C master channel
 *
 * OUTPUT:
 *      ptr_i2c_parm    --  I2C parameter
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
    AIR_I2C_Master_T *ptr_i2c_parm,
    const unsigned int sid,
    const unsigned int sch)
{
    unsigned char  DevID;
    I2C_BUS_STATUS_T BusStatus;
    signed int  i4Ret = I2C_ERROR_BUS_BUSY;
    unsigned char addr[ptr_i2c_parm->regAddrLen];

    if(channel >= I2C_MAX_NUM_BUS)
    {
        return E_BAD_PARAMETER;
    }
    if(i2cStruct[channel] == NULL)
    {
        return E_NOT_INITED;
    }
    if(ptr_i2c_parm->dataLen == 0)
    {
        return E_BAD_PARAMETER;
    }
    if(ptr_i2c_parm->buffPtr == 0)
    {
        return E_BAD_PARAMETER;
    }

    /* slave id <<1 to dev register */
    DevID = ptr_i2c_parm->deviceID << 1;

    /* check bus status */
    BusStatus = I2C_CheckBus(unit, channel, sid, sch);
    if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA) == FALSE)       /* bus is busy */
    {
        i4Ret = I2C_PutStopBit(unit, channel, sid, sch);
        if (i4Ret != I2C_OK)
        {
            LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
        }
        BusStatus= I2C_CheckBus(unit, channel, sid, sch);                            /* check bus status, again. */
        if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA) == FALSE)    /* still bus busy */
        {
            i4Ret = I2C_ERROR_BUS_BUSY;
        }
    }

    if (IS_BUS_FREE(BusStatus.I2C_BUS_SCL, BusStatus.I2C_BUS_SDA)  == TRUE)        /* bus free */
    {
        if (ptr_i2c_parm->regAddrLen)
        {
            memcpy(addr, ptr_i2c_parm->regAddr, ptr_i2c_parm->regAddrLen);
            _ReverseBytesOrder(addr, ptr_i2c_parm->regAddrLen);
            i4Ret = I2C_OptWrite(unit, channel, DevID, addr, ptr_i2c_parm->regAddrLen, I2C_CTL_NO_STOP, sid, sch);
            if (i4Ret != (signed int) ptr_i2c_parm->regAddrLen)
            {
                i4Ret = I2C_PutStopBit(unit, channel, sid, sch);
                if (i4Ret != I2C_OK)
                {
                    LOG(I2C_LOG_DBG, "Stop Bit error, %d.\n", i4Ret);
                }
                i4Ret = I2C_ERROR_SUBADDR;
            }
            else
            {
                i4Ret = I2C_OptWrite(unit, channel, DevID, ptr_i2c_parm->buffPtr, ptr_i2c_parm->dataLen, I2C_CTL_NO_START, sid, sch);
                if (i4Ret != (signed int) ptr_i2c_parm->dataLen)
                {
                    i4Ret = I2C_ERROR_DATA;
                }
            }
        }
        else
        {
            i4Ret = I2C_OptWrite(unit, channel, DevID, ptr_i2c_parm->buffPtr, ptr_i2c_parm->dataLen, I2C_CTL_NORMAL, sid, sch);
            if (i4Ret != (signed int) ptr_i2c_parm->dataLen)
            {
                i4Ret = I2C_ERROR_DATA;
            }
        }
    }
    if(i4Ret >= 0)
    {
        return E_OK;
    }
    return E_BAD_PARAMETER;
}

