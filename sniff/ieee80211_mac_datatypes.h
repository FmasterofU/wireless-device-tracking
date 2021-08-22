#ifndef _IEEE80211_MAC_DATATYPES_H_
#define _IEEE80211_MAC_DATATYPES_H_

#include <stdint.h>
/*
struct frame_control_flags_t {

};

struct frame_control_t {

};*/

struct ieee80211_frame_t {  // stucturing not possible since the word size is 32 bits and it would drop pieces on casting from a byte array
  unsigned ctrl_version:2;
  unsigned ctrl_type:2;
  unsigned ctrl_subtype:4;
  unsigned ctrl_flag_ds_status:2; // distribution system
  unsigned ctrl_flag_last_fragment:1;
  unsigned ctrl_flag_retransmitted_frame:1;
  unsigned ctrl_flag_pwr_mgmt:1;
  unsigned ctrl_flag_buffered_data:1;
  unsigned ctrl_flag_data_protected:1;
  unsigned ctrl_flag_htc_ordered:1;
  int16_t duration_id;
  uint8_t payload[0]; // variable number of addresses depending od DS flag + data + end of frame
};

struct frame_sequence_control_t {
    unsigned fragment_num:4;
    unsigned sequence_num:12;
};

#endif