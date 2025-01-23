SHELL=C:\Windows\system32\cmd.exe
CC:="$(ARMGCC_INSTALL_PATH)/bin/arm-none-eabi-gcc"
dir_guard = mkdir "$(subst /,\,$(@D))" >nul 2>&1 ||:

TGT_DIR:=build\ProfileRelease
TGT_NAME:=USBVideoClassBulk
SRC_EXT:=S c

SRCS:=$(foreach ext, $(SRC_EXT), $(wildcard *.$(ext)))
OBJS:=$(SRCS:%=$(TGT_DIR)/%.o)

ifneq ($(MAKECMDGOALS),clean)
DEPS:=$(OBJS:.o=.d)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

LIBS := -lcyu3lpp -lcyfxapi -lcyu3threadx -lc -lgcc

CMN_FLAGS:= -mcpu=arm926ej-s -marm -mthumb-interwork -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall -g

CMPL_FLAGS = $(CMN_FLAGS) -I"$(FX3_INSTALL_PATH)/fw_lib/1_3_5/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"

ASM_FLAGS = $(CMPL_FLAGS) -x assembler-with-cpp

CC_FLAGS = $(CMPL_FLAGS) -D__CYU3P_TX__=1 -DCYU3P_PROFILE_EN=1 -std=gnu11

LD_FLAGS:= $(CMN_FLAGS) -T "$(FX3_INSTALL_PATH)/fw_build/fx3_fw/fx3.ld" -nostartfiles -Xlinker --gc-sections -L"$(FX3_INSTALL_PATH)/fw_lib/1_3_5/fx3_profile_release" -L"$(ARMGCC_INSTALL_PATH)/lib/gcc/arm-none-eabi/$(ARMGCC_VERSION)" -L"$(ARMGCC_INSTALL_PATH)/arm-none-eabi/lib" -Wl,-Map,"$(TGT_DIR)/$(TGT_NAME).map" -Wl,-d  -Wl,--no-wchar-size-warning -Wl,--entry,CyU3PFirmwareEntry

# Each subdirectory must supply rules for building sources it contributes
$(TGT_DIR)/%.S.o: ./%.S makefile
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	@$(dir_guard)
	$(CC) $(ASM_FLAGS) -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

$(TGT_DIR)/%.c.o: ./%.c makefile
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	@$(dir_guard)
	$(CC) $(CC_FLAGS) -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# All Target
all: $(TGT_DIR)/$(TGT_NAME).elf secondary-outputs

# Tool invocations
$(TGT_DIR)/$(TGT_NAME).elf: $(OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	$(CC) $(LD_FLAGS) -o "$(TGT_DIR)/$(TGT_NAME).elf" $(OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-@erase /S /Q "$(subst /,\,$(TGT_DIR))"\* >nul 2>&1

post-build:
	-@echo 'Generate boot-loadable binary image'
	-elf2img.exe -i $(TGT_DIR)/$(TGT_NAME).elf -o $(TGT_DIR)/$(TGT_NAME).img -v
#	fc ProfileRelease\$(TGT_NAME).img ProfileRelease\$(TGT_NAME).img

secondary-outputs:

.PHONY: all clean dependents
.SECONDARY: post-build

