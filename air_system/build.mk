################################################################################
AIR_SYSTEM      := $(shell pwd)

################################################################################
BUILD_LIST      := AIR_SDK
ifeq ($(OS_TYPE),linux)
BUILD_LIST      += SDK_REF
endif
AIR_SDK         := $(AIR_SYSTEM)/air_sdk
SDK_REF         := $(AIR_SYSTEM)/app/sdk_ref

MK_OPTIONS      :=

################################################################################
# check input parameter
################################################################################
ifndef AIR_SYSTEM_BUILD
$(error AIR_SYSTEM_BUILD is not defined)
endif
ifndef ARCH
$(error ARCH is not defined)
endif
ifndef AIR_HOST_ENDIAN
$(error AIR_HOST_ENDIAN is not defined)
endif
ifndef OS_TYPE
$(error OS_TYPE is not defined)
endif
ifndef OS_PATH
$(error OS_PATH is not defined)
endif
ifndef WORK_SPACE
$(error WORK_SPACE is not defined)
endif
ifndef ENABLED_SWITCH_CHIPS
$(error ENABLED_SWITCH_CHIPS is not defined)
endif
ifndef AIR_EN_DEBUG
$(error AIR_EN_DEBUG is not defined)
endif
ifndef AIR_EN_COMPILER_SUPPORT_FUNCTION
$(error AIR_EN_COMPILER_SUPPORT_FUNCTION is not defined)
endif
ifndef CC
$(error CC is not defined)
endif
ifndef LD
$(error LD is not defined)
endif
ifndef AR
$(error AR is not defined)
endif
ifndef STRIP
$(error STRIP is not defined)
endif
ifndef MAKE
$(error MAKE is not defined)
endif
ifndef RM
$(error RM is not defined)
endif
ifndef TEST_PATH
$(error TEST_PATH is not defined)
endif
ifndef MKDIR
$(error MKDIR is not defined)
endif
ifndef CP
$(error CP is not defined)
endif
ifndef MV
$(error MV is not defined)
endif

################################################################################
include         $(AIR_SDK)/make/inc_path.mk
include         $(AIR_SDK)/make/common.mk

################################################################################
export AIR_SYSTEM
export AIR_SDK SDK_REF
