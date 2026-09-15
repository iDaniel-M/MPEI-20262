CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

all: main.bin

main.elf: main.c
	$(CC) -mcpu=cortex-m4 -mthumb -O0 -T linker.ld -nostdlib -nostartfiles main.c -o main.elf

main.bin: main.elf
	$(OBJCOPY) -O binary main.elf main.bin

flash: main.bin
	st-flash --reset write main.bin 0x8000000

clean:
	rm -f main.elf main.bin
