include $(MAKE_DIR)/build_include.mk

all: __build

src                     := $(object)
src                     := $(patsubst %/,%,$(src))
obj                     := $(object)
obj                     := $(patsubst $(SRC_DIR)/%,%,$(obj))
obj                     := $(TARGET_OUT_DIR)/$(obj)
obj                     := $(patsubst %/,%,$(obj))

target-build-dir        := $(if $(filter /%,$(src)),$(src),$(SRC_DIR)/$(src))
target-build-mk         := $(wildcard $(target-build-dir)/Makefile)

ifeq ($(target-build-mk),)
$(error "no Makefile to make target: $(obj)")
endif

include $(target-build-mk)

target                  := $(addsuffix built-in.a,$(obj)/)
subdir-obj-y            := $(addsuffix built-in.a,$(addprefix $(obj)/,$(subdir-y)))
subdir-y                := $(addprefix $(src)/,$(subdir-y))
obj-y                   := $(addprefix $(obj)/,$(obj-y))
obj-d                   := $(patsubst %.o,%.d,$(obj-y))
obj-y                   += $(subdir-obj-y)

# aggregate objects to archives
$(target): $(obj-y) FORCE
	$(call cmd,ar_builtin); \
	printf '%s\n' '$@ : $(cmd_ar_builtin)' > $(dot-target).cmd

# compile .c to objects
$(obj)/%.o: $(src)/%.c FORCE
	$(call if_changed,cc_o_c)

# assemble .s to objects
$(obj)/%.o: $(src)/%.S FORCE
	$(call if_changed,cc_o_s)

__build: $(target)
	@:

$(target): $(subdir-obj-y)

$(subdir-obj-y): $(subdir-y)

$(subdir-y):
	$(Q)$(MAKE) $(build)=$@

PHONY                   += $(subdir-y) FORCE

FORCE:
# if obj directory is not exist, create for it
ifeq ($(wildcard $(obj)),)
$(shell mkdir -p $(obj))
endif

-include $(obj-d)
.PHONY: $(PHONY)
