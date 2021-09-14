#ifdef TESTSERIAL

#include <Arduino.h>

char * data = "DaTa\0";

void setup() {
    Serial.begin(115200);
    delay(5000);
}

void loop() {
    Serial.write(data, 5);
    delay(5454);
}

#endif