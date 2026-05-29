squote                  := '
escsq                   = $(subst $(squote),'\$(squote)',$1)
dot-target              = $(patsubst %/,%,$(dir $@))/.$(notdir $@)
rm-src-dir              = $(patsubst $(SRC_DIR)/%,%,$1)
# short cut for make -f build.mk object=$(obj)
build                   := -f $(MAKE_DIR)/build.mk object

# if there has any prerequisites(only file) has been changed? if yes, then set $(any-prereq) as changed files
any-prereq              = $(filter-out $(PHONY),$?)

echo-cmd                = $(if $(QUIET)cmd_$(1),echo '  $(call escsq,$($(QUIET)cmd_$(1)))';)
cmd                     = @set -e; $(echo-cmd) $(cmd_$(1)) >> $(LOG_FILE) 2>&1
cmd-only                = @set -e; $(echo-cmd) $(cmd_$(1))

if_changed              = $(if $(any-prereq),                                           \
                          $(cmd);printf '%s\n' '$@ : $(cmd_$(1))' > $(dot-target).cmd,  \
                          @:)

quiet_cmd_ar_builtin    = AR          $@
cmd_ar_builtin          = rm -rf $@; $(AR) cDPrSsT $@ $(filter-out $(PHONY), $^)

cflags                  = $(CFLAGS) $(EXTRA_CFLAGS)

quiet_cmd_cc_o_c        = CC          $@
cmd_cc_o_c              = $(CC) $(cflags) -c -o $@ $<

quiet_cmd_cc_o_s        = ASM         $@
cmd_cc_o_s              = $(CC) $(cflags) -c -o $@ $<

quiet_cmd_objcopy       = OBJCOPY     $(call rm-src-dir,$(TARGET_OUT_DIR)/$(TARGET).bin)
cmd_objcopy             = $(OBJCOPY) -O binary -S $(TARGET_OUT_DIR)/$(TARGET).elf $(TARGET_OUT_DIR)/$(TARGET).bin &&    \
                          cp $(TARGET_OUT_DIR)/$(TARGET).bin $(SRC_DIR) &&                                              \
                          cp $(LOG_FILE) $(SRC_DIR)

quiet_cmd_objdump       = OBJDUMP     $(call rm-src-dir,$(TARGET_OUT_DIR)/$(TARGET).elf.txt)
cmd_objdump             = $(OBJDUMP) -t -x -d -C -h -S $(TARGET_OUT_DIR)/$(TARGET).elf > $(TARGET_OUT_DIR)/$(TARGET).elf.txt

quiet_cmd_clean         = CLEAN       $(call rm-src-dir,$(TARGET_OUT_DIR))      \
                          $(call rm-src-dir,$(LOG_FILE))                        \
                          $(call rm-src-dir,$(SRC_DIR)/$(TARGET).bin)           \
                          $(call rm-src-dir,$(SRC_DIR)/log.Makefile)
cmd_clean               = rm -rf $(TARGET_OUT_DIR) $(LOG_FILE) $(SRC_DIR)/$(TARGET).bin $(SRC_DIR)/log.Makefile

quiet_cmd_lib           = AR          $@
cmd_lib                 = rm -rf $@; \
                          $(AR) cDPrSsT $@ $(addprefix $(TARGET_OUT_DIR)/,$(addsuffix built-in.a, $^));