#include "esp8266NetConn.h"

void connectWiFi(const char *ssid, const char *password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
    }
    Serial.println();
    Serial.println(WiFi.localIP());
}

void disconnectWiFi() {
    WiFi.disconnect();
}