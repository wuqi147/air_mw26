export # export all symbol
################################################################################
# Platform Support
################################################################################
DUAL_IMAGE           := 1
ADDR                 := 4GB
USE_CACHEWB          := 1
V                    := 1
################################################################################
# USE_TLS usage:
# DISABLED: mbedtls is disabled.
# RSA: mbedtls is configured to support RSA cipher suites.
# ECDHE: mbedtls is configured to support ECDHE cipher suites.
# HASH: mbedtls is configured to support some hash algorithms
################################################################################
USE_TLS              := HASH
USE_HTTPS            := 0
ARCH                 := NDS32

################################################################################
# Configure platform
################################################################################

################################################################################
# Enable the following feature by set the value to "ENABLED"
# Disable the following feature by set the value to "DISABLED"
################################################################################
AIR_MW_SUPPORT       = ENABLED

################################################################################
# Enable i2c driver
################################################################################
AIR_I2C_SUPPORT      = ENABLED
