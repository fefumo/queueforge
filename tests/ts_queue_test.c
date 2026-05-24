#include "ts_queue.h"
#include <assert.h>
#include <stdlib.h>

#define DEFAULT_THREAD_NUM 2
#define DEFAULT_MESSAGE_COUNT 10000
#define DEFAULT_QUEUE_CAPACITY 8

typedef struct {
  const size_t thread_num;
  const size_t message_count;
  const size_t queue_capacity;
} Test_inits;

// maybe improve on the intarface for pthread_create() in the furure if needed
typedef struct {
  size_t producer_id;
  size_t message_count;
  TsQueue *ts_q;
} ProducerArgs;

typedef struct {
  TsQueue *ts_q;

  uint8_t *seen;
  size_t total_messages;
  size_t *consumed_count;

  pthread_mutex_t *seen_mutex;
} ConsumerArgs;

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

static void *start_producing(void *arg) {
  ProducerArgs *args = (ProducerArgs *)arg;
  TsQueue *ts_queue = args->ts_q;
  const size_t message_count = args->message_count;

  /*
   * Each producer gets its own id range.
   *
   * producer 0: 0 .. message_count - 1
   * producer 1: message_count .. 2 * message_count - 1
   * producer 2: 2 * message_count .. 3 * message_count - 1
   */
  const uint32_t base_id = (uint32_t)(args->producer_id * message_count);

  for (uint32_t i = 0; i < message_count; ++i) {
    Message msg = make_msg(base_id + i);
    assert(ts_queue_push(ts_queue, &msg) == TS_QUEUE_OK);
    // printf("PUSHED msg %d\n", base_id + i);
  }

  // The main test thread closes the queue after all producers are joined.
  pthread_exit(NULL);
}

static void *start_consuming(void *arg) {
  ConsumerArgs *args = (ConsumerArgs *)arg;
  TsQueue *ts_queue = args->ts_q;
  Message msg = {0};

  while (1) {
    TsQueueStatus status = ts_queue_pop(ts_queue, &msg);

    if (status == TS_QUEUE_CLOSED) {
      break;
    }

    assert(status == TS_QUEUE_OK);

    uint32_t id = get_id(&msg);

    /*
     * Check the global property:
     *
     * - id is inside expected range
     * - id was not seen before
     * - every id will be checked after all consumers finish
     */
    pthread_mutex_lock(args->seen_mutex);

    assert(id < args->total_messages);
    assert(args->seen[id] == 0);

    args->seen[id] = 1;
    (*args->consumed_count)++;

    pthread_mutex_unlock(args->seen_mutex);

    // printf("POPPED msg %d\n", id);
  }

  pthread_exit(NULL);
}

static void join_threads(pthread_t *threads, size_t created) {
  for (size_t i = 0; i < created; ++i) {
    if (pthread_join(threads[i], NULL) != 0)
      perror("Something crazy happened while waiting for threads...\n");
  }
}

static void verify_all_messages_consumed(uint8_t *seen, size_t total_messages,
                                         size_t consumed_count) {
  assert(consumed_count == total_messages);

  for (size_t i = 0; i < total_messages; ++i) {
    assert(seen[i] == 1);
  }
}

int test_ts_queue(const size_t thread_num, const size_t message_count,
                  const size_t queue_capacity) {
  TsQueue ts_queue;
  int rs;

  if (thread_num < 2) {
    fprintf(stderr, "thread_num must be at least 2\n");
    return -1;
  }

  /*
   * For odd thread count:
   *   consumers get the extra thread
   */
  const size_t producer_count = thread_num / 2;
  const size_t consumer_count = thread_num - producer_count;
  const size_t total_messages = producer_count * message_count;

  assert(total_messages <= UINT32_MAX);

  uint8_t *seen = calloc(total_messages, sizeof(*seen));
  assert(seen != NULL);

  size_t consumed_count = 0;
  pthread_mutex_t seen_mutex;

  assert(pthread_mutex_init(&seen_mutex, NULL) == 0);

  pthread_t producers[producer_count];
  pthread_t consumers[consumer_count];
  ProducerArgs producer_args[producer_count];
  ConsumerArgs consumer_args = {
      .ts_q = &ts_queue,
      .seen = seen,
      .total_messages = total_messages,
      .consumed_count = &consumed_count,
      .seen_mutex = &seen_mutex,
  };

  if (ts_queue_init(&ts_queue, queue_capacity) != TS_QUEUE_OK)
    exit(EXIT_FAILURE);

  printf("Running %zu messages through %zu threads on %zu queue capacity\n",
         message_count, thread_num, queue_capacity);

  printf("Producers: %zu, consumers: %zu, total messages: %zu\n",
         producer_count, consumer_count, total_messages);

  size_t created_producers = 0;
  size_t created_consumers = 0;

  for (size_t i = 0; i < consumer_count; ++i) {
    // printf("creating consuming thread...\n");
    rs = pthread_create(&consumers[i], NULL, start_consuming, &consumer_args);

    if (rs != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(rs));
      break;
    }

    created_consumers++;
  }

  for (size_t i = 0; i < producer_count; ++i) {
    producer_args[i] = (ProducerArgs){
        .producer_id = i,
        .message_count = message_count,
        .ts_q = &ts_queue,
    };

    // printf("creating producing thread...\n");
    rs =
        pthread_create(&producers[i], NULL, start_producing, &producer_args[i]);

    if (rs != 0) {
      fprintf(stderr, "pthread_create failed: %s\n", strerror(rs));
      break;
    }

    created_producers++;
  }

  /*
   * First wait for all producers.
   *
   * After that we know that no new messages will be pushed.
   */
  // printf("waiting for the producers to end...\n");
  join_threads(producers, created_producers);

  // Now it is safe to close the queue.
  ts_queue_close(&ts_queue);

  /*
   * Now consumers should drain the remaining messages and then receive
   * TS_QUEUE_CLOSED.
   */
  // printf("waiting for the consumers to end...\n");
  join_threads(consumers, created_consumers);

  verify_all_messages_consumed(seen, total_messages, consumed_count);

  // printf("destroying...\n");
  ts_queue_destroy(&ts_queue);

  pthread_mutex_destroy(&seen_mutex);
  free(seen);

  printf("\n========================= DONE =========================\n");
  return 0;
}

void run_tests(void) {
  printf("Running double_thread test...\n");
  Test_inits double_thread = {2, DEFAULT_MESSAGE_COUNT, DEFAULT_QUEUE_CAPACITY};
  assert(test_ts_queue(double_thread.thread_num, double_thread.message_count,
                       double_thread.queue_capacity) == 0);

  printf("Running triple_thread test...\n");
  Test_inits triple_thread = {3, DEFAULT_MESSAGE_COUNT, DEFAULT_QUEUE_CAPACITY};
  assert(test_ts_queue(triple_thread.thread_num, triple_thread.message_count,
                       triple_thread.queue_capacity) == 0);

  printf("Running ten_thread test...\n");
  Test_inits ten_thread = {10, DEFAULT_MESSAGE_COUNT, DEFAULT_QUEUE_CAPACITY};
  assert(test_ts_queue(ten_thread.thread_num, ten_thread.message_count,
                       ten_thread.queue_capacity) == 0);
}

int main(void) {
  run_tests();
  return 0;
}
