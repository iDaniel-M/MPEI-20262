CC = arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -Og -g -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard -Iinc -nostdlib
LDFLAGS = -nostdlib -Tlinker_script.ld -Wl,--no-relax,-Map=triqui.map

OBJS = src/main.o src/gpio_config.o src/game.o crt0.o

all: triqui.elf triqui.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.S
	$(CC) $(CFLAGS) -c $< -o $@

triqui.elf: $(OBJS) linker_script.ld
	$(CC) $(LDFLAGS) $(OBJS) -o triqui.elf

triqui.bin: triqui.elf
	arm-none-eabi-objcopy -O binary triqui.elf triqui.bin

flash: triqui.bin
	/mnt/d/Uni/MPEI/STM32/bin/STM32_Programmer_CLI.exe -c port=usb1 -w triqui.bin 0x08000000 -v

clean:
	rm -f src/*.o crt0.o triqui.elf triqui.bin triqui.map