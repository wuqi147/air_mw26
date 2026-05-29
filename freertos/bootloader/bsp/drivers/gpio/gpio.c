#include <gpio.h>

#ifdef USE_PINCTRL
#include "pinctrl.h"
#endif


int gpio_request(const unsigned int pin)
{
#ifndef USE_PINCTRL
    GPIO_WRITE_MASK(RG_IOMUX_CONTROL3, (1 << pin), (1 << pin));
#else
    pinctrl_request(pin, AIR_PINCTRL_GPIO);
#endif
    return 0;
}

int gpio_free(const unsigned int pin)
{
#ifndef USE_PINCTRL
    GPIO_WRITE_MASK(RG_IOMUX_CONTROL3, (0 << pin), (1 << pin));
#else
    pinctrl_free(pin);
#endif
    return 0;
}

int gpio_set_direction(const unsigned int pin, unsigned int direction)
{
    int reg, index;
    GPIO_WRITE_MASK(RG_GPIOOE, 0 << pin, 1 << pin);
    reg = (pin < 16) ? (RG_GPIOCTRL) : (RG_GPIOCTRL1);
    index = (pin < 16) ? (pin) : (pin - 16);
    direction = direction ? 1 : 0;
    GPIO_WRITE_MASK(reg, (direction << (index * 2)), (3 << (index * 2)));
    return 0;
}

int gpio_get_direction(const unsigned int pin, unsigned int *ptr_direction)
{
    int reg, index;
    reg = (pin < 16) ? (RG_GPIOCTRL) : (RG_GPIOCTRL1);
    index = (pin < 16) ? (pin) : (pin - 16);
    *ptr_direction = !!GPIO_GET_PIN(reg, (index * 2), TWO_BIT_MSK);
    return 0;
}

int gpio_set_data(const unsigned int pin, unsigned int data)
{
    GPIO_WRITE_MASK(RG_GPIODATA, (data << pin), (ONE_BIT_MSK << pin));
    GPIO_WRITE_MASK(RG_GPIOOE, (1 << pin), (ONE_BIT_MSK << pin));
    return 0;
}

int gpio_get_data(const unsigned int pin, unsigned int *ptr_data)
{
    *ptr_data = !!GPIO_GET_PIN(RG_GPIODATA, pin, ONE_BIT_MSK);
    return 0;
}
