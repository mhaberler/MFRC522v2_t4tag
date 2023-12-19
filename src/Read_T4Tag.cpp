

#ifdef USE_I2C
#include <MFRC522DriverI2C.h>
#include <Wire.h>
#else
#include <MFRC522DriverPinSimple.h>
#include <MFRC522DriverSPI.h>
#endif

#include <MFRC522Debug.h>
#include <MFRC522v2.h>
#include <SPI.h>
#include <T4Tag.h>
#include <stdlib.h>

#include "HexDump.h"
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

T4Tag ntag{driver};  // : T4Tag(ss, rst), NfcAdapter(this){};

bool deselectAndWakeupA = false;
void printHex(byte *buffer, uint16_t bufferSize);

void setup() {
  Serial.begin(115200);
#ifdef USE_I2C
  Wire.begin(I2C0_SDA, I2C0_SCL, I2C0_SPEED);
#else
  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
#endif
  ntag.PCD_Init();
  Serial.println(F("POWER ON"));
}

void loop() {
  StatusCode statusCode;
  delay(1000);
  if (deselectAndWakeupA) {
    deselectAndWakeupA = false;
    if (!ntag.PICC_TryDeselectAndWakeupA()) return;
  } else if (!ntag.PICC_IsNewCardPresent()) {
    return;
  }

  if (!ntag.PICC_ReadCardSerial()) return;

  statusCode = ntag.SelectNdefTag_Application();
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("SelectNdefTag_Application fail: %d   %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));

    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }

  uint16_t fileId, maxSize;
  uint8_t readAccess, writeAccess;
  statusCode = ntag.SelectCapabilityContainer();
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("SelectCapabilityContainer fail: %d   %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));

    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }

  statusCode = ntag.readCCFile(fileId, maxSize, readAccess, writeAccess);
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("readCCFile fail: %d\n", (int)statusCode);
    Serial.printf("readCCFile fail: %d %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));

    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }
  Serial.printf("NDEF file id=0x%x maxSize=%u read=0x%x write=0x%x\n", fileId,
                maxSize, readAccess, writeAccess);
  statusCode = ntag.selectFile(fileId);
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("selectNdefFile fail: %d   %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));
    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }

  uint16_t fileSize;
  statusCode = ntag.readFileLength(fileSize);
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("readNdefFileLength fail: %d   %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));
    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }
  Serial.printf("NDEF file size=%u\n", fileSize);

  uint8_t *buffer = (uint8_t *)calloc(maxSize, 1);

  statusCode = ntag.readFile(buffer, fileSize);
  if (statusCode != StatusCode::STATUS_OK) {
    Serial.printf("readNdefFile fail: %d   %s\n", (int)statusCode,
                  (const char *)MFRC522Debug::GetStatusCodeName(statusCode));
    if (statusCode != StatusCode::STATUS_TIMEOUT) deselectAndWakeupA = true;
    return;
  }
  Serial.printf("read NDEF file, size=%u\n", fileSize);

  NdefMessage msg(buffer, fileSize);
  msg.print();
  ;
}

void printHex(byte *buffer, uint16_t bufferSize) {
  HexDump(Serial, buffer, bufferSize);
}
