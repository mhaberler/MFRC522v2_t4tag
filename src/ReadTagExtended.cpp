#ifdef USE_I2C
#include <MFRC522DriverI2C.h>
#include <Wire.h>
#else
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#endif

#include "NdefMessage.h"
#include "NfcAdapter.h"
#include <ArduinoJson.h>
#include <MFRC522Debug.h>
#include <MFRC522Extended.h>
#include <MFRC522v2.h>
#include <SPI.h>
#include <stdlib.h>

using StatusCode = MFRC522Constants::StatusCode;

#ifdef USE_I2C
const uint8_t customAddress = 0x28;
TwoWire &customI2C = Wire;
MFRC522DriverI2C driver{customAddress, customI2C}; // Create I2C driver.

#else
MFRC522DriverPinSimple
    ss_pin(SS_PIN);       // Create pin driver. See typical pin layout above.
SPIClass &spiClass = SPI; // Alternative SPI e.g. SPI2 or from library e.g. softwarespi.
const SPISettings spiSettings = SPISettings(SPI_CLOCK_DIV4, MSBFIRST,
                                            SPI_MODE0); // May have to be set if hardware is not fully
                                                        // compatible to Arduino specifications.
MFRC522DriverSPI driver{ss_pin, spiClass, spiSettings}; // Create SPI driver.
#endif

MFRC522Extended mfrc522{driver}; // Create MFRC522 instance.

NfcAdapter nfc = NfcAdapter(&mfrc522);
StaticJsonDocument<2048> ndefmsg;

void setup(void)
{
  Serial.begin(9600);
  Serial.println("Extended NDEF Reader\nPlace an unformatted Mifare Classic "
                 "tag on the reader to show contents.");
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
    Serial.println("Reading NFC tag");
    NfcTag tag = nfc.read();
    Serial.println(tag.getTagType());
    Serial.print("UID: ");
    Serial.println(tag.getUidString());

    if (tag.hasNdefMessage()) // every tag won't have a message
    {

      NdefMessage message = tag.getNdefMessage();
      Serial.print(
          "\nThis NFC Tag contains an NDEF Message with ");
      Serial.print(message.getRecordCount());
      Serial.print(" NDEF Record");
      if (message.getRecordCount() != 1)
      {
        Serial.print("s");
      }
      Serial.println(".");

      message.toJson(ndefmsg);
      Serial.print("json='");

      serializeJsonPretty(ndefmsg, Serial);
      Serial.println("'");

      // cycle through the records, printing some info from each
      int recordCount = message.getRecordCount();
      for (int i = 0; i < recordCount; i++)
      {
        Serial.print("\nNDEF Record ");
        Serial.println(i + 1);
        NdefRecord record = message.getRecord(i);
        // NdefRecord record = message[i]; // alternate
        // syntax

        Serial.print("  TNF: ");
        Serial.println(record.getTnf());
        Serial.print("  Type: ");
        PrintHexChar(
            record.getType(),
            record.getTypeLength()); // will be "" for
                                     // TNF_EMPTY

        // The TNF and Type should be used to determine how
        // your application processes the payload There's no
        // generic processing for the payload, it's returned
        // as a byte[]
        int payloadLength = record.getPayloadLength();
        const byte *payload = record.getPayload();

        // Print the Hex and Printable Characters
        Serial.print("  Payload (HEX): ");
        PrintHexChar(payload, payloadLength);

        // Force the data into a String (might work
        // depending on the content) Real code should use
        // smarter processing
        String payloadAsString = "";
        for (int c = 0; c < payloadLength; c++)
        {
          payloadAsString += (char)payload[c];
        }
        Serial.print("  Payload (as String): ");
        Serial.println(payloadAsString);

        // id is probably blank and will return ""
        if (record.getIdLength() > 0)
        {
          Serial.print("  ID: ");
          PrintHexChar(record.getId(),
                       record.getIdLength());
        }
      }
    }
    nfc.haltTag();
  }
  delay(1000);
}
