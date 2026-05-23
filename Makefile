CC := gcc
CFLAGS := -Wall -Wextra -Wpedantic -std=c11 -g -fsanitize=address,undefined -Iinc -pthread

BUILD_DIR = build
TEST_DIR = tests
SRC_DIR = src
TARGET = parser
TEST_QUEUE_TARGET = run_queue_tests
TEST_TS_QUEUE_TARGET = run_ts_queue_tests

SRCS := $(wildcard $(SRC_DIR)/*.c)
TEST_QUEUE_SRCS := $(SRC_DIR)/queue.c \
									 $(SRC_DIR)/helpers.c \
									 $(TEST_DIR)/queue_test.c
TEST_TS_QUEUE_SRCS := $(SRC_DIR)/queue.c \
											$(SRC_DIR)/helpers.c \
											$(SRC_DIR)/ts_queue.c \
											$(TEST_DIR)/ts_queue_test.c

OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRCS))
TEST_QUEUE_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_QUEUE_SRCS))
TEST_TS_QUEUE_OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,$(TEST_TS_QUEUE_SRCS))
INCS := $(wildcard inc/*.h)
# TEST_OBJS := $(TEST_QUEUE_SRCS:%.c=$(BUILD_DIR)/%.o)

.PHONY: all clean test_queue test_ts_queue

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(TARGET)

test_queue: $(TEST_QUEUE_TARGET)
	./$(TEST_QUEUE_TARGET)

test_ts_queue: $(TEST_TS_QUEUE_TARGET)
	./$(TEST_TS_QUEUE_TARGET)

$(TEST_QUEUE_TARGET): $(TEST_QUEUE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(TEST_TS_QUEUE_TARGET): $(TEST_TS_QUEUE_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/%.o: %.c $(INCS)
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_QUEUE_TARGET) $(TEST_TS_QUEUE_TARGET)
