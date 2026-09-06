/*
 * DJIRemote4Pebble PoC 0.4
 * BLE Central/GATT backend for a PebbleOS internal SDK build.
 */
#include <pebble.h>

#include "ble_port.h"

static BlePortState s_state = BLE_PORT_IDLE;
static ble_state_cb_t s_state_cb;
static ble_rx_cb_t s_rx_cb;

static BTDevice s_device;
static bool s_have_device;
static BLECharacteristic s_notify_fff4;
static BLECharacteristic s_write_fff5;
static bool s_have_fff4;
static bool s_have_fff5;

static Uuid s_uuid_fff0;
static Uuid s_uuid_fff4;
static Uuid s_uuid_fff5;

static void set_state(BlePortState state) {
  s_state = state;
  if (s_state_cb) s_state_cb(state);
}

static bool uuid_eq(Uuid a, Uuid b) {
  return uuid_equal(&a, &b);
}

static bool is_dji_advertisement(const BLEAdData *ad) {
  uint8_t data[32] = {0};
  uint16_t company_id = 0;
  size_t n = ble_ad_copy_manufacturer_specific_data(ad, &company_id,
                                                     data, sizeof(data));
  (void)company_id;
  return n >= 5 && data[0] == 0xAA && data[1] == 0x08 && data[4] == 0xFA;
}

static void scan_handler(BTDevice device, int8_t rssi, const BLEAdData *ad) {
  (void)rssi;
  if (!is_dji_advertisement(ad)) return;

  s_device = device;
  s_have_device = true;
  ble_scan_stop();
  set_state(BLE_PORT_CONNECTING);

  BTErrno e = ble_central_connect(s_device, true, false);
  if (e != BTErrnoOK) set_state(BLE_PORT_ERROR);
}

static void connection_handler(BTDevice device, BTErrno status) {
  (void)device;
  if (status != BTErrnoConnected) {
    s_have_fff4 = false;
    s_have_fff5 = false;
    set_state(BLE_PORT_CONNECTING);
    return;
  }

  set_state(BLE_PORT_CONNECTED);
  ble_client_set_service_filter(&s_uuid_fff0, 1);
  if (ble_client_discover_services_and_characteristics(s_device) != BTErrnoOK) {
    set_state(BLE_PORT_ERROR);
  }
}

static void subscribe_handler(BLECharacteristic characteristic,
                              BLESubscription subscription_type,
                              BLEGATTError error) {
  (void)characteristic;
  if (error == BLEGATTErrorSuccess &&
      subscription_type != BLESubscriptionNone && s_have_fff5) {
    set_state(BLE_PORT_READY);
  } else {
    set_state(BLE_PORT_ERROR);
  }
}

static void read_handler(BLECharacteristic characteristic,
                         const uint8_t *value,
                         size_t value_length,
                         uint16_t value_offset,
                         BLEGATTError error) {
  (void)value_offset;
  if (error != BLEGATTErrorSuccess || !s_rx_cb) return;
  if (s_have_fff4 &&
      uuid_eq(ble_characteristic_get_uuid(characteristic), s_uuid_fff4)) {
    s_rx_cb(value, value_length);
  }
}

static void services_handler(BTDevice device,
                             BLEClientServiceChangeUpdate update_type,
                             const BLEService services[],
                             uint8_t num_services,
                             BTErrno status) {
  (void)device;
  if (status != BTErrnoOK) {
    set_state(BLE_PORT_ERROR);
    return;
  }

  if (update_type == BLEClientServicesRemoved ||
      update_type == BLEClientServicesInvalidateAll) {
    s_have_fff4 = false;
    s_have_fff5 = false;
    return;
  }

  for (uint8_t s = 0; s < num_services; ++s) {
    if (!uuid_eq(ble_service_get_uuid(services[s]), s_uuid_fff0)) continue;
    BLECharacteristic chars[16];
    uint8_t count = ble_service_get_characteristics(services[s], chars, 16);
    if (count > 16) count = 16;

    for (uint8_t i = 0; i < count; ++i) {
      Uuid u = ble_characteristic_get_uuid(chars[i]);
      if (uuid_eq(u, s_uuid_fff4)) {
        s_notify_fff4 = chars[i];
        s_have_fff4 = true;
      } else if (uuid_eq(u, s_uuid_fff5)) {
        s_write_fff5 = chars[i];
        s_have_fff5 = true;
      }
    }
  }

  if (!s_have_fff4 || !s_have_fff5) {
    set_state(BLE_PORT_ERROR);
    return;
  }
  if (ble_client_subscribe(s_notify_fff4, BLESubscriptionNotifications) != BTErrnoOK) {
    set_state(BLE_PORT_ERROR);
  }
}

void ble_port_init(ble_state_cb_t state_cb, ble_rx_cb_t rx_cb) {
  s_state_cb = state_cb;
  s_rx_cb = rx_cb;
  s_uuid_fff0 = bt_uuid_expand_16bit(0xFFF0);
  s_uuid_fff4 = bt_uuid_expand_16bit(0xFFF4);
  s_uuid_fff5 = bt_uuid_expand_16bit(0xFFF5);

  ble_central_set_connection_handler(connection_handler);
  ble_client_set_service_change_handler(services_handler);
  ble_client_set_read_handler(read_handler);
  ble_client_set_subscribe_handler(subscribe_handler);
  set_state(BLE_PORT_IDLE);
}

void ble_port_start_scan(void) {
  s_have_device = false;
  s_have_fff4 = false;
  s_have_fff5 = false;
  set_state(BLE_PORT_SCANNING);
  if (ble_scan_start(scan_handler) != BTErrnoOK) set_state(BLE_PORT_ERROR);
}

void ble_port_disconnect(void) {
  if (ble_scan_is_scanning()) ble_scan_stop();
  if (s_have_device) ble_central_cancel_connect(s_device);
  s_have_device = false;
  s_have_fff4 = false;
  s_have_fff5 = false;
  set_state(BLE_PORT_IDLE);
}

bool ble_port_write_fff5(const uint8_t *data, size_t len) {
  if (!s_have_fff5 || s_state != BLE_PORT_READY) return false;
  return ble_client_write(s_write_fff5, data, len) == BTErrnoOK;
}

BlePortState ble_port_state(void) {
  return s_state;
}

bool ble_port_get_local_identity_mac(uint8_t mac_out[6]) {
  if (!mac_out) return false;
  const uint8_t id[6] = {0x02, 0x12, 0x34, 0x56, 0x78, 0x9A};
  for (int i = 0; i < 6; ++i) mac_out[i] = id[i];
  return true;
}

uint16_t ble_port_max_write_len(void) {
  if (!s_have_device) return 0;
  return ble_client_get_maximum_value_length(s_device);
}
