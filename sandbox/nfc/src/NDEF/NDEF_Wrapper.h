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
// NDEF-Tag functions
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
void nfc_tag_print(NfcTag* nfcTag);


// ===================================================================
// NDEF-Message functions
// ===================================================================


// ===================================================================
// NDEF-Record functions
// ===================================================================


#ifdef __cplusplus
}
#endif

#endif //__NDEF_WRAPPER_H__