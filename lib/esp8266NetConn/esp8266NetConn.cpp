#include "esp8266NetConn.hpp"


void clientSetupWiFi(){
    WiFi.mode(WIFI_STA);
    if(Serial) {
        Serial.println("WiFi STA mode enabled.");
    }
}

void clientConnectWiFi(const char *ssid, const char *password){
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        if(Serial) Serial.print(".");
    }
    if(Serial){
        Serial.println();
        Serial.println(WiFi.localIP());
    }
}

void clientDisconnectWiFi() {
    WiFi.disconnect();
    if(Serial) {
        Serial.println("Disconnected.");
    }
}