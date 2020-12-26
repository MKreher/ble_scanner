#ifndef NfcTag_h
#define NfcTag_h

#include <stdio.h>
#include <inttypes.h>
#include <NdefMessage.h>

class NfcTag
{
    public:
        NfcTag();
        NfcTag(uint8_t *uid, uint16_t uidLength);
        NfcTag(uint8_t *uid, uint16_t uidLength, const char* tagType);
        NfcTag(uint8_t *uid, uint16_t uidLength, const char* tagType, NdefMessage& ndefMessage);
        NfcTag(uint8_t *uid, uint16_t uidLength, const char* tagType, const uint8_t *ndefData, const uint16_t ndefDataLength);
        ~NfcTag(void);
        NfcTag& operator=(const NfcTag& rhs);
        uint8_t getUidLength();
        void getUid(uint8_t *uid, uint16_t uidLength);
        char* getUidString();
        char* getTagType();
        bool hasNdefMessage();
        NdefMessage getNdefMessage();
#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        uint8_t *_uid;
        uint16_t _uidLength;
        char* _tagType; // Mifare Classic, NFC Forum Type {1,2,3,4}, Unknown
        NdefMessage* _ndefMessage;
        // TODO capacity
        // TODO isFormatted
};

#endif