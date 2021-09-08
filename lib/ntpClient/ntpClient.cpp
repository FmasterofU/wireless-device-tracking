#include "ntpClient.hpp"
#include <ESPNtpClient.h>


#define SYNCING_PRINT_INTERVAL 1000

const PROGMEM char* ntpServer = "192.168.1.65";//"pool.ntp.org";
NTPEvent_t ntpEvent; // Last triggered event
NTPClient_State_t ntpClientState = IDLE;
bool NTPClientRunning = false;

void (*additionalEventHandler)(NTPEvent_t event) = nullptr;

void addEHSerialPrintEvent(NTPEvent_t event) {
    if(Serial) {
    Serial.println();
    Serial.printf("NTPClient State: %d;  [NTP-event] %s\n", getNTPClientState(), NTP.ntpEvent2str(event));
    }
}

void registerNTPEventHandler(void (*func)(NTPEvent_t event)) {
    additionalEventHandler = func;
}

void unregisterNTPEventHandler() {
    additionalEventHandler = nullptr;
}

NTPClient_State_t getNTPClientState() {
    return ntpClientState;
}

void processSyncEvent(NTPEvent_t ntpEvent) {
    switch (ntpEvent.event) {
        case timeSyncd:
        case syncNotNeeded:
            ntpClientState = TIME_SYNCED;
            break;
        case requestSent:
        case partlySync:
            ntpClientState = TIME_SYNCING;
            break;
        default:
            ntpClientState = ERROR;
            break;
    }
    if(additionalEventHandler != nullptr) additionalEventHandler(ntpEvent);
}

void startNTPClient(){
    startNTPClient(20000, INT_MAX, TZ_Etc_UTC, nullptr);
}

void startNTPClient(uint16_t milli_ntp_timeout, int sync_period) {
    startNTPClient(milli_ntp_timeout, sync_period, TZ_Etc_UTC, nullptr);
}

void startNTPClient(uint16_t milli_ntp_timeout, int sync_period, const char * TZ, const char * ntp_server_nullable) {
    NTP.onNTPSyncEvent ([] (NTPEvent_t event) {
        ntpEvent = event;
        processSyncEvent(event);
    });
    NTP.setTimeZone(TZ);
    NTP.setInterval(sync_period);
    NTP.setNTPTimeout(milli_ntp_timeout);
    //NTP.setMinSyncAccuracy(750);
    //NTP.settimeSyncThreshold(500);
    if(ntp_server_nullable == nullptr) NTP.begin(ntpServer);
    else NTP.begin(ntp_server_nullable);
    NTPClientRunning = true;
}

void waitNTPClientSync() {
    unsigned long time = millis();
    if(Serial) Serial.println("NTP Client Sync in progress.");
    while(getNTPClientState() != TIME_SYNCED) {
        if(Serial && millis() - time > SYNCING_PRINT_INTERVAL) {
            Serial.printf(".");
            time = millis();
        }
    }
}

void stopNTPClient(bool reset) {
    NTP.stop();
    NTPClientRunning = false;
    if(reset) resetNTPClient();
}

void resetNTPClient() {
    if(NTPClientRunning) {
        NTP.stop();
        NTPClientRunning = false;
    }
    ntpClientState = IDLE;
    unregisterNTPEventHandler();
}

int64_t NTPmillis() {
    return NTP.millis();
}

int64_t NTPmicros() {
    return NTP.micros();
}

int64_t NTPUnixTics() {
    return NTP.millis() / 1000;
}
