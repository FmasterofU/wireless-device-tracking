#include "esp8266_nonos_sdk_datatypes.h"
#include "ieee80211_mac_datatypes.h"

extern "C"
{
  #include "user_interface.h"
}


struct promiscuous_mode_pkt {
  struct RxControl rx_ctrl;
  uint8_t payload[0];
};

void probe_request_filter(const promiscuous_mode_pkt * ppkt, uint16_t len) {
  // Base lenght check
  if(len <= sizeof(struct promiscuous_mode_pkt) + sizeof(struct ieee80211_frame_t) + 2*2 /* sniffer_buf2 cnt and len */ + 2*6 /* two addresses*/ + 2 /* fragment/sequence */ + 2 /* FCS or CRC*/) return;
  
  const struct ieee80211_frame_t * mac_frame = (struct ieee80211_frame_t *) ppkt->payload;

}

void sniffer_handler_func(uint8_t *buff, uint16_t len) {
  // Filter unreliable packets (HT40, LDPC)
  if(len <= sizeof(struct RxControl)) return;

  const struct promiscuous_mode_pkt * ppkt = (struct promiscuous_mode_pkt *) buff;
  probe_request_filter(ppkt, len);
}


long i = 0;
uint8_t ch = 1;

void setup() {
  Serial.begin(115200);
  delay(10);
  wifi_set_channel(ch);

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_station_disconnect();
  //WiFi.disconnect();

  wifi_promiscuous_enable(1);
  uint8_t mac_addr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  wifi_promiscuous_set_mac(mac_addr);
  wifi_set_promiscuous_rx_cb(sniffer_handler_func);
  wifi_set_channel(ch);
}

void loop() {
  delay(10);
  if(i++ % 6000 == 0){
    if(++ch == 15) ch-=14;
    wifi_set_channel(ch);
  Serial.printf("\nnow on channel %d\n",ch);
  }
}