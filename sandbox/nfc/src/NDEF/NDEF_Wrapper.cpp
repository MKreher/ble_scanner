#include "NfcAdapter.h"
#include "NDEF_Wrapper.h"

extern "C" {

#include "nrf_log.h"

// ===================================================================
// NFC-Adpater functions
// ===================================================================

NfcTag* g_NfcTag;
NdefMessage* g_NdefMessage;

NfcAdapter* create_nfc_adapter(nrf_drv_spi_t p_spi)
{
  return new NfcAdapter(p_spi);
}

void destroy_nfc_adapter(NfcAdapter* nfcAdapter)
{
  delete nfcAdapter;
}

bool nfc_begin(NfcAdapter* nfcAdapter, bool verbose)
{
  return nfcAdapter->begin(verbose);
}

bool nfc_tag_present(NfcAdapter* nfcAdapter, unsigned long timeout)
{
  return nfcAdapter->tagPresent(timeout);
}

NfcTag* nfc_read(NfcAdapter* nfcAdapter)
{
  g_NfcTag = nfcAdapter->read();
  return g_NfcTag;
}

bool nfc_write(NfcAdapter* nfcAdapter, NdefMessage* ndefMessage)
{
  return nfcAdapter->write(*ndefMessage);
}

bool nfc_erase(NfcAdapter* nfcAdapter)
{
  return nfcAdapter->erase();
}

bool nfc_format(NfcAdapter* nfcAdapter)
{
  return nfcAdapter->format();
}

bool nfc_clean(NfcAdapter* nfcAdapter)
{
  return nfcAdapter->clean();
}


// ===================================================================
// NFC-Tag functions
// ===================================================================
NfcTag* create_nfc_tag()
{
  return new NfcTag();
}

NfcTag* create_nfc_tag2(uint8_t *uid, unsigned int uidLength)
{
  return new NfcTag(uid, uidLength, NFCTAG_NO_ERROR);
}

NfcTag* create_nfc_tag3(uint8_t *uid, unsigned int uidLength, const char* tagType)
{
  String* tagTypeStr = new String(tagType);
  return new NfcTag(uid, uidLength, *tagTypeStr, NFCTAG_NO_ERROR);
}

NfcTag* create_nfc_tag4(uint8_t *uid, unsigned int uidLength, const char* tagType, NdefMessage* ndefMessage)
{
  String* tagTypeStr = new String(tagType);
  return new NfcTag(uid, uidLength, *tagTypeStr, *ndefMessage, NFCTAG_NO_ERROR);
}

void destroy_nfc_tag(NfcTag* nfcTag)
{
  delete nfcTag;
}

bool nfc_tag_equals(const NfcTag* lhs, const NfcTag* rhs)
{
  return lhs == rhs;
}

uint8_t nfc_tag_get_uid_length(NfcTag* nfcTag)
{
  return nfcTag->getUidLength();
}

const char* nfc_tag_get_uid(NfcTag* nfcTag)
{
  return nfcTag->getUidString().c_str();
}

const char* nfc_tag_get_tag_type(NfcTag* nfcTag)
{
  return nfcTag->getTagType().c_str();
}

boolean nfc_tag_has_ndef_message(NfcTag* nfcTag)
{
  return nfcTag->hasNdefMessage();
}

NdefMessage* nfc_tag_get_ndef_message(NfcTag* nfcTag)
{
  g_NdefMessage = nfcTag->getNdefMessage();
  return g_NdefMessage;
}

uint8_t nfc_tag_get_error_code(NfcTag* nfcTag)
{
  return nfcTag->getErrorCode();
}

void nfc_tag_print(NfcTag* nfcTag)
{
  return nfcTag->print();
}

}