INC                         += -I$(CROSS_COMPILE_PATH)/../lib/gcc/nds32le-elf/4.9.4/include
INC                         += -I$(APP_DIR)/common/include
INC                         += -I$(APP_DIR)/ace/include
ifneq ($(CONFIG_NDS32),)
INC                         += -I$(BSP_DIR)/arch/NDS32
endif
INC                         += -I$(BSP_DIR)/include
INC                         += -I$(BSP_DIR)/platform/$(CONFIG_PLATFORM)
INC                         += -include $(BSP_DIR)/platform/$(CONFIG_PLATFORM)/internal_feature.h
INC                         += -I$(KERNEL_DIR)/include
INC                         += -I$(PROJ_DIR)/$(PROJ)
INC                         += -I$(SDK_DIR)/bsp/platform/$(CONFIG_PLATFORM)

EXTRA_CFLAGS                += $(INC)
export EXTRA_CFLAGS