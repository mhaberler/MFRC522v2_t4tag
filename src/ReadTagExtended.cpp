

#ifdef USE_I2C
#include <MFRC522DriverI2C.h>
#include <Wire.h>
#else
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#endif

#include <MFRC522Debug.h>
#include <MFRC522v2.h>
#include <MFRC522Extended.h>
#include <SPI.h>
#include <stdlib.h>
#include "NdefMessage.h"
#include "NfcAdapter.h"

using StatusCode = MFRC522Constants::StatusCode;

#ifdef USE_I2C
const uint8_t customAddress = 0x28;
TwoWire &customI2C = Wire;
MFRC522DriverI2C driver{customAddress, customI2C};  // Create I2C driver.

#else
MFRC522DriverPinSimple ss_pin(
    SS_PIN);  // Create pin driver. See typical pin layout above.
SPIClass &spiClass =
    SPI;  // Alternative SPI e.g. SPI2 or from library e.g. softwarespi.
const SPISettings spiSettings =
    SPISettings(SPI_CLOCK_DIV4, MSBFIRST,
                SPI_MODE0);  // May have to be set if hardware is not fully
                             // compatible to Arduino specifications.
MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings};  // Create SPI driver.
#endif

MFRC522Extended mfrc522{driver}; // Create MFRC522 instance.


NfcAdapter nfc = NfcAdapter(&mfrc522);
StaticJsonDocument<2048> jsondoc;

void setup(void)
{
      Serial.begin(9600);
      Serial.println("Extended NDEF Reader\nPlace an unformatted Mifare Classic tag on the reader to show contents.");
#ifdef USE_I2C
  Wire.begin(I2C0_SDA, I2C0_SCL, I2C0_SPEED);
#else
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
#endif
      mfrc522.PCD_Init(); // Init MFRC522
      nfc.begin();
}

void loop(void)
{
  if (nfc.tagPresent())
  {
    Serial.println("\nReading NFC tag");
    NfcTag tag = nfc.read();

    tag.toJson(jsondoc);
    serializeJsonPretty(jsondoc, Serial);
    nfc.haltTag();
  }
  delay(1000);
}