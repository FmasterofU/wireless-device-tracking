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
                else if(ii == 14) break;
                else if(ii == 7 && rx_char=='S') {
                    srcnow = 1;
                    continue;
                }
                else if(ii==0 || ii==7) continue;
                iter = ii-1-srcnow;//(iter + ii - 1) % BUFFLEN;
                packet[PACKET_DEST_IND + iter] = rx_char;
                os_printf("iter %d SERIAL %02x = %c and datalen %d \n", iter, rx_char, rx_char, rx_len);
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

void light_switch() {
    static uint8_t state = 0;
	if (state) {
		GPIO_OUTPUT_SET(2, 1);
	} else {
		GPIO_OUTPUT_SET(2, 0);
	}
	state ^= 1;
}

//bool sender_available = true;

void sender(void *arg) {
    //if(!sender_available) return;
    //sender_available = false;
    const char * pmac = packet + PACKET_DEST_IND;
    const char * pmac1 = packet + PACKET_DEST_IND + 6;
    os_printf("Send status: %d on channel %d dest %02x:%02x:%02x:%02x:%02x:%02x and src %02x:%02x:%02x:%02x:%02x:%02x\n", wifi_send_pkt_freedom(packet, 25, 0), channel, pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5], pmac1[0], pmac1[1], pmac1[2], pmac1[3], pmac1[4], pmac1[5]);
    next_channel();
    light_switch();
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
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    wifi_set_opmode(STATION_MODE);
    wifi_promiscuous_enable(1); 
    // not available in sdk 1.3.0
    // wifi_register_send_pkt_freedom_cb(freedom_outside_cb);
    channel = 1;
    wifi_set_channel(channel);
    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)sender, NULL);
    os_timer_arm(&ptimer, 1250, 1);
}