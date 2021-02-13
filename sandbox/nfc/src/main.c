/**
 * Copyright (c) 2015 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/**
 * @file
 * @brief An example of use of Adafruit tag reader combined with Type 2 Tag parser.
 *
 * @sa nfc_adafruit_tag_reader_example
 */

/**
 * @defgroup nfc_adafruit_tag_reader_example This example presents combined use of the Adafruit tag reader
 *      (@ref adafruit_pn532) library with Type 2 Tag parser (@ref nfc_type_2_tag_parser).

 */

#include <stdbool.h>
#include <stdint.h>

#include "boards.h"
#include "app_util_platform.h"
#include "app_timer.h"
#include "app_scheduler.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_clock.h"
//#include "nrf_spi_mngr.h"
#include "app_error.h"
#include "bsp.h"
#include "hardfault.h"
#include "nrf_delay.h"
#include "sdk_macros.h"
#include "sdk_config.h"
#include "nrf_drv_gpiote.h"


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "PN532_Wrapper.h"

#define SCHED_MAX_EVENT_DATA_SIZE 16 // Maximum size of scheduler events
#define SCHED_QUEUE_SIZE 196         // Maximum number of events in the scheduler queue

static const nrf_drv_spi_t spi0 = NRF_DRV_SPI_INSTANCE(0);

static bool g_read_nfc = false;
static PN532 *nfc;

static void lfclk_config(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_clock_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_clock_lfclk_request(NULL);
}

void init_pn532() {
  NRF_LOG_INFO("init_pn532()");
  nfc = createPN532(spi0);
  pn532_begin(nfc);

  uint32_t versiondata = pn532_getFirmwareVersion(nfc);

  NRF_LOG_INFO("Found chip PN5%02x", (versiondata >> 24) & 0xFF);
  NRF_LOG_INFO("Firmware version %d.%d", (versiondata >> 16) & 0xFF,
                                             (versiondata >> 8)  & 0xFF);

  pn532_SAMConfig(nfc);

  destroyPN532(nfc);
}


void read_mifare_tag()
{
  NRF_LOG_INFO("read_mifare_tag()");

  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
    
  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  //success = pn532_readPassiveTargetID(nfc, PN532_MIFARE_ISO14443A_BAUD, uid, &uidLength, 1000);
  success = pn532_readPassiveTargetID(nfc, 0x00, uid, &uidLength, 1000);
  
  if (success) {
    // Display some basic information about the card
    NRF_LOG_INFO("Found an ISO14443A card");
    NRF_LOG_INFO("  UID Length: %d bytes", uidLength);
    NRF_LOG_INFO("  UID Value: ");
    pn532_PrintHex(nfc, uid, uidLength);
    
    if (uidLength == 4)
    {
      // We probably have a Mifare Classic card ... 
      NRF_LOG_INFO("Seems to be a Mifare Classic card (4 byte UID)");
	  
      // Now we need to try to authenticate it for read/write access
      // Try with the factory default KeyA: 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF
      NRF_LOG_INFO("Trying to authenticate block 4 with default KEYA value");
      uint8_t keya[6] = { 0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7 };
      //uint8_t keya[6] = { 0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5 };
      //uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	  
      // Start with block 4 (the first block of sector 1) since sector 0
      // contains the manufacturer data and it's probably better just
      // to leave it alone unless you know what you're doing
      success = pn532_mifareclassic_AuthenticateBlock(nfc, uid, uidLength, 4, 0, keya);
	  
      if (success)
      {
        NRF_LOG_INFO("Sector 1 (Blocks 4..7) has been authenticated");
        uint8_t data[16];
		
        // If you want to write something to block 4 to test with, uncomment
        // the following line and this text should be read back in a minute
        // data = { 'a', 'd', 'a', 'f', 'r', 'u', 'i', 't', '.', 'c', 'o', 'm', 0, 0, 0, 0};
        // success = nfc.mifareclassic_WriteDataBlock (4, data);

        // Try to read the contents of block 4
        NRF_LOG_INFO("Reading block 4");
        success = pn532_mifareclassic_ReadDataBlock(nfc, 4, data);
		
        if (success)
        {
          // Data seems to have been read ... spit it out
          NRF_LOG_INFO("Card content:");
          pn532_PrintHex(nfc, data, 16);
		  
          // Wait a bit before reading the card again
          nrf_delay_ms(3000);
        }
        else
        {
          NRF_LOG_INFO("Ooops ... unable to read the requested block.  Try another key?");
          nrf_delay_ms(3000);
        }
      }
      else
      {
        NRF_LOG_INFO("Ooops ... authentication failed: Try another key?");
        nrf_delay_ms(3000);
      }
    } //if (uidLength == 4)
  
    if (uidLength == 7)
    {
      // We probably have a Mifare Ultralight card ...
      NRF_LOG_INFO("Seems to be a Mifare Ultralight tag (7 byte UID)");
	  
      // Try to read the first general-purpose user page (#4)
      NRF_LOG_INFO("Reading page 4");
      uint8_t data[32];
      success = pn532_mifareultralight_ReadPage (nfc, 4, data);
      if (success)
      {
        NRF_LOG_INFO("Card content:");
        // Data seems to have been read ... spit it out
        pn532_PrintHex(nfc, data, 4);
                
        // Wait a bit before reading the card again
        nrf_delay_ms(3000);
      }
      else
      {
        NRF_LOG_INFO("Ooops ... unable to read the requested page!?");
        nrf_delay_ms(3000);
      }
    } //if (uidLength == 7)

  } // if (success)

}

