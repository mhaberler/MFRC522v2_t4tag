
#ifdef M5UNIFIED
    #include <M5Unified.h>
#endif
#ifdef PLAIN_ARDUINO
    #include <Arduino.h>
#endif
#ifdef M5STICK
    #include "M5StickCPlus.h"
#endif

#ifdef USE_I2C
    #include <MFRC522DriverI2C.h>
    #include <Wire.h>
#else
    #include <MFRC522DriverPinSimple.h>
    #include <MFRC522DriverSPI.h>
#endif

#include "NdefMessage.h"
#include "NfcAdapter.h"
#include <MFRC522Debug.h>
#include <MFRC522Extended.h>
#include <MFRC522v2.h>
#include <SPI.h>
#include <stdlib.h>

using StatusCode = MFRC522Constants::StatusCode;
MFRC522::MIFARE_Key key = {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}};

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
JsonDocument jsondoc;

typedef enum {
    BWTAG_NO_MATCH,
    BWTAG_RUUVI,
    BWTAG_RUUVI_OAT,
    BWTAG_RUUVI_ENV,
    BWTAG_TANK,
    BWTAG_BURNER,
    BWTAG_FLOWSENSOR,
    BWTAG_PRESSURESENSOR
} bwTagType_t;

#define BW_MIMETYPE "application/balloonware"
#define BW_ALT_MIMETYPE "bw"

static const char *ruuvi_ids[] = {
    "\002idID: ",
    "\002adMAC: ",
    "\002swSW: ",
    "\002dt",
};

bwTagType_t
analyseTag(NfcTag &tag, JsonDocument &doc) {
    if (!tag.hasNdefMessage()) {
        return BWTAG_NO_MATCH;
    }
    auto nrec = tag.getNdefMessage().getRecordCount();
    if ((tag.getTagType() == MFRC522Constants::PICC_TYPE_ISO_14443_4) && (nrec == 4)) {
        String content[4];
        for (auto i = 0; i < nrec; i++) {
            NdefRecord record = tag.getNdefMessage()[i];
            const byte *payload = record.getPayload();
            size_t prefix_len = strlen((const char *)ruuvi_ids[i]);
            if (payload == NULL) {
                return BWTAG_NO_MATCH;
            }
            if (memcmp(payload, ruuvi_ids[i], prefix_len) != 0) {
                return BWTAG_NO_MATCH;
            }
            content[i] = String(record.getPayload() + prefix_len,
                                record.getPayloadLength() - prefix_len);
        }
        // if we made it here, it's a Ruuvi tag
        doc["ID"] = content[0];
        doc["MAC"] = content[1];
        doc["SW"] = content[2];
        // skip the mystery 'dt' record
        return BWTAG_RUUVI;
    }
    if (tag.getTagType() == MFRC522Constants::PICC_TYPE_MIFARE_1K) {
        // tank tag?
        for (auto i = 0; i < nrec; i++) {
            NdefRecord record = tag.getNdefMessage()[i];

            if (record.getType() && (((strncmp((const char *)record.getType(),
                                              BW_MIMETYPE, record.getTypeLength()) == 0))||
                    (strncmp((const char *)record.getType(),
                             BW_ALT_MIMETYPE, record.getTypeLength()) == 0))) {
                // this is for us. Payload is a JSON string.
                String payload = String(record.getPayload(),
                                        record.getPayloadLength());
                DeserializationError e = deserializeJson(doc, payload);
                if (e == DeserializationError::Ok) {
                    return BWTAG_TANK;
                }
                Serial.printf("deserialisation failed: %s\n",
                              e.c_str());
                return BWTAG_NO_MATCH;
            }
        }
    }
    return BWTAG_NO_MATCH;
}

void setup(void) {
#ifdef M5UNIFIED
    M5.begin();
#endif
#ifdef PLAIN_ARDUINO
    Serial.begin(115200);
#endif
#ifdef M5STICK
    M5.begin();
#endif
#ifdef STARTUP_DELAY
    delay(STARTUP_DELAY);
#endif
    Serial.println("Extended NDEF Reader\nPlace an unformatted Mifare Classic "
                   "tag on the reader to show contents.");
#ifdef USE_I2C
#ifdef I2C0_SDA
    Wire.begin(I2C0_SDA, I2C0_SCL, I2C0_SPEED);
#else
    Wire.begin();
#endif
#else
    SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN);
#endif
    mfrc522.PCD_Init(); // Init MFRC522
    nfc.begin();
    nfc.setMifareKey(&key);
    MFRC522Debug::PCD_DumpVersionToSerial(
        mfrc522, Serial); // Show version of PCD - MFRC522 Card Reader.
}

void loop(void) {
    if (nfc.tagPresent()) {
        Serial.println("\ndetected NFC tag");
        // delay(1000);
        Serial.println("\nReading NFC tag");
        NfcTag tag = nfc.read();

        bwTagType_t type = analyseTag(tag, jsondoc);
        Serial.printf("analyseTag=%d\n", type);
        if (type != BWTAG_NO_MATCH) {
            serializeJsonPretty(jsondoc, Serial);
        }
        jsondoc.clear();

        tag.tagToJson (jsondoc);
        serializeJsonPretty (jsondoc, Serial);
        jsondoc.clear ();
        nfc.haltTag();
    }
    delay(10);
}
