APPNAME = stand00
MACHINE = netduinoplus2


CC =  arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
OBJCOPY =  arm-none-eabi-objcopy
OBJDUMP =  arm-none-eabi-objdump
LINK = arm-none-eabi-ld
ARCH =  -mcpu=cortex-m4 
LINKERSCRIPT = linker_script.ld

INCLUDE = -I./inc -I.

ifeq ($(DBG),1)	
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
else
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
endif

C_SRC = $(wildcard *.c)
C_SRC += $(wildcard ./src/*.c)
ASM_SRC = $(wildcard *.S)
# Convert the list of .c files into a list of .o object files
OBJS = $(patsubst %.c, %.o, $(C_SRC))
OBJS += $(patsubst %.S, %.o, $(ASM_SRC))

CFLAGS+=-Og -g 
CFLAGS+=-fno-pic
CFLAGS+=$(ARCH) --specs=nosys.specs --specs=nano.specs $(INCLUDE)

LDFLAGS +=-T$(LINKERSCRIPT) 
LDFLAGS +=-nostdlib 
LDFLAGS +=-Wl,--no-relax 

LDLIBS = -lc -lgcc 

build: $(APPNAME).elf 

$(APPNAME).elf: $(OBJS)
	@echo $(OBJS) $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@ 
	arm-none-eabi-size -A -x $@

%.o: %.S
	$(CC) $(CFLAGS) -c $^ -o $@ 
	
%.o: %.c 
	$(CC) $(CFLAGS)  -c $< -o $@ 

.PHONY: run
run: $(APPNAME).elf
	$(QEMU) -bios $(APPNAME).elf 

.PHONY: debug
debug: $(APPNAME).elf
	gdb-multiarch $(APPNAME).elf -ex "target remote :1234" -ex "tb Reset_Handler" 

.PHONY: clean
clean:	
	rm -f *.o *.elf *.img src/*.o


.PHONY: flash
flash: $(APPNAME).elf
	openocd -f board/ek-tm4c123gxl.cfg -c "program $(APPNAME).elf verify reset exit"