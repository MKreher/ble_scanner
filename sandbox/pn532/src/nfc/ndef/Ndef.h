#ifndef Ndef_h
#define Ndef_h

// To save memory and stop serial output comment out the next line
#define NDEF_USE_SERIAL

/* NOTE: To use the Ndef library in your code, don't include Ndef.h
   See README.md for details on which files to include in your sketch.
*/

#include <stdio.h>

#include "app_util_platform.h"

#ifdef NDEF_USE_SERIAL
void PrintHex(const uint8_t *data, const uint32_t numBytes);
void PrintHexChar(const uint8_t *data, const uint32_t numBytes);
void DumpHex(const uint8_t *data, const uint32_t numBytes, const uint16_t blockSize);
#endif

#endif
