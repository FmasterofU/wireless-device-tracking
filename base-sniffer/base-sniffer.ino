// ESP8266 Simple sniffer
// 2018 Carve Systems LLC
// Angel Suarez-B Martin

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "sdk_structs.h"
#include "ieee80211_structs.h"
#include "string_utils.h"

extern "C"
{
  #include "user_interface.h"
}
/*
// According to the SDK documentation, the packet type can be inferred from the
// size of the buffer. We are ignoring this information and parsing the type-subtype
// from the packet header itself. Still, this is here for reference.
wifi_promiscuous_pkt_type_t packet_type_parser(uint16_t len)
{
  	switch(len)
    {
      // If only rx_ctrl is returned, this is an unsupported packet
      case sizeof(wifi_pkt_rx_ctrl_t):
      return WIFI_PKT_MISC;

      // Management packet
      case sizeof(wifi_pkt_mgmt_t):
      return WIFI_PKT_MGMT;

      // Data packet
      default:
      return WIFI_PKT_DATA;
    }
}
*/
// In this example, the packet handler function does all the parsing and output work.
// This is NOT ideal.
void wifi_sniffer_packet_handler(uint8_t *buff, uint16_t len)
{
  // First layer: type cast the received buffer into our generic SDK structure
  const wifi_promiscuous_pkt_t *ppkt = (wifi_promiscuous_pkt_t *)buff;
  // Second layer: define pointer to where the actual 802.11 packet is within the structure
  const wifi_ieee80211_packet_t *ipkt = (wifi_ieee80211_packet_t *)ppkt->payload;
  // Third layer: define pointers to the 802.11 packet header and payload
  const wifi_ieee80211_mac_hdr_t *hdr = &ipkt->hdr;
  const uint8_t *data = ipkt->payload;

  // Pointer to the frame control section within the packet header
  const wifi_header_frame_control_t *frame_ctrl = (wifi_header_frame_control_t *)&hdr->frame_ctrl;

  // Parse MAC addresses contained in packet header into human-readable strings
  char addr1[] = "00:00:00:00:00:00\0";
  char addr2[] = "00:00:00:00:00:00\0";
  char addr3[] = "00:00:00:00:00:00\0";
  char addr4[] = "00:00:00:00:00:00\0";

  mac2str(hdr->addr1, addr1);
  mac2str(hdr->addr2, addr2);
  mac2str(hdr->addr3, addr3);
  mac2str(hdr->addr4, addr4);

  // Output info to serial
  //wifi_promiscuous_pkt_type_t type, wifi_mgmt_subtypes_t subtype
  if((wifi_promiscuous_pkt_type_t)frame_ctrl->type == WIFI_PKT_MGMT && (wifi_mgmt_subtypes_t)frame_ctrl->subtype == PROBE_REQ)
  //if(strcmp(addr1,"dc:f7:56:5d:ff:88\0")==0 || strcmp(addr2,"dc:f7:56:5d:ff:88\0")==0 || strcmp(addr3,"dc:f7:56:5d:ff:88\0")==0)
  //if(strcmp(addr1,"50:32:75:4d:2a:69\0")==0 || strcmp(addr2,"50:32:75:4d:2a:69\0")==0 || strcmp(addr3,"50:32:75:4d:2a:69\0")==0)
  //if(/*strcmp(addr1,"50:32:75:4d:2a:69\0")==0 || */strcmp(addr2,"40:4e:36:3a:45:1b\0")==0/* || strcmp(addr3,"50:32:75:4d:2a:69\0")==0*/)
  Serial.printf("\n%s | %s | %s | %s | %u | %02d | %u | %u(%-2u) | %-28s | %u | %u | %u | %u | %u | %u | %u | %u | ",
    addr1,
    addr2,
    addr3,
    addr4,
    wifi_get_channel(),
    ppkt->rx_ctrl.rssi,
    frame_ctrl->protocol,
    frame_ctrl->type,
    frame_ctrl->subtype,
    wifi_pkt_type2str((wifi_promiscuous_pkt_type_t)frame_ctrl->type, (wifi_mgmt_subtypes_t)frame_ctrl->subtype),
    frame_ctrl->to_ds,
    frame_ctrl->from_ds,
    frame_ctrl->more_frag,
    frame_ctrl->retry,
    frame_ctrl->pwr_mgmt,
    frame_ctrl->more_data,
    frame_ctrl->wep,
    frame_ctrl->strict);
/*
  // Print ESSID if beacon
  if (frame_ctrl->type == WIFI_PKT_MGMT && frame_ctrl->subtype == BEACON)
  {
    const wifi_mgmt_beacon_t *beacon_frame = (wifi_mgmt_beacon_t*) ipkt->payload;
    char ssid[32] = {0};

    if (beacon_frame->tag_length >= 32)
    {
      strncpy(ssid, beacon_frame->ssid, 31);
    }
    else
    {
      strncpy(ssid, beacon_frame->ssid, beacon_frame->tag_length);
    }

    Serial.printf("%s", ssid);
  }*/
}


long i = 0;
uint8_t ch = 1;

void setup()
{
  // Serial setup
  Serial.begin(115200);
  delay(10);
  wifi_set_channel(ch);

  // Wifi setup
  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();



  // Set sniffer callback
  wifi_promiscuous_enable(1);
  //uint8_t mac_address[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  //wifi_promiscuous_set_mac(mac_address);
  wifi_set_promiscuous_rx_cb(wifi_sniffer_packet_handler);
  wifi_set_channel(12);
  // Print header
  Serial.printf("\n\n     MAC Address 1|      MAC Address 2|      MAC Address 3| Ch| RSSI| Pr| T(S)  |           Frame type         |TDS|FDS| MF|RTR|PWR| MD|ENC|STR|   SSID");

}

void loop()
{
 delay(10);
  if(i++ % 6000 == 0){
    if(++ch == 15) ch-=14;
    wifi_set_channel(ch);
  Serial.printf("\nnow on channel %d\n",ch);
  }
}
