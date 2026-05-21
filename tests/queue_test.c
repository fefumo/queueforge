#include "parser.h"
#include "queue.h"
#include <assert.h>
#include <stdlib.h>

#define MESSAGE_INIT(arr, type) message_init(arr, sizeof(arr), type)

int test_init(void) {
  Queue q = {.capacity = 1};
  assert(queue_init(&q, q.capacity) == QUEUE_OK);
  free(q.data);
  return 0;
}

int test_destroy(void) {
  Queue q = {0};

  assert(queue_init(&q, 1) == QUEUE_OK);
  assert(q.data != NULL);
  assert(q.capacity == 1);
  assert(q.head == 0);
  assert(q.tail == 0);
  assert(q.size == 0);

  queue_destroy(&q);

  assert(q.data == NULL);
  assert(q.capacity == 0);
  assert(q.head == 0);
  assert(q.tail == 0);
  assert(q.size == 0);

  return 0;
}

// TODO: add head and tail asserts
int test_push_and_pop(void) {
  Queue q = {0, .capacity = 1};
  queue_init(&q, q.capacity);
  Message some_message = {.channel_id = 0x2a};
  queue_push(&q, &some_message);
  assert(q.data->channel_id == 0x2a);
  queue_pop(&q, q.data);
  queue_destroy(&q);
  assert(q.data == NULL);
  return 0;
}

int test_pop_from_empty(void) {
  Queue q = {.capacity = 1, .size = 0};
  queue_init(&q, q.capacity);
  Message msg = {0};
  QueueStatus ret = queue_pop(&q, &msg);
  assert(ret == QUEUE_ERR_EMPTY);
  queue_destroy(&q);
  return 0;
}

int test_push_into_full(void) {
  Queue q = {.capacity = 1, .size = 0};
  queue_init(&q, q.capacity);
  Message msg = {.type = 1, .payload = {1, 2, 3}, .payload_len = 3};
  printf("msg1 payload: ");
  print_payload(&msg);
  Message msg2 = {.type = 2, .payload = {4, 5, 6}, .payload_len = 3};
  printf("msg2 payload: ");
  print_payload(&msg2);
  assert(queue_push(&q, &msg) == QUEUE_OK);
  assert(queue_push(&q, &msg2) == QUEUE_ERR_FULL);
  assert(queue_pop(&q, &msg2) == QUEUE_OK);
  printf("msg2 payload after popping: ");
  print_payload(&msg2);
  queue_destroy(&q);
  return 0;
}

int test_fifo_order(void) {
  uint8_t pp[] = {1, 1, 1};
  Message msg1 = MESSAGE_INIT(pp, 1);
  assert(memcmp(msg1.payload, (uint8_t[]){1, 1, 1}, 3) == 0);

  uint8_t pp2[] = {2, 2, 2};
  Message msg2 = MESSAGE_INIT(pp2, 2);
  assert(memcmp(msg2.payload, (uint8_t[]){2, 2, 2}, 3) == 0);

  uint8_t pp3[] = {3, 3, 3};
  Message msg3 = MESSAGE_INIT(pp3, 3);
  assert(memcmp(msg3.payload, (uint8_t[]){3, 3, 3}, 3) == 0);

  Queue q = {.capacity = 3};
  Message tmp = {0};
  queue_init(&q, q.capacity);

  assert(queue_push(&q, &msg1) == QUEUE_OK && q.size == 1 && q.tail == 1 &&
         q.head == 0);
  assert(queue_push(&q, &msg2) == QUEUE_OK && q.size == 2 && q.tail == 2 &&
         q.head == 0);
  assert(queue_push(&q, &msg3) == QUEUE_OK && q.size == 3 && q.tail == 0 &&
         q.head == 0);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, (uint8_t[]){1, 1, 1}, 3) == 0);
  assert(q.tail == 0 && q.head == 1 && tmp.type == 1);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, (uint8_t[]){2, 2, 2}, 3) == 0);
  assert(q.tail == 0 && q.head == 2 && tmp.type == 2);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, (uint8_t[]){3, 3, 3}, 3) == 0);
  assert(q.tail == 0 && q.head == 0 && tmp.type == 3);

  queue_destroy(&q);
  return 0;
}

int test_wrap_around(void) { return 0; }
int test_capacity1(void) { return 0; }
int test_invalid_args(void) { return 0; }

void test_all(void) {
  assert(test_init() == 0);
  printf("==== test init PASSED ====\n");
  assert(test_destroy() == 0);
  printf("==== test destroy PASSED ====\n");
  assert(test_push_and_pop() == 0);
  printf("==== test push_and_pop PASSED ====\n");
  assert(test_pop_from_empty() == 0);
  printf("==== test pop_from_empty PASSED ====\n");
  assert(test_push_into_full() == 0);
  printf("==== test test_push_into_full PASSED ====\n");
  assert(test_push_and_pop() == 0);
  printf("==== test test_push_and_pop PASSED ====\n");
  assert(test_fifo_order() == 0);
  printf("==== test test_fifo_order PASSED ====\n");
  printf("\n==== All tests PASSED ====\n");
}

int main(void) { test_all(); }
