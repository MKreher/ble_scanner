/**************************************************************************/
/*!
    @file     PN532.cpp
    @author   Adafruit Industries & Seeed Studio
    @license  BSD
*/
/**************************************************************************/

#include "PN532.h"
#include "PN532_SPI.h"
#include <string.h>

extern "C"
{
#include "nrf_log.h"
}

// PN532Interface *m_pn532_hal;

PN532::PN532(nrf_drv_spi_t p_spi) { m_pn532_hal = new PN532_SPI(p_spi); }

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
void PN532::begin()
{
    m_pn532_hal->begin();
    m_pn532_hal->wakeup();
}

/**************************************************************************/
/*!
    @brief  Prints a hexadecimal value in plain characters

    @param  data      Pointer to the uint8_t data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void PN532::PrintHex(const uint8_t * data, const uint32_t numBytes)
{
    NRF_LOG_HEXDUMP_DEBUG(data, numBytes);
}

/**************************************************************************/
/*!
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t PN532::getFirmwareVersion(void)
{
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, COMMAND_GETFIRMWAREVERSION_LENGTH))
    {
        return 0;
    }

    int16_t status = m_pn532_hal->readResponse(pn532_packetbuffer, REPLY_GETFIRMWAREVERSION_LENGTH);
    if (0 > status)
    {
        return 0;
    }

    response = uint32_big_decode(pn532_packetbuffer + PN532_DATA_OFFSET + 1);

    return response;
}


/**************************************************************************/
/*!
    @brief  Read a PN532 register.

    @param  reg  the 16-bit register address.

    @returns  The register value.
*/
/**************************************************************************/
uint32_t PN532::readRegister(uint16_t reg)
{
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_READREGISTER;
    pn532_packetbuffer[1] = (reg >> 8) & 0xFF;
    pn532_packetbuffer[2] = reg & 0xFF;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 3))
    {
        return 0;
    }

    // read data packet
    int16_t status = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));
    if (0 > status)
    {
        return 0;
    }

    response = pn532_packetbuffer[0];

    return response;
}

/**************************************************************************/
/*!
    @brief  Write to a PN532 register.

    @param  reg  the 16-bit register address.
    @param  val  the 8-bit value to write.

    @returns  0 for failure, 1 for success.
*/
/**************************************************************************/
uint32_t PN532::writeRegister(uint16_t reg, uint8_t val)
{
    uint32_t response;

    pn532_packetbuffer[0] = PN532_COMMAND_WRITEREGISTER;
    pn532_packetbuffer[1] = (reg >> 8) & 0xFF;
    pn532_packetbuffer[2] = reg & 0xFF;
    pn532_packetbuffer[3] = val;


    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 4))
    {
        return 0;
    }

    // read data packet
    int16_t status = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));
    if (0 > status)
    {
        return 0;
    }

    return 1;
}

/**************************************************************************/
/*!
    Writes an 8-bit value that sets the state of the PN532's GPIO pins

    @warning This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.

             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532::writeGPIO(uint8_t pinstate)
{
    // Make sure pinstate does not try to toggle P32 or P34
    pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);

    // Fill command buffer
    pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
    pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate; // P3 Pins
    pn532_packetbuffer[2] = 0x00; // P7 GPIO Pins (not used ... taken by I2C)

    NRF_LOG_DEBUG("Writing P3 GPIO: ");
    NRF_LOG_DEBUG("0xX%", pn532_packetbuffer[1]);
    NRF_LOG_DEBUG("\n");

    // Send the WRITEGPIO command (0x0E)
    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 3))
        return 0;

    return (0 < m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}

/**************************************************************************/
/*!
    Reads the state of the PN532's GPIO pins

    @returns An 8-bit value containing the pin state where:

             pinState[0]  = P30
             pinState[1]  = P31
             pinState[2]  = P32
             pinState[3]  = P33
             pinState[4]  = P34
             pinState[5]  = P35
*/
/**************************************************************************/
uint8_t PN532::readGPIO(void)
{
    pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

    // Send the READGPIO command (0x0C)
    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 1))
        return 0x0;

    m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* READGPIO response without prefix and suffix should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              P3 GPIO Pins
      b1              P7 GPIO Pins (not used ... taken by I2C)
      b2              Interface Mode Pins (not used ... bus select pins)
    */


    NRF_LOG_DEBUG("P3 GPIO: 0x%X", pn532_packetbuffer[7]);
    NRF_LOG_DEBUG("P7 GPIO: 0x%X", pn532_packetbuffer[8]);
    NRF_LOG_DEBUG("I0I1 GPIO: 0x%X", pn532_packetbuffer[9]);
    NRF_LOG_DEBUG("\n");

    return pn532_packetbuffer[0];
}

