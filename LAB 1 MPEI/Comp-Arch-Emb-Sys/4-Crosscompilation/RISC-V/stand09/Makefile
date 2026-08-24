


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

C_SRC = $(wildcard *.c)
ASM_SRC = $(wildcard *.S)
# Convert the list of .c files into a list of .o object files
OBJS = $(patsubst %.c, %.o, $(C_SRC))
OBJS += $(patsubst %.S, %.o, $(ASM_SRC))

CFLAGS+=-Og -g
CFLAGS+=-fno-pic
CFLAGS+=$(ARCH)

LDFLAGS +=-T$(LINKERSCRIPT) 
LDFLAGS +=-nostdlib 
LDFLAGS +=-Wl,--no-relax 

LDLIBS = -lc -lgcc

build: myos.elf 

myos.elf: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@ 
	riscv32-unknown-elf-size -A -x $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@ 
	
%.o: %.c 
	$(CC) $(CFLAGS)  -c $< -o $@ 

qemu: myos.elf
	$(QEMU) -bios myos.elf 

gdb: myos.elf
	riscv32-unknown-elf-gdb myos.elf -ex "target remote :1234" -ex "tb _start" -ex "c"

clean:
	rm -f *.o *.elf *.img