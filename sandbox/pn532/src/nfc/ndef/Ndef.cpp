#include "Ndef.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#ifdef NDEF_USE_SERIAL
// Borrowed from Adafruit_NFCShield_I2C
void PrintHex(const uint8_t * data, const uint32_t numBytes)
{
  int32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
     NRF_LOG_INFO("0x");
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      NRF_LOG_INFO("0");
    NRF_LOG_INFO("%X", data[szPos]&0xff);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      NRF_LOG_INFO(" ");
    }
  }
  NRF_LOG_INFO("");
}

// Borrowed from Adafruit_NFCShield_I2C
void PrintHexChar(const uint8_t * data, const uint32_t numBytes)
{
  int32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++)
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
       NRF_LOG_INFO("0");
     NRF_LOG_INFO("%X", data[szPos]);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
       NRF_LOG_INFO(" ");
    }
  }
   NRF_LOG_INFO("  ");
  for (szPos=0; szPos < numBytes; szPos++)
  {
    if (data[szPos] <= 0x1F) {
       NRF_LOG_INFO(".");
    } else {
       NRF_LOG_INFO("%c", data[szPos]);
    }
  }
   NRF_LOG_INFO("");
}

// Note if buffer % blockSize != 0, last block will not be written
void DumpHex(const uint8_t * data, const uint32_t numBytes, const uint16_t blockSize)
{
    uint16_t i;
    for (i = 0; i < (numBytes / blockSize); i++)
    {
        PrintHexChar(data, blockSize);
        data += blockSize;
    }
}
#endif
