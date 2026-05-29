include $(MAKE_DIR)/build_include.mk

out-elf                 := $(elf)
src-lib                 := $(lib)

all: __link

__link: $(out-elf)

SAGTOOL                 := $(TOOL_DIR)/nds_ldsag
SAGSCRIPT               := $(SRC_DIR)/project/$(PROJ)/linker_script.sag
LDSCRIPT                := $(SRC_DIR)/project/$(PROJ)/linker_script.ld

# -static : do not link against shared libraries
# -Wl,<options> : Pass comma-separated <options> on to the linker
# --gc-sections : Remove unused sections (on some targets)
LD_FLAGS                := -T $(LDSCRIPT) $(OPTIM) -static -nostartfiles -Wl,--gc-sections,--undefine=__rtos_signature_freertos_v10_1_1
# -Map FILE : Write a map file
LD_FLAGS                += -Wl,-Map=$(TARGET_OUT_DIR)/$(TARGET).map

# Add `-fno-delete-null-pointer-checks` flag if the compiler supports it.
# GCC assumes that programs cannot safely dereference null pointers,
# and that no code or data element resides there.
# However, 0x0 is the vector table memory location, so the test must not be removed.
ifeq ($(shell $(CC) -fno-delete-null-pointer-checks -E - 2>/dev/null >/dev/null </dev/null ; echo $$?),0)
LD_FLAGS                += -fno-delete-null-pointer-checks
endif

ld-flags                := $(LD_FLAGS)
quiet_cmd_link          = LINK        $@
cmd_link                = $(CC) $(ld-flags) -o $@ $<
quiet_cmd_ld            = LD          $(call rm-src-dir,$(LDSCRIPT))
cmd_ld                  = $(SAGTOOL) $(SAGSCRIPT) -o $(LDSCRIPT)
$(out-elf): $(src-lib) FORCE
	$(call cmd,ld)
	$(call cmd,link)

quiet_cmd_clean_link    = CLEAN       $(call rm-src-dir,$(LDSCRIPT))
cmd_clean_link          = rm -f $(LDSCRIPT)
clean: FORCE
	$(call cmd,clean_link)

PHONY                   += FORCE
FORCE:

.PHONY: $(PHONY)