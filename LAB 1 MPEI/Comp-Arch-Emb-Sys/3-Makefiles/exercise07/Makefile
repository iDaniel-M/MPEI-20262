# Pattern rules & automatic variables ($@, $^, $<)                           
CC := cc                                           # Compiler
CFLAGS := -Wall -Wextra -O2                        # Common compilation flags
SRCS := main.c util.c math.c                       # Three source files compose the program
OBJS := $(SRCS:.c=.o)                              # Object files derived from sources
APP := app                                         # Executable name
$(APP): $(OBJS)                                    # Link step depends on all object files
	$(CC) $^ -o $@                             # $^ expands to all prerequisites (objects); $@ is the target (app)
%.o: %.c                                           # Generic rule: build .o from .c
	$(CC) $(CFLAGS) -c $< -o $@                # $< is the first prerequisite (the matched .c file)
clean:                                             # Target to remove build outputs
	$(RM) $(APP) $(OBJS)                       # Delete the executable and all .o files
