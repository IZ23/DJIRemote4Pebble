#include "ble_port.h"

static BlePortState s_state = BLE_PORT_IDLE;
static ble_state_cb_t s_state_cb;
static ble_rx_cb_t s_rx_cb;

void ble_port_init(ble_state_cb_t state_cb, ble_rx_cb_t rx_cb) {
  s_state_cb = state_cb;
  s_rx_cb = rx_cb;
  (void)s_rx_cb;
  s_state = BLE_PORT_IDLE;
}
void ble_port_start_scan(void) {
  s_state = BLE_PORT_ERROR;
  if (s_state_cb) s_state_cb(s_state);
}
void ble_port_disconnect(void) {
  s_state = BLE_PORT_IDLE;
  if (s_state_cb) s_state_cb(s_state);
}
bool ble_port_write_fff5(const uint8_t *data, size_t len) {
  (void)data; (void)len;
  return false;
}
BlePortState ble_port_state(void) { return s_state; }
bool ble_port_get_local_identity_mac(uint8_t mac_out[6]) {
  if (mac_out) {
    for (int i = 0; i < 6; ++i) mac_out[i] = 0;
  }
  return false;
}
uint16_t ble_port_max_write_len(void) { return 0; }
