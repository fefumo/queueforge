#include "parser.h"
#include "queue.h"
#include <assert.h>
#include <stdlib.h>

// for quick messages
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

int test_push(void) {
  Queue q = {.capacity = 2};
  queue_init(&q, q.capacity);
  uint8_t pl[] = {0xf, 0xf, 0xf};
  Message some_message = MESSAGE_INIT(pl, 1);
  QueueStatus ret_val = queue_push(&q, &some_message);
  assert(ret_val == QUEUE_OK);
  printf("initial_payload: ");
  print_payload(&some_message);
  printf("queue payload: ");
  print_payload(&q.data[q.head]);
  assert(memcmp(some_message.payload, q.data[q.head].payload,
                some_message.payload_len) == 0);
  assert(q.head == 0 && q.tail == 1);
  queue_destroy(&q);
  return 0;
}

int test_push_and_pop(void) {
  Queue q = {.capacity = 2};
  queue_init(&q, q.capacity);
  uint8_t pl[] = {0xf, 0xf, 0xf};
  Message some_message = MESSAGE_INIT(pl, 1);
  QueueStatus ret_val = queue_push(&q, &some_message);
  assert(ret_val == QUEUE_OK);
  printf("initial_payload: ");
  print_payload(&some_message);
  printf("queue payload: ");
  print_payload(&q.data[q.head]);
  assert(memcmp(some_message.payload, q.data[q.head].payload,
                some_message.payload_len) == 0);
  assert(q.head == 0 && q.tail == 1 && q.size == 1);

  Message tmp = {0};
  queue_pop(&q, &tmp);
  assert(memcmp(tmp.payload, some_message.payload, tmp.payload_len) == 0);
  assert(tmp.type == 1);
  assert(q.head == 1 && q.tail == 1 && q.size == 0);
  queue_destroy(&q);
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
         memcmp(tmp.payload, msg1.payload, msg1.payload_len) == 0);
  assert(q.tail == 0 && q.head == 1 && tmp.type == 1);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, msg2.payload, msg2.payload_len) == 0);
  assert(q.tail == 0 && q.head == 2 && tmp.type == 2);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, msg3.payload, msg3.payload_len) == 0);
  assert(q.tail == 0 && q.head == 0 && tmp.type == 3);

  queue_destroy(&q);
  return 0;
}

int test_wrap_around(void) {

  /*
   * start:
   * [ _ ][ _ ][ _ ]
   */

  // create A,B,C messages
  uint8_t ap[] = {1, 1, 1};
  Message a = MESSAGE_INIT(ap, 1);
  assert(memcmp(a.payload, (uint8_t[]){1, 1, 1}, 3) == 0);

  uint8_t bp[] = {2, 2, 2};
  Message b = MESSAGE_INIT(bp, 2);
  assert(memcmp(b.payload, (uint8_t[]){2, 2, 2}, 3) == 0);

  uint8_t cp[] = {3, 3, 3};
  Message c = MESSAGE_INIT(cp, 3);
  assert(memcmp(c.payload, (uint8_t[]){3, 3, 3}, 3) == 0);

  Queue q = {.capacity = 3};
  Message tmp = {0};
  queue_init(&q, q.capacity);

  // push a,b,c messages
  // [ A ][ B ][ C ]
  assert(queue_push(&q, &a) == QUEUE_OK && q.size == 1 && q.tail == 1 &&
         q.head == 0);
  assert(queue_push(&q, &b) == QUEUE_OK && q.size == 2 && q.tail == 2 &&
         q.head == 0);
  assert(queue_push(&q, &c) == QUEUE_OK && q.size == 3 && q.tail == 0 &&
         q.head == 0);

  // pop a,b
  // [ _ ][ _ ][ C ]
  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, (uint8_t[]){1, 1, 1}, 3) == 0);
  assert(q.tail == 0 && q.head == 1 && q.size == 2);

  assert(queue_pop(&q, &tmp) == QUEUE_OK &&
         memcmp(tmp.payload, (uint8_t[]){2, 2, 2}, 3) == 0);
  assert(q.tail == 0 && q.head == 2 && q.size == 1);

  // create D,E messages
  uint8_t dp[] = {4, 4, 4};
  Message d = MESSAGE_INIT(dp, 4);

  uint8_t ep[] = {5, 5, 5};
  Message e = MESSAGE_INIT(ep, 4);

  // push D,E messages
  // [ D ][ E ][ C ]
  queue_push(&q, &d);
  assert(q.tail == 1 && q.head == 2 && q.size == 2);
  queue_push(&q, &e);
  assert(q.tail == 2 && q.head == 2 && q.size == 3);

  // pop C,D,E (because head is at 2)
  queue_pop(&q, &tmp);
  assert(q.tail == 2 && q.head == 0 && q.size == 2);
  assert(tmp.type == 3);
  assert(tmp.type == 3 && memcmp(tmp.payload, c.payload, c.payload_len) == 0);
  queue_pop(&q, &tmp);
  assert(q.tail == 2 && q.head == 1 && q.size == 1);
  assert(tmp.type == 4 && memcmp(tmp.payload, d.payload, d.payload_len) == 0);
  queue_pop(&q, &tmp);
  assert(q.tail == 2 && q.head == 2 && q.size == 0);
  assert(tmp.type == 4 && memcmp(tmp.payload, e.payload, e.payload_len) == 0);

  queue_destroy(&q);
  return 0;
}

int test_capacity1(void) {
  uint8_t pl[] = {0xa, 0xa, 0xa};
  uint8_t pl2[] = {0xb, 0xb, 0xb};
  Message msg = MESSAGE_INIT(pl, 1);
  Message msg2 = MESSAGE_INIT(pl2, 1);

  Queue q = {.capacity = 1};
  queue_init(&q, q.capacity);

  assert(queue_push(&q, &msg) == QUEUE_OK);
  assert(queue_push(&q, &msg2) == QUEUE_ERR_FULL);

  queue_destroy(&q);
  return 0;
}
int test_invalid_args(void) {
  assert(queue_init(NULL, 1) == QUEUE_ERR_INVALID_ARG);
  Queue q = {.capacity = 0};
  assert(queue_init(&q, q.capacity) == QUEUE_ERR_INVALID_ARG);
  return 0;
}

void test_all(void) {
  assert(test_init() == 0);
  printf("==== test init PASSED ====\n");
  assert(test_destroy() == 0);
  printf("==== test destroy PASSED ====\n");
  assert(test_push() == 0);
  printf("==== test push PASSED ====\n");
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
  assert(test_wrap_around() == 0);
  printf("==== test test_wrap_around PASSED ====\n");
  assert(test_capacity1() == 0);
  printf("==== test test_capacity1 PASSED ====\n");
  assert(test_invalid_args() == 0);
  printf("==== test test_invalid_args PASSED ====\n");
  printf("\n==== All tests PASSED ====\n");
}

int main(void) { test_all(); }
