################################################################################
$(warning $(AIR_SDK))
dir                     := $(AIR_SYSTEM)/air_sdk/src
include                 $(dir)/make.mk

dir                     := $(AIR_SYSTEM)/app/diag_shell
include                 $(dir)/make.mk

dir                     := $(AIR_SYSTEM)/app/sfp_led
include                 $(dir)/make.mk

ifeq ($(findstring ENABLED,$(AIR_EN_I2C_BITBANG)),ENABLED)
dir                     := $(AIR_SYSTEM)/app/i2c_bitbang
include                 $(dir)/make.mk
endif
################################################################################
ifeq ($(findstring linux,$(OS_TYPE)),linux)
# To build air_dev.ko and air_sdk.ko
ifeq ($(findstring KERNEL_SPACE,$(WORK_SPACE)),KERNEL_SPACE)
include                 $(AIR_SDK)/make/linux_kernel.mk
endif

# To build air_dev.ko and air_sdk.a
ifeq ($(findstring USER_SPACE,$(WORK_SPACE)),USER_SPACE)
ifneq ($(findstring lamp,$(TARGET_NAME)),lamp)
include                 $(AIR_SDK)/make/linux_dev.mk
endif
include                 $(AIR_SDK)/make/linux_user.mk
endif
endif

ifeq ($(findstring freertos,$(OS_TYPE)),freertos)
ifeq ($(findstring ENABLED,$(AIR_EN_CHIP_DIAG)),ENABLED)
dir                     :=$(AIR_SYSTEM)/app/diag_shell/chip_diag/common/sdk_ref
include                 $(dir)/make.mk
else
dir                     := $(AIR_SYSTEM)/app/sdk_ref/freertos
include                 $(dir)/make.mk
endif
include                 $(AIR_SDK)/make/freertos_kernel.mk
endif
################################################################################
all:compile

compile::
	# Users can include air_flags.mk to acquire SDK compiler options in air_CFLAGS
	echo "AIR_CFLAGS = $(filter -D%, $(EXTRA_CFLAGS))" > $(AIR_SYSTEM_BUILD)/air_cflags.mk

install::

clean::
	$(RM) $(AIR_SYSTEM_BUILD)/air_cflags.mk

distclean:clean
	$(RM) $(AIR_SYSTEM_BUILD)/air_sdk
