export
################################################################################
# platform
################################################################################
CONFIG_PLATFORM_AN8855=y
CONFIG_PLATFORM=an8855

CONFIG_EXTRA_MEMORY=y
ifeq ($(CONFIG_EXTRA_MEMORY), y)
EXTRA_FLAGS    += -DCONFIG_EXTRA_MEMORY
endif

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

################################################################################
# driver, peripheral files
################################################################################
# chipscu
CONFIG_CHIPSCU_AN8855=y

# gpio
CONFIG_GPIO_AN8855=y

# i2c
ifeq ($(CONFIG_DRIVER_I2C), y)
CONFIG_I2C_AN8855=y
endif

# pdma
CONFIG_PDMA_AN8855=y

# spi
CONFIG_SPI_COMMON=y

# timer
CONFIG_TIMER_AN8855=y

# uart
CONFIG_UART_COMMON=y

# util
CONFIG_UTIL_COMMON=y

