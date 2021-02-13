extern "C" {
  #include "sdk_config.h"
  #include "app_error.h"
  #include "nrf_delay.h"
  #include "nrf_log.h"
  #include "nrf_gpio.h"
  #include "nrf_drv_gpiote.h"
  #include "nrf_drv_spi.h"
}

#include "PN532_SPI.h"

#define STATUS_READ     2
#define DATA_WRITE      1
#define DATA_READ       3

static bool g_spi_xfer_done; // Flag used to indicate that SPI instance completed the transfer.
static nrf_drv_spi_t g_spi;

inline void pn532_spi_event_handler(nrf_drv_spi_evt_t const * p_event, void * p_context)
{
    g_spi_xfer_done = true;
}

inline void pn532_irq_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (pin = PN532_IRQ_PIN)
    {
        NRF_LOG_INFO("PN532 IRQ: %d", action);
    } else {
        NRF_LOG_INFO("Unknow pin: %d", pin);
    }
}

PN532_SPI::PN532_SPI(nrf_drv_spi_t p_spi)
{
    m_command = 0;
    g_spi = p_spi;
}

void PN532_SPI::begin()
{
  nrf_gpio_cfg_output(SPI_SS_PIN_PN532);
  nrf_gpio_cfg_input(PN532_IRQ_PIN, NRF_GPIO_PIN_NOPULL);
       
  // regarding spi config see page 25 in pn532 datasheet
  nrf_drv_spi_config_t spi_config = 
    {
      spi_config.sck_pin = SPI_SCK_PIN,
      spi_config.mosi_pin = SPI_MOSI_PIN,
      spi_config.miso_pin = SPI_MISO_PIN,
      spi_config.ss_pin = SPI_SS_PIN_PN532,
      spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      spi_config.orc = 0xFF,
      spi_config.frequency = NRF_DRV_SPI_FREQ_2M, //NRF_DRV_SPI_FREQ_4M,
      spi_config.mode = NRF_DRV_SPI_MODE_0,
      spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST // PN532 uses LSB !!!
    };

  nrf_drv_spi_init(&g_spi, &spi_config, pn532_spi_event_handler, NULL);
}

void PN532_SPI::wakeup()
{    
    nrf_gpio_pin_clear(SPI_SS_PIN_PN532);
    nrf_delay_ms(2);
    nrf_gpio_pin_set(SPI_SS_PIN_PN532);
}



int8_t PN532_SPI::writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    m_command = header[0];
    writeFrame(header, hlen, body, blen);
    
    uint8_t timeout = PN532_ACK_WAIT_TIME;
    while (!isReady()) {
        nrf_delay_ms(1);
        timeout--;
        if (0 == timeout) {
            NRF_LOG_INFO("Time out when waiting for ACK");
            return PN532_TIMEOUT;
        }
    }
    if (readAckFrame()) {
        NRF_LOG_INFO("Invalid ACK");
        return PN532_INVALID_ACK;
    }
    return 0;
}

int16_t PN532_SPI::readResponse(uint8_t *p_resp_buff, uint8_t p_resp_len, uint16_t p_timeout)
{
    // Die zu lesende Byte-Anzahl ist um eins größer als das eigentliche Response-Array des jeweiligen Commands,
    // da als erstes das DATA_READ byte gesendet werden muss.
    uint8_t read_len = p_resp_len + 1;

    uint16_t time = 0;

    while (!isReady()) {
        nrf_delay_ms(1);
        time++;
        if (p_timeout > 0 && time > p_timeout) {
            return PN532_TIMEOUT;
        }
    }

    nrf_gpio_pin_clear(SPI_SS_PIN_PN532);
    nrf_delay_ms(1);

    NRF_LOG_INFO("Read response from PN532");

    g_spi_xfer_done = false;
    
    uint8_t tx_buff[read_len];
    memcpy(tx_buff, NULL, read_len);
    tx_buff[0] = DATA_READ; // DATA_READ Byte als erstes senden
    
    uint8_t rx_buff[read_len];
            
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, tx_buff, read_len, rx_buff, read_len));

    while (!g_spi_xfer_done)
    {
        //NRF_LOG_INFO("SPI read in progress...");
        __WFE();
    }
  
    //NRF_LOG_INFO("SPI Response read:");
    //NRF_LOG_HEXDUMP_INFO(rx_buff, read_len);

    // Das empfangene Byte Array enthällt als erstes Element das Response-Byte auf das DATA_READ Byte.
    // Das Byte Array umkopieren auf des Response Array und dabei dieses erste Byte weglassen.
    memcpy(p_resp_buff, rx_buff+1, p_resp_len);
    NRF_LOG_INFO("PN532 Response:");
    NRF_LOG_HEXDUMP_INFO(p_resp_buff, p_resp_len);

    int16_t result = 0;

    nrf_gpio_pin_set(SPI_SS_PIN_PN532);

    return result;
}

