export
################################################################################
# platform
################################################################################
CONFIG_PLATFORM_AN8858=y
CONFIG_PLATFORM=an8858
################################################################################
# app
################################################################################
ifeq ($(AIR_MW_SUPPORT), ENABLED)
CONFIG_AIR_MW=y
CONFIG_APP_BTN_RESET=y
endif

ifneq ($(USE_TLS), DISABLED)
ifneq ($(USE_TLS), 0)
CONFIG_USE_TLS=y
endif
endif

CONFIG_CMD_SHELL=y

################################################################################
# cpu architecture
################################################################################
CONFIG_ARCH_ANDES=y

################################################################################
# driver, peripheral feature
#
# Note: if you need the feature,
#       set the config here to y
################################################################################
CONFIG_DRIVER_CHIPSCU=y
CONFIG_DRIVER_GPIO=y

ifeq ($(AIR_I2C_SUPPORT), ENABLED)
CONFIG_DRIVER_I2C=y
endif

CONFIG_DRIVER_PDMA=y
CONFIG_DRIVER_SPI=y
CONFIG_DRIVER_TIMER=y
CONFIG_DRIVER_UART=y
CONFIG_DRIVER_UTIL=y
CONFIG_DRIVER_PINCTRL=y

ifeq ($(ERSP_API), ENABLED)
CONFIG_ERPS_API=y
endif

################################################################################
# driver, peripheral files
################################################################################
# chipscu
CONFIG_CHIPSCU_AN8858=y

# gpio
CONFIG_GPIO_AN8858=y

# i2c
ifeq ($(CONFIG_DRIVER_I2C),y)
CONFIG_I2C_EN8851=y
endif

# pdma
CONFIG_PDMA_AN8858=y

# spi
CONFIG_SPI_COMMON=y

# timer
CONFIG_TIMER_EN8851=y

# uart
CONFIG_UART_COMMON=y

# util
CONFIG_UTIL_COMMON=y