/**************************************************************************/
/*!
    @brief  Configures the SAM (Secure Access Module)
*/
/**************************************************************************/
bool PN532::SAMConfig(void)
{
    pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
    pn532_packetbuffer[1] = 0x01; // normal mode;
    pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
    pn532_packetbuffer[3] = 0x01; // use IRQ pin!

    NRF_LOG_DEBUG("SAMConfig\n");

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, COMMAND_SAMCONFIGURATION_LENGTH))
        return false;

    return (0 < m_pn532_hal->readResponse(pn532_packetbuffer, REPLY_SAMCONFIGURATION_LENGTH));
}

/**************************************************************************/
/*!
    Sets the MxRtyPassiveActivation uint8_t of the RFConfiguration register

    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532::setPassiveActivationRetries(uint8_t maxRetries)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
    pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
    pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
    pn532_packetbuffer[4] = maxRetries;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 5))
        return 0x0; // no ACK

    return (0 < m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}

/**************************************************************************/
/*!
    Sets the RFon/off uint8_t of the RFConfiguration register

    @param  autoRFCA    0x00 No check of the external field before
                        activation

                        0x02 Check the external field before
                        activation

    @param  rFOnOff     0x00 Switch the RF field off, 0x01 switch the RF
                        field on

    @returns    1 if everything executed properly, 0 for an error
*/
/**************************************************************************/

bool PN532::setRFField(uint8_t autoRFCA, uint8_t rFOnOff)
{
    pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = 0x00 | autoRFCA | rFOnOff;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 3))
    {
        return 0x0; // command failed
    }

    return (0 < m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer)));
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*!
    Waits for an ISO14443A target to enter the field

    @param  cardBaudRate  Baud rate of the caSrd
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
bool PN532::readPassiveTargetID(
    uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength, uint16_t timeout)
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1; // max 1 cards at once (we can set this to 2 later)
    pn532_packetbuffer[2] = cardbaudrate;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, COMMAND_INLISTPASSIVETARGET_BASE_LENGTH))
    {
        return 0x0; // command failed
    }

    // read data packet
    if (m_pn532_hal->readResponse(
            pn532_packetbuffer, REPLY_INLISTPASSIVETARGET_106A_TARGET_LENGTH, timeout) < 0)
    {
        return 0x0;
    }

    // check some basic stuff
    /* ISO14443A card response should be in the following format:

      byte            Description
      -------------   ------------------------------------------
      b0              Tags Found
      b1              Tag Number (only one used in this example)
      b2..3           SENS_RES
      b4              SEL_RES
      b5              NFCID Length
      b6..NFCIDLen    NFCID
    */


    uint8_t response_data[pn532_packetbuffer[3]];
    memcpy(response_data, &pn532_packetbuffer[PN532_DATA_OFFSET + 1], pn532_packetbuffer[3]);

    NRF_LOG_HEXDUMP_DEBUG(response_data, sizeof(response_data));

    if (response_data[0] != 1)
        return 0;

    uint16_t sens_res = response_data[2];
    sens_res <<= 8;
    sens_res |= response_data[3];

    NRF_LOG_DEBUG("ATQA: 0x%X", sens_res);
    NRF_LOG_DEBUG("SAK: 0x%X", response_data[4]);
    NRF_LOG_DEBUG("\n");

    /* Card appears to be Mifare Classic */
    *uidLength = response_data[5];

    for (uint8_t i = 0; i < response_data[5]; i++)
    {
        uid[i] = response_data[6 + i];
    }

    return 1;
}


/***** Mifare Classic Functions ******/

/**************************************************************************/
/*!
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
*/
/**************************************************************************/
bool PN532::mifareclassic_IsFirstBlock(uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock) % 4 == 0);
    else
        return ((uiBlock) % 16 == 0);
}

/**************************************************************************/
/*!
      Indicates whether the specified block number is the sector trailer
*/
/**************************************************************************/
bool PN532::mifareclassic_IsTrailerBlock(uint32_t uiBlock)
{
    // Test if we are in the small or big sectors
    if (uiBlock < 128)
        return ((uiBlock + 1) % 4 == 0);
    else
        return ((uiBlock + 1) % 16 == 0);
}

