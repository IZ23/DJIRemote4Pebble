#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#define DJI_SOF 0xAA
#define DJI_ACTION4_DEVICE_ID 0x0000FF33u
#define DJI_POC_CONTROLLER_ID 0x12345678u
#define DJI_CONNECTION_REQUEST_FRAME_LEN 51u
typedef struct { uint8_t cmd_type; uint16_t seq; uint8_t cmd_set; uint8_t cmd_id; const uint8_t *payload; size_t payload_len; } DjiFrameView;
typedef struct { uint8_t camera_mode,camera_status,video_resolution,fps_idx,eis_mode; uint16_t record_time_s; uint32_t remain_time_s; uint8_t power_mode,temperature_state,battery_percent; bool recording; } DjiCameraStatus;
uint16_t dji_crc16(const uint8_t*,size_t); uint32_t dji_crc32(const uint8_t*,size_t);
size_t dji_build_frame(uint8_t,uint16_t,uint8_t,uint8_t,const uint8_t*,size_t,uint8_t*,size_t);
bool dji_parse_frame(const uint8_t*,size_t,DjiFrameView*);
size_t dji_build_key_record(uint16_t,uint8_t*,size_t);
size_t dji_build_status_subscribe(uint16_t,uint8_t*,size_t);
size_t dji_build_record_control(uint16_t,bool,uint8_t*,size_t);
size_t dji_build_connection_request(uint16_t,uint32_t,const uint8_t[6],uint8_t,uint16_t,uint8_t*,size_t);
size_t dji_build_connection_response(uint16_t,uint32_t,uint32_t,uint8_t*,size_t);
bool dji_parse_status_push(const uint8_t*,size_t,DjiCameraStatus*);
