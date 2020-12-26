#ifndef MifareUltralight_h
#define MifareUltralight_h

#include <PN532.h>
#include <NfcTag.h>
#include <Ndef.h>

class MifareUltralight
{
    public:
        MifareUltralight(PN532& nfcShield);
        ~MifareUltralight();
        NfcTag read(uint8_t *uid, uint16_t uidLength);
        bool write(NdefMessage& ndefMessage, uint8_t *uid, uint16_t uidLength);
        bool clean();
    private:
        PN532* nfc;
        uint16_t tagCapacity;
        uint16_t messageLength;
        uint16_t bufferSize;
        uint16_t ndefStartIndex;
        bool isUnformatted();
        void readCapabilityContainer();
        void findNdefMessage();
        void calculateBufferSize();
};

#endif
