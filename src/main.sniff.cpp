#ifdef SNIFF

#include <Arduino.h>
#include "probe_request_sniffer.hpp"

void macprint(const uint8_t* pmac, char* ret){
  sprintf(ret, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
}

void macprintbuff(uint8_t * pmac, char ** pout) {
  macprint(pmac, *pout);
  Serial.printf("|%s|", *pout);
  *pout += 19;
}

void probe_request_handler(struct probe_request * ppr) {
  char out[19*5+1] = {0};
  char * p = out;
  macprintbuff(ppr->receiver_address, &p);
  macprintbuff(ppr->destination_address, &p);
  macprintbuff(ppr->transmission_address, &p);
  macprintbuff(ppr->source_address, &p);
  macprintbuff(ppr->bssid_address, &p);
  Serial.printf("\n%d %03x", ppr->seq_ctrl.sequence_num, ppr->seq_ctrl.sequence_num);
  Serial.println();
}

long i = 0;
uint8_t ch = 1;
void sniffer_loop() {
  delay(10);
  if(i++ % 6000 == 0){
    if(++ch == 15) ch-=14;
    change_sniffer_channel(ch);
    Serial.printf("\nnow on channel %d\n",ch);
  }
}

void setup() {
  Serial.begin(115200);
  delay(10);
  sniffer_init(ch, probe_request_handler);
}

void loop() {
  sniffer_loop();
}

#endif