#ifndef _ESP8266_NET_CONN_H_
#define _ESP8266_NET_CONN_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>

void clientSetupWiFi();
void clientConnectWiFi(const char * ssid, const char * password);
void clientDisconnectWiFi();

#endif