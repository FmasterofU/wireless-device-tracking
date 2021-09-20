#ifdef RTC

#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS3231.h>

#include "ntpClient.hpp"
#include "esp8266NetConn.hpp"


#define MAIN_LOOP_MILLI_INTERVAL 300000


void printTimeTrackersState(const char * ntp);
void printTimeTrackersState();

DS3231 Clock;
bool Century=false;
bool h12;
bool PM;

const char* ssid = "Igor";
const char* password = "";

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

bool ntp_pass = false;

void handler(NTPEvent_t event) {
  addEHSerialPrintEvent(event);
  if(event.event == 0 && ntp_pass) printTimeTrackersState("NTPTimeSynced");
}

void getAccurateTime() {
  registerNTPEventHandler(handler);
	startNTPClient(10000, 2*60*60);
	waitNTPClientSync();
  ntp_pass = true;
	setSyncProvider(NTPUnixTics);
	while (timeStatus() != timeSet);
	Serial.printf("\n%d/%d/%d %d:%d:%d\n", year(), month(), day(), hour(), minute(), second());
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

void printDS3231Time() {
  Serial.print((Clock.getYear()+2000), DEC);
  Serial.print('-');
  // then the month
  Serial.print(Clock.getMonth(Century), DEC);
  Serial.print('-');
  // then the date
  Serial.print(Clock.getDate(), DEC);
  Serial.print('-');
  // and the day of the week
  Serial.print(Clock.getDoW(), DEC);
  Serial.print(' ');
  // Finally the hour, minute, and second
  Serial.print(Clock.getHour(h12, PM), DEC);
  Serial.print("H ");
  Serial.print(Clock.getMinute(), DEC);
  Serial.print("M ");
  Serial.print(Clock.getSecond(), DEC);
  Serial.print("S ");
  // Add AM/PM indicator
  if (h12) {
    if (PM) {
      Serial.print(" PM ");
    } else {
      Serial.print(" AM ");
    }
  } else {
    Serial.print(" (24h) ");
  }
  // Display the temperature
  Serial.print("T=");
  Serial.print(Clock.getTemperature(), 2);
  // Tell whether the time is (likely to be) valid
  if (Clock.oscillatorCheck()) {
    Serial.print(" O+");
  } else {
    Serial.print(" O-");
  }
  Serial.print('\n');
}

void printTimeTrackersState(const char * ntp) {
  Serial.printf("%s--------\n%llu\n%llu\n%llu\n%llu\n", ntp, offeset + millis(), NTPmillis(), espRTCTime, getDS3231MilliTime());
  printDS3231Time();
}

void printTimeTrackersState(){
  printTimeTrackersState("");
}

void setup() {
  pinMode(D5, INPUT);
  // Start the I2C interface
  Wire.begin();
  Serial.begin(115200); while (!Serial);
  Serial.println("COM setup successful.");
  clientSetupWiFi();
  clientConnectWiFi(ssid, password);
  Clock.enable32kHz(false);
  Clock.enableOscillator(true, false, 1);
  attachInterrupt(digitalPinToInterrupt(D5), isr_func, RISING);
  getAccurateTime();
  setDS3231Time();
  //stopNTPClient(true);
  printTimeTrackersState();
}

void loop() {
  //static int i = 0;
  static unsigned long long last = 0;
  if((millis() - last) >= MAIN_LOOP_MILLI_INTERVAL) {
    last = millis();
    printTimeTrackersState();
    //if(++i == 6) changeSyncPeriod(60*60);
  }
  delay(0);
}

#endif