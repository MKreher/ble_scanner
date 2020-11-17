#include "stdint.h"

#include "nrf_delay.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "sdk_common.h"

#include "nrf52840_HAL.h"

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);
static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */


/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

void HAL_Init(void) {
  nrf_gpio_cfg_output(GPIO_PIN_RST);
  nrf_gpio_cfg_output(GPIO_PIN_DC);

  nrf_drv_spi_config_t spi_config = 
    {
      spi_config.sck_pin = SPI_SCK_PIN,
      spi_config.mosi_pin = SPI_MOSI_PIN,
      spi_config.miso_pin = NRF_DRV_SPI_PIN_NOT_USED,
      spi_config.ss_pin = SPI_SS_PIN,
      spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      spi_config.orc = 0xFF,
      spi_config.frequency = NRF_DRV_SPI_FREQ_8M,
      spi_config.mode = NRF_DRV_SPI_MODE_2,
      spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_MSB_FIRST
    };

  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));
};

void SPI_TXBuffer(uint8_t *buffer, uint32_t len) {
  spi_xfer_done = false;

  if (len < 1) {
    return;
  }

  for (uint16_t i = 0; i < len; i++) {
    //NRF_LOG_INFO("SPI_TXBuffer: Data=%X, Size=%d",buffer[i], sizeof(buffer[i]));
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &buffer[i], sizeof(buffer[i]), NULL, 0));
  }

  while (!spi_xfer_done)
  {
    __WFE();
  }
};

void SPI_TXByte(uint8_t data) {
  //NRF_LOG_INFO("SPI_TXBuffer: Data=%X, Size=%d",data, sizeof(data));
  SPI_TXBuffer(&data, sizeof(data));
}

void GPIO_SetPin(uint16_t Port, uint16_t Pin) {
  nrf_gpio_pin_set(Pin);
}

void GPIO_ResetPin(uint16_t Port, uint16_t Pin) {
  nrf_gpio_pin_clear(Pin);
}

void HAL_Delay(uint16_t ms) {
  nrf_delay_ms(ms);
};