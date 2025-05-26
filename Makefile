TARGET = tee_boot
BUILD_DIR = build

OPT = -O0

CC = arm-none-eabi-gcc
AS = arm-none-eabi-as
LD = arm-none-eabi-ld
SZ = arm-none-eabi-size
OBJCOPY = arm-none-eabi-objcopy

HEX = $(OBJCOPY) -O ihex
BIN = $(OBJCOPY) -O binary -S

CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

C_INCLUDES = \
			-ICore \
			-ICMSIS/Include \
			-IioLIB \
			-IioLIB/W5500 \
			-IcoapLIB \
			-ImbedTLS/include \
        	-ICMSIS/Device/ST/STM32F4xx/Include

C_SOURCES = \
			Core/blink.c\
			Core/tee.c\
			Core/boot.c \
			Core/uart.c \
			Core/clock.c \
			Core/main.c \
			Core/w5500_run.c \
			Core/ext_int.c \
			coapLIB/coap_port.c \
			coapLIB/coap.c \
			coapLIB/endpoints.c \
			ioLIB/dhcp.c \
			ioLIB/socket.c \
			ioLIB/w5500_host_config.c \
			ioLIB/w5500_phy.c \
			ioLIB/w5500_spi.c \
			ioLIB/wizchip_conf.c \
			ioLIB/W5500/w5500.c \
			mbedTLS/library/sha256.c \
			CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

C_DEFS =

AS_DEFS = 

AS_INCLUDES = 

LDSCRIPT = stm32f401.ld

ASM_SOURCES = startup_stm32f401.s


ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES)

CFLAGS = $(MCU) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections


# Generate dependency information
CFLAGS += -g -gdwarf-2
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


# libraries
LIBS = -lc -lm -lnosys
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).elf

OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c Makefile | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s Makefile | $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) Makefile
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@
	
$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@
	
$(BUILD_DIR):
	mkdir $@	

clean:
	rmdir /s $(BUILD_DIR)

-include $(wildcard $(BUILD_DIR)/*.d)
