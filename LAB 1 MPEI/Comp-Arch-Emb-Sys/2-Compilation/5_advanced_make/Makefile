# Compiler
CC = gcc

# Target executable
TARGET = main.elf

# Source files
SRC = main.c function.c

# Object files: converts .c files to .o files
OBJ = $(SRC:.c=.o) 

INCLUDE_PATH = ./Include

# Compiler flags
CFLAGS = -I $(INCLUDE_PATH)

# Compile and print executable and object sizes
all: $(TARGET) cmp_size

# Build the target ELF file by linking object files
$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@
	
	@# Execute the command without printing the command itself with @
	@echo "Linking completed\n"

# Compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
	
	@# Execute the command without printing the command itself with @
	@echo "Compilation of all sources completed\n"

# Compare sizes of the different sections of the object alone and of the ELF
cmp_size:
	size $(TARGET:.elf=.o)
	size $(TARGET)

# Phony target: execute always, even if a target with the same name exists
.PHONY: run
run: $(TARGET)
	@./$(TARGET)

# Good approach to declare clean target phony as well
.PHONY: clean
clean:
	rm -f $(OBJ) $(TARGET)
