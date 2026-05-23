#include "queue.h"
#include <stdlib.h>

inline int queue_is_empty(const Queue *q) {
  if (q->size == 0)
    return 1;
  return 0;
}

inline int queue_is_full(const Queue *q) {
  if (q->size == q->capacity)
    return 1;
  return 0;
}

size_t queue_size(const Queue *q) { return q->size; }

size_t queue_capacity(const Queue *q) { return q->capacity; }

QueueStatus queue_init(Queue *q, size_t capacity) {
  if (!q || capacity == 0) {
    return QUEUE_ERR_INVALID_ARG;
  }
  Message *qp = malloc(capacity * sizeof(Message));
  if (qp == NULL) {
    return QUEUE_ERR_MALLOC;
  }
  q->data = qp;
  q->capacity = capacity;
  q->head = 0;
  q->size = 0;
  q->tail = 0;

  return QUEUE_OK;
}

void queue_destroy(Queue *q) {
  if (!q)
    return;
  free(q->data);
  q->data = NULL;
  q->capacity = 0;
  q->head = 0;
  q->tail = 0;
  q->size = 0;
}

QueueStatus queue_push(Queue *q, const Message *msg) {
  if (!q || !msg || !q->data || q->capacity == 0)
    return QUEUE_ERR_INVALID_ARG;
  if (queue_is_full(q))
    return QUEUE_ERR_FULL;
  q->data[q->tail] = *msg; // copy by value
  q->tail = (q->tail + 1) % q->capacity;
  q->size++;
  return QUEUE_OK;
}

QueueStatus queue_pop(Queue *q, Message *out) {
  if (!q || !out || !q->data || q->capacity == 0)
    return QUEUE_ERR_INVALID_ARG;
  if (queue_size(q) == 0)
    return QUEUE_ERR_EMPTY;
  *out = q->data[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->size--;
  return QUEUE_OK;
}
