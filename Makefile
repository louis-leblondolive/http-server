# --- Config ---
CC       := cc
TARGET   := main
BUILD_DIR := build
OBJ_DIR   := obj

# Sources and objects
SRCS     := $(shell find src -name "*.c")
INC_DIRS := $(shell find src -type d)
INCLUDES := $(addprefix -I, $(INC_DIRS))

# Base flags 
BASE_CFLAGS := -Wall -Wextra -MMD -MP $(INCLUDES)

# --- Compilation Modes ---

# Debug Mode (by default) : ASan + Debug symbols + No optimization
DEBUG_CFLAGS   := $(BASE_CFLAGS) -g -fsanitize=address,undefined -DDEBUG
DEBUG_LDFLAGS  := -fsanitize=address,undefined
DEBUG_OBJ_DIR  := $(OBJ_DIR)/debug
DEBUG_OBJS     := $(SRCS:%.c=$(DEBUG_OBJ_DIR)/%.o)
DEBUG_TARGET   := $(BUILD_DIR)/debug/$(TARGET)

# Release mode : optimize and ignore ASan
RELEASE_CFLAGS  := $(BASE_CFLAGS) -O3 -march=native
RELEASE_LDFLAGS := 
RELEASE_OBJ_DIR := $(OBJ_DIR)/release
RELEASE_OBJS    := $(SRCS:%.c=$(RELEASE_OBJ_DIR)/%.o)
RELEASE_TARGET  := $(BUILD_DIR)/release/$(TARGET)

# --- Rules ---

.PHONY: all debug release clean re

# Build with debug mode by default 
all: debug

# Debug Target
debug: $(DEBUG_TARGET)

$(DEBUG_TARGET): $(DEBUG_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_OBJS) $(DEBUG_LDFLAGS) -o $@

$(DEBUG_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@

# Release Target 
release: $(RELEASE_TARGET)

$(RELEASE_TARGET): $(RELEASE_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_OBJS) $(RELEASE_LDFLAGS) -o $@

$(RELEASE_OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

re: clean all

-include $(DEBUG_OBJS:.o=.d) $(RELEASE_OBJS:.o=.d)