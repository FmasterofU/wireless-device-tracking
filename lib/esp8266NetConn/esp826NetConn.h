#ifndef _ESP8266_NET_CONN_H_
#define _ESP8266_NET_CONN_H_

#include <ESP8266WiFi.h>

void connectWiFi(const char * ssid, const char * password);
void disconnectWiFi();

#endif