SHELL=C:\Windows\system32\cmd.exe

TGT_NAME := USBVideoClassBulk

VSAPPIDDIR          ?= C:/Program Files/Microsoft Visual Studio/2022/Community/Common7/IDE/
ARMGCC_INSTALL_PATH ?= $(VSAPPIDDIR)../../Linux/gcc_arm
ARMGCC_VERSION      ?= 9.3.1
CLANG_ROOT          ?= $(VSAPPIDDIR)../../VC/Tools/Llvm
CY_SDK_ROOT         ?= ../CY_SDK_1_3_5
BLD_TYPE            ?= Release
$(info build type is [$(BLD_TYPE)])

CC  := "$(CLANG_ROOT)/bin/clang"
CXX := "$(CLANG_ROOT)/bin/clang++"
LD  := "$(CLANG_ROOT)/bin/clang++"

dir_guard = mkdir "$(subst /,\,$(@D))" >nul 2>&1 ||:

TGT_DIR := build/$(BLD_TYPE)
SRC_EXT := S c cpp

SRCS := $(foreach ext, $(SRC_EXT), $(wildcard *.$(ext)))
OBJS := $(SRCS:%=$(TGT_DIR)/%.o)

ifneq ($(MAKECMDGOALS),clean)
DEPS := $(OBJS:.o=.d)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

LIBS := -lcyu3lpp -lcyfxapi -lcyu3threadx -lc -lgcc

# Compiler and linker flags split and sorted, one per line
CMPL_FLAGS  = -D__CYU3P_TX__=1                   # Define macro for ThreadX usage
CMPL_FLAGS += -DCYU3P_PROFILE_EN=1               # Enable profiling in Cypress SDK
CMPL_FLAGS += -I"$(CY_SDK_ROOT)/inc"             # Add Cypress SDK include directory
CMPL_FLAGS += -MMD                               # Generate dependency file for each source
CMPL_FLAGS += -MF"$(@:%.o=%.d)"                  # Name of dependency file
CMPL_FLAGS += -MP                                # Add phony targets for dependencies
CMPL_FLAGS += -MT"$@"                            # Set target name in dependency file
CMPL_FLAGS += -Wall                              # Enable all common warnings
CMPL_FLAGS += -Wdouble-promotion                 # Warn on implicit double promotion
CMPL_FLAGS += -Werror                            # Treat all warnings as errors
CMPL_FLAGS += -Wextra                            # Enable extra warnings
CMPL_FLAGS += -Wfloat-equal                      # Warn on floating point equality tests
CMPL_FLAGS += -Wformat=2                         # Enable strict format string checking
CMPL_FLAGS += -Wpedantic                         # Warn on non-standard language usage
CMPL_FLAGS += -Wmissing-include-dirs             # Warn if an include directory is missing
CMPL_FLAGS += -Wnull-dereference                 # Warn on possible null pointer dereference
CMPL_FLAGS += -Wshadow                           # Warn if a variable shadows another
CMPL_FLAGS += -Wunused                           # Warn on unused variables/functions
CMPL_FLAGS += -Wuninitialized                    # Warn on uninitialized variables
CMPL_FLAGS += -Wconversion                       # Warn on implicit type conversions
CMPL_FLAGS += -fdata-sections                    # Place each data item in its own section
CMPL_FLAGS += -ffat-lto-objects                  # Generate fat LTO objects for link-time optimization
CMPL_FLAGS += -ffast-math                        # Enable fast (non-IEEE) math optimizations
CMPL_FLAGS += -ffunction-sections                # Place each function in its own section
CMPL_FLAGS += -flto                              # Enable link-time optimization
CMPL_FLAGS += -fmerge-all-constants              # Merge identical constants
CMPL_FLAGS += -fmessage-length=0                 # Do not wrap compiler diagnostic messages
CMPL_FLAGS += -fno-builtin                       # Disable built-in functions
CMPL_FLAGS += -fno-common                        # Do not use common symbol section
CMPL_FLAGS += -fno-math-errno                    # Do not set errno after math functions
CMPL_FLAGS += -fno-stack-protector               # Disable stack protection
CMPL_FLAGS += -fno-unroll-loops                  # Disable loop unrolling
CMPL_FLAGS += -fomit-frame-pointer               # Omit the frame pointer for optimization
CMPL_FLAGS += -fshort-enums                      # Use smallest possible size for enums
CMPL_FLAGS += -fsigned-char                      # Treat 'char' as signed
CMPL_FLAGS += -fstrict-enums                     # Optimize based on strict enum usage
CMPL_FLAGS += -funsafe-math-optimizations        # Allow unsafe math optimizations
CMPL_FLAGS += -mabi=apcs-gnu                     # Use GNU APCS ABI
CMPL_FLAGS += -march=armv5te                     # Target ARMv5TE architecture
CMPL_FLAGS += -marm                              # Generate ARM (not Thumb) instructions
CMPL_FLAGS += -mcpu=arm926ej-s                   # Target ARM926EJ-S CPU
CMPL_FLAGS += -mfloat-abi=soft                   # Use software floating point ABI
CMPL_FLAGS += -msoft-float                       # Use software floating point
CMPL_FLAGS += -mfpu=softvfp                      # Use software VFP floating point
CMPL_FLAGS += -mno-thumb                         # Disable Thumb instruction set
CMPL_FLAGS += -target arm-none-eabi              # Target ARM EABI bare-metal
CMPL_FLAGS += --sysroot="$(ARMGCC_INSTALL_PATH)/arm-none-eabi/" # Set sysroot for cross-compilation

