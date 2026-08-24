# Configurable builds with DEBUG=1 and clang ASan          
CC := cc                                                     # Compiler (override with 'make CC=clang' etc.)
BUILD := build                                               # Directory for object files
SRC_DIR := src                                               # Directory for source files
CFLAGS := -Wall -Wextra -O2 -MMD -MP                         # Default: optimized compile with dependency generation
LDFLAGS :=                                                   # Default: no special link flags
ifeq ($(DEBUG),1)                                            # Conditional branch enabled when invoking 'make DEBUG=1'
  CFLAGS := -Wall -Wextra -O0 -g -DDEBUG -MMD -MP            # Debug flags: no optimization, symbols, and DEBUG macro
  ifneq (,$(findstring clang,$(CC)))                         # Nested conditional: if CC contains 'clang'
    CFLAGS += -fsanitize=address                             # Add AddressSanitizer to compile flags for runtime memory checks
    LDFLAGS += -fsanitize=address                            # Link with AddressSanitizer runtime
  endif                                                      # End clang-specific block
endif                                                        # End DEBUG block
SRCS := $(wildcard $(SRC_DIR)/*.c)                           # Discover all .c files under src/ dynamically
OBJS := $(SRCS:%.c=$(BUILD)/%.o)                             # Map each source to an object under build/
DEPS := $(OBJS:.o=.d)                                        # Dependency files emitted by the compiler
APP := app                                                   # Name of the resulting executable
all: $(APP)                                                  # Default target builds the application
$(APP): $(OBJS) | $(BUILD)/                                  # Link rule depends on all objects; ensure build/ exists (order-only)
	$(CC) $(LDFLAGS) $^ -o $@                            # Link objects into the executable with any requested link flags
$(BUILD)/%.o: %.c | $(BUILD)/                                # Compile any .c into a corresponding build/%.o; ensure dir exists
	@mkdir -p $(dir $@)                                  # Create the output directory for the object file
	$(CC) $(CFLAGS) -c $< -o $@                          # Compile with (possibly debug/ASan) flags
$(BUILD)/:                                                   # Create build/ directory rule
	mkdir -p $@                                          # Make the build directory
-include $(DEPS)                                             # Include dependency files (ignore if not yet generated)
clean:                                                       # Cleaning rule
	$(RM) -r $(BUILD) $(APP)                             # Remove objects/dep files and the executable
