#ifdef TESTSERIAL

#include <Arduino.h>

uint8_t data[8] = {'D' , 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x00};

void setup() {
    Serial.begin(115200);
    delay(5000);
}

void loop() {
    Serial.write(data, 7);
    delay(5454);
}

#endif