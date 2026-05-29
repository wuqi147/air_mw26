#ifndef __GPIO_H__
#define __GPIO_H__

#include <platform.h>
#include "util.h"

#define GPIO_PIN_COUNT                 (23)

#define RG_IOMUX_CONTROL3              (0x10000090UL)

#define RG_GPIOCTRL                    (0x1000A300UL)
#define RG_GPIOCTRL1                   (0x1000A320UL)

#define RG_GPIODATA                    (0x1000A304UL)

#define RG_GPIOOE                      (0x1000A314UL)

#define RG_GPIOINT                     (0x1000A308UL)

#define RG_GPIOINTEDG                  (0x1000A30CUL)
#define RG_GPIOINTEDG1                 (0x1000A380UL)

#define RG_GPIOINTLVL                  (0x1000A310UL)
#define RG_GPIOINTLVL1                 (0x1000A38CUL)

/* read/write registers */
#define GPIO_READ(__ADDR__)              io_read32((__ADDR__))
#define GPIO_WRITE(__ADDR__, __VAL__)    io_write32((__ADDR__), (__VAL__))

#define GPIO_WRITE_MASK(__ADDR__, __VAL__, __MASK__) \
    GPIO_WRITE((__ADDR__), ((GPIO_READ(__ADDR__) & ~(__MASK__)) | ((__VAL__) & (__MASK__))))

#define GPIO_GET_PIN(__ADDR__, __PIN__, __MASK__)    \
    (((GPIO_READ(__ADDR__)) & ((__MASK__) << (__PIN__))) >> (__PIN__))

#define ONE_BIT_MSK                     (1)
#define TWO_BIT_MSK                     (3)

int gpio_request(const unsigned int pin);
int gpio_free(const unsigned int pin);
int gpio_set_direction(const unsigned int pin, unsigned int direction);
int gpio_get_direction(const unsigned int pin, unsigned int *ptr_direction);
int gpio_set_data(const unsigned int pin, unsigned int data);
int gpio_get_data(const unsigned int pin, unsigned int *ptr_data);

#endif