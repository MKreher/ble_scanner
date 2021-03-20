#include "MifareClassic.h"

extern "C" {
  #include "nrf_log.h"
  #include "nrf_delay.h"
}

#define BLOCK_SIZE 16
#define LONG_TLV_SIZE 4
#define SHORT_TLV_SIZE 2

#define MIFARE_CLASSIC ("Mifare Classic")

MifareClassic::MifareClassic(PN532 & nfcShield) { _nfcShield = &nfcShield; }

MifareClassic::~MifareClassic()
{
}

NfcTag* MifareClassic::read(byte *uid, unsigned int uidLength)
{
    uint8_t default_keys[9][6] = {
                                    { 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF },
                                    { 0XD3, 0XF7, 0XD3, 0XF7, 0XD3, 0XF7 },
                                    { 0XA0, 0XA1, 0XA2, 0XA3, 0XA4, 0XA5 },
                                    { 0XB0, 0XB1, 0XB2, 0XB3, 0XB4, 0XB5 },
                                    { 0X4D, 0X3A, 0X99, 0XC3, 0X51, 0XDD },
                                    { 0X1A, 0X98, 0X2C, 0X7E, 0X45, 0X9A },
                                    { 0XAA, 0XBB, 0XCC, 0XDD, 0XEE, 0XFF },
                                    { 0X00, 0X00, 0X00, 0X00, 0X00, 0X00 },
                                    { 0XAB, 0XCD, 0XEF, 0X12, 0X34, 0X56 }
                                  };
    int key_idx = 1;
    int currentBlock = 4;
    int messageStartIndex = 0;
    int messageLength = 0;
    byte data[BLOCK_SIZE];
    uint8_t success = 0;

    // read first block to get message length
    success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, default_keys[key_idx]);
    if (success)
    {
        success = _nfcShield->mifareclassic_ReadDataBlock(currentBlock, data);
        if (success)
        {
            if (!decodeTlv(data, messageLength, messageStartIndex)) {
                return new NfcTag(uid, uidLength, NFCTAG_READ_FAILED);
            }
        }
        else
        {
            NRF_LOG_INFO("Error. Failed read block %d", currentBlock);
            return new NfcTag(uid, uidLength, MIFARE_CLASSIC, NFCTAG_READ_FAILED);
        }
    }
    else
    {
        NRF_LOG_INFO("Tag authentication failed.");
        return new NfcTag(uid, uidLength, MIFARE_CLASSIC, NFCTAG_AUTHENTICATION_FAILED);
    }

    // this should be nested in the message length loop
    int index = 0;
    int bufferSize = getBufferSize(messageLength);
    uint8_t buffer[bufferSize];

    NRF_LOG_INFO("Message Length %d", messageLength);
    NRF_LOG_INFO("Buffer Size %d", bufferSize);

    while (index < bufferSize)
    {

        // authenticate on every sector
        if (_nfcShield->mifareclassic_IsFirstBlock(currentBlock))
        {
            NRF_LOG_INFO("First data block %d.", currentBlock);

            success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, default_keys[key_idx]);
            
            if (!success)
            {
                NRF_LOG_INFO("Error. Block Authentication failed for %d", currentBlock);
                return new NfcTag(uid, uidLength, MIFARE_CLASSIC, NFCTAG_AUTHENTICATION_FAILED);
            }
        }

        // read the data
        success = _nfcShield->mifareclassic_ReadDataBlock(currentBlock, &buffer[index]);
        if (success)
        {
            NRF_LOG_INFO("Block %d:", currentBlock);
            _nfcShield->PrintHex(&buffer[index], BLOCK_SIZE);
        }
        else
        {
            NRF_LOG_INFO("Read failed %d", currentBlock);
            return new NfcTag(uid, uidLength, MIFARE_CLASSIC, NFCTAG_READ_FAILED);
        }

        index += BLOCK_SIZE;
        currentBlock++;

        // skip the trailer block
        if (_nfcShield->mifareclassic_IsTrailerBlock(currentBlock))
        {
            NRF_LOG_INFO("Skipping block %d", currentBlock);
            currentBlock++;
        }
    }
     
    return new NfcTag(uid, uidLength, MIFARE_CLASSIC, &buffer[messageStartIndex], messageLength, NFCTAG_NO_ERROR);
}

int MifareClassic::getBufferSize(int messageLength)
{

    int bufferSize = messageLength;

    // TLV header is 2 or 4 bytes, TLV terminator is 1 byte.
    if (messageLength < 0xFF)
    {
        bufferSize += SHORT_TLV_SIZE + 1;
    }
    else
    {
        bufferSize += LONG_TLV_SIZE + 1;
    }

    // bufferSize needs to be a multiple of BLOCK_SIZE
    if (bufferSize % BLOCK_SIZE != 0)
    {
        bufferSize = ((bufferSize / BLOCK_SIZE) + 1) * BLOCK_SIZE;
    }

    return bufferSize;
}

