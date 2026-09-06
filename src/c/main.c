#include <pebble.h>
#include "dji_session.h"
#include "ble_port.h"

static Window *s_window;
static TextLayer *s_title,*s_status,*s_detail;

static bool send_ble(const uint8_t *data,size_t len){ return ble_port_write_fff5(data,len); }
static const char *session_text(void){
  switch(dji_session_state()){
    case DJI_SESSION_WAIT_CAMERA_ACK:return "CHECK CAMERA";
    case DJI_SESSION_WAIT_CAMERA_REQUEST:return "PAIRING...";
    case DJI_SESSION_CONNECTED:return dji_session_camera_status()->recording?"REC":"READY";
    case DJI_SESSION_REJECTED:return "REJECTED";
    case DJI_SESSION_ERROR:return "DJI ERROR";
    default:return "NOT PAIRED";
  }
}
static void refresh_ui(void){
  static char detail[160];
  const char *state="OFFLINE";
  if(ble_port_state()==BLE_PORT_SCANNING) state="SCANNING";
  else if(ble_port_state()==BLE_PORT_CONNECTING) state="CONNECTING";
  else if(ble_port_state()==BLE_PORT_CONNECTED) state="GATT...";
  else if(ble_port_state()==BLE_PORT_READY) state=session_text();
  else if(ble_port_state()==BLE_PORT_ERROR) state="BLE API NEEDED";
  text_layer_set_text(s_status,state);
  const DjiCameraStatus *cam=dji_session_camera_status();
  uint16_t code=dji_session_verify_code();
  if(dji_session_state()==DJI_SESSION_WAIT_CAMERA_ACK||dji_session_state()==DJI_SESSION_WAIT_CAMERA_REQUEST){
    snprintf(detail,sizeof(detail),"Verify %04u\nConfirm same code\non Action 4\n\nSELECT: REC after READY",(unsigned)code);
  } else if(dji_session_state()==DJI_SESSION_CONNECTED){
    snprintf(detail,sizeof(detail),"Batt %u%%\nREC %02u:%02u\n\nSELECT: REC\nUP: reconnect\nDOWN: future zoom",
             (unsigned)cam->battery_percent,(unsigned)(cam->record_time_s/60),(unsigned)(cam->record_time_s%60));
  } else {
    snprintf(detail,sizeof(detail),"UP: connect\nSELECT: REC\nDOWN: future zoom\n\nPoC 0.4");
  }
  text_layer_set_text(s_detail,detail);
}
static void state_changed(BlePortState state){
  if(state==BLE_PORT_READY){
    uint8_t mac[6]; uint16_t max_len=ble_port_max_write_len();
    if(!ble_port_get_local_identity_mac(mac)||!dji_session_start_first_pairing(mac,max_len)){ refresh_ui(); return; }
  }
  refresh_ui();
}
static void rx_data(const uint8_t *data,size_t len){ dji_session_on_frame(data,len); refresh_ui(); }
static void up_click(ClickRecognizerRef r,void *c){(void)r;(void)c;ble_port_disconnect();ble_port_start_scan();refresh_ui();}
static void select_click(ClickRecognizerRef r,void *c){(void)r;(void)c;if(!dji_session_send_record_toggle())text_layer_set_text(s_status,"NOT READY");}
static void down_click(ClickRecognizerRef r,void *c){(void)r;(void)c;text_layer_set_text(s_status,"ZOOM: WAIT DJI");}
static void click_config(void *ctx){(void)ctx;window_single_click_subscribe(BUTTON_ID_UP,up_click);window_single_click_subscribe(BUTTON_ID_SELECT,select_click);window_single_click_subscribe(BUTTON_ID_DOWN,down_click);}
static void window_load(Window *window){
  Layer *root=window_get_root_layer(window); GRect b=layer_get_bounds(root);
  s_title=text_layer_create(GRect(8,8,b.size.w-16,32)); text_layer_set_font(s_title,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_title,GTextAlignmentCenter); text_layer_set_text(s_title,"DJI REMOTE"); layer_add_child(root,text_layer_get_layer(s_title));
  s_status=text_layer_create(GRect(8,48,b.size.w-16,36)); text_layer_set_font(s_status,fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_status,GTextAlignmentCenter); layer_add_child(root,text_layer_get_layer(s_status));
  s_detail=text_layer_create(GRect(10,92,b.size.w-20,b.size.h-100)); text_layer_set_font(s_detail,fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_detail,GTextAlignmentCenter); layer_add_child(root,text_layer_get_layer(s_detail)); refresh_ui();
}
static void window_unload(Window *window){(void)window;text_layer_destroy(s_title);text_layer_destroy(s_status);text_layer_destroy(s_detail);}
static void init(void){ dji_session_init(send_ble); ble_port_init(state_changed,rx_data); s_window=window_create();
  window_set_window_handlers(s_window,(WindowHandlers){.load=window_load,.unload=window_unload});
  window_set_click_config_provider(s_window,click_config); window_stack_push(s_window,true);
}
static void deinit(void){ble_port_disconnect();window_destroy(s_window);}
int main(void){init();app_event_loop();deinit();}
