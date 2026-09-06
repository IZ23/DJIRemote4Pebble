#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
typedef enum { BLE_PORT_IDLE=0, BLE_PORT_SCANNING, BLE_PORT_CONNECTING, BLE_PORT_CONNECTED, BLE_PORT_READY, BLE_PORT_ERROR } BlePortState;
typedef void (*ble_state_cb_t)(BlePortState state);
typedef void (*ble_rx_cb_t)(const uint8_t *data, size_t len);
void ble_port_init(ble_state_cb_t state_cb, ble_rx_cb_t rx_cb);
void ble_port_start_scan(void);
void ble_port_disconnect(void);
bool ble_port_write_fff5(const uint8_t *data, size_t len);
BlePortState ble_port_state(void);
bool ble_port_get_local_identity_mac(uint8_t mac_out[6]);
uint16_t ble_port_max_write_len(void);