// skip null tlvs (0x0) before the real message
// technically unlimited null tlvs, but we assume
// T & L of TLV in the first block we read
int MifareClassic::getNdefStartIndex(byte *data)
{

    for (int i = 0; i < BLOCK_SIZE; i++)
    {
        if (data[i] == 0x0)
        {
            // do nothing, skip
        }
        else if (data[i] == 0x3)
        {
            return i;
        }
        else
        {
            NRF_LOG_INFO("Unknown TLV 0x%x", data[i]);
            return -2;
        }
    }

    return -1;
}

// Decode the NDEF data length from the Mifare TLV
// Leading null TLVs (0x0) are skipped
// Assuming T & L of TLV will be in the first block
// messageLength and messageStartIndex written to the parameters
// success or failure status is returned
//
// { 0x3, LENGTH }
// { 0x3, 0xFF, LENGTH, LENGTH }
bool MifareClassic::decodeTlv(byte *data, int &messageLength, int &messageStartIndex)
{
    int i = getNdefStartIndex(data);

    if (i < 0 || data[i] != 0x3)
    {
        NRF_LOG_INFO("Error. Can't decode message length.");
        return false;
    }
    else
    {
        if (data[i+1] == 0xFF)
        {
            messageLength = ((0xFF & data[i+2]) << 8) | (0xFF & data[i+3]);
            messageStartIndex = i + LONG_TLV_SIZE;
        }
        else
        {
            messageLength = data[i+1];
            messageStartIndex = i + SHORT_TLV_SIZE;
        }
    }

    return true;
}

// Intialized NDEF tag contains one empty NDEF TLV 03 00 FE - AN1304 6.3.1
// We are formatting in read/write mode with a NDEF TLV 03 03 and an empty NDEF record D0 00 00 FE - AN1304 6.3.2
boolean MifareClassic::formatNDEF(byte * uid, unsigned int uidLength)
{
    uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
    uint8_t emptyNdefMesg[16] = {0x03, 0x03, 0xD0, 0x00, 0x00, 0xFE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer0[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    boolean success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, 0, 0, keya);
    if (!success)
    {
        NRF_LOG_INFO("Unable to authenticate block 0 to enable card formatting!");
        return false;
    }
    success = _nfcShield->mifareclassic_FormatNDEF();
    if (!success)
    {
        NRF_LOG_INFO("Unable to format the card for NDEF");
    }
    else
    {
        for (int i=4; i<64; i+=4) {
            success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, i, 0, keya);

            if (success) {
                if (i == 4)  // special handling for block 4
                {
                    if (!(_nfcShield->mifareclassic_WriteDataBlock (i, emptyNdefMesg)))
                    {
                        NRF_LOG_INFO("Unable to write block %d", i);
                    }
                }
                else
                {
                    if (!(_nfcShield->mifareclassic_WriteDataBlock (i, sectorbuffer0)))
                    {
                        NRF_LOG_INFO("Unable to write block %d", i);
                    }
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+1, sectorbuffer0)))
                {
                    NRF_LOG_INFO("Unable to write block %d", i+1);
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+2, sectorbuffer0)))
                {
                    NRF_LOG_INFO("Unable to write block %d", i+2);
                }
                if (!(_nfcShield->mifareclassic_WriteDataBlock (i+3, sectorbuffer4)))
                {
                    NRF_LOG_INFO("Unable to write block %d", i+3);
                }
            } else {
                unsigned int iii=uidLength;
                NRF_LOG_INFO("Unable to authenticate block %d", i);
                _nfcShield->readPassiveTargetID(PN532_MIFARE_ISO14443A_BAUD, uid, (uint8_t*)&iii);
            }
        }
    }
    return success;
}

#define NR_SHORTSECTOR          (32)    // Number of short sectors on Mifare 1K/4K
#define NR_LONGSECTOR           (8)     // Number of long sectors on Mifare 4K
#define NR_BLOCK_OF_SHORTSECTOR (4)     // Number of blocks in a short sector
#define NR_BLOCK_OF_LONGSECTOR  (16)    // Number of blocks in a long sector

// Determine the sector trailer block based on sector number
#define BLOCK_NUMBER_OF_SECTOR_TRAILER(sector) (((sector)<NR_SHORTSECTOR)? \
  ((sector)*NR_BLOCK_OF_SHORTSECTOR + NR_BLOCK_OF_SHORTSECTOR-1):\
  (NR_SHORTSECTOR*NR_BLOCK_OF_SHORTSECTOR + (sector-NR_SHORTSECTOR)*NR_BLOCK_OF_LONGSECTOR + NR_BLOCK_OF_LONGSECTOR-1))

