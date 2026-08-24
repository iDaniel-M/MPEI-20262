# Multi-file build with variables and .PHONY                       

CC := cc                                           # C compiler command; can be overridden on the command line
CFLAGS := -Wall -Wextra -O2                        # Compilation flags: warnings and optimization
SRCS := main.c util.c                              # List of C source files in the project
OBJS := $(SRCS:.c=.o)                              # Maps each .c file to a corresponding .o object file
APP := app                                         # Name of the output executable
.PHONY: all                                        # 'all' is not a real file; always considered out-of-date
all: $(APP)                                        # Default target builds the final executable
$(APP): $(OBJS)                                    # Link step: app depends on all object files
	$(CC) $(OBJS) -o $@                        # Links objects into the executable; $@ expands to 'app'
%.o: %.c                                           # Pattern rule: how to build any .o from the matching .c
	$(CC) $(CFLAGS) -c $< -o $@                # Compiles a single .c to .o using flags; $< is the prerequisite
.PHONY: clean                                      # Phony declaration to ensure 'clean' always runs
clean:                                             # Clean target
	$(RM) $(APP) $(OBJS)                       # Removes the binary and all generated object files
