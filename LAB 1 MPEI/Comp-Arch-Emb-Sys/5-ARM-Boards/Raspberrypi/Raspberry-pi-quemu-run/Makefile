

ifeq ($(RP4),1)
    CC = aarch64-elf-gcc
	CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding
	OBJCOPY = aarch64-elf-objcopy
	OBJDUMP = aarch64-elf-objdump
	DEFS = -DAARCH64
	ARCH = cortex-a72
	BOOT = boot64
	QEMU = qemu-system-aarch64 -m 2048 -M raspi4b -nographic 
else
    CC = arm-none-eabi-gcc
	CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding
	OBJCOPY = arm-none-eabi-objcopy
	OBJDUMP = arm-none-eabi-objdump
	ARCH = cortex-a7
	BOOT = boot32
	QEMU = qemu-system-arm -m 1024 -M raspi2b -nographic 
	DEFS =
endif

ifeq ($(RP4),1)
	LINKERSCRIPT = linker64.ld
else
	LINKERSCRIPT = linker32.ld
endif


build: myos.elf kernel7.img

kernel7.img: myos.elf
	$(OBJCOPY) -O binary myos.elf kernel7.img

myos.elf: $(BOOT).o kernel.o
	$(CC) -T$(LINKERSCRIPT) -ffreestanding -O2 -nostdlib -lgcc $^ -o $@ 

$(BOOT).o: $(BOOT).S
	$(CC) -mcpu=$(ARCH) -fpic -ffreestanding -c $^ -o $@ 

%.o: %.c
	$(CC) -mcpu=$(ARCH) $(DEFS) -fpic -ffreestanding -std=gnu99 -O2 -Wall -Wextra -c  $< -o $@

qemu: myos.elf
	$(QEMU) -kernel myos.elf 

clean:
	rm -f *.o *.elf *.img