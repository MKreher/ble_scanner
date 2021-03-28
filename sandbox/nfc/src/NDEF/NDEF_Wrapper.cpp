#include "NDEF_Wrapper.h"
#include "NfcAdapter.h"

extern "C"
{

#include "nrf_log.h"

    // ===================================================================
    // NFC-Adpater functions
    // ===================================================================

    NfcTag * g_NfcTag;
    NdefMessage * g_NdefMessage;

    NfcAdapter * create_nfc_adapter(nrf_drv_spi_t p_spi) { return new NfcAdapter(p_spi); }

    void destroy_nfc_adapter(NfcAdapter * nfcAdapter) { delete nfcAdapter; }

    bool nfc_begin(NfcAdapter * nfcAdapter, bool verbose) { return nfcAdapter->begin(verbose); }

    bool nfc_tag_present(NfcAdapter * nfcAdapter, unsigned long timeout)
    {
        return nfcAdapter->tagPresent(timeout);
    }

    NfcTag * nfc_read(NfcAdapter * nfcAdapter)
    {
        g_NfcTag = nfcAdapter->read();
        return g_NfcTag;
    }

    bool nfc_write(NfcAdapter * nfcAdapter, NdefMessage * ndefMessage)
    {
        return nfcAdapter->write(*ndefMessage);
    }

    bool nfc_erase(NfcAdapter * nfcAdapter) { return nfcAdapter->erase(); }

    bool nfc_format(NfcAdapter * nfcAdapter) { return nfcAdapter->format(); }

    bool nfc_clean(NfcAdapter * nfcAdapter) { return nfcAdapter->clean(); }


    // ===================================================================
    // NFC-Tag functions
    // ===================================================================
    NfcTag * create_nfc_tag() { return new NfcTag(); }

    NfcTag * create_nfc_tag2(uint8_t * uid, unsigned int uidLength)
    {
        return new NfcTag(uid, uidLength, NFCTAG_NO_ERROR);
    }

    NfcTag * create_nfc_tag3(uint8_t * uid, unsigned int uidLength, const char * tagType)
    {
        String * tagTypeStr = new String(tagType);
        return new NfcTag(uid, uidLength, *tagTypeStr, NFCTAG_NO_ERROR);
    }

    NfcTag * create_nfc_tag4(
        uint8_t * uid, unsigned int uidLength, const char * tagType, NdefMessage * ndefMessage)
    {
        String * tagTypeStr = new String(tagType);
        return new NfcTag(uid, uidLength, *tagTypeStr, *ndefMessage, NFCTAG_NO_ERROR);
    }

    void destroy_nfc_tag(NfcTag * nfcTag) { delete nfcTag; }

    bool nfc_tag_equals(const NfcTag * lhs, const NfcTag * rhs) { return lhs == rhs; }

    uint8_t nfc_tag_get_uid_length(NfcTag * nfcTag) { return nfcTag->getUidLength(); }

    const char * nfc_tag_get_uid(NfcTag * nfcTag) { return nfcTag->getUidString().c_str(); }

    const char * nfc_tag_get_tag_type(NfcTag * nfcTag) { return nfcTag->getTagType().c_str(); }

    boolean nfc_tag_has_ndef_message(NfcTag * nfcTag) { return nfcTag->hasNdefMessage(); }

    NdefMessage * nfc_tag_get_ndef_message(NfcTag * nfcTag)
    {
        g_NdefMessage = nfcTag->getNdefMessage();
        return g_NdefMessage;
    }

    uint8_t nfc_tag_get_error_code(NfcTag * nfcTag) { return nfcTag->getErrorCode(); }

    void nfc_tag_print(NfcTag * nfcTag) { return nfcTag->print(); }


    // ===================================================================
    // NDEF-Message functions
    // ===================================================================

    NdefMessage * create_ndef_message() { return new NdefMessage(); }

    void destroy_ndef_message(NdefMessage * ndefMessage) { delete ndefMessage; }

    uint8_t ndef_message_get_encoded_size(NdefMessage * ndefMessage)
    {
        return ndefMessage->getEncodedSize();
    }

    void ndef_message_encode(NdefMessage * ndefMessage, char * data)
    {
        ndefMessage->encode((byte *)data);
    }

    bool ndef_message_add_record(NdefMessage * ndefMessage, NdefRecord * record)
    {
        return ndefMessage->addRecord(*record);
    }


    void ndef_message_add_media_record(
        NdefMessage * ndefMessage, const char * mimeType, char * payload, int payloadLength)
    {
        String * mimeTypeStr = new String(mimeType);
        return ndefMessage->addMimeMediaRecord(*mimeTypeStr, (byte *)payload, payloadLength);
    }

    void ndef_message_add_text_record(
        NdefMessage * ndefMessage, const char * text, const char * encoding)
    {
        String * textStr = new String(text);
        String * encodingStr = new String(encoding);
        ndefMessage->addTextRecord(*textStr, *encodingStr);
    }

    void ndef_message_add_uri_record(NdefMessage * ndefMessage, const char * uri)
    {
        String * uriStr = new String(uri);
        ndefMessage->addUriRecord(*uriStr);
    }

    void ndef_message_add_empty_record(NdefMessage * ndefMessage) { ndefMessage->addEmptyRecord(); }

    uint8_t ndef_message_get_record_count(NdefMessage * ndefMessage)
    {
        return ndefMessage->getRecordCount();
    }

    NdefRecord* ndef_message_get_record(NdefMessage * ndefMessage, int index)
    {
        NdefRecord ndef_rec = ndefMessage->getRecord(index);
        return new NdefRecord(ndef_rec);
    }

    void ndef_message_print(NdefMessage * ndefMessage) { ndefMessage->print(); }


    // ===================================================================
    // NDEF-Record functions
    // ===================================================================

    NdefRecord * create_ndef_record() { return new NdefRecord(); }

    void destroy_ndef_record(NdefRecord * ndefRecord) { delete ndefRecord; }

    int ndef_record_get_encoded_size(NdefRecord * ndefRecord) { ndefRecord->getEncodedSize(); }

    void ndef_record_encode(
        NdefRecord * ndefRecord, uint8_t * data, bool firstRecord, bool lastRecord)
    {
        ndefRecord->encode(data, firstRecord, lastRecord);
    }

    uint8_t ndef_record_get_type_length(NdefRecord * ndefRecord)
    {
        return ndefRecord->getTypeLength();
    }

    int ndef_record_get_payload_length(NdefRecord * ndefRecord)
    {
        return ndefRecord->getPayloadLength();
    }

    uint8_t ndef_record_get_id_length(NdefRecord * ndefRecord) { return ndefRecord->getIdLength(); }

    uint8_t ndef_record_get_tnf(NdefRecord * ndefRecord) { return ndefRecord->getTnf(); }

    void ndef_record_get_type(NdefRecord * ndefRecord, uint8_t * type)
    {
        ndefRecord->getType(type);
    }

    void ndef_record_get_payload(NdefRecord * ndefRecord, uint8_t * payload)
    {
        ndefRecord->getPayload(payload);
    }

    void ndef_record_get_id(NdefRecord * ndefRecord, uint8_t * id) { ndefRecord->getId(id); }

    void ndef_record_set_tnf(NdefRecord * ndefRecord, uint8_t tnf) { ndefRecord->setTnf(tnf); }

    void ndef_record_set_type(
        NdefRecord * ndefRecord, const uint8_t * type, const unsigned int numBytes)
    {
        ndefRecord->setType(type, numBytes);
    }

    void ndef_record_set_payload(
        NdefRecord * ndefRecord, const uint8_t * payload, const int numBytes)
    {
        ndefRecord->setPayload(payload, numBytes);
    }

    void ndef_record_set_id(
        NdefRecord * ndefRecord, const uint8_t * id, const unsigned int numBytes)
    {
        ndefRecord->setId(id, numBytes);
    }

    void ndef_record_print(NdefRecord * ndefRecord) { ndefRecord->print(); }
}