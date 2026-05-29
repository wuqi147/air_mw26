###############################################################################
# Switch chips
###############################################################################
# Scorpio family
ifeq ($(findstring scorpio,$(ENABLED_SWITCH_CHIPS)),scorpio)
COMPILED_SWITCH_CHIPS    += scorpio
else ifeq ($(findstring en8851c,$(ENABLED_SWITCH_CHIPS)),en8851c)
COMPILED_SWITCH_CHIPS    += scorpio
COMPILED_PHY_CHIPS       += en8808
else ifeq ($(findstring en8851e,$(ENABLED_SWITCH_CHIPS)),en8851e)
COMPILED_SWITCH_CHIPS    += scorpio
COMPILED_PHY_CHIPS       += en8808
else ifeq ($(findstring en8853c,$(ENABLED_SWITCH_CHIPS)),en8853c)
COMPILED_SWITCH_CHIPS    += scorpio
COMPILED_PHY_CHIPS       += en8808
else ifeq ($(findstring en8860c,$(ENABLED_SWITCH_CHIPS)),en8860c)
COMPILED_SWITCH_CHIPS    += scorpio
endif

# Pearl family
ifeq ($(findstring pearl,$(ENABLED_SWITCH_CHIPS)),pearl)
COMPILED_SWITCH_CHIPS    += pearl
else ifeq ($(findstring an8855m,$(ENABLED_SWITCH_CHIPS)),an8855m)
COMPILED_SWITCH_CHIPS    += pearl
COMPILED_PHY_CHIPS       += an8804
endif

# Coral family
ifeq ($(findstring coral,$(ENABLED_SWITCH_CHIPS)),coral)
COMPILED_SWITCH_CHIPS    += coral
else ifeq ($(findstring an8858b,$(ENABLED_SWITCH_CHIPS)),an8858b)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
else ifeq ($(findstring an8858c,$(ENABLED_SWITCH_CHIPS)),an8858c)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
else ifeq ($(findstring an8858e,$(ENABLED_SWITCH_CHIPS)),an8858e)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
else ifeq ($(findstring an8858h,$(ENABLED_SWITCH_CHIPS)),an8858h)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
else ifeq ($(findstring an8858f,$(ENABLED_SWITCH_CHIPS)),an8858f)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
else ifeq ($(findstring an8858d,$(ENABLED_SWITCH_CHIPS)),an8858d)
COMPILED_SWITCH_CHIPS    += coral
COMPILED_PHY_CHIPS       += an8858
endif

###############################################################################
# PHY chips
###############################################################################
ifeq ($(findstring en8801s,$(ENABLED_PHY_CHIPS)),en8801s)
COMPILED_PHY_CHIPS       += en8801s
endif
ifeq ($(findstring en8808,$(ENABLED_PHY_CHIPS)),en8808)
COMPILED_PHY_CHIPS       += en8808
endif
ifeq ($(findstring en8804,$(ENABLED_PHY_CHIPS)),en8804)
COMPILED_PHY_CHIPS       += en8808
endif
ifeq ($(findstring en8811h,$(ENABLED_PHY_CHIPS)),en8811h)
COMPILED_PHY_CHIPS       += en8811h
endif
ifeq ($(findstring an8801sb,$(ENABLED_PHY_CHIPS)),an8801sb)
COMPILED_PHY_CHIPS       += an8801sb
endif
ifeq ($(findstring an8811b,$(ENABLED_PHY_CHIPS)),an8811b)
COMPILED_PHY_CHIPS       += an8811b
endif
ifeq ($(findstring an8804,$(ENABLED_PHY_CHIPS)),an8804)
COMPILED_PHY_CHIPS       += an8804
endif
ifeq ($(findstring an8858,$(ENABLED_PHY_CHIPS)),an8858)
COMPILED_PHY_CHIPS       += an8858
endif
ifeq ($(findstring an8808q,$(ENABLED_PHY_CHIPS)),an8808q)
COMPILED_PHY_CHIPS       += an8808
endif

###############################################################################
# POE chips
###############################################################################
ifeq ($(findstring ip804ar,$(ENABLED_POE_CHIPS)),ip804ar)
COMPILED_POE_CHIPS       += an8502
endif
ifeq ($(findstring ip808ar,$(ENABLED_POE_CHIPS)),ip808ar)
COMPILED_POE_CHIPS       += an8503
endif
ifeq ($(findstring an8502,$(ENABLED_POE_CHIPS)),an8502)
COMPILED_POE_CHIPS       += an8502
endif
ifeq ($(findstring an8503,$(ENABLED_POE_CHIPS)),an8503)
COMPILED_POE_CHIPS       += an8503
endif

export COMPILED_SWITCH_CHIPS COMPILED_PHY_CHIPS COMPILED_POE_CHIPS
