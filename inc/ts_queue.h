#ifndef TS_QUEUE_H
#define TS_QUEUE_H

#include "queue.h"
#include <pthread.h>
#include <stddef.h>

typedef enum {
  TS_QUEUE_OK = 0,
  TS_QUEUE_CLOSED,
  TS_QUEUE_ERR_INVALID_ARG,
  TS_QUEUE_ERR_INIT
} TsQueueStatus;

typedef struct {
  Queue queue;
  pthread_mutex_t mutex;
  pthread_cond_t not_empty; // used to wake consumers when a message becomes available
  pthread_cond_t not_full;  // used to wake producers when free space becomes available
  int closed; // indicates that no new messages should be accepted
} TsQueue;

TsQueueStatus ts_queue_init(TsQueue *ts_q, size_t capacity);
void ts_queue_destroy(TsQueue *ts_q);

TsQueueStatus ts_queue_push(TsQueue *ts_q, const Message *msg);
TsQueueStatus ts_queue_pop(TsQueue *ts_q, Message *out);

void cleanup_exit(TsQueue *ts_q, const char *message);

void ts_queue_close(TsQueue *ts_q);
void cleanup_exit(TsQueue *ts_q, const char *message);

#endif // !TS_QUEUE_H
