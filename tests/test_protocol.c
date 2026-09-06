#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "dji_protocol.h"

static void test_official_mode_switch_vector(void) {
  const uint8_t payload[9] = {0x00,0x00,0x33,0xFF,0x0A,0x01,0x47,0x39,0x36};
  const uint8_t expected[] = {
    0xAA,0x1B,0x00,0x01,0,0,0,0,0x05,0x00,0x57,0xEE,
    0x1D,0x04,0x00,0x00,0x33,0xFF,0x0A,0x01,0x47,0x39,0x36,
    0xF4,0xFA,0xE1,0xD0
  };
  uint8_t out[64];
  size_t n = dji_build_frame(0x01, 5, 0x1D, 0x04,
                             payload, sizeof(payload), out, sizeof(out));
  assert(n == sizeof(expected));
  assert(memcmp(out, expected, sizeof(expected)) == 0);
}
static void test_record_key_frame(void) {
  uint8_t out[64];
  size_t n = dji_build_key_record(1, out, sizeof(out));
  assert(n == 22);
  assert(out[12] == 0x00 && out[13] == 0x11);
  assert(out[14] == 0x01 && out[15] == 0x01);
}
static void test_connection_request_is_single_51_byte_frame(void) {
  const uint8_t mac[6] = {0x38,0x34,0x56,0x78,0x9A,0xBC};
  uint8_t out[80];
  size_t n = dji_build_connection_request(7, 0x12345678u, mac, 1, 1234, out, sizeof(out));
  assert(n == 51);
  DjiFrameView v;
  assert(dji_parse_frame(out, n, &v));
  assert(v.payload_len == 33);
  assert(v.payload[4] == 6 && v.payload[26] == 1);
}
static void test_connection_response_is_27_bytes(void) {
  uint8_t out[40];
  size_t n = dji_build_connection_response(0x73, 0x12345678u, 0, out, sizeof(out));
  assert(n == 27);
  assert(out[3] == 0x20 && out[12] == 0x00 && out[13] == 0x19);
}
int main(void) {
  test_official_mode_switch_vector();
  test_record_key_frame();
  test_connection_request_is_single_51_byte_frame();
  test_connection_response_is_27_bytes();
  puts("DJIRemote4Pebble protocol tests: PASS");
  return 0;
}
