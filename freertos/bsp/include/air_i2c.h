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

/* FILE NAME:  air_I2C.h
 * PURPOSE:
 *      Define I2C API function in AIR SDK.
 *
 * NOTES:
 *
 */

#ifndef AIR_I2C_H
#define AIR_I2C_H

/* INCLUDE FILE DECLARATIONS
 */

/* NAMING DECLARATIONS
 */
#define DIAG_I2C_MAX_ADDR_LEN         (7)
#define DIAG_I2C_MAX_DATA_LEN         (24)
#define DSH_CH_PER_BYTES              (2)

/* MACRO FUNCTION DECLARATIONS
 */

/* DATA TYPE DECLARATIONS
 */
typedef enum
{
    I2C_ERROR_NODEV       = -1,       /* I2C device doesn't ack to I2C master                     */
    I2C_ERROR_SUBADDR     = -2,       /* There is error when I2C master transmits subaddress data */
    I2C_ERROR_DATA        = -3,       /* There is error when I2C master transmits/receives data   */
    I2C_ERROR_BUS_BUSY    = -4,       /* I2C bus is busy, can't access                            */
    I2C_ERROR_PARAM       = -5,       /* Function parameter is invalid                            */
    I2C_ERROR_TIMEOUT     = -6,       /* I2C semaphore timeout                                    */
} I2C_ERROR_T;

typedef enum I2C_Channel_E
{
    I2C_Channel_0,
    I2C_Channel_1
}I2C_Channel_T;

typedef enum I2C_Clock_E
{
    I2C_Clock_100K,
    I2C_Clock_400K,
    I2C_Clock_1M
}I2C_Clock_T;

typedef struct AIR_I2C_Master_S
{
    unsigned int deviceID;     /* slave ID */
    unsigned int regAddrLen;   /* Register address format (0..n) */
    unsigned char  *regAddr;     /* Register address */
    unsigned char  *buffPtr;     /* Buffer pointer */
    unsigned int dataLen;      /* Register data length */
}AIR_I2C_Master_T;

/* EXPORTED SUBPROGRAM SPECIFICATIONS
 */

/* FUNCTION NAME: air_i2c_open
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
 *      E_BAD_PARAMETER
 *
 * NOTES:
 *      None
 */
int
air_i2c_open(
    const unsigned int unit,
    const unsigned int channel);

/* FUNCTION NAME: air_i2c_setClock
 * PURPOSE:
 *      Set I2C clock to a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      clock           --  Clock speed
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
air_i2c_setClock(
    const unsigned int unit,
    const unsigned int channel,
    const unsigned int clock);

/* FUNCTION NAME: air_i2c_getClock
 * PURPOSE:
 *      Get I2C clock from a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *
 * OUTPUT:
 *      *ptr_clock      --  Clock speed
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
air_i2c_getClock(
    const unsigned int unit,
    const unsigned int channel,
          unsigned int *ptr_clock);

/* FUNCTION NAME: air_i2c_read
 * PURPOSE:
 *      Read data from a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      ptr_I2C_parm    --  I2C parameter
 *
 * OUTPUT:
 *      ptr_i2c_parm->buffPtr
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
air_i2c_read(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_I2C_parm);

/* FUNCTION NAME: air_i2c_write
 * PURPOSE:
 *      Write data to a specified channel.
 *
 * INPUT:
 *      unit            --  Device ID
 *      channel         --  Index of I2C channel
 *      ptr_i2c_parm    --  I2C parameter
 *
 * OUTPUT:
 *      ptr_I2C_parm->RxBuffPtr
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
air_i2c_write(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_I2C_parm);

/* FUNCTION NAME: air_i2c_readBySlave
 * PURPOSE:
 *      Read data from a specified channel via slave device.
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
air_i2c_readBySlave(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_i2c_parm,
    const unsigned int sid,
    const unsigned int sch);

/* FUNCTION NAME: air_i2c_writeBySlave
 * PURPOSE:
 *      Write data to a specified channel via slave device.
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
air_i2c_writeBySlave(
    const unsigned int unit,
    const unsigned int channel,
    AIR_I2C_Master_T *ptr_i2c_parm,
    const unsigned int sid,
    const unsigned int sch);

#endif  /* End of AIR_I2C_H */
