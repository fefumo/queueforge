# Stage 3: Thread-safe queue

The third stage of the project adds a thread-safe queue for producer-consumer
communication.

At this point, the project already has:

- a binary message parser;
- a single-threaded ring buffer for storing `Message` objects.

This stage does not replace the existing ring buffer. Instead, it wraps it with
synchronization primitives from `pthread`.

The goal is to make the queue safe for use by multiple threads.

## Purpose

The thread-safe queue allows one or more producer threads to push messages and
one or more consumer threads to pop messages without data races.

The queue is designed around a bounded buffer model:

```text
producer thread -> ts_queue_push()
consumer thread -> ts_queue_pop()
```

The internal storage is still the fixed-size `Queue` implemented in the previous
stage.

Synchronization is handled by:

## Public API

```c
TsQueueStatus ts_queue_init(TsQueue *q, size_t capacity);
void ts_queue_destroy(TsQueue *q);

TsQueueStatus ts_queue_push(TsQueue *q, const Message *msg);
TsQueueStatus ts_queue_pop(TsQueue *q, Message *out);

void ts_queue_close(TsQueue *q);
```
