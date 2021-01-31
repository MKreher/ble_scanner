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
  nrf_gpio_cfg_output(PN532_SPI_SS);
  nrf_gpio_cfg_input(PN532_IRQ_PIN, NRF_GPIO_PIN_NOPULL);
       
  // regarding spi config see page 25 in pn532 datasheet
  nrf_drv_spi_config_t spi_config = 
    {
      spi_config.sck_pin = SPI_SCK_PIN,
      spi_config.mosi_pin = SPI_MOSI_PIN,
      spi_config.miso_pin = SPI_MISO_PIN,
      spi_config.ss_pin = PN532_SPI_SS,
      spi_config.irq_priority = SPI_DEFAULT_CONFIG_IRQ_PRIORITY,
      spi_config.orc = 0xFF,
      spi_config.frequency = NRF_DRV_SPI_FREQ_4M,
      spi_config.mode = NRF_DRV_SPI_MODE_0,
      spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST // PN532 uses LSB !!!
    };

  nrf_drv_spi_init(&g_spi, &spi_config, pn532_spi_event_handler, NULL);
}

void PN532_SPI::wakeup()
{    
    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(2);
    nrf_gpio_pin_set(PN532_SPI_SS);
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
            return -2;
        }
    }
    if (readAckFrame()) {
        NRF_LOG_INFO("Invalid ACK");
        return PN532_INVALID_ACK;
    }
    return 0;
}

/*
int16_t PN532_SPI::readResponse(uint8_t buf[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    while (!isReady()) {
        nrf_delay_ms(1);
        time++;
        if (timeout > 0 && time > timeout) {
            return PN532_TIMEOUT;
        }
    }

    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(1);

    int16_t result;
    do {
        write(DATA_READ);

        if (0x00 != read()      ||       // PREAMBLE
                0x00 != read()  ||       // STARTCODE1
                0xFF != read()           // STARTCODE2
           ) {

            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t length = read();
        if (0 != (uint8_t)(length + read())) {   // checksum of length
            result = PN532_INVALID_FRAME;
            break;
        }

        uint8_t cmd = m_command + 1;               // response command
        if (PN532_PN532TOHOST != read() || (cmd) != read()) {
            result = PN532_INVALID_FRAME;
            break;
        }

        NRF_LOG_INFO("read: %x", cmd);

        length -= 2;
        if (length > len) {
            for (uint8_t i = 0; i < length; i++) {
                NRF_LOG_INFO("%x", read());                 // dump message
            }
            NRF_LOG_INFO("\nNot enough space\n");
            read();
            read();
            result = PN532_NO_SPACE;  // not enough space
            break;
        }

        uint8_t sum = PN532_PN532TOHOST + cmd;
        for (uint8_t i = 0; i < length; i++) {
            buf[i] = read();
            sum += buf[i];

            NRF_LOG_INFO("%x", buf[i]);
        }
        //NRF_LOG_INFO('\n');

        uint8_t checksum = read();
        if (0 != (uint8_t)(sum + checksum)) {
            NRF_LOG_INFO("checksum is not ok");
            result = PN532_INVALID_FRAME;
            break;
        }
        read();         // POSTAMBLE

        result = length;
    } while (0);

    nrf_gpio_pin_set(PN532_SPI_SS);

    return result;
}
*/


int16_t PN532_SPI::readResponse(uint8_t rx_buff[], uint8_t len, uint16_t timeout)
{
    uint16_t time = 0;
    while (!isReady()) {
        nrf_delay_ms(1);
        time++;
        if (timeout > 0 && time > timeout) {
            return PN532_TIMEOUT;
        }
    }

    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(1);

    NRF_LOG_INFO("Read data from PN532");

    g_spi_xfer_done = false;
    
    uint8_t tx_buff[len];
    memcpy(tx_buff, 0, len);
    tx_buff[0] = DATA_READ;
            
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, tx_buff, len, rx_buff, len));

    while (!g_spi_xfer_done)
    {
        NRF_LOG_INFO("SPI transfer (read) in progress...");
        __WFE();
    }
  
    NRF_LOG_INFO("SPI received data:");
    NRF_LOG_HEXDUMP_INFO(rx_buff, len);

    NRF_LOG_INFO("SPI transfer (write) done.");
    
    int16_t result = 0;

    nrf_gpio_pin_set(PN532_SPI_SS);

    return result;
}

bool PN532_SPI::isReady()
{   
    if (nrf_gpio_pin_read(PN532_IRQ_PIN) == 0)
    {
      NRF_LOG_INFO("PN532 IRQ is LOW (INTERUPT!!!)");
      return true;
    }
    
    NRF_LOG_INFO("PN532 IRQ is HIGH");
    return false;
    
    /*
    nrf_gpio_pin_clear(PN532_SPI_SS);

    write(STATUS_READ);
    uint8_t status = read() & 1;
    nrf_gpio_pin_set(PN532_SPI_SS);
    return status;
    */
}

