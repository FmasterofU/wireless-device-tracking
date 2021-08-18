#include "probe_request_sniffer.hpp"




void probe_request_filter(const promiscuous_mode_pkt * ppkt, uint16_t len, struct probe_request * ppr) {
  // Filter invalid packets - Base lenght check
  if(len <= sizeof(struct promiscuous_mode_pkt) + sizeof(struct ieee80211_frame_t) + 2*2 /* sniffer_buf2 cnt and len */ + 3*6 /* three addresses*/ + 2 /* fragment/sequence */ + 2 /* FCS or CRC*/) return;
  
  const struct ieee80211_frame_t * mac_frame = (struct ieee80211_frame_t *) ppkt->payload;

  memcpy(ppr->seq_ctrl, mac_frame->payload + 3*6, 2)
  switch (mac_frame->frame_ctrl.flags.ds_status) {
  case 00b: // first bit (from left) is TO, second FROM
      memcpy(ppr->destination_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->source_address, mac_frame->payload + 6, 6) //addr2
      memcpy(ppr->receiver_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->transmission_address, mac_frame->payload + 6, 6) //addr2
      memcpy(ppr->bssid_address, mac_frame->payload + 2*6, 6) //addr3
      break;
  case 01b:
      memcpy(ppr->destination_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->source_address, mac_frame->payload + 2*6, 6) //addr3
      memcpy(ppr->receiver_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->transmission_address, mac_frame->payload + 6, 6) //addr2
      memcpy(ppr->bssid_address, mac_frame->payload + 6, 6) //addr2
      break;
  case 10b:
      memcpy(ppr->destination_address, mac_frame->payload + 2*6, 6) //addr3
      memcpy(ppr->source_address, mac_frame->payload + 6, 6) //addr2
      memcpy(ppr->receiver_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->transmission_address, mac_frame->payload + 6, 6) //addr2
      memcpy(ppr->bssid_address, mac_frame->payload, 6) //addr1
      break;
  case 11b:
      memcpy(ppr->destination_address, mac_frame->payload + 2*6, 6) //addr3
      memcpy(ppr->source_address, mac_frame->payload + 3*6 + 2, 6) //addr4 (3*addr + sc)
      memcpy(ppr->receiver_address, mac_frame->payload, 6) //addr1
      memcpy(ppr->transmission_address, mac_frame->payload + 6, 6) //addr2
      break;
}

void sniffer_handler_func(uint8_t *buff, uint16_t len) {
  // Filter unreliable packets (HT40, LDPC)
  if(len <= sizeof(struct RxControl)) return;

  const struct promiscuous_mode_pkt * ppkt = (struct promiscuous_mode_pkt *) buff;
  struct probe_request pr;
  probe_request_filter(ppkt, len, &pr);
}

long i = 0;
uint8_t ch = 1;

void sniffer_init() {
  wifi_set_channel(ch);

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_station_disconnect();
  //WiFi.disconnect();

  wifi_promiscuous_enable(1);
  uint8_t mac_addr[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  wifi_promiscuous_set_mac(mac_addr);
  wifi_set_promiscuous_rx_cb(sniffer_handler_func);
  wifi_set_channel(ch);
}

void sniffer_loop() {
  delay(10);
  if(i++ % 6000 == 0){
    if(++ch == 15) ch-=14;
    wifi_set_channel(ch);
  Serial.printf("\nnow on channel %d\n",ch);
  }
}