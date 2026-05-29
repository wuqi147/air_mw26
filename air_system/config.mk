################################################################################
AIR_SYSTEM      := $(shell pwd)

################################################################################
include                 $(AIR_SYSTEM)/toolchain.mk
################################################################################
AIR_SYSTEM_BUILD        := $(AIR_SYSTEM)/build

################################################################################
TARGET_NAME             :=

################################################################################
#ARCH                    := arm
ARCH                    := andes

#AIR_HOST_ENDIAN         := AIR_EN_HOST_64_BIT_LITTLE_ENDIAN
AIR_HOST_ENDIAN         := AIR_EN_HOST_32_BIT_LITTLE_ENDIAN

#OS_TYPE                 := linux
OS_TYPE                 := freertos

#OS_PATH                 := $(AIR_SYSTEM)/../en7562_linux_platform/linux-5.10.181
OS_PATH                 := $(AIR_SYSTEM)/../freertos

WORK_SPACE              := USER_SPACE
#WORK_SPACE              := KERNEL_SPACE

BUILD_TYPE              := STATIC_LIB
#BUILD_TYPE              := SHARED_LIB

################################################################################
ENABLED_SWITCH_CHIPS    := en8851c an8855m an8858c
ENABLED_PHY_CHIPS       := en8808 en8801s en8811h an8801sb an8804 an8808q an8811b
ENABLED_POE_CHIPS       :=

################################################################################
# Enable the following feature by set the value to "ENABLED"
# Disable the following feature by set the value to "DISABLED"
################################################################################
AIR_EN_DEBUG            := ENABLED

################################################################################
AIR_EN_I2C_PHY          := ENABLED

################################################################################
AIR_EN_LED_CLK_SYNC     := ENABLED

################################################################################
AIR_EN_L2_SHADOW        := DISABLED

################################################################################
AIR_EN_CABLE_DIAG       := ENABLED

################################################################################
AIR_EN_COMPILER_SUPPORT_FUNCTION   := ENABLED

################################################################################
# Enable the following applications by set the value to "ENABLED"
# Disable the following applications by set the value to "DISABLED"
################################################################################
AIR_EN_SFP_LED          := DISABLED

################################################################################
AIR_EN_SFP_LED_WITH_THREAD     := ENABLED

################################################################################
AIR_EN_I2C_BITBANG      := DISABLED

################################################################################
AIR_EN_SDK_REF          := ENABLED

################################################################################

export AIR_SYSTEM_BUILD
export TARGET_NAME
export ARCH
export AIR_HOST_ENDIAN
export OS_TYPE
export OS_PATH
export WORK_SPACE
export BUILD_TYPE
export ENABLED_SWITCH_CHIPS
export ENABLED_PHY_CHIPS
export AIR_EN_DEBUG
export AIR_EN_I2C_PHY
export AIR_EN_LED_CLK_SYNC
export AIR_EN_L2_SHADOW
export AIR_EN_CABLE_DIAG
export AIR_EN_COMPILER_SUPPORT_FUNCTION
export AIR_EN_SFP_LED
export AIR_EN_I2C_BITBANG
export AIR_EN_SDK_REF
