#include <MifareUltralight.h>

#define ULTRALIGHT_PAGE_SIZE 4
#define ULTRALIGHT_READ_SIZE 4 // we should be able to read 16 bytes at a time

#define ULTRALIGHT_DATA_START_PAGE 4
#define ULTRALIGHT_MESSAGE_LENGTH_INDEX 1
#define ULTRALIGHT_DATA_START_INDEX 2
#define ULTRALIGHT_MAX_PAGE 63

#define NFC_FORUM_TAG_TYPE_2 ("NFC Forum Type 2")

extern "C"
{
#include "nrf_delay.h"
#include "nrf_log.h"
}

MifareUltralight::MifareUltralight(PN532 & nfcShield)
{
    _nfcShield = &nfcShield;
    _ndefStartIndex = 0;
    _messageLength = 0;
}

MifareUltralight::~MifareUltralight() {}

NfcTag * MifareUltralight::read(byte * uid, unsigned int uidLength)
{
    if (isUnformatted())
    {
        // tag is not NDEF formatted
        NRF_LOG_INFO("Tag is not NDEF formatted.");
        return new NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, NFCTAG_NOT_NDEF_FORMATTED);
    }

    readCapabilityContainer(); // meta info for tag
    findNdefMessage();
    calculateBufferSize();

    if (_messageLength == 0)
    {
        // no NDEF message
        // data is 0x44 0x03 0x00 0xFE
        NdefMessage message = NdefMessage();
        message.addEmptyRecord();
        return new NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, message, NFCTAG_NO_ERROR);
    }

    // tag has NDEF message
    boolean success;
    uint8_t page;
    uint8_t index = 0;
    byte buffer[_bufferSize];
    for (page = ULTRALIGHT_DATA_START_PAGE; page < ULTRALIGHT_MAX_PAGE; page++)
    {
        // read the data
        success = _nfcShield->mifareultralight_ReadPage(page, &buffer[index]);
        if (success)
        {
            NRF_LOG_INFO("Page  %d", page);
            _nfcShield->PrintHex(&buffer[index], ULTRALIGHT_PAGE_SIZE);
        }
        else
        {
            NRF_LOG_INFO("Read failed %d", page);
            _messageLength = 0;
            return new NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, NFCTAG_READ_FAILED);
        }

        if (index >= (_messageLength + _ndefStartIndex))
        {
            break;
        }

        index += ULTRALIGHT_PAGE_SIZE;
    }

    NdefMessage ndefMessage = NdefMessage(&buffer[_ndefStartIndex], _messageLength);
    return new NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_2, ndefMessage, NFCTAG_NO_ERROR);
}

boolean MifareUltralight::isUnformatted()
{
    uint8_t page = 4;
    byte data[ULTRALIGHT_READ_SIZE];
    boolean success = _nfcShield->mifareultralight_ReadPage(page, data);
    if (success)
    {
        return (data[0] == 0xFF && data[1] == 0xFF && data[2] == 0xFF && data[3] == 0xFF);
    }
    else
    {
        NRF_LOG_INFO("Error. Failed read page %d", page);
        return false;
    }
}

// page 3 has tag capabilities
void MifareUltralight::readCapabilityContainer()
{
    byte data[ULTRALIGHT_PAGE_SIZE];
    int success = _nfcShield->mifareultralight_ReadPage(3, data);
    if (success)
    {
        // See AN1303 - different rules for Mifare Family byte2 = (additional data + 48)/8
        _tagCapacity = data[2] * 8;
        NRF_LOG_INFO("Tag capacity %d bytes", _tagCapacity);

        // TODO future versions should get lock information
    }
}

