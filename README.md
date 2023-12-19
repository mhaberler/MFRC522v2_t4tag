## Read an NFC Type4 Tag with an RC522 RFID Reader

this example can read an NDEF message off a Type4 NFC tag

tested with a [Ruuvi Tag](https://ruuvi.com/ruuvitag/) which reports MAC address, software revision etc via an NDEF message

readers used:
- [M5Stack RFID2](https://docs.m5stack.com/en/unit/rfid2) via I2C
- [RC522 RFID Reader](https://www.ebay.at/itm/302933878447) via SPI

Notes:
- driver: MFRC522v2 https://github.com/OSSLibraries/Arduino_MFRC522v2
- adapted MFRC522Extended.cpp + MFRC522Extended.h from https://github.com/miguelbalboa/rfid/ for MFRC522v2
- adapted https://github.com/TheNitek/NDEF for MFRC522v2: 
https://github.com/mhaberler/NDEF.git#MFRC522v2-compatible

The T4Tag reading code is not yet integrated into the NDEF library, so this example supports reading of just a Type4 tag.

Platformio targets:
- M5Stack-Core2 (I2C - red port)
- M5Stack-CoreS3 (I2C - red port)
- ESP32-S3-DevKitC-1 v1.1 - I2C and SPI

Example output:

`````
...
read NDEF file, size=141
Decoding 141 bytes
81 01 00 00 00 1E 54 02 69 64 49 44 3A 20 43 35 3A 35 41 3A 42 34 3A 46 43 3A 35 32 3A 35 43 3A 46 34 3A 42 32 01 01 00 00 00 19 54 02 61 64 4D 41 43 3A 20 43 32 3A 36 90 00 44 31 3A 37 30 3A 32 42 3A 34 34 01 01 00 00 00 1F 54 02 73 77 53 57 3A 20 52 75 75 76 69 20 46 57 20 76 33 2E 33 31 2E 31 2B 64 65 66 61 75 6C 74 41 01 00 00 00 1B 54 90 00 74 05 0C BA 3F 49 FF FF FE 14 03 30 01 04 6D 76 4A 04 1A C2 6E D1 70 90 00  �.....T.idID: C5:5A:B4:FC:52:5C:F4:B2......T.adMAC: C2:6�.D1:70:2B:44......T.swSW: Ruuvi FW v3.31.1+defaultA.....T�.t..�?I���..0..mvJ..�n�p�.

NDEF Message 4 records, 129 bytes
  NDEF Record
    TNF 0x1 Well Known
    Type Length 0x1 1
    Payload Length 0x1E 30
    Type 54  T
    Payload 
02 69 64 49 44 3A 20 43 35 3A 35 41 3A 42 34 3A 46 43 3A 35 32 3A 35 43 3A 46 34 3A 42 32  .idID: C5:5A:B4:FC:52:5C:F4:B2
    Record is 34 bytes
  NDEF Record
    TNF 0x1 Well Known
    Type Length 0x1 1
    Payload Length 0x19 25
    Type 54  T
    Payload 
02 61 64 4D 41 43 3A 20 43 32 3A 36 90 00 44 31 3A 37 30 3A 32 42 3A 34 34  .adMAC: C2:6�.D1:70:2B:44
    Record is 29 bytes
  NDEF Record
    TNF 0x1 Well Known
    Type Length 0x1 1
    Payload Length 0x1F 31
    Type 54  T
    Payload 
02 73 77 53 57 3A 20 52 75 75 76 69 20 46 57 20 76 33 2E 33 31 2E 31 2B 64 65 66 61 75 6C 74  .swSW: Ruuvi FW v3.31.1+default
    Record is 35 bytes
  NDEF Record
    TNF 0x1 Well Known
    Type Length 0x1 1
    Payload Length 0x1B 27
    Type 54  T
    Payload 
90 00 74 05 0C BA 3F 49 FF FF FE 14 03 30 01 04 6D 76 4A 04 1A C2 6E D1 70 90 00  �.t..�?I���..0..mvJ..�n�p�.
    Record is 31 bytes

`````
