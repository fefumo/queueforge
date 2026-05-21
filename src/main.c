#include "parser.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  uint8_t valid_msg[] = {0xCA, 0xFE, 0x01, 0x03, 0x00, 0x2A,
                         0x00, 0x03, 0x10, 0x20, 0x30, 0x59};
  printf("%zu\n", sizeof(valid_msg));
  // printf("%02x\n", valid_msg[0]);
  Message message = {0};
  // printf("sizeof message: %zu\n", sizeof(message));

  ParseStatus ret = parse_message(valid_msg, sizeof(valid_msg), &message);
  if (ret != 0) {
    printf("Parsing failed, error: %s\n", type_to_str(ret));
    return EXIT_FAILURE;
  }
  printf("%s\n", "Parsed correctly\n");

  uint8_t zero_payload_msg[] = {0xCA, 0xFE, 0x01, 0x03, 0x00,
                                0x2A, 0x00, 0x00, 0xF6};
  ret = parse_message(zero_payload_msg, sizeof(zero_payload_msg), &message);
  if (ret != 0) {
    printf("Parsing failed, error: %s\n", type_to_str(ret));
    return EXIT_FAILURE;
  }
  printf("%s\n", "Parsed correctly\n");
  return EXIT_SUCCESS;
}
