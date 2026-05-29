################################################################################
TARGET_EXE              := sdk_ref

################################################################################
ifeq ($(findstring SHARED_LIB,$(BUILD_TYPE)),SHARED_LIB)
SDK_LIB                 := $(realpath $(AIR_SYSTEM_BUILD)/image/libair_sdk.so)
else
SDK_LIB                 := $(realpath $(AIR_SYSTEM_BUILD)/image/air_sdk.a)
endif
CINT_LIB                := $(AIR_SYSTEM)/app/sdk_cint/root_demo.o
CINT_LIB                += $(AIR_SYSTEM)/app/sdk_cint/cint_sdk.o
CINT_LIB                += $(AIR_SYSTEM)/app/sdk_cint/main.o
CINT_LIB                += $(AIR_SYSTEM)/app/sdk_cint/sdk_dict.o

################################################################################
SRCS_TOTAL              := $(wildcard $(SDK_REF)/linux/*.c)
SRCS_TOTAL              += $(wildcard $(SDK_REF)/linux/common/*.c)
OBJS_TOTAL              := $(SRCS_TOTAL:%.c=%.o)
DEPS_TOTAL              := $(SRCS_TOTAL:%.c=%.d)

EXTRA_CFLAGS            += -lpthread
EXTRA_CFLAGS            += -g
EXTRA_CFLAGS            += -I$(SDK_REF)/linux/inc

################################################################################
# Standard things
sinclude $(TGT_DEPS_TOTAL)

ifeq ($(findstring ENABLED,$(AIR_CODE_COVERAGE_CHECK)),ENABLED)
EXTRA_CFLAGS            += --coverage
endif

ifeq ($(findstring cint,$(TARGET_NAME)),cint)
BUILD_LIBS              += $(CINT_LIB)
EXTRA_CFLAGS            += -L $(CINT_ROOT_USRLIB) -L $(CINT_ROOT_LIB) -l:libpcre.so.0 -lCint -lNew -lCore -lRint -lgcc_s -lstdc++ -lm
EXTRA_CFLAGS            += -Wl,-rpath $(CINT_ROOT_USRLIB) -Wl,-rpath $(CINT_ROOT_LIB)
endif

################################################################################
TGT_OBJS_TOTAL          := $(addprefix $(AIR_SYSTEM_BUILD)/,$(subst $(AIR_SYSTEM),,$(OBJS_TOTAL)))
TGT_DEPS_TOTAL          := $(addprefix $(AIR_SYSTEM_BUILD)/,$(subst $(AIR_SYSTEM),,$(DEPS_TOTAL)))

################################################################################
compile::$(TARGET_EXE)

$(TARGET_EXE):$(TGT_OBJS_TOTAL) $(SDK_LIB)
	$(CC) -o $(TARGET_EXE) $(TGT_OBJS_TOTAL) $(BUILD_LIBS) $(SDK_LIB) $(EXTRA_CFLAGS) $(CFLAGS)

$(AIR_SYSTEM_BUILD)/%.o:../../%.c
	$(TEST_PATH) $(dir $@) || $(MKDIR) $(dir $@)
	$(CC) $(EXTRA_CFLAGS) $(CFLAGS) -c -o $@ $< >> $(AIR_LOG) 2>&1
	$(CC) -MM -MT $@ $(EXTRA_CFLAGS) $(CFLAGS) $< > $(basename $@).d

install:
	$(CP) -p $(TARGET_EXE) $(AIR_SYSTEM_BUILD)/image
	$(CP) -p $(TARGET_EXE) $(AIR_SYSTEM_BUILD)/image/$(TARGET_EXE)_nostrip
	$(STRIP) $(AIR_SYSTEM_BUILD)/image/$(TARGET_EXE)

clean:
	$(RM) $(TGT_OBJS_TOTAL) $(TGT_DEPS_TOTAL) *.gcno *.gcda $(TARGET_EXE) $(AIR_SYSTEM_BUILD)/image/$(TARGET_EXE)*

distclean:clean