/**************************************************************************/
/*!
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 bytes
                          key value

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareclassic_AuthenticateBlock(
    uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData)
{
    uint8_t i;

    // Hang on to the key and uid data
    memcpy(_key, keyData, 6);
    memcpy(_uid, uid, uidLen);
    _uidLen = uidLen;

    // Prepare the authentication command //
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE; /* Data Exchange Header */
    pn532_packetbuffer[1] = 1;                            /* Max card numbers */
    pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
    pn532_packetbuffer[3] = blockNumber; /* Block Number (1K = 0..63, 4K = 0..255 */
    memcpy(pn532_packetbuffer + 4, _key, 6);
    for (i = 0; i < _uidLen; i++)
    {
        pn532_packetbuffer[10 + i] = _uid[i]; /* 4 bytes card ID */
    }

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 10 + _uidLen))
        return 0;

    // Read the response packet
    m_pn532_hal->readResponse(pn532_packetbuffer, REPLY_INDATAEXCHANGE_BASE_LENGTH);

    // Check if the response is valid and we are authenticated???
    // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
    // Mifare auth error is technically byte 7: 0x14 but anything other than 0x00 is not good
    if (pn532_packetbuffer[PN532_DATA_OFFSET + 1] != 0x00)
    {
        NRF_LOG_DEBUG("Authentification failed\n");
        return 0;
    }

    return 1;
}

/**************************************************************************/
/*!
    Tries to read an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareclassic_ReadDataBlock(uint8_t blockNumber, uint8_t * data)
{
    NRF_LOG_DEBUG("Trying to read 16 bytes from block %d", blockNumber);

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;               /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ; /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = blockNumber;     /* Block Number (0..63 for 1K, 0..255 for 4K) */

    /* Send the command */
    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 4))
    {
        return 0;
    }

    /* Read the response packet */
    m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If byte 8 isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[PN532_DATA_OFFSET + 1] != 0x00)
    {
        return 0;
    }

    /* Copy the 16 data bytes to the output buffer        */
    /* Block content starts at byte 9 of a valid response */
    memcpy(&data[0], &pn532_packetbuffer[9 - 1], 16);

    // NRF_LOG_DEBUG("Data of block %d:", blockNumber);
    // NRF_LOG_HEXDUMP_DEBUG(data, 16);

    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 16-bytes data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareclassic_WriteDataBlock(uint8_t blockNumber, uint8_t * data)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE; /* Mifare Write command = 0xA0 */
    pn532_packetbuffer[3] = blockNumber;      /* Block Number (0..63 for 1K, 0..255 for 4K) */
    memcpy(pn532_packetbuffer + 4, data, 16); /* Data Payload */
    int8_t retcode;

    /* Send the command */
    retcode = m_pn532_hal->writeCommand(pn532_packetbuffer, 20);
    if (retcode != 0)
    {
        // error
        return 0;
    }

    /* Read the response packet */
    retcode = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));
    if (retcode != 0)
    {
        // error
        return (0);
    }

    return (1);
}

/**************************************************************************/
/*!
    Formats a Mifare Classic card to store NDEF Records

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareclassic_FormatNDEF(void)
{
    uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03,
        0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03,
        0xE1, 0x03, 0xE1, 0x03, 0xE1};
    uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
    // for the MAD sector in NDEF records (sector 0)

    // Write block 1 and 2 to the card
    if (!(mifareclassic_WriteDataBlock(1, sectorbuffer1)))
        return 0;
    if (!(mifareclassic_WriteDataBlock(2, sectorbuffer2)))
        return 0;
    // Write key A and access rights card
    if (!(mifareclassic_WriteDataBlock(3, sectorbuffer3)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/**************************************************************************/
/*!
    Writes an NDEF URI Record to the specified sector (1..15)

    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 =
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareclassic_WriteNDEFURI(
    uint8_t sectorNumber, uint8_t uriIdentifier, const char * url)
{
    // Figure out how long the string is
    uint8_t len = strlen(url);

    // Make sure we're within a 1K limit for the sector number
    if ((sectorNumber < 1) || (sectorNumber > 15))
        return 0;

    // Make sure the URI payload is between 1 and 38 chars
    if ((len < 1) || (len > 38))
        return 0;

    // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
    // in NDEF records

    // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
    uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, len + 5, 0xD1, 0x01, len + 1, 0x55,
        uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF,
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    if (len <= 6)
    {
        // Unlikely we'll get a url this short, but why not ...
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer1[len + 9] = 0xFE;
    }
    else if (len == 7)
    {
        // 0xFE needs to be wrapped around to next block
        memcpy(sectorbuffer1 + 9, url, len);
        sectorbuffer2[0] = 0xFE;
    }
    else if ((len > 7) && (len <= 22))
    {
        // Url fits in two blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer2[len - 7] = 0xFE;
    }
    else if (len == 23)
    {
        // 0xFE needs to be wrapped around to final block
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, len - 7);
        sectorbuffer3[0] = 0xFE;
    }
    else
    {
        // Url fits in three blocks
        memcpy(sectorbuffer1 + 9, url, 7);
        memcpy(sectorbuffer2, url + 7, 16);
        memcpy(sectorbuffer3, url + 23, len - 23);
        sectorbuffer3[len - 23] = 0xFE;
    }

    // Now write all three blocks back to the card
    if (!(mifareclassic_WriteDataBlock(sectorNumber * 4, sectorbuffer1)))
        return 0;
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 1, sectorbuffer2)))
        return 0;
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 2, sectorbuffer3)))
        return 0;
    if (!(mifareclassic_WriteDataBlock((sectorNumber * 4) + 3, sectorbuffer4)))
        return 0;

    // Seems that everything was OK (?!)
    return 1;
}

/***** Mifare Ultralight Functions ******/

