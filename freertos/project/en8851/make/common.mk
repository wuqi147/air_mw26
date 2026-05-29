export # export all symbol

# build command
build          := -f $(TOPDIR)/project/make/build.mk object
link           := -f $(BUILD_DIR)/make/link.mk

# define flags
EXTRA_FLAGS    := -DCFG_$(ADDR)

# quiet command, that is, originally, the command will show twice.
Q              := @

# some definition
EXTRA_FLAGS    += -DAIR_8851_SUPPORT -DDUAL_IMAGE=$(DUAL_IMAGE)

ifeq ($(USE_TLS), ECDHE)
EXTRA_FLAGS    += -DENABLE_TLS -I$(APP_MBEDTLS_CFG_DIR) -DAIR_SUPPORT_TLS_ECDHE -DMBEDTLS_CONFIG_FILE='"$(APP_MBEDTLS_CFG_DIR)/config-ecdhe.h"'
endif
ifeq ($(USE_TLS), RSA)
EXTRA_FLAGS    += -DENABLE_TLS -I$(APP_MBEDTLS_CFG_DIR) -DMBEDTLS_CONFIG_FILE='"$(APP_MBEDTLS_CFG_DIR)/config-rsa.h"'
endif
ifeq ($(USE_TLS), HASH)
EXTRA_FLAGS    += -I$(APP_MBEDTLS_CFG_DIR) -DMBEDTLS_CONFIG_FILE='"$(APP_MBEDTLS_CFG_DIR)/config-hash.h"'
endif
ifeq ($(USE_TLS), 1)
EXTRA_FLAGS    += -DENABLE_TLS -I$(APP_MBEDTLS_CFG_DIR) -DMBEDTLS_CONFIG_FILE='"$(APP_MBEDTLS_CFG_DIR)/config-rsa.h"'
endif

ifeq ($(AIR_MW_SUPPORT), ENABLED)
ifeq ($(USE_TLS), DISABLED)
$(error USE_TLS must be enabled in freeRTOS/project/$(PROJECT)/make/config.mk)
endif
ifeq ($(USE_TLS), 0)
$(error USE_TLS must be enabled in freeRTOS/project/$(PROJECT)/make/config.mk)
endif

ifeq ($(USE_HTTPS), 1)
ifeq ($(USE_TLS), HASH)
$(error USE_TLS must be RSA or ECDHE in freeRTOS/project/$(PROJECT)/make/config.mk)
endif
EXTRA_FLAGS    += -DENABLE_HTTPS
endif
endif

ifeq ($(USE_CACHEWB), 1)
EXTRA_FLAGS    += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITEBACK
endif

ifeq ($(USE_CACHEWT), 1)
EXTRA_FLAGS    += -DCFG_CACHE_ENABLE -DCFG_CACHE_WRITETHROUGH
endif

ifeq ($(ERSP_API), ENABLED)
EXTRA_FLAGS    += -DCONFIG_ERPS_API
ifeq ($(ERSP_API_DEMO), ENABLED)
EXTRA_FLAGS    += -DCONFIG_ERPS_API_DEMO
endif
endif

ifeq ($(AIR_EN_WARNING_AS_ERROR),ENABLED)
# todo: add Werror for files from other repository or fix all warning..
#EXTRA_CFLAGS         += -Werror
endif

ifeq ($(AIR_I2C_SUPPORT), ENABLED)
EXTRA_FLAGS    += -DCONFIG_DRIVER_I2C
endif
ifeq ($(AIR_MW_SUPPORT), ENABLED)
EXTRA_FLAGS    += -DAIR_MW_SUPPORT
HTTPSCHUNKED    = -DHTTP_TRANSFER_ENCODING_CHUNKED
EXTRA_FLAGS    += $(HTTPSCHUNKED)
endif