boolean MifareClassic::formatMifare(byte * uid, unsigned int uidLength)
{

    // The default Mifare Classic key
    uint8_t KEY_DEFAULT_KEYAB[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    uint8_t blockBuffer[16];                          // Buffer to store block contents
    uint8_t blankAccessBits[3] = { 0xff, 0x07, 0x80 };
    uint8_t idx = 0;
    uint8_t numOfSector = 16;                         // Assume Mifare Classic 1K for now (16 4-block sectors)
    boolean success = false;

    for (idx = 0; idx < numOfSector; idx++)
    {
        // Step 1: Authenticate the current sector using key B 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        success = _nfcShield->mifareclassic_AuthenticateBlock (uid, uidLength, BLOCK_NUMBER_OF_SECTOR_TRAILER(idx), 1, (uint8_t *)KEY_DEFAULT_KEYAB);
        if (!success)
        {
            NRF_LOG_INFO("Authentication failed for sector %d", idx);
            return false;
        }

        // Step 2: Write to the other blocks
        if (idx == 0)
        {
            memset(blockBuffer, 0, sizeof(blockBuffer));
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
            {
                NRF_LOG_INFO("Unable to write to sector %d", idx);
            }
        }
        else
        {
            memset(blockBuffer, 0, sizeof(blockBuffer));
            // this block has not to be overwritten for block 0. It contains Tag id and other unique data.
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 3, blockBuffer)))
            {
                NRF_LOG_INFO("Unable to write to sector %d", idx);
            }
            if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 2, blockBuffer)))
            {
                NRF_LOG_INFO("Unable to write to sector %d", idx);
            }
        }

        memset(blockBuffer, 0, sizeof(blockBuffer));

        if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)) - 1, blockBuffer)))
        {
            NRF_LOG_INFO("Unable to write to sector %d", idx);
        }

        // Step 3: Reset both keys to 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
        memcpy(blockBuffer, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));
        memcpy(blockBuffer + 6, blankAccessBits, sizeof(blankAccessBits));
        blockBuffer[9] = 0x69;
        memcpy(blockBuffer + 10, KEY_DEFAULT_KEYAB, sizeof(KEY_DEFAULT_KEYAB));

        // Step 4: Write the trailer block
        if (!(_nfcShield->mifareclassic_WriteDataBlock((BLOCK_NUMBER_OF_SECTOR_TRAILER(idx)), blockBuffer)))
        {
            NRF_LOG_INFO("Unable to write trailer block of sector %d", idx);
        }
    }
    return true;
}

boolean MifareClassic::write(NdefMessage& m, byte * uid, unsigned int uidLength)
{

    uint8_t encoded[m.getEncodedSize()];
    m.encode(encoded);

    uint8_t buffer[getBufferSize(sizeof(encoded))];
    memset(buffer, 0, sizeof(buffer));

    NRF_LOG_INFO("sizeof(encoded) %d", sizeof(encoded));
    NRF_LOG_INFO("sizeof(buffer) %d", sizeof(buffer));

    if (sizeof(encoded) < 0xFF)
    {
        buffer[0] = 0x3;
        buffer[1] = sizeof(encoded);
        memcpy(&buffer[2], encoded, sizeof(encoded));
        buffer[2+sizeof(encoded)] = 0xFE; // terminator
    }
    else
    {
        buffer[0] = 0x3;
        buffer[1] = 0xFF;
        buffer[2] = ((sizeof(encoded) >> 8) & 0xFF);
        buffer[3] = (sizeof(encoded) & 0xFF);
        memcpy(&buffer[4], encoded, sizeof(encoded));
        buffer[4+sizeof(encoded)] = 0xFE; // terminator
    }

    // Write to tag
    unsigned int index = 0;
    int currentBlock = 4;
    uint8_t key[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 }; // this is Sector 1 - 15 key

    while (index < sizeof(buffer))
    {

        if (_nfcShield->mifareclassic_IsFirstBlock(currentBlock))
        {
            int success = _nfcShield->mifareclassic_AuthenticateBlock(uid, uidLength, currentBlock, 0, key);
            if (!success)
            {
                NRF_LOG_INFO("Error. Block Authentication failed for %d", currentBlock);
                return false;
            }
        }

        int write_success = _nfcShield->mifareclassic_WriteDataBlock (currentBlock, &buffer[index]);
        if (write_success)
        {
            NRF_LOG_INFO("Wrote block %d", currentBlock);
            _nfcShield->PrintHex(&buffer[index], BLOCK_SIZE);
        }
        else
        {
            NRF_LOG_INFO("Write failed %d", currentBlock);
            return false;
        }
        index += BLOCK_SIZE;
        currentBlock++;

        if (_nfcShield->mifareclassic_IsTrailerBlock(currentBlock))
        {
            // can't write to trailer block
            NRF_LOG_INFO("Skipping block %d", currentBlock);
            currentBlock++;
        }

    }

    return true;
}