/*
void PN532_SPI::writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(2);               // wake up PN532

    write(DATA_WRITE);
    write(PN532_PREAMBLE);
    write(PN532_STARTCODE1);
    write(PN532_STARTCODE2);

    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    write(length);
    write(~length + 1);         // checksum of length

    write(PN532_HOSTTOPN532);
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    //NRF_LOG_INFO("write: ");

    for (uint8_t i = 0; i < hlen; i++) {
        write(header[i]);
        sum += header[i];

        //NRF_LOG_INFO("x%", header[i]);
    }
    for (uint8_t i = 0; i < blen; i++) {
        write(body[i]);
        sum += body[i];

        //NRF_LOG_INFO("x%", body[i]);
    }

    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    write(checksum);
    write(PN532_POSTAMBLE);

    nrf_gpio_pin_set(PN532_SPI_SS);

    //NRF_LOG_INFO('\n');
}
*/

void PN532_SPI::writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body, uint8_t blen)
{
    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(2);               // wake up PN532

    uint8_t tx_buff[64];
    uint8_t l = 0;
    tx_buff[l++] = DATA_WRITE;
    tx_buff[l++] = PN532_PREAMBLE;
    tx_buff[l++] = PN532_STARTCODE1;
    tx_buff[l++] = PN532_STARTCODE2;

    uint8_t length = hlen + blen + 1;   // length of data field: TFI + DATA
    tx_buff[l++] = length;
    tx_buff[l++] = (~length + 1);         // checksum of length

    tx_buff[l++] = PN532_HOSTTOPN532;
    uint8_t sum = PN532_HOSTTOPN532;    // sum of TFI + DATA

    for (uint8_t i = 0; i < hlen; i++) {
        tx_buff[l++] = header[i];
        sum += header[i];
    }
    for (uint8_t i = 0; i < blen; i++) {
        tx_buff[l++] = body[i];
        sum += body[i];
    }

    uint8_t checksum = ~sum + 1;        // checksum of TFI + DATA
    tx_buff[l++] = checksum;
    tx_buff[l++] = PN532_POSTAMBLE;
    
    NRF_LOG_INFO("SPI writing data:");
    NRF_LOG_HEXDUMP_INFO(tx_buff, l);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, tx_buff, l, NULL, 0));

    while (!g_spi_xfer_done)
    {
        //NRF_LOG_INFO("SPI transfer (write) in progress...");
        __WFE();
    }
  
    nrf_gpio_pin_set(PN532_SPI_SS);

    NRF_LOG_INFO("SPI transfer (write) done.");    
}

int8_t PN532_SPI::readAckFrame()
{
    const uint8_t PN532_ACK[] = {0, 0, 0xFF, 0, 0xFF, 0};

    uint8_t ackBuf[sizeof(PN532_ACK)];

    nrf_gpio_pin_clear(PN532_SPI_SS);
    nrf_delay_ms(1);
    write(DATA_READ);

    for (uint8_t i = 0; i < sizeof(PN532_ACK); i++) {
        ackBuf[i] = read();
    }

    NRF_LOG_INFO("SPI ACK-Frame read:");
    NRF_LOG_HEXDUMP_INFO(ackBuf, sizeof(PN532_ACK));

    nrf_gpio_pin_set(PN532_SPI_SS);

    return memcmp(ackBuf, PN532_ACK, sizeof(PN532_ACK));
}

void PN532_SPI::write(uint8_t* p_data, uint8_t data_len) {
  NRF_LOG_INFO("Write data to PN532");

  g_spi_xfer_done = false;
  uint8_t tx_buff[data_len];
  uint8_t rx_buff[data_len];
  memcpy(tx_buff, p_data, data_len);

  NRF_LOG_INFO("SPI writing data:");
  NRF_LOG_HEXDUMP_INFO(tx_buff, data_len);
  APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, tx_buff, data_len, rx_buff, data_len));

  while (!g_spi_xfer_done)
  {
      NRF_LOG_INFO("SPI transfer (write) in progress...");
      __WFE();
  }
  
  NRF_LOG_INFO("SPI received data:");
  NRF_LOG_HEXDUMP_INFO(rx_buff, data_len);

  NRF_LOG_INFO("SPI transfer (write) done.");
}

void PN532_SPI::write(uint8_t data) {
  NRF_LOG_INFO("Write data to PN532: %x", data);

  g_spi_xfer_done = false;
  uint8_t rx_buff;

  if (sizeof(data) < 1) {
      return;
  }

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, &data, 1, NULL, 0));

  while (!g_spi_xfer_done)
  {
      //NRF_LOG_INFO("SPI transfer in progress...");
      __WFE();
  }
};

uint8_t PN532_SPI::read() {
  NRF_LOG_INFO("Read data from PN532...");

  g_spi_xfer_done = false;

  uint8_t rx_buff;

  APP_ERROR_CHECK(nrf_drv_spi_transfer(&g_spi, NULL, 0, &rx_buff, 1));

  while (!g_spi_xfer_done)
  {
      //NRF_LOG_INFO("SPI transfer in progress...");
      __WFE();
  }

  NRF_LOG_INFO("Received data from PN532: %x", rx_buff);
  
  return rx_buff;
}; 