# Conditional flags
ifneq (,$(filter $(BLD_TYPE),Debug ProfileDebug))
  CMPL_FLAGS += -O0                              # No optimization (debug builds)
  CMPL_FLAGS += -g3                              # Maximum debug info
else
  CMPL_FLAGS += -Os                              # Optimize for size (release builds)
  CMPL_FLAGS += -g                               # Generate debug info
endif

ifneq (,$(filter $(BLD_TYPE),ProfileRelease ProfileDebug))
  CMPL_FLAGS += -DCYU3P_PROFILE_EN=1             # Enable profiling in Cypress SDK
endif

ASM_FLAGS = $(CMPL_FLAGS)
ASM_FLAGS += -DINTER=1                           # Define macro INTER for assembly
ASM_FLAGS += -x assembler-with-cpp               # Treat input as assembly with C preprocessor

CC_FLAGS = $(CMPL_FLAGS)
CC_FLAGS += -std=c17                             # Use C17 standard

ARM_NONE_EABI = $(ARMGCC_INSTALL_PATH)/arm-none-eabi/
CXX_FLAGS = $(CMPL_FLAGS)
CXX_FLAGS += -fno-exceptions                     # Disable C++ exceptions
CXX_FLAGS += -fno-rtti                           # Disable C++ RTTI
CXX_FLAGS += -nostdlib                           # Do not use standard C++ library
CXX_FLAGS += -I"$(ARM_NONE_EABI)/include/c++/$(ARMGCC_VERSION)/arm-none-eabi" # C++ stdlib include path
CXX_FLAGS += -I"$(ARM_NONE_EABI)/include/c++/$(ARMGCC_VERSION)"               # C++ stdlib include path
CXX_FLAGS += -fc++-abi=arm                       # Use ARM C++ ABI
CXX_FLAGS += -flto=full                          # Full link-time optimization
CXX_FLAGS += -fno-asynchronous-unwind-tables     # Disable async unwind tables
CXX_FLAGS += -fno-unwind-tables                  # Disable unwind tables
CXX_FLAGS += -fvisibility-inlines-hidden         # Hide inline function symbols by default
CXX_FLAGS += -std=c++20                          # Use C++20 standard
CXX_FLAGS += -Wctor-dtor-privacy                 # Warn on private ctors/dtors
CXX_FLAGS += -Wdeprecated-copy                   # Warn on deprecated copy operations
CXX_FLAGS += -Weffc++                            # Warn on effective C++ issues
CXX_FLAGS += -Wno-redundant-move                 # Suppress redundant move warnings
CXX_FLAGS += -Wno-write-strings                  # Suppress write-strings warnings
CXX_FLAGS += -Wnon-virtual-dtor                  # Warn on non-virtual destructors
CXX_FLAGS += -Wsuggest-override                  # Warn if override is suggested
CXX_FLAGS += -Wno-writable-strings               # Suppress writable-strings warnings

LD_FLAGS =
LD_FLAGS += $(CXX_FLAGS)                         # Use C++ flags for linking
LD_FLAGS += -L"$(ARMGCC_INSTALL_PATH)/arm-none-eabi/lib" # Linker search path for ARM EABI libs
LD_FLAGS += -L"$(ARMGCC_INSTALL_PATH)/lib/gcc/arm-none-eabi/$(ARMGCC_VERSION)" # Linker search path for GCC libs
LD_FLAGS += -T fx3cpp.ld                         # Use fx3cpp.ld linker script
LD_FLAGS += -Wl,--entry,CyU3PFirmwareEntry       # Set firmware entry point
LD_FLAGS += -Wl,--gc-sections                    # Remove unused sections
LD_FLAGS += -Wl,--icf=safe                       # Safe identical code folding
LD_FLAGS += -Wl,--hash-style=gnu                 # Use GNU hash style
LD_FLAGS += -Wl,--no-undefined                   # Fail if there are undefined symbols
LD_FLAGS += -Wl,--strip-all                      # Strip all symbols from output
LD_FLAGS += -Wl,-Map,"$(TGT_DIR)/$(TGT_NAME).map" # Generate a map file
LD_FLAGS += -Wl,-d                               # Print input files during linking
LD_FLAGS += -fuse-ld=ld                          # Use 'ld' as the linker
LD_FLAGS += -z nognustack                        # Mark stack as non-executable
LD_FLAGS += -Xlinker --gc-sections               # Remove unused sections (redundant, but safe)

ifeq ($(BLD_TYPE),ProfileRelease)
  LD_FLAGS += -L"$(CY_SDK_ROOT)/fx3_profile_release" # Cypress SDK profile release libs
else ifeq ($(BLD_TYPE),ProfileDebug)
  LD_FLAGS += -L"$(CY_SDK_ROOT)/fx3_profile_debug"   # Cypress SDK profile debug libs
else ifeq ($(BLD_TYPE),Debug)
  LD_FLAGS += -L"$(CY_SDK_ROOT)/fx3_debug"           # Cypress SDK debug libs
else
  LD_FLAGS += -L"$(CY_SDK_ROOT)/fx3_release"         # Cypress SDK release libs
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
	@"$(CY_SDK_ROOT)/bin/elf2img.exe" -i "$<" -o "$@"
#	fc $(BLD_TYPE)\$(TGT_NAME).img "$@"

deploy: $(TGT_DIR)/$(TGT_NAME).img
	@echo deploying image to RAM
	@"$(CY_SDK_ROOT)/bin/cyfwprog.exe" -fw "$<" -dest SYSTEM

.PHONY: all clean deploy
