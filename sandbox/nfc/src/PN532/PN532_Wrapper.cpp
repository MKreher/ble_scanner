#include "PN532.h"
#include "PN532_Wrapper.h"

extern "C" {

PN532* createPN532(nrf_drv_spi_t p_spi)
{
  return new PN532(p_spi);
}

void destroyPN532(PN532* pn532)
{
  delete pn532;
}

void pn532_begin(PN532* pn532)
{
  pn532->begin();
}


// Generic PN532 functions

bool pn532_SAMConfig(PN532* pn532)
{
  return pn532->SAMConfig();
}

uint32_t pn532_getFirmwareVersion(PN532* pn532)
{
  return pn532->getFirmwareVersion();
}

uint32_t pn532_readRegister(PN532* pn532, uint16_t reg)
{
  return pn532->readRegister(reg);
}

uint32_t pn532_writeRegister(PN532* pn532, uint16_t reg, uint8_t val)
{
  return pn532->writeRegister(reg, val);
}

bool pn532_writeGPIO(PN532* pn532, uint8_t pinstate)
{
  return pn532->writeGPIO(pinstate);
}

uint8_t pn532_readGPIO(PN532* pn532)
{
  return pn532->readGPIO();
}

bool pn532_setPassiveActivationRetries(PN532* pn532, uint8_t maxRetries)
{
  return pn532->setPassiveActivationRetries(maxRetries);
}

bool pn532_setRFField(PN532* pn532, uint8_t autoRFCA, uint8_t rFOnOff)
{
  return pn532->setRFField(autoRFCA, rFOnOff);
}

int8_t pn532_tgInitAsTarget(PN532* pn532, uint16_t timeout)
{
  return pn532->tgInitAsTarget(timeout);
}

int8_t pn532_tgInitAsTargetCmd(PN532* pn532, const uint8_t* command, const uint8_t len, const uint16_t timeout)
{
  return pn532->tgInitAsTarget(command, len, timeout);
}

int16_t pn532_tgGetData(PN532* pn532, uint8_t *buf, uint8_t len)
{
  return pn532->tgGetData(buf, len);
}

bool pn532_tgSetData(PN532* pn532, const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
  return pn532->tgSetData(header, hlen, body, blen);
}

int16_t pn532_inRelease(PN532* pn532, const uint8_t relevantTarget)
{
  return pn532->inRelease(relevantTarget);
}


// ISO14443A functions
bool pn532_inListPassiveTarget(PN532* pn532)
{
  return pn532->inListPassiveTarget();
}

bool pn532_readPassiveTargetID(PN532* pn532, uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout)
{
  return pn532->readPassiveTargetID(cardbaudrate, uid, uidLength, timeout);
}

bool pn532_inDataExchange(PN532* pn532, uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength)
{
  return pn532->inDataExchange(send, sendLength, response, responseLength);
}


// Mifare Classic functions
bool pn532_mifareclassic_IsFirstBlock (PN532* pn532, uint32_t uiBlock)
{
  return pn532->mifareclassic_IsFirstBlock(uiBlock);
}

bool pn532_mifareclassic_IsTrailerBlock (PN532* pn532, uint32_t uiBlock)
{
  return pn532->mifareclassic_IsTrailerBlock(uiBlock);
}

uint8_t pn532_mifareclassic_AuthenticateBlock (PN532* pn532, uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData)
{
  return pn532->mifareclassic_AuthenticateBlock(uid, uidLen, blockNumber, keyNumber, keyData);
}

uint8_t pn532_mifareclassic_ReadDataBlock (PN532* pn532, uint8_t blockNumber, uint8_t *data)
{
  return pn532->mifareclassic_ReadDataBlock(blockNumber, data);
}

uint8_t pn532_mifareclassic_WriteDataBlock (PN532* pn532, uint8_t blockNumber, uint8_t *data) {
  return pn532->mifareclassic_WriteDataBlock(blockNumber, data);
}

uint8_t pn532_mifareclassic_FormatNDEF (PN532* pn532)
{
  return pn532->mifareclassic_FormatNDEF();
}

uint8_t pn532_mifareclassic_WriteNDEFURI (PN532* pn532, uint8_t sectorNumber, uint8_t uriIdentifier, const char *url)
{
  return pn532->mifareclassic_WriteNDEFURI(sectorNumber, uriIdentifier, url);
}


// Mifare Ultralight functions
uint8_t pn532_mifareultralight_ReadPage (PN532* pn532, uint8_t page, uint8_t *buffer)
{
  return pn532->mifareultralight_ReadPage(page, buffer);
}

uint8_t pn532_mifareultralight_WritePage (PN532* pn532, uint8_t page, uint8_t *buffer)
{
  return pn532->mifareultralight_WritePage(page, buffer);
}


// FeliCa Functions
int8_t pn532_felica_Polling(PN532* pn532, uint16_t systemCode, uint8_t requestCode, uint8_t *idm, uint8_t *pmm, uint16_t *systemCodeResponse, uint16_t timeout)
{
  return pn532->felica_Polling(systemCode, requestCode, idm, pmm, systemCodeResponse);
}

int8_t pn532_felica_SendCommand (PN532* pn532, const uint8_t * command, uint8_t commandlength, uint8_t * response, uint8_t * responseLength)
{
  return pn532->felica_SendCommand(command, commandlength, response, responseLength);
}

int8_t pn532_felica_RequestService(PN532* pn532, uint8_t numNode, uint16_t *nodeCodeList, uint16_t *keyVersions)
{
  return pn532->felica_RequestService(numNode, nodeCodeList, keyVersions);
}

int8_t pn532_felica_RequestResponse(PN532* pn532, uint8_t *mode)
{
  return pn532->felica_RequestResponse(mode);
}

int8_t pn532_felica_ReadWithoutEncryption (PN532* pn532, uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16])
{
  return pn532->felica_ReadWithoutEncryption(numService, serviceCodeList, numBlock, blockList, blockData);
}

int8_t pn532_felica_WriteWithoutEncryption (PN532* pn532, uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16])
{
  return pn532->felica_WriteWithoutEncryption(numService, serviceCodeList, numBlock, blockList, blockData);
}

int8_t pn532_felica_RequestSystemCode(PN532* pn532, uint8_t *numSystemCode, uint16_t *systemCodeList)
{
  return pn532->felica_RequestSystemCode(numSystemCode, systemCodeList);
}

int8_t pn532_felica_Release(PN532* pn532)
{
  return pn532->felica_Release();
}


// Help functions to display formatted text
void pn532_PrintHex(PN532* pn532, const uint8_t *data, const uint32_t numBytes)
{
  pn532->PrintHex(data, numBytes);
}

void pn532_PrintHexChar(PN532* pn532, const uint8_t *pbtData, const uint32_t numBytes)
{
  pn532->PrintHexChar(pbtData, numBytes);
}

uint8_t *pn532_getBuffer(PN532* pn532, uint8_t *len)
{
  return pn532->getBuffer(len);
}

}