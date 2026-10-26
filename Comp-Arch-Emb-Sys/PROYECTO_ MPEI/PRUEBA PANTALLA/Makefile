# Makefile - Serpiente en panel P10 RGB (STM32F446RE, bare metal, GNU toolchain)
#
#   make          compila main.elf y main.bin
#   make flash    graba la placa con st-flash
#   make clean    borra lo compilado

APPNAME = main

CC      = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE    = arm-none-eabi-size

ARCH         = -mcpu=cortex-m4 -mthumb
LINKERSCRIPT = linker_script.ld
INCLUDE      = -I./inc

# -ffreestanding : no hay sistema operativo ni libc
# -fno-tree-loop-distribute-patterns : que GCC no cambie los "for" por llamadas a memset
CFLAGS  = $(ARCH) -std=c11 -Wall -Wextra -Og -g $(INCLUDE)
CFLAGS += -ffreestanding -fno-tree-loop-distribute-patterns

# -nostdlib : no usar libc ni el arranque de GCC (el arranque es crt0.S)
LDFLAGS = -T$(LINKERSCRIPT) -nostdlib
LDLIBS  = -lgcc

C_SRC   = $(wildcard *.c) $(wildcard ./src/*.c)
ASM_SRC = $(wildcard *.S)
OBJS    = $(patsubst %.c, %.o, $(C_SRC)) $(patsubst %.S, %.o, $(ASM_SRC))

all: $(APPNAME).bin

$(APPNAME).elf: $(OBJS) $(LINKERSCRIPT)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@
	$(SIZE) $@

$(APPNAME).bin: $(APPNAME).elf
	$(OBJCOPY) -O binary $< $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c $(wildcard inc/*.h)
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: flash
flash: $(APPNAME).bin
	st-flash --reset write $< 0x08000000

.PHONY: clean
clean:
	rm -f *.o src/*.o *.elf *.bin
