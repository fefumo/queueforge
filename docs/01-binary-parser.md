# Stage 1: Binary parser

The first stage of the project implements a binary message parser.

The goal of this stage is to safely read a byte buffer, validate its contents,
and convert it into a normal C structure that can be used by the rest of the
program.

This stage focuses on:

- working with raw byte buffers;
- parsing multi-byte integer fields;
- handling big-endian data;
- validating input length before reading from memory;
- checking protocol fields;
- calculating and validating checksum;
- returning explicit parse error statuses.

## Message format

Each input message is represented as a byte buffer.

The binary layout is:

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
```

The header size is 8 bytes:

```text
magic       uint16_t
version     uint8_t
msg_type    uint8_t
channel_id  uint16_t
payload_len uint16_t
```

The following fields are stored in big-endian order:

```text
magic
channel_id
payload_len
```

## Field constraints

The parser validates the following field values:

```text
magic       = 0xCAFE
version     = 1

msg_type:
    1 = MSG_CHAN_ACTIVATE
    2 = MSG_CHAN_RELEASE
    3 = MSG_DATA_FRAME
    4 = MSG_MEASUREMENT_REPORT

channel_id:
    0..1023

payload_len:
    0..256
```

## Checksum

The checksum is calculated as the sum of all bytes from the beginning of the
message up to the end of the payload, modulo 256.

The checksum byte itself is not included in the sum.

```text
checksum = sum(message bytes from magic to payload end) mod 256
```

Example:

```text
magic       = CA FE
version     = 01
msg_type    = 03
channel_id  = 00 2A
payload_len = 00 03
payload     = 10 20 30
```

The checksum input bytes are:

```text
CA FE 01 03 00 2A 00 03 10 20 30
```

Their decimal sum is:

```text
202 + 254 + 1 + 3 + 0 + 42 + 0 + 3 + 16 + 32 + 48 = 601
```

Modulo 256:

```text
601 mod 256 = 89 = 0x59
```

The full valid message is:

```c
uint8_t valid_msg[] = {
    0xCA, 0xFE,
    0x01,
    0x03,
    0x00, 0x2A,
    0x00, 0x03,
    0x10, 0x20, 0x30,
    0x59
};
```

## Parsing approach

The parser reads bytes explicitly from the input buffer.

For example, a big-endian `uint16_t` value is read as:

```c
uint16_t value = ((uint16_t)buf[pos] << 8) | buf[pos + 1];
```

The implementation does not cast the input buffer to a structure.

This is intentional because direct casting would make the parser depend on:

- platform endianness;
- structure padding;
- alignment requirements;
- exact in-memory layout of C structures.

The binary format is treated as an external protocol format, not as a native C
memory layout.
