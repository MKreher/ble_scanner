#ifndef __NDEF_WRAPPER_H__
#define __NDEF_WRAPPER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_drv_spi.h"

typedef struct NfcAdapter NfcAdapter;
typedef struct NfcTag NfcTag;
typedef struct NdefMessage NdefMessage;
typedef struct NdefRecord NdefRecord;

// ===================================================================
// NFC-Adpater functions
// ===================================================================
NfcAdapter* create_nfc_adapter(nrf_drv_spi_t p_spi);
void destroy_nfc_adapter(NfcAdapter* nfcAdapter);
bool nfc_begin(NfcAdapter* nfcAdapter, bool verbose);
bool nfc_tag_present(NfcAdapter* nfcAdapter, unsigned long timeout);
NfcTag* nfc_read(NfcAdapter* nfcAdapter);
bool nfc_write(NfcAdapter* nfcAdapter, NdefMessage* ndefMessage);
bool nfc_erase(NfcAdapter* nfcAdapter);
bool nfc_format(NfcAdapter* nfcAdapter);
bool nfc_clean(NfcAdapter* nfcAdapter);


// ===================================================================
// NFC-Tag functions
// ===================================================================
NfcTag* create_nfc_tag();
NfcTag* create_nfc_tag2(uint8_t* uid, unsigned int uidLength);
NfcTag* create_nfc_tag3(uint8_t* uid, unsigned int uidLength, const char* tagType);
NfcTag* create_nfc_tag4(uint8_t* uid, unsigned int uidLength, const char* tagType, NdefMessage* ndefMessage);
void destroy_nfc_tag(NfcTag* nfcTag);
bool nfc_tag_equals(const NfcTag* lhs, const NfcTag* rhs);
uint8_t nfc_tag_get_uid_length(NfcTag* nfcTag);
const char* nfc_tag_get_uid(NfcTag* nfcTag);
const char* nfc_tag_get_tag_type(NfcTag* nfcTag);
bool nfc_tag_has_ndef_message(NfcTag* nfcTag);
NdefMessage* nfc_tag_get_ndef_message(NfcTag* nfcTag);
uint8_t nfc_tag_get_error_code(NfcTag* nfcTag);
void nfc_tag_print(NfcTag* nfcTag);


// ===================================================================
// NDEF-Message functions
// ===================================================================
NdefMessage* create_ndef_message();
void destroy_ndef_message(NdefMessage* ndefMessage);
uint8_t ndef_message_get_encoded_size(NdefMessage* ndefMessage);
void ndef_message_encode(NdefMessage* ndefMessage, char *data);
bool ndef_message_add_record(NdefMessage* ndefMessage, NdefRecord* record);
void ndef_message_add_media_record(NdefMessage* ndefMessage,  const char* mimeType, char *payload, int payloadLength);
void ndef_message_add_text_record(NdefMessage* ndefMessage, const char* text, const char* encoding);
void ndef_message_add_uri_record(NdefMessage* ndefMessage, const char* uri);
void ndef_message_add_empty_record(NdefMessage* ndefMessage);
uint8_t ndef_message_get_record_count(NdefMessage* ndefMessage);
NdefRecord* ndef_message_get_record(NdefMessage* ndefMessage, int index);
void ndef_message_print(NdefMessage* ndefMessage);

// ===================================================================
// NDEF-Record functions
// ===================================================================

NdefRecord* create_ndef_record();
void destroy_ndef_record(NdefRecord* ndefRecord);
int ndef_record_get_encoded_size(NdefRecord* ndefRecord);
void ndef_record_encode(NdefRecord* ndefRecord, uint8_t *data, bool firstRecord, bool lastRecord);
uint8_t ndef_record_get_type_length(NdefRecord* ndefRecord);
int ndef_record_get_payload_length(NdefRecord* ndefRecord);
uint8_t ndef_record_get_id_length(NdefRecord* ndefRecord);
uint8_t ndef_record_get_tnf(NdefRecord* ndefRecord);
void ndef_record_get_type(NdefRecord* ndefRecord, uint8_t *type);
void ndef_record_get_payload(NdefRecord* ndefRecord, uint8_t *payload);
void ndef_record_get_id(NdefRecord* ndefRecord, uint8_t *id);
void ndef_record_set_tnf(NdefRecord* ndefRecord, uint8_t tnf);
void ndef_record_set_type(NdefRecord* ndefRecord, const uint8_t *type, const unsigned int numBytes);
void ndef_record_set_payload(NdefRecord* ndefRecord, const uint8_t *payload, const int numBytes);
void ndef_record_set_id(NdefRecord* ndefRecord, const uint8_t *id, const unsigned int numBytes);
void ndef_record_print(NdefRecord* ndefRecord);

#ifdef __cplusplus
}
#endif

#endif //__NDEF_WRAPPER_H__