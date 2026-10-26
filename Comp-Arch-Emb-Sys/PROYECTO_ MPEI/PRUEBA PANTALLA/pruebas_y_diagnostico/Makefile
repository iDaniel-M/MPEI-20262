# Makefile - PRUEBA PANTALLA P10 RGB (STM32F446RE, bare metal, GNU toolchain)
#
#   make                 compila (1 panel)
#   make N_PANELS=2      compila para 2 paneles en cadena
#   make flash           graba con st-flash
#   make flash-ocd       graba con OpenOCD (alternativa)
#   make clean           borra la carpeta build/
#   make size            muestra cuánta FLASH/RAM se usa

TARGET   = prueba_pantalla
BUILD    = build
N_PANELS ?= 1

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

SRCS = startup.c main.c
OBJS = $(addprefix $(BUILD)/,$(SRCS:.c=.o))

# --- Núcleo: Cortex-M4 con FPU de precisión simple ---
CPU = -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16

# --- Compilación ---
#  -ffreestanding                 : no hay sistema operativo ni libc
#  -fno-tree-loop-distribute-patterns : evita que GCC convierta lazos en
#                                   llamadas a memset/memcpy (no hay libc)
#  -ffunction-sections/-fdata-sections + --gc-sections : quita código no usado
CFLAGS  = $(CPU) -std=c11 -O1 -g3 -Wall -Wextra \
          -ffreestanding -fno-tree-loop-distribute-patterns \
          -ffunction-sections -fdata-sections \
          -DN_PANELS=$(N_PANELS)

# --- Enlazado ---
#  -nostdlib -nostartfiles : no usar libc ni el crt0 de GCC (el arranque es startup.c)
LDFLAGS = $(CPU) -T stm32f446.ld -nostdlib -nostartfiles \
          -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map

all: $(BUILD)/$(TARGET).elf $(BUILD)/$(TARGET).bin size

$(BUILD):
	mkdir -p $(BUILD)

# Recompilar todo si cambia N_PANELS o un header
$(BUILD)/%.o: %.c regs.h Makefile | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(TARGET).elf: $(OBJS) stm32f446.ld
	$(CC) $(OBJS) $(LDFLAGS) -o $@

$(BUILD)/$(TARGET).bin: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O binary $< $@

size: $(BUILD)/$(TARGET).elf
	$(SIZE) $<

flash: $(BUILD)/$(TARGET).bin
	st-flash --reset write $< 0x08000000

flash-ocd: $(BUILD)/$(TARGET).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg \
	        -c "program $< verify reset exit"

# --- Firmware de barrido de drivers (scan.c) ---
#   make scan        compila build/scan.bin
#   make flash-scan  graba con st-flash
SCAN = scan
$(BUILD)/$(SCAN).elf: $(BUILD)/startup.o $(BUILD)/scan.o stm32f446.ld
	$(CC) $(BUILD)/startup.o $(BUILD)/scan.o $(LDFLAGS) -o $@

$(BUILD)/$(SCAN).bin: $(BUILD)/$(SCAN).elf
	$(OBJCOPY) -O binary $< $@

scan: $(BUILD)/$(SCAN).bin
	$(SIZE) $(BUILD)/$(SCAN).elf

flash-scan: $(BUILD)/$(SCAN).bin
	st-flash --reset write $< 0x08000000

# --- Banco de pruebas del protocolo (pruebas.c), una variante por compilación ---
#   make pruebas VAR=3     compila build/pruebas.bin con la variante 3
#   make flash-pruebas     graba esa variante
# Se recompila siempre (FORCE) porque VAR cambia y make no lo detecta solo.
#   make pruebas VAR=3 MIRROR=1   misma variante con el cable espejado (ver pruebas.c)
VAR ?= 0
MIRROR ?= 0
ROW ?= 0
$(BUILD)/pruebas.o: pruebas.c regs.h FORCE | $(BUILD)
	$(CC) $(CFLAGS) -DVAR=$(VAR) -DMIRROR=$(MIRROR) -DROW=$(ROW) -c $< -o $@

$(BUILD)/pruebas.elf: $(BUILD)/startup.o $(BUILD)/pruebas.o stm32f446.ld
	$(CC) $(BUILD)/startup.o $(BUILD)/pruebas.o $(LDFLAGS) -o $@

$(BUILD)/pruebas.bin: $(BUILD)/pruebas.elf
	$(OBJCOPY) -O binary $< $@

pruebas: $(BUILD)/pruebas.bin
	$(SIZE) $(BUILD)/pruebas.elf

flash-pruebas: $(BUILD)/pruebas.bin
	st-flash --reset write $< 0x08000000

# --- Demo serpiente RGB (serpiente.c) ---
#   make serpiente        compila build/serpiente.bin
#   make flash-serpiente  graba con st-flash
$(BUILD)/serpiente.elf: $(BUILD)/startup.o $(BUILD)/serpiente.o stm32f446.ld
	$(CC) $(BUILD)/startup.o $(BUILD)/serpiente.o $(LDFLAGS) -o $@

$(BUILD)/serpiente.bin: $(BUILD)/serpiente.elf
	$(OBJCOPY) -O binary $< $@

serpiente: $(BUILD)/serpiente.bin
	$(SIZE) $(BUILD)/serpiente.elf

flash-serpiente: $(BUILD)/serpiente.bin
	st-flash --reset write $< 0x08000000

FORCE:

clean:
	rm -rf $(BUILD)

.PHONY: all size flash flash-ocd scan flash-scan pruebas flash-pruebas serpiente flash-serpiente clean FORCE
