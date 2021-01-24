#ifndef __PN532_WRAPPER_H__
#define __PN532_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_drv_spi.h"

typedef struct PN532 PN532;

PN532* createPN532_SPI(nrf_drv_spi_t p_spi);
void destroyPN532(PN532* pn532);

void pn532_begin(PN532* pn532);

// Generic PN532 functions
bool pn532_SAMConfig(PN532* pn532);
uint32_t pn532_getFirmwareVersion(PN532* pn532);
uint32_t pn532_readRegister(PN532* pn532, uint16_t reg);
uint32_t pn532_writeRegister(PN532* pn532, uint16_t reg, uint8_t val);
bool pn532_writeGPIO(PN532* pn532, uint8_t pinstate);
uint8_t pn532_readGPIO(PN532* pn532);
bool pn532_setPassiveActivationRetries(PN532* pn532, uint8_t maxRetries);
bool pn532_setRFField(PN532* pn532, uint8_t autoRFCA, uint8_t rFOnOff);

/**
* @brief    Init PN532 as a target
* @param    timeout max time to wait, 0 means no timeout
* @return   > 0     success
*           = 0     timeout
*           < 0     failed
*/
int8_t pn532_tgInitAsTarget(PN532* pn532, uint16_t timeout);
int8_t pn532_tgInitAsTargetCmd(PN532* pn532, const uint8_t* command, const uint8_t len, const uint16_t timeout);

int16_t pn532_tgGetData(PN532* pn532, uint8_t *buf, uint8_t len);
bool pn532_tgSetData(PN532* pn532, const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen);

int16_t pn532_inRelease(PN532* pn532, const uint8_t relevantTarget);

// ISO14443A functions
bool pn532_inListPassiveTarget(PN532* pn532);
bool pn532_readPassiveTargetID(PN532* pn532, uint8_t cardbaudrate, uint8_t *uid, uint8_t *uidLength, uint16_t timeout);
bool pn532_inDataExchange(PN532* pn532, uint8_t *send, uint8_t sendLength, uint8_t *response, uint8_t *responseLength);

// Mifare Classic functions
bool pn532_mifareclassic_IsFirstBlock (PN532* pn532, uint32_t uiBlock);
bool pn532_mifareclassic_IsTrailerBlock (PN532* pn532, uint32_t uiBlock);
uint8_t pn532_mifareclassic_AuthenticateBlock (PN532* pn532, uint8_t *uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t *keyData);
uint8_t pn532_mifareclassic_ReadDataBlock (PN532* pn532, uint8_t blockNumber, uint8_t *data);
uint8_t pn532_mifareclassic_WriteDataBlock (PN532* pn532, uint8_t blockNumber, uint8_t *data);
uint8_t pn532_mifareclassic_FormatNDEF (PN532* pn532);
uint8_t pn532_mifareclassic_WriteNDEFURI (PN532* pn532, uint8_t sectorNumber, uint8_t uriIdentifier, const char *url);

// Mifare Ultralight functions
uint8_t pn532_mifareultralight_ReadPage (PN532* pn532, uint8_t page, uint8_t *buffer);
uint8_t pn532_mifareultralight_WritePage (PN532* pn532, uint8_t page, uint8_t *buffer);

// FeliCa Functions
int8_t pn532_felica_Polling(PN532* pn532, uint16_t systemCode, uint8_t requestCode, uint8_t *idm, uint8_t *pmm, uint16_t *systemCodeResponse, uint16_t timeout);
int8_t pn532_felica_SendCommand (PN532* pn532, const uint8_t * command, uint8_t commandlength, uint8_t * response, uint8_t * responseLength);
int8_t pn532_felica_RequestService(PN532* pn532, uint8_t numNode, uint16_t *nodeCodeList, uint16_t *keyVersions) ;
int8_t pn532_felica_RequestResponse(PN532* pn532, uint8_t *mode);
int8_t pn532_felica_ReadWithoutEncryption (PN532* pn532, uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]);
int8_t pn532_felica_WriteWithoutEncryption (PN532* pn532, uint8_t numService, const uint16_t *serviceCodeList, uint8_t numBlock, const uint16_t *blockList, uint8_t blockData[][16]);
int8_t pn532_felica_RequestSystemCode(PN532* pn532, uint8_t *numSystemCode, uint16_t *systemCodeList);
int8_t pn532_felica_Release(PN532* pn532);

// Help functions to display formatted text
static void pn532_PrintHex(PN532* pn532, const uint8_t *data, const uint32_t numBytes);
static void pn532_PrintHexChar(PN532* pn532, const uint8_t *pbtData, const uint32_t numBytes);

uint8_t *pn532_getBuffer(PN532* pn532, uint8_t *len);

#ifdef __cplusplus
}
#endif

#endif //__PN532_WRAPPER_H__