#include "probe_request_sniffer.hpp"

#include <Arduino.h>
void (*sniffer_pr_cb)(struct probe_request *) = nullptr;

void print_mac_frame(const uint8_t * payload, uint16_t len){
  Serial.println();
  for(int j = 0; j<=len-sizeof(struct RxControl); j++){
    Serial.printf("%02x:", payload[j]);
  }
  Serial.println();
}

bool probe_request_filter(const promiscuous_mode_pkt * ppkt, uint16_t len, struct probe_request * ppr) {
  // Filter invalid packets - Base lenght check
  if(len <= sizeof(struct promiscuous_mode_pkt) + sizeof(struct ieee80211_frame_t) + 2*2 /* sniffer_buf2 cnt and len */ + 3*6 /* three addresses*/ + 2 /* fragment/sequence */ + 2 /* FCS or CRC*/) return false;
  
  const struct ieee80211_frame_t * mac_frame = (struct ieee80211_frame_t *) ppkt->payload;

  // Filter non-probe requests / Probe request have type 0b00 and subtype 0b0100
  if(mac_frame->frame_ctrl.type != 0b00 || mac_frame->frame_ctrl.subtype != 0b0100) return false;

  print_mac_frame(ppkt->payload,len);
  Serial.println(mac_frame->frame_ctrl.flags.ds_status);

  memcpy(&(ppr->seq_ctrl), mac_frame->payload + 3*6, 2);
  switch (mac_frame->frame_ctrl.flags.ds_status) {
  case 0b00: // first bit (from left) is TO, second FROM
    memcpy(ppr->destination_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->source_address, mac_frame->payload + 6, 6); //addr2
    memcpy(ppr->receiver_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->transmission_address, mac_frame->payload + 6, 6); //addr2
    memcpy(ppr->bssid_address, mac_frame->payload + 2*6, 6); //addr3
    break;
  case 0b01:
    memcpy(ppr->destination_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->source_address, mac_frame->payload + 2*6, 6); //addr3
    memcpy(ppr->receiver_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->transmission_address, mac_frame->payload + 6, 6); //addr2
    memcpy(ppr->bssid_address, mac_frame->payload + 6, 6); //addr2
    break;
  case 0b10:
    memcpy(ppr->destination_address, mac_frame->payload + 2*6, 6); //addr3
    memcpy(ppr->source_address, mac_frame->payload + 6, 6); //addr2
    memcpy(ppr->receiver_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->transmission_address, mac_frame->payload + 6, 6); //addr2
    memcpy(ppr->bssid_address, mac_frame->payload, 6); //addr1
    break;
  case 0b11:
    memcpy(ppr->destination_address, mac_frame->payload + 2*6, 6); //addr3
    memcpy(ppr->source_address, mac_frame->payload + 3*6 + 2, 6); //addr4 (3*addr + sc)
    memcpy(ppr->receiver_address, mac_frame->payload, 6); //addr1
    memcpy(ppr->transmission_address, mac_frame->payload + 6, 6); //addr2
    break;
  }
  return true;
}

void sniffer_handler_func(uint8_t *buff, uint16_t len) {
  // Filter unreliable packets (HT40, LDPC)
  if(len <= sizeof(struct RxControl)) return;
  
  const struct promiscuous_mode_pkt * ppkt = (struct promiscuous_mode_pkt *) buff;
  struct probe_request pr;
  bool success = probe_request_filter(ppkt, len, &pr);
  if(success && sniffer_pr_cb != nullptr) sniffer_pr_cb(&pr);
}

void change_sniffer_channel(uint8_t channel) {
  wifi_set_channel(channel);
}

void sniffer_init(uint8_t channel, void (*probe_request_cb)(struct probe_request *), uint8_t mac_addr[]) {
  
  sniffer_pr_cb = probe_request_cb;
  
  wifi_set_channel(channel);

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  wifi_station_disconnect();
  //WiFi.disconnect();

  wifi_promiscuous_enable(1);
  wifi_promiscuous_set_mac(mac_addr);
  wifi_set_promiscuous_rx_cb(sniffer_handler_func);
  wifi_set_channel(channel);
}

void sniffer_init(uint8_t channel, void (*probe_request_cb)(struct probe_request *)){
  uint8_t mac_addr[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
  sniffer_init(channel, probe_request_cb, mac_addr);
}