# STMicroelectronics board including a Cortex M4 core, simulated in QEMU
MACHINE = netduinoplus2

# ARM toolchain for baremetal code using eabi Application Binary Interface
TOOLCHAIN = arm-none-eabi

# Compilation tools
AS = $(TOOLCHAIN)-as
CC = $(TOOLCHAIN)-gcc
LD = $(TOOLCHAIN)-ld
SIZE = $(TOOLCHAIN)-size
OBJCOPY = $(TOOLCHAIN)-objcopy
OBJDUMP = $(TOOLCHAIN)-objdump

# -- Assembler flags -----------------------------------------------------------

# ARM Cortex M4 architecture
ASMFLAGS = -mcpu=cortex-m4

# Add debug symbols to the compiled code
ASMFLAGS += -g


# -- C compiler flags ----------------------------------------------------------

# ARM Cortex M4 architecture
CFLAGS = -mcpu=cortex-m4

# Add debug symbols to the compiled code
CFLAGS += -g

# Print all warnings
CFLAGS += -Wall

# Level of optimization 2 (max = 3)
CFLAGS += -O0


# -- C linker flags ------------------------------------------------------------

# Linker script file name
LD_SCRIPT = linker_script.ld

# Specify linker script to use
LD_FLAGS = -T $(LD_SCRIPT)


# Target of the compilation
TARGET = main.elf


# C source files
C_SRC = main.c

# Assembly source files
ASM_SRC = startup.s

# All sources
SRC = $(ASM_SRC) $(C_SRC) 

# Object files: same name of C and assembly sources, but with o extension
OBJS = $(ASM_SRC:%.s=%.o) $(C_SRC:%.c=%.o) 

# Create the target and print its size
all: $(TARGET) size Makefile

# Create the target by linking all the object files
$(TARGET): $(OBJS) $(LD_SCRIPT) Makefile
	
	# Use target name as an output name
	$(LD) $(OBJS) $(LD_FLAGS) -o $@

# Generate all required object files starting from assembly files
%.o: %.s Makefile 
	
	# Compile each dependency and use the name of the corresponding target
	# as output name
	$(AS) $(ASMFLAGS) $< -o $@


# Generate all required object files starting from C files
%.o: %.c Makefile 

	# Compile each dependency and use the name of the corresponding target
	# as output name
	$(CC) $(CFLAGS) -c $< -o $@


# Print size of the executable
.PHONY: size
size: $(TARGET)
	@echo "Generated ELF file"
	@$(SIZE) $<

# Run the executable simulating the target board in QEMU
.PHONY: run
run:
	qemu-system-arm -M $(MACHINE) -kernel $(TARGET) -serial mon:stdio -nographic

# Remove executable and all object files
.PHONY: clean
clean :
	rm -f $(TARGET) $(OBJS)
