#ifndef _PROBE_REQUEST_SNIFFER_HPP_
#define _PROBE_REQUEST_SNIFFER_HPP_

#include "esp8266_nonos_sdk_datatypes.h"
#include "ieee80211_mac_datatypes.h"

extern "C"{
  #include "user_interface.h"
}

struct promiscuous_mode_pkt {
  struct RxControl rx_ctrl;
  uint8_t payload[0];
};

struct probe_request {
    uint8_t source_address[6];
    uint8_t transmission_address[6];
    uint8_t destination_address[6];
    uint8_t receiver_address[6];
    uint8_t bssid_address[6];
    struct frame_sequence_control_t seq_ctrl;
}

void sniffer_init();
void sniffer_loop();

#endif