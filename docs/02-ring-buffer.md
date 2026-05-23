# Stage 2: Ring buffer

The second stage of the project implements a fixed-size ring buffer for storing
parsed `Message` objects.

At this point, the parser can already convert a binary byte buffer into a
validated `Message`. The next step is to store parsed messages in FIFO order
before they are processed by another part of the program.

This stage focuses on:

- implementing a fixed-size queue;
- storing structures by value;
- managing dynamic memory;
- preserving FIFO order;
- handling full and empty queue states;
- supporting wrap-around behavior;
- keeping ownership rules simple and explicit.

## Purpose

The ring buffer stores `Message` objects in a preallocated array.

It does not allocate memory on every push operation. Memory is allocated once
during initialization and released during destruction.

The queue has a fixed capacity.

When the queue is full, `queue_push()` returns an error instead of growing the
buffer dynamically.

When the queue is empty, `queue_pop()` returns an error instead of reading
invalid data.

## Ring buffer model

The queue uses a circular array.

```text
[ _ ][ _ ][ _ ][ _ ][ _ ][ _ ][ _ ][ _ ]
```

Two indexes are used:

```text
head - index of the next element to read
tail - index of the next position to write
```

The queue also stores the current number of elements:

```text
size - number of elements currently stored in the queue
```

When either `head` or `tail` reaches the end of the array, it wraps back to
index `0`.

```c
next = (index + 1) % capacity;
```

> Without `size`, the same `head == tail` state could mean either:
> 
> ```text
> the queue is empty
> ```
> or:
> ```text
> the queue is full after wrap-around
> ```
> Using `size` makes the implementation easier to understand and test.


## Ownership

The queue owns only its internal `data` array.

The caller owns the `Queue` object itself.

The caller also owns the input `Message` passed to `queue_push()` and the output
`Message` passed to `queue_pop()`.

Because messages are copied by value, the lifetime of the original input message
does not affect the queue after `queue_push()` returns.

## Notes

This stage intentionally keeps the queue single-threaded. Check next stage.
