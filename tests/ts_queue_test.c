#include "parser.h"
#include "ts_queue.h"
#include <assert.h>
#include <stdlib.h>

#define THREAD_NUM 2
#define MESSAGE_COUNT 10000
#define QUEUE_CAPACITY 8

/*
 * Creates a message with its actual id set in the first 4 bytes of payload
 *
 * Used for cases when there are more than 1023 messages available
 * (i.e. real-life implementation of a message queue)
 * */
static Message make_msg(uint32_t id) {
  Message msg = {0};

  msg.version = 1;
  msg.type = MSG_DATA_FRAME;
  msg.channel_id = id % 1024;
  msg.payload_len = 4;

  msg.payload[0] = (uint8_t)(id >> 24 & 0xFF);
  msg.payload[1] = (uint8_t)(id >> 16 & 0xFF);
  msg.payload[2] = (uint8_t)(id >> 8 & 0xFF);
  msg.payload[3] = (uint8_t)(id & 0xFF);

  return msg;
}

static uint32_t get_id(Message *msg) {
  uint32_t res = 0;
  res = (uint32_t)msg->payload[0] << 24 | (uint32_t)msg->payload[1] << 16 |
        (uint32_t)msg->payload[2] << 8 | (uint32_t)msg->payload[3];
  // printf("got id: 0x%08x\n", res);
  return res;
}

static void *start_pushing(void *arg) {
  TsQueue *ts_queue = (TsQueue *)arg;
  for (uint32_t i = 0; i < MESSAGE_COUNT; ++i) {
    Message msg = make_msg(i);
    assert(ts_queue_push(ts_queue, &msg) == TS_QUEUE_OK);
    // printf("PUSHED msg %d\n", i);
  }

  ts_queue_close(ts_queue);
  pthread_exit(NULL);
}

static void *start_popping(void *arg) {
  TsQueue *ts_queue = (TsQueue *)arg;
  Message msg = {0};
  for (uint32_t i = 0; i < MESSAGE_COUNT; ++i) {
    assert(ts_queue_pop(ts_queue, &msg) == TS_QUEUE_OK);
    // printf("POPPED msg %d\n", i);
    uint32_t id = get_id(&msg);
    assert(id == i);
  }
  assert(ts_queue_pop(ts_queue, &msg) == TS_QUEUE_CLOSED);

  pthread_exit(NULL);
}

int main(void) {
  TsQueue ts_queue;
  int i, rs;
  int created = 0;
  pthread_t threads[THREAD_NUM];

  if (ts_queue_init(&ts_queue, QUEUE_CAPACITY) != TS_QUEUE_OK)
    exit(EXIT_FAILURE);

  printf("Running %d messages through %d threads on %d queue capacity\n",
         MESSAGE_COUNT, THREAD_NUM, QUEUE_CAPACITY);

  for (i = 0; i < THREAD_NUM; ++i) {
    if (i % 2 == 0) {
      // printf("creating pushing thread...\n");
      rs = pthread_create(&threads[i], NULL, start_pushing, &ts_queue);
    } else {
      // printf("creating popping thread...\n");
      rs = pthread_create(&threads[i], NULL, start_popping, &ts_queue);
    }

    if (rs != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(rs));
      break;
    }

    created++;
  }
  printf("waiting for the threads to end...\n");
  for (i = 0; i < created; ++i) {
    if (pthread_join(threads[i], NULL) != 0)
      perror("i am too lazy to fix the thread errors lol, it shouldn't 've "
             "happenned\n");
  }
  // printf("destroying...\n");
  ts_queue_destroy(&ts_queue);
  printf("Just went through <%d> messages!!\n", MESSAGE_COUNT);
  printf("========================= DONE =========================");
  return 0;
}
