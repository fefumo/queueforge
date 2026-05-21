#ifndef PARSER_H
#define PARSER_H

/* valid message example:
 * magic       = CA FE
 * version     = 01
 * msg_type    = 03
 * channel_id  = 00 2A
 * payload_len = 00 03
 * payload     = 10 20 30
 *
 * visualization:
 * +---------+----------+------------+------------+--------------+
 * | magic   | version  | msg_type   | channel_id | payload_len  |
 * | 2 bytes | 1 byte   | 1 byte     | 2 bytes    | 2 bytes      |
 * +---------+----------+------------+------------+--------------+
 * | payload                                              |
 * | payload_len bytes                                    |
 * +------------------------------------------------------+
 * | checksum                                             |
 * | 1 byte                                               |
 * +------------------------------------------------------+
 *
 * boundaries:
 * magic = 0xCAFE
 * version = 1
 * msg_type:
 *     1 = CHAN_ACTIVATE
 *     2 = CHAN_RELEASE
 *     3 = DATA_FRAME
 *     4 = MEASUREMENT_REPORT
 *
 * channel_id:
 *     0..1023
 *
 * payload_len:
 *     0..256
 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef enum {
  PARSE_OK = 0,
  PARSE_ERR_TOO_SHORT,
  PARSE_ERR_BAD_MAGIC,
  PARSE_ERR_BAD_VERSION,
  PARSE_ERR_BAD_TYPE,
  PARSE_ERR_BAD_CHANNEL,
  PARSE_ERR_PAYLOAD_TOO_LARGE,
  PARSE_ERR_TRUNCATED,
  PARSE_ERR_BAD_CHECKSUM
} ParseStatus;

typedef enum {
  MSG_CHAN_ACTIVATE = 1,
  MSG_CHAN_RELEASE = 2,
  MSG_DATA_FRAME = 3,
  MSG_MEASUREMENT_REPORT = 4
} MsgType;

typedef struct {
  uint8_t version;
  MsgType type;
  uint16_t channel_id;
  uint16_t payload_len;
  uint8_t payload[256];
} Message;

Message message_init(uint8_t *payload, MsgType type);
ParseStatus parse_message(const uint8_t *buf, size_t len, Message *out);
uint16_t calc_sum(const uint8_t *buf, const size_t len);
const char *type_to_str(ParseStatus parse_status);
void print_payload(Message *msg);

// big-endian
// example input 0x1234
// representation in bytes
// 1  2
// |  |
// v  v
// 12 34
// 0x12 << 8 = 0x1200
// 0x1200 | 34 = 0x1234
static inline uint16_t read_be16(const uint8_t *buf, size_t *pos) {
  uint16_t value = ((uint16_t)buf[*pos] << 8) | ((uint16_t)buf[*pos + 1]);

  *pos += 2;
  return value;
}

#endif // !PARSER_H
