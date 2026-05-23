#include "ts_queue.h"
#include "queue.h"
#include <pthread.h>
#include <stdlib.h>

void cleanup_exit(TsQueue *ts_q, const char *message) {
  ts_queue_destroy(ts_q);

  perror(message);

  exit(EXIT_FAILURE);
}

void ts_queue_close(TsQueue *ts_q) {
  pthread_mutex_lock(&ts_q->mutex);
  ts_q->closed = 1;
  pthread_mutex_unlock(&ts_q->mutex);
}

// TODO: create an interface to pass QueueStatus errors upwards in code
// same goes to ts_queue_push/pop ret values
TsQueueStatus ts_queue_init(TsQueue *ts_q, size_t capacity) {
  if (!ts_q || capacity == 0)
    return TS_QUEUE_ERR_INVALID_ARG;

  QueueStatus queue_status_res = queue_init(&ts_q->queue, capacity);

  if (queue_status_res != QUEUE_OK)
    cleanup_exit(ts_q, "Error: failed to initialize queue.");

  if (pthread_mutex_init(&ts_q->mutex, NULL))
    cleanup_exit(ts_q, "Error: failed to initialize mutex.");
  if (pthread_cond_init(&ts_q->not_empty, NULL))
    cleanup_exit(ts_q, "Error: failed to initialize cond variable not_empty.");
  if (pthread_cond_init(&ts_q->not_full, NULL))
    cleanup_exit(ts_q, "Error: failed to initialize cond variable not_full.");

  return TS_QUEUE_OK;
}

void ts_queue_destroy(TsQueue *ts_q) {
  if (!ts_q)
    return;
  if (ts_q->queue.data != NULL)
    queue_destroy(&ts_q->queue);
  pthread_mutex_destroy(&ts_q->mutex);
  pthread_cond_destroy(&ts_q->not_empty);
  pthread_cond_destroy(&ts_q->not_full);
}

TsQueueStatus ts_queue_push(TsQueue *ts_q, const Message *msg) {
  if (!ts_q || !msg || !ts_q->queue.data)
    return TS_QUEUE_ERR_INVALID_ARG;
  // printf("in ts_queue_push\n");

  Queue *qp = &ts_q->queue;

  pthread_mutex_lock(&ts_q->mutex);

  while (queue_is_full(&ts_q->queue) || ts_q->closed == 1) {
    pthread_cond_wait(&ts_q->not_full, &ts_q->mutex);
  }

  if (ts_q->closed) {
    pthread_mutex_unlock(&ts_q->mutex);
    return TS_QUEUE_CLOSED;
  }

  // printf("queue is not full -> performing a queue push\n");
  QueueStatus ret_val = queue_push(qp, msg);
  if (ret_val != QUEUE_OK) {
    pthread_mutex_unlock(&ts_q->mutex);
    return TS_QUEUE_ERR_INVALID_ARG;
  }
  // printf("just pushed a message...\n");

  pthread_cond_signal(&ts_q->not_empty);
  pthread_mutex_unlock(&ts_q->mutex);
  return TS_QUEUE_OK;
}

TsQueueStatus ts_queue_pop(TsQueue *ts_q, Message *out) {

  if (!ts_q || !out || !ts_q->queue.data)
    return TS_QUEUE_ERR_INVALID_ARG;
  Queue *qp = &ts_q->queue;
  // printf("in ts_queue_pop\n");

  pthread_mutex_lock(&ts_q->mutex);

  while (queue_is_empty(&ts_q->queue) && !ts_q->closed) {
    pthread_cond_wait(&ts_q->not_empty, &ts_q->mutex);
  }

  if (queue_is_empty(&ts_q->queue) && ts_q->closed) {
    pthread_mutex_unlock(&ts_q->mutex);
    return TS_QUEUE_CLOSED;
  }

  // printf("queue is not empty -> performing a queue pop\n");
  QueueStatus ret_val = queue_pop(qp, out);
  if (ret_val != QUEUE_OK) {
    pthread_mutex_unlock(&ts_q->mutex);
    return TS_QUEUE_ERR_INVALID_ARG;
  }
  // printf("just popped a message...\n");

  pthread_cond_signal(&ts_q->not_full);
  pthread_mutex_unlock(&ts_q->mutex);
  return TS_QUEUE_OK;
}
