#include "parser.h"

static ParseStatus validate_header(uint16_t magic, uint8_t version,
                                   uint8_t msg_type, uint16_t channel_id,
                                   uint16_t payload_len) {
  if (((magic >> 8) != 0xCA) || ((magic & 0xFF) != 0xFE)) {
    fprintf(stderr, "magic left: %x, maigc right: %x\n", (magic >> 8),
            (magic & 0xFF));
    return PARSE_ERR_BAD_MAGIC;
  }

  if (version != 1) {
    fprintf(stderr, "message version: %d\n", version);
    return PARSE_ERR_BAD_VERSION;
  }

  if (msg_type < 1 || msg_type > 4) {
    fprintf(stderr, "message type: %d\n", msg_type);
    return PARSE_ERR_BAD_TYPE;
  }

  if (channel_id > 1023) {
    fprintf(stderr, "message channel_id: %d\n", channel_id);
    return PARSE_ERR_BAD_CHANNEL;
  }

  if (payload_len > 256) {
    fprintf(stderr, "message payload_len: %d\n", payload_len);
    return PARSE_ERR_PAYLOAD_TOO_LARGE;
  }

  return PARSE_OK;
}

ParseStatus parse_message(const uint8_t *buf, size_t len, Message *out) {
  if (buf == NULL || out == NULL)
    return PARSE_ERR_TRUNCATED;

  if (len < 9)
    return PARSE_ERR_TOO_SHORT;

  uint16_t magic;
  uint8_t version;
  uint8_t msg_type;
  uint16_t channel_id;
  uint16_t payload_len;
  size_t pos = 0;
  uint8_t checksum;

  magic = read_be16(buf, &pos);
  version = buf[pos++];
  msg_type = buf[pos++];
  channel_id = read_be16(buf, &pos);
  payload_len = read_be16(buf, &pos);

  ParseStatus status =
      validate_header(magic, version, msg_type, channel_id, payload_len);
  if (status != PARSE_OK)
    return status;

  if (len < pos + payload_len + 1)
    return PARSE_ERR_TRUNCATED;
  Message tmp = {0};

  tmp.version = version;
  tmp.type = (MsgType)msg_type;
  tmp.channel_id = channel_id;
  tmp.payload_len = payload_len;

  printf(" magic       = %x,\n version     = %x,\n msg_type    = %x,\n "
         "channel_id  = %x,\n payload_len = %x\n",
         magic, version, msg_type, channel_id, payload_len);

  // printf("buf_pos: %zu, payload_len: %d\n", pos, payload_len);

  for (size_t i = 0; i < payload_len; ++i) {
    tmp.payload[i] = buf[pos++];
  }

  checksum = buf[pos];
  printf(" checksum     = %d\n", checksum);
  uint16_t buf_sum = calc_sum(buf, pos);
  printf("buf_sum: %d, buf_sum mod 256: %d\n", buf_sum, buf_sum % 256);
  if (checksum != buf_sum % 256)
    return PARSE_ERR_BAD_CHECKSUM;

  *out = tmp;
  return PARSE_OK;
}
