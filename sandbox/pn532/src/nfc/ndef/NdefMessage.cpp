#include <NdefMessage.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

NdefMessage::NdefMessage(void)
{
    _recordCount = 0;
}

NdefMessage::NdefMessage(const uint8_t * data, const uint16_t numBytes)
{
    #ifdef NDEF_DEBUG
    Serial.print(F("Decoding "));Serial.print(numBytes);Serial.println(F(" bytes"));
    PrintHexChar(data, numBytes);
    //DumpHex(data, numBytes, 16);
    #endif

    _recordCount = 0;

    uint16_t index = 0;

    while (index <= numBytes)
    {

        // decode tnf - first uint8_t is tnf with bit flags
        // see the NFDEF spec for more info
        uint8_t tnf_byte = data[index];
        // bool mb = tnf_byte & 0x80;
        bool me = tnf_byte & 0x40;
        // bool cf = tnf_byte & 0x20;
        bool sr = tnf_byte & 0x10;
        bool il = tnf_byte & 0x8;
        uint8_t tnf = (tnf_byte & 0x7);

        NdefRecord record = NdefRecord();
        record.setTnf(tnf);

        index++;
        uint16_t typeLength = data[index];

        uint32_t payloadLength = 0;
        if (sr)
        {
            index++;
            payloadLength = data[index];
        }
        else
        {
            payloadLength =
                  (static_cast<uint32_t>(data[index])   << 24)
                | (static_cast<uint32_t>(data[index+1]) << 16)
                | (static_cast<uint32_t>(data[index+2]) << 8)
                |  static_cast<uint32_t>(data[index+3]);
            index += 4;
        }

        uint16_t idLength = 0;
        if (il)
        {
            index++;
            idLength = data[index];
        }

        index++;
        record.setType(&data[index], typeLength);
        index += typeLength;

        if (il)
        {
            record.setId(&data[index], idLength);
            index += idLength;
        }

        record.setPayload(&data[index], payloadLength);
        index += payloadLength;

        addRecord(record);

        if (me) break; // last message
    }

}

NdefMessage::NdefMessage(const NdefMessage& rhs)
{

    _recordCount = rhs._recordCount;
    for (uint16_t i = 0; i < _recordCount; i++)
    {
        _records[i] = rhs._records[i];
    }

}

NdefMessage::~NdefMessage()
{
}

NdefMessage& NdefMessage::operator=(const NdefMessage& rhs)
{

    if (this != &rhs)
    {

        // delete existing records
        for (uint16_t i = 0; i < _recordCount; i++)
        {
            // TODO Dave: is this the right way to delete existing records?
            _records[i] = NdefRecord();
        }

        _recordCount = rhs._recordCount;
        for (uint16_t i = 0; i < _recordCount; i++)
        {
            _records[i] = rhs._records[i];
        }
    }
    return *this;
}

uint16_t NdefMessage::getRecordCount()
{
    return _recordCount;
}

uint16_t NdefMessage::getEncodedSize()
{
    uint16_t size = 0;
    for (uint16_t i = 0; i < _recordCount; i++)
    {
        size += _records[i].getEncodedSize();
    }
    return size;
}

// TODO change this to return uint8_t*
void NdefMessage::encode(uint8_t* data)
{
    // assert sizeof(data) >= getEncodedSize()
    uint8_t* data_ptr = &data[0];

    for (uint16_t i = 0; i < _recordCount; i++)
    {
        _records[i].encode(data_ptr, i == 0, (i + 1) == _recordCount);
        // TODO can NdefRecord.encode return the record size?
        data_ptr += _records[i].getEncodedSize();
    }

}

bool NdefMessage::addRecord(NdefRecord& record)
{

    if (_recordCount < MAX_NDEF_RECORDS)
    {
        _records[_recordCount] = record;
        _recordCount++;
        return true;
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        NRF_LOG_INFO("WARNING: Too many records. Increase MAX_NDEF_RECORDS.");
#endif
        return false;
    }
}

void NdefMessage::addMimeMediaRecord(const uint8_t * mimeType, const uint8_t * payload)
{
    addMimeMediaRecord(mimeType, payload, sizeof(payload));
}

void NdefMessage::addMimeMediaRecord(const uint8_t * mimeType, const uint8_t * payload, uint16_t payloadLength)
{
    NdefRecord r = NdefRecord();
    r.setTnf(TNF_MIME_MEDIA);

    uint8_t type[sizeof(mimeType) + 1];
    r.setType(type, sizeof(mimeType));

    r.setPayload(payload, payloadLength);

    addRecord(r);
}

void NdefMessage::addTextRecord(const uint8_t* text)
{
    addTextRecord(text, (const uint8_t*) atoi("en"));
}

void NdefMessage::addTextRecord(const uint8_t* text, const uint8_t* encoding)
{
    NdefRecord r = NdefRecord();
    r.setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_TEXT[1] = { 0x54 }; // TODO this should be a constant or preprocessor
    r.setType(RTD_TEXT, sizeof(RTD_TEXT));

    // X is a placeholder for encoding length
    // TODO is it more efficient to build w/o string concatenation?
    String payloadString = "X" + encoding + text;

    uint8_t payload[payloadString.length() + 1];
    payloadString.getBytes(payload, sizeof(payload));

    // replace X with the real encoding length
    payload[0] = encoding.length();

    r.setPayload(payload, payloadString.length());

    addRecord(r);
}

void NdefMessage::addUriRecord(String uri)
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_WELL_KNOWN);

    uint8_t RTD_URI[1] = { 0x55 }; // TODO this should be a constant or preprocessor
    r->setType(RTD_URI, sizeof(RTD_URI));

    // X is a placeholder for identifier code
    String payloadString = "X" + uri;

    uint8_t payload[payloadString.length() + 1];
    payloadString.getBytes(payload, sizeof(payload));

    // add identifier code 0x0, meaning no prefix substitution
    payload[0] = 0x0;

    r->setPayload(payload, payloadString.length());

    addRecord(*r);
    delete(r);
}

void NdefMessage::addEmptyRecord()
{
    NdefRecord* r = new NdefRecord();
    r->setTnf(TNF_EMPTY);
    addRecord(*r);
    delete(r);
}

NdefRecord NdefMessage::getRecord(uint16_t index)
{
    if (index > -1 && index < static_cast<uint16_t>(_recordCount))
    {
        return _records[index];
    }
    else
    {
        return NdefRecord(); // would rather return NULL
    }
}

NdefRecord NdefMessage::operator[](uint16_t index)
{
    return getRecord(index);
}

#ifdef NDEF_USE_SERIAL
void NdefMessage::print()
{
    Serial.print(F("\nNDEF Message "));Serial.print(_recordCount);Serial.print(F(" record"));
    _recordCount == 1 ? Serial.print(", ") : Serial.print("s, ");
    Serial.print(getEncodedSize());Serial.println(F(" bytes"));

    for (unsigned uint16_t i = 0; i < _recordCount; i++)
    {
         _records[i].print();
    }
}
#endif
