################################################################################
# Default global configuration
################################################################################
include $(AIR_SDK)/make/chip.mk

EXTRA_CFLAGS            += $(INCS)

################################################################################
CHIP_MK_LIST            := $(addprefix $(AIR_SDK)/make/,$(addsuffix .mk,$(COMPILED_SWITCH_CHIPS)))

include $(CHIP_MK_LIST)

FEATURE_LIST            := $(sort $(FEATURE_LIST))

EN_FEATURE_LIST         := $(filter-out $(CUSTOM_DISABLE_LIST),$(FEATURE_LIST))

FEATURE_EN_FLAGS        := $(addprefix -DAIR_EN_,$(addsuffix =1,$(EN_FEATURE_LIST)))

EXTRA_CFLAGS            += $(FEATURE_EN_FLAGS)

################################################################################
# Internal features
################################################################################
ifeq ($(findstring linux,$(OS_TYPE)),linux)
EXTRA_CFLAGS            += -DAIR_EN_CHIP_INIT_RESET
EXTRA_CFLAGS            += -DAIR_EN_SPEED_UP_FIRMWARE_DOWNLOAD
endif
ifneq ($(COMPILED_POE_CHIPS),)
EXTRA_CFLAGS            += -DAIR_EN_POE
endif

################################################################################
# External features
################################################################################
# linux: ks or us
ifeq ($(findstring linux,$(OS_TYPE)),linux)
EXTRA_CFLAGS            += -DAIR_LINUX
ifeq ($(findstring USER_SPACE,$(WORK_SPACE)),USER_SPACE)
EXTRA_CFLAGS            += -DAIR_LINUX_USER_MODE
endif
ifeq ($(findstring KERNEL_SPACE,$(WORK_SPACE)),KERNEL_SPACE)
EXTRA_CFLAGS            += -DAIR_LINUX_KERNEL_MODE
endif
ifeq ($(findstring ENABLED,$(AIR_EN_L2_SHADOW)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_L2_SHADOW
endif
endif

# freertos:
ifeq ($(findstring freertos,$(OS_TYPE)),freertos)
EXTRA_CFLAGS            += -DAIR_FREERTOS
endif

ifeq ($(findstring ENABLED,$(AIR_EN_DEBUG)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_DEBUG
endif

ifeq ($(findstring ENABLED,$(AIR_EN_I2C_PHY)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_I2C_PHY
endif

ifeq ($(findstring ENABLED,$(AIR_EN_I2C_TO_I2C_ACCESS)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_I2C_TO_I2C_ACCESS
endif

ifeq ($(findstring ENABLED,$(AIR_EN_COMPILER_SUPPORT_FUNCTION)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_COMPILER_SUPPORT_FUNCTION
endif

ifeq ($(findstring ENABLED,$(AIR_EN_L2_HW_FLUSH)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_L2_HW_FLUSH
endif

################################################################################
# Endian
################################################################################
ifeq ($(findstring AIR_EN_HOST_32_BIT_BIG_ENDIAN,$(AIR_HOST_ENDIAN)),AIR_EN_HOST_32_BIT_BIG_ENDIAN)
EXTRA_CFLAGS            += -DAIR_EN_HOST_32_BIT_BIG_ENDIAN
EXTRA_CFLAGS            += -DAIR_EN_BIG_ENDIAN
endif

ifeq ($(findstring AIR_EN_HOST_32_BIT_LITTLE_ENDIAN,$(AIR_HOST_ENDIAN)),AIR_EN_HOST_32_BIT_LITTLE_ENDIAN)
EXTRA_CFLAGS            += -DAIR_EN_HOST_32_BIT_LITTLE_ENDIAN
EXTRA_CFLAGS            += -DAIR_EN_LITTLE_ENDIAN
endif

ifeq ($(findstring AIR_EN_HOST_64_BIT_BIG_ENDIAN,$(AIR_HOST_ENDIAN)),AIR_EN_HOST_64_BIT_BIG_ENDIAN)
EXTRA_CFLAGS            += -DAIR_EN_HOST_64_BIT_BIG_ENDIAN
EXTRA_CFLAGS            += -DAIR_EN_BIG_ENDIAN
endif

ifeq ($(findstring AIR_EN_HOST_64_BIT_LITTLE_ENDIAN,$(AIR_HOST_ENDIAN)),AIR_EN_HOST_64_BIT_LITTLE_ENDIAN)
EXTRA_CFLAGS            += -DAIR_EN_HOST_64_BIT_LITTLE_ENDIAN
EXTRA_CFLAGS            += -DAIR_EN_LITTLE_ENDIAN
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_64BIT_ADDR)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_64BIT_ADDR
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_LED_CLK_SYNC)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_LED_CLK_SYNC
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_SFP_LED)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_SFP_LED
endif

ifeq ($(findstring ENABLED,$(AIR_EN_SFP_LED_WITH_THREAD)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_SFP_LED_WITH_THREAD
endif

ifeq ($(findstring ENABLED,$(AIR_EN_I2C_BITBANG)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_I2C_BITBANG
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_CABLE_DIAG)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_CABLE_DIAG
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_DUMB_FIRMWARE_SUPPORT)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_DUMB_FIRMWARE_SUPPORT
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_SPEED_DOWNSHIFT_ENHANCE)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_SPEED_DOWNSHIFT_ENHANCE
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_CHIP_DIAG)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_CHIP_DIAG
endif

################################################################################
ifeq ($(findstring ENABLED,$(AIR_EN_API_DIAG)),ENABLED)
EXTRA_CFLAGS            += -DAIR_EN_API_DIAG
endif

################################################################################
ifeq ($(findstring scorpio,$(COMPILED_SWITCH_CHIPS)),scorpio)
EXTRA_CFLAGS            += -DAIR_EN_SCORPIO
endif

ifeq ($(findstring en8801s,$(COMPILED_PHY_CHIPS)),en8801s)
EXTRA_CFLAGS            += -DAIR_EN_EN8801S_PHY
endif
ifeq ($(findstring en8808,$(COMPILED_PHY_CHIPS)),en8808)
EXTRA_CFLAGS            += -DAIR_EN_EN8808_PHY
endif
ifeq ($(findstring en8811h,$(COMPILED_PHY_CHIPS)),en8811h)
EXTRA_CFLAGS            += -DAIR_EN_EN8811H_PHY
endif
ifeq ($(findstring an8801sb,$(COMPILED_PHY_CHIPS)),an8801sb)
EXTRA_CFLAGS            += -DAIR_EN_AN8801SB_PHY
endif
ifeq ($(findstring an8811b,$(COMPILED_PHY_CHIPS)),an8811b)
EXTRA_CFLAGS            += -DAIR_EN_AN8811B_PHY
endif

ifeq ($(findstring pearl,$(COMPILED_SWITCH_CHIPS)),pearl)
EXTRA_CFLAGS            += -DAIR_EN_PEARL
endif
ifeq ($(findstring an8804,$(COMPILED_PHY_CHIPS)),an8804)
EXTRA_CFLAGS            += -DAIR_EN_AN8804_PHY
endif

ifeq ($(findstring coral,$(COMPILED_SWITCH_CHIPS)),coral)
EXTRA_CFLAGS            += -DAIR_EN_CORAL
endif
ifeq ($(findstring an8858,$(COMPILED_PHY_CHIPS)),an8858)
EXTRA_CFLAGS            += -DAIR_EN_AN8858_PHY
endif

ifeq ($(findstring an8808,$(COMPILED_PHY_CHIPS)),an8808)
EXTRA_CFLAGS            += -DAIR_EN_AN8808_PHY
endif

ifeq ($(findstring an8502,$(COMPILED_POE_CHIPS)),an8502)
EXTRA_CFLAGS            += -DAIR_EN_AN8502_POE
endif
ifeq ($(findstring an8503,$(COMPILED_POE_CHIPS)),an8503)
EXTRA_CFLAGS            += -DAIR_EN_AN8503_POE
endif

################################################################################
export EXTRA_CFLAGS
