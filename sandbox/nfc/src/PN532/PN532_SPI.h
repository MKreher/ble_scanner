
#ifndef __PN532_SPI_H__
#define __PN532_SPI_H__

extern "C" {
  #include "nrf_drv_spi.h"
  #include "nrf_log.h"
}

#include "PN532Interface.h"

class PN532_SPI : public PN532Interface {
public:
    PN532_SPI(nrf_drv_spi_t p_spi);
    
    void begin();
    void wakeup();
    int8_t writeCommand(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);

    int16_t readResponse(uint8_t buf[], uint8_t len, uint16_t timeout);
    
private:    
    uint8_t m_command;    
    
    bool isReady();
    void writeFrame(const uint8_t *header, uint8_t hlen, const uint8_t *body = 0, uint8_t blen = 0);
    int8_t readAckFrame();        
    void write(uint8_t data);
    void write(uint8_t* p_data, uint8_t data_len);
    uint8_t read(); 
};

#endif
