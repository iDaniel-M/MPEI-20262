# Out-of-tree objects in build/ with dep files                               
CC := cc
# Compiler
CFLAGS := -Wall -Wextra -O2 -MMD -MP
# Compile flags with dependency generation
SRCS := main.c util.c math.c
# Source files live in the project root
BUILD := build
# Directory where all intermediates (.o/.d) will be placed
OBJS := $(SRCS:%.c=$(BUILD)/%.o)
# Remap each source to an object file under build/
DEPS := $(OBJS:.o=.d)
# Corresponding dependency files under build/
APP := app
# Final program name
all: $(APP)
# Default target builds the app
$(APP): $(OBJS)                                    # Link step uses objects from build/
	$(CC) $^ -o $@                             # Link objects into the 'app' binary
$(BUILD)/%.o: %.c | $(BUILD)/                      # Compile rule: produce build/%.o from %.c; order-only ensures dir exists
	$(CC) $(CFLAGS) -c $< -o $@                # Compile source into object inside build/ with deps
$(BUILD)/:                                         # Rule to create the build directory
	mkdir -p $@                                # Create directory (no error if it already exists)
-include $(DEPS)                                   # Include generated .d files, ignoring if missing
clean:                                             # Clean rule
	$(RM) -r $(BUILD) $(APP)                   # Remove build directory recursively and the app
