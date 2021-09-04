#ifdef NTPTEST

#include <Arduino.h>
#include <TimeLib.h>

#include "ntpClient.hpp"
#include "esp8266NetConn.hpp"


#define YOUR_WIFI_SSID "Igor"
#define YOUR_WIFI_PASSWD ""
#define SHOW_TIME_PERIOD 1000

void setup() {
	Serial.begin(115200);
	Serial.println();
	clientSetupWiFi();
	clientConnectWiFi(YOUR_WIFI_SSID, YOUR_WIFI_PASSWD);

	registerNTPEventHandler(addEHSerialPrintEvent);
	startNTPClient();
	waitNTPClientSync();
	setSyncProvider(NTPUnixTics);
	while (timeStatus() != timeSet);
	Serial.printf("\n%d/%d/%d %d:%d:%d\n", year(), month(), day(), hour(), minute(), second());
}

void loop() {
	static int i = 0;
	static int last = 0;
	static bool stopped = false;

	if ((millis() - last) > SHOW_TIME_PERIOD) {
		Serial.println("-----");
		Serial.println(millis());
		Serial.println(NTPmillis());
        Serial.printf("%d/%d/%d %d:%d:%d\n", year(), month(), day(), hour(), minute(), second());
		last = millis();
		i++;
	}

	if (i > 12) {
		Serial.println("-----");
		Serial.println(millis());
		if (!stopped) stopNTPClient(true);
		stopped = true;
		delay(3000);
		Serial.println("cekao");
	}
	if (stopped) delay(10);
	else delay(0);
}

#endif