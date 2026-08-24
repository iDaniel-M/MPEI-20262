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

# Compile .c files into .o object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Compare sizes of the different sections of the object alone and of the ELF
cmp_size:
	size $(TARGET:.elf=.o)
	size $(TARGET)

# Execute the code
run: $(TARGET)
	./$(TARGET)

# Clean rule
clean:
	rm -f $(OBJ) $(TARGET)