/**************************************************************************/
/*!
    Tries to read an entire 4-bytes page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
/**************************************************************************/
uint8_t PN532::mifareultralight_ReadPage(uint8_t page, uint8_t * buffer)
{
    if (page >= 64)
    {
        NRF_LOG_DEBUG("Page value out of range\n");
        return 0;
    }

    /* Prepare the command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;               /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_READ; /* Mifare Read command = 0x30 */
    pn532_packetbuffer[3] = page;            /* Page Number (0..63 in most cases) */

    /* Send the command */
    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 4))
    {
        return 0;
    }

    /* Read the response packet */
    m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));

    /* If statuy byte (byte 8) isn't 0x00 we probably have an error */
    if (pn532_packetbuffer[PN532_DATA_OFFSET + 1] == 0x00)
    {
        /* Copy the 4 data bytes to the output buffer         */
        /* Block content starts at byte 9 of a valid response */
        /* Note that the command actually reads 16 bytes or 4  */
        /* pages at a time ... we simply discard the last 12  */
        /* bytes                                              */
        memcpy(buffer, pn532_packetbuffer + PN532_DATA_OFFSET + 2, 4);
    }
    else
    {
        return 0;
    }

    // Return OK signal
    return 1;
}

/**************************************************************************/
/*!
    Tries to write an entire 4-bytes data buffer at the specified page
    address.

    @param  page     The page number to write into.  (0..63).
    @param  buffer   The byte array that contains the data to write.

    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t PN532::mifareultralight_WritePage(uint8_t page, uint8_t * buffer)
{
    /* Prepare the first command */
    pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = 1;                           /* Card number */
    pn532_packetbuffer[2] = MIFARE_CMD_WRITE_ULTRALIGHT; /* Mifare UL Write cmd = 0xA2 */
    pn532_packetbuffer[3] = page;                        /* page Number (0..63) */
    memcpy(pn532_packetbuffer + 4, buffer, 4);           /* Data Payload */
    int8_t retcode;

    /* Send the command */
    retcode = m_pn532_hal->writeCommand(pn532_packetbuffer, 8);
    if (retcode)
    {
        // error
        return 0;
    }

    /* Read the response packet */
    retcode = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));
    if (retcode != 0)
    {
        // error
        return (0);
    }

    return (1);
}

/**************************************************************************/
/*!
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
bool PN532::inDataExchange(
    uint8_t * send, uint8_t sendLength, uint8_t * response, uint8_t * responseLength)
{
    uint8_t i;

    pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = inListedTag;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 2, send, sendLength))
    {
        return false;
    }

    int16_t status = m_pn532_hal->readResponse(response, *responseLength, 1000);
    if (status < 0)
    {
        return false;
    }

    if ((response[0] & 0x3f) != 0)
    {
        NRF_LOG_DEBUG("Status code indicates an error\n");
        return false;
    }

    uint8_t length = status;
    length -= 1;

    if (length > *responseLength)
    {
        length = *responseLength; // silent truncation...
    }

    for (uint8_t i = 0; i < length; i++)
    {
        response[i] = response[i + 1];
    }
    *responseLength = length;

    return true;
}

/**************************************************************************/
/*!
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
/**************************************************************************/
bool PN532::inListPassiveTarget()
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = 0;

    NRF_LOG_DEBUG("inList passive target\n");

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 3))
    {
        return false;
    }

    int16_t status =
        m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 30000);
    if (status < 0)
    {
        return false;
    }

    if (pn532_packetbuffer[0] != 1)
    {
        return false;
    }

    inListedTag = pn532_packetbuffer[1];

    return true;
}

int8_t PN532::tgInitAsTarget(const uint8_t * command, const uint8_t len, const uint16_t timeout)
{

    int8_t status = m_pn532_hal->writeCommand(command, len);
    if (status < 0)
    {
        return -1;
    }

    status = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), timeout);
    if (status > 0)
    {
        return 1;
    }
    else if (PN532_TIMEOUT == status)
    {
        return 0;
    }
    else
    {
        return -2;
    }
}

