################################################################################
# SDK configuration for SPB-5060S (AN8855M + AN8801SB + SK49145B)
################################################################################
# Switch chips
override ENABLED_SWITCH_CHIPS := an8855m

# PHY chips
override ENABLED_PHY_CHIPS    := an8801sb

# POE chips
override ENABLED_POE_CHIPS    := sk49145b

# I2C bitbang for PSE
override AIR_EN_I2C_BITBANG   := ENABLED
