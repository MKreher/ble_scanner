#ifndef NfcTag_h
#define NfcTag_h

#include <inttypes.h>
#include <NdefMessage.h>
#include "Ndef.h"

class NfcTag
{
    public:
        NfcTag();
        NfcTag(byte *uid, unsigned int uidLength, const uint8_t errorCode);
        NfcTag(byte *uid, unsigned int uidLength, String tagType, const uint8_t errorCode);
        NfcTag(byte *uid, unsigned int uidLength, String tagType, NdefMessage& ndefMessage, const uint8_t errorCode);
        NfcTag(byte *uid, unsigned int uidLength, String tagType, const byte *ndefData, const int ndefDataLength, const uint8_t errorCode);
        ~NfcTag(void);
        NfcTag& operator=(const NfcTag& rhs);
        uint8_t getUidLength();
        void getUid(byte *uid, unsigned int uidLength);
        String getUidString();
        String getTagType();
        boolean hasNdefMessage();
        boolean isNdefFormatted();
        NdefMessage* getNdefMessage();
        uint8_t getErrorCode();
        void print();
    private:
        byte *_uid;
        unsigned int _uidLength;
        String _tagType; // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        NdefMessage* _ndefMessage;
        uint8_t _errorCode;
};

#endif