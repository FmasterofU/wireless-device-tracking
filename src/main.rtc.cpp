#ifdef RTC

// Required Libraries
// ------------------
// DS3231 (Library by Eric Ayars v1.0.2)
// Time (Library Manager v1.5)
// Wire (Arduino Standard Library v1.0)
// NTPtimeESP (https://github.com/SensorsIot/NTPtimeESP)
// ESP8266WiFi (v1.0)

#include <Arduino.h>
#include <TimeLib.h>
#include <NTPtimeESP.h>
#include <ESP8266WiFi.h>
//#include "esp8266NetConn/esp826NetConn.h"
#include <WiFiClient.h>
#include <DS3231.h>
#include <Wire.h>

DS3231 Clock;

const char* ssid = "Martin Ruter King";
const char* password = "";

//**** NTP Server to use
const char* NTP_Server = "ntp1.inrim.it"; //time.google.com

//**** Your time zone UTC related (floating point number)
#define TIME_ZONE 1.0f

NTPtime NTPch(NTP_Server);   
strDateTime dateTime;

//****************************************************
//* SYNCRONIZE SYSTEM TIME with NTP SERVERS
//****************************************************
#define SEND_INTV     10
#define RECV_TIMEOUT  10

// --------------------------------------
// epochUnixNTP set the UNIX time
// number of seconds sice Jan 1 1970
// --------------------------------------

time_t epochUnixNTP()
{
    Serial.println(">>>> TimeSync function called <<<<");

//**** BIG ISSUE: in case of poor connection, we risk to remain in this loop forever
    NTPch.setSendInterval(SEND_INTV);
    NTPch.setRecvTimeout(RECV_TIMEOUT);
    do
    {
      dateTime = NTPch.getNTPtime(TIME_ZONE, 1);
      delay(1);
    }
    while(!dateTime.valid);
    Serial.print("DS3231 time: ");
  PrintDS3231Time();
    Clock.setSecond(dateTime.second);
    Clock.setMinute(dateTime.minute);
    Clock.setClockMode(false);  // set to 24h
    //setClockMode(true); // set to 12h
    Clock.setHour(dateTime.hour);
    Clock.setYear(dateTime.year-2000);
    Clock.setMonth(dateTime.month);
    Clock.setDate(dateTime.day);
    Clock.setDoW(dateTime.dayofWeek);

    Serial.println(">>>> DS3231 sync'ed with NTP time <<<<");
    
    Serial.print("NTP time   : ");
    NTPch.printDateTime(dateTime);

    //set system time
    setTime(dateTime.hour,dateTime.minute,dateTime.second,dateTime.day,dateTime.month,dateTime.year); 

  return 0;
}

//***********************************************************************
//*  SSIDCONNECT
//***********************************************************************

void ssidConnect()
{
  Serial.println(ssid);
  Serial.println(password);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(700);
    Serial.print(".");
  }
     
  Serial.println();
  Serial.print(F("Connected to "));
  Serial.println(ssid);
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
}

//***********************************************************************
//*  SETUP
//***********************************************************************
void setup()
{
  bool i2c_found;
  
  Serial.begin(115200); while (!Serial);
  Serial.println("COM setup successful.");
  delay(10);

  WiFi.mode(WIFI_STA);
  // connect to WiFi network
  ssidConnect();

  //connectWiFi(ssid, password);
  // Start the I2C interface
  Wire.begin();
  
  // Set time sync provider
  setSyncProvider(epochUnixNTP);  //set function to call when sync required
  setSyncInterval(60);            //every 60 seconds
                                  //In the same moment you retrieve the time from the library, that function will look up whether it is time to sync the time.
                                  //If you don't retrieve the time or if you do, but the syninterval has not yet passed, nothing will happen.

  // display the time
  Serial.print("DS3231 time: ");
  PrintDS3231Time();
  
}

//***********************************************************************
//*  Print DS3231 time
//***********************************************************************
bool Century=false;
bool h12;
bool PM;

void PrintDS3231Time(){
  
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
  Serial.print('\n');
  
}

//***********************************************************************
//*  LOOP
//***********************************************************************
void loop() {
// nothing todo here
delay(100000);
epochUnixNTP();
}

#endif