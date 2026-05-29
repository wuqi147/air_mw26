################################################################################
all:compile

################################################################################
# Target dependent build flow
ifeq ($(OS_TYPE),linux)
ifeq ($(findstring KERNEL_SPACE,$(WORK_SPACE)),KERNEL_SPACE)
include $(SDK_REF)/make/linux_app.mk
include $(SDK_REF)/make/linux_kernel.mk
endif

ifeq ($(findstring USER_SPACE,$(WORK_SPACE)),USER_SPACE)
include $(SDK_REF)/make/linux_user.mk
endif
else

ifeq ($(findstring ENABLED,$(AIR_EN_SDK_REF)),ENABLED)
include $(SDK_REF)/freertos/make.mk
endif

endif

################################################################################


