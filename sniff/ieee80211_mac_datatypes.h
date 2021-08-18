#ifndef _IEEE80211_MAC_DATATYPES_H_
#define _IEEE80211_MAC_DATATYPES_H_

struct frame_control_flags_t {
    unsigned ds_status:2; // distribution system
    unsigned last_fragment:1;
    unsigned retransmitted_frame:1;
    unsigned pwr_mgmt:1;
    unsigned buffered_data:1;
    unsigned data_protected:1;
    unsigned htc_ordered:1;
};

struct frame_control_t {
    unsigned version:2;
    unsigned type:2;
    unsigned subtype:4;
    struct frame_control_flags_t flags;
};

struct ieee80211_frame_t {
    struct frame_control_t frame_ctrl;
    int16_t duration_id;
    uint8_t payload[0]; // variable number of addresses depending od DS flag + data + end of frame
};

struct frame_sequence_control_t {
    unsigned fragment_num:4;
    unsigned sequence_num:12;
}

#endif