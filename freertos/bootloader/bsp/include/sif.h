#ifndef __SIF_H__
#define __SIF_H__

#include <platform.h>
#include "util.h"

/* register offset */
#define RG_SIF_CTRL0                    (0x40)
#define RG_SIF_CTRL1                    (0x44)
#define RG_SIF_DATA0                    (0x50)
#define RG_SIF_ENINT                    (0x5C)
#define RG_SIF_INTST                    (0x60)
#define RG_SIF_CLINT                    (0x64)

/* HW Trap Normal Mode Read Back Register, used for I2C Mode */
#define RG_RGS_CPU_EN                   (0x100000A8UL)

/* I2C Master Register Base */
#define RG_SIF0_BASE_ADDR               (0x10008000UL)
#define RG_SIF1_BASE_ADDR               (0x10022000UL)

/* configure for the osal sif driver */
#define SIF_DATA_LEN                    (4)
#define SIF_ADDR_LEN                    (4)

/* configure the sif data transmission speed */
#define SIF_SPEED_100K                  (0xc7)
#define SIF_SPEED_400K                  (0x31)
#define SIF_SPEED_1M                    (0x13)

/* I2C Master Register Offset */
#define SIF_SIFMCTL0                    (0x40UL)
#define SIF_SIFMCTL1                    (0x44UL)
#define SIF_SIFMD0                      (0x50UL)
#define SIF_SIFMD1                      (0x54UL)

/* SIFM Configuration Register 0 Mask */
#define SIF_EN_MSK                      (1UL)
#define SIF_SDA_STATE_MSK               (1UL)
#define SIF_SCL_STATE_MSK               (1UL)
#define SIF_CLK_DIV_MSK                 (0xfffUL)

/* SIFM Configuration Register 1 Mask */
#define SIF_TRI_MSK                     (1UL)
#define SIF_MODE_MSK                    (0x7UL)
#define SIF_PGLEN_MSK                   (0x7UL)
#define SIF_ACK_MSK                     (0xffUL)

/* HW Trap Normal Mode Bit */
#define CPU_I2C0_MODE_BIT               (4UL)

/* SIFM Configuration Register 0 Bit Position */
#define SIF_EN_BIT                      (1UL)
#define SIF_SDA_STATE_BIT               (2UL)
#define SIF_SCL_STATE_BIT               (3UL)
#define SIF_CLK_DIV_BIT                 (16UL)

/* SIFM Configuration Register 1 Bit Position */
#define SIF_TRI_BIT                     (0UL)
#define SIF_MODE_BIT                    (4UL)
#define SIF_PGLEN_BIT                   (8UL)
#define SIF_ACK_BIT                     (16UL)

/* SIFM Configuration Register 0 Shifted Mask */
#define SIF_EN_SM                       ((SIF_EN_MSK) << (SIF_EN_BIT))
#define SIF_SDA_STATE_SM                ((SIF_SDA_STATE_MSK) << (SIF_SDA_STATE_BIT))
#define SIF_SCL_STATE_SM                ((SIF_SCL_STATE_MSK) << (SIF_SCL_STATE_BIT))
#define SIF_CLK_DIV_SM                  ((SIF_CLK_DIV_MSK) << (SIF_CLK_DIV_BIT))

/* SIFM Configuration Register 1 Mask */
#define SIF_TRI_SM                      ((SIF_TRI_MSK) << (SIF_TRI_BIT))
#define SIF_MODE_SM                     ((SIF_MODE_MSK) << (SIF_MODE_BIT))
#define SIF_PGLEN_SM                    ((SIF_PGLEN_MSK) << (SIF_PGLEN_BIT))
#define SIF_ACK_SM                      ((SIF_ACK_MSK) << (SIF_ACK_BIT))

/* read/write registers */
#define SIF_READ(__BASE__, __OFFSET__)              io_read32(((__BASE__) + (__OFFSET__)))
#define SIF_WRITE(__BASE__, __OFFSET__, __VAL__)    io_write32(((__BASE__) + (__OFFSET__)), (__VAL__))

#define SIF_WRITE_MASK(__BASE__, __OFFSET__, __VAL__, __MASK__) \
    SIF_WRITE((__BASE__),(__OFFSET__), ((SIF_READ((__BASE__),(__OFFSET__)) & ~(__MASK__)) | ((__VAL__) & (__MASK__))))

typedef struct SIF_PARAM_S
{
    unsigned int bus;
    unsigned int slave_id;
    unsigned int addr;
    unsigned int *ptr_data;
} SIF_PARAM_T;

typedef enum
{
    SIF_WRITE = 0,
    SIF_READ,
    SIF_LAST,
} SIF_OPERATION_T;

typedef enum
{
    SIF_START = 1,
    SIF_WRITE_DATA,
    SIF_STOP,
    SIF_READ_FINAL,
    SIF_MODE_LAST
} SIF_MODE_T;

void sif_init(void);
int sif_write(SIF_PARAM_T  *ptr_sif_param);
int sif_read(SIF_PARAM_T  *ptr_sif_param);

#endif
