#ifdef TFCARD
#define FS_NO_GLOBALS // not used since it does not help in main project
#include <Arduino.h>
#include <SPI.h>
//#include <SD.h>
#include <FS.h>
#include "SdFat.h"
SdFat SD;

#define SD_CS_PIN D0
SdFatLibFile myFile;


void setup() {
  // Open serial communications and wait for port to open:
  pinMode(SD_CS_PIN, OUTPUT);
  delay(5000);
  Serial.begin(112500);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(/*SdSpiConfig(*/SD_CS_PIN, SPI_HALF_SPEED/*, 0, SD_SCK_MHZ(4))*/)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to test.txt...");
    myFile.println("testing 1, 2, 3.");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void loop() {
  // nothing happens after setup
}

#endif