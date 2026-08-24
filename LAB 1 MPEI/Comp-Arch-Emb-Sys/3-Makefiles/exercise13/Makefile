# Auto-discovered unit tests + static lib + 'test' runner                   
CC := cc                                                                                   # C compiler
AR := ar                                                                                   # Static library archiver
ARFLAGS := rcs                                                                             # Archive flags: replace/insert, create, add index
CFLAGS := -Wall -Wextra -O2 -MMD -MP                                                       # Compile flags with dependency generation
SRC_DIR := src                                                                             # Directory containing library sources
TEST_DIR := tests                                                                          # Directory containing test sources
BUILD := build                                                                             # Root build directory
BUILD_LIB := $(BUILD)/lib                                                                  # Subdirectory for library object files and artifacts
BUILD_TST := $(BUILD)/tests                                                                # Subdirectory for compiled test binaries
LIB_SRCS := $(wildcard $(SRC_DIR)/*.c)                                                     # All .c files in src/ are part of the library
LIB_OBJS := $(LIB_SRCS:%.c=$(BUILD_LIB)/%.o)                                               # Object files for the library placed under build/lib/
LIB := $(BUILD_LIB)/libapp.a                                                               # Path to the generated static library
TEST_SRCS := $(wildcard $(TEST_DIR)/test_*.c)                                              # Discover all test source files prefixed with 'test_'
TEST_BINS := $(TEST_SRCS:$(TEST_DIR)/%.c=$(BUILD_TST)/%)                                   # Map each test source to an output binary path under build/tests/
all: $(LIB)                                                                                # Default target builds the library (tests can be built/run separately)
$(LIB): $(LIB_OBJS) | $(BUILD_LIB)/                                                        # Library depends on its objects; ensure build/lib exists (order-only)
	$(AR) $(ARFLAGS) $@ $^                                                                 # Create/update the static library archive from object files
$(BUILD_LIB)/%.o: %.c | $(BUILD_LIB)/                                                      # Compile any library source into build/lib/%.o
	@mkdir -p $(dir $@)                                                                    # Ensure the directory structure exists for the object
	$(CC) $(CFLAGS) -c $< -o $@                                                            # Compile library source with flags; generates .d alongside
$(BUILD_TST)/%: $(TEST_DIR)/%.c $(LIB) | $(BUILD_TST)/                                    # Rule to build each test binary, linking against libapp.a
	@mkdir -p $(dir $@)                                                                    # Ensure tests output directory exists
	$(CC) $(CFLAGS) $< $(LIB) -o $@                                                        # Compile and link the test with the library into an executable
test: $(TEST_BINS)                                                                         # 'make test' builds all tests then executes them
	@set -e; for t in $(TEST_BINS); do echo "▶ Running $$t"; "$$t"; echo "✓ Passed $$t"; done  # Run each test; stop on error and print progress
$(BUILD)/:                                                                                 # Create top-level build directory
	mkdir -p $@                                                                            # Make the build directory
$(BUILD_LIB)/: | $(BUILD)/                                                                 # Create build/lib/ after build/ exists
	mkdir -p $@                                                                            # Make the library subdirectory
$(BUILD_TST)/: | $(BUILD)/                                                                 # Create build/tests/ after build/ exists
	mkdir -p $@                                                                            # Make the tests subdirectory
clean:                                                                                     # Clean target
	$(RM) -r $(BUILD)                                                                      # Remove the entire build tree (objects, deps, tests, lib)