// read enough of the message to find the ndef message length
void MifareUltralight::findNdefMessage()
{
    int page;
    byte data[12]; // 3 pages
    byte * data_ptr = &data[0];

    // the nxp read command reads 4 pages, unfortunately adafruit give me one page at a time
    boolean success = true;
    for (page = 4; page < 6; page++)
    {
        success = success && _nfcShield->mifareultralight_ReadPage(page, data_ptr);
        NRF_LOG_INFO("Page %d", page);
        _nfcShield->PrintHex(data_ptr, 4);
        data_ptr += ULTRALIGHT_PAGE_SIZE;
    }

    if (success)
    {
        if (data[0] == 0x03)
        {
            _messageLength = data[1];
            _ndefStartIndex = 2;
        }
        else if (data[5] == 0x3) // page 5 byte 1
        {
            // TODO should really read the lock control TLV to ensure byte[5] is correct
            _messageLength = data[6];
            _ndefStartIndex = 7;
        }
    }

    NRF_LOG_INFO("messageLength %d", _messageLength);
    NRF_LOG_INFO("ndefStartIndex %d", _ndefStartIndex);
}

// buffer is larger than the message, need to handle some data before and after
// message and need to ensure we read full pages
void MifareUltralight::calculateBufferSize()
{
    // TLV terminator 0xFE is 1 byte
    _bufferSize = _messageLength + _ndefStartIndex + 1;

    if (_bufferSize % ULTRALIGHT_READ_SIZE != 0)
    {
        // buffer must be an increment of page size
        _bufferSize = ((_bufferSize / ULTRALIGHT_READ_SIZE) + 1) * ULTRALIGHT_READ_SIZE;
    }
}

boolean MifareUltralight::write(NdefMessage & m, byte * uid, unsigned int uidLength)
{
    if (isUnformatted())
    {
        NRF_LOG_INFO("Tag is not formatted.");
        return false;
    }
    readCapabilityContainer(); // meta info for tag

    _messageLength = m.getEncodedSize();
    _ndefStartIndex = _messageLength < 0xFF ? 2 : 4;
    calculateBufferSize();

    if (_bufferSize > _tagCapacity)
    {
        NRF_LOG_INFO("Encoded Message length exceeded tag Capacity %d", _tagCapacity);
        return false;
    }

    uint8_t encoded[_bufferSize];
    uint8_t * src = encoded;
    unsigned int position = 0;
    uint8_t page = ULTRALIGHT_DATA_START_PAGE;

    // Set message size. With ultralight should always be less than 0xFF but who knows?

    encoded[0] = 0x3;
    if (_messageLength < 0xFF)
    {
        encoded[1] = _messageLength;
    }
    else
    {
        encoded[1] = 0xFF;
        encoded[2] = ((_messageLength >> 8) & 0xFF);
        encoded[3] = (_messageLength & 0xFF);
    }

    m.encode(encoded + _ndefStartIndex);
    // this is always at least 1 byte copy because of terminator.
    memset(encoded + _ndefStartIndex + _messageLength, 0,
        _bufferSize - _ndefStartIndex - _messageLength);
    encoded[_ndefStartIndex + _messageLength] = 0xFE; // terminator

    NRF_LOG_INFO("messageLength %d", _messageLength);
    NRF_LOG_INFO("Tag Capacity %d", _tagCapacity);
    _nfcShield->PrintHex(encoded, _bufferSize);

    while (position < _bufferSize){ //bufferSize is always times pagesize so no "last chunk" check
        // write page
        if (!_nfcShield->mifareultralight_WritePage(page, src))
        {
            return false;
        }
        NRF_LOG_INFO("Wrote page %d", page);
        page++;
        src += ULTRALIGHT_PAGE_SIZE;
        position += ULTRALIGHT_PAGE_SIZE;
    }
    return true;
}

// Mifare Ultralight can't be reset to factory state
// zero out tag data like the NXP Tag Write Android application
boolean MifareUltralight::clean()
{
    readCapabilityContainer(); // meta info for tag

    uint8_t pages = (_tagCapacity / ULTRALIGHT_PAGE_SIZE) + ULTRALIGHT_DATA_START_PAGE;

    // factory tags have 0xFF, but OTP-CC blocks have already been set so we use 0x00
    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};

    for (int i = ULTRALIGHT_DATA_START_PAGE; i < pages; i++)
    {
        NRF_LOG_INFO("Wrote page %d", i);
        _nfcShield->PrintHex(data, ULTRALIGHT_PAGE_SIZE);
        if (!_nfcShield->mifareultralight_WritePage(i, data))
        {
            return false;
        }
    }
    return true;
}