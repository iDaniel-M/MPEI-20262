# -------------------------------------------------------
# Compiler configuration
# -------------------------------------------------------

# Define the C compiler to use
CC = gcc

# Define compiler flags
# -Wall enables all common warning messages
CFLAGS = -Wall

# -------------------------------------------------------
# Project files and target
# -------------------------------------------------------

# The final executable file name
TARGET = program.elf

# Source files in the project
SRCS = main.c utils.c

# Object files generated from the source files
# This replaces every ".c" with ".o"
OBJS = $(SRCS:.c=.o)

# -------------------------------------------------------
# Default rule
# -------------------------------------------------------

# 'all' is the default target when you just run 'make'
# It depends on the final executable, so 'make' will build that
all: $(TARGET)

# -------------------------------------------------------
# Linking stage
# -------------------------------------------------------

# This rule tells make how to build the final executable
# from the object files.
# $@ expands to the target name (program.elf)
# $^ expands to all dependencies (object files)
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# -------------------------------------------------------
# Compilation stage
# -------------------------------------------------------

# Generic rule for building any .o file from its .c source
# $< expands to the first dependency (the .c file)
# -c means "compile only" (don’t link)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# -------------------------------------------------------
# Cleanup rule
# -------------------------------------------------------

# 'make clean' removes all generated files so you can rebuild from scratch
clean:
	rm -f $(OBJS) $(TARGET)

# -------------------------------------------------------
# Special declarations
# -------------------------------------------------------

# These targets don't represent actual files
# This prevents conflicts if files named 'all' or 'clean' exist
.PHONY: all clean