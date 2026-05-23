# queueforge

Small C project that implements binary message parsing, a fixed-size ring buffer,
and a thread-safe producer-consumer queue based on `pthread`.

The project is built around a simple binary message format. Parsed messages are
stored in a ring buffer and can also be passed between producer and consumer
threads through a thread-safe queue wrapper.

## Message format

Each message is represented as a byte buffer with the following layout:

```text
+---------+----------+------------+------------+--------------+
| magic   | version  | msg_type   | channel_id | payload_len  |
| 2 bytes | 1 byte   | 1 byte     | 2 bytes    | 2 bytes      |
+---------+----------+------------+------------+--------------+
| payload                                              |
| payload_len bytes                                    |
+------------------------------------------------------+
| checksum                                             |
| 1 byte                                               |
+------------------------------------------------------+
````

## Build

Build main target with:

```bash
make
```

Test everything with:

```bash
make test
```
