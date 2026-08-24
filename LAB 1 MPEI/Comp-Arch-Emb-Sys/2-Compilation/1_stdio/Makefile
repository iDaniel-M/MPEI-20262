# Simple makefile to compile the example code. The native gcc will be used. This
# means that the generated code will be compatible with the processor of your
# machine (probably X86_64)

# By default the first target, and all the targets needed to create its
# dependencies is executed if you call make.
#
# If you want to execute a particular target you can call 
# 					make <target-name>

all: main.elf cmp_size

# Full compilation. This is the default target
main.elf: main.o function.o Makefile
	
	# Link the object files: specify output name with -o
	gcc -o main.elf main.o function.o

# Call the assembler to create main object file
main.o: main.c function.h Makefile
	
	# Stop copilation at assembler level with -c. Specify output name with -o
	gcc -c -o main.o main.c


# Call the assembler to create function object file
function.o: function.c function.h Makefile
	
	# Stop copilation at assembler level with -c. Specify output name with -o
	gcc -c -o function.o function.c

# Call the assembler and generate assembly code in text format
assemble: main.c Makefile
	
	# Stop compilation at assembler with -S: Output text instead of object file
	gcc -S -o main.s main.c

# Call the preprocessor only. Use .i for intermediate format. In practice the
# output is still C code
preprocess: main.c Makefile
	gcc -E -o main.i main.c

# Show assembly code of object file. You need to crearte the object first
disassemble:
	objdump -d main.o

sections:
	objdump -S main.o

# Print the symbols table
symbols:
	objdump -t main.o

# Show all the names included in the object
names:
	nm main.o

# Show ELF header
header:
	readelf -h main.elf
	
# Compare sizes of the different sections of the object alone and of the ELF
cmp_size:
	size main.o
	size main.elf

# Show the architecture on which you are running
show_arch:
	arch

# Execute the code
run: main.elf
	./main.elf

# Remove generated files
clean:
	rm -f *.o
	rm -f *.i
	rm -f *.s
	rm -f *.elf
