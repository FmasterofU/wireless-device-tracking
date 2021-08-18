#include "probe_request_sniffer.hpp"


void setup() {
  Serial.begin(115200);
  delay(10);

  sniffer_init()
}

void loop() {
  sniffer_loop();
}