# Multiple executables (server/client) with target-specific flags           
CC := cc                                                     # C compiler
CFLAGS := -Wall -Wextra -MMD -MP                             # Common compile flags with dependency generation
COMMON_SRCS := net.c util.c                                  # Sources shared by both targets
SERVER_SRCS := $(COMMON_SRCS) server_main.c                  # Sources for server executable
CLIENT_SRCS := $(COMMON_SRCS) client_main.c                  # Sources for client executable
BUILD := build                                               # Directory for object files
BIN := bin                                                   # Directory where final binaries are placed
PROGS := $(BIN)/server $(BIN)/client                         # List of executables to produce
all: $(PROGS)                                                # Default target builds both programs
$(BIN)/server: CFLAGS += -O2 -DSERVER                        # Add flags only when building 'server': optimize and define SERVER
$(BIN)/client: CFLAGS += -Os -DCLIENT                        # Add flags only when building 'client': size-optimize and define CLIENT
SERVER_OBJS := $(SERVER_SRCS:%.c=$(BUILD)/%.o)               # Object list for server sources under build/
CLIENT_OBJS := $(CLIENT_SRCS:%.c=$(BUILD)/%.o)               # Object list for client sources under build/
DEPS := $(SERVER_OBJS:.o=.d) $(CLIENT_OBJS:.o=.d)            # Dependency files for both sets of objects
$(BIN)/server: $(SERVER_OBJS) | $(BIN)/                      # Server link rule depends on its objects; ensure bin/ exists (order-only)
	$(CC) $(CFLAGS) $^ -o $@                             # Link server with current CFLAGS (including target-specific ones)
$(BIN)/client: $(CLIENT_OBJS) | $(BIN)/                      # Client link rule with order-only dir prerequisite
	$(CC) $(CFLAGS) $^ -o $@                             # Link client with its objects
$(BUILD)/%.o: %.c | $(BUILD)/                                # Generic rule to compile .c into build/%.o; ensure build/ exists
	@mkdir -p $(dir $@)                                  # Create any nested directories in build/ for the object path
	$(CC) $(CFLAGS) -c $< -o $@                          # Compile source using flags and generate .d file
$(BUILD)/:                                                   # Rule to create build/ directory
	mkdir -p $@                                          # Create build/ if absent
$(BIN)/:                                                     # Rule to create bin/ directory
	mkdir -p $@                                          # Create bin/ if absent
-include $(DEPS)                                             # Include dependency files for correct incremental rebuilds
clean:                                                       # Clean rule
	$(RM) -r $(BUILD) $(BIN)                             # Remove all generated directories and files
