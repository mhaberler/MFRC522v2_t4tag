#pragma once

#include <MFRC522Extended.h>

class T4Tag : public MFRC522Extended {
 public:
  using StatusCode = MFRC522Constants::StatusCode;

  T4Tag(MFRC522Driver& driver) : MFRC522Extended(driver){};

  StatusCode SelectNdefTag_Application() {
    byte sendData[] = {0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76,
                       0x00, 0x00, 0x85, 0x01, 0x01, 0x00};
    byte sendLen = sizeof(sendData);
    Serial.printf("sendlen=%u\n", sendLen);
    byte backData[2];
    byte backLen = 2;

    StatusCode statusCode = TCL_Transceive(
        &this->tag, sendData, sizeof(sendData), backData, &backLen);
    if (statusCode != StatusCode::STATUS_OK) return statusCode;
    if (backLen < 2) return StatusCode::STATUS_INTERNAL_ERROR;

    if (backData[backLen - 2] != 0x90 || backData[backLen - 1] != 0x00)
      return StatusCode::STATUS_ERROR;
    return StatusCode::STATUS_OK;
  };

  StatusCode SelectCapabilityContainer() {
    byte sendData[] = {0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03};
    byte backData[3] = {0};
    byte backLen = 3;  // sizeof(backData);

    StatusCode statusCode = TCL_Transceive(
        &this->tag, sendData, sizeof(sendData), backData, &backLen);
    if (statusCode != StatusCode::STATUS_OK) return statusCode;
    if (backLen < 2) return StatusCode::STATUS_INTERNAL_ERROR;
    if (backData[backLen - 2] != 0x90 || backData[backLen - 1] != 0x00)
      return StatusCode::STATUS_ERROR;

    return StatusCode::STATUS_OK;
  };

  StatusCode readCCFile(uint16_t& fileId, uint16_t& maxSize,
                        uint8_t& readAccess, uint8_t& writeAccess) {
    uint8_t readCmd[] = {0x00, 0xB0, 0x00, 0x00, 0x0F};
    uint8_t response[17];
    uint8_t responseLen = sizeof(response);

    StatusCode statusCode = TCL_Transceive(&this->tag, readCmd, sizeof(readCmd),
                                           response, &responseLen);
    if (statusCode != StatusCode::STATUS_OK) return statusCode;
    if (responseLen < 2) return StatusCode::STATUS_INTERNAL_ERROR;
    if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
      return StatusCode::STATUS_ERROR;

    fileId = (response[9] << 8) | response[10];
    maxSize = (response[11] << 8) | response[12];
    readAccess = response[13];
    writeAccess = response[14];
    return statusCode;
  };

  StatusCode selectFile(const uint16_t fileId) {
    uint8_t selectCmd[] = {
        0x00, 0xA4, 0x00, 0x0C, 0x02, (fileId >> 8) & 0xff, fileId & 0xff};
    uint8_t response[3];
    uint8_t responseLen = sizeof(response);

    StatusCode statusCode = TCL_Transceive(
        &this->tag, selectCmd, sizeof(selectCmd), response, &responseLen);
    if (statusCode != StatusCode::STATUS_OK) return statusCode;
    if (responseLen < 2) return StatusCode::STATUS_INTERNAL_ERROR;

    if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
      return StatusCode::STATUS_ERROR;
    return StatusCode::STATUS_OK;
  }

  StatusCode readFileLength(uint16_t& fileSize) {
    uint8_t cmd[] = {0x00, 0xB0, 0x00, 0x00, 0x02};
    uint8_t response[5];
    uint8_t responseLen = sizeof(response);

    StatusCode statusCode =
        TCL_Transceive(&this->tag, cmd, sizeof(cmd), response, &responseLen);
    if (statusCode != StatusCode::STATUS_OK) return statusCode;
    if (responseLen < 2) return StatusCode::STATUS_INTERNAL_ERROR;
    if (response[responseLen - 2] != 0x90 || response[responseLen - 1] != 0x00)
      return StatusCode::STATUS_ERROR;

    fileSize = (response[0] << 8) | response[1];
    return statusCode;
  };

  // caller allocates
  StatusCode readFile(uint8_t* response, const size_t fileLength) {
    StatusCode statusCode = StatusCode::STATUS_OK;
    uint8_t buffer[fileLength];
    uint8_t responseLen;
    bool success = true;
    size_t mtu = FIFO_SIZE - 6;  // leave room for CRC etc
    size_t pos = 0;
    while ((pos < fileLength) && success) {
      uint8_t read_len = ((fileLength - pos) > mtu) ? mtu : (fileLength - pos);
      uint8_t readstart = pos;
      uint8_t readCmd[5] = {0x00, 0xB0, (readstart >> 8) & 0xff,
                            readstart & 0xff, read_len};
      responseLen = fileLength;
      statusCode = TCL_Transceive(&this->tag, readCmd, sizeof(readCmd), buffer,
                                  &responseLen);
#if defined(TRACE_IO) && defined(NDEF_USE_SERIAL)
      Serial.println(F("response:"));
      HexDump(Serial, response, responseLen);
#endif
      success = (statusCode == StatusCode::STATUS_OK);
      if (success) {
        memcpy(response, buffer + 2, responseLen - 2);
        response += (responseLen - 2);
      }
      pos += read_len;
    }
    return success ? StatusCode::STATUS_OK : statusCode;
  }

  bool PICC_TryDeselectAndWakeupA() {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);

    TCL_Deselect(&tag);  // deselect current card

    // Reset baud rates
    _driver.PCD_WriteRegister(PCD_Register::TxModeReg, 0x00);
    _driver.PCD_WriteRegister(PCD_Register::RxModeReg, 0x00);
    // Reset ModWidthReg
    _driver.PCD_WriteRegister(PCD_Register::ModWidthReg, 0x26);

    MFRC522::StatusCode result = PICC_WakeupA(bufferATQA, &bufferSize);

    if (result == StatusCode::STATUS_OK ||
        result == StatusCode::STATUS_COLLISION) {
      tag.atqa = ((uint16_t)bufferATQA[1] << 8) | bufferATQA[0];
      tag.ats.size = 0;
      tag.ats.fsc = 32;  // default FSC value

      // Defaults for TA1
      tag.ats.ta1.transmitted = false;
      tag.ats.ta1.sameD = false;
      tag.ats.ta1.ds = MFRC522Extended::BITRATE_106KBITS;
      tag.ats.ta1.dr = MFRC522Extended::BITRATE_106KBITS;

      // Defaults for TB1
      tag.ats.tb1.transmitted = false;
      tag.ats.tb1.fwi = 0;   // TODO: Don't know the default for this!
      tag.ats.tb1.sfgi = 0;  // The default value of SFGI is 0 (meaning that the
                             // card does not need any particular SFGT)

      // Defaults for TC1
      tag.ats.tc1.transmitted = false;
      tag.ats.tc1.supportsCID = true;
      tag.ats.tc1.supportsNAD = false;

      memset(tag.ats.data, 0, FIFO_SIZE - 2);

      tag.blockNumber = false;
      return true;
    }
    return false;
  }
};
