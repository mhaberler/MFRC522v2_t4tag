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
- the latter modified to generate JSON output (see `toJson()` methods)
- non-goal: backwards compatibility with existing code using don/NDEF and variants thereof


Platformio targets:
- M5Stack-Core2 (I2C - red port)
- M5Stack-CoreS3 (I2C - red port)
- ESP32-S3-DevKitC-1 v1.1 - I2C and SPI

Example output:

`````
...
Reading NFC tag
Decoding 141 bytes
00000000: 81 01 00 00 00 1E 54 02 69 64 49 44 3A 20 43 35  �.....T.idID: C5
00000010: 3A 35 41 3A 42 34 3A 46 43 3A 35 32 3A 35 43 3A  :5A:B4:FC:52:5C:
00000020: 46 34 3A 42 32 01 01 00 00 00 19 54 02 61 64 4D  F4:B2......T.adM
00000030: 41 43 3A 20 43 32 3A 36 45 3A 44 31 3A 37 30 3A  AC: C2:6E:D1:70:
00000040: 32 42 3A 34 34 01 01 00 00 00 1F 54 02 73 77 53  2B:44......T.swS
00000050: 57 3A 20 52 75 75 76 69 20 46 57 20 76 33 2E 33  W: Ruuvi FW v3.3
00000060: 31 2E 31 2B 64 65 66 61 75 6C 74 41 01 00 00 00  1.1+defaultA....
00000070: 1B 54 02 64 74 05 12 3A 29 34 FF FF 01 8C FE C8  .T.dt..:)4��.���
00000080: FC 4C 77 F6 A5 0C 97 C2 6E D1 70 00 AD           �Lw��.��n�p.�
{
  "uid": "5F 3E 20 3F 0E AE 5E",
  "sak": 32,
  "type": 4,
  "ndef": [
    {
      "tnf": 1,
      "name": "Well Known",
      "type": "T",
      "payload": "idID: C5:5A:B4:FC:52:5C:F4:B2"
    },
    {
      "tnf": 1,
      "name": "Well Known",
      "type": "T",
      "payload": "adMAC: C2:6E:D1:70:2B:44"
    },
    {
      "tnf": 1,
      "name": "Well Known",
      "type": "T",
      "payload": "swSW: Ruuvi FW v3.31.1+default"
    },
    {
      "tnf": 1,
      "name": "Well Known",
      "type": "T",
      "payload": "dt:)4������Lw��\f��n�p\u0000�"
    }
  ]
}

`````
