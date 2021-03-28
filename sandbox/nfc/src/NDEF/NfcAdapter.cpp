#include <NfcAdapter.h>

extern "C" {
  #include "nrf_log.h"
}

NfcAdapter::NfcAdapter(nrf_drv_spi_t p_spi)
{
    shield = new PN532(p_spi);
}

NfcAdapter::~NfcAdapter(void)
{
    delete shield;
}

boolean NfcAdapter::begin(boolean verbose)
{
    shield->begin();

    uint32_t versiondata = shield->getFirmwareVersion();

    if (! versiondata)
    {
        NRF_LOG_DEBUG("Didn't find PN53x board");
        return false;
    }

    if (verbose)
    {
        NRF_LOG_DEBUG("Found chip PN5%02x", (versiondata >> 24) & 0xFF);
        NRF_LOG_DEBUG("Firmware version %d.%d", (versiondata >> 16) & 0xFF, (versiondata >> 8)  & 0xFF);
    }
    // configure board to read RFID tags
    shield->SAMConfig();
    return true;
}

boolean NfcAdapter::tagPresent(unsigned long timeout)
{
    uint8_t success;
    uidLength = 0;

    if (timeout == 0)
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A_BAUD, uid, (uint8_t*)&uidLength);
    }
    else
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A_BAUD, uid, (uint8_t*)&uidLength, timeout);
    }
    return success;
}

boolean NfcAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

boolean NfcAdapter::format()
{
    boolean success;
    if (uidLength == 4)
    {
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.formatNDEF(uid, uidLength);
    }
    else
    {
        NRF_LOG_DEBUG("Unsupported Tag.");
        success = false;
    }
    return success;
}

boolean NfcAdapter::clean()
{
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        NRF_LOG_DEBUG("Cleaning Mifare Classic");
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.formatMifare(uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        NRF_LOG_DEBUG("Cleaning Mifare Ultralight");
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.clean();
    }
    else
    {
        NRF_LOG_DEBUG("No driver for card type %d", type);
        return false;
    }

}


NfcTag* NfcAdapter::read()
{
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        NRF_LOG_DEBUG("Reading Mifare Classic");
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        NRF_LOG_DEBUG("Reading Mifare Ultralight");
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.read(uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        NRF_LOG_DEBUG("Can not determine tag type");
        return new NfcTag(uid, uidLength, NFCTAG_OTHER_ERROR);
    }
    else
    {
        NRF_LOG_DEBUG("No driver for card type %d", type);
        return new NfcTag(uid, uidLength, NFCTAG_OTHER_ERROR);
    }

}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
    boolean success;
    uint8_t type = guessTagType();

    if (type == TAG_TYPE_MIFARE_CLASSIC)
    {
        NRF_LOG_DEBUG("Writing Mifare Classic");
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_2)
    {
        NRF_LOG_DEBUG("Writing Mifare Ultralight");
        MifareUltralight mifareUltralight = MifareUltralight(*shield);
        success = mifareUltralight.write(ndefMessage, uid, uidLength);
    }
    else if (type == TAG_TYPE_UNKNOWN)
    {
        NRF_LOG_DEBUG("Can not determine tag type");
        success = false;
    }
    else
    {
        NRF_LOG_DEBUG("No driver for card type %d", type);
        success = false;
    }

    return success;
}

// TODO this should return a Driver MifareClassic, MifareUltralight, Type 4, Unknown
// Guess Tag Type by looking at the ATQA and SAK values
// Need to follow spec for Card Identification. Maybe AN1303, AN1305 and ???
unsigned int NfcAdapter::guessTagType()
{

    // 4 byte id - Mifare Classic
    //  - ATQA 0x4 && SAK 0x8
    // 7 byte id
    //  - ATQA 0x44 && SAK 0x8 - Mifare Classic
    //  - ATQA 0x44 && SAK 0x0 - Mifare Ultralight NFC Forum Type 2
    //  - ATQA 0x344 && SAK 0x20 - NFC Forum Type 4

    if (uidLength == 4)
    {
        return TAG_TYPE_MIFARE_CLASSIC;
    }
    else
    {
        return TAG_TYPE_2;
    }
}
