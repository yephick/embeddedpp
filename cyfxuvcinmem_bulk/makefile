TGT_DIR = ./ProfileRelease

-include ./makefile.init

RM := cs-rm -rf

# All of the sources participating in the build are defined here
O_SRCS := 
C_SRCS := 
S_UPPER_SRCS := 
OBJ_SRCS := 
ASM_SRCS := 
OBJS := 
C_DEPS := 
ASM_DEPS := 
S_UPPER_DEPS := 

# Every subdirectory with source files must be described here
SUBDIRS := \
. \

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
./cyfxtx.c \
./cyfxuvcdscr.c \
./cyfxuvcinmem.c \
./cyfxuvcvidframes.c 

S_UPPER_SRCS += \
./cyfx_gcc_startup.S 

OBJS += \
./$(TGT_DIR)/cyfx_gcc_startup.o \
./$(TGT_DIR)/cyfxtx.o \
./$(TGT_DIR)/cyfxuvcdscr.o \
./$(TGT_DIR)/cyfxuvcinmem.o \
./$(TGT_DIR)/cyfxuvcvidframes.o 

C_DEPS += \
./$(TGT_DIR)/cyfxtx.d \
./$(TGT_DIR)/cyfxuvcdscr.d \
./$(TGT_DIR)/cyfxuvcinmem.d \
./$(TGT_DIR)/cyfxuvcvidframes.d 

S_UPPER_DEPS += \
./$(TGT_DIR)/cyfx_gcc_startup.d 


# Each subdirectory must supply rules for building sources it contributes
$(TGT_DIR)/%.o: ./%.S
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM GNU Assembler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -marm -mthumb-interwork -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -x assembler-with-cpp -I"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\/fw_lib/1_3_5/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

$(TGT_DIR)/%.o: ./%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=arm926ej-s -marm -mthumb-interwork -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -D__CYU3P_TX__=1 -DCYU3P_PROFILE_EN=1 -I"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\/fw_lib/1_3_5/inc" -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


USER_OBJS :=

LIBS := -lcyu3lpp -lcyfxapi -lcyu3threadx -lc -lgcc


ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
endif

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: $(TGT_DIR)/USBVideoClassBulk.elf secondary-outputs

# Tool invocations
$(TGT_DIR)/USBVideoClassBulk.elf: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross ARM C Linker'
	arm-none-eabi-gcc -mcpu=arm926ej-s -marm -mthumb-interwork -O3 -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall  -g -T "C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\/fw_build/fx3_fw/fx3.ld" -nostartfiles -Xlinker --gc-sections -L"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\/fw_lib/1_3_5/fx3_profile_release" -L"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\ARM GCC\/lib/gcc/arm-none-eabi/4.8.1" -L"C:\Program Files (x86)\Cypress\EZ-USB FX3 SDK\1.3\ARM GCC\/arm-none-eabi/lib" -Wl,-Map,"$(TGT_DIR)/USBVideoClassBulk.map" -Wl,-d  -Wl,--no-wchar-size-warning -Wl,--entry,CyU3PFirmwareEntry -o "$(TGT_DIR)/USBVideoClassBulk.elf" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(TGT_DIR)/*
	-@echo ' '

post-build:
	-@echo 'Generate boot-loadable binary image'
	-elf2img.exe -i $(TGT_DIR)/USBVideoClassBulk.elf -o $(TGT_DIR)/USBVideoClassBulk.img -v
	-@echo ' '

secondary-outputs:

.PHONY: all clean dependents
.SECONDARY: post-build

