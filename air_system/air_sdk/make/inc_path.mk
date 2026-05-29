################################################################################
INCS    += -I$(AIR_SDK)/include
INCS    += -I$(AIR_SDK)/src/inc
INCS    += -I$(AIR_SYSTEM)/app/diag_shell/inc
INCS    += -I$(AIR_SYSTEM)/app/sdk_ref
ifeq ($(findstring ENABLED,$(AIR_EN_SFP_LED)),ENABLED)
INCS    += -I$(AIR_SYSTEM)/app/sfp_led
endif

ifeq ($(findstring ENABLED,$(AIR_EN_I2C_BITBANG)),ENABLED)
INCS    += -I$(AIR_SYSTEM)/app/i2c_bitbang
endif

ifeq ($(findstring ENABLED,$(AIR_EN_CHIP_DIAG)),ENABLED)
INCS    += -I$(AIR_SYSTEM)/app/diag_shell/chip_diag/inc
endif

ifeq ($(findstring ENABLED,$(AIR_EN_API_DIAG)),ENABLED)
INCS    += -I$(AIR_SYSTEM)/app/diag_shell/api_diag/inc
endif
################################################################################
