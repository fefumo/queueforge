CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -fsanitize=address,undefined -Iinc

BUILD_DIR = build
TEST_DIR = tests
SRC_DIR = src
TARGET = parser
TEST_TARGET = run_tests

SRCS := $(wildcard $(SRC_DIR)/*.c)
TEST_SRCS := $(SRC_DIR)/queue.c $(TEST_DIR)/queue_test.c $(SRC_DIR)/helpers.c
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_SRCS))
INCS := $(wildcard inc/*.h)
# TEST_OBJS := $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c $(INCS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_TARGET)
