export # export all symbol

INC := -I$(TOPDIR)/kernel/include
INC += -I$(TOPDIR)/app/common/inc
INC += -I$(TOPDIR)/bsp/include
INC += -I$(TOPDIR)/bsp/arch/$(ARCH)
INC += -I$(TOPDIR)/bsp/platform/$(CONFIG_PLATFORM)
INC += -I$(TOPDIR)/project/$(PROJECT)
INC += -I$(TOPDIR)/project/$(PROJECT)/customer
INC += -I$(AIR_SDK_DIR)/air_sdk/include
INC += -I$(AIR_SDK_DIR)/air_sdk/src/inc
INC += -I$(AIR_SDK_DIR)/air_sdk/src/inc/osal
INC += -I$(AIR_SDK_DIR)/air_sdk/src/inc/cmlib
INC += -I$(AIR_SDK_DIR)/app/diag_shell/inc/parser
INC += -I$(AIR_SDK_DIR)/app/sdk_ref/freertos/inc
INC += -I$(APP_LWIP_ROOT_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_LWIP_EN8851_PORT_DIR)/include
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_LWIP_INCLUDE_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_LWIP_INCLUDE_DIR)/netif
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_MBEDTLS_INC_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_COMMON_DIR)/include/logging
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_COMMON_DIR)/platform/posix/transport/include
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_COMMON_DIR)/mbedtls_freertos
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_HTTP_CLIENT_CORE_DIR)/include
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_HTTP_CLIENT_CORE_DIR)/interface
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_HTTP_CLIENT_PARSER_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_HTTP_CLIENT_DIR)/common/include
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_HTTP_CLIENT_DIR)/http_demo_mbed_tls
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_LWIP_APPS_SNMP_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_DROPBEAR_LIBTOMATH_DIR)
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_DROPBEAR_LIBTOCRYPT_DIR)/headers
INC += -I$(APP_LWIP_ROOT_DIR)$(APP_DROPBEAR_DIR)

INC += -I$(TOOLCHAIN_INC_PATH)
INC += -I$(TOPDIR)/app/btn_reset/inc
INC += -I$(TOPDIR)/tool

# for MW
ifeq ($(AIR_MW_SUPPORT), ENABLED)
ifneq ("$(wildcard $(AIR_MW_SYSTEM))", "")
INC += -I$(AIR_MW_SYSTEM)/cmd/inc/parser
INC += -I$(AIR_MW_SYSTEM)/util/inc
INC += -I$(AIR_MW_SYSTEM)/init/inc
INC += -I$(AIR_MW_SYSTEM)/httpd/inc
INC += -I$(AIR_MW_SYSTEM)/db/freeRTOS/inc
INC += -I$(AIR_MW_SYSTEM)/os_api/freeRTOS/inc
INC += -I$(AIR_MW_SYSTEM)/html/cgi
INC += -I$(AIR_MW_SYSTEM)/switching/icmp_client/inc
INC += -I$(AIR_MW_SYSTEM)/switching/snmp/inc
INC += -I$(AIR_MW_SYSTEM)/mqttd/inc
INC += -I$(AIR_MW_SYSTEM)/driver/sfp/inc
INC += -I$(AIR_MW_SYSTEM)/ref
INC += -I$(AIR_MW_PROJECT)
INC += -I$(AIR_MW_SYSTEM)/switching/erps/inc
INC += -I$(AIR_MW_SYSTEM)/switching/stp/rstp/inc
INC += -I$(AIR_MW_SYSTEM)/sys_mgmt/inc
INC += -I$(AIR_MW_SYSTEM)/telnet/inc
INC += -I$(AIR_MW_SYSTEM)/ssh/inc
endif
endif
