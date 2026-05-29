#ifndef __SPI_CONTROLLER_H__
#define __SPI_CONTROLLER_H__

#define u8 unsigned char
#define u32 unsigned int

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int

typedef enum{
    SPI_CONTROLLER_SPEED_SINGLE =0,
    SPI_CONTROLLER_SPEED_DUAL,
    SPI_CONTROLLER_SPEED_QUAD
} SPI_CONTROLLER_SPEED_T;

typedef enum{
    SPI_CONTROLLER_RTN_NO_ERROR =0,
    SPI_CONTROLLER_RTN_SET_OPFIFO_ERROR,
    SPI_CONTROLLER_RTN_READ_DATAPFIFO_ERROR,
    SPI_CONTROLLER_RTN_WRITE_DATAPFIFO_ERROR,

    SPI_CONTROLLER_RTN_DEF_NO
} SPI_CONTROLLER_RTN_T;

typedef enum{
    SPI_CONTROLLER_MODE_AUTO=0,
    SPI_CONTROLLER_MODE_MANUAL,
    SPI_CONTROLLER_MODE_DMA,
    SPI_CONTROLLER_MODE_NO
} SPI_CONTROLLER_MODE_T;

typedef struct SPI_CONTROLLER_CONFIGURE
{
    SPI_CONTROLLER_MODE_T   mode;
    u32                     dummy_byte_num;
} SPI_CONTROLLER_CONF_T;

void spi_byte_mode(unsigned int *AutoMode_4Byte);
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Manual_Mode(void);
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_Low(void);
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_One_Byte( u8 data );
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Chip_Select_High(void);
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Write_NByte( u8 *ptr_data, u32 len, SPI_CONTROLLER_SPEED_T speed );
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Read_NByte(u8 *ptr_rtn_data, u32 len, SPI_CONTROLLER_SPEED_T speed);
void SPI_CONTROLLER_Get_Addressing_Mode( u8 *ptr_addressing_mode );
SPI_CONTROLLER_RTN_T SPI_CONTROLLER_Enable_Auto_Mode( void );

#define HIGH_BIT_UNC        (0x1<<30)

/* MACRO DECLARATIONS ---------------------------------------------------------------- */
#define _SPI_CONTROLLER_VAL_OP_CSH                  (0x00)
#define _SPI_CONTROLLER_VAL_OP_CSL                  (0x01)
#define _SPI_CONTROLLER_VAL_OP_CK                   (0x02)
#define _SPI_CONTROLLER_VAL_OP_OUTS                 (0x08)
#define _SPI_CONTROLLER_VAL_OP_OUTD                 (0x09)
#define _SPI_CONTROLLER_VAL_OP_OUTQ                 (0x0A)
#define _SPI_CONTROLLER_VAL_OP_INS                  (0x0C)
#define _SPI_CONTROLLER_VAL_OP_INS0                 (0x0D)
#define _SPI_CONTROLLER_VAL_OP_IND                  (0x0E)
#define _SPI_CONTROLLER_VAL_OP_INQ                  (0x0F)
#define _SPI_CONTROLLER_VAL_OP_OS2IS                (0x10)
#define _SPI_CONTROLLER_VAL_OP_OS2ID                (0x11)
#define _SPI_CONTROLLER_VAL_OP_OS2IQ                (0x12)
#define _SPI_CONTROLLER_VAL_OP_OD2IS                (0x13)
#define _SPI_CONTROLLER_VAL_OP_OD2ID                (0x14)
#define _SPI_CONTROLLER_VAL_OP_OD2IQ                (0x15)
#define _SPI_CONTROLLER_VAL_OP_OQ2IS                (0x16)
#define _SPI_CONTROLLER_VAL_OP_OQ2ID                (0x17)
#define _SPI_CONTROLLER_VAL_OP_OQ2IQ                (0x18)
#define _SPI_CONTROLLER_VAL_OP_OSNIS                (0x19)
#define _SPI_CONTROLLER_VAL_OP_ODNID                (0x1A)

#define _SPI_CONTROLLER_VAL_SFC_STRAP_ADDR_4B       (0x1)

#endif /* ifndef __SPI_CONTROLLER_H__ */
/* End of [spi_controller.h] package */
