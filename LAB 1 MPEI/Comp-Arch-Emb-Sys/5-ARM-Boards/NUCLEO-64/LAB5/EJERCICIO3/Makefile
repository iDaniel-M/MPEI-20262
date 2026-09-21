# Makefile
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m4 -mthumb -O2 -Wall -g
LDFLAGS = -T linker.ld -nostdlib -Wl,-Map=main.map

all: main.bin

main.elf: main.c
	$(CC) $(CFLAGS) main.c $(LDFLAGS) -o main.elf

main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

flash: main.bin
	st-flash write main.bin 0x08000000

clean:
	rm -f *.elf *.bin *.map *.o