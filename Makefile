# --- Config ---
CC       := gcc
CFLAGS   := -WCL4 -Wextra -g -MMD -MP 
TARGET   := main

# --- Path ---
SRCS     := $(shell find src -name "*.c")
OBJS     := $(SRCS:%.c=obj/%.o)
DEPS     := $(OBJS:.o=.d)

# --- Includes ---
INC_DIRS := $(shell find src -type d)
INCLUDES := $(addprefix -I, $(INC_DIRS))

# --- Rules ---

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@


obj/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf obj/ $(TARGET)

re: clean all

-include $(DEPS)

.PHONY: all clean re