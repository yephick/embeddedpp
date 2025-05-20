SHELL=C:\Windows\system32\cmd.exe
ARMGCC_INSTALL_PATH:=$(VSAPPIDDIR)../../Linux/gcc_arm
ARMGCC_VERSION:=9.3.1
CC:="$(ARMGCC_INSTALL_PATH)/bin/arm-none-eabi-gcc"
CXX:="$(ARMGCC_INSTALL_PATH)/bin/arm-none-eabi-g++"
LD:="$(ARMGCC_INSTALL_PATH)/bin/arm-none-eabi-g++"
SDK:=../CY_SDK_1_3_5
dir_guard = mkdir "$(subst /,\,$(@D))" >nul 2>&1 ||:

BLD_TYPE?=Release
$(info build type is [$(BLD_TYPE)])
TGT_DIR:=build/$(BLD_TYPE)
TGT_NAME:=USBVideoClassBulk
SRC_EXT:=S c cpp

SRCS:=$(foreach ext, $(SRC_EXT), $(wildcard *.$(ext)))
OBJS:=$(SRCS:%=$(TGT_DIR)/%.o)

ifneq ($(MAKECMDGOALS),clean)
DEPS:=$(OBJS:.o=.d)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

LIBS := -lcyu3lpp -lcyfxapi -lcyu3threadx

CMN_FLAGS:= -mcpu=arm926ej-s -marm -mthumb-interwork -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -Wall

ifneq (,$(filter $(BLD_TYPE),Debug ProfileDebug))
  CMN_FLAGS += -O0 -g3
else
  CMN_FLAGS += -Os -g
endif

ifneq (,$(filter $(BLD_TYPE),ProfileRelease ProfileDebug))
  CMPL_FLAGS += -DCYU3P_PROFILE_EN=1
endif

CMPL_FLAGS = $(CMN_FLAGS) -D__CYU3P_TX__=1 -I"$(SDK)/inc" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@"

ASM_FLAGS = $(CMPL_FLAGS) -x assembler-with-cpp

CC_FLAGS = $(CMPL_FLAGS) -std=gnu11
CXX_FLAGS = $(CMPL_FLAGS) -std=gnu++11 -fno-exceptions -fno-rtti -fabi-version=0

LD_FLAGS:= $(CXX_FLAGS) -T fx3cpp.ld -nostartfiles -Xlinker --gc-sections -L"$(ARMGCC_INSTALL_PATH)/lib/gcc/arm-none-eabi/$(ARMGCC_VERSION)" -L"$(ARMGCC_INSTALL_PATH)/arm-none-eabi/lib" -Wl,-Map,"$(TGT_DIR)/$(TGT_NAME).map" -Wl,-d  -Wl,--no-wchar-size-warning -Wl,--entry,CyU3PFirmwareEntry

ifeq ($(BLD_TYPE),ProfileRelease)
  LD_FLAGS += -L"$(SDK)/fx3_profile_release"
else ifeq ($(BLD_TYPE),ProfileDebug)
  LD_FLAGS += -L"$(SDK)/fx3_profile_debug"
else ifeq ($(BLD_TYPE),Debug)
  LD_FLAGS += -L"$(SDK)/fx3_debug"
else
  LD_FLAGS += -L"$(SDK)/fx3_release"
endif

# Each subdirectory must supply rules for building sources it contributes
$(TGT_DIR)/%.S.o: ./%.S makefile
	@echo $<
	@$(dir_guard)
	@$(CC) $(ASM_FLAGS) -c -o "$@" "$<"

$(TGT_DIR)/%.c.o: ./%.c makefile
	@echo $<
	@$(dir_guard)
	@$(CC) $(CC_FLAGS) -c -o "$@" "$<"

$(TGT_DIR)/%.cpp.o: ./%.cpp makefile
	@echo $<
	@$(dir_guard)
	@$(CXX) $(CXX_FLAGS) -c -o "$@" "$<"

all: $(TGT_DIR)/$(TGT_NAME).img

$(TGT_DIR)/$(TGT_NAME).elf: $(OBJS)
	@echo $@
	@$(LD) $(LD_FLAGS) -o "$(TGT_DIR)/$(TGT_NAME).elf" $(OBJS) $(LIBS)
	@"$(ARMGCC_INSTALL_PATH)/bin/arm-none-eabi-size" --format=berkley "$@"

clean:
	-@erase /S /Q "$(subst /,\,$(TGT_DIR))"\* >nul 2>&1

$(TGT_DIR)/$(TGT_NAME).img: $(TGT_DIR)/$(TGT_NAME).elf
	@echo 'Generate boot-loadable binary image'
	@"$(SDK)/bin/elf2img.exe" -i "$<" -o "$@"
#	fc $(BLD_TYPE)\$(TGT_NAME).img "$@"

deploy: $(TGT_DIR)/$(TGT_NAME).img
	@echo deploying image to RAM
	@"$(SDK)/bin/cyfwprog.exe" -fw "$<" -dest SYSTEM

.PHONY: all clean deploy