void button1_scheduled_event_handler(void * p_event_data, uint16_t event_size)
{
    // execution in thread/main mode.

    if (current_int_priority_get() == APP_IRQ_PRIORITY_THREAD)
    {
        NRF_LOG_INFO("button1_scheduled_event_handler() [executing in thread/main mode]");
    }
    else
    {
        NRF_LOG_INFO("button1_scheduled_event_handler() [executing in interrupt handler mode]");
    }
    
}

void bsp_event_handler(bsp_event_t event)
{
    switch (event)
    {
    case BSP_EVENT_KEY_0:
        NRF_LOG_INFO("Button 1.");
        app_sched_event_put(NULL, 0, button1_scheduled_event_handler);
        break;
    case BSP_EVENT_KEY_1:
        NRF_LOG_INFO("Button 2.");
        g_read_nfc = false;
        bsp_board_led_invert(BSP_BOARD_LED_0);
        break;
    case BSP_EVENT_KEY_2:
        NRF_LOG_INFO("Button 3.");
        break;
    case BSP_EVENT_KEY_3:
        NRF_LOG_INFO("Button 4.");
        break;
    default:
        break;
    }
}

/**

/**
 * @brief Function for initializations not directly related to Adafruit.
 */
void utils_setup(void)
{
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    lfclk_config();
    APP_ERROR_CHECK(app_timer_init());
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE);
    APP_ERROR_CHECK(bsp_init(BSP_INIT_BUTTONS, bsp_event_handler));

    bsp_board_init(BSP_INIT_LEDS);
}

int main(void)
{
    ret_code_t err_code;

    utils_setup();

    init_pn532();
    
    NRF_LOG_INFO("Firmware initialization finshed.");

    g_read_nfc = true;

    while (g_read_nfc == true)
    {
      read_mifare_tag();
      __WFE();
      app_sched_execute();
      //nrf_pwr_mgmt_run();
      NRF_LOG_FLUSH();
    }

    while (true)
    {      
      __WFE();
      app_sched_execute();
      //nrf_pwr_mgmt_run();
      NRF_LOG_FLUSH();
    }
}


/** @} */ /* End of group nfc_adafruit_tag_reader_example */

