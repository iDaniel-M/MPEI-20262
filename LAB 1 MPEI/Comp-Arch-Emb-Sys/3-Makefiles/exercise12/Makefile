# Parallel-friendly build with order-only prerequisites + format target      
CC := cc                                                                                   # Compiler
CFLAGS := -Wall -Wextra -O2 -MMD -MP                                                       # Compile flags with dependency generation for correct rebuilds
SRC_DIR := src                                                                             # Sources are under src/
BUILD := build                                                                             # Directory for object files
BIN := bin                                                                                 # Directory for the final executable
APP := $(BIN)/app                                                                          # Full path to the binary in bin/
SRCS := $(wildcard $(SRC_DIR)/*.c)                                                         # Discover all .c files in src/
OBJS := $(SRCS:%.c=$(BUILD)/%.o)                                                           # Map sources to objects in build/
DEPS := $(OBJS:.o=.d)                                                                      # Dependency files associated with the objects
all: $(APP)                                                                                # Default target builds the app
$(APP): $(OBJS) | $(BIN)/                                                                  # Link rule: ensure bin/ exists via order-only prerequisite
	$(CC) $^ -o $@                                                                     # Link objects into the final executable
$(BUILD)/%.o: %.c | $(BUILD)/                                                              # Compile rule: ensure build/ exists; safe for 'make -j'
	@mkdir -p $(dir $@)                                                                # Create any needed subdirectories within build/
	$(CC) $(CFLAGS) -c $< -o $@                                                        # Compile a single source into its object with dep gen
$(BUILD)/:                                                                                 # Create build/ directory
	mkdir -p $@                                                                        # Make the directory if missing
$(BIN)/:                                                                                   # Create bin/ directory
	mkdir -p $@                                                                        # Make the output directory for the binary
format:                                                                                    # Formatting helper target (phony by behavior; could be marked .PHONY)
	@echo "Formatting sources with clang-format (if installed)..."                     # Message shown before running the formatter
	clang-format -i $(SRCS)                                                            # Run clang-format in-place on all discovered sources
-include $(DEPS)                                                                           # Include dependency files for accurate incremental builds
clean:                                                                                     # Clean target
	$(RM) -r $(BUILD) $(BIN)                                                           # Remove build and bin directories recursively
