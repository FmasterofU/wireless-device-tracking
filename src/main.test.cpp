#ifdef TEST

#include <Arduino.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <TimeLib.h>
#include <NTPtimeESP.h>
#include "esp8266NetConn.hpp"
#include <DS3231.h>

#include <ESPNtpClient.h>

const char* ssid = "Igor";
const char* password = "";


boolean syncEventTriggered = false; // True if a time even has been triggered
NTPEvent_t ntpEvent; // Last triggered event
void processSyncEvent (NTPEvent_t ntpEvent) {
    Serial.println("unutra");
    switch (ntpEvent.event) {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.println("pred print");
            Serial.printf ("[NTP-event] %s\n", NTP.ntpEvent2str (ntpEvent));
            Serial.println("post print");
            break;
        default:
            break;
    }
}

#define NTP_TIMEOUT 1500

const PROGMEM char* ntpServer = "pool.ntp.org";

void setup()
{
    Serial.begin(115200); while (!Serial);
  Serial.println("COM setup successful.");
  delay(10);

  clientSetupWiFi();
  clientConnectWiFi(ssid, password);
  NTP.setTimeZone (TZ_Etc_UTC);
  NTP.setInterval (600);
    NTP.setNTPTimeout (NTP_TIMEOUT);
  NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
    });
    Serial.println("pre begin");
    NTP.begin (ntpServer);
Serial.println("post begin");
//Serial.println(NTP.getTimeDateStringUs());
Serial.println("pred while");
  while(!syncEventTriggered);
  
    Serial.println("post while");
  if (syncEventTriggered) {
        syncEventTriggered = false;
        Serial.println("procesujem");
        processSyncEvent (ntpEvent);
    }
    
    Serial.println("post if");
}

void loop() {
// nothing todo here
unsigned long a = 0;
a= millis();
delay(3000);
Serial.println(millis()-a);
}

#endif