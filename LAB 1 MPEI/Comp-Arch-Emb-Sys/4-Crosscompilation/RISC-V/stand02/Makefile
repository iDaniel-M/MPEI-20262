


CC =  riscv32-unknown-elf-gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding
OBJCOPY =  riscv32-unknown-elf-objcopy
OBJDUMP =  riscv32-unknown-elf-objdump
LINK = riscv32-unknown-elf-ld
ARCH =  -march=rv32imc -mabi=ilp32
LINKERSCRIPT = riscv-qemu.ld

ifeq ($(DBG),1)	
	QEMU = qemu-system-riscv32 -machine virt -nographic --no-reboot -S -s
else
	QEMU = qemu-system-riscv32 -machine virt -nographic --no-reboot -S -s
endif

CFLAGS+=-Og -g
CFLAGS+=-fno-pic
CFLAGS+=$(ARCH)

LDFLAGS +=-T$(LINKERSCRIPT) 
LDFLAGS +=-nostdlib 
LDFLAGS +=-Wl,--no-relax 


build: myos.elf 

myos.elf: crt0.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS) -o $@ 
	riscv32-unknown-elf-size -A -x $@

crt0.o: crt0.S
	$(CC) $(CFLAGS) -c $^ -o $@ 
	
%.o: %.c 
	$(CC) $(CFLAGS)  -c $< -o $@ 

qemu: myos.elf
	$(QEMU) -bios myos.elf 

gdb: myos.elf
	riscv32-unknown-elf-gdb myos.elf -ex "target remote :1234" -ex "tb _start" -ex "c"

clean:
	rm -f *.o *.elf *.img