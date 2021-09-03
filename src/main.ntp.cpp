#ifdef NTPTEST

#include <Arduino.h>

//#include "WifiConfig.h"
#include <ESPNtpClient.h>
#include <ESP8266WiFi.h>

#ifndef WIFI_CONFIG_H
#define YOUR_WIFI_SSID "Igor"
#define YOUR_WIFI_PASSWD ""
#endif // !WIFI_CONFIG_H

#define ONBOARDLED 2 // Built in LED on ESP-12/ESP-07
#define SHOW_TIME_PERIOD 1000
#define NTP_TIMEOUT 1500

const PROGMEM char* ntpServer = "pool.ntp.org";
bool wifiFirstConnected = false;

boolean syncEventTriggered = false; // True if a time even has been triggered
NTPEvent_t ntpEvent; // Last triggered event
double offset;
double timedelay;


void processSyncEvent (NTPEvent_t ntpEvent) {
    switch (ntpEvent.event) {
        case timeSyncd:
        case partlySync:
        case syncNotNeeded:
        case accuracyError:
            Serial.printf("[NTP-event] %s\n", NTP.ntpEvent2str(ntpEvent));
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin (115200);
    Serial.println ();
    WiFi.mode (WIFI_STA);
    WiFi.begin (YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);
    
    pinMode (ONBOARDLED, OUTPUT); // Onboard LED
    digitalWrite (ONBOARDLED, HIGH); // Switch off LED
    
    NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
        ntpEvent = event;
        syncEventTriggered = true;
    });
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if(Serial) Serial.print(".");
    }
    NTP.setTimeZone (TZ_Europe_Belgrade);
    NTP.setInterval (30);
    NTP.setNTPTimeout (NTP_TIMEOUT);
    // NTP.setMinSyncAccuracy (750);
    // NTP.settimeSyncThreshold (500);
    NTP.begin (ntpServer);
}

void loop() {
    static int i = 0;
    static int last = 0;
    static bool stopped = false;

    if (syncEventTriggered) {
        syncEventTriggered = false;
        processSyncEvent (ntpEvent);
    }

    if ((millis () - last) > SHOW_TIME_PERIOD) {
    Serial.println("-----");
    Serial.println(millis());
    Serial.println(NTP.millis());
        last = millis ();/*
        Serial.print (i); Serial.print (" ");
        Serial.print (NTP.getTimeDateStringUs ()); Serial.print (" ");
        Serial.print ("WiFi is ");
        Serial.print (WiFi.isConnected () ? "connected" : "not connected"); Serial.print (". ");
        Serial.print ("Uptime: ");
        Serial.print (NTP.getUptimeString ()); Serial.print (" since ");
        Serial.println (NTP.getTimeDateString (NTP.getFirstSyncUs ()));
        Serial.printf ("Free heap: %u\n", ESP.getFreeHeap ());*/
        i++;
    }

    if(i>12){
    Serial.println("-----");
    Serial.println(millis());
        if(!stopped) NTP.stop();
        stopped = true;
        delay(3000);
        Serial.println("cekao");
    }
    if(stopped) delay(10);
    else delay (0);
}

#endif