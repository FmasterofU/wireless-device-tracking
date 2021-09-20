#define MAIN
#ifdef MAIN

#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS3231.h>
#include <SPI.h>
#include <SdFat.h>

#include "cfg.hpp"
#include "ntpClient.hpp"
#include "esp8266NetConn.hpp"
#include "probe_request_sniffer.hpp"


DS3231 Clock;
bool Century=false;
bool h12;
bool PM;
bool clockPresent = true;

SdFat SD;
char filename[30] = {0};
SdFatLibFile thisFile;
bool sdCardPresent = true;
volatile bool writingAllowed = false;

uint8_t channel = 1;

unsigned int rts_commands_num = 2;
uint8_t rtsBuff[28] = {'D' , 0x40, 0x4e, 0x36, 0x3a, 0x45, 0x1b, 'S', 0x00, 0x11, 0x22, 0x33, 0x44, 0x55,
                        'D' , 0xc6, 0xa4, 0x1a, 0xc6, 0x48, 0x23, 'S', 0x00, 0x11, 0x22, 0x33, 0x44, 0x55};


uint16_t int_freq = 1024;
double milli_period = double(int_freq) / 1000;
uint16_t floor_freq = floor(milli_period);
uint16_t ceil_freq = ceil(milli_period);
uint16_t millisecond_breaker = 33;//int_freq / 1000;
uint64_t offeset = 0;
volatile uint64_t espRTCTime = 0;
volatile uint16_t milliseconds = 0;
volatile unsigned long long counter = 0;
volatile unsigned long long millisecond_isr_counter = 0;
volatile unsigned long long milli_isr_total = 0;
void IRAM_ATTR isr_func() {
    ++counter;
    ++millisecond_isr_counter;
    if(millisecond_isr_counter == ceil_freq || (millisecond_isr_counter == floor_freq && (double(counter) - milli_period * (milli_isr_total + 1)) >= 0.0)) {
        millisecond_isr_counter = 0;
        ++milli_isr_total;
        ++espRTCTime;
        if(++milliseconds == 1000) milliseconds = 0;
    }
}

void busy_wait(uint16_t wait_milliseconds) {
    int unsigned long last = millis();
	while((millis() - last) < wait_milliseconds) yield();
} 


void setDS3231Time() {
    Clock.setYear(tmYearToY2k(CalendarYrToTm(year())));
    Clock.setMonth(month());
    Clock.setDate(day());
    Clock.setDoW(weekday());
    Clock.setClockMode(false);  // set to 24h
    //setClockMode(true); // set to 12h
    Clock.setHour(hour());
    Clock.setMinute(minute());
    Clock.setSecond(second());
    milli_isr_total = 0;
    millisecond_isr_counter = 0;
    counter = 0;
    milliseconds = NTPmillis() % 1000;
    espRTCTime = NTPmillis();
    offeset = NTPmillis() - millis();
}

int64_t getDS3231UnixTime() {
    tmElements_t tm;
    tm.Year = y2kYearToTm(Clock.getYear());
    tm.Month = Clock.getMonth(Century);
    tm.Day = Clock.getDate();
    tm.Wday = Clock.getDoW();
    tm.Hour = Clock.getHour(h12, PM);
    tm.Minute = Clock.getMinute();
    tm.Second = Clock.getSecond();
    return makeTime(tm);
}

uint64_t getDS3231MilliTime() {
    return getDS3231UnixTime() * 1000 + milliseconds;
}

void openFile() {
    if(sdCardPresent && !writingAllowed) {
        thisFile = SD.open(filename, FILE_WRITE);
        if(thisFile) writingAllowed = true;
    }
}

void closeFile() {
    if(sdCardPresent && thisFile && writingAllowed) {
        writingAllowed = false;
        busy_wait(15);
        thisFile.close();
    }
}

void logln(const char * line) {
    Serial.println(line);
    if(!sdCardPresent) return;
    while(!writingAllowed) busy_wait(10);
    thisFile.println(line);
}

uint64_t getPrefferedTime(){
    if(clockPresent && MILLISECOND_PRECISION) {
        return espRTCTime;
    } else if(clockPresent) {
        return getDS3231UnixTime() * 1000;
    } else {
        return NTPmillis();
    }
}

void getPrefferedTimeAsCString(char * ret) {
    sprintf(ret, "%lld", getPrefferedTime());
}

void logTime() {
    char temp[30] = {0};
    getPrefferedTimeAsCString(temp);
    logln(temp);
}

void macprint(const uint8_t* pmac, char* ret){
    sprintf(ret, "%02x:%02x:%02x:%02x:%02x:%02x", pmac[0], pmac[1], pmac[2], pmac[3], pmac[4], pmac[5]);
}

