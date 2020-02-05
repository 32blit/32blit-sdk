
# Single user cpp file containing game code to build into firmware
PROJECT = Src/flash-loader.cpp
# Set the output filename to match the user code filename
TARGET = $(basename $(notdir $(PROJECT)))
DEBUG = 1
OPT = -Og -g -DEXTERNAL_LOAD_ADDRESS=0x90000000 -DAPPLICATION_VTOR=0x08000000 -DINITIALISE_QSPI=1 -DCDC_FIFO_BUFFERS=64
BUILD_DIR = build

CPP_SOURCES = \
Src/CDCDataStream.cpp \
Src/CDCCommandStream.cpp \
Src/CDCCommandHandler.cpp \
Src/CDCResetHandler.cpp \
Src/CDCInfoHandler.cpp

# C sources
C_SOURCES =  \
Src/main.c \
Src/stm32h7xx_it.c \
Src/stm32h7xx_hal_msp.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c \
Src/system_stm32h7xx.c \
Src/user_diskio.c \
Src/fatfs.c \
Src/fatfs_sd.c \
Src/fatfs/option/ccsbcs.c \
Middlewares/Third_Party/FatFs/src/diskio.c \
Middlewares/Third_Party/FatFs/src/ff.c \
Middlewares/Third_Party/FatFs/src/ff_gen_drv.c \
Middlewares/Third_Party/FatFs/src/option/syscall.c \
Src/gpio.cpp \
Src/display.cpp \
Src/adc.c \
Src/debug.c \
Src/dma.c \
Src/hrtim.c \
Src/i2c.c \
Src/i2c-msa301.c \
Src/i2c-bq24295.c \
Src/ltdc.c \
Src/quadspi.c \
Src/spi.c \
Src/spi-st7272a.c \
Src/sys.c \
Src/tim.c \
Src/rng.c \
Src/usb_otg.c \
Src/32blit.c \
Src/usb_device.c \
Src/usbd_conf.c \
Src/usbd_desc.c \
Src/usbd_cdc_if.c \
Src/CDCLogging.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_core.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ctlreq.c \
Middlewares/ST/STM32_USB_Device_Library/Core/Src/usbd_ioreq.c \
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src/usbd_cdc.c

# ASM sources
ASM_SOURCES =  \
startup_stm32h750xx.s


PREFIX := arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)g++
AS = $(GCC_PATH)/$(PREFIX)g++ -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
else
CC = $(PREFIX)g++
AS = $(PREFIX)g++ -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S


CPU = -mcpu=cortex-m7
FPU = -mfpu=fpv5-d16
FLOAT-ABI = -mfloat-abi=hard
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI) -specs=nosys.specs -fpermissive

# macros for gcc
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32H750xx \
-DUSE_USB_HS

# AS includes
AS_INCLUDES =

# C includes
C_INCLUDES =  \
-IInc \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/CMSIS/Include \
-IMiddlewares/Third_Party/FatFs/src \
-I../32blit \
-I$(dir $(PROJECT)) \
-IMiddlewares/ST/STM32_USB_Device_Library/Core/Inc \
-IMiddlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc

# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -fshort-wchar

# TODO: fix warnings that we're suppressing with -Wno-write-strings
CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -Wattributes -Wdouble-promotion -Wno-unused-variable -Wno-write-strings -std=c++14 -fshort-wchar

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

# link script
LDSCRIPT = STM32H750VBTx_FLASH.ld

# libraries
LIBS =  -l32blit -lc -lm -lnosys #-llua
LIBDIR = -L../32blit/build.$(PREFIX)32blit-stm32 #-L../lua
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) -static $(LIBDIR) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -Wl,--no-wchar-size-warning

# default action: build all
all: lib32blit-stm32 $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin

# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(C_SOURCES)))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(PROJECT:.cpp=.o)))
vpath %.cpp $(sort $(dir $(PROJECT)))

# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

lib32blit-stm32:
	make PREFIX=$(PREFIX) -C ../32blit

$(BUILD_DIR)/%.o: %.cpp Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

clean:
	-rm -fR $(BUILD_DIR)

flash:
	openocd -f flash.cfg

winflash:
	../openocd/bin/openocd.exe -f flash.cfg

-include $(wildcard $(BUILD_DIR)/*.d)
