#include "NdefRecord.h"

#include <stdlib.h>
#include <string.h>

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

NdefRecord::NdefRecord()
{
    //Serial.println("NdefRecord Constructor 1");
    _tnf = 0;
    _typeLength = 0;
    _payloadLength = 0;
    _idLength = 0;
    _type = (uint8_t *)NULL;
    _payload = (uint8_t *)NULL;
    _id = (uint8_t *)NULL;
}

NdefRecord::NdefRecord(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord Constructor 2 (copy)");

    _tnf = rhs._tnf;
    _typeLength = rhs._typeLength;
    _payloadLength = rhs._payloadLength;
    _idLength = rhs._idLength;
    _type = (uint8_t *)NULL;
    _payload = (uint8_t *)NULL;
    _id = (uint8_t *)NULL;

    if (_typeLength)
    {
        _type = (uint8_t*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (uint8_t*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (uint8_t*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);
    }

}

// TODO NdefRecord::NdefRecord(tnf, type, payload, id)

NdefRecord::~NdefRecord()
{
    //Serial.println("NdefRecord Destructor");
    if (_typeLength)
    {
        free(_type);
    }

    if (_payloadLength)
    {
        free(_payload);
    }

    if (_idLength)
    {
        free(_id);
    }
}

NdefRecord& NdefRecord::operator=(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord ASSIGN");

    if (this != &rhs)
    {
        // free existing
        if (_typeLength)
        {
            free(_type);
        }

        if (_payloadLength)
        {
            free(_payload);
        }

        if (_idLength)
        {
            free(_id);
        }

        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        if (_typeLength)
        {
            _type = (uint8_t*)malloc(_typeLength);
            memcpy(_type, rhs._type, _typeLength);
        }

        if (_payloadLength)
        {
            _payload = (uint8_t*)malloc(_payloadLength);
            memcpy(_payload, rhs._payload, _payloadLength);
        }

        if (_idLength)
        {
            _id = (uint8_t*)malloc(_idLength);
            memcpy(_id, rhs._id, _idLength);
        }
    }
    return *this;
}

// size of records in bytes
uint16_t NdefRecord::getEncodedSize()
{
    uint16_t size = 2; // tnf + typeLength
    if (_payloadLength > 0xFF)
    {
        size += 4;
    }
    else
    {
        size += 1;
    }

    if (_idLength)
    {
        size += 1;
    }

    size += (_typeLength + _payloadLength + _idLength);

    return size;
}

void NdefRecord::encode(uint8_t *data, bool firstRecord, bool lastRecord)
{
    // assert data > getEncodedSize()

    uint8_t* data_ptr = &data[0];

    *data_ptr = getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = _typeLength;
    data_ptr += 1;

    if (_payloadLength <= 0xFF) {  // short record
        *data_ptr = _payloadLength;
        data_ptr += 1;
    } else { // uint32_t format
        // 4 bytes but we store length as an uint16_t
        data_ptr[0] = 0x0; // (_payloadLength >> 24) & 0xFF;
        data_ptr[1] = 0x0; // (_payloadLength >> 16) & 0xFF;
        data_ptr[2] = (_payloadLength >> 8) & 0xFF;
        data_ptr[3] = _payloadLength & 0xFF;
        data_ptr += 4;
    }

    if (_idLength)
    {
        *data_ptr = _idLength;
        data_ptr += 1;
    }

    //Serial.println(2);
    memcpy(data_ptr, _type, _typeLength);
    data_ptr += _typeLength;

    if (_idLength)
    {
        memcpy(data_ptr, _id, _idLength);
        data_ptr += _idLength;
    }
    
    memcpy(data_ptr, _payload, _payloadLength);
    data_ptr += _payloadLength;
}

uint8_t NdefRecord::getTnfByte(bool firstRecord, bool lastRecord)
{
    uint16_t value = _tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (_payloadLength <= 0xFF) {
        value = value | 0x10;
    }

    if (_idLength) {
        value = value | 0x8;
    }

    return value;
}

uint8_t NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(uint8_t tnf)
{
    _tnf = tnf;
}

uint16_t NdefRecord::getTypeLength()
{
    return _typeLength;
}

uint16_t NdefRecord::getPayloadLength()
{
    return _payloadLength;
}

uint16_t NdefRecord::getIdLength()
{
    return _idLength;
}

char* NdefRecord::getType()
{
    char type[_typeLength + 1];
    memcpy(type, _type, _typeLength);
    type[_typeLength] = '\0'; // null terminate
    return type;
}

// this assumes the caller created type correctly
void NdefRecord::getType(uint8_t* type)
{
    memcpy(type, _type, _typeLength);
}

void NdefRecord::setType(const uint8_t * type, const uint16_t numBytes)
{
    if(_typeLength)
    {
        free(_type);
    }

    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

// assumes the caller sized payload properly
void NdefRecord::getPayload(uint8_t *payload)
{
    memcpy(payload, _payload, _payloadLength);
}

void NdefRecord::setPayload(const uint8_t * payload, const uint16_t numBytes)
{
    if (_payloadLength)
    {
        free(_payload);
    }

    _payload = (uint8_t*)malloc(numBytes);
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

char* NdefRecord::getId()
{
    char id[_idLength + 1];
    memcpy(id, _id, _idLength);
    id[_idLength] = '\0'; // null terminate
    return id;
}

void NdefRecord::getId(uint8_t *id)
{
    memcpy(id, _id, _idLength);
}

void NdefRecord::setId(const uint8_t * id, const uint16_t numBytes)
{
    if (_idLength)
    {
        free(_id);
    }

    _id = (uint8_t*)malloc(numBytes);
    memcpy(_id, id, numBytes);
    _idLength = numBytes;
}
#ifdef NDEF_USE_SERIAL

void NdefRecord::print()
{
    NRF_LOG_INFO("  NDEF Record");
    NRF_LOG_INFO("    TNF 0x%x ", _tnf);
    switch (_tnf) {
    case TNF_EMPTY:
        NRF_LOG_INFO("Empty");
        break;
    case TNF_WELL_KNOWN:
        NRF_LOG_INFO("Well Known");
        break;
    case TNF_MIME_MEDIA:
        NRF_LOG_INFO("Mime Media");
        break;
    case TNF_ABSOLUTE_URI:
        NRF_LOG_INFO("Absolute URI");
        break;
    case TNF_EXTERNAL_TYPE:
        NRF_LOG_INFO("External");
        break;
    case TNF_UNKNOWN:
        NRF_LOG_INFO("Unknown");
        break;
    case TNF_UNCHANGED:
        NRF_LOG_INFO("Unchanged");
        break;
    case TNF_RESERVED:
        NRF_LOG_INFO("Reserved");
        break;
    default:
        NRF_LOG_INFO("");
    }
    NRF_LOG_INFO("    Type Length 0x%x %d", _typeLength, _typeLength);
    NRF_LOG_INFO("    Payload Length 0x%X %d", _payloadLength, _payloadLength);
    if (_idLength)
    {
        NRF_LOG_INFO("    Id Length 0x%X", _idLength);
    }
    NRF_LOG_INFO("    Type ");PrintHexChar(_type, _typeLength);
    // TODO chunk large payloads so this is readable
    NRF_LOG_INFO("    Payload "));PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        NRF_LOG_INFO("    Id "));PrintHexChar(_id, _idLength);
    }
    NRF_LOG_INFO("    Record is %d bytes", getEncodedSize());

}
#endif
