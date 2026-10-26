# Makefile - el del ejercicio 21-SPI-4 del profesor, con estos cambios:
#  - sin CMSIS (no se incluye STM32utils ni system_stm32f4xx.c): registros en inc/stm32f446_regs.h
#  - -mthumb explícito y se genera main.bin
#  - flash con st-flash (el ST-LINK está conectado a WSL); flash-ocd queda con OpenOCD
#
#   make         compila main.elf y main.bin
#   make flash   graba la placa
#   make clean   borra lo compilado

APPNAME = main

CC =  arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
OBJCOPY =  arm-none-eabi-objcopy
ARCH =  -mcpu=cortex-m4 -mthumb
LINKERSCRIPT = linker_script.ld
BUILD = build

# Carpetas donde buscar los *.h
INCLUDE = -I./inc
INCLUDE += -I.

C_SRC = $(wildcard *.c)
C_SRC += $(wildcard src/*.c)
ASM_SRC = $(wildcard *.S)
# Los .o van a build/ (solo el nombre del archivo, sin carpeta)
OBJS = $(patsubst %.c, $(BUILD)/%.o, $(notdir $(C_SRC)))
OBJS += $(patsubst %.S, $(BUILD)/%.o, $(notdir  $(ASM_SRC)))

# Dónde buscar los *.c y *.S
vpath %.c $(sort $(dir $(C_SRC)))
vpath %.S $(sort $(dir $(ASM_SRC)))

CFLAGS+=-O0 -g
CFLAGS+=-fno-pic
# nano.specs = libc pequeña (para sprintf); nosys.specs + src/stub_stdlib.c = sin sistema operativo
CFLAGS+=$(ARCH) --specs=nosys.specs --specs=nano.specs $(INCLUDE)

LDFLAGS +=-T$(LINKERSCRIPT)
LDFLAGS +=-nostdlib
LDFLAGS +=-Wl,--no-relax

LDLIBS = -lc -lgcc

all: $(APPNAME).bin

$(APPNAME).elf: $(OBJS) $(LINKERSCRIPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	arm-none-eabi-size $@

$(APPNAME).bin: $(APPNAME).elf
	$(OBJCOPY) -O binary $< $@

$(BUILD)/%.o: %.S
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $^ -o $@

$(BUILD)/%.o: %.c $(wildcard inc/*.h)
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS)  -c $< -o $@

.PHONY: clean
clean:
	rm -f *.o *.elf *.bin
	rm -rf  $(BUILD)

.PHONY: flash
flash: $(APPNAME).bin
	st-flash --reset write $< 0x08000000

.PHONY: flash-ocd
flash-ocd: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"