void probe_request_handler(struct probe_request * ppr) {
    char temp[256] = {0};
    char recv[20] = {0};
    char dest[20] = {0};
    char trans[20] = {0};
    char src[20] = {0};
    char bssid[20] = {0};
    macprint(ppr->receiver_address, recv);
    macprint(ppr->destination_address, dest);
    macprint(ppr->transmission_address, trans);
    macprint(ppr->source_address, src);
    macprint(ppr->bssid_address, bssid);
    char timestr[30] = {0};
    getPrefferedTimeAsCString(timestr);
    sprintf(temp, "PR timestamp %s channel %d |recv->%s|dest->%s|trans->%s|src->%s|bssid->%s|", timestr, channel, recv, dest, trans, src, bssid);
    logln(temp);
}

void generic_timer(unsigned long * last, unsigned long current, unsigned long desired_interval, void (*handler)()) {
    if((current - *last) < desired_interval) return;
    *last = current;
    handler();
}

void hop_channel() {
    channel = channel % 13 + 1;
    change_sniffer_channel(channel);
}

void channel_hopping() {
    static unsigned long last = millis();
    generic_timer(&last, millis(), CHANNEL_SWITCH_INTERVAL, hop_channel);
}

void file_close_open() {
    closeFile();
    busy_wait(5);
    openFile();
    if(sdCardPresent) logln("data saved to sd card");
}

void sd_file_saving() {
    static unsigned long last = millis();
    generic_timer(&last, millis(), FILE_SAVE_INTERVAL, file_close_open);
}

void rts_switch_command() {
    static int i = 0;
    char tempdest[30] = {0};
    char tempsrc[30] = {0};
    macprint(rtsBuff + 1 + 14 * i, tempdest);
    macprint(rtsBuff + 1 + 14 * i + 7, tempsrc);
    char log[128] = {0};
    sprintf(log, "sending serial rts command: dest %s and src %s", tempdest, tempsrc);
    logln(log);
    Serial.write(rtsBuff + 14 * i, 14);
    logln("rts command sent");
    i = ++i < rts_commands_num ? i : 0;
}

void rts_switching() {
    static unsigned long last = millis();
    generic_timer(&last, millis(), RTS_COMMAND_SWITCHING_INTERVAL, rts_switch_command);
}

void led_blinker() {
    static uint8_t next_state = HIGH;
    digitalWrite(ESP8266_ONBOARD_LED, next_state);
    next_state = next_state == LOW ? HIGH : LOW;
}

void loop_led_blinking() {
    static unsigned long last = millis();
    generic_timer(&last, millis(), INTERNAL_LED_BLINK_INTERVAL, led_blinker);
}

void setup() {
    // setting up pin modes
    pinMode(SD_CS_PIN, OUTPUT);
    pinMode(SQW_INTERRUPT_PIN, INPUT_PULLUP);
    pinMode(ESP8266_ONBOARD_LED, OUTPUT);
    digitalWrite(ESP8266_ONBOARD_LED, LOW);
    digitalWrite(SD_CS_PIN, HIGH);

    // setting up serial comms
    Serial.begin(115200); while (!Serial);

    // connecting to wifi
    clientSetupWiFi();
	clientConnectWiFi(WIFI_SSID, WIFI_PASSWD);
    digitalWrite(SD_CS_PIN, LOW);

    // setup I2C interface
    Wire.begin();
    //Clock.enable32kHz(true);
    if(!Clock.oscillatorCheck()) clockPresent = false;

    // interfacing with RTC oscillators
    if(clockPresent) {
        Clock.enable32kHz(false);
        Clock.enableOscillator(true, false, 1);
    }

    // synchronising time
    registerNTPEventHandler(addEHSerialPrintEvent);
    startNTPClient();
	waitNTPClientSync();

    // setting internal timelib
	setSyncProvider(NTPUnixTics);
	while (timeStatus() != timeSet);

    // setting RTC
    if(clockPresent) {
        attachInterrupt(digitalPinToInterrupt(SQW_INTERRUPT_PIN), isr_func, RISING);
        setDS3231Time();
    }

    // stopping ntp client's resync jobs
    stopNTPClient(false);

    // disconnecting from wifi 
    clientDisconnectWiFi();

    // SPI interface to SD card
    sdCardPresent = SD.begin(SD_CS_PIN, SPI_HALF_SPEED);

    // open file and prepare for writing
    sprintf(filename, "%llu.txt", getPrefferedTime());
    openFile();

    // start logging
    logTime();
    rts_switch_command();
    sniffer_init(channel, probe_request_handler);
}

void loop() {
    loop_led_blinking();
    channel_hopping();
    sd_file_saving();
    rts_switching();
    yield();
}

#endif