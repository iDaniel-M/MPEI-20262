APPNAME = main
MACHINE = netduinoplus2


CC =  arm-none-eabi-gcc
CFLAGS = -Wall -Wextra -std=c11 -ffreestanding
OBJCOPY =  arm-none-eabi-objcopy
OBJDUMP =  arm-none-eabi-objdump
LINK = arm-none-eabi-ld
ARCH =  -mcpu=cortex-m4 
LINKERSCRIPT = linker_script.ld
BUILD = build

#CMSIS directory
CMSIS = ../STM32utils/Drivers/CMSIS
#STM32 register definitions
STM32F4 =$(CMSIS)/Device/ST/STM32F4xx
STM32F4_HAL =../STM32utils/Drivers/STM32F4xx_HAL_Driver

# Add include directories to search for *.h files
INCLUDE = -I./inc
INCLUDE += -I.
INCLUDE += -I$(STM32F4)/Include
INCLUDE += -I$(CMSIS)/Core/Include
INCLUDE += -I$(STM32F4_HAL)/Inc


ifeq ($(DBG),1)	
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
else
	QEMU = qemu-system-arm -M $(MACHINE) -kernel $(APPNAME).elf -serial mon:stdio -nographic --no-reboot -S -s
endif

C_SRC = $(wildcard *.c)
C_SRC += $(wildcard src/*.c)
C_SRC += $(wildcard $(STM32F4_HAL)/Src/*.c)
# add some smt32f4 helper functions
C_SRC += $(STM32F4)/Source/Templates/system_stm32f4xx.c
C_SRC := $(filter-out %stm32f4xx_hal_timebase_tim_template.c, $(C_SRC))
C_SRC := $(filter-out %stm32f4xx_hal_timebase_rtc_alarm_template.c, $(C_SRC))
C_SRC := $(filter-out %stm32f4xx_hal_timebase_rtc_wakeup_template.c, $(C_SRC))

ASM_SRC = $(wildcard *.S)
# Remove the path dirs and keep only the filenames
OBJS = $(patsubst %.c, $(BUILD)/%.o, $(notdir $(C_SRC)))
OBJS += $(patsubst %.S, $(BUILD)/%.o, $(notdir  $(ASM_SRC)))

# we add the paths where to find the *.c and *.S files
vpath %.c $(sort $(dir $(C_SRC))) 
vpath %.S $(sort $(dir $(ASM_SRC))) 

CFLAGS+=-Og -g 
CFLAGS+=-fno-pic
CFLAGS+=$(ARCH) --specs=nosys.specs --specs=nano.specs $(INCLUDE) -DSTM32F401xE

LDFLAGS +=-T$(LINKERSCRIPT) 
LDFLAGS +=-nostdlib -DSTM32F410Rx
LDFLAGS +=-Wl,--no-relax 

LDLIBS = -lc -lgcc 

all: $(APPNAME).elf 

$(APPNAME).elf: $(OBJS) 
	@echo $(OBJS) $(SRC)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) -o $@ 
	arm-none-eabi-size -A -x $@

$(BUILD)/%.o: %.S 
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c $^ -o $@ 
	
$(BUILD)/%.o: %.c 
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS)  -c $< -o $@ 

.PHONY: run
run: $(APPNAME).elf
	$(QEMU) -bios $(APPNAME).elf 

$(BUILD): 
	mkdir -p $@

.PHONY: debug
debug: $(APPNAME).elf
	gdb-multiarch $(APPNAME).elf -ex "target remote :1234" -ex "tb Reset_Handler" 

.PHONY: clean
clean:	
	rm -f *.o *.elf *.img src/*.o 
	rm -rf  $(BUILD)


.PHONY: flash
flash: $(APPNAME).elf
	openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program $(APPNAME).elf verify reset exit"