bool PN532_SPI::isReady()
{   
    if (nrf_gpio_pin_read(PN532_IRQ_PIN) == 0)
    {
      //NRF_LOG_INFO("PN532 IRQ is LOW (INTERUPT!!!)");
      return true;
    }
    
    //NRF_LOG_INFO("PN532 IRQ is HIGH");
    return false;
    
    /* wenn der IRO Pin gespart werden soll,
       kann auch STATUS_READ Command zur Busy Abfrage
       gesendet werden
    nrf_gpio_pin_clear(PN532_SPI_SS);

    write(STATUS_READ);
    uint8_t status = read() & 1;
    nrf_gpio_pin_set(PN532_SPI_SS);
    return status;
    */
}

void PN532_SPI::writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    nrf_gpio_pin_clear(SPI_SS_PIN_PN532);
    nrf_delay_ms(2); // wake up PN532

    uint8_t tx_buff[64];
    uint8_t len = 0;
    tx_buff[len++] = DATA_WRITE;
    tx_buff[len++] = PN532_PREAMBLE;
    tx_buff[len++] = PN532_STARTCODE1;
    tx_buff[len++] = PN532_STARTCODE2;

    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    tx_buff[len++] = length;
    tx_buff[len++] = (~length + 1);         // checksum of length

    tx_buff[len++] = PN532_HOSTTOPN532;
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    for (uint8_t i = 0; i < hlen; i++) {
        tx_buff[len++] = header[i];
        sum += header[i];
    }
    for (uint8_t i = 0; i < blen; i++) {
        tx_buff[len++] = body[i];
        sum += body[i];
    }

    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    tx_buff[len++] = checksum;
    tx_buff[len++] = PN532_POSTAMBLE;

    write(tx_buff, len);
    
    nrf_gpio_pin_set(SPI_SS_PIN_PN532);
}

int8_t PN532_SPI::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    nrf_gpio_pin_clear(SPI_SS_PIN_PN532);
    nrf_delay_ms(1);

    uint8_t tx_buff[1];
    memcpy(tx_buff, NULL, 1);
    tx_buff[0] = DATA_READ;

    write(tx_buff, sizeof(tx_buff));

    uint8_t ackBuff[sizeof(PN532_ACK)];
    memcpy(ackBuff, NULL, sizeof(PN532_ACK));
    
    g_spi_xfer_done = false;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, NULL, 0, ackBuff, sizeof(PN532_ACK)));

    while (!g_spi_xfer_done)
    {
        //NRF_LOG_INFO("SPI transfer in progress...");
        __WFE();
    }

    NRF_LOG_INFO("SPI ACK-Frame read:");
    NRF_LOG_HEXDUMP_INFO(ackBuff, sizeof(PN532_ACK));

    nrf_gpio_pin_set(SPI_SS_PIN_PN532);

    return memcmp(ackBuff, PN532_ACK, sizeof(PN532_ACK));
}

void PN532_SPI::write(uint8_t* p_data, uint8_t p_len) {
  NRF_LOG_INFO("Write data to PN532...");

  g_spi_xfer_done = false;

  NRF_LOG_INFO("SPI writing data:");
  NRF_LOG_HEXDUMP_INFO(p_data, p_len);

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, p_data, p_len, NULL, 0));

  while (!g_spi_xfer_done)
  {
      //NRF_LOG_INFO("SPI write in progress...");
      __WFE();
  }
  
  NRF_LOG_INFO("SPI write done.");
}
