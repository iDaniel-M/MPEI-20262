# -------------------------------------------------------
# Compiler configuration
# -------------------------------------------------------

# Define the compiler
CC = gcc

# Compiler flags
# -Wall : enable all common warnings
CFLAGS = -Wall

# -------------------------------------------------------
# Project files and target
# -------------------------------------------------------

# The name of the final executable
TARGET = math_program.elf

# Source files
SRCS = main.c math_utils.c

# Object files (replace .c with .o)
OBJS = $(SRCS:.c=.o)

# -------------------------------------------------------
# Default rule
# -------------------------------------------------------

# 'all' is the default target — it builds the executable
all: $(TARGET)

# -------------------------------------------------------
# Linking stage
# -------------------------------------------------------

# Link all object files into the final executable
# $@ = target name (math_program.elf)
# $^ = all dependencies (object files)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# -------------------------------------------------------
# Compilation stage
# -------------------------------------------------------

# Rule to compile main.c into main.o
main.o: main.c math_utils.h
	$(CC) $(CFLAGS) -c main.c -o main.o

# Rule to compile math_utils.c into math_utils.o
# It depends on math_utils.h so if the header changes,
# math_utils.o will be rebuilt.
math_utils.o: math_utils.c math_utils.h
	$(CC) $(CFLAGS) -c math_utils.c -o math_utils.o

# -------------------------------------------------------
# Cleanup rule
# -------------------------------------------------------

# Remove all object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)

# -------------------------------------------------------
# Special declarations
# -------------------------------------------------------

# Mark targets that do not correspond to actual files
.PHONY: all clean
