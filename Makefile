TARGET = tee_boot
BUILD_DIR = build

OPT = -Og

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
			-I./CMSIS/Include \
        	-I./CMSIS/Device/ST/STM32F4xx/Include

C_SOURCES = \
			Core/main.c \
			CMSIS/Device/ST/STM32F4xx/Source/Templates/system_stm32f4xx.c

C_DEFS =  \
			-DSTM32F401xC

# CFLAGS = $(MCU) $(C_INCLUDES) $(OPT) -Wall

# ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

AS_DEFS = 

AS_INCLUDES = 

LDSCRIPT = stm32f401.ld

ASM_SOURCES = startup_stm32f401xc.s

# OBJS = $(SRCS:.c=.o)


ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) -Wall

CFLAGS = $(MCU) $(C_INCLUDES) $(OPT) -Wall


# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = 
LDFLAGS = $(MCU) -specs=nano.specs -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections


all: $(BUILD_DIR)/$(TARGET).bin $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).elf

# %.o: %.c
# 	$(CC) $(CFLAGS) -c $< -o $@

# %.o: %.s
# 	$(AS) -mcpu=cortex-m4 -mthumb $< -o $@

# $(TARGET).elf: $(OBJS)
# 	$(CC) $(CFLAGS) $(OBJS) -T$(LDSCRIPT) -o $@

# $(TARGET).bin: $(TARGET).elf
# 	$(OBJCOPY) -O binary $< $@

# $(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
# 	$(HEX) $< $@


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