/**
 * Peer to Peer
 */
int8_t PN532::tgInitAsTarget(uint16_t timeout)
{
    const uint8_t command[] = {
        PN532_COMMAND_TGINITASTARGET, 0, 0x00, 0x00, // SENS_RES
        0x00, 0x00, 0x00,                            // NFCID1
        0x40,                                        // SEL_RES

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, // POL_RES
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF,

        0x01, 0xFE, 0x0F, 0xBB, 0xBA, 0xA6, 0xC9, 0x89, 0x00,
        0x00, // NFCID3t: Change this to desired value

        0x06, 0x46, 0x66, 0x6D, 0x01, 0x01, 0x10, 0x00 // LLCP magic number and version parameter
    };
    return tgInitAsTarget(command, sizeof(command), timeout);
}

int16_t PN532::tgGetData(uint8_t * buf, uint8_t len)
{
    buf[0] = PN532_COMMAND_TGGETDATA;

    if (m_pn532_hal->writeCommand(buf, 1))
    {
        return -1;
    }

    int16_t status = m_pn532_hal->readResponse(buf, len, 3000);
    if (0 >= status)
    {
        return status;
    }

    uint16_t length = status - 1;


    if (buf[0] != 0)
    {
        NRF_LOG_DEBUG("status is not ok\n");
        return -5;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        buf[i] = buf[i + 1];
    }

    return length;
}

bool PN532::tgSetData(const uint8_t * header, uint8_t hlen, const uint8_t * body, uint8_t blen)
{
    if (hlen > (sizeof(pn532_packetbuffer) - 1))
    {
        if ((body != 0) || (header == pn532_packetbuffer))
        {
            NRF_LOG_DEBUG("tgSetData:buffer too small\n");
            return false;
        }

        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;
        if (m_pn532_hal->writeCommand(pn532_packetbuffer, 1, header, hlen))
        {
            return false;
        }
    }
    else
    {
        for (int8_t i = hlen - 1; i >= 0; i--)
        {
            pn532_packetbuffer[i + 1] = header[i];
        }
        pn532_packetbuffer[0] = PN532_COMMAND_TGSETDATA;

        if (m_pn532_hal->writeCommand(pn532_packetbuffer, hlen + 1, body, blen))
        {
            return false;
        }
    }

    if (0 > m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 3000))
    {
        return false;
    }

    if (0 != pn532_packetbuffer[0])
    {
        return false;
    }

    return true;
}

int16_t PN532::inRelease(const uint8_t relevantTarget)
{

    pn532_packetbuffer[0] = PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = relevantTarget;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 2))
    {
        return 0;
    }

    // read data packet
    return m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer));
}


