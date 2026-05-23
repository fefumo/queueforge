#include "parser.h"

/*
 * VERY unsafe constructor only used for quick init in tests.
 * Sets everything except the payload and type of a message object to 0
 * */
Message message_init(uint8_t *payload, size_t len, MsgType type) {
  if (!payload)
    return (Message){0};
  if (len > 256)
    len = 256;
  if (type > 4)
    type = type % 4;
  Message msg = {
      .payload_len = len, .type = type, .version = 1, .channel_id = 0x0001};
  memcpy(msg.payload, payload, len);
  return msg;
}

const char *type_to_str(ParseStatus parse_status) {
  switch (parse_status) {
  case PARSE_OK:
    return "parsed correctly";
  case PARSE_ERR_TOO_SHORT:
    return "too short";
  case PARSE_ERR_BAD_MAGIC:
    return "bad magic number";
  case PARSE_ERR_BAD_VERSION:
    return "bad version";
  case PARSE_ERR_BAD_TYPE:
    return "bad type";
  case PARSE_ERR_BAD_CHANNEL:
    return "bad channel";
  case PARSE_ERR_PAYLOAD_TOO_LARGE:
    return "payload is too large";
  case PARSE_ERR_TRUNCATED:
    return "truncated";
  case PARSE_ERR_BAD_CHECKSUM:
    return "bad checksum";
  default:
    return "parse status unknown";
  }
}

uint16_t calc_sum(const uint8_t *buf, const size_t len) {
  uint16_t res = 0;
  for (size_t i = 0; i < len; i++) {
    printf("buf[i] = %d\n", buf[i]);
    res += buf[i];
  }
  return res;
}

void print_payload(Message *msg) {
  size_t i = 0;
  printf("[");
  for (; i < msg->payload_len; i++) {
    printf("%d", msg->payload[i]);
    if (i + 1 < msg->payload_len) {
      printf(", ");
    }
  }

  printf("]\n");
}
