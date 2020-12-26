#ifndef NdefMessage_h
#define NdefMessage_h

#include <Ndef.h>
#include <NdefRecord.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define MAX_NDEF_RECORDS 4

class NdefMessage
{
    public:
        NdefMessage(void);
        NdefMessage(const uint8_t *data, const uint16_t numBytes);
        NdefMessage(const NdefMessage& rhs);
        ~NdefMessage();
        NdefMessage& operator=(const NdefMessage& rhs);

        uint16_t getEncodedSize(); // need so we can pass array to encode
        void encode(uint8_t *data);

        bool addRecord(NdefRecord& record);
        void addMimeMediaRecord(const uint8_t * mimeType, const uint8_t* payload);
        void addMimeMediaRecord(const uint8_t * mimeType, const uint8_t *payload, uint16_t payloadLength);
        void addTextRecord(const uint8_t* text);
        void addTextRecord(const uint8_t* text, const uint8_t* encoding);
        void addUriRecord(const uint8_t* uri);
        void addEmptyRecord();

        uint16_t getRecordCount();
        NdefRecord getRecord(uint16_t index);
        NdefRecord operator[](uint16_t index);

#ifdef NDEF_USE_SERIAL
        void print();
#endif
    private:
        NdefRecord _records[MAX_NDEF_RECORDS];
        uint16_t _recordCount;
};

#endif