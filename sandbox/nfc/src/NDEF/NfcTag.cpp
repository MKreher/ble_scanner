#include <NfcTag.h>

extern "C" {
  #include "nrf_log.h"
}

NfcTag::NfcTag()
{
    _uid = 0;
    _uidLength = 0;
    _tagType = "Unknown";
    _ndefMessage = (NdefMessage*)NULL;
    _errorCode = 0;
}

NfcTag::NfcTag(byte *uid, unsigned int uidLength, uint8_t errorCode)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = "Unknown";
    _ndefMessage = (NdefMessage*)NULL;
    _errorCode = errorCode;
}

NfcTag::NfcTag(byte *uid, unsigned int  uidLength, String tagType, uint8_t errorCode)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = (NdefMessage*)NULL;
    _errorCode = errorCode;
}

NfcTag::NfcTag(byte *uid, unsigned int  uidLength, String tagType, NdefMessage& ndefMessage, uint8_t errorCode)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefMessage);
    _errorCode = errorCode;
}

// I don't like this version, but it will use less memory
NfcTag::NfcTag(byte *uid, unsigned int uidLength, String tagType, const byte *ndefData, const int ndefDataLength, uint8_t errorCode)
{
    _uid = uid;
    _uidLength = uidLength;
    _tagType = tagType;
    _ndefMessage = new NdefMessage(ndefData, ndefDataLength);
    _errorCode = errorCode;
}

NfcTag::~NfcTag()
{
    delete _ndefMessage;
}

NfcTag& NfcTag::operator=(const NfcTag& rhs)
{
    if (this != &rhs)
    {
        delete _ndefMessage;
        _uid = rhs._uid;
        _uidLength = rhs._uidLength;
        _tagType = rhs._tagType;
        // TODO do I need a copy here?
        _ndefMessage = rhs._ndefMessage;
    }
    return *this;
}

uint8_t NfcTag::getUidLength()
{
    return _uidLength;
}

void NfcTag::getUid(byte *uid, unsigned int uidLength)
{
    memcpy(uid, _uid, _uidLength < uidLength ? _uidLength : uidLength);
}

String NfcTag::getUidString()
{
    String uidString = "";
    for (unsigned int i = 0; i < _uidLength; i++)
    {
        if (i > 0)
        {
            uidString += " ";
        }

        if (_uid[i] < 0xF)
        {
            uidString += "0";
        }

        uidString += String((unsigned int)_uid[i], (unsigned char)HEX);
    }
    uidString.toUpperCase();
    return uidString;
}

String NfcTag::getTagType()
{
    return _tagType;
}

boolean NfcTag::hasNdefMessage()
{
    return (_ndefMessage != NULL);
}

NdefMessage* NfcTag::getNdefMessage()
{
    return _ndefMessage;
}

uint8_t NfcTag::getErrorCode()
{
    return _errorCode;
}

void NfcTag::print()
{
    String uid = getUidString();
    NRF_LOG_INFO("NFC Tag: type=%s , uid=%s", _tagType.c_str(), uid.c_str());
    if (_ndefMessage == NULL)
    {
        NRF_LOG_INFO("No NDEF Message");
    }
    else
    {
        _ndefMessage->print();
    }
}
