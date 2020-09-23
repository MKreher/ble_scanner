/**
 *  @filename   :   epdif.cpp
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     August 10 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "sdk_common.h"
#include "nrf_drv_spi.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"

#include "epdif.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);

#define LOW  0
#define HIGH 1

EpdIf::EpdIf() {
};

EpdIf::~EpdIf() {
};

void EpdIf::DigitalWrite(int pin, int value) {
    nrf_gpio_pin_write(pin, value);
}

int EpdIf::DigitalRead(int pin) {
    return nrf_gpio_pin_read(pin);
}

void EpdIf::DelayMs(unsigned int delaytime) {
    nrf_delay_ms(delaytime);
}

void EpdIf::SpiTransfer(const uint8_t* data) {
    DigitalWrite(SPI_CS_PIN, LOW);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, sizeof(data), NULL, 0));
    DigitalWrite(SPI_CS_PIN, HIGH);
}

int EpdIf::IfInit(void) {
    nrf_gpio_cfg_output(SPI_CS_PIN);
    nrf_gpio_cfg_output(RST_PIN);
    nrf_gpio_cfg_output(DC_PIN);
    nrf_gpio_cfg_input(BUSY_PIN, NRF_GPIO_PIN_NOPULL);
    
    nrf_drv_spi_config_t spi_config =
    {
        spi_config.sck_pin        = SPI_CLK_PIN,
        spi_config.mosi_pin       = SPI_MOSI_PIN,
        spi_config.miso_pin       = NRF_DRV_SPI_PIN_NOT_USED,
        spi_config.ss_pin         = SPI_CS_PIN,
        spi_config.irq_priority   = APP_IRQ_PRIORITY_LOWEST,
        spi_config.orc            = 0xFF,
        spi_config.frequency      = NRF_DRV_SPI_FREQ_8M,
        spi_config.mode           = NRF_DRV_SPI_MODE_0,
        spi_config.bit_order      = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
    };
    
    ret_code_t err_code;
    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);
    
    return err_code;
}