/***** FeliCa Functions ******/
/**************************************************************************/
/*!
    @brief  Poll FeliCa card. PN532 acting as reader/initiator,
            peer acting as card/responder.
    @param[in]  systemCode             Designation of System Code. When sending FFFFh as System
   Code, all FeliCa cards can return response.
    @param[in]  requestCode            Designation of Request Data as follows:
                                         00h: No Request
                                         01h: System Code request (to acquire System Code of the
   card) 02h: Communication perfomance request
    @param[out] idm                    IDm of the card (8 bytes)
    @param[out] pmm                    PMm of the card (8 bytes)
    @param[out] systemCodeResponse     System Code of the card (Optional, 2bytes)
    @return                            = 1: A FeliCa card has detected
                                       = 0: No card has detected
                                       < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_Polling(uint16_t systemCode, uint8_t requestCode, uint8_t * idm, uint8_t * pmm,
    uint16_t * systemCodeResponse, uint16_t timeout)
{
    pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
    pn532_packetbuffer[1] = 1;
    pn532_packetbuffer[2] = 1;
    pn532_packetbuffer[3] = FELICA_CMD_POLLING;
    pn532_packetbuffer[4] = (systemCode >> 8) & 0xFF;
    pn532_packetbuffer[5] = systemCode & 0xFF;
    pn532_packetbuffer[6] = requestCode;
    pn532_packetbuffer[7] = 0;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 8))
    {
        NRF_LOG_DEBUG("Could not send Polling command\n");
        return -1;
    }

    int16_t status = m_pn532_hal->readResponse(pn532_packetbuffer, 22, timeout);
    if (status < 0)
    {
        NRF_LOG_DEBUG("Could not receive response\n");
        return -2;
    }

    // Check NbTg (pn532_packetbuffer[7])
    if (pn532_packetbuffer[0] == 0)
    {
        NRF_LOG_DEBUG("No card had detected\n");
        return 0;
    }
    else if (pn532_packetbuffer[0] != 1)
    {
        NRF_LOG_DEBUG("Unhandled number of targets inlisted. NbTg: 0x%X", pn532_packetbuffer[7]);
        NRF_LOG_DEBUG("\n");
        return -3;
    }

    inListedTag = pn532_packetbuffer[1];
    NRF_LOG_DEBUG("Tag number: 0x%X", pn532_packetbuffer[1]);
    NRF_LOG_DEBUG("\n");

    // length check
    uint8_t responseLength = pn532_packetbuffer[2];
    if (responseLength != 18 && responseLength != 20)
    {
        NRF_LOG_DEBUG("Wrong response length\n");
        return -4;
    }

    uint8_t i;
    for (i = 0; i < 8; ++i)
    {
        idm[i] = pn532_packetbuffer[4 + i];
        _felicaIDm[i] = pn532_packetbuffer[4 + i];
        pmm[i] = pn532_packetbuffer[12 + i];
        _felicaPMm[i] = pn532_packetbuffer[12 + i];
    }

    if (responseLength == 20)
    {
        *systemCodeResponse = (uint16_t)((pn532_packetbuffer[20] << 8) + pn532_packetbuffer[21]);
    }

    return 1;
}

/**************************************************************************/
/*!
    @brief  Sends FeliCa command to the currently inlisted peer

    @param[in]  command         FeliCa command packet. (e.g. 00 FF FF 00 00  for Polling command)
    @param[in]  commandlength   Length of the FeliCa command packet. (e.g. 0x05 for above Polling
   command )
    @param[out] response        FeliCa response packet. (e.g. 01 NFCID2(8 bytes) PAD(8 bytes)  for
   Polling response)
    @param[out] responselength  Length of the FeliCa response packet. (e.g. 0x11 for above Polling
   command )
    @return                          = 1: Success
                                     < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_SendCommand(
    const uint8_t * command, uint8_t commandlength, uint8_t * response, uint8_t * responseLength)
{
    if (commandlength > 0xFE)
    {
        NRF_LOG_DEBUG("Command length too long\n");
        return -1;
    }

    pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
    pn532_packetbuffer[1] = inListedTag;
    pn532_packetbuffer[2] = commandlength + 1;

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 3, command, commandlength))
    {
        NRF_LOG_DEBUG("Could not send FeliCa command\n");
        return -2;
    }

    // Wait card response
    int16_t status = m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 200);
    if (status < 0)
    {
        NRF_LOG_DEBUG("Could not receive response\n");
        return -3;
    }

    // Check status (pn532_packetbuffer[0])
    if ((pn532_packetbuffer[0] & 0x3F) != 0)
    {
        NRF_LOG_DEBUG("Status code indicates an error: 0x%X", pn532_packetbuffer[0]);
        NRF_LOG_DEBUG("\n");
        return -4;
    }

    // length check
    *responseLength = pn532_packetbuffer[1] - 1;
    if ((status - 2) != *responseLength)
    {
        NRF_LOG_DEBUG("Wrong response length\n");
        return -5;
    }

    memcpy(response, &pn532_packetbuffer[2], *responseLength);

    return 1;
}


/**************************************************************************/
/*!
    @brief  Sends FeliCa Request Service command

    @param[in]  numNode           length of the nodeCodeList
    @param[in]  nodeCodeList      Node codes(Big Endian)
    @param[out] keyVersions       Key Version of each Node (Big Endian)
    @return                          = 1: Success
                                     < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_RequestService(
    uint8_t numNode, uint16_t * nodeCodeList, uint16_t * keyVersions)
{
    if (numNode > FELICA_REQ_SERVICE_MAX_NODE_NUM)
    {
        NRF_LOG_DEBUG("numNode is too large\n");
        return -1;
    }

    uint8_t i, j = 0;
    uint8_t cmdLen = 1 + 8 + 1 + 2 * numNode;
    uint8_t cmd[cmdLen];
    cmd[j++] = FELICA_CMD_REQUEST_SERVICE;
    for (i = 0; i < 8; ++i)
    {
        cmd[j++] = _felicaIDm[i];
    }
    cmd[j++] = numNode;
    for (i = 0; i < numNode; ++i)
    {
        cmd[j++] = nodeCodeList[i] & 0xFF;
        cmd[j++] = (nodeCodeList[i] >> 8) & 0xff;
    }

    uint8_t response[10 + 2 * numNode];
    uint8_t responseLength;

    if (felica_SendCommand(cmd, cmdLen, response, &responseLength) != 1)
    {
        NRF_LOG_DEBUG("Request Service command failed\n");
        return -2;
    }

    // length check
    if (responseLength != 10 + 2 * numNode)
    {
        NRF_LOG_DEBUG("Request Service command failed (wrong response length)\n");
        return -3;
    }

    for (i = 0; i < numNode; i++)
    {
        keyVersions[i] = (uint16_t)(response[10 + i * 2] + (response[10 + i * 2 + 1] << 8));
    }
    return 1;
}


/**************************************************************************/
/*!
    @brief  Sends FeliCa Request Service command

    @param[out]  mode         Current Mode of the card
    @return                   = 1: Success
                              < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_RequestResponse(uint8_t * mode)
{
    uint8_t cmd[9];
    cmd[0] = FELICA_CMD_REQUEST_RESPONSE;
    memcpy(&cmd[1], _felicaIDm, 8);

    uint8_t response[10];
    uint8_t responseLength;
    if (felica_SendCommand(cmd, 9, response, &responseLength) != 1)
    {
        NRF_LOG_DEBUG("Request Response command failed\n");
        return -1;
    }

    // length check
    if (responseLength != 10)
    {
        NRF_LOG_DEBUG("Request Response command failed (wrong response length)\n");
        return -2;
    }

    *mode = response[9];
    return 1;
}

/**************************************************************************/
/*!
    @brief  Sends FeliCa Read Without Encryption command

    @param[in]  numService         Length of the serviceCodeList
    @param[in]  serviceCodeList    Service Code List (Big Endian)
    @param[in]  numBlock           Length of the blockList
    @param[in]  blockList          Block List (Big Endian, This API only accepts 2-byte block list
   element)
    @param[out] blockData          Block Data
    @return                        = 1: Success
                                   < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_ReadWithoutEncryption(uint8_t numService, const uint16_t * serviceCodeList,
    uint8_t numBlock, const uint16_t * blockList, uint8_t blockData[][16])
{
    if (numService > FELICA_READ_MAX_SERVICE_NUM)
    {
        NRF_LOG_DEBUG("numService is too large\n");
        return -1;
    }
    if (numBlock > FELICA_READ_MAX_BLOCK_NUM)
    {
        NRF_LOG_DEBUG("numBlock is too large\n");
        return -2;
    }

    uint8_t i, j = 0, k;
    uint8_t cmdLen = 1 + 8 + 1 + 2 * numService + 1 + 2 * numBlock;
    uint8_t cmd[cmdLen];
    cmd[j++] = FELICA_CMD_READ_WITHOUT_ENCRYPTION;
    for (i = 0; i < 8; ++i)
    {
        cmd[j++] = _felicaIDm[i];
    }
    cmd[j++] = numService;
    for (i = 0; i < numService; ++i)
    {
        cmd[j++] = serviceCodeList[i] & 0xFF;
        cmd[j++] = (serviceCodeList[i] >> 8) & 0xff;
    }
    cmd[j++] = numBlock;
    for (i = 0; i < numBlock; ++i)
    {
        cmd[j++] = (blockList[i] >> 8) & 0xFF;
        cmd[j++] = blockList[i] & 0xff;
    }

    uint8_t response[12 + 16 * numBlock];
    uint8_t responseLength;
    if (felica_SendCommand(cmd, cmdLen, response, &responseLength) != 1)
    {
        NRF_LOG_DEBUG("Read Without Encryption command failed\n");
        return -3;
    }

    // length check
    if (responseLength != 12 + 16 * numBlock)
    {
        NRF_LOG_DEBUG("Read Without Encryption command failed (wrong response length)\n");
        return -4;
    }

    // status flag check
    if (response[9] != 0 || response[10] != 0)
    {
        NRF_LOG_DEBUG("Read Without Encryption command failed (Status Flag: 0x%X 0x%X)",
            pn532_packetbuffer[9], pn532_packetbuffer[10]);
        NRF_LOG_DEBUG(")\n");
        return -5;
    }

    k = 12;
    for (i = 0; i < numBlock; i++)
    {
        for (j = 0; j < 16; j++)
        {
            blockData[i][j] = response[k++];
        }
    }

    return 1;
}


/**************************************************************************/
/*!
    @brief  Sends FeliCa Write Without Encryption command

    @param[in]  numService         Length of the serviceCodeList
    @param[in]  serviceCodeList    Service Code List (Big Endian)
    @param[in]  numBlock           Length of the blockList
    @param[in]  blockList          Block List (Big Endian, This API only accepts 2-byte block list
   element)
    @param[in]  blockData          Block Data (each Block has 16 bytes)
    @return                        = 1: Success
                                   < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_WriteWithoutEncryption(uint8_t numService, const uint16_t * serviceCodeList,
    uint8_t numBlock, const uint16_t * blockList, uint8_t blockData[][16])
{
    if (numService > FELICA_WRITE_MAX_SERVICE_NUM)
    {
        NRF_LOG_DEBUG("numService is too large\n");
        return -1;
    }
    if (numBlock > FELICA_WRITE_MAX_BLOCK_NUM)
    {
        NRF_LOG_DEBUG("numBlock is too large\n");
        return -2;
    }

    uint8_t i, j = 0, k;
    uint8_t cmdLen = 1 + 8 + 1 + 2 * numService + 1 + 2 * numBlock + 16 * numBlock;
    uint8_t cmd[cmdLen];
    cmd[j++] = FELICA_CMD_WRITE_WITHOUT_ENCRYPTION;
    for (i = 0; i < 8; ++i)
    {
        cmd[j++] = _felicaIDm[i];
    }
    cmd[j++] = numService;
    for (i = 0; i < numService; ++i)
    {
        cmd[j++] = serviceCodeList[i] & 0xFF;
        cmd[j++] = (serviceCodeList[i] >> 8) & 0xff;
    }
    cmd[j++] = numBlock;
    for (i = 0; i < numBlock; ++i)
    {
        cmd[j++] = (blockList[i] >> 8) & 0xFF;
        cmd[j++] = blockList[i] & 0xff;
    }
    for (i = 0; i < numBlock; ++i)
    {
        for (k = 0; k < 16; k++)
        {
            cmd[j++] = blockData[i][k];
        }
    }

    uint8_t response[11];
    uint8_t responseLength;
    if (felica_SendCommand(cmd, cmdLen, response, &responseLength) != 1)
    {
        NRF_LOG_DEBUG("Write Without Encryption command failed\n");
        return -3;
    }

    // length check
    if (responseLength != 11)
    {
        NRF_LOG_DEBUG("Write Without Encryption command failed (wrong response length)\n");
        return -4;
    }

    // status flag check
    if (response[9] != 0 || response[10] != 0)
    {
        NRF_LOG_DEBUG("Write Without Encryption command failed (Status Flag: 0x%X 0x%X)",
            pn532_packetbuffer[9], pn532_packetbuffer[10]);
        NRF_LOG_DEBUG(")\n");
        return -5;
    }

    return 1;
}

/**************************************************************************/
/*!
    @brief  Sends FeliCa Request System Code command

    @param[out] numSystemCode        Length of the systemCodeList
    @param[out] systemCodeList       System Code list (Array length should longer than 16)
    @return                          = 1: Success
                                     < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_RequestSystemCode(uint8_t * numSystemCode, uint16_t * systemCodeList)
{
    uint8_t cmd[9];
    cmd[0] = FELICA_CMD_REQUEST_SYSTEM_CODE;
    memcpy(&cmd[1], _felicaIDm, 8);

    uint8_t response[10 + 2 * 16];
    uint8_t responseLength;
    if (felica_SendCommand(cmd, 9, response, &responseLength) != 1)
    {
        NRF_LOG_DEBUG("Request System Code command failed\n");
        return -1;
    }
    *numSystemCode = response[9];

    // length check
    if (responseLength < 10 + 2 * *numSystemCode)
    {
        NRF_LOG_DEBUG("Request System Code command failed (wrong response length)\n");
        return -2;
    }

    uint8_t i;
    for (i = 0; i < *numSystemCode; i++)
    {
        systemCodeList[i] = (uint16_t)((response[10 + i * 2] << 8) + response[10 + i * 2 + 1]);
    }

    return 1;
}


/**************************************************************************/
/*!
    @brief  Release FeliCa card
    @return                          = 1: Success
                                     < 0: error
*/
/**************************************************************************/
int8_t PN532::felica_Release()
{
    // InRelease
    pn532_packetbuffer[0] = PN532_COMMAND_INRELEASE;
    pn532_packetbuffer[1] = 0x00; // All target
    NRF_LOG_DEBUG("Release all FeliCa target\n");

    if (m_pn532_hal->writeCommand(pn532_packetbuffer, 2))
    {
        NRF_LOG_DEBUG("No ACK\n");
        return -1; // no ACK
    }

    // Wait card response
    int16_t frameLength =
        m_pn532_hal->readResponse(pn532_packetbuffer, sizeof(pn532_packetbuffer), 1000);
    if (frameLength < 0)
    {
        NRF_LOG_DEBUG("Could not receive response\n");
        return -2;
    }

    // Check status (pn532_packetbuffer[0])
    if ((pn532_packetbuffer[0] & 0x3F) != 0)
    {
        NRF_LOG_DEBUG("Status code indicates an error: 0x%X", pn532_packetbuffer[7]);
        NRF_LOG_DEBUG("\n");
        return -3;
    }

    return 1;
}