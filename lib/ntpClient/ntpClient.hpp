#ifndef _NTP_CLIENT_HPP_
#define _NTP_CLIENT_HPP_

#include <Arduino.h>
#include <NTPEventTypes.h>

typedef enum {
    TIME_SYNCED,
    TIME_SYNCING,
    ERROR,
    IDLE
} NTPClient_State_t;

void registerNTPEventHandler(void (*func)(NTPEvent_t event));
void addEHSerialPrintEvent(NTPEvent_t event);
void unregisterNTPEventHandler();

NTPClient_State_t getNTPClientState();

void startNTPClient();
void startNTPClient(uint16_t milli_ntp_timeout, int sync_period, const char * TZ, const char * ntp_server_nullable);
void waitNTPClientSync();
void stopNTPClient(bool reset);
void resetNTPClient();

int64_t NTPmillis();
int64_t NTPmicros();
int64_t NTPUnixTics();


#endif