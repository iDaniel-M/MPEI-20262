# Header dependency generation with -MMD -MP                                  
CC := cc                                           # C compiler
CFLAGS := -Wall -Wextra -O2 -MMD -MP               # Add -MMD (emit .d files) and -MP (phony header targets) for dep tracking
SRCS := main.c util.c                              # Sources used in this program
OBJS := $(SRCS:.c=.o)                              # Derive object file names from sources
DEPS := $(OBJS:.o=.d)                              # Dependency files (.d) corresponding to each object
APP := app                                         # Output program name
all: $(APP)                                        # Default target produces the executable
$(APP): $(OBJS)                                    # Link rule depends on all objects
	$(CC) $^ -o $@                             # Link all prerequisites ($^) into the target ($@)
%.o: %.c                                           # Pattern rule to compile .c to .o
	$(CC) $(CFLAGS) -c $< -o $@                # Compile with flags and generate .d files as a side effect
-include $(DEPS)                                   # Includes .d files if present; '-' prevents errors if they don't exist yet
clean:                                             # Clean target
	$(RM) $(APP) $(OBJS) $(DEPS)               # Remove executable, objects, and dependency files
