#ifndef QUEUE_H
#define QUEUE_H

#include "parser.h"
#include <stddef.h>

typedef enum {
  QUEUE_OK = 0,
  QUEUE_ERR_MALLOC,
  QUEUE_ERR_FULL,
  QUEUE_ERR_EMPTY,
  QUEUE_ERR_INVALID_ARG
} QueueStatus;

typedef struct {
  Message *data;
  size_t capacity;
  size_t head;
  size_t tail;
  size_t size;
} Queue;

QueueStatus queue_init(Queue *q, size_t capacity);
void queue_destroy(Queue *q);

QueueStatus queue_push(Queue *q, const Message *msg);
QueueStatus queue_pop(Queue *q, Message *out);

int queue_is_empty(const Queue *q);
int queue_is_full(const Queue *q);
size_t queue_size(const Queue *q);
size_t queue_capacity(const Queue *q);

#endif // !QUEUE_H
