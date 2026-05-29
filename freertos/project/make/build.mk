src                 := $(object)
src                 := $(patsubst %/,%,$(src))

obj                 := $(object)
ifeq ($(MAKE_VERSION), 4.1)
obj                 := $(patsubst $(TOPDIR)/%,$(TARGET_OUTDIR)/%,$(obj))
else
obj                 := $(patsubst $(TOPDIR)%,$(TARGET_OUTDIR)%,$(obj))
endif
obj                 := $(patsubst %/,%,$(obj))

all: __build
target              := $(addsuffix built-in.a,$(obj)/)

target-build-dir    := $(if $(filter /%, $(src)), $(src),$(TOPDIR)/$(src))
target-build-mk     := $(wildcard $(target-build-dir)/Makefile)

ifeq ($(target-build-mk),)
$(error "no Makefile to make target: $(obj)")
endif

include $(target-build-mk)

subdir-obj-y        := $(addsuffix built-in.a,$(addprefix $(obj)/,$(subdir-y)))
subdir-y            := $(addprefix $(src)/, $(subdir-y))
obj-y               := $(addprefix $(obj)/, $(obj-y))
obj-d               := $(patsubst %.o,%.d,$(obj-y))
obj-y               += $(subdir-obj-y)

# compiler flags
CFLAGS              += -MMD
CFLAGS              += $(INC) -mcmodel=large $(EXTRA_FLAGS) -include internal_feature.h

ASFLAGS             := -D__ASSEMBLY__ $(CFLAGS)

# check the depecdency is same
dep-check            = $(filter-out $(PHONY),$?)$(filter-out $(PHONY) $(wildcard $^),$^)
changed              = $(strip $(dep-check))

cmd                  = $(Q)set -e;                          \
                       $(MKDIR) $(dir $@);                  \
                       echo $(quiet_cmd_$(1));              \
                       $(cmd_$(1)) >> $(LOG_FILE) 2>&1;     \
                       echo cmd_$@ := $(cmd_$(1)) > $@.cmd

cmd_dep              = $(Q)set -e;                          \
                       rm -f $@;                            \
                       echo $(quiet_cmd_$(1));              \
                       $(cmd_$(1)) >> $(LOG_FILE) 2>&1;     \
                       echo cmd_$@ := $(cmd_$(1)) > $@.cmd

if_changed           = $(if $(changed), $(cmd))

quiet_cmd_cc         = "    CC    $(notdir $@)"
cmd_cc               = $(CC) -c $(CFLAGS) -o $@ $<

quiet_cmd_as         = "    ASM   $(notdir $@)"
cmd_as               = $(CC) -c $(ASFLAGS) -o $@ $<

quiet_cmd_ar         = "    AR    $(notdir $@)"
ifeq ($(AIR_MW_SUPPORT), ENABLED)
cmd_ar               = $(file >$@.in,$(filter-out $(PHONY), $^))$(AR) cDPrSsT $@ @$@.in
else
cmd_ar               = $(AR) cDPrSsT $@ $(filter-out $(PHONY), $^)
endif

$(obj)/%.o: $(src)/%.c FORCE
	$(call if_changed,cc)

$(obj)/%.o: $(src)/%.S FORCE
	$(call if_changed,as)

$(target): $(obj-y) FORCE
	$(call cmd_dep,ar)

__build: $(target)

$(target): $(subdir-obj-y) FORCE

$(subdir-obj-y): $(subdir-y) FORCE

PHONY += $(subdir-y)
$(subdir-y):
	$(Q)$(MAKE) $(build)=$@

PHONY += FORCE

FORCE:

-include $(obj-d)
.PHONY: $(PHONY)
