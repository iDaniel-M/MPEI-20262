# Easy to reconfigure the compiler
CC=gcc

# Easy to change the $(MAIN) file name
MAIN=main

# Compile and print executable and object sizes
all: $(MAIN).elf cmp_size

# Full compilation. This is the default target
$(MAIN).elf: $(MAIN).o function.o Makefile
	
	# Link the object files: specify output name with -o
	$(CC) -o $(MAIN).elf $(MAIN).o function.o

# Call the assembler to create $(MAIN) object file
$(MAIN).o: $(MAIN).c function.h Makefile
	
	# Stop copilation at assembler level with -c. Specify output name with -o
	$(CC) -c -o $(MAIN).o $(MAIN).c

# Call the assembler to create function object file
function.o: function.c function.h Makefile
	
	# Stop copilation at assembler level with -c. Specify output name with -o
	$(CC) -c -o function.o function.c
	
# Compare sizes of the different sections of the object alone and of the ELF
cmp_size:
	size $(MAIN).o
	size $(MAIN).elf

# Execute the code
run: ./$(MAIN).elf
	./$(MAIN).elf

# Remove generated files
clean:
	rm -f *.o
	rm -f *.i
	rm -f *.s
	rm -f *.elf
