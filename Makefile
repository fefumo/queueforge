CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -fsanitize=address,undefined -Iinc
LDFLAGS := -fsanitize=address,undefined -pthread

BUILD_DIR := build
TEST_DIR := tests
SRC_DIR := src

TARGET := parser
TEST_QUEUE_TARGET := run_queue_tests
TEST_TS_QUEUE_TARGET := run_ts_queue_tests

APP_SRCS := src/main.c src/parser.c src/helpers.c
QUEUE_TEST_SRCS := src/queue.c src/helpers.c tests/queue_test.c
TS_QUEUE_TEST_SRCS := src/queue.c src/ts_queue.c src/helpers.c tests/ts_queue_test.c

APP_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(APP_SRCS))
QUEUE_TEST_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(QUEUE_TEST_SRCS))
TS_QUEUE_TEST_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TS_QUEUE_TEST_SRCS))

INCS := $(wildcard inc/*.h)

.PHONY: all clean test test_queue test_ts_queue

all: $(TARGET)

test: test_queue test_ts_queue

$(TARGET): $(APP_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_queue: $(TEST_QUEUE_TARGET)
	./$(TEST_QUEUE_TARGET)

test_ts_queue: $(TEST_TS_QUEUE_TARGET)
	./$(TEST_TS_QUEUE_TARGET)

$(TEST_QUEUE_TARGET): $(QUEUE_TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(TEST_TS_QUEUE_TARGET): $(TS_QUEUE_TEST_OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c $(INCS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_QUEUE_TARGET) $(TEST_TS_QUEUE_TARGET)
