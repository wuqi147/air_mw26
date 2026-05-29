export # export all symbol

APP_LWIP_ROOT_DIR           := $(TOPDIR)/app/lwip/
APP_LWIP_EN8851_PORT_DIR    := ports/AIR_8851
APP_LWIP_2_1_2_DIR          := lwip-2.1.2/src
APP_LWIP_API_DIR            := $(APP_LWIP_2_1_2_DIR)/api
APP_LWIP_CORE_DIR           := $(APP_LWIP_2_1_2_DIR)/core
APP_LWIP_NETIF_DIR          := $(APP_LWIP_2_1_2_DIR)/netif
APP_LWIP_APPS_DIR           := $(APP_LWIP_2_1_2_DIR)/apps
APP_LWIP_INCLUDE_DIR        := $(APP_LWIP_2_1_2_DIR)/include
APP_LWIP_APPS_HTTP_DIR      := $(APP_LWIP_APPS_DIR)/http
APP_LWIP_APPS_SNMP_DIR      := $(APP_LWIP_APPS_DIR)/snmp
APP_LWIP_APPS_MBEDTLS_DIR   := $(APP_LWIP_2_1_2_DIR)/apps/mbedtls-3.6.5
APP_MBEDTLS_LIB_DIR         := $(APP_LWIP_APPS_MBEDTLS_DIR)/library
APP_MBEDTLS_INC_DIR         := $(APP_LWIP_APPS_MBEDTLS_DIR)/include
APP_HTTP_CLIENT_DIR         := $(APP_LWIP_APPS_DIR)/freertos_http_client
APP_COMMON_DIR              := $(APP_LWIP_APPS_DIR)/common_utilities
APP_HTTP_CLIENT_CORE_DIR    := $(APP_HTTP_CLIENT_DIR)/coreHTTP/source
APP_HTTP_CLIENT_PARSER_DIR  := $(APP_HTTP_CLIENT_CORE_DIR)/dependency/3rdparty/http_parser
APP_TELNETD_DIR             := $(APP_LWIP_APPS_DIR)/freertos_telnetd
APP_LWIP_APPS_SNTP_DIR      := $(APP_LWIP_APPS_DIR)/sntp
APP_DROPBEAR_DIR            := $(APP_LWIP_APPS_DIR)/dropbear_ssh/src
APP_DROPBEAR_LIBTOMATH_DIR  := $(APP_LWIP_APPS_DIR)/dropbear_ssh/libtommath
APP_DROPBEAR_LIBTOCRYPT_DIR := $(APP_LWIP_APPS_DIR)/dropbear_ssh/libtomcrypt/src

ifneq ($(USE_TLS), DISABLED)
ifneq ($(USE_TLS),0)
APP_MBEDTLS_CFG_DIR          = $(APP_LWIP_APPS_MBEDTLS_DIR)/configs
APP_ALTCP_TLS_DIR            = $(APP_LWIP_APPS_DIR)/altcp_tls
endif
endif
# path for MW
ifeq ($(AIR_MW_SUPPORT),ENABLED)
AIR_MW_DIR = $(TOPDIR)/../air_magic_wand
AIR_MW_SYSTEM = $(TOPDIR)/../air_magic_wand/air_mw_system
AIR_MW_PROJECT = $(AIR_MW_DIR)/project/$(PROJECT)
HTMP_TOOL_DIR = $(AIR_MW_SYSTEM)/html/tools/makefsdata
AIR_MW_HTTP_CGI_DIR = $(AIR_MW_SYSTEM)/html/cgi
MW_OBJ = $(AIR_MW_SYSTEM)/obj
endif

# path for MW (MQTT)
APP_LWIP_APPS_MQTT_DIR = $(APP_LWIP_APPS_DIR)/mqtt
