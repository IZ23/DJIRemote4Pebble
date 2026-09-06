#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "dji_protocol.h"
typedef enum { DJI_SESSION_IDLE=0,DJI_SESSION_WAIT_CAMERA_ACK,DJI_SESSION_WAIT_CAMERA_REQUEST,DJI_SESSION_CONNECTED,DJI_SESSION_REJECTED,DJI_SESSION_ERROR } DjiSessionState;
typedef bool (*dji_session_send_fn)(const uint8_t*,size_t);
void dji_session_init(dji_session_send_fn);
bool dji_session_start_first_pairing(const uint8_t[6],uint16_t);
void dji_session_on_frame(const uint8_t*,size_t);
bool dji_session_send_record_toggle(void);
DjiSessionState dji_session_state(void);
uint16_t dji_session_verify_code(void);
uint32_t dji_session_camera_device_id(void);
const DjiCameraStatus *dji_session_camera_status(void);
