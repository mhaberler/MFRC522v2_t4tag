## Read an NFC Type4 Tag with an RC522 RFID Reader

this example can read an NDEF message off a Type4 NFC tag, among the other types supported by https://github.com/IktaS/NDEF - Mifare Classic & Ultralight

tested with a [Ruuvi Tag](https://ruuvi.com/ruuvitag/) which reports MAC address, software revision etc via an NDEF message

reader hardware used:
- [M5Stack RFID2](https://docs.m5stack.com/en/unit/rfid2) via I2C
- [RC522 RFID Reader](https://www.ebay.at/itm/302933878447) via SPI

Notes:
- driver: MFRC522v2 https://github.com/OSSLibraries/Arduino_MFRC522v2
- adapted MFRC522Extended.cpp + MFRC522Extended.h from https://github.com/miguelbalboa/rfid/ for MFRC522v2 - spun out into a separate repo at https://github.com/mhaberler/Arduino_MFRC522v2Extended.git
- extended https://github.com/IktaS/NDEF to support Type4 tag reading


Platformio targets:
- M5Stack-Core2 (I2C - red port)
- M5Stack-CoreS3 (I2C - red port)
- ESP32-S3-DevKitC-1 v1.1 - I2C and SPI

Example output:

`````
...
Reading NFC tag
...
Decoding 141 bytes
81 01 00 00 00 1E 54 02 69 64 49 44 3A 20 43 35 3A 35 41 3A 42 34 3A 46 43 3A 35 32 3A 35 43 3A 46 34 3A 42 32 01 01 00 00 00 19 54 02 61 64 4D 41 43 3A 20 43 32 3A 36 90 00 44 31 3A 37 30 3A 32 42 3A 34 34 01 01 00 00 00 1F 54 02 73 77 53 57 3A 20 52 75 75 76 69 20 46 57 20 76 33 2E 33 31 2E 31 2B 64 65 66 61 75 6C 74 41 01 00 00 00 1B 54 90 00 74 05 11 FB 29 96 FF FF 01 08 FE B4 FC 2C 76 36 C8 0F A4 C2 6E D1 70 90 00  �.....T.idID: C5:5A:B4:FC:52:5C:F4:B2......T.adMAC: C2:6�.D1:70:2B:44......T.swSW: Ruuvi FW v3.31.1+defaultA.....T�.t..�)���..���,v6�.��n�p�.
4
UID: 5F 3E 20 3F 0E AE 5E

This NFC Tag contains an NDEF Message with 4 NDEF Records.

NDEF Record 1
  TNF: 1
  Type: 54  T
  Payload (HEX): 02 69 64 49 44 3A 20 43 35 3A 35 41 3A 42 34 3A 46 43 3A 35 32 3A 35 43 3A 46 34 3A 42 32  .idID: C5:5A:B4:FC:52:5C:F4:B2
  Payload (as String): idID: C5:5A:B4:FC:52:5C:F4:B2

NDEF Record 2
  TNF: 1
  Type: 54  T
  Payload (HEX): 02 61 64 4D 41 43 3A 20 43 32 3A 36 90 00 44 31 3A 37 30 3A 32 42 3A 34 34  .adMAC: C2:6�.D1:70:2B:44
  Payload (as String): adMAC: C2:6�D1:70:2B:44

NDEF Record 3
  TNF: 1
  Type: 54  T
  Payload (HEX): 02 73 77 53 57 3A 20 52 75 75 76 69 20 46 57 20 76 33 2E 33 31 2E 31 2B 64 65 66 61 75 6C 74  .swSW: Ruuvi FW v3.31.1+default
  Payload (as String): swSW: Ruuvi FW v3.31.1+default

NDEF Record 4
  TNF: 1
  Type: 54  T
  Payload (HEX): 90 00 74 05 11 FB 29 96 FF FF 01 08 FE B4 FC 2C 76 36 C8 0F A4 C2 6E D1 70 90 00  �.t..�)���..���,v6�.��n�p�.
  Payload (as String): �t�)�����,v6���n�p�

`````
