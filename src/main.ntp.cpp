#ifdef NTPTEST

#include "ntpClient.hpp"
#include "esp8266NetConn.hpp"

#include <Arduino.h>


#define YOUR_WIFI_SSID "Igor"
#define YOUR_WIFI_PASSWD ""

#define ONBOARDLED 2 // Built in LED on ESP-12/ESP-07
#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT 1500


void setup() {
    Serial.begin (115200);
    Serial.println ();
    clientSetupWiFi();
    clientConnectWiFi(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
    
    pinMode (ONBOARDLED, OUTPUT); // Onboard LED
    digitalWrite (ONBOARDLED, HIGH); // Switch off LED
    
    registerNTPEventHandler(addEHSerialPrintEvent);
    startNTPClient();
    waitNTPClientSync();
}

void loop() {
    static int i = 0;
    static int last = 0;
    static bool stopped = false;

    if ((millis () - last) > SHOW_TIME_PERIOD) {
        Serial.println("-----");
        Serial.println(millis());
        Serial.println(NTPmillis());
        last = millis ();
        i++;
    }

    if(i>12){
        Serial.println("-----");
        Serial.println(millis());
        if(!stopped) stopNTPClient(true);
        stopped = true;
        delay(3000);
        Serial.println("cekao");
    }
    if(stopped) delay(10);
    else delay (0);
}

#endif