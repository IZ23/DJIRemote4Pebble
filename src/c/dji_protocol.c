#include "dji_protocol.h"
#include <string.h>

static void put_le16(uint8_t *p, uint16_t v) { p[0]=(uint8_t)(v&0xFF); p[1]=(uint8_t)((v>>8)&0xFF); }
static void put_le32(uint8_t *p, uint32_t v) { p[0]=(uint8_t)(v&0xFF); p[1]=(uint8_t)((v>>8)&0xFF); p[2]=(uint8_t)((v>>16)&0xFF); p[3]=(uint8_t)((v>>24)&0xFF); }
static uint16_t get_le16(const uint8_t *p) { return (uint16_t)p[0] | ((uint16_t)p[1]<<8); }
static uint32_t get_le32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }

uint16_t dji_crc16(const uint8_t *data, size_t len) {
  uint16_t crc=0x3AA3u;
  while(len--){ crc^=*data++; for(int i=0;i<8;++i) crc=(crc&1u)?(uint16_t)((crc>>1)^0xA001u):(uint16_t)(crc>>1); }
  return crc;
}
uint32_t dji_crc32(const uint8_t *data, size_t len) {
  uint32_t crc=0x00003AA3u;
  while(len--){ crc^=*data++; for(int i=0;i<8;++i) crc=(crc&1u)?((crc>>1)^0xEDB88320u):(crc>>1); }
  return crc;
}
size_t dji_build_frame(uint8_t cmd_type,uint16_t seq,uint8_t cmd_set,uint8_t cmd_id,
                       const uint8_t *payload,size_t payload_len,uint8_t *out,size_t out_cap){
  const size_t total=12+(2+payload_len)+4;
  if(!out||total>out_cap||total>0x3FFu) return 0;
  memset(out,0,total); out[0]=DJI_SOF; put_le16(out+1,(uint16_t)total); out[3]=cmd_type; out[4]=0;
  put_le16(out+8,seq); put_le16(out+10,dji_crc16(out,10)); out[12]=cmd_set; out[13]=cmd_id;
  if(payload_len) memcpy(out+14,payload,payload_len);
  put_le32(out+total-4,dji_crc32(out,total-4)); return total;
}
bool dji_parse_frame(const uint8_t *frame,size_t len,DjiFrameView *out){
  if(!frame||!out||len<18||frame[0]!=DJI_SOF) return false;
  if((get_le16(frame+1)&0x03FFu)!=len) return false;
  if(get_le16(frame+10)!=dji_crc16(frame,10)) return false;
  if(get_le32(frame+len-4)!=dji_crc32(frame,len-4)) return false;
  out->cmd_type=frame[3]; out->seq=get_le16(frame+8); out->cmd_set=frame[12]; out->cmd_id=frame[13];
  out->payload=frame+14; out->payload_len=len-18; return true;
}
size_t dji_build_key_record(uint16_t seq,uint8_t *out,size_t out_cap){
  const uint8_t payload[4]={0x01,0x01,0x00,0x00};
  return dji_build_frame(0x01,seq,0x00,0x11,payload,sizeof(payload),out,out_cap);
}
size_t dji_build_status_subscribe(uint16_t seq,uint8_t *out,size_t out_cap){
  const uint8_t payload[6]={0x03,20,0,0,0,0};
  return dji_build_frame(0x01,seq,0x1D,0x05,payload,sizeof(payload),out,out_cap);
}
size_t dji_build_record_control(uint16_t seq,bool start,uint8_t *out,size_t out_cap){
  uint8_t payload[9]={0}; put_le32(payload,DJI_ACTION4_DEVICE_ID); payload[4]=start?0:1;
  return dji_build_frame(0x01,seq,0x1D,0x03,payload,sizeof(payload),out,out_cap);
}
size_t dji_build_connection_request(uint16_t seq,uint32_t controller_device_id,const uint8_t mac[6],
                                    uint8_t verify_mode,uint16_t verify_code,uint8_t *out,size_t out_cap){
  uint8_t payload[33]={0}; put_le32(payload,controller_device_id); payload[4]=6;
  if (mac) {
    memcpy(payload + 5, mac, 6);
  }
  put_le32(payload + 21, 0);
  payload[25] = 0;
  payload[26] = verify_mode;
  put_le16(payload + 27, verify_code);
  return dji_build_frame(0x01,seq,0x00,0x19,payload,sizeof(payload),out,out_cap);
}
size_t dji_build_connection_response(uint16_t seq,uint32_t controller_device_id,uint32_t camera_number,uint8_t *out,size_t out_cap){
  uint8_t payload[9]={0}; put_le32(payload,controller_device_id); payload[4]=0; put_le32(payload+5,camera_number);
  return dji_build_frame(0x20,seq,0x00,0x19,payload,sizeof(payload),out,out_cap);
}
bool dji_parse_status_push(const uint8_t *frame,size_t len,DjiCameraStatus *out){
  DjiFrameView v; if(!out||!dji_parse_frame(frame,len,&v)) return false;
  if(v.cmd_set!=0x1D||v.cmd_id!=0x02||v.payload_len<38) return false;
  const uint8_t *p=v.payload; memset(out,0,sizeof(*out));
  out->camera_mode=p[0]; out->camera_status=p[1]; out->video_resolution=p[2]; out->fps_idx=p[3]; out->eis_mode=p[4];
  out->record_time_s=get_le16(p+5); out->remain_time_s=get_le32(p+23); out->power_mode=p[28]; out->temperature_state=p[30];
  out->battery_percent=p[37]; out->recording=(out->camera_status==0x03); return true;
}
