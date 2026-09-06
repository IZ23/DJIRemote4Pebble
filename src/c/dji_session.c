#include "dji_session.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

static dji_session_send_fn s_send;
static DjiSessionState s_state = DJI_SESSION_IDLE;
static uint16_t s_seq = 1;
static uint16_t s_verify_code;
static uint32_t s_camera_device_id;
static DjiCameraStatus s_status;

static uint16_t le16(const uint8_t *p) {
  return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}
static uint32_t le32(const uint8_t *p) {
  return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}
static bool send_frame(const uint8_t *data, size_t len) {
  return s_send && s_send(data, len);
}
void dji_session_init(dji_session_send_fn send_fn) {
  s_send = send_fn; s_state = DJI_SESSION_IDLE; s_seq = 1;
  s_verify_code = 0; s_camera_device_id = 0;
  memset(&s_status, 0, sizeof(s_status));
}
bool dji_session_start_first_pairing(const uint8_t local_mac[6],
                                     uint16_t max_single_write_len) {
  if (!local_mac || max_single_write_len < DJI_CONNECTION_REQUEST_FRAME_LEN) {
    s_state = DJI_SESSION_ERROR; return false;
  }
  srand((unsigned int)time(NULL));
  s_verify_code = (uint16_t)(rand() % 10000u);
  uint8_t frame[64];
  size_t n = dji_build_connection_request(
      s_seq++, DJI_POC_CONTROLLER_ID, local_mac, 1, s_verify_code,
      frame, sizeof(frame));
  if (n != DJI_CONNECTION_REQUEST_FRAME_LEN || !send_frame(frame, n)) {
    s_state = DJI_SESSION_ERROR; return false;
  }
  s_state = DJI_SESSION_WAIT_CAMERA_ACK; return true;
}
static void handle_connection_frame(const DjiFrameView *v) {
  if (v->payload_len == 9) {
    uint8_t ret_code = v->payload[4];
    if (ret_code != 0) { s_state = DJI_SESSION_ERROR; return; }
    s_state = DJI_SESSION_WAIT_CAMERA_REQUEST; return;
  }
  if (v->payload_len >= 33) {
    uint8_t verify_mode = v->payload[26];
    uint16_t verify_data = le16(v->payload + 27);
    s_camera_device_id = le32(v->payload + 0);
    if (verify_mode != 2) { s_state = DJI_SESSION_ERROR; return; }
    if (verify_data != 0) { s_state = DJI_SESSION_REJECTED; return; }
    uint8_t response[40];
    size_t n = dji_build_connection_response(
        v->seq, DJI_POC_CONTROLLER_ID, 0, response, sizeof(response));
    if (!n || !send_frame(response, n)) { s_state = DJI_SESSION_ERROR; return; }
    s_state = DJI_SESSION_CONNECTED;
    uint8_t sub[32];
    n = dji_build_status_subscribe(s_seq++, sub, sizeof(sub));
    if (!n || !send_frame(sub, n)) s_state = DJI_SESSION_ERROR;
  }
}
void dji_session_on_frame(const uint8_t *frame, size_t len) {
  DjiFrameView v;
  if (!dji_parse_frame(frame, len, &v)) return;
  if (v.cmd_set == 0x00 && v.cmd_id == 0x19) { handle_connection_frame(&v); return; }
  (void)dji_parse_status_push(frame, len, &s_status);
}
bool dji_session_send_record_toggle(void) {
  if (s_state != DJI_SESSION_CONNECTED) return false;
  uint8_t frame[32];
  size_t n = dji_build_key_record(s_seq++, frame, sizeof(frame));
  return n && send_frame(frame, n);
}
DjiSessionState dji_session_state(void) { return s_state; }
uint16_t dji_session_verify_code(void) { return s_verify_code; }
uint32_t dji_session_camera_device_id(void) { return s_camera_device_id; }
const DjiCameraStatus *dji_session_camera_status(void) { return &s_status; }
